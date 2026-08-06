# Effect Tool G06 Shader와 남은 반영 가이드

기준일: 2026-08-05

기준 브랜치: `codex/effect-tool-reboot`

이 문서는 현재 실제 코드에서 G06-07까지 반영된 상태를 기준으로 한다.
새 HLSL 두 파일은 모든 선언과 코드를 설명하고 전문을 보존한다. 기존 H/CPP는 이미 있는
내용을 반복하지 않고 앞으로 추가·교체할 부분과 정확한 기준점만 적는다.

## 0. 먼저 구분할 번호

현재 완료된 `G06-07`은 **G06 안의 일곱 번째 작업 단계**다. Effect Tool 전체 단계의
`G07`과는 다르다.

```text
현재: G06-07 Preview C++ backend까지
남은 G06: G06-08~G06-17 Shader, Tool 연결, 검증
다음 전체 G: G07 Play/Stop/Loop, 자동 시간, Lerp/Curve
```

## 1. 현재 실제 반영 상태

### 반영 완료

```text
Effect_AuthoringDocument.h
  format v4 / minimum v3
  EFFECT_RENDER_PROFILE
  다중 ResourceBindings
  Transform/Color/UV/Timing/Mesh/Sprite/Decal Detail

Effect_Tool.cpp
  v3/v4 parse
  v4 serialize
  Detail validate
  Material slot과 Render Profile UI

Effect_Preview.h/.cpp
  Preview GPU 상태와 함수 선언
  Render Target 생성
  CModel/DDS resource cache
  UV/sequence/dissolve CPU 계산과 shader bind
  Mesh/Rect render 함수
  원래 RTV/DSV/Viewport 복구

Client.vcxproj/.filters
  Effect_Preview.h/.cpp 등록
```

### 아직 미반영

```text
Shader_VtxEffectMeshPreview.hlsl
Shader_VtxEffectRectPreview.hlsl
Tool의 CEffectPreview 소유
Detail 편집 panel
Preview panel과 ImGui::Image
MainApp의 Device/Context 전달
HLSL project/filter 등록
ProjectAudit G4 -> G6 교체
F1 runtime smoke
```

따라서 현재 `CEffectPreview::Render()` 구현은 존재하지만 호출자가 없다. Shader 파일도 아직
없으므로 지금 단계에서 Preview를 억지로 생성하면 `Initialize()`가 실패한다. HLSL 두 파일을
먼저 추가한 뒤 Tool과 연결해야 한다.

## 2. HLSL이 들어가는 전체 흐름

```text
EFFECT_ELEMENT_DESC
→ CEffectPreview::Bind_CommonShaderValues
→ CShader::Bind_Matrix / Bind_RawValue / Bind_Texture
→ HLSL 전역 변수
→ VS_MAIN이 정점을 화면 좌표로 변환
→ Rasterizer가 삼각형과 pixel 후보 생성
→ PS_MAIN이 최종 색 계산
→ 선택한 pass의 Rasterizer/Depth/Blend 상태 적용
→ Preview Color RTV에 기록
→ 같은 texture의 SRV를 ImGui::Image가 읽음
```

CPP 문자열과 HLSL 변수 이름은 정확히 같아야 한다.

```cpp
pShader->Bind_Matrix("g_WorldMatrix", &World);
pShader->Bind_Texture("g_BaseTexture", Base);
```

```hlsl
float4x4 g_WorldMatrix;
Texture2D g_BaseTexture;
```

이름이 다르거나 HLSL에 변수가 없으면 `CShader::Bind_*()`가 실패하고 Preview는
`E_FAIL`을 반환한다.

## 3. 공통 Shader 상태 include

두 HLSL의 첫 줄은 다음 include다.

```hlsl
#include "Engine_Shader_Defines.hlsli"
```

이 파일은 다음 공통 상태를 제공한다.

### `LinearSampler`

한 줄 책임: DDS texture를 부드럽게 보간하고 UV가 0~1을 넘으면 반복한다.

```text
MIN_MAG_MIP_LINEAR: 인접 texel과 mip을 선형 보간
AddressU/V = WRAP: 이동하는 UV가 범위를 넘으면 처음부터 반복
```

### `RS_Default`

한 줄 책임: 뒷면을 제거하는 일반 Mesh Rasterizer 상태다.

`OPAQUE_BACK_DEPTH_WRITE` pass가 사용한다.

### `RS_Cull_None`

한 줄 책임: 앞면과 뒷면을 모두 그린다.

얇은 Sprite/Decal 카드와 양면 Effect Mesh가 어느 방향에서도 보이게 한다.

### `DSS_Default`

한 줄 책임: Depth Test를 하고 통과한 pixel의 depth도 기록한다.

Opaque pass가 사용한다.

### `DSS_ReadOnly`

한 줄 책임: 기존 depth와 비교하지만 새로운 depth는 기록하지 않는다.

여러 투명 Effect가 뒤의 Effect를 부당하게 가리지 않도록 Alpha/Additive pass가 사용한다.

### `BS_Default`

한 줄 책임: blending 없이 shader 출력으로 target 값을 교체한다.

### `BS_AlphaBlend`

한 줄 책임: source alpha를 기준으로 기존 target 색과 섞는다.

개념식:

```text
Result = Source.rgb * Source.a + Destination.rgb * (1 - Source.a)
```

### `BS_Additive`

한 줄 책임: 빛처럼 기존 target 색 위에 source 색을 더한다.

개념식:

```text
Result = Source.rgb * Source.a + Destination.rgb
```

## G06-08. Mesh Preview HLSL

### 4. 파일 추가 위치

