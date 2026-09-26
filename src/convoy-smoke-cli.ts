import { existsSync, readFileSync } from "node:fs";
import { mkdir, writeFile } from "node:fs/promises";
import path from "node:path";
import { pathToFileURL } from "node:url";

import { enteredGameState, runClientCli } from "./client-cli.js";

const ADDON_ID = "5A5FB20BD40C7C70";
const WORLDS: Record<number, string> = {
  1: "Worlds/Tests/ConvoyFollower_Arland_Auto_1Truck.ent",
  2: "Worlds/Tests/ConvoyFollower_Arland_Auto_2Trucks.ent",
  3: "Worlds/Tests/ConvoyFollower_Arland_Auto_3Trucks.ent",
};

type SmokeVariant = "harbor" | "open-road" | "release" | "release-wide" | "pause-resume" | "forward-wait" | "forward-blocked" | "forward-twoahead" | "forward-twoahead-wide";

// Workbench 1.8.0.13 reports these same base-world data warnings on every
// Arland F5 load on this machine, before any ConvoyFollower probe initializes.
// Keep the list exact and narrow: a changed offset or an error during gameplay
// must be reviewed rather than silently excused.
const KNOWN_WORKBENCH_LOAD_ERRORS = new Set([
  "Unknown keyword/data 'SlidingTrackMaterial' at offset 19441(0x4bf1)",
  "Unknown keyword/data 'Parent' at offset 35955(0x8c73)",
  "Unknown keyword/data 'Parent' at offset 36915(0x9033)",
  "Unknown keyword/data 'Parent' at offset 42646(0xa696)",
  "Unknown keyword/data 'Parent' at offset 43645(0xaa7d)",
  "Unknown keyword/data 'Parent' at offset 44617(0xae49)",
  "Unknown keyword/data 'Parent' at offset 45590(0xb216)",
]);

function worldFor(trucks: 1 | 2 | 3, variant: SmokeVariant): string {
  if (variant === "harbor") return WORLDS[trucks];
  if (variant === "release") {
    if (trucks !== 2) throw new Error("The release variant requires --trucks 2.");
    return "Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_Release.ent";
  }
  if (variant === "release-wide") {
    if (trucks !== 2) throw new Error("The release-wide variant requires --trucks 2.");
    return "Worlds/Tests/ConvoyFollower_Arland_WideRoad_Auto_2Trucks_Release.ent";
  }
  if (variant === "pause-resume") {
    if (trucks !== 2) throw new Error("The pause-resume variant requires --trucks 2.");
    return "Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_PauseResume.ent";
  }
  if (variant === "forward-wait") {
    if (trucks !== 2) throw new Error("The forward-wait variant requires --trucks 2.");
    return "Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_ForwardWait.ent";
  }
  if (variant === "forward-blocked") {
    if (trucks !== 2) throw new Error("The forward-blocked variant requires --trucks 2.");
    return "Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_ForwardBlocked.ent";
  }
  if (variant === "forward-twoahead") {
    if (trucks !== 3) throw new Error("The forward-twoahead variant requires --trucks 3.");
    return "Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_3Trucks_ForwardTwoAhead.ent";
  }
  if (variant === "forward-twoahead-wide") {
    if (trucks !== 3) throw new Error("The forward-twoahead-wide variant requires --trucks 3.");
    return "Worlds/Tests/ConvoyFollower_Arland_WideRoad_Auto_3Trucks_ForwardTwoAhead.ent";
  }
  return `Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_${trucks}Truck${trucks === 1 ? "" : "s"}.ent`;
}

const USAGE = `Convoy Follower scripted road-route test runner (dry run by default)

  npm run convoy:test -- --trucks 1|2|3 [--variant harbor|open-road|release|release-wide|pause-resume|forward-wait|forward-blocked|forward-twoahead|forward-twoahead-wide] [--duration-seconds 360 | --keep-open] [--run-dir <new-directory>] [--profile <isolated-directory>] [--addons-dir <packed-addons-root>] [--game <ArmaReforgerSteam.exe>] [--execute]
  npm run convoy:test -- --trucks 1|2|3 [--variant harbor|open-road|release|release-wide|pause-resume|forward-wait|forward-blocked|forward-twoahead|forward-twoahead-wide] --report <console.log> [--require-pass]

Pack the addon first. Each auto world has a test-only probe that attempts player possession, convoy orders, and a sustained road drive. The 1-truck scene also orders explicit unload release. The report distinguishes script-observed movement from visual gameplay proof. Standalone client behavior still needs a live test.
`;

