# 2026-09-05 Effect V2 Rect 공유 + Particle 버퍼 free-list PLAN

브랜치: `feature/effect-v2-buffer-pool` (main `e57ed73a` 이후 분기 예정)
대상 파일: `Client/Public/EffectV2_Object.h`, `Client/Private/EffectV2_Object.cpp` 두 개만 수정한다.
Engine, `EffectV2_Runtime`, Valtan/Npc/ClientReplication, Effect Tool V2는 건드리지 않는다.
새 파일이 없으므로 `.vcxproj`/`.filters` 등록 변경도 없다.

## 문제와 제약

- `CEffectV2Runtime::Spawn`은 group child마다 `Add_GameObject_to_Layer`로 `CEffectV2Object`를 clone하고
  끝나면 `Remove_GameObject_from_Layer`로 지운다. 객체 pool은 없다.
- 현재 `CEffectV2Object::Initialize`가 인스턴스마다 GPU 버퍼를 새로 만든다.
  Sprite/Decal은 `CVIBuffer_Rect::Create`(vertex+index 2회 `CreateBuffer`),
  sprite Particle은 `CVIBuffer_ParticleRect::Create(maxParticles)`(vertex+index+instance 3회).
  `boss.valtan.six.sonic` 한 번 재생이면 child 22개에서 `CreateBuffer` 40~60회가 같은 프레임에 몰린다.
- 메시·셰이더·DDS는 이미 `g_ModelCache`/`g_ShaderCache`/`g_TextureCache`로 process-global 공유다.
  이번 작업은 같은 자리에 rect 공유와 particle 버퍼 free-list를 추가한다.
- Trail 버퍼(`CVIBuffer_DynamicTrail`)와 mesh particle의 `m_pMeshInstanceBuffer`는 이번 범위 밖이다.
  이유: 발탄 sonic/after 그룹에 trail·mesh particle이 없고, 실측 후 같은 패턴으로 확장할 수 있다.
- Engine `CVIBuffer_Rect`/`CVIBuffer_ParticleRect`는 V1 툴 렌더러(`Effect_DocumentRenderer.cpp`)도 쓰므로
  Engine public 헤더와 `Create` 시그니처는 그대로 둔다. free-list는 Client `EffectV2_Object.cpp` 안에 둔다.
- 스레드 규칙: `CEffectV2Object` 생성(`Clone`→`Initialize`)과 소멸은 main thread에서만 일어난다.
  Loader worker의 `Prewarm`은 model/shader/texture만 만지고 rect·particle pool은 만지지 않는다.
  그래서 pool에 lock을 두지 않는다.

## G 구성

| G | 내용 | 종료 증거 |
|---|---|---|
| G01 | Sprite/Decal이 process-global `CVIBuffer_Rect` 하나를 공유 | Debug Core build PASS, Effect Tool V2에서 six.sonic 재생 시 decal 6개·sprite가 이전과 동일하게 보임(사용자 확인) |
| G02 | sprite Particle 인스턴스 버퍼를 capacity bucket free-list에서 대여·반납 | Debug Core build PASS, 같은 그룹 두 번째 재생부터 `CVIBuffer_ParticleRect::Create` 호출 0회(디버거 breakpoint 또는 F1 Profiler spike 감소), 파티클 표시 동일(사용자 확인) |

아래 전체 코드는 G02까지 적용한 최종 상태다. G01만 먼저 적용하려면 `Acquire_ParticleBuffer`,
`Release_ParticleBuffer`, `g_ParticleBufferPool`, `Round_ParticleCapacity`, 소멸자 본문과
`Clear_ResourceCache`의 pool clear를 빼고 particle 분기는 현재 코드를 유지하면 된다.

## H 계약 delta — `Client/Public/EffectV2_Object.h`

```text
파일: Client/Public/EffectV2_Object.h
작업: 추가
기준점: private 구역의 static HRESULT Acquire_Texture(...) 선언
위치: 기준점 바로 아래, void Apply_Transform(); 바로 위
추가할 대상: 함수 선언 3개 (static)
정의 위치: Client/Private/EffectV2_Object.cpp, Acquire_Texture 정의 바로 뒤
필요한 이유: Initialize/Build_ParticleInstances/소멸자가 GPU 버퍼를 직접 Create하지 않고 공유·대여 경로를 타게 한다
연결되는 부분: Initialize(SPRITE/DECAL/PARTICLE 분기), Build_ParticleInstances, ~CEffectV2Object, Clear_ResourceCache
```

- `static HRESULT Acquire_SharedRect(pDevice, pContext, shared_ptr<CVIBuffer_Rect>& OutRect)`
  process-global rect를 한 번만 만들고 모든 Sprite/Decal 인스턴스에 같은 shared_ptr을 준다.
  호출자는 `Initialize`뿐이다. Rect는 렌더 중 상태가 바뀌지 않으므로 공유해도 draw 결과가 같다.
- `static HRESULT Acquire_ParticleBuffer(pDevice, pContext, iRequiredCapacity, shared_ptr<CVIBuffer_ParticleRect>& OutBuffer)`
  요구 capacity를 bucket으로 올림한 뒤 free-list에서 꺼내거나 없으면 새로 만든다.
  이미 충분한 버퍼를 들고 있으면 아무것도 하지 않는다. 호출자는 `Initialize`와 `Build_ParticleInstances`다.
- `static void Release_ParticleBuffer(shared_ptr<CVIBuffer_ParticleRect>& Buffer)`
  `use_count()==1`인 버퍼만 free-list에 되돌리고 인자를 reset한다. 호출자는 소멸자와 `Acquire_ParticleBuffer`(교체 시)다.

기존 멤버 `m_pRect`, `m_pParticleBuffer`는 이미 `shared_ptr`이라 자료형 변경이 없다.
`Clear_ResourceCache`의 주석 계약만 "rect와 pool도 비운다"로 넓어진다.

## CPP 변수와 함수 — `Client/Private/EffectV2_Object.cpp`

### 새 전역(두 번째 anonymous namespace, `g_TextureCache` 바로 아래)

- `g_SharedRect` : Sprite/Decal 공용 unit quad. owner는 이 전역이고 각 인스턴스의 `m_pRect`는 같은 shared_ptr 사본이다.
  수명은 process 전체이며 `Clear_ResourceCache`에서만 reset한다.
- `PARTICLE_POOL_MIN_CAPACITY = 64` : bucket 최소 크기. authored `maxParticles`가 64 미만이어도 64짜리를 쓴다.
- `PARTICLE_POOL_MAX_PER_BUCKET = 64` : bucket당 보관 상한. 넘치면 반납 대신 그냥 파괴해 무한 성장을 막는다.
- `g_ParticleBufferPool` : `map<capacity, vector<shared_ptr<CVIBuffer_ParticleRect>>>`. owner container다.
  vector 안의 버퍼는 아무도 참조하지 않는 유휴 버퍼만 있어야 한다(불변식). 대여 순간 vector에서 빠지고 `m_pParticleBuffer`가 유일 owner가 된다.
- `Round_ParticleCapacity(iRequired)` : 64부터 두 배씩 올려 `iRequired` 이상인 첫 값을 돌려준다. 상한은 `MAX_PARTICLE_CAPACITY(2048)`.
  authored 64/100/128이 모두 128 bucket을 공유하게 만드는 것이 목적이다. `Get_Capacity() >= 요구값` 검사와 `Update_Instances`의 `m_iNumInstances`가 실제 개수를 따로 들고 있어 capacity가 커도 draw 결과는 같다.

### 변경 함수 한 줄 책임과 흐름

- `~CEffectV2Object()` : `= default` → 본문 추가. `Release_ParticleBuffer(m_pParticleBuffer)` 한 줄.
  `Remove_GameObject_from_Layer` 뒤 마지막 shared_ptr가 풀릴 때 호출되므로 이 시점의 `use_count()`는 1이다.
- `Initialize` SPRITE/DECAL 분기 : `CVIBuffer_Rect::Create` → `Acquire_SharedRect(m_pDevice, m_pContext, m_pRect)`. 실패 메시지는 그대로 `"Rect buffer creation failed."`.
- `Initialize` PARTICLE 분기(sprite particle) : `CVIBuffer_ParticleRect::Create(iCapacity)` → `Acquire_ParticleBuffer(..., iCapacity, m_pParticleBuffer)`.
  mesh particle(`strMeshAssetId` 있음) 경로는 변경 없음.
- `Acquire_SharedRect` : `g_SharedRect`가 비어 있으면 한 번 Create, 이후는 사본 반환. 실패 시 `E_FAIL`이고 `g_SharedRect`는 비어 있는 채로 남아 다음 호출이 재시도한다.
- `Acquire_ParticleBuffer` :
  호출자 → `Round_ParticleCapacity` → 이미 `OutBuffer`가 충분하면 `S_OK` → 아니면 기존 버퍼를 `Release_ParticleBuffer`로 반납 → bucket에 유휴 버퍼가 있으면 `back()`을 꺼내 반환 → 없으면 `Create` → 실패 시 `E_FAIL`이고 `OutBuffer`는 비어 있다.
- `Release_ParticleBuffer` :
  `nullptr`이면 return → `use_count()==1`이고 bucket이 상한 미만이면 push → 그 외는 그냥 reset해 파괴. `Get_Capacity()`를 key로 쓰므로 bucket과 실제 capacity가 항상 일치한다.
- `Clear_ResourceCache` : 기존 세 cache clear 뒤 `g_SharedRect.reset()`, `g_ParticleBufferPool.clear()` 추가. 호출자는 현재 없다(기존과 동일).
- `Build_ParticleInstances` : capacity 부족 시 `Create` → `Acquire_ParticleBuffer(..., iMax, m_pParticleBuffer)`. 툴에서 `maxParticles`를 키웠을 때만 타는 경로이고, 작은 버퍼는 반납되고 큰 bucket에서 새로 받는다.

### 유지되는 불변식

- 한 `CVIBuffer_ParticleRect`를 두 `CEffectV2Object`가 동시에 draw하지 않는다. `Release`가 `use_count()==1`을 요구하고 `Acquire`가 vector에서 빼내므로 성립한다.
- `Update_Instances`는 `WRITE_DISCARD`로 매 프레임 전체를 다시 쓰므로 재사용 버퍼에 이전 파티클이 남지 않는다.
- `Restart`/`Seek_ElapsedSeconds`/`Stop_Emission`/`Finish`는 버퍼 소유권을 건드리지 않는다.
- Effect Tool V2가 같은 `CEffectV2Object`를 쓰므로 툴 preview도 자동으로 같은 pool을 탄다. 툴 lane과 product lane의 clock 분리에는 영향이 없다.

## 사용자가 작성할 순서

1. `EffectV2_Object.h`에 static 선언 3개 추가.
2. `EffectV2_Object.cpp` 두 번째 anonymous namespace에 전역 4개와 `Round_ParticleCapacity` 추가.
3. `Acquire_Texture` 정의 뒤에 `Acquire_SharedRect`, `Acquire_ParticleBuffer`, `Release_ParticleBuffer` 정의 추가.
4. 소멸자 본문, `Initialize`의 SPRITE/DECAL·PARTICLE 분기, `Clear_ResourceCache`, `Build_ParticleInstances` 교체.
5. `git diff --check` 후 아래 검증.

## 검증

- 자동: `powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`
  (Core). Engine/Shared/Server/Client 빌드, CSO closure, NetworkProtocol, Character Select live harness, Valtan presentation 계약까지 PASS여야 한다.
  Client 파일만 바뀌므로 `PrepareEngineSdk` 재배포는 없다.
- 수치: Debug Client에서 F1 Profiler를 켜고 Effect Tool V2로 `boss.valtan.six.sonic`을 3회 재생.
  1회차(버퍼 첫 생성)와 2·3회차(pool 재사용) 스폰 프레임 시간을 비교해 RESULT에 적는다.
  `CVIBuffer_ParticleRect::Create`에 breakpoint를 두면 2회차부터 안 걸려야 한다.
- 화면(사용자): six.sonic·six.sonic.after·impact·shout 그룹이 이전과 같은 형태로 보이는지, particle이 두 객체에 겹쳐 그려지지 않는지 확인. 에이전트는 이 판정을 대신하지 않는다.
- 실패 경로: `Acquire_SharedRect` 실패는 기존과 같이 해당 인스턴스 `Initialize` 실패로 격리되고 `s_strLastError`에 남는다. pool 반납 실패(상한 초과)는 버퍼 파괴로 끝나며 오류가 아니다.

## 남는 경계

- Trail 버퍼와 mesh particle 인스턴스 버퍼는 같은 bucket 패턴으로 다음 G에서 확장한다.
- `CEffectV2Object` 자체의 객체 pool(layer 유지 + `Reset(Desc)`)은 이번 실측 결과가 부족할 때만 진행한다.
- `Clear_ResourceCache`는 호출자가 없다. Device 소멸 전에 cache를 비우는 정리 지점은 별도 과제다.

## 전체 코드 — `Client/Public/EffectV2_Object.h`

