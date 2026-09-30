class CF_PrivateReturnMotion
{
 vector Previous; vector Axis; AIActionBase Action; AIWaypoint Waypoint;
 float Progress; int Power; float BestProgress; int BestPower; bool Qualified; bool OrdinaryQualified;
}
class CF_ExplicitBayFeatureProbeComponentClass : CF_LoadedSupplyTripProbeComponentClass {}
// Private physical feature course. Inherited setup recruits the original unit.
// Production session commands own follower driving/Hold/admission/parking.
// Fixture lead seat/rear transitions are setup, never normal input evidence.
class CF_ExplicitBayFeatureProbeComponent : CF_LoadedSupplyTripProbeComponent
{
 // Private observer: automatic native lead proves only disclosed physical return.
 protected vector m_vReturnHeadHome;
 protected vector m_vReturnTailHome;
 protected vector m_vReturnTailStart;
 protected vector m_vReturnHeadStable;
 protected vector m_vReturnTailStable;
 protected float m_fReturnBeginMs;
 protected float m_fReturnStartGap;
 protected float m_fReturnNetApproach;
 protected float m_fReturnStableDrift;
 protected bool m_bReturnCommand;
 protected bool m_bReturnMergedObserved;
 protected bool m_bReturnReported;
 protected ref CF_PrivateReturnMotion m_ReturnHead;
 protected ref CF_PrivateReturnMotion m_ReturnTail;

