#include "Effect_DocumentRenderer_Internal.h"
#include "Effect_RuntimeAuthority.h"
#include "RuntimeAssetRoot.h"
#include "Shader.h"
#include "VIBuffer_DynamicTrail.h"
#include "VIBuffer_ParticleRect.h"
#include "VIBuffer_Rect.h"
#include <d3d11sdklayers.h>
#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <optional>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Model.h"
#include "Engine_RenderTypes.h"

namespace EffectDocumentRendererDetail
{


	std::mutex g_EffectRenderCacheMutex;
	std::mutex g_EffectRendererCoreBuildMutex;
	std::unordered_map<ID3D11Device*, std::shared_ptr<EFFECT_RENDERER_CORE>>
		g_EffectRendererCores;
	std::unordered_map<ID3D11Device*, std::string>
		g_EffectRendererCoreFailures;
	std::map<PREPARED_KEY,
		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>>
		g_PreparedEffectDocuments;
	std::unordered_map<const Client::EFFECT_DOCUMENT_DESC*,
		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>>
		g_PreparedEffectDocumentsByIdentity;
	std::shared_ptr<Client::CEffectDocumentRenderer::PRODUCT_PREWARM_SESSION>
		g_pProductPrewarmSession;
	ID3D11Device* g_pPreparedDevice = nullptr;
	uint64_t g_iPreparedCatalogRevision = 0u;
	uint64_t g_iPreparedCatalogGeneration = 0u;
	Client::EFFECT_RENDER_PREWARM_PROBE g_EffectRenderPrewarmProbe;

	bool_t Read_ReconstructedAssetBytes(
		const std::string& strAssetId,
		const uint64_t iExpectedByteCount,
		const std::string& strExpectedSha256,
		std::filesystem::path& OutPath,
		std::vector<uint8_t>& OutBytes,
		std::string& strOutError)
	{
		const std::filesystem::path Path =
			Client::CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
		if (Path.empty() || !std::filesystem::is_regular_file(Path))
		{
			strOutError = "Reconstructed diagnostic asset is missing: " +
				strAssetId;
			return false;
		}
		std::ifstream Input(Path, std::ios::binary | std::ios::ate);
		if (!Input)
		{
			strOutError = "Reconstructed diagnostic asset could not be opened: " +
				strAssetId;
			return false;
		}
		const std::streamoff Size = Input.tellg();
		if (Size < 0 || static_cast<uint64_t>(Size) != iExpectedByteCount)
		{
			strOutError = "Reconstructed diagnostic asset byte count changed: " +
				strAssetId;
			return false;
		}
		std::vector<uint8_t> Bytes(static_cast<size_t>(Size));
		Input.seekg(0, std::ios::beg);
		if (!Bytes.empty() && !Input.read(
			reinterpret_cast<char*>(Bytes.data()), Size))
		{
			strOutError = "Reconstructed diagnostic asset read failed: " +
				strAssetId;
			return false;
		}
		const std::string_view ByteView(
			reinterpret_cast<const char*>(Bytes.data()), Bytes.size());
		if (Client::CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(ByteView) !=
			strExpectedSha256)
		{
			strOutError = "Reconstructed diagnostic asset SHA-256 changed: " +
				strAssetId;
			return false;
		}
		OutPath = Path;
		OutBytes = std::move(Bytes);
		return true;
	}

	std::string Sha256Hex(const std::array<uint8_t, 32u>& Bytes)
	{
		static constexpr char Hex[] = "0123456789abcdef";
		std::string Result(Bytes.size() * 2u, '0');
		for (size_t Index = 0u; Index < Bytes.size(); ++Index)
		{
			Result[Index * 2u] = Hex[Bytes[Index] >> 4u];
			Result[Index * 2u + 1u] = Hex[Bytes[Index] & 0x0fu];
		}
		return Result;
	}

