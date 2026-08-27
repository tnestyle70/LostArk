# 발탄 HIGH_JUMP 혼합 도끼 3웨이브 구현 계획

## 1. 목표와 시간 계약

`VALTAN_HIGH_JUMP/AIRBORNE`의 전체 Server duration을 `4000ms`로 고정한다. 이 한 구간 안에서
`0ms`, `1333ms`, `2666ms`에 총 세 웨이브를 생성한다. 각 웨이브는 다음 두 묶음을 하나의
Server transaction으로 준비하고, 어느 한 묶음이라도 검증에 실패하면 해당 웨이브 전체를 생성하지 않는다.

1. 웨이브 시작 시점의 생존·전투 가능·비 `DEAD/FALLING` 플레이어마다 추적 도끼 1개
2. 발탄의 저작된 아레나 중앙 스폰 좌표를 기준으로 반경 `14m` 안의 랜덤 도끼 4개

`3번 반복`은 4초짜리 구간을 세 번 재생하는 12초가 아니라, 사용자가 말한 4초 duration 안의
총 세 웨이브로 해석한다. 마지막 웨이브의 기존 `1200ms` 타격은 약 `3867ms`에 발생해 4초 안에
들어온다.

## 2. 정본과 생성물

| 구분 | 경로 | 역할 |
|---|---|---|
| 정본 | `Data/Valtan/Valtan.gameplay.json` | AIRBORNE 4000ms, 혼합 volley 하나와 3회 spawn schedule |
| 정본 | `Data/Valtan/Valtan.combatobjects.json` | 공용 도끼 hit geometry와 resolved-position origin |
| 정본 | `Data/Actors/BossCatalog.json` | 기존 target-axe의 stable client visual/effect join |
| 생성물 | `Data/Encounters/Valtan/ValtanEncounter.json` | strict typed stage actions |
| 생성물 | `Data/Encounters/Valtan/ValtanCombatObjects.json` | Server combat-object product |
| 생성물 | `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap` | publisher가 생성하는 Server 입력 |

현재 worktree의 `manualAuditions`와 `VALTAN_SEQUENCE_*` 변경은 다른 작업이다. projector와 publisher를
사용하더라도 해당 row와 ordinal을 그대로 보존하고, HIGH_JUMP 관련 hunk만 변경한다.

## 3. 데이터와 런타임 계약

1. 기존 target-axe `SPAWN_COMBAT_OBJECT_VOLLEY` 하나에
   `spawnSchedule { kind, count, firstOffsetMs, intervalMs }`와
   `arenaRandom { kind, anchor, count, radiusM, heightToleranceM }`를 추가한다.
   `count`는 첫 ENTER 웨이브를 포함하며, `count > 1`이면 `intervalMs > 0`이어야 한다. 마지막
   spawn offset은 owner stage duration보다 작아야 한다.
2. 추적 묶음은 기존 `PER_ALIVE_PLAYER + TARGET_CENTER`를 유지하고, 같은 event의 arena supplement가
   `RANDOM_NAVIGABLE_CIRCLE(radiusM: 14)` 네 점을 더한다. 공용 schedule은
   `{ kind: INTERVAL, count: 3, firstOffsetMs: 0, intervalMs: 1333 }`이다.
3. 기존 `combatobject.valtan.high-jump.target-axe` archetype과 sky-axe Effect asset을 재사용한다.
   combat-object spawn context를 `resolved pose + optional locked target`으로 일반화하여 player 도끼만
   첫 pulse까지 추적하고 arena 도끼는 같은 visual/damage를 가진 고정 world pose로 만든다. 가짜 player
   target이나 현재 boss pose를 spawn origin으로 사용하지 않는다.
4. 아레나 중앙은 navigation projection 뒤 보존된 `boss.fSpawnPosition*`를 사용한다. 랜덤 후보는
   stateless deterministic hash로 생성하고 중심점의 `Is_PointWalkableExact`와 중앙 높이 차이
   `1m` 이하를 통과해야 한다. 실제 발탄 nav paint에는 위험 원 안의 의도된 비보행 seam이 있으므로
   공격 반경 전체를 walkable로 강제하지 않는다. 네 랜덤 도끼는 서로 최소 `7m` 떨어져야 한다.
5. 동일 pattern sequence, wave ordinal, action identity는 동일 후보 순서를 만든다. 재현성과 테스트를
   위해 process RNG나 wall clock은 사용하지 않는다.
6. 30Hz fixed tick에서 후속 웨이브는 stage start 기준 40/80 tick에 적용한다. 같은 stage의
   repeat-enabled volley들은 하나의 transaction으로 stage/commit한다.
7. 플레이어 수가 0이면 추적 묶음만 비고 랜덤 도끼 4개는 계속 생성한다. 용량, navigation,
   spacing 또는 catalog join 실패는 부분 생성 없이 mechanic failure 경계로 전달한다.

## 4. 소비자와 구현 파일

- `Tools/ValtanPipeline/valtan_tuning_pipeline.py`: split authoring validation, Product projection,
  combat-object origin projection, draft read/write 호환
- `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`: 새 volley row field, 단일 HIGH_JUMP owner,
  새 origin/layout/policy strict validation
- `Server/Public|Private/GameplayCatalog.*`: bootstrap parse와 admission
- `Server/Public|Private/CombatObjectRuntime.*`: resolved pose와 optional target object staging
- `Server/Private/GameRoom.cpp`, `Server/Public/GameRoom.h`: fixed-tick repeat scheduler,
  deterministic arena candidate resolution, atomic wave commit
- `Client/Private/ValtanPatternTree.cpp`, `Client/Private/EncounterPatternReference.cpp`,
  `Client/Private/BalanceTool.cpp`: 새 source/Product field를 보존하고 기존 target-axe 편집은 유지
- focused Python tests와 `ServerGameplayContractTests.cpp`: exact schema, deterministic points,
  3-wave count/timing, alive filtering, rollback, capacity 검증

## 5. 검증 계획

- 변경 JSON parse, `git diff --check`
- `test_valtan_pattern_master_v2.py`, `test_valtan_pattern_tree_contract.py`, 관련 Balance Tool test
- `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate`
- `Publish-GameplayBalance.ps1 -Mode Validate`
- Server x64 Debug/Release build와 `Server.exe --contract-test`
- Client x64 Debug/Release build

자동 검증은 생성 수, 좌표 계약, timing, replication 준비만 증명한다. Effect Tool/Arena에서 빨간 원의
확장 속도·농도와 폭발 visual fidelity는 사용자가 직접 확인하며, 이번 변경은 사용자가 편집 중인
`effect.valtan.sky-axe.active.effect.json` element 값을 덮어쓰지 않는다.
