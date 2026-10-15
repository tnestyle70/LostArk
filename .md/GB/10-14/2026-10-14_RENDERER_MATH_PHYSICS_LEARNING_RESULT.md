# LostArk 렌더러의 수학 물리와 실제 구현

목표는 한 픽셀이 만들어지는 과정을 CPU 코드, GPU 셰이더, 좌표 변환, 빛의 물리 모델로 이어 설명하는 것이다. 기술소개서에는 이 연결의 핵심을 싣고, 면접에서는 필요한 함수·변수·수식과 선택의 비용으로 내려간다. DX11 사용, Deferred 사용, PBR 사용은 서로 다른 선택이다. DX11은 그래픽 API, Deferred는 계산과 저장의 순서, PBR은 빛과 재질을 모델링하는 방식이다.

2026-10-14 작업 폴더의 현재 코드를 기준으로 한다. 실제 성능 수치는 아직 넣지 않는다. 아래 예제의 해상도·굴절률·좌표는 계산 원리를 확인하는 가정이며 제품 실행 측정값과 구분한다. GPU Zem은 앞선 학습 기록과 이번 문맥에 따라 GPU Gems를 뜻하는 것으로 정리했다.

| 읽는 목적 | 시작할 구간 |
|---|---|
| 전체 학습 순서부터 보기 | G16 책의 25장 지도 → G00 한 프레임 |
| 수학부터 이해하기 | G01~G02 → G17 유도와 증명 → G14 실제 카메라 |
| 빛과 PBR 이해하기 | G05~G07 이론 → G08~G09 실제식 → G11 합성 |
| GS·CS·안개·애니메이션 이해하기 | G18 단계별 적용 → G14 bone·skinning |
| PPT·면접에 연결하기 | G19 작은 계산 → G21 결과물·검증 범위 |

## G00 한 프레임을 만드는 실제 파일

Client의 [CMainApp::Render](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:4246)는 렌더 시작과 Level 정책을 결정한다. [CGameInstance::Render](C:/Users/tnest/Desktop/LostArk/Engine/Private/GameInstance.cpp:267)가 [CRenderer::Draw](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:899)를 호출하고, 마지막에 [CGraphic_Device::Present](C:/Users/tnest/Desktop/LostArk/Engine/Private/Graphic_Device.cpp:202)가 swap chain을 표시한다.

~~~mermaid
flowchart LR
    A[Client의 최종 카메라와 객체 제출] --> B[Shadow와 불투명 G-buffer]
    B --> C[SSAO와 광원 누적]
    C --> D[HDR 합성]
    E[Forward 투명 재질과 이펙트] --> D
    D --> F[왜곡과 화면 효과]
    F --> G[Bloom과 Tone mapping]
    G --> H[AA와 일반 UI]
    H --> I[Present]
~~~

Shadow·SSAO·Bloom·AA 등은 설정과 입력에 따른 조건부 단계다. 정확한 순서는 Shadow, 요청된 Portrait, G-buffer와 DeferredOverlay, opaque picking depth 보관, SSAO, Light accumulation, HDR 안의 Priority·Combined·SSGI/SSR·NonLight·SceneReplacement·요청된 SceneColor snapshot·Blend·SceneUI, ScreenPosts, Bloom, Final, DisplayOverlay, UI, Debug, Picking이다. 개념도에서 생략한 단계도 실제 호출 순서에는 존재한다.

| 파일과 실제 객체 | 소유하는 정보와 책임 |
|---|---|
| [Renderer.h](C:/Users/tnest/Desktop/LostArk/Engine/Public/Renderer.h) / Renderer.cpp | render group, pass 순서, 중간 RT, 렌더 설정, 최종 합성 |
| [Target_Manager.cpp](C:/Users/tnest/Desktop/LostArk/Engine/Private/Target_Manager.cpp:106) | RT 태그와 MRT 묶음, 출력 바인딩과 복원 |
| [RenderTarget.h](C:/Users/tnest/Desktop/LostArk/Engine/Public/RenderTarget.h:11) | Texture2D 한 개와 RTV/SRV의 COM 수명 |
| [Shader.cpp](C:/Users/tnest/Desktop/LostArk/Engine/Private/Shader.cpp:232) | CSO의 Effects11 객체, pass, 변수·texture·matrix 바인딩 |
| [VIBuffer.cpp](C:/Users/tnest/Desktop/LostArk/Engine/Private/VIBuffer.cpp:64) | VB·IB·topology 연결 및 indexed draw 제출 |
| [Light_Manager.cpp](C:/Users/tnest/Desktop/LostArk/Engine/Private/Light_Manager.cpp:239) | 광원 입력, 광원 batch와 실제 lighting draw |
| [Shader_Deferred.hlsl](C:/Users/tnest/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_Deferred.hlsl) | 저장된 표면의 조명, AO, 합성, Bloom, 최종 화면 계산 |

CPU의 DrawIndexed 호출은 GPU 작업의 제출이다. 함수가 반환됐다고 그 삼각형의 픽셀 계산이 끝났다는 뜻은 아니다. 셰이더의 입력을 바인딩한 CPU 호출 순서와 GPU가 그 명령을 실행하는 시점을 구분한다.

## G01 정점과 빛을 설명하는 벡터 수학

점 p는 위치, 벡터 d는 방향과 크기를 나타낸다. 두 점의 차 p₂−p₁은 이동 벡터다. 동차좌표에서는 점을 (x,y,z,1), 방향을 (x,y,z,0)으로 두어 이동 행렬이 점에만 작용하게 한다. 빛 방향과 법선에 위치 이동을 적용하면 조명 결과가 잘못된다.

내적 a·b=|a||b|cosθ는 두 방향의 정렬 정도와 정사영을 계산한다. 단위 법선 N과 표면에서 광원으로 향하는 L이면 max(N·L,0)이 입사광의 면적 투영 항이다. N·V는 시선에 대한 표면의 기울기다. 서로 다른 좌표계의 벡터를 내적하면 같은 방향을 비교하는 계산이 아니다.

외적 a×b는 두 벡터에 수직인 방향을 만든다. 카메라의 right/up/look, 삼각형 기하법선, tangent basis에 쓰인다. 외적의 피연산자 순서를 바꾸면 부호가 뒤집힌다. normalize는 방향을 유지하고 길이를 1로 만들지만 길이 0에 대한 나눗셈은 정의되지 않으므로 fallback이나 epsilon이 필요하다.

반사 방향은 입사 진행 방향 I에 대해 R=I−2(N·I)N이다. 환경 반사에서 V를 표면에서 눈으로 향하는 방향으로 정의했다면 reflect의 입사 인자는 −V다. 프레넬은 이 방향에 실을 반사 비율이고, reflect는 샘플할 방향이다.

스케일·회전·이동의 순서는 결과를 바꾼다. 행벡터 관례에서는 p_world=p_local S R T다. 회전 후 이동은 물체를 제자리에서 돌린 후 배치하고, 이동 후 회전은 이동된 점을 원점 주위로 돌린다. 행렬 곱은 일반적으로 교환되지 않는다.

## G02 카메라와 World View Projection

카메라를 움직이는 것은 장면을 카메라 좌표계에서 다시 표현하는 일이다. W는 local→world, V는 world→view, P는 view→clip을 담당한다.

$$p_{clip}=(x,y,z,1)\,WVP,\qquad p_{ndc}=p_{clip}.xyz/p_{clip}.w$$

