// One server-owned convoy per ordering player. The ordered roster is also the
// physical driving chain: Unit One follows the player; each later unit follows
// the vehicle driven by the unit immediately ahead of it.
class CF_ConvoySession
{
	protected static const int CF_PENDING_NONE = 0;
	protected static const int CF_PENDING_START = 1;
	protected static const int CF_PENDING_ADD = 2;
	protected static const int CF_PENDING_REPLACE = 3;
	protected static const int CF_UNLOAD_NONE = 0;
	protected static const int CF_UNLOAD_SHIFTING = 1;
	protected static const int CF_UNLOAD_DEPARTING = 2;
	protected static const int CF_UNLOAD_POLL_MS = 1000;
	protected static const int CF_SHIFT_MAX_POLLS = 60;
	protected static const float CF_UNLOAD_LEAVE_DISTANCE = 20.0;
	protected static const float CF_RETURN_ARM_DISTANCE = 5.0;
	protected static const float CF_RETURN_CROSS_DISTANCE = 9.0;
	protected static const float CF_RETURN_CORRIDOR = 24.0;
	protected static const int CF_ORDER_REWIRE_CONFIRM_POLLS = 5;
	protected static const int CF_ORDER_REWIRE_COOLDOWN_POLLS = 12;
	protected static const float CF_ORDER_REWIRE_MIN_AHEAD = 15.0;
	protected static const float CF_ORDER_REWIRE_MAX_AHEAD = 65.0;
	protected static const float CF_ORDER_REWIRE_MAX_LATERAL = 7.0;

	protected static ref map<int, ref CF_ConvoySession> s_mSessions = new map<int, ref CF_ConvoySession>();

	protected int m_iOrderingPlayerId;
	protected IEntity m_OrderingPlayer;
	protected ref array<CF_DriverControllerComponent> m_aUnits = {};
	// Identity is kept apart from physical chain position. A newly promoted
	// service-front truck must retain its original numbered radio calls.
	protected ref array<CF_DriverControllerComponent> m_aIdentityDrivers = {};
	protected ref array<int> m_aIdentityNumbers = {};
	protected CF_DriverControllerComponent m_PendingDriver;
	protected CF_DriverControllerComponent m_LeaderBeingReplaced;
	protected int m_iPendingOperation;
	// Released trucks stay seated in a physically parked return line. They do
	// not follow until the owner drives homeward past the entire line.
	protected ref array<CF_DriverControllerComponent> m_aReturnQueue = {};
	// Rare activation failures remain owned and seated, rather than being
	// silently dropped from the session during a partial return merge.
	protected ref array<CF_DriverControllerComponent> m_aStrandedUnits = {};
	protected CF_DriverControllerComponent m_UnloadHead;
	protected CF_DriverControllerComponent m_ShiftingReturn;
	protected int m_iUnloadPhase;
	protected int m_iShiftIndex;
	protected int m_iShiftPolls;
	protected vector m_vUnloadAnchor;
	protected IEntity m_OriginalLeadVehicle;
	protected bool m_bUnloadAnchorLock;
	protected bool m_bUnloadReleaseBlocked;
	protected bool m_bUnloadPollScheduled;
	protected bool m_bSessionClosed;
	protected bool m_bReturnMerged;
	protected bool m_bReturnPending;
	protected int m_iReturnPendingPolls;
	protected int m_iOwnerMissingPolls;
	protected bool m_bReturnCrossingArmed;
	protected bool m_bReturnSampleValid;
	protected IEntity m_ReturnMarkerVehicle;
	protected vector m_vReturnMarkerPosition;
	protected vector m_vLastOwnerVehiclePosition;
	protected CF_DriverControllerComponent m_OrderInversionCandidate;
	protected int m_iOrderInversionPolls;
	protected int m_iOrderRewireCooldownPolls;
	protected bool m_bOrderRecoveryDeferredLogged;

	protected static int GetPlayerId(IEntity user)
	{
		if (!user || !GetGame())
			return 0;

		PlayerManager manager = GetGame().GetPlayerManager();
		if (!manager)
			return 0;

		return manager.GetPlayerIdFromControlledEntity(user);
	}

	static CF_ConvoySession GetForPlayer(IEntity user)
	{
		int playerId = GetPlayerId(user);
		if (playerId <= 0 || !s_mSessions.Contains(playerId))
			return null;

		return s_mSessions.Get(playerId);
	}

	// This is used only for local interaction visibility. Server-side action
	// checks always read the authoritative session instead of this RplProp.
	static bool HasActiveConvoyForActions(IEntity user)
	{
		int playerId = GetPlayerId(user);
		if (playerId <= 0)
			return false;

		PlayerManager manager = GetGame().GetPlayerManager();
		if (!manager)
			return false;

		SCR_PlayerController controller = SCR_PlayerController.Cast(manager.GetPlayerController(playerId));
		return controller && controller.CF_HasActiveConvoy();
	}

	static bool CanStart(IEntity user, CF_DriverControllerComponent candidate)
	{
		return GetPlayerId(user) > 0 && !GetForPlayer(user) && candidate && candidate.CanAssign(user);
	}

	static bool CanAdd(IEntity user, CF_DriverControllerComponent candidate)
	{
		CF_ConvoySession session = GetForPlayer(user);
		return session && session.m_aUnits.Count() > 0 &&
			session.m_aUnits.Count() + session.m_aReturnQueue.Count() + session.m_aStrandedUnits.Count() <
			CF_ConvoySettings.Get().m_iMaxConvoyUnits &&
			!session.m_PendingDriver && !session.m_UnloadHead && candidate && candidate.CanAssign(user);
	}

	static bool CanReplace(IEntity user, CF_DriverControllerComponent candidate)
	{
		CF_ConvoySession session = GetForPlayer(user);
		return session && session.m_aUnits.Count() > 0 && !session.m_PendingDriver &&
			!session.m_UnloadHead && candidate && candidate.CanAssign(user);
	}

	static bool Start(IEntity user, CF_DriverControllerComponent candidate)
	{
		if (!Replication.IsServer() || !CanStart(user, candidate))
			return false;

		int playerId = GetPlayerId(user);
		CF_ConvoySession session = new CF_ConvoySession();
		session.m_iOrderingPlayerId = playerId;
		session.m_OrderingPlayer = user;
		s_mSessions.Set(playerId, session);
		session.ScheduleUnloadPoll();
		if (!session.BeginPending(candidate, CF_PENDING_START))
		{
			session.CloseSession();
			return false;
		}

		Print("[ConvoyFollower] CONVOY_START_PENDING: leader boarding");
		return true;
	}

	static bool Add(IEntity user, CF_DriverControllerComponent candidate)
	{
		if (!Replication.IsServer() || !CanAdd(user, candidate))
			return false;

		CF_ConvoySession session = GetForPlayer(user);
		if (!session.BeginPending(candidate, CF_PENDING_ADD))
			return false;

		Print("[ConvoyFollower] CONVOY_ADD_PENDING: new unit boarding");
		return true;
	}

	static bool Replace(IEntity user, CF_DriverControllerComponent candidate)
	{
		if (!Replication.IsServer() || !CanReplace(user, candidate))
			return false;

		CF_ConvoySession session = GetForPlayer(user);
		if (!session.BeginPending(candidate, CF_PENDING_REPLACE))
			return false;

		Print("[ConvoyFollower] CONVOY_REPLACE_PENDING: replacement boarding; current leader still active");
		return true;
	}

