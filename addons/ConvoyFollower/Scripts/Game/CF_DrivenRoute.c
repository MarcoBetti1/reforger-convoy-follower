// Geometry only: one recorder/cursor per predecessor-follower link. No roads,
// entities, waypoints, controls, or synthetic connector points are involved.
class CF_DrivenRouteGuidance
{
	int State;
	vector Goal;
	vector Tangent;
	float Progress;
	float RecordedEnd;
	float ArcGap;
	float CrossTrack;
	bool HasArcGap;

	// Value-only failure snapshot. These fields never feed route decisions.
	bool HasAdvanceDiagnostic;
	vector DiagnosticPose;
	int DiagnosticPriorSegment;
	float DiagnosticPriorProgress;
	int DiagnosticSegment;
	float DiagnosticLoopProgress;
	vector DiagnosticA;
	vector DiagnosticB;
	vector DiagnosticNext;
	float DiagnosticStationA;
	float DiagnosticStationB;
	float DiagnosticStationNext;
	float DiagnosticLength;
	float DiagnosticRaw;
	bool DiagnosticAdjacentAvailable;
	float DiagnosticAdjacentRaw;
	bool DiagnosticRoundedCorner;
	bool DiagnosticNextSegment;
	float DiagnosticCandidate;
	float DiagnosticBudgetEnd;
	float DiagnosticBudget;
	float DiagnosticCorridorError;
}

class CF_DrivenRoute
{
	static const int UNSEEDED = 0;
	static const int APPROACH_START = 1;
	static const int TRACKING = 2;
	static const int SPACING_HOLD = 3;
	static const int OFF_ROUTE = 4;
	static const int HISTORY_LOST = 5;
	static const int TARGET_MISMATCH = 6;
	static const int DISCONTINUITY = 7;
	static const int ADVANCE_LIMIT = 8;
	static const int INVALID_ARGUMENT = 9;
	static const int MAX_POINTS = 256;
	static const float MAX_LENGTH = 600.0;
	static const float MIN_SAMPLE = 0.5;
	static const float MAX_SAMPLE_STEP = 30.0;

	protected ref array<vector> m_Points = {};
	protected ref array<float> m_Stations = {};
	protected int m_TargetKey;
	protected int m_Segment;
	protected float m_Progress;
	protected bool m_Initialized;
	protected bool m_Joined;
	protected bool m_HistoryLost;
	protected bool m_Discontinuous;

	// Caller changes the key deliberately when assignment, direction, or route
	// epoch changes. A mismatched key never silently erases the current history.
	void Reset(int targetKey, vector actualPredecessorPosition)
	{
		m_TargetKey = targetKey;
		m_Points.Clear();
		m_Stations.Clear();
		m_Points.Insert(actualPredecessorPosition);
		m_Stations.Insert(0.0);
		m_Segment = 0;
		m_Progress = 0;
		m_Initialized = true;
		m_Joined = false;
		m_HistoryLost = false;
		m_Discontinuous = false;
	}

	bool Record(int targetKey, vector actualPredecessorPosition)
	{
		if (!m_Initialized || targetKey != m_TargetKey || m_HistoryLost || m_Discontinuous)
			return false;
		int last = m_Points.Count() - 1;
		float step = vector.DistanceXZ(m_Points[last], actualPredecessorPosition);
		if (step < MIN_SAMPLE)
			return true;
		if (step > MAX_SAMPLE_STEP)
		{
			m_Discontinuous = true;
			return false;
		}
		m_Points.Insert(actualPredecessorPosition);
		m_Stations.Insert(m_Stations[last] + step);
		while (m_Points.Count() > MAX_POINTS || GetRetainedLength() > MAX_LENGTH)
		{
			m_Points.RemoveOrdered(0);
			m_Stations.RemoveOrdered(0);
			if (m_Segment > 0)
				m_Segment--;
			if (!m_Joined || m_Progress < m_Stations[0] - 0.001)
				m_HistoryLost = true;
		}
		return !m_HistoryLost;
	}

