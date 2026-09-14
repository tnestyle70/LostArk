#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string_view>
#include <memory>
#include <vector>

namespace Engine { class CModel; }

NS_BEGIN(Client)

class CWorldSequenceObject;
struct SAYDON_WEAPON_REPLACEMENT;

class CNpcPresentationAssetService final
{
public:
	static void Begin_LevelLoad(uint32_t iLevelIndex);
	static HRESULT Ensure_Prototypes(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iLevelIndex,
		std::string_view archetypeId);
	/* Registers the shared CNpc GameObject prototype once per level for a body
	that is not an NpcCatalog archetype (a vehicle previewed in a tool). */
	static HRESULT Ensure_ObjectPrototype(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iLevelIndex);
	static bool_t Is_Ready(
		uint32_t iLevelIndex,
		std::string_view archetypeId);

	/* Stable per-archetype CModel prototype tag, derived from the catalog
	modelAssetId stem. Empty when the archetype is not in the catalog. */
	static wstring_t Get_ModelPrototypeTag(std::string_view archetypeId);
	// Body source seconds drive the existing animated hammer, with rest fallback.
	static void Synchronize_SaydonHammerPose(const std::shared_ptr<Engine::CModel>& body,
		const std::shared_ptr<Engine::CModel>& weapon, const std::vector<float4x4_t>& restPose);
	// The active WORLD object owns this registration until it is released or reused.
	static void Track_SaydonWeaponReplacement(std::shared_ptr<const SAYDON_WEAPON_REPLACEMENT>& registration,
		const std::shared_ptr<Engine::CModel>& body, const std::shared_ptr<CWorldSequenceObject>& object);
	static bool_t Is_SaydonHammerSuppressed(const std::shared_ptr<Engine::CModel>& body);
};

NS_END
