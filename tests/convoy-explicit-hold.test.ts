import { mkdtempSync, rmSync, writeFileSync } from "node:fs";
import os from "node:os";
import path from "node:path";

import { describe, expect, it, vi } from "vitest";

import { EXPLICIT_HOLD_WORLD, runExplicitHoldReport, summarizeExplicitHoldLog } from "../src/convoy-explicit-hold-report.js";

const event = (name: string, detail: string) => `SCRIPT : [ConvoyFollower] ${name}: ${detail}`;

function goodLog(): string {
  const lines = [
    `WORLD : Entities load '$ConvoyFollower:${EXPLICIT_HOLD_WORLD}'`,
    event("EXPLICIT_HOLD_INIT", "expected=1 native_lead=true production_follower=true test_seat_transfers=true human_input_claim=false"),
    "SCRIPT : SCR_BaseGameMode::OnGameStateChanged = GAME",
    event("OFFROAD_ROUTE_SELECTED", "length=80 start=<1620, 38, 2940> goal=<1700, 38, 2940> axis=<1, 0, 0> candidate=120"),
    event("EXPLICIT_HOLD_RECRUIT", "accepted=true"),
    event("EXPLICIT_HOLD_PHASE", "seconds=1 phase=1"),
    event("CONVOY_STARTED", "Unit One is ready"),
    event("EXPLICIT_HOLD_PHASE", "seconds=10 phase=2"),
  ];
  function motion(phase: number, seconds: number, vehicle: number, x: number, progress: number, step: number, powered: boolean) {
    lines.push(event("EXPLICIT_HOLD_MOTION", `phase=${phase} seconds=${seconds} vehicle=${vehicle} origin=<${x}, 38, 2940> progress_m=${progress} signed_step_m=${step} speed_kmh=${powered ? 18 : 0} throttle=${powered ? 0.4 : 0} brake=${powered ? 0 : 1} engine=true gear=2 powered=${powered} lateral_m=0`));
  }
  function transfer(phase: number, seconds: number, toOwner: boolean) {
    lines.push(event("EXPLICIT_HOLD_PHASE", `seconds=${seconds} phase=${phase}`));
    lines.push(event("EXPLICIT_HOLD_SEAT_SETUP", `seconds=${seconds} to_owner_pilot=${toOwner} vehicle=CF_SmokeLead temporary_test_setup=true human_input_claim=false`));
    if (toOwner) {
      lines.push(event("EXPLICIT_HOLD_SEAT_EXIT", `seconds=${seconds + 1} owner=false accepted=true temporary_test_setup=true`));
      lines.push(event("EXPLICIT_HOLD_SEAT_EXIT", `seconds=${seconds + 2} owner=true accepted=true temporary_test_setup=true`));
      lines.push(event("EXPLICIT_HOLD_SEAT_BOARD", `seconds=${seconds + 3} owner=true pilot=true accepted=true temporary_test_setup=true`));
    } else {
      lines.push(event("EXPLICIT_HOLD_SEAT_EXIT", `seconds=${seconds + 1} owner=true accepted=true temporary_test_setup=true`));
      lines.push(event("EXPLICIT_HOLD_SEAT_BOARD", `seconds=${seconds + 2} owner=true pilot=false accepted=true temporary_test_setup=true`));
      lines.push(event("EXPLICIT_HOLD_SEAT_BOARD", `seconds=${seconds + 3} owner=false pilot=true accepted=true temporary_test_setup=true`));
    }
  }
  for (let seconds = 11; seconds <= 18; seconds++) {
    motion(2, seconds, 0, 1620 + (seconds - 10) * 5, (seconds - 10) * 5, 5, true);
    motion(2, seconds, 1, 1600 + (seconds - 10) * 3, (seconds - 10) * 3, 3, true);
  }
  lines.push(event("EXPLICIT_HOLD_PHASE", "seconds=18 phase=3"));
  transfer(4, 22, true);
  lines.push(event("PANEL_HOLD_ACCEPTED", "owner stopped; active trucks will hold once individually slow"));
  lines.push(event("PANEL_HOLD", "Unit 1 remains seated"));
  lines.push(event("EXPLICIT_HOLD_COMMAND", "accepted=true owner_pilot=true state=completed: all trucks holding in vehicles"));
  lines.push(event("EXPLICIT_HOLD_PHASE", "seconds=26 phase=5"));
  lines.push(event("EXPLICIT_HOLD_CAPTURE", "seconds=29 origin=<1624, 38, 2940>"));
  transfer(6, 29, false);
  lines.push(event("EXPLICIT_HOLD_LEAD_MOVE", "seconds=33 goal=<1740, 38, 2940> native=true"));
  lines.push(event("EXPLICIT_HOLD_PHASE", "seconds=33 phase=7"));
  for (let seconds = 34; seconds <= 63; seconds++) {
    const progress = Math.min((seconds - 33) * 5, 25);
    motion(7, seconds, 0, 1660 + progress, progress, seconds <= 38 ? 5 : 0, seconds <= 38);
    motion(7, seconds, 1, 1624, 0, 0, false);
    lines.push(event("EXPLICIT_HOLD_OBSERVE", `seconds=${seconds} held_s=${seconds - 33} max_drift_m=0 max_speed_kmh=0 lead_progress_m=${progress} hold_requested=true`));
  }
  lines.push(event("EXPLICIT_HOLD_VERIFIED", "held_s=30 lead_progress_m=25 max_drift_m=0 same_driver_truck_roster=true"));
  transfer(8, 63, true);
  lines.push(event("PANEL_RESUME", "Unit 1 follows its assigned predecessor"));
  lines.push(event("PANEL_RESUME_ACCEPTED", "owner resumed the assigned daisy chain"));
  lines.push(event("EXPLICIT_RESUME_COMMAND", "accepted=true owner_pilot=true state=completed: convoy following"));
  transfer(9, 67, false);
  lines.push(event("EXPLICIT_HOLD_LEAD_MOVE", "seconds=71 goal=<1780, 38, 2940> native=true"));
  lines.push(event("EXPLICIT_HOLD_PHASE", "seconds=71 phase=10"));
  for (let seconds = 72; seconds <= 77; seconds++) {
    motion(10, seconds, 0, 1685 + (seconds - 71) * 4, (seconds - 71) * 4, 4, true);
    motion(10, seconds, 1, 1624 + (seconds - 71) * 3, (seconds - 71) * 3, 3, true);
  }
  lines.push(event("EXPLICIT_RESUME_VERIFIED", "lead_progress_m=24 follower_progress_m=18 same_driver_truck_roster=true"));
  lines.push(event("EXPLICIT_HOLD_PHASE", "seconds=77 phase=11"));
  lines.push(event("EXPLICIT_HOLD_FINAL", "seconds=80 phase=11 hold_accepted=true resume_accepted=true max_hold_drift_m=0 max_hold_speed_kmh=0 lead_progress_m=24 follower_progress_m=18 lead_powered_samples=6 follower_powered_samples=6"));
  lines.push(event("EXPLICIT_HOLD_ROSTER", "1|1|CF_SmokeFollower1 #1|arriving|owner"));
  lines.push(event("EXPLICIT_HOLD_RESULT", "PASS real_server_hold_30s_lead_advance_then_resume_powered_progress"));
  lines.push(event("WORLD_CLEANUP", "begin"));
  lines.push("ENGINE : Game destroyed");
  return lines.join("\n");
}

