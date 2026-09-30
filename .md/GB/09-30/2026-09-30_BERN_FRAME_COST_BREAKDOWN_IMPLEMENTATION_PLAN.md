# 베른 프레임 병목 분해 구현 계획

## G00. 저장된 ON/OFF 캡처의 기준

`Client/Bin/ProfilerCaptures/베른성_DirectionalLightOff_20260930_225855_580_frame1021_69252_0.json`과
`베른성_DirectionalLightOn_20260930_225936_967_frame1443_69252_1.json`을 비교한다.
각 120 CPU frame, 116 valid GPU frame, raw CPU drop 0이다. OFF/ON 평균 간격은
69.240/68.235ms, NonBlend CPU는 21.392/21.130ms, 전체 animation은 4.109/4.101ms다.
directional 값 자체는 이전 capture metadata에 없으며 파일명과 사용자 설명에 따른 구분이다.
export camera가 같아도 모든 history frame의 상태가 같았다고 단정하지 않는다.

맵 batch 16,424개, visible instance 973개, 실제 전체 draw 약 1,211회다. 모든 batch를
GPU로 그린다는 뜻은 아니다. authored-visible batch는 큐에 들어간 뒤 최종 camera에서
cull한다. NPC는 authored visibility만 검사하며 camera 밖에서도 pose/update/draw를 수행한다.
환경 effect VisibleUpdate는 약 7ms, ImGui는 7~8ms로 별도의 큰 비용이다.

## G01. Profiler.h / Profiler.cpp의 고빈도 CPU 누적

기존 raw scope는 상세 OFF에서 빠지고 상세 ON에서는 8,192개 cap에 도달할 수 있다.
`EProfilerWork`의 고정 배열에 호출수와 inclusive CPU 경과 시간을 main thread에서 합산한다.
`CProfilerWorkScope`는 begin/end frame과 capture epoch를 확인하고 capture OFF에서는
clock·문자열 intern·mutex·heap allocation을 하지 않는다. GPU query를 추가하지 않는다.

분류는 MapBatchRender, MapBatchVisibility, MapBatchMaterial, MapBatchPass, MapBatchDraw,
MapObjectRender, MapWaterRender, NpcUpdate, NpcLateUpdate, NpcRender,
AmbientVisibility, AmbientAdvance, AmbientSubmit이다. 부모/자식 누적은 합산하지 않는다.
동일 프레임의 Frame → Snapshot → JSON과 LiveStats → ProfilerTool Workload로 연결한다.
JSON v3에 additive `cpuWork`를 추가하고 기존 field와 counter ordinal은 유지한다.

## G02. 실제 소비자와 작업량

`MapStaticBatchObject.cpp`에서 empty Render, visibility cache hit/rebuild, batch bounds reject,
실제 upload bytes를 기록한다. G04 적용 뒤 visibility는 최종 camera 제출에서 준비하고 Render가 재사용한다.
`MapAssetObject.cpp`의 실제 Render 경로를 물/나머지 개별 map으로 나눈다.
`Npc.cpp`에서 Update/LateUpdate/Render를 기록하고 authored-hidden update 호출을 별도로 센다.
`Effect_PresentationService.cpp`의 ambient visibility/advance/submit을 나누며 bounds 없이
진행한 수를 기록한다. 초기 계측은 동작을 유지하며 실제 draw/pose 생략은 G04~G05에서 처리한다.

`Layer.h/.cpp`와 `Object_Manager.cpp`는 layer 생성 시 level/tag별 phase scope 이름을 한 번
만들고 Priority/Update/PostPhysics/LateUpdate의 layer 전체 비용을 기존 CPU scope에 연결한다.
instance별 raw scope를 추가하지 않는다. 정적 맵의 빈 phase 순회와 NPC/update를 구분한다.

## G03. 계측 검증

새 C++ 파일은 없으므로 vcxproj/filter 등록은 필요 없다. 기존 파일 encoding/newline을 유지한다.
계측 core의 disabled/reset/stale token/main-thread guard 및 JSON 소비를 focused 검사하고,
Engine public header 변경은 정상 Debug Product Build로 SDK와 Client까지 확인한다.
다른 작업이 실행 중인 build와 충돌하지 않는다. 변경 diff와 문서 경로도 확인한다.

Client/UI 실행은 사용자가 한다. 새 Debug에서 같은 위치·camera·창 조건으로 120 frame을
저장하며 Detailed per-draw CPU scopes는 OFF로 둔다. `cpuWork`와 Layer scope가 빈 batch
검사·material/pass/draw·NPC·물·ambient 비용을 구분한다. 창을 닫은 비교 capture는 별도로 둔다.
Release도 같은 조건으로 측정해야 실제 제품 성능을 판단할 수 있다.

