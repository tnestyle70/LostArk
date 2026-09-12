# Effect 셰이더 분리와 Play All 성능 개선 구현 계획

기준일: 2026-09-09. 기존 브랜치 `codex/dimensionmaster-tool-round3`의 공유 변경을 보존한다.
사용자는 진행 중인 복원 빌드 완료 후 셰이더 분리와 Play All 병목 개선까지 구현하도록 승인했다.
이 문서는 구현 범위와 순서를 소유하며, 실제 완료와 측정 결과는 같은 주제의 RESULT에 기록한다.
원본 복원 기준은 Round2 RESULT G36이다. 카메라 연출·맵 복원 범위를 새로 확장하지 않는다.

## G00. 복원 빌드와 변경 전 기준 보존

현재 Debug Product 빌드를 끝까지 완료하고 receipt, EXE와 Mesh/Particle CSO, 검토 입력 SHA를
보존한다. 진행 중인 제품 소스는 변경하지 않는다. 이후 동일 설정에서 분리 빌드 시간과 변경 없는
증분 빌드, 한 재질군 변경의 재컴파일 범위를 측정한다. 첫 빌드와 증분 빌드는 별도로 보고한다.

현 구조는 두 FX 입력 각각 약6.3MB, native 함수639개 정의를 포함한다. 보수적 텍스트 호출
추적은 Mesh336개·Particle504개에 도달한다. 이전 성공 Product는50분14.853초였고 현재
빌드는 아직 완료 전이다. Debug `/Od`는 X4505 임시 register4096 한도로 실패한 이력이 있어
수식을 유지한 채 현재 `/O1`을 사용한다. 단순 최적화 해제로 시간을 줄이지 않는다.

GPU 실행시간·FPS와 정적 instruction 수는 구분한다. FXC의 낮은 CPU 사용률 원인은 완전히
확정되지 않았으므로 같은 소스를 받은 팀원에게 정확히 같은 시간이 걸린다고 가정하지 않는다.

## G01. 실제 native 재질군별 FX 컴파일과 준비 경로

| 대상 | 변경 목적 |
|---|---|
| `Client/Public/Effect_ShaderFamily.h` 추가 | 실제 실행할 carrier·family·구간·고유 셰이더 basename의 단일 constexpr 표 |
| `Client/Public/Effect_DocumentRenderer.h`, `Client/Private/Effect_DocumentRenderer.cpp` | 기존 준비 리소스와 device별 공유 Core에 family 선택과 shader 배열 연결 |
| `Client/Bin/ShaderFiles/Shader_VtxEffectMeshPreview.hlsl`, `Shader_VtxEffectParticle.hlsl` | generic 진입점 유지, native 코드를 별도 FX로 분리 |
| 같은 ShaderFiles의 native `.hlsli`와 family별 새 `.hlsl` | 함수 본문은 한 물리 파일만 소유하고 필요한 묶음만 컴파일 |
| `Client/Default/Client.vcxproj`, `.vcxproj.filters` | 새 헤더·include·FX 입력의 실제 등록 |
| `Tools/Build/Test-CompiledShaderClosure.ps1`, `Tools/RenderingPipeline/ProductEffectShaderWarpProbe.cpp` | 실제 family 표를 소비하는 출력·입력·pass 검증 연결 |

실행 단위는 generic2개와 native34개, 총 Mesh/Particle FX36개다.

| family | 내부 구간 | Mesh/Particle 합계 |
|---|---|---:|
| Generic | 기존 두 basename |2|
| DimensionMaster Q, V, WR, SD, Warlord | 각각 기존 검증된 family 전체 |10|
| DimensionMaster ALT_V |64–127,128–191,192–255|6|
| Artist |448–511,512–575,768–831,832–895|8|
| LanceMaster |512–575,576–639,640–703,704–767,768–831|10|

기존 `Find_*`와 `Has_*MaterialContract`로 승인한 family 안에서만 구간을 선택한다.
숫자 범위만으로 알 수 없는 profile을 허용하지 않는다. enum과 descriptor는 실행 준비 상태이며
저장 asset ID, source profile ID, 문서 스키마를 변경하지 않는다. 정확한36개 basename 설계 목록은
`out/EffectBuildPlayAllOptimization20260909/Design/native_shader_36_units.json`에 있고 구현 후
실제 C++ 표가 유일한 실행 정본이 된다. 별도 영구 manifest를 만들지 않는다.

