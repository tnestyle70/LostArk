#include "EffectV2_Document.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <set>

namespace
{
	const char* EFFECT_TYPE_KEYS[] = { "Mesh", "Texture", "Particle", "Decal", "Trail", "ScreenPost" };
	const char* SCREEN_POST_PROFILE_KEYS[] = { "ZoomBlur", "RgbNoise", "FilmNoise", "ChromaticAberration" };
	const char* BLEND_KEYS[] = { "Alpha", "Additive", "Opaque", "Multiply" };
	const char* CLIP_CHANNEL_KEYS[] = { "RGB", "Alpha" };
	const char* PIVOT_ROTATION_KEYS[] = { "Bone", "TargetYaw", "World" };
	const char* SLOT_KEYS[] = { "mesh", "base", "noise", "mask", "emissive", "dissolve" };
	const char* SPAWN_SHAPE_KEYS[] = { "Point", "Sphere", "Ring", "Box" };
	const char* VELOCITY_MODE_KEYS[] = { "Fixed", "Outward", "Cone" };
	const char* ALIGNMENT_KEYS[] = { "Camera", "Velocity", "Horizontal" };
	const char* TRAIL_EDGE_KEYS[] = { "CenterlineCamera", "CenterlineUp", "LocalOffset" };
	const char* CHILD_STOP_KEYS[] = { "Kill", "Deactivate" };
	const char* RESOURCE_KIND_KEYS[] = { "LEAF", "GROUP" };
	const char* CLOCK_BASIS_KEYS[] = { "STAGE", "CLIP_OCCURRENCE" };
	const char* REPEAT_POLICY_KEYS[] = { "ONCE", "EACH_LOOP" };
	const char* FOLLOW_POLICY_KEYS[] = { "FOLLOW_SLOT", "SNAPSHOT_AT_START" };
	const char* ROTATION_BASIS_KEYS[] = { "SLOT", "TARGET_YAW", "WORLD" };
	const char* STOP_POLICY_KEYS[] = {
		"NATURAL", "STAGE_END", "CLIP_OCCURRENCE_END", "EXPLICIT" };
	constexpr double MAX_BINDING_MS = 600000.0;
	constexpr size_t MAX_BINDINGS = 4096u;
	constexpr size_t MAX_GROUP_CHILDREN = 4096u;
	constexpr size_t MAX_STABLE_ID_LENGTH = 160u;

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

	const char* Enum_Key(const char* const* const pKeys,
		const size_t iKeyCount, const int32_t iValue, const char* const pFallback)
	{
		return iValue >= 0 && static_cast<size_t>(iValue) < iKeyCount ?
			pKeys[iValue] : pFallback;
	}

	std::string Json_LocalTransform(
		const Client::EFFECT_V2_LOCAL_TRANSFORM& Transform)
	{
		return "{ \"translation\": " + Json_Float3(Transform.vTranslation) +
			", \"rotation\": " + Json_Float3(Transform.vRotation) +
			", \"scale\": " + Json_Float3(Transform.vScale) + " }";
	}

	bool_t Is_StableAsciiId(const std::string& strValue, const size_t iMaximumLength)
	{
		if (strValue.empty() || strValue.size() > iMaximumLength)
			return false;
		for (const unsigned char Character : strValue)
		{
			const bool_t bAlphaNumeric =
				(Character >= 'A' && Character <= 'Z') ||
				(Character >= 'a' && Character <= 'z') ||
				(Character >= '0' && Character <= '9');
			if (!bAlphaNumeric && '.' != Character &&
				'_' != Character && '-' != Character)
			{
				return false;
			}
		}
		return true;
	}

	bool_t Has_ExactFields(const Client::DATA_JSON_VALUE& Object,
		const std::initializer_list<const char*> Fields)
	{
		if (!Object.Is_Object() || Object.Get_Object().size() != Fields.size())
			return false;
		for (const char* const pField : Fields)
		{
			if (nullptr == Object.Find(pField))
				return false;
		}
		return true;
	}

	bool_t Read_StableId(const Client::DATA_JSON_VALUE& Object,
		const char* const pKey, std::string& strOut, std::string& strError,
		const size_t iMaximumLength = MAX_STABLE_ID_LENGTH)
	{
		const Client::DATA_JSON_VALUE* const pValue = Object.Find(pKey);
		if (nullptr == pValue || !pValue->Is_String() ||
			!Is_StableAsciiId(pValue->Get_String(), iMaximumLength))
		{
			strError = std::string(pKey) + " must be a stable ID.";
			return false;
		}
		strOut = pValue->Get_String();
		return true;
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

	bool_t Read_Uint(const Client::DATA_JSON_VALUE& Object,
		const char* pKey, uint32_t& iOut, std::string& strError)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Number() || !std::isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() < 0.0 || pValue->Get_Number() > 4294967295.0)
		{
			strError = std::string("params.") + pKey + " must be a non-negative integer.";
			return false;
		}
		iOut = static_cast<uint32_t>(pValue->Get_Number());
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

