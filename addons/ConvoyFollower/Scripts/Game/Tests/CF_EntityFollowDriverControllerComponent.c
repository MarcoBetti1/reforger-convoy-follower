// Private arrival/explicit-Hold comparison. This unparented native Wait owns
// no group activity, seat command or vehicle control. Retirement completes
// only this action; it never cancels another action or changes pilot inputs.
class CF_EntityCapturedWait : SCR_AIWaitBehavior
{
	protected CF_EntityFollowDriverControllerComponent m_CaptureController;
	protected CF_EntityFollowDriverControllerComponent m_CaptureControllerIdentity;
	protected ChimeraCharacter m_CaptureDriver;
	protected Vehicle m_CaptureTruck;
	protected SCR_AIGroup m_CaptureGroup;
	protected World m_CaptureWorld;
	protected CF_ConvoySession m_CaptureSession;
	protected IEntity m_CaptureOwner;
	protected int m_iCaptureOwnerId;
	protected bool m_bCapturePanelHold;
	protected bool m_bCaptureExplicitBay;
	protected vector m_vCaptureBay;
	protected vector m_vCaptureBayTruckAnchor;
	protected bool m_bCaptureBound;
	protected bool m_bCaptureLease;
	protected bool m_bCapturePilotDeparture;

	void CF_EntityCapturedWait(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity)
	{
	}

	void Bind(CF_EntityFollowDriverControllerComponent controller, ChimeraCharacter driver, Vehicle truck, SCR_AIGroup group,
		CF_ConvoySession session, IEntity owner, int ownerId, bool panelHold)
	{
		if (m_bCaptureBound) return;
		m_bCaptureBound = true;
		m_CaptureController = controller;
		m_CaptureControllerIdentity = controller;
		m_CaptureDriver = driver;
		m_CaptureTruck = truck;
		m_CaptureGroup = group;
		m_CaptureWorld = GetGame().GetWorld();
		m_CaptureSession = session;
		m_CaptureOwner = owner;
		m_iCaptureOwnerId = ownerId;
		m_bCapturePanelHold = panelHold;
		m_bCaptureLease = true;
		SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_GAMEMASTER + 100);
	}

	protected bool m_bCaptureParked;
	bool IsParked() { return m_bCaptureParked; }
	void BindParked() { if (m_bCaptureBound) m_bCaptureParked = true; }
	bool IsPanelHold() { return m_bCapturePanelHold; }
	bool IsExplicitBay() { return m_bCaptureExplicitBay; }
	void BindExplicitBay(vector bay, vector truckAnchor)
	{
		if (!m_bCaptureBound || m_bCapturePanelHold || m_bCaptureExplicitBay) return;
		m_bCaptureExplicitBay = true;
		m_vCaptureBay = bay;
		m_vCaptureBayTruckAnchor = truckAnchor;
	}
	bool HasBayAnchors(vector bay, vector truckPosition)
	{
		return m_bCaptureExplicitBay && vector.Distance(m_vCaptureBay, bay) < 0.01 &&
			vector.Distance(m_vCaptureBayTruckAnchor, truckPosition) <= 0.25;
	}
	bool WasPilotDeparture() { return m_bCapturePilotDeparture; }

	bool HasOriginalBinding(CF_EntityFollowDriverControllerComponent controller, ChimeraCharacter driver, Vehicle truck,
		SCR_AIGroup group, CF_ConvoySession session, IEntity owner, int ownerId)
	{
		return m_bCaptureLease && HasImmutableBinding(controller, driver, truck, group, session, owner, ownerId);
	}

	// Retirement evidence only. This never authorizes a Wait or movement.
	// CustomEvaluate may already have revoked the lease after the same exit.
	bool HasImmutableBinding(CF_EntityFollowDriverControllerComponent controller, ChimeraCharacter driver, Vehicle truck,
		SCR_AIGroup group, CF_ConvoySession session, IEntity owner, int ownerId)
	{
		return m_bCaptureBound && GetGame() && m_CaptureControllerIdentity == controller && m_CaptureDriver == driver &&
			m_CaptureTruck == truck && m_CaptureGroup == group && m_CaptureSession == session &&
			m_CaptureOwner == owner && m_iCaptureOwnerId == ownerId && m_CaptureWorld == GetGame().GetWorld();
	}

	bool HasOriginalPilot()
	{
		if (!GetGame() || GetGame().GetWorld() != m_CaptureWorld || !m_CaptureWorld ||
			!m_CaptureDriver || !m_CaptureTruck || !m_CaptureGroup || !m_Utility)
			return false;
		PlayerManager players = GetGame().GetPlayerManager();
		if (!players || players.GetPlayerIdFromControlledEntity(m_CaptureDriver) > 0 ||
			players.GetPlayerIdFromControlledEntity(m_CaptureTruck) > 0)
			return false;
		CompartmentAccessComponent access = m_CaptureDriver.GetCompartmentAccessComponent();
		BaseCompartmentSlot slot;
		if (access)
			slot = access.GetCompartment();
		CarControllerComponent car = CarControllerComponent.Cast(m_CaptureTruck.FindComponent(CarControllerComponent));
		AIAgent agent = m_Utility.GetOwner();
		return slot && slot.IsPiloting() && slot.GetOccupant() == m_CaptureDriver &&
			car && car.GetPilotCompartmentSlot() == slot && access.GetVehicleIn(m_CaptureDriver) == m_CaptureTruck &&
			!access.IsGettingIn() && !access.IsGettingOut() && agent &&
			agent.GetControlledEntity() == m_CaptureDriver && agent.GetParentGroup() == m_CaptureGroup &&
			m_CaptureGroup.GetAgentsCount() == 1;
	}

	void Retire(bool allowNativeCompletion)
	{
		m_bCaptureLease = false;
		m_CaptureController = null;
		if (allowNativeCompletion && !CF_ConvoySession.CF_IsWorldCleanup() && HasOriginalPilot() &&
			GetActionState() != EAIActionState.COMPLETED && GetActionState() != EAIActionState.FAILED)
			Complete();
	}

	override float CustomEvaluate()
	{
		if (!GetGame() || GetGame().GetWorld() != m_CaptureWorld || CF_ConvoySession.CF_IsWorldCleanup())
			return 0;
		if (GetActionState() == EAIActionState.COMPLETED || GetActionState() == EAIActionState.FAILED)
			return 0;
		if (m_bCaptureLease && m_CaptureController &&
			m_CaptureController.CF_CanRetirePanelWaitForPilotDeparture(this))
		{
			m_bCapturePilotDeparture = true;
			Retire(false);
			Complete(); // Unparented Wait only; never cancel the native seat action.
			return 0;
		}
		if (!m_bCaptureLease || !HasOriginalPilot() || !m_CaptureController ||
			!m_CaptureController.CF_HasCapturedWaitLease(this, m_CaptureDriver, m_CaptureTruck, m_CaptureGroup))
		{
			m_bCaptureLease = false;
			// This Wait has no related group activity, so completion cannot
			// fail a boarding child or issue a late GetOut after possession.
			Complete();
			return 0;
		}
		return GetPriority();
	}
}

class CF_EntityFollowDriverControllerComponentClass : CF_DriverControllerComponentClass
{
}

// Gameplay base inherited by CF_ConvoyFollowDriverControllerComponent through
// CF_TrailGuideDriverControllerComponent. Ordinary driver prefabs explicitly
// enable entity follow, original graph and captured Wait. Attribute defaults
// remain off for standalone components; the Tests path is historical.
class CF_EntityFollowDriverControllerComponent : CF_DriverControllerComponent
{
	[Attribute(defvalue: "0", desc: "Private persistent entity-follow comparison; never a production default")]
	protected bool m_bEntityFollowPrototype;
	[Attribute(defvalue: "0", desc: "Private comparison: owned native Wait only after measured stopped-entity arrival capture")]
	protected bool m_bEntityCapturedWait;
	[Attribute(defvalue: "0", desc: "Private comparison: reduce catch-up allowance for an active stretched immediate successor")]
	protected bool m_bEntityRearPacing;
	[Attribute(defvalue: "0", desc: "Private original graph comparison; requires settled native vehicle registration")]
	protected bool m_bOriginalFollowGraph;
	protected ref CF_OriginalFollowLease m_OriginalFollowLease;
	// Weak, synchronous scope for one ARRIVING -> FOLLOWING lifecycle call.
	protected CF_OriginalFollowLease m_OriginalArrivalResumeLease;
	protected int m_iOriginalFollowGeneration;
	protected bool m_bOriginalFollowBlocked;
	protected bool m_bOriginalFollowBlockApplied;
	protected bool m_bOriginalFollowResetPending;
	protected float m_fOriginalRetireStartMs = -1;
	protected bool m_bOriginalRetireTimedOut;
	protected string m_sOriginalFollowFailure;
	protected bool m_bOriginalParkedLeadAdmitted;
	protected int m_iOriginalParkedLeadLogs;
	protected bool m_bOriginalParkedLeadRejectLogged;

	bool CF_OriginalPilotSafe(CF_OriginalFollowLease lease)
	{
		if (!lease || !lease.BindingUnchanged() || !GetGame() || !lease.OriginalWorld || GetGame().GetWorld() != lease.OriginalWorld)
			return false;
		if (!Replication.IsServer() || CF_ConvoySession.CF_IsWorldCleanup() || CF_IsControlBlocked())
			return false;
		if (lease.Controller != this || lease.Driver != m_Driver || lease.Truck != m_Truck || lease.Group != m_Group)
			return false;
		if (!lease.Driver || GetOwner() != lease.Driver || lease.Driver.GetWorld() != lease.OriginalWorld || !lease.Truck)
			return false;
		if (lease.Truck.GetWorld() != lease.OriginalWorld || !m_Group || m_Group.GetAgentsCount() != 1)
			return false;
		if (m_Group.GetWorld() != lease.OriginalWorld)
			return false;
		if (!CF_IsLiveAIPilot(lease.Driver, lease.Truck) || IsDriverDestroyed(lease.Driver) || IsAssignedTruckDestroyed())
			return false;
		CharacterControllerComponent character = lease.Driver.GetCharacterController();
		CompartmentAccessComponent access = lease.Driver.GetCompartmentAccessComponent();
		if (!character || character.IsUnconscious() || !access || access.IsGettingIn() || access.IsGettingOut())
			return false;
		AIControlComponent control = lease.Driver.GetAIControlComponent();
		return control && lease.Agent && control.GetAIAgent() == lease.Agent &&
			lease.Agent.GetControlledEntity() == lease.Driver && lease.Agent.GetParentGroup() == m_Group;
	}

	// An empty parked owner vehicle is still a valid arrival reference. This
	// exception changes only first-link target admission, never pilot ownership.
	protected bool CF_CanAdmitParkedPlayerLead(CF_OriginalFollowLease lease, CarControllerComponent car, BaseCompartmentSlot slot, out string rejectReason)
	{
		// Preserve the existing predicate order; report only the first rejection.
		rejectReason = "";
		if (!lease) { rejectReason = "lease_missing"; return false; }
		if (m_Predecessor) { rejectReason = "not_first_link"; return false; }
		if (!m_Session) { rejectReason = "session_missing"; return false; }
		if (!m_Session.IsCurrentLeader(this)) { rejectReason = "not_current_leader"; return false; }
		if (lease.Session != m_Session) { rejectReason = "lease_session_mismatch"; return false; }
		if (lease.NativeTarget != lease.Predecessor) { rejectReason = "native_target_not_predecessor"; return false; }
		if (lease.Predecessor != CF_GetCachedPlayerVehicle()) { rejectReason = "cached_player_vehicle_mismatch"; return false; }
		Vehicle lead = Vehicle.Cast(lease.Predecessor);
		if (!lead) { rejectReason = "predecessor_not_vehicle"; return false; }
		if (!car) { rejectReason = "predecessor_car_missing"; return false; }
		if (!slot) { rejectReason = "pilot_slot_missing"; return false; }
		if (car.GetPilotCompartmentSlot() != slot) { rejectReason = "pilot_slot_changed"; return false; }
		if (slot.GetOccupant()) { rejectReason = "pilot_slot_not_empty"; return false; }
		if (!car.GetSimulation()) { rejectReason = "predecessor_simulation_missing"; return false; }
		if (CF_ConvoySession.IsVehicleAssignedToAnotherDriver(lead, this)) { rejectReason = "predecessor_reserved_elsewhere"; return false; }
		ChimeraCharacter owner = ChimeraCharacter.Cast(m_Leader);
		if (!owner) { rejectReason = "owner_missing"; return false; }
		if (owner.GetWorld() != lease.OriginalWorld) { rejectReason = "owner_world_mismatch"; return false; }
		if (IsDriverDestroyed(owner)) { rejectReason = "owner_destroyed"; return false; }
		PlayerManager players = GetGame().GetPlayerManager();
		if (!players) { rejectReason = "player_manager_missing"; return false; }
		if (m_iOrderingPlayerId <= 0) { rejectReason = "ordering_player_id_invalid"; return false; }
		if (players.GetPlayerControlledEntity(m_iOrderingPlayerId) != owner) { rejectReason = "controlled_owner_mismatch"; return false; }
		if (CF_ConvoySession.GetForPlayer(owner) != m_Session) { rejectReason = "owner_session_mismatch"; return false; }
		float speed = Math.AbsFloat(car.GetSimulation().GetSpeedKmh());
		// Positive bounded comparison rejects NaN/infinity as well as motion.
		if (!(speed >= 0 && speed <= 0.5)) { rejectReason = "speed_invalid_or_above_0_5"; return false; }
		return true;
	}

