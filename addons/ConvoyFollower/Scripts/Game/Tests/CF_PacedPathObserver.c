// Test-only native path reads. No native action/component is retained or changed.
// GetCurrentPath does not document ordering, coordinate space or planning horizon:
// first/last/nearest below describe the returned array, not a steering target.
class CF_PacedPathObserver
{
	protected ref array<vector> m_GroupPoints = {};
	protected ref array<vector> m_CarPoints = {};
	protected int m_iRead;
	protected int m_iRawRecords;
	protected bool m_bRawExhausted;
	protected bool m_bFinished;
	protected float m_fLastReadMs;
	protected float m_fLastSummaryMs = -1000;
	protected string m_sWaypoint = "none";
	protected vector m_vWaypoint;
	protected string m_sBehavior = "none";
	protected string m_sActivity = "none";
	protected string m_sNavlink = "none";
	protected string m_sResolution;
	protected bool m_bBraking;
	protected bool m_bReverse;
	protected static const int RAW_POINT_LIMIT = 128;
	protected static const int RAW_RECORD_LIMIT = 3200;

	protected string EntityKey(IEntity entity)
	{
		if (!entity)
			return "none";
		return entity.GetID().ToString();
	}

	protected string ActionKey(AIActionBase action)
	{
		if (!action)
			return "none";
		return action.Type().ToString() + ":" + action.GetActionState() + ":" + action.ToString();
	}

	protected bool OwnsTruck(AICarMovementComponent movement, Vehicle truck)
	{
		return movement && movement.GetAIAgent() && movement.GetAIAgent().GetControlledEntity() == truck;
	}

	protected AICarMovementComponent FromAgent(AIAgent agent)
	{
		if (!agent)
			return null;
		return AICarMovementComponent.Cast(agent.GetMovementComponent());
	}

	// Same exact-truck resolution as CF_NativeCruiseControl, without Request/Reset.
	protected AICarMovementComponent ResolveCar(Vehicle truck, ChimeraCharacter pilot, out string resolver)
	{
		resolver = "none";
		AICarMovementComponent movement = AICarMovementComponent.Cast(truck.FindComponent(AICarMovementComponent));
		if (OwnsTruck(movement, truck))
		{
			resolver = "truck.component";
			return movement;
		}
		AIControlComponent control = AIControlComponent.Cast(truck.FindComponent(AIControlComponent));
		if (control)
		{
			movement = FromAgent(control.GetAIAgent());
			if (OwnsTruck(movement, truck))
			{
				resolver = "truck.agent";
				return movement;
			}
			movement = FromAgent(control.GetControlAIAgent());
			if (OwnsTruck(movement, truck))
			{
				resolver = "truck.control_agent";
				return movement;
			}
		}
		control = pilot.GetAIControlComponent();
		if (control)
		{
			movement = FromAgent(control.GetControlAIAgent());
			if (OwnsTruck(movement, truck))
			{
				resolver = "pilot.control_agent";
				return movement;
			}
			movement = FromAgent(control.GetAIAgent());
			if (OwnsTruck(movement, truck))
			{
				resolver = "pilot.agent";
				return movement;
			}
		}
		return null;
	}

	protected int NearestIndex(array<vector> points, vector origin)
	{
		int nearest = -1;
		float nearestSq;
		for (int index = 0; index < points.Count(); index++)
		{
			float distanceSq = vector.DistanceSq(origin, points[index]);
			if (nearest < 0 || distanceSq < nearestSq)
			{
				nearest = index;
				nearestSq = distanceSq;
			}
		}
		return nearest;
	}

	protected void LogPath(string prefix, string kind, bool resolved, array<vector> points, vector origin)
	{
		int count = points.Count();
		int nearest = NearestIndex(points, origin);
		string geometry = " empty=true";
		if (count > 0)
		{
			geometry = " empty=false first=" + points[0] + " last=" + points[count - 1] +
				" nearest_index=" + nearest + " nearest=" + points[nearest] +
				" nearest_distance_m=" + vector.Distance(origin, points[nearest]);
			if (nearest > 0)
				geometry += " adjacent_before_index=" + (nearest - 1) + " adjacent_before=" + points[nearest - 1];
			if (nearest + 1 < count)
				geometry += " adjacent_after_index=" + (nearest + 1) + " adjacent_after=" + points[nearest + 1];
		}
		Print("[ConvoyFollower] PACED_PATH_ARRAY: " + prefix + " path=" + kind + " resolved=" + resolved +
			" count=" + count + geometry + " coordinates_and_order=unverified_returned_array");
	}