`Stage_ElementResource`는 기존 effective profile 검증 뒤 carrier/family/구간을 확정한다.
mesh section의 재귀 준비 리소스도 각각 선택을 소유한다. 실패는 기존 prepare transaction 전에
이유를 반환하고 이전 문서·리소스를 보존한다. `Build_RendererCore`는 기존 device별 cold-build
잠금과 성공/실패 캐시 아래 필요한36개 FX를 준비한다. `Initialize`는 같은 Core의 참조만 보관한다.
`Render_Mesh`와 `Render_Particles`는 준비된 local shader를 선택해 모든 bind와 Begin에 사용한다.
draw 중 디스크 로드·shader 생성·전역 shader pointer 교체를 추가하지 않는다.

분리된 FX 입력은 설치된 MSBuild의 MultiProcFXC 경로로 병렬 컴파일한다. 기존 사용자 설정을
보존하고 동시에4개를 시작 기준으로 제한하며 실제 실행·tracking 결과를 검증한다. C++/MIDL의
UseMultiToolTask를 일괄 활성화하거나 별도 빌드 시스템을 만들지 않는다. 격리된 작은 FX 프로젝트로
동시 실행과 무변경 증분 동작을 먼저 확인한 뒤 제품 프로젝트에 연결한다.
기본값은 Microsoft.Cpp props가 false/0을 채우기 전 Globals에서 설정하고 명시된 override는 보존한다.

`Bind_Common`과 `Bind_MaterialInputs`의 기존 두 shader 포인터 비교를 명시적 carrier/family
계약으로 교체한다. 선택된 family의 native packet만 전달한다. WR/SD는 현재 V packet을 그대로
사용한다. Warlord ambient 조회는 Warlord에만 수행한다.

첫 분리본 검증 뒤 확인한 native와 범용 backend의 배타성을 사용해 native FX의 큰 공통 PS
front-end와 불필요한 typed/reconstructed packet binding까지 제거한다.224개 Authored 문서의
native material1619행에 Execution.enabled=true가 없고, 실제 Has_* admission과 후반
reconstructed/visual adapter가 mixed backend를 거부하는 현재 계약을 근거로 한다.
원본639개 native 함수, VS 입력·Warlord WPO·color/UV/SourceSRV/sampler·scene/depth·pass를
유지한다. Generic FX에는 StandardColorV1, RuntimeMaterialV2, reconstructed/ArtistVisualV4의
기존 선행 분기와 등록 경로를 보존한다. Native에4개 범용 backend flag가 함께 켜진 모순 상태는
준비/선택 경계에서 실패하며 generic fallback으로 숨기지 않는다.

이 추가 단계 전에 첫36개 분리 CSO를 보존하고, 한 Q native FX의 실제 컴파일과 정적 지표로
효과를 확인한다. 이후 전체 native FX와 required binding 검증을 다시 수행하며, 원래 대형 FX와
최종 Q51/V63/WR208/260 픽셀 출력을 직접 대조한다. 중간 분리본 PASS를 최종 입력에 재사용하지 않는다.

큰 세 family의 함수 본문은64 구간별 물리 include로 옮긴다. 기존 giant include에 조건문만
덧붙여 모든 구간이 같은 파일 변경에 다시 빌드되는 구조를 남기지 않는다. V/WR/SD가 공유하는
uniform·sampling helper는 작은 공통 include가 소유하고 V 본문 변경은 WR의 의존성이 되지 않게 한다.
원본 Warlord WPO, Q51 tangent/literal, WR260 CPU uniform, UV와 sampling 식은 그대로 유지한다.

Mesh7개·Particle5개의 pass 이름·번호·blend/depth/rasterizer·MRT 계약을 유지한다.
VS 입력과 pass 코드는 carrier별 한 소스에서 공유한다. FX5가 각 CSO의 stage를 적용하므로
공통 VS 소스와 GPU VS 객체의 공유를 혼동하지 않는다. 새로운 두 번째 Engine shader runtime은 없다.

## G02. CShader의 반복 이름·pass 조회 제거

