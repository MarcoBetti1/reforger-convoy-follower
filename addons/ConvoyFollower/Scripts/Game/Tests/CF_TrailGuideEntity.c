// Staged private experiment. This entity has no physics, collision, AI agent,
// visual model or vehicle controls. It is only an owned native target.
class CF_TrailGuideEntityClass : GenericEntityClass
{
}

class CF_TrailGuideEntity : GenericEntity
{
	protected CF_TrailGuideDriverControllerComponent m_Controller;
	protected World m_World;
	protected int m_Epoch;
	protected bool m_Bound;

	void CF_Bind(CF_TrailGuideDriverControllerComponent controller, World world, int epoch)
	{
		if (m_Bound)
			return;
		m_Bound = true;
		m_Controller = controller;
		m_World = world;
		m_Epoch = epoch;
	}

	bool CF_HasLease(CF_EntityFollowWaypoint waypoint)
	{
		if (!m_Bound || !m_Controller || !GetGame() || GetGame().GetWorld() != m_World)
			return false;
		if (GetWorld() != m_World || CF_ConvoySession.CF_IsWorldCleanup())
			return false;
		return m_Controller.CF_HasTrailGuideLease(this, waypoint, m_Epoch);
	}

	void CF_Revoke()
	{
		m_Controller = null;
	}
}

// The original waypoint still admits vehicles only. This separate type admits
// only its exact controller-owned guide, with a live original pilot lease.
class CF_TrailGuideWaypointClass : CF_EntityFollowWaypointClass
{
}

class CF_TrailGuideWaypoint : CF_EntityFollowWaypoint
{
	override SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new CF_TrailGuideWaypointState(groupUtilityComp, this);
	}
}

class CF_TrailGuideWaypointState : SCR_AIWaypointState
{
	protected CF_TrailGuideWaypoint m_EntityWaypoint;

	void CF_TrailGuideWaypointState(notnull SCR_AIGroupUtilityComponent utility, SCR_AIWaypoint waypoint)
	{
		m_EntityWaypoint = CF_TrailGuideWaypoint.Cast(waypoint);
	}

	override void OnSelected()
	{
		super.OnSelected();
		CF_TrailGuideEntity guide;
		if (m_EntityWaypoint)
			guide = CF_TrailGuideEntity.Cast(m_EntityWaypoint.GetEntity());
		if (!guide || !guide.CF_HasLease(m_EntityWaypoint))
		{
			Print("[ConvoyFollower] TRAIL_GUIDE_ADMISSION_FAILED: exact_owned_guide=false");
			return;
		}
		CF_EntityFollowActivity activity = m_EntityWaypoint.CF_CreateBoundActivity(m_Utility);
		if (!activity)
			return;
		m_EntityWaypoint.CF_SetActivity(activity);
		m_Utility.AddAction(activity);
		Print("[ConvoyFollower] TRAIL_GUIDE_ACTIVITY: sequence=" + activity.CF_GetSequence() +
			" activity=" + activity.ToString() + " waypoint_id=" + m_Waypoint.GetID() +
			" native_target_id=" + guide.GetID() + " desired_distance=1 vehicle_planning=true");
	}

	override void OnDeselected()
	{
		if (!CF_ConvoySession.CF_IsWorldCleanup())
			super.OnDeselected();
		if (m_EntityWaypoint)
			m_EntityWaypoint.CF_SetActivity(null);
	}
}

// Read-only access to retained actual samples. Query remains the unchanged
// production geometry implementation and alone decides whether entry joined.
class CF_TrailGuideRoute : CF_DrivenRoute
{
	// On two adjacent legs with nonnegative on-route distances a,b,
	// (a+b)/chord <= sec(turn/2). This does not exempt inside-corner
	// projection jumps from Query's ADVANCE_LIMIT check.
	bool PhysicalBudget(float measured, out float allowed, out float turnCos, out float factor, out string reason)
	{
		turnCos = 1;
		factor = 1;
		allowed = Math.Max(0.001, measured);
		if (!m_Joined || m_Segment >= m_Points.Count() - 2)
			return true;
		vector first = m_Points[m_Segment + 1] - m_Points[m_Segment];
		vector second = m_Points[m_Segment + 2] - m_Points[m_Segment + 1];
		first[1] = 0;
		second[1] = 0;
		first.Normalize();
		second.Normalize();
		turnCos = Math.Clamp(vector.Dot(first, second), -1.0, 1.0);
		if (turnCos < 0)
		{
			reason = "local_turn_over_90_degrees";
			return false;
		}
		factor = Math.Sqrt(2.0 / (1.0 + turnCos));
		allowed = Math.Max(0.001, measured * factor);
		// A budget capable of reaching the second future vertex is outside
		// this single-vertex derivation. Never grant catch-up across it.
		if (m_Segment < m_Points.Count() - 3 && m_Progress + allowed > m_Stations[m_Segment + 2] + 0.001)
		{
			reason = "measured_step_spans_multiple_vertices";
			return false;
		}
		return true;
	}

	bool ReadEntry(out vector start, out vector tangent, out float firstLength, out float endStation)
	{
		if (m_Points.Count() < 2 || m_HistoryLost || m_Discontinuous)
			return false;
		start = m_Points[0];
		tangent = m_Points[1] - start;
		tangent[1] = 0;
		firstLength = tangent.Length();
		if (firstLength <= 0)
			return false;
		tangent.Normalize();
		endStation = m_Stations[m_Stations.Count() - 1];
		return true;
	}

	bool ReadRecorded(float station, out vector position, out vector tangent)
	{
		if (m_Points.Count() < 2 || m_HistoryLost || m_Discontinuous)
			return false;
		if (station < m_Stations[0] || station > m_Stations[m_Stations.Count() - 1])
			return false;
		SampleAt(station, position, tangent);
		return true;
	}
}
