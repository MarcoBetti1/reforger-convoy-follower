// Recovery control on the same rear cargo menu as Pull off and regroup.
// It cancels a held unload attempt without dismissing any convoy driver.
class CF_ResumeConvoyAction : CF_ReleaseAtUnloadAction
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Resume after pull-off";
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
			if (driver.CF_HasPanelHoldRequest() || driver.CF_IsPanelHeld())
				SetCannotPerformReason("Return to your lead truck, open the map and choose RESUME ALL");
			else
				SetCannotPerformReason("No pull-off hold to cancel, or a truck is still moving out");
			return false;
		}
		return true;
	}

	override void OnRejected(IEntity pUserEntity)
	{
		if (System.IsConsoleApp() || !GetGame() || !GetGame().GetPlayerController())
			return;
		SCR_HintManagerComponent.ShowCustomHint(
			"After HOLD ALL SEATED, return to your lead truck and use the map's RESUME ALL. Pull-off recovery requires the front truck to stop.",
			"Convoy", 7.0, true);
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		CF_DriverControllerComponent driver = GetConvoyDriver();
		if (driver)
			CF_ConvoySession.ResumeFollowing(pUserEntity, driver);
	}
}
