#pragma once

#include "Effect_DocumentRenderer.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_MaterialProgramRegistry.h"
#include "Effect_VisualProgramCorpus.h"
#include "Engine_VertexTypes.h"
#include <mutex>
#include <map>
#include <chrono>
#include <tuple>


// Private implementation contracts shared by the separately compiled Effect units.
	struct Client::CEffectDocumentRenderer::SOURCE_MATERIAL_SLOT_RESOURCE final
	{
		uint32_t iSourceMaterialIndex = 0u;
		std::shared_ptr<const ELEMENT_RESOURCE> pResource;
	};
	struct Client::CEffectDocumentRenderer::ELEMENT_RESOURCE final
	{
		std::vector<SOURCE_MATERIAL_SLOT_RESOURCE> SourceMaterialSlots;
		shared_ptr<Engine::CModel> pModel;
		/* One lane per EFFECT_RESOURCE_SLOT texture slot, indexed by
		   slot - BASE_TEXTURE. Grew from 5 to 8 with base2/mask2/noise2. */
		std::array<ComPtr<ID3D11ShaderResourceView>, 8> Textures;
		std::array<ComPtr<ID3D11ShaderResourceView>, 10> SourceTextures;
		uint32_t iSourceTextureMask = 0u;
		uint32_t iSourceMaterialProfile = 0u;
		uint32_t iShaderProgramIndex = UINT32_MAX;
		float4_t vSourceScalars0{};
		float4_t vSourceScalars1{};
		float4_t vSourceVector0{};
		float4_t vSourceVector1{};
		std::array<float4_t, 8u> TypedTrailParameters{};
		std::array<float4_t, 32u> QSourceMaterialParameters{};
		std::array<float4_t, 32u> VSourceMaterialParameters{};
		std::array<float4_t, 32u> ALTVSourceMaterialParameters{};
		std::array<float4_t, 32u> ArtistSourceMaterialParameters{};
		std::array<float4_t, 32u> LanceVASourceMaterialParameters{};
		bool_t bSourceRequiresSceneColor = false;
		bool_t bSourceRequiresSceneDepth = false;
		ComPtr<ID3D11BlendState> pNativeOneLayerBlend;
		uint32_t iSourceMeshHasUV1 = 0u;
		std::array<float4_t, 16u> LinearFlowParameters{};
		float4_t vLinearFlowMaskAColor{ 1.f, 1.f, 1.f, 1.f };
		float4_t vLinearFlowMaskBColor{ 1.f, 1.f, 1.f, 1.f };
		std::array<float4_t, 16u> BlacklineParameters{};
		float4_t vBlacklineDiffuseColor{ 1.f, 1.f, 1.f, 1.f };
		float4_t vBlacklineMaskColor{ 1.f, 1.f, 1.f, 1.f };
		std::array<float4_t, 5u> LocalCrackParameters{};
		float4_t vLocalCrackOutColor{ 0.1f, 0.1f, 0.1f, 1.f };
		float4_t vLocalCrackInColor{ 1.f, 1.f, 1.f, 1.f };
		float4_t vLocalCrackReflectionColor{ 1.f, 1.f, 1.f, 1.f };
		uint32_t iReconstructedMaterialEvaluatorEnabled = 0u;
		uint32_t iReconstructedMaterialFeatureMask = 0u;
		uint32_t iRuntimeMaterialV2Enabled = 0u;
		uint32_t iRuntimeMaterialV2Opcode = 0u;
		uint32_t iRuntimeMaterialV2TextureLaneCount = 0u;
		uint32_t iRuntimeMaterialV2TextureMask = 0u;
		uint32_t iRuntimeMaterialV2DynamicConsumedMask = 0u;
		uint32_t iRuntimeMaterialV2DynamicSuppressedMask = 0u;
		uint32_t iRuntimeMaterialV2ParticleColorPolicy = 0u;
		uint32_t iRuntimeMaterialV2ParticleColorConsumedMask = 0u;
		uint32_t iRuntimeMaterialV2ParticleColorSuppressedMask = 0u;
		uint32_t iRuntimeMaterialV2ScalarCount = 0u;
		uint32_t iRuntimeMaterialV2VectorCount = 0u;
		uint32_t iRuntimeMaterialV2InputCount = 0u;
		std::array<uint32_t, 2u> RuntimeMaterialV2InputConsumedMask{};
		std::array<uint32_t, 2u> RuntimeMaterialV2InputSuppressedMask{};
		uint32_t iRuntimeMaterialV2StaticInputCount = 0u;
		uint32_t iRuntimeMaterialV2StaticSelectedMask = 0u;
		uint32_t iRuntimeMaterialV2StaticConsumedMask = 0u;
		uint32_t iRuntimeMaterialV2StaticSuppressedMask = 0u;
		uint32_t iRuntimeMaterialV2RenderInputCount = 0u;
		uint32_t iRuntimeMaterialV2RenderConsumedMask = 0u;
		uint32_t iRuntimeMaterialV2RenderSuppressedMask = 0u;
		uint32_t iStandardColorV1Enabled = 0u;
		std::array<uint32_t, 4u> StandardColorV1Header{};
		std::array<uint32_t, 4u> StandardColorV1BaseCoverage{};
		std::array<uint32_t, 4u> StandardColorV1Dissolve{};
		std::array<uint32_t, 4u> StandardColorV1Policies{};
		float4_t vStandardColorV1Scalars{};
		EFFECT_STANDARD_COLOR_V1_DESC StandardColorV1;
		/* Artist F V4 is a finite, occurrence-admitted visual program.  It
		   deliberately owns a separate opcode namespace from RuntimeMaterialV2;
		   SourceTextures remain the shared typed SRV carrier. */
		uint32_t iArtistVisualV4Opcode = 0u;
		uint32_t iArtistVisualV4TextureMask = 0u;
		std::array<float4_t, 8u> ArtistVisualV4Params{};
		std::array<float4_t, 2u> ArtistVisualV4Colors{};
		std::array<float4_t, 13u> RuntimeMaterialV2ScalarBlocks{};
		std::array<float4_t, 3u> RuntimeMaterialV2Vectors{};
		std::array<uint32_t, 3u>
			RuntimeMaterialV2VectorComponentConsumedMask{};
		std::array<uint32_t, 3u>
			RuntimeMaterialV2VectorComponentSuppressedMask{};
		std::array<ComPtr<ID3D11SamplerState>, 6u>
			RuntimeMaterialV2Samplers{};
		/* Import-time capture only.  The ordinary authored stage reconstructs
		   the same GPU resources from Material.Execution and never consults the
		   reconstructed source sidecars. */
		std::array<std::optional<EFFECT_MATERIAL_TEXTURE_LANE_DESC>, 6u>
			MaterialExecutionLanes{};
		float2_t vReconstructedUVScale{ 1.f, 1.f };
		float4_t vReconstructedPanRotationAux{};
		float4_t vReconstructedColor{ 1.f, 1.f, 1.f, 1.f };
		float4_t vReconstructedParams0{};
		float4_t vReconstructedParams1{};
		uint32_t iSourceTextureClampUMask = 0u;
		uint32_t iSourceTextureClampVMask = 0u;
		std::array<uint32_t, 4u> DynamicParameterSemantics{};
		EFFECT_GROUPED_TRANSLUCENT_CONSTANTS GroupedConstants;
		bool_t bSourceMaterialFallbackBlocked = false;
		bool_t bOccurrenceVisualSuppressed = false;
		std::shared_ptr<const EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING>
			pMaterialProgramBinding;
	};

namespace EffectDocumentRendererDetail
{

    bool Requires_StartingSceneCapture(const Client::EFFECT_DOCUMENT_DESC& Document);


    bool Is_StartingSceneCaptureCameraEmitter(const Client::EFFECT_ELEMENT_DESC& Element);


