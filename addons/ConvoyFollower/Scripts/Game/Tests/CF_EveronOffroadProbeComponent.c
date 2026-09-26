// Test-only Regina open-field survey and one-follower physical drive.
// Geometry produces a candidate, never a gameplay PASS. The live drive must
// keep both trucks measurably outside the mapped road corridor.
class CF_EveronOffroadProbeComponentClass : ScriptComponentClass
{
}

class CF_EveronOffroadProbeComponent : ScriptComponent
{
	protected Vehicle m_Lead;
	protected Vehicle m_Follower;
	protected ChimeraCharacter m_Player;
	protected ChimeraCharacter m_Pilot;
	protected SCR_AIGroup m_PilotGroup;
	protected CF_DriverControllerComponent m_Driver;
	protected AIWaypoint m_PilotWaypoint;
	protected RoadNetworkManager m_Roads;
	protected vector m_vRouteStart;
	protected vector m_vRouteGoal;
	protected vector m_vRouteAxis;
	protected ref array<vector> m_Starts = {};
	protected ref array<vector> m_LastPositions = {};
	protected ref array<float> m_Paths = {};
	protected ref array<float> m_OffroadPaths = {};
	protected ref array<int> m_OffroadMovingSeconds = {};
	protected ref array<int> m_MaxOffroadMovingSeconds = {};
	protected ref array<float> m_LastSteps = {};
	protected bool m_bHoldLead = true;
	protected bool m_bFinished;
	protected bool m_bOccupied;
	protected bool m_bGoalReached;
	protected AICarMovementComponent m_LeadCruiseMovement;
	protected bool m_bOwnLeadCruise;
	protected bool m_bLeadHoldPoseValid;
	protected vector m_vLeadHeldPosition;
	protected int m_iLeadHoldStillSeconds;
	protected bool m_bOrderSent;
	protected IEntity m_eOccupant;
	protected int m_iStage;
	protected int m_iTicks;
	protected int m_iDrivingSeconds;
	protected int m_iSettledSeconds;
	protected int m_iNoProgressSeconds;
	protected int m_iMaxNoProgressSeconds;
	protected float m_fSelectedRouteLength;
	protected float m_fMaxGap;
	protected static const float OFFROAD_BUFFER = 8.0;
	protected static const float MIN_OFFROAD_PATH = 40.0;
	protected static const int MIN_OFFROAD_MOVING_SECONDS = 8;
	protected static const int MIN_SETTLED_SECONDS = 10;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Lead = Vehicle.Cast(owner);
		SetEventMask(owner, EntityEvent.POSTFRAME);
		CF_ConvoySettings.Get();
		Print("[ConvoyFollower] OFFROAD_INIT: expected=1 world=ConvoyFollower_Everon_Offroad_Survey_1Truck");
		GetGame().GetCallqueue().CallLater(Poll, 1000, true);
	}

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (CF_ConvoySession.CF_IsWorldCleanup())
			return;
		if (m_bHoldLead && m_Lead)
			ApplyLeadBrake();
	}

	override void OnDelete(IEntity owner)
	{
		if (GetGame())
			GetGame().GetCallqueue().Remove(Poll);
		ResetLeadCruiseOverride();
		m_Lead = null;
		m_Follower = null;
	}

	protected void ResetLeadCruiseOverride()
	{
		if (m_bOwnLeadCruise && m_LeadCruiseMovement && GetGame() && GetGame().GetWorld() &&
			!CF_ConvoySession.CF_IsWorldCleanup())
			m_LeadCruiseMovement.ResetCruiseSpeed();
		m_bOwnLeadCruise = false;
		m_LeadCruiseMovement = null;
	}

	protected void RequestLeadCruiseStop()
	{
		AICarMovementComponent movement = AICarMovementComponent.Cast(m_Lead.FindComponent(AICarMovementComponent));
		AIAgent agent;
		if (movement)
			agent = movement.GetAIAgent();
		if (!agent || agent.GetControlledEntity() != m_Lead)
		{
			Print("[ConvoyFollower] OFFROAD_LEAD_CRUISE_MISSING: no native car movement proven to own lead");
			return;
		}
		m_LeadCruiseMovement = movement;
		m_LeadCruiseMovement.SetCruiseSpeed(0);
		m_bOwnLeadCruise = true;
		Print("[ConvoyFollower] OFFROAD_LEAD_CRUISE_STOP: controlled=" + m_Lead.GetName() +
			" requested_kmh=0 physical_hold_pending=true");
	}

	protected bool CheckLeadHold()
	{
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (!car || !car.GetSimulation())
			return false;
		VehicleWheeledSimulation sim = car.GetSimulation();
		if (!m_bLeadHoldPoseValid)
		{
			if (Speed(m_Lead) <= 2.0 && m_LastSteps[0] <= 0.3)
				m_iLeadHoldStillSeconds++;
			else
				m_iLeadHoldStillSeconds = 0;
			if (m_iLeadHoldStillSeconds >= 2)
			{
				m_bLeadHoldPoseValid = true;
				m_vLeadHeldPosition = m_Lead.GetOrigin();
			}
		}
		float drift;
		if (m_bLeadHoldPoseValid)
			drift = vector.DistanceXZ(m_Lead.GetOrigin(), m_vLeadHeldPosition);
		Print("[ConvoyFollower] OFFROAD_LEAD_HOLD: origin=" + m_Lead.GetOrigin() +
			" forward=" + m_Lead.GetWorldTransformAxis(2) + " speed_kmh=" + sim.GetSpeedKmh() +
			" brake=" + sim.GetBrake() + " throttle=" + sim.GetThrottle() + " gear=" + sim.GetGear() +
			" engine=" + sim.EngineIsOn() + " handbrake=" + sim.IsHandbrakeOn() +
			" persistent=" + car.GetPersistentHandBrake() + " owns_cruise_zero=" + m_bOwnLeadCruise +
			" settled_pose=" + m_bLeadHoldPoseValid + " drift_m=" + drift);
		if (m_bLeadHoldPoseValid && drift > 2.0)
		{
			Finish("FAIL test lead drift exceeded 2m after settled hold");
			return false;
		}
		return true;
	}

	protected bool Resolve()
	{
		BaseWorld world = GetGame().GetWorld();
		m_Follower = Vehicle.Cast(world.FindEntityByName("CF_SmokeFollower1"));
		m_PilotGroup = SCR_AIGroup.Cast(world.FindEntityByName("CF_SmokePilotGroup"));
		SCR_AIGroup driverGroup = SCR_AIGroup.Cast(world.FindEntityByName("CF_SmokeGroup1"));
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!m_Lead || !m_Follower || !m_PilotGroup || !driverGroup || !aiWorld)
			return false;
		m_Roads = aiWorld.GetRoadNetworkManager();
		ref array<AIAgent> pilots = {};
		ref array<AIAgent> drivers = {};
		m_PilotGroup.GetAgents(pilots);
		driverGroup.GetAgents(drivers);
		if (!m_Roads || pilots.Count() != 1 || drivers.Count() != 1 || !pilots[0] || !drivers[0])
			return false;
		m_Pilot = ChimeraCharacter.Cast(pilots[0].GetControlledEntity());
		IEntity driverCharacter = drivers[0].GetControlledEntity();
		if (!m_Pilot || !driverCharacter)
			return false;
		m_Driver = CF_DriverControllerComponent.Cast(driverCharacter.FindComponent(CF_DriverControllerComponent));
		return m_Driver != null;
	}

	protected bool EnsureOwnerPassenger()
	{
		PlayerManager players = GetGame().GetPlayerManager();
		if (!players)
			return false;
		ref array<int> ids = {};
		players.GetPlayers(ids);
		if (ids.IsEmpty())
			return false;
		PlayerController controller = players.GetPlayerController(ids[0]);
		if (!controller)
			return false;
		m_Player = ChimeraCharacter.Cast(controller.GetControlledEntity());
		if (!m_Player)
		{
			Resource crew = Resource.Load("{E1CB513B8B9B08F4}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_US_Crew.et");
			if (!crew.IsValid())
				return false;
			EntitySpawnParams params = EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = m_Lead.GetOrigin() + Vector(3, 0, 2);
			m_Player = ChimeraCharacter.Cast(GetGame().SpawnEntityPrefab(crew, GetGame().GetWorld(), params));
			if (!m_Player || !controller.SetControlledEntity(m_Player))
				return false;
			Print("[ConvoyFollower] OFFROAD_PLAYER: possessed test owner");
		}
		if (players.GetPlayerIdFromControlledEntity(m_Player) <= 0)
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
				access.GetInVehicle(m_Lead, slot, true, -1, ECloseDoorAfterActions.CLOSE_DOOR, true);
				break;
			}
		}
		return false;
	}

	protected bool PilotSeated()
	{
		if (!m_Pilot)
			return false;
		CompartmentAccessComponent access = m_Pilot.GetCompartmentAccessComponent();
		BaseCompartmentSlot slot;
		if (access)
			slot = access.GetCompartment();
		return slot && slot.IsPiloting() && access.GetVehicleIn(m_Pilot) == m_Lead;
	}

	protected bool EnsurePilot()
	{
		if (PilotSeated())
			return true;
		if (m_PilotWaypoint)
			return false;
		Resource prefab = Resource.Load("{8AD8C82346156494}Prefabs/AI/Waypoints/AIWaypoint_GetInSelected.et");
		if (!prefab.IsValid())
			return false;
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = m_Lead.GetOrigin();
		SCR_BoardingEntityWaypoint waypoint = SCR_BoardingEntityWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(prefab, null, params));
		if (!waypoint)
			return false;
		waypoint.SetEntity(m_Lead);
		waypoint.SetAllowance(true, false, false);
		waypoint.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_GAMEMASTER);
		waypoint.SetCompletionRadius(8.0);
		m_PilotWaypoint = waypoint;
		m_PilotGroup.AddWaypoint(waypoint);
		Print("[ConvoyFollower] OFFROAD_PILOT: boarding lead truck");
		return false;
	}

	protected bool RoadMeasurement(vector position, out float gap, out float width, out int roadId)
	{
		gap = -1.0;
		width = -1.0;
		roadId = -1;
		if (!m_Roads)
			return false;
		BaseRoad road;
		roadId = m_Roads.GetClosestRoad(position, road, gap);
		if (!road || gap < 0)
			return false;
		width = road.GetWidth();
		return width > 0;
	}

	protected bool IsOffroad(vector position, out float gap, out float width, out int roadId)
	{
		return RoadMeasurement(position, gap, width, roadId) && gap > width * 0.5 + OFFROAD_BUFFER;
	}

	protected bool SurveyTraceFilter(IEntity entity, vector start, vector direction)
	{
		return entity != m_Lead && entity != m_Follower && entity != m_Player && entity != m_Pilot;
	}

	protected bool SurveyOccupant(IEntity entity)
	{
		if (entity != m_Lead && entity != m_Follower && Vehicle.Cast(entity))
		{
			m_bOccupied = true;
			m_eOccupant = entity;
		}
		return true;
	}

	protected string Describe(IEntity entity)
	{
		if (!entity)
			return "none";
		string result = entity.GetName() + " origin=" + entity.GetOrigin();
		EntityPrefabData prefab = entity.GetPrefabData();
		if (prefab)
			result += " prefab=" + prefab.GetPrefabName();
		return result;
	}

	protected bool SurveyCandidate(vector direction, float length, int candidateId, out float offroadRun)
	{
		BaseWorld world = GetGame().GetWorld();
		vector side = Vector(-direction[2], 0, direction[0]);
		vector previous = m_vRouteStart;
		previous[1] = world.GetSurfaceY(previous[0], previous[2]);
		float priorSurface = previous[1];
		float continuousOffroad = 0;
		offroadRun = 0;
		int samples = (int)(length / 5.0);
		for (int sampleIndex = 0; sampleIndex <= samples; sampleIndex++)
		{
			vector sample = m_vRouteStart + direction * (sampleIndex * 5.0);
			sample[1] = world.GetSurfaceY(sample[0], sample[2]);
			float gap;
			float width;
			int roadId;
			bool offroad = IsOffroad(sample, gap, width, roadId);
			float rise = sample[1] - priorSurface;
			float leftSurface = world.GetSurfaceY(sample[0] - side[0] * 2.0, sample[2] - side[2] * 2.0);
			float rightSurface = world.GetSurfaceY(sample[0] + side[0] * 2.0, sample[2] + side[2] * 2.0);
			float crossRise = rightSurface - leftSurface;
			Print("[ConvoyFollower] OFFROAD_SURVEY_SAMPLE: candidate=" + candidateId +
				" sample=" + sampleIndex + " point=" + sample + " road_id=" + roadId +
				" road_dist=" + gap + " road_width=" + width + " required_dist=" + (width * 0.5 + OFFROAD_BUFFER) +
				" offroad=" + offroad + " grade_rise_5m=" + rise + " cross_rise_4m=" + crossRise);
			if (width <= 0 || rise > 0.8 || rise < -0.8 || crossRise > 0.7 || crossRise < -0.7)
			{
				Print("[ConvoyFollower] OFFROAD_SURVEY_REJECT: candidate=" + candidateId + " reason=road_measurement_or_grade");
				return false;
			}
			if (offroad)
			{
				if (sampleIndex > 0)
					continuousOffroad += 5.0;
				if (continuousOffroad > offroadRun)
					offroadRun = continuousOffroad;
			}
			else
				continuousOffroad = 0;
			m_bOccupied = false;
			m_eOccupant = null;
			world.QueryEntitiesBySphere(sample + Vector(0, 1.0, 0), 3.0, SurveyOccupant);
			if (m_bOccupied)
			{
				Print("[ConvoyFollower] OFFROAD_SURVEY_REJECT: candidate=" + candidateId +
					" reason=vehicle point=" + sample + " occupant=" + Describe(m_eOccupant));
				return false;
			}
			if (sampleIndex > 0)
			{
				for (int rail = -1; rail <= 1; rail++)
				{
					vector railStart = previous + side * (rail * 2.0);
					vector railEnd = sample + side * (rail * 2.0);
					railStart[1] = world.GetSurfaceY(railStart[0], railStart[2]) + 1.0;
					railEnd[1] = world.GetSurfaceY(railEnd[0], railEnd[2]) + 1.0;
					TraceParam trace = new TraceParam();
					trace.Start = railStart;
					trace.End = railEnd;
					trace.Flags = TraceFlags.ENTS;
					trace.Exclude = m_Lead;
					float clear = world.TraceMove(trace, SurveyTraceFilter);
					if (clear < 0.98)
					{
						Print("[ConvoyFollower] OFFROAD_SURVEY_REJECT: candidate=" + candidateId +
							" reason=trace sample=" + sampleIndex + " rail=" + rail +
							" fraction=" + clear + " hit=" + Describe(trace.TraceEnt));
						return false;
					}
				}
			}
			priorSurface = sample[1];
			previous = sample;
		}
		float goalGap;
		float goalWidth;
		int goalRoadId;
		if (offroadRun < 50.0 || !IsOffroad(previous, goalGap, goalWidth, goalRoadId))
		{
			Print("[ConvoyFollower] OFFROAD_SURVEY_REJECT: candidate=" + candidateId +
				" reason=insufficient_offroad_corridor continuous_m=" + offroadRun);
			return false;
		}
		Print("[ConvoyFollower] OFFROAD_SURVEY_CANDIDATE: candidate=" + candidateId +
			" length=" + length + " continuous_offroad_m=" + offroadRun + " goal=" + previous);
		return true;
	}

	protected bool SelectRoute()
	{
		m_vRouteStart = m_Lead.GetOrigin();
		vector forward = m_Lead.GetWorldTransformAxis(2);
		forward[1] = 0;
		if (forward.LengthSq() < 0.5)
			return false;
		forward.Normalize();
		vector side = Vector(-forward[2], 0, forward[0]);
		ref array<vector> directions = {};
		directions.Insert(forward);
		directions.Insert(forward * 0.965926 + side * 0.258819);
		directions.Insert(forward * 0.965926 - side * 0.258819);
		directions.Insert(forward * 0.866025 + side * 0.5);
		directions.Insert(forward * 0.866025 - side * 0.5);
		float bestScore = -1.0;
		int candidateId = 0;
		for (int lengthIndex = 0; lengthIndex < 3; lengthIndex++)
		{
			float length = 100.0 - lengthIndex * 20.0;
			for (int directionIndex = 0; directionIndex < directions.Count(); directionIndex++)
			{
				float offroadRun;
				vector direction = directions[directionIndex];
				candidateId++;
				if (!SurveyCandidate(direction, length, candidateId, offroadRun))
					continue;
				float score = length + offroadRun * 0.1 - directionIndex;
				if (score <= bestScore)
					continue;
				bestScore = score;
				m_fSelectedRouteLength = length;
				m_vRouteAxis = direction;
				m_vRouteGoal = m_vRouteStart + direction * length;
				m_vRouteGoal[1] = GetGame().GetWorld().GetSurfaceY(m_vRouteGoal[0], m_vRouteGoal[2]);
			}
		}
		if (bestScore < 0)
			return false;
		Print("[ConvoyFollower] OFFROAD_ROUTE_SELECTED: length=" + m_fSelectedRouteLength +
			" start=" + m_vRouteStart + " goal=" + m_vRouteGoal +
			" axis=" + m_vRouteAxis + " geometry_only=true physical_drive_pending=true");
		return true;
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

	protected bool StartDrive()
	{
		ResetLeadCruiseOverride();
		m_bLeadHoldPoseValid = false;
		m_iLeadHoldStillSeconds = 0;
		if (m_PilotWaypoint)
			m_PilotGroup.RemoveWaypoint(m_PilotWaypoint);
		Resource prefab = Resource.Load("{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et");
		if (!prefab.IsValid())
			return false;
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = m_vRouteGoal;
		SCR_AIWaypoint waypoint = SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(prefab, null, params));
		if (!waypoint)
			return false;
		waypoint.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_GAMEMASTER);
		waypoint.SetCompletionRadius(5.0);
		m_PilotWaypoint = waypoint;
		m_PilotGroup.AddWaypoint(waypoint);
		m_bHoldLead = false;
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (car)
		{
			car.SetPersistentHandBrake(false);
			if (car.GetSimulation())
				car.GetSimulation().SetBreak(0, false);
		}
		for (int index = 0; index < 2; index++)
		{
			Vehicle truck = m_Lead;
			if (index == 1)
				truck = m_Follower;
			m_Starts.Insert(truck.GetOrigin());
			m_LastPositions.Insert(truck.GetOrigin());
			m_Paths.Insert(0);
			m_OffroadPaths.Insert(0);
			m_OffroadMovingSeconds.Insert(0);
			m_MaxOffroadMovingSeconds.Insert(0);
			m_LastSteps.Insert(0);
		}
		SCR_EditorManagerEntity.CloseInstance();
		Print("[ConvoyFollower] OFFROAD_DRIVE_STARTED: production one-truck chain; direct AI goal=" + m_vRouteGoal);
		return true;
	}

	protected bool ChainSeated()
	{
		if (!m_Driver || !m_Driver.CF_IsActiveConvoyMember() || !m_Driver.CF_IsBoarded() ||
			m_Driver.CF_GetAssignedVehicle() != m_Follower || m_Driver.CF_GetUnitNumber() != 1 ||
			m_Driver.CF_IsOrderInverted() || !PilotSeated())
			return false;
		CompartmentAccessComponent access = m_Player.GetCompartmentAccessComponent();
		return access && access.GetVehicleIn(m_Player) == m_Lead;
	}

	protected bool MeasureDrive()
	{
		for (int index = 0; index < 2; index++)
		{
			Vehicle truck = m_Lead;
			if (index == 1)
				truck = m_Follower;
			vector current = truck.GetOrigin();
			vector previous = m_LastPositions[index];
			float step = vector.DistanceXZ(current, previous);
			if (step > 25.0)
			{
				Finish("FAIL position discontinuity exceeds 25m per second");
				return false;
			}
			m_LastSteps[index] = step;
			m_Paths[index] = m_Paths[index] + step;
			float roadGap;
			float roadWidth;
			int roadId;
			bool offroad = IsOffroad(current, roadGap, roadWidth, roadId);
			float priorGap;
			float priorWidth;
			int priorRoadId;
			float midGap;
			float midWidth;
			int midRoadId;
			bool wholeStepOffroad = offroad && IsOffroad(previous, priorGap, priorWidth, priorRoadId) &&
				IsOffroad((current + previous) * 0.5, midGap, midWidth, midRoadId);
			if (wholeStepOffroad && step >= 0.25)
			{
				m_OffroadPaths[index] = m_OffroadPaths[index] + step;
				m_OffroadMovingSeconds[index] = m_OffroadMovingSeconds[index] + 1;
				if (m_OffroadMovingSeconds[index] > m_MaxOffroadMovingSeconds[index])
					m_MaxOffroadMovingSeconds[index] = m_OffroadMovingSeconds[index];
			}
			else
				m_OffroadMovingSeconds[index] = 0;
			vector offset = current - m_vRouteStart;
			float lateral = -offset[0] * m_vRouteAxis[2] + offset[2] * m_vRouteAxis[0];
			Print("[ConvoyFollower] OFFROAD_POSITION: vehicle=" + index + " seconds=" + m_iDrivingSeconds +
				" origin=" + current + " path=" + m_Paths[index] + " step=" + step +
				" offroad_path=" + m_OffroadPaths[index] + " offroad_moving_s=" + m_OffroadMovingSeconds[index] +
				" max_offroad_moving_s=" + m_MaxOffroadMovingSeconds[index] + " road_id=" + roadId +
				" road_dist=" + roadGap + " road_width=" + roadWidth +
				" required_dist=" + (roadWidth * 0.5 + OFFROAD_BUFFER) + " offroad=" + offroad +
				" lateral_m=" + lateral);
			m_LastPositions[index] = current;
			if (roadWidth <= 0 || lateral > 14.0 || lateral < -14.0)
			{
				Finish("FAIL truck left surveyed corridor or road measurement unavailable");
				return false;
			}
		}
		return true;
	}

	protected float Speed(Vehicle truck)
	{
		CarControllerComponent car = CarControllerComponent.Cast(truck.FindComponent(CarControllerComponent));
		if (!car || !car.GetSimulation())
			return 1000.0;
		float speed = car.GetSimulation().GetSpeedKmh();
		if (speed < 0)
			speed = -speed;
		return speed;
	}

	protected void Finish(string result)
	{
		if (m_bFinished)
			return;
		m_bFinished = true;
		m_bHoldLead = true;
		if (m_PilotGroup && m_PilotWaypoint)
			m_PilotGroup.RemoveWaypoint(m_PilotWaypoint);
		m_PilotWaypoint = null;
		if (m_Lead)
			ApplyLeadBrake();
		if (m_Paths.Count() == 2)
		{
			Print("[ConvoyFollower] OFFROAD_FINAL: lead_path=" + m_Paths[0] +
				" follower_path=" + m_Paths[1] + " lead_offroad_path=" + m_OffroadPaths[0] +
				" follower_offroad_path=" + m_OffroadPaths[1] +
				" lead_offroad_moving_s=" + m_MaxOffroadMovingSeconds[0] +
				" follower_offroad_moving_s=" + m_MaxOffroadMovingSeconds[1] +
				" lead_displacement=" + vector.DistanceXZ(m_Lead.GetOrigin(), m_Starts[0]) +
				" follower_displacement=" + vector.DistanceXZ(m_Follower.GetOrigin(), m_Starts[1]) +
				" goal_gap=" + vector.DistanceXZ(m_Lead.GetOrigin(), m_vRouteGoal) +
				" link_gap=" + vector.DistanceXZ(m_Follower.GetOrigin(), m_Lead.GetOrigin()) +
				" max_link_gap=" + m_fMaxGap + " settled_s=" + m_iSettledSeconds +
				" seated_chain=" + ChainSeated() + " max_nonprogress_s=" + m_iMaxNoProgressSeconds);
		}
		if (m_Player)
			Print("[ConvoyFollower] OFFROAD_ROSTER: " + CF_ConvoySession.CF_GetOwnerPanelSnapshot(m_Player));
		Print("[ConvoyFollower] OFFROAD_RESULT: " + result);
		GetGame().GetCallqueue().Remove(Poll);
	}

	protected void Poll()
	{
		if (CF_ConvoySession.CF_IsWorldCleanup())
			return;
		if (m_bFinished)
			return;
		m_iTicks++;
		if (m_iTicks > 300)
		{
			Finish("FAIL total test timeout");
			return;
		}
		if (m_iStage == 0)
		{
			if (!Resolve() || !EnsureOwnerPassenger() || !EnsurePilot())
				return;
			if (!SelectRoute())
			{
				Finish("FAIL no surveyed 60-100m open-field candidate");
				return;
			}
			m_iStage = 1;
		}
		if (m_iStage == 1)
		{
			if (!m_bOrderSent)
			{
				m_bOrderSent = CF_ConvoySession.Start(m_Player, m_Driver);
				Print("[ConvoyFollower] OFFROAD_ORDER: accepted=" + m_bOrderSent);
				if (!m_bOrderSent)
					Finish("FAIL production convoy order rejected");
				return;
			}
			if (!m_Driver.CF_IsActiveConvoyMember())
				return;
			if (!ChainSeated() || vector.DistanceXZ(m_Lead.GetOrigin(), m_vRouteStart) > 2.0)
			{
				Finish("FAIL assigned chain or lead moved from surveyed start");
				return;
			}
			if (!StartDrive())
			{
				Finish("FAIL direct AI lead waypoint unavailable");
				return;
			}
			m_iStage = 2;
		}
		if (m_iStage != 2)
			return;
		m_iDrivingSeconds++;
		if (!ChainSeated() || m_Driver.CF_GetPanelStateLabel() == "lost")
		{
			Finish("FAIL seated chain lost or driver entered LOST");
			return;
		}
		if (!MeasureDrive())
			return;
		float gap = vector.DistanceXZ(m_Follower.GetOrigin(), m_Lead.GetOrigin());
		if (gap > m_fMaxGap)
			m_fMaxGap = gap;
		if (gap > 30.0 && m_LastSteps[0] > 0.5 && m_LastSteps[1] < 0.25)
			m_iNoProgressSeconds++;
		else
			m_iNoProgressSeconds = 0;
		if (m_iNoProgressSeconds > m_iMaxNoProgressSeconds)
			m_iMaxNoProgressSeconds = m_iNoProgressSeconds;
		if (gap > 100.0 || m_iNoProgressSeconds >= 15)
		{
			Finish("FAIL sustained follower separation or nonprogress");
			return;
		}
		float goalGap = vector.DistanceXZ(m_Lead.GetOrigin(), m_vRouteGoal);
		if (!m_bGoalReached && goalGap <= 8.0)
		{
			m_bGoalReached = true;
			m_bHoldLead = true;
			if (m_PilotWaypoint)
				m_PilotGroup.RemoveWaypoint(m_PilotWaypoint);
			m_PilotWaypoint = null;
			RequestLeadCruiseStop();
			Print("[ConvoyFollower] OFFROAD_GOAL_REACHED: goal_gap=" + goalGap + " lead brake applied");
		}
		if (m_bGoalReached && !CheckLeadHold())
			return;
		float finalLeadGap;
		float finalLeadWidth;
		int finalLeadRoad;
		float finalFollowerGap;
		float finalFollowerWidth;
		int finalFollowerRoad;
		bool finalOffroad = IsOffroad(m_Lead.GetOrigin(), finalLeadGap, finalLeadWidth, finalLeadRoad) &&
			IsOffroad(m_Follower.GetOrigin(), finalFollowerGap, finalFollowerWidth, finalFollowerRoad);
		bool settled = m_bGoalReached && goalGap <= 10.0 && gap <= 30.0 && gap >= 7.0 &&
			finalOffroad && Speed(m_Lead) <= 2.0 && Speed(m_Follower) <= 2.0 &&
			m_LastSteps[0] <= 0.3 && m_LastSteps[1] <= 0.3;
		if (settled)
			m_iSettledSeconds++;
		else
			m_iSettledSeconds = 0;
		if (m_iSettledSeconds >= MIN_SETTLED_SECONDS &&
			m_OffroadPaths[0] >= MIN_OFFROAD_PATH && m_OffroadPaths[1] >= MIN_OFFROAD_PATH &&
			m_MaxOffroadMovingSeconds[0] >= MIN_OFFROAD_MOVING_SECONDS &&
			m_MaxOffroadMovingSeconds[1] >= MIN_OFFROAD_MOVING_SECONDS &&
			vector.DistanceXZ(m_Lead.GetOrigin(), m_Starts[0]) >= 40.0 &&
			vector.DistanceXZ(m_Follower.GetOrigin(), m_Starts[1]) >= 40.0)
		{
			Finish("PASS physical offroad one-truck chain and settled goal");
			return;
		}
		if (m_iDrivingSeconds >= 180)
			Finish("FAIL physical offroad movement or settled-goal deadline");
	}
}
