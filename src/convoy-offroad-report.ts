import { readFileSync } from "node:fs";
import path from "node:path";
import { pathToFileURL } from "node:url";

export type OffroadFixture = "everon" | "arland";
export type SurfaceRequirement = "unpaved" | "off-network";
const WORLDS: Record<OffroadFixture, string> = {
  everon: "Worlds/Tests/ConvoyFollower_Everon_Offroad_Survey_1Truck.ent",
  arland: "Worlds/Tests/ConvoyFollower_Arland_ClearField_Offroad_1Truck.ent",
};
const PREFIX = /\[ConvoyFollower\]\s+([A-Z0-9_]+):\s*(.*)$/;
const ERROR = /\b(?:SCRIPT|ENGINE|WORLD|RESOURCES|RPL)\s*\(E\)/;
const HELIPAD_RESOURCE = "Prefabs/Compositions/Misc/SubCompositions/Utility/Helipad_Lights_US_01.et";
const HELIPAD_BASELINE = ".cache/client/runs/vanilla-gm-arland-helipad-baseline/logs/console.log";

export interface OffroadDiagnostic {
  line: number;
  text: string;
  phase: "load" | "gameplay" | "post-result" | "shutdown";
  resource?: string;
  reproducedBaseline?: { id: string; reference: string; interpretation: string };
}

interface ReportGate {
  passed: boolean;
  failures: string[];
}

