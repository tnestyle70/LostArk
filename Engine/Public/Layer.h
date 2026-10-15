#pragma once

// 소유할 GameObject의 갱신/제출 함수와 공간 경계·CPU 작업 타입을 사용한다.
#include "GameObject.h"
// 네 갱신 단계와 여섯 카메라 평면, 공간 노드의 세 축 범위를 고정 크기 배열로 보관한다.
#include <array>

NS_BEGIN(Engine)

// 객체의 shared_ptr를 소유하고 프레임 단계별 raw pointer 목록과 최종 카메라 후보를 함께 관리한다.
// 등록/제거와 순회는 호출자가 직렬화해야 한다. 병렬 준비 작업은 별도 CPU job만 수행하고 반환 전에 합류한다.
class CLayer
{
private:
	CLayer(uint32_t levelIndex, const wstring_t& layerTag);
public:
	~CLayer();

public:
	// 목록 순서로 선형 탐색한다. 삭제하면 뒤 인덱스가 바뀌므로 저장 ID로 사용하지 않는다.
	shared_ptr<CGameObject> Get_GameObject(uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(const wstring_t& strComponentTag, uint32_t iIndex);
	// part 조회는 해당 객체가 CContainerObject인 경우에만 호출하는 계약이다.
	shared_ptr<CComponent> Get_Component(const wstring_t& strPartTag, const wstring_t& strComponentTag, uint32_t iIndex);


public:
	// 등록 때 클래스의 고정 phase mask를 읽는다. 등록 후 phase 목록을 매 프레임 다시 만들지 않는다.
	HRESULT Add_GameObject(shared_ptr<CGameObject> pGameObject);
	// 미등록/null 객체는 E_FAIL이다. 갱신/공간 역참조를 먼저 정리한 뒤 소유 참조를 제거하며 외부 shared_ptr는 유지될 수 있다.
	HRESULT Remove_GameObject(const shared_ptr<CGameObject>& pGameObject);
	// Object_Manager가 초 단위 시간을 전달한다. 각 단계는 등록 시 만든 목록 순서대로 실행하며 순회 중 구조 변경을 잠그지 않는다.
	virtual void Priority_Update(f32_t fTimeDelta);
	virtual void Update(f32_t fTimeDelta);
	virtual void Post_Physics_Update(f32_t fTimeDelta);
	virtual void Late_Update(f32_t fTimeDelta);
	// 카메라 확정 후 보수적 공간 후보를 만들고, 필요한 CPU 준비를 마친 객체들을 원래 등록 순서로 제출한다.
	void Submit_FinalCamera();
private:
    friend class CGameObject;
    // GameObject의 변경 알림을 중복 없이 모은다. 등록자/소유 스레드가 호출하며 큐 할당 실패는 전체 트리 재구축으로 전환한다.
    void Invalidate_FinalCameraSpatialBounds(CGameObject* object);
    // Submit이 선택한 색상 후보만 CPU 작업으로 준비하고 동기 합류한다. 작업/할당 예외 뒤에도 객체의 기존 Submit 경로를 사용한다.
    void Prepare_FinalCameraCpuJobs();
    // 현재 멤버를 유효 구가 있는 트리와 경계 없는 항상 제출 목록으로 다시 분류한다. 기존 후보 캐시를 그대로 재사용하지 않는다.
    void Rebuild_FinalCameraHierarchy();
    // dirty 객체의 경계만 재확인해 부모까지 갱신한다. 경계 제공 여부가 바뀌면 트리 구성이 달라져 전체 재구축한다.
    void Refresh_FinalCameraHierarchy();
    // 비어 있지 않은 [first,end) 범위를 재귀 분할하고 부모·다음 subtree 위치를 기록한다. 반환값은 이 Layer 안의 노드 index다.
    uint32_t Build_FinalCameraNode(size_t first, size_t end, uint32_t parent);
    // 유효 노드의 구 또는 두 자식 AABB를 합친다. 경계가 바뀌었으므로 연속 탈락 유예 카운터도 0으로 돌린다.
    void Refit_FinalCameraNode(uint32_t index);
    // Object는 m_GameObjects가 수명을 보장한다. Leaf는 공간 트리의 index이며 영구 객체 ID가 아니다.
    struct FINAL_CAMERA_MEMBER final
    {
        CGameObject* Object = nullptr;
        CGameObject::FINAL_CAMERA_SPATIAL_BOUNDS Bounds{};
        uint32_t Leaf = UINT32_MAX;
        bool Bounded = false;
    };
    // 월드 공간 AABB를 담는다. End는 깊이 우선 배열에서 이 subtree 다음 위치여서 가지 전체를 건너뛸 수 있다.
    // RemainingPlanes의 6bit는 아직 자식에서 검사해야 할 frustum 평면이다.
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
    // Order는 트리 분할 순서, Always는 bounds 미제공 객체, Candidates는 실제 제출 순서 복원용 목록이다.
    std::vector<FINAL_CAMERA_MEMBER> m_FinalCameraMembers;
    std::vector<FINAL_CAMERA_NODE> m_FinalCameraNodes;
    std::vector<size_t> m_FinalCameraOrder, m_FinalCameraAlways, m_FinalCameraCandidates;
    std::vector<size_t> m_FinalCameraCpuCandidates;
    std::vector<CGameObject*> m_FinalCameraDirty;
    std::vector<CGameObject::FINAL_CAMERA_CPU_JOB> m_FinalCameraCpuJobs;
    // 작업 배열의 [First,End) 묶음이다. worker는 이 구간만 읽고 모든 callback의 합류 전까지 배열을 재할당하지 않는다.
    struct FINAL_CAMERA_CPU_RANGE final
    {
        size_t First = 0u, End = 0u;
    };
    std::vector<FINAL_CAMERA_CPU_RANGE> m_FinalCameraCpuRanges;
    // 구성 변경은 rebuild, bounds만 바뀐 경우는 refit한다. 카메라/그림자/최적화 revision도 후보 재사용 조건이다.
    bool m_FinalCameraTopologyDirty = true;
    std::array<float4_t, 6> m_FinalCameraPlanes{};
    bool m_FinalCameraShadowEnabled = false;
    bool m_FinalCameraCandidatesReusable = false;
    uint64_t m_FinalCameraOptimizationRevision = 0u;
private:
	list<shared_ptr<CGameObject>>		m_GameObjects;
	// 생성 때 Level과 Layer 이름으로 만들어 매 프레임 문자열 조립과 객체별 scope 생성을 피한다.
	std::array<std::string, 4> m_ProfileScopeNames;
	// 비소유 갱신 목록이다. 제거 전에 목록에서 빼고, 그때까지 m_GameObjects가 객체 수명을 보장한다.
	std::array<std::list<CGameObject*>, 4> m_PhaseObjects;
	std::string m_FinalCameraScopeName;

public:
	// Object_Manager가 첫 객체 등록 전에 만든다. 생성한 Layer만 반환하며 new/문자열 할당 예외를 nullptr로 변환하지 않는다.
	static shared_ptr<CLayer> Create(uint32_t levelIndex, const wstring_t& layerTag);
};

NS_END
