#pragma once

// Client 이름 공간과 공통 타입을 사용한다.
#include "Client_Defines.h"
// 부모 행렬 차용·Component 소유·local×parent 합성 계약을 상속한다.
#include "PartObject.h"
// 잔상에 전달할 Model·palette·월드행렬의 MODEL_VIEW 타입을 사용한다.
#include "SkeletalAfterimage.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CCharacter;

struct DEFERRED_EMISSIVE_OVERRIDE;

// 캐릭터 본체의 스키닝 Model과 애니메이션 시간을 갱신하는 Part다.
// 장비는 이 본체의 현재 본 행렬을 소켓 부착 또는 스키닝에 사용한다.
class CPart_Body final : public CPartObject
{
public:
	// Clone 입력이다. tag는 복사/조회에 쓰고 부모 행렬·Emissive 포인터는 초기화 후에도 차용한다.
	typedef struct tagPartBodyDesc : public CPartObject::PARTOBJECT_DESC
	{
		uint32_t iPrototypeLevelIndex = {};
		wstring_t strModelTag;
		wstring_t strShaderTag;

		// 비트 i가 켜지면 i번 submesh를 숨긴다. 장비와 겹치는 피부를 제외할 때 쓴다.
		uint32_t iHiddenMeshMask = {};

		const char_t* pInitialAnimation = { nullptr };
		const DEFERRED_EMISSIVE_OVERRIDE* pEmissiveOverride = { nullptr };
	} PART_BODY_DESC;

private:
	// Create 또는 원형 복제로만 생성해 장치·Context 참조를 기반 객체에 전달한다.
	CPart_Body(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	// 멤버와 기반 Component의 공유 참조를 정리한다. 차용 부모·Emissive 메모리는 해제하지 않는다.
	virtual ~CPart_Body();

public:
	// 캐릭터 숨김 상태만 읽는 weak 참조다. Part가 캐릭터의 수명을 늘리지 않는다.
	void Set_CharacterPresentationOwner(const std::shared_ptr<CCharacter>& owner)
	{ m_pCharacterPresentationOwner = owner; }
	// 장비·미리보기에 모델의 공유 참조를 반환한다. 반환 참조가 남으면 모델도 살아 있다.
	shared_ptr<CModel> Get_Model() const {
		return m_pModelCom;
	}
	void Set_HiddenMeshes(uint32_t iHiddenMeshMask) {
		m_iHiddenMeshMask = iHiddenMeshMask;
	}
    // 잔상에 현재 모델·셰이더 공유 참조와 최신 월드행렬·숨김 mask를 복사한다.
    bool Get_AfterimageView(CSkeletalAfterimage::MODEL_VIEW& view) const
    {
        if (!m_pModelCom || !Try_Get_PresentationRootMatrix(&view.world)) return false;
        view.model = view.paletteModel = m_pModelCom; view.shader = m_pShaderCom;
        view.hiddenMeshMask = m_iHiddenMeshMask; view.socketed = false; return true;
    }
	void Set_WeaponReplacementBody(const shared_ptr<CModel>& body) { m_WeaponReplacementBody = body; }
	bool_t Set_Animation(const char_t* pClipName, bool_t isLoop);
	// 현재 local×parent를 다시 계산한다. Update 뒤 저작 도구가 바꾼 부모 행렬도 반영한다.
	bool_t Try_Get_PresentationRootMatrix(float4x4_t* pOutWorld) const;

public:
	// 원형에는 추가 자원이 없어 성공한다. 모델·셰이더는 Clone 초기화에서 준비한다.
	virtual HRESULT Initialize_Prototype() override;
	// descriptor로 Component를 복제하고 초기 clip과 forward 재질 분기를 준비한다.
	virtual HRESULT Initialize(void* pArg) override;
	// 현재 Priority_Update는 비어 있다. Update에서 본체 애니메이션과 합성 행렬을 갱신한다.
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	// 캐릭터 표시 상태·재질·그림자 사용에 맞는 Renderer 그룹에 자신을 등록한다.
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Group(RENDERGROUP group) override;
	virtual HRESULT Render_Shadow() override;
	// 미리보기에서 pass를 직접 지정한다. 기본 pass는 forward 재질을 별도 단계에 맡긴다.
	HRESULT Render_Pass(uint32_t iPassIndex);
	HRESULT Render_ForwardSource(bool opaqueGhost, ID3D11BlendState* pCoverageBlend = nullptr);

private:
	// 캐릭터와 대체 본체는 weak 참조이고 렌더에 필요한 Model·Shader는 공유 소유한다.
	std::weak_ptr<CCharacter> m_pCharacterPresentationOwner;
	bool_t Is_CharacterPresentationHidden() const;
	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pModelCom = { nullptr };
	shared_ptr<CModel> m_pSaydonHatModel;
	uint32_t m_iHiddenMeshMask = {};
	std::weak_ptr<CModel> m_WeaponReplacementBody;
	// 호출자가 소유한 발광 상태를 렌더 때 읽는다. Part 사용 동안 주소·수명이 유지되어야 한다.
	const DEFERRED_EMISSIVE_OVERRIDE* m_pEmissiveOverride = { nullptr };
	bool_t m_hasTranslucentMeshes = { false };
    bool_t m_hasOpaqueGhostMeshes = { false };

private:
	// descriptor tag로 기존 Prototype의 Shader·Model Component를 복제해 보유한다.
	HRESULT Ready_Components(const PART_BODY_DESC* pDesc);
	HRESULT Bind_ShaderResources();
	HRESULT Bind_ShadowShaderResources();

public:
	// Loader 등록용 원형이다. 초기화 실패는 nullptr이며 생성 중 참조를 정리한다.
	static unique_ptr<CPart_Body> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	// Character의 Part 또는 미리보기 Layer가 쓸 복제본이다. 초기화 실패는 nullptr다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