FPS 개선, GPU 포화, NPC instancing 효과, 항구 marker의 주병목 여부는 새 실행 증거 없이 완료로 기록하지 않는다.


## G04. 최종 카메라 기준 제출과 빈 update phase 제외

사용자가 화면 밖 렌더링·애니메이션의 실제 생략을 요청했다. 이어 현재 직접 빌드 중이므로
EXE 종료 뒤 반영할 후보를 준비하라고 지정했다. 추가 최적화는 live C++에 설치하지 않고
`out/BernVisibilityCandidate20260930`의 상대 경로 후보, baseline SHA와 patch로 준비한다.
초기 계측 변경은 이 지정 이전 live source에 들어갔으며, 새 EXE 적용 완료와 구분한다.

Engine의 기존 GameObject/Layer/ObjectManager 경로에 최종 카메라 제출 단계를 추가한다.
`CRenderer::Draw -> Submit_FrameProviders -> Submit_FinalCameraObjects -> shadow/world passes`
순서로 최종 camera/light를 사용한다. `Uses_FinalCameraSubmission`을 선택한 기존 객체만
Layer의 비소유 phase 목록에 등록하며 기존 owning list가 객체 수명을 유지한다.
MapStaticBatch/MapAsset/Npc가 소비하고 새 Manager나 별도 모델 런타임을 만들지 않는다.

`Get_UpdatePhaseMask`의 기본값은 기존 네 phase 전부다. 정적 map은 UPDATE/LATE만 선택해
Priority/PostPhysics의 빈 함수 호출 자체를 생략한다. 등록/삭제/중복·순서는 기존 Layer
계약을 보존한다. scalar shader/water elapsed time은 유지하여 재진입 시 물·바람 위상이 깨지지 않는다.

MapStaticBatch는 최종 단계에서 visibility payload를 한 번 준비해 empty batch를 NONBLEND
큐에 넣지 않는다. 동일 frame의 Render는 준비된 payload를 소비한다. shadow OFF에서는
caster 큐 제출과 준비를 생략하며 ON에서는 camera와 별개의 light-volume 판정을 따른다.
MapAsset은 최종 camera를 통과한 실제 material group만 큐에 넣고 불필요한 scene-color
요청을 하지 않는다. invalid bounds/camera와 upload 실패는 기존 경로로 되돌아간다.

## G05. NPC 포즈 계산 생략과 재진입

정상 town NPC에 명시된 visual pose culling을 활성화한다. 무기/모자, source vertex 변형,
외부 bone 소비자, action/effect cue, network window, afterimage 등 종속성이 있으면 기존
eager 경로를 유지한다. 효과·부착점을 화면 밖이라는 이유로 잘못 끊지 않는다.

Engine CModel은 실제 WModel의 bone 영향 bounds, inverse bind, rest transform, 모든 clip의
translation/scale key 상계와 parent chain으로 보수적인 전체 animation sphere를 계산한다.
현재 pose나 샘플 몇 개의 AABB를 전체 clip bound로 대체하지 않는다. 지원하지 않는 자료는
실패하여 기존 갱신을 유지한다. root motion suppression/pretransform을 같은 단위로 반영한다.

`Advance_AnimationClock`은 기존 loop/finished/cursor/blend 시간을 유지하되 channel sampling,
bone combine/palette 계산을 생략한다. 최종 카메라에서 bound가 완전히 밖이면 pose와 render
제출을 건너뛰고, 재진입 frame에는 `Play_Animation(0)`으로 현재 시각의 pose만 복구한다.
network interpolation, collider와 action/effect 의미 시계는 유지한다. bone/socket 소비나
clip 전환이 먼저 발생하면 pending pose를 복구한다. NPC 후보/제외/포즈 복구 counter를 연결한다.

## G06. 후보 검증과 설치 경계

기존 Layer의 default/opt-in/삭제/중복 등록/phase 중 추가와 16,424개 객체의 빈 phase 제외를
실제 후보 함수 추출 fixture로 확인한다. map queue/cached cull/실패 보존, NPC sphere·clock
재진입, 실제 설치된 town model의 conservative envelope를 범위에 맞게 검사한다.
검증 산출물은 out에 격리한다. Product build·EXE/DLL 교체·Client 실행은 준비 상태와 구분한다.

사용자가 후속으로 최적화를 전부 반영해 빌드를 진행하도록 명시 승인했다. 최신 live source와 baseline hash를 재확인한다.
다른 작업 변경은 field/hunk 기준으로 보존하고 충돌 파일 전체를 덮어쓰지 않는다. 그 다음
정상 증분 Debug/Release Product Build로 Engine SDK/Client까지 검증한다. 최종 FPS와 화면 판정은
동일 카메라의 사용자 재캡처로 확인하며 60/100fps를 미리 보장하지 않는다.
