#include "Effect_ValtanTranslatedCanaryRuntime.h"

#include "Effect_RuntimeAuthority.h"
#include "GameInstance.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"

#include "DirectXTK/DDSTextureLoader.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <vector>

namespace
{
	using FAMILY = Client::VALTAN_TRANSLATED_CANARY_FAMILY;

	constexpr size_t FAMILY_COUNT = static_cast<size_t>(FAMILY::END);
	constexpr uint32_t MAX_TEXTURE_COUNT = 9u;
	constexpr uint32_t MAX_SAMPLER_COUNT = 9u;

	constexpr size_t Family_Index(const FAMILY eFamily)
	{
		return static_cast<size_t>(eFamily);
	}

	struct FAMILY_GPU_RESOURCE final
	{
		shared_ptr<Engine::CShader> pShader;
		std::array<ComPtr<ID3D11ShaderResourceView>, MAX_TEXTURE_COUNT>
			Textures{};
		std::array<ComPtr<ID3D11SamplerState>, MAX_SAMPLER_COUNT> Samplers{};
		uint32_t iTextureMask = 0u;
		uint32_t iSamplerMask = 0u;
		uint32_t iSamplerCount = 0u;
	};

	struct TEXTURE_PIN final
	{
		FAMILY eFamily;
		uint32_t iTextureRegister;
		uint32_t iSamplerRegister;
		const char* pAssetId;
		uint64_t iByteCount;
		const char* pSha256;
		bool_t bSRGB;
		bool_t bClampU;
		bool_t bClampV;
	};

	/* Texture order, register wires, byte identities and colour spaces are
	   frozen by Valtan.front-back-front-texture-sampler-closure.receipt.v1.
	   Only explicitly serialized clamp axes are admitted.  Every other address
	   axis and the linear filter are bounded Tool choices (SAMPLER_EXACT=false). */
	constexpr std::array<TEXTURE_PIN, 19u> TEXTURE_PINS = {{
		{ FAMILY::MASKED_DISSOLVE, 1u, 0u,
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_decal_005.dds", 32896u,
			"aa9af6ba71f87936935c01e8fac06290b53bba9fee4d1d1f4df3132f5101311e",
			true, false, false },
		{ FAMILY::MASKED_DISSOLVE, 2u, 1u,
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_normal_102.dds", 262272u,
			"e966b832b441fdc439d9f586fc1c631ec2c99bc15f8a2bed0154a4c674f22537",
			false, false, false },
		{ FAMILY::MASKED_DISSOLVE, 3u, 2u,
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_fluid_020.dds", 32896u,
			"21bb4ed2b7d8758cd8c225aa1946b7f40e6c2e9d1a7d00a6ef0a43646f51dae3",
			true, false, false },
		{ FAMILY::MASKED_DISSOLVE, 4u, 3u,
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_stoneparts_002.dds", 131200u,
			"e9b7fae9a47868ad05be52000a6580f5d133e4e0d7e771fc0be30a17e22cf238",
			true, false, false },
		{ FAMILY::MASKED_DISSOLVE, 0u, 4u,
			"Effect/Valtan/Textures/FX_TEX_04/fx_h_noise_001.dds", 32896u,
			"c3d5c5db7995c07bb04bc940fc8aaad5bd82614ea10f92c15bb5748d2fbd5bd2",
			true, false, false },

		{ FAMILY::GROUND_DECAL, 0u, 0u,
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_normal_012_1.dds", 32896u,
			"27edebc94df0813690976c889cbe5bad5aecebab037266022d87f1ea3fe9d443",
			true, false, false },
		{ FAMILY::GROUND_DECAL, 4u, 1u,
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_fragment_016.dds", 32896u,
			"8f2bb7cc5da371d2e269c4ce9ea3ca3d74026cbf1441651d5670175d94d81222",
			true, false, false },
		{ FAMILY::GROUND_DECAL, 5u, 2u,
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_noise_030.dds", 32896u,
			"9a876ceb5d173869af5350d348462f54fc8a9b00134957bd3cd56805860d0581",
			true, false, false },
		{ FAMILY::GROUND_DECAL, 1u, 3u,
			"Effect/Valtan/Textures/FX_TEX_03/fx_e_noise_002.dds", 65664u,
			"9abbd5202b3a68654cd4b8a5b1a7afc31f596d1ef03431ab5333e4e2062a0d3a",
			true, false, false },
		{ FAMILY::GROUND_DECAL, 2u, 4u,
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_normal_012.dds", 65664u,
			"15593b1de7fc71c59eb71c93f7c799a120dcead45c7fb292674ebf1230b8149b",
			false, false, false },
		{ FAMILY::GROUND_DECAL, 3u, 5u,
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_decal_012.dds", 65664u,
			"a02f2184b25786a4e699dad6ff4405cce717defd458990fa832d2e94f34f91ac",
			true, false, false },

		{ FAMILY::CRACK_TRANSLUCENT, 7u, 1u,
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_normal_053.dds", 65664u,
			"55452d24b322f4271a0e896e9ce001bfb440170cf91d55937ad2331b4d1be06e",
			false, false, false },
		{ FAMILY::CRACK_TRANSLUCENT, 8u, 2u,
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_electric_013_ycl.dds", 65664u,
			"270acb6fadcdd03488e30825a086fc3e4b01d2b7027cd06ab79993a0085b76b1",
			true, false, true },
		{ FAMILY::CRACK_TRANSLUCENT, 0u, 3u,
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_097.dds", 65664u,
			"46ba6a315b34e3cfde1a273786c703d15fe769542eb9a2f7749971df3a111dc0",
			true, false, false },
		{ FAMILY::CRACK_TRANSLUCENT, 1u, 4u,
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_039_1.dds", 384u,
			"e2f7f0f1090cff2ced06f097d7965dd0632511662a7c786d82d20d4140fb7a80",
			false, false, false },
		{ FAMILY::CRACK_TRANSLUCENT, 2u, 5u,
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_091_2_cl.dds", 384u,
			"4992ae031ae7eccabcdd74e4ceb5059fff4bbc7f07086ef79c7b4d9a1f101cb2",
			false, true, true },
		{ FAMILY::CRACK_TRANSLUCENT, 3u, 6u,
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_environ_035.dds", 32896u,
			"67845b6501a645505f2ccb016d164a221d58f0f5bd3f23e58c83f7c4abaaacd8",
			true, false, false },
		{ FAMILY::CRACK_TRANSLUCENT, 4u, 7u,
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_noise_031.dds", 32896u,
			"d9e03979a39e8c24fb5645ea87e0957303785b1ba1ddf31b7c68bab3a5ad9c6c",
			true, false, false },
		{ FAMILY::CRACK_TRANSLUCENT, 6u, 8u,
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_decal_046_1.dds", 131200u,
			"8d6f55c37c9ec0ccf9937497e88ab21841f18f26e24947b6720bf52db555268e",
			true, false, false },
	}};

