// Fixed candidate from the live Arland field survey. All physical driving,
// owner/passenger, AI pilot, production follower and acceptance logic is inherited.
class CF_ArlandClearFieldOffroadProbeComponentClass : CF_EveronOffroadProbeComponentClass
{
}

class CF_ArlandClearFieldOffroadProbeComponent : CF_EveronOffroadProbeComponent
{
	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Lead = Vehicle.Cast(owner);
		SetEventMask(owner, EntityEvent.POSTFRAME);
		CF_ConvoySettings.Get();
		Print("[ConvoyFollower] OFFROAD_INIT: expected=1 world=ConvoyFollower_Arland_ClearField_Offroad_1Truck hold_restart=true");
		GetGame().GetCallqueue().CallLater(Poll, 1000, true);
	}

	override protected bool SelectRoute()
	{
		m_vRouteStart = m_Lead.GetOrigin();
		if (vector.DistanceXZ(m_vRouteStart, Vector(1620, 38.5938, 2940)) > 2.0 ||
			vector.DistanceXZ(m_Follower.GetOrigin(), Vector(1600, 38.0312, 2940)) > 2.0)
		{
			Print("[ConvoyFollower] OFFROAD_SURVEY_REJECT: candidate=120 reason=authored_start_displaced lead=" +
				m_vRouteStart + " follower=" + m_Follower.GetOrigin());
			return false;
		}
		m_vRouteAxis = Vector(1, 0, 0);
		m_fSelectedRouteLength = 80.0;
		float offroadRun;
		if (!SurveyCandidate(m_vRouteAxis, m_fSelectedRouteLength, 120, offroadRun))
			return false;
		m_vRouteGoal = m_vRouteStart + m_vRouteAxis * m_fSelectedRouteLength;
		m_vRouteGoal[1] = GetGame().GetWorld().GetSurfaceY(m_vRouteGoal[0], m_vRouteGoal[2]);
		Print("[ConvoyFollower] OFFROAD_ROUTE_SELECTED: length=" + m_fSelectedRouteLength +
			" start=" + m_vRouteStart + " goal=" + m_vRouteGoal +
			" axis=" + m_vRouteAxis + " candidate=120 geometry_only=true physical_drive_pending=true");
		return true;
	}
}
