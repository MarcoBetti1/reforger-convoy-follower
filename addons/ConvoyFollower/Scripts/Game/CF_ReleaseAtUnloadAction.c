// Rear cargo-menu action on a supported vehicle. The ordering player
// explicitly sends the front truck out of the stopping spot.
class CF_ReleaseAtUnloadAction : ScriptedUserAction
{
	protected CF_DriverControllerComponent GetConvoyDriver()
	{
		// The rear cargo action belongs to an attached cargo part. The slot
		// registers that part's actions into the parent truck's scroll menu.
		IEntity parent = GetOwner();
		Vehicle vehicle;
		while (parent && !vehicle)
		{
			vehicle = Vehicle.Cast(parent);
			parent = parent.GetParent();
		}
		if (!vehicle)
			return null;

		BaseCompartmentManagerComponent compartments = BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(BaseCompartmentManagerComponent));
		if (!compartments)
			return null;

		ref array<BaseCompartmentSlot> slots = {};
		compartments.GetCompartments(slots);
		foreach (BaseCompartmentSlot slot : slots)
		{
			if (!slot || !slot.IsPiloting())
				continue;
			IEntity occupant = slot.GetOccupant();
			if (!occupant)
				continue;
			CF_DriverControllerComponent driver = CF_DriverControllerComponent.Cast(occupant.FindComponent(CF_DriverControllerComponent));
			// The occupant itself is visible on clients; assigned vehicle and
			// server driver references are checked authoritatively at release.
			if (driver)
				return driver;
		}
		return null;
	}

	override bool GetActionNameScript(out string outName)
	{
		outName = "Pull off and regroup";
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		CF_DriverControllerComponent driver = GetConvoyDriver();
		return driver && driver.CF_GetUnitNumber() == 1 && driver.CF_IsOwnedBy(user) &&
			driver.CF_CanReleaseAtUnload();
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
		if (Replication.IsServer() && !CF_ConvoySession.CanPlanReleaseAtUnload(user, driver))
		{
			SetCannotPerformReason("No clear road slot behind convoy; stop on a wider road");
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
		CF_DriverControllerComponent driver = GetConvoyDriver();
		if (driver)
			CF_ConvoySession.ReleaseAtUnload(pUserEntity, driver);
	}
}