```text
파일: Client/Bin/ShaderFiles/Shader_VtxEffectMeshPreview.hlsl
작업: 새 파일
인코딩: UTF-8 BOM 없음
직접 생성자: CEffectPreview::Initialize
직접 소비자: CEffectPreview::Render_Mesh
입력 layout: Engine::VTXMESH::Elements
```

이 파일은 `CModel`의 Mesh vertex와 Material/Base texture를 받아 Preview Color Target에
그리는 shader다.

### 5. Matrix 변수

#### `g_WorldMatrix`

한 줄 책임: 모델의 local vertex를 Preview world 위치·회전·크기로 변환한다.

CPP 연결:

```text
CEffectPreview::Render_Mesh
→ Element.Detail.Transform 읽기
→ 중심 보정 * Scale * Rotation * Translation
→ Bind_CommonShaderValues
→ g_WorldMatrix
```

#### `g_ViewMatrix`

한 줄 책임: Preview camera 기준으로 world 좌표를 바라본 좌표로 변환한다.

Mesh bounds 반지름을 기준으로 camera가 모델 뒤쪽에 배치된다.

#### `g_ProjMatrix`

한 줄 책임: camera 좌표를 perspective clip 좌표로 변환한다.

45도 FOV와 Preview target의 가로세로 비율을 사용한다.

### 6. Texture 변수

#### `g_BaseTexture`

한 줄 책임: Effect의 기본 색과 alpha를 제공한다.

```text
Mesh.UseModelMaterial = true
→ CModel::Bind_Material의 diffuse SRV

Mesh.UseModelMaterial = false
→ ResourceBindings의 BASE_TEXTURE SRV
```

선택은 CPP에서 끝난다. HLSL은 어느 경로에서 왔는지와 관계없이 `g_BaseTexture`를
sample한다.

#### `g_NoiseTexture`

한 줄 책임: Dissolve 경계를 단조롭지 않게 흔드는 보조 값이다.

현재 G06에서는 noise가 단독으로 UV distortion을 만들지는 않는다.

#### `g_MaskTexture`

한 줄 책임: Base alpha에 곱해 보이는 영역을 제한한다.

Mask가 없으면 흰색 fallback을 사용하므로 `alpha * 1`이 되어 원본 alpha를 유지한다.

#### `g_EmissiveTexture`

한 줄 책임: 조명과 별도로 스스로 빛나는 색을 Base RGB에 더한다.

Emissive가 없으면 검은색 fallback을 사용하므로 더해지는 값이 0이다.

#### `g_DissolveTexture`

한 줄 책임: 현재 Dissolve 진행도보다 낮은 pixel을 `clip()`으로 제거한다.

Dissolve가 없으면 `g_HasDissolve == 0`이므로 sampling과 clip을 건너뛴다.

### 7. UV와 Color 변수

#### `g_UVScale`

한 줄 책임: Tile Columns/Rows에 맞춰 한 frame이 차지하는 UV 크기를 정한다.

```text
columns = 4, rows = 2
→ UVScale = (0.25, 0.5)
```

#### `g_UVOffset`

한 줄 책임: UV Start, Speed, Wave, 현재 Sequence frame 위치를 모두 합친 이동량이다.

이 값은 HLSL에서 시간을 계산하지 않고 CPP의 `Bind_CommonShaderValues()`가 명시적인
Sample Time으로 계산한다.

#### `g_ColorOffset`

한 줄 책임: Base color에 더할 RGBA 값이다.

#### `g_ColorMultiply`

한 줄 책임: Base color에 곱할 RGBA 값이다.

최종 기본 색 계산 순서:

```text
Base Sample * ColorMultiply + ColorOffset
```

#### `g_ColorClip`

한 줄 책임: alpha가 이 값보다 낮은 pixel을 제거한다.

```hlsl
clip(color.a - g_ColorClip);
```

차이가 음수면 pixel이 폐기된다.

#### `g_EmissiveIntensity`

한 줄 책임: Emissive texture가 최종 RGB에 더해지는 세기를 조절한다.

#### `g_DissolveAmount`

한 줄 책임: 0~1 사이의 현재 Dissolve 진행도다.

CPP는 `DissolveStartNormalized` 이후 구간을 다시 0~1로 정규화해 전달한다.

### 8. 존재 flag

#### `g_UseBaseOverride`

한 줄 책임: Base가 모델 diffuse가 아니라 Element의 Base slot에서 왔다는 계약 값이다.

실제 SRV 선택은 이미 CPP가 끝낸다. 현재 G06의 색 계산 결과를 바꾸지는 않지만 CPP와
shader의 Preview 입력 계약을 보존한다.

#### `g_HasNoise`, `g_HasMask`, `g_HasEmissive`, `g_HasDissolve`

한 줄 책임: 해당 slot이 실제로 연결됐는지 shader 분기를 결정한다.

fallback texture가 바인딩돼 있어도 “실제 입력이 있는가”와 “안전한 SRV가 있는가”는
다른 의미이므로 flag와 fallback을 함께 사용한다.

### 9. `VS_IN`

한 줄 책임: `VTXMESH::Elements`로부터 한 vertex의 값을 받는다.

```text
POSITION: local 위치
NORMAL: local 법선
TANGENT/BINORMAL: VTXMESH input layout 일치를 위해 받는 접선 축
TEXCOORD0: 원본 UV
```

Tangent와 Binormal은 현재 G06 Preview에서 normal map을 계산하지 않으므로 다음 단계로
전달하지 않는다. 그러나 C++ input layout과 HLSL input signature는 일치해야 한다.

