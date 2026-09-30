// Gameplay lease used by ordinary CF_ConvoyFollowDriverControllerComponent
// prefabs. The Tests path reflects its development origin. Authored orchestration;
// no copied native tree. Standalone base components still default to opt-in.
// References are weak except the lease shared by the controller/waypoint/activity.
class CF_OriginalFollowLease
{
	CF_EntityFollowDriverControllerComponent Controller;
	ChimeraCharacter Driver;
	Vehicle Truck;
	IEntity Predecessor;
	IEntity NativeTarget;
	SCR_AIGroup Group;
	AIAgent Agent;
	SCR_AIGroupUtilityComponent Utility;
	CF_ConvoySession Session;
	CF_EntityFollowWaypoint Waypoint;
	CF_OriginalFollowActivity Activity;
	World OriginalWorld;
	int Generation;
	float Distance;
	float PriorityLevel;
	bool Revoked;
	bool Failed;
	// A typed native failure accepted by the controller for a separate,
	// bounded arrival recovery. This failed lease can never execute again.
	bool ArrivalRecoveryOwned;
	protected int m_Logs;
	protected int m_Activation;
	protected int m_Requests;
	protected string m_LastReason;
	protected ref CF_OriginalFollowIdentity m_Binding;
	protected bool m_RequestOpen;
	protected int m_CompletedRequests;
	protected int m_HeartbeatCount;
	protected float m_NextHeartbeatMs;

	void Heartbeat()
	{
		if (!GetGame() || !OriginalWorld || GetGame().GetWorld() != OriginalWorld || m_HeartbeatCount >= 180)
			return;
		float now = OriginalWorld.GetWorldTime();
		if (now < m_NextHeartbeatMs)
			return;
		m_NextHeartbeatMs = now + 2000;
		m_HeartbeatCount++;
		string line = "[ConvoyFollower] ORIGINAL_FOLLOW_HEARTBEAT: generation=" + Generation;
		line += " world_ms=" + now + " activation=" + m_Activation + " requests=" + m_Requests;
		line += " request_completions=" + m_CompletedRequests + " request_interval_open=" + m_RequestOpen;
		line += " selected_exact=" + (Utility && Utility.GetCurrentAction() == Activity);
		line += " executed_exact=" + (Utility && Utility.GetExecutedAction() == Activity);
		if (Activity) line += " sequence=" + Activity.CF_GetSequence() + " action_state=" + Activity.GetActionState();
		if (Waypoint) line += " waypoint_id=" + Waypoint.GetID();
		if (Truck) line += " truck_id=" + Truck.GetID();
		if (Driver) line += " driver_id=" + Driver.GetID();
		if (Predecessor) line += " predecessor_id=" + Predecessor.GetID();
		if (NativeTarget) line += " native_target_id=" + NativeTarget.GetID();
		line += " observer=tree_guard request_state_basis=node_edges native_internal_state_claim=false";
		Print(line);
		if (m_HeartbeatCount == 180)
			Print("[ConvoyFollower] ORIGINAL_FOLLOW_HEARTBEAT_LIMIT: generation=" + Generation + " limit=180 guards_continue=true");
	}

	// Captured once before AddWaypoint. Subsequent binding changes invalidate
	// both movement and retirement; lifecycle flags/activity are separate.
	void Seal()
	{
		if (!m_Binding)
			m_Binding = new CF_OriginalFollowIdentity(this);
	}

	bool BindingUnchanged()
	{
		return m_Binding && m_Binding.Matches(this);
	}

	void Record(string reason)
	{
		if (reason == m_LastReason || m_Logs >= 64)
			return;
		m_LastReason = reason;
		m_Logs++;
		string line = "[ConvoyFollower] ORIGINAL_FOLLOW_LEASE: generation=" + Generation;
		line += " reason=" + reason + " revoked=" + Revoked + " failed=" + Failed;
		line += " activation=" + m_Activation + " requests=" + m_Requests;
		if (Waypoint) line += " waypoint_id=" + Waypoint.GetID();
		if (NativeTarget) line += " native_target_id=" + NativeTarget.GetID();
		if (Predecessor) line += " predecessor_id=" + Predecessor.GetID();
		if (Activity) line += " sequence=" + Activity.CF_GetSequence();
		Print(line);
		if (m_Logs == 64)
			Print("[ConvoyFollower] ORIGINAL_FOLLOW_LOG_LIMIT: generation=" + Generation + " limit=64 guards_continue=true");
	}

