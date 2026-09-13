#include "Effect_DocumentCodec_Internal.h"
#include "Effect_MaterialTemplate.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <atomic>
#include <array>
#include <cctype>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <set>
#include <sstream>
#include <system_error>
#include <unordered_map>
#include <unordered_set>


using namespace Client::EffectDocumentCodecDetail;

namespace Client::EffectDocumentCodecDetail
{


	bool_t Read_ModelCueTransform(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_MODEL_CUE_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pLocal = Find_Field(
			Value, "localTransform", Client::DATA_JSON_TYPE::OBJECT,
			strOutError);
		const Client::DATA_JSON_VALUE* pPre = Find_Field(
			Value, "assetPreTransform", Client::DATA_JSON_TYPE::OBJECT,
			strOutError);
		return nullptr != pLocal && nullptr != pPre &&
			Read_Array(*pLocal, "position",
				&Out.LocalTransform.vPosition.x, 3u, strOutError) &&
			Read_Array(*pLocal, "rotationDegrees",
				&Out.LocalTransform.vRotationDegrees.x, 3u, strOutError) &&
			Read_OptionalArray(*pLocal, "revolutionDegreesPerSecond",
				&Out.LocalTransform.vRevolutionDegreesPerSecond.x, 3u,
				strOutError) &&
			Read_Array(*pLocal, "scale",
				&Out.LocalTransform.vScale.x, 3u, strOutError) &&
			Read_OptionalArray(*pLocal, "velocityPerSecond",
				&Out.LocalTransform.vVelocityPerSecond.x, 3u, strOutError) &&
			Read_Array(*pPre, "scale",
				&Out.vAssetPreScale.x, 3u, strOutError) &&
			Read_Array(*pPre, "rotationDegrees",
				&Out.vAssetPreRotationDegrees.x, 3u, strOutError);
	}


	bool_t Read_ActionCueAttachment(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_ACTION_CUE_ATTACHMENT_DESC& Out,
		std::string& strOutError)
	{
		if (const Client::DATA_JSON_VALUE* pOrientation = Value.Find("orientation"))
		{
			if (!pOrientation->Is_String() ||
				!Parse_Token(pOrientation->Get_String(), ATTACHMENT_ORIENTATION_TOKENS,
					std::size(ATTACHMENT_ORIENTATION_TOKENS), Out.eOrientation))
			{
				strOutError = "Effect Action cue attachment orientation must be bone, owner_yaw, or camera_view.";
				return false;
			}
		}
		if (Value.Find("modelCueId") &&
			!Read_String(Value, "modelCueId", Out.strModelCueId, strOutError))
			return false;
		const Client::DATA_JSON_VALUE* pSocketLocal = Find_Field(
			Value, "socketLocalTransform", Client::DATA_JSON_TYPE::OBJECT,
			strOutError);
		return nullptr != pSocketLocal &&
			Read_Bool(Value, "enabled", Out.bEnabled, strOutError) &&
			Read_Bool(Value, "follow", Out.bFollow, strOutError) &&
			Read_String(Value, "sourceAnchorSlotId",
				Out.strSourceAnchorSlotId, strOutError) &&
			Read_String(Value, "runtimeAnchorSlotId",
				Out.strRuntimeAnchorSlotId, strOutError) &&
			Read_String(Value, "runtimeBoneName",
				Out.strRuntimeBoneName, strOutError) &&
			Read_OptionalFloat(Value, "snapshotRootSourceBasisYawDegrees",
				Out.fSnapshotRootSourceBasisYawDegrees, strOutError) &&
			Read_Array(*pSocketLocal, "position",
				&Out.SocketLocalTransform.vPosition.x, 3u, strOutError) &&
			Read_Array(*pSocketLocal, "rotationDegrees",
				&Out.SocketLocalTransform.vRotationDegrees.x, 3u,
				strOutError) &&
			Read_Array(*pSocketLocal, "scale",
				&Out.SocketLocalTransform.vScale.x, 3u, strOutError);
	}


	bool_t Read_TransformInheritance(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_TRANSFORM_INHERITANCE_DESC& Out,
		std::string& strOutError)
	{
		return Read_Bool(Value, "enabled", Out.bEnabled, strOutError) &&
			Read_String(Value, "masterElementId",
				Out.strMasterElementId, strOutError);
	}


	bool_t Is_SafeModelCueAssetIdInternal(const std::string& strAssetId)
	{
		const bool_t bAllowedRoot =
			0u == strAssetId.rfind("Character/", 0u) ||
			0u == strAssetId.rfind("Effect/", 0u);
		if (strAssetId.empty() || strAssetId.size() > MAX_RESOURCE_ID_BYTES ||
			!bAllowedRoot ||
			std::string::npos != strAssetId.find('\\') ||
			std::string::npos != strAssetId.find(':'))
		{
			return false;
		}
		const std::filesystem::path RelativePath(strAssetId);
		if (RelativePath.is_absolute() || RelativePath.has_root_path() ||
			RelativePath.lexically_normal().generic_string() != strAssetId ||
			RelativePath.extension() != ".wmodel")
		{
			return false;
		}
		for (const std::filesystem::path& Component : RelativePath)
		{
			const std::string Value = Component.generic_string();
			if (Value.empty() || Value == "." || Value == "..")
				return false;
		}
		const std::filesystem::path Resolved =
			CRuntimeAssetRoot::Resolve(RelativePath);
		std::error_code Error;
		return !Resolved.empty() &&
			std::filesystem::is_regular_file(Resolved, Error) && !Error;
	}


