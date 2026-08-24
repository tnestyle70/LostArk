# 캐릭터 애니메이션 SOUND 이벤트 실제 재생 파이프라인 — PLAN

## 0. 실측 요약

- `Data/Animation/Authored/<Class>/<Class>.animevents`는 텍스트 포맷(`LOSTARK_ANIM_EVENTS <ver> "<Asset>" <rows>` 헤더 +
  `"clip" TYPE startms=N [endms=N] payload="..." src=orig` 행)이며, JSON이 아니다.
- 6개 클래스 중 5개(Artist/DimensionMaster/LanceMaster/Slayer/Warlord/GunSlinger 중 실제 파일은
  Artist, DimensionMaster, GunSlinger, LanceMaster, Slayer, Warlord 전부 6개)에 `SOUND` 행이 이미 존재한다
  (`f26615af` 커밋, 플레이어 담당자 JS가 Animation Tool에 SOUND/EFFECT 저작 기능을 추가하며 저작).
  - 총 1741개 SOUND 행: LanceMaster 451, GunSlinger 356, Slayer 302, Warlord 229, DimensionMaster 240, Artist 163.
  - GunSlinger/Slayer는 사용자 확인상 "미구현이라 나중에 추가될 수도 있어서 우선 냅둔 것"이며 이번 파이프라인에서
    실패로 취급하지 않는다 — 해당 클래스는 매칭 실패해도 그냥 빈 목록으로 통과시킨다.
- `payload`는 파일 경로가 아니라 원작 사운드 이벤트 이름(`<BANK>.<이벤트명>`, 예
  `PC_COMMON_DUAL.PC_Common_Dual1_1`, `PC_LANCEMASTER_F.PC_LanceMaster_F_Att_Battle1_1`).
