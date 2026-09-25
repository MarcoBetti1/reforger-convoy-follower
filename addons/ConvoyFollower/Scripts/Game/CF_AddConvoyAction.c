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
		if (!driver || !(driver.IsIdle() || (driver.IsOnFootFollower() && driver.CF_IsOwnedBy(user))) ||
			!CF_ConvoySession.HasActiveConvoyForActions(user))
			return false;

		PlayerManager manager = GetGame().GetPlayerManager();
		if (!manager)
			return false;
		int playerId = manager.GetPlayerIdFromControlledEntity(user);
		SCR_PlayerController controller = SCR_PlayerController.Cast(manager.GetPlayerController(playerId));
		return controller && !controller.CF_IsAtHardConvoyLimit();
	}

	override bool CanBePerformedScript(IEntity user)
	{
		CF_DriverControllerComponent driver = CF_DriverControllerComponent.Cast(GetOwner().FindComponent(CF_DriverControllerComponent));
		if (!driver)
			return false;
		// Client settings do not include the dedicated server's radius override.
		// The server-first action path validates the actual vehicle and roster.
		if (!Replication.IsServer())
			return true;
		if (!driver.CanAssign(user))
		{
			SetCannotPerformReason("Place an empty wheeled vehicle within the configured search radius of the driver");
			return false;
		}
		if (!CF_ConvoySession.CanAdd(user, driver))
		{
			SetCannotPerformReason("Start a convoy first, wait for boarding or release, or check the configured convoy limit");
			return false;
		}
		return true;
	}

	override void OnRejected(IEntity pUserEntity)
	{
		if (System.IsConsoleApp() || !GetGame() || !GetGame().GetPlayerController())
			return;
		SCR_HintManagerComponent.ShowCustomHint(
			"Could not add driver. Check the empty vehicle, convoy limit, and any boarding or release order.",
			"Convoy", 7.0, true);
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
