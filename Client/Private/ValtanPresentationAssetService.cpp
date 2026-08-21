#include "ValtanPresentationAssetService.h"

#include "ActorCatalog.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "Body_Valtan.h"
#include "GameInstance.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"
#include "Valtan.h"

#include <mutex>
#include <system_error>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
	std::mutex g_ValtanAssetMutex;
	std::unordered_set<uint32_t> g_ReadyLevels;
}

void Client::CValtanPresentationAssetService::Begin_LevelLoad(
	const uint32_t iLevelIndex)
{
	std::scoped_lock lock{ g_ValtanAssetMutex };
	g_ReadyLevels.erase(iLevelIndex);
}

HRESULT Client::CValtanPresentationAssetService::Ensure_Prototypes(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iLevelIndex)
{
	if (nullptr == pDevice || nullptr == pContext ||
		iLevelIndex >= ETOUI(LEVEL::END))
	{
		return E_INVALIDARG;
	}

	std::scoped_lock lock{ g_ValtanAssetMutex };
	if (g_ReadyLevels.contains(iLevelIndex))
		return S_FALSE;

	const BOSS_ACTOR_ENTRY* pActor = CActorCatalog::Find_Boss("BOSS_VALTAN");
	if (nullptr == pActor ||
		pActor->clientPresentationId != "boss.valtan.client.v1")
	{
		return E_FAIL;
	}

	const std::filesystem::path bodyPath =
		CRuntimeAssetRoot::Resolve(pActor->bodyModel);
	const std::filesystem::path weaponPath =
		CRuntimeAssetRoot::Resolve(pActor->weaponModel);
	const std::filesystem::path animSetPath =
		CRuntimeAssetRoot::Resolve(pActor->animationSetId);
	if (bodyPath.empty() || weaponPath.empty() || animSetPath.empty())
		return E_FAIL;
	std::vector<std::pair<uint32_t, std::filesystem::path>> armorAssets;
	armorAssets.reserve(pActor->armorParts.size());
	for (const BOSS_ARMOR_PART_ENTRY& armorPart : pActor->armorParts)
	{
		std::filesystem::path armorPath =
			CRuntimeAssetRoot::Resolve(armorPart.modelAssetId);
		if (armorPath.empty())
			return E_FAIL;
		armorAssets.emplace_back(armorPart.stateMask, std::move(armorPath));
	}
	std::error_code animSetFileError;
	const bool_t hasAnimSetFile = std::filesystem::is_regular_file(
		animSetPath, animSetFileError);
	const bool_t isMissingAnimSetFile = !hasAnimSetFile &&
		(ERROR_FILE_NOT_FOUND == animSetFileError.value() ||
			ERROR_PATH_NOT_FOUND == animSetFileError.value());
	if (!hasAnimSetFile && !isMissingAnimSetFile)
		return E_FAIL;

#ifndef _DEBUG
	if (isMissingAnimSetFile)
		return E_FAIL;
#else
	if (!hasAnimSetFile)
	{
		::OutputDebugStringA(
			"[ValtanPresentation] The authored 146-clip animation set is missing. "
			"Debug will continue with the clips embedded in the Valtan body model; "
			"Release remains fail-closed. Missing asset: "
			"Character/Valtan/AnimSets/MN_RPBF_01_AnimSet.wmodel\n");
	}
#endif

	unique_ptr<CModel> bodyModel = CModel::Create(
		pDevice,
		pContext,
		MODEL::ANIM,
		bodyPath.string().c_str(),
		XMMatrixScaling(0.0001f, 0.0001f, 0.0001f));
	if (nullptr == bodyModel)
		return E_FAIL;
	if (hasAnimSetFile)
	{
		const unique_ptr<CModel> animSetModel = CModel::Create(
			pDevice,
			pContext,
			MODEL::ANIM,
			animSetPath.string().c_str(),
			XMMatrixScaling(0.0001f, 0.0001f, 0.0001f));
		if (nullptr == animSetModel ||
			FAILED(bodyModel->Attach_AnimationSet(*animSetModel)))
		{
			return E_FAIL;
		}
	}

	std::vector<std::pair<std::wstring, unique_ptr<CPrototype>>> staged;
	staged.reserve(4u + armorAssets.size());
	staged.emplace_back(
		TEXT("Prototype_Component_Model_Valtan"),
		std::move(bodyModel));
	staged.emplace_back(
		TEXT("Prototype_Component_Model_ValtanWeapon"),
		CModel::Create(
			pDevice,
			pContext,
			MODEL::NONANIM,
			weaponPath.string().c_str(),
			XMMatrixScaling(100.f, 100.f, 100.f)));
	/* Armour rides the body rig, so it is cooked against the same skeleton
	and pre-transformed exactly like the body. It carries no animation of
	its own: the skinned part borrows the palette the body model built. */
	for (const auto& [stateMask, armorPath] : armorAssets)
	{
		unique_ptr<CModel> armorModel = CModel::Create(
			pDevice,
			pContext,
			MODEL::ANIM,
			armorPath.string().c_str(),
			XMMatrixScaling(0.0001f, 0.0001f, 0.0001f));
		/* A plate is presentation. An unreadable one is dropped here so the
		boss still reaches its prototypes and spawns; the part loop skips the
		tag it cannot find. */
		if (nullptr == armorModel)
		{
			OutputDebugStringA(("[Client][Valtan] armour plate rejected: " +
				armorPath.string() + " | " +
				CModelDecoderRegistry::Get().Get_LastReport().error +
				"\n").c_str());
			continue;
		}
		staged.emplace_back(
			CValtan::Build_ArmorModelPrototypeTag(stateMask),
			std::move(armorModel));
	}
	staged.emplace_back(
		TEXT("Prototype_GameObject_Body_Valtan"),
		CBody_Valtan::Create(pDevice, pContext));
	staged.emplace_back(
		TEXT("Prototype_GameObject_Valtan"),
		CValtan::Create(pDevice, pContext));

	for (const auto& [tag, prototype] : staged)
	{
		(void)tag;
		if (nullptr == prototype)
			return E_FAIL;
	}
	if (FAILED(CGameInstance::Get().Add_Prototypes(
		iLevelIndex,
		std::move(staged))))
	{
		return E_FAIL;
	}

	g_ReadyLevels.insert(iLevelIndex);
	return S_OK;
}

bool_t Client::CValtanPresentationAssetService::Is_Ready(
	const uint32_t iLevelIndex)
{
	std::scoped_lock lock{ g_ValtanAssetMutex };
	return g_ReadyLevels.contains(iLevelIndex);
}
