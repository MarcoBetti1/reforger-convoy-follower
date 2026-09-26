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
	[Attribute(defvalue: "0", params: "0 1 1", desc: "Scan multiple width-8 road and bay candidates with static M923 proxies")]
	protected bool m_bAtlas;
	protected Vehicle m_Lead;
	protected Vehicle m_Park1;
	protected Vehicle m_Park2;
	protected IEntity m_eOccupant;
	protected bool m_bOccupied;
	protected bool m_bUpstreamStaged;
	protected vector m_vBay;
	protected vector m_vRoadLead;
	protected vector m_vMeasuredSouthShoulder;
	protected ref array<vector> m_aAtlasBays = {};
	protected ref array<vector> m_aAtlasLeads = {};
	protected ref array<vector> m_aAtlasPark1 = {};
	protected ref array<vector> m_aAtlasPark2 = {};
	protected ref array<vector> m_aAtlasTangents = {};
	protected ref array<vector> m_aAtlasFrontStops = {};
	protected ref array<vector> m_aAtlasApproachTangents = {};
	protected ref array<vector> m_aAtlasStartCenters = {};
	protected ref array<vector> m_aAtlasStartTangents = {};
	protected ref array<int> m_aAtlasRoadIndex = {};
	protected int m_iAtlasCursor;
	protected int m_iAtlasClearSlots;
	protected bool m_bAtlasPrepared;
	protected bool m_bAtlasSlotStaged;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Lead = Vehicle.Cast(owner);
		GetGame().GetCallqueue().CallLater(Survey, 5000, false);
	}

	protected bool TraceEntityFilter(IEntity entity, vector start, vector direction)
	{
		// Atlas proxies are moved between distant stations. Enfusion's broadphase
		// can still report their old bounds after SetWorldTransform, so compare
		// their current oriented footprints explicitly below instead.
		return entity != m_Lead && entity != m_Park1 && entity != m_Park2;
	}

	protected bool FindVehicle(IEntity entity)
	{
		if (entity != m_Lead && entity != m_Park1 && entity != m_Park2 && Vehicle.Cast(entity))
		{
			m_bOccupied = true;
			m_eOccupant = entity;
		}
		return true;
	}

	// Conservative M923 footprint plus clearance. This check uses the *actual*
	// transform rather than a moved proxy's potentially stale physics broadphase.
	protected bool InsideParkedProxyFootprint(vector sample, Vehicle vehicle)
	{
		if (!vehicle)
			return false;
		vector origin = vehicle.GetOrigin();
		vector forward = vehicle.GetWorldTransformAxis(2);
		float length = Math.Sqrt(forward[0] * forward[0] + forward[2] * forward[2]);
		if (length < 0.5)
			return vector.DistanceXZ(sample, origin) <= 7.5;
		float fx = forward[0] / length;
		float fz = forward[2] / length;
		float dx = sample[0] - origin[0];
		float dz = sample[2] - origin[2];
		float along = dx * fx + dz * fz;
		float across = -dx * fz + dz * fx;
		return along >= -6.0 && along <= 6.0 && across >= -4.5 && across <= 4.5;
	}

	protected IEntity ParkedProxyAt(vector sample)
	{
		if (InsideParkedProxyFootprint(sample, m_Park1))
			return m_Park1;
		if (InsideParkedProxyFootprint(sample, m_Park2))
			return m_Park2;
		return null;
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

	protected bool ClearSegment(BaseWorld world, vector start, vector end, string label, float maxRoadGap)
	{
		float length = vector.DistanceXZ(start, end);
		int samples = (int)(length / 2.0) + 1;
		float previousSurface = world.GetSurfaceY(start[0], start[2]);
		for (int i = 1; i <= samples; i++)
		{
			float fraction = (float)i / samples;
			vector sample = start + (end - start) * fraction;
			float surface = world.GetSurfaceY(sample[0], sample[2]);
			float rise = surface - previousSurface;
			float roadGap = RoadGap(sample);
			if (rise > 1.5 || rise < -1.5 || roadGap > maxRoadGap)
			{
				Print("[ConvoyFollower] ROAD_BYPASS_REJECT: " + label + " sample=" + i +
					" point=" + sample + " grade_step=" + rise + " width8_road_gap=" + roadGap +
					" allowed_road_gap=" + maxRoadGap);
				return false;
			}
			sample[1] = surface;
			IEntity parkedProxy = ParkedProxyAt(sample);
			if (parkedProxy)
			{
				Print("[ConvoyFollower] ROAD_BYPASS_REJECT: " + label + " sample=" + i +
					" point=" + sample + " parked_proxy_footprint=" + Describe(parkedProxy));
				return false;
			}
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
		if (!ClearSegment(world, roadLead, shoulder, label + " stage", 20.0))
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
			// The lead may start parked 12–14 m off center. Its first, checked
			// shoulder connector can use that same 20 m terrain corridor; after
			// rejoining the passing lane, every segment stays within 11 m.
			float maxRoadGap = 11.0;
			if (i == 0)
				maxRoadGap = 20.0;
			if (!ClearSegment(world, previous, goal, label + " target=" + i, maxRoadGap))
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
		vector finalDelta = points[points.Count() - 1] - points[points.Count() - 2];
		float finalLength = vector.DistanceXZ(points[points.Count() - 1], points[points.Count() - 2]);
		if (finalLength < 0.01)
			return Vector(1, 0, 0);
		return Vector(finalDelta[0] / finalLength, 0, finalDelta[2] / finalLength);
	}

	protected bool PrepareAtlas(BaseWorld world)
	{
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return false;
		RoadNetworkManager manager = aiWorld.GetRoadNetworkManager();
		ref array<BaseRoad> roads = {};
		manager.GetRoadsInAABB(Vector(700, -100, 2000), Vector(3500, 700, 4100), roads);
		ref array<float> fractions = {0.4, 0.55, 0.7};
		for (int roadIndex = 0; roadIndex < roads.Count(); roadIndex++)
		{
			BaseRoad road = roads[roadIndex];
			if (!road || road.GetWidth() < 8.0)
				continue;
			ref array<vector> points = {};
			road.GetPoints(points);
			if (points.Count() < 2)
				continue;
			float roadLength = 0.0;
			for (int p = 1; p < points.Count(); p++)
				roadLength += vector.DistanceXZ(points[p - 1], points[p]);
			if (roadLength < 340.0)
				continue;
			for (int station = 0; station < fractions.Count(); station++)
			{
				float bayArc = roadLength * fractions[station];
				// Keep enough same-road distance behind for a later real
				// three-truck route and beyond the forward parking line.
				if (bayArc < 220.0 || bayArc + 115.0 > roadLength)
					continue;
				vector bay = PointAtArc(points, bayArc);
				vector lead = PointAtArc(points, bayArc + 13.0);
				vector park2 = PointAtArc(points, bayArc + 60.0);
				vector park1 = PointAtArc(points, bayArc + 85.0);
				vector initial = PointAtArc(points, bayArc - 180.0);
				vector resolvedBay;
				vector resolvedPark;
				bool connectedIn = manager.GetReachableWaypointInRoad(initial, bay, 25.0, resolvedBay);
				bool connectedOut = manager.GetReachableWaypointInRoad(bay, park1, 25.0, resolvedPark);
				if (!connectedIn || !connectedOut ||
					vector.DistanceXZ(resolvedBay, bay) > 8.0 ||
					vector.DistanceXZ(resolvedPark, park1) > 8.0 ||
					RoadGap(bay) > 2.0 || RoadGap(park1) > 2.0 || RoadGap(park2) > 2.0)
					continue;
				m_aAtlasBays.Insert(bay);
				m_aAtlasLeads.Insert(lead);
				m_aAtlasPark1.Insert(park1);
				m_aAtlasPark2.Insert(park2);
				m_aAtlasTangents.Insert(TangentAtArc(points, bayArc + 60.0));
				m_aAtlasFrontStops.Insert(PointAtArc(points, bayArc - 13.0));
				m_aAtlasApproachTangents.Insert(TangentAtArc(points, bayArc - 13.0));
				m_aAtlasRoadIndex.Insert(roadIndex);
				for (int startStation = 0; startStation < 4; startStation++)
				{
					float startArc = bayArc - 185.0 - startStation * 20.0;
					if (startArc < 10.0)
					{
						m_aAtlasStartCenters.Insert(vector.Zero);
						m_aAtlasStartTangents.Insert(vector.Zero);
					}
					else
					{
						m_aAtlasStartCenters.Insert(PointAtArc(points, startArc));
						m_aAtlasStartTangents.Insert(TangentAtArc(points, startArc));
					}
				}
				Print("[ConvoyFollower] ROAD_ATLAS_SLOT: index=" + (m_aAtlasBays.Count() - 1) +
					" road=" + roadIndex + " width=" + road.GetWidth() + " road_length=" + roadLength +
					" bay_arc=" + bayArc + " bay=" + bay + " park2=" + park2 + " park1=" + park1 +
					" connected_in=" + connectedIn + " connected_out=" + connectedOut);
			}
		}
		Print("[ConvoyFollower] ROAD_ATLAS_BEGIN: connected_width8_slots=" + m_aAtlasBays.Count() +
			" measured_road_count=" + roads.Count() + " geometry_only=true");
		return !m_aAtlasBays.IsEmpty();
	}

	protected void PlaceProxy(Vehicle vehicle, vector point, vector heading, BaseWorld world)
	{
		point[1] = world.GetSurfaceY(point[0], point[2]) + 0.1;
		vector matrix[4];
		Math3D.DirectionAndUpMatrix(heading, Vector(0, 1, 0), matrix);
		matrix[3] = point;
		vehicle.SetWorldTransform(matrix);
		vehicle.Update();
		CarControllerComponent car = CarControllerComponent.Cast(vehicle.FindComponent(CarControllerComponent));
		if (car)
		{
			car.SetPersistentHandBrake(true);
			VehicleWheeledSimulation sim = car.GetSimulation();
			if (sim)
			{
				sim.SetThrottle(0);
				sim.SetBreak(1, true);
			}
		}
	}

	protected void StageAtlasSlot(BaseWorld world)
	{
		m_vBay = m_aAtlasBays[m_iAtlasCursor];
		m_vRoadLead = m_aAtlasLeads[m_iAtlasCursor];
		vector park1 = m_aAtlasPark1[m_iAtlasCursor];
		vector park2 = m_aAtlasPark2[m_iAtlasCursor];
		vector delta = park1 - m_vBay;
		float length = vector.DistanceXZ(park1, m_vBay);
		vector axis = Vector(delta[0] / length, 0, delta[2] / length);
		vector south = Vector(-axis[2], 0, axis[0]);
		m_vMeasuredSouthShoulder = m_vRoadLead + south * 10.0;
		vector tangent = m_aAtlasTangents[m_iAtlasCursor];
		PlaceProxy(m_Park1, park1, tangent, world);
		PlaceProxy(m_Park2, park2, tangent, world);
		PlaceProxy(m_Lead, m_vMeasuredSouthShoulder, tangent, world);
		Print("[ConvoyFollower] ROAD_ATLAS_STAGE: TEST_ONLY_STATIC_REPOSITION index=" + m_iAtlasCursor +
			" road=" + m_aAtlasRoadIndex[m_iAtlasCursor] + " bay=" + m_vBay +
			" road_lead=" + m_vRoadLead + " staged_lead=" + m_Lead.GetOrigin() +
			" parked2=" + m_Park2.GetOrigin() + " parked1=" + m_Park1.GetOrigin());
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

	protected int ScanStation(BaseWorld world, string prefix)
	{
		vector bay = Vector(1432.99, 37.0454, 3046.19);
		if (m_bUpstream || m_bAtlas)
			bay = m_vBay;
		vector delta = m_Park1.GetOrigin() - bay;
		float length = vector.DistanceXZ(m_Park1.GetOrigin(), bay);
		if (length < 50.0)
		{
			Print("[ConvoyFollower] ROAD_BYPASS_REJECT: " + prefix + " invalid bay axis");
			return 0;
		}
		vector axis = Vector(delta[0] / length, 0, delta[2] / length);
		vector south = Vector(-axis[2], 0, axis[0]);
		vector roadLead = Vector(1445.63, 36.8553, 3050.31);
		if (m_bUpstream || m_bAtlas)
			roadLead = m_vRoadLead;
		ref array<float> offsets = {6.5, 8.25, 10.0};
		int clearCount = 0;
		Print("[ConvoyFollower] ROAD_BYPASS_SURVEY_BEGIN: " + prefix + " bay=" + bay + " axis=" + axis +
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
				if (sideIndex == 0 && shoulderIndex == 0 && !m_bUpstream && !m_bAtlas)
					shoulder = Vector(1441.25, 36.4462, 3059.3);
				string stageLabel = prefix + " " + sideName + " shoulder=" + shoulderIndex;
				if (m_bAtlas)
				{
					vector front = m_aAtlasFrontStops[m_iAtlasCursor];
					vector approach = m_aAtlasApproachTangents[m_iAtlasCursor];
					vector toShoulder = shoulder - front;
					float ahead = toShoulder[0] * approach[0] + toShoulder[2] * approach[2];
					float lateral = toShoulder[0] * approach[2] - toShoulder[2] * approach[0];
					if (lateral < 0)
						lateral = -lateral;
					bool laneClear = !(ahead > -4.0 && ahead < 30.0 && lateral < 10.0);
					Print("[ConvoyFollower] ROAD_ATLAS_APPROACH_PREFLIGHT: " + stageLabel +
						" expected_front=" + front + " approach=" + approach +
						" lead_ahead=" + ahead + " lead_lateral=" + lateral +
						" lane_clear_with_2m_margin=" + laneClear);
					if (!laneClear)
						continue;
				}
				if (!CheckShoulder(world, roadLead, shoulder, stageLabel))
					continue;
				for (int offsetIndex = 0; offsetIndex < offsets.Count(); offsetIndex++)
				{
					float offset = offsets[offsetIndex];
					if (CheckRoute(world, shoulder, axis, side, offset,
						stageLabel + " offset=" + offset))
						clearCount++;
				}
			}
		}
		return clearCount;
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
		if (m_bAtlas)
		{
			if (!m_bAtlasPrepared)
			{
				if (!PrepareAtlas(world))
				{
					Print("[ConvoyFollower] ROAD_ATLAS_RESULT: FAIL no connected width-8 candidates");
					return;
				}
				m_bAtlasPrepared = true;
			}
			if (m_iAtlasCursor >= m_aAtlasBays.Count() || m_iAtlasClearSlots >= 3)
			{
				Print("[ConvoyFollower] ROAD_ATLAS_RESULT: DONE clear_static_slots=" + m_iAtlasClearSlots +
					" scanned=" + m_iAtlasCursor + " of " + m_aAtlasBays.Count() +
					" (geometry only; physical M923 passing and convoy resume untested)");
				return;
			}
			if (!m_bAtlasSlotStaged)
			{
				StageAtlasSlot(world);
				m_bAtlasSlotStaged = true;
				GetGame().GetCallqueue().CallLater(Survey, 1500, false);
				return;
			}
			int atlasClear = ScanStation(world, "atlas=" + m_iAtlasCursor +
				" road=" + m_aAtlasRoadIndex[m_iAtlasCursor]);
			if (atlasClear > 0)
			{
				m_iAtlasClearSlots++;
				Print("[ConvoyFollower] ROAD_ATLAS_CLEAR_SLOT: index=" + m_iAtlasCursor +
					" road=" + m_aAtlasRoadIndex[m_iAtlasCursor] + " clear_routes=" + atlasClear +
					" bay=" + m_vBay + " road_lead=" + m_vRoadLead +
					" park2=" + m_Park2.GetOrigin() + " park1=" + m_Park1.GetOrigin() +
					" (static geometry only)");
				for (int startStation = 0; startStation < 4; startStation++)
				{
					int startIndex = m_iAtlasCursor * 4 + startStation;
					if (m_aAtlasStartCenters[startIndex] == vector.Zero)
					{
						Print("[ConvoyFollower] ROAD_ATLAS_START_SLOT: candidate=" + m_iAtlasCursor +
							" station=" + startStation + " unavailable: road arc too short");
						continue;
					}
					Print("[ConvoyFollower] ROAD_ATLAS_START_SLOT: candidate=" + m_iAtlasCursor +
						" station=" + startStation + " center=" + m_aAtlasStartCenters[startIndex] +
						" tangent=" + m_aAtlasStartTangents[startIndex] +
						" direct_to_bay=" + vector.DistanceXZ(m_aAtlasStartCenters[startIndex], m_vBay) +
						" (geometry only; physical spawn clearance untested)");
				}
			}
			m_iAtlasCursor++;
			m_bAtlasSlotStaged = false;
			GetGame().GetCallqueue().CallLater(Survey, 100, false);
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
		int clearCount = ScanStation(world, "single");
		Print("[ConvoyFollower] ROAD_BYPASS_SURVEY_RESULT: DONE static_clear_candidates=" + clearCount +
			" (geometry only; no convoy pass or resume tested)");
	}
}
