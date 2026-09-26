// Private observation only. No method below changes route history/cursor,
// native requests, physics or controls. The inherited guide policy is intact.
class CF_OriginalTailGuideReadback
{
	bool HasHistory;
	bool Joined;
	bool HasArcGap;
	int Epoch;
	int RouteState;
	int Points;
	float Progress;
	float RecordedEnd;
	float GuideStation;
	float ArcGap;
	float CrossTrack;
}

class CF_OriginalTailGuideDriverComponentClass : CF_TrailGuideDriverControllerComponentClass
{
}

class CF_OriginalTailGuideDriverComponent : CF_TrailGuideDriverControllerComponent
{
	void CF_ReadOriginalTailGuide(CF_OriginalTailGuideReadback sample)
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

// All retained evidence is scalar/string/vector data. No native action,
// movement component or lease pointer is retained between observations.
class CF_OriginalTailGuideEvidence
{
	int Samples;
	bool Failed;
	bool AssignmentEstablished;
	bool PreviousEligible;
	int ActivitySequence;
	string ActivityKey;
	string WaypointKey;
	string NativeTargetKey;
	int RouteEpoch;
	float PreviousMs;
	vector PreviousPosition;
	vector PreviousForward;
	float Progress;
	int PoweredSamples;
	float BestProgress;
	int BestPoweredSamples;
	int BestSequence;
	string BestWaypointKey;
	string BestActivityKey;
	bool SawJoined;
	int ObservedEpoch;
	float LastRouteProgress;
	float LastRecordedEnd;
	int WaitSamples;
	string WaitKey;
}

class CF_OriginalTailGuideProbeComponentClass : CF_PacedRoadProbeComponentClass
{
}

// Unit1 uses the unchanged direct Original Follow controller. Only Unit2's
// native target is a recorded guide; its actual predecessor remains Unit1.
class CF_OriginalTailGuideProbeComponent : CF_PacedRoadProbeComponent
{
	protected ref array<ref CF_OriginalTailGuideEvidence> m_MixedUnits = {};
	protected bool m_bMixedFailed;
	protected bool m_bMixedFatal;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!Replication.IsServer()) return;
		m_MixedUnits.Insert(new CF_OriginalTailGuideEvidence());
		m_MixedUnits.Insert(new CF_OriginalTailGuideEvidence());
		if (m_iExpectedTrucks != 2)
			MixedFailure("requires_exactly_two_original_followers", 0);
		CF_ConvoySettings settings = CF_ConvoySettings.Get();
		if (settings.m_fMovingGap != 20.0 || settings.m_fStoppedGap != 10.0)
			MixedFailure("comparison_requires_unchanged_20_10_gaps", 0);
		Print("[ConvoyFollower] MIXED_FOLLOW_INIT: run_id=" + m_sPacedRun + " world=" + m_sPacedWorld +
			" expected=2 unit1=direct_original unit2=recorded_guide_original" +
			" moving_distance_m=20 guide_capture_m=1 recorded_standoff_m=20 stopped_distance_m=10" +
			" required_each_progress_m=100 required_each_powered=3 max_link_m=60 hold_s=180 max_drift_m=2" +
			" first_interval_credit=false selection_gap_credit=false follower_writes=false test_only=true");
	}

	protected void MixedFailure(string reason, int unit)
	{
		if (unit > 0 && unit <= m_MixedUnits.Count())
		{
			if (m_MixedUnits[unit - 1].Failed) return;
			m_MixedUnits[unit - 1].Failed = true;
		}
		else if (m_bMixedFatal) return;
		m_bMixedFailed = true;
		m_bMixedFatal = true;
		NoteFailure("mixed_original_guide", reason + "_unit_" + unit);
	}

	protected int CountPrivateActions(SCR_AIGroupUtilityComponent utility)
	{
		if (!utility) return 0;
		array<ref AIActionBase> actions = {};
		utility.GetActions(actions);
		int count;
		foreach (AIActionBase action : actions)
		{
			if (!CF_EntityFollowActivity.Cast(action)) continue;
			if (action.GetActionState() == EAIActionState.COMPLETED || action.GetActionState() == EAIActionState.FAILED) continue;
			count++;
		}
		return count;
	}

	protected void ObserveMixedUnit(int unit)
	{
		CF_OriginalTailGuideEvidence evidence = m_MixedUnits[unit - 1];
		CF_PacedRoadTruckSample sample = m_PacedTrucks[unit];
		CF_PacedRoadTruckSample predecessor = m_PacedTrucks[unit - 1];
		if (!sample || !predecessor || !sample.Truck || !predecessor.Truck)
		{
			MixedFailure("original_identity_or_predecessor_missing", unit);
			return;
		}
		// BindOriginals snapshots both trucks before the sequential order loop
		// has assigned Unit2. Permit that staging interval only until this
		// unit's first exact assignment; never erase a later identity loss.
		bool assignmentRetained = IdentityRetained(unit, false);
		if (assignmentRetained && !evidence.AssignmentEstablished)
		{
			evidence.AssignmentEstablished = true;
			Print("[ConvoyFollower] MIXED_FOLLOW_BIND: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
				" seconds=" + PacedSeconds() + " unit=" + unit + " truck_id=" + EntityKey(sample.Truck) +
				" pilot_id=" + EntityKey(sample.Pilot) + " group_id=" + EntityKey(sample.Group) +
				" predecessor_id=" + EntityKey(predecessor.Truck) + " drive_started=" + m_bPacedStarted);
		}
		if ((evidence.AssignmentEstablished || m_bPacedStarted) &&
			(!assignmentRetained || (m_bPacedStarted && !ExactPilot(sample))))
		{
			MixedFailure("original_identity_or_predecessor_missing", unit);
			return;
		}
		CF_EntityFollowDriverControllerComponent driver = CF_EntityFollowDriverControllerComponent.Cast(sample.Driver);
		CF_OriginalTailGuideDriverComponent tail = CF_OriginalTailGuideDriverComponent.Cast(sample.Driver);
		if (!driver || !driver.CF_IsEntityFollowPrototypeEnabled() || driver.CF_HasEntityFallbackFailure())
		{
			MixedFailure("private_controller_missing_or_failed", unit);
			return;
		}
		if ((unit == 1 && CF_TrailGuideDriverControllerComponent.Cast(driver)) || (unit == 2 && !tail))
		{
			MixedFailure("direct_and_guide_mode_assignment_mismatch", unit);
			return;
		}
		CF_OriginalTailGuideReadback route = new CF_OriginalTailGuideReadback();
		CF_TrailGuideEntity guide;
		IEntity routePredecessor;
		bool routeBound;
		if (tail)
		{
			tail.CF_ReadOriginalTailGuide(route);
			guide = CF_TrailGuideEntity.Cast(tail.CF_GetTrailGuideEntity());
			routePredecessor = tail.CF_GetTrailRealPredecessor();
			routeBound = evidence.AssignmentEstablished && route.HasHistory && routePredecessor == predecessor.Truck;
			if (!tail.CF_IsTrailGuidePrototypeEnabled() || tail.CF_IsTrailGuideBlocked())
				MixedFailure("tail_guide_disabled_or_blocked", unit);
			if ((m_bPacedStarted || evidence.ObservedEpoch > 0) && !routeBound)
				MixedFailure("missing_actual_predecessor_history", unit);
			// Assignment initially seeds against the player lead while boarding.
			// Observe that unbound history, but latch only the real Unit1 chain.
			if (routeBound && evidence.ObservedEpoch == 0)
			{
				evidence.ObservedEpoch = route.Epoch;
				Print("[ConvoyFollower] MIXED_FOLLOW_ROUTE_BIND: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
					" seconds=" + PacedSeconds() + " unit=" + unit + " epoch=" + route.Epoch +
					" predecessor_id=" + EntityKey(routePredecessor) + " drive_started=" + m_bPacedStarted);
			}
			if (evidence.ObservedEpoch > 0 && route.Epoch != evidence.ObservedEpoch)
				MixedFailure("route_epoch_changed_without_roster_change", unit);
			if (evidence.SawJoined && !route.Joined)
				MixedFailure("genuine_route_join_was_reset", unit);
			if (routeBound && route.Joined && route.HasArcGap)
			{
				if (evidence.SawJoined && route.Progress + 0.01 < evidence.LastRouteProgress)
					MixedFailure("route_cursor_moved_backward", unit);
				if (evidence.SawJoined && route.RecordedEnd + 0.01 < evidence.LastRecordedEnd)
					MixedFailure("recorded_history_end_moved_backward", unit);
				evidence.SawJoined = true;
				evidence.LastRouteProgress = route.Progress;
				evidence.LastRecordedEnd = route.RecordedEnd;
			}
		}
		CF_EntityFollowWaypoint waypoint = driver.CF_GetEntityFollowWaypoint();
		SCR_AIGroupUtilityComponent utility = GroupUtility(sample);
		CF_EntityFollowActivity selected;
		if (utility) selected = CF_EntityFollowActivity.Cast(utility.GetCurrentAction());
		CF_OriginalFollowActivity original = CF_OriginalFollowActivity.Cast(selected);
		int activeActions = CountPrivateActions(utility);
		if (activeActions > 1) MixedFailure("multiple_active_private_activities", unit);
		bool live = selected && selected.GetActionState() != EAIActionState.COMPLETED && selected.GetActionState() != EAIActionState.FAILED;
		IEntity target;
		AIWaypoint related;
		float distance = -1;
		int sequence;
		if (selected)
		{
			target = selected.m_Entity.m_Value;
			related = selected.m_RelatedWaypoint;
			distance = selected.m_fCompletionDistance.m_Value;
			sequence = selected.CF_GetSequence();
		}
		bool guideTarget = unit == 2 && guide && target == guide;
		bool directTarget = target == predecessor.Truck;
		bool movingMode = (unit == 1 && directTarget && distance == CF_ConvoySettings.Get().m_fMovingGap);
		if (guideTarget) movingMode = distance == 1.0;
		bool stoppedMode = directTarget && distance == CF_ConvoySettings.Get().m_fStoppedGap;
		bool exact = live && original && original.Lease && waypoint && utility && activeActions == 1;
		string leaseReason = "not_selected";
		if (exact) exact = original.Lease.Executing(sample.Group, leaseReason);
		if (exact) exact = waypoint.CF_GetActivity() == selected && related == waypoint && waypoint.GetEntity() == target;
		if (exact) exact = sample.Group.GetCurrentWaypoint() == waypoint && (movingMode || stoppedMode);
		if (exact && guideTarget) exact = guide.CF_HasLease(waypoint) && tail.CF_GetTrailRealPredecessor() == predecessor.Truck;
		if (live && !exact) MixedFailure("selected_original_activity_binding_mismatch", unit);
		SCR_AIUtilityComponent pilotUtility = PilotUtility(sample);
		SCR_AIMoveInFormationBehavior formation;
		if (pilotUtility) formation = SCR_AIMoveInFormationBehavior.Cast(pilotUtility.GetCurrentBehavior());
		bool exactFormation = formation && formation.GetActionState() != EAIActionState.COMPLETED && formation.GetActionState() != EAIActionState.FAILED;
		if (exactFormation) exactFormation = formation.GetRelatedGroupActivity() == selected;
		if (m_bPacedObserving)
		{
			if (live || waypoint || activeActions > 0 || guide) MixedFailure("private_order_survived_arrival", unit);
			CF_EntityCapturedWait wait;
			if (pilotUtility) wait = CF_EntityCapturedWait.Cast(pilotUtility.GetCurrentBehavior());
			bool exactWait = wait && wait.GetActionState() != EAIActionState.COMPLETED && wait.GetActionState() != EAIActionState.FAILED;
			if (exactWait) exactWait = driver.CF_HasCapturedWaitLease(wait, sample.Pilot, sample.Truck, sample.Group);
			string waitKey;
			if (wait) waitKey = wait.ToString();
			if (!exactWait || (!evidence.WaitKey.IsEmpty() && waitKey != evidence.WaitKey))
				MixedFailure("exact_captured_wait_lost_or_replaced", unit);
			else { evidence.WaitKey = waitKey; evidence.WaitSamples++; }
		}
		bool eligible = m_bPacedStarted && exact && exactFormation && movingMode;
		if (unit == 2) eligible = eligible && routeBound && guideTarget && route.Joined && route.HasArcGap && route.RouteState == CF_DrivenRoute.TRACKING;
		float now = GetGame().GetWorld().GetWorldTime();
		vector position = sample.Truck.GetOrigin();
		vector forward = sample.Truck.GetWorldTransformAxis(2);
		string actionKey;
		if (selected) actionKey = selected.ToString();
		string waypointKey = EntityKey(waypoint);
		string targetKey = EntityKey(target);
		bool consecutive = eligible && evidence.PreviousEligible && sequence == evidence.ActivitySequence;
		if (consecutive) consecutive = actionKey == evidence.ActivityKey && waypointKey == evidence.WaypointKey && targetKey == evidence.NativeTargetKey;
		if (consecutive && unit == 2) consecutive = route.Epoch == evidence.RouteEpoch;
		float elapsed = (now - evidence.PreviousMs) * 0.001;
		if (consecutive) consecutive = elapsed > 0 && elapsed <= 3.0;
		float signedStep;
		bool powered;
		CarControllerComponent car = CarControllerComponent.Cast(sample.Truck.FindComponent(CarControllerComponent));
		VehicleWheeledSimulation sim;
		if (car) sim = car.GetSimulation();
		if (!sim) MixedFailure("vehicle_simulation_unavailable", unit);
		if (consecutive && sim)
		{
			vector delta = position - evidence.PreviousPosition;
			vector axis = evidence.PreviousForward;
			axis[1] = 0;
			axis.Normalize();
			signedStep = vector.Dot(delta, axis);
			if (vector.DistanceXZ(position, evidence.PreviousPosition) > 20.0)
			{
				MixedFailure("sample_position_discontinuity", unit);
				consecutive = false;
			}
			powered = sim.EngineIsOn() && sim.GetThrottle() > 0.05 && sim.GetGear() >= 2 && sim.GetSpeedKmh() > 1.5 && signedStep >= 0.25;
		}
		if (!consecutive) { evidence.Progress = 0; evidence.PoweredSamples = 0; }
		else
		{
			evidence.Progress += signedStep;
			if (powered) evidence.PoweredSamples++;
			if (evidence.Progress > evidence.BestProgress)
			{
				evidence.BestProgress = evidence.Progress;
				evidence.BestPoweredSamples = evidence.PoweredSamples;
				evidence.BestSequence = sequence;
				evidence.BestWaypointKey = waypointKey;
				evidence.BestActivityKey = actionKey;
			}
		}
		evidence.PreviousEligible = eligible && !evidence.Failed;
		evidence.ActivitySequence = sequence;
		evidence.ActivityKey = actionKey;
		evidence.WaypointKey = waypointKey;
		evidence.NativeTargetKey = targetKey;
		evidence.RouteEpoch = route.Epoch;
		evidence.PreviousPosition = position;
		evidence.PreviousForward = forward;
		evidence.PreviousMs = now;
		// All predicates keep running after the print cap. No silent evidence gap.
		evidence.Samples++;
		if (evidence.Samples > 480)
		{
			if (evidence.Samples == 481) Print("[ConvoyFollower] MIXED_FOLLOW_SAMPLE_LIMIT: unit=" + unit + " validation_continues=true");
			return;
		}
		string line = "[ConvoyFollower] MIXED_FOLLOW_SAMPLE: run_id=" + m_sPacedRun;
		line += " tick=" + m_iPacedTick + " seconds=" + PacedSeconds() + " unit=" + unit;
		line += " truck_id=" + EntityKey(sample.Truck) + " pilot_id=" + EntityKey(sample.Pilot) + " group_id=" + EntityKey(sample.Group);
		line += " assignment_established=" + evidence.AssignmentEstablished + " identity_retained=" + assignmentRetained;
		line += " predecessor_id=" + EntityKey(predecessor.Truck) + " native_target_id=" + targetKey + " guide_id=" + EntityKey(guide);
		line += " waypoint_id=" + waypointKey + " related_id=" + EntityKey(related) + " activity=" + actionKey + " sequence=" + sequence;
		line += " live=" + live + " exact=" + exact + " lease_reason=" + leaseReason + " active_actions=" + activeActions;
		line += " desired_distance=" + distance + " moving_mode=" + movingMode + " stopped_mode=" + stoppedMode + " exact_formation=" + exactFormation;
		line += " eligible=" + eligible + " consecutive=" + consecutive + " signed_step_m=" + signedStep + " powered=" + powered;
		line += " origin=" + position + " progress_m=" + evidence.Progress + " powered_samples=" + evidence.PoweredSamples;
		line += " route_history=" + route.HasHistory + " route_points=" + route.Points + " route_joined=" + route.Joined + " route_epoch=" + route.Epoch;
		line += " route_bound=" + routeBound + " observed_epoch=" + evidence.ObservedEpoch + " route_predecessor_id=" + EntityKey(routePredecessor);
		line += " route_state=" + route.RouteState + " route_progress_m=" + route.Progress + " recorded_end_m=" + route.RecordedEnd;
		line += " guide_station_m=" + route.GuideStation + " arc_gap_valid=" + route.HasArcGap + " arc_gap_m=" + route.ArcGap + " cross_track_m=" + route.CrossTrack;
		line += " observing=" + m_bPacedObserving + " wait_samples=" + evidence.WaitSamples + " wait=" + evidence.WaitKey + " unit_failed=" + evidence.Failed;
		Print(line);
	}

	override protected void LogPacedSamples()
	{
		if (!m_PacedTrucks.IsEmpty())
		{
			if (m_PacedTrucks.Count() != 3 || m_MixedUnits.Count() != 2)
				MixedFailure("original_roster_count_mismatch", 0);
			else
			{
				ObserveMixedUnit(1);
				ObserveMixedUnit(2);
			}
		}
		super.LogPacedSamples();
	}

	override protected void Finish(string result)
	{
		if (result.IndexOf("PASS") == 0)
		{
			if (m_MixedUnits.Count() != 2 || m_PacedTrucks.Count() != 3) MixedFailure("route_terminal_roster", 0);
			for (int unit = 1; unit <= m_MixedUnits.Count(); unit++)
			{
				CF_OriginalTailGuideEvidence evidence = m_MixedUnits[unit - 1];
				if (evidence.BestProgress < CF_MIN_FOLLOWER_PATH || evidence.BestPoweredSamples < 3)
					MixedFailure("continuous_exact_original_powered_progress_missing", unit);
				if (unit == 2 && !evidence.SawJoined) MixedFailure("tail_never_genuinely_joined_route", unit);
			}
		}
		super.Finish(result);
	}

	override protected void Poll()
	{
		super.Poll();
		if (m_bMixedFatal && !m_bPacedTerminal) EndPaced();
	}

	override protected void EndPaced()
	{
		if (m_bPacedTerminal) return;
		for (int unit = 1; unit <= m_MixedUnits.Count(); unit++)
		{
			CF_OriginalTailGuideEvidence evidence = m_MixedUnits[unit - 1];
			if (!m_bPacedFailed && evidence.WaitSamples < 180) MixedFailure("exact_wait_observation_under_180_samples", unit);
			string line = "[ConvoyFollower] MIXED_FOLLOW_COVERAGE: run_id=" + m_sPacedRun + " unit=" + unit;
			line += " best_progress_m=" + evidence.BestProgress + " powered_samples=" + evidence.BestPoweredSamples;
			line += " sequence=" + evidence.BestSequence + " waypoint=" + evidence.BestWaypointKey + " activity=" + evidence.BestActivityKey;
			line += " route_joined=" + evidence.SawJoined + " route_epoch=" + evidence.ObservedEpoch;
			line += " wait_samples=" + evidence.WaitSamples + " unit_failed=" + evidence.Failed + " gate_failed=" + m_bMixedFailed;
			Print(line);
		}
		super.EndPaced();
	}
}