- `D:\로아 리소스\Sound\Character\{Common,Artist,DimensionMaster,LanceMaster,Warlord}\`에 실제 wav가 있고,
  파일명은 `<이벤트명(소문자)>__<숫자>.wav`이며 같은 이벤트에 배리에이션 파일이 여러 개 있을 수 있다
  (`pc_lancemaster_f_att_battle1_1__341217507.wav` / `..._391784688.wav` / `..._448986509.wav`).
  `Character/GunSlinger`, `Character/Slayer` 폴더는 없다(위와 같은 이유로 스킵 대상).
- `CAnimationEffectCueDocument::Load_FromText`(`Client/Private/AnimationEffectCueDocument.cpp:604`)가 이
  `.animevents`를 **직접** 파싱해 `ANIMATION_EFFECT_CUE_DOCUMENT`를 만들고, `CCharacter::Ready_EffectCueDocument`류
  경로(`Client/Private/Character.cpp:237-270`)가 Character 초기화 시 이걸 로드해 `m_EffectCueDocument`에 보관한다.
  지금은 `"HIT"`과 `"EFFECT"` 토큰만 처리하고 `"SOUND"`는 무시(그냥 `continue`) — 별도 발행(publish) 파이프라인이
  따로 있는 게 아니라 **이 파서가 곧 런타임 소비자**다.
- `CCharacter::Update_EffectCues()`(`Client/Private/Character.cpp:354-487`)가 매 프레임(`Character.cpp:1397`에서 호출)
  `m_EffectCueDocument.Cues`를 현재 재생 중인 clip과 대조해 wall-clock 상 정확한 트리거 시각을 계산하고
  (`CActionPresentationTimeline::Resolve_ClipDuration`/`Resolve_CueWallOffset`로 루프 epoch까지 처리)
  `CEffectPresentationService::Spawn`을 부른다. SOUND도 같은 시간 해석 로직을 그대로 재사용해야
  이펙트와 동일한 정확도로 맞물린다.
- `CGameInstance::Get().Play_Sound(경로, 볼륨)`은 이번 세션에 이미 실제로 쓰이는 걸 확인함(로비 BGM에 `Play_Music`도
  추가함). SOUND 큐는 1회성이므로 `Play_Sound`를 그대로 쓴다.

## 1. 이 작업의 목표와 종료 증거

**목표**: `.animevents`의 SOUND 행이 실제 Character 재생 중 정확한 타이밍에 실제 wav 파일을 재생한다.

**종료 증거**:
1. 신규 발행 스크립트가 6개 클래스 `.animevents`의 SOUND 행 전부를 파싱하고, `D:\로아 리소스\Sound\Character\...`에서
   대소문자 무시 접두사 매칭으로 실제 파일을 찾아 `Client/Bin/Resources/Sound/Character/<Class|Common>/`에 배포하고,
   `Data/Sound/CharacterSoundCatalog.json`에 `이벤트명 -> [Resources 상대 wav 경로...]` 매핑을 기록한다.
   GunSlinger/Slayer는 소스 폴더가 없어 매칭 0건이어도 실패가 아니라 카탈로그에 빈 배열로 기록한다.
2. `ANIMATION_EFFECT_CUE_DOCUMENT`가 `Sounds` 벡터를 갖고, `.animevents`의 SOUND 행이 여기 채워진다.
3. `CCharacter::Update_SoundCues()`가 `Update_EffectCues()`와 같은 wall-clock 해석으로 SOUND 큐 트리거 시각을 찾고
   그 시각에 카탈로그에서 이벤트명을 찾아 배리에이션 중 하나를 골라 `CGameInstance::Get().Play_Sound()`를 호출한다.
4. Debug 빌드 성공, LanceMaster/Warlord/Artist/DimensionMaster 중 하나로 Bern/Valtan 진입해 공격 스킬을 써서
   실제 타격음이 애니메이션과 맞물려 들리는지 사용자가 직접 확인(이 프로젝트의 화면·사운드 최종 판정은 사용자 전용).

## 2. 수정/신규 파일과 이유

| 파일 | 종류 | 이유 |
|---|---|---|
| `Tools/CharacterAnimationIntake/build_sound_catalog.py` | 신규 | `.animevents` SOUND 행 → 실제 wav 매칭 → Resources 배포 → 카탈로그 JSON 생성 |
| `Data/Sound/CharacterSoundCatalog.json` | 신규(생성물, git 추적) | 이벤트명 -> wav 경로 배열의 정본 카탈로그 |
| `Client/Bin/Resources/Sound/Character/**` | 신규(런타임 리소스, 팀장 관리 폴더 정책과 동일하게 취급) | 실제 재생될 wav |
| `Client/Public/AnimationEffectCueDocument.h` | 기존 수정 | `ANIMATION_SOUND_CUE` struct, `Sounds` 벡터 추가 |
| `Client/Private/AnimationEffectCueDocument.cpp` | 기존 수정 | `Load_FromText`에 `"SOUND"` 토큰 분기 추가 |
| `Client/Public/SoundCueCatalog.h` | 신규 | 카탈로그 JSON 로더 계약 |
| `Client/Private/SoundCueCatalog.cpp` | 신규 | 카탈로그 JSON 로드 + 이벤트명 -> 배리에이션 조회 |
| `Client/Public/Character.h` | 기존 수정 | `Update_SoundCues()` 선언 |
| `Client/Private/Character.cpp` | 기존 수정 | `Update_SoundCues()` 정의 + 호출부 추가 |
| `Client/Default/Client.vcxproj`, `.vcxproj.filters` | 기존 수정 | `SoundCueCatalog.h/.cpp` 등록 |

## 3. `Data/Sound/CharacterSoundCatalog.json` 스키마

```json
{
  "formatVersion": 1,
  "classes": {
    "LanceMaster": {
      "PC_LanceMaster_F_Att_Battle1_1": [
        "Sound/Character/LanceMaster/pc_lancemaster_f_att_battle1_1__341217507.wav",
        "Sound/Character/LanceMaster/pc_lancemaster_f_att_battle1_1__391784688.wav",
        "Sound/Character/LanceMaster/pc_lancemaster_f_att_battle1_1__448986509.wav"
      ]
    },
    "Common": {
      "PC_Common_Dual1_1": [
        "Sound/Character/Common/PC_Common_Dual1_1__707528264.wav"
      ]
    },
    "GunSlinger": {},
    "Slayer": {}
  }
}
```

- 이벤트명은 `payload`의 `.` 뒤 부분(뱅크명 제외) 그대로 키로 쓴다. 뱅크가 `PC_COMMON_*`이면 `Common`
  클래스 아래, 그 외에는 해당 클래스 아래 저장한다 — 같은 이벤트명이 클래스마다 달라질 수 있으므로
  클래스별로 분리한다(전역 단일 map으로 합치면 다른 클래스의 동명 이벤트를 덮어써 잘못된 배리에이션을
  재생할 수 있다).
- 값 배열이 비어 있으면(매칭 실패) 런타임은 그 이벤트를 조용히 스킵한다 — SOUND 재생 실패가 애니메이션
  재생이나 데미지 판정을 막지 않는다(Client 전용 표현 계층, Server authority 없음).

## 4. `Tools/CharacterAnimationIntake/build_sound_catalog.py` (신규, 전체 코드)

```python
#!/usr/bin/env python3
"""Scan every class .animevents for SOUND rows, resolve each payload event name
to real wav files under the raw extracted sound library, copy the matched files
into the Client runtime Resources folder, and emit Data/Sound/CharacterSoundCatalog.json.

No network/DB use. Pure filesystem read + deterministic copy + JSON write.
"""
from __future__ import annotations

import argparse
import json
import re
import shutil
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
ANIMEVENTS_DIR = REPO_ROOT / "Data" / "Animation" / "Authored"
CATALOG_PATH = REPO_ROOT / "Data" / "Sound" / "CharacterSoundCatalog.json"
RESOURCES_SOUND_ROOT = REPO_ROOT / "Client" / "Bin" / "Resources" / "Sound" / "Character"

# class folder name (as it appears in Data/Animation/Authored/<Class>) ->
# source wav folder name under the raw extracted library, and the Resources
# deploy subfolder name. Both happen to match the class folder name here.
CLASSES = [
    "Artist",
    "DimensionMaster",
    "GunSlinger",
    "LanceMaster",
    "Slayer",
    "Warlord",
]

SOUND_ROW_RE = re.compile(
    r'^"(?P<clip>[^"]+)"\s+SOUND\s+(?P<fields>.*?)\s*src=\S+\s*$'
)
PAYLOAD_RE = re.compile(r'payload="([^"]*)"')


def parse_sound_events(animevents_path: Path) -> list[str]:
    """Returns the ordered (possibly duplicated) list of payload strings for
    every SOUND row in one .animevents file. Duplicates are expected (the same
    cue can be authored on more than one clip) and collapsed by the caller."""
    if not animevents_path.is_file():
        return []
    payloads: list[str] = []
    text = animevents_path.read_text(encoding="utf-8")
    for line in text.splitlines():
        line = line.strip()
        if not line or " SOUND " not in line:
            continue
        match = SOUND_ROW_RE.match(line)
        if not match:
            continue
        payload_match = PAYLOAD_RE.search(match.group("fields"))
        if not payload_match or not payload_match.group(1):
            continue
        payloads.append(payload_match.group(1))
    return payloads


def split_bank_event(payload: str) -> tuple[str, str]:
    """'PC_COMMON_DUAL.PC_Common_Dual1_1' -> ('PC_COMMON_DUAL', 'PC_Common_Dual1_1')."""
    bank, _, event = payload.partition(".")
    return bank, event if event else bank


def resolve_source_folder(bank: str, class_name: str, raw_sound_root: Path) -> Path:
    """PC_COMMON_* payloads live under the shared Common folder; everything
    else is looked up under the owning class's own folder."""
    if bank.upper().startswith("PC_COMMON"):
        return raw_sound_root / "Character" / "Common"
    return raw_sound_root / "Character" / class_name


def find_variant_files(source_folder: Path, event_name: str) -> list[Path]:
    if not source_folder.is_dir():
        return []
    prefix = (event_name + "__").lower()
    matches = [
        entry for entry in source_folder.iterdir()
        if entry.is_file() and entry.name.lower().startswith(prefix)
    ]
    matches.sort(key=lambda p: p.name)
    return matches


def build_catalog(raw_sound_root: Path, dry_run: bool) -> dict:
    catalog: dict[str, dict[str, list[str]]] = {}
    unmatched: list[str] = []

    for class_name in CLASSES:
        animevents_path = (
            ANIMEVENTS_DIR / class_name / f"{class_name}.animevents"
        )
        payloads = parse_sound_events(animevents_path)
        class_bucket: dict[str, list[str]] = {}
        common_bucket = catalog.setdefault("Common", {})

        seen_events: set[str] = set()
        for payload in payloads:
            bank, event_name = split_bank_event(payload)
            if event_name in seen_events:
                continue
            seen_events.add(event_name)

            source_folder = resolve_source_folder(bank, class_name, raw_sound_root)
            variants = find_variant_files(source_folder, event_name)
            is_common = bank.upper().startswith("PC_COMMON")
            bucket = common_bucket if is_common else class_bucket
            deploy_subfolder = "Common" if is_common else class_name

            if not variants:
                bucket.setdefault(event_name, [])
                unmatched.append(f"{class_name}: {payload}")
                continue

            relative_paths: list[str] = []
            for variant in variants:
                dest_dir = RESOURCES_SOUND_ROOT / deploy_subfolder
                dest_path = dest_dir / variant.name
                relative_paths.append(
                    f"Sound/Character/{deploy_subfolder}/{variant.name}"
                )
                if dry_run:
                    continue
                dest_dir.mkdir(parents=True, exist_ok=True)
                if not dest_path.exists() or dest_path.stat().st_size != variant.stat().st_size:
                    shutil.copyfile(variant, dest_path)

            bucket[event_name] = relative_paths

        catalog[class_name] = class_bucket

    return {"formatVersion": 1, "classes": catalog}, unmatched


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--raw-sound-root",
        type=Path,
        default=Path(r"D:\\로아 리소스"),
        help="Root of the extracted raw sound library (contains a Sound/ subfolder).",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Resolve and report matches without copying files or writing the catalog.",
    )
    args = parser.parse_args()

    catalog, unmatched = build_catalog(args.raw_sound_root, args.dry_run)

    total_events = sum(
        len(bucket) for bucket in catalog["classes"].values()
    )
    matched_events = sum(
        1
        for bucket in catalog["classes"].values()
        for files in bucket.values()
        if files
    )
    print(f"Events discovered: {total_events}")
    print(f"Events matched to at least one wav: {matched_events}")
    print(f"Events unmatched: {len(unmatched)}")
    for line in unmatched[:40]:
        print(f"  unmatched: {line}")
    if len(unmatched) > 40:
        print(f"  ... and {len(unmatched) - 40} more")

    if args.dry_run:
        return 0

    CATALOG_PATH.parent.mkdir(parents=True, exist_ok=True)
    CATALOG_PATH.write_text(
        json.dumps(catalog, ensure_ascii=False, indent=1) + "\n",
        encoding="utf-8",
    )
    print(f"Wrote {CATALOG_PATH}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
```

### 검증 (harness)

`Tools/CharacterAnimationIntake/test_build_sound_catalog.py`를 같은 폴더의 기존 테스트들과 같은 스타일로 추가한다.
최소 검증 항목:

- `split_bank_event("PC_COMMON_DUAL.PC_Common_Dual1_1") == ("PC_COMMON_DUAL", "PC_Common_Dual1_1")`
- `find_variant_files`가 대소문자 다른 접두사도 매칭하고, 다른 이벤트명의 파일(`..._1_10__...`가
  `..._1_1__...` prefix 매칭에 잘못 걸리지 않는지 — `__` 구분자까지 포함해서 매칭하므로 안전함)을
  섞지 않는지
- `--dry-run`이 파일을 복사하지 않는지
- GunSlinger/Slayer가 매칭 0건이어도 `unmatched`에 쌓일 뿐 스크립트가 0이 아닌 종료 코드를 반환하지
  않는지(실패 취급 금지)

## 5. `Client/Public/AnimationEffectCueDocument.h` 수정 (기존 파일)

**기준점**: `struct ANIMATION_HIT_CUE final { ... };` 블록의 닫는 `};` 바로 아래, `ANIMATION_PROJECTILE_CUE` 선언
바로 위. **작업**: 새 struct 삽입.

```cpp
/* A wav playback trigger, parsed from a "SOUND" .animevents row. strEventName
   is the payload's event name (bank prefix stripped, e.g. "PC_LanceMaster_F_
   Att_Battle1_1") -- the actual wav path(s) are resolved later at Character
   load time via CSoundCueCatalog, not stored here, so this struct stays a
   pure parse result independent of which variant file gets picked at
   playback. */
