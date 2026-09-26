// Test-only non-owning readback. These methods copy values or inspect the
// existing role key; no lifecycle override, assignment, order or control write.
modded class CF_ConvoyFollowDriverControllerComponent
{
	bool CF_TestOrdinaryRoleReady(bool tail)
	{
		return m_bRoleBound && !m_bRolePending && m_bTrailGuidePrototype == tail &&
			m_bEntityRearPacing == !tail && CF_RoleMatchesCurrent();
	}

	void CF_TestOrdinaryReadRoute(CF_OriginalTailGuideReadback sample)
	{
		if (!sample) return;
		sample.HasHistory = m_TrailRoute != null;
		sample.Joined = m_bTrailJoined;
		sample.Epoch = m_iTrailEpoch;
		sample.GuideStation = m_fTrailGuideStation;
		if (m_TrailRoute) sample.Points = m_TrailRoute.GetPointCount();
		if (!m_TrailGuidance) return;
		sample.HasArcGap = m_TrailGuidance.HasArcGap;
		sample.RouteState = m_TrailGuidance.State;
		sample.Progress = m_TrailGuidance.Progress;
		sample.RecordedEnd = m_TrailGuidance.RecordedEnd;
		sample.ArcGap = m_TrailGuidance.ArcGap;
		sample.CrossTrack = m_TrailGuidance.CrossTrack;
	}
}

class CF_OrdinaryDirectRoadProbeComponentClass : CF_OriginalFollowResumeProbeComponentClass
{
}

// Parent supplies the existing native lead and real server Hold/Resume course.
// This extension only checks ordinary identity/mode; it issues no new command.
class CF_OrdinaryDirectRoadProbeComponent : CF_OriginalFollowResumeProbeComponent
{
	protected bool m_bOrdinaryHeadCommitted;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!Replication.IsServer()) return;
		CF_ConvoySettings settings = CF_ConvoySettings.Get();
		if (m_iExpectedTrucks != 1 || m_fPacedLeadSpeedKmh != 20 || m_bCommandsAfterSpacingFailure ||
			settings.m_fMovingGap != 20 || settings.m_fStoppedGap != 10)
			EntityGateFailure("ordinary_fixture_contract_mismatch");
		Print("[ConvoyFollower] ORDINARY_FIXTURE_INIT: run_id=" + m_sPacedRun + " world=" + m_sPacedWorld +
			" expected=1 head=direct_original tail=none ordinary_controller=CF_ConvoyFollowDriverControllerComponent" +
			" lead_cap_kmh=20 restart_lead_cap_kmh=20 moving_gap_m=20 stopped_gap_m=10 readback_only=true commands=existing_server_hold_resume spacing_diagnostic=false");
	}

	protected void ObserveOrdinaryHead()
	{
		if (m_PacedTrucks.Count() != 2) return;
		CF_PacedRoadTruckSample sample = m_PacedTrucks[1];
		CF_ConvoyFollowDriverControllerComponent driver;
		if (sample) driver = CF_ConvoyFollowDriverControllerComponent.Cast(sample.Driver);
		if (!driver || driver.Type().ToString() != "CF_ConvoyFollowDriverControllerComponent")
		{ EntityGateFailure("ordinary_exact_controller_missing", 1); return; }
		CF_OriginalTailGuideReadback route = new CF_OriginalTailGuideReadback();
		driver.CF_TestOrdinaryReadRoute(route);
		if (driver.CF_IsTrailGuidePrototypeEnabled() || driver.CF_GetTrailGuideEntity() || route.HasHistory || route.Joined)
		{ EntityGateFailure("ordinary_head_not_direct", 1); return; }
		bool ready = driver.CF_TestOrdinaryRoleReady(false);
		if (!ready && (m_bOrdinaryHeadCommitted || m_bPacedStarted))
			EntityGateFailure("ordinary_head_role_lost_or_uncommitted", 1);
		if (ready && !m_bOrdinaryHeadCommitted && IdentityRetained(1, true))
		{
			m_bOrdinaryHeadCommitted = true;
			Print("[ConvoyFollower] ORDINARY_FIXTURE_ROLE: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
				" seconds=" + PacedSeconds() + " unit=1 truck_id=" + EntityKey(sample.Truck) + " pilot_id=" + EntityKey(sample.Pilot) +
				" group_id=" + EntityKey(sample.Group) + " predecessor_id=" + EntityKey(m_PacedTrucks[0].Truck) +
				" controller=CF_ConvoyFollowDriverControllerComponent role_ready=true exact_identity=true tail_guide=false rear_pacing=true");
		}
	}

	override protected void LogPacedSamples()
	{
		ObserveOrdinaryHead();
		super.LogPacedSamples();
	}

	override protected void Poll()
	{
		// Parent command stages stop calling LogPacedSamples. Keep checking the
		// same ordinary mode during owner seat transfer, Hold and Resume too.
		if (m_iCommandStage > 0 && !m_bPacedTerminal) ObserveOrdinaryHead();
		super.Poll();
	}
}

