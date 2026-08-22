#include "EffectV2_Document.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iterator>

namespace
{
	const char* EFFECT_TYPE_KEYS[] = { "Mesh", "Texture", "Particle", "Decal", "Trail" };
	const char* BLEND_KEYS[] = { "Alpha", "Additive", "Opaque" };
	const char* CLIP_CHANNEL_KEYS[] = { "RGB", "Alpha" };
	const char* PIVOT_ROTATION_KEYS[] = { "Bone", "TargetYaw", "World" };
	const char* SLOT_KEYS[] = { "mesh", "base", "noise", "mask", "emissive", "dissolve" };

	std::string Json_String(const std::string& strValue)
	{
		return "\"" + Client::CDataJson::Escape(strValue) + "\"";
	}

	std::string Json_Number(const f32_t fValue)
	{
		char szBuffer[48]{};
		std::snprintf(szBuffer, sizeof(szBuffer), "%.7g",
			std::isfinite(fValue) ? static_cast<double>(fValue) : 0.0);
		std::string strText = szBuffer;
		if (std::string::npos == strText.find_first_of(".eE"))
			strText += ".0";
		return strText;
	}

	std::string Json_Float2(const float2_t& vValue)
	{
		return "[" + Json_Number(vValue.x) + ", " + Json_Number(vValue.y) + "]";
	}

	std::string Json_Float3(const float3_t& vValue)
	{
		return "[" + Json_Number(vValue.x) + ", " + Json_Number(vValue.y) + ", " +
			Json_Number(vValue.z) + "]";
	}

	std::string Json_Float4(const float4_t& vValue)
	{
		return "[" + Json_Number(vValue.x) + ", " + Json_Number(vValue.y) + ", " +
			Json_Number(vValue.z) + ", " + Json_Number(vValue.w) + "]";
	}

	std::string Json_Lerp(const Client::CEffectV2Object::LERP_FLOAT3& Track)
	{
		return "{ \"start\": " + Json_Float3(Track.vStart) +
			", \"end\": " + Json_Float3(Track.vEnd) +
			", \"lerp\": " + (Track.bLerp ? "true" : "false") + " }";
	}

	const char* Json_Bool(const bool_t bValue)
	{
		return bValue ? "true" : "false";
	}

	bool_t Read_Number(const Client::DATA_JSON_VALUE& Object,
		const char* pKey, f32_t& fOut, std::string& strError)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Number() || !std::isfinite(pValue->Get_Number()))
		{
			strError = std::string("params.") + pKey + " must be a finite number.";
			return false;
		}
		fOut = static_cast<f32_t>(pValue->Get_Number());
		return true;
	}

	bool_t Read_Bool(const Client::DATA_JSON_VALUE& Object,
		const char* pKey, bool_t& bOut, std::string& strError)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Boolean())
		{
			strError = std::string(pKey) + " must be a boolean.";
			return false;
		}
		bOut = pValue->Get_Boolean();
		return true;
	}

	bool_t Read_FloatArray(const Client::DATA_JSON_VALUE& Object,
		const char* pKey, f32_t* pOut, const size_t iCount, std::string& strError)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Array() || pValue->Get_Array().size() != iCount)
		{
			strError = std::string("params.") + pKey + " must be an array of " +
				std::to_string(iCount) + " numbers.";
			return false;
		}
		for (size_t iIndex = 0u; iIndex < iCount; ++iIndex)
		{
			const Client::DATA_JSON_VALUE& Element = pValue->Get_Array()[iIndex];
			if (!Element.Is_Number() || !std::isfinite(Element.Get_Number()))
			{
				strError = std::string("params.") + pKey + " contains a non-finite value.";
				return false;
			}
			pOut[iIndex] = static_cast<f32_t>(Element.Get_Number());
		}
		return true;
	}

	bool_t Read_Lerp(const Client::DATA_JSON_VALUE& Object,
		const char* pKey, Client::CEffectV2Object::LERP_FLOAT3& Track, std::string& strError)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Object())
		{
			strError = std::string("params.") + pKey + " must be an object.";
			return false;
		}
		return Read_FloatArray(*pValue, "start", &Track.vStart.x, 3u, strError) &&
			Read_FloatArray(*pValue, "end", &Track.vEnd.x, 3u, strError) &&
			Read_Bool(*pValue, "lerp", Track.bLerp, strError);
	}

	bool_t Read_Enum(const Client::DATA_JSON_VALUE& Object, const char* pKey,
		const char* const* pKeys, const size_t iKeyCount, int32_t& iOut, std::string& strError)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		if (nullptr == pValue)
			return true;
		if (pValue->Is_String())
		{
			for (size_t iIndex = 0u; iIndex < iKeyCount; ++iIndex)
			{
				if (pValue->Get_String() == pKeys[iIndex])
				{
					iOut = static_cast<int32_t>(iIndex);
					return true;
				}
			}
		}
		strError = std::string(pKey) + " has an unknown value.";
		return false;
	}

	bool_t Asset_Exists(const std::string& strAssetId)
	{
		const std::filesystem::path Resolved =
			Client::CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
		return !Resolved.empty() && std::filesystem::is_regular_file(Resolved);
	}

	bool_t Read_TextFile(const std::filesystem::path& Path, std::string& OutText)
	{
		std::ifstream Stream(Path, std::ios::binary);
		if (!Stream.is_open())
			return false;
		OutText.assign((std::istreambuf_iterator<char>(Stream)),
			std::istreambuf_iterator<char>());
		return true;
	}
}

