// Private observation of the actual existing ready branch; never clears/rearms it.
modded class CF_DriverControllerComponent
{
 protected IEntity m_PrivateReadyTruck;
 protected IEntity m_PrivateReadyDriver;
 protected IEntity m_PrivateReadyTarget;
 override protected bool CF_WaitForInitialDeparture(IEntity target)
 {
  bool pendingBefore = m_bInitialDeparturePending;
  bool waiting = super.CF_WaitForInitialDeparture(target);
  if (pendingBefore && !waiting && !m_bInitialDeparturePending)
  {
   m_PrivateReadyTruck = m_Truck; m_PrivateReadyDriver = m_Driver; m_PrivateReadyTarget = target;
   Print("[ConvoyFollower] ORDINARY_DEPARTURE_EVENT: actual_ready_branch=true original_core_ready_log_emitted=true guard_written=false");
  }
  return waiting;
 }
 bool CF_PrivateRealDepartureObserved(IEntity target)
 {
  return m_PrivateReadyTruck == m_Truck && m_PrivateReadyDriver == m_Driver &&
   m_PrivateReadyTarget == target && !m_bInitialDeparturePending;
 }
}

class CF_JourneyActivityEvidence
{
 CF_OriginalFollowActivity Action; AIWaypoint Waypoint;
 vector Previous; vector PreviousGoal; vector RouteGoal;
 float ActivityStartRouteGap; float NetRouteApproach;
 float Progress; int Power; bool Qualified; int QualifiedSequence;
}
class CF_OrdinaryOutboundBayCourseClass : CF_ExplicitBayFeatureProbeComponentClass {}

// Ordinary outbound -> real arrival -> Hold -> fresh Resume -> existing bay course.
// Lead uses existing native fixture orders. Follower receives session orders only.
class CF_OrdinaryOutboundBayCourse : CF_ExplicitBayFeatureProbeComponent
{
 protected int m_iJourneyPhase;
 protected float m_fJourneyPhaseMs;
 protected float m_fJourneyStableMs = -1;
 protected bool m_bJourneyPrefixComplete;
 protected bool m_bJourneyArrival;
 protected bool m_bJourneyHold;
 protected bool m_bJourneyResume;
 protected bool m_bJourneyReported;
 protected float m_fJourneyHoldDrift;
 protected vector m_vJourneyHeld;
 protected ref CF_JourneyActivityEvidence m_JourneyOutbound;
 protected ref CF_JourneyActivityEvidence m_JourneyResumeEvidence;
 protected int m_iJourneyBaselineSequence;
 protected CameraBase m_JourneyCamera;
 protected bool m_bJourneyCameraLogged;
 protected float m_fJourneyCameraLogMs;

