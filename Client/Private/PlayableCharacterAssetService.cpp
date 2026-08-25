#include "PlayableCharacterAssetService.h"

#include "ActorCatalog.h"
#include "GameInstance.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
	struct CHARACTER_PROTOTYPE_TAGS final
	{
		const tchar_t* pBody = nullptr;
		/* Eight, not five: Warlord's body does not draw hair, so its hair rides
		along as an equipment part next to the five armour pieces, and a class
		with an avatar head/armor slot adds two more next to its own five. */
		std::array<const tchar_t*, 8> Equipment{};
		size_t iEquipmentCount = 0;
		std::array<const tchar_t*, 4> Weapons{};
		size_t iWeaponCount = 0;
	};

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
				TEXT("Prototype_Component_Model_LanceMaster_Upper_Mokoko")
			},
			7u,
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
				TEXT("Prototype_Component_Model_Artist_Helmet")
			},
			5u,
			{ TEXT("Prototype_Component_Model_Artist_Weapon") },
			1u
		};
		static const CHARACTER_PROTOTYPE_TAGS DIMENSIONMASTER
		{
			TEXT("Prototype_Component_Model_DimensionMaster"),
			{},
			0u,
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

	bool_t Is_Cancelled(
		const std::atomic_bool* pCancellationRequested,
		const std::stop_token* pStopToken)
	{
		return (nullptr != pCancellationRequested &&
			pCancellationRequested->load(std::memory_order_acquire)) ||
			(nullptr != pStopToken && pStopToken->stop_requested());
	}

	using STAGED_CHARACTER_PROTOTYPES =
		std::vector<std::pair<std::wstring, unique_ptr<CPrototype>>>;

	HRESULT Build_StagedCharacterPrototypes(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		const std::atomic_bool* pCancellationRequested,
		const std::stop_token* pStopToken,
		const Client::CPlayableCharacterAssetService::PROGRESS_CALLBACK& progress,
		STAGED_CHARACTER_PROTOTYPES& outStaged)
	{
		outStaged.clear();
		if (Is_Cancelled(pCancellationRequested, pStopToken))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);

		const CHARACTER_ACTOR_ENTRY* pActor =
			CActorCatalog::Find_Character(characterClass);
		const CHARACTER_PROTOTYPE_TAGS* pTags = Find_Tags(characterClass);
		if (nullptr == pActor || nullptr == pTags ||
			pActor->runtimeStatus != "supported" ||
			pActor->equipmentModels.size() != pTags->iEquipmentCount ||
			pActor->weaponModels.size() != pTags->iWeaponCount)
		{
			return E_FAIL;
		}

		const f32_t characterScale =
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER ==
				characterClass ? 0.01f : 0.0001f;
		const matrix_t characterTransform =
			XMMatrixScaling(characterScale, characterScale, characterScale) *
			XMMatrixRotationY(XMConvertToRadians(-90.f));

		const size_t totalModelCount =
			1u + pTags->iEquipmentCount + pTags->iWeaponCount;
		outStaged.reserve(totalModelCount);
		const auto StageModel = [
			&outStaged,
			&pDevice,
			&pContext,
			&pCancellationRequested,
			&pStopToken,
			&progress,
			totalModelCount](
			const tchar_t* pTag,
			const std::string& assetId,
			const MODEL modelType,
			const matrix_t& transform)
		{
			if (Is_Cancelled(pCancellationRequested, pStopToken))
				return false;
			if (progress)
				progress(outStaged.size(), totalModelCount, assetId);
			const std::filesystem::path path =
				CRuntimeAssetRoot::Resolve(assetId);
			if (nullptr == pTag || path.empty())
				return false;
			unique_ptr<CPrototype> pModel = CModel::Create(
				pDevice, pContext, modelType, path.string().c_str(), transform);
			if (nullptr == pModel)
				return false;
			outStaged.emplace_back(pTag, std::move(pModel));
			if (progress)
				progress(outStaged.size(), totalModelCount, assetId);
			return true;
		};

		{
			if (Is_Cancelled(pCancellationRequested, pStopToken))
				return HRESULT_FROM_WIN32(ERROR_CANCELLED);
			if (progress)
				progress(outStaged.size(), totalModelCount, pActor->bodyModel);
			const std::filesystem::path bodyPath =
				CRuntimeAssetRoot::Resolve(pActor->bodyModel);
			if (nullptr == pTags->pBody || bodyPath.empty())
				return E_FAIL;
			unique_ptr<CModel> pBodyModel = CModel::Create(
				pDevice, pContext, MODEL::ANIM,
				bodyPath.string().c_str(), characterTransform);
			if (nullptr == pBodyModel)
				return E_FAIL;
			if (!pActor->animationSetModel.empty())
			{
				if (Is_Cancelled(pCancellationRequested, pStopToken))
					return HRESULT_FROM_WIN32(ERROR_CANCELLED);
				const std::filesystem::path animSetPath =
					CRuntimeAssetRoot::Resolve(pActor->animationSetModel);
				if (animSetPath.empty())
					return E_FAIL;
				const unique_ptr<CModel> pAnimSet = CModel::Create(
					pDevice, pContext, MODEL::ANIM,
					animSetPath.string().c_str(), characterTransform);
				if (nullptr == pAnimSet ||
					FAILED(pBodyModel->Attach_AnimationSet(*pAnimSet)))
				{
					return E_FAIL;
				}
			}
			if (Is_Cancelled(pCancellationRequested, pStopToken))
				return HRESULT_FROM_WIN32(ERROR_CANCELLED);
			outStaged.emplace_back(pTags->pBody, std::move(pBodyModel));
			if (progress)
				progress(outStaged.size(), totalModelCount, pActor->bodyModel);
		}

		for (size_t index = 0; index < pTags->iEquipmentCount; ++index)
		{
			if (!StageModel(
				pTags->Equipment[index],
				pActor->equipmentModels[index],
				MODEL::ANIM,
				characterTransform))
			{
				return Is_Cancelled(pCancellationRequested, pStopToken) ?
					HRESULT_FROM_WIN32(ERROR_CANCELLED) : E_FAIL;
			}
		}

		for (size_t index = 0; index < pTags->iWeaponCount; ++index)
		{
			if (!StageModel(
				pTags->Weapons[index],
				pActor->weaponModels[index],
				MODEL::NONANIM,
				XMMatrixIdentity()))
			{
				return Is_Cancelled(pCancellationRequested, pStopToken) ?
					HRESULT_FROM_WIN32(ERROR_CANCELLED) : E_FAIL;
			}
		}

		return Is_Cancelled(pCancellationRequested, pStopToken) ?
			HRESULT_FROM_WIN32(ERROR_CANCELLED) : S_OK;
	}

	struct PREPARATION_KEY final
	{
		uint32_t iLevelIndex = 0u;
		uint64_t iGeneration = 0u;
		uint8_t iCharacterClass = 0u;

		bool operator==(const PREPARATION_KEY&) const = default;
	};

	struct PREPARATION_KEY_HASH final
	{
		size_t operator()(const PREPARATION_KEY& key) const noexcept
		{
			const size_t level = std::hash<uint32_t>{}(key.iLevelIndex);
			const size_t generation = std::hash<uint64_t>{}(key.iGeneration);
			const size_t characterClass =
				std::hash<uint8_t>{}(key.iCharacterClass);
			return level ^ (generation << 1u) ^ (characterClass << 2u);
		}
	};

	struct PREPARATION_REQUEST final
	{
		PREPARATION_KEY Key;
		ComPtr<ID3D11Device> pDevice;
		ComPtr<ID3D11DeviceContext> pContext;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
	};

	struct PREPARATION_RESULT final
	{
		PREPARATION_KEY Key;
		HRESULT hResult = E_FAIL;
		STAGED_CHARACTER_PROTOTYPES Staged;
	};

	struct ASYNC_PREPARATION_QUEUE final
	{
		~ASYNC_PREPARATION_QUEUE()
		{
			Shutdown_Bounded();
		}

		void Shutdown_Bounded()
		{
			std::unique_lock lock{ Mutex };
			if (Worker.joinable())
			{
				Worker.request_stop();
				Wake.notify_all();
				if (!WorkerExit.wait_for(
					lock,
					std::chrono::seconds(5),
					[this]() { return isWorkerExited; }))
				{
					OutputDebugStringA(
						"[PlayableCharacterAssetService] Worker exceeded shutdown deadline; terminating the process to preserve asset lifetime invariants.\n");
					if (!TerminateProcess(GetCurrentProcess(), ERROR_TIMEOUT))
						std::terminate();
					__assume(0);
				}
				lock.unlock();
				Worker.join();
				lock.lock();
			}

			Requests.clear();
			Results.clear();
			Pending.clear();
			ReadyClassesByLevel.clear();
			FailedClassesByLevel.clear();
			GenerationByLevel.clear();
		}

		void Start_Worker_Locked()
		{
			if (Worker.joinable())
				return;
			isWorkerExited = false;
			Worker = std::jthread([this](const std::stop_token stopToken)
			{
				const HRESULT comResult =
					CoInitializeEx(nullptr, COINIT_MULTITHREADED);
				while (!stopToken.stop_requested())
				{
					PREPARATION_REQUEST request;
					{
						std::unique_lock lock{ Mutex };
						if (!Wake.wait(lock, stopToken, [this]()
							{ return !Requests.empty(); }))
						{
							break;
						}
						if (stopToken.stop_requested())
							break;
						request = std::move(Requests.front());
						Requests.pop_front();
						const auto generation =
							GenerationByLevel.find(request.Key.iLevelIndex);
						if (generation == GenerationByLevel.end() ||
							generation->second != request.Key.iGeneration)
						{
							Pending.erase(request.Key);
							continue;
						}
					}

					PREPARATION_RESULT result;
					result.Key = request.Key;
					result.hResult = Build_StagedCharacterPrototypes(
						request.pDevice,
						request.pContext,
						request.eCharacterClass,
						nullptr,
						&stopToken,
						{},
						result.Staged);

					std::scoped_lock lock{ Mutex };
					if (stopToken.stop_requested())
					{
						Pending.erase(result.Key);
						break;
					}
					const auto generation =
						GenerationByLevel.find(result.Key.iLevelIndex);
					if (generation == GenerationByLevel.end() ||
						generation->second != result.Key.iGeneration)
					{
						Pending.erase(result.Key);
						continue;
					}
					Results.push_back(std::move(result));
				}
				if (SUCCEEDED(comResult))
					CoUninitialize();
				{
					std::scoped_lock lock{ Mutex };
					isWorkerExited = true;
				}
				WorkerExit.notify_all();
			});
		}

		std::mutex Mutex;
		std::condition_variable_any Wake;
		std::condition_variable WorkerExit;
		std::deque<PREPARATION_REQUEST> Requests;
		std::deque<PREPARATION_RESULT> Results;
		std::unordered_map<uint32_t, uint64_t> GenerationByLevel;
		std::unordered_map<uint32_t, std::unordered_set<uint8_t>>
			ReadyClassesByLevel;
		std::unordered_map<uint32_t, std::unordered_map<uint8_t, HRESULT>>
			FailedClassesByLevel;
		std::unordered_set<PREPARATION_KEY, PREPARATION_KEY_HASH> Pending;
		std::jthread Worker;
		bool_t isWorkerExited = true;
	};

	ASYNC_PREPARATION_QUEUE g_PreparationQueue;
	constexpr size_t MAX_PENDING_CLASS_PREPARATIONS = 12u;

	void Advance_LevelGeneration_Locked(
		ASYNC_PREPARATION_QUEUE& queue,
		const uint32_t iLevelIndex,
		const bool_t clearReadyClasses)
	{
		uint64_t& generation = queue.GenerationByLevel[iLevelIndex];
		++generation;
		if (0u == generation)
			++generation;
		if (clearReadyClasses)
			queue.ReadyClassesByLevel.erase(iLevelIndex);
		queue.FailedClassesByLevel.erase(iLevelIndex);

		for (auto request = queue.Requests.begin();
			request != queue.Requests.end();)
		{
			if (request->Key.iLevelIndex == iLevelIndex)
				request = queue.Requests.erase(request);
			else
				++request;
		}
		for (auto result = queue.Results.begin();
			result != queue.Results.end();)
		{
			if (result->Key.iLevelIndex == iLevelIndex)
				result = queue.Results.erase(result);
			else
				++result;
		}
		for (auto pending = queue.Pending.begin();
			pending != queue.Pending.end();)
		{
			if (pending->iLevelIndex == iLevelIndex)
				pending = queue.Pending.erase(pending);
			else
				++pending;
		}
	}
}

