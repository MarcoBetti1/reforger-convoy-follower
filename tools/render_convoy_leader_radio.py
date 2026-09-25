"""Render one-speaker convoy radio calls from a marked continuous recording.

Requires FFmpeg on PATH; uses only Python's standard library otherwise.
The original recording and the addon's current Sounds/Radio clips are never edited.
The opening and closing use the user's approved fuller, click-free squelch.
"""

from __future__ import annotations

import argparse
import array
import hashlib
import json
import math
import random
import subprocess
import sys
import wave
from pathlib import Path


RATE = 48_000
CURRENT_RADIO_DIR = Path(__file__).resolve().parents[1] / "addons" / "ConvoyFollower" / "Sounds" / "Radio"
SQUELCH_TEMPLATE = Path(__file__).resolve().parent / "assets" / "convoy_radio_squelch_no_click.wav"
SQUELCH_OPENING_SAMPLES = int(0.175 * RATE)
SQUELCH_CLOSING_SAMPLES = int(0.150 * RATE)
# The template was cut from ready_radio_squelch_no_click.wav, whose middle
# contains the rendered leader_ready voice without any further processing.
SQUELCH_REFERENCE_FIRST = -163
SQUELCH_REFERENCE_LAST = 166
LEADER_EVENTS = (
    "ready", "following", "holding", "far_warning", "stuck", "lost", "rejoined"
)
OPTIONAL_EVENTS = ("under_fire",)
MEMBER_EVENTS = ("far_warning", "stuck", "lost", "rejoined")
UNIT_NUMBERS = range(1, 11)
GAP_SAMPLES = int(0.060 * RATE)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def decode_voice(source: Path) -> list[float]:
    """Decode one source once and apply a narrow, compressed radio voice EQ."""
    command = [
        "ffmpeg", "-v", "error", "-i", str(source),
        "-af", "highpass=f=420,lowpass=f=3000,"
        "equalizer=f=1400:t=q:w=1:g=2.5,"
        "acompressor=threshold=0.035:ratio=4.2:attack=3:release=110:makeup=1.6",
        "-ac", "1", "-ar", str(RATE), "-f", "s16le", "-acodec", "pcm_s16le", "pipe:1",
    ]
    try:
        completed = subprocess.run(command, capture_output=True, check=True)
    except FileNotFoundError as exc:
        raise RuntimeError("FFmpeg is required on PATH") from exc
    except subprocess.CalledProcessError as exc:
        raise RuntimeError(exc.stderr.decode("utf-8", errors="replace").strip()) from exc
    samples = array.array("h")
    samples.frombytes(completed.stdout)
    if sys.byteorder != "little":
        samples.byteswap()
    if not samples:
        raise ValueError(f"No audio decoded from {source}")
    return [sample / 32768.0 for sample in samples]


