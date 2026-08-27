# Effect V2 소프트 파티클 계획

Effect Tool V2로 저작한 이펙트가 지형·메쉬와 교차할 때 depth test가 남기는 딱딱한 절단선을
씬 깊이 기반 알파 페이드로 없앤다. 적용 범위는 V2 경로(`CEffectV2Object` + `Shader_Effect*V2.hlsl`)
하나뿐이며, 제품 V1 경로(`CEffectDocumentRenderer` + `Shader_VtxEffect*.hlsl`)는 이번 변경에서 제외한다.

## 1. 현재 실측 상태

2026-08-27 기준으로 직접 읽은 값이다.

```text
branch  fix/basic-attack-hold-and-audition-cooldown (origin/main +2, ghost Valtan 작업)
        -> 이 작업은 별도 branch(feature/effect-v2-soft-particle)에서 진행한다

Engine/Private/Renderer.cpp:139   Target_Depth  R32G32B32A32_FLOAT, clear (1,1,1,1)
Engine/Private/Renderer.cpp:209   MRT_GameObject 에 Target_Depth 포함
Shader_VtxMeshBinary.hlsl:211     output.vDepth = float4(projZ/projW, projW/1000, specPower, 0)
                                  -> .y * 1000 == view-space W (월드 단위 선형 깊이)

Renderer.cpp:372  Render_NonBlend()        Target_Depth 채움
Renderer.cpp:388  Begin_MRT(MRT_SceneHDR)  RT0 씬컬러 / RT1 디스토션
Renderer.cpp:404  Render_Blend()           <- 이펙트가 여기서 그려짐
EffectV2_Object.cpp:1074  Add_RenderObject(RENDERGROUP::BLEND, ...)

Shader_EffectDecalV2.hlsl:6,37    이미 g_DepthTexture 선언 + depth.y * 1000.f 관례 사용
EffectV2_Object.cpp:1208          Render_Decal 이 Bind_RT_SRV(TEXT("Target_Depth"), ...) 수행

Data/Effects/V2/Authored/*.effectv2.json   61개, formatVersion 1
EffectV2_Document.cpp:70  Read_Number 는 키가 없으면 기본값 유지하고 true 반환
```

### 1.1 이번 변경이 성립하는 근거

`Target_Depth`는 `Render_Blend` 시점에 RTV로 묶여 있지 않고(그때 바인딩된 MRT는 `MRT_SceneHDR`)
불투명 지오메트리가 이미 다 쓴 상태다. 그래서 같은 프레임에 SRV로 읽어도 read/write 해저드가 없다.
데칼이 `CEffectV2Object` 안에서 이미 똑같이 하고 있으므로 바인딩 경로는 새로 증명할 것이 없다.

빠져 있는 것은 두 가지뿐이다.

```text
1. PS_EFFECT_V2 가 g_DepthTexture 도, 자기 픽셀의 화면 UV 도 모른다
2. 저작 문서에 페이드 거리를 담을 field 가 없다
```

`PS_EFFECT_IN`에 clip-space 위치(`vProjPos`)를 하나 추가하면 화면 UV(`xy/w`)와 픽셀 자신의
view-space 깊이(`w`)를 한 번에 얻는다. 별도 viewport 크기 uniform이나 역행렬이 필요 없다.

### 1.2 불변식

```text
g_SoftFadeDistance == 0        기존 동작과 완전히 동일. 저작 문서 61개는 손대지 않는다.
vProjPos.w == 0                소프트 페이드 opt-out 표식. 데칼 패스가 이 값을 쓴다.
Target_Depth 는 불투명만 기록   이펙트끼리 겹치는 경계는 부드러워지지 않는다.
formatVersion 은 1 을 유지      Read_Number 가 키 부재를 허용하므로 bump 하지 않는다.
```

### 1.3 데칼을 반드시 제외해야 하는 이유

데칼의 `PS_MAIN`은 `g_DepthTexture`에서 읽은 씬 깊이로 자기 픽셀의 월드 좌표를 **역산**한다.
즉 데칼 픽셀의 깊이는 씬 깊이와 정의상 같다. 여기에 소프트 페이드를 그대로 걸면
`sceneViewZ - pixelViewZ == 0`이 되어 알파가 0이 되고 데칼이 통째로 사라진다.
`vProjPos.w == 0` 표식으로 opt-out 시키고, 데칼은 기존 `fEdgeFade`를 계속 쓴다.

또한 데칼 셰이더가 이미 갖고 있는 `Texture2D g_DepthTexture;` 선언은 공통 hlsli로 올라가므로
**데칼 셰이더에서 반드시 삭제**해야 한다. 남겨두면 중복 선언으로 컴파일이 깨진다.

### 1.4 depth test 가 꺼진 이펙트

`bDepthTest == false`로 저작된 이펙트(`AlphaNoDepth`/`AdditiveNoDepth` 패스)는 지형을 덮고
그려지므로 원래 자를 경계가 없다. 여기에 소프트 페이드를 켜면 씬 깊이를 읽어 알파를 깎기
때문에 "지형 뒤에 숨는" 동작이 새로 생긴다. 막지는 않고 Tool에서 경고 문구만 띄운다.

## 2. G 분할

```text
G01  Shader_EffectV2_Common.hlsli   PS_EFFECT_IN 계약 + 깊이 uniform + PS_EFFECT_V2 페이드
G02  Shader_Effect{Mesh,AnimMesh,Rect,Particle,Trail}V2.hlsl   VS 7개가 vProjPos 채우기
G03  Shader_EffectDecalV2.hlsl      중복 선언 제거 + opt-out 표식
G04  EffectV2_Object.h / .cpp       PARAMS field + Bind_Common 바인딩
G05  EffectV2_Document.cpp          read / write
G06  Effect_Tool_V2.cpp             저작 슬라이더
```

G01~G03은 셰이더만이라 빌드 없이 런타임에서 바로 드러나고, G04~G06이 Client 빌드 대상이다.

---

## G01. Shader_EffectV2_Common.hlsli

### G01-1. 파일 역할

V2 이펙트 6종이 공유하는 uniform 선언, 렌더 상태, `PS_EFFECT_IN`/`PS_EFFECT_OUT` 계약,
그리고 실제 색을 만드는 `PS_EFFECT_V2` 하나를 소유한다. 각 shape 셰이더는 정점 단계만
자기 것으로 갖고 픽셀 단계는 전부 이 파일에 위임한다. 소프트 페이드가 여기 한 곳에만
들어가면 6종 전부에 동시에 적용되는 이유다.