void Client::CPlayableCharacterAssetService::Shutdown()
{
	g_PreparationQueue.Shutdown_Bounded();
}

void Client::CPlayableCharacterAssetService::Begin_LevelLoad(
	const uint32_t iLevelIndex)
{
	std::scoped_lock lock{ g_PreparationQueue.Mutex };
	Advance_LevelGeneration_Locked(
		g_PreparationQueue, iLevelIndex, true);
}

void Client::CPlayableCharacterAssetService::Cancel_LevelPreparations(
	const uint32_t iLevelIndex)
{
	std::scoped_lock lock{ g_PreparationQueue.Mutex };
	/* Prototypes already committed to the level remain valid across a network
	   world reset. Only work owned by the previous world generation is stale. */
	Advance_LevelGeneration_Locked(
		g_PreparationQueue, iLevelIndex, false);
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

	const uint8_t classKey = static_cast<uint8_t>(characterClass);
	PREPARATION_KEY preparationKey{};
	{
		std::scoped_lock lock{ g_PreparationQueue.Mutex };
		uint64_t& generation =
			g_PreparationQueue.GenerationByLevel[iLevelIndex];
		if (0u == generation)
			generation = 1u;
		if (g_PreparationQueue.ReadyClassesByLevel[iLevelIndex].contains(
			classKey))
		{
			return S_FALSE;
		}
		preparationKey = { iLevelIndex, generation, classKey };
		if (g_PreparationQueue.Pending.contains(preparationKey))
			return HRESULT_FROM_WIN32(ERROR_IO_PENDING);
		g_PreparationQueue.Pending.insert(preparationKey);
	}

	STAGED_CHARACTER_PROTOTYPES staged;
	const HRESULT buildResult = Build_StagedCharacterPrototypes(
		pDevice,
		pContext,
		characterClass,
		pCancellationRequested,
		nullptr,
		progress,
		staged);

	std::scoped_lock lock{ g_PreparationQueue.Mutex };
	g_PreparationQueue.Pending.erase(preparationKey);
	const auto generation =
		g_PreparationQueue.GenerationByLevel.find(iLevelIndex);
	if (generation == g_PreparationQueue.GenerationByLevel.end() ||
		generation->second != preparationKey.iGeneration)
	{
		return HRESULT_FROM_WIN32(ERROR_CANCELLED);
	}
	if (FAILED(buildResult))
	{
		g_PreparationQueue.FailedClassesByLevel[iLevelIndex][classKey] =
			buildResult;
		return buildResult;
	}
	const HRESULT commitResult = CGameInstance::Get().Add_Prototypes(
		iLevelIndex, std::move(staged));
	if (FAILED(commitResult))
	{
		g_PreparationQueue.FailedClassesByLevel[iLevelIndex][classKey] =
			commitResult;
		return commitResult;
	}
	g_PreparationQueue.ReadyClassesByLevel[iLevelIndex].insert(classKey);
	return S_OK;
}

