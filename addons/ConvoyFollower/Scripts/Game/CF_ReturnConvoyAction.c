// Manual alternative to the owner driving past the parked return line.
// It appears in that truck's existing rear cargo interaction menu.
class CF_ReturnConvoyAction : CF_ReleaseAtUnloadAction
{
	override bool GetActionNameScript(out string outName)
	{
		CF_DriverControllerComponent driver = GetConvoyDriver();
		if (driver && driver.CF_IsReturnBlockedForActions())
			outName = "Retry return turn";
		else
			outName = "Regroup convoy for return";
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		CF_DriverControllerComponent driver = GetConvoyDriver();
		return driver && driver.CF_IsOwnedBy(user) &&
			(driver.CF_IsReturnParkedForActions() || driver.CF_IsReturnBlockedForActions());
	}

	override bool CanBePerformedScript(IEntity user)
	{
		CF_DriverControllerComponent driver = GetConvoyDriver();
		if (!driver)
			return false;
		if (driver.CF_IsReturnBlockedForActions())
		{
			if (Replication.IsServer() && !CF_ConvoySession.CanRetryBlockedReturn(user, driver))
			{
				SetCannotPerformReason("Return truck must be seated and still owned by your convoy");
				return false;
			}
			return true;
		}
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
		{
			if (driver.CF_IsReturnBlockedForActions())
				CF_ConvoySession.RetryBlockedReturn(pUserEntity, driver);
			else
				CF_ConvoySession.StartReturnFromAction(pUserEntity, driver);
		}
	}
}
