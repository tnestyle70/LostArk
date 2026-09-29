#!/usr/bin/env python3
"""Write a Scaleform movie (EFSwfMovie export) out of a retail .upk package as raw bytes.

UModel exports the textures of an EFUI_* package but not its EFSwfMovie, and ffdec is not needed
for what the HUD builders lift, so this reads the export straight from the package with the same
reader the map placement extractor uses. The export's serial data is a short UE3 header followed
by the movie itself; the movie starts at the first ``GFX`` (or ``FWS``/``CFX``/``CWS``) signature.

  python dump_upk_movie.py <package.upk> <movie object name> <out dir> [--packages <Packages dir>]

Writes ``<out dir>/<movie>.export.bin`` (the whole export) and ``<out dir>/<movie>.gfx`` (the
movie only). Feed the .gfx to ``gfx_native_parse.py`` / ``gfx_native_tree.py``.

To find which package holds a movie, list the objects of every package that has an EFSwfMovie
export (the ``read_package_summary`` + ``parse_export_table`` pair below does it; a whole-client
sweep of ~500 packages takes a few seconds). The package's texture pages come from UModel:

  umodel_lostark_v7.exe -export -game=lostark -kr -nameresolve -path=<Packages> -out=<dir> EFUI_<NAME>
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / "Tools/LevelPlacementExtractor"))
import extract_ue3_placements as x  # noqa: E402

DEFAULT_PACKAGES = Path(r"C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages")
SIGNATURES = (b"GFX", b"CFX", b"FWS", b"CWS")


def dump(package: Path, movie: str, out_dir: Path) -> tuple[Path, Path]:
    summary = x.read_package_summary(package)
    reader = x.LostArkPackageRangeReader(package, summary)
    end = min(max(summary.header_size, summary.depends_offset) + 64, reader.logical_size)
    logical = reader.read_logical_range(0, end)
    names = x.parse_name_table(logical, summary)
    imports = x.parse_import_table(logical, summary, names)
    exports = x.parse_export_table(logical, summary, names)
    for export in exports:
        class_name = x.package_ref_name(export.class_index, imports, exports)
        if not (class_name and class_name.lower() == "efswfmovie" and export.object_name == movie):
            continue
        data = reader.read_logical_range(export.serial_offset, export.serial_size)
        starts = [data.find(sig) for sig in SIGNATURES if data.find(sig) >= 0]
        if not starts:
            raise SystemExit("no movie signature inside the %s export" % movie)
        start = min(starts)
        out_dir.mkdir(parents=True, exist_ok=True)
        whole = out_dir / (movie + ".export.bin")
        gfx = out_dir / (movie + ".gfx")
        whole.write_bytes(data)
        gfx.write_bytes(data[start:])
        return whole, gfx
    raise SystemExit("EFSwfMovie %s not found in %s" % (movie, package.name))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("package", help="package file name, e.g. OVSG0AMAOWF9SHDR2YW8WM.upk")
    parser.add_argument("movie")
    parser.add_argument("out_dir", type=Path)
    parser.add_argument("--packages", type=Path, default=DEFAULT_PACKAGES)
    args = parser.parse_args()
    whole, gfx = dump(args.packages / args.package, args.movie, args.out_dir)
    print("wrote", whole, gfx)
    return 0


if __name__ == "__main__":
    sys.exit(main())
