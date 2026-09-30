// Test-only value snapshot. No mutation, AI handle or ownership override.
modded class CF_ConvoySession
{
 string CF_TestBayMutationSnapshot()
 {
  string result = "bay=" + m_bExplicitBay + ",position=" + m_vUnloadBayPosition + ",anchor=" + m_vUnloadAnchor;
  result += ",anchor_valid=" + m_bUnloadBayPositionValid + ",anchor_lock=" + m_bUnloadAnchorLock + ",panel_order=" + m_iPanelOrder;
  result += ",unload_phase=" + m_iUnloadPhase + ",units=" + m_aUnits.Count() + ",return=" + m_aReturnQueue.Count() + ",forward=" + m_aForwardWait.Count();
  foreach (CF_DriverControllerComponent unit : m_aUnits)
   if (unit) result += ",unit=" + unit.CF_GetDriverEntity().GetID() + "/" + unit.CF_GetAssignedVehicle().GetID() + "/" + unit.CF_HasPanelHoldRequest() + "/" + unit.CF_IsPanelHeld();
  return result;
 }
}

modded class CF_EntityFollowDriverControllerComponent
{
 string CF_TestExplicitBayWaitReadback()
 {
  vector bay;
  ChimeraCharacter driver = ChimeraCharacter.Cast(m_Driver);
  bool admitted = m_Session && m_Session.CF_GetAdmittedExplicitBay(this, m_Truck, m_LeadVehicle, bay);
  bool settled = CF_IsSettledAtExplicitBay(bay);
  string result = "admitted=" + admitted + " settled=" + settled + " context=" + CF_IsExplicitBayWaitContext(bay);
  result += " capture_context=" + CF_IsExplicitBayWaitContext(bay, true) + " capture_ownership=" + CF_CapturedWaitOwnershipReady(driver, m_Truck, m_Group, true) + " tracked_bay_move=" + m_ExplicitBayMove;
  result += " ownership=" + CF_CapturedWaitOwnershipReady(driver, m_Truck, m_Group) + " stable_start_ms=" + m_fExplicitBayStableStartMs;
  result += " prototype=" + m_bEntityFollowPrototype + " wait_enabled=" + m_bEntityCapturedWait + " control_blocked=" + CF_IsControlBlocked();
  result += " native_waypoint=" + m_Waypoint + " original_lease=" + m_OriginalFollowLease + " deferred_waypoint=" + m_bDeferredWaypointClear;
  result += " deferred_dismiss=" + m_bDeferredControlDismiss + " reset_pending=" + m_bOriginalFollowResetPending;
  result += " fallback_failed=" + m_bEntityFallbackFailed + " original_blocked=" + m_bOriginalFollowBlocked + " recovery_blocked=" + m_bArrivalRoadRecoveryBlocked;
  result += " unload_hold=" + m_bUnloadSequenceHold + " bay_goal_valid=" + m_bUnloadBayGoalValid + " release_ready=" + m_bUnloadReleaseReady;
  result += " panel_requested=" + m_bPanelHoldRequested + " forward_requested=" + m_bForwardOutboundHoldRequested;
  result += " lead=" + m_LeadVehicle + " target=" + GetTargetVehicle(false) + " owner=" + m_Leader + " owner_id=" + m_iOrderingPlayerId;
  if (m_Session) result += " unit_number=" + m_Session.GetUnitNumber(this) + " owner_session=" + (CF_ConvoySession.GetForPlayer(m_Leader) == m_Session);
  result += " other_assignment=" + CF_ConvoySession.IsVehicleAssignedToAnotherDriver(m_Truck, this);
  if (m_Waypoint) result += " owned_waypoint_id=" + m_Waypoint.GetID() + " owned_goal=" + m_Waypoint.GetOrigin();
  if (driver) result += " live_original_pilot=" + CF_IsLiveAIPilot(driver, m_Truck);
  if (driver && driver.GetCompartmentAccessComponent()) result += " getting_in=" + driver.GetCompartmentAccessComponent().IsGettingIn() + " getting_out=" + driver.GetCompartmentAccessComponent().IsGettingOut();
  if (m_Group)
  {
   result += " group_agents=" + m_Group.GetAgentsCount();
   AIWaypoint current = m_Group.GetCurrentWaypoint();
   if (current) result += " current_waypoint_id=" + current.GetID() + " current_goal=" + current.GetOrigin();
   array<AIWaypoint> queued = {}; m_Group.GetWaypoints(queued);
   foreach (AIWaypoint waypoint : queued) if (waypoint) result += " queued_waypoint_id=" + waypoint.GetID() + " queued_goal=" + waypoint.GetOrigin();
   SCR_AIGroupUtilityComponent groupUtility = SCR_AIGroupUtilityComponent.Cast(m_Group.FindComponent(SCR_AIGroupUtilityComponent));
   if (groupUtility)
   {
    array<ref AIActionBase> actions = {}; groupUtility.GetActions(actions);
    foreach (AIActionBase action : actions)
    {
     if (!action) continue;
     result += " group_action=" + action.Type() + "/" + action.GetActionState();
     SCR_AIMoveActivity move = SCR_AIMoveActivity.Cast(action);
     if (move && move.m_RelatedWaypoint) result += " move_waypoint_id=" + move.m_RelatedWaypoint.GetID();
    }
   }
  }
  if (driver && driver.GetAIControlComponent() && driver.GetAIControlComponent().GetAIAgent())
  {
   SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(driver.GetAIControlComponent().GetAIAgent().FindComponent(SCR_AIUtilityComponent));
   if (utility)
   {
    array<ref AIActionBase> behaviors = {}; utility.GetActions(behaviors);
    foreach (AIActionBase behavior : behaviors) if (behavior) result += " pilot_action=" + behavior.Type() + "/" + behavior.GetActionState();
   }
  }
  return result;
 }
}

