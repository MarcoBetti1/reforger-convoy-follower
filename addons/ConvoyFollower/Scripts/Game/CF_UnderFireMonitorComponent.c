// Reports confirmed hits on an owned, seated convoy driver or their truck,
// including trucks parked in the return line.
// The existing convoy session sends the report privately in Unit One's voice.
class CF_UnderFireMonitorComponentClass : ScriptComponentClass
{
}

class CF_UnderFireMonitorComponent : ScriptComponent
{
	protected static const float CF_MONITOR_POLL_SECONDS = 0.5;
	protected static const float CF_ATTACK_QUIET_SECONDS = 18.0;

	protected CF_DriverControllerComponent m_DriverController;
	protected SCR_DamageManagerComponent m_DriverDamageManager;
	protected SCR_DamageManagerComponent m_VehicleDamageManager;
	protected Vehicle m_MonitoredVehicle;
	protected float m_fPollSeconds;
	protected float m_fQuietSeconds;
	protected float m_fDriverMissingSeconds;
	protected float m_fVehicleMissingSeconds;
	protected bool m_bAttackEpisode;
	protected bool m_bDriverRetryLogged;
	protected bool m_bVehicleRetryLogged;
	protected bool m_bDriverMissingLogged;
	protected bool m_bVehicleMissingLogged;

	protected SCR_DamageManagerComponent ResolveDamageManager(IEntity entity)
	{
		if (!entity)
			return null;

		// Character/vehicle convenience getters can still be null during
		// OnPostInit. The component lookup works as soon as it is registered.
		SCR_DamageManagerComponent manager = SCR_DamageManagerComponent.GetDamageManager(entity);
		if (!manager)
			manager = SCR_DamageManagerComponent.Cast(entity.FindComponent(SCR_DamageManagerComponent));
		if (!manager && ChimeraCharacter.Cast(entity))
			manager = SCR_CharacterDamageManagerComponent.Cast(entity.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!manager && BaseVehicle.Cast(entity))
			manager = SCR_VehicleDamageManagerComponent.Cast(entity.FindComponent(SCR_VehicleDamageManagerComponent));
		return manager;
	}

	protected void AttachDriver(IEntity owner)
	{
		if (m_DriverDamageManager || !owner)
			return;

		m_DriverDamageManager = ResolveDamageManager(owner);
		if (!m_DriverDamageManager)
		{
			if (!m_bDriverRetryLogged)
			{
				m_bDriverRetryLogged = true;
				Print("[ConvoyFollower] UNDER_FIRE_MONITOR: driver damage manager pending; retrying");
			}
			return;
		}

		m_DriverDamageManager.GetOnDamage().Insert(OnDamage);
		if (m_bDriverRetryLogged)
			Print("[ConvoyFollower] UNDER_FIRE_MONITOR: driver damage manager attached after retry");
		else
			Print("[ConvoyFollower] UNDER_FIRE_MONITOR: driver damage manager attached");
	}

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;

