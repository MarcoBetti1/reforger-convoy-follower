// Own entries and state, using the game's normal HUD radial and input lifecycle.
// Opening this menu never needs the map or transfers ownership of a convoy.
class CF_ConvoyCommandEntry : SCR_SelectionMenuEntry
{
	int Command;
	int UnitIdentity;
}

class CF_ConvoyCommandMenu : SCR_RadialMenu
{
	protected SCR_PlayerController m_CFController;
	protected ref SCR_RadialMenuControllerInputs m_CFControls;
	protected string m_sCFRoster;
	protected string m_sCFOrderState;
	protected ref array<ref SCR_SelectionMenuCategoryEntry> m_CFTruckCategories = {};
	protected ref array<int> m_CFTruckIdentities = {};
	protected ref array<ref CF_ConvoyCommandEntry> m_CFTruckCommands = {};
	protected ref array<ref CF_ConvoyCommandEntry> m_CFBayCommands = {};
	protected string m_sCFBaySetupReason = "waiting for server eligibility";
	protected string m_sCFBayAdmitReason = "waiting for server eligibility";
	protected bool m_bCFExplicitBay;

	void CF_ConvoyCommandMenu(SCR_PlayerController controller)
	{
		m_CFController = controller;
		SCR_RadialMenuInputs inputs = new SCR_RadialMenuInputs();
		inputs.m_sContext = "RadialMenuContext";
		inputs.m_sBackAction = "RadialBack";
		inputs.m_sPerformAction = "RadialConfirm";
		inputs.m_fMouseSelectionTreshold = 100;
		inputs.m_iContextDeactivationTime = 150;
		inputs.m_bDynamicMouseTreshold = true;
		inputs.m_fGamepadSelectionTreshhold = 0.2;
		inputs.m_iGamepadDeselectionDelay = 150;
		m_Inputs = inputs;
		m_CFControls = new SCR_RadialMenuControllerInputs();
		m_CFControls.m_bCloseOnReleaseOpen = false;
		m_CFControls.m_bPerformOnClose = false;
		m_CFControls.m_bCloseOnPerform = true;
		m_CFControls.m_bOpenInRoot = true;
		m_CFControls.m_bUseRightStick = true;
		m_CFControls.m_bDeselectInCenter = true;
		m_CFControls.m_bUseLargeSize = true;
		m_CFControls.m_fCustomSize = -1;
		m_CFControls.m_bShowInnerBackground = true;
		m_sOpenSound = "SOUND_INV_HOTKEY_OPEN";
		m_sCloseSound = "SOUND_INV_HOTKEY_CLOSE";
		m_sSelectionSound = "SOUND_FE_BUTTON_HOVER";
		m_sPerformSound = "SOUND_INV_HOTKEY_CONFIRM";
		m_sEnterCategorySound = "SOUND_INV_HOTKEY_OPEN";
		m_sLeaveCategorySound = "SOUND_INV_HOTKEY_CLOSE";
		GetOnPerform().Insert(CF_PerformCommand);
		GetOnClose().Insert(CF_OnMenuClosed);
	}

	bool CF_Open()
	{
		if (!m_CFController || !m_CFController.GetControlledEntity() ||
			!m_CFController.CF_HasActiveConvoy() || GetGame().GetMenuManager().IsAnyMenuOpen())
			return false;
		SCR_RadialMenu opened = SCR_RadialMenu.GetOpenedRadialMenu();
		if (opened && opened != this)
			return false;
		if (IsOpened())
		{
			Close();
			return true;
		}
		SetController(m_CFController.GetControlledEntity(), m_CFControls);
		CF_BuildEntries();
		SetMenuDisplay();
		if (!m_Display)
			return false;
		Open();
		m_CFController.CF_RequestPanelSnapshot();
		Print("[ConvoyFollower] COMMAND_MENU_OPEN: native radial, owner convoy");
		return IsOpened();
	}

