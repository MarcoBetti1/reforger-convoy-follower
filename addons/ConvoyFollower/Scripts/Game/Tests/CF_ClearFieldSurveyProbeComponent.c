// Test-only geometry survey: no convoy orders, no vehicle reposition and no
// driving PASS. Candidate coordinates use the live world's GetSurfaceY.
class CF_ClearFieldSurveyProbeComponentClass : ScriptComponentClass
{
}

class CF_ClearFieldSurveyProbeComponent : ScriptComponent
{
	protected IEntity m_Anchor;
	protected RoadNetworkManager m_Roads;
	protected ref array<vector> m_Origins = {};
	protected ref array<vector> m_Directions = {};
	protected int m_iCandidate;
	protected int m_iCandidatesFound;
	protected int m_iWaitTicks;
	protected bool m_bOccupied;
	protected IEntity m_Obstacle;
	protected string m_sObstacleKind;
	protected float m_fBestScore = -100000.0;
	protected vector m_vBestLead;
	protected vector m_vBestFollower;
	protected vector m_vBestGoal;
	protected vector m_vBestDirection;
	protected CameraBase m_Camera;
	protected int m_iCameraAttempts;
	protected bool m_bFinished;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Anchor = owner;
		// Road81's meadow is visible in prior live footage, but no particular
		// field lane is considered usable until this actual-world survey.
		ref array<float> xs = {1440.0, 1500.0, 1560.0, 1620.0};
		ref array<float> zs = {2940.0, 2990.0, 3110.0, 3160.0, 3210.0};
		foreach (float x : xs)
		{
			foreach (float z : zs)
				m_Origins.Insert(Vector(x, 0, z));
		}
		ref array<vector> directions = {Vector(1, 0, 0), Vector(0.707107, 0, 0.707107),
			Vector(0, 0, 1), Vector(-0.707107, 0, 0.707107), Vector(-1, 0, 0),
			Vector(-0.707107, 0, -0.707107), Vector(0, 0, -1), Vector(0.707107, 0, -0.707107)};
		m_Directions = directions;
		Print("[ConvoyFollower] FIELD_SURVEY_INIT: world=Arland_ClearFieldSurvey candidates=160 drive_length=80 follower_offset=20 geometry_only=true");
		GetGame().GetCallqueue().CallLater(PollSurvey, 1000, true);
	}

	override void OnDelete(IEntity owner)
	{
		if (GetGame())
		{
			GetGame().GetCallqueue().Remove(PollSurvey);
			GetGame().GetCallqueue().Remove(ShowBestCandidate);
		}
		m_Camera = null;
	}

	protected vector OnSurface(vector point)
	{
		point[1] = GetGame().GetWorld().GetSurfaceY(point[0], point[2]);
		return point;
	}

	protected string Describe(IEntity entity)
	{
		if (!entity)
			return "terrain-or-unknown";
		string result = entity.GetName() + " origin=" + entity.GetOrigin();
		EntityPrefabData prefab = entity.GetPrefabData();
		if (prefab)
			result += " prefab=" + prefab.GetPrefabName();
		return result;
	}

	protected bool InspectNearby(IEntity entity)
	{
		if (!entity || entity == m_Anchor)
			return true;
		if (Vehicle.Cast(entity))
		{
			m_bOccupied = true;
			m_Obstacle = entity;
			m_sObstacleKind = "vehicle";
		}
		EntityPrefabData prefab = entity.GetPrefabData();
		if (prefab && prefab.GetPrefabName().IndexOf("Vegetation/Tree/") >= 0)
		{
			m_bOccupied = true;
			m_Obstacle = entity;
			m_sObstacleKind = "tree-near-lane";
		}
		return true;
	}

	protected bool Reject(int id, string reason, vector point)
	{
		Print("[ConvoyFollower] FIELD_SURVEY_REJECT: candidate=" + id + " point=" + point + " reason=" + reason);
		return false;
	}

	protected bool SurveyCandidate(int id, vector start, vector direction)
	{
		BaseWorld world = GetGame().GetWorld();
		vector side = Vector(-direction[2], 0, direction[0]);
		vector previous;
		float minRoadMargin = 100000.0;
		float maxGrade;
		float maxCrossfall;
		// Survey truck-length margins beyond follower start and lead goal.
		// Every sample, not just the goal, must be truly outside mapped roads.
		for (int sampleIndex = 0; sampleIndex <= 23; sampleIndex++)
		{
			float station = -25.0 + sampleIndex * 5.0;
			vector point = OnSurface(start + direction * station);
			if (point[1] < 3.0)
				return Reject(id, "near-sea-level", point);
			BaseRoad road;
			float roadGap;
			int roadId = m_Roads.GetClosestRoad(point, road, roadGap);
			if (!road || road.GetWidth() <= 0.0)
				return Reject(id, "no-road-measurement", point);
			float requiredGap = road.GetWidth() * 0.5 + 8.0;
			float roadMargin = roadGap - requiredGap;
			if (roadMargin < 2.0)
				return Reject(id, "mapped-road-margin-below-2m gap=" + roadGap + " required=" + requiredGap, point);
			if (roadMargin < minRoadMargin)
				minRoadMargin = roadMargin;
			vector left = OnSurface(point - side * 3.0);
			vector right = OnSurface(point + side * 3.0);
			float crossfall = Math.AbsFloat(right[1] - left[1]) / 6.0;
			if (crossfall > 0.10)
				return Reject(id, "crossfall-above-10-percent value=" + crossfall, point);
			if (crossfall > maxCrossfall)
				maxCrossfall = crossfall;
			if (sampleIndex > 0)
			{
				float grade = Math.AbsFloat(point[1] - previous[1]) / 5.0;
				if (grade > 0.12)
					return Reject(id, "grade-above-12-percent value=" + grade, point);
				if (grade > maxGrade)
					maxGrade = grade;
				TraceBox sweep = new TraceBox();
				sweep.Start = previous;
				sweep.End = point;
				sweep.Mins = Vector(-1.8, 0.35, -1.8);
				sweep.Maxs = Vector(1.8, 3.5, 1.8);
				sweep.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
				sweep.Exclude = m_Anchor;
				float fraction = world.TraceMove(sweep, null);
				if (fraction < 0.995)
					return Reject(id, "vehicle-volume-sweep fraction=" + fraction + " hit=" + Describe(sweep.TraceEnt), point);
			}
			m_bOccupied = false;
			m_Obstacle = null;
			world.QueryEntitiesBySphere(point + Vector(0, 1, 0), 7.0, InspectNearby);
			if (m_bOccupied)
				return Reject(id, m_sObstacleKind + " hit=" + Describe(m_Obstacle), point);
			TraceParam overhead = new TraceParam();
			overhead.Start = point + Vector(0, 75, 0);
			overhead.End = point + Vector(0, 3.6, 0);
			overhead.Flags = TraceFlags.ENTS;
			overhead.Exclude = m_Anchor;
			float visible = world.TraceMove(overhead, null);
			if (visible < 0.995)
				return Reject(id, "overhead-camera-obstructed hit=" + Describe(overhead.TraceEnt), point);
			previous = point;
		}
		vector lead = OnSurface(start);
		vector follower = OnSurface(start - direction * 20.0);
		vector goal = OnSurface(start + direction * 80.0);
		float score = minRoadMargin - maxGrade * 100.0 - maxCrossfall * 100.0;
		m_iCandidatesFound++;
		Print("[ConvoyFollower] FIELD_SURVEY_CANDIDATE: candidate=" + id + " lead=" + lead +
			" follower=" + follower + " goal=" + goal + " direction=" + direction +
			" drive_length=80 corridor_length=115 min_offroad_margin=" + minRoadMargin +
			" max_grade=" + maxGrade + " max_crossfall=" + maxCrossfall +
			" vehicle_sweep_clear=true overhead_clear=true geometry_only=true");
		if (score > m_fBestScore)
		{
			m_fBestScore = score;
			m_vBestLead = lead;
			m_vBestFollower = follower;
			m_vBestGoal = goal;
			m_vBestDirection = direction;
		}
		return true;
	}

	protected void PollSurvey()
	{
		if (m_bFinished)
			return;
		m_iWaitTicks++;
		SCR_BaseGameMode mode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!mode || mode.GetState() != SCR_EGameModeState.GAME || !aiWorld || !aiWorld.GetRoadNetworkManager())
		{
			if (m_iWaitTicks > 35)
			{
				m_bFinished = true;
				GetGame().GetCallqueue().Remove(PollSurvey);
				Print("[ConvoyFollower] FIELD_SURVEY_RESULT: FAIL GAME/road network unavailable; no driving attempted");
			}
			return;
		}
		m_Roads = aiWorld.GetRoadNetworkManager();
		int total = m_Origins.Count() * m_Directions.Count();
		// Bounded batches keep the renderer and input responsive during survey.
		for (int batch = 0; batch < 8 && m_iCandidate < total; batch++)
		{
			int originIndex = m_iCandidate / m_Directions.Count();
			int directionIndex = m_iCandidate % m_Directions.Count();
			SurveyCandidate(m_iCandidate, m_Origins[originIndex], m_Directions[directionIndex]);
			m_iCandidate++;
		}
		if (m_iCandidate < total)
			return;
		m_bFinished = true;
		GetGame().GetCallqueue().Remove(PollSurvey);
		if (m_iCandidatesFound == 0)
		{
			Print("[ConvoyFollower] FIELD_SURVEY_RESULT: FAIL no clear field lane candidates=160; no driving attempted");
			return;
		}
		Print("[ConvoyFollower] FIELD_SURVEY_RESULT: CANDIDATES_FOUND count=" + m_iCandidatesFound +
			" best_lead=" + m_vBestLead + " best_follower=" + m_vBestFollower +
			" best_goal=" + m_vBestGoal + " best_direction=" + m_vBestDirection +
			" geometry_only=true physical_driving_unproved=true");
		GetGame().GetCallqueue().CallLater(ShowBestCandidate, 1000, true);
	}

	protected void ShowBestCandidate()
	{
		m_iCameraAttempts++;
		if (m_iCameraAttempts > 10)
		{
			GetGame().GetCallqueue().Remove(ShowBestCandidate);
			return;
		}
		if (!GetGame().GetPlayerController() || !GetGame().GetCameraManager())
			return;
		if (SCR_EditorManagerEntity.IsOpenedInstance() && !SCR_EditorManagerEntity.CloseInstance())
			return;
		if (!m_Camera)
			m_Camera = CameraBase.Cast(GetGame().SpawnEntity(CameraBase, GetGame().GetWorld()));
		if (!m_Camera)
			return;
		vector target = (m_vBestFollower + m_vBestGoal) * 0.5 + Vector(0, 1, 0);
		vector eye = target + Vector(0, 90, 0);
		vector matrix[4];
		Math3D.DirectionAndUpMatrix(Vector(0, -1, 0), m_vBestDirection, matrix);
		matrix[3] = eye;
		m_Camera.SetWorldTransform(matrix);
		m_Camera.SetFOVDegree(72.0);
		bool selected = GetGame().GetCameraManager().SetCamera(m_Camera);
		Print("[ConvoyFollower] FIELD_SURVEY_CAMERA: selected=" + selected + " eye=" + eye +
			" target=" + target + " survey_view_only=true");
		if (selected)
			GetGame().GetCallqueue().Remove(ShowBestCandidate);
	}
}
