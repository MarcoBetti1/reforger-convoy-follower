class CF_LoadedTripCargoProbeComponentClass : CF_NativeCargoActionProbeComponentClass
{
}

// Reuses the proven native action discovery, rear-context guards, exact queues,
// continuous action dispatch and cancellation. No resource or truck pose writes.
class CF_LoadedTripCargoProbeComponent : CF_NativeCargoActionProbeComponent
{
	protected IEntity m_Destination;
	protected SCR_ResourceContainer m_PhysicalSource;
	protected SCR_ResourceContainer m_PhysicalDestination;
	protected IEntity m_PhysicalSourceOwner;
	protected IEntity m_PhysicalDestinationOwner;
	protected void CF_ReadPhysicalStorage(IEntity entity, array<SCR_ResourceContainer> found)
	{
		if (!entity) return;
		SCR_ResourceComponent resources = SCR_ResourceComponent.Cast(entity.FindComponent(SCR_ResourceComponent));
		array<SCR_ResourceContainer> containers;
		if (resources) containers = resources.GetContainers();
		if (containers) foreach (SCR_ResourceContainer c : containers)
			if (c && c.GetResourceType() == EResourceType.SUPPLIES && !SCR_ResourceContainerVirtual.Cast(c) && c.GetOwner() == entity && !found.Contains(c)) found.Insert(c);
		IEntity child = entity.GetChildren();
		while (child) { CF_ReadPhysicalStorage(child, found); child = child.GetSibling(); }
	}

	protected SCR_ResourceContainer m_DestinationContainer;
	protected float m_fDestinationStart;
	protected float m_fDestinationHeld;
	protected bool m_bTripFailed;
	protected bool m_bTripLoaded;
	protected bool m_bTripDelivered;
	protected bool m_bUnloadRequested;
	protected float m_fUnloadRequestedMs;
	protected float m_fTripLastSampleMs;