std::filesystem::path Client::CEffectV2Document::Document_Directory()
{
	return CProjectDataRoot::Resolve(L"Effects/V2/Authored");
}

std::filesystem::path Client::CEffectV2Document::Binding_Directory()
{
	return CProjectDataRoot::Resolve(L"Effects/V2/Bindings");
}

std::filesystem::path Client::CEffectV2Document::Document_Path(const std::string& strEffectId)
{
	return Document_Directory() / (strEffectId + ".effectv2.json");
}

std::filesystem::path Client::CEffectV2Document::Binding_Path(const std::string& strArchetypeId)
{
	return Binding_Directory() / (strArchetypeId + ".effectv2bindings.json");
}

bool_t Client::CEffectV2Document::Is_ValidEffectId(const std::string& strEffectId)
{
	if (strEffectId.empty() || strEffectId.size() > 80u)
		return false;
	for (const char Character : strEffectId)
	{
		if (!std::isalnum(static_cast<unsigned char>(Character)) &&
			'.' != Character && '_' != Character && '-' != Character)
			return false;
	}
	return true;
}

const char* Client::CEffectV2Document::Type_Key(const EFFECT_V2_TYPE eType)
{
	const size_t iIndex = static_cast<size_t>(eType);
	return iIndex < _countof(EFFECT_TYPE_KEYS) ? EFFECT_TYPE_KEYS[iIndex] : "Mesh";
}

const char* Client::CEffectV2Document::Rotation_Key(const CEffectV2Object::PIVOT_ROTATION eRotation)
{
	const size_t iIndex = static_cast<size_t>(eRotation);
	return iIndex < _countof(PIVOT_ROTATION_KEYS) ? PIVOT_ROTATION_KEYS[iIndex] : "TargetYaw";
}

