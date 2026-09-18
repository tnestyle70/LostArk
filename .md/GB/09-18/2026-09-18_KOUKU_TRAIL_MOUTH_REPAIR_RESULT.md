# 쿠크 왼손 트레일과 입 본 부착 실측 결과

대응 계획은 [쿠크 패턴 재생 연결 및 저작 데이터 복구 계획](2026-09-18_KOUKU_PATTERN_RUNTIME_REPAIR_PLAN.md)이다.
이 문서는 왼손 Effect 정본에 반영한 필드와 P27/P81 입 부착 좌표의 계산 근거를 소유한다.
Composition 최종 병합, Product 게시 및 Client 변경의 통합 결과와는 구분한다.

## G00. 완료 범위와 실제 반영

`effect.kouku.gate3.ritual.hand.trail.full.restore`의 흰 트레일 1개 element에 남아 있던
추가 위치와 source segment clipping을 수정하여 Authored 정본에 반영했다. 오른손 지팡이 끝의
`effect.kouku.gate3.ritual.staff.tip.trail.full.restore`와 같은 origin 연결 방식을 적용했다.
기존 붉은 트레일, sprite, 재질, socket, source preview 및 공유 Effect 자체의 수명은 보존했다.

P33 왼손의 최신 저장 수명 24,891ms와 P27/P81 공통 불뿜기의 occurrence scale은 사용자 값이다.
이 값들을 다시 원본값으로 덮어쓰지 않고 stable ID별 필드 delta를 통합 작업에 전달했다.
이 담당 작업에서 Composition, EffectCatalog, tree, 공통 불뿜기 Effect 및 Product를 직접 편집하지 않았다.
Client 실행, UI 조작, Preview/Server Play 화면 판정도 수행하지 않았다.

## G01. 왼손 흰 트레일의 원인과 수정 필드

정본 파일은 `Data/Effects/Authored/effect.kouku.gate3.ritual.hand.trail.full.restore.effect.json`이다.
수정 element ID는 `kouku.4219911.3d960b9052ce1c6d6541`이며 source native emitter는 3007이다.
붉은 트레일 `kouku.4219911.49bb53ef6a74c4034b5c` 및 sprite
`kouku.4219911.97e6d571120c97228b9c`는 변경하지 않았다.

| 필드 | 수정 전 | 수정 후 |
|---|---|---|
| `detail.particle.initialPositionMin` | `[1,0,0]` | `[0,0,0]` |
| `detail.particle.initialPositionMax` | `[1,0,0]` | `[0,0,0]` |
| `particlemodulelocation_1`의 `startlocation.lookupTable` | `[0,100,100,0,0,100,0,0]` | 8개 값 모두 0 |
| `particlemoduletypedataribbon_6`의 `bclipsourcesegement` | `true` | `false` |

흰 트레일만 source StartLocation +100cm가 추가돼 있었다. 실제 설치 `MN_RPCT_05.wmodel`의
`bip001-l-hand` 골격, CModel preScale 0.017 및 socket basis를 함께 계산하면 추가 위치는 약 1.7m다.
이 값은 사용자 occurrence position이나 카메라 위치가 아니라 emitter 내부의 별도 offset이다.
`bclipsourcesegement=false`는 거리 기준으로 다음 입자가 태어나기 전에도 현재 socket과 첫 구간을 연결한다.

기존 builder `Tools/EffectPipeline/build_kouku_ritual_hand_trail.py --left-hand-only`가 이 4개 필드만
바꾸는 후보를 생성하므로 builder 또는 공통 renderer를 추가 수정하지 않았다. 최신 디스크 hash 확인,
백업, 임시 파일 기록과 flush, 교체 직전 hash 재확인, 원자적 교체 및 결과 검증을 거쳤다.
반영 증거는 `out/KoukuRepair20260918/trail/applied.json`이다.

| 항목 | SHA-256 |
|---|---|
| 왼손 반영 전 | `ce64030b56e6d3e7641e7cb8a9edbdb3528244379b0ff6a7fd95701758ea6e4d` |
| 왼손 반영 후 | `3faefc594552a24aa8166e2d7ede84483654ce9953c0e8d56f0ca25930f7c2de` |
| 보존한 오른손 | `7c05712af2509ad76f6de17d35a57f56633ecf8398fecf7dc01f59f7cef6a7c8` |

백업은 `out/KoukuRepair20260918/trail/before/`에 두었다. 실패 시 rollback은 이 작업이 설치한
hash와 일치하는 자기 변경에만 적용하며 이후의 다른 저장본을 덮어쓰지 않는다.

