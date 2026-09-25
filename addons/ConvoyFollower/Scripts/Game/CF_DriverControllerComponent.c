// Game Master spawned drivers form a convoy: Unit One follows the player and
// each later unit follows the truck immediately ahead. The server owns orders.
class CF_DriverControllerComponentClass : ScriptComponentClass
{
}

class CF_DriverControllerComponent : ScriptComponent
{
	protected static const ResourceName CF_BOARD_WAYPOINT = "{8AD8C82346156494}Prefabs/AI/Waypoints/AIWaypoint_GetInSelected.et";
	protected static const ResourceName CF_MOVE_WAYPOINT = "{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et";
	protected static const ResourceName CF_FOLLOW_WAYPOINT = "{A0509D3C4DD4475E}Prefabs/AI/Waypoints/AIWaypoint_Follow.et";
	protected static const ResourceName CF_GET_OUT_WAYPOINT = "{C40316EE26846CAB}Prefabs/AI/Waypoints/AIWaypoint_GetOut.et";

	protected static const int CF_IDLE = 0;
	protected static const int CF_BOARDING = 1;
	protected static const int CF_WAITING_FOR_LEAD = 2;
	protected static const int CF_FOLLOWING = 3;
	protected static const int CF_LOST = 4;
	protected static const int CF_GETTING_OUT = 5;
	protected static const int CF_PAUSED_ON_FOOT = 6;
	protected static const int CF_WAITING_FOR_PREDECESSOR = 7;
	protected static const int CF_ON_FOOT_FOLLOW = 8;
	protected static const int CF_ON_FOOT_HOLD = 9;
	protected static const int CF_ON_FOOT_BOARDING_PASSENGER = 10;
	protected static const int CF_ON_FOOT_RIDING_PASSENGER = 11;
	protected static const int CF_ON_FOOT_DISEMBARKING = 12;
	protected static const int CF_REBOARDING = 13;
	protected static const int CF_ARRIVING = 14;
	protected static const int CF_UNLOAD_QUEUE = 15;
	protected static const int CF_UNLOAD_DEPARTING = 16;
	protected static const int CF_UNLOAD_DEPARTED = 17;
	protected static const int CF_RETURN_WAIT = 18;
	protected static const int CF_RETURN_TURNING = 19;
	protected static const int CF_RETURN_BLOCKED = 20;

	protected static const float CF_ON_FOOT_GAP = 2.0;
	protected static const float CF_ON_FOOT_REORDER_DISTANCE = 3.0;
	protected static const float CF_ON_FOOT_STALL_CHECK_SECONDS = 8.0;
	protected static const float CF_ON_FOOT_STALL_DISTANCE = 6.0;
	protected static const float CF_PASSENGER_BOARD_TIMEOUT_SECONDS = 25.0;
	protected static const float CF_PASSENGER_EXIT_TIMEOUT_SECONDS = 45.0;
	protected static const float CF_WAYPOINT_REFRESH_SECONDS = 3.0;
	protected static const float CF_WAYPOINT_REFRESH_DISTANCE = 8.0;
	protected static const float CF_STOP_DETECT_DISTANCE = 1.0;
	protected static const float CF_STOP_DETECT_SECONDS = 6.0;
	protected static const float CF_ARRIVAL_RESUME_DISTANCE = 6.0;
	protected static const float CF_MISSING_WAYPOINT_RECOVERY_SECONDS = 8.0;
	protected static const float CF_ORDER_INVERSION_CHECK_DISTANCE = 80.0;
	protected static const float CF_ORDER_INVERSION_START_AHEAD = 12.0;
	protected static const float CF_ORDER_INVERSION_CLEAR_AHEAD = 3.0;
	protected static const float CF_ORDER_INVERSION_MAX_LATERAL = 12.0;
	protected static const float CF_STOP_SETTLE_DISTANCE = 14.0;
	protected static const float CF_ROUTE_SAMPLE_DISTANCE = 5.0;
	protected static const int CF_ROUTE_SAMPLE_LIMIT = 64;
	protected static const float CF_UNLOAD_BAY_CLEAR_DISTANCE = 18.0;
	protected static const float CF_UNLOAD_SLOT_RADIUS = 5.0;
	protected static const float CF_UNLOAD_SLOT_STILL_SECONDS = 2.0;
	protected static const float CF_UNLOAD_DEPART_TIMEOUT_SECONDS = 120.0;
	protected static const float CF_TURN_STAGE_TIMEOUT_SECONDS = 35.0;
	protected static const float CF_TURN_STAGE_RADIUS = 3.5;
	protected static const float CF_RETURN_TURN_TIMEOUT_SECONDS = 90.0;
	protected static const float CF_RETURN_HEADING_DOT = 0.7;
	protected static const float CF_RETURN_MERGE_GRACE_SECONDS = 180.0;

	// ScriptedUserAction visibility runs on clients, so mirror this compact state.
	[RplProp()]
	protected int m_iState = CF_IDLE;
	[RplProp()]
	protected bool m_bUnloadReleaseReady;
	[RplProp()]
	protected bool m_bSessionReleaseAllowed = true;
	protected int m_iStuckRetries;
	protected int m_iReboardAttempts;
	[RplProp()]
	protected int m_iOrderingPlayerId;
	[RplProp()]
	protected int m_iUnitNumber;
	protected bool m_bPauseWasLost;
	protected float m_fPollAccumulator;
	protected float m_fStateSeconds;
	protected float m_fLostSeconds;
	protected float m_fRangeWarningSeconds;
	protected bool m_bRangeWarningIssued;
	protected float m_fWaypointSeconds;
	protected float m_fStuckSeconds;
	protected float m_fTargetStillSeconds;
	protected bool m_bStopSettleIssued;
	protected bool m_bArrivalCloseLogged;
	protected bool m_bOrderInversionLogged;
	protected bool m_bUnloadSequenceHold;
	protected bool m_bUnloadAnchorValid;
	protected bool m_bUnloadClearReported;
	protected bool m_bUnloadSlotParkedLogged;
	protected bool m_bReturnTurnFailed;
	protected bool m_bSilentReturnFollow;
	protected bool m_bTurnProbeOccupied;
	protected int m_iUnloadReboardState;
	protected int m_iUnloadTurnPhase;
	protected int m_iReturnTurnPhase;
	protected float m_fUnloadSlotStillSeconds;
	protected float m_fArrivalTruckStillSeconds;
	protected float m_fTurnStageSeconds;
	protected float m_fReturnMergeGraceSeconds;
	protected float m_fCandidateDistance;
	protected vector m_vLastWaypointPosition;
	protected vector m_vLastTruckPosition;
	protected vector m_vLastTargetPosition;
	protected vector m_vLastStallTargetPosition;
	protected vector m_vArrivalAnchorPosition;
	protected vector m_vLastFootPosition;
	protected vector m_vUnloadAnchor;
	protected vector m_vUnloadWaitingPoint;
	protected vector m_vLastUnloadTruckPosition;
	protected vector m_vArrivalLastTruckPosition;
	protected vector m_vOutboundTravelDirection;
	protected vector m_vUnloadHomeDirection;
	protected vector m_vReturnHomeDirection;
	protected vector m_vUnloadTurnStage;
	protected vector m_vReturnTurnStage;
	protected ref array<vector> m_aRecentTruckPositions = {};
	protected IEntity m_Driver;
	protected IEntity m_Leader;
	protected IEntity m_LeadVehicle;
	protected IEntity m_LastPlayerVehicle;
	protected IEntity m_UnloadAnchorVehicle;
	protected Vehicle m_Truck;
	protected Vehicle m_Candidate;
	protected IEntity m_PassengerVehicle;
	protected bool m_bStopAfterPassengerExit;
	protected SCR_AIGroup m_Group;
	protected AIWaypoint m_Waypoint;
	protected CF_ConvoySession m_Session;
	protected CF_DriverControllerComponent m_Predecessor;

	protected void SetState(int state)
	{
		if (m_iState == state)
			return;

		if (state != CF_ARRIVING)
			SetUnloadReleaseReady(false);
		m_iState = state;
		if (Replication.IsServer())
			Replication.BumpMe();
	}

	protected void SetUnloadReleaseReady(bool ready)
	{
		if (m_bUnloadReleaseReady == ready)
			return;
		m_bUnloadReleaseReady = ready;
		if (Replication.IsServer())
			Replication.BumpMe();
	}

	protected void ResetRangeWarning()
	{
		m_fRangeWarningSeconds = 0;
		m_bRangeWarningIssued = false;
	}

	protected void RememberOrderingPlayer(IEntity leader)
	{
		SetOrderingPlayerId(0);
		PlayerManager manager = GetGame().GetPlayerManager();
		if (!manager || !leader)
			return;

		SetOrderingPlayerId(manager.GetPlayerIdFromControlledEntity(leader));
		if (m_iOrderingPlayerId <= 0)
		{
			Print("[ConvoyFollower] RADIO_UNAVAILABLE: ordering player ID unavailable");
		}
	}

	protected void SetOrderingPlayerId(int playerId)
	{
		if (m_iOrderingPlayerId == playerId)
			return;
		m_iOrderingPlayerId = playerId;
		if (Replication.IsServer())
			Replication.BumpMe();
	}

	protected void NotifySessionUnavailable()
	{
		CF_ConvoySession session = m_Session;
		m_Session = null;
		if (session && Replication.IsServer())
			session.OnDriverUnavailable(this);
	}

	protected void SendRadioCall(int eventId)
	{
		if (Replication.IsServer() && m_Session)
			m_Session.OnDriverStatus(this, eventId);
	}

	bool CanAssign(IEntity user)
	{
		if (!user || (m_iState != CF_IDLE && m_iState != CF_ON_FOOT_FOLLOW && m_iState != CF_ON_FOOT_HOLD))
			return false;
		if (IsOnFootFollower() && !CF_IsOwnedBy(user))
			return false;

		ChimeraCharacter driver = ChimeraCharacter.Cast(GetOwner());
		if (!driver || driver.IsInVehicle())
			return false;

		return FindNearestEmptyTruck(driver) != null;
	}

	bool CanStandDown()
	{
		return m_iState != CF_IDLE && m_iState != CF_GETTING_OUT;
	}

	bool IsIdle()
	{
		return m_iState == CF_IDLE;
	}

	bool IsOnFootFollower()
	{
		return m_iState == CF_ON_FOOT_FOLLOW || m_iState == CF_ON_FOOT_HOLD ||
			m_iState == CF_ON_FOOT_BOARDING_PASSENGER || m_iState == CF_ON_FOOT_RIDING_PASSENGER ||
			m_iState == CF_ON_FOOT_DISEMBARKING;
	}

	bool CanStartOnFoot(IEntity user)
	{
		if (m_iState != CF_IDLE || !user)
			return false;

		ChimeraCharacter driver = ChimeraCharacter.Cast(GetOwner());
		ChimeraCharacter player = ChimeraCharacter.Cast(user);
		return driver && player && !driver.IsInVehicle() && !player.IsInVehicle();
	}

	void StartOnFoot(IEntity user)
	{
		if (!Replication.IsServer() || !CanStartOnFoot(user))
			return;

		ChimeraCharacter driver = ChimeraCharacter.Cast(GetOwner());
		AIControlComponent aiControl = driver.GetAIControlComponent();
		if (!aiControl || !aiControl.GetAIAgent())
		{
			Print("[ConvoyFollower] FOOT_FOLLOW_REJECTED: driver has no AI agent");
			return;
		}

		m_Group = SCR_AIGroup.Cast(aiControl.GetAIAgent().GetParentGroup());
		if (!m_Group || m_Group.GetAgentsCount() != 1)
		{
			Print("[ConvoyFollower] FOOT_FOLLOW_REJECTED: driver needs its own one-member AI group");
			return;
		}

		m_Driver = driver;
		m_Leader = user;
		m_Truck = null;
		m_Session = null;
		m_Predecessor = null;
		m_iUnitNumber = 0;
		RememberOrderingPlayer(user);
		if (m_iOrderingPlayerId <= 0)
		{
			ResetToIdle();
			return;
		}

		ClearGroupWaypointsForNewOrder();
		m_vLastFootPosition = driver.GetOrigin();
		m_fWaypointSeconds = 0;
		SetState(CF_ON_FOOT_FOLLOW);
		if (!IssueOnFootFollowWaypoint())
		{
			Print("[ConvoyFollower] FOOT_FOLLOW_FAILED: could not create follow waypoint");
			ResetToIdle();
			return;
		}

		SetEventMask(m_Driver, EntityEvent.FRAME);
		Print("[ConvoyFollower] FOOT_FOLLOWING: driver staging behind player");
	}

	bool CanStopOnFoot(IEntity user)
	{
		return IsOnFootFollower() && CF_IsOwnedBy(user);
	}

	void StopOnFoot(IEntity user)
	{
		if (!Replication.IsServer() || !CanStopOnFoot(user))
			return;

		ClearWaypoints();
		ChimeraCharacter driver = ChimeraCharacter.Cast(m_Driver);
		if (driver && driver.IsInVehicle())
		{
			m_bStopAfterPassengerExit = true;
			if (BeginPassengerExit(driver))
				return;
		}
		ResetToIdle();
		Print("[ConvoyFollower] FOOT_FOLLOW_STOPPED: staged driver is idle");
	}

