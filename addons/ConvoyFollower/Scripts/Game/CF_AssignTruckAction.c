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

		if (!controller.CanAssign(user))
		{
			SetCannotPerformReason("Place an empty wheeled vehicle within 35 m of the driver");
			return false;
		}
		if (Replication.IsServer() && !CF_ConvoySession.CanStart(user, controller))
		{
			SetCannotPerformReason("You already have a convoy or this driver is unavailable");
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
		CF_DriverControllerComponent controller = CF_DriverControllerComponent.Cast(pOwnerEntity.FindComponent(CF_DriverControllerComponent));
		if (controller)
			CF_ConvoySession.Start(pUserEntity, controller);
	}
}
