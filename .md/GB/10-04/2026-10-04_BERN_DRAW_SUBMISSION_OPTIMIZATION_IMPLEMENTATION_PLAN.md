# 베른 컷신·기본 이동의 제출 비용 최적화 구현 계획

현재 추가 정본은 **G14(2026-10-10)**다. 앞선 G00~G13은 기존 구현 이력이며, 최종 검증은 대응 RESULT G21에 기록한다.

## G00. 저장된 두 캡처와 변경 범위

2026-10-04 Debug 캡처 `베른성_컷신_20261004_050800_808_frame104_45964_0.json`과
`베른성_기본이동_20261004_051021_021_frame496_45964_1.json`을 기준으로 한다.
컷신104프레임 평균 interval118.676ms, 기본이동120프레임40.650ms다.
NONBLEND CPU는59.311/13.476ms, Map.Batch.Draw는33.059/7.319ms이고 호출은
2405.5/582.9회다. Material8.685/2.123ms, Pass7.171/1.657ms도 호출 수에 비례한다.
GPU timestamp는 CPU 제출 공백을 포함하므로 GPU 사용률로 해석하지 않는다.
컷신4프레임의 CPU scope 누락은 Self 집계에서 제외한다. 두 캡처 모두 Detailed OFF다.

동일 geometry·profile·전체 재질 입력에서 asset ID만 제거해도 표시 대상 배치 감소는11개다.
구운 조명·그림자 texture identity가 실제 분할 원인이다. 재질이나 조명을 생략해 배치를
합치지 않고 기존 CModel/CMaterial 경로에 최대8개의 원래 SRV를 묶어 전달한다.
Resources, 저작·게시 데이터, 화질 옵션은 변경 대상이 아니다.
현재 checkout의 기존 미커밋 렌더링·캐릭터·데이터 변경은 보존한다.

## G01. Renderer와 GameObject의 인접 NONBLEND 제출

`Engine/Public/GameObject.h`, `Engine/Private/GameObject.cpp`, `Engine/Private/Renderer.cpp`의
기존 NONBLEND 순회에 선택적인 인접 제출 계약을 연결한다. 기본 객체는 원래
`Render_Group(NONBLEND)`를 호출하고 한 객체만 소비한다. 인접 객체의 순서를 바꾸거나
현재 큐의 수명을 넘겨 소유하지 않는다. 실패·0개·범위 밖 소비는 기존 프레임 실패 경로를 따른다.

## G02. CModel·CMaterial의 조명 텍스처 묶음

Engine의 기존 Model/Material 선언·정의에 단일 정적 mesh의 현재 입력 동등성을 추가한다.
같은 geometry, 전체 비조명 입력과 유효한 SOURCE_BG 재질만 허용한다.
override·morph·다른 재질은 원래 제출로 남긴다. LOD가 생성된 모델은 Client admission에서
제외하며 Mesh의 LOD 선택 API와 구현은 변경하지 않는다.
CMaterial이 원래 SRV를 계속 소유하고, bank는 최대8개 모델의 average/directional/static-shadow
SRV를 기존 CShader에 바인딩한다. texture 크기·포맷·mip·sRGB를 변환하거나 복제하지 않는다.
준비 중 실패하면 draw를 제출하지 않는다.

## G03. MapStaticBatchObject의 기존 가시 인스턴스 합성

`Client/Public/MapStaticBatchObject.h`, `Client/Private/MapStaticBatchObject.cpp`만 Client 객체
수명을 확장한다. 기존 batch별 visibility/grace/LOD·shadow 준비와 placement ID·편집 상태는
유지한다. 현재 NONBLEND 큐의 인접 객체 중 같은 pass/profile/시간/geometry인 LOD 없는 단일 mesh만
현재 순서대로4~8개씩 합친다.2~3개는 고정 폭 bank의 제출 비용이 더 커 기존 Render를
사용한다. 기존 가시 CPU payload를 재사용 staging/instance buffer로
복사하며, 복사본의 미사용 directionalScale.w에 bank index를 넣는다. 원래 placement lighting과
원래 가시 payload는 보존한다. 하나뿐이면 기존 Render를 그대로 사용한다.

## G04. MapInstance shader의 원래 조명식 유지

`Shader_VtxMeshMapInstance.hlsl`에 SOURCE_BG bank 전용27~29패스를 추가한다.
기존24~26은 별도 shader로 컴파일하여 일반 draw에 추가24개 SRV 바인딩 비용을 전파하지 않는다.
기존 RNM와 signed-distance shadow 식은 공용 helper로 보존하고, 원래 sample 또는 선택한
bank sample을 동일 식에 넣는다. bank count0이면 원래 입력 경로다. 다른 mesh/effect shader의
입력 layout, VTXMESHINSTANCE192바이트, 기존 pass 번호와 state는 유지한다.
공용 `Shader_StaticShadowMap.hlsli`는 Engine/Bin/ShaderFiles가 정본이며 정상 Product의
PrepareEngineSdk가 Client 배포본을 갱신한다. 정본과 추적 배포본을 함께 유지한다.

## G05. 이동 실패 탐색의 별도 범위

기본이동의 세 프레임에서 동기 Navigation.AStar가34~40ms,16384노드 상한까지 실행되고
경로를 찾지 못한다. 출발과 목적지의 실제 navigation 연결성을 원래 Can_Step 조건으로
판정하여 증명 가능한 불연결만 조기에 반환한다. Server command·권위·목표 위치·기존
예측 경로 보존·성공 경로 선택을 변경하지 않는다. Engine PathFinder의 목표에서 incoming
Can_Step 간선을 최대128셀까지 역탐색한다. 출발점을 만나거나 제한을 넘으면 기존 A*를
새 generation으로 그대로 실행한다. 출발점을 만나지 않고 연결 성분을 전부 소진했을 때만
UNREACHABLE을 반환한다. 런타임 blocker와 높이·대각선 조건을 원래 Can_Step으로 소비한다.

## G06. 검증과 종료 조건

새 제품 C++ 파일을 만들지 않으며 기존 project/filter 등록을 유지한다. Engine public header는
정상 Product Build를 통해 SDK와 모든 Client 소비자를 함께 갱신한다. 파일별 인코딩과 줄바꿈을
유지하고 실행 중 Client/Server와 다른 빌드를 종료하지 않는다.

1. 실제 캡처·설치 자료로 인접 후보와 기대 draw 감소량을 계산한다.
2. 기존·묶음 입력의 순서/LOD/재질 차이 거부/실패 경계를 작은 native 검증으로 대조한다.
3. 원래 SRV sample과 bank sample의 화면 없는 GPU 수치 결과를 대조한다.
4. 변경 CPP/HLSL 컴파일, 필요한 정상 증분 Debug Product Build와 diff-check를 수행한다.
5. 제품 출력이 점유됐으면 후보 컴파일·검증을 먼저 마치고 최종 링크를 위한 사용자 종료만 요청한다.
6. 새 EXE의 같은 컷신·이동 캡처와 화면 판정은 사용자가 수행한다. 후보 수치 검증을 실제 FPS
   개선으로 기록하지 않으며 완료·미완료와 실행한 증거만 대응 RESULT에 남긴다.

## G07. 정적 배경 효과의 지연 누적 방지와 실측 항목

14:34 캡처의 CPU frame1~15는 평균1,233.399ms이며 Ambient.Advance가573.308ms다.
회복 중 새 delta가155~171ms인 frame72~76에도 효과당60회 fixed-step이 남는다.
카메라는 `Level_Bern.cpp`에서 이미 한 frame당0.1초까지만 진행하므로 빠른 카메라 이동이
최초 병목이라는 근거는 없다. 카메라 경로·속도·FOV와 rendering option은 이번에 변경하지 않는다.

`Client/Private/Effect_PresentationService.cpp`의 기존 deferred ambient 승인 경로만 입력 시간을
최대0.1초로 제한한다. 이 경로는 level 소유, offscreen pause 허용, owner-sustained source loop,
고정 sprite bounds 검증, 외부 sample/character/boss/attachment 없음 조건을 이미 만족한다.
화면 안에서 실제 Advance를 수행할 때만 제한하며 hidden frame, initial Seek, 실패 제거의 흐름은
유지한다. object와 service elapsed에 같은 committed delta를 전달하고 초과 시간은 보관하지 않는다.
root 편집으로 bounds가 무효가 되어도 기존에 승인된 독립 배경 효과의 시간 정책은 유지한다.

기존 Playback.Update의1/60 step, particle age/spawn/RNG/loop 적분은 변경하지 않는다.
긴 frame에서는 배경 시각 시계가 실제 시간보다 느려지는 계약이며 wall-clock phase 보존을
주장하지 않는다. 이미 offscreen 동안 pause하는 장식 효과에만 적용하고 combat, typed history,
authoring Seek, 카메라와 Server 시간을 바꾸지 않는다. 새 API나 두 번째 playback 경로를 만들지 않는다.

Engine/Public/Profiler.h의 enum 끝에 ambient 제한 횟수, 제외한 effect-microseconds,
실제 fixed-step 횟수와 frame 내 단일 effect 최대 step을 추가한다. Client의 ProfilerCaptureIO와
ProfilerTool 이름표를 함께 갱신하며 이전 counter 순서를 보존한다. Effect_Playback.h와
Effect_Object.h에 기존 simulation step 정수의 읽기 전용 getter를 추가해 전후 차이를 집계한다.
기존 fixed-step clock에는 accumulator가 포함되므로 실제 실행 횟수로 환산하지 않는다.
상세 CPU scope capacity 누락과 독립적으로 JSON에 남긴다. 제외 시간은
효과별 합계이므로 실제 frame wall time이나 절약한 CPU 시간으로 해석하지 않는다.

실제 Service 함수와 Playback.Update 본문을 소비하는 headless native fixture로 정상 delta 동등성,
연속1~2초 지연, hidden/resume, initial Seek, owner 실패 제거,33.9만회 이상 누적 및 실제339개
캡처 delta 재생을 검증한다. Step 관찰 fixture는 호출 예산과 입력 경계의 증거이며 GPU 표시나
수정 후 FPS를 증명하지 않는다. 기존 particle 연산 함수가 무변경인지 diff로 확인한다.
새 제품 파일을 만들지 않으며 project/filter 등록은 유지한다. 정상 Debug Product 증분 빌드,
JSON counter export의 구조 확인과 diff-check를 마친 뒤 사용자 재캡처로 성능과 화면을 확인한다.

## G08. F1에서 같은 베른 입장 컷신 반복 재생

사용자의 두 번째 캡처에서도 저FPS가 남았으며 F6 수동 이동과 정확히 같은 카메라 경로·FOV로
비교하려면 종료 후 동일 컷신을 다시 재생할 수 있어야 한다. MainApp_Dragon.cpp의 F1
`Camera`에서 이동 속도 조절 바로 아래에 Debug Bern 전용 `Start Bern Cutscene`을 둔다.
버튼은 Level-owned `Request_DebugEntranceReplay`에 요청만 전달하고 상태를 읽어 표시한다.

Level_Bern.h/cpp가 Debug 요청·진행 상태를 소유하며 다음 Level Update에서 기존
Update_EntranceCinematic이 처리할 재생 상태를 준비한다. 기존 cue가 있으면 그대로 재사용하고,
session 최초 재생 latch 때문에 로딩하지 않은 재입장에서는 같은 정본 JSON을 local candidate로
검증한 뒤 commit한다. 실패는 기존 cue·카메라·자동 재생 latch를 보존하고 상태를 알린다.
자동 재생의 process-session latch를 false로 돌리거나 두 번째 컷신 runtime을 만들지 않는다.

동일한16key·duration·FOV·샘플러·시간 제한을 사용한다. 재생 중/중복 pending, 타 presentation
owner, level transfer·캐릭터 복원·맵 편집·플레이어 피킹과 충돌하는 요청은 거절한다.
처리 직전에도 상태를 재검증한다. ESC key edge를 현재 입력에 맞춰 시작해 기존의 held ESC가
새 재생을 즉시 취소하지 않게 한다. 종료·ESC·실패는 기존 End_EntranceCinematic의 camera
owner 해제와 시작 전 follow 요청·target·FOV·free pose 복원을 그대로 사용한다.

수정 제품 파일은 Client/Private/MainApp_Dragon.cpp, Client/Private/Level_Bern.cpp,
Client/Public/Level_Bern.h이며 새 제품 파일·project/filter 등록·JSON publish는 없다.
실제 요청/소비/재생 본문과 기존 sampler를 사용하는 headless 검사로 반복/거절/실패 시 보존,
자동 latch 유지와 follow/free 복원을 확인한다. Debug Product Build와 Release 조건부 컴파일
경계·diff-check를 확인하고 실제 버튼·화면·재캡처는 사용자가 수행한다. 이 추가 자체를
컷신 성능 해결 완료로 기록하지 않는다.

## G09. 컷신 카메라의 LOD view 준비와 적용 누락 수정

사용자가 기존 CPU LOD 선택을 유지하면서 거리·화면 크기별 적용 범위를 넓히고 draw 병합을
함께 개선하도록 승인했다. 실제 Bern cue10,568pose 재현에서 기존 LOD affine 검사3,012개가
inverse view의 `_44 != 1` 때문에 거절된다. 최대 차이는1.5 float epsilon이며 projection
거절은0이다. 원본 캡처에 frame별 행렬은 없으므로 실제 기록의 모든 누락 원인으로 확대하지 않는다.

Client의 기존 MAP_CAMERA_CULL_SNAPSHOT에 LOD 전용 canonical view와 보수 linear scale을
추가한다. MapAssetRenderUtils.cpp의 Build_CameraCullSnapshot이 camera revision당 한 번
준비하며 원본 view/projection·frustum plane·GPU 바인딩은 유지한다. projective 항은0이어야
하고 유한한 `_44`가1에서32 float epsilon 이내일 때만 전체 행렬을 같은 `_44`로 나눈다.
실패는 LOD 파생값만 invalid로 두고 기존 렌더·가시성은 유지한다. 셀별 정규화나 다른 카메라
runtime을 추가하지 않는다.

MapStaticBatchObject.cpp의 VIEW_LOD_ENVELOPE와 Build_ScreenLodView는 같은 파생 view/scale을
소비한다. center만 나누고 radius·error scale을 그대로 두는 혼합을 금지한다. LOD 없는 batch의
per-instance tight envelope 생략,0.25px 기준·0.9 여유·near/invalid fallback을 유지한다.
camera scale의 제곱합 계산도 각 batch/draw가 아닌 snapshot 준비에서 한 번 수행한다.

기존 sampler와 실제 camera inverse를 사용하는 native 재현으로 허용/거절 경계, 원본 homogeneous
좌표를 포함하는 보수 bounds, source view/plane 불변과 invalid 보존을 확인한다. 수정 파일은
Client/Public/MapAssetRenderUtils.h와 Client/Private/MapAssetRenderUtils.cpp,
Client/Private/MapStaticBatchObject.cpp이며 기존 프로젝트 등록을 사용한다.

## G10. 원래 화면 오차를 유지한 정적 메시 LOD와 병합 호환

설치 Bern opaque submesh68개에서 생성 하한6,144 후보를 검증했다. 새 대상51개 중3개만
감소하며 추가 생성 CPU 중앙값 합계는 Release241.60ms, Debug681.17ms였다. 이는 병렬
작업 중 개별 측정 합계이며 실제 입장 지연은 아니다. 작은 메시 전체의 생성 하한은24,576으로
유지한다. G09에서 실제 거절되던 카메라를 정상 선택에 포함하는 것으로 적용 범위를 넓힌다.
최대크기·원본 정점·19개 shading attribute·UV 경계·lock border·기존 생성 error와0.25px
선택 기준을 유지하며 매 frame 생성·GPU dispatch·readback을 추가하지 않는다.

새로 LOD가 생겼다는 이유로 기존 조명 bank 병합을 모두 잃지 않도록 CModel에 읽기 전용
Get_StaticMeshLodLevel(meshIndex, view)을 추가한다. CMesh의 같은 private 선택 함수를
실제 Render_Instanced와 이 조회가 공유한다. null/invalid view·morph·생성 range 없음은
기본 index와 level0이며 조회는 counters·GPU·model 상태를 변경하지 않는다.

Client는 인접 bank 후보 각각의 기존 camera/material admission으로 선택한 level이 같을
때만 묶는다. 이미 같은 CMesh identity를 요구하므로 level이 같으면 index range도 같다.
다른 level은 기존 순서의 병합 경계이며 원래 개별 draw로 남긴다. shadow는 기존 source LOD0,
masked/변형/다른 family의 geometry admission은 그대로다. 화면 오차 허용값을 높이지 않는다.

Engine/Private/Mesh.cpp·Model.cpp와 Engine/Public/Mesh.h·Model.h,
Client/Private/MapStaticBatchObject.cpp를 수정한다. 같은 선택기 결과·무효 입력·LOD가 서로
다른 인접 객체·변형/재질 변경의 fallback과 현재 작은 bank 이득 보존을 검증한다.

## G11. 두세 개 인접 배치용 작은 조명 bank

설치 Bern의 인접 SOURCE_BG 단일 mesh 후보를 대조하면 동일한 조명 SRV까지 공유하는 추가
병합 대상은 없지만, 다른 조명 SRV를 가진2~3개 prefix가815개다. 기존 bank8의24개 SRV
바인딩 비용 때문에 작은 묶음을 제외했다. 제품 수정 전 out 후보의 RTX4070 수치 검증에서
3slot/9SRV bank는256개 MRT 비교가 bitwise 동일하고, instance staging·Map/Unmap을 포함한
CPU 제출 중앙값도2개36.183→27.555us,3개46.247→23.116us로 감소했다. 이 fixture를 게임
FPS 개선율로 환산하지 않는다.

Shader_VtxMeshMapInstance.hlsl의 기존 ordinary24~26과 bank8 27~29는 그대로 두고,
3slot 전용30~32를 추가한다. 원래 SampleGrad·RNM·shadow 식을 공유하고 별도 pixel shader로
컴파일해 ordinary·bank8에 새 SRV 비용을 전파하지 않는다. instance와 wind carrier는 유지한다.

CMaterial::Bind_StaticLightingBank는2~3개일 때만 각3개 SRV 배열을 바인딩하고4~8개는 기존
배열을 사용한다. Client의 Render_AdjacentNonBlend는 같은 원본 순서의 최대8개 prefix에서
2~3개면 small pass,4~8개면 기존 pass를 선택한다. G10의 같은 geometry·선택 LOD 검사를
함께 적용하고, 재질·변형·카메라·진단 충돌과 실패 전/후 소비·reset/fallback은 유지한다.

제품 수정은 Client/Bin/ShaderFiles/Shader_VtxMeshMapInstance.hlsl,
Engine/Private/Material.cpp와 Client/Private/MapStaticBatchObject.cpp의 해당 함수다.
dirty Shader.cpp·Renderer.cpp의 다른 작업을 포함시키지 않으며 새 프로젝트 파일은 없다.
실제 API·bank별 reflection·고정 MRT 정합·source 순서/서로 다른 LOD·최소 묶음·실패 경로와
정상 Product의 실제 CSO를 확인한다. LOD 없는 객체의 불필요 bounds 계산은 되살리지 않는다.

## G12. 빌드별 텍스처 품질 기본값

사용자가 Debug 기본값은 하, Release 기본값은 최상으로 요청했다. 현재 UI의 최상/상/중/하는
0/1/2/3이며 기존 texture sampler 연결이 같은 최소 mip으로 소비한다. UserSettingsDocument의
단일 기본값 함수가 Debug3, Release0을 반환하고 Apply_Video의 누락 fallback과
SystemOptionWindowView::Effective_Default가 함께 사용한다. UI 생성 전 로드와 이후 seed,
Reset이 서로 다른 기본값을 적용하지 않게 한다. 명시적으로 저장된 사용자 값은 보존하며
잘못된 값의 기존 안전 fallback0도 유지한다. 기존 개인 저장0은 이 변경만으로3이 되지 않는다.

제품 변경은 Client/Public/UserSettingsDocument.h, Client/Private/UserSettingsDocument.cpp,
Client/Private/SystemOptionWindowView.cpp다. 현재 작업 트리에 완료된 mip sampler 연결은
필수 소비자 의존성으로 함께 검토하되 별도 HorizonAO/SSR Workbench 수정은 포함하지 않는다.
기존 UserSettingsContractHarness를 Debug/Release 양쪽 기본값으로 검증하고 정상 Product를
두 구성에서 빌드한다. 텍스처에 없는 mip을 sampler 옵션이 생성하지 않으며 실제 FPS는
사용자의 동일 컷신 재캡처로 확인한다. 새 제품 파일·JSON publish·렌더링 scene 설정 교체는 없다.

## G13. 후속 캡처에서 실효 texture mip 품질 확인

ProfilerTool::Build_CaptureContext의 기존 renderingOptions에 `Texture.minimumMip`을 추가한다.
기본값이나 UI row가 아니라 renderer가 현재 소비하는 iTextureMinMip을 기록한다. 저장 당시의
context이며 과거 모든 frame의 실제 texture sample 또는 mip residency를 의미하지 않는다.
기존 generic JSON writer/reader·비교 경로를 사용하며 schema나 frame/counter 저장 범위를
바꾸지 않는다. ProfilerTool.cpp 한 줄만 추가하고 인코딩/CRLF를 유지한다. Debug Product
증분 컴파일과 기존 renderingOptions의 export/parse 연결을 확인한다.

## G14. 2026-10-10 거리·LOD 적용 범위와 가림 검사 복구

사용자가 원인 조사 뒤 실제 수정과 Debug/Release 검증을 요청했다. 아래 G14가 이번
변경의 정본이며 앞선 날짜의 수치·구현 설명은 당시 이력이다. 현재 설치 Bern의 1,755개
geometry/submesh 중 생성된 축소 LOD는 6개다. 거리 컬링은 작은 소품과 화면 24px의
AND 조건 때문에 원거리 소품이 많이 남는다. 카메라·scene 렌더링 저작 JSON은 바꾸지 않는다.

Engine의 가림 검사는 CCamera inverse의 동차 성분 오차를 현재 LOD view처럼 정규화한다.
view의 projective 성분·비유한 값·32 float epsilon 초과 오차는 계속 거부한다. object world,
보수적 depth/coverage bias, 실패 시 표시 유지와 bounded raster budget은 유지한다.
실제 Release DLL과 16개 현재 카메라 키의 역행렬로 수정 전 탈락을 재현했다. 이 검증은
Client/UI 실행이나 실제 보간 재생의 FPS 측정이 아니다.

기존 C++ 파일만 변경하므로 프로젝트·filters 등록은 추가하지 않는다. 후보 전문을 아래에
기록한 뒤 해당 파일을 반영한다. 최종 Product Debug/Release 빌드 뒤 설치 DLL로 같은
입력과 잘못된 행렬, 앞/뒤/화면 밖 AABB를 검증하고 RESULT에 수정 전후를 구분한다.

### G14-1. Engine/Private/OcclusionCuller.cpp 전체 반영 코드

```cpp
#include "OcclusionCuller.h"

#pragma push_macro("new")
#pragma push_macro("min")
#pragma push_macro("max")
#undef new
#undef min
#undef max
#define PRECISE_COVERAGE 1
#define USE_D3D 1
#define USE_AVX512 0
#include "../ThirdPartyLib/MaskedOcclusionCulling/MaskedOcclusionCulling.h"
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
#pragma pop_macro("max")
#pragma pop_macro("min")
#pragma pop_macro("new")

using namespace Engine;

namespace
{
    bool OcclusionFinite(const float* values, size_t count)
    {
        for (size_t i = 0u; i < count; ++i) if (!std::isfinite(values[i])) return false;
        return true;
    }

    bool OcclusionAffine(const float4x4_t& matrix)
    {
        return OcclusionFinite(&matrix._11, 16u) && matrix._14 == 0.f &&
            matrix._24 == 0.f && matrix._34 == 0.f && matrix._44 == 1.f;
    }

    // CCamera의 역행렬은 동차 성분이 1에서 몇 ULP 벗어날 수 있다.
    // Renderer가 전달한 view만 정규화해 동일한 원근 나눗셈을 유지한다.
    // projective/비정상 입력은 거부하고, Rasterize의 object world 검증은 유지한다.
    bool OcclusionCameraAffine(const float4x4_t& source, float4x4_t& canonical)
    {
        if (!OcclusionFinite(&source._11, 16u) || source._14 != 0.f ||
            source._24 != 0.f || source._34 != 0.f ||
            std::abs(double(source._44) - 1.) > 32. * std::numeric_limits<float>::epsilon())
            return false;
        for (size_t row = 0u; row < 4u; ++row)
            for (size_t column = 0u; column < 4u; ++column)
                canonical.m[row][column] = static_cast<float>(double(source.m[row][column]) / source._44);
        return OcclusionAffine(canonical);
    }

    // MOC compares 1/w. Admit only D3D perspective projections where z/w is
    // monotone in w and the near plane is a positive constant w.
    bool OcclusionPerspective(const float4x4_t& projection, float& nearW)
    {
        if (!OcclusionFinite(&projection._11, 16u) || projection._11 <= 0.f || projection._22 <= 0.f ||
            projection._12 != 0.f || projection._13 != 0.f || projection._14 != 0.f ||
            projection._21 != 0.f || projection._23 != 0.f || projection._24 != 0.f ||
            projection._41 != 0.f || projection._42 != 0.f || projection._44 != 0.f ||
            std::abs(projection._34) != 1.f) return false;
        const double slope = double(projection._33) / projection._34;
        const double nearValue = -double(projection._43) / slope;
        if (slope < 1.0 || !std::isfinite(nearValue) || nearValue <= 0.0 || nearValue > 1.e6) return false;
        nearW = static_cast<float>(nearValue);
        return std::isfinite(nearW) && nearW > 0.f;
    }

    void* OcclusionAllocate(size_t alignment, size_t bytes)
    {
        void* result = _aligned_malloc(bytes ? bytes : 1u, alignment);
        if (!result) throw std::bad_alloc();
        return result;
    }
    void OcclusionFree(void* memory) { _aligned_free(memory); }

    struct OCCLUSION_POINT final { double x = 0.0, y = 0.0; };

}

struct COcclusionCuller::STATE final
{
    std::unique_ptr<MaskedOcclusionCulling, void(*)(MaskedOcclusionCulling*)> rasterizer{nullptr, MaskedOcclusionCulling::Destroy};
    float4x4_t viewProjection{};
    vector<float4_t> clipPositions, triangles;
    vector<uint32_t> sequentialIndices;
    uint32_t width = 0u, height = 0u, inputTriangles = 0u, rasterizedTriangles = 0u;
    float nearW = 0.f, viewportScaleX = 1.f, viewportScaleY = 1.f;
    bool ready = false;
};

COcclusionCuller::COcclusionCuller(bool_t forceSse2) : m_ForceSse2(forceSse2) {}
COcclusionCuller::~COcclusionCuller() = default;

bool_t COcclusionCuller::Begin(const float4x4_t& view, const float4x4_t& projection,
    uint32_t viewportWidth, uint32_t viewportHeight)
{
    if (m_State) { m_State->ready = false; m_State->inputTriangles = m_State->rasterizedTriangles = 0u; }
    float nearW = 0.f;
    float4x4_t canonicalView{};
    if (viewportWidth < 8u || viewportHeight < 4u || viewportWidth > 4096u || viewportHeight > 4096u ||
        !OcclusionCameraAffine(view, canonicalView) || !OcclusionPerspective(projection, nearW)) return false;
    try
    {
        if (!m_State) m_State = std::make_unique<STATE>();
        auto& state = *m_State;
        if (!state.rasterizer)
            state.rasterizer.reset(MaskedOcclusionCulling::Create(m_ForceSse2 ? MaskedOcclusionCulling::SSE2 :
                MaskedOcclusionCulling::AVX2, OcclusionAllocate, OcclusionFree));
        if (!state.rasterizer) return false;
        // Padded storage keeps exactly the viewport's pixel centers. A lower
        // resolution can hide subpixel openings; independently shrinking every
        // triangle instead creates artificial gaps along shared internal edges.
        const uint32_t width = (viewportWidth + 31u) / 32u * 32u;
        const uint32_t height = (viewportHeight + 7u) / 8u * 8u;
        state.viewportScaleX = static_cast<float>(viewportWidth) / width;
        state.viewportScaleY = static_cast<float>(viewportHeight) / height;
        if (state.width != width || state.height != height)
        {
            state.rasterizer->SetResolution(width, height);
            state.width = width; state.height = height;
        }
        XMStoreFloat4x4(&state.viewProjection, XMLoadFloat4x4(&canonicalView) * XMLoadFloat4x4(&projection));
        if (!OcclusionFinite(&state.viewProjection._11, 16u)) return false;
        state.nearW = nearW;
        state.rasterizer->SetNearClipPlane(nearW);
        state.rasterizer->ClearBuffer();
        state.ready = true;
        return true;
    }
    catch (const std::bad_alloc&)
    {
        // Upstream SetResolution releases the old allocation before allocating
        // the new one. Do not reuse that partially resized rasterizer on retry.
        if (m_State) { m_State->rasterizer.reset(); m_State->width = m_State->height = 0u; }
        return false;
    }
    catch (const std::length_error&) { return false; }
}

bool_t COcclusionCuller::Rasterize(const float* positions, uint32_t vertexCount,
    uint32_t strideBytes, std::span<const uint32_t> indices,
    const float4x4_t& world, CULL_MODE cullMode)
{
    if (!m_State || !m_State->ready || !positions || !vertexCount || vertexCount > MAXIMUM_INPUT_TRIANGLES * 3u ||
        strideBytes < 12u || strideBytes > 1024u || strideBytes % alignof(float) || indices.empty() || indices.size() % 3u ||
        indices.size() / 3u > MAXIMUM_INPUT_TRIANGLES - m_State->inputTriangles || !OcclusionAffine(world) ||
        (cullMode != CULL_MODE::NONE && cullMode != CULL_MODE::BACK && cullMode != CULL_MODE::FRONT)) return false;
    auto& state = *m_State;
    state.inputTriangles += static_cast<uint32_t>(indices.size() / 3u);
    try
    {
        state.clipPositions.resize(vertexCount);
        state.triangles.clear(); state.triangles.reserve(indices.size());
        const matrix_t matrix = XMLoadFloat4x4(&world) * XMLoadFloat4x4(&state.viewProjection);
        for (size_t i = 0u; i < vertexCount; ++i)
        {
            const auto* vertex = reinterpret_cast<const float3_t*>(reinterpret_cast<const uint8_t*>(positions) + i * strideBytes);
            if (!OcclusionFinite(&vertex->x, 3u)) return false;
            XMStoreFloat4(&state.clipPositions[i], XMVector4Transform(XMVectorSet(vertex->x, vertex->y, vertex->z, 1.f), matrix));
        }
        for (size_t triangle = 0u; triangle < indices.size(); triangle += 3u)
        {
            OCCLUSION_POINT points[3]; float farW = 0.f; bool usable = true;
            for (size_t corner = 0u; corner < 3u; ++corner)
            {
                const auto index = indices[triangle + corner]; if (index >= vertexCount) return false;
                const auto& vertex = state.clipPositions[index];
                if (!OcclusionFinite(&vertex.x, 4u) || vertex.w <= state.nearW + 1.e-4f ||
                    vertex.z <= 0.f || vertex.z >= vertex.w) { usable = false; break; }
                points[corner] = {state.viewportScaleX * (double(vertex.x) / vertex.w + 1.0) - 1.0,
                    state.viewportScaleY * (double(vertex.y) / vertex.w - 1.0) + 1.0};
                if (std::abs(points[corner].x) > 1.e5 || std::abs(points[corner].y) > 1.e5) { usable = false; break; }
                farW = (std::max)(farW, vertex.w);
            }
            if (!usable) continue;
            const double area = (points[1].x - points[0].x) * (points[2].y - points[0].y) -
                (points[1].y - points[0].y) * (points[2].x - points[0].x);
            // The product rasterizer uses FrontCounterClockwise = FALSE.
            // Its front-facing triangles have negative signed area in NDC.
            if (std::abs(area) < 1.e-16 || (cullMode == CULL_MODE::BACK && area >= 0.0) ||
                (cullMode == CULL_MODE::FRONT && area <= 0.0)) continue;
            if (area < 0.0) std::swap(points[1], points[2]);
            farW += (std::max)(.001f, farW * .0001f);
            for (const auto& point : points)
                state.triangles.push_back({static_cast<float>(point.x * farW), static_cast<float>(point.y * farW), 0.f, farW});
        }
        if (state.triangles.empty()) return true;
        if (state.sequentialIndices.size() < state.triangles.size())
        {
            const auto start = state.sequentialIndices.size(); state.sequentialIndices.resize(state.triangles.size());
            std::iota(state.sequentialIndices.begin() + start, state.sequentialIndices.end(), static_cast<uint32_t>(start));
        }
        state.rasterizer->RenderTriangles(&state.triangles.front().x, state.sequentialIndices.data(),
            static_cast<int>(state.triangles.size() / 3u), nullptr, MaskedOcclusionCulling::BACKFACE_NONE,
            MaskedOcclusionCulling::CLIP_PLANE_ALL);
        state.rasterizedTriangles += static_cast<uint32_t>(state.triangles.size() / 3u);
        return true;
    }
    catch (const std::bad_alloc&) { state.ready = false; return false; }
    catch (const std::length_error&) { state.ready = false; return false; }
}

bool_t COcclusionCuller::TestBounds(const float3_t& worldMin, const float3_t& worldMax) const
{
    if (!m_State || !m_State->ready || !m_State->rasterizedTriangles ||
        !OcclusionFinite(&worldMin.x, 3u) || !OcclusionFinite(&worldMax.x, 3u) ||
        worldMin.x > worldMax.x || worldMin.y > worldMax.y || worldMin.z > worldMax.z) return false;
    const auto& state = *m_State;
    const matrix_t matrix = XMLoadFloat4x4(&state.viewProjection);
    const double limit = (std::numeric_limits<double>::max)();
    double xMin = limit, yMin = limit, xMax = -limit, yMax = -limit, nearW = limit;
    for (uint32_t corner = 0u; corner < 8u; ++corner)
    {
        float4_t vertex;
        XMStoreFloat4(&vertex, XMVector4Transform(XMVectorSet(corner & 1u ? worldMax.x : worldMin.x,
            corner & 2u ? worldMax.y : worldMin.y, corner & 4u ? worldMax.z : worldMin.z, 1.f), matrix));
        if (!OcclusionFinite(&vertex.x, 4u) || vertex.w <= state.nearW + 1.e-4f ||
            vertex.z <= 0.f || vertex.z >= vertex.w) return false;
        const double x = state.viewportScaleX * (double(vertex.x) / vertex.w + 1.0) - 1.0;
        const double y = state.viewportScaleY * (double(vertex.y) / vertex.w - 1.0) + 1.0;
        xMin = (std::min)(xMin, x); xMax = (std::max)(xMax, x);
        yMin = (std::min)(yMin, y); yMax = (std::max)(yMax, y); nearW = (std::min)(nearW, double(vertex.w));
    }
    // The closest enclosing box point is even closer than any model triangle.
    // W bias and two pixel expansion cover transform/coverage rounding.
    nearW -= (std::max)(.001, nearW * .0002);
    if (nearW <= state.nearW || xMin >= 1.0 || xMax <= -1.0 || yMin >= 1.0 || yMax <= -1.0) return false;
    xMin = (std::max)(-1.0, xMin - 4.0 / state.width); xMax = (std::min)(1.0, xMax + 4.0 / state.width);
    yMin = (std::max)(-1.0, yMin - 4.0 / state.height); yMax = (std::min)(1.0, yMax + 4.0 / state.height);
    if (xMin >= xMax || yMin >= yMax) return false;
    return state.rasterizer->TestRect(static_cast<float>(xMin), static_cast<float>(yMin),
        static_cast<float>(xMax), static_cast<float>(yMax), static_cast<float>(nearW)) == MaskedOcclusionCulling::OCCLUDED;
}

uint32_t COcclusionCuller::GetInputTriangles() const { return m_State ? m_State->inputTriangles : 0u; }
uint32_t COcclusionCuller::GetRasterizedTriangles() const { return m_State ? m_State->rasterizedTriangles : 0u; }
uint32_t COcclusionCuller::GetWidth() const { return m_State ? m_State->width : 0u; }
uint32_t COcclusionCuller::GetHeight() const { return m_State ? m_State->height : 0u; }
uint32_t COcclusionCuller::GetImplementation() const
{ return m_State && m_State->rasterizer ? static_cast<uint32_t>(m_State->rasterizer->GetImplementation()) : UINT32_MAX; }
```

### G14-2. 원경 소품 거리 정책과 실효 설정

세션 기본 최대 화면 지름을24→48px로 조정한다. 현재 설치 모델·첫 키의 동일 입력에서
거리와 화면 크기를 모두 통과하는 작은 소품이1,565→8,230개가 되는 후보이며 실제 FPS와
외형 확인은 구분한다. 반경3m 초과 건물·지형은 계속 보존하고 저작 JSON은 변경하지 않는다.
DistanceScale은 거리 한계와 역비례 화면 지름을 함께 움직인다. 실효 지름은
clamp(DistanceMaxPixels / DistanceScale,4,128)이며 invalid 입력은 숨김에 사용하지 않는다.
기존 진입10%·재표시 hysteresis와 최종 카메라 기준 투영을 유지한다. F7의 표시와 캡처
metadata는 같은 계산을 사용하고 raw/실효 옵션을 함께 기록한다.
아래 전문이 후보이며 실제 적용은 최신 파일의 해당 필드·호출·UI 블록만 병합한다.

#### Engine/Public/Engine_RenderTypes.h 전체 반영 코드

```cpp
#pragma once

#include "Engine_Typedef.h"
#include "Engine_RenderFwd.h"
#include <d3d11.h>
#include "Engine_Enum.h"
#include <wrl/client.h>
#include <cstddef>
#include <array>
#include <string>
#include <memory>
#include <vector>

namespace Engine
{
    // Transient benchmark controls. They select equivalent runtime work paths,
    // never change authored quality, immutable asset preparation or gameplay.
    struct RENDER_OPTIMIZATION_SETTINGS final
    {
        bool_t FrustumEnabled = true, MeshLodEnabled = true, MapInstancingEnabled = true;
        bool_t IdenticalBatchEnabled = true, LightingBankEnabled = true, StaticShadowCacheEnabled = true;
        bool_t NpcPoseReuseEnabled = true, ParticleRootCacheEnabled = true, ParticleWorkersEnabled = true;
        uint64_t Revision = 1u;
    };
    // Session-only structural visibility controls; never overwrite authored render quality.
    struct MAP_VISIBILITY_SETTINGS final
    {
        bool_t OcclusionEnabled = true, DistanceEnabled = true, ParallelPreparationEnabled = false;
        // 베른의 작은 소품만 사용하는 세션 정책이다. 반경 3m 초과 건물과 저작 품질 값은 이 설정으로 바꾸지 않는다.
        f32_t DistanceScale = 1.f, DistanceMaxPixels = 48.f;
        uint64_t Revision = 1u;

        // 거리 배율을 줄일 때 화면 크기 문턱도 같은 방향으로 옮겨 두 조건 중 하나만 고정되지 않게 한다.
        // Profiler 표시와 실제 거리 컬링이 같은 문턱을 읽으며, 잘못된 입력은 0으로 돌려 숨김을 막는다.
        f32_t Get_DistancePixelLimit() const
        {
            if (!(DistanceScale >= .25f && DistanceScale <= 4.f) ||
                !(DistanceMaxPixels >= 4.f && DistanceMaxPixels <= 128.f)) return 0.f;
            const f32_t scaled = DistanceMaxPixels / DistanceScale;
            return scaled < 4.f ? 4.f : scaled > 128.f ? 128.f : scaled;
        }
    };
	// The renderer owns the active scene cube; a staged value retains its SRV
	// without changing current lighting until the profile transaction commits.
	struct RENDER_ENVIRONMENT_STATE final
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pCube;
		wstring_t strCubePath;
		float4_t vColor = { 1.f, 1.f, 1.f, 0.f };
		float4_t vRotationIntensity = { 0.f, 1.f, 1.f, 0.f };
        // Optional RGBM cube projection: Lambert E/pi in cube coordinates.
        // This project approximation is separate from native incident SH packing.
        std::array<float4_t, 7> vDiffuseSH{};
        f32_t fDiffuseIntensity = 0.f;
        bool_t bUseSourcePBRIndirect = false;
	};

	typedef struct tagLightDesc
	{
		LIGHT		eType;
		DirectX::XMFLOAT4	vDirection;
		DirectX::XMFLOAT4	vPosition;
		float		fRange;
		float		fFalloffExponent = 1.f;
		DirectX::XMFLOAT4	vDiffuse, vAmbient, vSpecular;
		float		fSpotInnerCos = 1.f;
		float		fSpotOuterCos = 1.f;
		LIGHT_RECEIVER eReceiver = LIGHT_RECEIVER::ALL;
        uint32_t staticShadowChannel = 0u;
        // Optional scene indirect input for native characters. Independent of direct RGB.
        float4_t vSourceCharacterAmbient{ 0.f, 0.f, 0.f, 0.f };
	}LIGHT_DESC;
	static_assert(sizeof(LIGHT_DESC) == 124u);
	static_assert(offsetof(LIGHT_DESC, vSourceCharacterAmbient) == 108u);
	static_assert(offsetof(LIGHT_DESC, fRange) == 36u);
	static_assert(offsetof(LIGHT_DESC, fFalloffExponent) == 40u);
	static_assert(offsetof(LIGHT_DESC, vDiffuse) == 44u);
	static_assert(offsetof(LIGHT_DESC, fSpotInnerCos) == 92u);
	static_assert(offsetof(LIGHT_DESC, fSpotOuterCos) == 96u);

	typedef struct tagShadowSettings
	{
		bool_t	bEnabled = false;
		f32_t	fOrthographicWidth = 40.f;
		f32_t	fOrthographicHeight = 40.f;
		f32_t	fNear = 0.1f;
		f32_t	fFar = 150.f;
		f32_t	fDepthBias = 0.0015f;
		f32_t	fNormalBias = 0.02f;
		f32_t	fStrength = 0.7f;
        // Optional modulation of PBR baked irradiance by dynamic casters only.
        f32_t fDynamicBakedStrength = 0.f;
        // Session quality experiment: radius 0/1/2 samples a 1/9/25-tap square.
        uint32_t iPCFFilterRadius = 1u;
	}SHADOW_SETTINGS;

	typedef struct tagShadowLightDesc
	{
		float4_t			vEye = float4_t(0.f, 20.f, -20.f, 1.f);
		float4_t			vAt = float4_t(0.f, 0.f, 0.f, 1.f);
		SHADOW_SETTINGS		Settings = {};
	}SHADOW_LIGHT_DESC;


	// Source LUT pixels are immutable, linear UNORM samples in a 16-cube atlas.
	struct SOURCE_COLOR_GRADING_LUT final
	{
		std::string strAssetId;
		std::array<float4_t, 256u * 16u> Pixels{};
	};
	struct SOURCE_COLOR_GRADING_LAYER final
	{
		std::shared_ptr<const SOURCE_COLOR_GRADING_LUT> pLut;
		f32_t fWeight = 1.f; // A null LUT denotes the neutral cube.
	};
	struct SOURCE_POST_PROCESS_SETTINGS final
	{
		bool_t bEnabled = false;
		f32_t fToneScale = 1.f, fToneRange = 8.f, fToneToe = 1.f;
		float3_t vHighlights{ 1.f, 1.f, 1.f }, vMidtones{ 1.f, 1.f, 1.f };
		float3_t vShadows{}, vColorize{ 1.f, 1.f, 1.f };
		f32_t fDesaturation = 0.f;
		// Authored profiles contain zero (neutral) or one layer. Runtime volume
		// transitions blend the original LUTs before applying the grading math.
		std::vector<SOURCE_COLOR_GRADING_LAYER> LutLayers;
	};

	typedef struct tagRenderQualitySettings
	{
		bool_t	bSSAOEnabled = true;
		f32_t	fSSAORadius = 0.75f;
		f32_t	fSSAOBias = 0.025f;
		f32_t	fSSAOIntensity = 1.f;
		f32_t	fSSAOPower = 1.25f;
		f32_t	fSSAODistanceFade = 60.f;
		bool_t	bBloomEnabled = true;
		f32_t	fBloomThreshold = 1.f;
		f32_t	fBloomSoftKnee = 0.5f;
		f32_t	fBloomIntensity = 0.8f;
		f32_t	fBloomScatter = 1.f;
		f32_t	fExposure = 2.f;
		f32_t	fWhitePoint = 11.2f;
		f32_t	fGamma = 2.2f;
		bool_t	bFXAAEnabled = false;
		f32_t	fFXAASubpixel = 0.75f;
		f32_t	fFXAAEdgeThreshold = 0.166f;
		f32_t	fFXAAEdgeThresholdMin = 0.0833f;
		// Optional scene color adjustment; identity preserves existing profiles.
		float4_t vBloomTint = { 1.f, 1.f, 1.f, 1.f };
		f32_t fSceneDesaturation = 0.f;
		// Accessibility colour-vision filter on the display colour: 0 off,
		// 1 protanopia, 2 deuteranopia, 3 tritanopia; strength 0..1.
		int32_t iColorFilterType = 0;
		f32_t fColorFilterStrength = 0.f;
		SOURCE_POST_PROCESS_SETTINGS SourcePostProcess{};
        // Session quality experiment; only 4, 8 and 12 samples are supported.
        uint32_t iSSAOSampleCount = 12u;
        // Experimental screen-space lighting, session-only and opt-in.
        // Receivers use the map PBR G-buffer; no history or off-screen scene.
        bool_t bSSGIEnabled = false;
        f32_t fSSGIStrength = 0.25f;
        f32_t fSSGIRadius = 4.f;
        uint32_t iSSGISampleCount = 8u;
        bool_t bSSREnabled = false;
        f32_t fSSRStrength = 0.5f;
        f32_t fSSRMaxDistance = 20.f;
        f32_t fSSRThickness = 0.2f;
        uint32_t iSSRStepCount = 32u;
        // Opt-in half-resolution SSGI gather; false retains the original path.
        bool_t bSSGIHalfResolution = false;
		// User texture detail: finest permitted surface mip, 0..3. Zero preserves source samplers.
		uint32_t iTextureMinMip = 0u;
        // Opt-in session methods; defaults preserve the authored render path.
        bool_t bHorizonAOEnabled = false;
        bool_t bSSRRefinementEnabled = false;
        bool_t bSSRRoughnessFilterEnabled = false;
	}RENDER_QUALITY_SETTINGS;

	enum class MATERIAL_DEBUG_VIEW : uint32_t
	{
		FINAL,
		BASE_COLOR,
		NORMAL,
		DIRECT_SPECULAR,
		REFLECTION_DELTA,
		ROUGHNESS,
		METALLIC,
		AMBIENT_OCCLUSION,
		BAKED_DIFFUSE,
		ENVIRONMENT_SPECULAR,
		DIRECT_DIFFUSE,
		SCENE_HDR,
		SCENE_TONE,
		SCENE_GRADED,
        CUBE_DIFFUSE,
		END,
	};

	struct MAP_PBR_COMPARISON_SETTINGS
	{
		bool_t bEnabled = false;
		uint32_t iLevel = 0u;
		// Direct diffuse, direct specular, RNM diffuse, environment specular.
		float4_t vContributionScale = { 1.f, 1.f, 1.f, 1.f };
		// Normal multiplier, roughness offset, legacy RNM equation (0/1), reserved.
		float4_t vSurfaceParameters = { 1.f, 0.f, 0.f, 0.f };
        f32_t fCubeDiffuseScale = 1.f;
		bool_t Is_Active(uint32_t level) const noexcept { return bEnabled && iLevel == level; }
	};

	/* Session-only comparison state; never serialized with scene quality. */
	struct MATERIAL_RENDER_SETTINGS
	{
		bool_t bUseSourceMaterials = true;
		MATERIAL_DEBUG_VIEW eDebugView = MATERIAL_DEBUG_VIEW::FINAL;
		MAP_PBR_COMPARISON_SETTINGS MapPBR;
	};

	/* Height fog is a screen space term applied where the deferred combine
	   already reconstructs world position, so terrain, buildings and
	   characters all receive it from one place. The blend group and effects
	   draw after that pass and stay clear of the fog on purpose.
	   fTopHeight is the world height the fog fades out at; fHeightFalloff is
	   the exponential rate below it. The drift fields let the fog breathe
	   without any per frame CPU work. */
	typedef struct tagHeightFogSettings
	{
		bool_t		bEnabled = false;
		float4_t	vColor = float4_t(0.55f, 0.62f, 0.72f, 1.f);
		f32_t		fDensity = 0.35f;
		f32_t		fHeightFalloff = 0.08f;
		f32_t		fTopHeight = 24.f;
		f32_t		fStartDistance = 0.f;
		f32_t		fMaximumOpacity = 0.9f;
		f32_t		fDriftSpeed = 0.f;
		f32_t		fDriftHeightAmplitude = 0.f;
		f32_t		fDriftDensityAmplitude = 0.f;
		/* Coverage turns the blanket into drifting cloud banks. 1 keeps the
		   whole map fogged; lower values thin it to patches whose total area
		   matches the fraction. The wind vector moves the pattern through
		   world XZ, so the banks travel without any CPU simulation. */
		f32_t		fCoveragePercent = 1.f;
		f32_t		fWindDirectionX = 1.f;
		f32_t		fWindDirectionZ = 0.f;
		f32_t		fWindSpeed = 0.f;
		f32_t		fPatchScale = 0.01f;
		f32_t		fPatchSoftness = 0.15f;
        // Optional source exponential model; distances/heights are runtime metres.
        bool_t bSourceExponential = false;
        float4_t vInscatteringColor = float4_t(0.f, 0.f, 0.f, 0.f);
        // xyz points toward the directional light; w is terminator-angle cosine.
        float4_t vFogLightDirection = float4_t(0.f, 1.f, 0.f, 0.f);
	}HEIGHT_FOG_SETTINGS;
}
```

MapStaticBatchObject.cpp의 거리·LOD 변경을 합친 최종 전문은 G14-3에 둔다.

#### Client/Private/ProfilerTool.cpp 전체 반영 코드

```cpp
#include "imgui.h"
#include "ProfilerTool.h"
#include "GameInstance.h"
#include "Level_Bern.h"
#include "Engine_RenderTypes.h"
#include "ClientWindowDisplay.h"
#include "UserSettingsDocument.h"
#include "RenderingReferenceGuide.h"
#include <cstring>
#include <dxgi.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <set>
#include <tuple>

namespace
{
    // SCOPE_CATALOG: kept in sync with actual CProfilerScope/GpuScope call sites.
    constexpr const char* CPU_SCOPE_CATALOG[] = {
        "Render.SSGI",
        "Render.SSGI.GatherHalf",
        "Render.SSGI.ResolveHalf",
        "Render.SSR",
        "Render.ScreenSpaceLighting",
        "Render.ScreenSpaceLighting.Copy",
        "Profiler.Memory.Sample",
        "Animation.Blend",
        "Animation.Bones.Combine",
        "Animation.Channels.Sample",
        "Animation.Channels.Update",
        "Animation.History.DebugVerify",
        "Animation.History.Sample",
        "Animation.Play",
        "Animation.SkinPalette.Bind",
        "Animation.SkinPalette.Build",
        "Animation.Transition.Build",
        "Catalog.PlayerSkills.Initialize",
        "Character.EffectCues.Admit",
        "Character.FaceMorph.Admit",
        "Character.FaceSliders.Admit",
        "Character.Initialize",
        "Character.SkillBindings.Admit",
        "CharacterAssets.Authoring.Capture",
        "CharacterAssets.Authoring.Prepare",
        "CharacterAssets.Commit",
        "CharacterAssets.FaceMorph.Prepare",
        "CharacterAssets.LevelTransition.Drain",
        "CharacterAssets.Prepare.Worker",
        "CharacterAssets.Retire.Worker",
        "Client.Render",
        "Client.Update",
        "Effect.Decal.Render",
        "Effect.FollowAnchors.Update",
        "Effect.LevelPresentation.VisibleUpdate",
        "Effect.Material.Bind",
        "Effect.Mesh.BindAndDraw",
        "Effect.Mesh.DrawSubmission",
        "Effect.Mesh.InstanceBuild",
        "Effect.Mesh.InstanceUpload",
        "Effect.Mesh.Render",
        "Effect.Occurrence.LateUpdate",
        "Effect.Occurrence.Render",
        "Effect.Occurrence.Update",
        "Effect.Particle.Render",
        "Effect.Particle.Spawn",
        "Effect.Particle.Update",
        "Effect.Playback.FixedStep",
        "Effect.Playback.FrameRebuild",
        "Effect.Playback.HistoryUpdate",
        "Effect.Playback.Update",
        "Effect.Prepare.Commit",
        "Effect.Prepare.Document",
        "Effect.Prepare.Metadata",
        "Effect.Prepare.Renderer",
        "Effect.Prepare.WorkerTarget",
        "Effect.Prewarm.Advance",
        "Effect.ProductGroups.Update",
        "Effect.Rect.Render",
        "Effect.Service.Update",
        "Effect.Spawn.Commit",
        "Effect.Spawn.CommitWorldRoots",
        "Effect.Sprite.DepthSort",
        "Effect.Sprite.DrawSubmission",
        "Effect.Sprite.InstanceBuild",
        "Effect.Sprite.InstanceUpload",
        "Effect.Trail.Render",
        "Effect.TrailAfterimage.Simulate",
        "EffectSequencer.LaneLayout",
        "EffectSequencer.Render",
        "EffectTool.AllEffectsWindow",
        "EffectTool.ArtistF.MaterialPreparation",
        "EffectTool.ArtistF.SourcePreparation",
        "EffectTool.AuthoringWindow",
        "EffectTool.DataFilesWindow",
        "EffectTool.DetailWindow",
        "EffectTool.DocumentLoad",
        "EffectTool.DocumentLoad.CanonicalBaseline",
        "EffectTool.DocumentLoad.Parse",
        "EffectTool.DocumentLoad.ValidateDrawable",
        "EffectTool.InitialIndexStep",
        "EffectTool.ModelViewWindow",
        "EffectTool.Render",
        "EffectTool.ResourceGrid",
        "EffectTool.ThumbnailTrim",
        "EffectTool.UnifiedEffectTree",
        "EffectTool.UnifiedFamilyRows",
        "Engine.Camera.Update",
        "Engine.Input.Update",
        "Engine.LateUpdate",
        "Engine.LevelUpdate",
        "Engine.ObjectUpdate",
        "Engine.Physics",
        "Engine.PostPhysicsUpdate",
        "Engine.PriorityUpdate",
        "Engine.Sound.Update",
        "ImGui.Composition.Details",
        "ImGui.Composition.PatternTree",
        "ImGui.Composition.PatternTree.Rebuild",
        "ImGui.Composition.Patterns",
        "ImGui.Composition.PresentationIndex.Rebuild",
        "ImGui.Composition.PresentationResources",
        "ImGui.Composition.Resources",
        "ImGui.Composition.Resources.Filter",
        "ImGui.Composition.Resources.Tree.Draw",
        "ImGui.Composition.Resources.Tree.Rebuild",
        "ImGui.Composition.Timeline",
        "ImGui.Composition.Timeline.Draw",
        "ImGui.Composition.Timeline.Layout",
        "ImGui.Composition.Toolbar",
        "Kouku.Presentation.BundleSample",
        "Kouku.Presentation.EncounterVisuals",
        "Kouku.Presentation.FrameLights",
        "Kouku.Presentation.PreviewPreparation",
        "Kouku.Presentation.SharedPresentation",
        "Kouku.Presentation.Update",
        "ImGui.BackendSubmit",
        "ImGui.BuildAndSubmit",
        "ImGui.DX11.BackupState",
        "ImGui.DX11.DeviceObjects",
        "ImGui.DX11.DrawSubmission",
        "ImGui.DX11.GrowBuffers",
        "ImGui.DX11.MapBuffers",
        "ImGui.DX11.RestoreState",
        "ImGui.DX11.SetupState",
        "ImGui.DX11.TextureUpdate",
        "ImGui.DX11.UploadBuffers",
        "ImGui.DeveloperTools",
        "ImGui.FinalizeDrawData",
        "ImGui.Hub.Build",
        "ImGui.Hub.CameraAndPlayer",
        "ImGui.Hub.FollowCamera",
        "ImGui.Hub.KoukuArena",
        "ImGui.Hub.KoukuCompletePlay",
        "ImGui.Hub.KoukuUIPreview",
        "ImGui.Hub.LevelNavigation",
        "ImGui.Hub.SequenceViewer.Build",
        "ImGui.Hub.SequenceViewer.Refresh",
        "ImGui.Hub.SequenceViewer.Update",
        "ImGui.Hub.ServerArena",
        "ImGui.Hub.ValtanCompletePlay",
        "ImGui.NewFrame",
        "ImGui.NewFrame.Core",
        "ImGui.NewFrame.DX11",
        "ImGui.NewFrame.Win32",
        "ImGui.PlatformRender",
        "ImGui.PlatformUpdate",
        "ImGui.PlatformViewport.Present",
        "ImGui.PlatformViewport.Render",
        "ImGui.ProfilerDetails",
        "ImGui.ProfilerOverlay",
        "ImGui.RenderDrawData",
        "ImGui.Tool.Animation.Build",
        "ImGui.Tool.Animation.Update",
        "ImGui.Tool.Balance.Build",
        "ImGui.Tool.Balance.Update_ServerRuntimeSetPublishJob",
        "ImGui.Tool.Balance.Update_ValtanSaveJob",
        "ImGui.Tool.Camera.Build",
        "ImGui.Tool.Camera.Update",
        "UI.Runtime.Chat.Update",
        "ImGui.Tool.Composition.Build",
        "ImGui.Tool.CompositionProfiler.Build",
        "ImGui.Tool.EffectV1.Build",
        "ImGui.Tool.EffectV1.Update",
        "ImGui.Tool.EffectV1.Update_AuthoringWorkspace",
        "ImGui.Tool.EffectV2.Build",
        "ImGui.Tool.EffectV2.Update_AuthoringWorkspace",
        "ImGui.Tool.Equipment.Build",
        "ImGui.Tool.HUDLayout.Build",
        "ImGui.Tool.KoukuBoss.Build",
        "ImGui.Tool.Map.Build",
        "ImGui.Tool.Map.Update",
        "ImGui.Tool.Open",
        "UI.Runtime.Party.Update",
        "ImGui.Tool.Rendering.Build",
        "ImGui.Tool.SequenceBenchmark.Build",
        "ImGui.Tool.ValtanBoss.Build",
        "ImGui.Tool.ValtanBoss.Render_LogicPatternWindow",
        "ImGui.Tool.ValtanBoss.Update",
        "ImGui.Tool.ValtanComposition.Update_SaveState",
        "ImGui.Tool.WorldObjects.Build",
        "ImGui.Tool.WorldObjects.Update",
        "Level.Kouku.Camera.Create",
        "Level.Kouku.CameraShots.Load",
        "Level.Kouku.Controller.Prepare",
        "Level.Kouku.DebugTriggers.Load",
        "Level.Kouku.DeployCommit",
        "Level.Kouku.InitialVisibility",
        "Level.Kouku.Initialize",
        "Level.Kouku.MapLights.Load",
        "Level.Kouku.MapPlacementCommit",
        "Level.Kouku.Replication.Initialize",
        "Level.Kouku.SelfMotion.Load",
        "Level.Kouku.StageMarkers.Load",
        "Level.Kouku.UI.Create",
        "Level.Kouku.WorldSequence.Load",
        "Loader.EffectPreparation",
        "Loader.LevelLoad",
        "MainApp.DebugTools.Update",
        "MainApp.Engine.Update",
        "MainApp.InputAndUI.Update",
        "MainApp.LevelAndEnvironment.Update",
        "MainApp.Presentation.Prepare",
        "Map.Batch.BindAndDraw",
        "Map.Batch.CullAndPack",
        "Map.Batch.InstanceUpload",
        "Map.Batch.ShadowPrepare",
        "Map.Batch.ShadowUpload",
        "Map.Batch.Visibility",
        "Model.Load.Animations",
        "Model.Load.Binary",
        "Model.Load.Bones",
        "Model.Load.Decode",
        "Model.Load.Materials",
        "Model.Load.MaterialWorker",
        "Model.Load.MaterialJoin",
        "Model.Load.Meshes",
        "Texture.Cache.Lookup",
        "Texture.Cache.SameKeyWait",
        "Texture.Load.FileAndUpload",
        "Navigation.AStar",
        "Navigation.FindPath",
        "Navigation.Follow",
        "Navigation.Request",
        "Navigation.RoundCorners",
        "Navigation.Simplify",
        "Network.DrainAndDispatch",
        "Network.PlayerAssets.Advance",
        "Network.PlayerPresentation.CommitSpawn",
        "Network.PlayerPresentation.Replace",
        "Picking.CopyPixel",
        "Picking.MapWait",
        "Picking.ReadPixel",
        "Picking.Readback",
        "Profiler.Capture.Snapshot",
        "Profiler.Panel.Refresh",
        "Map.Batch.Material.Bind",
        "Map.Batch.Pass.Apply",
        "Map.Batch.Mesh.Submit",
        "Map.Shadow.Material.Bind",
        "Map.Shadow.Pass.Apply",
        "Map.Shadow.Mesh.Submit",
        "Effect.Particle.Update.Worker",
        "Effect.Particle.Update.Join",
        "Render.BeginFrame",
        "Render.Blend",
        "Render.Bloom",
        "Render.BossShowcase",
        "Render.Combined",
        "Render.Debug",
        "Render.DisplayOverlays",
        "Render.Draw",
        "Render.Final",
        "Render.Lights",
        "Render.Lights.WorldReceivers",
        "Render.Lights.CharacterReceivers",
        "Render.Lights.StageAndSubmit",
        "Render.Lights.UploadAndDraw",
        "Render.NonBlend",
        "Render.NonLight",
        "Render.Portraits",
        "Render.Present",
        "Render.Priority",
        "Render.FinalCameraSubmission",
        "Render.Picking",
        "Render.SSAO",
        "Render.SceneColorSnapshot",
        "Render.SceneHDR",
        "Render.ScreenPosts",
        "Render.Shadow",
        "Render.Shadow.CacheAdmission",
        "Render.Shadow.CacheCopy",
        "Render.Shadow.StaticBuild",
        "Render.Shadow.Dynamic",
        "Render.SubmitFrameProviders",
        "Render.UI",
        "Render.UIText",
        "Render.World",
        "Replication.Update",
        "Shader.BuildBindings",
        "Shader.CreateEffect",
        "Shader.CreateInputLayouts",
        "Shader.Load",
        "Shader.ReadBytecode",
        "Tool.Composition.ReloadCanonical",
        "Tool.Composition.SaveReload",
        "Tool.ValtanBossTool.ReloadCanonicalGraph",
        "UI.Runtime.BossHealthBar.Update",
        "UI.Runtime.BossImmuneGauge.Update",
        "UI.Runtime.CharacterSelectWindow.Update",
        "UI.Runtime.ChargeGauge.Update",
        "UI.Runtime.CombatHUD.Update",
        "UI.Runtime.EstherGauge.Update",
        "UI.Runtime.ItemQuickSlots.Update",
        "UI.Runtime.ItemUpgrade.Update",
        "UI.Runtime.LanceMasterIdentityGauge.Update",
        "UI.Runtime.LobbyButtons.Update",
        "UI.Runtime.Minimap.Update",
        "UI.Runtime.PlayerHealthManaBar.Update",
        "UI.Runtime.QuickSlotFlash.Update",
        "UI.Runtime.SkillCooldowns.Update",
        "UI.Runtime.SkillIcons.Update",
        "WorldSequence.CollectLoadTargets",
        "WorldSequence.CommitPrepared",
        "WorldSequence.Document.Load",
        "WorldSequence.Document.Parse",
        "WorldSequence.Document.Validate",
        "WorldSequence.PrepareArea",
    };
    constexpr const char* GPU_SCOPE_CATALOG[] = {
        "Render.SSGI",
        "Render.SSGI.GatherHalf",
        "Render.SSGI.ResolveHalf",
        "Render.SSR",
        "Render.ScreenSpaceLighting.Copy",
        "ImGui.BackendSubmit",
        "ImGui.PlatformViewport.Present",
        "ImGui.PlatformViewport.Render",
        "ImGui.RenderDrawData",
        "Picking.CopyPixel",
        "Render.BeginFrame",
        "Render.Blend",
        "Render.Bloom",
        "Render.BossShowcase",
        "Render.Combined",
        "Render.Debug",
        "Render.DisplayOverlays",
        "Render.Draw",
        "Render.Final",
        "Render.Lights",
        "Render.Lights.WorldReceivers",
        "Render.Lights.CharacterReceivers",
        "Render.NonBlend",
        "Render.NonLight",
        "Render.Portraits",
        "Render.Priority",
        "Render.FinalCameraSubmission",
        "Render.Picking",
        "Render.SSAO",
        "Render.SceneColorCopy",
        "Render.SceneHDR",
        "Render.ScreenPosts",
        "Render.Shadow",
        "Render.Shadow.CacheCopy",
        "Render.Shadow.StaticBuild",
        "Render.Shadow.Dynamic",
        "Render.UI",
        "Render.UIText",
    };
    constexpr ImGuiTableFlags TABLE_FLAGS = ImGuiTableFlags_RowBg |
        ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;

    constexpr const char* COUNTER_LABELS[] = {
        "계측된 Engine draw 호출",
        "인스턴싱 draw 호출",
        "인스턴싱 제출 인스턴스",
        "제출 인덱스 (인스턴스 반영)",
        "렌더 큐: 우선",
        "렌더 큐: 그림자",
        "렌더 큐: 불투명",
        "렌더 큐: 투명",
        "맵 배치 레코드",
        "맵 가시 인스턴스",
        "맵 배치 묶음",
        "맵 개별 오브젝트",
        "텍스처 요청",
        "텍스처 경로 캐시 적중",
        "텍스처 내용 캐시 적중",
        "새 텍스처 SRV 생성 성공",
        "텍스처 GPU 추정 바이트",
        "클라이언트 경로 탐색 요청",
        "클라이언트 탐색 노드",
        "클라이언트 경로 탐색 시간 (us)",
        "클라이언트 경로 셀",
        "장면 색 복사",
        "장면 색 복사 바이트 (원본 크기)",
        "ImGui draw 목록",
        "ImGui 정점",
        "ImGui 인덱스",
        "ImGui draw 명령",
        "ImGui 실제 draw 호출",
        "ImGui 콜백",
        "ImGui 렌더 창",
        "ImGui 활성 창",
        "ImGui 외부 뷰포트",
        "ImGui 정점 업로드 바이트",
        "ImGui 인덱스 업로드 바이트",
        "ImGui 상수 업로드 바이트",
        "ImGui 텍스처 업로드 바이트",
        "ImGui 버퍼 확장",
        "ImGui 텍스처 생성",
        "ImGui 텍스처 갱신",
        "ImGui 장치 객체 생성",
        "ImGui 버퍼 Map 실패",
        "피킹 GPU 읽기",
        "피킹 읽기 바이트",
        "간접 draw 호출",
        "간접 인덱스 (LOD0 상한)",
        "그림자 캐시 적중",
        "그림자 캐시 실패",
        "정적 그림자 제출 후보",
        "동적 그림자 제출 후보",
        "맵 컬링 후보",
        "맵 컬링 통과",
        "맵 LOD0 draw",
        "맵 LOD1 draw",
        "맵 LOD2+ draw",
        "맵 원본 LOD0 인덱스",
        "맵 선택 LOD 제출 인덱스",
        "광원 제출 레코드 (수광 패스별 중복)",
        "광원 draw 호출",
        "광원 레코드 업로드 바이트",
        "생성 LOD가 있는 맵 draw",
        "로컬 광원 컬링 후보",
        "로컬 광원 컬링 제외",
        "이펙트 bounds 후보",
        "이펙트 bounds 컬링 제외",
        "트리거 마커 표본 시도",
        "트리거 진입 이력 요청",
        "주변 이펙트 정지",
        "주변 이펙트 진행",
        "ImGui Present 시도",
        "ImGui Present 지연 (busy)",
        "ImGui Present 실패",
        "ImGui Present 가림",
        "맵 가시성 캐시 적중",
        "맵 가시성 재계산",
        "빈 맵 배치 렌더",
        "가시 맵 배치 렌더",
        "맵 배치 bounds 제외",
        "맵 인스턴스 업로드 바이트",
        "저작 숨김 NPC 갱신",
        "bounds 무효화 주변 이펙트 갱신",
        "NPC 컬링 후보",
        "NPC 컬링 제외",
        "NPC 지연 pose 평가",
        "메시 제출 호출",
        "메시 인스턴스 제출",
        "메시 인덱스 제출",
        "고유 CMesh 객체",
        "고유 메시 계측 누락",
        "주변 이펙트 시간 제한 적용",
        "주변 이펙트 제외 시간 합계 (effect-us, CPU 시간 아님)",
        "주변 이펙트 fixed-step 합계",
        "주변 이펙트 단일 갱신 최대 fixed-step",
        "맵 공간 묶음 수",
        "맵 공간 묶음 근거리 제출",
        "맵 공간 묶음 원거리 제출",
        "맵 공간 묶음 원본 제출 수",
        "맵 공간 묶음 제출 인덱스",
        "맵 공간 묶음 원본 인덱스",
        "맵 공간 묶음 GPU 버퍼 바이트",
        "맵 공간 묶음 무효화",
        "맵 공간 묶음 활성 수",
        "맵 공간 묶음 HLOD 활성 수",
        "동일 모델 결합 전 source draw",
        "동일 모델 결합 draw",
        "조명 bank 결합 전 source draw",
        "조명 bank 결합 draw",
        "가림 검사 후보 배치",
        "가림 검사 실행 배치",
        "가림 제외 배치",
        "가림 제외 mesh 제출 (인스턴싱 결합 전)",
        "가림 제외 인덱스",
        "가림 깊이 생성 배치",
        "가림 깊이 래스터 삼각형",
        "거리 검사 인스턴스",
        "거리 제외 인스턴스",
        "거리 제외 원본 인덱스",
        "동일 카메라·정적 배치 가림 결과 재사용",
        "맵 CPU 준비 batch",
        "맵 CPU 큰 작업 묶음",
        "맵 CPU caller 완료 작업",
        "맵 CPU worker 완료 작업",
        "맵 CPU 보조 callback 제출",
        "공유 대상 애니메이션 샘플 요청",
        "애니메이션 샘플 캐시 재사용",
        "파티클 root 역행렬 요청",
        "파티클 root 역행렬 캐시 재사용",
        "파티클 CPU caller 완료 작업",
        "파티클 CPU worker 완료 작업",
        "파티클 CPU 보조 callback 제출",
    };
    static_assert(std::size(COUNTER_LABELS) == static_cast<size_t>(Engine::EProfilerCounter::Count));

    const char* Scope_Description(std::string_view name)
    {
        struct FLabel { std::string_view Name; const char* Description; };
        static constexpr FLabel labels[] = {
            {"Client.Update", "클라이언트 전체 갱신"}, {"Client.Render", "클라이언트 렌더 제출"},
            {"Render.World", "월드 렌더 제출"}, {"Render.Draw", "렌더 패스 전체"},
            {"Render.FinalCameraSubmission", "최종 카메라 가시성·제출"},
            {"Render.MapOcclusion", "맵 CPU 가림 처리 전체"},
            {"Render.MapOcclusion.Rasterize", "맵 CPU 가림 깊이 생성"},
            {"Render.MapOcclusion.Test", "맵 CPU 가림 검사·제외 집계"},
            {"Map.Visibility.Prepare", "맵 CPU 준비·분배 전체"},
            {"Map.Visibility.Dispatch", "맵 CPU 작업 실행·동기화"},
            {"Map.Visibility.Join", "맵 CPU worker 잔여 대기"},
            {"Map.Visibility.Worker", "맵 CPU 보조 스레드 계산"},
            {"Render.SubmitFrameProviders", "프레임 제공자 제출"},
            {"Render.NonBlend", "불투명 메시·G-buffer"}, {"Render.Shadow", "그림자 전체"},
            {"Render.Shadow.CacheAdmission", "그림자 캐시 조건 확인"},
            {"Render.Shadow.CacheCopy", "그림자 캐시 복사"},
            {"Render.Shadow.StaticBuild", "정적 그림자 생성"},
            {"Render.Shadow.Dynamic", "동적 그림자 생성"},
            {"Render.SSAO", "화면 공간 주변 차폐"}, {"Render.Lights", "월드 직접광 합산"},
            {"Render.SSGI", "실험 화면 공간 간접광"},
            {"Render.SSGI.GatherHalf", "GI 절반 해상도 계산"}, {"Render.SSGI.ResolveHalf", "GI 경계 보존 합성"}, {"Render.SSR", "실험 화면 공간 반사"},
            {"Render.ScreenSpaceLighting", "실험 화면 공간 조명 전체"},
            {"Render.ScreenSpaceLighting.Copy", "화면 공간 조명 입력 복사"},
            {"Render.Lights.WorldReceivers", "일반 수광체 직접광"},
            {"Render.Lights.CharacterReceivers", "캐릭터 수광체 직접광"},
            {"Render.Lights.StageAndSubmit", "광원 컬링·레코드 구성·제출"},
            {"Render.Lights.UploadAndDraw", "광원 상수 바인딩·draw 제출"},
            {"Render.SceneHDR", "HDR 장면 합성 전체"},
            {"Render.Combined", "직접광·베이크·환경광 합성"},
            {"Render.NonLight", "비조명 재질"}, {"Render.Blend", "투명 메시·이펙트"},
            {"Render.ScreenPosts", "화면 후처리"}, {"Render.Bloom", "빛 번짐"},
            {"Render.Final", "톤 매핑·색보정·최종 출력"},
            {"Render.DisplayOverlays", "출력 오버레이"}, {"Render.UI", "제품 UI"},
            {"Render.UIText", "제품 글자"}, {"Render.Debug", "디버그 도형"},
            {"Render.Picking", "선택 피킹"}, {"Render.Priority", "우선 렌더"},
            {"Render.Present", "화면 제출·드라이버 대기"}, {"Render.BeginFrame", "프레임 준비"},
            {"Render.Portraits", "캐릭터 초상 렌더"}, {"Render.BossShowcase", "보스 연출"},
            {"Render.SceneColorSnapshot", "장면 색 스냅샷 준비"},
            {"Render.SceneColorCopy", "장면 색 GPU 복사"},
            {"MainApp.InputAndUI.Update", "입력·UI 갱신"},
            {"MainApp.Engine.Update", "엔진 갱신"},
            {"MainApp.Presentation.Prepare", "표현 리소스 준비"},
            {"MainApp.DebugTools.Update", "저작 도구 갱신"},
            {"MainApp.LevelAndEnvironment.Update", "레벨·환경 갱신"},
            {"Engine.Physics", "물리 시뮬레이션"}, {"Engine.Sound.Update", "음향 갱신"},
            {"Engine.Camera.Update", "카메라 갱신"}, {"Engine.Input.Update", "입력 갱신"},
            {"Map.Batch.Visibility", "맵 배치 가시성 검사"},
            {"Map.Batch.Material.Bind", "맵 재질 바인딩"},
            {"Map.Batch.Pass.Apply", "맵 셰이더 패스 적용"},
            {"Map.Batch.Mesh.Submit", "맵 메시 draw 제출"},
            {"Map.Batch.InstanceUpload", "맵 인스턴스 업로드"},
            {"Map.Batch.CullAndPack", "맵 컬링·인스턴스 구성"},
            {"Map.Batch.BindAndDraw", "맵 바인딩·draw 전체"},
            {"ImGui.BackendSubmit", "도구 GPU 제출 준비"},
            {"ImGui.BuildAndSubmit", "도구 UI 구성·제출"},
            {"ImGui.RenderDrawData", "도구 UI draw 제출"},
            {"ImGui.PlatformViewport.Present", "외부 창 제출·대기"},
            {"Profiler.Panel.Refresh", "프로파일러 표 집계"},
            {"Profiler.Timeline.Snapshot", "타임라인 프레임 복사"},
            {"Profiler.Memory.Sample", "OS 메모리 표본 조회"},
            {"Profiler.Capture.Snapshot", "캡처 스냅샷 복사"},
            {"Network.DrainAndDispatch", "수신 패킷 배분 (클라이언트)"},
        };
        for (const auto& label : labels) if (label.Name == name) return label.Description;
        if (name.starts_with("ClassMovie.")) return "캐릭터 선택 무비 갱신·표본";
        if (name.starts_with("Animation.")) return "애니메이션·스키닝 CPU 준비";
        if (name.starts_with("Effect.")) return "이펙트 갱신·제출";
        if (name.starts_with("Map.")) return "맵 가시성·재질·제출";
        if (name.starts_with("Npc.")) return "NPC 갱신·제출";
        if (name.starts_with("Ambient.")) return "주변 이펙트";
        if (name.starts_with("ImGui.") || name.starts_with("EffectTool.") || name.starts_with("EffectSequencer.")) return "저작 도구 UI";
        if (name.starts_with("Navigation.")) return "클라이언트 경로 탐색";
        if (name.starts_with("Network.") || name.starts_with("Replication.")) return "네트워크·복제 표현";
        if (name.starts_with("UI.")) return "제품 UI 갱신";
        if (name.starts_with("Loader.") || name.starts_with("Model.Load") || name.starts_with("Texture.")) return "리소스 로드·준비";
        if (name.starts_with("Character")) return "캐릭터 준비·교체";
        if (name.starts_with("WorldSequence.") || name.starts_with("Kouku.")) return "장면·패턴 연출";
        if (name.starts_with("Shader.")) return "셰이더 준비";
        if (name.starts_with("Picking.")) return "선택·GPU 읽기";
        if (name.starts_with("Engine.") || name.starts_with("Level.")) return "엔진·레벨 처리";
        return "추가 계측 구간";
    }

    void Scope_Label(const char* name)
    {
        ImGui::Text("%s | %s", Scope_Description(name), name);
    }

    bool Contains_CaseInsensitive(std::string_view text, const char* query)
    {
        if (!query || !query[0]) return true;
        const std::string_view needle(query);
        return std::search(text.begin(), text.end(), needle.begin(), needle.end(),
            [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) ==
                std::tolower(static_cast<unsigned char>(b)); }) != text.end();
    }

    std::string Capture_PathLabel(const std::filesystem::path& path)
    {
        const auto text = path.u8string();
        return std::string(text.begin(), text.end());
    }

    const char* Gpu_Status(Engine::EProfilerGpuFrameStatus status)
    {
        switch (status)
        {
        case Engine::EProfilerGpuFrameStatus::Unsupported: return "미지원";
        case Engine::EProfilerGpuFrameStatus::Pending: return "결과 대기";
        case Engine::EProfilerGpuFrameStatus::Valid: return "유효";
        case Engine::EProfilerGpuFrameStatus::Disjoint: return "타임스탬프 무효";
        case Engine::EProfilerGpuFrameStatus::Dropped: return "누락";
        case Engine::EProfilerGpuFrameStatus::Error: return "쿼리 오류";
        }
        return "알 수 없음";
    }

    void Unobserved_Row(const char* name, int columns)
    {
        ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextDisabled("%s", name);
        for (int i = 1; i < columns; ++i)
        {
            ImGui::TableNextColumn(); ImGui::TextDisabled(i == 1 ? "미관측" : "--");
        }
    }
}

Client::CProfilerTool::CProfilerTool(ID3D11Device* device)
{
    if (!device) return;
    m_CaptureContext.DeviceCreationFlags = device->GetCreationFlags();
    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    DXGI_ADAPTER_DESC desc{};
    if (FAILED(device->QueryInterface(IID_PPV_ARGS(dxgiDevice.GetAddressOf()))) ||
        FAILED(dxgiDevice->GetAdapter(adapter.GetAddressOf())) || FAILED(adapter->GetDesc(&desc))) return;
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, nullptr, 0, nullptr, nullptr);
    if (bytes > 1)
    {
        std::string text(static_cast<size_t>(bytes), '\0');
        WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, text.data(), bytes, nullptr, nullptr);
        text.pop_back();
        m_CaptureContext.Adapter = std::move(text);
    }
}

void Client::CProfilerTool::Begin_Capture(Engine::CProfiler& profiler)
{
    profiler.Reset_History();
    CProfilerCaptureIO::Reset_MovementSamples();
    profiler.Set_Enabled(true);
    m_fLastRefreshTime = -1.0;
}

void Client::CProfilerTool::Refresh(Engine::CProfiler& profiler)
{
    if (!m_bCatalogRegistered)
    {
        for (const char* name : CPU_SCOPE_CATALOG) profiler.Register_ScopeName(name);
        for (const char* name : GPU_SCOPE_CATALOG) profiler.Register_ScopeName(name);
        m_bCatalogRegistered = true;
    }
    Engine::CProfilerScope scope(&profiler, "Profiler.Panel.Refresh");
    m_iMainThreadId = profiler.Get_MainThreadId();
    m_iHistoryFrames = profiler.Get_HistoryFrameCount();
    profiler.Get_ScopeNames(m_ScopeNames);
    const size_t window = static_cast<size_t>((std::max)(m_iWindowFrameInput, 1));
    m_SaveWindowCoverage = profiler.Get_CaptureWindow(Save_FrameWindow());
    profiler.Get_ScopeAggregates(window, m_Aggregates);
    profiler.Get_GpuScopeAggregates(window, m_GpuAggregates, m_iGpuValidFrames, m_iGpuPartialFrames);
    profiler.Get_WindowFrameStats(window, m_fWindowCpuAvgMs, m_fWindowCpuMaxMs,
        m_fWindowGpuAvgMs, m_fWindowGpuMaxMs, m_iWindowFrames, &m_iGpuFrameValidFrames);
    profiler.Get_LongOperations(m_LongOperations);
    m_bLiveValid = profiler.Get_LiveStats(m_Live);
    auto latest = profiler.Snapshot(1);
    m_LatestFrame = latest.Frames.empty() ? Engine::FProfilerFrame{} : std::move(latest.Frames.back());
    m_fUnattributedCpuMs = m_LatestFrame.CpuFrameMs;
    std::vector<std::pair<uint64_t, uint64_t>> intervals;
    for (const auto& sample : m_LatestFrame.CpuScopes)
    {
        if (sample.ThreadId != m_iMainThreadId) continue;
        const auto begin = (std::max)(sample.BeginTick, m_LatestFrame.FrameBeginTick);
        const auto end = (std::min)(sample.EndTick, m_LatestFrame.FrameEndTick);
        if (end > begin) intervals.emplace_back(begin, end);
    }
    std::sort(intervals.begin(), intervals.end());
    uint64_t covered = 0, end = m_LatestFrame.FrameBeginTick;
    for (const auto& range : intervals)
    {
        if (range.second > end) covered += range.second - (std::max)(end, range.first);
        end = (std::max)(end, range.second);
    }
    m_fUnattributedCpuMs = (std::max)(0.0, m_LatestFrame.CpuFrameMs - profiler.Ticks_ToMs(covered));
    m_bCpuRowsDirty = true;
    m_bComparisonRefresh = true;
    m_bTimelineRefresh = true;
}

const char_t* Client::CProfilerTool::Scope_Name(uint32_t id) const
{
    return id < m_ScopeNames.size() ? m_ScopeNames[id].c_str() : "<unknown>";
}

std::string Client::CProfilerTool::Thread_Label(uint32_t id) const
{
    return id == m_iMainThreadId ? "메인" : "작업 스레드 " + std::to_string(id);
}

size_t Client::CProfilerTool::Save_FrameWindow() const noexcept
{
    return m_bSaveWindowOnly ? static_cast<size_t>(std::clamp(m_iSaveFrameInput,
        1, static_cast<int32_t>(Engine::CProfiler::MAX_HISTORY_FRAMES))) : Engine::CProfiler::MAX_HISTORY_FRAMES;
}

void Client::CProfilerTool::Request_Save(Engine::CProfiler& profiler)
{
    if (m_Exporter.IsSaving()) return;
    std::string error;
    if (!CProfilerCaptureIO::Validate_Name(m_CaptureName.data(), &error))
    { m_strCaptureStatus = error; return; }
    Engine::FProfilerCaptureSnapshot snapshot;
    {
        Engine::CProfilerScope scope(&profiler, "Profiler.Capture.Snapshot");
        snapshot = profiler.Snapshot(Save_FrameWindow());
    }
    if (snapshot.Frames.empty())
    { m_strCaptureStatus = "완료 프레임이 없습니다. 수집을 켜고 기다린 뒤 저장하세요."; return; }
    auto context = Sample_Context();
    CProfilerCaptureIO::Copy_MovementSamples(snapshot, context);
    const uint64_t frame = snapshot.Frames.back().FrameNumber;
    std::filesystem::path output;
    if (!CProfilerCaptureIO::Make_NamedPath(m_CaptureName.data(), frame, output, &error))
    { m_strCaptureStatus = error; return; }
    // Describe this immutable export, not the live history that keeps changing while it is saved.
    const auto& coverage = snapshot.CaptureWindow;
    size_t gpuPending = 0, gpuDropped = 0;
    uint64_t cpuScopesDropped = 0, gpuScopesDropped = 0;
    for (const auto& savedFrame : snapshot.Frames)
    {
        gpuPending += savedFrame.GpuStatus == Engine::EProfilerGpuFrameStatus::Pending;
        gpuDropped += savedFrame.GpuStatus == Engine::EProfilerGpuFrameStatus::Dropped;
        cpuScopesDropped += savedFrame.DroppedCpuScopes;
        gpuScopesDropped += savedFrame.DroppedGpuScopes;
    }
    const std::string coverageText = std::to_string(snapshot.Frames.size()) + "프레임 (" +
        std::to_string(coverage.FirstSavedFrameNumber) + " - " + std::to_string(coverage.LastSavedFrameNumber) +
        ") | 보관 중 제외 " + std::to_string(coverage.ExcludedRetainedFrames) +
        " | 초기화 이후 퇴출 " + std::to_string(coverage.EvictedFramesSinceReset) +
        " | 저장 GPU 결과 대기 " + std::to_string(gpuPending) + " / 누락 " + std::to_string(gpuDropped) +
        " | 저장 CPU/GPU 구간 누락 " + std::to_string(cpuScopesDropped) + "/" + std::to_string(gpuScopesDropped);
    if (m_Exporter.BeginSave(std::move(snapshot), output, &error, std::move(context)))
    {
        m_strSavingCoverage = coverageText;
        m_strCaptureStatus = "백그라운드에서 JSON 저장 중... " + m_strSavingCoverage;
    }
    else m_strCaptureStatus = error;
}

Client::FProfilerCaptureContext Client::CProfilerTool::Sample_Context() const
{
    auto& game = Engine::CGameInstance::Get();
    auto context = m_CaptureContext;
    context.Valid = true;
    context.LevelId = game.Get_CurrentLevelID();
    const HWND foregroundWindow = GetForegroundWindow();
    DWORD foregroundProcessId = 0;
    context.ClientWindowForeground = foregroundWindow == g_hWnd;
    context.ProcessForeground = nullptr != foregroundWindow &&
        0 != GetWindowThreadProcessId(foregroundWindow, &foregroundProcessId) &&
        GetCurrentProcessId() == foregroundProcessId;
    context.WindowMinimized = CClientWindowDisplay::Is_Minimized();
    const auto& userSettings = CUserSettings::Get();
    context.ForegroundFpsLimit = userSettings.Get_FrameLimit(true);
    context.BackgroundFpsLimit = userSettings.Get_FrameLimit(false);
    context.EffectiveFpsLimit = userSettings.Get_FrameLimit(context.ProcessForeground);
    const auto viewport = game.Get_ViewportSize();
    context.Viewport = {viewport.x, viewport.y};
    if (const auto* camera = game.Get_CamPosition())
        context.CameraPosition = {camera->x, camera->y, camera->z, camera->w};
    if (const auto* view = game.Get_Transform(Engine::D3DTS::VIEW))
        std::memcpy(context.ViewMatrix.data(), view, sizeof(*view));
    if (const auto* projection = game.Get_Transform(Engine::D3DTS::PROJ))
        std::memcpy(context.ProjectionMatrix.data(), projection, sizeof(*projection));
    const auto& shadow = game.Get_ShadowLightDesc().Settings;
    context.ShadowEnabled = shadow.bEnabled;
    context.ShadowWidth = shadow.fOrthographicWidth;
    context.ShadowHeight = shadow.fOrthographicHeight;
    context.ShadowStrength = shadow.fStrength;
    const auto quality = game.Get_RenderQualitySettings();
    context.SSAOEnabled = quality.bSSAOEnabled;
    context.BloomEnabled = quality.bBloomEnabled;
    context.FXAAEnabled = quality.bFXAAEnabled;
    auto& options = context.RenderingOptions;
    options.clear(); context.RenderingAssets.clear();
    options["Texture.minimumMip"] = quality.iTextureMinMip;
    const auto visibility = game.Get_MapVisibilitySettings();
    options["MapVisibility.parallelPreparation"] = visibility.ParallelPreparationEnabled;
    options["MapVisibility.occlusion"] = visibility.OcclusionEnabled;
    options["Camera.bernEntranceAuthoredFovYDegrees"] = CLevel_Bern::Get_EntranceCinematicAuthoredFovYDegrees();
    options["Camera.bernEntranceFovYOverrideEnabled"] = CLevel_Bern::Get_EntranceCinematicFovYOverride().has_value();
    if (const auto fov = CLevel_Bern::Get_EntranceCinematicFovYOverride())
        options["Camera.bernEntranceFovYOverrideDegrees"] = *fov;
    options["MapVisibility.occlusionPolicyAllowed"] = visibility.OcclusionEnabled &&
        game.Get_CurrentLevelID() == ETOUI(LEVEL::BERN) && !game.Is_SceneEnvironmentReplaced();
    options["Optimization.frustum"] = game.Get_RenderOptimizationSettings().FrustumEnabled;
    options["MapVisibility.distance"] = visibility.DistanceEnabled;
    options["MapVisibility.distanceScale"] = visibility.DistanceScale;
    options["MapVisibility.distanceMaxPixels"] = visibility.DistanceMaxPixels;
    options["MapVisibility.distanceEffectiveMaxPixels"] = visibility.Get_DistancePixelLimit();
    const auto vector3 = [&](const std::string& key, const auto& value)
    { options[key + ".x"] = value.x; options[key + ".y"] = value.y; options[key + ".z"] = value.z; };
    const auto vector4 = [&](const std::string& key, const auto& value)
    { vector3(key, value); options[key + ".w"] = value.w; };
    options["SSAO.radius"] = quality.fSSAORadius; options["SSAO.bias"] = quality.fSSAOBias;
    options["SSAO.intensity"] = quality.fSSAOIntensity; options["SSAO.power"] = quality.fSSAOPower;
    options["SSAO.distanceFade"] = quality.fSSAODistanceFade; options["SSAO.samples"] = quality.iSSAOSampleCount;
    options["SSGI.enabled"] = quality.bSSGIEnabled; options["SSGI.strength"] = quality.fSSGIStrength;
    options["SSGI.radius"] = quality.fSSGIRadius; options["SSGI.samples"] = quality.iSSGISampleCount;
    options["SSGI.halfResolution"] = quality.bSSGIHalfResolution;
    options["SSR.enabled"] = quality.bSSREnabled; options["SSR.strength"] = quality.fSSRStrength;
    options["SSR.maxDistance"] = quality.fSSRMaxDistance; options["SSR.thickness"] = quality.fSSRThickness;
    options["SSR.steps"] = quality.iSSRStepCount;
    options["Bloom.threshold"] = quality.fBloomThreshold; options["Bloom.softKnee"] = quality.fBloomSoftKnee;
    options["Bloom.intensity"] = quality.fBloomIntensity; options["Bloom.scatter"] = quality.fBloomScatter;
    vector4("Bloom.tint", quality.vBloomTint);
    options["Tone.exposure"] = quality.fExposure; options["Tone.whitePoint"] = quality.fWhitePoint;
    options["Tone.gamma"] = quality.fGamma; options["Tone.sceneDesaturation"] = quality.fSceneDesaturation;
    options["ColorFilter.type"] = quality.iColorFilterType; options["ColorFilter.strength"] = quality.fColorFilterStrength;
    options["FXAA.subpixel"] = quality.fFXAASubpixel; options["FXAA.edgeThreshold"] = quality.fFXAAEdgeThreshold;
    options["FXAA.edgeThresholdMin"] = quality.fFXAAEdgeThresholdMin;
    options["Shadow.near"] = shadow.fNear; options["Shadow.far"] = shadow.fFar;
    options["Shadow.depthBias"] = shadow.fDepthBias; options["Shadow.normalBias"] = shadow.fNormalBias;
    options["Shadow.dynamicBakedStrength"] = shadow.fDynamicBakedStrength;
    options["Shadow.PCFRadius"] = shadow.iPCFFilterRadius;
    const auto& source = quality.SourcePostProcess;
    options["SourcePost.enabled"] = source.bEnabled; options["SourcePost.scale"] = source.fToneScale;
    options["SourcePost.range"] = source.fToneRange; options["SourcePost.toe"] = source.fToneToe;
    options["SourcePost.desaturation"] = source.fDesaturation;
    vector3("SourcePost.highlights", source.vHighlights); vector3("SourcePost.midtones", source.vMidtones);
    vector3("SourcePost.shadows", source.vShadows); vector3("SourcePost.colorize", source.vColorize);
    options["SourcePost.LUTCount"] = double(source.LutLayers.size());
    for (size_t i = 0; i < (std::min)(source.LutLayers.size(), size_t(32)); ++i)
    {
        const auto name = "LUT." + std::to_string(i);
        options[name + ".weight"] = source.LutLayers[i].fWeight;
        context.RenderingAssets[name] = source.LutLayers[i].pLut ? source.LutLayers[i].pLut->strAssetId : "neutral";
    }
    const auto material = game.Get_MaterialRenderSettings();
    options["Material.source"] = material.bUseSourceMaterials;
    options["Material.debugView"] = static_cast<uint32_t>(material.eDebugView);
    options["MapPBR.enabled"] = material.MapPBR.bEnabled; options["MapPBR.level"] = material.MapPBR.iLevel;
    vector4("MapPBR.contribution", material.MapPBR.vContributionScale);
    vector4("MapPBR.surface", material.MapPBR.vSurfaceParameters);
    options["MapPBR.cubeDiffuseScale"] = material.MapPBR.fCubeDiffuseScale;
    const auto environment = game.Get_RenderEnvironment();
    vector4("Environment.color", environment.vColor); vector4("Environment.rotationIntensity", environment.vRotationIntensity);
    options["Environment.diffuseIntensity"] = environment.fDiffuseIntensity;
    options["Environment.sourcePBRIndirect"] = environment.bUseSourcePBRIndirect;
    context.RenderingAssets["Environment.cube"] = Capture_PathLabel(std::filesystem::path(environment.strCubePath));
    const auto fog = game.Get_HeightFogSettings();
    options["Fog.enabled"] = fog.bEnabled; options["Fog.sourceExponential"] = fog.bSourceExponential;
    vector4("Fog.color", fog.vColor); vector4("Fog.inscattering", fog.vInscatteringColor); vector4("Fog.lightDirection", fog.vFogLightDirection);
    options["Fog.density"] = fog.fDensity; options["Fog.heightFalloff"] = fog.fHeightFalloff;
    options["Fog.topHeight"] = fog.fTopHeight; options["Fog.startDistance"] = fog.fStartDistance;
    options["Fog.maximumOpacity"] = fog.fMaximumOpacity; options["Fog.driftSpeed"] = fog.fDriftSpeed;
    options["Fog.driftHeightAmplitude"] = fog.fDriftHeightAmplitude; options["Fog.driftDensityAmplitude"] = fog.fDriftDensityAmplitude;
    options["Fog.coveragePercent"] = fog.fCoveragePercent; options["Fog.windDirectionX"] = fog.fWindDirectionX;
    options["Fog.windDirectionZ"] = fog.fWindDirectionZ; options["Fog.windSpeed"] = fog.fWindSpeed;
    options["Fog.patchScale"] = fog.fPatchScale; options["Fog.patchSoftness"] = fog.fPatchSoftness;
    return context;
}

void Client::CProfilerTool::Update_SaveState()
{
    FProfilerCaptureSaveResult saveResult;
    if (!m_Exporter.Poll(saveResult)) return;
    m_strCaptureStatus = saveResult.Succeeded ?
        "Saved " + Capture_PathLabel(saveResult.OutputPath) + "\n" + m_strSavingCoverage : saveResult.Error;
    if (saveResult.Succeeded && Refresh_CaptureFiles())
        for (const auto& file : m_CaptureFiles)
            if (file.FileName == saveResult.OutputPath.filename())
            { m_strSelectedCaptureId = file.StableId; break; }
}

void Client::CProfilerTool::Render(Engine::CProfiler* profiler)
{
    if (!m_bOpen) return;
    ImGui::SetNextWindowSize(ImVec2(1060.f, 720.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("프레임 Profiler###LostArkProfilerToolV1", &m_bOpen))
    {
        ImGui::End(); return;
    }
    if (!profiler)
    {
        ImGui::TextUnformatted("Engine Profiler를 사용할 수 없습니다."); ImGui::End(); return;
    }

    if (!m_bCaptureFilesLoaded) Refresh_CaptureFiles();
    ImGui::SetNextItemWidth(300.f);
    ImGui::InputTextWithHint("저장 이름", "이름 입력 (한글 지원)", m_CaptureName.data(), m_CaptureName.size());
    ImGui::SameLine(); ImGui::TextDisabled("저장할 때마다 새 JSON 파일을 만듭니다.");
#ifdef _DEBUG
    ImGui::TextDisabled("Debug | F7: 창 표시 / 수집(Capture): 계측 시작·정지");
#else
    ImGui::TextDisabled("Release | F7: 창 표시 / 수집(Capture): 계측 시작·정지");
#endif
    ImGui::TextWrapped("비교 순서: 장면 준비 → 초기화 → F7로 창 숨김 → 같은 카메라·동작 재현 → 다시 열어 저장. 창을 닫아도 수집은 계속됩니다. JSON의 카메라·설정 정보는 저장 시점 값입니다.");
    bool enabled = profiler->Is_Enabled();
    if (ImGui::Checkbox("수집 (Capture)", &enabled))
    {
        profiler->Set_Enabled(enabled); m_fLastRefreshTime = -1.0;
    }
    ImGui::SameLine(); ImGui::SetNextItemWidth(100.f);
    if (ImGui::DragInt("분석·표시 프레임 범위", &m_iWindowFrameInput, 1.f, 1,
        static_cast<int>(Engine::CProfiler::MAX_HISTORY_FRAMES), "%d", ImGuiSliderFlags_AlwaysClamp))
        m_fLastRefreshTime = -1.0;
    ImGui::SameLine();
    if (ImGui::Button("초기화"))
    {
        profiler->Reset_History();
        CProfilerCaptureIO::Reset_MovementSamples();
        m_fLastRefreshTime = -1.0;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(m_Exporter.IsSaving());
    if (ImGui::Button("JSON 저장")) Request_Save(*profiler);
    ImGui::EndDisabled(); ImGui::SameLine();
    ImGui::TextDisabled("보관 프레임 %zu / %zu", m_iHistoryFrames, Engine::CProfiler::MAX_HISTORY_FRAMES);

    bool detailed = profiler->Is_DetailedScopesEnabled();
    if (ImGui::Checkbox("draw별 상세 CPU 계측 (추가 비용 발생)", &detailed))
    {
        profiler->Set_DetailedScopesEnabled(detailed);
        profiler->Reset_History();
        CProfilerCaptureIO::Reset_MovementSamples();
        m_fLastRefreshTime = -1.0;
    }
    if (ImGui::Checkbox("JSON 저장 범위 제한", &m_bSaveWindowOnly))
        m_fLastRefreshTime = -1.0;
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_bSaveWindowOnly);
    ImGui::SetNextItemWidth(100.f);
    if (ImGui::DragInt("최근 프레임만 저장", &m_iSaveFrameInput, 1.f, 1,
        static_cast<int>(Engine::CProfiler::MAX_HISTORY_FRAMES), "%d", ImGuiSliderFlags_AlwaysClamp))
        m_fLastRefreshTime = -1.0;
    ImGui::EndDisabled();
    ImGui::TextDisabled("기본은 보관 전체 저장 (최대 %zu). 분석·표시 범위와 별도입니다.", Engine::CProfiler::MAX_HISTORY_FRAMES);
    ImGui::TextDisabled("GPU 결과 대기는 JSON에 pending으로 남으며, 완료된 0ms 측정값이 아닙니다.");

    const double now = ImGui::GetTime();
    if (m_fLastRefreshTime < 0.0 ||
        ((enabled || (m_bLiveValid && m_Live.LatestFrameGpuStatus == Engine::EProfilerGpuFrameStatus::Pending)) &&
            now - m_fLastRefreshTime >= m_fRefreshIntervalSeconds))
    {
        Refresh(*profiler); m_fLastRefreshTime = now;
    }
    ImGui::TextDisabled("저장 범위 %llu프레임 (%llu - %llu) | 보관 %llu | 초기화 이후 퇴출 %llu",
        static_cast<unsigned long long>(m_SaveWindowCoverage.SavedFrames),
        static_cast<unsigned long long>(m_SaveWindowCoverage.FirstSavedFrameNumber),
        static_cast<unsigned long long>(m_SaveWindowCoverage.LastSavedFrameNumber),
        static_cast<unsigned long long>(m_SaveWindowCoverage.RetainedFrames),
        static_cast<unsigned long long>(m_SaveWindowCoverage.EvictedFramesSinceReset));
    if (m_SaveWindowCoverage.ExcludedRetainedFrames != 0)
        ImGui::TextWrapped("선택 범위에서 보관 프레임 %llu개 제외 (최대 간격 %.2f ms). 전체 보관분을 저장하려면 JSON 저장 범위 제한을 끄세요.",
            static_cast<unsigned long long>(m_SaveWindowCoverage.ExcludedRetainedFrames),
            m_SaveWindowCoverage.ExcludedMaxFrameIntervalMs);
    if (m_SaveWindowCoverage.EvictedFramesSinceReset != 0)
        ImGui::TextWrapped("오래된 프레임은 1200개 보관 범위를 벗어나 복구할 수 없습니다. 보관 한도에 도달하기 전에 짧은 구간을 저장하세요.");
    if (ImGui::BeginTable("##FrameSummary", 3, ImGuiTableFlags_SizingStretchSame))
    {
        ImGui::TableNextColumn(); ImGui::TextDisabled("실제 프레임 간격 / FPS (최근)");
        if (m_bLiveValid && m_Live.FrameIntervalMs > 0.0)
            ImGui::Text("%.2f ms / %.1f FPS", m_Live.FrameIntervalMs, 1000.0 / m_Live.FrameIntervalMs);
        else ImGui::TextDisabled("--");
        ImGui::TableNextColumn(); ImGui::TextDisabled("CPU 처리 (구간 평균 / 최대)");
        if (m_iWindowFrames) ImGui::Text("%.2f / %.2f ms", m_fWindowCpuAvgMs, m_fWindowCpuMaxMs);
        else ImGui::TextDisabled("--");
        ImGui::TableNextColumn(); ImGui::TextDisabled("GPU 경과 시간 (유효 평균 / 최대)");
        if (m_iGpuFrameValidFrames > 0)
            ImGui::Text("%.2f / %.2f ms", m_fWindowGpuAvgMs, m_fWindowGpuMaxMs);
        else ImGui::TextDisabled("-- (유효 GPU 결과 없음)");
        ImGui::EndTable();
    }
    if (!enabled) ImGui::TextDisabled("수집 정지 상태입니다. 기존 결과는 유지됩니다.");
    if (m_bLiveValid && (m_Live.TotalDroppedCpuScopes || m_Live.TotalDroppedGpuFrames ||
        m_Live.TotalDroppedGpuScopes || m_Live.TotalDroppedModelAnimationSamples))
        ImGui::TextWrapped("초기화 이후 누락: CPU 구간 %llu / GPU 프레임 %llu / GPU 구간 %llu / 애니메이션 %llu",
            static_cast<unsigned long long>(m_Live.TotalDroppedCpuScopes), static_cast<unsigned long long>(m_Live.TotalDroppedGpuFrames),
            static_cast<unsigned long long>(m_Live.TotalDroppedGpuScopes), static_cast<unsigned long long>(m_Live.TotalDroppedModelAnimationSamples));
    if (m_bLiveValid && m_Live.DroppedCpuScopes)
        ImGui::TextColored(ImVec4(1.f, .65f, .25f, 1.f), "최근 프레임 CPU 구간 %llu개 누락: 전체·자체 비용 분석이 불완전합니다.",
            static_cast<unsigned long long>(m_Live.DroppedCpuScopes));
    if (m_bLiveValid && m_Live.TotalDroppedCpuScopes)
        ImGui::TextWrapped("CPU 구간 누락이 있습니다. 상세 계측을 끄고 초기화한 뒤 병목을 비교하세요. 누락이 있는 선택 구간의 자체 비용은 --로 표시합니다.");
    if (!m_strCaptureStatus.empty()) ImGui::TextWrapped("%s", m_strCaptureStatus.c_str());
    ImGui::Separator();
    ImGui::SetNextItemWidth(280.f);
    ImGui::InputTextWithHint("##ProfilerFilter", "이름·한국어 설명 검색", m_Filter.data(), m_Filter.size());
    ImGui::SameLine(); ImGui::Checkbox("미관측 구간 표시", &m_bShowUnobserved);
    if (ImGui::BeginTabBar("##ProfilerTabs"))
    {
        if (ImGui::BeginTabItem("한 프레임 해석")) { Render_FrameOverview(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("컬링 비용·제외량###CullingCost")) { Render_Culling(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("타임라인###ScopeTimeline")) { Render_Timeline(*profiler); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("병목 후보·다음 실험###BottleneckCandidates")) { Render_Candidates(*profiler); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("RAM·GPU 메모리###MemoryTimeline")) { Render_Memory(*profiler); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("상용 엔진과 비교###CommercialProfilerReference")) { RenderingReferenceGuide::Render(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("프레임 변화###FrameChanges", nullptr,
            m_iComparisonTabRequest == 1 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
        { if (m_iComparisonTabRequest == 1) m_iComparisonTabRequest = 0; Render_FrameChanges(*profiler); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("기준 A/B###BaselineComparison", nullptr,
            m_iComparisonTabRequest == 2 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
        { if (m_iComparisonTabRequest == 2) m_iComparisonTabRequest = 0; Render_Comparison(*profiler); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("CPU 병목")) { Render_Bottlenecks(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("GPU 패스·draw")) { Render_Gpu(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("ImGui 도구")) { Render_ImGui(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("전체 작업량")) { Render_Counters(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("긴 작업")) { Render_LongOperations(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("저장 JSON")) { Render_CaptureFiles(); ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void Client::CProfilerTool::Render_Bottlenecks(bool_t bImGuiOnly)
{
    if (ImGui::Checkbox("자체 비용 내림차순 (끄면 전체 비용)", &m_bCpuSortSelf)) m_bCpuRowsDirty = true;
    ImGui::SameLine();
    if (ImGui::Checkbox("메인 스레드만", &m_bMainThreadOnly)) m_bCpuRowsDirty = true;
    Rebuild_CpuRows(bImGuiOnly);
    if (m_bCpuSortSelf && std::any_of(m_Aggregates.begin(), m_Aggregates.end(), [](const auto& row) { return !row.SelfComplete; }))
        ImGui::TextWrapped("CPU 표본 누락: 자체 시간은 --로 표시하며 전체 비용 순서로 나열합니다.");
    ImGui::TextWrapped("전체는 자식 구간 포함, 자체는 같은 스레드의 계측된 자식 제외입니다. 부모와 자식 시간을 더하지 마세요. 자체 시간에는 미계측 작업·대기도 포함됩니다. 작업 스레드 시간은 CPU 프레임에 더하지 않습니다. 미관측은 0ms가 아닙니다.");
    const double frames = static_cast<double>((std::max)(m_iWindowFrames, size_t{1}));
    if (!ImGui::BeginTable("##CpuSections", 6, TABLE_FLAGS)) return;
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("구간", ImGuiTableColumnFlags_WidthStretch, 3.f);
    for (const char* label : { "스레드", "전체 ms/프레임", "자체 ms/프레임", "단일 호출 최대 ms", "호출/프레임" })
        ImGui::TableSetupColumn(label);
    ImGui::TableHeadersRow();
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(m_VisibleCpuRows.size()));
    while (clipper.Step())
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            const auto& visible = m_VisibleCpuRows[static_cast<size_t>(i)];
            if (!visible.Aggregate) { Unobserved_Row(visible.Name, 6); continue; }
            const auto& row = *visible.Aggregate;
            ImGui::TableNextRow(); ImGui::TableNextColumn(); Scope_Label(visible.Name);
            ImGui::TableNextColumn();
            if (row.ThreadId == m_iMainThreadId) ImGui::TextUnformatted("메인");
            else ImGui::Text("작업 %u", row.ThreadId);
            ImGui::TableNextColumn(); ImGui::Text("%.3f", row.InclusiveMs / frames);
            ImGui::TableNextColumn();
            if (row.SelfComplete) ImGui::Text("%.3f", row.SelfMs / frames);
            else
            {
                ImGui::TextDisabled("--");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("선택 범위에 CPU 구간 누락이 있어 자체 비용을 표시하지 않습니다. 빠진 자식을 부모 비용으로 오인할 수 있습니다. 상세 계측을 끄고 초기화해 다시 수집하세요.");
            }
            ImGui::TableNextColumn(); ImGui::Text("%.3f", row.MaxMs);
            ImGui::TableNextColumn(); ImGui::Text("%.2f", static_cast<double>(row.Calls) / frames);
        }
    ImGui::EndTable();
}

void Client::CProfilerTool::Rebuild_CpuRows(bool_t bImGuiOnly)
{
    if (!m_bCpuRowsDirty && m_strCpuRowFilter == m_Filter.data() &&
        m_bCpuRowsImGuiOnly == bImGuiOnly && m_bCpuRowsShowUnobserved == m_bShowUnobserved)
        return;
    m_VisibleCpuRows.clear();
    const auto matches = [&](const char* name)
    {
        const std::string_view text(name);
        const bool isImGui = text.starts_with("ImGui.") || text.starts_with("Profiler.Panel.") ||
            text.starts_with("Profiler.Capture.") || text.starts_with("EffectTool.") || text.starts_with("EffectSequencer.");
        return (!bImGuiOnly || isImGui) && (Contains_CaseInsensitive(text, m_Filter.data()) ||
            Contains_CaseInsensitive(Scope_Description(text), m_Filter.data()));
    };
    for (const auto& row : m_Aggregates)
        if ((!m_bMainThreadOnly || row.ThreadId == m_iMainThreadId) && matches(Scope_Name(row.NameId)))
            m_VisibleCpuRows.push_back({Scope_Name(row.NameId), &row});
    std::stable_sort(m_VisibleCpuRows.begin(), m_VisibleCpuRows.end(), [&](const auto& left, const auto& right)
    {
        const auto cost = [&](const auto& row) { return m_bCpuSortSelf && row.Aggregate->SelfComplete ? row.Aggregate->SelfMs : row.Aggregate->InclusiveMs; };
        return cost(left) > cost(right);
    });
    if (m_bShowUnobserved)
        for (const char* name : CPU_SCOPE_CATALOG)
            if (matches(name) && std::none_of(m_Aggregates.begin(), m_Aggregates.end(),
                [&](const auto& row) { return (!m_bMainThreadOnly || row.ThreadId == m_iMainThreadId) && std::string_view(name) == Scope_Name(row.NameId); }))
                m_VisibleCpuRows.push_back({name, nullptr});
    m_strCpuRowFilter = m_Filter.data();
    m_bCpuRowsImGuiOnly = bImGuiOnly;
    m_bCpuRowsShowUnobserved = m_bShowUnobserved;
    m_bCpuRowsDirty = false;
}

void Client::CProfilerTool::Render_ImGui()
{
    ImGui::TextWrapped("Build는 도구 코드·위젯 구성, DX11은 업로드·상태 변경·draw 제출입니다. 외부 창 Present에는 OS·드라이버 대기가 포함될 수 있습니다. 같은 장면에서 도구 닫힘·열림·외부 창 상태를 비교하세요.");
    if (m_bLiveValid)
    {
        const auto count = [&](Engine::EProfilerCounter counter)
        { return static_cast<unsigned long long>(m_Live.Counters[static_cast<size_t>(counter)]); };
        ImGui::Text("CPU 프레임 %llu | 창 %llu / 활성 %llu | 뷰포트 %llu | draw %llu / 명령 %llu",
            static_cast<unsigned long long>(m_Live.FrameNumber), count(Engine::EProfilerCounter::ImGuiRenderWindows),
            count(Engine::EProfilerCounter::ImGuiActiveWindows), count(Engine::EProfilerCounter::ImGuiPlatformViewports),
            count(Engine::EProfilerCounter::ImGuiDrawCalls), count(Engine::EProfilerCounter::ImGuiDrawCommands));
        ImGui::Text("외부 창 제출 %llu | 지연 %llu | 오류 %llu | 가림 %llu",
            count(Engine::EProfilerCounter::ImGuiPresentAttempts), count(Engine::EProfilerCounter::ImGuiPresentBusy),
            count(Engine::EProfilerCounter::ImGuiPresentFailures), count(Engine::EProfilerCounter::ImGuiPresentOccluded));
        ImGui::Text("정점 %llu / 인덱스 %llu | 정점+인덱스 업로드 %.1f KiB | 버퍼 확장 %llu | Map 실패 %llu",
            count(Engine::EProfilerCounter::ImGuiVertices), count(Engine::EProfilerCounter::ImGuiIndices),
            (count(Engine::EProfilerCounter::ImGuiVertexUploadBytes) + count(Engine::EProfilerCounter::ImGuiIndexUploadBytes)) / 1024.0,
            count(Engine::EProfilerCounter::ImGuiBufferGrowths), count(Engine::EProfilerCounter::ImGuiBufferMapFailures));
    }
    for (const auto& row : m_GpuAggregates)
        if (std::string_view(Scope_Name(row.NameId)) == "ImGui.RenderDrawData")
            ImGui::Text("GPU draw 구간: 평균 %.3f ms / P95 %.3f ms (완전 프레임 %zu개, 모든 뷰포트)",
                row.InclusiveMs / static_cast<double>((std::max)(m_iGpuValidFrames, size_t{1})), row.P95FrameMs, m_iGpuValidFrames);
    if (ImGui::TreeNode("도구 백엔드 작업량"))
    {
        if (ImGui::BeginTable("##ImGuiWorkload", 2, TABLE_FLAGS, ImVec2(0.f, 160.f)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("작업 항목"); ImGui::TableSetupColumn("최근 CPU 프레임"); ImGui::TableHeadersRow();
            for (size_t i = static_cast<size_t>(Engine::EProfilerCounter::ImGuiDrawLists); i <= static_cast<size_t>(Engine::EProfilerCounter::ImGuiBufferMapFailures); ++i)
            {
                ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(COUNTER_LABELS[i]); ImGui::TableNextColumn();
                if (m_bLiveValid) ImGui::Text("%llu", static_cast<unsigned long long>(m_Live.Counters[i]));
                else ImGui::TextDisabled("--");
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
    Render_Bottlenecks(true);
}

void Client::CProfilerTool::Render_FrameOverview()
{
    if (!m_bLiveValid) { ImGui::TextDisabled("수집을 켜면 프레임 비용을 표시합니다."); return; }
    if (!ImGui::BeginChild("##FrameOverviewScroll")) { ImGui::EndChild(); return; }
    ImGui::TextWrapped("60 FPS 예산은 16.667 ms, 30 FPS는 33.333 ms입니다. CPU와 GPU는 병렬로 진행하므로 시간을 더하지 않습니다. 프레임 간격에는 메시지 처리·프레임 제한·대기·계측 정리도 포함됩니다.");
    ImGui::Text("최근 CPU 프레임 #%llu", static_cast<unsigned long long>(m_Live.FrameNumber));
    if (ImGui::BeginTable("##FrameAccounting", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH))
    {
        ImGui::TableSetupColumn("비용 경계"); ImGui::TableSetupColumn("ms"); ImGui::TableHeadersRow();
        const auto timing = [](const char* label, double ms)
        {
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(label);
            ImGui::TableNextColumn(); ImGui::Text("%.3f", ms);
        };
        timing("현재 CPU 처리 전체", m_Live.CpuFrameMs);
        timing("현재 CPU 계측 구간 합집합 (중복 제외)", (std::max)(0.0, m_Live.CpuFrameMs - m_fUnattributedCpuMs));
        timing("현재 CPU 구간 밖·누락 비용", m_fUnattributedCpuMs);
        if (m_Live.FrameIntervalMs > 0.0)
        {
            timing("이전 CPU 처리 (아래 프레임 간격의 일부)", m_Live.PreviousCpuFrameMs);
            timing("이전 처리 종료 → 현재 시작 (대기·메시지·정리)", m_Live.FrameGapMs);
            timing("이전 시작 → 현재 시작 (실제 프레임 간격)", m_Live.FrameIntervalMs);
        }
        ImGui::EndTable();
    }
    ImGui::TextWrapped("구간 밖 비용은 특정 병목으로 단정하지 않습니다. CPU 구간 안에서도 자체 시간에는 미계측 자식과 스케줄링 대기가 포함됩니다. 상세 계측이 꺼진 작업과 누락 표본은 0ms가 아닙니다.");
    const auto count = [&](Engine::EProfilerCounter c) { return static_cast<unsigned long long>(m_Live.Counters[static_cast<size_t>(c)]); };
    ImGui::Separator();
    ImGui::Text("계측된 Engine draw %llu + ImGui draw %llu", count(Engine::EProfilerCounter::DrawCalls), count(Engine::EProfilerCounter::ImGuiDrawCalls));
    ImGui::Text("인스턴싱 draw %llu | 인스턴싱 제출 수 %llu | 인덱스 %llu", count(Engine::EProfilerCounter::InstancedDrawCalls), count(Engine::EProfilerCounter::Instances), count(Engine::EProfilerCounter::Indices));
    ImGui::Text("메시 draw %llu | 메시 인스턴스 %llu | 메시 인덱스 %llu | 고유 CMesh %llu",
        count(Engine::EProfilerCounter::MeshDrawCalls), count(Engine::EProfilerCounter::MeshInstances),
        count(Engine::EProfilerCounter::MeshIndices), count(Engine::EProfilerCounter::UniqueMeshes));
    ImGui::Text("동일 모델 결합 %llu -> %llu draw | 조명 bank %llu -> %llu draw",
        count(Engine::EProfilerCounter::MapIdenticalInstanceSourceDraws), count(Engine::EProfilerCounter::MapIdenticalInstanceDraws),
        count(Engine::EProfilerCounter::MapLightingBankSourceDraws), count(Engine::EProfilerCounter::MapLightingBankDraws));
    ImGui::TextWrapped("결합 전 수는 이번 프레임에 개별 제출했을 경우의 source draw입니다. 이전 제품 대비 감소율이나 전체 장면의 절감량을 의미하지 않습니다.");
    ImGui::TextDisabled("컬링 설정·CPU 비용·제외량·캐시는 '컬링 비용·제외량' 탭에서 함께 확인합니다.");

    if (count(Engine::EProfilerCounter::DroppedMeshSamples))
        ImGui::TextColored(ImVec4(1.f, .65f, .25f, 1.f), "고유 메시 표본 %llu개 누락: 고유 수는 하한입니다.", count(Engine::EProfilerCounter::DroppedMeshSamples));
    ImGui::TextWrapped("draw는 API 제출 횟수, 인스턴싱 제출 수는 instanced draw만의 반복 개수, 고유 CMesh는 서로 다른 CPU 메시 객체 수입니다. 인덱스는 인스턴스 수를 반영하며 고유 정점·오브젝트·화면 삼각형 수와 다릅니다. 그림자·초상 재제출도 포함합니다. DirectXTK 글자·디버그 도형 내부 draw는 위 합계에 미포함입니다. 간접 draw와 그 인덱스 상한은 생산자가 없는 미계측 예약 항목입니다.");
    ImGui::Separator();
    ImGui::Text("광원 레코드 제출 %llu | draw %llu | 업로드 %.1f KiB | 컬링 제외 %llu / %llu",
        count(Engine::EProfilerCounter::LightRecords), count(Engine::EProfilerCounter::LightDrawCalls),
        count(Engine::EProfilerCounter::LightUploadBytes) / 1024.0,
        count(Engine::EProfilerCounter::LightCullingRejected), count(Engine::EProfilerCounter::LightCullingCandidates));
    ImGui::TextWrapped("빛의 비용: CPU의 광원 컬링·구성·업로드, GPU의 그림자 생성·직접광·SSAO·간접광 합성을 각각 읽습니다. 수광 패스·초상마다 같은 광원을 재제출하므로 레코드는 유일 광원 수가 아닙니다. 픽셀 셰이더 호출 수는 수학 연산 수가 아닙니다. 투명 재질의 빛 계산은 Render.Blend에 포함됩니다.");
    if (ImGui::BeginTable("##LightingCost", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH))
    {
        ImGui::TableSetupColumn("빛 관련 GPU 구간"); ImGui::TableSetupColumn("전체 ms/프레임");
        ImGui::TableSetupColumn("자체 ms/프레임"); ImGui::TableHeadersRow();
        const double frames = static_cast<double>((std::max)(size_t{1}, m_iGpuValidFrames));
        for (const auto& row : m_GpuAggregates)
        {
            const std::string_view name(Scope_Name(row.NameId));
            if (!name.starts_with("Render.Lights") && !name.starts_with("Render.Shadow") &&
                name != "Render.SSAO" && name != "Render.Combined") continue;
            ImGui::TableNextRow(); ImGui::TableNextColumn(); Scope_Label(Scope_Name(row.NameId));
            ImGui::TableNextColumn(); ImGui::Text("%.3f", row.InclusiveMs / frames);
            ImGui::TableNextColumn(); ImGui::Text("%.3f", row.SelfMs / frames);
        }
        ImGui::EndTable();
    }
    if (!m_iGpuValidFrames) ImGui::TextDisabled("빛 GPU 구간은 아직 유효한 완료 결과가 없습니다.");
    ImGui::TextWrapped("이 표의 부모·자식 행은 중첩됩니다. 전체 값을 더하지 마세요. GPU timestamp는 명령 공급 대기를 포함한 경과 시간이며 GPU 점유율이 아닙니다. 개별 광원의 ALU·메모리·캐시 비용과 자원별 VRAM 귀속, Server 연산은 미계측입니다. 프로세스 GPU segment 사용량은 RAM·GPU 메모리 탭에서 확인합니다.");
    ImGui::EndChild();
}

void Client::CProfilerTool::Render_Culling()
{
    if (!ImGui::BeginChild("##CullingCostScroll")) { ImGui::EndChild(); return; }
    auto& game = Engine::CGameInstance::Get();
    auto visibility = game.Get_MapVisibilitySettings();
    auto optimization = game.Get_RenderOptimizationSettings();
    const bool bern = game.Get_CurrentLevelID() == ETOUI(LEVEL::BERN);
    ImGui::TextDisabled("베른 컷신 시야각은 F1 → Bern Entrance Camera에서 조절합니다.");
    if (ImGui::Checkbox("프러스텀 컬링 (세션)", &optimization.FrustumEnabled))
        (void)game.Apply_RenderOptimizationSettings(optimization);
    bool changedVisibility = ImGui::Checkbox("Bern occlusion culling", &visibility.OcclusionEnabled);
    changedVisibility |= ImGui::Checkbox("Bern parallel visibility preparation", &visibility.ParallelPreparationEnabled);
    changedVisibility |= ImGui::Checkbox("베른 소품 거리·화면 크기 컬링", &visibility.DistanceEnabled);
    changedVisibility |= ImGui::SliderFloat("소품 표시 거리 배율", &visibility.DistanceScale, .25f, 4.f, "%.2f");
    changedVisibility |= ImGui::SliderFloat("기준 최대 지름 (px, 배율 1)", &visibility.DistanceMaxPixels, 4.f, 128.f, "%.0f");
    ImGui::Text("실효 최대 지름 %.1fpx / 처음 숨김 %.1fpx 이하",
        visibility.Get_DistancePixelLimit(), visibility.Get_DistancePixelLimit() * .9f);
    if (changedVisibility) (void)game.Apply_MapVisibilitySettings(visibility);
    const bool occlusionAllowed = bern && visibility.OcclusionEnabled && !game.Is_SceneEnvironmentReplaced();
    ImGui::Text("현재 CPU 가림 옵션: %s / 현재 장면 정책: %s",
        visibility.OcclusionEnabled ? "ON" : "OFF", occlusionAllowed ? "허용" : "실행 제외");
    if (!bern) ImGui::TextWrapped("Bern 외 장면에서는 CPU 가림 패스를 실행하지 않습니다. Character Select 무비도 이 범위에 포함됩니다.");
    else if (game.Is_SceneEnvironmentReplaced()) ImGui::TextWrapped("장면 배경 교체 중에는 CPU 가림 패스를 실행하지 않습니다.");
    ImGui::TextWrapped("위 값은 현재 세션 설정입니다. 허용 상태라도 후보·가림 geometry가 없으면 실제 검사를 생략합니다. 아래 기록은 수집 당시 값이며 설정 변경만으로 갱신되지 않습니다.");
    ImGui::TextWrapped("거리 컬링은 반경 3m 이하 소품에만 적용합니다. 거리 배율을 줄이면 거리 제한을 줄이고 실효 픽셀 한계를 늘립니다(기준 지름 / 배율, 4~128px). 거리와 화면 크기 조건을 모두 만족해야 숨기며 큰 건물·지형은 이 설정으로 제거하지 않습니다.");
    if (ImGui::Button("프레임 변화에서 비교")) m_iComparisonTabRequest = 1;
    ImGui::SameLine();
    if (ImGui::Button("기준 A/B에서 비교")) m_iComparisonTabRequest = 2;
    ImGui::TextWrapped("같은 장면·카메라에서 OFF 수집창을 A로 보관 → 설정 변경·초기화 → ON 수집창을 B로 보관합니다. 전원 모드·해상도·상세 계측도 맞추고, 카메라 정지 캐시와 이동 중 재계산을 따로 비교하세요. 제외량만으로 FPS 개선을 확정하지 않습니다.");
    ImGui::Separator();
    ImGui::Text("CPU 비용: 분석 범위 %zu프레임의 평균 (ms/프레임)", m_iWindowFrames);
    ImGui::TextWrapped("전체 시간은 자식을 포함합니다. 가림 전체와 Rasterize/Test, Prepare와 Dispatch/Join/Worker를 더하지 마세요. FinalCameraSubmission은 프러스텀 검사 외 준비·제출도 포함하며 순수 프러스텀 비용이 아닙니다. Worker 시간도 메인 시간과 합산하지 않습니다.");
    if (ImGui::BeginTable("##CullingCpuCosts", 5,
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Resizable))
    {
        ImGui::TableSetupColumn("CPU 구간", ImGuiTableColumnFlags_WidthStretch, 3.f);
        for (const char* label : {"스레드", "전체 ms/프레임", "자체 ms/프레임", "호출/프레임"})
            ImGui::TableSetupColumn(label);
        ImGui::TableHeadersRow();
        const double frames = static_cast<double>((std::max)(m_iWindowFrames, size_t{1}));
        for (const char* name : {"Render.FinalCameraSubmission", "Map.Visibility.Prepare",
            "Map.Visibility.Dispatch", "Map.Visibility.Join", "Map.Visibility.Worker",
            "Map.Batch.CullAndPack", "Render.MapOcclusion", "Render.MapOcclusion.Rasterize", "Render.MapOcclusion.Test"})
        {
            bool observed = false;
            for (const auto& row : m_Aggregates)
            {
                if (!m_iWindowFrames || !row.Calls || std::string_view(Scope_Name(row.NameId)) != name) continue;
                observed = true;
                ImGui::TableNextRow(); ImGui::TableNextColumn(); Scope_Label(name);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(Thread_Label(row.ThreadId).c_str());
                ImGui::TableNextColumn(); ImGui::Text("%.3f", row.InclusiveMs / frames);
                ImGui::TableNextColumn();
                if (row.SelfComplete) ImGui::Text("%.3f", row.SelfMs / frames);
                else ImGui::TextDisabled("-- (누락)");
                ImGui::TableNextColumn(); ImGui::Text("%.2f", double(row.Calls) / frames);
            }
            if (!observed) Unobserved_Row(name, 5);
        }
        ImGui::EndTable();
    }
    ImGui::TextDisabled("미관측은 비활성·실행 생략·상세 OFF·표본 누락 등을 포함하며 0ms 성공 측정이 아닙니다.");
    ImGui::Separator();
    if (!m_bLiveValid)
    {
        ImGui::TextDisabled("최근 완료 프레임이 없습니다. 수집을 켜면 제외량과 캐시 기록을 표시합니다.");
        ImGui::EndChild(); return;
    }
    ImGui::Text("최근 CPU 완료 프레임 #%llu의 작업량 (위 구간 평균과 별도)",
        static_cast<unsigned long long>(m_Live.FrameNumber));
    const auto& visibilityWork = m_Live.CpuWork[static_cast<size_t>(Engine::EProfilerWork::MapBatchVisibility)];
    if (visibilityWork.Calls)
        ImGui::Text("맵 가시성 고정 계측: %.3f ms / %llu회 (상세 OFF에서도 기록)",
            visibilityWork.CpuMs, static_cast<unsigned long long>(visibilityWork.Calls));
    else ImGui::TextDisabled("맵 가시성 고정 계측: 미관측 (호출 없음)");
    ImGui::TextWrapped("고정 계측도 자식을 포함합니다. 위 scope 비용에 더하지 마세요. 아래 0은 이 프레임 카운터 증가 없음이며 기능 전체의 실행·성공을 증명하지 않습니다.");
    const auto count = [&](Engine::EProfilerCounter counter)
    { return static_cast<unsigned long long>(m_Live.Counters[static_cast<size_t>(counter)]); };
    using Counter = Engine::EProfilerCounter;
    if (ImGui::BeginTable("##CullingWorkCounts", 3,
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Resizable))
    {
        ImGui::TableSetupColumn("컬링 단계");
        ImGui::TableSetupColumn("최근 프레임 입력·작업", ImGuiTableColumnFlags_WidthStretch, 2.f);
        ImGui::TableSetupColumn("최근 프레임 결과·재사용", ImGuiTableColumnFlags_WidthStretch, 2.f);
        ImGui::TableHeadersRow();
        const auto row = [](const char* label)
        { ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(label); ImGui::TableNextColumn(); };
        row("맵 세부 가시성");
        ImGui::Text("인스턴스 후보 %llu", count(Counter::MapCullingCandidates)); ImGui::TableNextColumn();
        ImGui::Text("통과 %llu / bounds 제외 %llu batch", count(Counter::MapCullingVisible), count(Counter::MapBatchBoundsRejected));
        row("맵 가시성 캐시");
        ImGui::Text("재계산 %llu회", count(Counter::MapBatchVisibilityRebuilds)); ImGui::TableNextColumn();
        ImGui::Text("재사용 %llu회", count(Counter::MapBatchVisibilityCacheHits));
        row("가림 판정 (batch)");
        ImGui::Text("후보 %llu / 재검사 %llu", count(Counter::MapOcclusionCandidates), count(Counter::MapOcclusionTested)); ImGui::TableNextColumn();
        ImGui::Text("제외 %llu / 캐시 재사용 %llu회", count(Counter::MapOcclusionRejectedBatches), count(Counter::MapOcclusionCacheHits));
        row("가림으로 제외한 제출");
        ImGui::Text("source mesh draw %llu", count(Counter::MapOcclusionSourceDraws)); ImGui::TableNextColumn();
        ImGui::Text("제출 인덱스 %llu", count(Counter::MapOcclusionRejectedIndices));
        row("가림 깊이 생성");
        ImGui::Text("배치 %llu", count(Counter::MapOcclusionOccluders)); ImGui::TableNextColumn();
        ImGui::Text("CPU 래스터 삼각형 %llu", count(Counter::MapOcclusionRasterizedTriangles));
        row("거리·화면 크기");
        ImGui::Text("재검사 %llu instances", count(Counter::MapDistanceTestedInstances)); ImGui::TableNextColumn();
        ImGui::Text("제외 %llu / 원본 인덱스 %llu", count(Counter::MapDistanceRejectedInstances), count(Counter::MapDistanceRejectedIndices));
        row("맵 CPU 준비");
        ImGui::Text("%llu batch / %llu 작업", count(Counter::MapVisibilityPreparedBatches), count(Counter::MapVisibilityCpuJobs)); ImGui::TableNextColumn();
        ImGui::Text("caller %llu / worker %llu", count(Counter::MapVisibilityCallerJobs), count(Counter::MapVisibilityWorkerJobs));
        row("전체 렌더 제출");
        ImGui::Text("Engine draw %llu", count(Counter::DrawCalls)); ImGui::TableNextColumn();
        ImGui::Text("메시 제출 인덱스 %llu", count(Counter::MeshIndices));
        ImGui::EndTable();
    }
    ImGui::TextWrapped("맵 세부 가시성은 거리·프러스텀을 함께 검사하며 bounds 조기 제외는 세부 후보와 별도입니다. 거리·가림 제외에는 캐시 결과도 포함됩니다. source draw는 인스턴싱 결합 전 수이고, 거리 인덱스는 원본 LOD0 기준이므로 전체 제출·다른 제외량과 합산하지 마세요. 전체 렌더 제출에는 그림자·초상도 포함됩니다. 순수 프러스텀 비용·제외량은 별도 계측이 없습니다.");
    ImGui::EndChild();
}

void Client::CProfilerTool::Render_Gpu()
{
    ImGui::TextWrapped("GPU 결과는 제출했던 원래 프레임에 연결됩니다. 전체는 자식 포함, 자체는 계측된 자식 구간 제외입니다. CPU와 GPU·부모와 자식을 더하지 마세요. 불완전 GPU 프레임은 아래 평균·P95 분모에서 제외합니다.");
    ImGui::Text("완전 GPU 프레임 %zu / 구간 %zu | 제외된 부분 프레임 %zu", m_iGpuValidFrames, m_iWindowFrames, m_iGpuPartialFrames);
    if (m_bLiveValid)
    {
        ImGui::Text("최근 CPU #%llu 의 GPU 상태: %s", static_cast<unsigned long long>(m_Live.FrameNumber), Gpu_Status(m_Live.LatestFrameGpuStatus));
        if (m_Live.GpuValid) ImGui::Text("마지막 유효 GPU #%llu | 읽기 지연 %u프레임 | 누락 구간 %u",
            static_cast<unsigned long long>(m_Live.GpuFrameNumber), m_Live.GpuLatencyFrames, m_Live.DroppedGpuScopes);
        if (!m_Live.GpuScopesSupported) ImGui::TextDisabled("패스 query 미지원: 전체 프레임 계측만 가능할 수 있습니다.");
    }
    ImGui::Checkbox("GPU 자체 비용 내림차순 (끄면 전체 비용)", &m_bGpuSortSelf);
    std::stable_sort(m_GpuAggregates.begin(), m_GpuAggregates.end(), [&](const auto& left, const auto& right)
    { return (m_bGpuSortSelf ? left.SelfMs : left.InclusiveMs) > (m_bGpuSortSelf ? right.SelfMs : right.InclusiveMs); });
    const auto matches = [&](const char* name) { return Contains_CaseInsensitive(name, m_Filter.data()) || Contains_CaseInsensitive(Scope_Description(name), m_Filter.data()); };
    if (m_bLiveValid && m_Live.GpuValid && ImGui::TreeNode("마지막 유효 GPU 프레임 시간 순서"))
    {
        if (ImGui::BeginTable("##GpuIntervals", 5, TABLE_FLAGS, ImVec2(0, 180.f)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("패스", ImGuiTableColumnFlags_WidthStretch, 3.f);
            for (const char* label : {"시작 ms", "종료 ms", "전체 ms", "자체 ms"}) ImGui::TableSetupColumn(label);
            ImGui::TableHeadersRow();
            for (const auto& row : m_Live.GpuScopes)
            {
                if (!matches(Scope_Name(row.NameId))) continue;
                ImGui::TableNextRow(); ImGui::TableNextColumn();
                ImGui::Text("%*s%s | %s", static_cast<int>(row.Depth * 2), "", Scope_Description(Scope_Name(row.NameId)), Scope_Name(row.NameId));
                for (double value : {row.BeginMs, row.EndMs, row.DurationMs, row.SelfMs})
                { ImGui::TableNextColumn(); ImGui::Text("%.3f", value); }
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
    const double frames = static_cast<double>((std::max)(m_iGpuValidFrames, size_t{1}));
    if (!ImGui::BeginTabBar("##GpuDetailTabs")) return;
    if (ImGui::BeginTabItem("시간·셰이더 처리량"))
    {
        ImGui::TextWrapped("PS/VS는 셰이더 호출 수, IA는 GPU 입력 정점·primitive 수입니다. 고유 메시 정점이나 정확한 광원 연산 수가 아닙니다. 선택한 패스만 pipeline 계측하며 --는 미지원·미계측·불완전입니다.");
        if (ImGui::BeginTable("##GpuPasses", 10, TABLE_FLAGS | ImGuiTableFlags_ScrollX, ImVec2(0, 0), 1650.f))
        {
            ImGui::TableSetupScrollFreeze(1, 1);
            ImGui::TableSetupColumn("패스", ImGuiTableColumnFlags_WidthFixed, 380.f);
            for (const char* label : {"자체 ms/프레임", "전체 ms/프레임", "자체 P95 ms", "전체 P95 ms", "전체 최대 ms", "PS/프레임", "VS/프레임", "IA 정점/프레임", "IA 도형/프레임"}) ImGui::TableSetupColumn(label);
            ImGui::TableHeadersRow();
            for (const auto& row : m_GpuAggregates)
            {
                if (!matches(Scope_Name(row.NameId))) continue;
                ImGui::TableNextRow(); ImGui::TableNextColumn(); Scope_Label(Scope_Name(row.NameId));
                for (double value : {row.SelfMs / frames, row.InclusiveMs / frames, row.SelfP95FrameMs, row.P95FrameMs, row.MaxFrameMs})
                { ImGui::TableNextColumn(); ImGui::Text("%.3f", value); }
                for (uint64_t value : {row.PSInvocations, row.VSInvocations, row.IAVertices, row.IAPrimitives})
                {
                    ImGui::TableNextColumn();
                    if (row.PipelineSamples && row.PipelineSamples == row.Calls) ImGui::Text("%.0f", value / frames);
                    else ImGui::TextDisabled("--");
                }
            }
            if (m_bShowUnobserved)
                for (const char* name : GPU_SCOPE_CATALOG)
                    if (matches(name) && std::none_of(m_GpuAggregates.begin(), m_GpuAggregates.end(), [&](const auto& row) { return name == std::string_view(Scope_Name(row.NameId)); }))
                        Unobserved_Row(name, 10);
            ImGui::EndTable();
        }
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("draw·메시·인덱스"))
    {
        ImGui::TextWrapped("아래는 같은 유효 GPU 프레임 구간에서 CPU가 제출한 Engine draw 누계입니다. GPU가 최종 표시한 수가 아닙니다. 부모 값은 자식을 포함합니다. ImGui·DirectXTK는 이 패스 draw 표에 미포함이며 ImGui는 전체 작업량에서 별도 확인합니다. 간접 draw·인덱스 상한은 생산자가 없는 미계측 예약 항목입니다.");
        if (ImGui::BeginTable("##GpuDrawWork", 9, TABLE_FLAGS | ImGuiTableFlags_ScrollX, ImVec2(0, 0), 1540.f))
        {
            ImGui::TableSetupScrollFreeze(1, 1);
            ImGui::TableSetupColumn("패스", ImGuiTableColumnFlags_WidthFixed, 380.f);
            for (const char* label : {"전체 ms/프레임", "draw/프레임", "인스턴싱 draw", "인스턴싱 제출 수", "직접 제출 인덱스", "메시 draw", "메시 인스턴스", "메시 인덱스"}) ImGui::TableSetupColumn(label);
            ImGui::TableHeadersRow();
            for (const auto& row : m_GpuAggregates)
            {
                if (!matches(Scope_Name(row.NameId))) continue;
                ImGui::TableNextRow(); ImGui::TableNextColumn(); Scope_Label(Scope_Name(row.NameId));
                ImGui::TableNextColumn(); ImGui::Text("%.3f", row.InclusiveMs / frames);
                for (uint64_t value : {row.Draw.DrawCalls, row.Draw.InstancedDrawCalls, row.Draw.Instances, row.Draw.Indices, row.Draw.MeshDrawCalls, row.Draw.MeshInstances, row.Draw.MeshIndices})
                { ImGui::TableNextColumn(); ImGui::Text("%.1f", value / frames); }
            }
            ImGui::EndTable();
        }
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
}

void Client::CProfilerTool::Render_Counters() const
{
    if (!m_bLiveValid) { ImGui::TextDisabled("수집된 프레임이 없습니다."); return; }
    if (!ImGui::BeginChild("##WorkloadScroll")) { ImGui::EndChild(); return; }
    const auto& animation = m_Live.Animation;
    ImGui::Text("CPU 프레임 %llu", static_cast<unsigned long long>(m_Live.FrameNumber));
    ImGui::TextWrapped("아래 고정 CPU 작업은 상세 모드와 관계없이 메인 스레드 호출을 누적합니다. 시간은 자식을 포함하므로 행끼리 더하지 마세요. 호출 수는 시도 횟수이며 화면 표시 수가 아닙니다.");
    if (ImGui::BeginTable("##CpuWorkCategories", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH))
    {
        ImGui::TableSetupColumn("CPU 작업");
        ImGui::TableSetupColumn("최근 프레임 호출");
        ImGui::TableSetupColumn("전체 ms");
        ImGui::TableHeadersRow();
        for (size_t i = 0; i < m_Live.CpuWork.size(); ++i)
        {
            const auto& work = m_Live.CpuWork[i];
            ImGui::TableNextRow(); ImGui::TableNextColumn();
            Scope_Label(Engine::CProfiler::Get_WorkName(static_cast<Engine::EProfilerWork>(i)));
            ImGui::TableNextColumn(); ImGui::Text("%llu", static_cast<unsigned long long>(work.Calls));
            ImGui::TableNextColumn(); ImGui::Text("%.3f", work.CpuMs);
        }
        ImGui::EndTable();
    }
    ImGui::Text("애니메이션 평가 %.3f ms | 호출 %llu / 모델 %llu", animation.CpuMs,
        static_cast<unsigned long long>(animation.UpdateCalls), static_cast<unsigned long long>(animation.UpdatedModels));
    ImGui::Text("갱신 후 미제출: 모델 %llu / %.3f ms", static_cast<unsigned long long>(animation.NotSubmittedUpdatedModels), animation.NotSubmittedCpuMs);
    if (animation.DroppedSamples) ImGui::Text("애니메이션 표본 상한: 이 프레임 %llu개 누락", static_cast<unsigned long long>(animation.DroppedSamples));
    ImGui::TextWrapped("미제출은 해당 프레임에 성공한 모델 draw가 없다는 뜻입니다. 숨김·도구 preview·화면 밖 모델 등을 포함하므로 프러스텀 컬링 수와 다릅니다. 이력 표본은 CPU 표에서 별도로 확인합니다.");
    if (m_Live.GpuValid)
        ImGui::Text("GPU 프레임 %llu: IA 입력 정점 %llu | VS 호출 %llu | PS 호출 %llu | 입력 primitive %llu",
            static_cast<unsigned long long>(m_Live.GpuFrameNumber),
            static_cast<unsigned long long>(m_Live.Pipeline.IAVertices), static_cast<unsigned long long>(m_Live.Pipeline.VSInvocations),
            static_cast<unsigned long long>(m_Live.Pipeline.PSInvocations), static_cast<unsigned long long>(m_Live.Pipeline.IAPrimitives));
    ImGui::TextWrapped("경로 탐색은 클라이언트 측정값입니다. Server 연산은 별도입니다. 텍스처 요청은 LoadSharedTexture 시도(실패 포함), 경로 적중은 weak cache 재사용, 새 SRV는 생성 성공 건수이며 상주 개수가 아닙니다. worker 요청과 생성 완료는 다른 프레임일 수 있어 당프레임 hit/request를 효율로 단정하지 않습니다. content cache·추정 GPU bytes·간접 draw는 미계측 예약 항목입니다.");
    ImGui::TextWrapped("맵 가시성 캐시 적중은 이전 카메라 결과 재사용입니다. 재계산 후보 0은 컬링 비활성화를 뜻하지 않습니다. 저작 숨김 NPC는 프러스텀과 별개이며, bounds 무효화 이펙트는 root 변경으로 기존 bounds가 무효화된 항목입니다.");
    if (ImGui::BeginTable("##WorkCounters", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH))
    {
        ImGui::TableSetupColumn("작업 항목"); ImGui::TableSetupColumn("최근 CPU 프레임"); ImGui::TableHeadersRow();
        for (size_t i = 0; i < std::size(COUNTER_LABELS); ++i)
        {
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(COUNTER_LABELS[i]);
            ImGui::TableNextColumn();
            if (i == static_cast<size_t>(Engine::EProfilerCounter::TextureContentHits) ||
                i == static_cast<size_t>(Engine::EProfilerCounter::TextureEstimatedGpuBytes) ||
                i == static_cast<size_t>(Engine::EProfilerCounter::IndirectDrawCalls) ||
                i == static_cast<size_t>(Engine::EProfilerCounter::IndirectIndexUpperBound))
                ImGui::TextDisabled("미계측");
            else ImGui::Text("%llu", static_cast<unsigned long long>(m_Live.Counters[i]));
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
}

void Client::CProfilerTool::Render_LongOperations()
{
    ImGui::TextWrapped("%.0f ms 이상 완료된 호출을 최신순으로 표시합니다. 여러 프레임에 걸친 worker 작업은 종료 프레임에 기록하며 메인 스레드 시간에 합산하지 않습니다.", Engine::CProfiler::LONG_OPERATION_THRESHOLD_MS);
    if (!ImGui::BeginTable("##LongOperations", 4, TABLE_FLAGS)) return;
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("구간", ImGuiTableColumnFlags_WidthStretch, 3.f);
    for (const char* label : {"스레드", "소요 ms", "완료 프레임"}) ImGui::TableSetupColumn(label);
    ImGui::TableHeadersRow();
    for (auto row = m_LongOperations.rbegin(); row != m_LongOperations.rend(); ++row)
    {
        if (!Contains_CaseInsensitive(Scope_Name(row->NameId), m_Filter.data()) &&
            !Contains_CaseInsensitive(Scope_Description(Scope_Name(row->NameId)), m_Filter.data())) continue;
        ImGui::TableNextRow(); ImGui::TableNextColumn(); Scope_Label(Scope_Name(row->NameId));
        ImGui::TableNextColumn(); ImGui::TextUnformatted(Thread_Label(row->ThreadId).c_str());
        ImGui::TableNextColumn(); ImGui::Text("%.3f", row->DurationMs);
        ImGui::TableNextColumn(); ImGui::Text("%llu", static_cast<unsigned long long>(row->FrameNumber));
    }
    ImGui::EndTable();
}


bool_t Client::CProfilerTool::Refresh_CaptureFiles()
{
    m_bCaptureFilesLoaded = true;
    if (!CProfilerCaptureIO::List_JsonFiles(CProfilerCaptureIO::Get_CaptureDirectory(),
        m_CaptureFiles, &m_strCaptureFilesStatus)) return false;
    if (std::none_of(m_CaptureFiles.begin(), m_CaptureFiles.end(),
        [&](const auto& file) { return file.StableId == m_strSelectedCaptureId; }))
        m_strSelectedCaptureId.clear();
    return true;
}

void Client::CProfilerTool::Render_CaptureFiles()
{
    if (ImGui::Button("목록 새로고침")) Refresh_CaptureFiles();
    ImGui::SameLine();
    const auto selected = std::find_if(m_CaptureFiles.begin(), m_CaptureFiles.end(),
        [&](const auto& file) { return file.StableId == m_strSelectedCaptureId; });
    ImGui::BeginDisabled(selected == m_CaptureFiles.end() || m_Exporter.IsSaving());
    if (ImGui::Button("선택 JSON 삭제") && selected != m_CaptureFiles.end())
    {
        const FProfilerCaptureFile file = *selected;
        std::string error;
        if (CProfilerCaptureIO::Delete_JsonFile(CProfilerCaptureIO::Get_CaptureDirectory(), file, &error))
        {
            m_strSelectedCaptureId.clear();
            std::erase_if(m_CaptureFiles, [&](const auto& row) { return row.StableId == file.StableId; });
            const bool refreshed = Refresh_CaptureFiles();
            m_strCaptureFilesStatus = "Deleted " + file.DisplayName +
                (refreshed ? std::string{} : ". Refresh failed: " + m_strCaptureFilesStatus);
        }
        else m_strCaptureFilesStatus = error;
    }
    ImGui::EndDisabled();
    ImGui::SameLine(); ImGui::TextDisabled("파일 %zu개", m_CaptureFiles.size());
    ImGui::TextWrapped("%s", Capture_PathLabel(CProfilerCaptureIO::Get_CaptureDirectory()).c_str());
    if (!m_strCaptureFilesStatus.empty()) ImGui::TextWrapped("%s", m_strCaptureFilesStatus.c_str());
    if (!ImGui::BeginTable("##SavedCaptures", 3, TABLE_FLAGS)) return;
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("파일", ImGuiTableColumnFlags_WidthStretch, 5.f);
    ImGui::TableSetupColumn("크기 (KiB)"); ImGui::TableSetupColumn("수정 시각"); ImGui::TableHeadersRow();
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(m_CaptureFiles.size()));
    while (clipper.Step())
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            const auto& file = m_CaptureFiles[static_cast<size_t>(i)];
            ImGui::TableNextRow(); ImGui::TableNextColumn();
            ImGui::PushID(file.StableId.c_str());
            const ImVec2 labelPosition = ImGui::GetCursorScreenPos();
            if (ImGui::Selectable("##CaptureFile", file.StableId == m_strSelectedCaptureId,
                ImGuiSelectableFlags_SpanAllColumns, ImVec2(0.f, ImGui::GetTextLineHeight())))
                m_strSelectedCaptureId = file.StableId;
            ImGui::GetWindowDrawList()->AddText(labelPosition, ImGui::GetColorU32(ImGuiCol_Text), file.DisplayName.c_str());
            ImGui::PopID();
            ImGui::TableNextColumn(); ImGui::Text("%.1f", file.SizeBytes / 1024.0);
            ImGui::TableNextColumn();
            const FILETIME utc{static_cast<DWORD>(file.LastWriteTicks), static_cast<DWORD>(file.LastWriteTicks >> 32u)};
            FILETIME local{}; SYSTEMTIME time{};
            if (FileTimeToLocalFileTime(&utc, &local) && FileTimeToSystemTime(&local, &time))
                ImGui::Text("%04u-%02u-%02u %02u:%02u:%02u", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
            else ImGui::TextDisabled("--");
        }
    ImGui::EndTable();
}

namespace
{
    using FComparisonMeans = std::map<std::string, Client::FProfilerComparisonMean>;

    std::string ComparisonMetricLabel(const std::string& key)
    {
        if (key == "frame::cpu") return "CPU 프레임 전체";
        if (key == "frame::interval") return "프레임 시작 간격 (이전 시작 → 현재 시작)";
        if (key == "frame::gap") return "이전 CPU 종료 → 현재 시작 대기·간격";
        if (key == "frame::previousCpu") return "간격을 구성한 이전 CPU 프레임";
        if (key == "frame::gpu") return "같은 프레임 번호의 GPU 전체";
        const auto split = key.find("::");
        const auto name = split == std::string::npos ? key : key.substr(split + 2);
        if (key.starts_with("counter::"))
            for (size_t i = 0; i < std::size(COUNTER_LABELS); ++i)
                if (name == Client::CProfilerCaptureIO::Counter_Name(i)) return COUNTER_LABELS[i];
        if (key.starts_with("animation."))
        {
            static const std::map<std::string, std::string> labels = {
                {"cpuMs", "애니메이션 평가 CPU 전체"}, {"notSubmittedCpuMs", "갱신 후 미제출 애니메이션 CPU"},
                {"updateCalls", "애니메이션 평가 호출"}, {"updatedModels", "애니메이션 갱신 모델"},
                {"submittedUpdatedModels", "갱신 후 제출 모델"}, {"notSubmittedUpdatedModels", "갱신 후 미제출 모델"},
                {"droppedSamples", "애니메이션 계측 표본 누락"}
            };
            const auto found = labels.find(name); return found == labels.end() ? name : found->second;
        }
        if (key.starts_with("memory::"))
        {
            static const std::map<std::string, std::string> labels = {
                {"privateCommitBytes", "프로세스 전용 commit"}, {"workingSetBytes", "프로세스 Working Set"},
                {"peakWorkingSetBytes", "프로세스 lifetime 최대 Working Set"}, {"peakPrivateCommitBytes", "프로세스 lifetime 최대 전용 commit"},
                {"systemCommitBytes", "시스템 commit"}, {"systemCommitLimitBytes", "시스템 commit 한도"}, {"systemAvailableBytes", "시스템 가용 RAM"},
                {"localUsageBytes", "DXGI Local 프로세스 사용량"}, {"localBudgetBytes", "DXGI Local 예산"},
                {"nonLocalUsageBytes", "DXGI Non-local 프로세스 사용량"}, {"nonLocalBudgetBytes", "DXGI Non-local 예산"}
            };
            const auto found = labels.find(name);
            return (found == labels.end() ? name : found->second) + " (MiB / OS 표본)";
        }
        const char* prefix = "";
        if (key.starts_with("cpu.main.self::")) prefix = "CPU 메인 자체 | ";
        else if (key.starts_with("cpu.main.total::")) prefix = "CPU 메인 전체 | ";
        else if (key.starts_with("cpu.worker.self::")) prefix = "CPU worker 합계 자체 | ";
        else if (key.starts_with("cpu.worker.total::")) prefix = "CPU worker 합계 전체 | ";
        else if (key.starts_with("gpu.self::")) prefix = "GPU 자체 | ";
        else if (key.starts_with("gpu.total::")) prefix = "GPU 전체 | ";
        else if (key.starts_with("work.time::")) prefix = "고정 CPU 작업 전체 | ";
        else if (key.starts_with("work.calls::")) prefix = "고정 CPU 작업 호출 | ";
        else if (key.starts_with("pipeline::"))
        {
            if (name == "iaVertices") return "GPU 프레임 IA 입력 정점";
            if (name == "iaPrimitives") return "GPU 프레임 IA 입력 primitive";
            if (name == "psInvocations") return "GPU 프레임 PS 호출";
            if (name == "vsInvocations") return "GPU 프레임 VS 호출";
            return "GPU 프레임 " + name;
        }
        else return key;
        const auto description = Scope_Description(name);
        return std::string(prefix) + (description && *description ? std::string(description) + " | " : "") + name;
    }

    bool ComparisonTimeKey(const std::string& key)
    { return key.starts_with("frame::") || key.starts_with("cpu.") || key.starts_with("work.time::") ||
        key.starts_with("animation.time::") || key.starts_with("gpu.self::") || key.starts_with("gpu.total::"); }

    const Client::FProfilerComparisonMean* ComparisonFind(const FComparisonMeans& means, const std::string& key)
    { const auto it = means.find(key); return it == means.end() ? nullptr : &it->second; }

    void ComparisonValue(const Client::FProfilerComparisonMean* value, bool time, double divisor = 1.0)
    {
        if (!value || !value->Available) ImGui::TextDisabled("미계측");
        else if (time) ImGui::Text("%.3f", value->Value);
        else ImGui::Text("%.2f", value->Value / divisor);
        if (value && ImGui::IsItemHovered())
            ImGui::SetTooltip("유효 관측 %zu / 대상 관측 %zu", value->Samples, value->Expected);
    }

    void ComparisonDelta(const FComparisonMeans& a, const FComparisonMeans& b, const std::string& key, bool time, double divisor = 1.0)
    {
        const auto* left = ComparisonFind(a, key); const auto* right = ComparisonFind(b, key);
        if (!left || !right || !left->Available || !right->Available) { ImGui::TextDisabled("--"); return; }
        const double delta = (right->Value - left->Value) / divisor;
        ImGui::TextColored(delta > 0.0 ? ImVec4(1.f, .65f, .3f, 1.f) : ImVec4(.55f, .85f, .65f, 1.f),
            time ? "%+.3f" : "%+.2f", delta);
    }

    // Pointers only live for this call; captures/mean maps cannot be replaced mid-table.
    void RenderComparisonRows(const FComparisonMeans& a, const FComparisonMeans& b, bool time, const char* filter)
    {
        struct FRow { std::string Key, Label; const Client::FProfilerComparisonMean* A = nullptr; const Client::FProfilerComparisonMean* B = nullptr; };
        std::map<std::string, FRow> joined;
        const auto add = [&](const FComparisonMeans& values, bool right)
        {
            for (const auto& [key, value] : values)
            {
                if (ComparisonTimeKey(key) != time || key.starts_with("gpu.draw.") || key.starts_with("gpu.pipeline.")) continue;
                auto& row = joined[key]; row.Key = key; row.Label = ComparisonMetricLabel(key);
                if (right) row.B = &value; else row.A = &value;
            }
        };
        add(a, false); add(b, true);
        std::vector<FRow> rows;
        for (auto& [key, row] : joined)
            if (Contains_CaseInsensitive(row.Label, filter) || Contains_CaseInsensitive(key, filter)) rows.push_back(std::move(row));
        const auto comparable = [](const FRow& row) { return row.A && row.B && row.A->Available && row.B->Available; };
        const auto delta = [](const FRow& row)
        { return (row.B->Value - row.A->Value) / (row.Key.starts_with("memory::") ? 1024.0 * 1024.0 : 1.0); };
        std::stable_sort(rows.begin(), rows.end(), [&](const auto& x, const auto& y)
        {
            if (comparable(x) != comparable(y)) return comparable(x);
            if (comparable(x) && delta(x) != delta(y)) return delta(x) > delta(y);
            return x.Key < y.Key;
        });
        ImGui::TextWrapped(time ?
            "B − A 증가 순위 (ms). 부모·자식과 CPU·GPU·worker·고정 작업은 중첩되므로 행을 합산하지 마세요. GPU 옆 제출량은 같은 패스의 전체 범위입니다. 고정 CPU 작업 옆에는 호출 차이를 표시합니다." :
            "B − A 수치 증가 순위. 단위가 다른 항목은 시간 병목 순위가 아닙니다. draw는 제출 횟수, 인덱스는 인스턴스 포함 제출량, IA는 GPU 입력량입니다. 인스턴싱 제출 수는 instanced draw만 셉니다. 광원 PS 호출은 정확한 빛 연산 횟수가 아닙니다.");
        if (!ImGui::BeginTable(time ? "##ComparisonTimes" : "##ComparisonWork", time ? 9 : 6, TABLE_FLAGS)) return;
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("항목", ImGuiTableColumnFlags_WidthStretch, 3.5f);
        for (const char* label : {"A", "B", "증감 B−A", "증감 %", "분모 A / B"}) ImGui::TableSetupColumn(label);
        if (time) for (const char* label : {"Δ draw/호출", "Δ 포함 index", "Δ PS 호출"}) ImGui::TableSetupColumn(label);
        ImGui::TableHeadersRow();
        ImGuiListClipper clipper; clipper.Begin(static_cast<int>(rows.size()));
        while (clipper.Step()) for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            const auto& row = rows[static_cast<size_t>(i)];
            const double divisor = row.Key.starts_with("memory::") ? 1024.0 * 1024.0 : 1.0;
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(row.Label.c_str());
            ImGui::TableNextColumn(); ComparisonValue(row.A, time, divisor);
            ImGui::TableNextColumn(); ComparisonValue(row.B, time, divisor);
            ImGui::TableNextColumn(); ComparisonDelta(a, b, row.Key, time, divisor);
            ImGui::TableNextColumn();
            if (comparable(row) && row.A->Value > 0.0) ImGui::Text("%+.1f%%", (row.B->Value / row.A->Value - 1.0) * 100.0);
            else ImGui::TextDisabled("--");
            ImGui::TableNextColumn();
            ImGui::Text("%zu/%zu | %zu/%zu", row.A ? row.A->Samples : 0, row.A ? row.A->Expected : 0,
                row.B ? row.B->Samples : 0, row.B ? row.B->Expected : 0);
            if (time)
            {
                const bool pass = row.Key.starts_with("gpu.");
                const auto name = pass ? row.Key.substr(row.Key.find("::") + 2) : std::string{};
                for (const auto* metric : {"drawCalls", "indices", "psInvocations"})
                {
                    ImGui::TableNextColumn();
                    if (pass) ComparisonDelta(a, b, std::string(metric == std::string_view("psInvocations") ? "gpu.pipeline." : "gpu.draw.") + metric + "::" + name, false);
                    else if (row.Key.starts_with("work.time::") && std::string_view(metric) == "drawCalls")
                        ComparisonDelta(a, b, "work.calls::" + row.Key.substr(row.Key.find("::") + 2), false);
                    else if (row.Key == "animation.time::cpuMs" && std::string_view(metric) == "drawCalls")
                        ComparisonDelta(a, b, "animation.count::updateCalls", false);
                    else if (row.Key == "frame::cpu" || row.Key == "frame::gpu")
                        ComparisonDelta(a, b, std::string(metric == std::string_view("psInvocations") ? "pipeline::" : "counter::") + metric, false);
                    else ImGui::TextDisabled("--");
                }
            }
        }
        ImGui::EndTable();
    }

    void RenderMeshDraws(const Client::FProfilerComparisonFrame& frame, const FComparisonMeans& values, const char* filter)
    {
        ImGui::TextWrapped("선택 프레임 #%llu의 CMesh 실제 제출 순서입니다. 이름은 표시용이며 asset·placement 고유 ID가 아닙니다. 개별 draw GPU 시간은 미계측입니다. 마지막 열은 같은 이름의 패스 전체 GPU 시간이며 draw마다 나누거나 합산하지 마세요.",
            static_cast<unsigned long long>(frame.Number));
        if (!frame.MeshDrawsKnown) { ImGui::TextUnformatted("상세 메시 draw 목록 미계측: 상세 수집을 켜고 새 프레임을 수집하세요. 과거 JSON의 필드 부재도 미계측입니다."); return; }
        ImGui::Text("보관 %zu / 누락 %llu (프레임당 상한 %zu)", frame.MeshDraws.size(),
            static_cast<unsigned long long>(frame.DroppedMeshDraws), Engine::CProfiler::MAX_MESH_DRAWS_PER_FRAME);
        ImGui::TextWrapped("정점은 CMesh geometry의 정점 수, index/draw는 실제 선택된 제출 인덱스 수 (LOD 반영), 제출 index는 index × instances입니다. 실제 참조된 고유 정점 수나 IA/VS 호출 수가 아닙니다. 패스 미계측·미지원·누락은 --로 표시합니다.");
        if (!ImGui::BeginTable("##MeshDrawTrace", 9, TABLE_FLAGS)) return;
        ImGui::TableSetupScrollFreeze(0, 1);
        for (const char* label : {"순서", "패스", "메시 이름", "재질 slot", "정점", "index/draw", "instances", "제출 index", "패스 전체 ms"}) ImGui::TableSetupColumn(label);
        ImGui::TableHeadersRow();
        for (size_t i = 0; i < frame.MeshDraws.size(); ++i)
        {
            const auto& draw = frame.MeshDraws[i];
            if (!Contains_CaseInsensitive(draw.Mesh, filter) && !Contains_CaseInsensitive(draw.Pass, filter) &&
                !Contains_CaseInsensitive(Scope_Description(draw.Pass), filter)) continue;
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::Text("%zu", i + 1);
            ImGui::TableNextColumn(); ImGui::TextUnformatted(draw.Pass.empty() ? "--" : draw.Pass.c_str());
            ImGui::TableNextColumn(); ImGui::TextUnformatted(draw.Mesh.empty() ? "이름 없음" : draw.Mesh.c_str());
            ImGui::TableNextColumn(); ImGui::Text("%u", draw.MaterialSlot);
            ImGui::TableNextColumn(); ImGui::Text("%u", draw.VertexCount);
            ImGui::TableNextColumn(); ImGui::Text("%u", draw.IndexCount);
            ImGui::TableNextColumn(); ImGui::Text("%u", draw.Instances);
            ImGui::TableNextColumn(); ImGui::Text("%llu", static_cast<unsigned long long>(uint64_t(draw.IndexCount) * draw.Instances));
            ImGui::TableNextColumn(); ComparisonValue(ComparisonFind(values, "gpu.total::" + draw.Pass), true);
        }
        ImGui::EndTable();
    }

    void RenderComparisonTables(const FComparisonMeans& a, const FComparisonMeans& b, const char* filter,
        const Client::FProfilerComparisonFrame* frame = nullptr)
    {
        if (!ImGui::BeginTabBar("##ComparisonMetricTabs")) return;
        if (ImGui::BeginTabItem("CPU·GPU 비용")) { RenderComparisonRows(a, b, true, filter); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("draw·메시·광원·컬링 작업량")) { RenderComparisonRows(a, b, false, filter); ImGui::EndTabItem(); }
        if (frame && ImGui::BeginTabItem("선택 프레임 메시 draw")) { RenderMeshDraws(*frame, b, filter); ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }

    const char* ComparisonGpuStatus(Engine::EProfilerGpuFrameStatus status)
    {
        switch (status)
        {
        case Engine::EProfilerGpuFrameStatus::Valid: return "유효";
        case Engine::EProfilerGpuFrameStatus::Pending: return "GPU 응답 대기";
        case Engine::EProfilerGpuFrameStatus::Disjoint: return "GPU clock 불연속";
        case Engine::EProfilerGpuFrameStatus::Dropped: return "GPU 수집 누락";
        case Engine::EProfilerGpuFrameStatus::Error: return "GPU query 오류";
        default: return "GPU 미지원·미계측";
        }
    }

    std::string ComparisonConditionLabel(const std::string& key)
    {
        static const std::map<std::string, std::string> labels = {
            {"levelId", "장면 Level ID"}, {"viewport", "렌더 viewport 해상도"}, {"cameraPosition", "카메라 위치"},
            {"viewMatrix", "카메라 view 행렬"}, {"projectionMatrix", "카메라 projection 행렬"},
            {"configuredForegroundFpsLimit", "전경 FPS 제한"}, {"configuredBackgroundFpsLimit", "배경 FPS 제한"},
            {"effectiveFpsLimit", "적용 FPS 제한 (0=해제)"}, {"foregroundWindowOwnedByProcess", "프로세스 전경"},
            {"clientWindowForeground", "메인 창 전경"}, {"windowMinimized", "최소화"},
            {"shadowEnabled", "그림자"}, {"shadowWidthHeightStrength", "그림자 폭·높이·강도"},
            {"ssaoEnabled", "SSAO"}, {"bloomEnabled", "Bloom"}, {"fxaaEnabled", "FXAA"},
            {"sampledAtExport", "보관 시점 context"}, {"runtimeContextValid", "장면 context 유효"},
            {"buildConfiguration", "빌드 구성"}, {"adapter", "GPU adapter"}, {"deviceCreationFlags", "D3D 생성 옵션"},
            {"d3dDebugLayer", "D3D debug layer"}, {"debuggerAttached", "디버거 연결"}, {"logicalProcessors", "논리 CPU 수"}
        };
        const auto found = labels.find(key); return found == labels.end() ? key : found->second;
    }

    void RenderComparisonConditions(const Client::FProfilerComparisonCapture& a, const Client::FProfilerComparisonCapture& b)
    {
        if (!ImGui::CollapsingHeader("비교 조건·수집 범위 (차이와 미계측 확인)", ImGuiTreeNodeFlags_DefaultOpen)) return;
        const auto summarize = [](const auto& capture)
        {
            size_t gpu = 0, passes = 0, detail = 0, unknown = 0, self = 0;
            for (const auto& frame : capture.Frames)
            { gpu += frame.GpuValid; passes += frame.GpuComplete; detail += frame.DetailKnown && frame.Detailed; unknown += !frame.DetailKnown; self += frame.CpuSelfKnown; }
            ImGui::TextWrapped("%s | %zu 프레임 | GPU 전체 %zu / 패스 %zu | CPU self 완전 %zu | 상세 ON %zu, 상태 미계측 %zu",
                capture.Label.c_str(), capture.Frames.size(), gpu, passes, self, detail, unknown);
            ImGui::TextDisabled("출처: %s", capture.Source.c_str());
        };
        summarize(a); summarize(b);
        if (!ImGui::BeginTable("##ComparisonConditions", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, ImVec2(0.f, 145.f))) return;
        ImGui::TableSetupScrollFreeze(0, 1);
        for (const char* label : {"조건", "A", "B", "판정"}) ImGui::TableSetupColumn(label);
        ImGui::TableHeadersRow();
        std::map<std::string, bool> keys;
        for (const auto& [key, value] : a.Conditions) keys[key] = true;
        for (const auto& [key, value] : b.Conditions) keys[key] = true;
        // These must remain visible even for legacy captures with no metadata.
        for (const auto* key : {"levelId", "viewport", "cameraPosition", "viewMatrix", "projectionMatrix", "effectiveFpsLimit",
            "shadowEnabled", "ssaoEnabled", "bloomEnabled", "fxaaEnabled"}) keys[key] = true;
        for (const auto& [key, ignored] : keys)
        {
            const auto left = a.Conditions.find(key), right = b.Conditions.find(key);
            const bool known = left != a.Conditions.end() && right != b.Conditions.end();
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(ComparisonConditionLabel(key).c_str());
            ImGui::TableNextColumn(); ImGui::TextWrapped("%s", left == a.Conditions.end() ? "미계측" : left->second.c_str());
            ImGui::TableNextColumn(); ImGui::TextWrapped("%s", right == b.Conditions.end() ? "미계측" : right->second.c_str());
            ImGui::TableNextColumn(); ImGui::TextUnformatted(!known ? "미계측" : left->second == right->second ? "같음" : "다름");
        }
        ImGui::EndTable();
    }
}

void Client::CProfilerTool::Render_FrameChanges(Engine::CProfiler& profiler)
{
    if (ImGui::Checkbox("최신 완료 프레임 따라가기", &m_bFollowComparisonFrames)) m_bComparisonRefresh = true;
    ImGui::SameLine();
    const bool refresh = ImGui::Button("현재 창 다시 가져오기");
    if (refresh || (m_bComparisonRefresh && m_bFollowComparisonFrames) || m_ComparisonFrames.Frames.empty())
    {
        auto snapshot = profiler.Snapshot(static_cast<size_t>((std::max)(2, m_iWindowFrameInput)));
        m_ComparisonFrames = CProfilerCaptureIO::Build_Comparison(snapshot, {}, "완료 프레임");
        m_iComparisonFrame = static_cast<int>(m_ComparisonFrames.Frames.size()) - 1;
        m_iComparedFrame = -1;
        m_bComparisonRefresh = false;
    }
    if (m_ComparisonFrames.Frames.empty()) { ImGui::TextUnformatted("수집된 완료 프레임이 없습니다."); return; }
    ImGui::TextWrapped("실제 완료 프레임을 선택합니다. 이 표는 선택 프레임 B와 바로 이전 보관 프레임 A의 차이이며, A/B 기준창 평균과 다릅니다. GPU는 각 프레임 번호에 귀속된 결과만 표시합니다. 대기 중 결과를 다른 프레임 GPU 값으로 대신하지 않습니다.");
    ImGui::SetNextItemWidth(360.f);
    if (ImGui::SliderInt("보관 프레임 위치", &m_iComparisonFrame, 0, static_cast<int>(m_ComparisonFrames.Frames.size()) - 1))
        m_bFollowComparisonFrames = false;
    const auto& current = m_ComparisonFrames.Frames[static_cast<size_t>(m_iComparisonFrame)];
    ImGui::Text("선택 B #%llu | %s | 상세 %s", static_cast<unsigned long long>(current.Number), ComparisonGpuStatus(current.GpuStatus),
        !current.DetailKnown ? "미계측" : current.Detailed ? "ON" : "OFF");
    if (m_iComparisonFrame == 0) ImGui::TextUnformatted("이전 보관 프레임 없음: A와 차이는 미계측입니다.");
    else
    {
        const auto& previous = m_ComparisonFrames.Frames[static_cast<size_t>(m_iComparisonFrame - 1)];
        ImGui::Text("이전 A #%llu | %s", static_cast<unsigned long long>(previous.Number), ComparisonGpuStatus(previous.GpuStatus));
        if (previous.Number + 1 != current.Number)
            ImGui::TextColored(ImVec4(1.f, .65f, .3f, 1.f), "보관 프레임 번호 사이에 공백이 있습니다. 연속 실행 1프레임 차이가 아닙니다.");
    }
    if (m_iComparedFrame != m_iComparisonFrame)
    {
        FProfilerComparisonCapture a, b; b.Frames.push_back(current);
        if (m_iComparisonFrame > 0) a.Frames.push_back(m_ComparisonFrames.Frames[static_cast<size_t>(m_iComparisonFrame - 1)]);
        m_FrameComparisonMeans[0] = CProfilerCaptureIO::Compare_Means(a);
        m_FrameComparisonMeans[1] = CProfilerCaptureIO::Compare_Means(b);
        m_iComparedFrame = m_iComparisonFrame;
    }
    RenderComparisonTables(m_FrameComparisonMeans[0], m_FrameComparisonMeans[1], m_Filter.data(), &current);
}

void Client::CProfilerTool::Render_Comparison(Engine::CProfiler& profiler)
{
    ImGui::TextWrapped("쿠크 2관문·빙고·현재 장면처럼 이름을 붙여 완료 프레임 창을 보관합니다. 각 기준은 독립 스냅샷이며 새 수집·Reset 뒤에도 유지됩니다. 저장 JSON은 '저장 JSON' 탭에서 선택한 파일을 사용합니다.");
    for (size_t side = 0; side < m_Baselines.size(); ++side)
    {
        ImGui::PushID(static_cast<int>(side));
        ImGui::SetNextItemWidth(260.f);
        ImGui::InputTextWithHint(side ? "B 이름" : "A 이름", side ? "빙고 / 현재 장면" : "쿠크 2관문 / 기준 장면",
            m_BaselineLabels[side].data(), m_BaselineLabels[side].size());
        ImGui::SameLine();
        if (ImGui::Button(side ? "현재 수집창 → B" : "현재 수집창 → A"))
        {
            Engine::CProfilerScope scope(&profiler, "Profiler.Comparison.Snapshot");
            auto snapshot = profiler.Snapshot(static_cast<size_t>((std::max)(1, m_iWindowFrameInput)));
            if (snapshot.Frames.empty()) m_ComparisonStatus = "완료 프레임이 없어 이전 기준을 유지했습니다.";
            else
            {
                auto label = std::string(m_BaselineLabels[side].data());
                if (label.empty()) label = side ? "B 현재 장면" : "A 기준 장면";
                auto staged = CProfilerCaptureIO::Build_Comparison(snapshot, Sample_Context(), std::move(label));
                auto means = CProfilerCaptureIO::Compare_Means(staged);
                m_Baselines[side] = std::move(staged); m_BaselineMeans[side] = std::move(means);
                m_ComparisonStatus = "완료 프레임 창을 이 실행의 기준으로 보관했습니다. 상단 JSON 저장은 저장 버튼을 누른 시점의 수집창을 저장합니다.";
            }
        }
        ImGui::SameLine();
        const auto selected = std::find_if(m_CaptureFiles.begin(), m_CaptureFiles.end(),
            [&](const auto& file) { return file.StableId == m_strSelectedCaptureId; });
        ImGui::BeginDisabled(selected == m_CaptureFiles.end() || m_Exporter.IsSaving());
        if (ImGui::Button(side ? "선택 JSON → B" : "선택 JSON → A") && selected != m_CaptureFiles.end())
        {
            FProfilerComparisonCapture staged; std::string error;
            if (CProfilerCaptureIO::Load_Comparison(CProfilerCaptureIO::Get_CaptureDirectory(), *selected, staged, &error))
            {
                if (m_BaselineLabels[side][0]) staged.Label = m_BaselineLabels[side].data();
                auto means = CProfilerCaptureIO::Compare_Means(staged);
                m_Baselines[side] = std::move(staged); m_BaselineMeans[side] = std::move(means);
                m_ComparisonStatus = "선택 JSON을 검증한 뒤 기준을 교체했습니다. 파일 읽기·분석을 실행한 현재 프레임은 측정 실험에서 제외하세요.";
            }
            else m_ComparisonStatus = "불러오기 실패: 이전 기준 유지. " + error;
        }
        ImGui::EndDisabled();
        if (selected != m_CaptureFiles.end()) ImGui::TextDisabled("선택 JSON: %s", selected->DisplayName.c_str());
        ImGui::PopID();
    }
    if (!m_ComparisonStatus.empty()) ImGui::TextWrapped("%s", m_ComparisonStatus.c_str());
    if (m_Baselines[0].Frames.empty() || m_Baselines[1].Frames.empty())
    { ImGui::TextUnformatted("A와 B를 각각 보관하거나 불러오면 창 평균·작업량 차이를 표시합니다."); return; }
    ImGui::TextWrapped("구조 비교: 장면·카메라·설정·FPS 제한·상세 계측·유효 GPU 분모 차이가 섞일 수 있습니다. 다른 장면의 차이를 한 옵션의 성능 효과로 단정하지 않습니다. context는 보관/저장 시점 표본이며 과거 창 전체가 같은 조건이었다는 보장은 없습니다. 개별 오브젝트 재질·광원별 설정은 전부 저장하지 않습니다.");
    RenderComparisonConditions(m_Baselines[0], m_Baselines[1]);
    ImGui::Separator();
    ImGui::TextWrapped("아래는 캡처창의 프레임당 평균입니다. CPU는 같은 이름과 main/worker 역할로 병합합니다. 고정 CPU 작업·애니메이션은 상세 OFF에서도 수집한 전체 비용이며 다른 scope와 합산하지 않습니다. GPU 패스는 유효·완료·누락 없음 프레임만 평균에 포함하며 분모를 표시합니다. 누락 필드·부분 self·생산자 없는 항목은 미계측입니다.");
    ImGui::TextWrapped("메모리 항목만은 예외로 같은 OS 조회를 재사용한 프레임을 중복 가중하지 않습니다. memory:: 평균·분모는 서로 다른 ProcessId·SampleTick·SampleFrameNumber의 실제 OS 조회 표본 기준입니다.");
    RenderComparisonTables(m_BaselineMeans[0], m_BaselineMeans[1], m_Filter.data());
}

bool Client::CProfilerTool::Refresh_RecentFrames(Engine::CProfiler& profiler, bool force)
{
    if (!force && !m_bTimelineRefresh && !m_RecentFrameSnapshot.Frames.empty()) return false;
    Engine::CProfilerScope scope(&profiler, "Profiler.Timeline.Snapshot");
    m_RecentFrameSnapshot = profiler.Snapshot(120);
    m_bTimelineRefresh = false;
    if (m_bTimelineFollow) m_iTimelineBuiltFrame = -1;
    return true;
}

const Engine::FProfilerCaptureSnapshot& Client::CProfilerTool::Timeline_Snapshot() const
{
    return m_bTimelineFollow ? m_RecentFrameSnapshot : m_TimelineSnapshot;
}

void Client::CProfilerTool::Freeze_Timeline()
{
    if (m_bTimelineFollow) m_TimelineSnapshot = m_RecentFrameSnapshot;
    m_bTimelineFollow = false;
}

void Client::CProfilerTool::Render_Timeline(Engine::CProfiler& profiler)
{
    bool follow = m_bTimelineFollow;
    const bool followChanged = ImGui::Checkbox("완료 프레임 따라가기###TimelineFollow", &follow);
    if (followChanged)
    {
        if (!follow) Freeze_Timeline();
        else { m_bTimelineFollow = true; m_bTimelineRefresh = true; }
    }
    ImGui::SameLine();
    const bool refresh = ImGui::Button("완료 프레임 다시 가져오기###TimelineRefresh");
    ImGui::SameLine();
    const bool modeChanged = ImGui::Checkbox("최신 GPU 완료 우선###TimelinePreferGpu", &m_bTimelinePreferGpu);
    if (m_bTimelineFollow || refresh || Timeline_Snapshot().Frames.empty())
    {
        const bool updated = Refresh_RecentFrames(profiler, refresh);
        if (!m_bTimelineFollow) m_TimelineSnapshot = m_RecentFrameSnapshot;
        if (updated || refresh || followChanged || modeChanged || m_iTimelineFrame < 0 || m_iTimelineBuiltFrame < 0)
        {
            m_iTimelineFrame = ProfilerLatestObservableFrame(Timeline_Snapshot(), m_bTimelinePreferGpu);
            m_iTimelineBuiltFrame = -1; m_iTimelineSelectedEvent = -1;
        }
    }
    if (Timeline_Snapshot().Frames.empty()) { ImGui::TextUnformatted("완료 프레임 없음: 수집을 켜고 프레임이 끝날 때까지 기다리세요."); return; }
    ImGui::TextWrapped("최근 최대 120개 완료 프레임의 실제 scope입니다. CPU는 프레임 CPU 시작 QPC 기준, GPU는 같은 번호 GPU query 시작 기준의 독립 상대축입니다. 두 축의 x 위치를 동기화된 시각이나 CPU→GPU 지연으로 해석하지 마세요. GPU 응답 대기를 다른 프레임 결과로 대체하지 않습니다.");
    ImGui::SetNextItemWidth(340.f);
    m_iTimelineFrame = (std::clamp)(m_iTimelineFrame, 0, static_cast<int>(Timeline_Snapshot().Frames.size()) - 1);
    if (ImGui::SliderInt("완료 프레임 위치###TimelineFrame", &m_iTimelineFrame, 0, static_cast<int>(Timeline_Snapshot().Frames.size()) - 1)) Freeze_Timeline();
    const auto& snapshot = Timeline_Snapshot();
    const auto& frame = snapshot.Frames[static_cast<size_t>(m_iTimelineFrame)];
    if (m_iTimelineBuiltFrame != m_iTimelineFrame)
    {
        m_TimelineEvents = ProfilerBuildTimelineEvents(frame, snapshot.TicksPerSecond);
        m_CpuTimelineView = {0.0, (std::max)(0.01, frame.CpuFrameMs)};
        m_GpuTimelineView = {0.0, (std::max)(0.01, frame.GpuFrameMs)};
        m_iTimelineBuiltFrame = m_iTimelineFrame; m_iTimelineSelectedEvent = -1;
    }
    const double covered = ProfilerMainCoveredMs(frame, snapshot.MainThreadId, snapshot.TicksPerSecond);
    ImGui::Text("선택 #%llu / 최근 CPU 완료 #%llu | %llu 프레임 지연 | %s", static_cast<unsigned long long>(frame.FrameNumber),
        static_cast<unsigned long long>(snapshot.Frames.back().FrameNumber),
        static_cast<unsigned long long>(snapshot.Frames.back().FrameNumber - frame.FrameNumber),
        m_bTimelineFollow ? (m_bTimelinePreferGpu ? "GPU 유효 완료 우선" : "최신 CPU 완료") : "수동 고정");
    ImGui::Text("프레임 #%llu | CPU %.3f ms | GPU %s | 메인 scope 합집합 %.3f ms / 구간 밖 %.3f ms",
        static_cast<unsigned long long>(frame.FrameNumber), frame.CpuFrameMs, ComparisonGpuStatus(frame.GpuStatus), covered,
        (std::max)(0.0, frame.CpuFrameMs - covered));
    ImGui::Text("상세 %s | CPU 표본 누락 %llu / GPU 패스 누락 %u / 메시 draw 누락 %llu",
        frame.DetailedCpuScopes ? "ON" : "OFF", static_cast<unsigned long long>(frame.DroppedCpuScopes),
        frame.DroppedGpuScopes, static_cast<unsigned long long>(frame.DroppedMeshDraws));
    if (ImGui::CollapsingHeader("타임라인 해석: 부모·자식 / self / 대기 / 프레임 경계"))
        ImGui::TextWrapped("행은 스레드와 호출 깊이(depth)입니다. 부모 전체 시간에는 자식이 포함되며 self는 같은 스레드의 관측된 직접 자식만 뺀 시간입니다. 미계측 자식·스케줄링 대기는 self에 남습니다. 빈 곳은 idle 증거가 아니며 OS context switch·task dependency·GPU queue/fence는 미계측입니다. worker는 완료 프레임에 귀속되어 이전 프레임에서 시작한 구간도 있습니다. 주황 테두리는 축에서 잘린 구간이며 원본 시간은 선택 정보에 남습니다. 창 밖에 있는 부모·누락·잘못된 중첩은 self를 미계측으로 만듭니다. 고정한 스냅샷의 pending GPU는 자동 갱신되지 않습니다.");
    if (m_iTimelineSelectedEvent >= 0 && static_cast<size_t>(m_iTimelineSelectedEvent) < m_TimelineEvents.size())
    {
        const auto& event = m_TimelineEvents[static_cast<size_t>(m_iTimelineSelectedEvent)];
        const char* name = event.NameId < snapshot.ScopeNames.size() ? snapshot.ScopeNames[event.NameId].c_str() : "<unknown>";
        ImGui::Separator(); Scope_Label(name);
        ImGui::Text("%s | thread %u / depth %u | [%.3f, %.3f] ms | 전체 %.3f ms%s", event.Gpu ? "GPU 상대축" : "CPU QPC 상대축",
            event.ThreadId, event.Depth, event.BeginMs, event.EndMs, event.EndMs - event.BeginMs, event.CrossFrame ? " | CPU 프레임 경계 밖 포함" : "");
        if (event.SelfKnown) ImGui::Text("관측 self %.3f ms (순수 연산 시간 아님)", event.SelfMs);
        else ImGui::TextDisabled("self 미계측: 누락·부모 부재·계층 불완전·CPU 프레임 경계 밖");
        if (event.Gpu)
        {
            ImGui::Text("패스 포함 draw %llu | index %llu | mesh draw %llu | instanced 제출 %llu",
                static_cast<unsigned long long>(event.Draw.DrawCalls), static_cast<unsigned long long>(event.Draw.Indices),
                static_cast<unsigned long long>(event.Draw.MeshDrawCalls), static_cast<unsigned long long>(event.Draw.Instances));
            if (event.PipelineValid) ImGui::Text("PS %llu / VS %llu / IA 입력 정점 %llu", static_cast<unsigned long long>(event.PSInvocations),
                static_cast<unsigned long long>(event.VSInvocations), static_cast<unsigned long long>(event.IAVertices));
            else ImGui::TextDisabled("해당 패스 pipeline 통계 미계측");
        }
        else ImGui::TextDisabled("CPU event별 draw 귀속은 미계측입니다. GPU 패스 또는 '선택 프레임 메시 draw'에서 확인하세요.");
        if (ImGui::SmallButton("선택 구간 확대###FitTimelineEvent"))
        {
            auto& view = event.Gpu ? m_GpuTimelineView : m_CpuTimelineView;
            view = {event.BeginMs, (std::max)(0.001, event.EndMs - event.BeginMs)};
        }
    }
    Render_TimelineAxis(false, frame);
    Render_TimelineAxis(true, frame);
}

void Client::CProfilerTool::Render_TimelineAxis(bool gpu, const Engine::FProfilerFrame& frame)
{
    const auto& snapshot = Timeline_Snapshot();
    ImGui::PushID(gpu ? "GpuTimeline" : "CpuTimeline");
    ImGui::SeparatorText(gpu ? "GPU 패스 — GPU frame 시작 기준 ms" : "CPU 스레드·호출 깊이 — CPU frame 시작 QPC 기준 ms");
    if (gpu && !frame.GpuValid)
    { ImGui::TextDisabled("%s: 해당 완료 프레임의 유효 GPU 시간 없음", ComparisonGpuStatus(frame.GpuStatus)); ImGui::PopID(); return; }
    if (!gpu && !snapshot.TicksPerSecond)
    { ImGui::TextDisabled("QPC 주파수 미계측"); ImGui::PopID(); return; }
    auto& view = gpu ? m_GpuTimelineView : m_CpuTimelineView;
    if (ImGui::SmallButton("확대")) view.Zoom(0.5);
    ImGui::SameLine(); if (ImGui::SmallButton("축소")) view.Zoom(2.0);
    ImGui::SameLine(); if (ImGui::SmallButton("왼쪽")) view.Pan(-0.25);
    ImGui::SameLine(); if (ImGui::SmallButton("오른쪽")) view.Pan(0.25);
    ImGui::SameLine(); if (ImGui::SmallButton("프레임에 맞춤")) view = {0.0, (std::max)(0.01, gpu ? frame.GpuFrameMs : frame.CpuFrameMs)};
    ImGui::SameLine(); ImGui::SetNextItemWidth(110.f); ImGui::InputDouble("시작 ms", &view.BeginMs, 0.0, 0.0, "%.3f");
    ImGui::SameLine(); ImGui::SetNextItemWidth(100.f); ImGui::InputDouble("폭 ms", &view.SpanMs, 0.0, 0.0, "%.3f");
    if (!std::isfinite(view.BeginMs)) view.BeginMs = 0.0;
    if (!std::isfinite(view.SpanMs)) view.SpanMs = 16.6667;
    view.BeginMs = (std::clamp)(view.BeginMs, -3600000.0, 3600000.0);
    view.SpanMs = (std::clamp)(view.SpanMs, 0.001, 3600000.0);
    using FLane = std::pair<uint32_t, uint32_t>;
    std::vector<FLane> lanes;
    for (const auto& event : m_TimelineEvents)
        if (event.Gpu == gpu && std::find(lanes.begin(), lanes.end(), FLane{event.ThreadId, event.Depth}) == lanes.end())
            lanes.emplace_back(event.ThreadId, event.Depth);
    std::sort(lanes.begin(), lanes.end(), [&](const auto& a, const auto& b)
    {
        if ((a.first == snapshot.MainThreadId) != (b.first == snapshot.MainThreadId)) return a.first == snapshot.MainThreadId;
        return a < b;
    });
    if (lanes.empty()) { ImGui::TextDisabled("이 축에서 관측된 구간이 없습니다. 미지원·미계측·미실행은 시간 0과 다릅니다."); ImGui::PopID(); return; }
    const float laneHeight = ImGui::GetTextLineHeightWithSpacing() + 5.f;
    const float labelWidth = 155.f, headerHeight = 28.f;
    ImGui::BeginChild("Canvas", ImVec2(0.f, gpu ? 230.f : 290.f), true);
    const auto origin = ImGui::GetCursorScreenPos();
    const float width = (std::max)(220.f, ImGui::GetContentRegionAvail().x);
    const float axisWidth = (std::max)(50.f, width - labelWidth - 5.f);
    const float height = headerHeight + laneHeight * static_cast<float>(lanes.size());
    ImGui::InvisibleButton("##TimelineCanvas", ImVec2(width, height));
    const bool canvasHovered = ImGui::IsItemHovered();
    const auto mouse = ImGui::GetIO().MousePos;
    auto* draw = ImGui::GetWindowDrawList();
    const auto visibleOrigin = ImGui::GetWindowPos(); const auto visibleSize = ImGui::GetWindowSize();
    draw->AddRectFilled(origin, ImVec2(origin.x + width, origin.y + height), IM_COL32(25, 29, 36, 255));
    for (int tick = 0; tick <= 5; ++tick)
    {
        const float x = origin.x + labelWidth + axisWidth * (float(tick) / 5.f);
        char text[48]; std::snprintf(text, sizeof(text), "%.3f", view.BeginMs + view.SpanMs * double(tick) / 5.0);
        draw->AddText(ImVec2(x, origin.y + 3.f), IM_COL32(190, 195, 205, 255), text);
        draw->AddLine(ImVec2(x, origin.y + headerHeight), ImVec2(x, origin.y + height), IM_COL32(55, 60, 72, 255));
    }
    for (size_t lane = 0; lane < lanes.size(); ++lane)
    {
        const float y = origin.y + headerHeight + laneHeight * float(lane);
        if (y + laneHeight < visibleOrigin.y || y > visibleOrigin.y + visibleSize.y) continue;
        char label[96];
        if (gpu) std::snprintf(label, sizeof(label), "GPU / depth %u", lanes[lane].second);
        else std::snprintf(label, sizeof(label), "%s %u / d%u", lanes[lane].first == snapshot.MainThreadId ? "Main" : "Worker", lanes[lane].first, lanes[lane].second);
        draw->AddText(ImVec2(origin.x + 3.f, y + 2.f), IM_COL32(205, 211, 220, 255), label);
        draw->AddLine(ImVec2(origin.x, y + laneHeight), ImVec2(origin.x + width, y + laneHeight), IM_COL32(49, 54, 65, 255));
    }
    int hovered = -1; size_t clipped = 0, crossFrame = 0, filtered = 0;
    draw->PushClipRect(ImVec2(origin.x + labelWidth, visibleOrigin.y), ImVec2(origin.x + width, visibleOrigin.y + visibleSize.y), true);
    for (size_t index = 0; index < m_TimelineEvents.size(); ++index)
    {
        const auto& event = m_TimelineEvents[index]; if (event.Gpu != gpu) continue;
        const char* name = event.NameId < snapshot.ScopeNames.size() ? snapshot.ScopeNames[event.NameId].c_str() : "<unknown>";
        if (!Contains_CaseInsensitive(name, m_Filter.data()) && !Contains_CaseInsensitive(Scope_Description(name), m_Filter.data())) { ++filtered; continue; }
        crossFrame += event.CrossFrame;
        const auto clip = ProfilerClipTimeline(event.BeginMs, event.EndMs, view); if (!clip.Visible) continue;
        clipped += clip.LeftClipped || clip.RightClipped;
        const auto lane = std::find(lanes.begin(), lanes.end(), FLane{event.ThreadId, event.Depth});
        const float y = origin.y + headerHeight + float(lane - lanes.begin()) * laneHeight;
        if (y + laneHeight < visibleOrigin.y || y > visibleOrigin.y + visibleSize.y) continue;
        const float x1 = origin.x + labelWidth + float(clip.BeginFraction) * axisWidth;
        const float x2 = (std::max)(x1 + 1.0f, origin.x + labelWidth + float(clip.EndFraction) * axisWidth);
        const ImVec2 lo(x1, y + 2.f), hi(x2, y + laneHeight - 2.f);
        const auto color = ImGui::ColorConvertFloat4ToU32(ImColor::HSV(float((event.NameId * 47u) % 360u) / 360.f, .55f, .68f).Value);
        draw->AddRectFilled(lo, hi, color, 2.f);
        if (clip.LeftClipped || clip.RightClipped) draw->AddRect(lo, hi, IM_COL32(255, 177, 60, 255), 2.f, 0, 2.f);
        if (static_cast<int>(index) == m_iTimelineSelectedEvent) draw->AddRect(lo, hi, IM_COL32(255, 255, 255, 255), 2.f, 0, 2.f);
        if (x2 - x1 > 35.f)
        {
            draw->PushClipRect(lo, hi, true); draw->AddText(ImVec2(x1 + 3.f, y + 3.f), IM_COL32(255, 255, 255, 255), name); draw->PopClipRect();
        }
        if (canvasHovered && mouse.x >= lo.x && mouse.x <= hi.x && mouse.y >= lo.y && mouse.y <= hi.y) hovered = static_cast<int>(index);
    }
    draw->PopClipRect();
    if (hovered >= 0)
    {
        const auto& event = m_TimelineEvents[static_cast<size_t>(hovered)];
        const char* name = event.NameId < snapshot.ScopeNames.size() ? snapshot.ScopeNames[event.NameId].c_str() : "<unknown>";
        ImGui::SetTooltip("%s\n%s\n전체 %.3f ms | 시작 %.3f / 종료 %.3f\n클릭하여 선택·스냅샷 고정", Scope_Description(name), name, event.EndMs - event.BeginMs, event.BeginMs, event.EndMs);
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) { m_iTimelineSelectedEvent = hovered; Freeze_Timeline(); }
    }
    ImGui::EndChild();
    ImGui::TextDisabled("현재 축에서 잘림 %zu | CPU 프레임 경계 밖 포함 %zu | 검색으로 숨김 %zu", clipped, crossFrame, filtered);
    ImGui::PopID();
}

void Client::CProfilerTool::Render_Candidates(Engine::CProfiler& profiler)
{
    Refresh_RecentFrames(profiler);
    const int selected = ProfilerLatestObservableFrame(m_RecentFrameSnapshot);
    if (selected < 0) { ImGui::TextUnformatted("완료 프레임이 없어 후보를 판단하지 않습니다."); return; }
    ImGui::TextWrapped("최근 완료 CPU 프레임과 그 번호에 귀속된 GPU 결과만 사용합니다. 아래는 원인 확정이 아닌 다음 검사를 고르는 후보입니다. 프레임 경과·대기·GPU timestamp와 순수 연산 비용은 다르며 서로 더하지 않습니다.");
    ImGui::SetNextItemWidth(120.f); ImGui::InputFloat("검토 목표 FPS", &m_fCandidateTargetFps, 1.f, 10.f, "%.1f");
    ImGui::SameLine(); ImGui::SetNextItemWidth(120.f); ImGui::InputInt("draw 검토 기준", &m_iCandidateDrawThreshold, 100, 1000);
    if (!std::isfinite(m_fCandidateTargetFps)) m_fCandidateTargetFps = 60.f;
    m_fCandidateTargetFps = (std::clamp)(m_fCandidateTargetFps, 1.f, 1000.f);
    m_iCandidateDrawThreshold = (std::clamp)(m_iCandidateDrawThreshold, 1, 10000000);
    const auto& frame = m_RecentFrameSnapshot.Frames[static_cast<size_t>(selected)];
    const auto evidence = ProfilerAssessCandidates(frame, 1000.0 / m_fCandidateTargetFps, static_cast<uint64_t>(m_iCandidateDrawThreshold));
    ImGui::Text("프레임 #%llu | 검토 예산 %.3f ms | CPU %.3f ms | GPU %s", static_cast<unsigned long long>(frame.FrameNumber),
        evidence.TargetMs, frame.CpuFrameMs, ComparisonGpuStatus(frame.GpuStatus));
    ImGui::Text("최근 CPU 완료 #%llu보다 %llu 프레임 지연 | %s",
        static_cast<unsigned long long>(m_RecentFrameSnapshot.Frames.back().FrameNumber),
        static_cast<unsigned long long>(m_RecentFrameSnapshot.Frames.back().FrameNumber - frame.FrameNumber),
        frame.GpuValid ? "최근 120개 중 최신 GPU 유효 완료: CPU·GPU 같은 원본 프레임" : "유효 GPU 완료 없음: 최신 CPU 원본의 pending / 미계측 상태 유지");
    ImGui::TextDisabled("기준값은 이 패널의 후보 표시만 바꿉니다. 실제 FPS 제한이나 렌더 옵션을 변경하지 않습니다.");
    if (ImGui::BeginTable("##CandidateExperiments", 4, TABLE_FLAGS, ImVec2(0.f, 330.f)))
    {
        ImGui::TableSetupColumn("후보 / 상태", ImGuiTableColumnFlags_WidthFixed, 180.f);
        ImGui::TableSetupColumn("현재 표본 근거", ImGuiTableColumnFlags_WidthStretch, 1.f);
        ImGui::TableSetupColumn("다음 A/B·확인", ImGuiTableColumnFlags_WidthStretch, 1.3f);
        ImGui::TableSetupColumn("아직 모르는 원인", ImGuiTableColumnFlags_WidthStretch, 1.f); ImGui::TableHeadersRow();
        const auto row = [&](const char* name, bool candidate, const std::string& observed, const char* next, const char* missing)
        {
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextWrapped("%s\n%s", name, candidate ? "검토 후보" : "기준 미충족 / 보류");
            ImGui::TableNextColumn(); ImGui::TextWrapped("%s", observed.c_str());
            ImGui::TableNextColumn(); ImGui::TextWrapped("%s", next);
            ImGui::TableNextColumn(); ImGui::TextWrapped("%s", missing);
        };
        const auto ms = [](double value) { char text[40]; std::snprintf(text, sizeof(text), "%.3f ms", value); return std::string(text); };
        row("CPU 경과", evidence.CpuOverBudget, "전체 " + ms(frame.CpuFrameMs) + " / 예산 " + ms(evidence.TargetMs),
            "CPU 타임라인에서 큰 self·고정 작업·Present 구간을 확인하고 같은 장면에서 한 변수만 바꿔 A/B 비교합니다.",
            "OS 스케줄링·동기화 wait·디스크·page fault는 미계측입니다. CPU 우세만으로 계산 병목을 확정하지 않습니다.");
        row("GPU 경과", evidence.GpuOverBudget, frame.GpuValid ? "원래 프레임 GPU " + ms(frame.GpuFrameMs) : ComparisonGpuStatus(frame.GpuStatus),
            "GPU 패스와 포함 draw/PS·IA를 확인합니다. 같은 카메라에서 해상도·SSAO 표본·그림자 PCF 중 하나를 바꿔 비교합니다.",
            "queue stall·캐시·대역폭·wave 점유율·개별 draw ms는 미계측입니다. PS 호출은 광원 ALU 수가 아닙니다.");
        row("프레임 사이 간격", evidence.GapLarge, "이전 CPU " + ms(frame.PreviousCpuFrameMs) + " + gap " + ms(frame.FrameGapMs),
            "FPS cap·전경/배경·최소화 조건을 맞추고 A/B합니다. CPU/GPU 시간이 낮아도 cap 대기로 FPS가 제한될 수 있습니다.",
            "gap에는 loop 대기·메시지·계측 관리가 섞입니다. 현재 CPU 시간을 이전 CPU 대신 더하지 않습니다.");
        row("갱신 후 미제출 애니메이션", evidence.AnimationNotSubmitted,
            "모델 " + std::to_string(frame.Animation.NotSubmittedUpdatedModels) + " / CPU " + ms(frame.Animation.NotSubmittedCpuMs),
            "동일 장면의 카메라·저작 숨김·NPC culling·지연 pose 조건을 확인하고 미제출 CPU와 표시 결과를 함께 비교합니다.",
            "미제출은 화면 밖 확정이 아닙니다. 숨김·preview·draw 실패도 포함하며 animation 표본 누락이면 후보를 보류합니다.");
        row("많은 제출", evidence.ManySubmissions, "Engine + ImGui draw " + std::to_string(evidence.DrawCalls) + " / 검토 기준 " + std::to_string(m_iCandidateDrawThreshold),
            "패스·상세 메시 목록에서 반복 제출을 보고 instancing·LOD·culling을 한 항목씩 비교합니다. UI 창 수도 동일하게 맞춥니다.",
            "호출 수만으로 비용을 확정하지 않습니다. 재질 전환·shader 복잡도·overdraw·DirectXTK 내부 draw는 전부 귀속되지 않습니다.");
        row("계측 범위", evidence.MissingCoverage, "CPU 누락 " + std::to_string(frame.DroppedCpuScopes) + " / GPU 누락 " + std::to_string(frame.DroppedGpuScopes) + " / 상세 " + (frame.DetailedCpuScopes ? "ON" : "OFF") + " / GPU 패스 " + (frame.GpuScopesSupported ? "지원" : "미지원"),
            "동일 상세 수집 조건에서 다시 수집하고 GPU 응답이 완료된 원래 프레임을 확인합니다. 부모·자식과 메인·worker를 합산하지 않습니다.",
            "미계측은 0이 아닙니다. frame 밖 scope 공백·표본 상한·unsupported GPU는 병목이 없다는 증거가 아닙니다.");
        ImGui::EndTable();
    }
    ImGui::TextWrapped("기준을 넘지 않았어도 병목이 없다는 뜻은 아닙니다. 빠른 장면(쿠크 2관문·빙고 등)과 느린 장면의 차이는 '기준 A/B'에서 구조 비교로 조사하며 동일 조건 인과성으로 단정하지 않습니다.");
}

namespace
{
    void MemoryValueCell(const Engine::FProfilerMemoryStats& sample, Client::EProfilerMemoryMetric metric)
    {
        const auto value = Client::ProfilerMemoryValue(sample, metric);
        if (value.Available) ImGui::Text("%.1f", value.MiB); else ImGui::TextDisabled("미계측");
    }

    void MemorySamplePlot(const char* id, const char* title, const char* firstName, const char* secondName,
        const std::vector<Engine::FProfilerMemoryStats>& samples, Engine::CProfiler& profiler,
        Client::EProfilerMemoryMetric firstMetric, Client::EProfilerMemoryMetric secondMetric)
    {
        ImGui::PushID(id); ImGui::TextUnformatted(title);
        if (samples.empty()) { ImGui::TextDisabled("수집된 OS 조회 표본 없음"); ImGui::PopID(); return; }
        const ImU32 firstColor = IM_COL32(91, 177, 244, 255), secondColor = IM_COL32(136, 212, 139, 255);
        ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(firstColor), "%s", firstName); ImGui::SameLine();
        ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(secondColor), "%s", secondName); ImGui::SameLine(); ImGui::TextDisabled("단위 MiB / x 실제 표본 시각 (s)");
        const auto origin = ImGui::GetCursorScreenPos();
        const float width = (std::max)(200.f, ImGui::GetContentRegionAvail().x), height = 100.f;
        const float left = origin.x + 64.f, right = origin.x + width - 8.f, top = origin.y + 7.f, bottom = origin.y + height - 22.f;
        ImGui::InvisibleButton("##MemoryChart", ImVec2(width, height)); const bool hovered = ImGui::IsItemHovered();
        auto* draw = ImGui::GetWindowDrawList();
        draw->AddRectFilled(origin, ImVec2(origin.x + width, origin.y + height), IM_COL32(25, 29, 36, 255));
        const auto firstTick = samples.front().SampleTick, lastTick = samples.back().SampleTick;
        const double seconds = (std::max)(0.001, profiler.Ticks_ToMs(lastTick - firstTick) / 1000.0);
        double maximum = 1.0; bool anyValid = false;
        for (const auto& sample : samples) for (const auto metric : {firstMetric, secondMetric})
        {
            const auto value = Client::ProfilerMemoryValue(sample, metric);
            if (value.Available) { maximum = (std::max)(maximum, value.MiB); anyValid = true; }
        }
        maximum *= 1.05;
        char number[64]; std::snprintf(number, sizeof(number), "%.0f MiB", maximum);
        draw->AddText(ImVec2(origin.x + 2.f, top), IM_COL32(175, 182, 193, 255), number);
        draw->AddText(ImVec2(origin.x + 2.f, bottom - 12.f), IM_COL32(175, 182, 193, 255), "0");
        for (int tick = 0; tick <= 4; ++tick)
        {
            const float x = left + (right - left) * float(tick) / 4.f;
            draw->AddLine(ImVec2(x, top), ImVec2(x, bottom), IM_COL32(55, 60, 72, 255));
            std::snprintf(number, sizeof(number), "%.2f", seconds * double(tick) / 4.0);
            draw->AddText(ImVec2(x, bottom + 3.f), IM_COL32(175, 182, 193, 255), number);
        }
        int nearest = -1; float nearestDistance = 9.f;
        for (int series = 0; series < 2; ++series)
        {
            bool previousValid = false; ImVec2 previous;
            for (size_t i = 0; i < samples.size(); ++i)
            {
                const auto& sample = samples[i]; const auto value = Client::ProfilerMemoryValue(sample, series ? secondMetric : firstMetric);
                const float x = left + float(profiler.Ticks_ToMs(sample.SampleTick - firstTick) / 1000.0 / seconds) * (right - left);
                if (!series && hovered && std::abs(ImGui::GetIO().MousePos.x - x) < nearestDistance)
                { nearest = static_cast<int>(i); nearestDistance = std::abs(ImGui::GetIO().MousePos.x - x); }
                if (!value.Available) { previousValid = false; continue; }
                const ImVec2 point(x, bottom - float(value.MiB / maximum) * (bottom - top));
                if (previousValid) draw->AddLine(previous, point, series ? secondColor : firstColor, 1.5f);
                draw->AddCircleFilled(point, 2.5f, series ? secondColor : firstColor);
                previous = point; previousValid = true;
            }
        }
        if (!anyValid) draw->AddText(ImVec2(left + 8.f, top + 15.f), IM_COL32(200, 180, 150, 255), "N/A: no valid samples");
        if (nearest >= 0)
        {
            const auto& sample = samples[static_cast<size_t>(nearest)];
            ImGui::BeginTooltip();
            ImGui::Text("조회 frame #%llu | PID %u | 현재 기준 %.1f ms 전", static_cast<unsigned long long>(sample.SampleFrameNumber), sample.ProcessId, sample.AgeMs);
            for (int series = 0; series < 2; ++series)
            {
                const auto value = Client::ProfilerMemoryValue(sample, series ? secondMetric : firstMetric);
                if (value.Available) ImGui::Text("%s %.1f MiB", series ? secondName : firstName, value.MiB);
                else ImGui::Text("%s 미계측", series ? secondName : firstName);
            }
            ImGui::EndTooltip();
        }
        ImGui::PopID();
    }
}

void Client::CProfilerTool::Render_Memory(Engine::CProfiler& profiler)
{
    const double now = ImGui::GetTime();
    const bool refresh = ImGui::Button("메모리 표본 다시 읽기###MemoryRefresh");
    if (refresh || m_fLastMemoryRefresh < 0.0 || now - m_fLastMemoryRefresh >= 0.5)
    {
        profiler.Get_MemorySamples(Engine::CProfiler::MAX_HISTORY_FRAMES, m_MemorySamples);
        m_fLastMemoryRefresh = now;
    }
    ImGui::SameLine(); ImGui::TextDisabled("실제 OS 조회 %zu개 / 보관 최대 %zu 프레임", m_MemorySamples.size(), Engine::CProfiler::MAX_HISTORY_FRAMES);
    ImGui::TextWrapped("Capture ON에서 main thread가 약 1Hz로 관측한 실제 조회 표본만 표시합니다. 같은 SampleFrameNumber·SampleTick·PID를 여러 프레임이 재사용해도 점은 하나입니다. 조회 실패는 0으로 연결하지 않고 차트를 끊습니다. 재조회는 보관 표본을 읽을 뿐 OS 샘플 주기를 높이지 않습니다.");
    ImGui::TextWrapped("Private commit은 프로세스 전용 커밋, Working Set은 현재 RAM 상주량(공유 가능 페이지 포함)입니다. DXGI Local/Non-local은 이 프로세스의 adapter segment 사용량과 OS 동적 예산이며 UMA의 Local을 항상 전용 VRAM으로 볼 수 없습니다. RAM·commit·GPU 값을 합산하지 마세요.");
    if (ImGui::CollapsingHeader("메모리 표본의 한계와 다음 실험"))
        ImGui::TextWrapped("main thread stall 동안 표본이 없고 순간 peak를 놓칠 수 있습니다. 선 사이를 실제 연속 사용량으로 단정하지 마세요. 프로세스 lifetime peak는 이 capture의 peak가 아닙니다. 100~250ms 로딩 phase tracker·allocation callstack·resource별 bytes/owner·residency/page fault·해제 수명은 미지원입니다. 같은 프로세스에서 Lobby 안정 → 입장 → 안정 → 퇴장/재진입의 잔존량을 비교하고, 증가만으로 누수/OOM을 확정하지 마세요. 미제출/culling은 자원 메모리 해제를 뜻하지 않습니다.");
    if (m_MemorySamples.empty()) { ImGui::TextDisabled("완료 프레임에 귀속된 OS 조회 표본이 없습니다."); return; }
    const auto& latest = m_MemorySamples.back();
    ImGui::Text("최신 조회 frame #%llu | PID %u | 현재 기준 %.1f ms 전", static_cast<unsigned long long>(latest.SampleFrameNumber), latest.ProcessId, latest.AgeMs);
    if (latest.AdapterIdentityValid) ImGui::Text("Adapter LUID %08x:%08x / node %u", static_cast<unsigned>(latest.AdapterLuidHigh), latest.AdapterLuidLow, latest.AdapterNodeIndex);
    else ImGui::TextDisabled("Adapter identity 미계측");
    if (ImGui::BeginTable("##MemoryLatest", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH))
    {
        const auto row = [&](const char* label, EProfilerMemoryMetric metric)
        { ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(label); ImGui::TableNextColumn(); MemoryValueCell(latest, metric); };
        ImGui::TableSetupColumn("최신 OS 조회 항목"); ImGui::TableSetupColumn("MiB (실패는 미계측)"); ImGui::TableHeadersRow();
        row("프로세스 전용 commit", EProfilerMemoryMetric::PrivateCommit); row("프로세스 현재 Working Set", EProfilerMemoryMetric::WorkingSet);
        row("프로세스 lifetime 최대 전용 commit", EProfilerMemoryMetric::PeakPrivateCommit); row("프로세스 lifetime 최대 Working Set", EProfilerMemoryMetric::PeakWorkingSet);
        row("시스템 전체 commit", EProfilerMemoryMetric::SystemCommit); row("시스템 commit 한도", EProfilerMemoryMetric::SystemLimit); row("시스템 가용 RAM", EProfilerMemoryMetric::SystemAvailable);
        row("DXGI Local 프로세스 사용량", EProfilerMemoryMetric::LocalUsage); row("DXGI Local 동적 예산", EProfilerMemoryMetric::LocalBudget);
        row("DXGI Non-local 프로세스 사용량", EProfilerMemoryMetric::NonLocalUsage); row("DXGI Non-local 동적 예산", EProfilerMemoryMetric::NonLocalBudget);
        ImGui::EndTable();
    }
    if (ImGui::CollapsingHeader("실제 조회 표본의 시간 변화", ImGuiTreeNodeFlags_DefaultOpen))
    {
        MemorySamplePlot("Process", "프로세스 RAM·commit", "전용 commit", "Working Set", m_MemorySamples, profiler, EProfilerMemoryMetric::PrivateCommit, EProfilerMemoryMetric::WorkingSet);
        MemorySamplePlot("Local", "DXGI Local segment", "프로세스 사용량", "동적 예산", m_MemorySamples, profiler, EProfilerMemoryMetric::LocalUsage, EProfilerMemoryMetric::LocalBudget);
        MemorySamplePlot("NonLocal", "DXGI Non-local segment", "프로세스 사용량", "동적 예산", m_MemorySamples, profiler, EProfilerMemoryMetric::NonLocalUsage, EProfilerMemoryMetric::NonLocalBudget);
        MemorySamplePlot("System", "시스템 commit", "전체 commit", "commit 한도", m_MemorySamples, profiler, EProfilerMemoryMetric::SystemCommit, EProfilerMemoryMetric::SystemLimit);
    }
    if (ImGui::CollapsingHeader("표본별 값·실패 확인"))
        if (ImGui::BeginTable("##MemorySampleRows", 8, TABLE_FLAGS | ImGuiTableFlags_ScrollX, ImVec2(0.f, 220.f), 1250.f))
        {
            for (const char* label : {"OS 조회 frame", "현재 나이 ms", "private MiB", "WS MiB", "Local 사용 MiB", "Local 예산 MiB", "시스템 commit MiB", "시스템 가용 MiB"}) ImGui::TableSetupColumn(label);
            ImGui::TableSetupScrollFreeze(1, 1); ImGui::TableHeadersRow();
            ImGuiListClipper clipper; clipper.Begin(static_cast<int>(m_MemorySamples.size()));
            while (clipper.Step()) for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
            {
                const auto& sample = m_MemorySamples[static_cast<size_t>(i)];
                ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::Text("%llu", static_cast<unsigned long long>(sample.SampleFrameNumber));
                ImGui::TableNextColumn(); ImGui::Text("%.1f", sample.AgeMs);
                for (const auto metric : {EProfilerMemoryMetric::PrivateCommit, EProfilerMemoryMetric::WorkingSet, EProfilerMemoryMetric::LocalUsage,
                    EProfilerMemoryMetric::LocalBudget, EProfilerMemoryMetric::SystemCommit, EProfilerMemoryMetric::SystemAvailable})
                { ImGui::TableNextColumn(); MemoryValueCell(sample, metric); }
            }
            ImGui::EndTable();
        }
}
```

### G14-3. 정적 배경의 미세 LOD 보존과 원경 LOD 생성

설치 Bern의 실제 CModel decoder·0.01 preTransform·normal 정규화와 현재 캐시 해시를
사용해1,755개 고유 geometry/submesh를 조사했다. 현재 재질 적격840개 중811개는
4,096triangle 하한에 제외되고 나머지29개 중23개는 생성 감소율15%를 만족하지 못했다.
확인된 축소 LOD는6개이며 현재 데이터의 첫 material variant로 인한 추가 차단은0개다.

하한만256으로 낮춰도 생성은10개에 그쳤으므로 그것만으로 완료하지 않는다. 후보는
하한512triangle, 기존 UV100·오차0.001/0.002의 미세 두 단계 보존, UV10·오차0.01/0.02의
원경 두 단계 추가다. 최대5range(원본 포함)를 사용하고 캐시는v3으로 분리한다.
보더 고정, 원본 vertex/normal/UV/buffer·재질은 유지하며 새 index만 생성한다. 각 단계는
이전 단계보다15% 이상 index가 감소할 때만 보관한다. 생성 오차는 선택 시 투영되는
화면 오차와 별개이며 기존 Select_Range 검증을 그대로 소비한다.

Bern frame provider를 가진 batch만 화면 오차1pixel(10% 안전 여유 적용 후0.9pixel)을
사용하며 다른 맵은0.25pixel을 유지한다. Mesh profiler는 확장 level을 기존LOD2+로
집계하고 원본/제출 index를 실제 draw의 선택값으로 기록한다. 카메라·저작 재질·shader
변경은 없다. 기존 소스4개만 바꾸므로 project/filter 추가는 없다.

후보의 실제 Create/Select_Range와 D3D index buffer, 정상·실패·근거리·기존 단계 보존을
화면 없는 native probe에서 검증한 뒤 반영한다. 최종 설치 Debug/Release DLL에서도
검증하고 생성 개수·제출 index·GPU API 확인과 실제 게임 FPS를 RESULT에서 구분한다.

#### Engine/Private/StaticMeshLod.h 전체 반영 코드

```cpp
#pragma once

#include "MeshLod.h"
#include "Engine_VertexTypes.h"
#include <array>
#include <span>

NS_BEGIN(Engine)

// Additional immutable index ranges share the original vertex buffer. The
// original mesh's own index buffer remains the default and shadow path.
class CStaticMeshLod final
{
public:
    // 중간 크기 건물도 원경 LOD를 준비하되 원본 정점/재질과 보더는 유지한다.
    static constexpr size_t MINIMUM_INDICES = 512u * 3u;
    static constexpr size_t MAXIMUM_RANGES = 5u;
    struct SELECTION final
    {
        uint32_t indexCount = 0u;
        uint32_t firstIndex = 0u;
        uint32_t level = 0u;
    };

    static HRESULT Create(ID3D11Device* device, std::span<const VTXMESH> vertices,
        std::span<const uint32_t> indices, std::shared_ptr<CStaticMeshLod>& result);
    HRESULT Select_Range(const MESH_SCREEN_LOD_DESC& view, SELECTION& result) const;
    ID3D11Buffer* Get_IndexBuffer() const { return m_IndexBuffer.Get(); }

private:
    struct RANGE final { uint32_t count = 0u, first = 0u; f32_t error = 0.f; };
    std::array<RANGE, MAXIMUM_RANGES> m_Ranges{};
    uint32_t m_RangeCount = 0u;
    ComPtr<ID3D11Buffer> m_IndexBuffer;
};

NS_END
```

#### Engine/Private/StaticMeshLod.cpp 전체 반영 코드

```cpp
#include "../ThirdPartyLib/meshoptimizer/meshoptimizer.h"
#include "Engine_VertexTypes.h"
#include "StaticMeshLod.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#pragma push_macro("new")
#undef new
#include <bcrypt.h>
#include <filesystem>
#include <fstream>
#pragma pop_macro("new")
#pragma comment(lib, "bcrypt.lib")

NS_BEGIN(Engine)

namespace
{
    // This is disposable derived data, never the authority for model loading.
    // Bump both the version and algorithm tag when the simplification changes.
    struct LOD_CACHE_RANGE final { uint32_t count = 0u, first = 0u; float error = 0.f; };
    struct LOD_CACHE_HEADER final
    {
        uint32_t magic = 0x33444f4cu, version = 3u;
        uint32_t vertices = 0u, sourceIndices = 0u, rangeCount = 1u, combinedCount = 0u;
        std::array<LOD_CACHE_RANGE, CStaticMeshLod::MAXIMUM_RANGES> ranges{};
        std::array<uint8_t, 32> input{}, output{};
    };
    static_assert(sizeof(LOD_CACHE_HEADER) == 148u);

    class LodHash final
    {
    public:
        LodHash()
        {
            m_Valid = BCryptOpenAlgorithmProvider(&m_Algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) >= 0 &&
                BCryptCreateHash(m_Algorithm, &m_Hash, nullptr, 0, nullptr, 0, 0) >= 0;
        }
        ~LodHash()
        {
            if (m_Hash) BCryptDestroyHash(m_Hash);
            if (m_Algorithm) BCryptCloseAlgorithmProvider(m_Algorithm, 0);
        }
        void Add(const void* data, size_t bytes)
        {
            if (!m_Valid || bytes == 0u) return;
            if (!data || bytes > ULONG_MAX) { m_Valid = false; return; }
            m_Valid = BCryptHashData(m_Hash, static_cast<PUCHAR>(const_cast<void*>(data)),
                static_cast<ULONG>(bytes), 0) >= 0;
        }
        bool Finish(std::array<uint8_t, 32>& digest)
        {
            return m_Valid && BCryptFinishHash(m_Hash, digest.data(), static_cast<ULONG>(digest.size()), 0) >= 0;
        }
    private:
        BCRYPT_ALG_HANDLE m_Algorithm = nullptr;
        BCRYPT_HASH_HANDLE m_Hash = nullptr;
        bool m_Valid = false;
    };

    bool LodInputHash(std::span<const VTXMESH> vertices, std::span<const uint32_t> indices,
        std::array<uint8_t, 32>& digest)
    {
        // VTXMESH contains exactly eighteen float channels and one color uint;
        // these layout assertions exclude padding from the hashed input bytes.
        static_assert(sizeof(VTXMESH) == 18u * sizeof(float) + sizeof(uint32_t));
        static_assert(offsetof(VTXMESH, color0Rgba8) == 64u && offsetof(VTXMESH, vTexcoord2) == 68u);
        constexpr char algorithm[] = "StaticLod-v3-meshopt73583c335e541c139821d0de2bf5f12960a04941-attrs19-uv100fine-uv10far-border-.001-.002-.01-.02-.55-.30-.85";
        const uint64_t counts[] = { vertices.size(), indices.size() };
        LodHash hash;
        hash.Add(algorithm, sizeof(algorithm));
        hash.Add(counts, sizeof(counts));
        hash.Add(vertices.data(), vertices.size_bytes());
        hash.Add(indices.data(), indices.size_bytes());
        return hash.Finish(digest);
    }

    std::filesystem::path LodCachePath(const std::array<uint8_t, 32>& digest)
    {
        wchar_t local[MAX_PATH]{};
        const DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", local, MAX_PATH);
        if (!length || length >= MAX_PATH) return {};
        constexpr char hex[] = "0123456789abcdef";
        std::string name;
        name.reserve(68u);
        for (uint8_t byte : digest) { name += hex[byte >> 4u]; name += hex[byte & 15u]; }
        return std::filesystem::path(local) / L"LostArk" / L"StaticMeshLod" / L"v3" / (name + ".bin");
    }

    bool LodOutputHash(LOD_CACHE_HEADER header, std::span<const uint32_t> indices,
        std::array<uint8_t, 32>& digest)
    {
        header.output = {};
        LodHash hash;
        hash.Add(&header, sizeof(header));
        hash.Add(indices.data(), indices.size_bytes());
        return hash.Finish(digest);
    }

    bool LoadLodCache(const std::filesystem::path& path, const std::array<uint8_t, 32>& input,
        size_t vertexCount, std::span<const uint32_t> source, LOD_CACHE_HEADER& header,
        std::vector<uint32_t>& combined)
    {
        if (path.empty()) return false;
        try
        {
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file) return false;
            const auto bytes = file.tellg();
            file.seekg(0);
            LOD_CACHE_HEADER staged{};
            if (!file.read(reinterpret_cast<char*>(&staged), sizeof(staged)) || staged.magic != 0x33444f4cu ||
                staged.version != 3u || staged.input != input || staged.vertices != vertexCount ||
                staged.sourceIndices != source.size() || staged.rangeCount < 1u || staged.rangeCount > CStaticMeshLod::MAXIMUM_RANGES ||
                staged.combinedCount > source.size() * CStaticMeshLod::MAXIMUM_RANGES ||
                uint64_t(bytes) != sizeof(staged) + uint64_t(staged.combinedCount) * sizeof(uint32_t)) return false;
            if (staged.ranges[0].count != source.size() || staged.ranges[0].first != 0u ||
                staged.ranges[0].error != 0.f || (staged.rangeCount == 1u && staged.combinedCount != 0u)) return false;
            size_t end = staged.rangeCount == 1u ? 0u : source.size();
            for (uint32_t range = 1u; range < staged.rangeCount; ++range)
            {
                const auto& current = staged.ranges[range];
                if (!current.count || current.count % 3u || current.first != end ||
                    current.count >= staged.ranges[range - 1u].count * .85 ||
                    !std::isfinite(current.error) || current.error < 0.f) return false;
                end += current.count;
            }
            if (end != staged.combinedCount) return false;
            std::vector<uint32_t> payload(staged.combinedCount);
            if (!file.read(reinterpret_cast<char*>(payload.data()), payload.size() * sizeof(uint32_t))) return false;
            if (std::any_of(payload.begin(), payload.end(), [&](uint32_t index) { return index >= vertexCount; }) ||
                (!payload.empty() && !std::equal(source.begin(), source.end(), payload.begin()))) return false;
            std::array<uint8_t, 32> output{};
            if (!LodOutputHash(staged, payload, output) || output != staged.output) return false;
            header = staged;
            combined = std::move(payload);
            return true;
        }
        catch (const std::exception&) { return false; }
    }

    void SaveLodCache(const std::filesystem::path& path, LOD_CACHE_HEADER header,
        std::span<const uint32_t> combined)
    {
        if (path.empty()) return;
        try
        {
            if (!LodOutputHash(header, combined, header.output)) return;
            std::error_code error;
            std::filesystem::create_directories(path.parent_path(), error);
            if (error) return;
            static volatile LONG serial = 0;
            auto temporary = path;
            temporary += L"." + std::to_wstring(GetCurrentProcessId()) + L"." +
                std::to_wstring(InterlockedIncrement(&serial)) + L".tmp";
            {
                std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
                if (!file) return;
                file.write(reinterpret_cast<const char*>(&header), sizeof(header));
                file.write(reinterpret_cast<const char*>(combined.data()), combined.size_bytes());
                file.flush();
                if (!file) { file.close(); std::filesystem::remove(temporary, error); return; }
            }
            if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING))
                std::filesystem::remove(temporary, error);
        }
        catch (const std::exception&) { /* Optional cache failure keeps the generated LOD usable. */ }
    }
}

HRESULT CStaticMeshLod::Create(ID3D11Device* device, std::span<const VTXMESH> vertices,
    std::span<const uint32_t> indices, std::shared_ptr<CStaticMeshLod>& result)
{
    result.reset();
    // Keep generation bounded at asset load time. Selection and drawing reuse
    // these immutable ranges without per-draw compute work or GPU readback.
    constexpr size_t minimumIndices = MINIMUM_INDICES;
    constexpr size_t maximumIndices = 3u * 1024u * 1024u;
    if (!device || vertices.empty() || indices.size() % 3u != 0u) return E_INVALIDARG;
    if (indices.size() < minimumIndices || indices.size() > maximumIndices ||
        vertices.size() > 1024u * 1024u || device->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0)
        return S_FALSE;
    try
    {
        // Validate even on cache hits: invalid source data never becomes a LOD.
        for (const auto& v : vertices)
        {
            const float channels[] = { v.vPosition.x, v.vPosition.y, v.vPosition.z,
                v.vNormal.x, v.vNormal.y, v.vNormal.z, v.vTangent.x, v.vTangent.y, v.vTangent.z,
                v.vBinormal.x, v.vBinormal.y, v.vBinormal.z, v.vTexcoord.x, v.vTexcoord.y,
                v.vTexcoord1.x, v.vTexcoord1.y, v.vTexcoord2.x, v.vTexcoord2.y };
            if (std::any_of(std::begin(channels), std::end(channels), [](float f) { return !std::isfinite(f); }))
                return E_INVALIDARG;
        }
        if (std::any_of(indices.begin(), indices.end(), [&](uint32_t i) { return i >= vertices.size(); }))
            return E_INVALIDARG;
        std::array<uint8_t, 32> input{};
        std::filesystem::path cachePath;
        if (LodInputHash(vertices, indices, input))
        {
            try { cachePath = LodCachePath(input); }
            catch (const std::exception&) { /* Cache discovery is optional. */ }
        }
        LOD_CACHE_HEADER cached{};
        std::vector<uint32_t> combined;
        const bool cacheHit = LoadLodCache(cachePath, input, vertices.size(), indices, cached, combined);
        if (cacheHit && cached.rangeCount == 1u) return S_FALSE;
        auto staged = std::make_shared<CStaticMeshLod>();
        if (cacheHit)
        {
            staged->m_RangeCount = cached.rangeCount;
            for (uint32_t i = 0u; i < cached.rangeCount; ++i)
                staged->m_Ranges[i] = { cached.ranges[i].count, cached.ranges[i].first, cached.ranges[i].error };
        }
        else
        {
            // All source shading channels participate in the error metric. The
            // simplifier keeps the original vertices and locks mesh boundaries.
            constexpr size_t attributeCount = 19u;
            float weights[attributeCount] = {
                1.f, 1.f, 1.f, .5f, .5f, .5f, .5f, .5f, .5f,
                100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 1.f, 1.f, 1.f, 1.f };
            std::vector<std::array<float, attributeCount>> attributes(vertices.size());
            for (size_t i = 0; i < vertices.size(); ++i)
            {
                const auto& v = vertices[i];
                auto& a = attributes[i];
                a = { v.vNormal.x, v.vNormal.y, v.vNormal.z,
                    v.vTangent.x, v.vTangent.y, v.vTangent.z,
                    v.vBinormal.x, v.vBinormal.y, v.vBinormal.z,
                    v.vTexcoord.x, v.vTexcoord.y, v.vTexcoord1.x, v.vTexcoord1.y,
                    v.vTexcoord2.x, v.vTexcoord2.y,
                    float(v.color0Rgba8 & 255u) / 255.f,
                    float((v.color0Rgba8 >> 8u) & 255u) / 255.f,
                    float((v.color0Rgba8 >> 16u) & 255u) / 255.f,
                    float((v.color0Rgba8 >> 24u) & 255u) / 255.f };
            }
            const float scale = meshopt_simplifyScale(&vertices[0].vPosition.x, vertices.size(), sizeof(VTXMESH));
            if (!std::isfinite(scale) || scale <= 0.f) return S_FALSE;
            staged->m_Ranges[0] = { static_cast<uint32_t>(indices.size()), 0u, 0.f };
            staged->m_RangeCount = 1u;
            combined.assign(indices.begin(), indices.end());
            std::vector<uint32_t> reduced(indices.size());
            // 기존 UV100의 미세 두 단계 생성 결과를 먼저 보관한다.
            // 원경 단계만 UV 가중치10과 더 큰 생성 오차를 허용한다. 실제 사용은 아래
            // Select_Range의 화면 오차 검사로 제한하며 vertex/UV/normal 자체는 바꾸지 않는다.
            constexpr float errors[] = { .001f, .002f, .01f, .02f };
            for (uint32_t step = 0u; step < MAXIMUM_RANGES - 1u; ++step)
            {
                if (step == 2u)
                    for (size_t channel = 9u; channel < 15u; ++channel) weights[channel] = 10.f;
                const float fraction = (step & 1u) == 0u ? .55f : .30f;
                const size_t target = (static_cast<size_t>(indices.size() * fraction) / 3u) * 3u;
                float error = 0.f;
                const size_t count = meshopt_simplifyWithAttributes(reduced.data(), indices.data(), indices.size(),
                    &vertices[0].vPosition.x, vertices.size(), sizeof(VTXMESH),
                    attributes[0].data(), sizeof(attributes[0]), weights, attributeCount, nullptr,
                    target, errors[step], meshopt_SimplifyLockBorder, &error);
                if (count == 0u || count % 3u != 0u || !std::isfinite(error) || error < 0.f ||
                    count >= staged->m_Ranges[staged->m_RangeCount - 1u].count * .85)
                    continue;
                if (std::any_of(reduced.begin(), reduced.begin() + count,
                    [&](uint32_t i) { return i >= vertices.size(); })) return E_UNEXPECTED;
                staged->m_Ranges[staged->m_RangeCount++] = {
                    static_cast<uint32_t>(count), static_cast<uint32_t>(combined.size()), error * scale };
                combined.insert(combined.end(), reduced.begin(), reduced.begin() + count);
            }
            cached.vertices = static_cast<uint32_t>(vertices.size());
            cached.sourceIndices = static_cast<uint32_t>(indices.size());
            cached.rangeCount = staged->m_RangeCount;
            cached.input = input;
            for (uint32_t i = 0u; i < staged->m_RangeCount; ++i)
                cached.ranges[i] = { staged->m_Ranges[i].count, staged->m_Ranges[i].first, staged->m_Ranges[i].error };
            if (staged->m_RangeCount == 1u) combined.clear();
            cached.combinedCount = static_cast<uint32_t>(combined.size());
            SaveLodCache(cachePath, cached, combined);
            if (staged->m_RangeCount == 1u) return S_FALSE;
        }
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = static_cast<UINT>(combined.size() * sizeof(uint32_t));
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        D3D11_SUBRESOURCE_DATA initial{};
        initial.pSysMem = combined.data();
        const HRESULT hr = device->CreateBuffer(&desc, &initial, &staged->m_IndexBuffer);
        if (FAILED(hr)) return hr;
        result = std::move(staged);
        return S_OK;
    }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
    catch (const std::length_error&) { return E_OUTOFMEMORY; }
}

HRESULT CStaticMeshLod::Select_Range(const MESH_SCREEN_LOD_DESC& view,
    SELECTION& result) const
{
    const float values[] = { view.viewBounds.x, view.viewBounds.y, view.viewBounds.z,
        view.viewBounds.w, view.projectionPixels.x, view.projectionPixels.y,
        view.maximumScale, view.nearPlane, view.maximumPixelError };
    if (!m_IndexBuffer || m_RangeCount < 2u ||
        std::any_of(std::begin(values), std::end(values), [](float f) { return !std::isfinite(f); }) ||
        view.viewBounds.w <= 0.f || view.projectionPixels.x <= 0.f || view.projectionPixels.y <= 0.f ||
        view.maximumScale <= 0.f || view.nearPlane <= 0.f ||
        view.maximumPixelError <= 0.f || view.maximumPixelError > 1.f)
        return E_INVALIDARG;

    if (view.hasTightViewBounds &&
        (!std::isfinite(view.maximumAbsViewXY.x) || !std::isfinite(view.maximumAbsViewXY.y) ||
         !std::isfinite(view.minimumViewDepth) || view.maximumAbsViewXY.x < 0.f || view.maximumAbsViewXY.y < 0.f))
        return E_INVALIDARG;

    uint32_t selected = 0u;
    const double nearestZ = view.hasTightViewBounds ? view.minimumViewDepth :
        double(view.viewBounds.z) - view.viewBounds.w;
    if (nearestZ > view.nearPlane)
    {
        const double largestX = view.hasTightViewBounds ? view.maximumAbsViewXY.x :
            std::abs(double(view.viewBounds.x)) + view.viewBounds.w;
        const double largestY = view.hasTightViewBounds ? view.maximumAbsViewXY.y :
            std::abs(double(view.viewBounds.y)) + view.viewBounds.w;
        for (uint32_t level = 1u; level < m_RangeCount; ++level)
        {
            const double error = double(m_Ranges[level].error) * view.maximumScale;
            const double safeZ = nearestZ - error;
            if (safeZ <= view.nearPlane) continue;
            // Same conservative off-axis bound and 10% margin as the former
            // compute selector. All inputs already live on the render thread.
            const double pixelX = view.projectionPixels.x * error * (1. + largestX / nearestZ) / safeZ;
            const double pixelY = view.projectionPixels.y * error * (1. + largestY / nearestZ) / safeZ;
            if (std::isfinite(pixelX) && std::isfinite(pixelY) &&
                (std::max)(pixelX, pixelY) <= double(view.maximumPixelError) * .9)
                selected = level;
        }
    }
    result = { m_Ranges[selected].count, m_Ranges[selected].first, selected };
    return S_OK;
}

NS_END
```

#### Engine/Private/Mesh.cpp 전체 반영 코드

```cpp
#include "Mesh.h"
#pragma push_macro("new")
#undef new
#include "Assimp/scene.h"
#pragma pop_macro("new")
#include "Engine_VertexTypes.h"
#include "BinaryAsset/ModelAssetData.h"
#include "Bone.h"

#include "Shader.h"
#include "GameInstance.h"
#include "Engine_RenderTypes.h"
#include "Profiler.h"
#include "StaticMeshLod.h"
#include <algorithm>
#include <cmath>
#include <limits>

CMesh::CMesh(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CVIBuffer { pDevice, pContext }
{
}

CMesh::~CMesh()
{
}

HRESULT CMesh::Initialize_Prototype(MODEL eType, const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones, fmatrix_t PreTransformMatrix)
{
#pragma region PUBLIC_DATA
	strcpy_s(m_szName, pAIMesh->mName.C_Str());
	m_iMaterialIndex = pAIMesh->mMaterialIndex;
	m_iVertexStride = MODEL::NONANIM == eType ? sizeof(VTXMESH) : sizeof(VTXANIMMESH);
	m_iNumVertices = pAIMesh->mNumVertices;
	m_iIndexStride = 4;
	m_iNumIndices = pAIMesh->mNumFaces * 3;
	m_iNumVertexBuffers = 1;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
#pragma endregion

#pragma region VERTEX_BUFFER
	HRESULT hr = MODEL::NONANIM == eType ?
		Ready_VertexBuffer_NonAnim(pAIMesh, PreTransformMatrix) :
		Ready_VertexBuffer_Anim(pAIMesh, Bones);

	if (FAILED(hr))
		return E_FAIL;
#pragma endregion

#pragma region INDEX_BUFFER
	D3D11_BUFFER_DESC		IndexBufferDesc{};
	IndexBufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = 0;
	IndexBufferDesc.MiscFlags = 0;
	IndexBufferDesc.StructureByteStride = m_iIndexStride;

	unique_ptr<uint32_t[]>		pIndices = make_unique<uint32_t[]>(m_iNumIndices);

	uint32_t	iNumIndices = {};

	for (uint32_t i = 0; i < pAIMesh->mNumFaces; i++)
	{
		pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[0];
		pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[1];
		pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[2];
	}

	D3D11_SUBRESOURCE_DATA			IndexInitialData{};
	IndexInitialData.pSysMem = pIndices.get();

	if (FAILED(m_pDevice->CreateBuffer(&IndexBufferDesc, &IndexInitialData, &m_pIB)))
		return E_FAIL;

#pragma endregion

	return S_OK;
}

HRESULT CMesh::Initialize_Prototype(MODEL eType, const MODEL_MESH_DATA& mesh,
	const MODEL_SKELETON_DATA& skeleton, fmatrix_t PreTransformMatrix, bool_t preserveStaticVertices, bool_t buildPickingGeometry)
{
	const bool_t isAnimated = MODEL::ANIM == eType;
	if ((preserveStaticVertices && isAnimated) || mesh.name.size() >= MAX_PATH || mesh.indices.empty() ||
		(isAnimated && mesh.vertexKind != MODEL_VERTEX_KIND::SKINNED) ||
		(!isAnimated && mesh.vertexKind != MODEL_VERTEX_KIND::STATIC))
		return E_FAIL;

	strcpy_s(m_szName, mesh.name.c_str());
	m_iMaterialIndex = mesh.materialIndex;
	m_iVertexStride = isAnimated ? sizeof(VTXANIMMESH) : sizeof(VTXMESH);
	m_iNumVertices = isAnimated
		? static_cast<uint32_t>(mesh.skinnedVertices.size())
		: static_cast<uint32_t>(mesh.vertices.size());
	m_iIndexStride = sizeof(uint32_t);
	m_iNumIndices = static_cast<uint32_t>(mesh.indices.size());
	m_iNumVertexBuffers = 1;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	if (0 == m_iNumVertices)
		return E_FAIL;

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.StructureByteStride = m_iVertexStride;
	D3D11_SUBRESOURCE_DATA vertexInitialData{};

	vector<VTXMESH> staticVertices;
	if (isAnimated)
	{
		vertexInitialData.pSysMem = mesh.skinnedVertices.data();
		if (skeleton.bones.empty() || skeleton.bones.size() > 512)
			return E_FAIL;

		m_bUsesSkeletonPalette = true;
		m_iNumBones = static_cast<uint32_t>(skeleton.bones.size());
		m_BoneIndices.reserve(m_iNumBones);
		m_OffsetMatrices.reserve(m_iNumBones);
		for (uint32_t i = 0; i < m_iNumBones; ++i)
		{
			m_BoneIndices.push_back(i);
			m_OffsetMatrices.push_back(skeleton.bones[i].inverseBind);
		}
        Prepare_BoneVertexBounds(mesh);
	}
	else
	{
		if ((mesh.hasColor0 && mesh.color0Rgba8.size() != mesh.vertices.size()) ||
			(!mesh.hasColor0 && !mesh.color0Rgba8.empty()))
			return E_INVALIDARG;
		staticVertices = mesh.vertices;
		for (size_t index = 0; index < staticVertices.size(); ++index)
		{
			VTXMESH& vertex = staticVertices[index];
			// The WModel reader already decoded RGBA; upload without a second swizzle.
			vertex.color0Rgba8 = mesh.hasColor0 ? mesh.color0Rgba8[index] : 0xffffffffu;
            if (preserveStaticVertices) continue;
			XMStoreFloat3(&vertex.vPosition,
				XMVector3TransformCoord(XMLoadFloat3(&vertex.vPosition), PreTransformMatrix));
			XMStoreFloat3(&vertex.vNormal,
				XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&vertex.vNormal), PreTransformMatrix)));
			XMStoreFloat3(&vertex.vTangent,
				XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&vertex.vTangent), PreTransformMatrix)));
			XMStoreFloat3(&vertex.vBinormal,
				XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&vertex.vBinormal), PreTransformMatrix)));
		}
		vertexInitialData.pSysMem = staticVertices.data();
	}

	if (FAILED(m_pDevice->CreateBuffer(&vertexBufferDesc, &vertexInitialData, &m_pVB)))
		return E_FAIL;

	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.StructureByteStride = m_iIndexStride;
	D3D11_SUBRESOURCE_DATA indexInitialData{};
	indexInitialData.pSysMem = mesh.indices.data();
	if (FAILED(m_pDevice->CreateBuffer(&indexBufferDesc, &indexInitialData, &m_pIB)))
		return E_FAIL;

    // Derived clusters retain the original placements for picking. Avoid a
    // second CPU geometry copy and BVH for presentation-only aggregate buffers.
    if (!buildPickingGeometry) return S_OK;

    auto pick = make_shared<PICK_GEOMETRY>();
    pick->skinned = isAnimated;
    pick->indices = mesh.indices;
    pick->vertices.reserve(m_iNumVertices);
    if (isAnimated)
        for (const auto& vertex : mesh.skinnedVertices)
            pick->vertices.push_back({vertex.vPosition, vertex.vBlendIndices, vertex.vBlendWeights});
    else
        for (const auto& vertex : staticVertices)
            pick->vertices.push_back({vertex.vPosition, {}, {}});
    if (!isAnimated) Prepare_StaticPickGeometry(*pick);
    m_PickGeometry = std::move(pick);
	return S_OK;
}

void CMesh::Prepare_StaticPickGeometry(PICK_GEOMETRY& geometry)
{
    if (geometry.skinned) return;
    geometry.triangles.clear();
    geometry.nodes.clear();
    geometry.triangles.reserve(geometry.indices.size() / 3u);
    for (size_t index = 0u; index + 2u < geometry.indices.size(); index += 3u)
    {
        bool valid = true;
        for (size_t lane = 0u; lane < 3u; ++lane)
        {
            const uint32_t vertex = geometry.indices[index + lane];
            if (vertex >= geometry.vertices.size()) { valid = false; break; }
            const auto& point = geometry.vertices[vertex].position;
            if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z))
            { valid = false; break; }
        }
        if (valid) geometry.triangles.push_back(static_cast<uint32_t>(index / 3u));
    }
    if (geometry.triangles.empty()) return;
    geometry.nodes.reserve(2u * (geometry.triangles.size() / 4u + 1u));
    const auto build = [&](const auto& self, const uint32_t first, const uint32_t count) -> void
    {
        const uint32_t nodeIndex = static_cast<uint32_t>(geometry.nodes.size());
        geometry.nodes.emplace_back();
        vector_t minimum = XMVectorReplicate((std::numeric_limits<float>::max)());
        vector_t maximum = -minimum;
        for (uint32_t ordinal = first; ordinal < first + count; ++ordinal)
            for (uint32_t lane = 0u; lane < 3u; ++lane)
            {
                const auto vertex = geometry.indices[geometry.triangles[ordinal] * 3u + lane];
                const vector_t point = XMLoadFloat3(&geometry.vertices[vertex].position);
                minimum = XMVectorMin(minimum, point); maximum = XMVectorMax(maximum, point);
            }
        XMStoreFloat3(&geometry.nodes[nodeIndex].minimum, minimum);
        XMStoreFloat3(&geometry.nodes[nodeIndex].maximum, maximum);
        if (count <= 8u)
        {
            geometry.nodes[nodeIndex].first = first;
            geometry.nodes[nodeIndex].count = count;
        }
        else
        {
            float3_t extent;
            XMStoreFloat3(&extent, maximum - minimum);
            const uint32_t axis = extent.x >= extent.y && extent.x >= extent.z ? 0u :
                (extent.y >= extent.z ? 1u : 2u);
            const auto centroid = [&](const uint32_t triangle)
            {
                double value = 0.0;
                for (uint32_t lane = 0u; lane < 3u; ++lane)
                {
                    const auto& point = geometry.vertices[geometry.indices[triangle * 3u + lane]].position;
                    value += axis == 0u ? point.x : (axis == 1u ? point.y : point.z);
                }
                return value;
            };
            const uint32_t half = count / 2u;
            std::nth_element(geometry.triangles.begin() + first,
                geometry.triangles.begin() + first + half, geometry.triangles.begin() + first + count,
                [&](const uint32_t left, const uint32_t right)
                {
                    const double a = centroid(left), b = centroid(right);
                    return a < b || (a == b && left < right);
                });
            self(self, first, half);
            self(self, first + half, count - half);
        }
        geometry.nodes[nodeIndex].end = static_cast<uint32_t>(geometry.nodes.size());
    };
    build(build, 0u, static_cast<uint32_t>(geometry.triangles.size()));
}

bool_t CMesh::Try_PickStaticLocal(fvector_t origin, fvector_t direction,
    const f32_t maxDistance, const f32_t cullSign, f32_t& distance) const
{
    if (m_hasUniqueVertexBuffer || !m_PickGeometry || m_PickGeometry->skinned ||
        m_PickGeometry->nodes.empty()) return false;
    const auto& geometry = *m_PickGeometry;
    float closest = maxDistance;
    bool hit = false;
    for (uint32_t nodeIndex = 0u; nodeIndex < geometry.nodes.size();)
    {
        const auto& node = geometry.nodes[nodeIndex];
        BoundingBox bounds;
        BoundingBox::CreateFromPoints(bounds, XMLoadFloat3(&node.minimum), XMLoadFloat3(&node.maximum));
        float entry = 0.f;
        if (!bounds.Intersects(origin, direction, entry) || entry > closest)
        { nodeIndex = node.end; continue; }
        for (uint32_t ordinal = node.first; ordinal < node.first + node.count; ++ordinal)
        {
            const uint32_t triangle = geometry.triangles[ordinal] * 3u;
            const vector_t a = XMLoadFloat3(&geometry.vertices[geometry.indices[triangle]].position);
            const vector_t b = XMLoadFloat3(&geometry.vertices[geometry.indices[triangle + 1u]].position);
            const vector_t c = XMLoadFloat3(&geometry.vertices[geometry.indices[triangle + 2u]].position);
            // D3D's default CW front faces have normals toward the viewer:
            // dot(normal, incoming ray) is negative in the LH view basis.
            if (cullSign != 0.f && XMVectorGetX(XMVector3Dot(
                XMVector3Cross(b - a, c - a), direction)) * cullSign >= 0.f) continue;
            float candidate;
            if (TriangleTests::Intersects(origin, direction, a, b, c, candidate) && candidate <= closest)
            { closest = candidate; hit = true; }
        }
        ++nodeIndex;
    }
    if (hit) distance = closest;
    return hit;
}

HRESULT CMesh::Initialize(void* pArg)
{
	return S_OK;
}

void CMesh::Prepare_BoneVertexBounds(const MODEL_MESH_DATA& mesh)
{
    vector<BONE_VERTEX_BOUNDS> staged(m_iNumBones);
    for (const auto& vertex : mesh.skinnedVertices)
    {
        const float3_t& position = vertex.vPosition;
        if (!std::isfinite(position.x) || !std::isfinite(position.y) ||
            !std::isfinite(position.z)) return;
        const uint32_t indices[4] = { vertex.vBlendIndices.x, vertex.vBlendIndices.y,
            vertex.vBlendIndices.z, vertex.vBlendIndices.w };
        const float weights[4] = { vertex.vBlendWeights.x, vertex.vBlendWeights.y,
            vertex.vBlendWeights.z, vertex.vBlendWeights.w };
        float sum = 0.f;
        for (uint32_t lane = 0u; lane < 4u; ++lane)
        {
            if (!std::isfinite(weights[lane]) || weights[lane] < 0.f ||
                indices[lane] >= staged.size()) return;
            sum += weights[lane];
            if (weights[lane] == 0.f) continue;
            auto& bounds = staged[indices[lane]];
            if (!bounds.valid)
            {
                bounds.minimum = bounds.maximum = position;
                bounds.valid = true;
                continue;
            }
            bounds.minimum.x = (std::min)(bounds.minimum.x, position.x);
            bounds.minimum.y = (std::min)(bounds.minimum.y, position.y);
            bounds.minimum.z = (std::min)(bounds.minimum.z, position.z);
            bounds.maximum.x = (std::max)(bounds.maximum.x, position.x);
            bounds.maximum.y = (std::max)(bounds.maximum.y, position.y);
            bounds.maximum.z = (std::max)(bounds.maximum.z, position.z);
        }
        // WModel decoding normalizes nonnegative weights. This convex bound is
        // unavailable for any other weight contract; never change the geometry.
        if (!std::isfinite(sum) || std::fabs(sum - 1.f) > 1.e-5f) return;
    }
    m_BoneVertexBounds = std::move(staged);
}

void CMesh::Build_SkinPalette(const vector<shared_ptr<class CBone>>& Bones,
    float4x4_t* pOutMatrices) const
{
    for (uint32_t i = 0; i < m_iNumBones; ++i)
    {
        XMStoreFloat4x4(&pOutMatrices[i],
            XMLoadFloat4x4(&m_OffsetMatrices[i]) *
            Bones[m_BoneIndices[i]]->Get_CombinedTransformationMatrix());
    }
}

HRESULT CMesh::Render()
{
    const HRESULT result = CVIBuffer::Render();
    if (result == S_OK)
        if (CProfiler* profiler = CGameInstance::Get().Get_Profiler())
            profiler->Record_MeshSubmitted(this, m_iNumIndices, 1, m_szName, m_iNumVertices, m_iMaterialIndex);
    return result;
}

uint32_t CMesh::Select_StaticLod(const MESH_SCREEN_LOD_DESC* view,
    uint32_t& indexCount, uint32_t& firstIndex) const
{
    indexCount = m_iNumIndices;
    firstIndex = 0u;
    CStaticMeshLod::SELECTION selection{ m_iNumIndices, 0u, 0u };
    if (!view || !m_StaticLod || Has_MorphBaseVertices() ||
        !CGameInstance::Get().Get_RenderOptimizationSettings().MeshLodEnabled ||
        S_OK != m_StaticLod->Select_Range(*view, selection))
        return 0u;
    indexCount = selection.indexCount;
    firstIndex = selection.firstIndex;
    return selection.level;
}

HRESULT CMesh::Render_Instanced(ID3D11Buffer* pInstanceBuffer,
	uint32_t iInstanceStride, uint32_t iNumInstances,
	uint32_t iInstanceByteOffset, const MESH_SCREEN_LOD_DESC* screenLod)
{
	if (nullptr == pInstanceBuffer ||
		0 == iInstanceStride ||
		0 == iNumInstances)
	{
		return E_INVALIDARG;
	}

	D3D11_BUFFER_DESC instanceDesc{};
	pInstanceBuffer->GetDesc(&instanceDesc);
	const uint64_t requiredBytes = static_cast<uint64_t>(iInstanceByteOffset) +
		static_cast<uint64_t>(iInstanceStride) * iNumInstances;
	if (nullptr == m_pVB || nullptr == m_pIB || 0u == m_iNumIndices ||
		0u == (instanceDesc.BindFlags & D3D11_BIND_VERTEX_BUFFER) ||
		0u != (iInstanceByteOffset % sizeof(f32_t)) ||
		requiredBytes > instanceDesc.ByteWidth)
		return E_INVALIDARG;

    // The view and immutable LOD errors are already CPU-owned. Select a range
    // directly instead of submitting a one-thread compute dispatch per draw.
    CStaticMeshLod::SELECTION selection{ m_iNumIndices, 0u, 0u };
    selection.level = Select_StaticLod(screenLod, selection.indexCount, selection.firstIndex);
    const bool_t useLod = selection.level > 0u;

	ID3D11Buffer* vertexBuffers[] =
	{
		m_pVB.Get(), pInstanceBuffer
	};

	const uint32_t strides[] =
	{
		m_iVertexStride,
		iInstanceStride
	};

	const uint32_t offsets[] =
	{
		0,
		iInstanceByteOffset
	};

	m_pContext->IASetVertexBuffers(
		0, 2, vertexBuffers, strides, offsets
	);

	m_pContext->IASetIndexBuffer(
        useLod ? m_StaticLod->Get_IndexBuffer() : m_pIB.Get(),
		m_eIndexFormat,
		0
	);

	m_pContext->IASetPrimitiveTopology(
		m_ePrimitiveTopology);

	if (CProfiler* pProfiler =
		CGameInstance::Get().Get_Profiler())
	{
		pProfiler->Add_Counter(
			EProfilerCounter::DrawCalls);

		pProfiler->Add_Counter(
			EProfilerCounter::InstancedDrawCalls);

		pProfiler->Add_Counter(
			EProfilerCounter::Instances,
			iNumInstances);

        const uint64_t submittedIndices = uint64_t(selection.indexCount) * iNumInstances;
        pProfiler->Add_Counter(EProfilerCounter::Indices, submittedIndices);
        if (screenLod)
        {
            if (m_StaticLod && !Has_MorphBaseVertices())
                pProfiler->Add_Counter(EProfilerCounter::MapLodAvailableDraws);
            // 확장된 원경 단계도 LOD0으로 오기록하지 않고 기존 LOD2+ 집계에 포함한다.
            const EProfilerCounter levelCounter = selection.level >= 2u ? EProfilerCounter::MapLod2Draws :
                selection.level == 1u ? EProfilerCounter::MapLod1Draws : EProfilerCounter::MapLod0Draws;
            pProfiler->Add_Counter(levelCounter);
            pProfiler->Add_Counter(EProfilerCounter::MapLodSourceIndices,
                uint64_t(m_iNumIndices) * iNumInstances);
            pProfiler->Add_Counter(EProfilerCounter::MapLodSubmittedIndices, submittedIndices);
        }
    }

    m_pContext->DrawIndexedInstanced(
        selection.indexCount,
        iNumInstances,
        selection.firstIndex,
        0,
        0);

    if (CProfiler* profiler = CGameInstance::Get().Get_Profiler())
        profiler->Record_MeshSubmitted(this, selection.indexCount, iNumInstances, m_szName, m_iNumVertices, m_iMaterialIndex);

	return S_OK;
}

HRESULT CMesh::Prepare_StaticClusterStreams(std::span<const uint32_t> sourceIndices,
    std::span<const uint32_t> farIndices)
{
    if (m_iVertexStride != sizeof(VTXMESH) || sourceIndices.size() != m_iNumVertices ||
        sourceIndices.empty() || sourceIndices.size() > UINT32_MAX / sizeof(uint32_t) ||
        farIndices.size() > UINT32_MAX / sizeof(uint32_t) || farIndices.size() % 3u != 0u ||
        std::any_of(farIndices.begin(), farIndices.end(),
            [&](uint32_t index) { return index >= m_iNumVertices; })) return E_INVALIDARG;
    const auto create = [&](std::span<const uint32_t> values, UINT flags, ComPtr<ID3D11Buffer>& output)
    {
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = static_cast<UINT>(values.size_bytes());
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = flags;
        D3D11_SUBRESOURCE_DATA data{};
        data.pSysMem = values.data();
        return m_pDevice->CreateBuffer(&desc, &data, &output);
    };
    ComPtr<ID3D11Buffer> sources, reduced;
    HRESULT result = create(sourceIndices, D3D11_BIND_VERTEX_BUFFER, sources);
    if (FAILED(result)) return result;
    if (!farIndices.empty() && FAILED(result = create(farIndices, D3D11_BIND_INDEX_BUFFER, reduced))) return result;
    m_ClusterSourceIndices = std::move(sources);
    m_ClusterFarIndices = std::move(reduced);
    m_iClusterFarIndexCount = static_cast<uint32_t>(farIndices.size());
    return S_OK;
}

HRESULT CMesh::Render_StaticCluster(bool_t farGeometry)
{
    if (!m_pVB || !m_pIB || !m_ClusterSourceIndices || m_iNumIndices == 0u ||
        Has_MorphBaseVertices()) return E_INVALIDARG;
    const bool reduced = farGeometry && m_ClusterFarIndices && m_iClusterFarIndexCount > 0u;
    const uint32_t count = reduced ? m_iClusterFarIndexCount : m_iNumIndices;
    ID3D11Buffer* buffers[] = { m_pVB.Get(), m_ClusterSourceIndices.Get() };
    const UINT strides[] = { sizeof(VTXMESH), sizeof(uint32_t) };
    const UINT offsets[] = { 0u, 0u };
    m_pContext->IASetVertexBuffers(0u, 2u, buffers, strides, offsets);
    m_pContext->IASetIndexBuffer(reduced ? m_ClusterFarIndices.Get() : m_pIB.Get(), DXGI_FORMAT_R32_UINT, 0u);
    m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    if (auto* profiler = CGameInstance::Get().Get_Profiler())
    {
        profiler->Add_Counter(EProfilerCounter::DrawCalls);
        profiler->Add_Counter(EProfilerCounter::Indices, count);
    }
    m_pContext->DrawIndexed(count, 0u, 0);
    if (auto* profiler = CGameInstance::Get().Get_Profiler())
        profiler->Record_MeshSubmitted(this, count, 1u, m_szName, m_iNumVertices, m_iMaterialIndex);
    return S_OK;
}

HRESULT CMesh::Ready_VertexBuffer_NonAnim(const aiMesh* pAIMesh, fmatrix_t PreTransformMatrix)
{
	D3D11_BUFFER_DESC		VertexBufferDesc{};
	VertexBufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;
	VertexBufferDesc.StructureByteStride = m_iVertexStride;

	unique_ptr<VTXMESH[]> pVertices = make_unique<VTXMESH[]>(m_iNumVertices);

	for (uint32_t i = 0; i < m_iNumVertices; i++)
	{
		memcpy(&pVertices[i].vPosition, &pAIMesh->mVertices[i], sizeof(float3_t));
		XMStoreFloat3(&pVertices[i].vPosition,
			XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PreTransformMatrix));

		memcpy(&pVertices[i].vNormal, &pAIMesh->mNormals[i], sizeof(float3_t));
		XMStoreFloat3(&pVertices[i].vNormal,
			XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PreTransformMatrix));

		memcpy(&pVertices[i].vTangent, &pAIMesh->mTangents[i], sizeof(float3_t));
		XMStoreFloat3(&pVertices[i].vTangent,
			XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vTangent), PreTransformMatrix));

		memcpy(&pVertices[i].vBinormal, &pAIMesh->mBitangents[i], sizeof(float3_t));
		XMStoreFloat3(&pVertices[i].vBinormal,
			XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vBinormal), PreTransformMatrix));

		memcpy(&pVertices[i].vTexcoord, &pAIMesh->mTextureCoords[0][i], sizeof(float2_t));
		if (pAIMesh->HasTextureCoords(1))
			memcpy(&pVertices[i].vTexcoord1, &pAIMesh->mTextureCoords[1][i], sizeof(float2_t));
	}

	D3D11_SUBRESOURCE_DATA			VertexInitialData{};
	VertexInitialData.pSysMem = pVertices.get();

	if (FAILED(m_pDevice->CreateBuffer(&VertexBufferDesc, &VertexInitialData, &m_pVB)))
		return E_FAIL;


	return S_OK;
}

HRESULT CMesh::Ready_VertexBuffer_Anim(const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones)
{
	D3D11_BUFFER_DESC		VertexBufferDesc{};
	VertexBufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;
	VertexBufferDesc.StructureByteStride = m_iVertexStride;

	unique_ptr<VTXANIMMESH[]> pVertices = make_unique<VTXANIMMESH[]>(m_iNumVertices);

	for (uint32_t i = 0; i < m_iNumVertices; i++)
	{
		memcpy(&pVertices[i].vPosition, &pAIMesh->mVertices[i], sizeof(float3_t));
		memcpy(&pVertices[i].vNormal, &pAIMesh->mNormals[i], sizeof(float3_t));
		memcpy(&pVertices[i].vTangent, &pAIMesh->mTangents[i], sizeof(float3_t));
		memcpy(&pVertices[i].vBinormal, &pAIMesh->mBitangents[i], sizeof(float3_t));
		memcpy(&pVertices[i].vTexcoord, &pAIMesh->mTextureCoords[0][i], sizeof(float2_t));
		if (pAIMesh->HasTextureCoords(1))
			memcpy(&pVertices[i].vTexcoord1, &pAIMesh->mTextureCoords[1][i], sizeof(float2_t));
		if (pAIMesh->HasTextureCoords(2))
			memcpy(&pVertices[i].vTexcoord2, &pAIMesh->mTextureCoords[2][i], sizeof(float2_t));
	}

	m_iNumBones = pAIMesh->mNumBones;



	for (uint32_t i = 0; i < m_iNumBones; i++)
	{
		aiBone* pAIBone = pAIMesh->mBones[i];
		int32_t	iTotalBoneIndex = { -1 };
		float4x4_t	OffsetMatrix = {};

		memcpy(&OffsetMatrix, &pAIBone->mOffsetMatrix, sizeof(float4x4_t));
		XMStoreFloat4x4(&OffsetMatrix, XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));


		m_OffsetMatrices.push_back(OffsetMatrix);


		auto	iter = find_if(Bones.begin(), Bones.end(), [&](shared_ptr<class CBone> pBone)->bool_t {
			++iTotalBoneIndex;
			return pBone->Compare_Name(pAIBone->mName.C_Str());
			});

		if (iter == Bones.end())
			return E_FAIL;

		m_BoneIndices.push_back(iTotalBoneIndex);

		// pAIBone->mNumWeights : 이 뼈가 몇개 정점에게 영향을 주는가?
		for (uint32_t j = 0; j < pAIBone->mNumWeights; j++)
		{
			// pAIBone->mWeights[j].mVertexId : i번째 뼈가 영향을 주는 정점들중 j번째 영향을 주는 정점의 인덱스
			if(0.f == pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.x)
			{
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.x = i;
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.x = pAIBone->mWeights[j].mWeight;
			}

			else if (0.f == pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.y)
			{
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.y = i;
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.y = pAIBone->mWeights[j].mWeight;
			}

			else if (0.f == pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.z)
			{
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.z = i;
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.z = pAIBone->mWeights[j].mWeight;
			}

			else if (0.f == pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.w)
			{
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.w = i;
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.w = pAIBone->mWeights[j].mWeight;
			}
		}
	}

	if (0 == m_iNumBones)
	{
		m_iNumBones = 1;
		int32_t	iTotalBoneIndex = { -1 };
		float4x4_t	OffsetMatrix = {};

		auto	iter = find_if(Bones.begin(), Bones.end(), [&](shared_ptr<class CBone> pBone)->bool_t {
			++iTotalBoneIndex;
			return pBone->Compare_Name(m_szName);
		});

		if (iter == Bones.end())
			return E_FAIL;

		m_BoneIndices.push_back(iTotalBoneIndex);

		XMStoreFloat4x4(&OffsetMatrix, XMMatrixIdentity());

		m_OffsetMatrices.push_back(OffsetMatrix);
	}

	D3D11_SUBRESOURCE_DATA			VertexInitialData{};
	VertexInitialData.pSysMem = pVertices.get();

	if (FAILED(m_pDevice->CreateBuffer(&VertexBufferDesc, &VertexInitialData, &m_pVB)))
		return E_FAIL;


	return S_OK;
}

shared_ptr<CMesh> CMesh::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, MODEL eType, const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones, fmatrix_t PreTransformMatrix)
{
	auto pInstance = shared_ptr<CMesh>(new CMesh(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype(eType, pAIMesh, Bones, PreTransformMatrix)))
	{
		MSG_BOX("Failed to Created : CMesh");
		return nullptr;
	}

	return pInstance;
}

shared_ptr<CMesh> CMesh::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext, MODEL eType,
	const MODEL_MESH_DATA& mesh, const MODEL_SKELETON_DATA& skeleton,
	fmatrix_t PreTransformMatrix)
{
	auto pInstance = shared_ptr<CMesh>(new CMesh(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype(
		eType, mesh, skeleton, PreTransformMatrix)))
	{
		MSG_BOX("Failed to Created : CMesh");
		return nullptr;
	}
	return pInstance;
}


bool_t CMesh::Get_MorphBaseVertex(uint32_t iIndex, float3_t& OutPosition, float3_t& OutNormal) const
{
	if (!Has_MorphBaseVertices() || iIndex >= m_iNumVertices)
		return false;
	OutPosition = (*m_pMorphBasePositions)[iIndex];
	OutNormal = (*m_pMorphBaseNormals)[iIndex];
	return true;
}

HRESULT CMesh::Make_VertexBuffer_Unique()
{
	if (m_hasUniqueVertexBuffer)
		return S_OK;
	if (0u == m_iNumVertices || nullptr == m_pVB)
		return E_FAIL;

	const uint32_t iByteWidth = m_iVertexStride * m_iNumVertices;

	/* The unmorphed rest position/normal has to come from somewhere, and the vertex buffer
	is D3D11_USAGE_DEFAULT with CPUAccessFlags 0, so it cannot be Map()ed directly. Copying
	it into a staging buffer first is the one supported readback path, and doing it here --
	once, for the one character that actually opens the face editor -- costs nothing for
	every other model, which is why nothing is retained at load time. */
	D3D11_BUFFER_DESC stagingDesc{};
	stagingDesc.ByteWidth = iByteWidth;
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.BindFlags = 0;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingDesc.StructureByteStride = m_iVertexStride;

	ComPtr<ID3D11Buffer> pStaging;
	if (FAILED(m_pDevice->CreateBuffer(&stagingDesc, nullptr, &pStaging)))
		return E_FAIL;
	m_pContext->CopyResource(pStaging.Get(), m_pVB.Get());

	D3D11_MAPPED_SUBRESOURCE mapped{};
	if (FAILED(m_pContext->Map(pStaging.Get(), 0, D3D11_MAP_READ, 0, &mapped)) ||
		nullptr == mapped.pData)
	{
		return E_FAIL;
	}

	auto pPositions = make_shared<vector<float3_t>>(m_iNumVertices);
	auto pNormals = make_shared<vector<float3_t>>(m_iNumVertices);
	const uint8_t* pVertices = static_cast<const uint8_t*>(mapped.pData);
	for (uint32_t i = 0; i < m_iNumVertices; ++i)
	{
		/* vPosition sits at offset 0 and vNormal at 12 in both VTXMESH and VTXANIMMESH. */
		const uint8_t* pVertex = pVertices + static_cast<size_t>(i) * m_iVertexStride;
		memcpy(&(*pPositions)[i], pVertex, sizeof(float3_t));
		memcpy(&(*pNormals)[i], pVertex + sizeof(float3_t), sizeof(float3_t));
	}
	m_pContext->Unmap(pStaging.Get(), 0);

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth = iByteWidth;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.StructureByteStride = m_iVertexStride;

	ComPtr<ID3D11Buffer> pUniqueVB;
	if (FAILED(m_pDevice->CreateBuffer(&vertexBufferDesc, nullptr, &pUniqueVB)))
		return E_FAIL;

	/* GPU-side copy of the buffer this instance still shares with the prototype and its
	siblings, so the new one starts identical -- every field, not just the position/normal
	this class knows how to touch. */
	m_pContext->CopyResource(pUniqueVB.Get(), m_pVB.Get());

	m_pMorphBasePositions = pPositions;
	m_pMorphBaseNormals = pNormals;
	m_pVB = pUniqueVB;
	m_hasUniqueVertexBuffer = true;
	m_TouchedVertexIndices.clear();
	return S_OK;
}

HRESULT CMesh::Update_Vertices(const vector<uint32_t>& iIndices,
	const vector<float3_t>& Positions, const vector<float3_t>& Normals)
{
	if (!m_hasUniqueVertexBuffer)
		return E_FAIL;
	if (iIndices.size() != Positions.size() || iIndices.size() != Normals.size())
		return E_INVALIDARG;

	for (size_t i = 0; i < iIndices.size(); ++i)
	{
		const uint32_t iVertex = iIndices[i];
		if (iVertex >= m_iNumVertices)
			return E_INVALIDARG;

		const f32_t PositionNormal[6] = {
			Positions[i].x, Positions[i].y, Positions[i].z,
			Normals[i].x, Normals[i].y, Normals[i].z,
		};

		D3D11_BOX Box{};
		Box.left = iVertex * m_iVertexStride;
		Box.right = Box.left + sizeof(PositionNormal);
		Box.top = 0u;
		Box.bottom = 1u;
		Box.front = 0u;
		Box.back = 1u;

		/* vPosition and vNormal are adjacent (offset 0 and 12) in both VTXMESH and
		VTXANIMMESH, so one 24-byte box covers both; every other field (tangent, binormal,
		UV, blend indices/weights) is outside this box and is left untouched. */
		m_pContext->UpdateSubresource(m_pVB.Get(), 0, &Box, PositionNormal, 0, 0);
		m_TouchedVertexIndices.push_back(iVertex);
	}
	return S_OK;
}

HRESULT CMesh::Reset_Vertices()
{
	if (!m_hasUniqueVertexBuffer || m_TouchedVertexIndices.empty())
		return S_OK;
	if (!Has_MorphBaseVertices())
		return E_FAIL;

	for (uint32_t iVertex : m_TouchedVertexIndices)
	{
		const float3_t& BasePosition = (*m_pMorphBasePositions)[iVertex];
		const float3_t& BaseNormal = (*m_pMorphBaseNormals)[iVertex];
		const f32_t PositionNormal[6] = {
			BasePosition.x, BasePosition.y, BasePosition.z,
			BaseNormal.x, BaseNormal.y, BaseNormal.z,
		};

		D3D11_BOX Box{};
		Box.left = iVertex * m_iVertexStride;
		Box.right = Box.left + sizeof(PositionNormal);
		Box.top = 0u;
		Box.bottom = 1u;
		Box.front = 0u;
		Box.back = 1u;

		m_pContext->UpdateSubresource(m_pVB.Get(), 0, &Box, PositionNormal, 0, 0);
	}
	m_TouchedVertexIndices.clear();
	return S_OK;
}

shared_ptr<CPrototype> CMesh::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CMesh>(new CMesh(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMesh");
		return nullptr;
	}

	return pInstance;
}


HRESULT CMesh::Prepare_StaticLod(const MODEL_MESH_DATA& mesh, fmatrix_t preTransform)
{
    if (mesh.vertexKind != MODEL_VERTEX_KIND::STATIC || mesh.indices.size() < CStaticMeshLod::MINIMUM_INDICES ||
        mesh.indices.size() > 3u * 1024u * 1024u || Has_MorphBaseVertices()) return S_FALSE;
    try
    {
        auto vertices = mesh.vertices;
        for (size_t i = 0u; i < vertices.size(); ++i)
        {
            auto& v = vertices[i];
            v.color0Rgba8 = mesh.hasColor0 ? mesh.color0Rgba8[i] : 0xffffffffu;
            XMStoreFloat3(&v.vPosition, XMVector3TransformCoord(XMLoadFloat3(&v.vPosition), preTransform));
            XMStoreFloat3(&v.vNormal, XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&v.vNormal), preTransform)));
            XMStoreFloat3(&v.vTangent, XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&v.vTangent), preTransform)));
            XMStoreFloat3(&v.vBinormal, XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&v.vBinormal), preTransform)));
        }
        return CStaticMeshLod::Create(m_pDevice.Get(), vertices, mesh.indices, m_StaticLod);
    }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
    catch (const std::length_error&) { return E_OUTOFMEMORY; }
}
```

#### Client/Private/MapStaticBatchObject.cpp 전체 반영 코드

```cpp
#include "MapStaticBatchObject.h"
#include "MapStaticChunkObject.h"
#pragma push_macro("new")
#undef new
#include "Engine_RenderTypes.h"
#pragma pop_macro("new")
#include "Engine_VertexTypes.h"

#include "GameInstance.h"
#include "MapAssetRenderUtils.h"
#include "Model.h"
#include "MeshLod.h"
#include "OcclusionCuller.h"
#include "Profiler.h"
#include "EffectFailureDiagnostic.h"
#include "Shader.h"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <cstring>
#include <limits>
#include <cmath>
#include <cfloat>
#include <sstream>

namespace
{
    bool SameInstanceProfile(const MAP_ASSET_RENDER_PROFILE& a,
        const MAP_ASSET_RENDER_PROFILE& b)
    {
        return a.renderMode == b.renderMode && a.cullMode == b.cullMode &&
            a.uvScale.x == b.uvScale.x && a.uvScale.y == b.uvScale.y &&
            a.uvSpeed.x == b.uvSpeed.x && a.uvSpeed.y == b.uvSpeed.y &&
            a.opacity == b.opacity && a.opacityPower == b.opacityPower &&
            a.emissiveIntensity == b.emissiveIntensity &&
            a.specularIntensity == b.specularIntensity && a.specularPower == b.specularPower &&
            a.colorTint.x == b.colorTint.x && a.colorTint.y == b.colorTint.y &&
            a.colorTint.z == b.colorTint.z && a.colorTint.w == b.colorTint.w &&
            a.triplanarHeightScale == b.triplanarHeightScale && a.castsShadow == b.castsShadow;
    }
    // Conservative operator-norm bound for signed/nonuniform scale and shear.
    float LinearScaleBound(const float4x4_t& matrix)
    {
        double maximum = 0.;
        for (size_t i = 0u; i < 3u; ++i)
        {
            double row = 0.;
            for (size_t j = 0u; j < 3u; ++j)
            {
                double dot = 0.;
                for (size_t k = 0u; k < 3u; ++k) dot += double(matrix.m[i][k]) * matrix.m[j][k];
                row += std::abs(dot);
            }
            if (!std::isfinite(row)) return 0.f;
            maximum = (std::max)(maximum, row);
        }
        const double scale = std::sqrt(maximum);
        if (!std::isfinite(scale) || scale <= 0. || scale >= (std::numeric_limits<float>::max)()) return 0.f;
        return std::nextafter(static_cast<float>(scale), (std::numeric_limits<float>::infinity)());
    }
    float StaticPropDistanceLimit(float radius, float scale)
    {
        if (!std::isfinite(radius) || !(radius > 0.f) || radius > 3.f ||
            !std::isfinite(scale) || scale < .25f || scale > 4.f) return 0.f;
        return (radius <= .5f ? 35.f : radius <= 1.5f ? 45.f : 60.f) * scale;
    }

    bool BeyondStaticPropDistance(const FMapStaticInstance& instance, const float4_t& camera,
        float limit, float margin)
    {
        if (!(limit > 0.f) || !std::isfinite(margin) || margin < 0.f ||
            !std::isfinite(camera.x) || !std::isfinite(camera.y) || !std::isfinite(camera.z)) return false;
        const double x = double(instance.WorldBoundsCenter.x) - camera.x;
        const double y = double(instance.WorldBoundsCenter.y) - camera.y;
        const double z = double(instance.WorldBoundsCenter.z) - camera.z;
        const double distanceSquared = x * x + y * y + z * z;
        const double threshold = double(limit) + instance.WorldBoundsRadius + margin;
        return std::isfinite(distanceSquared) && std::isfinite(threshold) && distanceSquared > threshold * threshold;
    }

    bool StaticPropFitsPixelLimit(const FMapStaticInstance& instance, const MAP_CAMERA_CULL_SNAPSHOT& camera,
        const float2_t& viewport, float maximumPixels, float margin)
    {
        const auto& p = camera.projection;
        if (!(camera.lodViewScale > 0.f) || !std::isfinite(maximumPixels) || !(maximumPixels > 0.f) || maximumPixels > 128.f ||
            !(viewport.x > 0.f) || !(viewport.y > 0.f) || p._14 != 0.f || p._24 != 0.f || p._34 != 1.f || p._44 != 0.f ||
            p._12 != 0.f || p._21 != 0.f || p._13 != 0.f || p._23 != 0.f || p._41 != 0.f || p._42 != 0.f ||
            !(p._11 > 0.f) || !(p._22 > 0.f) || !(p._33 > 1.f) || !(p._43 < 0.f)) return false;
        float3_t center{};
        XMStoreFloat3(&center, XMVector3TransformCoord(XMLoadFloat3(&instance.WorldBoundsCenter), XMLoadFloat4x4(&camera.lodView)));
        if (!std::isfinite(center.x) || !std::isfinite(center.y) || !std::isfinite(center.z)) return false;
        const double radius = (double(instance.WorldBoundsRadius) + margin) * camera.lodViewScale;
        const double depth = center.z;
        if (!(radius > 0.) || !(depth - radius > -double(p._43) / p._33)) return false;
        // Project all eight corners of the enclosing view-space cube. A sphere
        // cannot exceed this rectangle; off-axis depth expansion is retained.
        double maximumDiameter = 0.;
        for (size_t axis = 0u; axis < 2u; ++axis)
        {
            double minimum = DBL_MAX, maximum = -DBL_MAX;
            for (int sign = -1; sign <= 1; sign += 2)
                for (int side = -1; side <= 1; side += 2)
                {
                    const double projected = (double((&center.x)[axis]) + sign * radius) / (depth + side * radius);
                    minimum = (std::min)(minimum, projected); maximum = (std::max)(maximum, projected);
                }
            const double pixels = (axis == 0u ? double(p._11) * viewport.x : double(p._22) * viewport.y) * .5;
            maximumDiameter = (std::max)(maximumDiameter, (maximum - minimum) * pixels);
        }
        return std::isfinite(maximumDiameter) && maximumDiameter > 0. &&
            maximumDiameter * (1. + 32. * FLT_EPSILON) <= maximumPixels;
    }

    bool StaticPropDistanceHidden(const FMapStaticInstance& instance, const float4_t& position,
        const MAP_CAMERA_CULL_SNAPSHOT& camera, const float2_t& viewport, float limit,
        float maximumPixels, uint64_t settingsRevision, float margin)
    {
        const bool wasHidden = instance.DistanceHidden && instance.DistanceSettingsRevision == settingsRevision;
        return BeyondStaticPropDistance(instance, position, wasHidden ? limit : limit * 1.1f, margin) &&
            StaticPropFitsPixelLimit(instance, camera, viewport, wasHidden ? maximumPixels : maximumPixels * .9f, margin);
    }

    bool StaticOcclusionBoundsMesh(const Engine::CModel& model, uint32_t mesh)
    {
        const auto* surface = model.Get_MaterialSurface(mesh);
        // Alpha-tested BG still has immutable geometry. It can be hidden by
        // another opaque object, but cannot itself fill the occlusion buffer.
        return surface && surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
            !surface->sourceFoliageWind && !model.Has_MorphBaseVertices(mesh) &&
            (surface->renderMode == Engine::MODEL_SURFACE_RENDER_MODE::INHERIT ||
             surface->renderMode == Engine::MODEL_SURFACE_RENDER_MODE::DEFERRED);
    }

    bool StaticOpaqueOcclusionMesh(const Engine::CModel& model, uint32_t mesh)
    {
        const auto* surface = model.Get_MaterialSurface(mesh);
        return surface && surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
            !surface->sourceFoliageWind && (surface->sourceBgFlags & 64u) == 0u &&
            (surface->renderMode == Engine::MODEL_SURFACE_RENDER_MODE::INHERIT ||
             surface->renderMode == Engine::MODEL_SURFACE_RENDER_MODE::DEFERRED) &&
            !model.Has_MaterialTexture(mesh, aiTextureType_OPACITY) && !model.Has_MorphBaseVertices(mesh);
    }

    bool TransformStaticBounds(const float3_t& localMin, const float3_t& localMax, const float4x4_t& world,
        float3_t& minimum, float3_t& maximum)
    {
        if (world._14 != 0.f || world._24 != 0.f || world._34 != 0.f || world._44 != 1.f) return false;
        float3_t stagedMin{}, stagedMax{};
        for (size_t axis = 0u; axis < 3u; ++axis)
        {
            double lo = world.m[3][axis], hi = lo, magnitude = std::abs(lo);
            for (size_t source = 0u; source < 3u; ++source)
            {
                if (!std::isfinite((&localMin.x)[source]) || !std::isfinite((&localMax.x)[source]) ||
                    (&localMin.x)[source] > (&localMax.x)[source]) return false;
                const double a = double((&localMin.x)[source]) * world.m[source][axis];
                const double b = double((&localMax.x)[source]) * world.m[source][axis];
                lo += (std::min)(a, b); hi += (std::max)(a, b); magnitude += (std::max)(std::abs(a), std::abs(b));
            }
            const double margin = 16. * FLT_EPSILON * (magnitude + 1.);
            lo -= margin; hi += margin;
            if (!std::isfinite(lo) || !std::isfinite(hi) || lo <= -FLT_MAX || hi >= FLT_MAX) return false;
            (&stagedMin.x)[axis] = std::nextafter(static_cast<float>(lo), -INFINITY);
            (&stagedMax.x)[axis] = std::nextafter(static_cast<float>(hi), INFINITY);
        }
        minimum = stagedMin; maximum = stagedMax;
        return true;
    }

    struct INSTANCE_ENVELOPE final
    {
        double minimum[3] = { DBL_MAX, DBL_MAX, DBL_MAX };
        double maximum[3] = { -DBL_MAX, -DBL_MAX, -DBL_MAX };
        bool valid = true, any = false;
        float maximumScale = 0.f;
        void Add(const FMapStaticInstance& instance, const float scale)
        {
            const float* center = &instance.WorldBoundsCenter.x;
            if (!std::isfinite(instance.WorldBoundsRadius) || instance.WorldBoundsRadius <= 0.f) valid = false;
            for (size_t axis = 0u; axis < 3u; ++axis)
            {
                if (!std::isfinite(center[axis])) valid = false;
                minimum[axis] = (std::min)(minimum[axis], double(center[axis]) - instance.WorldBoundsRadius);
                maximum[axis] = (std::max)(maximum[axis], double(center[axis]) + instance.WorldBoundsRadius);
            }
            if (scale <= 0.f) valid = false;
            maximumScale = (std::max)(maximumScale, scale);
            any = true;
        }
        bool Store(float4_t& sphere) const
        {
            if (!valid || !any) return false;
            double radiusSquared = 0.;
            for (size_t axis = 0u; axis < 3u; ++axis)
            {
                const double c = (minimum[axis] + maximum[axis]) * .5;
                if (!std::isfinite(c) || std::abs(c) > (std::numeric_limits<float>::max)()) return false;
                (&sphere.x)[axis] = static_cast<float>(c);
                const double extent = (std::max)(maximum[axis] - (&sphere.x)[axis], double((&sphere.x)[axis]) - minimum[axis]);
                radiusSquared += extent * extent;
            }
            const double radius = std::sqrt(radiusSquared);
            if (!std::isfinite(radius) || radius <= 0. || radius >= (std::numeric_limits<float>::max)()) return false;
            sphere.w = std::nextafter(static_cast<float>(radius), (std::numeric_limits<float>::infinity)());
            return true;
        }
    };

    struct VIEW_LOD_ENVELOPE final
    {
        const float4x4_t* view = nullptr;
        double viewScale = 0.;
        double maximumX = 0., maximumY = 0., minimumZ = DBL_MAX;
        bool valid = false, any = false;

        explicit VIEW_LOD_ENVELOPE(const MAP_CAMERA_CULL_SNAPSHOT* camera)
        {
            if (!camera) return;
            view = &camera->lodView;
            viewScale = camera->lodViewScale;
            valid = viewScale > 0.;
        }

        void Add(const FMapStaticInstance& instance)
        {
            if (!valid) return;
            const double radius = double(instance.WorldBoundsRadius) * viewScale;
            if (!std::isfinite(radius) || radius <= 0.) { valid = false; return; }
            double center[3]{}, margin[3]{};
            for (size_t axis = 0u; axis < 3u; ++axis)
            {
                center[axis] = view->m[3][axis];
                double magnitude = std::abs(center[axis]);
                for (size_t source = 0u; source < 3u; ++source)
                {
                    const double term = double((&instance.WorldBoundsCenter.x)[source]) * view->m[source][axis];
                    center[axis] += term;
                    magnitude += std::abs(term);
                }
                // Enclose float matrix evaluation as well as the sphere itself,
                // including cancellation in translated/rotated view coordinates.
                margin[axis] = 16. * FLT_EPSILON * (magnitude + radius + 1.);
                if (!std::isfinite(center[axis]) || !std::isfinite(margin[axis])) { valid = false; return; }
            }
            maximumX = (std::max)(maximumX, std::abs(center[0]) + radius + margin[0]);
            maximumY = (std::max)(maximumY, std::abs(center[1]) + radius + margin[1]);
            minimumZ = (std::min)(minimumZ, center[2] - radius - margin[2]);
            any = true;
        }

        bool Store(float4_t& result) const
        {
            if (!valid || !any || maximumX >= FLT_MAX || maximumY >= FLT_MAX || std::abs(minimumZ) >= FLT_MAX)
                return false;
            result = {
                std::nextafter(static_cast<float>(maximumX), (std::numeric_limits<float>::infinity)()),
                std::nextafter(static_cast<float>(maximumY), (std::numeric_limits<float>::infinity)()),
                std::nextafter(static_cast<float>(minimumZ), -(std::numeric_limits<float>::infinity)()), 1.f };
            return true;
        }
    };

}

struct CMapStaticBatchObject::CPU_VISIBILITY_PREPARATION final
{
    enum class STATE { EMPTY, STAGED, READY, FAILED };
    STATE State = STATE::EMPTY;
    HRESULT Result = S_OK;
    uint64_t Frame = 0u;
    bool HasCamera = false, DistanceEnabled = false, CountersCommitted = false;
    // Borrowed only until synchronous CPU execution ends. READY/FAILED states
    // retain the revision separately and never dereference this pointer again.
    const MAP_CAMERA_CULL_SNAPSHOT* Camera = nullptr;
    uint64_t CameraRevision = 0u;
    Engine::MAP_VISIBILITY_SETTINGS Settings{};
    float2_t Viewport{};
    float4_t CameraPosition{};
    MAP_FRUSTUM_RUNTIME_STATE BatchFrustum{};
    std::vector<MAP_FRUSTUM_RUNTIME_STATE> FrustumStates;
    bool RequiresNextTick = false, RejectedBatch = false, HasOcclusionBounds = false;
    float3_t OcclusionMin{}, OcclusionMax{};
    float4_t LodBounds{}, TightLodBounds{};
    float LodScale = 0.f;
    uint64_t CullingCandidates = 0u, DistanceTested = 0u, DistanceRejected = 0u;
};

CMapStaticBatchObject::CMapStaticBatchObject(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject{ pDevice, pContext }
{}

CMapStaticBatchObject::CMapStaticBatchObject(
	const CMapStaticBatchObject& prototype)
	: CGameObject{ prototype }
{}

CMapStaticBatchObject::~CMapStaticBatchObject()
{}

HRESULT CMapStaticBatchObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMapStaticBatchObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_INVALIDARG;

	const DESC& desc =
		*static_cast<DESC*>(pArg);

	if (desc.AssetId.empty() ||
		desc.ModelPrototypeTag.empty() ||
		desc.Instances.empty() ||
		MAP_ASSET_RENDER_MODE::DEFERRED !=
		desc.RenderProfile.renderMode)
	{
		return E_INVALIDARG;
	}

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_AssetId = desc.AssetId;
	m_AssetGroupId = desc.AssetGroupId;
	m_RenderProfile = desc.RenderProfile;
	m_FrustumCulling = desc.FrustumCulling;
	m_bMirrored = desc.Mirrored;
	m_Instances = desc.Instances;
	// Initialize before Layer captures the immutable update phase mask.
	m_FrameState = desc.FrameState;

    // Hidden gate batches become visible together at the cinematic handoff.
    // Allocate both streams while staging so their first shadow draw only uploads.
	if (FAILED(Ready_Components(
		desc.PrototypeLevelIndex,
		desc.ModelPrototypeTag)) ||
		FAILED(Rebuild_PlacementLookup()) ||
		FAILED(Ensure_InstanceCapacity(
			static_cast<uint32_t>(
				m_Instances.size()))) ||
        (m_RenderProfile.castsShadow && FAILED(Ensure_ShadowInstanceCapacity(
            static_cast<uint32_t>(m_Instances.size())))))
	{
		return E_FAIL;
	}

	return S_OK;
}

void CMapStaticBatchObject::Update(
	f32_t fTimeDelta)
{
	if (!m_FrameState)
		m_fElapsedTime += fTimeDelta;
}

void CMapStaticBatchObject::Late_Update(
	f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (m_FrameState)
		return;
	m_bFinalCameraPrepared = false;
    Invalidate_VisibilityCpu();

	if (Engine::CProfiler* profiler =
		CGameInstance::Get().Get_Profiler())
	{
		profiler->Add_Counter(
			Engine::EProfilerCounter::MapPlacements,
			m_Instances.size());

		profiler->Add_Counter(
			Engine::EProfilerCounter::MapBatchCount);
	}
}

bool CMapStaticBatchObject::Is_ChunkMeshClaimed(uint32_t mesh) const
{
    return mesh < m_ChunkClaims.size() && m_ChunkClaims[mesh] && m_ChunkClaims[mesh]->Is_Active(
        mesh < m_ChunkClaimMemberIndices.size() ? m_ChunkClaimMemberIndices[mesh] : UINT32_MAX);
}

bool CMapStaticBatchObject::Are_AllChunkMeshesClaimed() const
{
    if (!m_pModelCom || m_pModelCom->Get_NumMeshes() == 0u ||
        m_ChunkClaims.size() < m_pModelCom->Get_NumMeshes()) return false;
    for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
        if (!Is_ChunkMeshClaimed(mesh)) return false;
    return true;
}

void CMapStaticBatchObject::Invalidate_ChunkClaims()
{
    for (const auto& claim : m_ChunkClaims)
    {
        if (!claim || !claim->valid) continue;
        claim->valid = false;
        if (auto* profiler = CGameInstance::Get().Get_Profiler())
            profiler->Add_Counter(Engine::EProfilerCounter::MapChunkInvalidations);
    }
}

bool_t CMapStaticBatchObject::Try_GetFinalCameraSpatialBounds(FINAL_CAMERA_SPATIAL_BOUNDS& bounds) const
{
    bounds = {};
    if (m_FrustumCulling.bypass || m_FrustumCulling.diagnostics) return false;
    const f32_t policyValues[] = { m_FrustumCulling.baseMargin, m_FrustumCulling.largeObjectRadiusThreshold,
        m_FrustumCulling.largeObjectAbsoluteMargin, m_FrustumCulling.largeObjectRelativeMargin };
    for (float value : policyValues) if (!std::isfinite(value) || value < 0.f) return false;
    if (m_bBatchBoundsDirty) Rebuild_BatchCullBounds();
    if (!m_bHasBatchBounds) return false;
    const double radius = m_BatchBounds.w;
    double margin = m_FrustumCulling.baseMargin;
    if (m_AssetGroupId == "landscape" || (m_FrustumCulling.largeObjectRadiusThreshold > 0.f &&
        radius >= m_FrustumCulling.largeObjectRadiusThreshold))
        margin = (std::max)({ margin, double(m_FrustumCulling.largeObjectAbsoluteMargin),
            radius * m_FrustumCulling.largeObjectRelativeMargin });
    const double expanded = radius + margin;
    if (!std::isfinite(expanded) || expanded <= 0. || expanded >= FLT_MAX) return false;
    bounds.Center = { m_BatchBounds.x, m_BatchBounds.y, m_BatchBounds.z };
    bounds.Radius = std::nextafter(static_cast<float>(expanded), (std::numeric_limits<float>::infinity)());
    bounds.RejectGraceFrames = (std::max)(m_FrustumCulling.rejectHysteresisFrames,
        m_BatchFrustumState.rejectGraceFrames);
    bounds.ShadowCaster = m_RenderProfile.castsShadow;
    return std::isfinite(bounds.Radius);
}

bool_t CMapStaticBatchObject::Try_GetStaticOcclusionDesc(STATIC_OCCLUSION_DESC& output) const
{
    output = {};
    auto& game = CGameInstance::Get();
    if (!m_FrameState || !m_pModelCom || !Is_FinalCameraPrepared() || m_VisibleInstances.empty() ||
        m_iVisibilitySettingsRevision != game.Get_MapVisibilitySettings().Revision ||
        game.Is_SceneEnvironmentReplaced() || !game.Get_MaterialRenderSettings().bUseSourceMaterials ||
        CMapAssetRenderUtils::Is_SurfaceBindingCollectionActive() || m_FrustumCulling.bypass || m_FrustumCulling.diagnostics ||
        m_RenderProfile.renderMode != MAP_ASSET_RENDER_MODE::DEFERRED || m_RenderProfile.opacity != 1.f ||
        !m_bVisibleOcclusionBounds || m_iStaticShadowRevision == 0u) return false;
    const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
    if (!camera || !m_bVisibleInstancesUsedCamera || m_iVisibleCameraRevision != camera->revision) return false;
    STATIC_OCCLUSION_DESC staged{};
    staged.BoundsMin = m_VisibleOcclusionMin; staged.BoundsMax = m_VisibleOcclusionMax;
    staged.Revision = m_iStaticShadowRevision;
    Engine::MESH_SCREEN_LOD_DESC lod{};
    const bool hasLod = Build_ScreenLodView(*camera, lod);
    uint64_t occluderTriangles = 0u;
    for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
    {
        if (Is_ChunkMeshClaimed(mesh)) continue;
        if (!StaticOcclusionBoundsMesh(*m_pModelCom, mesh)) return false;
        const auto* view = Uses_MeshScreenLod(mesh, hasLod) ? &lod : nullptr;
        const size_t draws = game.Get_RenderOptimizationSettings().MapInstancingEnabled ? 1u : m_VisibleInstances.size();
        if (draws > UINT32_MAX - staged.Draws) return false;
        staged.Draws += static_cast<uint32_t>(draws);
        staged.Indices += uint64_t(m_pModelCom->Get_StaticMeshSelectedIndexCount(mesh, view)) * m_VisibleInstances.size();
        Engine::CModel::STATIC_OCCLUSION_MESH geometry;
        if (StaticOpaqueOcclusionMesh(*m_pModelCom, mesh) && m_pModelCom->Get_StaticMeshLodLevel(mesh, view) == 0u &&
            m_pModelCom->Try_GetStaticOcclusionMesh(mesh, geometry))
            occluderTriangles += uint64_t(geometry.indices.size() / 3u) * m_VisibleInstances.size();
    }
    if (staged.Draws == 0u || staged.Indices == 0u) return false;
    staged.OccluderTriangles = static_cast<uint32_t>((std::min)(occluderTriangles, uint64_t(UINT32_MAX)));
    output = staged;
    return true;
}

uint32_t CMapStaticBatchObject::Rasterize_StaticOccluder(Engine::COcclusionCuller& culler, uint32_t triangleBudget) const
{
    STATIC_OCCLUSION_DESC descriptor;
    if (triangleBudget == 0u || !Try_GetStaticOcclusionDesc(descriptor) || descriptor.OccluderTriangles == 0u) return 0u;
    const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
    if (!camera) return 0u;
    Engine::MESH_SCREEN_LOD_DESC lod{};
    const bool hasLod = Build_ScreenLodView(*camera, lod);
    const auto pass = CMapAssetRenderUtils::Select_Pass(m_RenderProfile, m_bMirrored);
    const auto cull = pass == 0u ? Engine::COcclusionCuller::CULL_MODE::BACK :
        pass == 1u ? Engine::COcclusionCuller::CULL_MODE::FRONT : Engine::COcclusionCuller::CULL_MODE::NONE;
    const uint32_t initialRasterized = culler.GetRasterizedTriangles();
    uint32_t attempted = 0u;
    for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
    {
        if (Is_ChunkMeshClaimed(mesh) || !StaticOpaqueOcclusionMesh(*m_pModelCom, mesh)) continue;
        const auto* view = Uses_MeshScreenLod(mesh, hasLod) ? &lod : nullptr;
        if (m_pModelCom->Get_StaticMeshLodLevel(mesh, view) != 0u) continue;
        Engine::CModel::STATIC_OCCLUSION_MESH geometry;
        if (!m_pModelCom->Try_GetStaticOcclusionMesh(mesh, geometry)) continue;
        const uint32_t triangles = static_cast<uint32_t>(geometry.indices.size() / 3u);
        if (triangles == 0u || triangles > triangleBudget - attempted) continue;
        for (const auto& instance : m_VisibleInstances)
        {
            if (triangles > triangleBudget - attempted) break;
            attempted += triangles;
            (void)culler.Rasterize(geometry.positions, geometry.vertexCount, geometry.strideBytes,
                geometry.indices, instance.World, cull);
        }
    }
    return culler.GetRasterizedTriangles() - initialRasterized;
}

void CMapStaticBatchObject::Rebuild_InstanceOcclusionBounds(FMapStaticInstance& instance) const
{
    instance.OcclusionBoundsValid = m_bOcclusionGeometryEligible && m_pModelCom->Has_LocalBounds() &&
        TransformStaticBounds(m_pModelCom->Get_LocalBoundsMin(), m_pModelCom->Get_LocalBoundsMax(), instance.World,
            instance.OcclusionBoundsMin, instance.OcclusionBoundsMax);
}

void CMapStaticBatchObject::Commit_DistanceSelection(uint64_t revision)
{
    for (const auto& [index, hidden] : m_CandidateDistanceChanges)
    {
        m_Instances[index].DistanceHidden = hidden;
        m_Instances[index].DistanceSettingsRevision = revision;
    }
}

HRESULT CMapStaticBatchObject::Prepare_FinalCameraVisibility(
	const MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot)
{
    const bool hasCamera = cameraSnapshot != nullptr;
    if (Is_FinalCameraPrepared() && m_bVisibleInstancesUsedCamera == hasCamera &&
        m_iVisibleCameraRevision == (hasCamera ? cameraSnapshot->revision : 0u))
    {
        const auto settings = CGameInstance::Get().Get_MapVisibilitySettings();
        const bool distanceEnabled = m_bDistanceEligible && hasCamera && CGameInstance::Get().Get_CamPosition() &&
            settings.DistanceEnabled && !m_FrustumCulling.bypass && !m_FrustumCulling.diagnostics &&
            !CMapAssetRenderUtils::Is_SurfaceBindingCollectionActive();
        if (m_iVisibilitySettingsRevision == settings.Revision && m_bVisibleDistanceEnabled == distanceEnabled)
            return S_OK;
    }

	// Dirty can also mean next-frame rejection grace. Real instance mutations
	// clear prepared state, so repeated consumers must not advance grace twice.
	const HRESULT visibility = Upload_VisibleInstances(cameraSnapshot);
	m_bFinalCameraPrepared = SUCCEEDED(visibility);
	if (m_bFinalCameraPrepared && m_FrameState)
		m_iPreparedFrame = m_FrameState->frameNumber;
	return visibility;
}

void CMapStaticBatchObject::Submit_FinalCamera()
{
    auto& game = CGameInstance::Get();
    if (game.Is_SceneEnvironmentReplaced() || m_iAuthoredVisibleInstanceCount == 0u)
        return;

    if (!Are_AllChunkMeshesClaimed())
    {
        const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
        const HRESULT visibility = Prepare_FinalCameraVisibility(camera);
        // A failed preparation keeps the original draw callback as the error/retry path.
        if (FAILED(visibility) || !m_VisibleInstances.empty())
            game.Add_RenderObject(RENDERGROUP::NONBLEND,
                static_pointer_cast<CGameObject>(shared_from_this()));
    }

    // Camera rejection does not reject a caster whose shadow reaches the view.
    // Providers have now committed the final shadow switch and light volume.
    if (m_RenderProfile.castsShadow && game.Is_ShadowLightEnabled())
    {
        const HRESULT shadows = Upload_ShadowInstances();
        if (FAILED(shadows) || !m_ShadowInstances.empty())
            game.Add_RenderObject(RENDERGROUP::SHADOW,
                static_pointer_cast<CGameObject>(shared_from_this()));
    }
}

HRESULT CMapStaticBatchObject::Render_AdjacentNonBlend(
    std::span<const std::shared_ptr<CGameObject>> objects, size_t& consumed)
{
    // Shader capacity limits distinct lighting bundles, not adjacent batches.
    // One bundle uses the ordinary pass; two/three use the small bank.
    constexpr size_t MINIMUM_LIGHTING_BANK_BATCHES = 2u;
    consumed = 1u;
    auto& game = CGameInstance::Get();
    // Keep authoring diagnostics and all unsupported material/geometry paths exact.
    const auto optimization = game.Get_RenderOptimizationSettings();
    if (!optimization.MapInstancingEnabled ||
        objects.size() < MINIMUM_LIGHTING_BANK_BATCHES || objects.front().get() != this ||
        game.Is_SceneEnvironmentReplaced() ||
        !game.Get_MaterialRenderSettings().bUseSourceMaterials ||
        CMapAssetRenderUtils::Is_SurfaceBindingCollectionActive())
        return Render();

    if (optimization.IdenticalBatchEnabled)
    {
        const HRESULT identical = Try_RenderIdenticalInstances(objects, consumed);
        if (identical != S_FALSE) return identical;
    }
    if (!optimization.LightingBankEnabled) return Render();

    const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
    const bool_t hasCamera = camera != nullptr;
    const uint64_t cameraRevision = camera ? camera->revision : 0u;
    const auto ready = [&](const CMapStaticBatchObject& batch)
    {
        if (!batch.Is_FinalCameraPrepared() || !batch.m_pModelCom || !batch.m_pShaderCom ||
            batch.m_pModelCom->Get_NumMeshes() == 0u ||
            batch.m_bVisibleInstancesUsedCamera != hasCamera ||
            batch.m_iVisibleCameraRevision != cameraRevision || batch.m_VisibleInstances.empty() ||
            batch.m_VisibleInstances.size() > UINT32_MAX / sizeof(VTXMESHINSTANCE)) return false;
        for (uint32_t mesh = 0u; mesh < batch.m_pModelCom->Get_NumMeshes(); ++mesh)
            if (batch.Is_ChunkMeshClaimed(mesh)) return false;
        return true;
    };
    if (!ready(*this) || !m_pModelCom->Can_BatchStaticLightingWith(*m_pModelCom))
        return Render();

    // Match each ordinary draw's own LOD decision before combining instances.
    // Shared CMesh identity below makes equal levels select the same index range.
    Engine::MESH_SCREEN_LOD_DESC screenLod{};
    const bool_t hasScreenLod = camera && m_RenderProfile.opacity >= 1.f &&
        Build_ScreenLodView(*camera, screenLod);
    const uint32_t meshCount = m_pModelCom->Get_NumMeshes();

    std::array<const Engine::CModel*, 8u> models{};
    models[0] = m_pModelCom.get();
    size_t bankCount = 1u, count = 1u;
    size_t instanceCount = m_VisibleInstances.size();
    Engine::CProfiler* const profiler = game.Get_Profiler();
    try
    {
        m_CandidateLightingBankSlots.clear();
        m_CandidateLightingBankSlots.push_back(0u);
        for (; count < objects.size(); ++count)
        {
            const auto* next = dynamic_cast<const CMapStaticBatchObject*>(objects[count].get());
            // A different object is an ordering barrier; never scan or sort past it.
            if (!next || !ready(*next) || next->m_bMirrored != m_bMirrored ||
                next->Get_RenderElapsedTime() != Get_RenderElapsedTime() || !SameInstanceProfile(m_RenderProfile, next->m_RenderProfile) ||
                !m_pModelCom->Can_BatchStaticLightingWith(*next->m_pModelCom))
                break;
            Engine::MESH_SCREEN_LOD_DESC nextLod{};
            const bool_t hasNextLod = camera && next->m_RenderProfile.opacity >= 1.f &&
                next->Build_ScreenLodView(*camera, nextLod);
            bool_t matchingLods = true;
            for (uint32_t mesh = 0u; mesh < meshCount; ++mesh)
                if (m_pModelCom->Get_StaticMeshLodLevel(mesh,
                        Uses_MeshScreenLod(mesh, hasScreenLod) ? &screenLod : nullptr) !=
                    next->m_pModelCom->Get_StaticMeshLodLevel(mesh,
                        next->Uses_MeshScreenLod(mesh, hasNextLod) ? &nextLod : nullptr))
                { matchingLods = false; break; }
            if (!matchingLods) break;
            if (next->m_VisibleInstances.size() > UINT32_MAX / sizeof(VTXMESHINSTANCE) - instanceCount)
                break;
            size_t slot = 0u;
            for (; slot < bankCount; ++slot)
                if (models[slot]->Has_SameStaticLightingTextures(*next->m_pModelCom)) break;
            if (slot == bankCount)
            {
                if (bankCount == models.size()) break;
                models[bankCount++] = next->m_pModelCom.get();
            }
            m_CandidateLightingBankSlots.push_back(static_cast<uint8_t>(slot));
            instanceCount += next->m_VisibleInstances.size();
        }
    }
    catch (const std::bad_alloc&) { return Render(); }
    catch (const std::length_error&) { return Render(); }
    if (count < MINIMUM_LIGHTING_BANK_BATCHES)
        return Render();

    Engine::CProfilerWorkScope renderWork(profiler, Engine::EProfilerWork::MapBatchRender);
    try
    {
        m_CandidateLightingBankInstances.clear();
        m_CandidateLightingBankInstances.reserve(instanceCount);
        for (size_t batch = 0u; batch < count; ++batch)
        {
            const auto& source = static_cast<const CMapStaticBatchObject*>(objects[batch].get())->m_VisibleInstances;
            const size_t first = m_CandidateLightingBankInstances.size();
            m_CandidateLightingBankInstances.insert(m_CandidateLightingBankInstances.end(),
                source.begin(), source.end());
            // The single-bundle ordinary pass retains every original payload byte.
            if (bankCount > 1u)
                for (size_t i = first; i < m_CandidateLightingBankInstances.size(); ++i)
                    m_CandidateLightingBankInstances[i].vLightmapDirectionalScale.w =
                        static_cast<float>(m_CandidateLightingBankSlots[batch]);
        }
        if (FAILED(Upload_LightingBankInstances()))
            return Render();
    }
    catch (const std::bad_alloc&) { return Render(); }
    catch (const std::length_error&) { return Render(); }

    const HRESULT cameraBind = camera ?
        CMapAssetRenderUtils::Bind_CameraCullSnapshot(m_pShaderCom, *camera) :
        (FAILED(game.Bind_Transform(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
         FAILED(game.Bind_Transform(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)) ? E_FAIL : S_OK);
    if (FAILED(cameraBind))
        return Render();
    const uint32_t pass = CMapAssetRenderUtils::Select_Pass(m_RenderProfile, m_bMirrored);
    if (pass > 2u)
        return Render();
    const uint32_t noBank = 0u;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_MapLightingBankSize", &noBank, sizeof(noBank))))
        return Render();
    uint32_t submittedMeshes = 0u;
    const auto failedPreparation = [&](HRESULT failure)
    {
        // After any mesh was submitted, restarting ordinary Render would draw
        // that successful prefix again. Only an untouched batch may fall back.
        (void)m_pShaderCom->Bind_RawValue("g_MapLightingBankSize", &noBank, sizeof(noBank));
        return submittedMeshes == 0u ? Render() : failure;
    };
    for (uint32_t mesh = 0u; mesh < meshCount; ++mesh)
    {
        {
            Engine::CProfilerDetailScope scope(profiler, "Map.Batch.Material.Bind");
            Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchMaterial);
            HRESULT result = CMapAssetRenderUtils::Bind_Material(m_pModelCom, m_pShaderCom,
                mesh, m_RenderProfile, Get_RenderElapsedTime(), nullptr, m_AssetId, nullptr, nullptr,
                MAP_MATERIAL_BINDING_MODE::INSTANCED);
            if (FAILED(result)) return failedPreparation(result);
            if (bankCount > 1u)
            {
                result = m_pModelCom->Bind_StaticLightingBankMesh(m_pShaderCom,
                    std::span<const Engine::CModel* const>(models.data(), bankCount), mesh);
                if (FAILED(result)) return failedPreparation(result);
            }
        }
        {
            Engine::CProfilerDetailScope scope(profiler, "Map.Batch.Pass.Apply");
            Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchPass);
            const uint32_t bankPass = bankCount == 1u ? 24u : bankCount < 4u ? 30u : 27u;
            const HRESULT result = m_pShaderCom->Begin(bankPass + pass);
            if (FAILED(result)) return failedPreparation(result);
        }
        // Every constituent selected this mesh's same immutable index range
        // from its own visible bounds before any draw started.
        HRESULT result = E_FAIL;
        {
            Engine::CProfilerDetailScope scope(profiler, "Map.Batch.Mesh.Submit");
            Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchDraw);
            result = m_pModelCom->Render_Instanced(mesh, m_pLightingBankInstanceBuffer.Get(),
                sizeof(VTXMESHINSTANCE), static_cast<uint32_t>(instanceCount), 0u,
                Uses_MeshScreenLod(mesh, hasScreenLod) ? &screenLod : nullptr);
        }
        if (FAILED(result))
        {
            (void)m_pShaderCom->Bind_RawValue("g_MapLightingBankSize", &noBank, sizeof(noBank));
            return result;
        }
        ++submittedMeshes;
    }
    const HRESULT reset = m_pShaderCom->Bind_RawValue("g_MapLightingBankSize", &noBank, sizeof(noBank));
    consumed = count;
    if (profiler)
    {
        profiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibleRenders);
        profiler->Add_Counter(Engine::EProfilerCounter::MapLightingBankSourceDraws, count * meshCount);
        profiler->Add_Counter(Engine::EProfilerCounter::MapLightingBankDraws, meshCount);
    }
    return reset;
}

HRESULT CMapStaticBatchObject::Try_RenderIdenticalInstances(
    std::span<const std::shared_ptr<CGameObject>> objects, size_t& consumed)
{
    const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
    const auto ready = [&](const CMapStaticBatchObject& batch)
    {
        return batch.Is_FinalCameraPrepared() && batch.m_pModelCom && batch.m_pShaderCom &&
            batch.m_bVisibleInstancesUsedCamera == (camera != nullptr) &&
            batch.m_iVisibleCameraRevision == (camera ? camera->revision : 0u) &&
            !batch.m_VisibleInstances.empty() &&
            batch.m_VisibleInstances.size() <= UINT32_MAX / sizeof(VTXMESHINSTANCE);
    };
    if (objects.size() < 2u || !ready(*this) ||
        !m_pModelCom->Can_ShareStaticInstanceStateWith(*m_pModelCom)) return S_FALSE;
    Engine::MESH_SCREEN_LOD_DESC firstView{};
    const bool_t firstLod = camera && m_RenderProfile.opacity >= 1.f && Build_ScreenLodView(*camera, firstView);
    size_t count = 1u, instanceCount = m_VisibleInstances.size();
    uint32_t drawnMeshes = 0u;
    for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
        if (!Is_ChunkMeshClaimed(mesh)) ++drawnMeshes;
    if (drawnMeshes == 0u) return S_FALSE;
    for (; count < objects.size(); ++count)
    {
        const auto* next = dynamic_cast<const CMapStaticBatchObject*>(objects[count].get());
        // The existing submission order and every non-map object remain barriers.
        if (!next || !ready(*next) || next->m_bMirrored != m_bMirrored ||
            next->Get_RenderElapsedTime() != Get_RenderElapsedTime() ||
            !SameInstanceProfile(m_RenderProfile, next->m_RenderProfile) ||
            !m_pModelCom->Can_ShareStaticInstanceStateWith(*next->m_pModelCom) ||
            next->m_VisibleInstances.size() > UINT32_MAX / sizeof(VTXMESHINSTANCE) - instanceCount)
            break;
        Engine::MESH_SCREEN_LOD_DESC nextView{};
        const bool_t nextLod = camera && next->m_RenderProfile.opacity >= 1.f &&
            next->Build_ScreenLodView(*camera, nextView);
        bool_t matching = true;
        for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
        {
            const bool_t claimed = Is_ChunkMeshClaimed(mesh);
            if (claimed != next->Is_ChunkMeshClaimed(mesh)) { matching = false; break; }
            if (claimed) continue;
            if (m_pModelCom->Get_StaticMeshLodLevel(mesh, Uses_MeshScreenLod(mesh, firstLod) ? &firstView : nullptr) !=
                next->m_pModelCom->Get_StaticMeshLodLevel(mesh, next->Uses_MeshScreenLod(mesh, nextLod) ? &nextView : nullptr))
            { matching = false; break; }
        }
        if (!matching) break;
        instanceCount += next->m_VisibleInstances.size();
    }
    if (count < 2u) return S_FALSE;
    // An exact prefix uses one lighting bundle regardless of its batch count.
    // Let the bank include a following compatible variant in the same draw.
    if (CGameInstance::Get().Get_RenderOptimizationSettings().LightingBankEnabled &&
        count < objects.size() && drawnMeshes == m_pModelCom->Get_NumMeshes() &&
        m_pModelCom->Can_BatchStaticLightingWith(*m_pModelCom))
    {
        const auto* next = dynamic_cast<const CMapStaticBatchObject*>(objects[count].get());
        if (next && ready(*next) && next->m_bMirrored == m_bMirrored &&
            next->Get_RenderElapsedTime() == Get_RenderElapsedTime() &&
            SameInstanceProfile(m_RenderProfile, next->m_RenderProfile) &&
            m_pModelCom->Can_BatchStaticLightingWith(*next->m_pModelCom) &&
            next->m_VisibleInstances.size() <= UINT32_MAX / sizeof(VTXMESHINSTANCE) - instanceCount)
        {
            Engine::MESH_SCREEN_LOD_DESC nextView{};
            const bool_t nextLod = camera && next->m_RenderProfile.opacity >= 1.f &&
                next->Build_ScreenLodView(*camera, nextView);
            bool_t extendsLightingBank = true;
            for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
            {
                if (next->Is_ChunkMeshClaimed(mesh) ||
                    m_pModelCom->Get_StaticMeshLodLevel(mesh, Uses_MeshScreenLod(mesh, firstLod) ? &firstView : nullptr) !=
                    next->m_pModelCom->Get_StaticMeshLodLevel(mesh, next->Uses_MeshScreenLod(mesh, nextLod) ? &nextView : nullptr))
                { extendsLightingBank = false; break; }
            }
            if (extendsLightingBank) return S_FALSE;
        }
    }
    try
    {
        m_CandidateLightingBankInstances.clear();
        m_CandidateLightingBankInstances.reserve(instanceCount);
        for (size_t i = 0u; i < count; ++i)
        {
            const auto& instances = static_cast<const CMapStaticBatchObject*>(objects[i].get())->m_VisibleInstances;
            m_CandidateLightingBankInstances.insert(m_CandidateLightingBankInstances.end(), instances.begin(), instances.end());
        }
        if (FAILED(Upload_LightingBankInstances())) return S_FALSE;
    }
    catch (const std::bad_alloc&) { return S_FALSE; }
    catch (const std::length_error&) { return S_FALSE; }
    const uint32_t noBank = 0u;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_MapLightingBankSize", &noBank, sizeof(noBank)))) return S_FALSE;
    auto* profiler = CGameInstance::Get().Get_Profiler();
    Engine::CProfilerWorkScope renderWork(profiler, Engine::EProfilerWork::MapBatchRender);
    const HRESULT result = Render_VisibleInstances(m_pLightingBankInstanceBuffer.Get(),
        static_cast<uint32_t>(instanceCount), camera);
    // Once submission starts, a failure must not draw a successful prefix twice.
    if (FAILED(result)) return result;
    consumed = count;
    if (profiler)
    {
        profiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibleRenders);
        profiler->Add_Counter(Engine::EProfilerCounter::MapIdenticalInstanceSourceDraws, count * drawnMeshes);
        profiler->Add_Counter(Engine::EProfilerCounter::MapIdenticalInstanceDraws, drawnMeshes);
    }
    return result;
}

HRESULT CMapStaticBatchObject::Upload_LightingBankInstances()
{
    const size_t required = m_CandidateLightingBankInstances.size();
    if (required == 0u || required > UINT32_MAX / sizeof(VTXMESHINSTANCE))
        return E_INVALIDARG;
    const bool_t samePayload = required == m_LightingBankInstances.size() &&
        0 == std::memcmp(m_CandidateLightingBankInstances.data(), m_LightingBankInstances.data(),
            required * sizeof(VTXMESHINSTANCE));
    if (samePayload && m_pLightingBankInstanceBuffer)
        return S_OK;

    ComPtr<ID3D11Buffer> target = m_pLightingBankInstanceBuffer;
    uint32_t capacity = m_iLightingBankInstanceCapacity;
    if (!target || required > capacity)
    {
        capacity = 1u;
        const uint32_t maximum = UINT32_MAX / sizeof(VTXMESHINSTANCE);
        while (capacity < required && capacity <= maximum / 2u) capacity *= 2u;
        if (capacity < required) capacity = static_cast<uint32_t>(required);
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = capacity * sizeof(VTXMESHINSTANCE);
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (FAILED(m_pDevice->CreateBuffer(&desc, nullptr, target.ReleaseAndGetAddressOf())))
            return E_FAIL;
    }
    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (FAILED(m_pContext->Map(target.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mapped)))
        return E_FAIL;
    std::memcpy(mapped.pData, m_CandidateLightingBankInstances.data(), required * sizeof(VTXMESHINSTANCE));
    m_pContext->Unmap(target.Get(), 0u);
    m_pLightingBankInstanceBuffer = std::move(target);
    m_iLightingBankInstanceCapacity = capacity;
    m_LightingBankInstances.swap(m_CandidateLightingBankInstances);
    if (auto* profiler = CGameInstance::Get().Get_Profiler())
        profiler->Add_Counter(Engine::EProfilerCounter::MapBatchUploadBytes,
            required * sizeof(VTXMESHINSTANCE));
    return S_OK;
}

HRESULT CMapStaticBatchObject::Render()
{
	Engine::CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
	Engine::CProfilerWorkScope renderWork(profiler, Engine::EProfilerWork::MapBatchRender);
	const uint32_t noBank = 0u;
	if (!m_pShaderCom || FAILED(m_pShaderCom->Bind_RawValue("g_MapLightingBankSize", &noBank, sizeof(noBank))))
		return E_FAIL;
	if (CGameInstance::Get().Is_SceneEnvironmentReplaced())
		return S_OK;

	const MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot =
		CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
	if (FAILED(Prepare_FinalCameraVisibility(cameraSnapshot)))
	{
		return E_FAIL;
	}
	if (m_VisibleInstances.empty())
	{
		if (profiler) profiler->Add_Counter(Engine::EProfilerCounter::MapBatchEmptyRenders);
		return S_OK;
	}
	if (profiler) profiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibleRenders);

    return Render_VisibleInstances(m_pInstanceBuffer.Get(),
        static_cast<uint32_t>(m_VisibleInstances.size()), cameraSnapshot);
}

bool_t CMapStaticBatchObject::Uses_MeshScreenLod(uint32_t meshIndex, bool_t hasScreenLod) const
{
    const auto* surface = m_pModelCom->Get_MaterialSurface(meshIndex);
    return hasScreenLod && CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials && surface &&
        surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
        !surface->sourceFoliageWind && (surface->sourceBgFlags & 64u) == 0u &&
        (surface->renderMode == Engine::MODEL_SURFACE_RENDER_MODE::INHERIT ||
         surface->renderMode == Engine::MODEL_SURFACE_RENDER_MODE::DEFERRED) &&
        !m_pModelCom->Has_MaterialTexture(meshIndex, aiTextureType_OPACITY);
}

HRESULT CMapStaticBatchObject::Render_VisibleInstances(ID3D11Buffer* buffer, uint32_t instanceCount,
    const MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot)
{
    Engine::CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    const bool instancing = CGameInstance::Get().Get_RenderOptimizationSettings().MapInstancingEnabled;
    const bool_t hasCameraSnapshot = cameraSnapshot != nullptr;
	const HRESULT cameraBindResult = hasCameraSnapshot ?
		CMapAssetRenderUtils::Bind_CameraCullSnapshot(
			m_pShaderCom, *cameraSnapshot) :
		(FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		 FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)) ? E_FAIL : S_OK);
	if (FAILED(cameraBindResult))
	{
		return E_FAIL;
	}

	const uint32_t passIndex =
		CMapAssetRenderUtils::Select_Pass(
			m_RenderProfile,
			m_bMirrored);

	if (passIndex > 2u)
		return E_UNEXPECTED;

    Engine::MESH_SCREEN_LOD_DESC screenLod{};
    const bool_t hasScreenLod = hasCameraSnapshot && m_RenderProfile.opacity >= 1.f &&
        Build_ScreenLodView(*cameraSnapshot, screenLod);
	const bool_t useSourceMaterials =
		CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials;
	{
		Engine::CProfilerDetailScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.BindAndDraw");
	for (uint32_t meshIndex = 0;
		meshIndex < m_pModelCom->Get_NumMeshes();
		++meshIndex)
	{
        if (Is_ChunkMeshClaimed(meshIndex)) continue;
		const auto* surface = m_pModelCom->Get_MaterialSurface(meshIndex);
        // Material variants share CMesh geometry: re-admit the current draw's
        // material instead of inheriting the source model's LOD eligibility.
        const bool_t useMeshLod = Uses_MeshScreenLod(meshIndex, hasScreenLod);
		const uint32_t meshPass = useSourceMaterials && surface &&
			surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED ?
			24u + passIndex : passIndex;
		{
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Batch.Material.Bind");
			Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchMaterial);
			if (FAILED(CMapAssetRenderUtils::Bind_Material(m_pModelCom, m_pShaderCom,
				meshIndex, m_RenderProfile, Get_RenderElapsedTime(), nullptr, m_AssetId, nullptr, nullptr,
				MAP_MATERIAL_BINDING_MODE::INSTANCED))) return E_FAIL;
		}
		{
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Batch.Pass.Apply");
			Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchPass);
			if (FAILED(m_pShaderCom->Begin(meshPass))) return E_FAIL;
		}
        // Keep the exact payload, mesh, material and LOD. The baseline removes
        // instance aggregation only; it does not invent a second object runtime.
        const uint32_t draws = instancing ? 1u : instanceCount;
        for (uint32_t instance = 0u; instance < draws; ++instance)
        {
            Engine::CProfilerDetailScope scope(profiler, "Map.Batch.Mesh.Submit");
            Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchDraw);
            if (FAILED(m_pModelCom->Render_Instanced(meshIndex, buffer,
                sizeof(VTXMESHINSTANCE), instancing ? instanceCount : 1u,
                instancing ? 0u : instance * sizeof(VTXMESHINSTANCE),
                useMeshLod ? &screenLod : nullptr))) return E_FAIL;
        }
	}
	}

	return S_OK;
}

HRESULT CMapStaticBatchObject::Render_Shadow()
{
	if (CGameInstance::Get().Is_SceneEnvironmentReplaced())
		return S_OK;

	if (!m_RenderProfile.castsShadow)
		return S_OK;
	// Keep the failing asset and operation; the renderer only knows the object type.
	const auto fail = [this](const char* stage, HRESULT result,
		uint32_t mesh = UINT_MAX, uint32_t pass = UINT_MAX) noexcept -> HRESULT
	{
		try
		{
			std::ostringstream detail;
			detail << "stage=" << stage << " asset=" << std::quoted(m_AssetId)
				<< " mesh=" << mesh << " pass=" << pass
				<< " instances=" << m_ShadowInstances.size()
				<< " hr=0x" << std::hex << static_cast<unsigned long>(result)
				<< " device_hr=0x" << static_cast<unsigned long>(m_pDevice->GetDeviceRemovedReason());
			Write_EffectFailureDiagnostic("Map.StaticBatch.Shadow", detail.str());
		}
		catch (...) { }
		return result;
	};
	// Frame providers can change the light after Late_Update queued this batch.
	HRESULT result = Upload_ShadowInstances();
	if (FAILED(result))
		return fail("UploadInstances", result);
	if (m_ShadowInstances.empty())
		return S_OK;
	const bool_t useSourceMaterials =
		CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials;

	result = CGameInstance::Get().Bind_ShadowLight_ShaderResource(
		m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW);
	if (FAILED(result)) return fail("BindLightView", result);
	result = CGameInstance::Get().Bind_ShadowLight_ShaderResource(
		m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ);
	if (FAILED(result)) return fail("BindLightProjection", result);

	const uint32_t iCullPass =
		CMapAssetRenderUtils::Select_Pass(
			m_RenderProfile, m_bMirrored);
	if (iCullPass > 2u)
		return fail("SelectPass", E_UNEXPECTED);

	const uint32_t iInstanceCount =
		static_cast<uint32_t>(m_ShadowInstances.size());
    const bool instancing = CGameInstance::Get().Get_RenderOptimizationSettings().MapInstancingEnabled;
	for (uint32_t iMesh = 0;
		iMesh < m_pModelCom->Get_NumMeshes(); ++iMesh)
	{
		const auto* surface = m_pModelCom->Get_MaterialSurface(iMesh);
		if (surface && !surface->castsShadow)
			continue;
		const bool_t opaqueShadow = CMapAssetRenderUtils::Uses_OpaqueShadowPass(
			surface, m_RenderProfile, useSourceMaterials);
		uint32_t shadowPassBase = opaqueShadow ? 21u : 12u;
		if (!opaqueShadow)
		{
			// Simple families retain their alpha test when source mode is off or masked.
			if (!surface || surface->family == Engine::MODEL_SURFACE_FAMILY::LEGACY ||
				surface->family == Engine::MODEL_SURFACE_FAMILY::SPECULAR_TEXTURE_REFLECTION ||
				surface->family == Engine::MODEL_SURFACE_FAMILY::DIFFUSE_SPECULAR_REFLECTION ||
				surface->family == Engine::MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE ||
				surface->family == Engine::MODEL_SURFACE_FAMILY::PBR_OPAQUE ||
				surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE)
				shadowPassBase = 18u;
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Shadow.Material.Bind");
			result = CMapAssetRenderUtils::Bind_ShadowMaterial(m_pModelCom, m_pShaderCom,
				iMesh, m_RenderProfile, Get_RenderElapsedTime());
			if (FAILED(result)) return fail("BindMaterial", result, iMesh, shadowPassBase + iCullPass);
		}
		{
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Shadow.Pass.Apply");
			result = m_pShaderCom->Begin(shadowPassBase + iCullPass);
			if (FAILED(result)) return fail("ApplyPass", result, iMesh, shadowPassBase + iCullPass);
		}
        const uint32_t draws = instancing ? 1u : iInstanceCount;
        for (uint32_t instance = 0u; instance < draws; ++instance)
        {
            Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Shadow.Mesh.Submit");
            result = m_pModelCom->Render_Instanced(iMesh, m_pShadowInstanceBuffer.Get(),
                sizeof(VTXMESHINSTANCE), instancing ? iInstanceCount : 1u,
                instancing ? 0u : instance * sizeof(VTXMESHINSTANCE));
            if (FAILED(result)) return fail("DrawMesh", result, iMesh, shadowPassBase + iCullPass);
        }
	}

	return S_OK;
}

bool_t CMapStaticBatchObject::Try_GetStaticShadowRevision(uint64_t& outRevision) const
{
	outRevision = 0u;
	if (m_iStaticShadowRevision == 0u || !m_RenderProfile.castsShadow ||
		!m_pModelCom || m_pModelCom->Get_NumMeshes() == 0u ||
		!CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials)
		return false;

	if (!m_bStaticShadowMaterialInputs)
		return false;
	// Surface/profile constants were admitted when this model was staged.
	// Mutable texture overrides and morph clones still invalidate cache use.
	for (const uint32_t mesh : m_StaticShadowCasterMeshes)
	{
		if (m_pModelCom->Has_MorphBaseVertices(mesh) ||
			m_pModelCom->Has_MaterialTextureOverrides(mesh))
			return false;
	}

	outRevision = m_iStaticShadowRevision;
	return true;
}

bool_t CMapStaticBatchObject::Try_PickMovementSurface(
	const float3_t& rayOrigin, const float3_t& rayDirection,
	const f32_t maxDistance, f32_t& outDistance) const
{
	if (!m_pModelCom || m_RenderProfile.opacity <= 0.f ||
		m_RenderProfile.renderMode != MAP_ASSET_RENDER_MODE::DEFERRED)
		return false;
	const vector_t origin = XMLoadFloat3(&rayOrigin);
	const vector_t direction = XMLoadFloat3(&rayDirection);
	// The cached envelope contains every authored-visible instance. A dirty
	// envelope cannot reject the current transform, so retain the instance scan.
	if (!m_bBatchBoundsDirty && m_bHasBatchBounds)
	{
		const BoundingBox bounds(
			float3_t(m_BatchBounds.x, m_BatchBounds.y, m_BatchBounds.z),
			float3_t(m_BatchBounds.w, m_BatchBounds.w, m_BatchBounds.w));
		f32_t entry = 0.f;
		if (!bounds.Intersects(origin, direction, entry) || entry > maxDistance)
			return false;
	}
	const uint32_t cull = CMapAssetRenderUtils::Select_Pass(m_RenderProfile, m_bMirrored) % 3u;
	const auto cullMode = cull == 0u ? CModel::PICK_CULL_MODE::BACK :
		cull == 1u ? CModel::PICK_CULL_MODE::FRONT : CModel::PICK_CULL_MODE::NONE;
	f32_t nearest = maxDistance;
	bool_t hit = false;
	for (const auto& instance : m_Instances)
	{
		if (!instance.Visible || instance.Suppressed || instance.CameraPreviewSuppressed)
			continue;
		const BoundingBox bounds(instance.WorldBoundsCenter, float3_t(
			instance.WorldBoundsRadius, instance.WorldBoundsRadius, instance.WorldBoundsRadius));
		f32_t boundDistance = 0.f;
		if (!bounds.Intersects(origin, direction, boundDistance) || boundDistance > nearest)
			continue;
		for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
		{
			const auto* surface = m_pModelCom->Get_MaterialSurface(mesh);
			// Masked floor geometry remains eligible; GPU alpha coverage is not
			// the movement contract. Animated foliage and shader displacement are excluded.
			if (m_pModelCom->Has_MorphBaseVertices(mesh) ||
				(surface && (surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED ||
					surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED ||
					(surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
						surface->sourceCharacter.program == 43u))))
				continue;
			f32_t distance = nearest;
			if (m_pModelCom->Try_PickStaticSurface(mesh, instance.World, rayOrigin, rayDirection,
				nearest, cullMode, distance) && distance < nearest)
			{
				nearest = distance;
				hit = true;
			}
		}
	}
	if (hit) outDistance = nearest;
	return hit;
}

#ifdef _DEBUG
bool_t CMapStaticBatchObject::Try_PickInspectionSurface(
    const float3_t& rayOrigin, const float3_t& rayDirection,
    const f32_t maxDistance, f32_t& outDistance,
    uint64_t& outPlacementId, uint32_t& outMeshIndex, std::string& outMaterialName) const
{
    if (!m_pModelCom || m_RenderProfile.opacity <= 0.f) return false;
    const vector_t origin = XMLoadFloat3(&rayOrigin);
    const vector_t direction = XMLoadFloat3(&rayDirection);
    if (!m_bBatchBoundsDirty && m_bHasBatchBounds)
    {
        const BoundingBox bounds(float3_t(m_BatchBounds.x, m_BatchBounds.y, m_BatchBounds.z),
            float3_t(m_BatchBounds.w, m_BatchBounds.w, m_BatchBounds.w));
        f32_t entry = 0.f;
        if (!bounds.Intersects(origin, direction, entry) || entry > maxDistance) return false;
    }
    const uint32_t cull = CMapAssetRenderUtils::Select_Pass(m_RenderProfile, m_bMirrored) % 3u;
    const auto mode = cull == 0u ? CModel::PICK_CULL_MODE::BACK :
        cull == 1u ? CModel::PICK_CULL_MODE::FRONT : CModel::PICK_CULL_MODE::NONE;
    f32_t nearest = maxDistance;
    uint64_t nearestPlacement = 0u;
    uint32_t nearestMesh = 0u;
    bool_t hit = false;
    for (const auto& instance : m_Instances)
    {
        if (!instance.Visible || instance.Suppressed || instance.CameraPreviewSuppressed) continue;
        const BoundingBox bounds(instance.WorldBoundsCenter, float3_t(
            instance.WorldBoundsRadius, instance.WorldBoundsRadius, instance.WorldBoundsRadius));
        f32_t entry = 0.f;
        if (!bounds.Intersects(origin, direction, entry) || entry > nearest) continue;
        for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
        {
            f32_t distance = nearest;
            if (m_pModelCom->Try_PickStaticSurface(mesh, instance.World, rayOrigin, rayDirection,
                nearest, mode, distance) && distance < nearest)
            { nearest = distance; nearestPlacement = instance.PlacementId; nearestMesh = mesh; hit = true; }
        }
    }
    if (!hit) return false;
    outDistance = nearest;
    outPlacementId = nearestPlacement;
    outMeshIndex = nearestMesh;
    outMaterialName = m_pModelCom->Get_MaterialName(nearestMesh);
    return true;
}
#endif

HRESULT CMapStaticBatchObject::Update_Instance(
	uint64_t placementId,
	const FMapStaticInstance& instance)
{
	const auto iter =
		m_PlacementLookup.find(placementId);

	if (iter == m_PlacementLookup.end() ||
		instance.PlacementId != placementId)
	{
		return E_INVALIDARG;
	}

    Invalidate_ChunkClaims();
	FMapStaticInstance& current = m_Instances[iter->second];
	// Bounds participate in light-volume culling even when the world is unchanged.
	const bool_t shadowChanged = current.Visible != instance.Visible ||
		0 != std::memcmp(&current.World, &instance.World, sizeof(current.World)) ||
		0 != std::memcmp(&current.WorldInvTranspose, &instance.WorldInvTranspose,
			sizeof(current.WorldInvTranspose)) ||
		0 != std::memcmp(&current.WorldBoundsCenter, &instance.WorldBoundsCenter,
			sizeof(current.WorldBoundsCenter)) ||
		current.WorldBoundsRadius != instance.WorldBoundsRadius;
	if (shadowChanged && m_iStaticShadowRevision != 0u)
		++m_iStaticShadowRevision;
	if (current.Visible != instance.Visible)
	{
		if (instance.Visible)
			++m_iAuthoredVisibleInstanceCount;
		else
			--m_iAuthoredVisibleInstanceCount;
	}
	const bool_t suppressed = current.Suppressed;
	const bool_t cameraPreviewSuppressed = current.CameraPreviewSuppressed;
	current = instance;
	current.Suppressed = suppressed;
	current.CameraPreviewSuppressed = cameraPreviewSuppressed;
    m_InstanceLinearScaleBounds[iter->second] = LinearScaleBound(current.World);
    current.DistanceHidden = false; current.DistanceSettingsRevision = 0u;
    Rebuild_InstanceOcclusionBounds(current);
    m_bBatchBoundsDirty = true;
	m_bShadowInstancesDirty = true;
	m_bVisibleInstancesDirty = true;
	m_bFinalCameraPrepared = false;
    Invalidate_VisibilityCpu();
    Invalidate_FinalCameraSpatialBounds();
	return S_OK;
}

HRESULT CMapStaticBatchObject::Set_InstanceVisible(
	uint64_t placementId,
	bool_t visible)
{
	const auto iter =
		m_PlacementLookup.find(placementId);

	if (iter == m_PlacementLookup.end())
		return HRESULT_FROM_WIN32(
			ERROR_NOT_FOUND);

	FMapStaticInstance& instance = m_Instances[iter->second];
	if (instance.Visible != visible)
	{
        Invalidate_ChunkClaims();
        Invalidate_FinalCameraSpatialBounds();
		if (visible)
			++m_iAuthoredVisibleInstanceCount;
		else
			--m_iAuthoredVisibleInstanceCount;
		instance.Visible = visible;
        instance.DistanceHidden = false; instance.DistanceSettingsRevision = 0u;
		if (m_iStaticShadowRevision != 0u)
			++m_iStaticShadowRevision;
        m_bBatchBoundsDirty = true;
		m_bShadowInstancesDirty = true;
		m_bVisibleInstancesDirty = true;
		m_bFinalCameraPrepared = false;
    Invalidate_VisibilityCpu();
	}
	return S_OK;
}

HRESULT CMapStaticBatchObject::Try_GetInstanceVisible(
	const uint64_t placementId,
	bool_t& outVisible) const
{
	const auto iter = m_PlacementLookup.find(placementId);
	if (iter == m_PlacementLookup.end() || iter->second >= m_Instances.size())
		return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
	outVisible = m_Instances[iter->second].Visible;
	return S_OK;
}

HRESULT CMapStaticBatchObject::Set_InstanceSuppressed(
	const uint64_t placementId,
	const bool_t suppressed)
{
	const auto iter = m_PlacementLookup.find(placementId);
	if (iter == m_PlacementLookup.end() || iter->second >= m_Instances.size())
		return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

	FMapStaticInstance& instance = m_Instances[iter->second];
	if (instance.Suppressed != suppressed)
	{
        Invalidate_ChunkClaims();
        Invalidate_FinalCameraSpatialBounds();
		instance.Suppressed = suppressed;
        instance.DistanceHidden = false; instance.DistanceSettingsRevision = 0u;
		// Batch bounds stay conservative (they still cover this instance); only
		// the draw and shadow payloads and the cached static shadow change.
		if (m_iStaticShadowRevision != 0u)
			++m_iStaticShadowRevision;
		m_bShadowInstancesDirty = true;
		m_bVisibleInstancesDirty = true;
		m_bFinalCameraPrepared = false;
    Invalidate_VisibilityCpu();
	}
	return S_OK;
}

HRESULT CMapStaticBatchObject::Set_InstanceCameraPreviewSuppressed(
	const uint64_t placementId,
	const bool_t suppressed)
{
	const auto iter = m_PlacementLookup.find(placementId);
	if (iter == m_PlacementLookup.end() || iter->second >= m_Instances.size())
		return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

	FMapStaticInstance& instance = m_Instances[iter->second];
	if (instance.CameraPreviewSuppressed != suppressed)
	{
        Invalidate_ChunkClaims();
        Invalidate_FinalCameraSpatialBounds();
		instance.CameraPreviewSuppressed = suppressed;
        instance.DistanceHidden = false; instance.DistanceSettingsRevision = 0u;
		if (m_iStaticShadowRevision != 0u)
			++m_iStaticShadowRevision;
		m_bShadowInstancesDirty = true;
		m_bVisibleInstancesDirty = true;
		m_bFinalCameraPrepared = false;
    Invalidate_VisibilityCpu();
	}
	return S_OK;
}

HRESULT CMapStaticBatchObject::Ready_Components(
	uint32_t prototypeLevelIndex,
	const std::wstring& modelPrototypeTag)
{
	if (FAILED(__super::Add_Component(
		prototypeLevelIndex,
		TEXT(
			"Prototype_Component_Shader_VtxMeshMapInstance"),
		TEXT("Com_Shader"),
		m_pShaderCom)) ||

		FAILED(__super::Add_Component(
			prototypeLevelIndex,
			modelPrototypeTag,
			TEXT("Com_Model"),
			m_pModelCom)))
	{
		return E_FAIL;
	}

    // Distance is a separate presentation policy: static masked foliage/grass
    // remain eligible. Semantic landscape/background and gameplay models do not.
    m_bDistanceEligible = m_FrameState && m_AssetGroupId != "landscape" && m_RenderProfile.opacity == 1.f;
    m_DistanceSourceIndices = 0u;
    m_bOcclusionGeometryEligible = m_FrameState && m_pModelCom->Has_LocalBounds();
    for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
    {
        m_DistanceSourceIndices += m_pModelCom->Get_MeshIndexCount(mesh);
        m_bOcclusionGeometryEligible &= StaticOcclusionBoundsMesh(*m_pModelCom, mesh);
        const auto* surface = m_pModelCom->Get_MaterialSurface(mesh);
        if (surface && (surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_LANDSCAPE_OPAQUE ||
            surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER ||
            (surface->renderMode != Engine::MODEL_SURFACE_RENDER_MODE::INHERIT &&
             surface->renderMode != Engine::MODEL_SURFACE_RENDER_MODE::DEFERRED))) m_bDistanceEligible = false;
    }

	m_StaticShadowCasterMeshes.clear();
	m_bStaticShadowMaterialInputs = true;
	for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
	{
		const auto* surface = m_pModelCom->Get_MaterialSurface(mesh);
		if (surface && !surface->castsShadow)
			continue;
		m_StaticShadowCasterMeshes.push_back(mesh);
		m_bStaticShadowMaterialInputs &=
			CMapAssetRenderUtils::Uses_StaticShadowInputs(surface, m_RenderProfile, true);
	}
	m_bStaticShadowMaterialInputs &= !m_StaticShadowCasterMeshes.empty();

	// Small or unsimplifiable meshes never consume tight LOD bounds. Keep the
	// ordinary world envelope/culling and profiler draw denominators unchanged.
	m_bHasStaticMeshLod = false;
	for (uint32_t meshIndex = 0u; meshIndex < m_pModelCom->Get_NumMeshes(); ++meshIndex)
	{
		if (m_pModelCom->Has_StaticMeshLod(meshIndex))
		{
			m_bHasStaticMeshLod = true;
			break;
		}
	}
	return S_OK;
}

HRESULT CMapStaticBatchObject::Ensure_InstanceCapacity(
	uint32_t requiredCount)
{
	if (0 == requiredCount)
		return E_INVALIDARG;

	if (requiredCount <= m_iInstanceCapacity &&
		nullptr != m_pInstanceBuffer)
	{
		return S_OK;
	}

	uint32_t newCapacity = 1u;

	while (newCapacity < requiredCount)
		newCapacity <<= 1u;

	const uint64_t byteWidth =
		static_cast<uint64_t>(newCapacity) *
		sizeof(VTXMESHINSTANCE);

	if (byteWidth >
		(std::numeric_limits<uint32_t>::max)())
	{
		return E_OUTOFMEMORY;
	}

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth =
		static_cast<uint32_t>(byteWidth);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags =
		D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags =
		D3D11_CPU_ACCESS_WRITE;

	ComPtr<ID3D11Buffer> stagedBuffer;

	if (FAILED(m_pDevice->CreateBuffer(
		&bufferDesc,
		nullptr,
		&stagedBuffer)))
	{
		return E_FAIL;
	}

	m_pInstanceBuffer =
		std::move(stagedBuffer);

	m_iInstanceCapacity =
		newCapacity;

	m_VisibleInstances.reserve(
		newCapacity);
	m_CandidateVisibleInstances.reserve(
		newCapacity);
    if (m_bTrackProxySources)
    {
        m_VisibleInstanceIndices.reserve(newCapacity);
        m_CandidateVisibleInstanceIndices.reserve(newCapacity);
    }

	return S_OK;
}

HRESULT CMapStaticBatchObject::Ensure_ShadowInstanceCapacity(
	uint32_t requiredCount)
{
	if (0 == requiredCount)
		return E_INVALIDARG;

	if (requiredCount <= m_iShadowInstanceCapacity &&
		nullptr != m_pShadowInstanceBuffer)
	{
		return S_OK;
	}

	uint32_t newCapacity = 1u;
	while (newCapacity < requiredCount)
		newCapacity <<= 1u;

	const uint64_t byteWidth =
		static_cast<uint64_t>(newCapacity) *
		sizeof(VTXMESHINSTANCE);
	if (byteWidth >
		(std::numeric_limits<uint32_t>::max)())
	{
		return E_OUTOFMEMORY;
	}

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = static_cast<uint32_t>(byteWidth);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ComPtr<ID3D11Buffer> stagedBuffer;
	if (FAILED(m_pDevice->CreateBuffer(
		&bufferDesc, nullptr, &stagedBuffer)))
	{
		return E_FAIL;
	}

	m_pShadowInstanceBuffer = std::move(stagedBuffer);
	m_iShadowInstanceCapacity = newCapacity;
	m_ShadowInstances.reserve(newCapacity);
	m_CandidateShadowInstances.reserve(newCapacity);
	return S_OK;
}

void CMapStaticBatchObject::Invalidate_VisibilityCpu()
{
    if (m_CpuVisibility) m_CpuVisibility->State = CPU_VISIBILITY_PREPARATION::STATE::EMPTY;
}

HRESULT CMapStaticBatchObject::Stage_VisibilityCpu(const MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot)
{
    auto& game = CGameInstance::Get();
    const auto settings = game.Get_MapVisibilitySettings();
    const auto viewport = game.Get_ViewportSize();
    const bool hasCamera = cameraSnapshot != nullptr;
    const uint64_t cameraRevision = hasCamera ? cameraSnapshot->revision : 0u;
    const float4_t* position = hasCamera ? game.Get_CamPosition() : nullptr;
    const bool distanceEnabled = m_bDistanceEligible && position && settings.DistanceEnabled &&
        !m_FrustumCulling.bypass && !m_FrustumCulling.diagnostics &&
        !CMapAssetRenderUtils::Is_SurfaceBindingCollectionActive();
    const uint64_t frame = m_FrameState ? m_FrameState->frameNumber : 0u;
    const bool alreadyPrepared = m_bFinalCameraPrepared && (!m_FrameState || m_iPreparedFrame == frame);
    if ((alreadyPrepared || !m_bVisibleInstancesDirty) &&
        m_bVisibleInstancesUsedCamera == hasCamera && m_iVisibleCameraRevision == cameraRevision &&
        m_iVisibilitySettingsRevision == settings.Revision && m_bVisibleDistanceEnabled == distanceEnabled &&
        m_VisibleViewportSize.x == viewport.x && m_VisibleViewportSize.y == viewport.y)
    {
        if (!alreadyPrepared)
        {
            m_bFinalCameraPrepared = true;
            m_iPreparedFrame = m_FrameState ? m_FrameState->frameNumber : 0u;
            if (auto* profiler = game.Get_Profiler())
            {
                profiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibilityCacheHits);
                profiler->Add_Counter(Engine::EProfilerCounter::MapDistanceRejectedInstances, m_DistanceRejectedInstances);
                profiler->Add_Counter(Engine::EProfilerCounter::MapDistanceRejectedIndices, m_DistanceRejectedInstances * m_DistanceSourceIndices);
                profiler->Add_Counter(Engine::EProfilerCounter::MapVisibleInstances, m_VisibleInstances.size());
            }
        }
        return S_FALSE;
    }

    if (m_CpuVisibility)
    {
        const auto& previous = *m_CpuVisibility;
        if (previous.State != CPU_VISIBILITY_PREPARATION::STATE::EMPTY && previous.Frame == frame &&
            previous.HasCamera == hasCamera && previous.CameraRevision == cameraRevision &&
            previous.Settings.Revision == settings.Revision && previous.DistanceEnabled == distanceEnabled &&
            previous.Viewport.x == viewport.x && previous.Viewport.y == viewport.y)
            return previous.State == CPU_VISIBILITY_PREPARATION::STATE::FAILED ? previous.Result : S_OK;
    }
    // Most shadow candidates are outside the camera. Resolve their cheap broad
    // rejection before allocating/copying a detailed CPU job for every tiny batch.
    if (m_bBatchBoundsDirty) Rebuild_BatchCullBounds();
    MAP_FRUSTUM_RUNTIME_STATE batchFrustum = m_BatchFrustumState;
    bool requiresNextTick = false;
    if (hasCamera && m_bHasBatchBounds && !m_FrustumCulling.bypass)
    {
        MAP_FRUSTUM_CULL_DECISION decision{};
        MAP_FRUSTUM_CULLING_POLICY policy = m_FrustumCulling;
        policy.diagnostics = false;
        const float3_t center(m_BatchBounds.x, m_BatchBounds.y, m_BatchBounds.z);
        if (CMapAssetRenderUtils::Evaluate_FrustumVisibility(policy, *cameraSnapshot,
            m_AssetId, m_AssetGroupId, 0u, center, m_BatchBounds.w, batchFrustum,
            decision, nullptr, MAP_FRUSTUM_CULL_DETAIL::VISIBILITY_ONLY))
        {
            requiresNextTick = !decision.wouldBeVisible && decision.shouldRender;
            if (!decision.shouldRender && !m_FrustumCulling.diagnostics)
            {
                // Empty payload has no Map failure point. Commit only the broad
                // state; per-instance grace and distance state were not evaluated.
                m_BatchFrustumState = batchFrustum;
                m_VisibleInstances.clear(); m_VisibleInstanceIndices.clear();
                m_VisibleLodBounds = {}; m_VisibleTightLodBounds = {}; m_fVisibleLodScale = 0.f;
                m_bVisibleOcclusionBounds = false;
                m_VisibleOcclusionMin = {FLT_MAX, FLT_MAX, FLT_MAX};
                m_VisibleOcclusionMax = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
                m_bVisibleInstancesDirty = false; m_bVisibleInstancesUsedCamera = hasCamera;
                m_iVisibleCameraRevision = cameraRevision; m_iVisibilitySettingsRevision = settings.Revision;
                m_VisibleViewportSize = viewport; m_bVisibleDistanceEnabled = distanceEnabled;
                m_DistanceRejectedInstances = 0u;
                m_bFinalCameraPrepared = true; m_iPreparedFrame = frame;
                Invalidate_VisibilityCpu();
                if (auto* profiler = game.Get_Profiler())
                {
                    profiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibilityRebuilds);
                    profiler->Add_Counter(Engine::EProfilerCounter::MapBatchBoundsRejected);
                }
                return HRESULT(2); // Successful new empty commit, not a cache hit.
            }
        }
    }
    try
    {
        if (!m_CpuVisibility) m_CpuVisibility = std::make_unique<CPU_VISIBILITY_PREPARATION>();
        auto& prepared = *m_CpuVisibility;
        prepared.State = CPU_VISIBILITY_PREPARATION::STATE::EMPTY;
        // No allocation, global capture, resource access or bounds rebuild remains in Execute.
        m_CandidateVisibleInstances.reserve(m_Instances.size());
        m_CandidateDistanceChanges.reserve(m_Instances.size());
        if (m_bTrackProxySources) m_CandidateVisibleInstanceIndices.reserve(m_Instances.size());
        prepared.FrustumStates.resize(m_Instances.size());
        prepared.BatchFrustum = batchFrustum; prepared.RequiresNextTick = requiresNextTick;
        prepared.Frame = frame; prepared.HasCamera = hasCamera;
        prepared.Camera = cameraSnapshot; prepared.CameraRevision = cameraRevision;
        prepared.Settings = settings; prepared.Viewport = viewport;
        prepared.CameraPosition = position ? *position : float4_t{};
        prepared.DistanceEnabled = distanceEnabled;
        prepared.CountersCommitted = false; prepared.Result = S_OK;
        prepared.State = CPU_VISIBILITY_PREPARATION::STATE::STAGED;
        return S_OK;
    }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
    catch (...) { return E_FAIL; }
}

bool_t CMapStaticBatchObject::Try_PrepareFinalCameraCpuJob(FINAL_CAMERA_CPU_JOB& output)
{
    output = {};
    auto& game = CGameInstance::Get();
    if (!m_FrameState || !game.Get_MapVisibilitySettings().ParallelPreparationEnabled ||
        game.Is_SceneEnvironmentReplaced() || m_iAuthoredVisibleInstanceCount == 0u ||
        m_FrustumCulling.diagnostics || m_FrustumCulling.bypass || Are_AllChunkMeshesClaimed())
        return false;
    if (Stage_VisibilityCpu(CMapAssetRenderUtils::Capture_CameraCullSnapshotView()) != S_OK ||
        !m_CpuVisibility || m_CpuVisibility->State != CPU_VISIBILITY_PREPARATION::STATE::STAGED)
        return false;
    output.Context = this; output.Execute = &Execute_VisibilityCpu;
    output.Cost = static_cast<uint32_t>((std::min)(m_Instances.size(), size_t(UINT32_MAX)));
    return output.Cost != 0u;
}

void CMapStaticBatchObject::Execute_VisibilityCpu(void* context) noexcept
{
    auto& batch = *static_cast<CMapStaticBatchObject*>(context);
    auto& prepared = *batch.m_CpuVisibility;
    if (prepared.State != CPU_VISIBILITY_PREPARATION::STATE::STAGED) return;
    try
    {
        batch.Compute_VisibilityCpu();
        prepared.Camera = nullptr;
        prepared.State = CPU_VISIBILITY_PREPARATION::STATE::READY;
    }
    catch (const std::bad_alloc&)
    {
        prepared.Camera = nullptr;
        prepared.Result = E_OUTOFMEMORY; prepared.State = CPU_VISIBILITY_PREPARATION::STATE::FAILED;
    }
    catch (...)
    {
        prepared.Camera = nullptr;
        prepared.Result = E_FAIL; prepared.State = CPU_VISIBILITY_PREPARATION::STATE::FAILED;
    }
}

void CMapStaticBatchObject::Compute_VisibilityCpu()
{
    auto& prepared = *m_CpuVisibility;
    const bool hasCameraSnapshot = prepared.HasCamera;
    const auto* cameraSnapshot = prepared.Camera;
    const auto& visibilitySettings = prepared.Settings;
    const auto& distanceViewport = prepared.Viewport;
    for (size_t i = 0u; i < m_Instances.size(); ++i)
        prepared.FrustumStates[i] = m_Instances[i].FrustumState;
	m_CandidateVisibleInstances.clear();
    m_CandidateVisibleInstanceIndices.clear();
    m_CandidateDistanceChanges.clear();
    bool candidateOcclusionBounds = m_bOcclusionGeometryEligible;
    float3_t candidateOcclusionMin{ FLT_MAX, FLT_MAX, FLT_MAX }, candidateOcclusionMax{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
	bool_t requiresNextCameraTick = prepared.RequiresNextTick;
    INSTANCE_ENVELOPE visibleEnvelope;
    VIEW_LOD_ENVELOPE visibleViewEnvelope(m_bHasStaticMeshLod ? cameraSnapshot : nullptr);
    uint64_t cullingCandidates = 0u, distanceTested = 0u, distanceRejected = 0u;
    const float4_t* distanceCamera = &prepared.CameraPosition;
    const bool distanceEnabled = prepared.DistanceEnabled;
	for (size_t index = 0u; index < m_Instances.size(); ++index)
	{
        const FMapStaticInstance& instance = m_Instances[index];
		if (!instance.Visible || instance.Suppressed ||
			instance.CameraPreviewSuppressed)
			continue;

        ++cullingCandidates;
		MAP_FRUSTUM_CULL_DECISION decision{};
		const bool_t evaluated = hasCameraSnapshot &&
			CMapAssetRenderUtils::Evaluate_FrustumVisibility(
				m_FrustumCulling,
				*cameraSnapshot,
				m_AssetId,
				m_AssetGroupId,
				instance.PlacementId,
				instance.WorldBoundsCenter,
				instance.WorldBoundsRadius,
				prepared.FrustumStates[index],
				decision, nullptr, MAP_FRUSTUM_CULL_DETAIL::VISIBILITY_ONLY);
		if (evaluated && !decision.wouldBeVisible &&
			decision.shouldRender && !m_FrustumCulling.bypass)
		{
			requiresNextCameraTick = true;
		}
		if (evaluated && !decision.shouldRender)
		{
			continue;
		}

        bool distanceHidden = false;
        if (distanceEnabled)
        {
            const float limit = StaticPropDistanceLimit(instance.WorldBoundsRadius, visibilitySettings.DistanceScale);
            if (limit > 0.f)
            {
                ++distanceTested;
                float margin = (std::max)(0.f, m_FrustumCulling.baseMargin);
                if (instance.WorldBoundsRadius >= m_FrustumCulling.largeObjectRadiusThreshold)
                    margin += (std::max)(m_FrustumCulling.largeObjectAbsoluteMargin,
                        instance.WorldBoundsRadius * m_FrustumCulling.largeObjectRelativeMargin);
                // 배율은 거리와 화면 크기를 함께 바꾼다. 반경 3m 초과와 큰 화면 투영은 계속 보존한다.
                distanceHidden = StaticPropDistanceHidden(instance, *distanceCamera, *cameraSnapshot, distanceViewport,
                    limit, visibilitySettings.Get_DistancePixelLimit(), visibilitySettings.Revision, margin);
            }
        }
        if (distanceHidden != instance.DistanceHidden || (distanceHidden && instance.DistanceSettingsRevision != visibilitySettings.Revision))
            m_CandidateDistanceChanges.emplace_back(static_cast<uint32_t>(index), distanceHidden);
        if (distanceHidden) { ++distanceRejected; continue; }
        if (candidateOcclusionBounds)
        {
            candidateOcclusionBounds = instance.OcclusionBoundsValid;
            if (candidateOcclusionBounds)
            {
                XMStoreFloat3(&candidateOcclusionMin, XMVectorMin(XMLoadFloat3(&candidateOcclusionMin), XMLoadFloat3(&instance.OcclusionBoundsMin)));
                XMStoreFloat3(&candidateOcclusionMax, XMVectorMax(XMLoadFloat3(&candidateOcclusionMax), XMLoadFloat3(&instance.OcclusionBoundsMax)));
            }
        }

        visibleEnvelope.Add(instance, m_InstanceLinearScaleBounds[index]);
        if (m_bHasStaticMeshLod) visibleViewEnvelope.Add(instance);
		VTXMESHINSTANCE gpuInstance{};
		gpuInstance.World =
			instance.World;
		gpuInstance.WorldInvTranspose =
			instance.WorldInvTranspose;
        gpuInstance.vLightmapScaleBias = instance.BakedLighting.scaleBias;
        gpuInstance.vLightmapAverageScale = instance.BakedLighting.averageScale;
        gpuInstance.vLightmapDirectionalScale = instance.BakedLighting.directionalScale;
        gpuInstance.vStaticShadowScaleBias = instance.BakedLighting.shadowScaleBias;
        gpuInstance.vSourceWindOwnerPosition = instance.SourceWind.actorPositionSourceCm;
        gpuInstance.vSourceWindDimensionsAndRadius = instance.SourceWind.objectDimensionsAndRadiusSourceCm;

		m_CandidateVisibleInstances.push_back(
			gpuInstance);
        if (m_bTrackProxySources)
            m_CandidateVisibleInstanceIndices.push_back(static_cast<uint32_t>(index));
	}

    prepared.RequiresNextTick = requiresNextCameraTick;
    prepared.RejectedBatch = false;
    prepared.HasOcclusionBounds = candidateOcclusionBounds && !m_CandidateVisibleInstances.empty();
    prepared.OcclusionMin = candidateOcclusionMin; prepared.OcclusionMax = candidateOcclusionMax;
    prepared.LodBounds = {}; prepared.TightLodBounds = {};
    const bool hasLodBounds = visibleEnvelope.Store(prepared.LodBounds);
    if (!hasLodBounds) prepared.LodBounds = {};
    if (!hasLodBounds || !visibleViewEnvelope.Store(prepared.TightLodBounds)) prepared.TightLodBounds = {};
    prepared.LodScale = hasLodBounds ? visibleEnvelope.maximumScale : 0.f;
    prepared.CullingCandidates = cullingCandidates;
    prepared.DistanceTested = distanceTested; prepared.DistanceRejected = distanceRejected;
}

HRESULT CMapStaticBatchObject::Upload_VisibleInstances(const MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot)
{
    auto* profiler = CGameInstance::Get().Get_Profiler();
    Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchVisibility);
    const HRESULT staged = Stage_VisibilityCpu(cameraSnapshot);
    if (FAILED(staged)) return staged;
    if (staged == HRESULT(2)) return S_OK;
    if (staged == S_FALSE) return S_OK;
    // A Layer allocation/dispatch failure can leave a valid staged job unexecuted.
    if (m_CpuVisibility->State == CPU_VISIBILITY_PREPARATION::STATE::STAGED)
    {
        Engine::CProfilerDetailScope scope(profiler, "Map.Batch.CullAndPack");
        Execute_VisibilityCpu(this);
    }
    auto& prepared = *m_CpuVisibility;
    if (prepared.State != CPU_VISIBILITY_PREPARATION::STATE::READY) return prepared.Result;
    if (!prepared.CountersCommitted)
    {
        prepared.CountersCommitted = true;
        if (profiler)
        {
            profiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibilityRebuilds);
            profiler->Add_Counter(Engine::EProfilerCounter::MapBatchBoundsRejected, prepared.RejectedBatch ? 1u : 0u);
            profiler->Add_Counter(Engine::EProfilerCounter::MapDistanceTestedInstances, prepared.DistanceTested);
            profiler->Add_Counter(Engine::EProfilerCounter::MapCullingCandidates, prepared.CullingCandidates);
            profiler->Add_Counter(Engine::EProfilerCounter::MapCullingVisible, m_CandidateVisibleInstances.size());
        }
    }
    const bool payloadUnchanged = m_CandidateVisibleInstances.size() == m_VisibleInstances.size() &&
        (m_CandidateVisibleInstances.empty() || 0 == std::memcmp(m_CandidateVisibleInstances.data(),
            m_VisibleInstances.data(), m_CandidateVisibleInstances.size() * sizeof(VTXMESHINSTANCE)));
    if (!payloadUnchanged && !m_CandidateVisibleInstances.empty())
    {
        Engine::CProfilerDetailScope scope(profiler, "Map.Batch.InstanceUpload");
        if (FAILED(Ensure_InstanceCapacity(static_cast<uint32_t>(m_CandidateVisibleInstances.size())))) return E_FAIL;
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (FAILED(m_pContext->Map(m_pInstanceBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mapped))) return E_FAIL;
        std::memcpy(mapped.pData, m_CandidateVisibleInstances.data(), m_CandidateVisibleInstances.size() * sizeof(VTXMESHINSTANCE));
        m_pContext->Unmap(m_pInstanceBuffer.Get(), 0u);
        if (profiler) profiler->Add_Counter(Engine::EProfilerCounter::MapBatchUploadBytes,
            m_CandidateVisibleInstances.size() * sizeof(VTXMESHINSTANCE));
    }
    // Payload, source identity, bounds and both kinds of hysteresis commit together.
    m_VisibleLodBounds = prepared.LodBounds; m_VisibleTightLodBounds = prepared.TightLodBounds;
    m_fVisibleLodScale = prepared.LodScale;
    m_bVisibleOcclusionBounds = prepared.HasOcclusionBounds;
    m_VisibleOcclusionMin = prepared.OcclusionMin; m_VisibleOcclusionMax = prepared.OcclusionMax;
    Commit_DistanceSelection(prepared.Settings.Revision);
    m_BatchFrustumState = prepared.BatchFrustum;
    for (size_t i = 0u; i < m_Instances.size(); ++i) m_Instances[i].FrustumState = prepared.FrustumStates[i];
    if (!payloadUnchanged) m_VisibleInstances.swap(m_CandidateVisibleInstances);
    m_VisibleInstanceIndices.swap(m_CandidateVisibleInstanceIndices);
    m_bVisibleInstancesDirty = prepared.RequiresNextTick;
    m_bVisibleInstancesUsedCamera = prepared.HasCamera;
    m_iVisibleCameraRevision = prepared.CameraRevision;
    m_iVisibilitySettingsRevision = prepared.Settings.Revision;
    m_VisibleViewportSize = prepared.Viewport;
    m_bVisibleDistanceEnabled = prepared.DistanceEnabled;
    m_DistanceRejectedInstances = prepared.DistanceRejected;
    prepared.State = CPU_VISIBILITY_PREPARATION::STATE::EMPTY;
    if (profiler)
    {
        profiler->Add_Counter(Engine::EProfilerCounter::MapDistanceRejectedInstances, m_DistanceRejectedInstances);
        profiler->Add_Counter(Engine::EProfilerCounter::MapDistanceRejectedIndices, m_DistanceRejectedInstances * m_DistanceSourceIndices);
        profiler->Add_Counter(Engine::EProfilerCounter::MapVisibleInstances, m_VisibleInstances.size());
    }
    return S_OK;
}

HRESULT CMapStaticBatchObject::Upload_ShadowInstances()
{
	Engine::CProfilerDetailScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.ShadowPrepare");
	MAP_SHADOW_CULL_SNAPSHOT lightSnapshot{};
	const bool_t hasLightSnapshot =
		CMapAssetRenderUtils::Capture_ShadowCullSnapshot(lightSnapshot);
	const uint64_t lightRevision = hasLightSnapshot ? lightSnapshot.revision : 0u;
	if (!m_bShadowInstancesDirty &&
		m_bShadowInstancesUsedLight == hasLightSnapshot &&
		m_iShadowLightRevision == lightRevision)
		return S_OK;

	m_CandidateShadowInstances.clear();
	for (const FMapStaticInstance& instance : m_Instances)
	{
		if (!instance.Visible || instance.Suppressed ||
			instance.CameraPreviewSuppressed ||
			(hasLightSnapshot && !CMapAssetRenderUtils::Intersects_ShadowCullSnapshot(
				lightSnapshot, instance.WorldBoundsCenter, instance.WorldBoundsRadius)))
			continue;

		VTXMESHINSTANCE gpuInstance{};
		gpuInstance.World = instance.World;
		gpuInstance.WorldInvTranspose = instance.WorldInvTranspose;
		gpuInstance.vLightmapScaleBias = instance.BakedLighting.scaleBias;
		gpuInstance.vLightmapAverageScale = instance.BakedLighting.averageScale;
		gpuInstance.vLightmapDirectionalScale = instance.BakedLighting.directionalScale;
		gpuInstance.vStaticShadowScaleBias = instance.BakedLighting.shadowScaleBias;
		gpuInstance.vSourceWindOwnerPosition = instance.SourceWind.actorPositionSourceCm;
		gpuInstance.vSourceWindDimensionsAndRadius = instance.SourceWind.objectDimensionsAndRadiusSourceCm;
		m_CandidateShadowInstances.push_back(gpuInstance);
	}

	const bool_t payloadUnchanged =
		m_CandidateShadowInstances.size() == m_ShadowInstances.size() &&
		(m_CandidateShadowInstances.empty() || 0 == std::memcmp(
			m_CandidateShadowInstances.data(), m_ShadowInstances.data(),
			m_CandidateShadowInstances.size() * sizeof(VTXMESHINSTANCE)));
	if (!payloadUnchanged && !m_CandidateShadowInstances.empty())
	{
		Engine::CProfilerDetailScope uploadScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.ShadowUpload");
		HRESULT result = Ensure_ShadowInstanceCapacity(
			static_cast<uint32_t>(m_CandidateShadowInstances.size()));
		if (FAILED(result)) return result;

		D3D11_MAPPED_SUBRESOURCE mapped{};
		result = m_pContext->Map(m_pShadowInstanceBuffer.Get(), 0,
			D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		if (FAILED(result)) return result;

		std::memcpy(mapped.pData, m_CandidateShadowInstances.data(),
			m_CandidateShadowInstances.size() * sizeof(VTXMESHINSTANCE));
		m_pContext->Unmap(m_pShadowInstanceBuffer.Get(), 0);
	}
	// A failed Map leaves both the successful payload and light revision intact.
	// The next call retries even when only the light, rather than instances, moved.
	if (!payloadUnchanged)
		m_ShadowInstances.swap(m_CandidateShadowInstances);
	m_bShadowInstancesDirty = false;
	m_bShadowInstancesUsedLight = hasLightSnapshot;
	m_iShadowLightRevision = lightRevision;
	return S_OK;
}

HRESULT CMapStaticBatchObject::
Rebuild_PlacementLookup()
{
	m_PlacementLookup.clear();
	m_iAuthoredVisibleInstanceCount = 0u;
	m_PlacementLookup.reserve(
		m_Instances.size());
    m_InstanceLinearScaleBounds.resize(m_Instances.size());

	for (uint32_t index = 0;
		index < m_Instances.size();
		++index)
	{
		FMapStaticInstance& instance =
			m_Instances[index];

		if (0 == instance.PlacementId ||
			instance.WorldBoundsRadius <= 0.f)
		{
			return E_INVALIDARG;
		}

		const auto [iter, inserted] =
			m_PlacementLookup.emplace(
				instance.PlacementId,
				index);

		UNREFERENCED_PARAMETER(iter);

		if (!inserted)
			return E_INVALIDARG;
        m_InstanceLinearScaleBounds[index] = LinearScaleBound(instance.World);
        Rebuild_InstanceOcclusionBounds(instance);
		if (instance.Visible)
			++m_iAuthoredVisibleInstanceCount;
	}

	return S_OK;
}

unique_ptr<CMapStaticBatchObject>
CMapStaticBatchObject::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance =
		unique_ptr<CMapStaticBatchObject>(
			new CMapStaticBatchObject(
				pDevice,
				pContext));

	if (FAILED(
		instance->Initialize_Prototype()))
	{
		return nullptr;
	}

	return instance;
}

shared_ptr<CPrototype>
CMapStaticBatchObject::Clone(void* pArg)
{
	auto instance =
		shared_ptr<CMapStaticBatchObject>(
			new CMapStaticBatchObject(
				*this));

	if (FAILED(instance->Initialize(pArg)))
		return nullptr;

	return instance;
}

void CMapStaticBatchObject::Rebuild_BatchCullBounds() const
{
    INSTANCE_ENVELOPE envelope;
    uint32_t grace = 0u;
    for (size_t index = 0u; index < m_Instances.size(); ++index)
    {
        const auto& instance = m_Instances[index];
        if (!instance.Visible) continue;
        envelope.Add(instance, m_InstanceLinearScaleBounds[index]);
        grace = (std::max)(grace, instance.FrustumState.rejectGraceFrames);
    }
    m_bHasBatchBounds = envelope.Store(m_BatchBounds);
    m_BatchFrustumState = {};
    m_BatchFrustumState.rejectGraceFrames = grace;
    m_bBatchBoundsDirty = false;
}

bool_t CMapStaticBatchObject::Build_ScreenLodView(const MAP_CAMERA_CULL_SNAPSHOT& camera,
    Engine::MESH_SCREEN_LOD_DESC& result) const
{
    const auto& p = camera.projection;
    // Other projection conventions retain the original direct draw.
    if (p._14 != 0.f || p._24 != 0.f || p._34 != 1.f || p._44 != 0.f ||
        p._12 != 0.f || p._21 != 0.f || p._13 != 0.f || p._23 != 0.f ||
        p._41 != 0.f || p._42 != 0.f || p._11 <= 0.f || p._22 <= 0.f ||
        p._33 <= 1.f || p._43 >= 0.f || m_fVisibleLodScale <= 0.f || m_VisibleLodBounds.w <= 0.f)
        return false;
    const auto& v = camera.lodView;
    const float viewScale = camera.lodViewScale;
    if (viewScale <= 0.f) return false;
    const auto viewport = CGameInstance::Get().Get_ViewportSize();
    if (viewport.x <= 0.f || viewport.y <= 0.f) return false;
    const vector_t center = XMVectorSet(m_VisibleLodBounds.x, m_VisibleLodBounds.y, m_VisibleLodBounds.z, 1.f);
    Engine::MESH_SCREEN_LOD_DESC candidate{};
    // Bern의 입장 컷신과 일반 이동에 1pixel 예산을 준다. 선택기는 여기에
    // 10% 여유를 적용하며 다른 맵은 기존0.25pixel 한계로 생성된 단계들을 선택한다.
    if (m_FrameState) candidate.maximumPixelError = 1.f;
    XMStoreFloat4(&candidate.viewBounds, XMVector3TransformCoord(center, XMLoadFloat4x4(&v)));
    candidate.viewBounds.w = m_VisibleLodBounds.w * viewScale;
    candidate.projectionPixels = { p._11 * viewport.x * .5f, p._22 * viewport.y * .5f };
    candidate.maximumScale = m_fVisibleLodScale * viewScale;
    candidate.nearPlane = -p._43 / p._33;
    if (m_VisibleTightLodBounds.w == 1.f && m_bVisibleInstancesUsedCamera &&
        m_iVisibleCameraRevision == camera.revision)
    {
        candidate.maximumAbsViewXY = { m_VisibleTightLodBounds.x, m_VisibleTightLodBounds.y };
        candidate.minimumViewDepth = m_VisibleTightLodBounds.z;
        candidate.hasTightViewBounds = true;
    }
    const float values[] = { candidate.viewBounds.x, candidate.viewBounds.y, candidate.viewBounds.z,
        candidate.viewBounds.w, candidate.projectionPixels.x, candidate.projectionPixels.y,
        candidate.maximumScale, candidate.nearPlane };
    for (const auto value : values) if (!std::isfinite(value)) return false;
    if (candidate.nearPlane <= 0.f) return false;
    result = candidate;
    return true;
}
```