export interface OffroadReport {
  surfaceRequirement: SurfaceRequirement;
  movingSurfaceSamples: Record<string, { unpaved: number; pavedOrUnknown: number }>;
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
  scope: {
    startLine: number;
    endLine: number;
    gameLine?: number;
    initLine?: number;
    terminalLine?: number;
    shutdownLine?: number;
  };
  scenario: ReportGate;
  surface: ReportGate & { requirement: SurfaceRequirement };
  runtime: { clean: boolean; errors: OffroadDiagnostic[]; reproducedBaselineCount: number };
  lifecycle: { observed: boolean; clean: boolean | null; errors: OffroadDiagnostic[]; failureEvents: string[] };
  fullRun: { passed: boolean; clean: boolean; errorCount: number; reproducedBaselineCount: number; failureEventCount: number };
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
export function summarizeOffroadLog(logText: string, fixture: OffroadFixture = "everon", surfaceRequirement: SurfaceRequirement = "unpaved"): OffroadReport {
  const expectedWorld = WORLDS[fixture];
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
  // Include this launch's load errors, including those before probe init. A
  // previous world teardown/reload is a boundary, not part of the new result.
  let start = 0;
  for (let index = 0; index < gameIndex; index += 1) {
    if (/Workbench Reload Game|\bGame destroyed\b/.test(lines[index])) start = index + 1;
  }
  let initIndex = -1;
  for (let index = end - 1; index >= start; index -= 1) {
    if (/\[ConvoyFollower\]\s+OFFROAD_INIT:/.test(lines[index])) {
      initIndex = index;
      break;
    }
  }
  let terminalIndex = -1;
  let shutdownIndex = -1;
  for (let index = Math.max(start, initIndex); index < end; index += 1) {
    if (terminalIndex < 0 && /\[ConvoyFollower\]\s+OFFROAD_RESULT:/.test(lines[index])) terminalIndex = index;
    if (shutdownIndex < 0 && /\[ConvoyFollower\]\s+WORLD_CLEANUP:|\bGame destroyed\b/.test(lines[index])) shutdownIndex = index;
  }
  const physicalEnd = Math.min(terminalIndex >= 0 ? terminalIndex + 1 : end, shutdownIndex >= 0 ? shutdownIndex : end);
  const run = initIndex >= 0 ? lines.slice(initIndex, physicalEnd) : [];
  let lastLoadedFixture: string | undefined;
  for (const line of lines.slice(start, initIndex + 1)) {
    const normalized = line.replaceAll("\\", "/");
    const loadedWorld = /Entities load ['"]([^'"]+\.ent)['"]/.exec(normalized)?.[1];
    if (loadedWorld) lastLoadedFixture = loadedWorld;
  }
  const initWorld = initIndex >= 0 ? /\bworld=(\S+)/.exec(lines[initIndex])?.[1] : undefined;
  const expectedLabel = path.posix.basename(expectedWorld, ".ent");
  const observedWorld = (lastLoadedFixture === expectedWorld || lastLoadedFixture?.endsWith(`:${expectedWorld}`) === true) &&
    (initWorld === undefined || initWorld === expectedLabel);
  const enteredGame = gameIndex >= start && initIndex >= start && gameIndex < physicalEnd;
  const eventCounts: Record<string, number> = {};
  const failureEvents: string[] = [];
  const diagnostics: OffroadDiagnostic[] = [];
  let lastResource: { name: string; line: number } | undefined;
  for (let index = start; index < end; index += 1) {
    const line = lines[index];
    const resource = /(?:GetResourceObject|Entity prefab load)\s+@?"(?:\{[^}]+\})?([^"]+)"/.exec(line)?.[1];
    if (resource) lastResource = { name: resource.replaceAll("\\", "/"), line: index };
    if (!ERROR.test(line)) continue;
    const diagnostic: OffroadDiagnostic = {
      line: index + 1,
      text: line,
      phase: shutdownIndex >= 0 && index >= shutdownIndex ? "shutdown"
        : terminalIndex >= 0 && index > terminalIndex ? "post-result"
          : gameIndex < 0 || index < gameIndex ? "load" : "gameplay",
    };
    // Require nearby resource context as well as the exact known signature;
    // the same field name in another resource is not the reproduced baseline.
    if (lastResource && index - lastResource.line <= 3) diagnostic.resource = lastResource.name;
    if (diagnostic.resource === HELIPAD_RESOURCE &&
        /\bWORLD\s*\(E\):\s*Unknown keyword\/data 'm_bShowDebugShape' at offset 2307\(0x903\)\s*$/.test(line)) {
      diagnostic.reproducedBaseline = {
        id: "vanilla-arland-helipad-show-debug-shape",
        reference: HELIPAD_BASELINE,
        interpretation: "Exact resource/signature reproduced without this addon; retained as a strict error.",
      };
    }
    diagnostics.push(diagnostic);
  }
  const errorLines = diagnostics.map((diagnostic) => diagnostic.text);
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
  const surfaceGroups = new Map<string, { truck: string; wheels: Set<number>; unpaved: boolean }>();
  for (const line of run) {
    const event = PREFIX.exec(line);
    if (!event) continue;
    const [, name, detail] = event;
    eventCounts[name] = (eventCounts[name] ?? 0) + 1;
    if (name === "TEST_WHEEL_SURFACE") {
      const truck = /\btruck=(\S+)/.exec(detail)?.[1];
      const sample = numberField(detail, "sample");
      const wheel = numberField(detail, "wheel");
      const speed = numberField(detail, "speed_kmh");
      const material = /\bmaterial=(.*?)\s+speed_kmh=/.exec(detail)?.[1] ?? "";
      if (truck && sample !== undefined && wheel !== undefined && speed !== undefined && Math.abs(speed) >= 1) {
        const key = `${truck}:${sample}`;
        const group = surfaceGroups.get(key) ?? { truck, wheels: new Set<number>(), unpaved: true };
        group.wheels.add(wheel);
        // An unmapped taxiway can be concrete. Require every observed wheel
        // in this moving sample to touch a known natural ground material.
        group.unpaved &&= /\/(?:grass[^/]*|dirt[^/]*|soil[^/]*|gravel[^/]*|sand[^/]*|mud[^/]*)\.gamemat$/i.test(material);
        surfaceGroups.set(key, group);
      }
    }
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
  const movingSurfaceSamples: OffroadReport["movingSurfaceSamples"] = {};
  for (const group of surfaceGroups.values()) {
    const counts = movingSurfaceSamples[group.truck] ??= { unpaved: 0, pavedOrUnknown: 0 };
    if (group.unpaved && group.wheels.size >= 4) counts.unpaved += 1;
    else counts.pavedOrUnknown += 1;
  }
  const surfaceFailures: string[] = [];
  if (surfaceRequirement === "unpaved") {
    for (const truck of ["CF_SmokeLead", "CF_SmokeFollower1"]) {
      if ((movingSurfaceSamples[truck]?.unpaved ?? 0) < 2) {
        surfaceFailures.push(`${truck} needs at least two moving samples with four or more wheels on known unpaved ground; road-network distance is not surface evidence.`);
      }
    }
  }
  const failures: string[] = [];
  if (!observedWorld) failures.push(`Expected isolated ${fixture} offroad world was not observed for this run.`);
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
  if (typeof finalMetrics.max_link_gap !== "number" || finalMetrics.max_link_gap > 60) failures.push("The following link exceeded its 60 m short-course bound or lacks evidence; catching up only after the lead stops is insufficient.");
  if (typeof finalMetrics.max_nonprogress_s !== "number" || finalMetrics.max_nonprogress_s >= 15) failures.push("The follower stopped making progress for too long or lacks evidence.");
  if (typeof finalMetrics.settled_s !== "number" || finalMetrics.settled_s < 10 || finalMetrics.seated_chain !== true) failures.push("A seated assigned chain must settle for ten consecutive seconds.");
  if (failureEvents.length) failures.push("The selected run contains a lost, stuck, removed, reboarded, inverted, or failed convoy event.");
  const scenario: ReportGate = { passed: failures.length === 0, failures: [...failures] };
  const surface = { requirement: surfaceRequirement, passed: surfaceFailures.length === 0, failures: surfaceFailures };
  failures.unshift(...surfaceFailures);
  const runtimeErrors = diagnostics.filter(({ phase }) => phase === "load" || phase === "gameplay");
  const lifecycleErrors = diagnostics.filter(({ phase }) => phase === "post-result" || phase === "shutdown");
  // Keep the old full-run event gate strict. A teardown removal is not an
  // in-game chain failure, but it is still visible and prevents a clean full
  // lifecycle claim; terminal-bounded scenario acceptance remains separate.
  const laterFailureEvents = lines.slice(physicalEnd, end).filter((line) => {
    const event = PREFIX.exec(line);
    return event !== null && (["LOST", "STUCK_TERMINAL", "CONVOY_UNIT_REMOVED", "REBOARD_STARTED", "ORDER_INVERSION"].includes(event[1]) ||
      (event[1] === "OFFROAD_RESULT" && event[2].startsWith("FAIL")));
  });
  failureEvents.push(...laterFailureEvents);
  if (laterFailureEvents.length) failures.push("Post-result or shutdown convoy failure events remain in the strict full-run gate; see lifecycle findings separately from the physical scenario.");
  if (errorLines.length) failures.push("The selected full run contains engine, script, resource, world, or replication errors; reproduced baseline errors remain strict failures.");
  const reproducedBaselineCount = diagnostics.filter(({ reproducedBaseline }) => reproducedBaseline !== undefined).length;
  return {
    surfaceRequirement, movingSurfaceSamples,
    expectedWorld, observedWorld, enteredGame, routeLength, result, finalMetrics,
    independentlyObservedMovingSeconds: maxStreak, eventCounts, failureEvents, errorLines,
    failures, passed: failures.length === 0,
    scope: {
      startLine: start + 1, endLine: end,
      gameLine: gameIndex >= 0 ? gameIndex + 1 : undefined,
      initLine: initIndex >= 0 ? initIndex + 1 : undefined,
      terminalLine: terminalIndex >= 0 ? terminalIndex + 1 : undefined,
      shutdownLine: shutdownIndex >= 0 ? shutdownIndex + 1 : undefined,
    },
    scenario, surface,
    runtime: {
      clean: runtimeErrors.length === 0, errors: runtimeErrors,
      reproducedBaselineCount: runtimeErrors.filter(({ reproducedBaseline }) => reproducedBaseline !== undefined).length,
    },
    lifecycle: {
      observed: shutdownIndex >= 0,
      clean: lifecycleErrors.length || laterFailureEvents.length ? false : shutdownIndex >= 0 ? true : null,
      errors: lifecycleErrors,
      failureEvents: laterFailureEvents,
    },
    fullRun: {
      passed: failures.length === 0, clean: diagnostics.length === 0 && failureEvents.length === 0,
      errorCount: diagnostics.length, reproducedBaselineCount, failureEventCount: failureEvents.length,
    },
    interpretation: failures.length === 0
      ? `Strict log evidence supports a physical one-truck ${surfaceRequirement} run within the supplied log. Inspect the gameplay video; unobserved shutdown remains untested.`
      : scenario.passed && surface.passed
        ? "Physical scenario and requested surface gates passed, but the supplied full run has strict runtime/lifecycle errors. No error-free or overall PASS claim is supported."
        : "Physical scenario or requested surface acceptance failed; see the separate gates and runtime/lifecycle diagnostics. A probe PASS alone is insufficient.",
  };
}

