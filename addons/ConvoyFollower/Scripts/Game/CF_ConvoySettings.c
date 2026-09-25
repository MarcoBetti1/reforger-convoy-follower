// Author/mission-maker defaults live in Configs/CF_ConvoySettings.conf.
// A dedicated server may override them at startup through its profile JSON.
// Neither path creates an in-game player menu.
[BaseContainerProps(configRoot: true)]
class CF_ConvoySettings : ScriptAndConfig
{
	protected static const ResourceName CF_SETTINGS_RESOURCE = "{A736F034F11893C2}Configs/CF_ConvoySettings.conf";
	protected static const string CF_PROFILE_OVERRIDE = "$profile:ConvoyFollowerSettings.json";
	protected static ref CF_ConvoySettings s_Settings;

	[Attribute(defvalue: "12", params: "6 40 0.5", category: "Spacing", desc: "Moving waypoint completion radius in metres. Road routing and vehicle collision may make the visible gap larger.")]
	float m_fMovingGap;
	[Attribute(defvalue: "10", params: "4 30 0.5", category: "Spacing", desc: "Completion radius used for the final approach to a stopped predecessor, in metres. Must not exceed moving gap.")]
	float m_fStoppedGap;
	[Attribute(defvalue: "35", params: "5 100 1", category: "Formation", desc: "Search radius in metres for the closest empty wheeled vehicle when assigning a driver.")]
	float m_fTruckSearchRadius;
	[Attribute(defvalue: "5", params: "1 5 1", category: "Formation", desc: "Hard maximum number of driver vehicles in one player's convoy (one through five).")]
	int m_iMaxConvoyUnits;

	[Attribute(defvalue: "180", params: "40 900 1", category: "Separation", desc: "Distance in metres from the preceding vehicle that starts a sustained range warning.")]
	float m_fRangeWarningDistance;
	[Attribute(defvalue: "3", params: "1 30 0.5", category: "Separation", desc: "Seconds beyond warning distance before the warning call is emitted.")]
	float m_fRangeWarningSeconds;
	[Attribute(defvalue: "140", params: "20 850 1", category: "Separation", desc: "Distance in metres below which a range warning can be issued again.")]
	float m_fRangeWarningRearmDistance;
	[Attribute(defvalue: "275", params: "60 1000 1", category: "Separation", desc: "Distance in metres beyond which a unit becomes lost after the grace time.")]
	float m_fLostDistance;
	[Attribute(defvalue: "10", params: "1 60 0.5", category: "Separation", desc: "Seconds beyond lost distance before the unit stops following.")]
	float m_fLostGraceSeconds;
	[Attribute(defvalue: "100", params: "10 800 1", category: "Separation", desc: "Distance in metres within which a lost unit may rejoin.")]
	float m_fRejoinDistance;

	[Attribute(defvalue: "40", params: "15 300 1", category: "Recovery", desc: "A stationary truck this far behind its predecessor enters the stall retry path.")]
	float m_fStuckLeadDistance;
	[Attribute(defvalue: "30", params: "5 90 1", category: "Recovery", desc: "Seconds between stall checks while following.")]
	float m_fStuckCheckSeconds;
	[Attribute(defvalue: "3", params: "1 6 1", category: "Recovery", desc: "Consecutive stalled checks before the unit stands down and the convoy rewires.")]
	int m_iStallMaxChecks;
	[Attribute(defvalue: "12", params: "3 45 1", category: "Recovery", desc: "Seconds allowed for each attempt to reboard after an unexpected dismount.")]
	float m_fReboardRetrySeconds;
	[Attribute(defvalue: "3", params: "1 6 1", category: "Recovery", desc: "Maximum attempts to reboard the assigned driver's seat.")]
	int m_iReboardMaxAttempts;

	[Attribute(defvalue: "1", category: "Radio", desc: "Enable private convoy voice calls for every player. Dedicated server owners can override this from the server profile JSON without rebuilding the addon.")]
	bool m_bVoiceEnabled;
	[Attribute(defvalue: "0", params: "0 1 1", category: "Radio", desc: "Convoy leader voice pack: 0 = original player-recorded voice, 1 = generated alternate leader. Dedicated servers can override from the profile JSON.")]
	int m_iVoicePack;
	[Attribute(defvalue: "40", params: "0 100 1", category: "Radio", desc: "Percent chance to play each routine Unit One ready, following, or holding call. First exception reports do not use this chance.")]
	int m_iRoutineCallChancePercent;
	[Attribute(defvalue: "45", params: "0 300 1", category: "Radio", desc: "Minimum seconds before repeating the same routine call for the same unit.")]
	float m_fRoutineRepeatSeconds;
	[Attribute(defvalue: "15", params: "0 120 1", category: "Radio", desc: "Minimum seconds between any two routine calls to the same player.")]
	float m_fRoutineSpacingSeconds;
	[Attribute(defvalue: "45", params: "0 300 1", category: "Radio", desc: "Suppress rapid repeats of range-warning and rejoined calls for the same unit. Stuck, lost, and under-fire reports always pass.")]
	float m_fExceptionRepeatSeconds;