	bool TripFailed() { return m_bTripFailed; }
	bool TripLoaded() { return m_bTripLoaded; }
	bool TripDelivered() { return m_bTripDelivered; }
	ChimeraCharacter TripOwner() { return m_Player; }

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer()) return;
		m_World = GetGame().GetWorld();
		m_Truck = Vehicle.Cast(owner);
		m_fBeginMs = m_World.GetWorldTime();
		m_sRun = Key(owner) + "_" + m_fBeginMs;
		SetEventMask(owner, EntityEvent.POSTFRAME);
		Print("[ConvoyFollower] LOADED_TRIP_CARGO_INIT: run_id=" + m_sRun +
			" truck_name=CF_SmokeFollower1 source_name=CF_JourneySource destination_name=CF_JourneyDestination" +
			" transfer=100 native_dispatch=true resource_writes=false truck_pose_writes=false ordinary_input=false");
	}

	override protected bool Bind()
	{
		if (!super.Bind()) return false;
		m_Destination = m_World.FindEntityByName("CF_JourneyDestination");
		SCR_ResourceComponent resources;
		if (m_Destination) resources = SCR_ResourceComponent.Cast(m_Destination.FindComponent(SCR_ResourceComponent));
		if (resources) m_DestinationContainer = resources.GetContainer(EResourceType.SUPPLIES);
		if (!m_DestinationContainer || m_Destination == m_Source || m_DestinationContainer == m_SourceContainer ||
			m_DestinationContainer == m_CargoContainer || vector.DistanceXZ(m_Destination.GetOrigin(), m_Source.GetOrigin()) < 15)
		{ Finish(false, "focused_distinct_bay_destination_binding_failed"); return false; }
		array<SCR_ResourceContainer> sourcePhysical = {}, destinationPhysical = {};
		CF_ReadPhysicalStorage(m_Source, sourcePhysical);
		CF_ReadPhysicalStorage(m_Destination, destinationPhysical);
		if (sourcePhysical.Count() != 1 || destinationPhysical.Count() != 1 || sourcePhysical[0] == destinationPhysical[0])
		{ Finish(false, "unique_original_physical_storage_binding_failed"); return false; }
		m_PhysicalSource = sourcePhysical[0];
		m_PhysicalDestination = destinationPhysical[0];
		m_PhysicalSourceOwner = m_PhysicalSource.GetOwner();
		m_PhysicalDestinationOwner = m_PhysicalDestination.GetOwner();
		Print("[ConvoyFollower] LOADED_TRIP_PHYSICAL_BIND: source_root=" + Key(m_Source) + " source_proxy_type=" + m_SourceContainer.Type().ToString() +
			" source_physical_owner=" + Key(m_PhysicalSourceOwner) + " destination_root=" + Key(m_Destination) +
			" destination_proxy_type=" + m_DestinationContainer.Type().ToString() + " destination_physical_owner=" + Key(m_PhysicalDestinationOwner) + " read_only=true");
		Print("[ConvoyFollower] LOADED_TRIP_STORAGES: run_id=" + m_sRun + " source_id=" + Key(m_Source) +
			" destination_id=" + Key(m_Destination) + " cargo_owner_id=" + Key(m_CargoOwner) +
			" source_origin=" + m_Source.GetOrigin() + " destination_origin=" + m_Destination.GetOrigin());
		return true;
	}

	override protected bool CF_HasCargoBinding()
	{
		if (!GetGame() || GetGame().GetWorld() != m_World || CF_ConvoySession.CF_IsWorldCleanup()) return false;
		if (!m_Truck || m_World.FindEntityByName("CF_SmokeFollower1") != m_Truck ||
			!m_Source || m_World.FindEntityByName("CF_JourneySource") != m_Source ||
			!m_Destination || m_World.FindEntityByName("CF_JourneyDestination") != m_Destination) return false;
		if (!m_Local || !GetGame().GetPlayerManager() || GetGame().GetPlayerController() != m_Local || !m_Player || m_Local.GetControlledEntity() != m_Player) return false;
		if (GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(m_Player) != m_Local.GetPlayerId()) return false;
		if (!Under(m_CargoOwner, m_Truck) || !m_Load || !m_Unload || !m_Manager || !m_Rear) return false;
		return m_Load.GetOwner() == m_CargoOwner && m_Unload.GetOwner() == m_CargoOwner &&
			m_Load.GetActionsManager() == m_Manager && m_Unload.GetActionsManager() == m_Manager &&
			m_Manager.GetContext("door_rear") == m_Rear && Contains(m_Rear, m_Load) && Contains(m_Rear, m_Unload);
	}

	override protected bool Queues(bool loading, bool log = false)
	{
		if (loading) return super.Queues(true, log);
		return Queue(m_UnloadConsumer.GetContainerQueue(), m_CargoContainer, "unload_original_cargo", log) &&
			Queue(m_UnloadGenerator.GetContainerQueue(), m_DestinationContainer, "unload_distinct_destination", log);
	}

	protected bool TripCounts(out float source, out float cargo, out float destination)
	{
		if (!Counts(source, cargo) || !m_DestinationContainer || m_DestinationContainer.GetOwner() != m_Destination) return false;
		SCR_ResourceComponent resources = SCR_ResourceComponent.Cast(m_Destination.FindComponent(SCR_ResourceComponent));
		if (!resources || resources.GetContainer(EResourceType.SUPPLIES) != m_DestinationContainer) return false;
		destination = m_DestinationContainer.GetResourceValue();
		array<SCR_ResourceContainer> currentSource = {}, currentDestination = {};
		CF_ReadPhysicalStorage(m_Source, currentSource);
		CF_ReadPhysicalStorage(m_Destination, currentDestination);
		if (currentSource.Count() != 1 || currentDestination.Count() != 1 || currentSource[0] != m_PhysicalSource || currentDestination[0] != m_PhysicalDestination ||
			m_PhysicalSource.GetOwner() != m_PhysicalSourceOwner || m_PhysicalDestination.GetOwner() != m_PhysicalDestinationOwner) return false;
		if (Math.AbsFloat(source - m_PhysicalSource.GetResourceValue()) > TOLERANCE || Math.AbsFloat(destination - m_PhysicalDestination.GetResourceValue()) > TOLERANCE) return false;
		if (source < 0 || source > m_PhysicalSource.GetMaxResourceValue() || destination < 0 || destination > m_PhysicalDestination.GetMaxResourceValue()) return false;

		return destination >= 0 && destination <= m_DestinationContainer.GetMaxResourceValue() && destination < 100000;
	}

	override protected bool StaticTruck()
	{
		CarControllerComponent car = CarControllerComponent.Cast(m_Truck.FindComponent(CarControllerComponent));
		return car && car.GetSimulation() && Math.AbsFloat(car.GetSimulation().GetSpeedKmh()) <= 0.5 &&
			(m_iStage == 0 || vector.Distance(m_vTruckOrigin, m_Truck.GetOrigin()) <= 0.5);
	}

	// Explicit test-owner staging only. Native driver and every truck stay put.
	bool StageOwnerAtRear()
	{
		if (!CF_HasCargoBinding() || m_Player.IsInVehicle()) return false;
		CompartmentAccessComponent access = m_Player.GetCompartmentAccessComponent();
		if (!access || access.IsGettingIn() || access.IsGettingOut()) return false;
		vector rearMatrix[4];
		if (!m_Rear.GetTransformationWorld(rearMatrix)) return false;
		vector position = rearMatrix[3] + rearMatrix[2] * 1.2;
		position[1] = m_World.GetSurfaceY(position[0], position[2]) + 0.05;
		m_Player.SetOrigin(position);
		Print("[ConvoyFollower] LOADED_TRIP_OWNER_STAGE: run_id=" + m_sRun + " character_id=" + Key(m_Player) +
			" origin=" + position + " test_only=true walking_claim=false truck_pose_writes=false");
		return true;
	}

	bool RequestTripUnload()
	{
		if (!m_bTripLoaded || m_bTripDelivered || m_bTripFailed || m_iStage != 3 || m_bUnloadRequested || !CF_HasCargoBinding()) return false;
		m_bUnloadRequested = true;
		m_fUnloadRequestedMs = m_World.GetWorldTime();
		m_fStageMs = m_fUnloadRequestedMs;
		m_vTruckOrigin = m_Truck.GetOrigin();
		Print("[ConvoyFollower] LOADED_TRIP_UNLOAD_REQUEST: run_id=" + m_sRun + " truck_id=" + Key(m_Truck) +
			" origin=" + m_vTruckOrigin + " source_distance=" + vector.DistanceXZ(m_Source.GetOrigin(), m_vTruckOrigin) +
			" destination_distance=" + vector.DistanceXZ(m_Destination.GetOrigin(), m_vTruckOrigin) + " completion_claim=false");
		return true;
	}

	override protected void Finish(bool pass, string reason)
	{
		if (m_bFinished) return;
		Cancel("loaded_trip_terminal_" + reason);
		m_bFinished = true;
		m_bTripFailed = !pass;
		LogTripCargo("terminal_" + reason);
		Print("[ConvoyFollower] LOADED_TRIP_CARGO_FAILURE: run_id=" + m_sRun + " reason=" + reason);
	}

	void LogTripCargo(string phase)
	{
		float source, cargo, destination;
		bool valid = TripCounts(source, cargo, destination);
		Print("[ConvoyFollower] LOADED_TRIP_CARGO: run_id=" + m_sRun + " phase=" + phase + " stage=" + m_iStage +
			" valid=" + valid + " source=" + source + " cargo=" + cargo + " destination=" + destination +
			" total=" + (source + cargo + destination) + " source_id=" + Key(m_Source) + " truck_id=" + Key(m_Truck) +
			" cargo_owner_id=" + Key(m_CargoOwner) + " destination_id=" + Key(m_Destination) +
			" loaded=" + m_bTripLoaded + " delivered=" + m_bTripDelivered + " failed=" + m_bTripFailed);
	}

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (m_bFinished || !Replication.IsServer() || !GetGame() || GetGame().GetWorld() != m_World || CF_ConvoySession.CF_IsWorldCleanup()) return;
		float now = m_World.GetWorldTime();
		if (now - m_fBeginMs > 720000 || (!m_bTripLoaded && now - m_fBeginMs > 90000))
		{ Finish(false, "cargo_total_or_load_timeout_" + m_sReason); return; }
		if (!m_bBound && !Bind()) return;
		if (!CF_HasCargoBinding()) { Finish(false, "original_cargo_binding_lost"); return; }
		if (m_iStage == 0 && !CF_PrepareCargoPlayMode()) { m_bStartupReadyTracking = false; return; }
		float source, cargo, destination;
		if (!TripCounts(source, cargo, destination)) { Finish(false, "original_container_or_counts_invalid"); return; }
		if (now - m_fTripLastSampleMs >= 1000) { m_fTripLastSampleMs = now; LogTripCargo("sample"); }
		if (m_iStage == 0)
		{
			if (!StaticTruck() || !Reach(m_Load) || !Queues(true)) { m_bStartupReadyTracking = false; return; }
			if (Math.AbsFloat(source - 1800) > TOLERANCE || cargo != 0 || destination != 0 || m_DestinationContainer.GetMaxResourceValue() < 100)
			{ m_bStartupReadyTracking = false; m_sReason = "initial_authored_counts_not_ready"; return; }
			if (!CF_CargoStartupReady(now)) return;
			m_fSourceStart = source; m_fCargoStart = cargo; m_fDestinationStart = destination;
			m_fSourceHeld = source; m_fCargoHeld = cargo; m_fDestinationHeld = destination;
			m_vTruckOrigin = m_Truck.GetOrigin(); m_fStageMs = now; m_iStage = 1;
			LogTripCargo("initial_conserved_counts");
			return;
		}
		if (Math.AbsFloat(source + cargo + destination - m_fSourceStart - m_fCargoStart - m_fDestinationStart) > TOLERANCE)
		{ Finish(false, "three_container_total_not_conserved"); return; }
		if (m_iStage == 1 || m_iStage == 3 || m_iStage == 5 || m_iStage == 6)
		{
			if (Math.AbsFloat(source - m_fSourceHeld) > TOLERANCE || Math.AbsFloat(cargo - m_fCargoHeld) > TOLERANCE ||
				Math.AbsFloat(destination - m_fDestinationHeld) > TOLERANCE || m_Load.IsInProgress() || m_Unload.IsInProgress())
			{ Finish(false, "cancelled_counts_or_action_not_stable"); return; }
			if (m_iStage == 6) return;
			if (m_iStage == 3 && !m_bUnloadRequested)
			{
				if (!m_bTripLoaded && !StaticTruck()) { Finish(false, "loaded_cancel_stationary_condition_lost"); return; }
				if (!m_bTripLoaded && now - m_fStageMs >= 3000)
				{ m_bTripLoaded = true; LogTripCargo("loaded_cancel_stable_departure_allowed"); }
				return;
			}
			if (!StaticTruck()) { Finish(false, "transfer_stationary_condition_lost"); return; }
			if (now - m_fStageMs < 3000) return;
			if (m_iStage == 1) Start(true, now);
			else if (m_iStage == 3) Start(false, now);
			else { m_bTripDelivered = true; m_iStage = 6; LogTripCargo("delivered_cancel_stable_resume_allowed"); }
			return;
		}
		if (!StaticTruck()) { Finish(false, "active_transfer_stationary_condition_lost"); return; }
		bool loading = m_iStage == 2;
		float transferred = cargo - m_fCargoHeld;
		if (!loading) transferred = m_fCargoHeld - cargo;
		if (transferred > TOLERANCE)
		{
			Cancel("first_positive_transfer");
			if (!Queues(loading, true)) { Finish(false, m_sReason); return; }
			if (Math.AbsFloat(transferred - 100) > TOLERANCE) { Finish(false, "transfer_not_exactly_100"); return; }
			if (loading && (Math.AbsFloat(source - m_fSourceStart + transferred) > TOLERANCE || Math.AbsFloat(destination - m_fDestinationStart) > TOLERANCE))
			{ Finish(false, "load_wrong_source_or_destination_changed"); return; }
			if (!loading && (Math.AbsFloat(source - m_fSourceHeld) > TOLERANCE || Math.AbsFloat(destination - m_fDestinationStart - transferred) > TOLERANCE || Math.AbsFloat(cargo - m_fCargoStart) > TOLERANCE))
			{ Finish(false, "unload_not_delivered_to_distinct_destination"); return; }
			if (loading) m_fLoaded = transferred;
			m_fSourceHeld = source; m_fCargoHeld = cargo; m_fDestinationHeld = destination;
			m_fStageMs = now; m_iStage++;
			LogTripCargo("native_transfer_cancelled");
			return;
		}
		if (transferred < -TOLERANCE || now - m_fStageMs > 15000 || !(timeSlice > 0 && timeSlice <= 0.25))
		{ Finish(false, "wrong_direction_timeout_or_invalid_frame_slice"); return; }
		if (!Reach(m_Active) || !Queues(loading)) { Finish(false, m_sReason); return; }
		m_fProgress = m_Active.GetActionProgress(m_fProgress, timeSlice);
		m_Player.DoPerformContinuousObjectAction(m_Active, timeSlice);
	}
}

