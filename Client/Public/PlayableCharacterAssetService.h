#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketType.h"

#include <atomic>
#include <cstddef>
#include <functional>
#include <string>

NS_BEGIN(Client)

// Owns the one runtime path that admits playable character model prototypes.
// The level loader admits the locally selected class first. Replication uses
// the same service when a remote class is observed for the first time.
class CPlayableCharacterAssetService final
{
public:
	using PROGRESS_CALLBACK = std::function<void(
		size_t completedModelCount,
		size_t totalModelCount,
		const std::string& assetId)>;

	static void Begin_LevelLoad(uint32_t iLevelIndex);
	static void Cancel_LevelPreparations(uint32_t iLevelIndex);
	static void Shutdown();
	static HRESULT Ensure_Prototypes(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iLevelIndex,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		const std::atomic_bool* pCancellationRequested = nullptr,
		const PROGRESS_CALLBACK& progress = {});
	// Queues CPU decode and D3D device-only resource creation on one bounded
	// worker. Commit_PrototypePreparation must be pumped on the main thread.
	static HRESULT Queue_PrototypePreparation(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iLevelIndex,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	static HRESULT Commit_PrototypePreparation(
		uint32_t iLevelIndex,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	static bool_t Is_Ready(
		uint32_t iLevelIndex,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
};

NS_END