struct ANIMATION_SOUND_CUE final
{
    std::string strClipName;
    uint32_t iStartMs = 0u;
    std::string strEventName;
};
```

**기준점**: `struct ANIMATION_EFFECT_CUE_DOCUMENT final { ... std::vector<ANIMATION_PROJECTILE_CUE> Projectiles; };`
**작업**: `Projectiles;` 줄 바로 아래에 필드 추가(구조체 닫는 `};`는 그대로 유지).

```cpp
    std::vector<ANIMATION_PROJECTILE_CUE> Projectiles;
    std::vector<ANIMATION_SOUND_CUE> Sounds;
```

## 6. `Client/Private/AnimationEffectCueDocument.cpp` 수정 (기존 파일)

**기준점**: `Load_FromText`의 `if ("HIT" == Tokens[1]) { ... Staged.Hits.push_back(std::move(Hit)); continue; }`
블록(약 687~741행) 바로 아래, `if ("EFFECT" != Tokens[1]) continue;` 바로 위.
**작업**: 새 분기 삽입 — HIT 분기보다 훨씬 단순하다(shape 없음, payload만 있으면 됨).

```cpp
        if ("SOUND" == Tokens[1])
        {
            if (bFilterToAvailableClips &&
                !Is_AvailableClip(Tokens[0]))
            {
                continue;
            }
            bool SoundFieldsValid = false;
            const auto SoundFields = Make_Fields(Tokens, 2u, SoundFieldsValid);
            if (!SoundFieldsValid)
            {
                strOutStatus = "Animation SOUND row has an invalid or duplicate field.";
                return false;
            }
            const auto StartField = SoundFields.find("startms");
            const auto PayloadField = SoundFields.find("payload");
            ANIMATION_SOUND_CUE Sound;
            if (SoundFields.end() == StartField ||
                !Parse_UInt(StartField->second, Sound.iStartMs) ||
                SoundFields.end() == PayloadField ||
                PayloadField->second.empty() ||
                !Is_AvailableClip(Tokens[0]))
            {
                strOutStatus = "Animation SOUND row failed clip/time/payload validation.";
                return false;
            }
            Sound.strClipName = Tokens[0];
            const std::string& Payload = PayloadField->second;
            const auto DotPos = Payload.find('.');
            Sound.strEventName = (std::string::npos == DotPos) ?
                Payload : Payload.substr(DotPos + 1u);
            Staged.Sounds.push_back(std::move(Sound));
            continue;
        }
        if ("EFFECT" != Tokens[1])
            continue;