수정 파일은 `Engine/Public/Shader.h`, `Engine/Private/Shader.cpp`다. 기존 `CShader`가 소유한
Effect 수명 안에서 valid variable과 pass의 빌려 쓰는 포인터를 캐시한다. 문자열 key는 캐시가 소유하고
준비 단계에서 채운 불변 해시 슬롯으로 매 호출의 임시 할당·Debug iterator 비용을 피한다.
원래 Effect의 이름 조회 우선순위를 준비 시 확인하며 clone은 Effect와 같은 캐시를 공유한다.
실제 caller는 현재 모든 `Bind_*`와 `Begin`이다.

`Initialize_Prototype`는 Effect/pass/layout을 먼저 검증하고 함께 commit한다. 새 Effect commit 시
이전 variable cache를 지운다. `Clone`이 공유하는 Effect의 수명이 포인터 수명보다 길어야 한다.
uniform 값 자체는 캐시하지 않아 같은 Effect를 공유하는 clone 간 갱신이 누락되지 않게 한다.
없거나 잘못된 변수·pass, 캐시 할당 실패는 기존 HRESULT 실패 경계로 반환한다.

새 C++ 파일은 필요 없다. Engine public header 변경은 Product 빌드로 SDK 배포와 Client 재컴파일까지
검증한다. 실제 Effect를 사용하는 headless 검사로 반복 bind, 실패, 재초기화와 clone 수명을 확인한다.

## G03. Playback의 요소 불변 조회 재사용

수정 시작점은 `Client/Private/Effect_Playback.cpp`의 `Rebuild_Frame`이다. 같은 클래스의
`Client/Public/Effect_Playback.h`에는 준비된 update module의 private 선언과 실제 호출 인자를 연결한다.
`Resolve_SourceSpritePresentation`, mesh-particle 판정과 type pre-rotation을 입자별 조회에서
요소별 조회로 옮긴다. 시간·RNG·분포 계산은 기존 순서에 남긴다. local-space 입자에 공유되는
역행렬만 요소당 한 번 계산하고, birth root가 서로 다른 world-space 입자는 개별 계산을 유지한다.
특이 행렬과 비유한 값의 기존 결과를 숨기는 근사·영벡터 생략은 하지 않는다.

실제 source Playback으로 R/S와 큰 ALT_V 문서들의 Update, Seek, Rebuild 비용을 나누어 측정한다.
Sequencer가 사용하는 Update_WithTransformHistory도 같은 비교에 포함한다. 기존 Profiler에
HistoryUpdate와 FrameRebuild 범위를 연결해 일반 Update scope만으로 Play All 비용을 누락하지 않는다.
고정 step이 남은 CPU 비용의 주된 부분이면 immutable update module 분류 준비를 같은
`PREPARED_RESOURCES` 소유권에 제한해 추가한다. value-only 문서 편집도 허용하는 기존 계약에 맞춰
실제 staged document의 class·enabled·원본 index를 stage마다 준비하며, 기존 vector field 리소스는
공유한다. literal/distribution 계산과 모든 모듈의 실행 순서·RNG는 유지한다. 이미 존재하는 frame
vector 재사용이나 순방향 incremental Update를 중복 구현하지 않는다.

5개 고정 문서와 동일 seed·clock·root로 변경 전후 전체 frame, emitter/module RNG, 실패 결과를
비교한다. pause/resume, 역방향 Seek, loop, fractional time, event rollback을 포함한다.
headless CPU 시간은 Client의 실제 FPS라고 보고하지 않는다.

## G04. 입자 전송과 SceneColor 필요 판정 재사용

`Engine/Public/VIBuffer_ParticleRect.h`, `Engine/Private/VIBuffer_ParticleRect.cpp`의 기존 동적
instance buffer가 append cursor와 현재 slice byte offset을 소유한다. 첫 upload 또는 tail 부족 때
`WRITE_DISCARD`, 남는 공간에는 `WRITE_NO_OVERWRITE`를 사용한다. 기존2048 capacity와136-byte
payload를 유지하고 `Bind_Resources`의 instance VB offset만 선택한다. `DrawIndexedInstanced`의
startInstance0, 데이터·순서·불변 quad/index는 유지한다.

offset/cursor는 성공한 Map/Unmap 뒤 commit한다. empty/Map 실패는 기존처럼 draw count0이고,
oversize는 E_INVALIDARG를 반환하면서 이전 활성 slice/count를 보존한다. 사용 중인 GPU 영역을
프레임 경계에서 임의로 덮지 않는다. Initialize 성공 시
cursor를 초기화하고 Clone은 기존 독립 buffer 생성 경로를 유지한다.

