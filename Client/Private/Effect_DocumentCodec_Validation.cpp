#include "Effect_DocumentCodec_Internal.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_Playback.h"

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


bool_t Client::CEffectDocumentCodec::Validate(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	if (Document.iFormatVersion != EFFECT_AUTHORING_FORMAT_VERSION ||
		Document.iLoadedFormatVersion < EFFECT_AUTHORING_MIN_SUPPORTED_VERSION ||
		Document.iLoadedFormatVersion > EFFECT_AUTHORING_MAX_SUPPORTED_VERSION)
	{
		strOutError = "Unsupported Effect document version.";
		return false;
	}
	if (Document.bSourceContract ||
		Document.iLoadedFormatVersion == EFFECT_SOURCE_CONTRACT_FORMAT_VERSION)
	{
		strOutError =
			"Native-v14 source contracts are not runtime Effect documents.";
		return false;
	}
	if (!Is_ValidEffectBloomIntensity(Document.fBloomIntensity))
	{
		strOutError = "Effect bloomIntensity must be finite and between 0 and 16.";
		return false;
	}
	if (!Validate_AuthoredRuntimeExtensions(Document, strOutError))
		return false;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		const EFFECT_CASCADE_RECIPE_DESC& Recipe = Element.SourceRecipe;
		const EFFECT_SOURCE_COMPILER_EVIDENCE_DESC& Evidence =
			Recipe.CompilerEvidence;
		const EFFECT_SOURCE_GEOMETRY_BINDING_DESC& Geometry =
			Recipe.GeometryBinding;
		const bool_t bCompilerEvidencePresent =
			!Evidence.strArtifactFileSha256.empty() ||
			!Evidence.strArtifactSelfSha256.empty() ||
			!Evidence.strEvidenceId.empty() ||
			!Evidence.strSourceEvidenceStatus.empty() ||
			!Evidence.strSourceCueId.empty() ||
			!Evidence.strSourceOccurrenceId.empty() ||
			!Evidence.strSourceSystemId.empty() ||
			!Evidence.strSourceEmitterPath.empty() ||
			!Evidence.strSourceEmitterNodeId.empty() ||
			!Evidence.strLodSelectionPolicy.empty() ||
			!Evidence.strSelectedLodPath.empty() ||
			!Evidence.strSelectedLodNodeId.empty() ||
			0u != Evidence.iSelectedLodArrayIndex ||
			!Evidence.strSelectedLodLevelProvenance.empty() ||
			!Evidence.strSelectedLodEnabledProvenance.empty() ||
			0u != Evidence.iNonSelectedLodCount ||
			!Evidence.ModuleReferenceOrder.empty() ||
			Evidence.vCueSourcePositionUeUnits.x != 0.f ||
			Evidence.vCueSourcePositionUeUnits.y != 0.f ||
			Evidence.vCueSourcePositionUeUnits.z != 0.f ||
			Evidence.CueLocalTransform.vPosition.x != 0.f ||
			Evidence.CueLocalTransform.vPosition.y != 0.f ||
			Evidence.CueLocalTransform.vPosition.z != 0.f ||
			Evidence.CueLocalTransform.vRotationDegrees.x != 0.f ||
			Evidence.CueLocalTransform.vRotationDegrees.y != 0.f ||
			Evidence.CueLocalTransform.vRotationDegrees.z != 0.f ||
			Evidence.CueLocalTransform.vScale.x != 1.f ||
			Evidence.CueLocalTransform.vScale.y != 1.f ||
			Evidence.CueLocalTransform.vScale.z != 1.f ||
			!Evidence.ParameterOverrides.empty() ||
			!Evidence.CompositionOrder.empty() ||
			!Evidence.strLocalReferenceClosureFileSha256.empty() ||
			!Evidence.strLocalReferenceClosureSelfSha256.empty() ||
			!Evidence.strGeometryParityFileSha256.empty() ||
			!Evidence.strGeometryParitySelfSha256.empty();
		const bool_t bDistributionEvidencePresent = std::any_of(
			Recipe.Modules.begin(), Recipe.Modules.end(),
			[](const EFFECT_SOURCE_MODULE_DESC& Module)
			{
				return std::any_of(Module.Distributions.begin(),
					Module.Distributions.end(),
					[](const EFFECT_DISTRIBUTION_DESC& Distribution)
					{
						return !Distribution.strReferenceId.empty() ||
							!Distribution.strOccurrenceId.empty() ||
							!Distribution.strPayloadStatus.empty() ||
							!Distribution.strFidelity.empty() ||
							!Distribution.strParameterName.empty() ||
							Distribution.eParameterBinding !=
								EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE ||
							Distribution.ExecutionAdmission.bAllowed ||
							!Distribution.ExecutionAdmission.Blockers.empty();
					});
			});
		if (Element.Renderer.eType != EFFECT_RENDERER_TYPE::END ||
			Element.Renderer.eSourceSpace != EFFECT_SOURCE_SPACE::END ||
			!Recipe.strSourceContractProfileId.empty() ||
			!Recipe.strSourceContractSha256.empty() ||
			!Recipe.strSourceGraphSha256.empty() ||
			!Recipe.strSourceClosureSha256.empty() ||
			!Recipe.strSourceMaterialClosureSha256.empty() ||
			0u != Recipe.iSourcePeakActiveParticles ||
			!Recipe.LocalReferenceBindings.empty() ||
			!Recipe.ModuleCoverage.empty() || bCompilerEvidencePresent ||
			bDistributionEvidencePresent ||
			Recipe.CompiledExecutionAdmission.bAllowed ||
			!Recipe.CompiledExecutionAdmission.Blockers.empty() ||
			!Recipe.MaterialAdmission.strStatus.empty() ||
			!Recipe.MaterialAdmission.SourceMaterialPaths.empty() ||
			!Recipe.MaterialAdmission.strMaterialRecipeId.empty() ||
			!Recipe.MaterialAdmission.strRenderStateRecipeId.empty() ||
			!Recipe.MaterialAdmission.Blockers.empty() ||
			Geometry.bEnabled || !Geometry.strAssetId.empty() ||
			!Geometry.strReceiptFileSha256.empty() ||
			!Geometry.strReceiptSelfSha256.empty() ||
			Geometry.fCarrierGeometryPreScale != 1.f ||
			!Geometry.strParticleScaleSemantics.empty() ||
			!Geometry.strStatus.empty() || !Geometry.Blockers.empty())
		{
			strOutError =
				"Legacy Effect documents cannot carry native-v14 source-contract fields.";
			return false;
		}
	}
	if (!Is_StableId(Document.strEffectAssetId))
	{
		strOutError = "Effect Asset ID is invalid.";
		return false;
	}
	if (Document.strDisplayName.size() > 256u ||
		!Has_VisibleCharacter(Document.strDisplayName))
	{
		strOutError = "Display Name must be 1-256 bytes and not blank.";
		return false;
	}
	const EFFECT_PARTICLE_SYSTEM_DESC& ParticleSystem =
		Document.ParticleSystem;
	if (!std::isfinite(ParticleSystem.fUniformScaleMultiplier) ||
		ParticleSystem.fUniformScaleMultiplier <= 0.f ||
		ParticleSystem.fUniformScaleMultiplier > 100.f ||
		!std::isfinite(ParticleSystem.fYawOffsetDegrees) ||
		std::abs(ParticleSystem.fYawOffsetDegrees) > 3600.f ||
		!std::isfinite(ParticleSystem.fDirectionYawDegrees) ||
		std::abs(ParticleSystem.fDirectionYawDegrees) > 3600.f ||
		!std::isfinite(ParticleSystem.fInitialSpeedMultiplier) ||
		ParticleSystem.fInitialSpeedMultiplier < 0.f ||
		ParticleSystem.fInitialSpeedMultiplier > 100.f)
	{
		strOutError = "Particle System modifier contains an invalid number or range.";
		return false;
	}
	if (Document.Elements.size() > MAX_ELEMENTS)
	{
		strOutError = "Effect Element count exceeds 2048.";
		return false;
	}
    if (Document.SourceModelPreview)
    {
        const auto& preview = *Document.SourceModelPreview;
        if ((preview.strGateId != "GATE1" && preview.strGateId != "GATE2" && preview.strGateId != "GATE3" && preview.strGateId != "ENCORE") ||
            preview.strActorProfileId.empty() || preview.strActorProfileId.size() > 128u ||
            preview.strTargetBossPlacementId.empty() || preview.strTargetBossPlacementId.size() > 128u ||
            preview.Animations.empty() || preview.Animations.size() > 256u)
        { strOutError = "Effect source model preview has missing actor identity or animation windows."; return false; }
        uint32_t previousEnd = 0u;
        for (const auto& animation : preview.Animations)
        {
            if (animation.strRuntimeClip.empty() || animation.strRuntimeClip.size() > 256u ||
                !animation.iPlayMs || animation.iPlayMs > 600000u || animation.iStartOffsetMs > 600000u - animation.iPlayMs ||
                animation.iStartOffsetMs < previousEnd || animation.iSourceStartMs > 600000u ||
                !std::isfinite(animation.fPlayRate) || animation.fPlayRate < .01f || animation.fPlayRate > 100.f ||
                (animation.strEndPolicy != "EXACT" && animation.strEndPolicy != "HOLD_LAST_POSE" && animation.strEndPolicy != "LOOP_TO_WINDOW"))
            { strOutError = "Effect source model animation has an unsupported clip window."; return false; }
            previousEnd = animation.iStartOffsetMs + animation.iPlayMs;
        }
    }
	if (Document.ModelCues.size() > MAX_MODEL_CUES)
	{
		strOutError = "Effect Model Cue count exceeds 16.";
		return false;
	}
	std::unordered_set<std::string> ModelCueIds;
	for (const EFFECT_MODEL_CUE_DESC& Cue : Document.ModelCues)
	{
		const bool_t bTransformValid =
			Is_Finite(Cue.LocalTransform.vPosition) &&
			Is_Finite(Cue.LocalTransform.vRotationDegrees) &&
			Is_Finite(Cue.LocalTransform.vScale) &&
			Cue.LocalTransform.vScale.x > 0.f &&
			Cue.LocalTransform.vScale.y > 0.f &&
			Cue.LocalTransform.vScale.z > 0.f &&
			Is_Finite(Cue.vAssetPreScale) &&
			Cue.vAssetPreScale.x > 0.f && Cue.vAssetPreScale.y > 0.f &&
			Cue.vAssetPreScale.z > 0.f &&
			Is_Finite(Cue.vAssetPreRotationDegrees) &&
			Is_Finite(Cue.LocalTransform.vRevolutionDegreesPerSecond) &&
			Is_Finite(Cue.LocalTransform.vVelocityPerSecond) &&
			Is_Finite(Cue.vColorMultiply) &&
			std::isfinite(Cue.fOpacity) && Cue.fOpacity >= 0.f &&
			Cue.fOpacity <= 1.f;
		if (!Is_StableId(Cue.strCueId) ||
			!ModelCueIds.insert(Cue.strCueId).second ||
			Cue.strClipName.empty() || Cue.strClipName.size() > 128u ||
			!Has_VisibleCharacter(Cue.strClipName) ||
			!Is_SafeModelCueAssetId(Cue.strModelAssetId) ||
			!std::isfinite(Cue.fStartDelaySeconds) ||
			Cue.fStartDelaySeconds < 0.f ||
			!std::isfinite(Cue.fDurationSeconds) ||
			Cue.fDurationSeconds <= 0.f || Cue.fDurationSeconds > 30.f ||
			Cue.eAlphaMode >= EFFECT_MODEL_CUE_ALPHA_MODE::END ||
			!bTransformValid)
		{
			strOutError =
				"Effect Model Cue identity, resource, time, or transform is invalid.";
			return false;
		}
		if (!Cue.strSuppressHorizontalRootMotionBone.empty() &&
			(Cue.strSuppressHorizontalRootMotionBone.size() > 128u ||
			 !Has_VisibleCharacter(Cue.strSuppressHorizontalRootMotionBone)))
		{
			strOutError = "Effect Model Cue horizontal root-motion bone is invalid: " + Cue.strCueId;
			return false;
		}
		if (Cue.bLoop && Cue.bHoldLastFrame)
		{
			strOutError = "Effect Model Cue cannot loop and hold its last frame: " +
				Cue.strCueId;
			return false;
		}
		if (Cue.Material)
		{
			EFFECT_ELEMENT_DESC Metadata;
			Metadata.strElementId = Cue.strCueId;
			Metadata.eKind = EFFECT_ELEMENT_KIND::PARTICLE;
			Metadata.Material = *Cue.Material;
			if (Cue.eAlphaMode != EFFECT_MODEL_CUE_ALPHA_MODE::TRANSLUCENT_SURFACE ||
				!Validate_ElementMaterial(Metadata, true, strOutError) ||
				!(Has_ArtistModelCueMaterialContract(Cue) || Has_LanceMasterVAModelCueMaterialContract(Cue) ||
                  Has_DimensionMasterALTVModelCueMaterialContract(Cue)))
			{
				if (strOutError.empty())
					strOutError = "Model Cue recovered skeletal material contract is invalid: " + Cue.strCueId;
				return false;
			}
		}
	}

	std::unordered_set<std::string> ElementIds;
	std::unordered_map<std::string, const EFFECT_ELEMENT_DESC*> ElementsById;
	uint64_t iTotalParticles = 0u;
	uint64_t iTotalTrailPoints = 0u;
	uint64_t iTotalAfterImages = 0u;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
        if (Element.SourceTransformTrack && !Validate_SourceTransformTrack(*Element.SourceTransformTrack, strOutError))
            return false;
		if (Element.strDisplayName.size() > 64u ||
			!Has_VisibleCharacter(Element.strDisplayName))
		{
			strOutError = "Element '" + Element.strElementId +
				"' display name must be 1-64 UTF-8 bytes and not blank (got " +
				std::to_string(Element.strDisplayName.size()) + " bytes).";
			return false;
		}
		if (!Is_StableId(Element.strElementId) ||
			(!Element.strGroupId.empty() && !Is_StableId(Element.strGroupId)) ||
			Element.strSourceNode.size() > 256u ||
			Element.eKind >= EFFECT_ELEMENT_KIND::END ||
			Element.Material.eRenderProfile >= EFFECT_RENDER_PROFILE::END ||
			!ElementIds.insert(Element.strElementId).second)
		{
			strOutError = "Element metadata, kind, profile, or duplicate is invalid.";
			return false;
		}
		ElementsById.emplace(Element.strElementId, &Element);
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Attachment =
			Element.ActionCueAttachment;
		const bool_t bAttachmentTransformValid =
			std::isfinite(Attachment.fSnapshotRootSourceBasisYawDegrees) &&
			std::abs(Attachment.fSnapshotRootSourceBasisYawDegrees) <= 3600.f &&
			Is_Finite(Attachment.SocketLocalTransform.vPosition) &&
			Is_Finite(Attachment.SocketLocalTransform.vRotationDegrees) &&
			Is_Finite(Attachment.SocketLocalTransform.vScale) &&
			Attachment.SocketLocalTransform.vScale.x > 0.f &&
			Attachment.SocketLocalTransform.vScale.y > 0.f &&
			Attachment.SocketLocalTransform.vScale.z > 0.f;
		const bool_t bCameraViewAttachment = Attachment.eOrientation ==
			EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW;
		const bool_t bOwnerYawAttachment = Attachment.eOrientation ==
			EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW;
		if (!Attachment.strModelCueId.empty() &&
			(!Is_StableId(Attachment.strModelCueId) ||
			 !ModelCueIds.contains(Attachment.strModelCueId) ||
			 !Attachment.bEnabled || !Attachment.bFollow ||
			 Attachment.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::BONE))
		{
			strOutError = "Effect model-cue attachment owner is invalid: " +
				Attachment.strModelCueId;
			return false;
		}
		if (!bAttachmentTransformValid ||
			Attachment.eOrientation >= EFFECT_ATTACHMENT_ORIENTATION::END ||
			(bCameraViewAttachment && (!Attachment.bEnabled || !Attachment.bFollow ||
				!Attachment.strRuntimeBoneName.empty())) ||
			(bOwnerYawAttachment &&
				(!Attachment.bEnabled || !Attachment.bFollow ||
				 Document.bSourceContract || Element.SourceRecipe.bEnabled ||
				 !Element.RuntimeCarrier.Is_Empty())) ||
			((!Attachment.bEnabled || Attachment.bFollow) &&
				0.f != Attachment.fSnapshotRootSourceBasisYawDegrees) ||
			(Attachment.bEnabled &&
				(Attachment.strSourceAnchorSlotId.empty() ||
					Attachment.strSourceAnchorSlotId.size() > 128u ||
					!Has_VisibleCharacter(
						Attachment.strSourceAnchorSlotId) ||
					Attachment.strRuntimeAnchorSlotId.empty() ||
					Attachment.strRuntimeAnchorSlotId.size() > 128u ||
					!Has_VisibleCharacter(
						Attachment.strRuntimeAnchorSlotId) ||
					(Attachment.bFollow && !bCameraViewAttachment &&
						(Attachment.strRuntimeBoneName.empty() ||
							Attachment.strRuntimeBoneName.size() > 128u ||
							!Has_VisibleCharacter(
								Attachment.strRuntimeBoneName))))))
		{
			strOutError = "Effect Action cue attachment contract is invalid.";
			return false;
		}
		const bool_t bAuthoringExecutionTarget =
			Is_EffectElementAuthoringExecutionTarget(Element);
		if (!Validate_ElementSourceMaterialSlots(Element, strOutError))
			return false;
		const EFFECT_CASCADE_RECIPE_DESC& Recipe = Element.SourceRecipe;
		if (Recipe.bEnabled)
		{
			const bool_t bRendererShapeValid =
				Recipe.strRendererShape == "sprite" ||
				Recipe.strRendererShape == "mesh" ||
				Recipe.strRendererShape == "decal" ||
				Recipe.strRendererShape == "ribbon" ||
				Recipe.strRendererShape == "beam" ||
				Recipe.strRendererShape == "light" ||
				Recipe.strRendererShape == "screenPost";
			if (!bRendererShapeValid ||
				!std::isfinite(Recipe.fEmitterDelaySeconds) ||
				Recipe.fEmitterDelaySeconds < 0.f ||
				!std::isfinite(Recipe.fEmitterDurationSeconds) ||
				Recipe.fEmitterDurationSeconds < 0.f ||
				Recipe.fEmitterDurationSeconds > 600.f ||
				Recipe.iEmitterLoopCount > 100000u ||
				Recipe.Bursts.size() > MAX_SOURCE_BURSTS_PER_ELEMENT ||
				Recipe.Modules.size() > MAX_SOURCE_MODULES_PER_ELEMENT)
			{
				strOutError = "Effect source recipe metadata or size is invalid.";
				return false;
			}
			f32_t fPreviousBurstTime = -1.f;
			for (const EFFECT_PARTICLE_BURST_DESC& Burst : Recipe.Bursts)
			{
				if (!std::isfinite(Burst.fTimeSeconds) ||
					Burst.fTimeSeconds < fPreviousBurstTime ||
					Burst.iCountMinimum > Burst.iCountMaximum ||
					Burst.iCountMaximum > 65535u)
				{
					strOutError = "Effect source burst is invalid.";
					return false;
				}
				fPreviousBurstTime = Burst.fTimeSeconds;
			}
			std::unordered_set<std::string> SourceModuleIds;
			for (const EFFECT_SOURCE_MODULE_DESC& Module : Recipe.Modules)
			{
				if (Module.strStableId.empty() ||
					Module.strStableId.size() > 256u ||
					!SourceModuleIds.insert(Module.strStableId).second ||
					Module.strClassName.empty() ||
					Module.strClassName.size() > 128u ||
					Module.strObjectPath.empty() ||
					Module.strObjectPath.size() > 512u ||
					Module.Literals.size() > MAX_SOURCE_LITERALS_PER_MODULE ||
					Module.Distributions.size() >
						MAX_SOURCE_DISTRIBUTIONS_PER_MODULE)
				{
					strOutError = "Effect source module metadata or size is invalid.";
					return false;
				}
				std::unordered_set<std::string> PropertyPaths;
				for (const EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
				{
					if (Literal.strPropertyPath.empty() ||
						Literal.strPropertyPath.size() > 512u ||
						Literal.eKind >= EFFECT_SOURCE_LITERAL_KIND::END ||
						!PropertyPaths.insert(Literal.strPropertyPath).second ||
						(EFFECT_SOURCE_LITERAL_KIND::NUMBER == Literal.eKind &&
							!std::isfinite(Literal.fNumber)) ||
						Literal.strString.size() > 2048u)
					{
						strOutError = "Effect source module literal is invalid.";
						return false;
					}
				}
				for (const EFFECT_DISTRIBUTION_DESC& Distribution :
					Module.Distributions)
				{
					if (!PropertyPaths.insert(
						Distribution.strPropertyPath).second ||
						!CEffectDistribution::Validate(Distribution,
							strOutError))
					{
						if (strOutError.empty())
							strOutError =
								"Effect source distribution is duplicated.";
						return false;
					}
				}
			}
		}
		if (!Document.bSourceContract && Recipe.bEnabled &&
			bAuthoringExecutionTarget &&
			(Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE ||
			 Element.eKind == EFFECT_ELEMENT_KIND::DECAL ||
             (Element.eKind == EFFECT_ELEMENT_KIND::TRAIL && Element.RuntimeCarrier.eKind ==
                EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_RIBBON_V1)) &&
			!ValidatePortableAuthoredParticleRuntimeCarrier(
				Element, strOutError))
		{
			strOutError =
				"Ordinary authored emitter sourceRecipe is not admitted by the portable runtime: " +
				Element.strElementId + ": " + strOutError;
			return false;
		}
		const EFFECT_SOURCE_PRESENTATION_DESC& SourcePresentation =
			Element.SourcePresentation;
		if (SourcePresentation.bEnabled)
		{
			if (SourcePresentation.strSchema !=
					EFFECT_SOURCE_PRESENTATION_SCHEMA ||
				SourcePresentation.iVersion != 1u ||
				!Is_StableId(SourcePresentation.strProfileId) ||
				SourcePresentation.eStatus >=
					EFFECT_SOURCE_PRESENTATION_STATUS::END ||
				SourcePresentation.strSourceObjectPath.empty() ||
				SourcePresentation.strSourceObjectPath.size() > 512u ||
				!Has_VisibleCharacter(
					SourcePresentation.strSourceObjectPath) ||
				SourcePresentation.strSourceActionCueId.size() > 256u ||
				(!SourcePresentation.strSourceActionCueId.empty() &&
					!Has_VisibleCharacter(
						SourcePresentation.strSourceActionCueId)) ||
				SourcePresentation.strSourceEventId.empty() ||
				SourcePresentation.strSourceEventId.size() > 256u ||
				!Has_VisibleCharacter(
					SourcePresentation.strSourceEventId) ||
				!std::isfinite(SourcePresentation.fSourceTimeSeconds) ||
				SourcePresentation.fSourceTimeSeconds < 0.f ||
				SourcePresentation.Parameters.size() >
					MAX_SOURCE_PRESENTATION_PARAMETERS)
			{
				strOutError =
					"Effect source presentation metadata is invalid.";
				return false;
			}
			std::unordered_set<std::string> ParameterNames;
			for (const EFFECT_SOURCE_PRESENTATION_PARAMETER_DESC& Parameter :
				SourcePresentation.Parameters)
			{
				if (Parameter.strName.empty() ||
					Parameter.strName.size() > 128u ||
					!Has_VisibleCharacter(Parameter.strName) ||
					!ParameterNames.insert(Parameter.strName).second ||
					Parameter.eKind >=
						EFFECT_SOURCE_PRESENTATION_PARAMETER_KIND::END ||
					Parameter.eStatus >=
						EFFECT_SOURCE_PRESENTATION_PARAMETER_STATUS::END ||
					Parameter.strSourcePropertyPath.size() > 512u ||
					!std::isfinite(Parameter.fNumberValue) ||
					!Is_Finite(Parameter.vVectorValue) ||
					Parameter.strStringValue.size() > 2048u)
				{
					strOutError =
						"Effect source presentation parameter is invalid.";
					return false;
				}
			}
			/* An unresolved presentation is retained source evidence, not an
			   executable authored carrier.  Keeping this invariant in the codec
			   prevents a saved Visible toggle from bypassing Tool-side locks for
			   deferred AnimationTrail, Light, Dust, or future source families. */
			if (!Document.bSourceContract && bAuthoringExecutionTarget &&
				SourcePresentation.eStatus ==
					EFFECT_SOURCE_PRESENTATION_STATUS::UNRESOLVED &&
				Element.bVisible)
			{
				strOutError =
					"Unresolved source presentation cannot be enabled for ordinary playback: " +
					Element.strElementId + ".";
				return false;
			}
		}
		const EFFECT_DETAIL_DESC& D = Element.Detail;
		const bool_t bMeshParticle =
			Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			std::any_of(Element.ResourceBindings.begin(),
				Element.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
				});
		const bool_t bDirectHandAuthored = Element.strSourceNode.empty() ||
			Element.strSourceNode.starts_with("authored-copy:");
		const bool_t bManualParticle =
			Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			bDirectHandAuthored && !Element.SourceRecipe.bEnabled &&
			!Element.SourcePresentation.bEnabled &&
			Element.Renderer.eType == EFFECT_RENDERER_TYPE::END;
		const bool_t bManualSprite =
			Element.eKind == EFFECT_ELEMENT_KIND::SPRITE &&
			bDirectHandAuthored && !Element.SourceRecipe.bEnabled &&
			!Element.SourcePresentation.bEnabled &&
			Element.Renderer.eType == EFFECT_RENDERER_TYPE::END;
		const bool_t bGenericMeshRingFillCarrier =
			bManualParticle && bMeshParticle &&
			Element.Material.strTemplateId == EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			Element.Material.strSourceMaterialPath.empty() &&
			!Element.Material.SourceMaterial.bEnabled &&
			!Element.Material.Execution.bEnabled &&
			!Element.Material.Execution.bFailClosed &&
			!Element.Material.Execution.bAuthoringApproximate &&
			Element.Material.eRenderProfile !=
				EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE;
		const bool_t bGenericLinearRevealCarrier =
			(bManualSprite || (bManualParticle && !bMeshParticle)) &&
			Element.Material.strTemplateId == EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			Element.Material.strSourceMaterialPath.empty() &&
			!Element.Material.SourceMaterial.bEnabled &&
			!Element.Material.Execution.bEnabled &&
			!Element.Material.Execution.bFailClosed &&
			!Element.Material.Execution.bAuthoringApproximate &&
			Element.Material.eRenderProfile !=
				EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE;
		const bool_t bCompositionLayerValid =
			Element.eCompositionLayer < EFFECT_COMPOSITION_LAYER::END &&
			(Element.eCompositionLayer == EFFECT_COMPOSITION_LAYER::NORMAL ||
             (Element.eCompositionLayer == EFFECT_COMPOSITION_LAYER::WORLD_MARK && Is_EffectWorldMarkCarrier(Element)) ||
             (Element.eCompositionLayer == EFFECT_COMPOSITION_LAYER::SCENE_BACKDROP && Is_EffectSceneBackdropCarrier(Element)));
		const bool_t bMeshTransformMotionCarrier =
			Element.eKind == EFFECT_ELEMENT_KIND::MESH ||
			bMeshParticle;
		const int64_t iTileCount = static_cast<int64_t>(D.UV.iTileColumns) * D.UV.iTileRows;
		const bool_t bCommonValid =
			Is_Finite(D.Transform.vPosition) && Is_Finite(D.Transform.vRotationDegrees) &&
			Is_Finite(D.Transform.vRevolutionDegreesPerSecond) && Is_Finite(D.Transform.vScale) &&
			D.Transform.vScale.x > 0.f && D.Transform.vScale.y > 0.f && D.Transform.vScale.z > 0.f &&
			Is_Finite(D.Transform.vVelocityPerSecond) && Is_Finite(D.Color.vColorOffset) &&
			Is_Finite(D.Color.vColorMultiply) &&
			std::isfinite(D.Color.fColorClip) &&
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
			std::isfinite(D.Timing.fTransformMotionDurationSeconds) &&
			D.Timing.fTransformMotionDurationSeconds >= 0.f &&
			D.Timing.fTransformMotionDurationSeconds <= D.Timing.fLifeTimeSeconds &&
			(0.f == D.Timing.fTransformMotionDurationSeconds ||
			 bMeshTransformMotionCarrier) &&
			std::isfinite(D.Timing.fAfterImageSeconds) && D.Timing.fAfterImageSeconds >= 0.f &&
			std::isfinite(D.Timing.fDissolveStartNormalized) &&
			D.Timing.fDissolveStartNormalized >= 0.f && D.Timing.fDissolveStartNormalized <= 1.f &&
			std::isfinite(D.Mesh.fModelPreScale) &&
			D.Mesh.fModelPreScale > 0.f && D.Mesh.fModelPreScale <= 100.f &&
			Is_Finite(D.Mesh.vSourceTypeDataRotationDegrees) &&
			std::abs(D.Mesh.vSourceTypeDataRotationDegrees.x) <= 3600.f &&
			std::abs(D.Mesh.vSourceTypeDataRotationDegrees.y) <= 3600.f &&
			std::abs(D.Mesh.vSourceTypeDataRotationDegrees.z) <= 3600.f &&
			(Element.Renderer.eType == EFFECT_RENDERER_TYPE::MESH_PARTICLE ||
				(Element.Renderer.eType == EFFECT_RENDERER_TYPE::END &&
				 Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
				 Element.SourceRecipe.bEnabled &&
				 Element.SourceRecipe.strRendererShape == "mesh") ||
				(0.f == D.Mesh.vSourceTypeDataRotationDegrees.x &&
				 0.f == D.Mesh.vSourceTypeDataRotationDegrees.y &&
				 0.f == D.Mesh.vSourceTypeDataRotationDegrees.z)) &&
			std::isfinite(D.Sprite.fBillboardRollDegrees) &&
			std::abs(D.Sprite.fBillboardRollDegrees) <= 3600.f &&
			std::isfinite(D.Sprite.fBillboardRollDegreesPerSecond) &&
			std::abs(D.Sprite.fBillboardRollDegreesPerSecond) <= 3600.f &&
			Is_Finite(D.Decal.vSize) && D.Decal.vSize.x > 0.f && D.Decal.vSize.y > 0.f &&
			std::isfinite(D.Decal.fDepth) && D.Decal.fDepth > 0.f;
		const bool_t bLerpValid =
			Is_Finite(D.LinearLerp.vEndPosition) && Is_Finite(D.LinearLerp.vEndRotationDegrees) &&
			Is_Finite(D.LinearLerp.vEndRevolutionDegreesPerSecond) && Is_Finite(D.LinearLerp.vEndScale) &&
			D.LinearLerp.vEndScale.x > 0.f && D.LinearLerp.vEndScale.y > 0.f && D.LinearLerp.vEndScale.z > 0.f &&
			Is_Finite(D.LinearLerp.vEndVelocityPerSecond) && Is_Finite(D.LinearLerp.vEndColorOffset) &&
			Is_Finite(D.LinearLerp.vEndColorMultiply) &&
			std::isfinite(D.LinearLerp.fEndEmissiveIntensity) &&
			D.LinearLerp.fEndEmissiveIntensity >= 0.f &&
			std::isfinite(D.LinearLerp.fEndRingFillProgress) &&
			D.LinearLerp.fEndRingFillProgress >= 0.f &&
			D.LinearLerp.fEndRingFillProgress <= 1.f &&
			(D.LinearLerp.bRingFillProgress ||
			 D.LinearLerp.fEndRingFillProgress == 1.f) &&
			(!D.LinearLerp.bRingFillProgress || D.Mesh.RingFill.bEnabled);
		const bool_t bParticleValid =
			D.Particle.iMaxParticles >= 1u && D.Particle.iMaxParticles <= 2048u &&
			D.Particle.iBurstCount <= D.Particle.iMaxParticles && D.Particle.iRandomSeed != 0u &&
			std::isfinite(D.Particle.fSpawnRatePerSecond) && D.Particle.fSpawnRatePerSecond >= 0.f && D.Particle.fSpawnRatePerSecond <= 2048.f &&
			std::isfinite(D.Particle.fFixedCenterSpacingWorldUnits) &&
			D.Particle.fFixedCenterSpacingWorldUnits >= 0.f &&
			(D.Particle.fFixedCenterSpacingWorldUnits == 0.f ||
			 D.Particle.fFixedCenterSpacingWorldUnits >= 0.001f) &&
			D.Particle.fFixedCenterSpacingWorldUnits <= 1000.f &&
			Is_Finite(D.Particle.vLifeTimeSeconds) && D.Particle.vLifeTimeSeconds.x > 0.f && D.Particle.vLifeTimeSeconds.y >= D.Particle.vLifeTimeSeconds.x && D.Particle.vLifeTimeSeconds.y <= 30.f &&
			Is_Finite(D.Particle.vInitialPositionMin) && Is_Finite(D.Particle.vInitialPositionMax) &&
			D.Particle.vInitialPositionMax.x >= D.Particle.vInitialPositionMin.x &&
			D.Particle.vInitialPositionMax.y >= D.Particle.vInitialPositionMin.y &&
			D.Particle.vInitialPositionMax.z >= D.Particle.vInitialPositionMin.z &&
			Is_Finite(D.Particle.vInitialVelocityMin) && Is_Finite(D.Particle.vInitialVelocityMax) && Is_Finite(D.Particle.vAcceleration) &&
			D.Particle.vInitialVelocityMax.x >= D.Particle.vInitialVelocityMin.x &&
			D.Particle.vInitialVelocityMax.y >= D.Particle.vInitialVelocityMin.y &&
			D.Particle.vInitialVelocityMax.z >= D.Particle.vInitialVelocityMin.z &&
			Is_Finite(D.Particle.vStartSize) && D.Particle.vStartSize.x > 0.f && D.Particle.vStartSize.y > 0.f &&
			Is_Finite(D.Particle.vEndSize) && D.Particle.vEndSize.x >= 0.f && D.Particle.vEndSize.y >= 0.f &&
			D.Particle.iDynamicParameterComponentMask <= 0x0fu &&
			Is_Finite(D.Particle.vDynamicParameterStart) &&
			Is_Finite(D.Particle.vDynamicParameterEnd) &&
			(EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind ||
				0u == D.Particle.iDynamicParameterComponentMask);
		/* A shape that cannot be sampled - a negative or inverted radius, an empty
		   arc, a zero-extent box - would silently collapse every particle onto the
		   origin instead of failing, so it is rejected at load. */
		const EFFECT_PARTICLE_SPAWN_SHAPE_DESC& Shape = D.Particle.SpawnShape;
		const EFFECT_PARTICLE_INITIAL_ORIENTATION_DESC& Orientation =
			D.Particle.InitialOrientation;
		const EFFECT_PARTICLE_INITIAL_VELOCITY_DESC& Emission =
			D.Particle.InitialVelocity;
		const bool_t bFixedCenterSpacingEnabled =
			D.Particle.fFixedCenterSpacingWorldUnits > 0.f;
		const bool_t bFixedCenterSpacingValid =
			!bFixedCenterSpacingEnabled ||
			(bManualParticle && !D.Particle.bLocalSpace &&
			 D.Particle.fSpawnRatePerSecond == 0.f &&
			 D.Particle.iBurstCount == 0u &&
			 D.Particle.vInitialPositionMin.x == 0.f &&
			 D.Particle.vInitialPositionMin.y == 0.f &&
			 D.Particle.vInitialPositionMin.z == 0.f &&
			 D.Particle.vInitialPositionMax.x == 0.f &&
			 D.Particle.vInitialPositionMax.y == 0.f &&
			 D.Particle.vInitialPositionMax.z == 0.f &&
			 D.Particle.vInitialVelocityMin.x == 0.f &&
			 D.Particle.vInitialVelocityMin.y == 0.f &&
			 D.Particle.vInitialVelocityMin.z == 0.f &&
			 D.Particle.vInitialVelocityMax.x == 0.f &&
			 D.Particle.vInitialVelocityMax.y == 0.f &&
				 D.Particle.vInitialVelocityMax.z == 0.f &&
				 D.Particle.vAcceleration.x == 0.f &&
				 D.Particle.vAcceleration.y == 0.f &&
				 D.Particle.vAcceleration.z == 0.f &&
				 Shape.eKind == EFFECT_PARTICLE_SPAWN_SHAPE::POINT &&
				 Shape.eDistribution == EFFECT_PARTICLE_SPAWN_DISTRIBUTION::RANDOM &&
				 Emission.eMode == EFFECT_PARTICLE_VELOCITY_MODE::FIXED &&
				 D.Particle.TargetAttractor.Is_Default());
		const bool_t bSpawnShapeValid =
			Shape.eKind < EFFECT_PARTICLE_SPAWN_SHAPE::END &&
			Shape.eDistribution < EFFECT_PARTICLE_SPAWN_DISTRIBUTION::END &&
			std::isfinite(Shape.fRadius) && std::isfinite(Shape.fInnerRadius) &&
			Is_Finite(Shape.vExtents) && std::isfinite(Shape.fArcDegrees) &&
			Shape.fRadius >= 0.f && Shape.fInnerRadius >= 0.f &&
			Shape.fInnerRadius <= Shape.fRadius &&
			Shape.fArcDegrees > 0.f && Shape.fArcDegrees <= 360.f &&
			Shape.vExtents.x >= 0.f && Shape.vExtents.y >= 0.f &&
			Shape.vExtents.z >= 0.f &&
			(EFFECT_PARTICLE_SPAWN_SHAPE::POINT == Shape.eKind ||
				EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind) &&
			(EFFECT_PARTICLE_SPAWN_SHAPE::SPHERE != Shape.eKind || Shape.fRadius > 0.f) &&
			(EFFECT_PARTICLE_SPAWN_SHAPE::RING != Shape.eKind || Shape.fRadius > 0.f) &&
			(EFFECT_PARTICLE_SPAWN_SHAPE::BOX != Shape.eKind ||
				Shape.vExtents.x > 0.f || Shape.vExtents.y > 0.f ||
				Shape.vExtents.z > 0.f) &&
			(EFFECT_PARTICLE_SPAWN_DISTRIBUTION::RANDOM ==
				Shape.eDistribution ||
			 (bManualParticle &&
			  EFFECT_PARTICLE_SPAWN_SHAPE::RING == Shape.eKind &&
			  D.Particle.fSpawnRatePerSecond == 0.f &&
			  D.Particle.iBurstCount >= 2u));
		const bool_t bInitialOrientationValid =
			Orientation.eMode < EFFECT_PARTICLE_ORIENTATION_MODE::END &&
			std::isfinite(Orientation.fOffsetDegrees) &&
			std::abs(Orientation.fOffsetDegrees) <= 3600.f &&
			(EFFECT_PARTICLE_ORIENTATION_MODE::FIXED != Orientation.eMode ||
			 Orientation.fOffsetDegrees == 0.f) &&
			(EFFECT_PARTICLE_ORIENTATION_MODE::FIXED == Orientation.eMode ||
			 (bManualParticle && !bMeshParticle &&
			  EFFECT_PARTICLE_SPAWN_SHAPE::RING == Shape.eKind &&
			  !D.Particle.bBillboard));
		const bool_t bInitialVelocityValid =
			Emission.eMode < EFFECT_PARTICLE_VELOCITY_MODE::END &&
			Is_Finite(Emission.vSpeedRange) &&
			std::isfinite(Emission.fConeAngleDegrees) &&
			Emission.vSpeedRange.y >= Emission.vSpeedRange.x &&
			Emission.fConeAngleDegrees >= 0.f &&
			Emission.fConeAngleDegrees <= 180.f &&
			(EFFECT_PARTICLE_VELOCITY_MODE::FIXED == Emission.eMode ||
				EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind);
		const bool_t bHasNativeSpriteDynamics = D.Particle.fDrag != 0.f ||
			D.Particle.vRotationRangeDegrees.x != 0.f ||
			D.Particle.vRotationRangeDegrees.y != 0.f ||
			D.Particle.vSpinRangeDegreesPerSecond.x != 0.f ||
			D.Particle.vSpinRangeDegreesPerSecond.y != 0.f ||
			D.Particle.bSubUVOverLife || Emission.bUniformSolidAngle;
		const bool_t bNativeSpriteDynamicsValid =
			std::isfinite(D.Particle.fDrag) &&
			D.Particle.fDrag >= 0.f && D.Particle.fDrag <= 1000.f &&
			Is_Finite(D.Particle.vRotationRangeDegrees) &&
			Is_Finite(D.Particle.vSpinRangeDegreesPerSecond) &&
			D.Particle.vRotationRangeDegrees.x >= -3600.f &&
			D.Particle.vRotationRangeDegrees.y <= 3600.f &&
			D.Particle.vRotationRangeDegrees.x <=
				D.Particle.vRotationRangeDegrees.y &&
			D.Particle.vSpinRangeDegreesPerSecond.x >= -3600.f &&
			D.Particle.vSpinRangeDegreesPerSecond.y <= 3600.f &&
			D.Particle.vSpinRangeDegreesPerSecond.x <=
				D.Particle.vSpinRangeDegreesPerSecond.y &&
			(!bHasNativeSpriteDynamics ||
			 (bManualParticle && !bMeshParticle && D.Particle.bBillboard &&
			  Element.Material.strTemplateId == EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			  Element.Material.strSourceMaterialPath.empty() &&
			  !Element.Material.SourceMaterial.bEnabled &&
			  !Element.Material.Execution.bEnabled &&
			  !Element.Material.Execution.bFailClosed &&
			  !Element.Material.Execution.bAuthoringApproximate)) &&
			(!Emission.bUniformSolidAngle ||
			 (Emission.eMode == EFFECT_PARTICLE_VELOCITY_MODE::CONE &&
			  Emission.vSpeedRange.x >= 0.f)) &&
			(!D.Particle.bSubUVOverLife ||
			 (!D.UV.bSequence && !D.UV.bLoop && D.UV.iTileIndex == 0 &&
			  iTileCount > 1 && iTileCount <= UINT32_MAX));
		if (!bNativeSpriteDynamicsValid)
		{
			strOutError =
				"Effect native sprite particle dynamics or life SubUV contract is invalid: " +
				Element.strElementId;
			return false;
		}
		const EFFECT_PARTICLE_TARGET_ATTRACTOR_DESC& Attractor =
			D.Particle.TargetAttractor;
		const bool_t bTargetAttractorValid =
			Attractor.eTargetSpace <
				EFFECT_PARTICLE_ATTRACTOR_TARGET_SPACE::END &&
			Is_Finite(Attractor.vTargetOffset) &&
			std::abs(Attractor.vTargetOffset.x) <= 1000.f &&
			std::abs(Attractor.vTargetOffset.y) <= 1000.f &&
			std::abs(Attractor.vTargetOffset.z) <= 1000.f &&
			Is_Finite(Attractor.vActiveNormalized) &&
			std::isfinite(Attractor.fRadialAcceleration) &&
			std::isfinite(Attractor.fTangentialAcceleration) &&
			std::isfinite(Attractor.fMaximumSpeed) &&
			std::isfinite(Attractor.fConvergenceRadius) &&
			std::isfinite(Attractor.fArrivalDamping) &&
			Attractor.vActiveNormalized.x >= 0.f &&
			Attractor.vActiveNormalized.y > Attractor.vActiveNormalized.x &&
			Attractor.vActiveNormalized.y <= 1.f &&
			Attractor.fRadialAcceleration >= 0.f &&
			Attractor.fRadialAcceleration <= 10000.f &&
			Attractor.fTangentialAcceleration >= -10000.f &&
			Attractor.fTangentialAcceleration <= 10000.f &&
			Attractor.fMaximumSpeed > 0.f &&
			Attractor.fMaximumSpeed <= 1000.f &&
			Attractor.fConvergenceRadius > 0.f &&
			Attractor.fConvergenceRadius <= 1000.f &&
			Attractor.fArrivalDamping >= 0.f &&
			Attractor.fArrivalDamping <= 1000.f &&
			(!Attractor.bEnabled ||
				EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind) &&
			(Attractor.bEnabled || Attractor.Is_Default());
		/* The trim multiplies the source's own numbers. Count, size and lifetime
		   stay positive; speed and rotation may intentionally stop or reverse;
		   alpha and delay may be zero. Ceilings reject accidental extreme input. */
		const EFFECT_PARTICLE_SOURCE_SCALE_DESC& SourceScale =
			D.Particle.SourceScale;
		const bool_t bSourceScaleValid =
			std::isfinite(SourceScale.fCount) &&
			std::isfinite(SourceScale.fSize) &&
			std::isfinite(SourceScale.fLifeTime) &&
			std::isfinite(SourceScale.fSpeed) &&
			std::isfinite(SourceScale.fRotation) &&
			std::isfinite(SourceScale.fAlpha) &&
			std::isfinite(SourceScale.fSpawnDelay) &&
			SourceScale.fCount > 0.f && SourceScale.fCount <= 16.f &&
			SourceScale.fSize > 0.f && SourceScale.fSize <= 16.f &&
			SourceScale.fLifeTime > 0.f && SourceScale.fLifeTime <= 16.f &&
			/* Speed and rotation may legitimately be reversed or stopped, so
			   they allow zero and negative. Rotation is a source-angle/rate
			   multiplier and therefore has the wider authoring range. */
			SourceScale.fSpeed >= -16.f && SourceScale.fSpeed <= 16.f &&
			SourceScale.fRotation >= -360.f && SourceScale.fRotation <= 360.f &&
			SourceScale.fAlpha >= 0.f && SourceScale.fAlpha <= 16.f &&
			SourceScale.fSpawnDelay >= 0.f && SourceScale.fSpawnDelay <= 16.f;
		const bool_t bTrailValid =
			D.Trail.iMaxPoints >= 2u && D.Trail.iMaxPoints <= 512u &&
			std::isfinite(D.Trail.fPointLifeTimeSeconds) && D.Trail.fPointLifeTimeSeconds > 0.f &&
			std::isfinite(D.Trail.fSampleIntervalSeconds) && D.Trail.fSampleIntervalSeconds > 0.f &&
			std::isfinite(D.Trail.fMinimumDistance) && D.Trail.fMinimumDistance >= 0.f &&
			std::isfinite(D.Trail.fStartWidth) && D.Trail.fStartWidth > 0.f &&
			std::isfinite(D.Trail.fEndWidth) && D.Trail.fEndWidth >= 0.f &&
			std::isfinite(D.Trail.fTilingDistanceWorldUnits) &&
			D.Trail.fTilingDistanceWorldUnits >= 0.f &&
			std::isfinite(D.Trail.fDistanceTessellationStepWorldUnits) &&
			D.Trail.fDistanceTessellationStepWorldUnits >= 0.f;
		const bool_t bAfterImageValid =
			std::isfinite(D.AfterImage.fSampleIntervalSeconds) && D.AfterImage.fSampleIntervalSeconds > 0.f &&
			D.AfterImage.iMaxCopies <= 32u && std::isfinite(D.AfterImage.fAlphaExponent) && D.AfterImage.fAlphaExponent > 0.f &&
			(D.Timing.fAfterImageSeconds <= 0.f || D.AfterImage.iMaxCopies == 0u ||
				Element.eKind == EFFECT_ELEMENT_KIND::MESH || Element.eKind == EFFECT_ELEMENT_KIND::SPRITE);
		const bool_t bLightValid =
			(EFFECT_ELEMENT_KIND::LIGHT == Element.eKind ||
				!D.Light.bEnabled) &&
			(!D.Light.bEnabled ||
				(D.Light.eProfile < EFFECT_LIGHT_PROFILE::END &&
					D.Light.eStatus ==
						EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE &&
					std::isfinite(D.Light.fRange) && D.Light.fRange > 0.f &&
					std::isfinite(D.Light.fIntensity) &&
					D.Light.fIntensity >= 0.f && Is_Finite(D.Light.vColor) &&
					Is_Finite(D.Light.vAmbient) &&
					std::isfinite(D.Light.fFalloffExponent) &&
					D.Light.fFalloffExponent > 0.f));
		const bool_t bScreenPostValid =
			(EFFECT_ELEMENT_KIND::SCREEN_POST == Element.eKind ||
				!D.ScreenPost.bEnabled) &&
			(!D.ScreenPost.bEnabled ||
				(D.ScreenPost.eProfile < EFFECT_SCREEN_POST_PROFILE::END &&
					(D.ScreenPost.eProfile != EFFECT_SCREEN_POST_PROFILE::MOTION_BLUR_RECONSTRUCTED_V1 ||
						Has_ArtistMaterialContract(Element)) &&
					D.ScreenPost.eStatus ==
						EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE &&
					std::isfinite(D.ScreenPost.fIntensity) &&
					D.ScreenPost.fIntensity >= 0.f &&
					std::isfinite(D.ScreenPost.fSecondaryIntensity) &&
					D.ScreenPost.fSecondaryIntensity >= 0.f &&
					std::isfinite(D.ScreenPost.fFrequency) &&
					D.ScreenPost.fFrequency >= 0.f &&
					Is_Finite(D.ScreenPost.vTint) &&
					0u != D.ScreenPost.iRandomSeed));
		const EFFECT_MESH_RING_FILL_DESC& RingFill = D.Mesh.RingFill;
		const bool_t bRingFillValid =
			RingFill.eDirection < EFFECT_RING_FILL_DIRECTION::END &&
			std::isfinite(RingFill.fProgress) &&
			RingFill.fProgress >= 0.f && RingFill.fProgress <= 1.f &&
			std::isfinite(RingFill.fFeather) && RingFill.fFeather >= 0.f &&
			RingFill.fFeather <= 0.5f &&
			(RingFill.bEnabled || RingFill.Is_Default()) &&
			(!RingFill.bEnabled || bGenericMeshRingFillCarrier);
		const EFFECT_LINEAR_REVEAL_DESC& LinearReveal = D.Sprite.LinearReveal;
		const bool_t bFixedParticleRevealClock = !bManualParticle ||
			(std::fabs(D.Particle.vLifeTimeSeconds.x -
				D.Particle.vLifeTimeSeconds.y) <= 0.00001f &&
			 LinearReveal.fStartSeconds + LinearReveal.fDurationSeconds <=
				D.Particle.vLifeTimeSeconds.x);
		const bool_t bLinearRevealValid =
			LinearReveal.eAxis < EFFECT_LINEAR_REVEAL_AXIS::END &&
			std::isfinite(LinearReveal.fStartSeconds) &&
			LinearReveal.fStartSeconds >= 0.f &&
			LinearReveal.fStartSeconds <= 30.f &&
			std::isfinite(LinearReveal.fDurationSeconds) &&
			LinearReveal.fDurationSeconds > 0.f &&
			LinearReveal.fDurationSeconds <= 30.f &&
			std::isfinite(LinearReveal.fEdgeWidth) &&
			LinearReveal.fEdgeWidth >= 0.f &&
			LinearReveal.fEdgeWidth <= 0.5f &&
			std::isfinite(LinearReveal.fSoftness) &&
			LinearReveal.fSoftness >= 0.f &&
			LinearReveal.fSoftness <= 0.25f &&
			LinearReveal.fEdgeWidth + LinearReveal.fSoftness <= 0.5f &&
			Is_Finite(LinearReveal.vEdgeColor) &&
			LinearReveal.vEdgeColor.x >= 0.f &&
			LinearReveal.vEdgeColor.x <= 1.f &&
			LinearReveal.vEdgeColor.y >= 0.f &&
			LinearReveal.vEdgeColor.y <= 1.f &&
			LinearReveal.vEdgeColor.z >= 0.f &&
			LinearReveal.vEdgeColor.z <= 1.f &&
			LinearReveal.vEdgeColor.w >= 0.f &&
			LinearReveal.vEdgeColor.w <= 1.f &&
			std::isfinite(LinearReveal.fEdgeEmissive) &&
			LinearReveal.fEdgeEmissive >= 0.f &&
			LinearReveal.fEdgeEmissive <= 100.f &&
			(LinearReveal.bEnabled || LinearReveal.Is_Default()) &&
			(!LinearReveal.bEnabled ||
				 (bGenericLinearRevealCarrier && bFixedParticleRevealClock &&
				  LinearReveal.fStartSeconds + LinearReveal.fDurationSeconds <=
					D.Timing.fLifeTimeSeconds));
		const EFFECT_DECAL_DETAIL_DESC& Decal = D.Decal;
		const bool_t bDecalReceiverValid =
			Decal.eReceiverMode < EFFECT_DECAL_RECEIVER_MODE::END &&
			std::isfinite(Decal.fNormalCutoff) &&
			std::isfinite(Decal.fEdgeFade) &&
			Decal.fEdgeFade >= 0.f && Decal.fEdgeFade <= 1.f &&
			((Decal.eReceiverMode == EFFECT_DECAL_RECEIVER_MODE::ALL_OPAQUE &&
			  Decal.fNormalCutoff == -1.f) ||
			 (Decal.eReceiverMode ==
				EFFECT_DECAL_RECEIVER_MODE::UPWARD_SURFACES &&
			  Element.eKind == EFFECT_ELEMENT_KIND::DECAL &&
			  Decal.fNormalCutoff >= 0.f && Decal.fNormalCutoff <= 1.f)) &&
			(Element.eKind == EFFECT_ELEMENT_KIND::DECAL ||
			 Decal.Is_ReceiverDefault());
		if (!bCompositionLayerValid || !bCommonValid ||
			!bDecalReceiverValid || !bLerpValid || !bParticleValid ||
			!bFixedCenterSpacingValid || !bSpawnShapeValid ||
			!bInitialOrientationValid ||
			!bInitialVelocityValid || !bRingFillValid ||
			!bLinearRevealValid ||
			!bTargetAttractorValid ||
			!bSourceScaleValid ||
			!bTrailValid || !bAfterImageValid || !bLightValid ||
			!bScreenPostValid)
		{
			strOutError = "Effect Detail contains an invalid number or range.";
			return false;
		}
		/* Playback also simulates source-visual mesh, sprite, and decal carriers
		   through Detail.Particle.  Count the same carrier set here so a source
		   document cannot bypass the document-wide particle cap merely because
		   its authored Element kind describes the renderer shape. */
		const bool_t bSourceRecipeParticleCarrier =
			Element.SourceRecipe.bEnabled &&
			(Element.SourceRecipe.strRendererShape == "mesh" ||
			 Element.SourceRecipe.strRendererShape == "sprite" ||
			 Element.SourceRecipe.strRendererShape == "decal");
		if (EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind ||
			bSourceRecipeParticleCarrier)
		{
			iTotalParticles += SourceScaledParticleCeiling(Element);
		}
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
	if (!Document.bSourceContract &&
		!ValidatePortableAuthoredParticleEventRoutes(Document, strOutError))
	{
		return false;
	}

	const auto SameFloat3 = [](const float3_t& Left, const float3_t& Right)
	{
		return Left.x == Right.x && Left.y == Right.y && Left.z == Right.z;
	};
	const auto SameTransform = [&SameFloat3](
		const EFFECT_TRANSFORM_DESC& Left,
		const EFFECT_TRANSFORM_DESC& Right)
	{
		return SameFloat3(Left.vPosition, Right.vPosition) &&
			SameFloat3(Left.vRotationDegrees, Right.vRotationDegrees) &&
			SameFloat3(Left.vRevolutionDegreesPerSecond,
				Right.vRevolutionDegreesPerSecond) &&
			SameFloat3(Left.vScale, Right.vScale) &&
			SameFloat3(Left.vVelocityPerSecond, Right.vVelocityPerSecond);
	};
	const auto SameAttachment = [&SameTransform](
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Left,
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Right)
	{
		return Left.bEnabled == Right.bEnabled &&
			Left.bFollow == Right.bFollow &&
			Left.eOrientation == Right.eOrientation &&
			Left.strSourceAnchorSlotId == Right.strSourceAnchorSlotId &&
			Left.strRuntimeAnchorSlotId == Right.strRuntimeAnchorSlotId &&
			Left.strRuntimeBoneName == Right.strRuntimeBoneName &&
			Left.strModelCueId == Right.strModelCueId &&
			Left.fSnapshotRootSourceBasisYawDegrees ==
				Right.fSnapshotRootSourceBasisYawDegrees &&
			SameTransform(Left.SocketLocalTransform,
				Right.SocketLocalTransform);
	};
	std::unordered_map<std::string, const EFFECT_ACTION_CUE_ATTACHMENT_DESC*>
		FollowAnchorsById;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Attachment =
			Element.ActionCueAttachment;
		if (!Attachment.bEnabled || !Attachment.bFollow)
			continue;
		const auto [Iterator, bInserted] = FollowAnchorsById.emplace(
			Attachment.strRuntimeAnchorSlotId, &Attachment);
		if (!bInserted &&
			(Iterator->second->strRuntimeBoneName != Attachment.strRuntimeBoneName ||
			 Iterator->second->eOrientation != Attachment.eOrientation ||
			 Iterator->second->strModelCueId != Attachment.strModelCueId ||
			 !SameTransform(Iterator->second->SocketLocalTransform,
				Attachment.SocketLocalTransform)))
		{
			strOutError = "Effect follow anchor ID has conflicting bone, socket, or orientation: " +
				Attachment.strRuntimeAnchorSlotId + ".";
			return false;
		}
	}
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		const EFFECT_TRANSFORM_INHERITANCE_DESC& Inheritance =
			Element.TransformInheritance;
		if (!Inheritance.bEnabled)
		{
			if (!Inheritance.strMasterElementId.empty())
			{
				strOutError =
					"Disabled Effect transform inheritance must not name a master Element.";
				return false;
			}
			continue;
		}
		if (!Is_StableId(Inheritance.strMasterElementId) ||
			Inheritance.strMasterElementId == Element.strElementId)
		{
			strOutError =
				"Effect transform inheritance master identity is invalid or self-referential.";
			return false;
		}
		const auto MasterIterator = ElementsById.find(
			Inheritance.strMasterElementId);
		if (MasterIterator == ElementsById.end())
		{
			strOutError = "Effect transform inheritance master Element is missing.";
			return false;
		}
		const EFFECT_ELEMENT_DESC& Master = *MasterIterator->second;
		if (Element.strGroupId.empty() ||
			Element.strGroupId != Master.strGroupId)
		{
			strOutError =
				"Effect transform inheritance must remain inside one Component group.";
			return false;
		}
		if (!Master.bVisible ||
			EFFECT_ELEMENT_KIND::SCREEN_POST == Master.eKind ||
			EFFECT_ELEMENT_KIND::SCREEN_POST == Element.eKind)
		{
			strOutError =
				"Effect transform inheritance requires a visible world-space master and companion.";
			return false;
		}
		if (Element.Detail.Timing.fStartDelaySeconds !=
				Master.Detail.Timing.fStartDelaySeconds ||
			Element.SourceRecipe.fEmitterDelaySeconds !=
				Master.SourceRecipe.fEmitterDelaySeconds)
		{
			strOutError =
				"Effect transform inheritance master and companion start times must match.";
			return false;
		}
		if (!SameAttachment(Element.ActionCueAttachment,
			Master.ActionCueAttachment))
		{
			strOutError =
				"Effect transform inheritance master and companion attachment spaces must match.";
			return false;
		}
	}

	std::unordered_map<std::string, uint8_t> VisitStates;
	const auto VisitInheritance = [&](const auto& Self,
		const EFFECT_ELEMENT_DESC& Element) -> bool_t
	{
		uint8_t& State = VisitStates[Element.strElementId];
		if (1u == State)
		{
			strOutError = "Effect transform inheritance cycle is not allowed.";
			return false;
		}
		if (2u == State)
			return true;
		State = 1u;
		if (Element.TransformInheritance.bEnabled)
		{
			const auto MasterIterator = ElementsById.find(
				Element.TransformInheritance.strMasterElementId);
			if (MasterIterator == ElementsById.end() ||
				!Self(Self, *MasterIterator->second))
			{
				return false;
			}
		}
		State = 2u;
		return true;
	};
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (!VisitInheritance(VisitInheritance, Element))
			return false;
	}
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (!Element.TransformInheritance.bEnabled)
			continue;
		const EFFECT_ELEMENT_DESC& Master = *ElementsById.at(
			Element.TransformInheritance.strMasterElementId);
		if (Master.TransformInheritance.bEnabled)
		{
			strOutError =
				"Effect transform inheritance companions must reference one terminal master directly.";
			return false;
		}
	}
	if (!CEffectPlayback::Validate_SourceParticleProviders(Document, strOutError))
		return false;
	strOutError.clear();
	return true;
}
