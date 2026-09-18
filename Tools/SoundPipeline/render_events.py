#!/usr/bin/env python3
"""Turn Wwise event names into playable audio in one step.

Chains the two halves of this pipeline: `wwise_audio_package` decrypts the
`.pck` packages, walks the bank hierarchy and resolves an event name to the
`.wem` media it plays, and `wwise_vorbis_to_ogg` converts each of those into a
standard Ogg Vorbis file. Nothing outside this repo is needed -- no vgmstream,
no ww2ogg, no revorb.

Ogg is the output format on purpose. FMOD opens it through the same
`System::createSound` path the engine already uses for `.wav`, so a file
dropped into `Client/Bin/Resources/Sound/...` plays with no code change, and
every desktop player opens it for auditioning candidates. Pass `--wav` to also
write 16-bit PCM, which is what the rest of the sound tree is.

  python render_events.py --package-root <.../ReleasePC/WwiseAudioPackage>
      --filter SOUND_UI --event ui_pc_inst_warrior_return
      --out Client/Bin/Resources/Sound/UI/SquareHole --name squarehole_song

An event that resolves to several media writes `<name>_<n>.ogg` -- those are the
equally weighted variations Wwise picks between. A single source writes
`<name>.ogg`. The `.wem` is kept beside it so a re-run does not need the
packages again. Without `--name` the event name is the stem.
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import wwise_audio_package as wwise          # noqa: E402
import wwise_vorbis_to_ogg as vorbis         # noqa: E402


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--package-root", type=Path, default=wwise.DEFAULT_PACKAGE_ROOT)
    ap.add_argument("--filter", default="", help="deobfuscated package name substring")
    ap.add_argument("--event", action="append", required=True, help="event name (repeatable)")
    ap.add_argument("--name", help="output stem for a single --event (default: the event name)")
    ap.add_argument("--out", type=Path, required=True)
    ap.add_argument("--keep-wem", action="store_true", default=True,
                    help="keep the extracted .wem beside the .ogg (default)")
    ap.add_argument("--codebooks", type=Path, default=vorbis.CODEBOOK_LIBRARY)
    ap.add_argument("--wav", action="store_true",
                    help="also write 16-bit PCM wav, decoded through the project's FMOD")
    a = ap.parse_args()

    decoder = vorbis.open_wav_decoder() if a.wav else None
    if a.wav and decoder is None:
        return 2

    packages = wwise.load_packages(wwise.find_packages(a.package_root, a.filter))
    if not packages:
        print("no package matches %r under %s" % (a.filter, a.package_root), file=sys.stderr)
        return 2
    objects = wwise.merged_objects(packages)
    a.out.mkdir(parents=True, exist_ok=True)

    failures = 0
    for event in a.event:
        try:
            sources, unresolved = wwise.resolve_event(event, objects)
        except KeyError:
            print("%s: no such event in the selected banks" % event, file=sys.stderr)
            failures += 1
            continue

        stem = a.name if (a.name and 1 == len(a.event)) else event
        converted = 0
        for index, media_id in enumerate(sources, start=1):
            payload = None
            for package in packages:
                entry = package.stream_by_id(media_id)
                if entry is not None:
                    payload = package.payload(entry)
                    break
            if payload is None:
                print("%s: media %s is not in the selected packages" % (event, media_id),
                      file=sys.stderr)
                failures += 1
                continue

            suffix = "" if 1 == len(sources) else "_%d" % index
            wem = a.out / ("%s%s.wem" % (stem, suffix))
            wem.write_bytes(payload)
            try:
                vorbis.convert(wem, wem.with_suffix(".ogg"), a.codebooks)
            except vorbis.WemError as error:
                print("%s: %s: %s" % (event, wem.name, error), file=sys.stderr)
                failures += 1
                continue
            if decoder is not None and not vorbis.write_wav(decoder, wem.with_suffix(".ogg")):
                failures += 1
            if not a.keep_wem:
                wem.unlink()
            converted += 1

        print("%s: %d source(s), %d ogg written, unresolved=%d -> %s"
              % (event, len(sources), converted, len(unresolved), a.out))
    if decoder is not None:
        decoder.close()
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
