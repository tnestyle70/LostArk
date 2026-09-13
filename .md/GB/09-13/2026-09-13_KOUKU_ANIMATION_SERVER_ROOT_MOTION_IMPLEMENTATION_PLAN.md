# 쿠크 Animation Append 원본 이동의 Server 반영 계획

## G00. 목표와 현재 원인

Animation Append로 배치한 실제 클립의 좌우·전후·상하 이동을 Server Play의 권위 actor 위치에 반영한다. 기존 사용자 배치·클립·시각·콜라이더는 보존한다. 사용자가 계속 Client 편집과 직접 컴파일을 수행하므로 정본 Data, 실행 프로세스, 제품 출력은 변경하지 않고 소스와 out의 격리 검증으로 진행한다.

현재 Append는 clip/time만 저장하고 `_project_stage`에는 원본 root 이동 projection이 없다. `CNpc`는 b_root의 수평 translation을 억제하고 수직만 모델 자세에 남긴다. Server의 Kouku stage 검증은 기존 RootMotion을 거부하고 수동 BossMotion만 절대 XZ 직선으로 소비한다. 결과적으로 원본 클립의 수평 이동은 Server에 전달되지 않는다.

기존 `PATTERNSTAGEROOTMOTION`과 `ROOT_MOTION_SAMPLE` 경로를 확장한다. snapshot에는 이미 XYZ가 있으므로 새 이동 packet이나 별도 모델 runtime은 만들지 않는다. 원본 b_root 이동이 전 구간 0인 in-place 클립은 액터 script/Matinee 이동을 추측하지 않는다.

## G01. 원본 이동 투영

`Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py`는 기존 WModel reader와 publication cache를 사용해 b_root의 부모 skeleton basis와 BossCatalog model preScale을 포함한 실제 displacement를 계산한다. G1 actor의 실측 환산은 단순 .017이 아니라 부모 scale100을 포함한 1.7이며 큰 세이튼은 6.92다. Valtan의 상수를 복사하지 않는다.

선택된 runtimeClip, sourceStart/sourceEnd, startOffset, playMs, playRate, LOOP/HOLD를 같은 시각 함수로 소비한다. Source In을 기준점으로 삼고 LOOP는 한 cycle의 끝 displacement를 누적한다. 끝점이 0이어도 중간 왕복·점프가 있으면 곡선을 보존한다. 기존 512 sample 제한 안에서 정수 ms 입력 표본 대비 축약의 추가 XYZ 오차를 1mm 이하로 제한한다. 원본 fractional key를 정수 ms로 옮기는 양자화 오차는 별도로 측정하며 1mm 보장에 포함하지 않는다. 원본 배율과 속도에 따라 달라지는 이 오차를 고정 cm 기준으로 거절하지 않는다. 밀리초보다 짧아 실제 왕복·급반전이 사라지는 구간과 표본 한도로 표현하지 못하는 곡선은 이유를 보존하며 게시를 거절하고, source crop 경계가 key에 가깝다는 이유만으로 정상 trim을 거절하지 않는다.

생성 stage의 rootMotionSamples는 timeMs와 meter 단위 forward/lateral/up을 전달한다. 수동 BossMotion, enabled charge 또는 REAL_GAZE_TELEPORT가 하나라도 있는 Pattern 전체는 자동 root에서 제외한다. 자동 대상 Pattern에 실제 nonzero 곡선이 있으면 모든 animation binding과 bone sampling의 vertical scale을0으로 맞춘다. 과거 V1 source-bone query가 현재 모델 scale을 소비하므로 한 Pattern 안에 서로 다른 root 소유 방식을 섞지 않는다. 기존 저작 animationRootVerticalScale은 Server up에 먼저 반영한다. Bone Collider의 source anchor도 actor에 이전한 root displacement를 두 번 포함하지 않게 동일 sampling 기준으로 확인한다.

`Tools/GameplayPipeline/Publish-GameplayBalance.ps1`은 기존 root-motion 운반 경로를 확장한다. 기존 3성분 payload는 up=0으로 유지하고 새 4성분을 엄격하게 검증한다. Save에는 WModel 읽기나 publish를 추가하지 않는다.

## G02. Server 소비와 Client 표현

`Server/Public/GameplayCatalog.h`, `Server/Private/GameplayCatalog.cpp`의 기존 sample/parser를 확장하고, Kouku brain과 room audition의 기존 이동 소비자가 XYZ를 적용한다. 기존 Valtan과 player consumer의 동작은 변경하지 않는다.

Stage 진입 후 spawn reset·retarget 등 ENTER action이 끝난 실제 이동 평가 시점에 원점·yaw를 캡처한다. fixed tick마다 원본 sample을 한 번만 소비하며 stage 전환·종료·재생 취소의 상태를 초기화한다. XZ는 기존 navigation/collision 경로로 검증하고 지면 Y와 애니메이션 up을 분리한다. 이동 이후 같은 tick의 collider와 Logic이 확정된 actor 위치를 소비한다.

