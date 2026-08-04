#include "Character.h"

#include "GameInstance.h"
#include "Navigation.h"
#include "Part_Body.h"
#include "Part_Equipment.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr f32_t CLIP_BLEND_SECONDS = 0.12f;
}

CCharacter::CCharacter(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CContainerObject { pDevice, pContext }
{
}

CCharacter::CCharacter(const CCharacter& Prototype)
	: CContainerObject { Prototype }
{
}

CCharacter::~CCharacter()
{
}

HRESULT CCharacter::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCharacter::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const auto pDesc = static_cast<CHARACTER_DESC*>(pArg);

	m_pSpec = pDesc->pSpec;
	m_iPrototypeLevelIndex = pDesc->iPrototypeLevelIndex;
	m_fMoveSpeed = pDesc->fSpeedPerSec > 0.f ?
		pDesc->fSpeedPerSec : 5.f;
	//nickname과 local control 여부 추가
	m_strNickName = pDesc->strNickName;
	m_isLocallyControlled = pDesc->isLocallyControlled;

	if (nullptr != pDesc->pNavigationPrototypeTag)
	{
		m_strNavigationPrototypeTag =
			pDesc->pNavigationPrototypeTag;
	}

	if (nullptr == m_pSpec)
		return E_FAIL;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	m_pTransformCom->Set_State(
		STATE::POSITION,
		XMVectorSet(
			pDesc->vPosition.x,
			pDesc->vPosition.y,
			pDesc->vPosition.z,
			1.f));

	if (FAILED(Ready_Components()) ||
		FAILED(Ready_PartObjects()))
	{
		return E_FAIL;
	}

	Load_ClipChains();

	// Remote Character는 local keyboard logic를 만들지 않는다.
	if (m_isLocallyControlled &&
		nullptr != m_pSpec->pCreateLogic)
	{
		m_pLogic = m_pSpec->pCreateLogic();
	}

	return S_OK;
}

/* Reads <asset>.clipseq, the clip order the game plays each skill in. Extracted
from the game's Action table; see the 07-31 RESULT doc. */
bool_t CCharacter::Load_ClipChains()
{
	m_Chains.clear();

	if (nullptr == m_pSpec->pAssetName)
		return false;

	const std::string assetName = m_pSpec->pAssetName;
	const std::string path = CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(assetName) /
		filesystem::path(assetName + ".clipseq")).string();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[2048]{};
	/* header: LOSTARK_CLIP_SEQ <version> "<owner>" <count> */
	if (nullptr == fgets(szLine, sizeof(szLine), pFile) ||
		nullptr == strstr(szLine, "LOSTARK_CLIP_SEQ"))
	{
		fclose(pFile);
		return false;
	}

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		CLIP_CHAIN chain{};
		chain.iSkillId = atoi(szLine);

		const char_t* pSeq = strstr(szLine, "seq=");
		if (nullptr != pSeq)
			chain.iSeqIndex = atoi(pSeq + 4);

		const char_t* pMode = strstr(szLine, "mode=");
		if (nullptr != pMode)
		{
			pMode += 5;
			while ('\0' != *pMode && ' ' != *pMode && '\r' != *pMode && '\n' != *pMode)
				chain.sMode.push_back(*pMode++);
		}

		/* clips="a,b,c" */
		const char_t* pClips = strstr(szLine, "clips=\"");
		if (nullptr == pClips || 0 == chain.iSkillId)
			continue;

		pClips += 7;
		std::string one;
		for (; '\0' != *pClips && '\"' != *pClips; ++pClips)
		{
			if (',' == *pClips)
			{
				if (!one.empty())
					chain.clips.push_back(one);
				one.clear();
				continue;
			}
			one.push_back(*pClips);
		}
		if (!one.empty())
			chain.clips.push_back(one);

		if (!chain.clips.empty())
			m_Chains.push_back(std::move(chain));
	}

	fclose(pFile);
	return true;
}

bool_t CCharacter::Start_Clip(const char_t* pClipName)
{
	if (!Set_Animation(pClipName, false))
		return false;

	m_pBodyModel->Set_AnimTrackPosition(m_pBodyModel->Get_CurrentAnimIndex(), 0.f);
	return true;
}

bool_t CCharacter::Is_ClipFinished() const
{
	if (nullptr == m_pBodyModel)
		return false;

	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	if (!m_pBodyModel->Get_AnimationProgress(
		m_pBodyModel->Get_CurrentAnimIndex(), fPosition, fDuration))
		return false;

	/* Same test CAnimation uses to report a non-looping clip as done; the track
	is left past the end rather than clamped. */
	return fDuration > 0.f && fPosition >= fDuration;
}