	// The explicit rear-menu order is available only on the truck currently
	// at the unload bay. Merely stopping or getting out never dispatches it.
	static bool CanReleaseAtUnload(IEntity user, CF_DriverControllerComponent driver)
	{
		CF_ConvoySession session = GetForPlayer(user);
		if (!session || !driver || session.m_aUnits.IsEmpty() ||
			session.m_aUnits[0] != driver || session.m_PendingDriver ||
			session.m_UnloadHead || session.m_iUnloadPhase != CF_UNLOAD_NONE ||
			!driver.CF_CanReleaseAtUnload() || !driver.CF_IsBoarded())
			return false;
		// A parked return line belongs to one unloading place. The player
		// may continue forward with unreleased trucks, but a second bay
		// cannot silently move that existing return rendezvous.
		if (!session.m_aReturnQueue.IsEmpty() &&
			vector.Distance(driver.CF_GetUnloadAnchor(), session.m_vUnloadAnchor) > 20.0)
			return false;
		return true;
	}

	// The rear action may ask the server whether a road slot exists before
	// accepting the order. This gives the player an immediate menu reason
	// instead of a command that appears to do nothing.
	static bool CanPlanReleaseAtUnload(IEntity user, CF_DriverControllerComponent driver)
	{
		if (!Replication.IsServer() || !CanReleaseAtUnload(user, driver))
			return false;
		CF_ConvoySession session = GetForPlayer(user);
		vector waitingPoint;
		return session.TryFindReleaseSlot(waitingPoint) || session.CanShiftReturnLineForGap();
	}

	static bool ReleaseAtUnload(IEntity user, CF_DriverControllerComponent driver)
	{
		if (!Replication.IsServer() || !CanReleaseAtUnload(user, driver))
			return false;

		CF_ConvoySession session = GetForPlayer(user);
		vector anchor = driver.CF_GetUnloadAnchor();
		if (session.m_aReturnQueue.IsEmpty())
		{
			session.m_vUnloadAnchor = anchor;
			session.m_OriginalLeadVehicle = driver.CF_GetCachedPlayerVehicle();
			session.m_bReturnMerged = false;
			session.m_bReturnPending = false;
			session.m_iReturnPendingPolls = 0;
		}
		else if (vector.Distance(anchor, session.m_vUnloadAnchor) > 20.0)
		{
			Print("[ConvoyFollower] UNLOAD_RELEASE_FAILED: parked return line belongs to another stopping place");
			return false;
		}

		vector waitingPoint;
		bool hasGap = session.TryFindReleaseSlot(waitingPoint);
		if (!hasGap && !session.CanShiftReturnLineForGap())
		{
			// No driving order was issued. This is a route preflight failure,
			// not evidence that the truck is stuck.
			Print("[ConvoyFollower] UNLOAD_RELEASE_REJECTED: no reachable, clear road slot behind the convoy; driver remains at bay");
			return false;
		}

		session.m_UnloadHead = driver;
		session.m_bUnloadAnchorLock = true;
		session.m_bUnloadReleaseBlocked = false;
		session.ResetReturnCrossing();
		foreach (CF_DriverControllerComponent unit : session.m_aUnits)
		{
			if (unit)
				unit.CF_SetUnloadSequenceHold(true);
		}
		for (int i = 1; i < session.m_aUnits.Count(); i++)
		{
			if (session.m_aUnits[i])
				session.m_aUnits[i].CF_HoldForUnloadQueue();
		}
		if (hasGap)
			session.StartHeadDeparture(waitingPoint);
		else
		{
			session.m_iUnloadPhase = CF_UNLOAD_SHIFTING;
			session.m_iShiftIndex = 0;
			session.BeginNextReturnShift();
		}
		session.ScheduleUnloadPoll();
		return true;
	}

	protected CF_DriverControllerComponent GetNearestReturnUnit(vector outboundTail)
	{
		CF_DriverControllerComponent nearest;
		float shortestDistance = 99999.0;
		foreach (CF_DriverControllerComponent parked : m_aReturnQueue)
		{
			if (!parked || !parked.CF_GetAssignedVehicle())
				continue;
			float distance = vector.Distance(parked.CF_GetAssignedVehicle().GetOrigin(), outboundTail);
			if (distance < shortestDistance)
			{
				shortestDistance = distance;
				nearest = parked;
			}
		}
		return nearest;
	}

	protected vector GetOutboundTailPosition()
	{
		vector tail = m_vUnloadAnchor;
		if (m_aUnits.IsEmpty())
			return tail;
		CF_DriverControllerComponent lastOutbound = m_aUnits[m_aUnits.Count() - 1];
		if (lastOutbound && lastOutbound.CF_GetAssignedVehicle())
			tail = lastOutbound.CF_GetAssignedVehicle().GetOrigin();
		return tail;
	}

	protected bool CanShiftReturnLineForGap()
	{
		if (m_aReturnQueue.IsEmpty())
			return false;
		vector tail = GetOutboundTailPosition();
		CF_DriverControllerComponent nearest = GetNearestReturnUnit(tail);
		if (!nearest || !nearest.CF_GetAssignedVehicle())
			return false;
		// A generous existing gap rejected by the road probe is a routing
		// failure. Shifting parked trucks would not fix that geometry.
		return vector.Distance(tail, nearest.CF_GetAssignedVehicle().GetOrigin()) < 27.0;
	}
	protected bool TryFindReleaseSlot(out vector waitingPoint)
	{
		waitingPoint = vector.Zero;
		if (m_aUnits.IsEmpty())
			return false;
		CF_DriverControllerComponent front = m_UnloadHead;
		if (!front)
			front = m_aUnits[0];
		if (!front || !front.CF_GetAssignedVehicle())
			return false;
		vector outboundTail = GetOutboundTailPosition();
		CF_DriverControllerComponent nearest = GetNearestReturnUnit(outboundTail);
		vector nearestPosition = vector.Zero;
		bool hasParked = nearest && nearest.CF_GetAssignedVehicle();
		if (hasParked)
			nearestPosition = nearest.CF_GetAssignedVehicle().GetOrigin();
		return front.CF_FindReturnUnloadWaitingPoint(outboundTail, nearestPosition, hasParked, waitingPoint);
	}

	protected void StartHeadDeparture(vector waitingPoint)
	{
		if (!m_UnloadHead || m_aUnits.IsEmpty() || m_aUnits[0] != m_UnloadHead)
		{
			FailUnloadSequence("front truck was lost before its release order");
			return;
		}
		m_iUnloadPhase = CF_UNLOAD_DEPARTING;
		if (!m_UnloadHead.CF_BeginUnloadDeparture(waitingPoint))
		{
			FailUnloadSequence("front truck could not begin its road turn");
			return;
		}
		Print("[ConvoyFollower] UNLOAD_RELEASE_REQUESTED: front truck turning into a road slot behind the outbound tail");
	}

	// Farthest parked truck moves first. A later released truck can then
	// occupy a newly opened slot BETWEEN the outbound tail and return line.
	protected void BeginNextReturnShift()
	{
		if (!Replication.IsServer() || m_iUnloadPhase != CF_UNLOAD_SHIFTING || !m_UnloadHead)
			return;
		if (m_iShiftIndex >= m_aReturnQueue.Count())
		{
			vector waitingPoint;
			if (!TryFindReleaseSlot(waitingPoint))
			{
				FailUnloadSequence("return line moved but no safe slot opened");
				return;
			}
			StartHeadDeparture(waitingPoint);
			return;
		}
		CF_DriverControllerComponent parked = m_aReturnQueue[m_iShiftIndex];
		vector shiftPoint;
		if (!parked || !parked.CF_IsAtUnloadWaitingPoint() ||
			!parked.CF_FindHomewardShiftPoint(shiftPoint, 14.0) ||
			!parked.CF_ShiftDepartedVehicle(shiftPoint))
		{
			FailUnloadSequence("parked return truck cannot shift homeward");
			return;
		}
		m_ShiftingReturn = parked;
		m_iShiftPolls = 0;
		ResetReturnCrossing();
		Print("[ConvoyFollower] RETURN_LINE_SHIFT: parked truck moving homeward to free another slot");
		GetGame().GetCallqueue().CallLater(PollReturnShift, CF_UNLOAD_POLL_MS, false);
	}

