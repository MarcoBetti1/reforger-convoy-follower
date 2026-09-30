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
// budget. QueryPhysical also admits a proven, bounded connected local chain.
class CF_TrailGuideRoute : CF_DrivenRoute
{
	// Value-only, one-query context. Ordinary Query never arms the hook.
	protected bool m_PhysicalQueryActive;
	protected bool m_PhysicalAllowReverse;
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
	protected bool m_PreviousHasBehind;
	protected vector m_PreviousBehind;
	protected float m_PreviousStationBehind;
	protected bool m_PreviousHasNext;
	// At most four connected legs, captured only after a successful query.
	// Absolute point/station identity remains valid across harmless pruning.
	protected ref array<vector> m_PreviousChainPoints = {};
	protected ref array<float> m_PreviousChainStations = {};
	protected static const int CF_LOCAL_CHAIN_LEGS = 4;

	protected bool FiniteValue(float value)
	{
		// NaN and either infinity fail; no new distance threshold is introduced.
		return value - value == 0;
	}

	protected bool FinitePose(vector pose)
	{
		return FiniteValue(pose[0]) && FiniteValue(pose[1]) && FiniteValue(pose[2]);
	}

	void InvalidatePhysicalQuery()
	{
		m_PhysicalQueryActive = false;
		m_PhysicalAllowReverse = false;
		m_PreviousPhysicalQuery = false;
	}

	protected void RememberPhysicalQuery(int targetKey, vector pose)
	{
		m_PreviousTargetKey = targetKey;
		m_PreviousSegment = m_Segment;
		m_PreviousProgress = m_Progress;
		m_PreviousPose = pose;
		m_PreviousA = m_Points[m_Segment];
		m_PreviousB = m_Points[m_Segment + 1];
		m_PreviousStationA = m_Stations[m_Segment];
		m_PreviousStationB = m_Stations[m_Segment + 1];
		m_PreviousHasBehind = m_Segment > 0;
		if (m_PreviousHasBehind)
		{
			m_PreviousBehind = m_Points[m_Segment - 1];
			m_PreviousStationBehind = m_Stations[m_Segment - 1];
		}
		m_PreviousHasNext = m_Segment < m_Points.Count() - 2;
		if (m_PreviousHasNext)
		{
			m_PreviousNext = m_Points[m_Segment + 2];
			m_PreviousStationNext = m_Stations[m_Segment + 2];
		}
		m_PreviousChainPoints.Clear();
		m_PreviousChainStations.Clear();
		for (int local = 0; local <= CF_LOCAL_CHAIN_LEGS && m_Segment + local < m_Points.Count(); local++)
		{
			m_PreviousChainPoints.Insert(m_Points[m_Segment + local]);
			m_PreviousChainStations.Insert(m_Stations[m_Segment + local]);
		}
		m_PreviousPhysicalQuery = true;
	}

	protected bool HasLocalPhysicalContext(int targetKey)
	{
		if (!m_PreviousPhysicalQuery || !m_Joined || targetKey != m_TargetKey) return false;
		if (m_PreviousTargetKey != targetKey || m_PreviousProgress != m_Progress) return false;
		if (m_Segment < 0 || m_Segment >= m_Points.Count() - 1) return false;
		// Absolute stations and points survive harmless pruning/index shifts.
		if (m_PreviousA != m_Points[m_Segment] || m_PreviousB != m_Points[m_Segment + 1]) return false;
		if (m_PreviousStationA != m_Stations[m_Segment] || m_PreviousStationB != m_Stations[m_Segment + 1]) return false;
		return FinitePose(m_PreviousPose) && FinitePose(m_PreviousA) && FinitePose(m_PreviousB);
	}

