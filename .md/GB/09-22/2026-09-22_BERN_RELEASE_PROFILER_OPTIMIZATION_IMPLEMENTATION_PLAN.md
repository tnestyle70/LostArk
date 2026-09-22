# 베른성 캡처 기반 Release 계측·렌더링 최적화 구현 계획

## G00. 현재 실측과 범위

한국어 베른 캡처 다섯 개를 원본 JSON에서 분석한다. 기록에 있는 frame interval, CPU inclusive/self,
유효 GPU pass, 작업량과 누락 표본을 구분한다. 현재 브랜치는 `GB/collider-pattern-bug-fix`이며
다른 세션의 렌더링·게임플레이 미커밋 변경을 유지한다. Client/UI 실행과 화면 판정은 사용자에게 남긴다.

풀줌아웃 첫 프레임은 map placements 50,017, batch 16,421, draw 1,071, indirect draw 0이다.
기존 LOD는 존재하지만 GPU dispatch 비용 때문에 index×instance 294,912 이상의 일부 불투명 재질에만
허용된다. 캡처의 CPU drop은 누적값이므로 이번 프레임의 계측 누락과 구별할 수 없다.

## G01. Profiler.h/.cpp와 실제 렌더 소비자

기존 `CProfiler`를 확장한다. `FProfilerFrame::DroppedCpuScopes`는 해당 프레임에 귀속된
완료 scope 누락 수이며, 누적값은 그대로 유지한다. Reset은 프레임 간격 기준점을 초기화한다.
기존 counter ordinal 뒤에 map culling, LOD 선택·원본/제출 index, shadow cache/caster,
light 제출 작업량을 추가한다. counter는 실제 작업 지점에서 기록하고 object마다 새 scope를 만들지 않는다.
필요한 CPU scope는 light record 작성/제출과 shadow cache 복사 같은 batch/pass 경계에 둔다.
대량의 map batch/mesh CPU raw scope는 `CProfilerDetailScope`로 선택 수집한다. 기본은 pass와 counter이며
상세 모드 변경은 다음 frame boundary에서 적용하고 각 frame에 모드를 기록한다. 선택 window만 Snapshot에서
복사하여 저장할 120프레임을 위해 최대 1200프레임 전체를 복사하고 버리는 비용을 없앤다.

## G02. MainApp → ProfilerTool → ProfilerCaptureIO

Debug와 Release에서 기존 ImGui/CProfilerTool을 공유한다. 사용자 요청으로 F7을 profiler 창
열기/닫기 키로 허용한다. 첫 열기는 수집을 활성화하고 창을 닫아도 명시적으로 멈출 때까지 수집한다.
창의 Capture/Reset/이름/Save JSON으로 조건을 분리해 비동기 저장한다. F1 개발 도구는 Debug 전용이다.
JSON v3에 build/adapter/해상도/장면/렌더 설정의 export 시점 metadata, 분포 요약, GPU 유효율,
CPU 부분 계측 상태를 additive field로 기록한다. 저장 시점 설정을 과거 모든 프레임의 설정으로 단정하지 않는다.

## G03. StaticMeshLod → Mesh → MapStaticBatchObject

CPU에 이미 있는 화면 오차 입력을 단일 compute dispatch로 다시 계산하는 비용을 제거한다.
`CStaticMeshLod::Select_Range`는 indexCount/firstIndex/level을 반환하고 `CMesh`가 기존 index buffer를
직접 `DrawIndexedInstanced`로 제출한다. 생성된 geometry, 0.25 pixel 오차 기준과 material admission을
유지하며 invalid/near 입력은 원본 geometry로 되돌린다. 동일 선택 fixture와 직접 draw 비교를 먼저 확인한 뒤
작은 draw의 기존 dispatch 상환 한계를 제거한다. 여러 instance를 감싼 sphere가 LOD를 막는 경우에는
각 instance의 보수적인 투영 상한으로 동일 품질 한계를 유지한다.

새 영구 C++ 파일은 만들지 않는다. 제거된 compute 소비자가 확인되면 해당 shader의 프로젝트·filter·배포
등록만 정리한다. map/mesh 경로 공유로 베른 외 맵에도 같은 조건에서 적용하며 개선 FPS를 미리 약속하지 않는다.

## G04. 계측 후보 전수 조사와 검증