	protected void PollReturnShift()
	{
		if (!Replication.IsServer() || m_iUnloadPhase != CF_UNLOAD_SHIFTING || !m_UnloadHead)
			return;
		if (!m_ShiftingReturn || !m_ShiftingReturn.CF_IsBoarded())
		{
			FailUnloadSequence("parked return truck became unavailable while shifting");
			return;
		}
		if (m_ShiftingReturn.CF_IsAtUnloadWaitingPoint())
		{
			m_ShiftingReturn = null;
			m_iShiftIndex++;
			BeginNextReturnShift();
			return;
		}
		m_iShiftPolls++;
		if (m_iShiftPolls >= CF_SHIFT_MAX_POLLS)
		{
			FailUnloadSequence("parked return truck did not reach its new slot");
			return;
		}
		GetGame().GetCallqueue().CallLater(PollReturnShift, CF_UNLOAD_POLL_MS, false);
	}

	protected void FailUnloadSequence(string reason)
	{
		m_UnloadHead = null;
		m_ShiftingReturn = null;
		m_iUnloadPhase = CF_UNLOAD_NONE;
		m_bUnloadReleaseBlocked = true;
		foreach (CF_DriverControllerComponent unit : m_aUnits)
		{
			if (unit)
				unit.CF_HoldForUnloadQueue();
		}
		Print("[ConvoyFollower] UNLOAD_RELEASE_FAILED: " + reason + "; outbound line held");
	}

	// Called only after the controller verifies the released truck reached
	// and settled at its road slot. Issuing a waypoint does not promote Unit
	// Two or remove the truck from the session.
	void OnUnloadBayCleared(CF_DriverControllerComponent driver)
	{
		if (!Replication.IsServer() || m_iUnloadPhase != CF_UNLOAD_DEPARTING ||
			driver != m_UnloadHead || m_aUnits.IsEmpty() || m_aUnits[0] != driver ||
			!driver.CF_IsUnloadBayClear(m_vUnloadAnchor))
			return;

		driver.CF_CompleteUnloadDeparture();
		if (!driver.CF_IsUnloadDeparted())
			return;
		m_aUnits.RemoveOrdered(0);
		m_aReturnQueue.Insert(driver);
		m_UnloadHead = null;
		m_ShiftingReturn = null;
		m_iUnloadPhase = CF_UNLOAD_NONE;
		m_bUnloadReleaseBlocked = false;
		ResetReturnCrossing();
		Print("[ConvoyFollower] UNLOAD_RETURN_PARKED: front truck settled behind the convoy; next truck may approach bay");
		RewireTargets();
		ResumeUnloadQueueAtAnchor();
		ScheduleUnloadPoll();
	}

	void OnUnloadDepartureFailed(CF_DriverControllerComponent driver)
	{
		if (!Replication.IsServer() || m_iUnloadPhase != CF_UNLOAD_DEPARTING ||
			driver != m_UnloadHead)
			return;
		m_UnloadHead = null;
		m_ShiftingReturn = null;
		m_iUnloadPhase = CF_UNLOAD_NONE;
		m_bUnloadReleaseBlocked = true;
		// Do not advance Unit Two into an obstructed bay. The owner can
		// drive away to resume ordinary follow, or retry after recovery.
		foreach (CF_DriverControllerComponent unit : m_aUnits)
		{
			if (!unit)
				continue;
			unit.CF_SetUnloadSequenceHold(true);
			unit.CF_HoldForUnloadQueue();
		}
		Print("[ConvoyFollower] UNLOAD_RELEASE_FAILED: front turn did not settle; outbound queue held");
		// The route controller logs the observed failure. Do not translate
		// every failed unload maneuver into a radio claim that the truck is
		// mechanically stuck.
	}

	// Explicit rear-menu recovery after a failed or interrupted unload.
	// Released return trucks keep waiting; only the outbound line resumes.
	static bool CanResumeFollowing(IEntity user, CF_DriverControllerComponent driver)
	{
		CF_ConvoySession session = GetForPlayer(user);
		return session && driver && !session.m_aUnits.IsEmpty() &&
			session.m_aUnits[0] == driver && !session.m_PendingDriver &&
			!session.m_UnloadHead && session.m_iUnloadPhase == CF_UNLOAD_NONE &&
			(session.m_bUnloadAnchorLock || session.m_bUnloadReleaseBlocked);
	}

	static bool ResumeFollowing(IEntity user, CF_DriverControllerComponent driver)
	{
		if (!Replication.IsServer() || !CanResumeFollowing(user, driver))
			return false;
		CF_ConvoySession session = GetForPlayer(user);
		session.m_bUnloadAnchorLock = false;
		session.m_bUnloadReleaseBlocked = false;
		session.m_bReturnPending = false;
		session.m_iReturnPendingPolls = 0;
		session.ResetReturnCrossing();
		foreach (CF_DriverControllerComponent unit : session.m_aUnits)
		{
			if (!unit)
				continue;
			unit.CF_SetUnloadSequenceHold(false);
			unit.CF_ResumeFromUnloadQueue();
		}
		session.UpdateReleasePermission();
		Print("[ConvoyFollower] UNLOAD_CANCELLED_BY_PLAYER: outbound trucks resume following; parked return line stays parked");
		return true;
	}

	// An owner can explicitly call the parked return line from any of its
	// rear cargo menus. This is a backup to the drive-past crossing trigger.
	// Drivers wait for the owner to drive before ordinary following resumes.
	static bool CanStartReturnFromAction(IEntity user, CF_DriverControllerComponent driver)
	{
		CF_ConvoySession session = GetForPlayer(user);
		return session && driver && session.m_aReturnQueue.Contains(driver) &&
			!session.m_aReturnQueue.IsEmpty() && !session.m_UnloadHead &&
			session.m_iUnloadPhase == CF_UNLOAD_NONE && !session.m_bReturnMerged &&
			!session.m_bReturnPending;
	}

	static bool StartReturnFromAction(IEntity user, CF_DriverControllerComponent driver)
	{
		if (!Replication.IsServer() || !CanStartReturnFromAction(user, driver))
			return false;
		CF_ConvoySession session = GetForPlayer(user);
		session.m_bReturnPending = true;
		session.m_iReturnPendingPolls = 0;
		session.ResetReturnCrossing();
		Print("[ConvoyFollower] CONVOY_RETURN_MANUAL: owner ordered parked line to regroup; waiting for safe roster merge");
		session.ScheduleUnloadPoll();
		return true;
	}

	protected void ResumeUnloadQueueAtAnchor()
	{
		for (int i = 0; i < m_aUnits.Count(); i++)
		{
			CF_DriverControllerComponent unit = m_aUnits[i];
			if (!unit)
				continue;
			unit.CF_SetUnloadSequenceHold(true);
			if (i == 0)
				unit.CF_ResumeFromUnloadQueue();
			else
				unit.CF_HoldForUnloadQueue();
		}
	}

