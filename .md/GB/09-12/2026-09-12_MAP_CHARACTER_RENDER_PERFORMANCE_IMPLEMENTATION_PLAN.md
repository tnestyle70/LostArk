# 맵·캐릭터 렌더링 비용 개선 구현 계획

## G00. 현재 측정과 작업 기준

시작 소스는 `a7205863`, 브랜치는 `codex/map-character-render-performance`다. 시작 worktree는 clean이며 origin/main은 같은 tree의 후속 merge 두 commit이다. 사용자는 Character Select의 맵을 우선 조사하고 캐릭터도 함께 최적화하도록 요청했다. 추가 캐릭터는 Effect Tool에서 띄웠고, 최신 제공 캡처는 캐릭터 두 개와 맵 조건이다.

`Client/Bin/ProfilerCaptures/profiler_20260912_161101_142_frame390_8384_0.json`의 worker 완료 이후 241–386, GPU valid 146프레임을 기준으로 사용한다. 평균 interval 15.381ms, CPU 15.262ms, Client.Render 10.879ms다. 맵 직접 계측 self 합계 3.722ms 중 Map.Batch.Mesh.Submit 1.929ms/98회가 크다. 캐릭터 두 모델 animation 1.232ms, palette build 0.190ms다. Shadow GPU elapsed 8.511ms는 이후 CPU 제출 시간과 연동되므로 순수 shader 실행시간으로 확정하지 않는다.

Alt+V의 scene backdrop은 활성화될 때 기존 맵 본체·그림자를 생략하며 연출 카메라도 달라진다. 최신 캡처에는 effect playback이 없어 사용자 관찰의 정확한 원인은 A/B 측정 전이다. 기존 instancing, camera revision 및 동일 payload upload cache, pose revision palette, source light stencil 최적화는 유지한다.

## G01. 실제 light volume 밖의 맵 그림자 제출 제거

기존 ShadowLight가 만든 view/projection 행렬을 읽어 보수적인 sphere 판정을 구성한다. 카메라 frustum으로 그림자를 자르지 않는다. 정적 배치의 실제 world bounds와 light volume이 만나지 않을 때만 그림자 instance에서 제외하며, invalid bounds나 판정 불가 상태는 기존 제출을 유지한다. light 행렬 변경과 placement/visibility 변경은 cache를 무효화한다. buffer upload는 후보 준비가 성공한 뒤 commit하고 실패 시 다음 시도에 재사용할 이전 상태를 보존한다. castsShadow=false 배치는 준비·queue 전에 제외한다.

소유 후보는 `Client/Private/MapStaticBatchObject.cpp`, 대응 H와 `MapAssetRenderUtils`의 공용 cull helper다. light 행렬의 기존 read-only 소비 경로를 우선하며 필요한 Engine getter만 추가한다. 새 C++ 파일을 추가하지 않으므로 project/filter 등록 추가는 없다. 실제 설치 WModel/placement audit에서 authored-visible 746개 중 light sphere 교차 410개라는 후보를 확인했으며 이 수치는 실행 FPS가 아니다.

## G02. 단순 재질의 그림자 shader 계산 분리

`Shader_VtxMeshMapInstance.hlsl`의 기존 shadow pass는 일반 VS_MAIN으로 tangent frame과 RNM/static lighting 입력을 계산한다. 기존 `Bind_ShadowMaterial`이 빠르게 처리하는 family 0~5의 alpha 계약을 먼저 확인한 뒤, 위치와 필요한 UV/alpha만 계산하는 shadow pass를 기존 pass 뒤에 추가한다. family 0의 transformed UV/tint alpha, family 1/2 raw UV alpha, family 3/4/5 opaque 계약을 보존하고 복잡한 family는 기존 pass를 사용한다. shader pass 수의 실제 admission 소비자도 함께 갱신한다. shader 데이터/Resources를 수정하지 않는다.