    // Project framing adapter only: endpoint geometry and all authored transforms stay intact.
    // Return without touching World on invalid bounds/camera or at the exact authored endpoint.
    void Fit_StartingCaptureMeshToCamera(const float3_t& BoundsMin, const float3_t& BoundsMax,
        const float4x4_t& View, const float4x4_t& Projection, const f32_t fProgress,
        float4x4_t& World);


    void Apply_StartingCaptureCameraFraming(const Client::EFFECT_DOCUMENT_DESC& Document,
        const Client::EFFECT_EVALUATED_PARTICLE& Particle, const Engine::CModel& Model,
        const f32_t fRootTimeSeconds, float4x4_t& World);


    // Copy the current CModel vertex ABI. Instance data never changes map/model vertices.
    inline const auto NativeMeshInstanceElements = []
    {
        std::array<D3D11_INPUT_ELEMENT_DESC, Engine::VTXMESH::iNumElements + 13u> Rows{};
        std::copy_n(Engine::VTXMESH::Elements, Engine::VTXMESH::iNumElements, Rows.begin());
        size_t i = Engine::VTXMESH::iNumElements;
        for (uint32_t Row = 0u; Row < 4u; ++Row)
            Rows[i++] = { "WORLD", Row, DXGI_FORMAT_R32G32B32A32_FLOAT,
                1u, Row * 16u, D3D11_INPUT_PER_INSTANCE_DATA, 1u };
        for (uint32_t Row = 0u; Row < 4u; ++Row)
            Rows[i++] = { "WORLDINVTRANSPOSE", Row, DXGI_FORMAT_R32G32B32A32_FLOAT,
                1u, 64u + Row * 16u, D3D11_INPUT_PER_INSTANCE_DATA, 1u };
        Rows[i++] = { "INSTANCE_COLOR", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT,
            1u, 128u, D3D11_INPUT_PER_INSTANCE_DATA, 1u };
        Rows[i++] = { "DYNAMIC", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT,
            1u, 144u, D3D11_INPUT_PER_INSTANCE_DATA, 1u };
        Rows[i++] = { "UVTRANSFORM", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT,
            1u, 160u, D3D11_INPUT_PER_INSTANCE_DATA, 1u };
        Rows[i++] = { "UVTRANSFORM", 1u, DXGI_FORMAT_R32G32B32A32_FLOAT,
            1u, 176u, D3D11_INPUT_PER_INSTANCE_DATA, 1u };
        Rows[i++] = { "PARTICLEDATA", 0u, DXGI_FORMAT_R32G32_FLOAT,
            1u, 192u, D3D11_INPUT_PER_INSTANCE_DATA, 1u };
        return Rows;
    }();

	bool_t Is_ZeroFloatBits(const FLOAT fValue);


	bool_t Is_DefaultStencilFace(
		const D3D11_DEPTH_STENCILOP_DESC& Face);


	bool_t Is_DefaultUnusedBlendTarget(
		const D3D11_RENDER_TARGET_BLEND_DESC& Target);


	bool_t Is_CompiledMaterialAdapter(
		const Client::EFFECT_COMPILED_MATERIAL_ADAPTER_DESC& Adapter);


	enum class COMPILED_ADAPTER_ACTUAL_BLEND : uint8_t
	{
		ALPHA_BLEND,
		ADDITIVE_BLEND,
	};

	bool_t Resolve_ActualMaterialAdapterPipelineReceipt(
		const Client::EFFECT_COMPILED_MATERIAL_ADAPTER_DESC& Adapter,
		const uint32_t iActualPassIndex,
		D3D11_CULL_MODE& eOutCullMode,
		bool_t& bOutDepthWrite,
		COMPILED_ADAPTER_ACTUAL_BLEND& eOutBlend);


	bool_t Validate_ActualMaterialAdapterFixedFunctionState(
		ID3D11DeviceContext* pContext,
		const Client::EFFECT_COMPILED_MATERIAL_ADAPTER_DESC& Adapter,
		const uint32_t iActualPassIndex);


	bool_t Validate_ActualMaterialAdapterPipeline(
		ID3D11DeviceContext* pContext,
		const Client::EFFECT_COMPILED_MATERIAL_ADAPTER_DESC& Adapter,
		const uint32_t iActualPassIndex);


	bool_t Validate_ActualLocalDecalSceneShaderResources(
		ID3D11DeviceContext* pContext);


	struct ARTIST31470_EMITTER_ROW_PIN final
	{
		uint32_t iOrder;
		std::string_view strEmitterRowSha256;
	};

	constexpr std::array<ARTIST31470_EMITTER_ROW_PIN, 35u>
		ARTIST31470_EMITTER_ROW_PINS = {{
		{ 0u, "5b7356ded7c7acd6958a0f6142e7323e15826799d463985faac4736e30810190" },
		{ 1u, "e9723f41e11a388cc25c49d0354dfb23c42988ce4150991403e6a40e106cc1ab" },
		{ 2u, "8879fb2dd9d2e5e03b0e5308f705d5a63ba7e8b4d4d28f0d2c8714b552bd300c" },
		{ 3u, "3cf36d8cbb5d065246f285198974019bd79b9c08543fa7a4ab37f171d149e8fd" },
		{ 4u, "95df71aea59355b1ee5e33ed7c835419f7fb81d2a7a93100e26fd6879dad3c7b" },
		{ 5u, "ac42ab12d87d656a686ae54ca92e3eaff8f071cf403b5c7f63622ea94fbfc218" },
		{ 6u, "aad0501641477b7de98b44995741a037e3523b8d82d5d4693734e6268d1216d1" },
		{ 7u, "91ca71963c4dd676a46b6ce94a1402f630f8a91aadf216b66fa1be9fb90b105b" },
		{ 8u, "29a2bfe52b5f18982e8c6d7dd986c96673e6f832a0b41de84140b6a6b76b9405" },
		{ 9u, "8816671187dedf2f13752c188a8e694780ed2960c57cfc72bea5092283a7e488" },
		{ 10u, "60b7fd2bbc97289fe49bf9232061b89ecdfa56116f5deeae2d605bb86c5f8a9c" },
		{ 11u, "99416894217b2799e4fb05d6e2e7e0f2fd9e0aea4ea1d67952792d237d4bde4b" },
		{ 12u, "aaf9843fb105d48afd1b714ca15582385ea1989f7d8764714eadfedb0eaa4cd5" },
		{ 13u, "28df4bc241309a88baa970cde323fea0026ca0dbaeedebccbbca4241bbc5f065" },
		{ 14u, "f941c08118629b9627b9f51094d6101b772544359c7e4d2ed3d18d9391cb7d52" },
		{ 15u, "90e3bd852a65684faef50bed8955dcfbbf32014ecf7e90b78e00d2266ecedf4c" },
		{ 16u, "48df03db9a3c64fe48fc6833336c70d50dd8ed4db2c1c4908daa9f40b6af16a5" },
		{ 17u, "926cf4eeb702114359031ca06a4c82f668074be0e0a2345337a94ec7a5356953" },
		{ 18u, "890416d755e4ae6bb0f1e3c5ec6e044c3aaa5b6e843edf26fa3547b47557c6b5" },
		{ 19u, "c979185b5c078b5e986f1322434f90a6eeda0406da5db89961aa0519aa7a5e52" },
		{ 20u, "db45e06aa718a8161a62c046f54e60b0ec271f659d6a1241c1bbf3bacf02306a" },
		{ 21u, "5fa10f3eb73cf2d5e0064f9b1c590158274f506dd0d7dea016b6b10ed531419d" },
		{ 22u, "f272a1f511f153c22728399e8d3ab20aabea5ef86c8e9bd573ff4ec230379f00" },
		{ 23u, "3025afbdf23d0acd403e991bf6b8bb6b5d44f38bad530c4afef3b291d0c7fd2b" },
		{ 24u, "5b437795eb0654fb51188e620cc6e06771d394da29760aed71619bb918f6ea8d" },
		{ 25u, "e5ae5dae0370f233306afbb8a1af178b3ceb53ba398de50f8d42baa9678610b5" },
		{ 26u, "88c1911a8122c0e5c2b6b7a1d48fe7e91628c1a64b5d0f8124902557a8b4bc22" },
		{ 27u, "e4957941115ed44d62f51de033f084b95c670ae8728c2bd699955d7f806bc3c8" },
		{ 28u, "de05e5a4a9b549190f8368fbee6b8f043ffb7db68bf67040be043d0c3b36b939" },
		{ 29u, "e9a471c39bdde31b984c6911d1580deb26ad731557831d548d1ac1b67b842385" },
		{ 30u, "2d06df4f3fd03da20cc787dca4ce282e8d8ff438a3b941a8cbcac766d83e21f5" },
		{ 31u, "28af70cd32eca7130e179d3dc1e96954016f598b616c743502083e80efbcb21b" },
		{ 32u, "3d4873252aaa8fc8a98f2511c4047b5295199053b6f15dd3906fda554b4aa720" },
		{ 33u, "b3c7e77416e2861bdb05e3a1cede96da4bfb4f7fef47377e339785b2cb2e3c5c" },
		{ 34u, "b3af4b7b34f7f3760fa0b4e4da261fdbed12210d7e93287a433bc510cf7a9de4" },
	}};

