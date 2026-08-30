# Effect Authored 런타임 도달성 정리 구현 결과

기준일: 2026-08-29

기준 물리 작업 폴더: `C:/Users/user/Desktop/LostArk`

## 0. 완료 요약

사용자가 지정한 303개 정리 후보에서 발탄 V1 alias 6개를 보류하고 실제 물리
`Data/Effects/Authored` 문서 297개를 삭제했다.

```text
기존 정리 대상                         268
추가 계약 정리 대상                     35
추가 대상 중 발탄 V1 보류                -6
실제 물리 삭제                         297
```

최종 보존 집합은 실제 캐릭터 입력 계약 102개, 발탄 Product 계약 63개, 보류한 발탄 V1
alias 6개로 정확히 171개다. 정렬한 297개 삭제 상대 경로 집합의 SHA-256은
`54e0e9d980b31cb55beb14249f247f624c52aa3be2c1be7d785a80de347fde01`이다.

## G00. 실제 반영 결과

| 항목 | 시작 | 완료 | 결과 |
|---|---:|---:|---|
| `Data/Effects/Authored/*.effect.json` | 468 | 171 | 297개 물리 삭제 |
| working `EffectCatalog.json` direct 행 | 197 | 171 | 도달하지 않는 26개 계약 삭제 |
| `ValtanPatternAuthoringEffects.json` draft binding | 4 | 0 | 빈 draft 계약으로 유지 |
| `PlayerSkills.json` non-empty fallback | 16 | 4 | stale 12개 해제 |
| DimensionMaster non-empty fallback | 12 | 0 | 구형 base 경로 제거 |

삭제 전 Authored 문서의 물리 크기 160,085,177 byte에서 완료 후 63,049,514 byte가 남아
97,035,663 byte가 물리 폴더에서 제거됐다.

## G01. 보존한 런타임 계약

최종 집합 대조 결과는 다음과 같다.

```text
Character input cue/fallback           102
Valtan Product cue/Boss visual          63
Held Valtan V1 alias                     6
Reachable union                        171
EffectCatalog                          171
Catalog - reachable                      0
Reachable - catalog                      0
```

보류한 발탄 V1 target은 다음 6개이며 모두 물리 파일과 catalog 행이 남아 있다.

```text
effect.valtan.carrier-v1.attack.magic-choice.inner.clip-01.v1.unified
effect.valtan.carrier-v1.attack.magic-choice.outer.clip-01.v1.unified
effect.valtan.carrier-v1.attack.magic-choice.recovery.clip-01.v1.unified
effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01.v1.unified
effect.valtan.carrier-v1.mechanic.armor-break-opening.charge.clip-01.v1.unified
effect.valtan.pattern.420633.active.v1.unified
```

남은 4개 PlayerSkills fallback은 Warlord 1개와 Artist 3개이며 모두 catalog에 존재한다.
DimensionMaster 표현은 현재 `.skillbindings.json -> .animevents -> effectref=asset` 입력 계약만
사용하고 구형 base fallback은 남지 않았다.

## G02. 코드와 검증 계약 정리

`validate_effect_sources.py`가 이제 다음을 fail-closed로 검사한다.

```text
PlayerSkills non-empty effectId -> Product catalog
물리 Authored = Product catalog + 명시된 Valtan draft
실제 캐릭터 입력 cue/fallback + Valtan cue/Boss visual + V1 alias = Product catalog
Product와 draft ID 충돌 금지
```

삭제한 corpus를 다시 생성하던 발탄 requested-element one-shot generator/test와 이미 제거된
red-vortex world row 전용 obsolete test도 제거했다. Effect Tool의 호출자 없는 구형
DimensionMaster T draft 생성 함수와 retired baseline/source ID는 삭제하고, 현재 입력 계약이
소비하는 `effect.dimensionmaster.skill.2050500.unified` 지원은 보존했다.

## G03. 자동 검증 결과

| 검증 | 결과 |
|---|---|
| `Validate-EffectSources.ps1 -AllowLocalResources` | PASS, direct 171 / unbound 0 |
| Effect source validator 단위 테스트 | PASS, 37개 |
| Valtan draft 문서 테스트 | PASS, 5개 |
| Effect Tool Valtan all-effects 계약 | PASS, 31개 |
| Effect Tool Valtan saved-row 계약 | PASS, 35개 / skip 7개 |
| Map effect runtime lifecycle 계약 | PASS, 6개 |
| Map effect presentation 계약 | PASS, 14개 |
| 변경 Python 파일 `py_compile` | PASS |
| `Project-ValtanPatternMaster.ps1 -Mode ValidateV2` | PASS |
| `Publish-BalanceRuntimeSet.ps1 -Mode Validate` | PASS |
| Client Debug x64 직접 빌드 | PASS, `Effect_Tool.cpp` compile 및 `Client.exe` link |
| JSON parse / 171 대 171 집합 대조 / V1 6개 존재 | PASS |
| scoped 및 전체 `git diff --check` | PASS, 기존 line-ending warning만 존재 |

엄격한 `Validate-EffectSources.ps1`은 이번 정리와 무관하게 이미 물리 폴더에 있던 Git 미추적
리소스 3개를 보고해 실패한다. 로컬 리소스를 허용한 동일 source/runtime 계약 검증은 통과했다.

```text
Client/Bin/Resources/Effect/Artist/Textures/fx_m_smokesq_01.dds
Client/Bin/Resources/Effect/DimensionMaster/Textures/BG_OCN_ETC_J/bg_ocn_etc_magicsquare08a_d_kmk.dds
Client/Bin/Resources/Effect/Esther/Wei/Textures/FX_TEX_00/fx_a_fire_023.dds
```

별도 `EffectRenderContractHarness` Debug/Release 기존 바이너리는 이번 297개 삭제 대상이 아닌
선행 구조 정리의 누락 입력
`Data/Effects/Imported/Artist/Candidates/skill.31470.reconstructed-runtime-program.candidate.json`을
요구해 exit 2로 시작 전 중단됐다. 이 harness는 현재 Core build/regression 실행 목록에서 호출되지
않으며, 이번 Product 도달성 validator와 focused 계약 테스트는 모두 통과했다.

## G04. 수동 검증과 작업 트리 경계

팀 규칙에 따라 Client/UI를 실행하거나 visual fidelity를 대신 판정하지 않았다. 사용자의 최종 화면
검증은 별도다.

작업 시작 전부터 대규모 미커밋 변경과 물리 구조 정리가 함께 존재했다. 다른 변경을 되돌리거나
정리하지 않았고, 이 결과는 stage/commit/push하지 않았다. 특히 working catalog는 선행 변경으로
HEAD의 298행에서 이미 197행이었으며 이번 작업이 소유한 변화는 그 197행에서 26행을 제거해
171행으로 만든 부분이다. balance provenance receipt도 이번 작업은 DimensionMaster 12개
`sourceValue/resultValue` 동기화만 소유한다.
