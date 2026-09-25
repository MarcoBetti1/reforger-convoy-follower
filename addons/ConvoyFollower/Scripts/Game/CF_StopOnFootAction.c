// Only the player who staged this driver can release it before boarding.
class CF_StopOnFootAction : ScriptedUserAction
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Stop following on foot";
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		CF_DriverControllerComponent driver = CF_DriverControllerComponent.Cast(GetOwner().FindComponent(CF_DriverControllerComponent));
		return driver && driver.IsOnFootFollower() && driver.CF_IsOwnedBy(user);
	}

	override bool CanBePerformedScript(IEntity user)
	{
		CF_DriverControllerComponent driver = CF_DriverControllerComponent.Cast(GetOwner().FindComponent(CF_DriverControllerComponent));
		if (!driver || !driver.CanStopOnFoot(user))
		{
			SetCannotPerformReason("Only the player who ordered this driver can stop the follow order");
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
			driver.StopOnFoot(pUserEntity);
	}
}