nav·Server tick·network·map cull·mesh/instancing·shadow·light/prebaked·material·animation·particle/pool·
worker·메모리·IO·ImGui·Present를 실제 함수와 기존 계측에 연결한 가이드를 작성한다.
Release 단독 실행 기준 반복 A/B와 Debug 정확성 검사를 분리한다. DOD/AoS/SoA/worker/fiber 효과는
동일 결과·동일 데이터·동일 compiler 조건의 kernel 비용과 전체 프레임 critical path를 각각 측정한다.

수정 소스의 인코딩을 유지하고 정상 Product Debug/Release Build, 관련 CPU/LOD/JSON focused 검사,
XML parse 및 diff-check를 수행한다. 다른 세션의 같은 출력 빌드와 겹치지 않는다.
실행한 검사만 RESULT에 기록하며 사용자의 실제 캡처와 화면 판정 전 FPS 개선·visual PASS로 기록하지 않는다.

## G05. ServerNavigation::Find_Path scratch 재사용

추가 조사에서 매 A* 요청마다 grid 전체 costs/parents/closed 배열을 할당·초기화하는 경로를 확인했다.
함수 내부 thread-local scratch와 query generation으로 실제 방문 cell만 초기화하는 후보를 기존 navgrid와
동일 시작/목표 입력으로 비교한다. 재진입에는 별도 임시 scratch를 사용하고 region dispatch는 기존대로 유지한다.
경로·실패 결과·heap tie 순서·Server 권위가 같고 유의미한 비용 감소를 확인한 뒤 해당 함수에 한정 반영한다.

## G06. 인스턴스 입력과 중복 재질 API 호출

추가 요청에 따라 SR의 atlas·월드 정점 bake와 실제 단일 draw를 현재 Bern 배치 키와 대조한다.
설치된 Bern에서는 lightmap/RNM·static shadow 텍스처 차이가 material variant를 나누므로
asset key만 느슨하게 바꾸지 않는다. texture array와 spatial chunk의 큰 구조 변경은 정확한
동일 재질 조건·메모리·컬링 손익을 확인하는 별도 후보로 문서화한다.

즉시 적용 가능한 범위는 `MapAssetRenderUtils` 재질 binder에 명시적인 instanced 입력 방식을
전달하는 것이다. `MapStaticBatchObject`의 실제 instance vertex에 이미 있는 lightmap/average/
directional/static-shadow scale-bias를 비인스턴스 global로 다시 설정하는 호출과 해당 instanced
shader에 없는 character 전용 reset만 생략한다. 일반 object와 preview의 기본 경로는 유지한다.
실제 HLSL의 소비 경계를 확인하고 baseline/candidate의 재질 입력·수치 출력 및 API 요청 수,
Debug/Release 최소 컴파일을 비교한 뒤 반영한다. 동작을 추측한 전역 state cache는 추가하지 않는다.


## G07. 화면 밖 이펙트·광원 제출 확인

발탄 이동 표식은 이미 final-camera sample culling을 사용하지만 8m bound에 16/32m padding을
더한다. Bern의 91개 ambient SOURCE_LOOP placement는 거리만 제한하며 화면 밖에서도 연산한다.
실제 trigger 권위와 표현을 분리한다. Marker는 기존 history 재진입을 유지한다.
LEVEL_ACTIVE SOURCE_LOOP 환경 표현만 정적 source bound가 증명되면 화면 밖에서 시각적 시계를
멈춘다. 입자·RNG 상태는 보존하되 숨은 wall time은 따라잡지 않으며, 재진입 때 현재 dt만 전진한다.
따라서 화면 밖에서도 계속 재생한 경우와 ambient 위상은 달라진다. Source feature 미지원,
외부 owner/anchor/control, 이동한 root는 제외한다. 레벨명별 정책은 만들지 않는다.
기존 marker의 자동 중복 Update는 없다는 코드 확인과 실제로 남은 숨은 작업을 구분한다.

Light_Manager의 Render_Lights 제출 경계에서 최종 view/projection으로 한 번 만든 보수적 clip
plane과 point/spot 영향 sphere(position, range)를 검사한다. directional·camera 경계 교차는
유지하고 invalid 입력은 fail-open한다. transient 원본 목록/forward shader 입력이나 shadow
caster는 변경하지 않는다. profiler에 후보/제외 건수를 추가해 업로드·draw와 함께 비교한다.
실제 shader의 유한 range 의미, 경계/뒤쪽/대형 광원 포함 및 source 순서·batch 경계를 검증한다.
