#include "Effect_DocumentCodec.h"

#include "DataJson.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <system_error>
#include <unordered_set>

namespace
{
	constexpr const char_t* EFFECT_DOCUMENT_SCHEMA =
		"lostark.effect-authoring";
	constexpr size_t MAX_RESOURCE_ID_BYTES = 512u;
	constexpr size_t MAX_ELEMENTS = 256u;
	constexpr uint64_t MAX_DOCUMENT_PARTICLES = 8192u;
	constexpr uint64_t MAX_DOCUMENT_TRAIL_POINTS = 2048u;
	constexpr uint64_t MAX_DOCUMENT_AFTERIMAGES = 256u;

	constexpr const char_t* KIND_TOKENS[] =
	{
		"mesh", "sprite", "particle", "decal", "trail"
	};
	constexpr const char_t* SLOT_TOKENS[] =
	{
		"meshModel", "base", "noise", "mask", "emissive", "dissolve"
	};
	constexpr const char_t* PROFILE_TOKENS[] =
	{
		"opaque_back_depth_write",
		"alpha_two_sided_depth_read",
		"additive_two_sided_depth_read"
	};

	bool_t Is_StableId(const std::string& Value)
	{
		if (Value.empty() || Value.size() > 128u)
			return false;
		return std::all_of(Value.begin(), Value.end(),
			[](const char_t Character)
			{
				const unsigned char Value =
					static_cast<unsigned char>(Character);
				return 0 != std::isalnum(Value) || Character == '_' ||
					Character == '.' || Character == '-';
			});
	}

	bool_t Has_VisibleCharacter(const std::string& Value)
	{
		return std::any_of(Value.begin(), Value.end(),
			[](const char_t Character)
			{
				return 0 == std::isspace(
					static_cast<unsigned char>(Character));
			});
	}

