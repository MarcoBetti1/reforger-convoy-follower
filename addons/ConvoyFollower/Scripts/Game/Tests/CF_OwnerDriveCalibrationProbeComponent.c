// Test-only calibration of a locally controlled player's lead M923. This
// isolates scripted physical driving from route, convoy, and parking logic.
// PASS requires powered forward progress uphill, not unsigned/coasting
// displacement. The original gear-1 run coasted backward and is invalid.
class CF_OwnerDriveCalibrationProbeComponentClass : ScriptComponentClass
{
}

class CF_OwnerDriveCalibrationProbeComponent : ScriptComponent
{
	protected Vehicle m_Lead;
	protected ChimeraCharacter m_Player;
	protected vector m_vStart;
	protected vector m_vStartForward;
	protected vector m_vLast;
	protected bool m_bHoldingBrake;
	protected bool m_bDriving;
	protected bool m_bFinished;
	protected bool m_bConfirming;
	protected bool m_bOriginalPilotLock;
	protected int m_iReadyTicks;
	protected int m_iDriveTicks;
	protected int m_iPostframeTicks;
	protected int m_iSimulateTicks;
	protected int m_iPostSimulateTicks;
	protected int m_iPoweredForwardSamples;
	protected int m_iPhysicsPoweredSamples;
	protected int m_iMode;
	protected int m_iModeTicks;
	protected int m_iLastPreLogTick = -1;
	protected int m_iLastPostLogTick = -1;
	protected int m_iPreThrottleResetCount;
	protected int m_iPostThrottleResetCount;
	protected int m_iBrakeResetCount;
	protected float m_fRequestedThrottle = 0.28;
	protected float m_fModeStartProgress;
	protected float m_fPath;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Lead = Vehicle.Cast(owner);
		if (!m_Lead)
			return;
		m_bHoldingBrake = true;
		SetEventMask(owner, EntityEvent.POSTFRAME | EntityEvent.SIMULATE | EntityEvent.POSTSIMULATE);
		GetGame().GetCallqueue().CallLater(Poll, 1000, true);
		Print("[ConvoyFollower] OWNER_DRIVE_CAL_INIT: test-only graduated owner-pilot physics calibration; signed uphill gate");
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

	protected VehicleWheeledSimulation Simulation()
	{
		if (!m_Lead)
			return null;
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (!car)
			return null;
		return car.GetSimulation();
	}

	protected float ForwardProgress()
	{
		vector delta = m_Lead.GetOrigin() - m_vStart;
		return delta[0] * m_vStartForward[0] + delta[2] * m_vStartForward[2];
	}

	protected string ModeName()
	{
		if (m_iMode < 3)
			return "POSTFRAME";
		if (m_iMode == 3)
			return "SIMULATE";
		return "SIMULATE_PILOT_LOCKED";
	}

