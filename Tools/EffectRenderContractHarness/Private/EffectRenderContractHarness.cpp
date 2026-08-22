#include "Client_Defines.h"

#include "Effect_Artist31470ShaderRegistry.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "Effect_DocumentRenderer.h"
#include "Effect_MaterialProgramRegistry.h"
#include "Effect_Object.h"
#include "Effect_OccurrenceTuning.h"
#include "Effect_ReconstructedExecution.h"
#include "Effect_VisualProgramCorpus.h"
#include "GameInstance.h"
#include "Level.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <Windows.h>

namespace
{
	constexpr std::string_view ARTIST_FULL_EFFECT_ID =
		"effect.artist.skill.31470";
	constexpr std::string_view ARTIST_UNIFIED_EFFECT_ID =
		"effect.artist.skill.31470.unified";
	constexpr std::string_view ARTIST_CANARY_ELEMENT_ID =
		"sprite.2b3dc6842507e910";
	constexpr std::string_view ARTIST_CANARY_PROGRAM_ID =
		"effect.program.runtime-material-v2.opcode-6.artist-f-sprite.v1";
	constexpr std::string_view ARTIST_CANARY_LAYOUT_ID =
		"effect.layout.runtime-material-v2.artist-f-sprite.v1";
	constexpr std::string_view ARTIST_CANARY_DESCRIPTOR_ID =
		"effect.descriptor.artist-f.sprite-2b3dc6842507e910.v1";
	constexpr std::string_view LANCE_EFFECT_ID =
		"effect.lancemaster.skill.34010.ba1";
	constexpr std::string_view LANCE_BA1_OCCURRENCE_ID =
		"authored.approx.s002.mesh01";
	constexpr uint32_t WINDOW_WIDTH = 320u;
	constexpr uint32_t WINDOW_HEIGHT = 180u;
	constexpr wchar_t WINDOW_CLASS_NAME[] =
		L"LostArkEffectRenderContractHarness";

	void Write_Progress(const std::string_view strStage)
	{
		std::cerr << "[effect-render-contract] stage=" << strStage << '\n';
	}

	struct AGGREGATE_STATS final
	{
		uint64_t iConfigured = 0u;
		uint64_t iEvaluated = 0u;
		uint64_t iActive = 0u;
		uint64_t iCandidate = 0u;
		uint64_t iAttempted = 0u;
		uint64_t iSubmitted = 0u;
		uint64_t iSuppressed = 0u;
		uint64_t iFailed = 0u;
		bool_t bCompleted = false;
		bool_t bCommitted = false;
	};

	struct OCCURRENCE_EVIDENCE final
	{
		std::string strElementId;
		uint64_t iActive = 0u;
		uint64_t iCandidate = 0u;
		uint64_t iAttempted = 0u;
		uint64_t iSubmitted = 0u;
		uint64_t iSuppressed = 0u;
		uint64_t iFailed = 0u;
		uint64_t iMaterialBindCount = 0u;
		uint64_t iTextureSrvBindCount = 0u;
		uint64_t iSamplerBindCount = 0u;
		uint64_t iShaderPassApplyCount = 0u;
		uint64_t iVIBufferBindCount = 0u;
		uint64_t iVIBufferDrawCount = 0u;
		uint64_t iIssuedDrawCallCount = 0u;
		uint64_t iDrawSelectionCount = 0u;
		uint64_t iCompiledAdapterPipelineValidationCount = 0u;
		uint32_t iSelectedPassIndex = UINT32_MAX;
		Client::EFFECT_GPU_RENDER_CARRIER eCarrier =
			Client::EFFECT_GPU_RENDER_CARRIER::END;
		bool_t bDrawSelectionDiverged = false;
	};

	struct FRAME_EVIDENCE final
	{
		std::string strScenarioId;
		std::string strContract;
		f32_t fSampleTimeSeconds = 0.f;
		bool_t bGameInstanceRender = false;
		HRESULT hRenderResult = E_FAIL;
		HRESULT hDeviceResult = E_FAIL;
		AGGREGATE_STATS Aggregate;
		std::optional<OCCURRENCE_EVIDENCE> Occurrence;
		bool_t bBoundAdapterActualPipelineValidated = false;
	};

	struct FULL35_DISPOSITION_ROW final
	{
		std::string strOccurrenceId;
		std::string strElementId;
		Client::EFFECT_RUNTIME_RENDERER_KIND eRenderer =
			Client::EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE;
		uint32_t iPassIndex = Client::EFFECT_ARTIST31470_NO_PASS;
		bool_t bDrawAdmitted = false;
	};

	struct FULL35_FIXED_STEP_DISPOSITION final
	{
		f32_t fSampleTimeSeconds = 0.f;
		uint64_t iFixedStepIndex = 0u;
		uint64_t iActiveParticlePacketCount = 0u;
		uint64_t iAttempted = 0u;
		uint64_t iSubmitted = 0u;
		uint64_t iSuppressed = 0u;
		uint64_t iFailed = 0u;
		bool_t bCommitted = false;
		std::string strStateProjectionSha256;
		std::string strFrameProjectionSha256;
		std::vector<FULL35_DISPOSITION_ROW> Rows;
	};

	class HEADLESS_LEVEL final : public Engine::CLevel
	{
	public:
		HEADLESS_LEVEL(ComPtr<ID3D11Device> pDevice,
			ComPtr<ID3D11DeviceContext> pContext)
			: CLevel(std::move(pDevice), std::move(pContext))
		{
		}
	};

	class HEADLESS_ENGINE_SCOPE final
	{
	public:
		~HEADLESS_ENGINE_SCOPE()
		{
			if (m_bEngineInitialized)
				Engine::CGameInstance::Get().Release_Engine();
			if (nullptr != m_hWnd)
				DestroyWindow(m_hWnd);
			if (0u != m_ClassAtom)
				UnregisterClassW(WINDOW_CLASS_NAME, m_hInstance);
		}

		bool_t Initialize(std::string& strOutError)
		{
			m_hInstance = GetModuleHandleW(nullptr);
			if (nullptr == m_hInstance)
			{
				strOutError = "harness module identity is unavailable";
				return false;
			}

			WNDCLASSEXW WindowClass{};
			WindowClass.cbSize = sizeof(WindowClass);
			WindowClass.lpfnWndProc = DefWindowProcW;
			WindowClass.hInstance = m_hInstance;
			WindowClass.lpszClassName = WINDOW_CLASS_NAME;
			m_ClassAtom = RegisterClassExW(&WindowClass);
			if (0u == m_ClassAtom && ERROR_CLASS_ALREADY_EXISTS != GetLastError())
			{
				strOutError = "hidden WARP window class registration failed";
				return false;
			}

			m_hWnd = CreateWindowExW(0u, WINDOW_CLASS_NAME,
				L"LostArk Effect Render Contract Harness", WS_OVERLAPPED,
				CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
				nullptr, nullptr, m_hInstance, nullptr);
			if (nullptr == m_hWnd)
			{
				strOutError = "hidden WARP window creation failed";
				return false;
			}

			ENGINE_DESC Desc{};
			Desc.hInstance = m_hInstance;
			Desc.hWnd = m_hWnd;
			Desc.eWinMode = WINMODE::WIN;
			Desc.eDriverType = D3D_DRIVER_TYPE_WARP;
			Desc.bNonInteractiveErrors = true;
			Desc.iNumLevels = ETOUI(Client::LEVEL::END);
			Desc.iWinSizeX = WINDOW_WIDTH;
			Desc.iWinSizeY = WINDOW_HEIGHT;
			if (FAILED(Engine::CGameInstance::Get().Initialize_Engine(
				Desc, m_pDevice, m_pContext)) || nullptr == m_pDevice ||
				nullptr == m_pContext)
			{
				strOutError = "production Engine initialization failed";
				return false;
			}
			m_bEngineInitialized = true;

			ComPtr<IDXGIDevice> DxgiDevice;
			ComPtr<IDXGIAdapter> Adapter;
			DXGI_ADAPTER_DESC AdapterDesc{};
			if (FAILED(m_pDevice.As(&DxgiDevice)) ||
				FAILED(DxgiDevice->GetAdapter(&Adapter)) ||
				FAILED(Adapter->GetDesc(&AdapterDesc)) ||
				AdapterDesc.VendorId != 0x1414u ||
				AdapterDesc.DeviceId != 0x008cu)
			{
				strOutError = "Engine did not select the Microsoft WARP adapter";
				return false;
			}

			Engine::CGameInstance::Get().Set_Transform(D3DTS::VIEW,
				XMMatrixLookAtLH(XMVectorSet(0.f, 1.f, -5.f, 1.f),
					XMVectorSet(0.f, 1.f, 0.f, 1.f),
					XMVectorSet(0.f, 1.f, 0.f, 0.f)));
			Engine::CGameInstance::Get().Set_Transform(D3DTS::PROJ,
				XMMatrixPerspectiveFovLH(XMConvertToRadians(60.f),
					static_cast<f32_t>(WINDOW_WIDTH) /
						static_cast<f32_t>(WINDOW_HEIGHT),
					0.1f, 100.f));
			std::unique_ptr<HEADLESS_LEVEL> Level =
				std::make_unique<HEADLESS_LEVEL>(m_pDevice, m_pContext);
			if (FAILED(Level->Initialize()) ||
				FAILED(Engine::CGameInstance::Get().Change_Level(
					ETOUI(Client::LEVEL::DEVELOPMENT), std::move(Level))))
			{
				strOutError = "headless production Level staging failed";
				return false;
			}
			Engine::CGameInstance::Get().Update_Engine(0.f);
			strOutError.clear();
			return true;
		}

