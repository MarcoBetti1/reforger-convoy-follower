// Test-only owner-pilot panel fixture. Attach this to the lead M923 in a
// separate OpenRoad world; it never drives the truck or simulates UI clicks.
class CF_PanelPilotProbeComponentClass : ScriptComponentClass
{
}

class CF_PanelPilotProbeComponent : ScriptComponent
{
	[Attribute(defvalue: "1", params: "1 2 1", desc: "Number of staged followers to recruit")]
	protected int m_iFollowerCount;
	[Attribute(defvalue: "1", desc: "Automatically open native MapMenu after recruitment; disable for a real map-key test")]
	protected bool m_bAutoOpenMap;
	protected Vehicle m_Lead;
	protected ChimeraCharacter m_Player;
	protected ref array<CF_DriverControllerComponent> m_Drivers = {};
	protected int m_iNextOrder;
	protected int m_iCameraAttempts;
	protected int m_iTicks;
	protected bool m_bCameraSelected;
	protected bool m_bReady;
	protected bool m_bMenuRequested;
	protected int m_iReadyTick;
	protected InputManager m_ObservedInput;
	protected IEntity m_InitialPilot;
	protected bool m_bObservationStopped;
	protected bool m_bPoseCaptured;
	protected bool m_bLastMapOpen;
	protected int m_iMapActionCount;
	protected float m_fSampleSeconds;
	protected float m_fThrustPeak;
	protected float m_fSteeringMin;
	protected float m_fSteeringMax;
	protected vector m_vInputStart;
	protected vector m_vInputPrevious;
	protected vector m_vInputForward;
	protected float m_fObservationStartMs;
	protected ref array<int> m_RawPrevious = {};
	protected bool m_bRawObserved;
	protected int m_iRawEventCount;
	protected int m_iRoutingEventCount;
	protected int m_iBindingSnapshots;
	protected int m_iLastObservedDevice = -1;
	protected float m_fLastRoutingMs = -5000;
	protected string m_sLastRouting;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Lead = Vehicle.Cast(owner);
		m_fObservationStartMs = GetGame().GetWorld().GetWorldTime();
		m_RawPrevious.Resize(8);
		SetEventMask(owner, EntityEvent.POSTFRAME);
		Print("[ConvoyFollower] PANEL_FIXTURE_INIT: followers=" + m_iFollowerCount + " auto_open_map=" + m_bAutoOpenMap +
			" telemetry=postframe_read_only human_input_claim=false raw_events_max=256 routing_events_max=64 binding_snapshots_max=4 observation_ms=240000");
		GetGame().GetCallqueue().CallLater(Poll, 1000, true);
	}

	protected void StopObservation()
	{
		m_bObservationStopped = true;
		if (GetGame())
		{
			GetGame().GetCallqueue().Remove(Poll);
			if (m_ObservedInput)
				m_ObservedInput.RemoveActionListener("GadgetMap", EActionTrigger.DOWN, ObserveMapAction);
		}
		m_ObservedInput = null;
	}

	override void OnDelete(IEntity owner)
	{
		StopObservation();
	}

	// An action callback shows that the input system delivered GadgetMap. It
	// cannot identify a physical keyboard versus an injected key or other mod.
	protected void ObserveMapAction()
	{
		if (m_bObservationStopped || !GetGame() || !GetGame().GetWorld() || CF_ConvoySession.CF_IsWorldCleanup())
			return;
		m_iMapActionCount++;
		Print("[ConvoyFollower] PANEL_INPUT_MAP_ACTION: world_ms=" + GetGame().GetWorld().GetWorldTime() +
			" count=" + m_iMapActionCount + " fixture_auto_requested=" + m_bMenuRequested + " key_origin=unverified");
	}

	// Read Debug state without ClearKey: held bits and edge/press counts are
	// different evidence. A changed counter is not a synthesized widget event.
	protected void ObserveRawChannel(int index, string name, int raw, int pressedMask, int countMask, float now)
	{
		int previous = m_RawPrevious[index];
		m_RawPrevious[index] = raw;
		if (m_bRawObserved && raw == previous)
			return;
		if (m_iRawEventCount >= 256)
		{
			if (m_iRawEventCount == 256)
				Print("[ConvoyFollower] PANEL_INPUT_RAW_LIMIT: world_ms=" + now + " limit=256 later_raw_events_unobserved=true");
			m_iRawEventCount = 257;
			return;
		}
		m_iRawEventCount++;
		bool held = (raw & pressedMask) != 0;
		bool previousHeld = (previous & pressedMask) != 0;
		string change = "count_or_value";
		if (!m_bRawObserved)
			change = "initial";
		else if (held && !previousHeld)
			change = "down";
		else if (!held && previousHeld)
			change = "up";
		int mouseX, mouseY;
		WidgetManager.GetMousePos(mouseX, mouseY);
		Print("[ConvoyFollower] PANEL_INPUT_RAW: world_ms=" + now + " sequence=" + m_iRawEventCount +
			" input=" + name + " change=" + change + " raw=" + raw + " previous_raw=" + previous +
			" had_previous=" + m_bRawObserved + " held=" + held + " pressed_mask=" + pressedMask +
			" count_bits=" + (raw & countMask) + " previous_count_bits=" + (previous & countMask) +
			" count_mask=" + countMask + " mouse_x=" + mouseX + " mouse_y=" + mouseY);
	}

	protected void ObserveRawInput(float now)
	{
		if (m_iRawEventCount > 256)
			return;
		ObserveRawChannel(0, "mouse_left", Debug.GetMouseState(MouseState.LEFT), Debug.MB_PRESSED_MASK, 0x7fffffff, now);
		ObserveRawChannel(1, "mouse_right", Debug.GetMouseState(MouseState.RIGHT), Debug.MB_PRESSED_MASK, 0x7fffffff, now);
		ObserveRawChannel(2, "mouse_middle", Debug.GetMouseState(MouseState.MIDDLE), Debug.MB_PRESSED_MASK, 0x7fffffff, now);
		ObserveRawChannel(3, "mouse_wheel", Debug.GetMouseState(MouseState.WHEEL), 0, 0, now);
		ObserveRawChannel(4, "key_escape", Debug.KeyState(KeyCode.KC_ESCAPE), 0x8000, 0x7fff, now);
		ObserveRawChannel(5, "key_up", Debug.KeyState(KeyCode.KC_UP), 0x8000, 0x7fff, now);
		ObserveRawChannel(6, "key_w", Debug.KeyState(KeyCode.KC_W), 0x8000, 0x7fff, now);
		ObserveRawChannel(7, "key_m", Debug.KeyState(KeyCode.KC_M), 0x8000, 0x7fff, now);
		m_bRawObserved = true;
	}

	protected string DescribeInputWidget(Widget widget)
	{
		if (!widget)
			return "none";
		float x, y, width, height;
		widget.GetScreenPos(x, y);
		widget.GetScreenSize(width, height);
		string description = "widget=" + widget;
		description += ",name='" + widget.GetName() + "'";
		description += ",type=" + widget.GetTypeName();
		description += ",flags=" + widget.GetFlags();
		description += ",enabled=" + widget.IsEnabledInHierarchy();
		description += ",visible=" + widget.IsVisibleInHierarchy();
		description += ",handlers=" + widget.GetNumHandlers();
		description += ",rect=" + x + "," + y + "," + width + "," + height;
		return description;
	}

	protected void LogInputBindings(InputManager input, float now, string reason)
	{
		if (m_iBindingSnapshots >= 4)
		{
			if (m_iBindingSnapshots == 4)
				Print("[ConvoyFollower] PANEL_INPUT_BINDING_LIMIT: world_ms=" + now + " limit=4");
			m_iBindingSnapshots = 5;
			return;
		}
		m_iBindingSnapshots++;
		ref array<string> actions = {"CarThrust", "CarSteering", "CarBrake", "CarHandBrake", "GadgetMap"};
		foreach (string action : actions)
		{
			// Default query plus the first three explicit slots, not an assertion
			// that every possible binding/device/preset has been enumerated.
			for (int bindingIndex = -1; bindingIndex < 3; bindingIndex++)
			{
				ref array<string> keys = {};
				ref array<BaseContainer> filters = {};
				bool found = input.GetActionKeybinding(action, keys, filters, EInputDeviceType.INVALID, string.Empty, bindingIndex);
				string keyNames;
				foreach (string key : keys)
					keyNames += "[" + key + "]";
				string filterNames;
				foreach (BaseContainer filter : filters)
				{
					if (filter)
						filterNames += "[" + filter.GetClassName() + "]";
					else
						filterNames += "[null]";
				}
				Print("[ConvoyFollower] PANEL_INPUT_BINDING: world_ms=" + now + " snapshot=" + m_iBindingSnapshots +
					" reason=" + reason + " action=" + action + " active=" + input.IsActionActive(action) +
					" index=" + bindingIndex + " device=default preset=default found=" + found +
					" keys='" + keyNames + "' filters='" + filterNames + "'");
				if (bindingIndex >= 0 && !found)
					break;
			}
		}
	}

	protected void ObserveInputRouting(InputManager input, MenuBase mapMenu, MenuBase topMenu, float now)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		Widget focused, modal, panel, holdButton, closeButton;
		if (workspace)
		{
			focused = workspace.GetFocusedWidget();
			modal = workspace.GetModal();
			panel = workspace.FindAnyWidget("CF_ConvoyPanel");
			if (panel)
			{
				holdButton = panel.FindAnyWidget("CF_Hold");
				closeButton = panel.FindAnyWidget("CF_Close");
			}
		}
		bool topFocused = topMenu && topMenu.IsFocused();
		bool mapFocused = mapMenu && mapMenu.IsFocused();
		bool thrustActive = input.IsActionActive("CarThrust");
		bool steeringActive = input.IsActionActive("CarSteering");
		bool mapActionActive = input.IsActionActive("GadgetMap");
		string state = " keyboard_mouse_preferred=" + input.IsUsingMouseAndKeyboard();
		state += " last_device=" + input.GetLastUsedInputDevice();
		state += " focused={" + DescribeInputWidget(focused) + "}";
		state += " modal={" + DescribeInputWidget(modal) + "}";
		state += " panel={" + DescribeInputWidget(panel) + "}";
		state += " hold={" + DescribeInputWidget(holdButton) + "}";
		state += " close={" + DescribeInputWidget(closeButton) + "}";
		state += " top_menu=" + topMenu;
		state += " top_focused=" + topFocused;
		state += " map_focused=" + mapFocused;
		state += " thrust_active=" + thrustActive;
		state += " steering_active=" + steeringActive;
		state += " map_action_active=" + mapActionActive;
		bool changed = state != m_sLastRouting;
		m_sLastRouting = state;
		if (!changed && now - m_fLastRoutingMs < 5000)
			return;
		m_fLastRoutingMs = now;
		if (m_iRoutingEventCount >= 64)
		{
			if (m_iRoutingEventCount == 64)
				Print("[ConvoyFollower] PANEL_INPUT_ROUTING_LIMIT: world_ms=" + now + " limit=64 later_routing_unobserved=true");
			m_iRoutingEventCount = 65;
			return;
		}
		m_iRoutingEventCount++;
		int mouseX, mouseY;
		WidgetManager.GetMousePos(mouseX, mouseY);
		Print("[ConvoyFollower] PANEL_INPUT_ROUTING: world_ms=" + now + " sequence=" + m_iRoutingEventCount +
			" changed=" + changed + " mouse_x=" + mouseX + " mouse_y=" + mouseY + state);
	}

	// Local standalone observation only. Read every rendered frame so short
	// axis pulses are retained as interval extrema; emit at most four Hz.
	// POSTFRAME control readbacks are not before-physics or powered-drive proof.
	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (m_bObservationStopped || !Replication.IsServer())
			return;
		if (!GetGame() || !GetGame().GetWorld() || CF_ConvoySession.CF_IsWorldCleanup() || !m_Lead)
		{
			StopObservation();
			return;
		}
		float now = GetGame().GetWorld().GetWorldTime();
		if (now - m_fObservationStartMs >= 240000)
		{
			Print("[ConvoyFollower] PANEL_FIXTURE_END: observation_240000ms_bound world_ms=" + now);
			StopObservation();
			return;
		}
		PlayerController local = GetGame().GetPlayerController();
		InputManager input = GetGame().GetInputManager();
		if (!local || !input)
			return;
		if (m_ObservedInput != input)
		{
			if (m_ObservedInput)
				m_ObservedInput.RemoveActionListener("GadgetMap", EActionTrigger.DOWN, ObserveMapAction);
			m_ObservedInput = input;
			input.AddActionListener("GadgetMap", EActionTrigger.DOWN, ObserveMapAction);
			m_iLastObservedDevice = input.GetLastUsedInputDevice();
			LogInputBindings(input, now, "input_manager_attached");
			Print("[ConvoyFollower] PANEL_INPUT_OBSERVER: local=true actions=CarThrust,CarSteering,CarBrake,CarHandBrake,GadgetMap sample=POSTFRAME write_controls=false");
		}
		ObserveRawInput(now);
		float thrust = input.GetActionValue("CarThrust");
		float steering = input.GetActionValue("CarSteering");
		if (thrust > m_fThrustPeak) m_fThrustPeak = thrust;
		if (steering < m_fSteeringMin) m_fSteeringMin = steering;
		if (steering > m_fSteeringMax) m_fSteeringMax = steering;
		m_fSampleSeconds += timeSlice;
		if (m_fSampleSeconds < 0.25)
			return;

		IEntity controlled = local.GetControlledEntity();
		ChimeraCharacter character = ChimeraCharacter.Cast(controlled);
		SCR_PlayerController scripted = SCR_PlayerController.Cast(local);
		AIControlComponent aiControl;
		CharacterControllerComponent characterControl;
		if (character)
		{
			aiControl = character.GetAIControlComponent();
			characterControl = character.GetCharacterController();
		}
		CompartmentAccessComponent access;
		BaseCompartmentSlot seat;
		IEntity vehicle;
		if (character) access = character.GetCompartmentAccessComponent();
		if (access)
		{
			seat = access.GetCompartment();
			vehicle = access.GetVehicleIn(character);
		}
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		VehicleWheeledSimulation sim;
		BaseCompartmentSlot pilotSlot;
		IEntity pilot;
		if (car)
		{
			sim = car.GetSimulation();
			pilotSlot = car.GetPilotCompartmentSlot();
			if (pilotSlot) pilot = pilotSlot.GetOccupant();
		}
		bool gettingIn = access && access.IsGettingIn();
		bool gettingOut = access && access.IsGettingOut();
		bool exactPilot = controlled && access && seat && seat == pilotSlot && seat.IsPiloting() &&
			pilot == controlled && vehicle == m_Lead && !gettingIn && !gettingOut;
		int playerId;
		PlayerManager players = GetGame().GetPlayerManager();
		if (players && controlled) playerId = players.GetPlayerIdFromControlledEntity(controlled);

		MenuManager menus = GetGame().GetMenuManager();
		MenuBase mapMenu;
		MenuBase topMenu;
		if (menus)
		{
			mapMenu = menus.FindMenuByPreset(ChimeraMenuPreset.MapMenu);
			topMenu = menus.GetTopMenu();
		}
		bool mapOpen = mapMenu && mapMenu.IsOpen();
		ObserveInputRouting(input, mapMenu, topMenu, now);
		int observedDevice = input.GetLastUsedInputDevice();
		if (observedDevice != m_iLastObservedDevice)
		{
			LogInputBindings(input, now, "input_device_changed");
			m_iLastObservedDevice = observedDevice;
		}
		if (mapOpen != m_bLastMapOpen)
		{
			LogInputBindings(input, now, "map_open_changed");
			Print("[ConvoyFollower] PANEL_INPUT_MENU: world_ms=" + now + " map_open=" + mapOpen +
				" auto_open_enabled=" + m_bAutoOpenMap + " fixture_auto_requested=" + m_bMenuRequested +
				" map_action_count=" + m_iMapActionCount + " opening_cause=correlate_external_input_record");
			m_bLastMapOpen = mapOpen;
		}
		vector position = m_Lead.GetOrigin();
		if (!m_bPoseCaptured && exactPilot && playerId > 0)
		{
			m_vInputForward = m_Lead.GetWorldTransformAxis(2);
			m_vInputForward[1] = 0;
			if (m_vInputForward.LengthSq() > 0.5)
			{
				m_vInputForward.Normalize();
				m_vInputStart = position;
				m_vInputPrevious = position;
				m_InitialPilot = controlled;
				m_bPoseCaptured = true;
				Print("[ConvoyFollower] PANEL_INPUT_ORIGIN: world_ms=" + now + " player=" + controlled +
					" truck=" + m_Lead + " position=" + position + " forward=" + m_vInputForward);
			}
		}
		vector displacement = position - m_vInputStart;
		vector delta = position - m_vInputPrevious;
		float progress = displacement[0] * m_vInputForward[0] + displacement[2] * m_vInputForward[2];
		float step = delta[0] * m_vInputForward[0] + delta[2] * m_vInputForward[2];
		float distance;
		if (m_bPoseCaptured) distance = vector.DistanceXZ(position, m_vInputStart);
		Print("[ConvoyFollower] PANEL_INPUT_CONTROL: world_ms=" + now + " scripted_controller=" + (scripted != null) +
			" possessing=" + (scripted && scripted.IsPossessing()) + " controlled_is_main=" + (scripted && scripted.GetMainEntity() == controlled) +
			" ai_component=" + (aiControl != null) + " ai_active=" + (aiControl && aiControl.IsAIActivated()) +
			" character_controller=" + (characterControl != null) +
			" movement_disabled=" + (characterControl && characterControl.GetDisableMovementControls()) +
			" view_disabled=" + (characterControl && characterControl.GetDisableViewControls()) +
			" any_menu_open=" + (menus && menus.IsAnyMenuOpen()) + " editor_open=" + SCR_EditorManagerEntity.IsOpenedInstance());
		Print("[ConvoyFollower] PANEL_INPUT_SAMPLE: world_ms=" + now + " interval_s=" + m_fSampleSeconds +
			" controlled=" + controlled + " player_id=" + playerId + " same_initial_player=" + (m_bPoseCaptured && controlled == m_InitialPilot) +
			" pilot=" + pilot + " seat=" + seat + " vehicle=" + vehicle + " exact_lead_pilot=" + exactPilot +
			" getting_in=" + gettingIn + " getting_out=" + gettingOut + " top_menu=" + topMenu + " map_open=" + mapOpen +
			" car_context=" + input.IsContextActive("CarContext") + " map_context=" + input.IsContextActive("MapContext") +
			" gadget_map_context=" + input.IsContextActive("GadgetMapContext") + " last_input_device=" + input.GetLastUsedInputDevice() +
			" thrust=" + thrust + " thrust_peak=" + m_fThrustPeak + " steering=" + steering +
			" steering_min=" + m_fSteeringMin + " steering_max=" + m_fSteeringMax +
			" brake_action=" + input.GetActionValue("CarBrake") + " handbrake_action=" + input.GetActionValue("CarHandBrake") +
			" map_action=" + input.GetActionValue("GadgetMap") + " map_action_count=" + m_iMapActionCount +
			" pose_valid=" + m_bPoseCaptured + " position=" + position + " forward=" + m_Lead.GetWorldTransformAxis(2) +
			" signed_progress_m=" + progress + " signed_step_m=" + step + " displacement_m=" + distance);
		if (sim)
			Print("[ConvoyFollower] PANEL_INPUT_SIM: world_ms=" + now + " throttle=" + sim.GetThrottle() +
				" brake=" + sim.GetBrake() + " persistent_handbrake=" + car.GetPersistentHandBrake() +
				" gear=" + sim.GetGear() + " clutch=" + sim.GetClutch() + " engine=" + sim.EngineIsOn() +
				" rpm=" + sim.EngineGetRPM() + " steering=" + sim.GetSteering() + " speed_kmh=" + sim.GetSpeedKmh());
		else
			Print("[ConvoyFollower] PANEL_INPUT_SIM: world_ms=" + now + " unavailable=true");
		m_vInputPrevious = position;
		m_fSampleSeconds = 0;
		m_fThrustPeak = 0;
		m_fSteeringMin = 0;
		m_fSteeringMax = 0;
	}

	protected bool EnsurePlayer()
	{
		PlayerManager manager = GetGame().GetPlayerManager();
		if (!manager)
			return false;
		ref array<int> playerIds = {};
		manager.GetPlayers(playerIds);
		if (playerIds.IsEmpty())
			return false;
		PlayerController controller = manager.GetPlayerController(playerIds[0]);
		if (!controller)
			return false;
		m_Player = ChimeraCharacter.Cast(controller.GetControlledEntity());
		if (!m_Player)
		{
			Resource prefab = Resource.Load("{E1CB513B8B9B08F4}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_US_Crew.et");
			if (!prefab.IsValid())
				return false;
			EntitySpawnParams params = EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = m_Lead.GetOrigin() + Vector(3, 0, 2);
			m_Player = ChimeraCharacter.Cast(GetGame().SpawnEntityPrefab(prefab, GetGame().GetWorld(), params));
			if (!m_Player || !controller.SetControlledEntity(m_Player))
				return false;
			Print("[ConvoyFollower] PANEL_FIXTURE_PLAYER: possessed US crew");
		}
		return manager.GetPlayerIdFromControlledEntity(m_Player) > 0;
	}

	protected bool EnsurePilotSeat()
	{
		CompartmentAccessComponent access = m_Player.GetCompartmentAccessComponent();
		if (!access)
			return false;
		BaseCompartmentSlot current = access.GetCompartment();
		if (current && current.IsPiloting() && access.GetVehicleIn(m_Player) == m_Lead)
			return true;
		BaseCompartmentManagerComponent manager = BaseCompartmentManagerComponent.Cast(m_Lead.FindComponent(BaseCompartmentManagerComponent));
		if (!manager)
			return false;
		ref array<BaseCompartmentSlot> slots = {};
		manager.GetCompartments(slots);
		foreach (BaseCompartmentSlot slot : slots)
		{
			if (slot && slot.IsPiloting() && !slot.IsOccupied() && !slot.IsReserved())
			{
				if (access.GetInVehicle(m_Lead, slot, true, -1, ECloseDoorAfterActions.CLOSE_DOOR, true))
					Print("[ConvoyFollower] PANEL_FIXTURE_SEAT: owner pilot seat requested");
				break;
			}
		}
		return false;
	}

	protected CF_DriverControllerComponent FindDriver(int index)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokeGroup" + (index + 1)));
		if (!group)
			return null;
		ref array<AIAgent> agents = {};
		group.GetAgents(agents);
		if (agents.Count() != 1 || !agents[0])
			return null;
		IEntity character = agents[0].GetControlledEntity();
		if (!character)
			return null;
		return CF_DriverControllerComponent.Cast(character.FindComponent(CF_DriverControllerComponent));
	}

	protected void SelectLocalPlayerCamera()
	{
		if (m_bCameraSelected || !m_Player || m_iCameraAttempts >= 10)
			return;
		PlayerController local = GetGame().GetPlayerController();
		if (!local || local.GetControlledEntity() != m_Player)
			return;
		m_iCameraAttempts++;
		bool editorWasOpen = SCR_EditorManagerEntity.IsOpenedInstance();
		bool editorClosed = !editorWasOpen || SCR_EditorManagerEntity.CloseInstance();
		PlayerCamera playerCamera = local.GetPlayerCamera();
		bool cameraSelected = playerCamera && GetGame().GetCameraManager().SetCamera(playerCamera);
		if (cameraSelected)
		{
			CameraHandlerComponent handler = CameraHandlerComponent.Cast(m_Player.FindComponent(CameraHandlerComponent));
			if (handler)
				handler.SetThirdPerson(true);
		}
		m_bCameraSelected = editorClosed && cameraSelected;
		Print("[ConvoyFollower] PANEL_FIXTURE_CAMERA: editor_open=" + editorWasOpen + " editor_closed=" + editorClosed + " player_camera=" + cameraSelected);
	}

	protected void Poll()
	{
		if (m_bObservationStopped || CF_ConvoySession.CF_IsWorldCleanup())
		{
			StopObservation();
			return;
		}
		m_iTicks++;
		if (!m_Lead || GetGame().GetWorld().GetWorldTime() - m_fObservationStartMs >= 240000 || m_iTicks > 240)
		{
			Print("[ConvoyFollower] PANEL_FIXTURE_END: lead missing or 240 second timeout");
			StopObservation();
			return;
		}
		if (!EnsurePlayer() || (!m_bReady && !EnsurePilotSeat()))
			return;
		SelectLocalPlayerCamera();
		while (m_Drivers.Count() < m_iFollowerCount)
		{
			CF_DriverControllerComponent driver = FindDriver(m_Drivers.Count());
			if (!driver)
				return;
			m_Drivers.Insert(driver);
		}
		if (m_iNextOrder > 0 && !m_Drivers[m_iNextOrder - 1].CF_IsActiveConvoyMember())
			return;
		if (m_iNextOrder < m_iFollowerCount)
		{
			bool accepted;
			if (m_iNextOrder == 0)
				accepted = CF_ConvoySession.Start(m_Player, m_Drivers[m_iNextOrder]);
			else
				accepted = CF_ConvoySession.Add(m_Player, m_Drivers[m_iNextOrder]);
			Print("[ConvoyFollower] PANEL_FIXTURE_ORDER: unit=" + (m_iNextOrder + 1) + " accepted=" + accepted);
			if (accepted)
				m_iNextOrder++;
			return;
		}
		if (!m_bReady)
		{
			m_bReady = true;
			m_iReadyTick = m_iTicks;
			Print("[ConvoyFollower] PANEL_FIXTURE_READY: owner in lead pilot seat, convoy recruited");
		}
		// Use the game's menu manager so MapMenu creates its widget root and
		// input context before SCR_MapEntity.OnMapOpen is invoked. The direct
		// SCR_MapEntity.OpenMap call cannot initialize RootWidgetRef by itself.
		if (m_bAutoOpenMap && !m_bMenuRequested && m_iTicks - m_iReadyTick >= 3)
		{
			MenuManager menuManager = GetGame().GetMenuManager();
			if (menuManager)
			{
				m_bMenuRequested = true;
				menuManager.OpenMenu(ChimeraMenuPreset.MapMenu);
				Print("[ConvoyFollower] PANEL_FIXTURE_MENU_REQUESTED: native MapMenu source=fixture_auto_open world_ms=" + GetGame().GetWorld().GetWorldTime());
			}
		}
		if (m_iTicks % 10 == 0)
			Print("[ConvoyFollower] PANEL_FIXTURE_STATE: " + CF_ConvoySession.CF_GetOwnerPanelSnapshot(m_Player) + " order=" + CF_ConvoySession.CF_GetOwnerPanelOrderState(m_Player));
	}
}