bool_t Client::CEffectV2Document::Parse_Document(
	const std::string& strText,
	EFFECT_V2_DOCUMENT& OutDocument,
	std::string& strOutError)
{
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(strText, Root, strOutError) || !Root.Is_Object())
	{
		if (strOutError.empty())
			strOutError = "Document root is not an object.";
		return false;
	}
	const DATA_JSON_VALUE* pSchema = Root.Find("schema");
	const DATA_JSON_VALUE* pVersion = Root.Find("formatVersion");
	const DATA_JSON_VALUE* pEffectId = Root.Find("effectId");
	if (nullptr == pSchema || !pSchema->Is_String() || pSchema->Get_String() != "lostark.effect-v2" ||
		nullptr == pVersion || !pVersion->Is_Number() || pVersion->Get_Number() != 1.0)
	{
		strOutError = "schema must be lostark.effect-v2 formatVersion 1.";
		return false;
	}
	if (nullptr == pEffectId || !pEffectId->Is_String() || !Is_ValidEffectId(pEffectId->Get_String()))
	{
		strOutError = "effectId is missing or invalid.";
		return false;
	}
	if (nullptr == Root.Find("effectType"))
	{
		strOutError = "effectType is required.";
		return false;
	}
	int32_t iType = 0;
	if (!Read_Enum(Root, "effectType", EFFECT_TYPE_KEYS,
		_countof(EFFECT_TYPE_KEYS), iType, strOutError))
		return false;

	EFFECT_V2_DOCUMENT Document{};
	Document.strEffectId = pEffectId->Get_String();
	Document.eType = static_cast<EFFECT_V2_TYPE>(iType);
	Document.Desc.eShape = EFFECT_V2_TYPE::MESH == Document.eType ?
		CEffectV2Object::SHAPE::MESH : CEffectV2Object::SHAPE::SPRITE;

	const DATA_JSON_VALUE* pSlots = Root.Find("slots");
	if (nullptr == pSlots || !pSlots->Is_Object())
	{
		strOutError = "slots object is required.";
		return false;
	}
	for (size_t iKey = 0u; iKey < _countof(SLOT_KEYS); ++iKey)
	{
		const DATA_JSON_VALUE* pValue = pSlots->Find(SLOT_KEYS[iKey]);
		if (nullptr == pValue)
			continue;
		if (!pValue->Is_String())
		{
			strOutError = std::string("slots.") + SLOT_KEYS[iKey] + " must be a string.";
			return false;
		}
		const std::string& strAssetId = pValue->Get_String();
		if (!strAssetId.empty() && !Asset_Exists(strAssetId))
		{
			strOutError = std::string("slots.") + SLOT_KEYS[iKey] + " asset is missing: " + strAssetId;
			return false;
		}
		if (0u == iKey)
			Document.Desc.strMeshAssetId = strAssetId;
		else
			Document.Desc.TextureAssetIds[iKey - 1u] = strAssetId;
	}
	if (CEffectV2Object::SHAPE::MESH == Document.Desc.eShape && Document.Desc.strMeshAssetId.empty())
	{
		strOutError = "Mesh effect requires slots.mesh.";
		return false;
	}

	const DATA_JSON_VALUE* pParams = Root.Find("params");
	if (nullptr == pParams || !pParams->Is_Object())
	{
		strOutError = "params object is required.";
		return false;
	}
	CEffectV2Object::PARAMS& P = Document.Desc.Params;
	int32_t iClipChannel = static_cast<int32_t>(P.eColorClipChannel);
	int32_t iBlend = static_cast<int32_t>(P.eBlend);
	if (!Read_Lerp(*pParams, "position", P.Position, strOutError) ||
		!Read_Lerp(*pParams, "rotation", P.Rotation, strOutError) ||
		!Read_Lerp(*pParams, "scale", P.Scale, strOutError) ||
		!Read_Lerp(*pParams, "velocity", P.Velocity, strOutError) ||
		!Read_FloatArray(*pParams, "colorOffset", &P.vColorOffset.x, 4u, strOutError) ||
		!Read_FloatArray(*pParams, "colorMul", &P.vColorMul.x, 4u, strOutError) ||
		!Read_Enum(*pParams, "colorClipChannel", CLIP_CHANNEL_KEYS,
			_countof(CLIP_CHANNEL_KEYS), iClipChannel, strOutError) ||
		!Read_Number(*pParams, "colorClip", P.fColorClip, strOutError) ||
		!Read_FloatArray(*pParams, "rimColor", &P.vRimColor.x, 4u, strOutError) ||
		!Read_Number(*pParams, "rimPower", P.fRimPower, strOutError) ||
		!Read_Number(*pParams, "rimIntensity", P.fRimIntensity, strOutError) ||
		!Read_Number(*pParams, "ghostAlpha", P.fGhostAlpha, strOutError) ||
		!Read_Number(*pParams, "bloomIntensity", P.fBloomIntensity, strOutError) ||
		!Read_Number(*pParams, "distortionIntensity", P.fDistortionIntensity, strOutError) ||
		!Read_FloatArray(*pParams, "uvStart", &P.vUVStart.x, 2u, strOutError) ||
		!Read_FloatArray(*pParams, "uvSpeed", &P.vUVSpeed.x, 2u, strOutError) ||
		!Read_FloatArray(*pParams, "uvTileCount", &P.vUVTileCount.x, 2u, strOutError) ||
		!Read_Number(*pParams, "noiseStrength", P.fNoiseStrength, strOutError) ||
		!Read_Number(*pParams, "noiseScale", P.fNoiseScale, strOutError) ||
		!Read_FloatArray(*pParams, "noisePan", &P.vNoisePan.x, 2u, strOutError) ||
		!Read_Number(*pParams, "dissolveStart", P.fDissolveStart, strOutError) ||
		!Read_Number(*pParams, "dissolveSoftness", P.fDissolveSoftness, strOutError) ||
		!Read_Enum(*pParams, "blend", BLEND_KEYS, _countof(BLEND_KEYS), iBlend, strOutError) ||
		!Read_Bool(*pParams, "billboard", P.bBillboard, strOutError) ||
		!Read_Bool(*pParams, "depthTest", P.bDepthTest, strOutError) ||
		!Read_Number(*pParams, "lifetime", P.fLifetime, strOutError) ||
		!Read_Bool(*pParams, "loop", P.bLoop, strOutError) ||
		!Read_Number(*pParams, "playRate", P.fPlayRate, strOutError) ||
		!Read_Number(*pParams, "meshPreScale", P.fMeshPreScale, strOutError) ||
		!Read_Bool(*pParams, "animationLoop", P.bAnimationLoop, strOutError))
	{
		return false;
	}
	P.eColorClipChannel = static_cast<CEffectV2Object::COLOR_CLIP_CHANNEL>(iClipChannel);
	P.eBlend = static_cast<CEffectV2Object::BLEND_MODE>(iBlend);
	if (P.fMeshPreScale <= 0.f || P.fLifetime < 0.f || P.fPlayRate < 0.f)
	{
		strOutError = "params.meshPreScale/lifetime/playRate out of range.";
		return false;
	}
	if (const DATA_JSON_VALUE* pClip = pParams->Find("animationClip"))
	{
		if (!pClip->Is_String())
		{
			strOutError = "params.animationClip must be a string.";
			return false;
		}
		Document.strAnimationClip = pClip->Get_String();
	}
	Document.Desc.bParamsAuthored = true;

	if (const DATA_JSON_VALUE* pParts = Root.Find("parts"))
	{
		if (!pParts->Is_Array())
		{
			strOutError = "parts must be an array.";
			return false;
		}
		for (const DATA_JSON_VALUE& Part : pParts->Get_Array())
		{
			const DATA_JSON_VALUE* pIndex = Part.Is_Object() ? Part.Find("index") : nullptr;
			if (nullptr == pIndex || !pIndex->Is_Number() || pIndex->Get_Number() < 0.0 ||
				pIndex->Get_Number() > 255.0)
			{
				strOutError = "parts[].index must be a number in [0, 255].";
				return false;
			}
			const size_t iIndex = static_cast<size_t>(pIndex->Get_Number());
			if (Document.Parts.size() <= iIndex)
				Document.Parts.resize(iIndex + 1u);
			EFFECT_V2_PART_OVERRIDE& Override = Document.Parts[iIndex];
			if (!Read_Bool(Part, "visible", Override.bVisible, strOutError))
				return false;
			if (const DATA_JSON_VALUE* pBase = Part.Find("base"))
			{
				if (!pBase->Is_String())
				{
					strOutError = "parts[].base must be a string.";
					return false;
				}
				Override.strBaseAssetId = pBase->Get_String();
				if (!Override.strBaseAssetId.empty() && !Asset_Exists(Override.strBaseAssetId))
				{
					strOutError = "parts[].base asset is missing: " + Override.strBaseAssetId;
					return false;
				}
			}
		}
	}
	OutDocument = std::move(Document);
	return true;
}

