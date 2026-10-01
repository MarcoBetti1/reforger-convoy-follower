// Convoy transport drivers keep their seats while their original assignment
// is healthy. Destroyed trucks, incapacitated drivers, possession and explicit
// dismissal retain native safety/seat behavior. No immunity or damage changes.
modded class SCR_AIUtilityComponent
{
	override void WrapBehaviorOutsideOfVehicle(SCR_AIActionBase action)
	{
		CF_DriverControllerComponent driver;
		if (m_OwnerEntity) driver = CF_DriverControllerComponent.Cast(m_OwnerEntity.FindComponent(CF_DriverControllerComponent));
		if (driver && driver.CF_ShouldRemainSeated()) return;
		super.WrapBehaviorOutsideOfVehicle(action);
	}
}

modded class SCR_AIVehicleCombatActivity
{
	override float CustomEvaluate()
	{
		if (m_Utility && m_Utility.m_Owner && m_Utility.m_Owner.GetAgentsCount() == 1)
		{
			array<AIAgent> agents = {};
			m_Utility.m_Owner.GetAgents(agents);
			if (agents.Count() == 1 && agents[0].GetControlledEntity())
			{
				CF_DriverControllerComponent driver = CF_DriverControllerComponent.Cast(agents[0].GetControlledEntity().FindComponent(CF_DriverControllerComponent));
				if (driver && driver.CF_ShouldRemainSeated()) return 0;
			}
		}
		return super.CustomEvaluate();
	}
}
