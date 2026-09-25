// Demo-only spawn point. The normal US spawn behavior remains available if
// the named LAV or its pilot seat is unavailable.
[EntityEditorProps(category: "GameScripted/Respawn", description: "Spawn the player in a named preplaced vehicle's pilot seat")]
class CF_LAVSpawnPointClass : SCR_SpawnPointClass
{
}

class CF_LAVSpawnPoint : SCR_SpawnPoint
{
	[Attribute(defvalue: "CF_EveronLeadLAV", desc: "World entity name of the preplaced lead LAV")]
	protected string m_sLeadVehicleName;

	protected IEntity m_SeatPendingEntity;
	protected float m_fSeatDeadlineMs;

	protected Vehicle FindLeadVehicle()
	{
		if (!GetGame() || !GetGame().GetWorld() || m_sLeadVehicleName.IsEmpty())
			return null;

		Vehicle vehicle = Vehicle.Cast(GetGame().GetWorld().FindEntityByName(m_sLeadVehicleName));
		if (!vehicle || vector.Distance(vehicle.GetOrigin(), GetOrigin()) > 60.0)
			return null;

		return vehicle;
	}

	protected BaseCompartmentSlot FindFreePilotSeat(Vehicle vehicle)
	{
		if (!vehicle)
			return null;

		BaseCompartmentManagerComponent manager = BaseCompartmentManagerComponent.Cast(
			vehicle.FindComponent(BaseCompartmentManagerComponent));
		if (!manager)
			return null;

		ref array<BaseCompartmentSlot> slots = {};
		manager.GetCompartments(slots);
		foreach (BaseCompartmentSlot slot : slots)
		{
			if (slot && slot.IsPiloting() && !slot.IsOccupied() && !slot.IsReserved())
				return slot;
		}
		return null;
	}

	override bool PrepareSpawnedEntity_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		if (!super.PrepareSpawnedEntity_S(requestComponent, data, entity))
			return false;

		m_SeatPendingEntity = null;
		ChimeraCharacter player = ChimeraCharacter.Cast(entity);
		Vehicle vehicle = FindLeadVehicle();
		BaseCompartmentSlot pilot = FindFreePilotSeat(vehicle);
		if (!player || !pilot)
		{
			Print("[ConvoyFollower] LAV_SPAWN_FALLBACK: named lead LAV or pilot seat unavailable");
			return true;
		}

		CompartmentAccessComponent access = player.GetCompartmentAccessComponent();
		if (!access || !access.GetInVehicle(vehicle, pilot, true, -1, ECloseDoorAfterActions.CLOSE_DOOR, true))
		{
			Print("[ConvoyFollower] LAV_SPAWN_FALLBACK: vehicle seat request failed");
			return true;
		}

		m_SeatPendingEntity = entity;
		m_fSeatDeadlineMs = GetGame().GetWorld().GetWorldTime() + 3000.0;
		return true;
	}

	override bool CanFinalizeSpawn_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		if (!super.CanFinalizeSpawn_S(requestComponent, data, entity))
			return false;

		if (m_SeatPendingEntity != entity)
			return true;

		ChimeraCharacter player = ChimeraCharacter.Cast(entity);
		CompartmentAccessComponent access;
		if (player)
			access = player.GetCompartmentAccessComponent();

		BaseCompartmentSlot seat;
		if (access)
			seat = access.GetCompartment();

		if (seat && seat.IsPiloting() && access.GetVehicleIn(player) == FindLeadVehicle())
		{
			m_SeatPendingEntity = null;
			Print("[ConvoyFollower] LAV_SPAWN_SEATED: player finalized in lead vehicle pilot seat");
			return true;
		}

		if (GetGame().GetWorld().GetWorldTime() < m_fSeatDeadlineMs)
			return false;

		m_SeatPendingEntity = null;
		Print("[ConvoyFollower] LAV_SPAWN_FALLBACK: pilot seat did not complete within 3 seconds");
		return true;
	}
}