```

`bDiscoverReferencedClips` 블록(약 682~686행, `"EFFECT" == Tokens[1] || "HIT" == Tokens[1]`)은 그대로 둔다 —
Product prewarm 클립 탐색은 EFFECT/HIT cue가 참조하는 clip만 대상으로 하며 SOUND는 그 계약에 포함되지
않는다(SOUND는 순수 표현이고, prewarm이 앞당겨 로드해야 할 대상은 Effect asset뿐이다).

## 7. `Client/Public/SoundCueCatalog.h` (신규, 전체 코드)

```cpp
#pragma once

#include "Client_Defines.h"

#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Client)

/* Loads Data/Sound/CharacterSoundCatalog.json once and answers "given this
   class and this SOUND cue's event name, which wav files (Resources-relative
   asset IDs) can play for it". Multiple entries are equally-weighted
   variations (the same hit/vox line recorded several times) -- the caller
   picks one, this catalog does not. A missing class or event name is not an
   error: the caller silently skips that cue (Client-only presentation, no
   gameplay authority depends on sound actually playing). */
class CSoundCueCatalog final
{
public:
    static bool_t Load(std::string& strOutStatus);

    /* Empty return means "no match" -- not a failure. strClassName is the
       same Data/Animation/Authored/<Class> folder name Character already
       uses for strAssetName (e.g. "LanceMaster"). */
    static const std::vector<std::string>& Find_Variants(
        const std::string& strClassName,
        const std::string& strEventName);

private:
    static std::unordered_map<std::string,
        std::unordered_map<std::string, std::vector<std::string>>> s_ClassEvents;
    static bool_t s_bLoaded;
};