	bool_t Same_SamplerDescriptor(
		const D3D11_SAMPLER_DESC& Left,
		const D3D11_SAMPLER_DESC& Right)
	{
		return Left.Filter == Right.Filter &&
			Left.AddressU == Right.AddressU &&
			Left.AddressV == Right.AddressV &&
			Left.AddressW == Right.AddressW &&
			Left.MipLODBias == Right.MipLODBias &&
			Left.MaxAnisotropy == Right.MaxAnisotropy &&
			Left.ComparisonFunc == Right.ComparisonFunc &&
			std::equal(std::begin(Left.BorderColor), std::end(Left.BorderColor),
				std::begin(Right.BorderColor)) &&
			Left.MinLOD == Right.MinLOD && Left.MaxLOD == Right.MaxLOD;
	}

	bool_t Is_AnisotropicSamplerFilter(const D3D11_FILTER Filter)
	{
		return Filter == D3D11_FILTER_ANISOTROPIC ||
			Filter == D3D11_FILTER_COMPARISON_ANISOTROPIC ||
			Filter == D3D11_FILTER_MINIMUM_ANISOTROPIC ||
			Filter == D3D11_FILTER_MAXIMUM_ANISOTROPIC;
	}

	bool_t Same_RuntimeSamplerReadbackDescriptor(
		const D3D11_SAMPLER_DESC& Actual,
		const D3D11_SAMPLER_DESC& Requested,
		const D3D11_SAMPLER_DESC& FrozenAuthority)
	{
		D3D11_SAMPLER_DESC CanonicalRequested = Requested;
		if (!Is_AnisotropicSamplerFilter(FrozenAuthority.Filter) &&
			0u == FrozenAuthority.MaxAnisotropy &&
			1u == Requested.MaxAnisotropy)
		{
			// D3D11 requires a legal creation value even though this field is
			// ignored for non-anisotropic filters.  GetDesc canonicalizes that
			// materialized value back to the frozen semantic zero.
			CanonicalRequested.MaxAnisotropy = 0u;
		}
		return Same_SamplerDescriptor(Actual, CanonicalRequested);
	}

	bool_t Materialize_RuntimeSamplerDescriptor(
		const D3D11_SAMPLER_DESC& Authority,
		D3D11_SAMPLER_DESC& OutRuntime)
	{
		OutRuntime = Authority;
		const bool_t bAnisotropic = Is_AnisotropicSamplerFilter(Authority.Filter);
		if (0u == OutRuntime.MaxAnisotropy)
		{
			// The frozen UE3 sampler evidence records zero when anisotropy is
			// disabled.  D3D11 still requires the otherwise ignored field to be
			// in [1, 16], so materialize the unique non-anisotropic identity.
			if (bAnisotropic)
				return false;
			OutRuntime.MaxAnisotropy = 1u;
		}
		return OutRuntime.MaxAnisotropy <= 16u;
	}

	bool_t Try_ToAuthoredFilter(
		const D3D11_FILTER Source,
		Client::EFFECT_MATERIAL_TEXTURE_FILTER& Out)
	{
		switch (Source)
		{
		case D3D11_FILTER_MIN_MAG_MIP_POINT:
			Out = Client::EFFECT_MATERIAL_TEXTURE_FILTER::POINT;
			return true;
		case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
			Out = Client::EFFECT_MATERIAL_TEXTURE_FILTER::LINEAR;
			return true;
		case D3D11_FILTER_ANISOTROPIC:
			Out = Client::EFFECT_MATERIAL_TEXTURE_FILTER::ANISOTROPIC;
			return true;
		default:
			return false;
		}
	}

	bool_t Try_ToD3dFilter(
		const Client::EFFECT_MATERIAL_TEXTURE_FILTER Source,
		D3D11_FILTER& Out)
	{
		switch (Source)
		{
		case Client::EFFECT_MATERIAL_TEXTURE_FILTER::POINT:
			Out = D3D11_FILTER_MIN_MAG_MIP_POINT;
			return true;
		case Client::EFFECT_MATERIAL_TEXTURE_FILTER::LINEAR:
			Out = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			return true;
		case Client::EFFECT_MATERIAL_TEXTURE_FILTER::ANISOTROPIC:
			Out = D3D11_FILTER_ANISOTROPIC;
			return true;
		case Client::EFFECT_MATERIAL_TEXTURE_FILTER::END:
		default:
			return false;
		}
	}

