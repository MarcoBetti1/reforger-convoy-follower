// Test-only negative case. The lead stays in the road lane; the real forward
// order must reject that obstruction and leave the first truck undisturbed.
class CF_ForwardBlockedProbeComponentClass : ScriptComponentClass
{
}

class CF_ForwardBlockedProbeComponent : ScriptComponent
{
	protected Vehicle m_Lead;
	protected Vehicle m_Truck1;
	protected ChimeraCharacter m_Player;
	protected CF_DriverControllerComponent m_Driver1;
	protected CF_DriverControllerComponent m_Driver2;
	protected CF_SmokeProbeComponent m_RouteProbe;
	protected vector m_vTruckAtOrder;
	protected int m_iTicks;
	protected int m_iHoldTicks;
	protected bool m_bOrderChecked;
	protected bool m_bFinished;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Lead = Vehicle.Cast(owner);
		Print("[ConvoyFollower] AUTO_FORWARD_BLOCKED_INIT: waiting for road arrival with lead in lane");
		GetGame().GetCallqueue().CallLater(Poll, 1000, true);
	}

	protected void Finish(string result)
	{
		if (m_bFinished)
			return;
		m_bFinished = true;
		Print("[ConvoyFollower] AUTO_FORWARD_BLOCKED_RESULT: " + result);
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
		return m_Truck1 && m_Driver1 && m_Driver2 && m_RouteProbe && m_Player;
	}

	protected void Poll()
	{
		if (m_bFinished)
			return;
		m_iTicks++;
		if (!Resolve())
		{
			if (m_iTicks >= 45)
				Finish("FAIL fixture owner, trucks, drivers, or route probe missing");
			return;
		}
		if (!m_bOrderChecked)
		{
			if (!m_RouteProbe.CF_HasPassedRoadArrival())
			{
				if (m_iTicks >= 250)
					Finish("FAIL two-truck road arrival did not pass");
				return;
			}
			if (!CF_ConvoySession.CanReleaseAtUnload(m_Player, m_Driver1))
			{
				if (m_iTicks >= 275)
					Finish("FAIL front driver did not reach stopped release-ready state");
				return;
			}
			if (!m_Driver1.CF_IsBoarded() || !m_Driver2.CF_IsBoarded())
			{
				Finish("FAIL front or rear driver was not seated");
				return;
			}
			m_vTruckAtOrder = m_Truck1.GetOrigin();
			bool accepted = CF_ConvoySession.CF_PanelPullAhead(m_Player, 1);
			string reason = CF_ConvoySession.GetReleasePlanFailureReason(m_Player);
			Print("[ConvoyFollower] AUTO_FORWARD_BLOCKED_ORDER: accepted=" + accepted +
				" reason=" + reason + " truck=" + m_vTruckAtOrder + " lead=" + m_Lead.GetOrigin());
			if (accepted || reason != "Move your lead vehicle off the driving lane to let this truck pass")
			{
				Finish("FAIL blocked-center-lane order was accepted or gave wrong reason: " + reason);
				return;
			}
			m_bOrderChecked = true;
			return;
		}
		m_iHoldTicks++;
		float movement = vector.Distance(m_Truck1.GetOrigin(), m_vTruckAtOrder);
		if (!m_Driver1.CF_IsActiveConvoyMember() || !m_Driver1.CF_IsBoarded() ||
			!m_Driver2.CF_IsBoarded() || m_Driver1.CF_IsForwardWaitParked() || movement > 2.5)
		{
			Finish("FAIL blocked order changed driver state or moved Unit 1 by " + movement + " m");
			return;
		}
		if (m_iHoldTicks >= 5)
			Finish("PASS center-lane lead rejected with specific obstruction reason; Unit 1 stayed seated and moved " + movement + " m");
	}
}
