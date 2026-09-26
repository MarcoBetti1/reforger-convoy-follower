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

function holdRestartLog(): string {
  const events = ["SCRIPT : [ConvoyFollower] OFFROAD_HOLD_BEGIN: seconds=20 origin=<100, 0, 0> cycle=1"];
  for (let second = 20; second <= 50; second += 1) {
    events.push(`SCRIPT : [ConvoyFollower] OFFROAD_LEAD_HOLD: seconds=${second} origin=<100.1, 0, 0> cycle=1 speed_kmh=0.1 settled_pose=true drift_m=0.1 max_drift_m=0.1 brake=1 throttle=0 gear=2 engine=true`);
  }
  events.push("SCRIPT : [ConvoyFollower] OFFROAD_HOLD_COMPLETE: seconds=50 held_s=30 max_drift_m=0.1 cycle=1");
  events.push("SCRIPT : [ConvoyFollower] OFFROAD_RESTART_ORDER: seconds=50 accepted=true start=<100, 0, 0> follower_start=<80, 0, 0> goal=<140, 0, 0> axis=<1, 0, 0>");
  for (let second = 51; second <= 56; second += 1) {
    for (const vehicle of [0, 1]) {
      const progress = (second - 50) * (vehicle === 0 ? 4 : 3);
      events.push(`SCRIPT : [ConvoyFollower] OFFROAD_RESTART_POSITION: vehicle=${vehicle} seconds=${second} origin=<${(vehicle === 0 ? 100 : 80) + progress}, 0, 0> progress_m=${progress} speed_kmh=12 throttle=0.4 brake=0 gear=2 engine=true seated_chain=true pilot_seated=true`);
    }
  }
  events.push("SCRIPT : [ConvoyFollower] OFFROAD_LEAD_RESTART_COMPLETE: seconds=56 lead_progress_m=24");
  events.push("SCRIPT : [ConvoyFollower] OFFROAD_RESTART_COMPLETE: seconds=56 lead_progress_m=24 follower_progress_m=18");
  events.push("SCRIPT : [ConvoyFollower] OFFROAD_HOLD_BEGIN: seconds=60 origin=<124, 0, 0> cycle=2");
  const later: string[] = [];
  for (let second = 60; second <= 120; second += 1) {
    (second <= 90 ? events : later).push(`SCRIPT : [ConvoyFollower] OFFROAD_LEAD_HOLD: seconds=${second} origin=<124.1, 0, 0> cycle=2 speed_kmh=0.1 settled_pose=true drift_m=0.1 max_drift_m=0.1 brake=1 throttle=0 gear=2 engine=true`);
  }
  events.push("SCRIPT : [ConvoyFollower] OFFROAD_HOLD_COMPLETE: seconds=90 held_s=30 max_drift_m=0.1 cycle=2");
  later.push("SCRIPT : [ConvoyFollower] OFFROAD_OBSERVATION_COMPLETE: seconds=120 observed_s=30 failed=false");
  return physicalLog().replace("OFFROAD_INIT: expected=1", "OFFROAD_INIT: expected=1 hold_restart=true")
    .replace("SCRIPT : [ConvoyFollower] OFFROAD_FINAL:", events.join("\n") + "\nSCRIPT : [ConvoyFollower] OFFROAD_FINAL:") + "\n" + later.join("\n");
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

  it("uses a larger bound vehicle gap observation without overwriting the probe's aggregate", () => {
    const log = physicalLog().replace("SCRIPT : [ConvoyFollower] OFFROAD_FINAL:",
      "01:02:10.022 SCRIPT : [ConvoyFollower] FOLLOW_LINK_STATUS: Unit 1 target=CF_SmokeLead state=3 gap=65.0769\nSCRIPT : [ConvoyFollower] OFFROAD_FINAL:");
    const report = summarizeOffroadLog(log);
    expect(report.finalMetrics.max_link_gap).toBe(41);
    expect(report.peakLinkGap).toMatchObject({ reportedMetres: 41, observedMetres: 65.0769, gateMetres: 65.0769,
      evidence: { source: "FOLLOW_LINK_STATUS", timestamp: "01:02:10.022" } });
    expect(report.passed).toBe(false);
  });

  it("independently measures synchronized fixture positions and ignores unrelated link telemetry", () => {
    const log = physicalLog().replace("vehicle=0 seconds=8 step=6", "vehicle=0 seconds=8 origin=<100, 0, 0> step=6")
      .replace("vehicle=1 seconds=8 step=6", "vehicle=1 seconds=8 origin=<35, 0, 0> step=6");
    expect(summarizeOffroadLog(log).peakLinkGap).toMatchObject({ gateMetres: 65, evidence: { source: "OFFROAD_POSITION", seconds: 8 } });
    expect(summarizeOffroadLog(log).passed).toBe(false);
    for (const detail of ["Unit 2 target=CF_SmokeLead state=3", "Unit 1 target=OtherLead state=3", "Unit 1 target=CF_SmokeLead state=8"]) {
      const unrelated = physicalLog().replace("OFFROAD_FINAL:", `FOLLOW_LINK_STATUS: ${detail} gap=200\nSCRIPT : [ConvoyFollower] OFFROAD_FINAL:`);
      expect(summarizeOffroadLog(unrelated).peakLinkGap.gateMetres).toBe(41);
      expect(summarizeOffroadLog(unrelated).passed).toBe(true);
    }
    expect(summarizeOffroadLog(physicalLog() + "\nSCRIPT : [ConvoyFollower] FOLLOW_LINK_STATUS: Unit 1 target=CF_SmokeLead state=3 gap=200").passed).toBe(true);
  });

  it("retains GUI load errors in the strict runtime gate", () => {
    const report = summarizeOffroadLog("GUI (E): Unknown class 'SCR_WidgetExportRuleRoot' at offset 282(0x11a)\n" + physicalLog());
    expect(report.runtime.errors).toHaveLength(1);
    expect(report.runtime.errors[0].phase).toBe("load");
    expect(report.runtime.clean).toBe(false);
    expect(report.passed).toBe(false);
  });

  it("retains NETWORK replication errors and their immediate resource context", () => {
    const report = summarizeOffroadLog([
      'RESOURCES : GetResourceObject @"{0123456789ABCDEF}Prefabs/Characters/Character_CF_Driver.et"',
      "NETWORK (E): RplNodeError: Attempting to put into hierarchy items where one is already registered into replication and the other is not!",
      physicalLog(),
    ].join("\n"));
    expect(report.runtime.errors).toHaveLength(1);
    expect(report.runtime.errors[0]).toMatchObject({ phase: "load", resource: "Prefabs/Characters/Character_CF_Driver.et" });
    expect(report.runtime.errors[0].reproducedBaseline).toBeUndefined();
    expect(report.runtime.clean).toBe(false);
    expect(report.passed).toBe(false);
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

  it("annotates the other exact vanilla signatures, including a four-error resource block, without waiving them", () => {
    const diagnostics = [
      "RESOURCES : GetResourceObject @\"{A}UI/layouts/Menus/MainMenu/IntroSplashScreen.layout\"",
      "GUI (E): Unknown class 'SCR_WidgetExportRuleRoot' at offset 282(0x11a)",
      "WORLD : Entity prefab load @\"{A}Prefabs/Vehicles/Wheeled/M151A2/M151A2.et\"",
      "WORLD (E): Unknown keyword/data 'SlidingTrackMaterial' at offset 19441(0x4bf1)",
      "WORLD : Entity prefab load @\"{A}Prefabs/Vehicles/Wheeled/BRDM2/BRDM2_base.et\"",
      "WORLD (E): Unknown keyword/data 'Parent' at offset 35955(0x8c73)",
      "WORLD (E): Unknown keyword/data 'Parent' at offset 36915(0x9033)",
      "WORLD : Entity prefab load @\"{A}Prefabs/Vehicles/Wheeled/BTR70/BTR70_Base.et\"",
      "WORLD (E): Unknown keyword/data 'Parent' at offset 42646(0xa696)",
      "WORLD (E): Unknown keyword/data 'Parent' at offset 43645(0xaa7d)",
      "WORLD (E): Unknown keyword/data 'Parent' at offset 44617(0xae49)",
      "WORLD (E): Unknown keyword/data 'Parent' at offset 45590(0xb216)",
    ].join("\n");
    const report = summarizeOffroadLog(diagnostics + "\n" + physicalLog());
    expect(report.runtime.errors).toHaveLength(8);
    expect(report.runtime.reproducedBaselineCount).toBe(8);
    expect(report.runtime.errors.map(error => error.reproducedBaseline?.line)).toEqual([98, 150, 155, 156, 161, 162, 163, 164]);
    expect(report.fullRun.errorCount).toBe(8);
    expect(report.passed).toBe(false);
    for (const changed of [
      diagnostics.replaceAll("BTR70_Base.et", "Other.et"),
      diagnostics.replace("offset 45590(0xb216)", "offset 45591(0xb217)"),
      diagnostics.replace("WORLD (E): Unknown keyword/data 'Parent' at offset 45590", "unrelated event\nWORLD (E): Unknown keyword/data 'Parent' at offset 45590"),
      diagnostics.replace("GUI (E):", "SCRIPT (E):"),
    ]) {
      const unmatched = summarizeOffroadLog(changed + "\n" + physicalLog());
      expect(unmatched.runtime.reproducedBaselineCount).toBeLessThan(8);
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

describe("native lead sustained hold and physical restart", () => {
  it("does not infer sustained hold from a legacy arrival PASS", () => {
    const historical = summarizeOffroadLog(physicalLog());
    expect(historical.holdRestart).toMatchObject({ required: false, passed: null });
    const required = summarizeOffroadLog(physicalLog(), "everon", "unpaved", true);
    expect(required.holdRestart).toMatchObject({ required: true, passed: false, fixture: { passed: false }, follower: { passed: null } });
    expect(required.passed).toBe(false);
  });

  it("corroborates 30 seconds of hold and forward powered restart from actual positions", () => {
    const report = summarizeOffroadLog(holdRestartLog());
    expect(report.holdRestart).toMatchObject({
      required: true, passed: true, observedHoldSeconds: 30,
      restartProgressMetres: [24, 18], maxRestartLinkGapMetres: 26, poweredLeadRestartSamples: 6,
      fixture: { passed: true, failures: [] }, follower: { passed: true, failures: [] },
      observation: { passed: true, failures: [] }, postResultObservedSeconds: 30,
    });
    expect(report.holdRestart.maxLeadDriftMetres).toBeCloseTo(0.1);
    expect(report.passed).toBe(true);
  });

  it("rejects a missing second or abbreviated physical hold despite a completion claim", () => {
    for (const log of [
      holdRestartLog().split("\n").filter(line => !line.includes("OFFROAD_LEAD_HOLD: seconds=35 ")).join("\n"),
      holdRestartLog().replace("OFFROAD_HOLD_BEGIN: seconds=20", "OFFROAD_HOLD_BEGIN: seconds=21"),
    ]) {
      const report = summarizeOffroadLog(log);
      expect(report.holdRestart.fixture.passed).toBe(false);
      expect(report.holdRestart.follower.passed).toBeNull();
      expect(report.passed).toBe(false);
    }
  });

  it("rejects an excursion beyond 2 m even when final pose and logged drift claim a hold", () => {
    const log = holdRestartLog().replace("seconds=35 origin=<100.1, 0, 0>", "seconds=35 origin=<103, 0, 0>");
    const report = summarizeOffroadLog(log);
    expect(report.holdRestart.maxLeadDriftMetres).toBe(3);
    expect(report.holdRestart.fixture.passed).toBe(false);
    expect(report.holdRestart.follower.passed).toBeNull();
    expect(report.passed).toBe(false);
  });

  it("rejects speed evidence that contradicts a stationary hold", () => {
    const report = summarizeOffroadLog(holdRestartLog().replace("speed_kmh=0.1", "speed_kmh=-4"));
    expect(report.holdRestart.fixture.passed).toBe(false);
    expect(report.passed).toBe(false);
  });

  it("cannot substitute reported progress for physical restart or accept backward coasting", () => {
    for (const direction of [0, -1]) {
      const log = holdRestartLog().replace(/(OFFROAD_RESTART_POSITION: vehicle=0 seconds=(\d+) origin=<)\d+,/g,
        (_, prefix: string, second: string) => `${prefix}${100 + direction * (Number(second) - 50) * 4},`);
      const report = summarizeOffroadLog(log);
      expect(report.holdRestart.fixture.passed).toBe(false);
      expect(report.holdRestart.poweredLeadRestartSamples).toBe(0);
      expect(report.passed).toBe(false);
    }
  });

  it("requires observed powered lead controls rather than displacement alone", () => {
    for (const log of [
      holdRestartLog().replaceAll("throttle=0.4", "throttle=0"),
      holdRestartLog().replaceAll("gear=2", "gear=1"),
      holdRestartLog().replaceAll("engine=true", "engine=false"),
    ]) {
      const report = summarizeOffroadLog(log);
      expect(report.holdRestart.restartProgressMetres).toEqual([24, 18]);
      expect(report.holdRestart.fixture.passed).toBe(false);
      expect(report.passed).toBe(false);
    }
  });

  it("attributes a follower restart failure only after native lead fixture acceptance", () => {
    const log = holdRestartLog().replace(/(OFFROAD_RESTART_POSITION: vehicle=1 seconds=\d+ origin=<)\d+,/g, "$180,");
    const report = summarizeOffroadLog(log);
    expect(report.holdRestart.fixture.passed).toBe(true);
    expect(report.holdRestart.follower.passed).toBe(false);
    expect(report.holdRestart.restartProgressMetres).toEqual([24, 0]);
    expect(report.passed).toBe(false);
  });

  it("rejects restart gaps above 60 m even when the aggregate claims a lower peak", () => {
    const log = holdRestartLog().replace("follower_start=<80, 0, 0>", "follower_start=<0, 0, 0>")
      .replace(/(OFFROAD_RESTART_POSITION: vehicle=1 seconds=(\d+) origin=<)\d+,/g,
        (_, prefix: string, second: string) => `${prefix}${(Number(second) - 50) * 3},`);
    const report = summarizeOffroadLog(log);
    expect(report.holdRestart.fixture.passed).toBe(true);
    expect(report.holdRestart.maxRestartLinkGapMetres).toBe(106);
    expect(report.holdRestart.follower.passed).toBe(false);
    expect(report.passed).toBe(false);
  });

  it("retains the original 60 m approach peak-gap gate", () => {
    const report = summarizeOffroadLog(holdRestartLog().replace("max_link_gap=41", "max_link_gap=70.08"));
    expect(report.holdRestart.passed).toBe(true);
    expect(report.scenario.passed).toBe(false);
    expect(report.passed).toBe(false);
  });

  it("reads later fixture failures and cannot accept a terminal before restart", () => {
    const later = summarizeOffroadLog(holdRestartLog() + "\nSCRIPT : [ConvoyFollower] OFFROAD_FIXTURE_FAILURE: seconds=75 reason=post_result_lead_drift");
    expect(later.holdRestart.fixture.passed).toBe(false);
    expect(later.holdRestart.follower.passed).toBeNull();
    expect(later.fullRun.clean).toBe(false);
    expect(later.lifecycle.failureEvents).toHaveLength(1);
    expect(later.passed).toBe(false);
    const early = summarizeOffroadLog(holdRestartLog().replace("SCRIPT : [ConvoyFollower] OFFROAD_HOLD_BEGIN:",
      "SCRIPT : [ConvoyFollower] OFFROAD_RESULT: PASS provisional\nSCRIPT : [ConvoyFollower] OFFROAD_HOLD_BEGIN:"));
    expect(early.holdRestart.observation.failures.some(failure => failure.includes("provisional"))).toBe(true);
    expect(early.passed).toBe(false);
  });

  it("keeps a proved lead fixture independent of missing follower completion and later observation", () => {
    const log = holdRestartLog().split("\n").filter(line => !line.includes("OFFROAD_RESTART_COMPLETE:") &&
      !line.includes("cycle=2") && !line.includes("OFFROAD_OBSERVATION_COMPLETE:")).join("\n")
      .replace(/(OFFROAD_RESTART_POSITION: vehicle=0[^\n]*)seated_chain=true/g, "$1seated_chain=false");
    const report = summarizeOffroadLog(log);
    expect(report.holdRestart.fixture.passed).toBe(true);
    expect(report.holdRestart.follower.passed).toBe(false);
    expect(report.holdRestart.observation.passed).toBe(false);
    expect(report.passed).toBe(false);
  });

  it("requires physical observation after the provisional result", () => {
    const log = holdRestartLog().split("\n").filter(line => !/OFFROAD_LEAD_HOLD: seconds=(?:9[1-9]|1\d\d)\b|OFFROAD_OBSERVATION_COMPLETE:/.test(line)).join("\n");
    const report = summarizeOffroadLog(log);
    expect(report.holdRestart.fixture.passed).toBe(true);
    expect(report.holdRestart.follower.passed).toBe(true);
    expect(report.holdRestart.observation.passed).toBe(false);
    expect(report.passed).toBe(false);
  });

  it("rejects later lead drift from position samples even without a failure marker", () => {
    const log = holdRestartLog().replace("seconds=110 origin=<124.1, 0, 0>", "seconds=110 origin=<128, 0, 0>");
    const report = summarizeOffroadLog(log);
    expect(report.holdRestart.maxLeadDriftMetres).toBe(4);
    expect(report.holdRestart.fixture.passed).toBe(false);
    expect(report.holdRestart.follower.passed).toBeNull();
    expect(report.passed).toBe(false);
  });

  it("retains a failed post-result observation even if its detailed failure marker is missing", () => {
    const report = summarizeOffroadLog(holdRestartLog().replace("observed_s=30 failed=false", "observed_s=30 failed=true"));
    expect(report.holdRestart.fixture.passed).toBe(false);
    expect(report.holdRestart.observation.passed).toBe(false);
    expect(report.passed).toBe(false);
  });

  it("retains runtime and full lifecycle gates after physical hold/restart success", () => {
    const report = summarizeOffroadLog("RPL (E): startup error\n" + holdRestartLog() +
      "\nSCRIPT : [ConvoyFollower] WORLD_CLEANUP: detached 1 sessions\nRESOURCES (E): shutdown error");
    expect(report.holdRestart.passed).toBe(true);
    expect(report.scenario.passed).toBe(true);
    expect(report.runtime.clean).toBe(false);
    expect(report.lifecycle.clean).toBe(false);
    expect(report.fullRun.errorCount).toBe(2);
    expect(report.passed).toBe(false);
  });
});
