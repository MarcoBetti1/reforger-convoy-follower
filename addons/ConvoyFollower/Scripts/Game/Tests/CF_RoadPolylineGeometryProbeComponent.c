// Test-only fixed geometry cases for the stateless road-polyline helper.
// The separate Arland geometry world attaches this probe to one inert truck.
class CF_RoadPolylineGeometryProbeComponentClass : ScriptComponentClass
{
}

class CF_RoadPolylineGeometryProbeComponent : ScriptComponent
{
	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		GetGame().GetCallqueue().CallLater(RunCases, 1000, false);
	}

	protected bool ExpectGoal(string label, array<vector> points, vector lead,
		vector travel, float distanceAhead, vector expected)
	{
		vector actual;
		bool found = CF_RoadPolyline.TryWalk(points, lead, travel, distanceAhead, actual);
		if (found && vector.Distance(actual, expected) <= 0.01)
			return true;
		Print("[ConvoyFollower] POLYLINE_CASE: FAIL " + label +
			" found=" + found + " actual=" + actual + " expected=" + expected);
		return false;
	}

	protected bool ExpectFailure(string label, array<vector> points, vector lead,
		vector travel, float distanceAhead)
	{
		vector actual;
		if (!CF_RoadPolyline.TryWalk(points, lead, travel, distanceAhead, actual))
			return true;
		Print("[ConvoyFollower] POLYLINE_CASE: FAIL " + label +
			" unexpectedly selected=" + actual);
		return false;
	}

	protected bool ExpectClearance(string label, array<vector> points, vector origin,
		vector travel, float distanceAhead, vector obstacle, float expected)
	{
		float actual;
		bool found = CF_RoadPolyline.TryForwardCorridorClearance(points, origin, travel,
			distanceAhead, obstacle, actual);
		if (found && actual >= expected - 0.01 && actual <= expected + 0.01)
			return true;
		Print("[ConvoyFollower] POLYLINE_CASE: FAIL " + label + " found=" + found +
			" clearance=" + actual + " expected=" + expected);
		return false;
	}

	protected bool ExpectClearanceFailure(string label, array<vector> points, vector origin,
		vector travel, float distanceAhead)
	{
		float clearance;
		if (!CF_RoadPolyline.TryForwardCorridorClearance(points, origin, travel,
			distanceAhead, Vector(50, 0, 9), clearance))
			return true;
		Print("[ConvoyFollower] POLYLINE_CASE: FAIL " + label + " unexpectedly mapped corridor");
		return false;
	}

	protected void RunCases()
	{
		int passed = 0;
		ref array<vector> straight = {};
		straight.Insert(Vector(0, 0, 0));
		straight.Insert(Vector(100, 0, 0));
		if (ExpectGoal("straight", straight, Vector(25, 0, 5), Vector(1, 0, 0),
			40.0, Vector(65, 0, 0)))
			passed++;
		if (ExpectGoal("reverse", straight, Vector(75, 0, 0), Vector(-1, 0, 0),
			25.0, Vector(50, 0, 0)))
			passed++;
		if (ExpectFailure("perpendicular", straight, Vector(25, 0, 0),
			Vector(0, 0, 1), 20.0))
			passed++;
		if (ExpectFailure("lateral_limit", straight, Vector(25, 0, 25),
			Vector(1, 0, 0), 20.0))
			passed++;
		if (ExpectFailure("road_end", straight, Vector(90, 0, 0),
			Vector(1, 0, 0), 20.0))
			passed++;

		ref array<vector> bend = {};
		bend.Insert(Vector(0, 0, 0));
		bend.Insert(Vector(50, 0, 0));
		bend.Insert(Vector(50, 0, 50));
		if (ExpectGoal("bend_arc", bend, Vector(45, 0, 0), Vector(1, 0, 0),
			30.0, Vector(50, 0, 25)))
			passed++;

		ref array<vector> duplicate = {};
		duplicate.Insert(Vector(0, 0, 0));
		duplicate.Insert(Vector(0, 0, 0));
		duplicate.Insert(Vector(100, 0, 0));
		if (ExpectGoal("zero_length_segment", duplicate, Vector(25, 0, 0),
			Vector(1, 0, 0), 40.0, Vector(65, 0, 0)))
			passed++;

		ref array<vector> slope = {};
		slope.Insert(Vector(0, 0, 0));
		slope.Insert(Vector(100, 10, 0));
		if (ExpectGoal("height_interpolation", slope, Vector(25, 2.5, 0),
			Vector(1, 0, 0), 25.0, Vector(50, 5, 0)))
			passed++;

		if (ExpectClearance("lead_blocks_lane", straight, Vector(25, 0, 0),
			Vector(1, 0, 0), 30.0, Vector(45, 0, 0), 0.0))
			passed++;
		if (ExpectClearance("diagonal_approach_clear_shoulder", straight, Vector(25, 0, 0),
			Vector(0.8, 0, 0.6), 30.0, Vector(45, 0, 9), 9.0))
			passed++;
		if (ExpectClearance("bend_blocks_corridor", bend, Vector(45, 0, 0),
			Vector(1, 0, 0), 30.0, Vector(50, 0, 20), 0.0))
			passed++;
		if (ExpectClearance("reverse_clear_shoulder", straight, Vector(75, 0, 0),
			Vector(-1, 0, 0), 30.0, Vector(55, 0, 9), 9.0))
			passed++;
		if (ExpectClearance("truck_to_road_connection", straight, Vector(25, 0, 6),
			Vector(1, 0, 0), 30.0, Vector(25, 0, 7), 1.0))
			passed++;
		if (ExpectClearance("duplicate_point_corridor", duplicate, Vector(25, 0, 0),
			Vector(1, 0, 0), 30.0, Vector(40, 0, 9), 9.0))
			passed++;
		if (ExpectClearance("obstacle_behind", straight, Vector(25, 0, 0),
			Vector(1, 0, 0), 30.0, Vector(10, 0, 0), 15.0))
			passed++;
		if (ExpectClearanceFailure("corridor_road_end", straight, Vector(90, 0, 0),
			Vector(1, 0, 0), 30.0))
			passed++;
		if (ExpectClearanceFailure("corridor_perpendicular", straight, Vector(25, 0, 0),
			Vector(0, 0, 1), 30.0))
			passed++;
		if (ExpectClearanceFailure("corridor_lateral_limit", straight, Vector(25, 0, 25),
			Vector(1, 0, 0), 30.0))
			passed++;

		if (passed == 18)
			Print("[ConvoyFollower] POLYLINE_RESULT: PASS cases=" + passed);
		else
			Print("[ConvoyFollower] POLYLINE_RESULT: FAIL cases=" + passed + "/18");
	}
}