class CF_OrdinaryMixedRoadProbeComponentClass : CF_OriginalTailGuideProbeComponentClass
{
}

class CF_OrdinaryMixedRoadProbeComponent : CF_OriginalTailGuideProbeComponent
{
	protected bool m_bOrdinaryHeadCommitted;
	protected bool m_bOrdinaryTailCommitted;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!Replication.IsServer()) return;
		if (m_fPacedLeadSpeedKmh != 20) MixedFailure("ordinary_fixture_lead_not_20", 0);
		Print("[ConvoyFollower] ORDINARY_FIXTURE_INIT: run_id=" + m_sPacedRun + " world=" + m_sPacedWorld +
			" expected=2 head=direct_original tail=recorded_guide_original ordinary_controller=CF_ConvoyFollowDriverControllerComponent" +
			" lead_cap_kmh=20 moving_gap_m=20 stopped_gap_m=10 readback_only=true commands=none spacing_diagnostic=false");
	}

	override protected CF_TrailGuideDriverControllerComponent MixedTail(CF_DriverControllerComponent driver, int unit)
	{
		if (unit != 2) return null;
		return CF_ConvoyFollowDriverControllerComponent.Cast(driver);
	}

	override protected bool MixedModesMatch(int unit, CF_EntityFollowDriverControllerComponent driver, CF_TrailGuideDriverControllerComponent tail)
	{
		CF_ConvoyFollowDriverControllerComponent ordinary = CF_ConvoyFollowDriverControllerComponent.Cast(driver);
		if (!ordinary || ordinary.Type().ToString() != "CF_ConvoyFollowDriverControllerComponent") return false;
		bool tailRole = unit == 2;
		bool ready = ordinary.CF_TestOrdinaryRoleReady(tailRole);
		bool committed = m_bOrdinaryHeadCommitted;
		if (tailRole) committed = m_bOrdinaryTailCommitted;
		if (tailRole && !tail) return false;
		if (!tailRole)
		{
			CF_OriginalTailGuideReadback head = new CF_OriginalTailGuideReadback();
			ordinary.CF_TestOrdinaryReadRoute(head);
			if (ordinary.CF_IsTrailGuidePrototypeEnabled() || ordinary.CF_GetTrailGuideEntity() || head.HasHistory || head.Joined) return false;
		}
		if (!ready && (committed || m_bPacedStarted || ordinary.CF_IsTrailGuidePrototypeEnabled())) return false;
		if (ready && !committed && IdentityRetained(unit, true))
		{
			if (tailRole) m_bOrdinaryTailCommitted = true;
			else m_bOrdinaryHeadCommitted = true;
			CF_PacedRoadTruckSample sample = m_PacedTrucks[unit];
			Print("[ConvoyFollower] ORDINARY_FIXTURE_ROLE: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
				" seconds=" + PacedSeconds() + " unit=" + unit + " truck_id=" + EntityKey(sample.Truck) + " pilot_id=" + EntityKey(sample.Pilot) +
				" group_id=" + EntityKey(sample.Group) + " predecessor_id=" + EntityKey(m_PacedTrucks[unit - 1].Truck) +
				" controller=CF_ConvoyFollowDriverControllerComponent role_ready=true exact_identity=true tail_guide=" + tailRole + " rear_pacing=" + !tailRole);
		}
		return true;
	}

	override protected void ReadMixedTail(CF_TrailGuideDriverControllerComponent tail, CF_OriginalTailGuideReadback route)
	{
		CF_ConvoyFollowDriverControllerComponent.Cast(tail).CF_TestOrdinaryReadRoute(route);
	}

	override protected bool MixedTailReady(CF_TrailGuideDriverControllerComponent tail, CF_OriginalTailGuideReadback route)
	{
		if (tail.CF_IsTrailGuideBlocked()) return false;
		if (tail.CF_IsTrailGuidePrototypeEnabled()) return true;
		// Ordinary recruits start disabled. Only the initial uncommitted,
		// pre-drive empty state is allowed; no post-bind loss is hidden.
		return !m_bPacedStarted && !m_bOrdinaryTailCommitted && !route.HasHistory && !route.Joined &&
			route.Epoch == 0 && !tail.CF_GetTrailGuideEntity() && !tail.CF_GetTrailRealPredecessor();
	}
}