	protected void LogRaw(string prefix, string kind, array<vector> points, vector origin)
	{
		if (m_bRawExhausted)
			return;
		// Reserve an entire header + at most eight chunks. Summary reads continue
		// after this independent raw-output budget is exhausted.
		if (m_iRawRecords + 9 > RAW_RECORD_LIMIT)
		{
			m_bRawExhausted = true;
			Print("[ConvoyFollower] PACED_PATH_RAW_EXHAUSTED: " + prefix + " records=" + m_iRawRecords +
				" limit=" + RAW_RECORD_LIMIT + " summaries_continue=true physical_gates_unchanged=true");
			return;
		}
		array<int> indices = {};
		int count = points.Count();
		int nearest = NearestIndex(points, origin);
		// On long paths retain both ends and the nearest returned-point neighborhood.
		// Explicit indices preserve the fact that any intervening points were skipped.
		for (int index = 0; index < count; index++)
		{
			if (count <= RAW_POINT_LIMIT || index < 60 || index >= count - 60 || Math.AbsInt(index - nearest) <= 3)
				indices.Insert(index);
		}
		Print("[ConvoyFollower] PACED_PATH_RAW: " + prefix + " path=" + kind + " count=" + count +
			" logged_count=" + indices.Count() + " point_limit=" + RAW_POINT_LIMIT +
			" truncated=" + (indices.Count() < count) + " index_order=returned_array");
		m_iRawRecords++;
		string chunk;
		int chunkPoints;
		int chunkNumber;
		foreach (int pointIndex : indices)
		{
			chunk += " point_" + pointIndex + "=" + points[pointIndex];
			chunkPoints++;
			if (chunkPoints == 16)
			{
				Print("[ConvoyFollower] PACED_PATH_POINTS: " + prefix + " path=" + kind + " chunk=" + chunkNumber + chunk);
				m_iRawRecords++;
				chunkNumber++;
				chunkPoints = 0;
				chunk = string.Empty;
			}
		}
		if (chunkPoints > 0)
		{
			Print("[ConvoyFollower] PACED_PATH_POINTS: " + prefix + " path=" + kind + " chunk=" + chunkNumber + chunk);
			m_iRawRecords++;
		}
	}

	void Finish(string runId, int unit, float driveSeconds)
	{
		if (m_bFinished)
			return;
		m_bFinished = true;
		m_GroupPoints.Clear();
		m_CarPoints.Clear();
		Print("[ConvoyFollower] PACED_PATH_END: run_id=" + runId + " unit=" + unit + " drive_seconds=" + driveSeconds +
			" reads=" + m_iRead + " raw_records=" + m_iRawRecords + " raw_exhausted=" + m_bRawExhausted +
			" reason=120s_window_complete native_actions_written=false");
	}