class CF_LoadedSupplyTripProbeComponentClass : CF_OrdinaryDirectRoadProbeComponentClass
{
}

// This coordinator adds load/unload barriers to the passing ordinary course.
// Driving, arrival, Hold and Resume gates remain in the original observer.
class CF_LoadedSupplyTripProbeComponent : CF_OrdinaryDirectRoadProbeComponent
{
	protected CF_LoadedTripCargoProbeComponent m_TripCargo;
	protected int m_iCargoOwnerStep;
	protected bool m_bTripResultLogged;
	protected bool m_bTripDepartureLogged;
	protected float m_fCargoOwnerStartMs;

	protected bool BindTripCargo()
	{
		Vehicle truck = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokeFollower1"));
		if (!truck) return false;
		CF_LoadedTripCargoProbeComponent cargo = CF_LoadedTripCargoProbeComponent.Cast(truck.FindComponent(CF_LoadedTripCargoProbeComponent));
		if (m_TripCargo && cargo != m_TripCargo) return false;
		m_TripCargo = cargo;
		return m_TripCargo != null;
	}

	override protected void Poll()
	{
		if (!PacedWorldAlive() || m_bPacedTerminal) return;
		if (!BindTripCargo()) { NoteFailure("cargo", "original_cargo_component_missing"); EndPaced(); return; }
		if (m_TripCargo.TripFailed()) { NoteFailure("cargo", "native_transfer_or_conservation_failed"); EndPaced(); return; }
		if (!m_TripCargo.TripLoaded())
		{
			if (PacedSeconds() > 95) { NoteFailure("cargo", "departure_load_timeout"); EndPaced(); }
			return;
		}
		if (!m_bTripDepartureLogged)
		{
			m_bTripDepartureLogged = true;
			m_TripCargo.LogTripCargo("original_course_released");
		}
		if (m_Player && m_Player != m_TripCargo.TripOwner())
		{ NoteFailure("cargo", "trip_owner_identity_changed"); EndPaced(); return; }
		float now = GetGame().GetWorld().GetWorldTime();
		// Let the parent measure its full 30 seconds, then keep real Hold active
		// throughout rear staging, native unload and the return to the lead seat.
		if (m_iCommandStage == 3 && now - m_fCommandStageMs >= 30000 && m_iCargoOwnerStep != 5)
		{
			m_iPacedTick++;
			if (!CommandIdentities() || !CommandDriver() || !CommandDriver().CF_IsPanelHeld() || !CommandDriver().CF_HasPanelHoldRequest())
			{ CommandFailure("cargo_unload_original_hold_or_identity_lost"); return; }
			ObserveOrdinaryHead();
			MeasurePeaks();
			if (m_bPacedFailed) { EndPaced(); return; }
			Print("[ConvoyFollower] ORIGINAL_RESUME_HOLD_SAMPLE: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick + " seconds=" + PacedSeconds() +
				" held_s=" + (now - m_fCommandStageMs) / 1000 + " origin=" + m_PacedTrucks[1].Truck.GetOrigin() +
				" max_drift_m=" + m_fCommandHoldDrift + " max_speed_kmh=" + m_fCommandHoldSpeed +
				" identities=true cargo_owner_step=" + m_iCargoOwnerStep + " native_unload_barrier=true");
			if (m_iCargoOwnerStep == 0)
			{
				m_fCargoOwnerStartMs = now;
				if (!CommandSeat(m_PacedOwner, true) || !ExitCommandSeat(m_PacedOwner))
				{ CommandFailure("cargo_owner_exit_rejected"); return; }
				m_iCargoOwnerStep = 1;
				Print("[ConvoyFollower] LOADED_TRIP_HOLD_UNLOAD_BEGIN: run_id=" + m_sPacedRun +
					" already_held_s=" + (now - m_fCommandStageMs) / 1000 + " hold_remains_active=true native_unload_pending=true");
			}
			else if (m_iCargoOwnerStep == 1 && CommandOnFoot(m_PacedOwner))
			{
				if (!m_TripCargo.StageOwnerAtRear() || !m_TripCargo.RequestTripUnload())
				{ CommandFailure("cargo_rear_setup_or_unload_request_failed"); return; }
				m_iCargoOwnerStep = 2;
			}
			else if (m_iCargoOwnerStep == 2 && m_TripCargo.TripDelivered())
			{
				if (!BoardCommandSeat(m_PacedOwner, true)) { CommandFailure("cargo_owner_return_rejected"); return; }
				m_iCargoOwnerStep = 3;
			}
			else if (m_iCargoOwnerStep == 3 && CommandSeat(m_PacedOwner, true))
			{
				m_iCargoOwnerStep = 5;
				m_TripCargo.LogTripCargo("owner_returned_resume_gate_released");
			}
			if (now - m_fCargoOwnerStartMs > 45000) { CommandFailure("cargo_unload_and_owner_return_timeout"); return; }
			return;
		}
		super.Poll();
	}

