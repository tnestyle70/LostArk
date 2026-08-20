#include "Effect_LightPresentation.h"
#include "Light.h"
#include "Light_Manager.h"
#include "MapLightDocument.h"
#include "Presentation_Manager.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <vector>

using namespace Client;
using namespace Engine;

static_assert(sizeof(LIGHT_DESC) == 92u);
static_assert(offsetof(LIGHT_DESC, fRange) == 36u);
static_assert(offsetof(LIGHT_DESC, fFalloffExponent) == 40u);
static_assert(offsetof(LIGHT_DESC, vDiffuse) == 44u);

namespace
{
	constexpr f32_t ARTIST_POINT_LIGHT_SOURCE_RADIUS_UE = 200.f;
	constexpr f32_t ARTIST_POINT_LIGHT_UE_SCALE = 0.01f;
	constexpr f32_t ARTIST_POINT_LIGHT_RANGE =
		ARTIST_POINT_LIGHT_SOURCE_RADIUS_UE * ARTIST_POINT_LIGHT_UE_SCALE;
	constexpr f32_t ARTIST_POINT_LIGHT_INTENSITY = 10.f;
	constexpr f32_t ARTIST_POINT_LIGHT_POSITION_X = 1.25f;
	constexpr f32_t ARTIST_POINT_LIGHT_POSITION_Y = -2.5f;
	constexpr f32_t ARTIST_POINT_LIGHT_POSITION_Z = 3.75f;

	LIGHT_DESC MakeValidPointLight()
	{
		LIGHT_DESC Desc{};
		Desc.eType = LIGHT::POINT;
		Desc.vPosition = float4_t(1.f, 2.f, 3.f, 1.f);
		Desc.fRange = 10.f;
		Desc.vDiffuse = float4_t(1.f, 0.5f, 0.25f, 1.f);
		Desc.vAmbient = float4_t(0.1f, 0.1f, 0.1f, 1.f);
		Desc.vSpecular = float4_t(1.f, 1.f, 1.f, 1.f);
		return Desc;
	}

	EFFECT_EVALUATED_LIGHT MakeArtistEvaluatedPointLight(
		const float fFalloffExponent)
	{
		EFFECT_EVALUATED_LIGHT Evaluated{};
		Evaluated.vWorldPosition = {
			ARTIST_POINT_LIGHT_POSITION_X,
			ARTIST_POINT_LIGHT_POSITION_Y,
			ARTIST_POINT_LIGHT_POSITION_Z };
		Evaluated.fRange = ARTIST_POINT_LIGHT_RANGE;
		Evaluated.fIntensity = ARTIST_POINT_LIGHT_INTENSITY;
		Evaluated.vColor = { 1.f, 1.f, 1.f, 0.f };
		Evaluated.vAmbient = { 0.f, 0.f, 0.f, 0.f };
		Evaluated.fFalloffExponent = fFalloffExponent;
		Evaluated.fNormalizedLife = 0.25f;
		return Evaluated;
	}

	bool_t SameBits(const float Left, const float Right)
	{
		return std::bit_cast<std::uint32_t>(Left) ==
			std::bit_cast<std::uint32_t>(Right);
	}

	bool_t MatchesArtistPointLightDesc(
		const LIGHT_DESC& Desc,
		const f32_t fExpectedFalloffExponent)
	{
		return LIGHT::POINT == Desc.eType &&
			SameBits(Desc.vPosition.x, ARTIST_POINT_LIGHT_POSITION_X) &&
			SameBits(Desc.vPosition.y, ARTIST_POINT_LIGHT_POSITION_Y) &&
			SameBits(Desc.vPosition.z, ARTIST_POINT_LIGHT_POSITION_Z) &&
			SameBits(Desc.vPosition.w, 1.f) &&
			SameBits(Desc.fRange, 2.f) &&
			SameBits(Desc.fFalloffExponent, fExpectedFalloffExponent) &&
			SameBits(Desc.vDiffuse.x, 10.f) &&
			SameBits(Desc.vDiffuse.y, 10.f) &&
			SameBits(Desc.vDiffuse.z, 10.f) &&
			SameBits(Desc.vDiffuse.w, 0.f) &&
			SameBits(Desc.vAmbient.x, 0.f) &&
			SameBits(Desc.vAmbient.y, 0.f) &&
			SameBits(Desc.vAmbient.z, 0.f) &&
			SameBits(Desc.vAmbient.w, 0.f) &&
			SameBits(Desc.vSpecular.x, 0.f) &&
			SameBits(Desc.vSpecular.y, 0.f) &&
			SameBits(Desc.vSpecular.z, 0.f) &&
			SameBits(Desc.vSpecular.w, 0.f);
	}