현재 이 Render는 CVIBuffer::Render를 거치지 않아 기존 profiler의 draw 집계에서 빠진다.
실제 DrawIndexedInstanced 제출에 DrawCalls1, InstancedDrawCalls1, Instances와 Indices×Instances를 기존 counter로 추가해
Play All 집계의 누락을 바로잡는다. 새로운 계측 체계나 UI를 만들지 않는다.

`Client/Private/Effect_Object.cpp`의 `Late_Update`는 매 frame 호출 안에서 같은 요소의 native
SceneColor 요구를 한 번 계산한다. visibility·preview 선택·입자 alpha 조건은 입자마다 유지한다.
문서 편집·다른 object·다음 frame으로 판정 캐시를 넘기지 않는다.

mesh particle의 재질 binding 전체를 루프 밖으로 옮기면 수명 기반 색·UV·dissolve가 달라질 수 있다.
우선 G01의 선택 packet binding을 적용하고, 추가 hoist는 실제 입자별 의존성을 분리한 경우만 적용한다.
투명 draw 순서를 shader별로 다시 정렬하거나 원본 입자·mesh·effect를 줄이는 방법은 사용하지 않는다.

## G05. 검증과 실행 인계

1. 진행 중 복원 Product 성공 후 원본 소스와 EXE/CSO를 out 기준점에 보존한다.
2. G01 shader/renderer, G02 Engine Shader, G03 Playback, G04 VB/Object를 파일 소유권으로 병렬 구현한다.
3. 물리 이동한 모든 native 함수 본문의 동일성을 확인하고36 FX와 aggregator를 공유하는
   Shader_VtxEffectNativeScreenPost, Shader_VtxAnimMeshBinary를 포함한 최소38 FX를 컴파일한다.
   실제 include closure에 추가 영향이 있으면 함께 검증한다. 전체Client57개 producer/output 존재와
   실제 descriptor의 일대일 연결도 확인한다. Runtime ZIP의 기존 Bin 전체 포함 경로를 유지한다.
4. 기존 generic/glass WARP 검사를 유지하고 family 경로·pass·필수 변수와 실제 Q51/V63/WR208/260
   출력 동등성, Warlord WPO 및 Artist/Lance 대표 입력을 검증한다.
5. 실제 Playback의 출력·RNG 동등성과 반복 비용, buffer slice/wrap/실패·clone 계약을 검증한다.
6. 실제 FxCompile tracked input/output으로 한 물리 native 구간 변경의 제한된 재빌드와 변경 없는0 FXC를
   확인한다. 소유권을 설명하는 최종 주석 변경을 사용하고, 검증 뒤 소스를 되돌려 추가 재컴파일을
   유발하지 않는다. 주석 전후 native 함수 본문 SHA는 같아야 한다.
7. Debug Product, 변경 XML/JSON parse, `git diff --check`와 최종 배포를 확인한다. 과거 성공 receipt를
   새 구조의 성공 증거로 쓰지 않는다. Runtime 배포는 기존 빌드 경로와 EXE 옆 CSO를 사용한다.
8. 사용자는 Server + Client profile의 Ctrl+F5 → Lobby → Character Select → 차원술사 → F1 Effect Tool에서
   R/A/S와 큰 ALT_V 문서의 Play All을 확인한다. Client/UI 조작·화면 캡처·시각 또는 FPS PASS는
   에이전트가 대신 수행하지 않는다. 소스·자동 검사·사용자 실제 화면 결과를 RESULT에서 구분한다.

Resources의12개 원본 mip DDS는 이전 G36의 별도 Drive 전달 대상이며 이 구조 변경으로 복제하거나
Git에 추가하지 않는다. 공유 dirty worktree의 무관한 변경은 stage/commit하지 않는다.

## G06. Alt V318개 full 요소의 재생 중단 후속 조사와 구조 개선

2026-09-10 사용자가 F1에서 `effect.dimensionmaster.skill.2050540.full.restore` 전체 재생이
끊긴다고 확인했다. 원본318요소를 유지하며 command-time Load/Validate/Stage, forward HistoryUpdate,
CPU 제출·shader first draw와 반복 draw 비용을 구분한다. 이미 구현된 G01~G04의 캐시·instancing·
append buffer와 순방향 clock을 다시 구현하지 않는다.

