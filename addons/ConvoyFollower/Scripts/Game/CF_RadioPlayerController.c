// Private prerecorded status calls. One convoy leader speaks to the ordering
// player about the convoy or a named unit, independent of world distance.
class CF_RadioEvent
{
	static const int READY = 1;
	static const int HOLDING = 2;
	static const int FOLLOWING = 3;
	static const int FAR_WARNING = 4;
	static const int STUCK = 5;
	static const int LOST = 6;
	static const int REJOINED = 7;
	static const int UNDER_FIRE = 8;
}

modded class SCR_PlayerController
{
	// Mirrored for local ScriptedUserAction visibility; the server still checks
	// every Start/Add/Replace request against its authoritative roster.
	[RplProp()]
	protected int m_iCFConvoyMemberCount;
	protected ref array<int> m_CFRadioEvents = {};
	protected ref array<int> m_CFRadioUnits = {};
	protected ref array<int> m_CFRadioPacks = {};
	// Per-event/unit alternation avoids the same generated wording twice in a row.
	protected ref array<int> m_CFVariantKeys = {};
	protected ref array<int> m_CFVariantValues = {};
	protected bool m_bCFRadioPlaying;
	protected int m_iCFPlayingEvent;
	protected int m_iCFPlayingUnit;
	// Server-side debounce survives a convoy roster change. A new event or a
	// different unit still gets its first report immediately.
	protected ref array<int> m_CFSentRadioEvents = {};
	protected ref array<int> m_CFSentRadioUnits = {};
	protected ref array<float> m_CFSentRadioTimes = {};
	protected bool m_bCFHasRoutineCall;
	protected float m_fCFLastRoutineCallMs;

	bool CF_HasActiveConvoy()
	{
		return m_iCFConvoyMemberCount > 0;
	}

	void CF_SetConvoyMemberCount(int count)
	{
		if (!Replication.IsServer() || m_iCFConvoyMemberCount == count)
			return;
		m_iCFConvoyMemberCount = count;
		Replication.BumpMe();
	}

	void CF_SendRadioCall(int eventId)
	{
		// Retain the previous signature for callers built against the first
		// prototype. The new convoy session uses the numbered form below.
		CF_SendConvoyRadioCall(eventId, 0);
	}

	void CF_SendConvoyRadioCall(int eventId, int unitNumber)
	{
		if (!Replication.IsServer() || !CF_ConvoySettings.Get().m_bVoiceEnabled)
			return;
		if (unitNumber != 0 && (unitNumber < 1 || unitNumber > 10))
		{
			Print("[ConvoyFollower] RADIO_NO_CLIP: unit " + unitNumber + " outside recorded range");
			return;
		}
		if (!CF_ShouldSendRadioCall(eventId, unitNumber))
			return;

		Rpc(CF_RpcDoConvoyRadioCall, eventId, unitNumber, CF_ConvoySettings.Get().m_iVoicePack);
	}

	protected bool CF_ShouldSendRadioCall(int eventId, int unitNumber)
	{
		CF_ConvoySettings settings = CF_ConvoySettings.Get();
		bool routine = CF_IsRoutineRadioEvent(eventId);
		float nowMs = GetGame().GetWorld().GetWorldTime();
		if (routine && m_bCFHasRoutineCall && nowMs >= m_fCFLastRoutineCallMs &&
			nowMs - m_fCFLastRoutineCallMs < settings.m_fRoutineSpacingSeconds * 1000.0)
		{
			Print("[ConvoyFollower] RADIO_REPEAT_SUPPRESSED: routine spacing, event " + eventId);
			return false;
		}

		// Preserve every stuck, lost, and under-fire report. The range and
		// recovered messages can oscillate at their distance thresholds.
		float repeatMs = 0.0;
		if (routine)
			repeatMs = settings.m_fRoutineRepeatSeconds * 1000.0;
		else if (eventId == CF_RadioEvent.FAR_WARNING || eventId == CF_RadioEvent.REJOINED)
			repeatMs = settings.m_fExceptionRepeatSeconds * 1000.0;
		for (int i = 0; i < m_CFSentRadioEvents.Count(); i++)
		{
			if (m_CFSentRadioEvents[i] != eventId || m_CFSentRadioUnits[i] != unitNumber)
				continue;
			if (nowMs >= m_CFSentRadioTimes[i] && nowMs - m_CFSentRadioTimes[i] < repeatMs)
			{
				Print("[ConvoyFollower] RADIO_REPEAT_SUPPRESSED: event " + eventId + ", unit " + unitNumber);
				return false;
			}
			m_CFSentRadioTimes[i] = nowMs;
			if (routine)
			{
				m_bCFHasRoutineCall = true;
				m_fCFLastRoutineCallMs = nowMs;
			}
			return true;
		}

		m_CFSentRadioEvents.Insert(eventId);
		m_CFSentRadioUnits.Insert(unitNumber);
		m_CFSentRadioTimes.Insert(nowMs);
		if (routine)
		{
			m_bCFHasRoutineCall = true;
			m_fCFLastRoutineCallMs = nowMs;
		}
		return true;
	}

	void CF_ClearConvoyRadioQueue()
	{
		if (!Replication.IsServer())
			return;

		Rpc(CF_RpcClearConvoyRadioQueue);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void CF_RpcClearConvoyRadioQueue()
	{
		if (System.IsConsoleApp() || this != GetGame().GetPlayerController())
			return;
		m_CFRadioEvents.Clear();
		m_CFRadioUnits.Clear();
		m_CFRadioPacks.Clear();
		Print("[ConvoyFollower] RADIO_QUEUE_CLEARED: convoy roster changed");
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void CF_RpcDoConvoyRadioCall(int eventId, int unitNumber, int voicePack)
	{
		if (System.IsConsoleApp() || this != GetGame().GetPlayerController())
			return;

		string resourceName;
		int delayMs;
		if (!CF_GetRadioClip(eventId, unitNumber, resourceName, delayMs))
		{
			Print("[ConvoyFollower] RADIO_NO_CLIP: event " + eventId + ", unit " + unitNumber);
			return;
		}

		if (m_bCFRadioPlaying && m_iCFPlayingEvent == eventId && m_iCFPlayingUnit == unitNumber)
			return;

		for (int i = 0; i < m_CFRadioEvents.Count(); i++)
		{
			if (m_CFRadioEvents[i] == eventId && m_CFRadioUnits[i] == unitNumber)
				return;
		}

		bool routine = CF_IsRoutineRadioEvent(eventId);
		if (!routine)
		{
			// An old ready/holding call is stale once a live exception arrives.
			for (int i = m_CFRadioEvents.Count() - 1; i >= 0; i--)
			{
				if (!CF_IsRoutineRadioEvent(m_CFRadioEvents[i]))
					continue;
				m_CFRadioEvents.RemoveOrdered(i);
				m_CFRadioUnits.RemoveOrdered(i);
				m_CFRadioPacks.RemoveOrdered(i);
			}
		}
		if (routine && m_CFRadioEvents.Count() >= 8)
		{
			Print("[ConvoyFollower] RADIO_QUEUE_FULL: routine call dropped");
			return;
		}

		m_CFRadioEvents.Insert(eventId);
		m_CFRadioUnits.Insert(unitNumber);
		m_CFRadioPacks.Insert(voicePack);
		if (!routine)
			Print("[ConvoyFollower] RADIO_URGENT_QUEUED: event " + eventId + ", unit " + unitNumber);
		if (!m_bCFRadioPlaying)
			CF_PlayNextRadioCall();
	}

	protected bool CF_IsRoutineRadioEvent(int eventId)
	{
		return eventId == CF_RadioEvent.READY || eventId == CF_RadioEvent.FOLLOWING ||
			eventId == CF_RadioEvent.HOLDING;
	}

	protected void CF_PlayNextRadioCall()
	{
		if (m_CFRadioEvents.IsEmpty())
		{
			m_bCFRadioPlaying = false;
			m_iCFPlayingEvent = 0;
			m_iCFPlayingUnit = 0;
			return;
		}

		m_bCFRadioPlaying = true;
		int nextIndex = 0;
		for (int i = 0; i < m_CFRadioEvents.Count(); i++)
		{
			if (!CF_IsRoutineRadioEvent(m_CFRadioEvents[i]))
			{
				nextIndex = i;
				break;
			}
		}
		int eventId = m_CFRadioEvents[nextIndex];
		int unitNumber = m_CFRadioUnits[nextIndex];
		int voicePack = m_CFRadioPacks[nextIndex];
		m_CFRadioEvents.RemoveOrdered(nextIndex);
		m_CFRadioUnits.RemoveOrdered(nextIndex);
		m_CFRadioPacks.RemoveOrdered(nextIndex);
		m_iCFPlayingEvent = eventId;
		m_iCFPlayingUnit = unitNumber;

		string resourceName;
		int delayMs;
		bool found = false;
		if (voicePack == 1)
			found = CF_GetGeneratedRadioClip(eventId, unitNumber, CF_NextGeneratedVariant(eventId, unitNumber), resourceName, delayMs);
		if (!found)
			found = CF_GetRadioClip(eventId, unitNumber, resourceName, delayMs);
		if (found)
		{
			AudioHandle handle = AudioSystem.PlaySound(resourceName);
			if (handle == AudioHandle.Invalid)
				Print("[ConvoyFollower] RADIO_PLAY_FAILED: event " + eventId + ", unit " + unitNumber);
			else
				Print("[ConvoyFollower] RADIO_PLAY: event " + eventId + ", unit " + unitNumber);
		}

		// Each generated WAV has its measured duration plus a small gap. The
		// longer assembled unit reports must finish before another call starts.
		GetGame().GetCallqueue().CallLater(CF_PlayNextRadioCall, delayMs, false);
	}

	protected int CF_NextGeneratedVariant(int eventId, int unitNumber)
	{
		int key = eventId * 16 + unitNumber;
		for (int i = 0; i < m_CFVariantKeys.Count(); i++)
		{
			if (m_CFVariantKeys[i] != key)
				continue;
			m_CFVariantValues[i] = 1 - m_CFVariantValues[i];
			return m_CFVariantValues[i];
		}
		int first = Math.RandomInt(0, 2);
		m_CFVariantKeys.Insert(key);
		m_CFVariantValues.Insert(first);
		return first;
	}

	// Generated from docs/convoy-generated-assets.json by the local Kokoro renderer.
	// This method is filled by tools/render_convoy_generated_mapping.py.
	protected bool CF_GetGeneratedRadioClip(int eventId, int unitNumber, int variant, out string resourceName, out int delayMs)
	{
		resourceName = string.Empty;
		delayMs = 250;
		// BEGIN GENERATED RADIO MAPPING
		if (unitNumber == 0 && eventId == CF_RadioEvent.READY && variant == 0)
		{
			resourceName = "{D9FAE199867CB92E}Sounds/LeaderGenerated/leader_ready_b.wav";
			delayMs = 2387;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.READY && variant == 1)
		{
			resourceName = "{01D164076B10D106}Sounds/LeaderGenerated/leader_ready_c.wav";
			delayMs = 2323;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.FOLLOWING && variant == 0)
		{
			resourceName = "{255BCB060CCAF405}Sounds/LeaderGenerated/leader_following_b.wav";
			delayMs = 1782;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.FOLLOWING && variant == 1)
		{
			resourceName = "{58D25EC3581B5998}Sounds/LeaderGenerated/leader_following_c.wav";
			delayMs = 2802;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.HOLDING && variant == 0)
		{
			resourceName = "{8EA7AFD148DB978C}Sounds/LeaderGenerated/leader_holding_b.wav";
			delayMs = 1955;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.HOLDING && variant == 1)
		{
			resourceName = "{93FC44BF5DE2311D}Sounds/LeaderGenerated/leader_holding_c.wav";
			delayMs = 1768;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.FAR_WARNING && variant == 0)
		{
			resourceName = "{3D471EF94EBFAA17}Sounds/LeaderGenerated/leader_far_warning_b.wav";
			delayMs = 2753;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.FAR_WARNING && variant == 1)
		{
			resourceName = "{A68828D938FD2BAE}Sounds/LeaderGenerated/leader_far_warning_c.wav";
			delayMs = 2593;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.STUCK && variant == 0)
		{
			resourceName = "{8B373EF9A9F134A1}Sounds/LeaderGenerated/leader_stuck_b.wav";
			delayMs = 3199;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.STUCK && variant == 1)
		{
			resourceName = "{1D1F99D446B858A2}Sounds/LeaderGenerated/leader_stuck_c.wav";
			delayMs = 2534;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.LOST && variant == 0)
		{
			resourceName = "{FAC1417273E906B7}Sounds/LeaderGenerated/leader_lost_b.wav";
			delayMs = 2855;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.LOST && variant == 1)
		{
			resourceName = "{C5FA1D3120E78C30}Sounds/LeaderGenerated/leader_lost_c.wav";
			delayMs = 2822;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.REJOINED && variant == 0)
		{
			resourceName = "{561BF59B1D0BFEC6}Sounds/LeaderGenerated/leader_rejoined_b.wav";
			delayMs = 1600;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.REJOINED && variant == 1)
		{
			resourceName = "{941085A2033F4DEF}Sounds/LeaderGenerated/leader_rejoined_c.wav";
			delayMs = 2156;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.UNDER_FIRE && variant == 0)
		{
			resourceName = "{29768D589306EAED}Sounds/LeaderGenerated/leader_under_fire_b.wav";
			delayMs = 2628;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.UNDER_FIRE && variant == 1)
		{
			resourceName = "{DB5E76F46462126E}Sounds/LeaderGenerated/leader_under_fire_c.wav";
			delayMs = 2371;
			return true;
		}
		if (unitNumber == 1 && eventId == CF_RadioEvent.FAR_WARNING && variant == 0)
		{
			resourceName = "{E1F27F6AB0EA6B33}Sounds/LeaderGenerated/unit_1_far_warning_b.wav";
			delayMs = 2353;
			return true;
		}
		if (unitNumber == 1 && eventId == CF_RadioEvent.FAR_WARNING && variant == 1)
		{
			resourceName = "{6B9A2057E2E8D2E8}Sounds/LeaderGenerated/unit_1_far_warning_c.wav";
			delayMs = 2206;
			return true;
		}
		if (unitNumber == 1 && eventId == CF_RadioEvent.STUCK && variant == 0)
		{
			resourceName = "{FADF77F7C4060AFB}Sounds/LeaderGenerated/unit_1_stuck_b.wav";
			delayMs = 3088;
			return true;
		}
		if (unitNumber == 1 && eventId == CF_RadioEvent.STUCK && variant == 1)
		{
			resourceName = "{FB042CE03A4CC1FA}Sounds/LeaderGenerated/unit_1_stuck_c.wav";
			delayMs = 3233;
			return true;
		}
		if (unitNumber == 1 && eventId == CF_RadioEvent.LOST && variant == 0)
		{
			resourceName = "{92F1AF961565F8A1}Sounds/LeaderGenerated/unit_1_lost_b.wav";
			delayMs = 2641;
			return true;
		}
		if (unitNumber == 1 && eventId == CF_RadioEvent.LOST && variant == 1)
		{
			resourceName = "{9AF15546544C5FB8}Sounds/LeaderGenerated/unit_1_lost_c.wav";
			delayMs = 3093;
			return true;
		}
		if (unitNumber == 1 && eventId == CF_RadioEvent.REJOINED && variant == 0)
		{
			resourceName = "{54E5891D0A14F271}Sounds/LeaderGenerated/unit_1_rejoined_b.wav";
			delayMs = 2525;
			return true;
		}
		if (unitNumber == 1 && eventId == CF_RadioEvent.REJOINED && variant == 1)
		{
			resourceName = "{18A7CD3256D92C80}Sounds/LeaderGenerated/unit_1_rejoined_c.wav";
			delayMs = 2864;
			return true;
		}
		if (unitNumber == 1 && eventId == CF_RadioEvent.UNDER_FIRE && variant == 0)
		{
			resourceName = "{002D48D6640C37D2}Sounds/LeaderGenerated/unit_1_under_fire_b.wav";
			delayMs = 1863;
			return true;
		}
		if (unitNumber == 1 && eventId == CF_RadioEvent.UNDER_FIRE && variant == 1)
		{
			resourceName = "{0DAEBC5085B93FBE}Sounds/LeaderGenerated/unit_1_under_fire_c.wav";
			delayMs = 2618;
			return true;
		}
		if (unitNumber == 2 && eventId == CF_RadioEvent.FAR_WARNING && variant == 0)
		{
			resourceName = "{A36E51BA0BC9746B}Sounds/LeaderGenerated/unit_2_far_warning_b.wav";
			delayMs = 2347;
			return true;
		}
		if (unitNumber == 2 && eventId == CF_RadioEvent.FAR_WARNING && variant == 1)
		{
			resourceName = "{EB7878AF4FB74013}Sounds/LeaderGenerated/unit_2_far_warning_c.wav";
			delayMs = 2233;
			return true;
		}
		if (unitNumber == 2 && eventId == CF_RadioEvent.STUCK && variant == 0)
		{
			resourceName = "{21525FFB9B1C2E79}Sounds/LeaderGenerated/unit_2_stuck_b.wav";
			delayMs = 3135;
			return true;
		}
		if (unitNumber == 2 && eventId == CF_RadioEvent.STUCK && variant == 1)
		{
			resourceName = "{FB97EDDC9503C990}Sounds/LeaderGenerated/unit_2_stuck_c.wav";
			delayMs = 3232;
			return true;
		}
		if (unitNumber == 2 && eventId == CF_RadioEvent.LOST && variant == 0)
		{
			resourceName = "{818CB99E7BC74DA3}Sounds/LeaderGenerated/unit_2_lost_b.wav";
			delayMs = 2706;
			return true;
		}
		if (unitNumber == 2 && eventId == CF_RadioEvent.LOST && variant == 1)
		{
			resourceName = "{89AC540266152A70}Sounds/LeaderGenerated/unit_2_lost_c.wav";
			delayMs = 3178;
			return true;
		}
		if (unitNumber == 2 && eventId == CF_RadioEvent.REJOINED && variant == 0)
		{
			resourceName = "{60C4F6FFF2E9CC39}Sounds/LeaderGenerated/unit_2_rejoined_b.wav";
			delayMs = 2558;
			return true;
		}
		if (unitNumber == 2 && eventId == CF_RadioEvent.REJOINED && variant == 1)
		{
			resourceName = "{C50884211EE693F8}Sounds/LeaderGenerated/unit_2_rejoined_c.wav";
			delayMs = 2849;
			return true;
		}
		if (unitNumber == 2 && eventId == CF_RadioEvent.UNDER_FIRE && variant == 0)
		{
			resourceName = "{0585C136DEC9F36A}Sounds/LeaderGenerated/unit_2_under_fire_b.wav";
			delayMs = 1921;
			return true;
		}
		if (unitNumber == 2 && eventId == CF_RadioEvent.UNDER_FIRE && variant == 1)
		{
			resourceName = "{A3E5EF4703B08319}Sounds/LeaderGenerated/unit_2_under_fire_c.wav";
			delayMs = 2492;
			return true;
		}
		if (unitNumber == 3 && eventId == CF_RadioEvent.FAR_WARNING && variant == 0)
		{
			resourceName = "{7C39377EFC9C3D00}Sounds/LeaderGenerated/unit_3_far_warning_b.wav";
			delayMs = 2396;
			return true;
		}
		if (unitNumber == 3 && eventId == CF_RadioEvent.FAR_WARNING && variant == 1)
		{
			resourceName = "{858A3C55AED35AC3}Sounds/LeaderGenerated/unit_3_far_warning_c.wav";
			delayMs = 2233;
			return true;
		}
		if (unitNumber == 3 && eventId == CF_RadioEvent.STUCK && variant == 0)
		{
			resourceName = "{3F2F1321ABA85C96}Sounds/LeaderGenerated/unit_3_stuck_b.wav";
			delayMs = 3233;
			return true;
		}
		if (unitNumber == 3 && eventId == CF_RadioEvent.STUCK && variant == 1)
		{
			resourceName = "{BE4E13FA13FDF8A9}Sounds/LeaderGenerated/unit_3_stuck_c.wav";
			delayMs = 3247;
			return true;
		}
		if (unitNumber == 3 && eventId == CF_RadioEvent.LOST && variant == 0)
		{
			resourceName = "{F733EAC4BB0CA39B}Sounds/LeaderGenerated/unit_3_lost_b.wav";
			delayMs = 2709;
			return true;
		}
		if (unitNumber == 3 && eventId == CF_RadioEvent.LOST && variant == 1)
		{
			resourceName = "{A067AA3F3FEB9712}Sounds/LeaderGenerated/unit_3_lost_c.wav";
			delayMs = 3274;
			return true;
		}
		if (unitNumber == 3 && eventId == CF_RadioEvent.REJOINED && variant == 0)
		{
			resourceName = "{E232D8B86DA3AE9A}Sounds/LeaderGenerated/unit_3_rejoined_b.wav";
			delayMs = 2587;
			return true;
		}
		if (unitNumber == 3 && eventId == CF_RadioEvent.REJOINED && variant == 1)
		{
			resourceName = "{3C5F3F9C83BF1579}Sounds/LeaderGenerated/unit_3_rejoined_c.wav";
			delayMs = 2848;
			return true;
		}
		if (unitNumber == 3 && eventId == CF_RadioEvent.UNDER_FIRE && variant == 0)
		{
			resourceName = "{4A0BB799CDB20E14}Sounds/LeaderGenerated/unit_3_under_fire_b.wav";
			delayMs = 1982;
			return true;
		}
		if (unitNumber == 3 && eventId == CF_RadioEvent.UNDER_FIRE && variant == 1)
		{
			resourceName = "{AECAB3E842C06ED3}Sounds/LeaderGenerated/unit_3_under_fire_c.wav";
			delayMs = 2615;
			return true;
		}
		if (unitNumber == 4 && eventId == CF_RadioEvent.FAR_WARNING && variant == 0)
		{
			resourceName = "{D62E2E0E50FEF4BB}Sounds/LeaderGenerated/unit_4_far_warning_b.wav";
			delayMs = 2430;
			return true;
		}
		if (unitNumber == 4 && eventId == CF_RadioEvent.FAR_WARNING && variant == 1)
		{
			resourceName = "{E65D2AEC6C42DD05}Sounds/LeaderGenerated/unit_4_far_warning_c.wav";
			delayMs = 2253;
			return true;
		}
		if (unitNumber == 4 && eventId == CF_RadioEvent.STUCK && variant == 0)
		{
			resourceName = "{FC1A992AAE5A309A}Sounds/LeaderGenerated/unit_4_stuck_b.wav";
			delayMs = 3302;
			return true;
		}
		if (unitNumber == 4 && eventId == CF_RadioEvent.STUCK && variant == 1)
		{
			resourceName = "{327EF1E47F0DBEEE}Sounds/LeaderGenerated/unit_4_stuck_c.wav";
			delayMs = 3309;
			return true;
		}
		if (unitNumber == 4 && eventId == CF_RadioEvent.LOST && variant == 0)
		{
			resourceName = "{5F241C52034CCCD7}Sounds/LeaderGenerated/unit_4_lost_b.wav";
			delayMs = 2732;
			return true;
		}
		if (unitNumber == 4 && eventId == CF_RadioEvent.LOST && variant == 1)
		{
			resourceName = "{C85C366FF49FEAFF}Sounds/LeaderGenerated/unit_4_lost_c.wav";
			delayMs = 3222;
			return true;
		}
		if (unitNumber == 4 && eventId == CF_RadioEvent.REJOINED && variant == 0)
		{
			resourceName = "{0B0FAF400D721143}Sounds/LeaderGenerated/unit_4_rejoined_b.wav";
			delayMs = 2569;
			return true;
		}
		if (unitNumber == 4 && eventId == CF_RadioEvent.REJOINED && variant == 1)
		{
			resourceName = "{689EEE7B534CB066}Sounds/LeaderGenerated/unit_4_rejoined_c.wav";
			delayMs = 2910;
			return true;
		}
		if (unitNumber == 4 && eventId == CF_RadioEvent.UNDER_FIRE && variant == 0)
		{
			resourceName = "{33C8F7D4C95319FB}Sounds/LeaderGenerated/unit_4_under_fire_b.wav";
			delayMs = 1959;
			return true;
		}
		if (unitNumber == 4 && eventId == CF_RadioEvent.UNDER_FIRE && variant == 1)
		{
			resourceName = "{FD028E207BFA8264}Sounds/LeaderGenerated/unit_4_under_fire_c.wav";
			delayMs = 2571;
			return true;
		}
		if (unitNumber == 5 && eventId == CF_RadioEvent.FAR_WARNING && variant == 0)
		{
			resourceName = "{65AC326973F6BBE2}Sounds/LeaderGenerated/unit_5_far_warning_b.wav";
			delayMs = 2380;
			return true;
		}
		if (unitNumber == 5 && eventId == CF_RadioEvent.FAR_WARNING && variant == 1)
		{
			resourceName = "{36EB2D694CDF7999}Sounds/LeaderGenerated/unit_5_far_warning_c.wav";
			delayMs = 2308;
			return true;
		}
		if (unitNumber == 5 && eventId == CF_RadioEvent.STUCK && variant == 0)
		{
			resourceName = "{269060D2A1944A9D}Sounds/LeaderGenerated/unit_5_stuck_b.wav";
			delayMs = 3280;
			return true;
		}
		if (unitNumber == 5 && eventId == CF_RadioEvent.STUCK && variant == 1)
		{
			resourceName = "{FE22CC02CD027F1E}Sounds/LeaderGenerated/unit_5_stuck_c.wav";
			delayMs = 3314;
			return true;
		}
		if (unitNumber == 5 && eventId == CF_RadioEvent.LOST && variant == 0)
		{
			resourceName = "{D597E71236F30E09}Sounds/LeaderGenerated/unit_5_lost_b.wav";
			delayMs = 2654;
			return true;
		}
		if (unitNumber == 5 && eventId == CF_RadioEvent.LOST && variant == 1)
		{
			resourceName = "{8AF5A35AEF7EE590}Sounds/LeaderGenerated/unit_5_lost_c.wav";
			delayMs = 3283;
			return true;
		}
		if (unitNumber == 5 && eventId == CF_RadioEvent.REJOINED && variant == 0)
		{
			resourceName = "{30BCD172E72F5209}Sounds/LeaderGenerated/unit_5_rejoined_b.wav";
			delayMs = 2557;
			return true;
		}
		if (unitNumber == 5 && eventId == CF_RadioEvent.REJOINED && variant == 1)
		{
			resourceName = "{BE730731159AF1DF}Sounds/LeaderGenerated/unit_5_rejoined_c.wav";
			delayMs = 2839;
			return true;
		}
		if (unitNumber == 5 && eventId == CF_RadioEvent.UNDER_FIRE && variant == 0)
		{
			resourceName = "{2E6B45D98C2D66BC}Sounds/LeaderGenerated/unit_5_under_fire_b.wav";
			delayMs = 1896;
			return true;
		}
		if (unitNumber == 5 && eventId == CF_RadioEvent.UNDER_FIRE && variant == 1)
		{
			resourceName = "{7518ED987CC6424D}Sounds/LeaderGenerated/unit_5_under_fire_c.wav";
			delayMs = 2615;
			return true;
		}
		// END GENERATED RADIO MAPPING
		return false;
	}

	protected bool CF_GetRadioClip(int eventId, int unitNumber, out string resourceName, out int delayMs)
	{
		resourceName = string.Empty;
		delayMs = 250;
		// These GUIDs and measured durations come from docs/convoy-leader-assets.json.
		// Unit One reports the whole convoy when leading. Its numbered clips
		// remain available if a different truck leads the return formation.
		if (unitNumber == 0 && eventId == CF_RadioEvent.READY)
		{
			resourceName = "{584D811CFC5A1FDD}Sounds/Leader/leader_ready.wav";
			delayMs = 1365;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.FOLLOWING)
		{
			resourceName = "{615074549B357F41}Sounds/Leader/leader_following.wav";
			delayMs = 1535;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.HOLDING)
		{
			resourceName = "{BFE8F488BF57D2FE}Sounds/Leader/leader_holding.wav";
			delayMs = 1451;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.FAR_WARNING)
		{
			resourceName = "{6C3F645931625D60}Sounds/Leader/leader_far_warning.wav";
			delayMs = 2706;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.STUCK)
		{
			resourceName = "{53F4F4C000E11BAA}Sounds/Leader/leader_stuck.wav";
			delayMs = 1961;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.LOST)
		{
			resourceName = "{A8D5244856339D99}Sounds/Leader/leader_lost.wav";
			delayMs = 2416;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.REJOINED)
		{
			resourceName = "{DF524D8E854388F7}Sounds/Leader/leader_rejoined.wav";
			delayMs = 2170;
			return true;
		}
		if (unitNumber == 0 && eventId == CF_RadioEvent.UNDER_FIRE)
		{
			resourceName = "{DF48D538CB07B6C2}Sounds/Leader/leader_under_fire.wav";
			delayMs = 2000;
			return true;
		}
		if (unitNumber == 1 && eventId == CF_RadioEvent.FAR_WARNING)
		{
			resourceName = "{1E6C3A9EDDFD88BD}Sounds/Leader/unit_1_far_warning.wav";
			delayMs = 2552;
			return true;
		}
		if (unitNumber == 1 && eventId == CF_RadioEvent.STUCK)
		{
			resourceName = "{09C5008997BB4FF2}Sounds/Leader/unit_1_stuck.wav";
			delayMs = 2189;
			return true;
		}
		if (unitNumber == 1 && eventId == CF_RadioEvent.LOST)
		{
			resourceName = "{AF58E12E5E445735}Sounds/Leader/unit_1_lost.wav";
			delayMs = 2795;
			return true;
		}
		if (unitNumber == 1 && eventId == CF_RadioEvent.REJOINED)
		{
			resourceName = "{95379F5B61B6023C}Sounds/Leader/unit_1_rejoined.wav";
			delayMs = 2651;
			return true;
		}
		if (unitNumber == 1 && eventId == CF_RadioEvent.UNDER_FIRE)
		{
			resourceName = "{0FB34B4A7F6ADF56}Sounds/Leader/unit_1_under_fire.wav";
			delayMs = 2440;
			return true;
		}
		if (unitNumber == 2 && eventId == CF_RadioEvent.FAR_WARNING)
		{
			resourceName = "{EC6A7D7167B6D127}Sounds/Leader/unit_2_far_warning.wav";
			delayMs = 2550;
			return true;
		}
		if (unitNumber == 2 && eventId == CF_RadioEvent.STUCK)
		{
			resourceName = "{D5AAE39CB02A840E}Sounds/Leader/unit_2_stuck.wav";
			delayMs = 2187;
			return true;
		}
		if (unitNumber == 2 && eventId == CF_RadioEvent.LOST)
		{
			resourceName = "{26B87501B35FD114}Sounds/Leader/unit_2_lost.wav";
			delayMs = 2793;
			return true;
		}
		if (unitNumber == 2 && eventId == CF_RadioEvent.REJOINED)
		{
			resourceName = "{3F48D580FE1145FB}Sounds/Leader/unit_2_rejoined.wav";
			delayMs = 2649;
			return true;
		}
		if (unitNumber == 2 && eventId == CF_RadioEvent.UNDER_FIRE)
		{
			resourceName = "{25B8DD2733A4A3E6}Sounds/Leader/unit_2_under_fire.wav";
			delayMs = 2438;
			return true;
		}
		if (unitNumber == 3 && eventId == CF_RadioEvent.FAR_WARNING)
		{
			resourceName = "{ABAE7CFFF5AB960D}Sounds/Leader/unit_3_far_warning.wav";
			delayMs = 2627;
			return true;
		}
		if (unitNumber == 3 && eventId == CF_RadioEvent.STUCK)
		{
			resourceName = "{18639F97B8F456EA}Sounds/Leader/unit_3_stuck.wav";
			delayMs = 2263;
			return true;
		}
		if (unitNumber == 3 && eventId == CF_RadioEvent.LOST)
		{
			resourceName = "{9CF3C8ABFD9ECA77}Sounds/Leader/unit_3_lost.wav";
			delayMs = 2869;
			return true;
		}
		if (unitNumber == 3 && eventId == CF_RadioEvent.REJOINED)
		{
			resourceName = "{61EF184437F488F5}Sounds/Leader/unit_3_rejoined.wav";
			delayMs = 2725;
			return true;
		}
		if (unitNumber == 3 && eventId == CF_RadioEvent.UNDER_FIRE)
		{
			resourceName = "{A49DC8378A61A04B}Sounds/Leader/unit_3_under_fire.wav";
			delayMs = 2514;
			return true;
		}
		if (unitNumber == 4 && eventId == CF_RadioEvent.FAR_WARNING)
		{
			resourceName = "{9C2F4DB94D1EF81E}Sounds/Leader/unit_4_far_warning.wav";
			delayMs = 2678;
			return true;
		}
		if (unitNumber == 4 && eventId == CF_RadioEvent.STUCK)
		{
			resourceName = "{FC21525BFDF8CADF}Sounds/Leader/unit_4_stuck.wav";
			delayMs = 2314;
			return true;
		}
		if (unitNumber == 4 && eventId == CF_RadioEvent.LOST)
		{
			resourceName = "{2EF93990D9514490}Sounds/Leader/unit_4_lost.wav";
			delayMs = 2920;
			return true;
		}
		if (unitNumber == 4 && eventId == CF_RadioEvent.REJOINED)
		{
			resourceName = "{93E19D9ECC468E3D}Sounds/Leader/unit_4_rejoined.wav";
			delayMs = 2776;
			return true;
		}
		if (unitNumber == 4 && eventId == CF_RadioEvent.UNDER_FIRE)
		{
			resourceName = "{82B8122E58B8C438}Sounds/Leader/unit_4_under_fire.wav";
			delayMs = 2565;
			return true;
		}
		if (unitNumber == 5 && eventId == CF_RadioEvent.FAR_WARNING)
		{
			resourceName = "{FD3491FC9BD26655}Sounds/Leader/unit_5_far_warning.wav";
			delayMs = 2707;
			return true;
		}
		if (unitNumber == 5 && eventId == CF_RadioEvent.STUCK)
		{
			resourceName = "{990F7A4B057A6F47}Sounds/Leader/unit_5_stuck.wav";
			delayMs = 2344;
			return true;
		}
		if (unitNumber == 5 && eventId == CF_RadioEvent.LOST)
		{
			resourceName = "{83E6BFC4E86F6A86}Sounds/Leader/unit_5_lost.wav";
			delayMs = 2949;
			return true;
		}
		if (unitNumber == 5 && eventId == CF_RadioEvent.REJOINED)
		{
			resourceName = "{F0E8AB9D90B7127C}Sounds/Leader/unit_5_rejoined.wav";
			delayMs = 2805;
			return true;
		}
		if (unitNumber == 5 && eventId == CF_RadioEvent.UNDER_FIRE)
		{
			resourceName = "{887D8C182F8B21B4}Sounds/Leader/unit_5_under_fire.wav";
			delayMs = 2594;
			return true;
		}
		if (unitNumber == 6 && eventId == CF_RadioEvent.FAR_WARNING)
		{
			resourceName = "{8F4B0274D757D4E7}Sounds/Leader/unit_6_far_warning.wav";
			delayMs = 2861;
			return true;
		}
		if (unitNumber == 6 && eventId == CF_RadioEvent.STUCK)
		{
			resourceName = "{5178066D0D11CE22}Sounds/Leader/unit_6_stuck.wav";
			delayMs = 2497;
			return true;
		}
		if (unitNumber == 6 && eventId == CF_RadioEvent.LOST)
		{
			resourceName = "{4D79A302306533AD}Sounds/Leader/unit_6_lost.wav";
			delayMs = 3103;
			return true;
		}
		if (unitNumber == 6 && eventId == CF_RadioEvent.REJOINED)
		{
			resourceName = "{BEB266E3BF75542C}Sounds/Leader/unit_6_rejoined.wav";
			delayMs = 2959;
			return true;
		}
		if (unitNumber == 6 && eventId == CF_RadioEvent.UNDER_FIRE)
		{
			resourceName = "{5A32158F1FD6AC78}Sounds/Leader/unit_6_under_fire.wav";
			delayMs = 2748;
			return true;
		}
		if (unitNumber == 7 && eventId == CF_RadioEvent.FAR_WARNING)
		{
			resourceName = "{A58F6686E0A3E781}Sounds/Leader/unit_7_far_warning.wav";
			delayMs = 2642;
			return true;
		}
		if (unitNumber == 7 && eventId == CF_RadioEvent.STUCK)
		{
			resourceName = "{43F77ED30C6ECFB4}Sounds/Leader/unit_7_stuck.wav";
			delayMs = 2278;
			return true;
		}
		if (unitNumber == 7 && eventId == CF_RadioEvent.LOST)
		{
			resourceName = "{2318481006BDBA56}Sounds/Leader/unit_7_lost.wav";
			delayMs = 2884;
			return true;
		}
		if (unitNumber == 7 && eventId == CF_RadioEvent.REJOINED)
		{
			resourceName = "{9953648C21EDEE5C}Sounds/Leader/unit_7_rejoined.wav";
			delayMs = 2740;
			return true;
		}
		if (unitNumber == 7 && eventId == CF_RadioEvent.UNDER_FIRE)
		{
			resourceName = "{086660F046BE980D}Sounds/Leader/unit_7_under_fire.wav";
			delayMs = 2529;
			return true;
		}
		if (unitNumber == 8 && eventId == CF_RadioEvent.FAR_WARNING)
		{
			resourceName = "{7BC36F9BACD06E86}Sounds/Leader/unit_8_far_warning.wav";
			delayMs = 2769;
			return true;
		}
		if (unitNumber == 8 && eventId == CF_RadioEvent.STUCK)
		{
			resourceName = "{2E64C9BC4429A471}Sounds/Leader/unit_8_stuck.wav";
			delayMs = 2405;
			return true;
		}
		if (unitNumber == 8 && eventId == CF_RadioEvent.LOST)
		{
			resourceName = "{9110C3A7DEE7505D}Sounds/Leader/unit_8_lost.wav";
			delayMs = 3011;
			return true;
		}
		if (unitNumber == 8 && eventId == CF_RadioEvent.REJOINED)
		{
			resourceName = "{69FD3955E6403E03}Sounds/Leader/unit_8_rejoined.wav";
			delayMs = 2867;
			return true;
		}
		if (unitNumber == 8 && eventId == CF_RadioEvent.UNDER_FIRE)
		{
			resourceName = "{9193503E4FF87B80}Sounds/Leader/unit_8_under_fire.wav";
			delayMs = 2656;
			return true;
		}
		if (unitNumber == 9 && eventId == CF_RadioEvent.FAR_WARNING)
		{
			resourceName = "{C51C402B070AE172}Sounds/Leader/unit_9_far_warning.wav";
			delayMs = 2674;
			return true;
		}
		if (unitNumber == 9 && eventId == CF_RadioEvent.STUCK)
		{
			resourceName = "{380650FFDCF46D33}Sounds/Leader/unit_9_stuck.wav";
			delayMs = 2311;
			return true;
		}
		if (unitNumber == 9 && eventId == CF_RadioEvent.LOST)
		{
			resourceName = "{B9C467D6CF1394EE}Sounds/Leader/unit_9_lost.wav";
			delayMs = 2916;
			return true;
		}
		if (unitNumber == 9 && eventId == CF_RadioEvent.REJOINED)
		{
			resourceName = "{23F2F39380E71D9D}Sounds/Leader/unit_9_rejoined.wav";
			delayMs = 2772;
			return true;
		}
		if (unitNumber == 9 && eventId == CF_RadioEvent.UNDER_FIRE)
		{
			resourceName = "{B166932DBF80B8ED}Sounds/Leader/unit_9_under_fire.wav";
			delayMs = 2561;
			return true;
		}
		if (unitNumber == 10 && eventId == CF_RadioEvent.FAR_WARNING)
		{
			resourceName = "{4B5CED2AD9FB2E90}Sounds/Leader/unit_10_far_warning.wav";
			delayMs = 2587;
			return true;
		}
		if (unitNumber == 10 && eventId == CF_RadioEvent.STUCK)
		{
			resourceName = "{F7AEFD0E23DA7FD2}Sounds/Leader/unit_10_stuck.wav";
			delayMs = 2223;
			return true;
		}
		if (unitNumber == 10 && eventId == CF_RadioEvent.LOST)
		{
			resourceName = "{9E3F052E78A7D531}Sounds/Leader/unit_10_lost.wav";
			delayMs = 2829;
			return true;
		}
		if (unitNumber == 10 && eventId == CF_RadioEvent.REJOINED)
		{
			resourceName = "{C17274AC854735BA}Sounds/Leader/unit_10_rejoined.wav";
			delayMs = 2685;
			return true;
		}
		if (unitNumber == 10 && eventId == CF_RadioEvent.UNDER_FIRE)
		{
			resourceName = "{031858CB1B822A90}Sounds/Leader/unit_10_under_fire.wav";
			delayMs = 2474;
			return true;
		}
		return false;
	}
}