기존 offscreen shader probe를 재사용해 이전/수정 depth, cutout, cull/mirror 조건과 상태 전환을 비교한다. pipeline 호출 수와 GPU elapsed는 별도 fixture 수치로 기록한다.

기존 0~17 pass는 유지한다. 18~20은 family 0~5의 간소화 alpha 경로, 21~23은 source material이 활성화된 family 3/4/5만 사용하는 opaque depth-only 경로다. 후자는 위치 VS와 null PS로 표면 픽셀 계산을 생략한다. source material 설정이 꺼지면 해당 family도 기존 legacy alpha를 검사하는 18~20으로 제출한다. CShader는 Effect reflection의 실제 pass 개수만큼 input layout을 준비하므로 고정 pass count 계약이나 새 project 등록은 없다.

## G03. 캐릭터 그림자와 장비 포즈의 반복 준비

`Part_Body.cpp` 및 `Part_Equipment.cpp`의 animated shadow는 diffuse alpha만 읽는데 일반 재질 전체를 준비한다. 실제 shader와 모든 caller를 확인해 animated shadow에 필요한 diffuse만 연결한다. socketed static weapon의 복잡 surface 분기는 유지한다. `CModel::Pose_BonesFrom`의 동일 source pose 복사 재사용은 source identity/revision과 대상 pose 변이까지 검증된 경우에만 적용한다. source pointer 수명과 clone 초기값, secondary motion 및 같은 프레임 pose 교체를 보존한다.

## G04. 검증과 실행 준비

기존 파일 인코딩·줄바꿈을 유지한다. 변경 CPP/FX의 Debug 컴파일, 필요한 culling/alpha/pose 수치 동등성, XML parse와 `git diff --check`를 확인한다. 같은 working tree에서 기존 MSBuild와 겹치지 않으며 실행 중 Client/Server를 종료하지 않는다. 제품 출력이 잠겨 있으면 먼저 별도 out 경로의 컴파일을 완료한다. Engine public 변경 뒤 제품 빌드는 Engine→Client 소비까지 확인한다.

RESULT는 구현, 자동 검증, 사용자 FPS·화면 미확인을 분리한다. 에이전트가 Client/UI를 실행·조작·캡처하지 않는다. 최종 사용자는 같은 camera·해상도·캐릭터 수로 기본 상태와 Alt+V 구간을 각각 기록한다. fixture 절감량을 실제 FPS 달성으로 환산하지 않는다.


## G06. 쿠크 후속 측정과 진단 목록 비용

사용자는 G01–G05 반영 후 Character Select가 130fps로 깔끔하게 실행된다고 확인했고, 같은 방식으로 쿠크 전체의 렌더링 병목 개선을 요청했다. 후속 시작점은 사용자 commit `aac4fbdd`이며 worktree는 clean이다. 새 입력은 `profiler_20260912_164743_351_frame104_63936_0.json`이다. UI 전환 frame 52와 다음 interval을 제외한 54–100, 47프레임 평균 CPU17.983ms, interval18.397ms다. 이 0.865초 구간을 장시간 안정 프레임이나 궁극기 성능으로 일반화하지 않는다.

쿠크 기본 profile의 shadow.enabled=false여서 shadow enqueue962는 실제 draw가 아니다. opaque 맵119mesh 제출의 BindAndDraw5.057ms 중 material2.149ms, submit2.499ms다. 1234batch 중962개가 visibility 경로를 통과하지만 culling 계산은0.015ms로 작다. 맵 batch 수를 FPS 원인으로 단정하지 않고 실제 hot caller를 줄인다.

`MapAssetRenderUtils.cpp`의 RecordSurfaceBinding은 매 draw마다 mutex, 32개 문자열 비교·만료 삭제·이동을 수행하며 유일 소비자는 MainApp Rendering Workbench다. 소비자가 최근 요청한 동안만 목록을 수집하고, 닫힌 상태에는 목록·문자열 갱신을 건너뛴다. 기존 1초 expiry와 level 교체 격리는 보존한다. 첫 pane 표시 후 다음 render에서 최신 목록이 채워지는 실제 UI 의미를 표시한다. 현재 함수와 수정 함수의 목록·만료·level 전환 및 Debug 비용을 out 임시 fixture로 비교한다.

