# 2026-09-04 발탄 돌 Effect V1 lane 복원 · 패턴별 돌 문서 분리 · 생성 위치 로직 분리 구현 계획서

브랜치 `GB/KoukuSaydon-Main-Pattern`, HEAD `8f8e5bea`. 작업 시작 시 worktree에는 사용자의
Effect 문서 편집(`ground-roar.rock.active`, `dash-charge.windup-telegraph`, `sequence.warp.portal`,
`sequence.cross`)과 Composition publish 생성물이 미커밋 상태로 있었다. 이 변경은 보존한다.

## 1. 목표와 종료 증거

| # | 요청 | 종료 증거 |
|---|---|---|
| 1 | 땅구르기 후 사자후 / 피자 / 발악 돌을 V1 문서 lane으로 되돌린다 | `BossCatalog.json` 세 archetype에 `effectV2Group`이 없고 `CClientReplication::Spawn_CombatObjectPresentation`이 `Spawn_WorldRoot`로 진입한다 |
| 2 | Server despawn(`lifetimeMs`)이 V1 Effect를 끊지 않고 문서의 자연 종료까지 재생한다 | `Apply_Despawn`이 `Release`를 호출하고 V1 handle에 `Stop_WorldRoot`를 호출하지 않는다. Tool preview도 `iLifetimeMs`에서 active root를 끊지 않는다 |
| 3 | 세 패턴이 서로 다른 돌 Effect 문서를 소유한다 | `effect.valtan.{ground-roar,six-pizza,struggling}.rock.{active,explode}` 6개 문서와 catalog 항목이 존재하고 All Effects `Open Editor`가 서로 다른 경로를 연다 |
| 4 | 생성 위치를 패턴별 로직으로 구분한다 | 땅구르기·발악 `BOSS_RELATIVE` 반지름 `4.5·√2`, 피자 `ARENA_CENTER` 반지름 `9·√2`가 Server bootstrap과 Server contract test에서 확인된다 |
| 5 | 통합 update bat 실행 | `RunFullPipeline.bat`(Debug / FullDiagnostic) PASS |

## 2. 현재 실측

- [BossCatalog.json:64-93](../../../Data/Actors/BossCatalog.json)의 세 돌 archetype이 `effectAssetId`(V1)와
  `effectV2Group=boss.valtan.rock-pillar.sequence`를 함께 선언한다.
  [ActorCatalog.cpp:427](../../../Client/Private/ActorCatalog.cpp)이 `effectV2Group`을 보면
  `activeEffectKind=EFFECT_V2_GROUP`으로 바꾸고,
  [ClientReplication.cpp:1987-2024](../../../Client/Private/ClientReplication.cpp)가 V2 group을 재생한다.
  Effect Tool의 `Play Combat Object Lifecycle`은 [Valtan.cpp:1459-1483](../../../Client/Private/Valtan.cpp)에서
  V1 `effectAssetId`/`hitEffectAssetId`만 재생한다. 두 lane이 달라 사용자가 편집한 V1 문서가 Server 재생에
  반영되지 않았다.
- [CombatObjectProjectionRuntime.h](../../../Client/Public/CombatObjectProjectionRuntime.h) `Apply_Despawn`은
  `sink.Stop`을 호출하고 [ClientReplication.cpp:2107-2117](../../../Client/Private/ClientReplication.cpp)이
  `CEffectPresentationService::Stop_WorldRoot`로 즉시 제거한다. `Spawn_WorldRoot`는
  `EFFECT_STOP_POLICY::NATURAL`로 spawn하므로 Stop을 생략하면
  [Effect_PresentationService.cpp:4762-4770](../../../Client/Private/Effect_PresentationService.cpp)의
  `Is_Finished()`/owner 소멸/level 변경 경로가 제거를 담당한다.
- Server 생성 위치는 [GameRoom.cpp:10047-10054](../../../Server/Private/GameRoom.cpp)가 boss 현재 pose와
  yaw 기준 `45 + 90n`도, `radiusM`으로 계산한다. `BOSS_COMBAT_OBJECT_VOLLEY_POLICY`는
  `NONE/PER_ALIVE_PLAYER/BOSS_RELATIVE`뿐이다. 아레나 중앙은 `boss.fSpawnPosition*`
  (placement `boss.valtan.center` = `156.03, 22.99751, -122.06`)이며 `Resolve_ArenaCenter`가 같은 값을 쓴다.
  피자 패턴 `serverMotion.landingPosition`도 같은 좌표다.
- Server `lifetimeMs`와 explode `atMs`는 [Valtan.combatobjects.json](../../../Data/Valtan/Valtan.combatobjects.json)이
  archetype별로 이미 분리(6200 / 5000)하고 있다. 공유는 BossCatalog visual join과 Effect 문서뿐이다.

## 3. 변경 파일

### 3.1 데이터 (G00)

