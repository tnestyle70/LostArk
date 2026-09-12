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