### G01-2. 추가·교체 위치

```text
파일: Client/Bin/ShaderFiles/Shader_EffectV2_Common.hlsli
작업: 추가
기준점: float4 g_OutlineColor = float4(1.f, 1.f, 1.f, 1.f);
위치: 기준점 바로 아래, Texture2D g_BaseTexture; 바로 위
추가할 대상: uniform float g_SoftFadeDistance
필요한 이유: 저작자가 정한 페이드 거리(월드 단위). 0 이면 기능 자체가 꺼진다.
연결되는 부분: CEffectV2Object::Bind_Common 이 Bind_RawValue 로 채운다.
```

```text
파일: Client/Bin/ShaderFiles/Shader_EffectV2_Common.hlsli
작업: 추가
기준점: Texture2D g_DissolveTexture;
위치: 기준점 바로 아래, uint g_HasBase = 0; 바로 위
추가할 대상: Texture2D g_DepthTexture
필요한 이유: Target_Depth SRV. 데칼 셰이더에 있던 같은 선언이 여기로 올라온다.
연결되는 부분: CEffectV2Object::Bind_Common 의 Bind_RT_SRV(TEXT("Target_Depth"), ...)
```

```text
파일: Client/Bin/ShaderFiles/Shader_EffectV2_Common.hlsli
작업: 추가
기준점: struct PS_EFFECT_IN 의 float4 vInstanceColor : COLOR0;
위치: 기준점 바로 아래, 닫는 }; 바로 위
추가할 대상: float4 vProjPos : TEXCOORD2
필요한 이유: 화면 UV(xy/w)와 픽셀 자신의 view-space 깊이(w)를 동시에 얻는 유일한 입력.
연결되는 부분: G02 의 VS 7 개와 G03 의 데칼 PS_MAIN 이 채운다.
```

```text
파일: Client/Bin/ShaderFiles/Shader_EffectV2_Common.hlsli
작업: 추가
기준점: PS_EFFECT_V2 안의 color.a *= lerp(1.f, fresnel, saturate(g_GhostAlpha));
위치: 기준점 바로 아래, if (color.a <= 0.001f) discard; 바로 위
추가할 대상: 소프트 페이드 블록
필요한 이유: 알파를 깎은 뒤 기존 discard 가 완전히 투명해진 픽셀을 그대로 버리게 하려면
            반드시 discard 앞이어야 한다. 뒤에 두면 알파 0 픽셀이 살아남는다.
연결되는 부분: g_DepthTexture, g_SoftFadeDistance, input.vProjPos
```

`PS_OUTLINE_V2`는 건드리지 않는다. 인버티드 헐 아웃라인은 본체 실루엣 바깥에만 남는 별도
계약이고, 여기에 깊이 페이드를 걸면 아웃라인만 따로 사라져 본체와 어긋난다.

### G01-3. 페이드 식

```text
sceneViewZ  = g_DepthTexture.Sample(PointSampler, screenUV).y * 1000.f
pixelViewZ  = input.vProjPos.w
color.a    *= saturate((sceneViewZ - pixelViewZ) / g_SoftFadeDistance)
```

`screenUV`는 `vProjPos.xy / vProjPos.w * float2(0.5, -0.5) + 0.5`다. y 부호가 뒤집히는 것은
NDC의 +y가 위쪽인데 텍스처 v는 아래쪽으로 증가하기 때문이다. `PointSampler`를 쓰는 이유는
깊이는 보간하면 안 되는 값이고 데칼도 같은 샘플러를 쓰기 때문이다.

`sceneViewZ`가 픽셀보다 멀수록(차이가 클수록) 알파가 1로 회복되고, 지형에 닿을수록 0으로
떨어진다. 하늘처럼 아무것도 안 그려진 곳은 clear 값 1.0이라 `1000` 단위가 되어 항상 1이다.

### G01-4. 전체 코드