	bool_t PreservesOutputOnFailure(
		const EFFECT_EVALUATED_LIGHT& Invalid)
	{
		LIGHT_DESC Output = MakeValidPointLight();
		Output.fRange = 123.f;
		Output.fFalloffExponent = 3.f;
		const LIGHT_DESC Before = Output;
		return !Try_BuildEffectPointLightDesc(Invalid, Output) &&
			0 == std::memcmp(&Before, &Output, sizeof Output);
	}

	bool_t RenderWithDeferredShader(
		const std::filesystem::path& RepoRoot,
		const LIGHT_DESC& Reconstructed,
		const LIGHT_DESC& Legacy)
	{
		ComPtr<ID3D11Device> Device;
		ComPtr<ID3D11DeviceContext> Context;
		D3D_FEATURE_LEVEL FeatureLevel{};
		constexpr D3D_FEATURE_LEVEL Requested[] = {
			D3D_FEATURE_LEVEL_11_0 };
		if (FAILED(D3D11CreateDevice(
			nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0u,
			Requested, 1u, D3D11_SDK_VERSION,
			Device.GetAddressOf(), &FeatureLevel, Context.GetAddressOf())) ||
			D3D_FEATURE_LEVEL_11_0 != FeatureLevel)
		{
			return false;
		}

		const std::filesystem::path ShaderPath = RepoRoot /
			L"Engine" / L"Bin" / L"ShaderFiles" / L"Shader_Deferred.hlsl";
		auto ShaderPrototype = CShader::Create(
			Device, Context, ShaderPath.c_str(),
			VTXTEX::Elements, VTXTEX::iNumElements);
		auto BufferPrototype = CVIBuffer_Rect::Create(Device, Context);
		if (nullptr == ShaderPrototype || nullptr == BufferPrototype)
			return false;

		std::shared_ptr<CShader> Shader(std::move(ShaderPrototype));
		std::shared_ptr<CVIBuffer_Rect> Buffer(std::move(BufferPrototype));
		const bool_t bRendered = SUCCEEDED(CLight::Render_Desc(
			Reconstructed, Shader, Buffer, true)) &&
			SUCCEEDED(CLight::Render_Desc(Legacy, Shader, Buffer));
		Context->ClearState();
		return bRendered;
	}

	int Fail(const char* Message)
	{
		std::cerr << "PointLightFalloffContractHarness FAIL: " << Message << '\n';
		return 1;
	}

	bool_t ReadText(const std::filesystem::path& Path, std::string& OutText)
	{
		std::ifstream Input(Path, std::ios::binary);
		if (!Input.is_open())
			return false;
		OutText.assign(
			std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>());
		return Input.good() || Input.eof();
	}

	bool_t WriteText(const std::filesystem::path& Path, const std::string& Text)
	{
		std::ofstream Output(Path, std::ios::binary | std::ios::trunc);
		if (!Output.is_open())
			return false;
		Output.write(Text.data(), static_cast<std::streamsize>(Text.size()));
		return Output.good();
	}
}

