// Uses the established unparented Wait's exact-pilot retirement. Its lease
// is a pending recovery, never a claim that ordinary arrival has completed.
class CF_ArrivalRecoveryWait : CF_EntityCapturedWait
{
	void CF_ArrivalRecoveryWait(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity) {}

	override float CustomEvaluate()
	{
		if (!GetGame() || GetGame().GetWorld() != m_CaptureWorld || CF_ConvoySession.CF_IsWorldCleanup())
			return 0;
		if (GetActionState() == EAIActionState.COMPLETED || GetActionState() == EAIActionState.FAILED)
			return 0;
		CF_ConvoyFollowDriverControllerComponent controller = CF_ConvoyFollowDriverControllerComponent.Cast(m_CaptureController);
		if (!m_bCaptureLease || !HasOriginalPilot() || !controller || !controller.CF_HasArrivalRecoveryWait(this))
		{
			m_bCaptureLease = false;
			Complete(); // Unparented: no boarding/group failure callback.
			return 0;
		}
		return GetPriority();
	}
}

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
	protected ref CF_OriginalFollowLease m_ArrivalRecoveryLease;
	protected CF_ConvoyFollowDriverControllerComponent m_ArrivalRecoveryPredecessor;
	protected ref CF_ArrivalRecoveryWait m_ArrivalRecoveryWait;
	protected int m_iArrivalRecoveryEpoch;
	protected float m_fArrivalRecoveryStartMs;
	protected bool m_bArrivalRecoveryActive;
	protected bool m_bArrivalRecoveryTimedOut;
	protected bool m_bArrivalRecoveryInternalClear;
	protected bool m_bArrivalRecoveryWaitSelected;
	protected string m_sArrivalRecoveryReason;

	protected bool CF_IsRecoveryPredecessor(CF_ConvoySession session, IEntity truck)
	{
		return m_Session == session && m_Truck == truck && m_iState == CF_ARRIVING &&
			CF_RoleMatchesCurrent() && CF_IsOrdinaryEntityContext() && !m_bOriginalFollowBlocked &&
			!m_bEntityFallbackFailed && !m_bArrivalRoadRecoveryBlocked;
	}

	override bool CF_TryRecoverOriginalMoveFailure(CF_OriginalFollowLease lease, int result, int handler)
	{
		if (result != EMoveError.UNREACHABLE || !lease || lease.Revoked || lease.Failed ||
			lease != m_OriginalFollowLease || m_bArrivalRecoveryActive || m_bEntityStopAttempted)
			return false;
		if (!CF_RoleMatchesCurrent() || !m_bTrailGuidePrototype || !m_Predecessor ||
			m_bEntityFallbackFailed || m_bOriginalFollowBlocked || m_bTrailBlocked)
			return false;
		string reason;
		if (!lease.Executing(m_Group, reason) || !CF_ReadRearGuideBinding(m_LeadVehicle, lease.Waypoint, lease.Activity, reason))
			return false;
		AIGroupMovementComponent movement = AIGroupMovementComponent.Cast(m_Group.GetMovementComponent());
		if (!movement || handler <= AIGroupMovementComponent.DEFAULT_HANDLER_ID ||
			handler != movement.GetAgentMoveHandlerId(lease.Agent))
			return false;
		CF_ConvoyFollowDriverControllerComponent predecessor = CF_ConvoyFollowDriverControllerComponent.Cast(m_Predecessor);
		if (!predecessor || !predecessor.CF_IsRecoveryPredecessor(m_Session, m_LeadVehicle) ||
			m_Session.CF_GetImmediateActiveSuccessor(predecessor) != this)
			return false;
		// Use the last exact successful Query, not a new query or fabricated
		// cursor. Its receipt is invalidated on every update/ownership change.
		float now = m_TrailWorld.GetWorldTime();
		float age = now - m_fRearGuideQueryMs;
		if (!(age >= 0 && age <= 250) || m_iRearGuideQueryEpoch != m_iTrailEpoch ||
			m_RearGuideQueryLease != lease || m_RearGuideQueryWaypoint != lease.Waypoint ||
			m_RearGuideQueryActivity != lease.Activity || m_RearGuideQueryGuide != m_TrailGuide)
			return false;
		float remainder = m_TrailGuidance.RecordedEnd - CF_ConvoySettings.Get().m_fMovingGap - m_fTrailGuideStation;
		float gap = vector.Distance(m_Truck.GetOrigin(), m_LeadVehicle.GetOrigin());
		if (!(remainder >= 0 && remainder <= 1) || !(gap >= 0 && gap <= 30) ||
			!(m_TrailGuidance.ArcGap >= 0 && m_TrailGuidance.ArcGap <= 30))
			return false;
		m_ArrivalRecoveryLease = lease;
		m_ArrivalRecoveryPredecessor = predecessor;
		m_iArrivalRecoveryEpoch = m_iTrailEpoch;
		m_fArrivalRecoveryStartMs = now;
		m_bArrivalRecoveryActive = true;
		m_bArrivalRecoveryTimedOut = false;
		m_bArrivalRecoveryWaitSelected = false;
		m_sArrivalRecoveryReason = "waiting for predecessor to finish arrival";
		lease.RecordArrivalRecoveryFailure(result, handler);
		CF_InvalidateRearGuideQuery();
		Print("[ConvoyFollower] ARRIVAL_RECOVERY_ADMITTED: unit=" + m_iUnitNumber + " generation=" + lease.Generation +
			" sequence=" + lease.Activity.CF_GetSequence() + " result=" + result + " handler=" + handler +
			" epoch=" + m_iTrailEpoch + " predecessor_id=" + lease.Predecessor.GetID() +
			" real_gap_m=" + gap + " arc_gap_m=" + m_TrailGuidance.ArcGap + " terminal_remainder_m=" + remainder +
			" request_failed=true arrival_completed=false timeout_s=60 fresh_approaches_max=1");
		return true; // No native mutation from the behavior-tree callback.
	}

	protected bool CF_ArrivalRecoveryContext()
	{
		CF_OriginalFollowLease lease = m_ArrivalRecoveryLease;
		if (!m_bArrivalRecoveryActive || !lease || !CF_OriginalPilotSafe(lease) || !lease.ArrivalRecoveryOwned)
			return false;
		if (!CF_RoleMatchesCurrent() || !CF_IsOrdinaryEntityContext() || m_iTrailEpoch != m_iArrivalRecoveryEpoch ||
			lease.Session != m_Session || !m_Session || m_Session.GetUnitNumber(this) <= 0)
			return false;
		if (!m_ArrivalRecoveryPredecessor || m_Predecessor != m_ArrivalRecoveryPredecessor ||
			lease.Predecessor != m_LeadVehicle || GetTargetVehicle(false) != lease.Predecessor ||
			m_ArrivalRecoveryPredecessor.CF_GetAssignedVehicle() != lease.Predecessor)
			return false;
		if (m_Session.CF_GetImmediateActiveSuccessor(m_ArrivalRecoveryPredecessor) != this ||
			m_ArrivalRecoveryPredecessor.CF_IsControlBlocked() || !m_ArrivalRecoveryPredecessor.CF_IsBoarded())
			return false;
		if (m_bTrailBlocked || m_bOriginalFollowBlocked || m_bEntityFallbackFailed || lease.HasSeatWork())
			return false;
		SCR_DamageManagerComponent damage = SCR_DamageManagerComponent.GetDamageManager(lease.Predecessor);
		return !damage || !damage.IsDestroyed();
	}

	bool CF_HasArrivalRecoveryWait(CF_ArrivalRecoveryWait action)
	{
		if (!action || action != m_ArrivalRecoveryWait || !CF_ArrivalRecoveryContext() || m_Waypoint || m_TrailGuide)
			return false;
		if (!CF_CapturedWaitOwnershipReady(ChimeraCharacter.Cast(m_Driver), m_Truck, m_Group)) return false;
		return action.HasOriginalBinding(this, ChimeraCharacter.Cast(m_Driver), m_Truck, m_Group,
			m_Session, m_Leader, m_iOrderingPlayerId);
	}

	protected void CF_CancelArrivalRecovery(string reason, bool allowNativeCompletion = true)
	{
		if (!m_bArrivalRecoveryActive && !m_ArrivalRecoveryWait) return;
		CF_ArrivalRecoveryWait action = m_ArrivalRecoveryWait;
		m_bArrivalRecoveryActive = false;
		m_ArrivalRecoveryWait = null;
		if (action) action.Retire(allowNativeCompletion && !CF_IsControlBlocked());
		Print("[ConvoyFollower] ARRIVAL_RECOVERY_RELEASE: unit=" + m_iUnitNumber + " reason=" + reason +
			" wait_owned=" + (action != null) + " only_owned_wait=true arrival_completed=false");
		m_ArrivalRecoveryLease = null;
		m_ArrivalRecoveryPredecessor = null;
		m_bArrivalRecoveryTimedOut = false;
	}

	protected void CF_TimeoutArrivalRecovery(string reason)
	{
		if (m_bArrivalRecoveryTimedOut) return;
		m_bArrivalRecoveryTimedOut = true;
		m_sArrivalRecoveryReason = reason;
		Print("[ConvoyFollower] ARRIVAL_RECOVERY_BLOCKED: unit=" + m_iUnitNumber + " reason=" + reason +
			" stays_seated=true automatic_retry=false explicit_resume_revalidates=true");
	}

	protected bool CF_RecoveryHandoffReady()
	{
		if (!CF_ArrivalRecoveryContext() || !m_ArrivalRecoveryWait || m_Waypoint || m_TrailGuide || m_OriginalFollowLease)
			return false;
		if (!CF_HasArrivalRecoveryWait(m_ArrivalRecoveryWait) || !CF_IsPanelVehicleSlow(2.0)) return false;
		SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(m_ArrivalRecoveryLease.Agent.FindComponent(SCR_AIUtilityComponent));
		if (!utility || utility.GetCurrentBehavior() != m_ArrivalRecoveryWait ||
			m_ArrivalRecoveryWait.GetActionState() == EAIActionState.COMPLETED ||
			m_ArrivalRecoveryWait.GetActionState() == EAIActionState.FAILED)
			return false;
		float gap = vector.Distance(m_Truck.GetOrigin(), m_LeadVehicle.GetOrigin());
		return gap >= 0 && gap <= 30 && m_ArrivalRecoveryPredecessor.CF_HasSelectedArrivalWait(m_Session, m_LeadVehicle);
	}

	protected bool CF_FinishArrivalRecovery()
	{
		if (!CF_RecoveryHandoffReady()) return false;
		int oldGeneration = m_ArrivalRecoveryLease.Generation;
		CF_CancelArrivalRecovery("predecessor_selected_arrival_wait");
		SetState(CF_ARRIVING);
		// Start this stopped episode at the current predecessor sample. Movement
		// during recovery must not reset the fresh approach on the next settle tick.
		m_vLastTargetPosition = m_LeadVehicle.GetOrigin();
		m_fTargetStillSeconds = 0;
		// No still-time, stable-pose or capture credit. The normal new real
		// predecessor approach must subsequently satisfy all arrival gates.
		bool issued = CF_BeginStoppedEntityApproach();
		if (!issued || !m_OriginalFollowLease || m_OriginalFollowLease.Generation <= oldGeneration ||
			m_OriginalFollowLease.NativeTarget != m_LeadVehicle || !HasOwnWaypointInGroup())
		{
			CF_FailEntityApproach("arrival_recovery_fresh_approach_not_created");
			return false;
		}
		Print("[ConvoyFollower] ARRIVAL_RECOVERY_APPROACH: unit=" + m_iUnitNumber + " failed_generation=" + oldGeneration +
			" fresh_generation=" + m_OriginalFollowLease.Generation + " predecessor_id=" + m_LeadVehicle.GetID() +
			" waypoint_id=" + m_Waypoint.GetID() + " stopped_gap_m=" + CF_ConvoySettings.Get().m_fStoppedGap +
			" predecessor_wait_selected=true arrival_completed=false physical_credit=0");
		return true;
	}

	protected void CF_UpdateArrivalRecovery()
	{
		if (!m_bArrivalRecoveryActive) return;
		if (!GetGame() || CF_ConvoySession.CF_IsWorldCleanup() || CF_IsControlBlocked())
		{
			CF_CancelArrivalRecovery("world_or_direct_control_handover", false);
			return;
		}
		if (!CF_ArrivalRecoveryContext())
		{
			bool safe = m_ArrivalRecoveryLease && CF_OriginalPilotSafe(m_ArrivalRecoveryLease);
			CF_CancelArrivalRecovery("recovery_ownership_or_context_lost", safe);
			CF_BlockTrail("arrival_recovery_guard_lost");
			return;
		}
		// Absolute bound includes retirement and slowing, not just time after
		// Wait selection. The base observer must also stop deferred retries.
		if (!m_bArrivalRecoveryTimedOut && GetGame().GetWorld().GetWorldTime() - m_fArrivalRecoveryStartMs >= 60000)
		{
			CF_TimeoutArrivalRecovery("predecessor_arrival_timeout_60s");
			if (m_Waypoint || m_OriginalFollowLease) m_bOriginalRetireTimedOut = true;
		}
		if (m_bArrivalRecoveryTimedOut && (m_Waypoint || m_OriginalFollowLease || m_TrailGuide)) return;
		if (m_Waypoint)
		{
			if (m_OriginalFollowLease != m_ArrivalRecoveryLease || m_Waypoint != m_ArrivalRecoveryLease.Waypoint)
			{
				CF_CancelArrivalRecovery("different_owned_slot", false);
				// Do not clear/fail a newly unrelated native order.
				m_bTrailBlocked = true;
				m_bEntityFallbackFailed = true;
				m_sTrailBlockReason = "arrival_recovery_slot_changed";
				Print("[ConvoyFollower] ARRIVAL_RECOVERY_GUARD_FAILED: unit=" + m_iUnitNumber + " reason=owned_slot_changed unrelated_order_untouched=true");
				return;
			}
			m_bArrivalRecoveryInternalClear = true;
			ClearWaypoints();
			m_bArrivalRecoveryInternalClear = false;
		}
		if (m_Waypoint || m_OriginalFollowLease || m_TrailGuide) return;
		if (!m_ArrivalRecoveryWait && !m_bArrivalRecoveryTimedOut && CF_IsPanelVehicleSlow(2.0) &&
			CF_CapturedWaitOwnershipReady(ChimeraCharacter.Cast(m_Driver), m_Truck, m_Group))
		{
			SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(m_ArrivalRecoveryLease.Agent.FindComponent(SCR_AIUtilityComponent));
			if (!utility) { CF_TimeoutArrivalRecovery("pilot_utility_missing"); return; }
			m_ArrivalRecoveryWait = new CF_ArrivalRecoveryWait(utility, null);
			m_ArrivalRecoveryWait.Bind(this, ChimeraCharacter.Cast(m_Driver), m_Truck, m_Group, m_Session, m_Leader, m_iOrderingPlayerId, false);
			utility.AddAction(m_ArrivalRecoveryWait);
			Print("[ConvoyFollower] ARRIVAL_RECOVERY_WAIT_BIND: unit=" + m_iUnitNumber + " wait=" + m_ArrivalRecoveryWait.ToString() +
				" driver_id=" + m_Driver.GetID() + " truck_id=" + m_Truck.GetID() + " predecessor_id=" + m_LeadVehicle.GetID());
		}
		if (m_ArrivalRecoveryWait)
		{
			if (m_ArrivalRecoveryWait.GetActionState() == EAIActionState.COMPLETED || m_ArrivalRecoveryWait.GetActionState() == EAIActionState.FAILED)
			{ CF_TimeoutArrivalRecovery("owned_recovery_wait_terminal"); return; }
			SCR_AIUtilityComponent selectedUtility = SCR_AIUtilityComponent.Cast(m_ArrivalRecoveryLease.Agent.FindComponent(SCR_AIUtilityComponent));
			bool selected = selectedUtility && selectedUtility.GetCurrentBehavior() == m_ArrivalRecoveryWait;
			if (selected != m_bArrivalRecoveryWaitSelected)
			{
				m_bArrivalRecoveryWaitSelected = selected;
				Print("[ConvoyFollower] ARRIVAL_RECOVERY_WAIT_SELECTED: unit=" + m_iUnitNumber + " selected=" + selected +
					" wait=" + m_ArrivalRecoveryWait.ToString() + " arrival_completed=false");
			}
		}
		if (m_bArrivalRecoveryTimedOut) return;
		CF_FinishArrivalRecovery();
	}

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
		if (m_bArrivalRecoveryActive) return; // Preserve the admitted history while waiting.
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
		if (m_bArrivalRecoveryActive) return;
		if (!m_bRolePending && CF_RoleMatchesCurrent() && m_bTrailGuidePrototype)
			super.CF_MeasureTrailPose();
	}

	override protected bool CF_IsOrdinaryEntityContext()
	{
		return !m_bRolePending && CF_RoleMatchesCurrent() && super.CF_IsOrdinaryEntityContext();
	}

	override protected bool CF_CanIssueEntityFollow()
	{
		return !m_bArrivalRecoveryActive && !m_bRolePending && CF_RoleMatchesCurrent() && super.CF_CanIssueEntityFollow();
	}

	override protected bool CF_HasPersistentFollowFailure()
	{
		// Reuses the existing post-lifecycle navigation pause and sole zero
		// cruise owner. It does not set an arrival/capture success latch.
		return m_bArrivalRecoveryActive || CF_RoleTransitionBlocks() || super.CF_HasPersistentFollowFailure();
	}

	override string CF_GetResumeFailureReason()
	{
		if (m_bArrivalRecoveryActive) return m_sArrivalRecoveryReason;
		if (CF_RoleTransitionBlocks())
			return "waiting for convoy target and owned order retirement";
		return super.CF_GetResumeFailureReason();
	}

	override string CF_GetPanelStateLabel()
	{
		if (m_bArrivalRecoveryActive && !CF_IsControlBlocked() && !m_bPanelHoldRequested)
		{
			if (m_bArrivalRecoveryTimedOut) return "arrival recovery blocked; Resume when predecessor is parked";
			return "waiting for predecessor to finish arrival";
		}
		if (CF_RoleTransitionBlocks() &&
			!CF_IsControlBlocked() && !m_bPanelHoldRequested)
			return "waiting for convoy link";
		return super.CF_GetPanelStateLabel();
	}

	override protected void StartFollowing(IEntity targetVehicle, bool rejoined, bool emitRadio)
	{
		if (m_bArrivalRecoveryActive) return;
		if (!CF_ReconcileRole())
			return; // Before inherited Wait release or stop-episode reset.
		super.StartFollowing(targetVehicle, rejoined, emitRadio);
	}

	override bool CF_CanPanelResume()
	{
		if (m_bArrivalRecoveryActive)
		{
			if (CF_RecoveryHandoffReady()) return true;
			if (!m_bArrivalRecoveryTimedOut || !CF_ArrivalRecoveryContext() ||
				!m_ArrivalRecoveryPredecessor.CF_HasSelectedArrivalWait(m_Session, m_LeadVehicle)) return false;
			if (m_Waypoint)
				return m_OriginalFollowLease == m_ArrivalRecoveryLease && m_Waypoint == m_ArrivalRecoveryLease.Waypoint &&
					CF_OriginalCanRetire(m_ArrivalRecoveryLease);
			return !m_OriginalFollowLease && !m_TrailGuide && !m_ArrivalRecoveryWait &&
				CF_CapturedWaitOwnershipReady(ChimeraCharacter.Cast(m_Driver), m_Truck, m_Group);
		}
		return super.CF_CanPanelResume();
	}

	override bool CF_PanelResume()
	{
		if (m_bArrivalRecoveryActive)
		{
			if (!Replication.IsServer() || !CF_CanPanelResume()) return false;
			bool applied;
			if (CF_RecoveryHandoffReady()) applied = CF_FinishArrivalRecovery();
			else
			{
				// Only an explicit command can reopen a timed-out retirement or
				// Wait acquisition, and only after that predecessor is parked.
				m_bArrivalRecoveryTimedOut = false;
				m_bOriginalRetireTimedOut = false;
				m_fOriginalRetireStartMs = -1;
				m_fArrivalRecoveryStartMs = GetGame().GetWorld().GetWorldTime();
				m_sArrivalRecoveryReason = "waiting for owned arrival handoff";
				applied = true; // Accepted pending work, not movement completion.
			}
			Print("[ConvoyFollower] ARRIVAL_RECOVERY_RESUME: unit=" + m_iUnitNumber + " accepted=" + applied +
				" physical_completion_claim=false same_predecessor_required=true");
			return applied;
		}
		if (!CF_ReconcileRole())
			return false;
		return super.CF_PanelResume();
	}

	override protected bool CF_CreateEntityFollow(vector destination, float desiredDistance, string reason)
	{
		if (m_bArrivalRecoveryActive) return true;
		if (!CF_ReconcileRole())
			return true; // Handled waiting, never replacement or movement credit.
		return super.CF_CreateEntityFollow(destination, desiredDistance, reason);
	}

	override protected bool IssueMoveWaypoint(vector destination)
	{
		if (m_bArrivalRecoveryActive) return true;
		if (CF_RoleTransitionBlocks() && (m_iState == CF_FOLLOWING || m_iState == CF_ARRIVING))
			return true; // Do not fall through to a base MOVE or StandDown/GetOut.
		return super.IssueMoveWaypoint(destination);
	}

	override protected bool MoveWaypoint(vector destination)
	{
		if (m_bArrivalRecoveryActive) return true;
		if (CF_RoleTransitionBlocks() && (m_iState == CF_FOLLOWING || m_iState == CF_ARRIVING))
			return true;
		return super.MoveWaypoint(destination);
	}

	override protected void ResetToIdle()
	{
		CF_CancelArrivalRecovery("reset_to_idle", !CF_ConvoySession.CF_IsWorldCleanup());
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

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		CF_UpdateArrivalRecovery();
		super.EOnFrame(owner, timeSlice);
	}

	override protected bool CF_ShouldHoldVehicleBrake()
	{
		if (m_bArrivalRecoveryActive && CF_ArrivalRecoveryContext()) return true;
		return super.CF_ShouldHoldVehicleBrake();
	}

	override protected void ClearWaypoints()
	{
		if (!m_bArrivalRecoveryInternalClear) CF_CancelArrivalRecovery("external_owned_order_clear");
		super.ClearWaypoints();
	}

	override protected void SetState(int state)
	{
		if (m_bArrivalRecoveryActive && state != m_iState)
			CF_CancelArrivalRecovery("state_change");
		super.SetState(state);
	}

	override bool CF_RequestPanelHold()
	{
		if (Replication.IsServer() && CF_CanApproachPanelHold())
			CF_CancelArrivalRecovery("explicit_hold");
		return super.CF_RequestPanelHold();
	}

	override void CF_SetConvoyTarget(CF_DriverControllerComponent predecessor, int unitNumber)
	{
		if (predecessor != m_Predecessor) CF_CancelArrivalRecovery("predecessor_changed");
		super.CF_SetConvoyTarget(predecessor, unitNumber);
	}

	override void CF_OnBeforePlayerPossess(IEntity entity)
	{
		if (entity && (entity == m_Driver || entity == m_Truck)) CF_CancelArrivalRecovery("before_player_possession");
		super.CF_OnBeforePlayerPossess(entity);
	}

	override void CF_OnPlayerControlChanged(IEntity from, IEntity to)
	{
		if (to && (to == m_Driver || to == m_Truck)) CF_CancelArrivalRecovery("late_player_control_change", false);
		super.CF_OnPlayerControlChanged(from, to);
	}

	override void CF_DetachForWorldCleanup()
	{
		CF_CancelArrivalRecovery("world_cleanup", false);
		super.CF_DetachForWorldCleanup();
	}

	override void OnDelete(IEntity owner)
	{
		CF_CancelArrivalRecovery("controller_deleted", !CF_ConvoySession.CF_IsWorldCleanup());
		super.OnDelete(owner);
	}
}
