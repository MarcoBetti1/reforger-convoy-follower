// Alternate cargo-bay order for a stopped convoy. The server checks the
// actual occupied front truck and uses the same road planner as the map panel.
class CF_PullAheadAtUnloadAction : CF_ReleaseAtUnloadAction
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Pull ahead and wait";
		return true;
	}

	override bool CanBePerformedScript(IEntity user)
	{
		CF_DriverControllerComponent driver = GetConvoyDriver();
		if (!driver || !driver.CF_CanReleaseAtUnload())
		{
			SetCannotPerformReason("Wait for the front truck to reach the stopping point");
			return false;
		}
		if (Replication.IsServer() && !CF_ConvoySession.CanReleaseAtUnload(user, driver))
		{
			SetCannotPerformReason("Only the convoy owner can release the front truck");
			return false;
		}
		if (Replication.IsServer() && !CF_ConvoySession.CanPlanForwardWaitAtUnload(user, driver))
		{
			SetCannotPerformReason(CF_ConvoySession.GetReleasePlanFailureReason(user));
			return false;
		}
		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		CF_DriverControllerComponent driver = GetConvoyDriver();
		if (driver)
			CF_ConvoySession.ReleaseForwardAtUnload(pUserEntity, driver);
	}
}
