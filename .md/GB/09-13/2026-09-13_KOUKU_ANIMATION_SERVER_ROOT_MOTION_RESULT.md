# 쿠크 Animation Append 원본 이동의 Server 반영 결과

## G00. 구현 상태

Kouku Composition에 Append한 원본 애니메이션의 root 이동을 기존 Publish와 Server Play 경로에 연결했다. Server가 XYZ 위치를 소유하고 Client는 기존 snapshot을 소비한다. source clip이나 사용자의 Composition을 직접 고치지 않았으며 실행 중인 Client·Server, 제품 Data와 제품 EXE도 교체하지 않았다.

이 결과는 소스 구현과 격리 검증이다. 사용자의 새 제품 빌드·Server 재시작·실제 Server Play 화면 확인은 아직 완료 증거에 포함하지 않는다.

## G01. 원본 이동과 게시 계약

- `project_kouku_saydon_composition.py`는 기존 selective WModel reader와 게시 수명 cache로 `b_root` translation을 읽는다. 실제 부모 skeleton basis와 BossCatalog body preScale을 적용한다. stage의 Source In을 원점으로 삼고 Source Out, 속도, 시작 지연, HOLD, LOOP를 반영한다. 반복은 cycle displacement를 누적하며 끝점이 0이어도 중간 왕복·점프는 유지한다.
- 새 이동은 생성 encounter stage의 optional `rootMotionSamples {timeMs, forward, lateral, up}`다. `Publish-GameplayBalance.ps1`은 기존 `PATTERNSTAGEROOTMOTION`에 Kouku만 네 성분을 출력한다. player/Valtan은 기존 세 성분을 유지한다. World 게시기의 실제 Kouku 소비자는 optional stage 필드를 허용하므로 Valtan 전용 validator를 수정하지 않았다.
- 수동 BossMotion, 활성 charge 또는 REAL_GAZE_TELEPORT가 있는 Pattern 전체는 자동 이동에서 제외한다. 자동 이동이 있는 Pattern은 모든 animation binding 및 Bone Collider source bake의 root vertical scale을 0으로 맞춘다. 저작 vertical scale은 Server up에 먼저 적용하므로 수직 이동을 두 번 표시하지 않는다. 기존 CNpc·ClientReplication의 소비 계약을 그대로 이용하고 Client/Engine에 두 번째 이동 runtime을 추가하지 않았다.
- Save에는 WModel 분석이나 게시를 추가하지 않았다. 선택 그룹과 이동 기능 모두 기존 문서 저장·CAS·게시 경계를 유지한다.

## G02. Server 소비

`ROOT_MOTION_SAMPLE` 끝에 `fUp`을 추가하고 기존 세 성분 bootstrap은 up=0으로 읽는다. 새 Kouku stage는 zero origin, 정확한 stage endpoint, 증가하는 시간, 유한·범위 내 XYZ와 2~512 표본을 검증한다. 수동 이동과의 중복도 서버에서 거부한다.

Kouku Room은 ENTER의 spawn reset·retarget 이후 처음 이동을 평가할 때 stage 기준점·yaw를 잡는다. 원본 절대 궤적을 stage 기준점에 적용하고 같은 tick의 Logic 판정 전에 navigation과 body sweep을 검사한다. 같은 tick 중복 소비를 막으며 stage 전환·종료·취소·사망에서 표본과 기준점을 정리한다. 다음 stage는 앞 stage에서 확정한 XYZ에서 시작한다.

XZ 검증과 source up을 지면 높이와 분리했다. 벽 충돌로 XYZ가 함께 잘리면 최종 XZ의 지면을 다시 구하고 지면 성분만 보정한다. 이 수직 보정도 collision을 확인하며 실패하면 위치 전체를 유지한다. 기존 snapshot에 XYZ가 있으므로 Shared packet이나 protocol version은 바꾸지 않았다.

## G03. 원본 수치와 정밀도

- G1 Saydon `rpct00_att_battle_34_02`: 원본 1.6초 종료의 모델 X 이동은 −6.79999959m, 1.1초 중간값은 −7.28590536m다. source 0.4~1.2초를 2배속으로 재생하면 0.4초 끝 displacement는 −2.81651344m다.
- G1 Saydon `rpct00_att_battle_1_01`: 시작·끝은 0이지만 0.8초에 X −0.79404486m가 있으므로 곡선을 생략하지 않았다.
- G1 Kouku `rpcz00_att_battle_7_01`: 원본 7.4초, vertical scale 1에서 최대 up 25.17098073m, 끝은 약 0이다. 이는 해당 actor 배율의 수치이며 다른 관문·사용자 배율의 높이로 일반화하지 않는다.
- 설치 모델의 374개 고유 clip을 원본 속도·full range·vertical scale 1로 조사했다. 양자화 전 해석에서 animated ancestor·중복 root·잘못된 native timing은 없었다. 축약 전 정수 ms 양자화 최대 오차는 Saydon 19.779mm, G1 Kouku 16.292mm, G2 Kouku 11.551mm, BigSaydon 2.197mm였다. 이 값은 임의 속도·crop의 상한 보장이 아니다.

