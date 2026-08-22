#include "Effect_ScreenOverlayPresentation.h"

#include "DataJson.h"
#include "Presentation_Manager.h"
#include "RuntimeAssetRoot.h"

#include "DirectXTK/DDSTextureLoader.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <initializer_list>
#include <limits>
#include <set>

namespace
{
	using namespace Client;
	using namespace Engine;

	constexpr size_t MAX_SCREEN_OVERLAY_ROWS = 64u;

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& Object,
		const char_t* pName,
		const DATA_JSON_TYPE eType)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(pName);
		return nullptr != pValue && pValue->Get_Type() == eType ?
			pValue : nullptr;
	}

	bool_t HasOnlyFields(
		const DATA_JSON_VALUE& Object,
		const std::initializer_list<std::string_view> Fields)
	{
		if (!Object.Is_Object() || Object.Get_Object().size() != Fields.size())
			return false;
		for (const auto& [Name, Value] : Object.Get_Object())
		{
			(void)Value;
			if (std::find(Fields.begin(), Fields.end(), Name) == Fields.end())
				return false;
		}
		return true;
	}

	bool_t IsStableId(const string& Value)
	{
		if (Value.empty() || Value.size() > 128u)
			return false;
		return std::all_of(Value.begin(), Value.end(), [](const char_t Value)
			{
				return (Value >= 'a' && Value <= 'z') ||
					(Value >= 'A' && Value <= 'Z') ||
					(Value >= '0' && Value <= '9') ||
					Value == '.' || Value == '_' || Value == '-';
			});
	}

	bool_t ReadU32(
		const DATA_JSON_VALUE& Object,
		const char_t* pName,
		uint32_t& OutValue)
	{
		const DATA_JSON_VALUE* pValue = Required(
			Object, pName, DATA_JSON_TYPE::NUMBER);
		if (nullptr == pValue || pValue->Was_FloatingPointToken() ||
			!std::isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() < 0.0 ||
			pValue->Get_Number() >
				static_cast<double>((std::numeric_limits<uint32_t>::max)()))
		{
			return false;
		}
		const double Value = pValue->Get_Number();
		OutValue = static_cast<uint32_t>(Value);
		return static_cast<double>(OutValue) == Value;
	}

	bool_t ReadFloat(
		const DATA_JSON_VALUE& Object,
		const char_t* pName,
		const f32_t fMinimum,
		const f32_t fMaximum,
		f32_t& OutValue)
	{
		const DATA_JSON_VALUE* pValue = Required(
			Object, pName, DATA_JSON_TYPE::NUMBER);
		if (nullptr == pValue || !std::isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() < static_cast<double>(fMinimum) ||
			pValue->Get_Number() > static_cast<double>(fMaximum))
		{
			return false;
		}
		OutValue = static_cast<f32_t>(pValue->Get_Number());
		return std::isfinite(OutValue);
	}

	bool_t ReadFloat2(
		const DATA_JSON_VALUE& Object,
		const char_t* pName,
		const f32_t fMinimum,
		const f32_t fMaximum,
		float2_t& OutValue)
	{
		const DATA_JSON_VALUE* pValue = Required(
			Object, pName, DATA_JSON_TYPE::ARRAY);
		if (nullptr == pValue || pValue->Get_Array().size() != 2u)
			return false;
		float2_t Staged;
		for (size_t i = 0u; i < 2u; ++i)
		{
			const DATA_JSON_VALUE& Component = pValue->Get_Array()[i];
			if (!Component.Is_Number() ||
				!std::isfinite(Component.Get_Number()) ||
				Component.Get_Number() < static_cast<double>(fMinimum) ||
				Component.Get_Number() > static_cast<double>(fMaximum))
			{
				return false;
			}
			(&Staged.x)[i] = static_cast<f32_t>(Component.Get_Number());
		}
		OutValue = Staged;
		return true;
	}

	bool_t ReadTint(
		const DATA_JSON_VALUE& Object,
		float4_t& OutValue)
	{
		const DATA_JSON_VALUE* pValue = Required(
			Object, "tint", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pValue || pValue->Get_Array().size() != 4u)
			return false;
		float4_t Staged;
		for (size_t i = 0u; i < 4u; ++i)
		{
			const DATA_JSON_VALUE& Component = pValue->Get_Array()[i];
			const double Maximum = i < 3u ? 64.0 : 1.0;
			if (!Component.Is_Number() ||
				!std::isfinite(Component.Get_Number()) ||
				Component.Get_Number() < 0.0 ||
				Component.Get_Number() > Maximum)
			{
				return false;
			}
			(&Staged.x)[i] = static_cast<f32_t>(Component.Get_Number());
		}
		OutValue = Staged;
		return true;
	}

	bool_t IsEffectDdsAssetId(
		const string& AssetId,
		std::filesystem::path& OutPath)
	{
		if (AssetId.empty() || AssetId.size() > 512u)
			return false;
		const std::filesystem::path Relative =
			std::filesystem::path(AssetId).lexically_normal();
		if (Relative.empty() || Relative.is_absolute() ||
			Relative.has_root_path() || Relative.begin() == Relative.end() ||
			*Relative.begin() != L"Effect")
		{
			return false;
		}
		string Extension = Relative.extension().string();
		std::transform(Extension.begin(), Extension.end(), Extension.begin(),
			[](const char_t Value)
			{
				return Value >= 'A' && Value <= 'Z' ?
					static_cast<char_t>(Value - 'A' + 'a') : Value;
			});
		if (Extension != ".dds")
			return false;
		OutPath = CRuntimeAssetRoot::Resolve(Relative);
		return !OutPath.empty() && std::filesystem::is_regular_file(OutPath);
	}

	bool_t IsSrgbFormat(const DXGI_FORMAT eFormat)
	{
		switch (eFormat)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return true;
		default:
			return false;
		}
	}

	bool_t HasCoverageChannel(
		const DXGI_FORMAT eFormat,
		const PRESENTATION_SCREEN_OVERLAY_CHANNEL eChannel)
	{
		if (PRESENTATION_SCREEN_OVERLAY_CHANNEL::R == eChannel)
			return DXGI_FORMAT_UNKNOWN != eFormat;
		const bool_t bHasRgb =
			eFormat == DXGI_FORMAT_R8G8B8A8_UNORM ||
			eFormat == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_B8G8R8A8_UNORM ||
			eFormat == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_B8G8R8X8_UNORM ||
			eFormat == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_R16G16B16A16_FLOAT ||
			eFormat == DXGI_FORMAT_R32G32B32A32_FLOAT ||
			eFormat == DXGI_FORMAT_BC1_UNORM ||
			eFormat == DXGI_FORMAT_BC1_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC2_UNORM ||
			eFormat == DXGI_FORMAT_BC2_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC3_UNORM ||
			eFormat == DXGI_FORMAT_BC3_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC7_UNORM ||
			eFormat == DXGI_FORMAT_BC7_UNORM_SRGB;
		if (PRESENTATION_SCREEN_OVERLAY_CHANNEL::G == eChannel)
			return bHasRgb || eFormat == DXGI_FORMAT_R8G8_UNORM ||
				eFormat == DXGI_FORMAT_R16G16_FLOAT ||
				eFormat == DXGI_FORMAT_R32G32_FLOAT;
		if (PRESENTATION_SCREEN_OVERLAY_CHANNEL::B == eChannel)
			return bHasRgb;
		return eFormat == DXGI_FORMAT_R8G8B8A8_UNORM ||
			eFormat == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_B8G8R8A8_UNORM ||
			eFormat == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_R16G16B16A16_FLOAT ||
			eFormat == DXGI_FORMAT_R32G32B32A32_FLOAT ||
			eFormat == DXGI_FORMAT_BC2_UNORM ||
			eFormat == DXGI_FORMAT_BC2_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC3_UNORM ||
			eFormat == DXGI_FORMAT_BC3_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC7_UNORM ||
			eFormat == DXGI_FORMAT_BC7_UNORM_SRGB;
	}
}

