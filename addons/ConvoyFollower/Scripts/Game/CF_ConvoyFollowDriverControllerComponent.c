class CF_ConvoyFollowDriverControllerComponentClass : CF_TrailGuideDriverControllerComponentClass
{
}

// Ordinary action backend. The session owns the chain; this component only
// selects the existing direct/head or recorded/tail moving-order path.
class CF_ConvoyFollowDriverControllerComponent : CF_TrailGuideDriverControllerComponent
{
	protected CF_ConvoySession m_RoleSession;
	protected CF_DriverControllerComponent m_RolePredecessor;
	protected IEntity m_RoleTarget;
	protected bool m_bRoleBound;
	protected bool m_bRolePending;
	protected bool m_bRoleRetirementRequested;
	protected int m_iRoleLogs;

	override void OnPostInit(IEntity owner)
	{
		// An unassigned recruit has neither a provisional player route nor
		// successor pressure. Commit the actual role after session admission.
		m_bTrailGuidePrototype = false;
		m_bEntityRearPacing = false;
		super.OnPostInit(owner);
	}

	protected bool CF_ReadRoleTarget(out IEntity target)
	{
		target = null;
		if (!Replication.IsServer() || !GetGame() || !GetGame().GetWorld() ||
			CF_ConvoySession.CF_IsWorldCleanup() || !m_Session || !m_Truck)
			return false;
		int number = m_Session.GetUnitNumber(this);
		if (number <= 0 || (!m_Predecessor && number != 1))
			return false;
		if (m_Predecessor && m_Session.CF_GetImmediateActiveSuccessor(m_Predecessor) != this)
			return false;
		target = GetTargetVehicle(false);
		return target && Vehicle.Cast(target) && target != m_Truck &&
			target.GetWorld() == GetGame().GetWorld() &&
			(!m_Predecessor || m_Predecessor.CF_GetAssignedVehicle() == target);
	}

	protected bool CF_RoleMatchesCurrent()
	{
		IEntity target;
		return m_bRoleBound && CF_ReadRoleTarget(target) && m_RoleSession == m_Session &&
			m_RolePredecessor == m_Predecessor && m_RoleTarget == target &&
			m_bTrailGuidePrototype == (m_Predecessor != null);
	}

	protected bool CF_RoleTransitionBlocks()
	{
		return m_Session && m_Session.GetUnitNumber(this) > 0 &&
			(m_bRolePending || (m_bRoleBound && !CF_RoleMatchesCurrent()));
	}

	protected void CF_ClearRetiredRoleTrail()
	{
		// Only called after the old owned waypoint, lease and guide are gone.
		CF_InvalidateRearGuideQuery();
		m_TrailRoute = null;
		m_TrailGuidance = null;
		m_TrailPredecessor = null;
		m_TrailWorld = null;
		m_bTrailGuideActive = false;
		m_bTrailEntryKnown = false;
		m_bTrailJoined = false;
		m_bTrailFollowerPose = false;
		m_bTrailPoseInterrupted = false;
		m_fTrailMeasuredBudget = 0;
		m_bTrailBlocked = false;
		m_sTrailBlockReason = "";
	}

	protected bool CF_ReconcileRole()
	{
		IEntity target;
		if (!CF_ReadRoleTarget(target))
			return false;
		if (!m_bRolePending && CF_RoleMatchesCurrent())
			return true; // Renumber, Hold and same-binding failure retain history.
		m_bRolePending = true;
		if (!m_bRoleRetirementRequested)
		{
			m_bRoleRetirementRequested = true;
			CF_InvalidateRearGuideQuery();
			CF_ResetRearPacing(-1, "ordinary_role_change");
			if (m_EntityCapturedWait && !m_EntityCapturedWait.IsPanelHold())
				CF_ReleaseCapturedWait("ordinary_role_change");
			// Clear only our exact original slot. The inherited observer owns
			// deferred retries and its timeout; never touch a seat waypoint.
			if (m_OriginalFollowLease && m_Waypoint == m_OriginalFollowLease.Waypoint &&
				!m_bOriginalRetireTimedOut && m_fOriginalRetireStartMs < 0)
			{
				CF_RevokeOriginalFollow("ordinary_role_change");
				ClearWaypoints();
			}
		}
		if (m_OriginalFollowLease || m_TrailGuide || CF_EntityFollowWaypoint.Cast(m_Waypoint) ||
			m_bOriginalRetireTimedOut || m_bOriginalFollowResetPending || m_bDeferredWaypointClear ||
			m_bDeferredControlDismiss || CF_IsControlBlocked())
			return false;
		string pilotReason;
		if (!CF_OriginalResumePilotReady(pilotReason))
			return false;

		bool tail = m_Predecessor != null;
		string oldFailure = m_sTrailBlockReason;
		CF_ClearRetiredRoleTrail();
		m_bTrailGuidePrototype = tail;
		m_bEntityRearPacing = !tail; // Match the two-truck comparison policy.
		m_RoleSession = m_Session;
		m_RolePredecessor = m_Predecessor;
		m_RoleTarget = target;
		m_bRoleBound = true;
		m_bRolePending = false;
		m_bRoleRetirementRequested = false;
		if (tail)
			CF_ResetTrailHistory(target);
		else
			m_iTrailEpoch++; // Invalidate any retired tail context on promotion.
		if (m_iRoleLogs < 64)
		{
			m_iRoleLogs++;
			Print("[ConvoyFollower] ORDINARY_FOLLOW_ROLE: unit=" + m_iUnitNumber +
				" tail_guide=" + tail + " head_rear_pacing=" + m_bEntityRearPacing +
				" target_id=" + target.GetID() + " epoch=" + m_iTrailEpoch +
				" old_route_failure=" + oldFailure + " old_native_slot_retired=true" +
				" panel_hold_preserved=" + m_bPanelHoldRequested + " movement_credit=false");
		}
		return true;
	}