	bool_t Read_MsField(const Client::DATA_JSON_VALUE& Object,
		const char* pKey, uint32_t& iOut, std::string& strError)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		if (nullptr == pValue)
		{
			strError = std::string(pKey) + " is required.";
			return false;
		}
		if (!pValue->Is_Number() || !std::isfinite(pValue->Get_Number()) ||
			pValue->Was_FloatingPointToken() ||
			pValue->Get_Number() < 0.0 || pValue->Get_Number() > MAX_BINDING_MS)
		{
			strError = std::string(pKey) + " must be an integer in [0, 600000].";
			return false;
		}
		iOut = static_cast<uint32_t>(pValue->Get_Number());
		return true;
	}

	bool_t Read_RequiredFloat3(const Client::DATA_JSON_VALUE& Object,
		const char* const pKey, float3_t& vOut, std::string& strError,
		const bool_t bNonZero)
	{
		const Client::DATA_JSON_VALUE* const pValue = Object.Find(pKey);
		if (nullptr == pValue || !pValue->Is_Array() ||
			3u != pValue->Get_Array().size())
		{
			strError = std::string(pKey) + " must be exactly three finite numbers.";
			return false;
		}
		f32_t* const pComponents = &vOut.x;
		for (size_t iComponent = 0u; iComponent < 3u; ++iComponent)
		{
			const Client::DATA_JSON_VALUE& Component = pValue->Get_Array()[iComponent];
			if (!Component.Is_Number() || !std::isfinite(Component.Get_Number()) ||
				(bNonZero && 0.0 == Component.Get_Number()))
			{
				strError = std::string(pKey) + (bNonZero ?
					" components must be finite and non-zero." :
					" must be exactly three finite numbers.");
				return false;
			}
			pComponents[iComponent] = static_cast<f32_t>(Component.Get_Number());
		}
		return true;
	}

	bool_t Read_LocalTransform(const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_V2_LOCAL_TRANSFORM& OutTransform, std::string& strError)
	{
		if (!Has_ExactFields(Value, { "translation", "rotation", "scale" }))
		{
			strError = "localTransform fields must be exactly translation, rotation, scale.";
			return false;
		}
		return Read_RequiredFloat3(
			Value, "translation", OutTransform.vTranslation, strError, false) &&
			Read_RequiredFloat3(
				Value, "rotation", OutTransform.vRotation, strError, false) &&
			Read_RequiredFloat3(
				Value, "scale", OutTransform.vScale, strError, true);
	}

	bool_t Equal_Float3(const float3_t& Left, const float3_t& Right)
	{
		return Left.x == Right.x && Left.y == Right.y && Left.z == Right.z;
	}

	bool_t Equal_LocalTransform(
		const Client::EFFECT_V2_LOCAL_TRANSFORM& Left,
		const Client::EFFECT_V2_LOCAL_TRANSFORM& Right)
	{
		return Equal_Float3(Left.vTranslation, Right.vTranslation) &&
			Equal_Float3(Left.vRotation, Right.vRotation) &&
			Equal_Float3(Left.vScale, Right.vScale);
	}

	bool_t Equal_BindingSemantic(
		const Client::EFFECT_V2_BINDING& Left,
		const Client::EFFECT_V2_BINDING& Right)
	{
		return Left.eResourceKind == Right.eResourceKind &&
			Left.strResourceId == Right.strResourceId &&
			Left.strPatternId == Right.strPatternId &&
			Left.strStageId == Right.strStageId &&
			Left.strActionId == Right.strActionId &&
			Left.eClockBasis == Right.eClockBasis &&
			Left.strClipOccurrenceId == Right.strClipOccurrenceId &&
			Left.iStartMs == Right.iStartMs &&
			Left.eRepeatPolicy == Right.eRepeatPolicy &&
			Left.strAnchorSlotId == Right.strAnchorSlotId &&
			Left.eFollowPolicy == Right.eFollowPolicy &&
			Left.eRotationBasis == Right.eRotationBasis &&
			Equal_LocalTransform(Left.LocalTransform, Right.LocalTransform) &&
			Left.eStopPolicy == Right.eStopPolicy;
	}

	bool_t Equal_GroupChildSemantic(
		const Client::EFFECT_V2_GROUP_CHILD& Left,
		const Client::EFFECT_V2_GROUP_CHILD& Right)
	{
		return Left.eResourceKind == Right.eResourceKind &&
			Left.strResourceId == Right.strResourceId &&
			Left.iStartMs == Right.iStartMs &&
			Left.iDurationMs == Right.iDurationMs &&
			Left.eStop == Right.eStop &&
			Equal_LocalTransform(Left.LocalTransform, Right.LocalTransform);
	}

	void Populate_BindingConvenience(Client::EFFECT_V2_BINDING& Binding)
	{
		Binding.strEffectId.clear();
		Binding.strGroupId.clear();
		if (Client::EFFECT_V2_RESOURCE_KIND::LEAF == Binding.eResourceKind)
			Binding.strEffectId = Binding.strResourceId;
		else
			Binding.strGroupId = Binding.strResourceId;

		Binding.strStage = Binding.strActionId;
		Binding.strClip = Client::EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE ==
			Binding.eClockBasis ? Binding.strClipOccurrenceId : std::string{};
		Binding.strBone = Binding.strAnchorSlotId;
		Binding.bFollowBone = Client::EFFECT_V2_FOLLOW_POLICY::FOLLOW_SLOT ==
			Binding.eFollowPolicy;
		switch (Binding.eRotationBasis)
		{
		case Client::EFFECT_V2_ROTATION_BASIS::SLOT:
			Binding.eRotation = Client::CEffectV2Object::PIVOT_ROTATION::BONE;
			break;
		case Client::EFFECT_V2_ROTATION_BASIS::WORLD:
			Binding.eRotation = Client::CEffectV2Object::PIVOT_ROTATION::WORLD;
			break;
		default:
			Binding.eRotation = Client::CEffectV2Object::PIVOT_ROTATION::TARGET_YAW;
			break;
		}
		Binding.bStopWithClip =
			Client::EFFECT_V2_STOP_POLICY::STAGE_END == Binding.eStopPolicy ||
			Client::EFFECT_V2_STOP_POLICY::CLIP_OCCURRENCE_END == Binding.eStopPolicy;
		Binding.vOffset = Binding.LocalTransform.vTranslation;
		Binding.fYawDegrees = Binding.LocalTransform.vRotation.y;
	}

	void Populate_GroupChildConvenience(Client::EFFECT_V2_GROUP_CHILD& Child)
	{
		Child.strEffectId.clear();
		Child.strGroupId.clear();
		if (Client::EFFECT_V2_RESOURCE_KIND::LEAF == Child.eResourceKind)
			Child.strEffectId = Child.strResourceId;
		else
			Child.strGroupId = Child.strResourceId;
		Child.vOffset = Child.LocalTransform.vTranslation;
		Child.fPitchDegrees = Child.LocalTransform.vRotation.x;
		Child.fYawDegrees = Child.LocalTransform.vRotation.y;
		Child.fRollDegrees = Child.LocalTransform.vRotation.z;
		Child.vScale = Child.LocalTransform.vScale;
	}

	/* formatVersion 1 is the clip-keyed lane (NPC, KoukuSaydon preview bodies):
	   rows carry no stable bindingId and no pattern/stage scope, and fire from
	   CEffectV2Runtime::Notify_Clip. Typed fields are filled so v2 consumers
	   see one shape. */
	bool_t Parse_ClipLaneBindings(
		const Client::DATA_JSON_VALUE& Root,
		std::vector<Client::EFFECT_V2_BINDING>& OutBindings,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* const pRows = Root.Find("bindings");
		if (nullptr == pRows || !pRows->Is_Array() ||
			pRows->Get_Array().size() > MAX_BINDINGS)
		{
			strOutError = "bindings must be an array with at most 4096 entries.";
			return false;
		}
		std::vector<Client::EFFECT_V2_BINDING> Staged;
		Staged.reserve(pRows->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& Row : pRows->Get_Array())
		{
			if (!Row.Is_Object())
			{
				strOutError = "bindings[] entries must be objects.";
				return false;
			}
			Client::EFFECT_V2_BINDING Binding;
			const Client::DATA_JSON_VALUE* const pEffect = Row.Find("effectId");
			const Client::DATA_JSON_VALUE* const pGroup = Row.Find("group");
			const Client::DATA_JSON_VALUE* const pClip = Row.Find("clip");
			const Client::DATA_JSON_VALUE* const pStart = Row.Find("startMs");
			const Client::DATA_JSON_VALUE* const pBone = Row.Find("bone");
			const bool_t bHasEffect = nullptr != pEffect && pEffect->Is_String() &&
				!pEffect->Get_String().empty();
			const bool_t bHasGroup = nullptr != pGroup && pGroup->Is_String() &&
				!pGroup->Get_String().empty();
			if (bHasEffect == bHasGroup ||
				(bHasEffect && !Client::CEffectV2Document::Is_ValidEffectId(pEffect->Get_String())) ||
				(bHasGroup && !Client::CEffectV2Document::Is_ValidEffectId(pGroup->Get_String())) ||
				nullptr != Row.Find("stage") ||
				nullptr == pClip || !pClip->Is_String() || pClip->Get_String().empty() ||
				nullptr == pStart || !pStart->Is_Number() || pStart->Get_Number() < 0.0 ||
				pStart->Get_Number() > MAX_BINDING_MS ||
				nullptr == pBone || !pBone->Is_String())
			{
				strOutError = "formatVersion 1 bindings[] requires exactly one of effectId/group, clip, startMs (0-600000), bone; stage rows need formatVersion 2.";
				return false;
			}
			Binding.eResourceKind = bHasEffect ?
				Client::EFFECT_V2_RESOURCE_KIND::LEAF : Client::EFFECT_V2_RESOURCE_KIND::GROUP;
			Binding.strResourceId = bHasEffect ? pEffect->Get_String() : pGroup->Get_String();
			Binding.eClockBasis = Client::EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE;
			Binding.strClipOccurrenceId = pClip->Get_String();
			Binding.iStartMs = static_cast<uint32_t>(pStart->Get_Number());
			Binding.strAnchorSlotId = pBone->Get_String();

			bool_t bFollowBone = true;
			bool_t bStopWithClip = false;
			int32_t iRotation = static_cast<int32_t>(Client::CEffectV2Object::PIVOT_ROTATION::TARGET_YAW);
			float3_t vOffset{ 0.f, 0.f, 0.f };
			f32_t fYawDegrees = 0.f;
			if (!Read_Bool(Row, "followBone", bFollowBone, strOutError) ||
				!Read_Enum(Row, "rotation", PIVOT_ROTATION_KEYS,
					_countof(PIVOT_ROTATION_KEYS), iRotation, strOutError) ||
				!Read_Bool(Row, "stopWithClip", bStopWithClip, strOutError) ||
				!Read_FloatArray(Row, "offset", &vOffset.x, 3u, strOutError) ||
				!Read_Number(Row, "yawDegrees", fYawDegrees, strOutError))
			{
				return false;
			}
			Binding.eFollowPolicy = bFollowBone ?
				Client::EFFECT_V2_FOLLOW_POLICY::FOLLOW_SLOT :
				Client::EFFECT_V2_FOLLOW_POLICY::SNAPSHOT_AT_START;
			switch (static_cast<Client::CEffectV2Object::PIVOT_ROTATION>(iRotation))
			{
			case Client::CEffectV2Object::PIVOT_ROTATION::BONE:
				Binding.eRotationBasis = Client::EFFECT_V2_ROTATION_BASIS::SLOT;
				break;
			case Client::CEffectV2Object::PIVOT_ROTATION::WORLD:
				Binding.eRotationBasis = Client::EFFECT_V2_ROTATION_BASIS::WORLD;
				break;
			default:
				Binding.eRotationBasis = Client::EFFECT_V2_ROTATION_BASIS::TARGET_YAW;
				break;
			}
			Binding.eStopPolicy = bStopWithClip ?
				Client::EFFECT_V2_STOP_POLICY::CLIP_OCCURRENCE_END :
				Client::EFFECT_V2_STOP_POLICY::NATURAL;
			Binding.LocalTransform.vTranslation = vOffset;
			Binding.LocalTransform.vRotation = { 0.f, fYawDegrees, 0.f };
			Binding.LocalTransform.vScale = { 1.f, 1.f, 1.f };
			for (const Client::EFFECT_V2_BINDING& Existing : Staged)
			{
				if (Equal_BindingSemantic(Existing, Binding))
				{
					strOutError = "duplicate binding: " + Binding.strResourceId + " / " +
						Binding.strClipOccurrenceId + " @" +
						std::to_string(Binding.iStartMs) + "ms";
					return false;
				}
			}
			Populate_BindingConvenience(Binding);
			Staged.push_back(std::move(Binding));
		}
		OutBindings = std::move(Staged);
		return true;
	}

	bool_t Is_ClipLane(const std::vector<Client::EFFECT_V2_BINDING>& Bindings)
	{
		return !Bindings.empty() && std::all_of(Bindings.begin(), Bindings.end(),
			[](const Client::EFFECT_V2_BINDING& Binding)
			{
				return Binding.strBindingId.empty() &&
					Binding.strActionId.empty() && Binding.strStage.empty();
			});
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

std::filesystem::path Client::CEffectV2Document::Group_Directory()
{
	return CProjectDataRoot::Resolve(L"Effects/V2/Groups");
}

std::filesystem::path Client::CEffectV2Document::Document_Path(const std::string& strEffectId)
{
	return Document_Directory() / (strEffectId + ".effectv2.json");
}

std::filesystem::path Client::CEffectV2Document::Binding_Path(const std::string& strArchetypeId)
{
	return Binding_Directory() / (strArchetypeId + ".effectv2bindings.json");
}

std::filesystem::path Client::CEffectV2Document::Group_Path(const std::string& strGroupId)
{
	return Group_Directory() / (strGroupId + ".effectv2group.json");
}

bool_t Client::CEffectV2Document::Is_ValidEffectId(const std::string& strEffectId)
{
	return Is_StableAsciiId(strEffectId, 80u);
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

const char* Client::CEffectV2Document::Child_Stop_Key(const EFFECT_V2_CHILD_STOP eStop)
{
	const size_t iIndex = static_cast<size_t>(eStop);
	return iIndex < _countof(CHILD_STOP_KEYS) ? CHILD_STOP_KEYS[iIndex] : "Deactivate";
}

Client::CEffectV2Object::SHAPE Client::CEffectV2Document::Shape_ForType(const EFFECT_V2_TYPE eType)
{
	switch (eType)
	{
	case EFFECT_V2_TYPE::MESH: return CEffectV2Object::SHAPE::MESH;
	case EFFECT_V2_TYPE::PARTICLE: return CEffectV2Object::SHAPE::PARTICLE;
	case EFFECT_V2_TYPE::DECAL: return CEffectV2Object::SHAPE::DECAL;
	case EFFECT_V2_TYPE::TRAIL: return CEffectV2Object::SHAPE::TRAIL;
	case EFFECT_V2_TYPE::SCREEN_POST: return CEffectV2Object::SHAPE::SCREEN_POST;
	default: return CEffectV2Object::SHAPE::SPRITE;
	}
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
	Document.Desc.eShape = Shape_ForType(Document.eType);

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
		!Read_FloatArray(*pParams, "colorOffsetEnd", &P.vColorOffsetEnd.x, 4u, strOutError) ||
		!Read_Bool(*pParams, "colorOffsetLerp", P.bColorOffsetLerp, strOutError) ||
		!Read_FloatArray(*pParams, "colorMul", &P.vColorMul.x, 4u, strOutError) ||
		!Read_FloatArray(*pParams, "colorMulEnd", &P.vColorMulEnd.x, 4u, strOutError) ||
		!Read_Bool(*pParams, "colorMulLerp", P.bColorMulLerp, strOutError) ||
		!Read_Enum(*pParams, "colorClipChannel", CLIP_CHANNEL_KEYS,
			_countof(CLIP_CHANNEL_KEYS), iClipChannel, strOutError) ||
		!Read_Number(*pParams, "colorClip", P.fColorClip, strOutError) ||
		!Read_FloatArray(*pParams, "rimColor", &P.vRimColor.x, 4u, strOutError) ||
		!Read_Number(*pParams, "rimPower", P.fRimPower, strOutError) ||
		!Read_Number(*pParams, "rimIntensity", P.fRimIntensity, strOutError) ||
		!Read_Number(*pParams, "ghostAlpha", P.fGhostAlpha, strOutError) ||
		!Read_Number(*pParams, "outlineWidth", P.fOutlineWidth, strOutError) ||
		!Read_FloatArray(*pParams, "outlineColor", &P.vOutlineColor.x, 4u, strOutError) ||
		!Read_Number(*pParams, "bloomIntensity", P.fBloomIntensity, strOutError) ||
		!Read_Number(*pParams, "distortionIntensity", P.fDistortionIntensity, strOutError) ||
		!Read_FloatArray(*pParams, "uvStart", &P.vUVStart.x, 2u, strOutError) ||
		!Read_FloatArray(*pParams, "uvSpeed", &P.vUVSpeed.x, 2u, strOutError) ||
		!Read_FloatArray(*pParams, "uvTileCount", &P.vUVTileCount.x, 2u, strOutError) ||
		!Read_Number(*pParams, "noiseStrength", P.fNoiseStrength, strOutError) ||
		!Read_Number(*pParams, "noiseScale", P.fNoiseScale, strOutError) ||
		!Read_FloatArray(*pParams, "noisePan", &P.vNoisePan.x, 2u, strOutError) ||
		!Read_Number(*pParams, "dissolveStart", P.fDissolveStart, strOutError) ||
		!Read_Number(*pParams, "dissolveInEnd", P.fDissolveInEnd, strOutError) ||
		!Read_Number(*pParams, "dissolveSoftness", P.fDissolveSoftness, strOutError) ||
		!Read_Bool(*pParams, "dissolveWarp", P.bDissolveWarp, strOutError) ||
		!Read_Bool(*pParams, "maskWarp", P.bMaskWarp, strOutError) ||
		!Read_Number(*pParams, "alphaInEnd", P.fAlphaInEnd, strOutError) ||
		!Read_Number(*pParams, "alphaOutStart", P.fAlphaOutStart, strOutError) ||
		!Read_Number(*pParams, "scaleInEnd", P.fScaleInEnd, strOutError) ||
		!Read_Number(*pParams, "scaleOutStart", P.fScaleOutStart, strOutError) ||
		!Read_Enum(*pParams, "blend", BLEND_KEYS, _countof(BLEND_KEYS), iBlend, strOutError) ||
		!Read_Bool(*pParams, "billboard", P.bBillboard, strOutError) ||
		!Read_Bool(*pParams, "depthTest", P.bDepthTest, strOutError) ||
		!Read_Number(*pParams, "softFadeDistance", P.fSoftFadeDistance, strOutError) ||
		!Read_Number(*pParams, "lifetime", P.fLifetime, strOutError) ||
		!Read_Bool(*pParams, "loop", P.bLoop, strOutError) ||
		!Read_Number(*pParams, "playRate", P.fPlayRate, strOutError) ||
		!Read_Number(*pParams, "meshPreScale", P.fMeshPreScale, strOutError) ||
		!Read_Bool(*pParams, "animationLoop", P.bAnimationLoop, strOutError) ||
		!Read_Bool(*pParams, "colorTexturesSRGB", P.bColorTexturesSRGB, strOutError))
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
	if (P.fSoftFadeDistance < 0.f)
	{
		strOutError = "params.softFadeDistance must be >= 0.";
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
	if (const DATA_JSON_VALUE* pParticle = pParams->Find("particle"))
	{
		if (!pParticle->Is_Object())
		{
			strOutError = "params.particle must be an object.";
			return false;
		}
		CEffectV2Object::PARTICLE_PARAMS& E = P.Particle;
		int32_t iSpawnShape = static_cast<int32_t>(E.eSpawnShape);
		int32_t iVelocityMode = static_cast<int32_t>(E.eVelocityMode);
		int32_t iAlignment = static_cast<int32_t>(E.eAlignment);
		if (!Read_Uint(*pParticle, "maxParticles", E.iMaxParticles, strOutError) ||
			!Read_Number(*pParticle, "spawnRate", E.fSpawnRate, strOutError) ||
			!Read_Uint(*pParticle, "burstCount", E.iBurstCount, strOutError) ||
			!Read_FloatArray(*pParticle, "lifetime", &E.vLifetime.x, 2u, strOutError) ||
			!Read_Enum(*pParticle, "spawnShape", SPAWN_SHAPE_KEYS,
				_countof(SPAWN_SHAPE_KEYS), iSpawnShape, strOutError) ||
			!Read_Number(*pParticle, "spawnRadius", E.fSpawnRadius, strOutError) ||
			!Read_Number(*pParticle, "spawnInnerRadius", E.fSpawnInnerRadius, strOutError) ||
			!Read_FloatArray(*pParticle, "spawnExtents", &E.vSpawnExtents.x, 3u, strOutError) ||
			!Read_Number(*pParticle, "spawnArcDegrees", E.fSpawnArcDegrees, strOutError) ||
			!Read_Enum(*pParticle, "velocityMode", VELOCITY_MODE_KEYS,
				_countof(VELOCITY_MODE_KEYS), iVelocityMode, strOutError) ||
			!Read_FloatArray(*pParticle, "velocityMin", &E.vVelocityMin.x, 3u, strOutError) ||
			!Read_FloatArray(*pParticle, "velocityMax", &E.vVelocityMax.x, 3u, strOutError) ||
			!Read_FloatArray(*pParticle, "speedRange", &E.vSpeedRange.x, 2u, strOutError) ||
			!Read_Number(*pParticle, "coneAngleDegrees", E.fConeAngleDegrees, strOutError) ||
			!Read_FloatArray(*pParticle, "acceleration", &E.vAcceleration.x, 3u, strOutError) ||
			!Read_Number(*pParticle, "drag", E.fDrag, strOutError) ||
			!Read_FloatArray(*pParticle, "sizeStart", &E.vSizeStart.x, 2u, strOutError) ||
			!Read_FloatArray(*pParticle, "sizeEnd", &E.vSizeEnd.x, 2u, strOutError) ||
			!Read_FloatArray(*pParticle, "rotationRange", &E.vRotationRange.x, 2u, strOutError) ||
			!Read_FloatArray(*pParticle, "spinRange", &E.vSpinRange.x, 2u, strOutError) ||
			!Read_FloatArray(*pParticle, "colorStart", &E.vColorStart.x, 4u, strOutError) ||
			!Read_FloatArray(*pParticle, "colorEnd", &E.vColorEnd.x, 4u, strOutError) ||
			!Read_Enum(*pParticle, "alignment", ALIGNMENT_KEYS,
				_countof(ALIGNMENT_KEYS), iAlignment, strOutError) ||
			!Read_Bool(*pParticle, "localSpace", E.bLocalSpace, strOutError) ||
			!Read_Uint(*pParticle, "tileColumns", E.iTileColumns, strOutError) ||
			!Read_Uint(*pParticle, "tileRows", E.iTileRows, strOutError) ||
			!Read_Bool(*pParticle, "subUVOverLife", E.bSubUVOverLife, strOutError) ||
			!Read_Uint(*pParticle, "randomSeed", E.iRandomSeed, strOutError) ||
			!Read_FloatArray(*pParticle, "meshRotationMin", &E.vMeshRotationMin.x, 3u, strOutError) ||
			!Read_FloatArray(*pParticle, "meshRotationMax", &E.vMeshRotationMax.x, 3u, strOutError) ||
			!Read_FloatArray(*pParticle, "meshSpinMin", &E.vMeshSpinMin.x, 3u, strOutError) ||
			!Read_FloatArray(*pParticle, "meshSpinMax", &E.vMeshSpinMax.x, 3u, strOutError))
		{
			return false;
		}
		E.eSpawnShape = static_cast<CEffectV2Object::PARTICLE_SPAWN_SHAPE>(iSpawnShape);
		E.eVelocityMode = static_cast<CEffectV2Object::PARTICLE_VELOCITY_MODE>(iVelocityMode);
		E.eAlignment = static_cast<CEffectV2Object::PARTICLE_ALIGNMENT>(iAlignment);
		if (0u == E.iMaxParticles || E.iMaxParticles > 2048u || E.fSpawnRate < 0.f ||
			E.vLifetime.x <= 0.f || E.vLifetime.y < E.vLifetime.x ||
			0u == E.iTileColumns || 0u == E.iTileRows)
		{
			strOutError = "params.particle maxParticles/spawnRate/lifetime/tile out of range.";
			return false;
		}
	}
	if (const DATA_JSON_VALUE* pDecal = pParams->Find("decal"))
	{
		if (!pDecal->Is_Object())
		{
			strOutError = "params.decal must be an object.";
			return false;
		}
		CEffectV2Object::DECAL_PARAMS& D = P.Decal;
		if (!Read_FloatArray(*pDecal, "size", &D.vSize.x, 2u, strOutError) ||
			!Read_Number(*pDecal, "depth", D.fDepth, strOutError) ||
			!Read_Number(*pDecal, "edgeFade", D.fEdgeFade, strOutError) ||
			!Read_Number(*pDecal, "normalCutoff", D.fNormalCutoff, strOutError))
		{
			return false;
		}
		if (D.vSize.x <= 0.f || D.vSize.y <= 0.f || D.fDepth <= 0.f)
		{
			strOutError = "params.decal size/depth must be positive.";
			return false;
		}
	}
	if (const DATA_JSON_VALUE* pTrail = pParams->Find("trail"))
	{
		if (!pTrail->Is_Object())
		{
			strOutError = "params.trail must be an object.";
			return false;
		}
		CEffectV2Object::TRAIL_PARAMS& T = P.Trail;
		int32_t iEdgeMode = static_cast<int32_t>(T.eEdgeMode);
		if (!Read_Uint(*pTrail, "maxPoints", T.iMaxPoints, strOutError) ||
			!Read_Number(*pTrail, "pointLifetime", T.fPointLifetime, strOutError) ||
			!Read_Number(*pTrail, "sampleInterval", T.fSampleInterval, strOutError) ||
			!Read_Number(*pTrail, "minDistance", T.fMinDistance, strOutError) ||
			!Read_Number(*pTrail, "startWidth", T.fStartWidth, strOutError) ||
			!Read_Number(*pTrail, "endWidth", T.fEndWidth, strOutError) ||
			!Read_Number(*pTrail, "tilingDistance", T.fTilingDistance, strOutError) ||
			!Read_Enum(*pTrail, "edgeMode", TRAIL_EDGE_KEYS,
				_countof(TRAIL_EDGE_KEYS), iEdgeMode, strOutError) ||
			!Read_FloatArray(*pTrail, "edgeOffset", &T.vEdgeOffset.x, 3u, strOutError) ||
			!Read_Bool(*pTrail, "fadeWithAge", T.bFadeWithAge, strOutError))
		{
			return false;
		}
		T.eEdgeMode = static_cast<CEffectV2Object::TRAIL_EDGE_MODE>(iEdgeMode);
		if (T.iMaxPoints < 2u || T.iMaxPoints > 4096u || T.fPointLifetime <= 0.f ||
			T.fSampleInterval < 0.f || T.fMinDistance < 0.f)
		{
			strOutError = "params.trail maxPoints/pointLifetime/sampleInterval/minDistance out of range.";
			return false;
		}
	}
	if (const DATA_JSON_VALUE* pScreenPost = pParams->Find("screenPost"))
	{
		if (!pScreenPost->Is_Object())
		{
			strOutError = "params.screenPost must be an object.";
			return false;
		}
		CEffectV2Object::SCREEN_POST_PARAMS& S = P.ScreenPost;
		int32_t iProfile = static_cast<int32_t>(S.eProfile);
		if (!Read_Enum(*pScreenPost, "profile", SCREEN_POST_PROFILE_KEYS,
				_countof(SCREEN_POST_PROFILE_KEYS), iProfile, strOutError) ||
			!Read_Number(*pScreenPost, "intensityStart", S.fIntensityStart, strOutError) ||
			!Read_Number(*pScreenPost, "intensityEnd", S.fIntensityEnd, strOutError) ||
			!Read_Bool(*pScreenPost, "intensityLerp", S.bIntensityLerp, strOutError) ||
			!Read_Number(*pScreenPost, "secondaryIntensity", S.fSecondaryIntensity, strOutError) ||
			!Read_Number(*pScreenPost, "frequency", S.fFrequency, strOutError) ||
			!Read_FloatArray(*pScreenPost, "tint", &S.vTint.x, 4u, strOutError) ||
			!Read_Uint(*pScreenPost, "randomSeed", S.iRandomSeed, strOutError))
		{
			return false;
		}
		S.eProfile = static_cast<CEffectV2Object::SCREEN_POST_PROFILE>(iProfile);
		if (S.fIntensityStart < 0.f || S.fIntensityEnd < 0.f || S.fSecondaryIntensity < 0.f ||
			S.fFrequency < 0.f || 0u == S.iRandomSeed)
		{
			strOutError = "params.screenPost intensities/frequency must be >= 0 and randomSeed >= 1.";
			return false;
		}
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
	if (!Has_ExactFields(
		Root, { "schema", "formatVersion", "archetypeId", "bindings" }))
	{
		strOutError = "Bindings fields must be exactly schema, formatVersion, archetypeId, bindings.";
		return false;
	}
	const DATA_JSON_VALUE* const pSchema = Root.Find("schema");
	const DATA_JSON_VALUE* const pVersion = Root.Find("formatVersion");
	const DATA_JSON_VALUE* const pArchetype = Root.Find("archetypeId");
	if (nullptr == pSchema || !pSchema->Is_String() ||
		pSchema->Get_String() != "lostark.effect-v2-bindings" ||
		nullptr == pVersion || !pVersion->Is_Number() ||
		(pVersion->Get_Number() != 2.0 && pVersion->Get_Number() != 1.0) ||
		nullptr == pArchetype || !pArchetype->Is_String() ||
		pArchetype->Get_String() != strExpectedArchetypeId)
	{
		strOutError = "schema/formatVersion/archetypeId mismatch.";
		return false;
	}
	if (pVersion->Get_Number() == 1.0)
		return Parse_ClipLaneBindings(Root, OutBindings, strOutError);
	const DATA_JSON_VALUE* pRows = Root.Find("bindings");
	if (nullptr == pRows || !pRows->Is_Array() ||
		pRows->Get_Array().size() > MAX_BINDINGS)
	{
		strOutError = "bindings must be an array with at most 4096 entries.";
		return false;
	}
	std::vector<EFFECT_V2_BINDING> Staged;
	Staged.reserve(pRows->Get_Array().size());
	std::set<std::string, std::less<>> BindingIds;
	std::string strPreviousBindingId;
	for (const DATA_JSON_VALUE& Row : pRows->Get_Array())
	{
		if (!Has_ExactFields(Row,
			{ "bindingId", "resource", "scope", "clock", "anchor", "stopPolicy" }))
		{
			strOutError = "bindings[] fields must be exactly bindingId, resource, scope, clock, anchor, stopPolicy.";
			return false;
		}
		EFFECT_V2_BINDING Binding;
		if (!Read_StableId(Row, "bindingId", Binding.strBindingId, strOutError))
			return false;
		if (!BindingIds.insert(Binding.strBindingId).second)
		{
			strOutError = "duplicate Effect V2 bindingId: " + Binding.strBindingId;
			return false;
		}
		if (!strPreviousBindingId.empty() &&
			!(strPreviousBindingId < Binding.strBindingId))
		{
			strOutError = "BOSS_VALTAN Effect V2 bindings must be sorted by bindingId.";
			return false;
		}
		strPreviousBindingId = Binding.strBindingId;

		const DATA_JSON_VALUE* const pResource = Row.Find("resource");
		if (nullptr == pResource ||
			!Has_ExactFields(*pResource, { "kind", "id" }))
		{
			strOutError = "bindings[].resource fields must be exactly kind, id.";
			return false;
		}
		int32_t iResourceKind = static_cast<int32_t>(Binding.eResourceKind);
		if (!Read_Enum(*pResource, "kind", RESOURCE_KIND_KEYS,
			_countof(RESOURCE_KIND_KEYS), iResourceKind, strOutError))
		{
			return false;
		}
		Binding.eResourceKind = static_cast<EFFECT_V2_RESOURCE_KIND>(iResourceKind);
		if (!Read_StableId(*pResource, "id", Binding.strResourceId, strOutError,
			EFFECT_V2_RESOURCE_KIND::LEAF == Binding.eResourceKind ?
			80u : MAX_STABLE_ID_LENGTH))
		{
			return false;
		}

		const DATA_JSON_VALUE* const pScope = Row.Find("scope");
		if (nullptr == pScope || !Has_ExactFields(
			*pScope, { "patternId", "stageId", "actionId" }) ||
			!Read_StableId(*pScope, "patternId", Binding.strPatternId, strOutError) ||
			!Read_StableId(*pScope, "stageId", Binding.strStageId, strOutError) ||
			!Read_StableId(*pScope, "actionId", Binding.strActionId, strOutError))
		{
			if (strOutError.empty())
				strOutError = "bindings[].scope fields must be exactly patternId, stageId, actionId.";
			return false;
		}

		const DATA_JSON_VALUE* const pClock = Row.Find("clock");
		if (nullptr == pClock || !Has_ExactFields(
			*pClock, { "basis", "clipOccurrenceId", "startMs", "repeatPolicy" }))
		{
			strOutError = "bindings[].clock fields must be exactly basis, clipOccurrenceId, startMs, repeatPolicy.";
			return false;
		}
		int32_t iClockBasis = static_cast<int32_t>(Binding.eClockBasis);
		int32_t iRepeatPolicy = static_cast<int32_t>(Binding.eRepeatPolicy);
		if (!Read_Enum(*pClock, "basis", CLOCK_BASIS_KEYS,
				_countof(CLOCK_BASIS_KEYS), iClockBasis, strOutError) ||
			!Read_MsField(*pClock, "startMs", Binding.iStartMs, strOutError) ||
			!Read_Enum(*pClock, "repeatPolicy", REPEAT_POLICY_KEYS,
				_countof(REPEAT_POLICY_KEYS), iRepeatPolicy, strOutError))
		{
			return false;
		}
		Binding.eClockBasis = static_cast<EFFECT_V2_CLOCK_BASIS>(iClockBasis);
		Binding.eRepeatPolicy = static_cast<EFFECT_V2_REPEAT_POLICY>(iRepeatPolicy);
		const DATA_JSON_VALUE* const pOccurrence = pClock->Find("clipOccurrenceId");
		if (EFFECT_V2_CLOCK_BASIS::STAGE == Binding.eClockBasis)
		{
			if (nullptr == pOccurrence || !pOccurrence->Is_Null() ||
				EFFECT_V2_REPEAT_POLICY::ONCE != Binding.eRepeatPolicy)
			{
				strOutError = "STAGE clock requires null clipOccurrenceId and ONCE repeatPolicy.";
				return false;
			}
		}
		else if (!Read_StableId(*pClock, "clipOccurrenceId",
			Binding.strClipOccurrenceId, strOutError))
		{
			return false;
		}

		const DATA_JSON_VALUE* const pAnchor = Row.Find("anchor");
		if (nullptr == pAnchor || !Has_ExactFields(
			*pAnchor, { "slotId", "followPolicy", "rotationBasis", "localTransform" }) ||
			!Read_StableId(*pAnchor, "slotId", Binding.strAnchorSlotId, strOutError))
		{
			if (strOutError.empty())
				strOutError = "bindings[].anchor fields must be exactly slotId, followPolicy, rotationBasis, localTransform.";
			return false;
		}
		int32_t iFollowPolicy = static_cast<int32_t>(Binding.eFollowPolicy);
		int32_t iRotationBasis = static_cast<int32_t>(Binding.eRotationBasis);
		const DATA_JSON_VALUE* const pTransform = pAnchor->Find("localTransform");
		if (!Read_Enum(*pAnchor, "followPolicy", FOLLOW_POLICY_KEYS,
				_countof(FOLLOW_POLICY_KEYS), iFollowPolicy, strOutError) ||
			!Read_Enum(*pAnchor, "rotationBasis", ROTATION_BASIS_KEYS,
				_countof(ROTATION_BASIS_KEYS), iRotationBasis, strOutError) ||
			nullptr == pTransform ||
			!Read_LocalTransform(*pTransform, Binding.LocalTransform, strOutError))
		{
			return false;
		}
		Binding.eFollowPolicy = static_cast<EFFECT_V2_FOLLOW_POLICY>(iFollowPolicy);
		Binding.eRotationBasis = static_cast<EFFECT_V2_ROTATION_BASIS>(iRotationBasis);

		int32_t iStopPolicy = static_cast<int32_t>(Binding.eStopPolicy);
		if (!Read_Enum(Row, "stopPolicy", STOP_POLICY_KEYS,
			_countof(STOP_POLICY_KEYS), iStopPolicy, strOutError))
			return false;
		Binding.eStopPolicy = static_cast<EFFECT_V2_STOP_POLICY>(iStopPolicy);
		if (EFFECT_V2_STOP_POLICY::CLIP_OCCURRENCE_END == Binding.eStopPolicy &&
			EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE != Binding.eClockBasis)
		{
			strOutError = "CLIP_OCCURRENCE_END requires a CLIP_OCCURRENCE clock.";
			return false;
		}

		for (const EFFECT_V2_BINDING& Existing : Staged)
		{
			if (Equal_BindingSemantic(Existing, Binding))
			{
				strOutError = "duplicate Effect V2 semantic binding occurrence: " +
					Binding.strBindingId;
				return false;
			}
		}
		Populate_BindingConvenience(Binding);
		Staged.push_back(std::move(Binding));
	}
	OutBindings = std::move(Staged);
	return true;
}

bool_t Client::CEffectV2Document::Parse_Group(
	const std::string& strText,
	EFFECT_V2_GROUP& OutGroup,
	std::string& strOutError)
{
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(strText, Root, strOutError) || !Root.Is_Object())
	{
		if (strOutError.empty())
			strOutError = "Group root is not an object.";
		return false;
	}
	if (!Has_ExactFields(
		Root, { "schema", "formatVersion", "groupId", "durationMs", "children" }))
	{
		strOutError = "Group fields must be exactly schema, formatVersion, groupId, durationMs, children.";
		return false;
	}
	const DATA_JSON_VALUE* const pSchema = Root.Find("schema");
	const DATA_JSON_VALUE* const pVersion = Root.Find("formatVersion");
	const DATA_JSON_VALUE* const pGroupId = Root.Find("groupId");
	if (nullptr == pSchema || !pSchema->Is_String() ||
		pSchema->Get_String() != "lostark.effect-v2-group" ||
		nullptr == pVersion || !pVersion->Is_Number() || pVersion->Get_Number() != 2.0 ||
		nullptr == pGroupId || !pGroupId->Is_String() ||
		!Is_StableAsciiId(pGroupId->Get_String(), MAX_STABLE_ID_LENGTH))
	{
		strOutError = "schema/formatVersion/groupId mismatch.";
		return false;
	}
	EFFECT_V2_GROUP Group;
	Group.strGroupId = pGroupId->Get_String();
	if (!Read_MsField(Root, "durationMs", Group.iDurationMs, strOutError))
		return false;
	const DATA_JSON_VALUE* pChildren = Root.Find("children");
	if (nullptr == pChildren || !pChildren->Is_Array() ||
		pChildren->Get_Array().empty() ||
		pChildren->Get_Array().size() > MAX_GROUP_CHILDREN)
	{
		strOutError = "children must be an array with 1..4096 entries.";
		return false;
	}
	Group.Children.reserve(pChildren->Get_Array().size());
	std::set<std::string, std::less<>> ChildIds;
	for (const DATA_JSON_VALUE& Row : pChildren->Get_Array())
	{
		if (!Has_ExactFields(Row,
			{ "childId", "resource", "startMs", "durationMs", "stop", "localTransform" }))
		{
			strOutError = "children[] fields must be exactly childId, resource, startMs, durationMs, stop, localTransform.";
			return false;
		}
		EFFECT_V2_GROUP_CHILD Child;
		if (!Read_StableId(Row, "childId", Child.strChildId, strOutError))
			return false;
		if (!ChildIds.insert(Child.strChildId).second)
		{
			strOutError = "duplicate Effect V2 group childId: " +
				Group.strGroupId + "/" + Child.strChildId;
			return false;
		}

		const DATA_JSON_VALUE* const pResource = Row.Find("resource");
		if (nullptr == pResource ||
			!Has_ExactFields(*pResource, { "kind", "id" }))
		{
			strOutError = "children[].resource fields must be exactly kind, id.";
			return false;
		}
		int32_t iResourceKind = static_cast<int32_t>(Child.eResourceKind);
		if (!Read_Enum(*pResource, "kind", RESOURCE_KIND_KEYS,
			_countof(RESOURCE_KIND_KEYS), iResourceKind, strOutError))
		{
			return false;
		}
		Child.eResourceKind = static_cast<EFFECT_V2_RESOURCE_KIND>(iResourceKind);
		if (!Read_StableId(*pResource, "id", Child.strResourceId, strOutError,
			EFFECT_V2_RESOURCE_KIND::LEAF == Child.eResourceKind ?
			80u : MAX_STABLE_ID_LENGTH))
		{
			return false;
		}

		int32_t iStop = static_cast<int32_t>(Child.eStop);
		const DATA_JSON_VALUE* const pTransform = Row.Find("localTransform");
		if (!Read_MsField(Row, "startMs", Child.iStartMs, strOutError) ||
			!Read_MsField(Row, "durationMs", Child.iDurationMs, strOutError) ||
			!Read_Enum(Row, "stop", CHILD_STOP_KEYS, _countof(CHILD_STOP_KEYS), iStop, strOutError) ||
			nullptr == pTransform ||
			!Read_LocalTransform(*pTransform, Child.LocalTransform, strOutError))
		{
			return false;
		}
		Child.eStop = static_cast<EFFECT_V2_CHILD_STOP>(iStop);
		for (const EFFECT_V2_GROUP_CHILD& Existing : Group.Children)
		{
			if (Equal_GroupChildSemantic(Existing, Child))
			{
				strOutError = "duplicate Effect V2 group semantic child: " +
					Group.strGroupId + "/" + Child.strChildId;
				return false;
			}
		}
		Populate_GroupChildConvenience(Child);
		Group.Children.push_back(std::move(Child));
	}
	OutGroup = std::move(Group);
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
	Text += "    \"colorOffsetEnd\": " + Json_Float4(P.vColorOffsetEnd) + ",\n";
	Text += std::string("    \"colorOffsetLerp\": ") + Json_Bool(P.bColorOffsetLerp) + ",\n";
	Text += "    \"colorMul\": " + Json_Float4(P.vColorMul) + ",\n";
	Text += "    \"colorMulEnd\": " + Json_Float4(P.vColorMulEnd) + ",\n";
	Text += std::string("    \"colorMulLerp\": ") + Json_Bool(P.bColorMulLerp) + ",\n";
	Text += "    \"colorClipChannel\": " + Json_String(
		CLIP_CHANNEL_KEYS[static_cast<size_t>(P.eColorClipChannel)]) + ",\n";
	Text += "    \"colorClip\": " + Json_Number(P.fColorClip) + ",\n";
	Text += "    \"rimColor\": " + Json_Float4(P.vRimColor) + ",\n";
	Text += "    \"rimPower\": " + Json_Number(P.fRimPower) + ",\n";
	Text += "    \"rimIntensity\": " + Json_Number(P.fRimIntensity) + ",\n";
	Text += "    \"ghostAlpha\": " + Json_Number(P.fGhostAlpha) + ",\n";
	Text += "    \"outlineWidth\": " + Json_Number(P.fOutlineWidth) + ",\n";
	Text += "    \"outlineColor\": " + Json_Float4(P.vOutlineColor) + ",\n";
	Text += "    \"bloomIntensity\": " + Json_Number(P.fBloomIntensity) + ",\n";
	Text += "    \"distortionIntensity\": " + Json_Number(P.fDistortionIntensity) + ",\n";
	Text += "    \"uvStart\": " + Json_Float2(P.vUVStart) + ",\n";
	Text += "    \"uvSpeed\": " + Json_Float2(P.vUVSpeed) + ",\n";
	Text += "    \"uvTileCount\": " + Json_Float2(P.vUVTileCount) + ",\n";
	Text += "    \"noiseStrength\": " + Json_Number(P.fNoiseStrength) + ",\n";
	Text += "    \"noiseScale\": " + Json_Number(P.fNoiseScale) + ",\n";
	Text += "    \"noisePan\": " + Json_Float2(P.vNoisePan) + ",\n";
	Text += "    \"dissolveStart\": " + Json_Number(P.fDissolveStart) + ",\n";
	Text += "    \"dissolveInEnd\": " + Json_Number(P.fDissolveInEnd) + ",\n";
	Text += "    \"dissolveSoftness\": " + Json_Number(P.fDissolveSoftness) + ",\n";
	Text += std::string("    \"dissolveWarp\": ") + Json_Bool(P.bDissolveWarp) + ",\n";
	Text += std::string("    \"maskWarp\": ") + Json_Bool(P.bMaskWarp) + ",\n";
	Text += "    \"alphaInEnd\": " + Json_Number(P.fAlphaInEnd) + ",\n";
	Text += "    \"alphaOutStart\": " + Json_Number(P.fAlphaOutStart) + ",\n";
	Text += "    \"scaleInEnd\": " + Json_Number(P.fScaleInEnd) + ",\n";
	Text += "    \"scaleOutStart\": " + Json_Number(P.fScaleOutStart) + ",\n";
	Text += "    \"blend\": " + Json_String(BLEND_KEYS[static_cast<size_t>(P.eBlend)]) + ",\n";
	Text += std::string("    \"billboard\": ") + Json_Bool(P.bBillboard) + ",\n";
	Text += std::string("    \"depthTest\": ") + Json_Bool(P.bDepthTest) + ",\n";
	Text += "    \"softFadeDistance\": " + Json_Number(P.fSoftFadeDistance) + ",\n";
	Text += "    \"lifetime\": " + Json_Number(P.fLifetime) + ",\n";
	Text += std::string("    \"loop\": ") + Json_Bool(P.bLoop) + ",\n";
	Text += "    \"playRate\": " + Json_Number(P.fPlayRate) + ",\n";
	Text += "    \"meshPreScale\": " + Json_Number(P.fMeshPreScale) + ",\n";
	Text += "    \"animationClip\": " + Json_String(Document.strAnimationClip) + ",\n";
	Text += std::string("    \"animationLoop\": ") + Json_Bool(P.bAnimationLoop) + ",\n";
	Text += std::string("    \"colorTexturesSRGB\": ") + Json_Bool(P.bColorTexturesSRGB) + ",\n";
	const CEffectV2Object::PARTICLE_PARAMS& E = P.Particle;
	Text += "    \"particle\": {\n";
	Text += "      \"maxParticles\": " + std::to_string(E.iMaxParticles) + ",\n";
	Text += "      \"spawnRate\": " + Json_Number(E.fSpawnRate) + ",\n";
	Text += "      \"burstCount\": " + std::to_string(E.iBurstCount) + ",\n";
	Text += "      \"lifetime\": " + Json_Float2(E.vLifetime) + ",\n";
	Text += "      \"spawnShape\": " + Json_String(
		SPAWN_SHAPE_KEYS[static_cast<size_t>(E.eSpawnShape)]) + ",\n";
	Text += "      \"spawnRadius\": " + Json_Number(E.fSpawnRadius) + ",\n";
	Text += "      \"spawnInnerRadius\": " + Json_Number(E.fSpawnInnerRadius) + ",\n";
	Text += "      \"spawnExtents\": " + Json_Float3(E.vSpawnExtents) + ",\n";
	Text += "      \"spawnArcDegrees\": " + Json_Number(E.fSpawnArcDegrees) + ",\n";
	Text += "      \"velocityMode\": " + Json_String(
		VELOCITY_MODE_KEYS[static_cast<size_t>(E.eVelocityMode)]) + ",\n";
	Text += "      \"velocityMin\": " + Json_Float3(E.vVelocityMin) + ",\n";
	Text += "      \"velocityMax\": " + Json_Float3(E.vVelocityMax) + ",\n";
	Text += "      \"speedRange\": " + Json_Float2(E.vSpeedRange) + ",\n";
	Text += "      \"coneAngleDegrees\": " + Json_Number(E.fConeAngleDegrees) + ",\n";
	Text += "      \"acceleration\": " + Json_Float3(E.vAcceleration) + ",\n";
	Text += "      \"drag\": " + Json_Number(E.fDrag) + ",\n";
	Text += "      \"sizeStart\": " + Json_Float2(E.vSizeStart) + ",\n";
	Text += "      \"sizeEnd\": " + Json_Float2(E.vSizeEnd) + ",\n";
	Text += "      \"rotationRange\": " + Json_Float2(E.vRotationRange) + ",\n";
	Text += "      \"spinRange\": " + Json_Float2(E.vSpinRange) + ",\n";
	Text += "      \"colorStart\": " + Json_Float4(E.vColorStart) + ",\n";
	Text += "      \"colorEnd\": " + Json_Float4(E.vColorEnd) + ",\n";
	Text += "      \"alignment\": " + Json_String(
		ALIGNMENT_KEYS[static_cast<size_t>(E.eAlignment)]) + ",\n";
	Text += std::string("      \"localSpace\": ") + Json_Bool(E.bLocalSpace) + ",\n";
	Text += "      \"tileColumns\": " + std::to_string(E.iTileColumns) + ",\n";
	Text += "      \"tileRows\": " + std::to_string(E.iTileRows) + ",\n";
	Text += std::string("      \"subUVOverLife\": ") + Json_Bool(E.bSubUVOverLife) + ",\n";
	Text += "      \"randomSeed\": " + std::to_string(E.iRandomSeed) + ",\n";
	Text += "      \"meshRotationMin\": " + Json_Float3(E.vMeshRotationMin) + ",\n";
	Text += "      \"meshRotationMax\": " + Json_Float3(E.vMeshRotationMax) + ",\n";
	Text += "      \"meshSpinMin\": " + Json_Float3(E.vMeshSpinMin) + ",\n";
	Text += "      \"meshSpinMax\": " + Json_Float3(E.vMeshSpinMax) + "\n";
	Text += "    },\n";
	const CEffectV2Object::DECAL_PARAMS& D = P.Decal;
	Text += "    \"decal\": {\n";
	Text += "      \"size\": " + Json_Float2(D.vSize) + ",\n";
	Text += "      \"depth\": " + Json_Number(D.fDepth) + ",\n";
	Text += "      \"edgeFade\": " + Json_Number(D.fEdgeFade) + ",\n";
	Text += "      \"normalCutoff\": " + Json_Number(D.fNormalCutoff) + "\n";
	Text += "    },\n";
	const CEffectV2Object::TRAIL_PARAMS& T = P.Trail;
	Text += "    \"trail\": {\n";
	Text += "      \"maxPoints\": " + std::to_string(T.iMaxPoints) + ",\n";
	Text += "      \"pointLifetime\": " + Json_Number(T.fPointLifetime) + ",\n";
	Text += "      \"sampleInterval\": " + Json_Number(T.fSampleInterval) + ",\n";
	Text += "      \"minDistance\": " + Json_Number(T.fMinDistance) + ",\n";
	Text += "      \"startWidth\": " + Json_Number(T.fStartWidth) + ",\n";
	Text += "      \"endWidth\": " + Json_Number(T.fEndWidth) + ",\n";
	Text += "      \"tilingDistance\": " + Json_Number(T.fTilingDistance) + ",\n";
	Text += "      \"edgeMode\": " + Json_String(
		TRAIL_EDGE_KEYS[static_cast<size_t>(T.eEdgeMode)]) + ",\n";
	Text += "      \"edgeOffset\": " + Json_Float3(T.vEdgeOffset) + ",\n";
	Text += std::string("      \"fadeWithAge\": ") + Json_Bool(T.bFadeWithAge) + "\n";
	Text += "    },\n";
	const CEffectV2Object::SCREEN_POST_PARAMS& S = P.ScreenPost;
	Text += "    \"screenPost\": {\n";
	Text += "      \"profile\": " + Json_String(
		SCREEN_POST_PROFILE_KEYS[static_cast<size_t>(S.eProfile)]) + ",\n";
	Text += "      \"intensityStart\": " + Json_Number(S.fIntensityStart) + ",\n";
	Text += "      \"intensityEnd\": " + Json_Number(S.fIntensityEnd) + ",\n";
	Text += std::string("      \"intensityLerp\": ") + Json_Bool(S.bIntensityLerp) + ",\n";
	Text += "      \"secondaryIntensity\": " + Json_Number(S.fSecondaryIntensity) + ",\n";
	Text += "      \"frequency\": " + Json_Number(S.fFrequency) + ",\n";
	Text += "      \"tint\": " + Json_Float4(S.vTint) + ",\n";
	Text += "      \"randomSeed\": " + std::to_string(S.iRandomSeed) + "\n";
	Text += "    }\n";
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
	if (Is_ClipLane(Bindings))
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
			Text += std::string("    { ") +
				(Binding.strGroupId.empty() ?
					"\"effectId\": " + Json_String(Binding.strEffectId) :
					"\"group\": " + Json_String(Binding.strGroupId)) +
				", \"clip\": " + Json_String(Binding.strClip) +
				", \"startMs\": " + std::to_string(Binding.iStartMs) +
				", \"bone\": " + Json_String(Binding.strBone) +
				", \"followBone\": " + Json_Bool(Binding.bFollowBone) +
				", \"rotation\": " + Json_String(Rotation_Key(Binding.eRotation)) +
				", \"stopWithClip\": " + Json_Bool(Binding.bStopWithClip) +
				", \"offset\": " + Json_Float3(Binding.vOffset) +
				", \"yawDegrees\": " + Json_Number(Binding.fYawDegrees) + " }" +
				(iIndex + 1u < Bindings.size() ? ",\n" : "\n");
		}
		Text += "  ]\n";
		Text += "}\n";
		return Text;
	}

	std::vector<const EFFECT_V2_BINDING*> Sorted;
	Sorted.reserve(Bindings.size());
	for (const EFFECT_V2_BINDING& Binding : Bindings)
		Sorted.push_back(&Binding);
	std::sort(Sorted.begin(), Sorted.end(),
		[](const EFFECT_V2_BINDING* const pLeft,
			const EFFECT_V2_BINDING* const pRight)
		{
			return pLeft->strBindingId < pRight->strBindingId;
		});

	std::string Text;
	Text += "{\n";
	Text += "  \"schema\": \"lostark.effect-v2-bindings\",\n";
	Text += "  \"formatVersion\": 2,\n";
	Text += "  \"archetypeId\": " + Json_String(strArchetypeId) + ",\n";
	Text += "  \"bindings\": [\n";
	for (size_t iIndex = 0u; iIndex < Sorted.size(); ++iIndex)
	{
		const EFFECT_V2_BINDING& Binding = *Sorted[iIndex];
		EFFECT_V2_RESOURCE_KIND eResourceKind = Binding.eResourceKind;
		std::string strResourceId = Binding.strResourceId;
		if (strResourceId.empty())
		{
			if (!Binding.strGroupId.empty())
			{
				eResourceKind = EFFECT_V2_RESOURCE_KIND::GROUP;
				strResourceId = Binding.strGroupId;
			}
			else
			{
				eResourceKind = EFFECT_V2_RESOURCE_KIND::LEAF;
				strResourceId = Binding.strEffectId;
			}
		}
		const std::string& strActionId = Binding.strActionId.empty() ?
			Binding.strStage : Binding.strActionId;
		const std::string& strAnchorSlotId = Binding.strAnchorSlotId.empty() ?
			Binding.strBone : Binding.strAnchorSlotId;

		Text += "    {\n";
		Text += "      \"bindingId\": " + Json_String(Binding.strBindingId) + ",\n";
		Text += "      \"resource\": { \"kind\": " +
			Json_String(Enum_Key(RESOURCE_KIND_KEYS, _countof(RESOURCE_KIND_KEYS),
				static_cast<int32_t>(eResourceKind), "LEAF")) +
			", \"id\": " + Json_String(strResourceId) + " },\n";
		Text += "      \"scope\": { \"patternId\": " +
			Json_String(Binding.strPatternId) + ", \"stageId\": " +
			Json_String(Binding.strStageId) + ", \"actionId\": " +
			Json_String(strActionId) + " },\n";
		Text += "      \"clock\": { \"basis\": " +
			Json_String(Enum_Key(CLOCK_BASIS_KEYS, _countof(CLOCK_BASIS_KEYS),
				static_cast<int32_t>(Binding.eClockBasis), "STAGE")) +
			", \"clipOccurrenceId\": " +
			(EFFECT_V2_CLOCK_BASIS::STAGE == Binding.eClockBasis ?
				std::string("null") : Json_String(Binding.strClipOccurrenceId)) +
			", \"startMs\": " + std::to_string(Binding.iStartMs) +
			", \"repeatPolicy\": " +
			Json_String(Enum_Key(REPEAT_POLICY_KEYS, _countof(REPEAT_POLICY_KEYS),
				static_cast<int32_t>(Binding.eRepeatPolicy), "ONCE")) + " },\n";
		Text += "      \"anchor\": { \"slotId\": " +
			Json_String(strAnchorSlotId) + ", \"followPolicy\": " +
			Json_String(Enum_Key(FOLLOW_POLICY_KEYS, _countof(FOLLOW_POLICY_KEYS),
				static_cast<int32_t>(Binding.eFollowPolicy), "FOLLOW_SLOT")) +
			", \"rotationBasis\": " +
			Json_String(Enum_Key(ROTATION_BASIS_KEYS, _countof(ROTATION_BASIS_KEYS),
				static_cast<int32_t>(Binding.eRotationBasis), "TARGET_YAW")) +
			", \"localTransform\": " + Json_LocalTransform(Binding.LocalTransform) + " },\n";
		Text += "      \"stopPolicy\": " +
			Json_String(Enum_Key(STOP_POLICY_KEYS, _countof(STOP_POLICY_KEYS),
				static_cast<int32_t>(Binding.eStopPolicy), "NATURAL")) + "\n";
		Text += std::string("    }") +
			(iIndex + 1u < Sorted.size() ? ",\n" : "\n");
	}
	Text += "  ]\n";
	Text += "}\n";
	return Text;
}

