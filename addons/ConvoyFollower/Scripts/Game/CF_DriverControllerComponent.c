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
	protected static const int CF_PANEL_HOLD = 21;
	protected static const int CF_FORWARD_WAIT_DEPARTED = 22;
	protected static const int CF_FORWARD_OUTBOUND_HOLD = 23;

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
	protected static const float CF_ARRIVAL_RESUME_GAP_BUFFER = 8.0;
	// Once a truck has actually settled at an arrival, leave its completed
	// MOVE alone until there is enough room for another useful road order.
	protected static const float CF_ARRIVAL_CLOSE_REAPPROACH_BUFFER = 18.0;
	// Once a stopped truck has met the close-arrival gate, a few metres of
	// residual vehicle motion must not hide its rear release action.
	protected static const float CF_ARRIVAL_RELEASE_EXIT_BUFFER = 8.0;
	// MOVE completes inside its radius. A following truck must not wait eight
	// seconds for a fresh order after the vehicle ahead drives away.
	protected static const float CF_MISSING_WAYPOINT_RECOVERY_SECONDS = 1.0;
	protected static const float CF_MISSING_WAYPOINT_STILL_RETRY_SECONDS = 12.0;
	protected static const float CF_MISSING_WAYPOINT_TARGET_ADVANCE = 4.0;
	protected static const float CF_MISSING_WAYPOINT_GAP_BUFFER = 10.0;
	protected static const float CF_MISSING_WAYPOINT_STILL_GAP_BUFFER = 10.0;
	protected static const float CF_ORDER_INVERSION_CHECK_DISTANCE = 80.0;
	protected static const float CF_ORDER_INVERSION_START_AHEAD = 12.0;
	protected static const float CF_ORDER_INVERSION_CLEAR_AHEAD = 3.0;
	protected static const float CF_ORDER_INVERSION_MAX_LATERAL = 12.0;
	protected static const float CF_STOP_SETTLE_DISTANCE = 14.0;
	protected static const float CF_STOPPED_TRAIL_BACK_DISTANCE = 12.0;
	protected static const float CF_STOPPED_TRAIL_MAX_TARGET_DISTANCE = 24.0;
	protected static const float CF_STOPPED_TRAIL_DIAGNOSTIC_INTERVAL_MS = 10000.0;
	protected static const float CF_STOPPED_ROAD_GOAL_RADIUS = 4.0;
	// A successor can stop a few metres before its saved cargo-bay MOVE goal
	// while still being close enough to unload at the same road position.
	protected static const float CF_UNLOAD_BAY_READY_RADIUS = 8.0;
	protected static const float CF_UNLOAD_BAY_READY_EXIT_RADIUS = 12.0;
	protected static const float CF_STOPPED_ROAD_CLOSE_DISTANCE = 5.0;
	protected static const float CF_STOPPED_ROAD_MAX_DISTANCE = 6.0;
	protected static const float CF_STOPPED_ROAD_SHOULDER_BUFFER = 2.0;
	protected static const float CF_ARRIVAL_ROAD_RECOVERY_RADIUS = 2.5;
	protected static const float CF_ARRIVAL_ROAD_RECOVERY_TIMEOUT = 25.0;
	protected static const float CF_ROUTE_SAMPLE_DISTANCE = 5.0;
	protected static const int CF_ROUTE_SAMPLE_LIMIT = 64;
	protected static const float CF_LEAD_TRAIL_SAMPLE_DISTANCE = 6.0;
	protected static const float CF_LEAD_TRAIL_REACHED_DISTANCE = 12.0;
	// A stopped truck can coast past its current sample while the waypoint
	// completes. Resync only to a later point it actually drove through.
	protected static const float CF_LEAD_TRAIL_RESYNC_DISTANCE = 8.0;
	protected static const float CF_LEAD_TRAIL_LOOKAHEAD_DISTANCE = 36.0;
	protected static const float CF_LEAD_TRAIL_COMPLETED_DISTANCE = 20.0;
	protected static const float CF_LEAD_TRAIL_WAYPOINT_LOOKAHEAD = 24.0;
	protected static const int CF_LEAD_TRAIL_SAMPLE_LIMIT = 128;
	protected static const float CF_REBOARD_EPISODE_PROGRESS_METERS = 5.0;
	protected static const float CF_REBOARD_EPISODE_RESET_SECONDS = 90.0;
	protected static const int CF_REBOARD_EPISODE_MAX_EXITS = 3;
	protected static const float CF_UNLOAD_BAY_CLEAR_DISTANCE = 18.0;
	// A forward-wait truck parks on the mapped road. The owner needs room to
	// pass it later; a single-track lane is not a usable forward waiting slot.
	protected static const float CF_FORWARD_PASS_MIN_ROAD_WIDTH = 8.0;
	// A direct rearward goal asks the vehicle AI to turn around without a
	// verified lateral stage. Only offer that attempt on a measured wide road.
	protected static const float CF_REAR_DIRECT_MIN_ROAD_WIDTH = 8.0;
	protected static const float CF_REAR_DIRECT_MAX_ROAD_OFFSET = 6.0;
	protected static const float CF_FORWARD_PASS_SAMPLE_DISTANCE = 8.0;
	protected static const float CF_FORWARD_PASS_MAX_ROAD_OFFSET = 4.0;
	protected static const float CF_UNLOAD_SLOT_RADIUS = 5.0;
	protected static const float CF_UNLOAD_SLOT_STILL_SECONDS = 2.0;
	// Forward parking needs spare distance for a truck to settle on a grade.
	// The rear return maneuver keeps its separately tested capture radius.
	protected static const float CF_FORWARD_SLOT_BRAKE_RADIUS = 6.5;
	protected static const float CF_FORWARD_SLOT_WAYPOINT_RADIUS = 3.5;
	protected static const float CF_FORWARD_SLOT_STILL_SECONDS = 6.0;
	protected static const float CF_FORWARD_SLOT_STILL_MOVEMENT = 0.4;
	protected static const float CF_UNLOAD_DEPART_TIMEOUT_SECONDS = 120.0;
	protected static const float CF_UNLOAD_SLOT_REISSUE_CHECK_SECONDS = 15.0;
	protected static const float CF_UNLOAD_SLOT_REISSUE_MIN_PROGRESS = 3.0;
	protected static const float CF_UNLOAD_SLOT_BEST_PROGRESS_TIMEOUT_SECONDS = 30.0;
	protected static const int CF_UNLOAD_SLOT_MAX_REISSUES = 2;
	protected static const float CF_TURN_STAGE_TIMEOUT_SECONDS = 35.0;
	protected static const float CF_UNLOAD_STAGE_STALL_FALLBACK_SECONDS = 12.0;
	protected static const float CF_UNLOAD_STAGE_PROGRESS_METERS = 3.0;
	protected static const float CF_TURN_STAGE_RADIUS = 3.5;
	protected static const float CF_RETURN_TURN_TIMEOUT_SECONDS = 90.0;
	protected static const float CF_RETURN_HEADING_DOT = 0.7;
	protected static const float CF_RETURN_MERGE_GRACE_SECONDS = 180.0;
	protected static const float CF_RETURN_OWNER_PASS_DISTANCE = 12.0;
	protected static const float CF_RETURN_OWNER_PASS_CORRIDOR = 24.0;
	protected static const float CF_RETURN_ROAD_GOAL_LOOKAHEAD = 20.0;
	protected static const float CF_RETURN_NAV_LOG_SECONDS = 5.0;
	protected static const float CF_RETURN_MIN_TURN_PROGRESS = 5.0;
	protected static const float CF_RETURN_GOAL_REISSUE_SECONDS = 5.0;
	protected static const int CF_RETURN_GOAL_MAX_REISSUES = 4;

	// ScriptedUserAction visibility runs on clients, so mirror this compact state.
	[RplProp()]
	protected int m_iState = CF_IDLE;
	[RplProp()]
	protected bool m_bUnloadReleaseReady;
	[RplProp()]
	protected bool m_bSessionReleaseAllowed = true;
	protected int m_iStuckRetries;
	protected int m_iReboardAttempts;
	protected int m_iStationaryUnexpectedExits;
	protected float m_fSinceUnexpectedExitSeconds;
	protected vector m_vUnexpectedExitEpisodeStart;
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
	protected float m_fNextStoppedTrailDiagnosticMs;
	protected float m_fNextForwardLaneDiagnosticMs;
	protected float m_fNextFollowWaitDiagnosticMs;
	protected bool m_bStopSettleIssued;
	protected bool m_bArrivalCloseLogged;
	protected bool m_bOrderInversionLogged;
	protected bool m_bUnloadSequenceHold;
	protected bool m_bForwardOutboundHoldRequested;
	protected bool m_bUnloadAnchorValid;
	protected bool m_bUnloadClearReported;
	protected bool m_bUnloadSlotParkedLogged;
	protected bool m_bReturnTurnFailed;
	protected bool m_bReturnWaitForOwnerPass;
	protected bool m_bReturnPassStartValid;
	protected bool m_bSilentReturnFollow;
	protected bool m_bTurnProbeOccupied;
	protected bool m_bReleaseRouteHistoryFallback;
	protected bool m_bArrivalRoadHold;
	protected bool m_bArrivalTrailMode;
	protected bool m_bArrivalTrailHold;
	protected bool m_bArrivalTrailGoalValid;
	protected bool m_bArrivalRoadRecoveryActive;
	protected bool m_bArrivalRoadRecoveryAttempted;
	protected bool m_bArrivalRoadRecoveryBlocked;
	protected bool m_bOwnVehicleBrake;
	protected bool m_bUnloadSlotBrakeCaptured;
	protected bool m_bForwardWaitDeparture;
	protected bool m_bUnloadBayGoalValid;
	protected bool m_bUnloadIntermediateActive;
	protected string m_sReleasePlanFailureReason;
	protected int m_iUnloadReboardState;
	protected int m_iUnloadTurnPhase;
	protected int m_iUnloadSlotReissues;
	protected int m_iReturnTurnPhase;
	protected int m_iReturnGoalReissues;
	protected float m_fUnloadSlotStillSeconds;
	protected float m_fForwardParkedDiagnosticSeconds;
	protected float m_fArrivalTruckStillSeconds;
	protected float m_fArrivalRoadRecoverySeconds;
	protected float m_fTurnStageSeconds;
	protected float m_fUnloadDepartureDiagnosticSeconds;
	protected float m_fUnloadDepartureElapsedSeconds;
	protected float m_fUnloadSlotProgressSeconds;
	protected float m_fUnloadSlotProgressGap;
	protected float m_fUnloadBestSlotGap;
	protected float m_fUnloadNoBestProgressSeconds;
	protected float m_fReturnMergeGraceSeconds;
	protected float m_fReturnNavLogSeconds;
	protected float m_fReturnGoalRetrySeconds;
	protected float m_fCandidateDistance;
	protected vector m_vLastWaypointPosition;
	protected vector m_vLastTruckPosition;
	protected vector m_vLastTargetPosition;
	protected vector m_vLastStallTargetPosition;
	protected vector m_vArrivalAnchorPosition;
	protected vector m_vArrivalTrailGoal;
	protected vector m_vLastFootPosition;
	protected vector m_vUnloadAnchor;
	protected vector m_vUnloadWaitingPoint;
	protected vector m_vUnloadBayGoal;
	protected vector m_vLastUnloadTruckPosition;
	protected vector m_vUnloadStillnessAnchor;
	protected vector m_vForwardParkedPosition;
	protected vector m_vUnloadSlotProgressPosition;
	protected vector m_vUnloadIntermediateGoal;
	protected vector m_vArrivalLastTruckPosition;
	protected vector m_vOutboundTravelDirection;
	protected vector m_vUnloadHomeDirection;
	protected vector m_vReturnHomeDirection;
	protected vector m_vUnloadTurnStage;
	protected vector m_vReturnTurnStage;
	protected vector m_vReturnTurnStartPosition;
	protected vector m_vReturnPassStartPosition;
	protected ref array<vector> m_aRecentTruckPositions = {};
	protected ref array<vector> m_aLeadTrailPositions = {};
	protected IEntity m_LeadTrailTarget;
	protected IEntity m_ReturnPassVehicle;
	protected int m_iLeadTrailCursor;
	protected bool m_bLeadTrailGuidanceLogged;
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
		if (state != CF_UNLOAD_DEPARTING && state != CF_UNLOAD_DEPARTED &&
			state != CF_FORWARD_WAIT_DEPARTED)
			m_bUnloadSlotBrakeCaptured = false;
		if (state != CF_ARRIVING)
			CF_ResetArrivalRoadRecovery();
		if (Replication.IsServer())
		{
			Replication.BumpMe();
			CF_UpdateVehicleBrake();
		}
	}

	// Removing a MOVE waypoint does not guarantee that an AI-controlled truck
	// stops. In live tests a waiting follower drove off-road with no waypoint,
	// and a released truck rolled 14 m beyond its completed parking waypoint.
	// Only this controller's seated truck is braked, and every moving state
	// releases the brake before a fresh driving order.
	protected bool CF_ShouldHoldVehicleBrake()
	{
		if (!m_Truck || !CF_IsBoarded())
			return false;
		if (m_iState == CF_UNLOAD_QUEUE || m_iState == CF_UNLOAD_DEPARTED ||
			m_iState == CF_FORWARD_WAIT_DEPARTED ||
			m_iState == CF_FORWARD_OUTBOUND_HOLD ||
			m_iState == CF_PANEL_HOLD ||
			m_iState == CF_RETURN_WAIT || m_iState == CF_RETURN_BLOCKED)
			return true;
		if (m_iState == CF_ARRIVING)
		{
			if (m_bArrivalTrailMode)
			{
				if (!m_bArrivalTrailHold && m_fTargetStillSeconds >= CF_STOP_DETECT_SECONDS &&
					CF_IsOrdinaryTrailArrivalClose() && CF_IsPanelVehicleSlow(2.0))
				{
					m_bArrivalTrailHold = true;
					ClearWaypoints();
					Print("[ConvoyFollower] ARRIVAL_TRAIL_HOLD: Unit " + m_iUnitNumber +
						" seated and stopped near actual predecessor trail; road-only release unavailable");
				}
				return m_bArrivalTrailHold;
			}
			if (m_bArrivalRoadRecoveryBlocked)
				return true;
			if (m_bArrivalRoadRecoveryActive)
				return false;
			bool withinArrivalGoal = false;
			if (m_bUnloadSequenceHold && m_bUnloadBayGoalValid)
				withinArrivalGoal = vector.Distance(m_Truck.GetOrigin(), m_vUnloadBayGoal) <=
					CF_UNLOAD_BAY_READY_RADIUS;
			else if (m_LeadVehicle)
				withinArrivalGoal = vector.Distance(m_Truck.GetOrigin(), m_LeadVehicle.GetOrigin()) <=
					CF_ConvoySettings.Get().m_fStoppedGap + 4.0;
			if (!m_bArrivalRoadHold && m_LeadVehicle &&
				m_fTargetStillSeconds >= CF_STOP_DETECT_SECONDS &&
				withinArrivalGoal && CF_IsTruckNearMappedRoad())
			{
				m_bArrivalRoadHold = true;
				ClearWaypoints();
				Print("[ConvoyFollower] ARRIVAL_ROAD_HOLD: Unit " + m_iUnitNumber +
					" stopped on mapped road behind predecessor");
			}
			return m_bArrivalRoadHold;
		}
		if (m_iState != CF_UNLOAD_DEPARTING || m_iUnloadTurnPhase != 2)
			return false;
		float brakeCaptureRadius = CF_UNLOAD_SLOT_RADIUS + 4.0;
		if (m_bForwardWaitDeparture)
			brakeCaptureRadius = CF_FORWARD_SLOT_BRAKE_RADIUS;
		if (!m_bUnloadSlotBrakeCaptured &&
			vector.Distance(m_Truck.GetOrigin(), m_vUnloadWaitingPoint) <= brakeCaptureRadius &&
			(m_bReleaseRouteHistoryFallback || CF_IsTruckNearMappedRoad()))
		{
			m_bUnloadSlotBrakeCaptured = true;
			ClearWaypoints();
			Print("[ConvoyFollower] UNLOAD_SLOT_BRAKE_CAPTURED: Unit " + m_iUnitNumber +
				" stopping at validated return slot " + m_vUnloadWaitingPoint);
			if (m_bForwardWaitDeparture)
				CF_LogForwardParkAlignment("brake_capture");
		}
		return m_bUnloadSlotBrakeCaptured;
	}

	// Road width alone does not show whether a long parked truck leaves room
	// to pass. Record its actual heading and lateral centerline offset at the
	// brake capture and completed park; the live pass test decides acceptance.
	protected void CF_LogForwardParkAlignment(string phase)
	{
		if (!m_Truck)
			return;
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
		{
			Print("[ConvoyFollower] FORWARD_PARK_ALIGNMENT: " + phase +
				" truck=" + m_Truck.GetName() + " road navigation unavailable");
			return;
		}
		vector origin = m_Truck.GetOrigin();
		BaseRoad road;
		float roadDistance;
		aiWorld.GetRoadNetworkManager().GetClosestRoad(origin, road, roadDistance);
		if (!road)
		{
			Print("[ConvoyFollower] FORWARD_PARK_ALIGNMENT: " + phase +
				" truck=" + m_Truck.GetName() + " no mapped road");
			return;
		}
		ref array<vector> points = {};
		road.GetPoints(points);
		float bestDistance = 1000000.0;
		vector tangent = vector.Zero;
		vector center = vector.Zero;
		for (int i = 0; i < points.Count() - 1; i++)
		{
			vector start = points[i];
			vector end = points[i + 1];
			float dx = end[0] - start[0];
			float dz = end[2] - start[2];
			float lengthSquared = dx * dx + dz * dz;
			if (lengthSquared < 0.01)
				continue;
			float progress = ((origin[0] - start[0]) * dx +
				(origin[2] - start[2]) * dz) / lengthSquared;
			if (progress < 0.0)
				progress = 0.0;
			else if (progress > 1.0)
				progress = 1.0;
			vector projected = Vector(start[0] + dx * progress, origin[1],
				start[2] + dz * progress);
			float distance = vector.DistanceXZ(origin, projected);
			if (distance >= bestDistance)
				continue;
			bestDistance = distance;
			center = projected;
			float length = Math.Sqrt(lengthSquared);
			tangent = Vector(dx / length, 0, dz / length);
		}
		if (tangent == vector.Zero)
			return;
		if (tangent[0] * m_vOutboundTravelDirection[0] +
			tangent[2] * m_vOutboundTravelDirection[2] < 0)
			tangent = Vector(-tangent[0], 0, -tangent[2]);
		vector facing = m_Truck.GetWorldTransformAxis(2);
		float facingLength = Math.Sqrt(facing[0] * facing[0] + facing[2] * facing[2]);
		float headingDot = -2.0;
		if (facingLength > 0.1)
			headingDot = (facing[0] * tangent[0] + facing[2] * tangent[2]) / facingLength;
		float signedLateral = -tangent[2] * (origin[0] - center[0]) +
			tangent[0] * (origin[2] - center[2]);
		Print("[ConvoyFollower] FORWARD_PARK_ALIGNMENT: " + phase +
			" truck=" + m_Truck.GetName() + " origin=" + origin +
			" slot_gap=" + vector.Distance(origin, m_vUnloadWaitingPoint) +
			" road_width=" + road.GetWidth() + " road_distance=" + roadDistance +
			" signed_lateral=" + signedLateral + " outbound_tangent=" + tangent +
			" heading_dot=" + headingDot);
	}

	protected void CF_ResetArrivalRoadRecovery()
	{
		m_bArrivalRoadHold = false;
		m_bArrivalTrailMode = false;
		m_bArrivalTrailHold = false;
		m_bArrivalTrailGoalValid = false;
		m_bArrivalRoadRecoveryActive = false;
		m_bArrivalRoadRecoveryAttempted = false;
		m_bArrivalRoadRecoveryBlocked = false;
		m_fArrivalRoadRecoverySeconds = 0;
	}

	protected void CF_UpdateVehicleBrake()
	{
		if (!Replication.IsServer() || !m_Truck)
			return;
		CarControllerComponent car = CarControllerComponent.Cast(m_Truck.FindComponent(CarControllerComponent));
		if (!car)
			return;
		VehicleWheeledSimulation sim = car.GetSimulation();
		if (CF_ShouldHoldVehicleBrake())
		{
			m_bOwnVehicleBrake = true;
			car.SetPersistentHandBrake(true);
			if (sim)
			{
				sim.SetThrottle(0);
				sim.SetBreak(1, true);
			}
			return;
		}
		CF_ReleaseVehicleBrake();
	}

	protected void CF_ReleaseVehicleBrake()
	{
		if (!m_bOwnVehicleBrake)
			return;
		m_bOwnVehicleBrake = false;
		if (!m_Truck)
			return;
		CarControllerComponent car = CarControllerComponent.Cast(m_Truck.FindComponent(CarControllerComponent));
		if (!car)
			return;
		VehicleWheeledSimulation sim = car.GetSimulation();
		car.SetPersistentHandBrake(false);
		if (sim)
			sim.SetBreak(0, true);
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
		SetEventMask(m_Driver, EntityEvent.POSTFRAME);
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

	// Prepare once at the confirmed initial driver-seat transition. A short
	// first MOVE can complete inside its radius before native AI starts the
	// engine; waiting for a later MOVE then adds a second startup delay.
	// StartEngine is the ordinary controller input request, not a forced
	// simulation start. Native AI keeps all throttle, gear and steering control.
	protected void CF_PrepareEngineAfterBoarding()
	{
		if (!Replication.IsServer() || m_iState != CF_BOARDING || !m_Session || !CF_IsBoarded())
			return;
		ChimeraCharacter driver = ChimeraCharacter.Cast(m_Driver);
		if (!driver || IsDriverDestroyed(driver) || IsAssignedTruckDestroyed())
			return;
		CarControllerComponent car = CarControllerComponent.Cast(m_Truck.FindComponent(CarControllerComponent));
		if (!car || !car.GetSimulation())
			return;
		VehicleWheeledSimulation sim = car.GetSimulation();
		if (sim.EngineIsOn())
		{
			Print("[ConvoyFollower] ENGINE_PREPARE: truck=" + m_Truck.GetName() + " already_on=true");
			return;
		}
		bool requestResult = car.StartEngine();
		Print("[ConvoyFollower] ENGINE_PREPARE: truck=" + m_Truck.GetName() +
			" request_result=" + requestResult + " engine_on=" + sim.EngineIsOn() +
			" seated=" + CF_IsBoarded() + " one_shot=true");
	}

	bool CF_IsMovementActive()
	{
		return m_iState == CF_FOLLOWING || m_iState == CF_ARRIVING;
	}

	int CF_GetUnitNumber()
	{
		return m_iUnitNumber;
	}

	string CF_GetPanelStateLabel()
	{
		if (m_iState == CF_PANEL_HOLD)
			return "holding in vehicle";
		if (m_iState == CF_FOLLOWING)
			return "following";
		if (m_iState == CF_ARRIVING)
		{
			if (m_bArrivalTrailHold)
				return "holding on driven route";
			if (m_bArrivalRoadRecoveryBlocked)
				return "road arrival blocked";
			return "closing at stop";
		}
		if (m_iState == CF_LOST)
			return "lost";
		if (m_iState == CF_UNLOAD_QUEUE)
			return "waiting to unload";
		if (m_iState == CF_FORWARD_OUTBOUND_HOLD)
			return "holding outbound for forward line";
		if (m_iState == CF_UNLOAD_DEPARTING)
			return "moving to waiting slot";
		if (m_iState == CF_UNLOAD_DEPARTED)
			return "parked in return line";
		if (m_iState == CF_FORWARD_WAIT_DEPARTED)
			return "parked ahead";
		if (m_iState == CF_RETURN_WAIT)
			return "waiting to return";
		if (m_iState == CF_RETURN_TURNING)
			return "turning for return";
		if (m_iState == CF_RETURN_BLOCKED)
			return "return blocked";
		if (m_iState == CF_REBOARDING)
			return "reboarding";
		if (m_iState == CF_BOARDING)
			return "boarding";
		if (m_iState == CF_WAITING_FOR_PREDECESSOR || m_iState == CF_WAITING_FOR_LEAD)
			return "waiting for lead";
		if (m_iState == CF_GETTING_OUT)
			return "getting out";
		if (m_iState == CF_PAUSED_ON_FOOT)
			return "waiting for owner";
		return "not driving";
	}

	bool CF_IsPanelHeld()
	{
		return m_iState == CF_PANEL_HOLD && CF_IsBoarded();
	}

	bool CF_IsPanelVehicleSlow(float maxKmh)
	{
		if (!CF_IsBoarded())
			return false;
		CarControllerComponent car = CarControllerComponent.Cast(m_Truck.FindComponent(CarControllerComponent));
		if (!car || !car.GetSimulation())
			return false;
		return car.GetSimulation().GetSpeedKmh() <= maxKmh;
	}

	bool CF_CanPanelHold()
	{
		return CF_CanApproachPanelHold() && CF_IsPanelVehicleSlow(10.0);
	}

	// A driver may still be closing the normal convoy gap when the owner
	// orders Hold. Keep that waypoint until this truck itself slows; clearing
	// a fast truck's MOVE order can send it into an uncontrolled stop.
	bool CF_CanApproachPanelHold()
	{
		return CF_IsBoarded() &&
			(m_iState == CF_FOLLOWING || m_iState == CF_ARRIVING ||
			m_iState == CF_WAITING_FOR_PREDECESSOR || m_iState == CF_WAITING_FOR_LEAD ||
			m_iState == CF_PAUSED_ON_FOOT || m_iState == CF_PANEL_HOLD);
	}

	bool CF_PanelHold()
	{
		if (!Replication.IsServer() || !CF_CanPanelHold())
			return false;
		if (m_iState == CF_PANEL_HOLD)
			return true;
		ClearWaypoints();
		SetState(CF_PANEL_HOLD);
		Print("[ConvoyFollower] PANEL_HOLD: Unit " + m_iUnitNumber + " remains seated");
		return true;
	}

	bool CF_CanPanelResume()
	{
		Resource movePrefab = Resource.Load(CF_MOVE_WAYPOINT);
		bool resumableState = m_iState == CF_PANEL_HOLD || m_iState == CF_FOLLOWING ||
			m_iState == CF_ARRIVING || m_iState == CF_WAITING_FOR_PREDECESSOR ||
			m_iState == CF_WAITING_FOR_LEAD || m_iState == CF_LOST;
		return resumableState && CF_IsBoarded() && m_Group && movePrefab.IsValid() &&
			GetTargetVehicle() && (!m_Predecessor || m_Predecessor.CF_IsBoarded());
	}

	bool CF_PanelResume()
	{
		if (!Replication.IsServer() || !CF_CanPanelResume())
			return false;
		IEntity target = GetTargetVehicle();
		StartFollowing(target, false, false);
		Print("[ConvoyFollower] PANEL_RESUME: Unit " + m_iUnitNumber +
			" follows its assigned predecessor");
		return m_iState == CF_FOLLOWING;
	}

	bool CF_CanPanelDismount()
	{
		return CF_IsPanelHeld() && CF_IsPanelVehicleSlow(2.0);
	}

	bool CF_IsOrderInverted()
	{
		return m_bOrderInversionLogged && CF_IsMovementActive();
	}

	bool CF_IsActiveConvoyMember()
	{
		return m_Session && m_Session.GetUnitNumber(this) > 0 && CF_IsBoarded();
	}

	bool CF_IsOwnedSeatedConvoyMember()
	{
		return m_Session && m_Session.IsOwnedRadioMember(this) && CF_IsBoarded();
	}

	bool CF_CanReleaseAtUnload()
	{
		if (m_iState != CF_ARRIVING || !m_bUnloadReleaseReady || !m_bSessionReleaseAllowed)
			return false;
		// m_Truck is server-owned; action visibility uses the replicated flag,
		// while the server also verifies that this driver remains seated.
		return !Replication.IsServer() || CF_IsBoarded();
	}

	string CF_GetReleaseEligibilityReason()
	{
		if (!CF_IsBoarded())
			return "driver is not seated in the assigned truck";
		if (m_iState != CF_ARRIVING)
			return "front truck has not settled at the unload bay";
		if (!m_bSessionReleaseAllowed)
			return "another unload maneuver is still active";
		if (!m_bUnloadReleaseReady)
			return "wait for the lead vehicle and front truck to settle";
		return "ready";
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
			m_bUnloadBayGoalValid = false;
		}
		m_bUnloadSequenceHold = active;
		if (!active)
		{
			m_bForwardOutboundHoldRequested = false;
			m_fLostSeconds = 0;
			m_fStuckSeconds = 0;
			m_iStuckRetries = 0;
		}
	}

	// The session records the first released truck's actual unloading place.
	// A queued successor gets this road goal only after that truck is parked
	// and the bay is physically clear. Never route to an occupied/unreachable
	// point or pull into the player's lead vehicle.
	bool CF_SetUnloadBayGoal(vector bay, vector leadAnchor)
	{
		if (!Replication.IsServer() || !m_Truck || !CF_IsBoarded())
			return false;
		m_bUnloadBayGoalValid = false;
		// This is the position occupied by the first truck at unload, so a
		// second truck can safely reuse it once that truck is parked. Keep a
		// minimum center gap, but do not reject a proven 6-8 m unload bay.
		float historicalLeadGap = vector.Distance(bay, leadAnchor);
		if (historicalLeadGap < 5.0)
		{
			Print("[ConvoyFollower] UNLOAD_BAY_REJECTED: recorded bay only " +
				historicalLeadGap + " m from original lead vehicle");
			return false;
		}
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
		{
			Print("[ConvoyFollower] UNLOAD_BAY_REJECTED: road network unavailable");
			return false;
		}
		vector roadGoal;
		if (!aiWorld.GetRoadNetworkManager().GetReachableWaypointInRoad(
			m_Truck.GetOrigin(), bay, 8.0, roadGoal))
		{
			Print("[ConvoyFollower] UNLOAD_BAY_REJECTED: recorded bay is not road-reachable");
			return false;
		}
		if (vector.Distance(roadGoal, bay) > 5.0 ||
			vector.Distance(roadGoal, leadAnchor) < 5.0)
		{
			Print("[ConvoyFollower] UNLOAD_BAY_REJECTED: road goal differs from saved bay or crowds original lead; goal=" + roadGoal);
			return false;
		}
		m_vUnloadBayGoal = roadGoal;
		m_bTurnProbeOccupied = false;
		GetGame().GetWorld().QueryEntitiesBySphere(roadGoal, 4.0, ConsiderUnloadBayObstacle);
		if (m_bTurnProbeOccupied)
		{
			Print("[ConvoyFollower] UNLOAD_BAY_REJECTED: current vehicle occupies road goal " + roadGoal);
			return false;
		}
		m_bUnloadBayGoalValid = true;
		Print("[ConvoyFollower] UNLOAD_BAY_APPROACH_ASSIGNED: Unit " + m_iUnitNumber +
			" aiming for recorded cargo bay " + roadGoal);
		return true;
	}

	bool CF_IsUnloadDeparted()
	{
		return m_iState == CF_UNLOAD_DEPARTED && CF_IsBoarded();
	}

	bool CF_IsForwardWaitParked()
	{
		return m_iState == CF_FORWARD_WAIT_DEPARTED && CF_IsBoarded() &&
			CF_IsAtUnloadWaitingPoint();
	}

	bool CF_CanResumeForwardWait()
	{
		return m_iState == CF_FORWARD_WAIT_DEPARTED && CF_IsBoarded() && m_Session;
	}

	bool CF_ResumeForwardWait()
	{
		if (!Replication.IsServer() || !CF_CanResumeForwardWait())
			return false;
		m_bForwardWaitDeparture = false;
		m_bUnloadSequenceHold = false;
		m_bUnloadAnchorValid = false;
		m_bUnloadSlotBrakeCaptured = false;
		ResetLeadTrail(null);
		SetState(CF_WAITING_FOR_PREDECESSOR);
		Print("[ConvoyFollower] FORWARD_WAIT_RESUME: Unit " + m_iUnitNumber +
			" returning to its assigned chain");
		return true;
	}

	// The rear action runs its visibility check on clients, where m_Truck is
	// server-owned. The session verifies the actual parked roster on use.
	bool CF_IsReturnParkedForActions()
	{
		return m_iState == CF_UNLOAD_DEPARTED;
	}

	bool CF_IsReturnBlockedForActions()
	{
		return m_iState == CF_RETURN_BLOCKED;
	}

	bool CF_IsAtUnloadWaitingPoint()
	{
		float requiredStillSeconds = CF_UNLOAD_SLOT_STILL_SECONDS;
		if (m_bForwardWaitDeparture)
			requiredStillSeconds = CF_FORWARD_SLOT_STILL_SECONDS;
		return (m_iState == CF_UNLOAD_DEPARTING || m_iState == CF_UNLOAD_DEPARTED ||
			m_iState == CF_FORWARD_WAIT_DEPARTED) &&
			CF_IsBoarded() && m_Truck && m_iUnloadTurnPhase == 2 &&
			(!m_bForwardWaitDeparture || m_bUnloadSlotBrakeCaptured) &&
			vector.Distance(m_Truck.GetOrigin(), m_vUnloadWaitingPoint) <= CF_UNLOAD_SLOT_RADIUS + 4.0 &&
			(m_bReleaseRouteHistoryFallback || CF_IsTruckNearMappedRoad()) &&
			m_fUnloadSlotStillSeconds >= requiredStillSeconds;
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

	// Forward waiting is a distinct player order. It is deliberately refused
	// when the player's parked lead vehicle occupies the departure lane; the
	// AI must not be asked to improvise a pass through a blocked narrow road.
	bool CF_FindForwardUnloadWaitingPoint(int alreadyParked, out vector candidate)
	{
		candidate = vector.Zero;
		m_sReleasePlanFailureReason = "No clear road waiting spot ahead";
		if (!Replication.IsServer() || !m_Truck || !CF_IsBoarded() ||
			m_iState != CF_ARRIVING || alreadyParked < 0 || alreadyParked > 4)
			return false;
		m_bReleaseRouteHistoryFallback = false;
		vector travel;
		if (!TryGetOutboundTravelDirection(travel))
		{
			m_sReleasePlanFailureReason = "Drive forward before ordering a truck ahead; no approach direction is known";
			return false;
		}
		IEntity leadVehicle = m_LastPlayerVehicle;
		if (!leadVehicle)
		{
			m_sReleasePlanFailureReason = "Park the lead vehicle before ordering a truck ahead";
			return false;
		}
		vector current = m_Truck.GetOrigin();
		vector lead = leadVehicle.GetOrigin();
		vector toLead = lead - current;
		float leadAhead = toLead[0] * travel[0] + toLead[2] * travel[2];
		float leadLateral = toLead[0] * travel[2] - toLead[2] * travel[0];
		if (leadLateral < 0)
			leadLateral = -leadLateral;
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
		{
			m_sReleasePlanFailureReason = "Vehicle road navigation is unavailable here";
			return false;
		}
		RoadNetworkManager roads = aiWorld.GetRoadNetworkManager();
		BaseRoad departureRoad;
		float truckRoadDistance;
		roads.GetClosestRoad(current, departureRoad, truckRoadDistance);
		ref array<vector> departurePoints = {};
		if (departureRoad)
			departureRoad.GetPoints(departurePoints);
		float laneClearance;
		bool laneMapped = departureRoad && truckRoadDistance <= 8.0 &&
			CF_RoadPolyline.TryForwardCorridorClearance(departurePoints, current,
				travel, 30.0, lead, laneClearance);
		float nowMs = GetGame().GetWorld().GetWorldTime();
		if (nowMs >= m_fNextForwardLaneDiagnosticMs)
		{
			m_fNextForwardLaneDiagnosticMs = nowMs + 10000.0;
			Print("[ConvoyFollower] FORWARD_WAIT_LANE_SCAN: Unit " + m_iUnitNumber +
				" truck=" + current + " lead=" + lead + " travel=" + travel +
				" approach_ahead=" + leadAhead + " approach_lateral=" + leadLateral +
				" road_gap=" + truckRoadDistance + " mapped=" + laneMapped +
				" corridor_clearance=" + laneClearance + " required=8");
		}
		if (!laneMapped)
		{
			m_sReleasePlanFailureReason = "Cannot verify a clear forward road here; choose another stopping place";
			return false;
		}
		// Use the actual forward road corridor. A successor can enter the bay
		// diagonally, so its short recent approach can point toward a safely
		// shoulder-parked lead even though the forward road is clear. The same
		// eight-metre clearance still applies, including the truck-to-road join.
		if (laneClearance < 8.0)
		{
			m_sReleasePlanFailureReason = "Move your lead vehicle off the driving lane to let this truck pass";
			return false;
		}
		if (alreadyParked >= 3)
		{
			m_sReleasePlanFailureReason = "Forward waiting line is full here (three trucks); resume it or use another stop";
			return false;
		}
		// Reserve the farthest slot first, leaving space for later trucks to
		// wait closer to the bay without overtaking one another.
		// The AI can settle about five metres before the mapped waypoint.
		// Leave that stopping shortfall in each successive truck's gap.
		float distanceAhead = 85.0 - alreadyParked * 28.0;
		// Follow the actual approach road's points. A parked lead may be angled
		// toward the shoulder; projecting its facing for 85 m can miss a bend
		// even when the same road remains connected and clear.
		vector desired;
		if (!TryGetForwardRoadArcPoint(roads, current, lead, travel, distanceAhead, desired))
		{
			m_sReleasePlanFailureReason = "No mapped road continues far enough ahead; choose a clearer stopping place";
			return false;
		}
		vector roadPoint;
		bool connected = roads.GetReachableWaypointInRoad(current, desired, 8.0, roadPoint);
		Print("[ConvoyFollower] FORWARD_WAIT_ROAD_SCAN: road_arc_goal=" + desired +
			" connected=" + connected + " road_point=" + roadPoint);
		if (!connected || vector.Distance(roadPoint, desired) > 8.0)
		{
			m_sReleasePlanFailureReason = "No connected road slot ahead; choose a clearer stopping place";
			return false;
		}
		vector toSlot = roadPoint - current;
		float slotAhead = toSlot[0] * travel[0] + toSlot[2] * travel[2];
		if (slotAhead < CF_UNLOAD_BAY_CLEAR_DISTANCE + 5.0 ||
			vector.Distance(roadPoint, lead) < CF_UNLOAD_BAY_CLEAR_DISTANCE)
		{
			m_sReleasePlanFailureReason = "The forward road slot is too close to the unloading bay";
			return false;
		}
		if (!HasForwardPassingRoadWidth(roads, roadPoint))
		{
			m_sReleasePlanFailureReason = "Road too narrow to pass a parked convoy truck; choose a wider road or turnout";
			return false;
		}
		m_bTurnProbeOccupied = false;
		GetGame().GetWorld().QueryEntitiesBySphere(roadPoint, 5.5, ConsiderTurnObstacle);
		if (m_bTurnProbeOccupied)
		{
			m_sReleasePlanFailureReason = "Another vehicle occupies the forward waiting spot";
			return false;
		}
		candidate = roadPoint;
		m_sReleasePlanFailureReason = string.Empty;
		return true;
	}

	// Measure a forward slot along the same road segment that the truck used
	// at the bay. Do not enlarge the road snap radius to pick an unrelated lane.
	protected bool TryGetForwardRoadArcPoint(RoadNetworkManager roads, vector truck,
		vector lead, vector travel, float distanceAhead, out vector goal)
	{
		goal = vector.Zero;
		if (!roads)
			return false;
		BaseRoad road;
		float truckRoadDistance;
		roads.GetClosestRoad(truck, road, truckRoadDistance);
		if (!road || truckRoadDistance > 8.0)
			return false;
		ref array<vector> points = {};
		road.GetPoints(points);
		if (points.Count() < 2)
			return false;
		return CF_RoadPolyline.TryWalk(points, lead, travel, distanceAhead, goal);
	}

	// Width is a conservative nominal-road preflight, not proof that shoulders,
	// rocks, trees, or another vehicle leave a passable physical corridor.
	// Sample either side of the slot so a wide road ending at a narrow bend is
	// not treated as a safe waiting place.
	protected bool HasForwardPassingRoadWidth(RoadNetworkManager roads, vector slot)
	{
		if (!roads)
			return false;
		BaseRoad road;
		float roadDistance;
		roads.GetClosestRoad(slot, road, roadDistance);
		float slotWidth;
		if (road)
			slotWidth = road.GetWidth();
		if (!road || roadDistance > CF_FORWARD_PASS_MAX_ROAD_OFFSET ||
			slotWidth < CF_FORWARD_PASS_MIN_ROAD_WIDTH)
		{
			Print("[ConvoyFollower] FORWARD_PASS_WIDTH_REJECTED: slot=" + slot +
				" width=" + slotWidth + " road_distance=" + roadDistance);
			return false;
		}
		ref array<vector> points = {};
		road.GetPoints(points);
		vector tangent = vector.Zero;
		float closestSegmentDistance = 1000000.0;
		for (int i = 1; i < points.Count(); i++)
		{
			vector start = points[i - 1];
			vector end = points[i];
			float dx = end[0] - start[0];
			float dz = end[2] - start[2];
			float lengthSquared = dx * dx + dz * dz;
			if (lengthSquared < 0.01)
				continue;
			float progress = ((slot[0] - start[0]) * dx + (slot[2] - start[2]) * dz) / lengthSquared;
			if (progress < 0.0)
				progress = 0.0;
			else if (progress > 1.0)
				progress = 1.0;
			vector projected = Vector(start[0] + dx * progress, slot[1], start[2] + dz * progress);
			float distance = vector.DistanceXZ(slot, projected);
			if (distance >= closestSegmentDistance)
				continue;
			closestSegmentDistance = distance;
			float length = Math.Sqrt(lengthSquared);
			tangent = Vector(dx / length, 0, dz / length);
		}
		if (tangent == vector.Zero)
			return false;
		for (int direction = -1; direction <= 1; direction += 2)
		{
			float signedDistance = CF_FORWARD_PASS_SAMPLE_DISTANCE;
			if (direction < 0)
				signedDistance = -signedDistance;
			vector sample = slot + tangent * signedDistance;
			BaseRoad sampleRoad;
			float sampleDistance;
			roads.GetClosestRoad(sample, sampleRoad, sampleDistance);
			float sampleWidth;
			if (sampleRoad)
				sampleWidth = sampleRoad.GetWidth();
			if (!sampleRoad || sampleDistance > CF_FORWARD_PASS_MAX_ROAD_OFFSET ||
				sampleWidth < CF_FORWARD_PASS_MIN_ROAD_WIDTH)
			{
				Print("[ConvoyFollower] FORWARD_PASS_WIDTH_REJECTED: sample=" + sample +
					" width=" + sampleWidth +
					" road_distance=" + sampleDistance);
				return false;
			}
		}
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
	string CF_GetReleasePlanFailureReason()
	{
		return m_sReleasePlanFailureReason;
	}

	bool CF_FindReturnUnloadWaitingPoint(vector outboundTail, vector nearestParked,
		bool hasParked, bool allowRecordedFallback, out vector candidate)
	{
		candidate = vector.Zero;
		m_bReleaseRouteHistoryFallback = false;
		m_sReleasePlanFailureReason = "No clear waiting spot behind the convoy";
		if (!Replication.IsServer() || !m_Truck || !CF_IsBoarded() || m_iState != CF_ARRIVING)
			return false;
		vector travel;
		if (!TryGetOutboundTravelDirection(travel))
		{
			m_sReleasePlanFailureReason = "Drive farther before releasing; no approach route recorded";
			return false;
		}
		m_vOutboundTravelDirection = travel;
		float slotDistance = 32.0;
		if (hasParked)
		{
			slotDistance = 15.0;
			if (vector.Distance(outboundTail, nearestParked) < 27.0)
			{
				m_sReleasePlanFailureReason = "Return line is full; waiting trucks must move first";
				return false;
			}
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
		{
			m_sReleasePlanFailureReason = "Vehicle navigation is unavailable here";
			return false;
		}
		RoadNetworkManager roads = aiWorld.GetRoadNetworkManager();
		if (!roads)
		{
			m_sReleasePlanFailureReason = "Vehicle road navigation is unavailable here";
			return false;
		}

		vector roadPoint;
		if (!roads.GetReachableWaypointInRoad(m_Truck.GetOrigin(), goal, 8.0, roadPoint))
		{
			// An open apron or dirt approach may have no mapped road. For a
			// single released truck, use only a spot this truck actually drove
			// through on its approach. Multi-truck turns still require a road
			// route until their off-road navigation is proven in a live test.
			if (!allowRecordedFallback || hasParked || tailIndex < 0 ||
				vector.Distance(m_Truck.GetOrigin(), outboundTail) > 12.0)
			{
				m_sReleasePlanFailureReason = "No mapped road or proven driven path behind the convoy";
				return false;
			}
			roadPoint = goal;
			m_bReleaseRouteHistoryFallback = true;
		}
		float tailDistance = vector.Distance(roadPoint, outboundTail);
		float behindTail = (outboundTail[0] - roadPoint[0]) * travel[0] +
			(outboundTail[2] - roadPoint[2]) * travel[2];
		if (vector.Distance(roadPoint, goal) > 8.0 || tailDistance < 14.0 ||
			tailDistance > 45.0 || behindTail < 10.0 ||
			vector.Distance(roadPoint, CF_GetUnloadAnchor()) < CF_UNLOAD_BAY_CLEAR_DISTANCE)
		{
			m_sReleasePlanFailureReason = "The approach has no safe waiting spot behind the convoy";
			return false;
		}
		if (hasParked)
		{
			float aheadOfParked = (roadPoint[0] - nearestParked[0]) * travel[0] +
				(roadPoint[2] - nearestParked[2]) * travel[2];
			if (vector.Distance(roadPoint, nearestParked) < 12.0 || aheadOfParked < 8.0)
			{
				m_sReleasePlanFailureReason = "The return line blocks the next waiting spot";
				return false;
			}
		}
		m_bTurnProbeOccupied = false;
		GetGame().GetWorld().QueryEntitiesBySphere(roadPoint, 5.5, ConsiderTurnObstacle);
		if (m_bTurnProbeOccupied)
		{
			m_sReleasePlanFailureReason = "Another vehicle blocks the waiting spot";
			return false;
		}
		if (!m_bReleaseRouteHistoryFallback)
		{
			vector turnStage;
			string turnSide;
			if (!TryChooseTurnStage(travel, turnStage, turnSide))
			{
				float currentWidth;
				float slotWidth;
				if (!CF_HasWideMappedRearTurn(m_Truck.GetOrigin(), roadPoint,
					currentWidth, slotWidth))
				{
					m_sReleasePlanFailureReason = "No safe rear turnaround here; use a wider turnout or Pull ahead and wait";
					Print("[ConvoyFollower] UNLOAD_REAR_REJECTED: Unit " + m_iUnitNumber +
						" no lateral road stage; road widths truck=" + currentWidth +
						" slot=" + slotWidth + " (need " + CF_REAR_DIRECT_MIN_ROAD_WIDTH + " m)");
					return false;
				}
			}
		}

		float homeX = roadPoint[0] - outboundTail[0];
		float homeZ = roadPoint[2] - outboundTail[2];
		float homeLength = Math.Sqrt(homeX * homeX + homeZ * homeZ);
		if (homeLength < 15.0)
		{
			m_sReleasePlanFailureReason = "The waiting spot is too close to the convoy";
			return false;
		}
		m_vUnloadHomeDirection = Vector(homeX / homeLength, 0, homeZ / homeLength);
		candidate = roadPoint;
		m_sReleasePlanFailureReason = string.Empty;
		return true;
	}

	protected bool ConsiderTurnObstacle(IEntity entity)
	{
		if (entity != m_Truck && Vehicle.Cast(entity))
			m_bTurnProbeOccupied = true;
		return true;
	}

	protected bool ConsiderUnloadBayObstacle(IEntity entity)
	{
		Vehicle vehicle = Vehicle.Cast(entity);
		if (!vehicle || vehicle == m_Truck)
			return true;
		float centerGap = vector.Distance(vehicle.GetOrigin(), m_vUnloadBayGoal);
		// The first convoy truck already occupied this recorded bay safely next
		// to the unchanged lead vehicle. A broad entity-bounds query can still
		// overlap that lead at ~7 m center separation. Exempt only that exact
		// lead vehicle while its center remains clear of the bay; every other
		// vehicle, or a lead that moved into the bay, blocks the approach.
		if (vehicle == m_UnloadAnchorVehicle && centerGap >= 6.0)
		{
			Print("[ConvoyFollower] UNLOAD_BAY_LEAD_EDGE: ignoring original lead bounds at " +
				vehicle.GetOrigin() + " center_gap=" + centerGap);
			return true;
		}
		m_bTurnProbeOccupied = true;
		Print("[ConvoyFollower] UNLOAD_BAY_OCCUPANT: vehicle at " + vehicle.GetOrigin() +
			" center_gap=" + centerGap + " is_original_lead=" + (vehicle == m_UnloadAnchorVehicle));
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

	protected bool CF_HasWideMappedRearTurn(vector current, vector slot,
		out float currentWidth, out float slotWidth)
	{
		currentWidth = 0;
		slotWidth = 0;
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return false;
		RoadNetworkManager roads = aiWorld.GetRoadNetworkManager();
		BaseRoad currentRoad;
		BaseRoad slotRoad;
		float currentOffset;
		float slotOffset;
		if (roads.GetClosestRoad(current, currentRoad, currentOffset) <= 0 || !currentRoad ||
			roads.GetClosestRoad(slot, slotRoad, slotOffset) <= 0 || !slotRoad)
			return false;
		currentWidth = currentRoad.GetWidth();
		slotWidth = slotRoad.GetWidth();
		return currentOffset <= CF_REAR_DIRECT_MAX_ROAD_OFFSET &&
			slotOffset <= CF_REAR_DIRECT_MAX_ROAD_OFFSET &&
			currentWidth >= CF_REAR_DIRECT_MIN_ROAD_WIDTH &&
			slotWidth >= CF_REAR_DIRECT_MIN_ROAD_WIDTH;
	}

	bool CF_BeginUnloadDeparture(vector waitingPoint)
	{
		return CF_BeginDeparture(waitingPoint, false);
	}

	bool CF_BeginForwardDeparture(vector waitingPoint)
	{
		return CF_BeginDeparture(waitingPoint, true);
	}

	protected bool CF_BeginDeparture(vector waitingPoint, bool forwardWait)
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
		if ((!forwardWait && behindCurrent < 12.0) ||
			(forwardWait && behindCurrent > -CF_UNLOAD_BAY_CLEAR_DISTANCE) ||
			vector.Distance(waitingPoint, anchor) < CF_UNLOAD_BAY_CLEAR_DISTANCE)
			return false;
		vector stage;
		string side;
		bool stagedTurn = !forwardWait && TryChooseTurnStage(travel, stage, side);
		if (!forwardWait && !stagedTurn && !m_bReleaseRouteHistoryFallback)
		{
			float currentWidth;
			float slotWidth;
			if (!CF_HasWideMappedRearTurn(current, waitingPoint,
				currentWidth, slotWidth))
			{
				m_sReleasePlanFailureReason = "No safe rear turnaround here; use a wider turnout or Pull ahead and wait";
				Print("[ConvoyFollower] UNLOAD_REAR_REJECTED: Unit " + m_iUnitNumber +
					" departure changed; road widths truck=" + currentWidth +
					" slot=" + slotWidth);
				return false;
			}
		}

		m_bForwardWaitDeparture = forwardWait;
		m_vUnloadAnchor = anchor;
		m_bUnloadAnchorValid = true;
		m_vOutboundTravelDirection = travel;
		m_vUnloadWaitingPoint = waitingPoint;
		m_vUnloadTurnStage = stage;
		m_iUnloadTurnPhase = 1;
		m_fTurnStageSeconds = 0;
		m_fUnloadDepartureDiagnosticSeconds = 0;
		m_fUnloadDepartureElapsedSeconds = 0;
		m_fUnloadSlotProgressSeconds = 0;
		m_fUnloadSlotProgressGap = vector.Distance(current, waitingPoint);
		m_fUnloadBestSlotGap = m_fUnloadSlotProgressGap;
		m_fUnloadNoBestProgressSeconds = 0;
		m_bUnloadSlotBrakeCaptured = false;
		m_bUnloadIntermediateActive = false;
		m_vUnloadSlotProgressPosition = current;
		m_iUnloadSlotReissues = 0;
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
			m_bForwardWaitDeparture = false;
			SetState(CF_ARRIVING);
			SetUnloadReleaseReady(true);
			return false;
		}
		SCR_AIWaypoint waypoint = SCR_AIWaypoint.Cast(m_Waypoint);
		if (waypoint)
		{
			if (stagedTurn)
				waypoint.SetCompletionRadius(CF_TURN_STAGE_RADIUS);
			else if (forwardWait)
				waypoint.SetCompletionRadius(CF_FORWARD_SLOT_WAYPOINT_RADIUS);
			else
				waypoint.SetCompletionRadius(CF_UNLOAD_SLOT_RADIUS);
		}
		if (forwardWait)
			Print("[ConvoyFollower] FORWARD_WAIT_REQUESTED: Unit " + m_iUnitNumber +
				" driving to connected road slot " + waitingPoint);
		else if (stagedTurn)
			Print("[ConvoyFollower] UNLOAD_TURN_STAGE: Unit " + m_iUnitNumber + " taking clear " + side + " road stage before return slot");
		else
			Print("[ConvoyFollower] UNLOAD_DIRECT_ROUTE: Unit " + m_iUnitNumber + " routing to validated return slot without a lateral road stage");
		if (m_bReleaseRouteHistoryFallback)
			Print("[ConvoyFollower] UNLOAD_ROUTE_HISTORY_FALLBACK: Unit " + m_iUnitNumber + " attempting recorded off-road approach; bay clearance remains unverified");
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

	// A forward waiting line is a deliberate stop, including any trucks that
	// have not been released. If the front truck is still approaching the bay,
	// keep its MOVE order until it is close and slow; never call it stalled
	// because the owner is driving past the parked-ahead trucks.
	void CF_RequestForwardOutboundHold()
	{
		if (!Replication.IsServer() || !m_Session || !CF_IsBoarded())
			return;
		m_bForwardOutboundHoldRequested = true;
		m_fLostSeconds = 0;
		m_fStuckSeconds = 0;
		m_iStuckRetries = 0;
		CF_TryCompleteForwardOutboundHold();
	}

	protected void CF_TryCompleteForwardOutboundHold()
	{
		if (!m_bForwardOutboundHoldRequested || !CF_IsBoarded() ||
			m_iState == CF_FORWARD_OUTBOUND_HOLD)
			return;
		if (m_iState != CF_UNLOAD_QUEUE)
		{
			if (m_iState != CF_ARRIVING || !m_bUnloadBayGoalValid ||
				vector.Distance(m_Truck.GetOrigin(), m_vUnloadBayGoal) >
				CF_UNLOAD_BAY_READY_EXIT_RADIUS + 3.0 ||
				!CF_IsPanelVehicleSlow(10.0))
				return;
		}
		ClearWaypoints();
		SetState(CF_FORWARD_OUTBOUND_HOLD);
		Print("[ConvoyFollower] FORWARD_OUTBOUND_HOLD: Unit " + m_iUnitNumber +
			" waiting seated at the unload line until explicit resume");
	}

	bool CF_IsForwardOutboundHeld()
	{
		return m_iState == CF_FORWARD_OUTBOUND_HOLD && CF_IsBoarded();
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
		if (m_iState != CF_UNLOAD_QUEUE && m_iState != CF_FORWARD_OUTBOUND_HOLD)
			return;
		m_fStateSeconds = 0;
		SetState(CF_WAITING_FOR_PREDECESSOR);
		Print("[ConvoyFollower] UNLOAD_QUEUE_RESUME: Unit " + m_iUnitNumber + " waiting for convoy target");
	}

	void CF_CompleteUnloadDeparture()
	{
		if (!Replication.IsServer() || m_iState != CF_UNLOAD_DEPARTING || !CF_IsAtUnloadWaitingPoint())
			return;
		// The session records the chosen waiting side. A forward wait remains
		// separate from the rear return line and cannot auto-rejoin on a
		// homeward crossing.
		ClearWaypoints();
		m_Predecessor = null;
		m_bUnloadSequenceHold = false;
		if (m_bForwardWaitDeparture)
		{
			SetState(CF_FORWARD_WAIT_DEPARTED);
			m_vForwardParkedPosition = m_Truck.GetOrigin();
			m_fForwardParkedDiagnosticSeconds = 0;
			Print("[ConvoyFollower] FORWARD_WAIT_PARKED: former Unit " + m_iUnitNumber +
				" seated ahead; explicit owner resume required");
			CF_LogForwardParkAlignment("parked");
		}
		else
		{
			SetState(CF_UNLOAD_DEPARTED);
			Print("[ConvoyFollower] RETURN_PARKED: former Unit " + m_iUnitNumber + " seated behind convoy tail");
		}
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
	// the active chain. A parked truck may still face outbound, so its heading
	// must be verified before ordinary following resumes.
	bool CF_CanBeginReturnFollow()
	{
		Resource movePrefab = Resource.Load(CF_MOVE_WAYPOINT);
		return m_Session && m_Group && m_iState == CF_UNLOAD_DEPARTED &&
			CF_IsAtUnloadWaitingPoint() && movePrefab.IsValid() &&
			m_vUnloadHomeDirection[0] * m_vUnloadHomeDirection[0] +
			m_vUnloadHomeDirection[2] * m_vUnloadHomeDirection[2] >= 0.5;
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
		m_bReturnTurnFailed = false;
		m_bReturnWaitForOwnerPass = !predecessor;
		m_bReturnPassStartValid = false;
		m_ReturnPassVehicle = null;
		m_vReturnHomeDirection = m_vUnloadHomeDirection;
		m_vReturnTurnStartPosition = m_Truck.GetOrigin();
		m_iReturnTurnPhase = 0;
		m_iReturnGoalReissues = 0;
		m_fReturnNavLogSeconds = 0;
		m_fReturnGoalRetrySeconds = 0;
		ResetLeadTrail(null);
		ClearWaypoints();
		SetState(CF_RETURN_WAIT);
		Print("[ConvoyFollower] RETURN_REJOIN_WAIT: parked Unit " + unitNumber +
			" checking its own homeward heading before following");
		return true;
	}

	// An unreleased outbound truck remains seated while the explicit return
	// event converts it from unload/arrival duty into a staged homeward turn.
	// The normal lost-distance rule is suspended until that turn completes.
	bool CF_AbortUnloadForReturn(IEntity currentLeader, vector homeDirection)
	{
		if (!Replication.IsServer() || !currentLeader || !CF_CanAbortUnloadForReturn() ||
			homeDirection[0] * homeDirection[0] + homeDirection[2] * homeDirection[2] < 0.5)
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
		m_bReturnWaitForOwnerPass = false;
		m_bReturnPassStartValid = false;
		m_ReturnPassVehicle = null;
		// The truck may have rotated while lining up at the bay. Use the
		// session's validated return-line direction, not an inverted snapshot
		// of whichever way this truck happens to face now.
		m_vReturnHomeDirection = homeDirection;
		m_vReturnTurnStartPosition = m_Truck.GetOrigin();
		m_fReturnMergeGraceSeconds = CF_RETURN_MERGE_GRACE_SECONDS;
		m_iReturnTurnPhase = 0;
		m_iReturnGoalReissues = 0;
		m_fTurnStageSeconds = 0;
		m_fReturnNavLogSeconds = 0;
		m_fReturnGoalRetrySeconds = 0;
		ResetLeadTrail(null);
		ClearWaypoints();
		SetState(CF_RETURN_WAIT);
		Print("[ConvoyFollower] RETURN_TURN_WAIT: outbound Unit " + m_iUnitNumber + " waiting for homeward predecessor");
		return true;
	}

	// A failed homeward turn is held seated, not dismissed. The rear-menu
	// retry starts a fresh bounded turn using the predecessor's current
	// heading; it never reinstates the old unload queue.
	bool CF_RetryBlockedReturn()
	{
		if (!Replication.IsServer() || !m_Session || m_iState != CF_RETURN_BLOCKED || !CF_IsBoarded())
			return false;
		ClearWaypoints();
		m_bReturnTurnFailed = false;
		m_iReturnTurnPhase = 0;
		m_iReturnGoalReissues = 0;
		m_fTurnStageSeconds = 0;
		m_fStateSeconds = 0;
		m_fReturnNavLogSeconds = 0;
		m_fReturnGoalRetrySeconds = 0;
		m_vReturnTurnStartPosition = m_Truck.GetOrigin();
		m_fReturnMergeGraceSeconds = CF_RETURN_MERGE_GRACE_SECONDS;
		SetState(CF_RETURN_WAIT);
		Print("[ConvoyFollower] RETURN_TURN_RETRY: Unit " + m_iUnitNumber + " waiting for a safe homeward turn");
		return true;
	}

	protected void FailUnloadDeparture(string reason)
	{
		if (m_bUnloadClearReported)
			return;
		m_bUnloadClearReported = true;
		ClearWaypoints();
		SetState(CF_UNLOAD_QUEUE);
		if (m_bForwardWaitDeparture)
			Print("[ConvoyFollower] FORWARD_WAIT_BLOCKED: Unit " + m_iUnitNumber +
				" holding seated: " + reason);
		else
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
		if (m_Session)
			m_Session.CF_NotifyOwnerFailure(CF_ConvoyFailureEvent.RETURN_TURN_BLOCKED);
	}

	protected bool CF_HasOwnerPassedForReturn(IEntity targetVehicle)
	{
		if (!m_bReturnWaitForOwnerPass)
			return true;
		if (!targetVehicle || !m_Truck)
			return false;
		// A narrow bend can turn the owner's truck away from home while it is
		// already travelling past the return line. Arm on this vehicle's first
		// observed position, then use actual homeward displacement to recognize
		// the crossing instead of waiting for a favorable heading snapshot.
		if (targetVehicle != m_ReturnPassVehicle || !m_bReturnPassStartValid)
		{
			m_ReturnPassVehicle = targetVehicle;
			m_vReturnPassStartPosition = targetVehicle.GetOrigin();
			m_bReturnPassStartValid = true;
			return false;
		}
		vector relative = targetVehicle.GetOrigin() - m_Truck.GetOrigin();
		float ahead = relative[0] * m_vReturnHomeDirection[0] +
			relative[2] * m_vReturnHomeDirection[2];
		float lateral = relative[0] * m_vReturnHomeDirection[2] -
			relative[2] * m_vReturnHomeDirection[0];
		if (lateral < 0)
			lateral = -lateral;
		vector travel = targetVehicle.GetOrigin() - m_vReturnPassStartPosition;
		float homewardTravel = travel[0] * m_vReturnHomeDirection[0] +
			travel[2] * m_vReturnHomeDirection[2];
		if (ahead < CF_RETURN_OWNER_PASS_DISTANCE || lateral > CF_RETURN_OWNER_PASS_CORRIDOR ||
			homewardTravel < CF_RETURN_OWNER_PASS_DISTANCE)
			return false;
		m_bReturnWaitForOwnerPass = false;
		Print("[ConvoyFollower] RETURN_OWNER_PASSED: Unit " + m_iUnitNumber +
			" has a lead truck " + ahead + " m ahead after " + homewardTravel +
			" m homeward travel");
		return true;
	}

	// A lateral road stage is optional on a narrow road. Give the vehicle AI a
	// connected point past its predecessor, rather than a near goal that a
	// 20 m MOVE radius would complete before the truck could turn.
	protected bool CF_TryGetReturnRoadGoal(IEntity targetVehicle, out vector roadGoal)
	{
		roadGoal = vector.Zero;
		if (!targetVehicle || !m_Truck)
			return false;
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return false;
		RoadNetworkManager roads = aiWorld.GetRoadNetworkManager();
		vector desired = targetVehicle.GetOrigin();
		float lookahead = CF_RETURN_ROAD_GOAL_LOOKAHEAD + m_iReturnGoalReissues * 15.0;
		desired[0] = desired[0] + m_vReturnHomeDirection[0] * lookahead;
		desired[2] = desired[2] + m_vReturnHomeDirection[2] * lookahead;
		if (roads.GetReachableWaypointInRoad(m_Truck.GetOrigin(), desired, 8.0, roadGoal) &&
			vector.Distance(roadGoal, desired) <= 10.0 &&
			vector.Distance(roadGoal, m_Truck.GetOrigin()) >= CF_ConvoySettings.Get().m_fMovingGap + 5.0)
			return true;
		// On a bend, the projected point may lie off the mapped road. The
		// predecessor itself is on a travelled segment; use that road point
		// only after it has opened a useful following gap.
		vector targetRoadPoint;
		if (!roads.GetReachableWaypointInRoad(m_Truck.GetOrigin(), targetVehicle.GetOrigin(),
			6.0, targetRoadPoint))
			return false;
		if (vector.Distance(targetRoadPoint, targetVehicle.GetOrigin()) > 6.0 ||
			vector.Distance(targetRoadPoint, m_Truck.GetOrigin()) < CF_ConvoySettings.Get().m_fMovingGap + 8.0)
			return false;
		roadGoal = targetRoadPoint;
		return true;
	}

	protected void CF_LogReturnNavigation(float elapsed)
	{
		m_fReturnNavLogSeconds += elapsed;
		if (m_fReturnNavLogSeconds < CF_RETURN_NAV_LOG_SECONDS)
			return;
		m_fReturnNavLogSeconds = 0;
		IEntity targetVehicle = GetTargetVehicle();
		vector targetPosition = vector.Zero;
		if (targetVehicle)
			targetPosition = targetVehicle.GetOrigin();
		vector truckFacing;
		TryGetVehicleFacing(m_Truck, truckFacing);
		float facingDot = truckFacing[0] * m_vReturnHomeDirection[0] +
			truckFacing[2] * m_vReturnHomeDirection[2];
		AIWaypoint activeWaypoint = m_Group.GetCurrentWaypoint();
		vector activeGoal = vector.Zero;
		if (activeWaypoint)
			activeGoal = activeWaypoint.GetOrigin();
		Print("[ConvoyFollower] RETURN_NAV_STATUS: Unit " + m_iUnitNumber +
			" state=" + m_iState + " phase=" + m_iReturnTurnPhase +
			" origin=" + m_Truck.GetOrigin() + " target=" + targetPosition +
			" facing_dot=" + facingDot + " progress=" +
			vector.Distance(m_Truck.GetOrigin(), m_vReturnTurnStartPosition) +
			" waiting_owner=" + m_bReturnWaitForOwnerPass +
			" own_waypoint=" + HasOwnWaypointInGroup() + " active_goal=" + activeGoal);
	}

	// A direct MOVE to a distant moving vehicle can choose another branch at a
	// junction. Keep the predecessor's actual traveled points and consume them
	// in order, so every truck receives the same local turns. The cursor never
	// moves backward; a new target or return leg starts a new trail.
	protected void ResetLeadTrail(IEntity target)
	{
		m_aLeadTrailPositions.Clear();
		m_LeadTrailTarget = target;
		m_iLeadTrailCursor = 0;
		m_bLeadTrailGuidanceLogged = false;
		if (target)
			m_aLeadTrailPositions.Insert(target.GetOrigin());
	}

	protected bool CF_IsTruckNearMappedRoad()
	{
		if (!m_Truck)
			return false;
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return false;
		BaseRoad nearestRoad;
		float roadDistance;
		if (aiWorld.GetRoadNetworkManager().GetClosestRoad(m_Truck.GetOrigin(),
			nearestRoad, roadDistance) <= 0 || !nearestRoad)
			return false;
		// Vehicle AI may finish a four-metre MOVE a little past its mapped
		// centerline. Allow only a small shoulder margin on a wide road; the
		// former five-metre guard remains on narrow roads.
		float allowedDistance = nearestRoad.GetWidth() * 0.5 + CF_STOPPED_ROAD_SHOULDER_BUFFER;
		if (allowedDistance < CF_STOPPED_ROAD_CLOSE_DISTANCE)
			allowedDistance = CF_STOPPED_ROAD_CLOSE_DISTANCE;
		if (allowedDistance > CF_STOPPED_ROAD_MAX_DISTANCE)
			allowedDistance = CF_STOPPED_ROAD_MAX_DISTANCE;
		return roadDistance <= allowedDistance;
	}

	protected bool CF_IsBeyondMappedRoad(IEntity vehicle)
	{
		if (!vehicle)
			return false;
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return false;
		BaseRoad road;
		float distance;
		int roadId = aiWorld.GetRoadNetworkManager().GetClosestRoad(vehicle.GetOrigin(), road, distance);
		return roadId >= 0 && road && road.GetWidth() > 0 && distance > road.GetWidth() * 0.5 + 8.0;
	}

	protected bool CF_UsesOrdinaryOffNetworkTrail()
	{
		return !m_bUnloadSequenceHold && CF_IsBeyondMappedRoad(m_Truck) &&
			CF_IsBeyondMappedRoad(m_LeadVehicle);
	}

	protected bool CF_IsOrdinaryTrailArrivalClose()
	{
		if (!m_Truck || !m_LeadVehicle || !m_bArrivalTrailGoalValid)
			return false;
		float separation = vector.DistanceXZ(m_Truck.GetOrigin(), m_LeadVehicle.GetOrigin());
		return separation >= 7.0 && separation <= CF_ConvoySettings.Get().m_fStoppedGap + 8.0 &&
			vector.DistanceXZ(m_Truck.GetOrigin(), m_vArrivalTrailGoal) <= 8.0;
	}

	protected void CF_LogFollowWait(string cause)
	{
		if (!m_Truck || !GetGame() || !GetGame().GetWorld())
			return;
		float now = GetGame().GetWorld().GetWorldTime();
		if (now < m_fNextFollowWaitDiagnosticMs)
			return;
		m_fNextFollowWaitDiagnosticMs = now + 5000.0;
		string target = "none";
		float gap = -1;
		if (m_LeadVehicle)
		{
			target = m_LeadVehicle.GetName();
			gap = vector.DistanceXZ(m_Truck.GetOrigin(), m_LeadVehicle.GetOrigin());
		}
		float speed = -1;
		float brake = -1;
		float throttle = -1;
		bool engine;
		CarControllerComponent car = CarControllerComponent.Cast(m_Truck.FindComponent(CarControllerComponent));
		if (car && car.GetSimulation())
		{
			VehicleWheeledSimulation sim = car.GetSimulation();
			speed = sim.GetSpeedKmh();
			brake = sim.GetBrake();
			throttle = sim.GetThrottle();
			engine = sim.EngineIsOn();
		}
		float radius = -1;
		if (m_Waypoint)
			radius = m_Waypoint.GetCompletionRadius();
		Print("[ConvoyFollower] FOLLOW_LINK_STATUS: Unit " + m_iUnitNumber + " target=" + target +
			" state=" + m_iState + " cause=" + cause + " speed_kmh=" + speed +
			" gap=" + gap + " waypoint=" + HasOwnWaypointInGroup() + " radius=" + radius +
			" waypoint_age_s=" + m_fWaypointSeconds + " engine=" + engine + " brake=" + brake +
			" throttle=" + throttle + " own_brake=" + m_bOwnVehicleBrake +
			" goal=" + m_vLastWaypointPosition);
	}

	// Near a stopped predecessor, aiming at its exact center across a junction
	// made trucks cut the corner and settle well off the mapped road. Keep the
	// final road MOVE on a point the predecessor actually drove through.
	protected void CF_LogStoppedTrailGoalReject(string reason, string detail)
	{
		if (!Replication.IsServer() || !GetGame() || !GetGame().GetWorld())
			return;
		float nowMs = GetGame().GetWorld().GetWorldTime();
		if (nowMs < m_fNextStoppedTrailDiagnosticMs)
			return;
		m_fNextStoppedTrailDiagnosticMs = nowMs + CF_STOPPED_TRAIL_DIAGNOSTIC_INTERVAL_MS;
		Print("[ConvoyFollower] STOPPED_TRAIL_GOAL_REJECT: Unit " + m_iUnitNumber +
			" reason=" + reason + " samples=" + m_aLeadTrailPositions.Count() +
			" has_truck=" + (m_Truck != null) +
			" has_trail_target=" + (m_LeadTrailTarget != null) + " " + detail);
	}

	protected bool CF_TryGetStoppedTrailPoint(IEntity target, out vector trailGoal)
	{
		trailGoal = vector.Zero;
		if (!target || !m_Truck || target != m_LeadTrailTarget ||
			m_aLeadTrailPositions.Count() < 2)
		{
			CF_LogStoppedTrailGoalReject("MISSING_TRAIL", "requested_target_present=" +
				(target != null) + " target_matches_trail=" + (target == m_LeadTrailTarget));
			return false;
		}
		float desiredBack = CF_ConvoySettings.Get().m_fStoppedGap;
		if (desiredBack < 10.0)
			desiredBack = 10.0;
		vector recent = target.GetOrigin();
		float routeBack = 0;
		for (int i = m_aLeadTrailPositions.Count() - 1; i >= 0; i--)
		{
			vector previous = m_aLeadTrailPositions[i];
			float segment = vector.Distance(recent, previous);
			if (segment < 0.1)
			{
				recent = previous;
				continue;
			}
			if (routeBack + segment < desiredBack)
			{
				routeBack += segment;
				recent = previous;
				continue;
			}
			// Samples are about six metres apart. Choosing the first sample
			// beyond the gap left 18-20 m behind a stopped lead; the MOVE then
			// completed before the cargo truck could reach release range.
			float fraction = (desiredBack - routeBack) / segment;
			vector candidate = recent + (previous - recent) * fraction;
			float targetDistance = vector.Distance(candidate, target.GetOrigin());
			if (targetDistance < 8.0 || targetDistance > CF_STOPPED_TRAIL_MAX_TARGET_DISTANCE)
			{
				CF_LogStoppedTrailGoalReject("CANDIDATE_GEOMETRY", "candidate=" + candidate +
					" target_gap=" + targetDistance + " route_back=" + routeBack +
					" desired_back=" + desiredBack);
				return false;
			}
			trailGoal = candidate;
			return true;
		}
		CF_LogStoppedTrailGoalReject("INSUFFICIENT_ROUTE", "route_back=" + routeBack +
			" desired_back=" + desiredBack);
		return false;
	}

	protected bool CF_TryGetStoppedTrailRoadGoal(IEntity target, out vector roadGoal)
	{
		roadGoal = vector.Zero;
		vector candidate;
		if (!CF_TryGetStoppedTrailPoint(target, candidate))
			return false;
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
		{
			CF_LogStoppedTrailGoalReject("NO_ROAD_NETWORK", "");
			return false;
		}
		if (!aiWorld.GetRoadNetworkManager().GetReachableWaypointInRoad(m_Truck.GetOrigin(), candidate, 6.0, roadGoal))
		{
			CF_LogStoppedTrailGoalReject("ROAD_UNREACHABLE", "candidate=" + candidate + " truck=" + m_Truck.GetOrigin());
			return false;
		}
		float projectedGap = vector.Distance(roadGoal, candidate);
		float projectedTargetGap = vector.Distance(roadGoal, target.GetOrigin());
		if (projectedGap > 6.0 || projectedTargetGap < 8.0)
			CF_LogStoppedTrailGoalReject("PROJECTED_GAP", "candidate=" + candidate + " road_goal=" + roadGoal +
				" projection_gap=" + projectedGap + " target_gap=" + projectedTargetGap);
		return projectedGap <= 6.0 && projectedTargetGap >= 8.0;
	}

	protected bool CF_TryGetOrdinaryStoppedTrailGoal(IEntity target, out vector goal)
	{
		if (!CF_UsesOrdinaryOffNetworkTrail())
		{
			if (m_bArrivalTrailMode)
			{
				m_bArrivalTrailMode = false;
				m_bArrivalTrailHold = false;
				m_bArrivalTrailGoalValid = false;
				m_bStopSettleIssued = false;
				m_bArrivalCloseLogged = false;
			}
			return CF_TryGetStoppedTrailRoadGoal(target, goal);
		}
		m_bArrivalTrailMode = true;
		m_bArrivalTrailGoalValid = false;
		if (!CF_TryGetStoppedTrailPoint(target, goal))
		{
			m_bArrivalTrailHold = false;
			return false;
		}
		m_vArrivalTrailGoal = goal;
		m_bArrivalTrailGoalValid = true;
		return true;
	}

	protected vector GetLeadTrailGoal(IEntity target)
	{
		if (!target)
			return vector.Zero;
		if (m_iState == CF_ARRIVING && m_bUnloadSequenceHold && m_bUnloadBayGoalValid)
			return m_vUnloadBayGoal;
		if (target != m_LeadTrailTarget || m_aLeadTrailPositions.IsEmpty())
			ResetLeadTrail(target);

		vector currentTargetPosition = target.GetOrigin();
		vector latestSample = m_aLeadTrailPositions[m_aLeadTrailPositions.Count() - 1];
		if (vector.Distance(currentTargetPosition, latestSample) >= CF_LEAD_TRAIL_SAMPLE_DISTANCE)
		{
			m_aLeadTrailPositions.Insert(currentTargetPosition);
			if (m_aLeadTrailPositions.Count() > CF_LEAD_TRAIL_SAMPLE_LIMIT)
			{
				m_aLeadTrailPositions.RemoveOrdered(0);
				if (m_iLeadTrailCursor > 0)
					m_iLeadTrailCursor--;
			}
		}

		// Following a stop and reboard, the engine may consume a near MOVE while
		// the truck coasts beyond the cursor's sample. The old cursor then stays
		// behind it, so the 36 m lookahead can repeatedly select the same
		// already-completed waypoint even as the predecessor drives away. Find
		// the first later sample under the truck and resume from there. Do not
		// jump straight to the moving target across a road junction.
		if (m_iLeadTrailCursor < m_aLeadTrailPositions.Count() - 1 &&
			vector.Distance(m_Truck.GetOrigin(), m_aLeadTrailPositions[m_iLeadTrailCursor]) >
			CF_LEAD_TRAIL_REACHED_DISTANCE)
		{
			for (int sampleIndex = m_iLeadTrailCursor + 1; sampleIndex < m_aLeadTrailPositions.Count(); sampleIndex++)
			{
				if (vector.Distance(m_Truck.GetOrigin(), m_aLeadTrailPositions[sampleIndex]) >
					CF_LEAD_TRAIL_RESYNC_DISTANCE)
					continue;
				Print("[ConvoyFollower] FOLLOW_TRAIL_RESYNC: Unit " + m_iUnitNumber +
					" route point " + m_iLeadTrailCursor + " to " + sampleIndex);
				m_iLeadTrailCursor = sampleIndex;
				break;
			}
		}

		// The moving gap itself is wide enough to consume nearby samples. A
		// point farther along the trail remains the next road-routed MOVE goal.
		while (m_iLeadTrailCursor < m_aLeadTrailPositions.Count() - 1 &&
			vector.Distance(m_Truck.GetOrigin(), m_aLeadTrailPositions[m_iLeadTrailCursor]) <=
			CF_LEAD_TRAIL_REACHED_DISTANCE)
			m_iLeadTrailCursor++;
		int goalIndex = m_iLeadTrailCursor;
		float routeLookahead = 0;
		while (goalIndex < m_aLeadTrailPositions.Count() - 1 &&
			routeLookahead < CF_LEAD_TRAIL_LOOKAHEAD_DISTANCE)
		{
			routeLookahead += vector.Distance(m_aLeadTrailPositions[goalIndex],
				m_aLeadTrailPositions[goalIndex + 1]);
			goalIndex++;
		}
		// Keep a distant MOVE active while crossing samples; otherwise the AI
		// brakes at every 6 m trail point and a truck cannot keep road speed.
		vector goal = m_aLeadTrailPositions[goalIndex];
		if (goalIndex == m_aLeadTrailPositions.Count() - 1)
			goal = currentTargetPosition;
		if (goalIndex == m_aLeadTrailPositions.Count() - 1 &&
			m_fTargetStillSeconds >= CF_STOP_DETECT_SECONDS)
		{
			vector stoppedRoadGoal;
			if (CF_TryGetOrdinaryStoppedTrailGoal(target, stoppedRoadGoal))
				goal = stoppedRoadGoal;
		}

		float remainingTrail = vector.Distance(goal, currentTargetPosition);
		if (remainingTrail >= 30.0 && !m_bLeadTrailGuidanceLogged)
		{
			m_bLeadTrailGuidanceLogged = true;
			Print("[ConvoyFollower] FOLLOW_TRAIL_GUIDE: Unit " + m_iUnitNumber +
				" using predecessor's driven route, live target " + remainingTrail + " m ahead");
		}
		else if (remainingTrail <= 15.0)
			m_bLeadTrailGuidanceLogged = false;
		return goal;
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
		if (m_bForwardWaitDeparture)
		{
			// Per-poll motion below one metre still let a truck roll several
			// metres downhill while the old timer called it stationary.
			if (m_fUnloadSlotStillSeconds <= 0)
				m_vUnloadStillnessAnchor = current;
			if (vector.Distance(current, m_vUnloadStillnessAnchor) > CF_FORWARD_SLOT_STILL_MOVEMENT)
			{
				m_vUnloadStillnessAnchor = current;
				m_fUnloadSlotStillSeconds = 0;
			}
			else
				m_fUnloadSlotStillSeconds += elapsed;
		}
		else if (vector.Distance(current, m_vLastUnloadTruckPosition) < 1.0)
			m_fUnloadSlotStillSeconds += elapsed;
		else
			m_fUnloadSlotStillSeconds = 0;
		m_vLastUnloadTruckPosition = current;
	}

	protected void CF_ResetUnloadSlotProgressSample()
	{
		m_fUnloadSlotProgressSeconds = 0;
		m_vUnloadSlotProgressPosition = m_Truck.GetOrigin();
		m_fUnloadSlotProgressGap = vector.Distance(m_vUnloadSlotProgressPosition, m_vUnloadWaitingPoint);
	}

	protected void CF_ResetUnloadBestProgress()
	{
		m_fUnloadBestSlotGap = vector.Distance(m_Truck.GetOrigin(), m_vUnloadWaitingPoint);
		m_fUnloadNoBestProgressSeconds = 0;
		m_bUnloadIntermediateActive = false;
	}

	// A full slot waypoint sometimes makes a truck circle around a fork. On a
	// bounded retry, ask the road graph for a shorter connected point toward
	// the same validated slot. Never use an unvalidated off-road point.
	protected bool CF_TryIssueUnloadIntermediate()
	{
		if (!m_Truck || m_bReleaseRouteHistoryFallback)
			return false;
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return false;
		vector current = m_Truck.GetOrigin();
		float dx = m_vUnloadWaitingPoint[0] - current[0];
		float dz = m_vUnloadWaitingPoint[2] - current[2];
		float gap = Math.Sqrt(dx * dx + dz * dz);
		if (gap < 24.0)
			return false;
		float step = 20.0;
		if (step > gap - 8.0)
			step = gap - 8.0;
		vector desired = current;
		desired[0] = current[0] + dx / gap * step;
		desired[2] = current[2] + dz / gap * step;
		vector roadPoint;
		RoadNetworkManager roads = aiWorld.GetRoadNetworkManager();
		if (!roads.GetReachableWaypointInRoad(current, desired, 6.0, roadPoint) ||
			vector.Distance(roadPoint, desired) > 6.0 ||
			vector.Distance(roadPoint, current) < 10.0 ||
			vector.Distance(roadPoint, m_vUnloadWaitingPoint) > gap - 6.0)
			return false;
		m_bTurnProbeOccupied = false;
		GetGame().GetWorld().QueryEntitiesBySphere(roadPoint, 5.5, ConsiderTurnObstacle);
		if (m_bTurnProbeOccupied || !IssueMoveWaypoint(roadPoint))
			return false;
		m_vUnloadIntermediateGoal = roadPoint;
		m_bUnloadIntermediateActive = true;
		SCR_AIWaypoint waypoint = SCR_AIWaypoint.Cast(m_Waypoint);
		if (waypoint)
			waypoint.SetCompletionRadius(CF_TURN_STAGE_RADIUS);
		Print("[ConvoyFollower] UNLOAD_ROUTE_INTERMEDIATE: Unit " + m_iUnitNumber +
			" road goal " + roadPoint + " before validated slot " + m_vUnloadWaitingPoint);
		return true;
	}

	void CF_ReportUnderFire()
	{
		if (!Replication.IsServer() || !CF_IsOwnedSeatedConvoyMember())
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
		CF_ResetArrivalRoadRecovery();
		m_iStuckRetries = 0;
		m_bOrderInversionLogged = false;
		if (m_iState == CF_RETURN_WAIT || m_iState == CF_RETURN_TURNING || m_iState == CF_RETURN_BLOCKED)
		{
			m_iReturnTurnPhase = 0;
			m_iReturnGoalReissues = 0;
			m_bReturnTurnFailed = false;
			SetState(CF_RETURN_WAIT);
			Print("[ConvoyFollower] RETURN_RETARGET: Unit " + m_iUnitNumber + " waiting for new homeward predecessor");
			return;
		}
		if (m_iState != CF_PAUSED_ON_FOOT && m_iState != CF_UNLOAD_QUEUE &&
			m_iState != CF_FORWARD_OUTBOUND_HOLD)
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
		CF_ResetArrivalRoadRecovery();
		m_iStuckRetries = 0;
		m_bOrderInversionLogged = false;
		if (m_iState != CF_PAUSED_ON_FOOT && m_iState != CF_BOARDING &&
			m_iState != CF_UNLOAD_QUEUE && m_iState != CF_FORWARD_OUTBOUND_HOLD)
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
		if (CF_ConvoySession.IsVehicleAssignedToAnotherDriver(m_Truck, this))
		{
			Print("[ConvoyFollower] ASSIGN_REJECTED: vehicle was claimed by another convoy driver before boarding");
			m_Truck = null;
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
		m_bForwardWaitDeparture = false;
		m_bUnloadSlotParkedLogged = false;
		m_bReturnTurnFailed = false;
		m_bReturnWaitForOwnerPass = false;
		m_bReturnPassStartValid = false;
		m_ReturnPassVehicle = null;
		m_bSilentReturnFollow = false;
		m_iUnloadTurnPhase = 0;
		m_iReturnTurnPhase = 0;
		m_iReturnGoalReissues = 0;
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
		m_iStationaryUnexpectedExits = 0;
		m_fSinceUnexpectedExitSeconds = 0;
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
		SetEventMask(m_Driver, EntityEvent.POSTFRAME);
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
		if (CF_ConvoySession.IsVehicleAssignedToAnotherDriver(vehicle, this))
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
		vector exitPosition = m_Truck.GetOrigin();
		if (m_iStationaryUnexpectedExits == 0 ||
			m_fSinceUnexpectedExitSeconds > CF_REBOARD_EPISODE_RESET_SECONDS ||
			vector.Distance(exitPosition, m_vUnexpectedExitEpisodeStart) >=
			CF_REBOARD_EPISODE_PROGRESS_METERS)
		{
			m_iStationaryUnexpectedExits = 1;
			m_vUnexpectedExitEpisodeStart = exitPosition;
		}
		else
			m_iStationaryUnexpectedExits++;
		m_fSinceUnexpectedExitSeconds = 0;
		if (m_iStationaryUnexpectedExits >= CF_REBOARD_EPISODE_MAX_EXITS)
		{
			Print("[ConvoyFollower] REBOARD_TERMINAL: Unit " + m_iUnitNumber +
				" exited " + m_iStationaryUnexpectedExits +
				" times without moving five metres; removing blocked truck from convoy");
			SendRadioCall(CF_RadioEvent.STUCK);
			StandDown();
			return;
		}
		m_iUnloadReboardState = 0;
		if (m_iState == CF_UNLOAD_QUEUE || m_iState == CF_FORWARD_OUTBOUND_HOLD ||
			m_iState == CF_UNLOAD_DEPARTING || m_iState == CF_UNLOAD_DEPARTED ||
			m_iState == CF_FORWARD_WAIT_DEPARTED ||
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
		else if (m_iState == CF_FOLLOWING)
			waypoint.SetCompletionRadius(CF_ConvoySettings.Get().GetMoveCompletionRadius());
		else
			waypoint.SetCompletionRadius(CF_ConvoySettings.Get().m_fMovingGap);
		m_Waypoint = waypoint;
		m_Group.AddWaypoint(waypoint);
		m_vLastWaypointPosition = destination;
		m_fWaypointSeconds = 0;
		Print("[ConvoyFollower] FOLLOW_MOVE_CREATED: Unit " + m_iUnitNumber +
			" state=" + m_iState + " goal=" + destination + " radius=" + waypoint.GetCompletionRadius());
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
		CF_LogFollowWait("MOVE_UPDATED");
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
			if (m_bArrivalTrailMode && !m_bArrivalTrailHold)
			{
				m_bArrivalTrailMode = false;
				m_bArrivalTrailHold = false;
				m_bArrivalTrailGoalValid = false;
				m_bArrivalCloseLogged = false;
			}
			if (m_bStopSettleIssued)
			{
				SCR_AIWaypoint activeWaypoint = SCR_AIWaypoint.Cast(m_Waypoint);
				if (activeWaypoint && HasOwnWaypointInGroup())
				{
					if (m_iState == CF_FOLLOWING)
						activeWaypoint.SetCompletionRadius(CF_ConvoySettings.Get().GetMoveCompletionRadius());
					else
						activeWaypoint.SetCompletionRadius(CF_ConvoySettings.Get().m_fMovingGap);
				}
				m_bStopSettleIssued = false;
			}
			return;
		}

		m_fTargetStillSeconds += elapsed;
		// Keep measuring how long the player's lead has stopped, but do not
		// replace a queued successor's validated cargo-bay waypoint with a
		// different predecessor trail point.
		if (m_bUnloadSequenceHold && m_bUnloadBayGoalValid)
			return;
		if (CF_ShouldHoldCompletedArrival(separation))
			return;
		if (m_bArrivalRoadHold)
			return;
		float settleDistance = CF_STOP_SETTLE_DISTANCE;
		if (m_iState == CF_ARRIVING)
			settleDistance = CF_ConvoySettings.Get().m_fStoppedGap + 1.0;
		else if (settleDistance < CF_ConvoySettings.Get().m_fStoppedGap + 4.0)
			settleDistance = CF_ConvoySettings.Get().m_fStoppedGap + 4.0;
		if (m_fTargetStillSeconds < CF_STOP_DETECT_SECONDS || m_bStopSettleIssued ||
			separation <= settleDistance || m_fWaypointSeconds < CF_WAYPOINT_REFRESH_SECONDS)
			return;
		// A distant truck must finish the predecessor's traversed route before
		// making its final approach on the same road segment.
		vector stoppedRoadGoal;
		if (!CF_TryGetOrdinaryStoppedTrailGoal(m_LeadVehicle, stoppedRoadGoal) ||
			vector.Distance(m_Truck.GetOrigin(), stoppedRoadGoal) > separation + 8.0)
			return;
		if (m_bArrivalTrailMode)
		{
			vector delta = stoppedRoadGoal - m_Truck.GetOrigin();
			vector forward = m_Truck.GetWorldTransformAxis(2);
			if (delta[0] * forward[0] + delta[2] * forward[2] < -2.0)
			{
				if (HasOwnWaypointInGroup())
					CF_LogFollowWait("BEHIND_POINT_NATIVE_MOVE_PENDING");
				else
					CF_LogFollowWait("BEHIND_POINT_NO_NEW_ORDER");
				return;
			}
		}

		// Mark this stationary episode even if a new waypoint cannot be made;
		// a persistent obstruction must not create an order every frame.
		m_bStopSettleIssued = true;
		if (!MoveWaypoint(stoppedRoadGoal))
		{
			Print("[ConvoyFollower] STOP_SETTLE_FAILED: Unit " + m_iUnitNumber + " could not refresh final waypoint");
			return;
		}

		SCR_AIWaypoint waypoint = SCR_AIWaypoint.Cast(m_Waypoint);
		if (waypoint)
			waypoint.SetCompletionRadius(CF_STOPPED_ROAD_GOAL_RADIUS);
		if (m_bArrivalTrailMode)
			Print("[ConvoyFollower] STOP_SETTLE_TRAIL: Unit " + m_iUnitNumber + " actual predecessor point " + stoppedRoadGoal);
		else
			Print("[ConvoyFollower] STOP_SETTLE_ROAD: Unit " + m_iUnitNumber +
				" closing on predecessor route point " + stoppedRoadGoal);
	}

	// A MOVE can finish while a fast arriving truck is still sliding. If it
	// settles outside the mapped shoulder, one tight road MOVE may correct it.
	// Never announce arrival or repeat short reverse-seeking orders off road.
	protected bool CF_UpdateArrivalRoadRecovery(float elapsed, float separation)
	{
		if (m_iState != CF_ARRIVING || !m_Truck || !m_LeadVehicle)
			return false;
		// Policy is a fact about this link, not whether a stop-point cache was
		// refreshed before the still timer crossed its threshold this poll.
		// Small lead creep must never send an off-network link to a distant road.
		if (m_bArrivalTrailMode || CF_UsesOrdinaryOffNetworkTrail())
			return false;
		if (m_bArrivalRoadRecoveryBlocked)
			return true;
		if (m_bArrivalRoadRecoveryActive)
		{
			m_fArrivalRoadRecoverySeconds += elapsed;
			if (CF_IsTruckNearMappedRoad() && CF_IsPanelVehicleSlow(2.0))
			{
				m_bArrivalRoadRecoveryActive = false;
				m_bArrivalRoadHold = true;
				ClearWaypoints();
				CF_UpdateVehicleBrake();
				Print("[ConvoyFollower] ARRIVAL_ROAD_RECOVERED: Unit " + m_iUnitNumber +
					" seated and stopped on mapped road");
				return true;
			}
			if (m_fArrivalRoadRecoverySeconds >= CF_ARRIVAL_ROAD_RECOVERY_TIMEOUT)
			{
				m_bArrivalRoadRecoveryActive = false;
				m_bArrivalRoadRecoveryBlocked = true;
				ClearWaypoints();
				CF_UpdateVehicleBrake();
				Print("[ConvoyFollower] ARRIVAL_ROAD_BLOCKED: Unit " + m_iUnitNumber +
					" could not settle on mapped road after one correction; release unavailable");
			}
			return true;
		}
		if (CF_IsTruckNearMappedRoad() ||
			m_fTargetStillSeconds < CF_STOP_DETECT_SECONDS ||
			separation > CF_ConvoySettings.Get().m_fStoppedGap + 16.0 ||
			!CF_IsPanelVehicleSlow(2.0) ||
			(!m_bArrivalRoadHold && HasOwnWaypointInGroup()))
			return false;

		m_bArrivalRoadHold = false;
		m_bArrivalCloseLogged = false;
		SetUnloadReleaseReady(false);
		if (m_bArrivalRoadRecoveryAttempted)
		{
			m_bArrivalRoadRecoveryBlocked = true;
			ClearWaypoints();
			CF_UpdateVehicleBrake();
			Print("[ConvoyFollower] ARRIVAL_ROAD_BLOCKED: Unit " + m_iUnitNumber +
				" drifted off mapped road again; release unavailable");
			return true;
		}
		m_bArrivalRoadRecoveryAttempted = true;
		vector roadGoal;
		if (!CF_TryGetStoppedTrailRoadGoal(m_LeadVehicle, roadGoal))
		{
			m_bArrivalRoadRecoveryBlocked = true;
			ClearWaypoints();
			CF_UpdateVehicleBrake();
			Print("[ConvoyFollower] ARRIVAL_ROAD_BLOCKED: Unit " + m_iUnitNumber +
				" has no reachable predecessor road point; release unavailable");
			return true;
		}
		m_bTurnProbeOccupied = false;
		GetGame().GetWorld().QueryEntitiesBySphere(roadGoal, 4.5, ConsiderTurnObstacle);
		if (m_bTurnProbeOccupied || !IssueMoveWaypoint(roadGoal))
		{
			m_bArrivalRoadRecoveryBlocked = true;
			ClearWaypoints();
			CF_UpdateVehicleBrake();
			Print("[ConvoyFollower] ARRIVAL_ROAD_BLOCKED: Unit " + m_iUnitNumber +
				" road correction is occupied or unavailable; release unavailable");
			return true;
		}
		SCR_AIWaypoint correction = SCR_AIWaypoint.Cast(m_Waypoint);
		if (correction)
			correction.SetCompletionRadius(CF_ARRIVAL_ROAD_RECOVERY_RADIUS);
		m_bArrivalRoadRecoveryActive = true;
		m_fArrivalRoadRecoverySeconds = 0;
		CF_UpdateVehicleBrake();
		Print("[ConvoyFollower] ARRIVAL_ROAD_RECOVERY: Unit " + m_iUnitNumber +
			" correcting off-road stop toward " + roadGoal);
		return true;
	}

	protected bool CF_ShouldHoldCompletedArrival(float separation)
	{
		if (m_bArrivalTrailMode)
			return m_iState == CF_ARRIVING && m_bArrivalTrailHold &&
				separation <= CF_ConvoySettings.Get().m_fStoppedGap + CF_ARRIVAL_CLOSE_REAPPROACH_BUFFER;
		if (!CF_IsTruckNearMappedRoad())
			return false;
		if (m_iState == CF_ARRIVING && m_bArrivalCloseLogged &&
			m_bUnloadSequenceHold && m_bUnloadBayGoalValid)
			return vector.Distance(m_Truck.GetOrigin(), m_vUnloadBayGoal) <=
				CF_UNLOAD_BAY_READY_EXIT_RADIUS;
		return m_iState == CF_ARRIVING && m_bArrivalCloseLogged &&
			separation <= CF_ConvoySettings.Get().m_fStoppedGap + CF_ARRIVAL_CLOSE_REAPPROACH_BUFFER;
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
		CF_ResetArrivalRoadRecovery();
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

		if (!IssueMoveWaypoint(GetLeadTrailGoal(targetVehicle)))
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
		bool targetAlreadyStopped = m_fTargetStillSeconds >= CF_STOP_DETECT_SECONDS;
		m_LeadVehicle = targetVehicle;
		m_vLastTargetPosition = targetVehicle.GetOrigin();
		m_vArrivalAnchorPosition = m_vLastTargetPosition;
		m_fTargetStillSeconds = 0;
		m_bStopSettleIssued = false;
		m_bArrivalCloseLogged = false;
		CF_ResetArrivalRoadRecovery();
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
		m_bArrivalTrailMode = CF_UsesOrdinaryOffNetworkTrail();
		ObserveConvoyOrder(targetVehicle, vector.Distance(m_Truck.GetOrigin(), targetVehicle.GetOrigin()));
		if (m_bOrderInversionLogged)
		{
			Print("[ConvoyFollower] ARRIVAL_HELD_OUT_OF_ORDER: Unit " + m_iUnitNumber + " waiting for its predecessor to pass");
			return;
		}

		// A stopped predecessor's sampled road point is a safe vehicle-length
		// behind it. The usual 20 m MOVE radius can complete before the truck
		// reaches that point, so the first approach must use the tight radius.
		vector arrivalGoal = GetLeadTrailGoal(targetVehicle);
		bool tightRoadGoal = m_bUnloadSequenceHold && m_bUnloadBayGoalValid;
		if (!tightRoadGoal && targetAlreadyStopped)
		{
			vector stoppedRoadGoal;
			if (CF_TryGetOrdinaryStoppedTrailGoal(targetVehicle, stoppedRoadGoal))
			{
				arrivalGoal = stoppedRoadGoal;
				tightRoadGoal = true;
				m_bStopSettleIssued = true;
			}
		}
		if (!MoveWaypoint(arrivalGoal))
		{
			Print("[ConvoyFollower] ARRIVAL_FAILED: Unit " + m_iUnitNumber + " could not create approach waypoint");
			StandDown();
			return;
		}
		SCR_AIWaypoint waypoint = SCR_AIWaypoint.Cast(m_Waypoint);
		if (waypoint)
		{
			if (tightRoadGoal)
				waypoint.SetCompletionRadius(CF_STOPPED_ROAD_GOAL_RADIUS);
			else
				waypoint.SetCompletionRadius(CF_ConvoySettings.Get().m_fMovingGap);
		}

		Print("[ConvoyFollower] ARRIVAL_APPROACH: Unit " + m_iUnitNumber + " closing behind parked convoy target");
	}

	protected void ClearWaypoints()
	{
		if (!m_Group || !m_Waypoint)
			return;
		if (m_iState == CF_FOLLOWING || m_iState == CF_ARRIVING)
			Print("[ConvoyFollower] FOLLOW_MOVE_CLEARED: Unit " + m_iUnitNumber +
				" state=" + m_iState + " present=" + HasOwnWaypointInGroup() +
				" goal=" + m_vLastWaypointPosition + " age_s=" + m_fWaypointSeconds);

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
		{
			ClearEventMask(m_Driver, EntityEvent.FRAME);
			ClearEventMask(m_Driver, EntityEvent.POSTFRAME);
		}
		CF_ReleaseVehicleBrake();

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
		m_bForwardWaitDeparture = false;
		m_bUnloadSlotParkedLogged = false;
		m_bReturnTurnFailed = false;
		m_bReturnWaitForOwnerPass = false;
		m_bSilentReturnFollow = false;
		m_iUnloadTurnPhase = 0;
		m_iReturnTurnPhase = 0;
		m_iReturnGoalReissues = 0;
		m_iUnloadReboardState = 0;
		m_fUnloadSlotStillSeconds = 0;
		m_fArrivalTruckStillSeconds = 0;
		m_fReturnMergeGraceSeconds = 0;
		m_vOutboundTravelDirection = vector.Zero;
		m_vUnloadHomeDirection = vector.Zero;
		m_vReturnHomeDirection = vector.Zero;
		m_aRecentTruckPositions.Clear();
		ResetLeadTrail(null);
		SetOrderingPlayerId(0);
		m_iUnitNumber = 0;
		m_bPauseWasLost = false;
		ResetRangeWarning();
		m_Truck = null;
		m_iReboardAttempts = 0;
		m_iStationaryUnexpectedExits = 0;
		m_fSinceUnexpectedExitSeconds = 0;
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

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		CF_UpdateVehicleBrake();
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
		if (m_iStationaryUnexpectedExits > 0)
			m_fSinceUnexpectedExitSeconds += elapsed;
		if (m_iState == CF_UNLOAD_DEPARTING ||
			(m_iState == CF_REBOARDING && m_iUnloadReboardState == CF_UNLOAD_DEPARTING))
			m_fUnloadDepartureElapsedSeconds += elapsed;

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
				CF_PrepareEngineAfterBoarding();
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
			if (m_iUnloadReboardState == CF_UNLOAD_QUEUE ||
				m_iUnloadReboardState == CF_FORWARD_OUTBOUND_HOLD)
			{
				int holdState = m_iUnloadReboardState;
				m_iUnloadReboardState = 0;
				SetState(holdState);
				return;
			}
				if (m_iUnloadReboardState == CF_UNLOAD_DEPARTING || m_iUnloadReboardState == CF_UNLOAD_DEPARTED ||
					m_iUnloadReboardState == CF_FORWARD_WAIT_DEPARTED)
				{
					int resumeState = m_iUnloadReboardState;
					m_iUnloadReboardState = 0;
					SetState(resumeState);
					m_vLastUnloadTruckPosition = m_Truck.GetOrigin();
					m_fUnloadSlotStillSeconds = 0;
					vector reboardDestination = m_vUnloadWaitingPoint;
					if (resumeState == CF_UNLOAD_DEPARTING && m_iUnloadTurnPhase == 1)
						reboardDestination = m_vUnloadTurnStage;
					if (resumeState == CF_UNLOAD_DEPARTING &&
						vector.Distance(m_Truck.GetOrigin(), reboardDestination) > CF_UNLOAD_SLOT_RADIUS + 4.0)
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
			m_iState != CF_PANEL_HOLD &&
			m_iState != CF_ARRIVING && m_iState != CF_UNLOAD_QUEUE &&
			m_iState != CF_FORWARD_OUTBOUND_HOLD &&
			m_iState != CF_UNLOAD_DEPARTING && m_iState != CF_UNLOAD_DEPARTED &&
			m_iState != CF_FORWARD_WAIT_DEPARTED &&
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
		if (m_iState == CF_PANEL_HOLD)
			return;
		if (m_bForwardOutboundHoldRequested)
			CF_TryCompleteForwardOutboundHold();
		if (m_iState == CF_FORWARD_OUTBOUND_HOLD)
			return;
		SampleTruckRoute();
		if (m_iState == CF_UNLOAD_QUEUE)
			return;
		if (m_iState == CF_UNLOAD_DEPARTING)
		{
			m_fUnloadDepartureDiagnosticSeconds += elapsed;
			if (m_fUnloadDepartureDiagnosticSeconds >= 5.0)
			{
				m_fUnloadDepartureDiagnosticSeconds = 0;
				AIWaypoint activeUnloadWaypoint = m_Group.GetCurrentWaypoint();
				vector activeUnloadWaypointPosition = vector.Zero;
				if (activeUnloadWaypoint)
					activeUnloadWaypointPosition = activeUnloadWaypoint.GetOrigin();
				CarControllerComponent departureCar = CarControllerComponent.Cast(m_Truck.FindComponent(CarControllerComponent));
				VehicleWheeledSimulation departureSim;
				if (departureCar)
					departureSim = departureCar.GetSimulation();
				if (departureSim)
					Print("[ConvoyFollower] UNLOAD_DEPART_BRAKE: Unit " + m_iUnitNumber +
						" captured=" + m_bUnloadSlotBrakeCaptured + " owned=" + m_bOwnVehicleBrake +
						" speed=" + departureSim.GetSpeedKmh() + " brake=" + departureSim.GetBrake() +
						" throttle=" + departureSim.GetThrottle() + " gear=" + departureSim.GetGear() +
						" handbrake=" + departureSim.IsHandbrakeOn() +
						" persistent=" + departureCar.GetPersistentHandBrake() + " seated=" + CF_IsBoarded());
				Print("[ConvoyFollower] UNLOAD_DEPART_STATUS: Unit " + m_iUnitNumber +
					" phase=" + m_iUnloadTurnPhase + " origin=" + m_Truck.GetOrigin() +
					" slot=" + m_vUnloadWaitingPoint +
					" slot_gap=" + vector.Distance(m_Truck.GetOrigin(), m_vUnloadWaitingPoint) +
					" still_s=" + m_fUnloadSlotStillSeconds +
					" own_waypoint=" + HasOwnWaypointInGroup() +
					" active_waypoint=" + activeUnloadWaypointPosition);
			}
			if (m_iUnloadTurnPhase == 1)
			{
				m_fTurnStageSeconds += elapsed;
				if (vector.Distance(m_Truck.GetOrigin(), m_vUnloadTurnStage) <= CF_TURN_STAGE_RADIUS + 2.0)
				{
					m_iUnloadTurnPhase = 2;
					m_vLastUnloadTruckPosition = m_Truck.GetOrigin();
					m_fUnloadSlotStillSeconds = 0;
					CF_ResetUnloadSlotProgressSample();
					CF_ResetUnloadBestProgress();
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
				else if ((m_fTurnStageSeconds >= CF_UNLOAD_STAGE_STALL_FALLBACK_SECONDS &&
					vector.Distance(m_Truck.GetOrigin(), m_vLastUnloadTruckPosition) < CF_UNLOAD_STAGE_PROGRESS_METERS) ||
					m_fTurnStageSeconds >= CF_TURN_STAGE_TIMEOUT_SECONDS)
				{
					// A nearby lateral road point can be reachable in the road
					// graph but impossible for a long truck to steer into. Try the
					// already validated rear slot once; normal clearance and overall
					// departure timeout still decide whether the maneuver succeeds.
					m_iUnloadTurnPhase = 2;
					m_vLastUnloadTruckPosition = m_Truck.GetOrigin();
					m_fUnloadSlotStillSeconds = 0;
					CF_ResetUnloadSlotProgressSample();
					CF_ResetUnloadBestProgress();
					if (!IssueMoveWaypoint(m_vUnloadWaitingPoint))
					{
						FailUnloadDeparture("could not issue direct return-slot fallback");
						return;
					}
					SCR_AIWaypoint fallbackWaypoint = SCR_AIWaypoint.Cast(m_Waypoint);
					if (fallbackWaypoint)
						fallbackWaypoint.SetCompletionRadius(CF_UNLOAD_SLOT_RADIUS);
					Print("[ConvoyFollower] UNLOAD_TURN_STAGE_FALLBACK: Unit " + m_iUnitNumber +
						" routing directly to validated return slot after " + m_fTurnStageSeconds + " s");
				}
				return;
			}
			UpdateUnloadSlotStillness(elapsed);
			float currentSlotGap = vector.Distance(m_Truck.GetOrigin(), m_vUnloadWaitingPoint);
			if (currentSlotGap <= m_fUnloadBestSlotGap - CF_UNLOAD_SLOT_REISSUE_MIN_PROGRESS)
			{
				m_fUnloadBestSlotGap = currentSlotGap;
				m_fUnloadNoBestProgressSeconds = 0;
			}
			else
				m_fUnloadNoBestProgressSeconds += elapsed;
			if (m_bUnloadIntermediateActive &&
				(vector.Distance(m_Truck.GetOrigin(), m_vUnloadIntermediateGoal) <= CF_TURN_STAGE_RADIUS + 3.0 ||
				!HasOwnWaypointInGroup() ||
				m_fUnloadNoBestProgressSeconds >= CF_UNLOAD_SLOT_BEST_PROGRESS_TIMEOUT_SECONDS))
			{
				m_bUnloadIntermediateActive = false;
				if (!IssueMoveWaypoint(m_vUnloadWaitingPoint))
				{
					FailUnloadDeparture("could not resume validated return slot after intermediate road goal");
					return;
				}
				SCR_AIWaypoint finalSlotWaypoint = SCR_AIWaypoint.Cast(m_Waypoint);
				if (finalSlotWaypoint)
					finalSlotWaypoint.SetCompletionRadius(CF_UNLOAD_SLOT_RADIUS);
				CF_ResetUnloadSlotProgressSample();
				CF_ResetUnloadBestProgress();
				Print("[ConvoyFollower] UNLOAD_ROUTE_INTERMEDIATE_COMPLETE: Unit " + m_iUnitNumber +
					" continuing to validated return slot");
			}
			m_fUnloadSlotProgressSeconds += elapsed;
			if (!m_bUnloadClearReported &&
				m_fUnloadSlotProgressSeconds >= CF_UNLOAD_SLOT_REISSUE_CHECK_SECONDS)
			{
				float currentGap = currentSlotGap;
				float netMovement = vector.Distance(m_Truck.GetOrigin(), m_vUnloadSlotProgressPosition);
				float gapImprovement = m_fUnloadSlotProgressGap - currentGap;
				Print("[ConvoyFollower] UNLOAD_ROUTE_STALL_CHECK: Unit " + m_iUnitNumber +
					" slot_gap=" + currentGap + " net_moved=" + netMovement +
					" gap_improvement=" + gapImprovement +
					" best_gap=" + m_fUnloadBestSlotGap +
					" no_best_s=" + m_fUnloadNoBestProgressSeconds +
					" reissues=" + m_iUnloadSlotReissues);
				CF_ResetUnloadSlotProgressSample();
				bool nearForwardSlotWithoutWaypoint = m_bForwardWaitDeparture &&
					currentGap > CF_FORWARD_SLOT_BRAKE_RADIUS &&
					!HasOwnWaypointInGroup() &&
					m_fUnloadNoBestProgressSeconds >= CF_UNLOAD_SLOT_REISSUE_CHECK_SECONDS;
				if ((currentGap > CF_UNLOAD_SLOT_RADIUS + 6.0 || nearForwardSlotWithoutWaypoint) &&
					!m_bUnloadSlotBrakeCaptured && !m_bUnloadIntermediateActive &&
					((netMovement < CF_UNLOAD_SLOT_REISSUE_MIN_PROGRESS &&
					gapImprovement < CF_UNLOAD_SLOT_REISSUE_MIN_PROGRESS) ||
					m_fUnloadNoBestProgressSeconds >= CF_UNLOAD_SLOT_BEST_PROGRESS_TIMEOUT_SECONDS ||
					nearForwardSlotWithoutWaypoint) &&
					m_iUnloadSlotReissues < CF_UNLOAD_SLOT_MAX_REISSUES)
				{
					m_iUnloadSlotReissues++;
					bool intermediateIssued = false;
					if (!nearForwardSlotWithoutWaypoint)
						intermediateIssued = CF_TryIssueUnloadIntermediate();
					if (!intermediateIssued && !IssueMoveWaypoint(m_vUnloadWaitingPoint))
					{
						FailUnloadDeparture("stalled return-slot route could not be replanned");
						return;
					}
					if (!intermediateIssued)
					{
						SCR_AIWaypoint retriedWaypoint = SCR_AIWaypoint.Cast(m_Waypoint);
						if (retriedWaypoint)
						{
							if (m_bForwardWaitDeparture)
								retriedWaypoint.SetCompletionRadius(CF_FORWARD_SLOT_WAYPOINT_RADIUS);
							else
								retriedWaypoint.SetCompletionRadius(CF_UNLOAD_SLOT_RADIUS);
						}
					}
					CF_ResetUnloadBestProgress();
					m_bUnloadIntermediateActive = intermediateIssued;
					Print("[ConvoyFollower] UNLOAD_ROUTE_REISSUED: Unit " + m_iUnitNumber +
						" attempt " + m_iUnloadSlotReissues + " intermediate=" + intermediateIssued);
				}
			}
			if (!m_bUnloadClearReported && CF_IsUnloadBayClear(m_vUnloadAnchor))
			{
				m_bUnloadClearReported = true;
				if (m_bForwardWaitDeparture)
					Print("[ConvoyFollower] FORWARD_WAIT_SLOT_CLEAR: Unit " + m_iUnitNumber + " settled ahead and cleared cargo bay");
				else
					Print("[ConvoyFollower] UNLOAD_BAY_CLEAR: Unit " + m_iUnitNumber + " settled behind tail");
				if (m_Session)
					m_Session.OnUnloadBayCleared(this);
			}
			// Keep the cumulative bound even if a bay-clear callback was sent
			// but session validation did not promote this truck to the return
			// roster. Otherwise a seated driver could wait here forever.
			if (m_iState == CF_UNLOAD_DEPARTING &&
				m_fUnloadDepartureElapsedSeconds >= CF_UNLOAD_DEPART_TIMEOUT_SECONDS)
				FailUnloadDeparture("return-slot parking was not validated within two minutes");
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
		if (m_iState == CF_FORWARD_WAIT_DEPARTED)
		{
			UpdateUnloadSlotStillness(elapsed);
			m_fForwardParkedDiagnosticSeconds += elapsed;
			if (m_fForwardParkedDiagnosticSeconds >= 5.0)
			{
				m_fForwardParkedDiagnosticSeconds = 0;
				CarControllerComponent parkedCar = CarControllerComponent.Cast(m_Truck.FindComponent(CarControllerComponent));
				VehicleWheeledSimulation parkedSim;
				if (parkedCar)
					parkedSim = parkedCar.GetSimulation();
				float parkedSpeedKmh = 0;
				float parkedBrake = 0;
				float parkedThrottle = 0;
				int parkedGear = 0;
				bool handbrakeOn = false;
				if (parkedSim)
				{
					parkedSpeedKmh = parkedSim.GetSpeedKmh();
					parkedBrake = parkedSim.GetBrake();
					parkedThrottle = parkedSim.GetThrottle();
					parkedGear = parkedSim.GetGear();
					handbrakeOn = parkedSim.IsHandbrakeOn();
				}
				Print("[ConvoyFollower] FORWARD_WAIT_PARK_STATUS: Unit " + m_iUnitNumber +
					" origin=" + m_Truck.GetOrigin() +
					" slot_gap=" + vector.Distance(m_Truck.GetOrigin(), m_vUnloadWaitingPoint) +
					" drift=" + vector.Distance(m_Truck.GetOrigin(), m_vForwardParkedPosition) +
					" still_s=" + m_fUnloadSlotStillSeconds +
					" parked=" + CF_IsForwardWaitParked() +
					" speed_kmh=" + parkedSpeedKmh +
					" brake=" + parkedBrake + " throttle=" + parkedThrottle +
					" gear=" + parkedGear + " handbrake=" + handbrakeOn +
					" persistent=" + (parkedCar && parkedCar.GetPersistentHandBrake()));
			}
			return;
		}
		if (m_iState == CF_RETURN_BLOCKED)
			return;
		if (m_iState == CF_RETURN_WAIT)
		{
			CF_LogReturnNavigation(elapsed);
			IEntity returnTarget = GetTargetVehicle();
			if (!returnTarget || !CanTargetMove())
				return;
			if (!CF_HasOwnerPassedForReturn(returnTarget))
				return;
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
			bool hasStage = TryChooseTurnStage(m_vOutboundTravelDirection, m_vReturnTurnStage, turnSide);
			vector firstGoal = m_vReturnTurnStage;
			if (!hasStage && !CF_TryGetReturnRoadGoal(returnTarget, firstGoal))
			{
				if (vector.Distance(m_Truck.GetOrigin(), returnTarget.GetOrigin()) <
					CF_ConvoySettings.Get().m_fMovingGap + 8.0)
					return;
				FailReturnTurn("no clear lateral stage or reachable homeward road goal");
				return;
			}
			m_iReturnTurnPhase = 2;
			if (hasStage)
				m_iReturnTurnPhase = 1;
			m_vReturnTurnStartPosition = m_Truck.GetOrigin();
			m_fStateSeconds = 0;
			m_fReturnNavLogSeconds = 0;
			m_fReturnGoalRetrySeconds = 0;
			SetState(CF_RETURN_TURNING);
			if (!IssueMoveWaypoint(firstGoal))
			{
				FailReturnTurn("could not issue homeward road move");
				return;
			}
			SCR_AIWaypoint turnWaypoint = SCR_AIWaypoint.Cast(m_Waypoint);
			if (turnWaypoint && hasStage)
				turnWaypoint.SetCompletionRadius(CF_TURN_STAGE_RADIUS);
			if (hasStage)
				Print("[ConvoyFollower] RETURN_TURN_STAGE: Unit " + m_iUnitNumber + " taking clear " + turnSide + " road stage");
			else
				Print("[ConvoyFollower] RETURN_TURN_DIRECT_ROUTE: Unit " + m_iUnitNumber +
					" no lateral road stage; AI turning toward connected road point " + firstGoal);
			return;
		}
		if (m_iState == CF_RETURN_TURNING)
		{
			CF_LogReturnNavigation(elapsed);
			if (m_iReturnTurnPhase == 1)
			{
				if (vector.Distance(m_Truck.GetOrigin(), m_vReturnTurnStage) <= CF_TURN_STAGE_RADIUS + 2.0)
				{
					IEntity followingTarget = GetTargetVehicle();
					vector roadGoal;
					if (!followingTarget || !CanTargetMove() ||
						!CF_TryGetReturnRoadGoal(followingTarget, roadGoal) || !IssueMoveWaypoint(roadGoal))
					{
						FailReturnTurn("no reachable homeward road goal after turn stage");
						return;
					}
					m_iReturnTurnPhase = 2;
					m_fStateSeconds = 0;
					m_fReturnGoalRetrySeconds = 0;
					Print("[ConvoyFollower] RETURN_TURN_ROUTE: Unit " + m_iUnitNumber +
						" routing to homeward road point " + roadGoal);
				}
				else if ((m_fStateSeconds >= CF_UNLOAD_STAGE_STALL_FALLBACK_SECONDS &&
					vector.Distance(m_Truck.GetOrigin(), m_vReturnTurnStartPosition) <
					CF_UNLOAD_STAGE_PROGRESS_METERS) ||
					m_fStateSeconds >= CF_TURN_STAGE_TIMEOUT_SECONDS)
				{
					IEntity fallbackTarget = GetTargetVehicle();
					vector fallbackGoal;
					if (!fallbackTarget || !CF_TryGetReturnRoadGoal(fallbackTarget, fallbackGoal) ||
						!IssueMoveWaypoint(fallbackGoal))
					{
						FailReturnTurn("lateral stage stalled and no reachable homeward road goal");
						return;
					}
					m_iReturnTurnPhase = 2;
					m_fStateSeconds = 0;
					m_fReturnGoalRetrySeconds = 0;
					Print("[ConvoyFollower] RETURN_TURN_STAGE_FALLBACK: Unit " + m_iUnitNumber +
						" trying connected road goal " + fallbackGoal);
				}
				return;
			}
			if (IsFacingDirection(m_vReturnHomeDirection) &&
				vector.Distance(m_Truck.GetOrigin(), m_vReturnTurnStartPosition) >= CF_RETURN_MIN_TURN_PROGRESS)
			{
				IEntity homeTarget = GetTargetVehicle();
				if (homeTarget && CanTargetMove())
				{
					Print("[ConvoyFollower] RETURN_TURN_ALIGNED: Unit " + m_iUnitNumber + " following homeward convoy");
					StartFollowing(homeTarget, false, false);
					return;
				}
			}
			m_fReturnGoalRetrySeconds += elapsed;
			if (!HasOwnWaypointInGroup() &&
				m_fReturnGoalRetrySeconds >= CF_RETURN_GOAL_REISSUE_SECONDS &&
				m_iReturnGoalReissues < CF_RETURN_GOAL_MAX_REISSUES &&
				m_fStateSeconds < CF_RETURN_TURN_TIMEOUT_SECONDS)
			{
				m_fReturnGoalRetrySeconds = 0;
				IEntity retryTarget = GetTargetVehicle();
				vector retryGoal;
				if (retryTarget && CF_TryGetReturnRoadGoal(retryTarget, retryGoal) &&
					IssueMoveWaypoint(retryGoal))
				{
					m_iReturnGoalReissues++;
					Print("[ConvoyFollower] RETURN_TURN_GOAL_REISSUED: Unit " + m_iUnitNumber +
						" attempt " + m_iReturnGoalReissues + " road goal " + retryGoal);
				}
				else
					Print("[ConvoyFollower] RETURN_TURN_GOAL_WAIT: Unit " + m_iUnitNumber +
						" no connected goal yet; issued " + m_iReturnGoalReissues +
						" of " + CF_RETURN_GOAL_MAX_REISSUES);
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
		if (m_bArrivalTrailHold)
			CF_LogFollowWait("HELD_ON_DRIVEN_TRAIL");
		else if (m_bArrivalRoadRecoveryBlocked)
			CF_LogFollowWait("ROAD_CORRECTION_BLOCKED");
		else if (m_aLeadTrailPositions.Count() < 2 && CF_UsesOrdinaryOffNetworkTrail())
			CF_LogFollowWait("WAITING_FOR_PREDECESSOR_TRAIL");
		else if (!HasOwnWaypointInGroup())
			CF_LogFollowWait("NATIVE_MOVE_COMPLETED");
		else
			CF_LogFollowWait("NATIVE_MOVE_ACTIVE");

		// Let the preceding vehicle pull clear before restarting a completed
		// MOVE. Issuing a goal only a truck-length away makes the vehicle AI
		// brake and turn while the convoy is still accelerating from a stop.
		float arrivalResumeGap = CF_ConvoySettings.Get().m_fMovingGap + CF_ARRIVAL_RESUME_GAP_BUFFER;
		if (m_iState == CF_ARRIVING && !playerOnFoot &&
			(targetVehicle != m_LeadVehicle ||
			(vector.Distance(m_vArrivalAnchorPosition, targetVehicle.GetOrigin()) >= CF_ARRIVAL_RESUME_DISTANCE &&
			separation >= arrivalResumeGap)))
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

		if (!returnMergeGrace && !m_bForwardOutboundHoldRequested &&
			separation > CF_ConvoySettings.Get().m_fLostDistance)
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
		vector leadTrailGoal = GetLeadTrailGoal(m_LeadVehicle);

		m_fStuckSeconds += elapsed;
		if (!m_bForwardOutboundHoldRequested &&
			m_fStuckSeconds >= CF_ConvoySettings.Get().m_fStuckCheckSeconds)
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
				IssueMoveWaypoint(leadTrailGoal);
			}
			else if (movement >= 3.0)
				m_iStuckRetries = 0;

			m_fStuckSeconds = 0;
			m_vLastTruckPosition = m_Truck.GetOrigin();
			m_vLastStallTargetPosition = m_LeadVehicle.GetOrigin();
		}

		SettleBehindStoppedTarget(elapsed, separation);
		// A new off-network convoy has no driven segment yet. Absence of a
		// mapped road is not an obstruction: keep the ordinary following order
		// while the predecessor starts, without a brake/restart episode.
		bool waitingForOffNetworkTrail = CF_UsesOrdinaryOffNetworkTrail() && !m_bArrivalTrailGoalValid;
		if (m_iState == CF_FOLLOWING && m_fTargetStillSeconds >= CF_STOP_DETECT_SECONDS &&
			!waitingForOffNetworkTrail)
		{
			// This also applies while the player remains in the stopped lead
			// vehicle. Keep the existing road waypoint and its settled radius.
			m_vArrivalLastTruckPosition = m_Truck.GetOrigin();
			m_fArrivalTruckStillSeconds = 0;
			m_bArrivalCloseLogged = false;
			m_vArrivalAnchorPosition = targetVehicle.GetOrigin();
			SetState(CF_ARRIVING);
			m_bArrivalTrailMode = CF_UsesOrdinaryOffNetworkTrail();
			Print("[ConvoyFollower] ARRIVAL_APPROACH: Unit " + m_iUnitNumber + " closing behind stopped convoy target");
		}
		if (CF_UpdateArrivalRoadRecovery(elapsed, separation))
			return;
		if (m_iState == CF_ARRIVING)
		{
			if (vector.Distance(m_Truck.GetOrigin(), m_vArrivalLastTruckPosition) < 0.8)
				m_fArrivalTruckStillSeconds += elapsed;
			else
				m_fArrivalTruckStillSeconds = 0;
			m_vArrivalLastTruckPosition = m_Truck.GetOrigin();
			bool withinArrivalGate = separation <= CF_ConvoySettings.Get().m_fStoppedGap + 4.0;
			if (m_bUnloadSequenceHold && m_bUnloadBayGoalValid)
			{
				float bayGap = vector.Distance(m_Truck.GetOrigin(), m_vUnloadBayGoal);
				withinArrivalGate = bayGap <= CF_UNLOAD_BAY_READY_RADIUS;
				if (m_bArrivalCloseLogged && bayGap <= CF_UNLOAD_BAY_READY_EXIT_RADIUS)
					withinArrivalGate = true;
				withinArrivalGate = withinArrivalGate && CF_IsTruckNearMappedRoad();
			}
			else if (m_bArrivalCloseLogged &&
				separation <= CF_ConvoySettings.Get().m_fStoppedGap + CF_ARRIVAL_RELEASE_EXIT_BUFFER)
				withinArrivalGate = true;
			if (m_bArrivalTrailMode)
				withinArrivalGate = CF_IsOrdinaryTrailArrivalClose();
			else
				withinArrivalGate = withinArrivalGate && CF_IsTruckNearMappedRoad();
			bool nearStoppedTarget = m_fTargetStillSeconds >= CF_STOP_DETECT_SECONDS &&
				m_fArrivalTruckStillSeconds >= CF_UNLOAD_SLOT_STILL_SECONDS && withinArrivalGate;
			// An ordinary off-network stop is usable for Hold/Resume, but does
			// not grant permission for a road-only unload/return maneuver.
			SetUnloadReleaseReady(!m_Predecessor && nearStoppedTarget && !m_bArrivalTrailMode);
			if (nearStoppedTarget && !m_bArrivalCloseLogged)
			{
				m_bArrivalCloseLogged = true;
				Print("[ConvoyFollower] ARRIVAL_CLOSE: Unit " + m_iUnitNumber + " stopped near convoy target");
			}
		}
		if (CF_ShouldHoldCompletedArrival(separation))
			return;
		if (m_bArrivalTrailMode && m_fTargetStillSeconds >= CF_STOP_DETECT_SECONDS)
		{
			vector remaining = leadTrailGoal - m_Truck.GetOrigin();
			vector truckForward = m_Truck.GetWorldTransformAxis(2);
			if (remaining[0] * truckForward[0] + remaining[2] * truckForward[2] < -2.0)
			{
				if (HasOwnWaypointInGroup())
					CF_LogFollowWait("BEHIND_POINT_NATIVE_MOVE_PENDING");
				else
					CF_LogFollowWait("BEHIND_POINT_NO_NEW_ORDER");
				return;
			}
		}

		bool missingMoveWaypoint = !HasOwnWaypointInGroup();
		if (missingMoveWaypoint &&
			separation > CF_ConvoySettings.Get().m_fMovingGap + 3.0 &&
			m_iLeadTrailCursor < m_aLeadTrailPositions.Count() - 1 &&
			vector.Distance(m_Truck.GetOrigin(), leadTrailGoal) <= CF_LEAD_TRAIL_COMPLETED_DISTANCE)
		{
			// The AI can consume a 12 m MOVE before the truck origin enters our
			// exact 12 m trail radius. Advance the completed sample and give the
			// next order enough road ahead to avoid another instant completion.
			int previousTrailCursor = m_iLeadTrailCursor;
			while (m_iLeadTrailCursor < m_aLeadTrailPositions.Count() - 1 &&
				vector.Distance(m_Truck.GetOrigin(), m_aLeadTrailPositions[m_iLeadTrailCursor]) <
				CF_LEAD_TRAIL_WAYPOINT_LOOKAHEAD)
				m_iLeadTrailCursor++;
			if (m_iLeadTrailCursor == previousTrailCursor)
				m_iLeadTrailCursor++;
			leadTrailGoal = GetLeadTrailGoal(m_LeadVehicle);
			Print("[ConvoyFollower] FOLLOW_TRAIL_COMPLETED_ADVANCE: Unit " + m_iUnitNumber +
				" route point " + previousTrailCursor + " to " + m_iLeadTrailCursor +
				"; new goal " + leadTrailGoal);
		}
		float targetAdvance = vector.Distance(m_vLastWaypointPosition, leadTrailGoal);
		if (missingMoveWaypoint)
		{
			// The previous MOVE order often completes at the configured gap.
			// Reissue promptly when its target then moves away. For a stationary
			// target that remains obstructed, back off instead of creating a new
			// path every poll. Keep the stopped approach radius after recovery.
			float desiredGap = CF_ConvoySettings.Get().m_fMovingGap;
			if (m_iState == CF_ARRIVING && m_bStopSettleIssued)
				desiredGap = CF_ConvoySettings.Get().m_fStoppedGap;
			float retryDelay = CF_MISSING_WAYPOINT_STILL_RETRY_SECONDS;
			float gapBuffer = CF_MISSING_WAYPOINT_STILL_GAP_BUFFER;
			if (targetAdvance >= CF_MISSING_WAYPOINT_TARGET_ADVANCE)
			{
				retryDelay = CF_MISSING_WAYPOINT_RECOVERY_SECONDS;
				gapBuffer = CF_MISSING_WAYPOINT_GAP_BUFFER;
			}
			if (m_fWaypointSeconds >= retryDelay &&
				separation > desiredGap + gapBuffer)
			{
				Print("[ConvoyFollower] FOLLOW_WAYPOINT_REBUILD: Unit " + m_iUnitNumber +
					" cursor=" + m_iLeadTrailCursor + "/" + m_aLeadTrailPositions.Count() +
					" truck=" + m_Truck.GetOrigin() + " goal=" + leadTrailGoal +
					" target=" + m_LeadVehicle.GetOrigin() + " advance=" + targetAdvance);
				if (!IssueMoveWaypoint(leadTrailGoal))
				{
					Print("[ConvoyFollower] FOLLOW_FAILED: cannot restore completed move waypoint");
					StandDown();
				}
				else
				{
					if (m_iState == CF_ARRIVING && m_bStopSettleIssued)
					{
						SCR_AIWaypoint settledWaypoint = SCR_AIWaypoint.Cast(m_Waypoint);
						if (settledWaypoint)
							settledWaypoint.SetCompletionRadius(CF_STOPPED_ROAD_GOAL_RADIUS);
					}
					Print("[ConvoyFollower] FOLLOW_WAYPOINT_RECOVERED: Unit " + m_iUnitNumber +
						" rebuilt completed move order at " + separation + " m; target advanced " + targetAdvance + " m");
				}
			}
		}
		else if (m_fWaypointSeconds >= CF_WAYPOINT_REFRESH_SECONDS &&
			targetAdvance >= CF_WAYPOINT_REFRESH_DISTANCE &&
			!MoveWaypoint(leadTrailGoal))
		{
			Print("[ConvoyFollower] FOLLOW_FAILED: cannot refresh move waypoint");
			StandDown();
		}
	}

	// World teardown must not call StandDown/ResetToIdle: both can notify the
	// session or create fresh AI orders. All referenced entities are unloading.
	void CF_DetachForWorldCleanup()
	{
		if (m_Driver)
		{
			ClearEventMask(m_Driver, EntityEvent.FRAME);
			ClearEventMask(m_Driver, EntityEvent.POSTFRAME);
		}
		m_iState = CF_IDLE;
		m_bOwnVehicleBrake = false;
		m_Session = null;
		m_Predecessor = null;
		m_Waypoint = null;
		m_Group = null;
		m_Driver = null;
		m_Leader = null;
		m_LeadVehicle = null;
		m_LastPlayerVehicle = null;
		m_UnloadAnchorVehicle = null;
		m_Truck = null;
		m_Candidate = null;
		m_PassengerVehicle = null;
		m_LeadTrailTarget = null;
		m_ReturnPassVehicle = null;
		m_aRecentTruckPositions.Clear();
		m_aLeadTrailPositions.Clear();
	}

	override void OnDelete(IEntity owner)
	{
		if (CF_ConvoySession.CF_IsWorldCleanup())
		{
			CF_DetachForWorldCleanup();
			return;
		}
		if (m_Driver)
		{
			ClearEventMask(m_Driver, EntityEvent.FRAME);
			ClearEventMask(m_Driver, EntityEvent.POSTFRAME);
		}
		CF_ReleaseVehicleBrake();
		NotifySessionUnavailable();

		// Only release this component's active order during entity deletion.
		if (Replication.IsServer() && m_Group && m_Waypoint)
			m_Group.RemoveWaypoint(m_Waypoint);
	}
}