		ComPtr<ID3D11Device> Get_Device() const { return m_pDevice; }
		ComPtr<ID3D11DeviceContext> Get_Context() const { return m_pContext; }

		bool_t Render_Frame(const std::shared_ptr<Client::CEffectObject>& pObject,
			const f32_t fSampleTimeSeconds, FRAME_EVIDENCE& OutEvidence,
			std::string& strOutError) const
		{
			if (!m_bEngineInitialized || nullptr == pObject)
			{
				strOutError = "frame target is not initialized";
				return false;
			}
			pObject->Set_SampleTime(fSampleTimeSeconds);
			pObject->Late_Update(0.f);
			const float4_t ClearColor(0.f, 0.f, 0.f, 0.f);
			if (FAILED(Engine::CGameInstance::Get().Render_Begin(&ClearColor)))
			{
				strOutError = "production Render_Begin failed";
				return false;
			}
			OutEvidence.fSampleTimeSeconds = fSampleTimeSeconds;
			OutEvidence.bGameInstanceRender = true;
			OutEvidence.hRenderResult = Engine::CGameInstance::Get().Render();
			m_pContext->Flush();
			OutEvidence.hDeviceResult = m_pDevice->GetDeviceRemovedReason();
			const Client::EFFECT_GPU_RENDER_SUBMISSION_STATS& Stats =
				pObject->Get_LastRenderSubmissionStats();
			for (const Client::EFFECT_GPU_RENDER_FAMILY_STATS& Family :
				Stats.Families)
			{
				OutEvidence.Aggregate.iConfigured += Family.iConfigured;
				OutEvidence.Aggregate.iEvaluated += Family.iEvaluated;
				OutEvidence.Aggregate.iActive += Family.iActive;
				OutEvidence.Aggregate.iCandidate += Family.iCandidate;
				OutEvidence.Aggregate.iAttempted += Family.iAttempted;
				OutEvidence.Aggregate.iSubmitted += Family.iSubmitted;
				OutEvidence.Aggregate.iSuppressed += Family.iSuppressed;
				OutEvidence.Aggregate.iFailed += Family.iFailed;
			}
			OutEvidence.Aggregate.bCompleted = Stats.bCompleted;
			OutEvidence.Aggregate.bCommitted = Stats.bCommitted;
			if (FAILED(OutEvidence.hRenderResult) ||
				S_OK != OutEvidence.hDeviceResult ||
				!Stats.bCompleted || !Stats.bCommitted)
			{
				strOutError = "production GameInstance::Render frame failed "
					"[render=" + std::to_string(static_cast<int64_t>(
						OutEvidence.hRenderResult)) + ", device=" +
					std::to_string(static_cast<int64_t>(
						OutEvidence.hDeviceResult)) + ", completed=" +
					(Stats.bCompleted ? "true" : "false") + ", committed=" +
					(Stats.bCommitted ? "true" : "false") + "]: " +
					pObject->Get_Status();
				return false;
			}
			strOutError.clear();
			return true;
		}

	private:
		HINSTANCE m_hInstance = nullptr;
		HWND m_hWnd = nullptr;
		ATOM m_ClassAtom = 0u;
		bool_t m_bEngineInitialized = false;
		ComPtr<ID3D11Device> m_pDevice;
		ComPtr<ID3D11DeviceContext> m_pContext;
	};

	AGGREGATE_STATS Read_FamilyStats(
		const Client::EFFECT_GPU_RENDER_SUBMISSION_STATS& Stats,
		const Client::EFFECT_GPU_RENDER_FAMILY eFamily)
	{
		const Client::EFFECT_GPU_RENDER_FAMILY_STATS& Family =
			Stats.Families[static_cast<size_t>(eFamily)];
		AGGREGATE_STATS Result;
		Result.iConfigured = Family.iConfigured;
		Result.iEvaluated = Family.iEvaluated;
		Result.iActive = Family.iActive;
		Result.iCandidate = Family.iCandidate;
		Result.iAttempted = Family.iAttempted;
		Result.iSubmitted = Family.iSubmitted;
		Result.iSuppressed = Family.iSuppressed;
		Result.iFailed = Family.iFailed;
		Result.bCompleted = Stats.bCompleted;
		Result.bCommitted = Stats.bCommitted;
		return Result;
	}

	std::optional<OCCURRENCE_EVIDENCE> Find_OccurrenceEvidence(
		const Client::EFFECT_GPU_RENDER_SUBMISSION_STATS& Stats,
		const std::string_view strElementId)
	{
		const auto Iterator = std::find_if(Stats.Occurrences.begin(),
			Stats.Occurrences.end(), [strElementId](const auto& Row)
			{
				return Row.strElementId == strElementId;
			});
		if (Iterator == Stats.Occurrences.end())
			return std::nullopt;
		OCCURRENCE_EVIDENCE Result;
		Result.strElementId = Iterator->strElementId;
		Result.iActive = Iterator->iActive;
		Result.iCandidate = Iterator->iCandidateRowCount;
		Result.iAttempted = Iterator->iAttempted;
		Result.iSubmitted = Iterator->iSubmitted;
		Result.iSuppressed = Iterator->iSuppressed;
		Result.iFailed = Iterator->iFailed;
		Result.iMaterialBindCount = Iterator->iMaterialBindCount;
		Result.iTextureSrvBindCount = Iterator->iTextureSrvBindCount;
		Result.iSamplerBindCount = Iterator->iSamplerBindCount;
		Result.iShaderPassApplyCount = Iterator->iShaderPassApplyCount;
		Result.iVIBufferBindCount = Iterator->iVIBufferBindCount;
		Result.iVIBufferDrawCount = Iterator->iVIBufferDrawCount;
		Result.iIssuedDrawCallCount = Iterator->iIssuedDrawCallCount;
		Result.iDrawSelectionCount = Iterator->iDrawSelectionCount;
		Result.iCompiledAdapterPipelineValidationCount =
			Iterator->iCompiledAdapterPipelineValidationCount;
		Result.iSelectedPassIndex = Iterator->iSelectedPassIndex;
		Result.eCarrier = Iterator->eCarrier;
		Result.bDrawSelectionDiverged = Iterator->bDrawSelectionDiverged;
		return Result;
	}

	std::optional<OCCURRENCE_EVIDENCE> Find_UniqueOccurrenceEvidence(
		const Client::EFFECT_GPU_RENDER_SUBMISSION_STATS& Stats,
		const Client::EFFECT_GPU_RENDER_FAMILY eFamily)
	{
		const Client::EFFECT_GPU_RENDER_OCCURRENCE_STATS* pMatch = nullptr;
		for (const Client::EFFECT_GPU_RENDER_OCCURRENCE_STATS& Row :
			Stats.Occurrences)
		{
			if (Row.eFamily != eFamily)
				continue;
			if (nullptr != pMatch)
				return std::nullopt;
			pMatch = &Row;
		}
		return nullptr == pMatch ? std::nullopt :
			Find_OccurrenceEvidence(Stats, pMatch->strElementId);
	}

