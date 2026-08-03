#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketType.h"

#include <atomic>

NS_BEGIN(Client)

// Owns the one runtime path that admits playable character model prototypes.
// The level loader admits the locally selected class first. Replication uses
// the same service when a remote class is observed for the first time.
class CPlayableCharacterAssetService final
{
public:
	static void Begin_LevelLoad(uint32_t iLevelIndex);
	static HRESULT Ensure_Prototypes(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iLevelIndex,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		const std::atomic_bool* pCancellationRequested = nullptr);
	static bool_t Is_Ready(
		uint32_t iLevelIndex,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
};

NS_END
