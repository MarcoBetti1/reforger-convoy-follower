// One controller-owned lease over one assigned AI truck's native speed cap.
// This class does not choose convoy state, target, route, steering or brakes.
// The native API has no priority/owner token or getter: callers must ensure
// there is only one Convoy cruise writer for this truck, including test probes.
class CF_NativeCruiseControl
{
	protected ChimeraCharacter m_Driver;
	protected Vehicle m_Truck;
	protected AICarMovementComponent m_Movement;
	protected bool m_bOwnOverride;
	protected float m_fRequestedSpeedKmh = -1;
	protected string m_sResolver;
	protected string m_sReason;
	protected string m_sLastFailure;

	protected bool WorldAlive()
	{
		return Replication.IsServer() && GetGame() && GetGame().GetWorld() &&
			!CF_ConvoySession.CF_IsWorldCleanup();
	}

	// Recheck the actual seat on every request/reset. Assignment alone is not
	// authority over a truck whose pilot has changed or become a player.
	protected bool IsAssignedAIPilot(ChimeraCharacter driver, Vehicle truck)
	{
		if (!driver || !truck || !WorldAlive())
			return false;
		PlayerManager players = GetGame().GetPlayerManager();
		if (!players || players.GetPlayerIdFromControlledEntity(driver) > 0 ||
			players.GetPlayerIdFromControlledEntity(truck) > 0)
			return false;
		CompartmentAccessComponent access = driver.GetCompartmentAccessComponent();
		if (!access || access.GetVehicleIn(driver) != truck)
			return false;
		BaseCompartmentSlot slot = access.GetCompartment();
		if (!slot || !slot.IsPiloting() || slot.GetOccupant() != driver)
			return false;
		CarControllerComponent car = CarControllerComponent.Cast(truck.FindComponent(CarControllerComponent));
		if (!car || !car.GetSimulation() || car.GetPilotCompartmentSlot() != slot)
			return false;
		AIControlComponent control = driver.GetAIControlComponent();
		AIAgent agent;
		if (control)
			agent = control.GetAIAgent();
		return agent && agent.GetControlledEntity() == driver;
	}

	protected bool DriverAndTruckAlive(ChimeraCharacter driver, Vehicle truck)
	{
		SCR_DamageManagerComponent driverDamage = driver.GetDamageManager();
		if (driverDamage && driverDamage.IsDestroyed())
			return false;
		SCR_DamageManagerComponent truckDamage = SCR_DamageManagerComponent.GetDamageManager(truck);
		return !truckDamage || !truckDamage.IsDestroyed();
	}

	protected bool MovementOwnsTruck(AICarMovementComponent movement, Vehicle truck)
	{
		if (!movement || !truck)
			return false;
		AIAgent agent = movement.GetAIAgent();
		return agent && agent.GetControlledEntity() == truck;
	}

	protected AICarMovementComponent FromAgent(AIAgent agent)
	{
		if (!agent)
			return null;
		return AICarMovementComponent.Cast(agent.GetMovementComponent());
	}

	protected bool TryMovement(AICarMovementComponent movement, string resolver)
	{
		if (!MovementOwnsTruck(movement, m_Truck))
			return false;
		m_Movement = movement;
		m_sResolver = resolver;
		return true;
	}

	protected bool ResolveMovement()
	{
		if (TryMovement(AICarMovementComponent.Cast(m_Truck.FindComponent(AICarMovementComponent)), "truck.component"))
			return true;
		AIControlComponent control = AIControlComponent.Cast(m_Truck.FindComponent(AIControlComponent));
		if (control && (TryMovement(FromAgent(control.GetAIAgent()), "truck.agent") ||
			TryMovement(FromAgent(control.GetControlAIAgent()), "truck.control_agent")))
			return true;
		control = m_Driver.GetAIControlComponent();
		return control && (TryMovement(FromAgent(control.GetControlAIAgent()), "driver.control_agent") ||
			TryMovement(FromAgent(control.GetAIAgent()), "driver.agent"));
	}

