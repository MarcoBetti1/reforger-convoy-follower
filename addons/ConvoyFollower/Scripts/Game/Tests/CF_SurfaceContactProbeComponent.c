// Test-only surface evidence. Reads each wheel's actual physics contact;
// this does not infer ground material from map color or road membership.
// Installed API: VehicleWheeledSimulation.WheelCount/WheelHasContact/
// WheelGetContactMaterial/WheelGetContactPosition. GameMaterial inherits
// GameLibMaterial, including BaseResourceObject.GetResourceName().
class CF_SurfaceContactProbeComponentClass : ScriptComponentClass
{
}

class CF_SurfaceContactProbeComponent : ScriptComponent
{
	protected Vehicle m_Truck;
	protected int m_iSample;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Truck = Vehicle.Cast(owner);
		if (m_Truck)
			GetGame().GetCallqueue().CallLater(SampleContacts, 5000, true);
	}

	override void OnDelete(IEntity owner)
	{
		if (GetGame())
			GetGame().GetCallqueue().Remove(SampleContacts);
	}

	protected void SampleContacts()
	{
		if (!m_Truck || !GetGame() || !GetGame().GetWorld())
			return;
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode || gameMode.GetState() != SCR_EGameModeState.GAME)
			return;
		CarControllerComponent car = CarControllerComponent.Cast(m_Truck.FindComponent(CarControllerComponent));
		if (!car)
			return;
		VehicleWheeledSimulation sim = car.GetSimulation();
		if (!sim)
			return;

		m_iSample++;
		int wheelCount = sim.WheelCount();
		int contacts;
		for (int wheel = 0; wheel < wheelCount; wheel++)
		{
			if (!sim.WheelHasContact(wheel))
				continue;
			contacts++;
			GameMaterial material = sim.WheelGetContactMaterial(wheel);
			string materialName = "<null-contact-material>";
			if (material)
			{
				materialName = material.GetResourceName();
				if (materialName.IsEmpty())
					materialName = "<unnamed-contact-material>";
			}
			Print("[ConvoyFollower] TEST_WHEEL_SURFACE: sample=" + m_iSample +
				" truck=" + m_Truck.GetName() + " wheel=" + wheel +
				" contact=" + sim.WheelGetContactPosition(wheel) +
				" material=" + materialName + " speed_kmh=" + sim.GetSpeedKmh());
		}
		Print("[ConvoyFollower] TEST_SURFACE_SAMPLE: sample=" + m_iSample +
			" truck=" + m_Truck.GetName() + " position=" + m_Truck.GetOrigin() +
			" wheels=" + wheelCount + " contacts=" + contacts +
			" speed_kmh=" + sim.GetSpeedKmh() + " game_state=GAME");
	}
}
