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
const ERROR = /\b(?:SCRIPT|ENGINE|WORLD|RESOURCES|RPL|NETWORK|GUI)\s*\(E\)/;
const HELIPAD_RESOURCE = "Prefabs/Compositions/Misc/SubCompositions/Utility/Helipad_Lights_US_01.et";
const VANILLA_BASELINE = ".cache/client/runs/vanilla-gm-arland-helipad-baseline/logs/console.log";
// All nine were rechecked against the same historical vanilla full log on
// September 26. Its loaded-addon lines 29–31 contain only core/ArmaReforger.
// Exact resource, family, field/class and offset matches annotate provenance;
// they never waive an error or change the strict acceptance threshold.
const VANILLA_SIGNATURES = [
  { id: "vanilla-arland-intro-widget-export", resource: "UI/layouts/Menus/MainMenu/IntroSplashScreen.layout", family: "GUI", message: "Unknown class 'SCR_WidgetExportRuleRoot' at offset 282(0x11a)", line: 98 },
  { id: "vanilla-arland-m151-sliding-track", resource: "Prefabs/Vehicles/Wheeled/M151A2/M151A2.et", family: "WORLD", message: "Unknown keyword/data 'SlidingTrackMaterial' at offset 19441(0x4bf1)", line: 150 },
  ...[[35955, "8c73", 155], [36915, "9033", 156]].map(([offset, hex, line]) => ({ id: `vanilla-arland-brdm-wheel-parent-${offset}`, resource: "Prefabs/Vehicles/Wheeled/BRDM2/BRDM2_base.et", family: "WORLD", message: `Unknown keyword/data 'Parent' at offset ${offset}(0x${hex})`, line: Number(line) })),
  ...[[42646, "a696", 161], [43645, "aa7d", 162], [44617, "ae49", 163], [45590, "b216", 164]].map(([offset, hex, line]) => ({ id: `vanilla-arland-btr-wheel-parent-${offset}`, resource: "Prefabs/Vehicles/Wheeled/BTR70/BTR70_Base.et", family: "WORLD", message: `Unknown keyword/data 'Parent' at offset ${offset}(0x${hex})`, line: Number(line) })),
  { id: "vanilla-arland-helipad-show-debug-shape", resource: HELIPAD_RESOURCE, family: "WORLD", message: "Unknown keyword/data 'm_bShowDebugShape' at offset 2307(0x903)", line: 237 },
];

export interface OffroadDiagnostic {
  line: number;
  text: string;
  phase: "load" | "gameplay" | "post-result" | "shutdown";
  resource?: string;
  reproducedBaseline?: { id: string; reference: string; line: number; interpretation: string };
}

interface ReportGate {
  passed: boolean;
  failures: string[];
}

