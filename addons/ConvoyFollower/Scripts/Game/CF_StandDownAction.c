// Optional manual cancellation while the driver is still reachable on foot.
class CF_StandDownAction : ScriptedUserAction
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Stand down";
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		CF_DriverControllerComponent controller = CF_DriverControllerComponent.Cast(GetOwner().FindComponent(CF_DriverControllerComponent));
		return controller && controller.CanStandDown() && !controller.IsOnFootFollower() && controller.CF_IsOwnedBy(user);
	}

	override bool CanBePerformedScript(IEntity user)
	{
		CF_DriverControllerComponent controller = CF_DriverControllerComponent.Cast(GetOwner().FindComponent(CF_DriverControllerComponent));
		if (Replication.IsServer() && !CF_ConvoySession.CanStandDown(user, controller))
		{
			SetCannotPerformReason("Only the player who ordered this convoy can stand it down");
			return false;
		}
		return controller && controller.CanStandDown();
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
			CF_ConvoySession.StandDown(pUserEntity, controller);
	}
}
