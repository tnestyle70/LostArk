#!/usr/bin/env python3
"""Write the team distribution pair for the Bern3 ship resources (list .txt + copy .ps1) in the repo root.

The resource tree under Client/Bin/Resources is not tracked by Git, so the new files travel by list
(same convention as Resource_Distribution_2026-09-24_KoukuStage1Bgm.txt). The copy script is for the
receiving teammates; this tool never runs it.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
RES = REPO / "Client" / "Bin" / "Resources"
DATE = "2026-09-25"
NAME = "Bern3Ship"

GROUPS = [
    ("Character/Vehicle", "Ship_*", "배 모델 9종(.wmodel과 textures)"),
    ("Character/NPC", "Npc_MN_RHKP_02_2", "조선공 NPC 모델(원작 EFTable_Npc 19991, 머리+몸 합성)"),
    ("Character/NPC", "Npc_NP_LRKK_01", "여객선 NPC 모델(원작 NP_LRKK_01, 머리+몸 합성)"),
    ("UI/Vehicle/Icons", "ship_*.png", "배 아이콘 9개(원작 Voyage_Ship_1_N 실루엣)"),
]


def main() -> None:
    files = []
    for folder, pattern, _ in GROUPS:
        for path in sorted((RES / folder).glob(pattern)):
            if path.is_dir():
                files += sorted(p for p in path.rglob("*") if p.is_file())
            else:
                files.append(path)
    relative = [str(p.relative_to(RES)).replace("\\", "/") for p in files]
    total = sum(p.stat().st_size for p in files)
    digest = hashlib.sha256("\n".join(f"{r}:{p.stat().st_size}" for r, p in zip(relative, files)).encode()).hexdigest()[:16]

    lines = [
        f"Bern3 배 탑승 리소스 배포 목록 ({DATE}) - Area LV_BER_BERNCASTLE",
        "대상: Client/Bin/Resources 아래 상대 경로. 이 폴더는 Git 비추적이라 파일로 전달한다.",
        f"새 파일 {len(files)}개, 합계 약 {total / 1048576:.1f} MB. 기존 파일은 바뀌지 않는다(복사 스크립트는 같은 이름을 덮어쓰므로 새 경로에만 쓴다).",
    ]
    for folder, pattern, description in GROUPS:
        lines.append(f"- {folder}/{pattern}: {description}")
    lines += [
        "이 파일 목록은 같은 변경의 Client/Server 코드, Data(VehicleCatalog·VehicleProfiles·VehicleUiCatalog·NpcCatalog·Gameplay.world.json)와 게시본(Vehicles.bootstrap·BERN.worldbootstrap)과 함께 받아야 쓸모가 있다.",
        f"목록 확인용 지문(경로:크기 sha256 앞 16자리): {digest}",
        "",
    ] + relative
    (REPO / f"Resource_Distribution_{DATE}_{NAME}.txt").write_text("\r\n".join(lines) + "\r\n", encoding="utf-8")

    script = [
        f"# Bern3 배 탑승 리소스 복사 스크립트 ({DATE}) - Area LV_BER_BERNCASTLE",
        "# 사용: -Source 에 받은 Resources 폴더, -Destination 에 본인 Client/Bin/Resources",
        "param(",
        "    [Parameter(Mandatory = $true)][string]$Source,",
        "    [Parameter(Mandatory = $true)][string]$Destination",
        ")",
        "$ErrorActionPreference = 'Stop'",
        "$files = @(",
        ",\r\n".join(f"    '{r}'" for r in relative).replace("\\r\\n", "\r\n"),
        ")",
        "foreach ($relative in $files) {",
        "    $from = Join-Path $Source $relative",
        "    $to = Join-Path $Destination $relative",
        "    if (-not (Test-Path -LiteralPath $from)) { throw \"missing source file: $relative\" }",
        "    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $to) | Out-Null",
        "    Copy-Item -LiteralPath $from -Destination $to -Force",
        "}",
        "Write-Output (\"copied {0} files\" -f $files.Count)",
    ]
    text = "\r\n".join(script) + "\r\n"
    (REPO / f"Copy_ResourceDistribution_{DATE}_{NAME}.ps1").write_bytes(b"\xef\xbb\xbf" + text.encode("utf-8"))
    print("files", len(files), "MB", round(total / 1048576, 1), "fingerprint", digest)


if __name__ == "__main__":
    main()
