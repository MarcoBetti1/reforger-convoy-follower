class CF_TrailGuideDriverControllerComponentClass : CF_EntityFollowDriverControllerComponentClass
{
}

// NEW private prefab only. Real predecessor remains in the inherited session,
// stop, spacing, health and cruise paths. Only the moving native target differs.
class CF_TrailGuideDriverControllerComponent : CF_EntityFollowDriverControllerComponent
{
	[Attribute(defvalue: "0", desc: "Private one-follower recorded trail-guide experiment")]
	protected bool m_bTrailGuidePrototype;
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
	protected string m_sTrailBlockReason;
	protected static const ResourceName CF_TRAIL_GUIDE_PREFAB = "{983C8B13A242454E}Prefabs/Tests/CF_TrailGuide.et";
	protected static const ResourceName CF_TRAIL_WAYPOINT_PREFAB = "{F1418F3E681BA55E}Prefabs/Tests/AIWaypoint_CF_TrailGuide.et";

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
				" record_s=0.2 query=each_frame lookahead_m=30 advance_remaining_m=15 native_capture_m=1" +
				" entry_window_m=5 entry_near_m=3 entry_corridor_m=2 route_corridor_m=5" +
				" actual_displacement_budget=true controls_writer=existing_controller test_only=true");
	}

	protected void CF_ResetTrailHistory(IEntity target)
	{
		m_TrailPredecessor = target;
		m_TrailWorld = GetGame().GetWorld();
		m_iTrailEpoch++;
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
		float budget;
		string reason;
		m_fTrailLastMeasured = m_fTrailMeasuredBudget;
		m_fTrailMeasuredBudget = 0;
		if (!m_TrailRoute.PhysicalBudget(m_fTrailLastMeasured, budget, m_fTrailTurnCos, m_fTrailArcFactor, reason))
		{
			CF_BlockTrail(reason);
			return 0;
		}
		m_TrailRoute.Query(m_iTrailEpoch, m_Truck.GetOrigin(), 30.0,
			CF_ConvoySettings.Get().m_fMovingGap, budget, 5.0, m_TrailGuidance);
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
			if (vector.DistanceXZ(pose, start) <= 3.0 && endStation - spacing >= 5.0 && m_fTrailGuideStation < 5.0)
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
		if (m_TrailGuide && entity && (entity == m_Driver || entity == m_Truck) && !CF_IsControlBlocked())
			ClearWaypoints();
		super.CF_OnBeforePlayerPossess(entity);
	}

	override void CF_DetachForWorldCleanup()
	{
		CF_DetachTrailGuide(false);
		super.CF_DetachForWorldCleanup();
	}

	override protected void ResetToIdle()
	{
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
		if (CF_ConvoySession.CF_IsWorldCleanup())
			CF_DetachTrailGuide(false);
		else if (m_TrailGuide && !CF_IsControlBlocked())
			ClearWaypoints();
		else if (m_TrailGuide)
			CF_DetachTrailGuide(false); // Freeze orphan until world cleanup; no late native callback.
		super.OnDelete(owner);
	}
}