	protected void CF_RecordParkedLeadAdmission(bool admitted)
	{
		if (admitted == m_bOriginalParkedLeadAdmitted)
			return;
		m_bOriginalParkedLeadAdmitted = admitted;
		if (m_iOriginalParkedLeadLogs >= 32)
			return;
		m_iOriginalParkedLeadLogs++;
		string line = "[ConvoyFollower] ORIGINAL_FOLLOW_PARKED_LEAD: unit=" + m_iUnitNumber;
		line += " generation=" + m_iOriginalFollowGeneration + " admitted=" + admitted;
		if (m_Driver) line += " driver_id=" + m_Driver.GetID();
		if (m_LeadVehicle) line += " predecessor_id=" + m_LeadVehicle.GetID();
		line += " first_link_only=true max_speed_kmh=0.5 control_writes=false";
		Print(line);
		if (m_iOriginalParkedLeadLogs == 32)
			Print("[ConvoyFollower] ORIGINAL_FOLLOW_PARKED_LEAD_LIMIT: limit=32 admission_checks_continue=true");
	}

	bool CF_CheckOriginalFollowLease(CF_OriginalFollowLease lease, out string reason)
	{
		if (!m_bOriginalFollowGraph || lease != m_OriginalFollowLease || !lease || lease.Generation != m_iOriginalFollowGeneration)
		{
			reason = "controller_generation";
			return false;
		}
		if (!CF_OriginalPilotSafe(lease))
		{
			reason = "original_pilot_ownership";
			return false;
		}
		if (lease.Session != m_Session || !m_Session || m_Session.GetUnitNumber(this) <= 0 ||
			CF_ConvoySession.IsVehicleAssignedToAnotherDriver(m_Truck, this))
		{
			reason = "session_membership_or_reservation";
			return false;
		}
		if (!CF_IsOrdinaryEntityContext() || m_bOriginalFollowBlocked || m_bArrivalRoadHold || m_bArrivalRoadRecoveryBlocked)
		{
			reason = "ordinary_order_context";
			return false;
		}
		if (!lease.Waypoint || m_Waypoint != lease.Waypoint || !HasOwnWaypointInGroup())
		{
			reason = "owned_waypoint";
			return false;
		}
		if (!lease.Predecessor || lease.Predecessor == m_Truck || lease.Predecessor != m_LeadVehicle ||
			GetTargetVehicle(false) != lease.Predecessor || lease.Predecessor.GetWorld() != lease.OriginalWorld)
		{
			reason = "actual_predecessor_identity";
			return false;
		}
		SCR_DamageManagerComponent damage = SCR_DamageManagerComponent.GetDamageManager(lease.Predecessor);
		if (damage && damage.IsDestroyed())
		{
			reason = "predecessor_destroyed";
			return false;
		}
		CarControllerComponent targetCar = CarControllerComponent.Cast(lease.Predecessor.FindComponent(CarControllerComponent));
		BaseCompartmentSlot targetSlot;
		if (targetCar) targetSlot = targetCar.GetPilotCompartmentSlot();
		IEntity targetPilot;
		if (targetSlot) targetPilot = targetSlot.GetOccupant();
		bool parkedPlayerLead;
		if (!targetPilot)
		{
			string parkedRejectReason;
			parkedPlayerLead = CF_CanAdmitParkedPlayerLead(lease, targetCar, targetSlot, parkedRejectReason);
			if (!parkedPlayerLead)
			{
				if (!m_bOriginalParkedLeadRejectLogged)
				{
					m_bOriginalParkedLeadRejectLogged = true;
					float rejectedLeadSpeed = -1;
					if (targetCar && targetCar.GetSimulation()) rejectedLeadSpeed = Math.AbsFloat(targetCar.GetSimulation().GetSpeedKmh());
					Print("[ConvoyFollower] ORIGINAL_FOLLOW_PARKED_LEAD_REJECT: unit=" + m_iUnitNumber +
						" generation=" + m_iOriginalFollowGeneration + " reason=" + parkedRejectReason +
						" speed_kmh=" + rejectedLeadSpeed + " max_speed_kmh=0.5 entry_pilot_empty=true once_per_controller=true control_writes=false");
				}
				reason = "empty_predecessor_not_parked_session_lead";
				return false;
			}
		}
		else if (targetPilot == lease.Driver)
		{
			reason = "external_predecessor_pilot";
			return false;
		}
		AIControlComponent targetControl;
		if (targetPilot) targetControl = AIControlComponent.Cast(targetPilot.FindComponent(AIControlComponent));
		AIAgent targetAgent;
		if (targetControl) targetAgent = targetControl.GetAIAgent();
		if (targetAgent && targetAgent.GetParentGroup() == m_Group)
		{
			reason = "predecessor_in_follower_group";
			return false;
		}
		if (!lease.NativeTarget || lease.NativeTarget.GetWorld() != lease.OriginalWorld)
		{
			reason = "native_target_world";
			return false;
		}
		if (lease.NativeTarget != lease.Predecessor)
		{
			CF_TrailGuideEntity guide = CF_TrailGuideEntity.Cast(lease.NativeTarget);
			if (!guide || !guide.CF_HasLease(lease.Waypoint))
			{
				reason = "native_guide_lease";
				return false;
			}
		}
		if (!(lease.Distance > 0 && lease.Distance < 1000) || lease.Waypoint.GetCompletionRadius() != lease.Distance ||
			lease.Waypoint.GetPriorityLevel() != lease.PriorityLevel)
		{
			reason = "distance_or_priority_binding";
			return false;
		}
		CF_RecordParkedLeadAdmission(parkedPlayerLead);
		return true;
	}

	protected void CF_BindOriginalFollow(CF_EntityFollowWaypoint waypoint)
	{
		if (!m_bOriginalFollowGraph)
			return;
		CF_OriginalFollowLease lease = new CF_OriginalFollowLease();
		lease.Controller = this;
		lease.Driver = ChimeraCharacter.Cast(m_Driver);
		lease.Truck = m_Truck;
		lease.Predecessor = m_LeadVehicle;
		lease.NativeTarget = waypoint.GetEntity();
		lease.Group = m_Group;
		lease.Session = m_Session;
		lease.Waypoint = waypoint;
		lease.OriginalWorld = GetGame().GetWorld();
		m_iOriginalFollowGeneration++;
		lease.Generation = m_iOriginalFollowGeneration;
		lease.Distance = waypoint.GetCompletionRadius();
		lease.PriorityLevel = waypoint.GetPriorityLevel();
		AIControlComponent control;
		if (lease.Driver) control = lease.Driver.GetAIControlComponent();
		if (control) lease.Agent = control.GetAIAgent();
		if (m_Group) lease.Utility = SCR_AIGroupUtilityComponent.Cast(m_Group.FindComponent(SCR_AIGroupUtilityComponent));
		lease.Seal();
		m_OriginalFollowLease = lease;
		waypoint.CF_BindOriginalLease(lease);
		lease.Record("bound_before_add_waypoint");
	}

	// Default remains terminal. Ordinary tails may admit one narrowly typed
	// recovery while the native request still has exact executing ownership.
	bool CF_TryRecoverOriginalMoveFailure(CF_OriginalFollowLease lease, int result, int handler)
	{
		return false;
	}

	void CF_BlockOriginalFollow(CF_OriginalFollowLease lease, string reason)
	{
		if (!lease || lease != m_OriginalFollowLease || lease.Generation != m_iOriginalFollowGeneration)
			return;
		CF_RecordParkedLeadAdmission(false);
		m_bOriginalFollowBlocked = true;
		m_bEntityFallbackFailed = true;
		m_sOriginalFollowFailure = reason;
	}

	protected void CF_RevokeOriginalFollow(string reason)
	{
		CF_RecordParkedLeadAdmission(false);
		if (m_OriginalFollowLease)
			m_OriginalFollowLease.Revoke(reason);
	}

	protected bool CF_OriginalCanRetire(CF_OriginalFollowLease lease)
	{
		if (!lease || lease != m_OriginalFollowLease || m_Waypoint != lease.Waypoint || !CF_OriginalPilotSafe(lease))
			return false;
		// Conservative retirement after identity loss never enters a native
		// cancellation path. In particular, don't fail any live seat behavior.
		return !lease.HasSeatWork();
	}

	override protected void ClearWaypoints()
	{
		CF_OriginalFollowLease lease = m_OriginalFollowLease;
		if (lease && m_Waypoint == lease.Waypoint)
		{
			CF_RecordParkedLeadAdmission(false);
			lease.Revoke("controller_clear");
			if (!CF_OriginalCanRetire(lease))
			{
				m_bDeferredWaypointClear = true;
				if (m_fOriginalRetireStartMs < 0 && GetGame() && GetGame().GetWorld())
					m_fOriginalRetireStartMs = GetGame().GetWorld().GetWorldTime();
				lease.Record("native_retirement_deferred_ownership_or_seat_work");
				return;
			}
		}
		super.ClearWaypoints();
		if (!m_Waypoint)
		{
			m_OriginalFollowLease = null;
			m_fOriginalRetireStartMs = -1;
			m_bOriginalRetireTimedOut = false;
		}
	}

	// Every base issuer can replace m_Waypoint. Protect that slot before
	// delegating, including explicit dismissal and boarding recovery paths.
	protected bool CF_OriginalSlotReady(string reason)
	{
		if (!m_OriginalFollowLease || m_Waypoint != m_OriginalFollowLease.Waypoint)
			return true;
		ClearWaypoints();
		if (!m_Waypoint)
			return true;
		m_OriginalFollowLease.Block("replacement_deferred_" + reason);
		return false;
	}

	override protected bool IssueBoardWaypoint()
	{
		if (!CF_OriginalSlotReady("boarding")) return false;
		return super.IssueBoardWaypoint();
	}

	override protected bool IssuePassengerBoardWaypoint(IEntity vehicle)
	{
		if (!CF_OriginalSlotReady("passenger_boarding")) return false;
		return super.IssuePassengerBoardWaypoint(vehicle);
	}

	override protected bool IssueOnFootFollowWaypoint()
	{
		if (!CF_OriginalSlotReady("on_foot_follow")) return false;
		return super.IssueOnFootFollowWaypoint();
	}

	override protected bool IssueGetOutWaypointFor(IEntity vehicle)
	{
		if (!CF_OriginalSlotReady("explicit_get_out")) return false;
		return super.IssueGetOutWaypointFor(vehicle);
	}

	protected void CF_ObserveOriginalFollow()
	{
		CF_OriginalFollowLease lease = m_OriginalFollowLease;
		if (!lease)
			return;
		// A normal deferred Clear revokes without necessarily failing the
		// experiment. Retry its exact slot once safe, even outside suspension.
		if (lease.Revoked && m_Waypoint == lease.Waypoint && !m_bOriginalRetireTimedOut)
		{
			if (CF_OriginalCanRetire(lease))
			{
				ClearWaypoints();
				if (!m_Waypoint && m_bOriginalFollowResetPending)
				{
					m_bOriginalFollowResetPending = false;
					ResetToIdle();
					return;
				}
			}
			else if (m_fOriginalRetireStartMs >= 0 && GetGame() && GetGame().GetWorld() == lease.OriginalWorld)
			{
				if (lease.OriginalWorld.GetWorldTime() - m_fOriginalRetireStartMs >= 60000)
				{
					m_bOriginalRetireTimedOut = true;
					lease.Block("retirement_timeout_60s_native_order_untouched");
					lease.Record("automatic_retirement_retry_stopped_explicit_cleanup_required");
				}
			}
		}
		if (m_bOriginalRetireTimedOut)
			return;
		if (!lease.Revoked && !lease.Failed)
		{
			string reason;
			if (!CF_CheckOriginalFollowLease(lease, reason))
				lease.Block(reason);
		}
		if (!m_bOriginalFollowBlocked || m_bOriginalFollowBlockApplied)
			return;
		if (m_Waypoint && !CF_OriginalCanRetire(lease))
			return;
		if (!CF_IsOrdinaryEntityContext())
			return;
		m_bOriginalFollowBlockApplied = true;
		CF_FailEntityApproach("original_graph_" + m_sOriginalFollowFailure);
	}

