#include "PlayableCharacterAssetService.h"

#include "ActorCatalog.h"
#include "CharacterCatalog.h"
#include "CharacterSpec.h"
#include "Effect_Catalog.h"
#include "BinaryAsset/ModelAssetData.h"
#include "GameInstance.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"
#include "Profiler.h"
#include "Prototype.h"

#include <array>
#include <condition_variable>
#include <exception>
#include <stdexcept>
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
	std::unordered_map<uint32_t, std::unordered_map<uint8_t,
		std::shared_ptr<const Client::CPlayableCharacterAssetService::PREPARED_PRESENTATION>>>
		g_ReadyClassesByLevel;
	std::unordered_map<uint32_t, uint64_t> g_ClassLevelGenerations;

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
	++g_ClassLevelGenerations[iLevelIndex];
}

struct Client::CPlayableCharacterAssetService::AUTHORING_INPUT final
{
	std::vector<PLAYER_SKILL_DEFINITION> Skills;
	std::unordered_set<std::string> EffectAssetIds;
	std::string FaceSliderRace;
};

std::shared_ptr<const Client::CPlayableCharacterAssetService::AUTHORING_INPUT>
Client::CPlayableCharacterAssetService::Capture_AuthoringInput(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "CharacterAssets.Authoring.Capture");
	auto result = std::make_shared<AUTHORING_INPUT>();
	result->Skills = CPlayerSkillCatalog::Get_Skills();
	if (result->Skills.empty())
	{
		OutputDebugStringA("[CharacterAssets] Player skills must be initialized before authoring snapshot capture.\n");
		return {};
	}
	const auto ids = CEffectCatalog::Get_EffectAssetIds();
	result->EffectAssetIds.insert(ids.begin(), ids.end());
	if (const auto* spec = CCharacterCatalog::Find_Spec(characterClass); spec && spec->pFaceSliderRace)
		result->FaceSliderRace = spec->pFaceSliderRace;
	return result;
}

HRESULT Client::CPlayableCharacterAssetService::Prepare_Models(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iLevelIndex,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const std::atomic_bool* pCancellationRequested,
	const PROGRESS_CALLBACK& progress, PREPARED_MODELS& staged,
	const AUTHORING_INPUT& authoring, std::shared_ptr<const PREPARED_PRESENTATION>& presentation)
{
	if (nullptr == pDevice || nullptr == pContext ||
		iLevelIndex >= ETOUI(LEVEL::END) ||
		!LostArk::Shared::Is_Supported_Playable_Character_Class(
			characterClass))
	{
		return E_INVALIDARG;
	}

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
			if (Is_Cancelled(pCancellationRequested)) return HRESULT_FROM_WIN32(ERROR_CANCELLED);
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

	if (Is_Cancelled(pCancellationRequested)) return HRESULT_FROM_WIN32(ERROR_CANCELLED);
	Engine::CProfilerScope authoringScope(CGameInstance::Get().Get_Profiler(), "CharacterAssets.Authoring.Prepare");
	auto documents = std::make_shared<PREPARED_PRESENTATION>();
	std::vector<std::string> clips;
	clips.reserve(pBodyPalette->Get_NumAnimations());
	for (uint32_t index = 0u; index < pBodyPalette->Get_NumAnimations(); ++index)
		if (const char_t* clip = pBodyPalette->Get_AnimationName(index)) clips.emplace_back(clip);
	documents->HasSkillBindings = CAnimationSkillBindingDocument::Load(pActor->assetId,
		characterClass, authoring.Skills, clips, documents->SkillBindings, documents->SkillStatus);
	if (Is_Cancelled(pCancellationRequested)) return HRESULT_FROM_WIN32(ERROR_CANCELLED);
	documents->HasEffectCues = CAnimationEffectCueDocument::Load_WithCatalogSnapshot(pActor->assetId,
		clips, documents->EffectCues, documents->EffectStatus, authoring.EffectAssetIds);
	if (!authoring.FaceSliderRace.empty())
		documents->HasFaceSliders = documents->FaceSliders.Load(authoring.FaceSliderRace.c_str(), documents->FaceSliderStatus);
	if (Is_Cancelled(pCancellationRequested)) return HRESULT_FROM_WIN32(ERROR_CANCELLED);
	const auto faceRoot = std::string("Character/") + pActor->assetId + "/FaceMorphs/" + pActor->assetId;
	const auto morphs = CRuntimeAssetRoot::Resolve(faceRoot + ".facemorphs");
	const auto mapping = CRuntimeAssetRoot::Resolve(faceRoot + ".facemorphmap");
	if (!morphs.empty() && !mapping.empty() && std::filesystem::exists(morphs) && std::filesystem::exists(mapping))
	{
		Engine::CProfilerScope morphScope(CGameInstance::Get().Get_Profiler(), "CharacterAssets.FaceMorph.Prepare");
		(void)documents->FaceMorphs.Initialize(*pBodyPalette, morphs, mapping);
	}
	presentation = std::move(documents);
	return Is_Cancelled(pCancellationRequested) ? HRESULT_FROM_WIN32(ERROR_CANCELLED) : S_OK;
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

HRESULT Client::CPlayableCharacterAssetService::Commit_Models(
	const uint32_t iLevelIndex,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const uint64_t generation, PREPARED_MODELS& staged,
	const std::shared_ptr<const PREPARED_PRESENTATION>& presentation)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "CharacterAssets.Commit");
	std::scoped_lock lock{ g_CharacterAssetMutex };
	if (g_ClassLevelGenerations[iLevelIndex] != generation)
		return HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED);
	const auto key = static_cast<uint8_t>(characterClass);
	if (g_ReadyClassesByLevel[iLevelIndex].contains(key)) return S_FALSE;
	// Prototype_Manager has no concurrent read/write contract. The asynchronous
	// caller reaches this only from Poll on the main thread; Loader is exclusive.
	if (!presentation) return E_INVALIDARG;
	// Allocate the small publication entry before committing any engine prototypes.
	auto& ready = g_ReadyClassesByLevel[iLevelIndex];
	ready.emplace(key, presentation);
	const HRESULT result = CGameInstance::Get().Add_Prototypes(iLevelIndex, std::move(staged));
	if (FAILED(result)) ready.erase(key);
	return result;
}

