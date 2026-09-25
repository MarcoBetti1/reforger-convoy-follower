// Recovery control on the same rear cargo menu as Pull off and regroup.
// It cancels a held unload attempt without dismissing any convoy driver.
class CF_ResumeConvoyAction : CF_ReleaseAtUnloadAction
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Resume convoy following";
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		CF_DriverControllerComponent driver = GetConvoyDriver();
		return driver && driver.CF_GetUnitNumber() == 1 && driver.CF_IsOwnedBy(user);
	}

	override bool CanBePerformedScript(IEntity user)
	{
		CF_DriverControllerComponent driver = GetConvoyDriver();
		if (!driver)
			return false;
		if (Replication.IsServer() && !CF_ConvoySession.CanResumeFollowing(user, driver))
		{
			SetCannotPerformReason("No unload hold to cancel, or a truck is still moving out");
			return false;
		}
		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		CF_DriverControllerComponent driver = GetConvoyDriver();
		if (driver)
			CF_ConvoySession.ResumeFollowing(pUserEntity, driver);
	}
}
