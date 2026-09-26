// Test-only owner-pilot panel fixture. Attach this to the lead M923 in a
// separate OpenRoad world; it never drives the truck or simulates UI clicks.
class CF_PanelPilotProbeComponentClass : ScriptComponentClass
{
}

class CF_PanelPilotProbeComponent : ScriptComponent
{
	[Attribute(defvalue: "1", params: "1 2 1", desc: "Number of staged followers to recruit")]
	protected int m_iFollowerCount;
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

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Lead = Vehicle.Cast(owner);
		Print("[ConvoyFollower] PANEL_FIXTURE_INIT: followers=" + m_iFollowerCount);
		GetGame().GetCallqueue().CallLater(Poll, 1000, true);
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
		m_iTicks++;
		if (!m_Lead || m_iTicks > 240)
		{
			Print("[ConvoyFollower] PANEL_FIXTURE_END: lead missing or 240 second timeout");
			GetGame().GetCallqueue().Remove(Poll);
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
		if (!m_bMenuRequested && m_iTicks - m_iReadyTick >= 3)
		{
			MenuManager menuManager = GetGame().GetMenuManager();
			if (menuManager)
			{
				m_bMenuRequested = true;
				menuManager.OpenMenu(ChimeraMenuPreset.MapMenu);
				Print("[ConvoyFollower] PANEL_FIXTURE_MENU_REQUESTED: native MapMenu");
			}
		}
		if (m_iTicks % 10 == 0)
			Print("[ConvoyFollower] PANEL_FIXTURE_STATE: " + CF_ConvoySession.CF_GetOwnerPanelSnapshot(m_Player) + " order=" + CF_ConvoySession.CF_GetOwnerPanelOrderState(m_Player));
	}
}
