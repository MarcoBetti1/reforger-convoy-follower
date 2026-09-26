class CF_LoadedPairCargoProbeComponentClass : CF_NativeCargoActionProbeComponentClass
{
}

// Passive until the one coordinator grants this truck the owner's next action.
// Native discovery/reach/queues/start/cancel are inherited from the v6 probe.
class CF_LoadedPairCargoProbeComponent : CF_NativeCargoActionProbeComponent
{
	[Attribute(defvalue: "1", desc: "Exact original follower number, 1 or 2")]
	protected int m_iPairUnit;
	protected IEntity m_Destination;
	protected SCR_ResourceContainer m_DestinationContainer;
	protected float m_fDestinationBefore;
	protected float m_fWorkerBeginMs;
	protected float m_fWorkerStableMs;
	protected int m_iWorkerState; // 0 idle, 1 eligibility, 2 transfer, 3 cancelled stability, 4 done
	protected bool m_bWorkerLoading;
	protected bool m_bPairFailed;
	protected bool m_bPairLoaded;
	protected bool m_bPairDelivered;

	int PairUnit() { return m_iPairUnit; }
	Vehicle PairTruck() { return m_Truck; }
	ChimeraCharacter PairOwner() { return m_Player; }
	SCR_ResourceContainer PairSource() { return m_SourceContainer; }
	SCR_ResourceContainer PairCargo() { return m_CargoContainer; }
	SCR_ResourceContainer PairDestination() { return m_DestinationContainer; }
	bool PairFailed() { return m_bPairFailed; }
	bool PairLoaded() { return m_bPairLoaded; }
	bool PairDelivered() { return m_bPairDelivered; }
	bool PairBusy() { return m_iWorkerState > 0 && m_iWorkerState < 4; }
	bool PairInactive() { return m_Load && m_Unload && !m_Load.IsInProgress() && !m_Unload.IsInProgress() && !m_Active; }

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer()) return;
		m_World = GetGame().GetWorld();
		m_Truck = Vehicle.Cast(owner);
		m_fBeginMs = m_World.GetWorldTime();
		m_sRun = Key(owner) + "_" + m_fBeginMs;
		// No independent callback: the coordinator alone advances native actions.
		Print("[ConvoyFollower] PAIR_CARGO_INIT: run_id=" + m_sRun + " unit=" + m_iPairUnit + " native=true passive=true count_writes=false truck_pose_writes=false");
	}

	bool BindPair()
	{
		if (m_bPairFailed || (m_iPairUnit != 1 && m_iPairUnit != 2)) return false;
		if (!m_bBound && !super.Bind()) return false;
		if (!m_Destination)
		{
			m_Destination = m_World.FindEntityByName("CF_LoadedTripDestination");
			SCR_ResourceComponent resources;
			if (m_Destination) resources = SCR_ResourceComponent.Cast(m_Destination.FindComponent(SCR_ResourceComponent));
			if (resources) m_DestinationContainer = resources.GetContainer(EResourceType.SUPPLIES);
		}
		if (!m_DestinationContainer || m_Destination == m_Source || m_DestinationContainer == m_SourceContainer ||
			m_DestinationContainer == m_CargoContainer || vector.DistanceXZ(m_Source.GetOrigin(), m_Destination.GetOrigin()) < 250)
			return false;
		return CF_HasCargoBinding();
	}

	override protected bool CF_HasCargoBinding()
	{
		if (!GetGame() || GetGame().GetWorld() != m_World || CF_ConvoySession.CF_IsWorldCleanup()) return false;
		if (!m_Truck || GetOwner() != m_Truck || m_World.FindEntityByName("CF_SmokeFollower" + m_iPairUnit) != m_Truck ||
			!m_Source || m_World.FindEntityByName("CF_CargoActionSource") != m_Source ||
			!m_Destination || m_World.FindEntityByName("CF_LoadedTripDestination") != m_Destination) return false;
		if (!m_Local || !GetGame().GetPlayerManager() || GetGame().GetPlayerController() != m_Local ||
			!m_Player || m_Local.GetControlledEntity() != m_Player ||
			GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(m_Player) != m_Local.GetPlayerId()) return false;
		return Under(m_CargoOwner, m_Truck) && m_Load && m_Unload && m_Manager && m_Rear &&
			m_Load.GetOwner() == m_CargoOwner && m_Unload.GetOwner() == m_CargoOwner &&
			m_Load.GetActionsManager() == m_Manager && m_Unload.GetActionsManager() == m_Manager &&
			m_Manager.GetContext("door_rear") == m_Rear && Contains(m_Rear, m_Load) && Contains(m_Rear, m_Unload);
	}

	bool PairValues(out float source, out float cargo, out float destination)
	{
		if (!CF_HasCargoBinding() || !Counts(source, cargo) || !m_DestinationContainer || m_DestinationContainer.GetOwner() != m_Destination) return false;
		SCR_ResourceComponent resources = SCR_ResourceComponent.Cast(m_Destination.FindComponent(SCR_ResourceComponent));
		if (!resources || resources.GetContainer(EResourceType.SUPPLIES) != m_DestinationContainer) return false;
		destination = m_DestinationContainer.GetResourceValue();
		return destination >= 0 && destination <= m_DestinationContainer.GetMaxResourceValue() &&
			m_DestinationContainer.GetMaxResourceValue() >= 200 && destination < 100000;
	}

	override protected bool Queues(bool loading, bool log = false)
	{
		if (loading) return super.Queues(true, log);
		return Queue(m_UnloadConsumer.GetContainerQueue(), m_CargoContainer, "pair_unload_original_cargo", log) &&
			Queue(m_UnloadGenerator.GetContainerQueue(), m_DestinationContainer, "pair_unload_exact_destination", log);
	}

	override protected bool StaticTruck()
	{
		CarControllerComponent car = CarControllerComponent.Cast(m_Truck.FindComponent(CarControllerComponent));
		return car && car.GetSimulation() && Math.AbsFloat(car.GetSimulation().GetSpeedKmh()) <= 0.5 &&
			vector.Distance(m_vTruckOrigin, m_Truck.GetOrigin()) <= 0.5;
	}

	bool BeginPairAction(bool loading)
	{
		if (m_bPairFailed || PairBusy() || !PairInactive() || !CF_HasCargoBinding() || m_Player.IsInVehicle()) return false;
		if ((loading && m_bPairLoaded) || (!loading && (!m_bPairLoaded || m_bPairDelivered))) return false;
		CompartmentAccessComponent access = m_Player.GetCompartmentAccessComponent();
		if (!access || access.IsGettingIn() || access.IsGettingOut()) return false;
		if (!PairValues(m_fSourceStart, m_fCargoStart, m_fDestinationBefore)) return false;
		if ((loading && m_fCargoStart != 0) || (!loading && Math.AbsFloat(m_fCargoStart - 100) > TOLERANCE)) return false;
		vector rear[4];
		if (!m_Rear.GetTransformationWorld(rear)) return false;
		vector position = rear[3] + rear[2] * 1.2;
		position[1] = m_World.GetSurfaceY(position[0], position[2]) + 0.05;
		m_Player.SetOrigin(position);
		m_vTruckOrigin = m_Truck.GetOrigin();
		m_bWorkerLoading = loading;
		m_iWorkerState = 1;
		m_iStage = 3;
		if (loading) m_iStage = 1;
		m_bStartupReadyTracking = false;
		m_bUnloadEligibilityPending = false;
		m_fWorkerBeginMs = m_World.GetWorldTime();
		m_fStageMs = m_fWorkerBeginMs;
		Print("[ConvoyFollower] PAIR_CARGO_REQUEST: run_id=" + m_sRun + " unit=" + m_iPairUnit + " loading=" + loading +
			" truck_id=" + Key(m_Truck) + " owner_id=" + Key(m_Player) + " source_id=" + Key(m_Source) +
			" cargo_id=" + Key(m_CargoOwner) + " destination_id=" + Key(m_Destination) + " owner_origin=" + position +
			" truck_origin=" + m_vTruckOrigin + " source_distance=" + vector.DistanceXZ(m_Source.GetOrigin(), m_Truck.GetOrigin()) +
			" destination_distance=" + vector.DistanceXZ(m_Destination.GetOrigin(), m_Truck.GetOrigin()) + " owner_staging=test_only completion_claim=false");
		return true;
	}

	void TickPairAction(float timeSlice)
	{
		if (!PairBusy() || m_bPairFailed) return;
		float now = m_World.GetWorldTime();
		if (!CF_HasCargoBinding() || !StaticTruck() || now - m_fWorkerBeginMs > 30000)
		{ Finish(false, "binding_stationary_or_30s_transaction_timeout"); return; }
		float source, cargo, destination;
		if (!PairValues(source, cargo, destination)) { Finish(false, "exact_container_lost"); return; }
		float amount = cargo - m_fCargoStart;
		if (!m_bWorkerLoading) amount = m_fCargoStart - cargo;
		if (amount < -TOLERANCE || amount > 100 + TOLERANCE) { Finish(false, "wrong_direction_or_over_100"); return; }
		if (m_bWorkerLoading && (Math.AbsFloat(source - m_fSourceStart + amount) > TOLERANCE || Math.AbsFloat(destination - m_fDestinationBefore) > TOLERANCE))
		{ Finish(false, "load_nonparticipant_or_source_delta"); return; }
		if (!m_bWorkerLoading && (Math.AbsFloat(source - m_fSourceStart) > TOLERANCE || Math.AbsFloat(destination - m_fDestinationBefore - amount) > TOLERANCE))
		{ Finish(false, "unload_nonparticipant_or_destination_delta"); return; }
		if (m_iWorkerState == 1)
		{
			if (Math.AbsFloat(amount) > TOLERANCE || !PairInactive()) { Finish(false, "change_before_native_start"); return; }
			if (!CF_PrepareCargoPlayMode()) { m_bStartupReadyTracking = false; return; }
			if (m_bWorkerLoading)
			{
				if (!Reach(m_Load) || !Queues(true)) { m_bStartupReadyTracking = false; return; }
				if (!CF_CargoStartupReady(now)) return;
			}
			else if (now - m_fWorkerBeginMs < 3000) return;
			Start(m_bWorkerLoading, now);
			if (m_Active && !m_bPairFailed) m_iWorkerState = 2;
			return;
		}
		if (m_iWorkerState == 2)
		{
			if (amount > TOLERANCE)
			{
				Cancel("pair_first_positive_transfer");
				if (Math.AbsFloat(amount - 100) > TOLERANCE || !Queues(m_bWorkerLoading, true)) { Finish(false, "transfer_not_exact_100_or_queue_changed"); return; }
				m_iWorkerState = 3;
				m_fWorkerStableMs = now;
				Print("[ConvoyFollower] PAIR_CARGO_TRANSFER: run_id=" + m_sRun + " unit=" + m_iPairUnit + " loading=" + m_bWorkerLoading + " amount=" + amount + " native_cancelled=true source=" + source + " cargo=" + cargo + " destination=" + destination);
				return;
			}
			if (now - m_fStageMs > 15000 || !(timeSlice > 0 && timeSlice <= 0.25) || !Reach(m_Active) || !Queues(m_bWorkerLoading))
			{ Finish(false, "active_action_guard_" + m_sReason); return; }
			m_fProgress = m_Active.GetActionProgress(m_fProgress, timeSlice);
			m_Player.DoPerformContinuousObjectAction(m_Active, timeSlice);
			return;
		}
		if (Math.AbsFloat(amount - 100) > TOLERANCE || !PairInactive()) { Finish(false, "post_cancel_not_stable"); return; }
		if (now - m_fWorkerStableMs < 3000) return;
		m_iWorkerState = 4;
		if (m_bWorkerLoading) m_bPairLoaded = true;
		else m_bPairDelivered = true;
		Print("[ConvoyFollower] PAIR_CARGO_STABLE: run_id=" + m_sRun + " unit=" + m_iPairUnit + " loading=" + m_bWorkerLoading + " stable_s=" + (now - m_fWorkerStableMs) / 1000 + " amount=100");
	}

	void StopPairAction(string reason) { Cancel("pair_coordinator_" + reason); }

	override protected void Finish(bool pass, string reason)
	{
		if (m_bPairFailed) return;
		Cancel(reason);
		m_bPairFailed = true;
		m_bFinished = true;
		Print("[ConvoyFollower] PAIR_CARGO_FAILURE: run_id=" + m_sRun + " unit=" + m_iPairUnit + " reason=" + reason);
	}

	override void EOnPostFrame(IEntity owner, float timeSlice) { }
}
