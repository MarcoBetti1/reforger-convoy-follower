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

	protected bool Physical(CF_TrailGuideRoute route, int key, vector pose, float measured, CF_DrivenRouteGuidance g)
	{
		float allowed, turnCos, factor;
		string reason;
		return route.QueryPhysical(key, pose, measured, 0.3, 0, 5, allowed, turnCos, factor, reason, g);
	}

	// These previous poses and incoming leg are inferred witnesses, NOT the
	// missing original per-frame pose or an exact replay of historical state.
	protected vector PrepareProjectionWitness(CF_TrailGuideRoute route, int key, int witness,
		CF_DrivenRouteGuidance g, bool onlyIncoming = false)
	{
		vector incoming = Vector(0.947309345, 0, 0.320320161);
		vector outgoing = Vector(1.6053474, 0, 0.751709);
		route.Reset(key, incoming * -1.5);
		route.Record(key, vector.Zero);
		if (!onlyIncoming)
		{
			route.Record(key, outgoing);
			route.Record(key, outgoing * 2);
		}
		vector prior = Vector(-0.121115532, 0, 0.0500993715);
		if (witness == 2)
			prior = Vector(-0.128715532, 0, 0.0725754797);
		Physical(route, key, prior, 2, g);
		return prior;
	}

	protected void RunProjectionCases()
	{
		ref CF_TrailGuideRoute route = new CF_TrailGuideRoute();
		ref CF_DrivenRouteGuidance g = new CF_DrivenRouteGuidance();
		vector query = Vector(0.0128174, 0, 0.10791);
		vector incoming = Vector(0.947309345, 0, 0.320320161);
		vector outgoing = Vector(1.6053474, 0, 0.751709);
		vector prior = PrepareProjectionWitness(route, 101, 1, g);
		float before = route.GetProgress();
		Check("projection_real_seed_query_commits_without_correction", g.State == CF_DrivenRoute.TRACKING && !g.HasProjectionCorrection && Near(before, 1.401314));
		route.Query(101, query, 0.3, 0, 0.146106, 5, g);
		Check("projection_default_query_still_rejects_logged_excess", g.State == CF_DrivenRoute.ADVANCE_LIMIT && !g.HasProjectionCorrection && Near(route.GetProgress(), before) && Math.AbsFloat(g.DiagnosticCandidate - g.DiagnosticBudgetEnd - 0.00994873) < 0.001);

		for (int witness = 1; witness <= 2; witness++)
		{
			prior = PrepareProjectionWitness(route, 102, witness, g);
			bool accepted = Physical(route, 102, query, 0.145877, g);
			Check("projection_inferred_witness_" + witness, accepted && g.State == CF_DrivenRoute.TRACKING && g.HasProjectionCorrection && Near(g.ProjectionCandidateAdvance, 0.156055) && g.ProjectionSignedCorrection > 0.009 && g.ProjectionSignedCorrection < 0.012 && Near(route.GetProgress(), 1.557369));
		}

		prior = PrepareProjectionWitness(route, 103, 1, g);
		before = route.GetProgress();
		Physical(route, 103, prior, 0, g);
		Check("projection_stationary_query_adds_no_credit", g.State == CF_DrivenRoute.TRACKING && !g.HasProjectionCorrection && Near(route.GetProgress(), before));
		bool negative = Physical(route, 103, query, -0.145877, g);
		Check("projection_negative_measurement_rejected", !negative && Near(route.GetProgress(), before));
		Physical(route, 103, query, 0.145877, g);
		Check("projection_negative_failure_invalidates_context", g.State == CF_DrivenRoute.ADVANCE_LIMIT && !g.HasProjectionCorrection);

		prior = PrepareProjectionWitness(route, 104, 1, g);
		Physical(route, 104, query, 0.01, g);
		Check("projection_insufficient_measurement_rejected", g.State == CF_DrivenRoute.ADVANCE_LIMIT && !g.HasProjectionCorrection);
		Physical(route, 104, query, 0.145877, g);
		Check("projection_failed_query_cannot_seed_retry", g.State == CF_DrivenRoute.ADVANCE_LIMIT && !g.HasProjectionCorrection);

		prior = PrepareProjectionWitness(route, 105, 1, g);
		route.Query(105, prior, 0.3, 0, 0.001, 5, g);
		Physical(route, 105, query, 0.145877, g);
		Check("projection_ordinary_query_disarms_prior_context", g.State == CF_DrivenRoute.ADVANCE_LIMIT && !g.HasProjectionCorrection);

		prior = PrepareProjectionWitness(route, 106, 1, g, true);
		route.Record(106, outgoing);
		route.Record(106, outgoing * 2);
		Physical(route, 106, query, 0.145877, g);
		Check("projection_new_corner_cannot_retrofit_prior_context", g.State == CF_DrivenRoute.ADVANCE_LIMIT && !g.HasProjectionCorrection);

		prior = PrepareProjectionWitness(route, 107, 1, g);
		Physical(route, 108, query, 0.145877, g);
		Check("projection_wrong_epoch_preserves_target_guard", g.State == CF_DrivenRoute.TARGET_MISMATCH && !g.HasProjectionCorrection);
		Physical(route, 107, query, 0.145877, g);
		Check("projection_epoch_failure_invalidates_context", g.State == CF_DrivenRoute.ADVANCE_LIMIT && !g.HasProjectionCorrection);

		prior = PrepareProjectionWitness(route, 109, 1, g);
		Physical(route, 109, prior - incoming * 0.003, 0.003, g);
		Physical(route, 109, query, 0.149, g);
		Check("projection_reverse_high_water_not_rebased", g.State == CF_DrivenRoute.ADVANCE_LIMIT && !g.HasProjectionCorrection);

		prior = PrepareProjectionWitness(route, 110, 1, g);
		route.Record(110, outgoing * 3);
		Physical(route, 110, query, 0.145877, g);
		Check("projection_unrelated_future_append_keeps_exact_local_context", g.State == CF_DrivenRoute.TRACKING && g.HasProjectionCorrection);

		route.Reset(111, vector.Zero);
		route.Record(111, Vector(20, 0, 0));
		route.Record(111, Vector(20, 0, 20));
		Physical(route, 111, Vector(18, 0, 2), 20, g);
		Physical(route, 111, Vector(18, 0, 8), 1, g);
		Check("projection_first_vertex_budget_remains_strict", g.State == CF_DrivenRoute.ADVANCE_LIMIT && !g.HasProjectionCorrection && Near(route.GetProgress(), 18));

		vector tangent = outgoing.Normalized();
		route.Reset(112, incoming * -20);
		route.Record(112, vector.Zero);
		route.Record(112, tangent);
		route.Record(112, tangent - incoming * 10);
		prior = Vector(-0.121115532, 0, 0.0500993715) * 20;
		Physical(route, 112, prior, 20, g);
		Physical(route, 112, query * 20, 0.145877 * 20, g);
		Check("projection_correction_cannot_pay_second_vertex", g.State == CF_DrivenRoute.ADVANCE_LIMIT && !g.HasProjectionCorrection);

		for (int sign = -1; sign <= 1; sign += 2)
		{
			route.Reset(113, Vector(-20, 0, 0));
			route.Record(113, vector.Zero);
			route.Record(113, Vector(0, 0, 20 * sign));
			route.Record(113, Vector(20, 0, 20 * sign));
			prior = Vector(-5, 0, 4.99 * sign);
			vector cornerPose = Vector(-4.99, 0, 8.6 * sign);
			Physical(route, 113, prior, 20, g);
			Physical(route, 113, prior, 0, g);
			float measured = vector.DistanceXZ(prior, cornerPose);
			Physical(route, 113, cornerPose, measured, g);
			Check("projection_actual_90_degree_edge_" + sign, g.State == CF_DrivenRoute.TRACKING && g.HasProjectionCorrection && Near(g.ProjectionSignedCorrection, 9.98) && Near(g.ProjectionCandidateAdvance, 13.6) && measured < 5);
			before = route.GetProgress();
			Physical(route, 113, prior, measured, g);
			Physical(route, 113, cornerPose, measured, g);
			Check("projection_90_degree_return_adds_no_credit_" + sign, g.State == CF_DrivenRoute.TRACKING && !g.HasProjectionCorrection && Near(route.GetProgress(), before));
		}

		route.Reset(114, vector.Zero);
		route.Record(114, Vector(20, 0, 0));
		route.Record(114, Vector(20, 0, 3));
		route.Record(114, Vector(0, 0, 3));
		Physical(route, 114, Vector(5, 0, 0), 10, g);
		Physical(route, 114, Vector(6, 0, 3), Math.Sqrt(10), g);
		Check("projection_nearer_hairpin_return_is_not_searched", g.State == CF_DrivenRoute.TRACKING && !g.HasProjectionCorrection && Near(route.GetProgress(), 6));
		route.Query(114, Vector(6, 0, 3), 10, 5, -1, 4, g);
		Check("projection_original_invalid_budget_guard_unchanged", g.State == CF_DrivenRoute.INVALID_ARGUMENT && !g.HasProjectionCorrection);

		route.Reset(115, vector.Zero);
		route.Record(115, Vector(20, 0, 0));
		route.Record(115, Vector(10, 0, 1));
		Physical(route, 115, Vector(18, 0, 0), 20, g);
		Check("projection_over_90_degree_turn_still_rejected", !Physical(route, 115, Vector(19, 0, 0), 1, g));

		// Current geometry only: a grid-aligned float32 witness inside the
		// logged world-coordinate rounding intervals, not original exact bits.
		vector worldA = Vector(1546.32421875, 30.5643, 3339.12890625);
		vector worldQ = worldA + Vector(0.0128174, -0.00338554, 0.10791);
		vector worldB = worldQ - Vector(-1.59253, 0.269512, -0.643799);
		vector edge = worldB - worldA;
		edge[1] = 0;
		vector relative = worldQ - worldA;
		relative[1] = 0;
		float length = edge.Length();
		float raw = vector.Dot(relative, edge) / (length * length);
		vector projected = worldA + (worldB - worldA) * raw;
		Check("projection_recorded_relative_float32_geometry", Math.AbsFloat(raw - 0.0323636) < 0.000001 && Math.AbsFloat(vector.DistanceXZ(worldQ, projected) - 0.0922335) < 0.0002);

		// Runtime IEEE overflow supplies malformed inputs without a division by
		// zero. If this native runtime handles overflow differently, the fixture
		// validity check fails rather than silently claiming nonfinite coverage.
		float nonfinite = 1e30;
		nonfinite = nonfinite * nonfinite;
		float notANumber = nonfinite - nonfinite;
		Check("projection_nonfinite_fixture_is_nonfinite", nonfinite - nonfinite != 0 && notANumber != notANumber);
		prior = PrepareProjectionWitness(route, 116, 1, g);
		before = route.GetProgress();
		Check("projection_infinite_measurement_rejected", !Physical(route, 116, query, nonfinite, g) && Near(route.GetProgress(), before));
		Physical(route, 116, query, 0.145877, g);
		Check("projection_nonfinite_failure_invalidates_context", g.State == CF_DrivenRoute.ADVANCE_LIMIT && !g.HasProjectionCorrection);
		prior = PrepareProjectionWitness(route, 117, 1, g);
		Check("projection_nan_measurement_rejected", !Physical(route, 117, query, notANumber, g));
		Check("projection_infinite_pose_rejected", !Physical(route, 117, Vector(nonfinite, 0, 0), 1, g));
		float allowed, turnCos, factor;
		string reason;
		Check("projection_nonfinite_corridor_rejected", !route.QueryPhysical(117, query, 1, 0.3, 0, nonfinite, allowed, turnCos, factor, reason, g));

		prior = PrepareProjectionWitness(route, 118, 1, g);
		Physical(route, 118, query, 0, g);
		Check("projection_zero_measurement_cannot_transfer", g.State == CF_DrivenRoute.ADVANCE_LIMIT && !g.HasProjectionCorrection);
		prior = PrepareProjectionWitness(route, 119, 1, g);
		route.Reset(119, incoming * -1.5);
		route.Record(119, vector.Zero);
		route.Record(119, outgoing);
		route.Record(119, outgoing * 2);
		Physical(route, 119, query, 0.145877, g);
		Check("projection_same_key_reset_cannot_reuse_context", g.State == CF_DrivenRoute.APPROACH_START && !g.HasArcGap && !g.HasProjectionCorrection && Near(route.GetProgress(), 0));
		prior = PrepareProjectionWitness(route, 120, 1, g);
		route.Record(120, Vector(100, 0, 100));
		Physical(route, 120, query, 0.145877, g);
		Check("projection_discontinuity_keeps_original_failure", g.State == CF_DrivenRoute.DISCONTINUITY && !g.HasProjectionCorrection);
		prior = PrepareProjectionWitness(route, 121, 1, g);
		for (int sample = 1; sample <= 270; sample++)
			route.Record(121, outgoing * 2 + Vector(sample, 0, 0));
		Physical(route, 121, query, 0.145877, g);
		Check("projection_history_loss_keeps_original_failure", g.State == CF_DrivenRoute.HISTORY_LOST && !g.HasProjectionCorrection);
		route.Reset(122, Vector(-20, 0, 0));
		route.Record(122, vector.Zero);
		route.Record(122, Vector(0, 0, 20));
		Physical(route, 122, Vector(-4.99, 0, 8.6), 20, g);
		Check("projection_adjacent_leg_cannot_fake_first_join", g.State == CF_DrivenRoute.APPROACH_START && !g.HasArcGap && !g.HasProjectionCorrection && Near(route.GetProgress(), 0));
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

		RunProjectionCases();
		if (m_Passed == m_Total)
			Print("[ConvoyFollower] DRIVEN_ROUTE_RESULT: PASS cases=" + m_Passed);
		else
			Print("[ConvoyFollower] DRIVEN_ROUTE_RESULT: FAIL passed=" + m_Passed + " total=" + m_Total);
	}
}
