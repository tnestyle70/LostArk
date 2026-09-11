#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketType.h"
#include "AnimationSkillBindingDocument.h"
#include "AnimationEffectCueDocument.h"
#include "FaceSliderDocument.h"
#include "FaceMorphApplier.h"

#include <atomic>
#include <cstddef>
#include <functional>
#include <string>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <utility>

NS_BEGIN(Engine)
class CPrototype;
NS_END

NS_BEGIN(Client)

// Owns the one runtime path that admits playable character model prototypes.
// The level loader admits the locally selected class first. Replication uses
// the same service when a remote class is observed for the first time.
class CPlayableCharacterAssetService final
{
public:
	struct PREPARED_PRESENTATION final
	{
		ANIMATION_SKILL_BINDING_DOCUMENT SkillBindings;
		ANIMATION_EFFECT_CUE_DOCUMENT EffectCues;
		FACE_SLIDER_DOCUMENT FaceSliders;
		CFaceMorphApplier FaceMorphs;
		bool HasSkillBindings = false, HasEffectCues = false, HasFaceSliders = false;
		std::string SkillStatus, EffectStatus, FaceSliderStatus;
	};
	static std::shared_ptr<const PREPARED_PRESENTATION> Get_PreparedPresentation(
		uint32_t level, LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	static void Update_PreparedSkillBindings(uint32_t level,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		const ANIMATION_SKILL_BINDING_DOCUMENT& document);
	struct AUTHORING_INPUT;
	static std::shared_ptr<const AUTHORING_INPUT> Capture_AuthoringInput(
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	static void Cancel_AllAsyncPreparations();
	static bool_t Has_ActivePreparations();
	CPlayableCharacterAssetService() = default;
	~CPlayableCharacterAssetService();
	CPlayableCharacterAssetService(const CPlayableCharacterAssetService&) = delete;
	CPlayableCharacterAssetService& operator=(const CPlayableCharacterAssetService&) = delete;

	// One selected-class preparation. Model decode uses only ID3D11Device creation;
	// the retained immediate context is never called by this worker.
	HRESULT Begin_AsyncPreparation(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext, uint32_t iLevelIndex,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	bool_t Poll_AsyncPreparation(bool_t commitWanted, HRESULT& outResult,
		std::string& outStatus);
	void Cancel_AsyncPreparation();
	bool_t Is_Preparing() const noexcept { return m_Worker.joinable(); }

	using PROGRESS_CALLBACK = std::function<void(
		size_t completedModelCount,
		size_t totalModelCount,
		const std::string& assetId)>;

	static void Begin_LevelLoad(uint32_t iLevelIndex);
	static HRESULT Ensure_Prototypes(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iLevelIndex,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		const std::atomic_bool* pCancellationRequested = nullptr,
		const PROGRESS_CALLBACK& progress = {},
		const std::shared_ptr<const AUTHORING_INPUT>& authoringInput = {});
	static bool_t Is_Ready(
		uint32_t iLevelIndex,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
private:
	using PREPARED_MODELS = std::vector<std::pair<std::wstring, unique_ptr<Engine::CPrototype>>>;
	static HRESULT Prepare_Models(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext, uint32_t iLevelIndex,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		const std::atomic_bool* pCancellationRequested,
		const PROGRESS_CALLBACK& progress, PREPARED_MODELS& staged,
		const AUTHORING_INPUT& authoring, std::shared_ptr<const PREPARED_PRESENTATION>& presentation);
	static HRESULT Commit_Models(uint32_t iLevelIndex,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		uint64_t generation, PREPARED_MODELS& staged,
		const std::shared_ptr<const PREPARED_PRESENTATION>& presentation);
	struct ASYNC_PREPARATION;
	static std::mutex s_AsyncJobsMutex;
	static std::vector<std::weak_ptr<ASYNC_PREPARATION>> s_AsyncJobs;
	std::shared_ptr<ASYNC_PREPARATION> m_AsyncPreparation;
	std::thread m_Worker;
};

NS_END
