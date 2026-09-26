import { readFileSync } from "node:fs";
import path from "node:path";
import { describe, expect, it } from "vitest";

import { parseSmokeArgs, summarizeSmokeLog } from "../src/convoy-smoke-cli.js";

describe("convoy smoke runner", () => {
  it("selects scenes with one, two, or three follower groups and a lead vehicle", () => {
    for (const trucks of [1, 2, 3] as const) {
      const options = parseSmokeArgs(["--trucks", String(trucks)], new Date("2026-09-25T00:00:00Z"), 123);
      expect(options.trucks).toBe(trucks);
      const layerName = `ConvoyFollower_Arland_Auto_${trucks}Truck${trucks === 1 ? "" : "s"}_Layers`;
      const layer = readFileSync(path.resolve("addons", "ConvoyFollower", "Worlds", "Tests", layerName, "default.layer"), "utf8");
      expect((layer.match(/SCR_AIGroup\s+CF_SmokeGroup\d\s*:/g) ?? []).length).toBe(trucks);
      const vehicleCount = (layer.match(/M923A1_transport\.et/g) ?? []).length;
      expect(vehicleCount).toBe(trucks + 1);
      expect(layer).toContain(`m_iExpectedTrucks ${trucks}`);
    }
  });

  it("reports only evidence actually present in the log", () => {
    const log = [
      "WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Arland_Auto_2Trucks.ent'",
      "SCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME",
      "SCRIPT : [ConvoyFollower] FOLLOWING: Unit 1 moving toward predecessor vehicle",
      "SCRIPT : [ConvoyFollower] FOLLOWING: Unit 2 moving toward predecessor vehicle",
      "SCRIPT : [ConvoyFollower] AUTO_RESULT: PASS lead moved=35 moving_followers=2",
      "SCRIPT (E): simulated unrelated error",
    ].join("\n");
    const report = summarizeSmokeLog(log, 2);
    expect(report.observedWorld).toBe(true);
    expect(report.enteredGame).toBe(true);
    expect(report.eventCounts.FOLLOWING).toBe(2);
    expect(report.autoResult).toContain("PASS");
    expect(report.passed).toBe(false);
    expect(report.errorLines).toHaveLength(1);
    expect(summarizeSmokeLog(log, 1).observedWorld).toBe(false);
  });

  it("selects the separate open-road baseline when requested", () => {
    for (const trucks of [1, 2, 3] as const) {
      const options = parseSmokeArgs(["--trucks", String(trucks), "--variant", "open-road"]);
      expect(options.variant).toBe("open-road");
      const layerName = `ConvoyFollower_Arland_OpenRoad_Auto_${trucks}Truck${trucks === 1 ? "" : "s"}_Layers`;
      const layer = readFileSync(path.resolve("addons", "ConvoyFollower", "Worlds", "Tests", layerName, "default.layer"), "utf8");
      expect((layer.match(/SCR_AIGroup\s+CF_SmokeGroup\d\s*:/g) ?? []).length).toBe(trucks);
      expect((layer.match(/M923A1_transport\.et/g) ?? []).length).toBe(trucks + 1);
      expect(layer).toContain(`m_iExpectedTrucks ${trucks}`);
      expect(layer).toContain("coords 1371.06 20.1534 3330.05");
      expect(layer).toContain("angles 0 92 0");
    }
    const log = "WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_3Trucks.ent'";
    expect(summarizeSmokeLog(log, 3, "open-road").observedWorld).toBe(true);
    expect(summarizeSmokeLog(log, 3).observedWorld).toBe(false);
    expect(() => parseSmokeArgs(["--trucks", "3", "--variant", "unknown"])).toThrow("--variant");
  });

  it("requires a parked return sequence in the dedicated two-truck release world", () => {
    const options = parseSmokeArgs(["--trucks", "2", "--variant", "release"]);
    expect(options.variant).toBe("release");
    const layer = readFileSync(path.resolve("addons", "ConvoyFollower", "Worlds", "Tests",
      "ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_Release_Layers", "default.layer"), "utf8");
    expect(layer).toContain("CF_UnloadSequenceProbeComponent");
    const log = [
      "WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_Release.ent'",
      "SCRIPT : [ConvoyFollower] AUTO_INIT: expected=2",
      "SCRIPT : [ConvoyFollower] AUTO_RESULT: PASS road arrival",
      "SCRIPT : [ConvoyFollower] AUTO_SEQUENCE_RESULT: FAIL truck did not park",
    ].join("\n");
    const report = summarizeSmokeLog(log, 2, "release");
    expect(report.observedWorld).toBe(true);
    expect(report.autoResult).toContain("PASS");
    expect(report.sequenceResult).toContain("FAIL");
    expect(report.passed).toBe(false);
    expect(report.interpretation).toContain("No passing unload");
    expect(() => parseSmokeArgs(["--trucks", "1", "--variant", "release"])).toThrow("requires --trucks 2");
  });

  it("requires a real stop, exit, reboard, and resumed drive in the pause-resume world", () => {
    const options = parseSmokeArgs(["--trucks", "2", "--variant", "pause-resume"]);
    expect(options.variant).toBe("pause-resume");
    const layer = readFileSync(path.resolve("addons", "ConvoyFollower", "Worlds", "Tests",
      "ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_PauseResume_Layers", "default.layer"), "utf8");
    expect(layer).toContain("m_iExpectedTrucks 2");
    expect((layer.match(/SCR_AIGroup\s+CF_SmokeGroup\d\s*:/g) ?? []).length).toBe(2);
    const base = [
      "WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_PauseResume.ent'",
      "SCRIPT : [ConvoyFollower] AUTO_INIT: expected=2",
      "SCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME",
      "SCRIPT : [ConvoyFollower] AUTO_EN_ROUTE_METRICS: max_gap=105 warning_s=0 no_progress_s=2",
      "SCRIPT : [ConvoyFollower] AUTO_RESULT: PASS stable road arrival",
    ];
    expect(summarizeSmokeLog(base.join("\n"), 2, "pause-resume").passed).toBe(false);
    const passed = summarizeSmokeLog([...base,
      "SCRIPT : [ConvoyFollower] AUTO_PAUSE_RESULT: PASS player exited and reboarded; both units stayed seated and resumed 100 m",
    ].join("\n"), 2, "pause-resume");
    expect(passed.passed).toBe(true);
    expect(passed.pauseResult).toContain("PASS");
    expect(summarizeSmokeLog([...base,
      "SCRIPT : [ConvoyFollower] AUTO_PAUSE_RESULT: FAIL player never reboarded",
    ].join("\n"), 2, "pause-resume").passed).toBe(false);
    expect(() => parseSmokeArgs(["--trucks", "1", "--variant", "pause-resume"])).toThrow("requires --trucks 2");
  });

  it("requires physical clearance and successor advance in the separate forward-wait world", () => {
    expect(parseSmokeArgs(["--trucks", "2", "--variant", "forward-wait"]).variant).toBe("forward-wait");
    const layer = readFileSync(path.resolve("addons", "ConvoyFollower", "Worlds", "Tests",
      "ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_ForwardWait_Layers", "default.layer"), "utf8");
    expect(layer).toContain("m_bStraightReleaseRoadGoal 1");
    expect((layer.match(/SCR_AIGroup\s+CF_SmokeGroup\d\s*:/g) ?? []).length).toBe(2);
    const base = [
      "WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_ForwardWait.ent'",
      "SCRIPT : [ConvoyFollower] AUTO_INIT: expected=2",
      "SCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME",
      "SCRIPT : [ConvoyFollower] AUTO_EN_ROUTE_METRICS: max_gap=101 warning_s=0 no_progress_s=1",
      "SCRIPT : [ConvoyFollower] AUTO_RESULT: PASS stable road arrival",
    ];
    expect(summarizeSmokeLog(base.join("\n"), 2, "forward-wait").passed).toBe(false);
    const good = summarizeSmokeLog([...base,
      "SCRIPT : [ConvoyFollower] AUTO_FORWARD_RESULT: PASS unit1 parked forward and unit2 advanced",
    ].join("\n"), 2, "forward-wait");
    expect(good.passed).toBe(true);
    expect(good.forwardResult).toContain("PASS");
    expect(() => parseSmokeArgs(["--trucks", "3", "--variant", "forward-wait"])).toThrow("requires --trucks 2");
  });

  it("passes the separate lane-blocked case only for the specific refusal and stationary seated probe", () => {
    expect(parseSmokeArgs(["--trucks", "2", "--variant", "forward-blocked"]).variant).toBe("forward-blocked");
    const layer = readFileSync(path.resolve("addons", "ConvoyFollower", "Worlds", "Tests",
      "ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_ForwardBlocked_Layers", "default.layer"), "utf8");
    expect(layer).toContain("CF_ForwardBlockedProbeComponent");
    expect(layer).toContain("m_bStraightReleaseRoadGoal 1");
    const base = [
      "WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_ForwardBlocked.ent'",
      "SCRIPT : [ConvoyFollower] AUTO_INIT: expected=2",
      "SCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME",
      "SCRIPT : [ConvoyFollower] AUTO_EN_ROUTE_METRICS: max_gap=90 warning_s=0 no_progress_s=2",
      "SCRIPT : [ConvoyFollower] AUTO_RESULT: PASS stable road arrival",
    ];
    const blocked = "SCRIPT : [ConvoyFollower] FORWARD_WAIT_REJECTED: Move your lead vehicle off the driving lane to let this truck pass";
    const probePass = "SCRIPT : [ConvoyFollower] AUTO_FORWARD_BLOCKED_RESULT: PASS center-lane lead rejected; Unit 1 remained seated and stationary";
    expect(summarizeSmokeLog([...base, blocked, probePass].join("\n"), 2, "forward-blocked").passed).toBe(true);
    expect(summarizeSmokeLog([...base, probePass].join("\n"), 2, "forward-blocked").passed).toBe(false);
    expect(summarizeSmokeLog([...base, blocked, probePass,
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_REQUESTED: unsafe move was accepted",
    ].join("\n"), 2, "forward-blocked").passed).toBe(false);
    expect(summarizeSmokeLog([...base, blocked,
      "SCRIPT : [ConvoyFollower] AUTO_FORWARD_BLOCKED_RESULT: FAIL truck moved",
    ].join("\n"), 2, "forward-blocked").passed).toBe(false);
    expect(() => parseSmokeArgs(["--trucks", "3", "--variant", "forward-blocked"])).toThrow("requires --trucks 2");
  });

  it("requires two physical forward parks and an explicit resumed three-truck chain", () => {
    expect(parseSmokeArgs(["--trucks", "3", "--variant", "forward-twoahead"]).variant).toBe("forward-twoahead");
    const layer = readFileSync(path.resolve("addons", "ConvoyFollower", "Worlds", "Tests",
      "ConvoyFollower_Arland_OpenRoad_Auto_3Trucks_ForwardTwoAhead_Layers", "default.layer"), "utf8");
    expect(layer).toContain("CF_ForwardTwoAheadProbeComponent");
    expect(layer).toContain("m_iFilmForwardMode 1");
    expect((layer.match(/SCR_AIGroup\s+CF_SmokeGroup\d\s*:/g) ?? []).length).toBe(3);
    const base = [
      "WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_3Trucks_ForwardTwoAhead.ent'",
      "SCRIPT : [ConvoyFollower] AUTO_INIT: expected=3",
      "SCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME",
      "SCRIPT : [ConvoyFollower] AUTO_EN_ROUTE_METRICS: max_gap=125 warning_s=0 no_progress_s=4",
      "SCRIPT : [ConvoyFollower] AUTO_RESULT: PASS stable road arrival",
    ];
    const sequence = [
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_PARKED: Unit 1",
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_BAY_CLEAR: Unit 1",
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_PARKED: Unit 2",
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_BAY_CLEAR: Unit 2",
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_PARKED: spacing=19 projected_order_spacing=18",
      "SCRIPT : [ConvoyFollower] FORWARD_OUTBOUND_HOLD_REQUESTED: owner passed 20m",
      "SCRIPT : [ConvoyFollower] FORWARD_OUTBOUND_HOLD_COMPLETE: Unit 3 seated",
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_RESUME_LINE: owner passed parked line",
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_POST_RESUME: owner/Unit1/Unit2/Unit3 positions",
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_FORWARD_PROGRESS: owner=31 goal_closure=28 unit1=14 unit2=13 unit3=10",
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_RESULT: PASS two trucks parked and convoy resumed",
    ];
    const good = summarizeSmokeLog([...base, ...sequence].join("\n"), 3, "forward-twoahead");
    expect(good.passed).toBe(true);
    expect(good.twoAheadParkingPassed).toBe(true);
    expect(good.twoAheadHoldObserved).toBe(true);
    expect(good.twoAheadHoldPassed).toBe(true);
    expect(good.twoAheadForwardProgressPassed).toBe(true);
    expect(summarizeSmokeLog([...base, ...sequence].join("\n").replace("owner=31", "owner=-31"), 3, "forward-twoahead").passed).toBe(false);
    expect(summarizeSmokeLog([...base, ...sequence].join("\n").replace("unit3=10", "unit3=0"), 3, "forward-twoahead").passed).toBe(false);
    expect(summarizeSmokeLog([...base, ...sequence.slice(0, 2), sequence.at(-1)!].join("\n"), 3, "forward-twoahead").passed).toBe(false);
    expect(summarizeSmokeLog([...base, ...sequence.slice(0, -1),
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_RESULT: FAIL physical inversion",
    ].join("\n"), 3, "forward-twoahead").passed).toBe(false);
    const failedPassAfterParking = summarizeSmokeLog([...base, ...sequence.slice(0, 5),
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_RESULT: FAIL owner did not pass",
    ].join("\n"), 3, "forward-twoahead");
    expect(failedPassAfterParking.passed).toBe(false);
    expect(failedPassAfterParking.twoAheadParkingPassed).toBe(true);
    expect(failedPassAfterParking.twoAheadHoldObserved).toBe(false);
    expect(failedPassAfterParking.twoAheadHoldPassed).toBe(false);
    const missingCompletedHold = summarizeSmokeLog([...base, ...sequence.filter((line) =>
      !line.includes("FORWARD_OUTBOUND_HOLD_COMPLETE"))].join("\n"), 3, "forward-twoahead");
    expect(missingCompletedHold.twoAheadHoldObserved).toBe(false);
    expect(missingCompletedHold.passed).toBe(false);
    const falseStall = summarizeSmokeLog([...base, ...sequence.slice(0, -1),
      "SCRIPT : [ConvoyFollower] STUCK_TERMINAL: Unit 3 left while waiting",
      "SCRIPT : [ConvoyFollower] CONVOY_UNIT_REMOVED: former position 1",
      sequence.at(-1)!,
    ].join("\n"), 3, "forward-twoahead");
    expect(falseStall.twoAheadParkingPassed).toBe(true);
    expect(falseStall.twoAheadHoldPassed).toBe(false);
    expect(falseStall.passed).toBe(false);
    const shutdownAfterPass = summarizeSmokeLog([...base, ...sequence,
      "SCRIPT : [ConvoyFollower] CONVOY_UNIT_REMOVED: F5 teardown after terminal PASS",
    ].join("\n"), 3, "forward-twoahead");
    expect(shutdownAfterPass.twoAheadHoldPassed).toBe(true);
    expect(shutdownAfterPass.passed).toBe(true);
    expect(() => parseSmokeArgs(["--trucks", "2", "--variant", "forward-twoahead"])).toThrow("requires --trucks 3");
  });

  it("keeps the surveyed width-8 road fixture distinct from the failed narrow two-ahead fixture", () => {
    expect(parseSmokeArgs(["--trucks", "3", "--variant", "forward-twoahead-wide"]).variant).toBe("forward-twoahead-wide");
    expect(() => parseSmokeArgs(["--trucks", "2", "--variant", "forward-twoahead-wide"])).toThrow("requires --trucks 3");
    const layer = readFileSync(path.resolve("addons", "ConvoyFollower", "Worlds", "Tests",
      "ConvoyFollower_Arland_WideRoad_Auto_3Trucks_ForwardTwoAhead_Layers", "default.layer"), "utf8");
    expect(layer).toContain("m_bWideRoadGoal 1");
    expect(layer).toContain("CF_ForwardTwoAheadProbeComponent");
    expect((layer.match(/SCR_AIGroup\s+CF_SmokeGroup\d\s*:/g) ?? []).length).toBe(3);
    const log = [
      "WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Arland_WideRoad_Auto_3Trucks_ForwardTwoAhead.ent'",
      "SCRIPT : [ConvoyFollower] AUTO_INIT: expected=3",
      "SCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME",
      "SCRIPT : [ConvoyFollower] AUTO_EN_ROUTE_METRICS: max_gap=125 warning_s=0 no_progress_s=4",
      "SCRIPT : [ConvoyFollower] AUTO_RESULT: PASS stable road arrival",
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_PARKED: Unit 1",
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_BAY_CLEAR: Unit 1",
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_PARKED: Unit 2",
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_BAY_CLEAR: Unit 2",
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_PARKED: spacing=19 projected_order_spacing=18",
      "SCRIPT : [ConvoyFollower] FORWARD_OUTBOUND_HOLD_REQUESTED: owner passed 20m",
      "SCRIPT : [ConvoyFollower] FORWARD_OUTBOUND_HOLD_COMPLETE: Unit 3 seated",
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_RESUME_LINE: owner passed parked line",
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_POST_RESUME: owner/Unit1/Unit2/Unit3 positions",
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_FORWARD_PROGRESS: owner=31 goal_closure=28 unit1=14 unit2=13 unit3=10",
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_RESULT: PASS physical chain resumed",
    ];
    const report = summarizeSmokeLog(log.join("\n"), 3, "forward-twoahead-wide");
    expect(report.expectedWorld).toContain("WideRoad_Auto_3Trucks_ForwardTwoAhead");
    expect(report.passed).toBe(true);
    expect(summarizeSmokeLog(log.join("\n"), 3, "forward-twoahead").passed).toBe(false);
    expect(summarizeSmokeLog([...log.slice(0, -1),
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_RESULT: FAIL no owner pass",
    ].join("\n"), 3, "forward-twoahead-wide").passed).toBe(false);
  });

  it("requires the full physical resume gate in the separate road-23 atlas fixture", () => {
    expect(parseSmokeArgs(["--trucks", "3", "--variant", "forward-twoahead-road23"]).variant).toBe("forward-twoahead-road23");
    expect(() => parseSmokeArgs(["--trucks", "2", "--variant", "forward-twoahead-road23"])).toThrow("requires --trucks 3");
    const layer = readFileSync(path.resolve("addons", "ConvoyFollower", "Worlds", "Tests",
      "ConvoyFollower_Arland_Road23_Auto_3Trucks_ForwardTwoAhead_Layers", "default.layer"), "utf8");
    expect(layer).toContain("m_bRoad23Goal 1");
    expect(layer).toContain("m_bRoad23Fixture 1");
    const prefix = [
      "WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Arland_Road23_Auto_3Trucks_ForwardTwoAhead.ent'",
      "SCRIPT : [ConvoyFollower] AUTO_INIT: expected=3",
      "SCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME",
      "SCRIPT : [ConvoyFollower] AUTO_EN_ROUTE_METRICS: max_gap=125 warning_s=0 no_progress_s=4",
      "SCRIPT : [ConvoyFollower] AUTO_RESULT: PASS stable road arrival",
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_PARKED: Unit 1",
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_BAY_CLEAR: Unit 1",
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_PARKED: Unit 2",
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_BAY_CLEAR: Unit 2",
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_PARKED: spacing=19 projected_order_spacing=18",
      "SCRIPT : [ConvoyFollower] FORWARD_OUTBOUND_HOLD_REQUESTED: owner passed 20m",
      "SCRIPT : [ConvoyFollower] FORWARD_OUTBOUND_HOLD_COMPLETE: Unit 3 seated",
    ];
    expect(summarizeSmokeLog(prefix.join("\n"), 3, "forward-twoahead-road23").passed).toBe(false);
    const complete = [...prefix,
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_RESUME_LINE: owner passed parked line",
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_POST_RESUME: owner/Unit1/Unit2/Unit3 positions",
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_FORWARD_PROGRESS: owner=31 goal_closure=28 unit1=14 unit2=13 unit3=10",
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_RESULT: PASS physical chain resumed",
    ];
    expect(summarizeSmokeLog(complete.join("\n"), 3, "forward-twoahead-road23").passed).toBe(true);
  });

  it("requires the full physical resume gate in the separate road-81 atlas fixture", () => {
    expect(parseSmokeArgs(["--trucks", "3", "--variant", "forward-twoahead-road81"]).variant).toBe("forward-twoahead-road81");
    expect(() => parseSmokeArgs(["--trucks", "2", "--variant", "forward-twoahead-road81"])).toThrow("requires --trucks 3");
    const layer = readFileSync(path.resolve("addons", "ConvoyFollower", "Worlds", "Tests",
      "ConvoyFollower_Arland_Road81_Auto_3Trucks_ForwardTwoAhead_Layers", "default.layer"), "utf8");
    expect(layer).toContain("m_bRoad81Goal 1");
    expect(layer).toContain("m_bRoad81Fixture 1");
    const prefix = [
      "WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Arland_Road81_Auto_3Trucks_ForwardTwoAhead.ent'",
      "SCRIPT : [ConvoyFollower] AUTO_INIT: expected=3",
      "SCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME",
      "SCRIPT : [ConvoyFollower] AUTO_EN_ROUTE_METRICS: max_gap=125 warning_s=0 no_progress_s=4",
      "SCRIPT : [ConvoyFollower] AUTO_RESULT: PASS stable road arrival",
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_PARKED: Unit 1",
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_BAY_CLEAR: Unit 1",
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_PARKED: Unit 2",
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_BAY_CLEAR: Unit 2",
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_PARKED: spacing=19 projected_order_spacing=18",
      "SCRIPT : [ConvoyFollower] FORWARD_OUTBOUND_HOLD_REQUESTED: owner passed 20m",
      "SCRIPT : [ConvoyFollower] FORWARD_OUTBOUND_HOLD_COMPLETE: Unit 3 seated",
    ];
    expect(summarizeSmokeLog(prefix.join("\n"), 3, "forward-twoahead-road81").passed).toBe(false);
    const complete = [...prefix,
      "SCRIPT : [ConvoyFollower] FORWARD_WAIT_RESUME_LINE: owner passed parked line",
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_POST_RESUME: owner/Unit1/Unit2/Unit3 positions",
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_FORWARD_PROGRESS: owner=31 goal_closure=28 unit1=14 unit2=13 unit3=10",
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_RESULT: PASS physical chain resumed",
    ];
    expect(summarizeSmokeLog(complete.join("\n"), 3, "forward-twoahead-road81").passed).toBe(true);
  });

  it("rejects a gameplay PASS when the selected F5 run has an engine or script error", () => {
    const goodRun = [
      "WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_ForwardWait.ent'",
      "SCRIPT : [ConvoyFollower] AUTO_INIT: expected=2",
      "SCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME",
      "SCRIPT : [ConvoyFollower] AUTO_EN_ROUTE_METRICS: max_gap=101 warning_s=0 no_progress_s=1",
      "SCRIPT : [ConvoyFollower] AUTO_RESULT: PASS stable road arrival",
      "SCRIPT : [ConvoyFollower] AUTO_FORWARD_RESULT: PASS unit1 parked forward and unit2 advanced",
    ];
    expect(summarizeSmokeLog(goodRun.join("\n"), 2, "forward-wait").passed).toBe(true);
    const errorAfterInit = summarizeSmokeLog([...goodRun, "SCRIPT (E): convoy action exception"].join("\n"), 2, "forward-wait");
    expect(errorAfterInit.passed).toBe(false);
    expect(errorAfterInit.errorLines).toHaveLength(1);
    expect(errorAfterInit.interpretation).toContain("Inspect them");
    const errorBeforeInit = summarizeSmokeLog([goodRun[0], "WORLD (E): failed to resolve a scene entity", ...goodRun.slice(1)].join("\n"), 2, "forward-wait");
    expect(errorBeforeInit.passed).toBe(false);
    expect(errorBeforeInit.errorLines).toHaveLength(1);
    const knownBaseLoad = "WORLD (E): Unknown keyword/data 'SlidingTrackMaterial' at offset 19441(0x4bf1)";
    const withKnownBaseline = summarizeSmokeLog([goodRun[0], knownBaseLoad, ...goodRun.slice(1)].join("\n"), 2, "forward-wait");
    expect(withKnownBaseline.passed).toBe(true);
    expect(withKnownBaseline.errorLines).toHaveLength(0);
    expect(withKnownBaseline.baselineLoadErrorLines).toHaveLength(1);
    const sameErrorDuringGame = summarizeSmokeLog([...goodRun, knownBaseLoad].join("\n"), 2, "forward-wait");
    expect(sameErrorDuringGame.passed).toBe(false);
    expect(sameErrorDuringGame.errorLines).toHaveLength(1);

    // Vanilla Arland loads this exact base helipad after GAME and AUTO_INIT.
    // It is excused only in the adjacent prefab-load sequence before gameplay.
    const vanillaHelipadPrefab = 'WORLD : Entity prefab load @"{472BE7BF1240C1CE}Prefabs/Compositions/Misc/SubCompositions/Utility/Helipad_Lights_US_01.et"';
    const vanillaHelipadLoad = "WORLD (E): Unknown keyword/data 'm_bShowDebugShape' at offset 2307(0x903)";
    const gameplayStarted = "SCRIPT : [ConvoyFollower] AUTO_PLAYER: spawned and possessed test character";
    const withVanillaHelipadLoad = summarizeSmokeLog([
      ...goodRun.slice(0, 3), vanillaHelipadPrefab, vanillaHelipadLoad, gameplayStarted, ...goodRun.slice(3),
    ].join("\n"), 2, "forward-wait");
    expect(withVanillaHelipadLoad.passed).toBe(true);
    expect(withVanillaHelipadLoad.errorLines).toHaveLength(0);
    expect(withVanillaHelipadLoad.baselineLoadErrorLines).toEqual([vanillaHelipadLoad]);
    const helipadErrorDuringGame = summarizeSmokeLog([
      ...goodRun.slice(0, 3), gameplayStarted, vanillaHelipadPrefab, vanillaHelipadLoad, ...goodRun.slice(3),
    ].join("\n"), 2, "forward-wait");
    expect(helipadErrorDuringGame.passed).toBe(false);
    expect(helipadErrorDuringGame.errorLines).toEqual([vanillaHelipadLoad]);
    const noHelipadAssetContext = summarizeSmokeLog([
      ...goodRun.slice(0, 3), vanillaHelipadLoad, gameplayStarted, ...goodRun.slice(3),
    ].join("\n"), 2, "forward-wait");
    expect(noHelipadAssetContext.passed).toBe(false);
    expect(noHelipadAssetContext.errorLines).toEqual([vanillaHelipadLoad]);
    const changedHelipadOffset = vanillaHelipadLoad.replace("2307(0x903)", "2308(0x904)");
    const unknownHelipadError = summarizeSmokeLog([
      ...goodRun.slice(0, 3), vanillaHelipadPrefab, changedHelipadOffset, gameplayStarted, ...goodRun.slice(3),
    ].join("\n"), 2, "forward-wait");
    expect(unknownHelipadError.passed).toBe(false);
    expect(unknownHelipadError.errorLines).toEqual([changedHelipadOffset]);
  });

  it("keeps appended Workbench previews from supplying stale pass evidence", () => {
    const log = [
      "WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Arland_Auto_1Truck.ent'",
      "SCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME",
      "SCRIPT : [ConvoyFollower] AUTO_INIT: expected=1",
      "SCRIPT : [ConvoyFollower] AUTO_RESULT: PASS old run",
      "WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Arland_Auto_2Trucks.ent'",
      "SCRIPT : [ConvoyFollower] AUTO_INIT: expected=2",
      "SCRIPT : [ConvoyFollower] AUTO_ORDER: unit=1 accepted=true",
    ].join("\n");
    const report = summarizeSmokeLog(log, 2);
    expect(report.observedWorld).toBe(true);
    expect(report.enteredGame).toBe(false);
    expect(report.autoResult).toBeUndefined();
    expect(report.eventCounts.AUTO_ORDER).toBe(1);
  });

  it("reports the last completed F5 session when edit-mode reload adds another AUTO_INIT", () => {
    const log = [
      "WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_3Trucks.ent'",
      "SCRIPT : [ConvoyFollower] AUTO_INIT: expected=3",
      "SCRIPT : [ConvoyFollower] AUTO_INIT: expected=3",
      "SCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME",
      "SCRIPT : [ConvoyFollower] AUTO_EN_ROUTE_METRICS: max_gap=107.2 warning_s=0 no_progress_s=4",
      "SCRIPT : [ConvoyFollower] AUTO_RESULT: PASS stable road arrival lead_path=357",
      "INIT : Workbench Reload Game",
      "SCRIPT : [ConvoyFollower] AUTO_INIT: expected=3",
    ].join("\n");
    const report = summarizeSmokeLog(log, 3, "open-road");
    expect(report.observedWorld).toBe(true);
    expect(report.enteredGame).toBe(true);
    expect(report.observedExpectedCount).toBe(true);
    expect(report.enRouteMetrics).toEqual({ maxGap: 107.2, warningSeconds: 0, noProgressSeconds: 4 });
    expect(report.eventCounts.AUTO_INIT).toBe(1);
    expect(report.passed).toBe(true);
    expect(summarizeSmokeLog(log, 2, "open-road").passed).toBe(false);
  });

  it("requires an en-route quality result and a physical return result", () => {
    const base = [
      "WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_Release.ent'",
      "SCRIPT : [ConvoyFollower] AUTO_INIT: expected=2",
      "SCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME",
      "SCRIPT : [ConvoyFollower] AUTO_RESULT: PASS stable road arrival",
    ];
    const noMetrics = summarizeSmokeLog([...base, "SCRIPT : [ConvoyFollower] AUTO_SEQUENCE_RESULT: PASS returned"].join("\n"), 2, "release");
    expect(noMetrics.passed).toBe(false);
    const badQuality = summarizeSmokeLog([...base,
      "SCRIPT : [ConvoyFollower] AUTO_EN_ROUTE_METRICS: max_gap=210 warning_s=9 no_progress_s=3",
      "SCRIPT : [ConvoyFollower] AUTO_SEQUENCE_RESULT: PASS returned",
    ].join("\n"), 2, "release");
    expect(badQuality.passed).toBe(false);
    const good = summarizeSmokeLog([...base,
      "SCRIPT : [ConvoyFollower] AUTO_EN_ROUTE_METRICS: max_gap=125 warning_s=0 no_progress_s=2",
      "SCRIPT : [ConvoyFollower] AUTO_SEQUENCE_RESULT: PASS explicit release parked, next truck advanced, and both returned",
    ].join("\n"), 2, "release");
    expect(good.passed).toBe(true);
  });

  it("requires physical parking, bay advance, and return evidence in the wide-road release fixture", () => {
    expect(parseSmokeArgs(["--trucks", "2", "--variant", "release-wide"]).variant).toBe("release-wide");
    expect(() => parseSmokeArgs(["--trucks", "3", "--variant", "release-wide"])).toThrow("requires --trucks 2");
    const base = [
      "WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Arland_WideRoad_Auto_2Trucks_Release.ent'",
      "SCRIPT : [ConvoyFollower] AUTO_INIT: expected=2",
      "SCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME",
      "SCRIPT : [ConvoyFollower] AUTO_RESULT: PASS stable road arrival",
      "SCRIPT : [ConvoyFollower] AUTO_EN_ROUTE_METRICS: max_gap=125 warning_s=0 no_progress_s=2",
    ];
    const result = "SCRIPT : [ConvoyFollower] AUTO_SEQUENCE_RESULT: PASS explicit release parked, next truck advanced, and both returned";
    expect(summarizeSmokeLog([...base, result].join("\n"), 2, "release-wide").passed).toBe(false);
    expect(summarizeSmokeLog([...base,
      "SCRIPT : [ConvoyFollower] AUTO_SEQUENCE_PARKED: unit1 seated; unit2 advanced to bay",
      "SCRIPT : [ConvoyFollower] AUTO_SEQUENCE_RETURN_MERGED: both drivers seated in active convoy",
      "SCRIPT : [ConvoyFollower] AUTO_SEQUENCE_RETURN_DRIVE: goal=<1268,34,3005>",
      result,
    ].join("\n"), 2, "release-wide").passed).toBe(true);
  });

  it("requires an explicit count and avoids a timed keep-open run", () => {
    expect(() => parseSmokeArgs([])).toThrow("--trucks");
    expect(() => parseSmokeArgs(["--trucks", "4"])).toThrow("--trucks");
    expect(() => parseSmokeArgs(["--trucks", "2", "--keep-open", "--duration-seconds", "30"])).toThrow("cannot be combined");
    expect(() => parseSmokeArgs(["--trucks", "2", "--require-pass"])).toThrow("requires --report");
  });

  it("requires measured movement and authoritative Hold/Resume in the short-arrival fixture", () => {
    for (const trucks of [1, 2] as const) {
      expect(parseSmokeArgs(["--trucks", String(trucks), "--variant", "short-arrival"]).variant).toBe("short-arrival");
      const basename = `ConvoyFollower_Arland_Road81_ShortArrival_${trucks}Truck${trucks === 1 ? "" : "s"}`;
      const world = readFileSync(path.resolve("addons/ConvoyFollower/Worlds/Tests", `${basename}.ent`), "utf8");
      const layer = readFileSync(path.resolve("addons/ConvoyFollower/Worlds/Tests", `${basename}_Layers/default.layer`), "utf8");
      expect(world).toMatch(/^SubScene\s*\{/);
      expect(world).not.toContain("CF_SmokeLead");
      expect((layer.match(/SCR_AIGroup\s+CF_SmokeGroup\d\s*:/g) ?? []).length).toBe(trucks);
      expect((layer.match(/Vehicle\s+CF_Smoke/g) ?? []).length).toBe(trucks + 1);
      expect((layer.match(/CF_ShortArrivalProbeComponent/g) ?? []).length).toBe(1);
      const log = shortArrivalLog(trucks);
      const report = summarizeSmokeLog(log, trucks, "short-arrival");
      expect(report.passed).toBe(true);
      expect(report.shortArrivalPhysicalPassed).toBe(true);
      expect(report.enRouteMetrics).toBeUndefined();
      expect(summarizeSmokeLog(log.replace("SHORT_ARRIVAL_HOLD_COMPLETED", "SHORT_ARRIVAL_HOLD_PENDING"), trucks, "short-arrival").passed).toBe(false);
      expect(summarizeSmokeLog(log.replaceAll("second_displacement=25", "second_displacement=0"), trucks, "short-arrival").passed).toBe(false);
      expect(summarizeSmokeLog(log.replace("signed_arc_progress=52", "signed_arc_progress=-52"), trucks, "short-arrival").passed).toBe(false);
      expect(summarizeSmokeLog(log.replace("second_arc_progress=58", "second_arc_progress=-58"), trucks, "short-arrival").passed).toBe(false);
      expect(summarizeSmokeLog(log.replace("second_elevation_gain=2", "second_elevation_gain=-2"), trucks, "short-arrival").passed).toBe(false);
      expect(summarizeSmokeLog(log.replaceAll("gear=2", "gear=1"), trucks, "short-arrival").passed).toBe(false);
      expect(summarizeSmokeLog(log.replace("SHORT_ARRIVAL_RESUME_STATE: completed: convoy following", "SHORT_ARRIVAL_RESUME_STATE: accepted: resuming"), trucks, "short-arrival").passed).toBe(false);
    }
    expect(() => parseSmokeArgs(["--trucks", "3", "--variant", "short-arrival"])).toThrow("requires --trucks 1 or 2");
  });

  it("keeps short-arrival errors and pre-terminal member loss fatal while ignoring teardown loss", () => {
    const clean = shortArrivalLog(2);
    const memberLoss = "SCRIPT : [ConvoyFollower] CONVOY_UNIT_REMOVED: Unit2 test failure";
    expect(summarizeSmokeLog(clean.replace("SCRIPT : [ConvoyFollower] SHORT_ARRIVAL_RESULT:", `${memberLoss}\nSCRIPT : [ConvoyFollower] SHORT_ARRIVAL_RESULT:`), 2, "short-arrival").passed).toBe(false);
    expect(summarizeSmokeLog(`${clean}\n${memberLoss}`, 2, "short-arrival").passed).toBe(true);
    expect(summarizeSmokeLog(`${clean}\nSCRIPT (E): runtime failure`, 2, "short-arrival").passed).toBe(false);
    const editReload = `${clean}\nINIT : Workbench Reload Game\nSCRIPT : [ConvoyFollower] SHORT_ARRIVAL_INIT: expected=2`;
    expect(summarizeSmokeLog(editReload, 2, "short-arrival").passed).toBe(true);
    expect(summarizeSmokeLog(clean, 1, "short-arrival").passed).toBe(false);
    expect(summarizeSmokeLog(clean.replace("SCRIPT : SCR_BaseGameMode::", "SCRIPT : [ConvoyFollower] SHORT_ARRIVAL_INIT: expected=2 duplicate\nSCRIPT : SCR_BaseGameMode::"), 2, "short-arrival").passed).toBe(false);
  });
});

function shortArrivalLog(trucks: 1 | 2): string {
  return [
    `WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Arland_Road81_ShortArrival_${trucks}Truck${trucks === 1 ? "" : "s"}.ent'`,
    `SCRIPT : [ConvoyFollower] SHORT_ARRIVAL_INIT: expected=${trucks} owner pilot`,
    "SCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME",
    ...Array.from({ length: trucks }, (_, i) => `SCRIPT : [ConvoyFollower] SHORT_ARRIVAL_ORDER: unit=${i + 1} accepted=true`),
    "SCRIPT : [ConvoyFollower] SHORT_ARRIVAL_DRIVETRAIN: stage=2 engine=true rpm=1600 gear=2 clutch=1 throttle=0.28 brake=0",
    "SCRIPT : [ConvoyFollower] SHORT_ARRIVAL_LEAD_STOP: physical_path=53 physical_displacement=52 goal_gap=8 signed_arc_progress=52",
    "SCRIPT : [ConvoyFollower] SHORT_ARRIVAL_SETTLED: stable_seconds=8 lead_path=54",
    "SCRIPT : [ConvoyFollower] SHORT_ARRIVAL_HOLD_ORDER: accepted=true state=executing: approaching",
    "SCRIPT : [ConvoyFollower] SHORT_ARRIVAL_HOLD_COMPLETED: all trucks stationary and seated",
    "SCRIPT : [ConvoyFollower] SHORT_ARRIVAL_RESUME_ORDER: accepted=true state=accepted: resuming",
    "SCRIPT : [ConvoyFollower] SHORT_ARRIVAL_RESUME_STATE: completed: convoy following",
    "SCRIPT : [ConvoyFollower] SHORT_ARRIVAL_DRIVETRAIN: stage=5 engine=true rpm=1600 gear=2 clutch=1 throttle=0.28 brake=0",
    "SCRIPT : [ConvoyFollower] SHORT_ARRIVAL_SECOND_COMPLETE: all moved",
    "SCRIPT : [ConvoyFollower] SHORT_ARRIVAL_FINAL: stage=6 speed=0 road_gap=1 path=114 second_path=60 second_displacement=58 bay_gap=61 postframe_ticks=1200 signed_arc_progress=110 second_arc_progress=58 second_elevation_gain=2",
    ...Array.from({ length: trucks }, (_, i) => `SCRIPT : [ConvoyFollower] SHORT_ARRIVAL_UNIT: unit=${i + 1} path=90 second_path=30 gap=13 road_gap=2 speed=0 seated=true inverted=false second_displacement=25`),
    "SCRIPT : [ConvoyFollower] SHORT_ARRIVAL_RESULT: PASS authoritative Hold/Resume and both physical legs",
  ].join("\n");
}