	struct TARGET_ELEMENT_PIN final
	{
		const char* pElementId;
		FAMILY eFamily;
	};

	constexpr std::array<TARGET_ELEMENT_PIN, 9u> TARGET_ELEMENT_PINS = {{
		{ "par_n_rpbf_atk_01_02.em07", FAMILY::MASKED_DISSOLVE },
		{ "par_n_rpbf_atk_01_02.em14", FAMILY::GROUND_DECAL },
		{ "par_n_rpbf_atk_04_11.em00", FAMILY::CRACK_TRANSLUCENT },
		{ "par_n_rpbf_atk_04_11.em01", FAMILY::CRACK_TRANSLUCENT },
		{ "par_n_rpbf_atk_04_12.em00", FAMILY::CRACK_TRANSLUCENT },
		{ "par_n_rpbf_atk_04_12.em01", FAMILY::CRACK_TRANSLUCENT },
		{ "par_n_rpbf_atk_04_12.em02", FAMILY::CRACK_TRANSLUCENT },
		{ "par_n_rpbf_atk_04_13.em00", FAMILY::CRACK_TRANSLUCENT },
		{ "par_n_rpbf_atk_04_13.em01", FAMILY::CRACK_TRANSLUCENT },
	}};

	constexpr std::string_view MASKED_SOURCE_MATERIAL =
		"fx_m_mi_n_00.fx_n_me_dissolve_04_011_ma";
	constexpr std::string_view GROUND_SOURCE_MATERIAL =
		"fx_m_mi_n_00.fx_mi.fx_n_de_ground_04_30_tr";
	constexpr std::string_view CRACK_SOURCE_MATERIAL =
		"fx_m_mi_o_00.fx_mi.fx_o_me_crack_01_01_tr";
	constexpr std::string_view MASKED_MODEL =
		"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel";
	constexpr std::string_view CRACK_MODEL =
		"Effect/Valtan/Meshes/FX_SM_00/fm_d_crackline_005.wmodel";

	const TARGET_ELEMENT_PIN* Find_Target_Pin(const std::string_view strElementId)
	{
		const auto It = std::find_if(TARGET_ELEMENT_PINS.begin(),
			TARGET_ELEMENT_PINS.end(), [strElementId](const auto& Pin)
			{
				return strElementId == Pin.pElementId;
			});
		return It == TARGET_ELEMENT_PINS.end() ? nullptr : &*It;
	}

	bool_t Read_Exact_Runtime_Asset(
		const TEXTURE_PIN& Pin,
		std::vector<uint8_t>& OutBytes,
		std::string& strOutError)
	{
		const std::filesystem::path Path = Client::CRuntimeAssetRoot::Resolve(
			std::filesystem::path(Pin.pAssetId));
		if (Path.empty() || !std::filesystem::is_regular_file(Path))
		{
			strOutError = "Valtan translated canary DDS is missing: " +
				std::string(Pin.pAssetId);
			return false;
		}
		std::ifstream Input(Path, std::ios::binary | std::ios::ate);
		if (!Input)
		{
			strOutError = "Valtan translated canary DDS could not be opened: " +
				std::string(Pin.pAssetId);
			return false;
		}
		const std::streamoff Size = Input.tellg();
		if (Size < 0 || static_cast<uint64_t>(Size) != Pin.iByteCount)
		{
			strOutError = "Valtan translated canary DDS byte count changed: " +
				std::string(Pin.pAssetId);
			return false;
		}
		std::vector<uint8_t> Bytes(static_cast<size_t>(Size));
		Input.seekg(0, std::ios::beg);
		if (!Bytes.empty() &&
			!Input.read(reinterpret_cast<char*>(Bytes.data()), Size))
		{
			strOutError = "Valtan translated canary DDS read failed: " +
				std::string(Pin.pAssetId);
			return false;
		}
		const std::string_view ByteView(
			reinterpret_cast<const char*>(Bytes.data()), Bytes.size());
		if (Client::CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(ByteView) !=
			Pin.pSha256)
		{
			strOutError = "Valtan translated canary DDS SHA-256 changed: " +
				std::string(Pin.pAssetId);
			return false;
		}
		OutBytes = std::move(Bytes);
		return true;
	}

	D3D11_SAMPLER_DESC Make_Bounded_Linear_Sampler(
		const bool_t bClampU, const bool_t bClampV)
	{
		D3D11_SAMPLER_DESC Desc{};
		Desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		Desc.AddressU = bClampU ? D3D11_TEXTURE_ADDRESS_CLAMP :
			D3D11_TEXTURE_ADDRESS_WRAP;
		Desc.AddressV = bClampV ? D3D11_TEXTURE_ADDRESS_CLAMP :
			D3D11_TEXTURE_ADDRESS_WRAP;
		Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		Desc.MaxAnisotropy = 1u;
		Desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		Desc.MinLOD = 0.f;
		Desc.MaxLOD = D3D11_FLOAT32_MAX;
		return Desc;
	}

	D3D11_SAMPLER_DESC Make_Scene_Depth_Sampler()
	{
		D3D11_SAMPLER_DESC Desc{};
		Desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		Desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		Desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		Desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		Desc.MaxAnisotropy = 1u;
		Desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		Desc.MinLOD = 0.f;
		Desc.MaxLOD = D3D11_FLOAT32_MAX;
		return Desc;
	}

