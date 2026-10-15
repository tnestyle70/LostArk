#pragma once

// Client 이름 공간과 공통 타입을 사용한다.
#include "Client_Defines.h"
// Component 보유와 부모 행렬 차용·합성 계약을 상속한다.
#include "PartObject.h"
// 장비의 현재 모델·palette·행렬을 잔상에 전달할 MODEL_VIEW를 사용한다.
#include "SkeletalAfterimage.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CCharacter;

struct DEFERRED_EMISSIVE_OVERRIDE;

// 본체를 따라가는 장비의 시각 Part다. 소켓 장비는 한 본에 붙이고 스키닝 장비는 본체 자세를 사용한다.
// 본체에 없는 장비 전용 본이 있으면 자체 골격을 본체 이름에 맞춰 동기화한다.
class CPart_Equipment final : public CPartObject
{
public:
	// Clone 입력이다. 모델 tag·프로필·소켓 이름은 값으로 저장하고 행렬·Emissive는 계속 차용한다.
	typedef struct tagPartEquipmentDesc : public CPartObject::PARTOBJECT_DESC
	{
		uint32_t iPrototypeLevelIndex = {};
		wstring_t strModelTag;
		wstring_t strShaderTag;

		// 소켓/스키닝이 참조할 본체 Model이다. 초기화 후 shared_ptr로 보유한다.
		shared_ptr<Engine::CModel> pSkeletonModel = { nullptr };

		// 비트 i가 켜지면 본체 등 다른 Part와 겹치는 i번 submesh를 숨긴다.
		uint32_t iHiddenMeshMask = {};
        bool_t isWeaponPart = false, isIdentityPart = false;

		// nullptr 또는 빈 이름은 스키닝 장비다. 이름이 있으면 그 본에 붙는 소켓 장비다.
		const char_t* pSocketBoneName = { nullptr };
		f32_t fSocketYawDegrees = 0.f;

		// 본체 Part의 local 행렬을 차용한다. 소켓 부착 뒤 적용해 본체와 같은 부모 공간에 놓는다.
		const float4x4_t* pSocketRootMatrix = { nullptr };
		string strMaterialProfileId;
		const DEFERRED_EMISSIVE_OVERRIDE* pEmissiveOverride = { nullptr };
	} PART_EQUIPMENT_DESC;

private:
	// Create/Clone에서만 생성해 장치·Context를 기반 객체에 전달한다.
	CPart_Equipment(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
public:
	// 장비·본체 Model과 Shader의 공유 참조를 정리한다. 차용 행렬·Emissive는 해제하지 않는다.
	virtual ~CPart_Equipment();

public:
	// 캐릭터의 표현 숨김만 읽는 weak 참조이며 수명을 연장하지 않는다.
	void Set_CharacterPresentationOwner(const std::shared_ptr<CCharacter>& owner)
	{ m_pCharacterPresentationOwner = owner; }
	// 외형 편집기가 장비 재질을 갱신할 수 있도록 장비 Model의 공유 참조를 반환한다.
	shared_ptr<CModel> Get_Model() const {
		return m_pModelCom;
	}

public:
	// 원형 준비는 성공만 반환한다. 실제 장비 자원은 Clone 초기화에서 복제한다.
	virtual HRESULT Initialize_Prototype() override;
	// 필수 본체 Model·소켓 존재를 검사한 뒤 Component와 추가 본·재질 분류를 준비한다.
	virtual HRESULT Initialize(void* pArg) override;
	// Priority_Update는 비어 있다. Update가 본체의 현재 본을 읽어 장비 월드행렬을 계산한다.
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	// 보이는 장비만 기본·반투명·그림자 렌더 그룹에 등록한다.
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Group(RENDERGROUP group) override;
	virtual HRESULT Render_Shadow() override;
	// 스키닝/소켓 셰이더의 pass 표가 다르므로 미리보기 pass를 각각 받는다.
	HRESULT Render_Pass(uint32_t iSkinnedPassIndex, uint32_t iSocketedPassIndex);
	HRESULT Render_Translucent(ID3D11BlendState* pCoverageBlend = nullptr);

public:
	// 일반 표시와 identity 연출 표시를 합치고 suppression·캐릭터 숨김을 마지막에 적용한다.
	void Set_Visible(bool_t isVisible) { m_isVisible = isVisible; }
	bool_t Is_Visible() const { return (m_isVisible || m_isPresentationVisible) && !m_isPresentationSuppressed && !Is_CharacterPresentationHidden(); }
    void Set_PresentationSuppressed(bool_t value) { m_isPresentationSuppressed = value; }
    void Set_PresentationVisible(bool_t value) { m_isPresentationVisible = m_isIdentityPart && value; }
    bool_t Is_WeaponPart() const { return m_isWeaponPart; }
    bool_t Is_IdentityPart() const { return m_isIdentityPart; }
    bool Is_Socketed() const { return !m_strSocketBoneName.empty(); }
    // 최신 소켓 합성과 palette를 준비해 잔상이 복사할 공유 참조·값을 반환한다.
    bool Get_AfterimageView(CSkeletalAfterimage::MODEL_VIEW& view);
	// 소켓 장비만 meter 위치·degree 회전을 받는다. 범위/유한성 실패는 기존 값을 유지한다.
	bool_t Set_SocketTransform(const float3_t& positionMeters, const float3_t& rotationDegrees);

private:
	bool_t m_isVisible = true;
    bool_t m_isPresentationSuppressed = false;
    bool_t m_isPresentationVisible = false;
    bool_t m_isWeaponPart = false, m_isIdentityPart = false;
	uint32_t m_iHiddenMeshMask = {};
	std::weak_ptr<CCharacter> m_pCharacterPresentationOwner;
	bool_t Is_CharacterPresentationHidden() const;
	// 장비 Model·Shader와 본체 Model을 공유 소유한다. 본체 자체 GameObject는 소유하지 않는다.
	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pModelCom = { nullptr };
	shared_ptr<CModel> m_pSkeletonModelCom = { nullptr };
	// 추가 본이 있으면 본체 자세를 이름으로 가져온 자체 palette를 mesh별로 바인딩한다.
	bool_t m_hasOwnBones = false;
	string m_strSocketBoneName;
	f32_t m_fSocketYawDegrees = 0.f;
	float3_t m_vSocketPositionMeters{};
	float3_t m_vSocketRotationDegrees{};
	// 행렬과 Emissive는 비소유 포인터다. 호출자는 장비 사용 동안 대상의 주소·수명을 유지한다.
	const float4x4_t* m_pSocketRootMatrix = { nullptr };
	string m_strMaterialProfileId;
	const DEFERRED_EMISSIVE_OVERRIDE* m_pEmissiveOverride = { nullptr };
	bool_t m_hasTranslucentMeshes = { false };

private:
	// descriptor의 Level/tag로 Shader·Model Component를 복제한다.
	HRESULT Ready_Components(const PART_EQUIPMENT_DESC* pDesc);
	HRESULT Bind_ShaderResources();
	HRESULT Bind_ShadowShaderResources();

public:
	// Loader 등록용 원형을 만든다. 초기화 실패는 nullptr다.
	static unique_ptr<CPart_Equipment> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	// Character·미리보기 등의 Part 등록이 사용할 복제본을 만든다. 초기화 실패는 nullptr다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
