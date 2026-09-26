// One low-priority, controller-owned staging action. Native danger reactions
// remain eligible; this action never changes vehicle controls or other actions.
class CF_InitialDepartureWait : SCR_AIWaitBehavior
{
	protected ChimeraCharacter m_DepartureDriver;
	protected Vehicle m_DepartureTruck;
	protected bool m_bDepartureLeaseActive;

	void CF_InitialDepartureWait(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity)
	{
		// The native base constructors bind utility and the Wait behavior tree.
	}

	void Bind(ChimeraCharacter driver, Vehicle truck)
	{
		m_DepartureDriver = driver;
		m_DepartureTruck = truck;
		m_bDepartureLeaseActive = true;
		SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_NORMAL);
	}

	protected bool HasOriginalAIPilot()
	{
		if (!GetGame() || !m_DepartureDriver || !m_DepartureTruck || !m_Utility)
			return false;
		PlayerManager players = GetGame().GetPlayerManager();
		if (!players || players.GetPlayerIdFromControlledEntity(m_DepartureDriver) > 0 ||
			players.GetPlayerIdFromControlledEntity(m_DepartureTruck) > 0)
			return false;
		CompartmentAccessComponent access = m_DepartureDriver.GetCompartmentAccessComponent();
		CarControllerComponent car = CarControllerComponent.Cast(m_DepartureTruck.FindComponent(CarControllerComponent));
		BaseCompartmentSlot slot;
		if (access)
			slot = access.GetCompartment();
		return slot && car && slot.IsPiloting() && slot.GetOccupant() == m_DepartureDriver &&
			car.GetPilotCompartmentSlot() == slot && access.GetVehicleIn(m_DepartureDriver) == m_DepartureTruck &&
			!access.IsGettingIn() && !access.IsGettingOut() &&
			m_Utility.GetOwner() && m_Utility.GetOwner().GetControlledEntity() == m_DepartureDriver;
	}

	bool IsUsable()
	{
		return m_bDepartureLeaseActive && GetActionState() != EAIActionState.COMPLETED &&
			GetActionState() != EAIActionState.FAILED && HasOriginalAIPilot();
	}

	// Late possession/cleanup only invalidates our script lease. If the AI
	// evaluates it again, it retires itself instead of becoming an orphan Wait.
	void Retire(bool allowNativeCompletion)
	{
		m_bDepartureLeaseActive = false;
		if (allowNativeCompletion && !CF_ConvoySession.CF_IsWorldCleanup() && HasOriginalAIPilot())
			Complete();
	}

	override float CustomEvaluate()
	{
		if (CF_ConvoySession.CF_IsWorldCleanup())
			return 0;
		if (!m_bDepartureLeaseActive || !HasOriginalAIPilot())
		{
			Complete();
			return 0;
		}
		return GetPriority();
	}
}
