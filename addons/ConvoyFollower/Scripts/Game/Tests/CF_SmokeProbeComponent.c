// Test-only component used by the separate Arland_Auto worlds. It orders the
// production convoy session, sends an AI pilot to a reachable road target,
// and leaves numerical position evidence in the console log.
class CF_SmokeProbeComponentClass : ScriptComponentClass
{
}

class CF_SmokeProbeComponent : ScriptComponent
{
	[Attribute(defvalue: "1", params: "1 3 1", desc: "Number of staged follower trucks")]
	protected int m_iExpectedTrucks;
	[Attribute(defvalue: "0", params: "0 1 1", desc: "Use the surveyed straight-road goal for the separate two-truck release fixture")]
	protected bool m_bStraightReleaseRoadGoal;
	[Attribute(defvalue: "0", params: "0 1 1", desc: "Use the surveyed eight-metre Arland road goal for the separate wide-road forward-line fixture")]
	protected bool m_bWideRoadGoal;
	[Attribute(defvalue: "300", params: "120 600 1", desc: "Maximum total test seconds; pause-resume needs an extra owner-on-foot interval")]
	protected int m_iMaxTestSeconds;
	[Attribute(defvalue: "210", params: "120 450 1", desc: "Maximum drive seconds including any planned stop or owner exit")]
	protected int m_iMaxDriveSeconds;

	protected Vehicle m_Lead;
	protected ChimeraCharacter m_Player;
	protected SCR_AIGroup m_PilotGroup;
	protected ChimeraCharacter m_Pilot;
	protected AIWaypoint m_PilotWaypoint;
	protected ref array<CF_DriverControllerComponent> m_Drivers = {};
	protected ref array<vector> m_InitialFollowerPositions = {};
	protected ref array<vector> m_LastFollowerPositions = {};
	protected ref array<float> m_FollowerPaths = {};
	protected ref array<float> m_FollowerStepMeters = {};
	protected ref array<int> m_OverWarningSeconds = {};
	protected ref array<int> m_NoProgressSeconds = {};
	protected ref array<vector> m_ArrivalFollowerSamples = {};
	protected vector m_vInitialLeadPosition;
	protected vector m_vLastLeadPosition;
	protected vector m_vArrivalLeadSample;
	protected vector m_vRoadGoal;
	protected vector m_vReleaseOrigin;
	protected float m_fLeadPath;
	protected float m_fMaxEnRouteGap;
	protected float m_fMinRoadGoalGap = 1000000.0;
	protected int m_iStage;
	protected int m_iNextOrder;
	protected int m_iTicks;
	protected int m_iDrivingTicks;
	protected int m_iReleaseTicks;
	protected int m_iCameraAttempts;
	protected int m_iArrivalStableSeconds;
	protected int m_iMaxWarningSeconds;
	protected int m_iMaxNoProgressSeconds;
	protected bool m_bFinished;
	protected bool m_bEnRouteQualityFailed;
	protected bool m_bRoadGoalReached;
	protected bool m_bRoadArrivalPassed;
	protected bool m_bMaintainArrivalBrake;
	protected bool m_bCameraSelected;
	protected bool m_bShortBaseline;
	protected static const float CF_MIN_LEAD_PATH = 150.0;
	protected static const float CF_MIN_FOLLOWER_PATH = 100.0;
	protected static const float CF_MAX_FINAL_LINK_GAP = 60.0;
	protected static const float CF_MAX_FINAL_ROAD_DISTANCE = 5.0;
	protected static const float CF_MAX_SETTLED_STEP_FIVE_SECONDS = 3.0;
	protected static const float CF_WARNING_GAP_METERS = 180.0;
	protected static const int CF_MAX_SUSTAINED_WARNING_SECONDS = 8;
	protected static const int CF_MAX_SUSTAINED_NONPROGRESS_SECONDS = 15;
	protected static const int CF_ARRIVAL_STABLE_SECONDS = 20;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;

