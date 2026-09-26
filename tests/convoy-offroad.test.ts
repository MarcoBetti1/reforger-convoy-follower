import { describe, expect, it } from "vitest";

import { runOffroadReport, summarizeOffroadLog } from "../src/convoy-offroad-report.js";

function physicalLog(): string {
  const lines = [
    "WORLD : Entities load '$ConvoyFollower:Worlds/Tests/ConvoyFollower_Everon_Offroad_Survey_1Truck.ent'",
    "SCRIPT : [ConvoyFollower] OFFROAD_INIT: expected=1",
    "SCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME",
    "SCRIPT : [ConvoyFollower] OFFROAD_ROUTE_SELECTED: length=100 geometry_only=true physical_drive_pending=true",
    "SCRIPT : [ConvoyFollower] OFFROAD_ORDER: accepted=true",
    "SCRIPT : [ConvoyFollower] OFFROAD_DRIVE_STARTED: production one-truck chain",
  ];
  for (let second = 1; second <= 8; second += 1) {
    for (const vehicle of [0, 1]) {
      lines.push(`SCRIPT : [ConvoyFollower] OFFROAD_POSITION: vehicle=${vehicle} seconds=${second} step=6 offroad_path=${second * 6} road_dist=25 road_width=8 offroad=true`);
    }
  }
  for (const truck of ["CF_SmokeLead", "CF_SmokeFollower1"]) {
    for (const sample of [1, 2]) {
      for (const wheel of [0, 1, 2, 3, 4, 5]) {
        lines.push(`SCRIPT : [ConvoyFollower] TEST_WHEEL_SURFACE: sample=${sample} truck=${truck} wheel=${wheel} material={A5388C69AEDCBF4A}Common/Materials/Game/grass_lush.gamemat speed_kmh=15`);
      }
    }
  }
  lines.push("SCRIPT : [ConvoyFollower] OFFROAD_FINAL: lead_path=92 follower_path=80 lead_offroad_path=92 follower_offroad_path=80 lead_offroad_moving_s=13 follower_offroad_moving_s=12 lead_displacement=90 follower_displacement=77 goal_gap=6 link_gap=19 max_link_gap=41 settled_s=10 seated_chain=true max_nonprogress_s=4");
  lines.push("SCRIPT : [ConvoyFollower] OFFROAD_RESULT: PASS physical offroad one-truck chain and settled goal");
  return lines.join("\n");
}

