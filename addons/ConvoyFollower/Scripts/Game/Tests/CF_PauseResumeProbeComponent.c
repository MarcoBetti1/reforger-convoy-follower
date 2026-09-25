// Test-only companion for the OpenRoad 2Trucks PauseResume world. The normal
// smoke probe recruits the chain and owns the route gate; this component
// briefly stops that route and verifies the owner's real exit/reboard flow.
class CF_PauseResumeProbeComponentClass : ScriptComponentClass
{
}

class CF_PauseResumeProbeComponent : ScriptComponent
{
	protected Vehicle m_Lead;
	protected CF_SmokeProbeComponent m_RouteProbe;
	protected ChimeraCharacter m_Player;
	protected SCR_AIGroup m_PilotGroup;
	protected AIWaypoint m_SavedPilotWaypoint;
	protected ref array<CF_DriverControllerComponent> m_Drivers = {};
	protected ref array<Vehicle> m_Trucks = {};
	protected ref array<vector> m_LastTruckPositions = {};
	protected ref array<vector> m_PausedTruckPositions = {};
	protected ref array<float> m_ResumeTruckPaths = {};
	protected ref array<float> m_CurrentTruckSteps = {};
	protected vector m_vLastLead;
	protected vector m_vPausedLead;
	protected float m_fLeadPath;
	protected float m_fResumeLeadPath;
	protected int m_iStage;
	protected int m_iStageTicks;
	protected int m_iSettledSeconds;
	protected bool m_bMaintainBrake;
	protected bool m_bFinished;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Lead = Vehicle.Cast(owner);
		m_RouteProbe = CF_SmokeProbeComponent.Cast(owner.FindComponent(CF_SmokeProbeComponent));
		m_vLastLead = owner.GetOrigin();
		SetEventMask(owner, EntityEvent.POSTFRAME);
		GetGame().GetCallqueue().CallLater(Poll, 1000, true);
		Print("[ConvoyFollower] AUTO_PAUSE_INIT: awaiting two-truck moving convoy");
	}

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (!m_bMaintainBrake || !m_Lead)
			return;
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (!car)
			return;
		car.SetPersistentHandBrake(true);
		VehicleWheeledSimulation sim = car.GetSimulation();
		if (sim)
		{
			sim.SetThrottle(0);
			sim.SetBreak(1, true);
		}
	}

	protected void Finish(string result)
	{
		if (m_bFinished)
			return;
		m_bFinished = true;
		Print("[ConvoyFollower] AUTO_PAUSE_RESULT: " + result);
		GetGame().GetCallqueue().Remove(Poll);
	}

	protected CF_DriverControllerComponent FindDriver(int unit)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokeGroup" + unit));
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

	protected bool IsPlayerPassenger()
	{
		if (!m_Player)
			return false;
		CompartmentAccessComponent access = m_Player.GetCompartmentAccessComponent();
		if (!access)
			return false;
		BaseCompartmentSlot slot = access.GetCompartment();
		return slot && !slot.IsPiloting() && access.GetVehicleIn(m_Player) == m_Lead;
	}

	protected bool StartReboard()
	{
		CompartmentAccessComponent access = m_Player.GetCompartmentAccessComponent();
		if (!access || access.GetCompartment())
			return false;
		BaseCompartmentManagerComponent manager = BaseCompartmentManagerComponent.Cast(m_Lead.FindComponent(BaseCompartmentManagerComponent));
		if (!manager)
			return false;
		ref array<BaseCompartmentSlot> slots = {};
		manager.GetCompartments(slots);
		foreach (BaseCompartmentSlot slot : slots)
		{
			if (slot && !slot.IsPiloting() && !slot.IsOccupied() && !slot.IsReserved())
				return access.GetInVehicle(m_Lead, slot, true, -1, ECloseDoorAfterActions.CLOSE_DOOR, true);
		}
		return false;
	}

	protected bool DriversStillAssigned()
	{
		for (int i = 0; i < 2; i++)
		{
			if (!m_Drivers[i] || m_Drivers[i].CF_GetAssignedVehicle() != m_Trucks[i] ||
				!m_Drivers[i].CF_IsActiveConvoyMember())
				return false;
		}
		return true;
	}

	protected void Poll()
	{
		if (m_bFinished || !m_Lead || !m_RouteProbe)
		{
			if (!m_bFinished)
				Finish("FAIL lead or primary route probe missing");
			return;
		}
		m_iStageTicks++;
		float leadStep = vector.Distance(m_Lead.GetOrigin(), m_vLastLead);
		m_fLeadPath += leadStep;
		m_vLastLead = m_Lead.GetOrigin();
		if (m_iStage == 0)
		{
			PlayerManager players = GetGame().GetPlayerManager();
			ref array<int> ids = {};
			if (players)
				players.GetPlayers(ids);
			if (ids.IsEmpty())
				return;
			PlayerController controller = players.GetPlayerController(ids[0]);
			if (controller)
				m_Player = ChimeraCharacter.Cast(controller.GetControlledEntity());
			m_PilotGroup = SCR_AIGroup.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokePilotGroup"));
			if (!m_PilotGroup || !IsPlayerPassenger())
				return;
			for (int i = 1; i <= 2; i++)
			{
				CF_DriverControllerComponent driver = FindDriver(i);
				if (!driver || !driver.CF_IsActiveConvoyMember())
					return;
				Vehicle truck = driver.CF_GetAssignedVehicle();
				if (!truck)
					return;
				if (m_Drivers.Count() < i)
				{
					m_Drivers.Insert(driver);
					m_Trucks.Insert(truck);
					m_LastTruckPositions.Insert(truck.GetOrigin());
					m_PausedTruckPositions.Insert(truck.GetOrigin());
					m_ResumeTruckPaths.Insert(0);
					m_CurrentTruckSteps.Insert(0);
				}
			}
			if (m_iStageTicks >= 180)
			{
				Finish("FAIL initial convoy never reached the pause distance");
				return;
			}
			if (m_fLeadPath < 90.0)
				return;
			m_SavedPilotWaypoint = m_PilotGroup.GetCurrentWaypoint();
			if (!m_SavedPilotWaypoint)
			{
				Finish("FAIL moving pilot waypoint missing at stop point");
				return;
			}
			m_PilotGroup.RemoveWaypoint(m_SavedPilotWaypoint);
			m_bMaintainBrake = true;
			m_iStage = 1;
			m_iStageTicks = 0;
			Print("[ConvoyFollower] AUTO_PAUSE_STOP: lead braked after " + m_fLeadPath + " m");
			return;
		}
		if (!DriversStillAssigned())
		{
			Finish("FAIL convoy assignment or driver seat lost during pause test");
			return;
		}
		for (int j = 0; j < 2; j++)
		{
			m_CurrentTruckSteps[j] = vector.Distance(m_Trucks[j].GetOrigin(), m_LastTruckPositions[j]);
			m_LastTruckPositions[j] = m_Trucks[j].GetOrigin();
		}
		if (m_iStage == 1)
		{
			bool settled = m_iStageTicks >= 8 && leadStep <= 0.8 && m_CurrentTruckSteps[0] <= 0.8 && m_CurrentTruckSteps[1] <= 0.8 &&
				vector.Distance(m_Trucks[0].GetOrigin(), m_Lead.GetOrigin()) <= 45.0 &&
				vector.Distance(m_Trucks[1].GetOrigin(), m_Trucks[0].GetOrigin()) <= 45.0;
			if (settled)
				m_iSettledSeconds++;
			else
				m_iSettledSeconds = 0;
			if (m_iSettledSeconds >= 5)
			{
				CompartmentAccessComponent access = m_Player.GetCompartmentAccessComponent();
				if (!access || !access.GetOutVehicle(EGetOutType.TELEPORT, -1, ECloseDoorAfterActions.CLOSE_DOOR, true, true))
				{
					Finish("FAIL owner exit request rejected");
					return;
				}
				m_vPausedLead = m_Lead.GetOrigin();
				for (int k = 0; k < 2; k++)
					m_PausedTruckPositions[k] = m_Trucks[k].GetOrigin();
				m_iStage = 2;
				m_iStageTicks = 0;
				Print("[ConvoyFollower] AUTO_PAUSE_EXIT: owner exit requested after all three trucks stopped");
			}
			else if (m_iStageTicks >= 90)
				Finish("FAIL followers did not settle behind stopped lead");
			return;
		}
		if (m_iStage == 2)
		{
			CompartmentAccessComponent access = m_Player.GetCompartmentAccessComponent();
			if (!access || access.GetCompartment())
			{
				if (m_iStageTicks >= 10)
					Finish("FAIL owner remained seated after exit request");
				return;
			}
			if (vector.Distance(m_Lead.GetOrigin(), m_vPausedLead) > 3.0 ||
				vector.Distance(m_Trucks[0].GetOrigin(), m_PausedTruckPositions[0]) > 4.0 ||
				vector.Distance(m_Trucks[1].GetOrigin(), m_PausedTruckPositions[1]) > 4.0)
			{
				Finish("FAIL convoy moved while owner was on foot");
				return;
			}
			if (m_iStageTicks >= 15)
			{
				m_iStage = 3;
				m_iStageTicks = 0;
				Print("[ConvoyFollower] AUTO_PAUSE_HOLD: owner on foot for 15 s; both drivers retained seats and trucks held");
			}
			return;
		}
		if (m_iStage == 3)
		{
			if (!IsPlayerPassenger())
			{
				if (m_iStageTicks % 5 == 1)
						Print("[ConvoyFollower] AUTO_PAUSE_REBOARD_REQUEST: accepted=" + StartReboard());
				if (m_iStageTicks >= 30)
						Finish("FAIL owner did not reboard lead passenger seat");
				return;
			}
			m_bMaintainBrake = false;
			CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
			if (car)
			{
				car.SetPersistentHandBrake(false);
				VehicleWheeledSimulation sim = car.GetSimulation();
				if (sim)
					sim.SetBreak(0, true);
			}
			m_PilotGroup.AddWaypoint(m_SavedPilotWaypoint);
			m_fResumeLeadPath = m_fLeadPath;
			m_iStage = 4;
			m_iStageTicks = 0;
			Print("[ConvoyFollower] AUTO_PAUSE_REBOARD: owner reseated; original AI waypoint restored");
			return;
		}
		if (m_iStage == 4)
		{
			for (int k = 0; k < 2; k++)
				m_ResumeTruckPaths[k] = m_ResumeTruckPaths[k] + m_CurrentTruckSteps[k];
			if (m_RouteProbe.CF_HasPassedRoadArrival())
			{
				float resumedLead = m_fLeadPath - m_fResumeLeadPath;
				if (resumedLead < 100.0 || m_ResumeTruckPaths[0] < 70.0 || m_ResumeTruckPaths[1] < 70.0)
					Finish("FAIL primary road probe passed without enough resumed travel lead=" + resumedLead +
						" unit1=" + m_ResumeTruckPaths[0] + " unit2=" + m_ResumeTruckPaths[1]);
				else
					Finish("PASS owner exited and reboarded; both drivers stayed assigned and completed second road leg lead=" + resumedLead +
						" unit1=" + m_ResumeTruckPaths[0] + " unit2=" + m_ResumeTruckPaths[1]);
			}
			else if (m_iStageTicks >= 180)
				Finish("FAIL resumed road arrival timed out");
		}
	}
}