	static CF_ConvoySettings Get()
	{
		if (s_Settings)
			return s_Settings;

		s_Settings = SCR_ConfigHelperT<CF_ConvoySettings>.GetConfigObject(CF_SETTINGS_RESOURCE);

		if (!s_Settings)
		{
			Print("[ConvoyFollower] SETTINGS_FALLBACK: config resource unavailable; using built-in defaults");
			s_Settings = new CF_ConvoySettings();
			s_Settings.SetDefaults();
		}

		if (Replication.IsServer())
			s_Settings.LoadProfileOverrides();
		s_Settings.Validate();
		return s_Settings;
	}

	protected static float ProfileFloat(JsonLoadContext context, string key, float previous)
	{
		float value;
		if (context.ReadValue(key, value))
			return value;
		return previous;
	}

	protected static int ProfileInt(JsonLoadContext context, string key, int previous)
	{
		int value;
		if (context.ReadValue(key, value))
			return value;
		return previous;
	}

	protected static bool ProfileBool(JsonLoadContext context, string key, bool previous)
	{
		bool value;
		if (context.ReadValue(key, value))
			return value;
		return previous;
	}

	// Optional per-server override, loaded once at startup from the -profile
	// directory. The installed addon is never modified by a server operator.
	protected void LoadProfileOverrides()
	{
		if (!FileIO.FileExists(CF_PROFILE_OVERRIDE))
			return;
		ref JsonLoadContext context = new JsonLoadContext();
		if (!context.LoadFromFile(CF_PROFILE_OVERRIDE))
		{
			Print("[ConvoyFollower] SETTINGS_PROFILE_INVALID: could not parse $profile:ConvoyFollowerSettings.json");
			return;
		}

		m_fMovingGap = ProfileFloat(context, "m_fMovingGap", m_fMovingGap);
		m_fStoppedGap = ProfileFloat(context, "m_fStoppedGap", m_fStoppedGap);
		m_fTruckSearchRadius = ProfileFloat(context, "m_fTruckSearchRadius", m_fTruckSearchRadius);
		m_iMaxConvoyUnits = ProfileInt(context, "m_iMaxConvoyUnits", m_iMaxConvoyUnits);
		m_fRangeWarningDistance = ProfileFloat(context, "m_fRangeWarningDistance", m_fRangeWarningDistance);
		m_fRangeWarningSeconds = ProfileFloat(context, "m_fRangeWarningSeconds", m_fRangeWarningSeconds);
		m_fRangeWarningRearmDistance = ProfileFloat(context, "m_fRangeWarningRearmDistance", m_fRangeWarningRearmDistance);
		m_fLostDistance = ProfileFloat(context, "m_fLostDistance", m_fLostDistance);
		m_fLostGraceSeconds = ProfileFloat(context, "m_fLostGraceSeconds", m_fLostGraceSeconds);
		m_fRejoinDistance = ProfileFloat(context, "m_fRejoinDistance", m_fRejoinDistance);
		m_fStuckLeadDistance = ProfileFloat(context, "m_fStuckLeadDistance", m_fStuckLeadDistance);
		m_fStuckCheckSeconds = ProfileFloat(context, "m_fStuckCheckSeconds", m_fStuckCheckSeconds);
		m_iStallMaxChecks = ProfileInt(context, "m_iStallMaxChecks", m_iStallMaxChecks);
		m_fReboardRetrySeconds = ProfileFloat(context, "m_fReboardRetrySeconds", m_fReboardRetrySeconds);
		m_iReboardMaxAttempts = ProfileInt(context, "m_iReboardMaxAttempts", m_iReboardMaxAttempts);
		m_bVoiceEnabled = ProfileBool(context, "m_bVoiceEnabled", m_bVoiceEnabled);
		m_iVoicePack = ProfileInt(context, "m_iVoicePack", m_iVoicePack);
		m_iRoutineCallChancePercent = ProfileInt(context, "m_iRoutineCallChancePercent", m_iRoutineCallChancePercent);
		m_fRoutineRepeatSeconds = ProfileFloat(context, "m_fRoutineRepeatSeconds", m_fRoutineRepeatSeconds);
		m_fRoutineSpacingSeconds = ProfileFloat(context, "m_fRoutineSpacingSeconds", m_fRoutineSpacingSeconds);
		m_fExceptionRepeatSeconds = ProfileFloat(context, "m_fExceptionRepeatSeconds", m_fExceptionRepeatSeconds);
		Print("[ConvoyFollower] SETTINGS_PROFILE_APPLIED: voice " + m_bVoiceEnabled + ", pack " + m_iVoicePack);
	}