DirectXMath는 row-major 저장과 행벡터의 왼쪽 곱을 사용한다. LH/RH는 선택한 함수가 정하며 DX11 자체가 LH만 강제하는 것은 아니다. 저장 순서와 수학적 곱셈 방향은 별개다. [Microsoft DirectXMath의 행렬 관례](https://learn.microsoft.com/en-us/windows/win32/dxmath/pg-xnamath-getting-started#matrix-convention)

카메라 월드 행렬을 C라고 하면 V=C⁻¹이다. 카메라 좌표를 이미 월드 좌표라고 취급한 뒤 다시 V를 곱하거나, 월드에서 view로 바꾼 빛 방향을 월드 법선과 내적하면 공간을 두 번 섞게 된다.

LH look-at의 전방 ẑ=normalize(target−eye), 오른쪽 x̂=normalize(up×ẑ), 위 ŷ=ẑ×x̂다. 이 기저를 기준으로 view 좌표는 ((p−eye)·x̂,(p−eye)·ŷ,(p−eye)·ẑ)가 된다. up과 전방이 평행하면 basis를 만들 수 없으므로 다른 up이나 기존 방향을 사용해야 한다.

세로 FOV φ, aspect a=width/height, near n, far f인 일반 LH 원근 행렬은 행벡터 기준 다음과 같다.

$$P=
\begin{bmatrix}
1/(a\tan(\phi/2))&0&0&0\\
0&1/\tan(\phi/2)&0&0\\
0&0&f/(f-n)&1\\
0&0&-nf/(f-n)&0
\end{bmatrix}$$

이때 w_clip=z_view이고 z_ndc=f/(f−n)−nf/((f−n)z_view)다. near에서 0, far에서 1이 되지만 거리에 선형이지 않다. near를 과도하게 작게 잡으면 일반적인 깊이 분포에서 먼 영역의 분해능이 나빠진다. reverse-Z는 별도의 projection·clear·depth 비교 계약이며 이 공식과 섞지 않는다.

직교 투영은 거리에 따른 크기 감소가 없다. UI, 일부 그림자·도구 카메라에 적합한 선택이지만 원근 카메라와 w·depth 관계가 다르다.

뷰포트 변환은 x_screen=(x_ndc+1)width/2, y_screen=(1−y_ndc)height/2다. y 부호 반전은 위쪽을 양수로 두는 NDC와 아래로 증가하는 화면 좌표를 연결한다.

### 현재 프로젝트의 행렬 전달

[CShader::Bind_Matrix](C:/Users/tnest/Desktop/LostArk/Engine/Private/Shader.cpp:750)는 CPU 행렬을 직접 transpose하지 않고 Effects11 SetMatrix에 전달한다. HLSL 행렬 선언·Effects11 저장 규칙·mul(v, W*V*P) 방향을 함께 읽어야 한다. [공식 FX11 SetMatrix 구현](https://github.com/microsoft/FX11/blob/main/EffectVariable.inl#L2659-L2728)은 column-major 내부 저장과 row-major 저장을 구분해 복사한다. CPU에 transpose가 없다는 이유만으로 잘못된 전달이라고 판정하거나 한 번 더 transpose하지 않는다. 이번에는 로컬 호출과 공식 구현을 대조했으며 연결된 Effects11 바이너리를 역분석하지 않았다. 정적 메시·스킨·인스턴스의 구체적인 소비 차이는 G14에 연결한다.

### 현재 Depth texture의 월드 위치 복원

[Deferred shader의 복원](C:/Users/tnest/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_Deferred.hlsl:709)은 writer가 저장한 두 값을 함께 사용한다.

$$d_x=z_{clip}/w_{clip},\quad d_y=w_{clip}/1000$$
$$w=1000d_y,\quad p_{clip}=(2u-1,\ 1-2v,\ d_x,\ 1)\,w$$
$$p_{world}=p_{clip}P^{-1}V^{-1}$$

1000은 저장한 값을 원래 clip w로 돌리는 scale이다. 임의의 다른 depth buffer에도 무조건 곱하는 상수가 아니다. 이 경로는 별도로 저장한 clip w를 쓰므로 일반적인 depth-only 역투영의 마지막 동차 나눗셈 설명과 구분한다.

## G03 DX11 파이프라인과 셰이더 실행

표준 그래픽 경로는 IA → VS → 선택적 HS/Tessellator/DS → 선택적 GS → Rasterizer → PS → OM이다. CS는 이 그래픽 단계 사이에 항상 들어가는 단계가 아니라 별도의 dispatch 계산 경로다. 프로젝트의 full-screen Deferred 조명은 raster draw를 사용한다. [Microsoft DX11 Graphics Pipeline](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-graphics-pipeline)

| 단계 | 하는 계산 | 프로젝트에서 따라갈 정보 |
|---|---|---|
| IA | 정점 stream, index, topology 해석 | CVIBuffer::Bind_Resources의 VB·IB·stride·offset |
| VS | 정점 변환, skinning과 varying 생성 | Shader_VtxMeshBinary / Shader_VtxAnimMeshBinary의 VS |
| Rasterizer | clipping, coverage, 보간 | clip 좌표, cull mode, viewport, depth |
| PS | 표면 입력 또는 조명 결과 계산 | normal·material sampling, SV_TARGET 출력 |
| OM | depth/stencil, blend, RT 기록 | DSV·RTV·blend state와 RenderTarget 묶음 |

래스터화는 삼각형의 연속 영역을 유한한 화면 표본으로 옮긴다. 원근 보간에서 속성 a는 화면 barycentric λᵢ에 대해 (Σλᵢaᵢ/wᵢ)/(Σλᵢ/wᵢ)로 계산한다. 화면에서 단순 선형 보간하면 원근이 걸린 texture가 찌그러진다. HLSL의 보간 수식과 normal의 재정규화도 별개다.

프로젝트는 [Shader.cpp:266](C:/Users/tnest/Desktop/LostArk/Engine/Private/Shader.cpp:266)에서 EXE 옆의 CSO bytes를 읽고, :274의 D3DX11CreateEffectFromMemory로 Effects11을 생성한다. [Begin](C:/Users/tnest/Desktop/LostArk/Engine/Private/Shader.cpp:824)은 실제 program variant와 pass를 선택하고 input layout 및 pass를 적용한다. [CVIBuffer::Render](C:/Users/tnest/Desktop/LostArk/Engine/Private/VIBuffer.cpp:29)가 DrawIndexed를 제출한다. 논리 파일명에 .hlsl이 있다고 런타임이 매번 소스 컴파일을 하는 것은 아니다.

cbuffer나 Effects 변수의 CPU/HLSL 레이아웃은 바이트 계약이다. HLSL packing은 기본적으로 16-byte vector 경계를 고려하며 배열·행렬·struct에서 C++ sizeof와 우연히 맞는다고 가정하지 않는다. vector<float3>의 연속 메모리를 float3 상수 배열과 항상 같은 stride라고 설명해서도 안 된다. [HLSL constant packing 규칙](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules)

## G04 Deferred와 Render Target

Forward는 물체를 그릴 때 표면과 조명을 함께 계산한다. Deferred는 먼저 보이는 불투명 표면의 정보를 저장하고 뒤에서 광원을 평가한다. 표면 계산의 재사용과 조명 분리가 장점이며, RT 메모리·대역폭, 재질 종류별 packing, 투명 처리의 부담이 생긴다. 현대 DX11의 모든 Deferred가 MSAA를 사용할 수 없다고 일반화하지 않는다. 현재 프로젝트 RT의 SampleDesc.Count는 1이다. [GPU Gems 3의 Deferred 사례](https://developer.nvidia.com/gpugems/gpugems3/part-iii-rendering/chapter-19-deferred-shading-tabula-rasa)

[CRenderTarget::Initialize](C:/Users/tnest/Desktop/LostArk/Engine/Private/RenderTarget.cpp:28)는 Texture2D를 만들고 RTV와 SRV를 만든다. 두 view는 같은 texture를 서로 다른 용도로 바라본다. RTV는 출력, SRV는 shader 읽기다. DSV는 depth/stencil용 view이며 Target_Depth라는 이름의 color texture와 다르다.

[Begin_MRT](C:/Users/tnest/Desktop/LostArk/Engine/Private/Target_Manager.cpp:106)는 PS의 기존 SRV를 해제하고 대상 RTV를 바인딩한다. 같은 subresource를 동시에 읽고 쓰지 않도록 pass 사이에 해제하거나 별도 texture를 쓰는 ping-pong이 필요하다. API가 충돌한 바인딩을 NULL로 만들 수 있으므로 검은 화면을 texture 파일 누락으로만 해석하지 않는다. [OMSetRenderTargets의 hazard 규칙](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-omsetrendertargets)

### 실제 8개 G-buffer

생성 형식은 [Renderer.cpp:261](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:261), MRT 순서는 [Renderer.cpp:367](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:367)에서 확인한다.

| slot | Target | format | texel byte | 대표 의미 |
|---|---|---|---:|---|
| 0 | Diffuse | RGBA8 UNORM | 4 | diffuse/base color와 coverage |
| 1 | Normal | RGBA16 UNORM | 8 | 인코딩 normal, 재질별 추가 값 |
| 2 | Depth | RGBA32 FLOAT | 16 | clip z/w, clip w/1000, 추가 값, material marker |
| 3 | PickPos | RGBA32 FLOAT | 16 | world position과 packed 보조 정보 |
| 4 | Emissive | RGBA16 FLOAT | 8 | 발광과 재질별 추가 조명 정보 |
| 5 | MaterialSpecular | RGBA16 FLOAT | 8 | specular RGB, metallic 또는 source payload |
| 6 | CharacterSurface | RGBA32 FLOAT | 16 | UV·tangent·source/IBL payload |
| 7 | CharacterGeometry | RGBA16 FLOAT | 8 | source geometry 또는 PBR indirect |

모든 재질에 동일한 채널 의미를 적용하면 틀린다. [일반 MapPBR writer](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl:343)는 depth.w=3, normal.a=roughness, depth.z=AO를 기록하지만 legacy는 normal.a에 specular mask, depth.z에 specular power를 넣는다. Deferred reader가 marker를 먼저 확인해야 하는 이유다.

총합은 84B/pixel이다. 1920×1080을 가정하면 174,182,400B, 약 166.1MiB다. 이는 이 8장 한 세트의 이론적 texel 저장량으로, depth·shadow·HDR·ping-pong·driver 정렬·압축·residency를 포함한 실제 VRAM 수치가 아니다. 대역폭은 이 저장량뿐 아니라 프레임당 읽고 쓰는 횟수와 coverage에 좌우된다.

## G05 빛의 물리량과 렌더링 방정식

색을 세 숫자로 저장해도 빛의 계산에는 구분해야 할 물리량이 있다. 복사속(방사속) Φ는 시간당 복사 에너지, 즉 전력(W)이다. 사람 눈의 감도를 가중하는 광속(lm)과 구분한다. Irradiance E는 면적당 입사 전력(W/m²), radiance L은 투영 면적과 입체각당 전력(W/(m²·sr))이다. 입체각 dω는 방향 범위의 크기다. 렌더링은 주로 방향별 L을 다룬다. [PBRT Radiometry](https://pbr-book.org/4ed/Radiometry,_Spectra,_and_Color/Radiometry)

$$L_o(x,\omega_o)=L_e(x,\omega_o)+
\int_{\Omega^+}f_r(x,\omega_i,\omega_o)\,
L_i(x,\omega_i)\,\max(N\cdot\omega_i,0)\,d\omega_i$$

Lₒ는 카메라 방향으로 나가는 빛, Lₑ는 표면의 자체 발광, Lᵢ는 들어오는 빛, fᵣ는 BRDF다. 식은 들어오는 모든 방향의 기여를 더한다. 그림자는 어떤 방향의 빛이 실제로 도달하는지에 영향을 준다. 빛의 이동 경로와 재질의 반사 분포는 별도 항이다. [PBRT Surface Reflection](https://pbr-book.org/4ed/Radiometry,_Spectra,_and_Color/Surface_Reflection)

실시간 렌더러는 이를 매 픽셀에서 무한히 적분하지 않는다. 직접광은 유한한 광원 합, 환경광은 cubemap·SH·사전 적분, 가시성은 shadow map·screen-space 근사 등으로 계산을 제한한다. PBR을 쓴다는 것은 이 모든 근사가 물리적으로 완전하거나 ray tracing이라는 뜻이 아니다.

프로젝트의 light intensity가 실제 candela나 lumen으로 교정됐다는 근거는 이번 코드 읽기에 없다. 식의 물리량 단위와 게임의 artist scale 값을 같은 측정 단위로 소개하지 않는다.

## G06 BRDF와 Lambert diffuse

BRDF는 주어진 입사 방향의 irradiance 변화가 출사 방향의 radiance로 얼마나 바뀌는지를 나타낸다. 단위는 sr⁻¹이다. 양방향이라는 말은 입사 방향과 출사 방향을 모두 인자로 갖는다는 뜻이다.

$$f_r={dL_o\over dE_i}={dL_o\over L_i\cos\theta_i\,d\omega_i}$$

수동 표면의 반사율은 입사 방향 하나에 대해 ∫fᵣ cosθₒ dωₒ≤1이어야 한다. fᵣ 자체는 분포 밀도이므로 1보다 클 수 있다. 적분한 에너지와 한 방향의 높은 하이라이트 값을 혼동하지 않는다. 보통의 reciprocal 재질 모델에서는 입사·출사 방향을 바꿔도 같은 BRDF가 된다.

Lambert 표면의 diffuse BRDF는 ρ/π다. ρ는 색별 반사율이다. π는 반구에서 cosθ를 적분한 값이므로 이것으로 나누어야 반사 에너지가 ρ가 된다. 실제 한 광원의 기여는 BRDF에 입사광과 N·L을 추가로 곱한다.

BaseColor texture를 화면에서 보이는 최종 색과 같다고 설명하지 않는다. 조명, specular, exposure, tone mapping을 거친 결과가 화면색이다. 금속의 base color를 비금속 diffuse 색과 같은 의미로 무조건 읽지도 않는다.

## G07 Fresnel과 미세면 반사

프레넬은 매질 경계에서 빛이 얼마나 반사되는지를 입사각과 굴절률로 계산한다. 굴절 방향은 Snell 법칙 ηᵢsinθᵢ=ηₜsinθₜ로 결정한다. 비흡수 유전체에서 비편광 반사율은 두 편광 성분 반사율의 평균이다.

$$R_s=\left({\eta_i\cos\theta_i-\eta_t\cos\theta_t\over
\eta_i\cos\theta_i+\eta_t\cos\theta_t}\right)^2,\quad
R_p=\left({\eta_t\cos\theta_i-\eta_i\cos\theta_t\over
\eta_t\cos\theta_i+\eta_i\cos\theta_t}\right)^2,\quad
F={R_s+R_p\over2}$$

정면의 F₀=((ηᵢ−ηₜ)/(ηᵢ+ηₜ))²다. 공기 1, 유리 1.5를 가정하면 F₀=0.04다. 높은 굴절률에서 낮은 굴절률 쪽으로 나갈 때 조건에 따라 전반사가 생긴다. 금속은 흡수를 포함한 복소 굴절률로 다루며 반사색도 파장에 따라 달라질 수 있다. [PBRT 반사와 굴절](https://pbr-book.org/4ed/Reflection_Models/Specular_Reflection_and_Transmission)

실시간에서 자주 쓰는 Schlick 근사는 다음과 같다.

$$F(c)=F_0+(1-F_0)(1-c)^5$$

F₀=0.04인 가정에서 c=1이면 0.04, c=0.5이면 0.07, c=0이면 1이다. 미세면 specular에서는 c=V·H이며, H=normalize(V+L)이다. 단순 rim 효과에서 쓰는 N·V와 항상 같은 값이 아니다. Schlick은 근사이며 프로젝트의 모든 source shader가 이 식을 그대로 쓰는 것도 아니다. [Schlick 논문 정보](https://onlinelibrary.wiley.com/doi/10.1111/1467-8659.1330233), [실시간 PBR의 BRDF 구성](https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf)

### Cook Torrance 계열의 D F G

$$f_s={D(N,H)\,F(V,H)\,G(N,V,L)\over
4\max(N\cdot V,0)\max(N\cdot L,0)}$$

D는 미세면 법선의 분포, F는 각 미세면의 반사율, G는 미세면 사이의 masking·shadowing이다. 장면의 다른 물체가 만드는 shadow map은 G와 다르다. 분모가 0에 가까울 때의 처리와 backface 제거는 구현에서 확인한다.

등방 GGX 분포의 대표식은 다음과 같다.

$$D={\alpha^2\over\pi\left[(N\cdot H)^2(\alpha^2-1)+1\right]^2}$$

현재 MapPBR의 perceptual roughness r에서 α=r²이고 분자에 쓰는 α²는 r⁴다. r, α, α²를 같은 roughness라고 읽으면 하이라이트 폭을 잘못 계산한다. GGX는 다른 분포와 꼬리 형태가 다르며, α는 단순히 반사 에너지를 줄이는 밝기 슬라이더가 아니다. [Walter 등의 미세면 모델과 GGX](https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf)

이론적인 Smith GGX의 한 방향 가시성은 Λ(w)=(sqrt(1+α²tan²θ_w)−1)/2, G₁=1/(1+Λ)로 표현할 수 있다. G₂에는 높이 상관을 고려한 1/(1+Λ(V)+Λ(L))와 독립 가정의 G₁(V)G₁(L) 등 서로 다른 선택이 있다. 저렴한 Schlick 계열의 G 근사를 이 정확식과 같다고 설명하지 않는다. [Heitz의 masking 분석](https://jcgt.org/published/0003/02/03/paper.pdf), [PBRT 미세면 이론](https://pbr-book.org/4ed/Reflection_Models/Roughness_Using_Microfacet_Theory)

일반적인 metalness workflow의 교육용 구성은 F₀=lerp(0.04,baseColor,metallic), k_d=(1−F)(1−metallic), f=k_d·baseColor/π+f_s다. 프로젝트의 원본 specular workflow와 별개의 예다. 에너지 분배를 의식한 구성이라는 설명은 가능하지만, 이 식 하나로 rough microfacet의 모든 다중 산란까지 보존했다고 주장하지 않는다.

## G08 현재 MapPBR의 입력과 직접광 식

실제 조사는 map surface program 3/4를 중심으로 했다. 캐릭터·물·식생·이펙트가 전부 같은 BRDF를 쓴다고 일반화하지 않는다. [MODEL_SURFACE_PARAMETERS](C:/Users/tnest/Desktop/LostArk/Engine/Public/BinaryAsset/ModelAssetData.h:208)는 재질 입력, [RENDER_ENVIRONMENT_STATE](C:/Users/tnest/Desktop/LostArk/Engine/Public/Engine_RenderTypes.h:56)는 장면 선택을 소유한다. [CMapAssetRenderUtils::Bind_Material](C:/Users/tnest/Desktop/LostArk/Client/Private/MapAssetRenderUtils.cpp:1006)이 둘을 조합해 상수와 SRV를 바인딩한다.

### Texture에서 표면 변수 만들기

[EvaluateMapPBRSurface](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_MapMaterialSurface.hlsli:509)는 normal의 RG를 2·RG−1로 풀고 z=sqrt(max(1−x²−y²,0))+10⁻⁵를 복원한다. detail normal, intensity, vertex alpha를 적용한 뒤 정규화하고 TBN으로 world 방향을 만든다. N은 normal map을 적용한 셰이딩 법선, Ng는 메시의 기하 법선이다. 이후 보정에서 둘 다 필요하다.

이 family의 ORM은 R=AO, G=roughness, B=metallic이다. [실제 변환](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_MapMaterialSurface.hlsli:551)의 핵심은 다음과 같다. P(x,p)는 abs(x)<10⁻⁶이면 0, 아니면 pow(abs(x),p)인 프로젝트 함수다.

~~~text
m  = saturate(P(ORM.b · metallicIntensity, metallicPower))
r0 = sourceIndirect ? .045 : minimumRoughness
r  = saturate(max(r0, P(ORM.g · clamp(roughnessIntensity,0,100),
                       roughnessPower)) + diagnosticOffset)
AO = saturate(P(ORM.r · AOIntensity, AOPower))
F0 = lerp((.08 · saturate(specularPBRIntensity)).xxx, albedo, m)
~~~

따라서 비금속이고 specularPBRIntensity=.5이면 F₀=.04다. metallic=1이면 F₀=albedo다. r0 뒤에 diagnostic offset을 더하므로 r≥.045가 항상 보장되는 것은 아니다. 실제 [FLOOR12 재질](C:/Users/tnest/Desktop/LostArk/Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapmaterials.json:7)은 metallicIntensity=0, metallicPower=0, specularPBRIntensity=.5다. P의 zero 처리를 빼고 0⁰을 계산하면 이 입력의 해석부터 달라진다.

### 표준식과 다른 부분을 숨기지 않는다

[Evaluate_MapSourcePBRDirect](C:/Users/tnest/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_Deferred.hlsl:393)는 다음 식을 사용한다. max와 saturate는 HLSL의 성분별 연산이며 r,m,F₀,N,Ng,V,L은 앞 단계에서 온 값이다.

~~~text
H   = (V+L) / sqrt(max(dot(V+L,V+L), 1e−12))
NoH = saturate(N·H)
NoV = min(abs(N·V)+1e−5, 1)
NoL = saturate(N·L)
VoH = saturate(V·H)
g   = min(normalize(Ng)·L+1, 1)
v'  = min(VoH+1−g, 1)
p   = (1−v')^5
F   = F0(1−p) + p·saturate(50F0.g)·(max(F0,1−r)−F0)
D   = r^4 / max(π[NoH²(r^4−1)+1]², 1e−30)
d   = NoL[NoV(1−r²)+r²] + NoV[NoL(1−r²)+r²]
peak = min(.5D/max(d,1e−30), 3/[dot(F,(.3,.59,.11))+.0001])
diffuseWeight        = (1−m)(1−F)NoL
specularContribution = F·peak·NoL·π
~~~

5제곱을 사용한다는 공통점만으로 표준 Schlick과 같다고 말하면 안 된다. Ng, roughness, F₀의 green 성분 보정과 휘도 기반 peak 제한이 있다. d는 이 코드의 visibility 근사이며 G07의 Smith 정확식과 그대로 동일하지 않다. epsilon도 각각 다른 분모를 보호할 뿐, 모든 입력의 NaN/Inf를 검증하는 기능이 아니다.

diffuseWeight에 albedo가 없는 이유는 최종 Combine에서 albedo를 한 번 곱하기 때문이다. 이 source 경로는 diffuse의 1/π와 광량의 π가 상쇄되는 정규화를 사용한다. 여기에 교과서 Lambert의 1/π를 다시 넣으면 중복 보정이다. [Resolve_MapPBRLight](C:/Users/tnest/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_Deferred.hlsl:450)는 직접 diffuse와 specular 모두 light.diffuse.rgb를 입사광으로 사용한다.

## G09 간접광, RNM, SH, 환경맵과 BRDF LUT

직접광은 명시적인 광원 방향을 평가한다. 간접광은 주변 환경과 이미 구워진 조명을 근사한다. 환경맵을 읽는다고 현재 장면에서 반사 광선을 추적한 것은 아니다.

### 일반 이론의 학습 순서

Diffuse 환경 조명은 E(N)=∫Lᵢ(ω)max(N·ω,0)dω다. Lambert 표면은 albedo·E/π를 반환한다. 저주파 irradiance를 소수의 구면 기저 계수로 근사하는 것이 diffuse SH의 목적이다. 0~2차의 SH에는 1+3+5=9개 기저가 있다. 이 결과를 specular의 날카로운 반사까지 표현하는 만능 압축으로 이해하면 안 된다. [Ramamoorthi와 Hanrahan의 환경 조명 연구](https://graphics.stanford.edu/papers/envmap/)

일반적인 실시간 specular IBL은 roughness별로 필터된 environment와 각도·roughness에 대한 BRDF 적분 LUT를 나누어 근사한다. 교육용 split-sum 형태는 prefilteredEnv·(F₀A+B)다. 어떤 값을 미리 적분하고 어떤 가정을 고정했는지가 오차를 결정한다. 현재 프로젝트 LUT의 채널을 이 A/B로 읽어서는 안 된다. [Karis의 실시간 환경 반사 구성](https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf)

### 실제 프로젝트의 서로 다른 표현

| 입력 | 저장한 정보 | 실제 소비와 주의점 |
|---|---|---|
| RNM directional lightmap | tangent 공간의 3개 고정 방향 응답 | ΣCᵢ·saturate(D·bᵢ)². SH9를 3개로 줄인 식이 아님 |
| source SH | 7×float4, 112 bytes의 packed 다항 계수 | source basis·정규화 포함. 일반 SH 상수를 또 곱하지 않음 |
| source 환경 cube | 방향별 RGBM6 색과 mip | RGB×A×6 복원, source 좌표 permutation 적용 |
| source BRDF lookup | R=Fresnel, G=base | 시선과 roughness로 조회, G 분모를 사용하는 추가 보상 |
| 선택적 project cubeDiffuse SH | Lambert E/π를 포함한 계수 | source SH의 π를 재적용하지 않음 |

[MapSourceDirectionalLightmapWeight](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_MapMaterialSurface.hlsli:602)의 basis는 (0,.81649658,.57735027), (−.70710678,−.40824829,.57735027), (.70710678,−.40824829,.57735027)다. Normal map의 방향을 이 basis에 투영하므로 baked 조명도 표면 요철에 반응한다.

[MapSourcePBRSHColor](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_MapMaterialSurface.hlsli:659)는 회전된 q=(bNx+aNz,Ny,aNx−bNz,1)와 2차 항 (qxqy,qyqz,qz²,qxqz), qx²−qy²를 packed 계수와 내적한다. 결과에 source 환경색과 π, offset을 적용한다. a,b는 저장된 sin/cos 성분이다.

### 실제 BRDF LUT와 aliasing 보정

[EvaluateMapSourceIndirectLighting](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_MapMaterialSurface.hlsli:778)의 핵심은 다음과 같다.

~~~text
NoVraw = Ntan·Vtan
Rtan   = reflect(−Vtan,Ntan)
u      = saturate(NoVraw+1−min(Rtan.z+1,1))
rAA    = saturate(r + .3·length((ddx_coarse(NoVraw),ddy_coarse(NoVraw))))
lookup = BRDFTexture(u,rAA).rg
delta  = saturate(50F0.g)·(max(1−rAA,F0)−F0)
Rbrdf  = (lookup.g·F0+lookup.r·delta)
         ·(1+F0·(1/max(lookup.g,1e−6)−1))
cubeLOD = rAA·5
~~~

ddx/ddy는 인접 pixel quad의 공간 변화이며 시간 미분이 아니다. 반사 응답이 급격하게 바뀌는 곳에서 roughness를 키워 aliasing을 완화한다. rAA를 mip 선택에도 쓰지만, 이 함수가 GGX importance sampling으로 cube를 새로 생성하는 것은 아니다. mip 존재만으로 정확한 GGX prefilter 품질까지 증명할 수 없다.

간접 RGB에는 albedo, RNM, hemisphere, source SH, AO 근사, 반사 cube·Rbrdf가 결합된다. MultiBounceAO라는 함수도 광선을 실제 여러 번 추적하는 것이 아니라 AO와 색의 다항 근사다. envSpec은 최종 간접 RGB 안에 이미 포함되어 있다. 별도의 진단 RT에 같은 성분을 저장했다고 다시 합산하면 두 번 더하게 된다.

### 활성화 조건도 구현의 일부다

[CPU binder](C:/Users/tnest/Desktop/LostArk/Client/Private/MapAssetRenderUtils.cpp:1280)의 sourceIndirectEnabled는 material.hasSourceIndirect && scene.bUseSourcePBRIndirect다. program 3/4, cube 입력, legacy 또는 source 선택 조건까지 통과해야 관련 SRV와 flag가 유효하게 연결된다. [CMaterial](C:/Users/tnest/Desktop/LostArk/Engine/Private/Material.cpp:766)은 cube/2D dimension과 source BRDF의 R16G16_UNORM 형식을 검사한다.

native source indirect가 선택되면 [Renderer의 project cubeDiffuse intensity](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:1962)는 0으로 전달된다. 서로 다른 간접광을 무조건 동시에 더하는 구조가 아니다. 현재 저장된 character-select source-rendering/warm-high-key profile은 source indirect=true, cubeDiffuse intensity=0이다. 이는 디스크 설정 확인이며 실행 중 선택을 관측한 결과는 아니다.

## G10 Shadow map, AO와 빛의 감쇠

세 종류의 차폐를 분리한다. Microfacet G는 한 표면의 미세면 가림, shadow map은 광원에서 보이는 장면의 가림, SSAO는 카메라에서 얻은 깊이·법선으로 주변 차폐를 근사한다.

### Shadow map의 비교

광원에서 장면을 렌더한 깊이를 z_map이라 하고, 현재 world 위치를 light view/projection으로 변환한 깊이를 z_receiver라 하면 z_receiver−bias≤z_map인 경우 빛이 도달한다고 판단한다. 유한한 깊이 정밀도와 표본 차이로 자기 그림자 acne가 생길 수 있고, bias를 지나치게 늘리면 그림자가 물체에서 떨어진다.

현재 [Renderer shadow 경로](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:1129)는 조건에 따라 단일 2048×2048 map을 사용하며 정적 내용 재사용 경로가 있다. [생성 형식](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:3150)은 R32_TYPELESS texture, D32_FLOAT DSV, R32_FLOAT SRV다. 동일한 메모리를 쓰는 view 역할의 예다.

[Deferred shadow 평가](C:/Users/tnest/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_Deferred.hlsl:289)는 world/normal bias, light VP 변환, UV와 깊이 비교를 수행한다. PCF는 이진 비교 결과를 주변에서 평균 내며 radius 0/1/2는 1/9/25회 sample이다. 넓게 필터하면 부드러워질 수 있지만 sample 비용과 누광도 늘 수 있다. 현재 단일 map을 CSM이라고 설명하지 않는다. Cascade/PSSM은 다른 거리 구간에 다른 shadow 해상도를 배분하는 확장 학습이다. [GPU Gems 3의 PSSM](https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-10-parallel-split-shadow-maps-programmable-gpus)

### SSAO의 표본과 필터

[현재 SSAO](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:1596)는 half resolution R16_FLOAT RT에서 4/8/12 sample 및 선택적 horizon 경로를 사용한다. bilateral blur는 공간 거리뿐 아니라 depth와 normal 차이를 가중치에 반영한다.

$$AO_{filtered}(p)={\sum_q w_s(p,q)w_d(p,q)w_n(p,q)AO(q)\over
\sum_q w_s(p,q)w_d(p,q)w_n(p,q)}$$

화면의 다른 물체 경계를 단순 평균으로 번지게 하지 않기 위한 선택이다. 분모가 부족하면 중심 값을 사용한다. 화면 밖이나 뒤에 가려진 geometry는 depth에 없으므로 이 자료만으로 정확한 전역 차폐를 알 수 없다. SSAO를 켰다고 광원별 그림자가 생기거나 물리적인 다중 반사가 해결되는 것은 아니다.

점광원의 이상적인 거리 감쇠는 power가 구면 면적 4πr²로 퍼지는 데서 1/r²가 나온다. 실시간 구현의 유한 range·cutoff·예술적 감쇠는 별개의 함수다. 방향광은 멀리 있는 광원의 평행한 방향을 근사하고, spot은 거리 항과 cone 각도 항을 함께 쓴다. 프로젝트의 광량이 물리 단위로 보정되었다고 이번에 확인하지 않았다.

## G11 선형 색, HDR, Bloom과 Tone mapping

빛의 덧셈과 BRDF 곱셈은 선형 공간에서 해야 한다. 일반 색 이미지의 sRGB 값은 선형 radiance 자체가 아니다. sRGB texture view는 sampling 때 decode하지만 normal·ORM·depth·lookup 같은 수치 데이터에 같은 변환을 적용하면 값의 의미가 깨진다.

[Material texture loader](C:/Users/tnest/Desktop/LostArk/Engine/Private/Material.cpp:254)는 일반 diffuse/emissive와 explicit linear를 구분하고, source 재질은 diffuseSRGB/reflectionSRGB/ormSRGB 입력도 따른다. shared texture cache key는 device·정규화 경로·sRGB·forceLinear·파일 크기·수정 시각을 구분한다. 같은 경로만 보고 다른 색 공간의 view를 합칠 수 없는 이유다.

### 현재 합성식

[PS_MAIN_COMBINED](C:/Users/tnest/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_Deferred.hlsl:1310)의 map PBR 흐름을 줄이면 다음과 같다.

~~~text
lit = albedo·shade + specular
if marker3:
    lit += (RT7indirect·dynamicBakedShadow + optionalCubeDiffuse)·SSAO
lit = heightFog(lit)
SceneHDR = lit + emissive
~~~

RT7에는 이미 albedo가 반영된 간접 RGB가 들어 있다. 다시 albedo를 곱하지 않는다. emissive는 이 branch의 SSAO나 dynamicBakedShadow를 받지 않고 fog 뒤에 더해진다. SceneHDR는 RGBA16_FLOAT라 1보다 큰 색을 보관할 수 있다.

Bloom은 밝은 영역이 주변으로 퍼지는 화면 효과다. 현재 [Bloom 경로](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:2796)는 half resolution R11G11B10_FLOAT RT와 가로·세로 9탭 blur다. 밝은 성분은 앞선 pass에서 SceneBloom으로 분리되며 extract에서 처음 threshold를 적용하는 구조가 아니다. 여러 mip의 pyramid를 사용한다고도 설명하지 않는다.

Exposure는 전체 밝기 scale, roughness는 반사 lobe의 폭이다. 둘 다 화면을 바꾸지만 같은 조절값이 아니다. Tone mapping은 넓은 HDR 범위를 표시 범위로 옮긴다. 프로젝트 [최종 함수](C:/Users/tnest/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_Deferred.hlsl:1883)는 source postprocess 또는 Hable을 선택한다.

Hable 분기의 실제 rational 함수는 다음과 같다.

$$T(x)={x(.15x+.05)+.004\over x(.15x+.50)+.06}-{.02\over.30}$$
$$C_{display}=\operatorname{pow}\left(
\operatorname{saturate}\left({T((scene+bloom\cdot tint)\max(exposure,.01))\over
\max(T(\max(whitePoint,1)),10^{-5})}\right),\ {1\over\max(gamma,1)}
\right)$$

이는 power gamma 근사이며 정확한 piecewise sRGB 함수와 같은 식이 아니다. source 분기는 별도의 toe/shoulder rational 곡선과 grading LUT를 사용한다. 현재 source profile에 대해 Hable만 작동한다고 설명하면 틀린다.

Source color grading LUT는 16³ 색을 256×16 atlas로 저장하고 R/G는 slice 안, B는 인접 slice 사이에서 보간한다. 이 LUT의 입력은 색이다. G09의 BRDF LUT 입력은 각도와 roughness다.

FXAA는 tone 이후 UI 전에 선택적으로 작동한다. 현재 구현은 enable뿐 아니라 subpixel 값이 0 이하일 때도 early return한다. 저장 profile에서 flag=true만 보고 유효한 AA 실행을 확정하지 않는다. 특히 팀장의 Mario FXAA OFF와 Bern fog OFF는 그대로 보존한다.

## G12 Forward 투명도, 굴절과 화면 기반 효과

일반 alpha blend는 C_out=αC_source+(1−α)C_dest다. A over B와 B over A는 보통 다르므로 그리는 순서가 결과를 바꾼다. premultiplied alpha는 색에 이미 alpha가 곱해진 입력을 사용하므로 blend state도 이에 맞아야 한다. alpha test의 clip은 픽셀을 버리는 것이며 반투명 혼합과 다르다.

Deferred G-buffer는 한 pixel에 선택된 표면 정보를 저장하는 데 적합하다. 여러 투명층을 한 묶음의 표면으로 덮어쓰면 순서와 뒤쪽 색을 잃는다. 그래서 현재 [Render_Blend](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:2335)는 별도 forward 목록을 priority와 카메라 거리 기준으로 stable_sort한 뒤 렌더한다. 이는 객체 단위 정렬이며 모든 교차 투명 면을 정확하게 해결하는 OIT는 아니다.

Blend object 수 B에 대해 이 정렬은 통상 O(B log B), scratch 보관은 O(B)다. 이것은 알고리즘 비용이며 현재 frame의 밀리초 측정값이 아니다. 투명 입자가 많이 겹치면 객체 수보다 덮는 pixel 수와 texture sample이 병목일 수 있다.

SceneColor snapshot은 현재 색을 읽어 굴절·왜곡을 계산할 때 사용한다. 같은 texture subresource를 RTV와 SRV로 동시에 읽고 쓰지 않도록 snapshot 또는 ping-pong이 필요하다. [ScreenPosts](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:2582)는 scene/bloom 쌍을 교대한다.

[SSGI/SSR 경로](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:2048)는 선택 기능이며 현재 shader에서 map PBR marker3를 대상으로 한다. SSGI의 half-resolution gather는 선택 경로이고 SSR와 최종 resolve는 full resolution이다. 이 경로는 frame history·화면 밖 geometry를 갖지 않는다. SSR의 반사 ray 탐색을 DXR 하드웨어 ray tracing이라고 부르지 않는다. full resolution texture 두 장 복사도 있어, 효과 비용은 ray step만으로 설명할 수 없다.

## G13 Sampling, mip과 화면 비용을 이해하기

Texture는 연속 함수의 유한 표본이다. minification에서 많은 texel이 한 pixel에 들어오면 고주파 패턴이 깜빡이거나 모아레가 생긴다. bilinear는 같은 mip의 주변 표본, trilinear는 mip 사이까지 보간한다. anisotropic filtering은 비스듬한 표면의 길쭉한 footprint를 더 잘 근사하기 위한 선택이다.

Mip이 가로·세로 절반으로 계속 줄어드는 2D texture라면 전체 texel 수는 WH(1+1/4+1/16+…)=4WH/3에 수렴한다. 압축·작은 level의 block 정렬·실제 allocation은 이 이론식과 다르다. Cube는 같은 크기의 face 6개다. G-buffer의 현재 RT는 mip 1개이므로 그 메모리에 4/3을 무조건 곱하지 않는다.

일반 texture sample의 derivative는 screen-space footprint와 LOD 선택에 관여한다. 명시적 SampleLevel은 LOD를 직접 지정한다. normal map을 평균낸 값은 길이가 줄어들 수 있고, 단순 재정규화만으로 specular aliasing이 모두 해결되지는 않는다. G09의 rAA는 이 문제를 줄이기 위한 프로젝트의 별도 보정이다.

| 비용을 설명하는 변수 | 어떤 작업이 늘어나는가 | 확인할 기준 |
|---|---|---|
| 화면 W×H | full-screen pass의 sample·write | 같은 해상도에서 비교 |
| overdraw k | 같은 pixel을 반복 shading/blend | 투명 효과 면적과 겹침 |
| 광원 L | 광원 draw와 표면 평가 | culling 뒤의 실제 batch |
| shadow filter sample S | 깊이 조회·비교 | 1/9/25회 등 선택값 |
| triangle/vertex V | transform·skinning·raster setup | draw call 수와 따로 기록 |
| format bytes B | RT 저장·읽기량 | RT 크기와 pass 횟수 |

모든 pass가 O(WHL)인 것은 아니다. 현재 광원은 동종 연속 batch와 culling을 사용한다. 복잡도는 모델이고 실제 GPU 성능은 캐시, 대역폭, 연산, latency, occupancy, 동기화와 겹쳐 결정된다.

## G14 실제 카메라, 법선, 애니메이션, Picking과 LOD

### 카메라 상태를 최종 행렬로 만드는 함수

[CTransform](C:/Users/tnest/Desktop/LostArk/Engine/Public/Transform.h:29)은 world의 행을 right/up/look/position으로 사용한다. [Camera::Update_Pipeline](C:/Users/tnest/Desktop/LostArk/Engine/Private/Camera.cpp:218)은 camera world의 역행렬을 View로, FOV를 radian으로 바꾸어 PerspectiveFovLH를 Projection으로 전달한다. [PipeLine](C:/Users/tnest/Desktop/LostArk/Engine/Private/PipeLine.cpp:59)은 역행렬과 camera position을 갱신한다.

카메라가 target을 부드럽게 따라갈 때 쓰는 가중치는 α=1−exp(−kΔt)다. k의 단위는 s⁻¹, Δt는 초다. 두 값을 곱한 지수는 무차원이어야 한다. 1차 응답의 정확한 시간 가중치라 고정된 lerp 상수보다 frame 간격 변화에 일관된 의미를 갖는다. target이 움직이는 동안의 전체 운동까지 완벽하게 frame-rate 독립이라는 뜻은 아니다.

[실제 FreeCamera](C:/Users/tnest/Desktop/LostArk/Client/Private/Camera_Free.cpp)는 target의 weak_ptr를 lock해서 사용하며 최종 target 반영·shake 후 pipeline을 갱신한다. 첫 frame·teleport·비양수 response에서는 즉시 맞추는 경로가 있다. k=18, Δt=1/60이면 α≈.25918이다. 이 숫자는 예제 계산이며 frame 측정이 아니다.

### 법선은 왜 역전치인가

접선 t와 법선 n은 t·n=0이어야 한다. row vector 위치·접선 변환이 t'=tA일 때 n'=nA⁻ᵀ이면

$$t'n'^T=tA(nA^{-T})^T=tAA^{-1}n^T=tn^T=0$$

이것이 비균일 스케일에서 법선에 역전치를 쓰는 이유다. t=(1,1,0), n=(1,−1,0), A=diag(2,1,1)로 계산하면 변환된 접선은 (2,1,0)이다. 법선에 A를 그대로 곱한 (2,−1,0)은 내적 3으로 수직이 아니지만, A⁻ᵀ을 곱한 (.5,−1,0)은 내적 0이다.

[MapAssetObject](C:/Users/tnest/Desktop/LostArk/Client/Private/MapAssetObject.cpp:744)와 [instance 구성](C:/Users/tnest/Desktop/LostArk/Client/Private/MapPlacementRuntime.cpp:1737)은 이 정보를 준비한다. [정적 메시 VS](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl:115)는 normal에 inverse transpose를, tangent에는 world 변환과 Gram–Schmidt를 적용한다.

반면 [animated VS](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl:151)의 normal은 bone blend 후 World를 사용한다. 정적 경로의 보장을 모든 skinned mesh의 비균일 스케일까지 확장하지 않는다.

### Skeleton, inverse bind와 Quaternion

Skeleton은 부모 index와 local transform을 갖는 계층이다. 현재 [Bone의 combined 계산](C:/Users/tnest/Desktop/LostArk/Engine/Private/Bone.cpp:76)은 childLocal·parentCombined이며 root에는 preTransform이 연결된다. [Mesh palette](C:/Users/tnest/Desktop/LostArk/Engine/Private/Mesh.cpp:340)는 inverseBind·currentCombined다.

행벡터 기준 skinning은 대략 p'=Σwᵢ(p·inverseBindᵢ·currentBoneᵢ)로 읽는다. inverseBind는 bind pose의 정점을 bone 기준으로 되돌리고 currentBone이 현재 pose로 옮긴다. 현재 shader는 정점당 4개 bone index/weight를 사용하고 palette 선언은 512개다. shader가 네 번째 weight를 1−합으로 다시 만들거나 전체 weight를 자동 정규화한다고 가정하지 않는다. CPU bone 계산 O(B), VS blend O(4V)는 연산량 모델이다.

Quaternion은 q=(u sin(θ/2),cos(θ/2))로 단위 축 u와 회전을 표현한다. 단위 q에서 벡터를 순허수 quaternion p=(v,0)로 놓으면 회전은 qpq⁻¹로 표현할 수 있다. 이것은 column/row 행렬 관례와 독립적인 개념식이며 라이브러리의 곱 순서는 실제 API 계약에 맞춘다. q와 −q는 같은 회전이므로 보간 전 내적 부호를 고려해야 짧은 경로를 고를 수 있다.

두 단위 quaternion 사이의 각 Ω에 대한 slerp는 다음과 같다.

$$q(t)={\sin((1-t)\Omega)\over\sin\Omega}q_0+
{\sin(t\Omega)\over\sin\Omega}q_1$$

작은 Ω에서는 분모가 작아져 normalized lerp 같은 안정적인 처리가 필요하다. [Bone의 blend](C:/Users/tnest/Desktop/LostArk/Engine/Private/Bone.cpp:56)는 SRT 분해 후 scale/translation lerp, rotation QuaternionSlerp를 사용하며 분해 실패 때 기존 상태를 보존한다. 행렬 16개 성분을 무조건 선형 보간하면 회전의 직교성이 깨질 수 있다.

### Picking에는 서로 다른 두 경로가 있다

[CPicking](C:/Users/tnest/Desktop/LostArk/Engine/Private/Picking.cpp:26)은 Target_PickPos에서 한 pixel을 읽는 1×1 RGBA32F staging texture 두 개를 사용한다. [Renderer](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:3098)는 opaque 결과를 보존하면서 forward PICKING 객체도 같은 RT에 추가한다. 동기 Map(READ)은 GPU를 기다릴 수 있다. 비동기 요청은 request ID와 DONOTWAIT를 사용하며 아직 준비되지 않았으면 S_FALSE로 보류한다.

반면 [PlayerController의 ray 구성](C:/Users/tnest/Desktop/LostArk/Client/Private/PlayerController.cpp:2147)은 depth0/1 위치를 Unproject해 near/far를 구하고 origin=near, direction=far−near로 만든다. 이 direction은 정규화되지 않았다. 지면 y=h 교차의 t=(h−origin.y)/direction.y는 ray parameter이며 자동으로 미터 단위 거리가 되지 않는다. ray 방향이 평면과 평행하거나 t<0이면 별도 실패 처리가 필요하다.

### Culling과 LOD/HLOD

평면 n·p+d=0에서 단위 n을 쓰면 값이 signed distance다. 구 중심 c, 반지름 r를 평면과 비교해 시야 밖을 빠르게 제거할 수 있다. 현재 [Frustum](C:/Users/tnest/Desktop/LostArk/Engine/Private/Frustum.cpp:41)은 VP에서 바깥쪽 법선을 추출·정규화하고 sphere 검사에서 distance≥r이면 제외한다. 이 구현의 equality 경계도 기억한다.

LOD는 가까운 물체를 작게 만드는 기능이 아니라, 화면 오차를 허용하며 기하의 세부 수준을 바꾸는 선택이다. HLOD는 여러 source draw를 묶은 표현을 선택한다. 둘은 서로 다른 묶음 단위다.

[StaticMeshLod](C:/Users/tnest/Desktop/LostArk/Engine/Private/StaticMeshLod.cpp:391)의 선택 기준은 view-space nearestZ=d, local geometric error를 scale한 e에 대해 대략 다음과 같다.

~~~text
e = localError·maxScale + additionalError
s = d−e
pixelErrorX = projectionPixels.x·e·(1+largestAbsX/d)/s
pixelErrorY = projectionPixels.y·e·(1+largestAbsY/d)/s
accept = s>near && max(pixelErrorX,pixelErrorY) <= maxErrorPixels·.9
~~~

거리만의 threshold보다 object 크기·화면 투영·scale·오차를 함께 반영한다. [MapStaticChunkObject](C:/Users/tnest/Desktop/LostArk/Client/Private/MapStaticChunkObject.cpp:229)는 visibility, source LOD, projected error, 실제 index 감소와 replacement draw 수를 고려한다. 코드에 선택 경로가 존재한다는 확인과 현재 모든 맵에서 활성화되었다는 확인은 다르다.

## G15 소유권, 메모리와 실제 프로파일러

| 정보의 수명 | 실제 자료구조와 이유 | 혼동하면 생기는 문제 |
|---|---|---|
| 재질·texture | CMaterial, shared SRV, ComPtr | view를 공유하면서 해석·device·수명을 무시 |
| render target | CRenderTarget의 texture/RTV/SRV | view와 texture allocation을 중복 계산 |
| frame의 source character 재질 | thread_local vector의 shared_ptr, pointer lookup map | lighting 전에 재질이 사라지거나 row를 영구 ID로 저장 |
| GPU 계측 요청 | FGpuQuerySlot ring, frame number와 validity | 다른 frame의 결과를 한 frame처럼 더함 |
| 저장 요청 | FProfilerCaptureSnapshot 복사 | 수집 중인 배열을 저장 thread가 동시에 순회 |

[CMaterial의 frame row](C:/Users/tnest/Desktop/LostArk/Engine/Private/Material.cpp:955)는 Deferred 이후에도 원본 material 입력을 찾기 위해 사용한다. row는 frame마다 초기화하는 조회값이며 asset ID가 아니다. R32_FLOAT로 정확하게 표현되는 정수 범위 계약도 있다.

[CShader::Bind_RawValue](C:/Users/tnest/Desktop/LostArk/Engine/Private/Shader.cpp:726)는 같은 bytes인지 검사해 중복 SetRaw 호출을 피하는 경로가 있다. shared Effect를 다른 material이 덮을 수 있으므로 “한 번 설정했으니 이후 material에서도 영구히 유효하다”고 가정하면 안 된다. 바인딩 호출 수 감소와 전체 frame 시간 개선은 별도로 측정한다.

### CPU 시간과 GPU 시간

[Profiler의 CPU scope](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:299)는 QPC 구간을 ms로 변환한다. CPU scope가 Draw 제출 시간을 잰다면 GPU shader 실행 시간 자체가 아니다.

[GPU query](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:1073)는 timestamp/disjoint를 사용한다.

$$t_{GPU,ms}={(timestamp_{end}-timestamp_{begin})\times1000\over Frequency}$$

Disjoint, Frequency=0, 역전 timestamp는 무효다. [Resolve_GpuFrames](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:1183)는 DONOTFLUSH로 조회하고 준비 전이면 보류한다. [Profiler.h](C:/Users/tnest/Desktop/LostArk/Engine/Public/Profiler.h:455)에 8-slot ring과 최소 4 poll-frame 지연 계약이 있다.

LiveStats의 CPU는 최신 frame이고 GPU는 최근 유효하게 완료된 frame일 수 있다. 두 수치를 더해 frame 시간이나 FPS로 바꾸면 잘못된다. CPU/GPU는 겹쳐 실행하며 Present·동기화 등도 영향을 준다.

### F7 수집과 F1 비교

[F7](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_Profiler.cpp:34)은 창을 토글한다. [Begin_Capture](C:/Users/tnest/Desktop/LostArk/Client/Private/ProfilerTool.cpp:656)가 history를 초기화하고 수집을 켜며, [Request_Save](C:/Users/tnest/Desktop/LostArk/Client/Private/ProfilerTool.cpp:724)는 snapshot을 복사한다. [ProfilerCaptureIO](C:/Users/tnest/Desktop/LostArk/Client/Private/ProfilerCaptureIO.cpp:827)는 별도 thread로 JSON을 저장한다. 저장 I/O를 옮겨도 snapshot 복사 자체의 CPU 비용은 남는다.

[RenderingBenchmark](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingBenchmark.cpp:944)는 A1–B1–B2–A2 순서, warmup 제외, GPU 지연 tail 처리와 raw evidence 보존 경로를 갖는다. 동일 장면·카메라·해상도·설정·workload를 맞추고 CPU/GPU validity를 확인한 뒤 수치를 추가한다.

메모리 sample은 Capture 중 최대 초당 1회 OS 정보를 읽기 때문에 짧은 peak를 놓칠 수 있다. Vtx는 GPU VS 실행 수, Idx는 계측한 제출 참조 수다. unique vertex 수나 실제 화면에 보인 삼각형 수와 같지 않다. 84 B/pixel의 G-buffer 이론량, DXGI 메모리 관측, process working set도 서로 다른 값이다.

## G16 Frank Luna DX11 책을 바탕으로 한 전체 학습 지도

사용자가 말한 DX9·DX11·DX12 책은 Frank D. Luna의 계열로 보이며, [출판사 서지 정보](https://www.degruyterbrill.com/de/document/doi/10.1515/9781937585952/html)에서 DX11 원서의 저자도 확인했다. 아래 장 번호와 주제는 [저자 공개 DX11 목차](https://www.d3dcoder.net/d3d11.htm)를 기준으로 한국어로 요약했다. Fog는 이 목차의 9장 blending에서 다룬다. 한국어 판본의 정확한 절 번호·페이지는 확인하지 않았으며, 아래 설명과 증명은 책 본문을 재현한 것이 아니라 프로젝트에 맞춰 작성한 학습 자료다.

| 장 | 주제 | 여기서 이해할 질문 | 현재 프로젝트와 연결 |
|---|---|---|---|
| 1 | 벡터 | 내적·외적·정사영은 무엇을 계산하는가 | N·L, 카메라 basis, ray |
| 2 | 행렬 | 역행렬·전치·곱 순서는 왜 필요한가 | W/V/P, inverse transpose |
| 3 | 좌표 변환 | 같은 점을 다른 basis로 어떻게 표현하는가 | local/world/view/clip |
| 4 | 장치 초기화 | device/context/swap chain의 책임은 무엇인가 | Graphic_Device, RTV/DSV |
| 5 | 렌더링 단계 | 정점이 pixel로 바뀌는 사이에 무엇을 잃는가 | G03과 G18 |
| 6 | Draw와 shader | 상태·buffer·pass를 어떤 순서로 연결하는가 | Shader, VIBuffer |
| 7 | 광원과 재질 | 빛 방향과 재질 응답은 어떻게 결합하는가 | Light_Manager, G05~G10 |
| 8 | Texture | UV·sample·filter·색 공간은 어떻게 다른가 | Material, TBN, mip |
| 9 | 혼합·안개 | 합성 순서와 거리 감쇠는 왜 필요한가 | Blend, height fog |
| 10 | Stencil | pixel별 mask로 어떤 영역을 통과시키는가 | 현재 depth/stencil state 조사 |
| 11 | Geometry Shader | primitive 단위에서 무엇을 생성할 수 있는가 | 현재 hover silhouette GS |
| 12 | Compute Shader | thread group·SRV·UAV로 무엇을 병렬 계산하는가 | 가시성, Hi-Z |
| 13 | Tessellation | patch를 언제 얼마나 세분화하는가 | 개념 학습과 현재 적용 여부 분리 |
| 14 | 조작 가능한 카메라 | 위치·회전·입력·추적을 어떻게 분리하는가 | Camera, Camera_Free |
| 15 | Instance와 시야 제거 | 같은 geometry를 어떻게 재사용하고 제외하는가 | MapPlacementRuntime, Frustum |
| 16 | Picking | 화면점에서 world ray·물체를 어떻게 찾는가 | CPicking, PlayerController |
| 17 | Cube 환경 | 3D 방향으로 어떤 색을 조회하는가 | source cube, IBL |
| 18 | 표면 법선·변위 | shading normal과 실제 geometry 변화는 어떻게 다른가 | TBN, normal map |
| 19 | 지형 | 높이장과 일반 mesh 배치의 구조는 어떻게 다른가 | 추출 맵과 heightfield를 구분 |
| 20 | 입자와 stream-out | 상태 갱신과 그림 생성을 어디서 수행하는가 | 현재 instanced particle와 대조 |
| 21 | 그림자 | light-space depth 비교는 무엇을 근사하는가 | single shadow map, PCF |
| 22 | 주변 차폐 | depth/normal에서 어떤 정보를 추정하는가 | SSAO, bilateral blur |
| 23 | Mesh | 정점·index·subset·material을 어떻게 보관하는가 | CMesh, CModel, CMaterial |
| 24 | Quaternion | 회전을 어떻게 안정적으로 표현·보간하는가 | Bone SRT/Slerp |
| 25 | 캐릭터 애니메이션 | 계층·keyframe·inverse bind는 어떻게 결합하는가 | Bone, palette, animated VS |
| 부록 | Win32·HLSL·해석기하·연습 | API 문법과 기반 수학의 빠진 부분은 무엇인가 | 공식 문서와 작은 계산 예제 |

이 순서를 모두 끝낸 뒤 기술소개서를 시작하는 방식으로 바꾸지 않는다. 현재 16~19페이지를 설명하는 핵심은 1~9, 14~18, 21~22와 PBR 확장이다. GS·CS·애니메이션은 해당 구현을 설명할 차례에 실제 코드까지 내려간다. 각 항목은 다음 순서로 학습한다.

1. 만들려는 화면 현상과 입력 정보를 말한다.
2. 수식의 변수·단위·공간을 정의하고 작은 예제로 계산한다.
3. C++의 소유자·자료구조·실제 HLSL 소비자를 연결한다.
4. 한 조건을 바꿨을 때의 결과를 예상한다.
5. 기술소개서 문장으로 압축하고, 실제 확인한 범위를 표시한다.

책의 예제가 current project의 구현이라는 뜻은 아니다. 예를 들어 책에서 GS billboard를 배울 수 있어도 현재 파티클이 GS를 사용한다고 주장하지 않는다. 반대로 Deferred·native PBR·SH·GPU 계측의 세부는 이 책의 목차만으로 충분하지 않아 공식 DX11 문서와 아래 원전으로 확장한다.

## G17 기초 수학을 공식 암기에서 증명으로 옮기기

### 내적과 코사인

삼각형의 두 변 a,b 사이 각을 θ라고 하면 코사인 법칙은 |a−b|²=|a|²+|b|²−2|a||b|cosθ다. 좌표로 전개하면 |a−b|²=|a|²+|b|²−2Σaᵢbᵢ이므로 a·b=|a||b|cosθ가 나온다. 단위 벡터이면 내적 자체가 cosθ다.

이 증명에서 조명으로 넘어가면 같은 빛이 경사진 면에 퍼질수록 단위 면적당 에너지가 줄어드는 cos 항이 연결된다. 모든 내적을 밝기라고 읽는 것이 아니라, 단위 방향과 면적 정사영이라는 조건을 확인한다.

### 좌표 변경과 역행렬

물체의 local 좌표 p에 W를 적용해 world 좌표 q=pW를 만들었다면, W가 가역일 때 p=qW⁻¹이다. 카메라를 기준으로 다시 표현할 때도 같은 원리로 V=C⁻¹을 사용한다. 스케일 한 축이 0이면 행렬이 특이해 역행렬이 없으므로 모든 TRS가 가역이라는 가정은 틀리다.

### 원근 투영의 x/z

핀홀과 투영 평면의 닮은 삼각형에서 x_projected/focal=x_view/z_view가 나온다. 따라서 거리가 두 배면 같은 물체의 투영 크기는 절반이 된다. 행렬 P가 clip.w=z_view를 만들고, 이후 x_clip/w_clip로 나누는 이유가 이것이다. 행렬 곱만으로 원근 나눗셈까지 끝난 것은 아니다.

### Lambert의 π

상수 BRDF f=ρ/π에서 표면 반구의 cos 가중 적분은

$$\int_0^{2\pi}\int_0^{\pi/2}\cos\theta\sin\theta\,d\theta\,d\phi
=2\pi\left[\tfrac12\sin^2\theta\right]_0^{\pi/2}=\pi$$

따라서 반사 비율은 f·π=ρ가 된다. 입체각 요소 dω=sinθdθdφ를 빠뜨리면 잘못된 정규화가 나온다. 이 교육용 증명과 G08 source의 별도 광량 정규화를 구분한다.

### 지수 추적과 지수 안개

고정 목표 x*를 향해 dx/dt=k(x*−x)로 움직이면 오차 e=x−x*는 de/dt=−ke다. 해는 e(t+Δt)=e(t)exp(−kΔt)이며, 따라서 x_new=x+(1−exp(−kΔt))(x*−x)다.

빛이 균일 매질을 지나며 dL/ds=−σₜL로 줄어드는 경우에도 같은 미분방정식 구조로 T(d)=exp(−σₜd)가 나온다. σₜ는 거리의 역수, d는 거리다. 이것은 흡수와 바깥 방향 산란에 의한 투과율이며 모든 방향에서 들어오는 산란까지 푼 완전한 volume rendering은 아니다. [PBRT Transmittance](https://www.pbr-book.org/4ed/Volume_Scattering/Transmittance)

### 회전 행렬의 직교성

회전은 길이와 각도를 보존한다. 기저 벡터가 서로 직교하고 길이가 1이면 RᵀR=I이므로 R⁻¹=Rᵀ다. 이 성질은 순수 회전에서 성립하며 scale·shear까지 포함된 일반 world 행렬에 무조건 적용되지 않는다. 그래서 카메라 basis 정규화와 animated transform의 SRT 분해가 필요하다.

## G18 Geometry Shader, Compute, Stencil과 실제 Fog

### GS는 primitive를 입력받아 새 primitive를 낸다

VS가 정점 단위를 처리하는 데 비해 GS는 point/line/triangle 같은 primitive와 그 정점들을 함께 받는다. 출력 stream으로 정점을 내보내거나 내보내지 않을 수 있다. 출력 상한은 maxvertexcount에 선언한다. 이 때문에 billboard 생성·외곽선 확장 등 서로 다른 용도로 사용할 수 있지만, GS 사용 자체가 instancing보다 빠르다는 뜻은 아니다. [Microsoft Geometry Shader 단계](https://learn.microsoft.com/en-us/windows/win32/direct3d11/geometry-shader-stage)

현재 실제 적용은 [GS_COMBAT_HOVER_SILHOUETTE](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl:900)다.

~~~text
입력 triangle VS_OUT[3]
  → 8개 화면 방향마다 triangle의 3정점을 복사
  → clip.xy += direction · ndcWidth · clip.w
  → 3개 Append 후 RestartStrip
출력 8 triangle, 최대 24 vertices
~~~

[CPU의 ndcWidth](C:/Users/tnest/Desktop/LostArk/Client/Private/DeferredMaterialRenderUtils.cpp:310)는 (8/viewportWidth,8/viewportHeight)다. w를 곱한 offset은 perspective divide 뒤에 일정한 NDC 이동량으로 남는다.

$$x'_{ndc}={x_{clip}+\delta_xw_{clip}\over w_{clip}}=x_{ndc}+\delta_x$$

viewport 변환까지 가면 x 방향 이동량은 (8/W)·W/2=4 pixel이다. 이는 식으로 계산한 복사본 이동량이지 최종 화면의 윤곽선 두께 실측은 아니다. 입력 triangle 수가 T면 이 pass에서 최대 8T개의 triangle을 내보내므로, 화면에서 좋아 보이는 효과와 geometry 증폭 비용을 함께 설명한다.

### Stencil은 깊이와 다른 pixel별 mask다

[NPC silhouette 호출](C:/Users/tnest/Desktop/LostArk/Client/Private/Npc.cpp:1532)은 몸체·모자·무기를 대상으로 MASK→OUTLINE→CLEAR를 수행한다. 상위 stencil bit 0x80에 원래 영역을 기록하고, 확장된 geometry는 해당 bit가 같지 않은 영역에 그린다. 마지막에 자신이 사용한 bit를 돌려주며 아래 7bit와 scene depth를 보존한다. 깊이 test는 앞뒤 관계를, stencil test는 미리 기록한 정수 mask 조건을 판정한다.

[Source light mask](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:1895)는 별도 DSV를 사용한다. source character marker5인 pixel을 stencil1로 표시한 뒤 관련 light pass를 제한한다. scene outline의 stencil을 같은 방식으로 덮어쓰는 구조가 아니다. mask 준비가 실패하면 mask 없는 경로로 돌아간다.

### 현재 파티클은 GS billboard와 다르다

[Particle renderer](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_Particles.cpp:502)는 CPU가 particle의 World·color·dynamicParameter·SubUV·life를 준비해 instance 배열에 넣는다. [CVIBuffer_ParticleRect](C:/Users/tnest/Desktop/LostArk/Engine/Private/VIBuffer_ParticleRect.cpp:31)는 공통 quad 4정점/6index와 dynamic instance buffer를 연결해 DrawIndexedInstanced(6,N,…)를 호출한다.

공통 billboard의 개념식은 W=Scale·RotationZ(roll)·CameraWorld_withoutTranslation·Translation이다. 실제 source particle는 정렬 방식과 scale·shear·source profile을 더 처리하므로 모든 particle이 이 단순식 하나만 쓰지는 않는다. [GeometryHelpers](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_GeometryHelpers.cpp:81)가 이 정보를 준비하고 [family VS](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_EffectParticleFamilyCarrier.hlsli:87)가 WVP와 SubUV를 소비한다. 확인한 pass의 GS는 NULL이다.

따라서 billboard는 카메라를 향하는 표현 방식이고, point를 GS로 늘릴지, 준비한 quad를 instancing할지는 별도의 구현 선택이다.

### CS의 thread group과 현재 가시성 판정

CS는 graphics draw와 별도로 Dispatch한다. SRV로 입력을 읽고 UAV로 결과를 쓸 수 있으며 thread group 크기와 전체 group 수를 구분한다. group 내부 barrier가 모든 group 사이의 전역 동기화를 보장하는 것은 아니다. [Microsoft Compute Shader 개요](https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-shader)

[CGpuSceneVisibility::Dispatch](C:/Users/tnest/Desktop/LostArk/Engine/Private/GpuSceneVisibility.cpp:264)는 SRV 5개/UAV 2개와 상수를 바인딩하고 ceil(instanceCount/64)개 group을 호출한다. [Shader_GpuSceneVisibility](C:/Users/tnest/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_GpuSceneVisibility.hlsl:30)는 numthreads(64,1,1)이며 각 thread가 instance 하나의 active·frustum·거리·pixel error·선택 Hi-Z 조건을 검사한다.

최대 5개 LOD bin 중 하나를 선택하고 InterlockedAdd로 output slot과 draw argument instance count를 확보한다. 여기의 자료구조는 StructuredBuffer<SourceInstance>, ByteAddressBuffer, RWByteAddressBuffer다. 1,000개 instance면 16group×64=1,024thread를 호출하며 마지막 24개는 count guard에서 반환한다. 이 계산은 설명용이며 속도 측정이 아니다.

이 CS의 View/Projection은 row_major로 선언되어 있고 CPU가 UpdateSubresource로 올린다. G02의 Effects11 SetMatrix 경계를 모든 shader 업로드에 일반화하면 안 된다.

### Hi-Z는 현재 maximum depth pyramid다

[Hi-Z 생성](C:/Users/tnest/Desktop/LostArk/Engine/Private/GpuSceneVisibility.cpp:376)은 depth 출력 바인딩을 해제하고 이를 SRV로 읽은 뒤 mip마다 CS를 실행한다. Copy/Reduce group은 8×8이다. [Reduce shader](C:/Users/tnest/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_GpuHiZReduce.hlsl:11)는 2×2 깊이의 최댓값을 기록하고 [Copy shader](C:/Users/tnest/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_GpuHiZCopy.hlsl:11)는 padding을 far depth1로 채운다.

큰 화면 footprint를 낮은 mip에서 보수적으로 검사하기 위한 구조다. 현재 depth convention에 맞춘 max이므로 다른 reverse-Z/min 예제를 그대로 적용하지 않는다. 가림 판정의 실패가 보이는 geometry를 없애지 않도록 bound·padding·깊이 부등식의 방향을 함께 검증해야 한다.

독립 Shader_MeshLod 파일도 있지만 이번 검색에서 해당 이름의 직접 CPU load/Dispatch 소비자는 찾지 못했다. 위 visibility CS 내부 LOD는 실제 소비자를 확인했다. 파일 존재와 실행 연결은 별도로 기록한다.

### Tessellation과 Stream-output의 학습 범위

HS는 patch의 control data와 tessellation factor, 고정 tessellator는 domain 표본 생성, DS는 그 위치의 실제 정점 평가를 담당한다. Displacement mapping은 정점을 움직여 silhouette도 바꿀 수 있지만 normal mapping은 shading normal을 바꾼다. Stream-output은 생성한 geometry 데이터를 buffer에 기록하는 경로이며 CPU instance 배열과 다른 계약이다.

이번 Engine/Client/Tools의 비무시 소스 2,106개와 지정 shader/C++ 확장자, Engine/External 제외 검색에서는 활성 HS/DS·patch topology·stream-output 생성/바인딩을 찾지 못했다. 설치 CSO 전체 reflection이나 과거 이력까지 확인한 것이 아니다. 따라서 책에서 배울 개념으로 포함하되 현재 적용 기술로 나열하지 않는다.

### 현재 높이 안개의 두 가지 식

[HEIGHT_FOG_SETTINGS](C:/Users/tnest/Desktop/LostArk/Engine/Public/Engine_RenderTypes.h:245)는 거리·높이를 미터 기준으로 보관한다. [Apply_HeightFog](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:878)는 finite/range 검증이 실패하면 기존 상태를 보존한다.

기본 [EvaluateSceneFog](C:/Users/tnest/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_SceneHeightFog.hlsli:85)는 다음과 같다.

~~~text
drift = sin(clock·driftSpeed)
ceiling = topHeight + drift·driftHeightAmplitude
density = max(0, baseDensity + drift·driftDensityAmplitude)
H = 1−exp(−max(0,ceiling−worldY)·heightFalloff)
D = 1−exp(−max(0,distance(world,camera)−startDistance)·density·.02)
a = saturate(H·D·coverageMask)·maximumOpacity
transfer = (fogColor·a, 1−a)
outputColor = inputColor·transfer.a + transfer.rgb
~~~

coverage<.999일 때 coverageMask는 world XZ와 바람 이동을 사용하는 3-octave noise에 smoothstep을 적용한 값이며, 그 외에는 1이다. coverage 값이 실제 지도 면적 비율과 정확히 같다는 보장은 없다. 이 식은 높이·거리·patch를 합친 저작 모델이며 전체 3D volume ray marching이 아니다.

Source [EvaluateSourceExponentialFog](C:/Users/tnest/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_SceneHeightFog.hlsli:62)는 source 규약에 따라 ray를 cm로 바꾸고 exp2 기반 높이 적분을 사용한다.

~~~text
ray = (world−camera)·100
vertical = abs(ray.y)>.01 ? ray.y : .01
h = falloff·.001·vertical
integral = (1−exp2(−h))/h
cameraDensity = density·.001
    ·exp2(clamp(−falloff·.001·(cameraY−topHeight)·100,−80,80))
T = clamp(exp2(−integral·max(length(ray)−startDistance·100,0)
                 ·cameraDensity), 1−maximumOpacity, 1)
~~~

이 분기는 drift/patch를 사용하지 않는다. view-ray와 fog light의 방향으로 inscattering 색을 섞고 (color·(1−T),T)를 반환한다. 반환 alpha는 불투명도 a가 아니라 투과율 T다. h→0의 극한을 실제 코드가 급수로 계산한다고 설명하지 않는다. 코드는 vertical 대체와 입구 조건을 사용한다.

예를 들어 기본 식에서 drift=0, ceiling24m, worldY14m, 거리50m, start0, density.35, falloff.08, mask1, maximumOpacity.9라면 a≈.1464다. 원래 색 약85.4%와 fog 색 약14.6%를 섞는 계산이다. 실제 화면 측정값은 아니다.

Deferred 합성에서는 일반 lighting 뒤 fog, 그 뒤 emissive다. 일부 source forward도 공통 EvaluateSceneFog를 사용한다. 기능이 존재한다고 모든 장면에서 켜져 있는 것은 아니며 Bern 기본 fog OFF는 유지한다.

## G19 계산과 코드 추적 연습

한 번에 모든 수식을 외우기보다 한 pixel의 입력과 결과를 직접 추적한다. 아래는 정답이 함께 필요한 학습용 예제다. 사용자가 이미 이 내용을 독립적으로 설명할 수 있다는 판정은 별도로 한다.

| 예제 | 가정과 계산 | 확인할 개념 |
|---|---|---|
| 원근→depth→world | W=V=I, FOV90°, aspect1, near1, far11, p=(1,0,2,1) → clip=(1,0,1.1,2) → NDC=(.5,0,.55), UV=(.75,.5), depth=(.55,.002) → 원래 p 복원 | clip w와 z/w 차이 |
| 법선 변환 | t=(1,1,0), n=(1,−1,0), scale(2,1,1) → 잘못된 내적3, 역전치 적용 내적0 | 수직 관계의 보존 |
| 표준 Fresnel | F₀=.04, c=1/.5/0 → F=.04/.07/1 | 각도에 따른 반사율 |
| 현재 source Fresnel | F₀=.04, r=.5, p=1 → F=.46 | 표준 Schlick과 실제식 차이 |
| GGX peak | r=.5, N·H=1 → D≈5.09296 | 분포 밀도는 반사율이 아님 |
| G-buffer 이론량 | 1920×1080×84=174,182,400bytes≈166.11MiB | RT 저장량과 실제 VRAM 측정 구분 |
| 추적 카메라 | k18s⁻¹, dt1/60s → α≈.25918 | 지수 응답·단위 |
| GS offset | ndcWidth.x=8/W → pixel 이동4 | clip offset에 w를 곱하는 이유 |
| CS group | 1,000instance → 16group×64thread | dispatch와 경계 guard |

원근 복원·법선 내적·Schlick·GGX·추적 α·G-buffer 산술은 별도 Python 계산으로 대조했다. 이 계산을 Client 실행, shader compile, GPU capture 또는 성능 측정으로 기록하지 않는다.

첫 코드 추적은 표면 하나로 제한한다. FLOOR12의 재질에서 비금속 F₀=.04가 만들어지는 지점, RT MaterialSpecular.rgb에 들어가는 지점, Deferred 직접광에서 읽는 지점, 마지막 albedo×shade+specular까지 이어 찾는다. 그다음 같은 표면의 normal·roughness·sourceIndirect flag를 하나씩 바꾼다고 가정하고 결과를 예상한다. 실제 옵션을 변경하는 실습은 저장된 팀장 설정과 별도 세션 조작 범위를 지키며 수행한다.

## G20 책과 원전의 역할

| 자료 | 이 프로젝트에서 연결할 목적 | 읽은 범위·한계 |
|---|---|---|
| [Frank Luna DX11 공식 목차](https://www.d3dcoder.net/d3d11.htm) | 수학→API→기법의 학습 순서 | 공개 목차·설명 확인. 본문 전체를 열람한 것은 아님 |
| [Microsoft DX11](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-graphics-pipeline) | stage·resource·binding 계약 | 현재 API 문서를 기준으로 확인 |
| [Real-Time Rendering 저자 사이트](https://www.realtimerendering.com/) | pipeline·shading·sampling·visibility를 연결하는 확장 독서 | 책 전체를 읽고 요약했다는 의미가 아님 |
| [GPU Gems 3 Deferred 사례](https://developer.nvidia.com/gpugems/gpugems3/part-iii-rendering/chapter-19-deferred-shading-tabula-rasa) | 표면 저장과 조명 분리의 설계 비교 | 당시 API 제약을 최신 DX11에 그대로 일반화하지 않음 |
| [PBRT 온라인 4판](https://pbr-book.org/4ed/contents) | radiometry·reflection·microfacet·volume의 정의 | 본문에서 연결한 해당 절 중심 |
| [Walter 등 2007](https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf) | GGX와 미세면 분포의 원전 | 프로젝트의 전체 BRDF가 논문 그대로라는 뜻은 아님 |
| [Heitz 2014](https://jcgt.org/published/0003/02/03/paper.pdf) | masking 모델·Smith 가정의 차이 | G와 scene shadow를 구분 |
| [Ramamoorthi·Hanrahan 2001](https://graphics.stanford.edu/papers/envmap/) | diffuse 환경 조명의 저차 SH | 논문의 오차를 프로젝트 측정값으로 가져오지 않음 |
| [Karis 2013](https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf) | 실시간 BRDF·IBL 근사의 비교 기준 | 현재 native LUT와 채널 의미가 다름 |

논문은 이름을 기술소개서에 많이 싣기 위한 목록이 아니다. 현재 선택한 근사가 어떤 입력을 버리고 무엇을 보존하는지 설명하는 근거다. 하나의 설명에는 문제, 입력, 수식, 실제 소비자, 비용과 한계를 함께 둔다.

## G21 기술소개서와 확인 범위

렌더링 본문은 16페이지 한 프레임, 17페이지 좌표·깊이 복원, 18페이지 8 MRT, 19페이지 PBR로 압축했다. GS·CS·fog·애니메이션의 상세는 이 학습 기록과 면접 보충으로 연결한다. 실제 그림 구성과 영상용 짧은 문장은 [기술소개서 작성 원고](C:/Users/tnest/Desktop/Interview/기술소개서/기술소개서_작성원고.md)에 모았다.

완성본은 [기술소개서_보스와_렌더링_구조정리.pptx](C:/Users/tnest/Desktop/Interview/out/2026-10-14-renderer-study/기술소개서_보스와_렌더링_구조정리.pptx)다. 이전 발탄·쿠크 7·8페이지를 포함한 43장 구성에서 렌더링 16~19페이지의 도형과 메모만 교체했다. 나머지 39장 및 무관한 패키지 내용은 바이트 단위로 보존했다. 작성한 4장은 native editable 도형과 18페이지 표를 사용한다.

PPT 구조·페이지 수·native table·재import 검사와 작성 4장의 폰트/레이아웃 검사를 통과했고, 43장 렌더 및 변경 4장 개별 검토를 마쳤다. 새 4장 경고는 0개다. 전체 파일의 기존 18개 레이아웃 경고는 이전 파일과 동일하며 무관한 페이지를 자동 수정하지 않았다. 이 검사는 PowerPoint 앱을 실행해 편집 동작을 확인한 결과와 구분한다.

이번 조사에서 현재 C++/HLSL/JSON의 연결과 수식, 공식 API 문서·공개 목차·논문을 대조했다. 사용자에게 전체 내용을 질문으로 재시험한 것은 아니며, 문서가 생겼다는 사실을 이해 완료나 개인 기여 증명으로 취급하지 않는다.

Client·Server 실행, 전체 C++ build, CSO 재컴파일, GPU 화면 관측, FPS/ms 성능 재측정은 하지 않았다. 현재 shader 두 복사본의 일치 확인은 설치된 CSO가 같은 코드라는 보장이 아니다. 과거 RESULT의 빌드 시간·성능·화면 검증은 당시 기록이며 이번 측정으로 바꾸어 적지 않는다.