| 파일 | 변경 |
|---|---|
| `Data/Effects/Authored/effect.valtan.six-pizza.rock.active.effect.json` | 신규. `ground-roar.rock.active` 복제, ID·표시명·element id를 `six-pizza`로 치환 |
| `Data/Effects/Authored/effect.valtan.six-pizza.rock.explode.effect.json` | 신규. `ground-roar.rock.explode` 복제 |
| `Data/Effects/Authored/effect.valtan.struggling.rock.active.effect.json` | 신규. `struggling` 치환 |
| `Data/Effects/Authored/effect.valtan.struggling.rock.explode.effect.json` | 신규 |
| `Data/Effects/EffectCatalog.json` | 위 4개 `DIRECT_AUTHORED_DOCUMENT` 항목 추가 |
| `Data/Actors/BossCatalog.json` | 세 archetype `effectV2Group` 제거, six-pizza/struggling `effectAssetId`/`hitEffectAssetId`를 전용 문서로 교체 |
| `Data/Valtan/Valtan.gameplay.json` | 땅구르기·발악 `radiusM=6.3639610307`, 피자 `volleyPolicy=ARENA_CENTER`, `layout.kind=RADIAL_AROUND_ARENA_CENTER`, `radiusM=12.7279220614` |
| `Data/Valtan/Valtan.presentation.json` | 피자·발악 independentEffect `displayName` 거리 표기 갱신 |
| `Tools/ValtanPipeline/author_valtan_phase_two_mechanics.py` | 재저작 스크립트의 돌 volley 값을 저장본과 일치시킴 |
| `.md/GB/gotchas.md` | `Client/Bin/Resources` 삭제 금지 항목 추가 |

`Data/Effects/V2/Groups/boss.valtan.rock-pillar.sequence.effectv2group.json`과 leaf 두 문서는 Product가 더
이상 참조하지 않지만 삭제하지 않는다. V2 authoring 자산이며 `Independent.json`이 나열한다.

### 3.2 Client V1 lane 자연 종료 (G01)

| 파일 | 변경 |
|---|---|
| `Client/Public/CombatObjectProjectionRuntime.h` | `Apply_Despawn`이 `sink.Release(handle)` 호출. `Remove_Source`/`Reset`은 `sink.Stop` 유지 |
| `Client/Public/ClientReplication.h` | sink에 `Release`, `CClientReplication::Release_CombatObjectPresentation` 선언 |
| `Client/Private/ClientReplication.cpp` | `Release_CombatObjectPresentation`: V2 group은 `Stop_Group`, V1 world root는 NATURAL 종료에 맡김 |
| `Client/Public/Valtan.h` | `LOCAL_PATTERN_COMBAT_OBJECT_INSTANCE::bActiveAttempted`, `LOCAL_PATTERN_COMBAT_OBJECT_TEMPLATE::bArenaCenterOrigin` |
| `Client/Private/Valtan.cpp` | preview active root를 `iLifetimeMs`에서 끊지 않고 자연 종료(seek 실패)를 소비 처리. rewind 시 flag 초기화 |
| `Client/Private/Effect_Tool.cpp` | All Effects 행에 Product lane(V1/V2 group) 표시, lifecycle 설명 문구를 자연 종료로 갱신, `ARENA_CENTER` 허용 |

### 3.3 `ARENA_CENTER` volley policy (G02)

| 파일 | 변경 |
|---|---|
| `Server/Public/GameplayCatalog.h` | `BOSS_COMBAT_OBJECT_VOLLEY_POLICY::ARENA_CENTER` 추가(주석: boss spawn placement 기준, world-absolute 각도) |
| `Server/Private/GameplayCatalog.cpp` | bootstrap 파서 `fields[7]`에 `ARENA_CENTER` 허용, BOSS_RELATIVE와 같은 topology 검증, origin `BOSS_POSITION` join |
| `Server/Private/CombatObjectRuntime.cpp` | policy 검증 허용, origin을 `boss.fSpawnPosition*`으로, 각도 basis 0도 |
| `Server/Private/GameRoom.cpp` | `arenaCenterVolley` 분기: origin/각도 basis 선택, 기존 off-navigation 예외 유지 |
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | `$isArenaCenter` topology |
| `Tools/ValtanPipeline/valtan_tuning_pipeline.py` | policy 허용, layout `RADIAL_AROUND_ARENA_CENTER` 검증·투영(`RADIAL`), draft patch validator |
| `Client/Private/ValtanPatternTree.cpp` | split source 파서와 Product 파서에 `ARENA_CENTER`/`RADIAL_AROUND_ARENA_CENTER` |
| `Client/Private/EncounterPatternReference.cpp` | Product 참조 파서 허용 |
| `Client/Private/Valtan.cpp` | local preview: `ARENA_CENTER` template은 `m_LocalPreviewArenaCenterAnchors[pattern]`를 origin으로, yaw basis 0 |