```cpp
#pragma once

#include "Client_Defines.h"
#include "EffectV2_Target.h"
#include "GameObject.h"
#include "Presentation_Manager.h"
#include "VIBuffer_DynamicTrail.h"
#include "VIBuffer_ParticleRect.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

NS_BEGIN(Engine)
class CModel;
class CShader;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CNpc;
class CValtan;

class CEffectV2Object final : public CGameObject, public Engine::IPresentationProvider
{
public:
	enum class PIVOT_ROTATION : int32_t
	{
		BONE,
		TARGET_YAW,
		WORLD,
		END
	};

	enum class SHAPE : int32_t
	{
		MESH,
		SPRITE,
		PARTICLE,
		DECAL,
		TRAIL,
		SCREEN_POST,
		END
	};

	enum class SCREEN_POST_PROFILE : int32_t
	{
		ZOOM_BLUR,
		RGB_NOISE,
		FILM_NOISE,
		CHROMATIC_ABERRATION,
		END
	};

	enum class BLEND_MODE : int32_t
	{
		ALPHA,
		ADDITIVE,
		SOLID,
		MULTIPLY,
		END
	};

	enum class TEXTURE_INPUT : int32_t
	{
		BASE,
		NOISE,
		MASK,
		EMISSIVE,
		DISSOLVE,
		END
	};

	enum class COLOR_CLIP_CHANNEL : int32_t
	{
		RGB,
		ALPHA,
		END
	};

	enum class PARTICLE_SPAWN_SHAPE : int32_t
	{
		POINT,
		SPHERE,
		RING,
		BOX,
		END
	};

	enum class PARTICLE_VELOCITY_MODE : int32_t
	{
		FIXED,
		OUTWARD,
		CONE,
		END
	};

	enum class PARTICLE_ALIGNMENT : int32_t
	{
		CAMERA,
		VELOCITY,
		HORIZONTAL,
		END
	};

	enum class TRAIL_EDGE_MODE : int32_t
	{
		CENTERLINE_CAMERA,
		CENTERLINE_UP,
		LOCAL_OFFSET,
		END
	};

	struct LERP_FLOAT3 final
	{
		float3_t vStart = { 0.f, 0.f, 0.f };
		float3_t vEnd = { 0.f, 0.f, 0.f };
		bool_t bLerp = false;
		float3_t Evaluate(f32_t fLifeRatio) const;
	};

	struct PARTICLE_PARAMS final
	{
		uint32_t iMaxParticles = 256u;
		f32_t fSpawnRate = 20.f;
		uint32_t iBurstCount = 0u;
		float2_t vLifetime = { 0.5f, 1.f };
		PARTICLE_SPAWN_SHAPE eSpawnShape = PARTICLE_SPAWN_SHAPE::POINT;
		f32_t fSpawnRadius = 0.5f;
		f32_t fSpawnInnerRadius = 0.f;
		float3_t vSpawnExtents = { 0.5f, 0.5f, 0.5f };
		f32_t fSpawnArcDegrees = 360.f;
		PARTICLE_VELOCITY_MODE eVelocityMode = PARTICLE_VELOCITY_MODE::CONE;
		float3_t vVelocityMin = { -0.5f, 1.f, -0.5f };
		float3_t vVelocityMax = { 0.5f, 2.f, 0.5f };
		float2_t vSpeedRange = { 1.f, 2.f };
		f32_t fConeAngleDegrees = 30.f;
		float3_t vAcceleration = { 0.f, -1.f, 0.f };
		f32_t fDrag = 0.f;
		float2_t vSizeStart = { 0.2f, 0.2f };
		float2_t vSizeEnd = { 0.f, 0.f };
		float2_t vRotationRange = { 0.f, 0.f };
		float2_t vSpinRange = { 0.f, 0.f };
		float4_t vColorStart = { 1.f, 1.f, 1.f, 1.f };
		float4_t vColorEnd = { 1.f, 1.f, 1.f, 0.f };
		PARTICLE_ALIGNMENT eAlignment = PARTICLE_ALIGNMENT::CAMERA;
		bool_t bLocalSpace = true;
		uint32_t iTileColumns = 1u;
		uint32_t iTileRows = 1u;
		bool_t bSubUVOverLife = true;
		uint32_t iRandomSeed = 1u;
		/* Mesh particles only (slots.mesh set on a Particle effect): random
		   per-axis start rotation and spin in degrees / degrees per second.
		   vSizeStart.x/vSizeEnd.x become the uniform scale over life. */
		float3_t vMeshRotationMin = { 0.f, 0.f, 0.f };
		float3_t vMeshRotationMax = { 0.f, 0.f, 0.f };
		float3_t vMeshSpinMin = { 0.f, 0.f, 0.f };
		float3_t vMeshSpinMax = { 0.f, 0.f, 0.f };
	};

	struct DECAL_PARAMS final
	{
		float2_t vSize = { 1.f, 1.f };
		f32_t fDepth = 0.5f;
		f32_t fEdgeFade = 0.f;
		f32_t fNormalCutoff = 0.5f;
	};

	struct TRAIL_PARAMS final
	{
		uint32_t iMaxPoints = 64u;
		f32_t fPointLifetime = 0.35f;
		f32_t fSampleInterval = 1.f / 60.f;
		f32_t fMinDistance = 0.01f;
		f32_t fStartWidth = 0.2f;
		f32_t fEndWidth = 0.f;
		f32_t fTilingDistance = 0.f;
		TRAIL_EDGE_MODE eEdgeMode = TRAIL_EDGE_MODE::CENTERLINE_CAMERA;
		float3_t vEdgeOffset = { 0.f, 1.f, 0.f };
		bool_t bFadeWithAge = true;
	};

	struct SCREEN_POST_PARAMS final
	{
		SCREEN_POST_PROFILE eProfile = SCREEN_POST_PROFILE::ZOOM_BLUR;
		f32_t fIntensityStart = 2.f;
		f32_t fIntensityEnd = 0.f;
		bool_t bIntensityLerp = true;
		f32_t fSecondaryIntensity = 0.f;
		f32_t fFrequency = 1.f;
		float4_t vTint = { 1.f, 1.f, 1.f, 1.f };
		uint32_t iRandomSeed = 1u;
	};

	struct PARAMS final
	{
		LERP_FLOAT3 Position;
		LERP_FLOAT3 Rotation;
		LERP_FLOAT3 Scale = { { 1.f, 1.f, 1.f }, { 1.f, 1.f, 1.f }, false };
		LERP_FLOAT3 Velocity;
		float4_t vColorOffset = { 0.f, 0.f, 0.f, 0.f };
		float4_t vColorOffsetEnd = { 0.f, 0.f, 0.f, 0.f };
		bool_t bColorOffsetLerp = false;
		float4_t vColorMul = { 1.f, 1.f, 1.f, 1.f };
		float4_t vColorMulEnd = { 1.f, 1.f, 1.f, 0.f };
		bool_t bColorMulLerp = false;
		COLOR_CLIP_CHANNEL eColorClipChannel = COLOR_CLIP_CHANNEL::ALPHA;
		f32_t fColorClip = 0.f;
		float4_t vRimColor = { 1.f, 1.f, 1.f, 1.f };
		f32_t fRimPower = 3.f;
		f32_t fRimIntensity = 0.f;
		f32_t fGhostAlpha = 0.f;
		f32_t fOutlineWidth = 0.f;
		float4_t vOutlineColor = { 1.f, 1.f, 1.f, 1.f };
		f32_t fBloomIntensity = 1.f;
		f32_t fDistortionIntensity = 0.f;
		float2_t vUVStart = { 0.f, 0.f };
		float2_t vUVSpeed = { 0.f, 0.f };
		float2_t vUVTileCount = { 1.f, 1.f };
		f32_t fNoiseStrength = 0.f;
		f32_t fNoiseScale = 1.f;
		float2_t vNoisePan = { 0.f, 0.f };
		f32_t fDissolveStart = 0.f;
		f32_t fDissolveInEnd = 0.f;
		f32_t fDissolveSoftness = 0.1f;
		bool_t bDissolveWarp = false;
		bool_t bMaskWarp = false;
		f32_t fAlphaInEnd = 0.f;
		f32_t fAlphaOutStart = 1.f;
		BLEND_MODE eBlend = BLEND_MODE::ADDITIVE;
		bool_t bBillboard = true;
		bool_t bDepthTest = true;
		f32_t fSoftFadeDistance = 0.f;
		f32_t fLifetime = 0.f;
		bool_t bLoop = true;
		f32_t fPlayRate = 1.f;
		f32_t fMeshPreScale = 0.01f;
		uint32_t iAnimationIndex = 0u;
		bool_t bAnimationLoop = true;
		bool_t bColorTexturesSRGB = true;
		PARTICLE_PARAMS Particle;
		DECAL_PARAMS Decal;
		TRAIL_PARAMS Trail;
		SCREEN_POST_PARAMS ScreenPost;
	};

	struct PART final
	{
		bool_t bVisible = true;
		std::string strBaseAssetId;
		ComPtr<ID3D11ShaderResourceView> pBaseView;
	};

	struct DESC final : public GAMEOBJECT_DESC
	{
		SHAPE eShape = SHAPE::SPRITE;
		std::string strMeshAssetId;
		std::array<std::string, static_cast<size_t>(TEXTURE_INPUT::END)> TextureAssetIds;
		float4x4_t PivotWorld = {
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f };
		PARAMS Params;
		bool_t bParamsAuthored = false;
	};

private:
	struct PARTICLE final
	{
		float3_t vPosition = { 0.f, 0.f, 0.f };
		float3_t vVelocity = { 0.f, 0.f, 0.f };
		f32_t fAge = 0.f;
		f32_t fLifetime = 1.f;
		f32_t fRotationDegrees = 0.f;
		f32_t fSpinDegrees = 0.f;
		float3_t vMeshRotationDegrees = { 0.f, 0.f, 0.f };
		float3_t vMeshSpinDegrees = { 0.f, 0.f, 0.f };
	};

	struct TRAIL_POINT final
	{
		float3_t vCenter = { 0.f, 0.f, 0.f };
		float3_t vEdge = { 0.f, 0.f, 0.f };
		f32_t fAge = 0.f;
		f32_t fCumulativeDistance = 0.f;
	};

private:
	CEffectV2Object(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CEffectV2Object();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Submit_Presentation() override;
	virtual bool_t Is_PresentationFailureIsolated() const override { return true; }
	virtual Engine::PRESENTATION_FAILURE_SCOPE Get_PresentationFailureScope() const override
	{
		return m_ePresentationFailureScope;
	}
	f32_t ScreenPost_Intensity() const;

	PARAMS& Params() { return m_Params; }
	const DESC& Creation_Desc() const { return m_CreationDesc; }
	float4x4_t& PivotWorld() { return m_PivotWorld; }
	const std::string& Status() const { return m_strStatus; }
	SHAPE Shape() const { return m_eShape; }
	f32_t Time() const { return m_fTime; }
	f32_t Life_Ratio() const;
	f32_t Dissolve_Amount() const;
	f32_t Alpha_Envelope() const;
	bool_t Has_Texture(const TEXTURE_INPUT eInput) const
	{
		return nullptr != m_Textures[static_cast<size_t>(eInput)];
	}
	uint32_t Part_Count() const { return static_cast<uint32_t>(m_Parts.size()); }
	const std::string& Part_Name(uint32_t iIndex) const;
	bool_t& Part_Visible(const uint32_t iIndex) { return m_Parts[iIndex].bVisible; }
	const std::string& Part_BaseAssetId(const uint32_t iIndex) const
	{
		return m_Parts[iIndex].strBaseAssetId;
	}
	HRESULT Set_PartBase(uint32_t iIndex, const std::string& strAssetId);
	HRESULT Reload_ColorTextures();
	bool_t Is_Skinned() const { return m_bSkinned; }
	bool_t Is_MeshParticle() const { return SHAPE::PARTICLE == m_eShape && nullptr != m_pModel; }
	uint32_t Animation_Count() const;
	const char_t* Animation_Name(uint32_t iIndex) const;
	f32_t Animation_DurationSeconds(uint32_t iIndex) const;
	bool_t Animation_Progress(f32_t& fOutSeconds, f32_t& fOutDurationSeconds) const;
	uint32_t Particle_Count() const { return static_cast<uint32_t>(m_Particles.size()); }
	uint32_t Trail_PointCount() const { return static_cast<uint32_t>(m_TrailPoints.size()); }
	bool_t Is_Finished() const { return m_bFinished; }
	void Finish() { m_bFinished = true; }
	/* Deactivate: particle/trail stop spawning and finish when their last
	   element dies; every other shape has nothing to drain and finishes now. */
	void Stop_Emission();
	bool_t Is_Hidden() const { return m_bHidden; }
	void Set_Hidden(const bool_t bHidden) { m_bHidden = bHidden; }
	void Restart();
	/* Deterministic late-occurrence seek used by owner-linked free groups. The
	   argument is real elapsed time; Params().fPlayRate remains authoritative. */
	void Seek_ElapsedSeconds(f32_t fElapsedSeconds);
	static HRESULT Prewarm(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext,
		const DESC& Desc,
		std::string& strOutError);
	static void Clear_ResourceCache();
	void Set_FollowTarget(
		const EFFECT_V2_TARGET& Target,
		std::string strBone,
		PIVOT_ROTATION eRotation);
	void Clear_FollowTarget();
	/* Applied as Local x bone pivot every frame while following, so a group
	   child keeps its offset/yaw on a moving bone. Identity by default. */
	void Set_FollowLocal(const float4x4_t& Local) { m_FollowLocal = Local; }
	bool_t Has_FollowTarget() const { return m_bFollowTarget; }
	static bool_t Resolve_TargetView(
		const EFFECT_V2_TARGET& Target,
		EFFECT_V2_TARGET_VIEW& OutView);
	static bool_t Resolve_TargetPivot(
		const EFFECT_V2_TARGET_VIEW& View,
		const std::string& strBone,
		PIVOT_ROTATION eRotation,
		float4x4_t& OutPivot);
	static const std::string& Last_Error() { return s_strLastError; }

private:
	HRESULT Load_Texture(
		const std::string& strAssetId,
		bool_t bColorTexture,
		ComPtr<ID3D11ShaderResourceView>& OutView);
	static bool_t Is_ColorInput(TEXTURE_INPUT eInput)
	{
		return TEXTURE_INPUT::BASE == eInput || TEXTURE_INPUT::EMISSIVE == eInput;
	}
	static HRESULT Acquire_Model(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext,
		const std::string& strAssetId,
		shared_ptr<Engine::CModel>& OutModel,
		bool_t& bOutSkinned,
		std::string& strOutError);
	static HRESULT Acquire_Shader(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext,
		const wstring_t& strFilePath,
		const D3D11_INPUT_ELEMENT_DESC* pElements,
		uint32_t iNumElements,
		shared_ptr<Engine::CShader>& OutShader);
	static HRESULT Acquire_ShapeShader(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext,
		SHAPE eShape,
		bool_t bSkinned,
		bool_t bMeshParticle,
		shared_ptr<Engine::CShader>& OutShader,
		std::string& strOutError);
	static HRESULT Acquire_Texture(
		const ComPtr<ID3D11Device>& pDevice,
		const std::string& strAssetId,
		bool_t bSRGB,
		ComPtr<ID3D11ShaderResourceView>& OutView);
	/* Sprite and Decal draw the same unit quad, so one process-global
	   CVIBuffer_Rect is shared by every instance instead of a CreateBuffer
	   pair per spawn. Rendering never mutates the rect. */
	static HRESULT Acquire_SharedRect(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext,
		shared_ptr<Engine::CVIBuffer_Rect>& OutRect);
	/* Sprite-particle instance buffers are checked out of a capacity-bucket
	   free list and returned by the destructor. A checked-out buffer is owned
	   by exactly one CEffectV2Object; Update_Instances rewrites it every frame
	   with WRITE_DISCARD so no stale instance data survives a reuse. */
	static HRESULT Acquire_ParticleBuffer(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext,
		uint32_t iRequiredCapacity,
		shared_ptr<Engine::CVIBuffer_ParticleRect>& OutBuffer);
	static void Release_ParticleBuffer(
		shared_ptr<Engine::CVIBuffer_ParticleRect>& Buffer);
	void Apply_Transform();
	void Sync_Animation(bool_t bRestart);
	HRESULT Bind_Common(const shared_ptr<Engine::CShader>& pShader);
	void Advance_Lifetime(f32_t fStep);
	f32_t Random_01();
	f32_t Random_Range(f32_t fMinimum, f32_t fMaximum);
	void Spawn_Particle();
	void Update_Particles(f32_t fStep);
	HRESULT Build_ParticleInstances();
	HRESULT Upload_MeshParticleInstances();
	void Update_Trail(f32_t fStep);
	HRESULT Build_TrailGeometry();
	HRESULT Render_Decal(uint32_t iPass);

private:
	SHAPE m_eShape = SHAPE::SPRITE;
	PARAMS m_Params;
	DESC m_CreationDesc;
	float4x4_t m_PivotWorld;
	float3_t m_vDisplacement = { 0.f, 0.f, 0.f };
	uint32_t m_iAppliedAnimationIndex = UINT32_MAX;
	std::vector<PART> m_Parts;
	bool_t m_bFollowTarget = false;
	EFFECT_V2_TARGET m_FollowTarget;
	std::string m_strFollowBone;
	PIVOT_ROTATION m_eFollowRotation = PIVOT_ROTATION::TARGET_YAW;
	float4x4_t m_FollowLocal = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f };
	f32_t m_fTime = 0.f;
	bool_t m_bFinished = false;
	bool_t m_bFirstUpdatePending = true;
	bool_t m_bEmissionStopped = false;
	bool_t m_bHidden = false;
	bool_t m_bSkinned = false;
	std::string m_strStatus;
	Engine::PRESENTATION_FAILURE_SCOPE m_ePresentationFailureScope =
		Engine::PRESENTATION_FAILURE_SCOPE::NONE;

	std::vector<PARTICLE> m_Particles;
	std::vector<Engine::VTXEFFECT_PARTICLE> m_ParticleInstances;
	f32_t m_fSpawnAccumulator = 0.f;
	bool_t m_bBurstPending = true;
	uint32_t m_iRandomState = 1u;
	ComPtr<ID3D11Buffer> m_pMeshInstanceBuffer;
	uint32_t m_iMeshInstanceCapacity = 0u;

	std::vector<TRAIL_POINT> m_TrailPoints;
	std::vector<Engine::VTXEFFECT_TRAIL> m_TrailVertices;
	std::vector<uint32_t> m_TrailIndices;
	f32_t m_fTrailSampleAccumulator = 0.f;
	f32_t m_fTrailCumulativeDistance = 0.f;
	uint32_t m_iTrailBufferPoints = 0u;

	shared_ptr<Engine::CShader> m_pShader;
	shared_ptr<Engine::CModel> m_pModel;
	shared_ptr<Engine::CVIBuffer_Rect> m_pRect;
	shared_ptr<Engine::CVIBuffer_ParticleRect> m_pParticleBuffer;
	shared_ptr<Engine::CVIBuffer_DynamicTrail> m_pTrailBuffer;
	std::array<ComPtr<ID3D11ShaderResourceView>,
		static_cast<size_t>(TEXTURE_INPUT::END)> m_Textures;
	static std::string s_strLastError;

public:
	static unique_ptr<CEffectV2Object> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

## 전체 코드 — `Client/Private/EffectV2_Object.cpp`

```cpp
#include "EffectV2_Object.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "RuntimeAssetRoot.h"
#include "Valtan.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"