HRESULT Client::CPlayableCharacterAssetService::Queue_PrototypePreparation(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iLevelIndex,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (nullptr == pDevice || nullptr == pContext ||
		iLevelIndex >= ETOUI(LEVEL::END) ||
		!LostArk::Shared::Is_Supported_Playable_Character_Class(characterClass))
	{
		return E_INVALIDARG;
	}

	const uint8_t classKey = static_cast<uint8_t>(characterClass);
	std::scoped_lock lock{ g_PreparationQueue.Mutex };
	uint64_t& generation =
		g_PreparationQueue.GenerationByLevel[iLevelIndex];
	if (0u == generation)
		generation = 1u;
	if (g_PreparationQueue.ReadyClassesByLevel[iLevelIndex].contains(classKey))
		return S_FALSE;
	const auto failed =
		g_PreparationQueue.FailedClassesByLevel[iLevelIndex].find(classKey);
	if (failed != g_PreparationQueue.FailedClassesByLevel[iLevelIndex].end())
		return failed->second;

	const PREPARATION_KEY key{ iLevelIndex, generation, classKey };
	if (g_PreparationQueue.Pending.contains(key))
		return S_FALSE;
	if (g_PreparationQueue.Pending.size() >= MAX_PENDING_CLASS_PREPARATIONS)
		return HRESULT_FROM_WIN32(ERROR_BUSY);

	g_PreparationQueue.Start_Worker_Locked();
	g_PreparationQueue.Pending.insert(key);
	g_PreparationQueue.Requests.push_back(PREPARATION_REQUEST{
		key, pDevice, pContext, characterClass });
	g_PreparationQueue.Wake.notify_one();
	return S_OK;
}