		m_DriverController = CF_DriverControllerComponent.Cast(owner.FindComponent(CF_DriverControllerComponent));
		SetEventMask(owner, EntityEvent.FRAME);
		AttachDriver(owner);
	}

	protected void DetachVehicle()
	{
		if (m_VehicleDamageManager)
			m_VehicleDamageManager.GetOnDamage().Remove(OnDamage);

		m_VehicleDamageManager = null;
		m_MonitoredVehicle = null;
		m_fVehicleMissingSeconds = 0;
		m_bVehicleRetryLogged = false;
		m_bVehicleMissingLogged = false;
	}

	protected void AttachVehicle(Vehicle vehicle)
	{
		if (vehicle != m_MonitoredVehicle)
		{
			DetachVehicle();
			m_MonitoredVehicle = vehicle;
		}
		if (!vehicle)
			return;
		if (m_VehicleDamageManager)
			return;

		m_VehicleDamageManager = ResolveDamageManager(vehicle);
		if (m_VehicleDamageManager)
		{
			m_VehicleDamageManager.GetOnDamage().Insert(OnDamage);
			if (m_bVehicleRetryLogged)
				Print("[ConvoyFollower] UNDER_FIRE_MONITOR: assigned vehicle damage manager attached after retry");
			else
				Print("[ConvoyFollower] UNDER_FIRE_MONITOR: assigned vehicle damage manager attached");
		}
		else if (!m_bVehicleRetryLogged)
		{
			m_bVehicleRetryLogged = true;
			Print("[ConvoyFollower] UNDER_FIRE_MONITOR: assigned vehicle damage manager pending; retrying");
		}
	}

	protected bool IsCombatDamage(notnull BaseDamageContext damageContext)
	{
		if (damageContext.damageValue <= 0)
			return false;

		EDamageType damageType = damageContext.damageType;
		return damageType == EDamageType.KINETIC ||
			damageType == EDamageType.FRAGMENTATION ||
			damageType == EDamageType.PROCESSED_FRAGMENTATION ||
			damageType == EDamageType.EXPLOSIVE;
	}

	protected void OnDamage(notnull BaseDamageContext damageContext)
	{
		if (!Replication.IsServer() || !m_DriverController || !m_DriverController.CF_IsOwnedSeatedConvoyMember())
			return;

		// The driver might be walking to a truck, leaving one, or waiting for a
		// replacement order. Those hits do not describe an active convoy truck.
		if (!m_DriverController.CF_IsBoarded() || !IsCombatDamage(damageContext))
			return;

		if (!m_bAttackEpisode)
		{
			m_bAttackEpisode = true;
			Print("[ConvoyFollower] UNDER_FIRE: confirmed hit on convoy unit");
			m_DriverController.CF_ReportUnderFire();
		}

		m_fQuietSeconds = 0;
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!Replication.IsServer())
			return;

		if (m_bAttackEpisode)
		{
			m_fQuietSeconds += timeSlice;
			if (m_fQuietSeconds >= CF_ATTACK_QUIET_SECONDS)
			{
				m_bAttackEpisode = false;
				m_fQuietSeconds = 0;
			}
		}

		m_fPollSeconds += timeSlice;
		if (m_fPollSeconds < CF_MONITOR_POLL_SECONDS)
			return;
		m_fPollSeconds = 0;

		if (!m_DriverController)
			m_DriverController = CF_DriverControllerComponent.Cast(owner.FindComponent(CF_DriverControllerComponent));
		if (!m_DriverDamageManager)
		{
			m_fDriverMissingSeconds += CF_MONITOR_POLL_SECONDS;
			AttachDriver(owner);
			if (!m_DriverDamageManager && m_fDriverMissingSeconds >= 10.0 && !m_bDriverMissingLogged)
			{
				m_bDriverMissingLogged = true;
				Print("[ConvoyFollower] UNDER_FIRE_MONITOR: driver damage manager still missing after 10 s; continuing retry");
			}
		}

		if (!m_DriverController || !m_DriverController.CF_IsActiveConvoyMember() || !m_DriverController.CF_IsBoarded())
		{
			DetachVehicle();
			m_bAttackEpisode = false;
			m_fQuietSeconds = 0;
			return;
		}

		AttachVehicle(m_DriverController.CF_GetAssignedVehicle());
		if (m_MonitoredVehicle && !m_VehicleDamageManager)
		{
			m_fVehicleMissingSeconds += CF_MONITOR_POLL_SECONDS;
			if (m_fVehicleMissingSeconds >= 10.0 && !m_bVehicleMissingLogged)
			{
				m_bVehicleMissingLogged = true;
				Print("[ConvoyFollower] UNDER_FIRE_MONITOR: assigned vehicle damage manager still missing after 10 s; continuing retry");
			}
		}
	}

	override void OnDelete(IEntity owner)
	{
		if (!Replication.IsServer())
			return;

		DetachVehicle();
		if (m_DriverDamageManager)
			m_DriverDamageManager.GetOnDamage().Remove(OnDamage);
		m_DriverDamageManager = null;
		m_DriverController = null;
	}
}
