# 세이튼 추적 회전과 피자 Summon 실행 연결 결과

기준일: 2026-09-18. 대응 범위는 [패턴 런타임 복구 계획](2026-09-18_KOUKU_PATTERN_RUNTIME_REPAIR_PLAN.md)의 추적 회전과 피자 Summon이다. 최초 구현의 계약은 [피자 Summon 결과](2026-09-18_KOUKU_PIZZA_SUMMON_LAYOUT_RESULT.md)를 유지한다.

## G01. Server 추적 회전 구현

`Server/Private/GameRoom_BossSimulation.cpp::Update_KoukuPlayerTargets`의 이동 추적을 수정했다.

- `BOSS_TRACK_TARGET.followSpeedScale > 0`은 수명에 관계없이 180도/초, 30Hz 한 tick당 최대 6도의 최단 회전을 사용한다. 여러 tick을 한 번에 처리하면 경과 tick만큼 회전 한도를 적용한다.
- `followSpeedScale == 0`인 기존 회전 전용 창은 남은 duration에 분배하는 보간을 유지한다. 기존 Showtime의 목표 선택과 복수 플레이어 시각 객체도 유지한다.
- `MN_RPCT_05`를 사용하는 G1/G3/Bingo Saydon의 이동 추적은 모델 전방 +X를 사용한다. 목표의 맵 heading에서 90도를 뺀 값이 body yaw이며, 전진 방향은 body yaw에 90도를 더해 계산한다. G2 Big Saydon의 기존 보정은 중복 적용하지 않는다.
- 이동은 기존 Server navigation과 body collision을 통과해야 한다. root origin과 ground base를 같은 delta만큼 옮겨 다음 absolute root sample이 이동을 되돌리지 않게 하는 기존 경로를 유지한다.

`CKoukuSaydonBrain::Apply_StageRootMotion`은 위치를 적용하고 body yaw를 다시 쓰지 않는다. 따라서 root motion이 회전을 덮는 문제는 아니었다. 기존 추적 회전이 8582ms 전체 수명에 분배되어 늦어지는 조건과 RPCT_05의 실제 +X 전방에 +Z heading을 적용한 조건을 함께 수정했다. Client Preview의 동일 계약 구현은 통합 작업에서 별도로 반영한다.

### 실제 설치 모델의 전방 실측

설치된 `MN_RPCT_05` WModel의 실제 공굴리기 clip을 읽고 frame 0의 bone local channel을 부모 계층으로 합성했다. `bip001-head → bip001-mouth`의 XZ 방향을 `atan2(dx, dz)`로 측정했다. 메타데이터의 캐릭터 이름만으로 축을 추정하지 않았다.

| 실제 clip | head→mouth X | head→mouth Z | +Z 기준 XZ yaw |
|---|---:|---:|---:|
| rpct00_att_battle_26_01 | 0.542495 | 0.024618 | 87.401718도 |
| rpct00_att_battle_26_02 | 0.538485 | -0.001930 | 90.205319도 |
| rpct00_att_battle_26_04 | 0.538489 | -0.001920 | 90.204337도 |
| rpct00_idle_battle_1 | 0.542188 | 0.024588 | 87.403487도 |

이 값은 실제 입이 모델 +X를 향함을 보여준다. source root translation의 축과 구분한 body facing 근거다. 상세 bone 좌표는 `out/KoukuPizzaSummon20260918/server-runtime/saydon-facing-bones.json`에 있다. 수치 검증은 화면의 불 부착/최종 자세 확인과 구분한다.

## G02. Summon 소비자와 실제 실패 원인

기존 `patternSpawns → SUMMON_PATTERNS → CGameRoom의 dependent boss clone` 경로는 유지했다. 각 clone은 별도의 Pattern clock/root motion/airborne 상태를 갖는다. MAP의 absolute XYZ/yaw, 최대 16행, actor-local JUMP/SLAM, 전체 stage 후 commit, MAP navigation 높이 오차 1m 이내 계약도 유지한다.

0ms JUMP는 spawn payload를 만들기 전에 반영한다. clone의 후속 tick에서는 root sample과 Brain update 후 due trigger를 실행한다. 동일 clock의 JUMP/SLAM이 역순으로 저장되어도 JUMP를 먼저 소비한다. 이미 소비한 trigger는 다시 실행하지 않는다. parent가 멈추면 10개의 owned actor도 함께 정리한다.

이번 복구 전 실제 파일과 프로세스를 읽은 결과는 다음과 같다.

- 저장된 P25 Summon occurrence에는 `patternSpawns`가 없었다. P25 부모는 18167ms, Summon 시작은 1461ms, 길이는 16586ms였다. P84 길이 17333ms를 끝까지 재생하려면 Summon/부모 수명도 맞아야 한다.
- 당시 게시된 Encounter P25의 mechanicTriggers는 비어 있었고 Server bootstrap의 `PATTERNSUMMONSPAWN` 행도 0개였다. UI 지원과 C++ 컴파일만으로 기존 이름뿐인 Summon에 자동으로 10명이 추가되지는 않는다.
- 당시 Server bootstrap에는 P81과 `PATTERNTRACKMOVE`도 없었다. source P81은 duration 8582ms와 followSpeedScale 1을 저장하고 있었지만 Product에 들어가지 못했다.
- 관측 당시 Server.exe의 수정 시각은 15:51:52, Client.exe는 15:52:23이었고 두 프로세스의 시작 시각은 15:52:24였다. 단순히 이전 실행 파일을 계속 쓴다는 설명만으로 실패를 설명할 수 없었다.

