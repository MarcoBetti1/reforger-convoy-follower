// Test-only companion for OpenRoad ForwardWait. The smoke probe establishes
// a real two-truck road arrival. This companion stages only the parked test
// lead on a clear shoulder, then exercises the production forward wait order.
class CF_ForwardWaitProbeComponentClass : ScriptComponentClass
{
}

class CF_ForwardWaitProbeComponent : ScriptComponent
{
	protected Vehicle m_Lead;
	protected Vehicle m_Truck1;
	protected Vehicle m_Truck2;
	protected ChimeraCharacter m_Player;
	protected CF_DriverControllerComponent m_Driver1;
	protected CF_DriverControllerComponent m_Driver2;
	protected CF_SmokeProbeComponent m_RouteProbe;
	protected vector m_vUnloadBay;
	protected vector m_vUnit2Start;
	protected vector m_vLeadShoulder;
	protected vector m_vForward;
	protected bool m_bShoulderOccupied;
	protected bool m_bFinished;
	protected int m_iStage;
	protected int m_iStageTicks;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Lead = Vehicle.Cast(owner);
		Print("[ConvoyFollower] AUTO_FORWARD_INIT: waiting for two-truck road arrival");
		GetGame().GetCallqueue().CallLater(Poll, 1000, true);
	}

	protected void Finish(string result)
	{
		if (m_bFinished)
			return;
		m_bFinished = true;
		Print("[ConvoyFollower] AUTO_FORWARD_RESULT: " + result);
		GetGame().GetCallqueue().Remove(Poll);
	}

	protected CF_DriverControllerComponent FindDriver(int unit)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokeGroup" + unit));
		if (!group)
			return null;
		ref array<AIAgent> agents = {};
		group.GetAgents(agents);
		if (agents.Count() != 1 || !agents[0])
			return null;
		IEntity character = agents[0].GetControlledEntity();
		if (!character)
			return null;
		return CF_DriverControllerComponent.Cast(character.FindComponent(CF_DriverControllerComponent));
	}

	protected bool Resolve()
	{
		if (!m_Lead)
			return false;
		BaseWorld world = GetGame().GetWorld();
		if (!m_Truck1)
			m_Truck1 = Vehicle.Cast(world.FindEntityByName("CF_SmokeFollower1"));
		if (!m_Truck2)
			m_Truck2 = Vehicle.Cast(world.FindEntityByName("CF_SmokeFollower2"));
		if (!m_Driver1)
			m_Driver1 = FindDriver(1);
		if (!m_Driver2)
			m_Driver2 = FindDriver(2);
		if (!m_RouteProbe)
			m_RouteProbe = CF_SmokeProbeComponent.Cast(m_Lead.FindComponent(CF_SmokeProbeComponent));
		if (!m_Player)
		{
			PlayerManager players = GetGame().GetPlayerManager();
			ref array<int> ids = {};
			if (players)
				players.GetPlayers(ids);
			if (!ids.IsEmpty())
				m_Player = ChimeraCharacter.Cast(players.GetPlayerControlledEntity(ids[0]));
		}
		return m_Truck1 && m_Truck2 && m_Driver1 && m_Driver2 && m_RouteProbe && m_Player;
	}

	protected bool FilterShoulderTrace(IEntity entity, vector start, vector direction)
	{
		return entity != m_Lead && entity != m_Player;
	}

	protected bool ConsiderShoulderOccupant(IEntity entity)
	{
		if (entity != m_Lead && Vehicle.Cast(entity))
			m_bShoulderOccupied = true;
		return true;
	}

	protected bool TryShoulderSide(float sideSign, float distance, out vector candidate, out float roadDistance)
	{
		candidate = vector.Zero;
		roadDistance = 0;
		BaseWorld world = GetGame().GetWorld();
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!world || !aiWorld || !aiWorld.GetRoadNetworkManager())
			return false;
		vector current = m_Lead.GetOrigin();
		float sideX = -m_vForward[2] * sideSign;
		float sideZ = m_vForward[0] * sideSign;
		candidate = current;
		candidate[0] = candidate[0] + sideX * distance;
		candidate[2] = candidate[2] + sideZ * distance;
		float originalSurface = world.GetSurfaceY(current[0], current[2]);
		float nextSurface = world.GetSurfaceY(candidate[0], candidate[2]);
		float surfaceRise = nextSurface - originalSurface;
		if (surfaceRise > 2.5 || surfaceRise < -2.5)
			return false;
		candidate[1] = nextSurface + (current[1] - originalSurface);
		if (vector.Distance(candidate, m_Truck1.GetOrigin()) < 9.0 ||
			vector.Distance(candidate, m_Truck2.GetOrigin()) < 12.0)
			return false;
		BaseRoad currentRoad;
		float currentRoadDistance;
		aiWorld.GetRoadNetworkManager().GetClosestRoad(current, currentRoad, currentRoadDistance);
		BaseRoad shoulderRoad;
		aiWorld.GetRoadNetworkManager().GetClosestRoad(candidate, shoulderRoad, roadDistance);
		if (!shoulderRoad || roadDistance < 9.0 ||
			roadDistance < currentRoadDistance + 3.0 || roadDistance > 20.0)
			return false;
		TraceParam trace = new TraceParam();
		trace.Start = current + Vector(0, 1.5, 0);
		trace.End = candidate + Vector(0, 1.5, 0);
		trace.Flags = TraceFlags.ENTS;
		trace.Exclude = m_Lead;
		if (world.TraceMove(trace, FilterShoulderTrace) < 0.98)
			return false;
		m_bShoulderOccupied = false;
		world.QueryEntitiesBySphere(candidate, 4.5, ConsiderShoulderOccupant);
		return !m_bShoulderOccupied;
	}

	protected bool StageParkedLeadOnShoulder()
	{
		vector forward = m_Lead.GetWorldTransformAxis(2);
		float length = Math.Sqrt(forward[0] * forward[0] + forward[2] * forward[2]);
		if (length < 0.5)
			return false;
		m_vForward = Vector(forward[0] / length, 0, forward[2] / length);
		vector original = m_Lead.GetOrigin();
		ChimeraAIWorld aiWorld = ChimeraAIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld || !aiWorld.GetRoadNetworkManager())
			return false;
		vector desiredAhead = original + m_vForward * 85.0;
		vector connectedAhead;
		if (!aiWorld.GetRoadNetworkManager().GetReachableWaypointInRoad(
			m_Truck1.GetOrigin(), desiredAhead, 25.0, connectedAhead) ||
			vector.Distance(connectedAhead, original) < 75.0)
		{
			Print("[ConvoyFollower] AUTO_FORWARD_FIXTURE_ROAD_REJECTED: no connected road at least 75 m ahead of " + original);
			return false;
		}
		Print("[ConvoyFollower] AUTO_FORWARD_FIXTURE_ROAD: connected ahead=" + connectedAhead +
			" gap=" + vector.Distance(connectedAhead, original));
		vector candidate;
		float roadDistance;
		for (int sideIndex = 0; sideIndex < 2; sideIndex++)
		{
			float side = 1.0;
			if (sideIndex == 1)
				side = -1.0;
			for (int distanceIndex = 0; distanceIndex < 2; distanceIndex++)
			{
				float offset = 10.0 + distanceIndex * 2.0;
				if (!TryShoulderSide(side, offset, candidate, roadDistance))
					continue;
				m_Lead.SetOrigin(candidate);
				m_vLeadShoulder = candidate;
				Print("[ConvoyFollower] AUTO_FORWARD_FIXTURE_SHOULDER: test lead only " + original +
					" -> " + candidate + " offset=" + offset + " side=" + side +
					" road_dist=" + roadDistance);
				return true;
			}
		}
		return false;
	}

	protected void Poll()
	{
		if (m_bFinished)
			return;
		m_iStageTicks++;
		if (!Resolve())
		{
			if (m_iStageTicks >= 45)
				Finish("FAIL test owner, trucks, drivers, or route probe missing");
			return;
		}
		if (m_iStage == 0)
		{
			if (!m_RouteProbe.CF_HasPassedRoadArrival())
			{
				if (m_iStageTicks >= 250)
					Finish("FAIL two-truck road arrival did not pass");
				return;
			}
			if (!m_Driver1.CF_IsBoarded() || !m_Driver2.CF_IsBoarded())
			{
				Finish("FAIL follower driver not seated at road arrival");
				return;
			}
			if (!StageParkedLeadOnShoulder())
			{
				Finish("FAIL fixture could not find clear level shoulder for test lead");
				return;
			}
			m_iStage = 1;
			m_iStageTicks = 0;
			return;
		}
		if (m_iStage == 1)
		{
			if (m_iStageTicks < 3)
				return;
			if (vector.Distance(m_Lead.GetOrigin(), m_vLeadShoulder) > 2.0)
			{
				Finish("FAIL staged lead did not remain parked on shoulder");
				return;
			}
			// The lead's shoulder movement temporarily invalidates the stopped
			// target. Wait for the normal six-second arrival/release-ready window
			// to settle before issuing the real player order.
			bool ready = CF_ConvoySession.CanReleaseAtUnload(m_Player, m_Driver1);
			if (!ready)
			{
				if (m_iStageTicks % 5 == 0)
					Print("[ConvoyFollower] AUTO_FORWARD_WAIT_READY: elapsed=" + m_iStageTicks +
						" boarded=" + m_Driver1.CF_IsBoarded() +
						" release_allowed=" + m_Driver1.CF_IsSessionReleaseAllowed() +
						" gap_to_lead=" + vector.Distance(m_Truck1.GetOrigin(), m_Lead.GetOrigin()));
				if (m_iStageTicks >= 25)
					Finish("FAIL front truck did not regain release-ready state after lead shoulder stop");
				return;
			}
			m_vUnloadBay = m_Truck1.GetOrigin();
			m_vUnit2Start = m_Truck2.GetOrigin();
			Print("[ConvoyFollower] AUTO_FORWARD_WAIT_READY: elapsed=" + m_iStageTicks +
				" boarded=" + m_Driver1.CF_IsBoarded() + " release_allowed=" +
				m_Driver1.CF_IsSessionReleaseAllowed() + " ready=true");
			bool accepted = CF_ConvoySession.CF_PanelPullAhead(m_Player, 1);
			Print("[ConvoyFollower] AUTO_FORWARD_ORDER: accepted=" + accepted + " bay=" + m_vUnloadBay);
			if (!accepted)
				Finish("FAIL forward release preflight or order rejected at shoulder-parked arrival");
			else
			{
				m_iStage = 2;
				m_iStageTicks = 0;
			}
			return;
		}
		if (m_iStage == 2)
		{
			float bayClearance = vector.Distance(m_Truck1.GetOrigin(), m_vUnloadBay);
			vector delta = m_Truck1.GetOrigin() - m_vUnloadBay;
			float forwardAdvance = delta[0] * m_vForward[0] + delta[2] * m_vForward[2];
			float nextAdvance = vector.Distance(m_Truck2.GetOrigin(), m_vUnit2Start);
			float nextBayGap = vector.Distance(m_Truck2.GetOrigin(), m_vUnloadBay);
			bool parked = m_Driver1.CF_IsForwardWaitParked();
			if (m_iStageTicks % 5 == 0)
				Print("[ConvoyFollower] AUTO_FORWARD_STATUS: parked=" + parked +
					" bay_clear=" + bayClearance + " forward_advance=" + forwardAdvance +
					" unit2_advance=" + nextAdvance + " unit2_bay_gap=" + nextBayGap +
					" seated=" + m_Driver1.CF_IsBoarded() + "," + m_Driver2.CF_IsBoarded());
			if (parked && m_Driver1.CF_IsBoarded() && m_Driver2.CF_IsBoarded() &&
				bayClearance >= 15.0 && forwardAdvance >= 15.0 &&
				nextAdvance >= 3.0 && nextBayGap <= 8.0)
				Finish("PASS Unit 1 physically passed shoulder-parked lead, cleared bay and parked; Unit 2 reached bay");
			else if (m_iStageTicks >= 150)
				Finish("FAIL forward truck did not park or successor did not reach bay; parked=" + parked +
					" clear=" + bayClearance + " forward=" + forwardAdvance +
					" unit2_advance=" + nextAdvance + " unit2_bay_gap=" + nextBayGap);
		}
	}
}