	bool_t Read_MeshRingFill(
		const Client::DATA_JSON_VALUE& Mesh,
		Client::EFFECT_MESH_RING_FILL_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pRingFill = Mesh.Find("ringFill");
		if (nullptr == pRingFill)
			return true;
		const Client::DATA_JSON_VALUE* pDirection = nullptr;
		if (!pRingFill->Is_Object() ||
			nullptr == (pDirection = pRingFill->Find("direction")) ||
			!pDirection->Is_String() ||
			!Parse_Token(pDirection->Get_String(), RING_FILL_DIRECTION_TOKENS,
				std::size(RING_FILL_DIRECTION_TOKENS), Out.eDirection))
		{
			strOutError = "Effect mesh ringFill direction is invalid.";
			return false;
		}
		return Read_Bool(*pRingFill, "enabled", Out.bEnabled, strOutError) &&
			Read_Float(*pRingFill, "progress", Out.fProgress, strOutError) &&
			Read_Float(*pRingFill, "feather", Out.fFeather, strOutError) &&
			Read_Bool(*pRingFill, "invert", Out.bInvert, strOutError);
	}


	bool_t Read_LinearReveal(
		const Client::DATA_JSON_VALUE& Sprite,
		Client::EFFECT_LINEAR_REVEAL_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pReveal = Sprite.Find("linearReveal");
		if (nullptr == pReveal)
			return true;
		const Client::DATA_JSON_VALUE* pAxis = nullptr;
		if (!pReveal->Is_Object() ||
			nullptr == (pAxis = pReveal->Find("axis")) ||
			!pAxis->Is_String() ||
			!Parse_Token(pAxis->Get_String(), LINEAR_REVEAL_AXIS_TOKENS,
				std::size(LINEAR_REVEAL_AXIS_TOKENS), Out.eAxis))
		{
			strOutError = "Effect linearReveal axis is invalid.";
			return false;
		}
		return Read_Bool(*pReveal, "enabled", Out.bEnabled, strOutError) &&
			Read_Bool(*pReveal, "invert", Out.bInvert, strOutError) &&
			Read_Float(*pReveal, "startSeconds", Out.fStartSeconds,
				strOutError) &&
			Read_Float(*pReveal, "durationSeconds", Out.fDurationSeconds,
				strOutError) &&
			Read_Float(*pReveal, "edgeWidth", Out.fEdgeWidth, strOutError) &&
			Read_Float(*pReveal, "softness", Out.fSoftness, strOutError) &&
			Read_Array(*pReveal, "edgeColor", &Out.vEdgeColor.x, 4u,
				strOutError) &&
			Read_Float(*pReveal, "edgeEmissive", Out.fEdgeEmissive,
				strOutError);
	}


	bool_t Read_DecalReceiver(
		const Client::DATA_JSON_VALUE& Decal,
		Client::EFFECT_DECAL_DETAIL_DESC& Out,
		std::string& strOutError)
	{
		if (const Client::DATA_JSON_VALUE* pMode =
			Decal.Find("receiverMode"))
		{
			if (!pMode->Is_String() ||
				!Parse_Token(pMode->Get_String(), DECAL_RECEIVER_MODE_TOKENS,
					std::size(DECAL_RECEIVER_MODE_TOKENS), Out.eReceiverMode))
			{
				strOutError = "Effect decal receiverMode is invalid.";
				return false;
			}
		}
		return Read_OptionalFloat(Decal, "normalCutoff",
			Out.fNormalCutoff, strOutError) &&
			Read_OptionalFloat(Decal, "edgeFade", Out.fEdgeFade,
				strOutError);
	}