 // The inherited helper asserts active-roster identity; a parked return unit
 // has intentionally left that roster. This private setup fallback checks its
 // original return-queue binding instead, without changing production gates.
 override protected bool CanCommandPark()
 {
  if (super.CanCommandPark()) return true;
  if (!m_bBayParkComplete || m_bReturnMergedObserved || !BayIdentities() ||
   !m_PacedSession.CF_TestOriginalReturnParked(CommandDriver())) return false;
  PlayerManager players = GetGame().GetPlayerManager();
  if (!players || players.GetPlayerIdFromControlledEntity(m_Pilot) > 0 || players.GetPlayerIdFromControlledEntity(m_Lead) > 0) return false;
  CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
  if (!car || !car.GetSimulation() || !car.GetPilotCompartmentSlot()) return false;
  IEntity pilot = car.GetPilotCompartmentSlot().GetOccupant();
  return !pilot || pilot == m_Pilot || pilot == m_PacedOwner;
 }
 protected bool ReturnCaptureStaging()
 {
  if (!m_PacedSession.CF_TestOriginalStaging(CommandDriver(), m_Lead, m_vReturnHeadHome, m_vReturnTailHome)) return false;
  Print("[ConvoyFollower] RETURN_FEATURE_STAGING: lead=" + m_vReturnHeadHome + " tail=" + m_vReturnTailHome + " from_original_recruitment=true pose_written=false");
  return true;
 }
 protected bool ReturnChain()
 {
  CF_EntityFollowDriverControllerComponent driver = CommandDriver();
  return BayIdentities() && driver && m_PacedSession.GetUnitNumber(driver) == 1 &&
   driver.CF_TestReturnPredecessor() == m_Lead && m_PacedSession.CF_TestOriginalReturnMerged();
 }
 protected void ReturnResult(bool pass, string reason)
 {
  if (m_bReturnReported) return;
  m_bReturnReported = true;
  float headProgress, tailProgress; int headPower, tailPower; bool headGate, tailGate, ordinaryGate;
  if (m_ReturnHead) { headProgress = m_ReturnHead.BestProgress; headPower = m_ReturnHead.BestPower; headGate = m_ReturnHead.Qualified; }
  if (m_ReturnTail) { tailProgress = m_ReturnTail.BestProgress; tailPower = m_ReturnTail.BestPower; tailGate = m_ReturnTail.Qualified; ordinaryGate = m_ReturnTail.OrdinaryQualified; }
  Print("[ConvoyFollower] RETURN_FEATURE_RESULT: feature_pass=" + pass + " reason=" + reason +
   " command_accepted=" + m_bReturnCommand + " original_merge=" + m_bReturnMergedObserved +
   " original_chain=" + ReturnChain() + " head_home=" + m_vReturnHeadHome + " tail_home=" + m_vReturnTailHome +
   " head=" + m_Lead.GetOrigin() + " tail=" + m_PacedTrucks[1].Truck.GetOrigin() +
   " head_single_order_m=" + headProgress + " head_single_order_powered=" + headPower + " head_power_gate=" + headGate +
   " tail_single_order_m=" + tailProgress + " tail_single_order_powered=" + tailPower + " tail_power_gate=" + tailGate +
   " ordinary_fresh_follow_gate=" + ordinaryGate + " tail_start_home_gap_m=" + m_fReturnStartGap +
   " tail_net_home_approach_m=" + m_fReturnNetApproach + " stable_max_drift_m=" + m_fReturnStableDrift +
   " production_return='" + m_PacedSession.CF_TestTripReturnReadback() + "' automated_native_lead_fixture=true owner_passenger=" + CommandSeat(m_PacedOwner, false) + " production_player_home_claim=false full_trip=false input_claim=false video_claim=false");
 }
 protected void ReturnEnd(bool pass, string reason)
 {
  ReturnResult(pass, reason); BayEnd(m_bBayParkComplete, "bay_complete_return_" + reason);
 }
 protected void ReturnSampleMotion(int unit, CF_PrivateReturnMotion sample)
 {
  CF_PacedRoadTruckSample s = m_PacedTrucks[unit];
  vector origin = s.Truck.GetOrigin(); float signed = vector.Dot(origin - sample.Previous, sample.Axis);
  SCR_AIGroupUtilityComponent groupUtility = GroupUtility(s); SCR_AIUtilityComponent utility = PilotUtility(s);
  AIActionBase action; if (groupUtility) action = groupUtility.GetCurrentAction();
  AIWaypoint waypoint = s.Group.GetCurrentWaypoint(); SCR_AIMoveActivity move = SCR_AIMoveActivity.Cast(action);
  SCR_AIMoveInFormationBehavior pilot; if (utility) pilot = SCR_AIMoveInFormationBehavior.Cast(utility.GetCurrentBehavior());
  int activeOrders;
  if (groupUtility) { array<ref AIActionBase> actions = {}; groupUtility.GetActions(actions); foreach (AIActionBase candidate : actions)
   if ((SCR_AIMoveActivity.Cast(candidate) || SCR_AIFollowActivity.Cast(candidate)) && candidate.GetActionState() != EAIActionState.COMPLETED && candidate.GetActionState() != EAIActionState.FAILED) activeOrders++; }
  bool exactMove = move && groupUtility.GetExecutedAction() == move && move.GetActionState() != EAIActionState.COMPLETED && move.GetActionState() != EAIActionState.FAILED &&
   move.m_RelatedWaypoint == waypoint && move.m_Entity.m_Value == waypoint && move.m_vPosition.m_Value == vector.Zero && move.m_bUseVehicles.m_Value &&
   pilot && pilot.GetRelatedGroupActivity() == move && pilot.GetActionState() != EAIActionState.COMPLETED && pilot.GetActionState() != EAIActionState.FAILED && activeOrders == 1 && ExactPilot(s);
  CF_OriginalFollowActivity ordinary; if (unit == 1) ordinary = ExactOriginalActivity();
  bool exactOrdinary = ordinary && ordinary == action && activeOrders == 1 && ReturnChain();
  bool exact = exactMove || exactOrdinary;
  if (!exact || action != sample.Action || waypoint != sample.Waypoint) { sample.Progress = 0; sample.Power = 0; }
  CarControllerComponent car = CarControllerComponent.Cast(s.Truck.FindComponent(CarControllerComponent));
  VehicleWheeledSimulation sim; if (car) sim = car.GetSimulation();
  bool powered = exact && sim && sim.EngineIsOn() && sim.GetGear() >= 2 && sim.GetThrottle() > 0.05 && Math.AbsFloat(sim.GetSpeedKmh()) > 1.5 && signed >= 0.25;
  if (exact) sample.Progress += signed; if (powered) sample.Power++;
  sample.BestProgress = Math.Max(sample.BestProgress, sample.Progress); sample.BestPower = Math.Max(sample.BestPower, sample.Power);
  if (exact && sample.Progress >= 20 && sample.Power >= 3) { sample.Qualified = true; if (exactOrdinary) sample.OrdinaryQualified = true; }
  Print("[ConvoyFollower] RETURN_FEATURE_MOTION: unit=" + unit + " phase=" + m_iBayPhase + " origin=" + origin +
   " action=" + action + " waypoint_id=" + EntityKey(waypoint) + " exact_native_move=" + exactMove + " exact_original_follow=" + exactOrdinary +
   " active_orders=" + activeOrders + " signed_home_axis_step_m=" + signed + " single_order_progress_m=" + sample.Progress +
   " single_order_powered=" + sample.Power + " powered=" + powered + " original_chain=" + ReturnChain() + " controls_written=false");
  sample.Previous = origin; sample.Action = action; sample.Waypoint = waypoint;
 }
 protected void ReturnPoll(float now)
 {
  float elapsed = now - m_fBayPhaseMs;
  Vehicle truck = m_PacedTrucks[1].Truck; CF_EntityFollowDriverControllerComponent driver = CommandDriver();
  Print("[ConvoyFollower] RETURN_FEATURE_SAMPLE: phase=" + m_iBayPhase + " elapsed_world_s=" + elapsed / 1000 +
   " lead=" + m_Lead.GetOrigin() + " tail=" + truck.GetOrigin() + " head_home_gap_m=" + vector.DistanceXZ(m_Lead.GetOrigin(), m_vReturnHeadHome) +
   " tail_home_gap_m=" + vector.DistanceXZ(truck.GetOrigin(), m_vReturnTailHome) + " driver_state=" + driver.CF_GetPanelStateLabel() +
   " original_chain=" + ReturnChain() + " owner_pilot=" + CommandSeat(m_PacedOwner, true) + " production='" + m_PacedSession.CF_TestTripReturnReadback() + "'");
  if (m_iBayPhase == 30)
  {
   if (!CommandSeat(m_PacedOwner, true) || !CommandOnFoot(m_Pilot)) { if (elapsed > 15000) ReturnEnd(false, "owner_native_entry_timeout"); return; }
   string before = m_PacedSession.CF_TestBayMutationSnapshot();
   m_bReturnCommand = CF_ConvoySession.CF_PanelRegroupReturn(m_PacedOwner, m_PacedSession.CF_TestReturnIdentity(driver));
   Print("[ConvoyFollower] RETURN_FEATURE_COMMAND: accepted=" + m_bReturnCommand + " before='" + before + "' after='" + m_PacedSession.CF_TestBayMutationSnapshot() + "' human_input=false");
   if (!m_bReturnCommand) { ReturnEnd(false, "production_regroup_rejected"); return; } BayPhase(31); return;
  }
  if (m_iBayPhase == 31)
  {
   if (!ReturnChain()) { if (elapsed > 20000) ReturnEnd(false, "original_merge_timeout"); return; }
   m_bReturnMergedObserved = true; BeginCommandSeatTransfer(false); BayPhase(32); return;
  }
  if (m_iBayPhase == 32)
  {
   if (!PollCommandSeatTransfer(false)) { if (elapsed > 15000) ReturnEnd(false, "native_return_seat_timeout"); return; }
   // Use the recorded original rear staging point so the head can physically
   // pass the returned truck; final original head/tail home radii stay25/40.
   if (vector.DistanceXZ(m_vReturnHeadHome, m_vReturnTailHome) > 25) { ReturnEnd(false, "recorded_tail_goal_outside_original_head_home_area"); return; }
   m_vRestartGoal = m_vReturnTailHome; m_vRestartAxis = m_vRestartGoal - m_Lead.GetOrigin(); m_vRestartAxis[1] = 0; m_vRestartAxis.Normalize();
   m_vReturnTailStart = truck.GetOrigin(); m_fReturnStartGap = vector.DistanceXZ(m_vReturnTailStart, m_vReturnTailHome);
   m_ReturnHead = new CF_PrivateReturnMotion(); m_ReturnHead.Previous = m_Lead.GetOrigin(); m_ReturnHead.Axis = m_vRestartAxis;
   m_ReturnTail = new CF_PrivateReturnMotion(); m_ReturnTail.Previous = m_vReturnTailStart;
   m_ReturnTail.Axis = m_vReturnTailHome - m_vReturnTailStart; m_ReturnTail.Axis[1] = 0; m_ReturnTail.Axis.Normalize();
   Print("[ConvoyFollower] RETURN_FEATURE_GOAL: native_goal=" + m_vRestartGoal + " original_head_home=" + m_vReturnHeadHome + " original_tail_home=" + m_vReturnTailHome + " moving_gap=" + CF_ConvoySettings.Get().m_fMovingGap + " completion_radius=" + CF_ConvoySettings.Get().GetMoveCompletionRadius() + " production_initial_guard_unchanged=true original_home_radii_unchanged=true");
   if (m_fReturnStartGap < 40 || !StartCommandRestart()) { ReturnEnd(false, "native_return_start_or_original_geometry_rejected"); return; }
   m_fReturnBeginMs = now; BayPhase(33); return;
  }
  if (!ReturnChain()) { ReturnEnd(false, "original_return_chain_lost"); return; }
  ReturnSampleMotion(0, m_ReturnHead); ReturnSampleMotion(1, m_ReturnTail);
  m_fReturnNetApproach = m_fReturnStartGap - vector.DistanceXZ(truck.GetOrigin(), m_vReturnTailHome);
  if (m_iBayPhase == 33)
  {
   if (vector.DistanceXZ(m_Lead.GetOrigin(), m_vRestartGoal) <= 8 && CanControlLead() && !m_bPacedHoldingLead) HoldLeadAtRoadGoal();
   if (LeadWaitSelected() && CommandSpeed(m_Lead) <= 2 && CommandSpeed(truck) <= 2 &&
    vector.DistanceXZ(m_Lead.GetOrigin(), m_vReturnHeadHome) <= 25 && vector.DistanceXZ(truck.GetOrigin(), m_vReturnTailHome) <= 40 &&
    m_ReturnHead.Qualified && m_ReturnTail.Qualified && m_fReturnNetApproach >= 20)
   { m_vReturnHeadStable = m_Lead.GetOrigin(); m_vReturnTailStable = truck.GetOrigin(); BayPhase(34); return; }
   if (now - m_fReturnBeginMs > 90000) ReturnEnd(false, "bounded_physical_home_approach_timeout"); return;
  }
  if (m_iBayPhase == 34)
  {
   if (CommandSpeed(m_Lead) > 2 || CommandSpeed(truck) > 2 || m_fReturnStableDrift > 2 ||
    vector.DistanceXZ(m_Lead.GetOrigin(), m_vReturnHeadHome) > 25 || vector.DistanceXZ(truck.GetOrigin(), m_vReturnTailHome) > 40 || m_fReturnNetApproach < 20)
   { ReturnEnd(false, "physical_home_stability_lost"); return; }
   if (elapsed >= 30000) ReturnEnd(true, "focused_original_regroup_home_approach_complete");
  }
 }

