"""Materialize an exact archived source set; never edits the canonical checkout.

Run only after development resumes. Example:
  python reconstruct-source.py --repo REFORGER --snapshot ordinary-road-start-join-v1 --out NEW_DIR
"""
import argparse, hashlib, json
from pathlib import Path

def digest(data):
    return hashlib.sha256(data).hexdigest().upper()

p = argparse.ArgumentParser()
p.add_argument('--repo', required=True, type=Path)
p.add_argument('--snapshot', required=True, choices=['manual-v8', 'ordinary-outbound-bay-v1', 'ordinary-outbound-bay-v2', 'ordinary-road-start-join-v1'])
p.add_argument('--out', required=True, type=Path)
args = p.parse_args()
archive = Path(__file__).resolve().parent
if args.out.exists():
    raise SystemExit('Output must not already exist; preserve existing source.')
manifest = json.loads((archive / args.snapshot / 'source-manifest.json').read_text(encoding='utf-8'))
pending = []
for entry in manifest:
    overlay = archive / args.snapshot / 'source-overlay' / entry['path']
    source = overlay if overlay.is_file() else args.repo / 'addons' / 'ConvoyFollower' / entry['path']
    raw = source.read_bytes()
    if digest(raw) != entry['sha256']:
        # Git's Windows autocrlf can change text checkout bytes. Select only a
        # conversion whose complete SHA matches the original frozen resource.
        lf = raw.replace(b'\r\n', b'\n')
        matches = [b for b in (lf, lf.replace(b'\n', b'\r\n')) if digest(b) == entry['sha256']]
        if not matches:
            raise SystemExit(f'Unreconstructable source: {entry["path"]}')
        raw = matches[0]
    if len(raw) != entry['bytes']:
        raise SystemExit(f'Size mismatch: {entry["path"]}')
    pending.append((entry['path'], raw))
# Complete read-only validation before writing the new destination.
for rel, data in pending:
    target = args.out / rel
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(data)
print(json.dumps({'snapshot': args.snapshot, 'files': len(pending), 'out': str(args.out.resolve()), 'exactHashes': True}))