	bool_t Try_ToAuthoredAddress(
		const D3D11_TEXTURE_ADDRESS_MODE Source,
		Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE& Out)
	{
		switch (Source)
		{
		case D3D11_TEXTURE_ADDRESS_WRAP:
			Out = Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP;
			return true;
		case D3D11_TEXTURE_ADDRESS_MIRROR:
			Out = Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::MIRROR;
			return true;
		case D3D11_TEXTURE_ADDRESS_CLAMP:
			Out = Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::CLAMP;
			return true;
		case D3D11_TEXTURE_ADDRESS_BORDER:
			Out = Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::BORDER;
			return true;
		default:
			return false;
		}
	}

	bool_t Try_ToD3dAddress(
		const Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE Source,
		D3D11_TEXTURE_ADDRESS_MODE& Out)
	{
		switch (Source)
		{
		case Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP:
			Out = D3D11_TEXTURE_ADDRESS_WRAP;
			return true;
		case Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::MIRROR:
			Out = D3D11_TEXTURE_ADDRESS_MIRROR;
			return true;
		case Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::CLAMP:
			Out = D3D11_TEXTURE_ADDRESS_CLAMP;
			return true;
		case Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::BORDER:
			Out = D3D11_TEXTURE_ADDRESS_BORDER;
			return true;
		case Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::END:
		default:
			return false;
		}
	}

	bool_t Try_ToAuthoredComparison(
		const D3D11_COMPARISON_FUNC Source,
		Client::EFFECT_MATERIAL_COMPARISON_FUNCTION& Out)
	{
		if (Source < D3D11_COMPARISON_NEVER ||
			Source > D3D11_COMPARISON_ALWAYS)
		{
			return false;
		}
		Out = static_cast<Client::EFFECT_MATERIAL_COMPARISON_FUNCTION>(
			static_cast<uint32_t>(Source) - 1u);
		return true;
	}

	bool_t Try_ToD3dComparison(
		const Client::EFFECT_MATERIAL_COMPARISON_FUNCTION Source,
		D3D11_COMPARISON_FUNC& Out)
	{
		if (Source >= Client::EFFECT_MATERIAL_COMPARISON_FUNCTION::END)
			return false;
		Out = static_cast<D3D11_COMPARISON_FUNC>(
			static_cast<uint32_t>(Source) + 1u);
		return true;
	}

	bool_t Try_ToAuthoredSampler(
		const D3D11_SAMPLER_DESC& Source,
		Client::EFFECT_MATERIAL_SAMPLER_DESC& Out)
	{
		Client::EFFECT_MATERIAL_SAMPLER_DESC Staged;
		if (!Try_ToAuthoredFilter(Source.Filter, Staged.eFilter) ||
			!Try_ToAuthoredAddress(Source.AddressU, Staged.eAddressU) ||
			!Try_ToAuthoredAddress(Source.AddressV, Staged.eAddressV) ||
			!Try_ToAuthoredAddress(Source.AddressW, Staged.eAddressW) ||
			!Try_ToAuthoredComparison(Source.ComparisonFunc, Staged.eComparison) ||
			!std::isfinite(Source.MipLODBias) ||
			!std::isfinite(Source.MinLOD) || !std::isfinite(Source.MaxLOD) ||
			Source.MaxAnisotropy > 16u)
		{
			return false;
		}
		Staged.fMipLodBias = Source.MipLODBias;
		/* D3D readback may report zero for a non-anisotropic sampler even
		   though MaxAnisotropy is semantically unused.  The authored contract
		   has no zero state, so store its canonical inert value. */
		Staged.iMaxAnisotropy = (std::max)(1u, Source.MaxAnisotropy);
		Staged.vBorderColor = { Source.BorderColor[0u], Source.BorderColor[1u],
			Source.BorderColor[2u], Source.BorderColor[3u] };
		Staged.fMinLod = Source.MinLOD;
		Staged.fMaxLod = Source.MaxLOD;
		Out = Staged;
		return true;
	}