	override protected void CF_RecordTrail()
	{
		if (m_bRoleBound && m_Session && m_Session.GetUnitNumber(this) <= 0)
		{
			// Explicit unload/return queues keep their own orders. On active
			// re-entry, do not reuse history recorded before leaving the chain.
			m_bRolePending = true;
			CF_InvalidateRearGuideQuery();
			return;
		}
		if (CF_ReconcileRole() && m_bTrailGuidePrototype)
			super.CF_RecordTrail();
	}

	override protected void CF_MeasureTrailPose()
	{
		if (!m_bRolePending && CF_RoleMatchesCurrent() && m_bTrailGuidePrototype)
			super.CF_MeasureTrailPose();
	}

	override protected bool CF_IsOrdinaryEntityContext()
	{
		return !m_bRolePending && CF_RoleMatchesCurrent() && super.CF_IsOrdinaryEntityContext();
	}

	override protected bool CF_CanIssueEntityFollow()
	{
		return !m_bRolePending && CF_RoleMatchesCurrent() && super.CF_CanIssueEntityFollow();
	}

	override protected bool CF_HasPersistentFollowFailure()
	{
		return CF_RoleTransitionBlocks() || super.CF_HasPersistentFollowFailure();
	}

	override string CF_GetResumeFailureReason()
	{
		if (CF_RoleTransitionBlocks())
			return "waiting for convoy target and owned order retirement";
		return super.CF_GetResumeFailureReason();
	}

	override string CF_GetPanelStateLabel()
	{
		if (CF_RoleTransitionBlocks() &&
			!CF_IsControlBlocked() && !m_bPanelHoldRequested)
			return "waiting for convoy link";
		return super.CF_GetPanelStateLabel();
	}

	override protected void StartFollowing(IEntity targetVehicle, bool rejoined, bool emitRadio)
	{
		if (!CF_ReconcileRole())
			return; // Before inherited Wait release or stop-episode reset.
		super.StartFollowing(targetVehicle, rejoined, emitRadio);
	}

	override bool CF_PanelResume()
	{
		if (!CF_ReconcileRole())
			return false;
		return super.CF_PanelResume();
	}

	override protected bool CF_CreateEntityFollow(vector destination, float desiredDistance, string reason)
	{
		if (!CF_ReconcileRole())
			return true; // Handled waiting, never replacement or movement credit.
		return super.CF_CreateEntityFollow(destination, desiredDistance, reason);
	}

	override protected bool IssueMoveWaypoint(vector destination)
	{
		if (CF_RoleTransitionBlocks() && (m_iState == CF_FOLLOWING || m_iState == CF_ARRIVING))
			return true; // Do not fall through to a base MOVE or StandDown/GetOut.
		return super.IssueMoveWaypoint(destination);
	}

	override protected bool MoveWaypoint(vector destination)
	{
		if (CF_RoleTransitionBlocks() && (m_iState == CF_FOLLOWING || m_iState == CF_ARRIVING))
			return true;
		return super.MoveWaypoint(destination);
	}

	override protected void ResetToIdle()
	{
		super.ResetToIdle();
		if (m_iState != CF_IDLE || m_bOriginalFollowResetPending || m_OriginalFollowLease || m_TrailGuide)
			return;
		m_RoleSession = null;
		m_RolePredecessor = null;
		m_RoleTarget = null;
		m_bRoleBound = false;
		m_bRolePending = false;
		m_bRoleRetirementRequested = false;
		m_bTrailGuidePrototype = false;
		m_bEntityRearPacing = false;
	}
}
