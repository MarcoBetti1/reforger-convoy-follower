import { readFileSync } from "node:fs";
import path from "node:path";
import { pathToFileURL } from "node:url";

const WORLD = "Worlds/Tests/ConvoyFollower_Everon_Offroad_Survey_1Truck.ent";
const PREFIX = /\[ConvoyFollower\]\s+([A-Z0-9_]+):\s*(.*)$/;

export interface OffroadReport {
  expectedWorld: string;
  observedWorld: boolean;
  enteredGame: boolean;
  routeLength?: number;
  result?: string;
  finalMetrics: Record<string, number | boolean>;
  independentlyObservedMovingSeconds: [number, number];
  eventCounts: Record<string, number>;
  failureEvents: string[];
  errorLines: string[];
  failures: string[];
  passed: boolean;
  interpretation: string;
}

function numberField(text: string, name: string): number | undefined {
  const match = new RegExp(`(?:^|\\s)${name}=(-?\\d+(?:\\.\\d+)?(?:[eE][+-]?\\d+)?)\\b`).exec(text);
  if (!match) return undefined;
  const value = Number(match[1]);
  return Number.isFinite(value) ? value : undefined;
}

function boolField(text: string, name: string): boolean | undefined {
  const match = new RegExp(`(?:^|\\s)${name}=(true|false)\\b`).exec(text);
  return match ? match[1] === "true" : undefined;
}