 protected int m_iBayPhase;
 protected float m_fBayPhaseMs;
 protected float m_fBayStableMs = -1;
 protected vector m_vBay;
 protected vector m_vHeld;
 protected vector m_vParked;
 protected vector m_vMotionPrevious;
 protected vector m_vMotionForward;
 protected float m_fLeadClearSigned;
 protected int m_iLeadClearPowered;
 protected float m_fLeadBayApproach;
 protected int m_iLeadBayApproachPowered;
 protected bool m_bLeadMotionGate;
 protected bool m_bBayMotionGate;
 protected bool m_bParkMotionGate;
 protected float m_fBayEntrySigned;
 protected int m_iBayEntryPowered;
 protected float m_fParkSigned;
 protected int m_iParkPowered;
 protected float m_fHoldDrift;
 protected float m_fParkDrift;
 protected AIActionBase m_BayLastMove;
 protected AIWaypoint m_BayLastWaypoint;
 protected float m_fExactProgress;
 protected int m_iExactPower;
 protected bool m_bBayRecorded;
 protected bool m_bBayAdmitted;
 protected bool m_bBaySettled;
 protected bool m_bBayUnload;
 protected bool m_bBayParking;
 protected bool m_bBayParkComplete;

 override void OnPostInit(IEntity owner)
 {
  super.OnPostInit(owner); SetEventMask(owner, EntityEvent.POSTFRAME);
  Print("[ConvoyFollower] BAY_STOP_GUARD_SCOPE: moving_negative_unverified=true optional_external_boarding_probe_bypassed=true stopped_owner_positive_pending=true speed_injected=false");
  Print("[ConvoyFollower] BAY_FEATURE_INIT: scope=physical_bay_native_unload_commanded_park_only full_trip=false following_pass=false arrival_180s_claim=false input_claim=false video_claim=false hold_s=30 hold_drift_m=2 admission_timeout_s=90 park_timeout_s=90 park_stable_s=30 short_distinct_destination_m=83 production_follower_unchanged=true");
 }
 protected bool m_bMovingGuardEntryAttempted;
 protected string m_sMovingSeatState;
 protected float m_fMovingSeatNextLogMs;
 protected bool m_bMovingGuardAttempted;
 protected bool m_bMovingGuardQualified;
 protected bool m_bMovingGuardPreserved;
 protected float m_fMovingGuardBeginMs;

