// Test-only, read-only road geometry survey. It never issues convoy orders or
// moves actors. A wide mapped road is a candidate, not a proven passing lane.
class CF_RoadWidthSurveyProbeComponentClass : ScriptComponentClass
{
}

class CF_RoadWidthSurveyProbeComponent : ScriptComponent
{
	protected bool m_bFinished;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		GetGame().GetCallqueue().CallLater(Survey, 3000, false);
	}

	protected float SegmentLength(vector a, vector b)
	{
		return vector.DistanceXZ(a, b);
	}

	protected void LogKnownPoint(RoadNetworkManager manager, string label, vector point)
	{
		BaseRoad road;
		float distance;
		manager.GetClosestRoad(point, road, distance);
		float width = -1;
		if (road)
			width = road.GetWidth();
		Print("[ConvoyFollower] ROAD_SURVEY_KNOWN: " + label + " point=" + point +
			" mapped_width=" + width + " distance_to_road=" + distance);
	}

	// Report the actual centerline at four staged-vehicle stations. The first
	// wide fixture used straight-line interpolation and unintentionally put two
	// trucks 16–27 m away from the winding mapped road. This survey only reads
	// road geometry; it does not move any entity or issue an AI order.
	protected void LogWideStartStations(RoadNetworkManager manager)
	{
		vector desiredLead = Vector(1269.0, 34.3, 3004.0);
		vector surveyedMiddle = Vector(1392.14, 37.0254, 3036.06);
		BaseRoad road;
		float middleGap;
		manager.GetClosestRoad(surveyedMiddle, road, middleGap);
		if (!road || road.GetWidth() < 8.0)
		{
			Print("[ConvoyFollower] ROAD_SURVEY_START_RESULT: FAIL width-8 centerline missing at candidate28");
			return;
		}
		ref array<vector> points = {};
		road.GetPoints(points);
		if (points.Count() < 2)
		{
			Print("[ConvoyFollower] ROAD_SURVEY_START_RESULT: FAIL candidate28 road has too few points");
			return;
		}
		float bestGap = 1000000.0;
		float leadArc = -1.0;
		float arcBefore;
		for (int i = 1; i < points.Count(); i++)
		{
			vector a = points[i - 1];
			vector b = points[i];
			float dx = b[0] - a[0];
			float dz = b[2] - a[2];
			float lengthSquared = dx * dx + dz * dz;
			if (lengthSquared < 0.01)
				continue;
			float segmentLength = Math.Sqrt(lengthSquared);
			float progress = ((desiredLead[0] - a[0]) * dx + (desiredLead[2] - a[2]) * dz) / lengthSquared;
			if (progress < 0.0)
				progress = 0.0;
			else if (progress > 1.0)
				progress = 1.0;
			vector projected = a + (b - a) * progress;
			float gap = vector.DistanceXZ(desiredLead, projected);
			if (gap < bestGap)
			{
				bestGap = gap;
				leadArc = arcBefore + segmentLength * progress;
			}
			arcBefore += segmentLength;
		}
		Print("[ConvoyFollower] ROAD_SURVEY_START_ANCHOR: desired=" + desiredLead +
			" lead_arc=" + leadArc + " lateral_gap=" + bestGap +
			" road_points=" + points.Count() + " road_width=" + road.GetWidth());
		if (leadArc < 98.0 || bestGap > 15.0)
		{
			Print("[ConvoyFollower] ROAD_SURVEY_START_RESULT: FAIL no safe four-station arc behind lead");
			return;
		}
		for (int station = 0; station < 4; station++)
		{
			float stationArc = leadArc - station * 30.0;
			float traversed;
			for (int j = 1; j < points.Count(); j++)
			{
				vector from = points[j - 1];
				vector to = points[j];
				float segment = vector.DistanceXZ(from, to);
				if (segment < 0.01)
					continue;
				if (traversed + segment < stationArc)
				{
					traversed += segment;
					continue;
				}
				float portion = (stationArc - traversed) / segment;
				vector center = from + (to - from) * portion;
				vector tangent = to - from;
				tangent[1] = 0;
				tangent.Normalize();
				BaseRoad stationRoad;
				float roadGap;
				manager.GetClosestRoad(center, stationRoad, roadGap);
				float width = -1.0;
				if (stationRoad)
					width = stationRoad.GetWidth();
				Print("[ConvoyFollower] ROAD_SURVEY_START_SLOT: station=" + station +
					" arc=" + stationArc + " center=" + center + " tangent=" + tangent +
					" mapped_width=" + width + " road_gap=" + roadGap);
				break;
			}
		}
		Print("[ConvoyFollower] ROAD_SURVEY_START_RESULT: DONE geometry only; spawn height and physical clearance need F5");
	}

	protected void Survey()
	{
		if (m_bFinished)
			return;
		m_bFinished = true;
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
		{
			Print("[ConvoyFollower] ROAD_SURVEY_RESULT: FAIL road manager missing");
			return;
		}
		RoadNetworkManager manager = aiWorld.GetRoadNetworkManager();
		LogKnownPoint(manager, "current_unload_bay", Vector(1526.38, 32.33, 3333.61));
		LogKnownPoint(manager, "current_second_ahead", Vector(1593.91, 21.08, 3363.14));
		LogKnownPoint(manager, "current_first_ahead", Vector(1613.24, 20.03, 3365.59));
		LogWideStartStations(manager);
		ref array<BaseRoad> roads = {};
		manager.GetRoadsInAABB(Vector(700, -100, 2500), Vector(2700, 700, 4100), roads);
		Print("[ConvoyFollower] ROAD_SURVEY_BEGIN: bounds_x=700..2700 bounds_z=2500..4100 road_count=" + roads.Count());
		int wideCount;
		int connectedCount;
		int loggedCount;
		float widest;
		for (int i = 0; i < roads.Count(); i++)
		{
			BaseRoad road = roads[i];
			if (!road)
				continue;
			float width = road.GetWidth();
			if (width > widest)
				widest = width;
			if (width < 8.0)
				continue;
			wideCount++;
			ref array<vector> points = {};
			road.GetPoints(points);
			if (points.Count() < 2)
				continue;
			float roadLength;
			for (int j = 1; j < points.Count(); j++)
				roadLength += SegmentLength(points[j - 1], points[j]);
			vector start = points[0];
			vector end = points[points.Count() - 1];
			vector axis = end - start;
			float axisLength = Math.Sqrt(axis[0] * axis[0] + axis[2] * axis[2]);
			if (axisLength < 1.0)
				continue;
			axis = Vector(axis[0] / axisLength, 0, axis[2] / axisLength);
			vector middle = points[points.Count() / 2];
			vector goal = middle + axis * 85.0;
			vector connectedGoal;
			bool connected = manager.GetReachableWaypointInRoad(middle, goal, 30.0, connectedGoal);
			BaseRoad goalRoad;
			float goalDistance;
			manager.GetClosestRoad(connectedGoal, goalRoad, goalDistance);
			float goalWidth = -1.0;
			if (goalRoad)
				goalWidth = goalRoad.GetWidth();
			float projectedAdvance = SegmentLength(middle, connectedGoal);
			if (connected && projectedAdvance >= 65.0 && goalWidth >= 8.0 && goalDistance <= 5.0)
				connectedCount++;
			if (loggedCount >= 100 || roadLength < 25.0)
				continue;
			Print("[ConvoyFollower] ROAD_SURVEY_CANDIDATE: index=" + i + " width=" + width +
				" segment_length=" + roadLength + " start=" + start + " middle=" + middle +
				" end=" + end + " connected85=" + connected + " connected_goal=" + connectedGoal +
				" connected_distance=" + projectedAdvance + " goal_width=" + goalWidth +
				" goal_road_distance=" + goalDistance);
			loggedCount++;
		}
		Print("[ConvoyFollower] ROAD_SURVEY_RESULT: DONE total=" + roads.Count() + " width_at_least_8=" +
			wideCount + " connected_85m_with_width_at_least_8=" + connectedCount +
			" logged=" + loggedCount + " widest=" + widest +
			" (geometry only; terrain, obstacles, and actual passing unverified)");
	}
}