NS_END
```

## 8. `Client/Private/SoundCueCatalog.cpp` (신규, 전체 코드)

```cpp
#include "SoundCueCatalog.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

namespace
{
    const std::vector<std::string> EmptyVariants;
}

std::unordered_map<std::string,
    std::unordered_map<std::string, std::vector<std::string>>>
    Client::CSoundCueCatalog::s_ClassEvents;
bool_t Client::CSoundCueCatalog::s_bLoaded = false;

bool_t Client::CSoundCueCatalog::Load(std::string& strOutStatus)
{
    s_ClassEvents.clear();
    s_bLoaded = false;

    const filesystem::path CatalogPath =
        CProjectDataRoot::Resolve(L"Sound/CharacterSoundCatalog.json");

    DATA_JSON_DOCUMENT Document;
    if (!Document.Load_FromFile(CatalogPath, strOutStatus))
        return false;

    const DATA_JSON_VALUE& Root = Document.Get_Root();
    const DATA_JSON_VALUE* pClasses = Root.Find("classes");
    if (nullptr == pClasses || !pClasses->Is_Object())
    {
        strOutStatus = "CharacterSoundCatalog.json is missing \"classes\".";
        return false;
    }

    for (const auto& [strClassName, ClassValue] : pClasses->Get_Members())
    {
        if (!ClassValue.Is_Object())
            continue;

        std::unordered_map<std::string, std::vector<std::string>> Events;
        for (const auto& [strEventName, EventValue] : ClassValue.Get_Members())
        {
            if (!EventValue.Is_Array())
                continue;

            std::vector<std::string> Variants;
            for (const DATA_JSON_VALUE& Entry : EventValue.Get_Array())
            {
                if (Entry.Is_String())
                    Variants.push_back(Entry.Get_String());
            }
            Events.emplace(strEventName, std::move(Variants));
        }
        s_ClassEvents.emplace(strClassName, std::move(Events));
    }

    s_bLoaded = true;
    return true;
}

