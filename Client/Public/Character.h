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
	/* One skill's clip chain, read from <asset>.clipseq. The game plays a skill
	as a fixed run of clips; a skill has several chains because each tripod build
	takes a different route, so a chain is picked by index. */
	struct CLIP_CHAIN
	{
		int32_t iSkillId = {};
		int32_t iSeqIndex = {};
		/* COMBO advances a step per press, HOLD is a charge, ONESHOT/SEQUENCE
		run to the end. Only carried for now -- every chain runs to the end. */
		std::string sMode;
		std::vector<std::string> clips;
	};

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
	shared_ptr<Engine::CTransform> Get_Transform() const {
		return m_pTransformCom;
	}

	/* Shows or hides one assembled part. A class whose identity swaps its weapon
	ships every variant as its own part and toggles between them; the hidden one
	keeps riding its socket, it just leaves the render queue. Returns false when
	no part carries that tag. */
	bool_t Set_PartVisible(const tchar_t* pPartTag, bool_t isVisible);

	bool_t Set_Animation(CHARACTER_ANIM eAnim, bool_t isLoop);
	bool_t Set_Animation(const char_t* pClipName, bool_t isLoop);
	PATH_RESULT_CODE Request_Move(fvector_t vGoalPosition);
	bool_t Is_Moving() const {
		return m_PathFollower.Has_Path();
	}

#ifdef _DEBUG
	void Set_NavigationDebugVisible(bool_t isVisible) {
		m_isNavigationDebugVisible = isVisible;
	}
#endif

	/* Starts the skill's chain and plays it through. Returns false when the skill
	has no chain or one is already running. */
	bool_t Play_Skill(int32_t iSkillId, int32_t iSeqIndex = 0);
	bool_t Is_PlayingSkill() const {
		return nullptr != m_pChain;
	}

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

	std::vector<CLIP_CHAIN> m_Chains;
	/* The chain being played, and how far into it. Null when idle. */
	const CLIP_CHAIN* m_pChain = { nullptr };
	int32_t m_iChainStep = {};

private:
	HRESULT Ready_Components();
	HRESULT Ready_PartObjects();
	void Set_Locomotion(bool_t isMoving);
	bool_t Load_ClipChains();
	/* Plays a clip from its first frame. Set_Animation alone only switches the
	index, so a clip that already ran would resume at its end -- which chains
	that repeat a clip do hit. */
	bool_t Start_Clip(const char_t* pClipName);
	bool_t Is_ClipFinished() const;
	/* Moves to the next clip once the current one ends, and drops back to idle
	after the last. */
	void Update_Chain();

public:
	static unique_ptr<CCharacter> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