bool_t CCharacter::Play_Skill(int32_t iSkillId, int32_t iSeqIndex)
{
	if (nullptr != m_pChain)
		return false;

	/* Sequence indices come from the game's own grouping, which is neither
	contiguous nor always zero-based, so an exact miss falls back to the skill's
	lowest chain rather than refusing to play. */
	const CLIP_CHAIN* pPick = nullptr;
	for (const CLIP_CHAIN& chain : m_Chains)
	{
		if (chain.iSkillId != iSkillId)
			continue;
		if (chain.iSeqIndex == iSeqIndex)
		{
			pPick = &chain;
			break;
		}
		if (nullptr == pPick || chain.iSeqIndex < pPick->iSeqIndex)
			pPick = &chain;
	}

	if (nullptr == pPick || !Start_Clip(pPick->clips[0].c_str()))
		return false;

	m_pChain = pPick;
	m_iChainStep = 0;
	return true;
}

void CCharacter::Update_Chain()
{
	if (nullptr == m_pChain || !Is_ClipFinished())
		return;

	/* A combo holds on its last clip until the server confirms the next stage.
	Every other mode keeps running to the end by itself. */
	if ("COMBO" == m_pChain->sMode)
		return;

	++m_iChainStep;

	if (m_iChainStep >= static_cast<int32_t>(m_pChain->clips.size()))
	{
		m_pChain = nullptr;
		m_iChainStep = 0;
		//judge animation, whether character is moving
		Set_Animation(
			m_isMoving ?
			CHARACTER_ANIM::RUN : CHARACTER_ANIM::IDLE,
			true);
		return;
	}

	Start_Clip(m_pChain->clips[m_iChainStep].c_str());
}

void CCharacter::Update_NetworkTransform(f32_t fTimeDelta)
{
	if (!m_hasNetworkState ||
		nullptr == m_pTransformCom)
	{
		return;
	}
	//current pos
	const vector_t current =
		m_pTransformCom->Get_State(STATE::POSITION);
	//network에서 받은 target pos와의 거리
	const vector_t target = XMVectorSet(
		m_vNetworkTargetPosition.x,
		m_vNetworkTargetPosition.y,
		m_vNetworkTargetPosition.z,
		1.f);
	//current와 target blending
	const f32_t blend = (std::min)(1.f, 12.f * fTimeDelta);

	vector_t next = XMVectorLerp(
		current,
		target,
		blend);

	const vector_t remaining = target - next;

	if (XMVectorGetX(XMVector3LengthSq(remaining)) < 0.0001f)
	{
		next = target;
	}

	m_pTransformCom->Set_State(
		STATE::POSITION,
		XMVectorSetW(next, 1.f));

	//G3에서는 위치만 보간하고, Yaw는 서버 최신값을 즉시 반영
	m_pTransformCom->Rotation(
		0.f,
		m_fNetworkTargetYawDegrees,
		0.f);
}

shared_ptr<CModel> CCharacter::Get_BodyModel() const
{
	return m_pBodyModel;
}

void CCharacter::Set_Position(fvector_t vPosition)
{
	m_pTransformCom->Set_State(STATE::POSITION, vPosition);
}

bool_t CCharacter::Apply_NetworkState(const float3_t& position, f32_t yawDegrees, bool_t isMoving)
{
	//client replication이 서버 상태를 character 표현 상태로 전달하는 public 함수이다.
	//character는 S2C_WORLD_SNAPSHOT을 직접 받지 않는다.
	//transform 존재 검사 -> position / yaw 유한성 검사 -> network target position 저장
	//network target yaw 저장 -> m_hasNetworkstate = true -> set locomotion(ismoving)
	if (nullptr == m_pTransformCom ||
		!std::isfinite(position.x) ||
		!std::isfinite(position.y) ||
		!std::isfinite(position.z) ||
		!std::isfinite(yawDegrees))
	{
		return false;
	}
	//network state apply!
	m_hasNetworkState = true;
	m_vNetworkTargetPosition = position;
	m_fNetworkTargetYawDegrees = yawDegrees;

	Set_Locomotion(isMoving);
	return true;
}

