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

// Read-only access to retained actual samples. Ordinary Query keeps its original
// budget. QueryPhysical explicitly opts into one adjacent station transfer.
class CF_TrailGuideRoute : CF_DrivenRoute
{
	// Value-only, one-query context. Ordinary Query never arms the hook.
	protected bool m_PhysicalQueryActive;
	protected float m_PhysicalMeasured;
	protected bool m_PreviousPhysicalQuery;
	protected int m_PreviousTargetKey;
	protected int m_PreviousSegment;
	protected float m_PreviousProgress;
	protected vector m_PreviousPose;
	protected vector m_PreviousA;
	protected vector m_PreviousB;
	protected vector m_PreviousNext;
	protected float m_PreviousStationA;
	protected float m_PreviousStationB;
	protected float m_PreviousStationNext;

	protected bool FiniteValue(float value)
	{
		// NaN and either infinity fail; no new distance threshold is introduced.
		return value - value == 0;
	}

	protected bool FinitePose(vector pose)
	{
		return FiniteValue(pose[0]) && FiniteValue(pose[1]) && FiniteValue(pose[2]);
	}

	override void Reset(int targetKey, vector actualPredecessorPosition)
	{
		m_PhysicalQueryActive = false;
		m_PreviousPhysicalQuery = false;
		super.Reset(targetKey, actualPredecessorPosition);
	}

	// Direct callers cannot leave a stale accepted physical snapshot behind.
	override void Query(int targetKey, vector followerPosition, float lookAhead, float spacing,
		float maxAdvance, float maxCrossTrack, CF_DrivenRouteGuidance result)
	{
		m_PhysicalQueryActive = false;
		m_PreviousPhysicalQuery = false;
		super.Query(targetKey, followerPosition, lookAhead, spacing, maxAdvance, maxCrossTrack, result);
	}

	bool QueryPhysical(int targetKey, vector pose, float measured, float lookAhead, float spacing,
		float maxCrossTrack, out float allowed, out float turnCos, out float factor,
		out string reason, CF_DrivenRouteGuidance result)
	{
		m_PhysicalQueryActive = false;
		allowed = 0;
		turnCos = 1;
		factor = 1;
		reason = "";
		if (!result || !FiniteValue(measured) || measured < 0 || !FinitePose(pose) ||
			!FiniteValue(lookAhead) || !FiniteValue(spacing) || !FiniteValue(maxCrossTrack))
		{
			m_PreviousPhysicalQuery = false;
			reason = "invalid_physical_query_measurement";
			return false;
		}
		if (!PhysicalBudget(measured, allowed, turnCos, factor, reason))
		{
			m_PreviousPhysicalQuery = false;
			return false;
		}
		if (!FiniteValue(allowed) || !FiniteValue(turnCos) || !FiniteValue(factor))
		{
			m_PreviousPhysicalQuery = false;
			reason = "invalid_physical_query_budget";
			return false;
		}
		m_PhysicalMeasured = measured;
		m_PhysicalQueryActive = true;
		bool hadPrevious = m_PreviousPhysicalQuery;
		vector priorPose = m_PreviousPose;
		vector priorStart = m_PreviousA;
		// Deliberately bypass the ordinary-Query override, which disarms context.
		super.Query(targetKey, pose, lookAhead, spacing, allowed, maxCrossTrack, result);
		result.HasPreviousPhysicalQuery = hadPrevious;
		result.PreviousPhysicalPose = priorPose;
		result.PreviousIncomingStart = priorStart;
		m_PhysicalQueryActive = false;
		m_PhysicalMeasured = 0;
		m_PreviousPhysicalQuery = false;
		if ((result.State == TRACKING || result.State == SPACING_HOLD) &&
			m_Joined && targetKey == m_TargetKey && m_Segment < m_Points.Count() - 2)
		{
			m_PreviousTargetKey = targetKey;
			m_PreviousSegment = m_Segment;
			m_PreviousProgress = m_Progress;
			m_PreviousPose = pose;
			m_PreviousA = m_Points[m_Segment];
			m_PreviousB = m_Points[m_Segment + 1];
			m_PreviousNext = m_Points[m_Segment + 2];
			m_PreviousStationA = m_Stations[m_Segment];
			m_PreviousStationB = m_Stations[m_Segment + 1];
			m_PreviousStationNext = m_Stations[m_Segment + 2];
			m_PreviousPhysicalQuery = true;
		}
		return true;
	}