Client::CEffectScreenOverlayPresentation::
	CEffectScreenOverlayPresentation(ComPtr<ID3D11Device> pDevice) :
	m_pDevice(std::move(pDevice))
{
}

shared_ptr<Client::CEffectScreenOverlayPresentation>
Client::CEffectScreenOverlayPresentation::Create(
	ComPtr<ID3D11Device> pDevice)
{
	if (nullptr == pDevice)
		return nullptr;
	return std::make_shared<CEffectScreenOverlayPresentation>(
		std::move(pDevice));
}

shared_ptr<Client::CEffectScreenOverlayPresentation>
Client::CEffectScreenOverlayPresentation::Clone_PlaybackInstance() const
{
	if (nullptr == m_pDevice || m_PreparedOverlays.empty() ||
		m_strPresentationId.empty() || m_iCommittedGeneration == 0u ||
		!std::isfinite(m_fMaximumEndSeconds) || m_fMaximumEndSeconds <= 0.f)
	{
		return nullptr;
	}
	shared_ptr<CEffectScreenOverlayPresentation> Instance =
		Create(m_pDevice);
	if (nullptr == Instance)
		return nullptr;
	Instance->m_PreparedOverlays = m_PreparedOverlays;
	Instance->m_strPresentationId = m_strPresentationId;
	Instance->m_iCommittedGeneration = m_iCommittedGeneration;
	Instance->m_fMaximumEndSeconds = m_fMaximumEndSeconds;
	Instance->Stop();
	return Instance;
}