	// Called no faster than every 200 ms from the existing POSTFRAME observer,
	// only during the first 120 s after actual PACED_DRIVE_BEGIN.
	void Observe(Vehicle truck, ChimeraCharacter pilot, SCR_AIGroup group, bool identityRetained,
		string runId, int unit, int tick, float fixtureSeconds, float driveMs)
	{
		if (m_bFinished || !GetGame() || !GetGame().GetWorld() || CF_ConvoySession.CF_IsWorldCleanup())
			return;
		m_iRead++;
		float readDt = driveMs - m_fLastReadMs;
		m_fLastReadMs = driveMs;
		string prefix = "run_id=" + runId + " unit=" + unit + " tick=" + tick + " read=" + m_iRead +
			" seconds=" + fixtureSeconds + " drive_seconds=" + (driveMs / 1000.0) + " read_dt_ms=" + readDt;
		m_GroupPoints.Clear();
		m_CarPoints.Clear();
		AIAgent pilotAgent;
		AIControlComponent pilotControl;
		if (pilot)
			pilotControl = pilot.GetAIControlComponent();
		if (pilotControl)
			pilotAgent = pilotControl.GetAIAgent();
		bool identityValid = identityRetained && truck && pilot && group && pilotAgent &&
			pilotAgent.GetControlledEntity() == pilot && pilotAgent.GetParentGroup() == group;
		AIGroupMovementComponent groupMovement;
		AICarMovementComponent carMovement;
		AIAgent carAgent;
		string resolver = "none";
		int pilotHandler = -1;
		SCR_AIUtilityComponent pilotUtility;
		SCR_AIGroupUtilityComponent groupUtility;
		if (identityValid)
		{
			groupMovement = AIGroupMovementComponent.Cast(group.GetMovementComponent());
			if (groupMovement && groupMovement.GetAIAgent() != group)
				groupMovement = null;
			if (groupMovement)
			{
				groupMovement.GetCurrentPath(m_GroupPoints);
				pilotHandler = groupMovement.GetAgentMoveHandlerId(pilotAgent);
			}
			carMovement = ResolveCar(truck, pilot, resolver);
			if (carMovement)
			{
				carAgent = carMovement.GetAIAgent();
				carMovement.GetCurrentPath(m_CarPoints);
			}
			pilotUtility = SCR_AIUtilityComponent.Cast(pilotAgent.FindComponent(SCR_AIUtilityComponent));
			groupUtility = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));
		}
		AIActionBase behavior;
		AIActionBase activity;
		AIWaypoint waypoint;
		IEntity navlink;
		if (pilotUtility)
			behavior = pilotUtility.GetCurrentBehavior();
		if (groupUtility)
			activity = groupUtility.GetCurrentAction();
		if (identityValid)
			waypoint = group.GetCurrentWaypoint();
		if (carMovement)
			navlink = carMovement.GetLastNavlinkEntity();
		vector waypointOrigin;
		if (waypoint)
			waypointOrigin = waypoint.GetOrigin();
		string waypointKey = EntityKey(waypoint);
		string behaviorKey = ActionKey(behavior);
		string activityKey = ActionKey(activity);
		string navlinkKey = EntityKey(navlink);
		string resolution = "identity=" + identityValid + ",group=" + (groupMovement != null) + ",car=" + resolver;
		CarControllerComponent car;
		VehicleWheeledSimulation simulation;
		vector origin;
		vector forward;
		vector velocity;
		if (truck)
		{
			origin = truck.GetOrigin();
			forward = truck.GetWorldTransformAxis(2);
			if (truck.GetPhysics())
				velocity = truck.GetPhysics().GetVelocity();
			car = CarControllerComponent.Cast(truck.FindComponent(CarControllerComponent));
		}
		if (car)
			simulation = car.GetSimulation();
		bool braking = simulation && simulation.GetBrake() > 0.5;
		bool reverse = simulation && simulation.GetGear() == 0;
		string reasons;
		if (m_iRead == 1)
			reasons += "initial,";
		if (resolution != m_sResolution)
			reasons += "resolution,";
		if (waypointKey != m_sWaypoint || vector.DistanceSq(waypointOrigin, m_vWaypoint) > 0.0625)
			reasons += "waypoint,";
		if (behaviorKey != m_sBehavior || activityKey != m_sActivity)
			reasons += "action,";
		if (navlinkKey != m_sNavlink)
			reasons += "navlink,";
		if (braking != m_bBraking)
			reasons += "brake,";
		if (reverse != m_bReverse)
			reasons += "reverse_gear,";
		bool transition = !reasons.IsEmpty();
		m_sResolution = resolution;
		m_sWaypoint = waypointKey;
		m_vWaypoint = waypointOrigin;
		m_sBehavior = behaviorKey;
		m_sActivity = activityKey;
		m_sNavlink = navlinkKey;
		m_bBraking = braking;
		m_bReverse = reverse;
		if (!transition && driveMs - m_fLastSummaryMs < 1000)
			return;
		m_fLastSummaryMs = driveMs;
		if (!transition)
			reasons = "periodic";
		string controls = " simulation=false";
		if (simulation)
			controls = " simulation=true speed_kmh=" + simulation.GetSpeedKmh() + " throttle=" + simulation.GetThrottle() +
				" brake=" + simulation.GetBrake() + " gear=" + simulation.GetGear() + " engine=" + simulation.EngineIsOn();
		string waypointFields = " waypoint_present=false";
		if (waypoint)
			waypointFields = " waypoint_present=true waypoint_origin=" + waypointOrigin + " waypoint_radius=" + waypoint.GetCompletionRadius();
		SCR_AIMoveActivity move = SCR_AIMoveActivity.Cast(activity);
		string moveFields = " move_activity=false";
		IEntity activityEntity;
		if (move)
		{
			moveFields = " move_activity=true";
			if (move.m_vPosition)
				moveFields += " activity_position_assigned=" + move.m_vPosition.m_AssignedOut + " activity_position=" + move.m_vPosition.m_Value;
			if (move.m_Entity)
				activityEntity = move.m_Entity.m_Value;
			moveFields += " activity_entity_id=" + EntityKey(activityEntity) + " activity_entity_is_waypoint=" + (activityEntity && activityEntity == waypoint);
			if (activityEntity)
				moveFields += " activity_entity_type=" + activityEntity.Type().ToString() + " activity_entity_origin=" + activityEntity.GetOrigin();
		}
		string navlinkFields;
		if (navlink)
			navlinkFields = " last_navlink_type=" + navlink.Type().ToString() + " last_navlink_name=" + navlink.GetName() + " last_navlink_origin=" + navlink.GetOrigin();
		Print("[ConvoyFollower] PACED_PATH_SAMPLE: " + prefix + " phase=POSTFRAME reasons=" + reasons +
			" identity_retained=" + identityRetained + " identity_valid=" + identityValid + " truck_id=" + EntityKey(truck) +
			" pilot_id=" + EntityKey(pilot) + " group_id=" + EntityKey(group) + " pilot_agent_id=" + EntityKey(pilotAgent) +
			" pilot_handler=" + pilotHandler + " group_path_handler=default_api_only car_agent_id=" + EntityKey(carAgent) +
			" car_resolver=" + resolver + " origin=" + origin + " forward=" + forward + " velocity_world=" + velocity + controls +
			" behavior=" + behaviorKey + " group_activity=" + activityKey + " waypoint_id=" + waypointKey + waypointFields +
			moveFields + " last_navlink_id=" + navlinkKey + navlinkFields + " native_actions_written=false");
		LogPath(prefix, "group", groupMovement != null, m_GroupPoints, origin);
		LogPath(prefix, "car", carMovement != null, m_CarPoints, origin);
		if (transition)
		{
			LogRaw(prefix, "group", m_GroupPoints, origin);
			LogRaw(prefix, "car", m_CarPoints, origin);
		}
	}
}
