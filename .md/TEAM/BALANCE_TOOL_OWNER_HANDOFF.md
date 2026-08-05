# F1 Balance Tool · 공식 데이터 provenance 인수인계

작성일: 2026-08-05
대상: Gameplay/Balance, UI, Boss, Animation/Effect, Server 담당자

## 1. 지금 바로 기억할 결론

Debug Client에서 `F1 -> Balance Tool`을 열면 왼쪽에서 다섯 character 또는 Valtan을 고르고,
가운데에서 수치를 편집하고, 오른쪽에서 실제 Server snapshot과 최근 damage event를 확인할 수 있다.
스킬 기준은 level 10이다. 단, 원본 table이 SecondaryKey 1만 가진 fixed basic/awakening definition은
명시적으로 level 1 row를 사용하며 Tool에서 source level을 바꾸지 않는다.

저장 흐름은 다음 한 방향이다.

```text
F1 Balance Tool draft
-> Data/Balance + Data/Encounters authoring JSON atomic staging
-> Update-BalanceProvenanceReceipt.ps1
-> 바뀐 field만 PROJECT_TUNED로 분류
-> Publish-BalanceRuntimeSet.ps1 -Mode Validate
-> Publish Server Data
-> gameplay bootstrap + world bootstrap 4종을 한 rollback set으로 promotion
-> Server.exe 재시작
-> Server snapshot / damage event로 실측
```

런타임 hot reload는 없다. `Publish Server Data` 뒤 Server를 재시작해야 적용된다. Client만 JSON을
다시 읽어서 Server와 다른 수치를 보여주는 경로는 만들지 않는다.

## 2. 정본 파일

| 역할 | 정본 |
|---|---|
| player HP/resource/AP/defense/move | `Data/Balance/PlayerProfiles.json` |
| class/slot/skill/timing/range/combo | `Data/Balance/PlayerSkills.json` |
| attack power에 곱하는 damage rate | `Data/Balance/DamageProfiles.json` |
| Valtan HP/AP/detection/movement/phase threshold | `Data/Balance/BossProfiles.json` |
| Valtan state/action/pattern timing/range | `Data/Encounters/Valtan/ValtanEncounter.json` |
| field-level 출처와 변환 | `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json` |
| 재추출기 | `Tools/GameplayPipeline/Export-OfficialBalanceReceipt.py` |
| Tool 편집 후 receipt 동기화 | `Tools/GameplayPipeline/Update-BalanceProvenanceReceipt.ps1` |
| domain 검증·cook | `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`, `Tools/WorldPipeline/Publish-WorldGameplay.ps1` |
| Balance/World 통합 promotion | `Tools/GameplayPipeline/Publish-BalanceRuntimeSet.ps1` |

Server가 읽는 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`은 생성물이다. 직접 편집하지 않는다.

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

왼쪽 `Players`에서 class를 고른다.

- `Basic stats`: HP, 공격력, 방어력, resource pool/regen
- `Movement`: 이동 속도
- `Skills`: input slot별 cooldown, resource cost, damage rate, action/hit time, range, skill movement
- `Basic attack combo stages`: BA 단계별 duration/hit/input open/input close

Animation clip 선택은 Balance Tool이 아니라 Animation Tool에서 한다. Balance Tool의 combo stage 수와
Animation Tool의 BA clip 수는 같아야 하며, 실제 stage 진행은 Server snapshot `iComboStage`가 정한다.
Effect/Collider 담당자는 `skillId`, `actionId`, hit timing과 `effectId`를 stable 연결점으로 사용하고
damage 정답을 Client notify에서 만들지 않는다.

## 6. Valtan 튜닝 방법

왼쪽 `Bosses -> 발탄`에서 다음을 분리해 본다.

- `Base stats`: HP, attack power, collision radius
- `Detection and movement`: engage distance, move speed
- `Phase`: phase 2 HP threshold
- `Patterns`: range, telegraph/active/recovery, damage rate, animation action ID

현재 Server 행동은 nearest alive player 감지 → chase → 8m 이내 windup → active 시작 tick에 radial 2D
1회 hit → recovery다. min range는 아직 행동 선택에 사용되지 않고, phase 2는 snapshot phase byte만
변경하며 별도 패턴 목록을 선택하지 않는다. Tool은 이 사실을 잠긴 진단 문구로 보여준다.

플레이어 방어력은 이제 실제 incoming damage에 사용한다. 공식 Server 감산식은 client payload에 없어서
다음 식은 `PROJECT_TUNED` 계약이다.

```text
raw = max(1, attackPower * damageRatePercent / 100)
applied = max(1, raw * 100 / (100 + playerDefense))
```

현재 발탄 raw 350은 창술사 defense 105에서 170으로 적용된다. outgoing player damage에는 아직 boss
defense가 없으므로 Tool이 boss 방어력을 표시하거나 가정하지 않는다.

## 7. 오른쪽 Live Verification

오른쪽은 JSON 예상값이 아니라 `CCombatHUDViewModel`의 Server snapshot을 읽는다.

- player HP/resource/server tick
- boss HP/phase/action
- 최근 128개 중 최신 16 damage event (`OUT`/`IN`, amount, target entity)

검증할 때는 Server와 Client를 함께 실행하고 실제 스킬 또는 발탄 hit를 발생시킨다. 저장 후 Server를
재시작하지 않았으면 새 authoring과 live event를 비교하지 않는다.

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
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
```

완료 보고에서는 자동 검증과 수동 F1 smoke를 분리한다. Client를 실행하지 않았다면 Balance Tool의
시각/입력/저장 smoke를 PASS라고 쓰지 않는다.