	bool_t Try_ToD3dSampler(
		const Client::EFFECT_MATERIAL_SAMPLER_DESC& Source,
		D3D11_SAMPLER_DESC& Out)
	{
		D3D11_SAMPLER_DESC Staged{};
		if (!Try_ToD3dFilter(Source.eFilter, Staged.Filter) ||
			!Try_ToD3dAddress(Source.eAddressU, Staged.AddressU) ||
			!Try_ToD3dAddress(Source.eAddressV, Staged.AddressV) ||
			!Try_ToD3dAddress(Source.eAddressW, Staged.AddressW) ||
			!Try_ToD3dComparison(Source.eComparison, Staged.ComparisonFunc) ||
			!std::isfinite(Source.fMipLodBias) ||
			!std::isfinite(Source.fMinLod) || !std::isfinite(Source.fMaxLod) ||
			Source.iMaxAnisotropy == 0u || Source.iMaxAnisotropy > 16u ||
			Source.fMinLod > Source.fMaxLod)
		{
			return false;
		}
		Staged.MipLODBias = Source.fMipLodBias;
		Staged.MaxAnisotropy = Source.iMaxAnisotropy;
		Staged.BorderColor[0u] = Source.vBorderColor.x;
		Staged.BorderColor[1u] = Source.vBorderColor.y;
		Staged.BorderColor[2u] = Source.vBorderColor.z;
		Staged.BorderColor[3u] = Source.vBorderColor.w;
		Staged.MinLOD = Source.fMinLod;
		Staged.MaxLOD = Source.fMaxLod;
		Out = Staged;
		return true;
	}

	bool_t Same_BlendDescriptor(
		const D3D11_BLEND_DESC& Left,
		const D3D11_BLEND_DESC& Right)
	{
		if (Left.AlphaToCoverageEnable != Right.AlphaToCoverageEnable ||
			Left.IndependentBlendEnable != Right.IndependentBlendEnable)
			return false;
		for (size_t Index = 0u; Index < 8u; ++Index)
		{
			const D3D11_RENDER_TARGET_BLEND_DESC& A = Left.RenderTarget[Index];
			const D3D11_RENDER_TARGET_BLEND_DESC& B = Right.RenderTarget[Index];
			if (A.BlendEnable != B.BlendEnable || A.SrcBlend != B.SrcBlend ||
				A.DestBlend != B.DestBlend || A.BlendOp != B.BlendOp ||
				A.SrcBlendAlpha != B.SrcBlendAlpha ||
				A.DestBlendAlpha != B.DestBlendAlpha ||
				A.BlendOpAlpha != B.BlendOpAlpha ||
				A.RenderTargetWriteMask != B.RenderTargetWriteMask)
				return false;
		}
		return true;
	}

	bool_t Same_RasterizerDescriptor(
		const D3D11_RASTERIZER_DESC& Left,
		const D3D11_RASTERIZER_DESC& Right)
	{
		return Left.FillMode == Right.FillMode &&
			Left.CullMode == Right.CullMode &&
			Left.FrontCounterClockwise == Right.FrontCounterClockwise &&
			Left.DepthBias == Right.DepthBias &&
			Left.DepthBiasClamp == Right.DepthBiasClamp &&
			Left.SlopeScaledDepthBias == Right.SlopeScaledDepthBias &&
			Left.DepthClipEnable == Right.DepthClipEnable &&
			Left.ScissorEnable == Right.ScissorEnable &&
			Left.MultisampleEnable == Right.MultisampleEnable &&
			Left.AntialiasedLineEnable == Right.AntialiasedLineEnable;
	}