int wmain(const int iArgumentCount, wchar_t* pArguments[])
{
	if (2 != iArgumentCount)
		return Fail("expected the repository root argument");
	const std::filesystem::path RepoRoot(pArguments[1]);
	const std::filesystem::path MapLightsPath = RepoRoot / L"Data" / L"Maps" /
		L"Authoring" / L"LV_LUT_HEARTRB_ED" /
		L"LV_LUT_HEARTRB_ED.maplights.json";
	CMapLightDocument MapLights;
	std::string MapLightStatus;
	if (!MapLights.Load(
		MapLightsPath, "LV_LUT_HEARTRB_ED", MapLightStatus) ||
		MapLights.Get_Provenance() !=
			"SOURCE_INSTANCE_EXACT_FALLOFF_INFERRED" ||
		22u != MapLights.Get_Lights().size())
	{
		return Fail("Valtan project-authored map light document did not load");
	}
	const MAP_POINT_LIGHT_RECORD& FirstTowerLight =
		MapLights.Get_Lights().front();
	if (FirstTowerLight.lightId != "light.valtan.tower.pointlight_102_lc" ||
		!SameBits(FirstTowerLight.position.x, 203.460606f) ||
		!SameBits(FirstTowerLight.position.y, 24.734033f) ||
		!SameBits(FirstTowerLight.position.z, -127.571465f) ||
		!SameBits(FirstTowerLight.radiusMeters, 9.f) ||
		!SameBits(FirstTowerLight.falloffExponent, 2.f) ||
		!SameBits(FirstTowerLight.brightness, 6.f))
	{
		return Fail("Valtan tower light tuple changed during strict load");
	}
	const MAP_POINT_LIGHT_RECORD* pLowerTowerLight = nullptr;
	for (const MAP_POINT_LIGHT_RECORD& Light : MapLights.Get_Lights())
	{
		if (Light.sourceObjectId ==
			"SL04:export:780:pointlight_98_lc")
		{
			pLowerTowerLight = &Light;
			break;
		}
	}
	if (nullptr == pLowerTowerLight ||
		pLowerTowerLight->lightId !=
			"light.valtan.tower.pointlight_98_lc" ||
		pLowerTowerLight->sourceLevel != "LV_LUT_HEARTRB_ED_SL04" ||
		!SameBits(pLowerTowerLight->position.x, 187.936191f) ||
		!SameBits(pLowerTowerLight->position.y, -35.772830f) ||
		!SameBits(pLowerTowerLight->position.z, -122.317480f) ||
		!SameBits(pLowerTowerLight->radiusMeters, 20.48f) ||
		!SameBits(pLowerTowerLight->falloffExponent, 2.f) ||
		!SameBits(pLowerTowerLight->brightness, 3.f))
	{
		return Fail("Valtan lower tower light tuple changed during strict load");
	}

	std::string MapLightText;
	if (!ReadText(MapLightsPath, MapLightText))
		return Fail("could not read Valtan map light fixture");
	const std::filesystem::path InvalidMapLightsPath =
		std::filesystem::temp_directory_path() /
		(L"lostark-point-light-invalid-" +
			std::to_wstring(GetCurrentProcessId()) + L".maplights.json");
	std::string InvalidMapLightText = MapLightText;
	const std::string VersionNeedle = "\"formatVersion\": 1";
	const size_t VersionOffset = InvalidMapLightText.find(VersionNeedle);
	if (std::string::npos == VersionOffset)
		return Fail("map light version fixture anchor is missing");
	InvalidMapLightText.replace(
		VersionOffset, VersionNeedle.size(), "\"formatVersion\": 2");
	if (!WriteText(InvalidMapLightsPath, InvalidMapLightText) ||
		MapLights.Load(
			InvalidMapLightsPath, "LV_LUT_HEARTRB_ED", MapLightStatus) ||
		22u != MapLights.Get_Lights().size() ||
		MapLights.Get_AreaId() != "LV_LUT_HEARTRB_ED")
	{
		std::filesystem::remove(InvalidMapLightsPath);
		return Fail("invalid map light version changed committed document");
	}
	InvalidMapLightText = MapLightText;
	const std::string DuplicateNeedle =
		"light.valtan.tower.pointlight_104_lc";
	const size_t DuplicateOffset = InvalidMapLightText.find(DuplicateNeedle);
	if (std::string::npos == DuplicateOffset)
		return Fail("map light duplicate fixture anchor is missing");
	InvalidMapLightText.replace(
		DuplicateOffset,
		DuplicateNeedle.size(),
		"light.valtan.tower.pointlight_102_lc");
	if (!WriteText(InvalidMapLightsPath, InvalidMapLightText) ||
		MapLights.Load(
			InvalidMapLightsPath, "LV_LUT_HEARTRB_ED", MapLightStatus) ||
		22u != MapLights.Get_Lights().size())
	{
		std::filesystem::remove(InvalidMapLightsPath);
		return Fail("duplicate map light changed committed document");
	}
	std::filesystem::remove(InvalidMapLightsPath);

	const LIGHT_DESC DefaultDesc{};
	if (!SameBits(DefaultDesc.fFalloffExponent, 1.f))
		return Fail("LIGHT_DESC default exponent is not bit-exact 1.0f");

	const LIGHT_DESC Valid = MakeValidPointLight();
	auto Light = CLight::Create(Valid);
	if (nullptr == Light)
		return Fail("valid point light did not initialize");

	for (const float InvalidRange : {
		0.f, -1.f, std::numeric_limits<float>::quiet_NaN(),
		std::numeric_limits<float>::infinity(),
		-std::numeric_limits<float>::infinity() })
	{
		LIGHT_DESC Invalid = Valid;
		Invalid.fRange = InvalidRange;
		if (nullptr != CLight::Create(Invalid))
			return Fail("CLight::Create accepted invalid point range");
		if (E_INVALIDARG != Light->Initialize(Invalid))
			return Fail("CLight::Initialize accepted invalid point range");
		if (E_INVALIDARG != CLight::Render_Desc(Invalid, nullptr, nullptr))
			return Fail("CLight::Render_Desc did not reject before shader bind");
	}

	for (const float InvalidValue : {
		0.f, -1.f, std::numeric_limits<float>::quiet_NaN(),
		std::numeric_limits<float>::infinity(),
		-std::numeric_limits<float>::infinity() })
	{
		LIGHT_DESC InvalidExponent = Valid;
		InvalidExponent.fFalloffExponent = InvalidValue;
		if (nullptr != CLight::Create(InvalidExponent) ||
			E_INVALIDARG != Light->Initialize(InvalidExponent) ||
			E_INVALIDARG != CLight::Render_Desc(
				InvalidExponent, nullptr, nullptr))
		{
			return Fail("final CLight boundary accepted invalid exponent");
		}
	}

	const EFFECT_EVALUATED_LIGHT EvaluatedTwo =
		MakeArtistEvaluatedPointLight(2.f);
	// This fixture starts at the consumer boundary. The CPU executor must already
	// have applied EMITTER_TRANSFORM_PLUS_PARTICLE_LOCATION and resolved
	// castCompositeShadow/affectCompositeShadowDirection before producing it.
	LIGHT_DESC MappedTwo{};
	if (!Try_BuildEffectPointLightDesc(EvaluatedTwo, MappedTwo) ||
		!MatchesArtistPointLightDesc(MappedTwo, 2.f))
	{
		return Fail("Artist exact evaluated tuple did not map bit-exact");
	}

	CPresentation_Manager& Presentation = CPresentation_Manager::Get();
	Presentation.Set_TransientLightsEnabled(true);
	Presentation.Clear_TransientLights();
	for (const MAP_POINT_LIGHT_RECORD& Record : MapLights.Get_Lights())
	{
		EFFECT_EVALUATED_LIGHT Evaluated{};
		Evaluated.vWorldPosition = Record.position;
		Evaluated.fRange = Record.radiusMeters;
		Evaluated.fIntensity = Record.brightness;
		Evaluated.vColor = Record.color;
		Evaluated.vAmbient = { 0.f, 0.f, 0.f, 0.f };
		Evaluated.fFalloffExponent = Record.falloffExponent;
		LIGHT_DESC Desc{};
		if (!Try_BuildEffectPointLightDesc(Evaluated, Desc) ||
			FAILED(Presentation.Add_TransientLight(Desc)))
		{
			return Fail("Valtan map light did not reach transient presentation");
		}
	}
	if (22u != Presentation.Get_TransientLights().size())
		return Fail("Valtan map light transient count changed");
	Presentation.Clear_TransientLights();
	if (FAILED(Presentation.Add_TransientLight(MappedTwo)) ||
		1u != Presentation.Get_TransientLights().size() ||
		!MatchesArtistPointLightDesc(
			Presentation.Get_TransientLights().front(), 2.f))
	{
		return Fail("Presentation Manager changed the Artist exact tuple");
	}
	const LIGHT_DESC StoredTwo = Presentation.Get_TransientLights().front();
	if (nullptr == CLight::Create(StoredTwo))
		return Fail("CLight rejected mapped exponent 2.0");

	const EFFECT_EVALUATED_LIGHT EvaluatedLegacy =
		MakeArtistEvaluatedPointLight(1.f);
	LIGHT_DESC MappedLegacy{};
	Presentation.Clear_TransientLights();
	if (!Try_BuildEffectPointLightDesc(EvaluatedLegacy, MappedLegacy) ||
		!MatchesArtistPointLightDesc(MappedLegacy, 1.f) ||
		FAILED(Presentation.Add_TransientLight(MappedLegacy)) ||
		1u != Presentation.Get_TransientLights().size() ||
		!MatchesArtistPointLightDesc(
			Presentation.Get_TransientLights().front(), 1.f))
	{
		return Fail("legacy exponent 1.0 was not preserved bit-exact");
	}
	const LIGHT_DESC StoredLegacy = Presentation.Get_TransientLights().front();
	if (nullptr == CLight::Create(StoredLegacy))
		return Fail("CLight rejected mapped legacy exponent 1.0");

	if (!RenderWithDeferredShader(RepoRoot, StoredTwo, StoredLegacy))
		return Fail("mapped 2.0/1.0 descriptors did not bind and render");

	for (const float InvalidValue : {
		0.f, -1.f, std::numeric_limits<float>::quiet_NaN(),
		std::numeric_limits<float>::infinity(),
		-std::numeric_limits<float>::infinity() })
	{
		EFFECT_EVALUATED_LIGHT Invalid = EvaluatedTwo;
		Invalid.fFalloffExponent = InvalidValue;
		if (!PreservesOutputOnFailure(Invalid))
			return Fail("invalid evaluated exponent changed staged output");

		Invalid = EvaluatedTwo;
		Invalid.fRange = InvalidValue;
		if (!PreservesOutputOnFailure(Invalid))
			return Fail("invalid evaluated range changed staged output");
	}

	EFFECT_EVALUATED_LIGHT InvalidEvaluated = EvaluatedTwo;
	InvalidEvaluated.vWorldPosition.x =
		std::numeric_limits<float>::quiet_NaN();
	if (!PreservesOutputOnFailure(InvalidEvaluated))
		return Fail("nonfinite evaluated position changed staged output");
	InvalidEvaluated = EvaluatedTwo;
	InvalidEvaluated.fIntensity = -1.f;
	if (!PreservesOutputOnFailure(InvalidEvaluated))
		return Fail("negative evaluated intensity changed staged output");
	InvalidEvaluated = EvaluatedTwo;
	InvalidEvaluated.fIntensity =
		std::numeric_limits<float>::infinity();
	if (!PreservesOutputOnFailure(InvalidEvaluated))
		return Fail("nonfinite evaluated intensity changed staged output");
	InvalidEvaluated = EvaluatedTwo;
	InvalidEvaluated.vColor.x =
		std::numeric_limits<float>::quiet_NaN();
	if (!PreservesOutputOnFailure(InvalidEvaluated))
		return Fail("nonfinite evaluated color changed staged output");
	InvalidEvaluated = EvaluatedTwo;
	InvalidEvaluated.vAmbient.x =
		std::numeric_limits<float>::infinity();
	if (!PreservesOutputOnFailure(InvalidEvaluated))
		return Fail("nonfinite evaluated ambient changed staged output");
	InvalidEvaluated = EvaluatedTwo;
	InvalidEvaluated.fIntensity = (std::numeric_limits<float>::max)();
	InvalidEvaluated.vColor.x = (std::numeric_limits<float>::max)();
	if (!PreservesOutputOnFailure(InvalidEvaluated))
		return Fail("overflowed evaluated color changed staged output");

	auto LightManager = CLight_Manager::Create();
	if (nullptr == LightManager || FAILED(LightManager->Replace_SceneLights(
		std::vector<LIGHT_DESC>{ Valid, Valid })) ||
		2u != LightManager->Get_SceneLightCount())
	{
		return Fail("valid scene-light baseline was not staged");
	}

	for (const float InvalidRange : { 0.f, -1.f })
	{
		LIGHT_DESC Invalid = Valid;
		Invalid.fRange = InvalidRange;
		if (E_INVALIDARG != LightManager->Replace_SceneLights(
			std::vector<LIGHT_DESC>{ Invalid }) ||
			2u != LightManager->Get_SceneLightCount())
		{
			return Fail("scene invalid range changed prior state");
		}
	}

	Presentation.Clear_TransientLights();
	if (FAILED(Presentation.Add_TransientLight(Valid)) ||
		1u != Presentation.Get_TransientLights().size())
	{
		return Fail("valid transient-light baseline was not pushed");
	}

	for (const float InvalidRange : { 0.f, -1.f })
	{
		LIGHT_DESC Invalid = Valid;
		Invalid.fRange = InvalidRange;
		if (E_FAIL != Presentation.Add_TransientLight(Invalid) ||
			1u != Presentation.Get_TransientLights().size() ||
			!SameBits(Presentation.Get_TransientLights().front().fRange,
				Valid.fRange))
		{
			return Fail("transient invalid range pushed or changed prior state");
		}
	}

	LIGHT_DESC InvalidExponent = Valid;
	InvalidExponent.fFalloffExponent = 0.f;
	if (E_FAIL != Presentation.Add_TransientLight(InvalidExponent) ||
		1u != Presentation.Get_TransientLights().size())
	{
		return Fail("transient invalid exponent pushed or changed prior state");
	}

	Presentation.Clear_TransientLights();
	std::cout <<
		"PointLightFalloffContractHarness PASS: artist-exact-tuple-"
		"desc-transient-shader-point-shadow0; legacy1; "
		"invalid-evaluated-rollback; abi/default; "
		"final-boundary; scene-rollback; transient-no-push; "
		"valtan-map-light-strict-load-rollback-twenty-two-submit\n";
	return 0;
}
