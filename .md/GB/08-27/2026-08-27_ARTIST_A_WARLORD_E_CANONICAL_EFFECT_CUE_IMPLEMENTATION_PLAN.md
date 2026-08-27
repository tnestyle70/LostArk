# 도화가 A·워로드 E 캐릭터 정본 Effect cue 선택 구현 계획

기준일: 2026-08-27

기준 브랜치: `codex/valtan-animation-tool-presentation-parity`

## G00. 현재 정본과 이번 변경 경계

제품 캐릭터 Effect는 새 캐릭터 Effect 카탈로그를 만들지 않고 다음 기존 정본을 순서대로 소비한다.

```text
Data/Balance/PlayerSkills.json
-> Data/Animation/Authored/<Asset>/<Asset>.skillbindings.json
-> Data/Animation/Authored/<Asset>/<Asset>.animevents의 effectref=asset exact cue
-> Data/Effects/EffectCatalog.json의 DIRECT_AUTHORED_DOCUMENT row
-> Data/Effects/Authored/<EffectAssetId>.effect.json
```

`Data/Actors/CharacterCatalog.json`은 body/equipment/weapon/animation-set model 등록 정본이며 skill
Effect occurrence를 소유하지 않는다. 여기에 Effect ID를 중복 저장하지 않는다.
`Data/Balance/PlayerSkills.json.effectId`는 스킬별 단일 대표 Effect와 cue 부재 시 fallback을 소유한다.
이번에 선택한 두 exact ID를 이 필드에 기록하고, timed occurrence는 계속 animevents가 소유한다.

현재 도화가 A `31460`은 `sdm_sk_butterflydream`에서
`effect.artist.skill.31460.unified`를 사용한다. 사용자가 선택한
`effect.artist.skill.31460.linear-reveal.unified`는 direct-authored catalog row와 문서는 있지만
Product cue가 없다.

현재 워로드 E `17080`은 두 Server COMBO stage를 유지하면서 stage 1은 `clip1.unified`, stage 2는
`clip2.unified`를 사용한다. 이번 변경은 애니메이션 stage를 합치지 않고 두 occurrence 모두 사용자가
선택한 `effect.warlord.skill.17080.clip2.unified`를 재사용한다.