	const char* Carrier_Token(const Client::EFFECT_GPU_RENDER_CARRIER eCarrier)
	{
		switch (eCarrier)
		{
		case Client::EFFECT_GPU_RENDER_CARRIER::MESH_CMODEL:
			return "MESH_CMODEL";
		case Client::EFFECT_GPU_RENDER_CARRIER::SPRITE_RECT:
			return "SPRITE_RECT";
		case Client::EFFECT_GPU_RENDER_CARRIER::SPRITE_INSTANCE:
			return "SPRITE_INSTANCE";
		case Client::EFFECT_GPU_RENDER_CARRIER::DECAL_RECT:
			return "DECAL_RECT";
		case Client::EFFECT_GPU_RENDER_CARRIER::RIBBON_DYNAMIC_TRAIL:
			return "RIBBON_DYNAMIC_TRAIL";
		default:
			return "END";
		}
	}

	bool_t Set_EnvironmentPath(const wchar_t* pName,
		const std::filesystem::path& Path, std::string& strOutError)
	{
		const std::wstring Value = Path.wstring();
		if (Value.empty() || !SetEnvironmentVariableW(pName, Value.c_str()))
		{
			strOutError = "failed to set a harness path environment variable";
			return false;
		}
		return true;
	}

	std::shared_ptr<Client::CEffectObject> Add_EffectObject(
		const wchar_t* pPrototypeTag, const wchar_t* pLayerTag,
		Client::CEffectObject::EFFECT_OBJECT_DESC& Desc)
	{
		std::shared_ptr<Engine::CGameObject> BaseObject;
		if (FAILED(Engine::CGameInstance::Get().Add_GameObject_to_Layer(
			ETOUI(Client::LEVEL::STATIC), pPrototypeTag,
			ETOUI(Client::LEVEL::DEVELOPMENT), pLayerTag, &Desc, &BaseObject)))
		{
			return nullptr;
		}
		return std::dynamic_pointer_cast<Client::CEffectObject>(BaseObject);
	}

	void Write_AggregateJson(const AGGREGATE_STATS& Stats)
	{
		std::cout << "{\"configured\":" << Stats.iConfigured <<
			",\"evaluated\":" << Stats.iEvaluated <<
			",\"active\":" << Stats.iActive <<
			",\"candidate\":" << Stats.iCandidate <<
			",\"attempted\":" << Stats.iAttempted <<
			",\"submitted\":" << Stats.iSubmitted <<
			",\"suppressed\":" << Stats.iSuppressed <<
			",\"failed\":" << Stats.iFailed <<
			",\"completed\":" << (Stats.bCompleted ? "true" : "false") <<
			",\"committed\":" << (Stats.bCommitted ? "true" : "false") << "}";
	}

	void Write_FrameJson(const FRAME_EVIDENCE& Frame)
	{
		std::cout << "{\"scenarioId\":\"" << Frame.strScenarioId <<
			"\",\"contract\":\"" << Frame.strContract <<
			"\",\"sampleTimeSeconds\":" << Frame.fSampleTimeSeconds <<
			",\"gameInstanceRenderHresult\":";
		if (Frame.bGameInstanceRender)
			std::cout << static_cast<int64_t>(Frame.hRenderResult);
		else
			std::cout << "null";
		std::cout << ",\"deviceHresult\":";
		if (Frame.bGameInstanceRender)
			std::cout << static_cast<int64_t>(Frame.hDeviceResult);
		else
			std::cout << "null";
		std::cout << ",\"stats\":";
		Write_AggregateJson(Frame.Aggregate);
		if (Frame.Occurrence.has_value())
		{
			const OCCURRENCE_EVIDENCE& Row = *Frame.Occurrence;
			std::cout << ",\"occurrence\":{\"elementId\":\"" <<
				Row.strElementId << "\",\"active\":" << Row.iActive <<
				",\"candidate\":" << Row.iCandidate <<
				",\"attempted\":" << Row.iAttempted <<
				",\"submitted\":" << Row.iSubmitted <<
				",\"suppressed\":" << Row.iSuppressed <<
				",\"failed\":" << Row.iFailed <<
				",\"materialBinds\":" << Row.iMaterialBindCount <<
				",\"textureSrvBinds\":" << Row.iTextureSrvBindCount <<
				",\"samplerBinds\":" << Row.iSamplerBindCount <<
				",\"shaderPassApplies\":" << Row.iShaderPassApplyCount <<
				",\"viBufferBinds\":" << Row.iVIBufferBindCount <<
				",\"viBufferDraws\":" << Row.iVIBufferDrawCount <<
				",\"issuedDrawCalls\":" << Row.iIssuedDrawCallCount <<
				",\"drawSelections\":" << Row.iDrawSelectionCount <<
				",\"compiledAdapterPipelineValidations\":" <<
					Row.iCompiledAdapterPipelineValidationCount <<
				",\"carrier\":\"" << Carrier_Token(Row.eCarrier) <<
				"\",\"passIndex\":" << Row.iSelectedPassIndex <<
				",\"drawSelectionDiverged\":" <<
					(Row.bDrawSelectionDiverged ? "true" : "false") << "}";
		}
		std::cout << ",\"boundAdapterActualPipelineValidated\":" <<
			(Frame.bBoundAdapterActualPipelineValidated ? "true" : "false");
		std::cout << "}";
	}

	const char* RuntimeRenderer_Token(
		const Client::EFFECT_RUNTIME_RENDERER_KIND eRenderer)
	{
		switch (eRenderer)
		{
		case Client::EFFECT_RUNTIME_RENDERER_KIND::MESH_PARTICLE:
			return "MESH_PARTICLE";
		case Client::EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE:
			return "SPRITE_PARTICLE";
		case Client::EFFECT_RUNTIME_RENDERER_KIND::DECAL_PARTICLE:
			return "DECAL_PARTICLE";
		case Client::EFFECT_RUNTIME_RENDERER_KIND::CASCADE_RIBBON:
			return "CASCADE_RIBBON";
		case Client::EFFECT_RUNTIME_RENDERER_KIND::LIGHT_PARTICLE:
			return "LIGHT_PARTICLE";
		case Client::EFFECT_RUNTIME_RENDERER_KIND::SCREEN_POST:
			return "SCREEN_POST";
		default:
			return "END";
		}
	}

	void Write_Full35DispositionJson(
		const FULL35_FIXED_STEP_DISPOSITION& Disposition)
	{
		std::cout << "{\"effectAssetId\":\"" << ARTIST_FULL_EFFECT_ID <<
			"\",\"sampleTimeSeconds\":" << Disposition.fSampleTimeSeconds <<
			",\"fixedStepIndex\":" << Disposition.iFixedStepIndex <<
			",\"activeParticlePacketCount\":" <<
				Disposition.iActiveParticlePacketCount <<
			",\"activeBasis\":\"CPU_FIXED_STEP_INSPECTOR_ACTIVE_PHASE_OR_LIVE_PARTICLE\""
			",\"dispositionBasis\":\"ARTIST31470_SHADER_REGISTRY_DRAW_ADMISSION\""
			",\"attempted\":" << Disposition.iAttempted <<
			",\"submitted\":" << Disposition.iSubmitted <<
			",\"suppressed\":" << Disposition.iSuppressed <<
			",\"failed\":" << Disposition.iFailed <<
			",\"committed\":" <<
				(Disposition.bCommitted ? "true" : "false") <<
			",\"stateProjectionSha256\":\"" <<
				Disposition.strStateProjectionSha256 <<
			"\",\"frameProjectionSha256\":\"" <<
				Disposition.strFrameProjectionSha256 << "\",\"rows\":[";
		for (size_t iRow = 0u; iRow < Disposition.Rows.size(); ++iRow)
		{
			if (0u != iRow)
				std::cout << ',';
			const FULL35_DISPOSITION_ROW& Row = Disposition.Rows[iRow];
			std::cout << "{\"occurrenceId\":\"" << Row.strOccurrenceId <<
				"\",\"elementId\":\"" << Row.strElementId <<
				"\",\"renderer\":\"" << RuntimeRenderer_Token(Row.eRenderer) <<
				"\",\"passIndex\":";
			if (Client::EFFECT_ARTIST31470_NO_PASS == Row.iPassIndex)
				std::cout << "null";
			else
				std::cout << Row.iPassIndex;
			std::cout << ",\"drawAdmitted\":" <<
				(Row.bDrawAdmitted ? "true" : "false") << '}';
		}
		std::cout << "]}";
	}