### 10. `VS_OUT`

한 줄 책임: Rasterizer와 Pixel Shader에 clip 위치, world 법선, 계산된 UV를 전달한다.

```text
SV_POSITION: 화면에서 삼각형을 만들 clip 좌표
NORMAL: 간단한 Preview 조명에 사용할 법선
TEXCOORD0: Tile/Offset이 적용된 UV
```

### 11. `VS_MAIN`

한 줄 책임: Mesh vertex를 화면 좌표로 변환하고 UV를 적용한다.

내부 흐름:

```text
input.position
→ World
→ View
→ Projection
→ output.position

input.normal
→ World의 방향 변환(w=0)
→ normalize
→ output.normal

input.uv
→ UVScale 곱
→ UVOffset 더하기
→ output.uv
```

### 12. `Shade_Effect`

한 줄 책임: Texture, Mask, Dissolve, Color, 간단한 조명, Emissive를 하나의 RGBA로 합친다.

내부 흐름:

```text
Base sample
→ 선택적 Noise/Mask 읽기
→ 선택적 Dissolve clip
→ Color Multiply + Offset
→ Mask를 alpha에 적용
→ Color Clip
→ normal 기반 간단한 방향광
→ 선택적 Emissive 더하기
→ 0~1 범위로 clamp
```

### 13. `PS_MAIN`

한 줄 책임: Rasterizer가 만든 pixel 후보 하나마다 `Shade_Effect()` 결과를 반환한다.

`SV_TARGET0`은 현재 Preview에서 바인딩한 Color RTV의 첫 번째 color target이다.

### 14. 세 개의 pass

#### Pass 0 `OpaqueBackDepthWrite`

```text
RS_Default: 뒷면 제거
DSS_Default: depth 검사 + 기록
BS_Default: blending 없음
```

#### Pass 1 `AlphaTwoSidedDepthRead`

```text
RS_Cull_None: 양면
DSS_ReadOnly: depth 검사, 기록 안 함
BS_AlphaBlend: alpha blending
```

#### Pass 2 `AdditiveTwoSidedDepthRead`

```text
RS_Cull_None: 양면
DSS_ReadOnly: depth 검사, 기록 안 함
BS_Additive: 빛처럼 기존 색에 더함
```

`CEffectPreview::Select_Pass()`가 정확히 0/1/2를 반환하므로 이 순서는 바꾸면 안 된다.

### 15. `Shader_VtxEffectMeshPreview.hlsl` 전체 코드

```hlsl
#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

Texture2D g_BaseTexture;
Texture2D g_NoiseTexture;
Texture2D g_MaskTexture;
Texture2D g_EmissiveTexture;
Texture2D g_DissolveTexture;

float2 g_UVScale = float2(1.f, 1.f);
float2 g_UVOffset = float2(0.f, 0.f);
float4 g_ColorOffset = float4(0.f, 0.f, 0.f, 0.f);
float4 g_ColorMultiply = float4(1.f, 1.f, 1.f, 1.f);
float g_ColorClip = 0.f;
float g_EmissiveIntensity = 1.f;
float g_DissolveAmount = 0.f;
uint g_UseBaseOverride = 0;
uint g_HasNoise = 0;
uint g_HasMask = 0;
uint g_HasEmissive = 0;
uint g_HasDissolve = 0;

struct VS_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;
    output.position = mul(
        float4(input.position, 1.f),
        mul(mul(g_WorldMatrix, g_ViewMatrix), g_ProjMatrix));
    output.normal = normalize(
        mul(float4(input.normal, 0.f), g_WorldMatrix).xyz);
    output.uv = input.uv * g_UVScale + g_UVOffset;
    return output;
}

float4 Shade_Effect(float2 uv, float3 normal)
{
    float4 color = g_BaseTexture.Sample(LinearSampler, uv);
    const float noise = 0 != g_HasNoise ?
        g_NoiseTexture.Sample(LinearSampler, uv).r : 0.f;
    const float mask = 0 != g_HasMask ?
        g_MaskTexture.Sample(LinearSampler, uv).r : 1.f;

    if (0 != g_HasDissolve)
    {
        const float dissolve =
            g_DissolveTexture.Sample(LinearSampler, uv).r +
            noise * 0.1f;
        clip(dissolve - g_DissolveAmount);
    }

    color = color * g_ColorMultiply + g_ColorOffset;
    color.a *= mask;
    clip(color.a - g_ColorClip);

    const float light = 0.35f +
        saturate(dot(
            normalize(normal),
            normalize(float3(0.4f, 0.8f, -0.3f)))) * 0.65f;
    color.rgb *= light;

    if (0 != g_HasEmissive)
    {
        color.rgb += g_EmissiveTexture.Sample(
            LinearSampler, uv).rgb * g_EmissiveIntensity;
    }

    return saturate(color);
}

float4 PS_MAIN(VS_OUT input) : SV_TARGET0
{
    return Shade_Effect(input.uv, input.normal);
}

technique11 DefaultTechnique
{
    pass OpaqueBackDepthWrite
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(
            BS_Default,
            float4(0.f, 0.f, 0.f, 0.f),
            0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass AlphaTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(
            BS_AlphaBlend,
            float4(0.f, 0.f, 0.f, 0.f),
            0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass AdditiveTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(
            BS_Additive,
            float4(0.f, 0.f, 0.f, 0.f),
            0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
```

## G06-09. Rect Preview HLSL

### 16. 파일 추가 위치

