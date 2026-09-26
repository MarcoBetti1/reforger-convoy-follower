// Isolated taxiway experiment. It requests native AI cruise speed only;
// production owns all waypoints, steering, throttle, brakes and convoy state.
class CF_FollowCruiseProbeComponentClass : ScriptComponentClass
{
}

class CF_FollowCruiseProbeComponent : ScriptComponent
{
	protected Vehicle m_Truck;
	protected Vehicle m_Lead;
	protected AICarMovementComponent m_Movement;
	protected bool m_bOwnOverride;
	protected bool m_bMissingLogged;
	protected float m_fLastLimit = -1;
	protected float m_fStartMs;
	protected float m_fNextLogMs;
	protected string m_sResolver;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Truck = Vehicle.Cast(owner);
		if (!m_Truck || m_Truck.GetName() != "CF_SmokeFollower1")
			return;
		m_fStartMs = GetGame().GetWorld().GetWorldTime();
		RunPolicyCases();
		GetGame().GetCallqueue().CallLater(Tick, 200, true);
	}

	protected bool WorldAlive()
	{
		return GetGame() && GetGame().GetWorld() && !CF_ConvoySession.CF_IsWorldCleanup();
	}

	protected void ReleaseOverride()
	{
		if (m_bOwnOverride && m_Movement && WorldAlive())
			m_Movement.ResetCruiseSpeed();
		m_bOwnOverride = false;
		m_Movement = null;
		m_fLastLimit = -1;
	}

	override void OnDelete(IEntity owner)
	{
		if (GetGame())
			GetGame().GetCallqueue().Remove(Tick);
		ReleaseOverride();
		m_Truck = null;
		m_Lead = null;
	}

	protected AICarMovementComponent FromAgent(AIAgent agent)
	{
		if (!agent)
			return null;
		return AICarMovementComponent.Cast(agent.GetMovementComponent());
	}

	protected bool TryMovement(AICarMovementComponent movement, string path)
	{
		if (!movement)
			return false;
		AIAgent agent = movement.GetAIAgent();
		IEntity controlled;
		if (agent)
			controlled = agent.GetControlledEntity();
		// The seated character's ordinary movement component is never eligible.
		if (controlled != m_Truck)
			return false;
		m_Movement = movement;
		m_sResolver = path;
		string controlledName = "none";
		if (controlled)
			controlledName = controlled.GetName();
		Print("[ConvoyFollower] FOLLOW_CRUISE_RESOLVED: path=" + path +
			" assigned_vehicle=" + m_Truck.GetName() + " controlled=" + controlledName);
		return true;
	}

	protected bool ResolveMovement(ChimeraCharacter driver)
	{
		if (m_Movement)
			return true;
		if (TryMovement(AICarMovementComponent.Cast(m_Truck.FindComponent(AICarMovementComponent)), "truck.component"))
			return true;
		AIControlComponent control = AIControlComponent.Cast(m_Truck.FindComponent(AIControlComponent));
		if (control)
		{
			if (TryMovement(FromAgent(control.GetAIAgent()), "truck.agent") ||
				TryMovement(FromAgent(control.GetControlAIAgent()), "truck.control_agent"))
				return true;
		}
		control = driver.GetAIControlComponent();
		if (control)
		{
			if (TryMovement(FromAgent(control.GetControlAIAgent()), "driver.control_agent") ||
				TryMovement(FromAgent(control.GetAIAgent()), "driver.agent"))
				return true;
		}
		if (!m_bMissingLogged)
		{
			m_bMissingLogged = true;
			Print("[ConvoyFollower] FOLLOW_CRUISE_RESOLVER_MISSING: no native car movement proven to own " + m_Truck.GetName());
		}
		return false;
	}

	protected void Tick()
	{
		if (!WorldAlive())
			return;
		float now = GetGame().GetWorld().GetWorldTime();
		if (now - m_fStartMs > 240000.0)
		{
			ReleaseOverride();
			GetGame().GetCallqueue().Remove(Tick);
			Print("[ConvoyFollower] FOLLOW_CRUISE_FINISHED: bounded experiment ended");
			return;
		}
		if (!m_Truck)
			return;
		if (!m_Lead)
			m_Lead = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokeLead"));
		CarControllerComponent car = CarControllerComponent.Cast(m_Truck.FindComponent(CarControllerComponent));
		if (!m_Lead || !car || !car.GetSimulation() || !car.GetPilotCompartmentSlot())
			return;
		ChimeraCharacter driver = ChimeraCharacter.Cast(car.GetPilotCompartmentSlot().GetOccupant());
		CF_DriverControllerComponent convoy;
		if (driver)
			convoy = CF_DriverControllerComponent.Cast(driver.FindComponent(CF_DriverControllerComponent));
		if (!convoy || convoy.CF_GetAssignedVehicle() != m_Truck ||
			!convoy.CF_IsBoarded() || !convoy.CF_IsMovementActive())
		{
			ReleaseOverride();
			return;
		}
		CarControllerComponent leadCar = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (!leadCar || !leadCar.GetSimulation() || !ResolveMovement(driver))
			return;
		VehicleWheeledSimulation sim = car.GetSimulation();
		float gap = vector.DistanceXZ(m_Truck.GetOrigin(), m_Lead.GetOrigin());
		float leadSpeed = leadCar.GetSimulation().GetSpeedKmh();
		float limit = CF_FollowSpeedPolicy.MaxSpeedKmh(gap, CF_ConvoySettings.Get().m_fStoppedGap, leadSpeed);
		if (!m_bOwnOverride || Math.AbsFloat(limit - m_fLastLimit) >= 1.0 ||
			(limit <= 0.1 && m_fLastLimit > 0.1))
		{
			m_Movement.SetCruiseSpeed(limit);
			m_fLastLimit = limit;
			m_bOwnOverride = true;
		}
		if (now < m_fNextLogMs)
			return;
		m_fNextLogMs = now + 1000.0;
		Print("[ConvoyFollower] FOLLOW_CRUISE_STATUS: gap=" + gap + " lead_kmh=" + leadSpeed +
			" actual_kmh=" + sim.GetSpeedKmh() + " requested_kmh=" + m_fLastLimit +
			" envelope_kmh=" + limit + " brake=" + sim.GetBrake() + " throttle=" + sim.GetThrottle() +
			" engine=" + sim.EngineIsOn() + " state=" + convoy.CF_GetPanelStateLabel() +
			" resolver=" + m_sResolver);
	}

	protected void RunPolicyCases()
	{
		int passed;
		if (CF_FollowSpeedPolicy.MaxSpeedKmh(12, 10, 0) == 0) passed++;
		if (CF_FollowSpeedPolicy.MaxSpeedKmh(-1, 10, 0) == 0) passed++;
		if (CF_FollowSpeedPolicy.MaxSpeedKmh(1000, 10, 0) == 45) passed++;
		if (Math.AbsFloat(CF_FollowSpeedPolicy.MaxSpeedKmh(32, 10, 0) - 23.1623) < 0.01) passed++;
		if (CF_FollowSpeedPolicy.MaxSpeedKmh(32, 10, 18) > CF_FollowSpeedPolicy.MaxSpeedKmh(32, 10, 0)) passed++;
		if (CF_FollowSpeedPolicy.MaxSpeedKmh(42, 10, 0) > CF_FollowSpeedPolicy.MaxSpeedKmh(32, 10, 0)) passed++;
		if (passed == 6)
			Print("[ConvoyFollower] FOLLOW_SPEED_POLICY_RESULT: PASS cases=6");
		else
			Print("[ConvoyFollower] FOLLOW_SPEED_POLICY_RESULT: FAIL passed=" + passed + " cases=6");
	}
}