```hlsl
#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

float g_Time;
float4 g_vCamPosition = float4(0.f, 0.f, 0.f, 1.f);
float4 g_RimColor = float4(1.f, 1.f, 1.f, 1.f);
float g_RimPower = 3.f;
float g_RimIntensity = 0.f;
float g_GhostAlpha = 0.f;
float4 g_ColorMul = float4(1.f, 1.f, 1.f, 1.f);
float4 g_ColorOffset = float4(0.f, 0.f, 0.f, 0.f);
float g_ColorClip = 0.f;
uint g_ColorClipChannel = 1;
float g_BloomIntensity = 1.f;
float g_DistortionIntensity = 0.f;
float2 g_UVStart = float2(0.f, 0.f);
float2 g_UVSpeed = float2(0.f, 0.f);
float2 g_UVTileCount = float2(1.f, 1.f);
float g_NoiseStrength = 0.f;
float g_NoiseScale = 1.f;
float2 g_NoisePan = float2(0.f, 0.f);
float g_DissolveAmount = 0.f;
float g_DissolveSoftness = 0.1f;
float g_OutlineWidth = 0.f;
float4 g_OutlineColor = float4(1.f, 1.f, 1.f, 1.f);
float g_SoftFadeDistance = 0.f;

Texture2D g_BaseTexture;
Texture2D g_NoiseTexture;
Texture2D g_MaskTexture;
Texture2D g_EmissiveTexture;
Texture2D g_DissolveTexture;
Texture2D g_DepthTexture;
uint g_HasBase = 0;
uint g_HasNoise = 0;
uint g_HasMask = 0;
uint g_HasEmissive = 0;
uint g_HasDissolve = 0;

RasterizerState RS_EffectV2
{
	FillMode = Solid;
	CullMode = None;
	FrontCounterClockwise = false;
};


/* Body passes stamp stencil 1 so the inverted-hull outline (stencil != 1)
   only survives outside the silhouette, even when the body itself does not
   write depth (alpha/additive). The stencil buffer is cleared to 0 each frame. */
DepthStencilState DSS_EffectV2_ReadOnlyStamp
{
	DepthEnable = true;
	DepthWriteMask = zero;
	DepthFunc = less_equal;
	StencilEnable = true;
	StencilReadMask = 0xff;
	StencilWriteMask = 0xff;
	FrontFaceStencilFunc = always;
	FrontFaceStencilPass = replace;
	FrontFaceStencilFail = keep;
	FrontFaceStencilDepthFail = keep;
	BackFaceStencilFunc = always;
	BackFaceStencilPass = replace;
	BackFaceStencilFail = keep;
	BackFaceStencilDepthFail = keep;
};

DepthStencilState DSS_EffectV2_NoDepthStamp
{
	DepthEnable = false;
	DepthWriteMask = zero;
	StencilEnable = true;
	StencilReadMask = 0xff;
	StencilWriteMask = 0xff;
	FrontFaceStencilFunc = always;
	FrontFaceStencilPass = replace;
	FrontFaceStencilFail = keep;
	FrontFaceStencilDepthFail = keep;
	BackFaceStencilFunc = always;
	BackFaceStencilPass = replace;
	BackFaceStencilFail = keep;
	BackFaceStencilDepthFail = keep;
};

DepthStencilState DSS_EffectV2_DefaultStamp
{
	DepthEnable = true;
	DepthWriteMask = all;
	DepthFunc = less_equal;
	StencilEnable = true;
	StencilReadMask = 0xff;
	StencilWriteMask = 0xff;
	FrontFaceStencilFunc = always;
	FrontFaceStencilPass = replace;
	FrontFaceStencilFail = keep;
	FrontFaceStencilDepthFail = keep;
	BackFaceStencilFunc = always;
	BackFaceStencilPass = replace;
	BackFaceStencilFail = keep;
	BackFaceStencilDepthFail = keep;
};

DepthStencilState DSS_EffectV2_OutlineTest
{
	DepthEnable = true;
	DepthWriteMask = zero;
	DepthFunc = less_equal;
	StencilEnable = true;
	StencilReadMask = 0xff;
	StencilWriteMask = 0x00;
	FrontFaceStencilFunc = not_equal;
	FrontFaceStencilPass = keep;
	FrontFaceStencilFail = keep;
	FrontFaceStencilDepthFail = keep;
	BackFaceStencilFunc = not_equal;
	BackFaceStencilPass = keep;
	BackFaceStencilFail = keep;
	BackFaceStencilDepthFail = keep;
};

RasterizerState RS_EffectV2_Hull
{
	FillMode = Solid;
	CullMode = Front;
	FrontCounterClockwise = false;
};

BlendState BS_EffectV2Alpha
{
	BlendEnable[0] = true;
	BlendEnable[1] = true;
	SrcBlend[0] = Src_Alpha;
	DestBlend[0] = Inv_Src_Alpha;
	BlendOp[0] = Add;
	SrcBlendAlpha[0] = One;
	DestBlendAlpha[0] = Inv_Src_Alpha;
	BlendOpAlpha[0] = Add;
	SrcBlend[1] = One;
	DestBlend[1] = One;
	BlendOp[1] = Add;
	SrcBlendAlpha[1] = One;
	DestBlendAlpha[1] = One;
	BlendOpAlpha[1] = Add;
	RenderTargetWriteMask[1] = 0x03;
};

BlendState BS_EffectV2Additive
{
	BlendEnable[0] = true;
	BlendEnable[1] = true;
	SrcBlend[0] = Src_Alpha;
	DestBlend[0] = One;
	BlendOp[0] = Add;
	SrcBlendAlpha[0] = One;
	DestBlendAlpha[0] = One;
	BlendOpAlpha[0] = Add;
	SrcBlend[1] = One;
	DestBlend[1] = One;
	BlendOp[1] = Add;
	SrcBlendAlpha[1] = One;
	DestBlendAlpha[1] = One;
	BlendOpAlpha[1] = Add;
	RenderTargetWriteMask[1] = 0x03;
};

BlendState BS_EffectV2Opaque
{
	BlendEnable[0] = false;
	BlendEnable[1] = true;
	SrcBlend[1] = One;
	DestBlend[1] = One;
	BlendOp[1] = Add;
	SrcBlendAlpha[1] = One;
	DestBlendAlpha[1] = One;
	BlendOpAlpha[1] = Add;
	RenderTargetWriteMask[1] = 0x03;
};

struct PS_EFFECT_IN
{
	float4 vPosition : SV_POSITION;
	float2 vTexcoord : TEXCOORD0;
	float3 vWorldNormal : NORMAL;
	float3 vWorldPosition : TEXCOORD1;
	float4 vInstanceColor : COLOR0;
	float4 vProjPos : TEXCOORD2;
};

struct PS_EFFECT_OUT
{
	float4 vSceneColor : SV_TARGET0;
	float4 vDistortion : SV_TARGET1;
};

PS_EFFECT_OUT PS_EFFECT_V2(PS_EFFECT_IN input)
{
	PS_EFFECT_OUT output;
	const float2 uv = input.vTexcoord * g_UVTileCount + g_UVStart + g_UVSpeed * g_Time;

	float fresnel = 0.f;
	if (dot(input.vWorldNormal, input.vWorldNormal) > 0.f)
	{
		const float3 N = normalize(input.vWorldNormal);
		const float3 V = normalize(g_vCamPosition.xyz - input.vWorldPosition);
		fresnel = pow(1.f - saturate(abs(dot(N, V))), max(g_RimPower, 0.01f));
	}

	float2 noise = float2(0.5f, 0.5f);
	float2 warp = float2(0.f, 0.f);
	if (0 != g_HasNoise)
	{
		const float2 noiseUV = uv * g_NoiseScale + g_NoisePan * g_Time;
		noise = g_NoiseTexture.Sample(LinearSampler, noiseUV).rg;
		warp = (noise * 2.f - 1.f) * g_NoiseStrength;
	}
	const float2 baseUV = uv + warp;

	float4 base = float4(1.f, 1.f, 1.f, 1.f);
	if (0 != g_HasBase)
		base = g_BaseTexture.Sample(LinearSampler, baseUV);

	float mask = 1.f;
	if (0 != g_HasMask)
		mask = g_MaskTexture.Sample(LinearSampler, uv).r;

	float dissolve = 1.f;
	if (0 != g_HasDissolve)
	{
		const float threshold = g_DissolveTexture.Sample(LinearSampler, uv).r;
		dissolve = smoothstep(
			g_DissolveAmount - g_DissolveSoftness,
			g_DissolveAmount + g_DissolveSoftness,
			threshold);
	}

	float4 color;
	color.rgb = max(base.rgb * g_ColorMul.rgb * input.vInstanceColor.rgb + g_ColorOffset.rgb,
		float3(0.f, 0.f, 0.f));
	color.a = saturate(base.a * mask * dissolve * g_ColorMul.a * input.vInstanceColor.a +
		g_ColorOffset.a);
	color.a *= lerp(1.f, fresnel, saturate(g_GhostAlpha));
	if (g_SoftFadeDistance > 0.f && input.vProjPos.w > 0.f)
	{
		const float2 screenUV =
			input.vProjPos.xy / input.vProjPos.w * float2(0.5f, -0.5f) + 0.5f;
		const float sceneViewZ =
			g_DepthTexture.Sample(PointSampler, screenUV).y * 1000.f;
		color.a *= saturate((sceneViewZ - input.vProjPos.w) / g_SoftFadeDistance);
	}

	if (color.a <= 0.001f)
		discard;
	const float clipValue = (0 == g_ColorClipChannel) ?
		max(color.r, max(color.g, color.b)) : color.a;
	if (g_ColorClip > 0.f && clipValue <= g_ColorClip)
		discard;

	color.rgb += g_RimColor.rgb * fresnel * g_RimIntensity;
	if (0 != g_HasEmissive)
		color.rgb += g_EmissiveTexture.Sample(LinearSampler, baseUV).rgb * g_BloomIntensity;

	const float2 distortion = (0 != g_HasNoise) ?
		(noise * 2.f - 1.f) * g_DistortionIntensity * color.a : float2(0.f, 0.f);

	output.vSceneColor = color;
	output.vDistortion = float4(distortion, 0.f, 0.f);
	return output;
}


/* Inverted-hull outline. Follows the body's dissolve so an eroding mesh
   sheds its outline in the same places. */
PS_EFFECT_OUT PS_OUTLINE_V2(PS_EFFECT_IN input)
{
	PS_EFFECT_OUT output;
	if (0 != g_HasDissolve)
	{
		const float2 uv = input.vTexcoord * g_UVTileCount + g_UVStart + g_UVSpeed * g_Time;
		const float threshold = g_DissolveTexture.Sample(LinearSampler, uv).r;
		const float dissolve = smoothstep(
			g_DissolveAmount - g_DissolveSoftness,
			g_DissolveAmount + g_DissolveSoftness,
			threshold);
		if (dissolve <= 0.001f)
			discard;
	}
	if (g_OutlineColor.a <= 0.001f)
		discard;
	output.vSceneColor = g_OutlineColor;
	output.vDistortion = float4(0.f, 0.f, 0.f, 0.f);
	return output;
}
```

