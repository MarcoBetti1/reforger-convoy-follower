// Native tests of the actual helper. Attach to an inert test-world entity.
// This does not issue convoy orders, drive vehicles, or require a road network.
class CF_DrivenRouteGeometryProbeComponentClass : ScriptComponentClass
{
}

class CF_DrivenRouteGeometryProbeComponent : ScriptComponent
{
	protected int m_Passed;
	protected int m_Total;

	override void OnPostInit(IEntity owner)
	{
		if (Replication.IsServer())
			GetGame().GetCallqueue().CallLater(RunCases, 1000, false);
	}

	protected void Check(string label, bool passed)
	{
		m_Total++;
		if (passed)
			m_Passed++;
		else
			Print("[ConvoyFollower] DRIVEN_ROUTE_CASE: FAIL " + label);
	}

	protected bool Near(float actual, float expected)
	{
		return Math.AbsFloat(actual - expected) < 0.01;
	}

	protected bool GoalNear(CF_DrivenRouteGuidance result, vector expected)
	{
		return vector.Distance(result.Goal, expected) < 0.01;
	}

	protected void RunCases()
	{
		ref CF_DrivenRoute route = new CF_DrivenRoute();
		ref CF_DrivenRouteGuidance g = new CF_DrivenRouteGuidance();
		route.Query(1, vector.Zero, 10, 5, 10, 4, g);
		Check("no_reset_is_unseeded", g.State == CF_DrivenRoute.UNSEEDED && !g.HasArcGap);
		route.Reset(1, vector.Zero);
		route.Query(1, Vector(-10, 0, 0), 10, 5, 10, 4, g);
		Check("one_point_does_not_claim_arrival", g.State == CF_DrivenRoute.UNSEEDED && !g.HasArcGap && g.ArcGap < 0);
		route.Record(1, Vector(20, 0, 0));
		route.Query(1, Vector(-10, 0, 0), 10, 5, 10, 4, g);
		Check("initial_follower_approaches_actual_start", g.State == CF_DrivenRoute.APPROACH_START && !g.HasArcGap && GoalNear(g, vector.Zero) && route.GetPointCount() == 2);
		route.Query(1, Vector(5, 0, 0), 8, 5, 10, 4, g);
		Check("straight_interpolated_lookahead", g.State == CF_DrivenRoute.TRACKING && GoalNear(g, Vector(13, 0, 0)) && Near(g.ArcGap, 15));
		route.Query(1, Vector(3, 0, 0), 8, 5, 10, 4, g);
		Check("progress_never_rewinds", Near(g.Progress, 5) && GoalNear(g, Vector(13, 0, 0)));
		route.Query(1, Vector(19, 0, 0), 8, 5, 5, 4, g);
		Check("bounded_advance_rejects_jump", g.State == CF_DrivenRoute.ADVANCE_LIMIT && Near(route.GetProgress(), 5));
		route.Query(1, Vector(8, 0, 10), 8, 5, 10, 4, g);
		Check("off_route_does_not_move_cursor", g.State == CF_DrivenRoute.OFF_ROUTE && Near(route.GetProgress(), 5));
		route.Query(1, Vector(17, 0, 0), 8, 5, 20, 4, g);
		Check("stopped_predecessor_spacing_hold", g.State == CF_DrivenRoute.SPACING_HOLD && Near(g.ArcGap, 3) && GoalNear(g, Vector(17, 0, 0)));
		route.Query(1, Vector(17, 0, 0), 8, 5, 20, 4, g);
		Check("repeated_stopped_query_stays_put", g.State == CF_DrivenRoute.SPACING_HOLD && Near(g.Progress, 17));
		Check("duplicate_samples_not_inserted", route.Record(1, Vector(20.1, 0, 0)) && route.GetPointCount() == 2);
		Check("target_record_requires_explicit_reset", !route.Record(2, Vector(21, 0, 0)) && route.GetPointCount() == 2);
		route.Query(2, Vector(17, 0, 0), 8, 5, 20, 4, g);
		Check("target_query_does_not_reuse_old_gap", g.State == CF_DrivenRoute.TARGET_MISMATCH && !g.HasArcGap);
		route.Reset(2, Vector(100, 0, 100));
		route.Query(2, Vector(90, 0, 100), 8, 5, 20, 4, g);
		Check("explicit_reset_drops_history_and_progress", g.State == CF_DrivenRoute.UNSEEDED && route.GetPointCount() == 1 && Near(route.GetProgress(), 0));
		Check("large_sample_gap_rejected", !route.Record(2, Vector(140, 0, 100)));
		route.Query(2, Vector(100, 0, 100), 8, 5, 20, 4, g);
		Check("discontinuity_latches_until_reset", g.State == CF_DrivenRoute.DISCONTINUITY && !route.Record(2, Vector(101, 0, 100)));

		route.Reset(3, vector.Zero);
		route.Record(3, Vector(20, 0, 0));
		route.Record(3, Vector(20, 0, 10));
		route.Record(3, Vector(40, 0, 10));
		route.Record(3, Vector(40, 0, 20));
		route.Record(3, Vector(60, 0, 20));
		route.Query(3, Vector(15, 0, 0), 35, 10, 20, 2, g);
		Check("s_bend_lookahead_uses_arc_length", g.State == CF_DrivenRoute.TRACKING && GoalNear(g, Vector(40, 0, 10)) && Near(g.ArcGap, 65));
		route.Query(3, Vector(20, 0, 8), 20, 10, 20, 9, g);
		Check("adjacent_corner_progress", g.State == CF_DrivenRoute.TRACKING && Near(g.Progress, 28) && GoalNear(g, Vector(38, 0, 10)));

		route.Reset(30, vector.Zero);
		route.Record(30, Vector(20, 0, 0));
		route.Record(30, Vector(20, 0, 20));
		route.Query(30, Vector(18, 0, 2), 5, 5, 20, 4, g);
		Check("inside_corner_entry_joins_first_leg", g.State == CF_DrivenRoute.TRACKING && Near(g.Progress, 18));
		route.Query(30, Vector(18, 0, 8), 5, 5, 5, 4, g);
		Check("rounded_corner_respects_progress_budget", g.State == CF_DrivenRoute.ADVANCE_LIMIT && Near(route.GetProgress(), 18));
		route.Query(30, Vector(18, 0, 8), 5, 5, 12, 4, g);
		Check("inside_corner_acquires_only_adjacent_leg", g.State == CF_DrivenRoute.TRACKING && Near(g.Progress, 28) && GoalNear(g, Vector(20, 0, 13)));

		route.Reset(4, vector.Zero);
		route.Record(4, Vector(20, 0, 0));
		route.Record(4, Vector(20, 0, 3));
		route.Record(4, Vector(0, 0, 3));
		route.Query(4, Vector(5, 0, 0), 5, 5, 10, 4, g);
		route.Query(4, Vector(6, 0, 3), 5, 5, 10, 4, g);
		Check("hairpin_nearer_return_leg_cannot_skip", g.State == CF_DrivenRoute.TRACKING && Near(g.Progress, 6) && Near(g.ArcGap, 37) && GoalNear(g, Vector(11, 0, 0)));

		route.Reset(5, vector.Zero);
		route.Record(5, Vector(20, 10, 0));
		route.Query(5, Vector(5, 2.5, 0), 5, 5, 10, 2, g);
		Check("xz_arc_interpolates_recorded_height", GoalNear(g, Vector(10, 5, 0)) && Near(g.ArcGap, 15));

		route.Reset(6, vector.Zero);
		route.Record(6, Vector(1, 0, 0));
		route.Query(6, vector.Zero, 10, 5, 2, 2, g);
		bool retained = true;
		for (int i = 2; i <= 300; i++)
		{
			if (!route.Record(6, Vector(i, 0, 0)))
				retained = false;
			route.Query(6, Vector(i, 0, 0), 10, 5, 2, 2, g);
		}
		Check("consumed_history_prunes_without_progress_reset", retained && route.GetPointCount() <= CF_DrivenRoute.MAX_POINTS && Near(route.GetProgress(), 300) && g.HasArcGap);
		route.Reset(7, vector.Zero);
		route.Record(7, Vector(10, 0, 0));
		route.Query(7, vector.Zero, 10, 5, 20, 2, g);
		retained = true;
		for (int j = 2; j <= 80; j++)
		{
			if (!route.Record(7, Vector(j * 10, 0, 0)))
				retained = false;
			route.Query(7, Vector(j * 10, 0, 0), 10, 5, 20, 2, g);
		}
		Check("retained_arc_length_is_bounded", retained && route.GetRetainedLength() <= CF_DrivenRoute.MAX_LENGTH && Near(route.GetProgress(), 800));
		route.Reset(8, vector.Zero);
		for (int k = 1; k <= 270; k++)
			route.Record(8, Vector(k, 0, 0));
		route.Query(8, vector.Zero, 10, 5, 20, 2, g);
		Check("unconsumed_pruning_reports_history_lost", g.State == CF_DrivenRoute.HISTORY_LOST && !g.HasArcGap && route.GetPointCount() <= CF_DrivenRoute.MAX_POINTS);

		if (m_Passed == m_Total)
			Print("[ConvoyFollower] DRIVEN_ROUTE_RESULT: PASS cases=" + m_Passed);
		else
			Print("[ConvoyFollower] DRIVEN_ROUTE_RESULT: FAIL passed=" + m_Passed + " total=" + m_Total);
	}
}