std::string Client::CEffectV2Document::Serialize_Group(const EFFECT_V2_GROUP& Group)
{
	std::string Text;
	Text += "{\n";
	Text += "  \"schema\": \"lostark.effect-v2-group\",\n";
	Text += "  \"formatVersion\": 2,\n";
	Text += "  \"groupId\": " + Json_String(Group.strGroupId) + ",\n";
	Text += "  \"durationMs\": " + std::to_string(Group.iDurationMs) + ",\n";
	Text += "  \"children\": [\n";
	for (size_t iIndex = 0u; iIndex < Group.Children.size(); ++iIndex)
	{
		const EFFECT_V2_GROUP_CHILD& Child = Group.Children[iIndex];
		EFFECT_V2_RESOURCE_KIND eResourceKind = Child.eResourceKind;
		std::string strResourceId = Child.strResourceId;
		if (strResourceId.empty())
		{
			if (!Child.strGroupId.empty())
			{
				eResourceKind = EFFECT_V2_RESOURCE_KIND::GROUP;
				strResourceId = Child.strGroupId;
			}
			else
			{
				eResourceKind = EFFECT_V2_RESOURCE_KIND::LEAF;
				strResourceId = Child.strEffectId;
			}
		}
		Text += "    { \"childId\": " + Json_String(Child.strChildId) +
			", \"resource\": { \"kind\": " +
			Json_String(Enum_Key(RESOURCE_KIND_KEYS, _countof(RESOURCE_KIND_KEYS),
				static_cast<int32_t>(eResourceKind), "LEAF")) +
			", \"id\": " + Json_String(strResourceId) + " }" +
			", \"startMs\": " + std::to_string(Child.iStartMs) +
			", \"durationMs\": " + std::to_string(Child.iDurationMs) +
			", \"stop\": " + Json_String(Child_Stop_Key(Child.eStop)) +
			", \"localTransform\": " + Json_LocalTransform(Child.LocalTransform) + " }" +
			(iIndex + 1u < Group.Children.size() ? ",\n" : "\n");
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
	EFFECT_V2_DOCUMENT Staged;
	if (!Parse_Document(Text, Staged, strOutError))
		return false;
	if (Staged.strEffectId != strEffectId)
	{
		strOutError = "effectId does not match the file name.";
		return false;
	}
	OutDocument = std::move(Staged);
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
	std::vector<EFFECT_V2_BINDING> Staged;
	if (!Parse_Bindings(Text, strArchetypeId, Staged, strOutError))
		return false;
	OutBindings = std::move(Staged);
	return true;
}

bool_t Client::CEffectV2Document::Load_GroupFile(
	const std::string& strGroupId,
	EFFECT_V2_GROUP& OutGroup,
	std::string& strOutError)
{
	if (!Is_ValidEffectId(strGroupId))
	{
		strOutError = "Invalid group ID.";
		return false;
	}
	std::string Text;
	const std::filesystem::path Path = Group_Path(strGroupId);
	if (!Read_TextFile(Path, Text))
	{
		strOutError = "Cannot open: " + Path.string();
		return false;
	}
	EFFECT_V2_GROUP Staged;
	if (!Parse_Group(Text, Staged, strOutError))
		return false;
	if (Staged.strGroupId != strGroupId)
	{
		strOutError = "groupId does not match the file name.";
		return false;
	}
	OutGroup = std::move(Staged);
	return true;
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