## G01. 수정·추가 파일

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | `C:/Users/user/Desktop/LostArk/Data/Balance/PlayerSkills.json` | 두 스킬의 단일 대표/fallback Effect ID를 선택본으로 설정 |
| 수정 | `C:/Users/user/Desktop/LostArk/Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json` | 두 `effectId` PROJECT_TUNED 결과를 현재 PlayerSkills와 동기화 |
| 수정 | `C:/Users/user/Desktop/LostArk/Data/Animation/Authored/Artist/Artist.animevents` | 도화가 A 단일 occurrence를 선택한 linear-reveal exact ID로 교체 |
| 수정 | `C:/Users/user/Desktop/LostArk/Data/Animation/Authored/Warlord/Warlord.animevents` | 워로드 E stage 1도 기존 stage 2의 `clip2.unified` exact ID를 재사용 |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/EffectPipeline/test_character_skill_effect_canonical_selection.py` | PlayerSkills→binding→cue→catalog→authored 문서의 exact canonical join 회귀 고정 |
| 추가 | `C:/Users/user/Desktop/LostArk/.md/GB/08-27/2026-08-27_ARTIST_A_WARLORD_E_CANONICAL_EFFECT_CUE_RESULT.md` | 실제 구현·자동 검증·사용자 수동 화면 확인 경계 기록 |

새 C++ 파일과 새 데이터 schema는 없으므로 `.vcxproj`와 `.vcxproj.filters` 변경은 없다.
`EffectCatalog.json`의 두 선택 ID는 이미 `DIRECT_AUTHORED_DOCUMENT`로 등록돼 있으므로 다른 세션의
동일 파일 미커밋 변경에 손대지 않고 검증만 한다.

## G02. PlayerSkills 대표/fallback 정본 설정

```json
{
  "skillId": 17080,
  "effectId": "effect.warlord.skill.17080.clip2.unified"
}
```

```json
{
  "skillId": 31460,
  "effectId": "effect.artist.skill.31460.linear-reveal.unified"
}
```

두 스킬의 현재 stage에 authored cue가 존재하면 `CCharacter::Spawn_FallbackEffect`는 fallback을 생성하지
않는다. cue 문서가 해당 stage occurrence를 제공하지 못할 때만 같은 selected ID를 한 번 사용한다.
공식 receipt의 두 `effectId` entry는 기존 `PROJECT_TUNED / project-effect-binding-v1 / identity` 근거를
유지하고 `sourceValue`와 `resultValue`만 동일 ID로 동기화한다.

## G03. Artist.animevents exact cue 교체

적용 위치: `sdm_sk_butterflydream`의 `effectref=asset` 행 전체 교체.

```text
"sdm_sk_butterflydream" EFFECT startms=0 payload="effect.artist.skill.31460.linear-reveal.unified" effectref=asset anchor="root" follow=follow stop=natural px=0 py=0 pz=0 rx=0 ry=0 rz=0 sx=1 sy=1 sz=1
```

문서 내부 두 particle의 `startDelaySeconds`가 reveal 시각을 소유하므로 cue는 clip 시작 0ms를 유지한다.
다른 `src=orig` EFFECT 행은 Product cue가 아니므로 변경하지 않는다.

## G04. Warlord.animevents clip2 canonical 재사용

적용 위치: `wgl_sk_dashupperfire_01`의 `effectref=asset` 행 전체 교체. 기존
`wgl_sk_dashupperfire_02 -> clip2.unified` 행은 유지한다.

```text
"wgl_sk_dashupperfire_01" EFFECT startms=0 payload="effect.warlord.skill.17080.clip2.unified" effectref=asset anchor="root" follow=follow stop=natural px=0 py=0 pz=0 rx=0 ry=0 rz=0 sx=1 sy=1 sz=1
"wgl_sk_dashupperfire_02" EFFECT startms=0 payload="effect.warlord.skill.17080.clip2.unified" effectref=asset anchor="root" follow=follow stop=natural px=0 py=0 pz=0 rx=0 ry=0 rz=0 sx=1 sy=1 sz=1
```

두 행은 서로 다른 clip occurrence이므로 하나로 합치지 않는다. Server가 확정한 COMBO stage와
`skillbindings`의 `_01`, `_02` 애니메이션 순서는 그대로 유지하고 visual document만 재사용한다.

## G05. 회귀 하네스 전체 코드

### G05-01. `Tools/EffectPipeline/test_character_skill_effect_canonical_selection.py`

```python
import json
import re
import unittest
from pathlib import Path