이 기록은 **통합 담당자의 최신 source 병합/게시 전 상태**다. 이후 source 설치와 게시물 확인은 아래 통합 반영 상태에 별도로 기록한다. 이 Server 조사 작업은 Composition/Effect/World 정본을 직접 덮어쓰지 않았다.

## G03. P81 Counter 게시 차단과 수정 계약

P81이 빠지는 첫 사유는 `PRODUCT pattern owns a DURATION logic box without a judgementKind: KAKULSAYDON_G1_PATTERN_81.logic.1`이었다. 참조하는 `logic.82`는 `logicType=DURATION`만 가진 빈 정의였다. 연결된 `logic.51` 역시 `logicType=RESULT`만 있고 `outcomeKind`가 없어 첫 오류를 해결해도 두 번째 게시 차단이 남는다. 두 정의는 무관한 P80과 공유되어 있었다.

통합 변경은 P81 전용 `COUNTER_WINDOW`, `endsPatternOnSuccess=true` 정의를 배정하고 P81 occurrence만 참조하게 한다. 사용자 start 0 / duration 2837ms는 유지하며 실행 의미가 없던 bare RESULT51 연결만 P81에서 제거한다. P80과 원래 공유 정의는 보존한다.

기존 Server 계약은 결과 배열이 없는 Counter 성공도 지원한다. `CKoukuSaydonLogicRuntime`은 성공 신호와 착지 admission을 처리한 뒤 `bCounterSuccessLanded`, `bEndPatternEarly`를 세운다. 비어 있는 OnSuccess는 후속 Pattern을 만들지 않는다. `Apply_KoukuLogicOutput`은 Pattern/root state와 해당 WORLD owner/pending mechanic을 정리한다. 검사기를 완화하거나 임의 groggy Pattern을 만들어 연결하지 않았다.

## G04. 실행한 자동 검증

기존 `ServerGameplayContractTests_KoukuSupportSurface.cpp`에 필요한 실제 소비자 케이스를 확장했다. 새 test framework나 제품 런타임 경로는 만들지 않았다. 실행용 entry point와 격리 object/EXE는 Git 제외 `out/`에만 있다.

```powershell
powershell -ExecutionPolicy Bypass -File out/KoukuPizzaSummon20260918/server-runtime/compile.ps1 -Incremental
```

격리 x64 Debug 컴파일은 실제 Server/Shared 구현을 링크한다. 최초 컴파일 후 최종 증분 빌드는 `KoukuSaydonBrain.cpp`, `GameRoom_BossSimulation.cpp`, 기존 SupportSurface test TU와 실행 entry point를 재컴파일했다. Product Server.exe/Client.exe를 링크하거나 실행하지 않는다.

실제 실행한 소비자 검증은 다음을 포함한다.

- 8582ms 이동 추적의 첫 tick 6도, 15tick 내 90도 회전, 실제 navigation/root 30tick 동안 이동과 body yaw 유지.
- 20000ms로 수명을 늘려도 첫 회전 6도, grouped tick의 동일 각도 시간, ±180도 경계의 최단 회전.
- 다음 absolute root sample 이후에도 방금 확정한 yaw 유지. 기존 회전 전용 보간 검증 유지.
- 역순인 같은 clock의 JUMP/SLAM을 실제 clone helper가 한 번씩 실행하고 이후 root sample이 중간 높이와 바닥까지 소비.
- 실제 CGameplayCatalog에서 MAP 16행 성공, 17행 거절 시 이전 generation 보존, 잘못된 anchor 거절, 기존 10-field 행의 BOSS 기본값 보존.
- 실제 CGameRoom에서 열 번째 navigation 지점 실패 시 생성된 actor 0명/next ID 보존, 잘못된 높이도 전체 rollback.
- 10개 MAP actor를 absolute pose/yaw로 commit하고 각각 child Pattern 실행, 다음 clone tick에서 10명 유지, parent 중지 시 전부 despawn.
- Counter 성공에 followup이 있는 기존 경로와 OnSuccess가 빈 경로를 각각 실행. 빈 결과에서는 착지/Pattern 종료/owner 정리와 후속 Pattern·transition 미생성을 검사.

Counter 빈 결과 케이스를 포함한 최종 증분 컴파일·링크·실행은 exit 0, `failures : 0`이다. 실제 출력의 `A counter with no outcomes lands and ends the Pattern without inventing a followup`과 기존 followup 케이스가 모두 PASS다.

