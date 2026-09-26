// Stateless road-point geometry. Road selection, reachability, occupancy,
// server orders, and AI waypoints remain with the driver controller.
class CF_RoadPolyline
{
	static float DistanceToSegmentXZ(vector point, vector start, vector end)
	{
		float dx = end[0] - start[0];
		float dz = end[2] - start[2];
		float lengthSquared = dx * dx + dz * dz;
		if (lengthSquared < 0.0001)
			return vector.DistanceXZ(point, start);
		float progress = ((point[0] - start[0]) * dx + (point[2] - start[2]) * dz) / lengthSquared;
		if (progress < 0)
			progress = 0;
		else if (progress > 1)
			progress = 1;
		return vector.DistanceXZ(point, Vector(start[0] + dx * progress, point[1], start[2] + dz * progress));
	}

	// Measure an obstacle against the exact forward road segments, including
	// the short connection from the truck's current origin to the centerline.
	// A stopped truck's heading or recent diagonal bay approach is not the
	// road that its next MOVE will use. Keep the clearance margin at the caller.
	static bool TryForwardCorridorClearance(array<vector> points, vector origin,
		vector travel, float distanceAhead, vector obstacle, out float clearance)
	{
		clearance = 0;
		if (!points || points.Count() < 2 || distanceAhead <= 0)
			return false;
		int closestSegment = -1;
		float closestProgress;
		float closestDistance = 1000000.0;
		vector closestDirection = vector.Zero;
		vector projectedStart = vector.Zero;
		for (int i = 0; i < points.Count() - 1; i++)
		{
			vector start = points[i];
			vector end = points[i + 1];
			float dx = end[0] - start[0];
			float dz = end[2] - start[2];
			float lengthSquared = dx * dx + dz * dz;
			if (lengthSquared < 0.01)
				continue;
			float progress = ((origin[0] - start[0]) * dx + (origin[2] - start[2]) * dz) / lengthSquared;
			if (progress < 0)
				progress = 0;
			else if (progress > 1)
				progress = 1;
			vector projected = start + (end - start) * progress;
			float distance = vector.DistanceXZ(origin, projected);
			if (distance >= closestDistance)
				continue;
			closestDistance = distance;
			closestSegment = i;
			closestProgress = progress;
			float length = Math.Sqrt(lengthSquared);
			closestDirection = Vector(dx / length, 0, dz / length);
			projectedStart = projected;
		}
		if (closestSegment < 0 || closestDistance > 24.0)
			return false;
		float alignment = closestDirection[0] * travel[0] + closestDirection[2] * travel[2];
		if (alignment < 0.25 && alignment > -0.25)
			return false;
		bool increasing = alignment > 0;
		float remaining = distanceAhead;
		int segment = closestSegment;
		float progressAlongSegment = closestProgress;
		clearance = DistanceToSegmentXZ(obstacle, origin, projectedStart);
		while (segment >= 0 && segment < points.Count() - 1)
		{
			vector a = points[segment];
			vector b = points[segment + 1];
			float segmentLength = vector.DistanceXZ(a, b);
			if (segmentLength >= 0.01)
			{
				float available = progressAlongSegment * segmentLength;
				if (increasing)
					available = (1.0 - progressAlongSegment) * segmentLength;
				vector from = a + (b - a) * progressAlongSegment;
				vector to = a;
				if (increasing)
					to = b;
				bool finalSegment = remaining <= available;
				if (finalSegment)
				{
					float finalProgress = progressAlongSegment - remaining / segmentLength;
					if (increasing)
						finalProgress = progressAlongSegment + remaining / segmentLength;
					to = a + (b - a) * finalProgress;
				}
				float segmentClearance = DistanceToSegmentXZ(obstacle, from, to);
				if (segmentClearance < clearance)
					clearance = segmentClearance;
				if (finalSegment)
					return true;
				remaining -= available;
			}
			if (increasing)
			{
				segment++;
				progressAlongSegment = 0;
			}
			else
			{
				segment--;
				progressAlongSegment = 1;
			}
		}
		return false;
	}

	static bool TryWalk(array<vector> points, vector lead, vector travel,
		float distanceAhead, out vector goal)
	{
		goal = vector.Zero;
		if (!points || points.Count() < 2)
			return false;
		int closestSegment = -1;
		float closestProgress;
		float closestDistance = 1000000.0;
		vector closestDirection = vector.Zero;
		for (int i = 0; i < points.Count() - 1; i++)
		{
			vector start = points[i];
			vector end = points[i + 1];
			float dx = end[0] - start[0];
			float dz = end[2] - start[2];
			float lengthSquared = dx * dx + dz * dz;
			if (lengthSquared < 0.01)
				continue;
			float progress = ((lead[0] - start[0]) * dx + (lead[2] - start[2]) * dz) / lengthSquared;
			if (progress < 0.0)
				progress = 0.0;
			else if (progress > 1.0)
				progress = 1.0;
			vector projected = Vector(start[0] + dx * progress, lead[1], start[2] + dz * progress);
			float distance = vector.DistanceXZ(lead, projected);
			if (distance >= closestDistance)
				continue;
			closestDistance = distance;
			closestSegment = i;
			closestProgress = progress;
			float length = Math.Sqrt(lengthSquared);
			closestDirection = Vector(dx / length, 0, dz / length);
		}
		if (closestSegment < 0 || closestDistance > 24.0)
			return false;
		float alignment = closestDirection[0] * travel[0] + closestDirection[2] * travel[2];
		if (alignment < 0.25 && alignment > -0.25)
			return false;
		bool increasing = alignment > 0.0;
		float remaining = distanceAhead;
		int segment = closestSegment;
		float progressAlongSegment = closestProgress;
		while (segment >= 0 && segment < points.Count() - 1)
		{
			vector a = points[segment];
			vector b = points[segment + 1];
			float segmentLength = vector.DistanceXZ(a, b);
			if (segmentLength < 0.01)
			{
				if (increasing)
					segment++;
				else
					segment--;
				continue;
			}
			float available;
			if (increasing)
				available = (1.0 - progressAlongSegment) * segmentLength;
			else
				available = progressAlongSegment * segmentLength;
			if (remaining <= available)
			{
				float goalProgress = progressAlongSegment;
				if (increasing)
					goalProgress += remaining / segmentLength;
				else
					goalProgress -= remaining / segmentLength;
				goal = a + (b - a) * goalProgress;
				return true;
			}
			remaining -= available;
			if (increasing)
			{
				segment++;
				progressAlongSegment = 0.0;
			}
			else
			{
				segment--;
				progressAlongSegment = 1.0;
			}
		}
		return false;
	}
}
