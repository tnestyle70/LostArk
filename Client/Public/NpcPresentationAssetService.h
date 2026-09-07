#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string_view>
#include <memory>
#include <vector>

namespace Engine { class CModel; }

NS_BEGIN(Client)

class CNpcPresentationAssetService final
{
public:
	static void Begin_LevelLoad(uint32_t iLevelIndex);
	static HRESULT Ensure_Prototypes(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iLevelIndex,
		std::string_view archetypeId);
	static bool_t Is_Ready(
		uint32_t iLevelIndex,
		std::string_view archetypeId);

	/* Stable per-archetype CModel prototype tag, derived from the catalog
	modelAssetId stem. Empty when the archetype is not in the catalog. */
	static wstring_t Get_ModelPrototypeTag(std::string_view archetypeId);
	// Body source seconds drive the existing animated hammer, with rest fallback.
	static void Synchronize_SaydonHammerPose(const std::shared_ptr<Engine::CModel>& body,
		const std::shared_ptr<Engine::CModel>& weapon, const std::vector<float4x4_t>& restPose);
};

NS_END
