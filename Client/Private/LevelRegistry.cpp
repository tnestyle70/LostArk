#include "LevelRegistry.h"

#include "Level_Bern.h"
#include "Level_CharacterSelect.h"
#include "Level_Development.h"
#include "Level_Lobby.h"
#include "Level_ValtanArena.h"
#include "Loader.h"

#include <algorithm>
#include <array>
#include <limits>

namespace
{
	using namespace Client;
	using namespace Engine;

	MAP_LOAD_SCOPE MakeFullMapScope(
		const char_t* excludedAssetGroupId = nullptr)
	{
		MAP_LOAD_SCOPE scope
		{
			true,
			true,
			(std::numeric_limits<f32_t>::lowest)(),
			(std::numeric_limits<f32_t>::lowest)(),
			(std::numeric_limits<f32_t>::max)(),
			(std::numeric_limits<f32_t>::max)()
		};
		if (nullptr != excludedAssetGroupId)
			scope.excludedAssetGroupId = excludedAssetGroupId;
		return scope;
	}

	MAP_LOAD_SCOPE MakeBernMapScope()
	{
		MAP_LOAD_SCOPE scope = MakeFullMapScope();
		/* The Bern-only bypass proved that the camera-dependent popping came
		   from false frustum rejection. Product rendering now uses normal
		   culling again, with final-render camera snapshots, rebuilt bounds and
		   conservative rejection below. Keep the switches explicit so the same
		   diagnostic can be staged again without changing other levels. */
		scope.frustumCulling.bypass = false;
		scope.frustumCulling.diagnostics = false;
		scope.frustumCulling.baseMargin = 0.25f;
		scope.frustumCulling.largeObjectRadiusThreshold = 4.f;
		scope.frustumCulling.largeObjectAbsoluteMargin = 2.f;
		scope.frustumCulling.largeObjectRelativeMargin = 0.12f;
		scope.frustumCulling.rejectHysteresisFrames = 3u;
		return scope;
	}

	unique_ptr<CLevel> CreateLobby(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext)
	{
		return CLevel_Lobby::Create(pDevice, pContext);
	}

	unique_ptr<CLevel> CreateBern(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext)
	{
		return CLevel_Bern::Create(pDevice, pContext);
	}

	unique_ptr<CLevel> CreateCharacterSelect(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext)
	{
		return CLevel_CharacterSelect::Create(pDevice, pContext);
	}

	unique_ptr<CLevel> CreateValtanArena(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext)
	{
		return CLevel_ValtanArena::Create(pDevice, pContext);
	}

	unique_ptr<CLevel> CreateDevelopment(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext)
	{
		return CLevel_Development::Create(pDevice, pContext);
	}

}

const CLIENT_LEVEL_DESCRIPTOR* CLevelRegistry::Find(
	const LEVEL eLevel)
{
	static const std::array<CLIENT_LEVEL_DESCRIPTOR, 5> levels =
	{{
		{
			LEVEL::LOBBY,
			CLIENT_LEVEL_KIND::PRODUCT,
			"front.lobby",
			nullptr,
			"scene.lobby.neutral.v1",
			{},
			CreateLobby,
			&CLoader::Ready_For_Lobby
		},
		{
			LEVEL::CHARACTER_SELECT,
			CLIENT_LEVEL_KIND::PRODUCT,
			"front.character-select",
			"LV_LOBBY_CLASSSELECT_SL00",
			"scene.character-select.warm-high-key.v1",
			{ true, true, -792.f, 158.f, -750.f, 218.f },
			CreateCharacterSelect,
			&CLoader::Ready_For_CharacterSelect
		},
		{
			LEVEL::BERN,
			CLIENT_LEVEL_KIND::PRODUCT,
			"world.bern",
			"LV_BER_BERNCASTLE",
			"scene.bern.neutral-day.v1",
			// The 42 landscape components are the ground under the whole walkable
			// approach, so excluding them left the player on static-mesh paving with
			// void everywhere else. The Map Editor keeps its own reversible
			// "Show Bern Landscape" toggle for authoring; the product level loads
			// the full area.
			MakeBernMapScope(),
			CreateBern,
			&CLoader::Ready_For_Bern
		},
		{
			LEVEL::VALTAN_ARENA,
			CLIENT_LEVEL_KIND::PRODUCT,
			"raid.valtan.arena",
			"LV_LUT_HEARTRB_ED",
			"scene.valtan.cool-low-key.v1",
			MakeFullMapScope(),
			CreateValtanArena,
			&CLoader::Ready_For_ValtanArena
		},
		{
			LEVEL::DEVELOPMENT,
			CLIENT_LEVEL_KIND::DEVELOPMENT,
			"dev.training.ground",
			"LV_DEV_TRAINING_GROUND",
			"scene.development.neutral.v1",
			{ true, false, -20.f, -20.f, 20.f, 20.f },
			CreateDevelopment,
			&CLoader::Ready_For_Development
		}
	}};

	const auto iter = std::find_if(
		levels.begin(),
		levels.end(),
		[eLevel](const CLIENT_LEVEL_DESCRIPTOR& descriptor)
		{
			return descriptor.eLevel == eLevel;
		});
	return levels.end() == iter ? nullptr : &(*iter);
}

unique_ptr<CLevel> CLevelRegistry::Create_Level(
	const LEVEL eLevel,
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	const CLIENT_LEVEL_DESCRIPTOR* pDescriptor = Find(eLevel);
	if (nullptr == pDescriptor || nullptr == pDescriptor->pCreate)
		return nullptr;

	return pDescriptor->pCreate(
		std::move(pDevice),
		std::move(pContext));
}

HRESULT CLevelRegistry::Execute_Load(
	const LEVEL eLevel,
	CLoader& loader)
{
	const CLIENT_LEVEL_DESCRIPTOR* pDescriptor = Find(eLevel);
	if (nullptr == pDescriptor || nullptr == pDescriptor->pLoad)
		return E_INVALIDARG;

	return (loader.*pDescriptor->pLoad)();
}