class CharacterSkillEffectCanonicalSelectionTests(unittest.TestCase):
    ARTIST_EFFECT = "effect.artist.skill.31460.linear-reveal.unified"
    WARLORD_EFFECT = "effect.warlord.skill.17080.clip2.unified"

    @classmethod
    def setUpClass(cls) -> None:
        cls.repository_root = Path(__file__).resolve().parents[2]
        cls.player_skills = cls.load_json("Data/Balance/PlayerSkills.json")[
            "skills"
        ]
        cls.catalog_rows = {
            row["effectAssetId"]: row
            for row in cls.load_json("Data/Effects/EffectCatalog.json")["effects"]
        }

    @classmethod
    def load_json(cls, relative_path: str) -> dict:
        return json.loads(
            (cls.repository_root / relative_path).read_text(encoding="utf-8")
        )

    @classmethod
    def load_binding(cls, asset_name: str, skill_id: int) -> dict:
        document = cls.load_json(
            f"Data/Animation/Authored/{asset_name}/{asset_name}.skillbindings.json"
        )
        return next(
            row for row in document["bindings"] if row["skillId"] == skill_id
        )

    @classmethod
    def collect_product_cues(
        cls, asset_name: str, clip_names: set[str]
    ) -> list[tuple[str, int, str]]:
        event_path = (
            cls.repository_root
            / f"Data/Animation/Authored/{asset_name}/{asset_name}.animevents"
        )
        cue_pattern = re.compile(
            r'^"([^"]+)" EFFECT startms=(\d+) payload="([^"]+)" '
            r'effectref=asset\b'
        )
        cues: list[tuple[str, int, str]] = []
        for line in event_path.read_text(encoding="utf-8").splitlines():
            match = cue_pattern.match(line)
            if match is None or match.group(1) not in clip_names:
                continue
            cues.append((match.group(1), int(match.group(2)), match.group(3)))
        return cues

    def test_player_skill_owners_and_stage_shapes_remain_authoritative(self) -> None:
        artist = next(row for row in self.player_skills if row["skillId"] == 31460)
        warlord = next(row for row in self.player_skills if row["skillId"] == 17080)

        self.assertEqual(
            (artist["characterClass"], artist["inputSlot"], artist["skillKind"]),
            ("ARTIST", "A", "ACTIVE"),
        )
        self.assertEqual(
            (warlord["characterClass"], warlord["inputSlot"], warlord["skillKind"]),
            ("WARLORD", "E", "COMBO"),
        )
        self.assertEqual(artist["effectId"], self.ARTIST_EFFECT)
        self.assertEqual(warlord["effectId"], self.WARLORD_EFFECT)
        self.assertEqual(len(warlord["comboStages"]), 2)

    def test_skillbindings_keep_the_selected_animation_occurrences(self) -> None:
        artist = self.load_binding("Artist", 31460)
        warlord = self.load_binding("Warlord", 17080)

        self.assertEqual(artist["clips"], ["sdm_sk_butterflydream"])
        self.assertEqual(
            warlord["clips"],
            [
                ["wgl_sk_dashupperfire_01"],
                ["wgl_sk_dashupperfire_02"],
            ],
        )

    def test_artist_a_uses_only_the_selected_linear_reveal_product(self) -> None:
        self.assertEqual(
            self.collect_product_cues("Artist", {"sdm_sk_butterflydream"}),
            [("sdm_sk_butterflydream", 0, self.ARTIST_EFFECT)],
        )

    def test_warlord_e_both_combo_occurrences_reuse_clip2_product(self) -> None:
        self.assertEqual(
            self.collect_product_cues(
                "Warlord",
                {"wgl_sk_dashupperfire_01", "wgl_sk_dashupperfire_02"},
            ),
            [
                ("wgl_sk_dashupperfire_01", 0, self.WARLORD_EFFECT),
                ("wgl_sk_dashupperfire_02", 0, self.WARLORD_EFFECT),
            ],
        )

    def test_selected_effects_are_direct_authored_catalog_definitions(self) -> None:
        for effect_asset_id in (self.ARTIST_EFFECT, self.WARLORD_EFFECT):
            row = self.catalog_rows[effect_asset_id]
            self.assertEqual(row["payloadKind"], "DIRECT_AUTHORED_DOCUMENT")
            expected_authoring_path = (
                f"Effects/Authored/{effect_asset_id}.effect.json"
            )
            self.assertEqual(row["authoringPath"], expected_authoring_path)

            document = self.load_json(f"Data/{expected_authoring_path}")
            self.assertEqual(document["effectAssetId"], effect_asset_id)
            self.assertTrue(any(element["visible"] for element in document["elements"]))


if __name__ == "__main__":
    unittest.main()
```

## G06. 검증

1. Artist/Warlord animevents 선언 row count와 실제 row count가 같은지 확인한다.
2. 새 focused unittest를 실행해 PlayerSkills 대표 ID, 두 exact cue와 catalog/document identity를 확인한다.
3. `Publish-GameplayBalance.ps1 -Mode Validate`로 PlayerSkills와 provenance coverage를 확인한다.
4. `Validate-EffectSources.ps1`로 direct-authored source와 Resources closure를 검사한다.
5. `Sync-EffectDataProject.ps1 -Check`로 DataFiles 프로젝트 등록을 확인한다.
6. 기존 EffectRenderContractHarness Debug를 실행해 direct-authored parse/draw 회귀를 확인한다.
7. 데이터-only 변경이므로 Client 전체 build는 기존 dirty C++ 변경과 분리해 필요 여부를 판정하고,
   미실행 시 RESULT에 명시한다.
8. `git diff --check`를 실행한다.
9. 사용자가 직접 Server + Client에서 도화가 A와 워로드 E 1·2단을 확인한다. 에이전트는 Client/UI를
   실행하거나 visual PASS를 대신 기록하지 않는다.