	protected CF_EntityFollowDriverControllerComponent m_RearPacingSuccessor;
	protected float m_fRearPacingCap = -1;
	protected float m_fRearPacingLastMs = -1;
	protected float m_fRearPacingNextLogMs;
	protected int m_iRearPacingLogs;
	protected string m_sRearPacingReason;
	protected ref CF_EntityCapturedWait m_EntityCapturedWait;
	protected IEntity m_EntityWaitTarget;
	protected vector m_vEntityWaitTargetAnchor;
	protected float m_fEntityWaitNextLogMs;
	protected int m_iEntityWaitLogs;
	protected bool m_bEntityWaitWasSelected;
	protected bool m_bEntityPanelWaitFailed;
	protected float m_fExplicitBayStableStartMs = -1;
	protected vector m_vExplicitBayStableTruck;
	protected vector m_vExplicitBayStableBay;
	protected AIWaypoint m_ExplicitBayMove;
	protected int m_iEntityRetainLogs;
	protected float m_fNextEntityRetainLogMs;
	protected bool m_bEntityFallbackFailed;
	// Historical evidence remains sticky after an explicit owned-order retry.
	protected bool m_bEntityFallbackFailedEver;
	protected IEntity m_EntityStopTarget;
	protected bool m_bEntityStopAttempted;
	protected int m_iEntityStopEpisode;
	protected int m_iEntityStopReplacements;
	protected bool m_bEntityStablePoseValid;
	protected vector m_vEntityStablePose;
	protected float m_fEntityStableSeconds;
	protected static const float CF_ENTITY_STOP_SPEED_KMH = 0.5;
	protected static const float CF_ENTITY_STOP_STABLE_SECONDS = 3.0;
	protected static const float CF_ENTITY_STOP_POSE_METERS = 0.25;
	protected static const ResourceName CF_ENTITY_WAYPOINT = "{CA8E704723F09B11}Prefabs/Tests/AIWaypoint_CF_EntityFollow.et";

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (Replication.IsServer())
		{
			Print("[ConvoyFollower] ORIGINAL_FOLLOW_INIT: driver_id=" + owner.GetID() +
				" enabled=" + m_bOriginalFollowGraph + " graph_guid=976835B5116F4BC5" +
				" native_planning=false native_failure_policy=blocked_no_seat_order test_only=true");
			Print("[ConvoyFollower] ENTITY_CAPTURE_WAIT_INIT: driver_id=" + owner.GetID() +
				" prototype=" + m_bEntityFollowPrototype + " enabled=" + m_bEntityCapturedWait +
				" priority_level=2100 arrival_requires_measured_capture=true panel_acquire_max_kmh=2 test_only=true");
			Print("[ConvoyFollower] ENTITY_REAR_PACING_INIT: driver_id=" + owner.GetID() +
				" enabled=" + m_bEntityRearPacing + " gap_band_multipliers=1.5,2 reaction_horizon_s=1.5" +
				" head_only=true braking_observation=true cap_down=prompt cap_up_kmh_per_s=3 sample_dt_max_s=0.5 cruise_writer=existing_controller test_only=true");
		}
	}

	// Read-only native-target seam. Common ownership, related-waypoint and MIF
	// checks stay in CF_ReadRearPacingParticipant for every backend.
	protected bool CF_ReadRearPacingTarget(IEntity predecessor, CF_EntityFollowWaypoint waypoint,
		CF_EntityFollowActivity activity, out string reason, out string binding)
	{
		reason = "direct_target_mismatch";
		binding = "binding=real_predecessor";
		if (activity.m_Entity.m_Value != predecessor || waypoint.GetEntity() != predecessor)
			return false;
		if (predecessor) binding += " native_target_id=" + predecessor.GetID();
		binding += " waypoint_id=" + waypoint.GetID() + " activity_sequence=" + activity.CF_GetSequence();
		reason = "direct_target";
		return true;
	}

	// Fresh original-pilot/selected-order evidence, not a cached speed request.
	// Pacing may observe a braking participant without granting movement credit
	// or changing its order. Other callers retain the forward-motion predicate.
	bool CF_ReadRearPacingParticipant(CF_ConvoySession session, IEntity predecessor, out vector velocity, out float speed,
		out string reason, out string binding, bool observeBraking = false, bool observeOwnedArrival = false)
	{
		reason = "common_guard";
		binding = "binding=unavailable";
		// Only the head's local cap caller opts in. Captured/Panel Wait is
		// never a moving participant; ordinary successor reads stay strict.
		bool ownedArrival;
		if (observeOwnedArrival && m_iState == CF_ARRIVING)
		{
			bool selectedWait;
			ownedArrival = CF_HasOwnedArrivalPhase(session, m_Truck, selectedWait) && !selectedWait;
		}
		if (m_Session != session || !session || (m_iState != CF_FOLLOWING && !ownedArrival) || !CF_IsOrdinaryEntityContext())
			return false;
		if (m_LeadVehicle != predecessor || CF_IsInitialDepartureWaiting() || m_bArrivalRoadHold || m_bArrivalTrailHold || m_bEntityFallbackFailed)
			return false;
		ChimeraCharacter driver = ChimeraCharacter.Cast(m_Driver);
		if (!CF_IsLiveAIPilot(driver, m_Truck) || IsDriverDestroyed(driver) || IsAssignedTruckDestroyed())
			return false;
		CompartmentAccessComponent access = driver.GetCompartmentAccessComponent();
		CharacterControllerComponent character = driver.GetCharacterController();
		if (!access || access.IsGettingIn() || access.IsGettingOut() || !character || character.IsUnconscious())
			return false;
		AIAgent agent = driver.GetAIControlComponent().GetAIAgent();
		if (agent.GetParentGroup() != m_Group || m_Truck.GetWorld() != GetGame().GetWorld())
			return false;
		SCR_AIUtilityComponent pilotUtility = SCR_AIUtilityComponent.Cast(agent.FindComponent(SCR_AIUtilityComponent));
		if (!pilotUtility)
			return false;
		SCR_AIMoveInFormationBehavior behavior = SCR_AIMoveInFormationBehavior.Cast(pilotUtility.GetCurrentBehavior());
		if (!behavior || behavior.GetActionState() == EAIActionState.COMPLETED || behavior.GetActionState() == EAIActionState.FAILED)
			return false;
		CF_EntityFollowWaypoint waypoint = CF_GetEntityFollowWaypoint();
		if (!waypoint || !HasOwnWaypointInGroup() || m_Group.GetCurrentWaypoint() != waypoint)
			return false;
		CF_EntityFollowActivity activity = waypoint.CF_GetActivity();
		SCR_AIGroupUtilityComponent groupUtility = SCR_AIGroupUtilityComponent.Cast(m_Group.FindComponent(SCR_AIGroupUtilityComponent));
		if (!activity || !groupUtility || groupUtility.GetCurrentAction() != activity)
			return false;
		if (activity.GetActionState() == EAIActionState.COMPLETED || activity.GetActionState() == EAIActionState.FAILED)
			return false;
		if (activity.m_RelatedWaypoint != waypoint)
			return false;
		if (behavior.GetRelatedGroupActivity() != activity)
			return false;
		if (!CF_ReadRearPacingTarget(predecessor, waypoint, activity, reason, binding))
			return false;
		reason = "physical_motion";
		CarControllerComponent car = CarControllerComponent.Cast(m_Truck.FindComponent(CarControllerComponent));
		Physics physics = m_Truck.GetPhysics();
		if (!car || !car.GetSimulation() || !physics)
			return false;
		speed = car.GetSimulation().GetSpeedKmh();
		if (!(speed > -1000 && speed < 1000))
			return false;
		if (!observeBraking && (car.GetSimulation().GetGear() < 2 || !(speed > 1.5)))
			return false;
		velocity = physics.GetVelocity();
		velocity[1] = 0;
		float velocitySquared = velocity.LengthSq();
		if (!(velocitySquared >= 0 && velocitySquared < 1000000))
			return false;
		vector facing = m_Truck.GetWorldTransformAxis(2);
		facing[1] = 0;
		if (facing.LengthSq() < 0.5)
			return false;
		facing.Normalize();
		float forwardMotion = vector.Dot(velocity, facing);
		if (!(forwardMotion > -1000 && forwardMotion < 1000))
			return false;
		if (observeBraking)
			return true;
		// Native speed/forward gear can coexist with backward world motion.
		return forwardMotion > 1.5 / 3.6;
	}

	protected float CF_ResetRearPacing(float frontLimit, string reason, string binding = "")
	{
		m_RearPacingSuccessor = null;
		m_fRearPacingCap = -1;
		m_fRearPacingLastMs = -1;
		if (m_bEntityRearPacing && reason != m_sRearPacingReason && m_iRearPacingLogs < 256)
		{
			m_iRearPacingLogs++;
			Print("[ConvoyFollower] ENTITY_REAR_PACING: unit=" + m_iUnitNumber +
				" eligible=false reason=" + reason + " " + binding + " front_limit_kmh=" + frontLimit + " adjustment_applied=false constraint_released=true");
		}
		m_sRearPacingReason = reason;
		return frontLimit;
	}

	override protected float CF_AdjustFollowingSpeedCap(float frontLimit, float predecessorSpeed, IEntity target)
	{
		if (!m_bEntityRearPacing)
			return frontLimit;
		if (m_Predecessor || !m_Session || !m_Session.IsCurrentLeader(this))
			return CF_ResetRearPacing(frontLimit, "not_current_head");
		vector velocity;
		float speed;
		string participantReason;
		string selfBinding;
		if (!CF_ReadRearPacingParticipant(m_Session, target, velocity, speed, participantReason, selfBinding, true, true))
			return CF_ResetRearPacing(frontLimit, "self_not_ordinary_moving_" + participantReason, selfBinding);
		CF_EntityFollowDriverControllerComponent successor = CF_EntityFollowDriverControllerComponent.Cast(m_Session.CF_GetImmediateActiveSuccessor(this));
		if (!successor)
			return CF_ResetRearPacing(frontLimit, "no_exact_private_successor");
		vector rearVelocity;
		float rearSpeed;
		string rearBinding;
		if (!successor.CF_ReadRearPacingParticipant(m_Session, m_Truck, rearVelocity, rearSpeed, participantReason, rearBinding, true))
			return CF_ResetRearPacing(frontLimit, "successor_not_ordinary_moving_" + participantReason, rearBinding);
		Vehicle rearTruck = successor.CF_GetAssignedVehicle();
		Physics targetPhysics = target.GetPhysics();
		if (!rearTruck || !targetPhysics)
			return CF_ResetRearPacing(frontLimit, "fresh_physics_unavailable");
		vector rearFacing = rearTruck.GetWorldTransformAxis(2);
		rearFacing[1] = 0;
		if (rearFacing.LengthSq() < 0.5)
			return CF_ResetRearPacing(frontLimit, "invalid_successor_heading");
		rearFacing.Normalize();
		float rearForwardSpeed = vector.Dot(rearVelocity, rearFacing) * 3.6;
		if (!(rearForwardSpeed > -1000 && rearForwardSpeed < 1000))
			return CF_ResetRearPacing(frontLimit, "invalid_successor_forward_speed");
		rearForwardSpeed = Math.Max(0, rearForwardSpeed);
		vector targetVelocity = targetPhysics.GetVelocity();
		targetVelocity[1] = 0;
		vector targetFacing = target.GetWorldTransformAxis(2);
		targetFacing[1] = 0;
		if (targetFacing.LengthSq() < 0.5)
			return CF_ResetRearPacing(frontLimit, "invalid_predecessor_heading");
		targetFacing.Normalize();
		bool movingPredecessor = predecessorSpeed > 1.5 && vector.Dot(targetVelocity, targetFacing) > 1.5 / 3.6;
		float stopMps = CF_ENTITY_STOP_SPEED_KMH / 3.6;
		bool stoppedPredecessor = Math.AbsFloat(predecessorSpeed) <= CF_ENTITY_STOP_SPEED_KMH;
		stoppedPredecessor = stoppedPredecessor && targetVelocity.LengthSq() >= 0 && targetVelocity.LengthSq() <= stopMps * stopMps;
		if (!movingPredecessor && !stoppedPredecessor)
			return CF_ResetRearPacing(frontLimit, "predecessor_not_forward_or_stopped");
		if (stoppedPredecessor)
		{
			// Stopping the player's lead does not remove rear pressure while
			// this head still approaches it. Require the exact healthy native
			// real-target lease; never extend this to a captured Wait or reverse.
			CF_OriginalFollowLease lease = m_OriginalFollowLease;
			if (m_EntityCapturedWait || !lease || lease.Predecessor != target || lease.NativeTarget != target ||
				!lease.Activity || lease.Activity.Lease != lease)
				return CF_ResetRearPacing(frontLimit, "stopped_head_lease_unavailable");
			string leaseReason;
			if (!lease.Executing(m_Group, leaseReason))
				return CF_ResetRearPacing(frontLimit, "stopped_head_lease_" + leaseReason);
			vector ownFacing = m_Truck.GetWorldTransformAxis(2);
			ownFacing[1] = 0;
			if (!(ownFacing.LengthSq() >= 0.5 && ownFacing.LengthSq() <= 1.5))
				return CF_ResetRearPacing(frontLimit, "stopped_head_heading_invalid");
			ownFacing.Normalize();
			if (!(vector.Dot(velocity, ownFacing) >= -stopMps))
				return CF_ResetRearPacing(frontLimit, "stopped_head_reversing");
		}
		vector rearLink = m_Truck.GetOrigin() - rearTruck.GetOrigin();
		rearLink[1] = 0;
		float rearGap = rearLink.Length();
		vector frontLink = target.GetOrigin() - m_Truck.GetOrigin();
		frontLink[1] = 0;
		float frontGap = frontLink.Length();
		if (rearGap < 1 || frontGap < 1)
			return CF_ResetRearPacing(frontLimit, "invalid_link_geometry");
		rearLink.Normalize();
		frontLink.Normalize();
		float opening = vector.Dot(velocity - rearVelocity, rearLink);
		float closing = vector.Dot(velocity - targetVelocity, frontLink);
		if (!(opening > -1000 && opening < 1000 && closing > -1000 && closing < 1000))
			return CF_ResetRearPacing(frontLimit, "invalid_relative_velocity");
		float movingGap = CF_ConvoySettings.Get().m_fMovingGap;
		float predictedGap = rearGap + Math.Max(0, opening) * 1.5;
		float pressure = Math.Clamp((predictedGap - movingGap * 1.5) / (movingGap * 0.5), 0, 1);
		// At full pressure cooperate with actual forward tail motion, including
		// zero during native braking/reverse. This cap never creates a new order.
		float cooperationLimit = Math.Min(frontLimit, rearForwardSpeed);
		float desiredCap = frontLimit + (cooperationLimit - frontLimit) * pressure;
		float now = GetGame().GetWorld().GetWorldTime();
		if (m_RearPacingSuccessor != successor || m_fRearPacingLastMs < 0)
		{
			m_RearPacingSuccessor = successor;
			m_fRearPacingCap = desiredCap;
			m_fRearPacingLastMs = now;
		}
		float elapsed = Math.Clamp((now - m_fRearPacingLastMs) / 1000.0, 0, 0.5);
		// Tighten immediately. Restore the current shaped cap promptly only
		// while the larger front gap opens and the rear gap is already closing.
		string recoveryReason = "steady";
		if (desiredCap < m_fRearPacingCap)
		{
			m_fRearPacingCap = desiredCap;
			recoveryReason = "tightening";
		}
		else if (desiredCap > m_fRearPacingCap)
		{
			bool frontNeedsRecovery = frontGap > rearGap && frontGap > movingGap * 2;
			// closing < 0 means front opening; opening < 0 means rear closing.
			if (frontNeedsRecovery && closing < 0 && opening < 0)
			{
				m_fRearPacingCap = desiredCap;
				recoveryReason = "front_opening_rear_closing";
			}
			else
			{
				m_fRearPacingCap = Math.Min(desiredCap, m_fRearPacingCap + 3.0 * elapsed);
				recoveryReason = "rate_limited";
			}
		}
		m_fRearPacingLastMs = now;
		float adjusted = Math.Min(frontLimit, m_fRearPacingCap);
		if (m_iRearPacingLogs < 256 && (m_sRearPacingReason != "active" || now >= m_fRearPacingNextLogMs))
		{
			m_iRearPacingLogs++;
			m_fRearPacingNextLogMs = now + 1000;
			string record = "[ConvoyFollower] ENTITY_REAR_PACING: unit=" + m_iUnitNumber + " eligible=true";
			record += " truck_id=" + m_Truck.GetID() + " predecessor_id=" + target.GetID();
			record += " successor_id=" + rearTruck.GetID() + " front_gap_m=" + frontGap + " rear_gap_m=" + rearGap;
			record += " front_closing_mps=" + closing + " rear_opening_mps=" + opening + " pressure=" + pressure;
			record += " predecessor_kmh=" + predecessorSpeed + " actual_kmh=" + speed + " successor_kmh=" + rearSpeed;
			record += " front_limit_kmh=" + frontLimit + " successor_forward_kmh=" + rearForwardSpeed + " desired_cap_kmh=" + desiredCap;
			record += " policy_request_kmh=" + adjusted + " stopped_predecessor_approach=" + stoppedPredecessor + " control_writes=false target_changed=false";
			record += " recovery_reason=" + recoveryReason;
			record += " " + rearBinding;
			Print(record);
		}
		m_sRearPacingReason = "active";
		return adjusted;
	}

	protected bool CF_IsPanelWaitContext()
	{
		return m_iState == CF_PANEL_HOLD && m_bPanelHoldRequested;
	}

	// Shared ownership only: neither mode may compete with seat work or a
	// private movement order. This observer never cancels an unowned action.
	protected bool CF_CapturedWaitOwnershipReady(ChimeraCharacter driver, Vehicle truck, SCR_AIGroup group, bool retiringBayMove = false)
	{
		if (!m_bEntityFollowPrototype || !m_bEntityCapturedWait || !Replication.IsServer() || !GetGame() ||
			!GetGame().GetWorld() || CF_ConvoySession.CF_IsWorldCleanup() || CF_IsControlBlocked())
			return false;
		if ((m_Waypoint && (!retiringBayMove || m_Waypoint != m_ExplicitBayMove)) || m_OriginalFollowLease || m_bDeferredWaypointClear ||
			m_bDeferredControlDismiss || m_bOriginalFollowResetPending)
			return false;
		if (!driver || !truck || !group || driver != m_Driver || truck != m_Truck || group != m_Group || GetOwner() != driver)
			return false;
		World world = GetGame().GetWorld();
		if (driver.GetWorld() != world || truck.GetWorld() != world || group.GetWorld() != world || group.GetAgentsCount() != 1)
			return false;
		if (!m_Session || (m_Session.GetUnitNumber(this) <= 0 && (!CF_IsExplicitWaitingParked() || !m_Session.IsOwnedRadioMember(this))) || !CF_IsLiveAIPilot(driver, truck) ||
			IsDriverDestroyed(driver) || IsAssignedTruckDestroyed())
			return false;
		PlayerManager players = GetGame().GetPlayerManager();
		if (!players || !m_Leader || m_iOrderingPlayerId <= 0 || m_Leader.GetWorld() != world ||
			players.GetPlayerControlledEntity(m_iOrderingPlayerId) != m_Leader)
			return false;
		if (CF_ConvoySession.GetForPlayer(m_Leader) != m_Session || m_Session.GetOrderingPlayerId() != m_iOrderingPlayerId ||
			CF_ConvoySession.IsVehicleAssignedToAnotherDriver(truck, this))
			return false;
		CharacterControllerComponent character = driver.GetCharacterController();
		CompartmentAccessComponent access = driver.GetCompartmentAccessComponent();
		if (!character || character.IsUnconscious() || !access || access.IsGettingIn() || access.IsGettingOut())
			return false;
		AIAgent agent = driver.GetAIControlComponent().GetAIAgent();
		if (!agent || agent.GetParentGroup() != group || agent.GetControlledEntity() != driver)
			return false;
		SCR_AIGroupUtilityComponent groupUtility = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));
		SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(agent.FindComponent(SCR_AIUtilityComponent));
		if (!groupUtility || !utility) return false;
		array<AIWaypoint> waypoints = {};
		group.GetWaypoints(waypoints);
		foreach (AIWaypoint waypoint : waypoints)
			if (!retiringBayMove || waypoint != m_ExplicitBayMove) return false;
		array<ref AIActionBase> actions = {};
		groupUtility.GetActions(actions);
		foreach (AIActionBase action : actions)
		{
			if (!action || action.GetActionState() == EAIActionState.COMPLETED || action.GetActionState() == EAIActionState.FAILED) continue;
			if (CF_EntityFollowActivity.Cast(action) || SCR_AIGetInActivity.Cast(action) || SCR_AIGetOutActivity.Cast(action)) return false;
			if (retiringBayMove)
			{
				if (SCR_AIFollowActivity.Cast(action)) return false;
				SCR_AIMoveActivity move = SCR_AIMoveActivity.Cast(action);
				if (move && (!m_ExplicitBayMove || move.m_RelatedWaypoint != m_ExplicitBayMove || move.m_Entity.m_Value != m_ExplicitBayMove)) return false;
			}
		}
		actions.Clear();
		utility.GetActions(actions);
		foreach (AIActionBase behavior : actions)
		{
			if (!behavior || behavior.GetActionState() == EAIActionState.COMPLETED || behavior.GetActionState() == EAIActionState.FAILED) continue;
			if (SCR_AIGetInVehicle.Cast(behavior) || SCR_AIGetOutVehicle.Cast(behavior)) return false;
			SCR_AIMoveInFormationBehavior movement = SCR_AIMoveInFormationBehavior.Cast(behavior);
			if (retiringBayMove && movement)
			{
				SCR_AIMoveActivity related = SCR_AIMoveActivity.Cast(movement.GetRelatedGroupActivity());
				if (!related || !m_ExplicitBayMove || related.m_RelatedWaypoint != m_ExplicitBayMove) return false;
			}
		}
		return true;
	}

	bool CF_HasCapturedWaitLease(CF_EntityCapturedWait action, ChimeraCharacter driver, Vehicle truck, SCR_AIGroup group)
	{
		if (!action || action != m_EntityCapturedWait || !action.HasOriginalPilot())
			return false;
		if (!CF_CapturedWaitOwnershipReady(driver, truck, group))
			return false;
		if (!action.HasOriginalBinding(this, driver, truck, group, m_Session, m_Leader, m_iOrderingPlayerId))
			return false;
		// Explicit Hold owns the stationary pilot independently of a moving,
		// replaced or absent predecessor. Speed limits acquisition, not tenure.
		if (action.IsParked()) return CF_IsExplicitWaitingParked() && !m_Waypoint;
		if (action.IsPanelHold()) return CF_IsPanelWaitContext() && !m_bEntityPanelWaitFailed;
		if (action.IsExplicitBay())
		{
			vector bay;
			return CF_IsExplicitBayWaitContext(bay) && action.HasBayAnchors(bay, truck.GetOrigin());
		}
		// Arrival retains its original target, stopped-anchor and state gates.
		if (!CF_IsOrdinaryEntityContext() || m_iState != CF_ARRIVING || !m_bArrivalRoadHold || m_bArrivalRoadRecoveryBlocked)
			return false;
		if (!m_bEntityStopAttempted || m_EntityStopTarget != m_EntityWaitTarget ||
			m_EntityWaitTarget != m_LeadVehicle || !m_EntityWaitTarget || m_Waypoint)
			return false;
		SCR_DamageManagerComponent targetDamage = SCR_DamageManagerComponent.GetDamageManager(m_EntityWaitTarget);
		if (m_EntityWaitTarget == truck || (targetDamage && targetDamage.IsDestroyed()))
			return false;
		// Acquisition still requires a measured slow, stable stop. Once held,
		// retain this seated Wait through predecessor settling/physics nudges.
		// Retire it on the same actual departure geometry as UpdateFollowing,
		// instead of losing it on a 1m shift and leaving native Idle driving.
		ChimeraCharacter leader = ChimeraCharacter.Cast(m_Leader);
		if (!leader) return false;
		bool departure = leader.IsInVehicle() && !m_bUnloadSequenceHold &&
			vector.Distance(m_vArrivalAnchorPosition, m_EntityWaitTarget.GetOrigin()) >= CF_ARRIVAL_RESUME_DISTANCE &&
			vector.Distance(truck.GetOrigin(), m_EntityWaitTarget.GetOrigin()) >= CF_ConvoySettings.Get().m_fMovingGap + CF_ARRIVAL_RESUME_GAP_BUFFER;
		return !departure;
	}

	// Read-only handoff evidence. A slow or explicitly held predecessor is
	// not an ordinary arrival; require its exact selected owned arrival Wait.
	bool CF_HasSelectedArrivalWait(CF_ConvoySession session, IEntity truck)
	{
		if (m_EntityCapturedWait && m_EntityCapturedWait.IsExplicitBay()) return false;
		return CF_HasSelectedStoppedWait(session, truck);
	}

	bool CF_HasSelectedExplicitBayWait(CF_ConvoySession session, IEntity truck)
	{
		if (!m_EntityCapturedWait || !m_EntityCapturedWait.IsExplicitBay()) return false;
		return CF_HasSelectedStoppedWait(session, truck);
	}

	protected bool CF_HasSelectedStoppedWait(CF_ConvoySession session, IEntity truck)
	{
		if (!session || session != m_Session || !truck || truck != m_Truck ||
			m_bEntityFallbackFailed || m_bOriginalFollowBlocked || m_bArrivalRoadRecoveryBlocked)
			return false;
		if (m_iState != CF_ARRIVING || !m_EntityCapturedWait || m_EntityCapturedWait.IsPanelHold())
			return false;
		if (m_EntityCapturedWait.GetActionState() == EAIActionState.COMPLETED ||
			m_EntityCapturedWait.GetActionState() == EAIActionState.FAILED)
			return false;
		ChimeraCharacter driver = ChimeraCharacter.Cast(m_Driver);
		if (!CF_HasCapturedWaitLease(m_EntityCapturedWait, driver, m_Truck, m_Group))
			return false;
		float speed;
		if (!CF_EntitySpeed(m_Truck, speed) || !(speed >= 0 && speed <= CF_ENTITY_STOP_SPEED_KMH))
			return false;
		SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(driver.GetAIControlComponent().GetAIAgent().FindComponent(SCR_AIUtilityComponent));
		return utility && utility.GetCurrentBehavior() == m_EntityCapturedWait;
	}

	// Arrival intent is distinct from measured capture. A following tail may
	// adopt our healthy real-target approach before our selected Wait exists.
	bool CF_HasOwnedArrivalPhase(CF_ConvoySession session, IEntity truck, out bool selectedWait)
	{
		selectedWait = CF_HasSelectedArrivalWait(session, truck);
		if (selectedWait) return true;
		if (!session || session != m_Session || !truck || truck != m_Truck || m_iState != CF_ARRIVING)
			return false;
		if (m_bEntityFallbackFailed || m_bOriginalFollowBlocked || m_bArrivalRoadRecoveryBlocked ||
			!m_bEntityStopAttempted || !m_LeadVehicle || m_EntityStopTarget != m_LeadVehicle)
			return false;
		CF_OriginalFollowLease lease = m_OriginalFollowLease;
		if (!lease || lease.Predecessor != m_LeadVehicle || lease.NativeTarget != m_LeadVehicle ||
			lease.Distance != CF_ConvoySettings.Get().m_fStoppedGap || !lease.Activity || lease.Activity.Lease != lease)
			return false;
		string reason;
		if (!lease.Executing(m_Group, reason)) return false;
		float targetSpeed;
		return CF_EntitySpeed(m_LeadVehicle, targetSpeed) && targetSpeed >= 0 && targetSpeed <= CF_ENTITY_STOP_SPEED_KMH;
	}

	protected void CF_ReleaseCapturedWait(string reason, bool allowNativeCompletion = true)
	{
		if (!m_EntityCapturedWait)
			return;
		CF_EntityCapturedWait action = m_EntityCapturedWait;
		bool mayComplete = allowNativeCompletion && !CF_IsControlBlocked();
		action.Retire(mayComplete);
		m_EntityCapturedWait = null;
		m_EntityWaitTarget = null;
		Print("[ConvoyFollower] ENTITY_CAPTURE_WAIT_RELEASE: unit=" + m_iUnitNumber +
			" action=" + action.ToString() + " reason=" + reason + " state=" + action.GetActionState() +
			" panel_hold=" + action.IsPanelHold() + " native_completion_allowed=" + mayComplete + " only_owned_wait=true");
	}

	protected void CF_CapturedWaitFailed(string reason, bool panelHold)
	{
		m_bEntityFallbackFailed = true;
		if (panelHold) m_bEntityPanelWaitFailed = true;
		Print("[ConvoyFollower] ENTITY_CAPTURE_WAIT_FAILED: unit=" + m_iUnitNumber +
			" reason=" + reason + " panel_hold=" + panelHold + " panel_retry_blocked=" + m_bEntityPanelWaitFailed);
	}

	protected void CF_AcquireCapturedWait(bool panelHold = false, bool explicitBay = false)
	{
		if (!m_bEntityCapturedWait || m_EntityCapturedWait)
			return;
		ChimeraCharacter driver = ChimeraCharacter.Cast(m_Driver);
		vector bay;
		if (explicitBay && (panelHold || !CF_IsExplicitBayWaitContext(bay) ||
			!CF_CapturedWaitOwnershipReady(driver, m_Truck, m_Group))) return;
		if (panelHold)
		{
			if (m_bEntityPanelWaitFailed || !CF_IsPanelWaitContext() || !CF_CapturedWaitOwnershipReady(driver, m_Truck, m_Group)) return;
			float speed;
			if (!CF_EntitySpeed(m_Truck, speed) || !(speed >= 0 && speed <= 2.0)) return;
		}
		if (!CF_IsLiveAIPilot(driver, m_Truck))
		{
			CF_CapturedWaitFailed("original_pilot_unavailable", panelHold);
			return;
		}
		SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(driver.GetAIControlComponent().GetAIAgent().FindComponent(SCR_AIUtilityComponent));
		if (!utility)
		{
			CF_CapturedWaitFailed("pilot_utility_missing", panelHold);
			return;
		}
		m_EntityWaitTarget = null;
		if (!panelHold && !explicitBay)
		{
			if (!m_LeadVehicle) { CF_CapturedWaitFailed("arrival_target_missing", false); return; }
			m_EntityWaitTarget = m_LeadVehicle;
			m_vEntityWaitTargetAnchor = m_LeadVehicle.GetOrigin();
		}
		m_EntityCapturedWait = new CF_EntityCapturedWait(utility, null);
		m_EntityCapturedWait.Bind(this, driver, m_Truck, m_Group, m_Session, m_Leader, m_iOrderingPlayerId, panelHold);
		if (explicitBay) m_EntityCapturedWait.BindExplicitBay(bay, m_vExplicitBayStableTruck);
		if (!CF_HasCapturedWaitLease(m_EntityCapturedWait, driver, m_Truck, m_Group))
		{
			CF_CapturedWaitFailed("capture_lease_rejected", panelHold);
			CF_ReleaseCapturedWait("capture_lease_rejected");
			return;
		}
		utility.AddAction(m_EntityCapturedWait);
		m_fEntityWaitNextLogMs = 0;
		m_iEntityWaitLogs = 0;
		m_bEntityWaitWasSelected = false;
		string targetId = "none";
		if (m_EntityWaitTarget) targetId = m_EntityWaitTarget.GetID().ToString();
		Print("[ConvoyFollower] ENTITY_CAPTURE_WAIT_BIND: unit=" + m_iUnitNumber +
			" enabled=" + m_bEntityCapturedWait + " action=" + m_EntityCapturedWait.ToString() +
			" driver_id=" + driver.GetID() + " truck_id=" + m_Truck.GetID() + " group_id=" + m_Group.GetID() +
			" owner_id=" + m_Leader.GetID() + " owner_player_id=" + m_iOrderingPlayerId + " panel_hold=" + panelHold +
			" target_id=" + targetId + " priority_level=2100 group_activity=null control_writes=false");
		if (explicitBay) Print("[ConvoyFollower] EXPLICIT_BAY_WAIT_BIND: unit=" + m_iUnitNumber +
			" bay=" + bay + " anchor=" + m_vExplicitBayStableTruck + " ordinary_arrival_credit=false");
	}

	protected bool CF_IsExplicitBayWaitContext(out vector bay, bool retiringBayMove = false)
	{
		if (!m_Session || !m_Truck || !m_Group || !m_LeadVehicle || m_bPanelHoldRequested || m_bForwardOutboundHoldRequested ||
			m_bEntityFallbackFailed || m_bOriginalFollowBlocked || m_bArrivalRoadRecoveryBlocked ||
			(m_Waypoint && (!retiringBayMove || m_Waypoint != m_ExplicitBayMove)) ||
			!m_Session.CF_GetAdmittedExplicitBay(this, m_Truck, m_LeadVehicle, bay) || !CF_IsSettledAtExplicitBay(bay)) return false;
		float speed;
		if (!CF_EntitySpeed(m_Truck, speed) || !(speed >= 0 && speed <= CF_ENTITY_STOP_SPEED_KMH)) return false;
		SCR_AIGroupUtilityComponent utility = SCR_AIGroupUtilityComponent.Cast(m_Group.FindComponent(SCR_AIGroupUtilityComponent));
		if (!utility) return false;
		array<ref AIActionBase> actions = {};
		utility.GetActions(actions);
		foreach (AIActionBase action : actions)
		{
			SCR_AIMoveActivity move = SCR_AIMoveActivity.Cast(action);
			if (move && action.GetActionState() != EAIActionState.COMPLETED && action.GetActionState() != EAIActionState.FAILED &&
				(!retiringBayMove || !m_ExplicitBayMove || move.m_RelatedWaypoint != m_ExplicitBayMove || move.m_Entity.m_Value != m_ExplicitBayMove)) return false;
		}
		return true;
	}

	protected void CF_ObserveExplicitBayStop()
	{
		if (m_EntityCapturedWait) return;
		vector bay;
		ChimeraCharacter driver = ChimeraCharacter.Cast(m_Driver);
		if (!CF_IsExplicitBayWaitContext(bay, true) || !CF_CapturedWaitOwnershipReady(driver, m_Truck, m_Group, true))
		{
			m_fExplicitBayStableStartMs = -1;
			return;
		}
		float now = GetGame().GetWorld().GetWorldTime();
		if (m_fExplicitBayStableStartMs < 0 || vector.Distance(m_vExplicitBayStableBay, bay) >= 0.01 ||
			vector.Distance(m_vExplicitBayStableTruck, m_Truck.GetOrigin()) > CF_ENTITY_STOP_POSE_METERS)
		{
			m_fExplicitBayStableStartMs = now;
			m_vExplicitBayStableBay = bay;
			m_vExplicitBayStableTruck = m_Truck.GetOrigin();
			return;
		}
		if (now - m_fExplicitBayStableStartMs < CF_ENTITY_STOP_STABLE_SECONDS * 1000) return;
		if (m_Waypoint)
		{
			if (m_Waypoint != m_ExplicitBayMove) return;
			Print("[ConvoyFollower] EXPLICIT_BAY_MOVE_RETIRE: unit=" + m_iUnitNumber + " waypoint_id=" + m_ExplicitBayMove.GetID() +
				" bay=" + bay + " stopped_anchor=" + m_vExplicitBayStableTruck + " stable_world_s=" + (now - m_fExplicitBayStableStartMs) / 1000 + " foreign_orders_preserved=true");
			SCR_AIGroupUtilityComponent groupUtility = SCR_AIGroupUtilityComponent.Cast(m_Group.FindComponent(SCR_AIGroupUtilityComponent));
			if (!groupUtility) return;
			// Native cancellation retires only MOVE activities bound to our exact
			// waypoint. Do not mark the waypoint completed or cancel seat/foreign
			// work. Physical arrival/stability above is the readiness evidence.
			groupUtility.CancelActivitiesRelatedToWaypoint(m_ExplicitBayMove, SCR_AIMoveActivity, true);
			ClearWaypoints();
		}
		CF_AcquireCapturedWait(false, true);
	}

	override bool CF_AdmitExplicitUnloadBay(vector bay, IEntity leadVehicle)
	{
		bool admitted = super.CF_AdmitExplicitUnloadBay(bay, leadVehicle);
		if (admitted) CF_RecordExplicitBayMove(m_vUnloadBayGoal);
		return admitted;
	}

	protected void CF_RecordExplicitBayMove(vector destination)
	{
		if (!m_Waypoint || !m_bUnloadSequenceHold || !m_bUnloadBayGoalValid || m_iState != CF_ARRIVING ||
			vector.Distance(destination, m_vUnloadBayGoal) >= 0.01 || !HasOwnWaypointInGroup()) return;
		m_ExplicitBayMove = m_Waypoint;
		m_fExplicitBayStableStartMs = -1;
		Print("[ConvoyFollower] EXPLICIT_BAY_MOVE_OWNED: unit=" + m_iUnitNumber + " waypoint_id=" + m_ExplicitBayMove.GetID() + " goal=" + destination);
	}

	// Seat departure ends our Wait ownership. The base controller retains
	// Hold/assignment and owns reboarding; this predicate issues no order.
	bool CF_CanRetirePanelWaitForPilotDeparture(CF_EntityCapturedWait action)
	{
		if (!action || action != m_EntityCapturedWait || !action.IsPanelHold() || !CF_IsPanelWaitContext()) return false;
		if (!Replication.IsServer() || !GetGame() || CF_ConvoySession.CF_IsWorldCleanup() || CF_IsControlBlocked()) return false;
		World world = GetGame().GetWorld();
		ChimeraCharacter driver = ChimeraCharacter.Cast(m_Driver);
		if (!world || !driver || GetOwner() != driver || !m_Truck || !m_Group || !m_Session || !m_Leader) return false;
		if (!action.HasImmutableBinding(this, driver, m_Truck, m_Group, m_Session, m_Leader, m_iOrderingPlayerId)) return false;
		if (driver.GetWorld() != world || m_Truck.GetWorld() != world || m_Group.GetWorld() != world || m_Leader.GetWorld() != world) return false;
		if (m_Waypoint || m_OriginalFollowLease || m_bDeferredWaypointClear || m_bDeferredControlDismiss || m_bOriginalFollowResetPending) return false;
		if (m_Session.GetUnitNumber(this) <= 0 || CF_ConvoySession.GetForPlayer(m_Leader) != m_Session ||
			m_Session.GetOrderingPlayerId() != m_iOrderingPlayerId || CF_ConvoySession.IsVehicleAssignedToAnotherDriver(m_Truck, this)) return false;
		PlayerManager players = GetGame().GetPlayerManager();
		if (!players || m_iOrderingPlayerId <= 0 || players.GetPlayerControlledEntity(m_iOrderingPlayerId) != m_Leader) return false;
		ChimeraCharacter owner = ChimeraCharacter.Cast(m_Leader);
		if (!owner || IsDriverDestroyed(owner) || IsDriverDestroyed(driver) || IsAssignedTruckDestroyed()) return false;
		CharacterControllerComponent character = driver.GetCharacterController();
		CompartmentAccessComponent access = driver.GetCompartmentAccessComponent();
		AIControlComponent control = driver.GetAIControlComponent();
		AIAgent agent;
		if (control) agent = control.GetAIAgent();
		if (!character || character.IsUnconscious() || !access || access.IsGettingIn() || !agent) return false;
		if (agent.GetControlledEntity() != driver || agent.GetParentGroup() != m_Group || m_Group.GetAgentsCount() != 1) return false;
		CarControllerComponent car = CarControllerComponent.Cast(m_Truck.FindComponent(CarControllerComponent));
		if (!car || !car.GetPilotCompartmentSlot()) return false;
		IEntity occupant = car.GetPilotCompartmentSlot().GetOccupant();
		if (occupant && occupant != driver) return false;
		array<AIWaypoint> waypoints = {};
		m_Group.GetWaypoints(waypoints);
		if (!waypoints.IsEmpty()) return false;
		if (!driver.IsInVehicle()) return true;
		BaseCompartmentSlot slot = access.GetCompartment();
		return access.IsGettingOut() && access.GetVehicleIn(driver) == m_Truck &&
			slot && slot == car.GetPilotCompartmentSlot() && slot.GetOccupant() == driver;
	}

	protected void CF_ObserveCapturedWait()
	{
		if (!m_EntityCapturedWait)
			return;
		bool panelHold = m_EntityCapturedWait.IsPanelHold();
		// A direct possession may precede the notification callback. Expected
		// retirement leaves no failure latch; safe AI handback may bind anew.
		if (panelHold && CF_IsControlBlocked())
		{
			CF_ReleaseCapturedWait("panel_player_control_changed", false);
			return;
		}
		ChimeraCharacter driver = ChimeraCharacter.Cast(m_Driver);
		bool terminalWait = m_EntityCapturedWait.GetActionState() == EAIActionState.COMPLETED ||
			m_EntityCapturedWait.GetActionState() == EAIActionState.FAILED;
		if ((!terminalWait || m_EntityCapturedWait.WasPilotDeparture()) &&
			CF_CanRetirePanelWaitForPilotDeparture(m_EntityCapturedWait))
		{
			CF_ReleaseCapturedWait("panel_original_pilot_departure", false);
			return; // No failure reset or capture credit; base reboarding owns the exit.
		}
		if (!CF_HasCapturedWaitLease(m_EntityCapturedWait, driver, m_Truck, m_Group))
		{
			if (panelHold) CF_CapturedWaitFailed("panel_lease_lost", true);
			CF_ReleaseCapturedWait("capture_state_target_or_owner_changed");
			return;
		}
		if (m_EntityCapturedWait.GetActionState() == EAIActionState.COMPLETED ||
			m_EntityCapturedWait.GetActionState() == EAIActionState.FAILED)
		{
			CF_CapturedWaitFailed("owned_wait_became_terminal", panelHold);
			CF_ReleaseCapturedWait("terminal_no_reacquire");
			return;
		}
		SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(driver.GetAIControlComponent().GetAIAgent().FindComponent(SCR_AIUtilityComponent));
		bool selected = utility && utility.GetCurrentBehavior() == m_EntityCapturedWait;
		float now = GetGame().GetWorld().GetWorldTime();
		if (m_iEntityWaitLogs < 48 && (selected != m_bEntityWaitWasSelected || now >= m_fEntityWaitNextLogMs))
		{
			m_iEntityWaitLogs++;
			m_fEntityWaitNextLogMs = now + 5000;
			m_bEntityWaitWasSelected = selected;
			string targetId = "none";
			if (m_EntityWaitTarget) targetId = m_EntityWaitTarget.GetID().ToString();
			Print("[ConvoyFollower] ENTITY_CAPTURE_WAIT_STATUS: unit=" + m_iUnitNumber +
				" enabled=true selected=" + selected + " action=" + m_EntityCapturedWait.ToString() +
				" state=" + m_EntityCapturedWait.GetActionState() + " target_id=" + targetId +
				" origin=" + m_Truck.GetOrigin() + " own_arrival_hold=" + m_bArrivalRoadHold +
				" panel_hold=" + panelHold + " panel_hold_requested=" + m_bPanelHoldRequested);
		}
	}

	protected void CF_AcquireParkedWait()
	{
		if (!m_bEntityCapturedWait || m_EntityCapturedWait || !CF_IsExplicitWaitingParked() || m_Waypoint || !CF_IsPanelVehicleSlow(2.0)) return;
		ChimeraCharacter driver = ChimeraCharacter.Cast(m_Driver);
		if (!CF_CapturedWaitOwnershipReady(driver, m_Truck, m_Group)) return;
		SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(driver.GetAIControlComponent().GetAIAgent().FindComponent(SCR_AIUtilityComponent));
		if (!utility) return;
		m_EntityCapturedWait = new CF_EntityCapturedWait(utility, null);
		m_EntityCapturedWait.Bind(this, driver, m_Truck, m_Group, m_Session, m_Leader, m_iOrderingPlayerId, false);
		m_EntityCapturedWait.BindParked();
		utility.AddAction(m_EntityCapturedWait);
		Print("[ConvoyFollower] WAITING_AREA_WAIT_BIND: unit=" + m_iUnitNumber + " original_pilot=true");
	}

	override void CF_CompleteUnloadDeparture()
	{
		if (m_bExplicitWaitingParking && CF_IsAtUnloadWaitingPoint() && m_Waypoint && CF_ShouldRemainSeated())
		{
			SCR_AIGroupUtilityComponent utility = SCR_AIGroupUtilityComponent.Cast(m_Group.FindComponent(SCR_AIGroupUtilityComponent));
			if (!utility) return;
			utility.CancelActivitiesRelatedToWaypoint(m_Waypoint, SCR_AIMoveActivity, true);
		}
		super.CF_CompleteUnloadDeparture();
		CF_AcquireParkedWait();
	}

	override bool CF_ResumeWaitingArea()
	{
		if (!CF_CanDepartWaitingArea()) return false;
		CF_ReleaseCapturedWait("explicit_departure_from_waiting_area");
		return super.CF_ResumeWaitingArea();
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		CF_ObserveOriginalFollow();
		CF_ObserveCapturedWait();
		super.EOnFrame(owner, timeSlice);
		CF_AcquireCapturedWait(true);
		CF_ObserveExplicitBayStop();
		CF_AcquireParkedWait();
	}

	override protected void SetState(int state)
	{
		if (state != CF_ARRIVING)
		{
			m_fExplicitBayStableStartMs = -1;
			m_ExplicitBayMove = null;
		}
		if (state != CF_FOLLOWING && state != CF_ARRIVING)
			CF_RevokeOriginalFollow("state_change");
		if (m_EntityCapturedWait)
		{
			bool keep = state == CF_ARRIVING && !m_EntityCapturedWait.IsPanelHold();
			if (m_EntityCapturedWait.IsPanelHold()) keep = state == CF_PANEL_HOLD;
			if (m_EntityCapturedWait.IsParked()) keep = state == CF_UNLOAD_DEPARTED;
			if (!keep) CF_ReleaseCapturedWait("state_change");
		}
		if (m_iState == CF_PANEL_HOLD && state != CF_PANEL_HOLD)
			m_bEntityPanelWaitFailed = false; // Historical fallback evidence is retained.
		if (state != CF_FOLLOWING)
			CF_ResetRearPacing(-1, "state_change");
		super.SetState(state);
	}

	override protected void ResetToIdle()
	{
		CF_RevokeOriginalFollow("reset_to_idle");
		if (m_OriginalFollowLease && m_Waypoint == m_OriginalFollowLease.Waypoint)
		{
			ChimeraCharacter originalDriver = m_OriginalFollowLease.Driver;
			bool terminal = CF_ConvoySession.CF_IsWorldCleanup() || !originalDriver || !m_Truck;
			if (!terminal) terminal = IsDriverDestroyed(originalDriver) || IsAssignedTruckDestroyed();
			if (terminal)
			{
				m_OriginalFollowLease.Record("terminal_reset_detached_no_native_cancellation");
				m_Waypoint = null;
			}
			else if (!CF_OriginalSlotReady("reset"))
			{
				m_bOriginalFollowResetPending = true;
				return;
			}
		}
		m_OriginalFollowLease = null;
		m_bOriginalFollowResetPending = false;
		m_fOriginalRetireStartMs = -1;
		m_bOriginalRetireTimedOut = false;
		m_bOriginalFollowBlocked = false;
		m_bOriginalFollowBlockApplied = false;
		// An ended assignment must not poison the same driver's next one.
		// Keep the historical witness; only clear active failure after its exact
		// old native slot passed the retirement checks above.
		m_bEntityFallbackFailedEver = m_bEntityFallbackFailedEver || m_bEntityFallbackFailed;
		m_bEntityFallbackFailed = false;
		m_sOriginalFollowFailure = string.Empty;
		CF_ResetEntityStopEpisode();
		CF_ReleaseCapturedWait("reset_to_idle");
		m_bEntityPanelWaitFailed = false;
		CF_ResetRearPacing(-1, "reset_to_idle");
		super.ResetToIdle();
	}

	override void CF_SetConvoyTarget(CF_DriverControllerComponent predecessor, int unitNumber)
	{
		if (predecessor != m_Predecessor)
			CF_RevokeOriginalFollow("predecessor_changed");
		if (Replication.IsServer() && (predecessor != m_Predecessor || unitNumber != m_iUnitNumber))
		{
			if (!m_EntityCapturedWait || !m_EntityCapturedWait.IsPanelHold())
				CF_ReleaseCapturedWait("convoy_target_change");
			CF_ResetRearPacing(-1, "convoy_target_change");
		}
		super.CF_SetConvoyTarget(predecessor, unitNumber);
	}

	override void CF_OnBeforePlayerPossess(IEntity entity)
	{
		if (entity && (entity == m_Driver || entity == m_Truck))
		{
			CF_RevokeOriginalFollow("before_player_possession");
			if (m_OriginalFollowLease && CF_OriginalCanRetire(m_OriginalFollowLease))
				ClearWaypoints();
			CF_ReleaseCapturedWait("before_player_possession");
			CF_ResetRearPacing(-1, "before_player_possession");
		}
		super.CF_OnBeforePlayerPossess(entity);
	}

	override void CF_OnPlayerControlChanged(IEntity from, IEntity to)
	{
		if (to && (to == m_Driver || to == m_Truck))
		{
			CF_RevokeOriginalFollow("late_player_control_change_no_native_retirement");
			CF_ReleaseCapturedWait("late_player_control_change", false);
			CF_ResetRearPacing(-1, "late_player_control_change");
		}
		super.CF_OnPlayerControlChanged(from, to);
	}

	override void CF_DetachForWorldCleanup()
	{
		CF_RevokeOriginalFollow("world_cleanup");
		CF_ReleaseCapturedWait("world_cleanup", false);
		CF_ResetRearPacing(-1, "world_cleanup");
		super.CF_DetachForWorldCleanup();
	}

	override void OnDelete(IEntity owner)
	{
		CF_RevokeOriginalFollow("controller_deleted");
		if (m_OriginalFollowLease && m_Waypoint == m_OriginalFollowLease.Waypoint)
		{
			if (CF_OriginalCanRetire(m_OriginalFollowLease))
				ClearWaypoints();
			else
			{
				// Base OnDelete removes m_Waypoint directly, bypassing virtual
				// ClearWaypoints. Detach our pointer before that unsafe path.
				m_OriginalFollowLease.Record("delete_detached_no_native_cancellation");
				m_Waypoint = null;
			}
		}
		CF_ReleaseCapturedWait("controller_deleted", !CF_ConvoySession.CF_IsWorldCleanup());
		CF_ResetRearPacing(-1, "controller_deleted");
		super.OnDelete(owner);
	}

	// Read-only admission for an explicit command, independent of movement
	// state. This temporary snapshot owns no action, waypoint or generation.
	protected bool CF_OriginalResumePilotReady(out string reason)
	{
		if (!GetGame() || !GetGame().GetWorld() || !m_Session || m_Session.GetUnitNumber(this) <= 0 ||
			CF_ConvoySession.IsVehicleAssignedToAnotherDriver(m_Truck, this))
		{
			reason = "session_membership_or_reservation";
			return false;
		}
		if (m_bOriginalFollowResetPending || m_bDeferredControlDismiss)
		{
			reason = "dismissal_or_reset_pending";
			return false;
		}
		CF_OriginalFollowLease snapshot = new CF_OriginalFollowLease();
		snapshot.Controller = this;
		snapshot.Driver = ChimeraCharacter.Cast(m_Driver);
		snapshot.Truck = m_Truck;
		snapshot.Group = m_Group;
		snapshot.Session = m_Session;
		snapshot.OriginalWorld = GetGame().GetWorld();
		AIControlComponent control;
		if (snapshot.Driver) control = snapshot.Driver.GetAIControlComponent();
		if (control) snapshot.Agent = control.GetAIAgent();
		if (m_Group) snapshot.Utility = SCR_AIGroupUtilityComponent.Cast(m_Group.FindComponent(SCR_AIGroupUtilityComponent));
		snapshot.Seal();
		if (!CF_OriginalPilotSafe(snapshot))
		{
			reason = "original_pilot_ownership";
			return false;
		}
		if (snapshot.HasSeatWork())
		{
			reason = "active_or_unobserved_seat_work";
			return false;
		}
		reason = "ready";
		return true;
	}

	// Active request failure guidance only; retained failure history is not a warning.
	override string CF_GetPanelStateLabel()
	{
		if (!m_bOriginalFollowGraph || !m_bOriginalFollowBlocked)
			return super.CF_GetPanelStateLabel();
		if (m_sOriginalFollowFailure.IndexOf("guard_or_request_failure_admitted_move_3_handler_") != 0)
			return super.CF_GetPanelStateLabel();
		// Keep ownership, boarding and genuine route/recovery failures authoritative.
		if (m_bControlBoardingTimedOut || CF_IsControlBlocked() || m_bPanelHoldReboardBlocked)
			return super.CF_GetPanelStateLabel();
		if (CF_HasPersistentFollowFailure() || m_bEntityPanelWaitFailed || !CF_IsBoarded())
			return super.CF_GetPanelStateLabel();
		if (m_bOriginalFollowResetPending || m_bDeferredControlDismiss)
			return super.CF_GetPanelStateLabel();
		if (CF_IsPanelHeld()) return "held: Resume retries order";
		if (m_bPanelHoldRequested) return super.CF_GetPanelStateLabel();
		if (m_iState == CF_FOLLOWING || m_iState == CF_ARRIVING)
			return "order failed: Hold, then Resume";
		return super.CF_GetPanelStateLabel();
	}

	override string CF_GetResumeFailureReason()
	{
		if (m_bEntityFallbackFailed && !m_bOriginalFollowBlocked)
			return "previous movement failed; dismiss and recruit this driver again";
		if (m_bOriginalFollowGraph && m_bOriginalFollowBlocked)
			return "original follow order blocked: " + m_sOriginalFollowFailure;
		return super.CF_GetResumeFailureReason();
	}

	// Explicit command preflight only: retire our exact old slot and acknowledge
	// its active failure. The caller still owns Wait release/state/departure.
	protected bool CF_PrepareOriginalFollowResume(out bool retry, out int previousGeneration)
	{
		retry = false;
		previousGeneration = m_iOriginalFollowGeneration;
		if (!m_bOriginalFollowGraph) return true;
		string reason;
		if (!CF_OriginalResumePilotReady(reason))
		{
			Print("[ConvoyFollower] ORIGINAL_FOLLOW_RESUME_REJECTED: unit=" + m_iUnitNumber + " reason=" + reason);
			return false;
		}
		if (CF_HasPersistentFollowFailure() || m_bEntityPanelWaitFailed)
		{
			Print("[ConvoyFollower] ORIGINAL_FOLLOW_RESUME_REJECTED: unit=" + m_iUnitNumber + " reason=unrelated_persistent_or_panel_wait_failure");
			return false;
		}
		bool originalFailure = m_bOriginalFollowBlocked;
		// Revocation/timeout alone does not attribute a fallback to this order.
		if (m_bEntityFallbackFailed && !originalFailure)
		{
			Print("[ConvoyFollower] ORIGINAL_FOLLOW_RESUME_REJECTED: unit=" + m_iUnitNumber + " reason=unattributed_entity_fallback_failure");
			return false;
		}
		retry = originalFailure || m_bOriginalRetireTimedOut;
		if (m_OriginalFollowLease && m_OriginalFollowLease.Revoked) retry = true;
		if (!retry) return true;
		string previousReason = m_sOriginalFollowFailure;
		if (m_OriginalFollowLease)
		{
			if (!CF_OriginalCanRetire(m_OriginalFollowLease))
			{
				Print("[ConvoyFollower] ORIGINAL_FOLLOW_RESUME_REJECTED: unit=" + m_iUnitNumber + " reason=old_slot_not_safe_to_retire");
				return false;
			}
			ClearWaypoints();
		}
		if (m_Waypoint || m_OriginalFollowLease)
		{
			Print("[ConvoyFollower] ORIGINAL_FOLLOW_RESUME_REJECTED: unit=" + m_iUnitNumber + " reason=old_order_slot_remains");
			return false;
		}
		if (originalFailure)
		{
			m_bEntityFallbackFailedEver = m_bEntityFallbackFailedEver || m_bEntityFallbackFailed;
			m_bEntityFallbackFailed = false;
		}
		m_bOriginalFollowBlocked = false;
		m_bOriginalFollowBlockApplied = false;
		m_bOriginalRetireTimedOut = false;
		m_fOriginalRetireStartMs = -1;
		m_sOriginalFollowFailure = string.Empty;
		string line = "[ConvoyFollower] ORIGINAL_FOLLOW_RETRY_PREPARED: unit=" + m_iUnitNumber;
		line += " previous_generation=" + previousGeneration + " previous_reason=" + previousReason;
		line += " original_failure=" + originalFailure + " historical_failure=" + CF_HasEntityFallbackFailure();
		line += " exact_old_slot_retired=true new_order=false movement_claim=false";
		Print(line);
		return true;
	}

	override bool CF_PanelResume()
	{
		if (!Replication.IsServer() || !CF_CanPanelResume())
			return false;
		bool retry;
		int previousGeneration;
		if (!CF_PrepareOriginalFollowResume(retry, previousGeneration)) return false;
		if (retry)
		{
			// Base Resume otherwise treats FOLLOWING/ARRIVING as still active.
			// Keep explicit Hold intent until the base command releases it.
			SetState(CF_WAITING_FOR_PREDECESSOR);
		}
		CF_ReleaseCapturedWait("panel_resume");
		CF_ResetRearPacing(-1, "panel_resume");
		bool accepted = super.CF_PanelResume();
		if (m_bOriginalFollowGraph && m_bOriginalFollowBlocked)
			accepted = false;
		if (m_bOriginalFollowGraph)
			Print("[ConvoyFollower] ORIGINAL_FOLLOW_RESUME_ATTEMPT: unit=" + m_iUnitNumber +
				" retry=" + retry + " previous_generation=" + previousGeneration + " generation=" + m_iOriginalFollowGeneration +
				" accepted=" + accepted + " physical_progress_claim=false");
		return accepted;
	}

	bool CF_IsEntityFollowPrototypeEnabled()
	{
		return m_bEntityFollowPrototype;
	}

	bool CF_HasEntityFallbackFailure()
	{
		return m_bEntityFallbackFailed || m_bEntityFallbackFailedEver;
	}

	// Current health only; the historical getter above never loses a failure.
	bool CF_HasActiveEntityFallbackFailure()
	{
		return m_bEntityFallbackFailed;
	}

	CF_EntityFollowWaypoint CF_GetEntityFollowWaypoint()
	{
		return CF_EntityFollowWaypoint.Cast(m_Waypoint);
	}

	protected bool CF_CanIssueEntityFollow()
	{
		return m_bEntityFollowPrototype && Replication.IsServer() && !CF_ConvoySession.CF_IsWorldCleanup() &&
			!CF_IsControlBlocked() && CF_IsBoarded() && m_Group && m_Group.GetAgentsCount() == 1 &&
			m_iState == CF_FOLLOWING && !m_bPanelHoldRequested && !m_bUnloadSequenceHold &&
			!m_bForwardOutboundHoldRequested && m_fTargetStillSeconds < CF_STOP_DETECT_SECONDS &&
			Vehicle.Cast(m_LeadVehicle) && GetTargetVehicle(false) == m_LeadVehicle;
	}

	protected bool CF_IsOrdinaryEntityContext()
	{
		return m_bEntityFollowPrototype && Replication.IsServer() && !CF_ConvoySession.CF_IsWorldCleanup() &&
			!CF_IsControlBlocked() && CF_IsBoarded() && m_Group && m_Group.GetAgentsCount() == 1 &&
			(m_iState == CF_FOLLOWING || m_iState == CF_ARRIVING) && !m_bPanelHoldRequested &&
			!m_bUnloadSequenceHold && !m_bForwardOutboundHoldRequested &&
			Vehicle.Cast(m_LeadVehicle) && GetTargetVehicle(false) == m_LeadVehicle;
	}

	protected void CF_ResetEntityStopEpisode()
	{
		m_EntityStopTarget = null;
		m_bEntityStopAttempted = false;
		m_iEntityStopReplacements = 0;
		m_bEntityStablePoseValid = false;
		m_fEntityStableSeconds = 0;
	}

	protected bool CF_EntitySpeed(IEntity vehicle, out float speedKmh)
	{
		if (!vehicle)
			return false;
		CarControllerComponent car = CarControllerComponent.Cast(vehicle.FindComponent(CarControllerComponent));
		if (!car || !car.GetSimulation())
			return false;
		speedKmh = Math.AbsFloat(car.GetSimulation().GetSpeedKmh());
		return true;
	}

	protected void CF_FailEntityApproach(string reason)
	{
		m_bEntityFallbackFailed = true;
		ClearWaypoints();
		SetState(CF_ARRIVING);
		m_bArrivalRoadRecoveryBlocked = true;
		CF_UpdateVehicleBrake();
		Print("[ConvoyFollower] ENTITY_FOLLOW_FALLBACK_FAILED: unit=" + m_iUnitNumber + " reason=" + reason);
	}

	override protected bool IssueMoveWaypoint(vector destination)
	{
		if (m_EntityCapturedWait && m_EntityCapturedWait.IsExplicitBay() &&
			CF_HasCapturedWaitLease(m_EntityCapturedWait, ChimeraCharacter.Cast(m_Driver), m_Truck, m_Group)) return true;
		if (m_bOriginalFollowGraph && m_bOriginalFollowBlocked)
			return true; // Handled failure: base false path would StandDown/GetOut.
		if (m_OriginalArrivalResumeLease)
		{
			CF_OriginalFollowLease retained = m_OriginalArrivalResumeLease;
			m_OriginalArrivalResumeLease = null;
			string reason;
			if (!CF_CanRetainOriginalArrivalResume(retained, m_LeadVehicle, reason))
			{
				retained.Block("arrival_resume_continuity_lost_" + reason);
				return true; // Fail the owned lease; never rescue it with a new order.
			}
			Print("[ConvoyFollower] ORIGINAL_FOLLOW_ARRIVAL_RESUME_RETAINED: unit=" + m_iUnitNumber +
				" generation=" + retained.Generation + " sequence=" + retained.Activity.CF_GetSequence() +
				" waypoint_id=" + retained.Waypoint.GetID() + " target_id=" + retained.NativeTarget.GetID() +
				" desired_distance=" + retained.Distance + " exact_executing=true activity_replaced=false");
			return true;
		}
		// A native retry must not replace this stationary episode's sole order.
		if (CF_IsOrdinaryEntityContext() && m_bEntityStopAttempted && m_EntityStopTarget == m_LeadVehicle)
		{
			if (m_bArrivalRoadHold || (CF_GetEntityFollowWaypoint() && HasOwnWaypointInGroup()))
				return true;
			CF_FailEntityApproach("stopped_entity_order_lost");
			return false;
		}
		if (!CF_CanIssueEntityFollow())
		{
			if (!CF_OriginalSlotReady("base_move")) return true;
			bool issued = super.IssueMoveWaypoint(destination);
			vector bay;
			if (issued && m_Session && m_Session.CF_GetAdmittedExplicitBay(this, m_Truck, m_LeadVehicle, bay)) CF_RecordExplicitBayMove(destination);
			return issued;
		}
		CF_ResetEntityStopEpisode();
		return CF_CreateEntityFollow(destination, CF_ConvoySettings.Get().m_fMovingGap, "moving_follow");
	}

	protected bool CF_CreateEntityFollow(vector destination, float desiredDistance, string reason)
	{
		CF_ReleaseCapturedWait("new_entity_order");
		Resource prefab = Resource.Load(CF_ENTITY_WAYPOINT);
		if (!prefab.IsValid())
			return false;
		ClearWaypoints();
		if (m_OriginalFollowLease && m_Waypoint)
		{
			m_OriginalFollowLease.Block("direct_order_retirement_deferred");
			return true; // Handled blocked order, not movement success.
		}
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = destination;
		CF_EntityFollowWaypoint waypoint = CF_EntityFollowWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(prefab, null, params));
		if (!waypoint)
			return false;
		waypoint.SetEntity(m_LeadVehicle);
		if (waypoint.GetEntity() != m_LeadVehicle)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(waypoint);
			return false;
		}
		waypoint.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_GAMEMASTER);
		waypoint.SetCompletionRadius(desiredDistance);
		// Exactly the base controller's owned slot: Hold, rechain, reboarding,
		// player-control handover and deletion keep their existing cleanup path.
		m_Waypoint = waypoint;
		m_vLastWaypointPosition = destination;
		m_fWaypointSeconds = 0;
		CF_BindOriginalFollow(waypoint);
		m_Group.AddWaypoint(waypoint);
		string created = "[ConvoyFollower] ENTITY_FOLLOW_CREATED: unit=" + m_iUnitNumber;
		created += " waypoint_id=" + waypoint.GetID() + " target_id=" + m_LeadVehicle.GetID();
		created += " initial_trail_goal=" + destination + " desired_distance=" + waypoint.GetCompletionRadius();
		created += " reason=" + reason + " stop_episode=" + m_iEntityStopEpisode;
		created += " stop_replacements=" + m_iEntityStopReplacements;
		created += " target_basis=current_predecessor_entity autocomplete=false test_only=true";
		created += " captured_wait_enabled=" + m_bEntityCapturedWait;
		created += " rear_pacing_enabled=" + m_bEntityRearPacing;
		Print(created);
		return true;
	}

	override protected bool MoveWaypoint(vector destination)
	{
		if (m_bOriginalFollowGraph && m_bOriginalFollowBlocked)
			return true; // Sticky failure is visible to the private probe.
		if (CF_IsControlBlocked())
			return false;
		CF_EntityFollowWaypoint waypoint = CF_GetEntityFollowWaypoint();
		if (CF_IsOrdinaryEntityContext() && m_bEntityStopAttempted && m_EntityStopTarget == m_LeadVehicle)
		{
			if (m_bArrivalRoadHold)
				return true;
			if (!waypoint || !HasOwnWaypointInGroup() || waypoint.GetEntity() != m_LeadVehicle)
			{
				CF_FailEntityApproach("stopped_entity_order_lost");
				return false;
			}
		}
		if (CF_IsOrdinaryEntityContext() && waypoint && HasOwnWaypointInGroup() && waypoint.GetEntity() == m_LeadVehicle)
		{
			float nowMs = GetGame().GetWorld().GetWorldTime();
			if (m_iEntityRetainLogs < 64 && nowMs >= m_fNextEntityRetainLogMs)
			{
				m_iEntityRetainLogs++;
				m_fNextEntityRetainLogMs = nowMs + 5000;
				Print("[ConvoyFollower] ENTITY_FOLLOW_RETAINED: unit=" + m_iUnitNumber +
					" waypoint_id=" + waypoint.GetID() + " target_id=" + waypoint.GetEntity().GetID() +
					" requested_trail_goal=" + destination + " waypoint_age_s=" + m_fWaypointSeconds +
					" origin_updated=false activity_replaced=false");
			}
			return true;
		}
		if (CF_CanIssueEntityFollow())
			return IssueMoveWaypoint(destination);
		// An entity waypoint cannot become a fixed MOVE by changing its origin.
		if (waypoint)
			return CF_ReplaceEntityWithMove(destination, "non_ordinary_approach");
		return super.MoveWaypoint(destination);
	}

	protected bool CF_ReplaceEntityWithMove(vector destination, string reason)
	{
		if (CF_IsControlBlocked() || CF_ConvoySession.CF_IsWorldCleanup())
			return false;
		if (!CF_OriginalSlotReady("fixed_approach"))
			return true; // Hold intent remains accepted; no new native order.
		bool applied = super.IssueMoveWaypoint(destination);
		if (!applied)
		{
			// Fail visibly and retire our live entity target rather than keep
			// following after accepted Hold or an arrival transition.
			m_bEntityFallbackFailed = true;
			ClearWaypoints();
		}
		Print("[ConvoyFollower] ENTITY_FOLLOW_FIXED_APPROACH: unit=" + m_iUnitNumber +
			" reason=" + reason + " applied=" + applied + " goal=" + destination);
		return applied;
	}

	// Callers establish ordinary context and a stopped/owned-arrival predecessor.
	// This starts one approach; it grants no still time or capture evidence.
	protected bool CF_BeginStoppedEntityApproach()
	{
		m_EntityStopTarget = m_LeadVehicle;
		m_bEntityStopAttempted = true;
		m_iEntityStopEpisode++;
		m_iEntityStopReplacements = 1;
		m_bStopSettleIssued = true;
		// Parameter values bind when the tree starts. Retire the old owned
		// target first, then use the unchanged real-predecessor stopped gap.
		if (!CF_CreateEntityFollow(m_vLastWaypointPosition, CF_ConvoySettings.Get().m_fStoppedGap, "stopped_predecessor"))
		{
			CF_FailEntityApproach("stopped_entity_replacement_failed");
			return false;
		}
		return true;
	}

	// Keep this already executing arrival order while the exact immediate
	// predecessor finishes its own arrival. This predicate never calls itself.
	protected bool CF_CanRetainArrivalEpisodeDuringTargetMotion()
	{
		if (!m_bEntityStopAttempted || !m_LeadVehicle || m_EntityStopTarget != m_LeadVehicle || !m_Session)
			return false;
		CF_EntityFollowDriverControllerComponent predecessor = CF_EntityFollowDriverControllerComponent.Cast(m_Predecessor);
		if (!predecessor || predecessor.CF_GetAssignedVehicle() != m_LeadVehicle ||
			m_Session.CF_GetImmediateActiveSuccessor(predecessor) != this)
			return false;
		CF_OriginalFollowLease lease = m_OriginalFollowLease;
		if (!lease || lease.Predecessor != m_LeadVehicle || lease.NativeTarget != m_LeadVehicle ||
			lease.Distance != CF_ConvoySettings.Get().m_fStoppedGap || !lease.Activity || lease.Activity.Lease != lease)
			return false;
		string reason;
		if (!lease.Executing(m_Group, reason)) return false;
		bool predecessorCaptured;
		return predecessor.CF_HasOwnedArrivalPhase(m_Session, m_LeadVehicle, predecessorCaptured);
	}

	override protected void SettleBehindStoppedTarget(float elapsed, float separation)
	{
		if (m_bOriginalFollowGraph && m_bOriginalFollowBlocked)
			return;
		if (!CF_IsOrdinaryEntityContext() || (!CF_GetEntityFollowWaypoint() && !m_bEntityStopAttempted))
		{
			super.SettleBehindStoppedTarget(elapsed, separation);
			return;
		}
		// Keep the base still detector, intercepting before its virtual MOVE
		// refresh can turn a persistent entity order into a fixed road point.
		vector targetPosition = m_LeadVehicle.GetOrigin();
		if (m_EntityStopTarget && m_EntityStopTarget != m_LeadVehicle)
			CF_ResetEntityStopEpisode();
		if (vector.Distance(m_vLastTargetPosition, targetPosition) >= CF_STOP_DETECT_DISTANCE)
		{
			m_vLastTargetPosition = targetPosition;
			m_fTargetStillSeconds = 0;
			m_bEntityStablePoseValid = false;
			m_fEntityStableSeconds = 0;
			if (CF_CanRetainArrivalEpisodeDuringTargetMotion()) return;
			m_bStopSettleIssued = false;
			CF_ResetEntityStopEpisode();
			return;
		}
		m_fTargetStillSeconds += elapsed;
		float targetSpeed;
		if (m_fTargetStillSeconds < CF_STOP_DETECT_SECONDS ||
			!CF_EntitySpeed(m_LeadVehicle, targetSpeed) || targetSpeed > CF_ENTITY_STOP_SPEED_KMH)
		{
			m_bEntityStablePoseValid = false;
			m_fEntityStableSeconds = 0;
			return;
		}
		// A brief leader stop must not replace a healthy moving-gap lease
		// while this truck still approaches. Native cruise already brakes for
		// the real predecessor. Enter the tighter stopped-gap approach only
		// after the truck is near and slow; final capture keeps all its guards.
		if (!m_bEntityStopAttempted && m_bOriginalFollowGraph && m_OriginalFollowLease &&
			m_OriginalFollowLease.Distance == CF_ConvoySettings.Get().m_fMovingGap &&
			m_OriginalFollowLease.Predecessor == m_LeadVehicle && m_OriginalFollowLease.NativeTarget == m_LeadVehicle)
		{
			string approachReason;
			float approachSpeed;
			if (m_OriginalFollowLease.Executing(m_Group, approachReason) && CF_EntitySpeed(m_Truck, approachSpeed) &&
				(separation > CF_ConvoySettings.Get().m_fMovingGap + 2.0 || approachSpeed > CF_ENTITY_STOP_SPEED_KMH))
			{
				m_bEntityStablePoseValid = false;
				m_fEntityStableSeconds = 0;
				return;
			}
		}
		if (!m_bEntityStopAttempted && !CF_BeginStoppedEntityApproach())
			return;
		if (m_bArrivalRoadHold || m_bArrivalRoadRecoveryBlocked)
			return;
		CF_EntityFollowWaypoint arrivalWaypoint = CF_GetEntityFollowWaypoint();
		if (!arrivalWaypoint || !HasOwnWaypointInGroup() || arrivalWaypoint.GetEntity() != m_LeadVehicle)
		{
			CF_FailEntityApproach("stopped_entity_order_lost");
			return;
		}
		CF_EntityFollowActivity arrivalActivity = arrivalWaypoint.CF_GetActivity();
		// Selection can lag waypoint creation. Never count the intervening
		// stationary pose as evidence that the replacement ran successfully.
		if (!arrivalActivity)
		{
			m_bEntityStablePoseValid = false;
			m_fEntityStableSeconds = 0;
			return;
		}
		if (arrivalActivity.GetActionState() == EAIActionState.FAILED ||
			arrivalActivity.GetActionState() == EAIActionState.COMPLETED)
		{
			string rejected = "[ConvoyFollower] ENTITY_FOLLOW_ACTIVITY_REJECTED: unit=" + m_iUnitNumber;
			rejected += " sequence=" + arrivalActivity.CF_GetSequence() + " activity=" + arrivalActivity.ToString();
			rejected += " state=" + arrivalActivity.GetActionState() + " reason=terminal_before_capture";
			Print(rejected);
			CF_FailEntityApproach("stopped_entity_activity_terminal");
			return;
		}
		if (arrivalActivity.m_Entity.m_Value != m_LeadVehicle || arrivalActivity.m_RelatedWaypoint != arrivalWaypoint)
		{
			CF_FailEntityApproach("stopped_entity_activity_binding_mismatch");
			return;
		}
		SCR_AIGroupUtilityComponent arrivalUtility = SCR_AIGroupUtilityComponent.Cast(m_Group.FindComponent(SCR_AIGroupUtilityComponent));
		if (!arrivalUtility || arrivalUtility.GetCurrentAction() != arrivalActivity || m_Group.GetCurrentWaypoint() != arrivalWaypoint)
		{
			m_bEntityStablePoseValid = false;
			m_fEntityStableSeconds = 0;
			return;
		}
		float speed;
		bool close = separation <= CF_ConvoySettings.Get().m_fStoppedGap + 4.0 && CF_IsTruckNearMappedRoad();
		if (!close || !CF_EntitySpeed(m_Truck, speed) || speed > CF_ENTITY_STOP_SPEED_KMH)
		{
			m_bEntityStablePoseValid = false;
			m_fEntityStableSeconds = 0;
			return;
		}
		if (!m_bEntityStablePoseValid || vector.Distance(m_vEntityStablePose, m_Truck.GetOrigin()) > CF_ENTITY_STOP_POSE_METERS)
		{
			m_vEntityStablePose = m_Truck.GetOrigin();
			m_bEntityStablePoseValid = true;
			m_fEntityStableSeconds = 0;
			return;
		}
		m_fEntityStableSeconds += elapsed;
		if (m_fEntityStableSeconds < CF_ENTITY_STOP_STABLE_SECONDS)
			return;
		int arrivalSequence = arrivalActivity.CF_GetSequence();
		SetState(CF_ARRIVING);
		m_bArrivalRoadHold = true;
		ClearWaypoints();
		CF_UpdateVehicleBrake();
		CF_AcquireCapturedWait();
		string capture = "[ConvoyFollower] ENTITY_FOLLOW_ARRIVAL_CAPTURE: unit=" + m_iUnitNumber;
		capture += " gap_m=" + separation + " speed_kmh=" + speed + " stable_s=" + m_fEntityStableSeconds;
		capture += " pose_drift_m=" + vector.Distance(m_vEntityStablePose, m_Truck.GetOrigin());
		capture += " episode=" + m_iEntityStopEpisode + " replacements=" + m_iEntityStopReplacements;
		capture += " retired_sequence=" + arrivalSequence + " target_id=" + m_LeadVehicle.GetID();
		capture += " mapped_road=true owned_brake=" + m_bOwnVehicleBrake;
		capture += " native_cruise_owner=existing_controller";
		capture += " captured_wait_enabled=" + m_bEntityCapturedWait + " wait_owned=" + (m_EntityCapturedWait != null);
		Print(capture);
	}

	override protected bool CF_ShouldHoldVehicleBrake()
	{
		// The base road latch uses distance alone. This isolated comparison
		// captures only after the measured slow/stable test above has passed.
		if (CF_IsOrdinaryEntityContext() && m_iState == CF_ARRIVING &&
			(CF_GetEntityFollowWaypoint() || m_bEntityStopAttempted) &&
			!m_bArrivalRoadHold && !m_bArrivalRoadRecoveryBlocked && !m_bArrivalTrailHold)
			return false;
		return super.CF_ShouldHoldVehicleBrake();
	}

	protected bool CF_CanRetainOriginalArrivalResume(CF_OriginalFollowLease lease, IEntity target, out string reason)
	{
		reason = "context_or_pending_cleanup";
		if (!m_bOriginalFollowGraph || !lease || lease != m_OriginalFollowLease || m_bOriginalFollowBlocked ||
			m_bOriginalRetireTimedOut || m_bOriginalFollowResetPending || m_bDeferredControlDismiss || m_bDeferredWaypointClear)
			return false;
		if (m_bEntityFallbackFailed || m_EntityCapturedWait || m_bPanelHoldRequested || m_bEntityStopAttempted)
			return false;
		reason = "target_or_distance_changed";
		if (!target || target != m_LeadVehicle || lease.Predecessor != target || lease.NativeTarget != target ||
			lease.Distance != CF_ConvoySettings.Get().m_fMovingGap)
			return false;
		reason = "activity_lease_identity";
		if (!lease.Activity || lease.Activity.Lease != lease)
			return false;
		// Rechecks sealed identities, native pilot/handler, seat work, owned
		// waypoint, exact selected/executed activity, target, distance and priority.
		return lease.Executing(m_Group, reason);
	}

	override protected void StartFollowing(IEntity targetVehicle, bool rejoined, bool emitRadio)
	{
		m_OriginalArrivalResumeLease = null;
		if (m_iState == CF_ARRIVING && !rejoined && !emitRadio)
		{
			string reason;
			if (CF_CanRetainOriginalArrivalResume(m_OriginalFollowLease, targetVehicle, reason))
				m_OriginalArrivalResumeLease = m_OriginalFollowLease;
		}
		CF_ReleaseCapturedWait("start_following");
		CF_ResetRearPacing(-1, "start_following");
		CF_ResetEntityStopEpisode();
		super.StartFollowing(targetVehicle, rejoined, emitRadio);
		m_OriginalArrivalResumeLease = null;
	}

	override bool CF_RequestPanelHold()
	{
		if (!super.CF_RequestPanelHold())
			return false;
		CF_ResetRearPacing(-1, "explicit_hold_accepted");
		if (CF_CanPanelHold())
			return CF_PanelHold();
		if (!CF_GetEntityFollowWaypoint())
			return true;
		// Intent is already accepted. A continuously moving entity target must
		// become one fixed native approach before the base Hold early return.
		IEntity target = GetTargetVehicle(false);
		if (target)
			CF_ReplaceEntityWithMove(GetLeadTrailGoal(target), "explicit_hold_accepted");
		else
		{
			m_bEntityFallbackFailed = true;
			ClearWaypoints();
		}
		return true;
	}

	override bool CF_PanelHold()
	{
		if (!super.CF_PanelHold()) return false;
		// Repeated Hold retains a healthy action and never clears a failure.
		// A faster approach waits for the existing brake owner to slow it.
		CF_AcquireCapturedWait(true);
		return true;
	}
}
