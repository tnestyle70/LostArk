# F1 Balance Test · 공식 데이터 provenance 인수인계

작성일: 2026-08-05
대상: Gameplay/Balance, UI, Boss, Animation/Effect, Server 담당자

## 1. 지금 바로 기억할 결론

Debug/Release `F1 -> Balance Test -> Save + Apply`의 현재 numeric 저장·반영 정본은
[BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md](BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md)의 1절이다.
Players/Skills/Damage/Bosses/Madness/Stagger/Pattern damage의 실제 Server 실효값을 읽고, stable ID·field
이전값과 base numeric revision을 typed command sink로 보낸다. Server가 canonical source/provenance를
검증·원자 저장하고 게시 bootstrap과 모든 shared/private room을 활성화한다. 모든 Client는 F1 창이 닫혀
있어도 새 snapshot으로 수치를 갱신하며 Client 로컬 PowerShell 저장이나 프로세스 재시작은 필요하지 않다.
실패·충돌은 현재 draft와 활성 catalog를 보존한다. 패턴 구조/이펙트/새 asset 편집은 기존 owning Tool의
별도 publish/Debug candidate 경로이며 numeric Save로 변경하지 않는다.

이 문서의 아래 provenance와 오프라인 publisher 절차는 외부 저작/게시용이다. 예전
`Save-BalanceTestDraft.ps1`과 `Publish-BalanceRuntimeSet.ps1`은 F1의 현재 네트워크 저장 구현이 아니다.

`Kill Current Gate Boss`는 Balance Test와 F1의 Valtan/KoukuSaydon Arena 영역에 같은
typed 명령으로 표시한다. Server가 현재 관문 primary boss만 결정하고 정상 사망 처리로
넘긴다. G2는 두 primary actor, G3는 G3 actor만 대상이며 다음 Bingo boss를 함께 지우지
않는다. 기존 clear/Encore/보상 흐름을 사용하고 최종 클리어 flag를 직접 설정하지 않는다.
Debug/Release Server가 같은 검증과 정상 사망 경로를 사용한다. 표시된 boss가 이미 바뀐 요청과 재전송은 거부한다.

`Debug (3s)` / `Release (Retail)`은 실행 중 현재 room 전체의 쿨타임 정책을 선택한다.
빌드 구성과 무관하게 새 room은 3초 모드로 시작하며 원래 0초인 평타/스킬은 0초를 유지한다.
Release 모드는 게시된 Retail 쿨타임(ALT_V 300초)을 사용한다. 아직 진행 중인 cooldown은
최초 사용 시점을 유지해 재계산하고 이미 끝난 cooldown과 기믹/차량 타이머는 재개하지 않는다.
같은 방의 모든 플레이어와 늦게 입장한 플레이어가 Server snapshot의 모드와 실제 duration을
받는다. 파일 Save/Publish 및 Server 재시작과 별개의 즉시 정책이다.

Server numeric snapshot은 Retail이 덮는 field의 실효값을 보내고 Server 저장기는 같은 profile row로 저장한다.
덮지 않는 이동/충돌/timing field는 base JSON에 저장한다. 현재 플레이어 치명타,
damage coefficient/addend/spread도 편집한다. 프로필 계수가 쓰이는 damage의 base rate는 숨긴다.
Retail이 새로 소유한 field에 대한 오래된 base draft는 저장을 거부한다. 통합 게시의 기본 및
오프라인 게시 프로필은 Retail이며 Gameplay와 World에 같은 값을 전달한다. F1 numeric 저장 범위는 위 정본을 따른다.

`Retail.balanceprofile.json`의 boss별 `staggerGaugeMaximum`은 현재 미소비 field다.
실제 무력화는 저작 `SET_STAGGER_GAUGE`와 Retail 전역 배율을 사용하며 공용 panel은
미소비 boss gauge field를 편집 항목으로 노출하지 않는다. `attackSpeedPercent`도 아직
행동시간/animation에 반영하지 않는다.

## 2. 정본 파일

