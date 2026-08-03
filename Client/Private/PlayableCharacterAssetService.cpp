#include "PlayableCharacterAssetService.h"

#include "ActorCatalog.h"
#include "GameInstance.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"

#include <array>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
	struct CHARACTER_PROTOTYPE_TAGS final
	{
		const tchar_t* pBody = nullptr;
		std::array<const tchar_t*, 5> Equipment{};
		const tchar_t* pWeapon = nullptr;
		bool_t isLanceMasterWeaponScale = false;
	};

	std::mutex g_CharacterAssetMutex;
	std::unordered_map<uint32_t, std::unordered_set<uint8_t>>
		g_ReadyClassesByLevel;

	const CHARACTER_PROTOTYPE_TAGS* Find_Tags(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		static const CHARACTER_PROTOTYPE_TAGS LANCE_MASTER
		{
			TEXT("Prototype_Component_Model_LanceMaster"),
			{
				TEXT("Prototype_Component_Model_LanceMaster_Upper"),
				TEXT("Prototype_Component_Model_LanceMaster_Lower"),
				TEXT("Prototype_Component_Model_LanceMaster_Arm"),
				TEXT("Prototype_Component_Model_LanceMaster_Shoulder"),
				TEXT("Prototype_Component_Model_LanceMaster_Helmet")
			},
			TEXT("Prototype_Component_Model_LanceMaster_Weapon"),
			true
		};
		static const CHARACTER_PROTOTYPE_TAGS GUNSLINGER
		{
			TEXT("Prototype_Component_Model_GunSlinger"),
			{
				TEXT("Prototype_Component_Model_GunSlinger_Upper"),
				TEXT("Prototype_Component_Model_GunSlinger_Lower"),
				TEXT("Prototype_Component_Model_GunSlinger_Arm"),
				TEXT("Prototype_Component_Model_GunSlinger_Shoulder"),
				TEXT("Prototype_Component_Model_GunSlinger_Helmet")
			},
			TEXT("Prototype_Component_Model_GunSlinger_Weapon"),
			false
		};
		static const CHARACTER_PROTOTYPE_TAGS SLAYER
		{
			TEXT("Prototype_Component_Model_Slayer"),
			{
				TEXT("Prototype_Component_Model_Slayer_Upper"),
				TEXT("Prototype_Component_Model_Slayer_Lower"),
				TEXT("Prototype_Component_Model_Slayer_Arm"),
				TEXT("Prototype_Component_Model_Slayer_Shoulder"),
				TEXT("Prototype_Component_Model_Slayer_Helmet")
			},
			TEXT("Prototype_Component_Model_Slayer_Weapon"),
			false
		};
		static const CHARACTER_PROTOTYPE_TAGS ARTIST
		{
			TEXT("Prototype_Component_Model_Artist"),
			{
				TEXT("Prototype_Component_Model_Artist_Upper"),
				TEXT("Prototype_Component_Model_Artist_Lower"),
				TEXT("Prototype_Component_Model_Artist_Arm"),
				TEXT("Prototype_Component_Model_Artist_Shoulder"),
				TEXT("Prototype_Component_Model_Artist_Helmet")
			},
			TEXT("Prototype_Component_Model_Artist_Weapon"),
			false
		};

		switch (characterClass)
		{
		case LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER:
			return &LANCE_MASTER;
		case LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER:
			return &GUNSLINGER;
		case LostArk::Shared::CHARACTER_CLASS_ID::SLAYER:
			return &SLAYER;
		case LostArk::Shared::CHARACTER_CLASS_ID::ARTIST:
			return &ARTIST;
		default:
			return nullptr;
		}
	}

	bool_t Is_Cancelled(const std::atomic_bool* pCancellationRequested)
	{
		return nullptr != pCancellationRequested &&
			pCancellationRequested->load(std::memory_order_acquire);
	}
}

void Client::CPlayableCharacterAssetService::Begin_LevelLoad(
	const uint32_t iLevelIndex)
{
	std::scoped_lock lock{ g_CharacterAssetMutex };
	g_ReadyClassesByLevel.erase(iLevelIndex);
}