export interface HoldRestartReport {
  required: boolean;
  passed: boolean | null;
  fixture: { passed: boolean | null; failures: string[] };
  follower: { passed: boolean | null; failures: string[] };
  observation: { passed: boolean | null; failures: string[] };
  holdCycles: Array<{ cycle: number; observedSeconds: number; maxDriftMetres: number }>;
  observedHoldSeconds: number;
  postResultObservedSeconds: number;
  maxLeadDriftMetres: number;
  restartProgressMetres: [number, number];
  maxRestartLinkGapMetres: number;
  poweredLeadRestartSamples: number;
  interpretation: string;
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
  peakLinkGap: {
    reportedMetres?: number;
    observedMetres?: number;
    gateMetres?: number;
    evidence?: { source: "FOLLOW_LINK_STATUS" | "OFFROAD_POSITION"; line: number; seconds?: number; timestamp?: string };
  };
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
  holdRestart: HoldRestartReport;
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

type Position = [number, number, number];

function vectorField(text: string, name: string): Position | undefined {
  const match = new RegExp(`(?:^|\\s)${name}=<([^>]+)>`).exec(text);
  const values = match?.[1].split(",").map(value => Number(value.trim()));
  return values?.length === 3 && values.every(Number.isFinite) ? values as Position : undefined;
}

function distanceXZ(left: Position, right: Position): number {
  return Math.hypot(left[0] - right[0], left[2] - right[2]);
}

// Hold/restart is an explicit fixture capability, never inferred from a legacy
// ten-second arrival PASS. Read through shutdown so later physical violations
// cannot be hidden behind an early terminal marker.
function summarizeHoldRestart(lines: string[], required: boolean): HoldRestartReport {
  const events = lines.flatMap((line, index) => {
    const match = PREFIX.exec(line);
    return match ? [{ name: match[1], detail: match[2], line: index }] : [];
  });
  required ||= events.some(event => event.name === "OFFROAD_INIT" && boolField(event.detail, "hold_restart") === true);
  const fixtureFailures: string[] = [];
  const followerFailures: string[] = [];
  const observationFailures: string[] = [];
  const report: HoldRestartReport = {
    required, passed: null,
    fixture: { passed: null, failures: fixtureFailures },
    follower: { passed: null, failures: followerFailures },
    observation: { passed: null, failures: observationFailures }, holdCycles: [],
    observedHoldSeconds: 0, postResultObservedSeconds: 0, maxLeadDriftMetres: 0,
    restartProgressMetres: [0, 0], maxRestartLinkGapMetres: 0, poweredLeadRestartSamples: 0,
    interpretation: "Hold and restart were not requested or declared; arrival evidence does not establish them.",
  };
  if (!required) return report;
  const restarts = events.filter(event => event.name === "OFFROAD_RESTART_ORDER");
  const leadCompletions = events.filter(event => event.name === "OFFROAD_LEAD_RESTART_COMPLETE");
  const restartCompletions = events.filter(event => event.name === "OFFROAD_RESTART_COMPLETE");
  const restart = restarts[0];
  const leadComplete = leadCompletions[0];
  const restartComplete = restartCompletions[0];
  const terminal = events.find(event => event.name === "OFFROAD_RESULT");
  const observationComplete = events.find(event => event.name === "OFFROAD_OBSERVATION_COMPLETE");
  if (observationComplete && boolField(observationComplete.detail, "failed") === true) {
    fixtureFailures.push("Native lead post-result observation reported a fixture failure.");
  }
  const restartSecond = restart && numberField(restart.detail, "seconds");
  if (restarts.length !== 1 || leadCompletions.length !== 1 || !restart || !leadComplete || restart.line >= leadComplete.line) {
    fixtureFailures.push("An ordered native restart request and independent lead restart completion are required.");
  }
  if (restartCompletions.length !== 1 || !restart || !restartComplete || restart.line >= restartComplete.line) {
    followerFailures.push("Assigned follower physical restart completion was not observed after the restart request.");
  }
  if (!terminal || !leadComplete || !restartComplete || terminal.line <= Math.max(leadComplete.line, restartComplete.line)) {
    observationFailures.push("The terminal result must follow physical lead and follower restart completion; an earlier arrival marker is provisional.");
  }
  for (const event of events.filter(event => event.name === "OFFROAD_FIXTURE_FAILURE")) {
    fixtureFailures.push(`Native lead fixture failure: ${event.detail}`);
  }
  for (const cycle of [1, 2]) {
    const holds = events.filter(event => event.name === "OFFROAD_HOLD_BEGIN" && numberField(event.detail, "cycle") === cycle);
    const completions = events.filter(event => event.name === "OFFROAD_HOLD_COMPLETE" && numberField(event.detail, "cycle") === cycle);
    const begin = holds[0];
    const complete = completions[0];
    const holdOrigin = begin && vectorField(begin.detail, "origin");
    const beginSecond = begin && numberField(begin.detail, "seconds");
    const completeSecond = complete && numberField(complete.detail, "seconds");
    // Missing later observation can follow a follower timeout. It is a separate
    // unmet gate, not evidence that the already-proved lead restart failed.
    const cycleFailures = cycle === 1 ? fixtureFailures : observationFailures;
    if (holds.length !== 1 || completions.length !== 1 || !begin || !complete || !restart || begin.line >= complete.line ||
        (cycle === 1 ? complete.line >= restart.line : begin.line <= restart.line)) {
      cycleFailures.push(`Hold cycle ${cycle} needs one ordered settled-pose and completion marker.`);
    }
    if (!holdOrigin || beginSecond === undefined || completeSecond === undefined || completeSecond < beginSecond + 30 ||
        (cycle === 1 && (restartSecond === undefined || restartSecond < completeSecond))) {
      cycleFailures.push(`Hold cycle ${cycle} must retain a recorded settled pose for at least 30 seconds.`);
    }
    const holdSamples = events.filter(event => event.name === "OFFROAD_LEAD_HOLD" && numberField(event.detail, "cycle") === cycle &&
      begin && event.line > begin.line && (cycle === 2 || !restart || event.line < restart.line));
    let previousSecond = beginSecond;
    let holdCoverageValid = holdOrigin !== undefined && beginSecond !== undefined;
    let observedSeconds = 0;
    let maxDriftMetres = 0;
    let lastPreTerminalSecond: number | undefined;
    let lastPostTerminalSecond: number | undefined;
    for (const sample of holdSamples) {
      const second = numberField(sample.detail, "seconds");
      const origin = vectorField(sample.detail, "origin");
      const speed = numberField(sample.detail, "speed_kmh");
      const validTime = second !== undefined && previousSecond !== undefined && second >= previousSecond && second <= previousSecond + 1;
      if (!validTime || !origin || speed === undefined || Math.abs(speed) > 2 || boolField(sample.detail, "settled_pose") !== true) holdCoverageValid = false;
      if (speed !== undefined && Math.abs(speed) > 2 && !fixtureFailures.includes(`Lead moved during settled hold cycle ${cycle}.`)) {
        fixtureFailures.push(`Lead moved during settled hold cycle ${cycle}.`);
      }
      if (origin && holdOrigin) maxDriftMetres = Math.max(maxDriftMetres, distanceXZ(origin, holdOrigin));
      maxDriftMetres = Math.max(maxDriftMetres, numberField(sample.detail, "drift_m") ?? 0, numberField(sample.detail, "max_drift_m") ?? 0);
      if (second !== undefined && beginSecond !== undefined) observedSeconds = Math.max(observedSeconds, second - beginSecond);
      if (terminal && sample.line < terminal.line) lastPreTerminalSecond = second;
      if (terminal && sample.line > terminal.line) lastPostTerminalSecond = second;
      previousSecond = second;
    }
    report.holdCycles.push({ cycle, observedSeconds, maxDriftMetres });
    report.maxLeadDriftMetres = Math.max(report.maxLeadDriftMetres, maxDriftMetres);
    if (cycle === 1) report.observedHoldSeconds = observedSeconds;
    if (!holdCoverageValid || observedSeconds < 30 || previousSecond === undefined || completeSecond === undefined || previousSecond < completeSecond) {
      cycleFailures.push(`Per-second settled lead positions and speeds do not corroborate the complete 30-second hold cycle ${cycle}.`);
    }
    if (maxDriftMetres > 2) fixtureFailures.push(`The native lead drifted more than 2 m after its settled pose in hold cycle ${cycle}.`);
    if (cycle === 2) {
      if (lastPreTerminalSecond !== undefined && lastPostTerminalSecond !== undefined) report.postResultObservedSeconds = lastPostTerminalSecond - lastPreTerminalSecond;
      const observationSecond = observationComplete && numberField(observationComplete.detail, "seconds");
      if (!terminal || !complete || complete.line >= terminal.line || !observationComplete || observationComplete.line <= terminal.line ||
          observationSecond === undefined || lastPostTerminalSecond === undefined || lastPostTerminalSecond < observationSecond ||
          boolField(observationComplete.detail, "failed") !== false ||
          (numberField(observationComplete.detail, "observed_s") ?? 0) < 30 || report.postResultObservedSeconds < 30) {
        observationFailures.push("Second hold needs 30 seconds of physical observation after the provisional terminal marker and an observation-complete marker.");
      }
    }
  }

  const leadStart = restart && vectorField(restart.detail, "start");
  const followerStart = restart && vectorField(restart.detail, "follower_start");
  const goal = restart && vectorField(restart.detail, "goal");
  const axis = restart && vectorField(restart.detail, "axis");
  const axisLength = axis ? Math.hypot(axis[0], axis[2]) : 0;
  const direction: Position | undefined = axis && axisLength >= 0.99 && axisLength <= 1.01 ? [axis[0] / axisLength, 0, axis[2] / axisLength] : undefined;
  if (!restart || boolField(restart.detail, "accepted") !== true || !leadStart || !goal || !direction ||
      (goal[0] - leadStart[0]) * direction[0] + (goal[2] - leadStart[2]) * direction[2] < 20) {
    fixtureFailures.push("An accepted native restart order with recorded start, goal, and forward route axis is required.");
  }
  if (!followerStart) followerFailures.push("The assigned follower's position at restart was not recorded.");
  const starts = [leadStart, followerStart];
  const priorPositions = [...starts];
  const priorSeconds = [restartSecond, restartSecond];
  const movingSamples = [0, 0];
  const positionsBySecond = new Map<number, Array<Position | undefined>>();
  const vehicleFailures = [fixtureFailures, followerFailures];
  const restartSamples = events.filter(event => event.name === "OFFROAD_RESTART_POSITION" && restart && event.line > restart.line &&
    (!terminal || event.line < terminal.line));
  for (const sample of restartSamples) {
    const vehicle = numberField(sample.detail, "vehicle");
    if (vehicle !== 0 && vehicle !== 1) continue;
    const second = numberField(sample.detail, "seconds");
    const origin = vectorField(sample.detail, "origin");
    const prior = priorPositions[vehicle];
    const priorSecond = priorSeconds[vehicle];
    const start = starts[vehicle];
    if (!origin || !prior || !start || !direction || second === undefined || priorSecond === undefined ||
        second <= priorSecond || second > priorSecond + 1 || distanceXZ(origin, prior) > 25) {
      const failure = `Vehicle ${vehicle} restart positions are missing, discontinuous, or not sampled once per second.`;
      if (!vehicleFailures[vehicle].includes(failure)) vehicleFailures[vehicle].push(failure);
      continue;
    }
    const step = (origin[0] - prior[0]) * direction[0] + (origin[2] - prior[2]) * direction[2];
    const proofEnd = vehicle === 0 ? leadComplete : restartComplete;
    const beforeCompletion = !proofEnd || sample.line < proofEnd.line;
    if (beforeCompletion) {
      report.restartProgressMetres[vehicle] = (origin[0] - start[0]) * direction[0] + (origin[2] - start[2]) * direction[2];
      if (step >= 0.25) movingSamples[vehicle] += 1;
    }
    const seated = boolField(sample.detail, vehicle === 0 ? "pilot_seated" : "seated_chain");
    if (seated !== true) {
      const failure = vehicle === 0 ? "Native lead pilot was not independently confirmed seated during restart." : "Follower restart does not establish the seated assigned chain.";
      if (!vehicleFailures[vehicle].includes(failure)) vehicleFailures[vehicle].push(failure);
    }
    const speed = numberField(sample.detail, "speed_kmh");
    const throttle = numberField(sample.detail, "throttle");
    const gear = numberField(sample.detail, "gear");
    if (vehicle === 0 && beforeCompletion && step >= 0.25 && speed !== undefined && Math.abs(speed) >= 1 &&
        throttle !== undefined && throttle > 0.05 && gear !== undefined && gear >= 2 && boolField(sample.detail, "engine") === true) {
      report.poweredLeadRestartSamples += 1;
    }
    priorPositions[vehicle] = origin;
    priorSeconds[vehicle] = second;
    const pair = positionsBySecond.get(second) ?? [];
    pair[vehicle] = origin;
    positionsBySecond.set(second, pair);
    if (pair[0] && pair[1]) report.maxRestartLinkGapMetres = Math.max(report.maxRestartLinkGapMetres, distanceXZ(pair[0], pair[1]));
  }
  for (const vehicle of [0, 1] as const) {
    const minimum = vehicle === 0 ? 20 : 15;
    if (report.restartProgressMetres[vehicle] < minimum || movingSamples[vehicle] < 3) {
      vehicleFailures[vehicle].push(`${vehicle === 0 ? "Native lead" : "Assigned follower"} restart needs at least ${minimum} m of signed forward progress and three moving position samples.`);
    }
  }
  if (report.poweredLeadRestartSamples < 2) fixtureFailures.push("Native lead restart needs two moving samples with the engine running, forward gear, and actual throttle above 0.05.");
  if (report.maxRestartLinkGapMetres > 60) followerFailures.push("The following link exceeded the unchanged 60 m bound during restart.");
  report.fixture.passed = fixtureFailures.length === 0;
  report.follower.passed = report.fixture.passed ? followerFailures.length === 0 : null;
  report.observation.passed = observationFailures.length === 0;
  report.passed = report.fixture.passed && report.follower.passed === true && report.observation.passed;
  report.interpretation = !report.fixture.passed
    ? "Native lead fixture acceptance failed; follower restart acceptance is inconclusive. Preserve any independent convoy failures separately."
    : !report.follower.passed
      ? "Native lead fixture passed, but the assigned follower did not meet physical restart acceptance."
      : !report.observation.passed
        ? "Initial native lead hold and powered restart, plus follower restart, passed; sustained second hold and post-result observation remain unproved."
        : "Native lead sustained holds and powered restart, assigned follower restart, and 30 seconds after the provisional result are corroborated by samples.";
  return report;
}

// A load or GAME transition is not a driving result. Select the most recent
// actual game interval and ignore its trailing Workbench editor re-init.
export function summarizeOffroadLog(logText: string, fixture: OffroadFixture = "everon", surfaceRequirement: SurfaceRequirement = "unpaved", requireHoldRestart = false): OffroadReport {
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
    // Retain context through a contiguous error block (the BTR has four
    // consecutive field errors), but never across a later unrelated log line.
    const priorDiagnostic = diagnostics.at(-1);
    if (lastResource && (index - lastResource.line <= 3 ||
        (priorDiagnostic?.line === index && priorDiagnostic.resource === lastResource.name))) diagnostic.resource = lastResource.name;
    const signature = /\b(WORLD|GUI)\s*\(E\):\s*(.*?)\s*$/.exec(line);
    const baseline = VANILLA_SIGNATURES.find(candidate => candidate.resource === diagnostic.resource &&
      candidate.family === signature?.[1] && candidate.message === signature?.[2]);
    if (baseline) {
      diagnostic.reproducedBaseline = {
        id: baseline.id,
        reference: VANILLA_BASELINE,
        line: baseline.line,
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
  const peakLinkGap: OffroadReport["peakLinkGap"] = {};
  const positionsBySecond = new Map<number, Array<{ origin: Position; timestamp?: string } | undefined>>();
  const observeGap = (metres: number, evidence: NonNullable<OffroadReport["peakLinkGap"]["evidence"]>) => {
    if (metres >= 0 && (peakLinkGap.observedMetres === undefined || metres > peakLinkGap.observedMetres)) {
      peakLinkGap.observedMetres = metres;
      peakLinkGap.evidence = evidence;
    }
  };
  const surfaceGroups = new Map<string, { truck: string; wheels: Set<number>; unpaved: boolean }>();
  for (const [runIndex, line] of run.entries()) {
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
    if (["LOST", "STUCK_TERMINAL", "CONVOY_UNIT_REMOVED", "REBOARD_STARTED", "ORDER_INVERSION", "OFFROAD_FIXTURE_FAILURE"].includes(name)) {
      failureEvents.push(line);
    }
    if (name === "OFFROAD_RESULT") {
      result = detail;
      if (detail.startsWith("FAIL")) failureEvents.push(line);
    }
    if (name === "OFFROAD_ROUTE_SELECTED") routeLength = numberField(detail, "length");
    if (name === "OFFROAD_ORDER" && boolField(detail, "accepted") === true) acceptedOrder = true;
    if (name === "OFFROAD_DRIVE_STARTED") observedDrive = true;
    // Bind extra gap observations to this one-follower fixture and its actual
    // following/arrival states. Unrelated units, on-foot states, earlier runs,
    // and post-terminal lifecycle activity cannot contaminate this gate.
    if (name === "FOLLOW_LINK_STATUS" && acceptedOrder && /^Unit 1\s+target=CF_SmokeLead\b/.test(detail) &&
        [3, 14].includes(numberField(detail, "state") ?? -1)) {
      const gap = numberField(detail, "gap");
      if (gap !== undefined) observeGap(gap, { source: name, line: initIndex + runIndex + 1, timestamp: /^\d{2}:\d{2}:\d{2}\.\d+/.exec(line)?.[0] });
    }
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
      const origin = vectorField(detail, "origin");
      if (acceptedOrder && observedDrive && second !== undefined && origin && step !== undefined && step >= 0 && step <= 25) {
        const timestamp = /^\d{2}:\d{2}:\d{2}\.\d+/.exec(line)?.[0];
        const pair = positionsBySecond.get(second) ?? [];
        pair[vehicle] ??= { origin, timestamp };
        positionsBySecond.set(second, pair);
        if (pair[0] && pair[1] && pair[0].timestamp === pair[1].timestamp) {
          observeGap(distanceXZ(pair[0].origin, pair[1].origin), { source: name, line: initIndex + runIndex + 1, seconds: second, timestamp });
        }
      }
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
  if (typeof finalMetrics.max_link_gap === "number") peakLinkGap.reportedMetres = finalMetrics.max_link_gap;
  if (peakLinkGap.reportedMetres !== undefined || peakLinkGap.observedMetres !== undefined) {
    peakLinkGap.gateMetres = Math.max(peakLinkGap.reportedMetres ?? 0, peakLinkGap.observedMetres ?? 0);
  }
  if (typeof finalMetrics.max_link_gap !== "number" || (peakLinkGap.gateMetres ?? Infinity) > 60) failures.push("The following link exceeded its 60 m short-course bound or lacks evidence; catching up only after the lead stops is insufficient.");
  if (typeof finalMetrics.max_nonprogress_s !== "number" || finalMetrics.max_nonprogress_s >= 15) failures.push("The follower stopped making progress for too long or lacks evidence.");
  if (typeof finalMetrics.settled_s !== "number" || finalMetrics.settled_s < 10 || finalMetrics.seated_chain !== true) failures.push("A seated assigned chain must settle for ten consecutive seconds.");
  if (failureEvents.length) failures.push("The selected run contains a lost, stuck, removed, reboarded, inverted, or failed fixture/convoy event.");
  const holdRestart = summarizeHoldRestart(initIndex >= 0 ? lines.slice(initIndex, shutdownIndex >= 0 ? shutdownIndex : end) : [], requireHoldRestart);
  if (holdRestart.required && holdRestart.passed !== true) failures.push(...holdRestart.fixture.failures, ...holdRestart.follower.failures, ...holdRestart.observation.failures);
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
    return event !== null && (["LOST", "STUCK_TERMINAL", "CONVOY_UNIT_REMOVED", "REBOARD_STARTED", "ORDER_INVERSION", "OFFROAD_FIXTURE_FAILURE"].includes(event[1]) ||
      (event[1] === "OFFROAD_RESULT" && event[2].startsWith("FAIL")));
  });
  failureEvents.push(...laterFailureEvents);
  if (laterFailureEvents.length) failures.push("Post-result or shutdown convoy failure events remain in the strict full-run gate; see lifecycle findings separately from the physical scenario.");
  if (errorLines.length) failures.push("The selected full run contains engine, script, resource, world, replication, network, or GUI errors; reproduced baseline errors remain strict failures.");
  const reproducedBaselineCount = diagnostics.filter(({ reproducedBaseline }) => reproducedBaseline !== undefined).length;
  return {
    surfaceRequirement, movingSurfaceSamples,
    expectedWorld, observedWorld, enteredGame, routeLength, result, finalMetrics,
    independentlyObservedMovingSeconds: maxStreak, peakLinkGap, eventCounts, failureEvents, errorLines,
    failures, passed: failures.length === 0,
    scope: {
      startLine: start + 1, endLine: end,
      gameLine: gameIndex >= 0 ? gameIndex + 1 : undefined,
      initLine: initIndex >= 0 ? initIndex + 1 : undefined,
      terminalLine: terminalIndex >= 0 ? terminalIndex + 1 : undefined,
      shutdownLine: shutdownIndex >= 0 ? shutdownIndex + 1 : undefined,
    },
    scenario, surface, holdRestart,
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
  const allowed = new Set(["--log", "--fixture", "--surface", "--require-pass", "--require-hold-restart"]);
  if (logIndex < 0 || !argv[logIndex + 1] || argv[logIndex + 1].startsWith("--") ||
      (fixtureIndex >= 0 && !["everon", "arland"].includes(argv[fixtureIndex + 1] ?? "")) ||
      (surfaceIndex >= 0 && !["unpaved", "off-network"].includes(argv[surfaceIndex + 1] ?? "")) ||
      argv.some((arg, index) => index !== logIndex + 1 && (fixtureIndex < 0 || index !== fixtureIndex + 1) && (surfaceIndex < 0 || index !== surfaceIndex + 1) && !allowed.has(arg))) {
    throw new Error("Usage: npx tsx src/convoy-offroad-report.ts --log <console.log> [--fixture everon|arland] [--surface unpaved|off-network] [--require-hold-restart] [--require-pass]");
  }
  const fixture = fixtureIndex >= 0 ? argv[fixtureIndex + 1] as OffroadFixture : "everon";
  const surface = surfaceIndex >= 0 ? argv[surfaceIndex + 1] as SurfaceRequirement : "unpaved";
  const report = summarizeOffroadLog(readFileSync(path.resolve(argv[logIndex + 1]), "utf8"), fixture, surface, argv.includes("--require-hold-restart"));
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
