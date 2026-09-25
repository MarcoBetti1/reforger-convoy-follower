// Test-only companion for the two-truck open-road world. The normal smoke
// probe drives and orders the convoy; this component exercises the explicit
// unload release, bay advance, and manual return action after its road stop.
class CF_UnloadSequenceProbeComponentClass : ScriptComponentClass
{
}

class CF_UnloadSequenceProbeComponent : ScriptComponent
{
	protected Vehicle m_Lead;
	protected Vehicle m_Follower1;
	protected Vehicle m_Follower2;
	protected ChimeraCharacter m_Player;
	protected CF_DriverControllerComponent m_Driver1;
	protected CF_DriverControllerComponent m_Driver2;
	protected SCR_AIGroup m_PilotGroup;
	protected CF_SmokeProbeComponent m_RouteProbe;
	protected AIWaypoint m_ReturnWaypoint;
	protected vector m_vLastLead;
	protected vector m_vLastFollower1;
	protected vector m_vLastFollower2;
	protected vector m_vBayPosition;
	protected vector m_vFollower2AtRelease;
	protected vector m_vReturnGoal;
	protected float m_fLeadPath;
	protected float m_fFollower1Path;
	protected float m_fFollower2Path;
	protected float m_fFollower1Step;
	protected float m_fFollower2Step;
	protected float m_fReturnLeadPath;
	protected float m_fReturnFollower1Path;
	protected float m_fReturnFollower2Path;
	protected int m_iStage;
	protected int m_iStageTicks;
	protected int m_iStillTicks;
	protected bool m_bPositionsReady;
	protected bool m_bFinished;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Lead = Vehicle.Cast(owner);
		Print("[ConvoyFollower] AUTO_SEQUENCE_INIT: two-truck release and return companion loaded");
		GetGame().GetCallqueue().CallLater(Poll, 1000, true);
	}

	protected CF_DriverControllerComponent FindDriver(int unitNumber)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokeGroup" + unitNumber));
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

	protected ChimeraCharacter FindTestPlayer()
	{
		PlayerManager manager = GetGame().GetPlayerManager();
		if (!manager)
			return null;
		ref array<int> ids = {};
		manager.GetPlayers(ids);
		if (ids.IsEmpty())
			return null;
		return ChimeraCharacter.Cast(manager.GetPlayerControlledEntity(ids[0]));
	}

	protected void SetStage(int stage)
	{
		m_iStage = stage;
		m_iStageTicks = 0;
	}

	protected void Finish(string result)
	{
		if (m_bFinished)
			return;
		m_bFinished = true;
		if (m_PilotGroup && m_ReturnWaypoint)
			m_PilotGroup.RemoveWaypoint(m_ReturnWaypoint);
		Print("[ConvoyFollower] AUTO_SEQUENCE_RESULT: " + result);
		GetGame().GetCallqueue().Remove(Poll);
	}

	protected bool ResolveEntities()
	{
		if (!m_Lead)
			return false;
		BaseWorld world = GetGame().GetWorld();
		if (!m_Follower1)
			m_Follower1 = Vehicle.Cast(world.FindEntityByName("CF_SmokeFollower1"));
		if (!m_Follower2)
			m_Follower2 = Vehicle.Cast(world.FindEntityByName("CF_SmokeFollower2"));
		if (!m_Driver1)
			m_Driver1 = FindDriver(1);
		if (!m_Driver2)
			m_Driver2 = FindDriver(2);
		if (!m_Player)
			m_Player = FindTestPlayer();
		if (!m_PilotGroup)
			m_PilotGroup = SCR_AIGroup.Cast(world.FindEntityByName("CF_SmokePilotGroup"));
		if (!m_RouteProbe)
			m_RouteProbe = CF_SmokeProbeComponent.Cast(m_Lead.FindComponent(CF_SmokeProbeComponent));
		if (!m_Follower1 || !m_Follower2 || !m_Driver1 || !m_Driver2 || !m_Player || !m_PilotGroup || !m_RouteProbe)
			return false;
		if (!m_bPositionsReady)
		{
			m_vLastLead = m_Lead.GetOrigin();
			m_vLastFollower1 = m_Follower1.GetOrigin();
			m_vLastFollower2 = m_Follower2.GetOrigin();
			m_bPositionsReady = true;
		}
		return true;
	}

	protected void UpdatePaths()
	{
		vector lead = m_Lead.GetOrigin();
		vector follower1 = m_Follower1.GetOrigin();
		vector follower2 = m_Follower2.GetOrigin();
		float leadStep = vector.Distance(lead, m_vLastLead);
		float follower1Step = vector.Distance(follower1, m_vLastFollower1);
		float follower2Step = vector.Distance(follower2, m_vLastFollower2);
		m_fLeadPath += leadStep;
		m_fFollower1Path += follower1Step;
		m_fFollower1Step = follower1Step;
		m_fFollower2Step = follower2Step;
		m_fFollower2Path += follower2Step;
		if (m_iStage >= 4)
		{
			m_fReturnLeadPath += leadStep;
			m_fReturnFollower1Path += follower1Step;
			m_fReturnFollower2Path += follower2Step;
		}
		if (leadStep < 0.75)
			m_iStillTicks++;
		else
			m_iStillTicks = 0;
		m_vLastLead = lead;
		m_vLastFollower1 = follower1;
		m_vLastFollower2 = follower2;
	}

	protected void LogStatus()
	{
		float gap1 = vector.Distance(m_Lead.GetOrigin(), m_Follower1.GetOrigin());
		float gap2 = vector.Distance(m_Follower1.GetOrigin(), m_Follower2.GetOrigin());
		Print("[ConvoyFollower] AUTO_SEQUENCE_STATUS: stage=" + m_iStage +
			" lead=" + m_Lead.GetOrigin() + " path=" + m_fLeadPath +
			" unit1=" + m_Follower1.GetOrigin() + " path=" + m_fFollower1Path +
			" unit2=" + m_Follower2.GetOrigin() + " path=" + m_fFollower2Path +
			" gaps=" + gap1 + "," + gap2 +
			" parked=" + m_Driver1.CF_IsAtUnloadWaitingPoint() +
			" return_blocked=" + m_Driver1.CF_IsReturnBlockedForActions() + "," +
			m_Driver2.CF_IsReturnBlockedForActions() +
			" unit2_ready=" + CF_ConvoySession.CanReleaseAtUnload(m_Player, m_Driver2));
	}

	// One-second test diagnostics during an explicit release. A valid road
	// point does not prove the AI has a working drive order or drivetrain.
	protected void LogReleaseNav()
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokeGroup1"));
		AIWaypoint waypoint;
		if (group)
			waypoint = group.GetCurrentWaypoint();
		vector goal;
		if (waypoint)
			goal = waypoint.GetOrigin();
		int roadId = -1;
		float roadDistance = -1.0;
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (aiWorld && aiWorld.GetRoadNetworkManager())
		{
			BaseRoad road;
			roadId = aiWorld.GetRoadNetworkManager().GetClosestRoad(m_Follower1.GetOrigin(), road, roadDistance);
		}
		CarControllerComponent car = CarControllerComponent.Cast(m_Follower1.FindComponent(CarControllerComponent));
		VehicleWheeledSimulation sim;
		if (car)
			sim = car.GetSimulation();
		bool engineOn;
		int gear;
		float rpm;
		float throttle;
		float brake;
		float clutch;
		if (sim)
		{
			engineOn = sim.EngineIsOn();
			gear = sim.GetGear();
			rpm = sim.EngineGetRPM();
			throttle = sim.GetThrottle();
			brake = sim.GetBrake();
			clutch = sim.GetClutch();
		}
		bool handBrake = car && car.GetHandBrake();
		bool persistentBrake = car && car.GetPersistentHandBrake();
		Print("[ConvoyFollower] AUTO_SEQUENCE_NAV: unit1=" + m_Follower1.GetOrigin() +
			" unit2=" + m_Follower2.GetOrigin() + " waypoint=" + waypoint +
			" goal=" + goal + " goal_gap=" + vector.Distance(m_Follower1.GetOrigin(), goal) +
			" forward=" + m_Follower1.GetWorldTransformAxis(2) +
			" step_m=" + m_fFollower1Step + " road_id=" + roadId +
			" road_dist=" + roadDistance + " engine=" + engineOn +
			" rpm=" + rpm + " gear=" + gear + " throttle=" + throttle +
			" brake=" + brake + " clutch=" + clutch + " handbrake=" + handBrake +
			" persistent=" + persistentBrake);
	}

	// The second truck must remain seated and clear of the release turn while
	// Unit One drives to its waiting slot. Capture its own AI and drivetrain
	// state each second; the front truck's telemetry cannot explain queue drift.
	protected void LogUnit2ReleaseNav()
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokeGroup2"));
		AIWaypoint waypoint;
		if (group)
			waypoint = group.GetCurrentWaypoint();
		vector goal;
		if (waypoint)
			goal = waypoint.GetOrigin();
		int roadId = -1;
		float roadDistance = -1.0;
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (aiWorld && aiWorld.GetRoadNetworkManager())
		{
			BaseRoad road;
			roadId = aiWorld.GetRoadNetworkManager().GetClosestRoad(m_Follower2.GetOrigin(), road, roadDistance);
		}
		CarControllerComponent car = CarControllerComponent.Cast(m_Follower2.FindComponent(CarControllerComponent));
		VehicleWheeledSimulation sim;
		if (car)
			sim = car.GetSimulation();
		bool engineOn;
		int gear;
		float rpm;
		float throttle;
		float brake;
		float clutch;
		if (sim)
		{
			engineOn = sim.EngineIsOn();
			gear = sim.GetGear();
			rpm = sim.EngineGetRPM();
			throttle = sim.GetThrottle();
			brake = sim.GetBrake();
			clutch = sim.GetClutch();
		}
		bool handBrake = car && car.GetHandBrake();
		bool persistentBrake = car && car.GetPersistentHandBrake();
		Print("[ConvoyFollower] AUTO_SEQUENCE_NAV_UNIT2: unit2=" + m_Follower2.GetOrigin() +
			" unit1=" + m_Follower1.GetOrigin() + " waypoint=" + waypoint +
			" goal=" + goal + " goal_gap=" + vector.Distance(m_Follower2.GetOrigin(), goal) +
			" forward=" + m_Follower2.GetWorldTransformAxis(2) +
			" step_m=" + m_fFollower2Step + " road_id=" + roadId +
			" road_dist=" + roadDistance + " engine=" + engineOn +
			" rpm=" + rpm + " gear=" + gear + " throttle=" + throttle +
			" brake=" + brake + " clutch=" + clutch + " handbrake=" + handBrake +
			" persistent=" + persistentBrake);
	}

	protected bool StartReturnDrive()
	{
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return false;
		RoadNetworkManager roads = aiWorld.GetRoadNetworkManager();
		vector desired = Vector(1285.5, 11.54, 3338.44);
		vector resolved;
		if (!roads.GetReachableWaypointInRoad(m_Lead.GetOrigin(), desired, 35.0, resolved) ||
			vector.Distance(desired, resolved) > 35.0 ||
			vector.Distance(m_Lead.GetOrigin(), resolved) < 100.0)
			return false;
		Resource prefab = Resource.Load("{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et");
		if (!prefab.IsValid())
			return false;
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = resolved;
		SCR_AIWaypoint waypoint = SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(prefab, null, params));
		if (!waypoint)
			return false;
		waypoint.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_GAMEMASTER);
		waypoint.SetCompletionRadius(5.0);
		AIWaypoint previous = m_PilotGroup.GetCurrentWaypoint();
		if (previous)
			m_PilotGroup.RemoveWaypoint(previous);
		m_ReturnWaypoint = waypoint;
		m_PilotGroup.AddWaypoint(waypoint);
		m_RouteProbe.CF_ReleaseLeadBrakeForReturn();
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (car)
		{
			car.SetPersistentHandBrake(false);
			VehicleWheeledSimulation sim = car.GetSimulation();
			if (sim)
				sim.SetBreak(0, true);
		}
		m_vReturnGoal = resolved;
		Print("[ConvoyFollower] AUTO_SEQUENCE_RETURN_DRIVE: goal=" + resolved + " from=" + m_Lead.GetOrigin());
		return true;
	}

	protected void Poll()
	{
		if (m_bFinished)
			return;
		m_iStageTicks++;
		if (!ResolveEntities())
		{
			if (m_iStageTicks >= 45)
				Finish("FAIL test player, group, or vehicle did not spawn");
			return;
		}
		UpdatePaths();
		if (m_iStageTicks % 5 == 0)
			LogStatus();

		if (m_iStage == 0)
		{
			float gap1 = vector.Distance(m_Lead.GetOrigin(), m_Follower1.GetOrigin());
			float gap2 = vector.Distance(m_Follower1.GetOrigin(), m_Follower2.GetOrigin());
			bool pilotStopped = !m_PilotGroup.GetCurrentWaypoint();
			if (m_RouteProbe.CF_HasPassedRoadArrival() &&
				m_fLeadPath >= 150.0 && m_fFollower1Path >= 100.0 && m_fFollower2Path >= 100.0 &&
				gap1 <= 60.0 && gap2 <= 60.0 && m_iStillTicks >= 3 && pilotStopped &&
				m_Driver1.CF_IsBoarded() && m_Driver2.CF_IsBoarded())
			{
				Print("[ConvoyFollower] AUTO_SEQUENCE_ARRIVAL: stopped road convoy with two seated followers");
				SetStage(1);
			}
			else if (m_iStageTicks >= 250)
				Finish("FAIL road convoy did not reach a stopped two-truck arrival");
			return;
		}
		if (m_iStage == 1)
		{
			if (CF_ConvoySession.CanReleaseAtUnload(m_Player, m_Driver1))
			{
				m_vBayPosition = m_Follower1.GetOrigin();
				m_vFollower2AtRelease = m_Follower2.GetOrigin();
				bool accepted = CF_ConvoySession.ReleaseAtUnload(m_Player, m_Driver1);
				Print("[ConvoyFollower] AUTO_SEQUENCE_RELEASE: accepted=" + accepted + " bay=" + m_vBayPosition);
				if (!accepted)
					Finish("FAIL explicit release was rejected");
				else
					SetStage(2);
			}
			else if (m_iStageTicks >= 60)
				Finish("FAIL front truck never became release-ready at stopped arrival");
			return;
		}
		if (m_iStage == 2)
		{
			LogReleaseNav();
			LogUnit2ReleaseNav();
			float bayClearance = vector.Distance(m_Follower1.GetOrigin(), m_vBayPosition);
			float nextAdvance = vector.Distance(m_Follower2.GetOrigin(), m_vFollower2AtRelease);
			float nextBayGap = vector.Distance(m_Follower2.GetOrigin(), m_vBayPosition);
			bool parked = m_Driver1.CF_IsUnloadDeparted() && m_Driver1.CF_IsAtUnloadWaitingPoint();
			bool nextReady = CF_ConvoySession.CanReleaseAtUnload(m_Player, m_Driver2);
			if (parked && bayClearance >= 15.0 && nextAdvance >= 3.0 && nextBayGap <= 8.0 && nextReady)
			{
				Print("[ConvoyFollower] AUTO_SEQUENCE_PARKED: unit1 seated in return slot; unit2 advanced " + nextAdvance + " m to bay, gap=" + nextBayGap);
				SetStage(3);
			}
			else if (m_iStageTicks >= 150)
				Finish("FAIL released truck did not park or unit2 did not reach bay; clear=" + bayClearance + " next_advance=" + nextAdvance + " bay_gap=" + nextBayGap + " parked=" + parked + " next_ready=" + nextReady);
			return;
		}
		if (m_iStage == 3)
		{
			if (CF_ConvoySession.CanStartReturnFromAction(m_Player, m_Driver1))
			{
				bool accepted = CF_ConvoySession.StartReturnFromAction(m_Player, m_Driver1);
				Print("[ConvoyFollower] AUTO_SEQUENCE_RETURN_ORDER: accepted=" + accepted);
				if (!accepted)
					Finish("FAIL manual regroup order rejected");
				else
					SetStage(4);
			}
			else if (m_iStageTicks >= 30)
				Finish("FAIL parked unit never offered manual regroup");
			return;
		}
		if (m_iStage == 4)
		{
			if (m_Driver1.CF_IsActiveConvoyMember() && m_Driver2.CF_IsActiveConvoyMember() &&
				m_Driver1.CF_IsBoarded() && m_Driver2.CF_IsBoarded())
			{
				Print("[ConvoyFollower] AUTO_SEQUENCE_RETURN_MERGED: both drivers seated in active convoy");
				if (!StartReturnDrive())
					Finish("FAIL merged convoy could not receive connected homeward road goal");
				else
					SetStage(5);
			}
			else if (m_iStageTicks >= 30)
				Finish("FAIL manual regroup did not merge both drivers");
			return;
		}
		if (m_iStage == 5)
		{
			if (m_Driver1.CF_IsReturnBlockedForActions())
			{
				Finish("FAIL Unit 1 return turn blocked after convoy merge");
				return;
			}
			if (m_Driver2.CF_IsReturnBlockedForActions())
			{
				Finish("FAIL Unit 2 return turn blocked after convoy merge");
				return;
			}
			float goalGap = vector.Distance(m_Lead.GetOrigin(), m_vReturnGoal);
			float gap1 = vector.Distance(m_Lead.GetOrigin(), m_Follower1.GetOrigin());
			float gap2 = vector.Distance(m_Follower1.GetOrigin(), m_Follower2.GetOrigin());
			if (goalGap <= 15.0 && m_fReturnLeadPath >= 100.0 &&
				m_fReturnFollower1Path >= 50.0 && m_fReturnFollower2Path >= 50.0 &&
				gap1 <= 60.0 && gap2 <= 60.0)
				Finish("PASS explicit release parked, next truck advanced, and both returned; paths=" +
					m_fReturnLeadPath + "," + m_fReturnFollower1Path + "," + m_fReturnFollower2Path);
			else if (m_iStageTicks >= 180)
				Finish("FAIL return drive or follower spacing; paths=" + m_fReturnLeadPath + "," +
					m_fReturnFollower1Path + "," + m_fReturnFollower2Path + " goal_gap=" + goalGap +
					" gaps=" + gap1 + "," + gap2);
		}
	}
}