## G07. 쿠크 source BG opaque 재질 준비와 shader

쿠크 catalog의 material1732개 중1698개는 family8(BG_SOURCE_OPAQUE_MASKED)다. MapStaticBatchObject의 실제 mesh surface가 family8이고 source material 설정이 활성화된 경우에만 같은 shader의 전용 opaque pass24–26을 사용한다. 기존0–23 index와 source off/다른 family 경로는 보존한다. dedicated PS는 기존 EvaluateMapSourceBGSurface와 동일한 BG 간접광·emissive 수학을 소비하며 다른 family의 큰 분기와 resource 의존성을 제거한다. 하위 수학은 기존 helper를 공유하고 같은 RNM, static shadow, cutout, mirror, debug marker와 8개 MRT를 유지한다.

`MapAssetRenderUtils`의 같은 CModel→CMaterial bind 경로에서 family8일 때만 legacy UV/tint/normal용 반복 설정을 생략한다. source diffuse, normal/specular/reflection flags, emissive time, RNM와 static shadow, diffuse mirror reset은 여전히 실제 material에서 가져온다. 일반 opaque 소품의 presentation dither가 읽는 profile.opacity는 유지한다. 다른 shader draw가 앞에 온 상태 및 source on/off 변경도 올바르게 재설정한다. 새 C++/shader 파일이나 prototype을 추가하지 않으므로 project/filter 등록은 없다.

검증은 실제 HLSL/FX의 기존 pass와 전용 pass에 같은 입력을 주는 offscreen 수치 비교로 수행한다. 8 MRT와 depth, 다양한 실제 source flags, RNM, alpha, mirror, cull, emissive, source toggle와 오염된 선행 상태를 포함한다. fixture의 resource 수·CPU 제출 개선을 실제 게임 FPS로 환산하지 않는다. 최종 Product Debug 빌드와 사용자 쿠크 실행 비교는 별도로 기록한다.


## G08. 쿠크 상시 native particle의 공통 입력 정리

기본5개 월드 marker는 effect.world.move_destination의12 emitter를 사용하며 실제 ARTIST/Kouku native particle family가 소비된다. Effect_DocumentRenderer.cpp의 Bind_MaterialInputs에서 native packet/profile/clamp/source texture와 UVScale/UVOffset, ColorMultiply/ColorOffset, EmissiveIntensity, ColorClip은 보존하고 해당 native particle shader가 읽지 않는 generic flag/texture 설정만 건너뛴다. mesh와 다른 family는 기존 binder를 유지한다. source packet과 shared shader 선행 상태를 포함해 실제 shader dependency 및 old/new 숫자·binding 비교, 변경 CPP 컴파일을 수행한다. 새로운 영구 하네스/파일/저장계약은 추가하지 않는다.


## G09. 렌더 제출 큐의 프레임별 메모리 할당 제거

Renderer의 렌더 큐는 list<shared_ptr<CGameObject>>이며 매 프레임 push_back/clear로 노드를 생성·해제한다. 쿠크 맵의 NONBLEND962+SHADOW962만으로1924개의 노드가 반복된다. 동일 순서와 소유 수명을 유지하는 vector로 교체해 clear 이후 capacity를 재사용하고, GameInstance→Renderer 전달에서 이미 소유한 shared_ptr을 move해 불필요한 참조 증가를 줄인다. blend 정렬·그리기 도중 제출·queue 정리 등 전체 호출자를 먼저 확인하고 iterator 수명 계약을 보존한다. 최종 camera 준비보다 앞에서 맵을 잘라내지 않는다. 기존 큐와 새 큐의 순서, 수명, 비활성 shadow/scene replacement 정리 및 반복 프레임 할당을 검증한다. Engine public header 배포와 Client 소비까지 Product 빌드로 확인한다.

