// Stage an idle convoy driver by walking it toward a selected vehicle.
class CF_FollowOnFootAction : ScriptedUserAction
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Follow me on foot";
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		CF_DriverControllerComponent driver = CF_DriverControllerComponent.Cast(GetOwner().FindComponent(CF_DriverControllerComponent));
		return driver && driver.IsIdle();
	}

	override bool CanBePerformedScript(IEntity user)
	{
		CF_DriverControllerComponent driver = CF_DriverControllerComponent.Cast(GetOwner().FindComponent(CF_DriverControllerComponent));
		if (!driver || !driver.CanStartOnFoot(user))
		{
			SetCannotPerformReason("This driver cannot follow on foot right now");
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
		CF_DriverControllerComponent driver = CF_DriverControllerComponent.Cast(pOwnerEntity.FindComponent(CF_DriverControllerComponent));
		if (driver)
			driver.StartOnFoot(pUserEntity);
	}
}