	// 0: ordinary forward query; 1: proven local rewind; -1: rejected rewind.
	// No nearest-history search: a reverse step may cross only the prior vertex.
	protected int TryReversePhysical(int targetKey, vector pose, float measured,
		float lookAhead, float spacing, float maxCrossTrack, out float allowed,
		out float turnCos, out float factor, out string reason, CF_DrivenRouteGuidance result)
	{
		if (!m_Initialized || !m_Joined || targetKey != m_TargetKey) return 0;
		if (m_HistoryLost || m_Discontinuous || m_Points.Count() < 2) return 0;
		if (lookAhead <= 0 || spacing < 0 || maxCrossTrack <= 0) return 0;
		vector a = m_Points[m_Segment];
		vector b = m_Points[m_Segment + 1];
		float length = m_Stations[m_Segment + 1] - m_Stations[m_Segment];
		vector tangent = b - a;
		tangent[1] = 0;
		tangent = tangent / length;
		float raw = vector.Dot(pose - a, tangent) / length;
		float candidate = m_Stations[m_Segment] + Math.Clamp(raw, 0, 1) * length;
		vector projected = a + (b - a) * Math.Clamp(raw, 0, 1);
		float crossTrack = vector.DistanceXZ(pose, projected);
		int segment = m_Segment;
		vector previousTangent;
		float previousRaw = -1;
		if (m_Segment > 0 && raw * length <= maxCrossTrack)
		{
			vector behind = m_Points[m_Segment - 1];
			float previousLength = m_Stations[m_Segment] - m_Stations[m_Segment - 1];
			previousTangent = a - behind;
			previousTangent[1] = 0;
			previousTangent = previousTangent / previousLength;
			previousRaw = vector.Dot(pose - behind, previousTangent) / previousLength;
			if (previousRaw >= 0 && previousRaw <= 1)
			{
				vector previousProjection = behind + (a - behind) * previousRaw;
				float previousError = vector.DistanceXZ(pose, previousProjection);
				if (previousError + 0.001 < crossTrack)
				{
					segment--;
					candidate = m_Stations[segment] + previousRaw * previousLength;
					projected = previousProjection;
					crossTrack = previousError;
				}
			}
		}
		// A clamped vertex pose can have raw < 0 without station retreat.
		// Leave it to ordinary Query's unchanged corridor and advance guards.
		if (candidate >= m_Progress - 0.001) return 0;
		ClearGuidance(result);
		result.State = ADVANCE_LIMIT;
		result.RecordedEnd = m_Stations[m_Stations.Count() - 1];
		result.CrossTrack = crossTrack;
		reason = "reverse_physical_context_unavailable";
		if (!HasLocalPhysicalContext(targetKey)) return -1;
		// Without a selected previous segment, a negative raw projection is
		// clamped to this retained segment's start. The remaining physical and
		// corridor proof may admit that endpoint, never an earlier segment.
		if (segment < m_Segment)
		{
			reason = "reverse_previous_history_changed";
			if (!m_PreviousHasBehind || m_PreviousBehind != m_Points[segment]) return -1;
			if (m_PreviousStationBehind != m_Stations[segment]) return -1;
		}
		vector delta = pose - m_PreviousPose;
		delta[1] = 0;
		float chord = delta.Length();
		reason = "reverse_motion_unproved";
		if (measured <= 0 || chord <= 0 || !FiniteValue(chord)) return -1;
		if (chord > measured + 0.001) return -1;
		vector oldCursor = a + (b - a) * ((m_Progress - m_Stations[m_Segment]) / length);
		vector oldResidual = m_PreviousPose - oldCursor;
		vector newResidual = pose - projected;
		oldResidual[1] = 0;
		newResidual[1] = 0;
		reason = "reverse_corridor_lost";
		if (crossTrack > maxCrossTrack || oldResidual.Length() > maxCrossTrack)
		{
			Print("[ConvoyFollower] TRAIL_REVERSE_CORRIDOR_REJECTED: epoch=" + targetKey +
				" segment=" + m_Segment + " candidate_segment=" + segment + " previous_pose=" + m_PreviousPose +
				" pose=" + pose + " segment_start=" + a + " segment_end=" + b +
				" progress=" + m_Progress + " candidate=" + candidate + " previous_residual_m=" + oldResidual.Length() +
				" cross_track_m=" + crossTrack + " limit_m=" + maxCrossTrack + " measured_m=" + measured +
				" history_changed=false control_writes=false");
			result.State = OFF_ROUTE;
			return -1;
		}
		// A successfully retained clamped endpoint can have a longitudinal
		// residual. The signed residual correction below accounts for it;
		// orthogonality is not required for actual reverse motion proof.
		vector reverseAxis = -tangent;
		turnCos = 1;
		factor = 1;
		if (segment < m_Segment)
		{
			turnCos = Math.Clamp(vector.Dot(tangent, previousTangent), -1.0, 1.0);
			reason = "reverse_local_turn_over_90_degrees";
			if (turnCos < 0) return -1;
			float cosineHalf = Math.Sqrt((1.0 + turnCos) * 0.5);
			factor = 1.0 / cosineHalf;
			reverseAxis = -(tangent + previousTangent) / (2.0 * cosineHalf);
		}
		float signedMotion = vector.Dot(reverseAxis, delta);
		float correction = -vector.Dot(reverseAxis, newResidual - oldResidual) * factor;
		float retreat = m_Progress - candidate;
		allowed = measured * factor;
		reason = "reverse_signed_motion_or_budget_rejected";
		if (!FiniteValue(signedMotion) || !FiniteValue(correction) || !FiniteValue(retreat)) return -1;
		if (!FiniteValue(allowed) || signedMotion <= 0 || retreat <= 0) return -1;
		if (retreat > signedMotion * factor + correction + 0.001) return -1;
		if (retreat > allowed + correction + 0.001) return -1;
		// The entire proof has passed. Neither a rejected query nor a stale
		// snapshot can consume history or add a reverse certificate.
		m_Segment = segment;
		m_Progress = candidate;
		m_ReverseProofSerial++;
		m_ReverseStationTotal += retreat;
		FillGuidance(lookAhead, spacing, result);
		result.Reversing = true;
		result.HasPreviousPhysicalQuery = true;
		result.PreviousPhysicalPose = m_PreviousPose;
		result.PreviousIncomingStart = m_PreviousA;
		RememberPhysicalQuery(targetKey, pose);
		reason = "";
		return 1;
	}