	bool_t Is_ArtistCoreRenderer(
		const Client::EFFECT_RUNTIME_RENDERER_KIND eRenderer)
	{
		switch (eRenderer)
		{
		case Client::EFFECT_RUNTIME_RENDERER_KIND::MESH_PARTICLE:
		case Client::EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE:
		case Client::EFFECT_RUNTIME_RENDERER_KIND::DECAL_PARTICLE:
		case Client::EFFECT_RUNTIME_RENDERER_KIND::CASCADE_RIBBON:
			return true;
		default:
			return false;
		}
	}

	bool_t Build_ArtistFull35FixedStepDisposition(
		const std::filesystem::path& CandidatePath,
		const uint64_t iCatalogRevision,
		FULL35_FIXED_STEP_DISPOSITION& OutDisposition,
		std::string& strOutError)
	{
		using namespace Client;
		constexpr double SAMPLE_TIME_SECONDS = 1.5;
		constexpr uint32_t INPUT_ARTIFACT_COUNT = 13u;
		constexpr uint64_t CANDIDATE_BYTE_COUNT = 15'121'873u;
		constexpr std::string_view COMPILER_REVISION =
			"artist31470.reconstructed-runtime-program-link-v1";

		std::ifstream Candidate(CandidatePath, std::ios::binary);
		if (!Candidate.is_open())
		{
			strOutError = "Full35 candidate could not be opened.";
			return false;
		}
		const std::string CandidateUtf8{
			std::istreambuf_iterator<char>(Candidate),
			std::istreambuf_iterator<char>()};
		if (Candidate.bad() || CandidateUtf8.size() != CANDIDATE_BYTE_COUNT)
		{
			strOutError = "Full35 candidate byte identity changed.";
			return false;
		}
		Write_Progress("full35-candidate-read.complete");

		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM_IDENTITY FrozenIdentity =
			CEffectRuntimeAuthorityCodec::Get_FrozenArtist31470FProgramIdentity();
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> Program;
		if (!CEffectRuntimeAuthorityCodec::Parse_ReconstructedRuntimeProgram(
			CandidateUtf8, FrozenIdentity, Program, strOutError) ||
			nullptr == Program)
		{
			if (strOutError.empty())
				strOutError = "Full35 frozen candidate Program parse failed.";
			return false;
		}
		Write_Progress("full35-candidate-parse.complete");

		EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY CatalogIdentity;
		CatalogIdentity.iCatalogRevision = iCatalogRevision;
		CatalogIdentity.iArtifactRevision = 1u;
		CatalogIdentity.iProgramVersion = FrozenIdentity.iProgramVersion;
		CatalogIdentity.iInputArtifactCount = INPUT_ARTIFACT_COUNT;
		CatalogIdentity.iCandidateByteCount = CANDIDATE_BYTE_COUNT;
		CatalogIdentity.strEffectAssetId = ARTIST_FULL_EFFECT_ID;
		CatalogIdentity.strCompilerRevision = COMPILER_REVISION;
		CatalogIdentity.strCandidateBuilderCommitId =
			FrozenIdentity.strBuilderAuthorityCommitId;
		CatalogIdentity.strCandidateBuilderTreeId =
			FrozenIdentity.strBuilderAuthorityTreeId;
		CatalogIdentity.strProgramId = FrozenIdentity.strProgramId;
		CatalogIdentity.strProgramSha256 = FrozenIdentity.strProgramSha256;
		CatalogIdentity.strCandidateRawSha256 =
			FrozenIdentity.strCandidateRawSha256;

		std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN> Plan;
		std::shared_ptr<const EFFECT_RECONSTRUCTED_CPU_INSPECTION_STATE> State;
		std::shared_ptr<const EFFECT_RECONSTRUCTED_CPU_INSPECTION_FRAME> Frame;
		if (Program->strRuntimeCatalogAssetId !=
				ARTIST_FULL_EFFECT_ID || Program->Emitters.size() != 35u ||
			!Validate_Artist31470ShaderRegistry() ||
			Get_Artist31470ShaderRegistry().size() != 35u)
		{
			strOutError = "Full35 Program/registry fixed denominator changed.";
			return false;
		}
		if (!CEffectReconstructedExecutionPlanCompiler::Compile_ProgramForTests(
			CatalogIdentity, Program, Plan, strOutError) || nullptr == Plan)
		{
			if (strOutError.empty())
				strOutError = "Full35 fixed-step compiler failed.";
			return false;
		}
		Write_Progress("full35-plan-compile.complete");
		if (!CEffectReconstructedCpuInspector::Simulate(
			Plan, SAMPLE_TIME_SECONDS, State, Frame, strOutError) ||
			nullptr == State || nullptr == Frame)
		{
			if (strOutError.empty())
				strOutError = "Full35 fixed-step CPU inspector failed.";
			return false;
		}
		if (State->Get_Plan() != Plan || Frame->Get_Plan() != Plan ||
			State->Get_FixedStepIndex() != 90u ||
			Frame->Get_FixedStepIndex() != State->Get_FixedStepIndex())
		{
			strOutError = "Full35 fixed-step inspector identity changed.";
			return false;
		}
		Write_Progress("full35-cpu-inspector.complete");

		std::unordered_map<std::string,
			const EFFECT_RECONSTRUCTED_CPU_EMITTER_STATE*> StatesByEmitter;
		for (const EFFECT_RECONSTRUCTED_CPU_EMITTER_STATE& EmitterState :
			State->Get_Emitters())
		{
			if (!StatesByEmitter.emplace(
				EmitterState.strEmitterId, &EmitterState).second)
			{
				strOutError = "Full35 inspector contains a duplicate emitter state.";
				return false;
			}
		}
		std::unordered_map<std::string, uint64_t> PacketCountsByEmitter;
		for (const EFFECT_RECONSTRUCTED_CPU_OCCURRENCE_PACKET& Packet :
			Frame->Get_ActiveOccurrences())
		{
			const auto Emitter = Plan->Get_Emitters().find(Packet.strEmitterId);
			if (Emitter == Plan->Get_Emitters().end() ||
				Emitter->second.eRenderer != Packet.eRenderer)
			{
				strOutError =
					"Full35 inspector packet lost its compiled emitter renderer.";
				return false;
			}
			++PacketCountsByEmitter[Packet.strEmitterId];
		}
		if (StatesByEmitter.size() != Program->Emitters.size() ||
			Plan->Get_Emitters().size() != Program->Emitters.size())
		{
			strOutError = "Full35 Program/Plan/inspector denominator changed.";
			return false;
		}

		std::unordered_map<std::string,
			const EFFECT_RUNTIME_PROGRAM_MATERIAL_OCCURRENCE*>
			MaterialOccurrencesById;
		for (const EFFECT_RUNTIME_PROGRAM_MATERIAL_OCCURRENCE& Occurrence :
			Program->MaterialOccurrences)
		{
			if (!MaterialOccurrencesById.emplace(
				Occurrence.Row.strId, &Occurrence).second)
			{
				strOutError =
					"Full35 Program contains a duplicate material occurrence.";
				return false;
			}
		}
		FULL35_FIXED_STEP_DISPOSITION Staged;
		Staged.fSampleTimeSeconds = static_cast<f32_t>(
			Frame->Get_SampleTimeSeconds());
		Staged.iFixedStepIndex = Frame->Get_FixedStepIndex();
		Staged.iActiveParticlePacketCount =
			static_cast<uint64_t>(Frame->Get_ActiveOccurrences().size());
		Staged.strStateProjectionSha256 = State->Get_ProjectionSha256();
		Staged.strFrameProjectionSha256 = Frame->Get_ProjectionSha256();
		std::unordered_set<std::string> ActiveOccurrenceIds;

		for (size_t iEmitter = 0u; iEmitter < Program->Emitters.size(); ++iEmitter)
		{
			const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter =
				Program->Emitters[iEmitter];
			const auto PlanEmitter = Plan->Get_Emitters().find(Emitter.Row.strId);
			const auto EmitterState = StatesByEmitter.find(Emitter.Row.strId);
			if (Emitter.Row.iOrder != iEmitter ||
				PlanEmitter == Plan->Get_Emitters().end() ||
				EmitterState == StatesByEmitter.end() ||
				PlanEmitter->second.iOrder != Emitter.Row.iOrder ||
				PlanEmitter->second.eRenderer != Emitter.eRenderer)
			{
				strOutError = "Full35 Program/Plan/emitter-state join changed.";
				return false;
			}

			const EFFECT_RECONSTRUCTED_CPU_EMITTER_STATE& Cpu =
				*EmitterState->second;
			const uint64_t iPacketCount = PacketCountsByEmitter.contains(
				Emitter.Row.strId) ? PacketCountsByEmitter.at(Emitter.Row.strId) : 0u;
			if (iPacketCount != Cpu.iActiveCount)
			{
				strOutError =
					"Full35 particle packet count differs from inspector active state.";
				return false;
			}
			if (!Emitter.bVisible || !Is_ArtistCoreRenderer(Emitter.eRenderer))
				continue;

			const bool_t bActive = 0u < Cpu.iActiveCount || Cpu.ePhase ==
				EFFECT_RECONSTRUCTED_CPU_EMITTER_PHASE::EMITTING;
			if (!bActive)
				continue;
			if (!Emitter.strMaterialOccurrenceId.has_value())
			{
				strOutError =
					"Full35 active core emitter has no material occurrence.";
				return false;
			}
			const auto MaterialOccurrence = MaterialOccurrencesById.find(
				*Emitter.strMaterialOccurrenceId);
			const std::optional<EFFECT_ARTIST31470_SHADER_REGISTRY_ROW> Shader =
				Find_Artist31470ShaderRegistry(
					Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId);
			if (MaterialOccurrence == MaterialOccurrencesById.end() ||
				!Shader.has_value() ||
				!Validate_Artist31470ShaderRegistryOccurrence(
					Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId,
					Emitter.strSourceElementId, Emitter.strSourceEmitterPath,
					*MaterialOccurrence->second) ||
				Shader->strRuntimeElementId != Emitter.strSourceElementId ||
				Shader->eRenderer != Emitter.eRenderer)
			{
				strOutError =
					"Full35 stable occurrence/shader-registry join changed.";
				return false;
			}
			if (!ActiveOccurrenceIds.emplace(
				*Emitter.strMaterialOccurrenceId).second)
			{
				strOutError =
					"Full35 active material occurrence identity is duplicated.";
				return false;
			}

			FULL35_DISPOSITION_ROW Row;
			Row.strOccurrenceId = *Emitter.strMaterialOccurrenceId;
			Row.strElementId = std::string(Shader->strRuntimeElementId);
			Row.eRenderer = Shader->eRenderer;
			Row.iPassIndex = Shader->iCurrentPassIndex;
			Row.bDrawAdmitted = Shader->bDrawAdmitted;
			Staged.Rows.emplace_back(std::move(Row));
			++Staged.iAttempted;
			if (Shader->bDrawAdmitted)
			{
				++Staged.iSubmitted;
			}
			else
			{
				++Staged.iSuppressed;
			}
		}

		Staged.bCommitted = 0u == Staged.iFailed &&
			Staged.iAttempted == Staged.Rows.size() &&
			Staged.iAttempted == ActiveOccurrenceIds.size() &&
			Staged.iSubmitted + Staged.iSuppressed == Staged.iAttempted &&
			Staged.strStateProjectionSha256.size() == 64u &&
			Staged.strFrameProjectionSha256.size() == 64u;
		if (!Staged.bCommitted)
		{
			strOutError = "Full35 fixed-step disposition denominator mismatched.";
			return false;
		}
		OutDisposition = std::move(Staged);
		Write_Progress("full35-disposition.complete");
		strOutError.clear();
		return true;
	}

