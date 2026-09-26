// Test-only road geometry survey for the short, visible arrival fixtures.
// It never moves a vehicle or reports a driving pass.
class CF_ShortArrivalGeometryProbeComponentClass : ScriptComponentClass
{
}

class CF_ShortArrivalGeometryProbeComponent : ScriptComponent
{
	protected vector PointAtArc(array<vector> points, float targetArc)
	{
		float arc = 0.0;
		for (int i = 1; i < points.Count(); i++)
		{
			float segment = vector.DistanceXZ(points[i - 1], points[i]);
			if (segment < 0.01)
				continue;
			if (arc + segment >= targetArc)
				return points[i - 1] + (points[i] - points[i - 1]) * ((targetArc - arc) / segment);
			arc += segment;
		}
		return points[points.Count() - 1];
	}

	protected vector TangentAtArc(array<vector> points, float targetArc)
	{
		float arc = 0.0;
		for (int i = 1; i < points.Count(); i++)
		{
			float segment = vector.DistanceXZ(points[i - 1], points[i]);
			if (segment < 0.01)
				continue;
			if (arc + segment >= targetArc)
			{
				vector delta = points[i] - points[i - 1];
				return Vector(delta[0] / segment, 0, delta[2] / segment);
			}
			arc += segment;
		}
		return vector.Zero;
	}

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		GetGame().GetCallqueue().CallLater(Survey, 3000, false);
	}

	protected void Survey()
	{
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
		{
			Print("[ConvoyFollower] SHORT_GEOMETRY_RESULT: FAIL road network unavailable");
			return;
		}
		RoadNetworkManager roads = aiWorld.GetRoadNetworkManager();
		ref array<BaseRoad> candidates = {};
		roads.GetRoadsInAABB(Vector(1100, -100, 2850), Vector(1750, 700, 3250), candidates);
		// Atlas road81 index9 has a visible, open junction-side unload area.
		vector expectedBay = Vector(1479.70, 35.5199, 3060.25);
		BaseRoad selected;
		ref array<vector> selectedPoints = {};
		float bayArc;
		float bestError = 1000000.0;
		for (int roadIndex = 0; roadIndex < candidates.Count(); roadIndex++)
		{
			BaseRoad candidate = candidates[roadIndex];
			if (!candidate || candidate.GetWidth() < 8.0)
				continue;
			ref array<vector> points = {};
			candidate.GetPoints(points);
			if (points.Count() < 2)
				continue;
			float length = 0.0;
			for (int p = 1; p < points.Count(); p++)
				length += vector.DistanceXZ(points[p - 1], points[p]);
			if (length < 450.0 || length > 520.0)
				continue;
			float probeArc = length * 0.70;
			float error = vector.DistanceXZ(PointAtArc(points, probeArc), expectedBay);
			if (error < bestError)
			{
				bestError = error;
				selected = candidate;
				selectedPoints = points;
				bayArc = probeArc;
			}
		}
		if (!selected || bestError > 3.0 || bayArc < 120.0)
		{
			Print("[ConvoyFollower] SHORT_GEOMETRY_RESULT: FAIL surveyed road81 bay not found candidates=" +
				candidates.Count() + " closest_gap=" + bestError);
			return;
		}
		ref array<float> offsets = {0.0, -60.0, -80.0, -100.0, 65.0};
		ref array<string> labels = {"bay", "lead_start", "unit1_start", "unit2_start", "second_leg_goal"};
		for (int i = 0; i < offsets.Count(); i++)
		{
			float arc = bayArc + offsets[i];
			vector center = PointAtArc(selectedPoints, arc);
			vector tangent = TangentAtArc(selectedPoints, arc);
			BaseRoad closest;
			float gap;
			roads.GetClosestRoad(center, closest, gap);
			vector connectedPoint;
			bool connected = roads.GetReachableWaypointInRoad(
				PointAtArc(selectedPoints, bayArc - 100.0), center, 20.0, connectedPoint);
			float surface = GetGame().GetWorld().GetSurfaceY(center[0], center[2]);
			Print("[ConvoyFollower] SHORT_GEOMETRY_SLOT: label=" + labels[i] + " arc=" + arc +
				" center=" + center + " tangent=" + tangent + " mapped_width=" + selected.GetWidth() +
				" nearest_gap=" + gap + " surface_y=" + surface + " connected=" + connected +
				" connected_point=" + connectedPoint + " geometry_only=true");
			if (!closest || gap > 2.0 || !connected || vector.DistanceXZ(connectedPoint, center) > 5.0)
			{
				Print("[ConvoyFollower] SHORT_GEOMETRY_RESULT: FAIL slot=" + labels[i] + " lacks mapped connection");
				return;
			}
		}
		Print("[ConvoyFollower] SHORT_GEOMETRY_RESULT: DONE road81 width=" + selected.GetWidth() +
			" bay_arc=" + bayArc + " match_gap=" + bestError +
			" (geometry only; spawn clearance and vehicle movement untested)");
	}
}