현재 소스와 같은 문서를 out에 보존하고 기존 CPU·FX 수치 probe를 사용해 기준을 측정한다.
source module의 불변 property 조회, 요소별 반복 material 준비와 mesh particle 제출 증폭을
실제 호출에서 확인한다. 원본 시간·난수·입자 수·투명 순서를 유지하면서 준비 때 확정할 수 있는
정보와 매 입자 바뀌는 정보를 분리해, 측정된 반복 비용을 기존 runtime 안에서 줄인다.

GPU instancing은 현재 sprite span에 이미 적용되어 있다. mesh batching은 CModel의 geometry와
per-instance world/color/dynamic/life/SubUV 및 source material slot·winding·blend 순서를 모두
보존할 수 있는 범위만 검토한다. 단순 element 삭제, source count 축소, 무조건적인 shader 정렬,
프레임 시간 버리기 또는 별도 시뮬레이션으로 재생 중단을 감추지 않는다.

완료 검증은 동일 seed/clock/history의 frame·RNG 상태 대조, 필요한 producer compile과 실제
GPU 수치 결과, JSON/XML 및 diff 검사를 사용한다. headless 결과를 실제 Client FPS나 사용자
화면 PASS로 표현하지 않는다. 최종 RESULT에는 지금 적용한 개선과 추가로 필요한 구조를 구분한다.

G06 실측 후 적용 범위는 일반 F1 저작 문서 준비의 중복 canonical 직렬화다. `Create_AuthoringOccurrence
-> CEffectObject::Stage_Document -> Renderer::Prepare_DocumentResources`는 이미 Drawable 검증을
끝냈으며 일반 `Stage_PrevalidatedDocument`는 value edit을 허용해 canonical digest를 소비하지 않는다.
Playback 준비 함수의 기본값은 계속 digest 생성이고, 일반 검증 완료 호출자 둘만 생략을 명시한다.
immutable document가 있으면 이 명시와 무관하게 hash를 생성하고 visual/reconstructed 준비도 유지한다.
Load/Validate, resource signature, shader 입력, prepared identity와 실패 시 기존 객체 보존은 그대로다.
이 변경은 모든 입자 simulation을 바꾸지 않으므로 Alt V318의2,060 frame/state checkpoint를 대조한다.

## G07. Alt V 전체 요소의 비용과 원본 발생 수 재조사 — 2026-09-10

사용자의 추가 요청은 수백 요소를 유지하는 구조적 최적화의 전수 검토다. 현재318행을
renderer shape, native program, material/texture, geometry/submesh, 원본 burst/rate/loop와
runtime maxParticles까지 대조한다. 문서의 element 수, 실제 살아 있는 입자 수, 렌더 요청 수,
화면 픽셀 중첩 비용을 구분하고 원본 게임의 실제 품질·LOD·분기 실행을 추정으로 확정하지 않는다.

현재 제품 Playback의 out 사본에서962개60Hz step을 계산하고 입자별 native 입력과 프레임별
geometry 제출량을 집계한다. 단일 submesh의 연속된 동일 occurrence만 묶는 보수적안과
submesh 순서를 바꾸는 이론상 상한을 분리한다. 실제 GPU 제출을 하지 않은 계산값을 draw 실측이나
FPS 개선율로 기록하지 않는다. source burst가 runtime 용량으로 잘리는 경우에는 요청/수용/손실을
직접 기록하며 peak가 cap에 닿았다는 이유만으로 손실을 확정하지 않는다.

CPU는 준비 시간이 아니라 실제 HistoryUpdate, Step, 입자 spawn/update, distribution 조회,
Rebuild_Frame을 out 진단 범위에서 계측한다. 계측 오버헤드와 Debug 실행 조건을 명시한다.
이미 source event가 없는 문서에 상태 전체 복사 비용을 가정하거나 기존 sprite instancing,
SceneColor 프레임당1회 복사를 새로운 개선처럼 제안하지 않는다.