`EFFECT_V2_OUTLINE_PASS`와 `EFFECT_V2_PASSES` 매크로 두 블록은 변경 없이 그대로 둔다.

---

## G02. shape 셰이더 5개의 vProjPos

7개 VS 함수 전부가 이미 아래 한 줄로 클립 공간 위치를 만든다.

```hlsl
output.vPosition = mul(mul(worldPosition, g_ViewMatrix), g_ProjMatrix);
```

따라서 각 함수에서 그 줄 바로 아래에 `output.vProjPos = output.vPosition;` 한 줄만 넣는다.
새로 계산할 것이 없다.

```text
파일: Client/Bin/ShaderFiles/Shader_EffectMeshV2.hlsl        VS_MAIN, VS_OUTLINE
파일: Client/Bin/ShaderFiles/Shader_EffectAnimMeshV2.hlsl    VS_MAIN, VS_OUTLINE
파일: Client/Bin/ShaderFiles/Shader_EffectRectV2.hlsl        VS_MAIN
파일: Client/Bin/ShaderFiles/Shader_EffectParticleV2.hlsl    VS_MAIN
파일: Client/Bin/ShaderFiles/Shader_EffectTrailV2.hlsl       VS_MAIN
작업: 추가
기준점: 각 함수의 output.vPosition = mul(mul(worldPosition, g_ViewMatrix), g_ProjMatrix);
위치: 기준점 바로 아래
추가할 대상: output.vProjPos = output.vPosition;
필요한 이유: PS_EFFECT_IN 이 새 필드를 요구한다. 하나라도 빠지면 컴파일이 깨진다.
연결되는 부분: PS_EFFECT_V2 의 소프트 페이드 블록
```

`Shader_EffectMeshV2.hlsl` 전체 코드:

```hlsl
#include "Shader_EffectV2_Common.hlsli"

struct VS_IN
{
	float3 vPosition : POSITION;
	float3 vNormal : NORMAL;
	float3 vTangent : TANGENT;
	float3 vBinormal : BINORMAL;
	float2 vTexcoord : TEXCOORD0;
};

PS_EFFECT_IN VS_MAIN(VS_IN input)
{
	PS_EFFECT_IN output;
	const float4 worldPosition = mul(float4(input.vPosition, 1.f), g_WorldMatrix);
	output.vPosition = mul(mul(worldPosition, g_ViewMatrix), g_ProjMatrix);
	output.vProjPos = output.vPosition;
	output.vTexcoord = input.vTexcoord;
	output.vWorldNormal = normalize(mul(input.vNormal, (float3x3)g_WorldMatrix));
	output.vWorldPosition = worldPosition.xyz;
	output.vInstanceColor = float4(1.f, 1.f, 1.f, 1.f);
	return output;
}

PS_EFFECT_IN VS_OUTLINE(VS_IN input)
{
	PS_EFFECT_IN output;
	float4 worldPosition = mul(float4(input.vPosition, 1.f), g_WorldMatrix);
	const float3 worldNormal = normalize(mul(input.vNormal, (float3x3)g_WorldMatrix));
	worldPosition.xyz += worldNormal * g_OutlineWidth;
	output.vPosition = mul(mul(worldPosition, g_ViewMatrix), g_ProjMatrix);
	output.vProjPos = output.vPosition;
	output.vTexcoord = input.vTexcoord;
	output.vWorldNormal = worldNormal;
	output.vWorldPosition = worldPosition.xyz;
	output.vInstanceColor = float4(1.f, 1.f, 1.f, 1.f);
	return output;
}

technique11 DefaultTechnique
{
	EFFECT_V2_PASSES(VS_MAIN)
	EFFECT_V2_OUTLINE_PASS(VS_OUTLINE)
}
```

`Shader_EffectAnimMeshV2.hlsl` 전체 코드:

```hlsl
#include "Shader_EffectV2_Common.hlsli"

matrix g_BoneMatrices[512];

struct VS_IN
{
	float3 vPosition : POSITION;
	float3 vNormal : NORMAL;
	float3 vTangent : TANGENT;
	float3 vBinormal : BINORMAL;
	float2 vTexcoord : TEXCOORD0;
	uint4 vBlendIndices : BLENDINDEX;
	float4 vBlendWeights : BLENDWEIGHT;
};

PS_EFFECT_IN VS_MAIN(VS_IN input)
{
	PS_EFFECT_IN output;
	const matrix boneMatrix =
		g_BoneMatrices[input.vBlendIndices.x] * input.vBlendWeights.x +
		g_BoneMatrices[input.vBlendIndices.y] * input.vBlendWeights.y +
		g_BoneMatrices[input.vBlendIndices.z] * input.vBlendWeights.z +
		g_BoneMatrices[input.vBlendIndices.w] * input.vBlendWeights.w;
	const float4 skinnedPosition = mul(float4(input.vPosition, 1.f), boneMatrix);
	const float3 skinnedNormal = mul(float4(input.vNormal, 0.f), boneMatrix).xyz;
	const float4 worldPosition = mul(skinnedPosition, g_WorldMatrix);
	output.vPosition = mul(mul(worldPosition, g_ViewMatrix), g_ProjMatrix);
	output.vProjPos = output.vPosition;
	output.vTexcoord = input.vTexcoord;
	output.vWorldNormal = normalize(mul(skinnedNormal, (float3x3)g_WorldMatrix));
	output.vWorldPosition = worldPosition.xyz;
	output.vInstanceColor = float4(1.f, 1.f, 1.f, 1.f);
	return output;
}

PS_EFFECT_IN VS_OUTLINE(VS_IN input)
{
	PS_EFFECT_IN output;
	const matrix boneMatrix =
		g_BoneMatrices[input.vBlendIndices.x] * input.vBlendWeights.x +
		g_BoneMatrices[input.vBlendIndices.y] * input.vBlendWeights.y +
		g_BoneMatrices[input.vBlendIndices.z] * input.vBlendWeights.z +
		g_BoneMatrices[input.vBlendIndices.w] * input.vBlendWeights.w;
	const float4 skinnedPosition = mul(float4(input.vPosition, 1.f), boneMatrix);
	const float3 skinnedNormal = mul(float4(input.vNormal, 0.f), boneMatrix).xyz;
	float4 worldPosition = mul(skinnedPosition, g_WorldMatrix);
	const float3 worldNormal = normalize(mul(skinnedNormal, (float3x3)g_WorldMatrix));
	worldPosition.xyz += worldNormal * g_OutlineWidth;
	output.vPosition = mul(mul(worldPosition, g_ViewMatrix), g_ProjMatrix);
	output.vProjPos = output.vPosition;
	output.vTexcoord = input.vTexcoord;
	output.vWorldNormal = worldNormal;
	output.vWorldPosition = worldPosition.xyz;
	output.vInstanceColor = float4(1.f, 1.f, 1.f, 1.f);
	return output;
}

technique11 DefaultTechnique
{
	EFFECT_V2_PASSES(VS_MAIN)
	EFFECT_V2_OUTLINE_PASS(VS_OUTLINE)
}
```

`Shader_EffectRectV2.hlsl` 전체 코드:

```hlsl
#include "Shader_EffectV2_Common.hlsli"

struct VS_IN
{
	float3 vPosition : POSITION;
	float2 vTexcoord : TEXCOORD0;
};

PS_EFFECT_IN VS_MAIN(VS_IN input)
{
	PS_EFFECT_IN output;
	const float4 worldPosition = mul(float4(input.vPosition, 1.f), g_WorldMatrix);
	output.vPosition = mul(mul(worldPosition, g_ViewMatrix), g_ProjMatrix);
	output.vProjPos = output.vPosition;
	output.vTexcoord = input.vTexcoord;
	output.vWorldNormal = float3(0.f, 0.f, 0.f);
	output.vWorldPosition = worldPosition.xyz;
	output.vInstanceColor = float4(1.f, 1.f, 1.f, 1.f);
	return output;
}

technique11 DefaultTechnique
{
	EFFECT_V2_PASSES(VS_MAIN)
}
```

`Shader_EffectParticleV2.hlsl` 전체 코드:

```hlsl
#include "Shader_EffectV2_Common.hlsli"

struct VS_IN
{
	float3 vPosition : POSITION;
	float2 vTexcoord : TEXCOORD0;
	float4 vRight : WORLD0;
	float4 vUp : WORLD1;
	float4 vLook : WORLD2;
	float4 vTranslation : WORLD3;
	float4 vColor : COLOR0;
	float4 vDynamicParameter : DYNAMIC0;
	float4 vUVTransform : UVTRANSFORM0;
	float4 vUVTransformNext : UVTRANSFORM1;
	float2 vParticleData : PARTICLEDATA0;
};

PS_EFFECT_IN VS_MAIN(VS_IN input)
{
	PS_EFFECT_IN output;
	const float4x4 instanceWorld = float4x4(
		input.vRight, input.vUp, input.vLook, input.vTranslation);
	const float4 worldPosition = mul(float4(input.vPosition, 1.f), instanceWorld);
	output.vPosition = mul(mul(worldPosition, g_ViewMatrix), g_ProjMatrix);
	output.vProjPos = output.vPosition;
	output.vTexcoord = input.vTexcoord * input.vUVTransform.xy + input.vUVTransform.zw;
	output.vWorldNormal = float3(0.f, 0.f, 0.f);
	output.vWorldPosition = worldPosition.xyz;
	output.vInstanceColor = input.vColor;
	return output;
}

technique11 DefaultTechnique
{
	EFFECT_V2_PASSES(VS_MAIN)
}
```

`Shader_EffectTrailV2.hlsl` 전체 코드:

