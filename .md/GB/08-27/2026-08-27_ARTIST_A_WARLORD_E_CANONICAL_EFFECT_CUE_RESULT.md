# 도화가 A·워로드 E 캐릭터 정본 Effect cue 선택 결과

기준일: 2026-08-27

기준 브랜치: `codex/valtan-animation-tool-presentation-parity`

## G00. 구현 결과

사용자가 지정한 두 saved unified Effect를 기존 제품 캐릭터 정본 경로에 연결했다.

| 캐릭터 / 슬롯 | skillId | Server skill shape | animation occurrence | 실제 Product cue | PlayerSkills 대표/fallback 정본 |
|---|---:|---|---|---|---|
| 도화가 A | 31460 | `ACTIVE` | `sdm_sk_butterflydream` | `effect.artist.skill.31460.linear-reveal.unified` | `effect.artist.skill.31460.linear-reveal.unified` |
| 워로드 E 1단 | 17080 | `COMBO` stage 1/2 | `wgl_sk_dashupperfire_01` | `effect.warlord.skill.17080.clip2.unified` | `effect.warlord.skill.17080.clip2.unified` |
| 워로드 E 2단 | 17080 | `COMBO` stage 2/2 | `wgl_sk_dashupperfire_02` | `effect.warlord.skill.17080.clip2.unified` | 같은 대표 ID 재사용 |

워로드 E의 Server COMBO stage 수와 `skillbindings`의 `_01 -> _02` 애니메이션 순서는 바꾸지 않았다.
서로 다른 두 animation occurrence가 같은 `clip2.unified` visual document를 재사용한다.

`Data/Actors/CharacterCatalog.json`은 캐릭터 body/equipment/animation-set model 정본이므로 Effect ID를
추가하지 않았다. 캐릭터 스킬 Effect의 기존 정본 join은 다음과 같다.

```text
Data/Balance/PlayerSkills.json의 대표/fallback effectId
-> Data/Animation/Authored/<Asset>/<Asset>.skillbindings.json의 skillId별 clip occurrence
-> Data/Animation/Authored/<Asset>/<Asset>.animevents의 effectref=asset Product cue
-> Data/Effects/EffectCatalog.json의 DIRECT_AUTHORED_DOCUMENT row
-> Data/Effects/Authored/<EffectAssetId>.effect.json
```

두 선택 ID의 `EffectCatalog.json` direct row와 authored 문서는 이미 존재했으므로 카탈로그 정의를
중복 추가하지 않았다. `PlayerSkills.json`과 provenance receipt의 두 `effectId`만 선택본으로 맞추고,
실제 timed occurrence는 animevents에 고정했다.

## G01. 변경 파일

| 파일 | 결과 |
|---|---|
| `Data/Balance/PlayerSkills.json` | skill `17080`, `31460`의 대표/fallback `effectId`를 선택본으로 설정 |
| `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json` | 두 `effectId`의 `PROJECT_TUNED` source/result 값을 PlayerSkills와 동기화 |
| `Data/Animation/Authored/Artist/Artist.animevents` | `sdm_sk_butterflydream` Product cue를 `linear-reveal.unified`로 교체 |
| `Data/Animation/Authored/Warlord/Warlord.animevents` | `_01` Product cue를 `clip2.unified`로 교체; `_02`의 기존 clip2 cue 유지 |
| `Tools/EffectPipeline/test_character_skill_effect_canonical_selection.py` | PlayerSkills→binding→cue→catalog→authored document exact join 5개 회귀 검사 추가 |

새 C++ 파일, 새 schema, 새 EffectCatalog row는 없으며 프로젝트 파일 변경도 없다.

## G02. 자동 검증

| 검증 | 결과 | 증거 |
|---|---|---|
| focused canonical selection unittest | PASS | 5 tests, `Ran 5 tests`, `OK` |
| Python syntax | PASS | `python -m py_compile Tools/EffectPipeline/test_character_skill_effect_canonical_selection.py` |
| Gameplay balance publisher | PASS | 6 profiles, 230 skills, 109 damage profiles, 53 boss patterns, 228 stages |
| Effect data project registration | PASS | `files=2375 filters=219` |
| EffectRenderContractHarness Debug | PASS | direct-authored source/catalog/linear/WARP contract 실행 종료 code 0 |
| Artist/Warlord animevents row count | PASS | Artist `1048 == 1048`, Warlord `1723 == 1723` |
| JSON parse | PASS | PlayerSkills, receipt, EffectCatalog, 두 selected authored document |
| 전체 `git diff --check` | PASS | whitespace error 없음; 기존 line-ending warning만 출력 |

`Validate-EffectSources.ps1` 전체 저장소 검사는 이번 두 선택 문서가 아니라 현재 함께 존재하는 다른
Valtan authored 작업의 Git 미추적 resource closure 5개 때문에 FAIL했다.

```text
Effect/Valtan/Textures/FX_TEX_02/fx_d_cloud_031.dds
Effect/Valtan/Textures/FX_TEX_05/fx_o_sector_04.dds
Effect/Valtan/Textures/FX_TEX_05/fx_o_sector_05.dds
Effect/Warlord/Textures/FX_TEX_00/fx_a_hit_007.dds
Effect/Warlord/Textures/FX_TEX_00/fx_a_ring_001.dds
```

참조 검색 결과 위 Warlord 경로 두 개도 Valtan authored 문서가 소비하며, 이번
`effect.warlord.skill.17080.clip2.unified` 또는 Artist 선택 문서의 dependency가 아니다. 다른 팀원의
dirty Valtan 변경을 되돌리거나 추적 상태를 바꾸지 않았다.

이번 변경은 JSON/animevents data-only이고 동일 worktree에 대규모 미커밋 C++ 변경이 있어 전체 Client
build는 실행하지 않았다. 기존 Debug EffectRenderContractHarness와 domain publisher/focused test로
이번 연결 계약을 검증했다.

## G03. 사용자 수동 화면 확인

Client/UI는 에이전트가 실행하지 않았고 visual PASS도 기록하지 않았다. 사용자가 직접 Server + Client를
실행한 뒤 다음을 확인해야 한다.

1. 도화가에서 A를 사용하면 `effect.artist.skill.31460.linear-reveal.unified`가 재생되는지 확인한다.
2. 워로드에서 E 1단과 2단을 연속 입력해 두 occurrence 모두
   `effect.warlord.skill.17080.clip2.unified` visual을 사용하는지 확인한다.
3. Effect Tool에서 Artist 선택 saved row는 `Active Product cue uses this saved unified Effect.`로,
   이전 `effect.artist.skill.31460.unified` row는 active mapping 없음으로 보이는지 확인한다.
4. Warlord clip2 saved row 아래 Product occurrence는 `_01`과 `_02` 두 clip으로 표시되는지 확인한다.

수동 육안 결과는 사용자의 관찰 전까지 미확정이다.
