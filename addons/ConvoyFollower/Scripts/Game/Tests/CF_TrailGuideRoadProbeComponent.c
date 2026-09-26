class CF_TrailGuideRoadProbeComponentClass : CF_PacedRoadProbeComponentClass
{
}

// One-follower comparison only. Same geometry and physical gates. Moving
// native target is an exact owned guide; actual predecessor identity stays real.
// The 100m/3powered gate counts consecutive exact samples after genuine Query join.
class CF_TrailGuideRoadProbeComponent : CF_PacedRoadProbeComponent
{
	protected ref array<ref CF_EntityFollowUnitEvidence> m_EntityUnits = {};
	protected bool m_bEntityGateFailed;
	protected int m_iPreviousGuideSequence;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!Replication.IsServer())
			return;
		if (m_iExpectedTrucks != 1)
			NoteFailure("prototype", "trail_guide_requires_exactly_one_follower");
		for (int unit = 1; unit <= m_iExpectedTrucks; unit++)
			m_EntityUnits.Insert(new CF_EntityFollowUnitEvidence());
		Print("[ConvoyFollower] ENTITY_FOLLOW_INIT: run_id=" + m_sPacedRun +
			" world=" + m_sPacedWorld + " expected=" + m_iExpectedTrucks + " test_only=true production_move_control_unchanged=true" +
			" vehicle_planning=true target_basis=owned_recorded_guide real_predecessor_retained=true" +
			" native_moving_capture_m=1 route_standoff_m=20 stopped_entity_distance_m=10 lead_move_radius_m=5" +
			" required_single_activity_progress_m=100 required_powered_samples=3 inherited_peak_gap_m=60 inherited_hold_s=180");
	}

	protected void EntityGateFailure(string reason, int unit = 0)
	{
		if (unit > 0 && unit <= m_EntityUnits.Count())
		{
			CF_EntityFollowUnitEvidence evidence = m_EntityUnits[unit - 1];
			if (!evidence.GateFailed)
			{
				evidence.GateFailed = true;
				Print("[ConvoyFollower] ENTITY_FOLLOW_UNIT_FAILURE: run_id=" + m_sPacedRun +
					" unit=" + unit + " reason=" + reason);
			}
		}
		if (m_bEntityGateFailed)
			return;
		m_bEntityGateFailed = true;
		if (m_iExpectedTrucks > 1 && unit > 0)
			reason += "_unit_" + unit;
		NoteFailure("prototype", reason);
	}

	override protected void LogPacedSamples()
	{
		// Read before parent sampling updates LastPosition/LastForward.
		if (!m_PacedTrucks.IsEmpty() && m_PacedTrucks.Count() != m_iExpectedTrucks + 1)
			EntityGateFailure("original_follower_count_mismatch");
		for (int unit = 1; unit <= m_iExpectedTrucks && unit < m_PacedTrucks.Count(); unit++)
		{
			CF_EntityFollowUnitEvidence unitEvidence = m_EntityUnits[unit - 1];
			if (unitEvidence.Samples >= 480)
				continue;
			unitEvidence.Samples++;
			CF_PacedRoadTruckSample sample = m_PacedTrucks[unit];
			CF_PacedRoadTruckSample predecessor = m_PacedTrucks[unit - 1];
			if (!sample || !predecessor)
			{
				EntityGateFailure("original_follower_or_predecessor_missing", unit);
				continue;
			}
			CF_TrailGuideDriverControllerComponent driver = CF_TrailGuideDriverControllerComponent.Cast(sample.Driver);
			CF_EntityFollowWaypoint waypoint;
			CF_EntityFollowActivity waypointActivity;
			SCR_AIGroupUtilityComponent utility = GroupUtility(sample);
			CF_EntityFollowActivity selected;
			if (driver)
				waypoint = driver.CF_GetEntityFollowWaypoint();
			if (waypoint)
				waypointActivity = waypoint.CF_GetActivity();
			if (utility)
				selected = CF_EntityFollowActivity.Cast(utility.GetCurrentAction());
			int activeActivities;
			if (utility)
			{
				array<ref AIActionBase> actions = {};
				utility.GetActions(actions);
				foreach (AIActionBase action : actions)
				{
					if (CF_EntityFollowActivity.Cast(action) && action.GetActionState() != EAIActionState.COMPLETED &&
						action.GetActionState() != EAIActionState.FAILED)
						activeActivities++;
				}
			}
			if (activeActivities > 1)
				EntityGateFailure("multiple_active_private_follow_activities", unit);
			if (!driver || !driver.CF_IsEntityFollowPrototypeEnabled() || !driver.CF_IsTrailGuidePrototypeEnabled())
				EntityGateFailure("isolated_entity_controller_not_enabled", unit);
			else if (driver.CF_HasEntityFallbackFailure() || driver.CF_IsTrailGuideBlocked())
				EntityGateFailure("entity_follow_handoff_failed", unit);
			IEntity target;
			AIWaypoint related;
			int sequence;
			float desiredDistance = -1;
			if (selected)
			{
				target = selected.m_Entity.m_Value;
				related = selected.m_RelatedWaypoint;
				sequence = selected.CF_GetSequence();
				desiredDistance = selected.m_fCompletionDistance.m_Value;
			}
			bool liveSelected = selected && selected.GetActionState() != EAIActionState.COMPLETED &&
				selected.GetActionState() != EAIActionState.FAILED;
			CF_TrailGuideEntity guide;
			if (driver)
				guide = CF_TrailGuideEntity.Cast(driver.CF_GetTrailGuideEntity());
			bool guideTarget = guide && target == guide && waypoint && waypoint.GetEntity() == guide;
			bool exactTarget = false;
			if (driver && guideTarget)
				exactTarget = guide.CF_HasLease(waypoint) && driver.CF_GetTrailRealPredecessor() == predecessor.Truck && desiredDistance == 1.0;
			else if (waypoint && target == predecessor.Truck && waypoint.GetEntity() == predecessor.Truck)
				exactTarget = desiredDistance == CF_ConvoySettings.Get().m_fStoppedGap;
			bool exact = liveSelected && waypoint && sample.Group && predecessor.Truck && exactTarget &&
				waypointActivity == selected && related == waypoint &&
				sample.Group.GetCurrentWaypoint() == waypoint && IdentityRetained(unit, true);
			if (liveSelected && !exact)
				EntityGateFailure("selected_activity_target_or_waypoint_mismatch", unit);
			if (m_bPacedObserving && (liveSelected || waypoint || activeActivities > 0 || guide))
				EntityGateFailure("persistent_entity_order_survived_arrival", unit);
			float signedStep;
			bool powered;
			if (sample.Truck)
			{
				vector delta = sample.Truck.GetOrigin() - sample.LastPosition;
				signedStep = delta[0] * sample.LastForward[0] + delta[2] * sample.LastForward[2];
				CarControllerComponent car = CarControllerComponent.Cast(sample.Truck.FindComponent(CarControllerComponent));
				if (car && car.GetSimulation())
				{
					VehicleWheeledSimulation simulation = car.GetSimulation();
					powered = simulation.EngineIsOn() && simulation.GetThrottle() > 0.05 && simulation.GetGear() >= 2 &&
						simulation.GetSpeedKmh() > 1.5 && signedStep >= 0.25;
				}
			}
			if (m_bPacedStarted && exact && guideTarget && driver.CF_HasTrailJoined())
			{
				if (sequence != unitEvidence.ActivitySequence || sequence != m_iPreviousGuideSequence)
				{
					unitEvidence.ActivitySequence = sequence;
					unitEvidence.Progress = 0;
					unitEvidence.PoweredSamples = 0;
				}
				if (sequence == m_iPreviousGuideSequence)
					unitEvidence.Progress += signedStep;
				if (powered && sequence == m_iPreviousGuideSequence)
					unitEvidence.PoweredSamples++;
				if (unitEvidence.Progress > unitEvidence.BestProgress)
				{
					unitEvidence.BestProgress = unitEvidence.Progress;
					unitEvidence.BestPoweredSamples = unitEvidence.PoweredSamples;
					unitEvidence.BestSequence = sequence;
				}
			}
			m_iPreviousGuideSequence = 0;
			if (exact && guideTarget && driver.CF_HasTrailJoined())
				m_iPreviousGuideSequence = sequence;
			string evidence = "[ConvoyFollower] ENTITY_FOLLOW_SAMPLE: run_id=" + m_sPacedRun;
			evidence += " tick=" + m_iPacedTick + " seconds=" + PacedSeconds() + " unit=" + unit + " sequence=" + sequence;
			evidence += " selected=" + (selected != null) + " exact=" + exact + " target_id=" + EntityKey(target);
			evidence += " live_selected=" + liveSelected + " guide_target=" + guideTarget + " guide_id=" + EntityKey(guide);
			if (driver)
				evidence += " route_joined=" + driver.CF_HasTrailJoined() + " route_epoch=" + driver.CF_GetTrailEpoch();
			evidence += " desired_distance=" + desiredDistance;
			evidence += " active_private_activities=" + activeActivities;
			evidence += " predecessor_id=" + EntityKey(predecessor.Truck) + " waypoint_id=" + EntityKey(waypoint);
			evidence += " related_id=" + EntityKey(related) + " signed_step_m=" + signedStep + " powered=" + powered;
			evidence += " activity_progress_m=" + unitEvidence.Progress + " powered_samples=" + unitEvidence.PoweredSamples;
			evidence += " observing=" + m_bPacedObserving + " gate_failed=" + m_bEntityGateFailed;
			evidence += " unit_gate_failed=" + unitEvidence.GateFailed;
			Print(evidence);
		}
		super.LogPacedSamples();
	}

	override protected void Finish(string result)
	{
		bool routePass = result.IndexOf("PASS") == 0;
		if (routePass && (m_EntityUnits.Count() != m_iExpectedTrucks || m_PacedTrucks.Count() != m_iExpectedTrucks + 1))
			EntityGateFailure("original_follower_count_mismatch");
		for (int unit = 1; unit <= m_EntityUnits.Count(); unit++)
		{
			CF_EntityFollowUnitEvidence evidence = m_EntityUnits[unit - 1];
			if (routePass && (evidence.BestProgress < CF_MIN_FOLLOWER_PATH || evidence.BestPoweredSamples < 3))
				EntityGateFailure("single_joined_guide_activity_powered_progress_not_established", unit);
		}
		// Emit the same final aggregate gate on every unit's coverage record.
		for (int coverageUnit = 1; coverageUnit <= m_EntityUnits.Count(); coverageUnit++)
		{
			CF_EntityFollowUnitEvidence coverage = m_EntityUnits[coverageUnit - 1];
			Print("[ConvoyFollower] ENTITY_FOLLOW_COVERAGE: run_id=" + m_sPacedRun +
				" best_activity_progress_m=" + coverage.BestProgress + " powered_samples=" + coverage.BestPoweredSamples +
				" gate_failed=" + m_bEntityGateFailed + " unit=" + coverageUnit + " best_sequence=" + coverage.BestSequence +
				" unit_gate_failed=" + coverage.GateFailed + " samples=" + coverage.Samples + " provisional=true");
		}
		super.Finish(result);
	}
}