	constexpr bool Validate_Artist31470EmitterRowPins() noexcept
	{
		for (size_t i = 0u; i < ARTIST31470_EMITTER_ROW_PINS.size(); ++i)
		{
			const auto& Row = ARTIST31470_EMITTER_ROW_PINS[i];
			if (Row.iOrder != i || Row.strEmitterRowSha256.size() != 64u)
				return false;
			for (const char_t Character : Row.strEmitterRowSha256)
			{
				if (!((Character >= '0' && Character <= '9') ||
					(Character >= 'a' && Character <= 'f')))
					return false;
			}
			for (size_t j = 0u; j < i; ++j)
			{
				if (Row.strEmitterRowSha256 ==
					ARTIST31470_EMITTER_ROW_PINS[j].strEmitterRowSha256)
					return false;
			}
		}
		return true;
	}

	static_assert(Validate_Artist31470EmitterRowPins());

	class PIXEL_SHADER_SAMPLER_SCOPE final
	{
	public:
		explicit PIXEL_SHADER_SAMPLER_SCOPE(ID3D11DeviceContext* pContext)
			: m_pContext(pContext)
		{
		}

		~PIXEL_SHADER_SAMPLER_SCOPE()
		{
			Restore();
		}

		bool_t Apply(const std::span<const ComPtr<ID3D11SamplerState>> Samplers)
		{
			m_bLastFailureContractInvalid = false;
			if (nullptr == m_pContext)
				return false;
			if (m_bApplied || Samplers.empty() ||
				Samplers.size() > m_Previous.size() ||
				std::any_of(Samplers.begin(), Samplers.end(),
					[](const auto& Sampler) { return nullptr == Sampler; }))
			{
				m_bLastFailureContractInvalid = true;
				return false;
			}
			m_iCount = static_cast<uint32_t>(Samplers.size());
			std::array<ID3D11SamplerState*, 6u> PreviousRaw{};
			m_pContext->PSGetSamplers(5u, m_iCount, PreviousRaw.data());
			for (size_t i = 0u; i < m_iCount; ++i)
				m_Previous[i].Attach(PreviousRaw[i]);

			std::array<ID3D11SamplerState*, 6u> Desired{};
			for (size_t i = 0u; i < m_iCount; ++i)
				Desired[i] = Samplers[i].Get();
			m_pContext->PSSetSamplers(5u, m_iCount, Desired.data());

			bool_t bMatches = true;
#if defined(_DEBUG) || \
	defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
			std::array<ID3D11SamplerState*, 6u> Applied{};
			m_pContext->PSGetSamplers(5u, m_iCount, Applied.data());
			for (size_t i = 0u; i < m_iCount; ++i)
			{
				bMatches = bMatches && Applied[i] == Desired[i];
				if (nullptr != Applied[i])
					Applied[i]->Release();
			}
#endif
			m_bApplied = true;
			if (!bMatches)
				Restore();
			return bMatches;
		}

		bool_t Was_LastFailureContractInvalid() const
		{
			return m_bLastFailureContractInvalid;
		}

	private:
		void Restore()
		{
			if (!m_bApplied || nullptr == m_pContext)
				return;
			std::array<ID3D11SamplerState*, 6u> PreviousRaw{};
			for (size_t i = 0u; i < m_iCount; ++i)
				PreviousRaw[i] = m_Previous[i].Get();
			m_pContext->PSSetSamplers(5u, m_iCount, PreviousRaw.data());
			m_bApplied = false;
		}

		ID3D11DeviceContext* m_pContext = nullptr;
		std::array<ComPtr<ID3D11SamplerState>, 6u> m_Previous{};
		uint32_t m_iCount = 0u;
		bool_t m_bApplied = false;
		bool_t m_bLastFailureContractInvalid = false;
	};

	Client::EFFECT_GPU_RENDER_FAMILY Resolve_GpuRenderFamily(
		const Client::EFFECT_ELEMENT_DESC& Element);


	bool_t Is_DimensionSummonCharacterSurfaceCue(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		const Client::EFFECT_MODEL_CUE_DESC& Cue);


	template <size_t Size>
	int32_t NamedSourceTextureIndex(
		const std::array<std::string_view, Size>& Names,
		const std::string_view strName)
	{
		const auto Iterator = std::find(Names.begin(), Names.end(), strName);
		return Iterator == Names.end() ? -1 :
			static_cast<int32_t>(std::distance(Names.begin(), Iterator));
	}

	int32_t LinearFlowSourceTextureIndex(const std::string_view strName);


	int32_t BlacklineSourceTextureIndex(const std::string_view strName);


	int32_t LocalCrackSourceTextureIndex(const std::string_view strName);


	f32_t SourceScalar(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::string_view strName,
		const f32_t fFallback);


	f32_t SourceScalarAny(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::initializer_list<std::string_view> Names,
		const f32_t fFallback);


	bool_t SourceStaticSwitch(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::string_view strName,
		const bool_t bFallback);


	float4_t SourceVector(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::string_view strName,
		const float4_t& vFallback);