interface SmokeOptions {
  trucks: 1 | 2 | 3;
  variant: SmokeVariant;
  runDir: string;
  profile: string;
  addonsDir: string;
  game?: string;
  durationSeconds?: number;
  keepOpen: boolean;
  execute: boolean;
  reportLog?: string;
  requirePass: boolean;
}

export interface EnRouteMetrics {
  maxGap: number;
  warningSeconds: number;
  noProgressSeconds: number;
}

export interface SmokeReport {
  expectedWorld: string;
  expectedFollowerTrucks: number;
  observedWorld: boolean;
  enteredGame: boolean;
  observedExpectedCount: boolean;
  enRouteMetrics?: EnRouteMetrics;
  eventCounts: Record<string, number>;
  autoResult?: string;
  releaseResult?: string;
  sequenceResult?: string;
  pauseResult?: string;
  forwardResult?: string;
  forwardBlockedResult?: string;
  twoAheadResult?: string;
  twoAheadParkingPassed?: boolean;
  twoAheadHoldObserved?: boolean;
  twoAheadHoldPassed?: boolean;
  errorLines: string[];
  baselineLoadErrorLines: string[];
  passed: boolean;
  interpretation: string;
}

export function parseSmokeArgs(argv: string[], now = new Date(), pid = process.pid): SmokeOptions {
  const values = new Map<string, string>();
  let keepOpen = false;
  let execute = false;
  let requirePass = false;
  for (let index = 0; index < argv.length; index += 1) {
    const flag = argv[index];
    if (flag === "--execute" || flag === "--keep-open" || flag === "--require-pass") {
      if (flag === "--execute") {
        if (execute) throw new Error("--execute was provided twice.");
        execute = true;
      } else if (flag === "--keep-open") {
        if (keepOpen) throw new Error("--keep-open was provided twice.");
        keepOpen = true;
      } else {
        if (requirePass) throw new Error("--require-pass was provided twice.");
        requirePass = true;
      }
      continue;
    }
    if (!["--trucks", "--variant", "--run-dir", "--profile", "--addons-dir", "--game", "--duration-seconds", "--report"].includes(flag)) {
      throw new Error(`Unknown option: ${flag}\n\n${USAGE}`);
    }
    const value = argv[++index];
    if (!value || value.startsWith("--") || values.has(flag)) throw new Error(`Expected one value after ${flag}.`);
    values.set(flag, value);
  }

  const count = Number(values.get("--trucks"));
  if (count !== 1 && count !== 2 && count !== 3) throw new Error("--trucks must be 1, 2, or 3.");
  const variant = values.get("--variant") ?? "harbor";
  if (variant !== "harbor" && variant !== "open-road" && variant !== "release" && variant !== "release-wide" && variant !== "pause-resume" && variant !== "forward-wait" && variant !== "forward-blocked" && variant !== "forward-twoahead" && variant !== "forward-twoahead-wide") throw new Error("--variant must be harbor, open-road, release, release-wide, pause-resume, forward-wait, forward-blocked, forward-twoahead, or forward-twoahead-wide.");
  if (variant === "release" && count !== 2) throw new Error("The release variant requires --trucks 2.");
  if (variant === "release-wide" && count !== 2) throw new Error("The release-wide variant requires --trucks 2.");
  if (variant === "pause-resume" && count !== 2) throw new Error("The pause-resume variant requires --trucks 2.");
  if (variant === "forward-wait" && count !== 2) throw new Error("The forward-wait variant requires --trucks 2.");
  if (variant === "forward-blocked" && count !== 2) throw new Error("The forward-blocked variant requires --trucks 2.");
  if (variant === "forward-twoahead" && count !== 3) throw new Error("The forward-twoahead variant requires --trucks 3.");
  if (variant === "forward-twoahead-wide" && count !== 3) throw new Error("The forward-twoahead-wide variant requires --trucks 3.");
  if (keepOpen && values.has("--duration-seconds")) throw new Error("--keep-open and --duration-seconds cannot be combined.");
  if (values.has("--report") && (execute || keepOpen || values.has("--duration-seconds"))) {
    throw new Error("--report reads an existing log and cannot launch a client.");
  }
  if (requirePass && !values.has("--report")) throw new Error("--require-pass requires --report.");
  const durationSeconds = values.has("--duration-seconds") ? Number(values.get("--duration-seconds")) : undefined;
  if (durationSeconds !== undefined && (!Number.isInteger(durationSeconds) || durationSeconds < 5 || durationSeconds > 7200)) {
    throw new Error("--duration-seconds must be an integer from 5 through 7200.");
  }
  const runName = `${now.toISOString().replaceAll(":", "-").replaceAll(".", "-")}-${pid}`;
  const runDir = path.resolve(values.get("--run-dir") ?? path.join(".cache", "convoy-tests", "runs", runName));
  return {
    trucks: count,
    variant,
    runDir,
    profile: path.resolve(values.get("--profile") ?? path.join(".cache", "client", "profiles", "convoy-smoke")),
    addonsDir: path.resolve(values.get("--addons-dir") ?? path.join(".cache", "local-addons")),
    game: values.get("--game") ? path.resolve(values.get("--game")!) : undefined,
    durationSeconds,
    keepOpen,
    execute,
    reportLog: values.get("--report") ? path.resolve(values.get("--report")!) : undefined,
    requirePass,
  };
}