	protected void CF_OnMenuClosed(SCR_SelectionMenu menu)
	{
		// Restore after all close listeners, including the HUD, received this
		// event. Rebinding inside the invoker would remove the HUD listener
		// before it hides this menu.
		GetGame().GetCallqueue().CallLater(CF_RestoreSharedDisplay, 0, false);
		Print("[ConvoyFollower] COMMAND_MENU_CLOSED: no order on cancellation");
	}

	protected void CF_RestoreSharedDisplay()
	{
		if (!GetGame() || CF_ConvoySession.CF_IsWorldCleanup() || IsOpened() || SCR_RadialMenu.GetOpenedRadialMenu()) return;
		SCR_RadialMenu stock = SCR_RadialMenu.GlobalRadialMenu();
		if (stock && stock != this && m_Display)
			stock.SetMenuDisplay(m_Display);
	}

	void CF_Dispose()
	{
		if (!GetGame()) return;
		GetGame().GetCallqueue().Remove(CF_RestoreSharedDisplay);
		GetGame().GetCallqueue().Remove(ReleaseContext);
		GetGame().GetCallqueue().Remove(AllowClosing);
		GetGame().GetCallqueue().Remove(UpdateEntries);
		RemoveActionListeners();
		if (m_OpenedRadialMenu == this) m_OpenedRadialMenu = null;
		m_bOpened = false;
		m_bActivateContext = false;
		m_CFController = null;
	}

	protected CF_ConvoyCommandEntry CF_NewCommand(int command, int identity, string name, string description, string reason = "ready")
	{
		CF_ConvoyCommandEntry entry = new CF_ConvoyCommandEntry();
		entry.Command = command;
		entry.UnitIdentity = identity;
		entry.SetName(name);
		entry.SetDescription(description);
		entry.SetIconFromDeafaultImageSet("dotsMenu");
		entry.Enable(reason == "ready");
		if (reason != "ready") entry.SetDescription("Unavailable: " + reason);
		return entry;
	}