const std::vector<std::string>& Client::CSoundCueCatalog::Find_Variants(
    const std::string& strClassName,
    const std::string& strEventName)
{
    if (!s_bLoaded)
        return EmptyVariants;

    const auto ClassIterator = s_ClassEvents.find(strClassName);
    if (s_ClassEvents.end() == ClassIterator)
        return EmptyVariants;

    const auto EventIterator = ClassIterator->second.find(strEventName);
    if (ClassIterator->second.end() == EventIterator)
        return EmptyVariants;

    return EventIterator->second;
}
```

`DATA_JSON_DOCUMENT`/`DATA_JSON_VALUE`의 정확한 멤버 이름(`Load_FromFile`, `Get_Root`, `Find`, `Is_Object`,
`Get_Members`, `Is_Array`, `Get_Array`, `Is_String`, `Get_String`)은 이번 PLAN 작성 시 직접 열어보지 않았다 —
**구현 전에 `Client/Public/DataJson.h`를 먼저 읽고 실제 시그니처에 맞춰 이 파일을 조정해야 한다.** 다른
Load 계열 코드(`CAnimationEffectCueDocument::Load`가 아니라, `Data/*.json`을 읽는 다른 Catalog 클래스,
예를 들어 `PlayerSkillCatalog.cpp`나 `EffectCatalog.cpp`)의 실제 JSON 리더 패턴을 그대로 따른다.

`CProjectDataRoot::Resolve`가 `Data/` 상대 경로를 받는 실제 시그니처인지도 구현 전에 `ProjectDataRoot.h`로
재확인한다(이 PLAN에서는 `CRuntimeAssetRoot::Resolve`와 이름이 비슷해 혼동하기 쉽다 — 이건 `Data/` 원본을
읽는 것이므로 `Client/Bin/Resources`를 해석하는 `CRuntimeAssetRoot`가 아니라 `Data/`를 해석하는
`CProjectDataRoot`가 맞다).

## 9. `Client/Public/Character.h` 수정 (기존 파일)

**기준점**: `void Update_EffectCues();` 바로 아래(§309-312 부근, `Reset_EffectCueCursor` 선언과 같은 그룹).
**작업**: 함수 선언 추가.

```cpp
	void Update_SoundCues();
```

**기준점**: `f32_t m_fPreviousEffectCueStageWallSeconds = -1.f;` 바로 아래(§240-241 부근).
**작업**: 멤버 변수 추가 — SOUND 전용 커서. EFFECT와 같은 `m_fPreviousEffectCueStageWallSeconds`를 공유하면
루프 epoch 계산이 EFFECT 처리 순서에 종속되므로, 독립된 커서를 둔다.

```cpp
	f32_t m_fPreviousSoundCueStageWallSeconds = -1.f;
```

`Reset_EffectCueCursor`가 `m_fPreviousEffectCueStageWallSeconds`를 `-1.f`로 되돌리는 것과 같은 지점에서
`m_fPreviousSoundCueStageWallSeconds`도 함께 리셋해야 한다(§10 참고).

## 10. `Client/Private/Character.cpp` 수정 (기존 파일)

**기준점**: `void CCharacter::Reset_EffectCueCursor(...) { m_fPreviousEffectCueStageWallSeconds = -1.f; ... }`
함수 본문(274-282행).
**작업**: 같은 함수 본문 안, `m_fPreviousEffectCueStageWallSeconds = -1.f;` 바로 아래에 한 줄 추가.

```cpp
	m_fPreviousSoundCueStageWallSeconds = -1.f;
```

**기준점**: `void CCharacter::Update_EffectCues() { ... }` 함수 전체(354-487행)의 닫는 `}` 바로 아래,
`void CCharacter::Commit_PendingClipChains()` 바로 위.
**작업**: 새 함수 삽입. `Update_EffectCues()`와 완전히 같은 wall-clock/루프-epoch 해석을 재사용하되
(`CActionPresentationTimeline::Resolve_ClipDuration`/`Resolve_CueWallOffset`, `MAX_EFFECT_CUE_OCCURRENCES_PER_UPDATE`
동일 상수), 트리거되면 `EFFECT_SPAWN_DESC`/`CEffectPresentationService::Spawn` 대신 카탈로그 조회 +
`CGameInstance::Get().Play_Sound()`를 호출한다. anchor/orientation/follow/stop policy는 SOUND에는 없는
개념이므로 생략한다.

```cpp
void CCharacter::Update_SoundCues()
{
	if (nullptr == m_pBodyModel || nullptr == m_pChain ||
		0u == m_iEffectActionStartTick || m_iChainStage < 0 ||
		m_iChainStage >= static_cast<int32_t>(m_pChain->stages.size()))
		return;
	if (nullptr == m_pSpec || m_EffectCueDocument.Sounds.empty())
		return;
	std::vector<ACTION_PRESENTATION_CLIP_TIMING> Timings;
	if (!Build_ActiveStageTimeline(Timings))
		return;
	const std::vector<CLIP_STEP>& Clips =
		m_pChain->stages[m_iChainStage].clips;
	const f32_t fCurrentStageWallSeconds = (std::max)(
		0.f, m_fActionPresentationSeconds);
	const f32_t fPreviousStageWallSeconds =
		m_fPreviousSoundCueStageWallSeconds;

	for (std::size_t iCue = 0u;
		iCue < m_EffectCueDocument.Sounds.size(); ++iCue)
	{
		const ANIMATION_SOUND_CUE& Cue = m_EffectCueDocument.Sounds[iCue];
		const std::vector<std::string>& Variants =
			CSoundCueCatalog::Find_Variants(
				m_pSpec->pAssetName, Cue.strEventName);
		if (Variants.empty())
			continue;

		for (std::size_t iClip = 0u; iClip < Clips.size(); ++iClip)
		{
			if (Cue.strClipName != Clips[iClip].clip)
				continue;
			f32_t fSourceDurationSeconds = 0.f;
			f32_t fWallDurationSeconds = 0.f;
			if (!CActionPresentationTimeline::Resolve_ClipDuration(
				Timings[iClip], fSourceDurationSeconds, fWallDurationSeconds))
			{
				continue;
			}
			const f32_t fCueSourceSeconds =
				static_cast<f32_t>(Cue.iStartMs) * 0.001f;
			f32_t fFirstOccurrenceWallSeconds = 0.f;
			if (!CActionPresentationTimeline::Resolve_CueWallOffset(
				Timings, iClip, fCueSourceSeconds, 0u,
				fFirstOccurrenceWallSeconds))
			{
				continue;
			}

			uint64_t iFirstEpoch = 0u;
			uint64_t iLastEpoch = 0u;
			if (Timings[iClip].bLoop)
			{
				if (fCurrentStageWallSeconds <
					fFirstOccurrenceWallSeconds)
				{
					continue;
				}
				if (fPreviousStageWallSeconds >=
					fFirstOccurrenceWallSeconds)
				{
					iFirstEpoch = static_cast<uint64_t>(std::floor(
						(fPreviousStageWallSeconds -
							fFirstOccurrenceWallSeconds) /
						fWallDurationSeconds)) + 1u;
				}
				iLastEpoch = static_cast<uint64_t>(std::floor((std::max)(
					0.f, fCurrentStageWallSeconds -
						fFirstOccurrenceWallSeconds) /
					fWallDurationSeconds));
				if (iFirstEpoch > iLastEpoch)
					continue;
				if (iLastEpoch - iFirstEpoch + 1u >
					MAX_EFFECT_CUE_OCCURRENCES_PER_UPDATE)
				{
					iFirstEpoch = iLastEpoch -
						MAX_EFFECT_CUE_OCCURRENCES_PER_UPDATE + 1u;
				}
			}

			for (uint64_t iEpoch = iFirstEpoch;
				iEpoch <= iLastEpoch; ++iEpoch)
			{
				f32_t fOccurrenceWallSeconds = 0.f;
				if (!CActionPresentationTimeline::Resolve_CueWallOffset(
					Timings, iClip, fCueSourceSeconds, iEpoch,
					fOccurrenceWallSeconds) ||
					fOccurrenceWallSeconds <=
						fPreviousStageWallSeconds ||
					fOccurrenceWallSeconds > fCurrentStageWallSeconds)
				{
					continue;
				}

				const std::size_t iVariant = Variants.size() == 1u ? 0u :
					(std::rand() % Variants.size());
				const filesystem::path SoundPath =
					CRuntimeAssetRoot::Resolve(
						filesystem::path("Sound") / Variants[iVariant]
						.substr(std::string("Sound/").size()));
				CGameInstance::Get().Play_Sound(
					SoundPath.wstring(), 1.f);

				if (iEpoch == (std::numeric_limits<uint64_t>::max)())
					break;
			}
		}
	}
	m_fPreviousSoundCueStageWallSeconds = fCurrentStageWallSeconds;
}
```

`Variants[iVariant]`는 카탈로그에 `"Sound/Character/LanceMaster/xxx.wav"` 형태(§3 스키마의 `Sound/...`
프리픽스 포함)로 저장돼 있고 `CRuntimeAssetRoot::Resolve`는 `Resources/` 상대경로를 받으므로, 위 코드의
`.substr(...)` 잘라내기는 실제로는 불필요한 이중 처리다 — **카탈로그 값 자체를 처음부터
`CRuntimeAssetRoot::Resolve`가 그대로 받는 형태(`"Sound/Character/..."`, 즉 `Resources/` 바로 아래 상대경로)로
통일하고, 이 함수에서는 `CRuntimeAssetRoot::Resolve(Variants[iVariant])`만 호출하도록 정리한다.** (§4 스크립트의
`relative_paths.append(f"Sound/Character/{deploy_subfolder}/{variant.name}")`가 이미 그 형태이므로, 실제로는
`.substr` 없이 `CRuntimeAssetRoot::Resolve(filesystem::path(Variants[iVariant]))`만 쓰면 된다 — 위 코드 블록에서
이 정리를 반영해 최종 구현 시 `.substr` 줄을 삭제한다.)

`std::rand()`는 이 프로젝트에 이미 시드된 PRNG가 다른 곳(예: `CEffectPresentationService`의 파티클
스폰 지터, 있다면)에 있는지 구현 전에 확인하고, 있으면 그걸 재사용한다. 없으면 `<cstdlib>` 기본
`std::rand()`로 충분하다 — 배리에이션 선택은 결정론이 필요 없는 순수 표현 난수다.

**기준점**: `Update_EffectCues();`가 호출되는 지점(1397행 부근).
**작업**: 바로 아래에 호출 추가.

```cpp
	Update_SoundCues();
```

`#include "SoundCueCatalog.h"`를 `Character.cpp`의 include 목록에 추가한다(다른 `#include "..."`들과 같은
스타일로, 알파벳 순서 유지).

## 11. Character 초기화 시점의 카탈로그 로드

`CSoundCueCatalog::Load()`는 게임 전체에서 한 번만 부르면 되는 전역 상태다 — Character 하나가 아니라
Client 시작 시퀀스(예: `Ready_Fonts()`와 같은 층위의 `CMainApp` 초기화, 또는 `CGameInstance::Initialize()`
근처)에서 한 번 호출한다. **정확한 위치는 구현 전에 `MainApp.cpp`의 `Ready_*` 계열 함수들(이번 세션에서 이미
`Ready_Fonts()`를 읽었음)과 `CEffectCatalog`가 언제 로드되는지를 먼저 확인해 같은 시점(비슷한 다른 Catalog들과
나란히)에 추가한다.** 실패 시 `MSG_BOX` 대신 `OutputDebugStringA`로 남기고 계속 진행한다 — 사운드 카탈로그
로드 실패가 Client 부팅을 막으면 안 된다(Client 전용 표현 실패, Server/gameplay와 무관).

## 12. `.vcxproj` / `.vcxproj.filters` 등록

`Client/Default/Client.vcxproj`의 기존 `SoundManager`류 또는 `AnimationEffectCueDocument.h/.cpp`가 등록된
`<ClInclude>`/`<ClCompile>` 항목 바로 옆에 `SoundCueCatalog.h`/`SoundCueCatalog.cpp`를 추가한다.
`.vcxproj.filters`도 `AnimationEffectCueDocument.h/.cpp`와 같은 필터 그룹에 넣는다(새 필터 카테고리를
만들지 않는다 — 기존 필터 재배치 금지 규칙).

## 13. 검증

```text
1. python Tools/CharacterAnimationIntake/build_sound_catalog.py --dry-run
   → 매칭/미매칭 개수가 이 PLAN의 §0 통계(LanceMaster 등 실제 매칭 다수, GunSlinger/Slayer 0)와 맞는지 확인
2. python Tools/CharacterAnimationIntake/build_sound_catalog.py
   → Client/Bin/Resources/Sound/Character/**에 실제 wav 배포됐는지, Data/Sound/CharacterSoundCatalog.json 생성 확인
3. python Tools/CharacterAnimationIntake/test_build_sound_catalog.py 통과
4. Engine 빌드 불필요(Client-only 변경) → UpdateLib.bat 불필요
5. Client Debug 빌드 성공
6. Bern 또는 Valtan 진입, LanceMaster로 평타/스킬 사용 → 타격 사운드가 애니메이션과 맞물려 들리는지
   사용자가 직접 확인(에이전트는 Client 자율 실행/화면·사운드 판정 금지 — 실행 준비와 빌드까지만 하고
   정확한 확인 경로를 사용자에게 전달한다)
7. 다른 애니메이션 재생 경로(Bern 몬스터, Valtan 등)에서 회귀 없는지 — Update_EffectCues()와 완전히 같은
   가드(m_pChain null, m_iEffectActionStartTick 0 등)를 재사용했으므로 위험은 낮지만, 특히 여러 클립이
   빠르게 연속 재생되는 콤보에서 SOUND 큐가 중복 트리거되지 않는지 눈으로 확인
```

## 14. 이번 PLAN에서 확정하지 못한 것 (구현 전 재확인 필요)

- `DATA_JSON_VALUE`의 정확한 API(§8) — 이번 세션에서 `DataJson.h`를 직접 읽지 않았다.
- `CProjectDataRoot::Resolve`의 정확한 시그니처와 `Data/Sound/` 하위를 실제로 해석하는지.
- `CSoundCueCatalog::Load()`를 부를 정확한 초기화 지점(§11).
- `m_pSpec->pAssetName`이 정확히 "LanceMaster" 같은 클래스 폴더 이름과 1:1로 같은 문자열인지(Character.cpp
  237행 `CAnimationEffectCueDocument::Load(m_pSpec->pAssetName, ...)`로 보아 그런 것 같지만 재확인 필요).
- 배리에이션 랜덤 선택에 프로젝트가 이미 쓰는 PRNG가 있는지.
- GunSlinger/Slayer가 나중에 실제 사운드를 갖게 되면 `CLASSES` 목록과 `Character/GunSlinger`,
  `Character/Slayer` 원본 폴더만 채우면 되고, 이 파이프라인의 다른 부분은 수정할 필요가 없다(설계
  자체가 이미 두 클래스를 "빈 매칭"으로 정상 처리하도록 되어 있음).