기존 wire가 정수 ms이므로 원본 fractional key를 정확히 전달한다는 주장은 하지 않는다. 정수 ms 입력 표본을 최대 512개로 축약하는 추가 XYZ 오차는 1mm 이하로 제한하고, native key 양자화 오차는 별도로 구분한다. 고정 cm 기준으로 정상 배율·클립의 Append를 거절하지 않는다. 밀리초보다 짧아 사라지는 왕복·급반전과 표본 예산 초과는 오류로 처리한다. CLI Publish/Validate summary의 `animationRootMotion`은 `stageCount`, `maxQuantizationErrorM`, `maxReductionErrorM`, `maxNativeErrorM`을 각각 기록한다.

원본 수치 증거는 `out/KoukuRootMotionOriginalProbe.json`, 양자화 조사는 `out/KoukuRootMotionQuantizationScan.json`이다.

## G04. 검증

- Server Debug 전체 컴파일·링크 성공. `out/KoukuAnimationRoot20260913/Server/Intermediate`, `Linked`로 격리하고 `BuildProjectReferences=false`, `LostArkPublishRuntimeData=false`를 사용했다.
- 기존 `--kouku-bundle-contract-test` 전체 성공(`failures : 0`). 3/4성분 호환, 잘못된 표본 거부, delay·XYZ·왕복·yaw, ENTER 후 기준점, 같은 tick 중복 방지, navigation 실패 보존, stage 전환·자연 종료·취소·사망 상태 정리를 확인했다.
- 기존 `--kouku-support-surface-contract-test` 전체 성공(`failures : 0`). 실제 지지면 높이와 벽에 잘린 root 이동의 지면 침하 재현을 추가하고 수정 후 검사했다.
- Python root 관련 14개 검사 성공: 신규 12개와 기존 root vertical 2개다. 실제 백스텝·crop·속도·loop·원위치 복귀·G1/G2 점프·native cache, 3/4성분 실제 PowerShell stage 직렬화와 잘못된 입력 거부를 포함한다. 1ms 안에서 사라지는 단일 축 왕복·XY 사각형 왕복·전진 중 상하 왕복을 거절하고 native key 가까이 자른 정상 crop은 허용했다. 신규 12개는 독립 검토에서도 성공했다.
- 기존 vertical projection 검사는 무관한 live lane lifetime fixture를 좁게 격리하고 수동 이동의 저작 scale 유지, 자동 이동의 scale 0, 수동 기본값의 필드 생략을 확인하도록 갱신했다. 다른 실제 source pose/cache 검사는 그대로 성공했다. 기존 World encounter admission 검사도 성공했다.
- 현재 저장 문서에서 expanded PRODUCT인 11개 Pattern의 자동 이동 추출은 오류 없이 처리했다. 진단 시점 6개 Pattern에 이동 표본이 있었고 나머지는 수동 이동 소유 또는 in-place였다. 원본 Data를 read-only로 조사한 결과이며 제품 출력으로 게시하지 않았다.
- Python 문법 검사, PowerShell 전체 AST parse, 임시 생성 JSON roundtrip과 `git diff --check` 성공. 그룹 검사는 별도 Collider 그룹 RESULT에 있다.

빌드·Server 검증 증거는 `out/KoukuAnimationRoot20260913/Server/server-build.log`, `kouku-bundle-contract.log`, `kouku-support-surface-contract.log`다. Python focused 증거는 `out/KoukuAnimationRootMotionFocusedTests.log`다. 광역 Python module의 기존 fixture 불일치는 Collider 그룹 RESULT에 구분되어 있으며 전체 광역 검사를 성공으로 기록하지 않는다.

## G05. 적용과 남은 경계

사용자가 새 Client·Server를 빌드하고 새 Server 소비 코드를 재시작으로 적용한 뒤, Workbench에서 `Animation Append → Save → Publish All Patterns → Server Play`를 수행한다. 최초 코드 교체 후의 데이터 게시 갱신은 기존 Product admission을 따른다. 게시 도중의 미저장 편집은 게시 완료 후 별도로 Save/Publish한다. 마감 시 process 확인에서 Client PID 45900, Server PID 48036은 기존 Debug EXE로 계속 실행 중이었으며 에이전트는 이 프로세스를 조작하지 않았다.

