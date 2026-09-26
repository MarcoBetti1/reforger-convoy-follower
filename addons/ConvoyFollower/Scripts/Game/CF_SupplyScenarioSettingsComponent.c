class CF_SupplyScenarioSettingsComponentClass : ScriptComponentClass
{
}

// Attaching this component explicitly enables native supplies for this scenario.
// It does not create resources, perform actions or change player/vehicle control.
class CF_SupplyScenarioSettingsComponent : ScriptComponent
{
	protected World m_World;
	protected SCR_BaseGameMode m_Mode;
	protected int m_iPolls;
	protected bool m_bRequested;
	protected bool m_bFinished;

	override void OnPostInit(IEntity owner)
	{
		if (!GetGame() || !GetGame().InPlayMode() || !Replication.IsServer()) return;
		m_World = GetGame().GetWorld();
		if (m_World) GetGame().GetCallqueue().CallLater(Poll, 250, true);
	}

	protected void Finish(string reason, bool enabled)
	{
		if (m_bFinished) return;
		m_bFinished = true;
		if (GetGame()) GetGame().GetCallqueue().Remove(Poll);
		Print("[ConvoyFollower] SUPPLY_SCENARIO_SETTINGS: reason=" + reason +
			" enabled=" + enabled + " requested=" + m_bRequested + " polls=" + m_iPolls);
	}

	protected void Poll()
	{
		if (m_bFinished) return;
		if (!GetGame() || GetGame().GetWorld() != m_World || !GetGame().InPlayMode())
		{ Finish("runtime_ended", false); return; }
		if (!Replication.IsServer() || CF_ConvoySession.CF_IsWorldCleanup())
		{ Finish("authority_or_world_cleanup", false); return; }
		m_iPolls++;
		SCR_BaseGameMode mode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (m_Mode && mode != m_Mode)
		{ Finish("game_mode_changed", false); return; }
		if (mode && mode.IsMaster())
		{
			m_Mode = mode;
			if (mode.IsResourceTypeEnabled(EResourceType.SUPPLIES))
			{ Finish("native_supplies_enabled", true); return; }
			if (!m_bRequested)
			{
				m_bRequested = true;
				mode.SetResourceTypeEnabled(true, EResourceType.SUPPLIES);
				Print("[ConvoyFollower] SUPPLY_SCENARIO_SETTINGS_REQUEST: native_setting=true resource_value_writes=false");
				// A later poll must observe the result. Never repeat the request.
				return;
			}
		}
		if (m_iPolls >= 120) Finish("startup_timeout", false);
	}

	override void OnDelete(IEntity owner)
	{
		m_bFinished = true;
		if (GetGame()) GetGame().GetCallqueue().Remove(Poll);
		m_Mode = null;
		m_World = null;
	}
}