| 역할 | 정본 |
|---|---|
| 실제 전투 수치 override | `Data/Balance/Profiles/Retail.balanceprofile.json` |
| player HP/resource/AP/defense/move | `Data/Balance/PlayerProfiles.json` |
| class/slot/skill/timing/range/combo | `Data/Balance/PlayerSkills.json` |
| attack power에 곱하는 damage rate | `Data/Balance/DamageProfiles.json` |
| Valtan HP/AP/detection/movement/phase threshold | `Data/Balance/BossProfiles.json` |
| Valtan pattern graph/selection/stage/action/hit/motion/volley | `Data/Valtan/Valtan.gameplay.json` |
| Valtan animation/Effect/camera invocation | `Data/Valtan/Valtan.presentation.json` |
| Valtan combat-object definition | `Data/Valtan/Valtan.combatobjects.json` |
| Valtan stable world-event membership | `Data/Valtan/Valtan.worldeventsets.json` |
| Valtan unmanaged Product migration closure | `Data/Valtan/Valtan.legacy-compatibility.json` |
| field-level 출처와 변환 | `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json` |
| 재추출기 | `Tools/GameplayPipeline/Export-OfficialBalanceReceipt.py` |
| Tool 편집 후 receipt 동기화 | `Tools/GameplayPipeline/Update-BalanceProvenanceReceipt.ps1` |
| domain 검증·cook | `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`, `Tools/WorldPipeline/Publish-WorldGameplay.ps1` |
| Balance/World/Items 통합 promotion | `Tools/GameplayPipeline/Publish-BalanceRuntimeSet.ps1` |
| F1 Server numeric 저장 | `Server/Private/ServerBalanceNumericStore.cpp` |
| 오프라인 숫자 draft 병합 도구 | `Tools/GameplayPipeline/Save-BalanceTestDraft.ps1` |

`Data/Valtan/Valtan.pattern.json`은 migration fixture다. `Data/Encounters/Valtan/ValtanEncounter.json`, rotations,
combat objects, world events, pattern bindings/cues와 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`은 생성물이다.
직접 편집하지 않는다.

## 3. 공식값 표기의 정확한 의미

receipt는 현재 5 player profile, 53 skill definition, 54 damage profile, Valtan boss/encounter의
1,058 field를 덮는다. 모든 field에 기록이 있다는 뜻이지 모든 값이 공식이라는 뜻은 아니다.

| basis | 의미 |
|---|---|
| `OFFICIAL_EXTRACTED` | 원작 table cell과 값이 직접 일치 |
| `OFFICIAL_DERIVED` | 원작 값을 단위 변환 또는 명시 join으로 유도 |
| `OFFICIAL_SCALED` | 원작 비율을 보존하고 프로젝트 규모로 축소 |
| `PROJECT_TUNED` | 팀이 게임플레이/표현 목적으로 결정 |
| `REFERENCE_ONLY` | runtime admission 근거가 아닌 조사 자료 |

현재 발탄 `basic-swing` 350%, 800/300/1200ms, phase 50%와 실제 phase별 패턴 세트는 공식 완료가
아니다. 원작 `MN_RPBF_01-1.loa`의 SHA-256은 receipt에 남지만, client action payload만으로 원작
Server pattern timing/damage를 증명하지 않는다.

Balance Tool에서 공식 표시 field를 수정하면 동기화 스크립트가 그 field의 basis를
`PROJECT_TUNED`로 바꾸고 `balance-tool-authored-override-v1`을 기록한다. 공식값으로 되돌렸다고
사람이 basis를 직접 고치지 않는다. 원본 DB로 재추출기를 다시 실행해 동일성이 재확인되어야 공식
basis가 복원된다.

## 4. 공식 receipt 재생성

원본 payload는 Git과 Resources에 넣지 않는다. 로컬에 `data2.lpk`, `data3.lpk`, 추출된 여섯 DB와
Valtan action 파일이 있을 때만 다음 명령을 실행한다.

```powershell
python Tools/GameplayPipeline/Export-OfficialBalanceReceipt.py `
  --project-root . `
  --table-root .codex_tmp/data2_tables/data2/EFGame_Extra/ClientData/TableData `
  --data2-lpk C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/data2.lpk `
  --data3-lpk C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/data3.lpk `
  --valtan-action .codex_tmp/data3_reextract_20260805/data3/EFGame_Extra/ClientData/XmlData/Action/MN_RPBF_01-1.loa `
  --output Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json