const rewrite = (log: string, match: (line: string) => boolean, edit: (line: string) => string) => log.split("\n").map(line => match(line) ? edit(line) : line).join("\n");

describe("strict explicit Hold/Resume report", () => {
  it("accepts complete real command, sustained hold, identity and powered motion evidence", () => {
    const report = summarizeExplicitHoldLog(goodLog());
    expect(report.failures).toEqual([]);
    expect(report.passed).toBe(true);
    expect(report.metrics).toMatchObject({ heldSeconds: 30, maxHoldDriftMetres: 0, holdLeadProgressMetres: 25, holdLeadPoweredSamples: 5, resumeProgressMetres: [24, 18], resumePoweredSamples: [6, 6] });
    expect(report.postTerminal.physicalObservationVerified).toBe(false);
    expect(report.fixtureSeatTransfers).toMatchObject({ count: 4, productionTeleportProof: false, humanInputProof: false });
  });

  it("rejects another world and duplicate/competing probes", () => {
    expect(summarizeExplicitHoldLog(goodLog().replace(EXPLICIT_HOLD_WORLD, "Worlds/Tests/Other.ent")).scenario.passed).toBe(false);
    for (const extra of [event("EXPLICIT_HOLD_INIT", "expected=1"), event("OFFROAD_INIT", "expected=1")]) {
      expect(summarizeExplicitHoldLog(goodLog().replace("SCRIPT : SCR_BaseGameMode", `${extra}\nSCRIPT : SCR_BaseGameMode`)).passed).toBe(false);
    }
  });

  it("accepts Enforce's observed numeric inline owner comparison, but no other numeric bool values", () => {
    const numeric = rewrite(goodLog(), line => /EXPLICIT_HOLD_SEAT_(?:EXIT|BOARD):/.test(line), line => line.replace("owner=true", "owner=1").replace("owner=false", "owner=0"));
    expect(summarizeExplicitHoldLog(numeric).passed).toBe(true);
    expect(summarizeExplicitHoldLog(numeric.replace("owner=1", "owner=2")).passed).toBe(false);
    expect(summarizeExplicitHoldLog(numeric.replace("hold_requested=true", "hold_requested=1")).passed).toBe(false);
  });

  it("does not reuse an older passing launch or trailing editor reload", () => {
    const incomplete = goodLog().split(event("EXPLICIT_HOLD_RECRUIT", "accepted=true"))[0];
    expect(summarizeExplicitHoldLog(goodLog() + "\nWorkbench Reload Game\n" + incomplete).scenario.passed).toBe(false);
    expect(summarizeExplicitHoldLog(goodLog() + "\nWorkbench Reload Game\nSCRIPT (E): editor-only").passed).toBe(true);
  });

  it.each(["PANEL_HOLD_ACCEPTED", "PANEL_HOLD", "PANEL_RESUME", "PANEL_RESUME_ACCEPTED", "CONVOY_STARTED"])("requires production %s, not just a claimed command acceptance", name => {
    const log = goodLog().split("\n").filter(line => !line.includes(`] ${name}:`)).join("\n");
    expect(summarizeExplicitHoldLog(log).scenario.passed).toBe(false);
  });

  it.each(["EXPLICIT_HOLD_COMMAND", "EXPLICIT_RESUME_COMMAND"])("rejects rejected or ineligible %s", name => {
    for (const edit of ["accepted=false", "owner_pilot=false"]) {
      const field = edit.split("=")[0];
      const log = rewrite(goodLog(), line => line.includes(`] ${name}:`), line => line.replace(`${field}=true`, edit));
      expect(summarizeExplicitHoldLog(log).scenario.passed).toBe(false);
    }
  });

  it("rejects shortened or missing Hold observations despite a 30-second completion claim", () => {
    for (const second of [34, 47, 63]) {
      const log = goodLog().split("\n").filter(line => !line.includes(`EXPLICIT_HOLD_OBSERVE: seconds=${second} `)).join("\n");
      expect(summarizeExplicitHoldLog(log).scenario.passed).toBe(false);
    }
    expect(summarizeExplicitHoldLog(goodLog().replaceAll("held_s=30", "held_s=29")).scenario.passed).toBe(false);
  });

  it("corroborates physical follower drift even when summary maxima claim zero", () => {
    const log = rewrite(goodLog(), line => line.includes("phase=7 seconds=45 vehicle=1"), line => line.replace("origin=<1624,", "origin=<1627,"));
    const report = summarizeExplicitHoldLog(log);
    expect(report.scenario.passed).toBe(false);
    expect(report.metrics.maxHoldDriftMetres).toBe(3);
  });

  it.each(["max_drift_m=2.1", "max_speed_kmh=2.1", "hold_requested=false", "held_s=Infinity"])("retains a bad Hold observation: %s", replacement => {
    const field = replacement.split("=")[0];
    const log = rewrite(goodLog(), line => line.includes("EXPLICIT_HOLD_OBSERVE: seconds=45 "), line => line.replace(new RegExp(`${field}=\\S+`), replacement));
    expect(summarizeExplicitHoldLog(log).scenario.passed).toBe(false);
  });

  it("requires actual lead movement and power during Hold", () => {
    const coast = rewrite(goodLog(), line => line.includes("EXPLICIT_HOLD_MOTION: phase=7") && line.includes("vehicle=0"), line => line.replace("engine=true", "engine=false").replace("powered=true", "powered=false"));
    expect(summarizeExplicitHoldLog(coast).scenario.passed).toBe(false);
    const forged = rewrite(goodLog(), line => line.includes("EXPLICIT_HOLD_MOTION: phase=7") && line.includes("vehicle=0"), line => line.replace(/origin=<[^>]+>/, "origin=<1660, 38, 2940>"));
    expect(summarizeExplicitHoldLog(forged).scenario.passed).toBe(false);
  });

  it("requires resumed follower power and position progress independent of aggregate metrics", () => {
    const coast = rewrite(goodLog(), line => line.includes("EXPLICIT_HOLD_MOTION: phase=10") && line.includes("vehicle=1"), line => line.replace("throttle=0.4", "throttle=0").replace("powered=true", "powered=false"));
    expect(summarizeExplicitHoldLog(coast).scenario.passed).toBe(false);
    const forged = rewrite(goodLog(), line => line.includes("EXPLICIT_HOLD_MOTION: phase=10") && line.includes("vehicle=1"), line => line.replace(/origin=<[^>]+>/, "origin=<1624, 38, 2940>"));
    expect(summarizeExplicitHoldLog(forged).scenario.passed).toBe(false);
  });

  it("rejects duplicate motion samples and mismatched steps", () => {
    const sample = goodLog().split("\n").find(line => line.includes("phase=10 seconds=74 vehicle=1"))!;
    expect(summarizeExplicitHoldLog(goodLog().replace(sample, `${sample}\n${sample}`)).scenario.passed).toBe(false);
    expect(summarizeExplicitHoldLog(goodLog().replace(sample, sample.replace("signed_step_m=3", "signed_step_m=24"))).scenario.passed).toBe(false);
  });

  it("retains changed assignment/identity/seats as failures", () => {
    for (const log of [
      goodLog().replaceAll("same_driver_truck_roster=true", "same_driver_truck_roster=false"),
      goodLog().replace("1|1|CF_SmokeFollower1 #1|arriving|owner", "1|1|DifferentTruck #1|arriving|owner"),
      goodLog().replace("1|1|CF_SmokeFollower1 #1|arriving|owner", "1|1|CF_SmokeFollower1 #1|arriving|owner;2|2|Other #2|following|Unit 1"),
      goodLog().replace("vehicle=CF_SmokeLead temporary_test_setup=true", "vehicle=CF_SmokeFollower1 temporary_test_setup=true"),
      goodLog().replace("owner=true pilot=true accepted=true", "owner=true pilot=false accepted=true"),
    ]) expect(summarizeExplicitHoldLog(log).scenario.passed).toBe(false);
  });

  it("does not rescue a first terminal FAIL with later correct telemetry and PASS", () => {
    const fail = event("EXPLICIT_HOLD_RESULT", "FAIL missing_seat");
    const log = goodLog().replace(event("EXPLICIT_HOLD_PHASE", "seconds=33 phase=7"), `${fail}\n${event("EXPLICIT_HOLD_PHASE", "seconds=33 phase=7")}`);
    const report = summarizeExplicitHoldLog(log);
    expect(report.result).toBe("FAIL missing_seat");
    expect(report.scenario.passed).toBe(false);
    expect(report.passed).toBe(false);
  });

  it("separates later failure from an earlier physical scenario pass", () => {
    const log = goodLog().replace(event("WORLD_CLEANUP", "begin"), `${event("REBOARD_STARTED", "Unit 1 unexpectedly left driver seat")}\n${event("WORLD_CLEANUP", "begin")}`);
    const report = summarizeExplicitHoldLog(log);
    expect(report.scenario.passed).toBe(true);
    expect(report.passed).toBe(false);
    expect(report.postTerminal.failureEvents).toHaveLength(1);
    expect(report.postTerminal.physicalObservationVerified).toBe(false);
  });

  it("keeps runtime, post-terminal and shutdown errors visible without changing physical evidence", () => {
    const log = "SCRIPT (E): load problem\n" + goodLog()
      .replace(event("WORLD_CLEANUP", "begin"), `NETWORK (E): late runtime problem\n${event("WORLD_CLEANUP", "begin")}\nSCRIPT (E): shutdown problem`);
    const report = summarizeExplicitHoldLog(log);
    expect(report.scenario.passed).toBe(true);
    expect(report.passed).toBe(false);
    expect(report.runtime.errors.map(error => error.phase)).toEqual(["load", "post-result"]);
    expect(report.postTerminal.errors).toHaveLength(1);
    expect(report.shutdown.errors).toHaveLength(1);
    expect(report.fullRun.errorCount).toBe(3);
  });

  it("reuses exact vanilla error provenance without waiving the error", () => {
    const error = 'WORLD : Entity prefab load @"{472BE7BF1240C1CE}Prefabs/Compositions/Misc/SubCompositions/Utility/Helipad_Lights_US_01.et"\nWORLD (E): Unknown keyword/data \'m_bShowDebugShape\' at offset 2307(0x903)\n';
    const report = summarizeExplicitHoldLog(error + goodLog());
    expect(report.scenario.passed).toBe(true);
    expect(report.runtime.reproducedBaselineCount).toBe(1);
    expect(report.passed).toBe(false);
  });

  it("does not claim a complete lifecycle from the terminal snapshot", () => {
    const report = summarizeExplicitHoldLog(goodLog().split(event("WORLD_CLEANUP", "begin"))[0]);
    expect(report.scenario.passed).toBe(true);
    expect(report.shutdown.clean).toBeNull();
    expect(report.passed).toBe(false);
  });

  it("implements strict CLI exit status and rejects duplicate/unknown arguments", () => {
    for (const args of [[], ["--log"], ["--log", "unused", "--wat"], ["--log", "a", "--log", "b"], ["--log", "a", "--require-pass", "--require-pass"]]) {
      expect(() => runExplicitHoldReport(args)).toThrow("Usage:");
    }
    const root = mkdtempSync(path.join(os.tmpdir(), "explicit-hold-report-"));
    const log = path.join(root, "console.log");
    const output = vi.spyOn(process.stdout, "write").mockReturnValue(true);
    try {
      writeFileSync(log, goodLog());
      expect(runExplicitHoldReport(["--log", log, "--require-pass"])).toBe(0);
      writeFileSync(log, goodLog() + "\nSCRIPT (E): shutdown error");
      expect(runExplicitHoldReport(["--log", log])).toBe(0);
      expect(runExplicitHoldReport(["--log", log, "--require-pass"])).toBe(1);
    } finally {
      output.mockRestore();
      rmSync(root, { recursive: true, force: true });
    }
  });
});