HRESULT Client::CPlayableCharacterAssetService::Ensure_Prototypes(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iLevelIndex,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const std::atomic_bool* cancellation, const PROGRESS_CALLBACK& progress,
	const std::shared_ptr<const AUTHORING_INPUT>& authoringInput)
{
	uint64_t generation;
	{
		std::scoped_lock lock{ g_CharacterAssetMutex };
		if (g_ReadyClassesByLevel[iLevelIndex].contains(static_cast<uint8_t>(characterClass))) return S_FALSE;
		generation = g_ClassLevelGenerations[iLevelIndex];
	}
	PREPARED_MODELS staged;
	std::shared_ptr<const PREPARED_PRESENTATION> presentation;
	const auto authoring = authoringInput ? authoringInput : Capture_AuthoringInput(characterClass);
	if (!authoring) return E_FAIL;
	const HRESULT result = Prepare_Models(pDevice, pContext, iLevelIndex, characterClass, cancellation, progress, staged, *authoring, presentation);
	return FAILED(result) ? result : Commit_Models(iLevelIndex, characterClass, generation, staged, presentation);
}

struct Client::CPlayableCharacterAssetService::ASYNC_PREPARATION final
{
	std::mutex Mutex;
	std::condition_variable Decision;
	std::atomic_bool Cancelled{false};
	std::atomic_bool Completed{false};
	std::atomic<HANDLE> WorkerHandle{nullptr};
	std::atomic<ULONGLONG> TransitionCancelTick{0u};
	bool ReadyForCommit = false;
	bool CommitDecided = false;
	uint32_t Level = 0u;
	uint64_t Generation = 0u;
	LostArk::Shared::CHARACTER_CLASS_ID CharacterClass = LostArk::Shared::CHARACTER_CLASS_ID::END;
	PREPARED_MODELS Models;
	std::shared_ptr<const AUTHORING_INPUT> Authoring;
	std::shared_ptr<const PREPARED_PRESENTATION> Presentation;
	HRESULT Result = E_PENDING;
	std::string Status;
};

