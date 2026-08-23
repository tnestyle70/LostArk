# 2026-08-23 발탄 비석 패턴 게임플레이 증명과 폐기된 이펙트 복원 결과

## 1. 판정

"비석 4개 생기면서 쓰는 스킬 — 양자택일, 붉은 검기, 그리고 비석이 부서지는 것"이
해결되지 않은 것 같다는 보고를 조사했다. 원인은 두 층으로 갈렸다.

- **게임플레이는 정상이었다.** 다만 아무도 그걸 증명한 적이 없었다. 기존 비석 계약
  6건은 전부 `CEncounterPropRuntime`을 직접 호출할 뿐, 패턴 스테이지에서 프롭
  런타임으로 이어지는 **실제 배선을 한 번도 통과시키지 않았다.** 그 배선을 통과하는
  계약 3건을 새로 넣었고 전부 통과한다.
- **시각 표현은 실제로 사라져 있었다.** 2026-08-22 커밋 `3dd2a3e0`
  "effect: rebuild Valtan Product from exact carriers"가 발탄 이펙트 소유자 105개를
  폐기했고, 그중 **48개만 carrier-v1 후속물을 받았으며 57개는 후속물 없이 사라졌다.**
  사용자가 지목한 세 패턴이 정확히 거기 걸려 있었다. 그 넷을 복원했다.

Client/UI는 에이전트가 실행하지 않았다. 복원된 표현이 원작에 맞는지는 사용자가
최종 판정해야 한다.

## 2. 게임플레이 사슬 — 실측으로 확인한 현재 계약

| 단계 | 소유 | 실측 |
|---|---|---|
| 비석 세우기 | 하드코딩 `GameRoom.cpp:28-29` | `VALTAN_FOUR_PILLARS_105` / `RECOVERY` 진입 시 `Prepare_Spawn` |
| 발동 시점 | `ValtanEncounter.json` | `triggerHealthBar = 100` (패턴 이름의 105가 아니다) |
| 비석 부수기 | `ValtanPropBreakActions.json` → `PATTERNSTAGEPROPBREAK` 4행 | `VALTAN_RED_BLADE_WAVE` PROJECTILE → slot00·slot02, RECOVERY → slot01·slot03 |
| 부채꼴 관통 | `ValtanCoverPiercingActions.json` → `PATTERNSTAGECOVERPIERCE` 1행 | `TARGET_CONE`만 엄폐 무시, 전용 damage `...-105-pierce` 100000% |
| 좌표 | `EncounterProps.world.json` → `VALTAN_ARENA.encounterpropsbootstrap` | 4 slot, 대각선 배치, coverRadius 0.9 |

`PATTERNSTAGEPROPBREAK` 행의 `actionId`와 인카운터 스테이지의 `actionId`가 정확히
일치한다. `GameRoom.cpp`에 남아 있던 주석 *"the shatter the product path has no owner
for yet"* 은 propbreak 도입 이전의 낡은 서술이다.

`양자택일`은 `VALTAN_MAGIC_CHOICE`(`displayName` "마력기운 양자택일",
`maximumHealthBar` 109)이며 어떤 로테이션에도 없고 가중치 추첨으로만 등장한다.
`VALTAN_RED_BLADE_WAVE`는 `rotation.valtan.100.84`(from 99 → to 84)의 0번 스텝이다.
즉 bar 100에서 비석이 서고, bar 99에 열리는 밴드의 첫 패턴이 비석을 부순다.

## 3. 사라진 시각 표현 — 폐기 receipt가 직접 기록한 것

`Data/Effects/Imported/Valtan/CarrierV1/Valtan.carrier-v1-materialization-receipt.v1.json`

```text
retiredOwnerSuccessorMappings                  105
  REPLACED_BY_EXACT_CARRIER_V1_CLIP_OWNER       48
  RETIRED_NO_EXACT_REVIEWED_CARRIER_OWNER       57   <- 후속물 없음
baselineValtanCatalogCount 108 -> finalValtanCatalogCount 46
baselineBossRootCueCount   106 -> finalBossRootCueCount   44
```