	protected void CF_BuildEntries()
	{
		ref array<ref SCR_SelectionMenuEntry> entries = {};
		m_CFTruckCategories.Clear();
		m_CFTruckIdentities.Clear();
		m_CFTruckCommands.Clear();
		m_CFBayCommands.Clear();
		entries.Insert(CF_NewCommand(CF_ConvoyPanelOrder.HOLD, 0, "Hold all trucks", "Whole convoy: approach the stopped lead, then remain seated. " + m_sCFOrderState));
		entries.Insert(CF_NewCommand(CF_ConvoyPanelOrder.FOLLOW, 0, "Resume all trucks", "Whole convoy: follow the original lead in predecessor order. " + m_sCFOrderState));
		ref array<string> rows = {};
		m_sCFRoster.Split(";", rows, false);
		foreach (string row : rows)
		{
			ref array<string> fields = {};
			row.Split("|", fields, false);
			if (fields.Count() < 5) continue;
			int identity = fields[0].ToInt();
			CF_ReadBayMenuState(fields);
			SCR_SelectionMenuCategoryEntry truck = new SCR_SelectionMenuCategoryEntry();
			truck.SetName("Unit " + identity);
			truck.SetDescription(CF_TruckDescription(fields));
			truck.SetIconFromDeafaultImageSet("dotsMenu");
			truck.Enable(true);
			truck.AddEntry(CF_NewCommand(CF_ConvoyPanelOrder.PULL_REAR, identity, "Pull off / regroup", "Selected Unit " + identity + ": drive to the rear return line.", CF_Field(fields, 8, "waiting for server eligibility")));
			truck.AddEntry(CF_NewCommand(CF_ConvoyPanelOrder.WAIT_AHEAD, identity, "Pull ahead / wait", "Selected Unit " + identity + ": drive ahead and remain seated.", CF_Field(fields, 9, "waiting for server eligibility")));
			truck.AddEntry(CF_NewCommand(CF_ConvoyPanelOrder.REBOARD_SELECTED, identity, "Reboard original truck", "Selected Unit " + identity + ": retry boarding; preserve Hold.", CF_Field(fields, 5, "select an active held driver")));
			truck.AddEntry(CF_NewCommand(CF_ConvoyPanelOrder.REGROUP_RETURN, identity, "Regroup for return", "Return line: regroup behind your original lead. You drive home.", CF_Field(fields, 10, "truck is not parked in the return line")));
			foreach (SCR_SelectionMenuEntry child : truck.GetEntries())
				m_CFTruckCommands.Insert(CF_ConvoyCommandEntry.Cast(child));
			m_CFTruckCategories.Insert(truck);
			m_CFTruckIdentities.Insert(identity);
			entries.Insert(truck);
		}
		SCR_SelectionMenuCategoryEntry options = new SCR_SelectionMenuCategoryEntry();
		options.SetName("Maneuver options");
		options.SetDescription("Whole convoy or waiting line commands. " + m_sCFOrderState);
		options.SetIconFromDeafaultImageSet("dotsMenu");
		options.Enable(true);
		string cancelName = "Cancel unload / follow";
		string cancelDescription = "End the unload maneuver and resume outbound following.";
		if (m_bCFExplicitBay) { cancelName = "Cancel bay / keep held"; cancelDescription = "After the trucks stop, cancel the bay while keeping the original drivers and cargo held. Resume all is a separate order."; }
		CF_ConvoyCommandEntry cancel = CF_NewCommand(CF_ConvoyPanelOrder.CANCEL_UNLOAD, 0, cancelName, cancelDescription);
		m_CFBayCommands.Insert(cancel);
		options.AddEntry(cancel);
		options.AddEntry(CF_NewCommand(CF_ConvoyPanelOrder.RESUME_AHEAD_LINE, 0, "Resume ahead line", "After passing the waiting trucks, bring that line back into following."));
		entries.Insert(options);
		SCR_SelectionMenuCategoryEntry bay = new SCR_SelectionMenuCategoryEntry();
		bay.SetName("Unload bay");
		bay.SetDescription("You unload your lead and drive clear. Admit one original truck, unload it, command parking, then repeat. " + m_sCFOrderState);
		bay.SetIconFromDeafaultImageSet("dotsMenu");
		bay.Enable(true);
		CF_ConvoyCommandEntry setup = CF_NewCommand(CF_ConvoyPanelOrder.SET_UNLOAD_BAY, 0, "Set bay / Hold queue", "Record your stopped original lead's position as the bay. Wait for all trucks to Hold before unloading your lead.", m_sCFBaySetupReason);
		CF_ConvoyCommandEntry admit = CF_NewCommand(CF_ConvoyPanelOrder.ADMIT_NEXT, 0, "Admit next truck", "After unloading and driving clear, admit only the front original truck. The server checks the queue and actual vehicle bounds.", m_sCFBayAdmitReason);
		m_CFBayCommands.Insert(setup);
		m_CFBayCommands.Insert(admit);
		bay.AddEntry(setup);
		bay.AddEntry(admit);
		entries.Insert(bay);
		AddEntries(entries, true);
	}

	protected string CF_Field(array<string> fields, int index, string fallback)
	{
		if (fields.Count() > index && !fields[index].IsEmpty()) return fields[index];
		return fallback;
	}

	protected void CF_ReadBayMenuState(array<string> fields)
	{
		m_sCFBaySetupReason = CF_Field(fields, 11, "waiting for server eligibility");
		m_sCFBayAdmitReason = CF_Field(fields, 12, "waiting for server eligibility");
		m_bCFExplicitBay = CF_Field(fields, 13, "false") == "true";
	}

	protected string CF_TruckDescription(array<string> fields)
	{
		string description = WidgetManager.Translate(fields[2]) + " | " + fields[3] + " | follows " + fields[4];
		if (fields.Count() > 6) description += " | " + fields[6];
		if (fields.Count() > 7) description += "\n" + fields[7];
		if (fields.Count() > 14) description += "\n" + fields[14];
		return description + "\n" + m_sCFOrderState;
	}

