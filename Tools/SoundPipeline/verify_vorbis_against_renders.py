#!/usr/bin/env python3
"""Check `wwise_vorbis_to_ogg` against WAVs a known-good decoder already made.

A converter that produces a file which *opens* proves very little -- a wrong bit
width in the setup header, a mis-set window flag or a bad page boundary all
yield a playable stream that drifts out of the original part way through. So
this compares decoded PCM against renders of the same media ids.

The reference is a sound dump plus its `ExtractionManifest.json`, which names
each WAV's media id and origin package; the tool pulls those same media out of
the installed game, converts them here, decodes both and reports the Pearson
correlation per channel. A sound that agrees is 1.000000 to six places.

Two mismatches are expected and reported as such rather than as failures: a
reference rendered at a different sample rate (it gets resampled here first, so
a few ten-thousandths of correlation are the resampler's), and a reference that
clipped a sample whose float value passes 1.0 -- lossy decoders regularly go
past full scale and a 16-bit WAV cannot.

  python verify_vorbis_against_renders.py --dump "<...>/Sound dump"
      --package-root <.../ReleasePC/WwiseAudioPackage> [--count 60] [--seed N]

Needs `soundfile` and `scipy`, which the runtime does not; they are only for
this check, so install them in a throwaway virtual environment rather than
adding them to anything the pipeline needs.
"""
from __future__ import annotations

import argparse
import io
import json
import random
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import wwise_audio_package as wwise          # noqa: E402
import wwise_vorbis_to_ogg as vorbis         # noqa: E402

try:
    import numpy as np
    import soundfile as sf
    from scipy.signal import resample_poly
except ImportError as error:                 # pragma: no cover - reported, not raised
    print("this check needs numpy, soundfile and scipy: %s" % error, file=sys.stderr)
    raise SystemExit(2)


def correlation(reference: Path, converted: Path):
    """(worst per-channel r, reference frames, converted frames, clipped)."""
    x, rate_x = sf.read(str(reference), dtype="float64", always_2d=True)
    y, rate_y = sf.read(str(converted), dtype="float64", always_2d=True)
    if 0 == len(y):
        return -1.0, len(x), 0, False
    if rate_x != rate_y:
        y = resample_poly(y, rate_x // 300, rate_y // 300, axis=0)
    count = min(len(x), len(y))
    clipped = bool(np.abs(x[:count]).max() >= 1.0 and np.abs(y[:count]).max() > 1.0)
    worst = 1.0
    for channel in range(min(x.shape[1], y.shape[1])):
        a = x[:count, channel] - x[:count, channel].mean()
        b = y[:count, channel] - y[:count, channel].mean()
        norm = np.linalg.norm(a) * np.linalg.norm(b)
        worst = min(worst, 1.0 if 0 == norm else float(np.dot(a, b) / norm))
    return worst, len(x), len(y), clipped


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--dump", type=Path, required=True,
                    help="reference dump holding ExtractionManifest.json")
    ap.add_argument("--package-root", type=Path, default=wwise.DEFAULT_PACKAGE_ROOT)
    ap.add_argument("--work", type=Path, default=Path("out/SoundPipelineVerify"))
    ap.add_argument("--count", type=int, default=60, help="how many entries to sample")
    ap.add_argument("--seed", type=int, default=20260917)
    a = ap.parse_args()

    manifest = json.load(io.open(a.dump / "ExtractionManifest.json", encoding="utf-8"))
    entries = [f for f in manifest["files"] if f.get("mediaIds") and f.get("origins")]
    random.seed(a.seed)
    sample = random.sample(entries, min(a.count, len(entries)))
    a.work.mkdir(parents=True, exist_ok=True)

    by_package = {}
    for entry in sample:
        by_package.setdefault(entry["origins"][0].replace("pck:", ""), []).append(entry)

    matched = resampled = clipped_only = failed = skipped = 0
    for package_name, items in sorted(by_package.items()):
        paths = wwise.find_packages(a.package_root, package_name)
        if not paths:
            print("  package %s is not installed, skipping %d" % (package_name, len(items)))
            skipped += len(items)
            continue
        packages = wwise.load_packages(paths)
        for entry in items:
            media_id = entry["mediaIds"][0]
            payload = None
            for package in packages:
                found = package.stream_by_id(media_id)
                if found is not None:
                    payload = package.payload(found)
                    break
            if payload is None:
                print("  %s: media %s not in %s" % (entry["assetId"], media_id, package_name))
                failed += 1
                continue

            wem = a.work / ("%s.wem" % media_id)
            ogg = a.work / ("%s.ogg" % media_id)
            wem.write_bytes(payload)
            try:
                vorbis.convert(wem, ogg)
            except vorbis.WemError as error:
                print("  %s: %s" % (entry["assetId"], error))
                failed += 1
                continue

            reference = a.dump / entry["assetId"].replace("Sound/", "", 1)
            if not reference.exists():
                skipped += 1
                continue
            worst, frames_x, frames_y, clipped = correlation(reference, ogg)
            if worst > 0.9999 and abs(frames_x - frames_y) <= 2:
                matched += 1
            elif clipped and worst > 0.99:
                clipped_only += 1
            elif worst > 0.999:
                resampled += 1
            else:
                failed += 1
                print("  MISMATCH %-58s r=%.6f  %d vs %d frames"
                      % (entry["assetId"], worst, frames_x, frames_y))

    print("exact %d, resampler noise %d, reference clipped %d, failed %d, skipped %d"
          % (matched, resampled, clipped_only, failed, skipped))
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