로그는 `out/KoukuPizzaSummon20260918/server-runtime/server_compile.log`, `server_link.log`, `server_result.log`에 있다. 로그는 PowerShell redirect의 UTF-16 인코딩이다. 새 C++ 파일과 project/filter 등록 변경은 없다. 기존 파일의 UTF-8 BOM 없음/CRLF를 유지했고 변경 Server 파일의 `git diff --check`를 확인했다.

## G05. 실제 게시에서 발견한 Object Collider의 부모 수명 누락

최신 source를 게시하는 통합 단계에서 P31의 첫 generated Object Collider가 `Publish-GameplayBalance.ps1`의 정상 범위 검사에 거절됐다. P31만 현재 저장본으로 다시 투영하여 직접 확인한 값은 부모 Stage 합 6000ms, generated window 40개 각각 end 11000ms였다. `stopYawDegrees=0`은 정상이었다. PowerShell 검사와 Server의 `window end <= Pattern end` 계약은 완화하지 않았다.

`Tools/KoukuSaydonPipeline/world_object_collider.py`는 Effect를 가진 이미 태어난 오브젝트가 WORLD birth deadline 이후 motion을 완주하는 기존 계약을 적용하고 있었다. WORLD box 길이에 강제로 자르면 이 기존 의미가 깨진다. 누락은 이 bake가 **부모 Pattern의 종료 시간**을 전달받지 않는다는 점이었다.

- `bake_windows(..., pattern_end_ms=...)`를 선택 인자로 추가했다. 기존 standalone bake와 birth 이후 finite tail은 유지하고, 부모가 지정되면 collider의 hit/carry cycle 종료를 그 부모 끝까지만 닫는다. 시간 범위 안에서 기존 sample을 다시 bake하므로 track의 마지막 key와 visible도 함께 닫힌다.
- `project_kouku_saydon_composition.py::project_encounter`가 확장된 실제 Pattern 길이를 전달한다. object-collider memo key에도 그 길이를 포함하여 같은 WORLD 배치가 다른 부모 수명에서 이전 bake를 잘못 재사용하지 않게 했다.
- canonical source의 사용자 duration, 오브젝트 motion, Effect 수명은 변경하지 않았다.

`python -m unittest Tools.KoukuSaydonPipeline.test_world_object_collider -q`는 **14 tests / OK**다. 새 테스트는 STOP/HOLD/LOOP와 DAMAGE/HOOK_CAPTURE의 여섯 조합에서 부모 끝의 hit/carry/마지막 key를 검사하고, 부모 시작과 같은 시각에 끝나면 window가 생기지 않는 것도 확인한다. 기존 birth deadline 뒤의 tail 허용 테스트도 계속 PASS다.

최신 P31 실제 투영을 다시 실행해 40개 window가 모두 end 6000ms이고 각 worldTrack도 부모 수명 안에 있음을 확인했다. 실제 `Publish-GameplayBalance.ps1`의 실패한 if guard를 그대로 추출하여 전후 40행에 실행한 결과도 before 40/40 거절, after 0/40 거절이었다. 전후 후보는 `out/KoukuPizzaSummon20260918/server-runtime/p31-latest-before.json`, `p31-latest-after.json`에 보존했다. 세 Python 파일의 `git diff --check`는 PASS다. Product 전체 재게시 성공 여부는 통합 작업에서 별도로 확인한다.

## G06. 통합 반영 및 사용자 화면 경계

통합 담당자가 source revision 1664의 네 domain 게시 성공을 보고한 뒤, 실제 설치된 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`을 독립적으로 다시 읽었다.

- P25의 `PATTERNSUMMONSPAWN`은 정확히 10행이고 모두 MAP, P84 참조다.
- P81의 Pattern과 `COUNTER_WINDOW`가 존재하며 start 0ms, duration 2837ms, endsPatternOnSuccess 1이다. `PATTERNTRACKMOVE`의 followSpeedScale은 1이다.
- 기존 격리 probe를 `KOUKU_DEBUG_DATA_ROOT=Server/Bin/DataFiles`로 실행하여 실제 `CGameplayCatalog::Load()`의 전체 게시본 admission을 확인했다. exit 0, `1 Loaded gameplay bootstrap`이다. 코드 재컴파일이나 새 framework 없이 이미 검증한 실제 catalog 객체를 재사용했다.

행 증거는 `out/KoukuPizzaSummon20260918/server-runtime/published-consumers.json`, 실제 전체 로드 출력은 `published-full-load.log`에 있다. 이 확인은 게시 파일과 Server parser의 admission 성공이며 이미 실행 중인 Server process의 catalog 교체 성공을 뜻하지 않는다. Product build와 실행 파일 적용 상태는 통합 담당자가 별도로 기록한다.

Client/UI를 자율 실행·조작하지 않았다. 실제 Preview의 10명 위치, 입에 붙는 불, 새 실행 파일과 게시 데이터를 로드한 Server Play의 최종 화면은 사용자 확인 범위다. 실행 중인 프로그램의 메모리 draft나 Server catalog가 파일 교체만으로 갱신됐다고 처리하지 않는다.
