// Private read-only visual observer over the frozen V12 course.
// No new camera, actor, route, resource, vehicle-control or order writes.
modded class CF_DriverControllerComponent
{
 string CF_ParkingVisualReadback()
 {
  string readback = "state=" + m_iState + " planned_slot=" + m_vUnloadWaitingPoint;
  readback += " turn_phase=" + m_iUnloadTurnPhase + " turn_stage=" + m_vUnloadTurnStage;
  readback += " unload_anchor=" + m_vUnloadAnchor + " home_direction=" + m_vUnloadHomeDirection;
  readback += " outbound_direction=" + m_vOutboundTravelDirection;
  readback += " intermediate_active=" + m_bUnloadIntermediateActive + " intermediate_goal=" + m_vUnloadIntermediateGoal;
  readback += " slot_radius=" + CF_UNLOAD_SLOT_RADIUS + " waypoint=" + m_Waypoint;
  return readback;
 }
}

modded class CF_ExplicitBayFeatureProbeComponent
{
 protected bool m_bParkingVisualBefore;
 protected bool m_bParkingVisualTerminal;
 protected int m_iParkingVisualTimed;
 protected float m_fParkingVisualSampleMs = -1;
 protected string CF_VisualPose(IEntity entity)
 {
  if (!EntityAlive(entity)) return "valid=false";
  return "valid=true id=" + EntityKey(entity) + " origin=" + entity.GetOrigin() +
   " forward=" + entity.GetWorldTransformAxis(2);
 }
 protected void CF_ParkingVisualGeometry(string tag)
 {
  if (!PacedWorldAlive() || m_PacedTrucks.Count() != 2) return;
  CF_EntityFollowDriverControllerComponent driver = CommandDriver();
  string planned; if (driver) planned = driver.CF_ParkingVisualReadback();
  string line = "[ConvoyFollower] PARKING_VISUAL_GEOMETRY: tag=" + tag + " phase=" + m_iBayPhase;
  line += " world_ms=" + GetGame().GetWorld().GetWorldTime() + " phase_elapsed_world_ms=" + (GetGame().GetWorld().GetWorldTime() - m_fBayPhaseMs);
  line += " original_identity=" + BayIdentities() + " owner='" + CF_VisualPose(m_PacedOwner);
  line += "' tail_driver='" + CF_VisualPose(m_PacedTrucks[1].Pilot) + "' lead_driver='" + CF_VisualPose(m_Pilot);
  line += "' tail='" + CF_VisualPose(m_PacedTrucks[1].Truck) + "' lead='" + CF_VisualPose(m_Lead);
  line += "' bay=" + m_vBay + " controller='" + planned + "' camera_unchanged=false observer_control_writes=false";
  Print(line);
 }
 protected void CF_ParkingVisualCapture(string tag)
 {
  if (!PacedWorldAlive()) return;
  CF_ParkingVisualGeometry(tag);
  bool writable = System.MakeScreenshot("$logs:parking-" + tag);
  Print("[ConvoyFollower] PARKING_VISUAL_CAPTURE: tag=" + tag + " writable=" + writable +
   " phase=" + m_iBayPhase + " world_ms=" + GetGame().GetWorld().GetWorldTime() +
   " camera_unchanged=false new_control_writes=false pixel_validation_pending=true visual_parking_pass=false input_claim=false video_claim=false");
 }
 override protected void BayPhase(int phase)
 {
  super.BayPhase(phase);
  if (phase == 12) CF_ParkingVisualCapture("started");
  if (phase == 13) CF_ParkingVisualCapture("arrived");
  if (phase == 30) CF_ParkingVisualCapture("stable-boundary");
 }
 override protected void BayEnd(bool pass, string reason)
 {
  if (!m_bPacedTerminal && !m_bParkingVisualTerminal)
  {
   m_bParkingVisualTerminal = true;
   CF_ParkingVisualCapture("terminal");
  }
  super.BayEnd(pass, reason);
 }
 override void EOnPostFrame(IEntity owner, float timeSlice)
 {
  super.EOnPostFrame(owner, timeSlice);
  if (m_bPacedTerminal || !PacedWorldAlive()) return;
  float now = GetGame().GetWorld().GetWorldTime();
  float elapsed = now - m_fBayPhaseMs;
  if (!m_bParkingVisualBefore && m_iBayPhase == 10 && elapsed >= 1500)
  { m_bParkingVisualBefore = true; CF_ParkingVisualCapture("before"); }
  if ((m_iBayPhase == 12 || m_iBayPhase == 13) && (m_fParkingVisualSampleMs < 0 || now - m_fParkingVisualSampleMs >= 1000))
  { m_fParkingVisualSampleMs = now; CF_ParkingVisualGeometry("sample"); }
  if (m_iBayPhase != 12) return;
  int limit;
  if (m_iParkingVisualTimed == 0) limit = 10000;
  else if (m_iParkingVisualTimed == 1) limit = 30000;
  else if (m_iParkingVisualTimed == 2) limit = 60000;
  else if (m_iParkingVisualTimed == 3) limit = 85000;
  else return;
  if (elapsed >= limit)
  { CF_ParkingVisualCapture("during-" + m_iParkingVisualTimed); m_iParkingVisualTimed++; }
 }
}