describe("strict offroad physical report", () => {
  it("accepts corroborated physical path, consecutive offroad movement, and settled chain", () => {
    const report = summarizeOffroadLog(physicalLog());
    expect(report.passed).toBe(true);
    expect(report.independentlyObservedMovingSeconds).toEqual([8, 8]);
  });

  it("accepts Arland only when that fixture is explicitly selected", () => {
    const log = physicalLog().replaceAll("ConvoyFollower_Everon_Offroad_Survey_1Truck", "ConvoyFollower_Arland_ClearField_Offroad_1Truck");
    expect(summarizeOffroadLog(log, "arland").passed).toBe(true);
    expect(summarizeOffroadLog(log).passed).toBe(false);
    expect(summarizeOffroadLog(physicalLog(), "arland").passed).toBe(false);
  });

  it("does not reuse an older matching world for a new fixture's PASS", () => {
    const arland = physicalLog().replaceAll("ConvoyFollower_Everon_Offroad_Survey_1Truck", "ConvoyFollower_Arland_ClearField_Offroad_1Truck");
    const combined = physicalLog() + "\nWorkbench Reload Game\n" + arland;
    expect(summarizeOffroadLog(combined).passed).toBe(false);
    expect(summarizeOffroadLog(combined, "arland").passed).toBe(true);
  });

  it("rejects a mismatched explicit probe world label", () => {
    const log = physicalLog().replace("OFFROAD_INIT: expected=1", "OFFROAD_INIT: expected=1 world=ConvoyFollower_Arland_ClearField_Offroad_1Truck");
    expect(summarizeOffroadLog(log).passed).toBe(false);
  });

  it("rejects missing or unsupported fixture arguments before reading a log", () => {
    expect(() => runOffroadReport(["--log", "unused", "--fixture"])).toThrow("--fixture everon|arland");
    expect(() => runOffroadReport(["--log", "unused", "--fixture", "unknown"])).toThrow("--fixture everon|arland");
  });

  it("never accepts a surveyed route or GAME alone", () => {
    const log = physicalLog().split("SCRIPT : [ConvoyFollower] OFFROAD_ORDER")[0];
    const report = summarizeOffroadLog(log);
    expect(report.routeLength).toBe(100);
    expect(report.enteredGame).toBe(true);
    expect(report.passed).toBe(false);
  });

  it("rejects road driving mislabeled as offroad even when final metrics claim PASS", () => {
    const log = physicalLog().replaceAll("road_dist=25 road_width=8", "road_dist=11 road_width=8");
    const report = summarizeOffroadLog(log);
    expect(report.result).toMatch(/^PASS/);
    expect(report.passed).toBe(false);
    expect(report.independentlyObservedMovingSeconds).toEqual([0, 0]);
  });

  it("rejects an unmapped concrete taxiway as unpaved, but can report the off-network comparison", () => {
    const log = physicalLog().replaceAll("grass_lush.gamemat", "concrete.gamemat");
    expect(summarizeOffroadLog(log).passed).toBe(false);
    expect(summarizeOffroadLog(log, "everon", "off-network").passed).toBe(true);
  });

  it("does not infer natural ground from parked contacts or missing follower samples", () => {
    expect(summarizeOffroadLog(physicalLog().replaceAll("speed_kmh=15", "speed_kmh=0")).passed).toBe(false);
    const log = physicalLog().split("\n").filter(line => !line.includes("truck=CF_SmokeFollower1")).join("\n");
    expect(summarizeOffroadLog(log).passed).toBe(false);
  });

  it("rejects catching up only after the lead has pulled far away", () => {
    expect(summarizeOffroadLog(physicalLog().replace("max_link_gap=41", "max_link_gap=91")).passed).toBe(false);
  });

  it.each(["LOST", "STUCK_TERMINAL", "CONVOY_UNIT_REMOVED", "REBOARD_STARTED"])("rejects %s even after a later physical PASS", (event) => {
    const log = physicalLog().replace("OFFROAD_FINAL:", `${event}: Unit 1 failure\nSCRIPT : [ConvoyFollower] OFFROAD_FINAL:`);
    const report = summarizeOffroadLog(log);
    expect(report.failureEvents).toHaveLength(1);
    expect(report.passed).toBe(false);
  });

  it("requires consecutive moving samples and will not count parked time offroad", () => {
    const log = physicalLog().replaceAll("seconds=4 step=6", "seconds=4 step=0");
    const report = summarizeOffroadLog(log);
    expect(report.independentlyObservedMovingSeconds).toEqual([4, 4]);
    expect(report.passed).toBe(false);
  });

  it("does not reuse a prior PASS for a newer incomplete live run", () => {
    const log = physicalLog() + "\nWorkbench Reload Game\nSCRIPT : [ConvoyFollower] OFFROAD_INIT: expected=1\nSCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME";
    const report = summarizeOffroadLog(log);
    expect(report.result).toBeUndefined();
    expect(report.passed).toBe(false);
  });

  it("ignores the trailing editor re-init after the completed F5 run", () => {
    const log = physicalLog() + "\nWorkbench Reload Game\nSCRIPT : [ConvoyFollower] OFFROAD_INIT: expected=1";
    expect(summarizeOffroadLog(log).passed).toBe(true);
  });

  it("rejects insufficient displacement, unstable arrival, or script errors", () => {
    expect(summarizeOffroadLog(physicalLog().replace("follower_displacement=77", "follower_displacement=39")).passed).toBe(false);
    expect(summarizeOffroadLog(physicalLog().replace("settled_s=10", "settled_s=9")).passed).toBe(false);
    expect(summarizeOffroadLog(physicalLog() + "\nSCRIPT (E): offroad runtime error").passed).toBe(false);
  });

  it("separates physical and surface acceptance without weakening the strict result", () => {
    const terrain = summarizeOffroadLog(physicalLog().replaceAll("grass_lush.gamemat", "concrete.gamemat"));
    expect(terrain.scenario.passed).toBe(true);
    expect(terrain.surface.passed).toBe(false);
    expect(terrain.runtime.clean).toBe(true);
    expect(terrain.passed).toBe(false);

    const spacing = summarizeOffroadLog(physicalLog().replace("max_link_gap=41", "max_link_gap=91"));
    expect(spacing.scenario.passed).toBe(false);
    expect(spacing.surface.passed).toBe(true);
    expect(spacing.runtime.clean).toBe(true);
  });

  it("includes load and replication errors before the probe initializes", () => {
    const log = physicalLog().replace("SCRIPT : [ConvoyFollower] OFFROAD_INIT:",
      "WORLD (E): early resource failure\nRPL (E): early replication failure\nSCRIPT : [ConvoyFollower] OFFROAD_INIT:");
    const report = summarizeOffroadLog(log);
    expect(report.scenario.passed).toBe(true);
    expect(report.runtime.clean).toBe(false);
    expect(report.runtime.errors.map(error => error.phase)).toEqual(["load", "load"]);
    expect(report.errorLines).toHaveLength(2);
    expect(report.passed).toBe(false);
  });

  it("also retains errors from this launch before the world-load line", () => {
    const report = summarizeOffroadLog("SCRIPT (E): addon startup failure\n" + physicalLog());
    expect(report.runtime.errors[0]).toMatchObject({ line: 1, phase: "load" });
    expect(report.passed).toBe(false);
  });

  it("annotates only the exact contextual Helipad baseline and never waives it", () => {
    const resource = "Prefabs/Compositions/Misc/SubCompositions/Utility/Helipad_Lights_US_01.et";
    const diagnostic = `WORLD : Entity prefab load @"{472BE7BF1240C1CE}${resource}"\nWORLD (E): Unknown keyword/data 'm_bShowDebugShape' at offset 2307(0x903)`;
    const log = physicalLog().replace("SCRIPT : [ConvoyFollower] OFFROAD_ROUTE_SELECTED:",
      diagnostic + "\nSCRIPT : [ConvoyFollower] OFFROAD_ROUTE_SELECTED:");
    const report = summarizeOffroadLog(log);
    expect(report.scenario.passed).toBe(true);
    expect(report.surface.passed).toBe(true);
    expect(report.runtime.errors[0]).toMatchObject({
      resource, phase: "gameplay",
      reproducedBaseline: {
        id: "vanilla-arland-helipad-show-debug-shape",
        reference: ".cache/client/runs/vanilla-gm-arland-helipad-baseline/logs/console.log",
      },
    });
    expect(report.fullRun).toEqual({ passed: false, clean: false, errorCount: 1, reproducedBaselineCount: 1, failureEventCount: 0 });
    expect(report.passed).toBe(false);
    expect(report.interpretation).toContain("Physical scenario and requested surface gates passed");

    for (const changed of [
      log.replace(resource, "Prefabs/Other.et"),
      log.replace("offset 2307(0x903)", "offset 2308(0x904)"),
      log.replace(`WORLD : Entity prefab load @"{472BE7BF1240C1CE}${resource}"\n`, ""),
    ]) {
      const unmatched = summarizeOffroadLog(changed);
      expect(unmatched.fullRun.reproducedBaselineCount).toBe(0);
      expect(unmatched.passed).toBe(false);
    }
  });

  it("bounds physical evidence at the terminal while retaining later shutdown failures", () => {
    const snapshot = physicalLog();
    const log = snapshot + "\nSCRIPT : [ConvoyFollower] WORLD_CLEANUP: detached 1 sessions" +
      "\nSCRIPT : [ConvoyFollower] CONVOY_UNIT_REMOVED: teardown" +
      "\nENGINE : Game destroyed.\nRESOURCES (E): texture leak\nRPL (E): shutdown replication error";
    const report = summarizeOffroadLog(log);
    expect(report.scenario.passed).toBe(true);
    expect(report.failureEvents).toHaveLength(1);
    expect(report.lifecycle.failureEvents).toEqual(report.failureEvents);
    expect(report.runtime.clean).toBe(true);
    expect(report.lifecycle.observed).toBe(true);
    expect(report.lifecycle.clean).toBe(false);
    expect(report.lifecycle.errors.map(error => error.phase)).toEqual(["shutdown", "shutdown"]);
    expect(report.fullRun.errorCount).toBe(2);
    expect(report.passed).toBe(false);
    expect(report.scope.terminalLine).toBe(snapshot.split("\n").length);
    expect(report.scope.shutdownLine).toBe(snapshot.split("\n").length + 1);
  });

  it("does not claim an unobserved lifecycle is clean or hide post-result errors", () => {
    const report = summarizeOffroadLog(physicalLog());
    expect(report.passed).toBe(true);
    expect(report.lifecycle).toEqual({ observed: false, clean: null, errors: [], failureEvents: [] });
    expect(report.interpretation).toContain("unobserved shutdown remains untested");
    const later = summarizeOffroadLog(physicalLog() + "\nSCRIPT (E): after-result callback failure");
    expect(later.scenario.passed).toBe(true);
    expect(later.runtime.clean).toBe(true);
    expect(later.lifecycle).toMatchObject({ observed: false, clean: false });
    expect(later.lifecycle.errors[0].phase).toBe("post-result");
    expect(later.passed).toBe(false);
  });

  it("does not waive a later convoy failure event even without engine errors", () => {
    const report = summarizeOffroadLog(physicalLog() +
      "\nSCRIPT : [ConvoyFollower] WORLD_CLEANUP: detached 1 sessions" +
      "\nSCRIPT : [ConvoyFollower] CONVOY_UNIT_REMOVED: teardown");
    expect(report.scenario.passed).toBe(true);
    expect(report.runtime.clean).toBe(true);
    expect(report.lifecycle.clean).toBe(false);
    expect(report.fullRun.failureEventCount).toBe(1);
    expect(report.passed).toBe(false);
  });

  it("keeps a previous run's errors outside the selected reloaded run", () => {
    const previous = physicalLog() + "\nRPL (E): old failure\nENGINE : Game destroyed.";
    const report = summarizeOffroadLog(previous + "\nWorkbench Reload Game\n" + physicalLog());
    expect(report.passed).toBe(true);
    expect(report.errorLines).toEqual([]);
    expect(report.scope.startLine).toBeGreaterThan(previous.split("\n").length);
  });

  it("cannot reuse an earlier fixture PASS for a later unrelated world's GAME", () => {
    const log = physicalLog() + "\nWorkbench Reload Game\nWORLD : Entities load 'worlds/GameMaster/GM_Arland.ent'" +
      "\nSCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME";
    const report = summarizeOffroadLog(log);
    expect(report.observedWorld).toBe(false);
    expect(report.result).toBeUndefined();
    expect(report.passed).toBe(false);
  });
});
