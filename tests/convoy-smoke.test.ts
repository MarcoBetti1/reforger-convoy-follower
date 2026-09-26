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
      "SCRIPT : [ConvoyFollower] AUTO_TWO_AHEAD_RESULT: PASS two trucks parked and convoy resumed",
    ];
    const good = summarizeSmokeLog([...base, ...sequence].join("\n"), 3, "forward-twoahead");
    expect(good.passed).toBe(true);
    expect(good.twoAheadParkingPassed).toBe(true);
    expect(good.twoAheadHoldObserved).toBe(true);
    expect(good.twoAheadHoldPassed).toBe(true);
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
});