```hlsl
#include "Shader_EffectV2_Common.hlsli"

struct VS_IN
{
	float3 vPosition : POSITION;
	float2 vTexcoord : TEXCOORD0;
	float4 vColor : COLOR0;
	float4 vDynamicParameter : TEXCOORD1;
};

PS_EFFECT_IN VS_MAIN(VS_IN input)
{
	PS_EFFECT_IN output;
	const float4 worldPosition = float4(input.vPosition, 1.f);
	output.vPosition = mul(mul(worldPosition, g_ViewMatrix), g_ProjMatrix);
	output.vProjPos = output.vPosition;
	output.vTexcoord = input.vTexcoord;
	output.vWorldNormal = float3(0.f, 0.f, 0.f);
	output.vWorldPosition = worldPosition.xyz;
	output.vInstanceColor = input.vColor;
	return output;
}

technique11 DefaultTechnique
{
	EFFECT_V2_PASSES(VS_MAIN)
}
```

---

## G03. Shader_EffectDecalV2.hlsl

두 가지를 한다. 중복이 될 `g_DepthTexture` 선언을 지우고, `PS_EFFECT_V2`에 넘기는
`PS_EFFECT_IN`에 opt-out 표식을 채운다.

```text
파일: Client/Bin/ShaderFiles/Shader_EffectDecalV2.hlsl
작업: 삭제
기준점: Texture2D g_DepthTexture;  (float4x4 g_DecalWorldInverse; 바로 아래)
필요한 이유: 같은 이름이 공통 hlsli 로 올라갔다. 남기면 중복 선언 컴파일 오류.
연결되는 부분: g_NormalTexture 선언은 데칼 전용이므로 그대로 둔다.
```

```text
파일: Client/Bin/ShaderFiles/Shader_EffectDecalV2.hlsl
작업: 추가
기준점: PS_MAIN 안의 effectInput.vInstanceColor = float4(1.f, 1.f, 1.f, 1.f);
위치: 기준점 바로 아래, PS_EFFECT_OUT output = PS_EFFECT_V2(effectInput); 바로 위
추가할 대상: effectInput.vProjPos = float4(0.f, 0.f, 0.f, 0.f);
필요한 이유: w == 0 이 소프트 페이드 opt-out 표식. 데칼은 씬 깊이에서 자기 위치를
            역산하므로 페이드를 걸면 알파가 항상 0 이 되어 사라진다.
연결되는 부분: PS_EFFECT_V2 의 input.vProjPos.w > 0.f 조건
```

전체 코드:

```hlsl
#include "Shader_EffectV2_Common.hlsli"

float4x4 g_ViewMatrixInverse;
float4x4 g_ProjMatrixInverse;
float4x4 g_DecalWorldInverse;
Texture2D g_NormalTexture;
float2 g_DecalSize = float2(1.f, 1.f);
float g_DecalDepth = 1.f;
float g_DecalEdgeFade = 0.f;
float3 g_DecalUp = float3(0.f, 1.f, 0.f);
float g_DecalNormalCutoff = -1.f;

struct VS_IN
{
	float3 vPosition : POSITION;
	float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
	float4 vPosition : SV_POSITION;
	float2 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN input)
{
	VS_OUT output;
	output.vPosition = float4(input.vPosition.x * 2.f, input.vPosition.y * 2.f, 0.f, 1.f);
	output.vTexcoord = input.vTexcoord;
	return output;
}

PS_EFFECT_OUT PS_MAIN(VS_OUT input)
{
	const float4 depth = g_DepthTexture.Sample(PointSampler, input.vTexcoord);
	const float viewZ = depth.y * 1000.f;
	float4 worldPosition;
	worldPosition.x = input.vTexcoord.x * 2.f - 1.f;
	worldPosition.y = input.vTexcoord.y * -2.f + 1.f;
	worldPosition.z = depth.x;
	worldPosition.w = 1.f;
	worldPosition *= viewZ;
	worldPosition = mul(worldPosition, g_ProjMatrixInverse);
	worldPosition = mul(worldPosition, g_ViewMatrixInverse);

	const float3 local = mul(worldPosition, g_DecalWorldInverse).xyz;
	const float2 halfSize = max(g_DecalSize, float2(0.0001f, 0.0001f)) * 0.5f;
	const float halfDepth = max(g_DecalDepth, 0.0001f) * 0.5f;
	clip(halfSize.x - abs(local.x));
	clip(halfSize.y - abs(local.z));
	clip(halfDepth - abs(local.y));
	if (g_DecalNormalCutoff > -1.f)
	{
		const float3 sceneNormal =
			g_NormalTexture.Sample(PointSampler, input.vTexcoord).xyz * 2.f - 1.f;
		clip(dot(normalize(sceneNormal), normalize(g_DecalUp)) - g_DecalNormalCutoff);
	}

	PS_EFFECT_IN effectInput;
	effectInput.vPosition = input.vPosition;
	effectInput.vTexcoord = float2(
		local.x / (halfSize.x * 2.f) + 0.5f,
		0.5f - local.z / (halfSize.y * 2.f));
	effectInput.vWorldNormal = float3(0.f, 0.f, 0.f);
	effectInput.vWorldPosition = worldPosition.xyz;
	effectInput.vInstanceColor = float4(1.f, 1.f, 1.f, 1.f);
	effectInput.vProjPos = float4(0.f, 0.f, 0.f, 0.f);
	PS_EFFECT_OUT output = PS_EFFECT_V2(effectInput);

	if (g_DecalEdgeFade > 0.f)
	{
		const float3 normalized = float3(
			abs(local.x) / halfSize.x, abs(local.y) / halfDepth, abs(local.z) / halfSize.y);
		const float edge = 1.f - max(normalized.x, max(normalized.y, normalized.z));
		const float fade = saturate(edge / g_DecalEdgeFade);
		output.vSceneColor.a *= fade;
		output.vDistortion *= fade;
	}
	return output;
}

technique11 DefaultTechnique
{
	pass Alpha
	{
		SetRasterizerState(RS_EffectV2);
		SetDepthStencilState(DSS_ZNone, 0);
		SetBlendState(BS_EffectV2Alpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}
	pass Additive
	{
		SetRasterizerState(RS_EffectV2);
		SetDepthStencilState(DSS_ZNone, 0);
		SetBlendState(BS_EffectV2Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}
}
```

---

## G04. EffectV2_Object.h / EffectV2_Object.cpp

### G04-1. PARAMS field