후속 구현은 기존 CModel/CMesh의 Render_Instanced와 CEffectPlayback 준비 구조를 확장한다.
instance stream에 world·normal/tangent·color·dynamic·life/time·SubUV를 전달하고 동일 geometry,
native family/program, texture/sampler, pass/capture와 원래 투명 순서를 보존한다. 불변 source
module/distribution/literal 조회는 준비 단계의 index/reference로 바꾸되 난수 호출과 curve 평가를
생략하지 않는다. shader별 전체 정렬이나 무조건적인 alpha0 생략은 허용 조건부터 검증한다.

이번 G07은 전수 조사와 구체적인 구현 순서의 정리다. 제품 renderer/particle clock을 바꾸는
후속 구현의 완료·FPS 개선·사용자 visual PASS는 실제 적용과 해당 검증 후 별도로 기록한다.

전수 계산에서 단일 submesh instancing만의 제출 감소는8.55%였고 두submesh 균열 모델의
반복 비용이 컸다. 현재 crack032/037의7개 occurrence는 같은 ALTV145 effective state를 사용한다.
CModel에 원래submesh0→1 index/attribute 순서의 준비 geometry를 두고 입자 전체를 반복하는
구조를 후속 우선안으로 선택한다. source slot0/1 범위는 보존하고 override/입력 변경 시 group을
재검증한다. 두 모델의 indexed attribute/논리 primitive 순서 대조는 완료했으며 구현 단계에는
instance native VS/PS의 실제 GPU 출력과 기존 Render_Mesh 경로의 parity가 필요하다.

## G08. 모든 full restore의 공통 재생 경로 구현 — 2026-09-10

사용자가 Alt V 외에도 수십 element에서 한 자리 FPS가 발생한다고 확인했고 구조 수정을 요청했다.
적용 범위를 네 클래스의 full 문서106개로 확장한다. Playback은 Stage 시 update module의 분포
index와 불변 literal을 준비하고 원래 난수·curve·시간 평가 순서를 유지한다. SceneColor 필요 재질의
검증은 문서 준비 때 수행하고 매 프레임에는 실제 가시성·Solo·입자 alpha를 계속 확인한다.

Renderer는 native static mesh의 같은 occurrence 안에서 연속된 동일 pass를 instance stream으로
제출한다. 원래 CModel/CMesh를 사용하며 static multi-submesh는 동일 effective material일 때
원래 primitive 순서로 준비한 기하를 사용한다. skinned, generic, material adapter, source slot
override는 기존 경로를 유지한다. 원본 vertex68-byte/index32 ABI와 source material slot을 보존한다.
새 public C++ 파일이나 두 번째 모델 runtime은 추가하지 않으므로 프로젝트 항목 추가도 없다.

native high-profile shader의 기존 외부 범위 누락과 source-character pass 이전 transient light
삭제는 별도 복원 결함으로 함께 수정한다. 이 두 수정은 표시 결과를 바꾸므로 성능 수정의
출력 동일성과 구분한다. 새 profile도 원래 native switch와 unknown-profile clip을 통과해야 한다.

검증은 수정 전후106문서의 Update/Seek frame·state·RNG checkpoint, 실제 CModel WARP 기하,
실제 production FX의 일반/instance VS·PS 수치 비교, Engine/Client/FX 최소 빌드로 수행한다.
소스 입자 cap·emitter clock·광원의 누락 source 초기값·Artist sibling provider는 이번 성능
변경에서 임의 수정하지 않는다. 사용자 실제 화면과 FPS는 사용자의 F1 Play 관찰로 확인한다.

## G09. 쿠크 대형 프로그램 분할과 전체 빌드 증분 유지 — 2026-09-12

사용자가 전체 빌드 시간 최적화를 요청했다. 현재 브랜치는
`codex/sequencer-camera-load-performance`이며 다른 기능의 미커밋 변경을 보존한다.
앞선 G01의 구간별 native-only 원칙을 쿠크 확장에도 적용한다. 복원한 수식, 원본 native ID,
carrier 입력, distortion companion, pass 순서·blend/depth/cull과 데이터 저장 계약은 유지한다.

### 현재 기준과 실제 호출 경계