	protected void ResumeOutboundAfterLeave()
	{
		if (!m_bUnloadAnchorLock || m_UnloadHead || m_bUnloadReleaseBlocked)
			return;
		m_bUnloadAnchorLock = false;
		foreach (CF_DriverControllerComponent unit : m_aUnits)
		{
			if (!unit)
				continue;
			unit.CF_SetUnloadSequenceHold(false);
			unit.CF_ResumeFromUnloadQueue();
		}
		Print("[ConvoyFollower] UNLOAD_SEQUENCE_LEFT: unreleased trucks resume ordinary follow; return line remains parked");
	}

	protected void ScheduleUnloadPoll()
	{
		if (!Replication.IsServer() || m_bSessionClosed || m_bUnloadPollScheduled || !GetGame())
			return;
		m_bUnloadPollScheduled = true;
		GetGame().GetCallqueue().CallLater(PollUnloadSequence, CF_UNLOAD_POLL_MS, false);
	}

	protected Vehicle GetOwnerPilotedVehicle()
	{
		PlayerManager manager = GetGame().GetPlayerManager();
		// Game Master may temporarily control another entity. Keep the
		// original character as the convoy owner rather than silently binding
		// the unloading return crossing to an edited or possessed entity.
		if (!manager || manager.GetPlayerControlledEntity(m_iOrderingPlayerId) != m_OrderingPlayer)
			return null;
		ChimeraCharacter player = ChimeraCharacter.Cast(m_OrderingPlayer);
		if (!player || !player.IsInVehicle())
			return null;
		CompartmentAccessComponent access = player.GetCompartmentAccessComponent();
		if (!access)
			return null;
		BaseCompartmentSlot slot = access.GetCompartment();
		if (!slot || !slot.IsPiloting())
			return null;
		return Vehicle.Cast(access.GetVehicleIn(player));
	}

	protected bool IsConvoyVehicle(IEntity vehicle)
	{
		foreach (CF_DriverControllerComponent unit : m_aUnits)
		{
			if (unit && unit.CF_GetAssignedVehicle() == vehicle)
				return true;
		}
		foreach (CF_DriverControllerComponent parked : m_aReturnQueue)
		{
			if (parked && parked.CF_GetAssignedVehicle() == vehicle)
				return true;
		}
		foreach (CF_DriverControllerComponent stranded : m_aStrandedUnits)
		{
			if (stranded && stranded.CF_GetAssignedVehicle() == vehicle)
				return true;
		}
		return false;
	}

	protected void ResetReturnCrossing()
	{
		m_bReturnCrossingArmed = false;
		m_bReturnSampleValid = false;
		m_ReturnMarkerVehicle = null;
		m_vReturnMarkerPosition = vector.Zero;
	}

	protected CF_DriverControllerComponent GetFarthestReturnUnit()
	{
		if (m_aReturnQueue.IsEmpty())
			return null;
		CF_DriverControllerComponent first = m_aReturnQueue[0];
		if (!first || !first.CF_GetAssignedVehicle())
			return null;
		vector firstPosition = first.CF_GetAssignedVehicle().GetOrigin();
		float homeX = firstPosition[0] - m_vUnloadAnchor[0];
		float homeZ = firstPosition[2] - m_vUnloadAnchor[2];
		float length = Math.Sqrt(homeX * homeX + homeZ * homeZ);
		if (length < 10.0)
			return null;
		homeX /= length;
		homeZ /= length;
		CF_DriverControllerComponent farthest = first;
		float farthestProjection = -99999.0;
		foreach (CF_DriverControllerComponent parked : m_aReturnQueue)
		{
			if (!parked || !parked.CF_GetAssignedVehicle())
				continue;
			vector position = parked.CF_GetAssignedVehicle().GetOrigin();
			float projected = (position[0] - m_vUnloadAnchor[0]) * homeX +
				(position[2] - m_vUnloadAnchor[2]) * homeZ;
			if (projected > farthestProjection)
			{
				farthestProjection = projected;
				farthest = parked;
			}
		}
		return farthest;
	}

	protected bool ShouldResumeOutbound(Vehicle ownerVehicle)
	{
		if (m_aUnits.IsEmpty() ||
			vector.Distance(ownerVehicle.GetOrigin(), m_vUnloadAnchor) <= CF_UNLOAD_LEAVE_DISTANCE)
			return false;
		if (m_aReturnQueue.IsEmpty())
			return true;
		CF_DriverControllerComponent first = m_aReturnQueue[0];
		if (!first || !first.CF_GetAssignedVehicle())
			return false;
		vector firstPosition = first.CF_GetAssignedVehicle().GetOrigin();
		float homeX = firstPosition[0] - m_vUnloadAnchor[0];
		float homeZ = firstPosition[2] - m_vUnloadAnchor[2];
		float length = Math.Sqrt(homeX * homeX + homeZ * homeZ);
		if (length < 10.0)
			return false;
		vector ownerPosition = ownerVehicle.GetOrigin();
		float homeward = ((ownerPosition[0] - m_vUnloadAnchor[0]) * homeX +
			(ownerPosition[2] - m_vUnloadAnchor[2]) * homeZ) / length;
		// Driving past the parked return trucks must wait for the return
		// crossing. Only continuing OUTBOUND abandons the unload sequence.
		return homeward <= -CF_UNLOAD_LEAVE_DISTANCE;
	}

	protected void UpdateReleasePermission()
	{
		if (m_aUnits.IsEmpty())
			return;
		CF_DriverControllerComponent front = m_aUnits[0];
		if (!front)
			return;
		bool allowed = !m_bUnloadReleaseBlocked && (m_aReturnQueue.IsEmpty() ||
			vector.Distance(front.CF_GetUnloadAnchor(), m_vUnloadAnchor) <= 20.0);
		front.CF_SetSessionReleaseAllowed(allowed);
	}

