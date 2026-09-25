import { spawn } from "node:child_process";
import { existsSync, mkdirSync } from "node:fs";
import path from "node:path";
import { pathToFileURL } from "node:url";

const USAGE = `Record one named Workbench or Reforger window with ffmpeg gdigrab (dry run by default)

  npm run convoy:record -- --window-title "<exact window title>" --duration-seconds 60 --output .cache/test-videos/convoy.mp4 [--fps 30] [--ffmpeg <ffmpeg.exe>] --execute

The title must match an existing window. The helper refuses to overwrite an existing file. It captures only that window, not the full desktop.
`;

export interface RecordOptions {
  windowTitle: string;
  durationSeconds: number;
  fps: number;
  output: string;
  ffmpeg: string;
  execute: boolean;
}

export function parseRecordArgs(args: string[]): RecordOptions {
  const values = new Map<string, string>();
  let execute = false;
  for (let i = 0; i < args.length; i += 1) {
    const flag = args[i];
    if (flag === "--execute") {
      if (execute) throw new Error("--execute was provided twice.");
      execute = true;
      continue;
    }
    if (!["--window-title", "--duration-seconds", "--output", "--fps", "--ffmpeg"].includes(flag)) {
      throw new Error(`Unknown option: ${flag}\n\n${USAGE}`);
    }
    const value = args[++i];
    if (!value || value.startsWith("--") || values.has(flag)) throw new Error(`Expected one value after ${flag}.`);
    values.set(flag, value);
  }
  const windowTitle = values.get("--window-title")?.trim();
  const output = values.get("--output");
  if (!windowTitle || !output) throw new Error("--window-title and --output are required.");
  const durationSeconds = Number(values.get("--duration-seconds") ?? "60");
  const fps = Number(values.get("--fps") ?? "30");
  if (!Number.isInteger(durationSeconds) || durationSeconds < 1 || durationSeconds > 7200) {
    throw new Error("--duration-seconds must be an integer from 1 through 7200.");
  }
  if (!Number.isInteger(fps) || fps < 1 || fps > 120) throw new Error("--fps must be an integer from 1 through 120.");
  if (windowTitle.includes("\n") || windowTitle.includes("\r")) throw new Error("Window title must be one line.");
  const resolvedOutput = path.resolve(output);
  if (path.extname(resolvedOutput).toLowerCase() !== ".mp4") throw new Error("--output must end in .mp4.");
  return { windowTitle, durationSeconds, fps, output: resolvedOutput, ffmpeg: values.get("--ffmpeg") ?? "ffmpeg", execute };
}

export function recordCommand(options: RecordOptions): string[] {
  return [
    "-hide_banner", "-n", "-f", "gdigrab", "-framerate", String(options.fps),
    "-i", `title=${options.windowTitle}`, "-t", String(options.durationSeconds),
    "-an", "-c:v", "libx264", "-preset", "ultrafast", "-crf", "23",
    "-pix_fmt", "yuv420p", options.output,
  ];
}

export async function runRecordCli(argv = process.argv.slice(2)): Promise<number> {
  if (argv.length === 0 || argv[0] === "--help" || argv[0] === "-h") {
    process.stdout.write(USAGE);
    return 0;
  }
  const options = parseRecordArgs(argv);
  if (existsSync(options.output)) throw new Error(`Refusing to overwrite existing recording: ${options.output}`);
  const command = recordCommand(options);
  process.stdout.write(`${options.ffmpeg} ${command.map((arg) => JSON.stringify(arg)).join(" ")}\n`);
  if (!options.execute) return 0;
  mkdirSync(path.dirname(options.output), { recursive: true });
  return await new Promise<number>((resolve, reject) => {
    const child = spawn(options.ffmpeg, command, { stdio: "inherit", windowsHide: true });
    child.once("error", reject);
    child.once("exit", (code) => resolve(code ?? 1));
  });
}

if (process.argv[1] && import.meta.url === pathToFileURL(path.resolve(process.argv[1])).href) {
  runRecordCli().then((code) => { process.exitCode = code; }).catch((error: unknown) => {
    process.stderr.write(`${error instanceof Error ? error.message : String(error)}\n`);
    process.exitCode = 1;
  });
}