	bool_t Is_ExactArtistFull35Baseline(
		const FULL35_FIXED_STEP_DISPOSITION& Disposition)
	{
		return Disposition.fSampleTimeSeconds == 1.5f &&
			Disposition.iFixedStepIndex == 90u &&
			Disposition.iActiveParticlePacketCount == 156u &&
			Disposition.iAttempted == 25u &&
			Disposition.iSubmitted == 22u &&
			Disposition.iSuppressed == 3u &&
			Disposition.iFailed == 0u && Disposition.bCommitted &&
			Disposition.Rows.size() == 25u &&
			Disposition.strStateProjectionSha256 ==
				"e2fee36b3f4e9383fb5f60f9f6dfa62d1a47558b013610abc35f1c088b9ed704" &&
			Disposition.strFrameProjectionSha256 ==
				"95830deb2d54d576c97985e628c4e432d788eb7de16ca4b07e39054865f10ad4";
	}

	bool_t Is_ExactArtistCanaryAggregate(const AGGREGATE_STATS& Stats)
	{
		return Stats.iConfigured == 17u && Stats.iEvaluated == 17u &&
			Stats.iActive == 14u && Stats.iCandidate == 14u &&
			Stats.iAttempted == 14u && Stats.iSubmitted == 1u &&
			Stats.iSuppressed == 13u && Stats.iFailed == 0u &&
			Stats.bCompleted && Stats.bCommitted;
	}

	bool_t Is_ExactArtistCanaryOccurrence(
		const OCCURRENCE_EVIDENCE& Occurrence,
		const bool_t bExpectedCanaryBinding)
	{
		return Occurrence.strElementId == ARTIST_CANARY_ELEMENT_ID &&
			Occurrence.iActive == 1u && Occurrence.iCandidate == 20u &&
			Occurrence.iAttempted == 1u && Occurrence.iSubmitted == 1u &&
			Occurrence.iSuppressed == 0u && Occurrence.iFailed == 0u &&
			Occurrence.iMaterialBindCount == 1u &&
			Occurrence.iTextureSrvBindCount == 12u &&
			Occurrence.iSamplerBindCount == 2u &&
			Occurrence.iShaderPassApplyCount == 1u &&
			Occurrence.iVIBufferBindCount == 1u &&
			Occurrence.iVIBufferDrawCount == 1u &&
			Occurrence.iIssuedDrawCallCount == 1u &&
			Occurrence.iDrawSelectionCount == 1u &&
			Occurrence.iCompiledAdapterPipelineValidationCount ==
				(bExpectedCanaryBinding ? 1u : 0u) &&
			Occurrence.eCarrier ==
				Client::EFFECT_GPU_RENDER_CARRIER::SPRITE_INSTANCE &&
			Occurrence.iSelectedPassIndex == 1u &&
			!Occurrence.bDrawSelectionDiverged;
	}

	bool_t Is_ExactLanceFloor(const AGGREGATE_STATS& Stats)
	{
		return Stats.iConfigured == 1u && Stats.iEvaluated == 1u &&
			Stats.iActive == 1u && Stats.iCandidate == 0u &&
			Stats.iAttempted == 1u && Stats.iSubmitted == 0u &&
			Stats.iSuppressed == 1u && Stats.iFailed == 0u &&
			Stats.bCompleted && Stats.bCommitted;
	}

	bool_t Is_ExactLanceTick32(const AGGREGATE_STATS& Stats)
	{
		return Stats.iConfigured == 1u && Stats.iEvaluated == 1u &&
			Stats.iActive == 1u && Stats.iCandidate == 1u &&
			Stats.iAttempted == 1u && Stats.iSubmitted == 1u &&
			Stats.iSuppressed == 0u && Stats.iFailed == 0u &&
			Stats.bCompleted && Stats.bCommitted;
	}
}