	bool_t Same_DepthStencilOperation(
		const D3D11_DEPTH_STENCILOP_DESC& Left,
		const D3D11_DEPTH_STENCILOP_DESC& Right)
	{
		return Left.StencilFailOp == Right.StencilFailOp &&
			Left.StencilDepthFailOp == Right.StencilDepthFailOp &&
			Left.StencilPassOp == Right.StencilPassOp &&
			Left.StencilFunc == Right.StencilFunc;
	}

	bool_t Same_DepthStencilDescriptor(
		const D3D11_DEPTH_STENCIL_DESC& Left,
		const D3D11_DEPTH_STENCIL_DESC& Right)
	{
		return Left.DepthEnable == Right.DepthEnable &&
			Left.DepthWriteMask == Right.DepthWriteMask &&
			Left.DepthFunc == Right.DepthFunc &&
			Left.StencilEnable == Right.StencilEnable &&
			Left.StencilReadMask == Right.StencilReadMask &&
			Left.StencilWriteMask == Right.StencilWriteMask &&
			Same_DepthStencilOperation(Left.FrontFace, Right.FrontFace) &&
			Same_DepthStencilOperation(Left.BackFace, Right.BackFace);
	}

	uint64_t Build_ResourceSignature(
		const Client::EFFECT_DOCUMENT_DESC& Document)
	{
		/* Product documents are immutable for one catalog revision.  Their
		   address and stable asset id already form the exact in-process cache
		   identity; serializing multi-megabyte JSON here repeated work performed
		   by the catalog parser on every prewarm target. */
		uint64_t Hash = static_cast<uint64_t>(
			reinterpret_cast<uintptr_t>(&Document));
		for (const unsigned char Byte : Document.strEffectAssetId)
		{
			Hash ^= static_cast<uint64_t>(Byte);
			Hash *= 1099511628211ull;
		}
		return Hash;
	}

	bool_t Needs_ParticleInstanceBuffer(
		const Client::EFFECT_DOCUMENT_DESC& Document)
	{
		return std::any_of(
			Document.Elements.begin(), Document.Elements.end(),
			[](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				if (Client::Is_EffectSimulationOnlyParticle(Element))
					return false;
				if (Client::EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind)
					return true;
				/* An admitted source recipe preserves its source carrier kind while
				   producing runtime particles. */
				return Element.SourceRecipe.bEnabled &&
					((Client::EFFECT_ELEMENT_KIND::MESH == Element.eKind &&
						Element.SourceRecipe.strRendererShape == "mesh") ||
					 (Client::EFFECT_ELEMENT_KIND::SPRITE == Element.eKind &&
						Element.SourceRecipe.strRendererShape == "sprite"));
			});
	}

	bool_t Try_ResolveTrailBufferPointCapacity(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		uint32_t& iOutPointCapacity,
		std::string& strOutError)
	{
		iOutPointCapacity = 0u;
		for (const Client::EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (Client::EFFECT_ELEMENT_KIND::TRAIL != Element.eKind)
				continue;
			uint32_t iElementRenderPoints = Element.Detail.Trail.iMaxPoints;
			if (Element.Detail.Trail.fDistanceTessellationStepWorldUnits > 0.f)
			{
				constexpr uint32_t MAX_SEGMENT_SUBDIVISIONS = 25u;
				if (Element.Detail.Trail.iMaxPoints >
					((std::numeric_limits<uint32_t>::max)() - 1u) /
						MAX_SEGMENT_SUBDIVISIONS + 1u)
				{
					strOutError =
						"Effect trail tessellation capacity overflowed.";
					return false;
				}
				iElementRenderPoints = 1u +
					(Element.Detail.Trail.iMaxPoints - 1u) *
					MAX_SEGMENT_SUBDIVISIONS;
			}
			iOutPointCapacity = (std::max)(
				iOutPointCapacity, iElementRenderPoints);
		}
		if (0u != iOutPointCapacity)
			iOutPointCapacity = (std::max)(256u, iOutPointCapacity);
		return true;
	}