```text
파일: Client/Bin/ShaderFiles/Shader_VtxEffectRectPreview.hlsl
작업: 새 파일
인코딩: UTF-8 BOM 없음
직접 생성자: CEffectPreview::Initialize
직접 소비자: CEffectPreview::Render_Rect
입력 layout: Engine::VTXTEX::Elements
```

이 파일은 `CVIBuffer_Rect`를 이용해 Sprite와 Decal의 Base texture를 카드 형태로 그린다.

```text
Sprite: 정면 사각형 texture Preview
Decal: Decal Size를 곱한 사각형 카드 Preview
```

현재 G06 Decal은 world surface에 투영하는 실제 decal projector가 아니다.

### 17. Mesh Shader와 같은 변수

다음 변수의 의미와 CPP Bind 경로는 Mesh Shader와 같다.

```text
g_WorldMatrix / g_ViewMatrix / g_ProjMatrix
g_BaseTexture / g_NoiseTexture / g_MaskTexture
g_EmissiveTexture / g_DissolveTexture
g_UVScale / g_UVOffset
g_ColorOffset / g_ColorMultiply / g_ColorClip
g_EmissiveIntensity / g_DissolveAmount
g_HasNoise / g_HasMask / g_HasEmissive / g_HasDissolve
```

Rect에는 `g_UseBaseOverride`가 없다. Sprite/Decal은 반드시 Base slot을 사용하므로 모델
Material과 Base override를 구분할 필요가 없다.

### 18. `VS_IN`

한 줄 책임: `VTXTEX::Elements`의 Position과 UV를 받는다.

```text
POSITION: CVIBuffer_Rect의 local 위치
TEXCOORD0: 사각형 원본 UV
```

Rect에는 normal/tangent/binormal이 없다.

### 19. `VS_OUT`

한 줄 책임: clip 위치와 계산된 UV만 Pixel Shader로 전달한다.

Mesh와 달리 normal을 전달하지 않으므로 Rect Pixel Shader에는 방향광 계산이 없다.

### 20. `VS_MAIN`

한 줄 책임: Rect vertex를 World/View/Projection으로 변환하고 UV Tile/Offset을 적용한다.

CPP의 `Render_Rect()`는 다음 World를 만든다.

```text
Transform Scale
→ Decal이면 Decal Size 추가 적용
→ Rotation + Revolution
→ Position
```

View는 Identity, Projection은 2x2 Orthographic다. 따라서 Rect는 3D 원근 카메라가 아니라
Preview 카드처럼 보인다.

### 21. `PS_MAIN`

한 줄 책임: Rect pixel의 Base/Mask/Dissolve/Color/Emissive를 계산한다.

내부 흐름:

```text
Base sample
→ 선택적 Noise/Mask 읽기
→ 선택적 Dissolve clip
→ Color Multiply + Offset
→ Mask를 alpha에 적용
→ Color Clip
→ 선택적 Emissive 더하기
→ 0~1 clamp
```

Mesh와의 차이는 조명 계산이 없다는 점뿐이다.

### 22. Rect pass 불변식

Rect도 Mesh와 정확히 같은 pass 번호를 사용한다.

```text
0 OpaqueBackDepthWrite
1 AlphaTwoSidedDepthRead
2 AdditiveTwoSidedDepthRead
```

Rect의 Opaque pass는 이름과 계약 통일을 위해 `RS_Default`를 사용한다. 일반 Sprite/Decal은
대부분 Alpha 또는 Additive profile을 선택해 `RS_Cull_None`으로 그린다.

### 23. `Shader_VtxEffectRectPreview.hlsl` 전체 코드

```hlsl
#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

Texture2D g_BaseTexture;
Texture2D g_NoiseTexture;
Texture2D g_MaskTexture;
Texture2D g_EmissiveTexture;
Texture2D g_DissolveTexture;

float2 g_UVScale = float2(1.f, 1.f);
float2 g_UVOffset = float2(0.f, 0.f);
float4 g_ColorOffset = float4(0.f, 0.f, 0.f, 0.f);
float4 g_ColorMultiply = float4(1.f, 1.f, 1.f, 1.f);
float g_ColorClip = 0.f;
float g_EmissiveIntensity = 1.f;
float g_DissolveAmount = 0.f;
uint g_HasNoise = 0;
uint g_HasMask = 0;
uint g_HasEmissive = 0;
uint g_HasDissolve = 0;

struct VS_IN
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;
    output.position = mul(
        float4(input.position, 1.f),
        mul(mul(g_WorldMatrix, g_ViewMatrix), g_ProjMatrix));
    output.uv = input.uv * g_UVScale + g_UVOffset;
    return output;
}

float4 PS_MAIN(VS_OUT input) : SV_TARGET0
{
    float4 color = g_BaseTexture.Sample(LinearSampler, input.uv);
    const float noise = 0 != g_HasNoise ?
        g_NoiseTexture.Sample(LinearSampler, input.uv).r : 0.f;
    const float mask = 0 != g_HasMask ?
        g_MaskTexture.Sample(LinearSampler, input.uv).r : 1.f;

    if (0 != g_HasDissolve)
    {
        const float dissolve =
            g_DissolveTexture.Sample(LinearSampler, input.uv).r +
            noise * 0.1f;
        clip(dissolve - g_DissolveAmount);
    }

    color = color * g_ColorMultiply + g_ColorOffset;
    color.a *= mask;
    clip(color.a - g_ColorClip);

    if (0 != g_HasEmissive)
    {
        color.rgb += g_EmissiveTexture.Sample(
            LinearSampler, input.uv).rgb * g_EmissiveIntensity;
    }

    return saturate(color);
}

technique11 DefaultTechnique
{
    pass OpaqueBackDepthWrite
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(
            BS_Default,
            float4(0.f, 0.f, 0.f, 0.f),
            0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass AlphaTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(
            BS_AlphaBlend,
            float4(0.f, 0.f, 0.f, 0.f),
            0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass AdditiveTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(
            BS_Additive,
            float4(0.f, 0.f, 0.f, 0.f),
            0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
```