	// Reorder only a persistent adjacent pass on a shared straight corridor.
	// A momentary crossing, a curve, or a truck ahead of the player's vehicle
	// must not make the chain turn around to chase a new predecessor.
	protected bool CanSwapAdjacentUnits(int index, Vehicle ownerVehicle)
	{
		if (index < 1 || index >= m_aUnits.Count() || !ownerVehicle)
			return false;
		CF_DriverControllerComponent predecessor = m_aUnits[index - 1];
		CF_DriverControllerComponent overtaker = m_aUnits[index];
		if (!predecessor || !overtaker || !predecessor.CF_IsBoarded() ||
			!overtaker.CF_IsBoarded() || !predecessor.CF_IsMovementActive() ||
			!overtaker.CF_IsMovementActive() || !overtaker.CF_IsOrderInverted())
			return false;
		Vehicle predecessorVehicle = predecessor.CF_GetAssignedVehicle();
		Vehicle overtakerVehicle = overtaker.CF_GetAssignedVehicle();
		if (!predecessorVehicle || !overtakerVehicle)
			return false;

		vector forward = predecessorVehicle.GetWorldTransformAxis(2);
		float forwardLength = Math.Sqrt(forward[0] * forward[0] + forward[2] * forward[2]);
		if (forwardLength < 0.5)
			return false;
		float fx = forward[0] / forwardLength;
		float fz = forward[2] / forwardLength;
		vector otherForward = overtakerVehicle.GetWorldTransformAxis(2);
		float otherLength = Math.Sqrt(otherForward[0] * otherForward[0] +
			otherForward[2] * otherForward[2]);
		if (otherLength < 0.5 ||
			(fx * otherForward[0] + fz * otherForward[2]) / otherLength < 0.8)
			return false;

		vector predecessorPosition = predecessorVehicle.GetOrigin();
		vector overtakerPosition = overtakerVehicle.GetOrigin();
		float dx = overtakerPosition[0] - predecessorPosition[0];
		float dz = overtakerPosition[2] - predecessorPosition[2];
		float ahead = dx * fx + dz * fz;
		float lateral = dx * fz - dz * fx;
		if (lateral < 0)
			lateral = -lateral;
		if (ahead < CF_ORDER_REWIRE_MIN_AHEAD || ahead > CF_ORDER_REWIRE_MAX_AHEAD ||
			lateral > CF_ORDER_REWIRE_MAX_LATERAL)
			return false;

		IEntity vehicleAhead = ownerVehicle;
		if (index > 1 && m_aUnits[index - 2])
			vehicleAhead = m_aUnits[index - 2].CF_GetAssignedVehicle();
		if (!vehicleAhead)
			return false;
		vector aheadForward = vehicleAhead.GetWorldTransformAxis(2);
		float aheadForwardLength = Math.Sqrt(aheadForward[0] * aheadForward[0] +
			aheadForward[2] * aheadForward[2]);
		if (aheadForwardLength < 0.5 ||
			(fx * aheadForward[0] + fz * aheadForward[2]) / aheadForwardLength < 0.7)
			return false;
		vector aheadPosition = vehicleAhead.GetOrigin();
		float behindNewPredecessor = (aheadPosition[0] - overtakerPosition[0]) * fx +
			(aheadPosition[2] - overtakerPosition[2]) * fz;
		float aheadLateral = (aheadPosition[0] - overtakerPosition[0]) * fz -
			(aheadPosition[2] - overtakerPosition[2]) * fx;
		if (aheadLateral < 0)
			aheadLateral = -aheadLateral;
		return behindNewPredecessor >= 10.0 && aheadLateral <= 16.0;
	}

	protected void PollOrdinaryOrderRecovery(Vehicle ownerVehicle)
	{
		if (m_aUnits.Count() < 2 || !ownerVehicle || m_PendingDriver ||
			m_UnloadHead || m_iUnloadPhase != CF_UNLOAD_NONE || m_bUnloadAnchorLock ||
			m_bReturnPending || m_bReturnMerged || !m_aReturnQueue.IsEmpty() ||
			!m_aStrandedUnits.IsEmpty())
		{
			m_OrderInversionCandidate = null;
			m_iOrderInversionPolls = 0;
			m_bOrderRecoveryDeferredLogged = false;
			return;
		}
		if (m_iOrderRewireCooldownPolls > 0)
		{
			m_iOrderRewireCooldownPolls--;
			return;
		}

		bool sawUnsafeInversion = false;
		for (int index = 1; index < m_aUnits.Count(); index++)
		{
			CF_DriverControllerComponent overtaker = m_aUnits[index];
			if (!overtaker || !overtaker.CF_IsOrderInverted())
				continue;
			if (!CanSwapAdjacentUnits(index, ownerVehicle))
			{
				sawUnsafeInversion = true;
				continue;
			}
			m_bOrderRecoveryDeferredLogged = false;
			if (m_OrderInversionCandidate != overtaker)
			{
				m_OrderInversionCandidate = overtaker;
				m_iOrderInversionPolls = 1;
				Print("[ConvoyFollower] ORDER_REWIRE_CANDIDATE: adjacent vehicles are out of order; confirming before relink");
				return;
			}
			m_iOrderInversionPolls++;
			if (m_iOrderInversionPolls < CF_ORDER_REWIRE_CONFIRM_POLLS)
				return;
			CF_DriverControllerComponent displaced = m_aUnits[index - 1];
			m_aUnits[index - 1] = overtaker;
			m_aUnits[index] = displaced;
			m_OrderInversionCandidate = null;
			m_iOrderInversionPolls = 0;
			m_iOrderRewireCooldownPolls = CF_ORDER_REWIRE_COOLDOWN_POLLS;
			Print("[ConvoyFollower] ORDER_REWIRED: physical positions " + index + " and " + (index + 1) +
				" swapped after sustained straight-road inversion");
			RewireTargets();
			return;
		}
		m_OrderInversionCandidate = null;
		m_iOrderInversionPolls = 0;
		if (sawUnsafeInversion && !m_bOrderRecoveryDeferredLogged)
		{
			Print("[ConvoyFollower] ORDER_REWIRE_DEFERRED: inverted vehicles lack an unambiguous straight-road position");
			m_bOrderRecoveryDeferredLogged = true;
		}
		else if (!sawUnsafeInversion)
			m_bOrderRecoveryDeferredLogged = false;
	}

	protected void PollUnloadSequence()
	{
		m_bUnloadPollScheduled = false;
		if (!Replication.IsServer() || m_bSessionClosed)
			return;
		ChimeraCharacter owner = ChimeraCharacter.Cast(m_OrderingPlayer);
		SCR_DamageManagerComponent ownerDamage;
		if (owner)
			ownerDamage = owner.GetDamageManager();
		// Only a confirmed disconnect or the original character's death ends
		// the session. Game Master possession can swap the controlled entity
		// while the original character and convoy remain valid.
		if (!GetOrderingController() || !owner ||
			(ownerDamage && ownerDamage.IsDestroyed()))
		{
			m_iOwnerMissingPolls++;
			if (m_iOwnerMissingPolls >= 15)
			{
				EndOwnerUnavailable();
				return;
			}
			ScheduleUnloadPoll();
			return;
		}
		m_iOwnerMissingPolls = 0;

		Vehicle ownerVehicle = GetOwnerPilotedVehicle();
		PollOrdinaryOrderRecovery(ownerVehicle);
		if (m_aReturnQueue.IsEmpty() && !m_bUnloadAnchorLock && !m_UnloadHead && ownerVehicle &&
			!IsConvoyVehicle(ownerVehicle))
			m_OriginalLeadVehicle = ownerVehicle;

		if (!m_UnloadHead && !m_bUnloadReleaseBlocked && m_bUnloadAnchorLock && ownerVehicle &&
			ShouldResumeOutbound(ownerVehicle))
			ResumeOutboundAfterLeave();
		UpdateReleasePermission();

		if (m_iUnloadPhase != CF_UNLOAD_SHIFTING && !m_aReturnQueue.IsEmpty() &&
			!m_bReturnMerged && !m_bReturnPending)
			CheckReturnCrossing(ownerVehicle);
		if (m_bReturnPending && !m_UnloadHead)
		{
			if (!BeginReturnMerge())
			{
				m_iReturnPendingPolls++;
				if (m_iReturnPendingPolls >= 10)
				{
					m_bReturnPending = false;
					m_iReturnPendingPolls = 0;
					ResetReturnCrossing();
					Print("[ConvoyFollower] CONVOY_RETURN_BLOCKED: turn or roster preflight failed; owner must drive back and retry crossing");
				}
			}
		}

		ScheduleUnloadPoll();
	}

