class CF_ActionInputProbeComponentClass : CF_PanelPilotProbeComponentClass
{
}

// Isolated player-pilot action calibration. Setup reuses the map-free owner
// fixture; no followers, input bindings, context forcing or physics writes.
class CF_ActionInputProbeComponent : CF_PanelPilotProbeComponent
{
	[Attribute(defvalue: "0", desc: "Enable the private callback action-input calibration")]
	protected bool m_bActionCalibrationEnabled;
	[Attribute(defvalue: "0", desc: "Offline client only: request native close 30 seconds after the result; never Workbench")]
	protected bool m_bActionAutoExit;
	protected CF_ActionInputCharacterController m_ActionController;
	protected World m_ActionWorld;
	protected string m_sActionWorldFile;
	protected string m_sActionRun;
	protected string m_sActionFailure;
	protected int m_iActionPhase;
	protected int m_iActionCalls;
	protected int m_iPhaseCalls;
	protected int m_iApplyCalls;
	protected int m_iDriveWrites;
	protected int m_iDriveReadbacks;
	protected int m_iReleasedNeutralCalls;
	protected int m_iPhysicsStep;
	protected int m_iPairedPhysicsSteps;
	protected int m_iPoweredPhysicsSteps;
	protected int m_iPhysicsLogs;
	protected int m_iHookLogs;
	protected int m_iUnpairedPhysicsSteps;
	protected int m_iPrePhase;
	protected int m_iPreActionCall;
	protected int m_iActionEpoch;
	protected int m_iPreEpoch;
	protected int m_iPostPhysicsCalls;
	protected int m_iDriveReadbackFaults;
	protected int m_iPreviousPostReadbackFaults;
	protected int m_iPreviousPostStep;
	protected int m_iPreviousPostEpoch;
	protected float m_fPrePhysicsMs;
	protected float m_fPrePhysicsTimeSlice;
	protected vector m_vPreBodyPosition;
	protected vector m_vPreviousPostBodyPosition;
	protected bool m_bPreviousPostEligible;
	protected int m_iPhysicsActiveEvents;
	protected float m_fLastPhysicsActiveMs;
	protected bool m_bLastPhysicsActive;
	protected bool m_bPrePhysicsActive;
	protected float m_fPreSpeedKmh;
	protected int m_iAllActionWrites;
	protected int m_iLastActionObservationCall;
	protected int m_iLastActionObservationEpoch;
	protected float m_fLastActionObservationMs;
	protected bool m_bLastActionObservationNeutral;
	protected int m_iBaselineSuspendedPre;
	protected int m_iRawUnfinishedPre;
	protected int m_iUnexpectedPostCallbacks;
	protected float m_fActionStartMs;
	protected float m_fPhaseStartMs;
	protected float m_fLastHookLogMs;
	protected float m_fLastPhysicsLogMs;
	protected float m_fEngineRequestMs;
	protected float m_fDriveProgress;
	protected float m_fDriveElevation;
	protected float m_fPoweredProgress;
	protected float m_fTerminalMs;
	protected float m_fPreThrottle;
	protected float m_fPreClutch;
	protected float m_fPreBrake;
	protected int m_iPreGear;
	protected bool m_bPreEngine;
	protected bool m_bAwaitPostPhysics;
	protected bool m_bLogPhysicsPair;
	protected bool m_bEngineRequested;
	protected bool m_bPrivateSpawnAttempted;
	protected bool m_bActionTerminal;
	protected bool m_bActionDeleting;
	protected bool m_bActionExitPending;
	protected bool m_bActionExitRequested;
	protected int m_iActionExitDeferrals;
	protected int m_iReadinessLogs;
	protected float m_fNextReadinessLogMs;
	protected float m_fBaselineReadyMs;
	protected bool m_bLogReadinessPair;
	protected bool m_bBaselineReady;
	protected vector m_vActionStart;
	protected vector m_vActionForward;
	protected vector m_vPrePhysicsPosition;
	protected static const ResourceName ACTION_CREW = "{B4C701EAAA4E2F60}Prefabs/Tests/CF_ActionInputCrew.et";
	protected static const int ACTION_EXIT_DELAY_MS = 30000;
	protected static const int ACTION_EXIT_RETRY_MS = 100;
	protected static const int ACTION_EXIT_MAX_DEFERRALS = 1;