	int GetPointCount() { return m_Points.Count(); }
	float GetProgress() { return m_Progress; }
	float GetRetainedLength()
	{
		if (m_Stations.Count() < 2)
			return 0;
		return m_Stations[m_Stations.Count() - 1] - m_Stations[0];
	}

	protected void SampleAt(float station, out vector position, out vector tangent)
	{
		int segment = m_Segment;
		while (segment < m_Points.Count() - 2 && m_Stations[segment + 1] < station)
			segment++;
		vector a = m_Points[segment];
		vector b = m_Points[segment + 1];
		float length = m_Stations[segment + 1] - m_Stations[segment];
		float t = Math.Clamp((station - m_Stations[segment]) / length, 0.0, 1.0);
		position = a + (b - a) * t;
		tangent = Vector((b[0] - a[0]) / length, 0, (b[2] - a[2]) / length);
	}

	// maxAdvance is a caller-supplied physical progress budget for this update,
	// not a catch-up radius. Query never searches later nonadjacent segments.
	// Before joining, APPROACH_START is a position to join, not an arrived state:
	// HasArcGap stays false and ArcGap=-1 until a recorded segment is acquired.
	void Query(int targetKey, vector followerPosition, float lookAhead, float spacing,
		float maxAdvance, float maxCrossTrack, CF_DrivenRouteGuidance result)
	{
		if (!result)
			return;
		result.State = UNSEEDED;
		result.Goal = vector.Zero;
		result.Tangent = vector.Zero;
		result.Progress = m_Progress;
		result.RecordedEnd = 0;
		result.ArcGap = -1;
		result.CrossTrack = 0;
		result.HasArcGap = false;
		result.HasAdvanceDiagnostic = false;
		if (!m_Initialized)
			return;
		result.Goal = m_Points[0];
		result.RecordedEnd = m_Stations[m_Stations.Count() - 1];
		if (targetKey != m_TargetKey)
		{
			result.State = TARGET_MISMATCH;
			return;
		}
		if (m_HistoryLost)
		{
			result.State = HISTORY_LOST;
			return;
		}
		if (m_Discontinuous)
		{
			result.State = DISCONTINUITY;
			return;
		}
		if (lookAhead <= 0 || spacing < 0 || maxAdvance <= 0 || maxCrossTrack <= 0)
		{
			result.State = INVALID_ARGUMENT;
			return;
		}
		if (m_Points.Count() < 2)
			return;

		int segment = m_Segment;
		float progress = m_Progress;
		float budgetEnd = m_Progress + maxAdvance;
		while (segment < m_Points.Count() - 1)
		{
			vector a = m_Points[segment];
			vector b = m_Points[segment + 1];
			float length = m_Stations[segment + 1] - m_Stations[segment];
			float dx = b[0] - a[0];
			float dz = b[2] - a[2];
			float raw = ((followerPosition[0] - a[0]) * dx + (followerPosition[2] - a[2]) * dz) / (length * length);
			float t = Math.Clamp(raw, 0.0, 1.0);
			vector projected = a + (b - a) * t;
			float crossTrack = vector.DistanceXZ(followerPosition, projected);
			result.CrossTrack = crossTrack;
			if (!m_Joined && (raw < 0 || raw > 1 || crossTrack > maxCrossTrack))
			{
				result.State = APPROACH_START;
				return;
			}
			// Past a vertex, only the adjacent segment can be examined, and only
			// while still inside the current segment's lateral corridor.
			bool nextSegment = raw >= 1.0 && segment < m_Points.Count() - 2;
			bool roundedCorner = false;
			if (m_Joined && segment < m_Points.Count() - 2 && raw >= 0 && raw < 1 &&
				(1.0 - raw) * length <= maxCrossTrack)
			{
				// A real truck can round inside the vertex without crossing the
				// old endpoint plane. Compare only the immediately adjacent leg,
				// inside a bounded vertex band, never all nearby route segments.
				vector next = m_Points[segment + 2];
				float nextLength = m_Stations[segment + 2] - m_Stations[segment + 1];
				float nextX = next[0] - b[0];
				float nextZ = next[2] - b[2];
				float nextRaw = ((followerPosition[0] - b[0]) * nextX + (followerPosition[2] - b[2]) * nextZ) / (nextLength * nextLength);
				if (nextRaw > 0 && nextRaw <= 1)
				{
					float nextError = vector.DistanceXZ(followerPosition, b + (next - b) * nextRaw);
					roundedCorner = nextError <= maxCrossTrack && nextError + 0.001 < crossTrack;
				}
			}
			float corridorError = crossTrack;
			if (nextSegment)
				corridorError = Math.AbsFloat(dx * (followerPosition[2] - a[2]) - dz * (followerPosition[0] - a[0])) / length;
			if (!roundedCorner && corridorError > maxCrossTrack)
			{
				result.State = OFF_ROUTE;
				return;
			}
			float candidate = m_Stations[segment] + t * length;
			if (roundedCorner)
			{
				candidate = m_Stations[segment + 1];
				nextSegment = true;
			}
			if (candidate > budgetEnd + 0.001)
			{
				// Snapshot only the already-rejected decision, before retirement.
				// Keep Query's cursor, candidate, budget and return behavior intact.
				result.HasAdvanceDiagnostic = true;
				result.DiagnosticPose = followerPosition;
				result.DiagnosticPriorSegment = m_Segment;
				result.DiagnosticPriorProgress = m_Progress;
				result.DiagnosticSegment = segment;
				result.DiagnosticLoopProgress = progress;
				result.DiagnosticA = a;
				result.DiagnosticB = b;
				result.DiagnosticStationA = m_Stations[segment];
				result.DiagnosticStationB = m_Stations[segment + 1];
				result.DiagnosticLength = length;
				result.DiagnosticRaw = raw;
				result.DiagnosticRoundedCorner = roundedCorner;
				result.DiagnosticNextSegment = nextSegment;
				result.DiagnosticCandidate = candidate;
				result.DiagnosticBudgetEnd = budgetEnd;
				result.DiagnosticBudget = maxAdvance;
				result.DiagnosticCorridorError = corridorError;
				result.DiagnosticAdjacentAvailable = segment < m_Points.Count() - 2;
				result.DiagnosticNext = vector.Zero;
				result.DiagnosticStationNext = 0;
				result.DiagnosticAdjacentRaw = 0;
				if (result.DiagnosticAdjacentAvailable)
				{
					vector diagnosticNext = m_Points[segment + 2];
					float diagnosticLength = m_Stations[segment + 2] - m_Stations[segment + 1];
					float diagnosticX = diagnosticNext[0] - b[0];
					float diagnosticZ = diagnosticNext[2] - b[2];
					result.DiagnosticNext = diagnosticNext;
					result.DiagnosticStationNext = m_Stations[segment + 2];
					result.DiagnosticAdjacentRaw = ((followerPosition[0] - b[0]) * diagnosticX +
						(followerPosition[2] - b[2]) * diagnosticZ) / (diagnosticLength * diagnosticLength);
				}
				result.State = ADVANCE_LIMIT;
				return;
			}
			if (candidate > progress)
				progress = candidate;
			if (!nextSegment)
				break;
			segment++;
		}
		m_Joined = true;
		m_Segment = segment;
		m_Progress = progress;
		result.Progress = progress;
		result.ArcGap = result.RecordedEnd - progress;
		result.HasArcGap = true;
		float goalStation = Math.Min(progress + lookAhead, result.RecordedEnd - spacing);
		result.State = TRACKING;
		if (goalStation <= progress)
		{
			goalStation = progress;
			result.State = SPACING_HOLD;
		}
		SampleAt(goalStation, result.Goal, result.Tangent);
	}
}