	void Revoke(string reason)
	{
		Revoked = true;
		Record(reason);
	}

	bool HasSeatWork()
	{
		if (!Utility || !Agent)
			return true;
		array<ref AIActionBase> actions = {};
		Utility.GetActions(actions);
		foreach (AIActionBase action : actions)
		{
			if (!action || action.GetActionState() == EAIActionState.COMPLETED || action.GetActionState() == EAIActionState.FAILED)
				continue;
			if (SCR_AIGetInActivity.Cast(action) || SCR_AIGetOutActivity.Cast(action))
				return true;
		}
		SCR_AIUtilityComponent pilotUtility = SCR_AIUtilityComponent.Cast(Agent.FindComponent(SCR_AIUtilityComponent));
		if (!pilotUtility)
			return true;
		actions.Clear();
		pilotUtility.GetActions(actions);
		foreach (AIActionBase behavior : actions)
		{
			if (!behavior || behavior.GetActionState() == EAIActionState.COMPLETED || behavior.GetActionState() == EAIActionState.FAILED)
				continue;
			if (SCR_AIGetInVehicle.Cast(behavior) || SCR_AIGetOutVehicle.Cast(behavior))
				return true;
		}
		return false;
	}

	bool Admit(SCR_AIGroupUtilityComponent utility, out string reason)
	{
		if (Revoked || Failed || !Controller || !BindingUnchanged())
		{
			reason = "revoked_or_failed";
			return false;
		}
		if (!Controller.CF_CheckOriginalFollowLease(this, reason))
			return false;
		if (!utility || utility != Utility || utility.GetOwner() != Group)
		{
			reason = "utility_identity";
			return false;
		}
		if (Group.GetCurrentWaypoint() != Waypoint || Waypoint.GetEntity() != NativeTarget)
		{
			reason = "current_waypoint_or_target";
			return false;
		}
		// Require native boarding/registration already complete. Never register,
		// reassign, select a substitute vehicle or emit a seat order here.
		SCR_AIGroupVehicle vehicle;
		if (utility.m_VehicleMgr)
			vehicle = utility.m_VehicleMgr.FindVehicle(Truck);
		if (!vehicle || vehicle.GetEntity() != Truck || vehicle.GetDriver() != Driver || !vehicle.CanMove())
		{
			reason = "registered_original_vehicle_unavailable";
			return false;
		}
		SCR_AIVehicleUsageComponent usage = vehicle.GetVehicleUsageComponent();
		AIGroupMovementComponent movement = AIGroupMovementComponent.Cast(Group.GetMovementComponent());
		if (!usage || !usage.CanBePiloted() || !movement || movement.GetAIAgent() != Group)
		{
			reason = "native_vehicle_movement_unavailable";
			return false;
		}
		int handler = movement.GetAgentMoveHandlerId(Agent);
		if (handler < 0 || handler == AIGroupMovementComponent.DEFAULT_HANDLER_ID || handler != vehicle.GetSubgroupHandleId())
		{
			reason = "original_vehicle_handler_not_ready";
			return false;
		}
		array<AIAgent> handledAgents = {};
		movement.GetAgentsInHandler(handledAgents, handler);
		if (handledAgents.Count() != 1 || handledAgents[0] != Agent)
		{
			reason = "handler_membership";
			return false;
		}
		array<ref AIActionBase> actions = {};
		utility.GetActions(actions);
		foreach (AIActionBase action : actions)
		{
			if (!action || action.GetActionState() == EAIActionState.COMPLETED || action.GetActionState() == EAIActionState.FAILED)
				continue;
			if (CF_EntityFollowActivity.Cast(action) && action != Activity)
			{
				reason = "competing_private_or_seat_activity";
				return false;
			}
		}
		if (HasSeatWork())
		{
			reason = "active_or_unobserved_seat_work";
			return false;
		}
		reason = "admitted";
		return true;
	}

	bool Executing(AIAgent owner, out string reason)
	{
		if (owner != Group || !Admit(Utility, reason))
			return false;
		if (!Activity || Activity.GetActionState() == EAIActionState.COMPLETED || Activity.GetActionState() == EAIActionState.FAILED)
		{
			reason = "missing_or_terminal_activity";
			return false;
		}
		if (Utility.GetExecutedAction() != Activity || Utility.GetCurrentAction() != Activity || Waypoint.CF_GetActivity() != Activity)
		{
			reason = "activity_not_exact_execution";
			return false;
		}
		if (Activity.m_RelatedWaypoint != Waypoint || Activity.m_Entity.m_Value != NativeTarget ||
			Activity.m_fCompletionDistance.m_Value != Distance || Activity.EvaluatePriorityLevel() != PriorityLevel)
		{
			reason = "activity_binding_changed";
			return false;
		}
		return true;
	}