bool_t CCharacter::Advance_ComboStage(const std::uint8_t comboStage)
{
	if (nullptr == m_pChain || 0u == comboStage)
		return false;
	const int32_t step = static_cast<int32_t>(comboStage) - 1;
	if (step >= static_cast<int32_t>(m_pChain->clips.size()))
		return false;
	m_iChainStep = step;
	return Start_Clip(m_pChain->clips[step].c_str());
}

bool_t CCharacter::Apply_NetworkAction(
	const LostArk::Shared::PLAYER_ACTION_STATE action,
	const LostArk::Shared::SKILL_ID skillId,
	const std::uint32_t actionStartTick,
	const std::uint8_t comboStage)
{
	using namespace LostArk::Shared;
	if (static_cast<std::uint8_t>(action) >=
		static_cast<std::uint8_t>(PLAYER_ACTION_STATE::END))
	{
		return false;
	}
	if (PLAYER_ACTION_STATE::SKILL == action)
	{
		if (INVALID_SKILL_ID == skillId || 0u == actionStartTick)
			return false;
		if (m_eNetworkAction == action &&
			m_iLastNetworkActionStartTick == actionStartTick)
		{
			return true;
		}
		/* Stage one starts the chain; later stages step the one already running,
		because the chain does not advance itself in COMBO mode. */
		if (comboStage > 1u)
		{
			if (!Advance_ComboStage(comboStage))
				return false;
		}
		else if (!Play_Skill(static_cast<int32_t>(skillId)))
		{
			return false;
		}
		m_iLastNetworkActionStartTick = actionStartTick;
	}
	else if (PLAYER_ACTION_STATE::DEAD == action)
	{
		m_pChain = nullptr;
		m_iChainStep = 0;
		Set_Animation(CHARACTER_ANIM::DEAD, false);
	}
	else if (PLAYER_ACTION_STATE::SKILL == m_eNetworkAction &&
		nullptr != m_pChain)
	{
		m_pChain = nullptr;
		m_iChainStep = 0;
		Set_Animation(
			m_isMoving ? CHARACTER_ANIM::RUN : CHARACTER_ANIM::IDLE,
			true);
	}
	m_eNetworkAction = action;
	return true;
}

bool_t CCharacter::Set_Animation(CHARACTER_ANIM eAnim, bool_t isLoop)
{
	if (eAnim >= CHARACTER_ANIM::END)
		return false;
	return Set_Animation(m_pSpec->AnimationClips[ETOUI(eAnim)], isLoop);
}

bool_t CCharacter::Set_Animation(const char_t* pClipName, bool_t isLoop)
{
	if (nullptr == m_pBodyModel || nullptr == pClipName)
		return false;
	return m_pBodyModel->Set_Animation(
		pClipName,
		isLoop,
		CLIP_BLEND_SECONDS);
}

PATH_RESULT_CODE CCharacter::Request_Move(fvector_t vGoalPosition)
{
	//유효하지 않은 Grid return 예외처리
	if (nullptr == m_pNavigationCom || nullptr == m_pTransformCom)
		return PATH_RESULT_CODE::INVALID_GRID;
	//PathFollower 객체에게 Path 요청
	const PATH_RESULT_CODE eResult = m_PathFollower.Request_Path(
		m_pNavigationCom,
		m_pTransformCom->Get_State(STATE::POSITION),
		vGoalPosition);

	if (PATH_RESULT_CODE::SUCCESS == eResult)
		Set_Locomotion(m_PathFollower.Has_Path());

	return eResult;
}

void CCharacter::Cancel_Move()
{
	m_PathFollower.Cancel();
	Set_Locomotion(false);
}

