#pragma once

#include "GameObject.h"
#include <array>

NS_BEGIN(Engine)

class CLayer
{
private:
	CLayer(uint32_t levelIndex, const wstring_t& layerTag);
public:
	~CLayer();

public:
	shared_ptr<CGameObject> Get_GameObject(uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(const wstring_t& strComponentTag, uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(const wstring_t& strPartTag, const wstring_t& strComponentTag, uint32_t iIndex);


public:
	HRESULT Add_GameObject(shared_ptr<CGameObject> pGameObject);
	HRESULT Remove_GameObject(const shared_ptr<CGameObject>& pGameObject);
	virtual void Priority_Update(f32_t fTimeDelta);
	virtual void Update(f32_t fTimeDelta);
	virtual void Post_Physics_Update(f32_t fTimeDelta);
	virtual void Late_Update(f32_t fTimeDelta);
	void Submit_FinalCamera();
private:
    friend class CGameObject;
    void Invalidate_FinalCameraSpatialBounds(CGameObject* object);
    void Prepare_FinalCameraCpuJobs();
    void Rebuild_FinalCameraHierarchy();
    void Refresh_FinalCameraHierarchy();
    uint32_t Build_FinalCameraNode(size_t first, size_t end, uint32_t parent);
    void Refit_FinalCameraNode(uint32_t index);
    struct FINAL_CAMERA_MEMBER final
    {
        CGameObject* Object = nullptr;
        CGameObject::FINAL_CAMERA_SPATIAL_BOUNDS Bounds{};
        uint32_t Leaf = UINT32_MAX;
        bool Bounded = false;
    };
    struct FINAL_CAMERA_NODE final
    {
        std::array<double, 3> Minimum{}, Maximum{};
        uint32_t Parent = UINT32_MAX, End = 0u, Left = UINT32_MAX, Right = UINT32_MAX;
        size_t Member = SIZE_MAX;
        uint32_t RejectGraceFrames = 0u, RejectedFrames = 0u;
        bool ShadowCaster = false;
        bool CameraIntersects = true;
        uint8_t RemainingPlanes = 0x3fu;
    };
    std::vector<FINAL_CAMERA_MEMBER> m_FinalCameraMembers;
    std::vector<FINAL_CAMERA_NODE> m_FinalCameraNodes;
    std::vector<size_t> m_FinalCameraOrder, m_FinalCameraAlways, m_FinalCameraCandidates;
    std::vector<size_t> m_FinalCameraCpuCandidates;
    std::vector<CGameObject*> m_FinalCameraDirty;
    std::vector<CGameObject::FINAL_CAMERA_CPU_JOB> m_FinalCameraCpuJobs;
    struct FINAL_CAMERA_CPU_RANGE final { size_t First = 0u, End = 0u; };
    std::vector<FINAL_CAMERA_CPU_RANGE> m_FinalCameraCpuRanges;
    bool m_FinalCameraTopologyDirty = true;
    std::array<float4_t, 6> m_FinalCameraPlanes{};
    bool m_FinalCameraShadowEnabled = false;
    bool m_FinalCameraCandidatesReusable = false;
    uint64_t m_FinalCameraOptimizationRevision = 0u;
private:
	list<shared_ptr<CGameObject>>		m_GameObjects;
	// Built once with the layer identity; no per-object scopes or per-frame formatting.
	std::array<std::string, 4> m_ProfileScopeNames;
	// Non-owning phase membership. m_GameObjects owns every pointer until removal.
	std::array<std::list<CGameObject*>, 4> m_PhaseObjects;
	std::string m_FinalCameraScopeName;

public:
	static shared_ptr<CLayer> Create(uint32_t levelIndex, const wstring_t& layerTag);
};

NS_END