bool_t Client::CEffectV2Document::Parse_Bindings(
	const std::string& strText,
	const std::string& strExpectedArchetypeId,
	std::vector<EFFECT_V2_BINDING>& OutBindings,
	std::string& strOutError)
{
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(strText, Root, strOutError) || !Root.Is_Object())
	{
		if (strOutError.empty())
			strOutError = "Bindings root is not an object.";
		return false;
	}
	const DATA_JSON_VALUE* pSchema = Root.Find("schema");
	const DATA_JSON_VALUE* pVersion = Root.Find("formatVersion");
	const DATA_JSON_VALUE* pArchetype = Root.Find("archetypeId");
	if (nullptr == pSchema || !pSchema->Is_String() ||
		pSchema->Get_String() != "lostark.effect-v2-bindings" ||
		nullptr == pVersion || !pVersion->Is_Number() || pVersion->Get_Number() != 1.0 ||
		nullptr == pArchetype || !pArchetype->Is_String() ||
		pArchetype->Get_String() != strExpectedArchetypeId)
	{
		strOutError = "schema/formatVersion/archetypeId mismatch.";
		return false;
	}
	const DATA_JSON_VALUE* pRows = Root.Find("bindings");
	if (nullptr == pRows || !pRows->Is_Array())
	{
		strOutError = "bindings must be an array.";
		return false;
	}
	std::vector<EFFECT_V2_BINDING> Staged;
	for (const DATA_JSON_VALUE& Row : pRows->Get_Array())
	{
		if (!Row.Is_Object())
		{
			strOutError = "bindings[] entries must be objects.";
			return false;
		}
		EFFECT_V2_BINDING Binding;
		const DATA_JSON_VALUE* pEffect = Row.Find("effectId");
		const DATA_JSON_VALUE* pClip = Row.Find("clip");
		const DATA_JSON_VALUE* pStart = Row.Find("startMs");
		const DATA_JSON_VALUE* pBone = Row.Find("bone");
		if (nullptr == pEffect || !pEffect->Is_String() || !Is_ValidEffectId(pEffect->Get_String()) ||
			nullptr == pClip || !pClip->Is_String() || pClip->Get_String().empty() ||
			nullptr == pStart || !pStart->Is_Number() || pStart->Get_Number() < 0.0 ||
			pStart->Get_Number() > 600000.0 ||
			nullptr == pBone || !pBone->Is_String())
		{
			strOutError = "bindings[] requires effectId, clip, startMs (0-600000), bone.";
			return false;
		}
		Binding.strEffectId = pEffect->Get_String();
		Binding.strClip = pClip->Get_String();
		Binding.iStartMs = static_cast<uint32_t>(pStart->Get_Number());
		Binding.strBone = pBone->Get_String();
		int32_t iRotation = static_cast<int32_t>(Binding.eRotation);
		if (!Read_Bool(Row, "followBone", Binding.bFollowBone, strOutError) ||
			!Read_Enum(Row, "rotation", PIVOT_ROTATION_KEYS,
				_countof(PIVOT_ROTATION_KEYS), iRotation, strOutError) ||
			!Read_Bool(Row, "stopWithClip", Binding.bStopWithClip, strOutError))
			return false;
		Binding.eRotation = static_cast<CEffectV2Object::PIVOT_ROTATION>(iRotation);
		for (const EFFECT_V2_BINDING& Existing : Staged)
		{
			if (Existing.strEffectId == Binding.strEffectId && Existing.strClip == Binding.strClip)
			{
				strOutError = "duplicate binding: " + Binding.strEffectId + " / " + Binding.strClip;
				return false;
			}
		}
		Staged.push_back(std::move(Binding));
	}
	OutBindings = std::move(Staged);
	return true;
}