```

receipt에는 개인 절대 경로가 저장되지 않는다. source build ID, LPK/DB/action SHA-256, extractor
SHA-256, table/key/column, source value, transform, result value만 저장한다. extractor는 level 10 row를 우선하고,
level 10이 없을 때 후보 전체의 SecondaryKey가 `{1}`인 fixed definition만 허용한다. 다른 fallback은 실패다.

## 5. Character 튜닝 방법

`Players`에서 class를, `Skills`에서 stable skill ID와 slot 행을 고른다. 일반 panel은
scalar 수치를 편집한다. combo stage 배열, stance/identity 정책과 패턴 구조를 재작성하지 않는다.

- `Basic stats`: HP, 공격력, 방어력, resource pool/regen
- `Movement`: 이동 속도
- `Skills`: input slot별 cooldown, resource cost, damage rate, action/hit time, range, skill movement
- COMBO/HOLD/COUNTER의 단계별 timing 구조는 기존 전용 authoring 계약에서 편집하며 공용 panel은 보존한다.

COMBO의 `hitTimeMs`는 damage 시점이다. non-final stage의 input window가 non-zero인 manual COMBO에서 `comboAdvanceMs`는 필수 caster hit와 projectile spawn이 끝난 뒤 buffered BA가 다음 stage로 갈 수 있는 가장 이른 시점이고, pending MOVE/SKILL은 이 전진을 막아 현재 `actionDurationMs` 종료 뒤 commit된다. non-final stage가 `inputOpenMs/inputCloseMs == 0/0`이면 automatic COMBO이며 `comboAdvanceMs == actionDurationMs`를 반드시 만족해야 한다. automatic chain은 추가 입력 없이 full-motion 경계마다 전진하고 pending MOVE/SKILL은 마지막 stage 종료 뒤 commit된다. 마지막 stage도 `comboAdvanceMs == actionDurationMs`와 닫힌 input window를 유지한다. 차원술사 `2050010`은 `1500/1067/1700ms` automatic 3-stage다. Balance Tool은 현행 94개 skill의 `ACTIVE/COMBO/HOLD/COUNTER/STANDUP`, player identity 필드, skill `identityCost`, Valtan `introPatternId/serverMotion`을 함께 무손실로 보존해야 한다.

Animation clip 선택은 Balance Tool이 아니라 Animation Tool에서 한다. Balance Tool의 combo stage 수와
Animation Tool의 BA clip 수는 같아야 하며, 실제 stage 진행은 Server snapshot `iComboStage`가 정한다.
Effect/Collider 담당자는 `skillId`, `actionId`, hit timing과 `effectId`를 stable 연결점으로 사용하고
damage 정답을 Client notify에서 만들지 않는다.

## 6. Valtan 튜닝 방법

공용 `Bosses`는 Retail HP/AP/health bars 및 base BossProfiles의 collision/detection/move scalar를 소유 문서에 저장한다.
Valtan 패턴 편집은 owning Boss/Animation/Effect Tool이 여는 `Valtan Authoring`을 사용하며
joined source, immutable revision과 Hot Reload 계약은 기존대로 유지한다.

- `Base stats`: `BossProfiles.json`의 HP, attack power, collision radius, detection/movement를 편집한다.
- `Decision / Pattern / Stage`: `Valtan.gameplay.json`의 selection window/set, weight, eligibility, duration,
  hit, branch, motion, volley를 편집한다.
- `Combat Object`: `Valtan.combatobjects.json`의 object-owned geometry/damage를 편집한다.
- `Presentation`: `Valtan.presentation.json`의 animation occurrence, Effect/camera invocation은 read-only로 표시하고
  owning Animation/Effect/Camera Tool로 안내한다.
- `World / Legacy`: `Valtan.worldeventsets.json` membership과 `Valtan.legacy-compatibility.json` closure는
  read-only다. generated Product를 편집 대상으로 열지 않는다.

`Resolved Scale` lane도 read-only다. boss row에는 `BossCatalog.json`의 `presentationScale: 1.0`을, cue row에는
presentation source의 `scalePolicy`와 resolved scale을 표시한다. `OWNER_RELATIVE`는 actor scale을 상속하고,
`GAMEPLAY_FOOTPRINT`와 `ARENA_ABSOLUTE`는 actor scale을 제거한 뒤 authored `worldScale`을 사용한다. Balance Tool이
이 lane에서 `BossCatalog.json`, `Valtan.presentation.json`, `Data/Effects/Authored`를 저장하거나 Effect element를
일괄 축소하지 않는다. 특히 sky-axe geometry와 속도·회전·decal은 Effect V1 저작 데이터와 Effect Tool이 소유한다.

플레이어 방어력은 이제 실제 incoming damage에 사용한다. 공식 Server 감산식은 client payload에 없어서
다음 식은 `PROJECT_TUNED` 계약이다.

```text
raw = max(1, attackPower * damageRatePercent / 100)
applied = max(1, raw * 100 / (100 + playerDefense))
```

현재 발탄 raw 350은 창술사 defense 105에서 170으로 적용된다. outgoing player damage에는 아직 boss
defense가 없으므로 Tool이 boss 방어력을 표시하거나 가정하지 않는다.

## 7. Live Verification

공용 Balance Test 아래의 HP/tick과 Valtan authoring 오른쪽 진단은 JSON 예상값이 아니라
`CCombatHUDViewModel`의 Server snapshot을 읽는다. 최근 damage event 상세는 기존 Valtan 진단에 남는다.

- player HP/resource/server tick
- boss HP/phase/action
- 최근 128개 중 최신 16 damage event (`OUT`/`IN`, amount, target entity)

검증할 때는 Server와 Client를 함께 실행하고 실제 스킬 또는 발탄 hit를 발생시킨다. F1 숫자 저장은
`SAVED AND APPLIED`와 새 numeric snapshot을 확인한 뒤 재시작 없이 live event를 비교한다. 외부 publisher로
게시한 패턴 구조는 해당 저작 경로의 Apply 또는 Server 재시작이 끝난 뒤 비교한다.

## 8. Map data와의 연결

`Data/Worlds/<AreaId>/Gameplay.world.json`은 formatVersion 2다. actor placement와 별도로
`triggerBox`와 `destroyable` 구조가 `CWorldGameplayDocument`에 추가되었다.

- `triggerBox`: position/yaw/halfExtents/triggerOnce/events
- `destroyable`: `deployRuntimePlacementId`(decimal string)/initialState
- event: `setCondition(bool)` 또는 `setDestroyableState(INTACT/FRACTURED/DESPAWNED)`

64-bit deploy ID는 JSON number로 저장하면 double 정밀도를 잃으므로 decimal string으로 저장한다.
현재 PR에서는 네 Area의 v2 migration과 parse/validate/atomic save 구조까지만 admission한다. 기존
`Publish-WorldGameplay.ps1`과 제품 Server는 actor placement만 허용하므로 trigger/destroyable을 실제
문서에 넣으면 publish가 실패한다. 이 fail-closed 경계는 의도적이며, Shared replication, Server trigger
authority, dynamic navigation, Client deploy presentation까지 수직으로 붙기 전에는 제거하지 않는다.

## 9. 검증 명령

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-BalanceRuntimeSet.ps1 -Mode Validate
Server/Bin/Debug/Server.exe --contract-test
```

