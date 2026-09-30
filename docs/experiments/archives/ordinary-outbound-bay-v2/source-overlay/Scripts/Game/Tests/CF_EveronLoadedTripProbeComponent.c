class CF_EveronLoadedTripProbeComponentClass : CF_LoadedSupplyTripProbeComponentClass {}
// Private coordinator: unchanged ordinary scene poses, normal controller/session,
// native100 cargo and original travel/arrival/Hold/Resume assertions. No home PASS.
class CF_EveronLoadedTripProbeComponent : CF_LoadedSupplyTripProbeComponent
{
	protected bool ProjectRoadHint(BaseRoad road, vector hint, out vector projected)
	{
		if (!road) return false;
		array<vector> points = {};
		road.GetPoints(points);
		float nearest = 1000000.0;
		bool found;
		for (int i = 0; i < points.Count() - 1; i++)
		{
			vector start = points[i];
			vector delta = points[i + 1] - start;
			float lengthSquared = delta[0] * delta[0] + delta[2] * delta[2];
			if (lengthSquared < 0.01) continue;
			float t = ((hint[0] - start[0]) * delta[0] + (hint[2] - start[2]) * delta[2]) / lengthSquared;
			t = Math.Clamp(t, 0.0, 1.0);
			vector candidate = start + delta * t;
			float distance = vector.DistanceXZ(hint, candidate);
			if (distance >= nearest) continue;
			nearest = distance;
			projected = candidate;
			found = true;
		}
		return found;
	}

	override protected bool CF_SelectAIDriveGoal(RoadNetworkManager roads)
	{
		array<vector> hints = { Vector(7233, 140, 2838), Vector(7350, 140, 2838) };
		bool found;
		foreach (vector hint : hints)
		{
			BaseRoad road;
			float distance;
			int roadId = roads.GetClosestRoad(hint, road, distance);
			vector roadPoint;
			bool projected = ProjectRoadHint(road, hint, roadPoint);
			vector resolved;
			bool connected = projected && roads.GetReachableWaypointInRoad(m_vInitialLeadPosition, roadPoint, 5.0, resolved);
			Print("[ConvoyFollower] EVERON_DAY_ROUTE_SURVEY: hint=" + hint + " road_id=" + roadId +
				" road_distance=" + distance + " projected=" + projected + " road_point=" + roadPoint +
				" connected=" + connected + " resolved=" + resolved + " geometry_only=true");
			if (found || !connected || vector.DistanceXZ(roadPoint, resolved) > 5.0 ||
				vector.DistanceXZ(m_vInitialLeadPosition, resolved) < 350.0 || resolved[2] - m_vInitialLeadPosition[2] < 300.0) continue;
			m_vRoadGoal = resolved;
			found = true;
		}
		if (!found)
		{
			Print("[ConvoyFollower] EVERON_DAY_ROUTE_SURVEY: FAIL reason=declared_north_goal_unavailable legacy_fallback=false");
			return false;
		}
		Print("[ConvoyFollower] EVERON_DAY_ROUTE_GOAL: start=" + m_vInitialLeadPosition + " goal=" + m_vRoadGoal +
			" native_pilot=true exact_activity_gate_m=100 inherited_spacing_m=60 inherited_arrival_s=180 native_loaded_follower=true return_home_tested=false");
		return true;
	}

}