std::string Client::CEffectV2Document::Serialize_Document(const EFFECT_V2_DOCUMENT& Document)
{
	const CEffectV2Object::DESC& Desc = Document.Desc;
	const CEffectV2Object::PARAMS& P = Desc.Params;
	std::string Text;
	Text += "{\n";
	Text += "  \"schema\": \"lostark.effect-v2\",\n";
	Text += "  \"formatVersion\": 1,\n";
	Text += "  \"effectId\": " + Json_String(Document.strEffectId) + ",\n";
	Text += "  \"effectType\": " + Json_String(Type_Key(Document.eType)) + ",\n";
	Text += "  \"slots\": {\n";
	Text += "    \"mesh\": " + Json_String(Desc.strMeshAssetId) + ",\n";
	for (size_t iInput = 0u; iInput < Desc.TextureAssetIds.size(); ++iInput)
	{
		Text += std::string("    \"") + SLOT_KEYS[iInput + 1u] + "\": " +
			Json_String(Desc.TextureAssetIds[iInput]) +
			(iInput + 1u < Desc.TextureAssetIds.size() ? ",\n" : "\n");
	}
	Text += "  },\n";
	Text += "  \"params\": {\n";
	Text += "    \"position\": " + Json_Lerp(P.Position) + ",\n";
	Text += "    \"rotation\": " + Json_Lerp(P.Rotation) + ",\n";
	Text += "    \"scale\": " + Json_Lerp(P.Scale) + ",\n";
	Text += "    \"velocity\": " + Json_Lerp(P.Velocity) + ",\n";
	Text += "    \"colorOffset\": " + Json_Float4(P.vColorOffset) + ",\n";
	Text += "    \"colorMul\": " + Json_Float4(P.vColorMul) + ",\n";
	Text += "    \"colorClipChannel\": " + Json_String(
		CLIP_CHANNEL_KEYS[static_cast<size_t>(P.eColorClipChannel)]) + ",\n";
	Text += "    \"colorClip\": " + Json_Number(P.fColorClip) + ",\n";
	Text += "    \"rimColor\": " + Json_Float4(P.vRimColor) + ",\n";
	Text += "    \"rimPower\": " + Json_Number(P.fRimPower) + ",\n";
	Text += "    \"rimIntensity\": " + Json_Number(P.fRimIntensity) + ",\n";
	Text += "    \"ghostAlpha\": " + Json_Number(P.fGhostAlpha) + ",\n";
	Text += "    \"bloomIntensity\": " + Json_Number(P.fBloomIntensity) + ",\n";
	Text += "    \"distortionIntensity\": " + Json_Number(P.fDistortionIntensity) + ",\n";
	Text += "    \"uvStart\": " + Json_Float2(P.vUVStart) + ",\n";
	Text += "    \"uvSpeed\": " + Json_Float2(P.vUVSpeed) + ",\n";
	Text += "    \"uvTileCount\": " + Json_Float2(P.vUVTileCount) + ",\n";
	Text += "    \"noiseStrength\": " + Json_Number(P.fNoiseStrength) + ",\n";
	Text += "    \"noiseScale\": " + Json_Number(P.fNoiseScale) + ",\n";
	Text += "    \"noisePan\": " + Json_Float2(P.vNoisePan) + ",\n";
	Text += "    \"dissolveStart\": " + Json_Number(P.fDissolveStart) + ",\n";
	Text += "    \"dissolveSoftness\": " + Json_Number(P.fDissolveSoftness) + ",\n";
	Text += "    \"blend\": " + Json_String(BLEND_KEYS[static_cast<size_t>(P.eBlend)]) + ",\n";
	Text += std::string("    \"billboard\": ") + Json_Bool(P.bBillboard) + ",\n";
	Text += std::string("    \"depthTest\": ") + Json_Bool(P.bDepthTest) + ",\n";
	Text += "    \"lifetime\": " + Json_Number(P.fLifetime) + ",\n";
	Text += std::string("    \"loop\": ") + Json_Bool(P.bLoop) + ",\n";
	Text += "    \"playRate\": " + Json_Number(P.fPlayRate) + ",\n";
	Text += "    \"meshPreScale\": " + Json_Number(P.fMeshPreScale) + ",\n";
	Text += "    \"animationClip\": " + Json_String(Document.strAnimationClip) + ",\n";
	Text += std::string("    \"animationLoop\": ") + Json_Bool(P.bAnimationLoop) + "\n";
	Text += "  },\n";
	Text += "  \"parts\": [\n";
	for (size_t iPart = 0u; iPart < Document.Parts.size(); ++iPart)
	{
		const EFFECT_V2_PART_OVERRIDE& Part = Document.Parts[iPart];
		Text += "    { \"index\": " + std::to_string(iPart) +
			", \"visible\": " + Json_Bool(Part.bVisible) +
			", \"base\": " + Json_String(Part.strBaseAssetId) + " }" +
			(iPart + 1u < Document.Parts.size() ? ",\n" : "\n");
	}
	Text += "  ]\n";
	Text += "}\n";
	return Text;
}