	bool_t Is_Finite(const float2_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y);
	}

	bool_t Is_Finite(const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	}

	bool_t Is_Finite(const float4x4_t& Value)
	{
		const f32_t* const pValue = &Value._11;
		return std::all_of(pValue, pValue + 16u,
			[](const f32_t Scalar) { return std::isfinite(Scalar); });
	}

	f32_t Signed_Fractional(const f32_t Value)
	{
		return Value - std::trunc(Value);
	}

	bool_t Build_Masked_CB0(
		const f32_t fLocalTimeSeconds,
		std::array<float4_t, 23u>& OutRows)
	{
		if (!std::isfinite(fLocalTimeSeconds) || fLocalTimeSeconds < 0.f)
			return false;
		OutRows = {{
			{ 1.f, 1.f, 1.f, 1.f },
			{ 0.f, 0.f, 0.f, 1.f }, { 1.f, 1.f, 1.f, 1.f },
			{ 1.f, 0.f, 0.f, 0.f }, { 0.f, 1.f, 1.f, 1.f },
			{ 0.f, 0.f, 0.f, 0.f }, { .5f, .5f, .5f, .5f },
			{ 2.f, 15.f, 15.f, 1.5f }, { 1.f, 1.f, 1.f, 1.f },
			{ 1.f, 1.f, 1.f, 1.f }, { 1.f, 1.f, 1.f, 5.f },
			{ 1.f, 1.f, 1.f, 1.f }, { 1.2f, .9f, .8f, 1.f },
			{ .15f, .2f, .25f, 1.f }, { 1.f, 0.f, 0.f, 0.f },
			{ 0.f, 1.f, 1.f, 1.f }, { .5f, .5f, 2.f, 1.f },
			{ 1.f, 0.f, 2.f, 1.f }, { 1.f, .6f, 1.2f, 2.f },
			{ 1.5f, .1f, 0.f, 2.f }, { 2.f, -.15f, 1.f, 0.f },
			{ 1.f, .5f, .5f, 2.f }, { .175f, 0.f, 0.f, 0.f },
		}};
		/* The sole live material-uniform lane in this target is the source Time
		   expression packed into cb0[19].z. */
		OutRows[19].z = fLocalTimeSeconds;
		return true;
	}

	bool_t Build_Crack_CB0(
		const float4_t& DynamicParameter,
		const f32_t fLocalTimeSeconds,
		std::array<float4_t, 17u>& OutRows)
	{
		if (!Is_Finite(DynamicParameter) || !std::isfinite(fLocalTimeSeconds) ||
			fLocalTimeSeconds < 0.f)
		{
			return false;
		}
		const f32_t Panner = .05f * fLocalTimeSeconds;
		OutRows = {{
			{ 0.f, 0.f, 0.f, 1.f }, { 1.f, 1.f, 1.f, 1.f },
			{ 0.f, 0.f, 0.f, 1.f }, DynamicParameter,
			{ 0.f, Signed_Fractional(Panner), 0.f, 0.f },
			{ Signed_Fractional(Panner), 0.f, 0.f, 0.f },
			{ 1.f, 10.f, 6.f, 1.5f }, { 1.f, 1.f, 1.f, .25f },
			{ 6.f, 1.33f, .05f, fLocalTimeSeconds },
			{ Panner, 0.f, Panner, 12.f },
			{ 1.5f, Signed_Fractional(Panner), 0.f, .05f },
			{ 6.f, 4.f, 1.f, .7f }, { .007f, 50.f, 1.5f, 50.f },
			{ 9.f, 1.f, 1.25f, 0.f },
			{ 0.f, 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 0.f },
			{ 0.f, 0.f, 0.f, 0.f },
		}};
		return true;
	}

	bool_t Build_Ground_CB0(
		const f32_t fLocalTimeSeconds,
		std::array<float4_t, 20u>& OutRows)
	{
		if (!std::isfinite(fLocalTimeSeconds) || fLocalTimeSeconds < 0.f)
			return false;
		const f32_t Panner = .1f * fLocalTimeSeconds;
		OutRows = {{
			{ 0.f, 1.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 1.f },
			{ 1.f, 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 1.f },
			{ 1.f, 1.f, 1.f, 1.f },
			{ Signed_Fractional(Panner), 0.f, 0.f, 0.f },
			{ 2.f, 20.f, 12.f, .8f }, { 1.f, 1.f, 1.f, .4f },
			{ 0.f, 2.f, 1.f, 1.f }, { 2.f, 5.f, .1f, fLocalTimeSeconds },
			{ Panner, Panner, 0.f, 5.f },
			{ 0.f, Signed_Fractional(Panner), 5.f, 1.f },
			{ 10.f, .8f, 2.f, 2.f }, { 0.f, 0.f, 0.f, 0.f },
			{ .5f, 0.f, 0.f, 0.f },
			{ 0.f, 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 0.f },
			{ 0.f, 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 0.f },
			{ 0.f, 0.f, 0.f, 0.f },
		}};
		return true;
	}

	class CValtanCanaryPipelineStateGuard final
	{
	public:
		explicit CValtanCanaryPipelineStateGuard(ID3D11DeviceContext* pContext)
			: m_pContext(pContext)
		{
			if (nullptr == m_pContext)
				return;

			ID3D11InputLayout* pInputLayout = nullptr;
			m_pContext->IAGetInputLayout(&pInputLayout);
			m_pInputLayout.Attach(pInputLayout);
			std::array<ID3D11Buffer*,
				D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> VertexBuffers{};
			m_pContext->IAGetVertexBuffers(0u,
				D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT,
				VertexBuffers.data(), m_VertexBufferStrides.data(),
				m_VertexBufferOffsets.data());
			for (size_t i = 0u; i < VertexBuffers.size(); ++i)
				m_VertexBuffers[i].Attach(VertexBuffers[i]);
			ID3D11Buffer* pIndexBuffer = nullptr;
			m_pContext->IAGetIndexBuffer(
				&pIndexBuffer, &m_IndexFormat, &m_iIndexOffset);
			m_pIndexBuffer.Attach(pIndexBuffer);
			m_pContext->IAGetPrimitiveTopology(&m_PrimitiveTopology);

			ID3D11VertexShader* pVertexShader = nullptr;
			ID3D11GeometryShader* pGeometryShader = nullptr;
			ID3D11PixelShader* pPixelShader = nullptr;
			m_pContext->VSGetShader(&pVertexShader, nullptr, nullptr);
			m_pContext->GSGetShader(&pGeometryShader, nullptr, nullptr);
			m_pContext->PSGetShader(&pPixelShader, nullptr, nullptr);
			m_pVertexShader.Attach(pVertexShader);
			m_pGeometryShader.Attach(pGeometryShader);
			m_pPixelShader.Attach(pPixelShader);

			Capture_ConstantBuffers();
			Capture_PixelResources();
			Capture_OutputMerger();
			Capture_Rasterizer();
			m_bCaptured = true;
		}

		~CValtanCanaryPipelineStateGuard()
		{
			Restore();
		}

		CValtanCanaryPipelineStateGuard(
			const CValtanCanaryPipelineStateGuard&) = delete;
		CValtanCanaryPipelineStateGuard& operator=(
			const CValtanCanaryPipelineStateGuard&) = delete;

	private:
		void Capture_ConstantBuffers()
		{
			std::array<ID3D11Buffer*,
				D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT> VSRaw{};
			std::array<ID3D11Buffer*,
				D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT> PSRaw{};
			m_pContext->VSGetConstantBuffers(0u,
				D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, VSRaw.data());
			m_pContext->PSGetConstantBuffers(0u,
				D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, PSRaw.data());
			for (size_t i = 0u; i < VSRaw.size(); ++i)
			{
				m_VSConstantBuffers[i].Attach(VSRaw[i]);
				m_PSConstantBuffers[i].Attach(PSRaw[i]);
			}
		}

		void Capture_PixelResources()
		{
			std::array<ID3D11ShaderResourceView*,
				D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> ResourceRaw{};
			std::array<ID3D11SamplerState*,
				D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT> SamplerRaw{};
			m_pContext->PSGetShaderResources(0u,
				D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT,
				ResourceRaw.data());
			m_pContext->PSGetSamplers(0u,
				D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, SamplerRaw.data());
			for (size_t i = 0u; i < ResourceRaw.size(); ++i)
				m_PSShaderResources[i].Attach(ResourceRaw[i]);
			for (size_t i = 0u; i < SamplerRaw.size(); ++i)
				m_PSSamplers[i].Attach(SamplerRaw[i]);
		}

		void Capture_OutputMerger()
		{
			std::array<ID3D11RenderTargetView*,
				D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> RenderTargetRaw{};
			ID3D11DepthStencilView* pDepthStencilView = nullptr;
			m_pContext->OMGetRenderTargets(
				D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
				RenderTargetRaw.data(), &pDepthStencilView);
			for (size_t i = 0u; i < RenderTargetRaw.size(); ++i)
				m_RenderTargets[i].Attach(RenderTargetRaw[i]);
			m_pDepthStencilView.Attach(pDepthStencilView);

			ID3D11BlendState* pBlendState = nullptr;
			ID3D11DepthStencilState* pDepthStencilState = nullptr;
			m_pContext->OMGetBlendState(
				&pBlendState, m_BlendFactor.data(), &m_iSampleMask);
			m_pContext->OMGetDepthStencilState(
				&pDepthStencilState, &m_iStencilReference);
			m_pBlendState.Attach(pBlendState);
			m_pDepthStencilState.Attach(pDepthStencilState);
		}

		void Capture_Rasterizer()
		{
			ID3D11RasterizerState* pRasterizerState = nullptr;
			m_pContext->RSGetState(&pRasterizerState);
			m_pRasterizerState.Attach(pRasterizerState);
			m_iViewportCount =
				D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
			m_iScissorCount =
				D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
			m_pContext->RSGetViewports(&m_iViewportCount, m_Viewports.data());
			m_pContext->RSGetScissorRects(&m_iScissorCount, m_Scissors.data());
		}

		void Restore()
		{
			if (!m_bCaptured || nullptr == m_pContext)
				return;

			/* Break SRV/RT hazards before restoring the complete captured graph. */
			std::array<ID3D11ShaderResourceView*,
				D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> NullResources{};
			std::array<ID3D11RenderTargetView*,
				D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> NullTargets{};
			m_pContext->PSSetShaderResources(0u,
				D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT,
				NullResources.data());
			m_pContext->OMSetRenderTargets(
				D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
				NullTargets.data(), nullptr);

			m_pContext->IASetInputLayout(m_pInputLayout.Get());
			std::array<ID3D11Buffer*,
				D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> VertexBufferRaw{};
			for (size_t i = 0u; i < VertexBufferRaw.size(); ++i)
				VertexBufferRaw[i] = m_VertexBuffers[i].Get();
			m_pContext->IASetVertexBuffers(0u,
				D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT,
				VertexBufferRaw.data(), m_VertexBufferStrides.data(),
				m_VertexBufferOffsets.data());
			m_pContext->IASetIndexBuffer(
				m_pIndexBuffer.Get(), m_IndexFormat, m_iIndexOffset);
			m_pContext->IASetPrimitiveTopology(m_PrimitiveTopology);

			m_pContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0u);
			m_pContext->GSSetShader(m_pGeometryShader.Get(), nullptr, 0u);
			m_pContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0u);
			std::array<ID3D11Buffer*,
				D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT> VSRaw{};
			std::array<ID3D11Buffer*,
				D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT> PSRaw{};
			for (size_t i = 0u; i < VSRaw.size(); ++i)
			{
				VSRaw[i] = m_VSConstantBuffers[i].Get();
				PSRaw[i] = m_PSConstantBuffers[i].Get();
			}
			m_pContext->VSSetConstantBuffers(0u,
				D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, VSRaw.data());
			m_pContext->PSSetConstantBuffers(0u,
				D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, PSRaw.data());

			std::array<ID3D11ShaderResourceView*,
				D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> ResourceRaw{};
			std::array<ID3D11SamplerState*,
				D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT> SamplerRaw{};
			for (size_t i = 0u; i < ResourceRaw.size(); ++i)
				ResourceRaw[i] = m_PSShaderResources[i].Get();
			for (size_t i = 0u; i < SamplerRaw.size(); ++i)
				SamplerRaw[i] = m_PSSamplers[i].Get();
			m_pContext->PSSetShaderResources(0u,
				D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT,
				ResourceRaw.data());
			m_pContext->PSSetSamplers(0u,
				D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, SamplerRaw.data());

			std::array<ID3D11RenderTargetView*,
				D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> RenderTargetRaw{};
			for (size_t i = 0u; i < RenderTargetRaw.size(); ++i)
				RenderTargetRaw[i] = m_RenderTargets[i].Get();
			m_pContext->OMSetRenderTargets(
				D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
				RenderTargetRaw.data(), m_pDepthStencilView.Get());
			m_pContext->OMSetBlendState(
				m_pBlendState.Get(), m_BlendFactor.data(), m_iSampleMask);
			m_pContext->OMSetDepthStencilState(
				m_pDepthStencilState.Get(), m_iStencilReference);
			m_pContext->RSSetState(m_pRasterizerState.Get());
			m_pContext->RSSetViewports(m_iViewportCount, m_Viewports.data());
			m_pContext->RSSetScissorRects(m_iScissorCount, m_Scissors.data());
			m_bCaptured = false;
		}

		ID3D11DeviceContext* m_pContext = nullptr;
		bool_t m_bCaptured = false;
		ComPtr<ID3D11InputLayout> m_pInputLayout;
		std::array<ComPtr<ID3D11Buffer>,
			D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> m_VertexBuffers{};
		std::array<uint32_t, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT>
			m_VertexBufferStrides{};
		std::array<uint32_t, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT>
			m_VertexBufferOffsets{};
		ComPtr<ID3D11Buffer> m_pIndexBuffer;
		DXGI_FORMAT m_IndexFormat = DXGI_FORMAT_UNKNOWN;
		uint32_t m_iIndexOffset = 0u;
		D3D11_PRIMITIVE_TOPOLOGY m_PrimitiveTopology =
			D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		ComPtr<ID3D11VertexShader> m_pVertexShader;
		ComPtr<ID3D11GeometryShader> m_pGeometryShader;
		ComPtr<ID3D11PixelShader> m_pPixelShader;
		std::array<ComPtr<ID3D11Buffer>,
			D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT>
			m_VSConstantBuffers{};
		std::array<ComPtr<ID3D11Buffer>,
			D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT>
			m_PSConstantBuffers{};
		std::array<ComPtr<ID3D11ShaderResourceView>,
			D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT>
			m_PSShaderResources{};
		std::array<ComPtr<ID3D11SamplerState>,
			D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT> m_PSSamplers{};
		std::array<ComPtr<ID3D11RenderTargetView>,
			D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> m_RenderTargets{};
		ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;
		ComPtr<ID3D11BlendState> m_pBlendState;
		ComPtr<ID3D11DepthStencilState> m_pDepthStencilState;
		ComPtr<ID3D11RasterizerState> m_pRasterizerState;
		std::array<f32_t, 4u> m_BlendFactor{};
		uint32_t m_iSampleMask = 0xffffffffu;
		uint32_t m_iStencilReference = 0u;
		std::array<D3D11_VIEWPORT,
			D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE>
			m_Viewports{};
		std::array<D3D11_RECT,
			D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE>
			m_Scissors{};
		uint32_t m_iViewportCount = 0u;
		uint32_t m_iScissorCount = 0u;
	};

	HRESULT Bind_Texture_Set(
		const shared_ptr<Engine::CShader>& pShader,
		const FAMILY_GPU_RESOURCE& Resource,
		const char* pVariablePrefix)
	{
		if (nullptr == pShader || nullptr == pVariablePrefix)
			return E_INVALIDARG;
		for (uint32_t iRegister = 0u; iRegister < MAX_TEXTURE_COUNT;
			++iRegister)
		{
			if (0u == (Resource.iTextureMask & (1u << iRegister)))
				continue;
			if (nullptr == Resource.Textures[iRegister])
				return E_POINTER;
			const std::string Variable = std::string(pVariablePrefix) +
				std::to_string(iRegister);
			const HRESULT Result = pShader->Bind_Texture(
				Variable.c_str(), Resource.Textures[iRegister]);
			if (FAILED(Result))
				return Result;
		}
		return S_OK;
	}

	HRESULT Apply_Samplers(
		ID3D11DeviceContext* pContext,
		const FAMILY_GPU_RESOURCE& Resource)
	{
		if (nullptr == pContext || 0u == Resource.iSamplerCount ||
			Resource.iSamplerCount > MAX_SAMPLER_COUNT)
		{
			return E_INVALIDARG;
		}
		std::array<ID3D11SamplerState*, MAX_SAMPLER_COUNT> Samplers{};
		for (uint32_t i = 0u; i < Resource.iSamplerCount; ++i)
		{
			if (0u == (Resource.iSamplerMask & (1u << i)) ||
				nullptr == Resource.Samplers[i])
			{
				return E_POINTER;
			}
			Samplers[i] = Resource.Samplers[i].Get();
		}
		pContext->PSSetSamplers(0u, Resource.iSamplerCount, Samplers.data());
		return S_OK;
	}

	const std::string* Find_Mesh_Asset(const Client::EFFECT_ELEMENT_DESC& Element)
	{
		const std::string* pFound = nullptr;
		for (const Client::EFFECT_RESOURCE_BINDING_DESC& Binding :
			Element.ResourceBindings)
		{
			if (Binding.strSlotId != "meshModel")
				continue;
			if (nullptr != pFound)
				return nullptr;
			pFound = &Binding.strAssetId;
		}
		return pFound;
	}

	size_t Count_Mesh_Bindings(const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return static_cast<size_t>(std::count_if(
			Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
			[](const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == "meshModel";
			}));
	}

}

