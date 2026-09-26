// Focused forward-parking diagnostic. Reuse only the short course's owner
// possession, camera and lead-brake helpers. The production follower drives
// itself from a pre-staged arrival bay; no vehicle transform is changed.
class CF_ForwardSlotProbeComponentClass : CF_ShortArrivalProbeComponentClass
{
}

class CF_ForwardSlotProbeComponent : CF_ShortArrivalProbeComponent
{
	protected Vehicle m_TestTruck;
	protected CF_DriverControllerComponent m_TestDriver;
	protected vector m_vReleaseStart;
	protected vector m_vPreviousTruck;
	protected vector m_vParkAnchor;
	protected float m_fReleasePath;
	protected float m_fMaxParkDrift;
	protected int m_iParkSeconds;

	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		m_Lead = Vehicle.Cast(owner);
		if (!m_Lead)
			return;
		SetEventMask(owner, EntityEvent.POSTFRAME);
		CF_ConvoySettings.Get();
		GetGame().GetCallqueue().CallLater(Poll, 1000, true);
		Print("[ConvoyFollower] FORWARD_SLOT_INIT: expected=1 parking-only; fixed starts; no reposition");
	}

	override protected void Finish(string result)
	{
		if (m_bFinished)
			return;
		m_bFinished = true;
		m_bBrake = true;
		ApplyBrake();
		Print("[ConvoyFollower] FORWARD_SLOT_RESULT: " + result +
			" path=" + m_fReleasePath + " max_park_drift=" + m_fMaxParkDrift +
			" parked_seconds=" + m_iParkSeconds);
		GetGame().GetCallqueue().Remove(Poll);
	}

	protected void LogTruck()
	{
		CarControllerComponent car = CarControllerComponent.Cast(m_TestTruck.FindComponent(CarControllerComponent));
		if (!car || !car.GetSimulation())
			return;
		VehicleWheeledSimulation sim = car.GetSimulation();
		Print("[ConvoyFollower] FORWARD_SLOT_PHYSICS: stage=" + m_iStage +
			" pos=" + m_TestTruck.GetOrigin() + " path=" + m_fReleasePath +
			" displacement=" + vector.DistanceXZ(m_vReleaseStart, m_TestTruck.GetOrigin()) +
			" road_gap=" + RoadGap(m_TestTruck.GetOrigin()) +
			" seated=" + m_TestDriver.CF_IsBoarded() +
			" parked=" + m_TestDriver.CF_IsForwardWaitParked() +
			" speed=" + sim.GetSpeedKmh() + " throttle=" + sim.GetThrottle() +
			" brake=" + sim.GetBrake() + " gear=" + sim.GetGear() +
			" handbrake=" + sim.IsHandbrakeOn() + " persistent=" + car.GetPersistentHandBrake());
	}

	override protected void Poll()
	{
		if (m_bFinished)
			return;
		m_iTotalSeconds++;
		m_iStageSeconds++;
		if (m_iTotalSeconds > 210)
		{
			Finish("FAIL total timeout");
			return;
		}
		if (m_iStage == 0)
		{
			if (!EnsureOwnerPilot())
				return;
			m_TestTruck = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("CF_SmokeFollower1"));
			m_TestDriver = FindDriver(1);
			if (!m_TestTruck || !m_TestDriver)
				return;
			float leadRoadGap = RoadGap(m_Lead.GetOrigin());
			if (leadRoadGap < 8.0 || leadRoadGap > 10.0 || RoadGap(m_TestTruck.GetOrigin()) > 3.0)
			{
				Finish("FAIL surveyed start geometry changed");
				return;
			}
			SelectTestCamera();
			bool accepted = CF_ConvoySession.Start(m_Player, m_TestDriver);
			Print("[ConvoyFollower] FORWARD_SLOT_RECRUIT: accepted=" + accepted);
			if (!accepted)
			{
				Finish("FAIL recruit rejected");
				return;
			}
			m_iStage = 1;
			m_iStageSeconds = 0;
			return;
		}
		if (!OwnerIsPilot() || !m_TestDriver || !m_TestTruck)
		{
			Finish("FAIL owner, driver or truck unavailable");
			return;
		}
		if (m_iStage == 1)
		{
			bool ready = CF_ConvoySession.CanReleaseAtUnload(m_Player, m_TestDriver);
			if (m_iStageSeconds % 5 == 0)
				Print("[ConvoyFollower] FORWARD_SLOT_READY: ready=" + ready +
					" reason=" + m_TestDriver.CF_GetReleaseEligibilityReason() +
					" lead_gap=" + vector.Distance(m_Lead.GetOrigin(), m_TestTruck.GetOrigin()) +
					" truck_speed=" + TruckSpeed(m_TestTruck) +
					" road_gap=" + RoadGap(m_TestTruck.GetOrigin()));
			if (!ready)
			{
				if (m_iStageSeconds > 65)
					Finish("FAIL normal arrival did not become release-ready");
				return;
			}
			if (!m_TestDriver.CF_IsBoarded() || m_TestDriver.CF_GetAssignedVehicle() != m_TestTruck)
			{
				Finish("FAIL assigned driver seat mismatch");
				return;
			}
			m_vReleaseStart = m_TestTruck.GetOrigin();
			m_vPreviousTruck = m_vReleaseStart;
			bool released = CF_ConvoySession.CF_PanelPullAhead(m_Player, m_TestDriver.CF_GetUnitNumber());
			Print("[ConvoyFollower] FORWARD_SLOT_ORDER: accepted=" + released);
			if (!released)
			{
				Finish("FAIL forward parking order rejected");
				return;
			}
			m_iStage = 2;
			m_iStageSeconds = 0;
			return;
		}
		vector position = m_TestTruck.GetOrigin();
		m_fReleasePath += vector.DistanceXZ(m_vPreviousTruck, position);
		m_vPreviousTruck = position;
		LogTruck();
		if (!m_TestDriver.CF_IsBoarded() || m_TestDriver.CF_GetAssignedVehicle() != m_TestTruck)
		{
			Finish("FAIL driver left assigned seat");
			return;
		}
		if (m_iStage == 2)
		{
			if (m_TestDriver.CF_IsForwardWaitParked())
			{
				m_vParkAnchor = position;
				m_iStage = 3;
				m_iStageSeconds = 0;
				Print("[ConvoyFollower] FORWARD_SLOT_PARK_LATCH: pos=" + position);
			}
			else if (m_iStageSeconds > 130)
				Finish("FAIL forward park did not complete");
			return;
		}
		float drift = vector.DistanceXZ(m_vParkAnchor, position);
		if (drift > m_fMaxParkDrift)
			m_fMaxParkDrift = drift;
		if (!m_TestDriver.CF_IsForwardWaitParked() || drift > 0.4 || TruckSpeed(m_TestTruck) > 0.5)
		{
			Finish("FAIL parked truck moved or lost parked state");
			return;
		}
		m_iParkSeconds++;
		if (m_iParkSeconds < 20)
			return;
		if (m_fReleasePath < 60.0 || vector.DistanceXZ(m_vReleaseStart, position) < 60.0 || RoadGap(position) > 4.0)
		{
			Finish("FAIL parking displacement or road-position evidence insufficient");
			return;
		}
		Finish("PASS physical forward parking held seated and stationary for twenty seconds");
	}
}