export function runOffroadReport(argv: string[]): number {
  const logIndex = argv.indexOf("--log");
  const fixtureIndex = argv.indexOf("--fixture");
  const surfaceIndex = argv.indexOf("--surface");
  const allowed = new Set(["--log", "--fixture", "--surface", "--require-pass"]);
  if (logIndex < 0 || !argv[logIndex + 1] || argv[logIndex + 1].startsWith("--") ||
      (fixtureIndex >= 0 && !["everon", "arland"].includes(argv[fixtureIndex + 1] ?? "")) ||
      (surfaceIndex >= 0 && !["unpaved", "off-network"].includes(argv[surfaceIndex + 1] ?? "")) ||
      argv.some((arg, index) => index !== logIndex + 1 && (fixtureIndex < 0 || index !== fixtureIndex + 1) && (surfaceIndex < 0 || index !== surfaceIndex + 1) && !allowed.has(arg))) {
    throw new Error("Usage: npx tsx src/convoy-offroad-report.ts --log <console.log> [--fixture everon|arland] [--surface unpaved|off-network] [--require-pass]");
  }
  const fixture = fixtureIndex >= 0 ? argv[fixtureIndex + 1] as OffroadFixture : "everon";
  const surface = surfaceIndex >= 0 ? argv[surfaceIndex + 1] as SurfaceRequirement : "unpaved";
  const report = summarizeOffroadLog(readFileSync(path.resolve(argv[logIndex + 1]), "utf8"), fixture, surface);
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
