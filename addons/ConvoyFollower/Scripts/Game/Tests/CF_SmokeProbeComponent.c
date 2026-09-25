// Test-only component used by the separate Arland_Auto worlds. It orders the
// production convoy session, drives the lead truck a short, bounded distance,
// and leaves numerical position evidence in the console log.
class CF_SmokeProbeComponentClass : ScriptComponentClass
{
}

class CF_SmokeProbeComponent : ScriptComponent
{
	[Attribute(defvalue: "1", params: "1 3 1", desc: "Number of staged follower trucks")]
	protected int m_iExpectedTrucks;

	protected Vehicle m_Lead;
	protected ChimeraCharacter m_Player;
	protected SCR_AIGroup m_PilotGroup;
	protected ChimeraCharacter m_Pilot;
	protected AIWaypoint m_PilotWaypoint;
	protected ref array<CF_DriverControllerComponent> m_Drivers = {};
	protected ref array<vector> m_InitialFollowerPositions = {};
	protected vector m_vInitialLeadPosition;
	protected int m_iStage;
	protected int m_iNextOrder;
	protected int m_iTicks;
	protected int m_iDrivingTicks;
	protected bool m_bFinished;
	protected static const int CF_MAX_TICKS = 150;
	protected static const int CF_MAX_DRIVING_TICKS = 45;
	protected static const float CF_TARGET_DISTANCE = 35.0;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;