std::string Client::CEffectV2Document::Serialize_Bindings(
	const std::string& strArchetypeId,
	const std::vector<EFFECT_V2_BINDING>& Bindings)
{
	std::string Text;
	Text += "{\n";
	Text += "  \"schema\": \"lostark.effect-v2-bindings\",\n";
	Text += "  \"formatVersion\": 1,\n";
	Text += "  \"archetypeId\": " + Json_String(strArchetypeId) + ",\n";
	Text += "  \"bindings\": [\n";
	for (size_t iIndex = 0u; iIndex < Bindings.size(); ++iIndex)
	{
		const EFFECT_V2_BINDING& Binding = Bindings[iIndex];
		Text += "    { \"effectId\": " + Json_String(Binding.strEffectId) +
			", \"clip\": " + Json_String(Binding.strClip) +
			", \"startMs\": " + std::to_string(Binding.iStartMs) +
			", \"bone\": " + Json_String(Binding.strBone) +
			", \"followBone\": " + Json_Bool(Binding.bFollowBone) +
			", \"rotation\": " + Json_String(Rotation_Key(Binding.eRotation)) +
			", \"stopWithClip\": " + Json_Bool(Binding.bStopWithClip) + " }" +
			(iIndex + 1u < Bindings.size() ? ",\n" : "\n");
	}
	Text += "  ]\n";
	Text += "}\n";
	return Text;
}

