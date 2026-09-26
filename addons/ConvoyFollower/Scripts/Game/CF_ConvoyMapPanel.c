// The normal map supplies the cursor and restores gameplay input on close.
class CF_ConvoyPanelOrder
{
	static const int FOLLOW = 1;
	static const int HOLD = 2;
	static const int WAIT_AHEAD = 3;
	static const int PULL_REAR = 4;
	static const int RESUME_AHEAD_LINE = 5;
	static const int CANCEL_UNLOAD = 6;
}

class CF_ConvoyMapPanel : ScriptedWidgetEventHandler
{
	protected ref Widget m_Root;
	protected CanvasWidget m_Backdrop;
	protected ref array<ref CanvasWidgetCommand> m_DrawCommands;
	protected SCR_PlayerController m_Controller;
	protected ref array<TextWidget> m_Rows = {};
	protected ref array<Widget> m_RowButtons = {};
	protected ref array<int> m_UnitIds = {};
	protected int m_iSelectedIdentity;
	protected TextWidget m_SelectedLabel;
	protected Widget m_WaitAheadButton;
	protected Widget m_PullRearButton;
	protected Widget m_ResumeAheadButton;
	protected TextWidget m_Feedback;

	void CF_ConvoyMapPanel(SCR_PlayerController controller)
	{
		m_Controller = controller;
	}

	void Open()
	{
		if (m_Root || !GetGame())
			return;
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		int x = workspace.GetWidth() - 440;
		if (x < 24)
			x = 24;
		m_Root = workspace.CreateWidgetInWorkspace(WidgetType.FrameWidgetTypeID, x, 75, 416, 644,
			WidgetFlags.VISIBLE, Color.FromRGBA(255, 255, 255, 255), 2000);
		if (!m_Root)
			return;
		m_Root.SetName("CF_ConvoyPanel");
		// A PanelWidget does not paint its color. Draw actual filled polygons so the
		// controls remain readable above the map's bright terrain and grid.
		m_Backdrop = CanvasWidget.Cast(workspace.CreateWidget(WidgetType.CanvasWidgetTypeID,
			WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR, Color.White, 0, m_Root));
		if (m_Backdrop)
		{
			FrameSlot.SetPos(m_Backdrop, 0, 0);
			FrameSlot.SetSize(m_Backdrop, 416, 644);
			PaintBackground();
		}
		MakeLabel(workspace, "CONVOY", 18, 12, 382, 40, 28);
		MakeButton(workspace, "CF_SelectAll", "ALL UNITS", 18, 54, 130, 32);
		m_SelectedLabel = MakeLabel(workspace, "Selected: all units", 158, 56, 238, 29, 15);
		for (int i = 0; i < 5; i++)
		{
			Widget rowButton = MakeButton(workspace, "CF_Row_" + i, "", 18, 94 + i * 48, 380, 46);
			m_RowButtons.Insert(rowButton);
			m_UnitIds.Insert(0);
			TextWidget row = MakeLabel(workspace, "", 18, 94 + i * 48, 380, 46, 15);
			m_Rows.Insert(row);
		}
		m_Feedback = MakeLabel(workspace, "Loading convoy state from server...", 18, 342, 380, 47, 15);
		MakeButton(workspace, "CF_Follow", "RESUME ALL", 18, 410, 184, 44);
		MakeButton(workspace, "CF_Hold", "HOLD ALL SEATED", 210, 410, 184, 44);
		m_WaitAheadButton = MakeButton(workspace, "CF_WaitAhead", "PULL AHEAD AND WAIT", 18, 464, 184, 44);
		m_PullRearButton = MakeButton(workspace, "CF_PullRear", "PULL OFF / REGROUP", 210, 464, 184, 44);
		m_ResumeAheadButton = MakeButton(workspace, "CF_ResumeAhead", "RESUME AHEAD LINE AFTER PASSING", 18, 518, 376, 42);
		MakeButton(workspace, "CF_CancelUnload", "CANCEL UNLOAD / RESUME OUTBOUND", 18, 570, 376, 42);
		MakeButton(workspace, "CF_Close", "CLOSE MAP", 274, 12, 120, 38);
		UpdateSelection();
		SetRoster("");
	}

	protected TextWidget MakeLabel(WorkspaceWidget workspace, string value, float x, float y, float w, float h, int fontSize)
	{
		TextWidget label = TextWidget.Cast(workspace.CreateWidget(WidgetType.TextWidgetTypeID,
			WidgetFlags.VISIBLE | WidgetFlags.NO_LOCALIZATION | WidgetFlags.IGNORE_CURSOR,
			Color.FromRGBA(234, 239, 231, 255), 0, m_Root));
		if (!label)
			return null;
		PlaceInPanel(workspace, label, x, y, w, h);
		label.SetText(value);
		label.SetExactFontSize((int)Math.Round(workspace.DPIUnscale(fontSize)));
		label.SetTextWrapping(true);
		return label;
	}

