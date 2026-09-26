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
});
