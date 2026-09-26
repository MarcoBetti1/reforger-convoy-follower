// Separate status regression. The parent supplies the existing driven Hold /
// Resume precondition; its first PASS starts this course instead of ending it.
// Only lead occupants are transferred for fixture setup. No human-input claim.
class CF_ResumeStatusProbeComponentClass : CF_ExplicitHoldResumeProbeComponentClass
{
}

class CF_ResumeStatusProbeComponent : CF_ExplicitHoldResumeProbeComponent
{
	// 0 parent course, 1 owner seat, 2 explicit Hold, 3 native seat,
	// 4 stationary wait, 5 powered restart, 6 settle, 7 post-result observe.
	protected int m_iResumeStage;
	protected int m_iResumeStageStart;
	protected float m_fResumeAcceptedMs;
	protected float m_fStationaryStartMs;
	protected float m_fResumeTerminalMs;
	protected bool m_bResumePoseMonitor;
	protected bool m_bResumeTerminal;
	protected bool m_bAuthoritativeCompleted;
	protected vector m_vResumeStatusGoal;
	protected vector m_vResumeLeadPose;
	protected vector m_vResumeFollowerPose;
	protected float m_fResumeLeadDrift;
	protected float m_fResumeFollowerDrift;
	protected float m_fWaitLeadDrift;
	protected float m_fWaitFollowerDrift;
	protected float m_fResumePeakGap;
	protected string m_sLastResumeStatus;
	protected bool m_bResumeSeatStaging;
	protected bool m_bResumeParking;
	protected bool m_bResumeTransferPoseMonitor;
	protected vector m_vResumeTransferPose;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (Replication.IsServer())
			Print("[ConvoyFollower] RESUME_STATUS_INIT: expected=1 timeout_s=780 stationary_s=100 postterminal_s=30 max_drift_m=2 native_lead=true production_follower=true test_seat_transfers=true human_input_claim=false scope=resume_status");
	}

	override protected bool SurveyFinalResumeExtension()
	{
		if (!super.SurveyFinalResumeExtension())
			return false;
		vector savedStart = m_vRouteStart;
		m_vRouteStart = m_vFinalResumeGoal;
		float clearRun;
		// The first live survey found a fence near x1817 on the 40 m leg.
		// Its first 30 m were clear. Keep the 20 m lead / 15 m follower
		// physical gates; this changes only the fixture's last surveyed goal.
		bool clear = SurveyCandidate(m_vRouteAxis, 30.0, 903, clearRun, 30.0);
		m_vRouteStart = savedStart;
		if (!clear)
			return false;
		m_vResumeStatusGoal = m_vFinalResumeGoal + m_vRouteAxis * 30.0;
		m_vResumeStatusGoal[1] = GetGame().GetWorld().GetSurfaceY(m_vResumeStatusGoal[0], m_vResumeStatusGoal[2]);
		Print("[ConvoyFollower] RESUME_STATUS_SURVEY: start=" + m_vFinalResumeGoal +
			" goal=" + m_vResumeStatusGoal + " geometry_only=true");
		return true;
	}

	protected void ResumeStage(int stage)
	{
		m_iResumeStage = stage;
		m_iResumeStageStart = m_iTicks;
		SetPhase(20 + stage);
		Print("[ConvoyFollower] RESUME_STATUS_PHASE: seconds=" + m_iTicks + " stage=" + stage);
	}

	protected void CaptureResumePose()
	{
		m_vResumeLeadPose = m_Lead.GetOrigin();
		m_vResumeFollowerPose = m_Follower.GetOrigin();
		m_fResumeLeadDrift = 0;
		m_fResumeFollowerDrift = 0;
		m_bResumePoseMonitor = true;
	}

	protected bool CanParkResumeLead()
	{
		if (!m_Lead || m_Lead == m_Follower || !GetGame() || !GetGame().GetWorld() ||
			CF_ConvoySession.CF_IsWorldCleanup() ||
			GetGame().GetWorld().FindEntityByName("CF_SmokeLead") != m_Lead)
			return false;
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (!car || !car.GetSimulation() || !car.GetPilotCompartmentSlot())
			return false;
		IEntity pilot = car.GetPilotCompartmentSlot().GetOccupant();
		return !pilot || pilot == m_Pilot || pilot == m_Player;
	}

	protected void ReleaseResumeParking(string reason, bool nativeHoldRequested = false)
	{
		if (!m_bResumeParking)
			return;
		m_bResumeParking = false;
		bool reset = !nativeHoldRequested && CanParkResumeLead();
		if (reset)
		{
			CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
			car.SetPersistentHandBrake(false);
			car.GetSimulation().SetBreak(0, true);
		}
		bool waitSelected = m_LeadWaitBehavior && m_PilotUtility &&
			m_PilotUtility.GetCurrentBehavior() == m_LeadWaitBehavior;
		Print("[ConvoyFollower] RESUME_STATUS_PARK_RELEASE: seconds=" + m_iTicks +
			" reason=" + reason + " native_hold_requested=" + nativeHoldRequested +
			" wait_selected=" + waitSelected + " native_reset=" + reset);
	}

	// The parent releases native Wait one poll before ejecting the pilot.
	// This course keeps it until a settled handoff, then releases and exits in
	// the same call. Only the named fixture lead is parked across empty seats.
	override protected void BeginSeatTransfer(bool toOwner)
	{
		if (m_iResumeStage == 0)
		{
			super.BeginSeatTransfer(toOwner);
			return;
		}
		if (!CanParkResumeLead() || (toOwner && !NativeLeadStopped()) ||
			(!toOwner && (!IsFixtureSeat(m_Player, true) || !FixtureOnFoot(m_Pilot))))
		{
			FailResumeStatus("seat_transfer_ownership_or_settling_precondition", true);
			return;
		}
		m_bResumeParking = true;
		m_vResumeTransferPose = m_Lead.GetOrigin();
		m_bResumeTransferPoseMonitor = true;
		ApplyLeadBrake();
		Print("[ConvoyFollower] RESUME_STATUS_PARK_ACQUIRE: seconds=" + m_iTicks +
			" to_owner_pilot=" + toOwner + " lead=" + m_vResumeTransferPose + " follower_writes=false");
		m_bTransferToOwner = toOwner;
		m_iTransferStep = 0;
		ReleaseLeadWait();
		ResetLeadCruiseOverride();
		m_bHoldLead = false;
		Print("[ConvoyFollower] EXPLICIT_HOLD_SEAT_SETUP: seconds=" + m_iTicks +
			" to_owner_pilot=" + toOwner + " vehicle=" + m_Lead.GetName() +
			" temporary_test_setup=true human_input_claim=false atomic_release_exit=true");
		ChimeraCharacter exiting = m_Player;
		if (toOwner) exiting = m_Pilot;
		if (ExitFixtureLead(exiting))
		{
			m_iTransferStep = 1;
			ApplyLeadBrake();
		}
	}

	override protected bool MoveNativeLead(vector goal)
	{
		ReleaseResumeParking("powered_leg");
		m_bResumeTransferPoseMonitor = false;
		return super.MoveNativeLead(goal);
	}

	override void OnDelete(IEntity owner)
	{
		ReleaseResumeParking("deleted");
		super.OnDelete(owner);
	}

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (m_bResumeParking && !m_bFinished && !CF_ConvoySession.CF_IsWorldCleanup())
		{
			if (!CanParkResumeLead())
				FailResumeStatus("seat_transfer_lead_ownership_lost", true);
			else
				ApplyLeadBrake();
		}
		super.EOnPostFrame(owner, timeSlice);
		if (m_bFinished || CF_ConvoySession.CF_IsWorldCleanup() || !m_Lead || !m_Follower)
			return;
		float gap = vector.DistanceXZ(m_Lead.GetOrigin(), m_Follower.GetOrigin());
		if (gap > m_fResumePeakGap)
			m_fResumePeakGap = gap;
		if (m_bResumeTransferPoseMonitor &&
			(vector.DistanceXZ(m_Lead.GetOrigin(), m_vResumeTransferPose) > 2.0 || Speed(m_Lead) > 2.0))
		{
			FailResumeStatus("lead_moved_during_test_seat_transfer", true);
			return;
		}
		if (!m_bResumePoseMonitor)
			return;
		float leadDrift = vector.DistanceXZ(m_Lead.GetOrigin(), m_vResumeLeadPose);
		float followerDrift = vector.DistanceXZ(m_Follower.GetOrigin(), m_vResumeFollowerPose);
		if (leadDrift > m_fResumeLeadDrift) m_fResumeLeadDrift = leadDrift;
		if (followerDrift > m_fResumeFollowerDrift) m_fResumeFollowerDrift = followerDrift;
		if (leadDrift > 2.0 || Speed(m_Lead) > 2.0)
			FailResumeStatus("fixture_lead_moved_during_stationary_window", true);
		else if (followerDrift > 2.0 || Speed(m_Follower) > 2.0)
			FailResumeStatus("follower_moved_during_stationary_window");
	}

	protected void FailResumeStatus(string reason, bool fixtureFailure = false)
	{
		Print("[ConvoyFollower] RESUME_STATUS_FAILURE: seconds=" + m_iTicks +
			" stage=" + m_iResumeStage + " fixture_failure=" + fixtureFailure +
			" postterminal=" + m_bResumeTerminal + " reason=" + reason);
		Finish("FAIL " + reason);
	}

	override protected void Finish(string result)
	{
		if (m_bFinished)
			return;
		if (m_iResumeStage == 0 && result.IndexOf("PASS ") == 0)
		{
			Print("[ConvoyFollower] RESUME_STATUS_WARMUP: " + result + " terminal=false");
			if (!NativeLeadReady() || !RetainedFollowerAndOwner() ||
				vector.DistanceXZ(m_Lead.GetOrigin(), m_Follower.GetOrigin()) > CF_ConvoySettings.Get().m_fStoppedGap + 4.0)
			{
				FailResumeStatus("close_stationary_spacing_precondition", true);
				return;
			}
			ResumeStage(1);
			m_bResumeSeatStaging = true;
			m_vResumeTransferPose = m_Lead.GetOrigin();
			m_bResumeTransferPoseMonitor = true;
			Print("[ConvoyFollower] RESUME_STATUS_SEAT_STAGING: seconds=" + m_iTicks +
				" lead=" + m_vResumeTransferPose + " native_wait_retained=true required_stable_samples=3");
			return;
		}
		ReleaseResumeParking("finished");
		m_bResumeTransferPoseMonitor = false;
		m_bResumeSeatStaging = false;
		m_bResumePoseMonitor = false;
		Print("[ConvoyFollower] RESUME_STATUS_FINAL: seconds=" + m_iTicks + " stage=" + m_iResumeStage +
			" wait_lead_drift_m=" + m_fWaitLeadDrift + " wait_follower_drift_m=" + m_fWaitFollowerDrift +
			" observation_lead_drift_m=" + m_fResumeLeadDrift + " observation_follower_drift_m=" + m_fResumeFollowerDrift +
			" peak_gap_m=" + m_fResumePeakGap + " lead_progress_m=" + m_fLeadProgress +
			" follower_progress_m=" + m_fFollowerProgress + " authoritative_completed=" + m_bAuthoritativeCompleted);
		Print("[ConvoyFollower] RESUME_STATUS_RESULT: " + result);
		if (m_bResumeTerminal)
			Print("[ConvoyFollower] RESUME_STATUS_OBSERVATION_COMPLETE: seconds=" + m_iTicks +
				" observed_s=" + ((GetGame().GetWorld().GetWorldTime() - m_fResumeTerminalMs) / 1000.0) + " failed=true");
		super.Finish(result);
	}

	protected string ReadResumeStatus(string reason)
	{
		string state = CF_ConvoySession.CF_GetOwnerPanelOrderState(m_Player);
		if (state != m_sLastResumeStatus || reason != "moving")
		{
			m_sLastResumeStatus = state;
			Print("[ConvoyFollower] RESUME_STATUS_QUERY: seconds=" + m_iTicks +
				" reason=" + reason + " state='" + state + "'");
		}
		return state;
	}

	protected void LogStationarySample(string phase)
	{
		Print("[ConvoyFollower] RESUME_STATUS_POSITION: seconds=" + m_iTicks + " phase=" + phase +
			" lead=" + m_Lead.GetOrigin() + " follower=" + m_Follower.GetOrigin() +
			" lead_speed_kmh=" + Speed(m_Lead) + " follower_speed_kmh=" + Speed(m_Follower) +
			" lead_drift_m=" + m_fResumeLeadDrift + " follower_drift_m=" + m_fResumeFollowerDrift +
			" same_driver_truck_roster=true lead_native_wait=" + (m_LeadWaitBehavior && m_PilotUtility.GetCurrentBehavior() == m_LeadWaitBehavior));
	}

	override protected void Poll()
	{
		if (m_bFinished || CF_ConvoySession.CF_IsWorldCleanup())
			return;
		if (m_iResumeStage == 0)
		{
			super.Poll();
			return;
		}
		m_iTicks++;
		int stageSeconds = m_iTicks - m_iResumeStageStart;
		int stageLimit = 90;
		if (m_iResumeStage == 1 || m_iResumeStage == 3) stageLimit = 30;
		if (m_iResumeStage == 4) stageLimit = 115;
		if (m_iResumeStage == 7) stageLimit = 40;
		if (m_iTicks > 780 || stageSeconds > stageLimit)
		{
			FailResumeStatus("stage_or_total_timeout", m_iResumeStage == 1 || m_iResumeStage == 3);
			return;
		}
		if (!RetainedFollowerAndOwner())
		{
			FailResumeStatus("original_driver_seat_assignment_owner_or_roster_lost");
			return;
		}
		if (m_iResumeStage == 1 || m_iResumeStage == 3)
		{
			if (m_bResumeSeatStaging)
			{
				if (!NativeLeadReady() || !m_LeadWaitBehavior || !m_PilotUtility ||
					m_PilotUtility.GetCurrentBehavior() != m_LeadWaitBehavior)
				{
					FailResumeStatus("native_wait_lost_before_seat_transfer", true);
					return;
				}
				if (NativeLeadStopped() && Speed(m_Lead) <= 1.0 && Speed(m_Follower) <= 1.0 &&
					vector.DistanceXZ(m_Follower.GetOrigin(), m_vPreviousFollower) <= 0.3)
					m_iStableSamples++;
				else m_iStableSamples = 0;
				m_vPreviousLead = m_Lead.GetOrigin();
				m_vPreviousFollower = m_Follower.GetOrigin();
				Print("[ConvoyFollower] RESUME_STATUS_SEAT_SETTLE: seconds=" + m_iTicks +
					" stable_samples=" + m_iStableSamples + " lead=" + m_vPreviousLead +
					" follower=" + m_vPreviousFollower + " lead_speed_kmh=" + Speed(m_Lead) +
					" follower_speed_kmh=" + Speed(m_Follower) + " native_wait_selected=true");
				if (m_iStableSamples < 3)
					return;
				m_bResumeSeatStaging = false;
				BeginSeatTransfer(true);
				return;
			}
			if (!PollSeatTransfer())
				return;
			if (m_iResumeStage == 1)
			{
				bool held = CF_ConvoySession.CF_PanelHold(m_Player);
				Print("[ConvoyFollower] RESUME_STATUS_HOLD_COMMAND: accepted=" + held +
					" owner_pilot=" + IsFixtureSeat(m_Player, true));
				if (!held || !m_Driver.CF_HasPanelHoldRequest())
					FailResumeStatus("hold_precondition_rejected");
				else
					ResumeStage(2);
			}
			else if (!BeginLeadHold())
				FailResumeStatus("native_wait_after_seat_setup_failed", true);
			else
			{
				ReleaseResumeParking("native_pilot_returned", true);
				m_bResumeTransferPoseMonitor = false;
				m_fStationaryStartMs = 0;
				ResumeStage(4);
			}
			return;
		}
		if (m_iResumeStage == 2)
		{
			if (!IsFixtureSeat(m_Player, true) || m_Pilot.IsInVehicle())
			{
				FailResumeStatus("hold_owner_seat_changed", true);
				return;
			}
			if (m_Driver.CF_IsPanelHeld() && Speed(m_Lead) <= 1.0 && Speed(m_Follower) <= 1.0)
				m_iStableSamples++;
			else m_iStableSamples = 0;
			if (m_iStableSamples < 3)
				return;
			float gap = vector.DistanceXZ(m_Lead.GetOrigin(), m_Follower.GetOrigin());
			if (gap > CF_ConvoySettings.Get().m_fStoppedGap + 4.0)
			{
				FailResumeStatus("resume_not_inside_stopped_spacing", true);
				return;
			}
			bool accepted = CF_ConvoySession.CF_PanelResume(m_Player);
			string initial = ReadResumeStatus("immediate_acceptance");
			Print("[ConvoyFollower] RESUME_STATUS_COMMAND: accepted=" + accepted + " gap_m=" + gap +
				" lead=" + m_Lead.GetOrigin() + " follower=" + m_Follower.GetOrigin());
			if (!accepted || initial.IndexOf("accepted:") != 0 || m_Driver.CF_HasPanelHoldRequest())
			{
				FailResumeStatus("resume_not_accepted_or_premature_completion");
				return;
			}
			m_fResumeAcceptedMs = GetGame().GetWorld().GetWorldTime();
			CaptureResumePose();
			Print("[ConvoyFollower] RESUME_STATUS_NO_GETTER_BEGIN: seconds=" + m_iTicks + " required_stationary_s=100");
			ResumeStage(3);
			BeginSeatTransfer(false);
			return;
		}
		if (!NativeLeadReady())
		{
			FailResumeStatus("native_lead_ownership_or_seat_lost", true);
			return;
		}
		if (m_iResumeStage == 4)
		{
			// Intentionally do not query panel order state in this interval.
			// The authoritative session must keep evaluating with no getter.
			if (!m_LeadWaitBehavior || m_PilotUtility.GetCurrentBehavior() != m_LeadWaitBehavior)
			{
				FailResumeStatus("native_wait_ownership_lost", true);
				return;
			}
			if (m_Driver.CF_HasPanelHoldRequest())
			{
				FailResumeStatus("explicit_hold_returned_without_command");
				return;
			}
			float now = GetGame().GetWorld().GetWorldTime();
			if (m_fStationaryStartMs == 0 && Speed(m_Lead) <= 1.0 && Speed(m_Follower) <= 1.0)
				m_fStationaryStartMs = now;
			LogStationarySample("no_getter_wait");
			if (m_fStationaryStartMs == 0 || now - m_fStationaryStartMs < 100000.0)
				return;
			string waiting = ReadResumeStatus("after_no_getter_window");
			if (waiting.IndexOf("waiting:") != 0)
			{
				FailResumeStatus("stationary_resume_completed_failed_or_not_waiting");
				return;
			}
			m_fWaitLeadDrift = m_fResumeLeadDrift;
			m_fWaitFollowerDrift = m_fResumeFollowerDrift;
			Print("[ConvoyFollower] RESUME_STATUS_WAIT_VERIFIED: seconds=" + m_iTicks +
				" stationary_s=" + ((now - m_fStationaryStartMs) / 1000.0) +
				" no_getter_s=" + ((now - m_fResumeAcceptedMs) / 1000.0) +
				" max_lead_drift_m=" + m_fWaitLeadDrift + " max_follower_drift_m=" + m_fWaitFollowerDrift + " state='" + waiting + "'");
			m_bResumePoseMonitor = false;
			if (!MoveNativeLead(m_vResumeStatusGoal))
				FailResumeStatus("native_restart_order_failed", true);
			else
				ResumeStage(5);
			return;
		}
		if (m_iResumeStage == 5 || m_iResumeStage == 6)
		{
			if (!SampleMotion())
				return;
			string state = ReadResumeStatus("moving");
			if (state.IndexOf("blocked:") == 0 || state.IndexOf("cancelled:") == 0 || state.IndexOf("failed:") == 0)
			{
				FailResumeStatus("resume_failed_during_powered_restart");
				return;
			}
			if (state.IndexOf("completed:") == 0 && !m_bAuthoritativeCompleted)
			{
				float commandDisplacement = vector.DistanceXZ(m_Follower.GetOrigin(), m_vResumeFollowerPose);
				if (commandDisplacement < 3.0 || m_fFollowerProgress <= 0.25)
				{
					FailResumeStatus("completion_preceded_meaningful_postcommand_motion");
					return;
				}
				// Production and fixture polls have different phase offsets; keep
				// independent strong movement gates below instead of equating counts.
				m_bAuthoritativeCompleted = true;
				Print("[ConvoyFollower] RESUME_STATUS_COMPLETED_OBSERVED: seconds=" + m_iTicks +
					" lead_progress_m=" + m_fLeadProgress + " follower_progress_m=" + m_fFollowerProgress +
					" command_displacement_m=" + commandDisplacement + " follower_powered_samples=" + m_iFollowerPoweredSamples);
			}
			if (m_iResumeStage == 5 && m_fLeadProgress >= 20.0 && m_fFollowerProgress >= 15.0 &&
				m_iLeadPoweredSamples >= 2 && m_iFollowerPoweredSamples >= 2 &&
				m_iLeadMovingSamples >= 3 && m_iFollowerMovingSamples >= 3 && m_bAuthoritativeCompleted)
			{
				Print("[ConvoyFollower] RESUME_STATUS_POWERED_VERIFIED: seconds=" + m_iTicks +
					" lead_progress_m=" + m_fLeadProgress + " follower_progress_m=" + m_fFollowerProgress +
					" lead_powered_samples=" + m_iLeadPoweredSamples + " follower_powered_samples=" + m_iFollowerPoweredSamples + " same_driver_truck_roster=true");
				if (!BeginLeadHold())
					FailResumeStatus("final_native_stop_failed", true);
				else ResumeStage(6);
			}
			else if (m_iResumeStage == 6)
			{
				if (NativeLeadStopped() && Speed(m_Follower) <= 1.0 &&
					vector.DistanceXZ(m_Follower.GetOrigin(), m_vPreviousFollower) <= 0.3)
					m_iStableSamples++;
				else m_iStableSamples = 0;
				if (m_iStableSamples >= 3)
				{
					CaptureResumePose();
					m_bResumeTerminal = true;
					m_fResumeTerminalMs = GetGame().GetWorld().GetWorldTime();
					Print("[ConvoyFollower] RESUME_STATUS_FINAL: seconds=" + m_iTicks +
						" wait_lead_drift_m=" + m_fWaitLeadDrift + " wait_follower_drift_m=" + m_fWaitFollowerDrift +
						" peak_gap_m=" + m_fResumePeakGap + " lead_progress_m=" + m_fLeadProgress +
						" follower_progress_m=" + m_fFollowerProgress + " authoritative_completed=" + m_bAuthoritativeCompleted);
					Print("[ConvoyFollower] RESUME_STATUS_RESULT: PASS stationary_no_getter_100s_then_powered_authoritative_completion observation_pending=true");
					ResumeStage(7);
				}
			}
			m_vPreviousLead = m_Lead.GetOrigin();
			m_vPreviousFollower = m_Follower.GetOrigin();
			return;
		}
		if (m_iResumeStage == 7)
		{
			if (!m_LeadWaitBehavior || m_PilotUtility.GetCurrentBehavior() != m_LeadWaitBehavior)
			{
				FailResumeStatus("postterminal_native_wait_lost", true);
				return;
			}
			string observedState = ReadResumeStatus("postterminal");
			if (observedState.IndexOf("completed:") != 0 || m_Driver.CF_HasPanelHoldRequest())
			{
				FailResumeStatus("postterminal_resume_status_changed");
				return;
			}
			LogStationarySample("postterminal");
			float observedSeconds = (GetGame().GetWorld().GetWorldTime() - m_fResumeTerminalMs) / 1000.0;
			if (observedSeconds >= 30.0)
			{
				Print("[ConvoyFollower] RESUME_STATUS_OBSERVATION_COMPLETE: seconds=" + m_iTicks +
					" observed_s=" + observedSeconds + " failed=false max_lead_drift_m=" + m_fResumeLeadDrift +
					" max_follower_drift_m=" + m_fResumeFollowerDrift + " peak_gap_m=" + m_fResumePeakGap + " same_driver_truck_roster=true");
				m_bResumePoseMonitor = false;
				m_bFinished = true;
				GetGame().GetCallqueue().Remove(Poll);
			}
		}
	}
}