	protected void ForgetBinding()
	{
		m_bOwnOverride = false;
		m_fRequestedSpeedKmh = -1;
		m_Driver = null;
		m_Truck = null;
		m_Movement = null;
		m_sResolver = string.Empty;
		m_sReason = string.Empty;
	}

	protected bool Refuse(string reason)
	{
		if (m_sLastFailure != reason)
		{
			Print("[ConvoyFollower] NATIVE_CRUISE_REFUSED: reason=" + reason);
			m_sLastFailure = reason;
		}
		return false;
	}

	// Speeds are km/h. The caller supplies its policy cap or zero for an
	// explicit hold. A true result means the request is owned, not that the
	// truck has physically reached that speed or stopped.
	bool Request(ChimeraCharacter driver, Vehicle assignedTruck, float maxSpeedKmh, string reason)
	{
		if (!WorldAlive())
		{
			DetachForWorldCleanup();
			return false;
		}
		// Range check also rejects NaN/infinity. 1000 is only an invalid-input
		// ceiling; the current convoy policy supplies at most 45 km/h.
		if (!(maxSpeedKmh >= 0 && maxSpeedKmh <= 1000))
		{
			Release("invalid_speed");
			return Refuse("invalid_speed");
		}
		if (!IsAssignedAIPilot(driver, assignedTruck))
		{
			Release("pilot_ownership_lost");
			return Refuse("assigned_ai_pilot_unavailable");
		}
		if (!DriverAndTruckAlive(driver, assignedTruck))
		{
			Release("driver_or_truck_destroyed");
			return Refuse("driver_or_truck_destroyed");
		}
		if (m_Driver != driver || m_Truck != assignedTruck ||
			(m_bOwnOverride && !MovementOwnsTruck(m_Movement, m_Truck)))
			Release("binding_changed");
		if (!m_Movement)
		{
			m_Driver = driver;
			m_Truck = assignedTruck;
			if (!ResolveMovement())
			{
				ForgetBinding();
				return Refuse("assigned_native_car_movement_unavailable");
			}
			Print("[ConvoyFollower] NATIVE_CRUISE_BOUND: truck=" + m_Truck.GetName() + " resolver=" + m_sResolver);
		}
		// Preserve the tested 1 km/h update tolerance, but never leave a
		// positive cap active when Hold asks for zero, or zero on Resume.
		bool zeroTransition = (maxSpeedKmh == 0) != (m_fRequestedSpeedKmh == 0);
		if (!m_bOwnOverride || zeroTransition || Math.AbsFloat(maxSpeedKmh - m_fRequestedSpeedKmh) >= 1.0)
		{
			m_Movement.SetCruiseSpeed(maxSpeedKmh);
			m_fRequestedSpeedKmh = maxSpeedKmh;
			m_bOwnOverride = true;
		}
		m_sReason = reason;
		m_sLastFailure = string.Empty;
		return true;
	}

	// Call before planned dismount, dismissal, assignment/target handoff or a
	// state leaving pacing. A late call after another pilot takes over must
	// abandon the lease without altering that pilot's native controller.
	void Release(string reason)
	{
		if (m_bOwnOverride && WorldAlive())
		{
			if (IsAssignedAIPilot(m_Driver, m_Truck) && MovementOwnsTruck(m_Movement, m_Truck))
			{
				m_Movement.ResetCruiseSpeed();
				Print("[ConvoyFollower] NATIVE_CRUISE_RELEASED: truck=" + m_Truck.GetName() + " reason=" + reason);
			}
			else
				Print("[ConvoyFollower] NATIVE_CRUISE_ABANDONED: reason=" + reason +
					" ownership_lost=true native_reset=false");
		}
		ForgetBinding();
	}

	// Destruction-time detach deliberately performs no native call. The owner
	// must use Release while live, before its references or seat control change.
	void DetachForWorldCleanup()
	{
		ForgetBinding();
		m_sLastFailure = string.Empty;
	}

	bool OwnsOverride() { return m_bOwnOverride; }
	float GetRequestedSpeedKmh() { return m_fRequestedSpeedKmh; }
	string GetResolver() { return m_sResolver; }
	string GetReason() { return m_sReason; }
	string GetLastFailure() { return m_sLastFailure; }
}
