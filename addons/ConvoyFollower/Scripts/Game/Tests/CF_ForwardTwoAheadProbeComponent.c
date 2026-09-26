// Test-only three-truck forward line. The normal smoke probe drives the
// outbound route; this companion tests two real forward releases, then a
// physical lead-vehicle pass and an explicit rejoin order. It never marks an
// accepted button press as a completed maneuver.
class CF_ForwardTwoAheadProbeComponentClass : ScriptComponentClass
{
}

class CF_ForwardTwoAheadProbeComponent : ScriptComponent
{
	protected Vehicle m_Lead;
	protected ChimeraCharacter m_Player;
	protected ChimeraCharacter m_Pilot;
	protected SCR_AIGroup m_PilotGroup;
	protected SCR_AIWaypoint m_PilotWaypoint;
	protected CF_SmokeProbeComponent m_RouteProbe;
	protected ref array<Vehicle> m_Trucks = {};
	protected ref array<CF_DriverControllerComponent> m_Drivers = {};
	protected ref array<vector> m_vResumeStarts = {};
	protected ref array<float> m_fResumePaths = {};
	protected vector m_vBay;
	protected vector m_vFirstPark;
	protected vector m_vLeadShoulder;
	protected vector m_vForward;
	protected vector m_vBayAxis;
	protected vector m_vLastLead;
	protected vector m_vOwnerResumeStart;
	protected vector m_vScriptRoadGoal;
	protected ref array<vector> m_aPilotBypassGoals = {};
	protected float m_fLeadCrossPath;
	protected float m_fOwnerResumePath;
	protected bool m_bShoulderOccupied;
	protected IEntity m_eShoulderOccupant;
	protected bool m_bScriptDriving;
	protected bool m_bPilotBypass;
	protected bool m_bFinished;
	protected int m_iStage;
	protected int m_iStageTicks;
	protected int m_iSeatAttempts;
	protected int m_iPilotBypassIndex;
	protected int m_iBypassPostframeTicks;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Lead = Vehicle.Cast(owner);
		m_RouteProbe = CF_SmokeProbeComponent.Cast(owner.FindComponent(CF_SmokeProbeComponent));
		Print("[ConvoyFollower] AUTO_TWO_AHEAD_INIT: awaiting three-truck road arrival");
		SetEventMask(owner, EntityEvent.POSTFRAME);
		GetGame().GetCallqueue().CallLater(Poll, 1000, true);
	}

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if ((!m_bScriptDriving && !m_bPilotBypass) || !m_Lead)
			return;
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (!car)
			return;
		VehicleWheeledSimulation sim = car.GetSimulation();
		if (!sim)
			return;
		car.SetPersistentHandBrake(false);
		if (!sim.EngineIsOn())
			sim.EngineStart();
		if (sim.GetGear() < 1)
			sim.SetGear(1);
		vector current = m_Lead.GetOrigin();
		vector facing = m_Lead.GetWorldTransformAxis(2);
		vector driveGoal = m_vScriptRoadGoal;
		if (m_bPilotBypass && !m_aPilotBypassGoals.IsEmpty())
		{
			if (m_iPilotBypassIndex < m_aPilotBypassGoals.Count() - 1 &&
				vector.DistanceXZ(current, m_aPilotBypassGoals[m_iPilotBypassIndex]) < 5.0)
			{
				m_iPilotBypassIndex++;
				Print("[ConvoyFollower] AUTO_TWO_AHEAD_BYPASS_STEP: index=" + m_iPilotBypassIndex +
					" goal=" + m_aPilotBypassGoals[m_iPilotBypassIndex]);
			}
			driveGoal = m_aPilotBypassGoals[m_iPilotBypassIndex];
		}
		vector toGoal = driveGoal - current;
		float length = Math.Sqrt(toGoal[0] * toGoal[0] + toGoal[2] * toGoal[2]);
		float steer = 0;
		if (length > 3.0)
		{
			float cross = (facing[0] * toGoal[2] - facing[2] * toGoal[0]) / length;
			steer = cross * 0.8;
			if (steer > 0.3)
				steer = 0.3;
			if (steer < -0.3)
				steer = -0.3;
		}
		sim.SetSteering(steer);
		sim.SetBreak(0, false);
		if (m_bPilotBypass)
		{
			sim.SetThrottle(0.25);
			m_iBypassPostframeTicks++;
			if (m_iBypassPostframeTicks % 250 == 0)
				LogLeadDriveState("AUTO_TWO_AHEAD_DRIVE_POSTFRAME");
		}
		else
			sim.SetThrottle(0.36);
	}

	protected void LogLeadDriveState(string marker)
	{
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (!car)
			return;
		VehicleWheeledSimulation sim = car.GetSimulation();
		if (!sim)
			return;
		Print("[ConvoyFollower] " + marker + ": postframe_ticks=" + m_iBypassPostframeTicks +
			" owner_pilot=" + OwnerIsPilot() + " origin=" + m_Lead.GetOrigin() +
			" engine=" + sim.EngineIsOn() + " rpm=" + sim.EngineGetRPM() +
			" gear=" + sim.GetGear() + " throttle=" + sim.GetThrottle() +
			" brake=" + sim.GetBrake() + " clutch=" + sim.GetClutch() +
			" speed_kmh=" + sim.GetSpeedKmh() +
			" handbrake=" + car.GetHandBrake() +
			" persistent=" + car.GetPersistentHandBrake());
	}

	protected void Finish(string result)
	{
		if (m_bFinished)
			return;
		m_bFinished = true;
		if (m_bPilotBypass && m_Lead)
		{
			CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
			if (car)
			{
				car.SetPersistentHandBrake(true);
				VehicleWheeledSimulation sim = car.GetSimulation();
				if (sim)
				{
					sim.SetThrottle(0);
					sim.SetBreak(1, true);
				}
			}
		}
		m_bScriptDriving = false;
		m_bPilotBypass = false;
		if (m_PilotWaypoint && m_PilotGroup)
			m_PilotGroup.RemoveWaypoint(m_PilotWaypoint);
		Print("[ConvoyFollower] AUTO_TWO_AHEAD_RESULT: " + result);
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

	protected bool Resolve()
	{
		if (!m_Lead || !m_RouteProbe)
			return false;
		BaseWorld world = GetGame().GetWorld();
		while (m_Trucks.Count() < 3)
		{
			Vehicle truck = Vehicle.Cast(world.FindEntityByName("CF_SmokeFollower" + (m_Trucks.Count() + 1)));
			if (!truck)
				return false;
			m_Trucks.Insert(truck);
		}
		while (m_Drivers.Count() < 3)
		{
			CF_DriverControllerComponent driver = FindDriver(m_Drivers.Count() + 1);
			if (!driver)
				return false;
			m_Drivers.Insert(driver);
		}
		if (!m_Player)
		{
			PlayerManager players = GetGame().GetPlayerManager();
			ref array<int> ids = {};
			if (players)
				players.GetPlayers(ids);
			if (!ids.IsEmpty())
				m_Player = ChimeraCharacter.Cast(players.GetPlayerControlledEntity(ids[0]));
		}
		if (!m_PilotGroup)
			m_PilotGroup = SCR_AIGroup.Cast(world.FindEntityByName("CF_SmokePilotGroup"));
		if (m_PilotGroup && !m_Pilot)
		{
			ref array<AIAgent> pilots = {};
			m_PilotGroup.GetAgents(pilots);
			if (pilots.Count() == 1 && pilots[0])
				m_Pilot = ChimeraCharacter.Cast(pilots[0].GetControlledEntity());
		}
		return m_Player && m_Pilot && m_PilotGroup;
	}

	protected bool FilterShoulderTrace(IEntity entity, vector start, vector direction)
	{
		return entity != m_Lead && entity != m_Player;
	}

	protected bool ConsiderShoulderOccupant(IEntity entity)
	{
		if (entity != m_Lead && Vehicle.Cast(entity))
		{
			m_eShoulderOccupant = entity;
			m_bShoulderOccupied = true;
		}
		return true;
	}

	protected string DescribeTraceEntity(IEntity entity)
	{
		if (!entity)
			return "none";
		string description = entity.GetName() + " origin=" + entity.GetOrigin();
		EntityPrefabData data = entity.GetPrefabData();
		if (data)
			description += " prefab=" + data.GetPrefabName();
		return description;
	}

	protected float RoadDistance(vector position)
	{
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return 999.0;
		BaseRoad road;
		float distance;
		aiWorld.GetRoadNetworkManager().GetClosestRoad(position, road, distance);
		if (!road)
			return 999.0;
		return distance;
	}

	protected float ProjectFromBay(vector position)
	{
		vector delta = position - m_vBay;
		return delta[0] * m_vBayAxis[0] + delta[2] * m_vBayAxis[2];
	}

	protected bool TryStageLeadAt(BaseWorld world, vector original, vector candidate, string label)
	{
		float originalSurface = world.GetSurfaceY(original[0], original[2]);
		float candidateSurface = world.GetSurfaceY(candidate[0], candidate[2]);
		float rise = candidateSurface - originalSurface;
		if (rise > 2.5 || rise < -2.5)
		{
			Print("[ConvoyFollower] AUTO_TWO_AHEAD_SHOULDER_REJECT: " + label + " surface_rise=" + rise);
			return false;
		}
		candidate[1] = candidateSurface + (original[1] - originalSurface);
		float roadGap = RoadDistance(candidate);
		if (roadGap < 9.0 || roadGap > 20.0 ||
			vector.Distance(candidate, m_Trucks[0].GetOrigin()) < 9.0 ||
			vector.Distance(candidate, m_Trucks[1].GetOrigin()) < 12.0 ||
			vector.Distance(candidate, m_Trucks[2].GetOrigin()) < 12.0)
		{
			Print("[ConvoyFollower] AUTO_TWO_AHEAD_SHOULDER_REJECT: " + label + " road_gap=" + roadGap +
				" truck_gaps=" + vector.Distance(candidate, m_Trucks[0].GetOrigin()) + "," +
				vector.Distance(candidate, m_Trucks[1].GetOrigin()) + "," +
				vector.Distance(candidate, m_Trucks[2].GetOrigin()));
			return false;
		}
		TraceParam trace = new TraceParam();
		trace.Start = original + Vector(0, 1.5, 0);
		trace.End = candidate + Vector(0, 1.5, 0);
		trace.Flags = TraceFlags.ENTS;
		trace.Exclude = m_Lead;
		float clearFraction = world.TraceMove(trace, FilterShoulderTrace);
		if (clearFraction < 0.98)
		{
			Print("[ConvoyFollower] AUTO_TWO_AHEAD_SHOULDER_REJECT: " + label + " trace_clear_fraction=" + clearFraction);
			return false;
		}
		m_bShoulderOccupied = false;
		world.QueryEntitiesBySphere(candidate, 4.5, ConsiderShoulderOccupant);
		if (m_bShoulderOccupied)
		{
			Print("[ConvoyFollower] AUTO_TWO_AHEAD_SHOULDER_REJECT: " + label + " occupied_by_vehicle=true");
			return false;
		}
		m_Lead.SetOrigin(candidate);
		m_vLeadShoulder = candidate;
		Print("[ConvoyFollower] AUTO_TWO_AHEAD_SHOULDER: test lead only " + original +
			" -> " + candidate + " road_dist=" + roadGap + " choice=" + label);
		return true;
	}

	protected bool StageParkedLeadOnShoulder()
	{
		vector heading = m_Lead.GetWorldTransformAxis(2);
		float length = Math.Sqrt(heading[0] * heading[0] + heading[2] * heading[2]);
		if (length < 0.5)
			return false;
		m_vForward = Vector(heading[0] / length, 0, heading[2] / length);
		BaseWorld world = GetGame().GetWorld();
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return false;
		vector original = m_Lead.GetOrigin();
		vector desiredAhead = original + m_vForward * 85.0;
		BaseRoad currentRoad;
		float currentRoadGap;
		aiWorld.GetRoadNetworkManager().GetClosestRoad(original, currentRoad, currentRoadGap);
		bool wideFixture = currentRoad && currentRoad.GetWidth() >= 8.0 &&
			vector.DistanceXZ(original, Vector(1445.0, 0, 3050.0)) < 35.0;
		if (wideFixture)
			desiredAhead = Vector(1527.0, 37.1, 3069.0);
		vector connectedAhead;
		if (!aiWorld.GetRoadNetworkManager().GetReachableWaypointInRoad(
			m_Trucks[0].GetOrigin(), desiredAhead, 25.0, connectedAhead) ||
			vector.Distance(connectedAhead, original) < 75.0)
		{
			Print("[ConvoyFollower] AUTO_TWO_AHEAD_SHOULDER_REJECT: road_ahead_unavailable desired=" +
				desiredAhead + " resolved=" + connectedAhead + " wide=" + wideFixture);
			return false;
		}
		if (wideFixture)
		{
			vector fixedShoulder = Vector(1441.25, 0, 3059.30);
			if (vector.DistanceXZ(original, fixedShoulder) > 15.0)
			{
				Print("[ConvoyFollower] AUTO_TWO_AHEAD_SHOULDER_REJECT: fixed shoulder too far from parked lead " +
					vector.DistanceXZ(original, fixedShoulder));
				return false;
			}
			return TryStageLeadAt(world, original, fixedShoulder, "candidate28_fixed");
		}
		for (int sideIndex = 0; sideIndex < 2; sideIndex++)
		{
			float side = 1.0;
			if (sideIndex == 1)
				side = -1.0;
			for (int distanceIndex = 0; distanceIndex < 2; distanceIndex++)
			{
				float offset = 10.0 + distanceIndex * 2.0;
				vector candidate = original;
				candidate[0] = candidate[0] - m_vForward[2] * side * offset;
				candidate[2] = candidate[2] + m_vForward[0] * side * offset;
				if (TryStageLeadAt(world, original, candidate, "dynamic"))
					return true;
			}
		}
		return false;
	}

	protected bool AllDriversSeated()
	{
		for (int i = 0; i < 3; i++)
		{
			if (!m_Drivers[i].CF_IsBoarded() || m_Drivers[i].CF_GetAssignedVehicle() != m_Trucks[i])
				return false;
		}
		return true;
	}

	protected bool IsWideBypassFixture()
	{
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return false;
		BaseRoad road;
		float roadGap;
		aiWorld.GetRoadNetworkManager().GetClosestRoad(m_vFirstPark, road, roadGap);
		return road && road.GetWidth() >= 8.0 && roadGap <= 5.0 &&
			vector.DistanceXZ(m_vFirstPark, Vector(1520.0, 0, 3068.0)) < 50.0;
	}

	// The owner test vehicle follows a surveyed, physically drivable lane around
	// the parked trucks. Every short segment is checked for terrain grade,
	// mapped-road proximity, and vehicle/entity clearance before moving.
	protected bool BuildPilotBypass()
	{
		for (int choice = 0; choice < 3; choice++)
		{
			float lateralOffset = 6.5 + choice * 1.75;
			if (!TryBuildPilotBypass(lateralOffset))
				continue;
			Print("[ConvoyFollower] AUTO_TWO_AHEAD_BYPASS_SELECTED: offset_m=" + lateralOffset +
				" goals=" + m_aPilotBypassGoals.Count());
			return true;
		}
		m_aPilotBypassGoals.Clear();
		return false;
	}

	protected bool TryBuildPilotBypass(float lateralOffset)
	{
		BaseWorld world = GetGame().GetWorld();
		// The test lead is staged on the positive-Z shoulder. Stay on that
		// side of the mapped road while physically passing the parked trucks.
		vector shoulderSide = Vector(-m_vBayAxis[2], 0, m_vBayAxis[0]);
		Print("[ConvoyFollower] AUTO_TWO_AHEAD_BYPASS_SIDE: shoulder_normal=" + shoulderSide +
			" lateral_offset_m=" + lateralOffset + " staged_lead=" + m_Lead.GetOrigin() +
			" staged_lead_road_gap=" + RoadDistance(m_Lead.GetOrigin()));
		m_aPilotBypassGoals.Clear();
		m_aPilotBypassGoals.Insert(m_Trucks[1].GetOrigin() - m_vBayAxis * 22.0 + shoulderSide * lateralOffset);
		m_aPilotBypassGoals.Insert(m_Trucks[1].GetOrigin() + shoulderSide * lateralOffset);
		m_aPilotBypassGoals.Insert(m_Trucks[0].GetOrigin() + shoulderSide * lateralOffset);
		m_aPilotBypassGoals.Insert(m_Trucks[0].GetOrigin() + m_vBayAxis * 30.0 + shoulderSide * 2.0);
		vector previous = m_Lead.GetOrigin();
		for (int targetIndex = 0; targetIndex < m_aPilotBypassGoals.Count(); targetIndex++)
		{
			vector target = m_aPilotBypassGoals[targetIndex];
			target[1] = world.GetSurfaceY(target[0], target[2]);
			m_aPilotBypassGoals[targetIndex] = target;
			float segmentLength = vector.DistanceXZ(previous, target);
			int samples = (int)(segmentLength / 5.0) + 1;
			float priorSurface = world.GetSurfaceY(previous[0], previous[2]);
			for (int sampleIndex = 1; sampleIndex <= samples; sampleIndex++)
			{
				float fraction = (float)sampleIndex / samples;
				vector sample = previous + (target - previous) * fraction;
				float surface = world.GetSurfaceY(sample[0], sample[2]);
				float rise = surface - priorSurface;
				float roadGap = RoadDistance(sample);
				if (rise > 1.5 || rise < -1.5 || roadGap > 11.0)
				{
					Print("[ConvoyFollower] AUTO_TWO_AHEAD_BYPASS_REJECT: offset=" + lateralOffset +
						" target=" + targetIndex + " sample=" + sampleIndex +
						" point=" + sample + " grade_step=" + rise + " road_gap=" + roadGap);
					return false;
				}
				sample[1] = surface;
				m_bShoulderOccupied = false;
				m_eShoulderOccupant = null;
				world.QueryEntitiesBySphere(sample, 4.0, ConsiderShoulderOccupant);
				if (m_bShoulderOccupied)
				{
					Print("[ConvoyFollower] AUTO_TWO_AHEAD_BYPASS_REJECT: offset=" + lateralOffset +
						" target=" + targetIndex + " sample=" + sampleIndex +
						" point=" + sample + " vehicle_occupied=" + DescribeTraceEntity(m_eShoulderOccupant));
					return false;
				}
				priorSurface = surface;
			}
			TraceParam trace = new TraceParam();
			trace.Start = previous + Vector(0, 1.5, 0);
			trace.End = target + Vector(0, 1.5, 0);
			trace.Flags = TraceFlags.ENTS;
			trace.Exclude = m_Lead;
			float clearFraction = world.TraceMove(trace, FilterShoulderTrace);
			if (clearFraction < 0.98)
			{
				vector hitPoint = trace.Start + (trace.End - trace.Start) * clearFraction;
				Print("[ConvoyFollower] AUTO_TWO_AHEAD_BYPASS_REJECT: offset=" + lateralOffset +
					" target=" + targetIndex + " start=" + trace.Start + " end=" + trace.End +
					" trace_clear_fraction=" + clearFraction + " hit_point=" + hitPoint +
					" hit_entity=" + DescribeTraceEntity(trace.TraceEnt) +
					" collider=" + trace.ColliderName + " material=" + trace.TraceMaterial);
				return false;
			}
			Print("[ConvoyFollower] AUTO_TWO_AHEAD_BYPASS_ROUTE: offset=" + lateralOffset +
				" target=" + targetIndex +
				" point=" + target + " segment_m=" + segmentLength + " road_gap=" + RoadDistance(target));
			previous = target;
		}
		m_iPilotBypassIndex = 0;
		return true;
	}

	protected bool StartPilotPass()
	{
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return false;
		if (IsWideBypassFixture())
		{
			if (!OwnerIsPilot() || !BuildPilotBypass())
				return false;
			m_RouteProbe.CF_ReleaseLeadBrakeForReturn();
			m_vLastLead = m_Lead.GetOrigin();
			m_iBypassPostframeTicks = 0;
			m_bPilotBypass = true;
			Print("[ConvoyFollower] AUTO_TWO_AHEAD_OWNER_PASS: player seated; test-only steering physically driving lead around parked line");
			return true;
		}
		vector desired = m_vFirstPark + m_vBayAxis * 45.0;
		vector connectedGoal;
		if (!aiWorld.GetRoadNetworkManager().GetReachableWaypointInRoad(
			m_Lead.GetOrigin(), desired, 35.0, connectedGoal) ||
			ProjectFromBay(connectedGoal) < ProjectFromBay(m_vFirstPark) + 25.0)
			return false;
		Resource prefab = Resource.Load("{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et");
		if (!prefab.IsValid())
			return false;
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = connectedGoal;
		m_PilotWaypoint = SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(prefab, null, params));
		if (!m_PilotWaypoint)
			return false;
		m_PilotWaypoint.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_GAMEMASTER);
		m_PilotWaypoint.SetCompletionRadius(5.0);
		m_RouteProbe.CF_ReleaseLeadBrakeForReturn();
		m_PilotGroup.AddWaypoint(m_PilotWaypoint);
		m_vLastLead = m_Lead.GetOrigin();
		Print("[ConvoyFollower] AUTO_TWO_AHEAD_OWNER_PASS: AI pilot physically driving lead from shoulder " +
			m_vLastLead + " to connected road " + connectedGoal + " beyond parked line");
		return true;
	}

	protected bool OwnerIsPilot()
	{
		CompartmentAccessComponent access = m_Player.GetCompartmentAccessComponent();
		BaseCompartmentSlot slot;
		if (access)
			slot = access.GetCompartment();
		return slot && slot.IsPiloting() && access.GetVehicleIn(m_Player) == m_Lead;
	}

	protected bool MoveOwnerToPilotSeat()
	{
		CompartmentAccessComponent pilotAccess = m_Pilot.GetCompartmentAccessComponent();
		CompartmentAccessComponent playerAccess = m_Player.GetCompartmentAccessComponent();
		if (!pilotAccess || !playerAccess)
			return false;
		BaseCompartmentSlot pilotSlot = pilotAccess.GetCompartment();
		if (pilotSlot && pilotSlot.IsPiloting())
		{
			if (!pilotAccess.GetOutVehicle(EGetOutType.TELEPORT, -1, ECloseDoorAfterActions.CLOSE_DOOR, true, true))
				return false;
			Print("[ConvoyFollower] AUTO_TWO_AHEAD_PILOT_EXIT: AI pilot left stopped lead");
			return false;
		}
		if (OwnerIsPilot())
			return true;
		BaseCompartmentSlot playerSlot = playerAccess.GetCompartment();
		if (playerSlot)
		{
			if (!playerAccess.GetOutVehicle(EGetOutType.TELEPORT, -1, ECloseDoorAfterActions.CLOSE_DOOR, true, true))
				return false;
			return false;
		}
		BaseCompartmentManagerComponent manager = BaseCompartmentManagerComponent.Cast(m_Lead.FindComponent(BaseCompartmentManagerComponent));
		if (!manager)
			return false;
		ref array<BaseCompartmentSlot> slots = {};
		manager.GetCompartments(slots);
		foreach (BaseCompartmentSlot slot : slots)
		{
			if (slot && slot.IsPiloting() && !slot.IsOccupied() && !slot.IsReserved())
			{
				if (playerAccess.GetInVehicle(m_Lead, slot, true, -1, ECloseDoorAfterActions.CLOSE_DOOR, true))
					Print("[ConvoyFollower] AUTO_TWO_AHEAD_OWNER_SEAT: owner pilot seat requested");
				break;
			}
		}
		return false;
	}

	protected void LogOrderPosition(string marker)
	{
		Print("[ConvoyFollower] " + marker + ": owner=" + m_Lead.GetOrigin() +
			" parked1=" + m_Trucks[0].GetOrigin() + " parked2=" + m_Trucks[1].GetOrigin() +
			" active3=" + m_Trucks[2].GetOrigin() + " projected=" +
			ProjectFromBay(m_Lead.GetOrigin()) + "," + ProjectFromBay(m_Trucks[0].GetOrigin()) +
			"," + ProjectFromBay(m_Trucks[1].GetOrigin()) + "," + ProjectFromBay(m_Trucks[2].GetOrigin()) +
			" road_dist=" + RoadDistance(m_Lead.GetOrigin()) + "," + RoadDistance(m_Trucks[0].GetOrigin()) +
			"," + RoadDistance(m_Trucks[1].GetOrigin()) + "," + RoadDistance(m_Trucks[2].GetOrigin()) +
			" seated=" + m_Drivers[0].CF_IsBoarded() + "," + m_Drivers[1].CF_IsBoarded() +
			"," + m_Drivers[2].CF_IsBoarded());
	}

	protected bool SnapshotShowsChain()
	{
		string snapshot = CF_ConvoySession.CF_GetOwnerPanelSnapshot(m_Player);
		ref array<string> rows = {};
		snapshot.Split(";", rows, false);
		if (rows.Count() < 3)
			return false;
		for (int i = 0; i < 3; i++)
		{
			ref array<string> fields = {};
			rows[i].Split("|", fields, false);
			if (fields.Count() < 5 || fields[0].ToInt() != i + 1 || fields[1].ToInt() != i + 1)
				return false;
			if (i == 0 && fields[4] != "owner")
				return false;
			if (i > 0 && fields[4] != "Unit " + i)
				return false;
		}
		Print("[ConvoyFollower] AUTO_TWO_AHEAD_CHAIN: " + snapshot);
		return true;
	}

	protected void Poll()
	{
		if (m_bFinished)
			return;
		m_iStageTicks++;
		if (!Resolve())
		{
			if (m_iStageTicks >= 60)
				Finish("FAIL owner, pilot, drivers, vehicles, or route probe missing");
			return;
		}
		if (m_iStage == 0)
		{
			if (!m_RouteProbe.CF_HasPassedRoadArrival())
			{
				if (m_iStageTicks >= 300)
					Finish("FAIL three-truck road arrival did not pass");
				return;
			}
			if (!AllDriversSeated())
			{
				Finish("FAIL a follower lost the assigned driver seat at arrival");
				return;
			}
			m_vBay = m_Trucks[0].GetOrigin();
			if (!StageParkedLeadOnShoulder())
			{
				Finish("FAIL no level, clear shoulder for test lead");
				return;
			}
			m_iStage = 1;
			m_iStageTicks = 0;
			return;
		}
		if (m_iStage == 1)
		{
			if (m_iStageTicks < 3)
				return;
			if (vector.Distance(m_Lead.GetOrigin(), m_vLeadShoulder) > 2.0)
			{
				Finish("FAIL lead did not remain on staged shoulder");
				return;
			}
			if (!CF_ConvoySession.CanReleaseAtUnload(m_Player, m_Drivers[0]))
			{
				if (m_iStageTicks >= 25)
					Finish("FAIL Unit 1 did not regain release-ready after shoulder staging");
				return;
			}
			bool accepted1 = CF_ConvoySession.CF_PanelPullAhead(m_Player, 1);
			if (!accepted1)
				Print("[ConvoyFollower] AUTO_TWO_AHEAD_ORDER: unit=1 accepted=false reason=" +
					CF_ConvoySession.GetReleasePlanFailureReason(m_Player));
			else
				Print("[ConvoyFollower] AUTO_TWO_AHEAD_ORDER: unit=1 accepted=true");
			if (!accepted1)
			{
				Finish("FAIL first forward order rejected");
				return;
			}
			m_iStage = 2;
			m_iStageTicks = 0;
			return;
		}
		if (m_iStage == 2)
		{
			bool parked1 = m_Drivers[0].CF_IsForwardWaitParked();
			bool ready2 = CF_ConvoySession.CanReleaseAtUnload(m_Player, m_Drivers[1]);
			if (m_iStageTicks % 5 == 0)
				Print("[ConvoyFollower] AUTO_TWO_AHEAD_STATUS: stage=first parked=" + parked1 +
					" unit2_ready=" + ready2 + " unit2_bay_gap=" + vector.Distance(m_Trucks[1].GetOrigin(), m_vBay));
			if (!parked1 || !ready2 || vector.Distance(m_Trucks[1].GetOrigin(), m_vBay) > 8.0)
			{
				if (m_iStageTicks >= 120)
					Finish("FAIL Unit 1 did not park ahead or Unit 2 did not reach the bay");
				return;
			}
			m_vFirstPark = m_Trucks[0].GetOrigin();
			vector axis = m_vFirstPark - m_vBay;
			float axisLength = Math.Sqrt(axis[0] * axis[0] + axis[2] * axis[2]);
			if (axisLength < 30.0 || RoadDistance(m_vFirstPark) > 5.0 || !AllDriversSeated())
			{
				Finish("FAIL first forward slot was not clear, connected, and seated");
				return;
			}
			m_vBayAxis = Vector(axis[0] / axisLength, 0, axis[2] / axisLength);
			bool accepted2 = CF_ConvoySession.CF_PanelPullAhead(m_Player, 2);
			if (!accepted2)
				Print("[ConvoyFollower] AUTO_TWO_AHEAD_ORDER: unit=2 accepted=false first_park=" +
					m_vFirstPark + " reason=" + CF_ConvoySession.GetReleasePlanFailureReason(m_Player));
			else
				Print("[ConvoyFollower] AUTO_TWO_AHEAD_ORDER: unit=2 accepted=true first_park=" + m_vFirstPark);
			if (!accepted2)
			{
				Finish("FAIL second forward order rejected; inspect distinct slot geometry");
				return;
			}
			m_iStage = 3;
			m_iStageTicks = 0;
			return;
		}
		if (m_iStage == 3)
		{
			bool parked2 = m_Drivers[1].CF_IsForwardWaitParked();
			bool ready3 = CF_ConvoySession.CanReleaseAtUnload(m_Player, m_Drivers[2]);
			if (m_iStageTicks % 5 == 0)
				Print("[ConvoyFollower] AUTO_TWO_AHEAD_STATUS: stage=second parked=" + parked2 +
					" unit3_ready=" + ready3 + " unit3_bay_gap=" + vector.Distance(m_Trucks[2].GetOrigin(), m_vBay));
			if (!parked2 || !ready3 || vector.Distance(m_Trucks[2].GetOrigin(), m_vBay) > 8.0)
			{
				if (m_iStageTicks >= 120)
					Finish("FAIL Unit 2 did not park ahead or Unit 3 did not reach the bay");
				return;
			}
			float spacing = vector.DistanceXZ(m_Trucks[0].GetOrigin(), m_Trucks[1].GetOrigin());
			float orderSpacing = ProjectFromBay(m_Trucks[0].GetOrigin()) - ProjectFromBay(m_Trucks[1].GetOrigin());
			Print("[ConvoyFollower] AUTO_TWO_AHEAD_PARKED: spacing=" + spacing +
				" projected_order_spacing=" + orderSpacing + " unit1=" + m_Trucks[0].GetOrigin() +
				" unit2=" + m_Trucks[1].GetOrigin() + " unit3_bay=" + m_Trucks[2].GetOrigin());
			if (!AllDriversSeated() || spacing < 16.0 || orderSpacing < 16.0 ||
				RoadDistance(m_Trucks[0].GetOrigin()) > 5.0 || RoadDistance(m_Trucks[1].GetOrigin()) > 5.0)
			{
				Finish("FAIL two parked trucks are too close, inverted, off road, or unseated");
				return;
			}
			if (IsWideBypassFixture() && !MoveOwnerToPilotSeat())
			{
				m_iSeatAttempts++;
				if (m_iSeatAttempts >= 15)
					Finish("FAIL owner could not take pilot seat before physical bypass");
				return;
			}
			m_iSeatAttempts = 0;
			if (!StartPilotPass())
			{
				Finish("FAIL no clear physical owner-vehicle route past parked line");
				return;
			}
			m_iStage = 4;
			m_iStageTicks = 0;
			return;
		}
		if (m_iStage == 4)
		{
			vector leadNow = m_Lead.GetOrigin();
			m_fLeadCrossPath += vector.Distance(leadNow, m_vLastLead);
			m_vLastLead = leadNow;
			if (m_iStageTicks % 5 == 0)
			{
				Print("[ConvoyFollower] AUTO_TWO_AHEAD_OWNER_CROSS: lead=" + leadNow +
					" physical_path=" + m_fLeadCrossPath + " ahead_of_first=" +
					(ProjectFromBay(leadNow) - ProjectFromBay(m_Trucks[0].GetOrigin())) +
					" road_dist=" + RoadDistance(leadNow) +
					" bypass_index=" + m_iPilotBypassIndex +
					" near_park_gap=" + vector.DistanceXZ(leadNow, m_Trucks[1].GetOrigin()) +
					" far_park_gap=" + vector.DistanceXZ(leadNow, m_Trucks[0].GetOrigin()));
				LogLeadDriveState("AUTO_TWO_AHEAD_DRIVE_POLL");
			}
			if (m_fLeadCrossPath >= 75.0 &&
				ProjectFromBay(leadNow) - ProjectFromBay(m_Trucks[0].GetOrigin()) >= 15.0 &&
				RoadDistance(leadNow) <= 6.0)
			{
				m_bPilotBypass = false;
				if (m_PilotWaypoint)
					m_PilotGroup.RemoveWaypoint(m_PilotWaypoint);
				m_PilotWaypoint = null;
				CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
				if (car)
				{
					car.SetPersistentHandBrake(true);
					VehicleWheeledSimulation sim = car.GetSimulation();
					if (sim)
					{
						sim.SetThrottle(0);
						sim.SetBreak(1, true);
					}
				}
				Print("[ConvoyFollower] AUTO_TWO_AHEAD_OWNER_PASSED: lead physically crossed forward line after " +
					m_fLeadCrossPath + " m; waiting to transfer owner into driver seat");
				m_iStage = 5;
				m_iStageTicks = 0;
				m_vLastLead = leadNow;
				return;
			}
			if (m_iStageTicks >= 120)
				Finish("FAIL AI-piloted owner vehicle did not physically pass parked-ahead line");
			return;
		}
		if (m_iStage == 5)
		{
			float step = vector.Distance(m_Lead.GetOrigin(), m_vLastLead);
			m_vLastLead = m_Lead.GetOrigin();
			if (m_iStageTicks < 3 || step > 1.5)
			{
				if (m_iStageTicks >= 20)
					Finish("FAIL lead did not settle safely for owner pilot transfer");
				return;
			}
			if (!MoveOwnerToPilotSeat())
			{
				m_iSeatAttempts++;
				if (m_iSeatAttempts >= 15)
					Finish("FAIL owner could not take lead pilot seat");
				return;
			}
			m_iStage = 6;
			m_iStageTicks = 0;
			return;
		}
		if (m_iStage == 6)
		{
			if (!OwnerIsPilot() || !AllDriversSeated())
			{
				Finish("FAIL owner pilot or convoy seats lost before explicit resume");
				return;
			}
			LogOrderPosition("AUTO_TWO_AHEAD_PRE_RESUME");
			if (ProjectFromBay(m_Trucks[2].GetOrigin()) > ProjectFromBay(m_Trucks[1].GetOrigin()) + 8.0)
			{
				Finish("FAIL unreleased Unit 3 passed parked Unit 2 before resume; chain physically inverted");
				return;
			}
			bool resumed = CF_ConvoySession.CF_PanelResumeForwardLine(m_Player);
			Print("[ConvoyFollower] AUTO_TWO_AHEAD_RESUME: accepted=" + resumed);
			if (!resumed || !SnapshotShowsChain())
			{
				Finish("FAIL explicit resume rejected or owner->Unit1->Unit2->Unit3 chain not rebuilt");
				return;
			}
			m_vOwnerResumeStart = m_Lead.GetOrigin();
			m_vLastLead = m_vOwnerResumeStart;
			for (int i = 0; i < 3; i++)
			{
				m_vResumeStarts.Insert(m_Trucks[i].GetOrigin());
				m_fResumePaths.Insert(0);
			}
			ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
			vector desired = m_Lead.GetOrigin() + m_vBayAxis * 65.0;
			if (!aiWorld || !aiWorld.GetRoadNetworkManager() ||
				!aiWorld.GetRoadNetworkManager().GetReachableWaypointInRoad(m_Lead.GetOrigin(), desired, 25.0, m_vScriptRoadGoal) ||
				ProjectFromBay(m_vScriptRoadGoal) < ProjectFromBay(m_Lead.GetOrigin()) + 30.0)
			{
				Finish("FAIL no connected road for player-piloted continuation");
				return;
			}
			m_bScriptDriving = true;
			Print("[ConvoyFollower] AUTO_TWO_AHEAD_SCRIPT_DRIVE: test-only simulation throttle with owner in real pilot seat toward " + m_vScriptRoadGoal);
			m_iStage = 7;
			m_iStageTicks = 0;
			return;
		}
		if (m_iStage == 7)
		{
			if (!OwnerIsPilot() || !AllDriversSeated())
			{
				Finish("FAIL pilot or follower seats lost during resumed drive");
				return;
			}
			m_fOwnerResumePath += vector.Distance(m_Lead.GetOrigin(), m_vLastLead);
			m_vLastLead = m_Lead.GetOrigin();
			for (int i = 0; i < 3; i++)
			{
				m_fResumePaths[i] = m_fResumePaths[i] + vector.Distance(m_Trucks[i].GetOrigin(), m_vResumeStarts[i]);
				m_vResumeStarts[i] = m_Trucks[i].GetOrigin();
			}
			if (m_iStageTicks % 5 == 0)
			{
				LogOrderPosition("AUTO_TWO_AHEAD_POST_RESUME");
				Print("[ConvoyFollower] AUTO_TWO_AHEAD_MOVEMENT: owner=" + m_fOwnerResumePath +
					" followers=" + m_fResumePaths[0] + "," + m_fResumePaths[1] + "," + m_fResumePaths[2]);
			}
			if (m_fOwnerResumePath >= 30.0 && m_fResumePaths[0] >= 12.0 &&
				m_fResumePaths[1] >= 12.0 && m_fResumePaths[2] >= 10.0 &&
				RoadDistance(m_Lead.GetOrigin()) <= 8.0 && SnapshotShowsChain())
				Finish("PASS two forward trucks parked in order, owner physically passed line, explicit resume rebuilt chain, all three drove again");
			else if (m_iStageTicks >= 120)
				Finish("FAIL resumed owner or one of three linked trucks did not physically move on road");
		}
	}
}
