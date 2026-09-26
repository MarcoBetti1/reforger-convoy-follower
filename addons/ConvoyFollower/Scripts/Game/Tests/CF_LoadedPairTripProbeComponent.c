class CF_LoadedPairTripProbeComponentClass : CF_OrdinaryMixedRoadProbeComponentClass
{
}

// Two ordinary recruits; one owner serially performs four real native actions.
// Baseline mixed driving/Wait evidence stays separate from fresh Resume evidence.
class CF_LoadedPairTripProbeComponent : CF_OrdinaryMixedRoadProbeComponent
{
	protected ref array<CF_LoadedPairCargoProbeComponent> m_PairCargo = {};
	protected ref array<SCR_ResourceContainer> m_PairContainers = {};
	protected ref array<float> m_PairHeldValues = {1800, 0, 0, 0};
	protected ref array<vector> m_LoadOrigins = {};
	protected bool m_bLoadSettled;
	protected bool m_bLoadSettleTracking;
	protected float m_fLoadSettleBeginMs;
	protected float m_fLoadSettleLastMs;
	protected ref array<vector> m_LoadSettlePrevious = {};
	protected ref array<float> m_LoadSettleTravel = {0, 0};
	protected ChimeraCharacter m_PairOwner;
	protected bool m_bPairBound;
	protected bool m_bPairComplete;
	protected bool m_bPairNonSpacingFailure;
	protected bool m_bPairEnding;
	protected bool m_bPairResultLogged;
	protected bool m_bPairPostFailure;
	protected int m_iPairPhase; // 0 load, 1 baseline, 2 owner seat, 3 Hold settling, 4 Hold, 5 unload, 6 return, 7 native seat, 8 Resume
	protected int m_iNextCargo = 1;
	protected int m_iActiveCargo;
	protected bool m_bActiveLoading;
	protected float m_fPairPhaseMs;
	protected float m_fPairCommandMs;
	protected float m_fPairLedgerLogMs;
	protected float m_fHoldBeginMs;
	protected float m_fHoldCompletedSeconds;
	protected float m_fHoldLastSampleMs;
	protected int m_iHoldSamples;
	protected int m_iHoldStable;
	protected bool m_bPairHoldMeasure;
	protected bool m_bPairHoldAccepted;
	protected bool m_bPairResumeAccepted;
	protected ref array<vector> m_HoldOrigins = {};
	protected ref array<float> m_HoldDrifts = {0, 0};
	protected ref array<float> m_HoldSpeeds = {0, 0};
	protected ref array<int> m_BaselineSequences = {0, 0};
	protected ref array<int> m_ResumeSequences = {0, 0};
	protected ref array<string> m_ResumeActivityKeys = {"", ""};
	protected ref array<string> m_ResumeWaypointKeys = {"", ""};
	protected ref array<ref CF_OriginalTailGuideEvidence> m_PairResumeEvidence = {};
	protected ref array<vector> m_ResumeStarts = {};
	protected bool m_bCommandParking;
	protected vector m_vTransferOrigin;
	protected int m_iSeatStep;
	protected vector m_vRestartGoal;
	protected vector m_vRestartAxis;
	protected vector m_vRestartLeadStart;
	protected vector m_vCommandPreviousLead;
	protected float m_fLeadRestartProgress;
	protected int m_iLeadRestartPowered;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!Replication.IsServer()) return;
		m_PairResumeEvidence.Insert(new CF_OriginalTailGuideEvidence());
		m_PairResumeEvidence.Insert(new CF_OriginalTailGuideEvidence());
		if (m_iExpectedTrucks != 2 || m_fPacedLeadSpeedKmh != 20 ||
			m_sPacedWorld != "Worlds/Tests/ConvoyFollower_Arland_LoadedSupplyTrip_2Trucks.ent")
			NoteFailure("fixture", "loaded_pair_contract_mismatch");
		Print("[ConvoyFollower] LOADED_PAIR_INIT: run_id=" + m_sPacedRun +
			" expected=2 load_each=100 destination_total=200 conserved_total=1800 baseline_hold_s=180 explicit_hold_s=30" +
			" resume_each_m=20 resume_each_powered=3 max_link_m=60 total_timeout_s=720 command_timeout_s=240" +
			" owner_staging=test_only ordinary_input=false source_count_writes=false follower_control_writes=false production_controller=true");
	}

	override protected CF_OriginalTailGuideEvidence MixedEvidence(int unit)
	{
		if (m_iPairPhase == 8) return m_PairResumeEvidence[unit - 1];
		return super.MixedEvidence(unit);
	}

	override protected void LogPacedSamples()
	{
		super.LogPacedSamples();
		if (m_iPairPhase != 1) return;
		for (int i = 0; i < m_MixedUnits.Count(); i++)
			if (m_MixedUnits[i].ActivitySequence > m_BaselineSequences[i]) m_BaselineSequences[i] = m_MixedUnits[i].ActivitySequence;
	}

	override protected void NoteFailure(string scope, string reason)
	{
		if (scope != "spacing" || reason.IndexOf("link_over_60m_unit_") != 0) m_bPairNonSpacingFailure = true;
		super.NoteFailure(scope, reason);
	}

	protected void PairFailure(string reason)
	{
		if (m_bPacedTerminal)
		{
			if (!m_bPairPostFailure) Print("[ConvoyFollower] LOADED_PAIR_POST_RESULT_FAILURE: run_id=" + m_sPacedRun + " reason=" + reason);
			m_bPairPostFailure = true;
			return;
		}
		NoteFailure("loaded_pair", reason);
		m_bPairEnding = true;
		EndPaced();
	}

	protected void CommandFailure(string reason) { PairFailure(reason); }

	protected bool BindPairCargo()
	{
		World world = GetGame().GetWorld();
		if (m_PairCargo.IsEmpty())
		{
			for (int unit = 1; unit <= 2; unit++)
			{
				Vehicle truck = Vehicle.Cast(world.FindEntityByName("CF_SmokeFollower" + unit));
				CF_LoadedPairCargoProbeComponent worker;
				if (truck) worker = CF_LoadedPairCargoProbeComponent.Cast(truck.FindComponent(CF_LoadedPairCargoProbeComponent));
				if (!worker || worker.PairUnit() != unit) { m_PairCargo.Clear(); return false; }
				m_PairCargo.Insert(worker);
			}
		}
		foreach (CF_LoadedPairCargoProbeComponent cargo : m_PairCargo)
		{
			if (cargo.PairFailed()) { PairFailure("cargo_worker_failed_during_binding"); return false; }
			if (!cargo.BindPair()) return false;
		}
		if (m_PairCargo[0].PairOwner() != m_PairCargo[1].PairOwner() ||
			m_PairCargo[0].PairSource() != m_PairCargo[1].PairSource() ||
			m_PairCargo[0].PairDestination() != m_PairCargo[1].PairDestination() ||
			m_PairCargo[0].PairCargo() == m_PairCargo[1].PairCargo())
		{ PairFailure("pair_binding_not_shared_owner_distinct_cargos"); return false; }
		m_PairOwner = m_PairCargo[0].PairOwner();
		m_PairContainers.Insert(m_PairCargo[0].PairSource());
		m_PairContainers.Insert(m_PairCargo[0].PairCargo());
		m_PairContainers.Insert(m_PairCargo[1].PairCargo());
		m_PairContainers.Insert(m_PairCargo[0].PairDestination());
		for (int a = 0; a < 4; a++)
			for (int b = a + 1; b < 4; b++)
				if (m_PairContainers[a] == m_PairContainers[b]) { PairFailure("aliased_original_containers"); return false; }
		// Initial authored origins may still be settling under native physics.
		m_bPairBound = true;
		Print("[ConvoyFollower] LOADED_PAIR_BIND: run_id=" + m_sPacedRun + " owner_id=" + EntityKey(m_PairOwner) +
			" source_id=" + EntityKey(m_PairContainers[0].GetOwner()) + " cargo1_id=" + EntityKey(m_PairContainers[1].GetOwner()) +
			" cargo2_id=" + EntityKey(m_PairContainers[2].GetOwner()) + " destination_id=" + EntityKey(m_PairContainers[3].GetOwner()));
		return CheckPairLedger(true);
	}

	// Same physical startup window as the v6 native probe, shared by both
	// original trucks before either native action is requested.
	protected void ObserveLoadSettling()
	{
		float now = GetGame().GetWorld().GetWorldTime();
		for (int i = 0; i < 2; i++)
			if (!(CommandSpeed(m_PairCargo[i].PairTruck()) <= 0.5))
			{ m_bLoadSettleTracking = false; return; }
		if (!m_bLoadSettleTracking)
		{
			m_bLoadSettleTracking = true;
			m_fLoadSettleBeginMs = now; m_fLoadSettleLastMs = now;
			m_LoadSettlePrevious.Clear();
			for (int unit = 0; unit < 2; unit++)
			{
				m_LoadSettlePrevious.Insert(m_PairCargo[unit].PairTruck().GetOrigin());
				m_LoadSettleTravel[unit] = 0;
			}
			return;
		}
		float elapsed = now - m_fLoadSettleLastMs;
		if (!(elapsed > 0 && elapsed <= 250)) { m_bLoadSettleTracking = false; return; }
		for (int index = 0; index < 2; index++)
		{
			vector origin = m_PairCargo[index].PairTruck().GetOrigin();
			float step = vector.Distance(origin, m_LoadSettlePrevious[index]);
			m_LoadSettleTravel[index] = m_LoadSettleTravel[index] + step;
			if (!(step >= 0 && step <= 0.05 && m_LoadSettleTravel[index] <= 0.05))
			{ m_bLoadSettleTracking = false; return; }
			m_LoadSettlePrevious[index] = origin;
		}
		m_fLoadSettleLastMs = now;
		if (now - m_fLoadSettleBeginMs < 3000) return;
		for (int pinned = 0; pinned < 2; pinned++)
			m_LoadOrigins.Insert(m_PairCargo[pinned].PairTruck().GetOrigin());
		m_bLoadSettled = true;
		Print("[ConvoyFollower] LOADED_PAIR_STARTUP_SETTLED: run_id=" + m_sPacedRun +
			" observed_s=" + (now - m_fLoadSettleBeginMs) / 1000 + " unit1_travel_m=" + m_LoadSettleTravel[0] +
			" unit2_travel_m=" + m_LoadSettleTravel[1] + " unit1_origin=" + m_LoadOrigins[0] + " unit2_origin=" + m_LoadOrigins[1] +
			" speed_limit_kmh=0.5 travel_limit_m=0.05 exact_original=true anchors_pinned=true controls_written=false native_actions_started=false");
	}

	protected bool ReadPairLedger(out float source, out float cargo1, out float cargo2, out float destination)
	{
		if (!m_bPairBound || m_PairCargo.Count() != 2 || m_PairContainers.Count() != 4) return false;
		float source2, destination2;
		for (int i = 0; i < 2; i++)
		{
			CF_LoadedPairCargoProbeComponent worker = m_PairCargo[i];
			Vehicle truck = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokeFollower" + (i + 1)));
			if (!truck || worker.PairTruck() != truck || truck.FindComponent(CF_LoadedPairCargoProbeComponent) != worker ||
				worker.PairOwner() != m_PairOwner || worker.PairSource() != m_PairContainers[0] ||
				worker.PairCargo() != m_PairContainers[i + 1] || worker.PairDestination() != m_PairContainers[3]) return false;
			if (i + 1 != m_iActiveCargo && (!worker.PairInactive() || worker.PairBusy())) return false;
		}
		return m_PairCargo[0].PairValues(source, cargo1, destination) && m_PairCargo[1].PairValues(source2, cargo2, destination2) &&
			Math.AbsFloat(source - source2) <= 0.001 && Math.AbsFloat(destination - destination2) <= 0.001;
	}

	protected bool CheckPairLedger(bool log = false)
	{
		float source, cargo1, cargo2, destination;
		if (!ReadPairLedger(source, cargo1, cargo2, destination)) { PairFailure("original_container_action_or_owner_binding_lost"); return false; }
		array<float> values = {source, cargo1, cargo2, destination};
		float amount;
		if (m_iActiveCargo > 0)
		{
			amount = values[m_iActiveCargo] - m_PairHeldValues[m_iActiveCargo];
			if (!m_bActiveLoading) amount = -amount;
		}
		bool valid = amount >= -0.001 && amount <= 100.001 && Math.AbsFloat(source + cargo1 + cargo2 + destination - 1800) <= 0.001;
		for (int i = 0; i < 4; i++)
		{
			float expected = m_PairHeldValues[i];
			if (m_iActiveCargo > 0)
			{
				if (m_bActiveLoading && i == 0) expected -= amount;
				if (!m_bActiveLoading && i == 3) expected += amount;
				if (i == m_iActiveCargo)
				{
					if (m_bActiveLoading) expected += amount;
					else expected -= amount;
				}
			}
			if (Math.AbsFloat(values[i] - expected) > 0.001) valid = false;
		}
		if (!valid) { PairFailure("four_container_delta_or_total_not_conserved"); return false; }
		float now = GetGame().GetWorld().GetWorldTime();
		if (log || now - m_fPairLedgerLogMs >= 1000)
		{
			m_fPairLedgerLogMs = now;
			Print("[ConvoyFollower] LOADED_PAIR_CARGO: run_id=" + m_sPacedRun + " seconds=" + PacedSeconds() + " phase=" + m_iPairPhase +
				" active_unit=" + m_iActiveCargo + " loading=" + m_bActiveLoading + " source=" + source + " cargo1=" + cargo1 + " cargo2=" + cargo2 +
				" destination=" + destination + " total=" + (source + cargo1 + cargo2 + destination) + " exact_original=true valid=true" +
				" source_id=" + EntityKey(m_PairContainers[0].GetOwner()) + " cargo1_id=" + EntityKey(m_PairContainers[1].GetOwner()) +
				" cargo2_id=" + EntityKey(m_PairContainers[2].GetOwner()) + " destination_id=" + EntityKey(m_PairContainers[3].GetOwner()));
		}
		return true;
	}

	protected bool BeginCargo(int unit, bool loading)
	{
		if (m_iActiveCargo != 0 || !CheckPairLedger(true)) return false;
		m_iActiveCargo = unit;
		m_bActiveLoading = loading;
		if (!m_PairCargo[unit - 1].BeginPairAction(loading)) { PairFailure("native_cargo_request_rejected_unit_" + unit); return false; }
		return true;
	}

	protected bool CommitCargo()
	{
		if (m_iActiveCargo == 0 || !CheckPairLedger(true)) return false;
		CF_LoadedPairCargoProbeComponent worker = m_PairCargo[m_iActiveCargo - 1];
		bool done = worker.PairDelivered();
		if (m_bActiveLoading) done = worker.PairLoaded();
		if (!done || worker.PairBusy() || !worker.PairInactive()) return false;
		float source, cargo1, cargo2, destination;
		if (!ReadPairLedger(source, cargo1, cargo2, destination)) { PairFailure("commit_original_container_lost"); return false; }
		float cargo = cargo1;
		if (m_iActiveCargo == 2) cargo = cargo2;
		float amount = cargo - m_PairHeldValues[m_iActiveCargo];
		if (!m_bActiveLoading) amount = -amount;
		if (Math.AbsFloat(amount - 100) > 0.001) { PairFailure("commit_not_exactly_100"); return false; }
		m_PairHeldValues[0] = source; m_PairHeldValues[1] = cargo1; m_PairHeldValues[2] = cargo2; m_PairHeldValues[3] = destination;
		Print("[ConvoyFollower] LOADED_PAIR_CARGO_COMMIT: run_id=" + m_sPacedRun + " unit=" + m_iActiveCargo + " loading=" + m_bActiveLoading + " amount=100 stable_s_min=3");
		m_iActiveCargo = 0;
		m_iNextCargo++;
		return CheckPairLedger(true);
	}

	protected CF_ConvoyFollowDriverControllerComponent PairDriver(int unit)
	{
		if (m_PacedTrucks.Count() != 3 || unit < 1 || unit > 2) return null;
		return CF_ConvoyFollowDriverControllerComponent.Cast(m_PacedTrucks[unit].Driver);
	}

	protected bool CommandIdentities()
	{
		if (!PacedWorldAlive() || m_PacedTrucks.Count() != 3 || m_iExpectedTrucks != 2 || m_Player != m_PairOwner || m_PacedOwner != m_PairOwner ||
			!EntityAlive(m_Lead) || !EntityAlive(m_Pilot) || m_PacedTrucks[0].Truck != m_Lead || m_PacedTrucks[0].Pilot != m_Pilot ||
			GetGame().GetWorld().FindEntityByName("CF_SmokeLead") != m_Lead || !GetGame().GetPlayerManager() ||
			GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iPacedOwnerId) != m_PairOwner ||
			!m_PacedSession || CF_ConvoySession.GetForPlayer(m_PairOwner) != m_PacedSession) return false;
		for (int unit = 1; unit <= 2; unit++)
		{
			CF_ConvoyFollowDriverControllerComponent driver = PairDriver(unit);
			if (!IdentityRetained(unit, true) || !driver || driver.Type().ToString() != "CF_ConvoyFollowDriverControllerComponent" ||
				!driver.CF_TestOrdinaryRoleReady(unit > 1) || driver.CF_HasEntityFallbackFailure() ||
				m_PacedTrucks[unit].Truck != m_PairCargo[unit - 1].PairTruck()) return false;
			if (unit == 2 && m_MixedUnits[1].ObservedEpoch > 0)
			{
				CF_OriginalTailGuideReadback route = new CF_OriginalTailGuideReadback();
				ReadMixedTail(driver, route);
				if (!route.HasHistory || route.Epoch != m_MixedUnits[1].ObservedEpoch || !route.Joined ||
					driver.CF_GetTrailRealPredecessor() != m_PacedTrucks[1].Truck) return false;
			}
		}
		return true;
	}

	protected bool PairHeld()
	{
		if (!CommandIdentities()) return false;
		for (int unit = 1; unit <= 2; unit++)
			if (!PairDriver(unit).CF_IsPanelHeld() || !PairDriver(unit).CF_HasPanelHoldRequest()) return false;
		return true;
	}

	protected void SetPairPhase(int phase)
	{
		m_iPairPhase = phase;
		m_fPairPhaseMs = GetGame().GetWorld().GetWorldTime();
		Print("[ConvoyFollower] LOADED_PAIR_PHASE: run_id=" + m_sPacedRun + " phase=" + phase + " seconds=" + PacedSeconds());
	}

	protected void MeasurePairHold()
	{
		if (!m_bPairHoldMeasure) return;
		if (!PairHeld()) { PairFailure("both_original_hold_intents_or_seats_lost"); return; }
		for (int unit = 1; unit <= 2; unit++)
		{
			float drift = vector.DistanceXZ(m_PacedTrucks[unit].Truck.GetOrigin(), m_HoldOrigins[unit - 1]);
			float speed = CommandSpeed(m_PacedTrucks[unit].Truck);
			if (drift > m_HoldDrifts[unit - 1]) m_HoldDrifts[unit - 1] = drift;
			if (speed > m_HoldSpeeds[unit - 1]) m_HoldSpeeds[unit - 1] = speed;
			if (drift > 2 || speed > 2) { PairFailure("explicit_hold_physical_limit_unit_" + unit); return; }
		}
	}

	protected void LogPairHold(float now)
	{
		if (!m_bPairHoldMeasure) return;
		if (now - m_fHoldLastSampleMs > 1500) { PairFailure("hold_observation_gap"); return; }
		m_fHoldLastSampleMs = now;
		m_iHoldSamples++;
		for (int unit = 1; unit <= 2; unit++)
			Print("[ConvoyFollower] LOADED_PAIR_HOLD_SAMPLE: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick + " seconds=" + PacedSeconds() +
				" unit=" + unit + " held_s=" + (now - m_fHoldBeginMs) / 1000 + " samples=" + m_iHoldSamples +
				" truck_id=" + EntityKey(m_PacedTrucks[unit].Truck) + " pilot_id=" + EntityKey(m_PacedTrucks[unit].Pilot) +
				" predecessor_id=" + EntityKey(m_PacedTrucks[unit - 1].Truck) + " origin=" + m_PacedTrucks[unit].Truck.GetOrigin() +
				" max_drift_m=" + m_HoldDrifts[unit - 1] + " max_speed_kmh=" + m_HoldSpeeds[unit - 1] + " held=true hold_requested=true identities=true");
	}

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		super.EOnPostFrame(owner, timeSlice);
		if (!PacedWorldAlive() || !m_bPairBound) return;
		if (!CheckPairLedger()) return;
		if (m_bPacedTerminal) return;
		if (m_iPairPhase == 0)
		{
			if (!m_bLoadSettled) { ObserveLoadSettling(); return; }
			for (int i = 0; i < 2; i++)
				if (CommandSpeed(m_PairCargo[i].PairTruck()) > 0.5 || vector.Distance(m_LoadOrigins[i], m_PairCargo[i].PairTruck().GetOrigin()) > 0.5)
				{ PairFailure("departure_loading_truck_moved_unit_" + (i + 1)); return; }
		}
		if (m_iPairPhase >= 2)
		{
			if (!CommandIdentities()) { PairFailure("command_original_identity_or_session_lost"); return; }
			if (m_bCommandParking)
			{
				if (!CanCommandPark() || vector.DistanceXZ(m_Lead.GetOrigin(), m_vTransferOrigin) > 2 || CommandSpeed(m_Lead) > 2)
				{ PairFailure("lead_moved_during_owner_setup"); return; }
				ApplyCommandParking();
			}
			MeasurePairHold();
			if (m_bPacedTerminal) return;
		}
		if (m_iActiveCargo > 0)
		{
			m_PairCargo[m_iActiveCargo - 1].TickPairAction(timeSlice);
			if (m_PairCargo[m_iActiveCargo - 1].PairFailed()) { PairFailure("native_cargo_failed_unit_" + m_iActiveCargo); return; }
			CheckPairLedger();
		}
	}

	override protected void Poll()
	{
		if (!PacedWorldAlive() || m_bPacedTerminal) return;
		float now = GetGame().GetWorld().GetWorldTime();
		if (PacedSeconds() > 720 || (m_iPairPhase == 0 && PacedSeconds() > 90) ||
			(m_iPairPhase >= 2 && (now - m_fPairCommandMs > 240000 || now - m_fPairPhaseMs > 90000)))
		{ PairFailure("pair_phase_or_total_timeout"); return; }
		if (m_bPairNonSpacingFailure) { m_bPairEnding = true; EndPaced(); return; }
		if (!m_bPairBound && !BindPairCargo()) return;
		if (m_bPacedTerminal || !CheckPairLedger()) return;
		if (m_iPairPhase == 0)
		{
			if (!m_bLoadSettled) return;
			if (m_iActiveCargo > 0) { CommitCargo(); return; }
			if (m_iNextCargo <= 2) { BeginCargo(m_iNextCargo, true); return; }
			if (!m_PairCargo[0].PairLoaded() || !m_PairCargo[1].PairLoaded() || m_PairHeldValues[0] != 1600 || m_PairHeldValues[1] != 100 || m_PairHeldValues[2] != 100 || m_PairHeldValues[3] != 0)
			{ PairFailure("both_native_loads_missing"); return; }
			SetPairPhase(1);
			Print("[ConvoyFollower] LOADED_PAIR_DEPARTURE: run_id=" + m_sPacedRun + " source=1600 cargo1=100 cargo2=100 destination=0 original_course_released=true");
		}
		if (m_iPairPhase == 1) { super.Poll(); return; }
		m_iPacedTick++;
		if (!CommandIdentities()) { PairFailure("command_original_identity_or_session_lost"); return; }
		MeasurePeaks();
		MeasurePairHold();
		if (m_bPacedTerminal) return;
		LogPairHold(now);
		if (m_bPacedTerminal) return;
		for (int observed = 1; observed <= 2; observed++)
			Print("[ConvoyFollower] LOADED_PAIR_COMMAND_STATE: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick + " phase=" + m_iPairPhase +
				" unit=" + observed + " truck_id=" + EntityKey(m_PacedTrucks[observed].Truck) + " predecessor_id=" + EntityKey(m_PacedTrucks[observed - 1].Truck) +
				" held=" + PairDriver(observed).CF_IsPanelHeld() + " hold_requested=" + PairDriver(observed).CF_HasPanelHoldRequest() +
				" owner_pilot=" + CommandSeat(m_PacedOwner, true) + " native_pilot=" + CommandSeat(m_Pilot, true) +
				" owner_id=" + EntityKey(m_PacedOwner) + " identities=true state=" + CF_ConvoySession.CF_GetOwnerPanelOrderState(m_PacedOwner));
		if (m_iPairPhase == 2)
		{
			if (now - m_fPairPhaseMs > 30000) { PairFailure("owner_seat_timeout"); return; }
			if (!PollCommandSeatTransfer(true)) return;
			m_bPairHoldAccepted = CF_ConvoySession.CF_PanelHold(m_PacedOwner);
			Print("[ConvoyFollower] LOADED_PAIR_COMMAND: command=Hold accepted=" + m_bPairHoldAccepted + " run_id=" + m_sPacedRun);
			if (!m_bPairHoldAccepted || !PairDriver(1).CF_HasPanelHoldRequest() || !PairDriver(2).CF_HasPanelHoldRequest())
			{ PairFailure("both_hold_requests_not_accepted"); return; }
			SetPairPhase(3);
		}
		else if (m_iPairPhase == 3)
		{
			if (PairHeld() && CommandSpeed(m_PacedTrucks[1].Truck) <= 2 && CommandSpeed(m_PacedTrucks[2].Truck) <= 2) m_iHoldStable++;
			else m_iHoldStable = 0;
			if (m_iHoldStable < 3) return;
			m_HoldOrigins.Insert(m_PacedTrucks[1].Truck.GetOrigin());
			m_HoldOrigins.Insert(m_PacedTrucks[2].Truck.GetOrigin());
			m_fHoldBeginMs = now; m_fHoldLastSampleMs = now;
			m_bPairHoldMeasure = true;
			SetPairPhase(4);
		}
		else if (m_iPairPhase == 4)
		{
			if (now - m_fHoldBeginMs < 30000 || m_iHoldSamples < 30) return;
			if (!CommandSeat(m_PacedOwner, true) || !ExitCommandSeat(m_PacedOwner)) { PairFailure("owner_exit_for_native_unload_failed"); return; }
			m_iNextCargo = 1;
			SetPairPhase(5);
		}
		else if (m_iPairPhase == 5)
		{
			if (!CommandOnFoot(m_PacedOwner)) return;
			if (m_iActiveCargo > 0) { CommitCargo(); return; }
			if (m_iNextCargo <= 2) { BeginCargo(m_iNextCargo, false); return; }
			if (!m_PairCargo[0].PairDelivered() || !m_PairCargo[1].PairDelivered() || m_PairHeldValues[0] != 1600 || m_PairHeldValues[1] != 0 || m_PairHeldValues[2] != 0 || m_PairHeldValues[3] != 200)
			{ PairFailure("both_native_deliveries_missing"); return; }
			if (!BoardCommandSeat(m_PacedOwner, true)) { PairFailure("owner_return_rejected"); return; }
			SetPairPhase(6);
		}
		else if (m_iPairPhase == 6)
		{
			if (now - m_fPairPhaseMs > 30000) { PairFailure("owner_return_timeout"); return; }
			if (!CommandSeat(m_PacedOwner, true)) return;
			m_fHoldCompletedSeconds = (now - m_fHoldBeginMs) / 1000;
			Print("[ConvoyFollower] LOADED_PAIR_HOLD_COMPLETE: run_id=" + m_sPacedRun + " held_s=" + m_fHoldCompletedSeconds + " samples=" + m_iHoldSamples + " both_held=true delivery=200");
			m_bPairResumeAccepted = CF_ConvoySession.CF_PanelResume(m_PacedOwner);
			Print("[ConvoyFollower] LOADED_PAIR_COMMAND: command=Resume accepted=" + m_bPairResumeAccepted + " run_id=" + m_sPacedRun + " completion_claim=false");
			if (!m_bPairResumeAccepted || PairDriver(1).CF_HasPanelHoldRequest() || PairDriver(2).CF_HasPanelHoldRequest())
			{ PairFailure("both_resume_requests_not_accepted"); return; }
			m_bPairHoldMeasure = false;
			SetPairPhase(7);
			BeginCommandSeatTransfer(false);
		}
		else if (m_iPairPhase == 7)
		{
			if (now - m_fPairPhaseMs > 30000) { PairFailure("native_pilot_return_timeout"); return; }
			if (!PollCommandSeatTransfer(false)) return;
			if (!StartPairRestart()) { PairFailure("native_lead_restart_failed"); return; }
			SetPairPhase(8);
		}
		else if (m_iPairPhase == 8)
			SamplePairRestart();
	}

	protected bool BaselineComplete()
	{
		if (!m_bPacedObserving || m_iObservationSamples < 180 ||
			GetGame().GetWorld().GetWorldTime() - m_fObservationStartMs < 180000 || !LeadWaitSelected() ||
			!CommandIdentities() || m_bPairNonSpacingFailure || m_bMixedFailed || m_bPacedDriftFailed) return false;
		for (int i = 0; i < 2; i++)
			if (m_MixedUnits[i].WaitSamples < 180 || m_MixedUnits[i].BestProgress < 100 || m_MixedUnits[i].BestPoweredSamples < 3 ||
				m_BaselineSequences[i] <= 0 || (i == 1 && !m_MixedUnits[i].SawJoined)) return false;
		return true;
	}

	override protected void EndPaced()
	{
		if (m_bPacedTerminal) return;
		if (!m_bPairEnding && m_iPairPhase == 1 && BaselineComplete())
		{
			Print("[ConvoyFollower] LOADED_PAIR_BASELINE_COMPLETE: run_id=" + m_sPacedRun + " observed_s=" +
				(GetGame().GetWorld().GetWorldTime() - m_fObservationStartMs) / 1000 + " samples=" + m_iObservationSamples +
				" unit1_wait=" + m_MixedUnits[0].WaitSamples + " unit2_wait=" + m_MixedUnits[1].WaitSamples +
				" spacing_failed=" + m_bPacedSpacingFailed + " provisional=true overall_pass_eligible=" + !m_bPacedFailed);
			m_bPacedObserving = false;
			// Hold keeps the existing tail history. Copy only continuity metadata;
			// every physical Resume counter and interval endpoint remains fresh.
			CF_OriginalTailGuideEvidence baselineTail = m_MixedUnits[1];
			CF_OriginalTailGuideEvidence resumedTail = m_PairResumeEvidence[1];
			resumedTail.AssignmentEstablished = baselineTail.AssignmentEstablished;
			resumedTail.ObservedEpoch = baselineTail.ObservedEpoch;
			resumedTail.SawJoined = baselineTail.SawJoined;
			resumedTail.LastRouteProgress = baselineTail.LastRouteProgress;
			resumedTail.LastRecordedEnd = baselineTail.LastRecordedEnd;
			m_fPairCommandMs = GetGame().GetWorld().GetWorldTime();
			if (!SurveyCommandExtension()) { PairFailure("restart_road_extension_unavailable"); return; }
			SetPairPhase(2);
			BeginCommandSeatTransfer(true);
			return;
		}
		m_bPairEnding = true;
		foreach (CF_LoadedPairCargoProbeComponent worker : m_PairCargo) worker.StopPairAction("terminal");
		if (!m_bPairComplete) NoteFailure("loaded_pair", "incomplete_native_delivery_hold_or_restart");
		ReleaseCommandParking("terminal");
		super.EndPaced();
		if (!m_bPairResultLogged)
		{
			m_bPairResultLogged = true;
			Print("[ConvoyFollower] LOADED_PAIR_RESULT: run_id=" + m_sPacedRun + " pass=" + (m_bPairComplete && !m_bPacedFailed) +
				" pair_complete=" + m_bPairComplete + " hold_accepted=" + m_bPairHoldAccepted + " resume_accepted=" + m_bPairResumeAccepted +
				" hold_s=" + m_fHoldCompletedSeconds + " hold_samples=" + m_iHoldSamples + " peak_gap_m=" + m_fPacedPeakGap +
				" spacing_failed=" + m_bPacedSpacingFailed + " ordinary_input=false owner_staging=test_only");
		}
	}

	protected bool StartPairRestart()
	{
		if (!CanControlLead() || !OwnerRetained() || !EntryReady(m_PacedTrucks[0]) || m_PilotWaypoint) return false;
		Resource prefab = Resource.Load("{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et");
		if (!prefab.IsValid()) return false;
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = m_vRestartGoal;
		SCR_AIWaypoint waypoint = SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(prefab, null, params));
		if (!waypoint) return false;
		if (!m_PacedCruise.Request(m_Pilot, m_Lead, m_fPacedLeadSpeedKmh, "loaded_pair_native_resume"))
		{ SCR_EntityHelper.DeleteEntityAndChildren(waypoint); return false; }
		waypoint.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_GAMEMASTER);
		waypoint.SetCompletionRadius(5);
		m_PilotWaypoint = waypoint;
		m_PilotGroup.AddWaypoint(waypoint);
		ReleaseCommandParking("native_pilot_reentered");
		m_vRestartLeadStart = m_Lead.GetOrigin();
		m_vCommandPreviousLead = m_vRestartLeadStart;
		m_ResumeStarts.Insert(m_PacedTrucks[1].Truck.GetOrigin());
		m_ResumeStarts.Insert(m_PacedTrucks[2].Truck.GetOrigin());
		Print("[ConvoyFollower] LOADED_PAIR_RESTART_BEGIN: run_id=" + m_sPacedRun + " lead=" + m_vRestartLeadStart + " goal=" + m_vRestartGoal +
			" unit1_baseline_sequence=" + m_BaselineSequences[0] + " unit2_baseline_sequence=" + m_BaselineSequences[1] + " evidence_reset=true native_lead=true");
		return true;
	}

	protected void SamplePairRestart()
	{
		if (!OwnerRetained() || !CanControlLead()) { PairFailure("resume_owner_or_native_lead_lost"); return; }
		bool both = true;
		for (int unit = 1; unit <= 2; unit++)
		{
			if (PairDriver(unit).CF_HasPanelHoldRequest()) { PairFailure("hold_reappeared_during_resume"); return; }
			CF_OriginalTailGuideEvidence evidence = m_PairResumeEvidence[unit - 1];
			ObserveMixedUnit(unit);
			if (m_bMixedFatal || m_bPairNonSpacingFailure) { PairFailure("resume_mixed_observer_failure"); return; }
			if (evidence.PreviousEligible)
			{
				if (evidence.ActivitySequence <= m_BaselineSequences[unit - 1]) { PairFailure("resume_not_fresh_unit_" + unit); return; }
				if (m_ResumeSequences[unit - 1] == 0)
				{
					m_ResumeSequences[unit - 1] = evidence.ActivitySequence;
					m_ResumeActivityKeys[unit - 1] = evidence.ActivityKey;
					m_ResumeWaypointKeys[unit - 1] = evidence.WaypointKey;
				}
				if (evidence.ActivitySequence != m_ResumeSequences[unit - 1] || evidence.ActivityKey != m_ResumeActivityKeys[unit - 1] ||
					evidence.WaypointKey != m_ResumeWaypointKeys[unit - 1]) { PairFailure("resume_activity_replaced_unit_" + unit); return; }
			}
			vector delta = m_PacedTrucks[unit].Truck.GetOrigin() - m_ResumeStarts[unit - 1];
			float net = delta[0] * m_vRestartAxis[0] + delta[2] * m_vRestartAxis[2];
			bool passed = evidence.PreviousEligible && evidence.ActivitySequence == m_ResumeSequences[unit - 1] &&
				evidence.Progress >= 20 && evidence.PoweredSamples >= 3 && net >= 20 && (unit == 1 || evidence.SawJoined);
			if (!passed) both = false;
			Print("[ConvoyFollower] LOADED_PAIR_RESUME_SAMPLE: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick + " seconds=" + PacedSeconds() +
				" unit=" + unit + " truck_id=" + EntityKey(m_PacedTrucks[unit].Truck) + " pilot_id=" + EntityKey(m_PacedTrucks[unit].Pilot) +
				" predecessor_id=" + EntityKey(m_PacedTrucks[unit - 1].Truck) + " origin=" + m_PacedTrucks[unit].Truck.GetOrigin() +
				" baseline_sequence=" + m_BaselineSequences[unit - 1] + " fresh_sequence=" + evidence.ActivitySequence +
				" activity=" + evidence.ActivityKey + " waypoint=" + evidence.WaypointKey + " native_target=" + evidence.NativeTargetKey +
				" epoch=" + evidence.RouteEpoch + " eligible=" + evidence.PreviousEligible + " exact_progress_m=" + evidence.Progress +
				" net_progress_m=" + net + " powered=" + evidence.PoweredSamples + " joined=" + evidence.SawJoined + " gate=" + passed);
		}
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		VehicleWheeledSimulation sim;
		if (car) sim = car.GetSimulation();
		vector position = m_Lead.GetOrigin();
		vector change = position - m_vCommandPreviousLead;
		vector overall = position - m_vRestartLeadStart;
		float step = change[0] * m_vRestartAxis[0] + change[2] * m_vRestartAxis[2];
		if (!sim || vector.DistanceXZ(position, m_vCommandPreviousLead) > 20) { PairFailure("resume_lead_simulation_or_discontinuity"); return; }
		m_fLeadRestartProgress = overall[0] * m_vRestartAxis[0] + overall[2] * m_vRestartAxis[2];
		bool powered = sim.EngineIsOn() && sim.GetThrottle() > 0.05 && sim.GetGear() >= 2 && sim.GetSpeedKmh() >= 1.5 && step >= 0.25;
		if (powered) m_iLeadRestartPowered++;
		m_vCommandPreviousLead = position;
		Print("[ConvoyFollower] LOADED_PAIR_RESUME_LEAD: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick + " origin=" + position +
			" progress_m=" + m_fLeadRestartProgress + " powered=" + powered + " powered_samples=" + m_iLeadRestartPowered + " speed_kmh=" + sim.GetSpeedKmh() +
			" engine=" + sim.EngineIsOn() + " throttle=" + sim.GetThrottle() + " gear=" + sim.GetGear() + " signed_step_m=" + step);
		if (both && m_fLeadRestartProgress >= 20 && m_iLeadRestartPowered >= 3 && CheckPairLedger(true))
		{
			m_bPairComplete = true;
			m_bPairEnding = true;
			EndPaced();
			return;
		}
		if (!m_bPacedHoldingLead && vector.DistanceXZ(position, m_vRestartGoal) <= 8) HoldLeadAtRoadGoal();
	}

	// Proven lead-only parking and native seat operations are copied verbatim
	// below from CF_OriginalFollowResumeProbeComponent; the survey walks 70 m
	// to give the second follower room to establish its fresh 20 m segment.
	protected float CommandSpeed(Vehicle truck)
	{
		CarControllerComponent car;
		if (truck) car = CarControllerComponent.Cast(truck.FindComponent(CarControllerComponent));
		if (!car || !car.GetSimulation()) return 1000000;
		return Math.AbsFloat(car.GetSimulation().GetSpeedKmh());
	}

	protected bool CommandSeat(ChimeraCharacter character, bool pilot)
	{
		if (!character) return false;
		CompartmentAccessComponent access = character.GetCompartmentAccessComponent();
		if (!access || access.IsGettingIn() || access.IsGettingOut()) return false;
		BaseCompartmentSlot slot = access.GetCompartment();
		return slot && slot.GetOccupant() == character && slot.IsPiloting() == pilot && access.GetVehicleIn(character) == m_Lead;
	}

	protected bool CommandOnFoot(ChimeraCharacter character)
	{
		if (!character || character.IsInVehicle()) return false;
		CompartmentAccessComponent access = character.GetCompartmentAccessComponent();
		return access && !access.IsGettingIn() && !access.IsGettingOut();
	}

	protected bool SurveyCommandExtension()
	{
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager()) return false;
		RoadNetworkManager roads = aiWorld.GetRoadNetworkManager();
		BaseRoad road;
		float roadDistance;
		int roadId = roads.GetClosestRoad(m_Lead.GetOrigin(), road, roadDistance);
		if (roadId < 0 || !road || road.GetWidth() < 6 || roadDistance > 5) return false;
		array<vector> points = {};
		road.GetPoints(points);
		vector forward = m_Lead.GetWorldTransformAxis(2);
		vector candidate;
		if (!CF_RoadPolyline.TryWalk(points, m_Lead.GetOrigin(), forward, 70, candidate)) return false;
		vector reached;
		if (!roads.GetReachableWaypointInRoad(m_Lead.GetOrigin(), candidate, 5, reached) || vector.DistanceXZ(candidate, reached) > 3) return false;
		vector delta = reached - m_Lead.GetOrigin();
		float distance = vector.DistanceXZ(reached, m_Lead.GetOrigin());
		if (distance < 55 || distance > 85 || delta[0] * forward[0] + delta[2] * forward[2] < distance * 0.5) return false;
		m_vRestartGoal = reached;
		m_vRestartAxis = Vector(delta[0] / distance, 0, delta[2] / distance);
		Print("[ConvoyFollower] ORIGINAL_RESUME_SURVEY: run_id=" + m_sPacedRun + " road_id=" + roadId +
			" width=" + road.GetWidth() + " road_distance=" + roadDistance + " start=" + m_Lead.GetOrigin() +
			" goal=" + reached + " axis=" + m_vRestartAxis + " recorded_road_walk_m=70 geometry_only=true");
		return true;
	}

	protected bool CanCommandPark()
	{
		if (!CommandIdentities()) return false;
		PlayerManager players = GetGame().GetPlayerManager();
		if (!players || players.GetPlayerIdFromControlledEntity(m_Pilot) > 0 || players.GetPlayerIdFromControlledEntity(m_Lead) > 0)
			return false;
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (!car || !car.GetSimulation() || !car.GetPilotCompartmentSlot()) return false;
		IEntity pilot = car.GetPilotCompartmentSlot().GetOccupant();
		return !pilot || pilot == m_Pilot || pilot == m_PacedOwner;
	}

	protected void ApplyCommandParking()
	{
		if (!m_bCommandParking || !CanCommandPark()) return;
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		car.SetPersistentHandBrake(true);
		car.GetSimulation().SetThrottle(0);
		car.GetSimulation().SetBreak(1, true);
	}

	protected void ReleaseCommandParking(string reason)
	{
		if (!m_bCommandParking) return;
		bool reset = CanCommandPark();
		m_bCommandParking = false;
		if (reset)
		{
			CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
			car.SetPersistentHandBrake(false);
			car.GetSimulation().SetBreak(0, true);
		}
		Print("[ConvoyFollower] ORIGINAL_RESUME_PARK_RELEASE: run_id=" + m_sPacedRun + " reason=" + reason + " native_reset=" + reset);
	}

	protected bool ExitCommandSeat(ChimeraCharacter character)
	{
		CompartmentAccessComponent access = character.GetCompartmentAccessComponent();
		return access && access.GetVehicleIn(character) == m_Lead &&
			access.GetOutVehicle(EGetOutType.TELEPORT, -1, ECloseDoorAfterActions.CLOSE_DOOR, true, true);
	}

	protected bool BoardCommandSeat(ChimeraCharacter character, bool pilot)
	{
		if (!CommandOnFoot(character)) return false;
		BaseCompartmentManagerComponent manager = BaseCompartmentManagerComponent.Cast(m_Lead.FindComponent(BaseCompartmentManagerComponent));
		if (!manager) return false;
		array<BaseCompartmentSlot> slots = {};
		manager.GetCompartments(slots);
		foreach (BaseCompartmentSlot slot : slots)
		{
			if (slot && slot.IsPiloting() == pilot && !slot.IsOccupied() && !slot.IsReserved())
				return character.GetCompartmentAccessComponent().GetInVehicle(m_Lead, slot, true, -1, ECloseDoorAfterActions.CLOSE_DOOR, true);
		}
		return false;
	}

	protected void BeginCommandSeatTransfer(bool toOwner)
	{
		if (!CanCommandPark() || CommandSpeed(m_Lead) > 2 ||
			(toOwner && (!CanControlLead() || !LeadWaitSelected())) ||
			(!toOwner && (!CommandSeat(m_PacedOwner, true) || !CommandOnFoot(m_Pilot))))
		{
			CommandFailure("seat_transfer_precondition");
			return;
		}
		m_vTransferOrigin = m_Lead.GetOrigin();
		m_bCommandParking = true;
		ApplyCommandParking();
		if (toOwner)
		{
			m_PacedWait.Complete(); // Exact unparented lead action, before its pilot leaves.
			m_PacedWait = null;
			m_PacedCruise.Release("original_resume_owner_seat_setup");
			m_bPacedHoldingLead = false;
			m_bPacedOwnBrake = false;
		}
		ChimeraCharacter exiting = m_PacedOwner;
		if (toOwner) exiting = m_Pilot;
		if (!ExitCommandSeat(exiting))
		{
			CommandFailure("seat_exit_rejected");
			return;
		}
		m_iSeatStep = 1;
		ApplyCommandParking();
		Print("[ConvoyFollower] ORIGINAL_RESUME_PARK_ACQUIRE: run_id=" + m_sPacedRun +
			" to_owner=" + toOwner + " origin=" + m_vTransferOrigin + " atomic_release_exit=true follower_writes=false human_input_claim=false");
	}

	protected bool PollCommandSeatTransfer(bool toOwner)
	{
		if (toOwner)
		{
			if (m_iSeatStep == 1 && CommandOnFoot(m_Pilot))
			{
				if (!ExitCommandSeat(m_PacedOwner)) { CommandFailure("owner_exit_rejected"); return false; }
				m_iSeatStep = 2;
			}
			else if (m_iSeatStep == 2 && CommandOnFoot(m_PacedOwner))
			{
				if (!BoardCommandSeat(m_PacedOwner, true)) { CommandFailure("owner_pilot_entry_rejected"); return false; }
				m_iSeatStep = 3;
			}
			return m_iSeatStep == 3 && CommandSeat(m_PacedOwner, true) && CommandOnFoot(m_Pilot);
		}
		if (m_iSeatStep == 1 && CommandOnFoot(m_PacedOwner))
		{
			if (!BoardCommandSeat(m_PacedOwner, false)) { CommandFailure("owner_passenger_entry_rejected"); return false; }
			m_iSeatStep = 2;
		}
		else if (m_iSeatStep == 2 && CommandSeat(m_PacedOwner, false))
		{
			if (!BoardCommandSeat(m_Pilot, true)) { CommandFailure("native_pilot_entry_rejected"); return false; }
			m_iSeatStep = 3;
		}
		return m_iSeatStep == 3 && CanControlLead() && OwnerRetained() && EntryReady(m_PacedTrucks[0]);
	}


	override void OnDelete(IEntity owner)
	{
		foreach (CF_LoadedPairCargoProbeComponent worker : m_PairCargo)
			if (worker) worker.StopPairAction("delete");
		ReleaseCommandParking("delete");
		super.OnDelete(owner);
	}
}