		m_Lead = Vehicle.Cast(owner);
		SetEventMask(owner, EntityEvent.POSTFRAME);
		CF_ConvoySettings.Get(); // Force test-only settings resolution in playerless server smoke.
		Print("[ConvoyFollower] AUTO_INIT: expected=" + m_iExpectedTrucks);
		GetGame().GetCallqueue().CallLater(LogOpenRoadSurvey, 3000, false);
		GetGame().GetCallqueue().CallLater(Poll, 1000, true);
	}

	protected void LogOpenRoadSurvey()
	{
		if (!m_Lead)
			return;
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return;
		RoadNetworkManager roads = aiWorld.GetRoadNetworkManager();
		for (int sample = 0; sample < 9; sample++)
		{
			float sampleX = 1240.0 + sample * 15.0;
			vector desired = Vector(sampleX, 1.0, 3335.0);
			BaseRoad road;
			float roadDistance;
			int roadId = roads.GetClosestRoad(desired, road, roadDistance);
			vector resolved;
			bool connected = roads.GetReachableWaypointInRoad(m_Lead.GetOrigin(), desired, 35.0, resolved);
			Print("[ConvoyFollower] AUTO_OPEN_ROAD_SURVEY: desired=" + desired + " road_id=" + roadId +
				" road_distance=" + roadDistance + " connected=" + connected + " resolved=" + resolved);
			if (sample == 4 && road)
			{
				ref array<vector> roadPoints = {};
				road.GetPoints(roadPoints);
				for (int pointIndex = 0; pointIndex < roadPoints.Count(); pointIndex++)
				{
					vector point = roadPoints[pointIndex];
					if (point[0] >= 1210.0 && point[0] <= 1450.0)
						Print("[ConvoyFollower] AUTO_OPEN_ROAD_POINT: road_id=" + roadId + " index=" + pointIndex + " pos=" + point);
				}
			}
		}
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
		if (m_iStage >= 4)
			Print("[ConvoyFollower] AUTO_RELEASE_RESULT: " + result);
		else
			Print("[ConvoyFollower] AUTO_RESULT: " + result);
		GetGame().GetCallqueue().Remove(Poll);
	}

	bool CF_HasPassedRoadArrival()
	{
		return m_bRoadArrivalPassed;
	}

	void CF_ReleaseLeadBrakeForReturn()
	{
		m_bMaintainArrivalBrake = false;
		if (m_Lead)
		{
			// Keep POSTFRAME active: the independent two-ahead companion on this
			// same lead entity needs its own physical-drive callback after arrival.
			// The false flag above makes this component's brake callback inert.
			CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
			if (car)
			{
				car.SetPersistentHandBrake(false);
				VehicleWheeledSimulation sim = car.GetSimulation();
				if (sim)
					sim.SetBreak(0, true);
			}
		}
		Print("[ConvoyFollower] AUTO_LEAD_BRAKE_RELEASED: companion return drive may begin");
	}

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (m_bMaintainArrivalBrake && m_Lead)
			ApplyLeadBrake();
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
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
		{
			Print("[ConvoyFollower] AUTO_ROUTE: road network unavailable");
			return false;
		}
		RoadNetworkManager roads = aiWorld.GetRoadNetworkManager();
		BaseRoad nearestStartRoad;
		float startRoadDistance;
		int startRoadId = roads.GetClosestRoad(m_vInitialLeadPosition, nearestStartRoad, startRoadDistance);
		Print("[ConvoyFollower] AUTO_ROUTE_SCAN: start_road=" + startRoadId + " distance=" + startRoadDistance);
		if (nearestStartRoad)
		{
			ref array<vector> startRoadPoints = {};
			nearestStartRoad.GetPoints(startRoadPoints);
			if (!startRoadPoints.IsEmpty())
				Print("[ConvoyFollower] AUTO_ROAD_GEOMETRY: start road points=" + startRoadPoints.Count() + " first=" + startRoadPoints[0] + " last=" + startRoadPoints[startRoadPoints.Count() - 1]);
		}
		ref array<vector> directions = {
			Vector(1, 0, 0), Vector(0.7, 0, 0.7), Vector(0, 0, 1), Vector(-0.7, 0, 0.7),
			Vector(-1, 0, 0), Vector(-0.7, 0, -0.7), Vector(0, 0, -1), Vector(0.7, 0, -0.7)
		};
		float bestScore = -100000;
		if (m_bStraightReleaseRoadGoal)
		{
			// The generic farthest-forward scan ends beside a tight bend. The
			// release fixture instead stops on the surveyed connected hill road.
			vector desiredStraightGoal = Vector(1541.59, 31.2082, 3337.58);
			vector resolvedStraightGoal;
			bool connectedStraightGoal = roads.GetReachableWaypointInRoad(m_vInitialLeadPosition, desiredStraightGoal, 35.0, resolvedStraightGoal);
			if (!connectedStraightGoal || vector.Distance(desiredStraightGoal, resolvedStraightGoal) > 15.0 ||
				vector.Distance(m_vInitialLeadPosition, resolvedStraightGoal) < 180.0)
			{
				Print("[ConvoyFollower] AUTO_ROUTE_EXPLICIT: FAIL straight release road goal unavailable desired=" +
					desiredStraightGoal + " resolved=" + resolvedStraightGoal + " connected=" + connectedStraightGoal);
				return false;
			}
			m_vRoadGoal = resolvedStraightGoal;
			bestScore = 1000000.0;
			Print("[ConvoyFollower] AUTO_ROUTE_EXPLICIT: connected straight release road goal=" + m_vRoadGoal +
				" from=" + m_vInitialLeadPosition);
		}
		else if (m_bWideRoadGoal)
		{
			// Surveyed nominal width-8 road near Arland candidate 28. Keeping
			// this fixture opt-in preserves every earlier smoke route unchanged.
			vector desiredWideGoal = Vector(1453.0, 36.7, 3052.0);
			vector resolvedWideGoal;
			bool connectedWideGoal = roads.GetReachableWaypointInRoad(m_vInitialLeadPosition, desiredWideGoal, 35.0, resolvedWideGoal);
			if (!connectedWideGoal || vector.Distance(desiredWideGoal, resolvedWideGoal) > 15.0 ||
				vector.Distance(m_vInitialLeadPosition, resolvedWideGoal) < 180.0)
			{
				Print("[ConvoyFollower] AUTO_ROUTE_EXPLICIT: FAIL wide road goal unavailable desired=" +
					desiredWideGoal + " resolved=" + resolvedWideGoal + " connected=" + connectedWideGoal);
				return false;
			}
			BaseRoad wideRoad;
			float wideRoadGap;
			roads.GetClosestRoad(resolvedWideGoal, wideRoad, wideRoadGap);
			if (!wideRoad || wideRoad.GetWidth() < 8.0 || wideRoadGap > 5.0)
			{
				Print("[ConvoyFollower] AUTO_ROUTE_EXPLICIT: FAIL wide road goal lacks measured width resolved=" +
					resolvedWideGoal + " road_gap=" + wideRoadGap);
				return false;
			}
			m_vRoadGoal = resolvedWideGoal;
			bestScore = 1000000.0;
			Print("[ConvoyFollower] AUTO_ROUTE_EXPLICIT: connected width-8 road goal=" + m_vRoadGoal +
				" from=" + m_vInitialLeadPosition + " mapped_width=" + wideRoad.GetWidth());
		}
		vector initialForward = m_Lead.GetWorldTransformAxis(2);
		for (int radiusIndex = 0; radiusIndex < 3 && !m_bStraightReleaseRoadGoal && !m_bWideRoadGoal; radiusIndex++)
		{
			float radius = 160.0 + radiusIndex * 70.0;
			for (int directionIndex = 0; directionIndex < directions.Count(); directionIndex++)
			{
				vector desired = m_vInitialLeadPosition + directions[directionIndex] * radius;
				BaseRoad nearestCandidateRoad;
				float candidateRoadDistance;
				int candidateRoadId = roads.GetClosestRoad(desired, nearestCandidateRoad, candidateRoadDistance);
				if (candidateRoadId < 0 || candidateRoadDistance > 35.0)
					continue;
				if (nearestCandidateRoad)
				{
					ref array<vector> candidateRoadPoints = {};
					nearestCandidateRoad.GetPoints(candidateRoadPoints);
					if (!candidateRoadPoints.IsEmpty())
						Print("[ConvoyFollower] AUTO_ROAD_GEOMETRY: candidate road=" + candidateRoadId + " points=" + candidateRoadPoints.Count() + " first=" + candidateRoadPoints[0] + " last=" + candidateRoadPoints[candidateRoadPoints.Count() - 1]);
				}
				vector reachable;
				bool roadConnected = roads.GetReachableWaypointInRoad(m_vInitialLeadPosition, desired, 35.0, reachable);
				Print("[ConvoyFollower] AUTO_ROUTE_SCAN: desired=" + desired + " road_id=" + candidateRoadId + " road_distance=" + candidateRoadDistance + " connected=" + roadConnected + " resolved=" + reachable);
				if (!roadConnected || vector.Distance(desired, reachable) > 35.0 ||
					vector.Distance(m_vInitialLeadPosition, reachable) < 140.0)
					continue;
				vector offset = reachable - m_vInitialLeadPosition;
				float forwardProgress = offset[0] * initialForward[0] + offset[2] * initialForward[2];
				float score = forwardProgress - vector.Distance(desired, reachable);
				if (score <= bestScore)
					continue;
				bestScore = score;
				m_vRoadGoal = reachable;
			}
		}
		if (bestScore < -99999)
		{
			m_bShortBaseline = true;
			m_vRoadGoal = Vector(1198, 1, 3283);
			Print("[ConvoyFollower] AUTO_ROUTE: no reachable 150 m road goal; short baseline only target=" + m_vRoadGoal);
		}
		else
			Print("[ConvoyFollower] AUTO_ROUTE: selected 150 m road goal=" + m_vRoadGoal + " score=" + bestScore);
		Resource prefab = Resource.Load("{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et");
		if (!prefab.IsValid())
			return false;
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = m_vRoadGoal;
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
		Print("[ConvoyFollower] AUTO_PILOT: AI driving road waypoint=" + params.Transform[3] + " from=" + m_vInitialLeadPosition);
		return true;
	}

	protected void UpdatePathDistances()
	{
		vector leadPosition = m_Lead.GetOrigin();
		float predecessorStep = vector.Distance(leadPosition, m_vLastLeadPosition);
		m_fLeadPath += predecessorStep;
		m_vLastLeadPosition = leadPosition;
		IEntity predecessor = m_Lead;
		for (int i = 0; i < m_Drivers.Count(); i++)
		{
			Vehicle truck = m_Drivers[i].CF_GetAssignedVehicle();
			if (!truck)
				continue;
			vector position = truck.GetOrigin();
			float step = vector.Distance(position, m_LastFollowerPositions[i]);
			m_FollowerStepMeters[i] = step;
			m_FollowerPaths[i] = m_FollowerPaths[i] + step;
			m_LastFollowerPositions[i] = position;
			float gap = vector.Distance(position, predecessor.GetOrigin());
			if (gap > m_fMaxEnRouteGap)
				m_fMaxEnRouteGap = gap;
			if (gap > CF_WARNING_GAP_METERS && predecessorStep > 2.0)
				m_OverWarningSeconds[i] = m_OverWarningSeconds[i] + 1;
			else
				m_OverWarningSeconds[i] = 0;
			if (gap > 30.0 && predecessorStep > 2.0 && step < 0.5)
				m_NoProgressSeconds[i] = m_NoProgressSeconds[i] + 1;
			else
				m_NoProgressSeconds[i] = 0;
			if (m_OverWarningSeconds[i] > m_iMaxWarningSeconds)
				m_iMaxWarningSeconds = m_OverWarningSeconds[i];
			if (m_NoProgressSeconds[i] > m_iMaxNoProgressSeconds)
				m_iMaxNoProgressSeconds = m_NoProgressSeconds[i];
			if (!m_bEnRouteQualityFailed &&
				(m_iMaxWarningSeconds >= CF_MAX_SUSTAINED_WARNING_SECONDS ||
				m_iMaxNoProgressSeconds >= CF_MAX_SUSTAINED_NONPROGRESS_SECONDS))
			{
				m_bEnRouteQualityFailed = true;
				Print("[ConvoyFollower] AUTO_EN_ROUTE_ALERT: unit=" + (i + 1) +
					" max_gap=" + m_fMaxEnRouteGap +
					" warning_s=" + m_iMaxWarningSeconds +
					" no_progress_s=" + m_iMaxNoProgressSeconds);
			}
			predecessor = truck;
			predecessorStep = step;
		}
	}

	protected bool FollowersReachedGoal(out float widestGap)
	{
		widestGap = 0;
		IEntity previous = m_Lead;
		bool allReady = true;
		RoadNetworkManager roads;
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (aiWorld)
			roads = aiWorld.GetRoadNetworkManager();
		for (int i = 0; i < m_Drivers.Count(); i++)
		{
			Vehicle truck = m_Drivers[i].CF_GetAssignedVehicle();
			if (!truck)
			{
				allReady = false;
				continue;
			}
			float gap = vector.Distance(truck.GetOrigin(), previous.GetOrigin());
			if (gap > widestGap)
				widestGap = gap;
			float roadDistance = 1000000.0;
			float roadWidth = -1.0;
			float allowedRoadDistance = CF_MAX_FINAL_ROAD_DISTANCE;
			if (roads)
			{
				BaseRoad road;
				roads.GetClosestRoad(truck.GetOrigin(), road, roadDistance);
				if (road)
				{
					roadWidth = road.GetWidth();
					float widthAllowance = roadWidth * 0.5 + 2.0;
					if (widthAllowance > allowedRoadDistance)
						allowedRoadDistance = widthAllowance;
					if (allowedRoadDistance > 6.0)
						allowedRoadDistance = 6.0;
				}
			}
			bool unitReady = m_Drivers[i].CF_IsActiveConvoyMember() &&
				!m_Drivers[i].CF_IsOrderInverted() &&
				m_FollowerPaths[i] >= CF_MIN_FOLLOWER_PATH && gap <= CF_MAX_FINAL_LINK_GAP &&
				roadDistance <= allowedRoadDistance;
			if (!unitReady)
				allReady = false;
			if (m_bRoadGoalReached && m_iDrivingTicks % 5 == 0)
				Print("[ConvoyFollower] AUTO_FINAL_ROAD_CHECK: unit=" + (i + 1) +
					" road_dist=" + roadDistance + " road_width=" + roadWidth +
					" allowed=" + allowedRoadDistance + " predecessor_gap=" + gap +
					" path=" + m_FollowerPaths[i] + " ready=" + unitReady);
			previous = truck;
		}
		return allReady;
	}

	protected void UpdateArrivalStability(bool followersReady, float goalGap)
	{
		if (m_iDrivingTicks % 5 != 0)
			return;
		float leadStep = vector.Distance(m_Lead.GetOrigin(), m_vArrivalLeadSample);
		m_vArrivalLeadSample = m_Lead.GetOrigin();
		bool allSettled = leadStep <= CF_MAX_SETTLED_STEP_FIVE_SECONDS;
		for (int i = 0; i < m_Drivers.Count(); i++)
		{
			Vehicle truck = m_Drivers[i].CF_GetAssignedVehicle();
			if (!truck)
			{
				allSettled = false;
				continue;
			}
			float step = vector.Distance(truck.GetOrigin(), m_ArrivalFollowerSamples[i]);
			m_ArrivalFollowerSamples[i] = truck.GetOrigin();
			if (step > CF_MAX_SETTLED_STEP_FIVE_SECONDS)
				allSettled = false;
		}
		if (m_bRoadGoalReached && goalGap <= 25.0 && followersReady && allSettled)
			m_iArrivalStableSeconds += 5;
		else
			m_iArrivalStableSeconds = 0;
		if (m_bRoadGoalReached)
			Print("[ConvoyFollower] AUTO_ARRIVAL_STABILITY: seconds=" + m_iArrivalStableSeconds +
				" goal_gap=" + goalGap + " min_goal_gap=" + m_fMinRoadGoalGap +
				" lead_step_5s=" + leadStep + " followers_ready=" + followersReady +
				" all_settled=" + allSettled);
	}

	protected void HoldLeadAtRoadGoal()
	{
		if (m_PilotGroup)
		{
			AIWaypoint pilotWaypoint = m_PilotGroup.GetCurrentWaypoint();
			if (pilotWaypoint)
				m_PilotGroup.RemoveWaypoint(pilotWaypoint);
		}
		m_PilotWaypoint = null;
		m_bMaintainArrivalBrake = true;
		ApplyLeadBrake();
	}

	protected void ApplyLeadBrake()
	{
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

	protected void LogPositions()
	{
		float goalDistance = vector.Distance(m_Lead.GetOrigin(), m_vRoadGoal);
		Print("[ConvoyFollower] AUTO_POSITION: lead=" + m_Lead.GetOrigin() + " path=" + m_fLeadPath + " goal_gap=" + goalDistance);
		IEntity previous = m_Lead;
		for (int i = 0; i < m_Drivers.Count(); i++)
		{
			Vehicle truck = m_Drivers[i].CF_GetAssignedVehicle();
			if (!truck)
				continue;
			float gap = vector.Distance(truck.GetOrigin(), previous.GetOrigin());
			Print("[ConvoyFollower] AUTO_POSITION: unit=" + (i + 1) + " pos=" + truck.GetOrigin() + " path=" + m_FollowerPaths[i] + " predecessor_gap=" + gap);
			LogUnitNav(i, truck, gap);
			previous = truck;
		}
	}

	protected void LogUnitNav(int index, Vehicle truck, float gap)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokeGroup" + (index + 1)));
		AIWaypoint waypoint;
		if (group)
			waypoint = group.GetCurrentWaypoint();
		vector waypointPosition;
		if (waypoint)
			waypointPosition = waypoint.GetOrigin();
		int roadId = -1;
		float roadDistance = -1.0;
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (aiWorld && aiWorld.GetRoadNetworkManager())
		{
			BaseRoad road;
			roadId = aiWorld.GetRoadNetworkManager().GetClosestRoad(truck.GetOrigin(), road, roadDistance);
		}
		CarControllerComponent unitCar = CarControllerComponent.Cast(truck.FindComponent(CarControllerComponent));
		VehicleWheeledSimulation unitSim;
		if (unitCar)
			unitSim = unitCar.GetSimulation();
		bool engineOn;
		int gear;
		float throttle;
		float brake;
		if (unitSim)
		{
			engineOn = unitSim.EngineIsOn();
			gear = unitSim.GetGear();
			throttle = unitSim.GetThrottle();
			brake = unitSim.GetBrake();
		}
		Print("[ConvoyFollower] AUTO_UNIT_NAV: unit=" + (index + 1) + " pos=" + truck.GetOrigin() +
			" predecessor_gap=" + gap + " waypoint=" + waypoint +
			" goal=" + waypointPosition + " forward=" + truck.GetWorldTransformAxis(2) +
			" step_m=" + m_FollowerStepMeters[index] + " road_id=" + roadId +
			" road_dist=" + roadDistance + " engine=" + engineOn + " gear=" + gear +
			" throttle=" + throttle + " brake=" + brake +
			" inverted=" + m_Drivers[index].CF_IsOrderInverted());
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
		Print("[ConvoyFollower] AUTO_CAMERA: editor_open=" + editorWasOpen + " editor_closed=" + editorClosed + " player_camera=" + cameraSelected);
	}

	protected void Poll()
	{
		if (m_bFinished)
			return;
		m_iTicks++;
		if (m_iTicks > m_iMaxTestSeconds)
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
		SelectLocalPlayerCamera();
		if (m_iStage == 1)
		{
			while (m_Drivers.Count() < m_iExpectedTrucks)
			{
				CF_DriverControllerComponent driver = FindDriver(m_Drivers.Count());
				if (!driver)
					return;
				m_Drivers.Insert(driver);
				m_InitialFollowerPositions.Insert(Vector(0, 0, 0));
				m_LastFollowerPositions.Insert(Vector(0, 0, 0));
				m_FollowerPaths.Insert(0);
				m_FollowerStepMeters.Insert(0);
				m_OverWarningSeconds.Insert(0);
				m_NoProgressSeconds.Insert(0);
				m_ArrivalFollowerSamples.Insert(Vector(0, 0, 0));
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
			m_vLastLeadPosition = m_vInitialLeadPosition;
			m_vArrivalLeadSample = m_vInitialLeadPosition;
			m_fLeadPath = 0;
			m_fMaxEnRouteGap = 0;
			m_fMinRoadGoalGap = 1000000.0;
			m_bRoadGoalReached = false;
			m_bEnRouteQualityFailed = false;
			m_iMaxWarningSeconds = 0;
			m_iMaxNoProgressSeconds = 0;
			m_iArrivalStableSeconds = 0;
			for (int i = 0; i < m_Drivers.Count(); i++)
			{
				Vehicle truck = m_Drivers[i].CF_GetAssignedVehicle();
				if (!truck)
				{
					Finish("FAIL unit truck missing unit=" + (i + 1));
					return;
				}
				m_InitialFollowerPositions[i] = truck.GetOrigin();
				m_LastFollowerPositions[i] = truck.GetOrigin();
				m_ArrivalFollowerSamples[i] = truck.GetOrigin();
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
			Print("[ConvoyFollower] AUTO_DRIVE: AI pilot sustained road-route test started");
		}
		if (m_iStage == 3)
		{
			m_iDrivingTicks++;
			UpdatePathDistances();
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
			else if (m_iExpectedTrucks >= 3 && m_iDrivingTicks <= 100)
			{
				IEntity previousTruck = m_Lead;
				bool nearGap = false;
				for (int i = 0; i < m_Drivers.Count(); i++)
				{
					Vehicle truck = m_Drivers[i].CF_GetAssignedVehicle();
					if (!truck)
						continue;
					if (vector.Distance(truck.GetOrigin(), previousTruck.GetOrigin()) < 30.0)
						nearGap = true;
					previousTruck = truck;
				}
				if (nearGap)
				{
					Print("[ConvoyFollower] AUTO_NAV_NEAR_GAP: diagnostic second=" + m_iDrivingTicks);
					previousTruck = m_Lead;
					for (int j = 0; j < m_Drivers.Count(); j++)
					{
						Vehicle nearTruck = m_Drivers[j].CF_GetAssignedVehicle();
						if (!nearTruck)
							continue;
						float nearDistance = vector.Distance(nearTruck.GetOrigin(), previousTruck.GetOrigin());
						LogUnitNav(j, nearTruck, nearDistance);
						previousTruck = nearTruck;
					}
				}
			}
			if (m_bShortBaseline && m_iDrivingTicks >= 45)
			{
				LogPositions();
				Print("[ConvoyFollower] AUTO_BASELINE_RESULT: lead_path=" + m_fLeadPath + " follower_path=" + m_FollowerPaths[0]);
				Finish("FAIL long road goal unavailable; short movement baseline only");
				return;
			}
			float goalGap = vector.Distance(m_Lead.GetOrigin(), m_vRoadGoal);
			if (goalGap < m_fMinRoadGoalGap)
				m_fMinRoadGoalGap = goalGap;
			if (!m_bRoadGoalReached && goalGap <= 14.0 && m_fLeadPath >= CF_MIN_LEAD_PATH)
			{
				m_bRoadGoalReached = true;
				Print("[ConvoyFollower] AUTO_ROAD_GOAL_REACHED: path=" + m_fLeadPath +
					" goal_gap=" + goalGap);
				Print("[ConvoyFollower] AUTO_LEAD_BRAKE: pilot waypoint removed; persistent brake applied at road goal");
			}
			if (m_bRoadGoalReached)
				HoldLeadAtRoadGoal();
			float widestGap;
			bool followersReady = FollowersReachedGoal(widestGap);
			UpdateArrivalStability(followersReady, goalGap);
			if (m_iArrivalStableSeconds >= CF_ARRIVAL_STABLE_SECONDS ||
				m_iDrivingTicks >= m_iMaxDriveSeconds)
			{
				LogPositions();
				Print("[ConvoyFollower] AUTO_EN_ROUTE_METRICS: max_gap=" + m_fMaxEnRouteGap +
					" warning_s=" + m_iMaxWarningSeconds +
					" no_progress_s=" + m_iMaxNoProgressSeconds);
				if (m_iArrivalStableSeconds < CF_ARRIVAL_STABLE_SECONDS)
					Finish("FAIL road arrival or stable chain lead_path=" + m_fLeadPath +
						" goal_gap=" + goalGap + " min_goal_gap=" + m_fMinRoadGoalGap +
						" widest_gap=" + widestGap + " stable_seconds=" + m_iArrivalStableSeconds);
				else if (m_bEnRouteQualityFailed)
					Finish("FAIL sustained en-route separation or nonprogress max_gap=" + m_fMaxEnRouteGap +
						" warning_s=" + m_iMaxWarningSeconds +
						" no_progress_s=" + m_iMaxNoProgressSeconds);
				else if (m_iExpectedTrucks == 1)
				{
					m_bRoadArrivalPassed = true;
					Print("[ConvoyFollower] AUTO_ROUTE_PASS: lead_path=" + m_fLeadPath + " follower_path=" + m_FollowerPaths[0] + " final_gap=" + widestGap + " min_goal_gap=" + m_fMinRoadGoalGap);
					Print("[ConvoyFollower] AUTO_RESULT: PASS sustained road following");
					m_iStage = 4;
				}
				else
				{
					m_bRoadArrivalPassed = true;
					Finish("PASS stable road arrival lead_path=" + m_fLeadPath + " goal_gap=" + goalGap +
						" min_goal_gap=" + m_fMinRoadGoalGap + " widest_gap=" + widestGap);
				}
			}
		}
		if (m_iStage == 4)
		{
			m_iReleaseTicks++;
			if (CF_ConvoySession.CanReleaseAtUnload(m_Player, m_Drivers[0]))
			{
				Vehicle truck = m_Drivers[0].CF_GetAssignedVehicle();
				m_vReleaseOrigin = truck.GetOrigin();
				bool accepted = CF_ConvoySession.ReleaseAtUnload(m_Player, m_Drivers[0]);
				Print("[ConvoyFollower] AUTO_RELEASE: order accepted=" + accepted + " unload_pos=" + m_vReleaseOrigin);
				if (!accepted)
					Finish("FAIL release order rejected at arrived truck");
				else
				{
					m_iStage = 5;
					m_iReleaseTicks = 0;
				}
			}
			else if (m_iReleaseTicks >= 60)
				Finish("FAIL unload release never became ready");
		}
		if (m_iStage == 5)
		{
			m_iReleaseTicks++;
			Vehicle truck = m_Drivers[0].CF_GetAssignedVehicle();
			float cleared;
			if (truck)
				cleared = vector.Distance(truck.GetOrigin(), m_vReleaseOrigin);
			if (m_iReleaseTicks % 5 == 0)
			{
				CarControllerComponent followerCar;
				VehicleWheeledSimulation followerSim;
				if (truck)
					followerCar = CarControllerComponent.Cast(truck.FindComponent(CarControllerComponent));
				if (followerCar)
					followerSim = followerCar.GetSimulation();
				SCR_AIGroup followerGroup = SCR_AIGroup.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokeGroup1"));
				AIWaypoint currentWaypoint;
				if (followerGroup)
					currentWaypoint = followerGroup.GetCurrentWaypoint();
				bool engineOn;
				float rpm;
				int gear;
				float throttle;
				float brake;
				if (followerSim)
				{
					engineOn = followerSim.EngineIsOn();
					rpm = followerSim.EngineGetRPM();
					gear = followerSim.GetGear();
					throttle = followerSim.GetThrottle();
					brake = followerSim.GetBrake();
				}
				bool handBrake = followerCar && followerCar.GetHandBrake();
				Print("[ConvoyFollower] AUTO_RELEASE_POSITION: unit=1 moved_from_unload=" + cleared +
					" seated=" + m_Drivers[0].CF_IsBoarded() + " waypoint=" + currentWaypoint +
					" engine=" + engineOn + " rpm=" + rpm + " gear=" + gear +
					" throttle=" + throttle + " brake=" + brake + " handbrake=" + handBrake);
			}
			if (cleared >= 15.0 && m_Drivers[0].CF_IsBoarded() &&
				m_Drivers[0].CF_IsUnloadDeparted() && m_Drivers[0].CF_IsAtUnloadWaitingPoint())
				Finish("PASS road arrival and explicit release; truck parked " + cleared + " m from bay");
			else if (m_iReleaseTicks >= 90)
				Finish("FAIL released truck did not settle in return slot; moved=" + cleared +
					" parked=" + m_Drivers[0].CF_IsAtUnloadWaitingPoint());
		}
	}

}
