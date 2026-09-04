# 2026-09-04 발탄 돌 Effect V1 lane 복원 · 패턴별 돌 문서 분리 · 생성 위치 로직 분리 RESULT

브랜치 `GB/KoukuSaydon-Main-Pattern`, HEAD `8f8e5bea`. 계획은
`2026-09-04_VALTAN_ROCK_EFFECT_LANE_AND_PATTERN_OFFSET_IMPLEMENTATION_PLAN.md`가 소유한다.
이 문서는 실제 diff와 실행 증거만 기록한다.

작업 시작 시 worktree에는 사용자의 Effect 문서 편집(`ground-roar.rock.active`,
`dash-charge.windup-telegraph`, `sequence.warp.portal`, `sequence.cross`)과 Composition publish 생성물이
미커밋 상태로 있었다. 이 변경은 손대지 않았다.

## 1. 원인 요약

- Server 재생(`Complete Play Owner`)은 `BossCatalog.json`의 `effectV2Group=boss.valtan.rock-pillar.sequence`를
  따라 `CEffectV2Runtime::Play_Group`으로 V2 leaf `boss.valtan.rock-pillar.active/explosion`을 재생했다.
  Effect Tool의 `Play Combat Object Lifecycle`은 V1 `effect.valtan.ground-roar.rock.active/.explode`만
  재생했다. 사용자가 편집한 V1 문서(sprite particle 5개, mesh timing)는 Server 경로에 존재하지 않았다.
- V2 mesh shader(`Shader_EffectV2_Common.hlsli`)는 mask 텍스처(`fx_d_fluid_020.dds`) R 채널을 alpha에 곱하고
  0.001 이하를 discard한다. Opaque 돌이 첫 프레임부터 뚫린 것처럼 보인 직접 원인이다.
- 세 돌 archetype이 같은 문서와 같은 V2 group을 공유해 한 패턴의 timing 편집이 다른 패턴에 그대로 전파됐다.

## 2. 요청과 반영 결과

| # | 요청 | 결과 |
|---|---|---|
| 1 | 선택 A: V1 lane 복원 | `BossCatalog.json` 세 돌 archetype의 `effectV2Group` 제거. `activeEffectKind=EFFECT_V1`로 `Spawn_WorldRoot` 경로 진입 |
| 2 | Server despawn이 Effect를 끊지 않게 | `CCombatObjectProjectionRuntime::Apply_Despawn`이 `sink.Release`를 호출하고 V1 world root는 `EFFECT_STOP_POLICY::NATURAL`로 자연 종료. Tool preview도 `iLifetimeMs`에서 active root를 끊지 않음 |
| 3 | 패턴별 돌 문서 분리 | `effect.valtan.six-pizza.rock.{active,explode}`, `effect.valtan.struggling.rock.{active,explode}` 신규. catalog 4항목 등록. All Effects `Open Editor`가 패턴별 경로를 연다 |
| 4 | 생성 위치 로직 분리 | 땅구르기·발악 `BOSS_RELATIVE` 반지름 `6.3639610307`(±4.5m). 피자 `ARENA_CENTER` 신규 policy, 반지름 `12.7279220614`(아레나 중앙 ±9m, world 절대 각도) |
| 5 | 통합 update bat | `-DataOnly` 실행 결과는 4절. 전체 빌드는 실행 중인 Debug `Client.exe`/`Server.exe`가 출력물을 잠가 사용자가 종료한 뒤 실행해야 한다 |
| 6 | gotchas에 Resources 삭제 금지 | `.md/GB/gotchas.md` merge 회귀 절에 항목 추가 |

## 3. 실제 변경

### 3.1 데이터