## 24. 현재 C++ Bind와 HLSL 이름 대조표

```text
Effect_Preview.cpp                          HLSL
Bind_Matrix("g_WorldMatrix")            g_WorldMatrix
Bind_Matrix("g_ViewMatrix")             g_ViewMatrix
Bind_Matrix("g_ProjMatrix")             g_ProjMatrix
Bind_RawValue("g_UVScale")              g_UVScale
Bind_RawValue("g_UVOffset")             g_UVOffset
Bind_RawValue("g_ColorOffset")          g_ColorOffset
Bind_RawValue("g_ColorMultiply")        g_ColorMultiply
Bind_RawValue("g_ColorClip")            g_ColorClip
Bind_RawValue("g_EmissiveIntensity")    g_EmissiveIntensity
Bind_RawValue("g_DissolveAmount")       g_DissolveAmount
Bind_RawValue("g_HasNoise")             g_HasNoise
Bind_RawValue("g_HasMask")              g_HasMask
Bind_RawValue("g_HasEmissive")          g_HasEmissive
Bind_RawValue("g_HasDissolve")          g_HasDissolve
Bind_Texture("g_NoiseTexture")          g_NoiseTexture
Bind_Texture("g_MaskTexture")           g_MaskTexture
Bind_Texture("g_EmissiveTexture")       g_EmissiveTexture
Bind_Texture("g_DissolveTexture")       g_DissolveTexture
Bind_Texture("g_BaseTexture")           g_BaseTexture
Mesh: Bind_RawValue("g_UseBaseOverride") g_UseBaseOverride
```

## 25. G06에서 저장하지만 아직 실제 화면에 소비하지 않는 값

다음 값은 Document 저장과 검증은 완료됐지만 G06 Shader의 최종 화면 계산에는 아직 들어가지 않는다.

### `DistortionIntensity`, `DistortionOnBaseMaterial`

실제 distortion은 Scene Color를 읽고 별도 composite해야 한다. Preview Color만 그리는 현재
G06 pass에 억지로 넣지 않고 G08에서 소비한다.

### `RadialTime`, `RadialIntensity`

Document에는 저장되지만 현재 G06 Preview shader에는 radial UV 연산이 없다. 후속 UV/curve
계약과 함께 연결한다.

### `AfterImageSeconds`

현재 G06에서는 Preview slider의 최대 시간 계산에만 사용한다. 잔상을 여러 장 누적해 그리는
실제 After Image renderer는 G08 module 범위다.

### `Sprite.bBillboard`

현재 Rect Preview는 View Identity인 카드이므로 camera-facing billboard 계산을 하지 않는다.
제품 world Sprite module에서 실제 camera를 기준으로 소비한다.

### `Decal.fDepth`

현재 G06은 투영 Decal이 아니라 카드 Preview이므로 Size만 화면 크기에 사용한다. Depth는
제품 Decal projector 계약에서 소비한다.

## G06-10. `Effect_Tool.h`에 앞으로 추가할 부분

현재 `m_eSelectedResourceSlot`, `Render_MaterialPanel`, `Try_ApplyRenderProfile`은 반영 완료다.
다시 추가하지 않는다.

### 26. `<memory>` include

```text
파일: Client/Public/Effect_Tool.h
작업: 추가
기준점: #include <array>
위치: #include <array> 바로 아래, #include <optional> 바로 위
추가할 대상: 표준 라이브러리 include
필요한 이유: unique_ptr<CEffectPreview>가 Preview의 강한 owner이기 때문
연결되는 부분: m_pPreview
```

```cpp
#include <memory>
```

### 27. `CEffectPreview` 전방 선언

```text
파일: Client/Public/Effect_Tool.h
작업: 추가
기준점: NS_BEGIN(Client)
위치: NS_BEGIN(Client) 바로 아래, class CEffect_Tool final 바로 위
추가할 대상: class 전방 선언
정의 위치: Client/Public/Effect_Preview.h
필요한 이유: Tool 헤더는 unique_ptr 타입 이름만 알면 되고 Preview 전체 정의는 CPP에서 읽음
연결되는 부분: m_pPreview, CEffect_Tool 소멸자
```

```cpp
class CEffectPreview;
```

### 28. 생성자와 소멸자 교체

```text
파일: Client/Public/Effect_Tool.h
작업: 교체
기준점 시작: CEffect_Tool() = default;
기준점 끝: ~CEffect_Tool() = default;
위치: 기존 두 선언 전체 교체
추가할 대상: public 함수 선언
정의 위치: Client/Private/Effect_Tool.cpp, anonymous namespace 닫는 } 아래
필요한 이유: MainApp의 Device/Context로 Preview GPU 객체를 만들기 위함
연결되는 부분: CMainApp::EnsureDebugTool, CEffectPreview::Initialize
```

```cpp
CEffect_Tool(
    ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext);
~CEffect_Tool();
```

소멸자를 CPP에 정의해야 `unique_ptr<CEffectPreview>`가 파괴되는 지점에서 완전한
`CEffectPreview` 타입을 볼 수 있다.

### 29. Panel 선언 두 개