### 3.4 계약 테스트 (G03)

| 파일 | 변경 |
|---|---|
| `Server/Private/ServerGameplayContractTests.cpp` | 땅구르기 반지름 `6.3639610307f`, 지연 돌 case에 policy/origin basis 추가 |
| `Tools/ValtanPipeline/test_valtan_rock_pillar_group_contract.py` | 패턴별 geometry(4.5 / 9.0 arena center / 4.5), V2 group 공유 테스트를 V1 전용 문서 테스트로 교체 |
| `Tools/EffectPipeline/test_valtan_model_view_composition.py` | BossCatalog literal |
| `Tools/ValtanPipeline/test_valtan_combat_object_hit_effect_presentation_contract.py` | visual literal, 반지름, `sink.Release`, preview 토큰 |
| `Tools/ValtanPipeline/test_action_composition_workbench_regression_oracles.py` | 반지름 |
| `Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py` | policy tuple |

## 4. 데이터와 호출 흐름

### 4.1 V1 lane

```text
S2C_COMBAT_OBJECT_SPAWNED
→ CClientReplication::Spawn_CombatObjectPresentation
  → CActorCatalog::Find_BossCombatObjectVisual (activeEffectKind=EFFECT_V1)
  → CEffectPresentationService::Spawn_WorldRoot(effectAssetId, NATURAL)
S2C_COMBAT_OBJECT_PRESENTATION_EVENT(HIT_PULSE, atMs 5000)
→ CValtan::Apply_CombatObjectPresentationEvent → Spawn_WorldRoot(hitEffectAssetId)
S2C_COMBAT_OBJECT_DESPAWNED(lifetimeMs 6200)
→ CCombatObjectProjectionRuntime::Apply_Despawn → sink.Release → V1 handle 유지
→ CEffectPresentationService::Update가 Is_Finished()에서 제거
boss 제거 / Reset_World → Remove_Source / Reset → sink.Stop → Stop_WorldRoot
```

### 4.2 ARENA_CENTER

```text
Valtan.gameplay.json event {volleyPolicy: ARENA_CENTER, layout: RADIAL_AROUND_ARENA_CENTER}
→ valtan_tuning_pipeline (validate, project targetingPolicy=ARENA_CENTER layout=RADIAL)
→ ValtanEncounter.json → Publish-GameplayBalance.ps1 → PATTERNSTAGEVOLLEY ... ARENA_CENTER
→ CGameplayCatalog (ePolicy=ARENA_CENTER, origin BOSS_POSITION join)
→ CGameRoom::Apply_BossPatternStageActions / Apply_BossPatternScheduledSpawnWave
   origin = boss.fSpawnPosition, degrees = 0 + 45 + 90n
→ CCombatObjectRuntime::Stage_BossCombatObject (같은 origin/basis)
Client: ValtanPatternTree / EncounterPatternReference 파서 허용
        CValtan local preview origin = ServerMotion landing anchor (= arena center)
```

각도 basis: `BOSS_RELATIVE`는 boss yaw 상대, `ARENA_CENTER`는 world 절대다. 따라서 피자 돌은 boss 방향과
무관하게 arena center `(±9, ±9)`에 놓인다.

## 5. G별 구현 범위

- G00 데이터: 3.1 전체. 검증: `python -m json.tool`, `Validate-EffectSources`(FullDiagnostic 포함).
- G01 Client 자연 종료: 3.2 전체. 검증: Client 빌드, `test_valtan_combat_object_hit_effect_presentation_contract`.
- G02 ARENA_CENTER: 3.3 전체. 검증: Server contract test, `Publish-GameplayBalance.ps1` publish, pipeline 테스트.
- G03 테스트 갱신: 3.4 전체.
- G04 문서: 이 PLAN, RESULT, gotchas.
- G05 `RunFullPipeline.bat` 실행 후 RESULT 기록.

## 6. 검증

```powershell
$env:PYTHONPATH='.'; $env:PYTHONIOENCODING='utf-8'
python -m unittest Tools.ValtanPipeline.test_valtan_rock_pillar_group_contract `
  Tools.ValtanPipeline.test_valtan_combat_object_hit_effect_presentation_contract `
  Tools.EffectPipeline.test_valtan_model_view_composition `
  Tools.ValtanPipeline.test_valtan_pattern_tree_contract `
  Tools.ValtanPipeline.test_action_composition_workbench_regression_oracles `
  Tools.ValtanPipeline.test_valtan_ground_roar_client_loader_source_contract
RunFullPipeline.bat
```

수동 검증(사용자): Valtan Arena에서 땅구르기 후 사자후 / 피자 / 발악 `Complete Play Owner`. 돌이 V1 문서대로
생성·폭발·sprite particle까지 재생되고 6.2초 이후에도 끊기지 않는지, 피자 돌이 boss 위치와 무관하게 아레나
중앙 `(±9, ±9)`에 서는지 확인한다.
