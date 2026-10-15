# 발탄 포탈 돌진 4회 연결과 원본 포탈 방향 확인

이 자료는 2026-10-14 현재 저장본을 읽어 만든 **직접 구현용 후보와 근거**다. 제품 C++·JSON, 게시 파일, 실행 중 Server에는 적용하지 않았다. 구조 분리 작업과 별개다. 사용자가 확인한 목표는 포탈 돌진을 네 번 연결하고 원본 포탈 이펙트의 방향을 맞추는 것이다. 다른 세션에서 합의했던 정확한 교체 코드를 확인한 자료는 아니므로 과거 구현을 복원했다고 표현하지 않는다.

## G01. 지금의 세 가지 포탈을 먼저 구분한다

| 현재 저장 ID | 현재 연결 | 이번 작업과의 관계 |
|---|---|---|
| `VALTAN_WARP` | `STEP_02`부터 `STEP_09`까지 `PORTAL_TARGET_RUSH` 8구간. 각 구간에 V2 `boss.valtan.portal` 시작·끝 2개, 총 16개 binding | 실제 서버 경로를 네 번 연결할 대상 |
| `VALTAN_PORTAL_RUSH` / `PORTAL` | legacy compatibility의 4개 stage. `PORTAL`에 원본 carrier 14 element 연결 | 원본 이펙트 자료. 4 stage라는 숫자가 네 번 이동을 뜻하지 않는다 |
| `VALTAN_STRUGGLING` / `STEP_01` | action 420624 carrier 23 element. 손 부착 7개 + root snapshot 16개 | 다른 패턴의 원본 부착·좌표계 비교 자료 |

`VALTAN_PORTAL_RUSH`의 `RUSHES.hitCount=3`과 `FINISH.hitCount=1`도 네 번의 이동 경로를 예약하는 값이 아니다. `VALTAN_WARP`의 각 이동 구간은 50ms 간격의 판정 표본 26개를 가진다. 판정 표본 26개, 플레이어별 타격 소비, 이동 구간 1개는 서로 다른 개수다.

현재 실제 정본은 [Valtan.gameplay.json](C:/Users/tnest/Desktop/LostArk/Data/Valtan/Valtan.gameplay.json:5199), [Valtan.presentation.json](C:/Users/tnest/Desktop/LostArk/Data/Valtan/Valtan.presentation.json:6086), [V2 binding](C:/Users/tnest/Desktop/LostArk/Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json:727)이다. Composition의 Valtan descriptor는 이 split 문서의 경로·패턴 목록을 알리는 `SHADOW` manifest다. 생성된 Encounter나 Animation 파일만 고치면 다음 projection에서 덮어써진다.

## G02. 한 번의 돌진을 만드는 정보와 연산

| 책임 | 실제 코드 |
|---|---|
| stage의 motion 정의를 보스 실행 상태에 복사 | [CValtanBrain::Configure_PortalMotion](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:3937) |
| ENTER에서 선택한 대상 방향과 출발·도착점을 잠금 | [LockPortalTargetRushAtStageStart](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:1823) |
| 대기 이후 이번 tick의 이동 거리 계산 | [CValtanBrain::Try_BuildStageMotion](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:3807) |
| 다음 stage에 들어가기 전에 종료 위치 정리 | [CompletePortalTargetRushAtDeadline](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:1791), [EnterPatternStage](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:1883) |
| 고정한 route의 start/end를 Client에 복제 | [GameRoom_Replication.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_Replication.cpp:806) |
| 복제된 route에서 두 포탈의 행렬 생성 | [CValtan::Try_Get_PortalRushAnchorMatrices](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:4767) |

출발점은 선택한 플레이어 위치가 아니라 그 시점의 `boss.fPositionX/Z`다. 대상 위치는 `FacePoint`가 보스의 방향을 계산할 때 사용한다. `retargetDelayMs`는 이동과 판정을 기다리는 시간이며, 대기 시간이 끝날 때 새로 경로를 선택한다는 뜻이 아니다.

평면은 XZ, 거리 단위는 m다. 보스 yaw를 라디안 `θ`로 바꾸면 다음과 같다.

```text
start = 현재 보스 위치
forward = (sin(θ), cos(θ))
end = start + forward × distanceM
첫 구간: 300ms 대기 + 1300ms 이동 = 1600ms
이후 구간: 600ms 대기 + 1300ms 이동 = 1900ms
이동 거리 16m / 속도 12.3076925m/s ≈ 1.3초
```

매번 ENTER의 `RETARGET_RANDOM_ALIVE`가 다음 대상을 정하고 새 stage의 route를 잠근다. 네 번을 만들기 위해 함수 안에 별도의 반복문이나 고정 횟수 카운터를 추가할 필요는 없다. 기존 stage 전이 목록이 네 번의 호출을 만들도록 연결하면 된다.

## G03. 네 번 연결하는 완전 교체 블록

제안은 첫 네 돌진의 ID와 값을 유지하고 네 번째 뒤에 기존 복귀 구간 `STEP_10`을 연결한다. `STEP_10`을 `STEP_06`으로 이름 바꾸지 않는다. 저장 ID와 배열 순번을 분리해야 기존 복귀 action·animation ID를 그대로 사용할 수 있다.

```text
현재: STEP_01 → 02 → 03 → 04 → 05 → 06 → 07 → 08 → 09 → 10
후보: STEP_01 → 02 → 03 → 04 → 05 → 10
                     돌진 네 번                 복귀
```

| 직접 편집할 원본 | 편집 범위 | 생략 없는 교체 블록 |
|---|---|---|
| `Data/Valtan/Valtan.gameplay.json` | `patterns`에서 `patternId == VALTAN_WARP`인 객체 전체. `STEP_06~09` 삭제, `STEP_05.defaultNextActionId`를 `valtan.sequence.warp.step-10`으로 변경 | [WARP gameplay 후보](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/blocks/proposed-four-rush/WARP.gameplay.pattern.json) |
| `Data/Valtan/Valtan.presentation.json` | 같은 패턴 객체 전체. `STEP_06~09` presentation 삭제. 첫 네 구간의 animation·bodyVisibility와 기존 `STEP_10` 유지 | [WARP presentation 후보](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/blocks/proposed-four-rush/WARP.presentation.pattern.json) |
| `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json` | `bindings`에서 `scope.patternId == VALTAN_WARP`인 16개 행만 후보의 8개 행으로 교체. 다른 패턴의 행 보존 | [WARP V2 binding 후보 8개](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/blocks/proposed-four-rush/WARP.v2bindings.rows.json) |