 protected bool BayClearGoal()
 {
  ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
  if (!aiWorld || !aiWorld.GetRoadNetworkManager()) return false;
  RoadNetworkManager roads = aiWorld.GetRoadNetworkManager(); BaseRoad road; float roadDistance;
  int roadId = roads.GetClosestRoad(m_Lead.GetOrigin(), road, roadDistance);
  vector hint = Vector(7175.9, 142.247, 2512.32), reached;
  bool connected = roads.GetReachableWaypointInRoad(m_Lead.GetOrigin(), hint, 5, reached);
  float width; if (road) width = road.GetWidth();
  Print("[ConvoyFollower] BAY_FEATURE_CLEAR_SURVEY: road_id=" + roadId + " width=" + width + " road_distance=" + roadDistance +
   " start=" + m_Lead.GetOrigin() + " hint=" + hint + " connected=" + connected + " resolved=" + reached +
   " min_declared_width_m=3.5 single_native_lead=true geometry_only=true production_admission_geometry_unchanged=true");
  if (!road || roadId < 0 || width < 3.5 || roadDistance > 5 || !connected || vector.DistanceXZ(hint, reached) > 3 ||
   vector.DistanceXZ(m_Lead.GetOrigin(), reached) < 35 || vector.DistanceXZ(m_Lead.GetOrigin(), reached) > 65) return false;
  m_vRestartGoal = reached; return true;
 }
 protected bool BayBeginMovingGuard()
 {
  if (m_bMovingGuardAttempted || !CanControlLead() || !OwnerRetained() || !m_PilotWaypoint ||
   CommandSpeed(m_Lead) < 10 || m_fExactProgress < 20 || m_iExactPower < 3) return false;
  m_bMovingGuardAttempted = true; m_fMovingGuardBeginMs = GetGame().GetWorld().GetWorldTime();
  // Retire only our original native lead order while its original pilot still owns it.
  SCR_AIGroupUtilityComponent groupUtility = GroupUtility(m_PacedTrucks[0]);
  groupUtility.CancelActivitiesRelatedToWaypoint(m_PilotWaypoint, doNotCompleteWaypoint: true);
  m_PilotGroup.RemoveWaypoint(m_PilotWaypoint); m_PilotWaypoint = null;
  m_PacedCruise.Release("bay_moving_guard_native_handoff");
  Print("[ConvoyFollower] BAY_STOP_GUARD_HANDOFF: native_powered_progress_m=" + m_fExactProgress + " native_powered_samples=" + m_iExactPower +
   " measured_speed_kmh=" + CommandSpeed(m_Lead) + " lead_order_retired=true truck_pose_written=false controls_written=false human_input=false");
  if (!ExitCommandSeat(m_PacedOwner) || !ExitCommandSeat(m_Pilot)) { BayEnd(false, "moving_guard_native_exit_rejected"); return true; }
  BayPhase(20); return true;
 }
 protected void BayPollMovingGuard(float now)
 {
  if (m_bPacedTerminal) return;
  if (now - m_fMovingGuardBeginMs > 15000) { BayEnd(false, "moving_guard_seat_or_stop_timeout"); return; }
  if (m_iBayPhase == 20)
  {
   if (!CommandOnFoot(m_Pilot) || !CommandOnFoot(m_PacedOwner)) return;
   CompartmentAccessComponent ownerAccess = m_PacedOwner.GetCompartmentAccessComponent();
   CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
   BaseCompartmentSlot slot; if (car) slot = car.GetPilotCompartmentSlot();
   bool occupied = slot && slot.IsOccupied(), reserved = slot && slot.IsReserved();
   bool accessible = slot && slot.IsCompartmentAccessible(), locked = slot && slot.IsGetInLockedFor(m_PacedOwner);
   bool nativeCanEnter = ownerAccess && ownerAccess.CanGetInVehicle(m_Lead);
   string state = "occupied=" + occupied + " reserved=" + reserved + " accessible=" + accessible + " locked=" + locked + " native_can_enter=" + nativeCanEnter;
   if (state != m_sMovingSeatState || now >= m_fMovingSeatNextLogMs)
   { m_sMovingSeatState = state; m_fMovingSeatNextLogMs = now + 500; Print("[ConvoyFollower] BAY_STOP_GUARD_SEAT_READINESS: " + state + " speed_kmh=" + CommandSpeed(m_Lead) + " commands_attempted=false"); }
   if (!slot || occupied || reserved || !accessible || locked || !nativeCanEnter || (m_bMovingGuardEntryAttempted && CommandSpeed(m_Lead) > 2)) return;
   if (!BoardCommandSeat(m_PacedOwner, true))
   {
    // One eligible moving-entry request may still be refused internally.
    // Recover only once later in a genuinely stopped state; no guard credit.
    if (m_bMovingGuardEntryAttempted || CommandSpeed(m_Lead) <= 2) { BayEnd(false, "eligible_owner_entry_or_stopped_recovery_rejected"); return; }
    m_bMovingGuardEntryAttempted = true;
    Print("[ConvoyFollower] BAY_STOP_GUARD_NEGATIVE: qualified=false reason=eligible_moving_seat_request_rejected command_attempted=false speed_injected=false recovery=wait_for_actual_stop");
    return;
   }
   BayPhase(21); return;
  }
  if (m_iBayPhase == 21)
  {
   if (!CommandSeat(m_PacedOwner, true)) return;
   float speed = CommandSpeed(m_Lead);
   string before = m_PacedSession.CF_TestBayMutationSnapshot();
   bool rejected;
   if (speed > 2)
   {
    rejected = !CF_ConvoySession.CF_PanelSetUnloadBay(m_PacedOwner);
    string after = m_PacedSession.CF_TestBayMutationSnapshot();
    m_bMovingGuardPreserved = before == after;
    m_bMovingGuardQualified = rejected && m_bMovingGuardPreserved;
    Print("[ConvoyFollower] BAY_STOP_GUARD_NEGATIVE: owner_pilot=true measured_speed_kmh=" + speed + " rejected=" + rejected +
     " state_preserved=" + m_bMovingGuardPreserved + " before='" + before + "' after='" + after + "' speed_injected=false");
    if (!m_bMovingGuardQualified) { BayEnd(false, "moving_guard_accepted_or_mutated_state"); return; }
   }
   else Print("[ConvoyFollower] BAY_STOP_GUARD_NEGATIVE: qualified=false measured_speed_kmh=" + speed + " reason=native_handoff_stopped_before_owner_entry command_attempted=false fake_speed=false");
   m_bCommandParking = true; ApplyCommandParking(); BayPhase(22); return;
  }
  if (m_iBayPhase == 22)
  {
   if (CommandSpeed(m_Lead) > 2) return;
   BeginCommandSeatTransfer(false); BayPhase(23); return;
  }
  if (m_iBayPhase == 23)
  {
   if (!PollCommandSeatTransfer(false)) return;
   if (!StartCommandRestart()) { BayEnd(false, "moving_guard_native_approach_resume_rejected"); return; }
   m_bLeadMotionGate = false; m_fLeadClearSigned = 0; m_iLeadClearPowered = 0;
   BayPhase(6);
  }
 }
 override void EOnPostFrame(IEntity owner, float timeSlice)
 {
  super.EOnPostFrame(owner, timeSlice);
  if (!m_bPacedTerminal && PacedWorldAlive() && m_PacedTrucks.Count() == 2)
  {
   Vehicle heldTruck = m_PacedTrucks[1].Truck;
   if (m_iBayPhase == 3 || m_iBayPhase == 4 || m_iBayPhase == 6 || m_iBayPhase == 7 ||
    m_iBayPhase == 15 || m_iBayPhase == 16 || m_iBayPhase == 17 || (m_iBayPhase >= 20 && m_iBayPhase < 30))
    m_fHoldDrift = Math.Max(m_fHoldDrift, vector.DistanceXZ(heldTruck.GetOrigin(), m_vHeld));
   if (m_iBayPhase == 34)
    m_fReturnStableDrift = Math.Max(m_fReturnStableDrift, Math.Max(vector.DistanceXZ(m_Lead.GetOrigin(), m_vReturnHeadStable), vector.DistanceXZ(heldTruck.GetOrigin(), m_vReturnTailStable)));
   if (m_iBayPhase == 13) m_fParkDrift = Math.Max(m_fParkDrift, vector.DistanceXZ(heldTruck.GetOrigin(), m_vParked));
  }
  if (!m_bPacedTerminal && PacedWorldAlive() && m_iBayPhase >= 20 && m_iBayPhase < 30)
  {
   if (!BayIdentities()) { BayEnd(false, "moving_guard_original_identity_lost"); return; }
   BayPollMovingGuard(GetGame().GetWorld().GetWorldTime());
  }
 }

