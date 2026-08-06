#!/usr/bin/env python3
"""Turn recovered Valtan action graphs into a `.clipseq` the animation tool reads.

`extract_valtan_actions.py` recovers every `CEFActionObject` with its stages and
the clip each stage plays.  This builds the playback chains out of that graph:

    start at the first stage, follow the stages in file order, and stop after
    the first stage that has no transition notify

The stop rule is the one piece of decoded evidence about where a chain ends -
in Valtan's axe combo the stage playing `Att_Battle_2_03` carries no
`MonsterMoveNextStage`, and that clip is the last of the combo the client
already hard-codes.  Advancing "to the next stage in file order" is still the
assumption recorded in the extractor's `contract.orderAssumption`.

Only chains whose clips are all present in the cooked model are emitted; the
receipt lists what was dropped and why, so a chain that needs clips we never
extracted fails loudly instead of producing a file the runtime cannot play.

Action names are NOT invented.  The Action `.loa` names only the ten NPC state
actions (STAND, MOVE, DIE, ...); combat actions carry no name, so the label is
built from the action index and the clip group it actually plays.  The id column
is the extractor's action index, not a game action id - no such id was found in
the source.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import json
import os
import re
import tempfile
from pathlib import Path
from typing import Any, Iterable


class ClipSeqBuildError(RuntimeError):
    pass


CLIP_SEQ_MAGIC = "LOSTARK_CLIP_SEQ"
CLIP_SEQ_VERSION = 2

# `section type=4 index=N size=N name=<clip>` in ModelAssetConverter info output.
MODEL_INFO_CLIP = re.compile(
    r"^\s*section\s+type=4\b.*\bname=(?P<name>\S+)\s*$", re.MULTILINE
)

ATT_GROUP = re.compile(r"^(att_battle_\d+)", re.IGNORECASE)
LOCOMOTION_PREFIXES = ("idle_", "run_", "walk_", "turn_", "fast_")


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def atomic_write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle, temporary_name = tempfile.mkstemp(
        prefix=path.name + ".", suffix=".tmp", dir=path.parent
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(handle, "w", encoding="utf-8", newline="\n") as output:
            output.write(text)
            output.flush()
            os.fsync(output.fileno())
        os.replace(temporary, path)
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise


def read_available_clips(model_info: Path) -> list[str]:
    text = model_info.read_text(encoding="utf-8", errors="replace")
    clips = [match.group("name") for match in MODEL_INFO_CLIP.finditer(text)]
    if not clips:
        raise ClipSeqBuildError(
            f"no 'section type=4 ... name=' animation rows in {model_info}"
        )
    return clips


def build_chain(action: dict[str, Any]) -> list[str]:
    """Clips from the first stage up to and including the first terminal stage."""
    chain: list[str] = []
    for stage in action["stages"]:
        for clip in stage["clips"]:
            # Consecutive repeats are alternate routes through the same clip.
            if not chain or chain[-1].casefold() != clip.casefold():
                chain.append(clip)
        if not stage["transitions"]:
            break
    return chain


def clip_group(chain: Iterable[str]) -> str:
    groups = [
        match.group(1).lower()
        for match in (ATT_GROUP.match(clip) for clip in chain)
        if match is not None
    ]
    if not groups:
        return "locomotion"
    return collections.Counter(groups).most_common(1)[0][0]


def is_attack_clip(clip: str) -> bool:
    return not clip.casefold().startswith(LOCOMOTION_PREFIXES)


def quoted(value: str) -> str:
    if '"' in value:
        raise ClipSeqBuildError(f"value contains a quote: {value!r}")
    return '"' + value + '"'


def build(
    report: dict[str, Any],
    available: list[str],
    mode: str,
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    lookup = {clip.casefold(): clip for clip in available}
    if len(lookup) != len(available):
        raise ClipSeqBuildError("cooked model lists a clip name twice")

    emitted: list[dict[str, Any]] = []
    dropped: list[dict[str, Any]] = []
    for action in report["actions"]:
        chain = build_chain(action)
        if not chain:
            continue
        if not any(is_attack_clip(clip) for clip in chain):
            dropped.append(
                {
                    "actionIndex": action["index"],
                    "chain": chain,
                    "reason": "chain has no attack clip",
                }
            )
            continue

        missing = [clip for clip in chain if clip.casefold() not in lookup]
        if missing:
            dropped.append(
                {
                    "actionIndex": action["index"],
                    "chain": chain,
                    "reason": "clips absent from the cooked model",
                    "missingClips": sorted(set(missing)),
                }
            )
            continue

        # Emit the model's own spelling so the runtime lookup matches exactly.
        resolved = [lookup[clip.casefold()] for clip in chain]
        emitted.append(
            {
                "actionIndex": action["index"],
                "group": clip_group(chain),
                "mode": mode,
                "clips": resolved,
                "stageCount": action["stageCount"],
                "soundFamilies": action["soundFamilies"],
            }
        )

    # Identical chains appear once per route through the graph; keep the first.
    unique: list[dict[str, Any]] = []
    seen: set[tuple[str, ...]] = set()
    for entry in emitted:
        key = tuple(clip.casefold() for clip in entry["clips"])
        if key in seen:
            dropped.append(
                {
                    "actionIndex": entry["actionIndex"],
                    "chain": entry["clips"],
                    "reason": "duplicate of an earlier chain",
                }
            )
            continue
        seen.add(key)
        unique.append(entry)
    return unique, dropped


def render(owner: str, chains: list[dict[str, Any]]) -> str:
    lines = [f"{CLIP_SEQ_MAGIC} {CLIP_SEQ_VERSION} {quoted(owner)} {len(chains)}"]
    for chain in chains:
        label = f"{owner} Action {chain['actionIndex']} ({chain['group']})"
        lines.append(
            " ".join(
                (
                    str(chain["actionIndex"]),
                    quoted(label),
                    "seq=0",
                    f"mode={chain['mode']}",
                    "clips=" + quoted(",".join(chain["clips"])),
                )
            )
        )
    return "\n".join(lines) + "\n"


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build a Valtan .clipseq from recovered action graphs"
    )
    parser.add_argument("--actions", type=Path, required=True)
    parser.add_argument(
        "--model-info",
        type=Path,
        required=True,
        help="text captured from ModelAssetConverter.exe info <model>.wmodel",
    )
    parser.add_argument("--owner", default="Valtan")
    parser.add_argument("--mode", default="SEQUENCE")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--receipt", type=Path)
    parser.add_argument("--expect-chains", type=int)
    parser.add_argument(
        "--require-chain",
        action="append",
        default=[],
        help="comma separated clip chain that must be emitted",
    )
    return parser.parse_args(list(argv) if argv is not None else None)


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(argv)
    if args.mode != args.mode.upper() or not args.mode.isalpha():
        raise ClipSeqBuildError(f"mode must be an uppercase word: {args.mode!r}")

    report = json.loads(args.actions.read_text(encoding="utf-8"))
    if report.get("schemaVersion") != 2:
        raise ClipSeqBuildError("actions report schemaVersion 2 is required")

    available = read_available_clips(args.model_info)
    chains, dropped = build(report, available, args.mode)

    if args.expect_chains is not None and len(chains) != args.expect_chains:
        raise ClipSeqBuildError(
            f"chain count {len(chains)} != expected {args.expect_chains}"
        )

    emitted_keys = {
        tuple(clip.casefold() for clip in chain["clips"]) for chain in chains
    }
    for requirement in args.require_chain:
        wanted = tuple(
            part.strip().casefold()
            for part in requirement.split(",")
            if part.strip()
        )
        if wanted not in emitted_keys:
            raise ClipSeqBuildError(f"required chain was not emitted: {requirement}")

    text = render(args.owner, chains)
    atomic_write_text(args.output, text)

    receipt = {
        "schemaVersion": 1,
        "owner": args.owner,
        "mode": args.mode,
        "inputs": {
            "actions": {
                "path": args.actions.as_posix(),
                "sha256": sha256_file(args.actions),
                "sourceLoa": report["source"]["path"],
                "sourceSha256": report["source"]["sha256"],
            },
            "modelInfo": {
                "path": args.model_info.as_posix(),
                "sha256": sha256_file(args.model_info),
                "clipCount": len(available),
            },
        },
        "contract": {
            "chainEnd": "first stage carrying no transition notify",
            "orderAssumption": report["contract"]["orderAssumption"],
            "idColumn": (
                "extractor action index; the source names no game action id"
            ),
            "labels": (
                "built from the action index and the clip group it plays; the "
                "source names only the ten NPC state actions"
            ),
        },
        "output": {
            "path": args.output.as_posix(),
            "sha256": sha256_bytes(text.encode("utf-8")),
            "chainCount": len(chains),
        },
        "chains": chains,
        "dropped": dropped,
    }
    if args.receipt is not None:
        atomic_write_text(
            args.receipt, json.dumps(receipt, ensure_ascii=False, indent=1) + "\n"
        )

    print(f"emitted {len(chains)} chains, dropped {len(dropped)}")
    for chain in chains:
        print(f"  {chain['actionIndex']:4d}  {chain['group']:16s} "
              f"{' -> '.join(chain['clips'])}")
    print(f"written: {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