	protected void ApplyDriveInputs()
	{
		VehicleWheeledSimulation sim = Simulation();
		if (!sim)
			return;
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		car.SetPersistentHandBrake(false);
		if (!sim.EngineIsOn())
			sim.EngineStart();
		// Installed NEUTRAL_GEAR is 1. Bohemia's vehicleGO sample uses 2.
		if (sim.GetGear() != 2)
			sim.SetGear(2);
		sim.SetClutch(1);
		sim.SetBreak(0, false);
		sim.SetSteering(0);
		sim.SetThrottle(m_fRequestedThrottle);
	}

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (m_bFinished || !m_Lead)
			return;
		VehicleWheeledSimulation sim = Simulation();
		if (!sim)
			return;
		if (m_bHoldingBrake)
		{
			CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
			car.SetPersistentHandBrake(true);
			sim.SetThrottle(0);
			sim.SetBreak(1, true);
			return;
		}
		if (!m_bDriving)
			return;
		m_iPostframeTicks++;
		if (m_iMode < 3)
			ApplyDriveInputs();
	}

	// Installed ScriptComponent API: SIMULATE runs before every physics
	// fixed step, POSTSIMULATE after it (possibly several times per frame).
	override void EOnSimulate(IEntity owner, float timeSlice)
	{
		if (!m_bDriving || m_bFinished)
			return;
		VehicleWheeledSimulation sim = Simulation();
		if (!sim)
			return;
		m_iSimulateTicks++;
		if (sim.GetThrottle() < 0.01)
			m_iPreThrottleResetCount++;
		if (sim.GetBrake() > 0.05)
			m_iBrakeResetCount++;
		if (m_iLastPreLogTick != m_iDriveTicks)
		{
			m_iLastPreLogTick = m_iDriveTicks;
			LogDrive("OWNER_DRIVE_CAL_PRE_PHYSICS_READ");
		}
		if (m_iMode >= 3)
		{
			ApplyDriveInputs();
			if (m_iSimulateTicks % 300 == 0)
				LogDrive("OWNER_DRIVE_CAL_SIMULATE_WRITE");
		}
	}

	override void EOnPostSimulate(IEntity owner, float timeSlice)
	{
		if (!m_bDriving || m_bFinished)
			return;
		VehicleWheeledSimulation sim = Simulation();
		if (!sim)
			return;
		m_iPostSimulateTicks++;
		if (sim.GetThrottle() < 0.01)
			m_iPostThrottleResetCount++;
		if (sim.EngineIsOn() && sim.GetGear() == 2 && sim.GetClutch() >= 0.9 &&
			sim.GetThrottle() > 0.1 && sim.GetBrake() == 0)
			m_iPhysicsPoweredSamples++;
		if (m_iLastPostLogTick != m_iDriveTicks)
		{
			m_iLastPostLogTick = m_iDriveTicks;
			LogDrive("OWNER_DRIVE_CAL_POST_PHYSICS_READ");
		}
	}

	protected void BeginMode(int mode)
	{
		m_iMode = mode;
		m_iModeTicks = 0;
		m_iPoweredForwardSamples = 0;
		m_iPhysicsPoweredSamples = 0;
		m_iPreThrottleResetCount = 0;
		m_iPostThrottleResetCount = 0;
		m_iBrakeResetCount = 0;
		m_fModeStartProgress = ForwardProgress();
		m_fRequestedThrottle = 0.5;
		if (mode == 0)
			m_fRequestedThrottle = 0.28;
		else if (mode == 2)
			m_fRequestedThrottle = 0.8;
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (car)
			car.LockPilotControls(mode == 4);
		// Explicitly isolate the v2 sleeping-body observation. Baseline
		// POSTFRAME stages remain untouched; do not keep the body awake.
		if (mode >= 3)
		{
			Physics physics = m_Lead.GetPhysics();
			if (physics)
			{
				bool wasActive = physics.IsActive();
				physics.SetActive(ActiveState.ACTIVE);
				Print("[ConvoyFollower] OWNER_DRIVE_CAL_WAKE: stage=" + mode +
					" active_before=" + wasActive + " active_after=" + physics.IsActive());
			}
		}
		Print("[ConvoyFollower] OWNER_DRIVE_CAL_STAGE: stage=" + mode + " mode=" + ModeName() +
			" requested_throttle=" + m_fRequestedThrottle + " gear=2 clutch=1 forward_start=" + m_fModeStartProgress);
	}

	protected void LogWheels()
	{
		VehicleWheeledSimulation sim = Simulation();
		if (!sim)
			return;
		for (int wheel = 0; wheel < sim.WheelCount(); wheel++)
		{
			Print("[ConvoyFollower] OWNER_DRIVE_CAL_WHEEL: stage=" + m_iMode + " wheel=" + wheel +
				" contact=" + sim.WheelHasContact(wheel) + " rpm=" + sim.WheelGetRPM(wheel) +
				" longitudinal_slip=" + sim.WheelGetLongitudinalSlip(wheel) +
				" rolling_drag=" + sim.WheelGetRollingDrag(wheel));
		}
	}

	protected void LogInputNames()
	{
		InputManager inputs = GetGame().GetInputManager();
		if (!inputs)
			return;
		int logged;
		for (int i = 0; i < inputs.GetActionCount() && logged < 25; i++)
		{
			string actionName = inputs.GetActionName(i);
			if (actionName.IndexOf("Car") == -1 && actionName.IndexOf("Vehicle") == -1)
				continue;
			Print("[ConvoyFollower] OWNER_DRIVE_CAL_INPUT_NAME: " + actionName + " value=" + inputs.GetActionValue(actionName));
			logged++;
		}
	}

	protected void LogDrive(string marker)
	{
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (!car)
			return;
		VehicleWheeledSimulation sim = car.GetSimulation();
		if (!sim)
			return;
		vector position = m_Lead.GetOrigin();
		vector delta = position - m_vStart;
		float forwardProgress = delta[0] * m_vStartForward[0] + delta[2] * m_vStartForward[2];
		Physics physics = m_Lead.GetPhysics();
		bool physicsActive;
		if (physics)
			physicsActive = physics.IsActive();
		Print("[ConvoyFollower] " + marker + ": stage=" + m_iMode + " mode=" + ModeName() +
			" requested_throttle=" + m_fRequestedThrottle + " callbacks=" + m_iPostframeTicks +
			" simulate_callbacks=" + m_iSimulateTicks + " postsimulate_callbacks=" + m_iPostSimulateTicks +
			" seconds=" + m_iDriveTicks + " path_m=" + m_fPath +
			" displacement_m=" + vector.DistanceXZ(m_vStart, m_Lead.GetOrigin()) +
			" origin=" + m_Lead.GetOrigin() + " engine=" + sim.EngineIsOn() +
			" controller_engine=" + car.IsEngineOn() + " can_move=" + car.CanMove() +
			" physics_present=" + (physics != null) + " physics_active=" + physicsActive +
			" rpm=" + sim.EngineGetRPM() + " gear=" + sim.GetGear() +
			" engine_load=" + sim.EngineGetLoad() + " feedback_rpm=" + sim.EngineGetRPMFeedback() +
			" gearbox_efficiency=" + sim.GearboxGetEfficiencyState() +
			" clutch=" + sim.GetClutch() + " throttle=" + sim.GetThrottle() +
			" brake=" + sim.GetBrake() + " speed_kmh=" + sim.GetSpeedKmh() +
			" handbrake=" + car.GetHandBrake() + " persistent=" + car.GetPersistentHandBrake() +
			" forward_m=" + forwardProgress + " elevation_gain=" + (position[1] - m_vStart[1]) +
			" powered_samples=" + m_iPoweredForwardSamples + " physics_powered_samples=" + m_iPhysicsPoweredSamples +
			" pre_throttle_zero=" + m_iPreThrottleResetCount + " post_throttle_zero=" + m_iPostThrottleResetCount +
			" pre_brake_nonzero=" + m_iBrakeResetCount + " pilot_locked=" + car.ArePilotControlsLocked());
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
			car.LockPilotControls(m_bOriginalPilotLock);
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
			m_vStartForward = m_Lead.GetWorldTransformAxis(2);
			m_vStartForward[1] = 0;
			m_vStartForward.Normalize();
			m_vLast = m_vStart;
			m_bHoldingBrake = false;
			CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
			if (car)
			{
				m_bOriginalPilotLock = car.ArePilotControlsLocked();
				car.SetPersistentHandBrake(false);
				car.StartEngine();
				VehicleWheeledSimulation sim = car.GetSimulation();
				if (sim)
					sim.SetBreak(0, false);
			}
			m_bDriving = true;
			BeginMode(0);
			LogInputNames();
			Print("[ConvoyFollower] OWNER_DRIVE_CAL_RELEASE: owner seated; gear=2 clutch=1; uphill forward axis=" + m_vStartForward);
			return;
		}
		m_iDriveTicks++;
		m_iModeTicks++;
		vector here = m_Lead.GetOrigin();
		m_fPath += vector.DistanceXZ(here, m_vLast);
		m_vLast = here;
		vector delta = here - m_vStart;
		float forwardProgress = delta[0] * m_vStartForward[0] + delta[2] * m_vStartForward[2];
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		VehicleWheeledSimulation sim;
		if (car)
			sim = car.GetSimulation();
		if (sim && sim.EngineIsOn() && sim.GetGear() == 2 && sim.GetClutch() >= 0.9 &&
			sim.GetThrottle() > 0 && sim.GetBrake() == 0 && !car.GetPersistentHandBrake())
			m_iPoweredForwardSamples++;
		else
			m_iPoweredForwardSamples = 0;
		LogDrive("OWNER_DRIVE_CAL_POLL");
		if (m_iDriveTicks % 5 == 0)
			LogWheels();
		if (forwardProgress < -2.0)
		{
			Finish("FAIL lead moved backward instead of powered forward");
			return;
		}
		if (m_fPath >= 20.0 && forwardProgress >= 20.0 && here[1] - m_vStart[1] >= 0.5 &&
			m_iPoweredForwardSamples >= 3 && m_iPhysicsPoweredSamples >= 3)
		{
			Finish("PASS owner-piloted lead drove at least 20 m forward uphill with engaged forward gear");
			return;
		}
		if (!m_bConfirming && forwardProgress - m_fModeStartProgress >= 2.0)
		{
			m_bConfirming = true;
			m_iModeTicks = 0;
			Print("[ConvoyFollower] OWNER_DRIVE_CAL_CONFIRMING: stage=" + m_iMode +
				" mode=" + ModeName() + " physical progress seen; keeping controls for signed uphill gate");
		}
		if (!m_bConfirming && m_iModeTicks >= 8)
		{
			Print("[ConvoyFollower] OWNER_DRIVE_CAL_STAGE_NO_PROGRESS: stage=" + m_iMode +
				" signed_stage_progress=" + (forwardProgress - m_fModeStartProgress));
			if (m_iMode < 4)
				BeginMode(m_iMode + 1);
			else
				Finish("FAIL all five bounded input stages made no forward progress");
			return;
		}
		if ((m_bConfirming && m_iModeTicks >= 40) || m_iDriveTicks >= 85)
			Finish("FAIL owner-piloted lead did not complete powered forward uphill gate");
	}
}