```text
파일: Client/Public/Effect_Tool.h
작업: 추가
기준점: void Render_MaterialPanel();
위치: Render_MaterialPanel 선언 바로 아래
추가할 대상: private 함수 선언
정의 위치: Client/Private/Effect_Tool.cpp, Render_MaterialPanel 정의 바로 아래부터
필요한 이유: Detail 편집과 Preview 출력을 Material 선택 책임과 분리
연결되는 부분: CEffect_Tool::Render
```

```cpp
void Render_DetailPanel();
void Render_PreviewPanel();
```

### 30. `Try_ApplyDetail` 선언

```text
파일: Client/Public/Effect_Tool.h
작업: 추가
기준점: bool_t Try_ApplyRenderProfile(EFFECT_RENDER_PROFILE eProfile);
위치: 기준점 바로 아래
추가할 대상: private 함수 선언
정의 위치: Client/Private/Effect_Tool.cpp, Try_ApplyRenderProfile 정의 바로 아래
필요한 이유: ImGui가 수정한 후보 Detail을 validate한 뒤 Document에 한 번만 commit
연결되는 부분: Render_DetailPanel
```

```cpp
bool_t Try_ApplyDetail(const EFFECT_DETAIL_DESC& Detail);
```

### 31. Preview session 멤버

```text
파일: Client/Public/Effect_Tool.h
작업: 추가
기준점: bool_t m_bResourceCatalogRefreshAttempted = false;
위치: 기준점 바로 아래, class 마지막 }; 바로 위
추가할 대상: 멤버 변수
필요한 이유: GPU Preview owner, 명시적인 정지 시간, 상태 문구를 Tool session이 소유
연결되는 부분: 생성자, Render_PreviewPanel, Element/Resource 변경 명령
```

```cpp
unique_ptr<CEffectPreview> m_pPreview;
f32_t m_fPreviewTimeSeconds = 0.f;
std::string m_strDetailStatus;
```

```text
m_pPreview
  Tool이 강하게 소유하는 Preview renderer
  Document나 제품 Runtime Effect를 소유하지 않음

m_fPreviewTimeSeconds
  사용자가 slider로 고른 정지 Sample Time
  Document에 저장하지 않는 Tool session 값

m_strDetailStatus
  Detail validate 또는 Preview 실패 이유
  실패를 숨기지 않고 F1 창에 표시
```

## G06-11. `Effect_Tool.cpp`에 앞으로 추가할 부분

### 32. Preview include

```text
파일: Client/Private/Effect_Tool.cpp
작업: 추가
기준점: #include "Effect_Tool.h"
위치: 기준점 바로 아래, #include "DataJson.h" 바로 위
추가할 대상: 프로젝트 include
필요한 이유: Preview 생성, 함수 호출, unique_ptr 파괴에 완전한 class 정의가 필요
연결되는 부분: Tool 생성자/소멸자, Render_PreviewPanel
```

```cpp
#include "Effect_Preview.h"
```

### 33. Tool 생성자와 소멸자 정의

```text
파일: Client/Private/Effect_Tool.cpp
작업: 추가
기준점: anonymous namespace를 닫는 }와 void Client::CEffect_Tool::Render()
위치: anonymous namespace 닫는 } 바로 아래, Render 정의 바로 위
추가할 대상: 함수 정의 두 개
헤더 선언: CEffect_Tool(...), ~CEffect_Tool()
필요한 이유: MainApp에서 받은 Device/Context로 Preview 공용 GPU 객체를 한 번 생성
연결되는 부분: CEffectPreview::Initialize
```

생성자 한 줄 책임: Device/Context를 Preview에 전달하고 공용 GPU resource를 초기화한다.

내부 흐름:

```text
MainApp Device/Context
→ make_unique<CEffectPreview>
→ CEffectPreview::Initialize
→ 실패하면 Get_Status를 m_strDetailStatus에 보존
```

소멸자 한 줄 책임: 완전한 `CEffectPreview` 타입을 보는 CPP에서 unique_ptr을 파괴한다.

### 34. `Render()` 작업 영역 교체

```text
파일: Client/Private/Effect_Tool.cpp
작업: 기존 Active Document UI 일부 교체
기준점 시작: Render_AddElementPanel();
기준점 끝: Render_MaterialPanel();
위치: 두 호출이 있는 기존 블록 교체
추가할 대상: ImGui 2열 작업 영역
필요한 이유: 왼쪽 편집, 오른쪽 Preview를 같은 선택 Element로 표시
연결되는 부분: Render_MaterialPanel, Render_DetailPanel, Render_PreviewPanel
```

새 호출 순서:

```text
Render_AddElementPanel
→ Render_ElementList
→ 왼쪽 Render_MaterialPanel + Render_DetailPanel
→ 오른쪽 Render_PreviewPanel
```

### 35. `Render_ElementList()`에 추가되는 reset

```text
파일: Client/Private/Effect_Tool.cpp
작업: 추가
기준점: Element 선택 성공 분기 안의 m_strResourceStatus.clear();
위치: 기준점 바로 아래
추가할 대상: 상태 초기화
필요한 이유: 다른 Element가 이전 Element의 시간과 GPU cache를 재사용하지 않게 함
연결되는 부분: m_fPreviewTimeSeconds, CEffectPreview::Clear
```

```cpp
m_fPreviewTimeSeconds = 0.f;
m_pPreview->Clear();
```

### 36. `Render_DetailPanel()`

```text
파일: Client/Private/Effect_Tool.cpp
작업: 추가
기준점: void Client::CEffect_Tool::Render_MaterialPanel() 정의의 닫는 }
위치: Render_MaterialPanel 정의 바로 아래, 새 Render_PreviewPanel 정의 바로 위
추가할 대상: 함수 정의
헤더 선언: void Render_DetailPanel();
필요한 이유: ImGui session 수정과 Active Document commit을 분리
연결되는 부분: Try_ApplyDetail
```

