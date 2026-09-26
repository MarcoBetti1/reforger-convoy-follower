class CF_TrailGuideDriverControllerComponentClass : CF_EntityFollowDriverControllerComponentClass
{
}

// NEW private prefab only. Real predecessor remains in the inherited session,
// stop, spacing, health and cruise paths. Only the moving native target differs.
class CF_TrailGuideDriverControllerComponent : CF_EntityFollowDriverControllerComponent
{
	[Attribute(defvalue: "0", desc: "Private one-follower recorded trail-guide experiment")]
	protected bool m_bTrailGuidePrototype;
	[Attribute(defvalue: "0", desc: "Private projected-station comparison: one adjacent corner; station is not physical travel")]
	protected bool m_bTrailProjectionCorrection;
	protected int m_iTrailProjectionLogs;
	// Non-owning observation receipt: no ref fields, actions or control writes.
	protected float m_fRearGuideQueryMs = -1;
	protected int m_iRearGuideQueryEpoch;
	protected CF_EntityFollowWaypoint m_RearGuideQueryWaypoint;
	protected CF_OriginalFollowActivity m_RearGuideQueryActivity;
	protected CF_OriginalFollowLease m_RearGuideQueryLease;
	protected CF_TrailGuideEntity m_RearGuideQueryGuide;
	[Attribute(defvalue: "0", desc: "Private recorded prejoin lookahead comparison; real Query still owns first join")]
	protected bool m_bTrailPrejoinLookahead;
	protected int m_iPrejoinWindowLogs;
	protected ref CF_TrailGuideRoute m_TrailRoute;
	protected ref CF_DrivenRouteGuidance m_TrailGuidance;
	protected IEntity m_TrailPredecessor;
	protected CF_TrailGuideEntity m_TrailGuide;
	protected World m_TrailWorld;
	protected int m_iTrailEpoch;
	protected float m_fTrailNextRecordMs;
	protected float m_fTrailNextLogMs;
	protected float m_fTrailGuideStation;
	protected float m_fTrailGuideIssuedMs;
	protected float m_fTrailMeasuredBudget;
	protected float m_fTrailLastMeasured;
	protected float m_fTrailTurnCos = 1;
	protected float m_fTrailArcFactor = 1;
	protected vector m_vTrailLastFollower;
	protected vector m_vTrailEntry;
	protected vector m_vTrailEntryTangent;
	protected float m_fTrailEntryLength;
	protected bool m_bTrailEntryKnown;
	protected bool m_bTrailGuideActive;
	protected bool m_bTrailPoseInterrupted;
	protected bool m_bTrailFollowerPose;
	protected bool m_bTrailJoined;
	protected bool m_bTrailBlocked;
	protected int m_iTrailLogs;
	protected int m_iTrailQueryFailureLogs;
	protected int m_iTrailResumeRejectLogs;
	protected string m_sTrailBlockReason;
	protected static const ResourceName CF_TRAIL_GUIDE_PREFAB = "{983C8B13A242454E}Prefabs/Tests/CF_TrailGuide.et";
	protected static const ResourceName CF_TRAIL_WAYPOINT_PREFAB = "{F1418F3E681BA55E}Prefabs/Tests/AIWaypoint_CF_TrailGuide.et";

	protected void CF_InvalidateRearGuideQuery()
	{
		m_fRearGuideQueryMs = -1;
		m_iRearGuideQueryEpoch = 0;
		m_RearGuideQueryWaypoint = null;
		m_RearGuideQueryActivity = null;
		m_RearGuideQueryLease = null;
		m_RearGuideQueryGuide = null;
	}

	override protected void SetState(int state)
	{
		if (state != m_iState)
			CF_InvalidateRearGuideQuery();
		super.SetState(state);
	}

	override void CF_OnPlayerControlChanged(IEntity from, IEntity to)
	{
		CF_InvalidateRearGuideQuery();
		super.CF_OnPlayerControlChanged(from, to);
	}

	// Observes exact existing ownership. Failure never blocks, repairs, queries
	// the route, advances the guide or emits a native request.
	protected bool CF_ReadRearGuideBinding(IEntity predecessor, CF_EntityFollowWaypoint waypoint,
		CF_EntityFollowActivity activity, out string reason)
	{
		reason = "guide_context";
		if (!m_bTrailGuidePrototype || m_bTrailBlocked || m_bEntityFallbackFailed ||
			!m_bTrailGuideActive || !m_TrailGuide || !m_TrailRoute || !m_bTrailEntryKnown ||
			!m_bTrailFollowerPose || m_bTrailPoseInterrupted || !GetGame() || !m_TrailWorld ||
			GetGame().GetWorld() != m_TrailWorld || CF_ConvoySession.CF_IsWorldCleanup())
			return false;
		if (!predecessor || predecessor != m_TrailPredecessor || predecessor != m_LeadVehicle ||
			predecessor != GetTargetVehicle(false))
			return false;
		reason = "guide_not_tracking";
		if (!m_bTrailJoined || !m_TrailGuidance || !m_TrailGuidance.HasArcGap ||
			m_TrailGuidance.State != CF_DrivenRoute.TRACKING)
			return false;
		reason = "guide_target_binding";
		if (!CF_TrailGuideWaypoint.Cast(waypoint) || !activity ||
			waypoint.GetEntity() != m_TrailGuide || activity.m_Entity.m_Value != m_TrailGuide ||
			!CF_HasTrailGuideLease(m_TrailGuide, waypoint, m_iTrailEpoch) || !m_TrailGuide.CF_HasLease(waypoint))
			return false;
		CF_OriginalFollowActivity original = CF_OriginalFollowActivity.Cast(activity);
		CF_OriginalFollowLease lease = m_OriginalFollowLease;
		reason = "guide_original_lease";
		if (!original || !lease || original.Lease != lease || lease.Activity != original ||
			lease.Waypoint != waypoint || lease.NativeTarget != m_TrailGuide || lease.Predecessor != predecessor)
			return false;
		string executionReason;
		if (!lease.Executing(m_Group, executionReason))
		{
			reason = "guide_execution_" + executionReason;
			return false;
		}
		reason = "guide_exact_tracking";
		return true;
	}

