class CF_DaylightScenarioSetupComponentClass : ScriptComponentClass {}

// Explicit environment setup for opted-in local journey scenes. It neither
// recruits drivers nor changes vehicle controls, cargo or command authority.
class CF_DaylightScenarioSetupComponent : ScriptComponent
{
	protected bool m_bDone;
	protected World m_World;
	protected float m_fStartedMs;

	override void OnPostInit(IEntity owner)
	{
		if (!GetGame() || !GetGame().InPlayMode() || !Replication.IsServer()) return;
		m_World = GetGame().GetWorld();
		m_fStartedMs = m_World.GetWorldTime();
		SetEventMask(owner, EntityEvent.POSTFRAME);
	}

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (m_bDone || !GetGame() || !GetGame().InPlayMode() || !Replication.IsServer() ||
			GetGame().GetWorld() != m_World || CF_ConvoySession.CF_IsWorldCleanup()) return;
		ChimeraWorld world = GetGame().GetWorld();
		TimeAndWeatherManagerEntity weather = world.GetTimeAndWeatherManager();
		if (weather && weather.SetTimeOfTheDay(12.0, true))
		{
			m_bDone = true;
			Print("[ConvoyFollower] JOURNEY_DAYLIGHT: requested_hour=12 readback_hour=" + weather.GetTimeOfTheDay() +
				" environment_only=true automatic_driving=false automatic_commands=false cargo_writes=false");
		}
		else if (m_World.GetWorldTime() - m_fStartedMs > 10000.0)
		{
			m_bDone = true;
			Print("[ConvoyFollower] JOURNEY_DAYLIGHT: FAIL reason=weather_manager_or_request_unavailable");
		}
	}
}