## G10. 100fps·Alt+V 40fps 후속 측정과 조명 제출

사용자 merge PR369의 소스 5168899d가 적용된 17:30~17:31의 세 캡처를 비교한다. 시작 위치251–548은 interval13.914ms, 3관문2161–2416은18.488ms다. 맵 BindAndDraw는1.688→1.259ms로 줄지만 조명 CPU는1.576→3.420ms, GPU PS는11.330M→23.741M로 늘었다. Alt+V 마지막3918–3953은 interval38.396ms, 조명 CPU7.664ms/GPU8.688ms/PS192.60M다. NonBlend elapsed에는 CPU 제출 간격이 섞이므로 map shader 단독 비용으로 읽지 않는다. 로딩 frame1624의1.681초 지연과 ring 누락/미완료 query는 일반 FPS 비교에서 분리한다.

기존 CRenderer→CLight_Manager→CLight의 실제 조명 경로에서 반복되는 전체 화면 제출을 줄인다. scene→transient 순서, receiver, 첫 directional shadow와 static channel, source material row, stencil, FP16 누적 순서를 유지한다. 동등한 조명들의 입력은 한 번 준비하고 기존 quad의 instanced 제출 또는 보수적인 투영 영역 제한을 검토한다. 조명 수·반경·밝기와 화면 해상도는 성능을 위해 줄이지 않는다. invalid 투영/near-plane 교차는 기존 전체 영역을 유지하며 명시적 실패는 기존 frame 실패 경로로 전달한다. 기존 shader pass는 유지하고 새 pass는 뒤에 추가한다. 실제 구현 전 shader 전역 입력·D3D 상태 소유를 확인하며 새 C++ 파일은 필요할 때만 project/filter에 함께 등록한다.

## G11. Alt+V CPU 준비와 native 재질 중복

Alt+V 구간의 Animation.Channels.Update4.130ms, Sprite.InstanceBuild3.787ms, Effect.FrameRebuild1.615ms를 실제 호출·데이터로 나누어 조사한다. 동일 emitter/pose에서 반복되는 준비만 제거하고 입자 수·수명·정렬·발생 순서·bone/socket 변환을 보존한다. 같은 Bind_Material 호출 안의 중복 SurfaceLighting와 native source texture 이름 생성은 실패 순서를 보존하면서 정리한다. native character의 미사용 legacy 입력 생략은 실제 shader base pass가 입증된 caller에만 적용한다. 객체별 uniform 캐시는 공유 Effect의 sibling 변경을 놓치므로 사용하지 않는다.

현재 공유 checkout의 쿠크 연출·데이터·MainApp 변경은 다른 작업이 소유한다. 해당 변경을 보존하고 이번 성능 변경의 파일과 검증 증거를 따로 기록한다. headless 수치 비교와 최소 컴파일을 먼저 완료하고 제품 빌드는 다른 MSBuild 및 실행 중 Client/Server와 겹치지 않는다. 100fps/40fps 달성은 같은 위치·같은 연출의 사용자 재캡처 전까지 미확인으로 남긴다.


G11의 compact WAnimation 키 탐색은 CAnimation이 실제 재생한 clip에만 scale/rotation/translation별 left cursor를 할당한다. CChannel의 공유 key 배열은 변경하지 않는다. 현재 time이 보관 구간에 포함될 때만 그 구간을 쓰고, seek·역재생·loop·중복 timestamp·다른 clone time은 같은 upper_bound의 raw pointer 탐색으로 돌아간다. transition용 stateless sampling도 기존 값과 보간 수학을 유지한다. Channel/Animation의 기존 H/CPP만 수정하며 project/filter 추가는 없다.