	void Build_LinearFlowConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 16u>& Parameters,
		float4_t& vMaskAColor,
		float4_t& vMaskBColor,
		float4_t& vAuxiliary0,
		float4_t& vAuxiliary1);


	void Build_BlacklineConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 16u>& Parameters,
		float4_t& vDiffuseColor,
		float4_t& vMaskColor);


	void Build_LocalCrackConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 5u>& Parameters,
		float4_t& vOutColor,
		float4_t& vInColor,
		float4_t& vReflectionColor);


	void Build_SliceConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		float4_t& vScalars0,
		float4_t& vScalars1,
		float4_t& vAuxiliary);


	void Build_MissileTrailConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters);


	void Build_WaterTrailConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters);


	void Build_MakeFlowConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters);


	void Build_MakeFlow03SpriteConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters);


	void Build_ParticleTrailConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters);


	void Build_RingConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters);


	void Build_ParticleMasterConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters,
		float4_t& vSourceColor);


	void Build_SpriteWaveConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters,
		float4_t& vEdgeColor);


	void Build_ArtistSpla01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters);


	void Build_ArtistSpla05Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters);


	void Build_ArtistTwinkleConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters);


	void Build_ArtistFluid01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters);


	void Build_ArtistWorldOffset01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters);


	void Build_ArtistLensFlare01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters);


	void Build_Glasshole02Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters,
		float4_t& vAuraColor,
		float4_t& vInHoleColor);


	void Build_FluidNinja01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters,
		float4_t& vColor1,
		float4_t& vColor2);


	void Build_CustomParticle01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters,
		float4_t& vDiffuseColor);


	void Build_CrackholeV2Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters,
		float4_t& vEmissionColor,
		float4_t& vBaseColor);


	void Build_Simple01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters);


	/* fx_mm_basic_01_ad / _tr.  The grouped path collapsed this master material
	   to one pan and one gray carrier, which loses the two independent uv_noise
	   domains and treats the dedicated alpha_tex as if it were artwork.  Lane
	   assignment here follows the parent parameter groups: emissive_tex and
	   alpha_tex are the "emissive" group, uv_noise_01/02 are the "uv_noise"
	   group with their own tiling and panning.

	   fresnel_power, edge_power, edge_intensity, depth_alpha_bias,
	   camera_distance and world_normal_intensity are deliberately not packed.
	   They need scene depth and world normal inputs that the RT0 base pass does
	   not carry, and inventing them would change coverage without evidence. */
	void Build_MmBasic01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters);


	/* fx_k_me_flowtrail_01_ts_tr.  Three source groups, three UV domains:
	   diff owns radiance, opacity owns coverage and noise offsets both.  The
	   wave group (wave_str, wave_tile, wave_pan_speed, wave_noise_str) is not
	   packed: no child in the corpus overrides wave_tile or wave_pan_speed and
	   the parent expression graph is not in evidence, so its geometry would be
	   invented.  cameravec_pow needs a camera vector the RT0 base pass does not
	   carry.  Both stay in the NATIVE_PARITY backlog. */
	void Build_FlowTrail01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters);


	void Build_Simple02Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters,
		float4_t& vEmissiveColor);


	void Build_MmFluid01SpriteConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters);


	void Build_FlowRibbon01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::string_view strSourceMaterialPath,
		std::array<float4_t, 8u>& Parameters);


	bool_t Has_LinearFlowSourceTextureContract(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source);


	bool_t Has_BlacklineSourceTextureContract(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source);


	bool_t Has_LocalCrackSourceTextureContract(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source);


	size_t Texture_Index(const Client::EFFECT_RESOURCE_SLOT eSlot);


	ComPtr<ID3D11ShaderResourceView> Find_Texture(
		const std::array<ComPtr<ID3D11ShaderResourceView>, 8>& Textures,
		const Client::EFFECT_RESOURCE_SLOT eSlot);


	const Client::EFFECT_RESOURCE_BINDING_DESC* Find_Binding(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const Client::EFFECT_RESOURCE_SLOT eSlot);


	constexpr uint32_t ARTIST_D_BLACK_TIGER_STROKE_OPCODE = 18u;

	struct ARTIST_D_BLACK_TIGER_STROKE_ROW final
	{
		std::string_view strElementId;
		std::string_view strSourceElementId;
		std::string_view strDynamicModuleStableId;
		uint32_t iScalarCount;
	};

	constexpr std::array<ARTIST_D_BLACK_TIGER_STROKE_ROW, 12u>
		ARTIST_D_BLACK_TIGER_STROKE_ROWS = {{
		{ "authored.source-particle.763aea38ab1100ba9072dbfb",
			"fx_pc_sdm_08.par_l_sdm_sk_01_3.particlespriteemitter_5",
			"FX_PC_MSR_03:export:2587@ref:4", 28u },
		{ "authored.source-particle.e6c3ffec9fbc27024e2ce78c",
			"fx_pc_sdm_08.par_l_sdm_sk_01_3.particlespriteemitter_5.event_source-event-002",
			"FX_PC_MSR_03:export:2587@ref:4", 28u },
		{ "authored.source-particle.91392dd3a1710c9d411bfff6",
			"fx_pc_sdm_08.par_l_sdm_sk_01_3.particlespriteemitter_6",
			"FX_PC_SDM_08:export:1241@ref:3", 24u },
		{ "authored.source-particle.382ed3229ddf083cfd22ee11",
			"fx_pc_sdm_08.par_l_sdm_sk_01_3.particlespriteemitter_6.event_source-event-002",
			"FX_PC_SDM_08:export:1241@ref:3", 24u },
		{ "authored.source-particle.4f0381d175d441978f26ebfc",
			"fx_pc_sdm_08.par_l_sdm_sk_01_3.particlespriteemitter_7",
			"FX_PC_SDM_08:export:1240@ref:3", 24u },
		{ "authored.source-particle.31fa700c084ab0b11447f7c7",
			"fx_pc_sdm_08.par_l_sdm_sk_01_3.particlespriteemitter_7.event_source-event-002",
			"FX_PC_SDM_08:export:1240@ref:3", 24u },
		{ "authored.source-particle.5571970d95f97aecb889fed7",
			"fx_pc_sdm_08.par_l_sdm_sk_05_3.particlespriteemitter_5",
			"FX_PC_MSR_03:export:2587@ref:4", 28u },
		{ "authored.source-particle.87c8abd0423fcb7e9a725659",
			"fx_pc_sdm_08.par_l_sdm_sk_05_3.particlespriteemitter_6",
			"FX_PC_SDM_08:export:1241@ref:3", 24u },
		{ "authored.source-particle.ac2d4d3e467dc4442cba60c3",
			"fx_pc_sdm_08.par_l_sdm_sk_05_3.particlespriteemitter_11",
			"FX_PC_SDM_08:export:1240@ref:3", 24u },
		{ "authored.source-particle.01c398219f73706b66509e77",
			"fx_pc_sdm_08.par_l_sdm_sk_06_3.particlespriteemitter_5",
			"FX_PC_MSR_03:export:2587@ref:4", 28u },
		{ "authored.source-particle.93420edbc5815b8a01b38ef4",
			"fx_pc_sdm_08.par_l_sdm_sk_06_3.particlespriteemitter_6",
			"FX_PC_SDM_08:export:1241@ref:3", 24u },
		{ "authored.source-particle.76d0b67fe194395ce21c51ab",
			"fx_pc_sdm_08.par_l_sdm_sk_06_3.particlespriteemitter_2",
			"FX_PC_SDM_08:export:1240@ref:3", 24u },
	}};

	constexpr std::array<f32_t, 28u> ARTIST_D_TIGER_CHILD5_SCALARS = {{
		0.f, 0.f, 1.f, 1.f, 1.f, 1.100000023841858f, 1.f, 0.f,
		1.f, 4.f, 0.10000000149011612f, 0.f, 0.f, 0.f,
		-0.10000000149011612f, 0.f, 2.f, 1.f, 1.f, 2.f, 3.f,
		-0.20000000298023224f, 0.f, 0.f, 0.5f, 0.5f, 0.f, 0.f,
	}};

	constexpr std::array<f32_t, 24u> ARTIST_D_TIGER_CHILD6_SCALARS = {{
		0.f, 0.f, 1.f, 1.f, 2.f, 1.2000000476837158f,
		0.10000000149011612f, 5.f, 25.f, 15.f, 0.f,
		-0.10000000149011612f, 0.f, 0.f, 1.f, 1.f, 1.f,
		-0.800000011920929f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f,
	}};

	const ARTIST_D_BLACK_TIGER_STROKE_ROW* Find_ArtistDBlackTigerStrokeRow(
		const std::string_view strElementId);


	bool_t Is_ArtistDBlackTigerSampler(
		const Client::EFFECT_MATERIAL_SAMPLER_DESC& Sampler);


	bool_t Is_ArtistDBlackTigerLane(
		const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane,
		const uint32_t iIndex,
		const std::string_view strRole,
		const std::string_view strAssetId,
		const std::string_view strSourceChannel);


	template <size_t ScalarCount>
	bool_t Is_ArtistDBlackTigerScalars(
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution,
		const std::array<f32_t, ScalarCount>& Expected)
	{
		if (Execution.Scalars.size() != Expected.size())
			return false;
		for (size_t i = 0u; i < Expected.size(); ++i)
		{
			const Client::EFFECT_MATERIAL_SCALAR_PARAMETER_DESC& Scalar =
				Execution.Scalars[i];
			if (Scalar.strName != "scalar." + std::to_string(i) ||
				Scalar.iPackedIndex != i || Scalar.fValue != Expected[i])
			{
				return false;
			}
		}
		return true;
	}

	bool_t Has_ArtistDBlackTigerDynamicModule(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strExpectedStableId);


	bool_t Validate_ArtistDBlackTigerStrokeExecution(
		const Client::EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError);


	constexpr uint32_t LANCE_DRAGON_MASKED_OPCODE = 19u;
	constexpr uint32_t PROJECT_BASE_COVERAGE_EMISSIVE_DISSOLVE_RECT_OPCODE = 21u;
	constexpr uint32_t WARLORD_WPO_SINWAVE_ELECTRIC_RT0_OPCODE = 22u;

	struct WARLORD_WPO_SINWAVE_ROW final
	{
		std::string_view strElementId;
		std::string_view strSourceNode;
	};

	constexpr std::array<WARLORD_WPO_SINWAVE_ROW, 2u>
		WARLORD_WPO_SINWAVE_ROWS = {{
		{ "authored.source-particle.8c0d6ab070c1a6c83479e590",
			"authored-source-particle:effect.warlord.skill.17140.unified|"
			"source:effect.warlord.skill.17140.imported|element:fx_pc_wgl_07."
			"par_s_wgl_guardianlightning_01.particlespriteemitter_13" },
		{ "authored.source-particle.59e6ffa8852fba74279b6ae9",
			"authored-source-particle:effect.warlord.skill.17140.unified|"
			"source:effect.warlord.skill.17140.imported|element:fx_pc_wgl_07."
			"par_s_wgl_guardianlightning_02.particlespriteemitter_52" },
	}};

	const WARLORD_WPO_SINWAVE_ROW* Find_WarlordWpoSinWaveRow(
		const std::string_view strElementId);


	bool_t Has_WarlordWpoSinWaveDynamicModule(
		const Client::EFFECT_ELEMENT_DESC& Element);


	bool_t Validate_WarlordWpoSinWaveElectricExecution(
		const Client::EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError);


	struct LANCE_DRAGON_MASKED_ROW final
	{
		std::string_view strElementId;
		std::string_view strSourceNode;
		std::string_view strMeshAssetId;
		std::string_view strSourceMaterialPath;
		std::string_view strDynamicModuleStableId;
		bool_t bBody;
	};

	constexpr std::array<LANCE_DRAGON_MASKED_ROW, 12u>
		LANCE_DRAGON_MASKED_ROWS = {{
		{ "authored.source-particle.2b0f00d91a20ba785ba034ec",
			"authored-source-particle:effect.lancemaster.skill.34630.clip1.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_14",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_01_msk",
			"FX_PC_DDK_03:export:4292@ref:5", true },
		{ "authored.source-particle.71ac47f40d13b3a7ca6ed561",
			"authored-source-particle:effect.lancemaster.skill.34630.clip1.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_15",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01_dragon.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_02_msk",
			"FX_PC_DDK_03:export:4292@ref:5", false },
		{ "authored.source-particle.aa3beb2d7ebbe4922f6df595",
			"authored-source-particle:effect.lancemaster.skill.34630.clip1.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_start.particlespriteemitter_3",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_01_msk",
			"FX_PC_DDK_03:export:4292@ref:5", true },
		{ "authored.source-particle.237b5cd9d1fafb4b95b41212",
			"authored-source-particle:effect.lancemaster.skill.34630.clip1.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_start.particlespriteemitter_4",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01_dragon.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_02_msk",
			"FX_PC_DDK_03:export:4292@ref:5", false },
		{ "authored.source-particle.6542736b94e7b9cd8ed5f2fd",
			"authored-source-particle:effect.lancemaster.skill.34630.clip2.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_14.event_source-event-033",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_01_msk",
			"FX_PC_DDK_03:export:4292@ref:5", true },
		{ "authored.source-particle.85571ac576a68cc3ff037cae",
			"authored-source-particle:effect.lancemaster.skill.34630.clip2.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_15.event_source-event-033",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01_dragon.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_02_msk",
			"FX_PC_DDK_03:export:4292@ref:5", false },
		{ "authored.source-particle.92af24faaaeb30c6ac77d37c",
			"authored-source-particle:effect.lancemaster.skill.34630.clip3.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_14.event_source-event-048",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_01_msk",
			"FX_PC_DDK_03:export:4292@ref:5", true },
		{ "authored.source-particle.80d7156c29e9bf140631ad2e",
			"authored-source-particle:effect.lancemaster.skill.34630.clip3.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_15.event_source-event-048",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01_dragon.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_02_msk",
			"FX_PC_DDK_03:export:4292@ref:5", false },
		{ "authored.source-particle.538fe0779d0718d30b68ef11",
			"authored-source-particle:effect.lancemaster.skill.34630.clip4.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_end.particlespriteemitter_0",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_01_msk",
			"FX_FS_AV_08:export:509@ref:5", true },
		{ "authored.source-particle.50385d998091ed0e55a047f8",
			"authored-source-particle:effect.lancemaster.skill.34630.clip4.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_end.particlespriteemitter_1",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01_dragon.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_02_msk",
			"FX_PC_DDK_03:export:4292@ref:5", false },
		{ "authored.source-particle.0a019ebaff2bb55941d23ab8",
			"authored-source-particle:effect.lancemaster.skill.34650.clip1.unified|source:effect.lance_master.skill.34650.imported|element:fx_pc_flm_08.par_t_flm_dragoncleave_01_wpcast_01_s.particlespriteemitter_14",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_01_msk",
			"FX_PC_DDK_03:export:4292@ref:5", true },
		{ "authored.source-particle.65b74589de96c3f44e625f24",
			"authored-source-particle:effect.lancemaster.skill.34650.clip1.unified|source:effect.lance_master.skill.34650.imported|element:fx_pc_flm_08.par_t_flm_dragoncleave_01_wpcast_01_s.particlespriteemitter_15",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01_dragon.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_02_msk",
			"FX_PC_DDK_03:export:4292@ref:5", false },
	}};

	constexpr std::array<std::string_view, 25u> LANCE_DRAGON_SCALAR_NAMES = {{
		"02.n.uvscale.x", "03.n.uvscale.y", "05.n.panning.x",
		"06.n.panning.y", "11.normalmap.str", "02.map_a_uvscale_r",
		"03.map_a_uvscale_g", "04.map_a_panning_x", "05.map_a_panning_y",
		"36.str", "37.power", "03.emap_uv.x.scale", "04.emap_uv.y.scale",
		"15.emissiion_power", "02.uvscale.x", "03.uvscale.y",
		"91.desaturation", "92.emissiion_power", "05.specmap_uvscale.x",
		"06.specmap_uvscale.y", "02.specmap_str", "07.desaturation",
		"08.specmap_power", "05.power", "06.str",
	}};

	constexpr std::array<f32_t, 25u> LANCE_DRAGON_SCALAR_VALUES = {{
		1.f, 1.f, 0.f, 0.f, 1.5f, 10.f, 10.f, 0.f, -0.125f, 10.f,
		1.f, 1.f, 1.f, 0.1f, 1.f, 1.f, 0.f, 1.f, 1.f, 1.f, 0.25f,
		0.f, 1.2f, 2.f, 1.f,
	}};

	const LANCE_DRAGON_MASKED_ROW* Find_LanceDragonMaskedRow(
		const std::string_view strElementId);


	bool_t Has_LanceDragonDynamicModule(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strExpectedStableId);


	bool_t Is_LanceDragonLane(
		const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane,
		const uint32_t iIndex,
		const std::string_view strRole,
		const std::string_view strAssetId,
		const std::string_view strChannel);


	bool_t Validate_LanceDragonMaskedExecution(
		const Client::EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError);


	constexpr uint32_t DIMENSIONMASTER_WATER_DROPLET_BURST_OPCODE = 1003u;
	constexpr uint32_t DIMENSIONMASTER_GLASS_MIRROR_MESH_OPCODE = 1004u;

	bool_t Validate_DimensionMasterWaterDropletBurstExecution(
		const Client::EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError);


	bool_t Validate_DimensionMasterGlassMirrorMeshExecution(
		const Client::EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError);


	bool_t Validate_DimensionMasterGlassMirrorDocumentOccurrence(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);


	bool_t Validate_DimensionMasterProjectTunedDocumentExecution(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);


	bool_t Is_SourceMaterialFallbackBlocked(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const Client::EFFECT_GROUPED_TRANSLUCENT_CONSTANTS& GroupedConstants);


	bool_t Same_MaterialFloat4(
		const float4_t& Left,
		const float4_t& Right);


	bool_t Same_MaterialSampler(
		const Client::EFFECT_MATERIAL_SAMPLER_DESC& Left,
		const Client::EFFECT_MATERIAL_SAMPLER_DESC& Right);


	bool_t Same_MaterialTextureLanes(
		const std::vector<Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC>& Left,
		const std::vector<Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC>& Right);


	bool_t Same_MaterialScalars(
		const std::vector<Client::EFFECT_MATERIAL_SCALAR_PARAMETER_DESC>& Left,
		const std::vector<Client::EFFECT_MATERIAL_SCALAR_PARAMETER_DESC>& Right);


	bool_t Same_MaterialVectors(
		const std::vector<Client::EFFECT_MATERIAL_VECTOR_PARAMETER_DESC>& Left,
		const std::vector<Client::EFFECT_MATERIAL_VECTOR_PARAMETER_DESC>& Right);


	bool_t Same_StandardColorV1(
		const Client::EFFECT_STANDARD_COLOR_V1_DESC& Left,
		const Client::EFFECT_STANDARD_COLOR_V1_DESC& Right);


	uint32_t StandardColorChannelMask(
		const Client::EFFECT_STANDARD_COLOR_CHANNEL eChannel);


	uint32_t StandardColorSourceChannelMask(const std::string_view strChannel);


	uint32_t StandardColorSrvChannelMask(const DXGI_FORMAT eFormat);


	bool_t Is_StandardColorSrgbFormat(const DXGI_FORMAT eFormat);


	bool_t Same_MaterialExecutionResourceSignature(
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Left,
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Right);


	bool_t Same_TypedDynamicParameterResourceSignature(
		const Client::EFFECT_CASCADE_RECIPE_DESC& Left,
		const Client::EFFECT_CASCADE_RECIPE_DESC& Right);


	bool_t Resource_SignatureMatches(
		const Client::EFFECT_DOCUMENT_DESC& Left,
		const Client::EFFECT_DOCUMENT_DESC& Right);


	float3_t To_Float3(const vector_t Value);


	bool_t Normalize_Safe(const vector_t Value, vector_t& Out);


	Client::EFFECT_COLOR_DESC Evaluate_CommonColor(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const f32_t fNormalizedLife);


	float4x4_t Make_BillboardWorld(
		const float4x4_t& Source,
		const f32_t fRollDegrees);


	struct PARTICLE_SPRITE_WORLD_CONTEXT final
	{
		float4x4_t CameraWorld{};
		const Client::EFFECT_ELEMENT_DESC* pNativeVelocityElement = nullptr;
		uint32_t iNativeVelocityProfile = UINT32_MAX;
		bool_t bCameraCaptured = false;
		bool_t bNativeVelocityResolved = false;
		bool_t bNativeVelocityBasis = false;
	};

	bool_t Make_ParticleSpriteWorld(
		const Client::EFFECT_EVALUATED_PARTICLE& Particle,
		float4x4_t& OutWorld,
		const uint32_t iSourceMaterialProfile,
		PARTICLE_SPRITE_WORLD_CONTEXT& Context);


	struct PARTICLE_CLIP_CONTEXT final
	{
		matrix_t View{};
		matrix_t Projection{};
		matrix_t MagnitudeViewProjection{};
		bool_t bEnabled = false;
	};

	bool_t Is_ParticleQuadOutsideViewXY(
		const float4x4_t& World, const PARTICLE_CLIP_CONTEXT& Context);



	float4x4_t Apply_ParticleCameraOffset(
		const Client::EFFECT_EVALUATED_PARTICLE& Particle);


	bool_t Uses_SourceLockedAxisMeshFacing(
		const Client::EFFECT_ELEMENT_DESC& Element);


	bool_t Make_SourceLockedAxisMeshWorld(
		const Client::EFFECT_EVALUATED_PARTICLE& Particle,
		float4x4_t& OutWorld);


	f32_t SourceLiteralNumber(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strPropertyPath,
		const f32_t fFallback);


	bool_t SourceLiteralBool(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strPropertyPath,
		const bool_t bFallback);


	std::string_view SourceLiteralString(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strPropertyPath);


	struct SOURCE_SPRITE_DEPTH_ORDER final
	{
		const Client::EFFECT_EVALUATED_PARTICLE* pParticle = nullptr;
		f32_t fClipDepth = 0.f;
	};

	bool_t Requires_SourceSpriteDepthSort(
		const Client::EFFECT_ELEMENT_DESC& Element);


	bool_t Build_SourceSpriteDepthOrder(
		const std::span<const Client::EFFECT_EVALUATED_PARTICLE> Particles,
		const Client::EFFECT_ELEMENT_DESC& Source,
		const float4x4_t& View, const float4x4_t& Projection,
		std::vector<SOURCE_SPRITE_DEPTH_ORDER>& OutOrder,
		std::string& OutError);


	bool_t SourceMaterialIdentityMatches(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::string_view strProfileId,
		const std::string_view strParentMaterialPath);


	uint32_t SourceMaterialProfileIndex(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source);


	bool_t BindingMatches(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const Client::EFFECT_RESOURCE_SLOT eSlot,
		const std::string_view strAssetId);


	bool_t NamedTextureMatches(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::string_view strName,
		const std::string_view strAssetId);


	bool_t Is_DimensionMasterDBoundarySpriteWaveContract(
		const Client::EFFECT_ELEMENT_DESC& Element);


	bool_t Is_DimensionMasterDBoundaryParticleMasterContract(
		const Client::EFFECT_ELEMENT_DESC& Element);


	bool_t Is_FamilyProfileCarrierContractSatisfied(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const uint32_t iProfile);


	bool_t Is_StrictParticleShapeCarrierContractSatisfied(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strRequiredShape);


	bool_t Is_StrictParticleBlendCarrierContractSatisfied(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const bool_t bAdditive);


	bool_t Is_StrictTwoSidedAlphaMeshCarrierContractSatisfied(
		const Client::EFFECT_ELEMENT_DESC& Element);


	bool_t Is_MissileTrailFourLaneCarrierContractSatisfied(
		const Client::EFFECT_ELEMENT_DESC& Element);


	uint32_t EffectiveSourceMaterialProfileIndex(
		const Client::EFFECT_ELEMENT_DESC& Element);


	uint32_t DynamicParameterSemanticIndex(const std::string& strSemantic);


	uint32_t TypedDynamicParameterSemanticIndex(
		const uint32_t iProfile,
		const std::string_view strParameterName);


	bool_t Try_ResolveTypedDynamicParameterSemantics(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const uint32_t iProfile,
		std::array<uint32_t, 4u>& OutSemantics);


	struct SOURCE_SUBUV_LAYOUT final
	{
		const Client::EFFECT_ELEMENT_DESC* pElement = nullptr;
		uint32_t iColumns = 1u;
		uint32_t iRows = 1u;
		bool_t bEnabled = false;
		bool_t bLinearBlend = false;
	};

	SOURCE_SUBUV_LAYOUT Resolve_SourceSubUVLayout(
		const Client::EFFECT_ELEMENT_DESC& Element);


	Client::EFFECT_SUBUV_FRAME_DESC Resolve_SubUVFrames(
		const Client::EFFECT_EVALUATED_PARTICLE& Particle,
		const SOURCE_SUBUV_LAYOUT& BatchLayout);

}
using namespace EffectDocumentRendererDetail;