전체 문서 후보도 [candidate 폴더](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/candidate)에 보존했다. JSON 값 비교용이며, 최신 제품 파일 전체를 무조건 덮어쓰는 용도가 아니다. 사용자가 추가 저장했다면 위 stable ID의 대상 객체·행만 최신 파일에 병합한다.

후보의 연결은 `STEP_01 → STEP_02 → STEP_03 → STEP_04 → STEP_05 → STEP_10`으로 전 구간 도달, dangling edge와 반복 없는지 검사했다. motion 구간은 4개, V2 binding은 8개, 전체 소요 시간은 기존 18,567ms에서 10,967ms다. 각 돌진의 16m, 1,300ms 이동, 판정 26표본, 대기·숨김 값은 유지한다. [구조 검사 결과](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/evidence/four-rush.graph-check.json).

이는 **JSON parse와 구조 검사를 통과한 미적용 후보**다. projector·전체 schema validator·Server 실행·화면을 통과했다고 기록하지 않는다. 아직 원본 14 element로 바꾸는 후보도 아니다. 이 네 번 연결 후보는 현재 V2 포탈을 유지한다.

## G04. 현재 V2 포탈 방향을 실제 값으로 확인한다

서버가 보낸 start/end에서 Client가 `yaw = atan2(endX-startX, endZ-startZ)`를 구한다. 시작점과 끝점에 같은 yaw의 행렬을 만든다. [Resolve_TargetPivot](C:/Users/tnest/Desktop/LostArk/Client/Private/EffectV2_Object.cpp:882)는 `portal.rush.start/end`를 본 이름과 구분해 route 행렬을 선택하며, `TARGET_YAW`도 그 route 행렬의 축을 사용한다.

현재 binding local 회전은 `[0,0,0]`, group child 회전도 `[0,0,0]`, leaf rotation도 `[0,0,0]`이다. [EffectV2_Runtime.cpp:315](C:/Users/tnest/Desktop/LostArk/Client/Private/EffectV2_Runtime.cpp:315)의 child 변환과 binding 변환, [EffectV2_Object.cpp:1092](C:/Users/tnest/Desktop/LostArk/Client/Private/EffectV2_Object.cpp:1092)의 leaf 변환은 다음 순서로 합쳐진다.

```text
leafWorld = leafScale × leafRotation × leafTranslation
          × childLocal × bindingLocal × routePivot
```

현재 [group 완전본](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/current/Data/Effects/V2/Groups/boss.valtan.portal.effectv2group.json), [black leaf 완전본](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/current/Data/Effects/V2/Authored/boss.valtan.portal.black_1.effectv2.json), [cyan leaf 완전본](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/current/Data/Effects/V2/Authored/boss.valtan.portal.cyan_1.effectv2.json)을 보존했다.

| 항목 | black | cyan |
|---|---|---|
| mesh | `fm_d_planeside_001.wmodel` | 동일 |
| meshPreScale | 0.01 | 0.01 |
| leaf position | `[-1,3,0]` | `[-0.95,3,0]` |
| leaf scale | `[4,6,2]` | `[4.8,7.2,2]` |
| child translation | `[0,0,0]` | `[-0.25,0,0]` |
| child scale | `[1,1,1]` | `[1,1,1]` |
| lifetime / group duration | 1.9초 / 1,900ms | 동일 |

설치된 mesh의 정점 4개·index 6개를 실제 WINT 1.0 / WMOD / WMSH layout으로 읽었다. bounds는 X 약 `0~100`, Y `-50~50`, Z 약 `-0.00001033~+0.00001033`이다. 거의 XY 평면이며 저장 normal은 `[-0.0078735,-0.0078735,-0.999938]`로 거의 -Z다. 따라서 회전 0인 현재 leaf의 포탈 평면은 route 방향과 거의 수직이다. [실측 결과와 SHA-256](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/evidence/installed-portal-mesh-measurement.json), [읽기 전용 측정 스크립트](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/Measure-InstalledPortalMeshes.py).

| 서버 yaw | start=(0,0)일 때 end(X,Z) | 돌진 방향 | 평면 법선의 주된 방향 |
|---|---|---|---|
| 0° | `(0,16)` | +Z | -Z |
| 90° | `(16,0)` | +X | -X |
| 180° | `(0,-16)` | -Z | +Z |
| 270° | `(-16,0)` | -X | +X |

이 표는 CPU 기하·행렬의 결론이며 shader를 거친 최종 화면 판정은 아니다. leaf의 `billboard=true`를 보고 mesh가 항상 카메라를 향한다고 설명하면 안 된다. [실제 billboard 분기](C:/Users/tnest/Desktop/LostArk/Client/Private/EffectV2_Object.cpp:1093)는 `SHAPE::SPRITE`만 처리하므로 현재 `Mesh` leaf에는 해당 회전이 적용되지 않는다. 현재 수치만으로 모든 포탈에 ±90°를 더해야 한다는 근거는 없다.

화면에서 특정 구간이 틀어져 보일 때 비교할 값은 다음과 같다.

1. 해당 occurrence의 서버 start/end와 `strPatternStageId`, `bPortalRushTargetLocked`.
2. `Try_Get_PortalRushAnchorMatrices`의 두 translation과 route yaw. 현재 start/end는 같은 회전이며 끝 포탈만 반대 yaw로 만들지 않는다.
3. 해당 binding의 `anchor.localTransform.rotation`과 선택한 group child의 local rotation.
4. leaf `params.rotation`, `meshPreScale`, 설치 mesh의 기하 법선·비균등 scale. 이동 방향과 기하 법선의 각도를 측정한다.
5. 원본 carrier를 사용한다면 아래 G05의 source 모듈과 snapshot basis까지 추가한다. 현재 V2의 수치를 원본 carrier의 보정값으로 복사하지 않는다.

특정 WARP occurrence만 조정할 이유가 확인되면 binding local 값이 가장 좁은 변경 지점이다. `boss.valtan.portal` group은 유령 포탈에도 재사용되므로 전역 group/leaf 회전을 바꾸면 다른 패턴에도 전파된다. 지금은 방향 교정값을 정하지 않았다.

## G05. 원본 14 element를 사용하려면 연결 계약부터 맞춘다