G10 조명 shader 정본은 Engine/Bin/ShaderFiles/Shader_Deferred.hlsl이다. 같은 Git 관리 Client 사본도 동기화하며, Product의 PrepareEngineSdk가 Engine→EngineSDK→Client로 배포하는 방향을 따른다. 400개 light record(16scene+384transient,112byte/개)는 별도44,800byte constant buffer에 담고 같은 type의 연속 구간만 DrawIndexedInstanced로 제출한다. 기존0–21 pass와 light helper wrapper를 보존하고 ordinary/source-stencil의 directional/point/spot pass22–27을 뒤에 추가한다. 범용 CVIBuffer::Render_Instanced는 실제 Light_Manager가 소비하고 기존 profiler draw/instance/index 카운터를 유지한다.


G11 최종 비교에서 기존 Channel/Animation 모두 /Od와 변경 Channel/Animation 모두 /O2를 별도 translation unit으로 링크해 검사한다. bone storage와 main은 /Od, Debug CRT·_DEBUG·checked iterator·/fp:precise는 유지한다. compact 및 legacy combined-key 649,002개 행렬과 seek/clone/loop 결과가 bitwise 일치했으므로 Engine.vcxproj의 기존 Animation.cpp와 Channel.cpp 두 항목에만 Debug x64 MaxSpeed를 적용한다. 기존 Shader.cpp/Profiler.cpp와 같은 BasicRuntimeChecks=Default, ProgramDatabase, SupportJustMyCode=false를 사용한다. 이 두 파일의 최적화된 debug stepping·지역변수 관찰은 제한될 수 있으며 Engine 전체 Debug ABI는 변경하지 않는다. 새 파일과 filters 등록은 없다. 최종 Product compile command에서 두 파일의 /O2 적용을 확인한다.


## G12. Debug 180fps 목표와 실제 광원 영향 영역

사용자는 Release가 아닌 현재 Debug에서180fps를 목표로 선택했다. 기본 창 크기1280×720를 유지하며, CPU frame과 GPU frame 각각5.56ms 이내가 목표 예산이다. 18:20 Product 이후 사용자가 저장한20:06 쿠크 캡처의190–249,60프레임은 interval13.251ms(75.46fps), CPU12.357ms다. 약0.795초의 짧은 표본이며17:31의71.87/54.09/26.04fps를 현재 비용으로 재사용하지 않는다. Debug CRT·D3D debug layer·일부 /Od와 Release의 차이는 측정 조건으로 유지하고, 일괄 Release 전환으로 목표를 대체하지 않는다.

다음 렌더링 변경의 첫 범위는 Engine Light_Manager 및 Deferred shader의 source-character point/spot 광원이다. 현재 Resolve_SourceCharacterLight는 attenuation<=0일 때 native 프로그램보다 먼저 discard하고 native RGB와 별도 ambient도 attenuation으로 감쇠한다. 따라서 이 공통 wrapper에 들어오는 local light의 fRange 밖은 기여가0이다. 같은 카메라의 광원 구체를 보수적으로 화면에 투영해 기존 전체 quad를 영향 영역으로 제한한다. spot은 우선 range 구체로 감싸고 directional은 전체 화면을 유지한다. near-plane 교차·nonfinite·projection 불가·경계 오차는 영역을 확대하거나 기존 전체 quad로 돌아간다. forward native 및 다른 map family에 이 근거를 확대하지 않는다.

기존 light record·instanced VS의 실제 소비를 확장하고 scene→transient·light별 FP16 blend 순서를 유지한다. 구현은 Engine/Private/Light_Manager.cpp, Engine/Bin/ShaderFiles/Shader_Deferred.hlsl와 동일 Client 사본의 현재 경로를 사용한다. 새 C++·prototype·두 번째 renderer·Resource payload를 추가하지 않는다. 계획 단계에서 project/filter 등록 추가는 없다. 투영 bounds의 포함 관계, camera/near-plane/해상도, source receiver/row, 실제 이전·수정 FP16 출력, PS invocation과 CPU/GPU 시간을 기존 out 수치 fixture에서 비교한 뒤 Product 빌드를 한다. 현재 섹션은 후속 계획이며 구현 완료 기록이 아니다.

## G13. 가시성·CPU 준비·재질 제출 구조의 후속 순서

