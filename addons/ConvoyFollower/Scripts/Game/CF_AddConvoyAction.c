// Add an idle driver to the end of the ordering player's existing convoy.
class CF_AddConvoyAction : ScriptedUserAction
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Add to convoy in closest empty vehicle";
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		CF_DriverControllerComponent driver = CF_DriverControllerComponent.Cast(GetOwner().FindComponent(CF_DriverControllerComponent));
		return driver && (driver.IsIdle() || (driver.IsOnFootFollower() && driver.CF_IsOwnedBy(user))) && CF_ConvoySession.HasActiveConvoyForActions(user);
	}

	override bool CanBePerformedScript(IEntity user)
	{
		CF_DriverControllerComponent driver = CF_DriverControllerComponent.Cast(GetOwner().FindComponent(CF_DriverControllerComponent));
		if (!driver || !driver.CanAssign(user))
		{
			SetCannotPerformReason("Place an empty wheeled vehicle within 35 m of the driver");
			return false;
		}
		if (Replication.IsServer() && !CF_ConvoySession.CanAdd(user, driver))
		{
			SetCannotPerformReason("Start a convoy first or wait for the current driver to board");
			return false;
		}
		return true;
	}

	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	override bool CheckOnServerFirstScript()
	{
		return true;
	}

	override bool CanBroadcastScript()
	{
		return false;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		CF_DriverControllerComponent driver = CF_DriverControllerComponent.Cast(pOwnerEntity.FindComponent(CF_DriverControllerComponent));
		if (driver)
			CF_ConvoySession.Add(pUserEntity, driver);
	}
}