한 줄 책임: 선택 Element의 Detail 복사본만 ImGui로 수정하고 변경된 경우 검증 명령을 제출한다.

내부 흐름:

```text
선택 Element 검색
→ EFFECT_DETAIL_DESC Staged = Element.Detail
→ ImGui가 Staged만 수정
→ 어떤 widget이라도 변경됐으면 Try_ApplyDetail(Staged)
→ 성공: Active Document 전체 stage/validate/commit
→ 실패: 기존 Detail 유지, m_strDetailStatus에 이유 표시
```

Panel 입력:

```text
Transform: Position, Rotation, Revolution, Scale
Color: Offset, Multiply, Clip, Emissive, Distortion, Radial
UV: Start, Speed, Wave, Wave Amplitude/Frequency,
    Sequence, Loop, Term, Columns, Rows, Index
Timing: Start Delay, Lifetime, After Image, Dissolve Start
Mesh: Use Model Material
Sprite: Billboard
Decal: Size, Depth
```

### 37. `Render_PreviewPanel()`

```text
파일: Client/Private/Effect_Tool.cpp
작업: 추가
기준점: 새 Render_DetailPanel() 정의의 닫는 }
위치: 기준점 바로 아래, Try_CreateDocument 정의보다 위
추가할 대상: 함수 정의
헤더 선언: void Render_PreviewPanel();
필요한 이유: Sample Time 입력, off-screen render, ImGui 출력 책임을 한곳에 둠
연결되는 부분: CEffectPreview::Render/Get_TextureView/Get_Status
```

한 줄 책임: 선택 Element를 명시적인 시간으로 한 장 렌더하고 성공한 SRV만 표시한다.

내부 흐름:

```text
선택 Element 검색
→ Max Time = StartDelay + LifeTime + AfterImage
→ Sample Time slider
→ Preview::Render(Element, SampleTime, Width, Height)
→ S_OK이고 SRV가 있으면 ImGui::Image
→ S_FALSE면 Particle/Trail 안내
→ 실패면 Get_Status 표시, 이전 성공 Preview cache 보존
```

`S_FALSE`는 `FAILED(S_FALSE) == false`이므로 성공 이미지로 취급하면 안 된다. 반드시
`S_OK == Result`를 확인하거나 `S_FALSE`를 별도로 분리한다.

### 38. `Try_ApplyDetail()`

```text
파일: Client/Private/Effect_Tool.cpp
작업: 추가
기준점: bool_t Client::CEffect_Tool::Try_ApplyRenderProfile(...) 정의의 닫는 }
위치: 기준점 바로 아래, Discard_ActiveDocument 정의 바로 위
추가할 대상: 함수 정의
헤더 선언: bool_t Try_ApplyDetail(const EFFECT_DETAIL_DESC& Detail);
필요한 이유: 잘못된 ImGui 숫자가 Active Document에 부분 반영되지 않게 함
연결되는 부분: Render_DetailPanel, Validate_EffectDocument
```

한 줄 책임: 후보 Detail을 Document 전체에 stage한 뒤 검증 성공 시 한 번만 commit한다.

내부 흐름:

```text
Active Document와 selected Element 확인
→ Document 전체 복사
→ Staged Element.Detail 교체
→ Validate_EffectDocument
→ 성공하면 m_ActiveDocument 교체
→ 실패하면 기존 Document/Preview 유지
```

### 39. 기존 명령 성공 뒤 추가되는 Preview reset

현재 각 함수의 기존 stage/validate/commit은 유지한다. 성공 commit 뒤에 다음 상태만 추가한다.

```text
Try_CreateDocument
  sample time = 0
  Preview Clear

Try_AddElement
  sample time = 0
  Preview Clear

Try_LoadDocument
  sample time = 0
  Preview Clear

Try_BindSelectedResource
  Preview Clear

Try_ClearSelectedResource
  Preview Clear

Discard_ActiveDocument
  detail status clear
  sample time = 0
  Preview Clear
```

## G06-12. MainApp 연결

### 40. 생성 한 줄 교체

```text
파일: Client/Private/MainApp.cpp
작업: 교체
함수: CMainApp::EnsureDebugTool
기준점: case DEBUG_TOOL::EFFECT 안의 make_unique<CEffect_Tool>()
위치: 기존 생성식 한 줄 교체
추가할 대상: 생성자 인자
필요한 이유: Preview가 GPU resource를 생성하고 draw할 Device/Context가 필요
연결되는 부분: CEffect_Tool 생성자
```

```text
기존 -> 변경
make_unique<CEffect_Tool>()
make_unique<CEffect_Tool>(m_pDevice, m_pContext)
```

Device/Context의 실제 owner는 MainApp/Engine 그래픽 계층이다. Tool과 Preview의 `ComPtr`은
같은 COM 객체의 reference를 보관할 뿐 Device/Context를 복제하지 않는다.

## G06-13. Project와 Filter 등록

### 41. 현재 등록 완료 항목

```text
Effect_Preview.h: Client.vcxproj + filters 등록 완료
Effect_Preview.cpp: Client.vcxproj + filters 등록 완료
```

다시 추가하지 않는다.

### 42. HLSL project 등록

```text
파일: Client/Default/Client.vcxproj
작업: 추가
기준점: 기존 FxCompile Include가 모여 있는 ItemGroup
위치: 같은 FxCompile ItemGroup 안
추가할 대상: FxCompile 두 줄
필요한 이유: Visual Studio build와 Solution Explorer가 shader를 정식 입력으로 인식
연결되는 부분: CEffectPreview::Initialize의 shader 경로
```

