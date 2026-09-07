# 쿠크 카드미로 외곽 카드 이동 중 깜빡임 수정

## 후속 영상에 따른 판정 정정

`12222.mp4`와 실제 CARD02 geometry/placement/material 대조 결과, PS의 무늬 카드와
SL03의 회색 카드가 동일 평면에서 겹치는 Z-fighting 조건을 확인했다. 아래 G1은 이미 수행한
밉 체인 보완의 구현 명세일 뿐 사용자 깜빡임의 원인·해결 증거가 아니다. 상세 실측과
원본 visibility state는 대응 RESULT의 재조사 절을 따른다.

## G2. 원본 숨김 상태 복구

원본 `LV_LUT_MIDNIGHTC_ED_PS` UPK의 CARD02 actor export 633~668은 소유 actor가
`bHiddenEdCustom=true`, `layer=lv_nav`이고 연결 component가 `HiddenGame=true`다. 반면
같은 위치를 담당하는 `LV_LUT_MIDNIGHTC_ED_SL03` CARD02 component export 1087~1122에는
`HiddenGame`이 없다. 추출 과정에서 이 표시 상태가 authoring placement에 보존되지 않아 두
세트가 모두 visible=1로 들어온 것이 실제 결함이다.

`Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.mapplacements`에서
PS export 633~668의 안정적인 placement ID, asset ID와 transform은 그대로 두고 마지막
visible 필드만 1에서 0으로 바꾼다. SL03 카드, 다른 카드, navigation, gameplay와 C++은
변경하지 않는다. 변경 후 Map publisher `Validate`로 입력 계약을 검사하고, 해당 Area만
`Publish`하여 runtime placement를 원자적으로 교체한다. authoring/runtime 양쪽에서 대상
36개가 visible=0, SL03 대응 36개가 visible=1인지 확인한다. 최종 화면 판정은 사용자가 같은
카메라 회전으로 확인한다.

## G1. 카드 diffuse DDS 밉 체인 복구

사용자 영상의 카드미로 외곽을 15fps로 분해해 확인했다. 카메라가 멈춘 마지막 구간에서는
카드 placement가 사라지지 않으며, 이동 중 비스듬한 카드의 고주파 무늬와 회색 뒷면 경계만
프레임마다 달라진다. 카드 배치의 같은 위치·회전 60쌍은 중복 placement가 아니라 같은 모델을
위·아래로 잇는 원본 Y 반전 구조이므로 삭제하지 않는다. 큰 카드 모델의 앞·뒤 mesh도 서로 다른
면이며 배경 plane과 수 m 떨어져 있어 투명 혼합이나 동일 평면 중복으로 처리하지 않는다.

실제 런타임 카드 diffuse DDS는 1024x1024 또는 512x512 DXT1이지만 모두 mipMapCount 0이며
파일 크기도 base level 한 장과 정확히 일치한다. `CMaterial`은 DDS에 저장된 mip을 그대로 읽고
맵 shader는 `MIN_MAG_MIP_LINEAR`를 사용하므로, 입력 DDS에 밉 체인을 복구하면 기존 렌더 경로가
추가 런타임 분기 없이 올바른 축소 레벨을 선택한다.

`Tools/MapPipeline/Build-DdsMipChain.py`를 새로 추가한다. 이 도구는 지정한 root 안에서 정규식과
일치하는 DDS만 수집하고, legacy DXT1·2D·단일 base level인지 먼저 검증한다. base DXT1 payload는
한 바이트도 재압축하지 않고 보존한다. Pillow로 읽은 base 이미지를 각 절반 크기로 Lanczos
축소해 DXT1 mip payload만 생성하고, 임시 파일을 완성·재검사한 뒤 `os.replace`로 교체한다.
기존 밉이 있는 파일, 다른 압축 형식, 잘린 payload는 변경하지 않고 실패한다. 기본은 dry-run이며
`--write`가 있을 때만 쓴다.

