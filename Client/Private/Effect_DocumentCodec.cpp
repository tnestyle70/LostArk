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


const char_t* Client::CEffectDocumentCodec::To_Token(
	const EFFECT_MATERIAL_EXECUTION_BACKEND eBackend)
{
	return eBackend < EFFECT_MATERIAL_EXECUTION_BACKEND::END ?
		MATERIAL_EXECUTION_BACKEND_TOKENS[static_cast<size_t>(eBackend)] :
		"invalid";
}


bool_t Client::CEffectDocumentCodec::Is_ResourceSlotAllowed(
	const EFFECT_ELEMENT_KIND eKind,
	const EFFECT_RESOURCE_SLOT eSlot)
{
	if (eKind >= EFFECT_ELEMENT_KIND::END || eSlot >= EFFECT_RESOURCE_SLOT::END)
		return false;
	if (EFFECT_RESOURCE_SLOT::MESH_MODEL == eSlot)
		return EFFECT_ELEMENT_KIND::MESH == eKind ||
			EFFECT_ELEMENT_KIND::PARTICLE == eKind;
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


bool_t Client::CEffectDocumentCodec::Is_SafeElementResourceAssetId(
	const EFFECT_ELEMENT_KIND eElementKind,
	const std::string_view strSlotId,
	const std::string& strAssetId,
	EFFECT_RESOURCE_FILE_KIND* pOutKind)
{
	EFFECT_RESOURCE_FILE_KIND eKind = EFFECT_RESOURCE_FILE_KIND::END;
	if (strSlotId == EFFECT_MESH_SHAPE_SLOT_ID)
	{
		if (!Is_ResourceSlotAllowed(
				eElementKind, EFFECT_RESOURCE_SLOT::MESH_MODEL))
		{
			return false;
		}
		if (0u == strAssetId.rfind("Character/", 0u))
		{
			if (!Is_SafeModelCueAssetIdInternal(strAssetId))
				return false;
			eKind = EFFECT_RESOURCE_FILE_KIND::MODEL;
		}
		else if (!Is_SafeResourceAssetId(strAssetId, &eKind) ||
			eKind != EFFECT_RESOURCE_FILE_KIND::MODEL)
		{
			return false;
		}
	}
	else if (!Is_SafeResourceAssetId(strAssetId, &eKind) ||
		eKind != EFFECT_RESOURCE_FILE_KIND::TEXTURE)
	{
		return false;
	}

	if (nullptr != pOutKind)
		*pOutKind = eKind;
	return true;
}


bool_t Client::CEffectDocumentCodec::Is_SafeModelCueAssetId(
	const std::string& strAssetId)
{
	return Is_SafeModelCueAssetIdInternal(strAssetId);
}


bool_t Client::CEffectDocumentCodec::Parse(
	const std::string_view Json,
	EFFECT_DOCUMENT_DESC& OutDocument,
	std::string& strOutError)
{
	DATA_JSON_VALUE Root;
	DATA_JSON_PARSE_LIMITS EffectDocumentLimits;
	EffectDocumentLimits.iMaximumBytes = MAXIMUM_DOCUMENT_BYTES;
	EffectDocumentLimits.iMaximumDepth = 64u;
	EffectDocumentLimits.iMaximumValues = 3'000'000u;
	if (!CDataJson::Parse(Json, Root, strOutError, EffectDocumentLimits))
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
	const DATA_JSON_VALUE* pPurpose = Root.Find("purpose");
	const DATA_JSON_VALUE* pRuntimeExtensions = Root.Find("runtimeExtensions");
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
		Version > EFFECT_AUTHORING_MAX_SUPPORTED_VERSION)
	{
		strOutError = "Effect document version is not supported.";
		return false;
	}
	const uint32_t iSourceVersion = static_cast<uint32_t>(Version);
	const bool_t bSourceContract =
		iSourceVersion == EFFECT_SOURCE_CONTRACT_FORMAT_VERSION;
	const bool_t bRuntimeExtensionDocument =
		iSourceVersion == EFFECT_AUTHORED_RUNTIME_EXTENSION_FORMAT_VERSION;
	if (bSourceContract)
	{
		if (nullptr == pSchema || !pSchema->Is_String() ||
			pSchema->Get_String() != EFFECT_DOCUMENT_SCHEMA ||
			nullptr == pPurpose || !pPurpose->Is_String() ||
			pPurpose->Get_String() != "source_contract" ||
			!Validate_ExactFields(Root,
				{ "schema", "version", "purpose", "effectAssetId",
					"displayName", "bloomIntensity", "particleSystem", "modelCues", "elements" },
				"Effect source-contract document", strOutError))
		{
			if (strOutError.empty())
				strOutError = "Native-v14 source-contract root is invalid.";
			return false;
		}
	}
	else if (bRuntimeExtensionDocument)
	{
		if (nullptr == pSchema || !pSchema->Is_String() ||
			pSchema->Get_String() != EFFECT_DOCUMENT_SCHEMA ||
			nullptr != pPurpose || nullptr == pRuntimeExtensions ||
			!pRuntimeExtensions->Is_Object() ||
			!Validate_ExactFields(Root,
				{ "schema", "version", "effectAssetId", "displayName", "bloomIntensity",
					"particleSystem", "modelCues", "sourceModelPreview", "runtimeExtensions",
					"elements" },
				"Effect authored-v15 document", strOutError))
		{
			if (strOutError.empty())
				strOutError = "Effect authored-v15 root is invalid.";
			return false;
		}
	}
	else if (nullptr != pPurpose)
	{
		strOutError = "Legacy Effect documents cannot declare source-contract purpose.";
		return false;
	}
	else if (nullptr != pRuntimeExtensions)
	{
		strOutError =
			"Effect runtimeExtensions require authored document version 15.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Staged;
	Staged.iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	Staged.iLoadedFormatVersion = iSourceVersion;
	Staged.bSourceContract = bSourceContract;
	Staged.strEffectAssetId = pAssetId->Get_String();
	Staged.strDisplayName = pDisplayName->Get_String();
	if (Root.Find("bloomIntensity") &&
		(!Read_Float(Root, "bloomIntensity", Staged.fBloomIntensity, strOutError) ||
		 !Is_ValidEffectBloomIntensity(Staged.fBloomIntensity)))
	{
		strOutError = "Effect bloomIntensity must be finite and between 0 and 16.";
		return false;
	}
	if (bRuntimeExtensionDocument &&
		!Read_AuthoredRuntimeExtensions(
			*pRuntimeExtensions, Staged.RuntimeExtensions, strOutError))
	{
		return false;
	}
	if (iSourceVersion >= 8u)
	{
		const DATA_JSON_VALUE* pParticleSystem = Root.Find("particleSystem");
		if (nullptr == pParticleSystem || !pParticleSystem->Is_Object() ||
			(bSourceContract && !Validate_ExactFields(*pParticleSystem,
				{ "uniformScaleMultiplier", "yawOffsetDegrees",
					"directionYawDegrees", "initialSpeedMultiplier" },
				"Effect source-contract particleSystem", strOutError)) ||
			!Read_Float(*pParticleSystem, "uniformScaleMultiplier",
				Staged.ParticleSystem.fUniformScaleMultiplier, strOutError) ||
			!Read_Float(*pParticleSystem, "yawOffsetDegrees",
				Staged.ParticleSystem.fYawOffsetDegrees, strOutError) ||
			!Read_Float(*pParticleSystem, "directionYawDegrees",
				Staged.ParticleSystem.fDirectionYawDegrees, strOutError) ||
			!Read_Float(*pParticleSystem, "initialSpeedMultiplier",
				Staged.ParticleSystem.fInitialSpeedMultiplier, strOutError))
		{
			if (strOutError.empty())
				strOutError = "Effect particleSystem fields are invalid.";
			return false;
		}
	}
    if (const auto* value = Root.Find("sourceModelPreview"))
    {
        EFFECT_SOURCE_MODEL_PREVIEW preview;
        if (!value->Is_Object() ||
            !Validate_ExactFields(*value, {"gateId", "actorProfileId", "targetBossPlacementId", "animations"},
                "Effect source model preview", strOutError) ||
            !Read_String(*value, "gateId", preview.strGateId, strOutError) ||
            !Read_String(*value, "actorProfileId", preview.strActorProfileId, strOutError) ||
            !Read_String(*value, "targetBossPlacementId", preview.strTargetBossPlacementId, strOutError)) return false;
        const auto* animations = value->Find("animations");
        if (!animations || !animations->Is_Array() || animations->Get_Array().empty() || animations->Get_Array().size() > 256u)
        { strOutError = "Effect source model preview needs 1-256 animation windows."; return false; }
        for (const auto& entry : animations->Get_Array())
        {
            EFFECT_SOURCE_MODEL_ANIMATION animation;
            if (!entry.Is_Object() || !Validate_ExactFields(entry,
                {"runtimeClip", "startOffsetMs", "sourceStartMs", "playMs", "playRate", "endPolicy"},
                "Effect source model animation", strOutError) ||
                !Read_String(entry, "runtimeClip", animation.strRuntimeClip, strOutError) ||
                !Read_UInt(entry, "startOffsetMs", animation.iStartOffsetMs, strOutError) ||
                !Read_UInt(entry, "sourceStartMs", animation.iSourceStartMs, strOutError) ||
                !Read_UInt(entry, "playMs", animation.iPlayMs, strOutError) ||
                !Read_Float(entry, "playRate", animation.fPlayRate, strOutError) ||
                !Read_String(entry, "endPolicy", animation.strEndPolicy, strOutError)) return false;
            preview.Animations.push_back(std::move(animation));
        }
        Staged.SourceModelPreview = std::move(preview);
    }
	const DATA_JSON_VALUE* pModelCues = Root.Find("modelCues");
	if (nullptr != pModelCues)
	{
		if (!pModelCues->Is_Array())
		{
			strOutError = "Effect modelCues must be an array.";
			return false;
		}
		Staged.ModelCues.reserve(pModelCues->Get_Array().size());
		for (const DATA_JSON_VALUE& CueValue : pModelCues->Get_Array())
		{
			if (!CueValue.Is_Object() ||
				(bSourceContract && !Validate_ExactFields(CueValue,
					{ "cueId", "modelAssetId", "clipName",
						"startDelaySeconds", "durationSeconds", "alphaMode",
						"opacity", "colorMultiply", "holdLastFrame", "loop", "visible",
						"suppressHorizontalRootMotionBone",
						"localTransform", "assetPreTransform", "material" },
					"Effect source-contract Model Cue", strOutError)))
			{
				strOutError = "Effect Model Cue must be an object.";
				return false;
			}
			const DATA_JSON_VALUE* pCueId = CueValue.Find("cueId");
			const DATA_JSON_VALUE* pModelAssetId =
				CueValue.Find("modelAssetId");
			const DATA_JSON_VALUE* pClipName = CueValue.Find("clipName");
			const DATA_JSON_VALUE* pAlphaMode = CueValue.Find("alphaMode");
			const DATA_JSON_VALUE* pVisible = CueValue.Find("visible");
			EFFECT_MODEL_CUE_DESC Cue;
			if (nullptr == pCueId || !pCueId->Is_String() ||
				nullptr == pModelAssetId || !pModelAssetId->Is_String() ||
				nullptr == pClipName || !pClipName->Is_String() ||
				nullptr == pVisible || !pVisible->Is_Boolean() ||
				!Read_Float(CueValue, "startDelaySeconds",
					Cue.fStartDelaySeconds, strOutError) ||
				!Read_Float(CueValue, "durationSeconds",
					Cue.fDurationSeconds, strOutError) ||
				!Read_OptionalFloat(CueValue, "opacity", Cue.fOpacity,
					strOutError) ||
				!Read_OptionalArray(CueValue, "colorMultiply",
					&Cue.vColorMultiply.x, 4u, strOutError) ||
				!Read_OptionalBool(CueValue, "holdLastFrame",
					Cue.bHoldLastFrame, strOutError) ||
				!Read_OptionalBool(CueValue, "loop", Cue.bLoop, strOutError) ||
				(CueValue.Find("suppressHorizontalRootMotionBone") &&
				 !Read_String(CueValue, "suppressHorizontalRootMotionBone",
					 Cue.strSuppressHorizontalRootMotionBone, strOutError)) ||
				!Read_ModelCueTransform(CueValue, Cue, strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect Model Cue fields are invalid.";
				return false;
			}
			Cue.strCueId = pCueId->Get_String();
			Cue.strModelAssetId = pModelAssetId->Get_String();
			Cue.strClipName = pClipName->Get_String();
			if (nullptr != pAlphaMode &&
				(!pAlphaMode->Is_String() ||
				 !Parse_Token(pAlphaMode->Get_String(),
					 MODEL_CUE_ALPHA_MODE_TOKENS,
					 std::size(MODEL_CUE_ALPHA_MODE_TOKENS),
					 Cue.eAlphaMode)))
			{
				strOutError = "Effect Model Cue alphaMode is invalid.";
				return false;
			}
			if (const DATA_JSON_VALUE* pMaterial = CueValue.Find("material"))
			{
				EFFECT_MATERIAL_DESC Material;
				if (!pMaterial->Is_Object() || !Read_Material(*pMaterial, Material,
					Staged.iLoadedFormatVersion, bSourceContract, strOutError))
					return false;
				Cue.Material = std::move(Material);
			}
			Cue.bVisible = pVisible->Get_Boolean();
			Staged.ModelCues.push_back(std::move(Cue));
		}
	}
	Staged.Elements.reserve(pElements->Get_Array().size());
	for (const DATA_JSON_VALUE& ElementValue : pElements->Get_Array())
	{
		if (!bSourceContract && ElementValue.Is_Object() &&
			nullptr != ElementValue.Find("renderer"))
		{
			strOutError =
				"Legacy Effect element contains native-v14 renderer evidence.";
			return false;
		}
		if (!ElementValue.Is_Object() ||
			(bSourceContract && !Validate_ExactFields(ElementValue,
				{ "id", "displayName", "groupId", "sourceNode", "visible",
					"kind", "renderer", "resources", "unboundResources",
					"material",
					"actionCueAttachment", "transformInheritance", "sourceTransformTrack", "detail",
					"sourceRecipe", "sourcePresentation" },
				"Effect source-contract Element", strOutError)) ||
			(bRuntimeExtensionDocument && !Validate_ExactFields(ElementValue,
				{ "id", "displayName", "groupId", "sourceNode", "visible",
					"kind", "runtimeCarrier", "compositionLayer", "resources",
					"unboundResources", "material", "actionCueAttachment",
					"transformInheritance", "sourceTransformTrack", "detail", "sourceRecipe",
					"sourcePresentation", "authoringOverrides" },
				"Effect authored-v15 Element", strOutError)))
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
		if (const DATA_JSON_VALUE* pCompositionLayer =
			ElementValue.Find("compositionLayer"))
		{
			if (!pCompositionLayer->Is_String() ||
				!Parse_Token(pCompositionLayer->Get_String(),
					COMPOSITION_LAYER_TOKENS,
					std::size(COMPOSITION_LAYER_TOKENS),
					Element.eCompositionLayer))
			{
				strOutError = "Effect Element compositionLayer is invalid.";
				return false;
			}
		}
		Element.strElementId = pId->Get_String();
		if (const DATA_JSON_VALUE* pRuntimeCarrier =
			ElementValue.Find("runtimeCarrier"))
		{
			if (!bRuntimeExtensionDocument || !pRuntimeCarrier->Is_Object() ||
				!Read_AuthoredRuntimeCarrier(
					*pRuntimeCarrier, Element.RuntimeCarrier, strOutError))
			{
				if (strOutError.empty())
					strOutError =
						"Effect runtimeCarrier is valid only in authored-v15 documents.";
				return false;
			}
		}
		if (bSourceContract)
		{
			const DATA_JSON_VALUE* pRenderer = ElementValue.Find("renderer");
			if (nullptr == pRenderer || !pRenderer->Is_Object() ||
				!Read_Renderer(*pRenderer, Element.Renderer, strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect source-contract renderer is missing.";
				return false;
			}
		}
		if (iSourceVersion >= 6u)
		{
			const DATA_JSON_VALUE* pElementDisplayName =
				ElementValue.Find("displayName");
			const DATA_JSON_VALUE* pGroupId = ElementValue.Find("groupId");
			const DATA_JSON_VALUE* pSourceNode = ElementValue.Find("sourceNode");
			const DATA_JSON_VALUE* pVisible = ElementValue.Find("visible");
			if (nullptr == pElementDisplayName ||
				!pElementDisplayName->Is_String() ||
				nullptr == pGroupId || !pGroupId->Is_String() ||
				nullptr == pSourceNode || !pSourceNode->Is_String() ||
				nullptr == pVisible || !pVisible->Is_Boolean())
			{
				strOutError = "Effect Element metadata is invalid.";
				return false;
			}
			Element.strDisplayName = pElementDisplayName->Get_String();
			Element.strGroupId = pGroupId->Get_String();
			Element.strSourceNode = pSourceNode->Get_String();
			Element.bVisible = pVisible->Get_Boolean();
		}
		else
		{
			Element.strDisplayName = Element.strElementId;
		}
        if (const auto* TrackValue = ElementValue.Find("sourceTransformTrack"))
        {
            EFFECT_SOURCE_TRANSFORM_TRACK Track;
            if (!Read_SourceTransformTrack(*TrackValue, Track, strOutError)) return false;
            Element.SourceTransformTrack = std::move(Track);
        }
		if (const DATA_JSON_VALUE* pActionCueAttachment =
			ElementValue.Find("actionCueAttachment"))
		{
			if (!pActionCueAttachment->Is_Object() ||
				!Read_ActionCueAttachment(*pActionCueAttachment,
					Element.ActionCueAttachment, strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect Action cue attachment is invalid.";
				return false;
			}
		}
		const DATA_JSON_VALUE* pTransformInheritance =
			ElementValue.Find("transformInheritance");
		if ((iSourceVersion >= 13u && nullptr == pTransformInheritance) ||
			(nullptr != pTransformInheritance &&
				(!pTransformInheritance->Is_Object() ||
					!Read_TransformInheritance(*pTransformInheritance,
						Element.TransformInheritance, strOutError))))
		{
			if (strOutError.empty())
				strOutError = "Effect transform inheritance is invalid.";
			return false;
		}
		for (const DATA_JSON_VALUE& ResourceValue : pResources->Get_Array())
		{
			if (!ResourceValue.Is_Object() ||
				(bSourceContract && !Validate_ExactFields(ResourceValue,
					{ "slotId", "assetId" },
					"Effect source-contract resource", strOutError)))
			{
				strOutError = "Effect resource must be an object.";
				return false;
			}
			const DATA_JSON_VALUE* pSlot = ResourceValue.Find(
				iSourceVersion >= 6u ? "slotId" : "slot");
			const DATA_JSON_VALUE* pResourceId = ResourceValue.Find("assetId");
			EFFECT_RESOURCE_BINDING_DESC Binding;
			if (nullptr == pSlot || !pSlot->Is_String() ||
				nullptr == pResourceId || !pResourceId->Is_String())
			{
				strOutError = "Effect resource binding is invalid.";
				return false;
			}
			Binding.strSlotId = pSlot->Get_String();
			Binding.strAssetId = pResourceId->Get_String();
			Element.ResourceBindings.push_back(std::move(Binding));
		}
		/* Optional: the seeder records every source texture that did not fit a
		   slot so the document keeps the full original reference list. */
		if (const DATA_JSON_VALUE* pUnbound =
			ElementValue.Find("unboundResources"))
		{
			if (!pUnbound->Is_Array())
			{
				strOutError = "Effect unboundResources must be an array.";
				return false;
			}
			for (const DATA_JSON_VALUE& UnboundValue : pUnbound->Get_Array())
			{
				if (!UnboundValue.Is_String())
				{
					strOutError =
						"Effect unboundResources entry must be a string.";
					return false;
				}
				Element.UnboundSourceResources.push_back(
					UnboundValue.Get_String());
			}
		}
		if (!Read_Material(*pMaterial, Element.Material,
			iSourceVersion, bSourceContract, strOutError))
			return false;
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
			if (iSourceVersion >= 12u &&
				!Read_PresentationDetail(*pDetail, Element.Detail, strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect presentation Detail is invalid.";
				return false;
			}
		}
		if (iSourceVersion >= 9u)
		{
			const DATA_JSON_VALUE* pSourceRecipe =
				ElementValue.Find("sourceRecipe");
			if (nullptr == pSourceRecipe || !pSourceRecipe->Is_Object() ||
				!Read_SourceRecipe(*pSourceRecipe, Element.SourceRecipe,
					bSourceContract,
					strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect source recipe is invalid.";
				return false;
			}
		}
		if (iSourceVersion >= 12u)
		{
			const DATA_JSON_VALUE* pSourcePresentation =
				ElementValue.Find("sourcePresentation");
			if (nullptr == pSourcePresentation ||
				!pSourcePresentation->Is_Object() ||
				!Read_SourcePresentation(*pSourcePresentation,
					Element.SourcePresentation, strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect source presentation is invalid.";
				return false;
			}
		}
		const DATA_JSON_VALUE* pAuthoringOverrides =
			ElementValue.Find("authoringOverrides");
		if (nullptr != pAuthoringOverrides)
		{
			if (bSourceContract)
			{
				strOutError =
					"Effect source-contract Element cannot carry authoring overrides.";
				return false;
			}
			if (!pAuthoringOverrides->Is_Object() ||
				!Read_AuthoringOverrides(*pAuthoringOverrides, Element,
					strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect authoring overrides are invalid.";
				return false;
			}
		}
		Staged.Elements.push_back(std::move(Element));
	}
	if (!bSourceContract &&
		!Apply_Warlord17090RetainedSourceProjection(Staged, strOutError))
	{
		return false;
	}
	if (!(bSourceContract ? Validate_SourceContract(Staged, strOutError) :
		Validate(Staged, strOutError)))
		return false;
	OutDocument = std::move(Staged);
	strOutError.clear();
	return true;
}


std::string Client::CEffectDocumentCodec::Serialize(
	const EFFECT_DOCUMENT_DESC& Document)
{
	const uint32_t iSerializedVersion = Document.iLoadedFormatVersion;
	const bool_t bSourceContract =
		iSerializedVersion == EFFECT_SOURCE_CONTRACT_FORMAT_VERSION &&
		Document.bSourceContract;
	std::ostringstream Output;
	Output << std::setprecision(9) << "{\n"
		<< "  \"schema\": \"" << EFFECT_DOCUMENT_SCHEMA << "\",\n"
		<< "  \"version\": " << iSerializedVersion << ",\n";
	if (bSourceContract)
		Output << "  \"purpose\": \"source_contract\",\n";
    if (Document.SourceModelPreview)
    {
        const auto& preview = *Document.SourceModelPreview;
        Output << "  \"sourceModelPreview\": {\"gateId\": \"" << CDataJson::Escape(preview.strGateId)
            << "\", \"actorProfileId\": \"" << CDataJson::Escape(preview.strActorProfileId)
            << "\", \"targetBossPlacementId\": \"" << CDataJson::Escape(preview.strTargetBossPlacementId) << "\", \"animations\": [";
        for (size_t i = 0u; i < preview.Animations.size(); ++i)
        {
            const auto& animation = preview.Animations[i];
            Output << (i ? ", " : "") << "{\"runtimeClip\": \"" << CDataJson::Escape(animation.strRuntimeClip)
                << "\", \"startOffsetMs\": " << animation.iStartOffsetMs << ", \"sourceStartMs\": " << animation.iSourceStartMs
                << ", \"playMs\": " << animation.iPlayMs << ", \"playRate\": " << animation.fPlayRate
                << ", \"endPolicy\": \"" << CDataJson::Escape(animation.strEndPolicy) << "\"}";
        }
        Output << "]},\n";
    }
	Output << "  \"effectAssetId\": \"" << CDataJson::Escape(Document.strEffectAssetId) << "\",\n"
		<< "  \"displayName\": \"" << CDataJson::Escape(Document.strDisplayName) << "\",\n"
		<< "  \"bloomIntensity\": " << Document.fBloomIntensity << ",\n"
		<< "  \"particleSystem\": { \"uniformScaleMultiplier\": "
		<< Document.ParticleSystem.fUniformScaleMultiplier
		<< ", \"yawOffsetDegrees\": "
		<< Document.ParticleSystem.fYawOffsetDegrees
		<< ", \"directionYawDegrees\": "
		<< Document.ParticleSystem.fDirectionYawDegrees
		<< ", \"initialSpeedMultiplier\": "
		<< Document.ParticleSystem.fInitialSpeedMultiplier << " },\n"
		<< "  \"modelCues\": [";
	for (size_t iCue = 0u; iCue < Document.ModelCues.size(); ++iCue)
	{
		const EFFECT_MODEL_CUE_DESC& Cue = Document.ModelCues[iCue];
		Output << (0u == iCue ? "\n" : ",\n")
			<< "    { \"cueId\": \"" << CDataJson::Escape(Cue.strCueId)
			<< "\", \"modelAssetId\": \""
			<< CDataJson::Escape(Cue.strModelAssetId)
			<< "\", \"clipName\": \""
			<< CDataJson::Escape(Cue.strClipName)
			<< "\", \"startDelaySeconds\": " << Cue.fStartDelaySeconds
			<< ", \"durationSeconds\": " << Cue.fDurationSeconds
			<< ", \"opacity\": " << Cue.fOpacity
			<< ", \"colorMultiply\": ";
		Write_Float4(Output, Cue.vColorMultiply);
		if (!Cue.strSuppressHorizontalRootMotionBone.empty())
			Output << ", \"suppressHorizontalRootMotionBone\": \""
				<< CDataJson::Escape(Cue.strSuppressHorizontalRootMotionBone) << "\"";
		Output << ", \"holdLastFrame\": "
			<< (Cue.bHoldLastFrame ? "true" : "false")
			<< ", \"loop\": " << (Cue.bLoop ? "true" : "false")
			<< ", \"alphaMode\": \""
			<< MODEL_CUE_ALPHA_MODE_TOKENS[static_cast<size_t>(Cue.eAlphaMode)]
			<< "\""
			<< ", \"visible\": " << (Cue.bVisible ? "true" : "false")
			<< ",\n      \"localTransform\": { \"position\": ";
		Write_Float3(Output, Cue.LocalTransform.vPosition);
		Output << ", \"rotationDegrees\": ";
		Write_Float3(Output, Cue.LocalTransform.vRotationDegrees);
		Output << ", \"revolutionDegreesPerSecond\": ";
		Write_Float3(Output, Cue.LocalTransform.vRevolutionDegreesPerSecond);
		Output << ", \"scale\": ";
		Write_Float3(Output, Cue.LocalTransform.vScale);
		Output << ", \"velocityPerSecond\": ";
		Write_Float3(Output, Cue.LocalTransform.vVelocityPerSecond);
		Output << " },\n      \"assetPreTransform\": { \"scale\": ";
		Write_Float3(Output, Cue.vAssetPreScale);
		Output << ", \"rotationDegrees\": ";
		Write_Float3(Output, Cue.vAssetPreRotationDegrees);
		Output << " }";
		if (Cue.Material)
		{
			Output << ", \"material\": ";
			Write_Material(Output, *Cue.Material);
		}
		Output << " }";
	}
	if (!Document.ModelCues.empty())
		Output << '\n';
	Output << "  ],\n";
	if (iSerializedVersion ==
		EFFECT_AUTHORED_RUNTIME_EXTENSION_FORMAT_VERSION)
	{
		Output << "  \"runtimeExtensions\": {\n"
			<< "    \"formatVersion\": "
			<< Document.RuntimeExtensions.iFormatVersion << ",\n"
			<< "    \"bakedEdgeHistories\": [";
		for (size_t iHistory = 0u;
			iHistory < Document.RuntimeExtensions.BakedEdgeHistories.size();
			++iHistory)
		{
			const EFFECT_AUTHORED_RUNTIME_EDGE_HISTORY_DESC& History =
				Document.RuntimeExtensions.BakedEdgeHistories[iHistory];
			Output << (0u == iHistory ? "\n" : ",\n")
				<< "      {\n"
				<< "        \"historyId\": \""
				<< CDataJson::Escape(History.strHistoryId) << "\",\n"
				<< "        \"coordinateBasis\": \""
				<< AUTHORED_RUNTIME_COORDINATE_BASIS_TOKENS[
					static_cast<size_t>(History.eCoordinateBasis)] << "\",\n"
				<< "        \"sourceEndTimeSeconds\": "
				<< History.fSourceEndTimeSeconds << ",\n"
				<< "        \"playbackClampSeconds\": "
				<< History.fPlaybackClampSeconds << ",\n"
				<< "        \"samples\": [";
			for (size_t iSample = 0u; iSample < History.Samples.size();
				++iSample)
			{
				const EFFECT_AUTHORED_RUNTIME_EDGE_SAMPLE_DESC& Sample =
					History.Samples[iSample];
				Output << (0u == iSample ? "\n" : ",\n")
					<< "          { \"relativeTimeSeconds\": "
					<< Sample.fRelativeTimeSeconds
					<< ", \"firstEdgeUE3Cm\": ";
				Write_Float3(Output, Sample.vFirstEdgeUE3Cm);
				Output << ", \"controlPointUE3Cm\": ";
				Write_Float3(Output, Sample.vControlPointUE3Cm);
				Output << ", \"secondEdgeUE3Cm\": ";
				Write_Float3(Output, Sample.vSecondEdgeUE3Cm);
				Output << " }";
			}
			if (!History.Samples.empty())
				Output << '\n';
			Output << "        ]\n      }";
		}
		if (!Document.RuntimeExtensions.BakedEdgeHistories.empty())
			Output << '\n';
		Output << "    ]\n  },\n";
	}
	Output << "  \"elements\": [";
	for (size_t iElement = 0u; iElement < Document.Elements.size(); ++iElement)
	{
		const EFFECT_ELEMENT_DESC& Element = Document.Elements[iElement];
		Output << (0u == iElement ? "\n" : ",\n")
			<< "    {\n      \"id\": \"" << CDataJson::Escape(Element.strElementId) << "\",\n"
			<< "      \"displayName\": \"" << CDataJson::Escape(Element.strDisplayName) << "\",\n"
			<< "      \"groupId\": \"" << CDataJson::Escape(Element.strGroupId) << "\",\n"
			<< "      \"sourceNode\": \"" << CDataJson::Escape(Element.strSourceNode) << "\",\n"
			<< "      \"visible\": " << (Element.bVisible ? "true" : "false") << ",\n"
			<< "      \"kind\": \"" << To_Token(Element.eKind) << "\",\n";
		if (iSerializedVersion ==
				EFFECT_AUTHORED_RUNTIME_EXTENSION_FORMAT_VERSION &&
			!Element.RuntimeCarrier.Is_Empty())
		{
			const EFFECT_AUTHORED_RUNTIME_CARRIER_DESC& Carrier =
				Element.RuntimeCarrier;
			Output << "      \"runtimeCarrier\": { \"formatVersion\": "
				<< Carrier.iFormatVersion << ", \"kind\": \""
				<< AUTHORED_RUNTIME_CARRIER_KIND_TOKENS[
					static_cast<size_t>(Carrier.eKind)]
				<< "\", \"admission\": \""
				<< AUTHORED_RUNTIME_CARRIER_ADMISSION_TOKENS[
					static_cast<size_t>(Carrier.eAdmission)] << "\"";
			switch (Carrier.eKind)
			{
			case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_RIBBON_V1:
			case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_BEAM_V1:
				Output << ", \"typeDataModuleStableId\": \""
					<< CDataJson::Escape(Carrier.strTypeDataModuleStableId)
					<< "\"";
				break;
			case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::
				ANIMATION_TRAIL_BAKED_EDGE_V1:
				Output << ", \"historyId\": \""
					<< CDataJson::Escape(Carrier.strHistoryId) << "\"";
				break;
			case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::
				LIGHT_BAKED_EDGE_ATTACHMENT_V1:
				Output << ", \"historyId\": \""
					<< CDataJson::Escape(Carrier.strHistoryId)
					<< "\", \"edgeLane\": \""
					<< AUTHORED_RUNTIME_BAKED_EDGE_LANE_TOKENS[
						static_cast<size_t>(Carrier.eEdgeLane)] << "\"";
				break;
			default:
				break;
			}
			Output << " },\n";
		}
		if (Element.eCompositionLayer != EFFECT_COMPOSITION_LAYER::NORMAL)
		{
			Output << "      \"compositionLayer\": \""
				<< COMPOSITION_LAYER_TOKENS[static_cast<size_t>(
					Element.eCompositionLayer)] << "\",\n";
		}
		if (bSourceContract)
			Write_Renderer(Output, Element.Renderer);
		Output << "      \"resources\": [";
		for (size_t iResource = 0u; iResource < Element.ResourceBindings.size(); ++iResource)
		{
			const EFFECT_RESOURCE_BINDING_DESC& Binding = Element.ResourceBindings[iResource];
			Output << (0u == iResource ? "\n" : ",\n")
				<< "        { \"" << (iSerializedVersion >= 6u ?
					"slotId" : "slot") << "\": \""
				<< CDataJson::Escape(Binding.strSlotId)
				<< "\", \"assetId\": \"" << CDataJson::Escape(Binding.strAssetId) << "\" }";
		}
		if (!Element.ResourceBindings.empty())
			Output << '\n';
		Output << "      ],\n";
		if (!Element.UnboundSourceResources.empty())
		{
			Output << "      \"unboundResources\": [";
			for (size_t iUnbound = 0u;
				iUnbound < Element.UnboundSourceResources.size(); ++iUnbound)
			{
				Output << (0u == iUnbound ? "\n" : ",\n")
					<< "        \"" << CDataJson::Escape(
						Element.UnboundSourceResources[iUnbound]) << "\"";
			}
			Output << "\n      ],\n";
		}
        if (Element.SourceTransformTrack)
            Write_SourceTransformTrack(Output, *Element.SourceTransformTrack);
		Output << "      \"material\": ";
		Write_Material(Output, Element.Material);
		Output << ",\n"
			<< "      \"actionCueAttachment\": { \"enabled\": "
			<< (Element.ActionCueAttachment.bEnabled ? "true" : "false")
			<< ", \"follow\": "
			<< (Element.ActionCueAttachment.bFollow ? "true" : "false");
		if (Element.ActionCueAttachment.eOrientation ==
			EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW)
		{
			Output << ", \"orientation\": \"owner_yaw\"";
		}
		else if (Element.ActionCueAttachment.eOrientation ==
			EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW)
		{
			Output << ", \"orientation\": \"camera_view\"";
		}
		if (!Element.ActionCueAttachment.strModelCueId.empty())
			Output << ", \"modelCueId\": \""
				<< CDataJson::Escape(Element.ActionCueAttachment.strModelCueId) << "\"";
		Output << ", \"sourceAnchorSlotId\": \""
			<< CDataJson::Escape(
				Element.ActionCueAttachment.strSourceAnchorSlotId)
			<< "\", \"runtimeAnchorSlotId\": \""
			<< CDataJson::Escape(
				Element.ActionCueAttachment.strRuntimeAnchorSlotId)
			<< "\", \"runtimeBoneName\": \""
			<< CDataJson::Escape(
				Element.ActionCueAttachment.strRuntimeBoneName)
			<< "\", \"snapshotRootSourceBasisYawDegrees\": "
			<< Element.ActionCueAttachment.fSnapshotRootSourceBasisYawDegrees
			<< ", \"socketLocalTransform\": { \"position\": ";
		Write_Float3(Output,
			Element.ActionCueAttachment.SocketLocalTransform.vPosition);
		Output << ", \"rotationDegrees\": ";
		Write_Float3(Output,
			Element.ActionCueAttachment.SocketLocalTransform.vRotationDegrees);
		Output << ", \"scale\": ";
		Write_Float3(Output,
			Element.ActionCueAttachment.SocketLocalTransform.vScale);
		Output << " } },\n";
		Output << "      \"transformInheritance\": { \"enabled\": "
			<< (Element.TransformInheritance.bEnabled ? "true" : "false")
			<< ", \"masterElementId\": \""
			<< CDataJson::Escape(
				Element.TransformInheritance.strMasterElementId)
			<< "\" },\n";
		Write_Detail(Output, Element.Detail);
		Output << ",\n";
		Write_SourceRecipe(Output, Element.SourceRecipe, bSourceContract);
		Output << ",\n";
		Write_SourcePresentation(Output, Element.SourcePresentation);
		if (!Element.AuthoringOverrides.Is_Empty())
		{
			// Absent when empty so untouched documents stay byte-identical.
			Output << ",\n      \"authoringOverrides\": { \"resources\": [";
			for (size_t i = 0u;
				i < Element.AuthoringOverrides.ResourceBindings.size(); ++i)
			{
				const Client::EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC&
					Binding = Element.AuthoringOverrides.ResourceBindings[i];
				Output << (0u == i ? " " : ", ")
					<< "{ \"slotId\": \""
					<< Client::CDataJson::Escape(Binding.strSlotId)
					<< "\", \"assetId\": \""
					<< Client::CDataJson::Escape(Binding.strAssetId)
					<< "\", \"compilerAssetId\": \""
					<< Client::CDataJson::Escape(Binding.strCompilerAssetId)
					<< "\" }";
			}
			Output << (Element.AuthoringOverrides.ResourceBindings.empty() ?
				"], \"scalars\": [" : " ], \"scalars\": [");
			for (size_t i = 0u;
				i < Element.AuthoringOverrides.Scalars.size(); ++i)
			{
				const Client::EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC& Scalar =
					Element.AuthoringOverrides.Scalars[i];
				Output << (0u == i ? " " : ", ")
					<< "{ \"name\": \""
					<< Client::CDataJson::Escape(Scalar.strName)
					<< "\", \"value\": " << Scalar.fValue
					<< ", \"compilerValue\": " << Scalar.fCompilerValue
					<< " }";
			}
			Output << (Element.AuthoringOverrides.Scalars.empty() ?
				"], \"colors\": [" : " ], \"colors\": [");
			for (size_t i = 0u;
				i < Element.AuthoringOverrides.Colors.size(); ++i)
			{
				const Client::EFFECT_AUTHORING_COLOR_OVERRIDE_DESC& Color =
					Element.AuthoringOverrides.Colors[i];
				Output << (0u == i ? " " : ", ")
					<< "{ \"name\": \""
					<< Client::CDataJson::Escape(Color.strName)
					<< "\", \"value\": ";
				Write_Float4(Output, Color.vValue);
				Output << ", \"compilerValue\": ";
				Write_Float4(Output, Color.vCompilerValue);
				Output << " }";
			}
			Output << (Element.AuthoringOverrides.Colors.empty() ?
				"] }" : " ] }");
		}
		Output << "\n    }";
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
	std::ifstream Input(Path, std::ios::binary | std::ios::ate);
	if (!Input)
	{
		strOutError = "Effect document could not be opened.";
		return false;
	}
	const std::streamoff FileBytes = Input.tellg();
	if (FileBytes < 0)
	{
		strOutError = "Effect document byte count could not be read.";
		return false;
	}
	if (0 == FileBytes ||
		static_cast<uint64_t>(FileBytes) > MAXIMUM_DOCUMENT_BYTES)
	{
		strOutError = "Effect document byte count is outside its limit: actualBytes=" +
			std::to_string(FileBytes) + ", limitBytes=" +
			std::to_string(MAXIMUM_DOCUMENT_BYTES) + ".";
		return false;
	}
	std::string Text(static_cast<size_t>(FileBytes), '\0');
	Input.seekg(0, std::ios::beg);
	Input.read(Text.data(), static_cast<std::streamsize>(FileBytes));
	if (!Input || Input.peek() != std::char_traits<char>::eof() || Input.bad())
	{
		strOutError = "Effect document read was incomplete or its byte count changed.";
		return false;
	}
	return Parse(Text, OutDocument, strOutError);
}

namespace Client::EffectDocumentCodecDetail
{

	std::filesystem::path Make_EffectSaveTransactionPath(
		const std::filesystem::path& Destination,
		const std::wstring_view strRole)
	{
		static std::atomic_uint64_t TransactionCounter = 0u;
		const uint64_t iCounter = TransactionCounter.fetch_add(
			1u, std::memory_order_relaxed);
		const auto iClock = std::chrono::steady_clock::now()
			.time_since_epoch().count();
		return Destination.wstring() + L"." + std::wstring(strRole) + L"." +
			std::to_wstring(iClock) + L"." + std::to_wstring(iCounter);
	}


	bool_t Matches_EffectDocumentCanonicalOnDisk(
		const std::filesystem::path& Path,
		const std::string_view ExpectedCanonical)
	{
		std::ifstream Input(Path, std::ios::binary);
		if (!Input)
			return false;
		std::ostringstream Buffer;
		Buffer << Input.rdbuf();
		if (!Input.eof() && Input.fail())
			return false;
		const std::string Bytes = Buffer.str();
		// Our previous save already wrote canonical JSON. Avoid parsing that same
		// graph again; formatting-only external edits retain the semantic check.
		if (Bytes == ExpectedCanonical)
			return true;
		Client::EFFECT_DOCUMENT_DESC Current;
		std::string Error;
		return Client::CEffectDocumentCodec::Parse(Bytes, Current, Error) &&
			Client::CEffectDocumentCodec::Serialize(Current) == ExpectedCanonical;
	}


	bool_t Save_EffectDocumentAtomic(
		const std::filesystem::path& Path,
		const Client::EFFECT_DOCUMENT_DESC& Document,
		const std::string_view* pExpectedCanonicalDocument,
		std::string& strOutError,
		std::string* pOutSavedCanonical = nullptr)
	{
		using Client::CEffectDocumentCodec;

		// Authoring save preserves valid partial drafts. The publisher/runtime gate
		// still calls Validate_Drawable before a document can ship or render.
		if (!(Document.bSourceContract ?
			CEffectDocumentCodec::Validate_SourceContract(Document, strOutError) :
			CEffectDocumentCodec::Validate(Document, strOutError)))
			return false;
		std::error_code Error;
		std::filesystem::create_directories(Path.parent_path(), Error);
		if (Error)
		{
			strOutError = "Effect authoring directory creation failed.";
			return false;
		}
		const std::filesystem::path Temporary =
			Make_EffectSaveTransactionPath(Path, L"tmp");
		const std::filesystem::path Backup =
			Make_EffectSaveTransactionPath(Path, L"bak");
		const std::string Json = CEffectDocumentCodec::Serialize(Document);
		{
			std::ofstream Output(Temporary, std::ios::binary | std::ios::trunc);
			Output.write(Json.data(), static_cast<std::streamsize>(Json.size()));
			Output.flush();
			if (!Output)
			{
				strOutError = "Effect temporary write failed.";
				std::filesystem::remove(Temporary, Error);
				return false;
			}
		}
		Client::EFFECT_DOCUMENT_DESC RoundTrip;
		if (!CEffectDocumentCodec::Load(Temporary, RoundTrip, strOutError))
		{
			std::filesystem::remove(Temporary, Error);
			return false;
		}
		if (CEffectDocumentCodec::Serialize(RoundTrip) != Json)
		{
			strOutError =
				"Effect temporary round-trip changed the authoring document.";
			std::filesystem::remove(Temporary, Error);
			return false;
		}

		if (nullptr != pExpectedCanonicalDocument)
		{
			Error.clear();
			const bool_t bDestinationExists =
				std::filesystem::exists(Path, Error);
			if (Error)
			{
				strOutError =
					"Effect destination state could not be checked before save.";
				std::filesystem::remove(Temporary, Error);
				return false;
			}
			if (pExpectedCanonicalDocument->empty())
			{
				if (bDestinationExists)
				{
					strOutError =
						"Effect destination appeared after this authoring session began; reload before saving.";
					std::filesystem::remove(Temporary, Error);
					return false;
				}
			}
			else
			{
				if (!bDestinationExists ||
					!Matches_EffectDocumentCanonicalOnDisk(
						Path, *pExpectedCanonicalDocument))
				{
					strOutError =
						"Effect document changed on disk after it was loaded; Reload Saved before applying this draft.";
					std::filesystem::remove(Temporary, Error);
					return false;
				}
			}
		}

		Error.clear();
		const bool_t bHadDestination =
			std::filesystem::exists(Path, Error) && !Error;
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
			std::filesystem::remove(Temporary, RestoreError);
			strOutError = RestoreError ?
				"Effect document promote and rollback failed." :
				"Effect document promote failed.";
			return false;
		}
		std::filesystem::remove(Backup, Error);
		if (nullptr != pOutSavedCanonical)
			*pOutSavedCanonical = Json;
		strOutError.clear();
		return true;
	}

}


bool_t Client::CEffectDocumentCodec::Save_Atomic(
	const std::filesystem::path& Path,
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	return Save_EffectDocumentAtomic(Path, Document, nullptr, strOutError);
}


bool_t Client::CEffectDocumentCodec::Save_AtomicIfUnchanged(
	const std::filesystem::path& Path,
	const EFFECT_DOCUMENT_DESC& Document,
	const std::string_view strExpectedCanonicalDocument,
	std::string& strOutError)
{
	return Save_EffectDocumentAtomic(
		Path, Document, &strExpectedCanonicalDocument, strOutError);
}


bool_t Client::CEffectDocumentCodec::Save_AtomicIfUnchanged(
	const std::filesystem::path& Path,
	const EFFECT_DOCUMENT_DESC& Document,
	const std::string_view strExpectedCanonicalDocument,
	std::string& strOutError,
	std::string* pOutSavedCanonical)
{
	return Save_EffectDocumentAtomic(Path, Document,
		&strExpectedCanonicalDocument, strOutError, pOutSavedCanonical);
}


void Client::CEffectDocumentCodec::Collect_ResourceAssetIds(
	const EFFECT_DOCUMENT_DESC& Document,
	std::vector<std::string>& OutAssetIds)
{
	std::unordered_set<std::string> Unique;
	for (const EFFECT_MODEL_CUE_DESC& Cue : Document.ModelCues)
	{
		Unique.insert(Cue.strModelAssetId);
		if (Cue.Material)
			for (const EFFECT_NAMED_TEXTURE_DESC& Texture : Cue.Material->SourceMaterial.Textures)
				if (!Texture.strAssetId.empty()) Unique.insert(Texture.strAssetId);
	}
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		for (const EFFECT_RESOURCE_BINDING_DESC& Binding : Element.ResourceBindings)
			Unique.insert(Binding.strAssetId);
		for (const EFFECT_NAMED_TEXTURE_DESC& Texture :
			Element.Material.SourceMaterial.Textures)
		{
			if (!Texture.strAssetId.empty())
				Unique.insert(Texture.strAssetId);
		}
		for (const EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane :
			Element.Material.Execution.TextureLanes)
		{
			if (!Lane.strAssetId.empty())
				Unique.insert(Lane.strAssetId);
		}
	}
	OutAssetIds.assign(Unique.begin(), Unique.end());
	std::sort(OutAssetIds.begin(), OutAssetIds.end());
}
