#pragma once

#include "Client_Defines.h"
#include "ContainerObject.h"
#include "CharacterSpec.h"
#include "NavPathFollower.h"

NS_BEGIN(Engine)
class CModel;
class CNavigation;
class CTransform;
NS_END

NS_BEGIN(Client)

/* One playable character, whatever the class. Everything class-specific arrives
as a CHARACTER_SPEC plus an ICharacterLogic, so this stays shared by the team.

Input stays outside this object. A controller or level converts input into a
world-space goal and calls Request_Move(). */
class CCharacter final : public CContainerObject
{
public:
	typedef struct tagCharacterDesc : public CContainerObject::CONTAINEROBJECT_DESC
	{
		uint32_t iPrototypeLevelIndex = {};
		const CHARACTER_SPEC* pSpec = { nullptr };
		const tchar_t* pNavigationPrototypeTag = { nullptr };
		float3_t vPosition = {};
	} CHARACTER_DESC;

private:
	CCharacter(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	/* The logic is a unique_ptr, so the implicit copy is deleted. Clones build
	their own from the spec; only the prototype's empty state is copied. */
	CCharacter(const CCharacter& Prototype);
public:
	virtual ~CCharacter();

public:
	const CHARACTER_SPEC* Get_Spec() const {
		return m_pSpec;
	}
	shared_ptr<Engine::CModel> Get_BodyModel() const;
	shared_ptr<Engine::CTransform> Get_Transform() const {
		return m_pTransformCom;
	}

	void Set_Position(fvector_t vPosition);
	bool_t Set_Animation(CHARACTER_ANIM eAnim, bool_t isLoop);
	bool_t Set_Animation(const char_t* pClipName, bool_t isLoop);
	PATH_RESULT_CODE Request_Move(fvector_t vGoalPosition);
	void Cancel_Move();
	bool_t Is_Moving() const {
		return m_PathFollower.Has_Path();
	}

#ifdef _DEBUG
	void Set_NavigationDebugVisible(bool_t isVisible) {
		m_isNavigationDebugVisible = isVisible;
	}
#endif

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	const CHARACTER_SPEC* m_pSpec = { nullptr };
	unique_ptr<ICharacterLogic> m_pLogic;
	shared_ptr<Engine::CModel> m_pBodyModel = { nullptr };
	shared_ptr<Engine::CNavigation> m_pNavigationCom = { nullptr };
	CNavPathFollower m_PathFollower;
	uint32_t m_iPrototypeLevelIndex = {};
	f32_t m_fMoveSpeed = { 5.f };
	bool_t m_isMoving = { false };
	wstring_t m_strNavigationPrototypeTag;

#ifdef _DEBUG
	bool_t m_isNavigationDebugVisible = { false };
#endif

private:
	HRESULT Ready_Components();
	HRESULT Ready_PartObjects();
	void Set_Locomotion(bool_t isMoving);

public:
	static unique_ptr<CCharacter> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