	bool_t Read_Material(const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_MATERIAL_DESC& Out, const uint32_t iSourceVersion,
		const bool_t bSourceContract, std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pTemplateId = Value.Find("templateId");
		if (const Client::DATA_JSON_VALUE* pColorTexturesSRGB =
			Value.Find("colorTexturesSRGB"))
		{
			if (!pColorTexturesSRGB->Is_Boolean())
			{
				strOutError = "Effect Material colorTexturesSRGB must be a boolean.";
				return false;
			}
			Out.bColorTexturesSRGB = pColorTexturesSRGB->Get_Boolean();
		}
		if (iSourceVersion >= 6u)
		{
			if (nullptr == pTemplateId || !pTemplateId->Is_String())
			{
				strOutError = "Effect Material Template ID is invalid.";
				return false;
			}
			Out.strTemplateId = pTemplateId->Get_String();
		}
		if (iSourceVersion >= 10u)
		{
			const Client::DATA_JSON_VALUE* pSourceMaterialPath =
				Value.Find("sourceMaterialPath");
			if (nullptr == pSourceMaterialPath ||
				!pSourceMaterialPath->Is_String())
			{
				strOutError = "Effect source Material path is invalid.";
				return false;
			}
			Out.strSourceMaterialPath =
				pSourceMaterialPath->Get_String();
		}
		if (iSourceVersion >= 11u)
		{
			const Client::DATA_JSON_VALUE* pSourceProfile =
				Value.Find("sourceProfile");
			if (nullptr == pSourceProfile ||
				!Read_SourceMaterialProfile(*pSourceProfile,
					Out.SourceMaterial, strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect source Material profile is invalid.";
				return false;
			}
		}
		if (const Client::DATA_JSON_VALUE* pExecution =
			Value.Find("execution"))
		{
			if (!pExecution->Is_Object() ||
				!Read_MaterialExecution(*pExecution,
					Out.Execution, strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect authored Material execution is invalid.";
				return false;
			}
		}
		const Client::DATA_JSON_VALUE* pProfile = Value.Find("renderProfile");
		if (nullptr == pProfile || !pProfile->Is_String() ||
			!Parse_Token(pProfile->Get_String(), PROFILE_TOKENS, std::size(PROFILE_TOKENS), Out.eRenderProfile))
		{
			strOutError = "Effect render profile is invalid.";
			return false;
		}
		if (!bSourceContract && iSourceVersion >= 11u &&
			Out.strTemplateId ==
				Client::EFFECT_SOURCE_MATERIAL_TEMPLATE_ID &&
			!Out.SourceMaterial.bEnabled)
		{
			strOutError =
				"Effect source Material template requires a staged profile.";
			return false;
		}
		return true;
	}


	void Write_Material(std::ostringstream& Output,
		const Client::EFFECT_MATERIAL_DESC& Material)
	{
		Output << "{ \"templateId\": \""
			<< Client::CDataJson::Escape(Material.strTemplateId)
			<< "\", \"sourceMaterialPath\": \""
			<< Client::CDataJson::Escape(Material.strSourceMaterialPath)
			<< "\", \"renderProfile\": \""
			<< Client::CEffectDocumentCodec::To_Token(Material.eRenderProfile)
			<< "\", \"sourceProfile\": ";
		Write_SourceMaterialProfile(Output, Material.SourceMaterial);
		if (Material.bColorTexturesSRGB)
			Output << ", \"colorTexturesSRGB\": true";
		if (Material.Execution.bEnabled || Material.Execution.bFailClosed)
		{
			Output << ", \"execution\": ";
			Write_MaterialExecution(Output, Material.Execution);
		}
		Output << " }";
	}


	bool_t Read_SourceMaterialSlots(const Client::DATA_JSON_VALUE& Mesh,
		std::vector<Client::EFFECT_SOURCE_MATERIAL_SLOT_DESC>& Out,
		std::string& strOutError)
	{
		const auto* pSlots = Mesh.Find("sourceMaterialSlots");
		if (nullptr == pSlots)
			return true;
		if (!pSlots->Is_Array() || pSlots->Get_Array().empty() ||
			pSlots->Get_Array().size() > 32u)
		{
			strOutError = "Mesh sourceMaterialSlots must contain 1 to 32 source slots.";
			return false;
		}
		std::vector<Client::EFFECT_SOURCE_MATERIAL_SLOT_DESC> Staged;
		std::unordered_set<uint32_t> Indices;
		for (const auto& Value : pSlots->Get_Array())
		{
			Client::EFFECT_SOURCE_MATERIAL_SLOT_DESC Slot;
			const auto* pIndex = Value.Find("sourceMaterialIndex");
			const auto* pMaterial = Value.Find("material");
			if (!Value.Is_Object() ||
				!Validate_ExactFields(Value, { "sourceMaterialIndex", "material" },
					"Mesh source material slot", strOutError) ||
				nullptr == pIndex || !pIndex->Is_Number() ||
				!std::isfinite(pIndex->Get_Number()) || pIndex->Get_Number() < 0.0 ||
				pIndex->Get_Number() > static_cast<double>(UINT32_MAX) ||
				std::floor(pIndex->Get_Number()) != pIndex->Get_Number() ||
				nullptr == pMaterial || !pMaterial->Is_Object())
			{
				strOutError = "Mesh source material slot/index/material is invalid.";
				return false;
			}
			Slot.iSourceMaterialIndex = static_cast<uint32_t>(pIndex->Get_Number());
			if (!Indices.insert(Slot.iSourceMaterialIndex).second ||
				!Read_Material(*pMaterial, Slot.Material,
					Client::EFFECT_AUTHORING_FORMAT_VERSION, false, strOutError))
			{
				if (strOutError.empty()) strOutError = "Mesh source material slot is duplicated.";
				return false;
			}
			Staged.push_back(std::move(Slot));
		}
		Out = std::move(Staged);
		return true;
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
			Read_OptionalFloat(*pTiming, "transformMotionDurationSeconds",
				Out.Timing.fTransformMotionDurationSeconds, strOutError) &&
			Read_Float(*pTiming, "afterImageSeconds", Out.Timing.fAfterImageSeconds, strOutError) &&
			Read_Float(*pTiming, "dissolveStartNormalized", Out.Timing.fDissolveStartNormalized, strOutError) &&
			Read_SourceMaterialSlots(*pMesh, Out.Mesh.SourceMaterialSlots, strOutError) &&
			Read_Bool(*pMesh, "useModelMaterial", Out.Mesh.bUseModelMaterial, strOutError) &&
			Read_OptionalFloat(*pMesh, "modelPreScale",
				Out.Mesh.fModelPreScale, strOutError) &&
			Read_OptionalArray(*pMesh, "sourceTypeDataRotationDegrees",
				&Out.Mesh.vSourceTypeDataRotationDegrees.x, 3u, strOutError) &&
			Read_MeshRingFill(*pMesh, Out.Mesh.RingFill, strOutError) &&
			Read_Bool(*pSprite, "billboard", Out.Sprite.bBillboard, strOutError) &&
			Read_OptionalFloat(*pSprite, "billboardRollDegrees",
				Out.Sprite.fBillboardRollDegrees, strOutError) &&
			Read_OptionalFloat(*pSprite, "billboardRollDegreesPerSecond",
				Out.Sprite.fBillboardRollDegreesPerSecond, strOutError) &&
			Read_LinearReveal(*pSprite, Out.Sprite.LinearReveal, strOutError) &&
			Read_Array(*pDecal, "size", &Out.Decal.vSize.x, 2u, strOutError) &&
			Read_Float(*pDecal, "depth", Out.Decal.fDepth, strOutError) &&
			Read_DecalReceiver(*pDecal, Out.Decal, strOutError);
	}


	/* These optional blocks are absent from documents written before they existed,
	   and absent has to keep meaning POINT/random/FIXED. They are read like
	   modelPreScale and the DynamicParameter triple: optional on the way in,
	   emitted on the way out only when they carry something other than the
	   historical default. */
	bool_t Read_ParticleSpawnShape(
		const Client::DATA_JSON_VALUE& Particle,
		Client::EFFECT_PARTICLE_SPAWN_SHAPE_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pShape = Particle.Find("spawnShape");
		if (nullptr == pShape)
			return true;
		const Client::DATA_JSON_VALUE* pKind = nullptr;
		if (!pShape->Is_Object() ||
			nullptr == (pKind = pShape->Find("kind")) || !pKind->Is_String() ||
			!Parse_Token(pKind->Get_String(), PARTICLE_SPAWN_SHAPE_TOKENS,
				std::size(PARTICLE_SPAWN_SHAPE_TOKENS), Out.eKind))
		{
			strOutError = "Effect particle spawnShape kind is invalid.";
			return false;
		}
		if (const Client::DATA_JSON_VALUE* pDistribution =
			pShape->Find("distribution"))
		{
			if (!pDistribution->Is_String() ||
				!Parse_Token(pDistribution->Get_String(),
					PARTICLE_SPAWN_DISTRIBUTION_TOKENS,
					std::size(PARTICLE_SPAWN_DISTRIBUTION_TOKENS),
					Out.eDistribution))
			{
				strOutError =
					"Effect particle spawnShape distribution is invalid.";
				return false;
			}
		}
		return Read_OptionalFloat(*pShape, "radius", Out.fRadius, strOutError) &&
			Read_OptionalFloat(*pShape, "innerRadius", Out.fInnerRadius,
				strOutError) &&
			Read_OptionalArray(*pShape, "extents", &Out.vExtents.x, 3u,
				strOutError) &&
			Read_OptionalFloat(*pShape, "arcDegrees", Out.fArcDegrees,
				strOutError);
	}


	bool_t Read_ParticleInitialOrientation(
		const Client::DATA_JSON_VALUE& Particle,
		Client::EFFECT_PARTICLE_INITIAL_ORIENTATION_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pOrientation =
			Particle.Find("initialOrientation");
		if (nullptr == pOrientation)
			return true;
		const Client::DATA_JSON_VALUE* pMode = nullptr;
		if (!pOrientation->Is_Object() ||
			nullptr == (pMode = pOrientation->Find("mode")) ||
			!pMode->Is_String() ||
			!Parse_Token(pMode->Get_String(), PARTICLE_ORIENTATION_MODE_TOKENS,
				std::size(PARTICLE_ORIENTATION_MODE_TOKENS), Out.eMode))
		{
			strOutError = "Effect particle initialOrientation mode is invalid.";
			return false;
		}
		return Read_Float(*pOrientation, "offsetDegrees", Out.fOffsetDegrees,
			strOutError);
	}


	bool_t Read_ParticleInitialVelocity(
		const Client::DATA_JSON_VALUE& Particle,
		Client::EFFECT_PARTICLE_INITIAL_VELOCITY_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pVelocity =
			Particle.Find("initialVelocity");
		if (nullptr == pVelocity)
			return true;
		const Client::DATA_JSON_VALUE* pMode = nullptr;
		if (!pVelocity->Is_Object() ||
			nullptr == (pMode = pVelocity->Find("mode")) || !pMode->Is_String() ||
			!Parse_Token(pMode->Get_String(), PARTICLE_VELOCITY_MODE_TOKENS,
				std::size(PARTICLE_VELOCITY_MODE_TOKENS), Out.eMode))
		{
			strOutError = "Effect particle initialVelocity mode is invalid.";
			return false;
		}
		return Read_OptionalArray(*pVelocity, "speed", &Out.vSpeedRange.x, 2u,
				strOutError) &&
			Read_OptionalFloat(*pVelocity, "coneAngleDegrees",
				Out.fConeAngleDegrees, strOutError) &&
			Read_OptionalBool(*pVelocity, "uniformSolidAngle",
				Out.bUniformSolidAngle, strOutError);
	}


	bool_t Read_ParticleTargetAttractor(
		const Client::DATA_JSON_VALUE& Particle,
		Client::EFFECT_PARTICLE_TARGET_ATTRACTOR_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pAttractor =
			Particle.Find("targetAttractor");
		if (nullptr == pAttractor)
			return true;
		const Client::DATA_JSON_VALUE* pTargetSpace = nullptr;
		if (!pAttractor->Is_Object() ||
			nullptr == (pTargetSpace = pAttractor->Find("targetSpace")) ||
			!pTargetSpace->Is_String() ||
			!Parse_Token(pTargetSpace->Get_String(),
				PARTICLE_ATTRACTOR_TARGET_SPACE_TOKENS,
				std::size(PARTICLE_ATTRACTOR_TARGET_SPACE_TOKENS),
				Out.eTargetSpace))
		{
			strOutError =
				"Effect particle targetAttractor targetSpace is invalid.";
			return false;
		}
		return Read_Bool(*pAttractor, "enabled", Out.bEnabled, strOutError) &&
			Read_Array(*pAttractor, "targetOffset", &Out.vTargetOffset.x, 3u,
				strOutError) &&
			Read_Array(*pAttractor, "activeNormalized",
				&Out.vActiveNormalized.x, 2u, strOutError) &&
			Read_Float(*pAttractor, "radialAcceleration",
				Out.fRadialAcceleration, strOutError) &&
			Read_Float(*pAttractor, "tangentialAcceleration",
				Out.fTangentialAcceleration, strOutError) &&
			Read_Float(*pAttractor, "maximumSpeed", Out.fMaximumSpeed,
				strOutError) &&
			Read_Float(*pAttractor, "convergenceRadius",
				Out.fConvergenceRadius, strOutError) &&
			Read_Float(*pAttractor, "arrivalDamping", Out.fArrivalDamping,
				strOutError);
	}


	bool_t Read_ParticleSourceScale(
		const Client::DATA_JSON_VALUE& Particle,
		Client::EFFECT_PARTICLE_SOURCE_SCALE_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pScale = Particle.Find("sourceScale");
		if (nullptr == pScale)
			return true;
		if (!pScale->Is_Object())
		{
			strOutError = "Effect particle sourceScale is not an object.";
			return false;
		}
		/* The four trailing factors were added after documents already existed
		   with the first three, so each one is optional and defaults to 1. */
		return Read_OptionalFloat(*pScale, "count", Out.fCount, strOutError) &&
			Read_OptionalFloat(*pScale, "size", Out.fSize, strOutError) &&
			Read_OptionalFloat(*pScale, "lifeTime", Out.fLifeTime, strOutError) &&
			Read_OptionalFloat(*pScale, "speed", Out.fSpeed, strOutError) &&
			Read_OptionalFloat(*pScale, "rotation", Out.fRotation,
				strOutError) &&
			Read_OptionalFloat(*pScale, "alpha", Out.fAlpha, strOutError) &&
			Read_OptionalFloat(*pScale, "spawnDelay", Out.fSpawnDelay,
				strOutError);
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
			Read_OptionalBool(*pLerp, "ringFillProgress",
				Out.LinearLerp.bRingFillProgress, strOutError) &&
			Read_OptionalFloat(*pLerp, "endRingFillProgress",
				Out.LinearLerp.fEndRingFillProgress, strOutError) &&
			Read_UInt(*pParticle, "maxParticles", Out.Particle.iMaxParticles, strOutError) &&
			Read_Float(*pParticle, "spawnRatePerSecond", Out.Particle.fSpawnRatePerSecond, strOutError) &&
			Read_OptionalFloat(*pParticle, "fixedCenterSpacingWorldUnits",
				Out.Particle.fFixedCenterSpacingWorldUnits, strOutError) &&
			Read_UInt(*pParticle, "burstCount", Out.Particle.iBurstCount, strOutError) &&
			Read_UInt(*pParticle, "randomSeed", Out.Particle.iRandomSeed, strOutError) &&
			Read_Array(*pParticle, "lifeTimeSeconds", &Out.Particle.vLifeTimeSeconds.x, 2u, strOutError) &&
			Read_OptionalArray(*pParticle, "initialPositionMin", &Out.Particle.vInitialPositionMin.x, 3u, strOutError) &&
			Read_OptionalArray(*pParticle, "initialPositionMax", &Out.Particle.vInitialPositionMax.x, 3u, strOutError) &&
			Read_Array(*pParticle, "initialVelocityMin", &Out.Particle.vInitialVelocityMin.x, 3u, strOutError) &&
			Read_Array(*pParticle, "initialVelocityMax", &Out.Particle.vInitialVelocityMax.x, 3u, strOutError) &&
			Read_Array(*pParticle, "acceleration", &Out.Particle.vAcceleration.x, 3u, strOutError) &&
			Read_Array(*pParticle, "startSize", &Out.Particle.vStartSize.x, 2u, strOutError) &&
			Read_Array(*pParticle, "endSize", &Out.Particle.vEndSize.x, 2u, strOutError) &&
			Read_Bool(*pParticle, "localSpace", Out.Particle.bLocalSpace, strOutError) &&
			Read_Bool(*pParticle, "billboard", Out.Particle.bBillboard, strOutError) &&
			Read_OptionalFloat(*pParticle, "drag", Out.Particle.fDrag,
				strOutError) &&
			Read_OptionalArray(*pParticle, "rotationRangeDegrees",
				&Out.Particle.vRotationRangeDegrees.x, 2u, strOutError) &&
			Read_OptionalArray(*pParticle, "spinRangeDegreesPerSecond",
				&Out.Particle.vSpinRangeDegreesPerSecond.x, 2u, strOutError) &&
			Read_OptionalBool(*pParticle, "subUVOverLife",
				Out.Particle.bSubUVOverLife, strOutError) &&
			Read_OptionalUInt(*pParticle, "dynamicParameterComponentMask",
				Out.Particle.iDynamicParameterComponentMask, strOutError) &&
			Read_OptionalArray(*pParticle, "dynamicParameterStart",
				&Out.Particle.vDynamicParameterStart.x, 4u, strOutError) &&
			Read_OptionalArray(*pParticle, "dynamicParameterEnd",
				&Out.Particle.vDynamicParameterEnd.x, 4u, strOutError) &&
			Read_ParticleSpawnShape(*pParticle, Out.Particle.SpawnShape,
				strOutError) &&
			Read_ParticleInitialOrientation(*pParticle,
				Out.Particle.InitialOrientation, strOutError) &&
			Read_ParticleInitialVelocity(*pParticle,
				Out.Particle.InitialVelocity, strOutError) &&
			Read_ParticleTargetAttractor(*pParticle,
				Out.Particle.TargetAttractor, strOutError) &&
			Read_ParticleSourceScale(*pParticle, Out.Particle.SourceScale,
				strOutError) &&
			Read_UInt(*pTrail, "maxPoints", Out.Trail.iMaxPoints, strOutError) &&
			Read_Float(*pTrail, "pointLifeTimeSeconds", Out.Trail.fPointLifeTimeSeconds, strOutError) &&
			Read_Float(*pTrail, "sampleIntervalSeconds", Out.Trail.fSampleIntervalSeconds, strOutError) &&
			Read_Float(*pTrail, "minimumDistance", Out.Trail.fMinimumDistance, strOutError) &&
			Read_Float(*pTrail, "startWidth", Out.Trail.fStartWidth, strOutError) &&
			Read_Float(*pTrail, "endWidth", Out.Trail.fEndWidth, strOutError) &&
			Read_OptionalFloat(*pTrail, "tilingDistanceWorldUnits",
				Out.Trail.fTilingDistanceWorldUnits, strOutError) &&
			Read_OptionalFloat(*pTrail, "distanceTessellationStepWorldUnits",
				Out.Trail.fDistanceTessellationStepWorldUnits, strOutError) &&
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
		Output << ", \"clip\": " << Detail.Color.fColorClip;
		Output
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
			<< ", \"lifeTimeSeconds\": " << Detail.Timing.fLifeTimeSeconds;
		/* Omission is the legacy identity: old documents remain byte-stable,
		   use Life for authored lerps, and keep velocity/Revolution on local time. */
		if (Detail.Timing.fTransformMotionDurationSeconds != 0.f)
		{
			Output << ", \"transformMotionDurationSeconds\": " <<
				Detail.Timing.fTransformMotionDurationSeconds;
		}
		Output << ", \"afterImageSeconds\": " << Detail.Timing.fAfterImageSeconds
			<< ", \"dissolveStartNormalized\": " << Detail.Timing.fDissolveStartNormalized
			<< " },\n        \"mesh\": { \"useModelMaterial\": " << (Detail.Mesh.bUseModelMaterial ? "true" : "false");
		/* Keep legacy v12/v13 typed-codec identities byte-stable. The optional
		   field is emitted only when an imported WModel carrier actually needs a
		   non-default scale such as Artist F's 0.01. */
		if (Detail.Mesh.fModelPreScale != 1.f)
			Output << ", \"modelPreScale\": " << Detail.Mesh.fModelPreScale;
		if (!Detail.Mesh.SourceMaterialSlots.empty())
		{
			Output << ", \"sourceMaterialSlots\": [";
			for (size_t i = 0u; i < Detail.Mesh.SourceMaterialSlots.size(); ++i)
			{
				const auto& Slot = Detail.Mesh.SourceMaterialSlots[i];
				Output << (i == 0u ? "" : ", ") << "{ \"sourceMaterialIndex\": "
					<< Slot.iSourceMaterialIndex << ", \"material\": ";
				Write_Material(Output, Slot.Material);
				Output << " }";
			}
			Output << "]";
		}
		Output << ", \"sourceTypeDataRotationDegrees\": ";
		Write_Float3(Output, Detail.Mesh.vSourceTypeDataRotationDegrees);
		if (Detail.Mesh.RingFill.bEnabled)
		{
			Output << ", \"ringFill\": { \"enabled\": true, \"progress\": "
				<< Detail.Mesh.RingFill.fProgress
				<< ", \"direction\": \""
				<< RING_FILL_DIRECTION_TOKENS[static_cast<size_t>(
					Detail.Mesh.RingFill.eDirection)]
				<< "\", \"feather\": " << Detail.Mesh.RingFill.fFeather
				<< ", \"invert\": "
				<< (Detail.Mesh.RingFill.bInvert ? "true" : "false")
				<< " }";
		}
		Output << " },\n        \"sprite\": { \"billboard\": " << (Detail.Sprite.bBillboard ? "true" : "false")
			<< ", \"billboardRollDegrees\": " << Detail.Sprite.fBillboardRollDegrees
			<< ", \"billboardRollDegreesPerSecond\": "
			<< Detail.Sprite.fBillboardRollDegreesPerSecond;
		if (Detail.Sprite.LinearReveal.bEnabled)
		{
			Output << ", \"linearReveal\": { \"enabled\": true, \"axis\": \""
				<< LINEAR_REVEAL_AXIS_TOKENS[static_cast<size_t>(
					Detail.Sprite.LinearReveal.eAxis)]
				<< "\", \"invert\": "
				<< (Detail.Sprite.LinearReveal.bInvert ? "true" : "false")
				<< ", \"startSeconds\": "
				<< Detail.Sprite.LinearReveal.fStartSeconds
				<< ", \"durationSeconds\": "
				<< Detail.Sprite.LinearReveal.fDurationSeconds
				<< ", \"edgeWidth\": " << Detail.Sprite.LinearReveal.fEdgeWidth
				<< ", \"softness\": " << Detail.Sprite.LinearReveal.fSoftness
				<< ", \"edgeColor\": ";
			Write_Float4(Output, Detail.Sprite.LinearReveal.vEdgeColor);
			Output << ", \"edgeEmissive\": "
				<< Detail.Sprite.LinearReveal.fEdgeEmissive << " }";
		}
		Output << " },\n        \"decal\": { \"size\": ";
		Write_Float2(Output, Detail.Decal.vSize);
		Output << ", \"depth\": " << Detail.Decal.fDepth;
		if (Detail.Decal.eReceiverMode !=
			EFFECT_DECAL_RECEIVER_MODE::ALL_OPAQUE)
		{
			Output << ", \"receiverMode\": \""
				<< DECAL_RECEIVER_MODE_TOKENS[static_cast<size_t>(
					Detail.Decal.eReceiverMode)]
				<< "\", \"normalCutoff\": "
				<< Detail.Decal.fNormalCutoff;
		}
		if (Detail.Decal.fEdgeFade != 0.f)
			Output << ", \"edgeFade\": " << Detail.Decal.fEdgeFade;
		Output << " },\n"
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
			<< ", \"endEmissiveIntensity\": " << Detail.LinearLerp.fEndEmissiveIntensity;
		if (Detail.LinearLerp.bRingFillProgress)
		{
			Output << ", \"ringFillProgress\": true, \"endRingFillProgress\": "
				<< Detail.LinearLerp.fEndRingFillProgress;
		}
		Output << " },\n"
			<< "        \"particle\": { \"maxParticles\": " << Detail.Particle.iMaxParticles
			<< ", \"spawnRatePerSecond\": " << Detail.Particle.fSpawnRatePerSecond;
		if (Detail.Particle.fFixedCenterSpacingWorldUnits > 0.f)
		{
			Output << ", \"fixedCenterSpacingWorldUnits\": "
				<< Detail.Particle.fFixedCenterSpacingWorldUnits;
		}
		Output << ", \"burstCount\": " << Detail.Particle.iBurstCount
			<< ", \"randomSeed\": " << Detail.Particle.iRandomSeed
			<< ", \"lifeTimeSeconds\": ";
		Write_Float2(Output, Detail.Particle.vLifeTimeSeconds);
		Output << ", \"initialPositionMin\": ";
		Write_Float3(Output, Detail.Particle.vInitialPositionMin);
		Output << ", \"initialPositionMax\": ";
		Write_Float3(Output, Detail.Particle.vInitialPositionMax);
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
			<< ", \"billboard\": " << (Detail.Particle.bBillboard ? "true" : "false");
		if (Detail.Particle.fDrag != 0.f)
			Output << ", \"drag\": " << Detail.Particle.fDrag;
		if (Detail.Particle.vRotationRangeDegrees.x != 0.f ||
			Detail.Particle.vRotationRangeDegrees.y != 0.f)
		{
			Output << ", \"rotationRangeDegrees\": ";
			Write_Float2(Output, Detail.Particle.vRotationRangeDegrees);
		}
		if (Detail.Particle.vSpinRangeDegreesPerSecond.x != 0.f ||
			Detail.Particle.vSpinRangeDegreesPerSecond.y != 0.f)
		{
			Output << ", \"spinRangeDegreesPerSecond\": ";
			Write_Float2(Output, Detail.Particle.vSpinRangeDegreesPerSecond);
		}
		if (Detail.Particle.bSubUVOverLife)
			Output << ", \"subUVOverLife\": true";
		/* Preserve the canonical typed-codec identity of existing v12/v13
		   documents. Dynamic Parameter authoring is optional and is emitted only
		   when at least one component is intentionally owned by the authored
		   particle. */
		if (0u != Detail.Particle.iDynamicParameterComponentMask)
		{
			Output << ", \"dynamicParameterComponentMask\": "
				<< Detail.Particle.iDynamicParameterComponentMask
				<< ", \"dynamicParameterStart\": ";
			Write_Float4(Output, Detail.Particle.vDynamicParameterStart);
			Output << ", \"dynamicParameterEnd\": ";
			Write_Float4(Output, Detail.Particle.vDynamicParameterEnd);
		}
		/* Same reason as modelPreScale above: a document that still spawns from a
		   point with random distribution and a fixed velocity/orientation must
		   serialize byte-identically to the legacy writer. */
		if (EFFECT_PARTICLE_SPAWN_SHAPE::POINT != Detail.Particle.SpawnShape.eKind)
		{
			Output << ", \"spawnShape\": { \"kind\": \""
				<< PARTICLE_SPAWN_SHAPE_TOKENS[
					static_cast<size_t>(Detail.Particle.SpawnShape.eKind)]
				<< "\", \"radius\": " << Detail.Particle.SpawnShape.fRadius
				<< ", \"innerRadius\": "
				<< Detail.Particle.SpawnShape.fInnerRadius
				<< ", \"extents\": ";
			Write_Float3(Output, Detail.Particle.SpawnShape.vExtents);
			Output << ", \"arcDegrees\": "
				<< Detail.Particle.SpawnShape.fArcDegrees;
			if (EFFECT_PARTICLE_SPAWN_DISTRIBUTION::RANDOM !=
				Detail.Particle.SpawnShape.eDistribution)
			{
				Output << ", \"distribution\": \""
					<< PARTICLE_SPAWN_DISTRIBUTION_TOKENS[static_cast<size_t>(
						Detail.Particle.SpawnShape.eDistribution)] << "\"";
			}
			Output << " }";
		}
		if (!Detail.Particle.InitialOrientation.Is_Default())
		{
			Output << ", \"initialOrientation\": { \"mode\": \""
				<< PARTICLE_ORIENTATION_MODE_TOKENS[static_cast<size_t>(
					Detail.Particle.InitialOrientation.eMode)]
				<< "\", \"offsetDegrees\": "
				<< Detail.Particle.InitialOrientation.fOffsetDegrees << " }";
		}
		if (EFFECT_PARTICLE_VELOCITY_MODE::FIXED !=
			Detail.Particle.InitialVelocity.eMode)
		{
			Output << ", \"initialVelocity\": { \"mode\": \""
				<< PARTICLE_VELOCITY_MODE_TOKENS[
					static_cast<size_t>(Detail.Particle.InitialVelocity.eMode)]
				<< "\", \"speed\": ";
			Write_Float2(Output, Detail.Particle.InitialVelocity.vSpeedRange);
			Output << ", \"coneAngleDegrees\": "
				<< Detail.Particle.InitialVelocity.fConeAngleDegrees;
			if (Detail.Particle.InitialVelocity.bUniformSolidAngle)
				Output << ", \"uniformSolidAngle\": true";
			Output << " }";
		}
		if (!Detail.Particle.TargetAttractor.Is_Default())
		{
			const EFFECT_PARTICLE_TARGET_ATTRACTOR_DESC& Attractor =
				Detail.Particle.TargetAttractor;
			Output << ", \"targetAttractor\": { \"enabled\": "
				<< (Attractor.bEnabled ? "true" : "false")
				<< ", \"targetSpace\": \""
				<< PARTICLE_ATTRACTOR_TARGET_SPACE_TOKENS[
					static_cast<size_t>(Attractor.eTargetSpace)]
				<< "\", \"targetOffset\": ";
			Write_Float3(Output, Attractor.vTargetOffset);
			Output << ", \"activeNormalized\": ";
			Write_Float2(Output, Attractor.vActiveNormalized);
			Output << ", \"radialAcceleration\": "
				<< Attractor.fRadialAcceleration
				<< ", \"tangentialAcceleration\": "
				<< Attractor.fTangentialAcceleration
				<< ", \"maximumSpeed\": " << Attractor.fMaximumSpeed
				<< ", \"convergenceRadius\": "
				<< Attractor.fConvergenceRadius
				<< ", \"arrivalDamping\": "
				<< Attractor.fArrivalDamping << " }";
		}
		/* Untouched trim is the overwhelming majority, and omitting it keeps
		   every document that predates the field byte-identical. */
		if (!Detail.Particle.SourceScale.Is_Default())
		{
			Output << ", \"sourceScale\": { \"count\": "
				<< Detail.Particle.SourceScale.fCount
				<< ", \"size\": " << Detail.Particle.SourceScale.fSize
				<< ", \"lifeTime\": " << Detail.Particle.SourceScale.fLifeTime
				<< ", \"speed\": " << Detail.Particle.SourceScale.fSpeed
				<< ", \"rotation\": " << Detail.Particle.SourceScale.fRotation
				<< ", \"alpha\": " << Detail.Particle.SourceScale.fAlpha
				<< ", \"spawnDelay\": "
				<< Detail.Particle.SourceScale.fSpawnDelay
				<< " }";
		}
		Output << " },\n"
			<< "        \"trail\": { \"maxPoints\": " << Detail.Trail.iMaxPoints
			<< ", \"pointLifeTimeSeconds\": " << Detail.Trail.fPointLifeTimeSeconds
			<< ", \"sampleIntervalSeconds\": " << Detail.Trail.fSampleIntervalSeconds
			<< ", \"minimumDistance\": " << Detail.Trail.fMinimumDistance
			<< ", \"startWidth\": " << Detail.Trail.fStartWidth
			<< ", \"endWidth\": " << Detail.Trail.fEndWidth
			<< ", \"tilingDistanceWorldUnits\": "
			<< Detail.Trail.fTilingDistanceWorldUnits
			<< ", \"distanceTessellationStepWorldUnits\": "
			<< Detail.Trail.fDistanceTessellationStepWorldUnits
			<< ", \"faceCamera\": " << (Detail.Trail.bFaceCamera ? "true" : "false") << " },\n"
			<< "        \"afterImage\": { \"sampleIntervalSeconds\": " << Detail.AfterImage.fSampleIntervalSeconds
			<< ", \"maxCopies\": " << Detail.AfterImage.iMaxCopies
			<< ", \"alphaExponent\": " << Detail.AfterImage.fAlphaExponent
			<< " },\n";
		Write_PresentationDetail(Output, Detail);
		Output << "      }";
	}

}