		m_Lead = Vehicle.Cast(owner);
		CF_ConvoySettings.Get(); // Force test-only settings resolution in playerless server smoke.
		Print("[ConvoyFollower] AUTO_INIT: expected=" + m_iExpectedTrucks);
		GetGame().GetCallqueue().CallLater(Poll, 1000, true);
	}

	protected void Finish(string result)
	{
		if (m_bFinished)
			return;
		m_bFinished = true;
		VehicleWheeledSimulation sim;
		if (m_Lead)
		{
			CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
			if (car)
				sim = car.GetSimulation();
		}
		if (sim)
		{
			sim.SetThrottle(0);
			sim.SetBreak(1, true);
		}
		if (m_PilotGroup && m_PilotWaypoint)
			m_PilotGroup.RemoveWaypoint(m_PilotWaypoint);
		if (m_Lead)
			ClearEventMask(m_Lead, EntityEvent.FRAME);
		Print("[ConvoyFollower] AUTO_RESULT: " + result);
		GetGame().GetCallqueue().Remove(Poll);
	}

	protected CF_DriverControllerComponent FindDriver(int unitIndex)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokeGroup" + (unitIndex + 1)));
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
			{
				Finish("FAIL player character resource invalid");
				return false;
			}
			EntitySpawnParams params = EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = m_Lead.GetOrigin() + Vector(3, 0, 2);
			m_Player = ChimeraCharacter.Cast(GetGame().SpawnEntityPrefab(prefab, GetGame().GetWorld(), params));
			if (!m_Player || !controller.SetControlledEntity(m_Player))
			{
				Finish("FAIL player possession rejected");
				return false;
			}
			Print("[ConvoyFollower] AUTO_PLAYER: spawned and possessed test character");
		}
		// Local Workbench preview may honor this immediately; a remote client may
		// still require its own camera toggle. This is only a filming aid.
		CameraHandlerComponent camera = CameraHandlerComponent.Cast(m_Player.FindComponent(CameraHandlerComponent));
		if (camera)
			camera.SetThirdPerson(true);
		return manager.GetPlayerIdFromControlledEntity(m_Player) > 0;
	}

	protected bool EnsurePassengerSeat()
	{
		if (!m_Player || !m_Lead)
			return false;
		CompartmentAccessComponent access = m_Player.GetCompartmentAccessComponent();
		if (!access)
			return false;
		BaseCompartmentSlot current = access.GetCompartment();
		if (current && !current.IsPiloting() && access.GetVehicleIn(m_Player) == m_Lead)
			return true;
		BaseCompartmentManagerComponent manager = BaseCompartmentManagerComponent.Cast(m_Lead.FindComponent(BaseCompartmentManagerComponent));
		if (!manager)
			return false;
		ref array<BaseCompartmentSlot> slots = {};
		manager.GetCompartments(slots);
		foreach (BaseCompartmentSlot slot : slots)
		{
			if (slot && !slot.IsPiloting() && !slot.IsOccupied() && !slot.IsReserved())
			{
				if (access.GetInVehicle(m_Lead, slot, true, -1, ECloseDoorAfterActions.CLOSE_DOOR, true))
					Print("[ConvoyFollower] AUTO_SEAT: lead passenger seat requested");
				break;
			}
		}
		return false;
	}

	protected bool EnsureAIPilot()
	{
		if (!m_PilotGroup)
			m_PilotGroup = SCR_AIGroup.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokePilotGroup"));
		if (!m_PilotGroup)
			return false;
		if (!m_Pilot)
		{
			ref array<AIAgent> agents = {};
			m_PilotGroup.GetAgents(agents);
			if (agents.Count() != 1 || !agents[0])
				return false;
			m_Pilot = ChimeraCharacter.Cast(agents[0].GetControlledEntity());
			if (!m_Pilot)
				return false;
		}
		CompartmentAccessComponent access = m_Pilot.GetCompartmentAccessComponent();
		BaseCompartmentSlot slot;
		if (access)
			slot = access.GetCompartment();
		if (slot && slot.IsPiloting() && access.GetVehicleIn(m_Pilot) == m_Lead)
			return true;
		if (m_PilotWaypoint)
			return false;
		Resource prefab = Resource.Load("{8AD8C82346156494}Prefabs/AI/Waypoints/AIWaypoint_GetInSelected.et");
		if (!prefab.IsValid())
		{
			Finish("FAIL AI boarding waypoint resource invalid");
			return false;
		}
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = m_Lead.GetOrigin();
		SCR_BoardingEntityWaypoint waypoint = SCR_BoardingEntityWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(prefab, null, params));
		if (!waypoint)
		{
			Finish("FAIL AI boarding waypoint spawn failed");
			return false;
		}
		waypoint.SetEntity(m_Lead);
		waypoint.SetAllowance(true, false, false);
		waypoint.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_GAMEMASTER);
		waypoint.SetCompletionRadius(8.0);
		m_PilotWaypoint = waypoint;
		m_PilotGroup.AddWaypoint(waypoint);
		Print("[ConvoyFollower] AUTO_PILOT: AI boarding lead truck");
		return false;
	}

	protected bool StartAIDrive()
	{
		if (!m_PilotGroup || !m_Pilot)
			return false;
		if (m_PilotWaypoint)
			m_PilotGroup.RemoveWaypoint(m_PilotWaypoint);
		Resource prefab = Resource.Load("{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et");
		if (!prefab.IsValid())
			return false;
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = m_vInitialLeadPosition + Vector(30, 0, -30);
		SCR_AIWaypoint waypoint = SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(prefab, null, params));
		if (!waypoint)
			return false;
		waypoint.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_GAMEMASTER);
		waypoint.SetCompletionRadius(5.0);
		m_PilotWaypoint = waypoint;
		m_PilotGroup.AddWaypoint(waypoint);
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (car)
			car.SetPersistentHandBrake(false);
		Print("[ConvoyFollower] AUTO_PILOT: AI driving waypoint=" + params.Transform[3]);
		return true;
	}

	protected void LogPositions()
	{
		float leadDistance = vector.Distance(m_Lead.GetOrigin(), m_vInitialLeadPosition);
		Print("[ConvoyFollower] AUTO_POSITION: lead=" + m_Lead.GetOrigin() + " moved=" + leadDistance);
		for (int i = 0; i < m_Drivers.Count(); i++)
		{
			Vehicle truck = m_Drivers[i].CF_GetAssignedVehicle();
			if (!truck)
				continue;
			float moved = vector.Distance(truck.GetOrigin(), m_InitialFollowerPositions[i]);
			float gap = vector.Distance(truck.GetOrigin(), m_Lead.GetOrigin());
			Print("[ConvoyFollower] AUTO_POSITION: unit=" + (i + 1) + " pos=" + truck.GetOrigin() + " moved=" + moved + " lead_gap=" + gap);
		}
	}

	protected void Poll()
	{
		if (m_bFinished)
			return;
		m_iTicks++;
		if (m_iTicks > CF_MAX_TICKS)
		{
			Finish("FAIL timeout stage=" + m_iStage);
			return;
		}
		if (!m_Lead)
		{
			Finish("FAIL lead vehicle unavailable");
			return;
		}
		if (m_iStage == 0)
		{
			if (!EnsurePlayer() || !EnsurePassengerSeat() || !EnsureAIPilot())
				return;
			m_iStage = 1;
			Print("[ConvoyFollower] AUTO_READY: player passenger and AI pilot in lead truck");
		}
		if (m_iStage == 1)
		{
			while (m_Drivers.Count() < m_iExpectedTrucks)
			{
				CF_DriverControllerComponent driver = FindDriver(m_Drivers.Count());
				if (!driver)
					return;
				m_Drivers.Insert(driver);
				m_InitialFollowerPositions.Insert(Vector(0, 0, 0));
			}
			m_iStage = 2;
		}
		if (m_iStage == 2)
		{
			if (m_iNextOrder > 0 && !m_Drivers[m_iNextOrder - 1].CF_IsActiveConvoyMember())
				return;
			if (m_iNextOrder < m_iExpectedTrucks)
			{
				bool accepted;
				if (m_iNextOrder == 0)
					accepted = CF_ConvoySession.Start(m_Player, m_Drivers[m_iNextOrder]);
				else
					accepted = CF_ConvoySession.Add(m_Player, m_Drivers[m_iNextOrder]);
				Print("[ConvoyFollower] AUTO_ORDER: unit=" + (m_iNextOrder + 1) + " accepted=" + accepted);
				if (!accepted)
				{
					Finish("FAIL order rejected unit=" + (m_iNextOrder + 1));
					return;
				}
				m_iNextOrder++;
				return;
			}
			m_vInitialLeadPosition = m_Lead.GetOrigin();
			for (int i = 0; i < m_Drivers.Count(); i++)
			{
				Vehicle truck = m_Drivers[i].CF_GetAssignedVehicle();
				if (!truck)
				{
					Finish("FAIL unit truck missing unit=" + (i + 1));
					return;
				}
				m_InitialFollowerPositions[i] = truck.GetOrigin();
			}
			vector heading = m_Lead.GetTransformAxis(2);
			vector separation = m_Lead.GetOrigin() - m_InitialFollowerPositions[0];
			float ahead = heading[0] * separation[0] + heading[2] * separation[2];
			if (ahead < 5.0)
			{
				Finish("FAIL unsafe heading; lead not in front of follower");
				return;
			}
			CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
			if (!car || !car.GetSimulation())
			{
				Finish("FAIL lead vehicle simulation or engine unavailable");
				return;
			}
			if (!StartAIDrive())
			{
				Finish("FAIL AI lead drive waypoint unavailable");
				return;
			}
			m_iStage = 3;
			Print("[ConvoyFollower] AUTO_DRIVE: AI pilot short route bounded 35 m test started");
		}
		if (m_iStage == 3)
		{
			m_iDrivingTicks++;
			if (m_iDrivingTicks % 5 == 0)
			{
				LogPositions();
				CarControllerComponent diagnosticCar = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
				VehicleWheeledSimulation diagnosticSim = diagnosticCar.GetSimulation();
				float wheelSpeed;
				if (diagnosticSim.WheelCount() > 0)
					wheelSpeed = diagnosticSim.WheelGetAngularSpeed(0);
				Print("[ConvoyFollower] AUTO_CONTROLS: engine=" + diagnosticSim.EngineIsOn() + " rpm=" + diagnosticSim.EngineGetRPM() + " gear=" + diagnosticSim.GetGear() + " throttle=" + diagnosticSim.GetThrottle() + " brake=" + diagnosticSim.GetBrake() + " clutch=" + diagnosticSim.GetClutch() + " wheel0=" + wheelSpeed + " handbrake=" + diagnosticCar.GetHandBrake() + " persistent=" + diagnosticCar.GetPersistentHandBrake());
			}
			float moved = vector.Distance(m_Lead.GetOrigin(), m_vInitialLeadPosition);
			if (moved >= CF_TARGET_DISTANCE || m_iDrivingTicks >= CF_MAX_DRIVING_TICKS)
			{
				LogPositions();
				int movingFollowers = 0;
				for (int i = 0; i < m_Drivers.Count(); i++)
				{
					Vehicle truck = m_Drivers[i].CF_GetAssignedVehicle();
					if (truck && vector.Distance(truck.GetOrigin(), m_InitialFollowerPositions[i]) > 5.0)
						movingFollowers++;
				}
				if (moved < 20.0)
					Finish("FAIL lead moved less than 20 m");
				else if (movingFollowers < m_iExpectedTrucks)
					Finish("FAIL follower movement " + movingFollowers + "/" + m_iExpectedTrucks);
				else
					Finish("PASS lead moved=" + moved + " moving_followers=" + movingFollowers);
			}
		}
	}

}