```xml
<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshPreview.hlsl" />
<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectRectPreview.hlsl" />
```

### 43. HLSL filter 등록

```text
파일: Client/Default/Client.vcxproj.filters
작업: 추가
기준점: <Filter>97.ShaderFiles</Filter>를 사용하는 기존 FxCompile 블록
위치: 같은 FxCompile ItemGroup 안
추가할 대상: FxCompile 두 블록
필요한 이유: Visual Studio에서 두 파일을 97.ShaderFiles 아래 표시
연결되는 부분: Client.vcxproj의 동일 Include 경로
```

```xml
<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshPreview.hlsl">
  <Filter>97.ShaderFiles</Filter>
</FxCompile>
<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectRectPreview.hlsl">
  <Filter>97.ShaderFiles</Filter>
</FxCompile>
```

## G06-14. ProjectAudit 교체

현재 실제 Audit은 여전히 `effect.g4-typed-resource-boundary`를 검사한다. 그래서 G06 코드가
정상이어도 Effect Audit은 실패한다.

```text
파일: Tools/ProjectAudit/Invoke-ProjectAudit.ps1
작업: 기존 Effect G4 check 전체 교체
기준점 시작: effect.g4-typed-resource-boundary를 만드는 shape 변수
기준점 끝: Add-Check 'effect.g4-typed-resource-boundary' 블록의 닫는 괄호
추가할 대상: effect.g6-detail-stateless-preview-boundary check
필요한 이유: 실제 v4/Detail/Preview/HLSL/MainApp/project 계약을 검사
연결되는 부분: ProjectAudit 완료 조건
```

새 check가 확인할 것:

```text
format 4 / minimum 3
ResourceBindings + Material + Detail
Document header에 filesystem/public parser 없음
Tool CPP가 parse/validate/serialize 소유
Detail/Preview panel 존재
Preview가 CModel/RenderTarget/Pass 사용
MainApp이 Device/Context 전달
두 Shader의 pass 순서 일치
Preview H/CPP와 Shader project 등록
Effect_AuthoringDocument.cpp 없음
```

## G06-15. 실제 작성 순서

```text
1. Shader_VtxEffectMeshPreview.hlsl 전문 작성
2. Shader_VtxEffectRectPreview.hlsl 전문 작성
3. 두 HLSL을 vcxproj/filters에 등록
4. Effect_Tool.h에 Preview owner 계약 추가
5. Effect_Tool.cpp에 생성자/소멸자 추가
6. Render_DetailPanel 작성
7. Try_ApplyDetail 작성
8. Render_PreviewPanel 작성
9. 기존 명령 성공 뒤 Preview Clear 추가
10. MainApp 생성 인자 교체
11. ProjectAudit G4 block을 G6 block으로 교체
12. Debug build와 F1 smoke
```

## G06-16. 자동 검증

### XML parse

```powershell
$project = [xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj' -Raw)
$filters = [xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj.filters' -Raw)
```

기대 결과: 두 명령 모두 XML 예외가 없다.

### Debug 정본 회귀

```powershell
powershell -ExecutionPolicy Bypass -File `
  Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
```

### ProjectAudit

```powershell
powershell -ExecutionPolicy Bypass -File `
  Tools/ProjectAudit/Invoke-ProjectAudit.ps1
```

기대 결과:

```text
effect.g6-detail-stateless-preview-boundary PASS
effect.g4-typed-resource-boundary 이름 없음
```

### 잔류 G4 검색

```powershell
rg -n "ResourceBinding\b|\bVU\b|CClientPreview|LostArkEffectToolG4" `
  Client/Public/Effect_AuthoringDocument.h `
  Client/Public/Effect_Tool.h `
  Client/Private/Effect_Tool.cpp
```

기대 결과: 단일 `ResourceBinding`, `VU`, `CClientPreview`, G4 window ID가 0건이다.

### whitespace 검사

```powershell
git diff --check
```

## G06-17. F1 Runtime smoke

Client 작업 디렉터리는 `Client/Default`를 사용한다.

```text
1. Client 실행
2. F1
3. Effect Tool 선택
4. Document Create 또는 Load
5. Mesh Element 추가
6. Mesh Model slot에 .wmodel 연결
7. Render Profile 0/1/2 변경
8. Transform/Color/UV/Timing 변경
9. 오른쪽 Sample Time 변경
10. Mesh Preview 확인
11. Sprite/Decal과 Base DDS 확인
12. Save → Discard → Load
13. binding/profile/detail 동일 확인
```

실패 경로:

```text
Particle/Trail 선택
→ G08 required 문구
→ Mesh/Rect 가짜 Preview를 표시하지 않음

잘못된 asset 또는 texture load 실패
→ 오류 문구
→ 직전 성공 Preview cache 유지

Start Delay보다 이른 Sample Time
→ 정상 빈 Preview target
→ 오류로 처리하지 않음
```

## G06 종료 경계

G06에서 끝나는 기능:

```text
사용자가 Sample Time을 직접 고름
Mesh/Sprite/Decal 한 장을 off-screen render
Material/Transform/Color/UV/Timing을 정지 상태로 확인
```

G06에 포함하지 않는 기능:

```text
Play/Stop/Loop 자동 시간: G07
Lerp/Curve timeline: G07
Particle/Trail module: G08
실제 After Image 누적: G08
Scene Color distortion composite: G08
실제 world projected Decal: 별도 runtime module 계약
```