	protected void CheckReturnCrossing(Vehicle ownerVehicle)
	{
		if (!ownerVehicle || IsConvoyVehicle(ownerVehicle))
			return;
		if (!m_OriginalLeadVehicle)
		{
			// A player who exited before the first action can safely bind
			// the current vehicle only while still near the unloading bay.
			if (vector.Distance(ownerVehicle.GetOrigin(), m_vUnloadAnchor) > 18.0)
				return;
			m_OriginalLeadVehicle = ownerVehicle;
		}
		if (ownerVehicle != m_OriginalLeadVehicle)
			return;

		// Pick the farthest homeward parked truck, not the most recently
		// ordered truck by assumption. The marker must have settled.
		foreach (CF_DriverControllerComponent parked : m_aReturnQueue)
		{
			if (!parked || !parked.CF_IsAtUnloadWaitingPoint() || !parked.CF_GetAssignedVehicle())
				return;
		}
		CF_DriverControllerComponent marker = GetFarthestReturnUnit();
		if (!marker || !marker.CF_GetAssignedVehicle())
			return;
		IEntity markerVehicle = marker.CF_GetAssignedVehicle();
		vector markerPosition = markerVehicle.GetOrigin();
		if (markerVehicle != m_ReturnMarkerVehicle ||
			vector.Distance(markerPosition, m_vReturnMarkerPosition) > 1.5)
		{
			ResetReturnCrossing();
			m_ReturnMarkerVehicle = markerVehicle;
			m_vReturnMarkerPosition = markerPosition;
			return;
		}

		float homeX = markerPosition[0] - m_vUnloadAnchor[0];
		float homeZ = markerPosition[2] - m_vUnloadAnchor[2];
		float homeLength = Math.Sqrt(homeX * homeX + homeZ * homeZ);
		if (homeLength < 12.0)
			return;
		homeX /= homeLength;
		homeZ /= homeLength;
		vector playerPosition = ownerVehicle.GetOrigin();
		float offsetX = playerPosition[0] - markerPosition[0];
		float offsetZ = playerPosition[2] - markerPosition[2];
		float signed = offsetX * homeX + offsetZ * homeZ;
		float lateral = offsetX * homeZ - offsetZ * homeX;
		if (lateral < 0)
			lateral = -lateral;
		if (lateral > CF_RETURN_CORRIDOR)
		{
			m_bReturnSampleValid = false;
			return;
		}

		if (signed < -CF_RETURN_ARM_DISTANCE)
			m_bReturnCrossingArmed = true;
		if (m_bReturnCrossingArmed && m_bReturnSampleValid && signed > CF_RETURN_CROSS_DISTANCE)
		{
			float stepX = playerPosition[0] - m_vLastOwnerVehiclePosition[0];
			float stepZ = playerPosition[2] - m_vLastOwnerVehiclePosition[2];
			float stepLength = Math.Sqrt(stepX * stepX + stepZ * stepZ);
			float homewardStep = stepX * homeX + stepZ * homeZ;
			vector forward = ownerVehicle.GetWorldTransformAxis(2);
			float headingHome = forward[0] * homeX + forward[2] * homeZ;
			if (homewardStep >= 1.5 && stepLength <= 35.0 && headingHome >= 0.35)
			{
				m_bReturnPending = true;
				m_iReturnPendingPolls = 0;
				Print("[ConvoyFollower] CONVOY_RETURN_CROSSING: owner passed parked return line heading home");
			}
		}
		m_vLastOwnerVehiclePosition = playerPosition;
		m_bReturnSampleValid = true;
	}

	protected bool BeginReturnMerge()
	{
		if (m_UnloadHead || m_aReturnQueue.IsEmpty() || m_bReturnMerged)
			return false;
		foreach (CF_DriverControllerComponent parked : m_aReturnQueue)
		{
			if (!parked || !parked.CF_CanBeginReturnFollow())
				return false;
		}
		foreach (CF_DriverControllerComponent outbound : m_aUnits)
		{
			if (!outbound || !outbound.CF_CanAbortUnloadForReturn())
				return false;
		}

		CF_DriverControllerComponent first = m_aReturnQueue[0];
		vector firstPosition = first.CF_GetAssignedVehicle().GetOrigin();
		float homeX = firstPosition[0] - m_vUnloadAnchor[0];
		float homeZ = firstPosition[2] - m_vUnloadAnchor[2];
		float length = Math.Sqrt(homeX * homeX + homeZ * homeZ);
		if (length < 10.0)
			return false;
		homeX /= length;
		homeZ /= length;

		ref array<CF_DriverControllerComponent> remainingParked = {};
		ref array<CF_DriverControllerComponent> plannedParked = {};
		foreach (CF_DriverControllerComponent returnUnit : m_aReturnQueue)
			remainingParked.Insert(returnUnit);
		// Order by projection toward home, so the physical front of the
		// parked line follows first even on a bent road.
		while (!remainingParked.IsEmpty())
		{
			int farthestIndex = 0;
			float farthestProjection = -99999.0;
			for (int i = 0; i < remainingParked.Count(); i++)
			{
				vector position = remainingParked[i].CF_GetAssignedVehicle().GetOrigin();
				float projected = (position[0] - m_vUnloadAnchor[0]) * homeX +
					(position[2] - m_vUnloadAnchor[2]) * homeZ;
				if (projected > farthestProjection)
				{
					farthestProjection = projected;
					farthestIndex = i;
				}
			}
			plannedParked.Insert(remainingParked[farthestIndex]);
			remainingParked.RemoveOrdered(farthestIndex);
		}

		ref array<CF_DriverControllerComponent> activated = {};
		ref array<CF_DriverControllerComponent> failedParked = {};
		ref array<CF_DriverControllerComponent> failedOutbound = {};
		CF_DriverControllerComponent predecessor;
		foreach (CF_DriverControllerComponent parkedUnit : plannedParked)
		{
			if (parkedUnit.CF_BeginReturnFollow(this, m_OrderingPlayer, predecessor, activated.Count() + 1))
			{
				activated.Insert(parkedUnit);
				predecessor = parkedUnit;
			}
			else
				failedParked.Insert(parkedUnit);
		}
		foreach (CF_DriverControllerComponent outboundUnit : m_aUnits)
		{
			outboundUnit.CF_SetConvoyTarget(predecessor, activated.Count() + 1);
			if (outboundUnit.CF_AbortUnloadForReturn(m_OrderingPlayer))
			{
				activated.Insert(outboundUnit);
				predecessor = outboundUnit;
			}
			else
			{
				outboundUnit.CF_HoldForUnloadQueue();
				failedOutbound.Insert(outboundUnit);
			}
		}

		// The rare failure after a successful preflight stays in an owned,
		// seated roster. Later followers target the previous SUCCESSFUL
		// truck, never a blocked unit.
		m_aUnits.Clear();
		foreach (CF_DriverControllerComponent active : activated)
			m_aUnits.Insert(active);
		m_aReturnQueue.Clear();
		foreach (CF_DriverControllerComponent parkedFailure : failedParked)
			m_aReturnQueue.Insert(parkedFailure);
		foreach (CF_DriverControllerComponent outboundFailure : failedOutbound)
			m_aStrandedUnits.Insert(outboundFailure);
		m_bUnloadAnchorLock = false;
		m_bUnloadReleaseBlocked = false;
		m_bReturnMerged = true;
		m_bReturnPending = false;
		m_iReturnPendingPolls = 0;
		ResetReturnCrossing();

		SCR_PlayerController controller = GetOrderingController();
		if (controller)
		{
			controller.CF_SetConvoyMemberCount(m_aUnits.Count() + m_aReturnQueue.Count() + m_aStrandedUnits.Count());
			controller.CF_ClearConvoyRadioQueue();
		}
		if (failedParked.IsEmpty() && failedOutbound.IsEmpty())
			Print("[ConvoyFollower] CONVOY_RETURN_MERGED: parked and unreleased trucks following owner in road order");
		else
		{
			Print("[ConvoyFollower] CONVOY_RETURN_PARTIAL: failed trucks remain seated and owned; active chain skips them");
			// A partial roster activation is a control failure, not a proved
			// vehicle stall. Keep the affected drivers seated for recovery.
		}
		return true;
	}
	protected bool BeginPending(CF_DriverControllerComponent candidate, int operation)
	{
		if (!candidate || m_PendingDriver)
			return false;

		m_PendingDriver = candidate;
		m_iPendingOperation = operation;
		m_LeaderBeingReplaced = null;
		if (operation == CF_PENDING_REPLACE && !m_aUnits.IsEmpty())
			m_LeaderBeingReplaced = m_aUnits[0];
		if (candidate.CF_BeginConvoyAssignment(this, m_OrderingPlayer))
			return true;

		m_PendingDriver = null;
		m_LeaderBeingReplaced = null;
		m_iPendingOperation = CF_PENDING_NONE;
		return false;
	}