HRESULT Client::CPlayableCharacterAssetService::Commit_PrototypePreparation(
	const uint32_t iLevelIndex,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) ||
		!LostArk::Shared::Is_Supported_Playable_Character_Class(characterClass))
	{
		return E_INVALIDARG;
	}

	const uint8_t classKey = static_cast<uint8_t>(characterClass);
	std::scoped_lock lock{ g_PreparationQueue.Mutex };
	uint64_t& generation =
		g_PreparationQueue.GenerationByLevel[iLevelIndex];
	if (0u == generation)
		generation = 1u;
	if (g_PreparationQueue.ReadyClassesByLevel[iLevelIndex].contains(classKey))
		return S_FALSE;
	const auto failed =
		g_PreparationQueue.FailedClassesByLevel[iLevelIndex].find(classKey);
	if (failed != g_PreparationQueue.FailedClassesByLevel[iLevelIndex].end())
		return failed->second;

	const PREPARATION_KEY key{ iLevelIndex, generation, classKey };
	const auto result = std::find_if(
		g_PreparationQueue.Results.begin(),
		g_PreparationQueue.Results.end(),
		[&key](const PREPARATION_RESULT& value)
		{ return value.Key == key; });
	if (result == g_PreparationQueue.Results.end())
	{
		return g_PreparationQueue.Pending.contains(key) ?
			HRESULT_FROM_WIN32(ERROR_IO_PENDING) :
			HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
	}

	PREPARATION_RESULT completed = std::move(*result);
	g_PreparationQueue.Results.erase(result);
	g_PreparationQueue.Pending.erase(key);
	if (FAILED(completed.hResult))
	{
		g_PreparationQueue.FailedClassesByLevel[iLevelIndex][classKey] =
			completed.hResult;
		return completed.hResult;
	}

	const HRESULT commitResult = CGameInstance::Get().Add_Prototypes(
		iLevelIndex, std::move(completed.Staged));
	if (FAILED(commitResult))
	{
		g_PreparationQueue.FailedClassesByLevel[iLevelIndex][classKey] =
			commitResult;
		return commitResult;
	}
	g_PreparationQueue.ReadyClassesByLevel[iLevelIndex].insert(classKey);
	return S_OK;
}

bool_t Client::CPlayableCharacterAssetService::Is_Ready(
	const uint32_t iLevelIndex,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	std::scoped_lock lock{ g_PreparationQueue.Mutex };
	const auto level = g_PreparationQueue.ReadyClassesByLevel.find(iLevelIndex);
	return level != g_PreparationQueue.ReadyClassesByLevel.end() &&
		level->second.contains(static_cast<uint8_t>(characterClass));
}