def normalise_fragment(samples: list[float]) -> list[float]:
    """Trim empty edges and equalise active-voice level without clipping."""
    active = [i for i, value in enumerate(samples) if abs(value) > 0.010]
    if not active:
        raise ValueError("A cut contains no audible voice; check its timestamps")
    pad = int(0.045 * RATE)
    samples = samples[max(0, active[0] - pad): min(len(samples), active[-1] + pad + 1)]
    voiced = [value for value in samples if abs(value) > 0.010]
    rms = math.sqrt(sum(value * value for value in voiced) / len(voiced))
    peak = max(abs(value) for value in samples)
    gain = min(3.0, 0.16 / max(rms, 0.0001), 0.84 / max(peak, 0.0001))
    fade = min(int(0.012 * RATE), len(samples) // 2)
    # Dense but gentle saturation makes a rough microphone sound deliberately
    # transmitted rather than accidentally noisy. The 0.87 factor leaves headroom.
    output = [math.tanh(value * gain * 1.5) * 0.87 for value in samples]
    for i in range(fade):
        scale = (i + 1) / fade
        output[i] *= scale
        output[-i - 1] *= scale
    return output


def read_cuts(cuts_path: Path, source_length: int) -> dict[str, tuple[float, float]]:
    raw = json.loads(cuts_path.read_text(encoding="utf-8"))
    if not isinstance(raw, dict):
        raise ValueError("Cut sheet must be a JSON object")
    required = (
        [f"leader_{event}" for event in LEADER_EVENTS]
        + ["unit_prefix"]
        + [f"number_{number}" for number in UNIT_NUMBERS]
        + [f"member_{event}" for event in MEMBER_EVENTS]
    )
    missing = [key for key in required if key not in raw or raw[key] is None]
    if missing:
        raise ValueError("Cut sheet needs timestamps for: " + ", ".join(missing))
    parsed: dict[str, tuple[float, float]] = {}
    for key, value in raw.items():
        if value is None:
            continue
        if not (
            isinstance(value, list) and len(value) == 2
            and all(isinstance(v, (int, float)) and not isinstance(v, bool) for v in value)
        ):
            raise ValueError(f"{key}: expected [start_seconds, end_seconds]")
        start, end = map(float, value)
        if not (0 <= start < end <= source_length / RATE):
            raise ValueError(f"{key}: cut is outside the recording or has no duration")
        parsed[key] = (start, end)
    for event in OPTIONAL_EVENTS:
        leader = f"leader_{event}"
        member = f"member_{event}"
        if (leader in parsed) != (member in parsed):
            raise ValueError(f"Optional event {event} needs both leader and member takes")
    return parsed


def band_noise(count: int, seed: int) -> list[float]:
    """Deterministic, very quiet hiss under voice, as in the approved preview."""
    rng = random.Random(seed)
    a_high = math.exp(-2 * math.pi * 5500 / RATE)
    a_low = math.exp(-2 * math.pi * 450 / RATE)
    high = 0.0
    low = 0.0
    output: list[float] = []
    for _ in range(count):
        white = rng.uniform(-1.0, 1.0)
        high = a_high * high + (1 - a_high) * white
        low = a_low * low + (1 - a_low) * white
        output.append(high - low)
    return output


def approved_squelch() -> tuple[list[float], list[float]]:
    """Load the exact opening/closing noise from the approved ready preview."""
    with wave.open(str(SQUELCH_TEMPLATE), "rb") as stream:
        if (stream.getnchannels(), stream.getsampwidth(), stream.getframerate()) != (1, 2, RATE):
            raise ValueError(f"Unexpected squelch template format: {SQUELCH_TEMPLATE}")
        if stream.getnframes() != SQUELCH_OPENING_SAMPLES + SQUELCH_CLOSING_SAMPLES:
            raise ValueError(f"Unexpected squelch template length: {SQUELCH_TEMPLATE}")
        pcm = array.array("h")
        pcm.frombytes(stream.readframes(stream.getnframes()))
    if sys.byteorder != "little":
        pcm.byteswap()
    if pcm[SQUELCH_OPENING_SAMPLES - 1] != SQUELCH_REFERENCE_FIRST:
        raise ValueError("Squelch opening no longer matches the approved preview")
    if pcm[SQUELCH_OPENING_SAMPLES] != SQUELCH_REFERENCE_LAST:
        raise ValueError("Squelch closing no longer matches the approved preview")
    opening = [sample / 32768.0 for sample in pcm[:SQUELCH_OPENING_SAMPLES]]
    closing = [sample / 32768.0 for sample in pcm[SQUELCH_OPENING_SAMPLES:]]
    return opening, closing


def pcm_boundary(sample: float) -> int:
    """Match the 16-bit rounding that save_wav will apply."""
    return int(round(max(-0.96, min(0.96, sample)) * 32767))


def assemble(fragments: list[list[float]], label: str) -> list[float]:
    seed = int.from_bytes(hashlib.sha256(label.encode("utf-8")).digest()[:8], "little")
    voice: list[float] = []
    for index, fragment in enumerate(fragments):
        if index:
            voice.extend([0.0] * GAP_SAMPLES)
        voice.extend(fragment)
    # Very quiet synthetic hiss under the spoken portion ties the edits together.
    bed = band_noise(len(voice), seed + 2)
    voice = [sample + hiss * 0.020 for sample, hiss in zip(voice, bed)]
    opening, closing = approved_squelch()

    # The preview template's noise already fades to the ready voice's first
    # sample and starts at its last sample. Match other calls' edge samples
    # over 12 ms so neither splice produces a click or pop.
    edge = int(0.012 * RATE)
    head_delta = (pcm_boundary(voice[0]) - SQUELCH_REFERENCE_FIRST) / 32768.0
    tail_delta = (pcm_boundary(voice[-1]) - SQUELCH_REFERENCE_LAST) / 32768.0
    for i in range(edge):
        opening[-edge + i] += head_delta * (i + 1) / edge
        closing[i] += tail_delta * (edge - i) / edge
    return opening + voice + closing


def save_wav(path: Path, samples: list[float], overwrite: bool) -> None:
    if path.exists() and not overwrite:
        raise FileExistsError(f"Refusing to overwrite {path}; pass --overwrite to replace it")
    path.parent.mkdir(parents=True, exist_ok=True)
    pcm = array.array("h", (
        int(round(max(-0.96, min(0.96, sample)) * 32767)) for sample in samples
    ))
    if sys.byteorder != "little":
        pcm.byteswap()
    with wave.open(str(path), "wb") as stream:
        stream.setnchannels(1)
        stream.setsampwidth(2)
        stream.setframerate(RATE)
        stream.writeframes(pcm.tobytes())


def run_demo(args: argparse.Namespace) -> None:
    source = args.source.resolve(strict=True)
    output = args.output.resolve()
    if output == source or output.is_relative_to(CURRENT_RADIO_DIR.resolve()):
        raise ValueError("Choose a new preview path outside the source and current addon radio folder")
    samples = decode_voice(source)
    if args.start is not None or args.end is not None:
        if args.start is None or args.end is None or not (0 <= args.start < args.end <= len(samples) / RATE):
            raise ValueError("Demo cuts require valid --start and --end seconds")
        samples = samples[int(args.start * RATE): int(args.end * RATE)]
    result = assemble([normalise_fragment(samples)], "demonstration")
    save_wav(args.output, result, args.overwrite)
    print(f"Created {args.output} ({len(result) / RATE:.2f} s); source unchanged: {source}")


def run_render(args: argparse.Namespace) -> None:
    source = args.source.resolve(strict=True)
    cuts_path = args.cuts.resolve(strict=True)
    output_dir = args.output_dir.resolve()
    if source.is_relative_to(output_dir) or output_dir.is_relative_to(CURRENT_RADIO_DIR.resolve()):
        raise ValueError("Use an output directory separate from source and current addon radio assets")
    if args.output_dir.exists() and any(args.output_dir.iterdir()) and not args.overwrite:
        raise FileExistsError("Output directory is not empty; use a new directory or --overwrite")
    samples = decode_voice(source)
    cuts = read_cuts(cuts_path, len(samples))
    fragments = {
        key: normalise_fragment(samples[int(start * RATE): int(end * RATE)])
        for key, (start, end) in cuts.items()
    }
    leader_events = list(LEADER_EVENTS)
    member_events = list(MEMBER_EVENTS)
    if all(f"leader_{event}" in fragments for event in OPTIONAL_EVENTS):
        leader_events += list(OPTIONAL_EVENTS)
        member_events += list(OPTIONAL_EVENTS)
    clips: list[dict[str, object]] = []

    def emit(stem: str, keys: list[str]) -> None:
        output = args.output_dir / f"{stem}.wav"
        assembled = assemble([fragments[key] for key in keys], stem)
        save_wav(output, assembled, args.overwrite)
        clips.append({"name": stem, "path": output.name,
                      "duration_seconds": round(len(assembled) / RATE, 3), "sha256": sha256(output)})

    for event in leader_events:
        emit(f"leader_{event}", [f"leader_{event}"])
    for number in UNIT_NUMBERS:
        for event in member_events:
            emit(f"unit_{number}_{event}", ["unit_prefix", f"number_{number}", f"member_{event}"])
    manifest = {
        "source": str(source), "source_sha256": sha256(source),
        "cut_sheet": str(cuts_path), "cut_sheet_sha256": sha256(cuts_path),
        "format": "mono 48000 Hz 16-bit PCM WAV",
        "effect": "approved fuller click-free opening/closing squelch, band-limited voice",
        "squelch_template": str(SQUELCH_TEMPLATE),
        "squelch_template_sha256": sha256(SQUELCH_TEMPLATE),
        "clips": clips,
    }
    manifest_path = args.output_dir / "manifest.json"
    if manifest_path.exists() and not args.overwrite:
        raise FileExistsError(f"Refusing to overwrite {manifest_path}")
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"Created {len(clips)} calls in {args.output_dir}; originals unchanged")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    demo = sub.add_parser("demo", help="Try the opening/closing effect on one existing voice clip")
    demo.add_argument("--source", type=Path, required=True)
    demo.add_argument("--output", type=Path, required=True)
    demo.add_argument("--start", type=float, help="Optional start of a voice take, in seconds")
    demo.add_argument("--end", type=float, help="Optional end of a voice take, in seconds")
    demo.add_argument("--overwrite", action="store_true")
    render = sub.add_parser("render", help="Render all marked leader and numbered-member reports")
    render.add_argument("--source", type=Path, required=True)
    render.add_argument("--cuts", type=Path, required=True)
    render.add_argument("--output-dir", type=Path, required=True)
    render.add_argument("--overwrite", action="store_true")
    args = parser.parse_args()
    try:
        (run_demo if args.command == "demo" else run_render)(args)
    except (ValueError, FileExistsError, RuntimeError, OSError) as exc:
        parser.exit(1, f"error: {exc}\n")


if __name__ == "__main__":
    main()
