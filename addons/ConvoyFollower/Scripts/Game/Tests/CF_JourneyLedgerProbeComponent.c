class CF_JourneyLedgerProbeComponentClass : ScriptComponentClass {}

// Opt-in read-only physical-container ledger for the ordinary one-truck scene.
// It never dispatches an action, writes resources or moves an actor.
class CF_JourneyLedgerProbeComponent : ScriptComponent
{
	protected World m_World;
	protected float m_fNextMs;
	protected bool m_bBound;
	protected int m_iBindingDiagnostics;
	protected ref array<IEntity> m_Entities = {};
	protected ref array<SCR_ResourceContainer> m_Containers = {};

	protected void FindPhysicalContainers(IEntity entity, array<SCR_ResourceContainer> found)
	{
		if (!entity) return;
		SCR_ResourceComponent resource = SCR_ResourceComponent.Cast(entity.FindComponent(SCR_ResourceComponent));
		array<SCR_ResourceContainer> containers;
		if (resource) containers = resource.GetContainers();
		if (containers)
			foreach (SCR_ResourceContainer container : containers)
				if (container && container.GetResourceType() == EResourceType.SUPPLIES &&
					!SCR_ResourceContainerVirtual.Cast(container) && container.GetOwner() == entity && !found.Contains(container))
					found.Insert(container);
		IEntity child = entity.GetChildren();
		while (child) { FindPhysicalContainers(child, found); child = child.GetSibling(); }
	}

	protected void FindCargoActions(IEntity entity, array<BaseUserAction> loads, array<BaseUserAction> unloads)
	{
		if (!entity) return;
		BaseActionsManagerComponent manager = BaseActionsManagerComponent.Cast(entity.FindComponent(BaseActionsManagerComponent));
		array<BaseUserAction> actions = {};
		if (manager) manager.GetActionsList(actions);
		foreach (BaseUserAction action : actions)
		{
			if (!action || action.GetOwner() != entity) continue;
			if (SCR_ResourceContainerVehicleLoadAction.Cast(action) && !loads.Contains(action)) loads.Insert(action);
			if (SCR_ResourceContainerVehicleUnloadAction.Cast(action) && !unloads.Contains(action)) unloads.Insert(action);
		}
		IEntity child = entity.GetChildren();
		while (child) { FindCargoActions(child, loads, unloads); child = child.GetSibling(); }
	}

	// Vehicle props also carry physical containers. The ledger must read the
	// native rear transfer endpoint, not aggregate every descendant or guess
	// by capacity. No action is dispatched by this binding.
	protected void FindLedgerContainers(IEntity entity, array<SCR_ResourceContainer> found)
	{
		if (!Vehicle.Cast(entity)) { FindPhysicalContainers(entity, found); return; }
		array<BaseUserAction> loads = {}, unloads = {};
		FindCargoActions(entity, loads, unloads);
		if (loads.Count() != 1 || unloads.Count() != 1) return;
		IEntity cargo = loads[0].GetOwner();
		BaseActionsManagerComponent manager = loads[0].GetActionsManager();
		if (!cargo || cargo.GetRootParent() != entity || unloads[0].GetOwner() != cargo ||
			!manager || unloads[0].GetActionsManager() != manager) return;
		UserActionContext rear = manager.GetContext("door_rear");
		array<BaseUserAction> rearActions = {};
		if (rear) rear.GetActionsList(rearActions);
		if (!rearActions.Contains(loads[0]) || !rearActions.Contains(unloads[0])) return;
		SCR_ResourceComponent resource = SCR_ResourceComponent.Cast(cargo.FindComponent(SCR_ResourceComponent));
		array<SCR_ResourceContainer> containers;
		if (resource) containers = resource.GetContainers();
		if (containers) foreach (SCR_ResourceContainer container : containers)
			if (container && container.GetResourceType() == EResourceType.SUPPLIES &&
				!SCR_ResourceContainerVirtual.Cast(container) && container.GetOwner() == cargo && !found.Contains(container))
				found.Insert(container);
	}

	protected bool Bind()
	{
		array<string> names = { "CF_JourneySource", "CF_JourneyLead", "CF_JourneyTruck1", "CF_JourneyDestination" };
		array<IEntity> entities = {};
		array<SCR_ResourceContainer> containers = {};
		foreach (string name : names)
		{
			IEntity entity = m_World.FindEntityByName(name);
			array<SCR_ResourceContainer> found = {};
			FindLedgerContainers(entity, found);
			if (!entity || found.Count() != 1)
			{
				if (m_iBindingDiagnostics < 8)
				{
					m_iBindingDiagnostics++;
					Print("[ConvoyFollower] JOURNEY_LEDGER_BIND_MISSING: name=" + name +
						" root_exists=" + (entity != null) + " physical_container_count=" + found.Count() +
						" read_only=true no_counts_assumed=true");
				}
				return false;
			}
			entities.Insert(entity);
			containers.Insert(found[0]);
		}
		m_Entities.Copy(entities);
		m_Containers.Copy(containers);
		m_bBound = true;
		for (int i = 0; i < names.Count(); i++)
			Print("[ConvoyFollower] JOURNEY_LEDGER_BOUND: slot=" + i + " name=" + names[i] +
				" root=" + m_Entities[i].GetID() + " physical_owner=" + m_Containers[i].GetOwner().GetID() +
				" container_type=" + m_Containers[i].Type().ToString() +
				" native_rear_endpoint=" + (Vehicle.Cast(m_Entities[i]) != null) + " virtual_proxy=false read_only=true");
		return true;
	}

	override void OnPostInit(IEntity owner)
	{
		if (!GetGame() || !GetGame().InPlayMode() || !Replication.IsServer()) return;
		m_World = GetGame().GetWorld();
		SetEventMask(owner, EntityEvent.POSTFRAME);
	}

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (!GetGame() || !GetGame().InPlayMode() || !Replication.IsServer() ||
			GetGame().GetWorld() != m_World || CF_ConvoySession.CF_IsWorldCleanup()) return;
		float now = m_World.GetWorldTime();
		if (now < m_fNextMs) return;
		m_fNextMs = now + 1000.0;
		if (!m_bBound && !Bind())
		{
			Print("[ConvoyFollower] JOURNEY_LEDGER: unavailable reason=one_original_physical_container_per_slot_not_bound no_counts_assumed=true");
			return;
		}
		float total;
		array<float> values = {};
		bool valid = true;
		for (int i = 0; i < m_Containers.Count(); i++)
		{
			array<SCR_ResourceContainer> current = {};
			FindLedgerContainers(m_Entities[i], current);
			if (current.Count() != 1 || current[0] != m_Containers[i]) { valid = false; break; }
			float value = m_Containers[i].GetResourceValue();
			if (value < 0 || value > m_Containers[i].GetMaxResourceValue()) valid = false;
			values.Insert(value);
			total += value;
		}
		if (!valid || values.Count() != 4)
		{
			Print("[ConvoyFollower] JOURNEY_LEDGER: FAIL reason=original_physical_container_binding_or_bounds_changed read_only=true");
			return;
		}
		Print("[ConvoyFollower] JOURNEY_LEDGER: source=" + values[0] + " lead=" + values[1] +
			" follower=" + values[2] + " destination=" + values[3] + " total=" + total +
			" conserved=" + (Math.AbsFloat(total - 1800.0) <= 0.01) +
			" read_only=true resource_writes=false automatic_actions=false journey_completion_claim=false");
	}
}
