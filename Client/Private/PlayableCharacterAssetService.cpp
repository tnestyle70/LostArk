#include "PlayableCharacterAssetService.h"

#include "ActorCatalog.h"
#include "BinaryAsset/ModelAssetData.h"
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
		/* Eight, not five: hair rides along as an equipment part next to the five
		armour pieces -- every class wears its hairstyle as a part rather than
		drawing one from the body -- and a class with an avatar head/armor slot
		adds two more next to its own five. */
		std::array<const tchar_t*, 8> Equipment{};
		size_t iEquipmentCount = 0;
		std::array<const tchar_t*, 4> Weapons{};
		size_t iWeaponCount = 0;
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
				TEXT("Prototype_Component_Model_LanceMaster_Helmet"),
				TEXT("Prototype_Component_Model_LanceMaster_Helmet_Mokoko"),
				TEXT("Prototype_Component_Model_LanceMaster_Upper_Mokoko"),
				TEXT("Prototype_Component_Model_LanceMaster_Hair")
			},
			8u,
			{ TEXT("Prototype_Component_Model_LanceMaster_Weapon"),
			  TEXT("Prototype_Component_Model_LanceMaster_Weapon_Short") },
			2u
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
			5u,
			{ TEXT("Prototype_Component_Model_GunSlinger_Weapon") },
			1u
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
			5u,
			{ TEXT("Prototype_Component_Model_Slayer_Weapon") },
			1u
		};
		static const CHARACTER_PROTOTYPE_TAGS ARTIST
		{
			TEXT("Prototype_Component_Model_Artist"),
			{
				TEXT("Prototype_Component_Model_Artist_Upper"),
				TEXT("Prototype_Component_Model_Artist_Lower"),
				TEXT("Prototype_Component_Model_Artist_Arm"),
				TEXT("Prototype_Component_Model_Artist_Shoulder"),
				TEXT("Prototype_Component_Model_Artist_Helmet"),
				TEXT("Prototype_Component_Model_Artist_Hair")
			},
			6u,
			{ TEXT("Prototype_Component_Model_Artist_Weapon") },
			1u
		};
		static const CHARACTER_PROTOTYPE_TAGS DIMENSIONMASTER
		{
			TEXT("Prototype_Component_Model_DimensionMaster"),
			{ TEXT("Prototype_Component_Model_DimensionMaster_Hair") },
			1u,
			{
				TEXT("Prototype_Component_Model_DimensionMaster_Weapon_L"),
				TEXT("Prototype_Component_Model_DimensionMaster_Weapon_S"),
				TEXT("Prototype_Component_Model_DimensionMaster_Weapon_P"),
				TEXT("Prototype_Component_Model_DimensionMaster_Weapon_E")
			},
			4u
		};

		static const CHARACTER_PROTOTYPE_TAGS WARLORD
		{
			TEXT("Prototype_Component_Model_Warlord"),
			{
				TEXT("Prototype_Component_Model_Warlord_Upper"),
				TEXT("Prototype_Component_Model_Warlord_Lower"),
				TEXT("Prototype_Component_Model_Warlord_Arm"),
				TEXT("Prototype_Component_Model_Warlord_Shoulder"),
				TEXT("Prototype_Component_Model_Warlord_Helmet"),
				TEXT("Prototype_Component_Model_Warlord_Hair")
			},
			6u,
			{ TEXT("Prototype_Component_Model_Warlord_Weapon"),
			  TEXT("Prototype_Component_Model_Warlord_Shield") },
			2u
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
		case LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER:
			return &DIMENSIONMASTER;
		case LostArk::Shared::CHARACTER_CLASS_ID::WARLORD:
			return &WARLORD;
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
	const std::atomic_bool* pCancellationRequested,
	const PROGRESS_CALLBACK& progress)
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
		pActor->runtimeStatus != "supported")
	{
		return E_FAIL;
	}
	if (pActor->equipmentModels.size() != pTags->iEquipmentCount ||
		pActor->weaponModels.size() != pTags->iWeaponCount)
	{
		return E_FAIL;
	}

	// The DimensionMaster body was cooked from the ActorX Blender intake at roughly
	// centimeter scale (about 111 units tall). The older UModel character pack
	// keeps its existing 0.0001 admission transform.
	const f32_t characterScale =
		LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER == characterClass ?
		0.01f : 0.0001f;
	const matrix_t characterTransform =
		XMMatrixScaling(
			characterScale,
			characterScale,
			characterScale) *
		XMMatrixRotationY(XMConvertToRadians(-90.f));

    const auto loadDescription = [pActor](const std::string& assetId,
        MODEL_ASSET_LOAD_DESC& description) {
        std::string status;
        if (CActorCatalog::Build_ModelLoadDescription(assetId, description, status, pActor->assetId))
            return true;
        OutputDebugStringA(("[PlayableCharacterAssetService] " + status + "\n").c_str());
        return false;
    };
	std::vector<std::pair<std::wstring, unique_ptr<CPrototype>>> staged;
	const size_t totalModelCount =
		1u + pTags->iEquipmentCount + pTags->iWeaponCount;
	staged.reserve(totalModelCount);
	CModel* pBodyPalette = nullptr;
	const auto StageModel = [
		&staged,
		&pBodyPalette,
		&pDevice,
		&pContext,
		&progress,
        &loadDescription,
		characterClass,
		totalModelCount](
		const tchar_t* pTag,
		const std::string& assetId,
		const MODEL modelType,
		const matrix_t& transform)
	{
		if (progress)
			progress(staged.size(), totalModelCount, assetId);
		const std::filesystem::path path =
			CRuntimeAssetRoot::Resolve(assetId);
		if (nullptr == pTag || path.empty())
			return false;
		MODEL_ASSET_LOAD_DESC description;
		if (!loadDescription(assetId, description)) return false;
		unique_ptr<CModel> pModel = CModel::Create(
			pDevice,
			pContext,
			modelType,
			description,
			transform);
		if (nullptr == pModel)
			return false;
		bool requiresBodyPalette = false;
		// This source-normalized rig contract belongs to the DimensionMaster pack.
		if (modelType == MODEL::ANIM &&
			characterClass == LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER)
		{
			for (uint32_t mesh = 0; mesh < pModel->Get_NumMeshes(); ++mesh)
			{
				const auto* surface = pModel->Get_MaterialSurface(mesh);
				requiresBodyPalette = requiresBodyPalette || (surface &&
					surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
					(surface->sourceCharacter.program == 6u || surface->sourceCharacter.program == 7u));
			}
		}
		if (requiresBodyPalette)
		{
			const auto boneNames = pModel->Get_BoneNames();
			bool compatible = pBodyPalette &&
				pModel->Get_SkeletonHash() == pBodyPalette->Get_SkeletonHash() &&
				boneNames == pBodyPalette->Get_BoneNames();
			for (uint32_t bone = 0; compatible && bone < boneNames.size(); ++bone)
			{
				matrix_t equipmentRest, bodyRest;
				compatible = pModel->Get_BoneParentIndex(bone) == pBodyPalette->Get_BoneParentIndex(bone) &&
					pModel->Get_BoneRestLocalMatrix(bone, equipmentRest) &&
					pBodyPalette->Get_BoneRestLocalMatrix(bone, bodyRest);
				for (uint32_t row = 0; compatible && row < 4u; ++row)
					compatible = XMVector4Equal(equipmentRest.r[row], bodyRest.r[row]);
			}
			if (!compatible)
			{
				OutputDebugStringA(("[PlayableCharacterAssetService] Equipment body palette mismatch: " +
					assetId + "\n").c_str());
				return false;
			}
		}
		staged.emplace_back(pTag, std::move(pModel));
		if (progress)
			progress(staged.size(), totalModelCount, assetId);
		return true;
	};

	{
		if (progress)
			progress(staged.size(), totalModelCount, pActor->bodyModel);
		const std::filesystem::path bodyPath =
			CRuntimeAssetRoot::Resolve(pActor->bodyModel);
		if (nullptr == pTags->pBody || bodyPath.empty())
			return E_FAIL;
		MODEL_ASSET_LOAD_DESC description;
		if (!loadDescription(pActor->bodyModel, description)) return E_FAIL;
		unique_ptr<CModel> pBodyModel = CModel::Create(
			pDevice,
			pContext,
			MODEL::ANIM,
			description,
			characterTransform);
		if (nullptr == pBodyModel)
			return E_FAIL;
		/* Shared clips the class borrows rather than cooks into its body -- the
		Esther summon cast, the character-creation idle -- ship as separate
		animation sets. Attaching is fail-closed on skeleton hash and clip
		collisions, so one bad set fails this class's admission rather than
		leaving the character half-built. */
		for (const std::string& animationSetModel : pActor->animationSetModels)
		{
			const std::filesystem::path animSetPath =
				CRuntimeAssetRoot::Resolve(animationSetModel);
			if (animSetPath.empty())
				return E_FAIL;
			const unique_ptr<CModel> pAnimSet = CModel::Create(
				pDevice,
				pContext,
				MODEL::ANIM,
				animSetPath.string().c_str(),
				characterTransform);
			if (nullptr == pAnimSet ||
				FAILED(pBodyModel->Attach_AnimationSet(*pAnimSet)))
			{
				return E_FAIL;
			}
		}
		pBodyPalette = pBodyModel.get();
		staged.emplace_back(pTags->pBody, std::move(pBodyModel));
		if (progress)
			progress(staged.size(), totalModelCount, pActor->bodyModel);
	}

	for (size_t index = 0; index < pTags->iEquipmentCount; ++index)
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

	for (size_t index = 0; index < pTags->iWeaponCount; ++index)
	{
		if (Is_Cancelled(pCancellationRequested))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);
		if (!StageModel(
			pTags->Weapons[index],
			pActor->weaponModels[index],
			MODEL::NONANIM,
			XMMatrixIdentity()))
		{
			return E_FAIL;
		}
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