struct Client::VALTAN_TRANSLATED_CANARY_ELEMENT_PACKET final
{
	VALTAN_TRANSLATED_CANARY_ELEMENT_PACKET(
		const FAMILY eInFamily,
		const uint64_t iInArmGeneration,
		std::string strInElementId)
		: eFamily(eInFamily),
		iArmGeneration(iInArmGeneration),
		strElementId(std::move(strInElementId))
	{
	}

	const FAMILY eFamily = FAMILY::END;
	const uint64_t iArmGeneration = 0u;
	const std::string strElementId;
};

struct Client::CValtanTranslatedCanaryRuntime::RUNTIME_STATE final
{
	ComPtr<ID3D11Device> pDevice;
	ComPtr<ID3D11DeviceContext> pContext;
	std::array<FAMILY_GPU_RESOURCE, FAMILY_COUNT> Families{};
	uint64_t iArmGeneration = 0u;
	bool_t bArmed = false;
};

Client::CValtanTranslatedCanaryRuntime::
	CValtanTranslatedCanaryRuntime(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext)
	: m_pState(std::make_unique<RUNTIME_STATE>())
{
	m_pState->pDevice = std::move(pDevice);
	m_pState->pContext = std::move(pContext);
}

Client::CValtanTranslatedCanaryRuntime::
	~CValtanTranslatedCanaryRuntime() = default;

