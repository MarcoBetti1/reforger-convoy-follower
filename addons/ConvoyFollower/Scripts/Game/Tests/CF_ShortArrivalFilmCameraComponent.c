// Fixed, elevated test-only camera for the visible road81 arrival fixture.
// It changes the local viewport only; no vehicle transform is changed.
class CF_ShortArrivalFilmCameraComponentClass : ScriptComponentClass
{
}

class CF_ShortArrivalFilmCameraComponent : ScriptComponent
{
	protected CameraBase m_Camera;
	protected bool m_bLogged;

	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.POSTFRAME);
	}

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		PlayerController local = GetGame().GetPlayerController();
		if (!local || !local.GetControlledEntity() || SCR_EditorManagerEntity.IsOpenedInstance())
			return;
		CameraManager manager = GetGame().GetCameraManager();
		BaseWorld world = GetGame().GetWorld();
		if (!manager || !world)
			return;
		// Index9 is beside the open junction. This south-side shot includes
		// approach, bay, and second-leg segment; visibility is a live test gate.
		vector target = Vector(1460.0, 39.0, 3055.0);
		vector eye = Vector(1493.0, 126.0, 2949.0);
		vector transform[4];
		Math3D.DirectionAndUpMatrix((target - eye).Normalized(), vector.Up, transform);
		transform[3] = eye;
		if (!m_Camera)
		{
			m_Camera = CameraBase.Cast(GetGame().SpawnEntity(CameraBase, world));
			if (!m_Camera)
				return;
			m_Camera.SetFOVDegree(62.0);
			m_Camera.SetWorldTransform(transform);
		}
		if (manager.CurrentCamera() != m_Camera && !manager.SetCamera(m_Camera))
			return;
		m_Camera.SetWorldTransform(transform);
		m_Camera.ApplyTransform(timeSlice);
		if (!m_bLogged)
		{
			m_bLogged = true;
			Print("[ConvoyFollower] SHORT_ARRIVAL_FILM_CAMERA: fixed=true eye=" + eye +
				" target=" + target + " current=" + (manager.CurrentCamera() == m_Camera));
		}
	}
}