현재 맵 CPU sphere frustum과 assetId+mirror instancing은 유지한다. 공간 계층과 큰 mesh의 부분 bounds로 candidate를 줄이고 최종 camera/light가 결정된 뒤 가시성을 확정하는 단계가 필요하다. 가려진 객체의 geometry 제출을 줄이는 occlusion은 이 후보 집합을 소비해야 한다. shadow caster는 camera visibility와 분리한다. 캐릭터/NPC는 authoritative state·animation/cue clock을 유지하면서 화면·그림자·socket의 실제 소비 여부에 따라 visual pose와 draw 준비를 분리한다.

Effect는 기존60Hz simulation과 presentation을 구분하고 simulation/root/anchor/camera revision에 맞춰 FrameRebuild와 instance 준비의 재사용 범위를 넓힌다. simulation step이0인 frame에도 현재 FrameRebuild를 수행하는 실제 경로가 대상이며, 입자 수·발생 순서·속도·trail·부착점은 유지한다. Draw/Effects Apply와 CShader의 공유 Effect 소유는 그대로 두고 먼저 CPU 준비를 줄인다. 병렬화 대상은 독립 계산으로 한정하며 같은 immediate context를 여러 worker에서 동시에 호출하지 않는다.

GPU source-row/tile 가시성은 G12 이후의 별도 변경이다. 현재 global marker-5 mask 한 번을 row마다 전체 화면 mask로 바꾸면 source 면적이 작은 장면에서 mask 비용이 커지므로 단순 교체하지 않는다. GPU에 실제 남은 depth/material row의 점유 영역과 local light 영역을 교차하는 방향으로 설계한다. 최신 frame 측정 없이 MRT8개 축소·depth prepass·후처리 합치기를 일괄 적용하지 않는다.

## G14. Geometry·material LOD의 구현 전제

현재 CModel의 MODEL_MESH_DATA는 단일 vertices/indices이고 runtime LOD chain/전환 기준이 없다. GPU가 완성된 저해상도 mesh를 자동 생성한다고 가정하지 않는다. 기존 변환기·CModel→CMaterial asset 경로에 LOD별 geometry와 화면상 오차 기준을 먼저 연결한 뒤 CPU 또는 GPU가 선택한다. texture mipmap과 runtime geometry LOD를 구분한다. 정적 map에서 시작하고 material 비용/애니메이션 pose 빈도도 별도 LOD 대상으로 평가한다. GPU cull/LOD 선택·indirect draw는 이 데이터와 가시성 계약을 소비하는 후속이며, geometry 병목 실측과 사용자 실루엣/전환 판정을 거친다.


## G12–G14. 20:06 쿠크 캡처 이후 구현 단위

20:06 캡처는 맵 mesh submit21회와 BindAndDraw0.464ms, 전체 draw255회, light CPU0.672ms를 기록했다. NonBlend 등록991개는 실제 draw 수가 아니다. ImGui BuildAndSubmit1.952ms, particle Render1.327ms, FrameRebuild0.207ms와 객체 갱신도 CPU 비용을 차지한다. GPU timestamp에는 CPU 제출 간격이 포함될 수 있으므로 NonBlend5.228ms를 순수 맵 shader 비용으로 단정하지 않는다.

G12의112-byte light record는 예약 flags.w를 source local bounds 활성화에 사용한다. instanced VS는 range 구체를 감싸는8개 world box 꼭짓점을 현재 source camera로 투영하고 기존 quad의 위치·UV를 유지한 채4개 clip distance로 제한한다. 해상도 margin은 기존 orthographic projection에서 구해 VS에 새 depth SRV를 추가하지 않는다. directional/ordinary와 near-plane·invalid 투영은 전체 영역을 유지한다. 기존 FP16 순서·출력 및1280×720 PS invocation을 비교한다.