bool_t Client::CValtanTranslatedCanaryRuntime::Arm(
	std::string& strOutError)
{
	strOutError.clear();
	if (nullptr == m_pState || nullptr == m_pState->pDevice ||
		nullptr == m_pState->pContext)
	{
		strOutError = "Valtan translated canary device/context is unavailable.";
		return false;
	}

	std::array<FAMILY_GPU_RESOURCE, FAMILY_COUNT> Staged{};
	Staged[Family_Index(FAMILY::MASKED_DISSOLVE)].pShader =
		Engine::CShader::Create(m_pState->pDevice, m_pState->pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxEffectUe3ValtanDissolve01.hlsl"),
			VTXMESH::Elements, VTXMESH::iNumElements);
	Staged[Family_Index(FAMILY::CRACK_TRANSLUCENT)].pShader =
		Engine::CShader::Create(m_pState->pDevice, m_pState->pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxEffectUe3ValtanCrack01.hlsl"),
			VTXMESH::Elements, VTXMESH::iNumElements);
	Staged[Family_Index(FAMILY::GROUND_DECAL)].pShader =
		Engine::CShader::Create(m_pState->pDevice, m_pState->pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxEffectUe3ValtanGround04.hlsl"),
			VTXTEX::Elements, VTXTEX::iNumElements);
	if (std::any_of(Staged.begin(), Staged.end(),
		[](const auto& Resource) { return nullptr == Resource.pShader; }))
	{
		strOutError =
			"Valtan translated canary failed to compile all three Tool shaders.";
		return false;
	}

	for (const TEXTURE_PIN& Pin : TEXTURE_PINS)
	{
		FAMILY_GPU_RESOURCE& Resource = Staged[Family_Index(Pin.eFamily)];
		if (Pin.iTextureRegister >= Resource.Textures.size() ||
			Pin.iSamplerRegister >= Resource.Samplers.size() ||
			nullptr != Resource.Textures[Pin.iTextureRegister] ||
			nullptr != Resource.Samplers[Pin.iSamplerRegister])
		{
			strOutError = "Valtan translated canary texture/sampler pin overlaps.";
			return false;
		}

		std::vector<uint8_t> Bytes;
		if (!Read_Exact_Runtime_Asset(Pin, Bytes, strOutError))
			return false;
		const DirectX::DDS_LOADER_FLAGS Flags = Pin.bSRGB ?
			DirectX::DDS_LOADER_FORCE_SRGB : DirectX::DDS_LOADER_IGNORE_SRGB;
		if (FAILED(DirectX::CreateDDSTextureFromMemoryEx(
			m_pState->pDevice.Get(), Bytes.data(), Bytes.size(), 0u,
			D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0u, 0u, Flags,
			nullptr, &Resource.Textures[Pin.iTextureRegister])) ||
			nullptr == Resource.Textures[Pin.iTextureRegister])
		{
			strOutError = "Valtan translated canary typed DDS upload failed: " +
				std::string(Pin.pAssetId);
			return false;
		}

		const D3D11_SAMPLER_DESC SamplerDesc = Make_Bounded_Linear_Sampler(
			Pin.bClampU, Pin.bClampV);
		if (FAILED(m_pState->pDevice->CreateSamplerState(
			&SamplerDesc, &Resource.Samplers[Pin.iSamplerRegister])) ||
			nullptr == Resource.Samplers[Pin.iSamplerRegister])
		{
			strOutError = "Valtan translated canary bounded sampler creation failed.";
			return false;
		}
		Resource.iTextureMask |= 1u << Pin.iTextureRegister;
		Resource.iSamplerMask |= 1u << Pin.iSamplerRegister;
	}

	/* Crack t5/s0 is engine-owned Target_Depth rather than a material DDS. */
	FAMILY_GPU_RESOURCE& Crack =
		Staged[Family_Index(FAMILY::CRACK_TRANSLUCENT)];
	const D3D11_SAMPLER_DESC SceneSampler = Make_Scene_Depth_Sampler();
	if (FAILED(m_pState->pDevice->CreateSamplerState(
		&SceneSampler, &Crack.Samplers[0u])) || nullptr == Crack.Samplers[0u])
	{
		strOutError = "Valtan Crack translated scene-depth sampler creation failed.";
		return false;
	}
	Crack.iSamplerMask |= 1u;
	Staged[Family_Index(FAMILY::MASKED_DISSOLVE)].iSamplerCount = 5u;
	Staged[Family_Index(FAMILY::GROUND_DECAL)].iSamplerCount = 6u;
	Crack.iSamplerCount = 9u;

	for (const FAMILY_GPU_RESOURCE& Resource : Staged)
	{
		const uint32_t ExpectedSamplerMask =
			(1u << Resource.iSamplerCount) - 1u;
		if (0u == Resource.iTextureMask ||
			Resource.iSamplerMask != ExpectedSamplerMask)
		{
			strOutError = "Valtan translated canary staged an incomplete family ABI.";
			return false;
		}
	}

	m_pState->Families = std::move(Staged);
	++m_pState->iArmGeneration;
	if (0u == m_pState->iArmGeneration)
		m_pState->iArmGeneration = 1u;
	m_pState->bArmed = true;
	return true;
}