 override void OnPostInit(IEntity owner)
 {
  super.OnPostInit(owner);
  Print("[ConvoyFollower] ORDINARY_BAY_COURSE_INIT: total_world_s=1650 setup_s=95 outbound_s=240 arrival_capture_s=90 arrival_stable_s=180 hold_s=30 hold_drift_m=2 resume_return_leg_s=240 bay_park_return_safety_unchanged=true spacing_60m_sticky=true diagnostic_continuation_after_spacing_only=true real_departure_required=true same_activity_outbound_m=100 powered_min=3 fresh_resume_m=20 camera_only_observer=true normal_input=false video=false");
 }
 override protected bool CF_SelectAIDriveGoal(RoadNetworkManager roads)
 {
  vector hint = Vector(7173.8, 139.817, 2917.3), reached;
  bool connected = roads && roads.GetReachableWaypointInRoad(m_vInitialLeadPosition, hint, 5, reached);
  Print("[ConvoyFollower] ORDINARY_BAY_ROUTE: original_start=" + m_vInitialLeadPosition + " hint=" + hint + " connected=" + connected + " resolved=" + reached + " fixed_previously_driven_route=true");
  if (!connected || vector.DistanceXZ(reached, hint) > 3 || vector.DistanceXZ(reached, m_vInitialLeadPosition) < 350) return false;
  m_vRoadGoal = reached; return true;
 }
 protected void JourneyPhase(int phase)
 {
  m_iJourneyPhase = phase; m_fJourneyPhaseMs = GetGame().GetWorld().GetWorldTime(); m_fJourneyStableMs = -1;
  Print("[ConvoyFollower] ORDINARY_BAY_PHASE: phase=" + phase + " world_ms=" + m_fJourneyPhaseMs + " spacing_failure_retained=" + m_bPacedSpacingFailed);
 }
 protected void JourneyCapture(string tag)
 {
  bool writable = System.MakeScreenshot("$logs:journey-" + tag);
  CameraManager manager = GetGame().GetCameraManager();
  bool selected = manager && m_JourneyCamera && manager.CurrentCamera() == m_JourneyCamera;
  Print("[ConvoyFollower] ORDINARY_BAY_CAPTURE: tag=" + tag + " writable=" + writable + " camera_selected=" + selected + " camera_only=true rendered_pixels_pending=true input_claim=false video_claim=false");
 }
 protected void JourneyResult(string reason)
 {
  if (m_bJourneyReported) return; m_bJourneyReported = true;
  bool outbound; if (m_JourneyOutbound) outbound = m_JourneyOutbound.Qualified;
  string line = "[ConvoyFollower] ORDINARY_BAY_PREFIX_RESULT: reason=" + reason + " complete=" + m_bJourneyPrefixComplete;
  line += " real_departure=" + (CommandDriver() && CommandDriver().CF_PrivateRealDepartureObserved(m_Lead));
  line += " ordinary_same_activity_100m3=" + outbound + " arrival_180s2m=" + m_bJourneyArrival;
  line += " hold_30s2m=" + m_bJourneyHold + " fresh_resume_20m3=" + m_bJourneyResume;
  line += " peak_gap_m=" + m_fPacedPeakGap + " whole_run_failed=" + m_bPacedFailed + " original_identities=" + BayIdentities();
  line += " guard_reset=false follower_controls_written=false native_lead_passenger_owner=true production_player_home_claim=false";
  Print(line);
 }
 override protected void BayEnd(bool pass, string reason)
 {
  JourneyResult(reason); super.BayEnd(pass, reason);
 }
 protected void JourneyFail(string reason)
 {
  NoteFailure("ordinary_course", reason); BayEnd(false, "ordinary_prefix_" + reason);
 }
 protected void JourneyMotion(CF_JourneyActivityEvidence evidence, bool resume)
 {
  Vehicle truck = m_PacedTrucks[1].Truck;
  vector origin = truck.GetOrigin();
  vector delta = origin - evidence.Previous;
  vector axis = evidence.PreviousGoal - evidence.Previous; axis[1] = 0;
  float signed; if (axis.Length() > 0.01) { axis.Normalize(); signed = vector.Dot(delta, axis); }
  CF_OriginalFollowActivity activity = ExactOriginalActivity();
  int sequence; if (activity) sequence = activity.CF_GetSequence();
  if (!resume && activity) m_iJourneyBaselineSequence = Math.Max(m_iJourneyBaselineSequence, sequence);
  bool fresh = !resume || sequence > m_iJourneyBaselineSequence;
  bool exact = activity && fresh && IdentityRetained(1, true);
  AIWaypoint waypoint; if (activity) waypoint = activity.m_RelatedWaypoint;
  bool continuous = exact && evidence.Action == activity && evidence.Waypoint == waypoint;
  if (vector.DistanceXZ(origin, evidence.Previous) > 20) { JourneyFail("motion_position_discontinuity_over20m"); return; }
  if (!continuous)
  {
   evidence.Progress = 0; evidence.Power = 0;
   evidence.ActivityStartRouteGap = vector.DistanceXZ(origin, evidence.RouteGoal);
  }
  evidence.NetRouteApproach = evidence.ActivityStartRouteGap - vector.DistanceXZ(origin, evidence.RouteGoal);
  CarControllerComponent car = CarControllerComponent.Cast(truck.FindComponent(CarControllerComponent));
  VehicleWheeledSimulation sim; if (car) sim = car.GetSimulation();
  bool powered = exact && continuous && sim && sim.EngineIsOn() && sim.GetThrottle() > 0.05 && sim.GetGear() >= 2 && sim.GetSpeedKmh() > 1.5 && signed >= 0.25;
  if (continuous) { evidence.Progress += signed; if (powered) evidence.Power++; }
  float required = 100; if (resume) required = 20;
  if (exact && continuous && evidence.Progress >= required && evidence.NetRouteApproach >= required && evidence.Power >= 3 && !evidence.Qualified)
  {
   evidence.Qualified = true; evidence.QualifiedSequence = sequence;
   if (resume) { m_bJourneyResume = true; JourneyCapture("resume-20m3"); }
   else JourneyCapture("outbound-100m3");
  }
  string line = "[ConvoyFollower] ORDINARY_BAY_MOTION: phase=" + m_iJourneyPhase + " resume=" + resume + " exact_original_activity=" + exact;
  line += " continuous=" + continuous + " sequence=" + sequence + " waypoint_id=" + EntityKey(waypoint);
  line += " origin=" + origin + " previous_actual_native_target_goal=" + evidence.PreviousGoal + " signed_previous_goal_step_m=" + signed;
  line += " single_activity_progress_m=" + evidence.Progress + " single_activity_powered=" + evidence.Power;
  line += " fixed_route_goal=" + evidence.RouteGoal + " independent_activity_net_route_approach_m=" + evidence.NetRouteApproach;
  line += " powered=" + powered + " qualified=" + evidence.Qualified + " original_identity=" + IdentityRetained(1, true);
  Print(line);
  evidence.Previous = origin; evidence.PreviousGoal = vector.Zero;
  if (activity && activity.Lease && activity.Lease.NativeTarget) evidence.PreviousGoal = activity.Lease.NativeTarget.GetOrigin();
  evidence.Action = null; evidence.Waypoint = null;
  if (exact) { evidence.Action = activity; evidence.Waypoint = waypoint; }
 }
 protected CF_JourneyActivityEvidence JourneyNewEvidence(bool resume = false)
 {
  CF_JourneyActivityEvidence evidence = new CF_JourneyActivityEvidence();
  evidence.Previous = m_PacedTrucks[1].Truck.GetOrigin(); evidence.PreviousGoal = m_Lead.GetOrigin();
  evidence.RouteGoal = m_vRoadGoal; if (resume) evidence.RouteGoal = m_vRestartGoal;
  evidence.ActivityStartRouteGap = vector.DistanceXZ(evidence.Previous, evidence.RouteGoal);
  return evidence;
 }
 protected bool JourneyArrivalReady()
 {
  return LeadWaitSelected() && CommandSpeed(m_Lead) <= 2 && CommandSpeed(m_PacedTrucks[1].Truck) <= 2 &&
   CommandDriver().CF_HasSelectedArrivalWait(m_PacedSession, m_PacedTrucks[1].Truck) && SelectedCapturedWait();
 }
 protected void JourneyCamera(float timeSlice)
 {
  if (!GetGame() || !PacedWorldAlive() || m_PacedTrucks.Count() != 2 || SCR_EditorManagerEntity.IsOpenedInstance()) return;
  if (!EntityAlive(m_PacedTrucks[1].Truck) || !EntityAlive(m_Lead)) return;
  CameraManager manager = GetGame().GetCameraManager(); if (!manager || !GetGame().GetPlayerController()) return;
  vector tail = m_PacedTrucks[1].Truck.GetOrigin(), target = (tail + m_Lead.GetOrigin()) * 0.5;
  if (m_iBayPhase == 12 || m_iBayPhase == 13)
  {
   CF_EntityFollowDriverControllerComponent driver = CommandDriver();
   if (driver) target = (tail + driver.CF_PrivateCameraSlotRead()) * 0.5;
  }
  float span = vector.DistanceXZ(tail, m_Lead.GetOrigin());
  if (m_iBayPhase == 12 || m_iBayPhase == 13) span = vector.DistanceXZ(tail, CommandDriver().CF_PrivateCameraSlotRead());
  float height = Math.Clamp(span * 0.65 + 25, 35, 120);
  bool tailOnly = span > 100;
  if (tailOnly) { target = tail; height = 45; }
  vector eye = target + Vector(18, height, -28);
  vector transform[4];
  Math3D.DirectionAndUpMatrix((target - eye).Normalized(), vector.Up, transform); transform[3] = eye;
  if (!m_JourneyCamera)
  {
   m_JourneyCamera = CameraBase.Cast(GetGame().SpawnEntity(CameraBase, GetGame().GetWorld()));
   if (!m_JourneyCamera) return; m_JourneyCamera.SetFOVDegree(62);
   m_JourneyCamera.SetWorldTransform(transform);
  }
  if (manager.CurrentCamera() != m_JourneyCamera && !manager.SetCamera(m_JourneyCamera)) return;
  m_JourneyCamera.SetWorldTransform(transform); m_JourneyCamera.ApplyTransform(timeSlice);
  float cameraNow = GetGame().GetWorld().GetWorldTime();
  if (!m_bJourneyCameraLogged || cameraNow - m_fJourneyCameraLogMs >= 5000)
  {
   m_bJourneyCameraLogged = true; m_fJourneyCameraLogMs = cameraNow;
   string line = "[ConvoyFollower] ORDINARY_BAY_CAMERA: selected=" + (manager.CurrentCamera() == m_JourneyCamera);
   line += " eye=" + eye + " target=" + target + " tail_only=" + tailOnly + " observed_span_m=" + span;
   line += " lead_framing_claim=" + !tailOnly;
   line += " camera_entity_only=true vehicle_actor_transform_writes=false visibility_pending=true";
   Print(line);
  }
 }
 override void EOnPostFrame(IEntity owner, float timeSlice)
 {
  JourneyCamera(timeSlice); super.EOnPostFrame(owner, timeSlice);
  if (!PacedWorldAlive() || m_bPacedTerminal || m_bJourneyPrefixComplete) return;
  if (m_iJourneyPhase == 6 || m_iJourneyPhase == 10)
   m_fJourneyHoldDrift = Math.Max(m_fJourneyHoldDrift, vector.DistanceXZ(m_PacedTrucks[1].Truck.GetOrigin(), m_vJourneyHeld));
  if (m_bCommandParking)
  {
   if (!CanCommandPark() || CommandSpeed(m_Lead) > 2 || vector.DistanceXZ(m_Lead.GetOrigin(), m_vTransferOrigin) > 2)
   { JourneyFail("original_lead_seat_handoff_park_lost"); return; }
   ApplyCommandParking();
  }
 }
 override protected void Poll()
 {
  if (!PacedWorldAlive() || m_bPacedTerminal) return;
  float now = GetGame().GetWorld().GetWorldTime();
  if (now - m_fPacedStartMs > 1650000) { JourneyFail("1650world_total_timeout"); return; }
  if ((m_iJourneyPhase == 0 || m_iJourneyPhase == 11 || m_iJourneyPhase == 12) && PacedSeconds() > 95)
  { JourneyFail("95world_complete_original_setup_timeout"); return; }
  if (m_bJourneyPrefixComplete) { super.Poll(); return; }
  if (!BindTripCargo() || m_TripCargo.TripFailed()) { JourneyFail("native_cargo_binding_or_conservation"); return; }
  if (m_iJourneyPhase == 0)
  {
   if (PacedSeconds() > 95) { JourneyFail("95world_loaded_recruitment_setup_timeout"); return; }
   if (m_TripCargo.TripLoaded() && m_iStage == 2 && m_iNextOrder == 1)
   {
    BindOriginals();
    if (m_PacedTrucks.Count() != 2 || !super.StartBarrier()) return;
    if (!BayIdentities()) { JourneyFail("original_setup_binding"); return; }
    m_vInitialLeadPosition = m_Lead.GetOrigin();
    // Let the unchanged session bind original head staging from its real owner
    // pilot seat while still at recruitment. No staging/guard fields are written.
    HoldLeadAtRoadGoal(); BeginCommandSeatTransfer(true); JourneyPhase(11); return;
   }
   super.Poll(); return;
  }
  if (m_iJourneyPhase == 11)
  {
   if (!PollCommandSeatTransfer(true) || !ReturnCaptureStaging())
   { if (now - m_fJourneyPhaseMs > 15000) JourneyFail("original_staging_native_owner_seat_timeout"); return; }
   BeginCommandSeatTransfer(false); JourneyPhase(12); return;
  }
  if (m_iJourneyPhase == 12)
  {
   if (!PollCommandSeatTransfer(false))
   { if (now - m_fJourneyPhaseMs > 15000) JourneyFail("original_native_pilot_reentry_timeout"); return; }
   vector separation = m_Lead.GetOrigin() - m_PacedTrucks[1].Truck.GetOrigin();
   vector heading = m_Lead.GetWorldTransformAxis(2);
   if (vector.Dot(separation, heading) < 5 || !super.StartBarrier() || !StartAIDrive())
   { JourneyFail("original_native_lead_start"); return; }
   ReleaseCommandParking("ordinary_first_native_drive_order_started");
   m_iStage = 3; m_JourneyOutbound = JourneyNewEvidence(); JourneyPhase(1); JourneyCapture("outbound-start"); return;
  }
  m_iPacedTick++; MeasurePeaks(); ObserveOrdinaryHead(); ObserveFollowerHealth();
  if (!BayIdentities() || CommandDriver().CF_HasEntityFallbackFailure() || m_bEntityGateFailed)
  { JourneyFail("original_identity_or_controller_failure"); return; }
  if (m_iJourneyPhase <= 3 || m_iJourneyPhase == 8) LogPacedSamples();
  if (m_iJourneyPhase <= 6)
  {
   m_iJourneyBaselineSequence = Math.Max(m_iJourneyBaselineSequence, m_iBaselineSequence);
   CF_OriginalFollowActivity beforeResume = ExactOriginalActivity();
   if (beforeResume) m_iJourneyBaselineSequence = Math.Max(m_iJourneyBaselineSequence, beforeResume.CF_GetSequence());
  }
  float elapsed = now - m_fJourneyPhaseMs;
  Print("[ConvoyFollower] ORDINARY_BAY_SAMPLE: phase=" + m_iJourneyPhase + " elapsed_world_s=" + elapsed / 1000 + " real_departure=" + CommandDriver().CF_PrivateRealDepartureObserved(m_Lead) + " native_cargo_loaded=" + m_TripCargo.TripLoaded() + " peak_gap_m=" + m_fPacedPeakGap + " spacing_failure_sticky=" + m_bPacedSpacingFailed);
  if (m_iJourneyPhase == 1)
  {
   JourneyMotion(m_JourneyOutbound, false);
   if (vector.DistanceXZ(m_Lead.GetOrigin(), m_vRoadGoal) <= 8 && !m_bPacedHoldingLead) HoldLeadAtRoadGoal();
   if (LeadWaitSelected() && CommandSpeed(m_Lead) <= 2)
   {
    if (!CommandDriver().CF_PrivateRealDepartureObserved(m_Lead) || !m_JourneyOutbound.Qualified || m_PacedTrucks[0].ForwardProgress < 150 || m_PacedTrucks[0].PoweredSamples < 3)
    { JourneyFail("real_departure_or_exact100m3_or_lead150m3_missing"); return; }
    JourneyPhase(2); return;
   }
   if (elapsed > 240000) JourneyFail("240world_outbound_timeout"); return;
  }
  if (m_iJourneyPhase == 2)
  {
   if (JourneyArrivalReady()) { BeginObservation(); JourneyPhase(3); return; }
   if (elapsed > 90000) JourneyFail("90world_owned_arrival_capture_timeout"); return;
  }
  if (m_iJourneyPhase == 3)
  {
   if (!JourneyArrivalReady() || m_bPacedDriftFailed) { JourneyFail("owned180s_arrival_wait_or_2m_drift_lost"); return; }
   m_iObservationSamples++;
   if (now - m_fObservationStartMs < 180000) return;
   if (m_iBaselineWaitSamples < 180) { JourneyFail("180_arrival_wait_samples_missing"); return; }
   m_bJourneyArrival = true; m_bPacedObserving = false; JourneyCapture("arrival-180s");
   BeginCommandSeatTransfer(true); JourneyPhase(4); return;
  }
  if (m_iJourneyPhase == 4 || m_iJourneyPhase == 9)
  {
   if (!PollCommandSeatTransfer(true)) { if (elapsed > 15000) JourneyFail("owner_pilot_seat_timeout"); return; }
   if (!CF_ConvoySession.CF_PanelHold(m_PacedOwner)) { JourneyFail("real_hold_rejected"); return; }
   m_vJourneyHeld = m_PacedTrucks[1].Truck.GetOrigin(); m_fJourneyHoldDrift = 0;
   int next = 5; if (m_iJourneyPhase == 9) next = 10;
   JourneyPhase(next); return;
  }
  if (m_iJourneyPhase == 5)
  {
   if (!CommandDriver().CF_IsPanelHeld() || CommandSpeed(m_PacedTrucks[1].Truck) > 2)
   { if (elapsed > 15000) JourneyFail("hold_capture_timeout"); return; }
   m_vJourneyHeld = m_PacedTrucks[1].Truck.GetOrigin(); m_fJourneyHoldDrift = 0; JourneyPhase(6); return;
  }
  if (m_iJourneyPhase == 6 || m_iJourneyPhase == 10)
  {
   string holdLine = "[ConvoyFollower] ORDINARY_BAY_HOLD: phase=" + m_iJourneyPhase;
   holdLine += " elapsed_world_s=" + elapsed / 1000 + " stable_start_world_ms=" + m_fJourneyStableMs;
   holdLine += " truck=" + m_PacedTrucks[1].Truck.GetOrigin() + " original_truck_id=" + EntityKey(m_PacedTrucks[1].Truck);
   holdLine += " frame_max_drift_m=" + m_fJourneyHoldDrift + " speed_kmh=" + CommandSpeed(m_PacedTrucks[1].Truck);
   holdLine += " panel_held=" + CommandDriver().CF_IsPanelHeld() + " hold_requested=" + CommandDriver().CF_HasPanelHoldRequest();
   Print(holdLine);
   if (!CommandDriver().CF_IsPanelHeld() || !CommandDriver().CF_HasPanelHoldRequest() || CommandSpeed(m_PacedTrucks[1].Truck) > 2)
   {
    m_fJourneyStableMs = -1;
    if (elapsed > 15000) JourneyFail("hold_ownership_or_stop_timeout"); return;
   }
   if (m_fJourneyStableMs < 0) { m_fJourneyStableMs = now; m_vJourneyHeld = m_PacedTrucks[1].Truck.GetOrigin(); m_fJourneyHoldDrift = 0; }
   if (m_fJourneyHoldDrift > 2) { JourneyFail("hold_frame_drift_over2m"); return; }
   if (now - m_fJourneyStableMs < 30000) return;
   if (m_iJourneyPhase == 10)
   {
    if (!m_bJourneyArrival || !m_bJourneyHold || !m_bJourneyResume || !m_JourneyOutbound.Qualified || !CommandDriver().CF_PrivateRealDepartureObserved(m_Lead))
    { JourneyFail("prefix_prerequisites_missing_before_bay"); return; }
    m_bJourneyPrefixComplete = true; m_vHeld = m_PacedTrucks[1].Truck.GetOrigin(); m_fHoldDrift = 0;
    JourneyCapture("bay-prefix-complete"); BayPhase(7); return;
   }
   m_bJourneyHold = true; JourneyCapture("hold-30s");
   Print("[ConvoyFollower] ORDINARY_BAY_RESUME_BASELINE: greatest_observed_pre_resume_sequence=" + m_iJourneyBaselineSequence + " frozen_for_resume=true post_resume_samples_excluded=true");
   if (!CF_ConvoySession.CF_PanelResume(m_PacedOwner)) { JourneyFail("real_resume_rejected"); return; }
   BeginCommandSeatTransfer(false); JourneyPhase(7); return;
  }
  if (m_iJourneyPhase == 7)
  {
   if (!PollCommandSeatTransfer(false)) { if (elapsed > 15000) JourneyFail("native_resume_seat_timeout"); return; }
   ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld()); vector reached;
   vector hint = Vector(7180.53, 140.153, 2468.41);
   if (!aiWorld || !aiWorld.GetRoadNetworkManager() || !aiWorld.GetRoadNetworkManager().GetReachableWaypointInRoad(m_Lead.GetOrigin(), hint, 5, reached) || vector.DistanceXZ(hint, reached) > 3)
   { JourneyFail("existing_corridor_bay_return_goal_unreachable"); return; }
   m_vRestartGoal = reached; m_vRestartAxis = reached - m_Lead.GetOrigin(); m_vRestartAxis[1] = 0; m_vRestartAxis.Normalize();
   if (!StartCommandRestart()) { JourneyFail("native_resume_lead_order_rejected"); return; }
   m_JourneyResumeEvidence = JourneyNewEvidence(true); JourneyPhase(8); return;
  }
  if (m_iJourneyPhase == 8)
  {
   JourneyMotion(m_JourneyResumeEvidence, true);
   if (vector.DistanceXZ(m_Lead.GetOrigin(), m_vRestartGoal) <= 8 && !m_bPacedHoldingLead) HoldLeadAtRoadGoal();
   if (LeadWaitSelected() && CommandSpeed(m_Lead) <= 2)
   {
    if (!m_bJourneyResume) { JourneyFail("fresh_continuous_resume20m3_missing"); return; }
    BeginCommandSeatTransfer(true); JourneyPhase(9); return;
   }
   if (elapsed > 240000) JourneyFail("240world_resume_corridor_timeout"); return;
  }
 }
}
modded class CF_DriverControllerComponent
{
 vector CF_PrivateCameraSlotRead() { return m_vUnloadWaitingPoint; }
}
