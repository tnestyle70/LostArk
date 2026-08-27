#include "Client_Defines.h"

#include "DataJson.h"
#include "Effect_Artist31470ShaderRegistry.h"
#include "Effect_Catalog.h"
#include "Effect_DirectAuthoredSourceIndex.h"
#include "Effect_DocumentCodec.h"
#include "Effect_DocumentRenderer.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_MaterialProgramRegistry.h"
#include "Effect_Object.h"
#include "Effect_OccurrenceTuning.h"
#include "Effect_Playback.h"
#include "Effect_ReconstructedExecution.h"
#include "Effect_RuntimeAuthority.h"
#include "Effect_ScreenOverlayPresentation.h"
#include "Effect_VisualProgramCorpus.h"
#include "GameInstance.h"
#include "Level.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <Windows.h>
#include <DirectXPackedVector.h>

namespace
{
	constexpr std::string_view ARTIST_FULL_EFFECT_ID =
		"effect.artist.skill.31470";
	constexpr std::string_view ARTIST_UNIFIED_EFFECT_ID =
		"effect.artist.skill.31470.unified";
	constexpr std::string_view ARTIST_E_CRANE_EFFECT_ID =
		"effect.artist.skill.31480.unified";
	constexpr std::string_view ARTIST_E_CRANE_CUE_ID =
		"artist_31480_crane_projectile";
	constexpr std::string_view ARTIST_E_CRANE_MODEL_ASSET_ID =
		"Effect/Artist/Meshes/SK_SDM_RCC_00_SK_FX_01/"
		"SK_SDM_RCC_00_SK_FX_01.wmodel";
	constexpr std::string_view ARTIST_E_CRANE_CLIP_NAME =
		"rcc_sk_flyinheaven";
	constexpr std::string_view ARTIST_E_CRANE_WITNESS_BONE =
		"b_l_wing_01";
	constexpr std::string_view ARTIST_CANARY_ELEMENT_ID =
		"sprite.2b3dc6842507e910";
	constexpr std::string_view ARTIST_CANARY_PROGRAM_ID =
		"effect.program.runtime-material-v2.opcode-6.v1";
	constexpr std::string_view ARTIST_CANARY_LAYOUT_ID =
		"effect.layout.runtime-material-v2.opcode-6.abi-3aafae1b4639c551.v1";
	constexpr std::string_view ARTIST_CANARY_DESCRIPTOR_ID =
		"effect.descriptor.artist-f.sprite-2b3dc6842507e910.v1";
	constexpr std::string_view ARTIST_MESH_CANARY_ELEMENT_ID =
		"mesh.062366ee9f9655d3";
	constexpr std::string_view ARTIST_MESH_CANARY_PROGRAM_ID =
		"effect.program.runtime-material-v2.opcode-3.v1";
	constexpr std::string_view ARTIST_MESH_CANARY_LAYOUT_ID =
		"effect.layout.runtime-material-v2.opcode-3.abi-85c02e5f1f646d22.v1";
	constexpr std::string_view ARTIST_MESH_CANARY_DESCRIPTOR_ID =
		"effect.descriptor.artist-f.mesh-062366ee9f9655d3.v1";
	constexpr std::string_view ARTIST_DECAL_CANARY_ELEMENT_ID =
		"decal.f3b5c3b63b4a7e34";
	constexpr std::string_view ARTIST_DECAL_CANARY_PROGRAM_ID =
		"effect.program.local-decal.opcode-14.v1";
	constexpr std::string_view ARTIST_DECAL_CANARY_LAYOUT_ID =
		"effect.layout.local-decal.opcode-14.abi-c6b52a791b98f0c5.v1";
	constexpr std::string_view ARTIST_DECAL_CANARY_DESCRIPTOR_ID =
		"effect.descriptor.artist-f.decal-f3b5c3b63b4a7e34.v1";
	constexpr std::string_view LANCE_EFFECT_ID =
		"effect.lancemaster.skill.34010.ba1";
	constexpr std::string_view LANCE_BA1_OCCURRENCE_ID =
		"authored.approx.s002.mesh01";
	struct ARTIST_PARTICLE_MASTER_STAGE_DESC final
	{
		uint32_t iStageNumber = 0u;
		std::string_view strEffectAssetId;
		std::string_view strElementId;
	};

	constexpr std::array<ARTIST_PARTICLE_MASTER_STAGE_DESC, 4u>
		ARTIST_PARTICLE_MASTER_STAGES = {{
		{ 1u, "effect.artist.skill.31000.ba1.unified",
			"authored.source-particle.a41eed26b6f8b7f9df8233da" },
		{ 2u, "effect.artist.skill.31000.ba2.unified",
			"authored.source-particle.25e72dfb5a9a416facc02888" },
		{ 3u, "effect.artist.skill.31000.ba3.unified",
			"authored.source-particle.65e208f117586003f28a3e91" },
		{ 4u, "effect.artist.skill.31000.ba4.unified",
			"authored.source-particle.50c7b1854919386dc3630e1e" }
	}};
	constexpr size_t REPRESENTATIVE_V1_BINDING_COUNT = 131u;
	constexpr size_t REPRESENTATIVE_V1_TOTAL_BINDING_COUNT = 134u;
	constexpr size_t REPRESENTATIVE_V1_REGISTRY_ADAPTER_COUNT = 7u;
	constexpr size_t GOLDEN_AND_REPRESENTATIVE_COMPILED_ADAPTER_COUNT =
		static_cast<size_t>(Client::EFFECT_COMPILED_MATERIAL_ADAPTER_ID::
			PROJECT_TUNED_SPRITE_PARTICLE_ALPHA_TWO_SIDED_V1);
	constexpr std::array<std::string_view, 5u> REPRESENTATIVE_V1_EFFECT_IDS = {{
		"effect.artist.skill.31460.v1.unified",
		"effect.dimensionmaster.skill.2050180.v1.unified",
		"effect.lancemaster.skill.34110.v1.unified",
		"effect.warlord.skill.17110.clip2.v1.unified",
		"effect.warlord.skill.17110.clip3.v1.unified"
	}};

	struct DIRECT_AUTHORED_INDEX_PROBE_DESC final
	{
		std::string_view strEffectAssetId;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		LostArk::Shared::SKILL_ID iSkillId =
			LostArk::Shared::INVALID_SKILL_ID;
	};

	constexpr std::array<DIRECT_AUTHORED_INDEX_PROBE_DESC, 13u>
		DIRECT_AUTHORED_INDEX_PROBES = {{
		{
			"effect.artist.skill.31460.unified",
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST,
			31460u
		},
		{
			"effect.artist.skill.31460.linear-reveal.unified",
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST,
			31460u
		},
		{
			"effect.artist.skill.31460.v1.unified",
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST,
			31460u
		},
		{
			"effect.dimensionmaster.skill.2050180.unified",
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER,
			2050180u
		},
		{
			"effect.dimensionmaster.skill.2050180.v1.unified",
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER,
			2050180u
		},
		{
			"effect.lancemaster.skill.34110.unified",
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,
			34110u
		},
		{
			"effect.lancemaster.skill.34110.v1.unified",
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,
			34110u
		},
		{
			"effect.lancemaster.skill.34040.clip1.unified",
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,
			34040u
		},
		{
			"effect.lancemaster.skill.34040.clip2.unified",
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,
			34040u
		},
		{
			"effect.warlord.skill.17110.clip2.unified",
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD,
			17110u
		},
		{
			"effect.warlord.skill.17110.clip2.v1.unified",
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD,
			17110u
		},
		{
			"effect.warlord.skill.17110.clip3.unified",
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD,
			17110u
		},
		{
			"effect.warlord.skill.17110.clip3.v1.unified",
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD,
			17110u
		}
	}};

	struct REPRESENTATIVE_V1_DRAW_PROBE_DESC final
	{
		std::string_view strScenarioStem;
		std::string_view strEffectAssetId;
		std::string_view strElementId;
		f32_t fSampleTimeSeconds = 0.f;
		Client::EFFECT_GPU_RENDER_CARRIER eCarrier =
			Client::EFFECT_GPU_RENDER_CARRIER::END;
		uint32_t iPassIndex = UINT32_MAX;
	};

	constexpr std::array<REPRESENTATIVE_V1_DRAW_PROBE_DESC, 6u>
		REPRESENTATIVE_V1_DRAW_PROBES = {{
		{
			"representative-v1-sprite-alpha-one",
			"effect.artist.skill.31460.v1.unified",
			"authored.source-particle.cb346af47371feedccf9b652",
			0.75f,
			Client::EFFECT_GPU_RENDER_CARRIER::SPRITE_INSTANCE,
			3u
		},
		{
			"representative-v1-sprite-additive-two",
			"effect.lancemaster.skill.34110.v1.unified",
			"authored.source-particle.b790ac14ac613d42bdfb6a58",
			0.05f,
			Client::EFFECT_GPU_RENDER_CARRIER::SPRITE_INSTANCE,
			2u
		},
		{
			"representative-v1-sprite-additive-one",
			"effect.dimensionmaster.skill.2050180.v1.unified",
			"authored.source-particle.65691ec294c8f04b9f0cad89",
			0.55f,
			Client::EFFECT_GPU_RENDER_CARRIER::SPRITE_INSTANCE,
			4u
		},
		{
			"representative-v1-mesh-alpha-one",
			"effect.dimensionmaster.skill.2050180.v1.unified",
			"authored.source-particle.98639f5f2e65e0f0193c09fe",
			0.55f,
			Client::EFFECT_GPU_RENDER_CARRIER::MESH_CMODEL,
			3u
		},
		{
			"representative-v1-mesh-subuv-alpha-two",
			"effect.warlord.skill.17110.clip3.v1.unified",
			"authored.source-particle.60c8e8ed9d5b0c41b7aa4e88",
			0.55f,
			Client::EFFECT_GPU_RENDER_CARRIER::MESH_CMODEL,
			1u
		},
		{
			"representative-v1-decal-alpha-two",
			"effect.artist.skill.31460.v1.unified",
			"authored.source-decal.7139d7fc84cfc2aee0b40621",
			0.75f,
			Client::EFFECT_GPU_RENDER_CARRIER::DECAL_RECT,
			1u
		}
	}};
	constexpr uint32_t WINDOW_WIDTH = 320u;
	constexpr uint32_t WINDOW_HEIGHT = 180u;
	constexpr wchar_t WINDOW_CLASS_NAME[] =
		L"LostArkEffectRenderContractHarness";
	constexpr std::string_view VALTAN_COMBAT_OBJECT_SWEEP_MODE =
		"--write-valtan-combat-object-runtime-sweep";
	constexpr std::array<std::string_view, 5u> VALTAN_SKY_AXE_ELEMENTS = {{
		"mesh.valtan.sky-axe.descent",
		"particle.valtan.sky-axe.impact",
		"sky-axe-target-inner-fill",
		"authored.copy.sky-axe-target-inner-fill.1",
		"sky-axe-flight-line"
	}};
	constexpr std::array<std::string_view, 5u> VALTAN_RED_BLADE_ELEMENTS = {{
		"source.055adf57f38ac8f25be3",
		"source.5cb81ed1867daa4c331a",
		"source.5d987e41721843a8d390",
		"source.94fe8920096dda4a4e73",
		"source.f997148180378f188150"
	}};
	constexpr f32_t VALTAN_SKY_AXE_OWNER_LIFETIME_SECONDS = 6.f;

	void Write_Progress(const std::string_view strStage)
	{
		std::cerr << "[effect-render-contract] stage=" << strStage << '\n';
	}