	void Block(string reason)
	{
		if (Failed)
			return;
		// Controller handles its own waypoint on a later frame. The node never
		// fails utility.GetExecutedAction blindly or sends a seat/control order.
		Failed = true;
		Revoked = true;
		Record(reason);
		if (Controller)
			Controller.CF_BlockOriginalFollow(this, reason);
	}

	void RecordArrivalRecoveryFailure(int result, int handler)
	{
		Failed = true;
		Revoked = true;
		ArrivalRecoveryOwned = true;
		Record("native_request_failed_recovery_pending_move_" + result + "_handler_" + handler);
	}

	void ActivationStarted()
	{
		m_Activation++;
		m_RequestOpen = false;
		Record("activation_started");
	}

	void RequestStarted()
	{
		m_Requests++;
		m_RequestOpen = true;
		Record("request_started_" + m_Requests);
	}

	void RequestCompleted()
	{
		m_CompletedRequests++;
		m_RequestOpen = false;
		Record("request_completed_" + m_Requests);
	}
}

class CF_OriginalFollowIdentity
{
	protected CF_EntityFollowDriverControllerComponent m_Controller;
	protected ChimeraCharacter m_Driver;
	protected Vehicle m_Truck;
	protected IEntity m_Predecessor;
	protected IEntity m_NativeTarget;
	protected SCR_AIGroup m_Group;
	protected AIAgent m_Agent;
	protected SCR_AIGroupUtilityComponent m_Utility;
	protected CF_ConvoySession m_Session;
	protected CF_EntityFollowWaypoint m_Waypoint;
	protected World m_World;
	protected int m_Generation;
	protected float m_Distance;
	protected float m_Priority;

	void CF_OriginalFollowIdentity(CF_OriginalFollowLease lease)
	{
		m_Controller = lease.Controller;
		m_Driver = lease.Driver;
		m_Truck = lease.Truck;
		m_Predecessor = lease.Predecessor;
		m_NativeTarget = lease.NativeTarget;
		m_Group = lease.Group;
		m_Agent = lease.Agent;
		m_Utility = lease.Utility;
		m_Session = lease.Session;
		m_Waypoint = lease.Waypoint;
		m_World = lease.OriginalWorld;
		m_Generation = lease.Generation;
		m_Distance = lease.Distance;
		m_Priority = lease.PriorityLevel;
	}

	bool Matches(CF_OriginalFollowLease lease)
	{
		if (!lease || lease.Controller != m_Controller || lease.Driver != m_Driver || lease.Truck != m_Truck)
			return false;
		if (lease.Predecessor != m_Predecessor || lease.NativeTarget != m_NativeTarget || lease.Group != m_Group)
			return false;
		if (lease.Agent != m_Agent || lease.Utility != m_Utility || lease.Session != m_Session)
			return false;
		if (lease.Waypoint != m_Waypoint || lease.OriginalWorld != m_World || lease.Generation != m_Generation)
			return false;
		return lease.Distance == m_Distance && lease.PriorityLevel == m_Priority;
	}
}

class CF_OriginalFollowActivity : CF_EntityFollowActivity
{
	ref CF_OriginalFollowLease Lease;
	void CF_OriginalFollowActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, vector pos, IEntity ent,
		EMovementType movementType = EMovementType.RUN, bool useVehicles = true,
		float priority = PRIORITY_ACTIVITY_FOLLOW, float priorityLevel = PRIORITY_LEVEL_NORMAL, float distance = 1.0)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/ConvoyTests/OriginalOrchestration/ActivityOwnedVehicleFollow.bt";
	}
}

// Exact action identity is obtained through the installed public utility API.
class CF_OriginalFollowGuard : DecoratorScripted
{
	protected override bool TestFunction(AIAgent owner)
	{
		SCR_AIGroupUtilityComponent utility = SCR_AIGroupUtilityComponent.Cast(owner.FindComponent(SCR_AIGroupUtilityComponent));
		if (!utility)
			return false;
		CF_OriginalFollowActivity activity = CF_OriginalFollowActivity.Cast(utility.GetExecutedAction());
		if (!activity || !activity.Lease)
			return false;
		string reason;
		bool allowed = activity.Lease.Executing(owner, reason);
		if (!allowed)
			activity.Lease.Record(reason);
		else
			activity.Lease.Heartbeat();
		return allowed;
	}
}