완료 보고에서는 자동 검증과 수동 F1 smoke를 분리한다. Client를 실행하지 않았다면 Balance Test의
시각/입력/저장 smoke를 PASS라고 쓰지 않는다.


## Madness 수치 계약

정본은 `Data/Balance/Profiles/Retail.balanceprofile.json`의 단일 `madness` row이며 stable ID는
`policyId=KOUKUSAYDON`이다. `madnessGaugeAddPercent=-1`은 패턴의 명시 광기 결과를 그대로 게시한다.
Retail과 생성기의 기본값은 `-1`이며, `0..100`은 양수로 저작된 결과에 대한 전역 override다.
이 필드는 피해/공/인형 배율로 재사용하지 않는다. 공용 scalar draft와 Save/Validate/Publish 절차는 같다.

`MADNESS_GAUGE_ADD_PERCENT`는 `0..100`을 허용한다. 피해 결과와 함께 둔 `0`은 자동 HP 비례 광기까지
억제하는 명시적인 피해 전용 판정이며, 전역 override로도 양수로 바꾸지 않는다. 양수 명시값도 같은
판정의 자동 광기와 중복되지 않는다. 피해가 무적이나 보호막으로 전부 막히면 해당 명시 광기도
증가하지 않는다. 명시 광기 결과가 없는 피해는 기존 자동 증가를 유지한다. 최대 HP 피해 결과의
percent는 계속 `1..100`만 허용한다.

