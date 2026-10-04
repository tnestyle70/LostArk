# 베른 컷신·기본 이동의 제출 비용 최적화 구현 계획

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
