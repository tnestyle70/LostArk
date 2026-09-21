"""Stage the vehicle rider clips against an installed WModel skeleton without Blender.

Same recipe as build_card_maze_player_animations.py: the source PSA files are
UModel exports of the class's base AnimSet (and its vehicle AnimSet where the
family keeps the newer mounts there), the installed skeleton and donor armature
scale are kept byte-for-byte, and body models are never rewritten.

Which clips each mount needs is read off Data/Actors/VehicleCatalog.json from
the reference class rows (armature prefix stripped), so a class added here gets
exactly the idle/run and skill clips the catalog already binds for the others.
The section names carry the class armature prefix like the Blender-cooked sets
and are cut to the 39 characters a WModel section name holds.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/ModelAssetConverter"))
sys.path.insert(0, str(Path(__file__).resolve().parent))
import append_psa_clip_to_wmodel as codec
from build_card_maze_player_animations import carrier, sections, subset

# class -> (package, armature prefix written into the clip names)
SOURCES = {
    "GuardianKnight": ("PC_DL_00", "ddk"),
}
REFERENCE_CLASS = "WARLORD"
REFERENCE_PREFIX = "wgl_"
MODES = {6705: "Horse", 9370: "Swing", 7209: "Hoverboard",
         8302: "HeavywalkerBm9", 8906: "Tube", 9524: "Dragon2"}
NAME_LIMIT = 39


def psa_clip_names(psa):
    names = set()
    with psa.open("rb") as handle:
        for name, size, count, at in codec.psa_chunks(psa):
            if name != "ANIMINFO":
                continue
            handle.seek(at)
            raw = handle.read(size * count)
            names |= {raw[i * size:i * size + 64].split(b"\0")[0].decode("ascii", "ignore")
                      for i in range(count)}
    return names


def reference_clips(vehicle):
    clips = []
    for rider in vehicle.get("riders", []):
        if rider["characterClass"] == REFERENCE_CLASS:
            clips += [rider["idleClip"], rider["runClip"]]
    for skill in vehicle.get("skills", []):
        for rider in skill.get("riders", []):
            if rider["characterClass"] == REFERENCE_CLASS:
                clips += rider["clips"]
    seen = []
    for clip in clips:
        if not clip.startswith(REFERENCE_PREFIX):
            raise SystemExit("Reference clip without armature prefix: " + clip)
        native = clip[len(REFERENCE_PREFIX):]
        if native not in seen:
            seen.append(native)
    return seen


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--classes", nargs="*", default=[])
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    wanted = set(args.classes) if args.classes else set(SOURCES)
    unknown = wanted - set(SOURCES)
    if unknown:
        raise SystemExit("Unknown class: %s" % ", ".join(sorted(unknown)))
    catalog = json.loads((ROOT / "Data/Actors/CharacterCatalog.json").read_text(encoding="utf-8"))
    vehicles = json.loads((ROOT / "Data/Actors/VehicleCatalog.json").read_text(encoding="utf-8"))["vehicles"]

    receipt = []
    for actor in catalog["characters"]:
        name = actor["assetId"]
        if name not in wanted:
            continue
        package, prefix = SOURCES[name]
        psas = [args.source / package / "AnimSet" / (package.lower() + "_ani.psa"),
                args.source / package / "AnimSet" / (package.lower() + "_vehicle_ani.psa")]
        psas = [p for p in psas if p.is_file()]
        if not psas:
            raise SystemExit("Missing AnimSet export under %s" % (args.source / package))
        available = {p: psa_clip_names(p) for p in psas}
        body = ROOT / "Client/Bin/Resources" / actor["bodyModel"]
        body_data = body.read_bytes()
        _, rows = sections(body_data)
        skeleton = next(s for s in rows if s[0] == 3)
        donor = next(s for s in rows if s[0] == 4)

        for vehicle in vehicles:
            mode = MODES.get(vehicle["vehicleId"])
            if mode is None:
                continue
            references = reference_clips(vehicle)
            if not references:
                continue
            # Every clip of one mount has to come from a single export; the
            # first psa that holds all of them wins. A reference name that the
            # 39-character section limit already cut is matched back to the one
            # native clip it is a prefix of.
            psa, natives = None, None
            for candidate in psas:
                resolved = []
                for reference in references:
                    if reference in available[candidate]:
                        resolved.append(reference)
                        continue
                    if len(REFERENCE_PREFIX + reference) < NAME_LIMIT:
                        break
                    longer = sorted(c for c in available[candidate] if c.startswith(reference))
                    if len(longer) != 1:
                        break
                    resolved.append(longer[0])
                if len(resolved) == len(references):
                    psa, natives = candidate, resolved
                    break
            if psa is None:
                missing = {p.name: sorted(set(references) - available[p]) for p in psas}
                raise SystemExit("%s %s: clips missing from every export: %s" % (name, mode, missing))
            staged = args.out / ("%s_Ride%s_donor.wmodel" % (name, mode))
            staged.write_bytes(carrier(body_data, skeleton, donor))
            outputs = {}
            for native in natives:
                bones, info, keys = codec.load_clip(psa, native)
                if len(set(bones)) != len(bones):
                    raise ValueError("Invalid native clip skeleton: " + native)
                section = (prefix + "_" + native)[:NAME_LIMIT]
                destination = args.out / ("%s_Ride%s_%s.wmodel" % (name, mode, native))
                subprocess.run([sys.executable, str(Path(codec.__file__)),
                                "--wmodel", str(staged), "--psa", str(psa),
                                "--clip", native, "--name", section,
                                "--out", str(destination)], check=True)
                staged = destination
                outputs[section] = {"native": native, "frames": info["frames"], "rate": info["rate"]}
            data = staged.read_bytes()
            _, staged_rows = sections(data)
            selected = [s for s in staged_rows if s[0] in (1, 2, 3) or
                        s[0] == 4 and s[4].split(b"\0")[0].decode() in outputs]
            final = args.out / ("%s_Ride%sAnimSet.wmodel" % (name, mode))
            final.write_bytes(subset(data, selected))
            actual = final.read_bytes()
            _, result_rows = sections(actual)
            target_skeleton = next(s for s in result_rows if s[0] == 3)
            assert actual[16 + target_skeleton[2]:16 + target_skeleton[2] + target_skeleton[3]] == \
                body_data[16 + skeleton[2]:16 + skeleton[2] + skeleton[3]]
            receipt.append({"class": name, "vehicleId": vehicle["vehicleId"], "mode": mode,
                            "sourcePsa": str(psa),
                            "sourceSha256": hashlib.sha256(psa.read_bytes()).hexdigest(),
                            "bodySha256": hashlib.sha256(body_data).hexdigest(),
                            "output": str(final),
                            "outputSha256": hashlib.sha256(actual).hexdigest(),
                            "clips": outputs, "skeletonPreserved": True})
            print("%s Ride%s <- %s: %s" % (name, mode, psa.name, ", ".join(outputs)))
    receipt_path = args.out / "receipt.json"
    existing = json.loads(receipt_path.read_text(encoding="utf-8")) if receipt_path.is_file() else []
    keep = {(r["class"], r["mode"]) for r in receipt}
    merged = [row for row in existing if (row["class"], row["mode"]) not in keep] + receipt
    merged.sort(key=lambda row: (row["class"], row["vehicleId"]))
    receipt_path.write_text(json.dumps(merged, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()