	bool_t Validate_ArtistECraneCloneAnimationContract(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		Client::EFFECT_MODEL_CUE_ANIMATION_PROBE& OutProbe,
		std::string& strOutError)
	{
		const std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC> RuntimeDocument =
			Client::CEffectCatalog::Find(std::string(ARTIST_E_CRANE_EFFECT_ID));
		if (nullptr == RuntimeDocument)
		{
			strOutError =
				"Artist E crane Authored Product document failed to load.";
			return false;
		}
		const Client::EFFECT_DOCUMENT_DESC& SourceDocument = *RuntimeDocument;
		const auto Crane = std::find_if(SourceDocument.ModelCues.begin(),
			SourceDocument.ModelCues.end(),
			[](const Client::EFFECT_MODEL_CUE_DESC& Cue)
			{
				return Cue.strCueId == ARTIST_E_CRANE_CUE_ID;
			});
		if (SourceDocument.strEffectAssetId != ARTIST_E_CRANE_EFFECT_ID ||
			SourceDocument.ModelCues.size() != 1u ||
			Crane == SourceDocument.ModelCues.end() ||
			Crane->strModelAssetId != ARTIST_E_CRANE_MODEL_ASSET_ID ||
			Crane->strClipName != ARTIST_E_CRANE_CLIP_NAME ||
			!Crane->bVisible || !Crane->bHoldLastFrame ||
			std::abs(Crane->fStartDelaySeconds - 1.f) > 0.0001f ||
			Crane->fDurationSeconds <= 1.f ||
			std::abs(Crane->LocalTransform.vVelocityPerSecond.x) > 0.0001f ||
			std::abs(Crane->LocalTransform.vVelocityPerSecond.y) > 0.0001f ||
			std::abs(Crane->LocalTransform.vVelocityPerSecond.z - 9.f) > 0.0001f)
		{
			strOutError = "Artist E crane runtime Model Cue identity drifted.";
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC ModelCueOnly = SourceDocument;
		ModelCueOnly.Elements.clear();
		ModelCueOnly.ModelCues = { *Crane };
		ModelCueOnly.ModelCues.front().LocalTransform.vVelocityPerSecond = {};
		ModelCueOnly.ModelCues.front().LocalTransform.
			vRevolutionDegreesPerSecond = {};
		const std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC> Fixture =
			std::make_shared<Client::EFFECT_DOCUMENT_DESC>(std::move(ModelCueOnly));
		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>
			Prepared;
		if (!Client::CEffectDocumentRenderer::
				Prepare_UnboundMaterialProgramDocumentForTests(
					pDevice, pContext, Fixture, nullptr, Prepared, strOutError) ||
			nullptr == Prepared ||
			!Client::CEffectDocumentRenderer::
				Probe_ModelCueCloneAnimationForTests(
					pDevice, pContext, Prepared, ARTIST_E_CRANE_CUE_ID,
					ARTIST_E_CRANE_WITNESS_BONE, OutProbe, strOutError))
		{
			strOutError = "Artist E crane clone animation probe failed: " +
				strOutError;
			return false;
		}
		if (OutProbe.fPrototypeDurationSeconds < 1.05f ||
			OutProbe.fPrototypeDurationSeconds > 1.08f ||
			std::abs(OutProbe.fCloneDurationSeconds -
				OutProbe.fPrototypeDurationSeconds) > 0.00001f ||
			OutProbe.fMidTrackPositionTicks < 15.9f ||
			OutProbe.fMidTrackPositionTicks > 16.1f ||
			OutProbe.fTrackDurationTicks < 31.9f ||
			OutProbe.fTrackDurationTicks > 32.1f ||
			std::abs(OutProbe.fTailTrackPositionTicks -
				OutProbe.fTrackDurationTicks) > 0.001f ||
			OutProbe.fWitnessBoneMaximumDelta <= 0.0001f)
		{
			strOutError =
				"Artist E crane clone timing or wing animation evidence drifted.";
			return false;
		}

		Client::EFFECT_MODEL_CUE_ANIMATION_PROBE InvalidProbe;
		std::string Rejection;
		if (Client::CEffectDocumentRenderer::
				Probe_ModelCueCloneAnimationForTests(
					pDevice, pContext, Prepared, "missing.artist-e.crane-cue",
					ARTIST_E_CRANE_WITNESS_BONE, InvalidProbe, Rejection) ||
			Rejection.empty())
		{
			strOutError = "Unknown Artist E crane cue did not fail closed.";
			return false;
		}
		strOutError.clear();
		return true;
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
		uint32_t iSourceMaterialProfile = UINT32_MAX;
		Client::EFFECT_GPU_RENDER_CARRIER eCarrier =
			Client::EFFECT_GPU_RENDER_CARRIER::END;
		bool_t bDrawSelectionDiverged = false;
		uint64_t iFirstSubmittedWorldHash = 0u;
		bool_t bHasFirstSubmittedWorld = false;
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
		bool_t bSceneHdrReadback = false;
		f32_t fSceneHdrBloomThreshold = 0.f;
		f32_t fSceneHdrPeak = 0.f;
		float4_t vSceneHdrPeakPixel{};
		uint64_t iSceneHdrPixelsAboveThreshold = 0u;
	};

	struct REPRESENTATIVE_V1_PREPARED_TARGET final
	{
		std::string strEffectAssetId;
		std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC> pDocument;
		std::shared_ptr<const Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection;
		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>
			pPrepared;
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

		bool_t Begin_Frame(std::string& strOutError) const
		{
			const float4_t ClearColor(0.f, 0.f, 0.f, 0.f);
			if (!m_bEngineInitialized ||
				FAILED(Engine::CGameInstance::Get().Render_Begin(&ClearColor)))
			{
				strOutError = "production Render_Begin failed";
				return false;
			}
			strOutError.clear();
			return true;
		}

		bool_t Render_Frame(const std::shared_ptr<Client::CEffectObject>& pObject,
			const f32_t fSampleTimeSeconds, FRAME_EVIDENCE& OutEvidence,
			std::string& strOutError,
			const std::optional<f32_t> SceneHdrBloomThreshold = std::nullopt) const
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
			if (SceneHdrBloomThreshold.has_value() &&
				!Read_SceneHdr(*SceneHdrBloomThreshold, OutEvidence, strOutError))
			{
				return false;
			}
			strOutError.clear();
			return true;
		}

	private:
		bool_t Read_SceneHdr(const f32_t fBloomThreshold,
			FRAME_EVIDENCE& OutEvidence, std::string& strOutError) const
		{
			const ComPtr<ID3D11ShaderResourceView> SceneHdr =
				Engine::CGameInstance::Get().Get_RT_SRV(L"Target_SceneHDR");
			ComPtr<ID3D11Resource> Resource;
			ComPtr<ID3D11Texture2D> Texture;
			if (nullptr == SceneHdr ||
				(SceneHdr->GetResource(Resource.GetAddressOf()), nullptr == Resource) ||
				FAILED(Resource.As(&Texture)) || nullptr == Texture)
			{
				strOutError = "Target_SceneHDR resource is unavailable";
				return false;
			}

			D3D11_TEXTURE2D_DESC Desc{};
			Texture->GetDesc(&Desc);
			if (Desc.Format != DXGI_FORMAT_R16G16B16A16_FLOAT ||
				Desc.MipLevels != 1u || Desc.ArraySize != 1u ||
				Desc.SampleDesc.Count != 1u)
			{
				strOutError = "Target_SceneHDR readback format changed";
				return false;
			}
			Desc.Usage = D3D11_USAGE_STAGING;
			Desc.BindFlags = 0u;
			Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			Desc.MiscFlags = 0u;
			ComPtr<ID3D11Texture2D> Readback;
			if (FAILED(m_pDevice->CreateTexture2D(
					&Desc, nullptr, Readback.GetAddressOf())) ||
				nullptr == Readback ||
				FAILED(Engine::CGameInstance::Get().Copy_RT_Resource(
					L"Target_SceneHDR", Readback)))
			{
				strOutError = "Target_SceneHDR staging copy failed";
				return false;
			}

			D3D11_MAPPED_SUBRESOURCE Mapped{};
			if (FAILED(m_pContext->Map(
				Readback.Get(), 0u, D3D11_MAP_READ, 0u, &Mapped)))
			{
				strOutError = "Target_SceneHDR staging map failed";
				return false;
			}
			f32_t fPeak = 0.f;
			float4_t PeakPixel{};
			uint64_t iPixelsAboveThreshold = 0u;
			for (uint32_t y = 0u; y < Desc.Height; ++y)
			{
				const uint16_t* const pRow = reinterpret_cast<const uint16_t*>(
					static_cast<const std::byte*>(Mapped.pData) +
					static_cast<size_t>(y) * Mapped.RowPitch);
				for (uint32_t x = 0u; x < Desc.Width; ++x)
				{
					const uint16_t* const pPixel = pRow + x * 4u;
					const float4_t Pixel(
						DirectX::PackedVector::XMConvertHalfToFloat(pPixel[0]),
						DirectX::PackedVector::XMConvertHalfToFloat(pPixel[1]),
						DirectX::PackedVector::XMConvertHalfToFloat(pPixel[2]),
						DirectX::PackedVector::XMConvertHalfToFloat(pPixel[3]));
					if (!std::isfinite(Pixel.x) || !std::isfinite(Pixel.y) ||
						!std::isfinite(Pixel.z) || !std::isfinite(Pixel.w))
					{
						m_pContext->Unmap(Readback.Get(), 0u);
						strOutError =
							"Target_SceneHDR readback contains a non-finite pixel";
						return false;
					}
					const f32_t fPixelPeak = (std::max)(Pixel.x,
						(std::max)(Pixel.y, Pixel.z));
					if (fPixelPeak > fPeak)
					{
						fPeak = fPixelPeak;
						PeakPixel = Pixel;
					}
					if (fPixelPeak > fBloomThreshold)
						++iPixelsAboveThreshold;
				}
			}
			m_pContext->Unmap(Readback.Get(), 0u);

			OutEvidence.bSceneHdrReadback = true;
			OutEvidence.fSceneHdrBloomThreshold = fBloomThreshold;
			OutEvidence.fSceneHdrPeak = fPeak;
			OutEvidence.vSceneHdrPeakPixel = PeakPixel;
			OutEvidence.iSceneHdrPixelsAboveThreshold = iPixelsAboveThreshold;
			strOutError.clear();
			return true;
		}

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

	uint64_t Hash_FirstSubmittedWorld(const float4x4_t& World)
	{
		constexpr uint64_t FNV_OFFSET_BASIS = 14'695'981'039'346'656'037ull;
		constexpr uint64_t FNV_PRIME = 1'099'511'628'211ull;
		uint64_t Hash = FNV_OFFSET_BASIS;
		const uint8_t* pBytes = reinterpret_cast<const uint8_t*>(&World);
		for (size_t iByte = 0u; iByte < sizeof(World); ++iByte)
		{
			Hash ^= pBytes[iByte];
			Hash *= FNV_PRIME;
		}
		return Hash;
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
		Result.iSourceMaterialProfile = Iterator->iSourceMaterialProfile;
		Result.eCarrier = Iterator->eCarrier;
		Result.bDrawSelectionDiverged = Iterator->bDrawSelectionDiverged;
		Result.bHasFirstSubmittedWorld =
			Iterator->bHasFirstSubmittedParticleWorld;
		if (Result.bHasFirstSubmittedWorld)
		{
			Result.iFirstSubmittedWorldHash = Hash_FirstSubmittedWorld(
				Iterator->FirstSubmittedParticleWorld);
		}
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

	struct TEMP_SOURCE_CATALOG_FIXTURE final
	{
		TEMP_SOURCE_CATALOG_FIXTURE() = default;
		TEMP_SOURCE_CATALOG_FIXTURE(const TEMP_SOURCE_CATALOG_FIXTURE&) = delete;
		TEMP_SOURCE_CATALOG_FIXTURE& operator=(
			const TEMP_SOURCE_CATALOG_FIXTURE&) = delete;
		TEMP_SOURCE_CATALOG_FIXTURE(
			TEMP_SOURCE_CATALOG_FIXTURE&&) noexcept = default;
		TEMP_SOURCE_CATALOG_FIXTURE& operator=(
			TEMP_SOURCE_CATALOG_FIXTURE&&) noexcept = default;
		std::filesystem::path Root;
		std::filesystem::path CatalogPath;

		~TEMP_SOURCE_CATALOG_FIXTURE()
		{
			if (!Root.empty())
			{
				std::error_code Ignored;
				std::filesystem::remove_all(Root, Ignored);
			}
		}
	};

	bool_t Validate_DirectAuthoredAuditionSourceIndex(
		const std::filesystem::path& RepositoryRoot,
		std::string& strOutError)
	{
		const std::filesystem::path CatalogPath =
			RepositoryRoot / L"Data" / L"Effects" / L"EffectCatalog.json";
		const std::filesystem::path AuthoredRoot =
			RepositoryRoot / L"Data" / L"Effects" / L"Authored";
		std::vector<Client::EFFECT_DIRECT_AUTHORED_SCANNED_FILE> ScannedFiles;
		Client::EFFECT_DIRECT_AUTHORED_OWNER_SET ValidOwners;
		ScannedFiles.reserve(DIRECT_AUTHORED_INDEX_PROBES.size());
		for (const DIRECT_AUTHORED_INDEX_PROBE_DESC& Probe :
			DIRECT_AUTHORED_INDEX_PROBES)
		{
			const std::string AssetId(Probe.strEffectAssetId);
			const std::filesystem::path Path =
				AuthoredRoot / (AssetId + ".effect.json");
			std::error_code FileError;
			if (!std::filesystem::is_regular_file(Path, FileError) || FileError)
			{
				strOutError =
					"direct-authored index probe file is unavailable: " + AssetId;
				return false;
			}
			ScannedFiles.push_back({ AssetId, Path });
			ValidOwners.emplace(Probe.eCharacterClass, Probe.iSkillId);
		}

		Client::EFFECT_DIRECT_AUTHORED_SOURCE_INDEX Index;
		std::string Status;
		if (!Client::CEffectDirectAuthoredSourceIndex::Build(
				CatalogPath, AuthoredRoot, ScannedFiles, ValidOwners, {}, {},
				Index, Status))
		{
			strOutError =
				"direct-authored audition index rejected the source catalog: " +
				Status;
			return false;
		}
		if (Index.iCatalogDirectCount < DIRECT_AUTHORED_INDEX_PROBES.size() ||
			Index.Entries.size() != DIRECT_AUTHORED_INDEX_PROBES.size())
		{
			strOutError =
				"direct-authored audition index admitted an unexpected probe count";
			return false;
		}
		for (const DIRECT_AUTHORED_INDEX_PROBE_DESC& Probe :
			DIRECT_AUTHORED_INDEX_PROBES)
		{
			const auto Entry = std::find_if(Index.Entries.begin(),
				Index.Entries.end(), [&Probe](
					const Client::EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Candidate)
				{
					return Candidate.strEffectAssetId == Probe.strEffectAssetId;
				});
			if (Entry == Index.Entries.end() ||
				Entry->eOwnerKind != Client::
					EFFECT_DIRECT_AUTHORED_OWNER_KIND::PLAYER_SKILL ||
				Entry->eCharacterClass != Probe.eCharacterClass ||
				Entry->iSkillId != Probe.iSkillId || Entry->Path.empty())
			{
				strOutError =
					"direct-authored audition index lost or changed probe owner: " +
					std::string(Probe.strEffectAssetId);
				return false;
			}
		}

		/* Product-owner metadata is not editor admission. Simulate a globally
		   unavailable owner catalog and require every exact source probe to remain
		   openable while its Product join is isolated. */
		Client::EFFECT_DIRECT_AUTHORED_SOURCE_INDEX EditorOnlyIndex;
		Status.clear();
		if (!Client::CEffectDirectAuthoredSourceIndex::Build(
				CatalogPath, AuthoredRoot, ScannedFiles, {}, {}, {},
				EditorOnlyIndex, Status) ||
			EditorOnlyIndex.Entries.size() !=
				DIRECT_AUTHORED_INDEX_PROBES.size() ||
			EditorOnlyIndex.iOwnerJoinUnavailableCount <
				DIRECT_AUTHORED_INDEX_PROBES.size() ||
			EditorOnlyIndex.strFirstOwnerJoinUnavailable.empty() ||
			!std::all_of(EditorOnlyIndex.Entries.begin(),
				EditorOnlyIndex.Entries.end(),
				[](const Client::EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Entry)
				{
					return !Entry.Path.empty() &&
						Entry.eOwnerKind == Client::
							EFFECT_DIRECT_AUTHORED_OWNER_KIND::END;
				}))
		{
			strOutError =
				"direct-authored editor admission was coupled to Product owners: " +
				Status;
			return false;
		}

		TEMP_SOURCE_CATALOG_FIXTURE MalformedFixture;
		std::error_code FileError;
		MalformedFixture.Root = std::filesystem::temp_directory_path(FileError) /
			("LostArkDirectAuthoredIndex-" +
			 std::to_string(static_cast<uint64_t>(GetCurrentProcessId())) + "-" +
			 std::to_string(static_cast<uint64_t>(GetTickCount64())));
		MalformedFixture.CatalogPath =
			MalformedFixture.Root / L"EffectCatalog.json";
		if (FileError || !std::filesystem::create_directories(
				MalformedFixture.Root, FileError) || FileError)
		{
			strOutError =
				"direct-authored malformed-catalog fixture directory failed";
			return false;
		}
		{
			std::ofstream Output(MalformedFixture.CatalogPath,
				std::ios::binary | std::ios::trunc);
			Output << R"({"formatVersion":1,"effects":[{"effectAssetId":"effect.artist.skill.31460.unified","payloadKind":"DIRECT_AUTHORED_DOCUMENT","authoringPath":"Effects/Authored/effect.artist.skill.31460.unified.effect.json"},{"effectAssetId":"effect.artist.skill.31460.unified","payloadKind":"DIRECT_AUTHORED_DOCUMENT","authoringPath":"Effects/Authored/effect.artist.skill.31460.unified.effect.json"},{"effectAssetId":"effect.artist.skill.31460.linear-reveal.unified","payloadKind":"DIRECT_AUTHORED_DOCUMENT","authoringPath":"Effects/Authored/effect.artist.skill.31460.linear-reveal.unified.effect.json"},{"effectAssetId":"effect.artist.skill.31460.v1.unified","payloadKind":"DIRECT_AUTHORED_DOCUMENT","authoringPath":"Effects/Authored/effect.artist.skill.31460.v1.unified.effect.json","runtimeAdmission":"REGISTRY_BOUND_AUDITION_ONLY"}]})";
			if (!Output)
			{
				strOutError =
					"direct-authored malformed-catalog fixture write failed";
				return false;
			}
		}

		Client::EFFECT_DIRECT_AUTHORED_SOURCE_INDEX Isolated;
		Status.clear();
		if (!Client::CEffectDirectAuthoredSourceIndex::Build(
				MalformedFixture.CatalogPath, AuthoredRoot, ScannedFiles,
				ValidOwners, {}, {}, Isolated, Status) ||
			Isolated.iCatalogDirectCount != 4u ||
			Isolated.iUnavailableCount != 3u ||
			Isolated.Entries.size() != 1u ||
			Isolated.Entries.front().strEffectAssetId !=
				"effect.artist.skill.31460.linear-reveal.unified" ||
			Status.find("duplicate Effect ID") == std::string::npos ||
			Status.find("Isolated 3 unavailable rows") == std::string::npos)
		{
			strOutError =
				"malformed/duplicate direct-authored rows were not isolated from valid editors";
			return false;
		}

		strOutError.clear();
		return true;
	}

	bool_t Validate_SourceScreenOverlayParserContract(
		const std::filesystem::path& RepositoryRoot,
		std::string& strOutError)
	{
		const std::filesystem::path SourcePath = RepositoryRoot / L"Data" /
			L"Effects" / L"ScreenOverlays" /
			L"effect.dimensionmaster.skill.2050230.unified.screen-overlay.json";
		std::ifstream Input(SourcePath, std::ios::binary);
		if (!Input)
		{
			strOutError = "Screen-overlay parser source could not be opened";
			return false;
		}
		const std::string Source{
			std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>() };
		Client::CEffectScreenOverlayPresentation::SOURCE_DOCUMENT_MANIFEST
			Manifest;
		if (!Client::CEffectScreenOverlayPresentation::Parse_SourceDocument(
				Source, Manifest, strOutError) ||
			Manifest.strPresentationId !=
				"effect.dimensionmaster.skill.2050230.unified.screen-overlay" ||
			Manifest.TextureAssetIds != std::vector<std::string>{
				"Effect/DimensionMaster/Textures/FX_TEX_02/"
				"fx_d_fragment_005.dds" })
		{
			strOutError = strOutError.empty() ?
				"Screen-overlay source manifest identity drifted" : strOutError;
			return false;
		}

		const auto ExpectRejectedWithoutMutation = [&Manifest, &strOutError](
			const std::string& Candidate,
			const std::string_view Label)
		{
			const auto Before = Manifest;
			std::string Error;
			if (Client::CEffectScreenOverlayPresentation::Parse_SourceDocument(
					Candidate, Manifest, Error) || Error.empty() ||
				Manifest.strPresentationId != Before.strPresentationId ||
				Manifest.TextureAssetIds != Before.TextureAssetIds)
			{
				strOutError =
					"Screen-overlay parser did not fail transactionally: " +
					std::string(Label);
				return false;
			}
			return true;
		};
		std::string WrongVersion = Source;
		const size_t Version = WrongVersion.find("\"formatVersion\": 1");
		if (Version == std::string::npos)
		{
			strOutError = "Screen-overlay version witness is absent";
			return false;
		}
		WrongVersion.replace(Version, std::string("\"formatVersion\": 1").size(),
			"\"formatVersion\": 2");
		std::string EscapedTexture = Source;
		const size_t Texture = EscapedTexture.find("Effect/DimensionMaster/");
		if (Texture == std::string::npos)
		{
			strOutError = "Screen-overlay texture witness is absent";
			return false;
		}
		EscapedTexture.replace(Texture,
			std::string("Effect/DimensionMaster/").size(),
			"../Effect/DimensionMaster/");
		if (!ExpectRejectedWithoutMutation(WrongVersion, "wrong version") ||
			!ExpectRejectedWithoutMutation(EscapedTexture, "escaping texture"))
		{
			return false;
		}

		strOutError.clear();
		return true;
	}

	bool_t Validate_SourceDirectProductIdentity(
		const std::filesystem::path& RepositoryRoot,
		std::string& strOutError)
	{
		constexpr std::array<std::string_view, 8u> PRODUCT_WITNESSES{
			"effect.artist.skill.31200.unified",
			"effect.artist.skill.31460.unified",
			"effect.artist.skill.31460.linear-reveal.unified",
			"effect.lancemaster.skill.34110.unified",
			"effect.artist.skill.31950.unified",
			"effect.valtan.pattern.420633.active",
			"effect.valtan.environment.red-vortex-sky",
			"effect.dimensionmaster.skill.2050230.unified"
		};
		const std::filesystem::path AuthoredRoot =
			RepositoryRoot / L"Data" / L"Effects" / L"Authored";
		std::map<std::string, Client::EFFECT_DOCUMENT_DESC, std::less<>>
			SourceDocuments;
		for (const std::string_view EffectAssetId : PRODUCT_WITNESSES)
		{
			const std::string Id(EffectAssetId);
			Client::EFFECT_DOCUMENT_DESC Source;
			const std::filesystem::path SourcePath =
				AuthoredRoot / std::filesystem::path(Id + ".effect.json");
			const std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC> Product =
				Client::CEffectCatalog::Find(Id);
			const std::shared_ptr<const
				Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> Projection =
				Client::CEffectCatalog::Find_VisualProjection(Id);
			if (!Client::CEffectDocumentCodec::Load(
					SourcePath, Source, strOutError) || nullptr == Product ||
				!Client::CEffectCatalog::Is_DirectAuthoredDocument(Id) ||
				Client::CEffectDocumentCodec::Serialize(Source) !=
					Client::CEffectDocumentCodec::Serialize(*Product) ||
				((Source.iLoadedFormatVersion ==
					Client::EFFECT_AUTHORED_RUNTIME_EXTENSION_FORMAT_VERSION) !=
					(nullptr != Projection)) ||
				(nullptr != Projection &&
					(!Projection->Is_Valid() ||
					 Projection->Get_ProjectionKind() != Client::
						EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 ||
					 Projection->Get_DocumentShared().get() != Product.get())))
			{
				strOutError =
					"Product does not consume the exact direct-authored source: " +
					Id + (strOutError.empty() ? std::string{} :
						std::string("; ") + strOutError);
				return false;
			}
			SourceDocuments.emplace(Id, std::move(Source));
		}

		const std::shared_ptr<const
			Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> ArtistT =
			Client::CEffectCatalog::Find_VisualProjection(
				"effect.artist.skill.31950.unified");
		const std::shared_ptr<const
			Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> ValtanWhirlwind =
			Client::CEffectCatalog::Find_VisualProjection(
				"effect.valtan.pattern.420633.active");
		if (nullptr == ArtistT || nullptr == ValtanWhirlwind ||
			ArtistT->Get_AdmittedSupplementalElements().size() != 1u ||
			!ArtistT->Get_BakedEdgeHistories().empty() ||
			ArtistT->Get_AdmittedSupplementalElements().front().
				TargetIdentity.strTargetElementId !=
					"authored.source-particle.29868adeb040d5a35e2f213c" ||
			!ArtistT->Get_AdmittedSupplementalElements().front().
				CascadeRibbonPacket.has_value() ||
			ValtanWhirlwind->Get_AdmittedSupplementalElements().size() != 4u ||
			ValtanWhirlwind->Get_BakedEdgeHistories().size() != 1u ||
			ValtanWhirlwind->Get_BakedEdgeHistories().front().iSampleCount != 409u ||
			ValtanWhirlwind->Get_BakedEdgeHistories().front().Samples.size() != 409u)
		{
			strOutError =
				"Source-owned Artist/Valtan carrier projection count regressed";
			return false;
		}
		std::set<std::string> WhirlwindTrailTargets;
		std::set<std::string> WhirlwindLightTargets;
		for (const Client::EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental :
			ValtanWhirlwind->Get_AdmittedSupplementalElements())
		{
			if (Supplemental.AnimationTrailPacket.has_value() &&
				!Supplemental.CascadeRibbonPacket.has_value() &&
				!Supplemental.BakedEdgeLightPacket.has_value())
			{
				WhirlwindTrailTargets.emplace(
					Supplemental.TargetIdentity.strTargetElementId);
			}
			else if (Supplemental.BakedEdgeLightPacket.has_value() &&
				!Supplemental.CascadeRibbonPacket.has_value() &&
				!Supplemental.AnimationTrailPacket.has_value())
			{
				WhirlwindLightTargets.emplace(
					Supplemental.TargetIdentity.strTargetElementId);
			}
		}
		const std::set<std::string> ExpectedWhirlwindTrailTargets{
			"valtan.420633.notify004.emitter5258",
			"valtan.420633.notify004.emitter5259",
			"valtan.420633.notify004.emitter5260" };
		const std::set<std::string> ExpectedWhirlwindLightTargets{
			"valtan.420633.notify009.emitter6823" };
		if (WhirlwindTrailTargets != ExpectedWhirlwindTrailTargets ||
			WhirlwindLightTargets != ExpectedWhirlwindLightTargets)
		{
			strOutError =
				"Source-owned Valtan carrier target identities regressed";
			return false;
		}

		const auto FindElement = [&SourceDocuments](
			const std::string_view EffectAssetId,
			const std::string_view ElementId)
			-> const Client::EFFECT_ELEMENT_DESC*
		{
			const auto Document =
				SourceDocuments.find(std::string(EffectAssetId));
			if (Document == SourceDocuments.end())
				return nullptr;
			const auto Element = std::find_if(
				Document->second.Elements.begin(), Document->second.Elements.end(),
				[ElementId](const Client::EFFECT_ELEMENT_DESC& Candidate)
				{
					return Candidate.strElementId == ElementId;
				});
			return Element == Document->second.Elements.end() ?
				nullptr : &*Element;
		};
		const auto RotationIs = [](const Client::EFFECT_ELEMENT_DESC* pElement,
			const float fX, const float fY, const float fZ)
		{
			return nullptr != pElement &&
				std::abs(pElement->Detail.Transform.vRotationDegrees.x - fX) <
					1.0e-6f &&
				std::abs(pElement->Detail.Transform.vRotationDegrees.y - fY) <
					1.0e-6f &&
				std::abs(pElement->Detail.Transform.vRotationDegrees.z - fZ) <
					1.0e-6f;
		};
		if (!RotationIs(FindElement("effect.artist.skill.31200.unified",
				"authored.source-decal.45800d0c0054acd91f11cfb4"),
				0.f, -4.75f, 0.f) ||
			!RotationIs(FindElement("effect.artist.skill.31200.unified",
				"authored.copy.authored.source-decal.45800d0c0054acd91f11cfb4.1"),
				0.f, -2.f, 0.f) ||
			!RotationIs(FindElement("effect.artist.skill.31460.unified",
				"authored.source-decal.2f8ebdae3ea1e2a30fc91123"),
				0.f, 0.f, 0.f) ||
			!RotationIs(FindElement("effect.artist.skill.31460.unified",
				"authored.source-decal.7139d7fc84cfc2aee0b40621"),
				0.f, 0.f, 0.f) ||
			!RotationIs(FindElement(
				"effect.artist.skill.31460.linear-reveal.unified",
				"particle.artist.31460.grass-linear-reveal.hit1.v1"),
				90.f, 0.f, 0.f) ||
			!RotationIs(FindElement(
				"effect.artist.skill.31460.linear-reveal.unified",
				"particle.artist.31460.grass-linear-reveal.hit2.v1"),
				90.f, 0.f, 0.f))
		{
			strOutError =
				"Artist Q/A canonical decal/linear-reveal orientation regressed";
			return false;
		}

		const std::shared_ptr<const Client::CEffectMaterialProgramRegistry>
			Registry = Client::CEffectCatalog::Acquire_MaterialProgramRegistry();
		const std::shared_ptr<const Client::EFFECT_VISUAL_PROGRAM_CORPUS>
			VisualCorpus = Client::CEffectCatalog::Find_VisualProgramCorpus();
		if (nullptr == Registry || 0u != Registry->Get_BindingCount() ||
			nullptr == VisualCorpus || !VisualCorpus->Programs.empty())
		{
			strOutError =
				"Direct-authored Product was coupled to a derived material/visual override";
			return false;
		}

		const std::filesystem::path OverlayPath = RepositoryRoot / L"Data" /
			L"Effects" / L"ScreenOverlays" /
			L"effect.dimensionmaster.skill.2050230.unified.screen-overlay.json";
		std::ifstream OverlayInput(OverlayPath, std::ios::binary);
		if (!OverlayInput)
		{
			strOutError = "Screen-overlay source sidecar could not be opened";
			return false;
		}
		const std::string OverlayText{
			std::istreambuf_iterator<char>(OverlayInput),
			std::istreambuf_iterator<char>() };
		const std::shared_ptr<const
			Client::EFFECT_SCREEN_OVERLAY_PRODUCT_BINDING> Overlay =
			Client::CEffectCatalog::Find_ScreenOverlayProductBinding(
				"effect.dimensionmaster.skill.2050230.unified");
		if (nullptr == Overlay ||
			Overlay->strUtf8Json != OverlayText ||
			Overlay->strPresentationId !=
				"effect.dimensionmaster.skill.2050230.unified.screen-overlay")
		{
			strOutError =
				"Screen-overlay Product does not consume the exact source sidecar";
			return false;
		}

		strOutError.clear();
		return true;
	}

	bool_t Validate_SourcePlaybackTuningContract(
		const std::filesystem::path& RepositoryRoot,
		std::string& strOutError)
	{
		const std::filesystem::path AuthoredRoot = RepositoryRoot / L"Data" /
			L"Effects" / L"Authored";
		Client::EFFECT_DOCUMENT_DESC BudgetDocument;
		if (!Client::CEffectDocumentCodec::Load(
				AuthoredRoot /
					L"effect.lancemaster.skill.34580.ba1.unified.effect.json",
				BudgetDocument, strOutError) || BudgetDocument.Elements.size() != 1u)
		{
			strOutError = "source playback tuning budget fixture failed to load: " +
				strOutError;
			return false;
		}
		Client::EFFECT_ELEMENT_DESC& BudgetElement =
			BudgetDocument.Elements.front();
		BudgetElement.Detail.Particle.iMaxParticles = 512u;
		BudgetElement.Detail.Particle.SourceScale.fCount = 16.f;
		std::string ValidationError;
		if (!Client::CEffectDocumentCodec::Validate(
				BudgetDocument, ValidationError))
		{
			strOutError =
				"source playback tuning rejected the exact 8192-particle boundary: " +
				ValidationError;
			return false;
		}
		BudgetElement.Detail.Particle.iMaxParticles = 513u;
		if (Client::CEffectDocumentCodec::Validate(
				BudgetDocument, ValidationError) ||
			ValidationError !=
				"Effect Document exceeds the particle, trail, or after-image budget.")
		{
			strOutError =
				"source playback Count x did not close the scaled document budget";
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC EventDocument;
		if (!Client::CEffectDocumentCodec::Load(
				AuthoredRoot / L"effect.lancemaster.skill.34550.unified.effect.json",
				EventDocument, strOutError))
		{
			strOutError = "source playback event fixture failed to load: " +
				strOutError;
			return false;
		}
		const auto Generator = std::find_if(EventDocument.Elements.begin(),
			EventDocument.Elements.end(), [](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId ==
					"authored.source-particle.54c1cad976d33db94d22d98a";
			});
		if (Generator == EventDocument.Elements.end())
		{
			strOutError = "source playback event generator fixture is missing";
			return false;
		}
		Generator->Detail.Particle.SourceScale.fCount = 16.f;
		Generator->Detail.Particle.iMaxParticles = 128u;
		ValidationError.clear();
		if (!Client::CEffectDocumentCodec::Validate(
				EventDocument, ValidationError))
		{
			strOutError =
				"source playback tuning rejected the exact 4096-event boundary: " +
				ValidationError;
			return false;
		}
		Generator->Detail.Particle.iMaxParticles = 129u;
		if (Client::CEffectDocumentCodec::Validate(
				EventDocument, ValidationError) ||
			ValidationError !=
				"Portable authored particle event queue has an unbounded per-step upper limit.")
		{
			strOutError =
				"source playback Count x did not close the scaled event queue bound";
			return false;
		}

		BudgetElement.Detail.Particle.iMaxParticles = 3u;
		BudgetElement.Detail.Particle.SourceScale.fCount = 1.f;
		BudgetElement.Detail.Timing.fStartDelaySeconds = 0.25f;
		BudgetElement.Detail.Timing.fLifeTimeSeconds = 0.75f;
		BudgetElement.Detail.Timing.fAfterImageSeconds = 0.f;
		BudgetElement.Detail.Particle.vLifeTimeSeconds = { 0.5f, 0.5f };
		BudgetElement.Detail.Particle.SourceScale.fLifeTime = 4.f;
		BudgetElement.SourceRecipe.fEmitterDelaySeconds = 0.f;
		BudgetElement.SourceRecipe.fEmitterDurationSeconds = 0.f;
		BudgetElement.SourceRecipe.iEmitterLoopCount = 0u;
		std::shared_ptr<const Client::CEffectPlayback::PREPARED_RESOURCES>
			Prepared;
		if (!Client::CEffectPlayback::Prepare_DocumentResources(
				BudgetDocument, Prepared, strOutError))
		{
			strOutError = "source playback duration resources failed: " +
				strOutError;
			return false;
		}
		Client::CEffectPlayback Playback;
		if (!Playback.Stage_PrevalidatedDocument(
				BudgetDocument, Prepared, strOutError) ||
			std::abs(Playback.Get_DurationSeconds() - 3.f) > 1.0e-4f)
		{
			strOutError =
				"source playback Life x did not extend the declared preview tail";
			return false;
		}

		strOutError.clear();
		return true;
	}

	bool_t Validate_ArtistParticleMasterHdrEmissiveContract(
		const std::filesystem::path& RepositoryRoot,
		std::string& strOutError)
	{
		const std::filesystem::path AuthoredRoot = RepositoryRoot / L"Data" /
			L"Effects" / L"Authored";
		for (const ARTIST_PARTICLE_MASTER_STAGE_DESC& Stage :
			ARTIST_PARTICLE_MASTER_STAGES)
		{
			Client::EFFECT_DOCUMENT_DESC Document;
			const std::filesystem::path DocumentPath = AuthoredRoot /
				(std::string(Stage.strEffectAssetId) + ".effect.json");
			if (!Client::CEffectDocumentCodec::Load(
					DocumentPath, Document, strOutError))
			{
				strOutError = "Artist BA" + std::to_string(Stage.iStageNumber) +
					" ParticleMaster HDR fixture failed to load: " + strOutError;
				return false;
			}
			const auto Element = std::find_if(Document.Elements.begin(),
				Document.Elements.end(), [&Stage](
					const Client::EFFECT_ELEMENT_DESC& Candidate)
				{
					return Candidate.strElementId == Stage.strElementId;
				});
			if (Document.strEffectAssetId != Stage.strEffectAssetId ||
				Element == Document.Elements.end() ||
				Client::Resolve_EffectStrictTypedSourceProfile(
					Element->Material.strSourceMaterialPath,
					Element->Material.SourceMaterial) !=
					Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::PARTICLE_MASTER_01)
			{
				strOutError = "Artist BA" + std::to_string(Stage.iStageNumber) +
					" ParticleMaster exact source identity drifted";
				return false;
			}

			const Client::EFFECT_NAMED_TEXTURE_DESC* const pAlphaGroupB =
				Client::Find_EffectUniqueNamedTexture(
					Element->Material.SourceMaterial, "11.map_b");
			const Client::EFFECT_NAMED_TEXTURE_DESC* const pEmissionE =
				Client::Find_EffectUniqueNamedTexture(
					Element->Material.SourceMaterial, "02.map_e");
			const auto GenericEmissive = std::find_if(
				Element->ResourceBindings.begin(), Element->ResourceBindings.end(),
				[](const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == "emissive";
				});
			if (nullptr == pAlphaGroupB || nullptr == pEmissionE ||
				GenericEmissive == Element->ResourceBindings.end() ||
				GenericEmissive->strAssetId != pAlphaGroupB->strAssetId ||
				GenericEmissive->strAssetId == pEmissionE->strAssetId)
			{
				strOutError = "Artist BA" + std::to_string(Stage.iStageNumber) +
					" ParticleMaster generic Emissive was confused with named E emission";
				return false;
			}

		}

		const std::filesystem::path ShaderPath = RepositoryRoot / L"Client" /
			L"Bin" / L"ShaderFiles" / L"Shader_EffectCommon.hlsli";
		std::ifstream ShaderInput(ShaderPath, std::ios::binary);
		const std::string ShaderSource{
			std::istreambuf_iterator<char>(ShaderInput),
			std::istreambuf_iterator<char>()};
		const size_t PreStage = ShaderSource.find(
			"19 == g_SourceMaterialProfile ? 1.f : g_EmissiveIntensity");
		const size_t AuthoredTintCarrier = ShaderSource.find(
			"g_AuthoredColorMultiply + g_ColorOffset");
		const size_t SharedCompression = ShaderSource.find(
			"if ((g_SourceMaterialProfile >= 19 && g_SourceMaterialProfile <= 32)");
		const size_t PostStageCondition = SharedCompression == std::string::npos ?
			std::string::npos : ShaderSource.find(
				"if (19 == g_SourceMaterialProfile)", SharedCompression);
		const size_t PostStage = PostStageCondition == std::string::npos ?
			std::string::npos : ShaderSource.find(
				"output.SceneColor.rgb *= g_EmissiveIntensity;",
				PostStageCondition);
		if (!ShaderInput.is_open() || ShaderInput.bad() ||
			PreStage == std::string::npos ||
			AuthoredTintCarrier == std::string::npos ||
			SharedCompression == std::string::npos || PostStage == std::string::npos ||
			!(PreStage < SharedCompression && SharedCompression < PostStage))
		{
			strOutError =
				"ParticleMaster shader no longer applies authored HDR after compression";
			return false;
		}

		strOutError.clear();
		return true;
	}

	bool_t Validate_ArtistParticleMasterSourceHdrEmissiveContract(
		std::string& strOutError)
	{
		for (const ARTIST_PARTICLE_MASTER_STAGE_DESC& Stage :
			ARTIST_PARTICLE_MASTER_STAGES)
		{
			const std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC> Document =
				Client::CEffectCatalog::Find(std::string(Stage.strEffectAssetId));
			if (nullptr == Document)
			{
				strOutError = "Artist BA" + std::to_string(Stage.iStageNumber) +
					" ParticleMaster source document is unavailable: " +
					Client::CEffectCatalog::Get_Status();
				return false;
			}
			const auto Element = std::find_if(Document->Elements.begin(),
				Document->Elements.end(), [&Stage](
					const Client::EFFECT_ELEMENT_DESC& Candidate)
				{
					return Candidate.strElementId == Stage.strElementId;
				});
			if (Element == Document->Elements.end() ||
				Client::Resolve_EffectStrictTypedSourceProfile(
					Element->Material.strSourceMaterialPath,
					Element->Material.SourceMaterial) !=
					Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::PARTICLE_MASTER_01)
			{
				strOutError = "Artist BA" + std::to_string(Stage.iStageNumber) +
					" ParticleMaster source identity drifted";
				return false;
			}

		}

		strOutError.clear();
		return true;
	}

	bool_t Validate_TransformMotionHoldContract(
		const std::filesystem::path& RepositoryRoot,
		std::string& strOutError)
	{
		const std::filesystem::path AuthoredRoot = RepositoryRoot / L"Data" /
			L"Effects" / L"Authored";
		Client::EFFECT_DOCUMENT_DESC SourceDocument;
		if (!Client::CEffectDocumentCodec::Load(
				AuthoredRoot /
					L"effect.dimensionmaster.skill.2050010.ba1.effect.json",
				SourceDocument, strOutError))
		{
			strOutError = "transform-motion standalone Mesh fixture failed to load: " +
				strOutError;
			return false;
		}
		const auto SourceMesh = std::find_if(SourceDocument.Elements.begin(),
			SourceDocument.Elements.end(),
			[](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				return Element.eKind == Client::EFFECT_ELEMENT_KIND::MESH;
			});
		if (SourceMesh == SourceDocument.Elements.end())
		{
			strOutError = "transform-motion standalone Mesh fixture is missing";
			return false;
		}

		const std::string LegacyJson =
			Client::CEffectDocumentCodec::Serialize(SourceDocument);
		Client::EFFECT_DOCUMENT_DESC LegacyRoundTrip;
		if (LegacyJson.find("transformMotionDurationSeconds") !=
				std::string::npos ||
			!Client::CEffectDocumentCodec::Parse(
				LegacyJson, LegacyRoundTrip, strOutError) ||
			std::any_of(LegacyRoundTrip.Elements.begin(),
				LegacyRoundTrip.Elements.end(),
				[](const Client::EFFECT_ELEMENT_DESC& Element)
				{
					return Element.Detail.Timing.
						fTransformMotionDurationSeconds != 0.f;
				}))
		{
			strOutError =
				"legacy timing omission did not round-trip as the zero fallback";
			return false;
		}

		const std::string SourceMeshId = SourceMesh->strElementId;
		Client::EFFECT_ELEMENT_DESC& TunedSourceMesh = *std::find_if(
			SourceDocument.Elements.begin(), SourceDocument.Elements.end(),
			[&SourceMeshId](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == SourceMeshId;
			});
		TunedSourceMesh.Detail.Timing.fLifeTimeSeconds = 3.f;
		TunedSourceMesh.Detail.Timing.fTransformMotionDurationSeconds = 1.f;
		std::string ValidationError;
		if (!Client::CEffectDocumentCodec::Validate(
				SourceDocument, ValidationError))
		{
			strOutError = "valid transform-motion timing was rejected: " +
				ValidationError;
			return false;
		}
		Client::EFFECT_DOCUMENT_DESC InvalidDuration = SourceDocument;
		auto InvalidMesh = std::find_if(InvalidDuration.Elements.begin(),
			InvalidDuration.Elements.end(),
			[&SourceMeshId](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == SourceMeshId;
			});
		InvalidMesh->Detail.Timing.fTransformMotionDurationSeconds = 3.001f;
		if (Client::CEffectDocumentCodec::Validate(
				InvalidDuration, ValidationError))
		{
			strOutError = "transform motion longer than Life was accepted";
			return false;
		}
		InvalidDuration = SourceDocument;
		InvalidMesh = std::find_if(InvalidDuration.Elements.begin(),
			InvalidDuration.Elements.end(),
			[&SourceMeshId](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == SourceMeshId;
			});
		InvalidMesh->Detail.Timing.fTransformMotionDurationSeconds = -0.001f;
		if (Client::CEffectDocumentCodec::Validate(
				InvalidDuration, ValidationError))
		{
			strOutError = "negative transform motion was accepted";
			return false;
		}
		Client::EFFECT_DOCUMENT_DESC UnsupportedCarrier = SourceDocument;
		const auto Sprite = std::find_if(UnsupportedCarrier.Elements.begin(),
			UnsupportedCarrier.Elements.end(),
			[](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				return Element.eKind == Client::EFFECT_ELEMENT_KIND::SPRITE;
			});
		if (Sprite == UnsupportedCarrier.Elements.end())
		{
			strOutError = "transform-motion Sprite rejection fixture is missing";
			return false;
		}
		Sprite->Detail.Timing.fTransformMotionDurationSeconds =
			(std::min)(Sprite->Detail.Timing.fLifeTimeSeconds, 0.1f);
		if (Client::CEffectDocumentCodec::Validate(
				UnsupportedCarrier, ValidationError))
		{
			strOutError =
				"transform motion was accepted on an unsupported Sprite carrier";
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC MeshDocument;
		if (!Client::CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
				SourceDocument, SourceMeshId,
				"effect.contract.transform-motion.mesh", MeshDocument,
				strOutError) || MeshDocument.Elements.size() != 1u ||
			MeshDocument.Elements.front().Detail.Timing.
				fTransformMotionDurationSeconds != 1.f)
		{
			strOutError =
				"generic authored Element copy did not preserve transform motion: " +
				strOutError;
			return false;
		}
		const std::string MotionJson =
			Client::CEffectDocumentCodec::Serialize(MeshDocument);
		Client::EFFECT_DOCUMENT_DESC MotionRoundTrip;
		if (MotionJson.find("transformMotionDurationSeconds") ==
				std::string::npos ||
			!Client::CEffectDocumentCodec::Parse(
				MotionJson, MotionRoundTrip, strOutError) ||
			MotionRoundTrip.Elements.size() != 1u ||
			MotionRoundTrip.Elements.front().Detail.Timing.
				fTransformMotionDurationSeconds != 1.f)
		{
			strOutError =
				"explicit transform motion did not survive codec round-trip";
			return false;
		}

		Client::EFFECT_ELEMENT_DESC& Mesh = MeshDocument.Elements.front();
		Mesh.Detail.Timing.fStartDelaySeconds = 0.f;
		Mesh.Detail.Timing.fLifeTimeSeconds = 3.f;
		Mesh.Detail.Timing.fTransformMotionDurationSeconds = 1.f;
		Mesh.Detail.Transform.vPosition = { 0.f, 0.f, 0.f };
		Mesh.Detail.Transform.vVelocityPerSecond = { 2.f, 1.f, -1.f };
		Mesh.Detail.Transform.vRotationDegrees = { 10.f, 20.f, 30.f };
		Mesh.Detail.Transform.vRevolutionDegreesPerSecond =
			{ 45.f, 90.f, 135.f };
		Mesh.Detail.Transform.vScale = { 1.f, 1.f, 1.f };
		Mesh.Detail.LinearLerp.bPosition = true;
		Mesh.Detail.LinearLerp.vEndPosition = { 3.f, 4.f, 5.f };
		Mesh.Detail.LinearLerp.bRotation = true;
		Mesh.Detail.LinearLerp.vEndRotationDegrees = { 40.f, 50.f, 60.f };
		Mesh.Detail.LinearLerp.bScale = true;
		Mesh.Detail.LinearLerp.vEndScale = { 2.f, 3.f, 4.f };
		Mesh.Detail.LinearLerp.bVelocity = true;
		Mesh.Detail.LinearLerp.vEndVelocityPerSecond = { 4.f, 3.f, 1.f };
		Mesh.Detail.Color.vColorMultiply = { 1.f, 1.f, 1.f, 1.f };
		Mesh.Detail.LinearLerp.bColorMultiply = true;
		Mesh.Detail.LinearLerp.vEndColorMultiply = { 1.f, 1.f, 1.f, 0.f };
		std::shared_ptr<const Client::CEffectPlayback::PREPARED_RESOURCES>
			PreparedMesh;
		if (!Client::CEffectPlayback::Prepare_DocumentResources(
				MeshDocument, PreparedMesh, strOutError))
		{
			strOutError = "transform-motion Mesh resources failed: " + strOutError;
			return false;
		}
		float4x4_t Identity{};
		XMStoreFloat4x4(&Identity, XMMatrixIdentity());
		Client::CEffectPlayback MeshPlayback;
		if (!MeshPlayback.Stage_PrevalidatedDocument(
				MeshDocument, PreparedMesh, strOutError))
		{
			strOutError = "transform-motion Mesh staging failed: " + strOutError;
			return false;
		}
		MeshPlayback.Seek(1.f, Identity);
		if (MeshPlayback.Get_Frame().Elements.size() != 1u)
		{
			strOutError = "transform-motion Mesh end sample is missing";
			return false;
		}
		const float4x4_t MotionEndWorld =
			MeshPlayback.Get_Frame().Elements.front().World;
		const f32_t fMotionEndAlpha =
			MeshPlayback.Get_Frame().Elements.front().Color.vColorMultiply.w;
		MeshPlayback.Seek(2.f, Identity);
		if (MeshPlayback.Get_Frame().Elements.size() != 1u)
		{
			strOutError = "transform-motion Mesh hold sample is missing";
			return false;
		}
		const Client::EFFECT_EVALUATED_ELEMENT& HoldSample =
			MeshPlayback.Get_Frame().Elements.front();
		for (size_t iRow = 0u; iRow < 4u; ++iRow)
		{
			for (size_t iColumn = 0u; iColumn < 4u; ++iColumn)
			{
				if (std::abs(MotionEndWorld.m[iRow][iColumn] -
						HoldSample.World.m[iRow][iColumn]) > 1.0e-4f)
				{
					strOutError =
						"Element root transform continued moving during hold";
					return false;
				}
			}
		}
		if (!(HoldSample.fNormalizedLife > 0.66f &&
			HoldSample.fNormalizedLife < 0.67f &&
			HoldSample.Color.vColorMultiply.w < fMotionEndAlpha - 0.3f))
		{
			strOutError =
				"Element Life/color clock did not continue during transform hold";
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC ParticleSource;
		if (!Client::CEffectDocumentCodec::Load(
				AuthoredRoot /
					L"effect.lancemaster.skill.34580.ba1.unified.effect.json",
				ParticleSource, strOutError) || ParticleSource.Elements.size() != 1u)
		{
			strOutError = "transform-motion Mesh Particle fixture failed: " +
				strOutError;
			return false;
		}
		Client::EFFECT_DOCUMENT_DESC ParticleDocument;
		if (!Client::CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
				ParticleSource, ParticleSource.Elements.front().strElementId,
				"effect.contract.transform-motion.mesh-particle",
				ParticleDocument, strOutError))
		{
			strOutError = "transform-motion Mesh Particle copy failed: " +
				strOutError;
			return false;
		}
		Client::EFFECT_ELEMENT_DESC& Particle = ParticleDocument.Elements.front();
		if (!ParticleSource.Elements.front().SourceRecipe.bEnabled ||
			ParticleSource.Elements.front().Detail.Particle.
				fSpawnRatePerSecond != 0.f ||
			0u != ParticleSource.Elements.front().Detail.Particle.iBurstCount ||
			Particle.SourceRecipe.bEnabled ||
			Particle.Detail.Particle.fSpawnRatePerSecond != 0.f ||
			1u != Particle.Detail.Particle.iBurstCount ||
			Particle.Detail.Particle.iMaxParticles < 1u)
		{
			strOutError =
				"generic Mesh Particle copy lost its bounded direct emission authority";
			return false;
		}
		const Client::EFFECT_PARTICLE_DESC& SourceParticleDetail =
			ParticleSource.Elements.front().Detail.Particle;
		if (std::abs(Particle.Detail.Mesh.fModelPreScale - 0.01f) > 1.0e-6f ||
			std::abs(SourceParticleDetail.vStartSize.x - 0.09f) > 1.0e-5f ||
			std::abs(SourceParticleDetail.vStartSize.y - 0.06f) > 1.0e-5f ||
			std::abs(Particle.Detail.Particle.vStartSize.x - 9.f) > 1.0e-4f ||
			std::abs(Particle.Detail.Particle.vStartSize.y - 6.f) > 1.0e-4f)
		{
			strOutError =
				"generic Mesh Particle copy retained a geometry-scaled direct StartSize";
			return false;
		}
		Client::EFFECT_DOCUMENT_DESC AlreadyDimensionlessSource = ParticleSource;
		AlreadyDimensionlessSource.Elements.front().Detail.Particle.vStartSize =
			{ 9.f, 6.f };
		AlreadyDimensionlessSource.Elements.front().Detail.Particle.vEndSize =
			{ 4.5f, 3.f };
		Client::EFFECT_DOCUMENT_DESC AlreadyDimensionlessCopy;
		if (!Client::CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
				AlreadyDimensionlessSource,
				AlreadyDimensionlessSource.Elements.front().strElementId,
				"effect.contract.transform-motion.mesh-particle-dimensionless",
				AlreadyDimensionlessCopy, strOutError) ||
			AlreadyDimensionlessCopy.Elements.size() != 1u ||
			std::abs(AlreadyDimensionlessCopy.Elements.front().Detail.Particle.
				vStartSize.x - 9.f) > 1.0e-4f ||
			std::abs(AlreadyDimensionlessCopy.Elements.front().Detail.Particle.
				vStartSize.y - 6.f) > 1.0e-4f ||
			std::abs(AlreadyDimensionlessCopy.Elements.front().Detail.Particle.
				vEndSize.x - 4.5f) > 1.0e-4f ||
			std::abs(AlreadyDimensionlessCopy.Elements.front().Detail.Particle.
				vEndSize.y - 3.f) > 1.0e-4f)
		{
			strOutError =
				"generic Mesh Particle copy rescaled an already-dimensionless StartSize";
			return false;
		}
		const auto VerifyGenericMeshParticleSizeCopy = [&AuthoredRoot, &strOutError](
			const std::filesystem::path& RelativePath,
			const std::string_view ElementId,
			const std::string_view TargetEffectId,
			const float2_t ExpectedStart,
			const float2_t ExpectedEnd)
		{
			Client::EFFECT_DOCUMENT_DESC Source;
			if (!Client::CEffectDocumentCodec::Load(
					AuthoredRoot / RelativePath, Source, strOutError))
			{
				strOutError = "generic Mesh Particle size fixture failed to load: " +
					strOutError;
				return false;
			}
			Client::EFFECT_DOCUMENT_DESC Copy;
			if (!Client::CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
					Source, ElementId, TargetEffectId, Copy, strOutError) ||
				Copy.Elements.size() != 1u)
			{
				strOutError = "generic Mesh Particle size fixture failed to copy: " +
					strOutError;
				return false;
			}
			const auto MatchesExpected = [&ExpectedStart, &ExpectedEnd](
				const Client::EFFECT_ELEMENT_DESC& Element)
			{
				const Client::EFFECT_PARTICLE_DESC& Detail = Element.Detail.Particle;
				return !Element.SourceRecipe.bEnabled &&
					std::abs(Detail.vStartSize.x - ExpectedStart.x) <= 1.0e-4f &&
					std::abs(Detail.vStartSize.y - ExpectedStart.y) <= 1.0e-4f &&
					std::abs(Detail.vEndSize.x - ExpectedEnd.x) <= 1.0e-4f &&
					std::abs(Detail.vEndSize.y - ExpectedEnd.y) <= 1.0e-4f &&
					std::abs(Detail.SourceScale.fSize - 1.f) <= 1.0e-6f;
			};
			if (!MatchesExpected(Copy.Elements.front()))
			{
				strOutError =
					"generic Mesh Particle size normalization or SourceScale bake drifted";
				return false;
			}
			Client::EFFECT_DOCUMENT_DESC RoundTrip;
			if (!Client::CEffectDocumentCodec::Parse(
					Client::CEffectDocumentCodec::Serialize(Copy),
					RoundTrip, strOutError) || RoundTrip.Elements.size() != 1u ||
				!MatchesExpected(RoundTrip.Elements.front()))
			{
				strOutError =
					"generic Mesh Particle normalized size did not survive codec round-trip";
				return false;
			}
			return true;
		};
		if (!VerifyGenericMeshParticleSizeCopy(
				L"effect.lancemaster.skill.34010.ba1.unified.effect.json",
				"authored.source-particle.ce43b71ae7ca3379dfe8529d",
				"effect.contract.saved-element.lance-ba1",
				{ 2.97f, 2.97f }, { 3.861f, 3.861f }) ||
			!VerifyGenericMeshParticleSizeCopy(
				L"effect.lancemaster.skill.34010.ba2.unified.effect.json",
				"authored.source-particle.7c74ff64061154626dddbca2",
				"effect.contract.saved-element.lance-ba2",
				{ 2.7f, 2.7f }, { 3.51f, 3.51f }) ||
			!VerifyGenericMeshParticleSizeCopy(
				L"effect.artist.skill.31470.unified.effect.json",
				"mesh.8165115d36cd1a8a",
				"effect.contract.saved-element.artist-dimensionless",
				{ 3.8f, 3.8f }, { 0.f, 0.f }) ||
			!VerifyGenericMeshParticleSizeCopy(
				L"effect.artist.skill.31470.unified.effect.json",
				"mesh.cc04feee8a36940b",
				"effect.contract.saved-element.artist-source-scale",
				{ 1.6f, 1.6f }, { 2.0288f, 2.2976f }))
		{
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC SavedParticleSource;
		constexpr std::string_view SavedParticleElementId =
			"authored.source-particle.ce43b71ae7ca3379dfe8529d";
		if (!Client::CEffectDocumentCodec::Load(
				AuthoredRoot /
					L"effect.lancemaster.skill.34010.ba1.unified.effect.json",
				SavedParticleSource, strOutError))
		{
			strOutError = "saved Mesh Particle portable fixture failed to load: " +
				strOutError;
			return false;
		}
		const auto SavedParticleSourceElement = std::find_if(
			SavedParticleSource.Elements.begin(), SavedParticleSource.Elements.end(),
			[](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == SavedParticleElementId;
			});
		if (SavedParticleSourceElement == SavedParticleSource.Elements.end())
		{
			strOutError =
				"saved Mesh Particle portable fixture lost its source Element";
			return false;
		}
		Client::EFFECT_DOCUMENT_DESC SavedParticleFallbackCopy;
		if (!Client::CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
				SavedParticleSource, SavedParticleElementId,
				"effect.contract.saved-element.lance-ba1-fallback",
				SavedParticleFallbackCopy, strOutError) ||
			SavedParticleFallbackCopy.Elements.size() != 1u)
		{
			strOutError = "saved Mesh Particle generic stage failed: " +
				strOutError;
			return false;
		}
		const Client::EFFECT_ELEMENT_DESC& SavedParticleFallback =
			SavedParticleFallbackCopy.Elements.front();
		if (SavedParticleFallback.SourceRecipe.bEnabled ||
			std::abs(SavedParticleFallback.Detail.Particle.SourceScale.fSize - 1.f) >
				1.0e-6f ||
			std::abs(SavedParticleFallback.Detail.Particle.vStartSize.x - 2.97f) >
				1.0e-4f ||
			std::abs(SavedParticleFallback.Detail.Particle.vEndSize.x - 3.861f) >
				1.0e-4f ||
			SavedParticleFallback.Detail.Particle.iBurstCount != 1u)
		{
			strOutError =
				"saved Mesh Particle generic stage did not expose its expected direct fallback";
			return false;
		}
		Client::EFFECT_DOCUMENT_DESC SavedParticleCopy;
		if (!Client::CEffectDocumentCodec::
			Build_PortableAuthoredElementStartingCopy(
				SavedParticleSource, SavedParticleElementId,
				"effect.lancemaster.contract.saved-element.ba1-portable",
				SavedParticleCopy, strOutError) ||
			SavedParticleCopy.Elements.size() != 1u)
		{
			strOutError = "saved Mesh Particle portable copy failed: " +
				strOutError;
			return false;
		}
		const Client::EFFECT_ELEMENT_DESC& SavedParticle =
			SavedParticleCopy.Elements.front();
		static constexpr std::array<std::string_view, 13u>
			EXPECTED_SAVED_PARTICLE_MODULE_CLASSES = {{
				"particlemodulerequired",
				"particlemodulelifetime",
				"particlemodulesize",
				"particlemodulecolor",
				"particlemodulecolorscaleoverlife",
				"particlemoduleparameterdynamic",
				"particlemodulemeshrotation",
				"particlemodulesizemultiplylife",
				"particlemodulelocation",
				"particlemodulemeshrotationrate",
				"particlemodulemeshrotationratemultiplylife",
				"particlemoduletypedatamesh",
				"particlemodulespawn"
			}};
		const auto MatchesPortableSavedParticle =
			[&SavedParticleSourceElement](
				const Client::EFFECT_ELEMENT_DESC& Element)
		{
			const Client::EFFECT_CASCADE_RECIPE_DESC& Recipe =
				Element.SourceRecipe;
			const Client::EFFECT_PARTICLE_SOURCE_SCALE_DESC& SourceScale =
				Element.Detail.Particle.SourceScale;
			if (!Recipe.bEnabled || Recipe.strRendererShape != "mesh" ||
				Recipe.fEmitterDelaySeconds != 0.f ||
				std::abs(Recipe.fEmitterDurationSeconds - 5.f) > 1.0e-6f ||
				Recipe.iEmitterLoopCount != 1u || Recipe.Bursts.size() != 1u ||
				Recipe.Bursts.front().fTimeSeconds != 0.f ||
				Recipe.Bursts.front().iCountMinimum != 1u ||
				Recipe.Bursts.front().iCountMaximum != 1u ||
				Recipe.Modules.size() !=
					EXPECTED_SAVED_PARTICLE_MODULE_CLASSES.size() ||
				std::abs(SourceScale.fCount - 1.58f) > 1.0e-4f ||
				std::abs(SourceScale.fSize - 1.1f) > 1.0e-4f ||
				std::abs(Element.Detail.Particle.vStartSize.x - 0.027f) >
					1.0e-6f ||
				std::abs(Element.Detail.Particle.vEndSize.x - 0.0351f) >
					1.0e-6f ||
				std::abs(Element.Detail.Mesh.fModelPreScale - 0.01f) >
					1.0e-7f ||
				Element.Detail.Mesh.vSourceTypeDataRotationDegrees.x !=
					SavedParticleSourceElement->Detail.Mesh.
						vSourceTypeDataRotationDegrees.x ||
				Element.strSourceNode !=
					"authored-copy:" + SavedParticleSourceElement->strElementId ||
				Element.Material.strSourceMaterialPath !=
					SavedParticleSourceElement->Material.strSourceMaterialPath ||
				Element.Material.SourceMaterial.strProfileId !=
					SavedParticleSourceElement->Material.SourceMaterial.strProfileId ||
				Element.Material.SourceMaterial.strRuntimeShaderProfileId !=
					SavedParticleSourceElement->Material.SourceMaterial.
						strRuntimeShaderProfileId ||
				Element.ResourceBindings.size() !=
					SavedParticleSourceElement->ResourceBindings.size())
			{
				return false;
			}
			for (size_t iModule = 0u;
				iModule < EXPECTED_SAVED_PARTICLE_MODULE_CLASSES.size();
				++iModule)
			{
				if (Recipe.Modules[iModule].strClassName !=
					EXPECTED_SAVED_PARTICLE_MODULE_CLASSES[iModule])
				{
					return false;
				}
			}
			for (size_t iBinding = 0u;
				iBinding < Element.ResourceBindings.size(); ++iBinding)
			{
				if (Element.ResourceBindings[iBinding].strSlotId !=
						SavedParticleSourceElement->ResourceBindings[iBinding].strSlotId ||
					Element.ResourceBindings[iBinding].strAssetId !=
						SavedParticleSourceElement->ResourceBindings[iBinding].strAssetId)
				{
					return false;
				}
			}
			return true;
		};
		if (!MatchesPortableSavedParticle(SavedParticle))
		{
			strOutError =
				"saved Mesh Particle portable carrier lost shader or module identity";
			return false;
		}
		SavedParticleCopy.Elements.front().strElementId =
			"authored.copy.contract.saved-mesh-particle.1";
		Client::EFFECT_DOCUMENT_DESC SavedParticleRoundTrip;
		if (!Client::CEffectDocumentCodec::Parse(
				Client::CEffectDocumentCodec::Serialize(SavedParticleCopy),
				SavedParticleRoundTrip, strOutError) ||
			SavedParticleRoundTrip.Elements.size() != 1u ||
			!MatchesPortableSavedParticle(SavedParticleRoundTrip.Elements.front()))
		{
			strOutError =
				"saved Mesh Particle portable carrier did not survive codec round-trip";
			return false;
		}

		const auto VerifyPortableSavedElement =
			[&AuthoredRoot, &strOutError](
				const std::filesystem::path& RelativePath,
				const std::string_view ElementId,
				const std::string_view TargetEffectId,
				const Client::EFFECT_ELEMENT_KIND ExpectedKind,
				const std::string_view ExpectedRecipeShape,
				const bool_t bExpectedAttachment,
				const bool_t bExpectedFollow,
				const std::string_view ExpectedSourceNode)
		{
			Client::EFFECT_DOCUMENT_DESC SourceDocument;
			if (!Client::CEffectDocumentCodec::Load(
					AuthoredRoot / RelativePath, SourceDocument, strOutError))
			{
				strOutError = "portable Saved Element family fixture failed to load: " +
					RelativePath.generic_string() + ": " + strOutError;
				return false;
			}
			const auto Source = std::find_if(SourceDocument.Elements.begin(),
				SourceDocument.Elements.end(),
				[ElementId](const Client::EFFECT_ELEMENT_DESC& Element)
				{
					return Element.strElementId == ElementId;
				});
			const bool_t bExpectedRecipe = !ExpectedRecipeShape.empty();
			if (Source == SourceDocument.Elements.end() ||
				std::find_if(std::next(Source), SourceDocument.Elements.end(),
					[ElementId](const Client::EFFECT_ELEMENT_DESC& Element)
					{
						return Element.strElementId == ElementId;
					}) != SourceDocument.Elements.end() ||
				Source->eKind != ExpectedKind ||
				Source->Renderer.eType != Client::EFFECT_RENDERER_TYPE::END ||
				Source->Renderer.eSourceSpace != Client::EFFECT_SOURCE_SPACE::END ||
				Source->TransformInheritance.bEnabled ||
				Source->SourcePresentation.bEnabled ||
				Source->SourceRecipe.bEnabled != bExpectedRecipe ||
				Source->SourceRecipe.strRendererShape != ExpectedRecipeShape ||
				Source->ActionCueAttachment.bEnabled != bExpectedAttachment ||
				Source->ActionCueAttachment.bFollow != bExpectedFollow ||
				!Client::Is_EffectAuthoringExecutionTarget(
					Source->Material.Execution))
			{
				strOutError =
					"portable Saved Element source Family or ownership gate drifted: " +
					std::string(ElementId);
				return false;
			}

			Client::EFFECT_DOCUMENT_DESC PortableCopy;
			if (!Client::CEffectDocumentCodec::
					Build_PortableAuthoredElementStartingCopy(
						SourceDocument, ElementId, TargetEffectId,
						PortableCopy, strOutError) ||
				PortableCopy.Elements.size() != 1u)
			{
				strOutError = "portable Saved Element family copy failed: " +
					std::string(ElementId) + ": " + strOutError;
				return false;
			}
			const Client::EFFECT_ELEMENT_DESC& Portable =
				PortableCopy.Elements.front();
			if (Portable.strElementId != ElementId ||
				Portable.strGroupId != "manual.authoring" ||
				Portable.strSourceNode != ExpectedSourceNode ||
				Portable.eKind != ExpectedKind ||
				Portable.Renderer.eType != Client::EFFECT_RENDERER_TYPE::END ||
				Portable.Renderer.eSourceSpace != Client::EFFECT_SOURCE_SPACE::END ||
				Portable.TransformInheritance.bEnabled ||
				Portable.SourcePresentation.bEnabled ||
				Portable.SourceRecipe.bEnabled != bExpectedRecipe ||
				Portable.SourceRecipe.strRendererShape != ExpectedRecipeShape ||
				Portable.ActionCueAttachment.bEnabled != bExpectedAttachment ||
				Portable.ActionCueAttachment.bFollow != bExpectedFollow)
			{
				strOutError =
					"portable Saved Element copied Family or provenance drifted: " +
					std::string(ElementId);
				return false;
			}

			/* Canonical whole-Element comparison intentionally normalizes only
			   portable provenance/ownership fields. This covers the complete Detail,
			   Material, ResourceBindings, AuthoringOverrides, attachment, and recipe
			   payload without a partial field list that can miss future fields. */
			Client::EFFECT_DOCUMENT_DESC Expected = PortableCopy;
			Expected.Elements.front() = *Source;
			Client::EFFECT_ELEMENT_DESC& ExpectedElement =
				Expected.Elements.front();
			ExpectedElement.strGroupId = Portable.strGroupId;
			ExpectedElement.strSourceNode = std::string(ExpectedSourceNode);
			ExpectedElement.Renderer = {};
			ExpectedElement.TransformInheritance = {};
			ExpectedElement.SourcePresentation = {};
			const std::string RuntimeElementId =
				"authored.copy.contract.saved-element.1";
			PortableCopy.Elements.front().strElementId = RuntimeElementId;
			Expected.Elements.front().strElementId = RuntimeElementId;
			const std::string PortableJson =
				Client::CEffectDocumentCodec::Serialize(PortableCopy);
			if (Client::CEffectDocumentCodec::Serialize(Expected) != PortableJson)
			{
				strOutError =
					"portable Saved Element lost an editable source field: " +
					std::string(ElementId);
				return false;
			}

			Client::EFFECT_DOCUMENT_DESC RoundTrip;
			if (!Client::CEffectDocumentCodec::Parse(
					PortableJson, RoundTrip, strOutError) ||
				!Client::CEffectDocumentCodec::Validate(RoundTrip, strOutError) ||
				RoundTrip.Elements.size() != 1u ||
				Client::CEffectDocumentCodec::Serialize(RoundTrip) != PortableJson)
			{
				strOutError =
					"portable Saved Element family did not survive codec round-trip: " +
					std::string(ElementId) + ": " + strOutError;
				return false;
			}
			return true;
		};

		if (!VerifyPortableSavedElement(
				L"effect.artist.skill.31460.v1.unified.effect.json",
				"authored.source-particle.cb346af47371feedccf9b652",
				"effect.artist.contract.saved-element.standard-color-sprite",
				Client::EFFECT_ELEMENT_KIND::PARTICLE, "sprite", false, false,
				"authored-copy:authored.source-particle.cb346af47371feedccf9b652") ||
			!VerifyPortableSavedElement(
				L"effect.artist.skill.31200.unified.effect.json",
				"authored.source-decal.45800d0c0054acd91f11cfb4",
				"effect.artist.contract.saved-element.recipe-decal",
				Client::EFFECT_ELEMENT_KIND::DECAL, "decal", true, false,
				"authored-copy:authored.source-decal.45800d0c0054acd91f11cfb4") ||
			!VerifyPortableSavedElement(
				L"effect.artist.skill.31200.unified.effect.json",
				"authored.copy.authored.source-decal.45800d0c0054acd91f11cfb4.1",
				"effect.artist.contract.saved-element.recipe-decal-chain",
				Client::EFFECT_ELEMENT_KIND::DECAL, "decal", true, false,
				"authored-copy:authored.source-decal.45800d0c0054acd91f11cfb4") ||
			!VerifyPortableSavedElement(
				L"effect.test.winters.effect.json", "sprite_1",
				"effect.test.contract.saved-element.sprite",
				Client::EFFECT_ELEMENT_KIND::SPRITE, {}, false, false,
				"authored-copy:sprite_1") ||
			!VerifyPortableSavedElement(
				L"effect.artist.skill.31470.unified.effect.json",
				"decal.f3b5c3b63b4a7e34",
				"effect.artist.contract.saved-element.direct-decal",
				Client::EFFECT_ELEMENT_KIND::DECAL, {}, false, false,
				"authored-copy:decal.f3b5c3b63b4a7e34"))
		{
			return false;
		}

		const auto ExpectPortableSavedElementReject = [&strOutError](
			const Client::EFFECT_DOCUMENT_DESC& SourceDocument,
			const std::string_view ElementId,
			const std::string_view ExpectedError)
		{
			Client::EFFECT_DOCUMENT_DESC Rejected;
			std::string Error;
			if (Client::CEffectDocumentCodec::
					Build_PortableAuthoredElementStartingCopy(
						SourceDocument, ElementId,
						"effect.contract.saved-element.expected-reject",
						Rejected, Error) ||
				Error.find(ExpectedError) == std::string::npos)
			{
				strOutError =
					"portable Saved Element fail-close gate drifted for " +
					std::string(ElementId) + ": " + Error;
				return false;
			}
			return true;
		};
		Client::EFFECT_DOCUMENT_DESC InvisibleSavedParticleSource =
			SavedParticleSource;
		auto InvisibleSavedParticle = std::find_if(
			InvisibleSavedParticleSource.Elements.begin(),
			InvisibleSavedParticleSource.Elements.end(),
			[](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == SavedParticleElementId;
			});
		InvisibleSavedParticle->bVisible = false;
		Client::EFFECT_DOCUMENT_DESC FollowSavedParticleSource =
			SavedParticleSource;
		auto FollowSavedParticle = std::find_if(
			FollowSavedParticleSource.Elements.begin(),
			FollowSavedParticleSource.Elements.end(),
			[](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == SavedParticleElementId;
			});
		FollowSavedParticle->ActionCueAttachment.bEnabled = true;
		FollowSavedParticle->ActionCueAttachment.bFollow = true;
		FollowSavedParticle->ActionCueAttachment.strSourceAnchorSlotId =
			"weapon";
		FollowSavedParticle->ActionCueAttachment.strRuntimeAnchorSlotId =
			"weapon";
		FollowSavedParticle->ActionCueAttachment.strRuntimeBoneName =
			"b_weapon_rhand";
		Client::EFFECT_DOCUMENT_DESC HistoryOnlySavedParticleSource =
			SavedParticleSource;
		auto HistoryOnlySavedParticle = std::find_if(
			HistoryOnlySavedParticleSource.Elements.begin(),
			HistoryOnlySavedParticleSource.Elements.end(),
			[](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == SavedParticleElementId;
			});
		HistoryOnlySavedParticle->SourceRecipe.Bursts.clear();
		Client::EFFECT_DOCUMENT_DESC TrailSource;
		if (!Client::CEffectDocumentCodec::Load(
				AuthoredRoot / L"effect.artist.skill.31470.unified.effect.json",
				TrailSource, strOutError) ||
			!ExpectPortableSavedElementReject(
				InvisibleSavedParticleSource, SavedParticleElementId,
				"invisible source Element") ||
			!ExpectPortableSavedElementReject(
				FollowSavedParticleSource, SavedParticleElementId,
				"FOLLOW attachment") ||
			!ExpectPortableSavedElementReject(
				HistoryOnlySavedParticleSource, SavedParticleElementId,
				"no autonomous positive Burst or Rate") ||
			!ExpectPortableSavedElementReject(
				TrailSource, "ribbon.a6fe27caa16b2630",
				"Trail transform history"))
		{
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC DelayedSavedParticleSource =
			SavedParticleSource;
		auto DelayedSavedParticle = std::find_if(
			DelayedSavedParticleSource.Elements.begin(),
			DelayedSavedParticleSource.Elements.end(),
			[](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == SavedParticleElementId;
			});
		DelayedSavedParticle->SourceRecipe.fEmitterDelaySeconds = 0.125f;
		Client::EFFECT_DOCUMENT_DESC DelayedSavedParticleCopy;
		if (!Client::CEffectDocumentCodec::
				Build_PortableAuthoredElementStartingCopy(
					DelayedSavedParticleSource, SavedParticleElementId,
					"effect.lancemaster.contract.saved-element.ba1-delayed",
					DelayedSavedParticleCopy, strOutError) ||
			DelayedSavedParticleCopy.Elements.size() != 1u ||
			std::abs(DelayedSavedParticleCopy.Elements.front().SourceRecipe.
				fEmitterDelaySeconds - 0.125f) > 1.0e-6f ||
			DelayedSavedParticleCopy.Elements.front().Detail.Timing.
				fStartDelaySeconds !=
					SavedParticleSourceElement->Detail.Timing.fStartDelaySeconds)
		{
			strOutError =
				"saved Particle portable copy did not preserve independent Detail and emitter delays";
			return false;
		}
		Particle.Detail.Timing.fStartDelaySeconds = 0.f;
		Particle.Detail.Timing.fLifeTimeSeconds = 3.f;
		Particle.Detail.Timing.fTransformMotionDurationSeconds = 1.f;
		Particle.Detail.Transform.vPosition = {};
		Particle.Detail.Transform.vVelocityPerSecond = { 2.f, 0.f, 0.f };
		Particle.Detail.LinearLerp = {};
		Particle.Detail.Particle.fSpawnRatePerSecond = 0.f;
		Particle.Detail.Particle.vLifeTimeSeconds = { 3.f, 3.f };
		Particle.Detail.Particle.vInitialPositionMin = {};
		Particle.Detail.Particle.vInitialPositionMax = {};
		Particle.Detail.Particle.vInitialVelocityMin = {};
		Particle.Detail.Particle.vInitialVelocityMax = {};
		Particle.Detail.Particle.vAcceleration = {};
		Particle.Detail.Particle.vStartSize = { 1.f, 1.f };
		Particle.Detail.Particle.vEndSize = { 1.f, 1.f };
		Particle.Detail.Particle.bLocalSpace = true;
		std::shared_ptr<const Client::CEffectPlayback::PREPARED_RESOURCES>
			PreparedParticle;
		if (!Client::CEffectPlayback::Prepare_DocumentResources(
				ParticleDocument, PreparedParticle, strOutError))
		{
			strOutError = "transform-motion Mesh Particle resources failed: " +
				strOutError;
			return false;
		}
		Client::CEffectPlayback ParticlePlayback;
		if (!ParticlePlayback.Stage_PrevalidatedDocument(
				ParticleDocument, PreparedParticle, strOutError))
		{
			strOutError = "transform-motion Mesh Particle staging failed: " +
				strOutError;
			return false;
		}
		Client::EFFECT_PARTICLE_RUNTIME_PROBE MotionEndParticle;
		Client::EFFECT_PARTICLE_RUNTIME_PROBE HoldParticle;
		ParticlePlayback.Seek(1.f, Identity);
		if (!ParticlePlayback.Query_ParticleRuntimeProbe(
				Particle.strElementId, MotionEndParticle) ||
			MotionEndParticle.iActiveParticleCount != 1u)
		{
			strOutError = "transform-motion Mesh Particle end probe is missing";
			return false;
		}
		ParticlePlayback.Seek(2.f, Identity);
		if (!ParticlePlayback.Query_ParticleRuntimeProbe(
				Particle.strElementId, HoldParticle) ||
			HoldParticle.iActiveParticleCount != 1u ||
			std::abs(MotionEndParticle.vFirstWorldPosition.x -
				HoldParticle.vFirstWorldPosition.x) > 1.0e-4f ||
			std::abs(MotionEndParticle.vFirstWorldPosition.y -
				HoldParticle.vFirstWorldPosition.y) > 1.0e-4f ||
			std::abs(MotionEndParticle.vFirstWorldPosition.z -
				HoldParticle.vFirstWorldPosition.z) > 1.0e-4f)
		{
			strOutError =
				"Mesh Particle Element root continued moving during hold";
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC LegacyRevolutionDocument =
			ParticleDocument;
		Client::EFFECT_ELEMENT_DESC& LegacyRevolution =
			LegacyRevolutionDocument.Elements.front();
		LegacyRevolution.Detail.Timing.fLifeTimeSeconds = 1.f / 60.f;
		LegacyRevolution.Detail.Timing.fTransformMotionDurationSeconds = 0.f;
		LegacyRevolution.Detail.Transform.vPosition = {};
		LegacyRevolution.Detail.Transform.vVelocityPerSecond = {};
		LegacyRevolution.Detail.Transform.vRotationDegrees = {};
		LegacyRevolution.Detail.Transform.vRevolutionDegreesPerSecond =
			{ 0.f, 1300.f, 0.f };
		LegacyRevolution.Detail.LinearLerp = {};
		LegacyRevolution.Detail.Particle.vLifeTimeSeconds = { 1.05f, 1.05f };
		LegacyRevolution.Detail.Particle.vInitialPositionMin = { 1.f, 0.f, 0.f };
		LegacyRevolution.Detail.Particle.vInitialPositionMax = { 1.f, 0.f, 0.f };
		LegacyRevolution.Detail.Particle.vInitialVelocityMin = {};
		LegacyRevolution.Detail.Particle.vInitialVelocityMax = {};
		LegacyRevolution.Detail.Particle.vAcceleration = {};
		LegacyRevolution.Detail.Particle.bLocalSpace = true;
		std::shared_ptr<const Client::CEffectPlayback::PREPARED_RESOURCES>
			PreparedLegacyRevolution;
		if (!Client::CEffectPlayback::Prepare_DocumentResources(
				LegacyRevolutionDocument, PreparedLegacyRevolution, strOutError))
		{
			strOutError = "legacy Revolution resources failed: " + strOutError;
			return false;
		}
		Client::CEffectPlayback LegacyRevolutionPlayback;
		if (!LegacyRevolutionPlayback.Stage_PrevalidatedDocument(
				LegacyRevolutionDocument, PreparedLegacyRevolution, strOutError))
		{
			strOutError = "legacy Revolution staging failed: " + strOutError;
			return false;
		}
		Client::EFFECT_PARTICLE_RUNTIME_PROBE FirstLegacyRevolutionSample;
		Client::EFFECT_PARTICLE_RUNTIME_PROBE SecondLegacyRevolutionSample;
		LegacyRevolutionPlayback.Seek(0.1f, Identity);
		if (!LegacyRevolutionPlayback.Query_ParticleRuntimeProbe(
				LegacyRevolution.strElementId, FirstLegacyRevolutionSample) ||
			FirstLegacyRevolutionSample.iActiveParticleCount != 1u)
		{
			strOutError = "legacy Revolution first sample is missing";
			return false;
		}
		LegacyRevolutionPlayback.Seek(0.2f, Identity);
		if (!LegacyRevolutionPlayback.Query_ParticleRuntimeProbe(
				LegacyRevolution.strElementId, SecondLegacyRevolutionSample) ||
			SecondLegacyRevolutionSample.iActiveParticleCount != 1u)
		{
			strOutError = "legacy Revolution second sample is missing";
			return false;
		}
		const float3_t LegacyRevolutionDelta = {
			SecondLegacyRevolutionSample.vFirstWorldPosition.x -
				FirstLegacyRevolutionSample.vFirstWorldPosition.x,
			SecondLegacyRevolutionSample.vFirstWorldPosition.y -
				FirstLegacyRevolutionSample.vFirstWorldPosition.y,
			SecondLegacyRevolutionSample.vFirstWorldPosition.z -
				FirstLegacyRevolutionSample.vFirstWorldPosition.z };
		const f32_t fLegacyRevolutionDeltaSquared =
			LegacyRevolutionDelta.x * LegacyRevolutionDelta.x +
			LegacyRevolutionDelta.y * LegacyRevolutionDelta.y +
			LegacyRevolutionDelta.z * LegacyRevolutionDelta.z;
		const auto ExpectedLegacyRevolutionPosition = [](const f32_t fTime)
		{
			float4x4_t ExpectedWorld{};
			XMStoreFloat4x4(&ExpectedWorld,
				XMMatrixTranslation(1.f, 0.f, 0.f) *
				XMMatrixRotationY(XMConvertToRadians(1300.f * fTime)));
			return float3_t(
				ExpectedWorld._41, ExpectedWorld._42, ExpectedWorld._43);
		};
		const float3_t ExpectedFirstLegacyRevolution =
			ExpectedLegacyRevolutionPosition(0.1f);
		const float3_t ExpectedSecondLegacyRevolution =
			ExpectedLegacyRevolutionPosition(0.2f);
		const auto PositionMatches = [](const float3_t& Actual,
			const float3_t& Expected)
		{
			return std::abs(Actual.x - Expected.x) <= 1.0e-4f &&
				std::abs(Actual.y - Expected.y) <= 1.0e-4f &&
				std::abs(Actual.z - Expected.z) <= 1.0e-4f;
		};
		if (!(fLegacyRevolutionDeltaSquared > 1.f) ||
			!PositionMatches(
				FirstLegacyRevolutionSample.vFirstWorldPosition,
				ExpectedFirstLegacyRevolution) ||
			!PositionMatches(
				SecondLegacyRevolutionSample.vFirstWorldPosition,
				ExpectedSecondLegacyRevolution))
		{
			strOutError =
				"zero Motion Duration did not preserve legacy Revolution time";
			return false;
		}

		strOutError.clear();
		return true;
	}

	Client::EFFECT_DOCUMENT_DESC Build_ManualParticleContractDocument(
		const std::string_view strEffectAssetId,
		const std::string_view strElementId,
		const bool_t bMeshParticle)
	{
		Client::EFFECT_DOCUMENT_DESC Document;
		Document.strEffectAssetId = std::string(strEffectAssetId);
		Document.strDisplayName = std::string(strEffectAssetId);

		Client::EFFECT_ELEMENT_DESC Element;
		Element.strElementId = std::string(strElementId);
		Element.strDisplayName = std::string(strElementId);
		Element.strGroupId = "contract.fixture";
		Element.eKind = Client::EFFECT_ELEMENT_KIND::PARTICLE;
		Element.Material = {};
		Element.Material.eRenderProfile =
			Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
		Element.ResourceBindings.push_back({
			"base",
			"Effect/Warlord/Textures/FX_TEX_04/fx_h_wave_01.dds"
		});
		if (bMeshParticle)
		{
			Element.ResourceBindings.push_back({
				"meshModel",
				"Effect/Warlord/Meshes/FX_SM_00/fm_d_ring_008.wmodel"
			});
		}

		Element.Detail.Timing.fStartDelaySeconds = 0.f;
		Element.Detail.Timing.fLifeTimeSeconds = 2.f;
		Element.Detail.Transform.vPosition = bMeshParticle ?
			float3_t(0.f, 1.f, 0.f) : float3_t{};
		Element.Detail.Mesh.bUseModelMaterial = false;
		Element.Detail.Mesh.fModelPreScale = bMeshParticle ? 0.01f : 1.f;
		Element.Detail.Sprite.bBillboard = false;
		Element.Detail.Particle.iMaxParticles = bMeshParticle ? 1u : 16u;
		Element.Detail.Particle.iBurstCount = bMeshParticle ? 1u : 16u;
		Element.Detail.Particle.fSpawnRatePerSecond = 0.f;
		Element.Detail.Particle.iRandomSeed = 17u;
		Element.Detail.Particle.vLifeTimeSeconds = { 2.f, 2.f };
		Element.Detail.Particle.vInitialPositionMin = {};
		Element.Detail.Particle.vInitialPositionMax = {};
		Element.Detail.Particle.vInitialVelocityMin = {};
		Element.Detail.Particle.vInitialVelocityMax = {};
		Element.Detail.Particle.vAcceleration = {};
		Element.Detail.Particle.vStartSize = { 1.f, 1.f };
		Element.Detail.Particle.vEndSize = { 1.f, 1.f };
		Element.Detail.Particle.bLocalSpace = true;
		Element.Detail.Particle.bBillboard = false;
		Document.Elements.emplace_back(std::move(Element));
		return Document;
	}

	bool_t Evaluate_ManualParticleContractDocument(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		const f32_t fSampleTimeSeconds,
		std::vector<Client::EFFECT_EVALUATED_PARTICLE>& OutParticles,
		std::string& strOutError)
	{
		std::string ValidationError;
		if (!Client::CEffectDocumentCodec::Validate(Document, ValidationError))
		{
			strOutError = "manual Particle contract fixture was invalid: " +
				ValidationError;
			return false;
		}
		std::shared_ptr<const Client::CEffectPlayback::PREPARED_RESOURCES>
			Prepared;
		if (!Client::CEffectPlayback::Prepare_DocumentResources(
				Document, Prepared, strOutError))
		{
			strOutError = "manual Particle contract resources failed: " +
				strOutError;
			return false;
		}
		Client::CEffectPlayback Playback;
		if (!Playback.Stage_PrevalidatedDocument(
				Document, Prepared, strOutError))
		{
			strOutError = "manual Particle contract staging failed: " +
				strOutError;
			return false;
		}
		float4x4_t Identity{};
		XMStoreFloat4x4(&Identity, XMMatrixIdentity());
		Playback.Seek(fSampleTimeSeconds, Identity);
		OutParticles = Playback.Get_Frame().Particles;
		strOutError.clear();
		return true;
	}

	bool_t Validate_ManualMeshParticleScaleComposition(
		std::string& strOutError)
	{
		constexpr f32_t EPSILON = 1.0e-4f;
		Client::EFFECT_DOCUMENT_DESC Document =
			Build_ManualParticleContractDocument(
				"effect.contract.manual-mesh-particle-scale",
				"manual.mesh-particle.scale", true);
		Client::EFFECT_ELEMENT_DESC& Element = Document.Elements.front();
		Element.Detail.Transform.vScale = { 2.f, 3.f, 4.f };
		Element.Detail.Particle.vStartSize = { 1.5f, 2.5f };
		Element.Detail.Particle.vEndSize = { 3.5f, 6.5f };

		std::vector<Client::EFFECT_EVALUATED_PARTICLE> Particles;
		if (!Evaluate_ManualParticleContractDocument(
				Document, 0.5f, Particles, strOutError) ||
			Particles.size() != 1u)
		{
			strOutError = "manual Mesh Particle scale sample failed: " +
				strOutError;
			return false;
		}

		const Client::EFFECT_EVALUATED_PARTICLE& Particle = Particles.front();
		vector_t Scale{};
		vector_t Rotation{};
		vector_t Translation{};
		if (!XMMatrixDecompose(&Scale, &Rotation, &Translation,
				XMLoadFloat4x4(&Particle.World)))
		{
			strOutError = "manual Mesh Particle world matrix did not decompose";
			return false;
		}
		float3_t ActualScale{};
		XMStoreFloat3(&ActualScale, Scale);
		const f32_t T = Particle.fNormalizedLife;
		const f32_t fParticleX =
			Element.Detail.Particle.vStartSize.x +
			(Element.Detail.Particle.vEndSize.x -
				Element.Detail.Particle.vStartSize.x) * T;
		const f32_t fParticleY =
			Element.Detail.Particle.vStartSize.y +
			(Element.Detail.Particle.vEndSize.y -
				Element.Detail.Particle.vStartSize.y) * T;
		const float3_t ExpectedScale{
			fParticleX * Element.Detail.Transform.vScale.x,
			fParticleY * Element.Detail.Transform.vScale.y,
			0.5f * (fParticleX + fParticleY) *
				Element.Detail.Transform.vScale.z
		};
		if (std::abs(ActualScale.x - ExpectedScale.x) > EPSILON ||
			std::abs(ActualScale.y - ExpectedScale.y) > EPSILON ||
			std::abs(ActualScale.z - ExpectedScale.z) > EPSILON)
		{
			strOutError =
				"manual Mesh Particle did not compose Transform Scale and Start/End Size";
			return false;
		}

		strOutError.clear();
		return true;
	}

	bool_t Validate_EvenRingOrientationAndMeshRingFillContract(
		std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>&
			OutRingFillDocument,
		std::string& strOutError)
	{
		constexpr f32_t RING_RADIUS = 2.f;
		constexpr size_t RING_PARTICLE_COUNT = 16u;
		constexpr f32_t EPSILON = 1.0e-4f;
		const auto Reject = [&strOutError](
			const Client::EFFECT_DOCUMENT_DESC& Candidate,
			const std::string_view strLabel)
		{
			std::string ValidationError;
			if (Client::CEffectDocumentCodec::Validate(
					Candidate, ValidationError))
			{
				strOutError = std::string(strLabel) + " was accepted";
				return false;
			}
			return true;
		};

		Client::EFFECT_DOCUMENT_DESC DefaultSprite =
			Build_ManualParticleContractDocument(
				"effect.contract.even-ring.default",
				"particle.even-ring.default", false);
		Client::EFFECT_PARTICLE_DESC& DefaultParticle =
			DefaultSprite.Elements.front().Detail.Particle;
		DefaultParticle.SpawnShape.eKind =
			Client::EFFECT_PARTICLE_SPAWN_SHAPE::RING;
		DefaultParticle.SpawnShape.fRadius = RING_RADIUS;
		DefaultParticle.SpawnShape.fInnerRadius = RING_RADIUS;
		DefaultParticle.SpawnShape.fArcDegrees = 360.f;
		const std::string DefaultSpriteJson =
			Client::CEffectDocumentCodec::Serialize(DefaultSprite);
		Client::EFFECT_DOCUMENT_DESC DefaultSpriteRoundTrip;
		if (DefaultSpriteJson.find("\"distribution\"") != std::string::npos ||
			DefaultSpriteJson.find("\"initialOrientation\"") !=
				std::string::npos ||
			!Client::CEffectDocumentCodec::Parse(
				DefaultSpriteJson, DefaultSpriteRoundTrip, strOutError) ||
			DefaultSpriteRoundTrip.Elements.size() != 1u ||
			DefaultSpriteRoundTrip.Elements.front().Detail.Particle.SpawnShape.
				eDistribution !=
				Client::EFFECT_PARTICLE_SPAWN_DISTRIBUTION::RANDOM ||
			!DefaultSpriteRoundTrip.Elements.front().Detail.Particle.
				InitialOrientation.Is_Default())
		{
			strOutError =
				"legacy Particle distribution/orientation omission did not round-trip";
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC EvenRing = DefaultSprite;
		EvenRing.strEffectAssetId = "effect.contract.even-ring.explicit";
		EvenRing.strDisplayName = EvenRing.strEffectAssetId;
		Client::EFFECT_PARTICLE_DESC& EvenParticle =
			EvenRing.Elements.front().Detail.Particle;
		EvenParticle.SpawnShape.eDistribution =
			Client::EFFECT_PARTICLE_SPAWN_DISTRIBUTION::EVEN;
		EvenParticle.InitialOrientation.eMode = Client::
			EFFECT_PARTICLE_ORIENTATION_MODE::GROUND_RADIAL_OUTWARD;
		EvenParticle.InitialOrientation.fOffsetDegrees = 0.f;
		const std::string EvenRingJson =
			Client::CEffectDocumentCodec::Serialize(EvenRing);
		Client::EFFECT_DOCUMENT_DESC EvenRingRoundTrip;
		if (EvenRingJson.find("\"distribution\"") == std::string::npos ||
			EvenRingJson.find("\"initialOrientation\"") ==
				std::string::npos ||
			!Client::CEffectDocumentCodec::Parse(
				EvenRingJson, EvenRingRoundTrip, strOutError) ||
			EvenRingRoundTrip.Elements.size() != 1u ||
			EvenRingRoundTrip.Elements.front().Detail.Particle.SpawnShape.
				eDistribution !=
				Client::EFFECT_PARTICLE_SPAWN_DISTRIBUTION::EVEN ||
			EvenRingRoundTrip.Elements.front().Detail.Particle.InitialOrientation.
				eMode != Client::EFFECT_PARTICLE_ORIENTATION_MODE::
					GROUND_RADIAL_OUTWARD)
		{
			strOutError =
				"explicit Even Ring orientation did not survive codec round-trip";
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC InvalidEven = EvenRingRoundTrip;
		InvalidEven.Elements.front().Detail.Particle.fSpawnRatePerSecond = 1.f;
		if (!Reject(InvalidEven, "continuous Even Ring distribution"))
			return false;
		InvalidEven = EvenRingRoundTrip;
		InvalidEven.Elements.front().Detail.Particle.SpawnShape.eKind =
			Client::EFFECT_PARTICLE_SPAWN_SHAPE::BOX;
		InvalidEven.Elements.front().Detail.Particle.SpawnShape.vExtents =
			{ 1.f, 1.f, 1.f };
		if (!Reject(InvalidEven, "non-Ring Even distribution"))
			return false;
		InvalidEven = EvenRingRoundTrip;
		InvalidEven.Elements.front().Detail.Particle.iBurstCount = 1u;
		if (!Reject(InvalidEven, "single-particle Even Ring distribution"))
			return false;
		Client::EFFECT_DOCUMENT_DESC InvalidOrientation = EvenRingRoundTrip;
		InvalidOrientation.Elements.front().Detail.Particle.bBillboard = true;
		if (!Reject(InvalidOrientation, "billboard radial orientation"))
			return false;

		std::vector<Client::EFFECT_EVALUATED_PARTICLE> FirstRing;
		std::vector<Client::EFFECT_EVALUATED_PARTICLE> SecondRing;
		if (!Evaluate_ManualParticleContractDocument(
				EvenRingRoundTrip, 0.25f, FirstRing, strOutError) ||
			!Evaluate_ManualParticleContractDocument(
				EvenRingRoundTrip, 0.25f, SecondRing, strOutError) ||
			FirstRing.size() != RING_PARTICLE_COUNT ||
			SecondRing.size() != RING_PARTICLE_COUNT)
		{
			strOutError = "Even Ring fixed burst did not produce exactly 16 particles: " +
				strOutError;
			return false;
		}

		float3_t Centroid{};
		std::array<float3_t, RING_PARTICLE_COUNT> RingPositions{};
		for (size_t iParticle = 0u; iParticle < RING_PARTICLE_COUNT; ++iParticle)
		{
			const float4x4_t& World = FirstRing[iParticle].World;
			const float4x4_t& RepeatedWorld = SecondRing[iParticle].World;
			const float3_t Position(
				World.m[3][0], World.m[3][1], World.m[3][2]);
			RingPositions[iParticle] = Position;
			Centroid.x += Position.x;
			Centroid.y += Position.y;
			Centroid.z += Position.z;
			const f32_t fRadius = std::sqrt(
				Position.x * Position.x + Position.z * Position.z);
			if (std::abs(fRadius - RING_RADIUS) > EPSILON ||
				std::abs(Position.y) > EPSILON)
			{
				strOutError = "Even Ring particle escaped its authored XZ radius";
				return false;
			}
			for (size_t iRow = 0u; iRow < 4u; ++iRow)
			{
				for (size_t iColumn = 0u; iColumn < 4u; ++iColumn)
				{
					if (std::bit_cast<uint32_t>(World.m[iRow][iColumn]) !=
						std::bit_cast<uint32_t>(
							RepeatedWorld.m[iRow][iColumn]))
					{
						strOutError =
							"Even Ring seek/reset did not reproduce the same matrices";
						return false;
					}
				}
			}
		}
		Centroid.x /= static_cast<f32_t>(RING_PARTICLE_COUNT);
		Centroid.y /= static_cast<f32_t>(RING_PARTICLE_COUNT);
		Centroid.z /= static_cast<f32_t>(RING_PARTICLE_COUNT);
		if (std::abs(Centroid.x) > EPSILON ||
			std::abs(Centroid.y) > EPSILON ||
			std::abs(Centroid.z) > EPSILON)
		{
			strOutError = "Even Ring centroid moved away from the Element origin";
			return false;
		}
		const f32_t fExpectedAdjacentDot = std::cos(
			2.f * XM_PI / static_cast<f32_t>(RING_PARTICLE_COUNT));
		for (size_t iParticle = 0u; iParticle < RING_PARTICLE_COUNT; ++iParticle)
		{
			for (size_t iOther = iParticle + 1u;
				iOther < RING_PARTICLE_COUNT; ++iOther)
			{
				const f32_t fDx = RingPositions[iParticle].x -
					RingPositions[iOther].x;
				const f32_t fDz = RingPositions[iParticle].z -
					RingPositions[iOther].z;
				if (fDx * fDx + fDz * fDz < 1.0e-4f)
				{
					strOutError = "Even Ring emitted a duplicate endpoint";
					return false;
				}
			}
			const float3_t& Current = RingPositions[iParticle];
			const float3_t& Next = RingPositions[
				(iParticle + 1u) % RING_PARTICLE_COUNT];
			const f32_t fAdjacentDot =
				(Current.x * Next.x + Current.z * Next.z) /
				(RING_RADIUS * RING_RADIUS);
			if (std::abs(fAdjacentDot - fExpectedAdjacentDot) > 1.0e-4f)
			{
				strOutError = "Even Ring adjacent angular spacing changed";
				return false;
			}
		}

		const std::array<Client::EFFECT_PARTICLE_ORIENTATION_MODE, 4u>
			OrientationModes = {{
				Client::EFFECT_PARTICLE_ORIENTATION_MODE::GROUND_RADIAL_OUTWARD,
				Client::EFFECT_PARTICLE_ORIENTATION_MODE::GROUND_RADIAL_INWARD,
				Client::EFFECT_PARTICLE_ORIENTATION_MODE::GROUND_TANGENT_CLOCKWISE,
				Client::EFFECT_PARTICLE_ORIENTATION_MODE::
					GROUND_TANGENT_COUNTER_CLOCKWISE
			}};
		for (const Client::EFFECT_PARTICLE_ORIENTATION_MODE eMode :
			OrientationModes)
		{
			Client::EFFECT_DOCUMENT_DESC Oriented = EvenRingRoundTrip;
			Oriented.Elements.front().Detail.Particle.InitialOrientation.eMode = eMode;
			std::vector<Client::EFFECT_EVALUATED_PARTICLE> OrientedParticles;
			if (!Evaluate_ManualParticleContractDocument(
					Oriented, 0.25f, OrientedParticles, strOutError) ||
				OrientedParticles.size() != RING_PARTICLE_COUNT)
			{
				strOutError = "oriented Even Ring evaluation failed: " + strOutError;
				return false;
			}
			for (size_t iParticle = 0u;
				iParticle < RING_PARTICLE_COUNT; ++iParticle)
			{
				const float4x4_t& World = OrientedParticles[iParticle].World;
				const float3_t Position(
					World.m[3][0], World.m[3][1], World.m[3][2]);
				if (std::abs(Position.x - RingPositions[iParticle].x) > EPSILON ||
					std::abs(Position.y - RingPositions[iParticle].y) > EPSILON ||
					std::abs(Position.z - RingPositions[iParticle].z) > EPSILON)
				{
					strOutError =
						"individual Sprite orientation moved its Ring position";
					return false;
				}
				const f32_t fBasisLength = std::sqrt(
					World.m[0][0] * World.m[0][0] +
					World.m[0][2] * World.m[0][2]);
				if (!(fBasisLength > EPSILON) || std::abs(World.m[0][1]) > EPSILON)
				{
					strOutError = "oriented Sprite local +X basis is invalid";
					return false;
				}
				const f32_t fRadialX = Position.x / RING_RADIUS;
				const f32_t fRadialZ = Position.z / RING_RADIUS;
				f32_t fExpectedX = fRadialX;
				f32_t fExpectedZ = fRadialZ;
				switch (eMode)
				{
				case Client::EFFECT_PARTICLE_ORIENTATION_MODE::
					GROUND_RADIAL_INWARD:
					fExpectedX = -fRadialX;
					fExpectedZ = -fRadialZ;
					break;
				case Client::EFFECT_PARTICLE_ORIENTATION_MODE::
					GROUND_TANGENT_CLOCKWISE:
					fExpectedX = -fRadialZ;
					fExpectedZ = fRadialX;
					break;
				case Client::EFFECT_PARTICLE_ORIENTATION_MODE::
					GROUND_TANGENT_COUNTER_CLOCKWISE:
					fExpectedX = fRadialZ;
					fExpectedZ = -fRadialX;
					break;
				case Client::EFFECT_PARTICLE_ORIENTATION_MODE::
					GROUND_RADIAL_OUTWARD:
				case Client::EFFECT_PARTICLE_ORIENTATION_MODE::FIXED:
				case Client::EFFECT_PARTICLE_ORIENTATION_MODE::END:
				default:
					break;
				}
				const f32_t fBasisDot =
					(World.m[0][0] / fBasisLength) * fExpectedX +
					(World.m[0][2] / fBasisLength) * fExpectedZ;
				if (fBasisDot < 0.9999f)
				{
					strOutError =
						"Sprite local +X did not follow its radial/tangent mode";
					return false;
				}
			}
		}

		Client::EFFECT_DOCUMENT_DESC OffsetOrientation = EvenRingRoundTrip;
		constexpr f32_t OFFSET_DEGREES = 37.f;
		OffsetOrientation.Elements.front().Detail.Particle.InitialOrientation.
			fOffsetDegrees = OFFSET_DEGREES;
		std::vector<Client::EFFECT_EVALUATED_PARTICLE> OffsetParticles;
		if (!Evaluate_ManualParticleContractDocument(
				OffsetOrientation, 0.25f, OffsetParticles, strOutError) ||
			OffsetParticles.size() != RING_PARTICLE_COUNT)
		{
			strOutError = "Sprite orientation offset evaluation failed: " +
				strOutError;
			return false;
		}
		const f32_t fOffsetRadians = XMConvertToRadians(OFFSET_DEGREES);
		const f32_t fOffsetCos = std::cos(fOffsetRadians);
		const f32_t fOffsetSin = std::sin(fOffsetRadians);
		for (size_t iParticle = 0u; iParticle < RING_PARTICLE_COUNT; ++iParticle)
		{
			const float4x4_t& World = OffsetParticles[iParticle].World;
			const f32_t fBasisLength = std::sqrt(
				World.m[0][0] * World.m[0][0] +
				World.m[0][2] * World.m[0][2]);
			const f32_t fRadialX = RingPositions[iParticle].x / RING_RADIUS;
			const f32_t fRadialZ = RingPositions[iParticle].z / RING_RADIUS;
			const f32_t fExpectedX =
				fRadialX * fOffsetCos + fRadialZ * fOffsetSin;
			const f32_t fExpectedZ =
				fRadialZ * fOffsetCos - fRadialX * fOffsetSin;
			const f32_t fBasisDot =
				(World.m[0][0] / fBasisLength) * fExpectedX +
				(World.m[0][2] / fBasisLength) * fExpectedZ;
			if (fBasisDot < 0.9999f)
			{
				strOutError = "Sprite orientation offset was not composed per particle";
				return false;
			}
		}

		Client::EFFECT_DOCUMENT_DESC DefaultMesh =
			Build_ManualParticleContractDocument(
				"effect.contract.mesh-ring-fill.default",
				"particle.mesh-ring-fill.default", true);
		const std::string DefaultMeshJson =
			Client::CEffectDocumentCodec::Serialize(DefaultMesh);
		Client::EFFECT_DOCUMENT_DESC DefaultMeshRoundTrip;
		if (DefaultMeshJson.find("\"ringFill\"") != std::string::npos ||
			DefaultMeshJson.find("\"ringFillProgress\"") != std::string::npos ||
			!Client::CEffectDocumentCodec::Parse(
				DefaultMeshJson, DefaultMeshRoundTrip, strOutError) ||
			DefaultMeshRoundTrip.Elements.size() != 1u ||
			!DefaultMeshRoundTrip.Elements.front().Detail.Mesh.RingFill.Is_Default() ||
			DefaultMeshRoundTrip.Elements.front().Detail.LinearLerp.
				bRingFillProgress)
		{
			strOutError = "legacy Mesh Ring Fill omission did not round-trip";
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC RingFill = DefaultMesh;
		RingFill.strEffectAssetId = "effect.contract.mesh-ring-fill.explicit";
		RingFill.strDisplayName = RingFill.strEffectAssetId;
		Client::EFFECT_ELEMENT_DESC& RingFillElement = RingFill.Elements.front();
		RingFillElement.Detail.Mesh.RingFill.bEnabled = true;
		RingFillElement.Detail.Mesh.RingFill.fProgress = 0.f;
		RingFillElement.Detail.Mesh.RingFill.eDirection =
			Client::EFFECT_RING_FILL_DIRECTION::OUTER_TO_INNER;
		RingFillElement.Detail.Mesh.RingFill.fFeather = 0.1f;
		RingFillElement.Detail.Mesh.RingFill.bInvert = true;
		RingFillElement.Detail.LinearLerp.bRingFillProgress = true;
		RingFillElement.Detail.LinearLerp.fEndRingFillProgress = 1.f;
		RingFillElement.Detail.Particle.vLifeTimeSeconds = { 2.25f, 2.25f };
		const std::string RingFillJson =
			Client::CEffectDocumentCodec::Serialize(RingFill);
		Client::EFFECT_DOCUMENT_DESC RingFillRoundTrip;
		if (RingFillJson.find("\"ringFill\"") == std::string::npos ||
			RingFillJson.find("\"ringFillProgress\"") == std::string::npos ||
			!Client::CEffectDocumentCodec::Parse(
				RingFillJson, RingFillRoundTrip, strOutError) ||
			RingFillRoundTrip.Elements.size() != 1u)
		{
			strOutError = "explicit Mesh Ring Fill did not survive codec round-trip";
			return false;
		}
		const Client::EFFECT_ELEMENT_DESC& ParsedRingFill =
			RingFillRoundTrip.Elements.front();
		if (!ParsedRingFill.Detail.Mesh.RingFill.bEnabled ||
			ParsedRingFill.Detail.Mesh.RingFill.fProgress != 0.f ||
			ParsedRingFill.Detail.Mesh.RingFill.eDirection !=
				Client::EFFECT_RING_FILL_DIRECTION::OUTER_TO_INNER ||
			ParsedRingFill.Detail.Mesh.RingFill.fFeather != 0.1f ||
			!ParsedRingFill.Detail.Mesh.RingFill.bInvert ||
			!ParsedRingFill.Detail.LinearLerp.bRingFillProgress ||
			ParsedRingFill.Detail.LinearLerp.fEndRingFillProgress != 1.f)
		{
			strOutError = "Mesh Ring Fill codec changed an explicit field";
			return false;
		}
		std::string ValidationError;
		if (!Client::CEffectDocumentCodec::Validate(
				RingFillRoundTrip, ValidationError))
		{
			strOutError = "valid Mesh Ring Fill was rejected: " + ValidationError;
			return false;
		}
		std::vector<Client::EFFECT_EVALUATED_PARTICLE> CompletedRingParticles;
		if (!Evaluate_ManualParticleContractDocument(
				RingFillRoundTrip, 2.f, CompletedRingParticles, strOutError) ||
			CompletedRingParticles.size() != 1u ||
			CompletedRingParticles.front().Color.w < 0.999f)
		{
			strOutError =
				"completed Mesh Ring Fill did not retain authored alpha: " +
				strOutError;
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC InvalidRingFill = RingFillRoundTrip;
		InvalidRingFill.Elements.front().Detail.Mesh.RingFill.fProgress = -0.001f;
		if (!Reject(InvalidRingFill, "negative Mesh Ring Fill progress"))
			return false;
		InvalidRingFill = RingFillRoundTrip;
		InvalidRingFill.Elements.front().Detail.Mesh.RingFill.fFeather = 0.501f;
		if (!Reject(InvalidRingFill, "overflow Mesh Ring Fill feather"))
			return false;
		InvalidRingFill = RingFillRoundTrip;
		InvalidRingFill.Elements.front().Detail.LinearLerp.
			fEndRingFillProgress = 1.001f;
		if (!Reject(InvalidRingFill, "overflow Mesh Ring Fill lerp end"))
			return false;
		InvalidRingFill = RingFillRoundTrip;
		InvalidRingFill.Elements.front().Detail.Mesh.RingFill.bEnabled = false;
		if (!Reject(InvalidRingFill, "disabled Mesh Ring Fill lerp"))
			return false;
		InvalidRingFill = RingFillRoundTrip;
		InvalidRingFill.Elements.front().Material.eRenderProfile =
			Client::EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE;
		if (!Reject(InvalidRingFill, "Opaque Mesh Ring Fill"))
			return false;
		InvalidRingFill = RingFillRoundTrip;
		InvalidRingFill.Elements.front().eKind =
			Client::EFFECT_ELEMENT_KIND::MESH;
		if (!Reject(InvalidRingFill, "standalone Mesh Ring Fill"))
			return false;
		InvalidRingFill = DefaultSprite;
		InvalidRingFill.Elements.front().Detail.Mesh.RingFill =
			RingFillRoundTrip.Elements.front().Detail.Mesh.RingFill;
		if (!Reject(InvalidRingFill, "Sprite Particle Mesh Ring Fill"))
			return false;

		OutRingFillDocument =
			std::make_shared<Client::EFFECT_DOCUMENT_DESC>(
				std::move(RingFillRoundTrip));
		strOutError.clear();
		return true;
	}

	bool_t Validate_GenericSpriteLinearRevealContract(
		std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>&
			OutRevealDocument,
		std::string& strOutError)
	{
		const auto Reject = [&strOutError](
			const Client::EFFECT_DOCUMENT_DESC& Candidate,
			const std::string_view strLabel)
		{
			std::string ValidationError;
			if (Client::CEffectDocumentCodec::Validate(
					Candidate, ValidationError))
			{
				strOutError = std::string(strLabel) + " was accepted";
				return false;
			}
			return true;
		};

		Client::EFFECT_DOCUMENT_DESC DefaultDocument =
			Build_ManualParticleContractDocument(
				"effect.contract.sprite-linear-reveal.default",
				"particle.sprite-linear-reveal.default", false);
		Client::EFFECT_ELEMENT_DESC& DefaultElement =
			DefaultDocument.Elements.front();
		DefaultElement.Detail.Particle.iMaxParticles = 1u;
		DefaultElement.Detail.Particle.iBurstCount = 1u;
		const std::string DefaultJson =
			Client::CEffectDocumentCodec::Serialize(DefaultDocument);
		Client::EFFECT_DOCUMENT_DESC DefaultRoundTrip;
		if (DefaultJson.find("\"linearReveal\"") != std::string::npos ||
			!Client::CEffectDocumentCodec::Parse(
				DefaultJson, DefaultRoundTrip, strOutError) ||
			DefaultRoundTrip.Elements.size() != 1u ||
			!DefaultRoundTrip.Elements.front().Detail.Sprite.LinearReveal.
				Is_Default())
		{
			strOutError =
				"legacy Sprite Linear Reveal omission did not round-trip";
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC RevealDocument = DefaultRoundTrip;
		RevealDocument.strEffectAssetId =
			"effect.contract.sprite-linear-reveal.explicit";
		RevealDocument.strDisplayName = RevealDocument.strEffectAssetId;
		Client::EFFECT_ELEMENT_DESC& RevealElement =
			RevealDocument.Elements.front();
		RevealElement.strElementId = "particle.sprite-linear-reveal.explicit";
		RevealElement.strDisplayName = RevealElement.strElementId;
		Client::EFFECT_LINEAR_REVEAL_DESC& Reveal =
			RevealElement.Detail.Sprite.LinearReveal;
		Reveal.bEnabled = true;
		Reveal.eAxis = Client::EFFECT_LINEAR_REVEAL_AXIS::V;
		Reveal.bInvert = true;
		Reveal.fStartSeconds = 0.1f;
		Reveal.fDurationSeconds = 0.55f;
		Reveal.fEdgeWidth = 0.045f;
		Reveal.fSoftness = 0.03f;
		Reveal.vEdgeColor = { 0.8f, 0.6f, 1.f, 1.f };
		Reveal.fEdgeEmissive = 7.f;
		RevealElement.Detail.Timing.fLifeTimeSeconds = 2.f;
		RevealElement.Detail.Particle.vLifeTimeSeconds = { 2.f, 2.f };

		const std::string RevealJson =
			Client::CEffectDocumentCodec::Serialize(RevealDocument);
		Client::EFFECT_DOCUMENT_DESC RevealRoundTrip;
		if (RevealJson.find("\"linearReveal\"") == std::string::npos ||
			RevealJson.find("\"durationSeconds\": 0.55") ==
				std::string::npos ||
			!Client::CEffectDocumentCodec::Parse(
				RevealJson, RevealRoundTrip, strOutError) ||
			RevealRoundTrip.Elements.size() != 1u)
		{
			strOutError =
				"explicit Sprite Linear Reveal did not survive codec round-trip";
			return false;
		}
		const Client::EFFECT_LINEAR_REVEAL_DESC& ParsedReveal =
			RevealRoundTrip.Elements.front().Detail.Sprite.LinearReveal;
		std::string ValidationError;
		if (!ParsedReveal.bEnabled ||
			ParsedReveal.eAxis != Client::EFFECT_LINEAR_REVEAL_AXIS::V ||
			!ParsedReveal.bInvert || ParsedReveal.fStartSeconds != 0.1f ||
			ParsedReveal.fDurationSeconds != 0.55f ||
			ParsedReveal.fEdgeWidth != 0.045f ||
			ParsedReveal.fSoftness != 0.03f ||
			ParsedReveal.vEdgeColor.x != 0.8f ||
			ParsedReveal.vEdgeColor.y != 0.6f ||
			ParsedReveal.vEdgeColor.z != 1.f ||
			ParsedReveal.vEdgeColor.w != 1.f ||
			ParsedReveal.fEdgeEmissive != 7.f ||
			!Client::CEffectDocumentCodec::Validate(
				RevealRoundTrip, ValidationError))
		{
			strOutError = "valid Sprite Linear Reveal was rejected or changed: " +
				ValidationError;
			return false;
		}

		std::vector<Client::EFFECT_EVALUATED_PARTICLE> DefaultParticles;
		std::vector<Client::EFFECT_EVALUATED_PARTICLE> RevealParticles;
		if (!Evaluate_ManualParticleContractDocument(
				DefaultRoundTrip, 1.5f, DefaultParticles, strOutError) ||
			!Evaluate_ManualParticleContractDocument(
				RevealRoundTrip, 1.5f, RevealParticles, strOutError) ||
			DefaultParticles.size() != 1u || RevealParticles.size() != 1u ||
			DefaultParticles.front().Color.w >= 0.3f ||
			RevealParticles.front().Color.w < 0.999f)
		{
			strOutError =
				"Sprite Linear Reveal did not own alpha independently of the legacy Particle fade";
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC StandaloneSprite = RevealRoundTrip;
		StandaloneSprite.Elements.front().eKind =
			Client::EFFECT_ELEMENT_KIND::SPRITE;
		if (!Client::CEffectDocumentCodec::Validate(
				StandaloneSprite, ValidationError))
		{
			strOutError =
				"standalone Sprite Linear Reveal was rejected: " + ValidationError;
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC Invalid = RevealRoundTrip;
		Invalid.Elements.front().Detail.Sprite.LinearReveal.eAxis =
			Client::EFFECT_LINEAR_REVEAL_AXIS::END;
		if (!Reject(Invalid, "invalid Sprite Linear Reveal axis"))
			return false;
		Invalid = RevealRoundTrip;
		Invalid.Elements.front().Detail.Sprite.LinearReveal.fDurationSeconds = 0.f;
		if (!Reject(Invalid, "zero Sprite Linear Reveal duration"))
			return false;
		Invalid = RevealRoundTrip;
		Invalid.Elements.front().Detail.Sprite.LinearReveal.fSoftness = 0.46f;
		if (!Reject(Invalid, "overflow Sprite Linear Reveal edge band"))
			return false;
		Invalid = RevealRoundTrip;
		Invalid.Elements.front().Detail.Particle.vLifeTimeSeconds = { 1.f, 2.f };
		if (!Reject(Invalid, "random Sprite Linear Reveal Particle lifetime"))
			return false;
		Invalid = RevealRoundTrip;
		Invalid.Elements.front().Detail.Sprite.LinearReveal.bEnabled = false;
		if (!Reject(Invalid, "disabled non-default Sprite Linear Reveal"))
			return false;
		Invalid = RevealRoundTrip;
		Invalid.Elements.front().Material.eRenderProfile =
			Client::EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE;
		if (!Reject(Invalid, "Opaque Sprite Linear Reveal"))
			return false;
		Invalid = RevealRoundTrip;
		Invalid.Elements.front().ResourceBindings.push_back({
			"meshModel",
			"Effect/Warlord/Meshes/FX_SM_00/fm_d_ring_008.wmodel"
		});
		if (!Reject(Invalid, "Mesh Particle Sprite Linear Reveal"))
			return false;

		OutRevealDocument =
			std::make_shared<Client::EFFECT_DOCUMENT_DESC>(
				std::move(RevealRoundTrip));
		strOutError.clear();
		return true;
	}

	bool_t Validate_WorldMarkCompositionContract(
		const std::filesystem::path& RepositoryRoot,
		std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>&
			OutWorldMarkDocument,
		std::string& strOutError)
	{
		const auto Reject = [&strOutError](
			const Client::EFFECT_DOCUMENT_DESC& Candidate,
			const std::string_view strLabel)
		{
			std::string ValidationError;
			if (Client::CEffectDocumentCodec::Validate(
					Candidate, ValidationError))
			{
				strOutError = std::string(strLabel) + " was accepted";
				return false;
			}
			return true;
		};

		Client::EFFECT_DOCUMENT_DESC DefaultDocument =
			Build_ManualParticleContractDocument(
				"effect.contract.world-mark.default",
				"particle.world-mark.default", false);
		Client::EFFECT_ELEMENT_DESC& DefaultElement =
			DefaultDocument.Elements.front();
		DefaultElement.Detail.Particle.iMaxParticles = 1u;
		DefaultElement.Detail.Particle.iBurstCount = 1u;
		const std::string DefaultJson =
			Client::CEffectDocumentCodec::Serialize(DefaultDocument);
		Client::EFFECT_DOCUMENT_DESC DefaultRoundTrip;
		if (DefaultJson.find("\"compositionLayer\"") != std::string::npos ||
			DefaultJson.find("\"receiverMode\"") != std::string::npos ||
			!Client::CEffectDocumentCodec::Parse(
				DefaultJson, DefaultRoundTrip, strOutError) ||
			DefaultRoundTrip.Elements.size() != 1u ||
			DefaultRoundTrip.Elements.front().eCompositionLayer !=
				Client::EFFECT_COMPOSITION_LAYER::NORMAL ||
			!DefaultRoundTrip.Elements.front().Detail.Decal.
				Is_ReceiverDefault())
		{
			strOutError =
				"legacy World Mark/Decal receiver omission did not round-trip";
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC WorldMarkDocument = DefaultRoundTrip;
		WorldMarkDocument.strEffectAssetId =
			"effect.contract.world-mark.explicit";
		WorldMarkDocument.strDisplayName =
			WorldMarkDocument.strEffectAssetId;
		Client::EFFECT_ELEMENT_DESC NormalElement =
			WorldMarkDocument.Elements.front();
		NormalElement.strElementId = "particle.world-mark.normal";
		NormalElement.strDisplayName = NormalElement.strElementId;
		NormalElement.eCompositionLayer =
			Client::EFFECT_COMPOSITION_LAYER::NORMAL;
		Client::EFFECT_ELEMENT_DESC MarkElement = NormalElement;
		MarkElement.strElementId = "particle.world-mark.ground";
		MarkElement.strDisplayName = MarkElement.strElementId;
		MarkElement.eCompositionLayer =
			Client::EFFECT_COMPOSITION_LAYER::WORLD_MARK;
		/* Keep NORMAL first in document order.  The WARP receipt below must
		   still prove that WORLD_MARK issued its draw first. */
		WorldMarkDocument.Elements = { NormalElement, MarkElement };
		std::string ValidationError;
		if (!Client::CEffectDocumentCodec::Validate(
				WorldMarkDocument, ValidationError))
		{
			strOutError = "valid World Mark document was rejected: " +
				ValidationError;
			return false;
		}
		const std::string WorldMarkJson =
			Client::CEffectDocumentCodec::Serialize(WorldMarkDocument);
		Client::EFFECT_DOCUMENT_DESC WorldMarkRoundTrip;
		if (WorldMarkJson.find(
				"\"compositionLayer\": \"worldMark\"") ==
				std::string::npos ||
			!Client::CEffectDocumentCodec::Parse(
				WorldMarkJson, WorldMarkRoundTrip, strOutError) ||
			WorldMarkRoundTrip.Elements.size() != 2u ||
			WorldMarkRoundTrip.Elements[0].eCompositionLayer !=
				Client::EFFECT_COMPOSITION_LAYER::NORMAL ||
			WorldMarkRoundTrip.Elements[1].eCompositionLayer !=
				Client::EFFECT_COMPOSITION_LAYER::WORLD_MARK)
		{
			strOutError =
				"explicit World Mark did not survive codec round-trip";
			return false;
		}

		Client::EFFECT_ELEMENT_DESC Applied = NormalElement;
		Client::Apply_EffectElementDetailDraft(
			Applied, WorldMarkRoundTrip.Elements[1]);
		if (Applied.eCompositionLayer !=
			Client::EFFECT_COMPOSITION_LAYER::WORLD_MARK)
		{
			strOutError =
				"Element detail draft did not preserve World Mark composition";
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC Invalid = WorldMarkRoundTrip;
		Invalid.Elements[1].eCompositionLayer =
			Client::EFFECT_COMPOSITION_LAYER::END;
		if (!Reject(Invalid, "invalid World Mark enum"))
			return false;
		Invalid = WorldMarkRoundTrip;
		Invalid.Elements[1].Material.eRenderProfile =
			Client::EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE;
		if (!Reject(Invalid, "Opaque World Mark"))
			return false;
		Invalid = WorldMarkRoundTrip;
		Invalid.Elements[1].strSourceNode =
			"authored-source-particle:world-mark-contract";
		if (!Reject(Invalid, "source Sprite Particle World Mark"))
			return false;
		Invalid = WorldMarkRoundTrip;
		Invalid.Elements[1].ResourceBindings.push_back({
			"meshModel",
			"Effect/Warlord/Meshes/FX_SM_00/fm_d_ring_008.wmodel"
		});
		if (!Reject(Invalid, "Mesh Particle World Mark"))
			return false;
		std::string InvalidTokenJson = WorldMarkJson;
		const size_t iWorldMarkToken =
			InvalidTokenJson.find("\"worldMark\"");
		if (std::string::npos == iWorldMarkToken)
		{
			strOutError = "World Mark invalid-token fixture is missing";
			return false;
		}
		InvalidTokenJson.replace(
			iWorldMarkToken, std::string_view("\"worldMark\"").size(),
			"\"invalidWorldMark\"");
		Client::EFFECT_DOCUMENT_DESC InvalidTokenDocument;
		if (Client::CEffectDocumentCodec::Parse(
				InvalidTokenJson, InvalidTokenDocument, ValidationError))
		{
			strOutError = "unknown World Mark token was accepted";
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC DecalDocument = DefaultRoundTrip;
		DecalDocument.strEffectAssetId =
			"effect.contract.world-mark.decal";
		DecalDocument.strDisplayName = DecalDocument.strEffectAssetId;
		Client::EFFECT_ELEMENT_DESC& DecalElement =
			DecalDocument.Elements.front();
		DecalElement.strElementId = "decal.world-mark.ground";
		DecalElement.strDisplayName = DecalElement.strElementId;
		DecalElement.strSourceNode =
			"authored-source-decal:world-mark-contract";
		DecalElement.eKind = Client::EFFECT_ELEMENT_KIND::DECAL;
		DecalElement.eCompositionLayer =
			Client::EFFECT_COMPOSITION_LAYER::WORLD_MARK;
		DecalElement.Detail.Decal.vSize = { 3.f, 4.f };
		DecalElement.Detail.Decal.fDepth = 0.25f;
		DecalElement.Detail.Decal.eReceiverMode =
			Client::EFFECT_DECAL_RECEIVER_MODE::UPWARD_SURFACES;
		DecalElement.Detail.Decal.fNormalCutoff = 0.75f;
		DecalElement.Detail.Decal.fEdgeFade = 0.05f;
		if (!Client::CEffectDocumentCodec::Validate(
				DecalDocument, ValidationError))
		{
			strOutError = "valid World Mark Decal was rejected: " +
				ValidationError;
			return false;
		}
		const std::string DecalJson =
			Client::CEffectDocumentCodec::Serialize(DecalDocument);
		Client::EFFECT_DOCUMENT_DESC DecalRoundTrip;
		if (DecalJson.find(
				"\"receiverMode\": \"upwardSurfaces\"") ==
				std::string::npos ||
			!Client::CEffectDocumentCodec::Parse(
				DecalJson, DecalRoundTrip, strOutError) ||
			DecalRoundTrip.Elements.front().Detail.Decal.fNormalCutoff !=
				0.75f ||
			DecalRoundTrip.Elements.front().Detail.Decal.fEdgeFade != 0.05f)
		{
			strOutError =
				"World Mark Decal receiver did not survive codec round-trip";
			return false;
		}
		Client::EFFECT_EVALUATED_ELEMENT EvaluatedDecal{};
		EvaluatedDecal.pElement = &DecalRoundTrip.Elements.front();
		XMStoreFloat4x4(&EvaluatedDecal.World, XMMatrixIdentity());
		Client::EFFECT_DECAL_SHADER_PROJECTION_DESC Projection;
		if (!Client::CEffectDocumentRenderer::Resolve_DecalShaderProjection(
				EvaluatedDecal, Projection) ||
			Projection.vSize.x != 3.f || Projection.vSize.y != 4.f ||
			Projection.fDepth != 0.25f ||
			Projection.fNormalCutoff != 0.75f ||
			Projection.fEdgeFade != 0.05f ||
			Projection.vUp.x != 0.f || Projection.vUp.y != 1.f ||
			Projection.vUp.z != 0.f)
		{
			strOutError =
				"World Mark Decal receiver changed at the shader projection seam";
			return false;
		}
		Invalid = DecalRoundTrip;
		Invalid.Elements.front().Detail.Decal.eReceiverMode =
			Client::EFFECT_DECAL_RECEIVER_MODE::ALL_OPAQUE;
		if (!Reject(Invalid, "all-opaque Decal with upward cutoff"))
			return false;
		Invalid = DecalRoundTrip;
		Invalid.Elements.front().Detail.Decal.fEdgeFade = 1.001f;
		if (!Reject(Invalid, "Decal edge fade overflow"))
			return false;
		Invalid = WorldMarkRoundTrip;
		Invalid.Elements.front().Detail.Decal.eReceiverMode =
			Client::EFFECT_DECAL_RECEIVER_MODE::UPWARD_SURFACES;
		Invalid.Elements.front().Detail.Decal.fNormalCutoff = 0.75f;
		if (!Reject(Invalid, "Sprite Particle Decal receiver"))
			return false;

		const std::filesystem::path AuthoredRoot =
			RepositoryRoot / L"Data" / L"Effects" / L"Authored";
		struct AUTHORED_WORLD_MARK_EXPECTATION final
		{
			const wchar_t* pFileName;
			std::vector<std::string_view> ElementIds;
		};
		const std::array<AUTHORED_WORLD_MARK_EXPECTATION, 3u> Expectations = {{
			{
				L"effect.warlord.skill.17240.ba3.unified.effect.json",
				{ "sprite_particle_13",
				  "authored.copy.sprite_particle_13.1" }
			},
			{
				L"effect.warlord.skill.17110.clip2.unified.effect.json",
				{ "sprite_particle_3" }
			},
			{
				L"effect.warlord.skill.17110.clip3.unified.effect.json",
				{ "authored.source-decal.f11325615bf47eea65604126",
				  "sprite_particle_3" }
			}
		}};
		for (const AUTHORED_WORLD_MARK_EXPECTATION& Expectation :
			Expectations)
		{
			Client::EFFECT_DOCUMENT_DESC Authored;
			if (!Client::CEffectDocumentCodec::Load(
					AuthoredRoot / Expectation.pFileName,
					Authored, strOutError))
			{
				strOutError =
					"Warlord World Mark authored fixture failed to load: " +
					strOutError;
				return false;
			}
			const size_t iWorldMarkCount = static_cast<size_t>(std::count_if(
				Authored.Elements.begin(), Authored.Elements.end(),
				[](const Client::EFFECT_ELEMENT_DESC& Element)
				{
					return Element.eCompositionLayer ==
						Client::EFFECT_COMPOSITION_LAYER::WORLD_MARK;
				}));
			if (iWorldMarkCount != Expectation.ElementIds.size())
			{
				strOutError =
					"Warlord authored World Mark denominator changed";
				return false;
			}
			for (const std::string_view strElementId :
				Expectation.ElementIds)
			{
				const auto Element = std::find_if(
					Authored.Elements.begin(), Authored.Elements.end(),
					[strElementId](const Client::EFFECT_ELEMENT_DESC& Candidate)
					{
						return Candidate.strElementId == strElementId;
					});
				if (Element == Authored.Elements.end() ||
					Element->eCompositionLayer !=
						Client::EFFECT_COMPOSITION_LAYER::WORLD_MARK)
				{
					strOutError =
						"Warlord authored World Mark stable ID is missing";
					return false;
				}
			}
			if (std::wstring_view(Expectation.pFileName).find(L"clip3") !=
				std::wstring_view::npos)
			{
				const auto SourceDecal = std::find_if(
					Authored.Elements.begin(), Authored.Elements.end(),
					[](const Client::EFFECT_ELEMENT_DESC& Element)
					{
						return Element.strElementId ==
							"authored.source-decal.f11325615bf47eea65604126";
					});
				if (SourceDecal == Authored.Elements.end() ||
					SourceDecal->Detail.Decal.eReceiverMode !=
						Client::EFFECT_DECAL_RECEIVER_MODE::UPWARD_SURFACES ||
					SourceDecal->Detail.Decal.fNormalCutoff != 0.75f ||
					SourceDecal->Detail.Decal.fEdgeFade != 0.05f ||
					SourceDecal->Detail.Decal.fDepth != 0.25f)
				{
					strOutError =
						"Warlord R source Decal receiver contract changed";
					return false;
				}
			}
		}

		OutWorldMarkDocument =
			std::make_shared<Client::EFFECT_DOCUMENT_DESC>(
				std::move(WorldMarkRoundTrip));
		strOutError.clear();
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
				",\"sourceMaterialProfile\":" << Row.iSourceMaterialProfile <<
				",\"firstSubmittedWorldFnv1a64\":" <<
					Row.iFirstSubmittedWorldHash <<
				",\"hasFirstSubmittedWorld\":" <<
					(Row.bHasFirstSubmittedWorld ? "true" : "false") <<
				",\"drawSelectionDiverged\":" <<
					(Row.bDrawSelectionDiverged ? "true" : "false") << "}";
		}
		std::cout << ",\"boundAdapterActualPipelineValidated\":" <<
			(Frame.bBoundAdapterActualPipelineValidated ? "true" : "false");
		if (Frame.bSceneHdrReadback)
		{
			std::cout << ",\"sceneHdr\":{\"bloomThreshold\":" <<
				Frame.fSceneHdrBloomThreshold << ",\"peak\":" <<
				Frame.fSceneHdrPeak << ",\"peakPixel\":[" <<
				Frame.vSceneHdrPeakPixel.x << ',' <<
				Frame.vSceneHdrPeakPixel.y << ',' <<
				Frame.vSceneHdrPeakPixel.z << ',' <<
				Frame.vSceneHdrPeakPixel.w <<
				"],\"pixelsAboveThreshold\":" <<
				Frame.iSceneHdrPixelsAboveThreshold << '}';
		}
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

	bool_t Is_BoundAdapterCanaryOccurrence(
		const OCCURRENCE_EVIDENCE& Occurrence,
		const std::string_view strElementId,
		const Client::EFFECT_GPU_RENDER_CARRIER eCarrier,
		const uint32_t iPassIndex)
	{
		return Occurrence.strElementId == strElementId &&
			Occurrence.iActive == 1u && Occurrence.iAttempted == 1u &&
			Occurrence.iSubmitted == 1u && Occurrence.iSuppressed == 0u &&
			Occurrence.iFailed == 0u && Occurrence.iMaterialBindCount >= 1u &&
			Occurrence.iTextureSrvBindCount >= 1u &&
			Occurrence.iSamplerBindCount >= 1u &&
			Occurrence.iShaderPassApplyCount >= 1u &&
			Occurrence.iVIBufferBindCount >= 1u &&
			Occurrence.iVIBufferDrawCount >= 1u &&
			Occurrence.iIssuedDrawCallCount >= 1u &&
			Occurrence.iDrawSelectionCount >= 1u &&
			Occurrence.iCompiledAdapterPipelineValidationCount >= 1u &&
			Occurrence.eCarrier == eCarrier &&
			Occurrence.iSelectedPassIndex == iPassIndex &&
			!Occurrence.bDrawSelectionDiverged;
	}

	bool_t Is_UnboundAdapterCanaryOccurrence(
		const OCCURRENCE_EVIDENCE& Occurrence,
		const std::string_view strElementId,
		const Client::EFFECT_GPU_RENDER_CARRIER eCarrier,
		const uint32_t iPassIndex)
	{
		return Occurrence.strElementId == strElementId &&
			Occurrence.iActive == 1u && Occurrence.iAttempted == 1u &&
			Occurrence.iSubmitted == 1u && Occurrence.iSuppressed == 0u &&
			Occurrence.iFailed == 0u && Occurrence.iMaterialBindCount >= 1u &&
			Occurrence.iTextureSrvBindCount >= 1u &&
			Occurrence.iSamplerBindCount >= 1u &&
			Occurrence.iShaderPassApplyCount >= 1u &&
			Occurrence.iVIBufferBindCount >= 1u &&
			Occurrence.iVIBufferDrawCount >= 1u &&
			Occurrence.iIssuedDrawCallCount >= 1u &&
			Occurrence.iDrawSelectionCount >= 1u &&
			Occurrence.iCompiledAdapterPipelineValidationCount == 0u &&
			Occurrence.eCarrier == eCarrier &&
			Occurrence.iSelectedPassIndex == iPassIndex &&
			Occurrence.bHasFirstSubmittedWorld &&
			!Occurrence.bDrawSelectionDiverged;
	}

	bool_t Is_Binding0Binding1DrawEquivalent(
		const OCCURRENCE_EVIDENCE& Unbound,
		const OCCURRENCE_EVIDENCE& Bound)
	{
		return Unbound.strElementId == Bound.strElementId &&
			Unbound.iActive == Bound.iActive &&
			Unbound.iCandidate == Bound.iCandidate &&
			Unbound.iAttempted == Bound.iAttempted &&
			Unbound.iSubmitted == Bound.iSubmitted &&
			Unbound.iSuppressed == Bound.iSuppressed &&
			Unbound.iFailed == Bound.iFailed &&
			Unbound.iMaterialBindCount == Bound.iMaterialBindCount &&
			Unbound.iTextureSrvBindCount == Bound.iTextureSrvBindCount &&
			Unbound.iSamplerBindCount == Bound.iSamplerBindCount &&
			Unbound.iShaderPassApplyCount == Bound.iShaderPassApplyCount &&
			Unbound.iVIBufferBindCount == Bound.iVIBufferBindCount &&
			Unbound.iVIBufferDrawCount == Bound.iVIBufferDrawCount &&
			Unbound.iIssuedDrawCallCount == Bound.iIssuedDrawCallCount &&
			Unbound.iDrawSelectionCount == Bound.iDrawSelectionCount &&
			Unbound.iCompiledAdapterPipelineValidationCount == 0u &&
			Bound.iCompiledAdapterPipelineValidationCount > 0u &&
			Bound.iCompiledAdapterPipelineValidationCount ==
				Bound.iIssuedDrawCallCount &&
			Unbound.eCarrier == Bound.eCarrier &&
			Unbound.iSelectedPassIndex == Bound.iSelectedPassIndex &&
			Unbound.bHasFirstSubmittedWorld && Bound.bHasFirstSubmittedWorld &&
			Unbound.iFirstSubmittedWorldHash ==
				Bound.iFirstSubmittedWorldHash &&
			!Unbound.bDrawSelectionDiverged &&
			!Bound.bDrawSelectionDiverged;
	}

	bool_t Is_RepresentativeV1EffectAssetId(
		const std::string_view strEffectAssetId)
	{
		return std::find(REPRESENTATIVE_V1_EFFECT_IDS.begin(),
			REPRESENTATIVE_V1_EFFECT_IDS.end(), strEffectAssetId) !=
			REPRESENTATIVE_V1_EFFECT_IDS.end();
	}

	bool_t Validate_GenericCompiledAdapterMetadata(
		const Client::EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING& Binding,
		std::string& strOutError)
	{
		using ADAPTER_ID = Client::EFFECT_COMPILED_MATERIAL_ADAPTER_ID;
		using CARRIER = Client::EFFECT_COMPILED_MATERIAL_CARRIER;
		using PROFILE = Client::EFFECT_RENDER_PROFILE;
		const Client::EFFECT_COMPILED_MATERIAL_ADAPTER_DESC& Adapter =
			Binding.Adapter;
		std::string_view strExpectedAdapterId;
		std::string_view strExpectedShaderId;
		std::string_view strExpectedVertexLayoutId;
		ADAPTER_ID eExpectedAdapterId = ADAPTER_ID::END;
		switch (Adapter.eCarrier)
		{
		case CARRIER::SPRITE_PARTICLE:
			strExpectedShaderId = "Shader_VtxEffectParticle.hlsl";
			strExpectedVertexLayoutId = "VTXEFFECT_PARTICLE";
			switch (Adapter.eRenderProfile)
			{
			case PROFILE::ALPHA_TWO_SIDED_DEPTH_READ:
				eExpectedAdapterId = ADAPTER_ID::
					SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1;
				strExpectedAdapterId =
					Client::EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ADAPTER_ID;
				break;
			case PROFILE::ALPHA_ONE_SIDED_DEPTH_READ:
				eExpectedAdapterId = ADAPTER_ID::
					SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_ONE_SIDED_V1;
				strExpectedAdapterId =
					Client::EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ALPHA_ONE_SIDED_ADAPTER_ID;
				break;
			case PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ:
				eExpectedAdapterId = ADAPTER_ID::
					SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ADDITIVE_TWO_SIDED_V1;
				strExpectedAdapterId =
					Client::EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ADDITIVE_TWO_SIDED_ADAPTER_ID;
				break;
			case PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ:
				eExpectedAdapterId = ADAPTER_ID::
					SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ADDITIVE_ONE_SIDED_V1;
				strExpectedAdapterId =
					Client::EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ADDITIVE_ONE_SIDED_ADAPTER_ID;
				break;
			case PROFILE::OPAQUE_BACK_DEPTH_WRITE:
			case PROFILE::END:
			default:
				break;
			}
			break;
		case CARRIER::MESH_PARTICLE_CMODEL:
			strExpectedShaderId = "Shader_VtxEffectMeshPreview.hlsl";
			strExpectedVertexLayoutId = "VTXMESH";
			if (Adapter.eRenderProfile == PROFILE::ALPHA_TWO_SIDED_DEPTH_READ)
			{
				eExpectedAdapterId = ADAPTER_ID::
					MESH_PARTICLE_CMODEL_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1;
				strExpectedAdapterId = Client::
					EFFECT_MESH_PARTICLE_SCENE_COLOR_ALPHA_TWO_SIDED_ADAPTER_ID;
			}
			else if (Adapter.eRenderProfile ==
				PROFILE::ALPHA_ONE_SIDED_DEPTH_READ)
			{
				eExpectedAdapterId = ADAPTER_ID::
					MESH_PARTICLE_CMODEL_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_ONE_SIDED_V1;
				strExpectedAdapterId = Client::
					EFFECT_MESH_PARTICLE_SCENE_COLOR_ALPHA_ONE_SIDED_ADAPTER_ID;
			}
			break;
		case CARRIER::LOCAL_DECAL_PROJECTOR:
			strExpectedShaderId = "Shader_VtxEffectDecal.hlsl";
			strExpectedVertexLayoutId = "VTXTEX";
			if (Adapter.eRenderProfile == PROFILE::ALPHA_TWO_SIDED_DEPTH_READ)
			{
				eExpectedAdapterId = ADAPTER_ID::
					LOCAL_DECAL_PROJECTOR_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1;
				strExpectedAdapterId = Client::
					EFFECT_LOCAL_DECAL_SCENE_COLOR_ALPHA_TWO_SIDED_ADAPTER_ID;
			}
			else if (Adapter.eRenderProfile ==
				PROFILE::ALPHA_ONE_SIDED_DEPTH_READ)
			{
				eExpectedAdapterId = ADAPTER_ID::
					LOCAL_DECAL_PROJECTOR_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_ONE_SIDED_V1;
				strExpectedAdapterId = Client::
					EFFECT_LOCAL_DECAL_SCENE_COLOR_ALPHA_ONE_SIDED_ADAPTER_ID;
			}
			break;
		case CARRIER::END:
		default:
			break;
		}

		uint32_t iExpectedPassIndex = UINT32_MAX;
		std::string_view strExpectedRasterizer;
		std::string_view strExpectedDepthStencil;
		std::string_view strExpectedBlend;
		switch (Adapter.eRenderProfile)
		{
		case PROFILE::ALPHA_TWO_SIDED_DEPTH_READ:
			iExpectedPassIndex = 1u;
			strExpectedRasterizer = "RS_Cull_None";
			strExpectedDepthStencil = Adapter.eCarrier ==
				CARRIER::LOCAL_DECAL_PROJECTOR ? "DSS_ZNone" : "DSS_ReadOnly";
			strExpectedBlend = "BS_EffectAlpha";
			break;
		case PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ:
			iExpectedPassIndex = 2u;
			strExpectedRasterizer = "RS_Cull_None";
			strExpectedDepthStencil = "DSS_ReadOnly";
			strExpectedBlend = "BS_EffectAdditive";
			break;
		case PROFILE::ALPHA_ONE_SIDED_DEPTH_READ:
			iExpectedPassIndex = 3u;
			strExpectedRasterizer = "RS_Default";
			strExpectedDepthStencil = "DSS_ReadOnly";
			strExpectedBlend = "BS_EffectAlpha";
			break;
		case PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ:
			iExpectedPassIndex = 4u;
			strExpectedRasterizer = "RS_Default";
			strExpectedDepthStencil = "DSS_ReadOnly";
			strExpectedBlend = "BS_EffectAdditive";
			break;
		case PROFILE::OPAQUE_BACK_DEPTH_WRITE:
		case PROFILE::END:
		default:
			break;
		}

		const bool_t bValid = eExpectedAdapterId != ADAPTER_ID::END &&
			Adapter.eAdapterId == eExpectedAdapterId &&
			Adapter.strAdapterId == strExpectedAdapterId &&
			Binding.strAdapterId == strExpectedAdapterId &&
			Adapter.strShaderId == strExpectedShaderId &&
			Adapter.strVertexLayoutId == strExpectedVertexLayoutId &&
			Adapter.iPassIndex == iExpectedPassIndex &&
			Adapter.strMrtId == "MRT_SceneHDR" &&
			Adapter.iSceneColorRenderTargetIndex == 0u &&
			Adapter.strSceneColorSemantic == "SV_TARGET0" &&
			Adapter.iDistortionRenderTargetIndex == 1u &&
			Adapter.strDistortionSemantic == "SV_TARGET1" &&
			Adapter.bDistortionDeterministicZero &&
			Adapter.strRasterizerState == strExpectedRasterizer &&
			Adapter.strDepthStencilState == strExpectedDepthStencil &&
			Adapter.strBlendState == strExpectedBlend &&
			Adapter.iStencilReference == 0u &&
			Binding.Execution.iPassIndex == Adapter.iPassIndex &&
			Binding.Execution.strRasterizerState == Adapter.strRasterizerState &&
			Binding.Execution.strDepthStencilState ==
				Adapter.strDepthStencilState &&
			Binding.Execution.strBlendState == Adapter.strBlendState &&
			Binding.Execution.iStencilReference == Adapter.iStencilReference;
		if (!bValid)
		{
			strOutError = "generic compiled Program/Layout/Adapter metadata changed: " +
				Binding.strEffectAssetId + "/" + Binding.strElementId;
			return false;
		}
		return true;
	}

	bool_t Validate_RepresentativeV1RegistryReceipts(
		const std::shared_ptr<const Client::CEffectMaterialProgramRegistry>& Registry,
		const uint64_t iCatalogRevision,
		size_t& iOutRepresentativeBindingCount,
		size_t& iOutCoveredAdapterCount,
		std::string& strOutError)
	{
		iOutRepresentativeBindingCount = 0u;
		iOutCoveredAdapterCount = 0u;
		if (nullptr == Registry)
		{
			strOutError = "representative V1 registry is unavailable";
			return false;
		}
		constexpr size_t ADAPTER_COUNT =
			GOLDEN_AND_REPRESENTATIVE_COMPILED_ADAPTER_COUNT;
		std::array<bool_t, ADAPTER_COUNT> CoveredAdapters{};
		for (const Client::EFFECT_MATERIAL_PROGRAM_BINDING_TARGET& Target :
			Registry->Get_BindingTargets())
		{
			if (!Is_RepresentativeV1EffectAssetId(Target.strEffectAssetId))
				continue;

			const auto Binding = Registry->Resolve(
				Target.strEffectAssetId, Target.strElementId);
			if (nullptr == Binding ||
				Binding->iCatalogRevision != iCatalogRevision ||
				Binding->iRegistryGenerationId != Registry->Get_GenerationId() ||
				Binding->strEffectAssetId != Target.strEffectAssetId ||
				Binding->strElementId != Target.strElementId ||
				Binding->eInlineMirrorPolicy != Client::
					EFFECT_MATERIAL_INLINE_MIRROR_POLICY::INLINE_MIRROR_REQUIRED ||
				!Validate_GenericCompiledAdapterMetadata(*Binding, strOutError))
			{
				if (strOutError.empty())
					strOutError = "material-program target failed generic adapter audit";
				return false;
			}
			const size_t iAdapter =
				static_cast<size_t>(Binding->Adapter.eAdapterId);
			if (iAdapter >= CoveredAdapters.size())
			{
				strOutError = "material-program target selected an invalid adapter enum";
				return false;
			}
			CoveredAdapters[iAdapter] = true;

			const auto Document = Client::CEffectCatalog::Find(Target.strEffectAssetId);
			if (nullptr == Document)
			{
				strOutError = "representative V1 document is unavailable: " +
					Target.strEffectAssetId;
				return false;
			}
			const auto Element = std::find_if(
				Document->Elements.begin(), Document->Elements.end(),
				[&Target](const Client::EFFECT_ELEMENT_DESC& Candidate)
				{
					return Candidate.strElementId == Target.strElementId;
				});
			const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution =
				Binding->Execution;
			const Client::EFFECT_STANDARD_COLOR_V1_DESC& Packet =
				Execution.StandardColorV1;
			const bool_t bDissolve = Execution.iTextureLaneCount == 3u;
			if (Element == Document->Elements.end() ||
				!Client::CEffectMaterialProgramRegistry::Is_ExecutionBitExact(
					Execution, Element->Material.Execution) ||
				Client::Get_EffectAuthoringFidelity(Execution) != Client::
					EFFECT_AUTHORING_FIDELITY::APPROXIMATE ||
				Client::Get_EffectAuthoringFidelity(
					Element->Material.Execution) != Client::
					EFFECT_AUTHORING_FIDELITY::APPROXIMATE ||
				Execution.eBackend != Client::
					EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 ||
				Execution.iOpcode != 1u ||
				(Execution.iTextureLaneCount != 2u && !bDissolve) ||
				Packet.iPacketVersion != 1u ||
				Packet.strBaseRadianceLaneId != "lane.0" ||
				Packet.strCoverageLaneId != "lane.1" ||
				Packet.eEmissiveMode != Client::
					EFFECT_STANDARD_COLOR_EMISSIVE_MODE::BASE_RADIANCE ||
				Packet.eLifetimeEnvelope != Client::
					EFFECT_STANDARD_COLOR_LIFETIME_ENVELOPE::CARRIER_ALPHA ||
				Packet.eMissingLanePolicy != Client::
					EFFECT_STANDARD_COLOR_MISSING_LANE_POLICY::FAIL_CLOSED ||
				(bDissolve &&
				 (Packet.eDissolveMode != Client::
					EFFECT_STANDARD_COLOR_DISSOLVE_MODE::LANE_THRESHOLD ||
				  Packet.strDissolveLaneId != "lane.2" ||
				  std::bit_cast<uint32_t>(Packet.fDissolveSoftness) !=
					std::bit_cast<uint32_t>(0.1f))) ||
				(!bDissolve &&
				 (Packet.eDissolveMode != Client::
					EFFECT_STANDARD_COLOR_DISSOLVE_MODE::NONE ||
				  !Packet.strDissolveLaneId.empty() ||
				  Packet.eDissolveChannel != Client::
					EFFECT_STANDARD_COLOR_CHANNEL::INVALID ||
				  std::bit_cast<uint32_t>(Packet.fDissolveSoftness) != 0u)))
			{
				strOutError = "representative V1 exact dual-resolve changed: " +
					Target.strEffectAssetId + "/" + Target.strElementId;
				return false;
			}
			++iOutRepresentativeBindingCount;
		}
		for (const bool_t bCovered : CoveredAdapters)
		{
			if (bCovered)
				++iOutCoveredAdapterCount;
		}
		if (iOutRepresentativeBindingCount != REPRESENTATIVE_V1_BINDING_COUNT ||
			iOutCoveredAdapterCount != REPRESENTATIVE_V1_REGISTRY_ADAPTER_COUNT)
		{
			strOutError = "representative V1 binding or compiled-adapter coverage count changed";
			return false;
		}
		strOutError.clear();
		return true;
	}

	struct VALTAN_COMBAT_OBJECT_DRAW_ROW final
	{
		std::string strElementId;
		std::string strFamily;
		uint64_t iConfigured = 0u;
		uint64_t iAttempted = 0u;
		uint64_t iSubmitted = 0u;
		uint64_t iCommitted = 0u;
		uint64_t iSuppressed = 0u;
		uint64_t iFailed = 0u;
		float3_t vPositionMin{};
		float3_t vPositionMax{};
		bool_t bHasSubmittedPosition = false;
	};

	struct VALTAN_COMBAT_OBJECT_DRAW_SAMPLE final
	{
		std::string strLabel;
		f32_t fRequestedSeconds = 0.f;
		f32_t fEvaluatedSeconds = 0.f;
		float3_t vRootPosition{};
		std::vector<VALTAN_COMBAT_OBJECT_DRAW_ROW> Rows;
		float3_t vSubmittedPositionMin{};
		float3_t vSubmittedPositionMax{};
		bool_t bHasSubmittedBounds = false;
		bool_t bTransactionCommitted = false;
	};

	struct VALTAN_COMBAT_OBJECT_ELEMENT_TOTAL final
	{
		std::string strElementId;
		std::string strFamily;
		uint64_t iPreparedSamples = 0u;
		uint64_t iAttemptedSamples = 0u;
		uint64_t iSubmittedDraws = 0u;
		uint64_t iCommittedDraws = 0u;
		uint64_t iSuppressedDraws = 0u;
		uint64_t iFailedDraws = 0u;
	};

	struct VALTAN_COMBAT_OBJECT_TARGET_PROOF final
	{
		std::string strCombatObjectArchetypeId;
		std::string strClientVisualId;
		std::string strEffectAssetId;
		std::filesystem::path DocumentPath;
		std::string strDocumentRawSha256;
		std::string strDocumentTypedCodecSha256;
		std::string strRootPolicy;
		f32_t fEffectDurationSeconds = 0.f;
		uint32_t iRootWorldDistinctCount = 0u;
		uint32_t iSubmittedBoundsDistinctCount = 0u;
		uint64_t iUnauditedSubmittedDraws = 0u;
		std::vector<std::string> AuditedElementIds;
		std::vector<VALTAN_COMBAT_OBJECT_DRAW_SAMPLE> Samples;
		std::vector<VALTAN_COMBAT_OBJECT_ELEMENT_TOTAL> ElementTotals;
	};

	const char* Get_ValtanCombatObjectProofFamilyLabel(
		const Client::EFFECT_GPU_RENDER_FAMILY eFamily)
	{
		switch (eFamily)
		{
		case Client::EFFECT_GPU_RENDER_FAMILY::MESH:
			return "MESH";
		case Client::EFFECT_GPU_RENDER_FAMILY::SPRITE:
			return "SPRITE";
		case Client::EFFECT_GPU_RENDER_FAMILY::DECAL:
			return "DECAL";
		case Client::EFFECT_GPU_RENDER_FAMILY::RIBBON:
			return "RIBBON";
		default:
			return "END";
		}
	}

	bool_t Is_FiniteValtanCombatObjectProofVector(const float3_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z);
	}

	bool_t Is_FiniteValtanCombatObjectProofFrame(
		const Client::EFFECT_EVALUATED_FRAME& Frame)
	{
		const f32_t* pRoot = &Frame.RootWorld._11;
		if (!std::isfinite(Frame.fSampleTimeSeconds) ||
			!std::all_of(pRoot, pRoot + 16u,
				[](const f32_t Value) { return std::isfinite(Value); }))
		{
			return false;
		}
		for (const Client::EFFECT_EVALUATED_ELEMENT& Element : Frame.Elements)
		{
			const f32_t* pWorld = &Element.World._11;
			if (nullptr == Element.pElement ||
				!std::all_of(pWorld, pWorld + 16u,
					[](const f32_t Value) { return std::isfinite(Value); }) ||
				!std::isfinite(Element.fLocalTimeSeconds) ||
				!std::isfinite(Element.fNormalizedLife))
			{
				return false;
			}
		}
		for (const Client::EFFECT_EVALUATED_PARTICLE& Particle : Frame.Particles)
		{
			const f32_t* pWorld = &Particle.World._11;
			if (nullptr == Particle.pElement ||
				!std::all_of(pWorld, pWorld + 16u,
					[](const f32_t Value) { return std::isfinite(Value); }) ||
				!Is_FiniteValtanCombatObjectProofVector(
					Particle.vWorldVelocity) ||
				!std::isfinite(Particle.fNormalizedLife))
			{
				return false;
			}
		}
		return true;
	}

	bool_t Read_ValtanProofFile(const std::filesystem::path& Path,
		std::string& OutBytes, std::string& strOutSha256,
		std::string& strOutError)
	{
		std::ifstream Input(Path, std::ios::binary);
		if (!Input)
		{
			strOutError = "Valtan combat-object proof input is unavailable: " +
				Path.string();
			return false;
		}
		OutBytes.assign(std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>());
		if (OutBytes.empty())
		{
			strOutError = "Valtan combat-object proof input is empty: " +
				Path.string();
			return false;
		}
		strOutSha256 = Client::CEffectRuntimeAuthorityCodec::
			Compute_Sha256Hex(OutBytes);
		if (strOutSha256.size() != 64u)
		{
			strOutError = "Valtan combat-object proof input hash failed: " +
				Path.string();
			return false;
		}
		strOutError.clear();
		return true;
	}

	bool_t Read_ValtanSkyAxeOwnerLifetimeSeconds(
		const std::string_view CombatObjectCatalogJson,
		f32_t& fOutSeconds, std::string& strOutError)
	{
		Client::DATA_JSON_VALUE Root;
		Client::DATA_JSON_PARSE_LIMITS Limits;
		Limits.iMaximumBytes = 4u * 1024u * 1024u;
		Limits.iMaximumDepth = 32u;
		Limits.iMaximumValues = 100'000u;
		if (!Client::CDataJson::Parse(
				CombatObjectCatalogJson, Root, strOutError, Limits) ||
			!Root.Is_Object())
		{
			strOutError = "Valtan combat-object owner catalog parse failed: " +
				strOutError;
			return false;
		}
		const Client::DATA_JSON_VALUE* pObjects = Root.Find("objects");
		if (nullptr == pObjects || !pObjects->Is_Array())
		{
			strOutError = "Valtan combat-object owner catalog objects are invalid";
			return false;
		}
		const Client::DATA_JSON_VALUE* pMatch = nullptr;
		for (const Client::DATA_JSON_VALUE& Object : pObjects->Get_Array())
		{
			const Client::DATA_JSON_VALUE* pId = Object.Find(
				"combatObjectArchetypeId");
			if (Object.Is_Object() && nullptr != pId && pId->Is_String() &&
				pId->Get_String() ==
					"combatobject.valtan.high-jump.target-axe")
			{
				if (nullptr != pMatch)
				{
					strOutError = "Valtan sky-axe Product owner is duplicated";
					return false;
				}
				pMatch = &Object;
			}
		}
		const Client::DATA_JSON_VALUE* pLifeMs = nullptr == pMatch ? nullptr :
			pMatch->Find("lifeMs");
		if (nullptr == pLifeMs || !pLifeMs->Is_Number() ||
			pLifeMs->Was_FloatingPointToken() ||
			pLifeMs->Get_Number() != 6000.0)
		{
			strOutError = "Valtan sky-axe Product owner lifeMs is not 6000";
			return false;
		}
		fOutSeconds = static_cast<f32_t>(pLifeMs->Get_Number() / 1000.0);
		strOutError.clear();
		return true;
	}

	bool_t Write_ValtanCombatObjectRuntimeSweep(
		const std::filesystem::path& OutputPath,
		const std::filesystem::path& ResourceRoot,
		const std::filesystem::path& RuntimeCatalogPath,
		const std::string_view strRuntimeCatalogRawSha256,
		const std::filesystem::path& BossCatalogPath,
		const std::string_view strBossCatalogRawSha256,
		const std::filesystem::path& CombatObjectCatalogPath,
		const std::string_view strCombatObjectCatalogRawSha256,
		const f32_t fOwnerLifetimeSeconds,
		const std::vector<VALTAN_COMBAT_OBJECT_TARGET_PROOF>& Targets,
		std::string& strOutError)
	{
		const auto WriteVector = [](std::ostringstream& Json,
			const float3_t& Value)
		{
			Json << '[' << std::setprecision(
				std::numeric_limits<float>::max_digits10) << Value.x << ", " <<
				Value.y << ", " << Value.z << ']';
		};
		std::ostringstream Json;
		Json << "{\n"
			<< "  \"schema\": \"lostark.valtan-combat-object-runtime-sweep\",\n"
			<< "  \"formatVersion\": 1,\n"
			<< "  \"bossArchetypeId\": \"BOSS_VALTAN\",\n"
			<< "  \"renderer\": {\"driver\": \"D3D_DRIVER_TYPE_WARP\", "
				"\"vendorId\": 5140, \"deviceId\": 140},\n"
			<< "  \"resourceRoot\": \"" << Client::CDataJson::Escape(
				ResourceRoot.generic_string()) << "\",\n"
			<< "  \"runtimeCatalog\": {\"path\": \"" <<
				Client::CDataJson::Escape(RuntimeCatalogPath.generic_string()) <<
				"\", \"rawSha256\": \"" << strRuntimeCatalogRawSha256 << "\"},\n"
			<< "  \"bossCatalog\": {\"path\": \"" <<
				Client::CDataJson::Escape(BossCatalogPath.generic_string()) <<
				"\", \"rawSha256\": \"" << strBossCatalogRawSha256 << "\"},\n"
			<< "  \"combatObjectCatalog\": {\"path\": \"" <<
				Client::CDataJson::Escape(
					CombatObjectCatalogPath.generic_string()) <<
				"\", \"rawSha256\": \"" <<
				strCombatObjectCatalogRawSha256 << "\"},\n"
			<< "  \"producerHarness\": {\n"
			<< "    \"mode\": \"" << VALTAN_COMBAT_OBJECT_SWEEP_MODE << "\",\n"
			<< "    \"runtimeCatalogVerified\": true,\n"
			<< "    \"auditedTargetCount\": 2,\n"
			<< "    \"auditedElementCount\": 10,\n"
			<< "    \"failureCount\": 0\n"
			<< "  },\n"
			<< "  \"targets\": [\n";
		for (size_t iTarget = 0u; iTarget < Targets.size(); ++iTarget)
		{
			const VALTAN_COMBAT_OBJECT_TARGET_PROOF& Target = Targets[iTarget];
			Json << "    {\n"
				<< "      \"combatObjectArchetypeId\": \"" <<
					Client::CDataJson::Escape(
						Target.strCombatObjectArchetypeId) << "\",\n"
				<< "      \"clientVisualId\": \"" <<
					Client::CDataJson::Escape(Target.strClientVisualId) << "\",\n"
				<< "      \"effectAssetId\": \"" <<
					Client::CDataJson::Escape(Target.strEffectAssetId) << "\",\n"
				<< "      \"documentPath\": \"" <<
					Client::CDataJson::Escape(
						Target.DocumentPath.generic_string()) << "\",\n"
				<< "      \"documentRawSha256\": \"" <<
					Target.strDocumentRawSha256 << "\",\n"
				<< "      \"documentTypedCodecSha256\": \"" <<
					Target.strDocumentTypedCodecSha256 << "\",\n"
				<< "      \"rootPolicy\": \"" << Target.strRootPolicy << "\",\n"
				<< "      \"effectDurationSeconds\": " <<
					std::setprecision(std::numeric_limits<float>::max_digits10) <<
					Target.fEffectDurationSeconds << ",\n"
				<< "      \"rootWorldDistinctCount\": " <<
					Target.iRootWorldDistinctCount << ",\n"
				<< "      \"submittedBoundsDistinctCount\": " <<
					Target.iSubmittedBoundsDistinctCount << ",\n"
				<< "      \"unauditedSubmittedDraws\": " <<
					Target.iUnauditedSubmittedDraws << ",\n"
				<< "      \"auditedElementIds\": [";
			for (size_t iElement = 0u;
				iElement < Target.AuditedElementIds.size(); ++iElement)
			{
				Json << '"' << Client::CDataJson::Escape(
					Target.AuditedElementIds[iElement]) << '"' <<
					(iElement + 1u < Target.AuditedElementIds.size() ? ", " : "");
			}
			Json << "],\n      \"samples\": [\n";
			for (size_t iSample = 0u; iSample < Target.Samples.size(); ++iSample)
			{
				const VALTAN_COMBAT_OBJECT_DRAW_SAMPLE& Sample =
					Target.Samples[iSample];
				Json << "        {\n"
					<< "          \"label\": \"" <<
						Client::CDataJson::Escape(Sample.strLabel) << "\",\n"
					<< "          \"requestedSeconds\": " <<
						std::setprecision(
							std::numeric_limits<float>::max_digits10) <<
						Sample.fRequestedSeconds << ",\n"
					<< "          \"evaluatedSeconds\": " <<
						Sample.fEvaluatedSeconds << ",\n"
					<< "          \"rootPosition\": ";
				WriteVector(Json, Sample.vRootPosition);
				Json << ",\n          \"hasSubmittedBounds\": " <<
					(Sample.bHasSubmittedBounds ? "true" : "false") <<
					",\n          \"submittedPositionMin\": ";
				WriteVector(Json, Sample.vSubmittedPositionMin);
				Json << ",\n          \"submittedPositionMax\": ";
				WriteVector(Json, Sample.vSubmittedPositionMax);
				Json << ",\n          \"transactionCommitted\": " <<
					(Sample.bTransactionCommitted ? "true" : "false") <<
					",\n          \"rows\": [\n";
				for (size_t iRow = 0u; iRow < Sample.Rows.size(); ++iRow)
				{
					const VALTAN_COMBAT_OBJECT_DRAW_ROW& Row = Sample.Rows[iRow];
					Json << "            {\"elementId\": \"" <<
						Client::CDataJson::Escape(Row.strElementId) <<
						"\", \"family\": \"" << Row.strFamily <<
						"\", \"configured\": " << Row.iConfigured <<
						", \"attempted\": " << Row.iAttempted <<
						", \"submitted\": " << Row.iSubmitted <<
						", \"committed\": " << Row.iCommitted <<
						", \"suppressed\": " << Row.iSuppressed <<
						", \"failed\": " << Row.iFailed <<
						", \"hasSubmittedPosition\": " <<
						(Row.bHasSubmittedPosition ? "true" : "false") <<
						", \"positionMin\": ";
					WriteVector(Json, Row.vPositionMin);
					Json << ", \"positionMax\": ";
					WriteVector(Json, Row.vPositionMax);
					Json << '}' << (iRow + 1u < Sample.Rows.size() ? "," : "") <<
						'\n';
				}
				Json << "          ]\n        }" <<
					(iSample + 1u < Target.Samples.size() ? "," : "") << '\n';
			}
			Json << "      ],\n      \"elementTotals\": [\n";
			for (size_t iElement = 0u;
				iElement < Target.ElementTotals.size(); ++iElement)
			{
				const VALTAN_COMBAT_OBJECT_ELEMENT_TOTAL& Element =
					Target.ElementTotals[iElement];
				Json << "        {\"elementId\": \"" <<
					Client::CDataJson::Escape(Element.strElementId) <<
					"\", \"family\": \"" << Element.strFamily <<
					"\", \"preparedSamples\": " << Element.iPreparedSamples <<
					", \"attemptedSamples\": " << Element.iAttemptedSamples <<
					", \"submittedDraws\": " << Element.iSubmittedDraws <<
					", \"committedDraws\": " << Element.iCommittedDraws <<
					", \"suppressedDraws\": " << Element.iSuppressedDraws <<
					", \"failedDraws\": " << Element.iFailedDraws << '}' <<
					(iElement + 1u < Target.ElementTotals.size() ? "," : "") <<
					'\n';
			}
			Json << "      ]\n    }" <<
				(iTarget + 1u < Targets.size() ? "," : "") << '\n';
		}
		Json << "  ],\n"
			<< "  \"ownerLifetimeBoundary\": {\n"
			<< "    \"combatObjectArchetypeId\": "
				"\"combatobject.valtan.high-jump.target-axe\",\n"
			<< "    \"seconds\": " << fOwnerLifetimeSeconds << ",\n"
			<< "    \"rendererSampleAttempted\": false,\n"
			<< "    \"productOwnerDurationVerified\": true\n"
			<< "  },\n"
			<< "  \"disposition\": \"DRAWABLE_PROOF_PASS\"\n"
			<< "}\n";

		std::error_code Error;
		const std::filesystem::path AbsoluteOutput = std::filesystem::absolute(
			OutputPath, Error).lexically_normal();
		if (Error || !AbsoluteOutput.is_absolute() ||
			!std::filesystem::is_directory(AbsoluteOutput.parent_path(), Error) ||
			Error)
		{
			strOutError = "Valtan combat-object sweep output parent is invalid";
			return false;
		}
		std::filesystem::path StagingPath = AbsoluteOutput;
		StagingPath += L".staging." + std::to_wstring(GetCurrentProcessId());
		std::filesystem::remove(StagingPath, Error);
		Error.clear();
		{
			std::ofstream Stream(StagingPath, std::ios::binary | std::ios::trunc);
			if (!Stream)
			{
				strOutError =
					"unable to create Valtan combat-object sweep staging file";
				return false;
			}
			const std::string Payload = Json.str();
			Stream.write(Payload.data(),
				static_cast<std::streamsize>(Payload.size()));
			Stream.flush();
			if (!Stream)
			{
				Stream.close();
				std::filesystem::remove(StagingPath, Error);
				strOutError =
					"unable to flush Valtan combat-object sweep staging file";
				return false;
			}
		}
		if (!MoveFileExW(StagingPath.c_str(), AbsoluteOutput.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		{
			std::filesystem::remove(StagingPath, Error);
			strOutError =
				"unable to atomically commit Valtan combat-object runtime sweep";
			return false;
		}
		strOutError.clear();
		return true;
	}

	bool_t Produce_ValtanCombatObjectRuntimeSweep(
		const std::filesystem::path& RepositoryRoot,
		const std::filesystem::path& RuntimeCatalogPath,
		const std::filesystem::path& ResourceRoot,
		const std::filesystem::path& OutputPath,
		std::string& strOutError)
	{
		using namespace Client;
		struct SAMPLE_CONFIG final
		{
			const char* pLabel = nullptr;
			f32_t fRequestedSeconds = 0.f;
			float3_t vRootPosition{};
		};
		const std::array<SAMPLE_CONFIG, 5u> SkySamples = {{
			{ "warning-midlife", 0.5f, { 2.f, 0.f, 4.f } },
			{ "descent-flight", 1.05f, { 2.f, 0.f, 4.f } },
			{ "impact-start", 1.2f, { 2.f, 0.f, 4.f } },
			{ "impact-midlife", 1.5f, { 2.f, 0.f, 4.f } },
			{ "effect-tail-end", 2.8f, { 2.f, 0.f, 4.f } }
		}};
		constexpr f32_t RED_SPEED_METRES_PER_SECOND = 24.444445f;
		const std::array<SAMPLE_CONFIG, 4u> RedSamples = {{
			{ "spawn-zero-step", 0.f, { 0.f, 0.f, 4.f } },
			{ "first-fixed-step", 1.f / 60.f,
				{ RED_SPEED_METRES_PER_SECOND / 60.f, 0.f, 4.f } },
			{ "late-initial-seek-mid-flight", 0.45f,
				{ RED_SPEED_METRES_PER_SECOND * 0.45f, 0.f, 4.f } },
			{ "pre-despawn", 0.899f,
				{ RED_SPEED_METRES_PER_SECOND * 0.899f, 0.f, 4.f } }
		}};

		const std::filesystem::path BossCatalogPath =
			std::filesystem::absolute(RepositoryRoot /
				L"Data/Actors/BossCatalog.json").lexically_normal();
		const std::filesystem::path CombatObjectCatalogPath =
			std::filesystem::absolute(RepositoryRoot /
				L"Data/Encounters/Valtan/ValtanCombatObjects.json").lexically_normal();
		std::string RuntimeCatalogRaw;
		std::string RuntimeCatalogRawSha256;
		std::string BossCatalogRaw;
		std::string BossCatalogRawSha256;
		std::string CombatObjectCatalogRaw;
		std::string CombatObjectCatalogRawSha256;
		f32_t fOwnerLifetimeSeconds = 0.f;
		if (!Read_ValtanProofFile(RuntimeCatalogPath, RuntimeCatalogRaw,
				RuntimeCatalogRawSha256, strOutError) ||
			!Read_ValtanProofFile(BossCatalogPath, BossCatalogRaw,
				BossCatalogRawSha256, strOutError) ||
			!Read_ValtanProofFile(CombatObjectCatalogPath, CombatObjectCatalogRaw,
				CombatObjectCatalogRawSha256, strOutError) ||
			!Read_ValtanSkyAxeOwnerLifetimeSeconds(
				CombatObjectCatalogRaw, fOwnerLifetimeSeconds, strOutError) ||
			std::abs(fOwnerLifetimeSeconds -
				VALTAN_SKY_AXE_OWNER_LIFETIME_SECONDS) > 0.0001f)
		{
			return false;
		}

		HEADLESS_ENGINE_SCOPE EngineScope;
		if (!EngineScope.Initialize(strOutError))
			return false;
		const ComPtr<ID3D11Device> Device = EngineScope.Get_Device();
		const ComPtr<ID3D11DeviceContext> Context = EngineScope.Get_Context();

		const auto BuildTarget = [&](const std::string_view strArchetypeId,
			const std::string_view strVisualId,
			const std::string_view strEffectAssetId,
			const std::filesystem::path& DocumentPath,
			const std::span<const std::string_view> ElementIds,
			const std::span<const SAMPLE_CONFIG> Samples,
			const std::string_view strRootPolicy,
			VALTAN_COMBAT_OBJECT_TARGET_PROOF& OutProof) -> bool_t
		{
			EFFECT_DOCUMENT_DESC AuthoredDocument;
			const std::shared_ptr<const EFFECT_DOCUMENT_DESC> RuntimeDocument =
				CEffectCatalog::Find(std::string(strEffectAssetId));
			if (!CEffectDocumentCodec::Load(
					DocumentPath, AuthoredDocument, strOutError) ||
				!CEffectDocumentCodec::Validate_Drawable(
					AuthoredDocument, strOutError) ||
				nullptr == RuntimeDocument ||
				AuthoredDocument.strEffectAssetId != strEffectAssetId ||
				RuntimeDocument->strEffectAssetId != strEffectAssetId)
			{
				strOutError = DocumentPath.string() + ": " + strOutError;
				return false;
			}
			std::string Raw;
			std::string RawSha256;
			std::string TypedStatus;
			const std::string AuthoredTypedSha256 =
				CEffectVisualProgramCorpusCodec::Compute_DocumentCanonicalSha256(
					AuthoredDocument, TypedStatus);
			const std::string RuntimeTypedSha256 =
				CEffectVisualProgramCorpusCodec::Compute_DocumentCanonicalSha256(
					*RuntimeDocument, TypedStatus);
			if (!Read_ValtanProofFile(
					DocumentPath, Raw, RawSha256, strOutError) ||
				AuthoredTypedSha256.size() != 64u ||
				RuntimeTypedSha256 != AuthoredTypedSha256 ||
				RuntimeDocument->Elements.size() != ElementIds.size() ||
				AuthoredDocument.Elements.size() != ElementIds.size())
			{
				if (strOutError.empty())
					strOutError =
						"Product Effect is not the current Authored source document";
				return false;
			}
			std::unordered_set<std::string> ExpectedElements;
			for (size_t iElement = 0u; iElement < ElementIds.size(); ++iElement)
			{
				if (RuntimeDocument->Elements[iElement].strElementId !=
						ElementIds[iElement] ||
					AuthoredDocument.Elements[iElement].strElementId !=
						ElementIds[iElement] ||
					!ExpectedElements.emplace(ElementIds[iElement]).second)
				{
					strOutError =
						"Valtan combat-object current element denominator changed";
					return false;
				}
			}

			CEffectPlayback Playback;
			CEffectDocumentRenderer Renderer(Device, Context);
			if (FAILED(Renderer.Initialize()) ||
				!Playback.Stage_Document(*RuntimeDocument, strOutError) ||
				!Renderer.Stage_Document(*RuntimeDocument, strOutError))
			{
				strOutError = DocumentPath.string() + ": " + strOutError;
				return false;
			}
			OutProof = {};
			OutProof.strCombatObjectArchetypeId = strArchetypeId;
			OutProof.strClientVisualId = strVisualId;
			OutProof.strEffectAssetId = strEffectAssetId;
			OutProof.DocumentPath = std::filesystem::absolute(
				DocumentPath).lexically_normal();
			OutProof.strDocumentRawSha256 = RawSha256;
			OutProof.strDocumentTypedCodecSha256 = AuthoredTypedSha256;
			OutProof.strRootPolicy = strRootPolicy;
			OutProof.fEffectDurationSeconds = Playback.Get_DurationSeconds();
			for (const std::string_view ElementId : ElementIds)
			{
				OutProof.AuditedElementIds.emplace_back(ElementId);
				VALTAN_COMBAT_OBJECT_ELEMENT_TOTAL Total;
				Total.strElementId = ElementId;
				OutProof.ElementTotals.push_back(std::move(Total));
			}
			std::vector<float3_t> DistinctRoots;
			std::vector<std::pair<float3_t, float3_t>> DistinctBounds;
			for (const SAMPLE_CONFIG& Config : Samples)
			{
				float4x4_t RootWorld{};
				XMStoreFloat4x4(&RootWorld, XMMatrixTranslation(
					Config.vRootPosition.x, Config.vRootPosition.y,
					Config.vRootPosition.z));
				Playback.Seek(Config.fRequestedSeconds, RootWorld);
				const EFFECT_EVALUATED_FRAME& Frame = Playback.Get_Frame();
				if (!Is_FiniteValtanCombatObjectProofFrame(Frame) ||
					!EngineScope.Begin_Frame(strOutError))
				{
					return false;
				}
				const HRESULT RenderResult = Renderer.Render(Frame);
				Context->Flush();
				const EFFECT_GPU_RENDER_SUBMISSION_STATS& Stats =
					Renderer.Get_LastRenderSubmissionStats();
				if (FAILED(RenderResult) ||
					S_OK != Device->GetDeviceRemovedReason() ||
					!Stats.bCompleted || !Stats.bCommitted)
				{
					strOutError =
						"Valtan combat-object renderer transaction did not commit";
					return false;
				}

				VALTAN_COMBAT_OBJECT_DRAW_SAMPLE Sample;
				Sample.strLabel = Config.pLabel;
				Sample.fRequestedSeconds = Config.fRequestedSeconds;
				Sample.fEvaluatedSeconds = Frame.fSampleTimeSeconds;
				Sample.vRootPosition = Config.vRootPosition;
				Sample.bTransactionCommitted = true;
				const bool_t bRootAlreadyCounted = std::any_of(
					DistinctRoots.begin(), DistinctRoots.end(),
					[&Config](const float3_t& Root)
					{
						return XMVectorGetX(XMVector3LengthSq(
							XMLoadFloat3(&Root) -
							XMLoadFloat3(&Config.vRootPosition))) <= 1.0e-10f;
					});
				if (!bRootAlreadyCounted)
					DistinctRoots.push_back(Config.vRootPosition);
				for (const std::string_view ElementId : ElementIds)
				{
					const auto Row = std::find_if(Stats.Occurrences.begin(),
						Stats.Occurrences.end(), [ElementId](
							const EFFECT_GPU_RENDER_OCCURRENCE_STATS& Candidate)
						{
							return Candidate.strElementId == ElementId;
						});
					if (Row == Stats.Occurrences.end())
					{
						strOutError =
							"Valtan combat-object renderer omitted an occurrence";
						return false;
					}
					VALTAN_COMBAT_OBJECT_DRAW_ROW Draw;
					Draw.strElementId = ElementId;
					Draw.strFamily =
						Get_ValtanCombatObjectProofFamilyLabel(Row->eFamily);
					Draw.iConfigured = Row->iConfigured;
					Draw.iAttempted = Row->iAttempted;
					Draw.iSubmitted = Row->iSubmitted;
					Draw.iCommitted = Row->iIssuedDrawCallCount;
					Draw.iSuppressed = Row->iSuppressed;
					Draw.iFailed = Row->iFailed;
					Draw.vPositionMin = Row->vSubmittedPositionMin;
					Draw.vPositionMax = Row->vSubmittedPositionMax;
					Draw.bHasSubmittedPosition = Row->bHasSubmittedPosition;
					if (Draw.strFamily == "END" || 0u != Draw.iFailed ||
						((0u < Draw.iSubmitted || 0u < Draw.iCommitted) &&
						 (UINT32_MAX == Row->iSelectedPassIndex ||
						  Row->bSourceMaterialFallbackBlocked ||
						  Row->bDrawSelectionDiverged ||
						  !Draw.bHasSubmittedPosition ||
						  !Is_FiniteValtanCombatObjectProofVector(
							  Draw.vPositionMin) ||
						  !Is_FiniteValtanCombatObjectProofVector(
							  Draw.vPositionMax))))
					{
						strOutError =
							"Valtan combat-object renderer selected an invalid draw";
						return false;
					}
					VALTAN_COMBAT_OBJECT_ELEMENT_TOTAL& Total =
						OutProof.ElementTotals[Sample.Rows.size()];
					if (0u < Draw.iConfigured)
						++Total.iPreparedSamples;
					if (0u < Draw.iAttempted)
						++Total.iAttemptedSamples;
					Total.strFamily = Draw.strFamily;
					Total.iSubmittedDraws += Draw.iSubmitted;
					Total.iCommittedDraws += Draw.iCommitted;
					Total.iSuppressedDraws += Draw.iSuppressed;
					Total.iFailedDraws += Draw.iFailed;
					if (Draw.bHasSubmittedPosition)
					{
						if (!Sample.bHasSubmittedBounds)
						{
							Sample.vSubmittedPositionMin = Draw.vPositionMin;
							Sample.vSubmittedPositionMax = Draw.vPositionMax;
							Sample.bHasSubmittedBounds = true;
						}
						else
						{
							Sample.vSubmittedPositionMin.x = (std::min)(
								Sample.vSubmittedPositionMin.x, Draw.vPositionMin.x);
							Sample.vSubmittedPositionMin.y = (std::min)(
								Sample.vSubmittedPositionMin.y, Draw.vPositionMin.y);
							Sample.vSubmittedPositionMin.z = (std::min)(
								Sample.vSubmittedPositionMin.z, Draw.vPositionMin.z);
							Sample.vSubmittedPositionMax.x = (std::max)(
								Sample.vSubmittedPositionMax.x, Draw.vPositionMax.x);
							Sample.vSubmittedPositionMax.y = (std::max)(
								Sample.vSubmittedPositionMax.y, Draw.vPositionMax.y);
							Sample.vSubmittedPositionMax.z = (std::max)(
								Sample.vSubmittedPositionMax.z, Draw.vPositionMax.z);
						}
					}
					Sample.Rows.push_back(std::move(Draw));
				}
				for (const EFFECT_GPU_RENDER_OCCURRENCE_STATS& Row :
					Stats.Occurrences)
				{
					if (!ExpectedElements.contains(Row.strElementId))
						OutProof.iUnauditedSubmittedDraws += Row.iSubmitted;
				}
				if (Sample.bHasSubmittedBounds)
				{
					const bool_t bKnown = std::any_of(DistinctBounds.begin(),
						DistinctBounds.end(), [&Sample](const auto& Bounds)
						{
							const float3_t DeltaMin(
								Bounds.first.x - Sample.vSubmittedPositionMin.x,
								Bounds.first.y - Sample.vSubmittedPositionMin.y,
								Bounds.first.z - Sample.vSubmittedPositionMin.z);
							const float3_t DeltaMax(
								Bounds.second.x - Sample.vSubmittedPositionMax.x,
								Bounds.second.y - Sample.vSubmittedPositionMax.y,
								Bounds.second.z - Sample.vSubmittedPositionMax.z);
							return XMVectorGetX(XMVector3LengthSq(
								XMLoadFloat3(&DeltaMin))) <= 1.0e-10f &&
								XMVectorGetX(XMVector3LengthSq(
									XMLoadFloat3(&DeltaMax))) <= 1.0e-10f;
						});
					if (!bKnown)
						DistinctBounds.emplace_back(
							Sample.vSubmittedPositionMin,
							Sample.vSubmittedPositionMax);
				}
				OutProof.Samples.push_back(std::move(Sample));
			}
			OutProof.iRootWorldDistinctCount =
				static_cast<uint32_t>(DistinctRoots.size());
			OutProof.iSubmittedBoundsDistinctCount =
				static_cast<uint32_t>(DistinctBounds.size());
			return true;
		};

		const std::filesystem::path SkyPath = RepositoryRoot /
			L"Data/Effects/Authored/effect.valtan.sky-axe.active.effect.json";
		const std::filesystem::path RedPath = RepositoryRoot /
			L"Data/Effects/Authored/effect.valtan.red-blade-wave.active.effect.json";
		std::vector<VALTAN_COMBAT_OBJECT_TARGET_PROOF> Targets(2u);
		if (!BuildTarget("combatobject.valtan.high-jump.target-axe",
				"combatobject.visual.valtan.high-jump.target-axe.v1",
				"effect.valtan.sky-axe.active", SkyPath,
				VALTAN_SKY_AXE_ELEMENTS, SkySamples, "FIXED_WORLD_ROOT",
				Targets[0u]) ||
			!BuildTarget("combatobject.valtan.red-blade-wave.projectile",
				"combatobject.visual.valtan.red-blade-wave.projectile.v1",
				"effect.valtan.red-blade-wave.active", RedPath,
				VALTAN_RED_BLADE_ELEMENTS, RedSamples, "MOVING_WORLD_ROOT",
				Targets[1u]))
		{
			return false;
		}

		const auto HasDraw = [](const VALTAN_COMBAT_OBJECT_DRAW_SAMPLE& Sample,
			const std::string_view ElementId)
		{
			const auto Row = std::find_if(Sample.Rows.begin(), Sample.Rows.end(),
				[ElementId](const VALTAN_COMBAT_OBJECT_DRAW_ROW& Candidate)
				{
					return Candidate.strElementId == ElementId;
				});
			return Row != Sample.Rows.end() && 0u < Row->iAttempted &&
				0u < Row->iSubmitted && 0u < Row->iCommitted &&
				0u == Row->iFailed && Row->bHasSubmittedPosition;
		};
		const auto HasNoDraw = [](const VALTAN_COMBAT_OBJECT_DRAW_SAMPLE& Sample,
			const std::string_view ElementId)
		{
			const auto Row = std::find_if(Sample.Rows.begin(), Sample.Rows.end(),
				[ElementId](const VALTAN_COMBAT_OBJECT_DRAW_ROW& Candidate)
				{
					return Candidate.strElementId == ElementId;
				});
			return Row != Sample.Rows.end() && 0u == Row->iSubmitted &&
				0u == Row->iCommitted && 0u == Row->iFailed;
		};
		const VALTAN_COMBAT_OBJECT_TARGET_PROOF& Sky = Targets[0u];
		const VALTAN_COMBAT_OBJECT_TARGET_PROOF& Red = Targets[1u];
		bool_t bSkyExact = Sky.Samples.size() == 5u &&
			Sky.iRootWorldDistinctCount == 1u &&
			HasDraw(Sky.Samples[0u], VALTAN_SKY_AXE_ELEMENTS[2u]) &&
			HasDraw(Sky.Samples[0u], VALTAN_SKY_AXE_ELEMENTS[3u]) &&
			HasDraw(Sky.Samples[1u], VALTAN_SKY_AXE_ELEMENTS[0u]) &&
			HasDraw(Sky.Samples[1u], VALTAN_SKY_AXE_ELEMENTS[4u]) &&
			HasDraw(Sky.Samples[2u], VALTAN_SKY_AXE_ELEMENTS[1u]);
		for (const std::string_view ElementId : VALTAN_SKY_AXE_ELEMENTS)
			bSkyExact = bSkyExact && HasNoDraw(Sky.Samples[4u], ElementId);
		for (const VALTAN_COMBAT_OBJECT_ELEMENT_TOTAL& Total : Sky.ElementTotals)
		{
			bSkyExact = bSkyExact && 0u < Total.iPreparedSamples &&
				0u < Total.iAttemptedSamples && 0u < Total.iSubmittedDraws &&
				0u < Total.iCommittedDraws && 0u == Total.iFailedDraws;
		}
		bool_t bRedExact = Red.Samples.size() == 4u &&
			Red.iRootWorldDistinctCount == 4u &&
			Red.iSubmittedBoundsDistinctCount >= 2u;
		for (size_t iElement = 0u;
			iElement < VALTAN_RED_BLADE_ELEMENTS.size(); ++iElement)
		{
			const VALTAN_COMBAT_OBJECT_ELEMENT_TOTAL& Total =
				Red.ElementTotals[iElement];
			bRedExact = bRedExact && Total.strFamily == "SPRITE" &&
				0u < Total.iPreparedSamples && 0u < Total.iAttemptedSamples &&
				0u < Total.iSubmittedDraws && 0u < Total.iCommittedDraws &&
				0u == Total.iFailedDraws &&
				HasNoDraw(Red.Samples[0u], VALTAN_RED_BLADE_ELEMENTS[iElement]) &&
				HasDraw(Red.Samples[2u], VALTAN_RED_BLADE_ELEMENTS[iElement]);
		}
		if (!bSkyExact || !bRedExact)
		{
			std::ostringstream Detail;
			Detail << "Valtan current combat-object WARP proof did not close"
				<< " sky=" << bSkyExact << " red=" << bRedExact;
			for (const VALTAN_COMBAT_OBJECT_TARGET_PROOF* pTarget : { &Sky, &Red })
			{
				for (const VALTAN_COMBAT_OBJECT_DRAW_SAMPLE& Sample : pTarget->Samples)
				{
					Detail << " | " << Sample.strLabel << '@' <<
						Sample.fEvaluatedSeconds;
					for (const VALTAN_COMBAT_OBJECT_DRAW_ROW& Row : Sample.Rows)
					{
						Detail << ' ' << Row.strElementId << "=(" << Row.strFamily <<
							",a" << Row.iAttempted << ",s" << Row.iSubmitted <<
							",c" << Row.iCommitted << ",f" << Row.iFailed << ')';
					}
				}
			}
			strOutError = Detail.str();
			return false;
		}

		return Write_ValtanCombatObjectRuntimeSweep(OutputPath, ResourceRoot,
			RuntimeCatalogPath, RuntimeCatalogRawSha256, BossCatalogPath,
			BossCatalogRawSha256, CombatObjectCatalogPath,
			CombatObjectCatalogRawSha256, fOwnerLifetimeSeconds, Targets,
			strOutError);
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
	if (argc != 2 || nullptr == argv[1])
	{
		std::cerr << "usage: EffectRenderContractHarness <repo-root>\n";
		return 2;
	}

	constexpr unsigned long ExpectedBindingCount = 0u;

	std::error_code FileError;
	const std::filesystem::path RepositoryRoot =
		std::filesystem::weakly_canonical(argv[1], FileError);
	const std::filesystem::path ClientWorkingDirectory =
		RepositoryRoot / L"Client" / L"Default";
	const std::filesystem::path ArtistFullCandidatePath =
		RepositoryRoot / L"Data" / L"Effects" / L"Imported" / L"Artist" /
			L"Candidates" /
			L"skill.31470.reconstructed-runtime-program.candidate.json";
	const std::filesystem::path ResourceRoot =
		RepositoryRoot / L"Client" / L"Bin" / L"Resources";
	if (FileError ||
		!std::filesystem::is_directory(ClientWorkingDirectory, FileError) ||
		FileError || !std::filesystem::is_regular_file(
			ArtistFullCandidatePath, FileError) ||
		FileError || !std::filesystem::is_directory(ResourceRoot, FileError) ||
		FileError)
	{
		std::cerr << "required repository runtime inputs are unavailable\n";
		return 2;
	}

	std::string Status;
	Write_Progress("direct-authored-index.begin");
	if (!Validate_DirectAuthoredAuditionSourceIndex(RepositoryRoot, Status))
	{
		std::cerr << Status << '\n';
		return 1;
	}
	Write_Progress("direct-authored-index.complete");
	Write_Progress("source-screen-overlay-parser.begin");
	if (!Validate_SourceScreenOverlayParserContract(RepositoryRoot, Status))
	{
		std::cerr << Status << '\n';
		return 1;
	}
	Write_Progress("source-screen-overlay-parser.complete");

	if (!Set_EnvironmentPath(L"LOSTARK_RESOURCE_ROOT", ResourceRoot, Status) ||
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

	Write_Progress("source-playback-tuning.begin");
	if (!Validate_SourcePlaybackTuningContract(RepositoryRoot, Status))
	{
		std::cerr << Status << '\n';
		return 1;
	}
	Write_Progress("source-playback-tuning.complete");
	Write_Progress("artist-particlemaster-hdr-emissive.begin");
	if (!Validate_ArtistParticleMasterHdrEmissiveContract(
			RepositoryRoot, Status))
	{
		std::cerr << Status << '\n';
		return 1;
	}
	Write_Progress("artist-particlemaster-hdr-emissive.complete");
	Write_Progress("transform-motion-hold.begin");
	if (!Validate_TransformMotionHoldContract(RepositoryRoot, Status))
	{
		std::cerr << Status << '\n';
		return 1;
	}
	Write_Progress("transform-motion-hold.complete");
	Write_Progress("manual-mesh-particle-scale.begin");
	if (!Validate_ManualMeshParticleScaleComposition(Status))
	{
		std::cerr << Status << '\n';
		return 1;
	}
	Write_Progress("manual-mesh-particle-scale.complete");
	std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC> RingFillContractDocument;
	Write_Progress("even-ring-orientation-ring-fill.begin");
	if (!Validate_EvenRingOrientationAndMeshRingFillContract(
			RingFillContractDocument, Status))
	{
		std::cerr << Status << '\n';
		return 1;
	}
	Write_Progress("even-ring-orientation-ring-fill.complete");
	std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>
		LinearRevealContractDocument;
	Write_Progress("generic-sprite-linear-reveal.begin");
	if (!Validate_GenericSpriteLinearRevealContract(
			LinearRevealContractDocument, Status))
	{
		std::cerr << Status << '\n';
		return 1;
	}
	Write_Progress("generic-sprite-linear-reveal.complete");
	std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>
		WorldMarkContractDocument;
	Write_Progress("world-mark-composition.begin");
	if (!Validate_WorldMarkCompositionContract(
			RepositoryRoot, WorldMarkContractDocument, Status))
	{
		std::cerr << Status << '\n';
		return 1;
	}
	Write_Progress("world-mark-composition.complete");

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
	Write_Progress("source-direct-product-identity.begin");
	if (!Validate_SourceDirectProductIdentity(RepositoryRoot, Status))
	{
		std::cerr << Status << '\n';
		return 1;
	}
	Write_Progress("source-direct-product-identity.complete");
	Write_Progress("artist-particlemaster-source-hdr-emissive.begin");
	if (!Validate_ArtistParticleMasterSourceHdrEmissiveContract(Status))
	{
		std::cerr << Status << '\n';
		return 1;
	}
	Write_Progress("artist-particlemaster-source-hdr-emissive.complete");

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
	const std::shared_ptr<const Client::EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING>
		MeshCanaryBinding = Registry->Resolve(
			ARTIST_UNIFIED_EFFECT_ID, ARTIST_MESH_CANARY_ELEMENT_ID);
	const std::shared_ptr<const Client::EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING>
		DecalCanaryBinding = Registry->Resolve(
			ARTIST_UNIFIED_EFFECT_ID, ARTIST_DECAL_CANARY_ELEMENT_ID);
	const bool_t bExpectedCanaryBinding = ExpectedBindingCount > 0u;
	const bool_t bExpectedRepresentativeV1 =
		ExpectedBindingCount >= REPRESENTATIVE_V1_TOTAL_BINDING_COUNT;
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
		 CanaryBinding->eInlineMirrorPolicy ==
			 Client::EFFECT_MATERIAL_INLINE_MIRROR_POLICY::
				 INLINE_MIRROR_REQUIRED &&
		 Client::Get_EffectAuthoringFidelity(CanaryBinding->Execution) ==
			 Client::EFFECT_AUTHORING_FIDELITY::EXACT &&
		 CanaryBinding->Adapter.eAdapterId ==
			 Client::EFFECT_COMPILED_MATERIAL_ADAPTER_ID::
			 SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1 &&
		 CanaryBinding->Adapter.eCarrier ==
			 Client::EFFECT_COMPILED_MATERIAL_CARRIER::SPRITE_PARTICLE &&
		 CanaryBinding->Adapter.strAdapterId ==
			 Client::EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ADAPTER_ID &&
		 CanaryBinding->Adapter.strShaderId == "Shader_VtxEffectParticle.hlsl" &&
		 CanaryBinding->Adapter.strVertexLayoutId == "VTXEFFECT_PARTICLE" &&
		 CanaryBinding->Adapter.eRenderProfile ==
			 Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ &&
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
	const bool_t bMeshCanaryBindingExact = bExpectedCanaryBinding ?
		(nullptr != MeshCanaryBinding &&
		 MeshCanaryBinding->iCatalogRevision == CatalogRevision &&
		 MeshCanaryBinding->iRegistryGenerationId == Registry->Get_GenerationId() &&
		 MeshCanaryBinding->strEffectAssetId == ARTIST_UNIFIED_EFFECT_ID &&
		 MeshCanaryBinding->strElementId == ARTIST_MESH_CANARY_ELEMENT_ID &&
		 MeshCanaryBinding->strProgramId == ARTIST_MESH_CANARY_PROGRAM_ID &&
		 MeshCanaryBinding->strLayoutId == ARTIST_MESH_CANARY_LAYOUT_ID &&
		 MeshCanaryBinding->strDescriptorId == ARTIST_MESH_CANARY_DESCRIPTOR_ID &&
		 MeshCanaryBinding->strAdapterId ==
			 Client::EFFECT_MESH_PARTICLE_SCENE_COLOR_ALPHA_TWO_SIDED_ADAPTER_ID &&
		 MeshCanaryBinding->eInlineMirrorPolicy ==
			 Client::EFFECT_MATERIAL_INLINE_MIRROR_POLICY::
				 INLINE_MIRROR_REQUIRED &&
		 MeshCanaryBinding->Execution.eBackend ==
			 Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
		 Client::Get_EffectAuthoringFidelity(MeshCanaryBinding->Execution) ==
			 Client::EFFECT_AUTHORING_FIDELITY::EXACT &&
		 MeshCanaryBinding->Execution.iOpcode == 3u &&
		 MeshCanaryBinding->Adapter.eCarrier ==
			 Client::EFFECT_COMPILED_MATERIAL_CARRIER::MESH_PARTICLE_CMODEL &&
		 MeshCanaryBinding->Adapter.eRenderProfile ==
			 Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ &&
		 MeshCanaryBinding->Adapter.iPassIndex == 1u) :
		(nullptr == MeshCanaryBinding);
	const bool_t bDecalCanaryBindingExact = bExpectedCanaryBinding ?
		(nullptr != DecalCanaryBinding &&
		 DecalCanaryBinding->iCatalogRevision == CatalogRevision &&
		 DecalCanaryBinding->iRegistryGenerationId == Registry->Get_GenerationId() &&
		 DecalCanaryBinding->strEffectAssetId == ARTIST_UNIFIED_EFFECT_ID &&
		 DecalCanaryBinding->strElementId == ARTIST_DECAL_CANARY_ELEMENT_ID &&
		 DecalCanaryBinding->strProgramId == ARTIST_DECAL_CANARY_PROGRAM_ID &&
		 DecalCanaryBinding->strLayoutId == ARTIST_DECAL_CANARY_LAYOUT_ID &&
		 DecalCanaryBinding->strDescriptorId == ARTIST_DECAL_CANARY_DESCRIPTOR_ID &&
		 DecalCanaryBinding->strAdapterId ==
			 Client::EFFECT_LOCAL_DECAL_SCENE_COLOR_ALPHA_ONE_SIDED_ADAPTER_ID &&
		 DecalCanaryBinding->eInlineMirrorPolicy ==
			 Client::EFFECT_MATERIAL_INLINE_MIRROR_POLICY::
				 INLINE_MIRROR_REQUIRED &&
		 DecalCanaryBinding->Execution.eBackend ==
			 Client::EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL &&
		 Client::Get_EffectAuthoringFidelity(DecalCanaryBinding->Execution) ==
			 Client::EFFECT_AUTHORING_FIDELITY::EXACT &&
		 DecalCanaryBinding->Execution.iOpcode == 14u &&
		 DecalCanaryBinding->Adapter.eCarrier ==
			 Client::EFFECT_COMPILED_MATERIAL_CARRIER::LOCAL_DECAL_PROJECTOR &&
		 DecalCanaryBinding->Adapter.eRenderProfile ==
			 Client::EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ &&
		 DecalCanaryBinding->Adapter.iPassIndex == 3u) :
		(nullptr == DecalCanaryBinding);
	if (!bCanaryBindingExact || !bMeshCanaryBindingExact ||
		!bDecalCanaryBindingExact)
	{
		std::cerr << "compiled Sprite/Mesh/Decal Binding identity mismatched\n";
		return 1;
	}
	size_t iRepresentativeV1BindingCount = 0u;
	size_t iCoveredCompiledAdapterCount = 0u;
	if (bExpectedRepresentativeV1 &&
		!Validate_RepresentativeV1RegistryReceipts(
			Registry, CatalogRevision, iRepresentativeV1BindingCount,
			iCoveredCompiledAdapterCount, Status))
	{
		std::cerr << "representative V1 registry receipt failed: " <<
			Status << '\n';
		return 1;
	}
	bool_t bCatalogBindingRollbackValidated = false;

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
	size_t iRepresentativeV1ActualDrawCount = 0u;
	std::array<bool_t, GOLDEN_AND_REPRESENTATIVE_COMPILED_ADAPTER_COUNT>
		ActualCompiledAdapterDrawCoverage{};
	size_t iActualCompiledAdapterDrawCount = 0u;

	Client::EFFECT_RENDER_PREWARM_PROBE PrewarmProbe{};
	Client::EFFECT_MODEL_CUE_ANIMATION_PROBE ArtistECraneProbe{};
	bool_t bArtistECraneCloneAnimationValidated = false;
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
		Write_Progress("artist-e-crane-clone-animation.begin");
		if (!Validate_ArtistECraneCloneAnimationContract(
				Device, Context, ArtistECraneProbe, Status))
		{
			std::cerr << Status << '\n';
			return 1;
		}
		bArtistECraneCloneAnimationValidated = true;
		Write_Progress("artist-e-crane-clone-animation.complete");
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

		constexpr f32_t ARTIST_CHARACTER_SELECT_BLOOM_THRESHOLD = 2.74f;
		constexpr std::array<f32_t, 5u> ARTIST_PARTICLE_MASTER_SAMPLE_OFFSETS =
			{ 0.02f, 0.06f, 0.12f, 0.20f, 0.28f };
		std::vector<std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>>
			ArtistParticleMasterFixtures;
		std::vector<std::shared_ptr<const
			Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>>
			ArtistParticleMasterPreparedDocuments;
		std::vector<std::shared_ptr<Client::CEffectObject>>
			ArtistParticleMasterObjects;
		for (const ARTIST_PARTICLE_MASTER_STAGE_DESC& Stage :
			ARTIST_PARTICLE_MASTER_STAGES)
		{
			const std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC> Published =
				Client::CEffectCatalog::Find(std::string(Stage.strEffectAssetId));
			if (nullptr == Published)
			{
				std::cerr << "Artist BA" << Stage.iStageNumber <<
					" ParticleMaster WARP fixture is unavailable\n";
				return 1;
			}
			const auto SourceElement = std::find_if(Published->Elements.begin(),
				Published->Elements.end(), [&Stage](
					const Client::EFFECT_ELEMENT_DESC& Candidate)
				{
					return Candidate.strElementId == Stage.strElementId;
				});
			if (SourceElement == Published->Elements.end())
			{
				std::cerr << "Artist BA" << Stage.iStageNumber <<
					" ParticleMaster WARP occurrence is unavailable\n";
				return 1;
			}

			Client::EFFECT_ELEMENT_DESC TunedElement = *SourceElement;
			TunedElement.Detail.Color.vColorOffset = { 0.f, 0.f, 0.f, 0.f };
			TunedElement.Detail.Color.vColorMultiply = { 1.f, 0.2f, 1.5f, 1.f };
			TunedElement.Detail.Color.fEmissiveIntensity = 5.f;
			TunedElement.Detail.LinearLerp.bColorOffset = false;
			TunedElement.Detail.LinearLerp.bColorMultiply = false;
			TunedElement.Detail.LinearLerp.bEmissiveIntensity = false;
			Client::EFFECT_DOCUMENT_DESC TunedDocument = *Published;
			TunedDocument.ModelCues.clear();
			TunedDocument.Elements.assign(1u, TunedElement);
			const std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC> Fixture =
				std::make_shared<Client::EFFECT_DOCUMENT_DESC>(
					std::move(TunedDocument));
			std::shared_ptr<const
				Client::CEffectDocumentRenderer::PREPARED_DOCUMENT> Prepared;
			Write_Progress("artist-particlemaster-warp-prewarm.begin");
			if (!Client::CEffectDocumentRenderer::
					Prepare_UnboundMaterialProgramDocumentForTests(
						Device, Context, Fixture, nullptr, Prepared, Status) ||
				nullptr == Prepared)
			{
				std::cerr << "Artist BA" << Stage.iStageNumber <<
					" ParticleMaster WARP prewarm failed: " << Status << '\n';
				return 1;
			}
			Write_Progress("artist-particlemaster-warp-prewarm.complete");
			Client::CEffectObject::EFFECT_OBJECT_DESC Desc{};
			Desc.pDocument = Fixture.get();
			Desc.pPreparedResources = Prepared;
			XMStoreFloat4x4(&Desc.RootWorld,
				XMMatrixTranslation(0.f, -2.1f, 0.f));
			Desc.bAutoPlay = false;
			Desc.bRequirePreparedResources = true;
			const std::shared_ptr<Client::CEffectObject> Object =
				Add_EffectObject(PrototypeTag, LayerTag, Desc);
			if (nullptr == Object)
			{
				std::cerr << "Artist BA" << Stage.iStageNumber <<
					" ParticleMaster WARP object staging failed\n";
				return 1;
			}
			ArtistParticleMasterFixtures.emplace_back(Fixture);
			ArtistParticleMasterPreparedDocuments.emplace_back(Prepared);
			ArtistParticleMasterObjects.emplace_back(Object);

			const f32_t fSourceBurstTime = TunedElement.SourceRecipe.Bursts.empty() ?
				0.f : TunedElement.SourceRecipe.Bursts.front().fTimeSeconds;
			const f32_t fSampleOrigin =
				TunedElement.Detail.Timing.fStartDelaySeconds + fSourceBurstTime;
			bool_t bActualDraw = false;
			bool_t bCrossedBloomThreshold = false;
			bool_t bPurplePeak = false;
			f32_t fPeak = 0.f;
			float4_t PeakPixel{};
			uint64_t iPixelsAboveThreshold = 0u;
			for (const f32_t fSampleOffset :
				ARTIST_PARTICLE_MASTER_SAMPLE_OFFSETS)
			{
				const f32_t fSampleTime = fSampleOrigin + fSampleOffset;
				FRAME_EVIDENCE Frame;
				Frame.strScenarioId = "artist-ba" +
					std::to_string(Stage.iStageNumber) +
					"-particlemaster-hdr-" + std::to_string(fSampleTime) + "s";
				Frame.strContract =
					"GAMEINSTANCE_WARP_MRT_SCENE_HDR_HARD_THRESHOLD";
				Write_Progress("artist-particlemaster-warp-render.begin");
				if (!EngineScope.Render_Frame(Object, fSampleTime, Frame, Status,
						ARTIST_CHARACTER_SELECT_BLOOM_THRESHOLD))
				{
					std::cerr << "Artist BA" << Stage.iStageNumber <<
						" ParticleMaster WARP render failed: " << Status << '\n';
					return 1;
				}
				Write_Progress("artist-particlemaster-warp-render.complete");
				Frame.Occurrence = Find_OccurrenceEvidence(
					Object->Get_LastRenderSubmissionStats(), Stage.strElementId);
				const bool_t bFrameActualDraw = Frame.Occurrence.has_value() &&
					Frame.Occurrence->iSubmitted > 0u &&
					Frame.Occurrence->iFailed == 0u &&
					Frame.Occurrence->iMaterialBindCount > 0u &&
					Frame.Occurrence->iShaderPassApplyCount > 0u &&
					Frame.Occurrence->iIssuedDrawCallCount > 0u &&
					Frame.Occurrence->eCarrier ==
						Client::EFFECT_GPU_RENDER_CARRIER::MESH_CMODEL &&
					Frame.Occurrence->iSelectedPassIndex == 1u &&
					!Frame.Occurrence->bDrawSelectionDiverged;
				bActualDraw |= bFrameActualDraw;
				const bool_t bFrameCrossedThreshold = bFrameActualDraw &&
					Frame.bSceneHdrReadback &&
					Frame.fSceneHdrPeak > ARTIST_CHARACTER_SELECT_BLOOM_THRESHOLD &&
					Frame.iSceneHdrPixelsAboveThreshold > 0u;
				bCrossedBloomThreshold |= bFrameCrossedThreshold;
				const float4_t& FramePeakPixel = Frame.vSceneHdrPeakPixel;
				bPurplePeak |= bFrameCrossedThreshold &&
					FramePeakPixel.x >= 2.f * FramePeakPixel.y &&
					FramePeakPixel.z >= 1.25f * FramePeakPixel.x;
				if (Frame.fSceneHdrPeak > fPeak)
				{
					fPeak = Frame.fSceneHdrPeak;
					PeakPixel = FramePeakPixel;
					iPixelsAboveThreshold = Frame.iSceneHdrPixelsAboveThreshold;
				}
				Frames.emplace_back(std::move(Frame));
			}
			if (!bActualDraw || !bCrossedBloomThreshold || !bPurplePeak)
			{
				std::cerr << "Artist BA" << Stage.iStageNumber <<
					" ParticleMaster HDR pixel contract failed [draw=" <<
					(bActualDraw ? "true" : "false") << ", peak=" << fPeak <<
					", peakPixel=[" << PeakPixel.x << ',' << PeakPixel.y << ',' <<
					PeakPixel.z << ',' << PeakPixel.w <<
					"], pixelsAboveThreshold=" << iPixelsAboveThreshold << "]\n";
				return 1;
			}
		}

		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>
			RingFillPrepared;
		Write_Progress("mesh-ring-fill-warp-prewarm.begin");
		if (nullptr == RingFillContractDocument ||
			!Client::CEffectDocumentRenderer::
				Prepare_UnboundMaterialProgramDocumentForTests(
					Device, Context, RingFillContractDocument, nullptr,
					RingFillPrepared, Status))
		{
			std::cerr << "Mesh Ring Fill WARP prewarm failed: " << Status << '\n';
			return 1;
		}
		Write_Progress("mesh-ring-fill-warp-prewarm.complete");
		Client::CEffectObject::EFFECT_OBJECT_DESC RingFillDesc{};
		RingFillDesc.pDocument = RingFillContractDocument.get();
		RingFillDesc.pPreparedResources = RingFillPrepared;
		RingFillDesc.RootWorld = Identity;
		RingFillDesc.bAutoPlay = false;
		RingFillDesc.bRequirePreparedResources = true;
		const std::shared_ptr<Client::CEffectObject> RingFillObject =
			Add_EffectObject(PrototypeTag, LayerTag, RingFillDesc);
		if (nullptr == RingFillObject)
		{
			std::cerr << "Mesh Ring Fill WARP object staging failed\n";
			return 1;
		}
		FRAME_EVIDENCE RingFillFrame;
		RingFillFrame.strScenarioId = "generic-mesh-particle-ring-fill-0.5s";
		RingFillFrame.strContract = "GAMEINSTANCE_WARP_MRT_SCENE_HDR";
		Write_Progress("mesh-ring-fill-warp-render.begin");
		if (!EngineScope.Render_Frame(
				RingFillObject, 0.5f, RingFillFrame, Status))
		{
			std::cerr << "Mesh Ring Fill WARP render failed: " << Status << '\n';
			return 1;
		}
		Write_Progress("mesh-ring-fill-warp-render.complete");
		RingFillFrame.Occurrence = Find_OccurrenceEvidence(
			RingFillObject->Get_LastRenderSubmissionStats(),
			RingFillContractDocument->Elements.front().strElementId);
		const bool_t bRingFillActualDraw =
			RingFillFrame.Occurrence.has_value() &&
			RingFillFrame.Occurrence->iActive == 1u &&
			RingFillFrame.Occurrence->iCandidate == 1u &&
			RingFillFrame.Occurrence->iAttempted == 1u &&
			RingFillFrame.Occurrence->iSubmitted == 1u &&
			RingFillFrame.Occurrence->iSuppressed == 0u &&
			RingFillFrame.Occurrence->iFailed == 0u &&
			RingFillFrame.Occurrence->iMaterialBindCount >= 1u &&
			RingFillFrame.Occurrence->iShaderPassApplyCount >= 1u &&
			RingFillFrame.Occurrence->iVIBufferBindCount >= 1u &&
			RingFillFrame.Occurrence->iVIBufferDrawCount >= 1u &&
			RingFillFrame.Occurrence->iIssuedDrawCallCount >= 1u &&
			RingFillFrame.Occurrence->eCarrier ==
				Client::EFFECT_GPU_RENDER_CARRIER::MESH_CMODEL &&
			!RingFillFrame.Occurrence->bDrawSelectionDiverged;
		if (!bRingFillActualDraw)
		{
			std::cerr <<
				"Mesh Ring Fill did not complete an actual WARP shader-bound draw\n";
			return 1;
		}
		if (!RingFillFrame.Occurrence->bHasFirstSubmittedWorld)
		{
			std::cerr <<
				"Mesh Ring Fill did not expose its submitted world matrix\n";
			return 1;
		}
		const uint64_t iBaselineScaleWorldHash =
			RingFillFrame.Occurrence->iFirstSubmittedWorldHash;

		Client::EFFECT_DOCUMENT_DESC ValueTunedScaleDocument =
			*RingFillContractDocument;
		Client::EFFECT_ELEMENT_DESC& ValueTunedScaleElement =
			ValueTunedScaleDocument.Elements.front();
		ValueTunedScaleElement.Detail.Transform.vScale = { 2.f, 3.f, 4.f };
		ValueTunedScaleElement.Detail.Particle.vStartSize = { 1.25f, 1.5f };
		ValueTunedScaleElement.Detail.Particle.vEndSize = { 1.75f, 2.f };
		const Client::EFFECT_RENDER_PREWARM_PROBE BeforeValueTuning =
			Client::CEffectDocumentRenderer::Get_PrewarmProbe();
		if (!RingFillObject->Stage_Document(ValueTunedScaleDocument, Status))
		{
			std::cerr << "Mesh Particle value-scale restage failed: " <<
				Status << '\n';
			return 1;
		}
		const Client::EFFECT_RENDER_PREWARM_PROBE AfterValueTuning =
			Client::CEffectDocumentRenderer::Get_PrewarmProbe();
		if (AfterValueTuning.iPreparedDocumentBuildCount !=
				BeforeValueTuning.iPreparedDocumentBuildCount ||
			AfterValueTuning.iModelDiskLoadCount !=
				BeforeValueTuning.iModelDiskLoadCount)
		{
			std::cerr <<
				"Transform/Particle Size tuning rebuilt the WModel resource\n";
			return 1;
		}
		FRAME_EVIDENCE ValueTunedScaleFrame;
		if (!EngineScope.Render_Frame(
				RingFillObject, 0.5f, ValueTunedScaleFrame, Status))
		{
			std::cerr << "Mesh Particle value-scale render failed: " <<
				Status << '\n';
			return 1;
		}
		const std::optional<OCCURRENCE_EVIDENCE> ValueTunedScaleOccurrence =
			Find_OccurrenceEvidence(
				RingFillObject->Get_LastRenderSubmissionStats(),
				ValueTunedScaleElement.strElementId);
		if (!ValueTunedScaleOccurrence.has_value() ||
			!ValueTunedScaleOccurrence->bHasFirstSubmittedWorld ||
			ValueTunedScaleOccurrence->iFirstSubmittedWorldHash ==
				iBaselineScaleWorldHash)
		{
			std::cerr <<
				"Transform/Particle Size tuning did not change the submitted world\n";
			return 1;
		}

		Client::EFFECT_DOCUMENT_DESC ImportTunedScaleDocument =
			ValueTunedScaleDocument;
		ImportTunedScaleDocument.Elements.front().Detail.Mesh.fModelPreScale =
			0.02f;
		const Client::EFFECT_RENDER_PREWARM_PROBE BeforeImportTuning =
			Client::CEffectDocumentRenderer::Get_PrewarmProbe();
		if (!RingFillObject->Stage_Document(ImportTunedScaleDocument, Status))
		{
			std::cerr << "Mesh Particle import-scale restage failed: " <<
				Status << '\n';
			return 1;
		}
		const Client::EFFECT_RENDER_PREWARM_PROBE AfterImportTuning =
			Client::CEffectDocumentRenderer::Get_PrewarmProbe();
		if (AfterImportTuning.iPreparedDocumentBuildCount !=
				BeforeImportTuning.iPreparedDocumentBuildCount + 1u ||
			AfterImportTuning.iModelDiskLoadCount !=
				BeforeImportTuning.iModelDiskLoadCount + 1u)
		{
			std::cerr <<
				"Model Import Scale tuning did not rebuild exactly one WModel resource\n";
			return 1;
		}

		Client::EFFECT_DOCUMENT_DESC RetunedScaleDocument =
			ImportTunedScaleDocument;
		RetunedScaleDocument.Elements.front().Detail.Transform.vScale =
			{ 4.f, 5.f, 6.f };
		const Client::EFFECT_RENDER_PREWARM_PROBE BeforeValueRetuning =
			Client::CEffectDocumentRenderer::Get_PrewarmProbe();
		if (!RingFillObject->Stage_Document(RetunedScaleDocument, Status))
		{
			std::cerr << "Mesh Particle scale retune failed: " << Status << '\n';
			return 1;
		}
		const Client::EFFECT_RENDER_PREWARM_PROBE AfterValueRetuning =
			Client::CEffectDocumentRenderer::Get_PrewarmProbe();
		if (AfterValueRetuning.iPreparedDocumentBuildCount !=
				BeforeValueRetuning.iPreparedDocumentBuildCount ||
			AfterValueRetuning.iModelDiskLoadCount !=
				BeforeValueRetuning.iModelDiskLoadCount)
		{
			std::cerr <<
				"Transform scale retune stopped reusing the corrected WModel resource\n";
			return 1;
		}
		Frames.emplace_back(std::move(RingFillFrame));

		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>
			LinearRevealPrepared;
		Write_Progress("sprite-linear-reveal-warp-prewarm.begin");
		if (nullptr == LinearRevealContractDocument ||
			!Client::CEffectDocumentRenderer::
				Prepare_UnboundMaterialProgramDocumentForTests(
					Device, Context, LinearRevealContractDocument, nullptr,
					LinearRevealPrepared, Status))
		{
			std::cerr << "Sprite Linear Reveal WARP prewarm failed: " <<
				Status << '\n';
			return 1;
		}
		Write_Progress("sprite-linear-reveal-warp-prewarm.complete");
		Client::CEffectObject::EFFECT_OBJECT_DESC LinearRevealDesc{};
		LinearRevealDesc.pDocument = LinearRevealContractDocument.get();
		LinearRevealDesc.pPreparedResources = LinearRevealPrepared;
		LinearRevealDesc.RootWorld = Identity;
		LinearRevealDesc.bAutoPlay = false;
		LinearRevealDesc.bRequirePreparedResources = true;
		const std::shared_ptr<Client::CEffectObject> LinearRevealObject =
			Add_EffectObject(PrototypeTag, LayerTag, LinearRevealDesc);
		if (nullptr == LinearRevealObject)
		{
			std::cerr << "Sprite Linear Reveal WARP object staging failed\n";
			return 1;
		}
		FRAME_EVIDENCE LinearRevealFrame;
		LinearRevealFrame.strScenarioId =
			"generic-sprite-particle-linear-reveal-0.375s";
		LinearRevealFrame.strContract = "GAMEINSTANCE_WARP_MRT_SCENE_HDR";
		Write_Progress("sprite-linear-reveal-warp-render.begin");
		if (!EngineScope.Render_Frame(
				LinearRevealObject, 0.375f, LinearRevealFrame, Status))
		{
			std::cerr << "Sprite Linear Reveal WARP render failed: " <<
				Status << '\n';
			return 1;
		}
		Write_Progress("sprite-linear-reveal-warp-render.complete");
		LinearRevealFrame.Occurrence = Find_OccurrenceEvidence(
			LinearRevealObject->Get_LastRenderSubmissionStats(),
			LinearRevealContractDocument->Elements.front().strElementId);
		const bool_t bLinearRevealActualDraw =
			LinearRevealFrame.Occurrence.has_value() &&
			LinearRevealFrame.Occurrence->iActive == 1u &&
			LinearRevealFrame.Occurrence->iCandidate == 1u &&
			LinearRevealFrame.Occurrence->iAttempted == 1u &&
			LinearRevealFrame.Occurrence->iSubmitted == 1u &&
			LinearRevealFrame.Occurrence->iSuppressed == 0u &&
			LinearRevealFrame.Occurrence->iFailed == 0u &&
			LinearRevealFrame.Occurrence->iMaterialBindCount >= 1u &&
			LinearRevealFrame.Occurrence->iShaderPassApplyCount >= 1u &&
			LinearRevealFrame.Occurrence->iVIBufferBindCount >= 1u &&
			LinearRevealFrame.Occurrence->iVIBufferDrawCount >= 1u &&
			LinearRevealFrame.Occurrence->iIssuedDrawCallCount >= 1u &&
			LinearRevealFrame.Occurrence->eCarrier ==
				Client::EFFECT_GPU_RENDER_CARRIER::SPRITE_INSTANCE &&
			!LinearRevealFrame.Occurrence->bDrawSelectionDiverged;
		if (!bLinearRevealActualDraw)
		{
			std::cerr <<
				"Sprite Linear Reveal did not complete an actual WARP shader-bound draw\n";
			return 1;
		}
		Frames.emplace_back(std::move(LinearRevealFrame));

		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>
			WorldMarkPrepared;
		Write_Progress("world-mark-warp-prewarm.begin");
		if (nullptr == WorldMarkContractDocument ||
			!Client::CEffectDocumentRenderer::
				Prepare_UnboundMaterialProgramDocumentForTests(
					Device, Context, WorldMarkContractDocument, nullptr,
					WorldMarkPrepared, Status))
		{
			std::cerr << "World Mark WARP prewarm failed: " <<
				Status << '\n';
			return 1;
		}
		Write_Progress("world-mark-warp-prewarm.complete");
		Client::CEffectObject::EFFECT_OBJECT_DESC WorldMarkDesc{};
		WorldMarkDesc.pDocument = WorldMarkContractDocument.get();
		WorldMarkDesc.pPreparedResources = WorldMarkPrepared;
		WorldMarkDesc.RootWorld = Identity;
		WorldMarkDesc.bAutoPlay = false;
		WorldMarkDesc.bRequirePreparedResources = true;
		const std::shared_ptr<Client::CEffectObject> WorldMarkObject =
			Add_EffectObject(PrototypeTag, LayerTag, WorldMarkDesc);
		if (nullptr == WorldMarkObject)
		{
			std::cerr << "World Mark WARP object staging failed\n";
			return 1;
		}
		FRAME_EVIDENCE WorldMarkFrame;
		WorldMarkFrame.strScenarioId =
			"world-mark-precedes-normal-translucency-0.5s";
		WorldMarkFrame.strContract = "GAMEINSTANCE_WARP_MRT_SCENE_HDR";
		Write_Progress("world-mark-warp-render.begin");
		if (!EngineScope.Render_Frame(
				WorldMarkObject, 0.5f, WorldMarkFrame, Status))
		{
			std::cerr << "World Mark WARP render failed: " <<
				Status << '\n';
			return 1;
		}
		Write_Progress("world-mark-warp-render.complete");
		const Client::EFFECT_GPU_RENDER_SUBMISSION_STATS&
			WorldMarkStats =
				WorldMarkObject->Get_LastRenderSubmissionStats();
		const auto FindWorldMarkOccurrence =
			[&WorldMarkStats](const std::string_view strElementId)
			-> const Client::EFFECT_GPU_RENDER_OCCURRENCE_STATS*
			{
				const auto Occurrence = std::find_if(
					WorldMarkStats.Occurrences.begin(),
					WorldMarkStats.Occurrences.end(),
					[strElementId](
						const Client::EFFECT_GPU_RENDER_OCCURRENCE_STATS& Row)
					{
						return Row.strElementId == strElementId;
					});
				return Occurrence == WorldMarkStats.Occurrences.end() ?
					nullptr : &*Occurrence;
			};
		const Client::EFFECT_GPU_RENDER_OCCURRENCE_STATS*
			pNormalOccurrence =
				FindWorldMarkOccurrence("particle.world-mark.normal");
		const Client::EFFECT_GPU_RENDER_OCCURRENCE_STATS*
			pWorldMarkOccurrence =
				FindWorldMarkOccurrence("particle.world-mark.ground");
		if (nullptr == pNormalOccurrence ||
			nullptr == pWorldMarkOccurrence ||
			pNormalOccurrence->eCompositionLayer !=
				Client::EFFECT_COMPOSITION_LAYER::NORMAL ||
			pWorldMarkOccurrence->eCompositionLayer !=
				Client::EFFECT_COMPOSITION_LAYER::WORLD_MARK ||
			pWorldMarkOccurrence->iFirstIssuedDrawOrdinal ==
				UINT64_MAX ||
			pNormalOccurrence->iFirstIssuedDrawOrdinal ==
				UINT64_MAX ||
			pWorldMarkOccurrence->iFirstIssuedDrawOrdinal >=
				pNormalOccurrence->iFirstIssuedDrawOrdinal ||
			pWorldMarkOccurrence->iSubmitted != 1u ||
			pNormalOccurrence->iSubmitted != 1u ||
			!WorldMarkStats.bCompleted || !WorldMarkStats.bCommitted)
		{
			std::cerr <<
				"World Mark did not issue before document-earlier normal translucency\n";
			return 1;
		}
		WorldMarkFrame.Occurrence = Find_OccurrenceEvidence(
			WorldMarkStats, "particle.world-mark.ground");
		Frames.emplace_back(std::move(WorldMarkFrame));

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
		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>
			UnboundCanaryPrepared;
		if (bExpectedCanaryBinding &&
			!Client::CEffectDocumentRenderer::
				Prepare_UnboundMaterialProgramDocumentForTests(
					Device, Context, CanaryDocument, CanaryProjection,
					UnboundCanaryPrepared, Status))
		{
			std::cerr << "Artist Binding0 comparison prewarm failed: " <<
				Status << '\n';
			return 1;
		}
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
		Client::CEffectObject::EFFECT_OBJECT_DESC UnboundCanaryDesc = CanaryDesc;
		UnboundCanaryDesc.pPreparedResources = UnboundCanaryPrepared;
		const std::shared_ptr<Client::CEffectObject> UnboundCanaryObject =
			bExpectedCanaryBinding ?
				Add_EffectObject(PrototypeTag, LayerTag, UnboundCanaryDesc) : nullptr;
		if (nullptr == CanaryPrepared || nullptr == CanaryObject)
		{
			std::cerr << "Artist canary stage failed: " << Status << '\n';
			return 1;
		}
		if (bExpectedCanaryBinding &&
			(nullptr == UnboundCanaryPrepared || nullptr == UnboundCanaryObject))
		{
			std::cerr << "Artist Binding0 comparison stage failed: " <<
				Status << '\n';
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

		if (bExpectedCanaryBinding)
		{
			if (!Client::CEffectDocumentRenderer::
					Validate_MaterialProgramPreparedComparisonForTests(
						Device, Context, UnboundCanaryPrepared, CanaryPrepared,
						ARTIST_MESH_CANARY_ELEMENT_ID, Status) ||
				!UnboundCanaryObject->Set_TestPreviewElementIsolation(
					{ std::string(ARTIST_MESH_CANARY_ELEMENT_ID) }, Status) ||
				!CanaryObject->Set_TestPreviewElementIsolation(
					{ std::string(ARTIST_MESH_CANARY_ELEMENT_ID) }, Status))
			{
				std::cerr << "Artist mesh Binding0/Binding1 closure failed: " <<
					Status << '\n';
				return 1;
			}
			FRAME_EVIDENCE UnboundMeshCanaryFrame;
			UnboundMeshCanaryFrame.strScenarioId =
				"artist-f-mesh-binding0-inline-1.5s";
			UnboundMeshCanaryFrame.strContract =
				"GAMEINSTANCE_WARP_MRT_SCENE_HDR";
			Write_Progress("artist-mesh-binding0-render.begin");
			if (!EngineScope.Render_Frame(
					UnboundCanaryObject, 1.5f, UnboundMeshCanaryFrame, Status))
			{
				std::cerr << "Artist mesh Binding0 render failed: " <<
					Status << '\n';
				return 1;
			}
			Write_Progress("artist-mesh-binding0-render.complete");
			UnboundMeshCanaryFrame.Occurrence = Find_OccurrenceEvidence(
				UnboundCanaryObject->Get_LastRenderSubmissionStats(),
				ARTIST_MESH_CANARY_ELEMENT_ID);
			FRAME_EVIDENCE MeshCanaryFrame;
			MeshCanaryFrame.strScenarioId =
				"artist-f-mesh-binding1-registry-1.5s";
			MeshCanaryFrame.strContract = "GAMEINSTANCE_WARP_MRT_SCENE_HDR";
			Write_Progress("artist-mesh-canary-render.begin");
			if (!EngineScope.Render_Frame(
					CanaryObject, 1.5f, MeshCanaryFrame, Status))
			{
				std::cerr << "Artist mesh canary render failed: " << Status << '\n';
				return 1;
			}
			Write_Progress("artist-mesh-canary-render.complete");
			MeshCanaryFrame.Occurrence = Find_OccurrenceEvidence(
				CanaryObject->Get_LastRenderSubmissionStats(),
				ARTIST_MESH_CANARY_ELEMENT_ID);
			if (!UnboundMeshCanaryFrame.Occurrence.has_value() ||
				!MeshCanaryFrame.Occurrence.has_value() ||
				!Is_UnboundAdapterCanaryOccurrence(
					*UnboundMeshCanaryFrame.Occurrence,
					ARTIST_MESH_CANARY_ELEMENT_ID,
					Client::EFFECT_GPU_RENDER_CARRIER::MESH_CMODEL, 1u) ||
				!Is_BoundAdapterCanaryOccurrence(
					*MeshCanaryFrame.Occurrence,
					ARTIST_MESH_CANARY_ELEMENT_ID,
					Client::EFFECT_GPU_RENDER_CARRIER::MESH_CMODEL, 1u) ||
				!Is_Binding0Binding1DrawEquivalent(
					*UnboundMeshCanaryFrame.Occurrence,
					*MeshCanaryFrame.Occurrence))
			{
				std::cerr <<
					"Artist mesh Binding0/Binding1 draw receipt mismatched\n";
				return 1;
			}
			MeshCanaryFrame.bBoundAdapterActualPipelineValidated = true;
			Frames.emplace_back(std::move(UnboundMeshCanaryFrame));
			Frames.emplace_back(std::move(MeshCanaryFrame));

			if (!Client::CEffectDocumentRenderer::
					Validate_MaterialProgramPreparedComparisonForTests(
						Device, Context, UnboundCanaryPrepared, CanaryPrepared,
						ARTIST_DECAL_CANARY_ELEMENT_ID, Status) ||
				!UnboundCanaryObject->Set_TestPreviewElementIsolation(
					{ std::string(ARTIST_DECAL_CANARY_ELEMENT_ID) }, Status) ||
				!CanaryObject->Set_TestPreviewElementIsolation(
					{ std::string(ARTIST_DECAL_CANARY_ELEMENT_ID) }, Status))
			{
				std::cerr << "Artist decal Binding0/Binding1 closure failed: " <<
					Status << '\n';
				return 1;
			}
			FRAME_EVIDENCE UnboundDecalCanaryFrame;
			UnboundDecalCanaryFrame.strScenarioId =
				"artist-f-decal-binding0-inline-1.55s";
			UnboundDecalCanaryFrame.strContract =
				"GAMEINSTANCE_WARP_MRT_SCENE_HDR";
			Write_Progress("artist-decal-binding0-render.begin");
			if (!EngineScope.Render_Frame(
					UnboundCanaryObject, 1.55f, UnboundDecalCanaryFrame, Status))
			{
				std::cerr << "Artist decal Binding0 render failed: " <<
					Status << '\n';
				return 1;
			}
			Write_Progress("artist-decal-binding0-render.complete");
			UnboundDecalCanaryFrame.Occurrence = Find_OccurrenceEvidence(
				UnboundCanaryObject->Get_LastRenderSubmissionStats(),
				ARTIST_DECAL_CANARY_ELEMENT_ID);
			FRAME_EVIDENCE DecalCanaryFrame;
			DecalCanaryFrame.strScenarioId =
				"artist-f-decal-binding1-registry-1.55s";
			DecalCanaryFrame.strContract = "GAMEINSTANCE_WARP_MRT_SCENE_HDR";
			Write_Progress("artist-decal-canary-render.begin");
			if (!EngineScope.Render_Frame(
					CanaryObject, 1.55f, DecalCanaryFrame, Status))
			{
				std::cerr << "Artist decal canary render failed: " << Status << '\n';
				return 1;
			}
			Write_Progress("artist-decal-canary-render.complete");
			DecalCanaryFrame.Occurrence = Find_OccurrenceEvidence(
				CanaryObject->Get_LastRenderSubmissionStats(),
				ARTIST_DECAL_CANARY_ELEMENT_ID);
			if (!UnboundDecalCanaryFrame.Occurrence.has_value() ||
				!DecalCanaryFrame.Occurrence.has_value() ||
				!Is_UnboundAdapterCanaryOccurrence(
					*UnboundDecalCanaryFrame.Occurrence,
					ARTIST_DECAL_CANARY_ELEMENT_ID,
					Client::EFFECT_GPU_RENDER_CARRIER::DECAL_RECT, 3u) ||
				!Is_BoundAdapterCanaryOccurrence(
					*DecalCanaryFrame.Occurrence,
					ARTIST_DECAL_CANARY_ELEMENT_ID,
					Client::EFFECT_GPU_RENDER_CARRIER::DECAL_RECT, 3u) ||
				!Is_Binding0Binding1DrawEquivalent(
					*UnboundDecalCanaryFrame.Occurrence,
					*DecalCanaryFrame.Occurrence))
			{
				std::cerr <<
					"Artist decal Binding0/Binding1 draw receipt mismatched\n";
				return 1;
			}
			DecalCanaryFrame.bBoundAdapterActualPipelineValidated = true;
			Frames.emplace_back(std::move(UnboundDecalCanaryFrame));
			Frames.emplace_back(std::move(DecalCanaryFrame));

			for (const auto& Binding :
				{ CanaryBinding, MeshCanaryBinding, DecalCanaryBinding })
			{
				const size_t iAdapter = static_cast<size_t>(
					Binding->Adapter.eAdapterId);
				if (iAdapter >= ActualCompiledAdapterDrawCoverage.size())
				{
					std::cerr <<
						"Artist actual draw selected an invalid adapter enum\n";
					return 1;
				}
				ActualCompiledAdapterDrawCoverage[iAdapter] = true;
			}
		}

		if (bExpectedRepresentativeV1)
		{
			std::vector<REPRESENTATIVE_V1_PREPARED_TARGET> V1PreparedTargets;
			V1PreparedTargets.reserve(REPRESENTATIVE_V1_EFFECT_IDS.size());
			for (const std::string_view strEffectAssetId :
				REPRESENTATIVE_V1_EFFECT_IDS)
			{
				REPRESENTATIVE_V1_PREPARED_TARGET PreparedTarget;
				PreparedTarget.strEffectAssetId = strEffectAssetId;
				PreparedTarget.pDocument = Client::CEffectCatalog::Find(
					PreparedTarget.strEffectAssetId);
				PreparedTarget.pProjection =
					Client::CEffectCatalog::Find_VisualProjection(
						PreparedTarget.strEffectAssetId);
				Client::EFFECT_RENDER_PREWARM_TARGET Target;
				Target.strEffectAssetId = PreparedTarget.strEffectAssetId;
				Target.pDocument = PreparedTarget.pDocument;
				Target.pVisualProgramProjection = PreparedTarget.pProjection;
				Target.pMaterialProgramRegistry = Registry;
				Write_Progress("representative-v1-prewarm.begin");
				if (nullptr == PreparedTarget.pDocument ||
					!Client::CEffectDocumentRenderer::Prepare_VisualProgramTarget(
						Device, Context, CatalogRevision, Target, Status))
				{
					std::cerr << "representative V1 prewarm failed for " <<
						PreparedTarget.strEffectAssetId << ": " << Status << '\n';
					return 1;
				}
				PreparedTarget.pPrepared =
					Client::CEffectDocumentRenderer::Find_Prepared(
						CatalogRevision, PreparedTarget.strEffectAssetId,
						*PreparedTarget.pDocument, PreparedTarget.pProjection,
						Registry);
				if (nullptr == PreparedTarget.pPrepared)
				{
					std::cerr << "representative V1 prepared identity lookup failed for " <<
						PreparedTarget.strEffectAssetId << '\n';
					return 1;
				}
				V1PreparedTargets.emplace_back(std::move(PreparedTarget));
				Write_Progress("representative-v1-prewarm.complete");
			}

			const Client::EFFECT_RENDER_PREWARM_PROBE V1PrewarmProbe =
				Client::CEffectDocumentRenderer::Get_PrewarmProbe();
			if (V1PrewarmProbe.iCatalogRevision != CatalogRevision ||
				V1PrewarmProbe.iMaterialProgramRegistryGeneration !=
					Registry->Get_GenerationId() ||
				V1PrewarmProbe.iMaterialProgramBindingCount !=
					ExpectedBindingCount ||
				V1PrewarmProbe.iMaterialProgramResolvedElementCount !=
					REPRESENTATIVE_V1_TOTAL_BINDING_COUNT)
			{
				std::cerr <<
					"representative V1 prewarm did not close every registry binding\n";
				return 1;
			}

			for (const REPRESENTATIVE_V1_DRAW_PROBE_DESC& Probe :
				REPRESENTATIVE_V1_DRAW_PROBES)
			{
				const auto PreparedTarget = std::find_if(
					V1PreparedTargets.begin(), V1PreparedTargets.end(),
					[&Probe](const REPRESENTATIVE_V1_PREPARED_TARGET& Candidate)
					{
						return Candidate.strEffectAssetId == Probe.strEffectAssetId;
					});
				const auto Binding = Registry->Resolve(
					Probe.strEffectAssetId, Probe.strElementId);
				Client::EFFECT_COMPILED_MATERIAL_CARRIER eExpectedCompiledCarrier =
					Client::EFFECT_COMPILED_MATERIAL_CARRIER::END;
				switch (Probe.eCarrier)
				{
				case Client::EFFECT_GPU_RENDER_CARRIER::SPRITE_INSTANCE:
					eExpectedCompiledCarrier = Client::
						EFFECT_COMPILED_MATERIAL_CARRIER::SPRITE_PARTICLE;
					break;
				case Client::EFFECT_GPU_RENDER_CARRIER::MESH_CMODEL:
					eExpectedCompiledCarrier = Client::
						EFFECT_COMPILED_MATERIAL_CARRIER::MESH_PARTICLE_CMODEL;
					break;
				case Client::EFFECT_GPU_RENDER_CARRIER::DECAL_RECT:
					eExpectedCompiledCarrier = Client::
						EFFECT_COMPILED_MATERIAL_CARRIER::LOCAL_DECAL_PROJECTOR;
					break;
				case Client::EFFECT_GPU_RENDER_CARRIER::SPRITE_RECT:
				case Client::EFFECT_GPU_RENDER_CARRIER::RIBBON_DYNAMIC_TRAIL:
				case Client::EFFECT_GPU_RENDER_CARRIER::END:
				default:
					break;
				}
				if (PreparedTarget == V1PreparedTargets.end() || nullptr == Binding ||
					Binding->Execution.eBackend != Client::
						EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 ||
					Binding->Adapter.eCarrier != eExpectedCompiledCarrier ||
					Binding->Adapter.iPassIndex != Probe.iPassIndex)
				{
					std::cerr << "representative V1 probe binding identity changed: " <<
						Probe.strElementId << '\n';
					return 1;
				}

				std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>
					UnboundPrepared;
				if (!Client::CEffectDocumentRenderer::
						Prepare_UnboundMaterialProgramDocumentForTests(
							Device, Context, PreparedTarget->pDocument,
							PreparedTarget->pProjection, UnboundPrepared, Status) ||
					!Client::CEffectDocumentRenderer::
						Validate_MaterialProgramPreparedComparisonForTests(
							Device, Context, UnboundPrepared,
							PreparedTarget->pPrepared, Probe.strElementId, Status))
				{
					std::cerr << "representative V1 Binding0/Binding1 packet closure failed: " <<
						Status << '\n';
					return 1;
				}

				Client::CEffectObject::EFFECT_OBJECT_DESC BoundDesc{};
				BoundDesc.pDocument = PreparedTarget->pDocument.get();
				BoundDesc.pPreparedResources = PreparedTarget->pPrepared;
				BoundDesc.pVisualProgramProjection = PreparedTarget->pProjection;
				BoundDesc.RootWorld = Identity;
				BoundDesc.bAutoPlay = false;
				BoundDesc.bRequirePreparedResources = true;
				if (nullptr != PreparedTarget->pProjection)
					BoundDesc.pDocument = nullptr;
				Client::CEffectObject::EFFECT_OBJECT_DESC UnboundDesc = BoundDesc;
				UnboundDesc.pPreparedResources = UnboundPrepared;
				const auto UnboundObject = Add_EffectObject(
					PrototypeTag, LayerTag, UnboundDesc);
				const auto BoundObject = Add_EffectObject(
					PrototypeTag, LayerTag, BoundDesc);
				if (nullptr == UnboundObject || nullptr == BoundObject ||
					!UnboundObject->Set_TestPreviewElementIsolation(
						{ std::string(Probe.strElementId) }, Status) ||
					!BoundObject->Set_TestPreviewElementIsolation(
						{ std::string(Probe.strElementId) }, Status))
				{
					std::cerr << "representative V1 draw object stage failed: " <<
						Status << '\n';
					return 1;
				}

				FRAME_EVIDENCE UnboundFrame;
				UnboundFrame.strScenarioId = std::string(Probe.strScenarioStem) +
					"-binding0-inline";
				UnboundFrame.strContract = "GAMEINSTANCE_WARP_MRT_SCENE_HDR";
				FRAME_EVIDENCE BoundFrame;
				BoundFrame.strScenarioId = std::string(Probe.strScenarioStem) +
					"-binding1-registry";
				BoundFrame.strContract = "GAMEINSTANCE_WARP_MRT_SCENE_HDR";
				Write_Progress("representative-v1-binding0-render.begin");
				if (!EngineScope.Render_Frame(UnboundObject,
						Probe.fSampleTimeSeconds, UnboundFrame, Status))
				{
					std::cerr << "representative V1 Binding0 draw failed: " <<
						Status << '\n';
					return 1;
				}
				Write_Progress("representative-v1-binding0-render.complete");
				Write_Progress("representative-v1-binding1-render.begin");
				if (!EngineScope.Render_Frame(BoundObject,
						Probe.fSampleTimeSeconds, BoundFrame, Status))
				{
					std::cerr << "representative V1 Binding1 draw failed: " <<
						Status << '\n';
					return 1;
				}
				Write_Progress("representative-v1-binding1-render.complete");
				UnboundFrame.Occurrence = Find_OccurrenceEvidence(
					UnboundObject->Get_LastRenderSubmissionStats(), Probe.strElementId);
				BoundFrame.Occurrence = Find_OccurrenceEvidence(
					BoundObject->Get_LastRenderSubmissionStats(), Probe.strElementId);
				const bool_t bUnboundReceipt =
					UnboundFrame.Occurrence.has_value() &&
					Is_UnboundAdapterCanaryOccurrence(
						*UnboundFrame.Occurrence, Probe.strElementId,
						Probe.eCarrier, Probe.iPassIndex);
				const bool_t bBoundReceipt =
					BoundFrame.Occurrence.has_value() &&
					Is_BoundAdapterCanaryOccurrence(
						*BoundFrame.Occurrence, Probe.strElementId,
						Probe.eCarrier, Probe.iPassIndex);
				const bool_t bDrawEquivalent =
					UnboundFrame.Occurrence.has_value() &&
					BoundFrame.Occurrence.has_value() &&
					Is_Binding0Binding1DrawEquivalent(
						*UnboundFrame.Occurrence, *BoundFrame.Occurrence);
				if (!UnboundFrame.Occurrence.has_value() ||
					!BoundFrame.Occurrence.has_value() ||
					!bUnboundReceipt || !bBoundReceipt || !bDrawEquivalent)
				{
					std::cerr <<
						"representative V1 actual carrier/pass/state/MRT receipt mismatched: " <<
						Probe.strElementId << " [unbound=" << bUnboundReceipt <<
						", bound=" << bBoundReceipt << ", equivalent=" <<
						bDrawEquivalent << "]\n";
					if (UnboundFrame.Occurrence.has_value() &&
						BoundFrame.Occurrence.has_value())
					{
						const OCCURRENCE_EVIDENCE& U = *UnboundFrame.Occurrence;
						const OCCURRENCE_EVIDENCE& B = *BoundFrame.Occurrence;
						std::cerr << "unbound/bound counts: active=" << U.iActive <<
							'/' << B.iActive << " candidate=" << U.iCandidate <<
							'/' << B.iCandidate << " attempted=" << U.iAttempted <<
							'/' << B.iAttempted << " submitted=" << U.iSubmitted <<
							'/' << B.iSubmitted << " failed=" << U.iFailed << '/'
							<< B.iFailed << " material=" << U.iMaterialBindCount << '/'
							<< B.iMaterialBindCount << " srv=" <<
							U.iTextureSrvBindCount << '/' << B.iTextureSrvBindCount <<
							" sampler=" << U.iSamplerBindCount << '/' <<
							B.iSamplerBindCount << " pass=" <<
							U.iShaderPassApplyCount << '/' <<
							B.iShaderPassApplyCount << " vi=" <<
							U.iVIBufferDrawCount << '/' << B.iVIBufferDrawCount <<
							" issued=" << U.iIssuedDrawCallCount << '/' <<
							B.iIssuedDrawCallCount << " adapter=" <<
							U.iCompiledAdapterPipelineValidationCount << '/' <<
							B.iCompiledAdapterPipelineValidationCount << " carrier=" <<
							static_cast<uint32_t>(U.eCarrier) << '/' <<
							static_cast<uint32_t>(B.eCarrier) << " selectedPass=" <<
							U.iSelectedPassIndex << '/' << B.iSelectedPassIndex <<
							" world=" << U.iFirstSubmittedWorldHash << '/' <<
							B.iFirstSubmittedWorldHash << '\n';
					}
					return 1;
				}
				BoundFrame.bBoundAdapterActualPipelineValidated = true;
				const size_t iActualAdapter = static_cast<size_t>(
					Binding->Adapter.eAdapterId);
				if (iActualAdapter >= ActualCompiledAdapterDrawCoverage.size())
				{
					std::cerr <<
						"representative V1 actual draw selected an invalid adapter enum\n";
					return 1;
				}
				ActualCompiledAdapterDrawCoverage[iActualAdapter] = true;
				Frames.emplace_back(std::move(UnboundFrame));
				Frames.emplace_back(std::move(BoundFrame));
				++iRepresentativeV1ActualDrawCount;
			}

			iActualCompiledAdapterDrawCount = static_cast<size_t>(std::count(
				ActualCompiledAdapterDrawCoverage.begin(),
				ActualCompiledAdapterDrawCoverage.end(), true));
			if (iRepresentativeV1ActualDrawCount !=
					REPRESENTATIVE_V1_DRAW_PROBES.size() ||
				iActualCompiledAdapterDrawCount !=
					ActualCompiledAdapterDrawCoverage.size())
			{
				std::cerr <<
					"actual Binding0/Binding1 draw or compiled-adapter coverage changed\n";
				return 1;
			}
		}

		constexpr std::string_view LANCE_PORTABLE_EFFECT_ID =
			"effect.lancemaster.contract.saved-element.ba1-portable-warp";
		constexpr std::string_view LANCE_PORTABLE_SOURCE_ELEMENT_ID =
			"authored.source-particle.ce43b71ae7ca3379dfe8529d";
		constexpr std::string_view LANCE_PORTABLE_RUNTIME_ELEMENT_ID =
			"authored.copy.contract.lance-ba1-portable.1";
		Client::EFFECT_DOCUMENT_DESC LancePortableSource;
		Client::EFFECT_DOCUMENT_DESC LancePortableCopy;
		const std::filesystem::path LancePortableSourcePath =
			RepositoryRoot / L"Data" / L"Effects" / L"Authored" /
				L"effect.lancemaster.skill.34010.ba1.unified.effect.json";
		Write_Progress("lance-ba1-portable-copy.begin");
		if (!Client::CEffectDocumentCodec::Load(
				LancePortableSourcePath, LancePortableSource, Status) ||
			!Client::CEffectDocumentCodec::
				Build_PortableAuthoredElementStartingCopy(
					LancePortableSource, LANCE_PORTABLE_SOURCE_ELEMENT_ID,
					LANCE_PORTABLE_EFFECT_ID, LancePortableCopy, Status) ||
			LancePortableCopy.Elements.size() != 1u ||
			LancePortableCopy.Elements.front().strElementId !=
				LANCE_PORTABLE_SOURCE_ELEMENT_ID)
		{
			std::cerr << "Lance BA1 portable authored copy failed: " <<
				Status << '\n';
			return 1;
		}
		LancePortableCopy.Elements.front().strElementId =
			std::string(LANCE_PORTABLE_RUNTIME_ELEMENT_ID);
		const std::string LancePortableJson =
			Client::CEffectDocumentCodec::Serialize(LancePortableCopy);
		Client::EFFECT_DOCUMENT_DESC LancePortableRoundTrip;
		if (!Client::CEffectDocumentCodec::Parse(
				LancePortableJson, LancePortableRoundTrip, Status) ||
			!Client::CEffectDocumentCodec::Validate(
				LancePortableRoundTrip, Status) ||
			Client::CEffectDocumentCodec::Serialize(LancePortableRoundTrip) !=
				LancePortableJson)
		{
			std::cerr << "Lance BA1 renamed portable round-trip failed: " <<
				Status << '\n';
			return 1;
		}
		LancePortableCopy = std::move(LancePortableRoundTrip);
		Write_Progress("lance-ba1-portable-copy.complete");
		/* Data Files exposes the direct-authored .unified document as the saved
		   BA1 source; the v12 non-unified document above is only the separate
		   legacy/rollback projection smoke.  Render this exact source before its
		   portable copy so the copy is compared with its real shader oracle rather
		   than with the legacy grouped-translucent carrier. */
		const std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>
			LanceUnifiedSourceDocument =
				std::make_shared<Client::EFFECT_DOCUMENT_DESC>(
					LancePortableSource);
		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>
			LanceUnifiedSourcePrepared;
		Write_Progress("lance-ba1-unified-source-prewarm.begin");
		if (!Client::CEffectDocumentRenderer::
				Prepare_UnboundMaterialProgramDocumentForTests(
					Device, Context, LanceUnifiedSourceDocument, nullptr,
					LanceUnifiedSourcePrepared, Status))
		{
			std::cerr << "Lance BA1 unified source WARP prewarm failed: " <<
				Status << '\n';
			return 1;
		}
		Write_Progress("lance-ba1-unified-source-prewarm.complete");
		Client::CEffectObject::EFFECT_OBJECT_DESC LanceUnifiedSourceDesc{};
		LanceUnifiedSourceDesc.pDocument = LanceUnifiedSourceDocument.get();
		LanceUnifiedSourceDesc.pPreparedResources = LanceUnifiedSourcePrepared;
		LanceUnifiedSourceDesc.RootWorld = Identity;
		LanceUnifiedSourceDesc.bAutoPlay = false;
		LanceUnifiedSourceDesc.bRequirePreparedResources = true;
		Write_Progress("lance-ba1-unified-source-stage.begin");
		const std::shared_ptr<Client::CEffectObject> LanceUnifiedSourceObject =
			Add_EffectObject(PrototypeTag, LayerTag, LanceUnifiedSourceDesc);
		if (nullptr == LanceUnifiedSourcePrepared ||
			nullptr == LanceUnifiedSourceObject)
		{
			std::cerr << "Lance BA1 unified source WARP stage failed\n";
			return 1;
		}
		Write_Progress("lance-ba1-unified-source-stage.complete");

		FRAME_EVIDENCE LanceUnifiedSourceTick32Frame;
		LanceUnifiedSourceTick32Frame.strScenarioId =
			"lance-ba1-unified-source-tick32";
		LanceUnifiedSourceTick32Frame.strContract =
			"GAMEINSTANCE_WARP_MRT_SCENE_HDR";
		Write_Progress("lance-ba1-unified-source-render.begin");
		if (!EngineScope.Render_Frame(
				LanceUnifiedSourceObject, 32.f / 60.f,
				LanceUnifiedSourceTick32Frame, Status))
		{
			std::cerr << "Lance BA1 unified source WARP render failed: " <<
				Status << '\n';
			return 1;
		}
		Write_Progress("lance-ba1-unified-source-render.complete");
		LanceUnifiedSourceTick32Frame.Aggregate = Read_FamilyStats(
			LanceUnifiedSourceObject->Get_LastRenderSubmissionStats(),
			Client::EFFECT_GPU_RENDER_FAMILY::MESH);
		LanceUnifiedSourceTick32Frame.Occurrence = Find_OccurrenceEvidence(
			LanceUnifiedSourceObject->Get_LastRenderSubmissionStats(),
			LANCE_PORTABLE_SOURCE_ELEMENT_ID);
		if (!Is_ExactLanceTick32(LanceUnifiedSourceTick32Frame.Aggregate) ||
			!LanceUnifiedSourceTick32Frame.Occurrence.has_value() ||
			LanceUnifiedSourceTick32Frame.Occurrence->strElementId !=
				LANCE_PORTABLE_SOURCE_ELEMENT_ID ||
			LanceUnifiedSourceTick32Frame.Occurrence->iSourceMaterialProfile !=
				15u)
		{
			std::cerr <<
				"Lance BA1 canonical unified source did not admit effective profile 15";
			if (LanceUnifiedSourceTick32Frame.Occurrence.has_value())
			{
				const OCCURRENCE_EVIDENCE& Row =
					*LanceUnifiedSourceTick32Frame.Occurrence;
				std::cerr << " [profile=" << Row.iSourceMaterialProfile <<
					", carrier=" << Carrier_Token(Row.eCarrier) <<
					", pass=" << Row.iSelectedPassIndex <<
					", srv=" << Row.iTextureSrvBindCount <<
					", submitted=" << Row.iSubmitted <<
					", failed=" << Row.iFailed << ']';
			}
			std::cerr << '\n';
			return 1;
		}
		const std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>
			LancePortableDocument =
				std::make_shared<Client::EFFECT_DOCUMENT_DESC>(
					std::move(LancePortableCopy));
		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>
			LancePortablePrepared;
		Write_Progress("lance-ba1-portable-prewarm.begin");
		if (!Client::CEffectDocumentRenderer::
				Prepare_UnboundMaterialProgramDocumentForTests(
					Device, Context, LancePortableDocument, nullptr,
					LancePortablePrepared, Status))
		{
			std::cerr << "Lance BA1 portable WARP prewarm failed: " <<
				Status << '\n';
			return 1;
		}
		Write_Progress("lance-ba1-portable-prewarm.complete");
		Client::CEffectObject::EFFECT_OBJECT_DESC LancePortableDesc{};
		LancePortableDesc.pDocument = LancePortableDocument.get();
		LancePortableDesc.pPreparedResources = LancePortablePrepared;
		LancePortableDesc.RootWorld = Identity;
		LancePortableDesc.bAutoPlay = false;
		LancePortableDesc.bRequirePreparedResources = true;
		Write_Progress("lance-ba1-portable-stage.begin");
		const std::shared_ptr<Client::CEffectObject> LancePortableObject =
			Add_EffectObject(PrototypeTag, LayerTag, LancePortableDesc);
		if (nullptr == LancePortablePrepared || nullptr == LancePortableObject)
		{
			std::cerr << "Lance BA1 portable WARP stage failed\n";
			return 1;
		}
		Write_Progress("lance-ba1-portable-stage.complete");

		FRAME_EVIDENCE LancePortableTick32Frame;
		LancePortableTick32Frame.strScenarioId =
			"lance-ba1-portable-saved-element-tick32";
		LancePortableTick32Frame.strContract =
			"GAMEINSTANCE_WARP_MRT_SCENE_HDR";
		Write_Progress("lance-ba1-portable-render.begin");
		if (!EngineScope.Render_Frame(
				LancePortableObject, 32.f / 60.f,
				LancePortableTick32Frame, Status))
		{
			std::cerr << "Lance BA1 portable WARP render failed: " <<
				Status << '\n';
			return 1;
		}
		Write_Progress("lance-ba1-portable-render.complete");
		LancePortableTick32Frame.Aggregate = Read_FamilyStats(
			LancePortableObject->Get_LastRenderSubmissionStats(),
			Client::EFFECT_GPU_RENDER_FAMILY::MESH);
		LancePortableTick32Frame.Occurrence = Find_OccurrenceEvidence(
			LancePortableObject->Get_LastRenderSubmissionStats(),
			LANCE_PORTABLE_RUNTIME_ELEMENT_ID);
		if (!LancePortableTick32Frame.Occurrence.has_value())
		{
			std::cerr <<
				"Lance BA1 portable source/copy comparison lost the portable occurrence\n";
			return 1;
		}

		const AGGREGATE_STATS& ExpectedAggregate =
			LanceUnifiedSourceTick32Frame.Aggregate;
		const AGGREGATE_STATS& ActualAggregate =
			LancePortableTick32Frame.Aggregate;
		const OCCURRENCE_EVIDENCE& ExpectedOccurrence =
			*LanceUnifiedSourceTick32Frame.Occurrence;
		const OCCURRENCE_EVIDENCE& ActualOccurrence =
			*LancePortableTick32Frame.Occurrence;
		bool_t bPortableMatchesUnifiedSource = true;
		const auto CompareU64 = [&bPortableMatchesUnifiedSource](
			const char* pField, const uint64_t iExpected,
			const uint64_t iActual)
		{
			if (iExpected == iActual)
				return;
			bPortableMatchesUnifiedSource = false;
			std::cerr << "  " << pField << ": source=" << iExpected <<
				", portable=" << iActual << '\n';
		};
		const auto CompareU32 = [&bPortableMatchesUnifiedSource](
			const char* pField, const uint32_t iExpected,
			const uint32_t iActual)
		{
			if (iExpected == iActual)
				return;
			bPortableMatchesUnifiedSource = false;
			std::cerr << "  " << pField << ": source=" << iExpected <<
				", portable=" << iActual << '\n';
		};
		const auto CompareBool = [&bPortableMatchesUnifiedSource](
			const char* pField, const bool_t bExpected,
			const bool_t bActual)
		{
			if (bExpected == bActual)
				return;
			bPortableMatchesUnifiedSource = false;
			std::cerr << "  " << pField << ": source=" <<
				(bExpected ? "true" : "false") << ", portable=" <<
				(bActual ? "true" : "false") << '\n';
		};

		if (ActualOccurrence.strElementId != LANCE_PORTABLE_RUNTIME_ELEMENT_ID)
		{
			bPortableMatchesUnifiedSource = false;
			std::cerr << "  occurrence.elementId: source=" <<
				ExpectedOccurrence.strElementId << ", portable=" <<
				ActualOccurrence.strElementId << ", expectedPortable=" <<
				LANCE_PORTABLE_RUNTIME_ELEMENT_ID << '\n';
		}
		CompareU64("aggregate.configured", ExpectedAggregate.iConfigured,
			ActualAggregate.iConfigured);
		CompareU64("aggregate.evaluated", ExpectedAggregate.iEvaluated,
			ActualAggregate.iEvaluated);
		CompareU64("aggregate.active", ExpectedAggregate.iActive,
			ActualAggregate.iActive);
		CompareU64("aggregate.candidate", ExpectedAggregate.iCandidate,
			ActualAggregate.iCandidate);
		CompareU64("aggregate.attempted", ExpectedAggregate.iAttempted,
			ActualAggregate.iAttempted);
		CompareU64("aggregate.submitted", ExpectedAggregate.iSubmitted,
			ActualAggregate.iSubmitted);
		CompareU64("aggregate.suppressed", ExpectedAggregate.iSuppressed,
			ActualAggregate.iSuppressed);
		CompareU64("aggregate.failed", ExpectedAggregate.iFailed,
			ActualAggregate.iFailed);
		CompareBool("aggregate.completed", ExpectedAggregate.bCompleted,
			ActualAggregate.bCompleted);
		CompareBool("aggregate.committed", ExpectedAggregate.bCommitted,
			ActualAggregate.bCommitted);
		CompareU64("occurrence.active", ExpectedOccurrence.iActive,
			ActualOccurrence.iActive);
		CompareU64("occurrence.candidate", ExpectedOccurrence.iCandidate,
			ActualOccurrence.iCandidate);
		CompareU64("occurrence.attempted", ExpectedOccurrence.iAttempted,
			ActualOccurrence.iAttempted);
		CompareU64("occurrence.submitted", ExpectedOccurrence.iSubmitted,
			ActualOccurrence.iSubmitted);
		CompareU64("occurrence.suppressed", ExpectedOccurrence.iSuppressed,
			ActualOccurrence.iSuppressed);
		CompareU64("occurrence.failed", ExpectedOccurrence.iFailed,
			ActualOccurrence.iFailed);
		CompareU64("occurrence.materialBinds",
			ExpectedOccurrence.iMaterialBindCount,
			ActualOccurrence.iMaterialBindCount);
		CompareU64("occurrence.textureSrvBinds",
			ExpectedOccurrence.iTextureSrvBindCount,
			ActualOccurrence.iTextureSrvBindCount);
		CompareU64("occurrence.samplerBinds",
			ExpectedOccurrence.iSamplerBindCount,
			ActualOccurrence.iSamplerBindCount);
		CompareU64("occurrence.shaderPassApplies",
			ExpectedOccurrence.iShaderPassApplyCount,
			ActualOccurrence.iShaderPassApplyCount);
		CompareU64("occurrence.viBufferBinds",
			ExpectedOccurrence.iVIBufferBindCount,
			ActualOccurrence.iVIBufferBindCount);
		CompareU64("occurrence.viBufferDraws",
			ExpectedOccurrence.iVIBufferDrawCount,
			ActualOccurrence.iVIBufferDrawCount);
		CompareU64("occurrence.issuedDrawCalls",
			ExpectedOccurrence.iIssuedDrawCallCount,
			ActualOccurrence.iIssuedDrawCallCount);
		CompareU64("occurrence.drawSelections",
			ExpectedOccurrence.iDrawSelectionCount,
			ActualOccurrence.iDrawSelectionCount);
		CompareU64("occurrence.compiledAdapterPipelineValidations",
			ExpectedOccurrence.iCompiledAdapterPipelineValidationCount,
			ActualOccurrence.iCompiledAdapterPipelineValidationCount);
		CompareU32("occurrence.passIndex",
			ExpectedOccurrence.iSelectedPassIndex,
			ActualOccurrence.iSelectedPassIndex);
		CompareU32("occurrence.sourceMaterialProfile",
			ExpectedOccurrence.iSourceMaterialProfile,
			ActualOccurrence.iSourceMaterialProfile);
		if (ExpectedOccurrence.eCarrier != ActualOccurrence.eCarrier)
		{
			bPortableMatchesUnifiedSource = false;
			std::cerr << "  occurrence.carrier: source=" <<
				Carrier_Token(ExpectedOccurrence.eCarrier) << ", portable=" <<
				Carrier_Token(ActualOccurrence.eCarrier) << '\n';
		}
		CompareBool("occurrence.drawSelectionDiverged",
			ExpectedOccurrence.bDrawSelectionDiverged,
			ActualOccurrence.bDrawSelectionDiverged);
		CompareU64("occurrence.firstSubmittedWorldFnv1a64",
			ExpectedOccurrence.iFirstSubmittedWorldHash,
			ActualOccurrence.iFirstSubmittedWorldHash);
		CompareBool("occurrence.hasFirstSubmittedWorld",
			ExpectedOccurrence.bHasFirstSubmittedWorld,
			ActualOccurrence.bHasFirstSubmittedWorld);
		if (ExpectedOccurrence.iSourceMaterialProfile != 15u ||
			ActualOccurrence.iSourceMaterialProfile != 15u)
		{
			bPortableMatchesUnifiedSource = false;
			std::cerr <<
				"  occurrence.sourceMaterialProfile contract: source=" <<
				ExpectedOccurrence.iSourceMaterialProfile << ", portable=" <<
				ActualOccurrence.iSourceMaterialProfile << ", required=15\n";
		}
		if (!bPortableMatchesUnifiedSource)
		{
			std::cerr <<
				"Lance BA1 portable copy diverged from its canonical unified source\n";
			return 1;
		}
		Frames.emplace_back(std::move(LanceUnifiedSourceTick32Frame));
		Frames.emplace_back(std::move(LancePortableTick32Frame));

		PrewarmProbe = Client::CEffectDocumentRenderer::Get_PrewarmProbe();
		if (PrewarmProbe.iCatalogRevision != CatalogRevision ||
			PrewarmProbe.iMaterialProgramRegistryGeneration !=
				Registry->Get_GenerationId() ||
			PrewarmProbe.iMaterialProgramBindingCount != ExpectedBindingCount ||
			PrewarmProbe.iMaterialProgramResolvedElementCount !=
				(bExpectedRepresentativeV1 ? REPRESENTATIVE_V1_TOTAL_BINDING_COUNT :
				 (bExpectedCanaryBinding ? 3u : 0u)))
		{
			std::cerr << "prepared registry generation/count propagation mismatched\n";
			return 1;
		}
	}
	iActualCompiledAdapterDrawCount = static_cast<size_t>(std::count(
		ActualCompiledAdapterDrawCoverage.begin(),
		ActualCompiledAdapterDrawCoverage.end(), true));

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
		",\"representativeV1RegistryBindingCount\":" <<
			iRepresentativeV1BindingCount <<
		",\"coveredCompiledAdapterCount\":" <<
			iCoveredCompiledAdapterCount <<
		",\"actualCompiledAdapterDrawCount\":" <<
			iActualCompiledAdapterDrawCount <<
		",\"representativeV1ActualDrawCount\":" <<
			iRepresentativeV1ActualDrawCount <<
		",\"catalogBindingCarrierProfileRollbackValidated\":" <<
			(bCatalogBindingRollbackValidated ? "true" : "false") <<
		",\"artistECraneCloneAnimationValidated\":" <<
			(bArtistECraneCloneAnimationValidated ? "true" : "false") <<
		",\"artistECraneCloneAnimation\":{\"prototypeDurationSeconds\":" <<
			ArtistECraneProbe.fPrototypeDurationSeconds <<
		",\"cloneDurationSeconds\":" <<
			ArtistECraneProbe.fCloneDurationSeconds <<
		",\"midTrackPositionTicks\":" <<
			ArtistECraneProbe.fMidTrackPositionTicks <<
		",\"tailTrackPositionTicks\":" <<
			ArtistECraneProbe.fTailTrackPositionTicks <<
		",\"trackDurationTicks\":" <<
			ArtistECraneProbe.fTrackDurationTicks <<
		",\"witnessBoneMaximumDelta\":" <<
			ArtistECraneProbe.fWitnessBoneMaximumDelta << "}" <<
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