// Private immutable recruitment and return-state reads. Never changes owner gate.
modded class CF_ConvoySession
{
 bool CF_TestOriginalStaging(CF_DriverControllerComponent driver, IEntity lead, out vector head, out vector tail)
 {
  if (!m_bOriginalStagingValid || m_OriginalLeadVehicle != lead) return false;
  foreach (CF_ConvoyTripMember member : m_aTripMembers)
   if (member.Driver == driver && member.DriverEntity == driver.CF_GetDriverEntity() && member.Truck == driver.CF_GetAssignedVehicle())
   { head = m_vOriginalStaging; tail = member.StagingPosition; return true; }
  return false;
 }
 bool CF_TestOriginalReturnMerged() { return m_bReturnMerged && !m_bReturnPending && CF_TripRosterMatches(); }
 string CF_TestTripReturnReadback()
 {
  string result = "merged=" + m_bReturnMerged + ",pending=" + m_bReturnPending + ",tracking=" + m_bTripReturnTracking + ",owner_original_pilot=" + (GetOwnerPilotedVehicle() == m_OriginalLeadVehicle) + ",state=" + m_sTripState;
  foreach (CF_ConvoyTripMember member : m_aTripMembers) result += ",member_progress=" + member.ReturnProgress + "/" + member.ForwardSamples + "/" + member.PoweredSamples;
  return result;
 }
}

modded class CF_ConvoySession
{
 int CF_TestReturnIdentity(CF_DriverControllerComponent driver) { return GetIdentityNumber(driver); }
}
modded class CF_EntityFollowDriverControllerComponent
{
 IEntity CF_TestReturnPredecessor() { return GetTargetVehicle(false); }
}

modded class CF_ConvoySession
{
 bool CF_TestOriginalReturnParked(CF_DriverControllerComponent driver)
 {
  if (!driver || !m_bOriginalStagingValid || !m_OriginalLeadVehicle || !m_aUnits.IsEmpty() ||
   m_aReturnQueue.Count() != 1 || m_aReturnQueue[0] != driver || m_bReturnMerged || m_bReturnPending ||
   m_UnloadHead || m_iUnloadPhase != CF_UNLOAD_NONE || !driver.CF_IsAtUnloadWaitingPoint() || !driver.CF_IsUnloadDeparted()) return false;
  foreach (CF_ConvoyTripMember member : m_aTripMembers)
   if (member.Driver == driver && member.DriverEntity == driver.CF_GetDriverEntity() && member.Truck == driver.CF_GetAssignedVehicle()) return true;
  return false;
 }
}