	protected void CF_StampRearGuideQuery(CF_EntityFollowWaypoint waypoint, CF_EntityFollowActivity activity)
	{
		string reason;
		if (!CF_ReadRearGuideBinding(m_TrailPredecessor, waypoint, activity, reason))
			return;
		float now = m_TrailWorld.GetWorldTime();
		if (!(now >= 0) || now - now != 0)
			return;
		m_fRearGuideQueryMs = now;
		m_iRearGuideQueryEpoch = m_iTrailEpoch;
		m_RearGuideQueryWaypoint = waypoint;
		m_RearGuideQueryActivity = CF_OriginalFollowActivity.Cast(activity);
		m_RearGuideQueryLease = m_OriginalFollowLease;
		m_RearGuideQueryGuide = m_TrailGuide;
	}

	override protected bool CF_ReadRearPacingTarget(IEntity predecessor, CF_EntityFollowWaypoint waypoint,
		CF_EntityFollowActivity activity, out string reason, out string binding)
	{
		// An orphaned/mismatched guide must not fall back to direct-target admission.
		if (!m_bTrailGuideActive && !m_TrailGuide && !m_bTrailBlocked &&
			!CF_TrailGuideWaypoint.Cast(waypoint) && !CF_TrailGuideEntity.Cast(waypoint.GetEntity()) &&
			!CF_TrailGuideEntity.Cast(activity.m_Entity.m_Value))
			return super.CF_ReadRearPacingTarget(predecessor, waypoint, activity, reason, binding);
		binding = "binding=owned_guide epoch=" + m_iTrailEpoch + " joined=" + m_bTrailJoined;
		if (m_TrailGuidance) binding += " route_state=" + m_TrailGuidance.State;
		if (m_TrailGuide) binding += " guide_id=" + m_TrailGuide.GetID();
		if (predecessor) binding += " real_predecessor_id=" + predecessor.GetID();
		binding += " waypoint_id=" + waypoint.GetID() + " activity_sequence=" + activity.CF_GetSequence();
		if (m_OriginalFollowLease) binding += " lease_generation=" + m_OriginalFollowLease.Generation;
		float age = -1;
		if (m_TrailWorld && GetGame() && GetGame().GetWorld() == m_TrailWorld)
			age = m_TrailWorld.GetWorldTime() - m_fRearGuideQueryMs;
		binding += " query_age_ms=" + age;
		if (!CF_ReadRearGuideBinding(predecessor, waypoint, activity, reason))
			return false;
		reason = "guide_query_receipt";
		if (!(m_fRearGuideQueryMs >= 0) || !(age >= 0 && age <= 250) ||
			m_iRearGuideQueryEpoch != m_iTrailEpoch || m_RearGuideQueryWaypoint != waypoint ||
			m_RearGuideQueryActivity != activity || m_RearGuideQueryLease != m_OriginalFollowLease ||
			m_RearGuideQueryGuide != m_TrailGuide)
			return false;
		reason = "guide_fresh_exact_tracking";
		return true;
	}

	override protected bool CF_HasPersistentFollowFailure()
	{
		return m_bTrailGuidePrototype && m_bTrailBlocked;
	}

	override string CF_GetResumeFailureReason()
	{
		if (CF_HasPersistentFollowFailure())
			return "recorded route is blocked";
		return super.CF_GetResumeFailureReason();
	}

	override string CF_GetPanelStateLabel()
	{
		if (CF_HasPersistentFollowFailure() && !CF_IsControlBlocked() && !m_bPanelHoldRequested &&
			(m_iState == CF_FOLLOWING || m_iState == CF_ARRIVING))
			return "route blocked";
		return super.CF_GetPanelStateLabel();
	}

	override bool CF_CanPanelResume()
	{
		return !CF_HasPersistentFollowFailure() && super.CF_CanPanelResume();
	}

	override bool CF_PanelResume()
	{
		if (CF_HasPersistentFollowFailure())
		{
			if (Replication.IsServer() && m_iTrailResumeRejectLogs < 32)
			{
				m_iTrailResumeRejectLogs++;
				Print("[ConvoyFollower] TRAIL_GUIDE_RESUME_REJECTED: unit=" + m_iUnitNumber +
					" epoch=" + m_iTrailEpoch + " reason=" + m_sTrailBlockReason +
					" history_preserved=true new_order=false failure_cleared=false");
			}
			return false;
		}
		return super.CF_PanelResume();
	}

	override protected void StartFollowing(IEntity targetVehicle, bool rejoined, bool emitRadio)
	{
		// Guard before the parent releases a Wait or resets its stop episode.
		if (CF_HasPersistentFollowFailure())
			return;
		super.StartFollowing(targetVehicle, rejoined, emitRadio);
	}

