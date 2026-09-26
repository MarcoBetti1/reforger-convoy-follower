// Private read-only target-window filter. This does not join, advance, reset,
// or grant a budget to CF_DrivenRoute. Query remains the only cursor owner.
class CF_TrailEntryWindowRoute : CF_TrailGuideRoute
{
	bool ReadPrejoinWindow(int epoch, vector expectedEntry, vector expectedTangent,
		float requestedStation, out float safeStation, out string reason)
	{
		safeStation = 0;
		reason = "recorded_bound";
		if (!m_Initialized || m_Joined || m_HistoryLost || m_Discontinuous)
			return false;
		if (epoch != m_TargetKey || m_Segment != 0 || m_Progress != 0)
			return false;
		if (m_Points.Count() < 2 || m_Stations.Count() != m_Points.Count())
			return false;
		if (m_Stations[0] != 0 || !(requestedStation > 0 && requestedStation <= 30.0))
			return false;
		if (requestedStation > m_Stations[m_Stations.Count() - 1])
			return false;
		vector entry;
		vector tangent;
		float firstLength;
		float endStation;
		if (!ReadEntry(entry, tangent, firstLength, endStation))
			return false;
		if (vector.DistanceXZ(entry, expectedEntry) > 0.001)
			return false;
		if (vector.DistanceXZ(tangent, expectedTangent) > 0.001)
			return false;

		// Every retained segment in the offered prefix is examined, never a
		// sparse sample or nearest/later-segment search. Endpoints bound the
		// whole linear segment inside the convex 2m entry strip.
		float previousAlong = 0;
		for (int i = 0; i < m_Points.Count() - 1; i++)
		{
			float aStation = m_Stations[i];
			float bStation = m_Stations[i + 1];
			float length = bStation - aStation;
			if (!(length > 0 && length <= MAX_SAMPLE_STEP))
				return false;
			vector edge = m_Points[i + 1] - m_Points[i];
			edge[1] = 0;
			if (!(edge.Length() > 0))
				return false;
			edge.Normalize();
			float alignment = vector.Dot(edge, tangent);
			if (!(alignment >= 0.9 && alignment <= 1.001))
			{
				reason = "first_turn";
				return true; // Offer no point beyond this segment's start.
			}
			float candidateStation = Math.Min(requestedStation, bStation);
			float fraction = (candidateStation - aStation) / length;
			vector point = m_Points[i] + (m_Points[i + 1] - m_Points[i]) * fraction;
			vector offset = point - entry;
			offset[1] = 0;
			float along = vector.Dot(offset, tangent);
			vector lateral = offset - tangent * along;
			if (!(along >= previousAlong && lateral.Length() <= 2.0))
			{
				reason = "first_corridor_boundary";
				return true;
			}
			safeStation = candidateStation;
			previousAlong = along;
			if (candidateStation >= requestedStation)
				return true;
		}
		return false;
	}
}
