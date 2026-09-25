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
    expect(report.errorLines).toHaveLength(1);
    expect(summarizeSmokeLog(log, 1).observedWorld).toBe(false);
  });

  it("requires an explicit count and avoids a timed keep-open run", () => {
    expect(() => parseSmokeArgs([])).toThrow("--trucks");
    expect(() => parseSmokeArgs(["--trucks", "4"])).toThrow("--trucks");
    expect(() => parseSmokeArgs(["--trucks", "2", "--keep-open", "--duration-seconds", "30"])).toThrow("cannot be combined");
  });
});