	void OnDriverBoarded(CF_DriverControllerComponent driver)
	{
		if (!Replication.IsServer() || !driver || driver != m_PendingDriver || !driver.CF_IsBoarded())
			return;

		int operation = m_iPendingOperation;
		CF_DriverControllerComponent oldLeader = m_LeaderBeingReplaced;
		m_PendingDriver = null;
		m_LeaderBeingReplaced = null;
		m_iPendingOperation = CF_PENDING_NONE;

		if (operation == CF_PENDING_START)
		{
			m_aUnits.Insert(driver);
			RegisterIdentity(driver, 1);
			Print("[ConvoyFollower] CONVOY_STARTED: Unit One is ready");
		}
		else if (operation == CF_PENDING_ADD)
		{
			m_aUnits.Insert(driver);
			RegisterIdentity(driver, 0);
			Print("[ConvoyFollower] CONVOY_ADDED: unit " + m_aUnits.Count() + " is ready");
		}
		else if (operation == CF_PENDING_REPLACE)
		{
			int inheritedIdentity = GetIdentityNumber(oldLeader);
			if (inheritedIdentity <= 0)
				inheritedIdentity = 1;
			if (m_aUnits.IsEmpty())
			{
				m_aUnits.Insert(driver);
			}
			else if (m_aUnits[0] == oldLeader)
			{
				m_aUnits[0] = driver;
				// The old leader leaves only after the new driver's seat has
				// been verified; the rest of the chain stays in the convoy.
				if (oldLeader && oldLeader != driver)
				{
					RemoveIdentity(oldLeader);
					oldLeader.CF_StandDownFromSession();
				}
			}
			else
			{
				// The intended old leader disappeared while the candidate
				// was boarding. Promote the candidate without ejecting the unit
				// that temporarily became the head of the chain.
				int previousCount = m_aUnits.Count();
				m_aUnits.Insert(driver);
				for (int i = previousCount; i > 0; i--)
					m_aUnits[i] = m_aUnits[i - 1];
				m_aUnits[0] = driver;
			}
			RegisterIdentity(driver, inheritedIdentity);
			Print("[ConvoyFollower] CONVOY_LEADER_REPLACED: former leader standing down");
		}
		else
		{
			driver.CF_StandDownFromSession();
			return;
		}

		RewireTargets();
	}

	void OnDriverUnavailable(CF_DriverControllerComponent driver)
	{
		if (!Replication.IsServer() || !driver)
			return;
		if (driver == m_UnloadHead)
			OnUnloadDepartureFailed(driver);
		else if (driver == m_ShiftingReturn)
			FailUnloadSequence("parked return truck became unavailable while shifting");
		bool removedReturn = false;
		for (int j = m_aReturnQueue.Count() - 1; j >= 0; j--)
		{
			if (m_aReturnQueue[j] == driver)
			{
				m_aReturnQueue.RemoveOrdered(j);
				removedReturn = true;
				ResetReturnCrossing();
			}
		}
		for (int j = m_aStrandedUnits.Count() - 1; j >= 0; j--)
		{
			if (m_aStrandedUnits[j] == driver)
			{
				m_aStrandedUnits.RemoveOrdered(j);
				removedReturn = true;
			}
		}
		RemoveIdentity(driver);

		if (driver == m_PendingDriver)
		{
			Print("[ConvoyFollower] CONVOY_PENDING_FAILED: candidate could not board or became unavailable");
			m_PendingDriver = null;
			m_LeaderBeingReplaced = null;
			m_iPendingOperation = CF_PENDING_NONE;
		}

		int index = FindUnit(driver);
		if (index >= 0)
		{
			m_aUnits.RemoveOrdered(index);
			if (index == 0)
				m_bUnloadReleaseBlocked = false;
			Print("[ConvoyFollower] CONVOY_UNIT_REMOVED: former position " + (index + 1));
			RewireTargets();
			if (m_bUnloadAnchorLock)
				ResumeUnloadQueueAtAnchor();
		}
		else if (removedReturn)
			RewireTargets();

		if (m_aUnits.IsEmpty() && m_aReturnQueue.IsEmpty() && m_aStrandedUnits.IsEmpty() && !m_PendingDriver)
			CloseSession();
	}

	void OnDriverStatus(CF_DriverControllerComponent driver, int eventId)
	{
		if (!Replication.IsServer() || !driver)
			return;

		int index = FindUnit(driver);
		if (index < 0)
			return;

		SCR_PlayerController controller = GetOrderingController();
		if (!controller)
			return;

		if (index == 0)
		{
			// Unit One is the only audible speaker. These clips use plural
			// 'we/us' language because they speak for the whole convoy.
			bool routine = eventId == CF_RadioEvent.READY || eventId == CF_RadioEvent.FOLLOWING ||
				eventId == CF_RadioEvent.HOLDING;
			if (routine && Math.RandomInt(0, 100) >= CF_ConvoySettings.Get().m_iRoutineCallChancePercent)
			{
				Print("[ConvoyFollower] RADIO_ROUTINE_OMITTED: event " + eventId);
				return;
			}

			if (routine || eventId == CF_RadioEvent.FAR_WARNING ||
				eventId == CF_RadioEvent.STUCK || eventId == CF_RadioEvent.LOST ||
				eventId == CF_RadioEvent.REJOINED || eventId == CF_RadioEvent.UNDER_FIRE)
			{
				Print("[ConvoyFollower] RADIO_AGGREGATE: event " + eventId);
				controller.CF_SendConvoyRadioCall(eventId, 0);
			}
			return;
		}

		// Routine boarding and movement changes from later units do not
		// interrupt the player. Exceptions are relayed in the leader's voice.
		if (eventId == CF_RadioEvent.READY || eventId == CF_RadioEvent.FOLLOWING || eventId == CF_RadioEvent.HOLDING)
			return;

		if (eventId != CF_RadioEvent.FAR_WARNING && eventId != CF_RadioEvent.STUCK &&
			eventId != CF_RadioEvent.LOST && eventId != CF_RadioEvent.REJOINED &&
			eventId != CF_RadioEvent.UNDER_FIRE)
			return;

		int unitNumber = GetIdentityNumber(driver);
		if (unitNumber <= 0 || unitNumber > 10)
		{
			// Never play the wrong unit number if an older or modified roster
			// somehow exceeds the available numbered recordings.
			Print("[ConvoyFollower] RADIO_UNSUPPORTED_UNIT: unit " + unitNumber + ", event " + eventId);
			return;
		}

		Print("[ConvoyFollower] RADIO_RELAY: unit " + unitNumber + ", event " + eventId);
		controller.CF_SendConvoyRadioCall(eventId, unitNumber);
	}

