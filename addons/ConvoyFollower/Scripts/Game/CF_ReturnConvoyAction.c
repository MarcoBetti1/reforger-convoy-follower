// Manual alternative to the owner driving past the parked return line.
// It appears in that truck's existing rear cargo interaction menu.
class CF_ReturnConvoyAction : CF_ReleaseAtUnloadAction
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Regroup convoy for return";
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		CF_DriverControllerComponent driver = GetConvoyDriver();
		return driver && driver.CF_IsReturnParkedForActions() && driver.CF_IsOwnedBy(user);
	}

	override bool CanBePerformedScript(IEntity user)
	{
		CF_DriverControllerComponent driver = GetConvoyDriver();
		if (!driver)
			return false;
		if (Replication.IsServer() && !CF_ConvoySession.CanStartReturnFromAction(user, driver))
		{
			SetCannotPerformReason("Wait until the truck parks in the return line");
			return false;
		}
		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		CF_DriverControllerComponent driver = GetConvoyDriver();
		if (driver)
			CF_ConvoySession.StartReturnFromAction(pUserEntity, driver);
	}
}