## G02. 현재 저장 수명을 유지하는 occurrence delta

실측한 P33 `KAKULSAYDON_G1_PATTERN_33.presentation.3`은 resource
`kakulsaydon.effect.5e81347e945c7120aa35`, start 3035ms, duration 24,891ms였다.
`out/KoukuRepair20260918/trail/composition-field-delta.json`은 이 stable occurrence에
`loopEffectToDuration=true`를 설정하고 최신 start/duration/resource를 보존하도록 전달한다.

공유 Effect의 `lifeTimeSeconds=3.1748640537261963`이나 particle lifetime을 일괄 늘리는 방식은
같은 Effect를 짧게 사용하는 P61에도 영향을 준다. 이번 수정은 공유 수명을 유지하고 기존
occurrence의 반복 방출 계약을 사용한다. source animation의 재생 속도를 수명에 맞춰 늘이지 않는다.
P33의 실제 `rpct00_att_battle_29_02` chain과 독립 Effect source preview `27_01`의 시간축도 구분한다.

## G03. 불뿜기의 실제 원본 socket과 본 축

입 부착은 공통 asset `effect.kouku.gate3.firebreath.shared`를 사용하는 occurrence의 수정이다.
기존 boss-root offset/yaw를 본 frame에 그대로 복사하면 원점과 축이 달라져 위치·방향이 틀어진다.
`KoukuSaydonPresentationPlayer.cpp::Make_Pivot`은 선택한 본의 축을 정규화한 뒤 occurrence
scale/rotation/position을 적용한다. 따라서 계산도 정규화한 mouth frame을 사용하고 user scale은 보존한다.
공유 asset 내부에는 이미 기존 1.7배가 들어 있으므로 bone scale을 다시 occurrence scale에 곱하지 않는다.

| 대상 | 설치 모델 / clip | 실제 불뿜기 notify | 선택한 원본 socket |
|---|---|---|---|
| P27 | `MN_RPCT_06` / `mn_rpct_06_sk.ao_att_battle_5_02` | `action-4221809/stage-001/notify-010`, stage-003의 같은 notify | `FX_Prj_01` → `bip001-head` |
| P81 | `MN_RPCT_05` / `rpct00_att_battle_26_02` | `action-4219866/stage-001/notify-005` 및 같은 반복 notify | `FX_Prj_02` → `bip001-head` |

두 socket의 원본 위치는 UE `[-3,-7,0]`cm다. 원본 FRotator는 P27 `[Pitch=0,Yaw=52792,Roll=16384]`,
P81 `[0,49152,16384]`이다. 원본 PSK bind와 설치 WModel rest basis를 모델별로 계산해 socket을
투영했으며, 두 모델의 head bind map은 `.01 * reflectZ`와 2e-6 이하 오차로 일치했다.
현재 CModel preScale은 RPCT05 0.017, RPCT06 0.0692이며 실측 bone basis 길이는 각각 약 1.7, 6.92다.
애니메이션은 CAnimation의 실제 cooked tick rate 30을 사용했다.

RPCT06 props에는 별도의 `fx_mouth_01` socket 25가 존재한다. 이는 `bip001-head`에 붙으며
UE 위치 `[-1.90073,-4.83983,0]`, yaw -5461이다. 그러나 위 불뿜기 breath notify가 선택한 socket은
`FX_Prj_01`이다. 기존 RESULT의 cast `Par_X_RPCT_FireCast_01_01_LOC_INT` 설명과 breath 설명을
혼동하지 않는다. RPCT05의 해당 props socket 목록에는 `fx_mouth_01`이 없다.

원본 firebreath velocity module의 lookup table은 앞의 범위 header 2개를 제외하면 UE +X 방출이다.
공유 불뿜기는 UE -Y를 변환한 Client +Z 방출이므로 +90도 yaw를 한 번 적용하여 source socket +X와 맞췄다.
그 socket frame을 현재 발생 시점의 normalized `bip001-mouth` frame으로 변환했다.
이 계산은 본 이름이나 화면의 임의 점으로 추측한 값이 아니다.

## G04. 통합에 전달한 입 부착 필드

`out/KoukuRepair20260918/trail/mouth-field-delta.json`에 패턴 번호 `27`, `81`을 key로 저장했다.
공통 필드는 `anchorKind=BOSS`, `followBoss=true`, `boneTarget=BODY`, `bone=bip001-mouth`,
`boneRotation=BONE`이다. position의 단위는 정규화한 본 frame에서의 m, rotation은 DirectX
`XMMatrixRotationRollPitchYaw`에 대응하는 `[pitch,yaw,roll]` degree다.

