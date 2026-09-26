// Pure conservative pacing envelope; no entity, route or control ownership.
// Input/output speeds are km/h. Gap is vehicle-origin separation in metres.
// This is an experimental advisory cap, not a collision-free path guarantee.
class CF_FollowSpeedPolicy
{
	static float MaxSpeedKmh(float gapMeters, float stoppedGapMeters, float leadSpeedKmh)
	{
		float freeGap = gapMeters - stoppedGapMeters - 2.0;
		if (freeGap < 0)
			freeGap = 0;
		float leadMps = leadSpeedKmh / 3.6;
		if (leadMps < 0)
			leadMps = 0;
		// a=2 m/s^2, reaction allowance=1.5 s, hence a*t=3 m/s.
		float limit = (Math.Sqrt(9.0 + leadMps * leadMps + 4.0 * freeGap) - 3.0) * 3.6;
		if (limit < 0)
			return 0;
		if (limit > 45.0)
			return 45.0;
		return limit;
	}
}
