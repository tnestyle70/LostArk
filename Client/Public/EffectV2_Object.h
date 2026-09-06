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
		float2_t vHueShiftRange = { 0.f, 0.f };
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
		f32_t fScaleInEnd = 0.f;
		f32_t fScaleOutStart = 1.f;
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
		f32_t fHueShiftDegrees = 0.f;
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
	f32_t Scale_Envelope() const;
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