void CCharacter::Priority_Update(f32_t fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CCharacter::Update(f32_t fTimeDelta)
{
	//network state -> apply snapshot, !networkstate -> pathfinding
	if (m_hasNetworkState)
	{
		Update_NetworkTransform(fTimeDelta);
	}
	else
	{
		m_PathFollower.Update(
			m_pTransformCom,
			m_fMoveSpeed,
			fTimeDelta);

		Set_Locomotion(m_PathFollower.Has_Path());
	}

	//Set_Locomotion(m_PathFollower.Has_Path());

	/* A running chain owns the clip until it ends, so it advances before the logic
	gets a say and Is_PlayingSkill() is already correct when the logic reads it. */
	Update_Chain();

	/* Class code may only update presentation. Input and gameplay commands are
	owned by PlayerController and its command sink. */
	if (m_isLocallyControlled && nullptr != m_pLogic)
		m_pLogic->Update_Presentation(*this, fTimeDelta);

	__super::Update(fTimeDelta);
}

void CCharacter::Late_Update(f32_t fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

#ifdef _DEBUG
	if (m_isNavigationDebugVisible && nullptr != m_pNavigationCom)
		CGameInstance::Get().Add_DebugComponent(m_pNavigationCom);
#endif
}

HRESULT CCharacter::Render()
{
	return S_OK;
}

HRESULT CCharacter::Ready_Components()
{
	if (m_strNavigationPrototypeTag.empty())
		return S_OK;

	return __super::Add_Component(
		m_iPrototypeLevelIndex,
		m_strNavigationPrototypeTag,
		TEXT("Com_Navigation"),
		m_pNavigationCom);
}

HRESULT CCharacter::Ready_PartObjects()
{
	/* Part tags are ordered because CContainerObject keeps parts in a std::map and
	updates them in tag order. The body has to run first: socketed parts read its
	bone matrices during their own Update. */
	CPart_Body::PART_BODY_DESC bodyDesc{};
	bodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
	bodyDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
	bodyDesc.strModelTag = m_pSpec->pBodyModelTag;
	bodyDesc.strShaderTag = m_pSpec->pShaderTag;
	bodyDesc.iHiddenMeshMask = m_pSpec->iBodyHiddenMeshMask;
	bodyDesc.pInitialAnimation = m_pSpec->AnimationClips[ETOUI(CHARACTER_ANIM::IDLE)];

	if (FAILED(__super::Add_PartObject(
		m_iPrototypeLevelIndex,
		TEXT("Prototype_GameObject_Part_Body"),
		TEXT("Part_00_Body"),
		&bodyDesc)))
		return E_FAIL;

	m_pBodyModel = dynamic_pointer_cast<CModel>(
		__super::Get_Component(TEXT("Part_00_Body"), TEXT("Com_Model")));
	if (nullptr == m_pBodyModel)
		return E_FAIL;

	/* Skinned equipment: no socket, it borrows the body's bone palette. */
	for (uint32_t i = 0; i < m_pSpec->iNumEquipment; ++i)
	{
		CPart_Equipment::PART_EQUIPMENT_DESC equipmentDesc{};
		equipmentDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
		equipmentDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
		equipmentDesc.strModelTag = m_pSpec->pEquipment[i].pModelTag;
		equipmentDesc.strShaderTag = m_pSpec->pShaderTag;
		equipmentDesc.pSkeletonModel = m_pBodyModel;

		if (FAILED(__super::Add_PartObject(
			m_iPrototypeLevelIndex,
			TEXT("Prototype_GameObject_Part_Equipment"),
			m_pSpec->pEquipment[i].pPartTag,
			&equipmentDesc)))
			return E_FAIL;
	}

	/* The weapon is the same part class in socket mode. It gets its own class once
	hit windows and trails need somewhere to live. */
	for (uint32_t i = 0; i < m_pSpec->iNumWeapons; ++i)
	{
		CPart_Equipment::PART_EQUIPMENT_DESC weaponDesc{};
		weaponDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
		weaponDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
		weaponDesc.strModelTag = m_pSpec->pWeapons[i].pModelTag;
		weaponDesc.strShaderTag = m_pSpec->pWeaponShaderTag;
		weaponDesc.pSkeletonModel = m_pBodyModel;
		weaponDesc.pSocketBoneName = m_pSpec->pWeapons[i].pSocketBone;
		weaponDesc.fSocketYawDegrees = m_pSpec->pWeapons[i].fSocketYawDegrees;

		if (FAILED(__super::Add_PartObject(
			m_iPrototypeLevelIndex,
			TEXT("Prototype_GameObject_Part_Equipment"),
			m_pSpec->pWeapons[i].pPartTag,
			&weaponDesc)))
			return E_FAIL;
	}

	return S_OK;
}

void CCharacter::Set_Locomotion(bool_t isMoving)
{
	if (m_isMoving == isMoving)
		return;

	m_isMoving = isMoving;
	if (Is_PlayingSkill())
		return;

	Set_Animation(
		isMoving ? CHARACTER_ANIM::RUN : CHARACTER_ANIM::IDLE,
		true);
}

unique_ptr<CCharacter> CCharacter::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CCharacter>(new CCharacter(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		OutputDebugStringA("[Client][Character] Create failed.\n");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CCharacter::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CCharacter>(new CCharacter(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		OutputDebugStringA("[Client][Character] Clone failed.\n");
		return nullptr;
	}
	return pInstance;
}