| 파일 | 변경 |
|---|---|
| `Data/Actors/BossCatalog.json` | ground-roar/six-pizza/struggling `effectV2Group` 제거, six-pizza/struggling `effectAssetId`/`hitEffectAssetId`를 전용 문서로 교체. part-break는 ground-roar 문서 유지 |
| `Data/Effects/Authored/effect.valtan.six-pizza.rock.active.effect.json` 외 3개 | 현재 편집본 ground-roar 문서 복제. `effectAssetId`, `displayName`, element id/groupId를 패턴 이름으로 치환 |
| `Data/Effects/EffectCatalog.json` | 4항목 추가 |
| `Data/Valtan/Valtan.gameplay.json` | ground-roar·struggling `radiusM=6.3639610307`, six-pizza `volleyPolicy=ARENA_CENTER`, `layout.kind=RADIAL_AROUND_ARENA_CENTER`, `radiusM=12.7279220614` |
| `Data/Valtan/Valtan.presentation.json` | 피자·발악 independentEffect displayName 거리 표기 |
| `Data/Effects/V2/Independent.json` | PR #305 merge로 들어온 미바인딩 V2 group 4개(`blackhole`, `breathe`, `breathe.red`, `six.sonic`) 등록. 이번 작업과 무관한 merge 공백이며 `effect.v2` domain이 HEAD 상태에서도 실패하던 것을 확인했다 |
| `Tools/ValtanPipeline/author_valtan_phase_two_mechanics.py` | 재저작 스크립트의 돌 volley 값·displayName을 저장본과 일치 |
| `.md/GB/gotchas.md` | `Client/Bin/Resources` 삭제 금지 항목 |

Product 투영(`Data/Encounters/Valtan/*`, `Client/Bin/DataFiles/Compositions/*`)은 pipeline이 다시 생성했다.
`Data/Effects/V2/Groups/boss.valtan.rock-pillar.sequence.effectv2group.json`과 leaf 두 문서는 Product가 더 이상
참조하지 않지만 V2 authoring 자산이므로 삭제하지 않았다.

### 3.2 Client

| 파일 | 변경 |
|---|---|
| `Client/Public/CombatObjectProjectionRuntime.h` | `Apply_Despawn` → `sink.Release`. `Remove_Source`/`Reset`은 `sink.Stop` 유지 |
| `Client/Public/ClientReplication.h`, `Client/Private/ClientReplication.cpp` | sink `Release`, `Release_CombatObjectPresentation`(V2 group만 `Stop_Group`, V1은 NATURAL 종료에 위임) |
| `Client/Public/Valtan.h` | `LOCAL_PATTERN_COMBAT_OBJECT_TEMPLATE::bArenaCenterOrigin`, `LOCAL_PATTERN_COMBAT_OBJECT_INSTANCE::bActiveAttempted` |
| `Client/Private/Valtan.cpp` | preview active root를 lifetime에서 끊지 않고 자연 종료를 소비. `ARENA_CENTER` template은 pattern의 landing anchor(= arena center)를 origin, yaw basis 0으로 사용. anchor 없는 ARENA_CENTER volley는 staging fail-closed |
| `Client/Private/ValtanPatternTree.cpp`, `Client/Public/ValtanPatternTree.h` | split source/Product 파서에 `ARENA_CENTER`, `RADIAL_AROUND_ARENA_CENTER` |
| `Client/Private/EncounterPatternReference.cpp` | Product 참조 파서 허용 |
| `Client/Private/Effect_Tool.cpp` | lifecycle preview에 `ARENA_CENTER` 허용, 행에 Product lane(V2 group override) 표시, 설명 문구를 자연 종료로 갱신 |

### 3.3 Server · pipeline

| 파일 | 변경 |
|---|---|
| `Server/Public/GameplayCatalog.h` | `BOSS_COMBAT_OBJECT_VOLLEY_POLICY::ARENA_CENTER` |
| `Server/Private/GameplayCatalog.cpp` | bootstrap `fields[7]` 파서, BOSS_RELATIVE와 같은 topology 검증, origin `BOSS_POSITION` join |
| `Server/Private/CombatObjectRuntime.cpp` | policy 허용, origin `boss.fSpawnPosition*`, yaw basis 0 |
| `Server/Private/GameRoom.cpp` | `arenaCenterVolley` 분기, 기존 off-navigation 예외 유지 |
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | `$isArenaCenter` topology |
| `Tools/ValtanPipeline/valtan_tuning_pipeline.py` | `RADIAL_LAYOUT_KIND_BY_VOLLEY_POLICY`, `PROJECT_TUNED_RADIAL_LAYOUT_KINDS`, 검증·투영·draft validator |

### 3.4 계약 테스트