	// The workspace root and CanvasWidget draw commands use physical pixels,
	// while FrameSlot and font sizes use reference-resolution units. Convert
	// every child rect to keep clicks and text aligned with painted controls.
	protected void PlaceInPanel(WorkspaceWidget workspace, Widget widget, float x, float y, float w, float h)
	{
		FrameSlot.SetPos(widget, workspace.DPIUnscale(x), workspace.DPIUnscale(y));
		FrameSlot.SetSize(widget, workspace.DPIUnscale(w), workspace.DPIUnscale(h));
	}

	protected Widget MakeButton(WorkspaceWidget workspace, string name, string label, float x, float y, float w, float h)
	{
		Widget button = workspace.CreateWidget(WidgetType.ButtonWidgetTypeID, WidgetFlags.VISIBLE,
			Color.FromRGBA(49, 65, 69, 255), 0, m_Root);
		if (!button)
			return null;
		button.SetName(name);
		PlaceInPanel(workspace, button, x, y, w, h);
		button.AddHandler(this);
		TextWidget text = TextWidget.Cast(workspace.CreateWidget(WidgetType.TextWidgetTypeID,
			WidgetFlags.VISIBLE | WidgetFlags.NO_LOCALIZATION | WidgetFlags.CENTER | WidgetFlags.VCENTER | WidgetFlags.IGNORE_CURSOR,
			Color.FromRGBA(255, 255, 255, 255), 0, button));
		if (text)
		{
			PlaceInPanel(workspace, text, 0, 0, w, h);
			text.SetText(label);
			text.SetExactFontSize((int)Math.Round(workspace.DPIUnscale(14)));
		}
		return button;
	}

	protected void AddRect(float x, float y, float w, float h, Color color)
	{
		ref PolygonDrawCommand rectangle = new PolygonDrawCommand();
		rectangle.m_Vertices = {x, y, x + w, y, x + w, y + h, x, y + h};
		rectangle.m_iColor = color.PackToInt();
		m_DrawCommands.Insert(rectangle);
	}

	protected void PaintBackground()
	{
		if (!m_Backdrop)
			return;
		m_DrawCommands = {};
		AddRect(0, 0, 416, 644, Color.FromRGBA(18, 27, 32, 246));
		AddRect(0, 0, 416, 48, Color.FromRGBA(30, 47, 51, 255));
		AddRect(18, 54, 130, 32, Color.FromRGBA(49, 65, 69, 255));
		for (int i = 0; i < 5; i++)
		{
			if (m_UnitIds.Count() > i && m_UnitIds[i] == m_iSelectedIdentity && m_iSelectedIdentity > 0)
				AddRect(18, 94 + i * 48, 380, 46, Color.FromRGBA(61, 102, 92, 255));
			else
				AddRect(18, 94 + i * 48, 380, 46, Color.FromRGBA(37, 52, 56, 255));
		}
		AddRect(18, 338, 380, 66, Color.FromRGBA(36, 54, 58, 255));
		AddRect(18, 410, 184, 44, Color.FromRGBA(58, 84, 78, 255));
		AddRect(210, 410, 184, 44, Color.FromRGBA(58, 84, 78, 255));
		AddRect(18, 464, 184, 44, Color.FromRGBA(54, 69, 73, 255));
		AddRect(210, 464, 184, 44, Color.FromRGBA(54, 69, 73, 255));
		AddRect(18, 518, 376, 42, Color.FromRGBA(54, 69, 73, 255));
		AddRect(18, 570, 376, 42, Color.FromRGBA(54, 69, 73, 255));
		AddRect(274, 12, 120, 38, Color.FromRGBA(54, 69, 73, 255));
		m_Backdrop.SetDrawCommands(m_DrawCommands);
	}

	void SetRoster(string snapshot)
	{
		ref array<string> rows = {};
		if (!snapshot.IsEmpty())
			snapshot.Split(";", rows, false);
		bool selectedStillPresent = m_iSelectedIdentity == 0;
		for (int i = 0; i < m_Rows.Count(); i++)
		{
			if (!m_Rows[i])
				continue;
			if (i < rows.Count())
			{
				ref array<string> fields = {};
				rows[i].Split("|", fields, false);
				if (fields.Count() >= 5)
				{
					m_UnitIds[i] = fields[0].ToInt();
					if (m_UnitIds[i] == m_iSelectedIdentity)
						selectedStillPresent = true;
					m_Rows[i].SetText("Unit " + fields[0] + "  |  position " + fields[1] + "  |  " + fields[2] +
						"\n" + fields[3] + "  |  target: " + fields[4]);
				}
				else
				{
					m_UnitIds[i] = 0;
					m_Rows[i].SetText("Convoy data updating...");
				}
			}
			else if (i == 0 && rows.IsEmpty())
			{
				m_UnitIds[i] = 0;
				m_Rows[i].SetText("Waiting for server roster...");
			}
			else
			{
				m_UnitIds[i] = 0;
				m_Rows[i].SetText("");
			}
		}
		if (!selectedStillPresent)
			m_iSelectedIdentity = 0;
		UpdateSelection();
	}