Client는 기존 `animationRootVerticalScale → PresentationAssetService → ClientReplication → CNpc` 계약으로 자동 이동의 모델 root를 억제하고 Server snapshot XYZ를 표시한다. 새 Engine 원본 샘플 API나 두 번째 이동 runtime은 추가하지 않는다. 이번 요청은 Server Play이며 standalone local preview에 별도 이동 곡선을 합성하는 범위는 포함하지 않는다.

## G03. 검증과 완료 범위

기존 Python test와 WModel reader로 실제 백스텝·점프·왕복 clip의 방향·단위·중간 값·crop·loop·지연을 검사한다. 기존 Server tests에서 sample의 3/4성분 호환, 첫 진입 기준점, 같은 tick 중복, stage 전환과 XYZ 위치·navigation 실패 경계를 확인한다. 변경 C++는 out 격리 컴파일하며 수정한 JSON/XML/Python 문법과 git diff --check를 확인한다. 새로운 검증 framework는 만들지 않는다.

원본 publish와 제품 EXE 실행은 현재 사용자의 편집·컴파일 흐름을 존중한다. 소스 검증, Product 링크, 실제 Server 재시작·Publish/Play, 사용자 시각 확인은 RESULT에 분리한다. 사용자가 직접 `Animation Append → Save → Publish All Patterns → Server 재시작/새 제품 실행 → Server Play`로 최종 이동과 충돌을 확인하며, 에이전트는 Client/UI를 실행·조작·캡처하지 않는다.


## G04. 알비온 상승 준비와 착지 하강 연결 (2026-09-14)

사용자는 알비온 감전 장판 STAGE_7의 지면 관통을 보고하고, nav로 막기 전에 STAGE_6에서 이후 하강량만큼 올리도록 요청했다. 실제 PATTERN_39의 sourceAction4219903을 설치 MN_RPCT_05 모델로 측정했다. `rpct00_att_battle_24_03`은 root 높이40.711243m가 전 구간 고정된 공중 자세이며 상승 곡선이 없다. `_24_04`는 시작 기준으로−3.278311m, `_24_05`는−7.122190m 내려온다. 각 클립 시작을 root delta의 기준점으로 삼는 현재 계약에서 `_03`은0이므로 뒤의 하강만 actor에 누적된다.

STAGE_7과8은 같은 `_04`를 source0부터 각각 재생하므로 STAGE_6에서 총6.556623m를 올려야 두 착지 단계 후 높이가 복귀한다. STAGE_9와10은 `_03`을 두 번 재생한 뒤 STAGE_11의 `_05`로 착지하므로 두 상승창에3.561095m씩 배분한다. 원본40.71m의 고정 root 높이를 상승 이동으로 간주하지 않는다.

`CKoukuSaydonPreviewRootMotion`과 `_project_pattern_root_motion`의 기존 actor root 경로 안에 이 source action의 연속 상승 준비(`_03`)→착지(`_04`/`_05`) 구간만 처리한다. 다음 착지창의 실제 source crop, 속도, 반복과 재생 끝의 순하강을 측정해 앞 상승창의 유효 재생 길이에 비례하는 선형 상승을 더한다. 시각을 직접 샘플링하므로 앞/뒤 탐색은 같은 위치를 반환한다. 그 외 clip/action, 원본 XZ, manual BossMotion/charge/teleport 소유권은 보존한다.

변경 파일은 기존 `Client/Private/KoukuSaydonPreviewRootMotion.cpp`, 대응 public header, `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py`와 기존 test module이다. 새 파일/project 등록과 Shared/Server wire 변경은 없다. 게시 결과의 기존 rootMotionSamples를 Server가 받아 nav/collision 평가 전에 상승이 포함된 actor 후보 위치를 계산한다. Animation Tool과 Complete Play의 기존 local preview evaluator는 같은 보정량을 소비한다. 현재 Composition과 실행 프로세스는 변경하지 않는다.

실제 설치 모델의 단계별 시작·끝 높이, 순/역 탐색, crop/rate와 상승 길이 분배, source action 경계와 수동 이동 제외를 검사하고 변경 C++는 제품 toolset/charset으로 격리 컴파일한다. 소스와 수치 검증, 사용자 Publish/Server 재시작, 화면 판정은 RESULT에 구분한다.


### G04 추가 실측: 고정 조상 행렬의 수치 오차

실제 MN_RPCT_05 `_24_01`을 현재 제품 Engine의 root query로1ms씩 샘플하자5ms에서 실패했다. raw key 상수 검사는 true지만 scale100 조상의 재샘플 행렬은 시작 행렬과2.38419e−5 차이여서 추가 절대1e−5 비교가 실패한다. 이는 다른 시각의 회전 키를 보간하며 생긴 float 오차다. `Engine/Private/Model.cpp::Sample_AnimationRootTranslation`은 이미 검사하는 원본 키 전체의 상수성으로 조상 이동 여부를 판단하고, 승인한 시작 행렬을 기준 basis로 사용한다. 중복 sampled-matrix 비교만 제거한다. 실제 ancestor가 움직이는 clip의 거부는 유지하며 eps를 전역 완화하지 않는다. 변경 Model.cpp는 기존 UTF-8/CRLF를 유지하고 Engine 격리 빌드 뒤 같은1ms native 검사를 반복한다.
