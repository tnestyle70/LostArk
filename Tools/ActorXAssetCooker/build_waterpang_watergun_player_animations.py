"""Stage the Waterpang water-gun player clips against each installed WModel skeleton.

Source: every class family AnimSet carries the same prop clips for the Waterpang
water gun (prop ITR_02164, CommonAction 54015 "rifle carry"; GADGET.loa actions
569x2/570x2 "[Maharaka]" use pr_itr_02164_att_1..6_01). The clips are renamed to
class-neutral names so one binding contract serves all seven classes.

The body skeleton is copied byte-for-byte into each set (Attach_AnimationSet
requires the same skeleton hash); the carrier mesh is never rendered. Body
models are never rewritten. Installation is a separate, hash-checked step.
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
from build_card_maze_player_animations import SOURCES, carrier, sections, subset

CLIPS = {
    "watergun_idle": "pr_itr_02164_idle_1",
    "watergun_run": "pr_itr_02164_run_1",
    "watergun_start": "pr_itr_02164_start_1",
    "watergun_end": "pr_itr_02164_end_1",
    "watergun_att_1": "pr_itr_02164_att_1_01",
    "watergun_att_2": "pr_itr_02164_att_2_01",
    "watergun_att_3": "pr_itr_02164_att_3_01",
    "watergun_att_4": "pr_itr_02164_att_4_01",
    "watergun_att_5": "pr_itr_02164_att_5_01",
    "watergun_att_6": "pr_itr_02164_att_6_01",
}


def find_psa(roots, package):
    for root in roots:
        candidate = root / package / "AnimSet" / (package.lower() + "_ani.psa")
        if candidate.is_file():
            return candidate
    raise SystemExit("Missing PSA for %s under %s" % (package, ", ".join(map(str, roots))))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, action="append", required=True,
                        help="UModel ActorX export root(s) holding <PKG>/AnimSet/<pkg>_ani.psa")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--classes", nargs="*", default=[])
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    catalog = json.loads((ROOT / "Data/Actors/CharacterCatalog.json").read_text(encoding="utf-8"))
    wanted = set(args.classes) if args.classes else set(SOURCES)
    unknown = wanted - set(SOURCES)
    if unknown:
        raise SystemExit("Unknown class: %s" % ", ".join(sorted(unknown)))
    receipt = []
    for actor in catalog["characters"]:
        name = actor["assetId"]
        if name not in wanted:
            continue
        psa = find_psa(args.source, SOURCES[name])
        body = ROOT / "Client/Bin/Resources" / actor["bodyModel"]
        body_data = body.read_bytes()
        _, rows = sections(body_data)
        skeleton = next(s for s in rows if s[0] == 3)
        donor = next(s for s in rows if s[0] == 4)
        staged = args.out / (name + "_watergun_donor.wmodel")
        staged.write_bytes(carrier(body_data, skeleton, donor))
        clip_rows = []
        for output_name, native_name in CLIPS.items():
            bones, info, _ = codec.load_clip(psa, native_name)
            if len(set(bones)) != len(bones) or info["rate"] != 30:
                raise ValueError("Invalid native clip skeleton or rate: %s %s" % (name, native_name))
            destination = args.out / (name + "_" + output_name + ".wmodel")
            subprocess.run([sys.executable, str(Path(codec.__file__)),
                            "--wmodel", str(staged), "--psa", str(psa),
                            "--clip", native_name, "--name", output_name,
                            "--out", str(destination)], check=True)
            staged = destination
            clip_rows.append({"name": output_name, "source": native_name, "frames": info.get("frames")})
        data = staged.read_bytes()
        _, rows = sections(data)
        selected = [s for s in rows if s[0] in (1, 2, 3) or
                    s[0] == 4 and s[4].split(b"\0")[0].decode() in CLIPS]
        final = args.out / (name + "_WaterGunAnimSet.wmodel")
        final.write_bytes(subset(data, selected))
        actual = final.read_bytes()
        _, result_rows = sections(actual)
        target_skeleton = next(s for s in result_rows if s[0] == 3)
        # The complete source skeleton, including its hash, is unchanged.
        assert actual[16 + target_skeleton[2]:16 + target_skeleton[2] + target_skeleton[3]] == \
            body_data[16 + skeleton[2]:16 + skeleton[2] + skeleton[3]]
        receipt.append({"class": name, "sourcePackage": SOURCES[name], "sourcePsa": str(psa),
                        "sourceSha256": hashlib.sha256(psa.read_bytes()).hexdigest(),
                        "bodySha256": hashlib.sha256(body_data).hexdigest(),
                        "output": str(final), "outputSha256": hashlib.sha256(actual).hexdigest(),
                        "clips": clip_rows, "skeletonPreserved": True})
    (args.out / "receipt.json").write_text(json.dumps(receipt, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()
