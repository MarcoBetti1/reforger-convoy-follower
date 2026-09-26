// Per-driver subscriptions to the installed player controller's real
// pre-possession event. A periodic refresh also covers later joining players.
// This observer never changes player control or native AI activation.
class CF_ControlHandoverWatch
{
	protected CF_DriverControllerComponent m_Owner;
	protected SCR_BaseGameMode m_GameMode;
	protected ref array<SCR_PlayerController> m_Controllers = {};
	protected float m_fRefreshSeconds;

	void Start(CF_DriverControllerComponent owner)
	{
		Stop();
		m_Owner = owner;
		m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (m_GameMode)
			m_GameMode.GetOnPlayerRegistered().Insert(OnPlayerRegistered);
		Refresh();
	}

	protected void OnPlayerRegistered(int playerId)
	{
		Refresh();
	}

	protected void BeforePossess(IEntity entity)
	{
		if (m_Owner)
			m_Owner.CF_OnBeforePlayerPossess(entity);
	}

	protected void ControlledEntityChanged(IEntity from, IEntity to)
	{
		if (m_Owner)
			m_Owner.CF_OnPlayerControlChanged(from, to);
	}

	protected void Refresh()
	{
		if (!m_Owner || !GetGame() || CF_ConvoySession.CF_IsWorldCleanup())
			return;
		PlayerManager players = GetGame().GetPlayerManager();
		if (!players)
			return;
		ref array<int> ids = {};
		ref array<SCR_PlayerController> active = {};
		players.GetPlayers(ids);
		foreach (int id : ids)
		{
			SCR_PlayerController controller = SCR_PlayerController.Cast(players.GetPlayerController(id));
			if (!controller)
				continue;
			active.Insert(controller);
			if (m_Controllers.Contains(controller))
				continue;
			controller.m_OnBeforePossess.Insert(BeforePossess);
			controller.m_OnControlledEntityChanged.Insert(ControlledEntityChanged);
			m_Controllers.Insert(controller);
		}
		for (int index = m_Controllers.Count() - 1; index >= 0; index--)
		{
			SCR_PlayerController previous = m_Controllers[index];
			if (previous && active.Contains(previous))
				continue;
			if (previous)
			{
				previous.m_OnBeforePossess.Remove(BeforePossess);
				previous.m_OnControlledEntityChanged.Remove(ControlledEntityChanged);
			}
			m_Controllers.RemoveOrdered(index);
		}
	}

	void Update(float elapsed)
	{
		m_fRefreshSeconds += elapsed;
		if (m_fRefreshSeconds < 1.0)
			return;
		m_fRefreshSeconds = 0;
		Refresh();
	}

	void Stop()
	{
		if (m_GameMode)
			m_GameMode.GetOnPlayerRegistered().Remove(OnPlayerRegistered);
		foreach (SCR_PlayerController controller : m_Controllers)
		{
			if (!controller)
				continue;
			controller.m_OnBeforePossess.Remove(BeforePossess);
			controller.m_OnControlledEntityChanged.Remove(ControlledEntityChanged);
		}
		m_Controllers.Clear();
		m_GameMode = null;
		m_Owner = null;
		m_fRefreshSeconds = 0;
	}
}