bool_t Client::CEffectScreenOverlayPresentation::Stage_AndCommit(
	const std::string_view strUtf8Json,
	std::string& strOutError)
{
	strOutError.clear();
	if (nullptr == m_pDevice)
	{
		strOutError = "Screen overlay staging has no D3D device.";
		return false;
	}

	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(strUtf8Json, Root, strOutError) ||
		!HasOnlyFields(Root,
			{ "schema", "formatVersion", "provenance",
			  "presentationId", "overlays" }))
	{
		if (strOutError.empty())
			strOutError = "Screen overlay root contract is invalid.";
		return false;
	}
	const DATA_JSON_VALUE* pSchema = Required(
		Root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* pVersion = Required(
		Root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* pProvenance = Required(
		Root, "provenance", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* pPresentationId = Required(
		Root, "presentationId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* pOverlays = Required(
		Root, "overlays", DATA_JSON_TYPE::ARRAY);
	if (nullptr == pSchema || pSchema->Get_String() !=
			"lostark.effect-screen-overlay" ||
		nullptr == pVersion || pVersion->Was_FloatingPointToken() ||
		pVersion->Get_Number() != 1.0 ||
		nullptr == pProvenance ||
		pProvenance->Get_String() != "PROJECT_TUNED" ||
		nullptr == pPresentationId ||
		!IsStableId(pPresentationId->Get_String()) ||
		nullptr == pOverlays || pOverlays->Get_Array().empty() ||
		pOverlays->Get_Array().size() > MAX_SCREEN_OVERLAY_ROWS)
	{
		strOutError = "Screen overlay schema, version, provenance, identity, or row count is invalid.";
		return false;
	}

	vector<PREPARED_OVERLAY> StagedOverlays;
	StagedOverlays.reserve(pOverlays->Get_Array().size());
	string StagedPresentationId = pPresentationId->Get_String();
	std::set<string> OverlayIds;
	std::set<uint32_t> SourceOrders;
	f32_t fMaximumEndSeconds = 0.f;
	for (const DATA_JSON_VALUE& Row : pOverlays->Get_Array())
	{
		if (!HasOnlyFields(Row,
				{ "id", "sourceOrder", "textureAssetId", "colorSpace",
				  "coverageChannel", "sampler", "timing", "transform", "tint" }))
		{
			strOutError = "Screen overlay row fields are invalid.";
			return false;
		}
		const DATA_JSON_VALUE* pId = Required(
			Row, "id", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAssetId = Required(
			Row, "textureAssetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pColorSpace = Required(
			Row, "colorSpace", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pCoverageChannel = Required(
			Row, "coverageChannel", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSampler = Required(
			Row, "sampler", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pTiming = Required(
			Row, "timing", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pTransform = Required(
			Row, "transform", DATA_JSON_TYPE::OBJECT);
		PREPARED_OVERLAY Staged;
		if (nullptr == pId || !IsStableId(pId->Get_String()) ||
			!OverlayIds.emplace(pId->Get_String()).second ||
			!ReadU32(Row, "sourceOrder", Staged.Desc.iSourceOrder) ||
			!SourceOrders.emplace(Staged.Desc.iSourceOrder).second ||
			nullptr == pAssetId || nullptr == pColorSpace ||
			nullptr == pCoverageChannel || nullptr == pSampler ||
			nullptr == pTiming || nullptr == pTransform ||
			!HasOnlyFields(*pSampler, { "filter", "address" }) ||
			!HasOnlyFields(*pTiming,
				{ "startSeconds", "lifetimeSeconds", "alphaStart", "alphaEnd" }) ||
			!HasOnlyFields(*pTransform,
				{ "position", "scale", "rotationDegrees",
				  "angularVelocityDegreesPerSecond", "uvDriftPerSecond" }) ||
			!ReadTint(Row, Staged.Desc.vTint))
		{
			strOutError = "Screen overlay row identity or typed field is invalid.";
			return false;
		}

		const string& ColorSpace = pColorSpace->Get_String();
		if (ColorSpace == "linear")
			Staged.Desc.eColorSpace =
				PRESENTATION_SCREEN_OVERLAY_COLOR_SPACE::LINEAR;
		else if (ColorSpace == "srgb")
			Staged.Desc.eColorSpace =
				PRESENTATION_SCREEN_OVERLAY_COLOR_SPACE::SRGB;
		else
		{
			strOutError = "Screen overlay color space is invalid.";
			return false;
		}
		const string& CoverageChannel = pCoverageChannel->Get_String();
		if (CoverageChannel == "r")
			Staged.Desc.eCoverageChannel =
				PRESENTATION_SCREEN_OVERLAY_CHANNEL::R;
		else if (CoverageChannel == "g")
			Staged.Desc.eCoverageChannel =
				PRESENTATION_SCREEN_OVERLAY_CHANNEL::G;
		else if (CoverageChannel == "b")
			Staged.Desc.eCoverageChannel =
				PRESENTATION_SCREEN_OVERLAY_CHANNEL::B;
		else if (CoverageChannel == "a")
			Staged.Desc.eCoverageChannel =
				PRESENTATION_SCREEN_OVERLAY_CHANNEL::A;
		else
		{
			strOutError = "Screen overlay coverage channel is invalid.";
			return false;
		}

		const DATA_JSON_VALUE* pFilter = Required(
			*pSampler, "filter", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAddress = Required(
			*pSampler, "address", DATA_JSON_TYPE::STRING);
		if (nullptr == pFilter || nullptr == pAddress)
		{
			strOutError = "Screen overlay sampler is invalid.";
			return false;
		}
		if (pFilter->Get_String() == "point")
			Staged.Desc.eFilter = PRESENTATION_SCREEN_OVERLAY_FILTER::POINT;
		else if (pFilter->Get_String() == "linear")
			Staged.Desc.eFilter = PRESENTATION_SCREEN_OVERLAY_FILTER::LINEAR;
		else
		{
			strOutError = "Screen overlay filter is invalid.";
			return false;
		}
		if (pAddress->Get_String() == "clamp")
			Staged.Desc.eAddress = PRESENTATION_SCREEN_OVERLAY_ADDRESS::CLAMP;
		else if (pAddress->Get_String() == "wrap")
			Staged.Desc.eAddress = PRESENTATION_SCREEN_OVERLAY_ADDRESS::WRAP;
		else
		{
			strOutError = "Screen overlay address mode is invalid.";
			return false;
		}

		if (!ReadFloat(*pTiming, "startSeconds", 0.f, 30.f,
				Staged.fStartSeconds) ||
			!ReadFloat(*pTiming, "lifetimeSeconds", 0.001f, 30.f,
				Staged.fLifetimeSeconds) ||
			!ReadFloat(*pTiming, "alphaStart", 0.f, 1.f,
				Staged.fAlphaStart) ||
			!ReadFloat(*pTiming, "alphaEnd", 0.f, 1.f,
				Staged.fAlphaEnd) ||
			!ReadFloat2(*pTransform, "position", -2.f, 3.f,
				Staged.Desc.vPosition) ||
			!ReadFloat2(*pTransform, "scale", 0.001f, 4.f,
				Staged.Desc.vScale) ||
			!ReadFloat(*pTransform, "rotationDegrees", -100000.f, 100000.f,
				Staged.Desc.fRotationDegrees) ||
			!ReadFloat(*pTransform, "angularVelocityDegreesPerSecond",
				-100000.f, 100000.f,
				Staged.Desc.fAngularVelocityDegreesPerSecond) ||
			!ReadFloat2(*pTransform, "uvDriftPerSecond", -1000.f, 1000.f,
				Staged.Desc.vUvDriftPerSecond))
		{
			strOutError = "Screen overlay timing or transform is invalid.";
			return false;
		}
		const f32_t fEndSeconds =
			Staged.fStartSeconds + Staged.fLifetimeSeconds;
		if (!std::isfinite(fEndSeconds) || fEndSeconds > 60.f)
		{
			strOutError = "Screen overlay lifetime end is invalid.";
			return false;
		}

		std::filesystem::path TexturePath;
		if (!IsEffectDdsAssetId(pAssetId->Get_String(), TexturePath))
		{
			strOutError = "Screen overlay texture asset is invalid or missing.";
			return false;
		}
		const DirectX::DDS_LOADER_FLAGS LoaderFlags =
			Staged.Desc.eColorSpace ==
				PRESENTATION_SCREEN_OVERLAY_COLOR_SPACE::SRGB ?
				DirectX::DDS_LOADER_FORCE_SRGB :
				DirectX::DDS_LOADER_IGNORE_SRGB;
		ComPtr<ID3D11Resource> TextureResource;
		DirectX::DDS_ALPHA_MODE eAlphaMode =
			DirectX::DDS_ALPHA_MODE_UNKNOWN;
		if (FAILED(DirectX::CreateDDSTextureFromFileEx(
				m_pDevice.Get(), TexturePath.c_str(), 0u,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
				0u, 0u, LoaderFlags, &TextureResource,
				&Staged.Desc.pTexture, &eAlphaMode)) ||
			nullptr == TextureResource || nullptr == Staged.Desc.pTexture)
		{
			strOutError = "Screen overlay DDS staging failed.";
			return false;
		}
		D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc{};
		Staged.Desc.pTexture->GetDesc(&SrvDesc);
		const bool_t bSrgb = IsSrgbFormat(SrvDesc.Format);
		if (D3D11_SRV_DIMENSION_TEXTURE2D != SrvDesc.ViewDimension ||
			bSrgb != (Staged.Desc.eColorSpace ==
				PRESENTATION_SCREEN_OVERLAY_COLOR_SPACE::SRGB) ||
			!HasCoverageChannel(
				SrvDesc.Format, Staged.Desc.eCoverageChannel) ||
			(Staged.Desc.eCoverageChannel ==
				PRESENTATION_SCREEN_OVERLAY_CHANNEL::A &&
			 eAlphaMode == DirectX::DDS_ALPHA_MODE_OPAQUE))
		{
			strOutError = "Screen overlay DDS channel or color-space contract is unavailable.";
			return false;
		}

		Staged.strOverlayId = pId->Get_String();
		fMaximumEndSeconds = (std::max)(fMaximumEndSeconds, fEndSeconds);
		StagedOverlays.push_back(std::move(Staged));
	}

	if (m_iCommittedGeneration ==
		(std::numeric_limits<uint64_t>::max)())
	{
		strOutError = "Screen overlay generation is exhausted.";
		return false;
	}
	m_PreparedOverlays.swap(StagedOverlays);
	m_strPresentationId.swap(StagedPresentationId);
	m_fMaximumEndSeconds = fMaximumEndSeconds;
	++m_iCommittedGeneration;
	Stop();
	return true;
}

HRESULT Client::CEffectScreenOverlayPresentation::Start()
{
	if (m_PreparedOverlays.empty())
		return S_FALSE;
	m_fElapsedSeconds = 0.f;
	m_bPlaying = true;
	return S_OK;
}

HRESULT Client::CEffectScreenOverlayPresentation::Seek(
	const f32_t fElapsedSeconds)
{
	if (!std::isfinite(fElapsedSeconds) || fElapsedSeconds < 0.f)
		return E_INVALIDARG;
	if (m_PreparedOverlays.empty())
		return S_FALSE;
	m_fElapsedSeconds = fElapsedSeconds;
	m_bPlaying = fElapsedSeconds < m_fMaximumEndSeconds;
	return m_bPlaying ? S_OK : S_FALSE;
}

HRESULT Client::CEffectScreenOverlayPresentation::Update(
	const f32_t fDeltaSeconds)
{
	if (!m_bPlaying)
		return S_FALSE;
	if (!std::isfinite(fDeltaSeconds) || fDeltaSeconds < 0.f)
		return E_INVALIDARG;
	const f32_t fNext = m_fElapsedSeconds + fDeltaSeconds;
	if (!std::isfinite(fNext))
		return E_INVALIDARG;
	if (fNext >= m_fMaximumEndSeconds)
	{
		Stop();
		return S_FALSE;
	}
	m_fElapsedSeconds = fNext;
	return S_OK;
}

void Client::CEffectScreenOverlayPresentation::Stop()
{
	m_bPlaying = false;
	m_fElapsedSeconds = 0.f;
}

void Client::CEffectScreenOverlayPresentation::Cancel()
{
	Stop();
}

size_t Client::CEffectScreenOverlayPresentation::Get_ActiveOverlayCount() const
{
	if (!m_bPlaying)
		return 0u;
	return static_cast<size_t>(std::count_if(
		m_PreparedOverlays.begin(), m_PreparedOverlays.end(),
		[this](const PREPARED_OVERLAY& Overlay)
		{
			const f32_t fEnd =
				Overlay.fStartSeconds + Overlay.fLifetimeSeconds;
			return m_fElapsedSeconds >= Overlay.fStartSeconds &&
				m_fElapsedSeconds < fEnd;
		}));
}

HRESULT Client::CEffectScreenOverlayPresentation::Queue_Frame()
{
	if (!m_bPlaying)
		return S_FALSE;
	const shared_ptr<CEffectScreenOverlayPresentation> Self =
		weak_from_this().lock();
	return nullptr == Self ? E_FAIL :
		CPresentation_Manager::Get().Add_FrameProvider(Self);
}

void Client::CEffectScreenOverlayPresentation::
	Begin_PresentationSubmission()
{
	m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::NONE;
}

HRESULT Client::CEffectScreenOverlayPresentation::Submit_Presentation()
{
	CPresentation_Manager& Presentation = CPresentation_Manager::Get();
	const size_t iExpectedCount = Get_ActiveOverlayCount();
	Presentation.Register_ProviderScreenOverlayExpectation(
		m_PreparedOverlays.size(), iExpectedCount);

	bool_t bSuppressed = false;
	for (const PREPARED_OVERLAY& Overlay : m_PreparedOverlays)
	{
		const f32_t fEnd =
			Overlay.fStartSeconds + Overlay.fLifetimeSeconds;
		if (!m_bPlaying || m_fElapsedSeconds < Overlay.fStartSeconds ||
			m_fElapsedSeconds >= fEnd)
		{
			continue;
		}
		PRESENTATION_SCREEN_OVERLAY_DESC Desc = Overlay.Desc;
		Desc.fSampleTimeSeconds =
			m_fElapsedSeconds - Overlay.fStartSeconds;
		const f32_t fNormalizedLife = std::clamp(
			Desc.fSampleTimeSeconds / Overlay.fLifetimeSeconds, 0.f, 1.f);
		Desc.fAlpha = Overlay.fAlphaStart +
			(Overlay.fAlphaEnd - Overlay.fAlphaStart) * fNormalizedLife;
		const HRESULT hResult = Presentation.Add_ScreenOverlay(Desc);
		if (FAILED(hResult))
		{
			m_eLastFailureScope = Presentation.Get_LastFailureScope();
			return hResult;
		}
		bSuppressed = bSuppressed || S_FALSE == hResult;
	}
	return bSuppressed ? S_FALSE : S_OK;
}
