// The first recruit becomes Unit One. Each later recruit joins the chain.
class CF_AssignTruckAction : ScriptedUserAction
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Join my convoy in closest empty vehicle";
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		CF_DriverControllerComponent driver = CF_DriverControllerComponent.Cast(GetOwner().FindComponent(CF_DriverControllerComponent));
		return driver && (driver.IsIdle() || (driver.IsOnFootFollower() && driver.CF_IsOwnedBy(user)));
	}

	override bool CanBePerformedScript(IEntity user)
	{
		CF_DriverControllerComponent driver = CF_DriverControllerComponent.Cast(GetOwner().FindComponent(CF_DriverControllerComponent));
		if (!driver)
			return false;
		if (!Replication.IsServer())
			return true;
		if (!driver.CanAssign(user))
		{
			SetCannotPerformReason("Place an empty wheeled vehicle within the server's search radius of this driver");
			return false;
		}
		CF_ConvoySession session = CF_ConvoySession.GetForPlayer(user);
		if (session && !CF_ConvoySession.CanAdd(user, driver))
		{
			SetCannotPerformReason("Convoy is busy or at its configured unit limit (maximum five)");
			return false;
		}
		if (!session && !CF_ConvoySession.CanStart(user, driver))
		{
			SetCannotPerformReason("This driver cannot start a convoy now");
			return false;
		}
		return true;
	}

	override void OnRejected(IEntity pUserEntity)
	{
		if (System.IsConsoleApp() || !GetGame() || !GetGame().GetPlayerController())
			return;
		SCR_HintManagerComponent.ShowCustomHint(
			"Could not join this driver. Check their empty vehicle, current convoy order, and unit limit.",
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
		if (!driver)
			return;
		if (CF_ConvoySession.GetForPlayer(pUserEntity))
			CF_ConvoySession.Add(pUserEntity, driver);
		else
			CF_ConvoySession.Start(pUserEntity, driver);
	}
}