원본 보존 asset은 [carrier 전체 JSON](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/current/Data/Effects/Authored/effect.valtan.carrier-v1.attack.portal-rush.portal.clip-01.effect.json)이다. schema `lostark.effect-authoring`, version 13이며 현재 `DIRECT_AUTHORED_DOCUMENT`로 등록된다. 원본 그룹 `par_n_rpbf_potal_02_01` 9개와 `par_n_rpbf_potal_02_02` 5개, 합계 14개다. sprite 11개·mesh 3개로 구성되며 현재 전체 yaw/element rotation은 0, element scale은 1이다.

이 파일의 Git blob은 현재와 과거 복원 기준 `3dd2a3e0` 모두 `50260f88b1bfc5da5cf485dd1830ea06fccf0ffe`로 같다. [08-30 복원 RESULT](C:/Users/tnest/Desktop/LostArk/.md/GB/08-30/2026-08-30_VALTAN_PORTAL_RUSH_ORIGINAL_ELEMENT_RESTORATION_RESULT.md)는 빠졌던 13개를 원래 14개로 복원한 작업이다. 현재 원본 element가 누락됐다고 전제할 수 없다.

현재 cue는 legacy `VALTAN_PORTAL_RUSH / PORTAL`, `valtan.attack.portal-rush.portal.clip.01`에 `root / follow / natural / once`, local identity로 연결된다. [정본 legacy entry 전체](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/blocks/current/PORTAL_RUSH.legacy-entry.json)와 [생성 cue 전체](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/blocks/current/PORTAL_RUSH.generated-cues.rows.json)를 함께 읽는다.

현재 WARP는 V2 group을 재생한다. V2의 `resource.kind=LEAF/GROUP`에 이 V1 asset ID만 넣으면 같은 로더가 읽어 준다는 계약은 없다. 원본 carrier를 WARP의 두 route 지점에 놓으려면 **기존 V1 cue 재생 경로가 route virtual anchor를 이해하도록 확장**하는 일이 필요하다.

| 필요한 소비자 경계 | 현재 코드와 적용해야 하는 의미 |
|---|---|
| V1 cue anchor admission | [Valtan.cpp:1646](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:1646), 2306, 2865, 4982에서 root·map·arena·target/landing snapshot·실제 bone을 검사한다. `portal.rush.start/end`를 본 이름으로 찾지 않도록 일관되게 분류해야 한다 |
| route snapshot 생성 | 기존 `Try_Get_PortalRushAnchorMatrices`를 재사용한다. 캐릭터의 현재 root나 플레이어 현재 위치로 포탈 끝점을 다시 계산하지 않는다 |
| V1 spawn 경계 | [Valtan.cpp:3231](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:3231)의 `EFFECT_SPAWN_DESC` 생성과 3273 이후 world-root 분기에서 route snapshot을 `Build_CueScalePolicyRoot → Spawn_WorldRoot`에 넘길 수 있어야 한다. root/bone follow로 등록하면 보스 이동을 따라 포탈도 움직인다 |
| 저장·검증·preview | `ValtanPatternEffectCueDocument.cpp`, `ValtanPatternTree.cpp`, Valtan pipeline의 anchor 검증·projection이 같은 이름과 snapshot 의미를 인정해야 한다. preview에 Server route가 없을 때의 명시적 처리도 필요하다 |
| 수명·중복·전환 | stage clock에서 한 occurrence당 한 번 생성하고 실패를 격리한다. 기존 attempted occurrence key·stop/tail 처리를 보존한다. 대체한 구간의 V2 binding을 제거해야 이중 재생되지 않는다 |
| 원본 자체 방향 | source TypeData, `MeshRotation` turn, sprite rotation, particle local/world, occurrence transform, attachment/source basis를 개별 대조한다. shader나 모든 element에 같은 회전을 더하지 않는다 |

위 표는 구현 경계 조사다. 이 소비자 확장을 작성·컴파일한 C++ 후보는 없으며, 두 schema를 단순 치환한 JSON도 만들지 않았다. 정확히 어떤 원본 sub-effect를 출발/도착의 어느 시각에 배치할지는 source 발생 정보와 사용자의 장면 확인에 맞춰 구현해야 한다. 기존 14개 carrier 전체를 출발·도착에 무조건 두 번 복제하면 두 원본 그룹의 의미까지 중복할 수 있다.

source recipe에는 일부 mesh의 `MeshRotation` 분포에 `-0.25`가 보존돼 있다. 이 값은 degree 단위의 `localTransform.rotation`과 다르므로 -0.25°나 전역 -90°로 해석하지 않는다. [모든 element transform·source recipe·attachment 완전본](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/evidence/original-element-transforms-and-source-recipes.json)에 배열을 생략하지 않고 저장했다. 외부 `../Resource_LostArk` 원본은 현재 없어서 이번 조사에서 원본 패키지 재추출을 수행하지 않았다.

비교 자료인 STRUGGLING의 root 16개는 이미 position `[0,0.5,-6]`, scale 0.75, `snapshotRootSourceBasisYawDegrees=-90`이다. hand 7개는 `bip001-l-hand` 및 socket `[0.29,-0.04,0]`을 사용한다. 이 원본 부착의 [Local × SourceBasis × Parent](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Playback.cpp:8365) 공식을 WARP V2에 그대로 옮기지 않는다. [09-30 RESULT](C:/Users/tnest/Desktop/LostArk/.md/GB/09-30/2026-09-30_VALTAN_COUNTER_LOOP_AND_STRUGGLING_RESULT.md)는 다른 occurrence의 기존 수정 근거다.

## G06. 직접 반영 뒤 검증·게시·Server 로드