bool_t Client::CEffectV2Document::Load_DocumentFile(
	const std::string& strEffectId,
	EFFECT_V2_DOCUMENT& OutDocument,
	std::string& strOutError)
{
	if (!Is_ValidEffectId(strEffectId))
	{
		strOutError = "Invalid effect ID.";
		return false;
	}
	std::string Text;
	const std::filesystem::path Path = Document_Path(strEffectId);
	if (!Read_TextFile(Path, Text))
	{
		strOutError = "Cannot open: " + Path.string();
		return false;
	}
	if (!Parse_Document(Text, OutDocument, strOutError))
		return false;
	if (OutDocument.strEffectId != strEffectId)
	{
		strOutError = "effectId does not match the file name.";
		return false;
	}
	return true;
}

bool_t Client::CEffectV2Document::Load_BindingsFile(
	const std::string& strArchetypeId,
	std::vector<EFFECT_V2_BINDING>& OutBindings,
	std::string& strOutError)
{
	std::string Text;
	const std::filesystem::path Path = Binding_Path(strArchetypeId);
	if (!Read_TextFile(Path, Text))
	{
		strOutError = "Cannot open: " + Path.string();
		return false;
	}
	return Parse_Bindings(Text, strArchetypeId, OutBindings, strOutError);
}

bool_t Client::CEffectV2Document::Write_AtomicFile(
	const std::filesystem::path& Target,
	const std::string& strText,
	std::string& strOutError)
{
	std::error_code Error;
	if (Target.empty() || Target.parent_path().empty())
	{
		strOutError = "Target path is empty.";
		return false;
	}
	std::filesystem::create_directories(Target.parent_path(), Error);
	const std::filesystem::path Temporary = Target.string() + ".tmp";
	{
		std::ofstream Stream(Temporary, std::ios::binary | std::ios::trunc);
		if (!Stream.is_open())
		{
			strOutError = "Cannot open for write: " + Temporary.string();
			return false;
		}
		Stream << strText;
		if (!Stream.good())
		{
			Stream.close();
			std::filesystem::remove(Temporary, Error);
			strOutError = "Write failed: " + Temporary.string();
			return false;
		}
	}
	std::filesystem::rename(Temporary, Target, Error);
	if (Error)
	{
		std::filesystem::remove(Temporary, Error);
		strOutError = "Rename failed: " + Target.string();
		return false;
	}
	return true;
}