적용 대상은
`Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED` 아래 파일명에
`koukusaton_card..._d`가 들어가는 카드 diffuse DDS다. normal/specular/grass와 다른 Area는
건드리지 않는다. 적용 전 원본 80개는 작업 백업 폴더에 상대 경로 그대로 복사한다.
authoring placement, mapassets, render profile, shader, C++ 및 Server 데이터는 변경하지 않는다.

검증은 dry-run 대상 수와 실제 write 수 일치, 모든 대상의 DXT1/base payload SHA-256 보존,
1024 텍스처 11단계·512 텍스처 10단계, 각 레벨 payload 크기와 전체 DDS 크기 검사,
Pillow 재로드, 두 번째 실행 no-op, Python compile과 `git diff --check`로 한다. Client/UI는
자율 실행하지 않는다. 사용자는 최신 리소스로 재시작한 뒤 Lobby → Test → MapTool 또는
KoukuSaydon → F1 → 카드미로에서 같은 카메라 이동으로 최종 시각 확인한다.

### 새 파일 전체 코드: `Tools/MapPipeline/Build-DdsMipChain.py`

```python
from __future__ import annotations

import argparse
import hashlib
import io
import os
from pathlib import Path
import re
import struct
import sys

from PIL import Image


DDS_HEADER_BYTES = 128
DDSD_MIPMAPCOUNT = 0x00020000
DDSCAPS_COMPLEX = 0x00000008
DDSCAPS_MIPMAP = 0x00400000


def read_u32(data: bytes | bytearray, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def write_u32(data: bytearray, offset: int, value: int) -> None:
    struct.pack_into("<I", data, offset, value)


def dxt1_payload_size(width: int, height: int) -> int:
    return max(1, (width + 3) // 4) * max(1, (height + 3) // 4) * 8


def dimensions(width: int, height: int) -> list[tuple[int, int]]:
    result = [(width, height)]
    while width > 1 or height > 1:
        width = max(1, width // 2)
        height = max(1, height // 2)
        result.append((width, height))
    return result


def validate_source(path: Path, source: bytes) -> tuple[int, int, int]:
    if len(source) < DDS_HEADER_BYTES or source[:4] != b"DDS ":
        raise ValueError(f"{path}: not a legacy DDS file")
    if read_u32(source, 4) != 124 or read_u32(source, 76) != 32:
        raise ValueError(f"{path}: unsupported DDS header")
    if source[84:88] != b"DXT1":
        raise ValueError(f"{path}: only DXT1 is supported")

    width = read_u32(source, 16)
    height = read_u32(source, 12)
    mip_count = read_u32(source, 28)
    if width == 0 or height == 0:
        raise ValueError(f"{path}: invalid dimensions {width}x{height}")
    if mip_count > 1:
        levels = dimensions(width, height)
        if mip_count != len(levels):
            raise ValueError(
                f"{path}: partial mip chain {mip_count}, expected {len(levels)}"
            )
        flags = read_u32(source, 8)
        caps = read_u32(source, 108)
        if 0 == flags & DDSD_MIPMAPCOUNT or (
            caps & (DDSCAPS_COMPLEX | DDSCAPS_MIPMAP)
        ) != (DDSCAPS_COMPLEX | DDSCAPS_MIPMAP):
            raise ValueError(f"{path}: mip count and DDS header flags disagree")
        expected = DDS_HEADER_BYTES + sum(
            dxt1_payload_size(mip_width, mip_height)
            for mip_width, mip_height in levels
        )
        if len(source) != expected:
            raise ValueError(
                f"{path}: complete mip payload should be {expected} bytes, got {len(source)}"
            )
        return width, height, mip_count

    expected = DDS_HEADER_BYTES + dxt1_payload_size(width, height)
    if len(source) != expected:
        raise ValueError(
            f"{path}: expected one DXT1 base payload ({expected} bytes), got {len(source)}"
        )
    return width, height, mip_count


def encode_dxt1(image: Image.Image) -> bytes:
    output = io.BytesIO()
    image.save(output, format="DDS", pixel_format="DXT1")
    encoded = output.getvalue()
    if len(encoded) < DDS_HEADER_BYTES or encoded[84:88] != b"DXT1":
        raise ValueError("Pillow did not produce a legacy DXT1 DDS")
    return encoded[DDS_HEADER_BYTES:]


def build_mipped_dds(path: Path, source: bytes) -> tuple[bytes, int, str]:
    width, height, existing_mips = validate_source(path, source)
    if existing_mips > 1:
        return source, existing_mips, "already-mipped"

    with Image.open(io.BytesIO(source)) as opened:
        base_image = opened.convert("RGB")
        base_image.load()

    levels = dimensions(width, height)
    payloads = [source[DDS_HEADER_BYTES:]]
    for mip_width, mip_height in levels[1:]:
        resized = base_image.resize(
            (mip_width, mip_height),
            Image.Resampling.LANCZOS,
            reducing_gap=3.0,
        )
        payload = encode_dxt1(resized)
        expected = dxt1_payload_size(mip_width, mip_height)
        if len(payload) != expected:
            raise ValueError(
                f"{path}: {mip_width}x{mip_height} mip is {len(payload)} bytes, expected {expected}"
            )
        payloads.append(payload)

    header = bytearray(source[:DDS_HEADER_BYTES])
    write_u32(header, 8, read_u32(header, 8) | DDSD_MIPMAPCOUNT)
    write_u32(header, 28, len(levels))
    write_u32(
        header,
        108,
        read_u32(header, 108) | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP,
    )
    result = bytes(header) + b"".join(payloads)
    expected_total = DDS_HEADER_BYTES + sum(
        dxt1_payload_size(mip_width, mip_height)
        for mip_width, mip_height in levels
    )
    if len(result) != expected_total:
        raise ValueError(
            f"{path}: output is {len(result)} bytes, expected {expected_total}"
        )
    return result, len(levels), "generated"


def replace_file(path: Path, content: bytes) -> None:
    temporary = path.with_name(path.name + ".miptmp")
    try:
        temporary.write_bytes(content)
        with Image.open(temporary) as verification:
            verification.load()
        os.replace(temporary, path)
    finally:
        if temporary.exists():
            temporary.unlink()


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build a complete mip chain for matching legacy DXT1 DDS files."
    )
    parser.add_argument("--root", required=True, type=Path)
    parser.add_argument("--include-regex", required=True)
    parser.add_argument("--write", action="store_true")
    args = parser.parse_args()

    root = args.root.resolve()
    if not root.is_dir():
        parser.error(f"root does not exist: {root}")
    pattern = re.compile(args.include_regex, re.IGNORECASE)
    paths = sorted(
        path for path in root.rglob("*.dds")
        if pattern.search(path.name)
    )
    if not paths:
        parser.error("no matching DDS files")

    generated = 0
    skipped = 0
    for path in paths:
        source = path.read_bytes()
        width, height, _ = validate_source(path, source)
        base_bytes = dxt1_payload_size(width, height)
        base_hash = hashlib.sha256(
            source[DDS_HEADER_BYTES:DDS_HEADER_BYTES + base_bytes]
        ).hexdigest()
        result, mip_count, state = build_mipped_dds(path, source)
        if state == "already-mipped":
            skipped += 1
        else:
            generated += 1
            if args.write:
                replace_file(path, result)
                written = path.read_bytes()
                written_hash = hashlib.sha256(
                    written[DDS_HEADER_BYTES:DDS_HEADER_BYTES + base_bytes]
                ).hexdigest()
                if written_hash != base_hash or read_u32(written, 28) != mip_count:
                    raise ValueError(f"{path}: post-write verification failed")
        print(
            f"{state}|mips={mip_count}|base_sha256={base_hash}|"
            f"bytes={len(result)}|{path.relative_to(root)}"
        )

    mode = "write" if args.write else "dry-run"
    print(f"summary|mode={mode}|matched={len(paths)}|generated={generated}|skipped={skipped}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, re.error, struct.error) as error:
        print(f"error: {error}", file=sys.stderr)
        raise SystemExit(1)
```