	bool IsWaitingForLead()
	{
		return m_iState == CF_WAITING_FOR_LEAD || m_iState == CF_WAITING_FOR_PREDECESSOR;
	}

	IEntity CF_GetDriverEntity()
	{
		return GetOwner();
	}

	int CF_GetOrderingPlayerId()
	{
		return m_iOrderingPlayerId;
	}

	bool CF_IsOwnedBy(IEntity user)
	{
		if (!user || m_iOrderingPlayerId <= 0)
			return false;
		PlayerManager manager = GetGame().GetPlayerManager();
		return manager && manager.GetPlayerIdFromControlledEntity(user) == m_iOrderingPlayerId;
	}

	Vehicle CF_GetAssignedVehicle()
	{
		return m_Truck;
	}

	bool CF_IsBoarded()
	{
		ChimeraCharacter driver = ChimeraCharacter.Cast(m_Driver);
		if (!driver || !m_Truck || !driver.IsInVehicle())
			return false;

		CompartmentAccessComponent compartment = driver.GetCompartmentAccessComponent();
		if (!compartment)
			return false;

		BaseCompartmentSlot slot = compartment.GetCompartment();
		return slot && slot.IsPiloting() && compartment.GetVehicleIn(driver) == m_Truck;
	}

	bool CF_IsMovementActive()
	{
		return m_iState == CF_FOLLOWING || m_iState == CF_ARRIVING;
	}

	int CF_GetUnitNumber()
	{
		return m_iUnitNumber;
	}

	bool CF_IsOrderInverted()
	{
		return m_bOrderInversionLogged && CF_IsMovementActive();
	}

	bool CF_IsActiveConvoyMember()
	{
		return m_Session && m_Session.GetUnitNumber(this) > 0 && CF_IsBoarded();
	}

	bool CF_CanReleaseAtUnload()
	{
		if (m_iState != CF_ARRIVING || !m_bUnloadReleaseReady || !m_bSessionReleaseAllowed)
			return false;
		// m_Truck is server-owned; action visibility uses the replicated flag,
		// while the server also verifies that this driver remains seated.
		return !Replication.IsServer() || CF_IsBoarded();
	}

	void CF_SetSessionReleaseAllowed(bool allowed)
	{
		if (!Replication.IsServer() || m_bSessionReleaseAllowed == allowed)
			return;
		m_bSessionReleaseAllowed = allowed;
		Replication.BumpMe();
	}

	bool CF_IsSessionReleaseAllowed()
	{
		return m_bSessionReleaseAllowed;
	}

	vector CF_GetUnloadAnchor()
	{
		if (m_bUnloadAnchorValid)
			return m_vUnloadAnchor;
		if (m_LastPlayerVehicle)
			return m_LastPlayerVehicle.GetOrigin();
		return vector.Zero;
	}

	IEntity CF_GetCachedPlayerVehicle()
	{
		return m_LastPlayerVehicle;
	}

	void CF_SetUnloadSequenceHold(bool active)
	{
		if (!Replication.IsServer())
			return;
		if (active && !m_bUnloadSequenceHold)
		{
			m_UnloadAnchorVehicle = m_LastPlayerVehicle;
			if (m_UnloadAnchorVehicle)
			{
				m_vUnloadAnchor = m_UnloadAnchorVehicle.GetOrigin();
				m_bUnloadAnchorValid = true;
			}
		}
		else if (!active)
		{
			m_UnloadAnchorVehicle = null;
			m_bUnloadAnchorValid = false;
		}
		m_bUnloadSequenceHold = active;
	}

	bool CF_IsUnloadDeparted()
	{
		return m_iState == CF_UNLOAD_DEPARTED && CF_IsBoarded();
	}

	// The rear action runs its visibility check on clients, where m_Truck is
	// server-owned. The session verifies the actual parked roster on use.
	bool CF_IsReturnParkedForActions()
	{
		return m_iState == CF_UNLOAD_DEPARTED;
	}

	bool CF_IsAtUnloadWaitingPoint()
	{
		return (m_iState == CF_UNLOAD_DEPARTING || m_iState == CF_UNLOAD_DEPARTED) &&
			CF_IsBoarded() && m_Truck && m_iUnloadTurnPhase == 2 &&
			vector.Distance(m_Truck.GetOrigin(), m_vUnloadWaitingPoint) <= CF_UNLOAD_SLOT_RADIUS + 2.0 &&
			m_fUnloadSlotStillSeconds >= CF_UNLOAD_SLOT_STILL_SECONDS;
	}

	bool CF_IsUnloadBayClear(vector anchor)
	{
		return CF_IsAtUnloadWaitingPoint() &&
			vector.Distance(m_Truck.GetOrigin(), anchor) >= CF_UNLOAD_BAY_CLEAR_DISTANCE;
	}

	protected bool TryGetOutboundTravelDirection(out vector direction)
	{
		direction = vector.Zero;
		if (!m_Truck)
			return false;

		vector current = m_Truck.GetOrigin();
		float dirX = 0;
		float dirZ = 0;
		float dirLength = 0;
		for (int i = m_aRecentTruckPositions.Count() - 1; i >= 0; i--)
		{
			vector previous = m_aRecentTruckPositions[i];
			dirX = current[0] - previous[0];
			dirZ = current[2] - previous[2];
			dirLength = Math.Sqrt(dirX * dirX + dirZ * dirZ);
			if (dirLength >= 12.0)
				break;
		}
		if (dirLength < 12.0)
		{
			// A short route or a long stop can leave too little sampled motion.
			// Use the truck heading only as a fallback; the slot still has to
			// pass the road, distance, and occupied-space checks below.
			return TryGetVehicleFacing(m_Truck, direction);
		}
		direction = Vector(dirX / dirLength, 0, dirZ / dirLength);
		vector facing;
		if (!TryGetVehicleFacing(m_Truck, facing) ||
			facing[0] * direction[0] + facing[2] * direction[2] < 0.5)
			return false;
		return true;
	}

	protected bool IsFacingDirection(vector direction)
	{
		if (!m_Truck || direction[0] * direction[0] + direction[2] * direction[2] < 0.5)
			return false;
		vector facing = m_Truck.GetWorldTransformAxis(2);
		float facingLength = Math.Sqrt(facing[0] * facing[0] + facing[2] * facing[2]);
		if (facingLength < 0.5)
			return false;
		float dot = (facing[0] * direction[0] + facing[2] * direction[2]) / facingLength;
		return dot >= CF_RETURN_HEADING_DOT;
	}

	protected bool TryGetVehicleFacing(IEntity vehicle, out vector direction)
	{
		direction = vector.Zero;
		if (!vehicle)
			return false;
		vector facing = vehicle.GetWorldTransformAxis(2);
		float length = Math.Sqrt(facing[0] * facing[0] + facing[2] * facing[2]);
		if (length < 0.5)
			return false;
		direction = Vector(facing[0] / length, 0, facing[2] / length);
		return true;
	}

	// A truck that has passed its predecessor can be sent backwards by a
	// literal moving-vehicle waypoint. Detect only nearby vehicles facing
	// roughly the same way, and keep the episode until the order clears.
	protected void ObserveConvoyOrder(IEntity targetVehicle, float separation)
	{
		if (!targetVehicle || !m_Truck)
		{
			m_bOrderInversionLogged = false;
			return;
		}
		if (separation > CF_ORDER_INVERSION_CHECK_DISTANCE && !m_bOrderInversionLogged)
			return;
		vector forward;
		if (!TryGetVehicleFacing(targetVehicle, forward))
			return;
		vector offset = m_Truck.GetOrigin() - targetVehicle.GetOrigin();
		float ahead = offset[0] * forward[0] + offset[2] * forward[2];
		if (ahead >= CF_ORDER_INVERSION_START_AHEAD && !m_bOrderInversionLogged)
		{
			float lateral = offset[0] * forward[2] - offset[2] * forward[0];
			if (lateral < 0)
				lateral = -lateral;
			if (lateral > CF_ORDER_INVERSION_MAX_LATERAL)
				return;
			vector truckForward;
			if (!TryGetVehicleFacing(m_Truck, truckForward) ||
				truckForward[0] * forward[0] + truckForward[2] * forward[2] < 0.55)
				return;
			m_bOrderInversionLogged = true;
			Print("[ConvoyFollower] ORDER_INVERSION: Unit " + m_iUnitNumber +
				" is " + ahead + " m ahead of its predecessor, gap " + separation + " m");
		}
		else if (ahead <= CF_ORDER_INVERSION_CLEAR_AHEAD && m_bOrderInversionLogged)
		{
			m_bOrderInversionLogged = false;
			m_fWaypointSeconds = CF_MISSING_WAYPOINT_RECOVERY_SECONDS;
			Print("[ConvoyFollower] ORDER_INVERSION_CLEARED: Unit " + m_iUnitNumber + " may resume its normal link");
		}
	}

	protected void HoldOutOfOrderWaypoint(IEntity targetVehicle)
	{
		if (!m_bOrderInversionLogged || !targetVehicle || !m_Truck || !HasOwnWaypointInGroup())
			return;
		vector forward;
		if (!TryGetVehicleFacing(targetVehicle, forward))
			return;
		vector remaining = m_vLastWaypointPosition - m_Truck.GetOrigin();
		float forwardDistance = remaining[0] * forward[0] + remaining[2] * forward[2];
		// A still-forward order can finish. If its goal is behind or at this
		// truck, remove it rather than asking the AI to turn through its convoy.
		if (forwardDistance > 5.0)
			return;
		ClearWaypoints();
		Print("[ConvoyFollower] ORDER_INVERSION_HOLD: Unit " + m_iUnitNumber + " waiting for predecessor/relink, no reverse chase");
	}

	// The head passed the outbound tail on its approach. Prefer that recorded
	// road path, then use a road-validated continuation. When a return truck
	// already parks behind the tail, the next slot must be between them.
	bool CF_FindReturnUnloadWaitingPoint(vector outboundTail, vector nearestParked,
		bool hasParked, out vector candidate)
	{
		candidate = vector.Zero;
		if (!Replication.IsServer() || !m_Truck || !CF_IsBoarded() || m_iState != CF_ARRIVING)
			return false;
		vector travel;
		if (!TryGetOutboundTravelDirection(travel))
			return false;
		m_vOutboundTravelDirection = travel;
		float slotDistance = 32.0;
		if (hasParked)
		{
			slotDistance = 15.0;
			if (vector.Distance(outboundTail, nearestParked) < 27.0)
				return false;
		}
		vector goal = outboundTail;
		float closest = 12.0;
		int tailIndex = -1;
		for (int i = 0; i < m_aRecentTruckPositions.Count(); i++)
		{
			float distance = vector.Distance(m_aRecentTruckPositions[i], outboundTail);
			if (distance < closest)
			{
				closest = distance;
				tailIndex = i;
			}
		}
		if (tailIndex >= 0)
		{
			float walked = 0;
			for (int j = tailIndex; j > 0; j--)
			{
				walked += vector.Distance(m_aRecentTruckPositions[j], m_aRecentTruckPositions[j - 1]);
				if (walked >= slotDistance)
				{
					goal = m_aRecentTruckPositions[j - 1];
					break;
				}
			}
			if (walked < slotDistance)
				tailIndex = -1;
		}
		if (tailIndex < 0)
		{
			goal[0] = goal[0] - travel[0] * slotDistance;
			goal[2] = goal[2] - travel[2] * slotDistance;
		}
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld)
			return false;
		RoadNetworkManager roads = aiWorld.GetRoadNetworkManager();
		if (!roads)
			return false;

		vector roadPoint;
		if (!roads.GetReachableWaypointInRoad(m_Truck.GetOrigin(), goal, 8.0, roadPoint))
			return false;
		float tailDistance = vector.Distance(roadPoint, outboundTail);
		float behindTail = (outboundTail[0] - roadPoint[0]) * travel[0] +
			(outboundTail[2] - roadPoint[2]) * travel[2];
		if (vector.Distance(roadPoint, goal) > 8.0 || tailDistance < 14.0 ||
			tailDistance > 45.0 || behindTail < 10.0 ||
			vector.Distance(roadPoint, CF_GetUnloadAnchor()) < CF_UNLOAD_BAY_CLEAR_DISTANCE)
			return false;
		if (hasParked)
		{
			float aheadOfParked = (roadPoint[0] - nearestParked[0]) * travel[0] +
				(roadPoint[2] - nearestParked[2]) * travel[2];
			if (vector.Distance(roadPoint, nearestParked) < 12.0 || aheadOfParked < 8.0)
				return false;
		}
		m_bTurnProbeOccupied = false;
		GetGame().GetWorld().QueryEntitiesBySphere(roadPoint, 5.5, ConsiderTurnObstacle);
		if (m_bTurnProbeOccupied)
			return false;