| 파일 | 변경 |
|---|---|
| `Server/Private/ServerGameplayContractTests.cpp` | ground-roar 반지름 `6.3639610307f`, `DELAYED_ROCK_PILLAR_CASE::arenaCenter`로 origin/yaw basis 분기 |
| `Tools/ValtanPipeline/test_valtan_rock_pillar_group_contract.py` | 패턴별 geometry(9.0 arena center / 4.5), 공유 V2 group 테스트를 패턴별 V1 문서 테스트로 교체 |
| `Tools/EffectPipeline/test_valtan_model_view_composition.py` | BossCatalog literal |
| `Tools/ValtanPipeline/test_valtan_combat_object_hit_effect_presentation_contract.py` | visual literal, 반지름, `sink.Release`, preview 토큰, mesh `transformMotionDurationSeconds` oracle을 사용자 편집값 `1.0`으로 |
| `Tools/ValtanPipeline/test_action_composition_workbench_regression_oracles.py` | 반지름 |
| `Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py` | policy tuple |
| `Tools/ValtanPipeline/test_valtan_ground_roar_client_loader_source_contract.py` | 마커 `(!bPerAlivePlayer && !bBossRelative && !bArenaCenter)` |

## 4. 자동 검증

| 검증 | 결과 |
|---|---|
| `Project-ValtanPatternMaster.ps1 -Mode PublishV2` | PASS, `changed=1 artifacts=7`. Product에 `targetingPolicy=ARENA_CENTER`, `radiusM=12.7279220614` / `6.3639610307` 투영 확인 |
| Python 집중 계약 7 module | `Ran 134 tests OK` (rock pillar, hit-effect presentation, model view composition, pattern tree, ground-roar loader source, part-break recovery, workbench oracles) |
| Python 광역 gate 12 module | `Ran 216 tests`, failures=1 errors=1. 둘 다 `test_valtan_cross_rock_wave_effect`이며 사용자의 미커밋 `effect.valtan.sequence.cross` 편집(`transformMotionDurationSeconds` 제거, stone base 텍스처 교체)이 원인. 이번 변경과 무관 |
| `Validate-EffectSources.ps1` | exit 0, `directSourceCount=180` (신규 4문서 catalog 도달성 통과) |
| `validate_effect_v2.py` | HEAD BossCatalog로도 `unbound Effect V2 authored effects` 실패 재현 → `Independent.json` 등록 후 `131 authored, 139 bindings, 16 groups, 13 independent` PASS |
| MSBuild `ClCompile` Debug x64 (link 없음) | Server `SERVER_EXIT=0`, Client `CLIENT_EXIT=0`, error 0. C4819 경고는 기존 `ClientReplication.*` UTF-8 파일의 기존 경고 |
| `Run-FullPipeline.ps1 -DataOnly` | PASS, 24.4 s. PublishV2 / valtan.product / composition.presentation / effect.v2 / gameplay.balance PASS, map.kakul·world.gameplay·navigation·world.destruction·items.catalog·valtan.rewards REUSED. `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`에 `VALTAN_SIX_PIZZA_106 … ARENA_CENTER 4 RADIAL 12.7279220614`, ground-roar·struggling `6.3639610307` 행 확인 |
| `RunFullPipeline.bat` 1차 (Debug / FullDiagnostic) | 빌드 전 gate `test_effect_v2_binding_pipeline`에서 정지. PR #305로 merge된 `boss.valtan.six.sonic` group(child 3, span 300ms)이 oracle에 없던 merge 공백. oracle 두 곳 갱신 |
| `RunFullPipeline.bat` 2차 (Debug / FullDiagnostic) | Engine·Shared·Server·Client와 하네스 6개 빌드·링크 PASS, `Test-ValtanPatternMaster.ps1` PASS. 이후 `test_valtan_cross_rock_wave_effect` gate에서 정지(사용자 cross 편집, 4절 위 항목과 동일). 557 s |
| `Server.exe --contract-test` (새 빌드, 수동) | `failures : 0`. ground-roar 반지름·지연 돌 case(ARENA_CENTER basis) 포함 |
| `ValtanPatternAuditionServiceHarness.exe`, `NetworkProtocolHarness.exe` (수동) | PASS, `failures : 0` |
| `test_publish_rendering_profiles.py` (수동) | `Ran 3 tests OK` |
| Character Select isolation Core/Party2/Party4, PointLight/Physics/WModel harness (수동) | 모두 PASS, `failures : 0` (Core 5 scenario, Party2, Party4, PointLight, Physics, WModel) |
| `test_valtan_cross_rock_wave_effect` oracle 정렬 | 사용자가 cross 돌 base를 `fx_k_turtlespec_01`로 튜닝한 것이 정본이라고 확인. `STONE_BASE` 갱신, codec이 0일 때 생략하는 `transformMotionDurationSeconds`를 `.get(..., 0.0)`으로 읽고, Tool 저장의 float32 반올림(`dissolveStartNormalized 0.649999976`)을 `assertAlmostEqual`로 허용하도록 수정 |
| `test_valtan_cross_rock_wave_effect` (정렬 후) | `Ran 6 tests OK` |
| `RunFullPipeline.bat` 3차 (Debug / FullDiagnostic) | 사용자가 새 Debug `Server.exe`/`Client.exe`를 실행 중이어서 admission `OUTPUT_LOCKED`로 정지(1 s). 회귀 실패가 아니며, 2차 빌드 산출물과 수동 실행한 회귀 단계가 전부 PASS인 상태 |
| `git diff --check` | 공백 오류 없음. 도입했던 LF 4곳은 CRLF로 정정 |