Client::CPlayableCharacterAssetService::~CPlayableCharacterAssetService()
{
	if (!m_Worker.joinable()) return;
	Cancel_AsyncPreparation();
	const DWORD waited = WaitForSingleObject(m_Worker.native_handle(), 5000u);
	if (waited != WAIT_OBJECT_0)
	{
		OutputDebugStringA("[CharacterAssets] Preparation shutdown exceeded its bound; exiting.\n");
		TerminateProcess(GetCurrentProcess(), waited == WAIT_TIMEOUT ? ERROR_TIMEOUT : ERROR_GEN_FAILURE);
		std::terminate();
	}
	m_AsyncPreparation->WorkerHandle.store(nullptr);
	m_Worker.join();
}

HRESULT Client::CPlayableCharacterAssetService::Begin_AsyncPreparation(
	ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context,
	const uint32_t level, const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (m_Worker.joinable()) return HRESULT_FROM_WIN32(ERROR_BUSY);
	if (!device || !context || level >= ETOUI(LEVEL::END) ||
		!LostArk::Shared::Is_Supported_Playable_Character_Class(characterClass)) return E_INVALIDARG;
	// Admission metadata is initialized by the level loader, before worker access.
	if (!CActorCatalog::Find_Character(characterClass)) return E_FAIL;
	if (Is_Ready(level, characterClass)) return S_FALSE;
	auto job = std::make_shared<ASYNC_PREPARATION>();
	job->Level = level;
	job->CharacterClass = characterClass;
	job->Authoring = Capture_AuthoringInput(characterClass);
	if (!job->Authoring) return E_FAIL;
	{
		std::scoped_lock lock{g_CharacterAssetMutex};
		job->Generation = g_ClassLevelGenerations[level];
	}
	try
	{
		// Publish all allocating owner/registry state before starting a worker that
		// can wait for the owner's commit decision.
		{
			std::scoped_lock lock{s_AsyncJobsMutex};
			std::erase_if(s_AsyncJobs, [](const auto& entry) { return entry.expired(); });
			s_AsyncJobs.push_back(job);
		}
		m_AsyncPreparation = job;
		m_Worker = std::thread([job, device, context]()
		{
			const HRESULT apartment = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
			try
			{
				const auto progress = [job](size_t done, size_t total, const std::string& asset)
				{
					std::scoped_lock lock{job->Mutex};
					job->Status = "Preparing class models " + std::to_string(done) + "/" + std::to_string(total) + " | " + asset;
				};
				{
					Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "CharacterAssets.Prepare.Worker");
					job->Result = FAILED(apartment) ? apartment : Prepare_Models(device, context,
						job->Level, job->CharacterClass, &job->Cancelled, progress, job->Models, *job->Authoring, job->Presentation);
					if (job->Cancelled.load()) job->Result = HRESULT_FROM_WIN32(ERROR_CANCELLED);
				}
				if (SUCCEEDED(job->Result))
				{
					std::unique_lock lock{job->Mutex};
					job->ReadyForCommit = true;
					job->Decision.wait(lock, [&]() { return job->CommitDecided || job->Cancelled.load(); });
					if (!job->CommitDecided) job->Result = HRESULT_FROM_WIN32(ERROR_CANCELLED);
				}
			}
			catch (const std::exception& error)
			{
				job->Result = E_FAIL;
				std::scoped_lock lock{job->Mutex}; job->Status = error.what();
			}
			catch (...) { job->Result = E_FAIL; }
			// Failed, cancelled, stale and duplicate preparations release all large
			// model/animation allocations here, never on the frame that polls them.
			{
				Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "CharacterAssets.Retire.Worker");
				job->Models.clear();
				job->Presentation.reset();
				job->Authoring.reset();
			}
			if (SUCCEEDED(apartment)) CoUninitialize();
			job->Completed.store(true, std::memory_order_release);
		});
	}
	catch (...) { m_AsyncPreparation.reset(); return E_FAIL; }
	job->WorkerHandle.store(m_Worker.native_handle());
	return S_OK;
}

void Client::CPlayableCharacterAssetService::Cancel_AsyncPreparation()
{
	if (!m_AsyncPreparation) return;
	m_AsyncPreparation->Cancelled.store(true);
	m_AsyncPreparation->Decision.notify_all();
	if (m_Worker.joinable()) CancelSynchronousIo(m_Worker.native_handle());
}