현재 저작값 보존은 불뿜기 `1%` 8창, Mario 공·인형 `5%` 20창, 불뿜기 잔류 장판 `0%` 21창과
기존 P6/P7 실패·시간초과 penalty `50%` 11개 결과 슬롯을 포함한다. `-1` 전환은 이전 전역 `0`이
지우던 P6/P7의 저작 penalty도 복구한다.

| 필드 | 초기값 | 의미/검증 |
|---|---:|---|
| damageGainPercent | 100 | HP 손실 대비 광기 변환 배율, 0~10000% |
| ballGainPercent | 10 | 공 접촉 1회당 최대 광기 대비 기본량, 0~100% |
| ballMultiplierPercent | 200 | 공 기본량에 곱할 배율, 0~10000% |
| ballRadiusM | 2.0 | 공 접촉 원 반경, 0 초과~20m |
| dollGainPercent | 10 | 인형 접촉 1회당 최대 광기 대비 기본량, 0~100% |
| dollMultiplierPercent | 200 | 인형 기본량에 곱할 배율, 0~10000% |
| dollRadiusM | 4.0 | 인형 양쪽 30도 sector 반경, 0 초과~20m |
| specialIntervalMs | 1000 | 살아 있는 특수 오브젝트의 접촉 간격, 100~60000ms |

Server 계산은 다음과 같다. Percent 100은 x1, 200은 x2다.

```text
damageGain = maximumGauge × actualHpLost / maximumHp × damageGainPercent / 100
sourcePulse = maximumGauge × sourceGainPercent / 100 × sourceMultiplierPercent / 100
total = clamp(previous + damageGain + sourcePulses + authoredMechanicGain, 0, maximumGauge)
```

실제 HP 손실은 방어/피해 버프·보호막·죽음 방지 뒤의 감소량이다. 무효 피격, 무적, 보호막으로
전부 흡수한 피격은 피해 충전을 만들지 않는다. 소수 잔여값은 다음 피격/접촉까지 누적하며 변신·
부활·관문 초기화 시 지운다. 최대 도달 시 기존 Server 변신 policy를 사용하고 이미 CLOWN인
플레이어는 추가 충전하지 않는다. 쿠크 외 월드는 피해 배율을 0으로 둔다.

공/인형 충전은 기존 damageable WORLD cue의 위치와 파괴 수명을 사용한다. 공은 원, 인형은
저장된 placement yaw의 앞/뒤 30도 sector를 Shared collision primitive로 검사한다. 높이 2m를
벗어난 플레이어, 마리오 내부, 사망·낙하·잡힘·binding 상태는 제외한다. 자연 pattern 종료 후
살아 있는 오브젝트는 유지하고 파괴/취소/소유 보스 사망으로 충전을 끝낸다. 실제 회전하는
mouth bone·화염 mesh 전체와 일치하는 collider 복원은 포함하지 않는다.

근거는 원본 설치 LPK의 `ZoneContentsGauge[3708100]`(최대 100, 자동충전 0, hold 15000ms),
`SkillBuff[4219994] -> SkillEffect[421991716] -> SkillEffect[421990117]`(NPC aura +10,
1000ms, 원형 200cm), `MN_CDMD_00.loa att_battle_2_01 -> SkillEffect[422230501/502/504]`
(인형 공격 400cm/30도)다. 피해 비례 변환 100%, 인형 광기 +10%, 공/인형 200%는
PROJECT_TUNED다. 후자는 사용자의 증가 요청에 따른 초기값이며 원작의 정확한 계수라고
주장하지 않는다. F1 panel에서 이 구분과 계산식을 함께 읽을 수 있다.

Gameplay bootstrap v37의 publisher는 `KOUKUMADNESS` row의 최대/hold 뒤에 위 8개 수치를 순서대로
기록한다. v37은 base 비교의 4-column row와 Retail의 12-column row만 허용하며 v36 이하를
generation admission에서 거부한다. Server/Client는 Shared version/max-row(65536) 계약을 공유하고,
Valtan generation parser도 같은 값을 사용한다. Server는 두 row variant를 검증하며 Sequence 메모리 draft가 활성
Balance Test 수치를 projector 기본값으로 되돌리지 않도록 현재 수치 배율을 보존한다.
외부 publisher로 게시한 파일은 Server 재시작으로 적용한다. F1 `Save + Apply`의 Madness 숫자는
Server 활성화 응답과 numeric snapshot 뒤 재시작 없이 반영되며 Client는 Server 상태를 소비한다.