export function summarizeSmokeLog(logText: string, trucks: 1 | 2 | 3, variant: SmokeVariant = "harbor"): SmokeReport {
  const expectedWorld = worldFor(trucks, variant);
  const lines = logText.split(/\r?\n/);
  // Workbench appends F5 previews and an edit-mode reload to one console.log.
  // The edit-mode reload creates another AUTO_INIT without entering GAME.
  // Report the last actual F5 session, not that trailing editor instance.
  let lastInit = -1;
  for (let index = lines.length - 1; index >= 0; index -= 1) {
    if (/\[ConvoyFollower\]\s+AUTO_INIT:/.test(lines[index])) {
      lastInit = index;
      break;
    }
  }
  let lastGame = -1;
  let lastReload = -1;
  for (let index = lines.length - 1; index >= 0; index -= 1) {
    if (lastGame < 0 && /OnGameStateChanged\s*=\s*GAME\b/.test(lines[index])) lastGame = index;
    if (lastReload < 0 && /Workbench Reload Game/.test(lines[index])) lastReload = index;
    if (lastGame >= 0 && lastReload >= 0) break;
  }
  let selectedInit = lastInit;
  let runEnd = lines.length;
  if (lastGame >= 0 && lastReload > lastGame) {
    runEnd = lastReload;
    for (let index = lastGame; index >= 0; index -= 1) {
      if (/\[ConvoyFollower\]\s+AUTO_INIT:/.test(lines[index])) {
        selectedInit = index;
        break;
      }
    }
  }
  let runStart = selectedInit >= 0 ? selectedInit : 0;
  if (selectedInit >= 0) {
    for (let index = selectedInit - 1; index >= 0; index -= 1) {
      if (/\bEntities load\b[^\r\n]*\.ent['"]/.test(lines[index])) {
        runStart = index;
        break;
      }
    }
  }
  const currentRunLines = lines.slice(runStart, runEnd);
  const eventLines = selectedInit >= 0 ? lines.slice(selectedInit, runEnd) : currentRunLines;
  const worldName = path.basename(expectedWorld).replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
  const worldLine = new RegExp(`\\bEntities load\\b[^\\r\\n]*${worldName}`, "i");
  const counts: Record<string, number> = {};
  const errors: string[] = [];
  const baselineLoadErrors: string[] = [];
  let autoResult: string | undefined;
  let releaseResult: string | undefined;
  let sequenceResult: string | undefined;
  let pauseResult: string | undefined;
  let forwardResult: string | undefined;
  let forwardBlockedResult: string | undefined;
  let twoAheadResult: string | undefined;
  let enRouteMetrics: EnRouteMetrics | undefined;
  for (const line of eventLines) {
    const event = /\[ConvoyFollower\]\s+([A-Z][A-Z0-9_]+):/.exec(line);
    if (event) counts[event[1]] = (counts[event[1]] ?? 0) + 1;
    const result = /\[ConvoyFollower\]\s+AUTO_RESULT:\s*(.+)/.exec(line);
    if (result) autoResult = result[1].trim();
    const release = /\[ConvoyFollower\]\s+AUTO_RELEASE_RESULT:\s*(.+)/.exec(line);
    if (release) releaseResult = release[1].trim();
    const sequence = /\[ConvoyFollower\]\s+AUTO_SEQUENCE_RESULT:\s*(.+)/.exec(line);
    if (sequence) sequenceResult = sequence[1].trim();
    const pause = /\[ConvoyFollower\]\s+AUTO_PAUSE_RESULT:\s*(.+)/.exec(line);
    if (pause) pauseResult = pause[1].trim();
    const forward = /\[ConvoyFollower\]\s+AUTO_FORWARD_RESULT:\s*(.+)/.exec(line);
    if (forward) forwardResult = forward[1].trim();
    const forwardBlocked = /\[ConvoyFollower\]\s+AUTO_FORWARD_BLOCKED_RESULT:\s*(.+)/.exec(line);
    if (forwardBlocked) forwardBlockedResult = forwardBlocked[1].trim();
    const twoAhead = /\[ConvoyFollower\]\s+AUTO_TWO_AHEAD_RESULT:\s*(.+)/.exec(line);
    if (twoAhead) twoAheadResult = twoAhead[1].trim();
    const metrics = /\[ConvoyFollower\]\s+AUTO_EN_ROUTE_METRICS:\s+max_gap=([0-9.]+)\s+warning_s=(\d+)\s+no_progress_s=(\d+)/.exec(line);
    if (metrics) enRouteMetrics = {
      maxGap: Number(metrics[1]),
      warningSeconds: Number(metrics[2]),
      noProgressSeconds: Number(metrics[3]),
    };
  }
  // Errors can occur during world/entity setup before AUTO_INIT. Keep the
  // event counts scoped to the probe, but inspect the whole selected F5 run.
  // Only the seven observed base-world load messages above are exempt, and
  // only before this run's probe init. Report them separately for visibility.
  for (let index = 0; index < currentRunLines.length; index += 1) {
    const line = currentRunLines[index];
    if (!/\b(?:ENGINE|WORLD|RESOURCES|SCRIPT|RPL)\s*\(E\):/.test(line)) continue;
    const worldError = /\bWORLD\s*\(E\):\s*(.+)$/.exec(line);
    if (worldError && selectedInit >= 0 && index < selectedInit - runStart &&
      KNOWN_WORKBENCH_LOAD_ERRORS.has(worldError[1].trim())) {
      if (baselineLoadErrors.length < 30) baselineLoadErrors.push(line.trim());
      continue;
    }
    if (errors.length < 30) errors.push(line.trim());
  }
  const observedWorld = currentRunLines.some((line) => worldLine.test(line));
  const enteredGame = enteredGameState(currentRunLines.join("\n"));
  const observedExpectedCount = selectedInit >= 0 && new RegExp(`\\[ConvoyFollower\\]\\s+AUTO_INIT:\\s+expected=${trucks}(?:\\D|$)`).test(lines[selectedInit]);
  const qualityPassed = enRouteMetrics !== undefined &&
    enRouteMetrics.warningSeconds < 8 && enRouteMetrics.noProgressSeconds < 15;
  const routePassed = autoResult?.startsWith("PASS") && qualityPassed;
  const blockedLaneReasonSeen = eventLines.some((line) =>
    /\[ConvoyFollower\]\s+FORWARD_WAIT_REJECTED:\s+Move your lead vehicle off the driving lane to let this truck pass/.test(line));
  const twoAheadParkingPassed = observedWorld && enteredGame && observedExpectedCount && Boolean(routePassed) && errors.length === 0 &&
    (counts.AUTO_TWO_AHEAD_PARKED ?? 0) >= 1 &&
    (counts.FORWARD_WAIT_PARKED ?? 0) >= 2 && (counts.FORWARD_WAIT_BAY_CLEAR ?? 0) >= 2;
  const twoAheadStageStart = eventLines.findIndex((line) => /\[ConvoyFollower\]\s+AUTO_TWO_AHEAD_SHOULDER:/.test(line));
  const twoAheadStageEnd = eventLines.findIndex((line) => /\[ConvoyFollower\]\s+AUTO_TWO_AHEAD_RESULT:/.test(line));
  const twoAheadStageLines = eventLines.slice(
    twoAheadStageStart >= 0 ? twoAheadStageStart : 0,
    twoAheadStageEnd >= 0 ? twoAheadStageEnd + 1 : undefined,
  );
  // F5 shutdown can remove members after a completed probe. Only judge the
  // active unloading/forward-wait interval through its terminal result.
  const twoAheadStageStallOrRemoval = twoAheadStageLines.some((line) =>
    /\[ConvoyFollower\]\s+(?:STUCK_TERMINAL|CONVOY_UNIT_REMOVED):/.test(line));
  const twoAheadHoldObserved = twoAheadStageLines.some((line) =>
    /\[ConvoyFollower\]\s+FORWARD_OUTBOUND_HOLD_REQUESTED:/.test(line)) &&
    twoAheadStageLines.some((line) =>
      /\[ConvoyFollower\]\s+FORWARD_OUTBOUND_HOLD_COMPLETE:/.test(line));
  const twoAheadHoldPassed = twoAheadParkingPassed &&
    twoAheadHoldObserved && !twoAheadStageStallOrRemoval;
  const passed = observedWorld && enteredGame && observedExpectedCount && Boolean(routePassed) && errors.length === 0 &&
    ((variant !== "release" && variant !== "release-wide") || Boolean(sequenceResult?.startsWith("PASS"))) &&
    (variant !== "release-wide" || ((counts.AUTO_SEQUENCE_PARKED ?? 0) >= 1 &&
      (counts.AUTO_SEQUENCE_RETURN_MERGED ?? 0) >= 1 &&
      (counts.AUTO_SEQUENCE_RETURN_DRIVE ?? 0) >= 1)) &&
    (variant !== "pause-resume" || Boolean(pauseResult?.startsWith("PASS"))) &&
    (variant !== "forward-wait" || Boolean(forwardResult?.startsWith("PASS"))) &&
    (variant !== "forward-blocked" || (Boolean(forwardBlockedResult?.startsWith("PASS")) &&
      blockedLaneReasonSeen && !counts.FORWARD_WAIT_REQUESTED)) &&
    ((variant !== "forward-twoahead" && variant !== "forward-twoahead-wide") || (twoAheadHoldPassed && Boolean(twoAheadResult?.startsWith("PASS")) &&
      (counts.AUTO_TWO_AHEAD_POST_RESUME ?? 0) >= 1 && (counts.FORWARD_WAIT_RESUME_LINE ?? 0) >= 1));
  return {
    expectedWorld,
    expectedFollowerTrucks: trucks,
    observedWorld,
    enteredGame,
    observedExpectedCount,
    enRouteMetrics,
    eventCounts: Object.fromEntries(Object.entries(counts).sort(([a], [b]) => a.localeCompare(b))),
    autoResult,
    releaseResult,
    sequenceResult,
    pauseResult,
    forwardResult,
    forwardBlockedResult,
    twoAheadResult,
    ...(variant === "forward-twoahead" || variant === "forward-twoahead-wide" ? { twoAheadParkingPassed, twoAheadHoldObserved, twoAheadHoldPassed } : {}),
    errorLines: errors,
    baselineLoadErrorLines: baselineLoadErrors,
    passed,
    interpretation: errors.length > 0
      ? "The selected F5 run contains engine, world, resource, script, or replication errors. Inspect them before treating any gameplay PASS marker as valid."
      : variant === "forward-twoahead" || variant === "forward-twoahead-wide"
      ? passed
        ? "Script observed two separated seated forward parks, a physical owner-vehicle pass, explicit roster rechain, and renewed movement of all three trucks. Review video for turn and spacing quality."
        : twoAheadParkingPassed && !twoAheadHoldObserved
          ? "Both forward parking steps passed, but the owner never moved far enough to exercise a completed outbound hold. The full resume gate failed."
        : twoAheadParkingPassed && !twoAheadHoldPassed
          ? "Both forward parking steps passed, but an active follower reached stuck terminal or was removed while waiting. The hold and final resume gates failed."
          : twoAheadParkingPassed
            ? "Both forward parking steps passed, but the owner pass or explicit resume and renewed physical movement did not."
            : "No passing two-ahead parking sequence with verified world, three trucks, GAME state, road arrival, and en-route quality."
      : variant === "forward-blocked"
      ? passed
        ? "Script observed the lane-blocked forward order being rejected with the specific obstruction reason while the front driver stayed seated and stationary. Review the live video for the actual lane geometry."
        : "No passing lane-blocked rejection sequence with verified world, truck count, GAME state, road arrival, and en-route quality. A generic rejection is insufficient."
      : variant === "forward-wait"
      ? passed
        ? "Script observed a forward-wait order, physical bay clearance, seated parking in a forward slot, and the successor reaching the unload bay. Review the live video for passing space and driving quality."
        : "No passing forward-wait sequence with verified world, truck count, GAME state, road arrival, and en-route quality. Order acceptance alone is insufficient."
      : variant === "pause-resume"
      ? passed
        ? "Script observed a stopped convoy, player exit, seated follower hold, player reboarding, and a completed second road leg. Review the live video for vehicle behavior and input quality."
        : "No passing exit-and-resume sequence with verified world, truck count, GAME state, road arrival, and en-route quality. Boarding or an accepted waypoint alone is insufficient."
      : variant === "release" || variant === "release-wide"
      ? passed
        ? "Script observed two-truck arrival, parked unload release, bay advance, and both trucks rejoining on a homeward route. Review the live video for driving quality."
        : "No passing unload and return sequence with verified world, truck count, GAME state, and en-route quality. A road arrival alone does not verify parking or regroup."
      : passed
        ? "Script observed sustained road travel, bounded en-route behavior, and a settled convoy. Review live video for driving quality and test combat, menus, and audio separately."
        : "No passing autonomous movement result with verified world, truck count, GAME state, and en-route quality. Startup and event counts alone do not verify driving.",
  };
}

function printReport(report: SmokeReport): void {
  process.stdout.write(`Expected world: ${report.expectedWorld}\n`);
  process.stdout.write(`Expected follower trucks: ${report.expectedFollowerTrucks}\n`);
  process.stdout.write(`World load seen: ${report.observedWorld ? "yes" : "no"}; GAME seen: ${report.enteredGame ? "yes" : "no"}\n`);
  process.stdout.write(`Expected truck count seen: ${report.observedExpectedCount ? "yes" : "no"}\n`);
  process.stdout.write(`Convoy events: ${Object.entries(report.eventCounts).map(([event, count]) => `${event}=${count}`).join(", ") || "none"}\n`);
  process.stdout.write(`Autonomous result: ${report.autoResult ?? "none"}\n`);
  if (report.enRouteMetrics) process.stdout.write(`En-route peak gap: ${report.enRouteMetrics.maxGap}m; warning: ${report.enRouteMetrics.warningSeconds}s; nonprogress: ${report.enRouteMetrics.noProgressSeconds}s\n`);
  if (report.releaseResult) process.stdout.write(`Unload-release result: ${report.releaseResult}\n`);
  if (report.sequenceResult) process.stdout.write(`Unload and return sequence: ${report.sequenceResult}\n`);
  if (report.pauseResult) process.stdout.write(`Stop, exit, and resume sequence: ${report.pauseResult}\n`);
  if (report.forwardResult) process.stdout.write(`Forward-wait sequence: ${report.forwardResult}\n`);
  if (report.forwardBlockedResult) process.stdout.write(`Forward lane-blocked sequence: ${report.forwardBlockedResult}\n`);
  if (report.twoAheadResult) process.stdout.write(`Two-ahead and explicit resume sequence: ${report.twoAheadResult}\n`);
  if (report.twoAheadParkingPassed !== undefined) {
    process.stdout.write(`Two-ahead physical parking gate: ${report.twoAheadParkingPassed ? "PASS" : "FAIL"}\n`);
    process.stdout.write(`Outbound hold requested and completed: ${report.twoAheadHoldObserved ? "yes" : "no"}\n`);
    process.stdout.write(`Unreleased follower hold gate (completed without stuck terminal/removal): ${report.twoAheadHoldPassed ? "PASS" : "FAIL"}\n`);
  }
  process.stdout.write(`Engine/world/resource/script/replication error lines (first 30): ${report.errorLines.length}\n`);
  if (report.baselineLoadErrorLines.length > 0) {
    process.stdout.write(`Known pre-init Workbench world-load errors ignored: ${report.baselineLoadErrorLines.length}\n`);
  }
  process.stdout.write(`Smoke gate: ${report.passed ? "PASS" : "FAIL"}\n`);
  process.stdout.write(`${report.interpretation}\n`);
}

export async function runSmokeCli(argv = process.argv.slice(2)): Promise<number> {
  if (argv.length === 0 || argv[0] === "--help" || argv[0] === "-h") {
    process.stdout.write(USAGE);
    return 0;
  }
  const options = parseSmokeArgs(argv);
  if (options.reportLog) {
    if (!existsSync(options.reportLog)) throw new Error(`Log does not exist: ${options.reportLog}`);
    const report = summarizeSmokeLog(readFileSync(options.reportLog, "utf8"), options.trucks, options.variant);
    printReport(report);
    return options.requirePass && !report.passed ? 1 : 0;
  }

  const packedProject = path.join(options.addonsDir, "ConvoyFollower", "addon.gproj");
  const packedData = path.join(options.addonsDir, "ConvoyFollower", "data.pak");
  if (!existsSync(packedProject) || !existsSync(packedData)) {
    throw new Error(`Pack ConvoyFollower first. Expected ${packedProject} and ${packedData}`);
  }

  const clientArgs = [
    "--run-dir", options.runDir,
    "--profile", options.profile,
    "--addons-dir", options.addonsDir,
    "--addon", ADDON_ID,
    "--world", worldFor(options.trucks, options.variant),
    "--expect-game",
  ];
  if (options.game) clientArgs.push("--game", options.game);
  if (options.keepOpen) clientArgs.push("--keep-open");
  else clientArgs.push("--duration-seconds", String(options.durationSeconds ?? 360));
  if (options.execute) clientArgs.push("--execute");

  process.stdout.write(`Convoy test: ${options.trucks} follower truck${options.trucks === 1 ? "" : "s"}\n`);
  process.stdout.write(`Scenario: ${worldFor(options.trucks, options.variant)}\n`);
  process.stdout.write(`Report: ${path.join(options.runDir, "smoke-report.json")}\n`);
  let clientResult = 1;
  let clientError: unknown;
  try {
    clientResult = await runClientCli(clientArgs);
  } catch (error) {
    clientError = error;
  }
  if (!options.execute) {
    if (clientError) throw clientError;
    return clientResult;
  }

  const consoleLog = path.join(options.runDir, "logs", "console.log");
  if (existsSync(consoleLog)) {
    const report = summarizeSmokeLog(readFileSync(consoleLog, "utf8"), options.trucks, options.variant);
    await mkdir(options.runDir, { recursive: true });
    await writeFile(path.join(options.runDir, "smoke-report.json"), `${JSON.stringify(report, null, 2)}\n`, "utf8");
    printReport(report);
    if (clientResult === 0 && !report.passed) {
      throw new Error(`Expected world, GAME, and passing autonomous result(s) were not all observed. Inspect ${consoleLog}`);
    }
  }
  if (clientError) throw clientError;
  return clientResult;
}

if (process.argv[1] && import.meta.url === pathToFileURL(path.resolve(process.argv[1])).href) {
  runSmokeCli().then((code) => {
    process.exitCode = code;
  }).catch((error: unknown) => {
    process.stderr.write(`${error instanceof Error ? error.message : String(error)}\n`);
    process.exitCode = 1;
  });
}