		float homeX = roadPoint[0] - outboundTail[0];
		float homeZ = roadPoint[2] - outboundTail[2];
		float homeLength = Math.Sqrt(homeX * homeX + homeZ * homeZ);
		if (homeLength < 15.0)
			return false;
		m_vUnloadHomeDirection = Vector(homeX / homeLength, 0, homeZ / homeLength);
		candidate = roadPoint;
		return true;
	}

	protected bool ConsiderTurnObstacle(IEntity entity)
	{
		if (entity != m_Truck && Vehicle.Cast(entity))
			m_bTurnProbeOccupied = true;
		return true;
	}

	protected bool FilterTurnTrace(IEntity entity, vector start, vector direction)
	{
		return entity != m_Truck && entity != m_Driver;
	}

	protected bool TryTurnStageSide(vector travel, float sideSign, out vector stage, out float score)
	{
		stage = vector.Zero;
		score = 0;
		if (!m_Truck || !GetGame() || !GetGame().GetWorld())
			return false;
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return false;
		vector current = m_Truck.GetOrigin();
		vector desired = current;
		desired[0] = desired[0] + travel[0] * 6.0 - travel[2] * 8.0 * sideSign;
		desired[2] = desired[2] + travel[2] * 6.0 + travel[0] * 8.0 * sideSign;
		vector roadPoint;
		if (!aiWorld.GetRoadNetworkManager().GetReachableWaypointInRoad(current, desired, 6.0, roadPoint))
			return false;
		float dx = roadPoint[0] - current[0];
		float dz = roadPoint[2] - current[2];
		float forward = dx * travel[0] + dz * travel[2];
		float lateral = (-dx * travel[2] + dz * travel[0]) * sideSign;
		if (forward < 3.5 || forward > 13.0 || lateral < 2.5 ||
			vector.Distance(current, roadPoint) < 6.0 ||
			vector.Distance(roadPoint, desired) > 6.5)
			return false;

		// An entity trace and vehicle footprint probe reject a blocked side.
		// AI road navigation still chooses the actual steering path.
		TraceParam trace = new TraceParam();
		vector traceStart = current;
		traceStart[1] = traceStart[1] + 2.0;
		vector traceEnd = roadPoint;
		traceEnd[1] = traceEnd[1] + 2.0;
		trace.Start = traceStart;
		trace.End = traceEnd;
		trace.Flags = TraceFlags.ENTS;
		trace.Exclude = m_Truck;
		if (GetGame().GetWorld().TraceMove(trace, FilterTurnTrace) < 0.98)
			return false;
		m_bTurnProbeOccupied = false;
		GetGame().GetWorld().QueryEntitiesBySphere(roadPoint, 4.5, ConsiderTurnObstacle);
		if (m_bTurnProbeOccupied)
			return false;
		stage = roadPoint;
		score = lateral;
		return true;
	}

	protected bool TryChooseTurnStage(vector travel, out vector stage, out string side)
	{
		stage = vector.Zero;
		side = "";
		vector leftPoint;
		vector rightPoint;
		float leftScore;
		float rightScore;
		bool left = TryTurnStageSide(travel, 1.0, leftPoint, leftScore);
		bool right = TryTurnStageSide(travel, -1.0, rightPoint, rightScore);
		if (!left && !right)
			return false;
		if (left && (!right || leftScore >= rightScore))
		{
			stage = leftPoint;
			side = "left";
		}
		else
		{
			stage = rightPoint;
			side = "right";
		}
		return true;
	}

	bool CF_BeginUnloadDeparture(vector waitingPoint)
	{
		if (!Replication.IsServer() || !CF_CanReleaseAtUnload() || !m_Session || !m_Truck ||
			(!m_bUnloadAnchorValid && !m_LastPlayerVehicle))
			return false;

		vector anchor = CF_GetUnloadAnchor();
		vector current = m_Truck.GetOrigin();
		vector travel;
		if (!TryGetOutboundTravelDirection(travel))
			return false;
		float behindCurrent = (current[0] - waitingPoint[0]) * travel[0] +
			(current[2] - waitingPoint[2]) * travel[2];
		if (behindCurrent < 12.0 || vector.Distance(waitingPoint, anchor) < CF_UNLOAD_BAY_CLEAR_DISTANCE)
			return false;
		vector stage;
		string side;
		bool stagedTurn = TryChooseTurnStage(travel, stage, side);

		m_vUnloadAnchor = anchor;
		m_bUnloadAnchorValid = true;
		m_vOutboundTravelDirection = travel;
		m_vUnloadWaitingPoint = waitingPoint;
		m_vUnloadTurnStage = stage;
		m_iUnloadTurnPhase = 1;
		m_fTurnStageSeconds = 0;
		m_vLastUnloadTruckPosition = m_Truck.GetOrigin();
		m_fUnloadSlotStillSeconds = 0;
		m_bUnloadSlotParkedLogged = false;
		m_bUnloadClearReported = false;
		m_fStateSeconds = 0;
		SetState(CF_UNLOAD_DEPARTING);
		vector firstWaypoint = stage;
		if (!stagedTurn)
		{
			// A single-lane road often has no lateral *road* point. Let the
			// vehicle AI plan its own turn to the already validated road slot.
			// The same finite departure timeout still applies.
			m_iUnloadTurnPhase = 2;
			firstWaypoint = waitingPoint;
		}
		if (!IssueMoveWaypoint(firstWaypoint))
		{
			Print("[ConvoyFollower] UNLOAD_RELEASE_FAILED: Unit " + m_iUnitNumber + " could not issue road waypoint");
			SetState(CF_ARRIVING);
			SetUnloadReleaseReady(true);
			return false;
		}
		SCR_AIWaypoint waypoint = SCR_AIWaypoint.Cast(m_Waypoint);
		if (waypoint)
		{
			if (stagedTurn)
				waypoint.SetCompletionRadius(CF_TURN_STAGE_RADIUS);
			else
				waypoint.SetCompletionRadius(CF_UNLOAD_SLOT_RADIUS);
		}
		if (stagedTurn)
			Print("[ConvoyFollower] UNLOAD_TURN_STAGE: Unit " + m_iUnitNumber + " taking clear " + side + " road stage before return slot");
		else
			Print("[ConvoyFollower] UNLOAD_DIRECT_ROUTE: Unit " + m_iUnitNumber + " routing to validated return slot without a lateral road stage");
		return true;
	}

	void CF_HoldForUnloadQueue()
	{
		if (!Replication.IsServer() || !m_Session || !CF_IsBoarded() ||
			m_iState == CF_UNLOAD_DEPARTING || m_iState == CF_UNLOAD_DEPARTED)
			return;
		if (m_iState == CF_UNLOAD_QUEUE)
			return;
		ClearWaypoints();
		SetState(CF_UNLOAD_QUEUE);
		Print("[ConvoyFollower] UNLOAD_QUEUE_HOLD: Unit " + m_iUnitNumber + " waiting seated for bay clearance");
	}

	void CF_ResumeFromUnloadQueue()
	{
		if (!Replication.IsServer() || !CF_IsBoarded())
			return;
		if (m_iState == CF_UNLOAD_DEPARTING)
		{
			ClearWaypoints();
			m_bUnloadClearReported = true;
			IEntity parkedTarget = m_UnloadAnchorVehicle;
			if (!parkedTarget)
				parkedTarget = GetTargetVehicle();
			if (parkedTarget)
				StartArriving(parkedTarget);
			else
				SetState(CF_PAUSED_ON_FOOT);
			Print("[ConvoyFollower] UNLOAD_RELEASE_ABORTED: Unit " + m_iUnitNumber + " returning toward parked target");
			return;
		}
		if (m_iState != CF_UNLOAD_QUEUE)
			return;
		m_fStateSeconds = 0;
		SetState(CF_WAITING_FOR_PREDECESSOR);
		Print("[ConvoyFollower] UNLOAD_QUEUE_RESUME: Unit " + m_iUnitNumber + " waiting for convoy target");
	}

	void CF_CompleteUnloadDeparture()
	{
		if (!Replication.IsServer() || m_iState != CF_UNLOAD_DEPARTING || !CF_IsAtUnloadWaitingPoint())
			return;
		// The session keeps this truck in its parked return roster. It remains
		// seated and receives no follow order until the lead vehicle crosses
		// the return line homeward.
		ClearWaypoints();
		m_Predecessor = null;
		m_bUnloadSequenceHold = false;
		SetState(CF_UNLOAD_DEPARTED);
		Print("[ConvoyFollower] RETURN_PARKED: former Unit " + m_iUnitNumber + " seated behind convoy tail");
	}

	bool CF_FindHomewardShiftPoint(out vector candidate, float distanceMeters)
	{
		candidate = vector.Zero;
		if (!Replication.IsServer() || !CF_IsAtUnloadWaitingPoint() ||
			distanceMeters < 12.0 || distanceMeters > 20.0)
			return false;
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return false;
		vector current = m_Truck.GetOrigin();
		vector goal = current;
		goal[0] = goal[0] + m_vUnloadHomeDirection[0] * distanceMeters;
		goal[2] = goal[2] + m_vUnloadHomeDirection[2] * distanceMeters;
		vector roadPoint;
		if (!aiWorld.GetRoadNetworkManager().GetReachableWaypointInRoad(current, goal, 6.0, roadPoint))
			return false;
		float progress = (roadPoint[0] - current[0]) * m_vUnloadHomeDirection[0] +
			(roadPoint[2] - current[2]) * m_vUnloadHomeDirection[2];
		if (vector.Distance(roadPoint, goal) > 6.0 || progress < distanceMeters - 1.0 ||
			vector.Distance(roadPoint, current) < distanceMeters - 1.0 ||
			vector.Distance(roadPoint, current) > distanceMeters + 6.0)
			return false;
		m_bTurnProbeOccupied = false;
		GetGame().GetWorld().QueryEntitiesBySphere(roadPoint, 5.5, ConsiderTurnObstacle);
		if (m_bTurnProbeOccupied)
			return false;
		candidate = roadPoint;
		return true;
	}

	bool CF_ShiftDepartedVehicle(vector newPoint)
	{
		if (!Replication.IsServer() || !CF_IsAtUnloadWaitingPoint())
			return false;
		vector current = m_Truck.GetOrigin();
		float shiftDistance = vector.Distance(current, newPoint);
		float progress = (newPoint[0] - current[0]) * m_vUnloadHomeDirection[0] +
			(newPoint[2] - current[2]) * m_vUnloadHomeDirection[2];
		if (shiftDistance < 10.0 || shiftDistance > 22.0 || progress < 10.0)
			return false;
		if (!IssueMoveWaypoint(newPoint))
		{
			Print("[ConvoyFollower] RETURN_SHIFT_FAILED: parked truck could not issue road move");
			return false;
		}
		m_vUnloadWaitingPoint = newPoint;
		m_vUnloadHomeDirection = Vector((newPoint[0] - current[0]) / shiftDistance, 0,
			(newPoint[2] - current[2]) / shiftDistance);
		m_vLastUnloadTruckPosition = current;
		m_fUnloadSlotStillSeconds = 0;
		m_bUnloadSlotParkedLogged = false;
		SCR_AIWaypoint waypoint = SCR_AIWaypoint.Cast(m_Waypoint);
		if (waypoint)
			waypoint.SetCompletionRadius(CF_UNLOAD_SLOT_RADIUS);
		Print("[ConvoyFollower] RETURN_SHIFT: parked truck moving one slot farther homeward");
		return true;
	}

	// Called by the session after it has merged the parked return roster into
	// the active chain. This truck has already turned and must not turn again.
	bool CF_CanBeginReturnFollow()
	{
		Resource movePrefab = Resource.Load(CF_MOVE_WAYPOINT);
		return m_Session && m_Group && m_iState == CF_UNLOAD_DEPARTED &&
			CF_IsAtUnloadWaitingPoint() && movePrefab.IsValid();
	}

	bool CF_CanAbortUnloadForReturn()
	{
		if (!m_Session || !m_Group || !CF_IsBoarded() ||
			m_iState == CF_UNLOAD_DEPARTING || m_iState == CF_UNLOAD_DEPARTED ||
			m_iState == CF_GETTING_OUT || m_iState == CF_IDLE)
			return false;
		Resource movePrefab = Resource.Load(CF_MOVE_WAYPOINT);
		vector facing;
		return movePrefab.IsValid() && TryGetVehicleFacing(m_Truck, facing);
	}

	bool CF_BeginReturnFollow(CF_ConvoySession session, IEntity currentLeader,
		CF_DriverControllerComponent predecessor, int unitNumber)
	{
		if (!Replication.IsServer() || !session || !currentLeader || unitNumber < 1 ||
			!CF_CanBeginReturnFollow())
			return false;
		m_Session = session;
		m_Leader = currentLeader;
		RememberOrderingPlayer(currentLeader);
		m_Predecessor = predecessor;
		m_iUnitNumber = unitNumber;
		Replication.BumpMe();
		CF_SetUnloadSequenceHold(false);
		m_bUnloadAnchorValid = false;
		m_LastPlayerVehicle = null;
		m_bUnloadSlotParkedLogged = false;
		m_fReturnMergeGraceSeconds = CF_RETURN_MERGE_GRACE_SECONDS;
		m_bSilentReturnFollow = true;
		SetState(CF_WAITING_FOR_PREDECESSOR);
		Print("[ConvoyFollower] RETURN_REJOIN: parked Unit " + unitNumber + " activated by homeward crossing");
		return true;
	}

	// An unreleased outbound truck remains seated while the explicit return
	// event converts it from unload/arrival duty into a staged homeward turn.
	// The normal lost-distance rule is suspended until that turn completes.
	bool CF_AbortUnloadForReturn(IEntity currentLeader)
	{
		if (!Replication.IsServer() || !currentLeader || !CF_CanAbortUnloadForReturn())
			return false;
		vector facing;
		if (!TryGetVehicleFacing(m_Truck, facing))
		{
			Print("[ConvoyFollower] RETURN_TURN_REJECTED: Unit " + m_iUnitNumber + " has no usable vehicle heading");
			return false;
		}
		m_vOutboundTravelDirection = facing;
		m_Leader = currentLeader;
		RememberOrderingPlayer(currentLeader);
		CF_SetUnloadSequenceHold(false);
		m_bUnloadAnchorValid = false;
		m_LastPlayerVehicle = null;
		m_bReturnTurnFailed = false;
		m_fReturnMergeGraceSeconds = CF_RETURN_MERGE_GRACE_SECONDS;
		m_iReturnTurnPhase = 0;
		m_fTurnStageSeconds = 0;
		ClearWaypoints();
		SetState(CF_RETURN_WAIT);
		Print("[ConvoyFollower] RETURN_TURN_WAIT: outbound Unit " + m_iUnitNumber + " waiting for homeward predecessor");
		return true;
	}

	protected void FailUnloadDeparture(string reason)
	{
		if (m_bUnloadClearReported)
			return;
		m_bUnloadClearReported = true;
		ClearWaypoints();
		SetState(CF_UNLOAD_QUEUE);
		Print("[ConvoyFollower] UNLOAD_TURN_BLOCKED: Unit " + m_iUnitNumber + " holding seated: " + reason);
		if (m_Session)
			m_Session.OnUnloadDepartureFailed(this);
	}

	protected void FailReturnTurn(string reason)
	{
		if (m_bReturnTurnFailed)
			return;
		m_bReturnTurnFailed = true;
		ClearWaypoints();
		SetState(CF_RETURN_BLOCKED);
		Print("[ConvoyFollower] RETURN_TURN_BLOCKED: Unit " + m_iUnitNumber + " holding seated: " + reason);
	}

	protected void SampleTruckRoute()
	{
		if (!m_Truck)
			return;
		vector current = m_Truck.GetOrigin();
		if (m_aRecentTruckPositions.IsEmpty() ||
			vector.Distance(current, m_aRecentTruckPositions[m_aRecentTruckPositions.Count() - 1]) >= CF_ROUTE_SAMPLE_DISTANCE)
		{
			m_aRecentTruckPositions.Insert(current);
			if (m_aRecentTruckPositions.Count() > CF_ROUTE_SAMPLE_LIMIT)
				m_aRecentTruckPositions.RemoveOrdered(0);
		}
	}

	protected void UpdateUnloadSlotStillness(float elapsed)
	{
		if (!m_Truck)
			return;
		vector current = m_Truck.GetOrigin();
		if (vector.Distance(current, m_vLastUnloadTruckPosition) < 1.0)
			m_fUnloadSlotStillSeconds += elapsed;
		else
			m_fUnloadSlotStillSeconds = 0;
		m_vLastUnloadTruckPosition = current;
	}

	void CF_ReportUnderFire()
	{
		if (!Replication.IsServer() || !CF_IsActiveConvoyMember())
			return;

		SendRadioCall(CF_RadioEvent.UNDER_FIRE);
	}

	bool CF_HasStallAhead()
	{
		if (!CF_IsMovementActive() || m_iStuckRetries > 0 || m_bOrderInversionLogged)
			return true;
		if (m_Predecessor)
			return m_Predecessor.CF_HasStallAhead();
		return false;
	}

	void CF_SetConvoyTarget(CF_DriverControllerComponent predecessor, int unitNumber)
	{
		if (!Replication.IsServer())
			return;

		bool changed = m_Predecessor != predecessor || m_iUnitNumber != unitNumber;
		// A newly promoted front unit may have joined after the player left
		// the lead vehicle. Inherit the former front unit's parked target.
		if (!predecessor && m_Predecessor && !m_LastPlayerVehicle)
			m_LastPlayerVehicle = m_Predecessor.m_LastPlayerVehicle;
		m_Predecessor = predecessor;
		m_iUnitNumber = unitNumber;
		if (changed)
			Replication.BumpMe();
		if (!changed || m_iState == CF_IDLE || m_iState == CF_BOARDING || m_iState == CF_REBOARDING || m_iState == CF_GETTING_OUT)
			return;

		// A roster change must not leave a waypoint aimed at the former leader.
		ClearWaypoints();
		m_LeadVehicle = null;
		m_fLostSeconds = 0;
		ResetRangeWarning();
		m_fStuckSeconds = 0;
		m_fTargetStillSeconds = 0;
		m_bStopSettleIssued = false;
		m_bArrivalCloseLogged = false;
		m_iStuckRetries = 0;
		m_bOrderInversionLogged = false;
		if (m_iState == CF_RETURN_WAIT || m_iState == CF_RETURN_TURNING || m_iState == CF_RETURN_BLOCKED)
		{
			m_iReturnTurnPhase = 0;
			m_bReturnTurnFailed = false;
			SetState(CF_RETURN_WAIT);
			Print("[ConvoyFollower] RETURN_RETARGET: Unit " + m_iUnitNumber + " waiting for new homeward predecessor");
			return;
		}
		if (m_iState != CF_PAUSED_ON_FOOT && m_iState != CF_UNLOAD_QUEUE)
			SetState(CF_WAITING_FOR_PREDECESSOR);
		Print("[ConvoyFollower] RETARGET: Unit " + m_iUnitNumber + " assigned new predecessor");
	}

	void CF_HoldForMissingPredecessor()
	{
		if (!Replication.IsServer() || m_iState == CF_IDLE || m_iState == CF_REBOARDING || m_iState == CF_GETTING_OUT)
			return;

		ClearWaypoints();
		m_LeadVehicle = null;
		m_fLostSeconds = 0;
		ResetRangeWarning();
		m_fStuckSeconds = 0;
		m_fTargetStillSeconds = 0;
		m_bStopSettleIssued = false;
		m_bArrivalCloseLogged = false;
		m_iStuckRetries = 0;
		m_bOrderInversionLogged = false;
		if (m_iState != CF_PAUSED_ON_FOOT && m_iState != CF_BOARDING && m_iState != CF_UNLOAD_QUEUE)
			SetState(CF_WAITING_FOR_PREDECESSOR);
		Print("[ConvoyFollower] LINK_HOLD: Unit " + m_iUnitNumber + " waiting for predecessor");
	}

	void CF_StandDownFromSession()
	{
		// The session already removed this unit from its roster. Suppress the
		// normal callback so replacement does not remove another member.
		m_Session = null;
		StandDown();
	}

	void AssignNearestTruck(IEntity user)
	{
		// Compatibility for the previous character action. The new actions call
		// Start/Add/Replace explicitly, never creating a second independent lead.
		if (!Replication.IsServer() || CF_ConvoySession.GetForPlayer(user))
			return;
		CF_ConvoySession.Start(user, this);
	}

	bool CF_BeginConvoyAssignment(CF_ConvoySession session, IEntity user)
	{
		if (!Replication.IsServer() || !session || !CanAssign(user))
			return false;
		bool wasOnFootFollower = IsOnFootFollower();

		ChimeraCharacter driver = ChimeraCharacter.Cast(GetOwner());
		ChimeraCharacter leader = ChimeraCharacter.Cast(user);
		if (!driver || !leader || driver.IsInVehicle())
		{
			Print("[ConvoyFollower] ASSIGN_REJECTED: driver or ordering player unavailable");
			return false;
		}

		AIControlComponent aiControl = driver.GetAIControlComponent();
		if (!aiControl || !aiControl.GetAIAgent())
		{
			Print("[ConvoyFollower] ASSIGN_REJECTED: driver has no AI agent");
			return false;
		}

		m_Group = SCR_AIGroup.Cast(aiControl.GetAIAgent().GetParentGroup());
		if (!m_Group || m_Group.GetAgentsCount() != 1)
		{
			Print("[ConvoyFollower] ASSIGN_REJECTED: driver needs its own one-member AI group");
			return false;
		}

		m_Truck = FindNearestEmptyTruck(driver);
		if (!m_Truck)
		{
			Print("[ConvoyFollower] ASSIGN_REJECTED: no empty ground vehicle within " + CF_ConvoySettings.Get().m_fTruckSearchRadius + " m");
			if (!wasOnFootFollower)
				m_Group = null;
			return false;
		}

		m_Driver = driver;
		m_Leader = leader;
		m_PassengerVehicle = null;
		m_bStopAfterPassengerExit = false;
		m_Session = session;
		m_Predecessor = null;
		m_iUnitNumber = 0;
		if (!wasOnFootFollower)
			RememberOrderingPlayer(leader);
		m_LeadVehicle = null;
		m_LastPlayerVehicle = null;
		m_UnloadAnchorVehicle = null;
		m_bUnloadSequenceHold = false;
		m_bUnloadAnchorValid = false;
		CF_SetSessionReleaseAllowed(true);
		m_bUnloadClearReported = false;
		m_bUnloadSlotParkedLogged = false;
		m_bReturnTurnFailed = false;
		m_bSilentReturnFollow = false;
		m_iUnloadTurnPhase = 0;
		m_iReturnTurnPhase = 0;
		m_iUnloadReboardState = 0;
		m_fUnloadSlotStillSeconds = 0;
		m_fArrivalTruckStillSeconds = 0;
		m_fReturnMergeGraceSeconds = 0;
		m_vOutboundTravelDirection = vector.Zero;
		m_vUnloadHomeDirection = vector.Zero;
		m_vReturnHomeDirection = vector.Zero;
		m_aRecentTruckPositions.Clear();
		m_aRecentTruckPositions.Insert(m_Truck.GetOrigin());
		m_iStuckRetries = 0;
		m_iReboardAttempts = 0;
		m_bPauseWasLost = false;
		m_fStateSeconds = 0;
		m_fLostSeconds = 0;
		ResetRangeWarning();
		m_fWaypointSeconds = 0;
		m_fStuckSeconds = 0;
		m_fTargetStillSeconds = 0;
		m_bStopSettleIssued = false;
		m_vLastTruckPosition = m_Truck.GetOrigin();

		if (wasOnFootFollower)
			ClearWaypoints();
		else
			ClearGroupWaypointsForNewOrder();
		if (!IssueBoardWaypoint())
		{
			Print("[ConvoyFollower] ASSIGN_FAILED: could not create boarding waypoint");
			ResetToIdle();
			return false;
		}

		SetState(CF_BOARDING);
		SetEventMask(m_Driver, EntityEvent.FRAME);
		Print("[ConvoyFollower] BOARDING: nearest empty ground vehicle assigned, driver seat requested");
		return true;
	}

	void FollowLead(IEntity user)
	{
		if (!Replication.IsServer() || m_iState != CF_WAITING_FOR_LEAD || user != m_Leader)
			return;

		IEntity targetVehicle = GetTargetVehicle();
		if (!targetVehicle || !CanTargetMove())
		{
			Print("[ConvoyFollower] WAITING_FOR_LEAD: convoy target not moving yet");
			return;
		}

		StartFollowing(targetVehicle, false, true);
	}

	void StandDown()
	{
		if (!Replication.IsServer() || !CanStandDown())
			return;

		NotifySessionUnavailable();
		Print("[ConvoyFollower] STAND_DOWN: stopping follow order");
		ClearWaypoints();
		m_bPauseWasLost = false;
		ResetRangeWarning();
		SetState(CF_GETTING_OUT);
		m_fStateSeconds = 0;

		ChimeraCharacter driver = ChimeraCharacter.Cast(m_Driver);
		if (!driver || !driver.IsInVehicle())
		{
			ResetToIdle();
			return;
		}

		if (!IssueGetOutWaypoint())
		{
			Print("[ConvoyFollower] GET_OUT_FAILED: driver remains in vehicle");
			ResetToIdle();
		}
	}

	protected Vehicle FindNearestEmptyTruck(IEntity driver)
	{
		float searchRadius = CF_ConvoySettings.Get().m_fTruckSearchRadius;
		m_Candidate = null;
		m_fCandidateDistance = searchRadius + 1.0;
		if (!driver)
			return null;

		GetGame().GetWorld().QueryEntitiesBySphere(driver.GetOrigin(), searchRadius, ConsiderTruck);
		return m_Candidate;
	}

	protected bool ConsiderTruck(IEntity entity)
	{
		Vehicle vehicle = Vehicle.Cast(entity);
		if (!vehicle)
			return true;

		// Wheeled vehicles, including the vanilla supply trucks. No helicopters.
		if (!vehicle.FindComponent(CarControllerComponent))
			return true;

		BaseCompartmentManagerComponent compartments = BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(BaseCompartmentManagerComponent));
		if (!compartments)
			return true;

		ref array<BaseCompartmentSlot> slots = {};
		compartments.GetCompartments(slots);
		bool pilotFree = false;
		foreach (BaseCompartmentSlot slot : slots)
		{
			if (slot && slot.IsPiloting() && !slot.IsOccupied() && !slot.IsReserved() && !slot.IsGetInLockedFor(GetOwner()))
			{
				pilotFree = true;
				break;
			}
		}

		if (!pilotFree)
			return true;

		float distance = vector.Distance(GetOwner().GetOrigin(), vehicle.GetOrigin());
		if (distance < m_fCandidateDistance)
		{
			m_fCandidateDistance = distance;
			m_Candidate = vehicle;
		}

		return true;
	}

	protected bool IssueBoardWaypoint()
	{
		if (!m_Truck || !m_Group)
			return false;

		Resource prefab = Resource.Load(CF_BOARD_WAYPOINT);
		if (!prefab.IsValid())
			return false;

		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = m_Truck.GetOrigin();
		SCR_BoardingEntityWaypoint waypoint = SCR_BoardingEntityWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(prefab, null, params));
		if (!waypoint)
			return false;

		waypoint.SetEntity(m_Truck);
		waypoint.SetAllowance(true, false, false);
		waypoint.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_PLAYER);
		waypoint.SetCompletionRadius(8.0);
		m_Waypoint = waypoint;
		m_Group.AddWaypoint(waypoint);
		m_fWaypointSeconds = 0;
		return true;
	}

	protected bool TryReboardAssignedTruck()
	{
		if (m_iReboardAttempts >= CF_ConvoySettings.Get().m_iReboardMaxAttempts)
			return false;

		ClearWaypoints();
		m_iReboardAttempts++;
		m_fStateSeconds = 0;
		if (!IssueBoardWaypoint())
			return false;

		Print("[ConvoyFollower] REBOARD_ATTEMPT: Unit " + m_iUnitNumber + " trying assigned driver seat, attempt " + m_iReboardAttempts);
		return true;
	}

	protected void BeginUnexpectedReboard()
	{
		m_iUnloadReboardState = 0;
		if (m_iState == CF_UNLOAD_QUEUE || m_iState == CF_UNLOAD_DEPARTING || m_iState == CF_UNLOAD_DEPARTED ||
			m_iState == CF_RETURN_WAIT || m_iState == CF_RETURN_TURNING || m_iState == CF_RETURN_BLOCKED)
			m_iUnloadReboardState = m_iState;
		m_iReboardAttempts = 0;
		SetState(CF_REBOARDING);
		if (TryReboardAssignedTruck())
			return;

		Print("[ConvoyFollower] REBOARD_FAILED: Unit " + m_iUnitNumber + " could not issue boarding order");
		StandDown();
	}

	protected bool IsDriverDestroyed(ChimeraCharacter driver)
	{
		if (!driver)
			return true;

		SCR_DamageManagerComponent damage = driver.GetDamageManager();
		return damage && damage.IsDestroyed();
	}

	protected bool IsAssignedTruckDestroyed()
	{
		BaseVehicle baseTruck = m_Truck;
		SCR_DamageManagerComponent damage;
		if (baseTruck)
			damage = baseTruck.GetDamageManager();
		else
			damage = SCR_DamageManagerComponent.GetDamageManager(m_Truck);
		return damage && damage.IsDestroyed();
	}

	protected bool IssueOnFootFollowWaypoint()
	{
		if (!m_Group || !m_Leader)
			return false;

		Resource prefab = Resource.Load(CF_FOLLOW_WAYPOINT);
		if (!prefab.IsValid())
			return false;

		ClearWaypoints();
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = m_Leader.GetOrigin();
		SCR_EntityWaypoint waypoint = SCR_EntityWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(prefab, null, params));
		if (!waypoint)
			return false;

		waypoint.SetEntity(m_Leader);
		waypoint.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_PLAYER);
		waypoint.SetCompletionRadius(CF_ON_FOOT_GAP);
		m_Waypoint = waypoint;
		m_Group.AddWaypoint(waypoint);
		m_fWaypointSeconds = 0;
		return true;
	}

	protected bool HasOwnWaypointInGroup()
	{
		if (!m_Group || !m_Waypoint)
			return false;

		if (m_Group.GetCurrentWaypoint() == m_Waypoint)
			return true;

		ref array<AIWaypoint> waypoints = {};
		m_Group.GetWaypoints(waypoints);
		foreach (AIWaypoint waypoint : waypoints)
		{
			if (waypoint == m_Waypoint)
				return true;
		}
		return false;
	}

	protected bool HasFreeCargoSeat(IEntity vehicle)
	{
		if (!vehicle || !m_Driver)
			return false;

		BaseCompartmentManagerComponent compartments = BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(BaseCompartmentManagerComponent));
		if (!compartments)
			return false;

		ref array<BaseCompartmentSlot> slots = {};
		compartments.GetCompartments(slots);
		foreach (BaseCompartmentSlot slot : slots)
		{
			if (slot && slot.GetType() == ECompartmentType.CARGO && !slot.IsOccupied() &&
				!slot.IsReserved() && !slot.IsGetInLockedFor(m_Driver))
				return true;
		}
		return false;
	}

	protected bool IssuePassengerBoardWaypoint(IEntity vehicle)
	{
		if (!m_Group || !vehicle || !HasFreeCargoSeat(vehicle))
			return false;

		Resource prefab = Resource.Load(CF_BOARD_WAYPOINT);
		if (!prefab.IsValid())
			return false;

		ClearWaypoints();
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = vehicle.GetOrigin();
		SCR_BoardingEntityWaypoint waypoint = SCR_BoardingEntityWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(prefab, null, params));
		if (!waypoint)
			return false;

		waypoint.SetEntity(vehicle);
		waypoint.SetAllowance(false, false, true);
		waypoint.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_PLAYER);
		waypoint.SetCompletionRadius(8.0);
		m_Waypoint = waypoint;
		m_Group.AddWaypoint(waypoint);
		return true;
	}

	protected bool IssueMoveWaypoint(vector destination)
	{
		if (!m_Group)
			return false;

		Resource prefab = Resource.Load(CF_MOVE_WAYPOINT);
		if (!prefab.IsValid())
			return false;

		ClearWaypoints();
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = destination;
		SCR_AIWaypoint waypoint = SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(prefab, null, params));
		if (!waypoint)
			return false;

		// A driving order must outrank the AI's autonomous combat dismount.
		// The on-foot staging order keeps its normal player priority.
		if (m_iState == CF_ON_FOOT_FOLLOW)
			waypoint.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_PLAYER);
		else
			waypoint.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_GAMEMASTER);
		if (m_iState == CF_ON_FOOT_FOLLOW)
			waypoint.SetCompletionRadius(CF_ON_FOOT_GAP);
		else
			waypoint.SetCompletionRadius(CF_ConvoySettings.Get().m_fMovingGap);
		m_Waypoint = waypoint;
		m_Group.AddWaypoint(waypoint);
		m_vLastWaypointPosition = destination;
		m_fWaypointSeconds = 0;
		return true;
	}

	// Keep one active MOVE waypoint so the vehicle path does not restart every few seconds.
	protected bool MoveWaypoint(vector destination)
	{
		// Completed waypoints can leave the group while the leader is stopped nearby.
		// In that case create a fresh order; otherwise only move the active one.
		if (!HasOwnWaypointInGroup())
			return IssueMoveWaypoint(destination);

		m_Waypoint.SetOrigin(destination);
		m_vLastWaypointPosition = destination;
		m_fWaypointSeconds = 0;
		return true;
	}

	// A MOVE waypoint can complete while its target stops just short of the
	// last 8 m refresh threshold. Give it one final, road-navigated approach
	// with a modestly tighter center gap; do not continuously reissue orders.
	protected void SettleBehindStoppedTarget(float elapsed, float separation)
	{
		if (!m_LeadVehicle)
			return;

		vector targetPosition = m_LeadVehicle.GetOrigin();
		if (vector.Distance(m_vLastTargetPosition, targetPosition) >= CF_STOP_DETECT_DISTANCE)
		{
			m_vLastTargetPosition = targetPosition;
			m_fTargetStillSeconds = 0;
			if (m_bStopSettleIssued)
			{
				SCR_AIWaypoint activeWaypoint = SCR_AIWaypoint.Cast(m_Waypoint);
				if (activeWaypoint && HasOwnWaypointInGroup())
					activeWaypoint.SetCompletionRadius(CF_ConvoySettings.Get().m_fMovingGap);
				m_bStopSettleIssued = false;
			}
			return;
		}

		m_fTargetStillSeconds += elapsed;
		float settleDistance = CF_STOP_SETTLE_DISTANCE;
		if (m_iState == CF_ARRIVING)
			settleDistance = CF_ConvoySettings.Get().m_fStoppedGap + 1.0;
		else if (settleDistance < CF_ConvoySettings.Get().m_fStoppedGap + 4.0)
			settleDistance = CF_ConvoySettings.Get().m_fStoppedGap + 4.0;
		if (m_fTargetStillSeconds < CF_STOP_DETECT_SECONDS || m_bStopSettleIssued ||
			separation <= settleDistance || m_fWaypointSeconds < CF_WAYPOINT_REFRESH_SECONDS)
			return;

		// Mark this stationary episode even if a new waypoint cannot be made;
		// a persistent obstruction must not create an order every frame.
		m_bStopSettleIssued = true;
		if (!MoveWaypoint(targetPosition))
		{
			Print("[ConvoyFollower] STOP_SETTLE_FAILED: Unit " + m_iUnitNumber + " could not refresh final waypoint");
			return;
		}

		SCR_AIWaypoint waypoint = SCR_AIWaypoint.Cast(m_Waypoint);
		if (waypoint)
			waypoint.SetCompletionRadius(CF_ConvoySettings.Get().m_fStoppedGap);
		Print("[ConvoyFollower] STOP_SETTLE: Unit " + m_iUnitNumber + " closing behind stopped predecessor");
	}

	protected bool IssueGetOutWaypointFor(IEntity vehicle)
	{
		if (!m_Group || !vehicle)
			return false;

		Resource prefab = Resource.Load(CF_GET_OUT_WAYPOINT);
		if (!prefab.IsValid())
			return false;

		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = vehicle.GetOrigin();
		ChimeraCharacter driver = ChimeraCharacter.Cast(m_Driver);
		if (driver)
		{
			CompartmentAccessComponent compartment = driver.GetCompartmentAccessComponent();
			if (compartment)
			{
				IEntity occupiedVehicle = compartment.GetVehicleIn(driver);
				if (occupiedVehicle)
					params.Transform[3] = occupiedVehicle.GetOrigin();
			}
		}
		SCR_AIWaypoint waypoint = SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(prefab, null, params));
		if (!waypoint)
			return false;

		waypoint.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_PLAYER);
		m_Waypoint = waypoint;
		m_Group.AddWaypoint(waypoint);
		return true;
	}

	protected bool IssueGetOutWaypoint()
	{
		return IssueGetOutWaypointFor(m_Truck);
	}

	protected IEntity GetLeadVehicle()
	{
		ChimeraCharacter leader = ChimeraCharacter.Cast(m_Leader);
		if (!leader || !leader.IsInVehicle())
			return null;

		CompartmentAccessComponent compartment = leader.GetCompartmentAccessComponent();
		if (!compartment)
			return null;

		IEntity vehicle = compartment.GetVehicleIn(leader);
		if (vehicle == m_Truck)
			return null;

		if (vehicle)
			m_LastPlayerVehicle = vehicle;
		return vehicle;
	}

	protected IEntity GetTargetVehicle()
	{
		// Every unit remembers the player's last vehicle. If the first unit
		// is lost while the player is outside, its successor can lead the
		// remaining chain toward the same parked vehicle.
		IEntity playerVehicle = GetLeadVehicle();
		if (m_Predecessor)
			return m_Predecessor.CF_GetAssignedVehicle();
		if (m_bUnloadSequenceHold)
			return m_UnloadAnchorVehicle;
		if (playerVehicle)
			return playerVehicle;

		ChimeraCharacter leader = ChimeraCharacter.Cast(m_Leader);
		if (leader && !leader.IsInVehicle())
			return m_LastPlayerVehicle;
		return null;
	}

	protected bool CanTargetMove()
	{
		// A parked, arriving predecessor remains an active target, so later
		// trucks keep closing their gaps in order.
		if (m_Predecessor)
			return m_Predecessor.CF_IsMovementActive() && m_Predecessor.CF_GetAssignedVehicle() != null;
		return GetTargetVehicle() != null;
	}

	protected void StartFollowing(IEntity targetVehicle, bool rejoined, bool emitRadio)
	{
		m_LeadVehicle = targetVehicle;
		m_vLastTargetPosition = targetVehicle.GetOrigin();
		m_vArrivalAnchorPosition = m_vLastTargetPosition;
		m_fTargetStillSeconds = 0;
		m_bStopSettleIssued = false;
		m_bArrivalCloseLogged = false;
		m_bPauseWasLost = false;
		m_fStateSeconds = 0;
		m_fLostSeconds = 0;
		ResetRangeWarning();
		m_fStuckSeconds = 0;
		m_iStuckRetries = 0;
		m_vLastTruckPosition = m_Truck.GetOrigin();
		m_vLastStallTargetPosition = targetVehicle.GetOrigin();
		m_bOrderInversionLogged = false;
		SetState(CF_FOLLOWING);
		ObserveConvoyOrder(targetVehicle, vector.Distance(m_Truck.GetOrigin(), targetVehicle.GetOrigin()));
		if (m_bOrderInversionLogged)
		{
			Print("[ConvoyFollower] FOLLOW_HELD_OUT_OF_ORDER: Unit " + m_iUnitNumber + " waiting for its predecessor to pass");
			return;
		}

		if (!IssueMoveWaypoint(targetVehicle.GetOrigin()))
		{
			Print("[ConvoyFollower] FOLLOW_FAILED: could not create move waypoint");
			StandDown();
			return;
		}

		Print("[ConvoyFollower] FOLLOWING: Unit " + m_iUnitNumber + " moving toward predecessor vehicle");
		if (!emitRadio)
			return;
		if (rejoined)
			SendRadioCall(CF_RadioEvent.REJOINED);
		else
			SendRadioCall(CF_RadioEvent.FOLLOWING);
	}

	protected void StartArriving(IEntity targetVehicle)
	{
		m_LeadVehicle = targetVehicle;
		m_vLastTargetPosition = targetVehicle.GetOrigin();
		m_vArrivalAnchorPosition = m_vLastTargetPosition;
		m_fTargetStillSeconds = 0;
		m_bStopSettleIssued = false;
		m_bArrivalCloseLogged = false;
		m_fStateSeconds = 0;
		m_fLostSeconds = 0;
		m_fStuckSeconds = 0;
		m_iStuckRetries = 0;
		m_vLastTruckPosition = m_Truck.GetOrigin();
		m_vLastStallTargetPosition = targetVehicle.GetOrigin();
		m_bOrderInversionLogged = false;
		m_vArrivalLastTruckPosition = m_Truck.GetOrigin();
		m_fArrivalTruckStillSeconds = 0;
		SetState(CF_ARRIVING);
		ObserveConvoyOrder(targetVehicle, vector.Distance(m_Truck.GetOrigin(), targetVehicle.GetOrigin()));
		if (m_bOrderInversionLogged)
		{
			Print("[ConvoyFollower] ARRIVAL_HELD_OUT_OF_ORDER: Unit " + m_iUnitNumber + " waiting for its predecessor to pass");
			return;
		}

		// Keep the normal road-routed MOVE order. Once the predecessor has
		// stopped, SettleBehindStoppedTarget tightens its completion radius.
		if (!MoveWaypoint(targetVehicle.GetOrigin()))
		{
			Print("[ConvoyFollower] ARRIVAL_FAILED: Unit " + m_iUnitNumber + " could not create approach waypoint");
			StandDown();
			return;
		}
		SCR_AIWaypoint waypoint = SCR_AIWaypoint.Cast(m_Waypoint);
		if (waypoint)
			waypoint.SetCompletionRadius(CF_ConvoySettings.Get().m_fMovingGap);

		Print("[ConvoyFollower] ARRIVAL_APPROACH: Unit " + m_iUnitNumber + " closing behind parked convoy target");
	}

	protected void ClearWaypoints()
	{
		if (!m_Group || !m_Waypoint)
			return;

		ref array<AIWaypoint> waypoints = {};
		m_Group.GetWaypoints(waypoints);
		foreach (AIWaypoint waypoint : waypoints)
		{
			if (waypoint == m_Waypoint)
			{
				m_Group.RemoveWaypoint(waypoint);
				break;
			}
		}

		m_Waypoint = null;
	}

	protected void ClearGroupWaypointsForNewOrder()
	{
		// Commanding a one-member driver group replaces its prior GM order.
		// Later updates remove only this controller's waypoint, never orders
		// issued to another group's AI.
		if (!m_Group || m_Group.GetAgentsCount() != 1)
			return;

		ref array<AIWaypoint> waypoints = {};
		m_Group.GetWaypoints(waypoints);
		foreach (AIWaypoint waypoint : waypoints)
			m_Group.RemoveWaypoint(waypoint);
		m_Waypoint = null;
	}

	protected void ResetToIdle()
	{
		if (m_Driver)
			ClearEventMask(m_Driver, EntityEvent.FRAME);

		NotifySessionUnavailable();
		SetState(CF_IDLE);
		m_bOrderInversionLogged = false;
		m_Driver = null;
		m_Leader = null;
		m_Predecessor = null;
		m_LeadVehicle = null;
		m_LastPlayerVehicle = null;
		m_UnloadAnchorVehicle = null;
		m_bUnloadSequenceHold = false;
		m_bUnloadAnchorValid = false;
		CF_SetSessionReleaseAllowed(true);
		m_bUnloadClearReported = false;
		m_bUnloadSlotParkedLogged = false;
		m_bReturnTurnFailed = false;
		m_bSilentReturnFollow = false;
		m_iUnloadTurnPhase = 0;
		m_iReturnTurnPhase = 0;
		m_iUnloadReboardState = 0;
		m_fUnloadSlotStillSeconds = 0;
		m_fArrivalTruckStillSeconds = 0;
		m_fReturnMergeGraceSeconds = 0;
		m_vOutboundTravelDirection = vector.Zero;
		m_vUnloadHomeDirection = vector.Zero;
		m_vReturnHomeDirection = vector.Zero;
		m_aRecentTruckPositions.Clear();
		SetOrderingPlayerId(0);
		m_iUnitNumber = 0;
		m_bPauseWasLost = false;
		ResetRangeWarning();
		m_Truck = null;
		m_iReboardAttempts = 0;
		m_PassengerVehicle = null;
		m_bStopAfterPassengerExit = false;
		m_Group = null;
		m_Waypoint = null;
		m_fPollAccumulator = 0;
		m_fStateSeconds = 0;
		m_fTargetStillSeconds = 0;
		m_bStopSettleIssued = false;
		m_bArrivalCloseLogged = false;
		Print("[ConvoyFollower] IDLE: driver ready for a new order");
	}

	protected bool BeginPassengerExit(ChimeraCharacter driver)
	{
		if (!driver || !driver.IsInVehicle())
			return false;

		CompartmentAccessComponent compartment = driver.GetCompartmentAccessComponent();
		if (!compartment)
			return false;

		IEntity occupiedVehicle = compartment.GetVehicleIn(driver);
		if (!occupiedVehicle)
			return false;

		ClearWaypoints();
		if (!IssueGetOutWaypointFor(occupiedVehicle))
			return false;

		m_PassengerVehicle = occupiedVehicle;
		m_fStateSeconds = 0;
		SetState(CF_ON_FOOT_DISEMBARKING);
		Print("[ConvoyFollower] FOOT_PASSENGER_EXITING: driver leaving player's vehicle");
		return true;
	}

	protected void ResumeOnFootFollow()
	{
		m_PassengerVehicle = null;
		m_fStateSeconds = 0;
		m_vLastFootPosition = m_Driver.GetOrigin();
		m_fWaypointSeconds = 0;
		SetState(CF_ON_FOOT_FOLLOW);
		if (!IssueOnFootFollowWaypoint())
		{
			Print("[ConvoyFollower] FOOT_FOLLOW_FAILED: could not resume follow waypoint");
			ResetToIdle();
			return;
		}
		Print("[ConvoyFollower] FOOT_FOLLOW_RESUMED: driver following player on foot");
	}

	protected void TryPassengerBoard(IEntity playerVehicle)
	{
		m_PassengerVehicle = playerVehicle;
		m_fStateSeconds = 0;
		if (!IssuePassengerBoardWaypoint(playerVehicle))
		{
			ClearWaypoints();
			SetState(CF_ON_FOOT_HOLD);
			Print("[ConvoyFollower] FOOT_PASSENGER_HOLD: no free cargo seat in player's vehicle");
			return;
		}

		SetState(CF_ON_FOOT_BOARDING_PASSENGER);
		Print("[ConvoyFollower] FOOT_PASSENGER_BOARDING: cargo seat requested in player's vehicle");
	}

	protected void UpdateOnFootFollow(ChimeraCharacter driver)
	{
		ChimeraCharacter player = ChimeraCharacter.Cast(m_Leader);
		if (!player || !CF_IsOwnedBy(player))
		{
			Print("[ConvoyFollower] FOOT_FOLLOW_ENDED: player or staged driver unavailable");
			ClearWaypoints();
			ResetToIdle();
			return;
		}

		IEntity playerVehicle = GetLeadVehicle();
		if (m_iState == CF_ON_FOOT_DISEMBARKING)
		{
			if (!driver.IsInVehicle())
			{
				ClearWaypoints();
				if (m_bStopAfterPassengerExit)
				{
					ResetToIdle();
					Print("[ConvoyFollower] FOOT_FOLLOW_STOPPED: staged driver is idle");
				}
				else if (playerVehicle)
					TryPassengerBoard(playerVehicle);
				else
					ResumeOnFootFollow();
			}
			else if (m_fStateSeconds >= CF_PASSENGER_EXIT_TIMEOUT_SECONDS)
			{
				Print("[ConvoyFollower] FOOT_PASSENGER_EXIT_TIMEOUT: driver did not leave vehicle");
				ClearWaypoints();
				ResetToIdle();
			}
			return;
		}

		if (driver.IsInVehicle())
		{
			CompartmentAccessComponent compartment = driver.GetCompartmentAccessComponent();
			IEntity occupiedVehicle;
			BaseCompartmentSlot slot;
			if (compartment)
			{
				occupiedVehicle = compartment.GetVehicleIn(driver);
				slot = compartment.GetCompartment();
			}

			if (playerVehicle && occupiedVehicle == playerVehicle && slot &&
				slot.GetType() == ECompartmentType.CARGO && !slot.IsPiloting())
			{
				if (m_iState != CF_ON_FOOT_RIDING_PASSENGER)
				{
					ClearWaypoints();
					m_PassengerVehicle = occupiedVehicle;
					m_fStateSeconds = 0;
					SetState(CF_ON_FOOT_RIDING_PASSENGER);
					Print("[ConvoyFollower] FOOT_PASSENGER_RIDING: cargo seat occupied");
				}
				return;
			}

			if (!BeginPassengerExit(driver))
			{
				Print("[ConvoyFollower] FOOT_PASSENGER_EXIT_FAILED: wrong seat or player left vehicle");
				ResetToIdle();
			}
			return;
		}

		if (playerVehicle)
		{
			if (m_iState == CF_ON_FOOT_BOARDING_PASSENGER)
			{
				if (m_PassengerVehicle == playerVehicle && m_fStateSeconds < CF_PASSENGER_BOARD_TIMEOUT_SECONDS)
					return;

				if (m_PassengerVehicle == playerVehicle)
				{
					ClearWaypoints();
					SetState(CF_ON_FOOT_HOLD);
					Print("[ConvoyFollower] FOOT_PASSENGER_TIMEOUT: driver could not reach a cargo seat");
					return;
				}
			}

			// One attempt per vehicle. A full vehicle leaves the driver on foot;
			// switching to a different vehicle allows a new cargo-seat attempt.
			if (m_iState != CF_ON_FOOT_HOLD || m_PassengerVehicle != playerVehicle)
				TryPassengerBoard(playerVehicle);
			return;
		}

		if (m_iState != CF_ON_FOOT_FOLLOW)
		{
			ClearWaypoints();
			ResumeOnFootFollow();
			return;
		}

		// A MOVE waypoint can complete while the player is still within
		// interaction distance. FOLLOW targets the moving player directly;
		// recreate it if the engine removes the waypoint after completion.
		if (vector.Distance(driver.GetOrigin(), player.GetOrigin()) >= CF_ON_FOOT_REORDER_DISTANCE &&
			!HasOwnWaypointInGroup() && !IssueOnFootFollowWaypoint())
		{
			Print("[ConvoyFollower] FOOT_FOLLOW_FAILED: could not restore follow waypoint");
			ResetToIdle();
			return;
		}

		if (m_fWaypointSeconds >= CF_ON_FOOT_STALL_CHECK_SECONDS)
		{
			float separation = vector.Distance(driver.GetOrigin(), player.GetOrigin());
			float movement = vector.Distance(m_vLastFootPosition, driver.GetOrigin());
			if (separation >= CF_ON_FOOT_STALL_DISTANCE && movement < 1.0)
			{
				Print("[ConvoyFollower] FOOT_FOLLOW_RETRY: driver stationary while player is " + separation + " m away");
				if (!IssueOnFootFollowWaypoint())
				{
					ResetToIdle();
					return;
				}
			}
			m_vLastFootPosition = driver.GetOrigin();
			m_fWaypointSeconds = 0;
		}
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!Replication.IsServer() || m_iState == CF_IDLE)
			return;

		m_fPollAccumulator += timeSlice;
		if (m_fPollAccumulator < 1.0)
			return;

		float elapsed = m_fPollAccumulator;
		m_fPollAccumulator = 0;
		m_fStateSeconds += elapsed;
		m_fWaypointSeconds += elapsed;

		ChimeraCharacter driver = ChimeraCharacter.Cast(m_Driver);
		if (!driver || !m_Group)
		{
			Print("[ConvoyFollower] ABORTED: driver or AI group disappeared");
			ClearWaypoints();
			ResetToIdle();
			return;
		}

		if (IsOnFootFollower())
		{
			UpdateOnFootFollow(driver);
			return;
		}

		if (!m_Truck)
		{
			Print("[ConvoyFollower] ABORTED: assigned vehicle disappeared");
			ClearWaypoints();
			ResetToIdle();
			return;
		}
		// Cache the lead vehicle even while this unit is still boarding. A
		// later player exit then has a stable parked target to approach.
		GetLeadVehicle();

		if (m_iState == CF_BOARDING)
		{
			if (driver.IsInVehicle())
			{
				if (!CF_IsBoarded())
				{
					Print("[ConvoyFollower] BOARD_FAILED: driver entered wrong seat or vehicle");
					StandDown();
					return;
				}

				Print("[ConvoyFollower] BOARDED: driver seat occupied");
				SetState(CF_WAITING_FOR_LEAD);
				m_fStateSeconds = 0;
				if (m_Session)
					m_Session.OnDriverBoarded(this);
				if (m_iState != CF_WAITING_FOR_LEAD && m_iState != CF_WAITING_FOR_PREDECESSOR)
					return;
				IEntity targetVehicle = GetTargetVehicle();
				if (targetVehicle && CanTargetMove())
				{
					ChimeraCharacter orderingPlayer = ChimeraCharacter.Cast(m_Leader);
					if (orderingPlayer && (!orderingPlayer.IsInVehicle() || m_bUnloadSequenceHold))
						StartArriving(targetVehicle);
					else
						StartFollowing(targetVehicle, false, true);
				}
				else
				{
					Print("[ConvoyFollower] WAITING_FOR_LEAD: Unit " + m_iUnitNumber + " ready for predecessor");
					SendRadioCall(CF_RadioEvent.READY);
				}
				return;
			}

			if (m_fStateSeconds >= 60.0)
			{
				Print("[ConvoyFollower] BOARD_TIMEOUT: no driver seat after 60 s");
				ClearWaypoints();
				ResetToIdle();
			}
			return;
		}

		if (m_iState == CF_REBOARDING)
		{
			if (IsDriverDestroyed(driver) || IsAssignedTruckDestroyed())
			{
				Print("[ConvoyFollower] REBOARD_CANCELLED: Unit " + m_iUnitNumber + " or assigned truck destroyed");
				StandDown();
				return;
			}

			if (driver.IsInVehicle())
			{
				if (!CF_IsBoarded())
				{
					Print("[ConvoyFollower] REBOARD_FAILED: Unit " + m_iUnitNumber + " entered the wrong seat or vehicle");
					StandDown();
					return;
				}

				ClearWaypoints();
				Print("[ConvoyFollower] REBOARDED: Unit " + m_iUnitNumber + " returned to assigned driver seat");
				m_iReboardAttempts = 0;
				m_fStateSeconds = 0;
				if (m_iUnloadReboardState == CF_UNLOAD_QUEUE)
				{
					m_iUnloadReboardState = 0;
					SetState(CF_UNLOAD_QUEUE);
					return;
				}
				if (m_iUnloadReboardState == CF_UNLOAD_DEPARTING || m_iUnloadReboardState == CF_UNLOAD_DEPARTED)
				{
					int resumeState = m_iUnloadReboardState;
					m_iUnloadReboardState = 0;
					SetState(resumeState);
					m_vLastUnloadTruckPosition = m_Truck.GetOrigin();
					m_fUnloadSlotStillSeconds = 0;
					vector reboardDestination = m_vUnloadWaitingPoint;
					if (resumeState == CF_UNLOAD_DEPARTING && m_iUnloadTurnPhase == 1)
						reboardDestination = m_vUnloadTurnStage;
					if (vector.Distance(m_Truck.GetOrigin(), reboardDestination) > CF_UNLOAD_SLOT_RADIUS + 2.0)
					{
						if (!IssueMoveWaypoint(reboardDestination))
							Print("[ConvoyFollower] UNLOAD_REBOARD_MOVE_FAILED: Unit " + m_iUnitNumber);
						else
						{
							SCR_AIWaypoint waypoint = SCR_AIWaypoint.Cast(m_Waypoint);
							if (waypoint)
								waypoint.SetCompletionRadius(CF_TURN_STAGE_RADIUS);
						}
					}
					return;
				}
				if (m_iUnloadReboardState == CF_RETURN_WAIT || m_iUnloadReboardState == CF_RETURN_TURNING ||
					m_iUnloadReboardState == CF_RETURN_BLOCKED)
				{
					int returnState = m_iUnloadReboardState;
					m_iUnloadReboardState = 0;
					SetState(returnState);
					m_fStateSeconds = 0;
					if (returnState == CF_RETURN_TURNING)
					{
						vector turnDestination = m_vReturnTurnStage;
						if (m_iReturnTurnPhase == 2 && GetTargetVehicle())
							turnDestination = GetTargetVehicle().GetOrigin();
						if (!IssueMoveWaypoint(turnDestination))
							FailReturnTurn("could not restore turn after reboarding");
					}
					return;
				}
				SetState(CF_WAITING_FOR_PREDECESSOR);
				ChimeraCharacter orderingPlayer = ChimeraCharacter.Cast(m_Leader);
				IEntity targetVehicle = GetTargetVehicle();
				if (orderingPlayer && targetVehicle && CanTargetMove())
				{
					if (orderingPlayer.IsInVehicle() && !m_bUnloadSequenceHold)
						StartFollowing(targetVehicle, true, true);
					else
						StartArriving(targetVehicle);
				}
				return;
			}

			if (m_fStateSeconds >= CF_ConvoySettings.Get().m_fReboardRetrySeconds)
			{
				if (!TryReboardAssignedTruck())
				{
					Print("[ConvoyFollower] REBOARD_TERMINAL: Unit " + m_iUnitNumber + " could not regain driver seat; convoy rewiring");
					StandDown();
				}
			}
			return;
		}

		if (m_iState == CF_WAITING_FOR_LEAD || m_iState == CF_WAITING_FOR_PREDECESSOR)
		{
			if (!CF_IsBoarded())
			{
				if (driver.IsInVehicle() || IsDriverDestroyed(driver) || IsAssignedTruckDestroyed())
					StandDown();
				else
					BeginUnexpectedReboard();
				return;
			}

			ChimeraCharacter orderingPlayer = ChimeraCharacter.Cast(m_Leader);
			IEntity targetVehicle = GetTargetVehicle();
			if (orderingPlayer && targetVehicle && CanTargetMove())
			{
				if (orderingPlayer.IsInVehicle() && !m_bUnloadSequenceHold)
				{
					bool emitRadio = !m_bSilentReturnFollow;
					m_bSilentReturnFollow = false;
					StartFollowing(targetVehicle, false, emitRadio);
				}
				else
					StartArriving(targetVehicle);
			}
			return;
		}

		if (m_iState == CF_GETTING_OUT)
		{
			if (!driver.IsInVehicle())
			{
				Print("[ConvoyFollower] DISEMBARKED");
				ClearWaypoints();
				ResetToIdle();
			}
			else if (m_fStateSeconds >= 45.0)
			{
				Print("[ConvoyFollower] GET_OUT_TIMEOUT: driver remains seated");
				ClearWaypoints();
				ResetToIdle();
			}
			return;
		}

		if (m_iState != CF_FOLLOWING && m_iState != CF_LOST && m_iState != CF_PAUSED_ON_FOOT &&
			m_iState != CF_ARRIVING && m_iState != CF_UNLOAD_QUEUE &&
			m_iState != CF_UNLOAD_DEPARTING && m_iState != CF_UNLOAD_DEPARTED &&
			m_iState != CF_RETURN_WAIT && m_iState != CF_RETURN_TURNING && m_iState != CF_RETURN_BLOCKED)
			return;

		// A destroyed truck is an explicit terminal vehicle state. Lesser
		// damage is not treated as immobility; only the stall retry path below
		// makes that decision from observed lack of movement.
		if (IsAssignedTruckDestroyed())
		{
			Print("[ConvoyFollower] VEHICLE_DESTROYED: Unit " + m_iUnitNumber + " dismounting; convoy rewiring");
			StandDown();
			return;
		}
		if (IsDriverDestroyed(driver))
		{
			Print("[ConvoyFollower] DRIVER_DESTROYED: Unit " + m_iUnitNumber + " leaving convoy");
			StandDown();
			return;
		}

		if (!CF_IsBoarded())
		{
			if (driver.IsInVehicle() || IsDriverDestroyed(driver))
			{
				Print("[ConvoyFollower] ABORTED: Unit " + m_iUnitNumber + " cannot drive assigned truck");
				StandDown();
			}
			else
			{
				Print("[ConvoyFollower] REBOARD_STARTED: Unit " + m_iUnitNumber + " unexpectedly left driver seat");
				BeginUnexpectedReboard();
			}
			return;
		}
		SampleTruckRoute();
		if (m_iState == CF_UNLOAD_QUEUE)
			return;
		if (m_iState == CF_UNLOAD_DEPARTING)
		{
			if (m_iUnloadTurnPhase == 1)
			{
				m_fTurnStageSeconds += elapsed;
				if (vector.Distance(m_Truck.GetOrigin(), m_vUnloadTurnStage) <= CF_TURN_STAGE_RADIUS + 2.0)
				{
					m_iUnloadTurnPhase = 2;
					m_vLastUnloadTruckPosition = m_Truck.GetOrigin();
					m_fUnloadSlotStillSeconds = 0;
					if (!IssueMoveWaypoint(m_vUnloadWaitingPoint))
					{
						FailUnloadDeparture("could not issue rear parking waypoint");
						return;
					}
					SCR_AIWaypoint parkingWaypoint = SCR_AIWaypoint.Cast(m_Waypoint);
					if (parkingWaypoint)
						parkingWaypoint.SetCompletionRadius(CF_UNLOAD_SLOT_RADIUS);
					Print("[ConvoyFollower] UNLOAD_TURN_COMPLETE_STAGE: Unit " + m_iUnitNumber + " routing behind convoy tail");
				}
				else if (m_fTurnStageSeconds >= CF_TURN_STAGE_TIMEOUT_SECONDS)
					FailUnloadDeparture("lateral turn stage not reached");
				return;
			}
			UpdateUnloadSlotStillness(elapsed);
			if (!m_bUnloadClearReported && CF_IsUnloadBayClear(m_vUnloadAnchor))
			{
				m_bUnloadClearReported = true;
				Print("[ConvoyFollower] UNLOAD_BAY_CLEAR: Unit " + m_iUnitNumber + " settled behind tail");
				if (m_Session)
					m_Session.OnUnloadBayCleared(this);
			}
			else if (!m_bUnloadClearReported && m_fStateSeconds >= CF_UNLOAD_DEPART_TIMEOUT_SECONDS)
				FailUnloadDeparture("rear return slot not reached within two minutes");
			return;
		}
		if (m_iState == CF_UNLOAD_DEPARTED)
		{
			UpdateUnloadSlotStillness(elapsed);
			if (!m_bUnloadSlotParkedLogged && CF_IsAtUnloadWaitingPoint())
			{
				m_bUnloadSlotParkedLogged = true;
				ClearWaypoints();
				Print("[ConvoyFollower] RETURN_WAITING: former Unit " + m_iUnitNumber + " seated at rear return slot");
			}
			return;
		}
		if (m_iState == CF_RETURN_BLOCKED)
			return;
		if (m_iState == CF_RETURN_WAIT)
		{
			IEntity returnTarget = GetTargetVehicle();
			if (!returnTarget || !CanTargetMove())
				return;
			if (!TryGetVehicleFacing(returnTarget, m_vReturnHomeDirection))
			{
				FailReturnTurn("homeward predecessor has no usable heading");
				return;
			}
			if (IsFacingDirection(m_vReturnHomeDirection))
			{
				Print("[ConvoyFollower] RETURN_ALIGNED: Unit " + m_iUnitNumber + " already facing homeward");
				StartFollowing(returnTarget, false, false);
				return;
			}
			if (!TryGetVehicleFacing(m_Truck, m_vOutboundTravelDirection))
			{
				FailReturnTurn("current vehicle heading unavailable");
				return;
			}
			string turnSide;
			if (!TryChooseTurnStage(m_vOutboundTravelDirection, m_vReturnTurnStage, turnSide))
			{
				FailReturnTurn("no clear left or right road stage");
				return;
			}
			m_iReturnTurnPhase = 1;
			m_fStateSeconds = 0;
			SetState(CF_RETURN_TURNING);
			if (!IssueMoveWaypoint(m_vReturnTurnStage))
			{
				FailReturnTurn("could not issue turn stage");
				return;
			}
			SCR_AIWaypoint turnWaypoint = SCR_AIWaypoint.Cast(m_Waypoint);
			if (turnWaypoint)
				turnWaypoint.SetCompletionRadius(CF_TURN_STAGE_RADIUS);
			Print("[ConvoyFollower] RETURN_TURN_STAGE: Unit " + m_iUnitNumber + " taking clear " + turnSide + " road stage");
			return;
		}
		if (m_iState == CF_RETURN_TURNING)
		{
			if (m_iReturnTurnPhase == 1)
			{
				if (vector.Distance(m_Truck.GetOrigin(), m_vReturnTurnStage) <= CF_TURN_STAGE_RADIUS + 2.0)
				{
					IEntity followingTarget = GetTargetVehicle();
					if (!followingTarget || !CanTargetMove() || !IssueMoveWaypoint(followingTarget.GetOrigin()))
					{
						FailReturnTurn("homeward predecessor unavailable after turn stage");
						return;
					}
					m_iReturnTurnPhase = 2;
					m_fStateSeconds = 0;
					Print("[ConvoyFollower] RETURN_TURN_ROUTE: Unit " + m_iUnitNumber + " routing to homeward predecessor");
				}
				else if (m_fStateSeconds >= CF_TURN_STAGE_TIMEOUT_SECONDS)
					FailReturnTurn("lateral turn stage not reached");
				return;
			}
			if (IsFacingDirection(m_vReturnHomeDirection))
			{
				IEntity homeTarget = GetTargetVehicle();
				if (homeTarget && CanTargetMove())
				{
					Print("[ConvoyFollower] RETURN_TURN_ALIGNED: Unit " + m_iUnitNumber + " following homeward convoy");
					StartFollowing(homeTarget, false, false);
					return;
				}
			}
			if (m_fStateSeconds >= CF_RETURN_TURN_TIMEOUT_SECONDS)
				FailReturnTurn("homeward heading not reached");
			return;
		}

		ChimeraCharacter leader = ChimeraCharacter.Cast(m_Leader);
		if (!leader)
		{
			Print("[ConvoyFollower] ABORTED: ordering player unavailable");
			StandDown();
			return;
		}

		bool playerOnFoot = !leader.IsInVehicle() || m_bUnloadSequenceHold;
		IEntity targetVehicle = GetTargetVehicle();
		bool targetAvailable = targetVehicle && CanTargetMove();
		if (playerOnFoot && targetAvailable)
		{
			float arrivalSeparation = vector.Distance(m_Truck.GetOrigin(), targetVehicle.GetOrigin());
			if (m_iState == CF_LOST)
			{
				if (arrivalSeparation <= CF_ConvoySettings.Get().m_fRejoinDistance)
				{
					Print("[ConvoyFollower] ARRIVAL_REJOIN: Unit " + m_iUnitNumber + " recovered near parked target");
					StartArriving(targetVehicle);
					if (m_iState == CF_ARRIVING)
						SendRadioCall(CF_RadioEvent.REJOINED);
				}
				return;
			}

			if (m_iState != CF_ARRIVING)
				StartArriving(targetVehicle);
			if (m_iState != CF_ARRIVING)
				return;
		}
		else if (playerOnFoot)
		{
			if (m_iState != CF_PAUSED_ON_FOOT && m_iState != CF_LOST)
			{
				m_bPauseWasLost = m_iState == CF_LOST;
				ClearWaypoints();
				m_LeadVehicle = null;
				m_fLostSeconds = 0;
				ResetRangeWarning();
				m_fStuckSeconds = 0;
				m_iStuckRetries = 0;
				m_iReboardAttempts = 0;
				SetState(CF_PAUSED_ON_FOOT);
				Print("[ConvoyFollower] PAUSED: Unit " + m_iUnitNumber + " has no available arrival target");
				SendRadioCall(CF_RadioEvent.HOLDING);
			}
			return;
		}

		if (!targetAvailable)
		{
			if (CF_IsMovementActive())
				CF_HoldForMissingPredecessor();
			return;
		}
		float separation = vector.Distance(m_Truck.GetOrigin(), targetVehicle.GetOrigin());

		if (m_iState == CF_ARRIVING && !playerOnFoot &&
			(targetVehicle != m_LeadVehicle ||
			vector.Distance(m_vArrivalAnchorPosition, targetVehicle.GetOrigin()) >= CF_ARRIVAL_RESUME_DISTANCE))
		{
			Print("[ConvoyFollower] ARRIVAL_RESUMED: convoy target moving; Unit " + m_iUnitNumber + " following again");
			StartFollowing(targetVehicle, false, false);
			return;
		}

		if (m_iState == CF_PAUSED_ON_FOOT)
		{
			// Preserve the prior lost boundary for this unit's own predecessor.
			if (separation > CF_ConvoySettings.Get().m_fLostDistance || (m_bPauseWasLost && separation > CF_ConvoySettings.Get().m_fRejoinDistance))
			{
				SetState(CF_LOST);
				Print("[ConvoyFollower] LOST: Unit " + m_iUnitNumber + " resumed beyond recovery range");
				if (!m_bPauseWasLost)
					SendRadioCall(CF_RadioEvent.LOST);
				return;
			}

			Print("[ConvoyFollower] RESUMED: player reentered vehicle, Unit " + m_iUnitNumber);
			StartFollowing(targetVehicle, m_bPauseWasLost, true);
			return;
		}

		if (m_iState == CF_LOST)
		{
			if (separation <= CF_ConvoySettings.Get().m_fRejoinDistance)
			{
				Print("[ConvoyFollower] RETRY: predecessor returned within range of Unit " + m_iUnitNumber);
				StartFollowing(targetVehicle, true, true);
			}
			return;
		}

		// The return line and outbound queue can start far apart. Give their
		// road-routed merge time to close before applying ordinary lost rules.
		if (m_fReturnMergeGraceSeconds > 0)
		{
			if (separation <= CF_ConvoySettings.Get().m_fRejoinDistance)
				m_fReturnMergeGraceSeconds = 0;
			else
				m_fReturnMergeGraceSeconds -= elapsed;
		}
		bool returnMergeGrace = m_fReturnMergeGraceSeconds > 0;
		// Emit one warning after sustained separation. A brief dip below the
		// warning threshold cannot rearm it until the closer reset distance.
		if (returnMergeGrace)
			ResetRangeWarning();
		else if (separation <= CF_ConvoySettings.Get().m_fRangeWarningRearmDistance)
			ResetRangeWarning();
		else if (!m_bRangeWarningIssued)
		{
			if (separation >= CF_ConvoySettings.Get().m_fRangeWarningDistance)
			{
				m_fRangeWarningSeconds += elapsed;
				if (m_fRangeWarningSeconds >= CF_ConvoySettings.Get().m_fRangeWarningSeconds)
				{
					m_bRangeWarningIssued = true;
					Print("[ConvoyFollower] RANGE_WARNING: Unit " + m_iUnitNumber + " beyond configured warning distance");
					SendRadioCall(CF_RadioEvent.FAR_WARNING);
				}
			}
			else
				m_fRangeWarningSeconds = 0;
		}

		if (!returnMergeGrace && separation > CF_ConvoySettings.Get().m_fLostDistance)
			m_fLostSeconds += elapsed;
		else
			m_fLostSeconds = 0;

		if (m_fLostSeconds >= CF_ConvoySettings.Get().m_fLostGraceSeconds)
		{
			Print("[ConvoyFollower] LOST: Unit " + m_iUnitNumber + " too far behind predecessor");
			ClearWaypoints();
			SetState(CF_LOST);
			SendRadioCall(CF_RadioEvent.LOST);
			return;
		}

		m_LeadVehicle = targetVehicle;
		if (!m_LeadVehicle)
			return;
		ObserveConvoyOrder(m_LeadVehicle, separation);
		if (m_bOrderInversionLogged)
		{
			HoldOutOfOrderWaypoint(m_LeadVehicle);
			SetUnloadReleaseReady(false);
			m_fStuckSeconds = 0;
			m_iStuckRetries = 0;
			return;
		}

		m_fStuckSeconds += elapsed;
		if (m_fStuckSeconds >= CF_ConvoySettings.Get().m_fStuckCheckSeconds)
		{
			float movement = vector.Distance(m_vLastTruckPosition, m_Truck.GetOrigin());
			float targetMovement = vector.Distance(m_vLastStallTargetPosition, m_LeadVehicle.GetOrigin());
			if (separation > CF_ConvoySettings.Get().m_fStuckLeadDistance && movement < 3.0 && !(m_Predecessor && m_Predecessor.CF_HasStallAhead()))
			{
				Print("[ConvoyFollower] DRIVE_STALL_CHECK: Unit " + m_iUnitNumber +
					" gap " + separation + " m, moved " + movement + " m, predecessor moved " + targetMovement + " m");
				m_iStuckRetries++;
				if (m_iStuckRetries >= CF_ConvoySettings.Get().m_iStallMaxChecks)
				{
					Print("[ConvoyFollower] STUCK_TERMINAL: Unit " + m_iUnitNumber + " dismounting after configured stall checks; convoy rewiring");
					StandDown();
					return;
				}

				Print("[ConvoyFollower] RETRY: driver stalled, issuing a new move waypoint");
				if (m_iStuckRetries == 1)
					SendRadioCall(CF_RadioEvent.STUCK);
				IssueMoveWaypoint(m_LeadVehicle.GetOrigin());
			}
			else if (movement >= 3.0)
				m_iStuckRetries = 0;

			m_fStuckSeconds = 0;
			m_vLastTruckPosition = m_Truck.GetOrigin();
			m_vLastStallTargetPosition = m_LeadVehicle.GetOrigin();
		}

		SettleBehindStoppedTarget(elapsed, separation);
		if (m_iState == CF_FOLLOWING && m_fTargetStillSeconds >= CF_STOP_DETECT_SECONDS)
		{
			// This also applies while the player remains in the stopped lead
			// vehicle. Keep the existing road waypoint and its settled radius.
			m_vArrivalLastTruckPosition = m_Truck.GetOrigin();
			m_fArrivalTruckStillSeconds = 0;
			m_bArrivalCloseLogged = false;
			m_vArrivalAnchorPosition = targetVehicle.GetOrigin();
			SetState(CF_ARRIVING);
			Print("[ConvoyFollower] ARRIVAL_APPROACH: Unit " + m_iUnitNumber + " closing behind stopped convoy target");
		}
		if (m_iState == CF_ARRIVING)
		{
			if (vector.Distance(m_Truck.GetOrigin(), m_vArrivalLastTruckPosition) < 0.8)
				m_fArrivalTruckStillSeconds += elapsed;
			else
				m_fArrivalTruckStillSeconds = 0;
			m_vArrivalLastTruckPosition = m_Truck.GetOrigin();
			bool nearStoppedTarget = m_fTargetStillSeconds >= CF_STOP_DETECT_SECONDS &&
				m_fArrivalTruckStillSeconds >= CF_UNLOAD_SLOT_STILL_SECONDS &&
				separation <= CF_ConvoySettings.Get().m_fStoppedGap + 4.0;
			SetUnloadReleaseReady(!m_Predecessor && nearStoppedTarget);
			if (nearStoppedTarget && !m_bArrivalCloseLogged)
			{
				m_bArrivalCloseLogged = true;
				Print("[ConvoyFollower] ARRIVAL_CLOSE: Unit " + m_iUnitNumber + " stopped near convoy target");
			}
		}

		bool missingMoveWaypoint = !HasOwnWaypointInGroup();
		float refreshDelay = CF_WAYPOINT_REFRESH_SECONDS;
		if (missingMoveWaypoint)
			refreshDelay = CF_MISSING_WAYPOINT_RECOVERY_SECONDS;
		bool targetAdvanced = vector.Distance(m_vLastWaypointPosition, m_LeadVehicle.GetOrigin()) >= CF_WAYPOINT_REFRESH_DISTANCE;
		bool outsideFollowGap = separation > CF_ConvoySettings.Get().m_fMovingGap + 6.0;
		if (m_fWaypointSeconds >= refreshDelay &&
			(targetAdvanced || (missingMoveWaypoint && outsideFollowGap)))
		{
			if (!MoveWaypoint(m_LeadVehicle.GetOrigin()))
			{
				Print("[ConvoyFollower] FOLLOW_FAILED: cannot refresh move waypoint");
				StandDown();
			}
			else if (missingMoveWaypoint)
				Print("[ConvoyFollower] FOLLOW_WAYPOINT_RECOVERED: Unit " + m_iUnitNumber + " rebuilt completed move order at " + separation + " m");
		}
	}

	override void OnDelete(IEntity owner)
	{
		if (m_Driver)
			ClearEventMask(m_Driver, EntityEvent.FRAME);
		NotifySessionUnavailable();

		// Only release this component's active order during entity deletion.
		if (Replication.IsServer() && m_Group && m_Waypoint)
			m_Group.RemoveWaypoint(m_Waypoint);
	}
}