	override protected void EndPaced()
	{
		if (!m_bPacedTerminal && m_bCommandComplete &&
			(!m_TripCargo || !m_TripCargo.TripLoaded() || !m_TripCargo.TripDelivered() || m_TripCargo.TripFailed()))
			NoteFailure("cargo", "complete_trip_cargo_gate_missing");
		super.EndPaced();
		// The parent can fail its final owned lead park. Emit this combined
		// verdict only after that terminal work, retaining any late failure.
		if (m_bPacedTerminal && !m_bTripResultLogged)
		{
			m_bTripResultLogged = true;
			bool cargoPassed = m_TripCargo && m_TripCargo.TripLoaded() && m_TripCargo.TripDelivered() && !m_TripCargo.TripFailed();
			if (m_TripCargo) m_TripCargo.LogTripCargo("final_driving_gate");
			Print("[ConvoyFollower] LOADED_TRIP_RESULT: run_id=" + m_sPacedRun + " pass=" + (!m_bPacedFailed && m_bCommandComplete && cargoPassed) +
				" native_load=100 distinct_destination_unload=100 cargo_gate=" + cargoPassed +
				" ordinary_original_driver=true baseline_stop_s=180 explicit_hold_min_s=30 resume_progress_min_m=20 resume_powered_min=3" +
				" peak_gap_m=" + m_fPacedPeakGap + " ordinary_input=false owner_staging=test_only");
		}
	}
}