	protected void SetDefaults()
	{
		m_fMovingGap = 12.0;
		m_fStoppedGap = 10.0;
		m_fTruckSearchRadius = 35.0;
		m_iMaxConvoyUnits = 5;
		m_fRangeWarningDistance = 180.0;
		m_fRangeWarningSeconds = 3.0;
		m_fRangeWarningRearmDistance = 140.0;
		m_fLostDistance = 275.0;
		m_fLostGraceSeconds = 10.0;
		m_fRejoinDistance = 100.0;
		m_fStuckLeadDistance = 40.0;
		m_fStuckCheckSeconds = 30.0;
		m_iStallMaxChecks = 3;
		m_fReboardRetrySeconds = 12.0;
		m_iReboardMaxAttempts = 3;
		m_bVoiceEnabled = true;
		m_iVoicePack = 0;
		m_iRoutineCallChancePercent = 40;
		m_fRoutineRepeatSeconds = 45.0;
		m_fRoutineSpacingSeconds = 15.0;
		m_fExceptionRepeatSeconds = 45.0;
	}

	protected static float ClampFloat(float value, float minimum, float maximum)
	{
		if (value < minimum)
			return minimum;
		if (value > maximum)
			return maximum;
		return value;
	}

	protected static int ClampInt(int value, int minimum, int maximum)
	{
		if (value < minimum)
			return minimum;
		if (value > maximum)
			return maximum;
		return value;
	}

	protected void Validate()
	{
		m_fMovingGap = ClampFloat(m_fMovingGap, 6.0, 40.0);
		m_fStoppedGap = ClampFloat(m_fStoppedGap, 4.0, m_fMovingGap);
		m_fTruckSearchRadius = ClampFloat(m_fTruckSearchRadius, 5.0, 100.0);
		m_iMaxConvoyUnits = ClampInt(m_iMaxConvoyUnits, 1, 5);

		m_fRangeWarningDistance = ClampFloat(m_fRangeWarningDistance, 40.0, 900.0);
		m_fRangeWarningSeconds = ClampFloat(m_fRangeWarningSeconds, 1.0, 30.0);
		m_fRangeWarningRearmDistance = ClampFloat(m_fRangeWarningRearmDistance, 20.0, m_fRangeWarningDistance - 10.0);
		m_fRejoinDistance = ClampFloat(m_fRejoinDistance, 10.0, m_fRangeWarningRearmDistance);
		m_fLostDistance = ClampFloat(m_fLostDistance, m_fRangeWarningDistance + 20.0, 1000.0);
		m_fLostGraceSeconds = ClampFloat(m_fLostGraceSeconds, 1.0, 60.0);

		m_fStuckLeadDistance = ClampFloat(m_fStuckLeadDistance, m_fMovingGap + 5.0, 300.0);
		m_fStuckCheckSeconds = ClampFloat(m_fStuckCheckSeconds, 5.0, 90.0);
		m_iStallMaxChecks = ClampInt(m_iStallMaxChecks, 1, 6);
		m_fReboardRetrySeconds = ClampFloat(m_fReboardRetrySeconds, 3.0, 45.0);
		m_iReboardMaxAttempts = ClampInt(m_iReboardMaxAttempts, 1, 6);
		m_iRoutineCallChancePercent = ClampInt(m_iRoutineCallChancePercent, 0, 100);
		m_iVoicePack = ClampInt(m_iVoicePack, 0, 1);
		m_fRoutineRepeatSeconds = ClampFloat(m_fRoutineRepeatSeconds, 0.0, 300.0);
		m_fRoutineSpacingSeconds = ClampFloat(m_fRoutineSpacingSeconds, 0.0, 120.0);
		m_fExceptionRepeatSeconds = ClampFloat(m_fExceptionRepeatSeconds, 0.0, 300.0);
	}
}