	bool CF_IsTrailGuidePrototypeEnabled() { return m_bTrailGuidePrototype; }
	IEntity CF_GetTrailGuideEntity() { return m_TrailGuide; }
	IEntity CF_GetTrailRealPredecessor() { return m_TrailPredecessor; }
	bool CF_IsTrailGuideBlocked() { return m_bTrailBlocked; }
	bool CF_HasTrailJoined() { return m_bTrailJoined; }
	int CF_GetTrailEpoch() { return m_iTrailEpoch; }

	bool CF_HasTrailGuideLease(CF_TrailGuideEntity guide, CF_EntityFollowWaypoint waypoint, int epoch)
	{
		if (!m_bTrailGuidePrototype || m_bTrailBlocked || !guide || guide != m_TrailGuide || epoch != m_iTrailEpoch)
			return false;
		if (!GetGame() || !m_TrailWorld || GetGame().GetWorld() != m_TrailWorld || CF_ConvoySession.CF_IsWorldCleanup())
			return false;
		if (!waypoint || m_Waypoint != waypoint || waypoint.GetEntity() != guide || !m_Group || m_Group.GetAgentsCount() != 1)
			return false;
		if (!m_TrailPredecessor || m_TrailPredecessor != m_LeadVehicle || GetTargetVehicle(false) != m_TrailPredecessor)
			return false;
		return CF_IsOrdinaryEntityContext() && CF_IsLiveAIPilot(ChimeraCharacter.Cast(m_Driver), m_Truck);
	}

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (Replication.IsServer())
			Print("[ConvoyFollower] TRAIL_GUIDE_INIT: driver_id=" + owner.GetID() + " enabled=" + m_bTrailGuidePrototype +
				" prejoin_lookahead=" + m_bTrailPrejoinLookahead +
				" projection_correction_opt_in=" + m_bTrailProjectionCorrection +
				" rear_guide_receipt_max_ms=250 rear_guide_receipt_nonowning=true" +
				" record_s=0.2 query=each_frame lookahead_m=30 advance_remaining_m=15 native_capture_m=1" +
				" entry_window_m=5 entry_near_m=3 entry_corridor_m=2 route_corridor_m=5" +
				" actual_displacement_budget=true controls_writer=existing_controller test_only=true");
	}

	protected void CF_ResetTrailHistory(IEntity target)
	{
		CF_InvalidateRearGuideQuery();
		m_TrailPredecessor = target;
		m_TrailWorld = GetGame().GetWorld();
		m_iTrailEpoch++;
		m_iTrailProjectionLogs = 0;
		if (m_bTrailPrejoinLookahead)
			m_TrailRoute = new CF_TrailEntryWindowRoute();
		else
			m_TrailRoute = new CF_TrailGuideRoute();
		m_TrailGuidance = new CF_DrivenRouteGuidance();
		m_TrailRoute.Reset(m_iTrailEpoch, target.GetOrigin());
		m_bTrailFollowerPose = false;
		m_fTrailMeasuredBudget = 0;
		m_bTrailEntryKnown = false;
		m_bTrailPoseInterrupted = false;
		m_bTrailJoined = false;
		m_bTrailBlocked = false;
		m_sTrailBlockReason = "";
		m_fTrailNextRecordMs = 0;
		Print("[ConvoyFollower] TRAIL_GUIDE_SEED: unit=" + m_iUnitNumber + " epoch=" + m_iTrailEpoch +
			" predecessor_id=" + target.GetID() + " origin=" + target.GetOrigin() +
			" world_ms=" + m_TrailWorld.GetWorldTime() + " state=" + m_iState + " source=actual_predecessor");
	}

	protected void CF_BlockTrail(string reason)
	{
		CF_InvalidateRearGuideQuery();
		if (m_bTrailBlocked)
			return;
		m_bTrailBlocked = true;
		m_sTrailBlockReason = reason;
		Print("[ConvoyFollower] TRAIL_GUIDE_BLOCKED: unit=" + m_iUnitNumber + " epoch=" + m_iTrailEpoch +
			" reason=" + reason + " preserved_history=true forced_join=false");
		m_bEntityFallbackFailed = true;
		// Preserve explicit Hold, boarding and player ownership. Only the live
		// ordinary native pilot may enter the inherited blocked-arrival path.
		if (CF_IsOrdinaryEntityContext() && CF_IsLiveAIPilot(ChimeraCharacter.Cast(m_Driver), m_Truck))
			CF_FailEntityApproach("trail_guide_" + reason);
	}

	protected void CF_RecordTrail()
	{
		if (!m_bTrailGuidePrototype || !Replication.IsServer() || !GetGame() ||
			CF_ConvoySession.CF_IsWorldCleanup() || !m_Session || !m_Truck)
			return;
		IEntity target = GetTargetVehicle(false);
		if (!target || !Vehicle.Cast(target) || target == m_Truck || target.GetWorld() != GetGame().GetWorld())
		{
			if (m_TrailPredecessor && m_TrailGuide && !CF_IsControlBlocked())
				CF_BlockTrail("real_predecessor_unavailable");
			return;
		}
		if (target != m_TrailPredecessor)
		{
			// A real rechain is the only automatic route reset. Renumber and stop
			// retain their original history and route epoch.
			if (m_TrailGuide)
				ClearWaypoints();
			if (m_TrailGuide)
				return; // Deferred native retirement under another pilot.
			CF_ResetTrailHistory(target);
		}
		if (m_bTrailBlocked)
			return;
		float now = m_TrailWorld.GetWorldTime();
		if (now < m_fTrailNextRecordMs)
			return;
		m_fTrailNextRecordMs = now + 200;
		if (!m_TrailRoute.Record(m_iTrailEpoch, target.GetOrigin()))
			CF_BlockTrail("history_discontinuous_or_lost");
		if (!m_bTrailEntryKnown)
		{
			float endStation;
			m_bTrailEntryKnown = m_TrailRoute.ReadEntry(m_vTrailEntry, m_vTrailEntryTangent, m_fTrailEntryLength, endStation);
		}
	}

	protected void CF_MeasureTrailPose()
	{
		if (!m_TrailRoute || !m_Truck || !m_Driver || m_bTrailBlocked)
			return;
		vector pose = m_Truck.GetOrigin();
		if (!CF_IsLiveAIPilot(ChimeraCharacter.Cast(m_Driver), m_Truck) || CF_IsControlBlocked())
		{
			if (m_bTrailFollowerPose)
				m_bTrailPoseInterrupted = true;
			return;
		}
		if (m_bTrailFollowerPose)
		{
			float moved = vector.DistanceXZ(pose, m_vTrailLastFollower);
			if (moved > 5.0 || (m_bTrailPoseInterrupted && moved > 0.25))
			{
				CF_BlockTrail("unobserved_or_discontinuous_follower_motion");
				return;
			}
			m_fTrailMeasuredBudget += moved;
		}
		m_vTrailLastFollower = pose;
		m_bTrailFollowerPose = true;
		m_bTrailPoseInterrupted = false;
	}

	protected float CF_QueryTrailPose()
	{
		CF_InvalidateRearGuideQuery();
		float budget;
		string reason;
		m_fTrailLastMeasured = m_fTrailMeasuredBudget;
		m_fTrailMeasuredBudget = 0;
		bool queried;
		if (m_bTrailProjectionCorrection)
		{
			queried = m_TrailRoute.QueryPhysical(m_iTrailEpoch, m_Truck.GetOrigin(), m_fTrailLastMeasured,
				30.0, CF_ConvoySettings.Get().m_fMovingGap, 5.0,
				budget, m_fTrailTurnCos, m_fTrailArcFactor, reason, m_TrailGuidance);
		}
		else
		{
			queried = m_TrailRoute.PhysicalBudget(m_fTrailLastMeasured, budget, m_fTrailTurnCos, m_fTrailArcFactor, reason);
			if (queried)
				m_TrailRoute.Query(m_iTrailEpoch, m_Truck.GetOrigin(), 30.0,
					CF_ConvoySettings.Get().m_fMovingGap, budget, 5.0, m_TrailGuidance);
		}
		if (!queried)
		{
			CF_BlockTrail(reason);
			return 0;
		}
		if (m_TrailGuidance.HasProjectionCorrection && m_iTrailProjectionLogs < 12)
		{
			m_iTrailProjectionLogs++;
			Print("[ConvoyFollower] TRAIL_PROJECTION_CORRECTION: unit=" + m_iUnitNumber + " epoch=" + m_iTrailEpoch +
				" previous_pose=" + m_TrailGuidance.PreviousPhysicalPose + " incoming_start=" + m_TrailGuidance.PreviousIncomingStart +
				" current_pose=" + m_Truck.GetOrigin() + " measured_m=" + m_fTrailLastMeasured +
				" nominal_centerline_budget_m=" + m_TrailGuidance.ProjectionPhysicalBound +
				" signed_correction_m=" + m_TrailGuidance.ProjectionSignedCorrection +
				" candidate_advance_m=" + m_TrailGuidance.ProjectionCandidateAdvance + " station_coordinate=true physical_credit=false");
		}
		CF_LogTrailQueryFailure();
		return budget;
	}

	protected void CF_LogTrailQueryFailure()
	{
		if (!m_TrailGuidance || !m_TrailGuidance.HasAdvanceDiagnostic || m_iTrailQueryFailureLogs >= 4)
			return;
		m_iTrailQueryFailureLogs++;
		CF_DrivenRouteGuidance diagnostic = m_TrailGuidance;
		vector guideOrigin;
		vector guideForward;
		if (m_TrailGuide)
		{
			guideOrigin = m_TrailGuide.GetOrigin();
			guideForward = m_TrailGuide.GetWorldTransformAxis(2);
		}
		string context = "[ConvoyFollower] TRAIL_GUIDE_QUERY_FAILURE: unit=" + m_iUnitNumber;
		context += " previous_pose_available=" + diagnostic.HasPreviousPhysicalQuery;
		context += " previous_pose=" + diagnostic.PreviousPhysicalPose + " incoming_start=" + diagnostic.PreviousIncomingStart;
		context += " epoch=" + m_iTrailEpoch + " diagnostic=" + m_iTrailQueryFailureLogs + " world_ms=" + m_TrailWorld.GetWorldTime();
		context += " state=" + diagnostic.State + " queried_pose=" + diagnostic.DiagnosticPose;
		context += " prior_segment=" + diagnostic.DiagnosticPriorSegment + " prior_progress=" + diagnostic.DiagnosticPriorProgress;
		context += " segment=" + diagnostic.DiagnosticSegment + " loop_progress=" + diagnostic.DiagnosticLoopProgress;
		context += " candidate=" + diagnostic.DiagnosticCandidate + " budget_end=" + diagnostic.DiagnosticBudgetEnd;
		context += " excess_m=" + (diagnostic.DiagnosticCandidate - diagnostic.DiagnosticBudgetEnd) + " epsilon_m=0.001";
		context += " allowed_budget_m=" + diagnostic.DiagnosticBudget + " measured_step_m=" + m_fTrailLastMeasured;
		context += " local_turn_cos=" + m_fTrailTurnCos + " arc_factor=" + m_fTrailArcFactor;
		context += " guide_available=" + (m_TrailGuide != null) + " guide_origin=" + guideOrigin + " guide_forward=" + guideForward;
		if (m_TrailGuide)
			context += " guide_id=" + m_TrailGuide.GetID();
		if (m_TrailPredecessor)
			context += " predecessor_id=" + m_TrailPredecessor.GetID();
		context += " guide_station=" + m_fTrailGuideStation + " before_retirement=true decision_changed=false";
		Print(context);
		string geometry = "[ConvoyFollower] TRAIL_GUIDE_QUERY_GEOMETRY: unit=" + m_iUnitNumber;
		geometry += " epoch=" + m_iTrailEpoch + " diagnostic=" + m_iTrailQueryFailureLogs;
		geometry += " a=" + diagnostic.DiagnosticA + " b=" + diagnostic.DiagnosticB;
		geometry += " station_a=" + diagnostic.DiagnosticStationA + " station_b=" + diagnostic.DiagnosticStationB;
		geometry += " length=" + diagnostic.DiagnosticLength + " raw=" + diagnostic.DiagnosticRaw;
		geometry += " pose_from_a=" + (diagnostic.DiagnosticPose - diagnostic.DiagnosticA);
		geometry += " adjacent_available=" + diagnostic.DiagnosticAdjacentAvailable + " adjacent_raw=" + diagnostic.DiagnosticAdjacentRaw;
		geometry += " next=" + diagnostic.DiagnosticNext + " station_next=" + diagnostic.DiagnosticStationNext;
		geometry += " pose_from_b=" + (diagnostic.DiagnosticPose - diagnostic.DiagnosticB);
		geometry += " rounded_corner=" + diagnostic.DiagnosticRoundedCorner + " next_segment=" + diagnostic.DiagnosticNextSegment;
		geometry += " cross_track=" + diagnostic.CrossTrack + " corridor_error=" + diagnostic.DiagnosticCorridorError;
		Print(geometry);
	}

	protected void CF_DetachTrailGuide(bool deleteOwned)
	{
		CF_InvalidateRearGuideQuery();
		CF_TrailGuideEntity guide = m_TrailGuide;
		if (!guide)
			return;
		m_TrailGuide = null;
		m_bTrailGuideActive = false;
		guide.CF_Revoke();
		Print("[ConvoyFollower] TRAIL_GUIDE_RELEASE: unit=" + m_iUnitNumber + " guide_id=" + guide.GetID() +
			" epoch=" + m_iTrailEpoch + " delete_owned=" + deleteOwned + " vehicle_transform_written=false");
		if (deleteOwned)
			SCR_EntityHelper.DeleteEntityAndChildren(guide);
	}

	override protected void ClearWaypoints()
	{
		CF_InvalidateRearGuideQuery();
		super.ClearWaypoints();
		// Parent defers removal while controlled by another pilot. Preserve the
		// frozen guide then; deleting a live target could trigger late callbacks.
		if (!m_Waypoint && m_TrailGuide && !CF_IsControlBlocked())
			CF_DetachTrailGuide(!CF_ConvoySession.CF_IsWorldCleanup());
		if (!m_Waypoint && !CF_IsControlBlocked())
			m_bTrailGuideActive = false;
	}

	override bool CF_BeginConvoyAssignment(CF_ConvoySession session, IEntity user)
	{
		bool accepted = super.CF_BeginConvoyAssignment(session, user);
		if (accepted)
			CF_RecordTrail();
		return accepted;
	}

	override void CF_SetConvoyTarget(CF_DriverControllerComponent predecessor, int unitNumber)
	{
		CF_InvalidateRearGuideQuery();
		super.CF_SetConvoyTarget(predecessor, unitNumber);
		CF_RecordTrail();
	}

	protected bool CF_SetGuideStation(float station, string reason)
	{
		if (!m_TrailGuide || !m_TrailRoute || station < m_fTrailGuideStation - 0.001)
			return false;
		vector position;
		vector tangent;
		if (!m_TrailRoute.ReadRecorded(station, position, tangent))
			return false;
		vector delta = position - m_Truck.GetOrigin();
		vector facing = m_Truck.GetWorldTransformAxis(2);
		facing[1] = 0;
		if (facing.LengthSq() < 0.5)
			return false;
		facing.Normalize();
		if (vector.Dot(delta, facing) <= 1.0)
			return false;
		vector transform[4];
		Math3D.DirectionAndUpMatrix(tangent, vector.Up, transform);
		transform[3] = position;
		m_TrailGuide.SetTransform(transform);
		m_fTrailGuideStation = station;
		Print("[ConvoyFollower] TRAIL_GUIDE_ADVANCE: unit=" + m_iUnitNumber + " epoch=" + m_iTrailEpoch +
			" guide_id=" + m_TrailGuide.GetID() + " predecessor_id=" + m_TrailPredecessor.GetID() +
			" reason=" + reason + " station=" + station + " origin=" + position + " tangent=" + tangent +
			" joined=" + m_bTrailJoined + " waypoint_replaced=false source=recorded_only");
		return true;
	}

	// A forward offered target does not mean the truck has joined the route.
	// This path executes only after the unchanged Query returns APPROACH_START.
	protected void CF_OfferPrejoinWindow(vector pose, vector start, vector tangent, float endStation)
	{
		vector entryOffset = pose - start;
		entryOffset[1] = 0;
		vector entryLateral = entryOffset - tangent * vector.Dot(entryOffset, tangent);
		vector facing = m_Truck.GetWorldTransformAxis(2);
		facing[1] = 0;
		if (!(facing.LengthSq() >= 0.5))
		{
			CF_BlockTrail("prejoin_window_facing_invalid");
			return;
		}
		facing.Normalize();
		if (!(entryLateral.Length() <= 2.0 && vector.Dot(facing, tangent) >= 0.9))
		{
			CF_BlockTrail("prejoin_window_alignment_lost");
			return;
		}
		float recordedBound = endStation - CF_ConvoySettings.Get().m_fMovingGap;
		float requestedStation = Math.Min(30.0, recordedBound);
		if (requestedStation <= m_fTrailGuideStation + 1.0)
			return;
		CF_TrailEntryWindowRoute entryRoute = CF_TrailEntryWindowRoute.Cast(m_TrailRoute);
		float safeStation;
		string reason;
		if (!entryRoute || !entryRoute.ReadPrejoinWindow(m_iTrailEpoch, start, tangent, requestedStation, safeStation, reason))
		{
			CF_BlockTrail("prejoin_window_history_invalid");
			return;
		}
		if (safeStation < m_fTrailGuideStation - 0.001)
		{
			CF_BlockTrail("prejoin_window_history_changed");
			return;
		}
		if (safeStation <= m_fTrailGuideStation + 1.0)
			return;
		if (!CF_SetGuideStation(safeStation, "prejoin_recorded_lookahead"))
		{
			CF_BlockTrail("prejoin_window_not_forward");
			return;
		}
		// At least1m increase and at most30m per unjoined episode bounds these
		// records; the per-controller cap also covers repeated reassignments.
		if (m_iPrejoinWindowLogs < 32)
		{
			m_iPrejoinWindowLogs++;
			string line = "[ConvoyFollower] TRAIL_PREJOIN_WINDOW: unit=" + m_iUnitNumber;
			line += " epoch=" + m_iTrailEpoch + " joined=false cursor=" + m_TrailGuidance.Progress;
			line += " station=" + safeStation + " requested_station=" + requestedStation;
			line += " recorded_bound=" + recordedBound + " corridor_limit_m=2 reason=" + reason;
			line += " actual_pose=" + pose + " entry=" + start + " source=recorded_only";
			Print(line);
		}
	}

	protected bool CF_CreateTrailGuide()
	{
		if (m_bTrailBlocked)
			return true; // Handled failure; physical execution is separately gated.
		if (m_bTrailGuideActive)
		{
			if (m_TrailGuide && CF_HasTrailGuideLease(m_TrailGuide, CF_GetEntityFollowWaypoint(), m_iTrailEpoch) && HasOwnWaypointInGroup())
				return true; // Native retry retains this exact activity and guide.
			CF_BlockTrail("moving_retry_lost_owned_guide");
			return true;
		}
		CF_RecordTrail();
		if (!m_TrailRoute || m_TrailPredecessor != m_LeadVehicle)
		{
			CF_BlockTrail("history_not_seeded_at_assignment");
			return true;
		}
		if (!m_bTrailEntryKnown)
		{
			CF_BlockTrail("no_recorded_entry_segment");
			return true;
		}
		vector start = m_vTrailEntry;
		vector tangent = m_vTrailEntryTangent;
		float firstLength = m_fTrailEntryLength;
		vector offset = m_Truck.GetOrigin() - start;
		float along = vector.Dot(offset, tangent);
		vector lateral = offset - tangent * along;
		lateral[1] = 0;
		vector facing = m_Truck.GetWorldTransformAxis(2);
		facing[1] = 0;
		facing.Normalize();
		if (!m_bTrailJoined && (along > firstLength || lateral.Length() > 2.0 || vector.Dot(facing, tangent) < 0.9))
		{
			CF_BlockTrail("unsupported_initial_entry_geometry");
			return true;
		}
		float initialStation;
		if (m_bTrailJoined)
		{
			CF_QueryTrailPose();
			if (m_bTrailBlocked)
				return true;
			if (!m_TrailGuidance.HasArcGap || m_TrailGuidance.State != CF_DrivenRoute.TRACKING)
			{
				CF_BlockTrail("resume_route_not_physically_acquired");
				return true;
			}
			initialStation = Math.Min(m_TrailGuidance.Progress + 30.0,
				m_TrailGuidance.RecordedEnd - CF_ConvoySettings.Get().m_fMovingGap);
			if (!m_TrailRoute.ReadRecorded(initialStation, start, tangent))
			{
				CF_BlockTrail("resume_recorded_goal_unavailable");
				return true;
			}
		}
		Resource guideResource = Resource.Load(CF_TRAIL_GUIDE_PREFAB);
		Resource waypointResource = Resource.Load(CF_TRAIL_WAYPOINT_PREFAB);
		if (!guideResource.IsValid() || !waypointResource.IsValid())
		{
			CF_BlockTrail("guide_resources_unavailable");
			return true;
		}
		CF_ReleaseCapturedWait("new_trail_guide_order");
		ClearWaypoints();
		if (m_Waypoint || m_TrailGuide || CF_IsControlBlocked())
		{
			CF_BlockTrail("prior_order_retirement_deferred");
			return true;
		}
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		Math3D.DirectionAndUpMatrix(tangent, vector.Up, params.Transform);
		params.Transform[3] = start;
		m_TrailGuide = CF_TrailGuideEntity.Cast(GetGame().SpawnEntityPrefabLocal(guideResource, null, params));
		if (!m_TrailGuide)
		{
			CF_BlockTrail("guide_spawn_failed");
			return true;
		}
		m_TrailGuide.CF_Bind(this, m_TrailWorld, m_iTrailEpoch);
		CF_TrailGuideWaypoint waypoint = CF_TrailGuideWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(waypointResource, null, params));
		if (!waypoint)
		{
			CF_DetachTrailGuide(true);
			CF_BlockTrail("guide_waypoint_spawn_failed");
			return true;
		}
		waypoint.SetEntity(m_TrailGuide);
		waypoint.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_GAMEMASTER);
		waypoint.SetCompletionRadius(1.0);
		m_Waypoint = waypoint;
		m_vLastWaypointPosition = start;
		m_fWaypointSeconds = 0;
		m_fTrailGuideStation = initialStation;
		m_bTrailGuideActive = true;
		m_fTrailGuideIssuedMs = m_TrailWorld.GetWorldTime();
		CF_BindOriginalFollow(waypoint);
		m_Group.AddWaypoint(waypoint);
		Print("[ConvoyFollower] TRAIL_GUIDE_CREATED: unit=" + m_iUnitNumber + " epoch=" + m_iTrailEpoch +
			" guide_id=" + m_TrailGuide.GetID() + " predecessor_id=" + m_TrailPredecessor.GetID() +
			" waypoint_id=" + waypoint.GetID() + " entry_origin=" + m_vTrailEntry + " first_segment_m=" + firstLength +
			" guide_station=" + initialStation + " origin=" + start +
			" native_capture_m=1 route_standoff_m=" + CF_ConvoySettings.Get().m_fMovingGap + " forced_join=false");
		return true;
	}

	override protected bool CF_CreateEntityFollow(vector destination, float desiredDistance, string reason)
	{
		if (m_bTrailGuidePrototype && reason == "moving_follow")
			return CF_CreateTrailGuide();
		// The inherited real-vehicle stopped approach clears our owned guide
		// through ClearWaypoints, then preserves its unchanged10m capture/Wait.
		return super.CF_CreateEntityFollow(destination, desiredDistance, reason);
	}

	override protected bool IssueMoveWaypoint(vector destination)
	{
		if (m_bTrailGuidePrototype && m_bTrailBlocked)
			return true; // Base false path would StandDown/GetOut.
		return super.IssueMoveWaypoint(destination);
	}

	override protected bool MoveWaypoint(vector destination)
	{
		if (m_bTrailGuidePrototype && m_bTrailBlocked)
			return true; // Base false path would StandDown/GetOut.
		if (m_bTrailGuidePrototype && m_bTrailGuideActive && CF_IsOrdinaryEntityContext())
		{
			CF_EntityFollowWaypoint waypoint = CF_GetEntityFollowWaypoint();
			if (waypoint && waypoint.GetEntity() == m_TrailGuide && HasOwnWaypointInGroup())
				return true;
			CF_BlockTrail("owned_moving_waypoint_lost");
			return true;
		}
		return super.MoveWaypoint(destination);
	}

	protected void CF_UpdateTrailGuide()
	{
		CF_InvalidateRearGuideQuery();
		if (!m_bTrailGuideActive || m_bTrailBlocked || !CF_CanIssueEntityFollow())
			return;
		if (!m_TrailGuide)
		{
			CF_BlockTrail("owned_guide_deleted");
			return;
		}
		CF_EntityFollowWaypoint waypoint = CF_GetEntityFollowWaypoint();
		if (!CF_HasTrailGuideLease(m_TrailGuide, waypoint, m_iTrailEpoch) || !HasOwnWaypointInGroup())
		{
			CF_BlockTrail("moving_lease_or_waypoint_lost");
			return;
		}
		CF_EntityFollowActivity activity = waypoint.CF_GetActivity();
		float now = m_TrailWorld.GetWorldTime();
		if (activity && (activity.GetActionState() == EAIActionState.COMPLETED || activity.GetActionState() == EAIActionState.FAILED))
		{
			CF_BlockTrail("moving_activity_terminal");
			return;
		}
		if (activity && (activity.m_Entity.m_Value != m_TrailGuide || activity.m_RelatedWaypoint != waypoint))
		{
			CF_BlockTrail("moving_activity_binding_mismatch");
			return;
		}
		if (!activity && now - m_fTrailGuideIssuedMs > 5000)
		{
			CF_BlockTrail("moving_activity_not_created");
			return;
		}
		SCR_AIGroupUtilityComponent utility = SCR_AIGroupUtilityComponent.Cast(m_Group.FindComponent(SCR_AIGroupUtilityComponent));
		if (!activity || !utility || utility.GetCurrentAction() != activity || m_Group.GetCurrentWaypoint() != waypoint)
			return; // Never advance a guide on unselected or unrelated activity evidence.
		vector pose = m_Truck.GetOrigin();
		float budget = CF_QueryTrailPose();
		if (m_bTrailBlocked)
			return;
		float spacing = CF_ConvoySettings.Get().m_fMovingGap;
		vector start = m_vTrailEntry;
		vector tangent = m_vTrailEntryTangent;
		float firstLength = m_fTrailEntryLength;
		float endStation = m_TrailGuidance.RecordedEnd;
		if (!m_bTrailEntryKnown)
		{
			CF_BlockTrail("entry_history_unavailable");
			return;
		}
		float raw = vector.Dot(pose - start, tangent) / firstLength;
		if (m_TrailGuidance.HasArcGap)
		{
			if (!m_bTrailJoined)
				Print("[ConvoyFollower] TRAIL_GUIDE_JOIN: unit=" + m_iUnitNumber + " epoch=" + m_iTrailEpoch +
					" origin=" + pose + " progress=" + m_TrailGuidance.Progress + " actual_budget_m=" + budget + " forced=false");
			m_bTrailJoined = true;
			float nextStation = Math.Min(m_TrailGuidance.Progress + 30.0, endStation - spacing);
			if (m_fTrailGuideStation - m_TrailGuidance.Progress <= 15.0 && nextStation > m_fTrailGuideStation + 1.0)
			{
				if (!CF_SetGuideStation(nextStation, "recorded_lookahead"))
					CF_BlockTrail("recorded_goal_not_forward");
			}
		}
		else if (m_TrailGuidance.State == CF_DrivenRoute.APPROACH_START)
		{
			// Query ran at the actual pose first. It alone can establish entry.
			if (raw > 1.0 || m_bTrailJoined)
			{
				CF_BlockTrail("missed_original_entry_segment");
				return;
			}
			if (now - m_fTrailGuideIssuedMs > 30000)
			{
				CF_BlockTrail("entry_acquisition_timeout");
				return;
			}
			if (m_bTrailPrejoinLookahead)
				CF_OfferPrejoinWindow(pose, start, tangent, endStation);
			else if (vector.DistanceXZ(pose, start) <= 3.0 && endStation - spacing >= 5.0 && m_fTrailGuideStation < 5.0)
			{
				vector entryOffset = pose - start;
				vector entryLateral = entryOffset - tangent * vector.Dot(entryOffset, tangent);
				entryLateral[1] = 0;
				vector facing = m_Truck.GetWorldTransformAxis(2);
				facing[1] = 0;
				facing.Normalize();
				if (entryLateral.Length() > 2.0 || vector.Dot(facing, tangent) < 0.9)
				{
					CF_BlockTrail("entry_window_alignment_lost");
					return;
				}
				if (!CF_SetGuideStation(5.0, "recorded_entry_window"))
					CF_BlockTrail("entry_window_not_forward");
			}
		}
		else
		{
			CF_BlockTrail("route_state_" + m_TrailGuidance.State);
			return;
		}
		CF_StampRearGuideQuery(waypoint, activity);
		if (m_iTrailLogs < 480 && now >= m_fTrailNextLogMs && m_TrailGuide)
		{
			m_iTrailLogs++;
			m_fTrailNextLogMs = now + 1000;
			Print("[ConvoyFollower] TRAIL_GUIDE_SAMPLE: unit=" + m_iUnitNumber + " epoch=" + m_iTrailEpoch +
				" world_ms=" + now + " route_state=" + m_TrailGuidance.State + " joined=" + m_bTrailJoined +
				" progress=" + m_TrailGuidance.Progress + " recorded_end=" + endStation + " guide_station=" + m_fTrailGuideStation +
				" entry_raw=" + raw + " first_segment_m=" + firstLength + " actual_budget_m=" + budget +
				" measured_displacement_m=" + m_fTrailLastMeasured + " local_turn_cos=" + m_fTrailTurnCos + " arc_factor=" + m_fTrailArcFactor +
				" arc_gap=" + m_TrailGuidance.ArcGap + " arc_gap_valid=" + m_TrailGuidance.HasArcGap +
				" cross_track=" + m_TrailGuidance.CrossTrack + " guide_id=" + m_TrailGuide.GetID() +
				" predecessor_id=" + m_TrailPredecessor.GetID() + " waypoint_id=" + waypoint.GetID() + " origin=" + m_TrailGuide.GetOrigin() +
				" guide_forward=" + m_TrailGuide.GetWorldTransformAxis(2));
		}
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		CF_RecordTrail();
		CF_MeasureTrailPose();
		CF_UpdateTrailGuide();
		super.EOnFrame(owner, timeSlice);
	}

	override void CF_OnBeforePlayerPossess(IEntity entity)
	{
		CF_InvalidateRearGuideQuery();
		if (m_TrailGuide && entity && (entity == m_Driver || entity == m_Truck) && !CF_IsControlBlocked())
			ClearWaypoints();
		super.CF_OnBeforePlayerPossess(entity);
	}

	override void CF_DetachForWorldCleanup()
	{
		CF_InvalidateRearGuideQuery();
		CF_DetachTrailGuide(false);
		super.CF_DetachForWorldCleanup();
	}

	override protected void ResetToIdle()
	{
		CF_InvalidateRearGuideQuery();
		if (m_TrailGuide)
			ClearWaypoints();
		m_TrailRoute = null;
		m_TrailGuidance = null;
		m_TrailPredecessor = null;
		m_bTrailBlocked = false;
		m_bTrailJoined = false;
		super.ResetToIdle();
	}

	override void OnDelete(IEntity owner)
	{
		CF_InvalidateRearGuideQuery();
		if (CF_ConvoySession.CF_IsWorldCleanup())
			CF_DetachTrailGuide(false);
		else if (m_TrailGuide && !CF_IsControlBlocked())
			ClearWaypoints();
		else if (m_TrailGuide)
			CF_DetachTrailGuide(false); // Freeze orphan until world cleanup; no late native callback.
		super.OnDelete(owner);
	}
}