bool_t Client::CPlayableCharacterAssetService::Poll_AsyncPreparation(
	const bool_t commitWanted, HRESULT& outResult, std::string& outStatus)
{
	if (!m_Worker.joinable()) { outResult = S_FALSE; return true; }
	const auto& job = m_AsyncPreparation;
	{
		std::scoped_lock lock{job->Mutex};
		outStatus = job->Status;
		if (job->ReadyForCommit && !job->CommitDecided && !job->Cancelled.load())
		{
			job->Result = commitWanted ? Commit_Models(job->Level, job->CharacterClass,
				job->Generation, job->Models, job->Presentation) : HRESULT_FROM_WIN32(ERROR_CANCELLED);
			job->CommitDecided = true;
			job->Decision.notify_all();
		}
	}
	const DWORD waited = WaitForSingleObject(m_Worker.native_handle(), 0u);
	if (waited == WAIT_TIMEOUT) return false;
	if (waited != WAIT_OBJECT_0)
	{
		OutputDebugStringA("[CharacterAssets] Could not observe preparation completion.\n");
		TerminateProcess(GetCurrentProcess(), ERROR_GEN_FAILURE);
		std::terminate();
	}
	job->WorkerHandle.store(nullptr);
	m_Worker.join();
	outResult = job->Result;
	if (FAILED(outResult) && outResult != HRESULT_FROM_WIN32(ERROR_CANCELLED))
		outStatus = "Class model preparation failed (HRESULT " + std::to_string(static_cast<unsigned long>(outResult)) + "): " + job->Status;
	m_AsyncPreparation.reset();
	return true;
}

std::shared_ptr<const Client::CPlayableCharacterAssetService::PREPARED_PRESENTATION>
Client::CPlayableCharacterAssetService::Get_PreparedPresentation(
	const uint32_t level, const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	std::scoped_lock lock{g_CharacterAssetMutex};
	const auto foundLevel = g_ReadyClassesByLevel.find(level);
	if (foundLevel == g_ReadyClassesByLevel.end()) return {};
	const auto foundClass = foundLevel->second.find(static_cast<uint8_t>(characterClass));
	return foundClass == foundLevel->second.end() ? nullptr : foundClass->second;
}

void Client::CPlayableCharacterAssetService::Update_PreparedSkillBindings(
	const uint32_t level, const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const ANIMATION_SKILL_BINDING_DOCUMENT& document)
{
	std::scoped_lock lock{g_CharacterAssetMutex};
	const auto foundLevel = g_ReadyClassesByLevel.find(level);
	if (foundLevel == g_ReadyClassesByLevel.end()) return;
	const auto foundClass = foundLevel->second.find(static_cast<uint8_t>(characterClass));
	if (foundClass == foundLevel->second.end()) return;
	auto updated = std::make_shared<PREPARED_PRESENTATION>(*foundClass->second);
	updated->SkillBindings = document;
	updated->HasSkillBindings = true;
	updated->SkillStatus.clear();
	foundClass->second = std::move(updated);
}

std::mutex Client::CPlayableCharacterAssetService::s_AsyncJobsMutex;
std::vector<std::weak_ptr<Client::CPlayableCharacterAssetService::ASYNC_PREPARATION>>
	Client::CPlayableCharacterAssetService::s_AsyncJobs;

void Client::CPlayableCharacterAssetService::Cancel_AllAsyncPreparations()
{
	std::scoped_lock lock{s_AsyncJobsMutex};
	for (const auto& weak : s_AsyncJobs)
	{
		if (const auto job = weak.lock(); job && !job->Completed.load(std::memory_order_acquire))
		{
			const ULONGLONG now = GetTickCount64();
			ULONGLONG unset = 0u;
			job->TransitionCancelTick.compare_exchange_strong(unset, now);
			if (now - job->TransitionCancelTick.load() > 60000u)
			{
				OutputDebugStringA("[CharacterAssets] Nonblocking transition drain exceeded 60 seconds; exiting.\n");
				TerminateProcess(GetCurrentProcess(), ERROR_TIMEOUT);
				std::terminate();
			}
			job->Cancelled.store(true);
			job->Decision.notify_all();
			if (const HANDLE worker = job->WorkerHandle.load()) CancelSynchronousIo(worker);
		}
	}
}

bool_t Client::CPlayableCharacterAssetService::Has_ActivePreparations()
{
	std::scoped_lock lock{s_AsyncJobsMutex};
	for (const auto& weak : s_AsyncJobs)
		if (const auto job = weak.lock(); job && !job->Completed.load(std::memory_order_acquire)) return true;
	return false;
}
