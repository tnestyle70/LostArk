# Composition Profiler 전체 프레임 계측 구현 계획

## G00. 사용자 요청과 실측 경계

ALT V 캡처 분석 뒤 사용자가 실제 계측 확장을 요청했다. 범위는 입자뿐 아니라 map culling·렌더 제출,
animation update·화면에 그리지 않은 pose 계산, navigation, main Update/Render 잔여 구간과
GPU pass·SceneColor 복사다. F1에 Open Composition Profiler를 연결하고 기존 CProfiler 데이터를
간결한 ImGui 패널과 JSON에 표시한다. JobSystem/Fiber나 visual LOD를 도입하는 작업은 아니다.

기존 Engine CProfiler의 CPU scope/counter/GPU frame query를 확장한다. 별도 profiler runtime을
만들지 않고 Server 권위 작업을 Client CPU 비용처럼 표시하지 않는다. 현재 실제 소유자와 호출자를
조사해 필요한 계측만 추가하며 per-particle/per-bone scope로 수집 비용을 폭증시키지 않는다.

## G01. CPU 구간과 작업량

MainApp의 Input·UI·Network drain·Engine update·Render prepare·UI text·ImGui backend·Present를
분리한다. 기존 Particle.Simulate 두 구간은 Spawn과 Update로 구분한다. native mesh/sprite의 instance
작성·GPU upload·material bind·Draw 제출, animation channel/blend/bone/palette/history,
map 가시 목록/packing/upload/draw를 실제 의미 단위에 계측한다.

화면 밖 animation은 기존 visibility owner를 먼저 조사한다. 정확한 frustum owner가 없으면
업데이트한 model과 같은 frame에 그리기로 제출한 model을 비교한 `updated but not submitted`
측정으로 표시하고 실제 frustum-out 판정과 혼동하지 않는다. navigation은 Client 경로와 Server
실제 path 계산을 구분하며 측정한 범위만 결과에 적는다. 구간의 inclusive/self와 작업량 counter를
함께 표시하고 계측을 위해 simulation 결과·culling·입력·네비 경로를 바꾸지 않는다.

ServerNavigation의 기존 owner에는 FindPath(A*), ReachablePath(BFS), ProjectPoint,
SmoothPath, TraversalStep, LineOfSight의 호출·누적/최대 시간을 연결하고 기존 RoomPerf 로그가
소비한다. leaf grid에서 측정하고 region root는 합산해 중복 집계하지 않는다. CPU/GPU Client
프레임과 시계·프로세스가 다르므로 네트워크 상태값을 시간처럼 쓰거나 Client 합계에 더하지 않는다.

## G02. GPU 시간과 불완전 측정 표시

기존 disjoint/timestamp ring 안에 bounded pass별 begin/end query를 둔다. main의 immediate
context만 사용하고 4frame 지연·DONOTFLUSH 결과 조회를 유지한다. frame당128구간 상한 초과는
drop count로 남긴다. Pending/Valid/Unsupported/Disjoint/Dropped/Error를 구분하고 CPU 기록은
GPU 미지원·실패와 독립적으로 유지한다. 중지 뒤 이미 발행한 query 결과도 완료 가능한 경로를 확인한다.

Renderer의 Shadow·NonBlend·SSAO·Lights·SceneHDR·ScreenPosts·Bloom·Final·UI 등과 실제
SceneColor snapshot 함수의 모든 복사를 계측한다. 같은 이름의 여러 복사는 개별 sample과 frame
합계로 남긴다. GPU 구간은 inclusive elapsed time이며 GPU 사용률이나 순수 shader busy 비율이 아니다.
RenderSubmissions의 기존4 counter를 실제 enqueue 지점에 연결하고 SceneColor 횟수/bytes를 추가한다.

## G03. F1 패널과 JSON 저장

기존 F1 Profiler 열기 동작을 Open Composition Profiler로 연결하고 단일 CProfilerTool을 확장한다.
상단은 frame/CPU/GPU 시간, CPU 구간 표는 평균·self·최대·호출, GPU 표는 pass별 평균·최대·P95,
작업량 표는 기록자의 실제 counter를 표시한다. GPU pending/미지원과 실행 안 된 구간을0ms로
표시하지 않는다. 전체 현재 scope 이름은 사전등록 metadata와 실제 동적 발견을 함께 사용한다.

JSON은 v3로 버전 올리고 기존 frame/counter/CPU fields를 보존하면서 GPU sample/status/coverage와
측정 조건을 추가한다. Save는 main에서 측정된 Snapshot 복사 뒤 immutable snapshot을 단일 exporter에
이동해 직렬화·파일 쓰기를 수행한다. 동시 저장은 한 개만 허용하고 임시 파일→최종 rename,
실패 사유·이전 파일 보존·취소 및 bounded 종료를 유지한다. UI가 닫혀도 완료 상태를 잃지 않는다.

## G04. 파일 소유와 검증

Engine Profiler.h/.cpp와 Renderer.cpp는 query/DTO/aggregate, Client ProfilerCaptureIO.h/.cpp는
JSON/exporter, ProfilerTool.h/.cpp는 UI를 소유한다. 실제 CPU caller 파일에는 scope/counter만 추가한다.
새 permanent C++ 파일은 현재 필요하지 않으며 필요해지면 project/filter 등록도 함께 한다.
기존 파일별 인코딩·미커밋 변경을 보존하고 build/EngineSDK/Resources를 stage하지 않는다.

실제 Product Debug compile/link/deploy, CPU scope/aggregate와 headless GPU query frame귀속·상한·
중지/재개·reset·미지원 경계, JSON parse/status/async 완료·오류/기존 파일 보존을 focused 검사한다.
World marker의 actual Codec/Playback/resource admission도 함께 마무리한다. Client/UI 실행과
실제 게임 benchmark capture는 사용자가 직접 하며 계측 연결 성공을 FPS 개선이나 visual PASS로
기록하지 않는다.