struct Client::CEffectDocumentRenderer::PREWARM_ASSET_CACHE final
{
	std::unordered_map<std::string, std::shared_ptr<Engine::CModel>>
		NonAnimatedModels;
	std::unordered_map<std::string, std::shared_ptr<Engine::CModel>>
		AnimatedModelPrototypes;
	std::unordered_map<std::string, ComPtr<ID3D11ShaderResourceView>> Textures;
};

struct Client::CEffectDocumentRenderer::PREPARED_DOCUMENT final
{
	uint64_t iCatalogRevision = 0u;
	uint64_t iMaterialProgramRegistryGeneration = 0u;
	uint64_t iResourceSignature = 0u;
	std::string strEffectAssetId;
	const EFFECT_DOCUMENT_DESC* pCatalogDocumentIdentity = nullptr;
	std::shared_ptr<const EFFECT_DOCUMENT_DESC> pImmutableDocument;
	EFFECT_DOCUMENT_DESC ResourceDocument;
	std::shared_ptr<const CEffectPlayback::PREPARED_RESOURCES>
		pPlaybackResources;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		pReconstructedRuntimePreparation;
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pVisualProgramProjection;
	std::shared_ptr<const CEffectMaterialProgramRegistry>
		pMaterialProgramRegistry;
	uint32_t iReconstructedNeutralBaseCount = 0u;
	uint32_t iReconstructedOneLayerCount = 0u;
	uint32_t iReconstructedMaterialEvaluatorCount = 0u;
	uint32_t iRuntimeMaterialV2Count = 0u;
	uint32_t iVisualProgramAdapterCount = 0u;
	uint32_t iArtistVisualV4Count = 0u;
	uint32_t iArtistVisualV4UnsupportedCount = 0u;
	uint32_t iLegacyOccurrenceVisualSuppressedCount = 0u;
	uint32_t iMaterialProgramResolvedElementCount = 0u;
	std::unordered_map<std::string, ELEMENT_RESOURCE> ElementResources;
	std::unordered_map<std::string, MODEL_CUE_RESOURCE>
		ModelCuePrototypes;
	std::shared_ptr<Engine::CVIBuffer_DynamicTrail> pTrailBuffer;
};