int wmain(const int argc, wchar_t** argv)
{
	std::cout << std::unitbuf;
	if (argc != 4 || nullptr == argv[1] || nullptr == argv[2] ||
		nullptr == argv[3])
	{
		std::cerr << "usage: EffectRenderContractHarness <repo-root> "
			"<expected-binding-count:0|1> <runtime-catalog>\n";
		return 2;
	}

	wchar_t* pEnd = nullptr;
	const unsigned long ExpectedBindingCount = wcstoul(argv[2], &pEnd, 10);
	if (nullptr == pEnd || L'\0' != *pEnd || ExpectedBindingCount > 1u)
	{
		std::cerr << "expected binding count must be 0 or 1\n";
		return 2;
	}

	std::error_code FileError;
	const std::filesystem::path RepositoryRoot =
		std::filesystem::weakly_canonical(argv[1], FileError);
	const std::filesystem::path ClientWorkingDirectory =
		RepositoryRoot / L"Client" / L"Default";
	const std::filesystem::path RuntimeCatalogPath =
		std::filesystem::weakly_canonical(argv[3], FileError);
	const std::filesystem::path ArtistFullCandidatePath =
		RepositoryRoot / L"Data" / L"Effects" / L"Imported" / L"Artist" /
			L"Candidates" /
			L"skill.31470.reconstructed-runtime-program.candidate.json";
	const std::filesystem::path ResourceRoot =
		RepositoryRoot / L"Client" / L"Bin" / L"Resources";
	if (FileError ||
		!std::filesystem::is_directory(ClientWorkingDirectory, FileError) ||
		FileError || !std::filesystem::is_regular_file(RuntimeCatalogPath, FileError) ||
		FileError || !std::filesystem::is_regular_file(
			ArtistFullCandidatePath, FileError) ||
		FileError || !std::filesystem::is_directory(ResourceRoot, FileError) ||
		FileError)
	{
		std::cerr << "required repository runtime inputs are unavailable\n";
		return 2;
	}

	std::string Status;
	if (!Set_EnvironmentPath(L"LOSTARK_EFFECT_RUNTIME_CATALOG_FIXTURE",
			RuntimeCatalogPath, Status) ||
		!Set_EnvironmentPath(L"LOSTARK_RESOURCE_ROOT", ResourceRoot, Status) ||
		!Set_EnvironmentPath(L"LOSTARK_PROJECT_DATA_ROOT",
			RepositoryRoot / L"Data", Status))
	{
		std::cerr << Status << '\n';
		return 2;
	}
	std::filesystem::current_path(ClientWorkingDirectory, FileError);
	if (FileError)
	{
		std::cerr << "could not enter Client/Default\n";
		return 2;
	}

	Engine::Set_NonInteractiveErrorMode(true);
	Client::CEffectDocumentRenderer::Clear_Prepared_Catalog();
	Client::CEffectCatalog::Clear();
	Write_Progress("catalog-load.begin");
	if (!Client::CEffectCatalog::Load(Status))
	{
		std::cerr << "catalog load failed: " << Status << '\n';
		return 1;
	}
	Write_Progress("catalog-load.complete");

	const uint64_t CatalogRevision = Client::CEffectCatalog::Get_RuntimeRevision();
	const std::shared_ptr<const Client::CEffectMaterialProgramRegistry> Registry =
		Client::CEffectCatalog::Acquire_MaterialProgramRegistry();
	if (nullptr == Registry || Registry->Get_CatalogRevision() != CatalogRevision ||
		Registry->Get_GenerationId() != CatalogRevision ||
		Registry->Get_BindingCount() != ExpectedBindingCount)
	{
		std::cerr << "registry generation or expected binding count mismatched\n";
		return 1;
	}

	const std::shared_ptr<const Client::EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING>
		CanaryBinding = Registry->Resolve(
			ARTIST_UNIFIED_EFFECT_ID, ARTIST_CANARY_ELEMENT_ID);
	const bool_t bExpectedCanaryBinding = 1u == ExpectedBindingCount;
	const bool_t bCanaryBindingExact = bExpectedCanaryBinding ?
		(nullptr != CanaryBinding &&
		 CanaryBinding->iCatalogRevision == CatalogRevision &&
		 CanaryBinding->iRegistryGenerationId == Registry->Get_GenerationId() &&
		 CanaryBinding->strEffectAssetId == ARTIST_UNIFIED_EFFECT_ID &&
		 CanaryBinding->strElementId == ARTIST_CANARY_ELEMENT_ID &&
		 CanaryBinding->strProgramId == ARTIST_CANARY_PROGRAM_ID &&
		 CanaryBinding->strLayoutId == ARTIST_CANARY_LAYOUT_ID &&
		 CanaryBinding->strDescriptorId == ARTIST_CANARY_DESCRIPTOR_ID &&
		 CanaryBinding->strAdapterId ==
			 Client::EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ADAPTER_ID &&
		 CanaryBinding->Adapter.eAdapterId ==
			 Client::EFFECT_COMPILED_MATERIAL_ADAPTER_ID::
			 SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1 &&
		 CanaryBinding->Adapter.eCarrier ==
			 Client::EFFECT_COMPILED_MATERIAL_CARRIER::SPRITE_PARTICLE &&
		 CanaryBinding->Adapter.strAdapterId ==
			 Client::EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ADAPTER_ID &&
		 CanaryBinding->Adapter.strShaderId == "Shader_VtxEffectParticle.hlsl" &&
		 CanaryBinding->Adapter.strVertexLayoutId == "VTXEFFECT_PARTICLE" &&
		 CanaryBinding->Adapter.iPassIndex == 1u &&
		 CanaryBinding->Adapter.strMrtId == "MRT_SceneHDR" &&
		 CanaryBinding->Adapter.iSceneColorRenderTargetIndex == 0u &&
		 CanaryBinding->Adapter.strSceneColorSemantic == "SV_TARGET0" &&
		 CanaryBinding->Adapter.iDistortionRenderTargetIndex == 1u &&
		 CanaryBinding->Adapter.strDistortionSemantic == "SV_TARGET1" &&
		 CanaryBinding->Adapter.bDistortionDeterministicZero &&
		 CanaryBinding->Adapter.strRasterizerState == "RS_Cull_None" &&
		 CanaryBinding->Adapter.strDepthStencilState == "DSS_ReadOnly" &&
		 CanaryBinding->Adapter.strBlendState == "BS_EffectAlpha" &&
		 CanaryBinding->Adapter.iStencilReference == 0u) :
		(nullptr == CanaryBinding);
	if (!bCanaryBindingExact)
	{
		std::cerr << "canary Binding 0/1 identity mismatched\n";
		return 1;
	}

	FULL35_FIXED_STEP_DISPOSITION Full35Disposition;
	Write_Progress("full35.begin");
	if (!Build_ArtistFull35FixedStepDisposition(
		ArtistFullCandidatePath, CatalogRevision, Full35Disposition, Status))
	{
		std::cerr << "Artist Full35 fixed-step disposition failed: " <<
			Status << '\n';
		return 1;
	}
	if (!Is_ExactArtistFull35Baseline(Full35Disposition))
	{
		std::cerr << "Artist Full35 Binding baseline drifted [attempted=" <<
			Full35Disposition.iAttempted << ", submitted=" <<
			Full35Disposition.iSubmitted << ", suppressed=" <<
			Full35Disposition.iSuppressed << ", failed=" <<
			Full35Disposition.iFailed << ", committed=" <<
			(Full35Disposition.bCommitted ? "true" : "false") << "]\n";
		return 1;
	}

	std::vector<FRAME_EVIDENCE> Frames;

	Client::EFFECT_RENDER_PREWARM_PROBE PrewarmProbe{};
	{
		HEADLESS_ENGINE_SCOPE EngineScope;
		Write_Progress("warp-engine-initialize.begin");
		if (!EngineScope.Initialize(Status))
		{
			std::cerr << "WARP Engine initialization failed: " << Status << '\n';
			return 1;
		}
		Write_Progress("warp-engine-initialize.complete");
		const ComPtr<ID3D11Device> Device = EngineScope.Get_Device();
		const ComPtr<ID3D11DeviceContext> Context = EngineScope.Get_Context();
		constexpr const wchar_t* PrototypeTag =
			L"Prototype_GameObject_EffectRenderContract";
		constexpr const wchar_t* LayerTag = L"Layer_EffectRenderContract";
		if (FAILED(Engine::CGameInstance::Get().Add_Prototype(
			ETOUI(Client::LEVEL::STATIC), PrototypeTag,
			Client::CEffectObject::Create(Device, Context))))
		{
			std::cerr << "EffectObject prototype registration failed\n";
			return 1;
		}
		float4x4_t Identity{};
		XMStoreFloat4x4(&Identity, XMMatrixIdentity());

		const std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC> CanaryDocument =
			Client::CEffectCatalog::Find(std::string(ARTIST_UNIFIED_EFFECT_ID));
		const std::shared_ptr<const Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			CanaryProjection = Client::CEffectCatalog::Find_VisualProjection(
				std::string(ARTIST_UNIFIED_EFFECT_ID));
		Client::EFFECT_RENDER_PREWARM_TARGET CanaryTarget;
		CanaryTarget.strEffectAssetId = ARTIST_UNIFIED_EFFECT_ID;
		CanaryTarget.pDocument = CanaryDocument;
		CanaryTarget.pVisualProgramProjection = CanaryProjection;
		CanaryTarget.pMaterialProgramRegistry = Registry;
		Write_Progress("artist-canary-prewarm.begin");
		if (nullptr == CanaryDocument ||
			!Client::CEffectDocumentRenderer::Prepare_VisualProgramTarget(
				Device, Context, CatalogRevision, CanaryTarget, Status))
		{
			std::cerr << "Artist canary Product prewarm failed: " << Status << '\n';
			return 1;
		}
		Write_Progress("artist-canary-prewarm.complete");
		const auto CanaryPrepared = Client::CEffectDocumentRenderer::Find_Prepared(
			CatalogRevision, std::string(ARTIST_UNIFIED_EFFECT_ID),
			*CanaryDocument, CanaryProjection, Registry);
		Client::CEffectObject::EFFECT_OBJECT_DESC CanaryDesc{};
		CanaryDesc.pDocument = CanaryDocument.get();
		CanaryDesc.pPreparedResources = CanaryPrepared;
		CanaryDesc.pVisualProgramProjection = CanaryProjection;
		CanaryDesc.RootWorld = Identity;
		CanaryDesc.bAutoPlay = false;
		CanaryDesc.bRequirePreparedResources = true;
		if (nullptr != CanaryProjection)
			CanaryDesc.pDocument = nullptr;
		Write_Progress("artist-canary-stage.begin");
		const std::shared_ptr<Client::CEffectObject> CanaryObject =
			Add_EffectObject(PrototypeTag, LayerTag, CanaryDesc);
		if (nullptr == CanaryPrepared || nullptr == CanaryObject)
		{
			std::cerr << "Artist canary stage failed: " << Status << '\n';
			return 1;
		}
		if (!CanaryObject->Set_TestPreviewElementIsolation(
				{ std::string(ARTIST_CANARY_ELEMENT_ID) }, Status))
		{
			std::cerr << "Artist canary Element isolation failed: " <<
				Status << '\n';
			return 1;
		}
		Write_Progress("artist-canary-stage.complete");
		FRAME_EVIDENCE CanaryFrame;
		CanaryFrame.strScenarioId = "artist-f-canary-first-occurrence-1.5s";
		CanaryFrame.strContract = "GAMEINSTANCE_WARP_MRT_SCENE_HDR";
		Write_Progress("artist-canary-render.begin");
		if (!EngineScope.Render_Frame(
				CanaryObject, 1.5f, CanaryFrame, Status))
		{
			std::cerr << "Artist canary render failed: " << Status << '\n';
			return 1;
		}
		Write_Progress("artist-canary-render.complete");
		CanaryFrame.Occurrence = Find_OccurrenceEvidence(
			CanaryObject->Get_LastRenderSubmissionStats(), ARTIST_CANARY_ELEMENT_ID);
		const bool_t bCanaryDrawExact =
			Is_ExactArtistCanaryAggregate(CanaryFrame.Aggregate) &&
			CanaryFrame.Occurrence.has_value() &&
			Is_ExactArtistCanaryOccurrence(
				*CanaryFrame.Occurrence, bExpectedCanaryBinding);
		if (!bCanaryDrawExact)
		{
			std::cerr << "Artist canary carrier/pass/draw evidence mismatched\n";
			return 1;
		}
		/* A bound Sprite draw can reach this point only after the production
		   renderer has validated the compiled adapter against the selected
		   carrier/pass, actual D3D state, and active MRT render targets. */
		CanaryFrame.bBoundAdapterActualPipelineValidated =
			CanaryFrame.Occurrence->
				iCompiledAdapterPipelineValidationCount == 1u;
		Frames.emplace_back(std::move(CanaryFrame));

		const std::shared_ptr<const Client::EFFECT_VISUAL_PROGRAM_CORPUS>
			VisualProgramCorpus = Client::CEffectCatalog::Find_VisualProgramCorpus();
		Client::EFFECT_DOCUMENT_DESC LanceBaseDocument;
		std::shared_ptr<const Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			LanceProjection;
		const std::filesystem::path LanceBaseDocumentPath =
			RepositoryRoot / L"Data" / L"Effects" / L"Authored" /
				L"effect.lancemaster.skill.34010.ba1.effect.json";
		Write_Progress("lance-ba1-projection.begin");
		if (nullptr == VisualProgramCorpus ||
			!Client::CEffectDocumentCodec::Load(
				LanceBaseDocumentPath, LanceBaseDocument, Status) ||
			!Client::CEffectVisualProgramCorpusCodec::Create_DocumentProjection(
				*VisualProgramCorpus, LanceBaseDocument, LanceProjection, Status) ||
			nullptr == LanceProjection || !LanceProjection->Is_Valid() ||
			LanceProjection->Get_EffectAssetId() != LANCE_EFFECT_ID)
		{
			std::cerr << "Lance BA1 production projection failed: " <<
				Status << '\n';
			return 1;
		}
		Write_Progress("lance-ba1-projection.complete");
		const std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC> LanceDocument =
			LanceProjection->Get_DocumentShared();
		Client::EFFECT_RENDER_PREWARM_TARGET LanceTarget;
		LanceTarget.strEffectAssetId = LANCE_EFFECT_ID;
		LanceTarget.pDocument = LanceDocument;
		LanceTarget.pVisualProgramProjection = LanceProjection;
		LanceTarget.pMaterialProgramRegistry = Registry;
		Write_Progress("lance-ba1-prewarm.begin");
		if (nullptr == LanceDocument ||
			!Client::CEffectDocumentRenderer::Prepare_VisualProgramTarget(
				Device, Context, CatalogRevision, LanceTarget, Status))
		{
			std::cerr << "Lance BA1 Product prewarm failed: " << Status << '\n';
			return 1;
		}
		Write_Progress("lance-ba1-prewarm.complete");
		const auto LancePrepared = Client::CEffectDocumentRenderer::Find_Prepared(
			CatalogRevision, std::string(LANCE_EFFECT_ID), *LanceDocument,
			LanceProjection, Registry);
		Client::CEffectObject::EFFECT_OBJECT_DESC LanceDesc{};
		LanceDesc.pDocument = LanceDocument.get();
		LanceDesc.pPreparedResources = LancePrepared;
		LanceDesc.pVisualProgramProjection = LanceProjection;
		LanceDesc.RootWorld = Identity;
		LanceDesc.bAutoPlay = false;
		LanceDesc.bRequirePreparedResources = true;
		if (nullptr != LanceProjection)
			LanceDesc.pDocument = nullptr;
		Write_Progress("lance-ba1-stage.begin");
		const std::shared_ptr<Client::CEffectObject> LanceObject =
			Add_EffectObject(PrototypeTag, LayerTag, LanceDesc);
		if (nullptr == LancePrepared || nullptr == LanceObject)
		{
			std::cerr << "Lance BA1 stage failed\n";
			return 1;
		}
		Write_Progress("lance-ba1-stage.complete");

		FRAME_EVIDENCE LanceFloorFrame;
		LanceFloorFrame.strScenarioId = "lance-ba1-floor-0.5207s";
		LanceFloorFrame.strContract = "GAMEINSTANCE_WARP_MRT_SCENE_HDR";
		Write_Progress("lance-ba1-floor-render.begin");
		if (!EngineScope.Render_Frame(
				LanceObject, 0.5207f, LanceFloorFrame, Status))
		{
			std::cerr << "Lance BA1 floor render failed: " << Status << '\n';
			return 1;
		}
		Write_Progress("lance-ba1-floor-render.complete");
		LanceFloorFrame.Aggregate = Read_FamilyStats(
			LanceObject->Get_LastRenderSubmissionStats(),
			Client::EFFECT_GPU_RENDER_FAMILY::MESH);
		LanceFloorFrame.Occurrence = Find_UniqueOccurrenceEvidence(
			LanceObject->Get_LastRenderSubmissionStats(),
			Client::EFFECT_GPU_RENDER_FAMILY::MESH);
		const bool_t bLanceFloorOccurrenceExact =
			LanceFloorFrame.Occurrence.has_value() &&
			LanceFloorFrame.Occurrence->strElementId == LANCE_BA1_OCCURRENCE_ID &&
			LanceFloorFrame.Occurrence->iActive == 1u &&
			LanceFloorFrame.Occurrence->iCandidate == 0u &&
			LanceFloorFrame.Occurrence->iAttempted == 1u &&
			LanceFloorFrame.Occurrence->iSubmitted == 0u &&
			LanceFloorFrame.Occurrence->iSuppressed == 1u &&
			LanceFloorFrame.Occurrence->iFailed == 0u;
		if (!Is_ExactLanceFloor(LanceFloorFrame.Aggregate) ||
			!bLanceFloorOccurrenceExact)
		{
			std::cerr << "Lance BA1 floor occurrence stats mismatched\n";
			return 1;
		}
		const std::string LanceOccurrenceId =
			LanceFloorFrame.Occurrence->strElementId;
		Frames.emplace_back(std::move(LanceFloorFrame));

		FRAME_EVIDENCE LanceTick32Frame;
		LanceTick32Frame.strScenarioId = "lance-ba1-tick32";
		LanceTick32Frame.strContract = "GAMEINSTANCE_WARP_MRT_SCENE_HDR";
		Write_Progress("lance-ba1-tick32-render.begin");
		if (!EngineScope.Render_Frame(
				LanceObject, 32.f / 60.f, LanceTick32Frame, Status))
		{
			std::cerr << "Lance BA1 tick32 render failed: " << Status << '\n';
			return 1;
		}
		Write_Progress("lance-ba1-tick32-render.complete");
		LanceTick32Frame.Aggregate = Read_FamilyStats(
			LanceObject->Get_LastRenderSubmissionStats(),
			Client::EFFECT_GPU_RENDER_FAMILY::MESH);
		LanceTick32Frame.Occurrence = Find_UniqueOccurrenceEvidence(
			LanceObject->Get_LastRenderSubmissionStats(),
			Client::EFFECT_GPU_RENDER_FAMILY::MESH);
		const bool_t bLanceTick32OccurrenceExact =
			LanceTick32Frame.Occurrence.has_value() &&
			LanceOccurrenceId == LANCE_BA1_OCCURRENCE_ID &&
			LanceTick32Frame.Occurrence->strElementId == LANCE_BA1_OCCURRENCE_ID &&
			LanceTick32Frame.Occurrence->iActive == 1u &&
			LanceTick32Frame.Occurrence->iCandidate == 1u &&
			LanceTick32Frame.Occurrence->iAttempted == 1u &&
			LanceTick32Frame.Occurrence->iSubmitted == 1u &&
			LanceTick32Frame.Occurrence->iSuppressed == 0u &&
			LanceTick32Frame.Occurrence->iFailed == 0u &&
			LanceTick32Frame.Occurrence->iMaterialBindCount == 1u &&
			LanceTick32Frame.Occurrence->iTextureSrvBindCount == 12u &&
			LanceTick32Frame.Occurrence->iSamplerBindCount == 1u &&
			LanceTick32Frame.Occurrence->iShaderPassApplyCount == 1u &&
			LanceTick32Frame.Occurrence->iVIBufferBindCount == 1u &&
			LanceTick32Frame.Occurrence->iVIBufferDrawCount == 1u &&
			LanceTick32Frame.Occurrence->iIssuedDrawCallCount == 1u &&
			LanceTick32Frame.Occurrence->iDrawSelectionCount == 1u &&
			LanceTick32Frame.Occurrence->
				iCompiledAdapterPipelineValidationCount == 0u &&
			LanceTick32Frame.Occurrence->eCarrier ==
				Client::EFFECT_GPU_RENDER_CARRIER::MESH_CMODEL &&
			LanceTick32Frame.Occurrence->iSelectedPassIndex == 1u &&
			!LanceTick32Frame.Occurrence->bDrawSelectionDiverged;
		if (!Is_ExactLanceTick32(LanceTick32Frame.Aggregate) ||
			!bLanceTick32OccurrenceExact)
		{
			std::cerr << "Lance BA1 tick32 carrier/pass/draw evidence mismatched\n";
			return 1;
		}
		Frames.emplace_back(std::move(LanceTick32Frame));

		PrewarmProbe = Client::CEffectDocumentRenderer::Get_PrewarmProbe();
		if (PrewarmProbe.iCatalogRevision != CatalogRevision ||
			PrewarmProbe.iMaterialProgramRegistryGeneration !=
				Registry->Get_GenerationId() ||
			PrewarmProbe.iMaterialProgramBindingCount != ExpectedBindingCount ||
			PrewarmProbe.iMaterialProgramResolvedElementCount !=
				ExpectedBindingCount)
		{
			std::cerr << "prepared registry generation/count propagation mismatched\n";
			return 1;
		}
	}

	Client::CEffectDocumentRenderer::Clear_Prepared_Catalog();
	Client::CEffectCatalog::Clear();
	std::cout << "{\"schema\":\"lostark.effect-render-contract-harness-result\""
		",\"formatVersion\":2,\"configuration\":\""
#if defined(_DEBUG)
		"Debug"
#else
		"Release"
#endif
		"\",\"expectedBindingCount\":" << ExpectedBindingCount <<
		",\"actualBindingCount\":" << Registry->Get_BindingCount() <<
		",\"catalogRevision\":" << CatalogRevision <<
		",\"registryGeneration\":" << Registry->Get_GenerationId() <<
		",\"prewarm\":{\"catalogRevision\":" <<
			PrewarmProbe.iCatalogRevision <<
		",\"registryGeneration\":" <<
			PrewarmProbe.iMaterialProgramRegistryGeneration <<
		",\"bindingCount\":" << PrewarmProbe.iMaterialProgramBindingCount <<
		",\"resolvedElementCount\":" <<
			PrewarmProbe.iMaterialProgramResolvedElementCount << "}"
		",\"canaryBinding\":{\"present\":" <<
			(nullptr != CanaryBinding ? "true" : "false") <<
		",\"effectAssetId\":\"" << ARTIST_UNIFIED_EFFECT_ID <<
		"\",\"elementId\":\"" << ARTIST_CANARY_ELEMENT_ID <<
		"\",\"compiledCarrier\":\"" <<
			(nullptr == CanaryBinding ? "NONE" : "SPRITE_PARTICLE") <<
		"\",\"compiledPassIndex\":" <<
			(nullptr == CanaryBinding ? UINT32_MAX :
			 CanaryBinding->Adapter.iPassIndex) <<
		",\"compiledMrtId\":\"" <<
			(nullptr == CanaryBinding ? "" : CanaryBinding->Adapter.strMrtId) <<
		"\"},\"full35StructuralSentinel\":";
	Write_Full35DispositionJson(Full35Disposition);
	std::cout << ",\"frames\":[";
	for (size_t iFrame = 0u; iFrame < Frames.size(); ++iFrame)
	{
		if (0u != iFrame)
			std::cout << ',';
		Write_FrameJson(Frames[iFrame]);
	}
	std::cout << "]}\n";
	return 0;
}