현재 게시 전에 해결할 별도 불일치가 있다. [Shared format version](C:/Users/tnest/Desktop/LostArk/Shared/Public/GameplayDataRevision.h:28)과 실제 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap` header는 38이다. [Publish-GameplayBalance.ps1:5849](C:/Users/tnest/Desktop/LostArk/Tools/GameplayPipeline/Publish-GameplayBalance.ps1:5849)도 rotation format 4일 때 38을 생성한다. 그런데 [valtan_tuning_pipeline.py:91](C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/valtan_tuning_pipeline.py:91)는 37이다.

`PublishCandidate`는 15032에서 baseline/candidate bootstrap 검사를 호출하고, 13206의 정확한 version 비교가 38을 거절한다. 실행해서 실패한 로그를 얻은 것은 아니며 코드에서 확인한 불일치다. header를 37로 손수 고치거나 검사를 제거하지 않는다. 공용 38 계약과 Python 소비자·해당 기대값을 함께 일치시킨 뒤 게시해야 한다. `PublishV2`의 `project-products`는 별도 정본 projection 경로이므로 이것의 성공만으로 `PublishCandidate`의 admission까지 성공했다고 판단하지 않는다.

네 번 연결을 제품에 채택하면 현재 8회 기대값을 가진 테스트도 의도에 맞게 갱신한다.

- [test_valtan_portal_rush_tuning_contract.py](C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/test_valtan_portal_rush_tuning_contract.py:46): WARP `[1:9]`를 유지한 채 개수만 바꾸지 않는다. 보존 stage ID/`motion.kind`로 4개 돌진을 선택하고, 기대 leg 4개·binding 8개·stage `STEP_02~05`·복귀 `STEP_10`을 검증한다. 끝 stage를 `[9]`로 찾는 부분도 stable ID로 바꾼다.
- [test_valtan_pattern_master_v2.py:1970](C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/test_valtan_pattern_master_v2.py:1970): WARP의 반복 motion·offset 기대값 `첫 1 + 반복 7`을 `첫 1 + 반복 3`으로 변경하고 duration·presentation 범위를 맞춘다. 같은 함수의 `VALTAN_GHOST_FINALE` 8회는 별도 패턴이므로 유지한다.
- 후보를 검증할 때는 실제 채택한 데이터·테스트·publisher가 같은 기준이어야 한다. 현재 제품이 8회인 상태에서 4회 기대값 테스트만 실행해 실패시키는 작업은 하지 않았다.

아래는 version 불일치 해결 및 사용자 저장본 병합을 마친 뒤 사용하는 기존 명령이다. 이번 조사에서는 실행하지 않았다. `python`이 실제 설치 실행 파일인지 먼저 확인한다. 현재 조사 도구는 WindowsApps alias 대신 번들 Python의 절대 경로를 사용했다.

```powershell
# 정본 구조·projection 검사
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode Validate -RepositoryRoot $PWD
python Tools/ValtanPipeline/test_valtan_portal_rush_tuning_contract.py
python Tools/ValtanPipeline/test_valtan_dynamic_authoring_admission.py

