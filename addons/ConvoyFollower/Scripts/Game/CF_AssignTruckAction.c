// Start a new convoy with this driver as Unit One. The next idle drivers can
// be added or made the new leader through their separate interactions.
class CF_AssignTruckAction : ScriptedUserAction
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Start convoy in closest empty vehicle";
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		CF_DriverControllerComponent controller = CF_DriverControllerComponent.Cast(GetOwner().FindComponent(CF_DriverControllerComponent));
		return controller && (controller.IsIdle() || (controller.IsOnFootFollower() && controller.CF_IsOwnedBy(user))) && !CF_ConvoySession.HasActiveConvoyForActions(user);
	}

	override bool CanBePerformedScript(IEntity user)
	{
		CF_DriverControllerComponent controller = CF_DriverControllerComponent.Cast(GetOwner().FindComponent(CF_DriverControllerComponent));
		if (!controller)
			return false;

		// A dedicated server may override the vehicle search radius. The client
		// does not have that profile value, so leave vehicle eligibility to the
		// authoritative server-first check below.
		if (!Replication.IsServer())
			return true;

		if (!controller.CanAssign(user))
		{
			SetCannotPerformReason("Place an empty wheeled vehicle within the configured search radius of the driver");
			return false;
		}
		if (!CF_ConvoySession.CanStart(user, controller))
		{
			SetCannotPerformReason("You already have a convoy or this driver is unavailable");
			return false;
		}

		return true;
	}

	override void OnRejected(IEntity pUserEntity)
	{
		if (System.IsConsoleApp() || !GetGame() || !GetGame().GetPlayerController())
			return;
		SCR_HintManagerComponent.ShowCustomHint(
			"Could not start convoy. Check this driver, a nearby empty wheeled vehicle, and any existing convoy.",
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
		CF_DriverControllerComponent controller = CF_DriverControllerComponent.Cast(pOwnerEntity.FindComponent(CF_DriverControllerComponent));
		if (controller)
			CF_ConvoySession.Start(pUserEntity, controller);
	}
}
