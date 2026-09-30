// Ordinary convoy prefabs use this waypoint through the controller's exact
// original-follow lease. It owns one persistent activity; stock FOLLOW reactions
// remain separate. The Tests path reflects its development origin.
class CF_EntityFollowActivity : SCR_AIFollowActivity
{
	protected static int s_iSequence;
	protected int m_iSequence;

	void CF_EntityFollowActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, vector pos, IEntity ent,
		EMovementType movementType = EMovementType.RUN, bool useVehicles = true,
		float priority = PRIORITY_ACTIVITY_FOLLOW, float priorityLevel = PRIORITY_LEVEL_NORMAL, float distance = 1.0)
	{
		// Shared diagnostics only. The admitted CF_OriginalFollowActivity
		// subclass selects the authored, exact-lease orchestration graph.
		s_iSequence++;
		m_iSequence = s_iSequence;
	}

	int CF_GetSequence()
	{
		return m_iSequence;
	}
}

class CF_EntityFollowWaypointClass : SCR_EntityWaypointClass
{
}

class CF_EntityFollowWaypoint : SCR_EntityWaypoint
{
	// Weak diagnostic handle. The group utility owns the activity lifetime.
	protected CF_EntityFollowActivity m_EntityActivity;

	protected ref CF_OriginalFollowLease m_OriginalLease;

	void CF_BindOriginalLease(CF_OriginalFollowLease lease)
	{
		if (!m_OriginalLease)
			m_OriginalLease = lease;
	}

	CF_EntityFollowActivity CF_CreateBoundActivity(SCR_AIGroupUtilityComponent utility)
	{
		if (m_OriginalLease)
		{
			string reason;
			if (!m_OriginalLease.Admit(utility, reason))
			{
				m_OriginalLease.Block("selection_admission_" + reason);
				return null;
			}
			if (m_EntityActivity && m_EntityActivity.GetActionState() != EAIActionState.COMPLETED &&
				m_EntityActivity.GetActionState() != EAIActionState.FAILED)
			{
				m_OriginalLease.Block("duplicate_selected_activity");
				return null;
			}
			CF_OriginalFollowActivity original = new CF_OriginalFollowActivity(utility, this,
				GetOrigin(), GetEntity(), useVehicles: true, priorityLevel: GetPriorityLevel(), distance: GetCompletionRadius());
			original.Lease = m_OriginalLease;
			m_OriginalLease.Activity = original;
			return original;
		}
		Print("[ConvoyFollower] ENTITY_FOLLOW_ADMISSION_FAILED: waypoint_id=" + GetID() +
			" reason=original_lease_required no_action_created=true");
		return null;
	}


	override SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new CF_EntityFollowWaypointState(groupUtilityComp, this);
	}

	void CF_SetActivity(CF_EntityFollowActivity activity)
	{
		m_EntityActivity = activity;
	}

	CF_EntityFollowActivity CF_GetActivity()
	{
		return m_EntityActivity;
	}
}

class CF_EntityFollowWaypointState : SCR_AIWaypointState
{
	protected CF_EntityFollowWaypoint m_EntityWaypoint;

	void CF_EntityFollowWaypointState(notnull SCR_AIGroupUtilityComponent utility, SCR_AIWaypoint waypoint)
	{
		m_EntityWaypoint = CF_EntityFollowWaypoint.Cast(waypoint);
	}

	override void OnSelected()
	{
		super.OnSelected();
		if (CF_ConvoySession.CF_IsWorldCleanup() || !m_EntityWaypoint || !Vehicle.Cast(m_EntityWaypoint.GetEntity()))
			return;
		CF_EntityFollowActivity activity = m_EntityWaypoint.CF_CreateBoundActivity(m_Utility);
		if (!activity)
			return;
		m_EntityWaypoint.CF_SetActivity(activity);
		m_Utility.AddAction(activity);
		string binding = "[ConvoyFollower] ENTITY_FOLLOW_ACTIVITY: sequence=" + activity.CF_GetSequence();
		binding += " activity=" + activity.ToString() + " waypoint_id=" + m_Waypoint.GetID();
		binding += " target_id=" + m_EntityWaypoint.GetEntity().GetID();
		binding += " vehicle_planning=true desired_distance=" + activity.m_fCompletionDistance.m_Value;
		Print(binding);
	}

	override void OnDeselected()
	{
		// Base cancellation is related-waypoint scoped and sends related-activity
		// child cancellation. Never complete another waypoint or touch controls.
		if (!CF_ConvoySession.CF_IsWorldCleanup())
			super.OnDeselected();
		if (m_EntityWaypoint)
			m_EntityWaypoint.CF_SetActivity(null);
	}
}
