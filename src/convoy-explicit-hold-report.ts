import { readFileSync } from "node:fs";
import path from "node:path";
import { pathToFileURL } from "node:url";

import { summarizeOffroadLog, type OffroadDiagnostic } from "./convoy-offroad-report.js";

export const EXPLICIT_HOLD_WORLD = "Worlds/Tests/ConvoyFollower_Arland_ExplicitHold_1Truck.ent";
const PREFIX = /\[ConvoyFollower\]\s+([A-Z0-9_]+):\s*(.*)$/;
const FAILURE_EVENTS = new Set([
  "LOST", "STUCK_TERMINAL", "CONVOY_UNIT_REMOVED", "CONVOY_LEADER_REPLACED", "CONVOY_ADDED",
  "REBOARD_STARTED", "REBOARDED", "REBOARD_TERMINAL", "ORDER_INVERSION", "DRIVER_DESTROYED",
  "VEHICLE_DESTROYED", "PANEL_HOLD_TIMEOUT", "PANEL_HOLD_REBOARD_BLOCKED", "ABORTED",
  "STAND_DOWN", "FOLLOW_FAILED", "GET_OUT_FAILED", "OFFROAD_FIXTURE_FAILURE",
]);
type Vec = [number, number, number];
interface Event { name: string; detail: string; line: number }
interface Motion { event: Event; seconds: number; vehicle: number; origin: Vec; progress: number; step: number; speed: number; powered: boolean }
interface Gate { passed: boolean; failures: string[] }
interface Leg {
  progressMetres: [number, number];
  poweredSamples: [number, number];
  movingSamples: [number, number];
  samples: Motion[];
}

function token(event: Event | undefined, field: string): string | undefined {
  return event && new RegExp(`(?:^|\\s)${field}=([^\\s]+)(?:\\s|$)`).exec(event.detail)?.[1];
}
function num(event: Event | undefined, field: string): number | undefined {
  const text = token(event, field);
  if (text === undefined || !/^-?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?$/.test(text)) return undefined;
  const value = Number(text);
  return Number.isFinite(value) ? value : undefined;
}
function bool(event: Event | undefined, field: string): boolean | undefined {
  const value = token(event, field);
  return value === "true" ? true : value === "false" ? false : undefined;
}
// Enforce prints the inline (character == owner) comparison as 0/1, while
// declared bool fields print true/false. Accept both only for this seat field.
function seatOwner(event: Event): boolean | undefined {
  const value = token(event, "owner");
  return value === "1" ? true : value === "0" ? false : bool(event, "owner");
}
function vec(event: Event | undefined, field: string): Vec | undefined {
  const match = event && new RegExp(`(?:^|\\s)${field}=<([^>]+)>`).exec(event.detail);
  const values = match?.[1].split(",").map(value => value.trim() === "" ? NaN : Number(value));
  return values?.length === 3 && values.every(Number.isFinite) ? values as Vec : undefined;
}
function distanceXZ(a: Vec, b: Vec): number { return Math.hypot(a[0] - b[0], a[2] - b[2]); }
function project(a: Vec, b: Vec, axis: Vec): number { return (a[0] - b[0]) * axis[0] + (a[2] - b[2]) * axis[2]; }
function failure(event: Event): boolean {
  return FAILURE_EVENTS.has(event.name) || (event.name === "EXPLICIT_HOLD_RESULT" && !/^PASS\b/.test(event.detail));
}
function gate(failures: string[]): Gate { return { passed: failures.length === 0, failures: [...new Set(failures)] }; }