사용자가 지목한 세 패턴의 폐기 내역:

| 패턴 | 스테이지 | 폐기된 effectAssetId | 후속물 |
|---|---|---|---|
| `VALTAN_FOUR_PILLARS_105` | TAKEOFF | `effect.valtan.four-pillars-105.takeoff` | **없음** |
| `VALTAN_FOUR_PILLARS_105` | TARGET_CONE | `effect.valtan.four-pillars-105.target-cone` | **없음** |
| `VALTAN_RED_BLADE_WAVE` | WINDUP | `effect.valtan.red-blade-wave.windup` | **없음** |
| `VALTAN_RED_BLADE_WAVE` | RECOVERY | `effect.valtan.red-blade-wave.recovery` | carrier-v1 |
| `VALTAN_MAGIC_CHOICE` | WINDUP | `effect.valtan.magic-choice.windup` | **없음** |
| `VALTAN_MAGIC_CHOICE` | INNER/OUTER/RECOVERY | — | carrier-v1 |

폐기 뒤 디스크에 남은 문서는 `"modelCues": [], "elements": []`인 약 350바이트 빈
껍데기였다. 발행해도 아무것도 그려지지 않는다. 즉 **비석 패턴은 노란 장판도 부채꼴
예고도 화면에 없었다.**

## 4. 복원 방법과 근거

receipt의 `physicalPreimageDisposition`이
`ELEMENTS_CLEARED_EVIDENCE_SHELL_GIT_HISTORY_OWNS_PREIMAGE`이고 `preimageByteSha256`를
남겨 두었다. `3dd2a3e0^`에서 원본을 꺼내 **기록된 SHA256과 바이트 단위로 대조**한 뒤
복원했다.

| effectAssetId | elements | bytes | sha256 일치 |
|---|---:|---:|---|
| `effect.valtan.four-pillars-105.takeoff` | 6 | 36,944 | 예 |
| `effect.valtan.four-pillars-105.target-cone` | 100 | 632,677 | 예 |
| `effect.valtan.red-blade-wave.windup` | 36 | 226,332 | 예 |
| `effect.valtan.magic-choice.windup` | 24 | 150,502 | 예 |

같은 커밋의 preimage에서 catalog 항목 4개와 cue 행 4개도 그대로 복원했다.
두 JSON은 `indent=2` 왕복이 바이트 동일함을 먼저 확인하고 재직렬화했다.

**이 복원물은 carrier-v1의 "exact reviewed carrier"가 아니라 그 이전의 저작본이다.**
재작업이 이들을 폐기한 이유는 검토된 exact carrier 소유자가 없었기 때문이고
(`RETIRED_NO_EXACT_REVIEWED_CARRIER_OWNER`), 그 소유자를 만들려면 원본 occurrence
검토와 육안 판정이 필요하다. 그건 사용자 영역이다. 이번 복원은 **표현이 아예 없는
상태보다 이전 표현이 낫다**는 판단의 임시 조치이며 되돌리기 쉽다.

## 5. 변경 내용

- `Data/Effects/Authored/` 4개 문서 복원
- `Data/Effects/EffectCatalog.json` 항목 4개 추가 (257 → 261)
- `Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json` 큐 4행 추가 (44 → 48)
- `Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json` 재발행 (145 → 149)
- `Server/Private/ServerGameplayContractTests.cpp` 계약 3건 추가

## 6. 자동 검증 결과

| 검증 | 결과 |
|---|---|
| `Publish-Effects.ps1 -Mode Validate` | **PASS**, 149 catalog entries |
| `Publish-Effects.ps1 -Mode Publish` | **PASS**, 149 Effects 발행 |
| `validate_boss_pattern_effects.py` | **PASS** |
| 큐 → 런타임 카탈로그 대조 | 48 cue 전부 해석, 미해결 0 |
| `Server.exe --contract-test` | **711 PASS, failures : 0** |
| `Server.vcxproj` Debug x64 | 성공, 오류 0 |

