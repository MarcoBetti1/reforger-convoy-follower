class CF_RecordedTrailLoadedProbeComponentClass : CF_EveronLoadedTripProbeComponentClass {}
// Bounded backend comparison; original100loading and outbound/180s arrival gates.
// No command/delivery/return completeness credit, even if paced result passes.
class CF_RecordedTrailLoadedProbeComponent : CF_EveronLoadedTripProbeComponent
{
 protected float m_fPrivateNextStateMs;
 protected bool m_bPrivateGeometryRead;
 override void EOnPostFrame(IEntity owner, float timeSlice)
 {
  super.EOnPostFrame(owner, timeSlice);
  if (!Replication.IsServer() || m_bPacedTerminal || !PacedWorldAlive()) return;
  World world = GetGame().GetWorld();
  float now = world.GetWorldTime();
  if (now >= m_fPrivateNextStateMs)
  {
   m_fPrivateNextStateMs = now + 5000;
   Print("[ConvoyFollower] SHARED_WORLD_STATE: world_ms=" + now + " stage=" + m_iStage + " drive=" + m_bPacedStarted +
    " world_time_scale=" + world.GetTimeScale() + " edit_mode=" + world.IsEditMode() +
    " editor_open=" + SCR_EditorManagerEntity.IsOpenedInstance() + " time_slice=" + world.GetTimeSlice() +
    " fixed_slice=" + world.GetFixedTimeSlice() + " physics_slice=" + world.GetPhysicsTimeSlice() + " observed_postframe_slice=" + timeSlice + " timing_writes=false");
   for (int unit = 0; unit < 2; unit++)
   {
    string groupName = "CF_SmokePilotGroup"; if (unit == 1) groupName = "CF_SmokeGroup1";
    SCR_AIGroup group = SCR_AIGroup.Cast(world.FindEntityByName(groupName));
    if (!group) continue;
    array<AIAgent> groupAgents = {}; group.GetAgents(groupAgents);
    AIAgent groupLeader = group.GetLeaderAgent();
    Print("[ConvoyFollower] SHARED_GROUP_MEMBERS: unit=" + unit + " count=" + groupAgents.Count() + " group_leader_agent=" + EntityKey(groupLeader) + " group_leader_entity=" + EntityKey(group.GetLeaderEntity()) + " membership_writes=false");
    foreach (AIAgent member : groupAgents)
    {
     IEntity controlled = member.GetControlledEntity();
     int playerId = -1; if (controlled && GetGame().GetPlayerManager()) playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(controlled);
     Print("[ConvoyFollower] SHARED_GROUP_MEMBER: unit=" + unit + " agent_id=" + EntityKey(member) + " controlled_id=" + EntityKey(controlled) + " is_group_leader=" + (member == groupLeader) + " player_id=" + playerId + " membership_writes=false");
    }
    AIGroupMovementComponent movement = AIGroupMovementComponent.Cast(group.FindComponent(AIGroupMovementComponent));
    if (movement)
     foreach (AIAgent member : groupAgents)
     {
      int handler = movement.GetAgentMoveHandlerId(member); array<AIAgent> handlerAgents = {}; movement.GetAgentsInHandler(handlerAgents, handler);
      Print("[ConvoyFollower] SHARED_FORMATION_ROLE: unit=" + unit + " agent_id=" + EntityKey(member) + " handler=" + handler + " handler_count=" + handlerAgents.Count() + " handler_leader_agent=" + EntityKey(movement.GetHandlerLeaderAgent(handler)) + " role_writes=false");
     }
    SCR_AIGroupUtilityComponent utility = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));
    AIActionBase action; if (utility) action = utility.GetCurrentAction();
    SCR_AIMoveActivity move = SCR_AIMoveActivity.Cast(action);
    string wanted = "unavailable", actualMoveType = "not_MOVE";
    if (movement) wanted = typename.EnumToString(EMovementType, movement.GetGroupCharactersMovementTypeWanted());
    if (move) actualMoveType = typename.EnumToString(EMovementType, move.m_eMovementType.m_Value);
    Print("[ConvoyFollower] SHARED_GROUP_STATE: unit=" + unit + " group_id=" + EntityKey(group) +
     " current_action=" + action + " wanted_character_movement=" + wanted + " move_activity_movement=" + actualMoveType + " movement_writes=false character_wanted_not_vehicle_cap=true");
    Vehicle truck = m_Lead; if (unit == 1) truck = Vehicle.Cast(world.FindEntityByName("CF_SmokeFollower1"));
    AIWaypoint waypoint = group.GetCurrentWaypoint();
    ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
    if (truck && waypoint && aiWorld && aiWorld.GetRoadNetworkManager())
    {
     RoadNetworkManager roads = aiWorld.GetRoadNetworkManager(); BaseRoad nearest; float offset;
     int roadId = roads.GetClosestRoad(waypoint.GetOrigin(), nearest, offset); vector projected;
     bool reachable = roads.GetReachableWaypointInRoad(truck.GetOrigin(), waypoint.GetOrigin(), 5.0, projected);
     Print("[ConvoyFollower] SHARED_GOAL_GEOMETRY: unit=" + unit + " waypoint_id=" + EntityKey(waypoint) +
      " goal=" + waypoint.GetOrigin() + " truck=" + truck.GetOrigin() + " gap_xz=" + vector.DistanceXZ(truck.GetOrigin(), waypoint.GetOrigin()) +
      " radius=" + waypoint.GetCompletionRadius() + " closest_road=" + roadId + " road_offset=" + offset +
      " reachable_within_5=" + reachable + " projected=" + projected + " projected_offset=" + vector.DistanceXZ(projected, waypoint.GetOrigin()) + " goal_writes=false");
    }
   }
   if (!m_bPrivateGeometryRead)
   {
    ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
    if (aiWorld && aiWorld.GetRoadNetworkManager())
    {
     m_bPrivateGeometryRead = true; RoadNetworkManager roads = aiWorld.GetRoadNetworkManager();
     array<vector> retainedGoals = {"7225.66 138.606 2438.29", "7209.07 138.614 2439.76", "7192.96 138.776 2441.48", "7183.06 138.881 2444.13", "7181.33 139.611 2457.82", "7180.53 140.153 2468.41", "7179.55 140.667 2479.09"};
     foreach (int index, vector goal : retainedGoals)
     {
      BaseRoad nearest; float offset; int roadId = roads.GetClosestRoad(goal, nearest, offset); vector projected;
      bool reachable = roads.GetReachableWaypointInRoad("7253 138.9 2438", goal, 5.0, projected);
      Print("[ConvoyFollower] RETAINED_V2_GOAL_ROAD_READ: index=" + index + " goal=" + goal + " closest_road=" + roadId +
       " road_offset=" + offset + " reachable_within_5=" + reachable + " projected=" + projected +
       " projected_offset=" + vector.DistanceXZ(projected, goal) + " source=retained_V2_waypoint_positions_same_world terrain_read_only=true path_order_claim=false");
     }
    }
   }
  }
  if (m_bPacedStarted && now - m_fPacedDriveStartMs >= 60000)
  {
   Print("[ConvoyFollower] SHARED_STATE_BOUNDARY: drive_world_s=" + (now - m_fPacedDriveStartMs) / 1000.0 + " scope=read_only_state_qualification full_route=false full_loop=false");
   NoteFailure("fixture_scope", "read_only_shared_state_60worlds_not_full_route"); EndPaced();
  }
 }
 override protected void EndPaced()
 {
  if (m_bPacedTerminal) return;
  Print("[ConvoyFollower] RECORDED_MOVE_BOUNDARY: run_id=" + m_sPacedRun + " scope=loaded_outbound_and_180s_owned_arrival_only command_extension=false unload_claim=false full_loop_claim=false original_thresholds=true");
  m_iCommandStage = 99; // Skip existing command extension; never fake completion.
  super.EndPaced();
 }
}