// Read the existing reporter only for launch boundaries and exact diagnostic
// provenance. Its offroad scenario/terminal/surface gates are never reused.
export function summarizeExplicitHoldLog(logText: string) {
  const baseline = summarizeOffroadLog(logText, "arland", "off-network");
  const lines = logText.split(/\r?\n/);
  const { startLine, endLine, gameLine, shutdownLine } = baseline.scope;
  const events: Event[] = [];
  for (let index = startLine - 1; index < endLine; index++) {
    const match = PREFIX.exec(lines[index]);
    if (match) events.push({ name: match[1], detail: match[2], line: index + 1 });
  }
  const initEvents = events.filter(event => event.name === "EXPLICIT_HOLD_INIT");
  const init = initEvents[0];
  const terminals = events.filter(event => event.name === "EXPLICIT_HOLD_RESULT");
  const terminal = terminals[0]; // The first failure can never be rescued later.
  const physicalEnd = Math.min(terminal?.line ?? endLine, shutdownLine ? shutdownLine - 1 : endLine);
  const before = events.filter(event => event.line >= (init?.line ?? startLine) && event.line <= physicalEnd);
  const later = events.filter(event => terminal && event.line > terminal.line);
  const failures: string[] = [];
  const counts: Record<string, number> = {};
  for (const event of events) counts[event.name] = (counts[event.name] ?? 0) + 1;
  const named = (name: string) => before.filter(event => event.name === name);
  const one = (name: string): Event | undefined => {
    const found = named(name);
    if (found.length !== 1) failures.push(`Exactly one ${name} is required before the first terminal.`);
    return found[0];
  };
  const atLeast = (event: Event | undefined, field: string, value: number) => (num(event, field) ?? -Infinity) >= value;

  let loadedWorld: string | undefined;
  for (const line of lines.slice(startLine - 1, init?.line ?? endLine)) {
    const found = /Entities load ['"]([^'"]+\.ent)['"]/.exec(line.replaceAll("\\", "/"));
    if (found) loadedWorld = found[1];
  }
  const observedWorld = loadedWorld === EXPLICIT_HOLD_WORLD || loadedWorld?.endsWith(`:${EXPLICIT_HOLD_WORLD}`) === true;
  if (!observedWorld) failures.push("The most recently loaded world is not the exact explicit Hold fixture.");
  if (initEvents.length !== 1 || num(init, "expected") !== 1 || bool(init, "native_lead") !== true ||
      bool(init, "production_follower") !== true || bool(init, "test_seat_transfers") !== true || bool(init, "human_input_claim") !== false ||
      events.some(event => event.name === "OFFROAD_INIT" || event.name === "AUTO_INIT" || /^AUTO_.*_INIT$/.test(event.name))) {
    failures.push("One explicit one-follower probe with declared native lead and test-only seat transfers is required.");
  }
  if (!terminal || !/^PASS\b/.test(terminal.detail) || (shutdownLine !== undefined && terminal.line >= shutdownLine)) {
    failures.push("The first explicit terminal is not a pre-shutdown PASS.");
  }
  const phases = named("EXPLICIT_HOLD_PHASE");
  if (phases.map(event => num(event, "phase")).join(",") !== "1,2,3,4,5,6,7,8,9,10,11" ||
      phases.some((event, index) => num(event, "seconds") === undefined ||
        (index > 0 && num(event, "seconds")! <= num(phases[index - 1], "seconds")!))) {
    failures.push("The ordered, strictly advancing fixture phases 1 through 11 are incomplete or duplicated.");
  }
  const phase = (value: number) => phases.find(event => num(event, "phase") === value);
  const enteredGame = gameLine !== undefined && gameLine >= startLine && gameLine < (phase(2)?.line ?? -1) && gameLine <= physicalEnd;
  if (!enteredGame) failures.push("GAME must precede the physical driving phases in this launch.");

  const recruit = one("EXPLICIT_HOLD_RECRUIT");
  const convoyStarted = one("CONVOY_STARTED");
  const holdCommand = one("EXPLICIT_HOLD_COMMAND");
  const holdAccepted = one("PANEL_HOLD_ACCEPTED");
  const held = one("PANEL_HOLD");
  const capture = one("EXPLICIT_HOLD_CAPTURE");
  const holdVerified = one("EXPLICIT_HOLD_VERIFIED");
  const resumeCommand = one("EXPLICIT_RESUME_COMMAND");
  const resumed = one("PANEL_RESUME");
  const resumeAccepted = one("PANEL_RESUME_ACCEPTED");
  const resumeVerified = one("EXPLICIT_RESUME_VERIFIED");
  const final = one("EXPLICIT_HOLD_FINAL");
  const roster = one("EXPLICIT_HOLD_ROSTER");
  if (bool(recruit, "accepted") !== true || !convoyStarted || convoyStarted.line >= (phase(2)?.line ?? -1)) failures.push("Recruitment and actual production membership were not established before driving.");
  for (const [command, accepted, action, name] of [
    [holdCommand, holdAccepted, held, "Hold"], [resumeCommand, resumeAccepted, resumed, "Resume"],
  ] as const) {
    if (bool(command, "accepted") !== true || bool(command, "owner_pilot") !== true ||
        !accepted || !command || accepted.line >= command.line || !action || !/^Unit 1\b/.test(action.detail)) {
      failures.push(`Real production ${name} acceptance and assigned Unit 1 execution with eligible owner pilot are required.`);
    }
  }
  const order = [init, recruit, phase(2), holdCommand, capture, phase(7), holdVerified, resumeCommand, phase(10), resumeVerified, final, roster, terminal];
  if (order.some((event, index) => !event || (index > 0 && event.line <= (order[index - 1]?.line ?? Infinity)))) failures.push("Command, capture, physical proof and first terminal ordering is invalid.");
  if (!held || !holdAccepted || held.line <= holdAccepted.line || held.line >= (capture?.line ?? -1) ||
      !resumed || !resumeAccepted || resumed.line >= resumeAccepted.line || resumed.line <= (holdVerified?.line ?? Infinity)) {
    failures.push("Production held/resumed state transitions do not surround the physical Hold window.");
  }
  if (bool(holdVerified, "same_driver_truck_roster") !== true || bool(resumeVerified, "same_driver_truck_roster") !== true ||
      !roster || !/^1\|1\|CF_SmokeFollower1 #1\|[^;|]+\|owner$/.test(roster.detail)) {
    failures.push("Same driver/truck/roster assertions and the single assigned follower roster are missing or changed.");
  }

  // Teleport exits here are narrowly declared lead-occupant setup. They are
  // never counted as proof of production recovery or restored human input.
  const setupDirections = [true, false, true, false];
  const setups = named("EXPLICIT_HOLD_SEAT_SETUP");
  if (setups.length !== 4 || setups.some((event, index) => bool(event, "to_owner_pilot") !== setupDirections[index] ||
      token(event, "vehicle") !== "CF_SmokeLead" || bool(event, "temporary_test_setup") !== true || bool(event, "human_input_claim") !== false)) {
    failures.push("The four declared fixture-only lead seat transfers are incomplete or target another vehicle.");
  }
  for (const [index, value] of [4, 6, 8, 9].entries()) {
    const begin = phase(value)?.line ?? Infinity;
    const end = phase(value + 1)?.line ?? -1;
    const setup = setups[index];
    const seats = before.filter(event => event.line > begin && event.line < end &&
      ["EXPLICIT_HOLD_SEAT_EXIT", "EXPLICIT_HOLD_SEAT_BOARD"].includes(event.name));
    const expected = setupDirections[index] ? ["exit:false", "exit:true", "board:true:true"] : ["exit:true", "board:true:false", "board:false:true"];
    const observed = seats.map(event => event.name === "EXPLICIT_HOLD_SEAT_EXIT"
      ? `exit:${seatOwner(event)}` : `board:${seatOwner(event)}:${bool(event, "pilot")}`);
    if (!setup || setup.line <= begin || setup.line >= end || observed.join(",") !== expected.join(",") ||
        seats.some(event => bool(event, "accepted") !== true || bool(event, "temporary_test_setup") !== true)) {
      failures.push(`Phase ${value} does not establish the expected lead occupants before the next command or move.`);
    }
  }
  const route = one("OFFROAD_ROUTE_SELECTED");
  const axis = vec(route, "axis");
  if (!axis || Math.abs(Math.hypot(axis[0], axis[2]) - 1) > 0.01 || Math.abs(axis[1]) > 0.01) failures.push("A valid horizontal signed route axis is missing.");
  const summarizeLeg = (value: number): Leg => {
    const leg: Leg = { progressMetres: [0, 0], poweredSamples: [0, 0], movingSamples: [0, 0], samples: [] };
    const samples = named("EXPLICIT_HOLD_MOTION").filter(event => num(event, "phase") === value);
    const previous: Array<Motion | undefined> = [];
    const sums = [0, 0];
    const phaseStart = phase(value);
    const phaseEnd = phase(value + 1);
    for (const event of samples) {
      const vehicle = num(event, "vehicle");
      const seconds = num(event, "seconds");
      const origin = vec(event, "origin");
      const progress = num(event, "progress_m");
      const step = num(event, "signed_step_m");
      const speed = num(event, "speed_kmh");
      const throttle = num(event, "throttle");
      const gear = num(event, "gear");
      if ((vehicle !== 0 && vehicle !== 1) || seconds === undefined || !Number.isInteger(seconds) || !origin ||
          progress === undefined || step === undefined || speed === undefined || throttle === undefined || gear === undefined ||
          bool(event, "engine") === undefined || bool(event, "powered") === undefined || Math.abs(step) > 25 ||
          event.line <= (phaseStart?.line ?? Infinity) || event.line >= (phaseEnd?.line ?? -1)) {
        failures.push(`Phase ${value} has missing, malformed, misplaced or discontinuous motion telemetry.`);
        continue;
      }
      const prior = previous[vehicle];
      if (prior && (seconds !== prior.seconds + 1 || distanceXZ(origin, prior.origin) > 25 ||
          Math.abs(progress - prior.progress - step) > 0.15 ||
          (axis && Math.abs(project(origin, prior.origin, axis) - step) > 0.15))) {
        failures.push(`Phase ${value} vehicle ${vehicle} positions, signed steps, progress or sample continuity disagree.`);
      }
      if (!prior && (seconds !== (num(phaseStart, "seconds") ?? -Infinity) + 1 || Math.abs(progress - step) > 0.15)) failures.push(`Phase ${value} vehicle ${vehicle} lacks the first motion sample.`);
      const powered = step >= 0.25 && Math.abs(speed) >= 1 && bool(event, "engine") === true && throttle > 0.05 && gear >= 2;
      if (bool(event, "powered") !== powered) failures.push(`Phase ${value} claimed power contradicts recorded engine, gear, throttle or motion.`);
      if (step >= 0.25) leg.movingSamples[vehicle]++;
      if (powered) leg.poweredSamples[vehicle]++;
      sums[vehicle] += step;
      if (Math.abs(sums[vehicle] - progress) > 0.2) failures.push(`Phase ${value} aggregate progress is not corroborated by signed movement samples.`);
      leg.progressMetres[vehicle] = progress;
      const sample = { event, seconds, vehicle, origin, progress, step, speed, powered };
      previous[vehicle] = sample;
      leg.samples.push(sample);
    }
    const leadTimes = leg.samples.filter(sample => sample.vehicle === 0).map(sample => sample.seconds);
    const followerTimes = leg.samples.filter(sample => sample.vehicle === 1).map(sample => sample.seconds);
    if (!leadTimes.length || leadTimes.join(",") !== followerTimes.join(",")) failures.push(`Phase ${value} requires synchronized lead/follower positions.`);
    return leg;
  };
  const initial = summarizeLeg(2);
  const holdLeg = summarizeLeg(7);
  const resumeLeg = summarizeLeg(10);
  if (initial.progressMetres[0] < 40 || initial.progressMetres[1] < 20 || initial.poweredSamples.some(count => count < 2)) failures.push("Both trucks must establish powered initial movement before Hold.");
  if (holdLeg.progressMetres[0] < 20 || holdLeg.poweredSamples[0] < 2) failures.push("The lead must physically advance at least 20 m with powered samples while the follower holds.");
  if (resumeLeg.progressMetres[0] < 20 || resumeLeg.progressMetres[1] < 15 || resumeLeg.poweredSamples.some(count => count < 2) || resumeLeg.movingSamples.some(count => count < 3)) failures.push("Resume requires corroborated powered progress of 20 m lead and 15 m follower, with three moving samples each.");

  const holdOrigin = vec(capture, "origin");
  const observations = named("EXPLICIT_HOLD_OBSERVE");
  let heldSeconds = 0;
  let maxDrift = 0;
  let maxSpeed = 0;
  let priorHeld = 0;
  const holdTimes = holdLeg.samples.filter(sample => sample.vehicle === 1).map(sample => sample.seconds);
  if (!holdOrigin || !observations.length || observations.map(event => num(event, "seconds")).join(",") !== holdTimes.join(",")) failures.push("The entire Hold window needs synchronized positions and uninterrupted observation records.");
  for (const event of observations) {
    const time = num(event, "held_s");
    const drift = num(event, "max_drift_m");
    const speed = num(event, "max_speed_kmh");
    const leadProgress = num(event, "lead_progress_m");
    const leadSample = holdLeg.samples.find(sample => sample.vehicle === 0 && sample.seconds === num(event, "seconds"));
    if (time === undefined || time <= priorHeld || time > priorHeld + 2 || drift === undefined || drift < 0 || speed === undefined || speed < 0 ||
        bool(event, "hold_requested") !== true || !leadSample || leadProgress === undefined || Math.abs(leadProgress - leadSample.progress) > 0.15 ||
        event.line <= (phase(7)?.line ?? Infinity) || event.line >= (holdVerified?.line ?? -1)) failures.push("Hold elapsed time, retained intent, drift/speed telemetry or synchronized lead advance is invalid.");
    if (time !== undefined) { heldSeconds = time; priorHeld = time; }
    maxDrift = Math.max(maxDrift, drift ?? Infinity);
    maxSpeed = Math.max(maxSpeed, speed ?? Infinity);
  }
  for (const sample of holdLeg.samples.filter(sample => sample.vehicle === 1)) {
    if (holdOrigin) maxDrift = Math.max(maxDrift, distanceXZ(sample.origin, holdOrigin));
    maxSpeed = Math.max(maxSpeed, Math.abs(sample.speed));
  }
  maxDrift = Math.max(maxDrift, num(final, "max_hold_drift_m") ?? Infinity, num(holdVerified, "max_drift_m") ?? Infinity);
  maxSpeed = Math.max(maxSpeed, num(final, "max_hold_speed_kmh") ?? Infinity);
  if (heldSeconds < 30 || !atLeast(holdVerified, "held_s", 30) || (num(holdVerified, "held_s") ?? Infinity) > heldSeconds + 0.01 || maxDrift > 2 || maxSpeed > 2) failures.push("Sustained Hold must cover at least 30 seconds, remain within 2 m, and stay at or below 2 km/h.");
  if (bool(final, "hold_accepted") !== true || bool(final, "resume_accepted") !== true || num(final, "phase") !== 11 ||
      !atLeast(final, "lead_progress_m", 20) || !atLeast(final, "follower_progress_m", 15) ||
      !atLeast(final, "lead_powered_samples", 2) || !atLeast(final, "follower_powered_samples", 2) ||
      !atLeast(resumeVerified, "lead_progress_m", 20) || !atLeast(resumeVerified, "follower_progress_m", 15)) failures.push("Final and Resume verification metrics do not corroborate completed commands and powered movement.");
  const scenarioFailureEvents = before.filter(failure);
  if (scenarioFailureEvents.length) failures.push("The physical scenario contains a convoy or fixture failure event.");
  const scenario = gate(failures);

  const diagnostics: OffroadDiagnostic[] = [...baseline.runtime.errors, ...baseline.lifecycle.errors]
    .sort((a, b) => a.line - b.line).map(error => ({ ...error, phase: shutdownLine && error.line >= shutdownLine ? "shutdown"
      : terminal && error.line > terminal.line ? "post-result" : gameLine && error.line >= gameLine ? "gameplay" : "load" }));
  const runtimeErrors = diagnostics.filter(error => error.phase !== "shutdown");
  const shutdownErrors = diagnostics.filter(error => error.phase === "shutdown");
  const laterFailures = later.filter(failure);
  const duplicateTerminal = terminals.length > 1;
  const shutdownObserved = shutdownLine !== undefined;
  const destroyed = shutdownObserved && lines.slice(shutdownLine - 1, endLine).some(line => /\bGame destroyed\b/.test(line));
  const strictFailures = [...scenario.failures];
  if (duplicateTerminal || laterFailures.length) strictFailures.push("Duplicate terminals or post-terminal failure events prevent a strict pass; later markers never repair earlier evidence.");
  if (diagnostics.length) strictFailures.push("Nonzero runtime/shutdown errors remain strict failures, including errors reproduced in vanilla.");
  if (!destroyed) strictFailures.push("A completed shutdown was not observed; a terminal snapshot cannot establish the full lifecycle.");
  const strict = gate(strictFailures);
  return {
    expectedWorld: EXPLICIT_HOLD_WORLD, observedWorld, enteredGame, result: terminal?.detail,
    passed: strict.passed, failures: strict.failures, scenario,
    scope: { startLine, endLine, gameLine, initLine: init?.line, terminalLine: terminal?.line, shutdownLine },
    metrics: { heldSeconds, maxHoldDriftMetres: Number.isFinite(maxDrift) ? maxDrift : null, maxHoldSpeedKmh: Number.isFinite(maxSpeed) ? maxSpeed : null,
      initialProgressMetres: initial.progressMetres, holdLeadProgressMetres: holdLeg.progressMetres[0], holdLeadPoweredSamples: holdLeg.poweredSamples[0],
      resumeProgressMetres: resumeLeg.progressMetres, resumePoweredSamples: resumeLeg.poweredSamples, resumeMovingSamples: resumeLeg.movingSamples },
    identity: { assertionsPresent: bool(holdVerified, "same_driver_truck_roster") === true && bool(resumeVerified, "same_driver_truck_roster") === true,
      finalRoster: roster?.detail, interpretation: "Same entity/seat/session checks run inside the fixture; this log corroborates its assertions and named roster, not independently logged entity IDs." },
    fixtureSeatTransfers: { count: setups.length, productionTeleportProof: false, humanInputProof: false,
      interpretation: "Declared lead-occupant teleport exits are temporary test setup. They do not validate production recovery, ordinary menu input, or restored human driving." },
    eventCounts: counts, failureEvents: [...scenarioFailureEvents, ...laterFailures],
    runtime: { clean: runtimeErrors.length === 0, errors: runtimeErrors, reproducedBaselineCount: runtimeErrors.filter(error => error.reproducedBaseline).length },
    postTerminal: { physicalObservationVerified: false, failureEvents: laterFailures.filter(event => !shutdownLine || event.line < shutdownLine),
      errors: diagnostics.filter(error => error.phase === "post-result"),
      interpretation: "This fixture stops physical sampling at its first terminal. Later silence is not a physical observation pass; later errors and failures remain visible." },
    shutdown: { observed: shutdownObserved, completed: destroyed, clean: shutdownObserved ? shutdownErrors.length === 0 && !laterFailures.some(event => event.line >= shutdownLine!) : null,
      errors: shutdownErrors, failureEvents: laterFailures.filter(event => shutdownLine && event.line >= shutdownLine) },
    fullRun: { passed: strict.passed, errorCount: diagnostics.length, reproducedBaselineCount: diagnostics.filter(error => error.reproducedBaseline).length },
    interpretation: scenario.passed ? strict.passed
      ? "Strict supplied-log gates passed for one follower and real server Hold/Resume. Inspect the gameplay recording; post-terminal physical observation and human input remain unverified."
      : "Physical Hold/Resume evidence passed, but strict full-run acceptance failed. Inspect runtime and shutdown findings separately."
      : "Physical explicit Hold/Resume evidence failed or is incomplete; command acceptance and a terminal claim alone are insufficient.",
  };
}

export function runExplicitHoldReport(argv: string[]): number {
  let log: string | undefined;
  let requirePass = false;
  for (let index = 0; index < argv.length; index++) {
    const arg = argv[index];
    if (arg === "--log" && log === undefined && argv[index + 1] && !argv[index + 1].startsWith("--")) log = argv[++index];
    else if (arg === "--require-pass" && !requirePass) requirePass = true;
    else throw new Error("Usage: npx tsx src/convoy-explicit-hold-report.ts --log <console.log> [--require-pass]");
  }
  if (!log) throw new Error("Usage: npx tsx src/convoy-explicit-hold-report.ts --log <console.log> [--require-pass]");
  const report = summarizeExplicitHoldLog(readFileSync(path.resolve(log), "utf8"));
  process.stdout.write(`${JSON.stringify(report, null, 2)}\n`);
  return requirePass && !report.passed ? 1 : 0;
}

if (process.argv[1] && import.meta.url === pathToFileURL(path.resolve(process.argv[1])).href) {
  try { process.exitCode = runExplicitHoldReport(process.argv.slice(2)); }
  catch (error) { process.stderr.write(`${error instanceof Error ? error.message : String(error)}\n`); process.exitCode = 1; }
}