```text
파일: Client/Public/EffectV2_Object.h
작업: 추가
기준점: struct PARAMS 안의 bool_t bDepthTest = true;
위치: 기준점 바로 아래, f32_t fLifetime = 0.f; 바로 위
추가할 대상: f32_t fSoftFadeDistance = 0.f;
필요한 이유: 깊이 관련 저작 값이라 bDepthTest 옆이 소유 위치로 맞다.
            0 은 "끔"이며 기존 문서 61 개가 그대로 이 기본값을 받는다.
연결되는 부분: Bind_Common, EffectV2_Document 의 read/write, Effect_Tool_V2 슬라이더
```

의미: 이펙트 표면이 뒤쪽 불투명 지오메트리에 이 거리(월드 단위)까지 접근하면 알파가 0으로
떨어진다. 단위는 `Target_Depth`가 담는 view-space W와 같은 월드 단위다. `fMeshPreScale`이
적용된 뒤의 화면 크기와 무관하게 카메라-지형 거리 기준이므로, 큰 이펙트라고 값을 키울
필요는 없다. 실전 범위는 0.1~1.0 정도다.

교체 후 `PARAMS`의 해당 구간:

```cpp
		BLEND_MODE eBlend = BLEND_MODE::ADDITIVE;
		bool_t bBillboard = true;
		bool_t bDepthTest = true;
		f32_t fSoftFadeDistance = 0.f;
		f32_t fLifetime = 0.f;
		bool_t bLoop = true;
```

### G04-2. Bind_Common

```text
파일: Client/Private/EffectV2_Object.cpp
작업: 교체
기준점: Bind_Common 안의 for (size_t iInput = 0u; ... 루프 바로 위
위치: FAILED(pShader->Bind_RawValue("g_DissolveSoftness", ...))) { return E_FAIL; } 블록 전체
추가할 대상: g_SoftFadeDistance 바인딩 + 조건부 Target_Depth SRV 바인딩
필요한 이유: 값이 0 이면 SRV 를 아예 묶지 않아, 깊이 바인딩 실패가 소프트 페이드를 쓰지 않는
            이펙트까지 E_FAIL 로 끌고 가지 않게 한다.
연결되는 부분: Shader_EffectV2_Common.hlsli 의 g_SoftFadeDistance, g_DepthTexture
```

교체 후 블록:

```cpp
	if (FAILED(pShader->Bind_RawValue("g_vCamPosition", &vCameraPosition, sizeof(vCameraPosition))) ||
		FAILED(pShader->Bind_RawValue("g_RimColor", &P.vRimColor, sizeof(P.vRimColor))) ||
		FAILED(pShader->Bind_RawValue("g_RimPower", &P.fRimPower, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_RimIntensity", &P.fRimIntensity, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_GhostAlpha", &P.fGhostAlpha, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_Time", &m_fTime, sizeof(m_fTime))) ||
		FAILED(pShader->Bind_RawValue("g_ColorMul", &vColorMul, sizeof(vColorMul))) ||
		FAILED(pShader->Bind_RawValue("g_ColorOffset", &vColorOffset, sizeof(vColorOffset))) ||
		FAILED(pShader->Bind_RawValue("g_ColorClip", &P.fColorClip, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_ColorClipChannel", &iColorClipChannel, sizeof(iColorClipChannel))) ||
		FAILED(pShader->Bind_RawValue("g_BloomIntensity", &P.fBloomIntensity, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_DistortionIntensity", &P.fDistortionIntensity, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_UVStart", &P.vUVStart, sizeof(P.vUVStart))) ||
		FAILED(pShader->Bind_RawValue("g_UVSpeed", &P.vUVSpeed, sizeof(P.vUVSpeed))) ||
		FAILED(pShader->Bind_RawValue("g_UVTileCount", &P.vUVTileCount, sizeof(P.vUVTileCount))) ||
		FAILED(pShader->Bind_RawValue("g_NoiseStrength", &P.fNoiseStrength, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_NoiseScale", &P.fNoiseScale, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_NoisePan", &P.vNoisePan, sizeof(P.vNoisePan))) ||
		FAILED(pShader->Bind_RawValue("g_DissolveAmount", &fDissolveAmount, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_DissolveSoftness", &P.fDissolveSoftness, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_SoftFadeDistance", &P.fSoftFadeDistance, sizeof(f32_t))))
	{
		return E_FAIL;
	}
	if (0.f < P.fSoftFadeDistance &&
		FAILED(GameInstance.Bind_RT_SRV(TEXT("Target_Depth"), pShader, "g_DepthTexture")))
		return E_FAIL;
```

`Render_Decal`은 자기 몫으로 `Target_Depth`를 이미 묶고 있으므로 건드리지 않는다.
데칼은 `fSoftFadeDistance` 값과 무관하게 `vProjPos.w == 0`으로 페이드에서 빠진다.

---

## G05. EffectV2_Document.cpp

### G05-1. read

```text
파일: Client/Private/EffectV2_Document.cpp
작업: 추가
기준점: !Read_Bool(*pParams, "depthTest", P.bDepthTest, strOutError) ||
위치: 기준점 바로 아래, !Read_Number(*pParams, "lifetime", P.fLifetime, strOutError) || 바로 위
추가할 대상: !Read_Number(*pParams, "softFadeDistance", P.fSoftFadeDistance, strOutError) ||
필요한 이유: Read_Number 는 키가 없으면 기본값 유지 후 true. 기존 문서 61 개가 그대로 통과한다.
연결되는 부분: CEffectV2Object::PARAMS::fSoftFadeDistance
```

```text
파일: Client/Private/EffectV2_Document.cpp
작업: 추가
기준점: if (P.fMeshPreScale <= 0.f || P.fLifetime < 0.f || P.fPlayRate < 0.f) { ... } 블록의 닫는 }
위치: 기준점 바로 아래, if (const DATA_JSON_VALUE* pClip = pParams->Find("animationClip")) 바로 위
추가할 대상: softFadeDistance 음수 거부 블록
필요한 이유: 셰이더 가드가 g_SoftFadeDistance > 0.f 라서 음수는 "끔"으로 흘러가 저작 의도가
            조용히 사라진다. 기존 범위 검사 문구에 끼워 넣으면 오류 메시지가 사실과
            달라지므로 별도 블록으로 둔다.
연결되는 부분: 실패 시 문서 전체를 거부하는 기존 parse 계약
```

추가할 블록:

```cpp
	if (P.fSoftFadeDistance < 0.f)
	{
		strOutError = "params.softFadeDistance must be >= 0.";
		return false;
	}
```

### G05-2. write