struct Client::CEffectDocumentRenderer::PRODUCT_PREWARM_SESSION final
{
	ID3D11Device* pDevice = nullptr;
	ID3D11DeviceContext* pContext = nullptr;
	uint64_t iCatalogRevision = 0u;
	std::shared_ptr<const CEffectMaterialProgramRegistry>
		pMaterialProgramRegistry;
	PREWARM_ASSET_CACHE SharedAssets;
};

struct Client::CEffectDocumentRenderer::RECONSTRUCTED_DIAGNOSTIC_COMPOSITE final
{
	struct GPU_RESOURCE final
	{
		std::shared_ptr<Engine::CModel> pModel;
		std::array<ComPtr<ID3D11ShaderResourceView>, 2u> Textures;
		std::array<ComPtr<ID3D11SamplerState>, 2u> Samplers;
		ComPtr<ID3D11BlendState> pBlendState;
		ComPtr<ID3D11RasterizerState> pRasterizerState;
		ComPtr<ID3D11DepthStencilState> pDepthStencilState;
		D3D11_BLEND_DESC BlendDescriptor{};
		D3D11_RASTERIZER_DESC RasterizerDescriptor{};
		D3D11_DEPTH_STENCIL_DESC DepthStencilDescriptor{};
		std::array<D3D11_SAMPLER_DESC, 2u> SamplerDescriptors{};
		bool_t bHasBlendDescriptor = false;
		bool_t bHasRasterizerDescriptor = false;
		bool_t bHasDepthStencilDescriptor = false;
		ComPtr<ID3D11Query> pPipelineStatisticsQuery;
		bool_t bPipelineStatisticsPending = false;
		uint64_t iDrawCount = 0u;
		D3D11_QUERY_DATA_PIPELINE_STATISTICS PipelineStatistics{};
	};

