// Test-only calibration of a locally controlled player's lead M923. This
// isolates scripted physical driving from route, convoy, and parking logic.
// PASS requires the seated lead to move at least 20 m in the world.
class CF_OwnerDriveCalibrationProbeComponentClass : ScriptComponentClass
{
}

class CF_OwnerDriveCalibrationProbeComponent : ScriptComponent
{
	protected Vehicle m_Lead;
	protected ChimeraCharacter m_Player;
	protected vector m_vStart;
	protected vector m_vLast;
	protected bool m_bHoldingBrake;
	protected bool m_bDriving;
	protected bool m_bClutchTrial;
	protected bool m_bFinished;
	protected int m_iReadyTicks;
	protected int m_iDriveTicks;
	protected int m_iPostframeTicks;
	protected float m_fPath;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Lead = Vehicle.Cast(owner);
		if (!m_Lead)
			return;
		m_bHoldingBrake = true;
		SetEventMask(owner, EntityEvent.POSTFRAME);
		GetGame().GetCallqueue().CallLater(Poll, 1000, true);
		Print("[ConvoyFollower] OWNER_DRIVE_CAL_INIT: test-only owner-pilot physics calibration");
	}

	protected bool EnsureOwnerPilot()
	{
		PlayerManager players = GetGame().GetPlayerManager();
		if (!players)
			return false;
		ref array<int> ids = {};
		players.GetPlayers(ids);
		if (ids.IsEmpty())
			return false;
		PlayerController controller = players.GetPlayerController(ids[0]);
		if (!controller)
			return false;
		m_Player = ChimeraCharacter.Cast(controller.GetControlledEntity());
		if (!m_Player)
		{
			Resource crew = Resource.Load("{E1CB513B8B9B08F4}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_US_Crew.et");
			if (!crew.IsValid())
				return false;
			EntitySpawnParams params = EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = m_Lead.GetOrigin() + Vector(3, 0, 2);
			m_Player = ChimeraCharacter.Cast(GetGame().SpawnEntityPrefab(crew, GetGame().GetWorld(), params));
			if (!m_Player || !controller.SetControlledEntity(m_Player))
				return false;
			Print("[ConvoyFollower] OWNER_DRIVE_CAL_PLAYER: test owner possessed crew");
		}
		if (players.GetPlayerIdFromControlledEntity(m_Player) <= 0)
			return false;
		CompartmentAccessComponent access = m_Player.GetCompartmentAccessComponent();
		if (!access)
			return false;
		BaseCompartmentSlot current = access.GetCompartment();
		if (current && current.IsPiloting() && access.GetVehicleIn(m_Player) == m_Lead)
			return true;
		BaseCompartmentManagerComponent manager = BaseCompartmentManagerComponent.Cast(m_Lead.FindComponent(BaseCompartmentManagerComponent));
		if (!manager)
			return false;
		ref array<BaseCompartmentSlot> slots = {};
		manager.GetCompartments(slots);
		foreach (BaseCompartmentSlot slot : slots)
		{
			if (slot && slot.IsPiloting() && !slot.IsOccupied() && !slot.IsReserved())
			{
				if (access.GetInVehicle(m_Lead, slot, true, -1, ECloseDoorAfterActions.CLOSE_DOOR, true))
					Print("[ConvoyFollower] OWNER_DRIVE_CAL_SEAT: owner pilot seat requested");
				break;
			}
		}
		return false;
	}

	protected void SelectPlayerCamera()
	{
		PlayerController local = GetGame().GetPlayerController();
		if (!local || local.GetControlledEntity() != m_Player)
			return;
		bool editorClosed = !SCR_EditorManagerEntity.IsOpenedInstance() || SCR_EditorManagerEntity.CloseInstance();
		PlayerCamera playerCamera = local.GetPlayerCamera();
		if (editorClosed && playerCamera && GetGame().GetCameraManager().SetCamera(playerCamera))
		{
			CameraHandlerComponent handler = CameraHandlerComponent.Cast(m_Player.FindComponent(CameraHandlerComponent));
			if (handler)
				handler.SetThirdPerson(true);
			Print("[ConvoyFollower] OWNER_DRIVE_CAL_CAMERA: local player third person selected");
		}
	}

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (m_bFinished || !m_Lead)
			return;
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (!car)
			return;
		VehicleWheeledSimulation sim = car.GetSimulation();
		if (!sim)
			return;
		if (m_bHoldingBrake)
		{
			car.SetPersistentHandBrake(true);
			sim.SetThrottle(0);
			sim.SetBreak(1, true);
			return;
		}
		if (!m_bDriving)
			return;
		car.SetPersistentHandBrake(false);
		if (!sim.EngineIsOn())
			sim.EngineStart();
		if (sim.GetGear() < 1)
			sim.SetGear(1);
		if (m_bClutchTrial)
			sim.SetClutch(0);
		sim.SetBreak(0, false);
		sim.SetSteering(0);
		sim.SetThrottle(0.4);
		m_iPostframeTicks++;
		if (m_iPostframeTicks % 120 == 0)
			LogDrive("OWNER_DRIVE_CAL_POSTFRAME");
	}

	protected void LogDrive(string marker)
	{
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (!car)
			return;
		VehicleWheeledSimulation sim = car.GetSimulation();
		if (!sim)
			return;
		Print("[ConvoyFollower] " + marker + ": callbacks=" + m_iPostframeTicks +
			" seconds=" + m_iDriveTicks + " path_m=" + m_fPath +
			" displacement_m=" + vector.DistanceXZ(m_vStart, m_Lead.GetOrigin()) +
			" origin=" + m_Lead.GetOrigin() + " engine=" + sim.EngineIsOn() +
			" rpm=" + sim.EngineGetRPM() + " gear=" + sim.GetGear() +
			" clutch=" + sim.GetClutch() + " throttle=" + sim.GetThrottle() +
			" brake=" + sim.GetBrake() + " speed_kmh=" + sim.GetSpeedKmh() +
			" handbrake=" + car.GetHandBrake() + " persistent=" + car.GetPersistentHandBrake() +
			" clutch_trial=" + m_bClutchTrial);
	}

	protected void Finish(string result)
	{
		if (m_bFinished)
			return;
		m_bFinished = true;
		m_bDriving = false;
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (car)
		{
			car.SetPersistentHandBrake(true);
			VehicleWheeledSimulation sim = car.GetSimulation();
			if (sim)
			{
				sim.SetThrottle(0);
				sim.SetBreak(1, true);
			}
		}
		LogDrive("OWNER_DRIVE_CAL_FINAL_STATE");
		Print("[ConvoyFollower] OWNER_DRIVE_CAL_RESULT: " + result);
		GetGame().GetCallqueue().Remove(Poll);
	}

	protected void Poll()
	{
		if (m_bFinished)
			return;
		if (!EnsureOwnerPilot())
			return;
		if (!m_bDriving)
		{
			m_iReadyTicks++;
			if (m_iReadyTicks < 2)
				return;
			SelectPlayerCamera();
			m_vStart = m_Lead.GetOrigin();
			m_vLast = m_vStart;
			m_bHoldingBrake = false;
			CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
			if (car)
			{
				car.SetPersistentHandBrake(false);
				VehicleWheeledSimulation sim = car.GetSimulation();
				if (sim)
					sim.SetBreak(0, false);
			}
			m_bDriving = true;
			Print("[ConvoyFollower] OWNER_DRIVE_CAL_RELEASE: owner seated; scripted throttle starting");
			return;
		}
		m_iDriveTicks++;
		vector here = m_Lead.GetOrigin();
		m_fPath += vector.DistanceXZ(here, m_vLast);
		m_vLast = here;
		LogDrive("OWNER_DRIVE_CAL_POLL");
		if (m_fPath >= 20.0 && vector.DistanceXZ(m_vStart, here) >= 20.0)
		{
			Finish("PASS owner-piloted lead physically drove 20 m under script control");
			return;
		}
		if (m_iDriveTicks == 10 && m_fPath < 5.0)
		{
			m_bClutchTrial = true;
			Print("[ConvoyFollower] OWNER_DRIVE_CAL_CLUTCH_TRIAL: little movement under gear/throttle; trying clutch input zero");
		}
		if (m_iDriveTicks >= 40)
			Finish("FAIL owner-piloted lead did not physically drive 20 m");
	}
}
