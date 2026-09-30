class CF_EntityFollowRoadProbeComponentClass : CF_PacedRoadProbeComponentClass
{
}

// Value-only evidence for one original follower; never shares another unit's
// activity progress or retains an AI action/control handle.
class CF_EntityFollowUnitEvidence
{
	int ActivitySequence;
	string PrivateMoveOrderKey;
	int PrivateMoveSequence;
	int PoweredSamples;
	float Progress;
	float BestProgress;
	int BestPoweredSamples;
	int BestSequence;
	int Samples;
	bool GateFailed;
}

// Same lead, geometry, ownership and physical gates as the matching paced
// control. Only the separate follower prefab selects the private controller.
class CF_EntityFollowRoadProbeComponent : CF_PacedRoadProbeComponent
{
	protected ref array<ref CF_EntityFollowUnitEvidence> m_EntityUnits = {};
	protected bool m_bEntityGateFailed;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!Replication.IsServer())
			return;
		for (int unit = 1; unit <= m_iExpectedTrucks; unit++)
			m_EntityUnits.Insert(new CF_EntityFollowUnitEvidence());
		Print("[ConvoyFollower] ENTITY_FOLLOW_INIT: run_id=" + m_sPacedRun +
			" world=" + m_sPacedWorld + " expected=" + m_iExpectedTrucks + " test_only=true production_move_control_unchanged=true" +
			" vehicle_planning=true target_basis=current_predecessor_entity driven_trail_claim=false" +
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
			CF_EntityFollowDriverControllerComponent driver = CF_EntityFollowDriverControllerComponent.Cast(sample.Driver);
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
			if (!driver || !driver.CF_IsEntityFollowPrototypeEnabled())
				EntityGateFailure("isolated_entity_controller_not_enabled", unit);
			else if (driver.CF_HasEntityFallbackFailure())
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
			bool exact = liveSelected && waypoint && sample.Group && predecessor.Truck &&
				waypointActivity == selected && related == waypoint &&
				waypoint.GetEntity() == predecessor.Truck && target == predecessor.Truck &&
				sample.Group.GetCurrentWaypoint() == waypoint && IdentityRetained(unit, true);
			if (liveSelected && !exact)
				EntityGateFailure("selected_activity_target_or_waypoint_mismatch", unit);
			if (m_bPacedObserving && (liveSelected || waypoint || activeActivities > 0))
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
					powered = simulation.GetThrottle() > 0.05 && simulation.GetGear() >= 2 &&
						simulation.GetSpeedKmh() > 1.5 && signedStep >= 0.25;
				}
			}
			if (driver && driver.CF_PrivateRecordedTrailEnabled())
			{
				AIWaypoint ownedMove = driver.CF_PrivateReadOwnedMove();
				SCR_AIMoveActivity nativeMove;
				if (utility) nativeMove = SCR_AIMoveActivity.Cast(utility.GetCurrentAction());
				SCR_AIUtilityComponent pilotUtility = PilotUtility(sample);
				SCR_AIMoveInFormationBehavior nativePilot;
				if (pilotUtility) nativePilot = SCR_AIMoveInFormationBehavior.Cast(pilotUtility.GetCurrentBehavior());
				bool moveLive = nativeMove && nativeMove.GetActionState() != EAIActionState.COMPLETED && nativeMove.GetActionState() != EAIActionState.FAILED;
				bool pilotLive = nativePilot && nativePilot.GetActionState() != EAIActionState.COMPLETED && nativePilot.GetActionState() != EAIActionState.FAILED;
				bool moveExact = moveLive && pilotLive && ownedMove && sample.Group && utility &&
					utility.GetExecutedAction() == nativeMove && sample.Group.GetCurrentWaypoint() == ownedMove &&
					nativeMove.m_RelatedWaypoint == ownedMove && nativePilot.GetRelatedGroupActivity() == nativeMove &&
					nativeMove.m_bUseVehicles.m_Value && nativeMove.m_Entity.m_Value == ownedMove &&
					nativeMove.m_vPosition.m_Value == vector.Zero && IdentityRetained(unit, true);
				int liveNativeOrders;
				if (utility)
				{
					array<ref AIActionBase> nativeActions = {}; utility.GetActions(nativeActions);
					foreach (AIActionBase na : nativeActions)
						if (SCR_AIMoveActivity.Cast(na) && na.GetActionState() != EAIActionState.COMPLETED && na.GetActionState() != EAIActionState.FAILED) liveNativeOrders++;
				}
				moveExact = moveExact && liveNativeOrders == 1;
				IEntity nativeEntity; AIWaypoint nativeRelated; vector waypointGoal;
				bool executedMatch, pilotRelatedMatch;
				if (nativeMove) { nativeEntity = nativeMove.m_Entity.m_Value; nativeRelated = nativeMove.m_RelatedWaypoint; executedMatch = utility && utility.GetExecutedAction() == nativeMove; }
				if (nativePilot) pilotRelatedMatch = nativePilot.GetRelatedGroupActivity() == nativeMove;
				if (ownedMove) waypointGoal = ownedMove.GetOrigin();
				string orderKey;
				if (nativeMove && ownedMove) orderKey = nativeMove.ToString() + "/" + ownedMove.GetID().ToString();
				if (moveExact && m_bPacedStarted)
				{
					if (orderKey != unitEvidence.PrivateMoveOrderKey)
					{
						unitEvidence.PrivateMoveOrderKey = orderKey;
						unitEvidence.PrivateMoveSequence++;
						unitEvidence.ActivitySequence = -unitEvidence.PrivateMoveSequence;
						unitEvidence.Progress = 0; unitEvidence.PoweredSamples = 0;
					}
					unitEvidence.Progress += signedStep;
					if (powered) unitEvidence.PoweredSamples++;
					if (unitEvidence.Progress > unitEvidence.BestProgress)
					{ unitEvidence.BestProgress = unitEvidence.Progress; unitEvidence.BestPoweredSamples = unitEvidence.PoweredSamples; unitEvidence.BestSequence = unitEvidence.ActivitySequence; }
				}
				vector nativeGoal;
				if (nativeMove) nativeGoal = nativeMove.m_vPosition.m_Value;
				Print("[ConvoyFollower] RECORDED_MOVE_SAMPLE: run_id=" + m_sPacedRun + " unit=" + unit + " tick=" + m_iPacedTick +
					" seconds=" + PacedSeconds() + " exact=" + moveExact + " live_selected=" + moveLive + " pilot_live=" + pilotLive +
					" order_key=" + orderKey + " sequence=" + unitEvidence.PrivateMoveSequence + " native_goal=" + nativeGoal +
					" owned_waypoint=" + EntityKey(ownedMove) + " native_entity_id=" + EntityKey(nativeEntity) + " related_id=" + EntityKey(nativeRelated) +
					" waypoint_goal=" + waypointGoal + " executed_match=" + executedMatch + " pilot_related_match=" + pilotRelatedMatch + " live_native_orders=" + liveNativeOrders +
					" target_basis=canonical_WP_Move_follow_waypoint_entity predecessor_id=" + EntityKey(predecessor.Truck) +
					" signed_step_m=" + signedStep + " powered=" + powered + " activity_progress_m=" + unitEvidence.Progress +
					" powered_samples=" + unitEvidence.PoweredSamples + " semantics=per_single_owned_native_MOVE_no_cross_order_sum read_only=true");
			}
			if (m_bPacedStarted && exact)
			{
				if (sequence != unitEvidence.ActivitySequence)
				{
					unitEvidence.ActivitySequence = sequence;
					unitEvidence.Progress = 0;
					unitEvidence.PoweredSamples = 0;
				}
				unitEvidence.Progress += signedStep;
				if (powered)
					unitEvidence.PoweredSamples++;
				if (unitEvidence.Progress > unitEvidence.BestProgress)
				{
					unitEvidence.BestProgress = unitEvidence.Progress;
					unitEvidence.BestPoweredSamples = unitEvidence.PoweredSamples;
					unitEvidence.BestSequence = sequence;
				}
			}
			string evidence = "[ConvoyFollower] ENTITY_FOLLOW_SAMPLE: run_id=" + m_sPacedRun;
			evidence += " tick=" + m_iPacedTick + " seconds=" + PacedSeconds() + " unit=" + unit + " sequence=" + sequence;
			evidence += " selected=" + (selected != null) + " exact=" + exact + " target_id=" + EntityKey(target);
			evidence += " live_selected=" + liveSelected;
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
				EntityGateFailure("single_persistent_activity_powered_progress_not_established", unit);
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