```text
파일: Client/Private/EffectV2_Document.cpp
작업: 추가
기준점: Text += std::string("    \"depthTest\": ") + Json_Bool(P.bDepthTest) + ",\n";
위치: 기준점 바로 아래, Text += "    \"lifetime\": " ... 바로 위
추가할 대상: softFadeDistance 직렬화 한 줄
필요한 이유: read 순서와 같은 자리에 둬야 저장 파일을 눈으로 대조하기 쉽다.
연결되는 부분: Data/Effects/V2/Authored/*.effectv2.json
```

```cpp
	Text += "    \"softFadeDistance\": " + Json_Number(P.fSoftFadeDistance) + ",\n";
```

저장 후 기존 문서에 새로 생기는 줄:

```json
    "depthTest": true,
    "softFadeDistance": 0,
    "lifetime": 0,
```

`formatVersion`은 1을 유지한다. 새 키는 optional이고 구버전 Client가 읽어도 `Find`가
`nullptr`을 돌려주는 대신 무시할 뿐이므로 계약이 깨지지 않는다.

---

## G06. Effect_Tool_V2.cpp

```text
파일: Client/Private/Effect_Tool_V2.cpp
작업: 추가
기준점: Blend section 의 if (CEffectV2Object::SHAPE::SPRITE == pPreview->Shape())
        { ... ImGui::Checkbox("Billboard", &P.bBillboard); } 블록의 닫는 }
위치: 기준점 바로 아래, ImGui::SeparatorText("Playback"); 바로 위
추가할 대상: Soft Fade DragFloat 와 depth test 경고 문구
필요한 이유: 깊이 관련 값이므로 Blend section 안이 저작자가 찾는 자리다.
            단 Depth Test 체크박스 바로 아래에 넣으면 안 된다. 그 뒤의 decal 안내 문구와
            Billboard 체크박스가 ImGui::SameLine() 으로 Depth Test 와 같은 줄에 붙는데,
            사이에 위젯을 끼우면 그 SameLine 들이 새 위젯 줄에 달라붙어 기존 배치가 깨진다.
연결되는 부분: P.fSoftFadeDistance -> Save_Document -> effectv2.json
```

추가할 블록:

```cpp
	ImGui::BeginDisabled(CEffectV2Object::SHAPE::DECAL == eShape);
	ImGui::DragFloat("Soft Fade (world units, 0 = off)",
		&P.fSoftFadeDistance, 0.01f, 0.f, 10.f, "%.2f");
	ImGui::EndDisabled();
	if (CEffectV2Object::SHAPE::DECAL == eShape)
		ImGui::TextDisabled("Decals fade through Decal Projection > Edge Fade instead.");
	else if (0.f < P.fSoftFadeDistance && !P.bDepthTest)
		ImGui::TextDisabled(
			"Depth Test is off, but Soft Fade still reads scene depth: "
			"this effect now dims behind geometry.");
```

`eShape`는 같은 함수의 기존 지역 변수이므로 새로 선언하지 않는다.
`DragFloat`의 min 이 0 이라 UI 에서 음수가 들어갈 수 없고, G05-1 의 검사는 손으로 편집한
JSON 을 막는 몫이다.

---

## 3. 검증

### 3.1 자동 검증

```text
Client x64 Debug ClCompile + Build/Link
git diff --check
Data/Effects/V2/Authored/*.effectv2.json 61 개 전부 parse (Effect Tool V2 로드로 확인)
저장 왕복: 임의 문서 1 개 Save -> softFadeDistance 키 생성 확인 -> 재로드 -> 값 보존 확인
```

셰이더는 런타임에 `../Bin/ShaderFiles`에서 컴파일되므로 빌드로는 검증되지 않는다.
`CEffectV2Object::Create` 실패는 modal 없이 `Last_Error()`로 돌아오므로,
Effect Tool V2 에서 6 shape 를 각각 한 번씩 만들어 생성 실패가 없는지 확인해야 컴파일이
통과했다는 증거가 된다. 특히 G02 에서 VS 7 개 중 하나라도 빠지면 그 shape 만 생성 실패한다.

### 3.2 사용자 전용 화면 검증

`AGENTS.md`의 `사용자 전용 화면 검증 경계`에 따라 아래는 사용자가 직접 판정한다.

```text
지형과 교차하는 이펙트에서 절단선이 사라지는지
Soft Fade 0.1 / 0.5 / 1.0 에서 페이드 폭이 눈에 맞는지
Additive 이펙트가 과하게 옅어지지 않는지
데칼이 이전과 동일하게 보이는지 (회귀 확인)
아웃라인이 본체와 어긋나지 않는지
```

### 3.3 미검증으로 남기는 것

```text
제품 V1 경로(Shader_VtxEffect*.hlsl + CEffectDocumentRenderer)는 이번 범위 밖이다.
이펙트끼리 겹치는 경계는 Target_Depth 가 불투명만 담으므로 부드러워지지 않는다.
MSAA 나 해상도 변경 시 Target_Depth 재생성 경로는 이번 변경이 건드리지 않는다.
```

## 4. 반영 순서

```text
1. G01  공통 hlsli          (이 시점에는 VS 가 vProjPos 를 안 채워 컴파일 실패가 정상)
2. G02  shape 셰이더 5 개    (여기서 셰이더 컴파일이 다시 통과)
3. G03  데칼 셰이더          (중복 선언 제거를 빠뜨리면 데칼만 생성 실패)
4. G04  PARAMS + Bind_Common
5. G05  문서 read/write
6. G06  Tool 슬라이더
7. Client x64 Debug 빌드 -> Effect Tool V2 에서 6 shape 생성 -> 저장 왕복 -> 사용자 화면 확인
```

G01 단독 상태에서는 셰이더가 깨져 있는 것이 정상이므로, G01 과 G02 는 한 번에 반영하고
그 뒤에 Client 를 띄우는 것이 확인이 빠르다.

## 5. 커밋 경계

한 커밋에 셰이더 6 개, `EffectV2_Object.h/.cpp`, `EffectV2_Document.cpp`,
`Effect_Tool_V2.cpp`, 그리고 이 PLAN 과 대응 RESULT 를 함께 담는다.
저작 문서(`Data/Effects/V2/Authored/*.effectv2.json`)는 실제로 값을 튜닝한 것만 담고,
Save 왕복으로 `"softFadeDistance": 0` 한 줄만 늘어난 문서는 커밋하지 않는다.