원본 root가 0인 in-place clip의 별도 actor script·Matinee 이동은 자동으로 만들어지지 않는다. 수동 이동이 있는 Pattern은 기존 이동 경로를 유지한다. 독립 local Preview의 이동 runtime은 이번 범위에 추가하지 않았다. fixed timeline의 기존 `Logic → Brain → ENTER` 순서상 다음 stage 이동의 첫 적용에는 최대 한 Server tick 경계가 있으며, 이후 sample은 해당 시각으로 따라간다. 이 격리 검증은 사용자 화면의 최종 위치·타이밍 판정을 대신하지 않는다.


## G06. 알비온 상승 누락과 순차 착지 누적 수정 (2026-09-14)

사용자는 `쿠크세이튼_알비온_감전장판`에서 STAGE_7이 바닥을 관통한다고 보고했다. 실제 저장 대상은 `KAKULSAYDON_G1_PATTERN_39`, actor `MN_RPCT_05`, source action4219903이다. nav 제한으로 하강을 잘라내기 전에 STAGE_6에서 실제로 올려 달라는 요청을 반영했다.

### 설치 모델의 원인 실측

설치된 `Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel`의 parent basis와 model preScale0.017을 포함해 측정했다. STAGE_6·9·10의 `rpct00_att_battle_24_03`은 source root up40.711243384m가 전 구간 동일하다. 공중에 놓인 자세는 있으나 **원본 상승 이동량은0**이다. 각 clip 시작을 기준으로 displacement를 구하는 기존 actor root 전달에서는 상승이 생성되지 않는다.

STAGE_7·8의 `_24_04`는 source up3.278311432m에서0으로 내려오는 서로 독립된 두 occurrence다. 두 stage가 각각 source0부터 시작하므로 순하강−3.278311432m가 두 번 누적된다. STAGE_11의 `_24_05`는 순하강−7.122190442m다. 따라서 STAGE_6/7만 맞추면 STAGE_8에서 다시 관통한다.

### 반영한 이동 계약

`CKoukuSaydonPreviewRootMotion::Prepare`와 publisher의 `_apply_albion_takeoff_motion`은 source action4219903의 **연속 `_03` run → 연속 `_04`/`_05` run**만 연결한다. 다음 착지창의 실제 source crop·재생속도·반복·끝 시각으로 산출한 순하강을 앞 상승창의 유효 재생 길이에 비례해 분배한다. `_03`의 고정40.71m를 그대로 actor 이동에 더하지 않는다. 샘플 시각에 따른 선형 상승이며 누적 frame delta에 의존하지 않는다.

| 단계 | 원본 clip | 이번 추가 상승 | 단계 완료 높이(해당 상승 run 시작 기준) |
|---|---|---:|---:|
| STAGE_6 | `_24_03` | +6.556622863m | 6.556622863m |
| STAGE_7 | `_24_04` | 0 | 3.278311432m |
| STAGE_8 | `_24_04` | 0 | 0m |
| STAGE_9 | `_24_03` | +3.561095221m | 3.561095221m |
| STAGE_10 | `_24_03` | +3.561095221m | 7.122190442m |
| STAGE_11 | `_24_05` | 0 | 0m |

위 수치는 현재 저장된1000ms 상승창과 원본 속도에 해당한다. 길이나 crop을 바꾸면 실제 착지 순하강으로 다시 계산한다. 다른 source action과 그 사이에 끼어든 다른 clip은 연결하지 않는다. 원본 XZ와 착지 곡선, 수동 BossMotion/charge/teleport 소유권은 보존한다. Server는 기존 rootMotionSamples의 up을 받아 nav/collision 전 후보 actor 위치를 계산하고, Animation Tool과 Complete Play는 기존 preview root evaluator를 사용한다. packet이나 Composition 저장 필드를 추가하지 않았다.

STAGE_4의 원본−0.003717496m 변화는 보존했다. 따라서 Pattern 시작의 절대Y로 모든 원본 미세 움직임을 강제고정한 수정은 아니다. 이번 높이 복귀 증거는 **각 상승 run 시작 높이로의 복귀**를 의미한다.

### 동일 조사에서 확인한 root query의5ms 실패