HRESULT Client::CPlayableCharacterAssetService::Ensure_Prototypes(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iLevelIndex,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const std::atomic_bool* pCancellationRequested)
{
	if (nullptr == pDevice || nullptr == pContext ||
		iLevelIndex >= ETOUI(LEVEL::END) ||
		!LostArk::Shared::Is_Supported_Playable_Character_Class(
			characterClass))
	{
		return E_INVALIDARG;
	}

	std::scoped_lock lock{ g_CharacterAssetMutex };
	const uint8_t classKey = static_cast<uint8_t>(characterClass);
	if (g_ReadyClassesByLevel[iLevelIndex].contains(classKey))
		return S_FALSE;

	if (Is_Cancelled(pCancellationRequested))
		return HRESULT_FROM_WIN32(ERROR_CANCELLED);

	const CHARACTER_ACTOR_ENTRY* pActor =
		CActorCatalog::Find_Character(characterClass);
	const CHARACTER_PROTOTYPE_TAGS* pTags = Find_Tags(characterClass);
	if (nullptr == pActor || nullptr == pTags ||
		pActor->runtimeStatus != "supported" ||
		pActor->equipmentModels.size() != pTags->Equipment.size())
	{
		return E_FAIL;
	}

	const matrix_t characterTransform =
		XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) *
		XMMatrixRotationY(XMConvertToRadians(-90.f));
	const matrix_t weaponTransform = pTags->isLanceMasterWeaponScale ?
		XMMatrixScaling(100.f, 100.f, 100.f) : XMMatrixIdentity();

	std::vector<std::pair<std::wstring, unique_ptr<CPrototype>>> staged;
	staged.reserve(2u + pTags->Equipment.size());
	const auto StageModel = [&staged, &pDevice, &pContext](
		const tchar_t* pTag,
		const std::string& assetId,
		const MODEL modelType,
		const matrix_t& transform)
	{
		const std::filesystem::path path =
			CRuntimeAssetRoot::Resolve(assetId);
		if (nullptr == pTag || path.empty())
			return false;
		unique_ptr<CPrototype> pModel = CModel::Create(
			pDevice,
			pContext,
			modelType,
			path.string().c_str(),
			transform);
		if (nullptr == pModel)
			return false;
		staged.emplace_back(pTag, std::move(pModel));
		return true;
	};

	if (!StageModel(
		pTags->pBody,
		pActor->bodyModel,
		MODEL::ANIM,
		characterTransform))
	{
		return E_FAIL;
	}

	for (size_t index = 0; index < pActor->equipmentModels.size(); ++index)
	{
		if (Is_Cancelled(pCancellationRequested))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);
		if (!StageModel(
			pTags->Equipment[index],
			pActor->equipmentModels[index],
			MODEL::ANIM,
			characterTransform))
		{
			return E_FAIL;
		}
	}

	if (Is_Cancelled(pCancellationRequested))
		return HRESULT_FROM_WIN32(ERROR_CANCELLED);
	if (!StageModel(
		pTags->pWeapon,
		pActor->weaponModel,
		MODEL::NONANIM,
		weaponTransform))
	{
		return E_FAIL;
	}

	// All binary models are decoded before the first prototype is committed.
	// The mutex makes duplicate admission impossible inside this service; with
	// valid level indices, the following adds therefore commit as one unit.
	for (auto& [tag, pPrototype] : staged)
	{
		if (FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			tag,
			std::move(pPrototype))))
		{
			return E_FAIL;
		}
	}

	g_ReadyClassesByLevel[iLevelIndex].insert(classKey);
	return S_OK;
}

bool_t Client::CPlayableCharacterAssetService::Is_Ready(
	const uint32_t iLevelIndex,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	std::scoped_lock lock{ g_CharacterAssetMutex };
	const auto level = g_ReadyClassesByLevel.find(iLevelIndex);
	return level != g_ReadyClassesByLevel.end() &&
		level->second.contains(static_cast<uint8_t>(characterClass));
}
