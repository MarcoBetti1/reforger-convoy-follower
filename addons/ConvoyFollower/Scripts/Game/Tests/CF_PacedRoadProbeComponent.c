// Opt-in controlled-speed variant of the existing open-road smoke course.
// Only its separate native lead is controlled here. Followers use production
// recruitment, engine preparation, navigation, cruise and arrival ownership.
class CF_PacedRoadTruckSample
{
	Vehicle Truck;
	ChimeraCharacter Pilot;
	CF_DriverControllerComponent Driver;
	SCR_AIGroup Group;
	vector StartPosition;
	vector LastPosition;
	vector LastForward;
	vector ObservationOrigin;
	float Path;
	float ForwardProgress;
	float MaxGap;
	float MaxDrift;
	int PoweredSamples;
	// Weak observer handle: the fixture never owns or changes this native action.
	SCR_AIPilotMoveFromIncomingVehicleBehavior LastDangerBehavior;
	bool HadDangerBehavior;
	int DangerSequence;
	string LastDangerEntityKey = "none";
	vector LastDangerGoal;
	ref CF_PacedPathObserver PathObserver;
}

class CF_PacedRoadProbeComponentClass : CF_SmokeProbeComponentClass
{
}

class CF_PacedRoadProbeComponent : CF_SmokeProbeComponent
{
	[Attribute(defvalue: "", desc: "Exact isolated paced world resource, for report binding")]
	protected string m_sPacedWorld;
	[Attribute(defvalue: "0", desc: "Opt-in isolated offline client: request native exit 30 seconds after PACED_RESULT; never exits Workbench")]
	protected bool m_bPacedAutoExit;
	// Weak identity snapshots prevent a delayed callback closing another world.
	protected IEntity m_PacedExitOwner;
	protected World m_PacedExitWorld;
	protected string m_sPacedExitWorldFile;
	protected float m_fPacedTerminalMs;
	protected bool m_bPacedExitPending;
	protected bool m_bPacedExitRequested;
	protected int m_iPacedExitDeferrals;
	protected bool m_bPacedDeleting;
	protected ref array<ref CF_PacedRoadTruckSample> m_PacedTrucks = {};
	protected ref CF_NativeCruiseControl m_PacedCruise;
	protected ref SCR_AIWaitBehavior m_PacedWait;
	protected ChimeraCharacter m_PacedOwner;
	protected CF_ConvoySession m_PacedSession;
	protected int m_iPacedOwnerId;
	protected int m_iPacedTick;
	protected int m_iBarrierTicks;
	protected int m_iObservationSamples;
	protected float m_fPacedStartMs;
	protected float m_fPacedDriveStartMs;
	protected float m_fLastPathReadMs;
	protected float m_fObservationStartMs;
	protected float m_fLeadHoldStartMs;
	protected float m_fPacedPeakGap;
	protected string m_sPacedRun;
	protected string m_sPendingRouteResult;
	protected string m_sFirstFailure;
	protected bool m_bPacedStarted;
	protected bool m_bPacedHoldingLead;
	protected bool m_bPacedOwnBrake;
	protected bool m_bPacedObserving;
	protected bool m_bPacedFailed;
	protected bool m_bPacedSpacingFailed;
	protected bool m_bPacedDriftFailed;
	protected bool m_bPacedTerminal;
	protected bool m_bPacedPathEnded;
	protected static const float PACED_LEAD_KMH = 25.0;
	protected static const float PACED_MAX_LINK_M = 60.0;
	protected static const float PACED_MAX_DRIFT_M = 2.0;
	protected static const float PACED_OBSERVATION_MS = 180000.0;
	protected static const float PACED_TIMEOUT_MS = 480000.0;
	protected static const int PACED_EXIT_DELAY_MS = 30000;
	protected static const int PACED_EXIT_RETRY_MS = 100;
	protected static const int PACED_EXIT_MAX_DEFERRALS = 1;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_fPacedStartMs = GetGame().GetWorld().GetWorldTime();
		m_sPacedRun = EntityKey(owner) + "_" + m_fPacedStartMs;
		if (m_bPacedAutoExit)
		{
			m_PacedExitOwner = owner;
			m_PacedExitWorld = GetGame().GetWorld();
			m_sPacedExitWorldFile = GetGame().GetWorldFile();
		}
		super.OnPostInit(owner);
		CF_ConvoySettings settings = CF_ConvoySettings.Get();
		string initMessage = "[ConvoyFollower] PACED_INIT: run_id=" + m_sPacedRun + " world=" + m_sPacedWorld;
		initMessage += " expected=" + m_iExpectedTrucks + " lead_cap_kmh=25 peak_gap_m=60 observation_s=180 max_drift_m=2 timeout_s=480";
		initMessage += " fixture_change=controlled_native_lead_and_entry_barrier legacy_worlds_unchanged=true follower_writes=false";
		initMessage += " native_cruise=" + settings.m_bNativeCruiseEnabled;
		initMessage += " stable_follow_waypoints=" + settings.m_bStableFollowWaypoints;
		initMessage += " auto_exit=" + m_bPacedAutoExit + " auto_exit_delay_s=30";
		initMessage += " moving_gap=" + settings.m_fMovingGap + " stopped_gap=" + settings.m_fStoppedGap;
		initMessage += " completion_radius=" + settings.GetMoveCompletionRadius();
		Print(initMessage);
	}

	protected bool PacedWorldAlive()
	{
		return Replication.IsServer() && GetGame() && GetGame().GetWorld() && !CF_ConvoySession.CF_IsWorldCleanup();
	}

	protected string EntityKey(IEntity entity)
	{
		if (!entity)
			return "none";
		return entity.GetID().ToString();
	}

	protected float PacedSeconds()
	{
		return (GetGame().GetWorld().GetWorldTime() - m_fPacedStartMs) / 1000.0;
	}

	protected string ActionKey(AIActionBase action)
	{
		if (!action)
			return "none";
		return action.Type().ToString() + ":" + action.GetActionState();
	}

	protected bool EntityAlive(IEntity entity)
	{
		if (!entity)
			return false;
		SCR_DamageManagerComponent damage = SCR_DamageManagerComponent.Cast(entity.FindComponent(SCR_DamageManagerComponent));
		return !damage || !damage.IsDestroyed();
	}

	protected SCR_AIUtilityComponent PilotUtility(CF_PacedRoadTruckSample sample)
	{
		if (!sample || !sample.Pilot)
			return null;
		AIControlComponent control = sample.Pilot.GetAIControlComponent();
		AIAgent agent;
		if (control)
			agent = control.GetAIAgent();
		if (!agent || agent.GetControlledEntity() != sample.Pilot || agent.GetParentGroup() != sample.Group)
			return null;
		return SCR_AIUtilityComponent.Cast(agent.FindComponent(SCR_AIUtilityComponent));
	}

	protected SCR_AIGroupUtilityComponent GroupUtility(CF_PacedRoadTruckSample sample)
	{
		if (!sample || !sample.Group)
			return null;
		return SCR_AIGroupUtilityComponent.Cast(sample.Group.FindComponent(SCR_AIGroupUtilityComponent));
	}

	protected bool ExactPilot(CF_PacedRoadTruckSample sample, bool requireEntryComplete = true)
	{
		if (!sample || !EntityAlive(sample.Truck) || !EntityAlive(sample.Pilot))
			return false;
		CompartmentAccessComponent access = sample.Pilot.GetCompartmentAccessComponent();
		CarControllerComponent car = CarControllerComponent.Cast(sample.Truck.FindComponent(CarControllerComponent));
		BaseCompartmentSlot slot;
		if (access)
			slot = access.GetCompartment();
		return access && slot && slot.IsPiloting() && slot.GetOccupant() == sample.Pilot &&
			access.GetVehicleIn(sample.Pilot) == sample.Truck && car && car.GetPilotCompartmentSlot() == slot &&
			(!requireEntryComplete || (!access.IsGettingIn() && !access.IsGettingOut()));
	}

	protected bool ActiveSeatAction(AIActionBase action)
	{
		if (!action || action.GetActionState() == EAIActionState.COMPLETED || action.GetActionState() == EAIActionState.FAILED)
			return false;
		string typeName = action.Type().ToString();
		return typeName.Contains("GetIn") || typeName.Contains("GetOut");
	}

	// Read-only for every follower. Do not remove a pending boarding waypoint,
	// complete an action, or start an engine to make this barrier pass.
	protected bool EntryReady(CF_PacedRoadTruckSample sample)
	{
		SCR_AIUtilityComponent utility = PilotUtility(sample);
		SCR_AIGroupUtilityComponent groupUtility = GroupUtility(sample);
		if (!ExactPilot(sample) || !utility || !groupUtility)
			return false;
		array<AIWaypoint> waypoints = {};
		sample.Group.GetWaypoints(waypoints);
		foreach (AIWaypoint waypoint : waypoints)
		{
			if (SCR_BoardingEntityWaypoint.Cast(waypoint))
				return false;
		}
		if (ActiveSeatAction(utility.GetCurrentBehavior()) || ActiveSeatAction(groupUtility.GetCurrentAction()))
			return false;
		array<ref AIActionBase> actions = {};
		utility.GetActions(actions);
		foreach (AIActionBase action : actions)
		{
			if (ActiveSeatAction(action))
				return false;
		}
		return true;
	}

	protected void BindOriginals()
	{
		if (!m_PacedTrucks.IsEmpty() || !m_Pilot || !m_Player || m_Drivers.Count() != m_iExpectedTrucks)
			return;
		m_PacedOwner = m_Player;
		m_iPacedOwnerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(m_Player);
		m_PacedSession = CF_ConvoySession.GetForPlayer(m_Player);
		if (!m_PacedSession)
			return;
		for (int index = 0; index <= m_iExpectedTrucks; index++)
		{
			CF_PacedRoadTruckSample sample = new CF_PacedRoadTruckSample();
			if (index == 0)
			{
				sample.Truck = m_Lead;
				sample.Pilot = m_Pilot;
				sample.Group = m_PilotGroup;
			}
			else
			{
				sample.Truck = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokeFollower" + index));
				sample.Driver = m_Drivers[index - 1];
				sample.Pilot = ChimeraCharacter.Cast(sample.Driver.CF_GetDriverEntity());
				sample.Group = SCR_AIGroup.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokeGroup" + index));
			}
			if (sample.Truck)
			{
				sample.StartPosition = sample.Truck.GetOrigin();
				sample.LastPosition = sample.StartPosition;
				sample.LastForward = sample.Truck.GetWorldTransformAxis(2);
			}
			m_PacedTrucks.Insert(sample);
			IEntity predecessor;
			if (index > 0)
				predecessor = m_PacedTrucks[index - 1].Truck;
			Print("[ConvoyFollower] PACED_BIND: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick + " unit=" + index +
				" truck_id=" + EntityKey(sample.Truck) + " pilot_id=" + EntityKey(sample.Pilot) +
				" predecessor_id=" + EntityKey(predecessor) + " owner_id=" + EntityKey(m_PacedOwner) +
				" owner_player_id=" + m_iPacedOwnerId + " origin=" + sample.StartPosition);
		}
	}

	protected bool OwnerRetained()
	{
		if (!EntityAlive(m_PacedOwner) || m_Player != m_PacedOwner || !GetGame().GetPlayerManager() ||
			GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iPacedOwnerId) != m_PacedOwner ||
			!m_PacedSession || CF_ConvoySession.GetForPlayer(m_PacedOwner) != m_PacedSession)
			return false;
		CompartmentAccessComponent access = m_PacedOwner.GetCompartmentAccessComponent();
		BaseCompartmentSlot slot;
		if (access)
			slot = access.GetCompartment();
		return access && slot && !slot.IsPiloting() && slot.GetOccupant() == m_PacedOwner &&
			!access.IsGettingIn() && !access.IsGettingOut() && access.GetVehicleIn(m_PacedOwner) == m_Lead;
	}

	protected bool IdentityRetained(int index, bool requireSeat)
	{
		CF_PacedRoadTruckSample sample = m_PacedTrucks[index];
		if (!sample || !EntityAlive(sample.Truck) || !EntityAlive(sample.Pilot) || !PilotUtility(sample))
			return false;
		string name = "CF_SmokeLead";
		if (index > 0)
			name = "CF_SmokeFollower" + index;
		if (GetGame().GetWorld().FindEntityByName(name) != sample.Truck)
			return false;
		PlayerManager players = GetGame().GetPlayerManager();
		if (!players || players.GetPlayerIdFromControlledEntity(sample.Pilot) > 0 ||
			players.GetPlayerIdFromControlledEntity(sample.Truck) > 0)
			return false;
		if (index > 0 && (!m_PacedSession || !sample.Driver || sample.Driver.CF_GetDriverEntity() != sample.Pilot ||
			sample.Driver.CF_GetAssignedVehicle() != sample.Truck || sample.Driver.CF_GetUnitNumber() != index ||
			m_PacedSession.GetUnitNumber(sample.Driver) != index ||
			sample.Driver.CF_GetDiagnosticTargetVehicle() != m_PacedTrucks[index - 1].Truck))
			return false;
		return !requireSeat || ExactPilot(sample);
	}

	protected bool CanControlLead()
	{
		return PacedWorldAlive() && m_PacedTrucks.Count() == m_iExpectedTrucks + 1 &&
			m_PacedTrucks[0].Truck == m_Lead && m_PacedTrucks[0].Pilot == m_Pilot &&
			IdentityRetained(0, true);
	}

	protected void NoteFailure(string scope, string reason)
	{
		m_bPacedFailed = true;
		if (m_sFirstFailure.IsEmpty())
			m_sFirstFailure = scope + ":" + reason;
		Print("[ConvoyFollower] PACED_FAILURE: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
			" seconds=" + PacedSeconds() + " scope=" + scope + " reason=" + reason);
	}

	protected bool StartBarrier()
	{
		bool ready = m_PacedTrucks.Count() == m_iExpectedTrucks + 1 && OwnerRetained();
		for (int index = 0; index < m_PacedTrucks.Count(); index++)
		{
			CF_PacedRoadTruckSample sample = m_PacedTrucks[index];
			bool entryReady = IdentityRetained(index, true) && EntryReady(sample);
			CarControllerComponent car;
			if (sample.Truck)
				car = CarControllerComponent.Cast(sample.Truck.FindComponent(CarControllerComponent));
			bool engineReady = car && car.GetSimulation() && car.GetSimulation().EngineIsOn();
			if (!entryReady)
				ready = false;
			Print("[ConvoyFollower] PACED_START_BARRIER: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
				" unit=" + index + " entry_ready=" + entryReady + " engine_on=" + engineReady + " engine_gate=false");
		}
		if (!ready)
			return false;
		// The exact GetIn order has completed naturally. Forget its stale pointer
		// before inherited StartAIDrive removes a waypoint. No GetIn is failed.
		if (SCR_BoardingEntityWaypoint.Cast(m_PilotWaypoint))
			m_PilotWaypoint = null;
		if (!m_PacedCruise)
			m_PacedCruise = new CF_NativeCruiseControl();
		return m_PacedCruise.Request(m_Pilot, m_Lead, PACED_LEAD_KMH, "paced_fixture_lead_25");
	}

	override protected bool StartAIDrive()
	{
		if (!CanControlLead() || !m_PacedCruise || !m_PacedCruise.OwnsOverride() || !super.StartAIDrive())
			return false;
		m_bPacedStarted = true;
		m_fPacedDriveStartMs = GetGame().GetWorld().GetWorldTime();
		m_fLastPathReadMs = m_fPacedDriveStartMs - 200;
		foreach (CF_PacedRoadTruckSample sample : m_PacedTrucks)
		{
			sample.PathObserver = new CF_PacedPathObserver();
			sample.StartPosition = sample.Truck.GetOrigin();
			sample.LastPosition = sample.StartPosition;
			sample.LastForward = sample.Truck.GetWorldTransformAxis(2);
			sample.Path = 0;
			sample.ForwardProgress = 0;
			sample.PoweredSamples = 0;
		}
		Print("[ConvoyFollower] PACED_DRIVE_BEGIN: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
			" seconds=" + PacedSeconds() + " leg=1 start=" + m_Lead.GetOrigin() + " goal=" + m_vRoadGoal +
			" lead_cap_kmh=25 progress_basis=previous_vehicle_forward_per_sample fixture_follower_engine_writes=false");
		Print("[ConvoyFollower] PACED_PATH_INIT: run_id=" + m_sPacedRun +
			" drive_window_s=120 read_interval_ms=200 summary_interval_ms=1000 raw_points_per_path=128 raw_records_per_unit=3200" +
			" trigger=waypoint_action_brake_reverse_navlink_resolution phase=POSTFRAME native_actions_written=false");
		return true;
	}

	override protected void HoldLeadAtRoadGoal()
	{
		if (m_bPacedHoldingLead || !CanControlLead())
			return;
		SCR_AIUtilityComponent utility = PilotUtility(m_PacedTrucks[0]);
		SCR_AIGroupUtilityComponent groupUtility = GroupUtility(m_PacedTrucks[0]);
		if (!utility || !groupUtility)
			return;
		Print("[ConvoyFollower] PACED_LEAD_HOLD: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
			" phase=before_cancel behavior=" + ActionKey(utility.GetCurrentBehavior()) +
			" group_action=" + ActionKey(groupUtility.GetCurrentAction()));
		if (m_PilotWaypoint)
		{
			groupUtility.CancelActivitiesRelatedToWaypoint(m_PilotWaypoint, doNotCompleteWaypoint: true);
			m_PilotGroup.RemoveWaypoint(m_PilotWaypoint);
			m_PilotWaypoint = null;
		}
		// This group contains only the fixture's separate lead pilot. Cancel its
		// queued move children after pruning completed boarding, as in the proven
		// native-hold course; no follower utility/group is ever mutated.
		utility.RemoveObsoleteActions();
		utility.CancelAllGroupActivityBehaviors(groupUtility);
		m_PacedWait = new SCR_AIWaitBehavior(utility, null);
		m_PacedWait.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_GAMEMASTER + 100);
		utility.AddAction(m_PacedWait);
		m_bPacedHoldingLead = true;
		m_fLeadHoldStartMs = GetGame().GetWorld().GetWorldTime();
		m_bMaintainArrivalBrake = false;
		if (!m_PacedCruise.Request(m_Pilot, m_Lead, 0, "paced_fixture_lead_hold"))
			NoteFailure("fixture", "lead_zero_cruise_request_refused");
		CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
		if (car && car.GetSimulation())
		{
			car.SetPersistentHandBrake(true);
			car.GetSimulation().SetThrottle(0);
			car.GetSimulation().SetBreak(1, true);
			m_bPacedOwnBrake = true;
		}
		Print("[ConvoyFollower] PACED_LEAD_HOLD: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
			" phase=requested wait_owned=true requested_cruise_kmh=0 frame_brake_writes=false");
	}

	protected bool LeadWaitSelected()
	{
		if (!m_bPacedHoldingLead || !m_PacedWait || m_PacedTrucks.IsEmpty())
			return false;
		SCR_AIUtilityComponent utility = PilotUtility(m_PacedTrucks[0]);
		return utility && utility.GetCurrentBehavior() == m_PacedWait;
	}

	// Measure between poll boundaries too. This callback is read-only: native
	// lead Wait/cruise and production follower controllers own all movement.
	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (!PacedWorldAlive() || m_bPacedTerminal || m_PacedTrucks.IsEmpty())
			return;
		LogDangerTransitions();
		MeasurePeaks();
		LogPathDiagnostics();
	}

	protected void LogPathDiagnostics()
	{
		if (!m_bPacedStarted || m_bPacedPathEnded)
			return;
		float now = GetGame().GetWorld().GetWorldTime();
		float driveMs = now - m_fPacedDriveStartMs;
		if (driveMs > 120000)
		{
			m_bPacedPathEnded = true;
			for (int endIndex = 0; endIndex < m_PacedTrucks.Count(); endIndex++)
			{
				if (m_PacedTrucks[endIndex].PathObserver)
					m_PacedTrucks[endIndex].PathObserver.Finish(m_sPacedRun, endIndex, driveMs / 1000.0);
			}
			return;
		}
		if (now - m_fLastPathReadMs < 200)
			return;
		m_fLastPathReadMs = now;
		for (int index = 0; index < m_PacedTrucks.Count(); index++)
		{
			CF_PacedRoadTruckSample sample = m_PacedTrucks[index];
			if (sample.PathObserver)
				sample.PathObserver.Observe(sample.Truck, sample.Pilot, sample.Group, IdentityRetained(index, true),
					m_sPacedRun, index, m_iPacedTick, PacedSeconds(), driveMs);
		}
	}

	// Installed SCR_AIMoveFromDanger.c exposes these BT parameters publicly.
	// Read selection/parameters only; Evaluate and selection hooks mutate AI.
	// Check every POSTFRAME, but emit only enter/replacement/goal-change/exit.
	protected void LogDangerTransitions()
	{
		for (int index = 0; index < m_PacedTrucks.Count(); index++)
		{
			CF_PacedRoadTruckSample sample = m_PacedTrucks[index];
			SCR_AIUtilityComponent utility = PilotUtility(sample);
			AIActionBase current;
			if (utility)
				current = utility.GetCurrentBehavior();
			SCR_AIPilotMoveFromIncomingVehicleBehavior incoming = SCR_AIPilotMoveFromIncomingVehicleBehavior.Cast(current);
			IEntity danger;
			vector dangerPosition;
			vector escapeGoal;
			if (incoming)
			{
				if (incoming.m_DangerEntity)
					danger = incoming.m_DangerEntity.m_Value;
				if (incoming.m_DangerPosition)
					dangerPosition = incoming.m_DangerPosition.m_Value;
				if (incoming.m_vMovePos)
					escapeGoal = incoming.m_vMovePos.m_Value;
			}
			string dangerKey = EntityKey(danger);
			bool hadDanger = sample.HadDangerBehavior;
			bool hasDanger = incoming != null;
			bool changed = hadDanger != hasDanger;
			if (hasDanger && (incoming != sample.LastDangerBehavior || dangerKey != sample.LastDangerEntityKey ||
				vector.DistanceSq(escapeGoal, sample.LastDangerGoal) > 0.0001))
				changed = true;
			if (!changed)
				continue;
			string transition = "exit";
			if (hasDanger)
			{
				transition = "enter";
				if (hadDanger)
					transition = "changed";
			}
			sample.DangerSequence++;
			IEntity predecessor;
			if (sample.Driver)
				predecessor = sample.Driver.CF_GetDiagnosticTargetVehicle();
			Print("[ConvoyFollower] PACED_DANGER_TRANSITION: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
				" seconds=" + PacedSeconds() + " unit=" + index + " sequence=" + sample.DangerSequence +
				" transition=" + transition + " phase=POSTFRAME behavior=" + ActionKey(current) +
				" truck_id=" + EntityKey(sample.Truck) + " pilot_id=" + EntityKey(sample.Pilot) +
				" predecessor_id=" + EntityKey(predecessor) + " danger_id=" + dangerKey +
				" previous_danger_id=" + sample.LastDangerEntityKey + " captured_danger_position=" + dangerPosition +
				" escape_goal=" + escapeGoal + " previous_escape_goal=" + sample.LastDangerGoal +
				" source_is_predecessor=" + (danger && danger == predecessor) + " native_actions_written=false");
			LogDangerEntity(sample.Truck, index, sample.DangerSequence, "own");
			LogDangerEntity(predecessor, index, sample.DangerSequence, "predecessor");
			if (hasDanger)
				LogDangerEntity(danger, index, sample.DangerSequence, "danger");
			sample.LastDangerBehavior = incoming;
			sample.HadDangerBehavior = hasDanger;
			sample.LastDangerEntityKey = dangerKey;
			sample.LastDangerGoal = escapeGoal;
		}
	}

	protected void LogDangerEntity(IEntity entity, int unit, int sequence, string role)
	{
		if (!entity)
			return;
		Physics physics = entity.GetPhysics();
		vector velocity;
		if (physics)
			velocity = physics.GetVelocity();
		vector boundsMin;
		vector boundsMax;
		entity.GetBounds(boundsMin, boundsMax);
		vector center = entity.CoordToParent(0.5 * (boundsMin + boundsMax));
		Print("[ConvoyFollower] PACED_DANGER_ENTITY: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
			" seconds=" + PacedSeconds() + " unit=" + unit + " sequence=" + sequence + " role=" + role +
			" entity_id=" + EntityKey(entity) + " entity_name=" + entity.GetName() + " origin=" + entity.GetOrigin() +
			" forward=" + entity.GetWorldTransformAxis(2) + " physics=" + (physics != null) +
			" velocity_world=" + velocity + " bounds_min_local=" + boundsMin + " bounds_max_local=" + boundsMax +
			" bounds_center_world=" + center);
	}

	protected void MeasurePeaks()
	{
		for (int index = 0; index < m_PacedTrucks.Count(); index++)
		{
			CF_PacedRoadTruckSample sample = m_PacedTrucks[index];
			if (!sample.Truck)
				continue;
			if (index > 0 && m_PacedTrucks[index - 1].Truck)
			{
				float gap = vector.Distance(sample.Truck.GetOrigin(), m_PacedTrucks[index - 1].Truck.GetOrigin());
				if (gap > sample.MaxGap)
					sample.MaxGap = gap;
				if (gap > m_fPacedPeakGap)
					m_fPacedPeakGap = gap;
				if (gap > PACED_MAX_LINK_M && !m_bPacedSpacingFailed)
				{
					m_bPacedSpacingFailed = true;
					NoteFailure("spacing", "link_over_60m_unit_" + index);
				}
			}
			if (m_bPacedObserving)
			{
				float drift = vector.DistanceXZ(sample.Truck.GetOrigin(), sample.ObservationOrigin);
				if (drift > sample.MaxDrift)
					sample.MaxDrift = drift;
				if (drift > PACED_MAX_DRIFT_M && !m_bPacedDriftFailed)
				{
					m_bPacedDriftFailed = true;
					string scope = "follower";
					if (index == 0)
						scope = "fixture";
					NoteFailure(scope, "postterminal_drift_over_2m_unit_" + index);
				}
			}
		}
	}

	protected void LogPacedSamples()
	{
		string phase = "setup";
		if (m_bPacedStarted)
			phase = "drive";
		if (m_bPacedHoldingLead)
			phase = "arrival";
		if (m_bPacedObserving)
			phase = "observation";
		for (int index = 0; index < m_PacedTrucks.Count(); index++)
		{
			CF_PacedRoadTruckSample sample = m_PacedTrucks[index];
			if (!sample.Truck || !sample.Pilot)
			{
				Print("[ConvoyFollower] PACED_SAMPLE: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
					" seconds=" + PacedSeconds() + " leg=1 phase=" + phase + " unit=" + index + " missing=true");
				continue;
			}
			vector origin = sample.Truck.GetOrigin();
			vector delta = origin - sample.LastPosition;
			vector stepAxis = sample.LastForward;
			stepAxis[1] = 0;
			stepAxis.Normalize();
			float signedStep = delta[0] * stepAxis[0] + delta[2] * stepAxis[2];
			float step = vector.Distance(origin, sample.LastPosition);
			CarControllerComponent car = CarControllerComponent.Cast(sample.Truck.FindComponent(CarControllerComponent));
			VehicleWheeledSimulation sim;
			if (car)
				sim = car.GetSimulation();
			bool powered = sim && sim.EngineIsOn() && sim.GetThrottle() > 0.05 && sim.GetGear() >= 2 &&
				sim.GetSpeedKmh() >= 1.0 && signedStep >= 0.25;
			if (m_bPacedStarted)
			{
				sample.Path += step;
				sample.ForwardProgress += signedStep;
				if (powered)
					sample.PoweredSamples++;
			}
			IEntity predecessor;
			IEntity actualTarget;
			float gap;
			if (index > 0)
			{
				predecessor = m_PacedTrucks[index - 1].Truck;
				if (sample.Driver)
					actualTarget = sample.Driver.CF_GetDiagnosticTargetVehicle();
				if (predecessor)
					gap = vector.Distance(origin, predecessor.GetOrigin());
			}
			SCR_AIUtilityComponent utility = PilotUtility(sample);
			SCR_AIGroupUtilityComponent groupUtility = GroupUtility(sample);
			string behavior = "none";
			string activity = "none";
			if (utility)
				behavior = ActionKey(utility.GetCurrentBehavior());
			if (groupUtility)
				activity = ActionKey(groupUtility.GetCurrentAction());
			AIWaypoint waypoint;
			if (sample.Group)
				waypoint = sample.Group.GetCurrentWaypoint();
			vector goal;
			if (waypoint)
				goal = waypoint.GetOrigin();
			CompartmentAccessComponent access = sample.Pilot.GetCompartmentAccessComponent();
			IEntity actualPilot;
			if (car && car.GetPilotCompartmentSlot())
				actualPilot = car.GetPilotCompartmentSlot().GetOccupant();
			float drift;
			if (m_bPacedObserving)
				drift = vector.DistanceXZ(origin, sample.ObservationOrigin);
			bool movementDemand = m_bPacedStarted && !m_bPacedHoldingLead;
			if (index > 0)
			{
				CarControllerComponent predecessorCar;
				if (predecessor)
					predecessorCar = CarControllerComponent.Cast(predecessor.FindComponent(CarControllerComponent));
				bool predecessorMoving = predecessorCar && predecessorCar.GetSimulation() &&
					Math.AbsFloat(predecessorCar.GetSimulation().GetSpeedKmh()) > 1.0;
				movementDemand = m_bPacedStarted && (predecessorMoving || gap > CF_ConvoySettings.Get().m_fStoppedGap + 4.0);
			}
			string controls = " simulation=false";
			float requestedLeadCap = -1;
			bool ownsLeadCap;
			if (index == 0 && m_PacedCruise)
			{
				requestedLeadCap = m_PacedCruise.GetRequestedSpeedKmh();
				ownsLeadCap = m_PacedCruise.OwnsOverride();
			}
			if (sim)
				controls = " simulation=true speed_kmh=" + sim.GetSpeedKmh() + " throttle=" + sim.GetThrottle() +
					" brake=" + sim.GetBrake() + " gear=" + sim.GetGear() + " engine=" + sim.EngineIsOn() +
					" handbrake=" + car.GetHandBrake() + " persistent=" + car.GetPersistentHandBrake();
			Print("[ConvoyFollower] PACED_SAMPLE: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
				" seconds=" + PacedSeconds() + " leg=1 phase=" + phase + " unit=" + index +
				" truck=" + sample.Truck.GetName() + " truck_id=" + EntityKey(sample.Truck) + " pilot_id=" + EntityKey(sample.Pilot) +
				" actual_pilot_id=" + EntityKey(actualPilot) + " predecessor_id=" + EntityKey(actualTarget) +
				" expected_predecessor_id=" + EntityKey(predecessor) + " identity_retained=" + IdentityRetained(index, false) +
				" pilot_seated=" + ExactPilot(sample) + " entry_ready=" + EntryReady(sample) +
				" getting_in=" + (access && access.IsGettingIn()) + " getting_out=" + (access && access.IsGettingOut()) +
				" origin=" + origin + " forward=" + sample.Truck.GetWorldTransformAxis(2) + " step_axis=" + stepAxis +
				" step_m=" + step + " signed_step_m=" + signedStep + " path_m=" + sample.Path +
				" forward_progress_m=" + sample.ForwardProgress + " gap_m=" + gap + " peak_gap_m=" + sample.MaxGap +
				" movement_demand=" + movementDemand + " deliberate_stationary=" + (m_bPacedObserving || (index == 0 && m_bPacedHoldingLead)) +
				" powered_sample=" + powered + " powered_samples=" + sample.PoweredSamples + controls +
				" fixture_cruise_owned=" + ownsLeadCap + " fixture_requested_cap_kmh=" + requestedLeadCap +
				" behavior=" + behavior + " group_action=" + activity + " waypoint_id=" + EntityKey(waypoint) + " goal=" + goal +
				" wait_selected=" + (index == 0 && LeadWaitSelected()) + " drift_m=" + drift + " max_drift_m=" + sample.MaxDrift);
			sample.LastPosition = origin;
			sample.LastForward = sample.Truck.GetWorldTransformAxis(2);
		}
	}

	// All paced counts finish the driving course at the same arrival gate,
	// then enter the stronger observation. The legacy one-truck smoke alone
	// retains its separate unload continuation in the parent.
	override protected bool ShouldRunUnloadAfterRoadArrival()
	{
		return false;
	}

	// Preserve the legacy numerical marker, but its PASS is provisional for
	// this subclass. Only PACED_RESULT after observation is the paced verdict.
	override protected void Finish(string result)
	{
		if (m_bPacedTerminal || !m_sPendingRouteResult.IsEmpty())
			return;
		m_sPendingRouteResult = result;
		Print("[ConvoyFollower] AUTO_RESULT: " + result);
	}

	protected void BeginObservation()
	{
		m_bPacedObserving = true;
		m_fObservationStartMs = GetGame().GetWorld().GetWorldTime();
		m_iObservationSamples = 0;
		for (int index = 0; index < m_PacedTrucks.Count(); index++)
		{
			CF_PacedRoadTruckSample sample = m_PacedTrucks[index];
			sample.ObservationOrigin = sample.Truck.GetOrigin();
			sample.MaxDrift = 0;
			Print("[ConvoyFollower] PACED_OBSERVATION_BEGIN: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
				" seconds=" + PacedSeconds() + " unit=" + index + " truck_id=" + EntityKey(sample.Truck) +
				" origin=" + sample.ObservationOrigin + " required_s=180 max_drift_m=2");
		}
	}

	protected void EndPaced()
	{
		if (m_bPacedTerminal)
			return;
		// Retain a safe, owned native park until deletion instead of releasing
		// the pilot back into a residual drive at the terminal marker.
		if (m_bPacedStarted && !m_bPacedHoldingLead && CanControlLead())
			HoldLeadAtRoadGoal();
		if (m_PacedCruise && (!m_bPacedStarted || !CanControlLead()))
			m_PacedCruise.Release("paced_terminal_without_owned_lead");
		m_bPacedTerminal = true;
		m_bFinished = true;
		string verdict = "PASS";
		if (m_bPacedFailed)
			verdict = "FAIL";
		Print("[ConvoyFollower] PACED_RESULT: " + verdict + " run_id=" + m_sPacedRun +
			" tick=" + m_iPacedTick + " seconds=" + PacedSeconds() + " expected=" + m_iExpectedTrucks +
			" peak_gap_m=" + m_fPacedPeakGap + " observation_samples=" + m_iObservationSamples +
			" first_failure=" + m_sFirstFailure + " owned_lead_park_retained=" + m_bPacedHoldingLead);
		GetGame().GetCallqueue().Remove(Poll);
		SchedulePacedExit();
	}

	// This is a supplemental capture grace period, not a new physical PASS
	// gate. The terminal result and required observation remain unchanged.
	protected string PacedExitRefusal()
	{
#ifdef WORKBENCH
		return "workbench";
#else
		if (!m_bPacedAutoExit)
			return "option_disabled";
		if (m_bPacedDeleting || !m_bPacedTerminal || !m_bFinished || m_sPacedRun.IsEmpty())
			return "terminal_owner_not_live";
		if (!GetGame() || !GetGame().GetWorld() || CF_ConvoySession.CF_IsWorldCleanup())
			return "world_missing_or_cleanup";
		if (!GetGame().InPlayMode())
			return "not_in_play_mode";
		if (System.IsConsoleApp() || RplSession.Mode() != RplMode.None || !Replication.IsServer())
			return "not_offline_client";
		if (!m_PacedExitOwner || GetOwner() != m_PacedExitOwner || !m_PacedExitWorld ||
			GetGame().GetWorld() != m_PacedExitWorld || m_PacedExitOwner.GetWorld() != m_PacedExitWorld)
			return "world_or_component_changed";
		if (m_sPacedExitWorldFile.IsEmpty() || GetGame().GetWorldFile() != m_sPacedExitWorldFile)
			return "world_resource_changed_or_unknown";
		return "";
#endif
	}

	protected void SchedulePacedExit()
	{
		if (!m_bPacedAutoExit || m_bPacedExitPending || m_bPacedExitRequested)
			return;
		string reason = PacedExitRefusal();
		if (!reason.IsEmpty())
		{
			Print("[ConvoyFollower] PACED_EXIT_REFUSED: run_id=" + m_sPacedRun + " phase=schedule reason=" + reason);
			return;
		}
		m_fPacedTerminalMs = GetGame().GetWorld().GetWorldTime();
		m_bPacedExitPending = true;
		Print("[ConvoyFollower] PACED_EXIT_SCHEDULED: run_id=" + m_sPacedRun + " world=" + m_sPacedExitWorldFile +
			" delay_s=30 supplemental_capture_only=true terminal_failed=" + m_bPacedFailed);
		GetGame().GetCallqueue().CallLater(RequestPacedExit, PACED_EXIT_DELAY_MS, false);
	}

	protected void RequestPacedExit()
	{
		if (!m_bPacedExitPending || m_bPacedExitRequested)
			return;
		m_bPacedExitPending = false;
		string reason = PacedExitRefusal();
		if (!reason.IsEmpty())
		{
			Print("[ConvoyFollower] PACED_EXIT_REFUSED: run_id=" + m_sPacedRun + " phase=request reason=" + reason);
			return;
		}
#ifndef WORKBENCH
		float elapsedMs = GetGame().GetWorld().GetWorldTime() - m_fPacedTerminalMs;
		if (elapsedMs < PACED_EXIT_DELAY_MS)
		{
			// Callqueue/world-clock rounding produced 29999.8 ms in a live
			// run. Recheck the same ownership gates once after a small margin;
			// never shorten the minimum grace or retry a large clock mismatch.
			float remainingMs = PACED_EXIT_DELAY_MS - elapsedMs;
			if (remainingMs <= PACED_EXIT_RETRY_MS && m_iPacedExitDeferrals < PACED_EXIT_MAX_DEFERRALS)
			{
				m_iPacedExitDeferrals++;
				m_bPacedExitPending = true;
				Print("[ConvoyFollower] PACED_EXIT_DEFERRED: run_id=" + m_sPacedRun +
					" elapsed_ms=" + elapsedMs + " retry_ms=" + PACED_EXIT_RETRY_MS + " deferrals=" + m_iPacedExitDeferrals);
				GetGame().GetCallqueue().CallLater(RequestPacedExit, PACED_EXIT_RETRY_MS, false);
				return;
			}
			Print("[ConvoyFollower] PACED_EXIT_REFUSED: run_id=" + m_sPacedRun +
				" phase=request reason=post_terminal_delay_not_elapsed elapsed_ms=" + elapsedMs + " deferrals=" + m_iPacedExitDeferrals);
			return;
		}
		m_bPacedExitRequested = true;
		Print("[ConvoyFollower] PACED_EXIT_REQUESTED: run_id=" + m_sPacedRun + " world=" + m_sPacedExitWorldFile +
			" elapsed_s=" + elapsedMs / 1000.0 + " terminal_failed=" + m_bPacedFailed + " shutdown_completed=false");
		GetGame().RequestClose();
#endif
	}

	override protected void Poll()
	{
		if (!PacedWorldAlive() || m_bPacedTerminal)
			return;
		m_iPacedTick++;
		bool blockedAtBarrier = false;
		if (!m_bPacedStarted && m_iStage == 2 && m_iNextOrder == m_iExpectedTrucks)
		{
			BindOriginals();
			blockedAtBarrier = !StartBarrier();
			if (blockedAtBarrier)
			{
				m_iBarrierTicks++;
				m_iTicks++; // Keep the inherited setup bound reachable while waiting.
				if (m_iBarrierTicks >= 60)
					Finish("FAIL native entry or lead cruise binding barrier timed out");
			}
		}
		if (!blockedAtBarrier && !m_bPacedObserving && m_sPendingRouteResult.IsEmpty())
			super.Poll();
		BindOriginals();
		if (!m_PacedTrucks.IsEmpty())
			MeasurePeaks();
		LogPacedSamples();
		if (m_bPacedStarted)
		{
			if (!OwnerRetained())
			{
				NoteFailure("fixture", "original_owner_or_session_lost");
				EndPaced();
				return;
			}
			for (int index = 0; index < m_PacedTrucks.Count(); index++)
			{
				if (!IdentityRetained(index, true))
				{
					string identityScope = "follower";
					if (index == 0)
						identityScope = "fixture";
					NoteFailure(identityScope, "original_pilot_truck_or_predecessor_lost_unit_" + index);
					EndPaced();
					return;
				}
			}
			float cap = PACED_LEAD_KMH;
			if (m_bPacedHoldingLead)
				cap = 0;
			if (!m_PacedCruise.Request(m_Pilot, m_Lead, cap, "paced_fixture_lead"))
			{
				NoteFailure("fixture", "lead_native_cruise_ownership_lost");
				EndPaced();
				return;
			}
			if (m_bPacedHoldingLead && !LeadWaitSelected() &&
				GetGame().GetWorld().GetWorldTime() - m_fLeadHoldStartMs > 5000)
			{
				NoteFailure("fixture", "owned_native_wait_not_selected");
				EndPaced();
				return;
			}
		}
		if (!m_sPendingRouteResult.IsEmpty() && !m_bPacedObserving)
		{
			bool legacyPass = m_sPendingRouteResult.IndexOf("PASS") == 0;
			bool poweredPass = true;
			for (int unit = 0; unit < m_PacedTrucks.Count(); unit++)
			{
				float requiredProgress = CF_MIN_FOLLOWER_PATH;
				if (unit == 0)
					requiredProgress = CF_MIN_LEAD_PATH;
				if (m_PacedTrucks[unit].PoweredSamples < 3 || m_PacedTrucks[unit].ForwardProgress < requiredProgress)
					poweredPass = false;
			}
			if (!legacyPass)
				NoteFailure("route", "legacy_route_gate_failed");
			else if (!poweredPass)
				NoteFailure("movement", "insufficient_powered_forward_travel");
			Print("[ConvoyFollower] PACED_ROUTE_RESULT: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
				" seconds=" + PacedSeconds() + " legacy_pass=" + legacyPass + " powered_pass=" + poweredPass +
				" spacing_pass=" + !m_bPacedSpacingFailed + " peak_gap_m=" + m_fPacedPeakGap + " provisional=true");
			if (!legacyPass || !LeadWaitSelected())
			{
				if (legacyPass)
					NoteFailure("fixture", "native_wait_not_selected_at_route_terminal");
				EndPaced();
				return;
			}
			BeginObservation();
			return;
		}
		if (m_bPacedObserving)
		{
			m_iObservationSamples++;
			float observedMs = GetGame().GetWorld().GetWorldTime() - m_fObservationStartMs;
			if (observedMs >= PACED_OBSERVATION_MS && m_iObservationSamples >= 180)
			{
				Print("[ConvoyFollower] PACED_OBSERVATION_COMPLETE: run_id=" + m_sPacedRun + " tick=" + m_iPacedTick +
					" seconds=" + PacedSeconds() + " observed_s=" + observedMs / 1000.0 + " samples=" + m_iObservationSamples +
					" drift_pass=" + !m_bPacedDriftFailed + " identities_retained=true lead_wait_selected=" + LeadWaitSelected());
				EndPaced();
				return;
			}
		}
		if (GetGame().GetWorld().GetWorldTime() - m_fPacedStartMs >= PACED_TIMEOUT_MS)
		{
			NoteFailure("fixture", "scenario_480s_timeout");
			EndPaced();
		}
	}

	override void OnDelete(IEntity owner)
	{
		m_bPacedDeleting = true;
		if (GetGame())
		{
			GetGame().GetCallqueue().Remove(RequestPacedExit);
			GetGame().GetCallqueue().Remove(Poll);
			GetGame().GetCallqueue().Remove(LogOpenRoadSurvey);
		}
		if (m_bPacedExitPending)
			Print("[ConvoyFollower] PACED_EXIT_REFUSED: run_id=" + m_sPacedRun + " phase=delete reason=owner_deleted_before_request");
		m_bPacedExitPending = false;
		m_PacedExitOwner = null;
		m_PacedExitWorld = null;
		bool mayReset = CanControlLead();
		if (m_bPacedStarted && mayReset && m_PilotWaypoint)
		{
			SCR_AIGroupUtilityComponent groupUtility = GroupUtility(m_PacedTrucks[0]);
			if (groupUtility)
				groupUtility.CancelActivitiesRelatedToWaypoint(m_PilotWaypoint, doNotCompleteWaypoint: true);
			m_PilotGroup.RemoveWaypoint(m_PilotWaypoint);
			m_PilotWaypoint = null;
		}
		if (m_PacedWait && mayReset)
		{
			m_PacedWait.SetFailReason(EAIActionFailReason.CANCELLED);
			m_PacedWait.Fail();
		}
		m_PacedWait = null;
		if (m_PacedCruise)
		{
			if (PacedWorldAlive())
				m_PacedCruise.Release("paced_probe_delete");
			else
				m_PacedCruise.DetachForWorldCleanup();
		}
		if (m_bPacedOwnBrake && mayReset)
		{
			CarControllerComponent car = CarControllerComponent.Cast(m_Lead.FindComponent(CarControllerComponent));
			if (car && car.GetSimulation())
			{
				car.SetPersistentHandBrake(false);
				car.GetSimulation().SetBreak(0, true);
			}
		}
		Print("[ConvoyFollower] PACED_CLEANUP: run_id=" + m_sPacedRun + " live_owner_reset=" + mayReset +
			" terminal=" + m_bPacedTerminal + " world_cleanup=" + CF_ConvoySession.CF_IsWorldCleanup());
		m_PacedTrucks.Clear();
		m_bPacedOwnBrake = false;
	}
}
