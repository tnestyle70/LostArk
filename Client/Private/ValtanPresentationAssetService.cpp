#include "ValtanPresentationAssetService.h"

#include "ActorCatalog.h"
#include "Body_Valtan.h"
#include "GameInstance.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"
#include "Valtan.h"

#include <mutex>
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
	if (bodyPath.empty() || weaponPath.empty())
		return E_FAIL;

	std::vector<std::pair<std::wstring, unique_ptr<CPrototype>>> staged;
	staged.reserve(4u);
	staged.emplace_back(
		TEXT("Prototype_Component_Model_Valtan"),
		CModel::Create(
			pDevice,
			pContext,
			MODEL::ANIM,
			bodyPath.string().c_str(),
			XMMatrixScaling(0.0001f, 0.0001f, 0.0001f)));
	staged.emplace_back(
		TEXT("Prototype_Component_Model_ValtanWeapon"),
		CModel::Create(
			pDevice,
			pContext,
			MODEL::NONANIM,
			weaponPath.string().c_str(),
			XMMatrixScaling(100.f, 100.f, 100.f)));
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