	bool_t Is_Finite(const float2_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y);
	}

	bool_t Is_Finite(const float3_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z);
	}

	bool_t Is_Finite(const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	}

	Client::EFFECT_RESOURCE_FILE_KIND FileKindForSlot(
		const Client::EFFECT_RESOURCE_SLOT eSlot)
	{
		if (Client::EFFECT_RESOURCE_SLOT::MESH_MODEL == eSlot)
			return Client::EFFECT_RESOURCE_FILE_KIND::MODEL;
		if (eSlot >= Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE &&
			eSlot <= Client::EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE)
		{
			return Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE;
		}
		return Client::EFFECT_RESOURCE_FILE_KIND::END;
	}

	const Client::DATA_JSON_VALUE* Find_Field(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		const Client::DATA_JSON_TYPE eType,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pName);
		if (nullptr == pValue || pValue->Get_Type() != eType)
		{
			strOutError = std::string("Missing or invalid field: ") + pName;
			return nullptr;
		}
		return pValue;
	}

	bool_t Read_Float(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		f32_t& OutValue,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pValue = Find_Field(
			Object, pName, Client::DATA_JSON_TYPE::NUMBER, strOutError);
		if (nullptr == pValue || !std::isfinite(pValue->Get_Number()))
			return false;
		OutValue = static_cast<f32_t>(pValue->Get_Number());
		return std::isfinite(OutValue);
	}

	bool_t Read_Int(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		int32_t& OutValue,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pValue = Find_Field(
			Object, pName, Client::DATA_JSON_TYPE::NUMBER, strOutError);
		if (nullptr == pValue || !std::isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() != std::floor(pValue->Get_Number()) ||
			pValue->Get_Number() < static_cast<double>(INT32_MIN) ||
			pValue->Get_Number() > static_cast<double>(INT32_MAX))
		{
			return false;
		}
		OutValue = static_cast<int32_t>(pValue->Get_Number());
		return true;
	}

	bool_t Read_UInt(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		uint32_t& OutValue,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pValue = Find_Field(
			Object, pName, Client::DATA_JSON_TYPE::NUMBER, strOutError);
		if (nullptr == pValue || !std::isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() != std::floor(pValue->Get_Number()) ||
			pValue->Get_Number() < 0.0 ||
			pValue->Get_Number() > static_cast<double>(UINT32_MAX))
		{
			return false;
		}
		OutValue = static_cast<uint32_t>(pValue->Get_Number());
		return true;
	}

	bool_t Read_Bool(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		bool_t& OutValue,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pValue = Find_Field(
			Object, pName, Client::DATA_JSON_TYPE::BOOLEAN, strOutError);
		if (nullptr == pValue)
			return false;
		OutValue = pValue->Get_Boolean();
		return true;
	}

	bool_t Read_Array(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		f32_t* pOut,
		const size_t iCount,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pValue = Find_Field(
			Object, pName, Client::DATA_JSON_TYPE::ARRAY, strOutError);
		if (nullptr == pValue || pValue->Get_Array().size() != iCount)
			return false;
		for (size_t iValue = 0u; iValue < iCount; ++iValue)
		{
			const Client::DATA_JSON_VALUE& Item = pValue->Get_Array()[iValue];
			if (!Item.Is_Number() || !std::isfinite(Item.Get_Number()))
				return false;
			pOut[iValue] = static_cast<f32_t>(Item.Get_Number());
			if (!std::isfinite(pOut[iValue]))
				return false;
		}
		return true;
	}

	template<typename ENUM>
	bool_t Parse_Token(
		const std::string& Value,
		const char_t* const* pTokens,
		const size_t iCount,
		ENUM& eOut)
	{
		for (size_t iToken = 0u; iToken < iCount; ++iToken)
		{
			if (Value == pTokens[iToken])
			{
				eOut = static_cast<ENUM>(iToken);
				return true;
			}
		}
		return false;
	}

	void Write_Float2(std::ostringstream& Output, const float2_t& Value)
	{
		Output << '[' << Value.x << ", " << Value.y << ']';
	}

	void Write_Float3(std::ostringstream& Output, const float3_t& Value)
	{
		Output << '[' << Value.x << ", " << Value.y << ", " << Value.z << ']';
	}

	void Write_Float4(std::ostringstream& Output, const float4_t& Value)
	{
		Output << '[' << Value.x << ", " << Value.y << ", "
			<< Value.z << ", " << Value.w << ']';
	}

	bool_t Read_CommonDetail(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_DETAIL_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pTransform = Find_Field(
			Value, "transform", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pColor = Find_Field(
			Value, "color", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pUV = Find_Field(
			Value, "uv", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pTiming = Find_Field(
			Value, "timing", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pMesh = Find_Field(
			Value, "mesh", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pSprite = Find_Field(
			Value, "sprite", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pDecal = Find_Field(
			Value, "decal", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		if (nullptr == pTransform || nullptr == pColor || nullptr == pUV ||
			nullptr == pTiming || nullptr == pMesh || nullptr == pSprite ||
			nullptr == pDecal)
		{
			return false;
		}

		return Read_Array(*pTransform, "position", &Out.Transform.vPosition.x, 3u, strOutError) &&
			Read_Array(*pTransform, "rotationDegrees", &Out.Transform.vRotationDegrees.x, 3u, strOutError) &&
			Read_Array(*pTransform, "revolutionDegreesPerSecond", &Out.Transform.vRevolutionDegreesPerSecond.x, 3u, strOutError) &&
			Read_Array(*pTransform, "scale", &Out.Transform.vScale.x, 3u, strOutError) &&
			Read_Array(*pColor, "offset", &Out.Color.vColorOffset.x, 4u, strOutError) &&
			Read_Array(*pColor, "multiply", &Out.Color.vColorMultiply.x, 4u, strOutError) &&
			Read_Float(*pColor, "clip", Out.Color.fColorClip, strOutError) &&
			Read_Float(*pColor, "emissiveIntensity", Out.Color.fEmissiveIntensity, strOutError) &&
			Read_Float(*pColor, "distortionIntensity", Out.Color.fDistortionIntensity, strOutError) &&
			Read_Bool(*pColor, "distortionOnBaseMaterial", Out.Color.bDistortionOnBaseMaterial, strOutError) &&
			Read_Float(*pColor, "radialTime", Out.Color.fRadialTime, strOutError) &&
			Read_Float(*pColor, "radialIntensity", Out.Color.fRadialIntensity, strOutError) &&
			Read_Array(*pUV, "start", &Out.UV.vStart.x, 2u, strOutError) &&
			Read_Array(*pUV, "speed", &Out.UV.vSpeed.x, 2u, strOutError) &&
			Read_Bool(*pUV, "wave", Out.UV.bWave, strOutError) &&
			Read_Array(*pUV, "waveAmplitude", &Out.UV.vWaveAmplitude.x, 2u, strOutError) &&
			Read_Float(*pUV, "waveFrequency", Out.UV.fWaveFrequency, strOutError) &&
			Read_Bool(*pUV, "sequence", Out.UV.bSequence, strOutError) &&
			Read_Bool(*pUV, "loop", Out.UV.bLoop, strOutError) &&
			Read_Float(*pUV, "sequenceTerm", Out.UV.fSequenceTerm, strOutError) &&
			Read_Int(*pUV, "tileColumns", Out.UV.iTileColumns, strOutError) &&
			Read_Int(*pUV, "tileRows", Out.UV.iTileRows, strOutError) &&
			Read_Int(*pUV, "tileIndex", Out.UV.iTileIndex, strOutError) &&
			Read_Float(*pTiming, "startDelaySeconds", Out.Timing.fStartDelaySeconds, strOutError) &&
			Read_Float(*pTiming, "lifeTimeSeconds", Out.Timing.fLifeTimeSeconds, strOutError) &&
			Read_Float(*pTiming, "afterImageSeconds", Out.Timing.fAfterImageSeconds, strOutError) &&
			Read_Float(*pTiming, "dissolveStartNormalized", Out.Timing.fDissolveStartNormalized, strOutError) &&
			Read_Bool(*pMesh, "useModelMaterial", Out.Mesh.bUseModelMaterial, strOutError) &&
			Read_Bool(*pSprite, "billboard", Out.Sprite.bBillboard, strOutError) &&
			Read_Array(*pDecal, "size", &Out.Decal.vSize.x, 2u, strOutError) &&
			Read_Float(*pDecal, "depth", Out.Decal.fDepth, strOutError);
	}

	bool_t Read_V5Detail(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_DETAIL_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pLerp = Find_Field(
			Value, "linearLerp", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pParticle = Find_Field(
			Value, "particle", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pTrail = Find_Field(
			Value, "trail", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pAfterImage = Find_Field(
			Value, "afterImage", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		if (nullptr == pLerp || nullptr == pParticle || nullptr == pTrail ||
			nullptr == pAfterImage)
		{
			return false;
		}

		return Read_Array(*Value.Find("transform"), "velocityPerSecond", &Out.Transform.vVelocityPerSecond.x, 3u, strOutError) &&
			Read_Bool(*pLerp, "position", Out.LinearLerp.bPosition, strOutError) &&
			Read_Array(*pLerp, "endPosition", &Out.LinearLerp.vEndPosition.x, 3u, strOutError) &&
			Read_Bool(*pLerp, "rotation", Out.LinearLerp.bRotation, strOutError) &&
			Read_Array(*pLerp, "endRotationDegrees", &Out.LinearLerp.vEndRotationDegrees.x, 3u, strOutError) &&
			Read_Bool(*pLerp, "revolution", Out.LinearLerp.bRevolution, strOutError) &&
			Read_Array(*pLerp, "endRevolutionDegreesPerSecond", &Out.LinearLerp.vEndRevolutionDegreesPerSecond.x, 3u, strOutError) &&
			Read_Bool(*pLerp, "scale", Out.LinearLerp.bScale, strOutError) &&
			Read_Array(*pLerp, "endScale", &Out.LinearLerp.vEndScale.x, 3u, strOutError) &&
			Read_Bool(*pLerp, "velocity", Out.LinearLerp.bVelocity, strOutError) &&
			Read_Array(*pLerp, "endVelocityPerSecond", &Out.LinearLerp.vEndVelocityPerSecond.x, 3u, strOutError) &&
			Read_Bool(*pLerp, "colorOffset", Out.LinearLerp.bColorOffset, strOutError) &&
			Read_Array(*pLerp, "endColorOffset", &Out.LinearLerp.vEndColorOffset.x, 4u, strOutError) &&
			Read_Bool(*pLerp, "colorMultiply", Out.LinearLerp.bColorMultiply, strOutError) &&
			Read_Array(*pLerp, "endColorMultiply", &Out.LinearLerp.vEndColorMultiply.x, 4u, strOutError) &&
			Read_Bool(*pLerp, "emissiveIntensity", Out.LinearLerp.bEmissiveIntensity, strOutError) &&
			Read_Float(*pLerp, "endEmissiveIntensity", Out.LinearLerp.fEndEmissiveIntensity, strOutError) &&
			Read_UInt(*pParticle, "maxParticles", Out.Particle.iMaxParticles, strOutError) &&
			Read_Float(*pParticle, "spawnRatePerSecond", Out.Particle.fSpawnRatePerSecond, strOutError) &&
			Read_UInt(*pParticle, "burstCount", Out.Particle.iBurstCount, strOutError) &&
			Read_UInt(*pParticle, "randomSeed", Out.Particle.iRandomSeed, strOutError) &&
			Read_Array(*pParticle, "lifeTimeSeconds", &Out.Particle.vLifeTimeSeconds.x, 2u, strOutError) &&
			Read_Array(*pParticle, "initialVelocityMin", &Out.Particle.vInitialVelocityMin.x, 3u, strOutError) &&
			Read_Array(*pParticle, "initialVelocityMax", &Out.Particle.vInitialVelocityMax.x, 3u, strOutError) &&
			Read_Array(*pParticle, "acceleration", &Out.Particle.vAcceleration.x, 3u, strOutError) &&
			Read_Array(*pParticle, "startSize", &Out.Particle.vStartSize.x, 2u, strOutError) &&
			Read_Array(*pParticle, "endSize", &Out.Particle.vEndSize.x, 2u, strOutError) &&
			Read_Bool(*pParticle, "localSpace", Out.Particle.bLocalSpace, strOutError) &&
			Read_Bool(*pParticle, "billboard", Out.Particle.bBillboard, strOutError) &&
			Read_UInt(*pTrail, "maxPoints", Out.Trail.iMaxPoints, strOutError) &&
			Read_Float(*pTrail, "pointLifeTimeSeconds", Out.Trail.fPointLifeTimeSeconds, strOutError) &&
			Read_Float(*pTrail, "sampleIntervalSeconds", Out.Trail.fSampleIntervalSeconds, strOutError) &&
			Read_Float(*pTrail, "minimumDistance", Out.Trail.fMinimumDistance, strOutError) &&
			Read_Float(*pTrail, "startWidth", Out.Trail.fStartWidth, strOutError) &&
			Read_Float(*pTrail, "endWidth", Out.Trail.fEndWidth, strOutError) &&
			Read_Bool(*pTrail, "faceCamera", Out.Trail.bFaceCamera, strOutError) &&
			Read_Float(*pAfterImage, "sampleIntervalSeconds", Out.AfterImage.fSampleIntervalSeconds, strOutError) &&
			Read_UInt(*pAfterImage, "maxCopies", Out.AfterImage.iMaxCopies, strOutError) &&
			Read_Float(*pAfterImage, "alphaExponent", Out.AfterImage.fAlphaExponent, strOutError);
	}

	void Write_Detail(
		std::ostringstream& Output,
		const Client::EFFECT_DETAIL_DESC& Detail)
	{
		Output << "      \"detail\": {\n"
			<< "        \"transform\": { \"position\": ";
		Write_Float3(Output, Detail.Transform.vPosition);
		Output << ", \"rotationDegrees\": ";
		Write_Float3(Output, Detail.Transform.vRotationDegrees);
		Output << ", \"revolutionDegreesPerSecond\": ";
		Write_Float3(Output, Detail.Transform.vRevolutionDegreesPerSecond);
		Output << ", \"scale\": ";
		Write_Float3(Output, Detail.Transform.vScale);
		Output << ", \"velocityPerSecond\": ";
		Write_Float3(Output, Detail.Transform.vVelocityPerSecond);
		Output << " },\n        \"color\": { \"offset\": ";
		Write_Float4(Output, Detail.Color.vColorOffset);
		Output << ", \"multiply\": ";
		Write_Float4(Output, Detail.Color.vColorMultiply);
		Output << ", \"clip\": " << Detail.Color.fColorClip
			<< ", \"emissiveIntensity\": " << Detail.Color.fEmissiveIntensity
			<< ", \"distortionIntensity\": " << Detail.Color.fDistortionIntensity
			<< ", \"distortionOnBaseMaterial\": " << (Detail.Color.bDistortionOnBaseMaterial ? "true" : "false")
			<< ", \"radialTime\": " << Detail.Color.fRadialTime
			<< ", \"radialIntensity\": " << Detail.Color.fRadialIntensity
			<< " },\n        \"uv\": { \"start\": ";
		Write_Float2(Output, Detail.UV.vStart);
		Output << ", \"speed\": ";
		Write_Float2(Output, Detail.UV.vSpeed);
		Output << ", \"wave\": " << (Detail.UV.bWave ? "true" : "false")
			<< ", \"waveAmplitude\": ";
		Write_Float2(Output, Detail.UV.vWaveAmplitude);
		Output << ", \"waveFrequency\": " << Detail.UV.fWaveFrequency
			<< ", \"sequence\": " << (Detail.UV.bSequence ? "true" : "false")
			<< ", \"loop\": " << (Detail.UV.bLoop ? "true" : "false")
			<< ", \"sequenceTerm\": " << Detail.UV.fSequenceTerm
			<< ", \"tileColumns\": " << Detail.UV.iTileColumns
			<< ", \"tileRows\": " << Detail.UV.iTileRows
			<< ", \"tileIndex\": " << Detail.UV.iTileIndex
			<< " },\n        \"timing\": { \"startDelaySeconds\": " << Detail.Timing.fStartDelaySeconds
			<< ", \"lifeTimeSeconds\": " << Detail.Timing.fLifeTimeSeconds
			<< ", \"afterImageSeconds\": " << Detail.Timing.fAfterImageSeconds
			<< ", \"dissolveStartNormalized\": " << Detail.Timing.fDissolveStartNormalized
			<< " },\n        \"mesh\": { \"useModelMaterial\": " << (Detail.Mesh.bUseModelMaterial ? "true" : "false")
			<< " },\n        \"sprite\": { \"billboard\": " << (Detail.Sprite.bBillboard ? "true" : "false")
			<< " },\n        \"decal\": { \"size\": ";
		Write_Float2(Output, Detail.Decal.vSize);
		Output << ", \"depth\": " << Detail.Decal.fDepth << " },\n"
			<< "        \"linearLerp\": { \"position\": " << (Detail.LinearLerp.bPosition ? "true" : "false")
			<< ", \"endPosition\": ";
		Write_Float3(Output, Detail.LinearLerp.vEndPosition);
		Output << ", \"rotation\": " << (Detail.LinearLerp.bRotation ? "true" : "false") << ", \"endRotationDegrees\": ";
		Write_Float3(Output, Detail.LinearLerp.vEndRotationDegrees);
		Output << ", \"revolution\": " << (Detail.LinearLerp.bRevolution ? "true" : "false") << ", \"endRevolutionDegreesPerSecond\": ";
		Write_Float3(Output, Detail.LinearLerp.vEndRevolutionDegreesPerSecond);
		Output << ", \"scale\": " << (Detail.LinearLerp.bScale ? "true" : "false") << ", \"endScale\": ";
		Write_Float3(Output, Detail.LinearLerp.vEndScale);
		Output << ", \"velocity\": " << (Detail.LinearLerp.bVelocity ? "true" : "false") << ", \"endVelocityPerSecond\": ";
		Write_Float3(Output, Detail.LinearLerp.vEndVelocityPerSecond);
		Output << ", \"colorOffset\": " << (Detail.LinearLerp.bColorOffset ? "true" : "false") << ", \"endColorOffset\": ";
		Write_Float4(Output, Detail.LinearLerp.vEndColorOffset);
		Output << ", \"colorMultiply\": " << (Detail.LinearLerp.bColorMultiply ? "true" : "false") << ", \"endColorMultiply\": ";
		Write_Float4(Output, Detail.LinearLerp.vEndColorMultiply);
		Output << ", \"emissiveIntensity\": " << (Detail.LinearLerp.bEmissiveIntensity ? "true" : "false")
			<< ", \"endEmissiveIntensity\": " << Detail.LinearLerp.fEndEmissiveIntensity << " },\n"
			<< "        \"particle\": { \"maxParticles\": " << Detail.Particle.iMaxParticles
			<< ", \"spawnRatePerSecond\": " << Detail.Particle.fSpawnRatePerSecond
			<< ", \"burstCount\": " << Detail.Particle.iBurstCount
			<< ", \"randomSeed\": " << Detail.Particle.iRandomSeed
			<< ", \"lifeTimeSeconds\": ";
		Write_Float2(Output, Detail.Particle.vLifeTimeSeconds);
		Output << ", \"initialVelocityMin\": ";
		Write_Float3(Output, Detail.Particle.vInitialVelocityMin);
		Output << ", \"initialVelocityMax\": ";
		Write_Float3(Output, Detail.Particle.vInitialVelocityMax);
		Output << ", \"acceleration\": ";
		Write_Float3(Output, Detail.Particle.vAcceleration);
		Output << ", \"startSize\": ";
		Write_Float2(Output, Detail.Particle.vStartSize);
		Output << ", \"endSize\": ";
		Write_Float2(Output, Detail.Particle.vEndSize);
		Output << ", \"localSpace\": " << (Detail.Particle.bLocalSpace ? "true" : "false")
			<< ", \"billboard\": " << (Detail.Particle.bBillboard ? "true" : "false") << " },\n"
			<< "        \"trail\": { \"maxPoints\": " << Detail.Trail.iMaxPoints
			<< ", \"pointLifeTimeSeconds\": " << Detail.Trail.fPointLifeTimeSeconds
			<< ", \"sampleIntervalSeconds\": " << Detail.Trail.fSampleIntervalSeconds
			<< ", \"minimumDistance\": " << Detail.Trail.fMinimumDistance
			<< ", \"startWidth\": " << Detail.Trail.fStartWidth
			<< ", \"endWidth\": " << Detail.Trail.fEndWidth
			<< ", \"faceCamera\": " << (Detail.Trail.bFaceCamera ? "true" : "false") << " },\n"
			<< "        \"afterImage\": { \"sampleIntervalSeconds\": " << Detail.AfterImage.fSampleIntervalSeconds
			<< ", \"maxCopies\": " << Detail.AfterImage.iMaxCopies
			<< ", \"alphaExponent\": " << Detail.AfterImage.fAlphaExponent << " }\n"
			<< "      }\n";
	}
}

const char_t* Client::CEffectDocumentCodec::To_Token(
	const EFFECT_ELEMENT_KIND eKind)
{
	return eKind < EFFECT_ELEMENT_KIND::END ?
		KIND_TOKENS[static_cast<size_t>(eKind)] : "invalid";
}

const char_t* Client::CEffectDocumentCodec::To_Token(
	const EFFECT_RESOURCE_SLOT eSlot)
{
	return eSlot < EFFECT_RESOURCE_SLOT::END ?
		SLOT_TOKENS[static_cast<size_t>(eSlot)] : "invalid";
}

const char_t* Client::CEffectDocumentCodec::To_Token(
	const EFFECT_RENDER_PROFILE eProfile)
{
	return eProfile < EFFECT_RENDER_PROFILE::END ?
		PROFILE_TOKENS[static_cast<size_t>(eProfile)] : "invalid";
}

bool_t Client::CEffectDocumentCodec::Is_ResourceSlotAllowed(
	const EFFECT_ELEMENT_KIND eKind,
	const EFFECT_RESOURCE_SLOT eSlot)
{
	if (eKind >= EFFECT_ELEMENT_KIND::END || eSlot >= EFFECT_RESOURCE_SLOT::END)
		return false;
	if (EFFECT_RESOURCE_SLOT::MESH_MODEL == eSlot)
		return EFFECT_ELEMENT_KIND::MESH == eKind;
	return true;
}

bool_t Client::CEffectDocumentCodec::Is_SafeResourceAssetId(
	const std::string& strAssetId,
	EFFECT_RESOURCE_FILE_KIND* pOutKind)
{
	if (strAssetId.empty() || strAssetId.size() > MAX_RESOURCE_ID_BYTES ||
		0u != strAssetId.rfind("Effect/", 0u) ||
		std::string::npos != strAssetId.find('\\') ||
		std::string::npos != strAssetId.find(':'))
	{
		return false;
	}

	const std::filesystem::path RelativePath(strAssetId);
	if (RelativePath.is_absolute() || RelativePath.has_root_path() ||
		RelativePath.lexically_normal().generic_string() != strAssetId)
	{
		return false;
	}
	for (const std::filesystem::path& Component : RelativePath)
	{
		const std::string Value = Component.generic_string();
		if (Value.empty() || Value == "." || Value == "..")
			return false;
	}

	std::string Extension = RelativePath.extension().string();
	std::transform(Extension.begin(), Extension.end(), Extension.begin(),
		[](const char_t Character)
		{
			return static_cast<char_t>(std::tolower(
				static_cast<unsigned char>(Character)));
		});
	EFFECT_RESOURCE_FILE_KIND eKind = EFFECT_RESOURCE_FILE_KIND::END;
	if (Extension == ".wmodel")
		eKind = EFFECT_RESOURCE_FILE_KIND::MODEL;
	else if (Extension == ".dds")
		eKind = EFFECT_RESOURCE_FILE_KIND::TEXTURE;
	else
		return false;

	const std::filesystem::path Resolved =
		CRuntimeAssetRoot::Resolve(RelativePath);
	std::error_code Error;
	if (Resolved.empty() ||
		!std::filesystem::is_regular_file(Resolved, Error) || Error)
	{
		return false;
	}
	if (nullptr != pOutKind)
		*pOutKind = eKind;
	return true;
}

bool_t Client::CEffectDocumentCodec::Validate(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	if (Document.iFormatVersion != EFFECT_AUTHORING_FORMAT_VERSION)
	{
		strOutError = "Unsupported Effect document version.";
		return false;
	}
	if (!Is_StableId(Document.strEffectAssetId))
	{
		strOutError = "Effect Asset ID is invalid.";
		return false;
	}
	if (Document.strDisplayName.size() > 64u ||
		!Has_VisibleCharacter(Document.strDisplayName))
	{
		strOutError = "Display Name must be 1-64 bytes and not blank.";
		return false;
	}
	if (Document.Elements.size() > MAX_ELEMENTS)
	{
		strOutError = "Effect Element count exceeds 256.";
		return false;
	}

	std::unordered_set<std::string> ElementIds;
	uint64_t iTotalParticles = 0u;
	uint64_t iTotalTrailPoints = 0u;
	uint64_t iTotalAfterImages = 0u;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (!Is_StableId(Element.strElementId) ||
			Element.eKind >= EFFECT_ELEMENT_KIND::END ||
			Element.Material.eRenderProfile >= EFFECT_RENDER_PROFILE::END ||
			!ElementIds.insert(Element.strElementId).second)
		{
			strOutError = "Element identity, kind, profile, or duplicate is invalid.";
			return false;
		}

		std::unordered_set<uint32_t> Slots;
		for (const EFFECT_RESOURCE_BINDING_DESC& Binding : Element.ResourceBindings)
		{
			EFFECT_RESOURCE_FILE_KIND eActualKind = EFFECT_RESOURCE_FILE_KIND::END;
			const EFFECT_RESOURCE_FILE_KIND eExpectedKind = FileKindForSlot(Binding.eSlot);
			if (!Is_ResourceSlotAllowed(Element.eKind, Binding.eSlot) ||
				eExpectedKind == EFFECT_RESOURCE_FILE_KIND::END ||
				!Slots.insert(static_cast<uint32_t>(Binding.eSlot)).second ||
				!Is_SafeResourceAssetId(Binding.strAssetId, &eActualKind) ||
				eActualKind != eExpectedKind)
			{
				strOutError = "Effect resource slot, path, file, or duplicate is invalid.";
				return false;
			}
		}
		const EFFECT_DETAIL_DESC& D = Element.Detail;
		const int64_t iTileCount = static_cast<int64_t>(D.UV.iTileColumns) * D.UV.iTileRows;
		const bool_t bCommonValid =
			Is_Finite(D.Transform.vPosition) && Is_Finite(D.Transform.vRotationDegrees) &&
			Is_Finite(D.Transform.vRevolutionDegreesPerSecond) && Is_Finite(D.Transform.vScale) &&
			D.Transform.vScale.x > 0.f && D.Transform.vScale.y > 0.f && D.Transform.vScale.z > 0.f &&
			Is_Finite(D.Transform.vVelocityPerSecond) && Is_Finite(D.Color.vColorOffset) &&
			Is_Finite(D.Color.vColorMultiply) && std::isfinite(D.Color.fColorClip) &&
			D.Color.fColorClip >= 0.f && D.Color.fColorClip <= 1.f &&
			std::isfinite(D.Color.fEmissiveIntensity) && D.Color.fEmissiveIntensity >= 0.f &&
			std::isfinite(D.Color.fDistortionIntensity) && D.Color.fDistortionIntensity >= 0.f &&
			std::isfinite(D.Color.fRadialTime) && std::isfinite(D.Color.fRadialIntensity) &&
			Is_Finite(D.UV.vStart) && Is_Finite(D.UV.vSpeed) && Is_Finite(D.UV.vWaveAmplitude) &&
			std::isfinite(D.UV.fWaveFrequency) && D.UV.fWaveFrequency >= 0.f &&
			std::isfinite(D.UV.fSequenceTerm) && D.UV.fSequenceTerm > 0.f &&
			D.UV.iTileColumns > 0 && D.UV.iTileRows > 0 && D.UV.iTileIndex >= 0 &&
			iTileCount > 0 && D.UV.iTileIndex < iTileCount &&
			std::isfinite(D.Timing.fStartDelaySeconds) && D.Timing.fStartDelaySeconds >= 0.f &&
			std::isfinite(D.Timing.fLifeTimeSeconds) && D.Timing.fLifeTimeSeconds > 0.f &&
			std::isfinite(D.Timing.fAfterImageSeconds) && D.Timing.fAfterImageSeconds >= 0.f &&
			std::isfinite(D.Timing.fDissolveStartNormalized) &&
			D.Timing.fDissolveStartNormalized >= 0.f && D.Timing.fDissolveStartNormalized <= 1.f &&
			Is_Finite(D.Decal.vSize) && D.Decal.vSize.x > 0.f && D.Decal.vSize.y > 0.f &&
			std::isfinite(D.Decal.fDepth) && D.Decal.fDepth > 0.f;
		const bool_t bLerpValid =
			Is_Finite(D.LinearLerp.vEndPosition) && Is_Finite(D.LinearLerp.vEndRotationDegrees) &&
			Is_Finite(D.LinearLerp.vEndRevolutionDegreesPerSecond) && Is_Finite(D.LinearLerp.vEndScale) &&
			D.LinearLerp.vEndScale.x > 0.f && D.LinearLerp.vEndScale.y > 0.f && D.LinearLerp.vEndScale.z > 0.f &&
			Is_Finite(D.LinearLerp.vEndVelocityPerSecond) && Is_Finite(D.LinearLerp.vEndColorOffset) &&
			Is_Finite(D.LinearLerp.vEndColorMultiply) &&
			std::isfinite(D.LinearLerp.fEndEmissiveIntensity) && D.LinearLerp.fEndEmissiveIntensity >= 0.f;
		const bool_t bParticleValid =
			D.Particle.iMaxParticles >= 1u && D.Particle.iMaxParticles <= 2048u &&
			D.Particle.iBurstCount <= D.Particle.iMaxParticles && D.Particle.iRandomSeed != 0u &&
			std::isfinite(D.Particle.fSpawnRatePerSecond) && D.Particle.fSpawnRatePerSecond >= 0.f && D.Particle.fSpawnRatePerSecond <= 2048.f &&
			Is_Finite(D.Particle.vLifeTimeSeconds) && D.Particle.vLifeTimeSeconds.x > 0.f && D.Particle.vLifeTimeSeconds.y >= D.Particle.vLifeTimeSeconds.x && D.Particle.vLifeTimeSeconds.y <= 30.f &&
			Is_Finite(D.Particle.vInitialVelocityMin) && Is_Finite(D.Particle.vInitialVelocityMax) && Is_Finite(D.Particle.vAcceleration) &&
			D.Particle.vInitialVelocityMax.x >= D.Particle.vInitialVelocityMin.x &&
			D.Particle.vInitialVelocityMax.y >= D.Particle.vInitialVelocityMin.y &&
			D.Particle.vInitialVelocityMax.z >= D.Particle.vInitialVelocityMin.z &&
			Is_Finite(D.Particle.vStartSize) && D.Particle.vStartSize.x > 0.f && D.Particle.vStartSize.y > 0.f &&
			Is_Finite(D.Particle.vEndSize) && D.Particle.vEndSize.x >= 0.f && D.Particle.vEndSize.y >= 0.f;
		const bool_t bTrailValid =
			D.Trail.iMaxPoints >= 2u && D.Trail.iMaxPoints <= 256u &&
			std::isfinite(D.Trail.fPointLifeTimeSeconds) && D.Trail.fPointLifeTimeSeconds > 0.f &&
			std::isfinite(D.Trail.fSampleIntervalSeconds) && D.Trail.fSampleIntervalSeconds > 0.f &&
			std::isfinite(D.Trail.fMinimumDistance) && D.Trail.fMinimumDistance >= 0.f &&
			std::isfinite(D.Trail.fStartWidth) && D.Trail.fStartWidth > 0.f &&
			std::isfinite(D.Trail.fEndWidth) && D.Trail.fEndWidth >= 0.f;
		const bool_t bAfterImageValid =
			std::isfinite(D.AfterImage.fSampleIntervalSeconds) && D.AfterImage.fSampleIntervalSeconds > 0.f &&
			D.AfterImage.iMaxCopies <= 32u && std::isfinite(D.AfterImage.fAlphaExponent) && D.AfterImage.fAlphaExponent > 0.f &&
			(D.Timing.fAfterImageSeconds <= 0.f || D.AfterImage.iMaxCopies == 0u ||
				Element.eKind == EFFECT_ELEMENT_KIND::MESH || Element.eKind == EFFECT_ELEMENT_KIND::SPRITE);
		if (!bCommonValid || !bLerpValid || !bParticleValid ||
			!bTrailValid || !bAfterImageValid)
		{
			strOutError = "Effect Detail contains an invalid number or range.";
			return false;
		}
		if (EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind)
			iTotalParticles += D.Particle.iMaxParticles;
		if (EFFECT_ELEMENT_KIND::TRAIL == Element.eKind)
			iTotalTrailPoints += D.Trail.iMaxPoints;
		if (D.Timing.fAfterImageSeconds > 0.f &&
			D.AfterImage.iMaxCopies > 0u)
			iTotalAfterImages += D.AfterImage.iMaxCopies;
	}
	if (iTotalParticles > MAX_DOCUMENT_PARTICLES ||
		iTotalTrailPoints > MAX_DOCUMENT_TRAIL_POINTS ||
		iTotalAfterImages > MAX_DOCUMENT_AFTERIMAGES)
	{
		strOutError = "Effect Document exceeds the particle, trail, or after-image budget.";
		return false;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentCodec::Validate_Drawable(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	if (!Validate(Document, strOutError))
		return false;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		const EFFECT_RESOURCE_SLOT eRequiredSlot =
			EFFECT_ELEMENT_KIND::MESH == Element.eKind ?
			EFFECT_RESOURCE_SLOT::MESH_MODEL :
			EFFECT_RESOURCE_SLOT::BASE_TEXTURE;
		const bool_t bBound = std::any_of(
			Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
			[eRequiredSlot](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.eSlot == eRequiredSlot;
			});
		if (!bBound)
		{
			strOutError = EFFECT_ELEMENT_KIND::MESH == Element.eKind ?
				"Mesh Element requires a Mesh Model binding." :
				"Sprite/Particle/Decal/Trail Element requires a Base texture binding.";
			return false;
		}
		if (EFFECT_ELEMENT_KIND::MESH == Element.eKind &&
			!Element.Detail.Mesh.bUseModelMaterial)
		{
			const bool_t bHasBaseOverride = std::any_of(
				Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.eSlot == EFFECT_RESOURCE_SLOT::BASE_TEXTURE;
				});
			if (!bHasBaseOverride)
			{
				strOutError = "Mesh Element with useModelMaterial=false requires a Base texture binding.";
				return false;
			}
		}
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentCodec::Parse(
	const std::string_view Json,
	EFFECT_DOCUMENT_DESC& OutDocument,
	std::string& strOutError)
{
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(Json, Root, strOutError))
		return false;
	return Parse_Value(Root, OutDocument, strOutError);
}

bool_t Client::CEffectDocumentCodec::Parse_Value(
	const DATA_JSON_VALUE& Root,
	EFFECT_DOCUMENT_DESC& OutDocument,
	std::string& strOutError)
{
	if (!Root.Is_Object())
	{
		strOutError = "Effect document root must be an object.";
		return false;
	}
	const DATA_JSON_VALUE* pSchema = Root.Find("schema");
	const DATA_JSON_VALUE* pVersion = Root.Find("version");
	const DATA_JSON_VALUE* pAssetId = Root.Find("effectAssetId");
	const DATA_JSON_VALUE* pDisplayName = Root.Find("displayName");
	const DATA_JSON_VALUE* pElements = Root.Find("elements");
	if ((nullptr != pSchema && (!pSchema->Is_String() || pSchema->Get_String() != EFFECT_DOCUMENT_SCHEMA)) ||
		nullptr == pVersion || !pVersion->Is_Number() ||
		nullptr == pAssetId || !pAssetId->Is_String() ||
		nullptr == pDisplayName || !pDisplayName->Is_String() ||
		nullptr == pElements || !pElements->Is_Array())
	{
		strOutError = "Effect document fields or types are invalid.";
		return false;
	}
	const double Version = pVersion->Get_Number();
	if (!std::isfinite(Version) || Version != std::floor(Version) ||
		Version < EFFECT_AUTHORING_MIN_SUPPORTED_VERSION ||
		Version > EFFECT_AUTHORING_FORMAT_VERSION)
	{
		strOutError = "Effect document version is not supported.";
		return false;
	}
	const uint32_t iSourceVersion = static_cast<uint32_t>(Version);

	EFFECT_DOCUMENT_DESC Staged;
	Staged.iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	Staged.strEffectAssetId = pAssetId->Get_String();
	Staged.strDisplayName = pDisplayName->Get_String();
	Staged.Elements.reserve(pElements->Get_Array().size());
	for (const DATA_JSON_VALUE& ElementValue : pElements->Get_Array())
	{
		if (!ElementValue.Is_Object())
		{
			strOutError = "Effect Element must be an object.";
			return false;
		}
		const DATA_JSON_VALUE* pId = ElementValue.Find("id");
		const DATA_JSON_VALUE* pKind = ElementValue.Find("kind");
		const DATA_JSON_VALUE* pResources = ElementValue.Find("resources");
		const DATA_JSON_VALUE* pMaterial = ElementValue.Find("material");
		EFFECT_ELEMENT_DESC Element;
		if (nullptr == pId || !pId->Is_String() || nullptr == pKind || !pKind->Is_String() ||
			nullptr == pResources || !pResources->Is_Array() || nullptr == pMaterial || !pMaterial->Is_Object() ||
			!Parse_Token(pKind->Get_String(), KIND_TOKENS, std::size(KIND_TOKENS), Element.eKind))
		{
			strOutError = "Effect Element identity, kind, resources, or material is invalid.";
			return false;
		}
		Element.strElementId = pId->Get_String();
		for (const DATA_JSON_VALUE& ResourceValue : pResources->Get_Array())
		{
			if (!ResourceValue.Is_Object())
			{
				strOutError = "Effect resource must be an object.";
				return false;
			}
			const DATA_JSON_VALUE* pSlot = ResourceValue.Find("slot");
			const DATA_JSON_VALUE* pResourceId = ResourceValue.Find("assetId");
			EFFECT_RESOURCE_BINDING_DESC Binding;
			if (nullptr == pSlot || !pSlot->Is_String() || nullptr == pResourceId || !pResourceId->Is_String() ||
				!Parse_Token(pSlot->Get_String(), SLOT_TOKENS, std::size(SLOT_TOKENS), Binding.eSlot))
			{
				strOutError = "Effect resource binding is invalid.";
				return false;
			}
			Binding.strAssetId = pResourceId->Get_String();
			Element.ResourceBindings.push_back(std::move(Binding));
		}
		const DATA_JSON_VALUE* pProfile = pMaterial->Find("renderProfile");
		if (nullptr == pProfile || !pProfile->Is_String() ||
			!Parse_Token(pProfile->Get_String(), PROFILE_TOKENS, std::size(PROFILE_TOKENS), Element.Material.eRenderProfile))
		{
			strOutError = "Effect render profile is invalid.";
			return false;
		}
		if (iSourceVersion >= 4u)
		{
			const DATA_JSON_VALUE* pDetail = ElementValue.Find("detail");
			if (nullptr == pDetail || !pDetail->Is_Object() ||
				!Read_CommonDetail(*pDetail, Element.Detail, strOutError))
			{
				return false;
			}
			if (iSourceVersion >= 5u &&
				!Read_V5Detail(*pDetail, Element.Detail, strOutError))
			{
				return false;
			}
		}
		Staged.Elements.push_back(std::move(Element));
	}
	if (!Validate(Staged, strOutError))
		return false;
	OutDocument = std::move(Staged);
	strOutError.clear();
	return true;
}

std::string Client::CEffectDocumentCodec::Serialize(
	const EFFECT_DOCUMENT_DESC& Document)
{
	std::ostringstream Output;
	Output << std::setprecision(9) << "{\n"
		<< "  \"schema\": \"" << EFFECT_DOCUMENT_SCHEMA << "\",\n"
		<< "  \"version\": " << EFFECT_AUTHORING_FORMAT_VERSION << ",\n"
		<< "  \"effectAssetId\": \"" << CDataJson::Escape(Document.strEffectAssetId) << "\",\n"
		<< "  \"displayName\": \"" << CDataJson::Escape(Document.strDisplayName) << "\",\n"
		<< "  \"elements\": [";
	for (size_t iElement = 0u; iElement < Document.Elements.size(); ++iElement)
	{
		const EFFECT_ELEMENT_DESC& Element = Document.Elements[iElement];
		Output << (0u == iElement ? "\n" : ",\n")
			<< "    {\n      \"id\": \"" << CDataJson::Escape(Element.strElementId) << "\",\n"
			<< "      \"kind\": \"" << To_Token(Element.eKind) << "\",\n"
			<< "      \"resources\": [";
		for (size_t iResource = 0u; iResource < Element.ResourceBindings.size(); ++iResource)
		{
			const EFFECT_RESOURCE_BINDING_DESC& Binding = Element.ResourceBindings[iResource];
			Output << (0u == iResource ? "\n" : ",\n")
				<< "        { \"slot\": \"" << To_Token(Binding.eSlot)
				<< "\", \"assetId\": \"" << CDataJson::Escape(Binding.strAssetId) << "\" }";
		}
		if (!Element.ResourceBindings.empty())
			Output << '\n';
		Output << "      ],\n      \"material\": { \"renderProfile\": \""
			<< To_Token(Element.Material.eRenderProfile) << "\" },\n";
		Write_Detail(Output, Element.Detail);
		Output << "    }";
	}
	if (!Document.Elements.empty())
		Output << "\n  ";
	Output << "]\n}\n";
	return Output.str();
}

bool_t Client::CEffectDocumentCodec::Load(
	const std::filesystem::path& Path,
	EFFECT_DOCUMENT_DESC& OutDocument,
	std::string& strOutError)
{
	std::ifstream Input(Path, std::ios::binary);
	if (!Input)
	{
		strOutError = "Effect document could not be opened.";
		return false;
	}
	std::ostringstream Buffer;
	Buffer << Input.rdbuf();
	if (!Input.eof() && Input.fail())
	{
		strOutError = "Effect document read failed.";
		return false;
	}
	return Parse(Buffer.str(), OutDocument, strOutError);
}

bool_t Client::CEffectDocumentCodec::Save_Atomic(
	const std::filesystem::path& Path,
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	// Authoring save preserves valid partial drafts. The publisher/runtime gate
	// still calls Validate_Drawable before a document can ship or render.
	if (!Validate(Document, strOutError))
		return false;
	std::error_code Error;
	std::filesystem::create_directories(Path.parent_path(), Error);
	if (Error)
	{
		strOutError = "Effect authoring directory creation failed.";
		return false;
	}
	const std::filesystem::path Temporary = Path.wstring() + L".tmp";
	const std::filesystem::path Backup = Path.wstring() + L".bak";
	{
		std::ofstream Output(Temporary, std::ios::binary | std::ios::trunc);
		const std::string Json = Serialize(Document);
		Output.write(Json.data(), static_cast<std::streamsize>(Json.size()));
		Output.flush();
		if (!Output)
		{
			strOutError = "Effect temporary write failed.";
			std::filesystem::remove(Temporary, Error);
			return false;
		}
	}
	EFFECT_DOCUMENT_DESC RoundTrip;
	if (!Load(Temporary, RoundTrip, strOutError))
	{
		std::filesystem::remove(Temporary, Error);
		return false;
	}

	std::filesystem::remove(Backup, Error);
	Error.clear();
	const bool_t bHadDestination = std::filesystem::exists(Path, Error) && !Error;
	if (bHadDestination)
	{
		std::filesystem::rename(Path, Backup, Error);
		if (Error)
		{
			strOutError = "Effect destination backup failed.";
			std::filesystem::remove(Temporary, Error);
			return false;
		}
	}
	std::filesystem::rename(Temporary, Path, Error);
	if (Error)
	{
		std::error_code RestoreError;
		if (bHadDestination)
			std::filesystem::rename(Backup, Path, RestoreError);
		strOutError = "Effect document promote failed.";
		return false;
	}
	std::filesystem::remove(Backup, Error);
	strOutError.clear();
	return true;
}

void Client::CEffectDocumentCodec::Collect_ResourceAssetIds(
	const EFFECT_DOCUMENT_DESC& Document,
	std::vector<std::string>& OutAssetIds)
{
	std::unordered_set<std::string> Unique;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		for (const EFFECT_RESOURCE_BINDING_DESC& Binding : Element.ResourceBindings)
			Unique.insert(Binding.strAssetId);
	}
	OutAssetIds.assign(Unique.begin(), Unique.end());
	std::sort(OutAssetIds.begin(), OutAssetIds.end());
}
