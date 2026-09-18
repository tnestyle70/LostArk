# 쿠크 World 칼날 복구 및 Effect 수명 연결 결과

## 실제 반영 상태

통합 계획은 [KOUKU_PATTERN_RUNTIME_REPAIR_PLAN](2026-09-18_KOUKU_PATTERN_RUNTIME_REPAIR_PLAN.md)을 따른다. 최신 저장본 기준 반영은 사용자가 승인한 범위다. `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json`의 stable ID와 변경 필드만 병합했다. 최초 복구 revision 2092 → 2093 뒤, 원래 속도의 Effect 반복 정책을 revision 2094로 반영했다. 최종 source SHA-256은 `a1669e2a3f9d9734fbaec1199ef5cb9b3c424abe1e94cb043a68275996945151`이다.

각 설치는 교체 직전 원본 bytes 재확인, 별도 백업, 임시 파일 parse 확인, 원자 교체를 사용했다. 다른 sequence, 사용자 Transform/scale, 기존 collider와 hook instance는 유지했다. runtime publish와 Product 빌드는 통합 작업에서 별도로 수행한다. 이 결과의 source 설치는 실행 중 Client의 미저장 draft나 Server 메모리를 자동 교체하지 않는다.

## 바닥_즉사칼날의 8개 배치 복구

복구 stable ID는 `world.object.kouku.cutting_blade.state.instant_death`다. 현재 문서에는 count 1과 첫 emission만 남아 있었다. 보존본을 revision 순서로 비교했으며, 가장 최근의 정상 8개 배치는 다음 파일의 revision 2037이었다.

`out/CardDiceScale20260917/staged/Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json`

revision 2036/2037에는 같은 8개 배치가 있으며 조사한 revision 2050 이후에는 첫 항목 하나만 있었다. 복구 필드는 `objectMotion.count`와 `objectMotion.emissions` 두 개다. 원본 8개 모두 delay 0, yaw 222도이며 위치는 다음과 같다.

| 순서 | X | Y | Z |
|---|---:|---:|---:|
| 1 | 3.64499331 | 1 | 11.5199928 |
| 2 | 5.13083076 | 1 | 10.1821394 |
| 3 | 6.61699343 | 1 | 8.84399223 |
| 4 | 8.10387993 | 1 | 7.50519371 |
| 5 | 9.59004211 | 1 | 6.16704702 |
| 6 | 11.0758801 | 1 | 4.82919312 |
| 7 | 2.66041589 | 1 | 12.4065104 |
| 8 | -0.112564072 | 1 | 14.9033136 |

기존 09-12 World Object Group RESULT의 G07 기준으로 yaw 222도는 camera yaw 135도에서 화면 왼쪽 → 오른쪽이다(dot 0.99863). 원본 velocity `[0,0,2]`, angular velocity `[1440,0,0]`, 수명 11000ms와 현재 Transform 키를 유지했다. 일반 칼날 scale 2→3, 즉사 칼날 scale 1→3도 그대로다. 실제 화면 방향은 사용자 검증 대상이다.

## 최신 일반·붉은 Effect를 같은 Object에 연결

각 template에 `slotId=object`, TIME 0, duration 11000, `followObject=true`, root bone, offset/rotation 0, scale 1의 V1 effect track 하나를 연결했다.

| template | 사용자 최신 source Effect |
|---|---|
| `world.object.kouku.cutting_blade.state.1` | `effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_cutting_pjt_01` |
| `world.object.kouku.cutting_blade.state.instant_death` | `effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_cutting_pjt_02_loc_int` |

World Effect provider가 실제 emitted Object의 위치·회전·scale을 샘플한다. 기존 collider도 같은 emission과 Object 위치를 소비한다. 회전하는 칼날 모델과 ground collider의 원래 회전 계약을 바꾸지는 않았다. 두 Object instance는 기존 `motionEnd=LOOP`를 유지하고 `loopFullPresentation=true`를 추가했다.

사용자가 확대한 source Effect와 내부 모델 삭제 결과는 byte 단위로 유지했다. 일반은 element 8개, 붉은 칼날은 9개, 둘 다 modelCues 0, bloomIntensity 1.29999995다. 다른 `.full.restore` 자산으로 대체하지 않았다.

- 일반 Effect SHA-256: `1716c8246fbeecb0cd19f15b102df3a7d69c3ecafb78bb657d8c511b18629797`
- 붉은 Effect SHA-256: `0c768f8fe445ef80d4731d2bde373db12ef908f2e03b7ca833d78f17f193e932`

최초 revision 2093 후보는 `fitEffectToDuration=true`였다. fit은 원본 전체 수명을 11초로 늘려 한 번 재생하므로 원본보다 느려질 수 있다. 최종 revision 2094는 `fitEffectToDuration=false`, `loopEffectToDuration=true`다. 원래 재생 속도를 유지하며 World Effect 창 안에서 반복한다.

## World Effect 반복의 실제 소비 경로

`Client/Public/WorldSequenceDocument.h`의 `WORLD_SEQUENCE_EFFECT_TRACK`에 optional `loopEffectToDuration=false`를 추가했다. `WorldSequenceDocument.cpp`는 parse/save/equivalence와 V1 전용·fit 배타 검증을 담당한다. `WorldObjectTool.cpp`는 `Loop Effect through window` 편집을 기존 draft와 Save 경로에 연결한다. `Tools/MapPipeline/Publish-MapAuthoring.ps1`도 같은 optional field와 배타 검사를 사용한다. 새 파일 또는 project/filter 등록은 없다.