void Client::CValtanTranslatedCanaryRuntime::Clear()
{
	if (nullptr == m_pState)
		return;
	m_pState->Families = {};
	++m_pState->iArmGeneration;
	if (0u == m_pState->iArmGeneration)
		m_pState->iArmGeneration = 1u;
	m_pState->bArmed = false;
}

bool_t Client::CValtanTranslatedCanaryRuntime::Is_Armed() const
{
	return nullptr != m_pState && m_pState->bArmed;
}

bool_t Client::CValtanTranslatedCanaryRuntime::Stage_Packet(
	const std::string& strEffectAssetId,
	const EFFECT_ELEMENT_DESC& Element,
	std::shared_ptr<const VALTAN_TRANSLATED_CANARY_ELEMENT_PACKET>& pOutPacket,
	std::string& strOutError) const
{
	pOutPacket.reset();
	strOutError.clear();
	if (!Is_Armed())
		return true;
	if (strEffectAssetId != EFFECT_ASSET_ID)
	{
		strOutError =
			"Valtan translated canary rejected a non-target Effect ID.";
		return false;
	}
	const TARGET_ELEMENT_PIN* const pPin = Find_Target_Pin(Element.strElementId);
	if (nullptr == pPin)
		return true;

	const std::string* const pMeshAsset = Find_Mesh_Asset(Element);
	const size_t iMeshBindingCount = Count_Mesh_Bindings(Element);
	const bool_t bPreScaleExact = std::isfinite(Element.Detail.Mesh.fModelPreScale) &&
		std::abs(Element.Detail.Mesh.fModelPreScale - .01f) <= 1.e-7f;
	bool_t bIdentityValid = false;
	switch (pPin->eFamily)
	{
	case FAMILY::MASKED_DISSOLVE:
		bIdentityValid = Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			Element.Renderer.eType == EFFECT_RENDERER_TYPE::MESH_PARTICLE &&
			Element.SourceRecipe.strRendererShape == "mesh" && bPreScaleExact &&
			Element.Material.strSourceMaterialPath == MASKED_SOURCE_MATERIAL &&
			1u == iMeshBindingCount && nullptr != pMeshAsset &&
			*pMeshAsset == MASKED_MODEL;
		break;
	case FAMILY::CRACK_TRANSLUCENT:
		bIdentityValid = Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			Element.Renderer.eType == EFFECT_RENDERER_TYPE::MESH_PARTICLE &&
			Element.SourceRecipe.strRendererShape == "mesh" && bPreScaleExact &&
			Element.Material.strSourceMaterialPath == CRACK_SOURCE_MATERIAL &&
			1u == iMeshBindingCount && nullptr != pMeshAsset &&
			*pMeshAsset == CRACK_MODEL;
		break;
	case FAMILY::GROUND_DECAL:
		bIdentityValid = Element.eKind == EFFECT_ELEMENT_KIND::DECAL &&
			Element.Renderer.eType == EFFECT_RENDERER_TYPE::DECAL_PARTICLE &&
			Element.SourceRecipe.strRendererShape == "decal" && bPreScaleExact &&
			Element.Material.strSourceMaterialPath == GROUND_SOURCE_MATERIAL &&
			0u == iMeshBindingCount && nullptr == pMeshAsset;
		break;
	default:
		break;
	}
	if (!bIdentityValid)
	{
		strOutError =
			"Valtan translated canary occurrence/material/carrier identity changed: " +
			Element.strElementId;
		return false;
	}

	pOutPacket = std::shared_ptr<const VALTAN_TRANSLATED_CANARY_ELEMENT_PACKET>(
		new VALTAN_TRANSLATED_CANARY_ELEMENT_PACKET(
			pPin->eFamily, m_pState->iArmGeneration, Element.strElementId));
	return true;
}