#include "DirectXTK/DDSTextureLoader.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <map>
#include <span>
#include <unordered_map>

namespace
{
	constexpr const char* TEXTURE_CONSTANTS[] = {
		"g_BaseTexture", "g_NoiseTexture", "g_MaskTexture",
		"g_EmissiveTexture", "g_DissolveTexture"
	};
	constexpr const char* TEXTURE_FLAG_CONSTANTS[] = {
		"g_HasBase", "g_HasNoise", "g_HasMask", "g_HasEmissive", "g_HasDissolve"
	};
	constexpr uint32_t MAX_PARTICLE_CAPACITY = 2048u;
	constexpr uint32_t MAX_TRAIL_POINTS = 4096u;
	constexpr f32_t PI_F = 3.14159265358979f;

	/* VTXMESH per-vertex stream (slot 0) plus the VTXEFFECT_PARTICLE
	   per-instance stream (slot 1) for instanced mesh particles. */
	constexpr D3D11_INPUT_ELEMENT_DESC MESH_PARTICLE_ELEMENTS[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "DYNAMIC", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "UVTRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "UVTRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 112, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "PARTICLEDATA", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 128, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
	constexpr uint32_t MESH_PARTICLE_NUM_ELEMENTS = 14u;

	f32_t Saturate(const f32_t fValue)
	{
		return (std::min)(1.f, (std::max)(0.f, fValue));
	}

	float3_t To_Float3(const vector_t Value)
	{
		float3_t vResult;
		XMStoreFloat3(&vResult, Value);
		return vResult;
	}

	bool_t Normalize_Safe(const vector_t Value, vector_t& OutNormalized)
	{
		const f32_t fLengthSq = XMVectorGetX(XMVector3LengthSq(Value));
		if (fLengthSq <= 1e-12f || !std::isfinite(fLengthSq))
			return false;
		OutNormalized = XMVector3Normalize(Value);
		return true;
	}
}

float3_t Client::CEffectV2Object::LERP_FLOAT3::Evaluate(const f32_t fLifeRatio) const
{
	if (!bLerp)
		return vStart;
	float3_t vResult;
	XMStoreFloat3(&vResult, XMVectorLerp(
		XMLoadFloat3(&vStart), XMLoadFloat3(&vEnd), fLifeRatio));
	return vResult;
}

Client::CEffectV2Object::CEffectV2Object(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject(std::move(pDevice), std::move(pContext))
{
	XMStoreFloat4x4(&m_PivotWorld, XMMatrixIdentity());
}

Client::CEffectV2Object::~CEffectV2Object()
{
	Release_ParticleBuffer(m_pParticleBuffer);
}

HRESULT Client::CEffectV2Object::Initialize_Prototype()
{
	return S_OK;
}

std::string Client::CEffectV2Object::s_strLastError;

HRESULT Client::CEffectV2Object::Initialize(void* pArg)
{
	const auto Fail = [this](std::string strReason)
	{
		m_strStatus = std::move(strReason);
		s_strLastError = m_strStatus;
		return E_FAIL;
	};
	s_strLastError.clear();
	if (nullptr == pArg)
		return Fail("Preview desc is null.");
	if (FAILED(__super::Initialize(pArg)))
		return Fail("Transform component creation failed.");
	const DESC& Desc = *static_cast<const DESC*>(pArg);
	m_CreationDesc = Desc;
	m_eShape = Desc.eShape;
	m_Params = Desc.Params;
	m_PivotWorld = Desc.PivotWorld;

	std::string strError;
	switch (m_eShape)
	{
	case SHAPE::MESH:
		if (FAILED(Acquire_Model(m_pDevice, m_pContext, Desc.strMeshAssetId,
			m_pModel, m_bSkinned, strError)))
			return Fail(strError);
		m_Parts.assign(m_pModel->Get_NumMeshes(), PART{});
		if (m_bSkinned && !Desc.bParamsAuthored)
			m_Params.fMeshPreScale = 0.0001f;
		break;
	case SHAPE::SPRITE:
	case SHAPE::DECAL:
		if (FAILED(Acquire_SharedRect(m_pDevice, m_pContext, m_pRect)))
			return Fail("Rect buffer creation failed.");
		break;
	case SHAPE::PARTICLE:
	{
		const uint32_t iCapacity = (std::min)(MAX_PARTICLE_CAPACITY,
			(std::max)(1u, m_Params.Particle.iMaxParticles));
		if (!Desc.strMeshAssetId.empty())
		{
			if (FAILED(Acquire_Model(m_pDevice, m_pContext, Desc.strMeshAssetId,
				m_pModel, m_bSkinned, strError)))
				return Fail(strError);
			if (m_bSkinned)
				return Fail("Mesh particles need a static (non-skinned) WModel.");
		}
		else if (FAILED(Acquire_ParticleBuffer(
			m_pDevice, m_pContext, iCapacity, m_pParticleBuffer)))
		{
			return Fail("Particle buffer creation failed.");
		}
		m_Particles.reserve(iCapacity);
		m_ParticleInstances.reserve(iCapacity);
		m_iRandomState = (std::max)(1u, m_Params.Particle.iRandomSeed);
		break;
	}
	case SHAPE::TRAIL:
	{
		const uint32_t iMaxPoints = (std::min)(MAX_TRAIL_POINTS,
			(std::max)(2u, m_Params.Trail.iMaxPoints));
		unique_ptr<Engine::CVIBuffer_DynamicTrail> Buffer =
			Engine::CVIBuffer_DynamicTrail::Create(m_pDevice, m_pContext, iMaxPoints);
		if (nullptr == Buffer)
			return Fail("Trail buffer creation failed.");
		m_pTrailBuffer = std::move(Buffer);
		m_iTrailBufferPoints = iMaxPoints;
		break;
	}
	case SHAPE::SCREEN_POST:
		break;
	default:
		return Fail("Unknown effect shape.");
	}
	if (FAILED(Acquire_ShapeShader(m_pDevice, m_pContext, m_eShape, m_bSkinned,
		Is_MeshParticle(), m_pShader, strError)))
		return Fail(strError);

	for (size_t iInput = 0u; iInput < m_Textures.size(); ++iInput)
	{
		const std::string& strAssetId = Desc.TextureAssetIds[iInput];
		if (strAssetId.empty())
			continue;
		if (FAILED(Load_Texture(strAssetId,
			Is_ColorInput(static_cast<TEXTURE_INPUT>(iInput)), m_Textures[iInput])))
			return Fail("Texture load failed: " + strAssetId);
	}
	Sync_Animation(true);
	m_strStatus = "Ready";
	Apply_Transform();
	return S_OK;
}

HRESULT Client::CEffectV2Object::Load_Texture(
	const std::string& strAssetId,
	const bool_t bColorTexture,
	ComPtr<ID3D11ShaderResourceView>& OutView)
{
	return Acquire_Texture(m_pDevice, strAssetId,
		bColorTexture && m_Params.bColorTexturesSRGB, OutView);
}

HRESULT Client::CEffectV2Object::Reload_ColorTextures()
{
	HRESULT hResult = S_OK;
	for (size_t iInput = 0u; iInput < m_Textures.size(); ++iInput)
	{
		if (!Is_ColorInput(static_cast<TEXTURE_INPUT>(iInput)))
			continue;
		const std::string& strAssetId = m_CreationDesc.TextureAssetIds[iInput];
		if (strAssetId.empty())
			continue;
		ComPtr<ID3D11ShaderResourceView> pView;
		if (FAILED(Load_Texture(strAssetId, true, pView)))
		{
			hResult = E_FAIL;
			continue;
		}
		m_Textures[iInput] = std::move(pView);
	}
	for (PART& Part : m_Parts)
	{
		if (Part.strBaseAssetId.empty())
			continue;
		ComPtr<ID3D11ShaderResourceView> pView;
		if (FAILED(Load_Texture(Part.strBaseAssetId, true, pView)))
		{
			hResult = E_FAIL;
			continue;
		}
		Part.pBaseView = std::move(pView);
	}
	return hResult;
}

namespace
{
	struct MODEL_CACHE_ENTRY final
	{
		shared_ptr<Engine::CModel> pPrototype;
		bool_t bSkinned = false;
	};
	std::unordered_map<std::string, MODEL_CACHE_ENTRY> g_ModelCache;
	std::map<wstring_t, shared_ptr<Engine::CShader>> g_ShaderCache;
	std::unordered_map<std::string, ComPtr<ID3D11ShaderResourceView>> g_TextureCache;
	shared_ptr<Engine::CVIBuffer_Rect> g_SharedRect;
	/* Free list keyed by rounded capacity. Only the main thread spawns and
	   destroys CEffectV2Object, so the pool needs no lock; the loader-thread
	   Prewarm path never touches it. */
	constexpr uint32_t PARTICLE_POOL_MIN_CAPACITY = 64u;
	constexpr size_t PARTICLE_POOL_MAX_PER_BUCKET = 64u;
	std::map<uint32_t, std::vector<shared_ptr<Engine::CVIBuffer_ParticleRect>>>
		g_ParticleBufferPool;

	/* Round an authored maxParticles up to a power-of-two bucket so that
	   64/100/128 all reuse the same 128-slot buffers. Capacity only ever
	   grows to MAX_PARTICLE_CAPACITY. */
	uint32_t Round_ParticleCapacity(const uint32_t iRequired)
	{
		uint32_t iCapacity = PARTICLE_POOL_MIN_CAPACITY;
		while (iCapacity < iRequired && iCapacity < MAX_PARTICLE_CAPACITY)
			iCapacity <<= 1u;
		return (std::min)(iCapacity, MAX_PARTICLE_CAPACITY);
	}
}

HRESULT Client::CEffectV2Object::Acquire_Model(
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext,
	const std::string& strAssetId,
	shared_ptr<Engine::CModel>& OutModel,
	bool_t& bOutSkinned,
	std::string& strOutError)
{
	auto Found = g_ModelCache.find(strAssetId);
	if (Found == g_ModelCache.end())
	{
		const std::filesystem::path MeshPath =
			CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
		if (MeshPath.empty() || !std::filesystem::is_regular_file(MeshPath))
		{
			strOutError = "Mesh asset is missing: " + strAssetId;
			return E_FAIL;
		}
		MODEL_CACHE_ENTRY Entry;
		unique_ptr<Engine::CModel> Model = Engine::CModel::Create(
			pDevice, pContext, MODEL::NONANIM,
			MeshPath.string().c_str(), XMMatrixIdentity());
		if (nullptr == Model)
		{
			Model = Engine::CModel::Create(
				pDevice, pContext, MODEL::ANIM,
				MeshPath.string().c_str(), XMMatrixIdentity());
			Entry.bSkinned = nullptr != Model;
		}
		if (nullptr == Model)
		{
			strOutError = "Mesh load failed: " + strAssetId + " | " +
				CModelDecoderRegistry::Get().Get_LastReport().error;
			return E_FAIL;
		}
		Entry.pPrototype = std::move(Model);
		Found = g_ModelCache.emplace(strAssetId, std::move(Entry)).first;
	}
	const shared_ptr<Engine::CModel> pClone =
		std::static_pointer_cast<Engine::CModel>(Found->second.pPrototype->Clone(nullptr));
	if (nullptr == pClone)
	{
		strOutError = "Mesh clone failed: " + strAssetId;
		return E_FAIL;
	}
	OutModel = pClone;
	bOutSkinned = Found->second.bSkinned;
	return S_OK;
}

HRESULT Client::CEffectV2Object::Acquire_Shader(
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext,
	const wstring_t& strFilePath,
	const D3D11_INPUT_ELEMENT_DESC* pElements,
	const uint32_t iNumElements,
	shared_ptr<Engine::CShader>& OutShader)
{
	auto Found = g_ShaderCache.find(strFilePath);
	if (Found == g_ShaderCache.end())
	{
		unique_ptr<Engine::CShader> Shader = Engine::CShader::Create(
			pDevice, pContext, strFilePath.c_str(), pElements, iNumElements);
		if (nullptr == Shader)
			return E_FAIL;
		Found = g_ShaderCache.emplace(strFilePath, std::move(Shader)).first;
	}
	OutShader = Found->second;
	return S_OK;
}

HRESULT Client::CEffectV2Object::Acquire_ShapeShader(
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext,
	const SHAPE eShape,
	const bool_t bSkinned,
	const bool_t bMeshParticle,
	shared_ptr<Engine::CShader>& OutShader,
	std::string& strOutError)
{
	const wchar_t* pFile = nullptr;
	const D3D11_INPUT_ELEMENT_DESC* pElements = nullptr;
	uint32_t iNumElements = 0u;
	if (SHAPE::PARTICLE == eShape && bMeshParticle)
	{
		pFile = TEXT("../Bin/ShaderFiles/Shader_EffectMeshParticleV2.hlsl");
		pElements = MESH_PARTICLE_ELEMENTS;
		iNumElements = MESH_PARTICLE_NUM_ELEMENTS;
		if (FAILED(Acquire_Shader(pDevice, pContext, pFile, pElements, iNumElements, OutShader)))
		{
			strOutError = "Effect v2 shader compile failed: Shader_EffectMeshParticleV2.hlsl";
			return E_FAIL;
		}
		return S_OK;
	}
	switch (eShape)
	{
	case SHAPE::MESH:
		pFile = bSkinned ?
			TEXT("../Bin/ShaderFiles/Shader_EffectAnimMeshV2.hlsl") :
			TEXT("../Bin/ShaderFiles/Shader_EffectMeshV2.hlsl");
		pElements = bSkinned ? VTXANIMMESH::Elements : VTXMESH::Elements;
		iNumElements = bSkinned ? VTXANIMMESH::iNumElements : VTXMESH::iNumElements;
		break;
	case SHAPE::SPRITE:
		pFile = TEXT("../Bin/ShaderFiles/Shader_EffectRectV2.hlsl");
		pElements = VTXTEX::Elements;
		iNumElements = VTXTEX::iNumElements;
		break;
	case SHAPE::PARTICLE:
		pFile = TEXT("../Bin/ShaderFiles/Shader_EffectParticleV2.hlsl");
		pElements = Engine::VTXEFFECT_PARTICLE::Elements;
		iNumElements = Engine::VTXEFFECT_PARTICLE::iNumElements;
		break;
	case SHAPE::DECAL:
		pFile = TEXT("../Bin/ShaderFiles/Shader_EffectDecalV2.hlsl");
		pElements = VTXTEX::Elements;
		iNumElements = VTXTEX::iNumElements;
		break;
	case SHAPE::TRAIL:
		pFile = TEXT("../Bin/ShaderFiles/Shader_EffectTrailV2.hlsl");
		pElements = Engine::VTXEFFECT_TRAIL::Elements;
		iNumElements = Engine::VTXEFFECT_TRAIL::iNumElements;
		break;
	case SHAPE::SCREEN_POST:
		OutShader.reset();
		return S_OK;
	default:
		strOutError = "Unknown effect shape.";
		return E_FAIL;
	}
	if (FAILED(Acquire_Shader(pDevice, pContext, pFile, pElements, iNumElements, OutShader)))
	{
		strOutError = "Effect v2 shader compile failed: " +
			std::filesystem::path(pFile).filename().string();
		return E_FAIL;
	}
	return S_OK;
}

HRESULT Client::CEffectV2Object::Acquire_Texture(
	const ComPtr<ID3D11Device>& pDevice,
	const std::string& strAssetId,
	const bool_t bSRGB,
	ComPtr<ID3D11ShaderResourceView>& OutView)
{
	const std::string strCacheKey = strAssetId + (bSRGB ? "|srgb" : "|linear");
	auto Found = g_TextureCache.find(strCacheKey);
	if (Found == g_TextureCache.end())
	{
		const std::filesystem::path Path =
			CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
		if (Path.empty() || !std::filesystem::is_regular_file(Path))
			return E_FAIL;
		ComPtr<ID3D11ShaderResourceView> pView;
		if (FAILED(DirectX::CreateDDSTextureFromFileEx(
			pDevice.Get(), Path.c_str(), 0u, D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE, 0u, 0u,
			bSRGB ? DirectX::DDS_LOADER_FORCE_SRGB : DirectX::DDS_LOADER_IGNORE_SRGB,
			nullptr, &pView)))
			return E_FAIL;
		Found = g_TextureCache.emplace(strCacheKey, std::move(pView)).first;
	}
	OutView = Found->second;
	return S_OK;
}

HRESULT Client::CEffectV2Object::Acquire_SharedRect(
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext,
	shared_ptr<Engine::CVIBuffer_Rect>& OutRect)
{
	if (nullptr == g_SharedRect)
	{
		unique_ptr<Engine::CVIBuffer_Rect> Rect =
			Engine::CVIBuffer_Rect::Create(pDevice, pContext);
		if (nullptr == Rect)
			return E_FAIL;
		g_SharedRect = std::move(Rect);
	}
	OutRect = g_SharedRect;
	return S_OK;
}

HRESULT Client::CEffectV2Object::Acquire_ParticleBuffer(
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext,
	const uint32_t iRequiredCapacity,
	shared_ptr<Engine::CVIBuffer_ParticleRect>& OutBuffer)
{
	const uint32_t iCapacity = Round_ParticleCapacity(iRequiredCapacity);
	if (nullptr != OutBuffer && OutBuffer->Get_Capacity() >= iCapacity)
		return S_OK;
	Release_ParticleBuffer(OutBuffer);
	std::vector<shared_ptr<Engine::CVIBuffer_ParticleRect>>& Bucket =
		g_ParticleBufferPool[iCapacity];
	if (!Bucket.empty())
	{
		OutBuffer = std::move(Bucket.back());
		Bucket.pop_back();
		return S_OK;
	}
	unique_ptr<Engine::CVIBuffer_ParticleRect> Buffer =
		Engine::CVIBuffer_ParticleRect::Create(pDevice, pContext, iCapacity);
	if (nullptr == Buffer)
		return E_FAIL;
	OutBuffer = std::move(Buffer);
	return S_OK;
}

void Client::CEffectV2Object::Release_ParticleBuffer(
	shared_ptr<Engine::CVIBuffer_ParticleRect>& Buffer)
{
	if (nullptr == Buffer)
		return;
	/* Only the sole owner may hand a buffer back; a buffer that another
	   holder still references would otherwise be drawn by two objects. */
	if (1 == Buffer.use_count())
	{
		std::vector<shared_ptr<Engine::CVIBuffer_ParticleRect>>& Bucket =
			g_ParticleBufferPool[Buffer->Get_Capacity()];
		if (Bucket.size() < PARTICLE_POOL_MAX_PER_BUCKET)
			Bucket.push_back(std::move(Buffer));
	}
	Buffer.reset();
}

HRESULT Client::CEffectV2Object::Prewarm(
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext,
	const DESC& Desc,
	std::string& strOutError)
{
	bool_t bSkinned = false;
	const bool_t bMeshParticle =
		SHAPE::PARTICLE == Desc.eShape && !Desc.strMeshAssetId.empty();
	if (SHAPE::MESH == Desc.eShape || bMeshParticle)
	{
		shared_ptr<Engine::CModel> pModel;
		if (FAILED(Acquire_Model(pDevice, pContext, Desc.strMeshAssetId, pModel, bSkinned, strOutError)))
			return E_FAIL;
		if (bMeshParticle && bSkinned)
		{
			strOutError = "Mesh particles need a static (non-skinned) WModel.";
			return E_FAIL;
		}
	}
	shared_ptr<Engine::CShader> pShader;
	if (FAILED(Acquire_ShapeShader(pDevice, pContext, Desc.eShape, bSkinned, bMeshParticle, pShader, strOutError)))
		return E_FAIL;
	for (size_t iInput = 0u; iInput < Desc.TextureAssetIds.size(); ++iInput)
	{
		const std::string& strAssetId = Desc.TextureAssetIds[iInput];
		if (strAssetId.empty())
			continue;
		ComPtr<ID3D11ShaderResourceView> pView;
		const bool_t bSRGB = Desc.Params.bColorTexturesSRGB &&
			Is_ColorInput(static_cast<TEXTURE_INPUT>(iInput));
		if (FAILED(Acquire_Texture(pDevice, strAssetId, bSRGB, pView)))
		{
			strOutError = "Texture load failed: " + strAssetId;
			return E_FAIL;
		}
	}
	return S_OK;
}

void Client::CEffectV2Object::Clear_ResourceCache()
{
	g_ModelCache.clear();
	g_ShaderCache.clear();
	g_TextureCache.clear();
	g_SharedRect.reset();
	g_ParticleBufferPool.clear();
}

const std::string& Client::CEffectV2Object::Part_Name(const uint32_t iIndex) const
{
	static const std::string strEmpty;
	if (nullptr == m_pModel || iIndex >= m_Parts.size())
		return strEmpty;
	return m_pModel->Get_MaterialName(iIndex);
}

HRESULT Client::CEffectV2Object::Set_PartBase(
	const uint32_t iIndex, const std::string& strAssetId)
{
	if (iIndex >= m_Parts.size())
		return E_INVALIDARG;
	PART& Part = m_Parts[iIndex];
	if (strAssetId.empty())
	{
		Part.strBaseAssetId.clear();
		Part.pBaseView.Reset();
		return S_OK;
	}
	ComPtr<ID3D11ShaderResourceView> pView;
	if (FAILED(Load_Texture(strAssetId, true, pView)))
		return E_FAIL;
	Part.strBaseAssetId = strAssetId;
	Part.pBaseView = std::move(pView);
	return S_OK;
}

void Client::CEffectV2Object::Restart()
{
	m_fTime = 0.f;
	m_vDisplacement = { 0.f, 0.f, 0.f };
	m_bFinished = false;
	m_bEmissionStopped = false;
	m_Particles.clear();
	m_fSpawnAccumulator = 0.f;
	m_bBurstPending = true;
	m_iRandomState = (std::max)(1u, m_Params.Particle.iRandomSeed);
	m_TrailPoints.clear();
	m_fTrailSampleAccumulator = 0.f;
	m_fTrailCumulativeDistance = 0.f;
	Sync_Animation(true);
}

void Client::CEffectV2Object::Seek_ElapsedSeconds(const f32_t fElapsedSeconds)
{
	if (!std::isfinite(fElapsedSeconds) || fElapsedSeconds < 0.f)
		return;
	Restart();
	constexpr f32_t MAX_STEP_SECONDS = 1.f / 60.f;
	f32_t fRemaining = fElapsedSeconds;
	while (fRemaining > 0.f && !m_bFinished)
	{
		const f32_t fStep = (std::min)(MAX_STEP_SECONDS, fRemaining);
		Update(fStep);
		fRemaining -= fStep;
	}
}

uint32_t Client::CEffectV2Object::Animation_Count() const
{
	if (!m_bSkinned || nullptr == m_pModel)
		return 0u;
	return m_pModel->Get_NumAnimations();
}

const char_t* Client::CEffectV2Object::Animation_Name(const uint32_t iIndex) const
{
	if (iIndex >= Animation_Count())
		return nullptr;
	return m_pModel->Get_AnimationName(iIndex);
}

f32_t Client::CEffectV2Object::Animation_DurationSeconds(const uint32_t iIndex) const
{
	if (iIndex >= Animation_Count())
		return 0.f;
	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const f32_t fTickPerSecond = m_pModel->Get_AnimationTickPerSecond(iIndex);
	if (fTickPerSecond <= 0.f ||
		!m_pModel->Get_AnimationProgress(iIndex, fPosition, fDuration))
		return 0.f;
	return fDuration / fTickPerSecond;
}

bool_t Client::CEffectV2Object::Animation_Progress(
	f32_t& fOutSeconds, f32_t& fOutDurationSeconds) const
{
	const uint32_t iIndex = m_Params.iAnimationIndex;
	if (iIndex >= Animation_Count())
		return false;
	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const f32_t fTickPerSecond = m_pModel->Get_AnimationTickPerSecond(iIndex);
	if (fTickPerSecond <= 0.f ||
		!m_pModel->Get_AnimationProgress(iIndex, fPosition, fDuration))
		return false;
	fOutSeconds = fPosition / fTickPerSecond;
	fOutDurationSeconds = fDuration / fTickPerSecond;
	return true;
}

void Client::CEffectV2Object::Sync_Animation(const bool_t bRestart)
{
	const uint32_t iCount = Animation_Count();
	if (0u == iCount)
		return;
	if (m_Params.iAnimationIndex >= iCount)
		m_Params.iAnimationIndex = iCount - 1u;
	const uint32_t iIndex = m_Params.iAnimationIndex;
	if (bRestart || iIndex != m_iAppliedAnimationIndex)
	{
		m_pModel->Start_Animation(iIndex, m_Params.bAnimationLoop);
		m_iAppliedAnimationIndex = iIndex;
		return;
	}
	m_pModel->Set_Animation(iIndex, m_Params.bAnimationLoop);
}

f32_t Client::CEffectV2Object::Life_Ratio() const
{
	if (m_Params.fLifetime <= 0.f)
		return 0.f;
	return Saturate(m_fTime / m_Params.fLifetime);
}

f32_t Client::CEffectV2Object::Dissolve_Amount() const
{
	if (m_Params.fLifetime <= 0.f)
		return 0.f;
	const f32_t fRatio = Life_Ratio();
	const f32_t fInEnd = Saturate(m_Params.fDissolveInEnd);
	if (0.f < fInEnd && fRatio < fInEnd)
		return Saturate(1.f - fRatio / fInEnd);
	const f32_t fStart = (std::max)(Saturate(m_Params.fDissolveStart), fInEnd);
	if (fStart >= 1.f)
		return 0.f;
	return Saturate((fRatio - fStart) / (1.f - fStart));
}

f32_t Client::CEffectV2Object::Alpha_Envelope() const
{
	if (m_Params.fLifetime <= 0.f)
		return 1.f;
	const f32_t fRatio = Life_Ratio();
	f32_t fEnvelope = 1.f;
	const f32_t fInEnd = Saturate(m_Params.fAlphaInEnd);
	if (0.f < fInEnd && fRatio < fInEnd)
		fEnvelope = fRatio / fInEnd;
	const f32_t fOutStart = (std::max)(Saturate(m_Params.fAlphaOutStart), fInEnd);
	if (fOutStart < 1.f && fRatio > fOutStart)
		fEnvelope = (std::min)(fEnvelope, (1.f - fRatio) / (1.f - fOutStart));
	return Saturate(fEnvelope);
}

Client::EFFECT_V2_TARGET Client::EFFECT_V2_TARGET::From_Npc(
	const std::shared_ptr<CNpc>& pNpc)
{
	EFFECT_V2_TARGET Target;
	if (nullptr == pNpc)
		return Target;
	Target.eKind = EFFECT_V2_TARGET_KIND::NPC;
	Target.pOwner = pNpc;
	Target.pKey = pNpc.get();
	return Target;
}

Client::EFFECT_V2_TARGET Client::EFFECT_V2_TARGET::From_Valtan(
	const std::shared_ptr<CValtan>& pValtan)
{
	EFFECT_V2_TARGET Target;
	if (nullptr == pValtan)
		return Target;
	Target.eKind = EFFECT_V2_TARGET_KIND::VALTAN;
	Target.pOwner = pValtan;
	Target.pKey = pValtan.get();
	return Target;
}

void Client::CEffectV2Object::Set_FollowTarget(
	const EFFECT_V2_TARGET& Target,
	std::string strBone,
	const PIVOT_ROTATION eRotation)
{
	m_FollowTarget = Target;
	m_strFollowBone = std::move(strBone);
	m_eFollowRotation = eRotation;
	m_bFollowTarget = m_FollowTarget.Is_Valid();
}

void Client::CEffectV2Object::Clear_FollowTarget()
{
	m_bFollowTarget = false;
	m_FollowTarget.Reset();
	m_strFollowBone.clear();
	XMStoreFloat4x4(&m_FollowLocal, XMMatrixIdentity());
}

bool_t Client::CEffectV2Object::Resolve_TargetView(
	const EFFECT_V2_TARGET& Target,
	EFFECT_V2_TARGET_VIEW& OutView)
{
	const std::shared_ptr<CGameObject> pOwner = Target.pOwner.lock();
	if (nullptr == pOwner)
		return false;
	switch (Target.eKind)
	{
	case EFFECT_V2_TARGET_KIND::NPC:
	{
		const std::shared_ptr<CNpc> pNpc = std::static_pointer_cast<CNpc>(pOwner);
		if (nullptr == pNpc->Get_Model() || nullptr == pNpc->Get_Transform())
			return false;
		OutView.pModel = pNpc->Get_Model();
		OutView.BoneRoot = *pNpc->Get_Transform()->Get_WorldMatrixPtr();
		OutView.YawBasis = OutView.BoneRoot;
		return true;
	}
	case EFFECT_V2_TARGET_KIND::VALTAN:
	{
		const std::shared_ptr<CValtan> pValtan = std::static_pointer_cast<CValtan>(pOwner);
		if (nullptr == pValtan->Get_BodyModel() || nullptr == pValtan->Get_Transform() ||
			!pValtan->Try_Get_PresentationRootMatrix(&OutView.BoneRoot))
		{
			return false;
		}
		OutView.pModel = pValtan->Get_BodyModel();
		OutView.YawBasis = *pValtan->Get_Transform()->Get_WorldMatrixPtr();
		OutView.bHasPortalRushRoute =
			pValtan->Try_Get_PortalRushAnchorMatrices(
				&OutView.PortalRushStart, &OutView.PortalRushEnd);
		return true;
	}
	default:
		return false;
	}
}

bool_t Client::CEffectV2Object::Resolve_TargetPivot(
	const EFFECT_V2_TARGET_VIEW& View,
	const std::string& strBone,
	const PIVOT_ROTATION eRotation,
	float4x4_t& OutPivot)
{
	if (nullptr == View.pModel)
		return false;
	const matrix_t BoneRoot = XMLoadFloat4x4(&View.BoneRoot);
	matrix_t Pivot = BoneRoot;
	bool_t bPortalRushVirtualAnchor = false;
	if ("portal.rush.start" == strBone || "portal.rush.end" == strBone)
	{
		if (!View.bHasPortalRushRoute)
			return false;
		Pivot = XMLoadFloat4x4(
			"portal.rush.start" == strBone ?
				&View.PortalRushStart : &View.PortalRushEnd);
		bPortalRushVirtualAnchor = true;
	}
	else if (!strBone.empty())
	{
		if (!View.pModel->Has_Bone(strBone.c_str()))
			return false;
		Pivot = View.pModel->Get_BoneMatrix(strBone.c_str()) * BoneRoot;
	}
	const matrix_t TargetWorld = bPortalRushVirtualAnchor ?
		Pivot : XMLoadFloat4x4(&View.YawBasis);
	const vector_t Translation = XMVectorSetW(Pivot.r[3], 1.f);
	if (PIVOT_ROTATION::BONE == eRotation)
	{
		const vector_t Right = XMVector3Normalize(Pivot.r[0]);
		const vector_t Up = XMVector3Normalize(Pivot.r[1]);
		const vector_t Look = XMVector3Normalize(Pivot.r[2]);
		if (XMVectorGetX(XMVector3LengthSq(Right)) > 0.f &&
			XMVectorGetX(XMVector3LengthSq(Up)) > 0.f &&
			XMVectorGetX(XMVector3LengthSq(Look)) > 0.f)
		{
			Pivot.r[0] = Right;
			Pivot.r[1] = Up;
			Pivot.r[2] = Look;
		}
		else
			Pivot = XMMatrixIdentity();
	}
	else if (PIVOT_ROTATION::TARGET_YAW == eRotation)
	{
		const vector_t Right = XMVector3Normalize(TargetWorld.r[0]);
		const vector_t Up = XMVector3Normalize(TargetWorld.r[1]);
		const vector_t Look = XMVector3Normalize(TargetWorld.r[2]);
		Pivot.r[0] = Right;
		Pivot.r[1] = Up;
		Pivot.r[2] = Look;
	}
	else
		Pivot = XMMatrixIdentity();
	Pivot.r[3] = Translation;
	XMStoreFloat4x4(&OutPivot, Pivot);
	return true;
}

void Client::CEffectV2Object::Stop_Emission()
{
	if (SHAPE::PARTICLE == m_eShape || SHAPE::TRAIL == m_eShape)
		m_bEmissionStopped = true;
	else
		m_bFinished = true;
}

void Client::CEffectV2Object::Update(const f32_t fTimeDeltaIn)
{
	/* The spawn frame can be one long hitch (deferred decode, texture load).
	   A newly spawned effect must not consume that delta or short-lived
	   elements finish before their first render. */
	f32_t fTimeDelta = fTimeDeltaIn;
	if (m_bFirstUpdatePending)
	{
		m_bFirstUpdatePending = false;
		fTimeDelta = 0.f;
	}
	if (m_bFollowTarget)
	{
		EFFECT_V2_TARGET_VIEW View;
		float4x4_t Pivot;
		if (!Resolve_TargetView(m_FollowTarget, View) ||
			!Resolve_TargetPivot(View, m_strFollowBone, m_eFollowRotation, Pivot))
		{
			m_bFinished = true;
		}
		else
		{
			XMStoreFloat4x4(&m_PivotWorld,
				XMLoadFloat4x4(&m_FollowLocal) * XMLoadFloat4x4(&Pivot));
		}
	}
	if (!m_bFinished)
	{
		const f32_t fStep = fTimeDelta * m_Params.fPlayRate;
		if (!m_bEmissionStopped)
		{
			const float3_t vVelocity = m_Params.Velocity.Evaluate(Life_Ratio());
			m_vDisplacement.x += vVelocity.x * fStep;
			m_vDisplacement.y += vVelocity.y * fStep;
			m_vDisplacement.z += vVelocity.z * fStep;
			m_fTime += fStep;
			Sync_Animation(false);
			if (0u != Animation_Count())
				m_pModel->Play_Animation(fStep);
		}
		Apply_Transform();
		if (SHAPE::PARTICLE == m_eShape)
			Update_Particles(fStep);
		else if (SHAPE::TRAIL == m_eShape)
			Update_Trail(fStep);
		if (!m_bEmissionStopped)
			Advance_Lifetime(fStep);
		else if ((SHAPE::PARTICLE == m_eShape && m_Particles.empty()) ||
			(SHAPE::TRAIL == m_eShape && m_TrailPoints.empty()))
			m_bFinished = true;
	}
	Apply_Transform();
}

void Client::CEffectV2Object::Advance_Lifetime(const f32_t fStep)
{
	UNREFERENCED_PARAMETER(fStep);
	if (m_Params.fLifetime <= 0.f || m_fTime < m_Params.fLifetime)
		return;
	if (m_Params.bLoop)
	{
		m_fTime = std::fmod(m_fTime, m_Params.fLifetime);
		m_vDisplacement = { 0.f, 0.f, 0.f };
		m_bBurstPending = true;
		Sync_Animation(true);
		return;
	}
	if (SHAPE::PARTICLE == m_eShape || SHAPE::TRAIL == m_eShape)
		m_bEmissionStopped = true;
	else
		m_bFinished = true;
}

void Client::CEffectV2Object::Apply_Transform()
{
	const f32_t fRatio = Life_Ratio();
	const float3_t vPosition = m_Params.Position.Evaluate(fRatio);
	const float3_t vRotation = m_Params.Rotation.Evaluate(fRatio);
	float3_t vScale = m_Params.Scale.Evaluate(fRatio);
	/* A zero axis makes the world matrix singular and Engine consumers of
	   its inverse crash, so clamp while the tool types values like "0.4". */
	vScale.x = std::fabs(vScale.x) < 0.001f ? 0.001f : vScale.x;
	vScale.y = std::fabs(vScale.y) < 0.001f ? 0.001f : vScale.y;
	vScale.z = std::fabs(vScale.z) < 0.001f ? 0.001f : vScale.z;
	const f32_t fPreScale =
		SHAPE::MESH == m_eShape ? (std::max)(0.0001f, m_Params.fMeshPreScale) : 1.f;
	const matrix_t Scale = XMMatrixScaling(
		vScale.x * fPreScale, vScale.y * fPreScale, vScale.z * fPreScale);
	const matrix_t Rotation = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(vRotation.x),
		XMConvertToRadians(vRotation.y),
		XMConvertToRadians(vRotation.z));
	const matrix_t LocalTranslation = XMMatrixTranslation(
		vPosition.x + m_vDisplacement.x,
		vPosition.y + m_vDisplacement.y,
		vPosition.z + m_vDisplacement.z);
	const matrix_t Pivot = XMLoadFloat4x4(&m_PivotWorld);
	matrix_t World = Scale * Rotation * LocalTranslation * Pivot;
	if (SHAPE::SPRITE == m_eShape && m_Params.bBillboard)
	{
		const float4x4_t* pCameraWorld =
			CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
		if (nullptr != pCameraWorld)
		{
			matrix_t CameraWorld = XMLoadFloat4x4(pCameraWorld);
			CameraWorld.r[0] = XMVector3Normalize(CameraWorld.r[0]);
			CameraWorld.r[1] = XMVector3Normalize(CameraWorld.r[1]);
			CameraWorld.r[2] = XMVector3Normalize(CameraWorld.r[2]);
			CameraWorld.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
			World = Scale * Rotation * CameraWorld *
				XMMatrixTranslationFromVector(World.r[3]);
		}
	}
	m_pTransformCom->Set_State(STATE::RIGHT, World.r[0]);
	m_pTransformCom->Set_State(STATE::UP, World.r[1]);
	m_pTransformCom->Set_State(STATE::LOOK, World.r[2]);
	m_pTransformCom->Set_State(STATE::POSITION, World.r[3]);
}

f32_t Client::CEffectV2Object::Random_01()
{
	uint32_t iState = m_iRandomState;
	iState ^= iState << 13;
	iState ^= iState >> 17;
	iState ^= iState << 5;
	m_iRandomState = 0u == iState ? 1u : iState;
	return static_cast<f32_t>(m_iRandomState & 0x00FFFFFFu) / 16777216.f;
}

f32_t Client::CEffectV2Object::Random_Range(const f32_t fMinimum, const f32_t fMaximum)
{
	return fMinimum + (fMaximum - fMinimum) * Random_01();
}

void Client::CEffectV2Object::Spawn_Particle()
{
	const PARTICLE_PARAMS& P = m_Params.Particle;
	const uint32_t iMax = (std::min)(MAX_PARTICLE_CAPACITY, (std::max)(1u, P.iMaxParticles));
	if (m_Particles.size() >= iMax)
		return;

	vector_t Position = XMVectorZero();
	switch (P.eSpawnShape)
	{
	case PARTICLE_SPAWN_SHAPE::SPHERE:
	{
		const f32_t fZ = Random_Range(-1.f, 1.f);
		const f32_t fPhi = Random_Range(0.f, 2.f * PI_F);
		const f32_t fRing = std::sqrt((std::max)(0.f, 1.f - fZ * fZ));
		const f32_t fRadius = P.fSpawnInnerRadius +
			(P.fSpawnRadius - P.fSpawnInnerRadius) * std::cbrt(Random_01());
		Position = XMVectorSet(fRing * std::cos(fPhi), fZ, fRing * std::sin(fPhi), 0.f) * fRadius;
		break;
	}
	case PARTICLE_SPAWN_SHAPE::RING:
	{
		const f32_t fAngle = XMConvertToRadians(Random_Range(0.f, P.fSpawnArcDegrees));
		const f32_t fRadius = P.fSpawnInnerRadius +
			(P.fSpawnRadius - P.fSpawnInnerRadius) * std::sqrt(Random_01());
		Position = XMVectorSet(std::cos(fAngle) * fRadius, 0.f, std::sin(fAngle) * fRadius, 0.f);
		break;
	}
	case PARTICLE_SPAWN_SHAPE::BOX:
		Position = XMVectorSet(
			Random_Range(-P.vSpawnExtents.x, P.vSpawnExtents.x),
			Random_Range(-P.vSpawnExtents.y, P.vSpawnExtents.y),
			Random_Range(-P.vSpawnExtents.z, P.vSpawnExtents.z), 0.f);
		break;
	default:
		break;
	}

	vector_t Velocity = XMVectorZero();
	switch (P.eVelocityMode)
	{
	case PARTICLE_VELOCITY_MODE::FIXED:
		Velocity = XMVectorSet(
			Random_Range(P.vVelocityMin.x, P.vVelocityMax.x),
			Random_Range(P.vVelocityMin.y, P.vVelocityMax.y),
			Random_Range(P.vVelocityMin.z, P.vVelocityMax.z), 0.f);
		break;
	case PARTICLE_VELOCITY_MODE::OUTWARD:
	{
		vector_t Direction;
		if (!Normalize_Safe(Position, Direction))
		{
			const f32_t fZ = Random_Range(-1.f, 1.f);
			const f32_t fPhi = Random_Range(0.f, 2.f * PI_F);
			const f32_t fRing = std::sqrt((std::max)(0.f, 1.f - fZ * fZ));
			Direction = XMVectorSet(fRing * std::cos(fPhi), fZ, fRing * std::sin(fPhi), 0.f);
		}
		Velocity = Direction * Random_Range(P.vSpeedRange.x, P.vSpeedRange.y);
		break;
	}
	case PARTICLE_VELOCITY_MODE::CONE:
	{
		const f32_t fCosHalf = std::cos(XMConvertToRadians(
			(std::min)(180.f, (std::max)(0.f, P.fConeAngleDegrees))));
		const f32_t fCosTheta = 1.f + (fCosHalf - 1.f) * Random_01();
		const f32_t fSinTheta = std::sqrt((std::max)(0.f, 1.f - fCosTheta * fCosTheta));
		const f32_t fPhi = Random_Range(0.f, 2.f * PI_F);
		Velocity = XMVectorSet(fSinTheta * std::cos(fPhi), fCosTheta, fSinTheta * std::sin(fPhi), 0.f) *
			Random_Range(P.vSpeedRange.x, P.vSpeedRange.y);
		break;
	}
	default:
		break;
	}

	if (!P.bLocalSpace)
	{
		const matrix_t World = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
		Position = XMVector3TransformCoord(Position, World);
		Velocity = XMVector3TransformNormal(Velocity, World);
	}

	PARTICLE Particle;
	Particle.vPosition = To_Float3(Position);
	Particle.vVelocity = To_Float3(Velocity);
	Particle.fAge = 0.f;
	Particle.fLifetime = (std::max)(0.001f, Random_Range(P.vLifetime.x, P.vLifetime.y));
	Particle.fRotationDegrees = Random_Range(P.vRotationRange.x, P.vRotationRange.y);
	Particle.fSpinDegrees = Random_Range(P.vSpinRange.x, P.vSpinRange.y);
	Particle.vMeshRotationDegrees = {
		Random_Range(P.vMeshRotationMin.x, P.vMeshRotationMax.x),
		Random_Range(P.vMeshRotationMin.y, P.vMeshRotationMax.y),
		Random_Range(P.vMeshRotationMin.z, P.vMeshRotationMax.z) };
	Particle.vMeshSpinDegrees = {
		Random_Range(P.vMeshSpinMin.x, P.vMeshSpinMax.x),
		Random_Range(P.vMeshSpinMin.y, P.vMeshSpinMax.y),
		Random_Range(P.vMeshSpinMin.z, P.vMeshSpinMax.z) };
	m_Particles.push_back(Particle);
}

void Client::CEffectV2Object::Update_Particles(const f32_t fStep)
{
	const PARTICLE_PARAMS& P = m_Params.Particle;
	if (fStep > 0.f)
	{
		for (PARTICLE& Particle : m_Particles)
			Particle.fAge += fStep;
		m_Particles.erase(std::remove_if(m_Particles.begin(), m_Particles.end(),
			[](const PARTICLE& Particle) { return Particle.fAge >= Particle.fLifetime; }),
			m_Particles.end());
		const f32_t fDragFactor = (std::max)(0.f, 1.f - P.fDrag * fStep);
		for (PARTICLE& Particle : m_Particles)
		{
			Particle.vVelocity.x = (Particle.vVelocity.x + P.vAcceleration.x * fStep) * fDragFactor;
			Particle.vVelocity.y = (Particle.vVelocity.y + P.vAcceleration.y * fStep) * fDragFactor;
			Particle.vVelocity.z = (Particle.vVelocity.z + P.vAcceleration.z * fStep) * fDragFactor;
			Particle.vPosition.x += Particle.vVelocity.x * fStep;
			Particle.vPosition.y += Particle.vVelocity.y * fStep;
			Particle.vPosition.z += Particle.vVelocity.z * fStep;
			Particle.fRotationDegrees += Particle.fSpinDegrees * fStep;
			Particle.vMeshRotationDegrees.x += Particle.vMeshSpinDegrees.x * fStep;
			Particle.vMeshRotationDegrees.y += Particle.vMeshSpinDegrees.y * fStep;
			Particle.vMeshRotationDegrees.z += Particle.vMeshSpinDegrees.z * fStep;
		}
	}
	if (m_bEmissionStopped)
		return;
	if (m_bBurstPending)
	{
		for (uint32_t iIndex = 0u; iIndex < P.iBurstCount; ++iIndex)
			Spawn_Particle();
		m_bBurstPending = false;
	}
	m_fSpawnAccumulator += (std::max)(0.f, P.fSpawnRate) * fStep;
	const uint32_t iMax = (std::min)(MAX_PARTICLE_CAPACITY, (std::max)(1u, P.iMaxParticles));
	while (m_fSpawnAccumulator >= 1.f && m_Particles.size() < iMax)
	{
		Spawn_Particle();
		m_fSpawnAccumulator -= 1.f;
	}
	m_fSpawnAccumulator = (std::min)(m_fSpawnAccumulator, 1.f);
}

HRESULT Client::CEffectV2Object::Build_ParticleInstances()
{
	const PARTICLE_PARAMS& P = m_Params.Particle;
	const uint32_t iMax = (std::min)(MAX_PARTICLE_CAPACITY, (std::max)(1u, P.iMaxParticles));
	const bool_t bMeshParticle = Is_MeshParticle();
	if (!bMeshParticle &&
		FAILED(Acquire_ParticleBuffer(m_pDevice, m_pContext, iMax, m_pParticleBuffer)))
	{
		return E_FAIL;
	}
	const float4x4_t* pCameraWorld = CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
	if (nullptr == pCameraWorld)
		return E_FAIL;
	const matrix_t CameraWorld = XMLoadFloat4x4(pCameraWorld);
	const vector_t CameraRight = XMVector3Normalize(CameraWorld.r[0]);
	const vector_t CameraUp = XMVector3Normalize(CameraWorld.r[1]);
	const vector_t CameraLook = XMVector3Normalize(CameraWorld.r[2]);
	const matrix_t World = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	const uint32_t iColumns = (std::max)(1u, P.iTileColumns);
	const uint32_t iRows = (std::max)(1u, P.iTileRows);
	const uint32_t iFrames = iColumns * iRows;

	m_ParticleInstances.clear();
	for (const PARTICLE& Particle : m_Particles)
	{
		const f32_t fLife = Saturate(Particle.fAge / Particle.fLifetime);
		vector_t Position = XMLoadFloat3(&Particle.vPosition);
		vector_t Velocity = XMLoadFloat3(&Particle.vVelocity);
		if (P.bLocalSpace)
		{
			Position = XMVector3TransformCoord(Position, World);
			Velocity = XMVector3TransformNormal(Velocity, World);
		}
		vector_t Right = CameraRight;
		vector_t Up = CameraUp;
		vector_t Look = CameraLook;
		if (PARTICLE_ALIGNMENT::VELOCITY == P.eAlignment)
		{
			const vector_t Planar = Velocity - CameraLook * XMVector3Dot(Velocity, CameraLook);
			vector_t Axis;
			if (Normalize_Safe(Planar, Axis))
			{
				Up = Axis;
				Right = XMVector3Normalize(XMVector3Cross(Up, CameraLook));
			}
		}
		else if (PARTICLE_ALIGNMENT::HORIZONTAL == P.eAlignment)
		{
			Right = XMVectorSet(1.f, 0.f, 0.f, 0.f);
			Up = XMVectorSet(0.f, 0.f, 1.f, 0.f);
			Look = XMVectorSet(0.f, -1.f, 0.f, 0.f);
		}
		const f32_t fRoll = XMConvertToRadians(Particle.fRotationDegrees);
		const f32_t fCos = std::cos(fRoll);
		const f32_t fSin = std::sin(fRoll);
		const vector_t RolledRight = Right * fCos + Up * fSin;
		const vector_t RolledUp = Up * fCos - Right * fSin;
		const f32_t fSizeX = P.vSizeStart.x + (P.vSizeEnd.x - P.vSizeStart.x) * fLife;
		const f32_t fSizeY = P.vSizeStart.y + (P.vSizeEnd.y - P.vSizeStart.y) * fLife;

		Engine::VTXEFFECT_PARTICLE Instance;
		matrix_t InstanceWorld;
		if (bMeshParticle)
		{
			const f32_t fScale = (std::max)(0.f, fSizeX) * m_Params.fMeshPreScale;
			InstanceWorld = XMMatrixScaling(fScale, fScale, fScale) *
				XMMatrixRotationRollPitchYaw(
					XMConvertToRadians(Particle.vMeshRotationDegrees.x),
					XMConvertToRadians(Particle.vMeshRotationDegrees.y),
					XMConvertToRadians(Particle.vMeshRotationDegrees.z)) *
				XMMatrixTranslationFromVector(Position);
		}
		else
		{
			InstanceWorld.r[0] = XMVectorSetW(RolledRight * fSizeX, 0.f);
			InstanceWorld.r[1] = XMVectorSetW(RolledUp * fSizeY, 0.f);
			InstanceWorld.r[2] = XMVectorSetW(Look, 0.f);
			InstanceWorld.r[3] = XMVectorSetW(Position, 1.f);
		}
		XMStoreFloat4x4(&Instance.World, InstanceWorld);
		XMStoreFloat4(&Instance.Color, XMVectorLerp(
			XMLoadFloat4(&P.vColorStart), XMLoadFloat4(&P.vColorEnd), fLife));
		const uint32_t iFrame = P.bSubUVOverLife ?
			(std::min)(iFrames - 1u, static_cast<uint32_t>(fLife * static_cast<f32_t>(iFrames))) : 0u;
		Instance.UVTransform = float4_t(
			1.f / static_cast<f32_t>(iColumns), 1.f / static_cast<f32_t>(iRows),
			static_cast<f32_t>(iFrame % iColumns) / static_cast<f32_t>(iColumns),
			static_cast<f32_t>(iFrame / iColumns) / static_cast<f32_t>(iRows));
		Instance.UVTransformNext = Instance.UVTransform;
		Instance.ParticleData = float2_t(fLife, 0.f);
		m_ParticleInstances.push_back(Instance);
	}
	if (bMeshParticle)
		return Upload_MeshParticleInstances();
	return m_pParticleBuffer->Update_Instances(
		std::span<const Engine::VTXEFFECT_PARTICLE>(m_ParticleInstances));
}

/* Owns the per-instance stream for mesh particles: CModel::Render_Instanced
   binds it on slot 1 next to each mesh's vertex buffer. Grows to the
   authored maximum and is rewritten every frame with WRITE_DISCARD. */
HRESULT Client::CEffectV2Object::Upload_MeshParticleInstances()
{
	const uint32_t iMax = (std::min)(MAX_PARTICLE_CAPACITY,
		(std::max)(1u, m_Params.Particle.iMaxParticles));
	if (nullptr == m_pMeshInstanceBuffer || m_iMeshInstanceCapacity < iMax)
	{
		D3D11_BUFFER_DESC BufferDesc{};
		BufferDesc.ByteWidth = iMax * sizeof(Engine::VTXEFFECT_PARTICLE);
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		ComPtr<ID3D11Buffer> pBuffer;
		if (FAILED(m_pDevice->CreateBuffer(&BufferDesc, nullptr, &pBuffer)))
			return E_FAIL;
		m_pMeshInstanceBuffer = std::move(pBuffer);
		m_iMeshInstanceCapacity = iMax;
	}
	if (m_ParticleInstances.empty())
		return S_OK;
	D3D11_MAPPED_SUBRESOURCE Mapped{};
	if (FAILED(m_pContext->Map(m_pMeshInstanceBuffer.Get(), 0u,
		D3D11_MAP_WRITE_DISCARD, 0u, &Mapped)))
		return E_FAIL;
	const size_t iCount = (std::min)(m_ParticleInstances.size(), static_cast<size_t>(iMax));
	std::memcpy(Mapped.pData, m_ParticleInstances.data(),
		iCount * sizeof(Engine::VTXEFFECT_PARTICLE));
	m_pContext->Unmap(m_pMeshInstanceBuffer.Get(), 0u);
	return S_OK;
}

void Client::CEffectV2Object::Update_Trail(const f32_t fStep)
{
	const TRAIL_PARAMS& T = m_Params.Trail;
	if (fStep > 0.f)
	{
		const f32_t fAgeStep = fStep / (std::max)(0.001f, T.fPointLifetime);
		for (TRAIL_POINT& Point : m_TrailPoints)
			Point.fAge += fAgeStep;
		m_TrailPoints.erase(std::remove_if(m_TrailPoints.begin(), m_TrailPoints.end(),
			[](const TRAIL_POINT& Point) { return Point.fAge >= 1.f; }),
			m_TrailPoints.end());
	}
	if (m_bEmissionStopped)
		return;
	m_fTrailSampleAccumulator += fStep;
	if (m_fTrailSampleAccumulator < T.fSampleInterval && !m_TrailPoints.empty())
		return;
	m_fTrailSampleAccumulator = 0.f;
	const matrix_t World = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	const vector_t Center = XMVectorSetW(World.r[3], 1.f);
	const vector_t Edge = TRAIL_EDGE_MODE::LOCAL_OFFSET == T.eEdgeMode ?
		XMVector3TransformCoord(XMLoadFloat3(&T.vEdgeOffset), World) : Center;
	f32_t fDistance = 0.f;
	if (!m_TrailPoints.empty())
	{
		fDistance = XMVectorGetX(XMVector3Length(
			Center - XMLoadFloat3(&m_TrailPoints.back().vCenter)));
		if (fDistance < T.fMinDistance)
			return;
	}
	m_fTrailCumulativeDistance += fDistance;
	TRAIL_POINT Point;
	Point.vCenter = To_Float3(Center);
	Point.vEdge = To_Float3(Edge);
	Point.fAge = 0.f;
	Point.fCumulativeDistance = m_fTrailCumulativeDistance;
	m_TrailPoints.push_back(Point);
	const uint32_t iMaxPoints = (std::min)(MAX_TRAIL_POINTS, (std::max)(2u, T.iMaxPoints));
	while (m_TrailPoints.size() > iMaxPoints)
		m_TrailPoints.erase(m_TrailPoints.begin());
}

HRESULT Client::CEffectV2Object::Build_TrailGeometry()
{
	const TRAIL_PARAMS& T = m_Params.Trail;
	const uint32_t iMaxPoints = (std::min)(MAX_TRAIL_POINTS, (std::max)(2u, T.iMaxPoints));
	if (m_TrailPoints.size() < 2u)
		return S_FALSE;
	const float4_t* pCameraPosition = CGameInstance::Get().Get_CamPosition();
	const vector_t CameraPosition = nullptr != pCameraPosition ?
		XMLoadFloat4(pCameraPosition) : XMVectorZero();
	const size_t iCount = m_TrailPoints.size();
	m_TrailVertices.clear();
	m_TrailIndices.clear();
	for (size_t iPoint = 0u; iPoint < iCount; ++iPoint)
	{
		const TRAIL_POINT& Point = m_TrailPoints[iPoint];
		const vector_t Center = XMLoadFloat3(&Point.vCenter);
		vector_t First = Center;
		vector_t Second = XMLoadFloat3(&Point.vEdge);
		if (TRAIL_EDGE_MODE::LOCAL_OFFSET != T.eEdgeMode)
		{
			const vector_t Previous = XMLoadFloat3(
				&m_TrailPoints[iPoint > 0u ? iPoint - 1u : iPoint].vCenter);
			const vector_t Next = XMLoadFloat3(
				&m_TrailPoints[iPoint + 1u < iCount ? iPoint + 1u : iPoint].vCenter);
			vector_t Tangent;
			if (!Normalize_Safe(Next - Previous, Tangent))
				continue;
			vector_t Side;
			if (TRAIL_EDGE_MODE::CENTERLINE_CAMERA == T.eEdgeMode)
			{
				if (!Normalize_Safe(XMVector3Cross(CameraPosition - Center, Tangent), Side))
					continue;
			}
			else if (!Normalize_Safe(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), Tangent), Side) &&
				!Normalize_Safe(XMVector3Cross(XMVectorSet(1.f, 0.f, 0.f, 0.f), Tangent), Side))
				continue;
			const f32_t fWidth = T.fStartWidth + (T.fEndWidth - T.fStartWidth) * Saturate(Point.fAge);
			First = Center - Side * (fWidth * 0.5f);
			Second = Center + Side * (fWidth * 0.5f);
		}
		const f32_t fU = T.fTilingDistance > 0.f ?
			Point.fCumulativeDistance / T.fTilingDistance : static_cast<f32_t>(iPoint);
		const f32_t fAlpha = T.bFadeWithAge ? 1.f - Saturate(Point.fAge) : 1.f;
		Engine::VTXEFFECT_TRAIL Vertex;
		Vertex.vColor = float4_t(1.f, 1.f, 1.f, fAlpha);
		Vertex.vPosition = To_Float3(First);
		Vertex.vTexcoord = float2_t(fU, 0.f);
		m_TrailVertices.push_back(Vertex);
		Vertex.vPosition = To_Float3(Second);
		Vertex.vTexcoord = float2_t(fU, 1.f);
		m_TrailVertices.push_back(Vertex);
	}
	if (m_TrailVertices.size() < 4u)
		return S_FALSE;
	for (uint32_t iBase = 0u; iBase + 3u < m_TrailVertices.size(); iBase += 2u)
	{
		m_TrailIndices.push_back(iBase);
		m_TrailIndices.push_back(iBase + 1u);
		m_TrailIndices.push_back(iBase + 2u);
		m_TrailIndices.push_back(iBase + 1u);
		m_TrailIndices.push_back(iBase + 3u);
		m_TrailIndices.push_back(iBase + 2u);
	}
	if (nullptr == m_pTrailBuffer || m_iTrailBufferPoints < iMaxPoints)
	{
		unique_ptr<Engine::CVIBuffer_DynamicTrail> Buffer =
			Engine::CVIBuffer_DynamicTrail::Create(m_pDevice, m_pContext, iMaxPoints);
		if (nullptr == Buffer)
			return E_FAIL;
		m_pTrailBuffer = std::move(Buffer);
		m_iTrailBufferPoints = iMaxPoints;
	}
	return m_pTrailBuffer->Update_Geometry(
		std::span<const Engine::VTXEFFECT_TRAIL>(m_TrailVertices),
		std::span<const uint32_t>(m_TrailIndices));
}

