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

const USAGE = `Convoy Follower scripted short-drive test runner (dry run by default)

  npm run convoy:test -- --trucks 1|2|3 [--duration-seconds 180 | --keep-open] [--run-dir <new-directory>] [--profile <isolated-directory>] [--addons-dir <packed-addons-root>] [--game <ArmaReforgerSteam.exe>] [--execute]
  npm run convoy:test -- --trucks 1|2|3 --report <console.log>

Pack the addon first. Each auto world has a test-only probe that attempts player possession, convoy orders, and a bounded 35 m lead drive. The report distinguishes script-observed movement from visual gameplay proof. The probe's runtime behavior still needs a live F5 and standalone client test.
`;

interface SmokeOptions {
  trucks: 1 | 2 | 3;
  runDir: string;
  profile: string;
  addonsDir: string;
  game?: string;
  durationSeconds?: number;
  keepOpen: boolean;
  execute: boolean;
  reportLog?: string;
}

export interface SmokeReport {
  expectedWorld: string;
  expectedFollowerTrucks: number;
  observedWorld: boolean;
  enteredGame: boolean;
  eventCounts: Record<string, number>;
  autoResult?: string;
  errorLines: string[];
  interpretation: string;
}

export function parseSmokeArgs(argv: string[], now = new Date(), pid = process.pid): SmokeOptions {
  const values = new Map<string, string>();
  let keepOpen = false;
  let execute = false;
  for (let index = 0; index < argv.length; index += 1) {
    const flag = argv[index];
    if (flag === "--execute" || flag === "--keep-open") {
      if (flag === "--execute") {
        if (execute) throw new Error("--execute was provided twice.");
        execute = true;
      } else {
        if (keepOpen) throw new Error("--keep-open was provided twice.");
        keepOpen = true;
      }
      continue;
    }
    if (!["--trucks", "--run-dir", "--profile", "--addons-dir", "--game", "--duration-seconds", "--report"].includes(flag)) {
      throw new Error(`Unknown option: ${flag}\n\n${USAGE}`);
    }
    const value = argv[++index];
    if (!value || value.startsWith("--") || values.has(flag)) throw new Error(`Expected one value after ${flag}.`);
    values.set(flag, value);
  }

  const count = Number(values.get("--trucks"));
  if (count !== 1 && count !== 2 && count !== 3) throw new Error("--trucks must be 1, 2, or 3.");
  if (keepOpen && values.has("--duration-seconds")) throw new Error("--keep-open and --duration-seconds cannot be combined.");
  if (values.has("--report") && (execute || keepOpen || values.has("--duration-seconds"))) {
    throw new Error("--report reads an existing log and cannot launch a client.");
  }
  const durationSeconds = values.has("--duration-seconds") ? Number(values.get("--duration-seconds")) : undefined;
  if (durationSeconds !== undefined && (!Number.isInteger(durationSeconds) || durationSeconds < 5 || durationSeconds > 7200)) {
    throw new Error("--duration-seconds must be an integer from 5 through 7200.");
  }
  const runName = `${now.toISOString().replaceAll(":", "-").replaceAll(".", "-")}-${pid}`;
  const runDir = path.resolve(values.get("--run-dir") ?? path.join(".cache", "convoy-tests", "runs", runName));
  return {
    trucks: count,
    runDir,
    profile: path.resolve(values.get("--profile") ?? path.join(".cache", "client", "profiles", "convoy-smoke")),
    addonsDir: path.resolve(values.get("--addons-dir") ?? path.join(".cache", "local-addons")),
    game: values.get("--game") ? path.resolve(values.get("--game")!) : undefined,
    durationSeconds,
    keepOpen,
    execute,
    reportLog: values.get("--report") ? path.resolve(values.get("--report")!) : undefined,
  };
}

export function summarizeSmokeLog(logText: string, trucks: 1 | 2 | 3): SmokeReport {
  const expectedWorld = WORLDS[trucks];
  const lines = logText.split(/\r?\n/);
  const worldName = path.basename(expectedWorld).replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
  const worldLine = new RegExp(`\\bEntities load\\b[^\\r\\n]*${worldName}`, "i");
  const counts: Record<string, number> = {};
  const errors: string[] = [];
  let autoResult: string | undefined;
  for (const line of lines) {
    const event = /\[ConvoyFollower\]\s+([A-Z][A-Z0-9_]+):/.exec(line);
    if (event) counts[event[1]] = (counts[event[1]] ?? 0) + 1;
    const result = /\[ConvoyFollower\]\s+AUTO_RESULT:\s*(.+)/.exec(line);
    if (result) autoResult = result[1].trim();
    if (/\b(?:ENGINE|WORLD|RESOURCES|SCRIPT|RPL)\s*\(E\):/.test(line) && errors.length < 30) errors.push(line.trim());
  }
  const observedWorld = lines.some((line) => worldLine.test(line));
  const enteredGame = enteredGameState(logText);
  return {
    expectedWorld,
    expectedFollowerTrucks: trucks,
    observedWorld,
    enteredGame,
    eventCounts: Object.fromEntries(Object.entries(counts).sort(([a], [b]) => a.localeCompare(b))),
    autoResult,
    errorLines: errors,
    interpretation: autoResult?.startsWith("PASS")
      ? "Script observed a short lead drive and follower displacement. Review positions and a live video for road behavior, spacing, contact response, menus, and audio."
      : "No passing autonomous movement result. World/GAME startup and event counts alone do not verify driving or gameplay.",
  };
}

function printReport(report: SmokeReport): void {
  process.stdout.write(`Expected world: ${report.expectedWorld}\n`);
  process.stdout.write(`Expected follower trucks: ${report.expectedFollowerTrucks}\n`);
  process.stdout.write(`World load seen: ${report.observedWorld ? "yes" : "no"}; GAME seen: ${report.enteredGame ? "yes" : "no"}\n`);
  process.stdout.write(`Convoy events: ${Object.entries(report.eventCounts).map(([event, count]) => `${event}=${count}`).join(", ") || "none"}\n`);
  process.stdout.write(`Autonomous result: ${report.autoResult ?? "none"}\n`);
  process.stdout.write(`Engine error lines (first 30): ${report.errorLines.length}\n`);
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
    printReport(summarizeSmokeLog(readFileSync(options.reportLog, "utf8"), options.trucks));
    return 0;
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
    "--world", WORLDS[options.trucks],
    "--expect-game",
  ];
  if (options.game) clientArgs.push("--game", options.game);
  if (options.keepOpen) clientArgs.push("--keep-open");
  else clientArgs.push("--duration-seconds", String(options.durationSeconds ?? 180));
  if (options.execute) clientArgs.push("--execute");

  process.stdout.write(`Convoy test: ${options.trucks} follower truck${options.trucks === 1 ? "" : "s"}\n`);
  process.stdout.write(`Scenario: ${WORLDS[options.trucks]}\n`);
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
    const report = summarizeSmokeLog(readFileSync(consoleLog, "utf8"), options.trucks);
    await mkdir(options.runDir, { recursive: true });
    await writeFile(path.join(options.runDir, "smoke-report.json"), `${JSON.stringify(report, null, 2)}\n`, "utf8");
    printReport(report);
    if (clientResult === 0 && (!report.observedWorld || !report.enteredGame || !report.autoResult?.startsWith("PASS"))) {
      throw new Error(`Expected world, GAME, and a passing AUTO_RESULT were not all observed. Inspect ${consoleLog}`);
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