	std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_FRAME> pFrame;
	std::array<GPU_RESOURCE, 2u> Resources;
};
namespace EffectDocumentRendererDetail
{

	struct EFFECT_RENDERER_CORE final
	{
		std::array<shared_ptr<Engine::CShader>, Client::EFFECT_SHADER_PROGRAMS.size()> ShaderPrograms;
		shared_ptr<Engine::CShader> pMeshShader;
		shared_ptr<Engine::CShader> pAnimatedModelShader;
		shared_ptr<Engine::CShader> pNativeScreenPostShader;
		shared_ptr<Engine::CShader> pRectShader;
		shared_ptr<Engine::CShader> pParticleShader;
		shared_ptr<Engine::CShader> pTrailShader;
		shared_ptr<Engine::CShader> pDecalShader;
		shared_ptr<Engine::CVIBuffer_Rect> pRect;
		shared_ptr<Engine::CVIBuffer_ParticleRect> pParticleBuffer;
		ComPtr<ID3D11ShaderResourceView> pWhiteTexture;
		ComPtr<ID3D11ShaderResourceView> pBlackTexture;
	};

	struct PREPARED_KEY final
	{
		uint64_t iCatalogRevision = 0u;
		std::string strEffectAssetId;
		uint64_t iResourceSignature = 0u;
		std::string strVisualProgramTokenSha256;

		bool operator<(const PREPARED_KEY& Right) const
		{
			return std::tie(iCatalogRevision, strEffectAssetId,
				iResourceSignature, strVisualProgramTokenSha256) <
				std::tie(Right.iCatalogRevision, Right.strEffectAssetId,
					Right.iResourceSignature,
					Right.strVisualProgramTokenSha256);
		}
	};
extern std::mutex g_EffectRenderCacheMutex;

extern std::mutex g_EffectRendererCoreBuildMutex;

extern std::unordered_map<ID3D11Device*, std::shared_ptr<EFFECT_RENDERER_CORE>>
		g_EffectRendererCores;

extern std::unordered_map<ID3D11Device*, std::string>
		g_EffectRendererCoreFailures;

extern std::map<PREPARED_KEY,
		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>>
		g_PreparedEffectDocuments;

extern std::unordered_map<const Client::EFFECT_DOCUMENT_DESC*,
		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>>
		g_PreparedEffectDocumentsByIdentity;

extern std::shared_ptr<Client::CEffectDocumentRenderer::PRODUCT_PREWARM_SESSION>
		g_pProductPrewarmSession;

extern ID3D11Device* g_pPreparedDevice;

extern uint64_t g_iPreparedCatalogRevision;

extern uint64_t g_iPreparedCatalogGeneration;

extern Client::EFFECT_RENDER_PREWARM_PROBE g_EffectRenderPrewarmProbe;


	class CTargetPreparationTimer final
	{
	public:
		CTargetPreparationTimer()
			: m_Started(std::chrono::steady_clock::now())
		{
		}

