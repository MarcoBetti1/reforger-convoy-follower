// Static test-only geometry survey for the measured width-8 forward parking
// scene. A clear result is a candidate for a later physical F5 drive, not a
// claim that the player truck or resumed convoy passed the parked line.
class CF_PassCorridorSurveyProbeComponentClass : ScriptComponentClass
{
}

class CF_PassCorridorSurveyProbeComponent : ScriptComponent
{
	[Attribute(defvalue: "0", params: "0 1 1", desc: "Restage static proxies upstream on the same surveyed width-8 road")]
	protected bool m_bUpstream;
	protected Vehicle m_Lead;
	protected Vehicle m_Park1;
	protected Vehicle m_Park2;
	protected IEntity m_eOccupant;
	protected bool m_bOccupied;
	protected bool m_bUpstreamStaged;
	protected vector m_vBay;
	protected vector m_vRoadLead;
	protected vector m_vMeasuredSouthShoulder;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Lead = Vehicle.Cast(owner);
		GetGame().GetCallqueue().CallLater(Survey, 5000, false);
	}

	protected bool TraceEntityFilter(IEntity entity, vector start, vector direction)
	{
		return entity != m_Lead;
	}

	protected bool FindVehicle(IEntity entity)
	{
		if (entity != m_Lead && Vehicle.Cast(entity))
		{
			m_bOccupied = true;
			m_eOccupant = entity;
		}
		return true;
	}

	protected string Describe(IEntity entity)
	{
		if (!entity)
			return "none";
		string result = entity.GetName() + " origin=" + entity.GetOrigin();
		EntityPrefabData data = entity.GetPrefabData();
		if (data)
			result += " prefab=" + data.GetPrefabName();
		return result;
	}

	protected float RoadGap(vector point)
	{
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return 999.0;
		BaseRoad road;
		float gap;
		aiWorld.GetRoadNetworkManager().GetClosestRoad(point, road, gap);
		if (!road || road.GetWidth() < 8.0)
			return 999.0;
		return gap;
	}

	protected bool ClearSegment(BaseWorld world, vector start, vector end, string label)
	{
		float length = vector.DistanceXZ(start, end);
		int samples = (int)(length / 5.0) + 1;
		float previousSurface = world.GetSurfaceY(start[0], start[2]);
		for (int i = 1; i <= samples; i++)
		{
			float fraction = (float)i / samples;
			vector sample = start + (end - start) * fraction;
			float surface = world.GetSurfaceY(sample[0], sample[2]);
			float rise = surface - previousSurface;
			float roadGap = RoadGap(sample);
			if (rise > 1.5 || rise < -1.5 || roadGap > 11.0)
			{
				Print("[ConvoyFollower] ROAD_BYPASS_REJECT: " + label + " sample=" + i +
					" point=" + sample + " grade_step=" + rise + " width8_road_gap=" + roadGap);
				return false;
			}
			sample[1] = surface;
			m_bOccupied = false;
			m_eOccupant = null;
			world.QueryEntitiesBySphere(sample, 4.0, FindVehicle);
			if (m_bOccupied)
			{
				Print("[ConvoyFollower] ROAD_BYPASS_REJECT: " + label + " sample=" + i +
					" point=" + sample + " vehicle=" + Describe(m_eOccupant));
				return false;
			}
			previousSurface = surface;
		}
		TraceParam trace = new TraceParam();
		trace.Start = start + Vector(0, 1.5, 0);
		trace.End = end + Vector(0, 1.5, 0);
		trace.Flags = TraceFlags.ENTS;
		trace.Exclude = m_Lead;
		float clearFraction = world.TraceMove(trace, TraceEntityFilter);
		if (clearFraction < 0.98)
		{
			vector hit = trace.Start + (trace.End - trace.Start) * clearFraction;
			Print("[ConvoyFollower] ROAD_BYPASS_REJECT: " + label + " trace_fraction=" + clearFraction +
				" hit=" + hit + " entity=" + Describe(trace.TraceEnt));
			return false;
		}
		return true;
	}

	protected bool CheckShoulder(BaseWorld world, vector roadLead, vector shoulder, string label)
	{
		float initialSurface = world.GetSurfaceY(roadLead[0], roadLead[2]);
		float surface = world.GetSurfaceY(shoulder[0], shoulder[2]);
		float rise = surface - initialSurface;
		shoulder[1] = surface + (roadLead[1] - initialSurface);
		float gap = RoadGap(shoulder);
		if (rise > 2.5 || rise < -2.5 || gap < 9.0 || gap > 20.0 ||
			vector.Distance(shoulder, m_Park1.GetOrigin()) < 12.0 ||
			vector.Distance(shoulder, m_Park2.GetOrigin()) < 12.0)
		{
			Print("[ConvoyFollower] ROAD_BYPASS_SHOULDER_REJECT: " + label +
				" point=" + shoulder + " road_gap=" + gap + " surface_rise=" + rise);
			return false;
		}
		m_bOccupied = false;
		m_eOccupant = null;
		world.QueryEntitiesBySphere(shoulder, 4.5, FindVehicle);
		if (m_bOccupied)
		{
			Print("[ConvoyFollower] ROAD_BYPASS_SHOULDER_REJECT: " + label +
				" occupied=" + Describe(m_eOccupant));
			return false;
		}
		if (!ClearSegment(world, roadLead, shoulder, label + " stage"))
			return false;
		return true;
	}

	protected bool CheckRoute(BaseWorld world, vector shoulder, vector axis, vector side,
		float offset, string label)
	{
		ref array<vector> goals = {};
		goals.Insert(m_Park2.GetOrigin() - axis * 22.0 + side * offset);
		goals.Insert(m_Park2.GetOrigin() + side * offset);
		goals.Insert(m_Park1.GetOrigin() + side * offset);
		goals.Insert(m_Park1.GetOrigin() + axis * 30.0 + side * 2.0);
		vector previous = shoulder;
		for (int i = 0; i < goals.Count(); i++)
		{
			vector goal = goals[i];
			goal[1] = world.GetSurfaceY(goal[0], goal[2]);
			if (!ClearSegment(world, previous, goal, label + " target=" + i))
				return false;
			Print("[ConvoyFollower] ROAD_BYPASS_SEGMENT_CLEAR: " + label + " target=" + i +
				" goal=" + goal + " width8_road_gap=" + RoadGap(goal));
			previous = goal;
		}
		Print("[ConvoyFollower] ROAD_BYPASS_CANDIDATE: CLEAR_STATIC_GEOMETRY " + label +
			" lead=" + shoulder + " park1=" + m_Park1.GetOrigin() +
			" park2=" + m_Park2.GetOrigin());
		return true;
	}

	protected vector PointAtArc(array<vector> points, float targetArc)
	{
		float arc = 0.0;
		for (int i = 1; i < points.Count(); i++)
		{
			float segment = vector.DistanceXZ(points[i - 1], points[i]);
			if (arc + segment >= targetArc && segment > 0.01)
			{
				float fraction = (targetArc - arc) / segment;
				return points[i - 1] + (points[i] - points[i - 1]) * fraction;
			}
			arc += segment;
		}
		return points[points.Count() - 1];
	}

	protected bool StageUpstream(BaseWorld world)
	{
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return false;
		vector requestedBay = Vector(1392.14, 37.0254, 3036.06);
		BaseRoad road;
		float gap;
		aiWorld.GetRoadNetworkManager().GetClosestRoad(requestedBay, road, gap);
		if (!road || road.GetWidth() < 8.0 || gap > 2.0)
			return false;
		ref array<vector> points = {};
		road.GetPoints(points);
		if (points.Count() < 2)
			return false;
		float bestDistance = 999999.0;
		float bestArc = -1.0;
		float arc = 0.0;
		for (int i = 1; i < points.Count(); i++)
		{
			vector a = points[i - 1];
			vector b = points[i];
			float dx = b[0] - a[0];
			float dz = b[2] - a[2];
			float segment = vector.DistanceXZ(a, b);
			if (segment < 0.01)
				continue;
			float fraction = ((requestedBay[0] - a[0]) * dx + (requestedBay[2] - a[2]) * dz) / (segment * segment);
			if (fraction < 0.0)
				fraction = 0.0;
			if (fraction > 1.0)
				fraction = 1.0;
			vector projection = a + (b - a) * fraction;
			float distance = vector.DistanceXZ(requestedBay, projection);
			if (distance < bestDistance)
			{
				bestDistance = distance;
				bestArc = arc + segment * fraction;
			}
			arc += segment;
		}
		if (bestArc < 0.0 || bestDistance > 2.0)
			return false;
		m_vBay = PointAtArc(points, bestArc);
		vector park2 = PointAtArc(points, bestArc + 60.0);
		vector park1 = PointAtArc(points, bestArc + 85.0);
		m_vRoadLead = PointAtArc(points, bestArc + 13.0);
		vector tangent = park1 - m_vBay;
		float tangentLength = vector.DistanceXZ(park1, m_vBay);
		if (tangentLength < 75.0)
			return false;
		vector south = Vector(-tangent[2] / tangentLength, 0, tangent[0] / tangentLength);
		m_vMeasuredSouthShoulder = m_vRoadLead + south * 10.0;
		park1[1] = world.GetSurfaceY(park1[0], park1[2]) + 0.1;
		park2[1] = world.GetSurfaceY(park2[0], park2[2]) + 0.1;
		m_vRoadLead[1] = world.GetSurfaceY(m_vRoadLead[0], m_vRoadLead[2]) + 0.1;
		m_vMeasuredSouthShoulder[1] = world.GetSurfaceY(m_vMeasuredSouthShoulder[0], m_vMeasuredSouthShoulder[2]) + 0.1;
		// Static geometry survey only: these proxy vehicles are deliberately
		// restaged. Their movement is never counted as gameplay path evidence.
		m_Park1.SetOrigin(park1);
		m_Park2.SetOrigin(park2);
		m_Lead.SetOrigin(m_vMeasuredSouthShoulder);
		Print("[ConvoyFollower] ROAD_BYPASS_SURVEY_STAGE: TEST_ONLY_STATIC_REPOSITION bay=" + m_vBay +
			" road_lead=" + m_vRoadLead + " shoulder=" + m_vMeasuredSouthShoulder +
			" park1=" + park1 + " park2=" + park2 + " measured_road_width=" + road.GetWidth());
		return true;
	}

	protected void Survey()
	{
		BaseWorld world = GetGame().GetWorld();
		m_Park1 = Vehicle.Cast(world.FindEntityByName("CF_SurveyPark1"));
		m_Park2 = Vehicle.Cast(world.FindEntityByName("CF_SurveyPark2"));
		if (!m_Lead || !m_Park1 || !m_Park2)
		{
			Print("[ConvoyFollower] ROAD_BYPASS_SURVEY_RESULT: FAIL missing test vehicles");
			return;
		}
		if (m_bUpstream && !m_bUpstreamStaged)
		{
			if (!StageUpstream(world))
			{
				Print("[ConvoyFollower] ROAD_BYPASS_SURVEY_RESULT: FAIL upstream width-8 arc staging unavailable");
				return;
			}
			m_bUpstreamStaged = true;
			GetGame().GetCallqueue().CallLater(Survey, 1500, false);
			return;
		}
		vector bay = Vector(1432.99, 37.0454, 3046.19);
		if (m_bUpstream)
			bay = m_vBay;
		vector delta = m_Park1.GetOrigin() - bay;
		float length = vector.DistanceXZ(m_Park1.GetOrigin(), bay);
		if (length < 50.0)
		{
			Print("[ConvoyFollower] ROAD_BYPASS_SURVEY_RESULT: FAIL invalid bay axis");
			return;
		}
		vector axis = Vector(delta[0] / length, 0, delta[2] / length);
		vector south = Vector(-axis[2], 0, axis[0]);
		vector roadLead = Vector(1445.63, 36.8553, 3050.31);
		if (m_bUpstream)
			roadLead = m_vRoadLead;
		ref array<float> offsets = {6.5, 8.25, 10.0};
		int clearCount = 0;
		Print("[ConvoyFollower] ROAD_BYPASS_SURVEY_BEGIN: bay=" + bay + " axis=" + axis +
			" parked=" + m_Park1.GetOrigin() + "," + m_Park2.GetOrigin());
		for (int sideIndex = 0; sideIndex < 2; sideIndex++)
		{
			float sideSign = 1.0;
			string sideName = "south";
			if (sideIndex == 1)
			{
				sideSign = -1.0;
				sideName = "north";
			}
			vector side = south * sideSign;
			for (int shoulderIndex = 0; shoulderIndex < 3; shoulderIndex++)
			{
				vector shoulder = roadLead + side * (10.0 + shoulderIndex * 2.0);
				if (sideIndex == 0 && shoulderIndex == 0 && !m_bUpstream)
					shoulder = Vector(1441.25, 36.4462, 3059.3);
				string stageLabel = sideName + " shoulder=" + shoulderIndex;
				if (!CheckShoulder(world, roadLead, shoulder, stageLabel))
					continue;
				for (int offsetIndex = 0; offsetIndex < offsets.Count(); offsetIndex++)
				{
					float offset = offsets[offsetIndex];
					if (CheckRoute(world, shoulder, axis, side, offset,
						sideName + " shoulder=" + shoulderIndex + " offset=" + offset))
						clearCount++;
				}
			}
		}
		Print("[ConvoyFollower] ROAD_BYPASS_SURVEY_RESULT: DONE static_clear_candidates=" + clearCount +
			" (geometry only; no convoy pass or resume tested)");
	}
}