`WorldSequencePlayer_Objects.cpp::Apply_ObjectEffects`는 기존 prepared Effect와 `Spawn_LevelPlacement / Seek_WorldRoot`를 재사용한다.

- native `EmitterLoops=0`가 포함된 source는 기존 `fSourceLoopEndSeconds`로 방출을 박스 끝까지 연장한다.
- 전부 finite인 source는 prepared Effect의 전체 수명(입자 tail 포함)마다 새 occurrence를 재생한다. source 시계는 매번 0에서 시작하지만 follow provider의 Object 시계에는 cycle 시작 나이를 더해, Effect가 칼날의 최초 위치로 되돌아가지 않는다.
- 두 사용자 blade source는 모든 emitter가 loopCount 1이므로 두 번째 경로를 사용한다. 원본 emitter 속도·scale·source JSON과 immutable prepared document는 변경하지 않는다. source 내부 방출과 tail 비율도 원본대로 반복된다.
- 11초 창의 끝에서 기존 wanted-handle 정리가 Effect를 제거한다. Object motion이 LOOP이면 다음 Object epoch가 다음 11초 재생을 소유한다.

기존 `Set_SourceLoopEndSeconds`는 loop0 전용이며 finite만 있는 문서에는 사용할 수 없다. 이를 강제로 통과시키거나 source loop count를 0으로 덮어쓰지 않았다. 이후 공통 불의 finite25개 emitter에서도 같은 오류를 확인해 Composition V1에 finite 반복과 전체 root/bone/source-anchor 시계를 연결했다. native loop0/finite 분기를 AREA 가이드와 gotchas/복원 V2에 기록했다.

## 원본 갈고리·즉사 칼날 리소스와 공굴리기 수명

Composition source는 통합 담당이 최신 revision에 병합한다. 이 작업은 `out/KoukuRepair20260918/world/composition-field-patch.json`으로 다음 stable 의도를 전달했다.

- 기존 world19 표시명은 `바닥_일반칼날`로 정리한다.
- `원본_갈고리`는 `world.sequence.instance.kouku.hook.original_preview`를 가리키는 새 World resource다. 기존 world36 및 이를 사용하는 P33 박스는 보존한다. original hook의 STOP, 11334ms, 15개 emission과 native animation/collider도 보존한다.
- `바닥_즉사칼날`은 위 복구 instance를 가리키는 새 World resource다.

Workbench를 읽기 전용 조사한 결과, Object 목록은 같은 objectResourceId를 defaultMotion 기준으로 걸러서 saved state만 추가해도 append가 막힐 수 있었다. 기존 `Append_WorldObject`는 기본 motion을 다시 찾고, `Append_WorldResource(..., false)`도 object ID 없는 alias를 찾는 구조다. 이 때문에 아래 Composition World Resources에서 선택한 정확한 stable World 정의를 append하도록 통합 담당에게 수정 근거를 전달했다. 이 파일은 이 작업에서 수정하지 않았다.

P81 `세이튼_공굴리기_카운터`의 사용자가 늘린 WORLD 박스는 start 740ms, duration 11832ms였다. `sequence.kouku.saydon.rolling.ball`의 template 수명과 마지막 hidden key를 7568→11832ms로 맞췄다. 공의 TRS·본 anchor·원본 spin을 보존했다. 실제 hide 시점은 pattern 12572ms(740+11832)이며 전체 13528ms pattern 끝으로 임의 연장하지 않았다.

## 검증과 남은 화면 확인

최초 설치 fixture는 실제 `CWorldSequenceDocument` load/save/readback 등가성을 포함해 16 checks / 0 failures였다. 최종 loop 정책은 같은 검사에 fit+loop 동시 활성 거절과 V2 loop 거절을 더해 18 checks / 0 failures다. WorldSequenceDocument, WorldObjectTool, WorldSequencePlayer_Objects 변경 3 TU를 실제 MSVC로 컴파일했다. PowerShell publisher parse, source JSON parse, 변경 파일 `git diff --check`가 통과했다. 기존 C4819 경고만 있었으며 Product 링크/실행 검증으로 대체해 표기하지 않는다.

증거와 백업은 `out/KoukuRepair20260918/world/`에 있다.

- `world.install.before.rev2092.json`, `world.install.before.rev2093.json`
- `world-install-receipt.json`, `world-loop-install-receipt.json`
- `backup-inventory.json`, `world-field-patch.json`, `world-loop-field-patch.json`
- `world-fixture.json`, `world-fixture.saved.json`, `probe-compile.log`, `probe-run.log`

Client/UI를 실행하거나 조작하지 않았다. 실제 8개 칼날의 좌→우 이동, 이동·회전하는 모델/Effect/Collider의 겹침, 11초 전체 Effect 표시와 다음 epoch 반복, 늘린 공 수명 및 새 Resource append 화면은 사용자가 새 코드와 게시 데이터로 확인해야 한다. 원본 finite Effect의 반복 경계에서 보이는 입자 tail/밀도는 이 CPU codec 검증으로 GPU 표시 성공을 주장할 수 없다.