	override void Reset(int targetKey, vector actualPredecessorPosition)
	{
		InvalidatePhysicalQuery();
		super.Reset(targetKey, actualPredecessorPosition);
	}

	// Direct callers cannot leave a stale accepted physical snapshot behind.
	override void Query(int targetKey, vector followerPosition, float lookAhead, float spacing,
		float maxAdvance, float maxCrossTrack, CF_DrivenRouteGuidance result)
	{
		InvalidatePhysicalQuery();
		super.Query(targetKey, followerPosition, lookAhead, spacing, maxAdvance, maxCrossTrack, result);
	}

	bool QueryPhysical(int targetKey, vector pose, float measured, float lookAhead, float spacing,
		float maxCrossTrack, out float allowed, out float turnCos, out float factor,
		out string reason, CF_DrivenRouteGuidance result, bool allowReverse = false)
	{
		m_PhysicalQueryActive = false;
		m_PhysicalAllowReverse = false;
		allowed = 0;
		turnCos = 1;
		factor = 1;
		reason = "";
		if (result)
		{
			ClearGuidance(result);
			result.State = INVALID_ARGUMENT;
		}
		if (!result || !FiniteValue(measured) || measured < 0 || !FinitePose(pose) ||
			!FiniteValue(lookAhead) || !FiniteValue(spacing) || !FiniteValue(maxCrossTrack))
		{
			m_PreviousPhysicalQuery = false;
			reason = "invalid_physical_query_measurement";
			return false;
		}
		if (allowReverse)
		{
			int reverse = TryReversePhysical(targetKey, pose, measured, lookAhead, spacing, maxCrossTrack,
				allowed, turnCos, factor, reason, result);
			if (reverse > 0) return true;
			if (reverse < 0)
			{
				InvalidatePhysicalQuery();
				return false;
			}
		}
		bool budgetReady = PhysicalBudget(measured, allowed, turnCos, factor, reason);
		bool densePhysicalStep = !budgetReady && reason == "measured_step_spans_multiple_vertices";
		if (!budgetReady && !densePhysicalStep)
		{
			m_PreviousPhysicalQuery = false;
			result.State = ADVANCE_LIMIT;
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
		m_PhysicalAllowReverse = allowReverse;
		bool hadPrevious = m_PreviousPhysicalQuery;
		vector priorPose = m_PreviousPose;
		vector priorStart = m_PreviousA;
		bool queryReady = true;
		if (densePhysicalStep)
		{
			// The single-corner budget has no derivation across dense samples.
			// Admit only the same bounded local-chain proof, never raw inflation.
			result.State = ADVANCE_LIMIT;
			queryReady = TryLocalChainProjection(targetKey, pose, lookAhead, spacing, maxCrossTrack, allowed, result, true);
			if (queryReady) reason = "";
		}
		else
		{
			// Bypass ordinary Query's override, which disarms physical context.
			super.Query(targetKey, pose, lookAhead, spacing, allowed, maxCrossTrack, result);
			// The ordinary failure commits nothing. Dense local samples can
			// require more than one adjacent leg for the corridor projection.
			TryLocalChainProjection(targetKey, pose, lookAhead, spacing, maxCrossTrack, allowed, result);
		}
		result.HasPreviousPhysicalQuery = hadPrevious;
		result.PreviousPhysicalPose = priorPose;
		result.PreviousIncomingStart = priorStart;
		m_PhysicalQueryActive = false;
		m_PhysicalAllowReverse = false;
		m_PhysicalMeasured = 0;
		m_PreviousPhysicalQuery = false;
		if ((result.State == TRACKING || result.State == SPACING_HOLD) &&
			m_Joined && targetKey == m_TargetKey && m_Segment < m_Points.Count() - 1)
		{
			RememberPhysicalQuery(targetKey, pose);
		}
		return queryReady;
	}

	protected bool TryLocalChainProjection(int targetKey, vector pose, float lookAhead,
		float spacing, float maxCrossTrack, float allowed, CF_DrivenRouteGuidance result, bool densePhysicalStep = false)
	{
		if (result.State != ADVANCE_LIMIT) return false;
		if (!m_Initialized || m_HistoryLost || m_Discontinuous) return false;
		if (lookAhead <= 0 || spacing < 0 || maxCrossTrack <= 0) return false;
		if (!densePhysicalStep)
		{
			if (!result.HasAdvanceDiagnostic || !result.DiagnosticNextSegment) return false;
			if (result.DiagnosticPriorSegment != m_Segment || result.DiagnosticSegment <= m_Segment) return false;
		}
		if (!m_PhysicalQueryActive || !HasLocalPhysicalContext(targetKey)) return false;
		// The unchanged physical budget must reach the first vertex. This is
		// not an exemption for unjoined, stale, or underreported physical motion.
		float firstVertex = m_Stations[m_Segment + 1];
		if (firstVertex > m_Progress + allowed + 0.001) return false;
		vector delta = pose - m_PreviousPose;
		delta[1] = 0;
		float chord = delta.Length();
		if (!FiniteValue(chord) || chord <= 0 || m_PhysicalMeasured <= 0) return false;
		if (chord > m_PhysicalMeasured + 0.001) return false;

		ref array<vector> tangents = {};
		float turnChordSum = 0;
		vector projected;
		float candidate = m_Progress;
		float crossTrack;
		int finalSegment = -1;
		for (int local = 0; local < CF_LOCAL_CHAIN_LEGS; local++)
		{
			int segment = m_Segment + local;
			if (segment + 1 >= m_Points.Count() || local + 1 >= m_PreviousChainPoints.Count()) return false;
			if (m_PreviousChainPoints[local] != m_Points[segment]) return false;
			if (m_PreviousChainPoints[local + 1] != m_Points[segment + 1]) return false;
			if (m_PreviousChainStations[local] != m_Stations[segment]) return false;
			if (m_PreviousChainStations[local + 1] != m_Stations[segment + 1]) return false;
			vector a = m_Points[segment];
			vector b = m_Points[segment + 1];
			float length = m_Stations[segment + 1] - m_Stations[segment];
			if (!FinitePose(a) || !FinitePose(b) || !FiniteValue(length) || length <= 0) return false;
			// All additional dense legs fit within one existing corridor radius
			// of recorded arc. A nearby but distant history branch is not searched.
			if (local > 0 && m_Stations[segment + 1] - firstVertex > maxCrossTrack) return false;
			vector tangent = b - a;
			tangent[1] = 0;
			tangent.Normalize();
			if (vector.Dot(tangent, delta) <= 0) return false;
			if (local > 0)
			{
				float turn = Math.Clamp(vector.Dot(tangents[local - 1], tangent), -1, 1);
				if (turn < 0) return false;
				turnChordSum += Math.Sqrt(2.0 - 2.0 * turn);
				// Sum of unit-tangent chord lengths <= sqrt(2) bounds total
				// absolute turning to <= 90 degrees, including alternating bends.
				if (turnChordSum > Math.Sqrt(2.0)) return false;
			}
			tangents.Insert(tangent);
			float raw = vector.Dot(pose - a, tangent) / length;
			if (!FiniteValue(raw) || raw < 0) return false;
			projected = a + (b - a) * Math.Clamp(raw, 0, 1);
			crossTrack = vector.DistanceXZ(pose, projected);
			if (!FiniteValue(crossTrack) || crossTrack > maxCrossTrack) return false;
			// Walk every endpoint plane in order. Never inspect a better-looking
			// later leg when this connected leg still has an interior projection.
			if (raw >= 1) continue;
			if (local < 2) return false; // existing single-corner proof owns these
			candidate = m_Stations[segment] + raw * length;
			finalSegment = segment;
			break;
		}
		if (finalSegment < 0 || candidate <= m_Progress) return false;
		vector axis = tangents[0] + tangents[tangents.Count() - 1];
		if (axis.LengthSq() <= 0) return false;
		axis.Normalize();
		float minimumDot = 1;
		foreach (vector direction : tangents)
			minimumDot = Math.Min(minimumDot, vector.Dot(axis, direction));
		if (!FiniteValue(minimumDot) || minimumDot < Math.Sqrt(0.5)) return false;
		float oldLength = m_Stations[m_Segment + 1] - m_Stations[m_Segment];
		vector oldCursor = m_Points[m_Segment] + (m_Points[m_Segment + 1] - m_Points[m_Segment]) *
			((m_Progress - m_Stations[m_Segment]) / oldLength);
		vector oldResidual = m_PreviousPose - oldCursor;
		vector newResidual = pose - projected;
		oldResidual[1] = 0;
		newResidual[1] = 0;
		if (oldResidual.Length() > maxCrossTrack) return false;
		float signedMotion = vector.Dot(axis, delta);
		float correction = -vector.Dot(axis, newResidual - oldResidual) / minimumDot;
		float advance = candidate - m_Progress;
		float physicalBound = m_PhysicalMeasured / minimumDot;
		if (!FiniteValue(signedMotion) || !FiniteValue(correction) || signedMotion <= 0) return false;
		if (!FiniteValue(advance) || !FiniteValue(physicalBound)) return false;
		if (advance > signedMotion / minimumDot + correction + 0.001) return false;
		if (advance > physicalBound + correction + 0.001) return false;
		// c*arc <= dot(axis, C1-C0) = dot(axis, Q-P-r1+r0).
		// This admits a branch-bound coordinate, never extra physical travel.
		m_Segment = finalSegment;
		m_Progress = candidate;
		ClearGuidance(result);
		FillGuidance(lookAhead, spacing, result);
		result.CrossTrack = crossTrack;
		result.HasProjectionCorrection = true;
		result.ProjectionPhysicalBound = physicalBound;
		result.ProjectionSignedCorrection = correction;
		result.ProjectionCandidateAdvance = advance;
		m_PreviousPhysicalQuery = false;
		return true;
	}

	protected bool HasAdjacentProjectionContext(int targetKey)
	{
		if (!m_PhysicalQueryActive || !m_PreviousPhysicalQuery || !m_Joined ||
			m_PreviousTargetKey != targetKey || targetKey != m_TargetKey ||
			m_PreviousProgress != m_Progress || !m_PreviousHasNext ||
			m_Segment >= m_Points.Count() - 2)
			return false;
		if (m_PreviousSegment != m_Segment && !m_PhysicalAllowReverse) return false;
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
