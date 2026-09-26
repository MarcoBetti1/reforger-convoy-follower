class CF_OriginalFollowResumeProbeComponentClass : CF_EntityFollowRoadProbeComponentClass
{
}

// Private one-follower extension. The unchanged paced course and its full
// 180-second captured Wait finish first. Only the fixture lead is ever driven
// or parked here; follower changes go through the real server session commands.
class CF_OriginalFollowResumeProbeComponent : CF_EntityFollowRoadProbeComponent
{
	[Attribute(defvalue: "0", desc: "Diagnostic only: observe commands after a completed safe baseline with spacing failure only; overall result remains FAIL")]
	protected bool m_bCommandsAfterSpacingFailure;
	protected bool m_bCommandNonSpacingFailure;
	protected bool m_bCommandDiagnosticContinuation;

	// 0 baseline, 1 owner seat, 2 Hold settling, 3 held observation,
	// 4 native seat, 5 powered restart. Physical restart is the final gate.
	protected int m_iCommandStage;
	protected int m_iSeatStep;
	protected int m_iCommandStable;
	protected int m_iBaselineWaitSamples;
	protected int m_iOriginalActivitySamples;
	protected int m_iBaselineSequence;
	protected int m_iRestartSequence;
	protected int m_iLeadRestartPowered;
	protected int m_iFollowerRestartPowered;
	protected float m_fCommandBeginMs;
	protected float m_fCommandStageMs;
	protected float m_fCommandHoldDrift;
	protected float m_fCommandHoldSpeed;
	protected float m_fLeadRestartProgress;
	protected float m_fFollowerRestartProgress;
	protected float m_fExactRestartProgress;
	protected vector m_vTransferOrigin;
	protected vector m_vCommandHeldOrigin;
	protected vector m_vRestartGoal;
	protected vector m_vRestartAxis;
	protected vector m_vRestartLeadStart;
	protected vector m_vRestartFollowerStart;
	protected vector m_vCommandPreviousLead;
	protected vector m_vCommandPreviousFollower;
	protected bool m_bCommandParking;
	protected bool m_bCommandHoldMeasure;
	protected bool m_bCommandHoldAccepted;
	protected bool m_bCommandResumeAccepted;
	protected bool m_bCommandComplete;
	protected CF_EntityCapturedWait m_BaselineCapturedWait;
	protected CF_OriginalFollowActivity m_PreviousRestartActivity;
	protected AIWaypoint m_PreviousRestartWaypoint;
	protected string m_sBaselineWaitKey;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (Replication.IsServer())
			Print("[ConvoyFollower] ORIGINAL_RESUME_INIT: run_id=" + m_sPacedRun +
				" baseline_hold_s=180 command_hold_s=30 max_drift_m=2 restart_progress_m=20 powered_samples=3" +
				" command_timeout_s=240 total_timeout_s=720 owner_seat_setup=true human_input_claim=false follower_writes=false");
	}

	protected CF_EntityFollowDriverControllerComponent CommandDriver()
	{
		if (m_PacedTrucks.Count() != 2)
			return null;
		return CF_EntityFollowDriverControllerComponent.Cast(m_PacedTrucks[1].Driver);
	}

	protected bool CommandIdentities()
	{
		if (!PacedWorldAlive() || m_iExpectedTrucks != 1 || m_PacedTrucks.Count() != 2 || !IdentityRetained(1, true))
			return false;
		if (!EntityAlive(m_Lead) || !EntityAlive(m_Pilot) || !EntityAlive(m_PacedOwner) || m_Player != m_PacedOwner)
			return false;
		if (m_PacedTrucks[0].Truck != m_Lead || m_PacedTrucks[0].Pilot != m_Pilot ||
			GetGame().GetWorld().FindEntityByName("CF_SmokeLead") != m_Lead)
			return false;
		PlayerManager players = GetGame().GetPlayerManager();
		return players && players.GetPlayerControlledEntity(m_iPacedOwnerId) == m_PacedOwner &&
			m_PacedSession && CF_ConvoySession.GetForPlayer(m_PacedOwner) == m_PacedSession;
	}

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

	protected CF_EntityCapturedWait SelectedCapturedWait()
	{
		CF_EntityFollowDriverControllerComponent driver = CommandDriver();
		if (!driver || !IdentityRetained(1, true)) return null;
		CF_PacedRoadTruckSample sample = m_PacedTrucks[1];
		SCR_AIUtilityComponent utility = PilotUtility(sample);
		if (!utility) return null;
		CF_EntityCapturedWait action = CF_EntityCapturedWait.Cast(utility.GetCurrentBehavior());
		if (!action || action.GetActionState() == EAIActionState.COMPLETED || action.GetActionState() == EAIActionState.FAILED)
			return null;
		if (!driver.CF_HasCapturedWaitLease(action, sample.Pilot, sample.Truck, sample.Group)) return null;
		return action;
	}

	protected CF_OriginalFollowActivity ExactOriginalActivity()
	{
		CF_EntityFollowDriverControllerComponent driver = CommandDriver();
		if (!driver || !IdentityRetained(1, true)) return null;
		CF_EntityFollowWaypoint waypoint = driver.CF_GetEntityFollowWaypoint();
		SCR_AIGroupUtilityComponent utility = GroupUtility(m_PacedTrucks[1]);
		if (!waypoint || !utility || m_PacedTrucks[1].Group.GetCurrentWaypoint() != waypoint) return null;
		CF_OriginalFollowActivity action = CF_OriginalFollowActivity.Cast(utility.GetCurrentAction());
		if (!action || utility.GetExecutedAction() != action || waypoint.CF_GetActivity() != action) return null;
		if (action.GetActionState() == EAIActionState.COMPLETED || action.GetActionState() == EAIActionState.FAILED) return null;
		if (action.m_Entity.m_Value != m_Lead || waypoint.GetEntity() != m_Lead || action.m_RelatedWaypoint != waypoint) return null;
		string reason;
		if (!action.Lease || !action.Lease.Executing(m_PacedTrucks[1].Group, reason)) return null;
		return action;
	}

	override protected void LogPacedSamples()
	{
		super.LogPacedSamples();
		if (m_iCommandStage != 0 || m_PacedTrucks.Count() != 2) return;
		CF_OriginalFollowActivity activity = ExactOriginalActivity();
		if (activity)
		{
			m_iOriginalActivitySamples++;
			m_iBaselineSequence = activity.CF_GetSequence();
		}
		if (!m_bPacedObserving) return;
		CF_EntityCapturedWait selected = SelectedCapturedWait();
		if (!selected || (m_BaselineCapturedWait && selected != m_BaselineCapturedWait))
		{
			NoteFailure("prototype", "original_resume_baseline_exact_captured_wait_lost");
			return;
		}
		m_BaselineCapturedWait = selected;
		m_sBaselineWaitKey = selected.ToString();
		m_iBaselineWaitSamples++;
		Print("[ConvoyFollower] ORIGINAL_RESUME_BASELINE_WAIT: run_id=" + m_sPacedRun +
			" tick=" + m_iPacedTick + " selected=true exact_lease=true action=" + m_sBaselineWaitKey + " samples=" + m_iBaselineWaitSamples);
	}

	// Every failure remains sticky in the parent. Keep a separate exclusion
	// latch so a later non-spacing failure cannot be hidden by the first cause.
	override protected void NoteFailure(string scope, string reason)
	{
		if (scope != "spacing" || reason != "link_over_60m_unit_1")
			m_bCommandNonSpacingFailure = true;
		super.NoteFailure(scope, reason);
	}

	protected bool MayContinueSpacingDiagnostic()
	{
		if (!m_bCommandsAfterSpacingFailure || !m_bPacedFailed || !m_bPacedSpacingFailed) return false;
		if (m_bCommandNonSpacingFailure || m_bPacedDriftFailed || m_bEntityGateFailed) return false;
		return true;
	}

	// Virtual existing seam: suppress the parent's terminal/close until all
	// command gates finish. No shared paced source hook is needed.
	override protected void EndPaced()
	{
		if (m_bPacedTerminal) return;
		bool diagnosticSpacing = MayContinueSpacingDiagnostic();
		if (m_iCommandStage == 0 && (!m_bPacedFailed || diagnosticSpacing))
		{
			if (!m_bPacedObserving || m_iObservationSamples < 180 || m_iBaselineWaitSamples < 180 ||
				m_iOriginalActivitySamples < 3 || !CommandIdentities() || !LeadWaitSelected())
			{
				CommandFailure("baseline_or_exact_wait_evidence_missing");
				return;
			}
			if (diagnosticSpacing)
			{
				m_bCommandDiagnosticContinuation = true;
				Print("[ConvoyFollower] ORIGINAL_RESUME_DIAGNOSTIC_CONTINUE: run_id=" + m_sPacedRun +
					" spacing_only=true overall_pass_eligible=false first_failure=" + m_sFirstFailure +
					" peak_gap_m=" + m_fPacedPeakGap + " same_numerical_gates=true");
			}
			Print("[ConvoyFollower] ORIGINAL_RESUME_BASELINE_COMPLETE: run_id=" + m_sPacedRun +
				" observed_s=" + (GetGame().GetWorld().GetWorldTime() - m_fObservationStartMs) / 1000.0 +
				" wait_samples=" + m_iBaselineWaitSamples + " wait=" + m_sBaselineWaitKey + " peak_gap_m=" + m_fPacedPeakGap + " provisional=true");
			m_bPacedObserving = false;
			m_fCommandBeginMs = GetGame().GetWorld().GetWorldTime();
			if (!SurveyCommandExtension())
			{
				CommandFailure("fixture_restart_road_extension_unavailable");
				return;
			}
			SetCommandStage(1);
			BeginCommandSeatTransfer(true);
			return;
		}
		Print("[ConvoyFollower] ORIGINAL_RESUME_RESULT: pass=" + (!m_bPacedFailed && m_bCommandComplete) +
			" run_id=" + m_sPacedRun + " stage=" + m_iCommandStage + " hold_accepted=" + m_bCommandHoldAccepted +
			" resume_accepted=" + m_bCommandResumeAccepted + " hold_drift_m=" + m_fCommandHoldDrift +
			" lead_progress_m=" + m_fLeadRestartProgress + " follower_progress_m=" + m_fFollowerRestartProgress +
			" exact_activity_progress_m=" + m_fExactRestartProgress +
			" lead_powered=" + m_iLeadRestartPowered + " follower_powered=" + m_iFollowerRestartPowered + " peak_gap_m=" + m_fPacedPeakGap);
		super.EndPaced();
	}

	protected void SetCommandStage(int stage)
	{
		m_iCommandStage = stage;
		m_fCommandStageMs = GetGame().GetWorld().GetWorldTime();
		m_iCommandStable = 0;
		Print("[ConvoyFollower] ORIGINAL_RESUME_STAGE: run_id=" + m_sPacedRun + " stage=" + stage + " seconds=" + PacedSeconds());
	}

	protected void CommandFailure(string reason)
	{
		if (m_bPacedTerminal) return;
		NoteFailure("command", reason);
		ReleaseCommandParking("failure");
		EndPaced();
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
		if (!CF_RoadPolyline.TryWalk(points, m_Lead.GetOrigin(), forward, 50, candidate)) return false;
		vector reached;
		if (!roads.GetReachableWaypointInRoad(m_Lead.GetOrigin(), candidate, 5, reached) || vector.DistanceXZ(candidate, reached) > 3) return false;
		vector delta = reached - m_Lead.GetOrigin();
		float distance = vector.DistanceXZ(reached, m_Lead.GetOrigin());
		if (distance < 35 || distance > 60 || delta[0] * forward[0] + delta[2] * forward[2] < distance * 0.5) return false;
		m_vRestartGoal = reached;
		m_vRestartAxis = Vector(delta[0] / distance, 0, delta[2] / distance);
		Print("[ConvoyFollower] ORIGINAL_RESUME_SURVEY: run_id=" + m_sPacedRun + " road_id=" + roadId +
			" width=" + road.GetWidth() + " road_distance=" + roadDistance + " start=" + m_Lead.GetOrigin() +
			" goal=" + reached + " axis=" + m_vRestartAxis + " recorded_road_walk_m=50 geometry_only=true");
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

	protected bool StartCommandRestart()
	{
		if (!CanControlLead() || !OwnerRetained() || !EntryReady(m_PacedTrucks[0]) || m_PilotWaypoint) return false;
		Resource prefab = Resource.Load("{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et");
		if (!prefab.IsValid()) return false;
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = m_vRestartGoal;
		SCR_AIWaypoint waypoint = SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(prefab, null, params));
		if (!waypoint) return false;
		if (!m_PacedCruise.Request(m_Pilot, m_Lead, m_fPacedLeadSpeedKmh, "original_resume_native_lead"))
		{
			SCR_EntityHelper.DeleteEntityAndChildren(waypoint);
			return false;
		}
		waypoint.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_GAMEMASTER);
		waypoint.SetCompletionRadius(5);
		m_PilotWaypoint = waypoint;
		m_PilotGroup.AddWaypoint(waypoint);
		ReleaseCommandParking("native_pilot_reentered_powered_leg");
		m_vRestartLeadStart = m_Lead.GetOrigin();
		m_vRestartFollowerStart = m_PacedTrucks[1].Truck.GetOrigin();
		m_vCommandPreviousLead = m_vRestartLeadStart;
		m_vCommandPreviousFollower = m_vRestartFollowerStart;
		Print("[ConvoyFollower] ORIGINAL_RESUME_RESTART_BEGIN: run_id=" + m_sPacedRun +
			" lead_start=" + m_vRestartLeadStart + " follower_start=" + m_vRestartFollowerStart +
			" goal=" + m_vRestartGoal + " axis=" + m_vRestartAxis + " lead_radius=5 native_lead=true passenger_owner=true");
		return true;
	}

	protected void SampleCommandMotion()
	{
		CF_OriginalFollowActivity activity = ExactOriginalActivity();
		int sequence;
		if (activity) sequence = activity.CF_GetSequence();
		bool fresh = activity && sequence > m_iBaselineSequence;
		if (fresh && m_iRestartSequence == 0) m_iRestartSequence = sequence;
		bool exact = fresh && sequence == m_iRestartSequence;
		if (m_iRestartSequence > 0 && activity && sequence != m_iRestartSequence)
		{ CommandFailure("restart_activity_replaced_before_physical_gate"); return; }
		bool consecutive = exact && activity == m_PreviousRestartActivity && activity.m_RelatedWaypoint == m_PreviousRestartWaypoint;
		if (!consecutive)
		{
			m_iFollowerRestartPowered = 0;
			m_fExactRestartProgress = 0;
		}
		for (int unit = 0; unit < 2; unit++)
		{
			CF_PacedRoadTruckSample sample = m_PacedTrucks[unit];
			vector start = m_vRestartLeadStart;
			vector previous = m_vCommandPreviousLead;
			if (unit == 1) { start = m_vRestartFollowerStart; previous = m_vCommandPreviousFollower; }
			vector position = sample.Truck.GetOrigin();
			vector delta = position - previous;
			vector overall = position - start;
			float step = delta[0] * m_vRestartAxis[0] + delta[2] * m_vRestartAxis[2];
			float progress = overall[0] * m_vRestartAxis[0] + overall[2] * m_vRestartAxis[2];
			CarControllerComponent car = CarControllerComponent.Cast(sample.Truck.FindComponent(CarControllerComponent));
			if (!car || !car.GetSimulation() || vector.DistanceXZ(position, previous) > 20)
			{ CommandFailure("restart_simulation_missing_or_position_discontinuity"); return; }
			VehicleWheeledSimulation sim = car.GetSimulation();
			bool powered = sim.EngineIsOn() && sim.GetThrottle() > 0.05 && sim.GetGear() >= 2 && sim.GetSpeedKmh() >= 1.5 && step >= 0.25;
			if (unit == 0)
			{
				m_fLeadRestartProgress = progress;
				if (powered) m_iLeadRestartPowered++;
				m_vCommandPreviousLead = position;
			}
			else
			{
				m_fFollowerRestartProgress = progress;
				if (consecutive)
				{
					m_fExactRestartProgress += step;
					if (powered) m_iFollowerRestartPowered++;
				}
				m_vCommandPreviousFollower = position;
			}
			Print("[ConvoyFollower] ORIGINAL_RESUME_POSITION: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
				" seconds=" + PacedSeconds() + " stage=" + m_iCommandStage + " unit=" + unit + " truck_id=" + EntityKey(sample.Truck) +
				" pilot_id=" + EntityKey(sample.Pilot) + " origin=" + position + " signed_step_m=" + step + " signed_progress_m=" + progress +
				" speed_kmh=" + sim.GetSpeedKmh() + " engine=" + sim.EngineIsOn() + " throttle=" + sim.GetThrottle() +
				" brake=" + sim.GetBrake() + " gear=" + sim.GetGear() + " powered=" + powered +
				" fresh_exact_activity=" + exact + " consecutive_exact=" + consecutive + " exact_activity_progress_m=" + m_fExactRestartProgress +
				" sequence=" + sequence + " predecessor_id=" + EntityKey(m_Lead));
		}
		m_PreviousRestartActivity = null;
		m_PreviousRestartWaypoint = null;
		if (exact)
		{
			m_PreviousRestartActivity = activity;
			m_PreviousRestartWaypoint = activity.m_RelatedWaypoint;
		}
	}

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		super.EOnPostFrame(owner, timeSlice);
		if (m_iCommandStage == 0 || m_bPacedTerminal || !PacedWorldAlive()) return;
		if (!CommandIdentities()) { CommandFailure("original_identity_or_session_lost"); return; }
		if (m_bCommandParking)
		{
			if (!CanCommandPark() || vector.DistanceXZ(m_Lead.GetOrigin(), m_vTransferOrigin) > 2 || CommandSpeed(m_Lead) > 2)
			{ CommandFailure("lead_moved_or_owner_changed_during_seat_setup"); return; }
			ApplyCommandParking();
		}
		if (m_bCommandHoldMeasure)
		{
			float drift = vector.DistanceXZ(m_PacedTrucks[1].Truck.GetOrigin(), m_vCommandHeldOrigin);
			float speed = CommandSpeed(m_PacedTrucks[1].Truck);
			if (drift > m_fCommandHoldDrift) m_fCommandHoldDrift = drift;
			if (speed > m_fCommandHoldSpeed) m_fCommandHoldSpeed = speed;
			if (drift > 2 || speed > 2) { CommandFailure("explicit_hold_physical_limit"); return; }
		}
	}

	override protected void Poll()
	{
		if (m_iCommandStage == 0) { super.Poll(); return; }
		if (m_bPacedTerminal || !PacedWorldAlive()) return;
		m_iPacedTick++;
		float now = GetGame().GetWorld().GetWorldTime();
		float stageMs = now - m_fCommandStageMs;
		if (now - m_fCommandBeginMs > 240000 || PacedSeconds() > 720 || stageMs > 90000)
		{ CommandFailure("command_stage_or_total_timeout"); return; }
		if (!CommandIdentities()) { CommandFailure("original_identity_or_session_lost"); return; }
		MeasurePeaks();
		if (m_bPacedFailed && (!m_bCommandDiagnosticContinuation || !MayContinueSpacingDiagnostic()))
		{ EndPaced(); return; }
		CF_EntityFollowDriverControllerComponent driver = CommandDriver();
		if (!driver || driver.CF_HasEntityFallbackFailure()) { CommandFailure("private_controller_failure"); return; }
		Print("[ConvoyFollower] ORIGINAL_RESUME_STATUS: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
			" seconds=" + PacedSeconds() + " stage=" + m_iCommandStage + " state=" + CF_ConvoySession.CF_GetOwnerPanelOrderState(m_PacedOwner) +
			" hold_requested=" + driver.CF_HasPanelHoldRequest() + " held=" + driver.CF_IsPanelHeld() +
			" owner_pilot=" + CommandSeat(m_PacedOwner, true) + " native_pilot=" + CommandSeat(m_Pilot, true) +
			" lead_id=" + EntityKey(m_Lead) + " follower_id=" + EntityKey(m_PacedTrucks[1].Truck) +
			" follower_pilot_id=" + EntityKey(m_PacedTrucks[1].Pilot) + " identities=true" +
			" lead=" + m_Lead.GetOrigin() + " follower=" + m_PacedTrucks[1].Truck.GetOrigin() +
			" lead_speed_kmh=" + CommandSpeed(m_Lead) + " follower_speed_kmh=" + CommandSpeed(m_PacedTrucks[1].Truck) +
			" baseline_wait_selected=" + (SelectedCapturedWait() == m_BaselineCapturedWait && m_BaselineCapturedWait != null));
		if (m_iCommandStage == 1)
		{
			if (stageMs > 30000) { CommandFailure("owner_seat_transfer_timeout"); return; }
			if (!PollCommandSeatTransfer(true)) return;
			m_bCommandHoldAccepted = CF_ConvoySession.CF_PanelHold(m_PacedOwner);
			Print("[ConvoyFollower] ORIGINAL_RESUME_COMMAND: run_id=" + m_sPacedRun + " command=Hold accepted=" + m_bCommandHoldAccepted);
			if (!m_bCommandHoldAccepted || !driver.CF_HasPanelHoldRequest()) { CommandFailure("server_hold_rejected"); return; }
			SetCommandStage(2);
		}
		else if (m_iCommandStage == 2)
		{
			if (driver.CF_IsPanelHeld() && driver.CF_HasPanelHoldRequest() && CommandSpeed(m_PacedTrucks[1].Truck) <= 2)
				m_iCommandStable++;
			else m_iCommandStable = 0;
			if (m_iCommandStable < 3) return;
			m_vCommandHeldOrigin = m_PacedTrucks[1].Truck.GetOrigin();
			m_bCommandHoldMeasure = true;
			SetCommandStage(3);
			Print("[ConvoyFollower] ORIGINAL_RESUME_HOLD_BEGIN: run_id=" + m_sPacedRun + " origin=" + m_vCommandHeldOrigin + " required_s=30 max_drift_m=2");
		}
		else if (m_iCommandStage == 3)
		{
			if (!driver.CF_HasPanelHoldRequest() || !driver.CF_IsPanelHeld()) { CommandFailure("explicit_hold_intent_lost"); return; }
			Print("[ConvoyFollower] ORIGINAL_RESUME_HOLD_SAMPLE: run_id=" + m_sPacedRun + " seconds=" + PacedSeconds() +
				" held_s=" + stageMs / 1000.0 + " origin=" + m_PacedTrucks[1].Truck.GetOrigin() +
				" max_drift_m=" + m_fCommandHoldDrift + " max_speed_kmh=" + m_fCommandHoldSpeed + " identities=true");
			if (stageMs < 30000) return;
			Print("[ConvoyFollower] ORIGINAL_RESUME_HOLD_COMPLETE: run_id=" + m_sPacedRun + " held_s=" + stageMs / 1000.0 + " max_drift_m=" + m_fCommandHoldDrift);
			m_bCommandResumeAccepted = CF_ConvoySession.CF_PanelResume(m_PacedOwner);
			Print("[ConvoyFollower] ORIGINAL_RESUME_COMMAND: run_id=" + m_sPacedRun + " command=Resume accepted=" + m_bCommandResumeAccepted + " completion_claim=false");
			if (!m_bCommandResumeAccepted || driver.CF_HasPanelHoldRequest()) { CommandFailure("server_resume_rejected_or_hold_retained"); return; }
			m_bCommandHoldMeasure = false;
			SetCommandStage(4);
			BeginCommandSeatTransfer(false);
		}
		else if (m_iCommandStage == 4)
		{
			if (stageMs > 30000) { CommandFailure("native_seat_transfer_timeout"); return; }
			if (!PollCommandSeatTransfer(false)) return;
			if (!StartCommandRestart()) { CommandFailure("native_restart_order_failed"); return; }
			SetCommandStage(5);
		}
		else if (m_iCommandStage == 5)
		{
			if (!OwnerRetained() || !CanControlLead() || driver.CF_HasPanelHoldRequest()) { CommandFailure("restart_identity_or_hold_intent"); return; }
			SampleCommandMotion();
			if (m_bPacedTerminal) return;
			if (m_fLeadRestartProgress >= 20 && m_fFollowerRestartProgress >= 20 && m_fExactRestartProgress >= 20 &&
				m_iLeadRestartPowered >= 3 && m_iFollowerRestartPowered >= 3 && ExactOriginalActivity())
			{
				Print("[ConvoyFollower] ORIGINAL_RESUME_VERIFIED: run_id=" + m_sPacedRun + " lead_progress_m=" + m_fLeadRestartProgress +
					" follower_progress_m=" + m_fFollowerRestartProgress + " lead_powered=" + m_iLeadRestartPowered +
					" follower_powered=" + m_iFollowerRestartPowered + " exact_activity_progress_m=" + m_fExactRestartProgress + " same_original_assignment=true");
				m_bCommandComplete = true;
				EndPaced(); // Parent retains its exact lead park and schedules the 30 s capture grace.
				return;
			}
			if (!m_bPacedHoldingLead && vector.DistanceXZ(m_Lead.GetOrigin(), m_vRestartGoal) <= 8)
				HoldLeadAtRoadGoal();
		}
	}

	override void OnDelete(IEntity owner)
	{
		ReleaseCommandParking("deleted_or_world_cleanup");
		m_BaselineCapturedWait = null;
		m_PreviousRestartActivity = null;
		m_PreviousRestartWaypoint = null;
		super.OnDelete(owner);
	}
}