void Client::CEffectV2Object::Late_Update(const f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (m_bHidden || m_bFinished)
		return;
	if (SHAPE::SCREEN_POST == m_eShape)
	{
		if (FAILED(Engine::CPresentation_Manager::Get().Add_FrameProvider(
			static_pointer_cast<Engine::IPresentationProvider>(
				static_pointer_cast<CEffectV2Object>(shared_from_this())))))
		{
			m_strStatus = "Presentation provider budget exceeded.";
		}
		return;
	}
	if (nullptr == m_pShader)
		return;
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::BLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

f32_t Client::CEffectV2Object::ScreenPost_Intensity() const
{
	const SCREEN_POST_PARAMS& S = m_Params.ScreenPost;
	if (!S.bIntensityLerp)
		return (std::max)(0.f, S.fIntensityStart);
	return (std::max)(0.f,
		S.fIntensityStart + (S.fIntensityEnd - S.fIntensityStart) * Life_Ratio());
}

HRESULT Client::CEffectV2Object::Submit_Presentation()
{
	m_ePresentationFailureScope = Engine::PRESENTATION_FAILURE_SCOPE::NONE;
	Engine::CPresentation_Manager& Presentation = Engine::CPresentation_Manager::Get();
	Presentation.Register_ProviderSubmissionExpectation(0u, 0u, 1u, 1u);
	const SCREEN_POST_PARAMS& S = m_Params.ScreenPost;
	Engine::PRESENTATION_SCREEN_POST_DESC Post;
	switch (S.eProfile)
	{
	case SCREEN_POST_PROFILE::RGB_NOISE:
		Post.eProfile = Engine::PRESENTATION_SCREEN_POST_PROFILE::RGB_NOISE_RECONSTRUCTED;
		break;
	case SCREEN_POST_PROFILE::FILM_NOISE:
		Post.eProfile = Engine::PRESENTATION_SCREEN_POST_PROFILE::FILM_NOISE_RECONSTRUCTED;
		break;
	case SCREEN_POST_PROFILE::CHROMATIC_ABERRATION:
		Post.eProfile = Engine::PRESENTATION_SCREEN_POST_PROFILE::CHROMATIC_ABERRATION_RECONSTRUCTED;
		break;
	default:
		Post.eProfile = Engine::PRESENTATION_SCREEN_POST_PROFILE::ZOOM_BLUR_RECONSTRUCTED;
		break;
	}
	Post.iSourceOrder = 0u;
	Post.iRandomSeed = (std::max)(1u, S.iRandomSeed);
	Post.fSampleTimeSeconds = (std::max)(0.f, m_fTime);
	Post.fIntensity = ScreenPost_Intensity();
	Post.fSecondaryIntensity = (std::max)(0.f, S.fSecondaryIntensity);
	Post.fFrequency = (std::max)(0.f, S.fFrequency);
	Post.vTint = S.vTint;
	const HRESULT hResult = Presentation.Add_ScreenPost(Post);
	if (FAILED(hResult))
	{
		m_ePresentationFailureScope = Presentation.Get_LastFailureScope();
		m_strStatus = "Screen post submission failed.";
	}
	return hResult;
}

HRESULT Client::CEffectV2Object::Bind_Common(
	const shared_ptr<Engine::CShader>& pShader)
{
	CGameInstance& GameInstance = CGameInstance::Get();
	if (FAILED(m_pTransformCom->Bind_ShaderResource(pShader, "g_WorldMatrix")) ||
		FAILED(GameInstance.Bind_Transform(pShader, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(GameInstance.Bind_Transform(pShader, "g_ProjMatrix", D3DTS::PROJ)))
	{
		return E_FAIL;
	}
	const PARAMS& P = m_Params;
	const uint32_t iColorClipChannel = static_cast<uint32_t>(P.eColorClipChannel);
	const f32_t fDissolveAmount = Dissolve_Amount();
	const uint32_t iDissolveWarp = P.bDissolveWarp ? 1u : 0u;
	const uint32_t iMaskWarp = P.bMaskWarp ? 1u : 0u;
	const f32_t fLifeRatio = Life_Ratio();
	float4_t vColorMul = P.vColorMul;
	float4_t vColorOffset = P.vColorOffset;
	if (P.bColorMulLerp)
		XMStoreFloat4(&vColorMul, XMVectorLerp(
			XMLoadFloat4(&P.vColorMul), XMLoadFloat4(&P.vColorMulEnd), fLifeRatio));
	if (P.bColorOffsetLerp)
		XMStoreFloat4(&vColorOffset, XMVectorLerp(
			XMLoadFloat4(&P.vColorOffset), XMLoadFloat4(&P.vColorOffsetEnd), fLifeRatio));
	vColorMul.w *= Alpha_Envelope();
	const float4_t* pCameraPosition = GameInstance.Get_CamPosition();
	const float4_t vCameraPosition =
		nullptr != pCameraPosition ? *pCameraPosition : float4_t(0.f, 0.f, 0.f, 1.f);
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
		FAILED(pShader->Bind_RawValue("g_DissolveWarp", &iDissolveWarp, sizeof(iDissolveWarp))) ||
		FAILED(pShader->Bind_RawValue("g_MaskWarp", &iMaskWarp, sizeof(iMaskWarp))) ||
		FAILED(pShader->Bind_RawValue("g_SoftFadeDistance", &P.fSoftFadeDistance, sizeof(f32_t))))
	{
		return E_FAIL;
	}
	if (0.f < P.fSoftFadeDistance &&
		FAILED(GameInstance.Bind_RT_SRV(TEXT("Target_Depth"), pShader, "g_DepthTexture")))
		return E_FAIL;
	for (size_t iInput = 0u; iInput < m_Textures.size(); ++iInput)
	{
		const uint32_t iHas = nullptr != m_Textures[iInput] ? 1u : 0u;
		if (FAILED(pShader->Bind_RawValue(
			TEXTURE_FLAG_CONSTANTS[iInput], &iHas, sizeof(iHas))))
			return E_FAIL;
		if (0u != iHas &&
			FAILED(pShader->Bind_Texture(TEXTURE_CONSTANTS[iInput], m_Textures[iInput])))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT Client::CEffectV2Object::Render_Decal(const uint32_t iPass)
{
	CGameInstance& GameInstance = CGameInstance::Get();
	const float4x4_t* pViewInverse = GameInstance.Get_InverseTransform(D3DTS::VIEW);
	const float4x4_t* pProjInverse = GameInstance.Get_InverseTransform(D3DTS::PROJ);
	if (nullptr == pViewInverse || nullptr == pProjInverse)
		return E_FAIL;
	const matrix_t World = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	vector_t Determinant = XMMatrixDeterminant(World);
	if (std::fabs(XMVectorGetX(Determinant)) <= 1e-12f)
		return E_FAIL;
	float4x4_t WorldInverse;
	XMStoreFloat4x4(&WorldInverse, XMMatrixInverse(&Determinant, World));
	const DECAL_PARAMS& D = m_Params.Decal;
	float3_t vDecalUp;
	XMStoreFloat3(&vDecalUp, XMVector3Normalize(World.r[1]));
	if (FAILED(m_pShader->Bind_Matrix("g_DecalWorldInverse", &WorldInverse)) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInverse", pViewInverse)) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInverse", pProjInverse)) ||
		FAILED(m_pShader->Bind_RawValue("g_DecalSize", &D.vSize, sizeof(D.vSize))) ||
		FAILED(m_pShader->Bind_RawValue("g_DecalDepth", &D.fDepth, sizeof(f32_t))) ||
		FAILED(m_pShader->Bind_RawValue("g_DecalEdgeFade", &D.fEdgeFade, sizeof(f32_t))) ||
		FAILED(m_pShader->Bind_RawValue("g_DecalUp", &vDecalUp, sizeof(vDecalUp))) ||
		FAILED(m_pShader->Bind_RawValue("g_DecalNormalCutoff", &D.fNormalCutoff, sizeof(f32_t))) ||
		FAILED(GameInstance.Bind_RT_SRV(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")) ||
		FAILED(GameInstance.Bind_RT_SRV(TEXT("Target_Normal"), m_pShader, "g_NormalTexture")))
		return E_FAIL;
	const uint32_t iDecalPass = BLEND_MODE::ADDITIVE == m_Params.eBlend ? 1u :
		BLEND_MODE::MULTIPLY == m_Params.eBlend ? 2u : 0u;
	UNREFERENCED_PARAMETER(iPass);
	if (FAILED(m_pShader->Begin(iDecalPass)) ||
		FAILED(m_pRect->Bind_Resources()) ||
		FAILED(m_pRect->Render()))
		return E_FAIL;
	return S_OK;
}

HRESULT Client::CEffectV2Object::Render()
{
	if (SHAPE::SCREEN_POST == m_eShape || nullptr == m_pShader)
		return S_OK;
	if (FAILED(Bind_Common(m_pShader)))
	{
		m_strStatus = "Shader bind failed.";
		return E_FAIL;
	}
	const uint32_t iPass = BLEND_MODE::SOLID == m_Params.eBlend ? 4u :
		BLEND_MODE::MULTIPLY == m_Params.eBlend ? (m_Params.bDepthTest ? 5u : 6u) :
		static_cast<uint32_t>(m_Params.eBlend) + (m_Params.bDepthTest ? 0u : 2u);
	switch (m_eShape)
	{
	case SHAPE::MESH:
		for (uint32_t iMesh = 0u; iMesh < m_pModel->Get_NumMeshes(); ++iMesh)
		{
			if (iMesh < m_Parts.size() && !m_Parts[iMesh].bVisible)
				continue;
			const ComPtr<ID3D11ShaderResourceView>& pBase =
				(iMesh < m_Parts.size() && nullptr != m_Parts[iMesh].pBaseView) ?
				m_Parts[iMesh].pBaseView :
				m_Textures[static_cast<size_t>(TEXTURE_INPUT::BASE)];
			const uint32_t iHasBase = nullptr != pBase ? 1u : 0u;
			if (FAILED(m_pShader->Bind_RawValue("g_HasBase", &iHasBase, sizeof(iHasBase))) ||
				(0u != iHasBase && FAILED(m_pShader->Bind_Texture("g_BaseTexture", pBase))))
			{
				m_strStatus = "Part base bind failed.";
				return E_FAIL;
			}
			if (m_bSkinned && FAILED(m_pModel->Bind_BoneMatrices(
				m_pShader, "g_BoneMatrices", iMesh)))
			{
				m_strStatus = "Bone matrix bind failed.";
				return E_FAIL;
			}
			if (FAILED(m_pShader->Begin(iPass)) || FAILED(m_pModel->Render(iMesh)))
			{
				m_strStatus = "Mesh draw failed.";
				return E_FAIL;
			}
		}
		if (m_Params.fOutlineWidth > 0.f && m_Params.vOutlineColor.w > 0.f)
		{
			if (FAILED(m_pShader->Bind_RawValue("g_OutlineWidth",
					&m_Params.fOutlineWidth, sizeof(f32_t))) ||
				FAILED(m_pShader->Bind_RawValue("g_OutlineColor",
					&m_Params.vOutlineColor, sizeof(m_Params.vOutlineColor))))
			{
				m_strStatus = "Outline bind failed.";
				return E_FAIL;
			}
			for (uint32_t iMesh = 0u; iMesh < m_pModel->Get_NumMeshes(); ++iMesh)
			{
				if (iMesh < m_Parts.size() && !m_Parts[iMesh].bVisible)
					continue;
				if (m_bSkinned && FAILED(m_pModel->Bind_BoneMatrices(
					m_pShader, "g_BoneMatrices", iMesh)))
				{
					m_strStatus = "Bone matrix bind failed.";
					return E_FAIL;
				}
				if (FAILED(m_pShader->Begin(7u)) || FAILED(m_pModel->Render(iMesh)))
				{
					m_strStatus = "Outline draw failed.";
					return E_FAIL;
				}
			}
		}
		return S_OK;
	case SHAPE::SPRITE:
		if (FAILED(m_pShader->Begin(iPass)) ||
			FAILED(m_pRect->Bind_Resources()) ||
			FAILED(m_pRect->Render()))
		{
			m_strStatus = "Sprite draw failed.";
			return E_FAIL;
		}
		return S_OK;
	case SHAPE::PARTICLE:
	{
		if (FAILED(Build_ParticleInstances()))
		{
			m_strStatus = "Particle instance upload failed.";
			return E_FAIL;
		}
		if (Is_MeshParticle())
		{
			const uint32_t iCount = (std::min)(m_iMeshInstanceCapacity,
				static_cast<uint32_t>(m_ParticleInstances.size()));
			if (0u == iCount)
				return S_OK;
			for (uint32_t iMesh = 0u; iMesh < m_pModel->Get_NumMeshes(); ++iMesh)
			{
				if (FAILED(m_pShader->Begin(iPass)) ||
					FAILED(m_pModel->Render_Instanced(iMesh, m_pMeshInstanceBuffer.Get(),
						sizeof(Engine::VTXEFFECT_PARTICLE), iCount)))
				{
					m_strStatus = "Mesh particle draw failed.";
					return E_FAIL;
				}
			}
			return S_OK;
		}
		if (0u == m_pParticleBuffer->Get_InstanceCount())
			return S_OK;
		if (FAILED(m_pShader->Begin(iPass)) || FAILED(m_pParticleBuffer->Render()))
		{
			m_strStatus = "Particle draw failed.";
			return E_FAIL;
		}
		return S_OK;
	}
	case SHAPE::TRAIL:
	{
		const HRESULT hGeometry = Build_TrailGeometry();
		if (FAILED(hGeometry))
		{
			m_strStatus = "Trail geometry upload failed.";
			return E_FAIL;
		}
		if (S_FALSE == hGeometry)
			return S_OK;
		if (FAILED(m_pShader->Begin(iPass)) ||
			FAILED(m_pTrailBuffer->Bind_Resources()) ||
			FAILED(m_pTrailBuffer->Render()))
		{
			m_strStatus = "Trail draw failed.";
			return E_FAIL;
		}
		return S_OK;
	}
	case SHAPE::DECAL:
		if (FAILED(Render_Decal(iPass)))
		{
			m_strStatus = "Decal draw failed.";
			return E_FAIL;
		}
		return S_OK;
	case SHAPE::SCREEN_POST:
		return S_OK;
	default:
		return E_FAIL;
	}
}

unique_ptr<Client::CEffectV2Object> Client::CEffectV2Object::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	unique_ptr<CEffectV2Object> Instance(new CEffectV2Object(
		std::move(pDevice), std::move(pContext)));
	if (FAILED(Instance->Initialize_Prototype()))
		return nullptr;
	return Instance;
}

shared_ptr<CPrototype> Client::CEffectV2Object::Clone(void* pArg)
{
	shared_ptr<CEffectV2Object> Instance(new CEffectV2Object(m_pDevice, m_pContext));
	if (FAILED(Instance->Initialize(pArg)))
		return nullptr;
	return Instance;
}
```