	static bool CanStandDown(IEntity user, CF_DriverControllerComponent driver)
	{
		CF_ConvoySession session = GetForPlayer(user);
		if (!session || !driver || !driver.CanStandDown())
			return false;
		if (session.m_UnloadHead && session.m_UnloadHead != driver)
			return false;
		// Prevent a manual order from changing the roster while a new unit
		// is boarding. Spontaneous driver loss is still repaired below.
		if (session.m_PendingDriver && session.m_PendingDriver != driver)
			return false;
		return session.FindUnit(driver) >= 0 || session.m_aReturnQueue.Contains(driver) ||
			session.m_aStrandedUnits.Contains(driver) || session.m_PendingDriver == driver;
	}

	static bool StandDown(IEntity user, CF_DriverControllerComponent driver)
	{
		if (!Replication.IsServer() || !CanStandDown(user, driver))
			return false;

		CF_ConvoySession session = GetForPlayer(user);
		if (session.m_UnloadHead == driver)
			session.OnUnloadDepartureFailed(driver);
		if (session.m_PendingDriver == driver)
		{
			session.m_PendingDriver = null;
			session.m_LeaderBeingReplaced = null;
			session.m_iPendingOperation = CF_PENDING_NONE;
		}

		int index = session.FindUnit(driver);
		bool resumeAtAnchor = session.m_bUnloadAnchorLock && index >= 0;
		if (index >= 0)
		{
			session.m_aUnits.RemoveOrdered(index);
			session.RemoveIdentity(driver);
			session.m_bUnloadReleaseBlocked = false;
		}
		for (int j = session.m_aReturnQueue.Count() - 1; j >= 0; j--)
		{
			if (session.m_aReturnQueue[j] == driver)
			{
				session.m_aReturnQueue.RemoveOrdered(j);
				session.RemoveIdentity(driver);
				session.ResetReturnCrossing();
			}
		}
		for (int k = session.m_aStrandedUnits.Count() - 1; k >= 0; k--)
		{
			if (session.m_aStrandedUnits[k] == driver)
			{
				session.m_aStrandedUnits.RemoveOrdered(k);
				session.RemoveIdentity(driver);
			}
		}

		driver.CF_StandDownFromSession();
		if (session.m_aUnits.IsEmpty() && session.m_aReturnQueue.IsEmpty() &&
			session.m_aStrandedUnits.IsEmpty() && !session.m_PendingDriver)
			session.CloseSession();
		else
		{
			session.RewireTargets();
			if (resumeAtAnchor)
				session.ResumeUnloadQueueAtAnchor();
		}
		return true;
	}

	int GetOrderingPlayerId()
	{
		return m_iOrderingPlayerId;
	}

	int GetUnitNumber(CF_DriverControllerComponent driver)
	{
		int index = FindUnit(driver);
		if (index < 0)
			return 0;

		return index + 1;
	}

	bool IsCurrentLeader(CF_DriverControllerComponent driver)
	{
		return !m_aUnits.IsEmpty() && m_aUnits[0] == driver;
	}

	protected int FindUnit(CF_DriverControllerComponent driver)
	{
		for (int i = 0; i < m_aUnits.Count(); i++)
		{
			if (m_aUnits[i] == driver)
				return i;
		}

		return -1;
	}

	protected int GetIdentityNumber(CF_DriverControllerComponent driver)
	{
		for (int i = 0; i < m_aIdentityDrivers.Count(); i++)
		{
			if (m_aIdentityDrivers[i] == driver)
				return m_aIdentityNumbers[i];
		}
		return 0;
	}

	protected void RegisterIdentity(CF_DriverControllerComponent driver, int requested)
	{
		if (!driver || GetIdentityNumber(driver) > 0)
			return;
		int identity = requested;
		if (identity <= 0)
		{
			for (int candidate = 2; candidate <= 10; candidate++)
			{
				if (!m_aIdentityNumbers.Contains(candidate))
				{
					identity = candidate;
					break;
				}
			}
		}
		if (identity <= 0 || identity > 10)
			return;
		m_aIdentityDrivers.Insert(driver);
		m_aIdentityNumbers.Insert(identity);
	}

	protected void RemoveIdentity(CF_DriverControllerComponent driver)
	{
		for (int i = m_aIdentityDrivers.Count() - 1; i >= 0; i--)
		{
			if (m_aIdentityDrivers[i] == driver)
			{
				m_aIdentityDrivers.RemoveOrdered(i);
				m_aIdentityNumbers.RemoveOrdered(i);
			}
		}
	}

	protected void RewireTargets()
	{
		CF_DriverControllerComponent predecessor;
		for (int i = 0; i < m_aUnits.Count(); i++)
		{
			CF_DriverControllerComponent unit = m_aUnits[i];
			if (!unit)
				continue;

			unit.CF_SetConvoyTarget(predecessor, i + 1);
			predecessor = unit;
		}
		UpdateReleasePermission();

		SCR_PlayerController controller = GetOrderingController();
		if (controller)
		{
			controller.CF_SetConvoyMemberCount(m_aUnits.Count() + m_aReturnQueue.Count() + m_aStrandedUnits.Count());
			controller.CF_ClearConvoyRadioQueue();
		}
	}

	protected SCR_PlayerController GetOrderingController()
	{
		PlayerManager manager = GetGame().GetPlayerManager();
		if (!manager)
			return null;

		return SCR_PlayerController.Cast(manager.GetPlayerController(m_iOrderingPlayerId));
	}

	// A disconnected player or dead original character must not leave a
	// parked return line polling forever. Release all drivers it owned.
	protected void EndOwnerUnavailable()
	{
		ref array<CF_DriverControllerComponent> drivers = {};
		foreach (CF_DriverControllerComponent active : m_aUnits)
		{
			if (active && !drivers.Contains(active))
				drivers.Insert(active);
		}
		foreach (CF_DriverControllerComponent parked : m_aReturnQueue)
		{
			if (parked && !drivers.Contains(parked))
				drivers.Insert(parked);
		}
		foreach (CF_DriverControllerComponent stranded : m_aStrandedUnits)
		{
			if (stranded && !drivers.Contains(stranded))
				drivers.Insert(stranded);
		}
		if (m_PendingDriver && !drivers.Contains(m_PendingDriver))
			drivers.Insert(m_PendingDriver);

		m_aUnits.Clear();
		m_PendingDriver = null;
		CloseSession();
		foreach (CF_DriverControllerComponent driver : drivers)
			driver.CF_StandDownFromSession();
		Print("[ConvoyFollower] CONVOY_OWNER_LEFT: ended old convoy after owner disconnected or original character died");
	}

	protected void CloseSession()
	{
		m_bSessionClosed = true;
		m_UnloadHead = null;
		m_iUnloadPhase = CF_UNLOAD_NONE;
		m_aReturnQueue.Clear();
		m_aStrandedUnits.Clear();
		m_aIdentityDrivers.Clear();
		m_aIdentityNumbers.Clear();
		SCR_PlayerController controller = GetOrderingController();
		if (controller)
		{
			controller.CF_SetConvoyMemberCount(0);
			controller.CF_ClearConvoyRadioQueue();
		}

		Print("[ConvoyFollower] CONVOY_ENDED: no units remain");
		if (s_mSessions.Contains(m_iOrderingPlayerId) && s_mSessions.Get(m_iOrderingPlayerId) == this)
			s_mSessions.Remove(m_iOrderingPlayerId);
	}
}
