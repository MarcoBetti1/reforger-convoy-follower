// Test-only side tracking shot for the automated Workbench convoy worlds.
// Attach to CF_SmokeLead only in a smoke scene; production driver prefabs do
// not use this component. The probe still owns the player/passenger session.
class CF_ConvoyFilmCameraComponentClass : ScriptComponentClass
{
}

class CF_ConvoyFilmCameraComponent : ScriptComponent
{
	[Attribute(defvalue: "3", params: "1 3 1", desc: "Last follower vehicle in this smoke scene")]
	protected int m_iExpectedTrucks;
	[Attribute(defvalue: "1", params: "-1 1 1", desc: "Which roadside to film from: 1 or -1")]
	protected int m_iCameraSide;
	[Attribute(defvalue: "0", params: "0 1 1", desc: "Hold a wide unload-bay shot until every truck starts the return route")]
	protected int m_iFilmBayMode;
	protected ref array<IEntity> m_Followers = {};
	protected CF_SmokeProbeComponent m_RouteProbe;
	protected CameraBase m_FilmCamera;
	protected vector m_vBayLead;
	protected vector m_vBayAxis;
	protected bool m_bBayFocus;
	protected bool m_bBayCaptured;
	protected int m_iCameraSetupAttempts;
	protected bool m_bActiveLogged;
	protected int m_iFrameCount;

	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.POSTFRAME);
	}

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		// F5 runs an authoritative world and a local viewport in one process.
		// A dedicated server has no local controller and never changes a view.
		PlayerController local = GetGame().GetPlayerController();
		if (!local || !local.GetControlledEntity() || SCR_EditorManagerEntity.IsOpenedInstance())
			return;
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return;
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		if (m_Followers.Count() != m_iExpectedTrucks)
		{
			m_Followers.Clear();
			for (int index = 1; index <= m_iExpectedTrucks; index++)
			{
				IEntity follower = world.FindEntityByName("CF_SmokeFollower" + index);
				if (follower)
					m_Followers.Insert(follower);
			}
		}
		if (m_Followers.Count() != m_iExpectedTrucks)
			return;
		vector leadPosition = owner.GetOrigin();
		vector tailPosition = m_Followers[0].GetOrigin();
		float farthest = vector.DistanceXZ(leadPosition, tailPosition);
		foreach (IEntity stagedTruck : m_Followers)
		{
			float distance = vector.DistanceXZ(leadPosition, stagedTruck.GetOrigin());
			if (distance > farthest)
			{
				farthest = distance;
				tailPosition = stagedTruck.GetOrigin();
			}
		}
		vector convoyAxis = leadPosition - tailPosition;
		convoyAxis[1] = 0;
		if (convoyAxis.LengthSq() < 9.0)
		{
			convoyAxis = owner.GetWorldTransformAxis(2);
			convoyAxis[1] = 0;
		}
		convoyAxis.Normalize();
		vector roadside = Vector(-convoyAxis[2], 0, convoyAxis[0]);
		float span = vector.DistanceXZ(leadPosition, tailPosition);
		float sideways = span * 0.75 + 30.0;
		if (sideways < 50.0)
			sideways = 50.0;
		if (sideways > 120.0)
			sideways = 120.0;
		float height = span * 0.4 + 18.0;
		if (height < 26.0)
			height = 26.0;
		if (height > 65.0)
			height = 65.0;
		vector target = (leadPosition + tailPosition) * 0.5 + vector.Up * 2.0;
		float sideSign = 1.0;
		if (m_iCameraSide < 0)
			sideSign = -1.0;
		vector eye = target + roadside * (sideways * sideSign) + convoyAxis * 8.0 + vector.Up * height;
		if (m_iFilmBayMode > 0)
		{
			if (!m_RouteProbe)
				m_RouteProbe = CF_SmokeProbeComponent.Cast(owner.FindComponent(CF_SmokeProbeComponent));
			if (!m_bBayCaptured && m_RouteProbe && m_RouteProbe.CF_HasPassedRoadArrival())
			{
				m_bBayCaptured = true;
				m_bBayFocus = true;
				m_vBayLead = leadPosition;
				m_vBayAxis = convoyAxis;
				Print("[ConvoyFollower] AUTO_FILM_BAY: holding unload area at " + m_vBayLead);
			}
			if (m_bBayFocus)
			{
				bool allDeparted = vector.DistanceXZ(leadPosition, m_vBayLead) >= 55.0;
				foreach (IEntity movingTruck : m_Followers)
				{
					if (vector.DistanceXZ(movingTruck.GetOrigin(), m_vBayLead) < 45.0)
						allDeparted = false;
				}
				if (allDeparted)
				{
					m_bBayFocus = false;
					Print("[ConvoyFollower] AUTO_FILM_BAY: all trucks left unload area; following return convoy");
				}
				else
				{
					vector bayRoadside = Vector(-m_vBayAxis[2], 0, m_vBayAxis[0]);
					target = m_vBayLead - m_vBayAxis * 20.0 + vector.Up * 2.0;
					eye = target + bayRoadside * (80.0 * sideSign) + m_vBayAxis * 8.0 + vector.Up * 45.0;
				}
			}
		}
		vector cameraMatrix[4];
		Math3D.DirectionAndUpMatrix((target - eye).Normalized(), vector.Up, cameraMatrix);
		cameraMatrix[3] = eye;

		// A PlayerCamera reapplies its own chase transform later in the frame.
		// Use a standalone camera entity with no player-camera update instead.
		if (!m_FilmCamera && m_iCameraSetupAttempts < 20)
		{
			m_iCameraSetupAttempts++;
			m_FilmCamera = CameraBase.Cast(GetGame().SpawnEntity(CameraBase, world));
			if (!m_FilmCamera)
			{
				Print("[ConvoyFollower] AUTO_FILM_CAMERA_SETUP_FAIL: spawn attempt=" + m_iCameraSetupAttempts);
				return;
			}
			m_FilmCamera.SetWorldTransform(cameraMatrix);
			m_FilmCamera.SetFOVDegree(55.0);
			bool selected = cameraManager.SetCamera(m_FilmCamera);
			Print("[ConvoyFollower] AUTO_FILM_CAMERA_SETUP: selected=" + selected +
				" index=" + m_FilmCamera.GetCameraIndex());
			if (!selected)
			{
				delete m_FilmCamera;
				m_FilmCamera = null;
				return;
			}
		}
		if (!m_FilmCamera)
			return;
		if (cameraManager.CurrentCamera() != m_FilmCamera)
		{
			bool reselected = cameraManager.SetCamera(m_FilmCamera);
			if (!reselected)
				return;
		}
		m_FilmCamera.SetWorldTransform(cameraMatrix);
		m_FilmCamera.ApplyTransform(timeSlice);

		m_iFrameCount++;
		if (!m_bActiveLogged || m_iFrameCount % 300 == 0)
		{
			m_bActiveLogged = true;
			Print("[ConvoyFollower] AUTO_FILM_CAMERA: index=" + m_FilmCamera.GetCameraIndex() +
				" current=" + (cameraManager.CurrentCamera() == m_FilmCamera) + " lead=" + leadPosition +
				" tail=" + tailPosition + " eye=" + eye + " span=" + span);
		}
	}
}
