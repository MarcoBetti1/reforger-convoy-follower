class CF_ActionInputCharacterControllerClass : SCR_CharacterControllerComponentClass
{
}

// Private character prefab only. This is an engine-action calibration, not
// simulated desktop input, and never changes the normal character controller.
class CF_ActionInputCharacterController : SCR_CharacterControllerComponent
{
	protected CF_ActionInputProbeComponent m_ActionProbe;
	protected Vehicle m_ActionTruck;
	protected World m_ActionWorld;
	protected bool m_bOwnThrust;
	protected bool m_bActionBound;

	bool CF_BindActionProbe(CF_ActionInputProbeComponent probe, Vehicle truck)
	{
		if (m_bActionBound || !probe || !truck || !GetGame())
			return false;
		m_bActionBound = true;
		m_ActionProbe = probe;
		m_ActionTruck = truck;
		m_ActionWorld = GetGame().GetWorld();
		return true;
	}

	void CF_DetachActionProbe(CF_ActionInputProbeComponent probe)
	{
		if (m_ActionProbe == probe)
			m_ActionProbe = null;
		// A pending value is neutralized only in a subsequent callback with
		// the original local pilot. Never retain/use an ActionManager elsewhere.
	}

	bool CF_IsExactActionPilot(IEntity owner, bool player)
	{
		if (!m_bActionBound || !player || owner != GetOwner() || !GetGame() ||
			!GetGame().InPlayMode() || GetGame().GetWorld() != m_ActionWorld ||
			!m_ActionWorld || CF_ConvoySession.CF_IsWorldCleanup() ||
			System.IsConsoleApp() || RplSession.Mode() != RplMode.None || !Replication.IsServer())
			return false;
		PlayerController local = GetGame().GetPlayerController();
		ChimeraCharacter character = ChimeraCharacter.Cast(owner);
		if (!local || local.GetControlledEntity() != owner || !character || !m_ActionTruck)
			return false;
		PlayerManager players = GetGame().GetPlayerManager();
		if (!players || players.GetPlayerIdFromControlledEntity(owner) != local.GetPlayerId() || local.GetPlayerId() <= 0)
			return false;
		AIControlComponent ai = character.GetAIControlComponent();
		CompartmentAccessComponent access = character.GetCompartmentAccessComponent();
		if (!ai || ai.IsAIActivated() || !access || access.IsGettingIn() || access.IsGettingOut())
			return false;
		BaseCompartmentSlot seat = access.GetCompartment();
		CarControllerComponent car = CarControllerComponent.Cast(m_ActionTruck.FindComponent(CarControllerComponent));
		return seat && seat.IsPiloting() && seat.GetOccupant() == owner && car &&
			car.GetPilotCompartmentSlot() == seat && access.GetVehicleIn(character) == m_ActionTruck;
	}

	override void OnPrepareControls(IEntity owner, ActionManager am, float dt, bool player)
	{
		float before;
		if (am)
			before = am.GetActionValue("CarThrust");
		float requested = -1;
		if (m_ActionProbe)
			requested = m_ActionProbe.CF_BeforeActionControls(this, owner, am, dt, player);
		bool exact = CF_IsExactActionPilot(owner, player);
		bool wrote;
		bool neutralCleanup;
		if (exact && am && requested >= 0)
		{
			am.SetActionValue("CarThrust", requested);
			m_bOwnThrust = requested > 0;
			wrote = true;
		}
		else if (m_bOwnThrust)
		{
			if (exact && am)
			{
				am.SetActionValue("CarThrust", 0);
				wrote = true;
				neutralCleanup = true;
				requested = 0;
			}
			// Ownership loss cannot authorize a write to another pilot's input.
			Print("[ConvoyFollower] ACTION_INPUT_NEUTRAL_CLEANUP: wrote=" + neutralCleanup +
				" exact_original_pilot=" + exact + " action_manager_from_callback=true");
			m_bOwnThrust = false;
		}
		float afterSet;
		if (am)
			afterSet = am.GetActionValue("CarThrust");
		super.OnPrepareControls(owner, am, dt, player);
		if (m_ActionProbe)
			m_ActionProbe.CF_AfterActionControls(this, owner, am, player, before, requested, afterSet, wrote);
	}

	override protected void OnApplyControls(IEntity owner, float timeSlice)
	{
		super.OnApplyControls(owner, timeSlice);
		if (m_ActionProbe)
			m_ActionProbe.CF_ObserveAppliedControls(owner);
	}
}
