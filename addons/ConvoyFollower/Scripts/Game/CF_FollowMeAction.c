// Reserved for a future vehicle-mounted interaction; intentionally not placed on
// the character prefab because its action is not known to be reachable in a truck.
class CF_FollowMeAction : ScriptedUserAction
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Follow me";
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		CF_DriverControllerComponent controller = CF_DriverControllerComponent.Cast(GetOwner().FindComponent(CF_DriverControllerComponent));
		return controller && controller.IsWaitingForLead();
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
			controller.FollowLead(pUserEntity);
	}
}