HRESULT Client::CValtanTranslatedCanaryRuntime::Draw_Mesh(
	const EFFECT_ELEMENT_DESC& Element,
	const std::shared_ptr<Engine::CModel>& pResourceModel,
	const std::shared_ptr<const VALTAN_TRANSLATED_CANARY_ELEMENT_PACKET>&
		pPacket,
	const float4x4_t& World,
	const float4x4_t& NormalMatrix,
	const float4_t& DynamicParameter,
	const f32_t fLocalTimeSeconds) const
{
	if (nullptr == pPacket)
		return E_POINTER;
	if (pPacket->eFamily == FAMILY::GROUND_DECAL)
		return S_FALSE;
	const TARGET_ELEMENT_PIN* const pTarget = Find_Target_Pin(Element.strElementId);
	if (nullptr == pTarget || pTarget->eFamily != pPacket->eFamily)
		return S_FALSE;
	if (!Is_Armed() || pPacket->iArmGeneration != m_pState->iArmGeneration ||
		pPacket->strElementId != Element.strElementId ||
		nullptr == pResourceModel || 0u == pResourceModel->Get_NumMeshes() ||
		!Is_Finite(World) || !Is_Finite(NormalMatrix) ||
		!Is_Finite(DynamicParameter))
	{
		return E_INVALIDARG;
	}

	const float4x4_t* const pView =
		CGameInstance::Get().Get_Transform(D3DTS::VIEW);
	const float4x4_t* const pProjection =
		CGameInstance::Get().Get_Transform(D3DTS::PROJ);
	if (nullptr == pView || nullptr == pProjection || !Is_Finite(*pView) ||
		!Is_Finite(*pProjection))
	{
		return E_POINTER;
	}

	const FAMILY_GPU_RESOURCE& Resource =
		m_pState->Families[Family_Index(pPacket->eFamily)];
	if (nullptr == Resource.pShader)
		return E_FAIL;
	shared_ptr<Engine::CShader> pShader = Resource.pShader;
	CValtanCanaryPipelineStateGuard StateGuard(m_pState->pContext.Get());

	HRESULT Result = pShader->Bind_Matrix("g_WorldMatrix", &World);
	if (SUCCEEDED(Result))
		Result = pShader->Bind_Matrix("g_NormalMatrix", &NormalMatrix);
	if (SUCCEEDED(Result))
		Result = pShader->Bind_Matrix("g_ViewMatrix", pView);
	if (SUCCEEDED(Result))
		Result = pShader->Bind_Matrix("g_ProjMatrix", pProjection);
	if (FAILED(Result))
		return Result;

	if (pPacket->eFamily == FAMILY::MASKED_DISSOLVE)
	{
		std::array<float4_t, 23u> CB0{};
		if (!Build_Masked_CB0(fLocalTimeSeconds, CB0))
			return E_INVALIDARG;
		Result = pShader->Bind_RawValue(
			"g_Ue3ValtanDissolve01CB0", CB0.data(), sizeof(CB0));
		if (SUCCEEDED(Result))
			Result = pShader->Bind_RawValue(
				"g_Ue3ValtanDissolve01LocalTimeSeconds", &fLocalTimeSeconds,
				sizeof(fLocalTimeSeconds));
		if (SUCCEEDED(Result))
			Result = Bind_Texture_Set(
				pShader, Resource, "g_Ue3ValtanDissolve01Texture");
		if (SUCCEEDED(Result))
		{
			const uint32_t iAdmission = 1u;
			Result = pShader->Bind_RawValue(
				"g_Ue3ValtanDissolve01ToolAdmission", &iAdmission,
				sizeof(iAdmission));
		}
	}
	else
	{
		std::array<float4_t, 17u> CB0{};
		if (!Build_Crack_CB0(DynamicParameter, fLocalTimeSeconds, CB0) ||
			std::abs(pProjection->_43) <= 1.e-8f)
		{
			return E_INVALIDARG;
		}
		DirectX::XMVECTOR Determinant{};
		const DirectX::XMMATRIX ProjectionInverse = DirectX::XMMatrixInverse(
			&Determinant, DirectX::XMLoadFloat4x4(pProjection));
		if (std::abs(DirectX::XMVectorGetX(Determinant)) <= 1.e-8f)
			return E_INVALIDARG;
		float4x4_t CB1{};
		DirectX::XMStoreFloat4x4(&CB1, ProjectionInverse);
		std::array<float4_t, 4u> CB2{};
		CB2[0] = { .5f, -.5f, .5f, .5f };
		CB2[1] = { 0.f, 0.f, 1.f / pProjection->_43,
			pProjection->_33 / pProjection->_43 };
		/* UE3's engine-owned scene-light lane was not recovered from the
		   material package.  A neutral scale/no-offset value keeps the Tool RT0
		   adapter deterministic without claiming source scene-CB parity. */
		CB2[3] = { 0.f, 0.f, 0.f, 1.f };

		Result = pShader->Bind_RawValue(
			"g_Ue3ValtanCrack01CB0", CB0.data(), sizeof(CB0));
		if (SUCCEEDED(Result))
			Result = pShader->Bind_RawValue(
				"g_Ue3ValtanCrack01CB1", &CB1, sizeof(CB1));
		if (SUCCEEDED(Result))
			Result = pShader->Bind_RawValue(
				"g_Ue3ValtanCrack01CB2", CB2.data(), sizeof(CB2));
		if (SUCCEEDED(Result))
			Result = pShader->Bind_RawValue(
				"g_Ue3ValtanCrack01LocalTimeSeconds", &fLocalTimeSeconds,
				sizeof(fLocalTimeSeconds));
		if (SUCCEEDED(Result))
			Result = Bind_Texture_Set(
				pShader, Resource, "g_Ue3ValtanCrack01Texture");
		if (SUCCEEDED(Result))
			Result = CGameInstance::Get().Bind_RT_SRV(
				TEXT("Target_Depth"), pShader, "g_Ue3ValtanCrack01Texture5");
		if (SUCCEEDED(Result))
		{
			const uint32_t iAdmission = 1u;
			Result = pShader->Bind_RawValue(
				"g_Ue3ValtanCrack01ToolAdmission", &iAdmission,
				sizeof(iAdmission));
		}
	}
	if (FAILED(Result))
		return Result;

	Result = pShader->Begin(0u);
	if (SUCCEEDED(Result))
		Result = Apply_Samplers(m_pState->pContext.Get(), Resource);
	if (FAILED(Result))
		return Result;

	bool_t bSubmitted = false;
	for (uint32_t iMesh = 0u; iMesh < pResourceModel->Get_NumMeshes(); ++iMesh)
	{
		Result = pResourceModel->Render(iMesh);
		if (S_OK != Result)
			return Result;
		bSubmitted = true;
	}
	return bSubmitted ? S_OK : S_FALSE;
}