class CF_OriginalFollowTask : SCR_AIActionTask
{
	protected CF_OriginalFollowActivity ExactActivity(AIAgent owner, out string reason)
	{
		CF_OriginalFollowActivity activity = CF_OriginalFollowActivity.Cast(GetExecutedAction());
		if (!activity || !activity.Lease || !activity.Lease.Executing(owner, reason))
			return null;
		return activity;
	}
}

// One exact receiver; no foreach expansion, occupancy planning or GetIn/GetOut.
class CF_OriginalFollowBegin : CF_OriginalFollowTask
{
	protected static ref TStringArray s_Variables = {"Entity", "DesiredDistance", "Receiver", "PriorityLevel", "WaypointRelated"};
	override TStringArray GetVariablesOut() { return s_Variables; }
	protected static override bool VisibleInPalette() { return true; }
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		string reason;
		CF_OriginalFollowActivity activity = ExactActivity(owner, reason);
		if (!activity)
			return ENodeResult.FAIL;
		CF_OriginalFollowLease lease = activity.Lease;
		SetVariableOut("Entity", lease.NativeTarget);
		SetVariableOut("DesiredDistance", lease.Distance);
		SetVariableOut("Receiver", lease.Agent);
		SetVariableOut("PriorityLevel", lease.PriorityLevel);
		SetVariableOut("WaypointRelated", true);
		lease.ActivationStarted();
		return ENodeResult.SUCCESS;
	}
}

class CF_OriginalFollowRequest : CF_OriginalFollowTask
{
	protected static override bool VisibleInPalette() { return true; }
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		string reason;
		CF_OriginalFollowActivity activity = ExactActivity(owner, reason);
		if (!activity)
			return ENodeResult.FAIL;
		activity.Lease.RequestStarted();
		return ENodeResult.SUCCESS;
	}
}

class CF_OriginalFollowRequestDone : CF_OriginalFollowTask
{
	protected static override bool VisibleInPalette() { return true; }
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		string reason;
		CF_OriginalFollowActivity activity = ExactActivity(owner, reason);
		if (!activity)
			return ENodeResult.FAIL;
		activity.Lease.RequestCompleted();
		return ENodeResult.SUCCESS;
	}
}

class CF_OriginalFollowRetire : CF_OriginalFollowTask
{
	protected static ref TStringArray s_Variables = {"MoveResult", "FailedHandlerId"};
	override TStringArray GetVariablesIn() { return s_Variables; }
	protected static override bool VisibleInPalette() { return true; }
	static override protected bool CanReturnRunning() { return true; }
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		CF_OriginalFollowActivity activity = CF_OriginalFollowActivity.Cast(GetExecutedAction());
		if (!activity || !activity.Lease || activity.Lease.Activity != activity || activity.Lease.Group != owner)
			return ENodeResult.RUNNING;
		// No native action cancellation here, including on late possession.
		// A stale/mismatched node cannot mutate some other selected activity.
		if (!m_UtilityComp || m_UtilityComp.GetCurrentAction() != activity)
			return ENodeResult.RUNNING;
		// Revocation precedes controller-owned removal. A final tree callback
		// can still execute during that native handover; it is not a request
		// failure and must not poison a safely deferred, still-owned slot.
		if (activity.Lease.Revoked && (!activity.Lease.Failed || activity.Lease.ArrivalRecoveryOwned))
		{
			activity.Lease.Record("revoked_callback_no_request_failure");
			return ENodeResult.RUNNING;
		}
		int result = -1;
		int handler = -1;
		bool hasResult = GetVariableIn("MoveResult", result);
		bool hasHandler = GetVariableIn("FailedHandlerId", handler);
		string reason;
		bool exactExecution = activity.Lease.Executing(owner, reason);
		// The outer guard-failure node has disconnected result ports. It must
		// never enter request recovery, even if a previous result was 3.
		if (hasResult && hasHandler && exactExecution &&
			activity.Lease.Controller.CF_TryRecoverOriginalMoveFailure(activity.Lease, result, handler))
			return ENodeResult.RUNNING;
		activity.Lease.Block("guard_or_request_failure_" + reason + "_move_" + result + "_handler_" + handler);
		return ENodeResult.RUNNING;
	}
}
