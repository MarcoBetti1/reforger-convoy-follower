// Test-only real server Hold/Resume, separate from the ordinary pacing course.
// Lead-seat transfers establish command eligibility; they are fixture setup,
// never evidence of keyboard input. This probe never controls the follower.
class CF_ExplicitHoldResumeProbeComponentClass : CF_ArlandClearFieldOffroadProbeComponentClass
{
}

class CF_ExplicitHoldResumeProbeComponent : CF_ArlandClearFieldOffroadProbeComponent
{
	// 0 setup, 1 recruit, 2 initial drive, 3 native stop, 4 owner pilot,
	// 5 production Hold capture, 6 native pilot, 7 lead advance/30s Hold,
	// 8 owner pilot/Resume, 9 native pilot, 10 resumed drive, 11 final stop.
	protected int m_iExplicitPhase;
	protected int m_iPhaseStart;
	protected int m_iTransferStep;
	protected int m_iStableSamples;
	protected int m_iOwnerId;
	protected bool m_bTransferToOwner;
	protected bool m_bAdvanceStopped;
	protected bool m_bMeasureFollowerHold;
	protected bool m_bHoldCommandAccepted;
	protected bool m_bResumeCommandAccepted;
	protected IEntity m_ExpectedDriver;
	protected CF_ConvoySession m_ExpectedSession;
	protected vector m_vHeldFollower;
	protected float m_fHoldMaxDrift;
	protected float m_fHoldMaxSpeed;
	protected float m_fHoldWindowStartMs;
	protected vector m_vLegLeadStart;
	protected vector m_vLegFollowerStart;
	protected vector m_vPreviousLead;
	protected vector m_vPreviousFollower;
	protected vector m_vFinalResumeGoal;
	protected int m_iLeadPoweredSamples;
	protected int m_iFollowerPoweredSamples;
	protected int m_iLeadMovingSamples;
	protected int m_iFollowerMovingSamples;
	protected float m_fLeadProgress;
	protected float m_fFollowerProgress;
	protected string m_sLeadBoardingDiagnostic;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Lead = Vehicle.Cast(owner);
		SetEventMask(owner, EntityEvent.POSTFRAME);
		CF_ConvoySettings.Get();
		Print("[ConvoyFollower] EXPLICIT_HOLD_INIT: expected=1 timeout_s=480 hold_s=30 max_drift_m=2 native_lead=true production_follower=true test_seat_transfers=true human_input_claim=false");
		Print("[ConvoyFollower] EXPLICIT_HOLD_SETTINGS: native_cruise=" + CF_ConvoySettings.Get().m_bNativeCruiseEnabled +
			" moving_gap=" + CF_ConvoySettings.Get().m_fMovingGap + " stopped_gap=" + CF_ConvoySettings.Get().m_fStoppedGap +
			" move_completion_radius=" + CF_ConvoySettings.Get().m_fMoveCompletionRadius);
		GetGame().GetCallqueue().CallLater(Poll, 1000, true);
	}

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		super.EOnPostFrame(owner, timeSlice);
		if (m_bFinished || CF_ConvoySession.CF_IsWorldCleanup() || !m_bMeasureFollowerHold || !m_Follower)
			return;
		// Read every frame so the drift maximum cannot hide a between-poll roll.
		float drift = vector.DistanceXZ(m_Follower.GetOrigin(), m_vHeldFollower);
		if (drift > m_fHoldMaxDrift)
			m_fHoldMaxDrift = drift;
		float speed = Speed(m_Follower);
		if (speed > m_fHoldMaxSpeed)
			m_fHoldMaxSpeed = speed;
		if (drift > 2.0 || speed > 2.0)
			Finish("FAIL follower_moved_during_explicit_hold");
	}

	protected void SetPhase(int phase)
	{
		m_iExplicitPhase = phase;
		m_iPhaseStart = m_iTicks;
		m_iStableSamples = 0;
		Print("[ConvoyFollower] EXPLICIT_HOLD_PHASE: seconds=" + m_iTicks + " phase=" + phase);
	}

	protected bool IsFixtureSeat(ChimeraCharacter character, bool pilot)
	{
		if (!character)
			return false;
		CompartmentAccessComponent access = character.GetCompartmentAccessComponent();
		BaseCompartmentSlot slot;
		if (access)
			slot = access.GetCompartment();
		return access && !access.IsGettingIn() && !access.IsGettingOut() &&
			slot && slot.GetOccupant() == character && slot.IsPiloting() == pilot &&
			access.GetVehicleIn(character) == m_Lead;
	}

	protected bool FixtureOnFoot(ChimeraCharacter character)
	{
		if (!character || character.IsInVehicle())
			return false;
		CompartmentAccessComponent access = character.GetCompartmentAccessComponent();
		return access && !access.IsGettingIn() && !access.IsGettingOut();
	}

	protected bool InitialLeadBoardingHandoverReady()
	{
		if (!NativeLeadReady())
			return false;
		SCR_BoardingEntityWaypoint boarding = SCR_BoardingEntityWaypoint.Cast(m_PilotWaypoint);
		ref array<AIWaypoint> waypoints = {};
		m_PilotGroup.GetWaypoints(waypoints);
		bool pending = boarding && waypoints.Contains(m_PilotWaypoint);
		string diagnostic = "pending=" + pending + " behavior=" + ActionDescription(m_PilotUtility.GetCurrentBehavior());
		if (diagnostic != m_sLeadBoardingDiagnostic)
		{
			m_sLeadBoardingDiagnostic = diagnostic;
			Print("[ConvoyFollower] EXPLICIT_HOLD_LEAD_BOARD_HANDOVER: seconds=" + m_iTicks + " " + diagnostic);
		}
		if (pending)
			return false;
		// The exact boarding waypoint has finished naturally and is absent.
		// Forget its stale pointer so inherited StartDrive cannot fail it again.
		// RemoveObsoleteActions only prunes finished actions; no active GetIn
		// is cancelled by this gate.
		m_PilotUtility.RemoveObsoleteActions();
		if (boarding)
			m_PilotWaypoint = null;
		return true;
	}

	protected bool RetainedFollowerAndOwner()
	{
		if (!FixtureEntityAlive(m_Lead) || !FixtureEntityAlive(m_Follower) ||
			!FixtureEntityAlive(m_Player) || !FixtureEntityAlive(m_Pilot) || !FixtureEntityAlive(m_ExpectedDriver))
			return false;
		PlayerManager players = GetGame().GetPlayerManager();
		if (!players || players.GetPlayerControlledEntity(m_iOwnerId) != m_Player ||
			!m_Driver || !m_ExpectedDriver || m_Driver.CF_GetDriverEntity() != m_ExpectedDriver ||
			m_Driver.CF_GetAssignedVehicle() != m_Follower || !m_Driver.CF_IsActiveConvoyMember() ||
			!m_Driver.CF_IsBoarded() || m_Driver.CF_GetUnitNumber() != 1 ||
			CF_ConvoySession.GetForPlayer(m_Player) != m_ExpectedSession ||
			!m_ExpectedSession || !m_ExpectedSession.IsCurrentLeader(m_Driver))
			return false;
		BaseWorld world = GetGame().GetWorld();
		if (world.FindEntityByName("CF_SmokeLead") != m_Lead ||
			world.FindEntityByName("CF_SmokeFollower1") != m_Follower)
			return false;
		CarControllerComponent car = CarControllerComponent.Cast(m_Follower.FindComponent(CarControllerComponent));
		if (!car || !car.GetPilotCompartmentSlot() || car.GetPilotCompartmentSlot().GetOccupant() != m_ExpectedDriver)
			return false;
		string roster = CF_ConvoySession.CF_GetOwnerPanelSnapshot(m_Player);
		return roster.IndexOf("1|1|") == 0 && roster.IndexOf(";") == -1;
	}

	protected bool FixtureEntityAlive(IEntity entity)
	{
		if (!entity)
			return false;
		SCR_DamageManagerComponent damage = SCR_DamageManagerComponent.Cast(entity.FindComponent(SCR_DamageManagerComponent));
		return !damage || !damage.IsDestroyed();
	}

	protected bool SurveyFinalResumeExtension()
	{
		// A separate second extension gives both later moves ample room even
		// when the first native stop finishes close to its 80 m waypoint.
		vector originalStart = m_vRouteStart;
		m_vRouteStart = m_vRestartGoal;
		float clearRun;
		bool clear = SurveyCandidate(m_vRouteAxis, 40.0, 901, clearRun, 40.0);
		m_vRouteStart = originalStart;
		if (!clear)
			return false;
		m_vFinalResumeGoal = m_vRestartGoal + m_vRouteAxis * 40.0;
		m_vFinalResumeGoal[1] = GetGame().GetWorld().GetSurfaceY(m_vFinalResumeGoal[0], m_vFinalResumeGoal[2]);
		Print("[ConvoyFollower] EXPLICIT_HOLD_FINAL_SURVEY: goal=" + m_vFinalResumeGoal + " geometry_only=true");
		return true;
	}

	protected bool NativeLeadReady()
	{
		return IsFixtureSeat(m_Pilot, true) && IsFixtureSeat(m_Player, false) && ResolveLeadOwnership();
	}

	protected bool NativeLeadStopped()
	{
		if (!NativeLeadReady() || !m_LeadWaitBehavior ||
			m_PilotUtility.GetCurrentBehavior() != m_LeadWaitBehavior)
			return false;
		return Speed(m_Lead) <= 2.0 && vector.DistanceXZ(m_Lead.GetOrigin(), m_vPreviousLead) <= 0.3;
	}

	protected void BeginSeatTransfer(bool toOwner)
	{
		// Release our native lead overrides while the same AI still owns them.
		// Only fixture lead occupants move; the assigned follower is untouched.
		ReleaseLeadWait();
		ResetLeadCruiseOverride();
		ApplyLeadBrake();
		m_bTransferToOwner = toOwner;
		m_iTransferStep = 0;
		Print("[ConvoyFollower] EXPLICIT_HOLD_SEAT_SETUP: seconds=" + m_iTicks +
			" to_owner_pilot=" + toOwner + " vehicle=" + m_Lead.GetName() +
			" temporary_test_setup=true human_input_claim=false");
	}

	protected bool ExitFixtureLead(ChimeraCharacter character)
	{
		CompartmentAccessComponent access = character.GetCompartmentAccessComponent();
		if (!access || access.GetVehicleIn(character) != m_Lead ||
			!access.GetOutVehicle(EGetOutType.TELEPORT, -1, ECloseDoorAfterActions.CLOSE_DOOR, true, true))
		{
			Finish("FAIL test_lead_exit_rejected");
			return false;
		}
		Print("[ConvoyFollower] EXPLICIT_HOLD_SEAT_EXIT: seconds=" + m_iTicks +
			" owner=" + (character == m_Player) + " accepted=true temporary_test_setup=true");
		return true;
	}

	protected bool BoardFixtureLead(ChimeraCharacter character, bool pilot)
	{
		CompartmentAccessComponent access = character.GetCompartmentAccessComponent();
		BaseCompartmentManagerComponent manager = BaseCompartmentManagerComponent.Cast(m_Lead.FindComponent(BaseCompartmentManagerComponent));
		if (!access || !FixtureOnFoot(character) || !manager)
		{
			Finish("FAIL test_lead_boarding_precondition");
			return false;
		}
		ref array<BaseCompartmentSlot> slots = {};
		manager.GetCompartments(slots);
		foreach (BaseCompartmentSlot slot : slots)
		{
			if (!slot || slot.IsPiloting() != pilot || slot.IsOccupied() || slot.IsReserved())
				continue;
			if (!access.GetInVehicle(m_Lead, slot, true, -1, ECloseDoorAfterActions.CLOSE_DOOR, true))
				break;
			Print("[ConvoyFollower] EXPLICIT_HOLD_SEAT_BOARD: seconds=" + m_iTicks +
				" owner=" + (character == m_Player) + " pilot=" + pilot +
				" accepted=true temporary_test_setup=true");
			return true;
		}
		Finish("FAIL test_lead_boarding_rejected_or_seat_unavailable");
		return false;
	}

	protected bool PollSeatTransfer()
	{
		if (Speed(m_Lead) > 2.0)
		{
			Finish("FAIL lead_moved_during_test_seat_transfer");
			return false;
		}
		if (m_bTransferToOwner)
		{
			if (m_iTransferStep == 0)
			{
				if (!IsFixtureSeat(m_Pilot, true) || !IsFixtureSeat(m_Player, false))
					Finish("FAIL expected_native_lead_seats_missing");
				else if (ExitFixtureLead(m_Pilot))
					m_iTransferStep = 1;
			}
			else if (m_iTransferStep == 1 && FixtureOnFoot(m_Pilot))
			{
				if (ExitFixtureLead(m_Player))
					m_iTransferStep = 2;
			}
			else if (m_iTransferStep == 2 && FixtureOnFoot(m_Player))
			{
				if (BoardFixtureLead(m_Player, true))
					m_iTransferStep = 3;
			}
			return m_iTransferStep == 3 && IsFixtureSeat(m_Player, true) && FixtureOnFoot(m_Pilot);
		}
		if (m_iTransferStep == 0)
		{
			if (!IsFixtureSeat(m_Player, true) || m_Pilot.IsInVehicle())
				Finish("FAIL expected_owner_lead_seats_missing");
			else if (ExitFixtureLead(m_Player))
				m_iTransferStep = 1;
		}
		else if (m_iTransferStep == 1 && FixtureOnFoot(m_Player))
		{
			if (BoardFixtureLead(m_Player, false))
				m_iTransferStep = 2;
		}
		else if (m_iTransferStep == 2 && IsFixtureSeat(m_Player, false))
		{
			if (BoardFixtureLead(m_Pilot, true))
				m_iTransferStep = 3;
		}
		return m_iTransferStep == 3 && NativeLeadReady();
	}

	protected void ResetMotionSamples()
	{
		m_vLegLeadStart = m_Lead.GetOrigin();
		m_vLegFollowerStart = m_Follower.GetOrigin();
		m_vPreviousLead = m_vLegLeadStart;
		m_vPreviousFollower = m_vLegFollowerStart;
		m_iLeadPoweredSamples = 0;
		m_iFollowerPoweredSamples = 0;
		m_iLeadMovingSamples = 0;
		m_iFollowerMovingSamples = 0;
		m_fLeadProgress = 0;
		m_fFollowerProgress = 0;
	}

	protected bool MoveNativeLead(vector goal)
	{
		if (!NativeLeadReady())
			return false;
		Resource prefab = Resource.Load("{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et");
		if (!prefab.IsValid())
			return false;
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = goal;
		SCR_AIWaypoint waypoint = SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(prefab, null, params));
		if (!waypoint)
			return false;
		ReleaseLeadWait();
		ResetLeadCruiseOverride();
		if (m_PilotWaypoint)
			m_PilotGroup.RemoveWaypoint(m_PilotWaypoint);
		waypoint.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_GAMEMASTER);
		waypoint.SetCompletionRadius(5.0);
		m_PilotWaypoint = waypoint;
		m_PilotGroup.AddWaypoint(waypoint);
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		car.SetPersistentHandBrake(false);
		car.GetSimulation().SetBreak(0, false);
		m_bHoldLead = false;
		ResetMotionSamples();
		Print("[ConvoyFollower] EXPLICIT_HOLD_LEAD_MOVE: seconds=" + m_iTicks + " goal=" + goal + " native=true");
		return true;
	}

	protected bool SampleMotion()
	{
		for (int index = 0; index < 2; index++)
		{
			Vehicle truck = m_Lead;
			vector previous = m_vPreviousLead;
			vector start = m_vLegLeadStart;
			if (index == 1)
			{
				truck = m_Follower;
				previous = m_vPreviousFollower;
				start = m_vLegFollowerStart;
			}
			CarControllerComponent car = CarControllerComponent.Cast(truck.FindComponent(CarControllerComponent));
			if (!car || !car.GetSimulation() || vector.DistanceXZ(truck.GetOrigin(), previous) > 25.0)
			{
				Finish("FAIL simulation_missing_or_position_discontinuity");
				return false;
			}
			VehicleWheeledSimulation sim = car.GetSimulation();
			vector routeOffset = truck.GetOrigin() - m_vRouteStart;
			float lateral = -routeOffset[0] * m_vRouteAxis[2] + routeOffset[2] * m_vRouteAxis[0];
			if (Math.AbsFloat(lateral) > 14.0)
			{
				Finish("FAIL truck_left_surveyed_corridor");
				return false;
			}
			float step = SignedProgress(truck.GetOrigin(), previous);
			float progress = SignedProgress(truck.GetOrigin(), start);
			bool moving = step >= 0.25;
			bool powered = moving && Speed(truck) >= 1.0 && sim.EngineIsOn() && sim.GetThrottle() > 0.05 && sim.GetGear() >= 2;
			if (index == 0)
			{
				m_fLeadProgress = progress;
				if (moving) m_iLeadMovingSamples++;
				if (powered) m_iLeadPoweredSamples++;
			}
			else
			{
				m_fFollowerProgress = progress;
				if (moving) m_iFollowerMovingSamples++;
				if (powered) m_iFollowerPoweredSamples++;
			}
			Print("[ConvoyFollower] EXPLICIT_HOLD_MOTION: phase=" + m_iExplicitPhase + " seconds=" + m_iTicks +
				" vehicle=" + index + " origin=" + truck.GetOrigin() + " progress_m=" + progress +
				" signed_step_m=" + step + " speed_kmh=" + sim.GetSpeedKmh() + " throttle=" + sim.GetThrottle() +
				" brake=" + sim.GetBrake() + " engine=" + sim.EngineIsOn() + " gear=" + sim.GetGear() +
				" powered=" + powered + " lateral_m=" + lateral);
		}
		return true;
	}

	override protected void Finish(string result)
	{
		if (m_bFinished)
			return;
		m_bFinished = true;
		if (!m_bHoldLead && NativeLeadReady())
			BeginLeadHold();
		Print("[ConvoyFollower] EXPLICIT_HOLD_FINAL: seconds=" + m_iTicks + " phase=" + m_iExplicitPhase +
			" hold_accepted=" + m_bHoldCommandAccepted + " resume_accepted=" + m_bResumeCommandAccepted +
			" max_hold_drift_m=" + m_fHoldMaxDrift + " max_hold_speed_kmh=" + m_fHoldMaxSpeed +
			" lead_progress_m=" + m_fLeadProgress + " follower_progress_m=" + m_fFollowerProgress +
			" lead_powered_samples=" + m_iLeadPoweredSamples + " follower_powered_samples=" + m_iFollowerPoweredSamples);
		if (m_Player)
			Print("[ConvoyFollower] EXPLICIT_HOLD_ROSTER: " + CF_ConvoySession.CF_GetOwnerPanelSnapshot(m_Player));
		Print("[ConvoyFollower] EXPLICIT_HOLD_RESULT: " + result);
		GetGame().GetCallqueue().Remove(Poll);
	}

	override protected void Poll()
	{
		if (m_bFinished || CF_ConvoySession.CF_IsWorldCleanup())
			return;
		m_iTicks++;
		if (m_iTicks > 480 || m_iTicks - m_iPhaseStart > 90)
		{
			Finish("FAIL phase_or_total_timeout");
			return;
		}
		if (m_iExplicitPhase == 0)
		{
			if (!Resolve() || !EnsureOwnerPassenger() || !EnsurePilot())
				return;
			if (!SelectRoute() || !SurveyRestartExtension() || !SurveyFinalResumeExtension())
			{
				Finish("FAIL fixed_route_or_extension_survey");
				return;
			}
			m_iOwnerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(m_Player);
			m_ExpectedDriver = m_Driver.CF_GetDriverEntity();
			m_bOrderSent = CF_ConvoySession.Start(m_Player, m_Driver);
			Print("[ConvoyFollower] EXPLICIT_HOLD_RECRUIT: accepted=" + m_bOrderSent);
			if (!m_bOrderSent)
				Finish("FAIL production_recruit_rejected");
			else
				SetPhase(1);
			return;
		}
		if (m_iExplicitPhase == 1)
		{
			if (!m_Driver || m_Driver.CF_GetDriverEntity() != m_ExpectedDriver || !m_Lead || !m_Follower)
			{
				Finish("FAIL initial_identity_missing");
				return;
			}
			if (!m_Driver.CF_IsActiveConvoyMember())
				return;
			if (!InitialLeadBoardingHandoverReady())
				return;
			m_ExpectedSession = CF_ConvoySession.GetForPlayer(m_Player);
			if (!RetainedFollowerAndOwner() || !StartDrive())
			{
				Finish("FAIL initial_assignment_or_native_drive");
				return;
			}
			m_iStage = 2; // disables inherited staging-only frame brake writes
			ResetMotionSamples();
			SetPhase(2);
			return;
		}
		if (!m_Lead || !m_Follower || !m_Player || !m_Pilot || !RetainedFollowerAndOwner())
		{
			Finish("FAIL follower_seat_assignment_owner_or_roster_identity_lost");
			return;
		}
		if (m_bMeasureFollowerHold && (!m_Driver.CF_HasPanelHoldRequest() || !m_Driver.CF_IsPanelHeld()))
		{
			Finish("FAIL explicit_hold_intent_or_state_lost_without_resume");
			return;
		}
		if (m_iExplicitPhase == 4 || m_iExplicitPhase == 6 || m_iExplicitPhase == 8 || m_iExplicitPhase == 9)
		{
			if (m_iTicks - m_iPhaseStart > 30)
			{
				Finish("FAIL test_seat_transfer_timeout");
				return;
			}
			if (!PollSeatTransfer())
				return;
			if (m_iExplicitPhase == 4)
			{
				m_bHoldCommandAccepted = CF_ConvoySession.CF_PanelHold(m_Player);
				Print("[ConvoyFollower] EXPLICIT_HOLD_COMMAND: accepted=" + m_bHoldCommandAccepted +
					" owner_pilot=" + IsFixtureSeat(m_Player, true) + " state=" + CF_ConvoySession.CF_GetOwnerPanelOrderState(m_Player));
				if (!m_bHoldCommandAccepted || !m_Driver.CF_HasPanelHoldRequest())
					Finish("FAIL real_server_hold_rejected_or_not_applied");
				else
					SetPhase(5);
			}
			else if (m_iExplicitPhase == 8)
			{
				m_bResumeCommandAccepted = CF_ConvoySession.CF_PanelResume(m_Player);
				Print("[ConvoyFollower] EXPLICIT_RESUME_COMMAND: accepted=" + m_bResumeCommandAccepted +
					" owner_pilot=" + IsFixtureSeat(m_Player, true) + " state=" + CF_ConvoySession.CF_GetOwnerPanelOrderState(m_Player));
				if (!m_bResumeCommandAccepted || m_Driver.CF_HasPanelHoldRequest() || !m_Driver.CF_IsMovementActive())
					Finish("FAIL real_server_resume_rejected_or_not_applied");
				else
				{
					m_bMeasureFollowerHold = false;
					SetPhase(9);
					BeginSeatTransfer(false);
				}
			}
			else
			{
				vector goal = m_vRestartGoal;
				int nextPhase = 7;
				if (m_iExplicitPhase == 9)
				{
					goal = m_vFinalResumeGoal;
					nextPhase = 10;
				}
				if (!MoveNativeLead(goal))
					Finish("FAIL native_lead_move_after_seat_setup");
				else
				{
					if (nextPhase == 7)
						m_fHoldWindowStartMs = GetGame().GetWorld().GetWorldTime();
					SetPhase(nextPhase);
				}
			}
			return;
		}
		if (m_iExplicitPhase == 5)
		{
			if (!IsFixtureSeat(m_Player, true) || m_Pilot.IsInVehicle())
			{
				Finish("FAIL hold_command_owner_pilot_changed");
				return;
			}
			if (m_Driver.CF_IsPanelHeld() && m_Driver.CF_IsPanelVehicleSlow(2.0))
				m_iStableSamples++;
			else
				m_iStableSamples = 0;
			if (m_iStableSamples >= 3)
			{
				m_vHeldFollower = m_Follower.GetOrigin();
				m_bMeasureFollowerHold = true;
				Print("[ConvoyFollower] EXPLICIT_HOLD_CAPTURE: seconds=" + m_iTicks + " origin=" + m_vHeldFollower);
				SetPhase(6);
				BeginSeatTransfer(false);
			}
			return;
		}
		if (!NativeLeadReady() || !SampleMotion())
		{
			if (!m_bFinished) Finish("FAIL native_lead_ownership_or_seat_lost");
			return;
		}
		if (m_iExplicitPhase == 2)
		{
			if (m_fLeadProgress >= 40.0 && m_fFollowerProgress >= 20.0 &&
				m_iLeadPoweredSamples >= 2 && m_iFollowerPoweredSamples >= 2)
			{
				if (!BeginLeadHold())
					Finish("FAIL initial_native_lead_hold");
				else
					SetPhase(3);
			}
		}
		else if (m_iExplicitPhase == 3)
		{
			if (NativeLeadStopped()) m_iStableSamples++;
			else m_iStableSamples = 0;
			if (m_iStableSamples >= 3)
			{
				SetPhase(4);
				BeginSeatTransfer(true);
			}
		}
		else if (m_iExplicitPhase == 7)
		{
			float heldSeconds = (GetGame().GetWorld().GetWorldTime() - m_fHoldWindowStartMs) / 1000.0;
			Print("[ConvoyFollower] EXPLICIT_HOLD_OBSERVE: seconds=" + m_iTicks + " held_s=" + heldSeconds +
				" max_drift_m=" + m_fHoldMaxDrift + " max_speed_kmh=" + m_fHoldMaxSpeed +
				" lead_progress_m=" + m_fLeadProgress + " hold_requested=" + m_Driver.CF_HasPanelHoldRequest());
			if (!m_bAdvanceStopped && (m_fLeadProgress >= 25.0 || vector.DistanceXZ(m_Lead.GetOrigin(), m_vRestartGoal) <= 8.0))
			{
				if (!BeginLeadHold())
					Finish("FAIL hold_advance_native_lead_stop");
				else
					m_bAdvanceStopped = true;
			}
			if (m_bAdvanceStopped && NativeLeadStopped()) m_iStableSamples++;
			else m_iStableSamples = 0;
			if (heldSeconds >= 30.0 && m_iStableSamples >= 3 && m_fLeadProgress >= 20.0 && m_iLeadPoweredSamples >= 2)
			{
				Print("[ConvoyFollower] EXPLICIT_HOLD_VERIFIED: held_s=" + heldSeconds + " lead_progress_m=" + m_fLeadProgress +
					" max_drift_m=" + m_fHoldMaxDrift + " same_driver_truck_roster=true");
				SetPhase(8);
				BeginSeatTransfer(true);
			}
		}
		else if (m_iExplicitPhase == 10)
		{
			if (m_Driver.CF_HasPanelHoldRequest())
				Finish("FAIL explicit_hold_returned_after_resume");
			else if (m_fLeadProgress >= 20.0 && m_fFollowerProgress >= 15.0 &&
				m_iLeadPoweredSamples >= 2 && m_iFollowerPoweredSamples >= 2 &&
				m_iLeadMovingSamples >= 3 && m_iFollowerMovingSamples >= 3)
			{
				Print("[ConvoyFollower] EXPLICIT_RESUME_VERIFIED: lead_progress_m=" + m_fLeadProgress +
					" follower_progress_m=" + m_fFollowerProgress + " same_driver_truck_roster=true");
				if (!BeginLeadHold()) Finish("FAIL final_native_lead_stop");
				else SetPhase(11);
			}
		}
		else if (m_iExplicitPhase == 11)
		{
			if (NativeLeadStopped() && Speed(m_Follower) <= 2.0 &&
				vector.DistanceXZ(m_Follower.GetOrigin(), m_vPreviousFollower) <= 0.3)
				m_iStableSamples++;
			else m_iStableSamples = 0;
			if (m_iStableSamples >= 3)
				Finish("PASS real_server_hold_30s_lead_advance_then_resume_powered_progress");
		}
		m_vPreviousLead = m_Lead.GetOrigin();
		m_vPreviousFollower = m_Follower.GetOrigin();
	}
}