현재 제품 Engine DLL과 실제 `_24_01`을1ms 간격으로 샘플하자5ms에서 `Sample_AnimationRootTranslation`이 실패했다. 조상 bone1의 원본 key들은 `Is_BoneTransformConstant=true`인데, 동일 회전 key를 다시 보간한 scale100 행렬은 시작 행렬과 최대2.38419e−5 차이가 났다. 중복된 sampled matrix의 절대1e−5 비교가 이 float 오차를 실제 조상 이동으로 거절했다.

`Engine/Private/Model.cpp`는 원본 key 전체의 상수성 검사를 유지하고, 그 검사를 통과한 초기 조상 행렬을 고정 basis로 사용한다. 중복 행렬 비교만 제거했다. finite 검사, 순환/부모 순서 검사, 실제 움직이는 ancestor의 거부는 유지했고 전역 epsilon은 바꾸지 않았다.

### 검증과 적용 상태

- Python 기존 `KoukuAnimationRootMotionTests`15개 모두 성공. 실제 설치 알비온 반복 착지, 수정한 crop/rate/delay/상승 길이 비율, 다른 action/clip 격리, finite 하강 한도, 기존 native root·publisher wire 회귀를 확인했다.
- 변경 PreviewRootMotion C++는 MSVC14.44, SDK10.0.26100.0, Debug/제품 기본 charset으로 새 컴파일했다. 새 Engine 링크는 현재79개 제품 translation unit의 source/header dependency 시각을 검사해 변경된 Model.cpp만 새 컴파일하고 최신 기존 object들과 `out/AlbionAscent20260914/LinkedEngine`으로 격리했다. 제품 출력은 바꾸지 않았다.
- 실제168bone/249clip 설치 모델을 CPU에서 읽고 GPU/Client/UI 없이 현재8개 stage의12,001개1ms 위치를 검사했다. native Client와 publisher XYZ 차이 최대0.000997332m, 순/역 탐색 결과는 동일값이다. STAGE_6 추가 상승6.556622505m, STAGE_8 높이 복귀 오차7.31e−8m, STAGE_11 높이 복귀 오차0m였다.
- 새 Engine과 evaluator로 기존 실제 backstep/crop/loop/delay/yaw/suppression 복원/animated ancestor 거절573개 검사도 모두 성공했다. 실제 ancestor의 시작·끝은 같고 중간만 움직이는 잘못된 입력을 계속 거절하며 기존 출력값을 보존한다.
- Python AST parse, 증거 JSON parse, 해당 파일의 `git diff --check`를 확인했다. 기존 미커밋 diff는 보존했고 수정 전 백업과 이번 증분 diff를 out에 남겼다.

증거: `out/AlbionAscent20260914/source_motion.json`, `source-receipt.json`, `before_failure.txt`, `python-tests.log`, `probe.run.log`, `regression.run.log`, `engine-build.log`, `incremental.diff`. 이는 소스/수치 검증이며 사용자 화면의 visual PASS가 아니다. 실행 중인 Client·Server 및 Composition은 직접 변경하지 않았다. 사용자가 새 Engine+Client 빌드와 재실행 후 전체 Pattern preview를 확인한다. 아래 게시 재확인에서 현재 Server runtime도 수정된 상승곡선과 일치했으며, 새로 시작하는 Server가 이 파일을 읽는다.


### 제품 게시 파일 재확인

현재 Composition을 기존 `prepare_publication/projected_outputs` 경로로 다시 계산한 Encounter와 patternbindings 두 JSON은 현재 정본과 차이가0이다. 기존 `Publish-GameplayBalance.ps1 -Mode Publish -OutputRoot out/AlbionAscent20260914/publish-review/Gameplay`를 실행하여 생성한 격리 후보도 제품 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`과 바이트 단위로 같았다. 수정시각만으로 stale 여부를 판정하지 않고 실제 wire를 다시 확인했다.

제품 `PATTERNSTAGEROOTMOTION`에서 알비온 stageIndex2(STAGE_6)의1000ms up은6.5566228632640948m, stageIndex5/6(STAGE_9/10)은 각각3.5610952209681304m다. 이미 수정곡선이 게시돼 있어 추가 게시나 Composition 덮어쓰기는 필요하지 않았다. 격리 publisher 전체검증이 성공했고 제품 파일을 바꾸지 않았다. 후보/제품 공통 SHA-256은 `d75f5367bf6306029c9785c282a5482e89ca49affc8107bcf9529c00f130b1d2`이다. `publish-review/receipt.json`, `gameplay-publish.log`, `bootstrap-diff.json`이 근거다. 사용자 요청에 따른 제품 전체 빌드와 재실행 상태는 이번 기능의 상위 작업 결과에서 기록한다.