파이프라인이 cross gate에서 정지한 뒤 남은 FullDiagnostic 단계(Server contract test, 하네스, Character Select
시나리오)는 같은 빌드 산출물로 수동 실행했다. 정지 원인인 cross oracle 불일치는 사용자 판단이 필요하다.

## 5. 수동 검증

미실행. 사용자 전용.

1. Debug 산출물은 2차 파이프라인이 이미 빌드했다(`Server/Bin/Debug/Server.exe`, `Client/Bin/Debug/Client.exe`).
   VS에서 다시 빌드해도 증분이며, 회귀 하네스 실행이 끝난 뒤에 한다.
2. cross oracle은 사용자 확인 후 문서 값으로 정렬했다(4절).
3. Server를 재시작하고 Valtan Arena에 진입한다.
4. F1 → Effect Tool → All Effects → `땅구르기 후 사자후 / 4방향 돌`, `피자 패턴 / … 아레나 중앙 ±9m 돌 기둥 4개`,
   `발악 패턴 / … ±4.5m 돌 기둥 4개` 각 행에서 `Open Editor`가 서로 다른 문서를 여는지, `Complete Play Owner`가
   V1 문서대로 돌 → 5초 폭발 → sprite particle까지 재생하고 6.2초 이후에도 끊기지 않는지 확인한다.
5. 피자 돌이 boss 위치·방향과 무관하게 아레나 중앙 `(156.03, -122.06)` 기준 `(±9, ±9)`에 서는지 확인한다.

## 6. 남은 경계

- Server `lifetimeMs 6200`, 폭발 `atMs 5000`은 세 archetype 모두 그대로다. 패턴별로 바꾸려면
  `Data/Valtan/Valtan.combatobjects.json`의 해당 archetype 값을 편집하고 `Workbench Save + Validate + Publish` 또는
  `Project-ValtanPatternMaster.ps1 -Mode PublishV2` 뒤 `Publish-GameplayBalance.ps1`과 Server 재시작이 필요하다.
  Client는 V1 문서의 자연 종료를 따르므로 Effect timeline이 `lifetimeMs`보다 길어도 잘리지 않는다.
- `ARENA_CENTER`의 origin은 Server가 `boss.fSpawnPosition*`(placement `boss.valtan.center`), Tool preview가
  pattern `serverMotion.landingPosition`이다. 두 값은 현재 동일하지만 별도 소유자다.
- `boss.valtan.rock-pillar.sequence` V2 group과 leaf 두 문서는 Product 미참조 상태로 남아 있다.
- 사용자 편집 `effect.valtan.sequence.cross`와 `test_valtan_cross_rock_wave_effect` oracle 불일치는 이번 작업이
  판정하지 않았다.