		~CTargetPreparationTimer()
		{
			const uint64_t iElapsedMicroseconds = static_cast<uint64_t>(
				std::chrono::duration_cast<std::chrono::microseconds>(
					std::chrono::steady_clock::now() - m_Started).count());
			const std::scoped_lock Lock(g_EffectRenderCacheMutex);
			++g_EffectRenderPrewarmProbe.iTargetPrepareCount;
			g_EffectRenderPrewarmProbe.iTargetPrepareMaximumMicroseconds =
				(std::max)(
					g_EffectRenderPrewarmProbe.iTargetPrepareMaximumMicroseconds,
					iElapsedMicroseconds);
		}

	private:
		std::chrono::steady_clock::time_point m_Started;
	};

	bool_t Read_ReconstructedAssetBytes(
		const std::string& strAssetId,
		const uint64_t iExpectedByteCount,
		const std::string& strExpectedSha256,
		std::filesystem::path& OutPath,
		std::vector<uint8_t>& OutBytes,
		std::string& strOutError);


	std::string Sha256Hex(const std::array<uint8_t, 32u>& Bytes);


	bool_t Same_SamplerDescriptor(
		const D3D11_SAMPLER_DESC& Left,
		const D3D11_SAMPLER_DESC& Right);


	bool_t Is_AnisotropicSamplerFilter(const D3D11_FILTER Filter);


	bool_t Same_RuntimeSamplerReadbackDescriptor(
		const D3D11_SAMPLER_DESC& Actual,
		const D3D11_SAMPLER_DESC& Requested,
		const D3D11_SAMPLER_DESC& FrozenAuthority);


	bool_t Materialize_RuntimeSamplerDescriptor(
		const D3D11_SAMPLER_DESC& Authority,
		D3D11_SAMPLER_DESC& OutRuntime);


	bool_t Try_ToAuthoredFilter(
		const D3D11_FILTER Source,
		Client::EFFECT_MATERIAL_TEXTURE_FILTER& Out);


	bool_t Try_ToD3dFilter(
		const Client::EFFECT_MATERIAL_TEXTURE_FILTER Source,
		D3D11_FILTER& Out);


	bool_t Try_ToAuthoredAddress(
		const D3D11_TEXTURE_ADDRESS_MODE Source,
		Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE& Out);


	bool_t Try_ToD3dAddress(
		const Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE Source,
		D3D11_TEXTURE_ADDRESS_MODE& Out);


	bool_t Try_ToAuthoredComparison(
		const D3D11_COMPARISON_FUNC Source,
		Client::EFFECT_MATERIAL_COMPARISON_FUNCTION& Out);


	bool_t Try_ToD3dComparison(
		const Client::EFFECT_MATERIAL_COMPARISON_FUNCTION Source,
		D3D11_COMPARISON_FUNC& Out);


	bool_t Try_ToAuthoredSampler(
		const D3D11_SAMPLER_DESC& Source,
		Client::EFFECT_MATERIAL_SAMPLER_DESC& Out);


	bool_t Try_ToD3dSampler(
		const Client::EFFECT_MATERIAL_SAMPLER_DESC& Source,
		D3D11_SAMPLER_DESC& Out);


	bool_t Same_BlendDescriptor(
		const D3D11_BLEND_DESC& Left,
		const D3D11_BLEND_DESC& Right);


	bool_t Same_RasterizerDescriptor(
		const D3D11_RASTERIZER_DESC& Left,
		const D3D11_RASTERIZER_DESC& Right);


	bool_t Same_DepthStencilOperation(
		const D3D11_DEPTH_STENCILOP_DESC& Left,
		const D3D11_DEPTH_STENCILOP_DESC& Right);


	bool_t Same_DepthStencilDescriptor(
		const D3D11_DEPTH_STENCIL_DESC& Left,
		const D3D11_DEPTH_STENCIL_DESC& Right);


	class CReconstructedPipelineStateGuard final
	{
	public:
		explicit CReconstructedPipelineStateGuard(ID3D11DeviceContext* pContext)
			: m_pContext(pContext)
		{
			ID3D11SamplerState* pSampler = nullptr;
			ID3D11BlendState* pBlend = nullptr;
			ID3D11RasterizerState* pRasterizer = nullptr;
			ID3D11DepthStencilState* pDepthStencil = nullptr;
			m_pContext->PSGetSamplers(0u, 1u, &pSampler);
			m_pContext->OMGetBlendState(
				&pBlend, m_BlendFactor.data(), &m_iSampleMask);
			m_pContext->RSGetState(&pRasterizer);
			m_pContext->OMGetDepthStencilState(
				&pDepthStencil, &m_iStencilReference);
			m_pSampler.Attach(pSampler);
			m_pBlendState.Attach(pBlend);
			m_pRasterizerState.Attach(pRasterizer);
			m_pDepthStencilState.Attach(pDepthStencil);
		}

		~CReconstructedPipelineStateGuard()
		{
			ID3D11SamplerState* pSampler = m_pSampler.Get();
			m_pContext->PSSetSamplers(0u, 1u, &pSampler);
			m_pContext->OMSetBlendState(m_pBlendState.Get(),
				m_BlendFactor.data(), m_iSampleMask);
			m_pContext->RSSetState(m_pRasterizerState.Get());
			m_pContext->OMSetDepthStencilState(
				m_pDepthStencilState.Get(), m_iStencilReference);
		}

	private:
		ID3D11DeviceContext* m_pContext = nullptr;
		ComPtr<ID3D11SamplerState> m_pSampler;
		ComPtr<ID3D11BlendState> m_pBlendState;
		ComPtr<ID3D11RasterizerState> m_pRasterizerState;
		ComPtr<ID3D11DepthStencilState> m_pDepthStencilState;
		std::array<float, 4u> m_BlendFactor{};
		uint32_t m_iSampleMask = 0xffffffffu;
		uint32_t m_iStencilReference = 0u;
	};

	uint64_t Build_ResourceSignature(
		const Client::EFFECT_DOCUMENT_DESC& Document);


	bool_t Needs_ParticleInstanceBuffer(
		const Client::EFFECT_DOCUMENT_DESC& Document);


	bool_t Try_ResolveTrailBufferPointCapacity(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		uint32_t& iOutPointCapacity,
		std::string& strOutError);


	HRESULT Create_SolidTexture(
		ID3D11Device* pDevice,
		const uint32_t iRGBA,
		ComPtr<ID3D11ShaderResourceView>& OutSRV);


	std::shared_ptr<EFFECT_RENDERER_CORE> Build_RendererCore(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);


	std::shared_ptr<EFFECT_RENDERER_CORE> Acquire_RendererCore(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);

}
using namespace EffectDocumentRendererDetail;


struct Client::CEffectDocumentRenderer::PRODUCT_TARGET_STAGE final
{
	ComPtr<ID3D11Device> pDevice;
	ComPtr<ID3D11DeviceContext> pContextIdentity;
	uint64_t iCatalogRevision = 0u;
	std::string strEffectAssetId;
	std::shared_ptr<const EFFECT_DOCUMENT_DESC> pDocument;
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pVisualProgramProjection;
	std::shared_ptr<const CEffectMaterialProgramRegistry>
		pMaterialProgramRegistry;
	std::shared_ptr<EFFECT_RENDERER_CORE> pRendererCoreIdentity;
	std::string strCommitSuccessStatus;
	PREPARED_KEY Key;
	std::shared_ptr<const PREPARED_DOCUMENT> pPrepared;
	std::map<PREPARED_KEY, std::shared_ptr<const PREPARED_DOCUMENT>>
		CandidateDocuments;
	std::unordered_map<const EFFECT_DOCUMENT_DESC*,
		std::shared_ptr<const PREPARED_DOCUMENT>> CandidateDocumentsByIdentity;
	std::shared_ptr<PRODUCT_PREWARM_SESSION> pCandidateSession;
	uint64_t iStagedFromGeneration = 0u;
	ID3D11Device* pStagedFromDevice = nullptr;
	uint64_t iStagedFromRevision = 0u;
	uint32_t iResolvedElementCount = 0u;
	bool_t bAlreadyPrepared = false;
	bool_t bConsumed = false;
};