| 대상 | `positionOffset` | `rotationDegrees` | 정렬 기준 clip 시각 |
|---|---|---|---|
| P27 | `[0.25844499,0.42992049,-0.00012965]` | `[-74.53557369,-89.95560904,179.99497004]` | 2.366초 |
| P81 | `[0.11236356,0.05043811,0.00000137]` | `[-50.62714241,90.00026607,-0.00048761]` | 0.002초 |

P27 기준은 최초 저장 occurrence start 2366ms이며, 다른 두 저장 occurrence의 start는 2347/2348ms였다.
P81은 stage 시작 3000ms 뒤 occurrence start 3002ms이므로 clip 0.002초다. 당시 P81의 최신 duration은
8646ms였으며 이전 bone RESULT에 기록된 4089ms를 복구 기준으로 사용하지 않는다.

원본은 head socket을 따라가고 사용자 요청은 mouth/jaw 본을 따라간다. 따라서 한 개의 고정
mouth-local transform으로 모든 턱 자세에서 원본 head-follow 결과까지 동일하게 만들 수는 없다.
P81의 32개 후속 sample 차이는 위치 5.62e-7m 이하, 전방 약 0.0305도 이하였다.
P27 원본 방출 구간 2.525316~4.153243초의 98개 sample은 위치 약 2.616cm, 전방 약 2.988도 이하다.
P27의 턱이 닫히는 5.5초까지 포함하면 차이는 약 0.2615m, 30.244도까지 늘어난다.
이는 mouth-follow 후보와 source head-follow를 비교한 값이며, 본 부착 실패를 뜻하지 않는다.
실제 사용자가 저장한 긴 방출 창에서 턱을 따라가는 외형이 적절한지는 화면 확인 대상이다.

## G05. 실행한 검증과 증거

| 검사 | 실제 결과 |
|---|---|
| 4개 필드 semantic diff | 왼손 white 1개만 변경; red/sprite/material/attachment/source preview 보존 |
| 실제 WModel P33 24.891초 sample | 1495개 finite; 수정 전 white 추가 offset 1.6999988~1.7000008m, 수정 후 0 |
| 기존 완성된 CModel CPU probe | 실제 설치 모델과 production anchor 사용; `27_01`, 676 samples, failed 0 |
| CPU white head와 독립 WModel socket 대조 | 174개 sample 최대 오차 9.163e-6m |
| 원본 PSK/socket와 설치 WModel 입 frame | 모델별 bind projection 및 동일 발생 시각 transform 계산 완료 |
| 데이터 | Effect 및 delta/evidence JSON parse 성공 |
| whitespace | 변경 Effect의 `git diff --check` 성공 |

핵심 증거는 `out/KoukuRepair20260918/trail/anchor-measurement.json`,
`cpu/after.legacy.result.json`, `mouth-measurement.json`이다. 측정 script는 같은 폴더의
`measure_anchor.py`, `measure_mouth.py`이며 원본 모델·props·PSK hash도 evidence에 기록했다.

P33 긴 창을 직접 재생하려고 기존 probe 일부만 현재 header로 재컴파일하고 이전 OBJ를 재사용한
확장 probe는 access violation으로 사용할 수 없었다. 이 실행을 성공 증거에 포함하지 않는다.
24.891초에 대한 성공 수치는 WModel 행렬 계산이며, 676샘플 CModel 재생은 별도의 원본 `27_01`
검사다. 이 둘을 긴 P33 CModel 전체 재생 성공으로 합쳐 보고하지 않는다.

## G06. 남은 확인 경계

왼손 Authored 정본 반영과 본 좌표 후보의 수치 검증은 완료했다. Composition 필드 delta의
최종 병합·게시 상태는 통합 작업의 설치 receipt와 RESULT를 따른다. 이 문서의 `applied.json`은
왼손 Effect 단일 파일 설치 receipt이며 Product 전체 게시 성공을 의미하지 않는다.

실행 중 도구의 Reload, Server 재시작, 변경 Client binary 실행, Play Preview/Server Play의
실제 화면은 이번 담당 작업에서 확인하지 않았다. 미저장 draft를 자동 Reload하거나 Client를
종료하지 않았다. 실제 화면에서 손/입에 붙어 보이는지와 긴 방출 창의 외형은 사용자가 확인한다.