 protected void BayPhase(int phase)
 {
  m_iBayPhase = phase; m_fBayPhaseMs = GetGame().GetWorld().GetWorldTime(); m_fBayStableMs = -1;
  m_BayLastMove = null; m_BayLastWaypoint = null; m_fExactProgress = 0; m_iExactPower = 0;
  if (m_PacedTrucks.Count() == 2)
  {
   Vehicle motionTruck = m_PacedTrucks[1].Truck; if (phase == 6) motionTruck = m_Lead;
   m_vMotionPrevious = motionTruck.GetOrigin(); m_vMotionForward = motionTruck.GetWorldTransformAxis(2);
  }
  Print("[ConvoyFollower] BAY_FEATURE_PHASE: phase=" + phase + " world_ms=" + m_fBayPhaseMs + " completion_claim=false");
 }
 protected bool BayIdentities()
 {
  if (!PacedWorldAlive() || m_PacedTrucks.Count() != 2 || !m_PacedSession || CF_ConvoySession.GetForPlayer(m_PacedOwner) != m_PacedSession) return false;
  CF_PacedRoadTruckSample s = m_PacedTrucks[1];
  return EntityAlive(s.Truck) && EntityAlive(s.Pilot) && EntityAlive(m_Lead) && EntityAlive(m_Pilot) &&
   s.Driver && s.Driver.CF_GetDriverEntity() == s.Pilot && s.Driver.CF_GetAssignedVehicle() == s.Truck &&
   ExactPilot(s) && PilotUtility(s) &&
   GetGame().GetWorld().FindEntityByName("CF_SmokeFollower1") == s.Truck &&
   GetGame().GetWorld().FindEntityByName("CF_SmokeLead") == m_Lead &&
   m_Player == m_PacedOwner && GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iPacedOwnerId) == m_PacedOwner;
 }
 protected void BayEnd(bool pass, string reason)
 {
  if (m_bPacedTerminal) return;
  if (m_iBayPhase >= 30) { ReturnResult(false, reason); pass = m_bBayParkComplete; }
  if (CanControlLead() && !m_bPacedHoldingLead) HoldLeadAtRoadGoal();
  Print("[ConvoyFollower] BAY_FEATURE_RESULT: feature_pass=" + pass + " reason=" + reason +
   " moving_guard_attempted=" + m_bMovingGuardAttempted + " moving_guard_qualified=" + m_bMovingGuardQualified + " moving_guard_preserved=" + m_bMovingGuardPreserved + " recorded=" + m_bBayRecorded + " admitted=" + m_bBayAdmitted + " settled=" + m_bBaySettled +
   " native_unload=" + m_bBayUnload + " commanded_park=" + m_bBayParking + " parked=" + m_bBayParkComplete +
   " lead_clear_signed_m=" + m_fLeadClearSigned + " lead_clear_powered=" + m_iLeadClearPowered +
   " lead_bay_approach_m=" + m_fLeadBayApproach + " lead_bay_approach_powered=" + m_iLeadBayApproachPowered +
   " bay_signed_m=" + m_fBayEntrySigned + " bay_powered=" + m_iBayEntryPowered +
   " park_signed_m=" + m_fParkSigned + " park_powered=" + m_iParkPowered +
   " hold_max_drift_m=" + m_fHoldDrift + " park_max_drift_m=" + m_fParkDrift +
   " physical_originals=" + BayIdentities() + " full_trip=false following_pass=false arrival_180s_claim=false input_claim=false");
  if (pass && !m_bMovingGuardQualified) Print("[ConvoyFollower] BAY_STOP_GUARD_RESULT: qualified=false reason=optional_moving_negative_unverified positive_stopped_record=" + m_bBayRecorded + " fake_speed=false");
  // PACED_RESULT remains FAIL for reduced scope even if this feature qualifies.
  NoteFailure("fixture_scope", "bay_feature_" + reason + "_not_full_trip");
  if (m_PacedCruise && !CanControlLead()) m_PacedCruise.Release("bay_terminal_without_owned_native_lead");
  m_bPacedTerminal = true; m_bFinished = true;
  Print("[ConvoyFollower] PACED_RESULT: FAIL run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
   " seconds=" + PacedSeconds() + " expected=1 peak_gap_m=" + m_fPacedPeakGap +
   " observation_samples=" + m_iObservationSamples + " first_failure=" + m_sFirstFailure + " owned_lead_park_retained=" + m_bPacedHoldingLead);
  GetGame().GetCallqueue().Remove(Poll); SchedulePacedExit();
 }
 override protected void EndPaced()
 {
  BayEnd(false, "inherited_failure");
 }
 protected void BaySampleMotion(int unit)
 {
  CF_PacedRoadTruckSample s = m_PacedTrucks[unit];
  vector origin = s.Truck.GetOrigin(), forward = Vector(m_vMotionForward[0], 0, m_vMotionForward[2]); forward.Normalize();
  vector delta = origin - m_vMotionPrevious;
  float signed = vector.Dot(delta, forward);
  CarControllerComponent car = CarControllerComponent.Cast(s.Truck.FindComponent(CarControllerComponent));
  VehicleWheeledSimulation sim; if (car) sim = car.GetSimulation();
  SCR_AIGroupUtilityComponent groupUtility = GroupUtility(s); SCR_AIUtilityComponent utility = PilotUtility(s);
  AIActionBase action; if (groupUtility) action = groupUtility.GetCurrentAction();
  SCR_AIMoveActivity move = SCR_AIMoveActivity.Cast(action); AIWaypoint waypoint = s.Group.GetCurrentWaypoint();
  SCR_AIMoveInFormationBehavior pilot; if (utility) pilot = SCR_AIMoveInFormationBehavior.Cast(utility.GetCurrentBehavior());
  int activeOrders;
  if (groupUtility) { array<ref AIActionBase> actions = {}; groupUtility.GetActions(actions); foreach (AIActionBase candidate : actions)
   if ((SCR_AIMoveActivity.Cast(candidate) || SCR_AIFollowActivity.Cast(candidate)) && candidate.GetActionState() != EAIActionState.COMPLETED && candidate.GetActionState() != EAIActionState.FAILED) activeOrders++; }
  bool exact = move && move.GetActionState() != EAIActionState.COMPLETED && move.GetActionState() != EAIActionState.FAILED &&
   groupUtility.GetExecutedAction() == move && move.m_RelatedWaypoint == waypoint && move.m_Entity.m_Value == waypoint &&
   move.m_vPosition.m_Value == vector.Zero && move.m_bUseVehicles.m_Value && pilot && pilot.GetRelatedGroupActivity() == move &&
   pilot.GetActionState() != EAIActionState.COMPLETED && pilot.GetActionState() != EAIActionState.FAILED && activeOrders == 1 && ExactPilot(s);
  if (!exact || move != m_BayLastMove || waypoint != m_BayLastWaypoint) { m_fExactProgress = 0; m_iExactPower = 0; }
  bool powered = exact && sim && sim.EngineIsOn() && sim.GetGear() >= 2 && sim.GetThrottle() > 0.05 && Math.AbsFloat(sim.GetSpeedKmh()) > 1.5 && signed >= 0.25;
  if (exact) m_fExactProgress += signed;
  if (powered) m_iExactPower++;
  if (exact && m_fExactProgress >= 20 && m_iExactPower >= 3 && unit == 0) m_bLeadMotionGate = true;
  if (exact && m_fExactProgress >= 5 && m_iExactPower >= 3 && unit == 1 && m_iBayPhase == 8) m_bBayMotionGate = true;
  if (exact && m_fExactProgress >= 5 && m_iExactPower >= 3 && unit == 1 && m_iBayPhase == 12) m_bParkMotionGate = true;
  if (unit == 0) { m_fLeadClearSigned = Math.Max(m_fLeadClearSigned, m_fExactProgress); m_iLeadClearPowered = Math.Max(m_iLeadClearPowered, m_iExactPower); }
  else if (m_iBayPhase == 8) { m_fBayEntrySigned = Math.Max(m_fBayEntrySigned, m_fExactProgress); m_iBayEntryPowered = Math.Max(m_iBayEntryPowered, m_iExactPower); }
  else if (m_iBayPhase == 12) { m_fParkSigned = Math.Max(m_fParkSigned, m_fExactProgress); m_iParkPowered = Math.Max(m_iParkPowered, m_iExactPower); }
  Print("[ConvoyFollower] BAY_FEATURE_MOTION: phase=" + m_iBayPhase + " unit=" + unit + " original_identity=" + BayIdentities() +
   " truck_id=" + EntityKey(s.Truck) + " pilot_id=" + EntityKey(s.Pilot) + " origin=" + origin + " native_move=" + move +
   " waypoint_id=" + EntityKey(waypoint) + " native_pilot=" + pilot + " active_orders=" + activeOrders +
   " exact=" + exact + " signed_step_m=" + signed + " single_order_progress_m=" + m_fExactProgress + " single_order_powered=" + m_iExactPower +
   " powered=" + powered + " engine=" + (sim && sim.EngineIsOn()) + " speed_kmh=" + CommandSpeed(s.Truck) + " gear=" + sim.GetGear() + " throttle=" + sim.GetThrottle() + " brake=" + sim.GetBrake() + " controls_written=false");
  m_BayLastMove = move; m_BayLastWaypoint = waypoint; m_vMotionPrevious = origin; m_vMotionForward = s.Truck.GetWorldTransformAxis(2);
 }
 override protected void Poll()
 {
  if (!PacedWorldAlive() || m_bPacedTerminal) return;
  float now = GetGame().GetWorld().GetWorldTime();
  if (now - m_fPacedStartMs > 1650000) { BayEnd(false, "1650world_total_timeout"); return; }
  if (m_iBayPhase == 0)
  {
   if (m_iStage == 2 && m_iNextOrder == 1)
   {
    BindOriginals();
    if (m_PacedTrucks.Count() == 2 && StartBarrier())
    {
     if (!BayIdentities() || !m_TripCargo || !m_TripCargo.TripLoaded()) { BayEnd(false, "loaded_original_setup_invalid"); return; }
     m_vBay = m_Lead.GetOrigin(); HoldLeadAtRoadGoal(); BayPhase(1); return;
    }
   }
   super.Poll(); return;
  }
  if (!BayIdentities() || !m_TripCargo || m_TripCargo.TripFailed()) { BayEnd(false, "original_identity_or_native_cargo_failure"); return; }
  CF_EntityFollowDriverControllerComponent driver = CommandDriver(); Vehicle truck = m_PacedTrucks[1].Truck;
  if (!driver || driver.CF_HasActiveEntityFallbackFailure()) { BayEnd(false, "persistent_production_controller_failure"); return; }
  float elapsed = now - m_fBayPhaseMs;
  Print("[ConvoyFollower] BAY_FEATURE_SAMPLE: phase=" + m_iBayPhase + " elapsed_world_s=" + elapsed / 1000 +
   " truck=" + truck.GetOrigin() + " lead=" + m_Lead.GetOrigin() + " bay=" + m_vBay + " driver_state=" + driver.CF_GetPanelStateLabel() +
   " held=" + driver.CF_IsPanelHeld() + " explicit_bay_wait=" + driver.CF_HasSelectedExplicitBayWait(m_PacedSession, truck) +
   " ordinary_arrival_wait=" + driver.CF_HasSelectedArrivalWait(m_PacedSession, truck) + " owner_pilot=" + CommandSeat(m_PacedOwner, true) + " originals=true world_scale=" + GetGame().GetWorld().GetTimeScale());
  if (m_iBayPhase == 8) { Print("[ConvoyFollower] BAY_WAIT_PRECONDITIONS: " + driver.CF_TestExplicitBayWaitReadback()); Print("[ConvoyFollower] BAY_WAIT_SESSION: " + m_PacedSession.CF_TestBayMutationSnapshot()); }
  if (m_iBayPhase >= 30) { ReturnPoll(now); return; }
  if (m_iBayPhase >= 20) { BayPollMovingGuard(now); return; }
  if (m_iBayPhase == 1)
  {
   if (!LeadWaitSelected() || CommandSpeed(m_Lead) > 2) { if (elapsed > 10000) BayEnd(false, "initial_native_lead_stop_timeout"); return; }
   BeginCommandSeatTransfer(true); BayPhase(2); return;
  }
  if (m_iBayPhase == 2)
  {
   if (!PollCommandSeatTransfer(true)) { if (elapsed > 15000) BayEnd(false, "owner_bay_record_seat_timeout"); return; }
   if (!ReturnCaptureStaging()) { if (elapsed > 15000) BayEnd(false, "original_staging_after_owner_entry_timeout"); return; }
   if (!CF_ConvoySession.CF_PanelHold(m_PacedOwner)) { BayEnd(false, "production_queue_hold_rejected"); return; }
   m_vHeld = truck.GetOrigin(); BayPhase(3); return;
  }
  if (m_iBayPhase == 3)
  {
   m_fHoldDrift = Math.Max(m_fHoldDrift, vector.DistanceXZ(truck.GetOrigin(), m_vHeld));
   if (m_fHoldDrift > 2) { BayEnd(false, "queue_hold_drift"); return; }
   if (!driver.CF_IsPanelHeld() || CommandSpeed(truck) > 2) { m_fBayStableMs = -1; if (elapsed > 15000) BayEnd(false, "production_hold_capture_timeout"); return; }
   if (m_fBayStableMs < 0) { m_fBayStableMs = now; m_vHeld = truck.GetOrigin(); }
   if (now - m_fBayStableMs < 30000) return;
   BeginCommandSeatTransfer(false); BayPhase(4); return;
  }
  if (m_iBayPhase == 4)
  {
   if (!PollCommandSeatTransfer(false)) { if (elapsed > 15000) BayEnd(false, "native_lead_clear_seat_timeout"); return; }
   // Authored short bay on an already observed native road, never a follower route/following claim.
   ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld()); vector reached;
   vector hint = Vector(7180.53, 140.153, 2468.41);
   if (!aiWorld || !aiWorld.GetRoadNetworkManager() || !aiWorld.GetRoadNetworkManager().GetReachableWaypointInRoad(m_Lead.GetOrigin(), hint, 5, reached) ||
    vector.DistanceXZ(hint, reached) > 3 || vector.DistanceXZ(m_Lead.GetOrigin(), reached) < 60) { BayEnd(false, "native_bay_approach_geometry_rejected"); return; }
   m_vRestartGoal = reached;
   if (!StartCommandRestart()) { BayEnd(false, "native_physical_bay_route_rejected"); return; }
   BayPhase(6); return;
  }
  if (m_iBayPhase == 6)
  {
   BaySampleMotion(0); /* Moving external-boarding negative preserved unqualified in V2/V3; this run continues the stopped physical bay workflow. */ m_fHoldDrift = Math.Max(m_fHoldDrift, vector.DistanceXZ(truck.GetOrigin(), m_vHeld));
   if (!driver.CF_IsPanelHeld() || m_fHoldDrift > 2) { BayEnd(false, "queue_moved_during_lead_clear"); return; }
   if (vector.DistanceXZ(m_Lead.GetOrigin(), m_vRestartGoal) <= 8) HoldLeadAtRoadGoal();
   if (LeadWaitSelected() && CommandSpeed(m_Lead) <= 2 && m_bLeadMotionGate)
   { BeginCommandSeatTransfer(true); BayPhase(7); return; }
   if (elapsed > 90000) BayEnd(false, "native_powered_lead_clear_timeout"); return;
  }
  if (m_iBayPhase == 7)
  {
   if (!PollCommandSeatTransfer(true)) { if (elapsed > 15000) BayEnd(false, "owner_admit_seat_timeout"); return; }
   m_vBay = m_Lead.GetOrigin();
   if (!CF_ConvoySession.CF_PanelSetUnloadBay(m_PacedOwner)) { BayEnd(false, "production_bay_record_rejected"); return; }
   m_bBayRecorded = true;
   m_fLeadBayApproach = m_fLeadClearSigned; m_iLeadBayApproachPowered = m_iLeadClearPowered;
   if (CF_ConvoySession.CF_PanelAdmitNextTruck(m_PacedOwner)) { BayEnd(false, "occupied_bay_incorrectly_admitted"); return; }
   Print("[ConvoyFollower] BAY_FEATURE_OCCUPIED_REJECTION: rejected=true lead_still_in_bay=true queue_held=" + driver.CF_IsPanelHeld() + " bay=" + m_vBay);
   BeginCommandSeatTransfer(false); BayPhase(15); return;
  }
  if (m_iBayPhase == 15)
  {
   if (!PollCommandSeatTransfer(false)) { if (elapsed > 15000) BayEnd(false, "native_lead_clear_seat_timeout"); return; }
   if (!BayClearGoal() || !StartCommandRestart()) { BayEnd(false, "native_physical_clear_route_rejected"); return; }
   m_bLeadMotionGate = false; m_fLeadClearSigned = 0; m_iLeadClearPowered = 0;
   BayPhase(16); m_vMotionPrevious = m_Lead.GetOrigin(); m_vMotionForward = m_Lead.GetWorldTransformAxis(2); return;
  }
  if (m_iBayPhase == 16)
  {
   BaySampleMotion(0); m_fHoldDrift = Math.Max(m_fHoldDrift, vector.DistanceXZ(truck.GetOrigin(), m_vHeld));
   if (!driver.CF_IsPanelHeld() || m_fHoldDrift > 2) { BayEnd(false, "queue_moved_during_lead_clear"); return; }
   if (vector.DistanceXZ(m_Lead.GetOrigin(), m_vRestartGoal) <= 8) HoldLeadAtRoadGoal();
   if (LeadWaitSelected() && CommandSpeed(m_Lead) <= 2 && m_bLeadMotionGate)
   { BeginCommandSeatTransfer(true); BayPhase(17); return; }
   if (elapsed > 90000) BayEnd(false, "native_powered_lead_clear_timeout"); return;
  }
  if (m_iBayPhase == 17)
  {
   if (!PollCommandSeatTransfer(true)) { if (elapsed > 15000) BayEnd(false, "owner_admit_seat_timeout"); return; }
   if (!CF_ConvoySession.CF_PanelAdmitNextTruck(m_PacedOwner)) { BayEnd(false, "production_admission_rejected"); return; }
   m_bBayAdmitted = true; BayPhase(8); return;
  }
  if (m_iBayPhase == 8)
  {
   BaySampleMotion(1);
   bool settled = driver.CF_IsSettledAtExplicitBay(m_vBay) && driver.CF_HasSelectedExplicitBayWait(m_PacedSession, truck) && CF_ConvoySession.CanReleaseAtUnload(m_PacedOwner, driver);
   if (!settled) m_fBayStableMs = -1;
   else if (m_fBayStableMs < 0) m_fBayStableMs = now;
   if (settled && now - m_fBayStableMs >= 3000)
   {
    if (!m_bBayMotionGate) { BayEnd(false, "bay_native_powered_progress_missing"); return; }
    m_bBaySettled = true;
    if (!ExitCommandSeat(m_PacedOwner)) { BayEnd(false, "native_unload_owner_exit_rejected"); return; }
    BayPhase(9); return;
   }
   if (elapsed > 90000) BayEnd(false, "physical_bay_arrival_timeout"); return;
  }
  if (m_iBayPhase == 9)
  {
   if (!CommandOnFoot(m_PacedOwner)) { if (elapsed > 15000) BayEnd(false, "unload_owner_exit_timeout"); return; }
   if (!m_TripCargo.StageOwnerAtRear() || !m_TripCargo.RequestTripUnload()) { BayEnd(false, "native_rear_unload_request_rejected"); return; }
   BayPhase(10); return;
  }
  if (m_iBayPhase == 10)
  {
   if (!driver.CF_IsSettledAtExplicitBay(m_vBay) || !driver.CF_HasSelectedExplicitBayWait(m_PacedSession, truck)) { BayEnd(false, "bay_ownership_lost_during_native_unload"); return; }
   if (!m_TripCargo.TripDelivered()) { if (elapsed > 30000) BayEnd(false, "native_destination_unload_timeout"); return; }
   m_bBayUnload = true;
   if (!CF_ConvoySession.CanPlanReleaseAtUnload(m_PacedOwner, driver) || !CF_ConvoySession.ReleaseAtUnload(m_PacedOwner, driver)) { BayEnd(false, "production_safe_parking_rejected"); return; }
   m_bBayParking = true; BayPhase(12); return;
  }
  if (m_iBayPhase == 12)
  {
   BaySampleMotion(1);
   if (driver.CF_IsAtUnloadWaitingPoint() && driver.CF_IsUnloadDeparted())
   { if (!m_bParkMotionGate) { BayEnd(false, "parking_powered_progress_missing"); return; } m_vParked = truck.GetOrigin(); BayPhase(13); return; }
   if (elapsed > 90000) BayEnd(false, "commanded_physical_parking_timeout"); return;
  }
  if (m_iBayPhase == 13)
  {
   m_fParkDrift = Math.Max(m_fParkDrift, vector.DistanceXZ(truck.GetOrigin(), m_vParked));
   if (!driver.CF_IsAtUnloadWaitingPoint() || !driver.CF_IsUnloadDeparted() || CommandSpeed(truck) > 2 || m_fParkDrift > 2) { BayEnd(false, "safe_park_state_or_drift_lost"); return; }
   if (elapsed >= 30000)
   {
    m_bBayParkComplete = true;
    Print("[ConvoyFollower] BAY_FEATURE_BOUNDARY: feature_pass=true original_parked=" + truck.GetOrigin() + " original_tail_staging=" + m_vReturnTailHome + " home_not_yet_qualified=true");
    Print("[ConvoyFollower] RETURN_FEATURE_ENTRY_READY: scoped_park=" + CanCommandPark() + " active_unit=" + m_PacedSession.GetUnitNumber(driver) + " original_parked=" + m_PacedSession.CF_TestOriginalReturnParked(driver) + " owner_onfoot=" + CommandOnFoot(m_PacedOwner) + " pilot_onfoot=" + CommandOnFoot(m_Pilot) + " lead_speed_kmh=" + CommandSpeed(m_Lead) + " native_request_not_yet_sent=true");
    if (!CanCommandPark() || CommandSpeed(m_Lead) > 2 || !CommandOnFoot(m_PacedOwner) || !CommandOnFoot(m_Pilot) || !BoardCommandSeat(m_PacedOwner, true))
    { ReturnResult(false, "owner_return_entry_rejected"); BayEnd(true, "bay_complete_owner_return_entry_rejected"); return; }
    BayPhase(30);
   }
  }
 }
}