G13의 Effect_Playback H/CPP는60Hz step이 없고 root/anchor/document 상태가 동일한 경우 FrameRebuild 결과를 재사용한다. 외부 frame provider는 매 호출 검사하며 실패나 상태 변경은 cache를 무효화한다. Effect_DocumentRenderer H/CPP는 실제 native particle VS의 위치 변환 계약이 일치하는 sprite만 최종 camera clip XY 밖일 때 material/upload/draw 전에 제외한다. 크기·alpha·depth occlusion 추정으로 입자를 숨기지 않고 invalid 계산은 제출을 유지한다.

G13의 Shader H/CPP는 같은 Effect를 공유하는 Clone 전부의 실제 마지막 입력을 동일한 EFFECT_BINDINGS owner에서 기록하는 방식을 검증한다. 객체별 uniform cache는 사용하지 않는다. 작은 raw/matrix 입력과 단일 SRV의 완전히 동일한 재설정만 생략하고, setter 종류 변경·array/큰 입력·실패는 해당 값 기록을 무효화한다. pass Apply는 생략하지 않는다. 모든 Effect variable 쓰기가 CShader를 통하는 현재 private 소유 계약과 clone 교차, 서로 다른 Effect, 선행 상태 오염, 실패 재시도, raw/matrix/array 교차 및 실제8 MRT 출력/제출 시간을 확인한다. 개선이 없으면 이 변경은 채택하지 않는다.

G14는 CMesh가 기존 MODEL_MESH_DATA의 decoded static vertex/index를 소비할 때 원본 vertex buffer를 유지하고 seam/border를 보존하는 index LOD를 준비한다. meshoptimizer v1.0의 고정 commit73583c335e541c139821d0de2bf5f12960a04941에서 MIT source subset를 vendor하며 Engine project/filter에 등록한다. Engine private StaticMeshLod H/CPP가 GPU resource와 selection을 소유하고 public MeshLod.h는 실제 CModel/MapStaticBatch caller의 bounds·screen error 입력이다. Shader_MeshLod.hlsl은 Engine 정본으로 컴파일하고 기존 SDK shader 복사 및 Client compiled shader 배포 target에 연결한다.

최종 camera CPU cull에서 남은 큰 static opaque mesh에만 GPU dispatch를 수행하고, visible batch의 가장 가까운 instance보다 보수적인 bounds·최대 scale로 단일 LOD를 선택한다. 합쳐진 index buffer range와 DrawIndexedInstancedIndirect1회로 제출해 LOD별3개 draw를 만들지 않는다. 작은 mesh, animated/morph/투명/변위 재질과 invalid/near projection은 기존 LOD0 경로를 유지한다. shadow는 기존 LOD0/light culling이다. MapStaticBatch의 전체 bounds broad phase는 per-instance margin·히스테리시스를 포함한 확실한 외부 batch에만 적용하며 최종 camera revision과 placement 변경을 반영한다. Resources 원본이나 별도 모델 runtime을 만들지 않는다. 실제 GPU index 수를 CPU가 알 수 없는 indirect counter는 LOD0 상한과 구분하며 동기 readback을 넣지 않는다.

최소 검증은 새 CPP/CS 컴파일과 기존 out 수치 fixture에서 index 범위·감소율·경계/invalid fallback·clone 수명·GPU selected args·큰/작은 mesh 손익을 확인한다. Engine public header 변경은 Product SDK 배포와 Client 전체 소비까지 빌드한다. 최종 visual/LOD 전환과180fps는 사용자의 같은 위치·해상도·캐릭터·Alt+V 재캡처로만 판정한다.


G14 최종 적용은 실측 손익에 맞춰 draw별 원본 index×instance≥294,912로 제한한다. 작은 draw는 기존 direct LOD0이며 GPU가 쓸모없는 선택 비용을 추가하지 않게 한다. `.25px`는 meshoptimizer의 속성 포함 오차 지표를 투영한 선택 기준으로, 엄밀한 최대 실루엣 오차 보장을 뜻하지 않는다. Engine project의 새 StaticMeshLod·vendor·compute shader 등록, public MeshLod header와 Client compiled-shader 배포는 실제 소비 경로까지 함께 반영한다.
