// Test-only, visible arrival/hold/resume route. The owner occupies the real
// pilot seat; POSTFRAME steering physically drives the lead truck on road81.
// Server orders and vehicle movement, rather than button acceptance alone,
// determine the terminal result.
class CF_ShortArrivalProbeComponentClass : ScriptComponentClass
{
}

class CF_ShortArrivalProbeComponent : ScriptComponent
{
	[Attribute(defvalue: "1", params: "1 2 1", desc: "Number of staged convoy followers")]
	protected int m_iExpectedTrucks;
	protected Vehicle m_Lead;
	protected ChimeraCharacter m_Player;
	protected ref array<CF_DriverControllerComponent> m_Drivers = {};
	protected ref array<Vehicle> m_Trucks = {};
	protected ref array<float> m_FollowerPaths = {};
	protected ref array<float> m_SecondPaths = {};
	protected ref array<vector> m_LastFollowerPositions = {};
	protected ref array<vector> m_ArrivalSamples = {};
	protected ref array<vector> m_SecondStarts = {};
	protected ref array<vector> m_RoadPoints = {};
	protected vector m_vBay;
	protected vector m_vSecondGoal;
	protected vector m_vLastLead;
	protected vector m_vLeadStart;
	protected vector m_vSecondLeadStart;
	protected vector m_vArrivalLeadSample;
	protected float m_fBayArc;
	protected float m_fSecondArc;
	protected float m_fTargetArc;
	protected float m_fLeadPath;
	protected float m_fSecondLeadPath;
	protected float m_fLastPilotArc;
	protected int m_iStage;
	protected int m_iStageSeconds;
	protected int m_iTotalSeconds;
	protected int m_iNextOrder;
	protected int m_iStableSeconds;
	protected int m_iPostframeTicks;
	protected bool m_bBrake = true;
	protected bool m_bDriving;
	protected bool m_bFinished;
	protected string m_sLastPanelState;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Lead = Vehicle.Cast(owner);
		if (!m_Lead)
			return;
		SetEventMask(owner, EntityEvent.POSTFRAME);
		CF_ConvoySettings.Get();
		GetGame().GetCallqueue().CallLater(Poll, 1000, true);
		Print("[ConvoyFollower] SHORT_ARRIVAL_INIT: expected=" + m_iExpectedTrucks +
			" owner pilot; physically driven lead; no vehicle teleport");
	}

	protected void SetStage(int stage, string reason)
	{
		m_iStage = stage;
		m_iStageSeconds = 0;
		m_iStableSeconds = 0;
		Print("[ConvoyFollower] SHORT_ARRIVAL_STAGE: stage=" + stage + " " + reason);
	}

	protected void Finish(string result)
	{
		if (m_bFinished)
			return;
		m_bFinished = true;
		m_bDriving = false;
		m_bBrake = true;
		ApplyBrake();
		LogState("SHORT_ARRIVAL_FINAL");
		Print("[ConvoyFollower] SHORT_ARRIVAL_RESULT: " + result +
			" lead_path=" + m_fLeadPath + " second_lead_path=" + m_fSecondLeadPath +
			" lead=" + m_Lead.GetOrigin() + " bay=" + m_vBay);
		GetGame().GetCallqueue().Remove(Poll);
	}

	protected bool EnsureOwnerPilot()
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
			Print("[ConvoyFollower] SHORT_ARRIVAL_PLAYER: possessed test crew");
		}
		if (players.GetPlayerIdFromControlledEntity(m_Player) <= 0)
			return false;
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
					Print("[ConvoyFollower] SHORT_ARRIVAL_OWNER_SEAT: pilot seat requested");
				break;
			}
		}
		return false;
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

	protected bool OwnerIsPilot()
	{
		if (!m_Player)
			return false;
		CompartmentAccessComponent access = m_Player.GetCompartmentAccessComponent();
		if (!access)
			return false;
		BaseCompartmentSlot slot = access.GetCompartment();
		return slot && slot.IsPiloting() && access.GetVehicleIn(m_Player) == m_Lead;
	}

	protected void SelectTestCamera()
	{
		PlayerController local = GetGame().GetPlayerController();
		if (!local || local.GetControlledEntity() != m_Player)
			return;
		bool closed = !SCR_EditorManagerEntity.IsOpenedInstance() || SCR_EditorManagerEntity.CloseInstance();
		PlayerCamera playerCamera = local.GetPlayerCamera();
		if (closed && playerCamera && GetGame().GetCameraManager().SetCamera(playerCamera))
		{
			CameraHandlerComponent handler = CameraHandlerComponent.Cast(m_Player.FindComponent(CameraHandlerComponent));
			if (handler)
				handler.SetThirdPerson(true);
			Print("[ConvoyFollower] SHORT_ARRIVAL_CAMERA_READY: owner camera released to test film component");
		}
	}

	protected vector PointAtArc(float targetArc)
	{
		float arc = 0.0;
		for (int i = 1; i < m_RoadPoints.Count(); i++)
		{
			float length = vector.DistanceXZ(m_RoadPoints[i - 1], m_RoadPoints[i]);
			if (length < 0.01)
				continue;
			if (arc + length >= targetArc)
				return m_RoadPoints[i - 1] +
					(m_RoadPoints[i] - m_RoadPoints[i - 1]) * ((targetArc - arc) / length);
			arc += length;
		}
		return m_RoadPoints[m_RoadPoints.Count() - 1];
	}

	protected float ArcAt(vector position)
	{
		float cumulative = 0.0;
		float bestArc = 0.0;
		float bestGap = 1000000.0;
		for (int i = 1; i < m_RoadPoints.Count(); i++)
		{
			vector start = m_RoadPoints[i - 1];
			vector end = m_RoadPoints[i];
			float dx = end[0] - start[0];
			float dz = end[2] - start[2];
			float length = vector.DistanceXZ(start, end);
			if (length < 0.01)
				continue;
			float fraction = ((position[0] - start[0]) * dx +
				(position[2] - start[2]) * dz) / (length * length);
			if (fraction < 0.0)
				fraction = 0.0;
			if (fraction > 1.0)
				fraction = 1.0;
			vector projected = start + (end - start) * fraction;
			float gap = vector.DistanceXZ(projected, position);
			if (gap < bestGap)
			{
				bestGap = gap;
				bestArc = cumulative + length * fraction;
			}
			cumulative += length;
		}
		return bestArc;
	}

	protected bool FindSurveyedRoad()
	{
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return false;
		RoadNetworkManager roads = aiWorld.GetRoadNetworkManager();
		ref array<BaseRoad> candidates = {};
		roads.GetRoadsInAABB(Vector(1100, -100, 2850), Vector(1750, 700, 3250), candidates);
		vector expectedBay = Vector(1479.70, 35.5199, 3060.25);
		float bestError = 1000000.0;
		BaseRoad selected;
		for (int roadIndex = 0; roadIndex < candidates.Count(); roadIndex++)
		{
			BaseRoad road = candidates[roadIndex];
			if (!road || road.GetWidth() < 8.0)
				continue;
			ref array<vector> points = {};
			road.GetPoints(points);
			if (points.Count() < 2)
				continue;
			float length = 0.0;
			for (int i = 1; i < points.Count(); i++)
				length += vector.DistanceXZ(points[i - 1], points[i]);
			if (length < 450.0 || length > 520.0)
				continue;
			m_RoadPoints = points;
			float probeArc = length * 0.70;
			float error = vector.DistanceXZ(PointAtArc(probeArc), expectedBay);
			if (error < bestError)
			{
				bestError = error;
				selected = road;
				m_fBayArc = probeArc;
			}
		}
		if (!selected || bestError > 3.0)
			return false;
		m_RoadPoints.Clear();
		selected.GetPoints(m_RoadPoints);
		m_fSecondArc = m_fBayArc + 65.0;
		m_vBay = PointAtArc(m_fBayArc);
		m_vSecondGoal = PointAtArc(m_fSecondArc);
		Print("[ConvoyFollower] SHORT_ARRIVAL_ROAD: width=" + selected.GetWidth() +
			" bay=" + m_vBay + " second=" + m_vSecondGoal + " bay_arc=" + m_fBayArc +
			" start_arc=" + ArcAt(m_Lead.GetOrigin()) + " surveyed_match_gap=" + bestError);
		return true;
	}

	protected float RoadGap(vector point)
	{
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return 999.0;
		BaseRoad road;
		float gap;
		aiWorld.GetRoadNetworkManager().GetClosestRoad(point, road, gap);
		if (!road)
			return 999.0;
		return gap;
	}

	protected void ApplyBrake()
	{
		if (!m_Lead)
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

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (m_bFinished || !m_Lead)
			return;
		if (m_bBrake)
		{
			ApplyBrake();
			return;
		}
		if (!m_bDriving || m_RoadPoints.Count() < 2)
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
		// Installed SCR_FuelConsumptionComponent defines neutral as index 1.
		// Bohemia's cinematic vehicleGO sample drives in index 2; clutch 1
		// is fully engaged according to VehicleWheeledSimulation.GetClutch.
		if (sim.GetGear() != 2)
			sim.SetGear(2);
		sim.SetClutch(1);
		float arc = ArcAt(m_Lead.GetOrigin());
		if (arc > m_fLastPilotArc)
			m_fLastPilotArc = arc;
		float lookaheadArc = m_fLastPilotArc + 15.0;
		if (lookaheadArc > m_fTargetArc)
			lookaheadArc = m_fTargetArc;
		vector goal = PointAtArc(lookaheadArc);
		vector facing = m_Lead.GetWorldTransformAxis(2);
		vector toGoal = goal - m_Lead.GetOrigin();
		float distance = vector.DistanceXZ(goal, m_Lead.GetOrigin());
		float steer = 0.0;
		if (distance > 2.0)
		{
			steer = (facing[0] * toGoal[2] - facing[2] * toGoal[0]) / distance * 0.8;
			if (steer > 0.3)
				steer = 0.3;
			if (steer < -0.3)
				steer = -0.3;
		}
		sim.SetSteering(steer);
		sim.SetBreak(0, false);
		sim.SetThrottle(0.28);
		m_iPostframeTicks++;
	}

	protected float TruckSpeed(Vehicle truck)
	{
		CarControllerComponent car = CarControllerComponent.Cast(truck.FindComponent(CarControllerComponent));
		if (!car || !car.GetSimulation())
			return 999.0;
		float speed = car.GetSimulation().GetSpeedKmh();
		if (speed < 0.0)
			speed = -speed;
		return speed;
	}

	protected bool AllAssignedSeated()
	{
		if (m_Drivers.Count() != m_iExpectedTrucks || m_Trucks.Count() != m_iExpectedTrucks)
			return false;
		for (int i = 0; i < m_iExpectedTrucks; i++)
		{
			if (!m_Drivers[i] || !m_Drivers[i].CF_IsActiveConvoyMember() ||
				!m_Drivers[i].CF_IsBoarded() || m_Drivers[i].CF_GetAssignedVehicle() != m_Trucks[i] ||
				m_Drivers[i].CF_IsOrderInverted())
				return false;
		}
		return true;
	}

	protected void UpdatePaths()
	{
		vector lead = m_Lead.GetOrigin();
		m_fLeadPath += vector.DistanceXZ(lead, m_vLastLead);
		if (m_iStage >= 5)
			m_fSecondLeadPath += vector.DistanceXZ(lead, m_vLastLead);
		m_vLastLead = lead;
		for (int i = 0; i < m_Trucks.Count(); i++)
		{
			vector here = m_Trucks[i].GetOrigin();
			float step = vector.DistanceXZ(here, m_LastFollowerPositions[i]);
			m_FollowerPaths[i] = m_FollowerPaths[i] + step;
			if (m_iStage >= 5)
				m_SecondPaths[i] = m_SecondPaths[i] + step;
			m_LastFollowerPositions[i] = here;
		}
	}

	protected bool FollowersAtStop(bool secondLeg)
	{
		IEntity previous = m_Lead;
		for (int i = 0; i < m_Trucks.Count(); i++)
		{
			Vehicle truck = m_Trucks[i];
			float path = m_FollowerPaths[i];
			float displacement = vector.DistanceXZ(m_ArrivalSamples[i], truck.GetOrigin());
			if (secondLeg)
			{
				path = m_SecondPaths[i];
				displacement = vector.DistanceXZ(m_SecondStarts[i], truck.GetOrigin());
			}
			if ((path < 35.0 && !secondLeg) || (path < 20.0 && secondLeg) ||
				(displacement < 35.0 && !secondLeg) || (displacement < 20.0 && secondLeg) ||
				vector.DistanceXZ(truck.GetOrigin(), previous.GetOrigin()) > 30.0 ||
				RoadGap(truck.GetOrigin()) > 6.0 || TruckSpeed(truck) > 2.0)
				return false;
			previous = truck;
		}
		return true;
	}

	protected void LogState(string marker)
	{
		float steering;
		vector leadPosition = m_Lead.GetOrigin();
		CarControllerComponent leadCar = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (leadCar && leadCar.GetSimulation())
		{
			VehicleWheeledSimulation sim = leadCar.GetSimulation();
			steering = sim.GetSteering();
			Print("[ConvoyFollower] SHORT_ARRIVAL_DRIVETRAIN: stage=" + m_iStage +
				" engine=" + sim.EngineIsOn() + " rpm=" + sim.EngineGetRPM() +
				" gear=" + sim.GetGear() + " clutch=" + sim.GetClutch() +
				" throttle=" + sim.GetThrottle() + " brake=" + sim.GetBrake() +
				" handbrake=" + leadCar.GetPersistentHandBrake() + " postframe_ticks=" + m_iPostframeTicks);
		}
		Print("[ConvoyFollower] " + marker + ": stage=" + m_iStage +
			" lead=" + m_Lead.GetOrigin() + " speed=" + TruckSpeed(m_Lead) +
			" road_gap=" + RoadGap(m_Lead.GetOrigin()) + " path=" + m_fLeadPath +
			" second_path=" + m_fSecondLeadPath + " second_displacement=" +
			vector.DistanceXZ(m_vSecondLeadStart, m_Lead.GetOrigin()) +
			" bay_gap=" + vector.DistanceXZ(m_Lead.GetOrigin(), m_vBay) +
			" postframe_ticks=" + m_iPostframeTicks + " steering=" + steering +
			" forward=" + m_Lead.GetWorldTransformAxis(2) + " arc=" + ArcAt(m_Lead.GetOrigin()) +
			" target_arc=" + m_fTargetArc +
			" signed_arc_progress=" + (ArcAt(m_Lead.GetOrigin()) - ArcAt(m_vLeadStart)) +
			" second_arc_progress=" + (ArcAt(m_Lead.GetOrigin()) - ArcAt(m_vSecondLeadStart)) +
			" second_elevation_gain=" + (leadPosition[1] - m_vSecondLeadStart[1]));
		IEntity previous = m_Lead;
		for (int i = 0; i < m_Trucks.Count(); i++)
		{
			Vehicle truck = m_Trucks[i];
			Print("[ConvoyFollower] SHORT_ARRIVAL_UNIT: unit=" + (i + 1) +
				" pos=" + truck.GetOrigin() + " path=" + m_FollowerPaths[i] +
				" second_path=" + m_SecondPaths[i] + " gap=" +
				vector.DistanceXZ(truck.GetOrigin(), previous.GetOrigin()) +
				" road_gap=" + RoadGap(truck.GetOrigin()) + " speed=" + TruckSpeed(truck) +
				" seated=" + m_Drivers[i].CF_IsBoarded() +
				" inverted=" + m_Drivers[i].CF_IsOrderInverted() +
				" second_displacement=" + vector.DistanceXZ(m_SecondStarts[i], truck.GetOrigin()));
			previous = truck;
		}
	}

	protected void Poll()
	{
		if (m_bFinished)
			return;
		m_iTotalSeconds++;
		m_iStageSeconds++;
		if (m_iTotalSeconds >= 270)
		{
			Finish("FAIL total timeout stage=" + m_iStage);
			return;
		}
		if (m_iStage >= 2 && !OwnerIsPilot())
		{
			Finish("FAIL owner left lead pilot seat");
			return;
		}
		if (m_iStage == 0)
		{
			if (!EnsureOwnerPilot())
				return;
			if (!FindSurveyedRoad())
			{
				Finish("FAIL surveyed road81 geometry unavailable");
				return;
			}
			m_Drivers.Clear();
			m_Trucks.Clear();
			m_FollowerPaths.Clear();
			m_SecondPaths.Clear();
			m_LastFollowerPositions.Clear();
			m_ArrivalSamples.Clear();
			m_SecondStarts.Clear();
			for (int i = 1; i <= m_iExpectedTrucks; i++)
			{
				CF_DriverControllerComponent driver = FindDriver(i);
				Vehicle truck = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokeFollower" + i));
				if (!driver || !truck)
					return;
				m_Drivers.Insert(driver);
				m_Trucks.Insert(truck);
				m_FollowerPaths.Insert(0);
				m_SecondPaths.Insert(0);
				m_LastFollowerPositions.Insert(truck.GetOrigin());
				m_ArrivalSamples.Insert(truck.GetOrigin());
				m_SecondStarts.Insert(truck.GetOrigin());
			}
			m_vLeadStart = m_Lead.GetOrigin();
			m_vLastLead = m_vLeadStart;
			m_vArrivalLeadSample = m_vLeadStart;
			SelectTestCamera();
			SetStage(1, "owner seated and surveyed road ready");
		}
		if (m_iStage == 1)
		{
			if (m_iNextOrder < m_iExpectedTrucks)
			{
				if (m_iNextOrder > 0 && !m_Drivers[m_iNextOrder - 1].CF_IsActiveConvoyMember())
					return;
				bool accepted;
				if (m_iNextOrder == 0)
					accepted = CF_ConvoySession.Start(m_Player, m_Drivers[m_iNextOrder]);
				else
					accepted = CF_ConvoySession.Add(m_Player, m_Drivers[m_iNextOrder]);
				Print("[ConvoyFollower] SHORT_ARRIVAL_ORDER: unit=" + (m_iNextOrder + 1) +
					" accepted=" + accepted);
				if (!accepted)
					Finish("FAIL recruit order rejected unit=" + (m_iNextOrder + 1));
				m_iNextOrder++;
				return;
			}
			if (!AllAssignedSeated())
			{
				if (m_iStageSeconds > 60)
					Finish("FAIL drivers did not board assigned trucks");
				return;
			}
			m_fLastPilotArc = ArcAt(m_Lead.GetOrigin());
			m_fTargetArc = m_fBayArc;
			m_bBrake = false;
			m_bDriving = true;
			SetStage(2, "physical owner-piloted approach to bay");
		}
		if (m_iStage == 2)
		{
			UpdatePaths();
			float approachArcProgress = ArcAt(m_Lead.GetOrigin()) - ArcAt(m_vLeadStart);
			if (approachArcProgress < -3.0 || (m_iStageSeconds >= 20 && approachArcProgress < 2.0))
			{
				Finish("FAIL scripted lead made no positive route progress arc=" + approachArcProgress);
				return;
			}
			if (m_iStageSeconds % 5 == 0)
				LogState("SHORT_ARRIVAL_DRIVE");
			if (!AllAssignedSeated())
			{
				Finish("FAIL member left assigned driver seat on approach");
				return;
			}
			if (RoadGap(m_Lead.GetOrigin()) > 10.0)
			{
				Finish("FAIL lead left mapped road on approach");
				return;
			}
			if (m_fLeadPath >= 45.0 && approachArcProgress >= 45.0 && vector.DistanceXZ(m_vLeadStart, m_Lead.GetOrigin()) >= 45.0 &&
				vector.DistanceXZ(m_Lead.GetOrigin(), m_vBay) <= 9.0)
			{
				m_bDriving = false;
				m_bBrake = true;
				ApplyBrake();
				Print("[ConvoyFollower] SHORT_ARRIVAL_LEAD_STOP: physical_path=" + m_fLeadPath +
					" physical_displacement=" + vector.DistanceXZ(m_vLeadStart, m_Lead.GetOrigin()) +
					" goal_gap=" + vector.DistanceXZ(m_Lead.GetOrigin(), m_vBay) +
					" signed_arc_progress=" + approachArcProgress);
				SetStage(3, "lead braked at bay; waiting for followers");
				return;
			}
			if (m_iStageSeconds >= 75)
				Finish("FAIL physical lead did not reach bay");
			return;
		}
		if (m_iStage == 3)
		{
			UpdatePaths();
			if (m_iStageSeconds % 5 == 0)
				LogState("SHORT_ARRIVAL_SETTLING");
			if (!AllAssignedSeated())
			{
				Finish("FAIL member left assigned seat while arriving");
				return;
			}
			if (TruckSpeed(m_Lead) <= 2.0 && RoadGap(m_Lead.GetOrigin()) <= 6.0 &&
				FollowersAtStop(false))
				m_iStableSeconds++;
			else
				m_iStableSeconds = 0;
			if (m_iStableSeconds >= 8)
			{
				Print("[ConvoyFollower] SHORT_ARRIVAL_SETTLED: stable_seconds=" + m_iStableSeconds +
					" lead_path=" + m_fLeadPath);
				bool accepted = CF_ConvoySession.CF_PanelHold(m_Player);
				Print("[ConvoyFollower] SHORT_ARRIVAL_HOLD_ORDER: accepted=" + accepted +
					" state=" + CF_ConvoySession.CF_GetOwnerPanelOrderState(m_Player));
				if (!accepted)
					Finish("FAIL server rejected Hold");
				else
					SetStage(4, "awaiting authoritative Hold completion");
				return;
			}
			if (m_iStageSeconds >= 115)
				Finish("FAIL followers did not settle at short bay");
			return;
		}
		if (m_iStage == 4)
		{
			UpdatePaths();
			string state = CF_ConvoySession.CF_GetOwnerPanelOrderState(m_Player);
			if (state != m_sLastPanelState)
			{
				m_sLastPanelState = state;
				Print("[ConvoyFollower] SHORT_ARRIVAL_HOLD_STATE: " + state);
			}
			if (state == "completed: all trucks holding in vehicles" &&
				AllAssignedSeated() && TruckSpeed(m_Lead) <= 2.0 && FollowersAtStop(false))
			{
				m_iStableSeconds++;
				if (m_iStableSeconds < 5)
					return;
				Print("[ConvoyFollower] SHORT_ARRIVAL_HOLD_COMPLETED: all trucks stationary and seated");
				bool accepted = CF_ConvoySession.CF_PanelResume(m_Player);
				Print("[ConvoyFollower] SHORT_ARRIVAL_RESUME_ORDER: accepted=" + accepted +
					" state=" + CF_ConvoySession.CF_GetOwnerPanelOrderState(m_Player));
				if (!accepted)
				{
					Finish("FAIL server rejected Resume");
					return;
				}
				m_vSecondLeadStart = m_Lead.GetOrigin();
				m_fSecondLeadPath = 0;
				for (int i = 0; i < m_Trucks.Count(); i++)
				{
					m_SecondStarts[i] = m_Trucks[i].GetOrigin();
					m_SecondPaths[i] = 0;
				}
				m_fLastPilotArc = ArcAt(m_Lead.GetOrigin());
				m_fTargetArc = m_fSecondArc;
				m_bBrake = false;
				m_bDriving = true;
				m_sLastPanelState = "";
				SetStage(5, "physically driving second leg after Resume");
				return;
			}
			m_iStableSeconds = 0;
			if (state.IndexOf("blocked:") == 0 || m_iStageSeconds >= 30)
				Finish("FAIL Hold did not complete state=" + state);
			return;
		}
		if (m_iStage == 5)
		{
			UpdatePaths();
			float secondArcProgress = ArcAt(m_Lead.GetOrigin()) - ArcAt(m_vSecondLeadStart);
			if (secondArcProgress < -3.0 || (m_iStageSeconds >= 20 && secondArcProgress < 2.0))
			{
				Finish("FAIL resumed lead made no positive route progress arc=" + secondArcProgress);
				return;
			}
			string state = CF_ConvoySession.CF_GetOwnerPanelOrderState(m_Player);
			if (state != m_sLastPanelState)
			{
				m_sLastPanelState = state;
				Print("[ConvoyFollower] SHORT_ARRIVAL_RESUME_STATE: " + state);
			}
			if (!AllAssignedSeated())
			{
				Finish("FAIL member left assigned seat after Resume");
				return;
			}
			if (m_iStageSeconds % 5 == 0)
				LogState("SHORT_ARRIVAL_SECOND_DRIVE");
			if (RoadGap(m_Lead.GetOrigin()) > 10.0)
			{
				Finish("FAIL lead left mapped road after Resume");
				return;
			}
			if (m_fSecondLeadPath >= 35.0 && secondArcProgress >= 35.0 &&
				vector.DistanceXZ(m_Lead.GetOrigin(), m_vSecondGoal) <= 9.0)
			{
				m_bDriving = false;
				m_bBrake = true;
				ApplyBrake();
				SetStage(6, "second leg lead stopped; waiting for follower proof");
				return;
			}
			if (m_iStageSeconds >= 85)
				Finish("FAIL lead did not physically reach second goal");
			return;
		}
		if (m_iStage == 6)
		{
			UpdatePaths();
			vector settledLeadPosition = m_Lead.GetOrigin();
			string state = CF_ConvoySession.CF_GetOwnerPanelOrderState(m_Player);
			if (state != m_sLastPanelState)
			{
				m_sLastPanelState = state;
				Print("[ConvoyFollower] SHORT_ARRIVAL_RESUME_STATE: " + state);
			}
			if (m_iStageSeconds % 5 == 0)
				LogState("SHORT_ARRIVAL_SECOND_SETTLING");
			if (!AllAssignedSeated())
			{
				Finish("FAIL member left assigned seat on second leg");
				return;
			}
			if (FollowersAtStop(true) && m_fSecondLeadPath >= 35.0 &&
				vector.DistanceXZ(m_vSecondLeadStart, m_Lead.GetOrigin()) >= 30.0 &&
				ArcAt(m_Lead.GetOrigin()) - ArcAt(m_vSecondLeadStart) >= 30.0 &&
				settledLeadPosition[1] - m_vSecondLeadStart[1] >= 1.0 &&
				TruckSpeed(m_Lead) <= 2.0 &&
				m_sLastPanelState == "completed: convoy following")
			{
				Print("[ConvoyFollower] SHORT_ARRIVAL_SECOND_COMPLETE: owner and every assigned truck physically moved and settled");
				Finish("PASS authoritative Hold/Resume and both physical legs");
				return;
			}
			if (m_iStageSeconds >= 90)
				Finish("FAIL followers did not complete second leg");
		}
	}
}
