class CF_NativeCargoActionProbeComponentClass : ScriptComponentClass
{
}

// Static action plumbing only. Native initialized cargo-menu actions perform
// every transfer. This fixture writes no resource value or vehicle control.
class CF_NativeCargoActionProbeComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.CheckBox, "Enable native scenario supplies for this explicit static cargo course")]
	protected bool m_bEnableScenarioSupplies;
	protected bool m_bSupplyEnableRequested;
	protected Vehicle m_Truck;
	protected IEntity m_Source;
	protected IEntity m_CargoOwner;
	protected World m_World;
	protected PlayerController m_Local;
	protected ChimeraCharacter m_Player;
	protected SCR_ResourceComponent m_CargoResources;
	protected SCR_ResourceContainer m_SourceContainer;
	protected SCR_ResourceContainer m_CargoContainer;
	protected SCR_ResourceConsumer m_LoadConsumer;
	protected SCR_ResourceGenerator m_LoadGenerator;
	protected SCR_ResourceConsumer m_UnloadConsumer;
	protected SCR_ResourceGenerator m_UnloadGenerator;
	protected BaseUserAction m_Load;
	protected BaseUserAction m_Unload;
	protected BaseUserAction m_Active;
	protected UserActionContext m_Rear;
	protected BaseActionsManagerComponent m_Manager;
	protected BaseInteractionHandlerComponent m_Interaction;
	protected int m_iStage;
	protected int m_iVisited;
	protected bool m_bAmbiguous;
	protected bool m_bFinished;
	protected bool m_bBound;
	protected bool m_bEditorCloseRequested;
	protected float m_fNextInteractionLogMs;
	protected float m_fNextActionGuardLogMs;
	protected bool m_bStartupReadyTracking;
	protected bool m_bUnloadEligibilityPending;
	protected float m_fUnloadEligibilityMs;
	protected float m_fStartupReadyMs;
	protected float m_fStartupReadyLastMs;
	protected float m_fStartupReadyTravel;
	protected vector m_vStartupReadyLastOrigin;
	protected float m_fBeginMs;
	protected float m_fStageMs;
	protected float m_fLastPrintMs;
	protected float m_fSourceStart;
	protected float m_fCargoStart;
	protected float m_fSourceHeld;
	protected float m_fCargoHeld;
	protected float m_fLoaded;
	protected float m_fProgress;
	protected vector m_vTruckOrigin;
	protected string m_sRun;
	protected string m_sReason = "waiting_for_native_bindings";
	protected static const float TOLERANCE = 0.001;

	protected string Key(IEntity entity)
	{
		if (!entity) return "none";
		return entity.GetID().ToString();
	}

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer()) return;
		m_World = GetGame().GetWorld();
		m_Truck = Vehicle.Cast(owner);
		m_fBeginMs = m_World.GetWorldTime();
		m_sRun = Key(owner) + "_" + m_fBeginMs;
		SetEventMask(owner, EntityEvent.POSTFRAME);
		Print("[ConvoyFollower] CARGO_ACTION_INIT: run_id=" + m_sRun + " world=" + GetGame().GetWorldFile() +
			" static=true transfer_max=100 stable_s=3 timeout_s=90 native_dispatch=true resource_writes=false vehicle_controls=false input_claim=false delivery_claim=false");
	}

	protected bool Under(IEntity entity, IEntity parent)
	{
		return entity && parent && (entity == parent || entity.GetRootParent() == parent);
	}

	protected void FindActions(IEntity entity)
	{
		if (!entity || m_bAmbiguous) return;
		m_iVisited++;
		if (m_iVisited > 256) { m_bAmbiguous = true; return; }
		BaseActionsManagerComponent manager = BaseActionsManagerComponent.Cast(entity.FindComponent(BaseActionsManagerComponent));
		array<BaseUserAction> actions = {};
		if (manager) manager.GetActionsList(actions);
		foreach (BaseUserAction action : actions)
		{
			if (!action || !Under(action.GetOwner(), m_Truck)) continue;
			if (SCR_ResourceContainerVehicleLoadAction.Cast(action))
			{
				if (m_Load && m_Load != action) m_bAmbiguous = true;
				m_Load = action;
			}
			if (SCR_ResourceContainerVehicleUnloadAction.Cast(action))
			{
				if (m_Unload && m_Unload != action) m_bAmbiguous = true;
				m_Unload = action;
			}
		}
		IEntity child = entity.GetChildren();
		while (child) { FindActions(child); child = child.GetSibling(); }
	}

	protected bool Contains(UserActionContext context, BaseUserAction action)
	{
		if (!context || !action) return false;
		array<BaseUserAction> actions = {};
		context.GetActionsList(actions);
		return actions.Find(action) >= 0;
	}

	protected bool Bind()
	{
		SCR_BaseGameMode mode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!mode || !mode.IsMaster() || !m_Truck) return false;
		m_Local = GetGame().GetPlayerController();
		if (!m_Local || m_Local.GetPlayerId() <= 0) return false;
		if (!m_Local.FindComponent(SCR_ResourcePlayerControllerInventoryComponent))
		{ m_sReason = "local_resource_inventory_missing"; return false; }
		m_Source = m_World.FindEntityByName("CF_CargoActionSource");
		SCR_ResourceComponent sourceResources;
		if (m_Source) sourceResources = SCR_ResourceComponent.Cast(m_Source.FindComponent(SCR_ResourceComponent));
		if (!sourceResources) return false;
		m_SourceContainer = sourceResources.GetContainer(EResourceType.SUPPLIES);
		m_iVisited = 0;
		FindActions(m_Truck);
		if (m_bAmbiguous) { Finish(false, "ambiguous_initialized_cargo_actions"); return false; }
		if (!m_Load || !m_Unload) { m_sReason = "native_load_or_unload_action_missing"; return false; }
		m_CargoOwner = m_Load.GetOwner();
		m_Manager = m_Load.GetActionsManager();
		if (!m_Manager || m_Unload.GetActionsManager() != m_Manager || m_Unload.GetOwner() != m_CargoOwner) return false;
		m_Rear = m_Manager.GetContext("door_rear");
		if (!Contains(m_Rear, m_Load) || !Contains(m_Rear, m_Unload))
		{ m_sReason = "exact_rear_context_membership_missing"; return false; }
		m_CargoResources = SCR_ResourceComponent.Cast(m_CargoOwner.FindComponent(SCR_ResourceComponent));
		if (!m_CargoResources) return false;
		m_CargoContainer = m_CargoResources.GetContainer(EResourceType.SUPPLIES);
		m_LoadConsumer = m_CargoResources.GetConsumer(EResourceGeneratorID.VEHICLE_LOAD, EResourceType.SUPPLIES);
		m_LoadGenerator = m_CargoResources.GetGenerator(EResourceGeneratorID.VEHICLE_LOAD, EResourceType.SUPPLIES);
		m_UnloadConsumer = m_CargoResources.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, EResourceType.SUPPLIES);
		m_UnloadGenerator = m_CargoResources.GetGenerator(EResourceGeneratorID.VEHICLE_UNLOAD, EResourceType.SUPPLIES);
		if (!m_SourceContainer || !m_CargoContainer || !m_LoadConsumer || !m_LoadGenerator || !m_UnloadConsumer || !m_UnloadGenerator) return false;
		m_Interaction = BaseInteractionHandlerComponent.Cast(m_Local.FindComponent(BaseInteractionHandlerComponent));
		if (!m_Interaction) return false;
		m_Player = ChimeraCharacter.Cast(m_Local.GetControlledEntity());
		if (!m_Player)
		{
			vector rearMatrix[4];
			if (!m_Rear.GetTransformationWorld(rearMatrix)) return false;
			vector position = rearMatrix[3] + rearMatrix[2] * 1.2;
			position[1] = m_World.GetSurfaceY(position[0], position[2]) + 0.05;
			Resource prefab = Resource.Load("{E1CB513B8B9B08F4}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_US_Crew.et");
			if (!prefab.IsValid()) { Finish(false, "owner_prefab_invalid"); return false; }
			EntitySpawnParams params = new EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = position;
			m_Player = ChimeraCharacter.Cast(GetGame().SpawnEntityPrefab(prefab, m_World, params));
			if (!m_Player || !m_Local.SetControlledEntity(m_Player))
			{ Finish(false, "local_owner_possession_failed"); return false; }
			Print("[ConvoyFollower] CARGO_ACTION_OWNER_SETUP: run_id=" + m_sRun + " player_id=" + m_Local.GetPlayerId() +
				" character_id=" + Key(m_Player) + " origin=" + position + " spawned_native_owner=true movement_claim=false");
		}
		m_bBound = true;
		Print("[ConvoyFollower] CARGO_ACTION_BIND: run_id=" + m_sRun + " truck_id=" + Key(m_Truck) +
			" cargo_owner_id=" + Key(m_CargoOwner) + " source_id=" + Key(m_Source) + " character_id=" + Key(m_Player) +
			" player_id=" + m_Local.GetPlayerId() + " context=" + m_Rear.GetContextName() +
			" load_id=" + m_Load.GetActionID() + " unload_id=" + m_Unload.GetActionID());
		return true;
	}

	protected bool CF_HasCargoBinding()
	{
		if (!GetGame() || GetGame().GetWorld() != m_World || CF_ConvoySession.CF_IsWorldCleanup()) return false;
		if (!m_Truck || m_World.FindEntityByName("CF_CargoActionTruck") != m_Truck ||
			!m_Source || m_World.FindEntityByName("CF_CargoActionSource") != m_Source) return false;
		if (!m_Local || !GetGame().GetPlayerManager() || GetGame().GetPlayerController() != m_Local || !m_Player || m_Local.GetControlledEntity() != m_Player) return false;
		if (GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(m_Player) != m_Local.GetPlayerId()) return false;
		if (!Under(m_CargoOwner, m_Truck) || !m_Load || !m_Unload || !m_Manager || !m_Rear) return false;
		return m_Load.GetOwner() == m_CargoOwner && m_Unload.GetOwner() == m_CargoOwner &&
			m_Load.GetActionsManager() == m_Manager && m_Unload.GetActionsManager() == m_Manager &&
			m_Manager.GetContext("door_rear") == m_Rear && Contains(m_Rear, m_Load) && Contains(m_Rear, m_Unload);
	}

	protected void CF_LogCargoInteractionGuard(float now)
	{
		if (now < m_fNextInteractionLogMs) return;
		m_fNextInteractionLogMs = now + 1000;
		SCR_CharacterControllerComponent character = SCR_CharacterControllerComponent.Cast(m_Player.GetCharacterController());
		MenuManager menus = GetGame().GetMenuManager();
		Print("[ConvoyFollower] CARGO_ACTION_INTERACTION_GUARD: run_id=" + m_sRun + " world_ms=" + now +
			" character_controller=" + (character != null) + " can_interact=" + (character && character.CanInteract()) +
			" menu_open=" + (menus && menus.IsAnyMenuOpen()) + " editor_open=" + SCR_EditorManagerEntity.IsOpenedInstance() +
			" editor_can_close=" + SCR_EditorManagerEntity.CanCloseInstance() + " close_requested=" + m_bEditorCloseRequested);
	}

	protected bool CF_PrepareCargoPlayMode()
	{
		if (!SCR_EditorManagerEntity.IsOpenedInstance()) return true;
		m_sReason = "waiting_for_native_editor_close";
		if (!m_bEditorCloseRequested && SCR_EditorManagerEntity.CanCloseInstance())
		{
			m_bEditorCloseRequested = true;
			bool dispatched = SCR_EditorManagerEntity.CloseInstance();
			Print("[ConvoyFollower] CARGO_ACTION_EDITOR_CLOSE: run_id=" + m_sRun + " dispatched=" + dispatched +
				" completion_claim=false source=native_editor_api local_owner_id=" + Key(m_Player));
		}
		// CloseInstance only confirms an instance existed, not successful close.
		// A later frame must observe closed before normal action guards run.
		return false;
	}

	protected bool CF_PrepareCargoSupplies()
	{
		if (!m_bEnableScenarioSupplies) return true;
		SCR_BaseGameMode mode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!mode || !mode.IsMaster()) { m_sReason = "native_supply_scenario_not_master"; return false; }
		if (mode.IsResourceTypeEnabled(EResourceType.SUPPLIES)) return true;
		m_sReason = "waiting_for_native_scenario_supplies";
		if (!m_bSupplyEnableRequested)
		{
			m_bSupplyEnableRequested = true;
			mode.SetResourceTypeEnabled(true, EResourceType.SUPPLIES);
			Print("[ConvoyFollower] CARGO_ACTION_SUPPLIES_SETUP: run_id=" + m_sRun +
				" before_enabled=false after_enabled=" + mode.IsResourceTypeEnabled(EResourceType.SUPPLIES) +
				" scenario_opt_in=true native_setting=true resource_value_writes=false transfer_claim=false");
		}
		// Observe the setting again on a later frame; normal action and queue gates remain.
		return false;
	}

	protected int CF_CargoQueueCount(SCR_ResourceContainerQueueBase queue)
	{
		if (!queue) return -1;
		return queue.GetContainerCount();
	}

	protected bool Reach(BaseUserAction action)
	{
		if (!CF_HasCargoBinding() || !m_Manager.IsEnabled() || !action || !m_Interaction)
		{ m_sReason = "identity_manager_or_interaction_handler_missing"; return false; }
		// IsInteractionAvailable means reticle-selected context availability.
		// This below-input fixture instead mirrors stock GetCanInteractScript.
		SCR_CharacterControllerComponent character = SCR_CharacterControllerComponent.Cast(m_Player.GetCharacterController());
		MenuManager menus = GetGame().GetMenuManager();
		if (!character || !character.CanInteract())
		{ m_sReason = "native_character_interaction_blocked"; return false; }
		if (menus && menus.IsAnyMenuOpen())
		{ m_sReason = "native_menu_interaction_blocked"; return false; }
		CompartmentAccessComponent access = m_Player.GetCompartmentAccessComponent();
		SCR_DamageManagerComponent damage = m_Player.GetDamageManager();
		if (!character || character.IsUnconscious() || !damage || damage.GetState() == EDamageState.DESTROYED ||
			m_Player.IsInVehicle() || !access || access.IsGettingIn() || access.IsGettingOut())
		{ m_sReason = "owner_not_conscious_on_foot"; return false; }
		vector eye = m_Player.EyePosition();
		float range = Math.Max(action.GetVisibilityRange(), m_Interaction.GetVisibilityRange());
		if (!(range > 0 && range < 20) || vector.Distance(eye, m_Rear.GetOrigin()) > range || !m_Rear.IsInVisibilityAngle(eye))
		{ m_sReason = "rear_context_range_or_angle"; return false; }
		if (m_Rear.ShouldCheckLineOfSight())
		{
			TraceParam trace = new TraceParam();
			trace.Start = eye;
			trace.End = m_Rear.GetOrigin();
			trace.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
			trace.Exclude = m_Player;
			float fraction = m_World.TraceMove(trace, null);
			vector hit = eye + (trace.End - eye) * fraction;
			if (fraction < 0.999 && (!Under(trace.TraceEnt, m_Truck) || vector.Distance(hit, trace.End) > m_Rear.GetRadius()))
			{ m_sReason = "rear_context_line_of_sight"; return false; }
		}
		bool perFrame = action.ShouldPerformPerFrame();
		float duration = action.GetActionDuration();
		// -1 means not evaluated: retain the original short-circuit order.
		// CanBePerformed has native subscription side effects, so do not call it
		// solely for telemetry when any earlier predicate rejected the action.
		int shown = -1;
		int performed = -1;
		if (perFrame && duration == -1)
		{
			shown = 0;
			if (action.CanBeShown(m_Player))
			{
				shown = 1;
				performed = 0;
				if (action.CanBePerformed(m_Player)) performed = 1;
			}
		}
		float now = m_World.GetWorldTime();
		if (now >= m_fNextActionGuardLogMs)
		{
			m_fNextActionGuardLogMs = now + 1000;
			SCR_ResourceComponent sourceResources = SCR_ResourceComponent.Cast(m_Source.FindComponent(SCR_ResourceComponent));
			int loadConsumerCount = -1, loadGeneratorCount = -1, unloadConsumerCount = -1, unloadGeneratorCount = -1;
			if (m_LoadConsumer) loadConsumerCount = CF_CargoQueueCount(m_LoadConsumer.GetContainerQueue());
			if (m_LoadGenerator) loadGeneratorCount = CF_CargoQueueCount(m_LoadGenerator.GetContainerQueue());
			if (m_UnloadConsumer) unloadConsumerCount = CF_CargoQueueCount(m_UnloadConsumer.GetContainerQueue());
			if (m_UnloadGenerator) unloadGeneratorCount = CF_CargoQueueCount(m_UnloadGenerator.GetContainerQueue());
			Print("[ConvoyFollower] CARGO_ACTION_NATIVE_GUARD: run_id=" + m_sRun + " world_ms=" + now +
				" action_id=" + action.GetActionID() + " loading=" + (action == m_Load) +
				" per_frame=" + perFrame + " duration=" + duration + " can_be_shown=" + shown + " can_be_performed=" + performed +
				" cargo_resource_enabled=" + (m_CargoResources && m_CargoResources.IsResourceTypeEnabled(EResourceType.SUPPLIES)) +
				" source_resource_enabled=" + (sourceResources && sourceResources.IsResourceTypeEnabled(EResourceType.SUPPLIES)) +
				" load_consumer_count=" + loadConsumerCount + " load_generator_count=" + loadGeneratorCount +
				" unload_consumer_count=" + unloadConsumerCount + " unload_generator_count=" + unloadGeneratorCount +
				" cannot_perform_reason=" + action.GetCannotPerformReason());
		}
		if (!perFrame) { m_sReason = "native_action_not_per_frame"; return false; }
		if (duration != -1) { m_sReason = "native_action_duration_not_minus_one"; return false; }
		if (shown != 1) { m_sReason = "native_action_not_shown"; return false; }
		if (performed != 1) { m_sReason = "native_action_not_performed_" + action.GetCannotPerformReason(); return false; }
		return true;
	}

	protected bool Queue(SCR_ResourceContainerQueueBase queue, SCR_ResourceContainer expected, string label, bool log)
	{
		if (!queue || !expected || queue.GetContainerCount() != 1) { m_sReason = "unexpected_queue_count_" + label; return false; }
		SCR_ResourceContainer container = queue.GetContainerAt(0);
		if (container != expected || container.GetResourceType() != EResourceType.SUPPLIES ||
			!(Math.AbsFloat(queue.GetAggregatedResourceValue() - expected.GetResourceValue()) <= TOLERANCE) ||
			!(Math.AbsFloat(queue.GetAggregatedMaxResourceValue() - expected.GetMaxResourceValue()) <= TOLERANCE))
		{ m_sReason = "unexpected_queue_contributor_" + label; return false; }
		if (log) Print("[ConvoyFollower] CARGO_ACTION_QUEUE: run_id=" + m_sRun + " queue=" + label +
			" count=1 owner_id=" + Key(container.GetOwner()) + " current=" + container.GetResourceValue() + " max=" + container.GetMaxResourceValue());
		return true;
	}

	protected bool Queues(bool loading, bool log = false)
	{
		if (loading) return Queue(m_LoadConsumer.GetContainerQueue(), m_SourceContainer, "load_source", log) &&
			Queue(m_LoadGenerator.GetContainerQueue(), m_CargoContainer, "load_cargo", log);
		return Queue(m_UnloadConsumer.GetContainerQueue(), m_CargoContainer, "unload_cargo", log) &&
			Queue(m_UnloadGenerator.GetContainerQueue(), m_SourceContainer, "unload_storage", log);
	}

	protected bool Counts(out float source, out float cargo)
	{
		if (!m_Source || !m_CargoOwner || !m_CargoResources || !m_SourceContainer || !m_CargoContainer ||
			m_SourceContainer.GetOwner() != m_Source || m_CargoContainer.GetOwner() != m_CargoOwner) return false;
		SCR_ResourceComponent sourceResources = SCR_ResourceComponent.Cast(m_Source.FindComponent(SCR_ResourceComponent));
		if (!sourceResources || sourceResources.GetContainer(EResourceType.SUPPLIES) != m_SourceContainer ||
			m_CargoOwner.FindComponent(SCR_ResourceComponent) != m_CargoResources ||
			m_CargoResources.GetContainer(EResourceType.SUPPLIES) != m_CargoContainer) return false;
		if (m_CargoResources.GetConsumer(EResourceGeneratorID.VEHICLE_LOAD, EResourceType.SUPPLIES) != m_LoadConsumer ||
			m_CargoResources.GetGenerator(EResourceGeneratorID.VEHICLE_LOAD, EResourceType.SUPPLIES) != m_LoadGenerator ||
			m_CargoResources.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, EResourceType.SUPPLIES) != m_UnloadConsumer ||
			m_CargoResources.GetGenerator(EResourceGeneratorID.VEHICLE_UNLOAD, EResourceType.SUPPLIES) != m_UnloadGenerator) return false;
		source = m_SourceContainer.GetResourceValue();
		cargo = m_CargoContainer.GetResourceValue();
		return source >= 0 && source <= m_SourceContainer.GetMaxResourceValue() && source < 100000 &&
			cargo >= 0 && cargo <= m_CargoContainer.GetMaxResourceValue() && cargo < 100000;
	}

	protected bool StaticTruck()
	{
		CarControllerComponent car = CarControllerComponent.Cast(m_Truck.FindComponent(CarControllerComponent));
		VehicleWheeledSimulation sim;
		if (car) sim = car.GetSimulation();
		return sim && !sim.EngineIsOn() && Math.AbsFloat(sim.GetSpeedKmh()) <= 0.5 &&
			(m_iStage == 0 || vector.Distance(m_vTruckOrigin, m_Truck.GetOrigin()) <= 0.5);
	}

	protected bool CF_CargoStartupReady(float now)
	{
		m_sReason = "waiting_for_three_seconds_stationary_and_eligible";
		vector origin = m_Truck.GetOrigin();
		if (!m_bStartupReadyTracking)
		{
			m_bStartupReadyTracking = true;
			m_fStartupReadyMs = now;
			m_fStartupReadyLastMs = now;
			m_fStartupReadyTravel = 0;
			m_vStartupReadyLastOrigin = origin;
			return false;
		}
		float elapsed = now - m_fStartupReadyLastMs;
		float step = vector.Distance(origin, m_vStartupReadyLastOrigin);
		if (!(elapsed > 0 && elapsed <= 250) || !(step >= 0 && step <= 0.05))
		{ m_bStartupReadyTracking = false; return false; }
		m_fStartupReadyTravel += step;
		if (!(m_fStartupReadyTravel <= 0.05))
		{ m_bStartupReadyTracking = false; return false; }
		m_fStartupReadyLastMs = now;
		m_vStartupReadyLastOrigin = origin;
		if (now - m_fStartupReadyMs < 3000) return false;
		Print("[ConvoyFollower] CARGO_ACTION_STARTUP_READY: run_id=" + m_sRun + " world_ms=" + now +
			" observed_s=" + (now - m_fStartupReadyMs) / 1000 + " truck_travel_m=" + m_fStartupReadyTravel +
			" speed_limit_kmh=0.5 travel_limit_m=0.05 exact_original=true native_action_and_queues_eligible=true controls_written=false");
		return true;
	}

	protected void Cancel(string reason)
	{
		if (!m_Active) return;
		bool safe = CF_HasCargoBinding() && (m_Active == m_Load || m_Active == m_Unload) && m_Active.GetOwner() == m_CargoOwner;
		if (safe) m_Player.DoCancelObjectAction(m_Active);
		Print("[ConvoyFollower] CARGO_ACTION_CANCEL: run_id=" + m_sRun + " action_id=" + m_Active.GetActionID() + " dispatched=" + safe + " reason=" + reason);
		m_Active = null;
	}

	protected void Start(bool loading, float now)
	{
		BaseUserAction action = m_Unload;
		if (loading) action = m_Load;
		if (!loading && m_bUnloadEligibilityPending && now - m_fUnloadEligibilityMs >= 5000)
		{ Finish(false, "unload_subscription_eligibility_timeout_" + m_sReason); return; }
		if (!Reach(action) || !Queues(loading, true))
		{
			// Native CanBePerformed requests the subscription, then reads aggregates
			// before the resource system's subsequent FixedFrame spatial update.
			if (!loading && m_iStage == 3 && m_sReason == "native_action_not_performed_#AR-Supplies_CannotPerform_Vehicle_NoStorage")
			{
				SCR_ResourceContainerQueueBase queue;
				if (m_UnloadGenerator) queue = m_UnloadGenerator.GetContainerQueue();
				if (queue && queue.GetContainerCount() == 0)
				{
					if (!m_bUnloadEligibilityPending)
					{
						m_bUnloadEligibilityPending = true;
						m_fUnloadEligibilityMs = now;
						Print("[ConvoyFollower] CARGO_ACTION_UNLOAD_WARMUP: run_id=" + m_sRun +
							" world_ms=" + now + " max_s=5 reason=native_no_storage_empty_generator action_started=false");
					}
					return;
				}
			}
			Finish(false, m_sReason);
			return;
		}
		if (!loading && m_bUnloadEligibilityPending)
			Print("[ConvoyFollower] CARGO_ACTION_UNLOAD_READY: run_id=" + m_sRun +
				" waited_s=" + (now - m_fUnloadEligibilityMs) / 1000 + " native_predicates=true exact_queues=true");
		m_Active = action;
		m_Active.SetActiveContext(m_Rear);
		m_Player.DoStartObjectAction(m_Active);
		m_fProgress = 0;
		m_fStageMs = now;
		m_iStage = 4;
		if (loading) m_iStage = 2;
		Print("[ConvoyFollower] CARGO_ACTION_START: run_id=" + m_sRun + " loading=" + loading +
			" action_id=" + action.GetActionID() + " owner_id=" + Key(action.GetOwner()) +
			" rear=" + m_Rear.GetOrigin() + " eye=" + m_Player.EyePosition() + " native_dispatch=true");
	}

	protected void Finish(bool pass, string reason)
	{
		if (m_bFinished) return;
		Cancel("terminal_" + reason);
		m_bFinished = true;
		float source, cargo;
		Counts(source, cargo);
		Print("[ConvoyFollower] CARGO_ACTION_RESULT: run_id=" + m_sRun + " pass=" + pass + " reason=" + reason +
			" loaded=" + m_fLoaded + " source=" + source + " cargo=" + cargo + " static=true delivery=false ordinary_input=false");
	}

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (m_bFinished || !Replication.IsServer() || !GetGame() || GetGame().GetWorld() != m_World || CF_ConvoySession.CF_IsWorldCleanup()) return;
		float now = m_World.GetWorldTime();
		if (now - m_fBeginMs > 90000) { Finish(false, "timeout_" + m_sReason); return; }
		if (!m_bBound && !Bind()) return;
		if (!CF_HasCargoBinding()) { Finish(false, "original_binding_lost"); return; }
		CF_LogCargoInteractionGuard(now);
		if (m_iStage == 0 && !CF_PrepareCargoPlayMode()) { m_bStartupReadyTracking = false; return; }
		if (m_iStage == 0 && !CF_PrepareCargoSupplies()) { m_bStartupReadyTracking = false; return; }
		float source, cargo;
		if (!Counts(source, cargo)) { Finish(false, "invalid_or_replaced_resource_container"); return; }
		if (now - m_fLastPrintMs >= 500)
		{
			m_fLastPrintMs = now;
			Print("[ConvoyFollower] CARGO_ACTION_SAMPLE: run_id=" + m_sRun + " stage=" + m_iStage + " world_ms=" + now +
				" source=" + source + " cargo=" + cargo + " total=" + (source + cargo) + " source_cap=" + m_SourceContainer.GetMaxResourceValue() +
				" cargo_cap=" + m_CargoContainer.GetMaxResourceValue() + " reason=" + m_sReason);
		}
		if (m_iStage == 0)
		{
			if (!StaticTruck())
			{ m_bStartupReadyTracking = false; m_sReason = "waiting_for_static_truck"; return; }
			if (!Reach(m_Load) || !Queues(true)) { m_bStartupReadyTracking = false; return; }
			if (source < 100 || cargo != 0 || source >= m_SourceContainer.GetMaxResourceValue())
			{ m_bStartupReadyTracking = false; m_sReason = "initial_source_or_empty_cargo_invalid"; return; }
			if (!CF_CargoStartupReady(now)) return;
			m_fSourceStart = source; m_fCargoStart = cargo;
			m_fSourceHeld = source; m_fCargoHeld = cargo;
			m_vTruckOrigin = m_Truck.GetOrigin(); m_fStageMs = now; m_iStage = 1;
			return;
		}
		if (!StaticTruck()) { Finish(false, "static_truck_condition_lost"); return; }
		if (Math.AbsFloat(source + cargo - m_fSourceStart - m_fCargoStart) > TOLERANCE)
		{ Finish(false, "resource_total_not_conserved"); return; }
		if (m_iStage == 1 || m_iStage == 3 || m_iStage == 5)
		{
			if (Math.AbsFloat(source - m_fSourceHeld) > TOLERANCE || Math.AbsFloat(cargo - m_fCargoHeld) > TOLERANCE || m_Load.IsInProgress() || m_Unload.IsInProgress())
			{ Finish(false, "counts_or_action_not_stable_after_cancel"); return; }
			if (now - m_fStageMs < 3000) return;
			if (m_iStage != 3 || !m_bUnloadEligibilityPending)
				Print("[ConvoyFollower] CARGO_ACTION_STABLE: run_id=" + m_sRun + " stage=" + m_iStage + " observed_s=" + (now - m_fStageMs) / 1000 + " source=" + source + " cargo=" + cargo);
			if (m_iStage == 1) Start(true, now);
			else if (m_iStage == 3) Start(false, now);
			else Finish(true, "conserved_load_cancel_unload_cancel");
			return;
		}
		bool loading = m_iStage == 2;
		float transferred = cargo - m_fCargoHeld;
		if (!loading) transferred = m_fCargoHeld - cargo;
		if (transferred > TOLERANCE)
		{
			Cancel("first_positive_transfer");
			if (!Queues(loading, true)) { Finish(false, m_sReason); return; }
			if (transferred > 100 + TOLERANCE || (!loading && Math.AbsFloat(transferred - m_fLoaded) > TOLERANCE))
			{ Finish(false, "unexpected_transfer_amount"); return; }
			if (loading) m_fLoaded = transferred;
			if (!loading && (Math.AbsFloat(source - m_fSourceStart) > TOLERANCE || Math.AbsFloat(cargo - m_fCargoStart) > TOLERANCE))
			{ Finish(false, "unload_did_not_restore_original_totals"); return; }
			Print("[ConvoyFollower] CARGO_ACTION_TRANSFER: run_id=" + m_sRun + " loading=" + loading + " amount=" + transferred + " source=" + source + " cargo=" + cargo + " conserved=true");
			m_fSourceHeld = source; m_fCargoHeld = cargo; m_fStageMs = now;
			m_iStage++;
			return;
		}
		if (transferred < -TOLERANCE || now - m_fStageMs > 15000 || !(timeSlice > 0 && timeSlice <= 0.25))
		{ Finish(false, "wrong_direction_timeout_or_invalid_frame_slice"); return; }
		if (!Reach(m_Active) || !Queues(loading)) { Finish(false, m_sReason); return; }
		m_fProgress = m_Active.GetActionProgress(m_fProgress, timeSlice);
		m_Player.DoPerformContinuousObjectAction(m_Active, timeSlice);
	}

	override void OnDelete(IEntity owner)
	{
		Cancel("fixture_deleted");
		Print("[ConvoyFollower] CARGO_ACTION_CLEANUP: run_id=" + m_sRun + " terminal=" + m_bFinished);
		m_Load = null; m_Unload = null; m_Rear = null; m_Manager = null;
		m_SourceContainer = null; m_CargoContainer = null; m_CargoResources = null;
		m_LoadConsumer = null; m_LoadGenerator = null; m_UnloadConsumer = null; m_UnloadGenerator = null;
		m_Player = null; m_Local = null; m_Interaction = null; m_CargoOwner = null; m_Source = null; m_Truck = null; m_World = null;
	}
}