	protected void UpdateSelection()
	{
		if (m_SelectedLabel)
		{
			if (m_iSelectedIdentity > 0)
				m_SelectedLabel.SetText("Selected: Unit " + m_iSelectedIdentity);
			else
				m_SelectedLabel.SetText("Selected: all units");
		}
		for (int i = 0; i < m_RowButtons.Count(); i++)
		{
			if (!m_RowButtons[i])
				continue;
			if (m_UnitIds[i] == 0)
				m_RowButtons[i].SetVisible(false);
			else
			{
				m_RowButtons[i].SetVisible(true);
				if (m_UnitIds[i] == m_iSelectedIdentity)
					m_RowButtons[i].SetColor(Color.FromRGBA(71, 111, 101, 255));
				else
					m_RowButtons[i].SetColor(Color.FromRGBA(49, 65, 69, 255));
			}
		}
		if (m_WaitAheadButton)
			m_WaitAheadButton.SetEnabled(m_iSelectedIdentity > 0);
		if (m_PullRearButton)
			m_PullRearButton.SetEnabled(m_iSelectedIdentity > 0);
		PaintBackground();
	}

	void SetFeedback(string message)
	{
		if (m_Feedback)
			m_Feedback.SetText(message);
	}

	void Close()
	{
		if (m_Root)
			m_Root.RemoveFromHierarchy();
		m_Root = null;
		m_Backdrop = null;
		m_DrawCommands = null;
		m_Rows.Clear();
		m_RowButtons.Clear();
		m_UnitIds.Clear();
		m_SelectedLabel = null;
		m_WaitAheadButton = null;
		m_PullRearButton = null;
		m_ResumeAheadButton = null;
		m_Feedback = null;
	}

	bool IsOpen()
	{
		return m_Root != null;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!w)
			return false;
		string name = w.GetName();
		if (name == "CF_Close")
		{
			// Close the native map menu, which restores gameplay input. Merely
			// hiding this overlay would leave the player's map cursor active.
			GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.MapMenu);
			return true;
		}
		if (name == "CF_SelectAll")
		{
			m_iSelectedIdentity = 0;
			UpdateSelection();
			return true;
		}
		for (int i = 0; i < m_RowButtons.Count(); i++)
		{
			if (name == "CF_Row_" + i && m_UnitIds[i] > 0)
			{
				m_iSelectedIdentity = m_UnitIds[i];
				UpdateSelection();
				return true;
			}
		}
		if (!m_Controller)
			return false;
		int command;
		int targetIdentity;
		if (name == "CF_Follow")
			command = CF_ConvoyPanelOrder.FOLLOW;
		else if (name == "CF_Hold")
			command = CF_ConvoyPanelOrder.HOLD;
		else if (name == "CF_WaitAhead")
		{
			command = CF_ConvoyPanelOrder.WAIT_AHEAD;
			targetIdentity = m_iSelectedIdentity;
		}
		else if (name == "CF_PullRear")
		{
			command = CF_ConvoyPanelOrder.PULL_REAR;
			targetIdentity = m_iSelectedIdentity;
		}
		else if (name == "CF_ResumeAhead")
			command = CF_ConvoyPanelOrder.RESUME_AHEAD_LINE;
		else if (name == "CF_CancelUnload")
			command = CF_ConvoyPanelOrder.CANCEL_UNLOAD;
		else
			return false;
		SetFeedback("Order sent. Waiting for server...");
		Print("[ConvoyFollower] PANEL_ORDER_REQUESTED: " + command + " unit " + targetIdentity);
		m_Controller.CF_SubmitPanelOrder(command, targetIdentity);
		return true;
	}
}

modded class SCR_MapEntity
{
	protected ref CF_ConvoyMapPanel m_CFPanel;

	override protected void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		GetGame().GetCallqueue().CallLater(CF_OpenConvoyPanel, 20, false);
	}

	override protected void OnMapClose()
	{
		GetGame().GetCallqueue().Remove(CF_OpenConvoyPanel);
		GetGame().GetCallqueue().Remove(CF_PollConvoyPanel);
		if (m_CFPanel)
			m_CFPanel.Close();
		m_CFPanel = null;
		super.OnMapClose();
	}

	protected void CF_OpenConvoyPanel()
	{
		if (System.IsConsoleApp())
			return;
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller || !controller.CF_HasActiveConvoy())
			return;
		if (m_CFPanel)
			m_CFPanel.Close();
		m_CFPanel = new CF_ConvoyMapPanel(controller);
		m_CFPanel.Open();
		Print("[ConvoyFollower] PANEL_OPEN: map overlay");
		CF_PollConvoyPanel();
	}

	protected void CF_PollConvoyPanel()
	{
		if (!m_CFPanel || !m_CFPanel.IsOpen())
			return;
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (controller)
			controller.CF_RequestPanelSnapshot();
		GetGame().GetCallqueue().CallLater(CF_PollConvoyPanel, 1000, false);
	}

	void CF_SetPanelSnapshot(string snapshot)
	{
		if (m_CFPanel)
			m_CFPanel.SetRoster(snapshot);
	}

	void CF_SetPanelFeedback(string message)
	{
		if (m_CFPanel)
			m_CFPanel.SetFeedback(message);
	}
}