	protected bool HasAdjacentProjectionContext(int targetKey)
	{
		if (!m_PhysicalQueryActive || !m_PreviousPhysicalQuery || !m_Joined ||
			m_PreviousTargetKey != targetKey || targetKey != m_TargetKey ||
			m_PreviousSegment != m_Segment || m_PreviousProgress != m_Progress ||
			m_Segment >= m_Points.Count() - 2)
			return false;
		if (m_PreviousA != m_Points[m_Segment] || m_PreviousB != m_Points[m_Segment + 1] ||
			m_PreviousNext != m_Points[m_Segment + 2] ||
			m_PreviousStationA != m_Stations[m_Segment] || m_PreviousStationB != m_Stations[m_Segment + 1] ||
			m_PreviousStationNext != m_Stations[m_Segment + 2])
			return false;
		if (!FinitePose(m_PreviousA) || !FinitePose(m_PreviousB) || !FinitePose(m_PreviousNext) ||
			!FiniteValue(m_PreviousStationA) || !FiniteValue(m_PreviousStationB) || !FiniteValue(m_PreviousStationNext))
			return false;
		return true;
	}

	override protected bool CanInspectRoundedAdjacent(int targetKey, int segment)
	{
		return segment == m_Segment && HasAdjacentProjectionContext(targetKey);
	}

	override protected bool AdmitAdjacentProjection(int targetKey, vector pose, int segment,
		float candidate, vector projected, bool nextSegment, float maxCrossTrack,
		CF_DrivenRouteGuidance result)
	{
		if (!HasAdjacentProjectionContext(targetKey) || segment != m_Segment + 1 || nextSegment)
			return false;
		vector delta = pose - m_PreviousPose;
		delta[1] = 0;
		float chord = delta.Length();
		if (!FiniteValue(chord) || m_PhysicalMeasured <= 0 || chord <= 0 || chord > m_PhysicalMeasured + 0.001)
			return false;
		vector vertex = m_Points[m_Segment + 1];
		vector incoming = vertex - m_Points[m_Segment];
		vector outgoing = m_Points[m_Segment + 2] - vertex;
		incoming[1] = 0;
		outgoing[1] = 0;
		if (incoming.LengthSq() <= 0 || outgoing.LengthSq() <= 0)
			return false;
		incoming.Normalize();
		outgoing.Normalize();
		float turn = Math.Clamp(vector.Dot(incoming, outgoing), -1.0, 1.0);
		if (turn < 0)
			return false;
		float before = m_Progress - m_Stations[m_Segment + 1];
		float after = candidate - m_Stations[m_Segment + 1];
		if (before > 0.001 || after < 0 || candidate >= m_Stations[m_Segment + 2] - 0.001)
			return false;
		vector oldCursor = vertex + incoming * before;
		vector newCursor = vertex + outgoing * after;
		if (vector.DistanceXZ(newCursor, projected) > 0.001)
			return false;
		vector oldResidual = m_PreviousPose - oldCursor;
		vector newResidual = pose - newCursor;
		oldResidual[1] = 0;
		newResidual[1] = 0;
		if (Math.AbsFloat(vector.Dot(oldResidual, incoming)) > 0.001 || oldResidual.Length() > maxCrossTrack)
			return false;
		float cosineHalf = Math.Sqrt((1.0 + turn) * 0.5);
		vector bisector = (incoming + outgoing) / (2.0 * cosineHalf);
		if (vector.Dot(bisector, delta) <= 0)
			return false;
		float physicalBound = m_PhysicalMeasured / cosineHalf;
		float correction = -vector.Dot(bisector, newResidual - oldResidual) / cosineHalf;
		float advance = candidate - m_Progress;
		if (!FiniteValue(physicalBound) || !FiniteValue(correction) || !FiniteValue(advance) ||
			!FiniteValue(physicalBound + correction) || advance > physicalBound + correction + 0.001)
			return false;
		result.HasProjectionCorrection = true;
		result.ProjectionPhysicalBound = physicalBound;
		result.ProjectionSignedCorrection = correction;
		result.ProjectionCandidateAdvance = advance;
		// A second candidate/query cannot reuse this proof before a committed query.
		m_PreviousPhysicalQuery = false;
		return true;
	}

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