HRESULT Client::CValtanTranslatedCanaryRuntime::Draw_Ground(
	const EFFECT_ELEMENT_DESC& Element,
	const std::shared_ptr<Engine::CVIBuffer_Rect>& pRect,
	const std::shared_ptr<const VALTAN_TRANSLATED_CANARY_ELEMENT_PACKET>&
		pPacket,
	const float4x4_t& InverseDecalWorld,
	const float2_t& vDecalSize,
	const f32_t fDecalDepth,
	const f32_t fLocalTimeSeconds) const
{
	if (nullptr == pPacket)
		return E_POINTER;
	if (pPacket->eFamily != FAMILY::GROUND_DECAL)
		return S_FALSE;
	const TARGET_ELEMENT_PIN* const pTarget = Find_Target_Pin(Element.strElementId);
	if (nullptr == pTarget || pTarget->eFamily != FAMILY::GROUND_DECAL)
		return S_FALSE;
	if (!Is_Armed() || pPacket->iArmGeneration != m_pState->iArmGeneration ||
		pPacket->strElementId != Element.strElementId || nullptr == pRect ||
		!Is_Finite(InverseDecalWorld) || !Is_Finite(vDecalSize) ||
		vDecalSize.x <= 0.f || vDecalSize.y <= 0.f ||
		!std::isfinite(fDecalDepth) || fDecalDepth <= 0.f)
	{
		return E_INVALIDARG;
	}

	const float4x4_t* const pView =
		CGameInstance::Get().Get_Transform(D3DTS::VIEW);
	const float4x4_t* const pProjection =
		CGameInstance::Get().Get_Transform(D3DTS::PROJ);
	if (nullptr == pView || nullptr == pProjection || !Is_Finite(*pView) ||
		!Is_Finite(*pProjection))
	{
		return E_POINTER;
	}
	DirectX::XMVECTOR ViewDeterminant{};
	DirectX::XMVECTOR ProjectionDeterminant{};
	const DirectX::XMMATRIX ViewInverse = DirectX::XMMatrixInverse(
		&ViewDeterminant, DirectX::XMLoadFloat4x4(pView));
	const DirectX::XMMATRIX ProjectionInverse = DirectX::XMMatrixInverse(
		&ProjectionDeterminant, DirectX::XMLoadFloat4x4(pProjection));
	if (std::abs(DirectX::XMVectorGetX(ViewDeterminant)) <= 1.e-8f ||
		std::abs(DirectX::XMVectorGetX(ProjectionDeterminant)) <= 1.e-8f)
	{
		return E_INVALIDARG;
	}
	float4x4_t ViewInverseValue{};
	float4x4_t ProjectionInverseValue{};
	DirectX::XMStoreFloat4x4(&ViewInverseValue, ViewInverse);
	DirectX::XMStoreFloat4x4(&ProjectionInverseValue, ProjectionInverse);

	std::array<float4_t, 20u> CB0{};
	if (!Build_Ground_CB0(fLocalTimeSeconds, CB0))
		return E_INVALIDARG;
	std::array<float4_t, 4u> CB2{};
	/* Ground RT0 consumes cb2[3].  The source engine scene lane is outside
	   material/source-value closure, so bind the same bounded neutral value as
	   Crack instead of leaving the constant buffer undefined. */
	CB2[3] = { 0.f, 0.f, 0.f, 1.f };
	const FAMILY_GPU_RESOURCE& Resource =
		m_pState->Families[Family_Index(FAMILY::GROUND_DECAL)];
	if (nullptr == Resource.pShader)
		return E_FAIL;
	shared_ptr<Engine::CShader> pShader = Resource.pShader;
	CValtanCanaryPipelineStateGuard StateGuard(m_pState->pContext.Get());

	HRESULT Result = pShader->Bind_Matrix("g_ViewMatrix", pView);
	if (SUCCEEDED(Result))
		Result = pShader->Bind_Matrix(
			"g_ViewMatrixInverse", &ViewInverseValue);
	if (SUCCEEDED(Result))
		Result = pShader->Bind_Matrix(
			"g_ProjMatrixInverse", &ProjectionInverseValue);
	if (SUCCEEDED(Result))
		Result = pShader->Bind_Matrix(
			"g_DecalWorldInverse", &InverseDecalWorld);
	if (SUCCEEDED(Result))
		Result = pShader->Bind_RawValue(
			"g_DecalSize", &vDecalSize, sizeof(vDecalSize));
	if (SUCCEEDED(Result))
		Result = pShader->Bind_RawValue(
			"g_DecalDepth", &fDecalDepth, sizeof(fDecalDepth));
	const float2_t SourceTexcoord0ZW{ 1.f, 0.f };
	if (SUCCEEDED(Result))
		Result = pShader->Bind_RawValue(
			"g_Ue3ValtanGround04SourceTexcoord0ZW", &SourceTexcoord0ZW,
			sizeof(SourceTexcoord0ZW));
	if (SUCCEEDED(Result))
		Result = pShader->Bind_RawValue(
			"g_Ue3ValtanGround04CB0", CB0.data(), sizeof(CB0));
	if (SUCCEEDED(Result))
		Result = pShader->Bind_RawValue(
			"g_Ue3ValtanGround04CB2", CB2.data(), sizeof(CB2));
	if (SUCCEEDED(Result))
		Result = pShader->Bind_RawValue(
			"g_Ue3ValtanGround04LocalTimeSeconds", &fLocalTimeSeconds,
			sizeof(fLocalTimeSeconds));
	if (SUCCEEDED(Result))
		Result = Bind_Texture_Set(
			pShader, Resource, "g_Ue3ValtanGround04Texture");
	if (SUCCEEDED(Result))
		Result = CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_Depth"), pShader,
			"g_Ue3ValtanGround04ProjectionDepth");
	if (SUCCEEDED(Result))
	{
		const uint32_t iAdmission = 1u;
		Result = pShader->Bind_RawValue(
			"g_Ue3ValtanGround04ToolAdmission", &iAdmission,
			sizeof(iAdmission));
	}
	if (FAILED(Result))
		return Result;
	Result = pShader->Begin(0u);
	if (SUCCEEDED(Result))
		Result = Apply_Samplers(m_pState->pContext.Get(), Resource);
	if (FAILED(Result))
		return Result;
	Result = pRect->Bind_Resources();
	if (S_OK != Result)
		return Result;
	return pRect->Render();
}

Client::VALTAN_TRANSLATED_CANARY_FAMILY
Client::CValtanTranslatedCanaryRuntime::Get_Family(
	const VALTAN_TRANSLATED_CANARY_ELEMENT_PACKET& Packet) noexcept
{
	return Packet.eFamily;
}

bool_t Client::CValtanTranslatedCanaryRuntime::Is_Target_ElementId(
	const std::string_view strElementId) noexcept
{
	return nullptr != Find_Target_Pin(strElementId);
}

bool_t Client::CValtanTranslatedCanaryRuntime::Is_Ground_ElementId(
	const std::string_view strElementId) noexcept
{
	const TARGET_ELEMENT_PIN* const pPin = Find_Target_Pin(strElementId);
	return nullptr != pPin && pPin->eFamily == FAMILY::GROUND_DECAL;
}