	HRESULT Create_SolidTexture(
		ID3D11Device* pDevice,
		const uint32_t iRGBA,
		ComPtr<ID3D11ShaderResourceView>& OutSRV)
	{
		if (nullptr == pDevice)
			return E_INVALIDARG;
		D3D11_TEXTURE2D_DESC Desc{};
		Desc.Width = 1u;
		Desc.Height = 1u;
		Desc.MipLevels = 1u;
		Desc.ArraySize = 1u;
		Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		Desc.SampleDesc.Count = 1u;
		Desc.Usage = D3D11_USAGE_IMMUTABLE;
		Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		D3D11_SUBRESOURCE_DATA Data{};
		Data.pSysMem = &iRGBA;
		Data.SysMemPitch = sizeof(iRGBA);
		ComPtr<ID3D11Texture2D> Texture;
		if (FAILED(pDevice->CreateTexture2D(&Desc, &Data, &Texture)))
			return E_FAIL;
		return pDevice->CreateShaderResourceView(
			Texture.Get(), nullptr, &OutSRV);
	}

	std::shared_ptr<EFFECT_RENDERER_CORE> Build_RendererCore(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext)
	{
		if (nullptr == pDevice || nullptr == pContext)
			return nullptr;
		auto Core = std::make_shared<EFFECT_RENDERER_CORE>();
        for (size_t i = 0u; i < Client::EFFECT_SHADER_PROGRAMS.size(); ++i)
        {
            const auto& Program = Client::EFFECT_SHADER_PROGRAMS[i];
            const bool Mesh = Program.eCarrier == Client::EFFECT_SHADER_CARRIER::MESH;
            const bool NativeMesh = Mesh && Program.eFamily != Client::EFFECT_SHADER_FAMILY::GENERIC;
            Core->ShaderPrograms[i] = Engine::CShader::Create(pDevice, pContext,
                Program.pLogicalShaderPath,
                NativeMesh ? NativeMeshInstanceElements.data() :
                    (Mesh ? VTXMESH::Elements : Engine::VTXEFFECT_PARTICLE::Elements),
                NativeMesh ? static_cast<uint32_t>(NativeMeshInstanceElements.size()) :
                    (Mesh ? VTXMESH::iNumElements : Engine::VTXEFFECT_PARTICLE::iNumElements));
            if (nullptr == Core->ShaderPrograms[i]) return nullptr;
        }
        Core->pMeshShader = Core->ShaderPrograms[Client::Find_EffectShaderProgramIndex(
            Client::EFFECT_SHADER_CARRIER::MESH, Client::EFFECT_SHADER_FAMILY::GENERIC, 0u)];
        Core->pParticleShader = Core->ShaderPrograms[Client::Find_EffectShaderProgramIndex(
            Client::EFFECT_SHADER_CARRIER::PARTICLE, Client::EFFECT_SHADER_FAMILY::GENERIC, 0u)];
		Core->pAnimatedModelShader = Engine::CShader::Create(
			pDevice, pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl"),
			VTXANIMMESH::Elements, VTXANIMMESH::iNumElements);
		Core->pNativeScreenPostShader = Engine::CShader::Create(pDevice, pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxEffectNativeScreenPost.hlsl"), VTXTEX::Elements, VTXTEX::iNumElements);
		Core->pRectShader = Engine::CShader::Create(
			pDevice, pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxEffectRectPreview.hlsl"),
			VTXTEX::Elements, VTXTEX::iNumElements);
		Core->pTrailShader = Engine::CShader::Create(
			pDevice, pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxEffectTrail.hlsl"),
			Engine::VTXEFFECT_TRAIL::Elements,
			Engine::VTXEFFECT_TRAIL::iNumElements);
		Core->pDecalShader = Engine::CShader::Create(
			pDevice, pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxEffectDecal.hlsl"),
			VTXTEX::Elements, VTXTEX::iNumElements);
		Core->pRect = Engine::CVIBuffer_Rect::Create(pDevice, pContext);
		/* The immediate context consumes particle instances and draws them before
		   the next Effect is visited, so one device-local dynamic buffer is shared
		   safely by every renderer instance. This keeps action-edge attach free of
		   GPU buffer creation. */
		Core->pParticleBuffer = Engine::CVIBuffer_ParticleRect::Create(
			pDevice, pContext, 2048u);
		if (nullptr == Core->pMeshShader ||
			nullptr == Core->pAnimatedModelShader ||
			nullptr == Core->pNativeScreenPostShader ||
			nullptr == Core->pRectShader ||
			nullptr == Core->pParticleShader ||
			nullptr == Core->pTrailShader ||
			nullptr == Core->pDecalShader || nullptr == Core->pRect ||
			nullptr == Core->pParticleBuffer ||
			FAILED(Create_SolidTexture(
				pDevice.Get(), 0xffffffffu, Core->pWhiteTexture)) ||
			FAILED(Create_SolidTexture(
				pDevice.Get(), 0xff000000u, Core->pBlackTexture)))
		{
			return nullptr;
		}
		return Core;
	}

	std::shared_ptr<EFFECT_RENDERER_CORE> Acquire_RendererCore(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext)
	{
		if (nullptr == pDevice || nullptr == pContext)
			return nullptr;
		{
			const std::scoped_lock Lock(g_EffectRenderCacheMutex);
			const auto Existing = g_EffectRendererCores.find(pDevice.Get());
			if (Existing != g_EffectRendererCores.end())
				return Existing->second;
			if (g_EffectRendererCoreFailures.contains(pDevice.Get()))
				return nullptr;
		}

		/* Shader and buffer construction can perform file I/O and device work.
		   Keep that work outside the cache lock so probes and an already-built
		   core never wait behind a cold first-use candidate.  The dedicated cold
		   build lock prevents duplicate candidates and makes a failed CSO closure
		   a single root failure for this device instead of retrying six shaders for
		   every Product target. */
		const std::scoped_lock BuildLock(g_EffectRendererCoreBuildMutex);
		{
			const std::scoped_lock Lock(g_EffectRenderCacheMutex);
			const auto Existing = g_EffectRendererCores.find(pDevice.Get());
			if (Existing != g_EffectRendererCores.end())
				return Existing->second;
			if (g_EffectRendererCoreFailures.contains(pDevice.Get()))
				return nullptr;
		}
		const auto CoreBuildStarted = std::chrono::steady_clock::now();
		std::shared_ptr<EFFECT_RENDERER_CORE> Staged =
			Build_RendererCore(pDevice, pContext);
		const uint64_t iCoreBuildMicroseconds = static_cast<uint64_t>(
			std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::steady_clock::now() - CoreBuildStarted).count());

		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		g_EffectRenderPrewarmProbe.iCoreBuildMaximumMicroseconds = (std::max)(
			g_EffectRenderPrewarmProbe.iCoreBuildMaximumMicroseconds,
			iCoreBuildMicroseconds);
		const auto Existing = g_EffectRendererCores.find(pDevice.Get());
		if (Existing != g_EffectRendererCores.end())
			return Existing->second;
		if (nullptr == Staged)
		{
			g_EffectRendererCoreFailures.emplace(
				pDevice.Get(), "Effect renderer compiled-shader core build failed.");
			++g_EffectRenderPrewarmProbe.iCoreBuildFailureCount;
			return nullptr;
		}
		const auto [Inserted, bInserted] =
			g_EffectRendererCores.emplace(pDevice.Get(), std::move(Staged));
		if (!bInserted)
			return nullptr;
		++g_EffectRenderPrewarmProbe.iCoreBuildCount;
		++g_EffectRenderPrewarmProbe.iMutableInstanceBufferBuildCount;
		return Inserted->second;
	}
}