새로 추가한 계약 3건:

```text
Raise the Valtan stele set from the mechanic stage edge the room runs
Shatter one stele diagonal on the wave projectile edge and leave the other standing
Shatter the opposite stele diagonal on the wave recovery edge
```

복원 후 세 패턴의 스테이지 커버리지:

```text
VALTAN_FOUR_PILLARS_105  TAKEOFF OK  YELLOW_ZONE 없음(원래 없었음)  TARGET_CONE OK  RECOVERY 없음(원래 없었음)
VALTAN_RED_BLADE_WAVE    WINDUP OK   PROJECTILE 없음(combat-object 소유가 정상)  RECOVERY OK
VALTAN_MAGIC_CHOICE      WINDUP OK   INNER OK   OUTER OK   RECOVERY OK
```

`PROJECTILE`에 pattern cue가 없는 것은 결함이 아니다. 그 표현은 `BossCatalog.json`의
combat-object visual `effect.valtan.red-blade-wave.active`가 소유하며 런타임 카탈로그에
실재한다.

## 7. 환경에서 발견한 것 (수정하지 않음)

`Publish-Effects.ps1`은 `core.autocrlf=true` 체크아웃에서 그냥 실패한다.
`artist-f-golden.material-program-fragment.v1.json`의 git blob은 LF인데 체크아웃이
CRLF로 바꿔 놓고, 퍼블리셔는 작업 트리 파일이 LF일 것을 요구한다. `Data/Effects`
아래 1,610개 파일이 같은 상태다. `.gitattributes`는 팀 규칙상 건드리지 않았고,
이번에는 발행 동안만 작업 트리를 LF로 정규화한 뒤 되돌렸다. 영구 해결은
`.gitattributes`에 `eol=lf`를 선언하는 것이며 팀장 판단이 필요하다.

`validate_direct_authored_effect_runtime.py --catalog Data/Effects/EffectCatalog.json`은
`runtime catalog fields or order are invalid`로 실패하는데, **HEAD 원본 카탈로그로
돌려도 같은 에러**가 나므로 기존 상태다.

## 8. 사용자 확인이 필요한 것

1. bar 100에서 비석 4개가 서고 노란 장판과 부채꼴 예고가 보이는지
2. 부채꼴에 맞으면 즉사하고, 비석 뒤에 서면 링은 막히는지
3. 붉은 검기가 날아갈 때 대각선 비석 2개가 먼저 부서지고, 회수 동작에서 나머지 2개가
   부서지는지
4. 양자택일의 windup 표현이 돌아왔는지

## 9. 남은 경계

carrier-v1 재작업이 후속물 없이 폐기한 소유자는 총 57개이고 이번에 4개를 되살렸다.
**53개가 24개 패턴에 걸쳐 남아 있다.** 스테이지 수 기준 상위는
`VALTAN_GHOST_TRANSITION_15` 5, `VALTAN_TRIPLE_COUNTER` 5,
`VALTAN_CENTER_GRAB_COUNTER_64` 4, `VALTAN_ARENA_BREAK_109` 3,
`VALTAN_BIND_CHARGE_SMASH` 3, `VALTAN_DASH_CHARGE` 3, `VALTAN_JUMP_SPIN` 3,
`VALTAN_STOMP` 3이다. 전부 같은 방법으로 되살릴 수 있으나, 어디까지 되살릴지는
carrier-v1 정책과의 절충이므로 사용자가 정한다.

`VALTAN_FOUR_PILLARS_105`의 `YELLOW_ZONE`과 `RECOVERY`는 폐기 이전에도 표현이 없었다.
노란 장판을 보이게 하려면 새 저작이 필요하다.