09-12 Product는 35분23.966초이며 Client 단계는 30분43.670초였다.
후속 사용자 IDE 빌드는 ParticleKouku2304 하나가 13:49:50.693부터 14:16:57.975까지
27분7.282초 걸렸고 Client 링크·배포는 14:19에 끝났다. 같은 주요 입력의 이전 `/O1`
독립 검사는 디버그 정보 없이도 1543.812초였다. 따라서 `/Zi` 제거만으로 해결하지 않는다.
Group2304는 853개 기본 재질 프로그램과 companion을 한 파일에 누적하고 Particle switch에
617개 분기를 포함한다. 원본 shader 함수 본문이 바뀌지 않는 분할이 첫 변경 단위다.

`install_kouku_gate1_native_shaders.py`가 HLSLI·dispatch·wrapper를 만들고,
`Effect_ShaderFamily.h`의 기존 constexpr 표를 renderer가 소비한다. 현재 단일 2304~3711
구간을 기존 64 ID 구간 방식으로 나눈다. 실제 필요한 carrier만 등록하고 프로그램 누락·중복을
검사한다. 물리 include도 분리하여 한 leaf 수정이 다른 구간의 재컴파일 원인이 되지 않게 한다.
생성 결과와 생성기를 함께 바꾸며 반복 생성 시 내용이 같은 파일의 수정 시각을 보존한다.
새 wrapper/HLSLI는 `Client.vcxproj`와 `.filters`에 실제 기존 filter 아래 등록한다.
새 C++ runtime이나 별도 shader compiler는 만들지 않는다.

추가로 `Shader_VtxMeshBinary.hlsl`의20개 패스와 `Shader_VtxAnimMeshBinary.hlsl`의
기존 패스에서 같은 entry/profile/인자의 compile 호출을 공유된 VertexShader/PixelShader
변수로 옮긴다. 이미 공유된 EffectModelCue 방식과 같으며 함수 본문·패스 이름/번호·상태를
유지한다. 원래 compile 식으로 되돌려 정규화한 패스 전체가 같음을 확인한다.
진행 중인 FXC 입력은 수정하지 않고 out 후보를 준비한 뒤 해당 빌드 종료 후 적용한다.
전후 실제 FXC 시간·CSO 및 패스 선택을 비교하며 최적화 해제나 shader 계산 생략은 하지 않는다.

### 도구 선택과 실패 처리

현재 Product runner는 MSBuild 17.14를, 사용자 IDE는 VS18 Insiders를 사용했다.
후속 빌드에서 shader 80개와 OBJ 226개가 다시 생성됐지만 이전 build state가 덮였으므로
특정 toolchain 전환이 원인이었다고 확정하지 않는다. 기존 runner에 명시 MSBuild 경로 선택을
추가하고 활성 개발 환경·설치된 VS를 실제 조회해 선택한 설치의 amd64 MSBuild를 사용한다.
C++/SDK 도구의 x64 host를 일치시키고 선택 경로·이유·버전과 프로젝트 전후 build state를
기존 Product 결과에 기록한다. 명시 사용자 선택은 보존하며 도구 누락·실패는 이유를 반환한다.
출력의 존재만으로 성공 처리하거나 timestamp 조작·강제 skip으로 재컴파일을 숨기지 않는다.
기존 4개 FX 병렬화, C++ /MP, 변경 없는 SDK 복사는 유지한다.

### 검증과 종료

수정 전후 기본/왜곡 함수 본문, carrier guard, native ID 및 pass 공유 정의를 비교한다.
853개 source 목록 밖의 World2351~2359도 carrier별 선택 보존 검사에 포함한다.
현재 native Mesh14/Particle5 pass 계약과 table.size() 기반 renderer 준비 경로를 유지한다.
분할 후 CSO 합계와 Effect 최초 준비 비용은 컴파일 시간 개선과 별도로 기록한다.
프로젝트 XML과 생성기 Python/빌드 PowerShell 구문을 검사하고 기존 셰이더 closure 검사를
새 실제 표로 수행한다. root만 Product 빌드를 소유하며 같은 도구의 첫 변경 빌드와 무변경
Build를 순차 측정한다. 필요한 모든 CSO 생성, Client 링크·배포와 증분에서 재생성이 없음을
확인한다. 한 leaf 변경의 영향 범위는 기존 tracking으로 확인하되 수식 변경은 하지 않는다.
로그·측정은 `out/BuildTimeOptimization20260912/`, 결과는 대응 RESULT의 G09에 기록한다.
Client 실행·UI 조작·화면 캡처는 하지 않으며 컴파일 시간 개선을 FPS나 visual PASS로 쓰지 않는다.
