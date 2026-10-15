#pragma once

// Add_Component 템플릿이 원형 복제를 요청한다. 기존 간접 include로 CPrototype/Component/Transform과 공통 타입도 제공받는다.
#include "GameInstance.h"
// Renderer가 넘긴 연속 렌더 큐 구간을 복사 없이 차용해 인접 객체 묶음을 처리한다.
#include <span>

NS_BEGIN(Engine)

class CLayer;
class COcclusionCuller;

// Prototype 복제 계약 위에 Component 소유와 프레임/렌더 콜백을 제공한다.
// Layer는 객체를 소유하고, 객체가 보유한 Layer 링크는 알림을 위한 비소유 역참조다.
class ENGINE_DLL CGameObject abstract : public CPrototype
{
public:
	// Clone → Initialize로 전달하는 공통 생성 입력이다. Transform의 이동·회전 속도를 포함한다.
	// 파생 DESC가 UI 배치·텍스처 같은 정보를 더하며, 현재 기반 GameObject 전용 필드는 없다.
	typedef struct tagGameObjectDesc : public CTransform::TRANSFORM_DESC
	{

	}GAMEOBJECT_DESC;
protected:
	CGameObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CGameObject();

public:
	// tag에 등록된 Component의 공유 소유권을 반환한다. 없는 tag는 nullptr이며 조회가 새 Component를 만들지는 않는다.
	shared_ptr<CComponent> Get_Component(const wstring_t& strComponentTag);

public:
	virtual HRESULT Initialize_Prototype() override;
	// 인자는 GAMEOBJECT_DESC/TRANSFORM_DESC 계약으로 해석한다. 기본 Transform을 만들고 Component map에 등록한다.
	virtual HRESULT Initialize(void* pArg) override;
	// Layer가 등록 때 선택한 단계별 목록으로 호출한다. 시간은 초이며 기본 네 callback은 모두 아무 작업도 하지 않는다.
	virtual void Priority_Update(f32_t fTimeDelta);
	virtual void Update(f32_t fTimeDelta);
	virtual void Post_Physics_Update(f32_t fTimeDelta);
	virtual void Late_Update(f32_t fTimeDelta);
	static constexpr uint8_t UPDATE_PHASE_PRIORITY = 1u;
	static constexpr uint8_t UPDATE_PHASE_UPDATE = 2u;
	static constexpr uint8_t UPDATE_PHASE_POST_PHYSICS = 4u;
	static constexpr uint8_t UPDATE_PHASE_LATE = 8u;
	// 클래스별 고정 참여 마스크다. Layer가 등록 시 한 번 읽어 단계별 갱신 목록을 만든다.
	virtual uint8_t Get_UpdatePhaseMask() const
	{
		return 15u;
	}
	// 카메라/광원 제공자가 갱신된 뒤 월드 pass 전에 별도 준비가 필요한 객체만 참여한다.
	virtual bool_t Uses_FinalCameraSubmission() const
	{
		return false;
	}
	// 최종 카메라 준비/CPU 작업이 끝난 뒤 Layer의 소유 스레드에서 호출한다. 기본 객체에는 제출할 작업이 없다.
	virtual void Submit_FinalCamera() {}
    // owner thread에서 준비한 상태를 CPU 전용 callback 하나가 차용한다.
    // Layer는 모든 callback이 끝날 때까지 기다린 뒤 GPU 제출과 후속 owner 변경으로 넘어간다.
    struct FINAL_CAMERA_CPU_JOB final
    {
        void* Context = nullptr;
        void (*Execute)(void*) = nullptr;
        uint32_t Cost = 0u;
    };
    // 기본 구현은 출력 작업을 비우고 false다. 참여 구현의 Context는 Layer가 작업을 합류할 때까지 유효해야 한다.
    // Cost는 묶음 분할용 상대 작업량으로 시간 단위가 아니며 0인 작업은 Layer가 수집하지 않는다.
    virtual bool_t Try_PrepareFinalCameraCpuJob(FINAL_CAMERA_CPU_JOB& output)
    {
        output = {};
        return false;
    }
    // 최종 카메라 후보 트리용 보수적 월드 공간 구다. Radius에는 소비자가 필요한 여유까지 포함한다.
    // 제공하지 않으면 객체를 탈락시키지 않는다. 경계/정책/표시가 바뀌면 제출 전에 invalidate해야 한다.
    struct FINAL_CAMERA_SPATIAL_BOUNDS final
    {
        float3_t Center{};
        f32_t Radius = 0.f;
        uint32_t RejectGraceFrames = 0u;
        bool_t ShadowCaster = false;
    };
    // 기본 구현은 출력 구를 비우고 false다. Layer는 경계 미제공 객체를 별도 무경계 후보로 유지한다.
    virtual bool_t Try_GetFinalCameraSpatialBounds(FINAL_CAMERA_SPATIAL_BOUNDS& out) const
    {
        out = {};
        return false;
    }
    // 이번 프레임에 확정한 표시 지오메트리 전체를 감싸는 bounds다.
    // color occlusion과 그림자 제출은 별개이므로 화면에서 가려져도 그림자 조건을 유지한다.
    struct STATIC_OCCLUSION_DESC final
    {
        float3_t BoundsMin{}, BoundsMax{};
        uint32_t Draws = 0u, OccluderTriangles = 0u;
        // 지오메트리/표시/cull 상태/재질 적격성이 달라지면 0이 아닌 revision을 갱신해야 한다.
        uint64_t Indices = 0u, Revision = 0u;
    };
    // Renderer가 가림 판정 후보를 조회한다. 기본 구현은 출력을 비우고 false로 참여하지 않는다.
    virtual bool_t Try_GetStaticOcclusionDesc(STATIC_OCCLUSION_DESC& output) const
    {
        output = {};
        return false;
    }
    // 실제 그리는 불투명 비변형 지오메트리만 depth 가림 정보로 제공할 수 있다.
    virtual uint32_t Rasterize_StaticOccluder(COcclusionCuller&, uint32_t) const
    {
        return 0u;
    }
    // 현재 카메라의 보수적 HiZ 입력 전용이다. 동적/alpha/deform geometry는 opt-in하지 않는다.
    // GPU depth 선행 pass의 선택적 입력이다. 기본 구현은 출력을 비우고 false이며 실제 표면 렌더와 별도 opt-in이다.
    virtual bool_t Try_GetVisibilityDepthDesc(STATIC_OCCLUSION_DESC& output) const
    {
        output = {};
        return false;
    }
    // 기본 구현은 0개를 기록하고 S_FALSE를 반환한다. Renderer는 S_OK이면서 실제 삼각형 수가 있을 때만 예산에 합산한다.
    virtual HRESULT Render_VisibilityDepth(uint32_t triangleBudget, uint32_t& drawnTriangles)
    {
        (void)triangleBudget;
        drawnTriangles = 0u;
        return S_FALSE;
    }
	// Renderer가 제출된 큐의 소유 참조를 통해 호출한다. 기본 Render는 성공하며 Render_Group은 그룹별 특수 동작 없이 Render에 위임한다.
	virtual HRESULT Render();
	virtual HRESULT Render_Group(RENDERGROUP group);
	// 현재 렌더 큐를 호출 중에만 차용한다. 참여 객체는 인접 항목을 묶고 소비한 개수를 돌려줄 수 있다.
	// 기본 구현은 consumed를 먼저 1로 설정한다. 비어 있거나 첫 객체가 this가 아니면 E_INVALIDARG이며 실패 시 consumed 보존 계약은 아니다.
	virtual HRESULT Render_AdjacentNonBlend(
		std::span<const std::shared_ptr<CGameObject>> objects, size_t& consumed);
	// BLEND 내부에서는 작은 값부터 그린다. 같은 값끼리는 기존 거리 순서를 유지한다.
	virtual int32_t Get_BlendSortPriority() const
	{
		return 0;
	}
	// UI 내부에서는 작은 값부터 그리며 같은 값은 제출 순서를 유지한다.
	// 생성/갱신 시점만으로 겹침이 결정되지 않도록 실제 값은 Client의 UI layer 표가 정한다.
	virtual int32_t Get_UISortLayer() const
	{
		return 0;
	}
	// 추가 overlay와 그림자 pass의 기본 구현은 그리기 없이 성공한다. 참여 파생 객체가 실제 draw를 제공한다.
	virtual HRESULT Render_DeferredOverlay();
	virtual HRESULT Render_Shadow();
    // 시간에 따라 변하지 않고 부수 효과가 없는 depth만 정적 그림자 캐시에 참여한다.
    // 렌더 지오메트리나 표시가 바뀌면 0이 아닌 revision도 함께 바꿔야 한다.
    // 기본 구현은 revision을 0으로 비우고 false다. Renderer는 true와 0이 아닌 revision을 함께 확인해야 캐시 후보로 모은다.
    virtual bool_t Try_GetStaticShadowRevision(uint64_t& outRevision) const
    {
        outRevision = 0u;
        return false;
    }

protected:
    // 표시/경계가 변한 파생 객체가 현재 Layer에 재계산을 요청한다. 등록 전이면 무시하며 내부 스레드 동기화는 제공하지 않는다.
    void Invalidate_FinalCameraSpatialBounds();

private:
    friend class CLayer;
    struct FINAL_CAMERA_LAYER_LINK final
    {
        CLayer* Owner = nullptr;
        size_t Index = 0u;
        bool DirtyQueued = false;
        FINAL_CAMERA_LAYER_LINK() = default;
        // 복사 생성한 Clone은 원본의 Layer 소속을 물려받지 않는다. 대입 연산은 현재 링크를 보존한다.
        FINAL_CAMERA_LAYER_LINK(const FINAL_CAMERA_LAYER_LINK&) noexcept {}
        FINAL_CAMERA_LAYER_LINK& operator=(const FINAL_CAMERA_LAYER_LINK&) noexcept
        {
            return *this;
        }
    } m_FinalCameraLayer;

protected:
	// map과 전용 멤버가 같은 Component의 소유권을 공유할 수 있다. map을 비워도 외부 참조는 유지된다.
	map<const wstring_t, shared_ptr<CComponent>>		m_Components;

protected:
	shared_ptr<class CTransform>			m_pTransformCom = { nullptr };


	// 중복 tag를 거부하고 원형을 복제한 뒤 Component map에 등록한다.
	// 현재 구현은 마지막 T 변환의 nullptr를 실패로 바꾸지 않는다. 호출자는 요청한 타입과 원형을 맞춰야 한다.
	template<typename T>
	// 중복 tag·원형 복제 실패는 pOut을 보존한다. map 등록 뒤 T 변환이 실패하면 pOut은 nullptr로 바뀌지만 S_OK를 반환하는 기존 순서를 유지한다.
	HRESULT Add_Component(
		uint32_t iPrototypeLevelIndex,
		const wstring_t& strPrototypeTag,
		const wstring_t& strComponentTag,
		shared_ptr<T>& pOut,
		void* pArg = nullptr)
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
	// raw pointer는 map이 소유한 Component를 빌린다. Free/객체 파괴 이후 보관하면 안 된다.
	CComponent* Find_Component(const wstring_t& strComponentTag);

public:
	virtual shared_ptr<CPrototype> Clone(void* pArg) = 0;
	// Component map의 공유 참조만 비운다. 전용 Transform 멤버와 외부 소유 참조까지 이 함수에서 해제하지 않는다.
	void Free();

};

NS_END