// A load or GAME transition is not a driving result. Select the most recent
// actual game interval and ignore its trailing Workbench editor re-init.
export function summarizeOffroadLog(logText: string): OffroadReport {
  const lines = logText.split(/\r?\n/);
  let gameIndex = -1;
  for (let index = lines.length - 1; index >= 0; index -= 1) {
    if (/OnGameStateChanged\s*=\s*GAME\b/.test(lines[index])) {
      gameIndex = index;
      break;
    }
  }
  let end = lines.length;
  if (gameIndex >= 0) {
    const relativeReload = lines.slice(gameIndex + 1).findIndex((line) => /Workbench Reload Game/.test(line));
    if (relativeReload >= 0) end = gameIndex + 1 + relativeReload;
  }
  let initIndex = -1;
  for (let index = end - 1; index >= 0; index -= 1) {
    if (/\[ConvoyFollower\]\s+OFFROAD_INIT:/.test(lines[index])) {
      initIndex = index;
      break;
    }
  }
  const run = initIndex >= 0 ? lines.slice(initIndex, end) : [];
  const observedWorld = lines.slice(0, end).some((line) => line.replaceAll("\\", "/").includes(WORLD));
  const enteredGame = gameIndex >= 0 && initIndex >= 0 && gameIndex < end;
  const eventCounts: Record<string, number> = {};
  const failureEvents: string[] = [];
  const errorLines = run.filter((line) => /\b(?:SCRIPT|ENGINE|WORLD|RESOURCES)\s*\(E\)/.test(line));
  const finalMetrics: Record<string, number | boolean> = {};
  const numericNames = [
    "lead_path", "follower_path", "lead_offroad_path", "follower_offroad_path",
    "lead_offroad_moving_s", "follower_offroad_moving_s", "lead_displacement", "follower_displacement",
    "goal_gap", "link_gap", "max_link_gap", "settled_s", "max_nonprogress_s",
  ];
  let result: string | undefined;
  let routeLength: number | undefined;
  let acceptedOrder = false;
  let observedDrive = false;
  const currentStreak = [0, 0];
  const maxStreak: [number, number] = [0, 0];
  const previousSecond = [-1, -1];
  const observedOffroadPath = [0, 0];
  for (const line of run) {
    const event = PREFIX.exec(line);
    if (!event) continue;
    const [, name, detail] = event;
    eventCounts[name] = (eventCounts[name] ?? 0) + 1;
    if (["LOST", "STUCK_TERMINAL", "CONVOY_UNIT_REMOVED", "REBOARD_STARTED", "ORDER_INVERSION"].includes(name)) {
      failureEvents.push(line);
    }
    if (name === "OFFROAD_RESULT") {
      result = detail;
      if (detail.startsWith("FAIL")) failureEvents.push(line);
    }
    if (name === "OFFROAD_ROUTE_SELECTED") routeLength = numberField(detail, "length");
    if (name === "OFFROAD_ORDER" && boolField(detail, "accepted") === true) acceptedOrder = true;
    if (name === "OFFROAD_DRIVE_STARTED") observedDrive = true;
    if (name === "OFFROAD_FINAL") {
      for (const key of numericNames) {
        const value = numberField(detail, key);
        if (value !== undefined) finalMetrics[key] = value;
      }
      const seated = boolField(detail, "seated_chain");
      if (seated !== undefined) finalMetrics.seated_chain = seated;
    }
    if (name === "OFFROAD_POSITION") {
      const vehicle = numberField(detail, "vehicle");
      if (vehicle !== 0 && vehicle !== 1) continue;
      const second = numberField(detail, "seconds");
      const step = numberField(detail, "step");
      const distance = numberField(detail, "road_dist");
      const width = numberField(detail, "road_width");
      const pathValue = numberField(detail, "offroad_path");
      if (pathValue !== undefined) observedOffroadPath[vehicle] = Math.max(observedOffroadPath[vehicle], pathValue);
      const physicallyOffroad = second !== undefined && step !== undefined && step >= 0.25 && step <= 25 &&
        distance !== undefined && width !== undefined && width > 0 && distance > width / 2 + 8 &&
        boolField(detail, "offroad") === true;
      if (physicallyOffroad) {
        currentStreak[vehicle] = second === previousSecond[vehicle] + 1 ? currentStreak[vehicle] + 1 : 1;
        maxStreak[vehicle] = Math.max(maxStreak[vehicle], currentStreak[vehicle]);
      } else currentStreak[vehicle] = 0;
      previousSecond[vehicle] = second ?? -1;
    }
  }
  const failures: string[] = [];
  if (!observedWorld) failures.push("Expected isolated Everon offroad world was not observed.");
  if (!enteredGame) failures.push("No actual GAME interval with the offroad probe was observed.");
  if (routeLength === undefined || routeLength < 60 || routeLength > 100) failures.push("No surveyed 60–100 m route was selected.");
  if (!acceptedOrder || !observedDrive) failures.push("Production convoy order and physical drive start were not both observed.");
  if (!result?.startsWith("PASS")) failures.push("The selected game run has no terminal offroad PASS.");
  for (const key of ["lead_offroad_path", "follower_offroad_path", "lead_displacement", "follower_displacement"]) {
    if (typeof finalMetrics[key] !== "number" || finalMetrics[key] < 40) failures.push(`${key} must be at least 40 m.`);
  }
  for (const key of ["lead_offroad_moving_s", "follower_offroad_moving_s"]) {
    if (typeof finalMetrics[key] !== "number" || finalMetrics[key] < 8) failures.push(`${key} must be at least 8 consecutive moving seconds.`);
  }
  if (maxStreak.some((seconds) => seconds < 8)) failures.push("Position samples do not independently show eight consecutive moving offroad seconds for both trucks.");
  if (observedOffroadPath.some((metres) => metres < 40)) failures.push("Position samples do not corroborate 40 m offroad for both trucks.");
  if (typeof finalMetrics.goal_gap !== "number" || finalMetrics.goal_gap > 10) failures.push("Lead must settle within 10 m of the selected goal.");
  if (typeof finalMetrics.link_gap !== "number" || finalMetrics.link_gap < 7 || finalMetrics.link_gap > 30) failures.push("Final follower gap must remain between 7 and 30 m.");
  if (typeof finalMetrics.max_link_gap !== "number" || finalMetrics.max_link_gap > 100) failures.push("The following link exceeded its 100 m test bound or lacks evidence.");
  if (typeof finalMetrics.max_nonprogress_s !== "number" || finalMetrics.max_nonprogress_s >= 15) failures.push("The follower stopped making progress for too long or lacks evidence.");
  if (typeof finalMetrics.settled_s !== "number" || finalMetrics.settled_s < 10 || finalMetrics.seated_chain !== true) failures.push("A seated assigned chain must settle for ten consecutive seconds.");
  if (failureEvents.length) failures.push("The selected run contains a lost, stuck, removed, reboarded, inverted, or failed convoy event.");
  if (errorLines.length) failures.push("The selected probe run contains engine or script errors.");
  return {
    expectedWorld: WORLD, observedWorld, enteredGame, routeLength, result, finalMetrics,
    independentlyObservedMovingSeconds: maxStreak, eventCounts, failureEvents, errorLines,
    failures, passed: failures.length === 0,
    interpretation: failures.length === 0
      ? "Strict log evidence supports a physical one-truck offroad run. Inspect the gameplay video before making a visual demonstration claim."
      : "Offroad acceptance remains unproved; a geometry candidate or GAME transition alone is insufficient.",
  };
}

export function runOffroadReport(argv: string[]): number {
  const logIndex = argv.indexOf("--log");
  const allowed = new Set(["--log", "--require-pass"]);
  if (logIndex < 0 || !argv[logIndex + 1] || argv[logIndex + 1].startsWith("--") ||
      argv.some((arg, index) => index !== logIndex + 1 && !allowed.has(arg))) {
    throw new Error("Usage: npx tsx src/convoy-offroad-report.ts --log <console.log> [--require-pass]");
  }
  const report = summarizeOffroadLog(readFileSync(path.resolve(argv[logIndex + 1]), "utf8"));
  process.stdout.write(`${JSON.stringify(report, null, 2)}\n`);
  return argv.includes("--require-pass") && !report.passed ? 1 : 0;
}

if (process.argv[1] && import.meta.url === pathToFileURL(path.resolve(process.argv[1])).href) {
  try {
    process.exitCode = runOffroadReport(process.argv.slice(2));
  } catch (error) {
    process.stderr.write(`${error instanceof Error ? error.message : String(error)}\n`);
    process.exitCode = 1;
  }
}
