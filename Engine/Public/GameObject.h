#pragma once

#include "GameInstance.h"
#include <span>

NS_BEGIN(Engine)

class CLayer;
class COcclusionCuller;

class ENGINE_DLL CGameObject abstract : public CPrototype
{
public:
	typedef struct tagGameObjectDesc : public CTransform::TRANSFORM_DESC
	{

	}GAMEOBJECT_DESC;
protected:
	CGameObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CGameObject();

public:
	shared_ptr<CComponent> Get_Component(const wstring_t& strComponentTag);

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta);
	virtual void Update(f32_t fTimeDelta);
	virtual void Post_Physics_Update(f32_t fTimeDelta);
	virtual void Late_Update(f32_t fTimeDelta);
	static constexpr uint8_t UPDATE_PHASE_PRIORITY = 1u;
	static constexpr uint8_t UPDATE_PHASE_UPDATE = 2u;
	static constexpr uint8_t UPDATE_PHASE_POST_PHYSICS = 4u;
	static constexpr uint8_t UPDATE_PHASE_LATE = 8u;
	// Immutable per-class phase membership, selected when a Layer takes ownership.
	virtual uint8_t Get_UpdatePhaseMask() const { return 15u; }
	// Opt-in render preparation after camera/light providers, before any world pass.
	virtual bool_t Uses_FinalCameraSubmission() const { return false; }
	virtual void Submit_FinalCamera() {}
    // Owner-thread staging, then one exclusive CPU-only callback before Submit.
    // The layer joins every callback before GPU submission or any owner mutation.
    struct FINAL_CAMERA_CPU_JOB final
    {
        void* Context = nullptr;
        void (*Execute)(void*) = nullptr;
        uint32_t Cost = 0u;
    };
    virtual bool_t Try_PrepareFinalCameraCpuJob(FINAL_CAMERA_CPU_JOB& output)
    { output = {}; return false; }
    // Opt-in conservative envelope for Layer's final-camera candidate hierarchy.
    // Radius includes all consumer-specific margins; false keeps the callback.
    // Every bounds/policy/visibility change must invalidate before submission.
    struct FINAL_CAMERA_SPATIAL_BOUNDS final
    {
        float3_t Center{};
        f32_t Radius = 0.f;
        uint32_t RejectGraceFrames = 0u;
        bool_t ShadowCaster = false;
    };
    virtual bool_t Try_GetFinalCameraSpatialBounds(FINAL_CAMERA_SPATIAL_BOUNDS& out) const
    { out = {}; return false; }
    // Bounds enclose every vertex of this frame's committed visible submission.
    // Original shadow submissions remain independent from color occlusion.
    struct STATIC_OCCLUSION_DESC final
    {
        float3_t BoundsMin{}, BoundsMax{};
        uint32_t Draws = 0u, OccluderTriangles = 0u;
        // Nonzero revision changes with geometry, visibility, cull state or material eligibility.
        uint64_t Indices = 0u, Revision = 0u;
    };
    virtual bool_t Try_GetStaticOcclusionDesc(STATIC_OCCLUSION_DESC& output) const
    { output = {}; return false; }
    // Only opaque, undeformed, actually rendered geometry may populate depth.
    virtual uint32_t Rasterize_StaticOccluder(COcclusionCuller&, uint32_t) const { return 0u; }
	virtual HRESULT Render();
	virtual HRESULT Render_Group(RENDERGROUP group);
	// Borrow the current queue only; opt-in objects may consume adjacent entries.
	virtual HRESULT Render_AdjacentNonBlend(
		std::span<const std::shared_ptr<CGameObject>> objects, size_t& consumed);
	// Lower values draw first within BLEND; equal values retain distance order.
	virtual int32_t Get_BlendSortPriority() const { return 0; }
	/* Lower values draw first within UI; equal values keep submission order. UI objects are
	submitted in whatever order their owners happen to update in, which made a surface created
	later (a HUD gauge built on entering an arena) cover a window created at start-up. The
	Client's UI layer table is the single place that decides this. */
	virtual int32_t Get_UISortLayer() const { return 0; }
	virtual HRESULT Render_DeferredOverlay();
	virtual HRESULT Render_Shadow();
    // Opt in only for side-effect-free, time-invariant depth. Every change to
    // the rendered geometry or visibility must change the nonzero revision.
    virtual bool_t Try_GetStaticShadowRevision(uint64_t& outRevision) const
    { outRevision = 0u; return false; }

protected:
    void Invalidate_FinalCameraSpatialBounds();

private:
    friend class CLayer;
    struct FINAL_CAMERA_LAYER_LINK final
    {
        CLayer* Owner = nullptr;
        size_t Index = 0u;
        bool DirtyQueued = false;
        FINAL_CAMERA_LAYER_LINK() = default;
        // A prototype clone never inherits another object's layer ownership.
        FINAL_CAMERA_LAYER_LINK(const FINAL_CAMERA_LAYER_LINK&) noexcept {}
        FINAL_CAMERA_LAYER_LINK& operator=(const FINAL_CAMERA_LAYER_LINK&) noexcept
        { return *this; }
    } m_FinalCameraLayer;

protected:
	map<const wstring_t, shared_ptr<CComponent>>		m_Components;

protected:
	shared_ptr<class CTransform>			m_pTransformCom = { nullptr };


	template<typename T>
	HRESULT Add_Component(uint32_t iPrototypeLevelIndex, const wstring_t& strPrototypeTag, const wstring_t& strComponentTag, shared_ptr<T>& pOut, void* pArg = nullptr)
	{
		if (nullptr != Find_Component(strComponentTag))
			return E_FAIL;

		shared_ptr<CComponent> pComponent = dynamic_pointer_cast<CComponent>(CGameInstance::Get().Clone_Prototype(iPrototypeLevelIndex, strPrototypeTag, pArg));
		if (nullptr == pComponent)
			return E_FAIL;

		m_Components.emplace(strComponentTag, pComponent);

		pOut = dynamic_pointer_cast<T>(pComponent);

		return S_OK;
	}
protected:
	CComponent* Find_Component(const wstring_t& strComponentTag);

public:
	virtual shared_ptr<CPrototype> Clone(void* pArg) = 0;
	void Free();

};

NS_END