	void CF_SetSnapshot(string roster, string orderState)
	{
		m_sCFRoster = roster;
		m_sCFOrderState = orderState;
		// Read availability even while closed, before building the next menu.
		ref array<string> statusRows = {};
		roster.Split(";", statusRows, false);
		if (!statusRows.IsEmpty())
		{
			ref array<string> statusFields = {};
			statusRows[0].Split("|", statusFields, false);
			CF_ReadBayMenuState(statusFields);
		}
		// Avoid replacing entries underneath a hovered category or selected
		// command. Update matching identities in place; a new roster is built
		// at the next opening. Server admission remains authoritative.
		if (!IsOpened()) return;
		if (m_CFTruckCategories.IsEmpty() && !roster.IsEmpty())
		{
			CF_BuildEntries();
			return;
		}
		ref array<string> rows = {};
		roster.Split(";", rows, false);
		foreach (string row : rows)
		{
			ref array<string> fields = {};
			row.Split("|", fields, false);
			if (fields.Count() < 5) continue;
			int identity = fields[0].ToInt();
			int index = m_CFTruckIdentities.Find(identity);
			if (index >= 0) m_CFTruckCategories[index].SetDescription(CF_TruckDescription(fields));
			foreach (CF_ConvoyCommandEntry entry : m_CFTruckCommands)
			{
				if (!entry || entry.UnitIdentity != identity) continue;
				int reasonIndex = 5;
				if (entry.Command == CF_ConvoyPanelOrder.PULL_REAR) reasonIndex = 8;
				else if (entry.Command == CF_ConvoyPanelOrder.WAIT_AHEAD) reasonIndex = 9;
				else if (entry.Command == CF_ConvoyPanelOrder.REGROUP_RETURN) reasonIndex = 10;
				string reason = CF_Field(fields, reasonIndex, "waiting for server eligibility");
				entry.Enable(reason == "ready");
				entry.SetDescription(orderState);
				if (reason != "ready") entry.SetDescription("Unavailable: " + reason);
			}
		}
		foreach (CF_ConvoyCommandEntry bayCommand : m_CFBayCommands)
		{
			string availability = "ready";
			if (bayCommand.Command == CF_ConvoyPanelOrder.SET_UNLOAD_BAY) availability = m_sCFBaySetupReason;
			else if (bayCommand.Command == CF_ConvoyPanelOrder.ADMIT_NEXT) availability = m_sCFBayAdmitReason;
			else if (bayCommand.Command == CF_ConvoyPanelOrder.CANCEL_UNLOAD)
			{
				bayCommand.SetName("Cancel unload / follow");
				if (m_bCFExplicitBay) bayCommand.SetName("Cancel bay / keep held");
			}
			bayCommand.Enable(availability == "ready");
			bayCommand.SetDescription(orderState);
			if (availability != "ready") bayCommand.SetDescription("Unavailable: " + availability);
		}
		UpdateEntries();
	}

	protected void CF_PerformCommand(SCR_SelectionMenu menu, SCR_SelectionMenuEntry selected)
	{
		CF_ConvoyCommandEntry entry = CF_ConvoyCommandEntry.Cast(selected);
		if (!entry || !m_CFController) return;
		m_CFController.CF_SubmitPanelOrder(entry.Command, entry.UnitIdentity);
	}
}

// A discoverable native interaction on either an owned driver or truck rear.
class CF_OpenConvoyCommandsAction : ScriptedUserAction
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Convoy commands";
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		return controller && controller.GetControlledEntity() == user && controller.CF_HasActiveConvoy();
	}

	override bool CanBePerformedScript(IEntity user)
	{
		return CanBeShownScript(user);
	}

	override bool HasLocalEffectOnlyScript() { return true; }
	override bool CanBroadcastScript() { return false; }

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (controller && controller.GetControlledEntity() == pUserEntity) controller.CF_OpenCommandMenu();
	}
}