# 저작 정본에서 Product와 runtime 생성
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode PublishV2 -RepositoryRoot $PWD
powershell -ExecutionPolicy Bypass -File Tools/CompositionPipeline/Publish-Compositions.ps1 -Mode SyncValtanShadow -RepositoryRoot $PWD
powershell -ExecutionPolicy Bypass -File Tools/CompositionPipeline/Publish-Compositions.ps1 -Mode Validate -RepositoryRoot $PWD
powershell -ExecutionPolicy Bypass -File Tools/CompositionPipeline/Publish-Compositions.ps1 -Mode Publish -RepositoryRoot $PWD
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish
python Tools/GameplayPipeline/test_valtan_presentation_generation.py
python Tools/EffectToolV2/validate_effect_v2.py --repository-root . --resource-root Client/Bin/Resources
git diff --check
```

변경 직후 `Validate`가 기존 생성물과의 Product drift를 보고할 수 있다. 정본을 수정했는데 이전 생성물이 남아 있다는 뜻인지 확인한 뒤 `PublishV2`로 생성하고 다시 검증한다. drift 검사를 없애거나 원본 아닌 generated Encounter만 수정해서 맞추지 않는다. V1 소비자 C++까지 확장한다면 Client 증분 컴파일과 그 admission/route/snapshot 소비 검증이 추가로 필요하다. 이 자료의 데이터만으로 C++ 파일·프로젝트 항목을 추가할 필요는 없다.

최종적으로 Server는 원본 `Data/Valtan/*.json`을 읽지 않는다. [CGameplayCatalog::Load](C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:1419)가 `DataFiles/Gameplay/Gameplay.bootstrap`을 임시 catalog에 읽어 검증한 뒤 교체한다. [Resolve_DataRoot](C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:89)는 `LOSTARK_SERVER_DATA_ROOT`를 우선하고, 없으면 실행 EXE 상위의 `DataFiles`를 사용한다. 현재 저장소 게시 경로는 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`이다.

[GameRoom 생성자](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom.cpp:85)는 전달받은 generation으로 초기화하거나 catalog Load를 호출한다. 실행 중 서버에 반영하려면 기존 stage/commit generation transaction을 통과해야 하며, [Commit_GameplayGeneration](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom.cpp:647)은 진행 중 boss occurrence의 pin을 임의로 바꾸지 않는다. 파일 게시 성공·활성 generation 전환·다음 occurrence의 네 번 실행·Client의 포탈 표시를 각각 확인한다. 서버 종료나 reload를 이번 자료 생성에서 수행하지 않았다.

## G07. 보존한 자료와 실제 확인 범위

- [current](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/current): 현재 정본 JSON 12개를 byte 단위로 복사했다.
- [blocks/current](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/blocks/current): 관련 객체·배열 전체를 독립 JSON으로 추출했다. 원래 문서의 일부이므로 이 파일 자체를 publisher 입력 전체로 쓰지 않는다.
- [candidate](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/candidate): 현재 V2 포탈을 유지한 4회 연결의 전체 문서 후보 3개다.
- [source-manifest](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/evidence/source-manifest.json): 현재 snapshot과 원본의 SHA-256 및 bytes. 수집 종료 시 원본 12개의 hash 불변을 확인했다.
- [Build-ReadOnlyEvidence.ps1](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-valtan/Build-ReadOnlyEvidence.ps1): 출력 경로를 지정된 out 폴더 안으로 제한한다. 제품 파일에 쓰는 명령은 없다.

실행한 것은 출력 JSON parse, 후보 연결·개수 검사, 4방향 route 수식 대조, 설치 mesh 3종의 정점·법선 읽기다. 제품 build·publish·Server reload·Client 실행·GPU 화면 확인은 수행하지 않았다. 원본 14 carrier의 route anchor 확장과 최종 방향 선택은 아직 구현 전이다.

## 직접 교체할 JSON 전문

현재 V2 포탈을 유지한 4회 연결 후보다. 원본 carrier 교체 코드는 아니다.

### WARP.gameplay.pattern.json

```json
{
    "patternId":  "VALTAN_WARP",
    "displayName":  "워프 패턴",
    "category":  "NORMAL",
    "compatibilitySelectionWeight":  1,
    "actionId":  "valtan.sequence.warp",
    "entryActionId":  "valtan.sequence.warp.step-01",
    "targetPolicy":  "LOCK_RANDOM_ALIVE_ON_START",
    "aimPolicy":  "LOCK_FACING_ON_START",
    "eligibility":  {
                        "armorRequirement":  "ANY",
                        "phaseRequirement":  "ANY",
                        "minimumGameplayPhase":  2,
                        "maximumGameplayPhase":  2,
                        "minimumHealthBarInclusive":  16,
                        "maximumHealthBarInclusive":  80,
                        "minimumRangeM":  0.0,
                        "maximumRangeM":  1.0,
                        "cooldownPolicy":  "DERIVED_SOURCE_ACTION",
                        "selectionCooldownMs":  null,
                        "cooldownGroupId":  null,
                        "repeatPolicy":  {
                                             "kind":  "SOFT_AVOID_UNLESS_ONLY_ELIGIBLE",
                                             "limit":  1
                                         }
                    },
    "invulnerableWhileRunning":  false,
    "sourceActionIds":  [
                            420624,
                            420622
                        ],
    "serverMotion":  null,
    "reactions":  [

                  ],
    "stages":  [
                   {
                       "stageId":  "STEP_01",
                       "actionId":  "valtan.sequence.warp.step-01",
                       "stageKind":  "ACTIVE",
                       "durationMs":  2000,
                       "defaultNextActionId":  "valtan.sequence.warp.step-02",
                       "hit":  {
                                   "shape":  {
                                                 "kind":  "NONE"
                                             }
                               },
                       "motion":  null,
                       "events":  [

                                  ],
                       "branches":  [

                                    ]
                   },
                   {
                       "stageId":  "STEP_02",
                       "actionId":  "valtan.sequence.warp.step-02",
                       "stageKind":  "ACTIVE",
                       "durationMs":  1600,
                       "defaultNextActionId":  "valtan.sequence.warp.step-03",
                       "hit":  {
                                   "shape":  {
                                                 "kind":  "BOX",
                                                 "lengthM":  8.0,
                                                 "halfWidthM":  2.5
                                             },
                                   "schedule":  {
                                                    "kind":  "EXPLICIT_OFFSETS",
                                                    "offsetsMs":  [
                                                                      300,
                                                                      350,
                                                                      400,
                                                                      450,
                                                                      500,
                                                                      550,
                                                                      600,
                                                                      650,
                                                                      700,
                                                                      750,
                                                                      800,
                                                                      850,
                                                                      900,
                                                                      950,
                                                                      1000,
                                                                      1050,
                                                                      1100,
                                                                      1150,
                                                                      1200,
                                                                      1250,
                                                                      1300,
                                                                      1350,
                                                                      1400,
                                                                      1450,
                                                                      1500,
                                                                      1550
                                                                  ]
                                                },
                                   "serverDamageProfileId":  "damage.valtan.portal-rush",
                                   "pushRangeM":  3.0,
                                   "pushMs":  180,
                                   "knockdown":  true,
                                   "downMs":  1500
                               },
                       "motion":  {
                                      "kind":  "PORTAL_TARGET_RUSH",
                                      "retargetDelayMs":  300,
                                      "speedMps":  12.3076925,
                                      "distanceM":  16.0
                                  },
                       "events":  [
                                      {
                                          "eventId":  "event.valtan.phase2.warp.leg-01.retarget",
                                          "trigger":  "ENTER",
                                          "kind":  "RETARGET_RANDOM_ALIVE"
                                      }
                                  ],
                       "branches":  [

                                    ]
                   },
                   {
                       "stageId":  "STEP_03",
                       "actionId":  "valtan.sequence.warp.step-03",
                       "stageKind":  "ACTIVE",
                       "durationMs":  1900,
                       "defaultNextActionId":  "valtan.sequence.warp.step-04",
                       "hit":  {
                                   "shape":  {
                                                 "kind":  "BOX",
                                                 "lengthM":  8.0,
                                                 "halfWidthM":  2.5
                                             },
                                   "schedule":  {
                                                    "kind":  "EXPLICIT_OFFSETS",
                                                    "offsetsMs":  [
                                                                      600,
                                                                      650,
                                                                      700,
                                                                      750,
                                                                      800,
                                                                      850,
                                                                      900,
                                                                      950,
                                                                      1000,
                                                                      1050,
                                                                      1100,
                                                                      1150,
                                                                      1200,
                                                                      1250,
                                                                      1300,
                                                                      1350,
                                                                      1400,
                                                                      1450,
                                                                      1500,
                                                                      1550,
                                                                      1600,
                                                                      1650,
                                                                      1700,
                                                                      1750,
                                                                      1800,
                                                                      1850
                                                                  ]
                                                },
                                   "serverDamageProfileId":  "damage.valtan.portal-rush",
                                   "pushRangeM":  3.0,
                                   "pushMs":  180,
                                   "knockdown":  true,
                                   "downMs":  1500
                               },
                       "motion":  {
                                      "kind":  "PORTAL_TARGET_RUSH",
                                      "retargetDelayMs":  600,
                                      "speedMps":  12.3076925,
                                      "distanceM":  16.0
                                  },
                       "events":  [
                                      {
                                          "eventId":  "event.valtan.phase2.warp.leg-02.retarget",
                                          "trigger":  "ENTER",
                                          "kind":  "RETARGET_RANDOM_ALIVE"
                                      }
                                  ],
                       "branches":  [

                                    ]
                   },
                   {
                       "stageId":  "STEP_04",
                       "actionId":  "valtan.sequence.warp.step-04",
                       "stageKind":  "ACTIVE",
                       "durationMs":  1900,
                       "defaultNextActionId":  "valtan.sequence.warp.step-05",
                       "hit":  {
                                   "shape":  {
                                                 "kind":  "BOX",
                                                 "lengthM":  8.0,
                                                 "halfWidthM":  2.5
                                             },
                                   "schedule":  {
                                                    "kind":  "EXPLICIT_OFFSETS",
                                                    "offsetsMs":  [
                                                                      600,
                                                                      650,
                                                                      700,
                                                                      750,
                                                                      800,
                                                                      850,
                                                                      900,
                                                                      950,
                                                                      1000,
                                                                      1050,
                                                                      1100,
                                                                      1150,
                                                                      1200,
                                                                      1250,
                                                                      1300,
                                                                      1350,
                                                                      1400,
                                                                      1450,
                                                                      1500,
                                                                      1550,
                                                                      1600,
                                                                      1650,
                                                                      1700,
                                                                      1750,
                                                                      1800,
                                                                      1850
                                                                  ]
                                                },
                                   "serverDamageProfileId":  "damage.valtan.portal-rush",
                                   "pushRangeM":  3.0,
                                   "pushMs":  180,
                                   "knockdown":  true,
                                   "downMs":  1500
                               },
                       "motion":  {
                                      "kind":  "PORTAL_TARGET_RUSH",
                                      "retargetDelayMs":  600,
                                      "speedMps":  12.3076925,
                                      "distanceM":  16.0
                                  },
                       "events":  [
                                      {
                                          "eventId":  "event.valtan.phase2.warp.leg-03.retarget",
                                          "trigger":  "ENTER",
                                          "kind":  "RETARGET_RANDOM_ALIVE"
                                      }
                                  ],
                       "branches":  [

                                    ]
                   },
                   {
                       "stageId":  "STEP_05",
                       "actionId":  "valtan.sequence.warp.step-05",
                       "stageKind":  "ACTIVE",
                       "durationMs":  1900,
                       "defaultNextActionId":  "valtan.sequence.warp.step-10",
                       "hit":  {
                                   "shape":  {
                                                 "kind":  "BOX",
                                                 "lengthM":  8.0,
                                                 "halfWidthM":  2.5
                                             },
                                   "schedule":  {
                                                    "kind":  "EXPLICIT_OFFSETS",
                                                    "offsetsMs":  [
                                                                      600,
                                                                      650,
                                                                      700,
                                                                      750,
                                                                      800,
                                                                      850,
                                                                      900,
                                                                      950,
                                                                      1000,
                                                                      1050,
                                                                      1100,
                                                                      1150,
                                                                      1200,
                                                                      1250,
                                                                      1300,
                                                                      1350,
                                                                      1400,
                                                                      1450,
                                                                      1500,
                                                                      1550,
                                                                      1600,
                                                                      1650,
                                                                      1700,
                                                                      1750,
                                                                      1800,
                                                                      1850
                                                                  ]
                                                },
                                   "serverDamageProfileId":  "damage.valtan.portal-rush",
                                   "pushRangeM":  3.0,
                                   "pushMs":  180,
                                   "knockdown":  true,
                                   "downMs":  1500
                               },
                       "motion":  {
                                      "kind":  "PORTAL_TARGET_RUSH",
                                      "retargetDelayMs":  600,
                                      "speedMps":  12.3076925,
                                      "distanceM":  16.0
                                  },
                       "events":  [
                                      {
                                          "eventId":  "event.valtan.phase2.warp.leg-04.retarget",
                                          "trigger":  "ENTER",
                                          "kind":  "RETARGET_RANDOM_ALIVE"
                                      }
                                  ],
                       "branches":  [

                                    ]
                   },
                   {
                       "stageId":  "STEP_10",
                       "actionId":  "valtan.sequence.warp.step-10",
                       "stageKind":  "ACTIVE",
                       "durationMs":  1667,
                       "defaultNextActionId":  null,
                       "hit":  {
                                   "shape":  {
                                                 "kind":  "NONE"
                                             }
                               },
                       "motion":  null,
                       "events":  [
                                      {
                                          "eventId":  "event.valtan.warp.return-center",
                                          "trigger":  "ENTER",
                                          "kind":  "RETURN_TO_ARENA_CENTER"
                                      }
                                  ],
                       "branches":  [

                                    ]
                   }
               ]
}
```

### WARP.presentation.pattern.json

```json
{
    "patternId":  "VALTAN_WARP",
    "sourceSequenceIndex":  1,
    "presentationSources":  [
                                {
                                    "sourceActionId":  420624,
                                    "sequenceIndex":  1,
                                    "role":  "PRIMARY"
                                },
                                {
                                    "sourceActionId":  420622,
                                    "sequenceIndex":  1,
                                    "role":  "REFERENCE"
                                }
                            ],
    "stages":  [
                   {
                       "stageId":  "STEP_01",
                       "actionId":  "valtan.sequence.warp.step-01",
                       "sequenceRole":  "STEP",
                       "animation":  {
                                         "endPolicy":  "EXACT",
                                         "repeatCount":  1,
                                         "occurrences":  [
                                                             {
                                                                 "clipOccurrenceId":  "valtan.sequence.warp.step-01.clip-01",
                                                                 "clip":  "mesh_att_battle_18_01",
                                                                 "mappingBasis":  "PROJECT_AUTHORED",
                                                                 "sourceStartMs":  0,
                                                                 "playMs":  2000,
                                                                 "playRate":  1.0,
                                                                 "repeatUntilStageEnd":  false
                                                             }
                                                         ]
                                     },
                       "effectCues":  [

                                      ],
                       "cameraInvocations":  [

                                             ]
                   },
                   {
                       "stageId":  "STEP_02",
                       "actionId":  "valtan.sequence.warp.step-02",
                       "sequenceRole":  "STEP",
                       "animation":  {
                                         "endPolicy":  "LOOP_TO_STAGE_END",
                                         "repeatCount":  1,
                                         "occurrences":  [
                                                             {
                                                                 "clipOccurrenceId":  "valtan.sequence.warp.step-02.clip-01",
                                                                 "clip":  "mesh_att_battle_18_02",
                                                                 "mappingBasis":  "PROJECT_AUTHORED",
                                                                 "sourceStartMs":  0,
                                                                 "playMs":  0,
                                                                 "playRate":  1.0,
                                                                 "repeatUntilStageEnd":  true
                                                             }
                                                         ]
                                     },
                       "effectCues":  [

                                      ],
                       "cameraInvocations":  [

                                             ],
                       "bodyVisibility":  {
                                              "hiddenFromMs":  0,
                                              "hiddenToMs":  300
                                          }
                   },
                   {
                       "stageId":  "STEP_03",
                       "actionId":  "valtan.sequence.warp.step-03",
                       "sequenceRole":  "STEP",
                       "animation":  {
                                         "endPolicy":  "LOOP_TO_STAGE_END",
                                         "repeatCount":  1,
                                         "occurrences":  [
                                                             {
                                                                 "clipOccurrenceId":  "valtan.sequence.warp.step-03.clip-01",
                                                                 "clip":  "mesh_att_battle_18_02",
                                                                 "mappingBasis":  "PROJECT_AUTHORED",
                                                                 "sourceStartMs":  0,
                                                                 "playMs":  0,
                                                                 "playRate":  1.0,
                                                                 "repeatUntilStageEnd":  true
                                                             }
                                                         ]
                                     },
                       "effectCues":  [

                                      ],
                       "cameraInvocations":  [

                                             ],
                       "bodyVisibility":  {
                                              "hiddenFromMs":  0,
                                              "hiddenToMs":  600
                                          }
                   },
                   {
                       "stageId":  "STEP_04",
                       "actionId":  "valtan.sequence.warp.step-04",
                       "sequenceRole":  "STEP",
                       "animation":  {
                                         "endPolicy":  "LOOP_TO_STAGE_END",
                                         "repeatCount":  1,
                                         "occurrences":  [
                                                             {
                                                                 "clipOccurrenceId":  "valtan.sequence.warp.step-04.clip-01",
                                                                 "clip":  "mesh_att_battle_18_02",
                                                                 "mappingBasis":  "PROJECT_AUTHORED",
                                                                 "sourceStartMs":  0,
                                                                 "playMs":  0,
                                                                 "playRate":  1.0,
                                                                 "repeatUntilStageEnd":  true
                                                             }
                                                         ]
                                     },
                       "effectCues":  [

                                      ],
                       "cameraInvocations":  [

                                             ],
                       "bodyVisibility":  {
                                              "hiddenFromMs":  0,
                                              "hiddenToMs":  600
                                          }
                   },
                   {
                       "stageId":  "STEP_05",
                       "actionId":  "valtan.sequence.warp.step-05",
                       "sequenceRole":  "STEP",
                       "animation":  {
                                         "endPolicy":  "LOOP_TO_STAGE_END",
                                         "repeatCount":  1,
                                         "occurrences":  [
                                                             {
                                                                 "clipOccurrenceId":  "valtan.sequence.warp.step-05.clip-01",
                                                                 "clip":  "mesh_att_battle_18_02",
                                                                 "mappingBasis":  "PROJECT_AUTHORED",
                                                                 "sourceStartMs":  0,
                                                                 "playMs":  0,
                                                                 "playRate":  1.0,
                                                                 "repeatUntilStageEnd":  true
                                                             }
                                                         ]
                                     },
                       "effectCues":  [

                                      ],
                       "cameraInvocations":  [

                                             ],
                       "bodyVisibility":  {
                                              "hiddenFromMs":  0,
                                              "hiddenToMs":  600
                                          }
                   },
                   {
                       "stageId":  "STEP_10",
                       "actionId":  "valtan.sequence.warp.step-10",
                       "sequenceRole":  "STEP",
                       "animation":  {
                                         "endPolicy":  "EXACT",
                                         "repeatCount":  1,
                                         "occurrences":  [
                                                             {
                                                                 "clipOccurrenceId":  "valtan.sequence.warp.step-10.clip-01",
                                                                 "clip":  "mesh_att_battle_18_03-2",
                                                                 "mappingBasis":  "PROJECT_AUTHORED",
                                                                 "sourceStartMs":  0,
                                                                 "playMs":  1667,
                                                                 "playRate":  1.0,
                                                                 "repeatUntilStageEnd":  false
                                                             }
                                                         ]
                                     },
                       "effectCues":  [

                                      ],
                       "cameraInvocations":  [

                                             ],
                       "bodyVisibility":  {
                                              "hiddenFromMs":  0,
                                              "hiddenToMs":  300
                                          }
                   }
               ]
}
```

### WARP.v2bindings.rows.json

```json
[
    {
        "bindingId":  "binding.valtan.project-tuned.warp.step-02.portal-0-start",
        "resource":  {
                         "kind":  "GROUP",
                         "id":  "boss.valtan.portal"
                     },
        "scope":  {
                      "patternId":  "VALTAN_WARP",
                      "stageId":  "STEP_02",
                      "actionId":  "valtan.sequence.warp.step-02"
                  },
        "clock":  {
                      "basis":  "STAGE",
                      "clipOccurrenceId":  null,
                      "startMs":  0,
                      "repeatPolicy":  "ONCE"
                  },
        "anchor":  {
                       "slotId":  "portal.rush.start",
                       "followPolicy":  "SNAPSHOT_AT_START",
                       "rotationBasis":  "TARGET_YAW",
                       "localTransform":  {
                                              "translation":  [
                                                                  0.0,
                                                                  0.0,
                                                                  0.0
                                                              ],
                                              "rotation":  [
                                                               0.0,
                                                               0.0,
                                                               0.0
                                                           ],
                                              "scale":  [
                                                            1.0,
                                                            1.0,
                                                            1.0
                                                        ]
                                          }
                   },
        "stopPolicy":  "NATURAL"
    },
    {
        "bindingId":  "binding.valtan.project-tuned.warp.step-02.portal-1-end",
        "resource":  {
                         "kind":  "GROUP",
                         "id":  "boss.valtan.portal"
                     },
        "scope":  {
                      "patternId":  "VALTAN_WARP",
                      "stageId":  "STEP_02",
                      "actionId":  "valtan.sequence.warp.step-02"
                  },
        "clock":  {
                      "basis":  "STAGE",
                      "clipOccurrenceId":  null,
                      "startMs":  0,
                      "repeatPolicy":  "ONCE"
                  },
        "anchor":  {
                       "slotId":  "portal.rush.end",
                       "followPolicy":  "SNAPSHOT_AT_START",
                       "rotationBasis":  "TARGET_YAW",
                       "localTransform":  {
                                              "translation":  [
                                                                  0.0,
                                                                  0.0,
                                                                  0.0
                                                              ],
                                              "rotation":  [
                                                               0.0,
                                                               0.0,
                                                               0.0
                                                           ],
                                              "scale":  [
                                                            1.0,
                                                            1.0,
                                                            1.0
                                                        ]
                                          }
                   },
        "stopPolicy":  "NATURAL"
    },
    {
        "bindingId":  "binding.valtan.project-tuned.warp.step-03.portal-0-start",
        "resource":  {
                         "kind":  "GROUP",
                         "id":  "boss.valtan.portal"
                     },
        "scope":  {
                      "patternId":  "VALTAN_WARP",
                      "stageId":  "STEP_03",
                      "actionId":  "valtan.sequence.warp.step-03"
                  },
        "clock":  {
                      "basis":  "STAGE",
                      "clipOccurrenceId":  null,
                      "startMs":  300,
                      "repeatPolicy":  "ONCE"
                  },
        "anchor":  {
                       "slotId":  "portal.rush.start",
                       "followPolicy":  "SNAPSHOT_AT_START",
                       "rotationBasis":  "TARGET_YAW",
                       "localTransform":  {
                                              "translation":  [
                                                                  0.0,
                                                                  0.0,
                                                                  0.0
                                                              ],
                                              "rotation":  [
                                                               0.0,
                                                               0.0,
                                                               0.0
                                                           ],
                                              "scale":  [
                                                            1.0,
                                                            1.0,
                                                            1.0
                                                        ]
                                          }
                   },
        "stopPolicy":  "NATURAL"
    },
    {
        "bindingId":  "binding.valtan.project-tuned.warp.step-03.portal-1-end",
        "resource":  {
                         "kind":  "GROUP",
                         "id":  "boss.valtan.portal"
                     },
        "scope":  {
                      "patternId":  "VALTAN_WARP",
                      "stageId":  "STEP_03",
                      "actionId":  "valtan.sequence.warp.step-03"
                  },
        "clock":  {
                      "basis":  "STAGE",
                      "clipOccurrenceId":  null,
                      "startMs":  300,
                      "repeatPolicy":  "ONCE"
                  },
        "anchor":  {
                       "slotId":  "portal.rush.end",
                       "followPolicy":  "SNAPSHOT_AT_START",
                       "rotationBasis":  "TARGET_YAW",
                       "localTransform":  {
                                              "translation":  [
                                                                  0.0,
                                                                  0.0,
                                                                  0.0
                                                              ],
                                              "rotation":  [
                                                               0.0,
                                                               0.0,
                                                               0.0
                                                           ],
                                              "scale":  [
                                                            1.0,
                                                            1.0,
                                                            1.0
                                                        ]
                                          }
                   },
        "stopPolicy":  "NATURAL"
    },
    {
        "bindingId":  "binding.valtan.project-tuned.warp.step-04.portal-0-start",
        "resource":  {
                         "kind":  "GROUP",
                         "id":  "boss.valtan.portal"
                     },
        "scope":  {
                      "patternId":  "VALTAN_WARP",
                      "stageId":  "STEP_04",
                      "actionId":  "valtan.sequence.warp.step-04"
                  },
        "clock":  {
                      "basis":  "STAGE",
                      "clipOccurrenceId":  null,
                      "startMs":  300,
                      "repeatPolicy":  "ONCE"
                  },
        "anchor":  {
                       "slotId":  "portal.rush.start",
                       "followPolicy":  "SNAPSHOT_AT_START",
                       "rotationBasis":  "TARGET_YAW",
                       "localTransform":  {
                                              "translation":  [
                                                                  0.0,
                                                                  0.0,
                                                                  0.0
                                                              ],
                                              "rotation":  [
                                                               0.0,
                                                               0.0,
                                                               0.0
                                                           ],
                                              "scale":  [
                                                            1.0,
                                                            1.0,
                                                            1.0
                                                        ]
                                          }
                   },
        "stopPolicy":  "NATURAL"
    },
    {
        "bindingId":  "binding.valtan.project-tuned.warp.step-04.portal-1-end",
        "resource":  {
                         "kind":  "GROUP",
                         "id":  "boss.valtan.portal"
                     },
        "scope":  {
                      "patternId":  "VALTAN_WARP",
                      "stageId":  "STEP_04",
                      "actionId":  "valtan.sequence.warp.step-04"
                  },
        "clock":  {
                      "basis":  "STAGE",
                      "clipOccurrenceId":  null,
                      "startMs":  300,
                      "repeatPolicy":  "ONCE"
                  },
        "anchor":  {
                       "slotId":  "portal.rush.end",
                       "followPolicy":  "SNAPSHOT_AT_START",
                       "rotationBasis":  "TARGET_YAW",
                       "localTransform":  {
                                              "translation":  [
                                                                  0.0,
                                                                  0.0,
                                                                  0.0
                                                              ],
                                              "rotation":  [
                                                               0.0,
                                                               0.0,
                                                               0.0
                                                           ],
                                              "scale":  [
                                                            1.0,
                                                            1.0,
                                                            1.0
                                                        ]
                                          }
                   },
        "stopPolicy":  "NATURAL"
    },
    {
        "bindingId":  "binding.valtan.project-tuned.warp.step-05.portal-0-start",
        "resource":  {
                         "kind":  "GROUP",
                         "id":  "boss.valtan.portal"
                     },
        "scope":  {
                      "patternId":  "VALTAN_WARP",
                      "stageId":  "STEP_05",
                      "actionId":  "valtan.sequence.warp.step-05"
                  },
        "clock":  {
                      "basis":  "STAGE",
                      "clipOccurrenceId":  null,
                      "startMs":  300,
                      "repeatPolicy":  "ONCE"
                  },
        "anchor":  {
                       "slotId":  "portal.rush.start",
                       "followPolicy":  "SNAPSHOT_AT_START",
                       "rotationBasis":  "TARGET_YAW",
                       "localTransform":  {
                                              "translation":  [
                                                                  0.0,
                                                                  0.0,
                                                                  0.0
                                                              ],
                                              "rotation":  [
                                                               0.0,
                                                               0.0,
                                                               0.0
                                                           ],
                                              "scale":  [
                                                            1.0,
                                                            1.0,
                                                            1.0
                                                        ]
                                          }
                   },
        "stopPolicy":  "NATURAL"
    },
    {
        "bindingId":  "binding.valtan.project-tuned.warp.step-05.portal-1-end",
        "resource":  {
                         "kind":  "GROUP",
                         "id":  "boss.valtan.portal"
                     },
        "scope":  {
                      "patternId":  "VALTAN_WARP",
                      "stageId":  "STEP_05",
                      "actionId":  "valtan.sequence.warp.step-05"
                  },
        "clock":  {
                      "basis":  "STAGE",
                      "clipOccurrenceId":  null,
                      "startMs":  300,
                      "repeatPolicy":  "ONCE"
                  },
        "anchor":  {
                       "slotId":  "portal.rush.end",
                       "followPolicy":  "SNAPSHOT_AT_START",
                       "rotationBasis":  "TARGET_YAW",
                       "localTransform":  {
                                              "translation":  [
                                                                  0.0,
                                                                  0.0,
                                                                  0.0
                                                              ],
                                              "rotation":  [
                                                               0.0,
                                                               0.0,
                                                               0.0
                                                           ],
                                              "scale":  [
                                                            1.0,
                                                            1.0,
                                                            1.0
                                                        ]
                                          }
                   },
        "stopPolicy":  "NATURAL"
    }
]
```