	override void OnPostInit(IEntity owner)
	{
		if (!m_bActionCalibrationEnabled || !Replication.IsServer() || !GetGame() ||
			!GetGame().GetWorld() || System.IsConsoleApp() || RplSession.Mode() != RplMode.None)
			return;
		m_ActionWorld = GetGame().GetWorld();
		m_sActionWorldFile = GetGame().GetWorldFile();
		m_fActionStartMs = m_ActionWorld.GetWorldTime();
		m_sActionRun = owner.GetID().ToString() + "_" + m_fActionStartMs;
		m_iFollowerCount = 0;
		m_bAutoOpenMap = false;
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.POSTFRAME | EntityEvent.SIMULATE | EntityEvent.POSTSIMULATE | EntityEvent.PHYSICSACTIVE);
		Print("[ConvoyFollower] ACTION_INPUT_INIT: run_id=" + m_sActionRun + " world=" + m_sActionWorldFile +
			" enabled=true followers=0 setup_timeout_s=45 total_timeout_s=90 baseline_s=2 baseline_calls=30" +
			" thrust=0.6 drive_max_s=8 drive_max_progress_m=20 min_progress_m=10 min_elevation_m=0.25" +
			" neutral_write_s=0.5 released_observation_s=3 readiness_timeout_s=10 readiness_contiguous=true" +
			" readiness_mode=callback_action_active context_diagnostic_only=true" +
			" powered_metric=consecutive_completed_post_direct_body within_pair_diagnostic_only=true unexpected_unmatched_fails=true" +
			" physics_identity=original_entity_each_callback physics_body_continuity_claim=false" +
			" physics_active_observer=true active_detail_limit=32" +
			" pairing_policy=owned_neutral_baseline_sleep_else_complete_pairs suspended_baseline_credit=false" +
			" human_input_claim=false physics_writes=false auto_exit=" + m_bActionAutoExit);
	}

	protected bool ActionWorldAlive()
	{
		return m_bActionCalibrationEnabled && !m_bActionDeleting && GetGame() &&
			GetGame().GetWorld() == m_ActionWorld && m_ActionWorld &&
			!CF_ConvoySession.CF_IsWorldCleanup() && GetOwner() == m_Lead;
	}

	protected VehicleWheeledSimulation ActionSimulation()
	{
		if (!m_Lead)
			return null;
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (!car)
			return null;
		return car.GetSimulation();
	}

	protected float ActionProgress()
	{
		vector delta = m_Lead.GetOrigin() - m_vActionStart;
		return delta[0] * m_vActionForward[0] + delta[2] * m_vActionForward[2];
	}

	protected void ActionPhase(int phase)
	{
		m_iActionEpoch++;
		m_bPreviousPostEligible = false;
		m_iActionPhase = phase;
		m_iPhaseCalls = 0;
		m_fPhaseStartMs = m_ActionWorld.GetWorldTime();
		string record = "[ConvoyFollower] ACTION_INPUT_PHASE: run_id=" + m_sActionRun + " phase=" + phase;
		record += " world_ms=" + m_fPhaseStartMs + " callbacks=" + m_iActionCalls + " apply_callbacks=" + m_iApplyCalls;
		record += ActionPhysicsActiveFields();
		Print(record);
	}

	protected void FailAction(string reason)
	{
		if (m_sActionFailure.IsEmpty())
			m_sActionFailure = reason;
		FinishAction();
	}

	override protected bool EnsurePlayer()
	{
		PlayerController local = GetGame().GetPlayerController();
		PlayerManager players = GetGame().GetPlayerManager();
		if (!local || !players || local.GetPlayerId() <= 0)
			return false;
		if (m_Player)
			return local.GetControlledEntity() == m_Player && players.GetPlayerIdFromControlledEntity(m_Player) == local.GetPlayerId();
		if (ChimeraCharacter.Cast(local.GetControlledEntity()))
		{
			FailAction("preexisting_player_character_not_owned_by_fixture");
			return false;
		}
		if (m_bPrivateSpawnAttempted)
			return false;
		m_bPrivateSpawnAttempted = true;
		Resource prefab = Resource.Load(ACTION_CREW);
		if (!prefab.IsValid())
		{
			FailAction("private_character_resource_unavailable");
			return false;
		}
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = m_Lead.GetOrigin() + Vector(3, 0, 2);
		m_Player = ChimeraCharacter.Cast(GetGame().SpawnEntityPrefab(prefab, m_ActionWorld, params));
		if (m_Player)
			m_ActionController = CF_ActionInputCharacterController.Cast(m_Player.GetCharacterController());
		if (!m_Player || !m_ActionController)
		{
			FailAction("private_controller_replacement_not_instantiated");
			return false;
		}
		if (!m_ActionController.CF_BindActionProbe(this, m_Lead) || !local.SetControlledEntity(m_Player))
		{
			FailAction("private_character_control_binding_failed");
			return false;
		}
		Print("[ConvoyFollower] ACTION_INPUT_PLAYER: run_id=" + m_sActionRun + " player=" + m_Player.GetID() +
			" truck=" + m_Lead.GetID() + " controller=" + m_ActionController.Type().ToString() +
			" source=test_setup_SetControlledEntity human_input_claim=false");
		return players.GetPlayerIdFromControlledEntity(m_Player) == local.GetPlayerId();
	}

	override protected void Poll()
	{
		if (!ActionWorldAlive() || m_bActionTerminal)
			return;
		float now = m_ActionWorld.GetWorldTime();
		if (now - m_fActionStartMs >= 90000)
		{
			FailAction("total_90s_timeout");
			return;
		}
		if (m_iActionPhase == 0)
		{
			if (now - m_fActionStartMs >= 45000)
			{
				FailAction("setup_45s_timeout");
				return;
			}
			if (!m_bReady)
				super.Poll();
			else
				SelectLocalPlayerCamera();
			if (m_bActionTerminal || !m_bReady || !m_ActionController ||
				!m_ActionController.CF_IsExactActionPilot(m_Player, true) || !m_bCameraSelected)
				return;
			VehicleWheeledSimulation sim = ActionSimulation();
			if (!sim)
			{
				FailAction("vehicle_simulation_missing");
				return;
			}
			if (!sim.EngineIsOn())
			{
				if (!m_bEngineRequested)
				{
					m_bEngineRequested = true;
					m_fEngineRequestMs = now;
					CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
					bool running = car.StartEngine();
					Print("[ConvoyFollower] ACTION_INPUT_ENGINE_SETUP: run_id=" + m_sActionRun +
						" ordinary_native_start_requested=true returned_running=" + running + " action_delivery_claim=false");
				}
				else if (now - m_fEngineRequestMs > 10000)
					FailAction("native_engine_setup_timeout");
				return;
			}
			if (Math.AbsFloat(sim.GetSpeedKmh()) > 2.0)
			{
				FailAction("setup_truck_not_stationary");
				return;
			}
			m_vActionStart = m_Lead.GetOrigin();
			m_vActionForward = m_Lead.GetWorldTransformAxis(2);
			m_vActionForward[1] = 0;
			if (m_vActionForward.LengthSq() < 0.5)
			{
				FailAction("invalid_initial_heading");
				return;
			}
			m_vActionForward.Normalize();
			ActionPhase(1);
			return;
		}
		if (!m_ActionController.CF_IsExactActionPilot(m_Player, true))
		{
			FailAction("original_player_pilot_identity_lost");
			return;
		}
		if (m_iActionPhase == 1 && now - m_fPhaseStartMs > 10000)
			FailAction("seated_prepare_callback_calibration_timeout");
		else if (m_iActionPhase == 2 && now - m_fPhaseStartMs > 10000)
			FailAction("drive_callback_timeout");
		else if (m_iActionPhase >= 3 && now - m_fPhaseStartMs > 10000)
			FailAction("neutral_callback_timeout");
	}

	protected void ResetActionBaselineReadiness()
	{
		m_bBaselineReady = false;
		m_fBaselineReadyMs = 0;
		m_iPhaseCalls = 0;
	}

	// Observe the callback's actual readiness before and after native controls.
	// The global input manager is diagnostic only; no context/disable flag is set.
	// This isolated comparison requires the callback action; CarContext stays diagnostic.
	// Its false value in v2 did not establish the native car-consumption timing.
	protected bool ActionControlsReady(CF_ActionInputCharacterController controller, ActionManager am, string checkpoint)
	{
		MenuManager menus = GetGame().GetMenuManager();
		bool movementDisabled = controller.GetDisableMovementControls();
		bool viewDisabled = controller.GetDisableViewControls();
		bool menuOpen = menus && menus.IsAnyMenuOpen();
		bool editorOpen = SCR_EditorManagerEntity.IsOpenedInstance();
		bool carContext = am.IsContextActive("CarContext");
		bool thrustActive = am.IsActionActive("CarThrust");
		bool ready = !movementDisabled && !viewDisabled && !menuOpen && !editorOpen && thrustActive;
		if (m_bLogReadinessPair && m_iReadinessLogs < 48)
		{
			m_iReadinessLogs++;
			MenuBase topMenu;
			if (menus)
				topMenu = menus.GetTopMenu();
			InputManager input = GetGame().GetInputManager();
			string record = "[ConvoyFollower] ACTION_INPUT_READINESS: run_id=" + m_sActionRun;
			record += " world_ms=" + m_ActionWorld.GetWorldTime() + " sequence=" + m_iActionCalls;
			record += " phase=" + m_iActionPhase + " checkpoint=" + checkpoint + " ready=" + ready;
			record += " movement_disabled=" + movementDisabled + " view_disabled=" + viewDisabled;
			record += " any_menu_open=" + menuOpen + " top_menu=" + topMenu + " editor_open=" + editorOpen;
			record += " callback_car_context=" + carContext + " callback_thrust_active=" + thrustActive;
			record += " callback_thrust=" + am.GetActionValue("CarThrust") + " callback_brake=" + am.GetActionValue("CarBrake");
			record += " global_manager=" + input + " global_car_context=" + (input && input.IsContextActive("CarContext"));
			record += " global_thrust_active=" + (input && input.IsActionActive("CarThrust"));
			record += " baseline_ready=" + m_bBaselineReady + " baseline_ready_since_ms=" + m_fBaselineReadyMs;
			record += " baseline_calls=" + m_iPhaseCalls + " forced_context=false";
			Print(record);
		}
		return ready;
	}

	// Return -1 to observe only. The private controller owns all action writes.
	float CF_BeforeActionControls(CF_ActionInputCharacterController controller, IEntity owner, ActionManager am, float dt, bool player)
	{
		if (!ActionWorldAlive() || m_bActionTerminal || m_iActionPhase == 0)
			return -1;
		m_iActionCalls++;
		if (controller != m_ActionController || owner != m_Player || !am ||
			!controller.CF_IsExactActionPilot(owner, player))
		{
			FailAction("prepare_callback_identity_or_manager_invalid");
			return -1;
		}
		float now = m_ActionWorld.GetWorldTime();
		m_bLogReadinessPair = m_iReadinessLogs < 48 && (m_iActionCalls <= 2 || now >= m_fNextReadinessLogMs);
		if (m_bLogReadinessPair)
			m_fNextReadinessLogMs = now + 500;
		if (!ActionControlsReady(controller, am, "before_super"))
		{
			if (m_iActionPhase == 1)
			{
				ResetActionBaselineReadiness();
				return -1;
			}
			FailAction("prepare_callback_controls_or_action_unavailable");
			return -1;
		}
		m_iPhaseCalls++;
		if (m_iActionPhase == 1)
		{
			if (Math.AbsFloat(am.GetActionValue("CarThrust")) > 0.01 || Math.AbsFloat(am.GetActionValue("CarBrake")) > 0.01)
			{
				FailAction("baseline_action_not_neutral");
				return -1;
			}
			if (!m_bBaselineReady)
			{
				m_bBaselineReady = true;
				m_fBaselineReadyMs = now;
			}
			if (now - m_fBaselineReadyMs >= 2000 && m_iPhaseCalls >= 30)
			{
				m_vActionStart = m_Lead.GetOrigin();
				ActionPhase(2);
			}
		}
		if (m_iActionPhase == 2)
		{
			if (now - m_fPhaseStartMs < 8000 && ActionProgress() < 20.0)
				return 0.6;
			m_fDriveProgress = ActionProgress();
			vector driveEnd = m_Lead.GetOrigin();
			m_fDriveElevation = driveEnd[1] - m_vActionStart[1];
			ActionPhase(3);
		}
		if (m_iActionPhase == 3)
		{
			if (now - m_fPhaseStartMs < 500 || m_iPhaseCalls < 2)
				return 0;
			ActionPhase(4);
		}
		return -1;
	}

	void CF_AfterActionControls(CF_ActionInputCharacterController controller, IEntity owner, ActionManager am,
		bool player, float before, float requested, float afterSet, bool wrote)
	{
		if (!ActionWorldAlive() || m_bActionTerminal || m_iActionPhase == 0 || !am)
			return;
		if (controller != m_ActionController || owner != m_Player || !controller.CF_IsExactActionPilot(owner, player))
		{
			FailAction("original_pilot_changed_during_native_controls");
			return;
		}
		if (wrote)
			m_iAllActionWrites++;
		if (!ActionControlsReady(controller, am, "after_super"))
		{
			if (m_iActionPhase == 1)
			{
				ResetActionBaselineReadiness();
				return;
			}
			FailAction("controls_or_action_lost_during_native_controls");
			return;
		}
		float afterSuper = am.GetActionValue("CarThrust");
		m_iLastActionObservationCall = m_iActionCalls;
		m_iLastActionObservationEpoch = m_iActionEpoch;
		m_fLastActionObservationMs = m_ActionWorld.GetWorldTime();
		m_bLastActionObservationNeutral = !wrote && requested < 0;
		if (!(Math.AbsFloat(before) <= 0.01 && Math.AbsFloat(afterSet) <= 0.01 && Math.AbsFloat(afterSuper) <= 0.01))
			m_bLastActionObservationNeutral = false;
		if (!(Math.AbsFloat(am.GetActionValue("CarBrake")) <= 0.01))
			m_bLastActionObservationNeutral = false;
		if (m_iActionPhase == 2 && wrote && requested > 0)
		{
			m_iDriveWrites++;
			if (Math.AbsFloat(afterSet - requested) <= 0.01 && Math.AbsFloat(afterSuper - requested) <= 0.01)
				m_iDriveReadbacks++;
			else
				m_iDriveReadbackFaults++;
		}
		else if (m_iActionPhase == 2)
			m_iDriveReadbackFaults++;
		if (m_iActionPhase == 4)
		{
			if (wrote || Math.AbsFloat(before) > 0.01 || Math.AbsFloat(afterSuper) > 0.01)
			{
				FailAction("released_action_not_neutral");
				return;
			}
			m_iReleasedNeutralCalls++;
		}
		float now = m_ActionWorld.GetWorldTime();
		if (m_iHookLogs < 96 && (m_iPhaseCalls <= 2 || now - m_fLastHookLogMs >= 500))
		{
			m_iHookLogs++;
			m_fLastHookLogMs = now;
			Print("[ConvoyFollower] ACTION_INPUT_HOOK: run_id=" + m_sActionRun + " world_ms=" + now +
				" sequence=" + m_iActionCalls + " phase=" + m_iActionPhase + " player=" + player +
				" exact_pilot=" + controller.CF_IsExactActionPilot(owner, player) + " manager=" + am +
				" before=" + before + " requested=" + requested + " after_set=" + afterSet +
				" after_super=" + afterSuper + " wrote=" + wrote + " applies=" + m_iApplyCalls);
		}
		if (m_iActionPhase == 4 && now - m_fPhaseStartMs >= 3000 && m_iReleasedNeutralCalls >= 30)
		{
			if (m_iDriveWrites < 30 || m_iDriveReadbacks < 30)
				m_sActionFailure = "insufficient_action_write_readback_evidence";
			else if (m_iPairedPhysicsSteps < 30 || m_iPoweredPhysicsSteps < 20 || m_fPoweredProgress < 5.0)
				m_sActionFailure = "insufficient_powered_physics_evidence";
			else if (m_fDriveProgress < 10.0 || m_fDriveElevation < 0.25)
				m_sActionFailure = "insufficient_signed_uphill_drive_progress";
			else if (m_iUnpairedPhysicsSteps > 0)
				m_sActionFailure = "physics_pairing_gap";
			FinishAction();
		}
	}

	void CF_ObserveAppliedControls(IEntity owner)
	{
		if (ActionWorldAlive() && !m_bActionTerminal && owner == m_Player && m_iActionPhase > 0)
			m_iApplyCalls++;
	}

	// Suppress the parent's broad input observer; this fixture has its own
	// bounded callback/physics evidence and never installs action listeners.
	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
	}

	// Fresh local-only physics evidence; no body identity is retained.
	protected string ActionPhysicsActiveFields()
	{
		if (!ActionWorldAlive())
			return " physics_query=original_world_unavailable";
		Physics body = m_Lead.GetPhysics();
		string record = " physics_present=" + (body != null);
		if (body)
			record += " physics_active=" + body.IsActive();
		record += " active_event_count=" + m_iPhysicsActiveEvents;
		if (m_iPhysicsActiveEvents > 0)
		{
			record += " last_active_event_ms=" + m_fLastPhysicsActiveMs;
			record += " last_active_event_state=" + m_bLastPhysicsActive;
		}
		return record;
	}

	override void EOnPhysicsActive(IEntity owner, bool activeState)
	{
		if (!ActionWorldAlive() || m_bActionTerminal || owner != m_Lead)
			return;
		m_iPhysicsActiveEvents++;
		m_fLastPhysicsActiveMs = m_ActionWorld.GetWorldTime();
		m_bLastPhysicsActive = activeState;
		if (!activeState)
			TrySuspendNeutralBaselinePre(owner);
		if (m_iPhysicsActiveEvents > 32)
		{
			if (m_iPhysicsActiveEvents == 33)
				Print("[ConvoyFollower] ACTION_INPUT_PHYSICS_ACTIVE_LIMIT: run_id=" + m_sActionRun +
					" detailed_records=32 aggregate_continues=true physics_writes=false");
			return;
		}
		string record = "[ConvoyFollower] ACTION_INPUT_PHYSICS_ACTIVE: run_id=" + m_sActionRun;
		record += " world_ms=" + m_fLastPhysicsActiveMs + " active_state=" + activeState;
		record += " truck=" + owner.GetID() + " original_owner=true phase=" + m_iActionPhase + " epoch=" + m_iActionEpoch;
		record += " before_ordinal=" + m_iPhysicsStep + " after_ordinal=" + m_iPostPhysicsCalls;
		record += " pending=" + m_bAwaitPostPhysics + " previous_before_ms=" + m_fPrePhysicsMs;
		record += " previous_before_phase=" + m_iPrePhase + " previous_before_active=" + m_bPrePhysicsActive;
		record += ActionPhysicsActiveFields() + " physics_writes=false";
		Print(record);
	}

	// Prospective v6 accounting only: an observed sleep event may close a
	// fresh, stationary, never-actuated baseline PRE without crediting a pair.
	// A later PRE discovering an unobserved gap never uses this classification.
	protected void TrySuspendNeutralBaselinePre(IEntity owner)
	{
		if (!ActionWorldAlive() || m_bActionTerminal || owner != m_Lead || !m_bAwaitPostPhysics)
			return;
		if (m_iActionPhase != 1 || m_iPrePhase != 1 || m_iPreEpoch != m_iActionEpoch)
			return;
		if (!m_ActionController || !m_Player || !m_ActionController.CF_IsExactActionPilot(m_Player, true))
			return;
		if (!m_bBaselineReady || !m_bPrePhysicsActive || m_iAllActionWrites != 0)
			return;
		if (!m_bLastActionObservationNeutral || m_iLastActionObservationCall != m_iPreActionCall)
			return;
		if (m_iActionCalls != m_iPreActionCall || m_iLastActionObservationEpoch != m_iPreEpoch)
			return;
		float stepMs = m_fPrePhysicsTimeSlice * 1000.0;
		if (!(stepMs > 0 && stepMs < 1000))
			return;
		float now = m_ActionWorld.GetWorldTime();
		float preAgeMs = now - m_fPrePhysicsMs;
		float actionAgeMs = now - m_fLastActionObservationMs;
		if (!(preAgeMs >= 0 && preAgeMs <= stepMs && actionAgeMs >= 0 && actionAgeMs <= stepMs))
			return;
		Physics body = m_Lead.GetPhysics();
		VehicleWheeledSimulation sim = ActionSimulation();
		if (!body || !sim || body.IsActive())
			return;
		if (!(Math.AbsFloat(m_fPreThrottle) <= 0.01 && Math.AbsFloat(m_fPreBrake) <= 0.01))
			return;
		if (!(Math.AbsFloat(sim.GetThrottle()) <= 0.01 && Math.AbsFloat(sim.GetBrake()) <= 0.01))
			return;
		if (!(Math.AbsFloat(m_fPreSpeedKmh) <= 0.5 && Math.AbsFloat(sim.GetSpeedKmh()) <= 0.5))
			return;

		m_iRawUnfinishedPre++;
		m_iBaselineSuspendedPre++;
		m_bAwaitPostPhysics = false;
		m_bPreviousPostEligible = false;
		if (m_iBaselineSuspendedPre > 32)
		{
			if (m_iBaselineSuspendedPre == 33)
				Print("[ConvoyFollower] ACTION_INPUT_BASELINE_SUSPENDED_LIMIT: run_id=" + m_sActionRun +
					" detailed_records=32 aggregate_continues=true motion_credit=false");
			return;
		}
		string record = "[ConvoyFollower] ACTION_INPUT_BASELINE_SUSPENDED: run_id=" + m_sActionRun;
		record += " reason=observed_owned_neutral_sleep world_ms=" + now + " phase=" + m_iActionPhase;
		record += " epoch=" + m_iActionEpoch + " before_ordinal=" + m_iPhysicsStep + " after_ordinal=" + m_iPostPhysicsCalls;
		record += " previous_before_ms=" + m_fPrePhysicsMs + " pre_age_ms=" + preAgeMs + " fixed_step_ms=" + stepMs;
		record += " hook_sequence=" + m_iLastActionObservationCall + " action_age_ms=" + actionAgeMs;
		record += " all_action_writes=" + m_iAllActionWrites + " callback_neutral=" + m_bLastActionObservationNeutral;
		record += " pre_throttle=" + m_fPreThrottle + " throttle=" + sim.GetThrottle();
		record += " pre_brake=" + m_fPreBrake + " brake=" + sim.GetBrake();
		record += " pre_speed_kmh=" + m_fPreSpeedKmh + " speed_kmh=" + sim.GetSpeedKmh();
		record += " raw_unfinished_pre=" + m_iRawUnfinishedPre + " baseline_suspended_pre=" + m_iBaselineSuspendedPre;
		record += " unexpected_pairing_gaps=" + m_iUnpairedPhysicsSteps + " pair_credit=false motion_credit=false";
		record += ActionPhysicsActiveFields();
		Print(record);
	}

	protected float SignedActionDelta(vector delta)
	{
		return delta[0] * m_vActionForward[0] + delta[2] * m_vActionForward[2];
	}

	protected void LogActionPhysics(string phase, VehicleWheeledSimulation sim, vector bodyPosition,
		float bodyPairStep, float entityPairStep, float postIntervalStep, bool intervalCredited)
	{
		string record = "[ConvoyFollower] ACTION_INPUT_PHYSICS: run_id=" + m_sActionRun;
		record += " world_ms=" + m_ActionWorld.GetWorldTime() + " phase=" + phase + " step_id=" + m_iPhysicsStep;
		record += " post_ordinal=" + m_iPostPhysicsCalls + " action_phase=" + m_iPrePhase + " epoch=" + m_iPreEpoch;
		record += " hook_sequence=" + m_iPreActionCall + " player=" + m_Player.GetID() + " truck=" + m_Lead.GetID();
		record += " exact_pilot=" + m_ActionController.CF_IsExactActionPilot(m_Player, true);
		record += " position=" + m_Lead.GetOrigin() + " body_position=" + bodyPosition;
		record += " signed_step_m=" + entityPairStep + " body_pair_step_m=" + bodyPairStep;
		record += " post_interval_step_m=" + postIntervalStep + " interval_credited=" + intervalCredited;
		record += " signed_progress_m=" + ActionProgress() + " readback_faults=" + m_iDriveReadbackFaults;
		record += " engine=" + sim.EngineIsOn() + " rpm=" + sim.EngineGetRPM() + " gear=" + sim.GetGear();
		record += " clutch=" + sim.GetClutch() + " throttle=" + sim.GetThrottle() + " brake=" + sim.GetBrake();
		record += " steering=" + sim.GetSteering() + " speed_kmh=" + sim.GetSpeedKmh();
		record += ActionPhysicsActiveFields();
		Print(record);
	}

	// Preserve every pairing failure, including a startup boundary. V3's one
	// unpaired callback was not traced, so its cause must not be inferred.
	protected void RecordActionPhysicsGap(string reason, float timeSlice)
	{
		m_iUnpairedPhysicsSteps++;
		if (reason == "after_without_before")
			m_iUnexpectedPostCallbacks++;
		else
			m_iRawUnfinishedPre++;
		m_bPreviousPostEligible = false;
		if (m_iUnpairedPhysicsSteps > 32)
		{
			if (m_iUnpairedPhysicsSteps == 33)
				Print("[ConvoyFollower] ACTION_INPUT_PHYSICS_GAP_LIMIT: run_id=" + m_sActionRun +
					" detailed_records=32 aggregate_continues=true unmatched_fails=true");
			return;
		}
		string record = "[ConvoyFollower] ACTION_INPUT_PHYSICS_GAP: run_id=" + m_sActionRun + " reason=" + reason;
		record += " world_ms=" + m_ActionWorld.GetWorldTime() + " time_slice=" + timeSlice;
		record += " before_ordinal=" + m_iPhysicsStep + " after_ordinal=" + m_iPostPhysicsCalls;
		record += " phase=" + m_iActionPhase + " epoch=" + m_iActionEpoch + " hook_sequence=" + m_iActionCalls;
		record += " pending=" + m_bAwaitPostPhysics + " previous_before_ms=" + m_fPrePhysicsMs;
		record += " previous_before_time_slice=" + m_fPrePhysicsTimeSlice;
		record += " previous_before_phase=" + m_iPrePhase + " previous_before_epoch=" + m_iPreEpoch;
		record += " previous_before_hook=" + m_iPreActionCall + " unpaired_steps=" + m_iUnpairedPhysicsSteps;
		record += " raw_unfinished_pre=" + m_iRawUnfinishedPre + " unexpected_post=" + m_iUnexpectedPostCallbacks;
		record += " previous_before_active=" + m_bPrePhysicsActive + ActionPhysicsActiveFields();
		Print(record);
	}

	protected bool ActionPhysicsPairPowered(VehicleWheeledSimulation sim)
	{
		if (m_iPrePhase != 2 || m_iActionPhase != 2 || m_iPreEpoch != m_iActionEpoch)
			return false;
		if (!m_bPreEngine || !sim.EngineIsOn() || m_iPreGear < 2 || sim.GetGear() < 2)
			return false;
		if (!(m_fPreThrottle > 0.05 && m_fPreClutch >= 0.8 && m_fPreBrake < 0.1))
			return false;
		return sim.GetThrottle() > 0.05 && sim.GetClutch() >= 0.8 && sim.GetBrake() < 0.1;
	}

	override void EOnSimulate(IEntity owner, float timeSlice)
	{
		if (!ActionWorldAlive() || m_bActionTerminal || m_iActionPhase == 0)
			return;
		if (owner != m_Lead)
		{
			FailAction("pre_physics_original_owner_changed");
			return;
		}
		VehicleWheeledSimulation sim = ActionSimulation();
		// NativeComponent is a local-only pointer. Reacquire through the exact
		// original truck; no between-callback physics-body identity is claimed.
		Physics body = m_Lead.GetPhysics();
		if (!sim || !body || !m_ActionController.CF_IsExactActionPilot(m_Player, true))
		{
			FailAction("pre_physics_identity_or_simulation_lost");
			return;
		}
		m_iPhysicsStep++;
		if (m_bAwaitPostPhysics)
			RecordActionPhysicsGap("before_replaced_pending_before", timeSlice);
		m_bAwaitPostPhysics = true;
		m_iPrePhase = m_iActionPhase;
		m_iPreEpoch = m_iActionEpoch;
		m_iPreActionCall = m_iActionCalls;
		m_vPrePhysicsPosition = m_Lead.GetOrigin();
		vector bodyTransform[4];
		body.GetDirectWorldTransform(bodyTransform);
		m_vPreBodyPosition = bodyTransform[3];
		m_fPrePhysicsMs = m_ActionWorld.GetWorldTime();
		m_fPrePhysicsTimeSlice = timeSlice;
		m_bPrePhysicsActive = body.IsActive();
		m_fPreSpeedKmh = sim.GetSpeedKmh();
		m_fPreThrottle = sim.GetThrottle();
		m_fPreBrake = sim.GetBrake();
		m_fPreClutch = sim.GetClutch();
		m_iPreGear = sim.GetGear();
		m_bPreEngine = sim.EngineIsOn();
		float now = m_ActionWorld.GetWorldTime();
		m_bLogPhysicsPair = m_iPhysicsLogs < 96 && now - m_fLastPhysicsLogMs >= 500;
		if (m_bLogPhysicsPair)
		{
			m_iPhysicsLogs++;
			m_fLastPhysicsLogMs = now;
			LogActionPhysics("before", sim, m_vPreBodyPosition, 0, 0, 0, false);
		}
	}

	override void EOnPostSimulate(IEntity owner, float timeSlice)
	{
		if (!ActionWorldAlive() || m_bActionTerminal || m_iActionPhase == 0)
			return;
		if (owner != m_Lead)
		{
			FailAction("post_physics_original_owner_changed");
			return;
		}
		VehicleWheeledSimulation sim = ActionSimulation();
		Physics body = m_Lead.GetPhysics();
		if (!sim || !body || !m_ActionController.CF_IsExactActionPilot(m_Player, true))
		{
			FailAction("post_physics_identity_or_simulation_lost");
			return;
		}
		m_iPostPhysicsCalls++;
		if (!m_bAwaitPostPhysics)
		{
			RecordActionPhysicsGap("after_without_before", timeSlice);
			return;
		}
		m_bAwaitPostPhysics = false;
		m_iPairedPhysicsSteps++;
		vector bodyTransform[4];
		body.GetDirectWorldTransform(bodyTransform);
		vector bodyPosition = bodyTransform[3];
		float entityPairStep = SignedActionDelta(m_Lead.GetOrigin() - m_vPrePhysicsPosition);
		float bodyPairStep = SignedActionDelta(bodyPosition - m_vPreBodyPosition);
		bool pairPowered = ActionPhysicsPairPowered(sim);
		bool adjacent = m_bPreviousPostEligible && pairPowered;
		if (m_iPreviousPostStep + 1 != m_iPhysicsStep || m_iPreviousPostEpoch != m_iActionEpoch)
			adjacent = false;
		if (m_iPreviousPostReadbackFaults != m_iDriveReadbackFaults)
			adjacent = false;
		float postIntervalStep;
		bool credited;
		// Sole powered metric: non-overlapping consecutive completed-post
		// body intervals in one drive epoch. Same-pair deltas are diagnostic.
		if (adjacent)
		{
			postIntervalStep = SignedActionDelta(bodyPosition - m_vPreviousPostBodyPosition);
			if (postIntervalStep > 0.001)
			{
				m_iPoweredPhysicsSteps++;
				m_fPoweredProgress += postIntervalStep;
				credited = true;
			}
		}
		m_bPreviousPostEligible = pairPowered;
		m_iPreviousPostStep = m_iPhysicsStep;
		m_iPreviousPostEpoch = m_iActionEpoch;
		m_iPreviousPostReadbackFaults = m_iDriveReadbackFaults;
		m_vPreviousPostBodyPosition = bodyPosition;
		if (m_bLogPhysicsPair)
			LogActionPhysics("after", sim, bodyPosition, bodyPairStep, entityPairStep, postIntervalStep, credited);
	}

	protected void FinishAction()
	{
		if (m_bActionTerminal)
			return;
		if (m_bAwaitPostPhysics && m_iPrePhase == 2)
		{
			RecordActionPhysicsGap("terminal_pending_drive_before", 0);
			m_bAwaitPostPhysics = false;
			if (m_sActionFailure.IsEmpty())
				m_sActionFailure = "physics_pairing_gap";
		}
		m_bActionTerminal = true;
		m_iActionPhase = 5;
		if (m_ActionController)
			m_ActionController.CF_DetachActionProbe(this);
		StopObservation();
		string verdict = "PASS";
		if (!m_sActionFailure.IsEmpty())
			verdict = "FAIL";
		string record = "[ConvoyFollower] ACTION_INPUT_RESULT: " + verdict + " run_id=" + m_sActionRun;
		record += " reason=" + m_sActionFailure + " prepare_calls=" + m_iActionCalls + " apply_calls=" + m_iApplyCalls;
		record += " drive_writes=" + m_iDriveWrites + " readbacks=" + m_iDriveReadbacks;
		record += " drive_progress_m=" + m_fDriveProgress + " elevation_gain_m=" + m_fDriveElevation;
		record += " physics_pairs=" + m_iPairedPhysicsSteps + " powered_steps=" + m_iPoweredPhysicsSteps;
		record += " powered_progress_m=" + m_fPoweredProgress + " unpaired_steps=" + m_iUnpairedPhysicsSteps;
		record += " powered_metric=consecutive_completed_post_direct_body readback_faults=" + m_iDriveReadbackFaults;
		record += " physics_identity=original_entity_each_callback physics_body_continuity_claim=false";
		record += " physics_active_events=" + m_iPhysicsActiveEvents;
		record += " raw_unfinished_pre=" + m_iRawUnfinishedPre + " baseline_suspended_pre=" + m_iBaselineSuspendedPre;
		record += " unexpected_post=" + m_iUnexpectedPostCallbacks + " pairing_policy=owned_neutral_baseline_sleep_else_complete_pairs";
		record += " released_neutral_calls=" + m_iReleasedNeutralCalls + " human_input_claim=false stopped_pose_claim=false";
		Print(record);
		if (!m_bActionAutoExit)
			return;
		string refusal = ActionExitRefusal();
		if (!refusal.IsEmpty())
		{
			Print("[ConvoyFollower] ACTION_INPUT_EXIT_REFUSED: run_id=" + m_sActionRun + " reason=" + refusal);
			return;
		}
		m_fTerminalMs = m_ActionWorld.GetWorldTime();
		m_bActionExitPending = true;
		Print("[ConvoyFollower] ACTION_INPUT_EXIT_SCHEDULED: run_id=" + m_sActionRun + " delay_s=30 supplemental_capture_only=true");
		GetGame().GetCallqueue().CallLater(RequestActionExit, ACTION_EXIT_DELAY_MS, false);
	}

	protected string ActionExitRefusal()
	{
#ifdef WORKBENCH
		return "workbench";
#else
		if (!ActionWorldAlive() || !m_bActionAutoExit || !m_bActionTerminal || !GetGame().InPlayMode())
			return "world_or_terminal_not_eligible";
		if (System.IsConsoleApp() || RplSession.Mode() != RplMode.None || !Replication.IsServer())
			return "not_offline_client";
		if (m_sActionWorldFile.IsEmpty() || GetGame().GetWorldFile() != m_sActionWorldFile || GetOwner().GetWorld() != m_ActionWorld)
			return "original_world_changed";
		return "";
#endif
	}

	protected void RequestActionExit()
	{
		if (!m_bActionExitPending || m_bActionExitRequested)
			return;
		m_bActionExitPending = false;
		string refusal = ActionExitRefusal();
		if (!refusal.IsEmpty())
		{
			Print("[ConvoyFollower] ACTION_INPUT_EXIT_REFUSED: run_id=" + m_sActionRun + " reason=" + refusal);
			return;
		}
#ifndef WORKBENCH
		float elapsed = m_ActionWorld.GetWorldTime() - m_fTerminalMs;
		if (elapsed < ACTION_EXIT_DELAY_MS)
		{
			// Match the paced fixture's bounded correction for a slightly
			// early callback; the next call repeats every eligibility check.
			float remainingMs = ACTION_EXIT_DELAY_MS - elapsed;
			if (remainingMs <= ACTION_EXIT_RETRY_MS && m_iActionExitDeferrals < ACTION_EXIT_MAX_DEFERRALS)
			{
				m_iActionExitDeferrals++;
				m_bActionExitPending = true;
				Print("[ConvoyFollower] ACTION_INPUT_EXIT_DEFERRED: run_id=" + m_sActionRun +
					" elapsed_ms=" + elapsed + " retry_ms=" + ACTION_EXIT_RETRY_MS + " deferrals=" + m_iActionExitDeferrals);
				GetGame().GetCallqueue().CallLater(RequestActionExit, ACTION_EXIT_RETRY_MS, false);
				return;
			}
			Print("[ConvoyFollower] ACTION_INPUT_EXIT_REFUSED: run_id=" + m_sActionRun +
				" reason=delay_not_elapsed elapsed_ms=" + elapsed + " deferrals=" + m_iActionExitDeferrals);
			return;
		}
		m_bActionExitRequested = true;
		Print("[ConvoyFollower] ACTION_INPUT_EXIT_REQUESTED: run_id=" + m_sActionRun + " elapsed_s=" + elapsed / 1000.0 + " shutdown_completed=false");
		GetGame().RequestClose();
#endif
	}

	override void OnDelete(IEntity owner)
	{
		m_bActionDeleting = true;
		if (GetGame())
			GetGame().GetCallqueue().Remove(RequestActionExit);
		if (m_ActionController)
			m_ActionController.CF_DetachActionProbe(this);
		m_bActionExitPending = false;
		super.OnDelete(owner);
		Print("[ConvoyFollower] ACTION_INPUT_CLEANUP: run_id=" + m_sActionRun + " terminal=" + m_bActionTerminal +
			" exit_requested=" + m_bActionExitRequested + " world_cleanup=" + CF_ConvoySession.CF_IsWorldCleanup());
	}
}
