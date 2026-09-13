#include "Effect_DocumentCodec_Internal.h"
#include "Effect_MaterialTemplate.h"

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


bool_t Client::CEffectDocumentCodec::Build_DuplicatedAuthoredElements(
	const EFFECT_DOCUMENT_DESC& SourceDocument,
	const std::vector<std::string>& SourceElementIds,
	EFFECT_DOCUMENT_DESC& InOutDocument,
	std::unordered_map<std::string, std::string>& OutDuplicatedElementIds,
	std::string& strOutError)
{
	if (SourceDocument.bSourceContract || SourceElementIds.empty())
	{
		strOutError = "Duplicate requires an authored Effect and selected Element IDs.";
		return false;
	}
	if (!Validate(SourceDocument, strOutError))
		return false;

	std::unordered_set<std::string> Targets;
	for (const std::string& ElementId : SourceElementIds)
	{
		if (!Is_StableId(ElementId) || !Targets.insert(ElementId).second)
		{
			strOutError = "Duplicate rejected an invalid or repeated selected Element ID.";
			return false;
		}
	}

	std::unordered_set<std::string> UsedIds;
	for (const EFFECT_ELEMENT_DESC& Element : SourceDocument.Elements)
		UsedIds.insert(Element.strElementId);
	std::unordered_map<std::string, std::string> DuplicateIds;
	const std::string Prefix = "authored.copy.";
	for (const EFFECT_ELEMENT_DESC& Element : SourceDocument.Elements)
	{
		if (!Targets.contains(Element.strElementId))
			continue;
		if (Element.eKind == EFFECT_ELEMENT_KIND::LIGHT ||
			Element.eKind == EFFECT_ELEMENT_KIND::SCREEN_POST)
		{
			strOutError = "Presentation Light and Screen Post duplication is not admitted.";
			return false;
		}
		std::string DuplicateId;
		for (size_t iCopy = 1u; iCopy <= UsedIds.size() + 1u; ++iCopy)
		{
			const std::string Suffix = "." + std::to_string(iCopy);
			const size_t iMaximumSourceLength = 128u - Prefix.size() - Suffix.size();
			DuplicateId = Prefix +
				Element.strElementId.substr(0u, iMaximumSourceLength) + Suffix;
			if (!UsedIds.contains(DuplicateId))
				break;
			DuplicateId.clear();
		}
		if (DuplicateId.empty())
		{
			strOutError = "Duplicate could not allocate a unique authored Element ID.";
			return false;
		}
		UsedIds.insert(DuplicateId);
		DuplicateIds.emplace(Element.strElementId, std::move(DuplicateId));
	}
	if (DuplicateIds.size() != Targets.size())
	{
		strOutError = "A selected Element no longer exists; nothing was duplicated.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Staged = SourceDocument;
	Staged.Elements.clear();
	Staged.Elements.reserve(SourceDocument.Elements.size() + DuplicateIds.size());
	for (const EFFECT_ELEMENT_DESC& Element : SourceDocument.Elements)
	{
		Staged.Elements.push_back(Element);
		const auto CopyId = DuplicateIds.find(Element.strElementId);
		if (CopyId == DuplicateIds.end())
			continue;
		EFFECT_ELEMENT_DESC Duplicate = Element;
		Duplicate.strElementId = CopyId->second;
		Duplicate.strSourceNode = std::string(EFFECT_PORTABLE_AUTHORED_COPY_PREFIX) +
			std::string(Resolve_EffectPortableOriginElementId(Element));
		Duplicate.SourcePresentation = {};
		if (Duplicate.TransformInheritance.bEnabled)
		{
			const auto MasterCopy = DuplicateIds.find(
				Duplicate.TransformInheritance.strMasterElementId);
			if (MasterCopy != DuplicateIds.end())
				Duplicate.TransformInheritance.strMasterElementId = MasterCopy->second;
		}
		Staged.Elements.push_back(std::move(Duplicate));
	}
	if (!Validate(Staged, strOutError))
		return false;
	InOutDocument = std::move(Staged);
	OutDuplicatedElementIds = std::move(DuplicateIds);
	strOutError.clear();
	return true;
}


bool_t Client::CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
	const EFFECT_DOCUMENT_DESC& SourceDocument,
	const std::string_view strElementId,
	const std::string_view strTargetEffectAssetId,
	EFFECT_DOCUMENT_DESC& InOutDocument,
	std::string& strOutError)
{
	if (strElementId.empty() || strTargetEffectAssetId.empty())
	{
		strOutError =
			"Generic authored starting copy requires stable source and target IDs.";
		return false;
	}
	const auto First = std::find_if(SourceDocument.Elements.begin(),
		SourceDocument.Elements.end(),
		[strElementId](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == strElementId;
		});
	if (First == SourceDocument.Elements.end() ||
		std::find_if(std::next(First), SourceDocument.Elements.end(),
			[strElementId](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == strElementId;
			}) != SourceDocument.Elements.end())
	{
		strOutError =
			"Generic authored starting copy requires exactly one source Element.";
		return false;
	}
	if (First->eKind != EFFECT_ELEMENT_KIND::MESH &&
		First->eKind != EFFECT_ELEMENT_KIND::SPRITE &&
		First->eKind != EFFECT_ELEMENT_KIND::PARTICLE &&
		First->eKind != EFFECT_ELEMENT_KIND::DECAL &&
		First->eKind != EFFECT_ELEMENT_KIND::TRAIL)
	{
		strOutError =
			"Generic authored starting copy supports Mesh, Sprite, Particle, Decal, and Trail Elements only.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Candidate;
	Candidate.strEffectAssetId = std::string(strTargetEffectAssetId);
	Candidate.strDisplayName = First->strDisplayName;
	Candidate.ParticleSystem = SourceDocument.ParticleSystem;
	Candidate.Elements.push_back(*First);
	EFFECT_ELEMENT_DESC& Lowered = Candidate.Elements.front();
	const bool_t bSourceMeshParticle =
		Lowered.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
		Lowered.SourceRecipe.bEnabled &&
		Lowered.SourceRecipe.strRendererShape == "mesh" &&
		std::any_of(Lowered.ResourceBindings.begin(),
			Lowered.ResourceBindings.end(),
			[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
			});
	EFFECT_DISTRIBUTION_DESC SourceStartSize;
	bool_t bHasSourceStartSize = false;
	size_t iSourceStartSizeCandidateCount = 0u;
	if (bSourceMeshParticle)
	{
		for (const EFFECT_SOURCE_MODULE_DESC& Module :
			Lowered.SourceRecipe.Modules)
		{
			std::string_view SourceClass = Module.strClassName;
			if (SourceClass.starts_with("efparticlemodule"))
				SourceClass.remove_prefix(2u);
			if (SourceClass.ends_with("_seeded"))
				SourceClass.remove_suffix(7u);
			if (SourceClass != "particlemodulesize")
				continue;
			for (const EFFECT_DISTRIBUTION_DESC& Distribution :
				Module.Distributions)
			{
				if (Distribution.strPropertyPath == "startsize")
				{
					++iSourceStartSizeCandidateCount;
					std::string DistributionError;
					if (1u == iSourceStartSizeCandidateCount &&
						Distribution.iComponentCount >= 2u &&
						Distribution.iOperation == 1u &&
						CEffectDistribution::Validate(
							Distribution, DistributionError))
					{
						SourceStartSize = Distribution;
						bHasSourceStartSize = true;
					}
				}
			}
		}
		/* Multiple or random StartSize modules require the portable source
		   executor's ordered composition.  The generic direct carrier cannot
		   reproduce that composition after SourceRecipe is removed, so keep the
		   existing fallback unchanged instead of guessing. */
		if (1u != iSourceStartSizeCandidateCount)
			bHasSourceStartSize = false;
	}
	/* The document already provides the effect-level namespace.  Keep the
	   composite group stable and bounded even when the target asset ID is at
	   the 128-byte contract limit. */
	Lowered.strGroupId = "manual.authoring";
	Lowered.strSourceNode.clear();
	Lowered.Renderer = {};
	Lowered.ActionCueAttachment = {};
	Lowered.TransformInheritance = {};
	Lowered.SourceRecipe = {};
	Lowered.SourcePresentation = {};
	Lowered.Detail.Mesh.vSourceTypeDataRotationDegrees = {};
	if (bHasSourceStartSize)
	{
		/* Older Track A projections stored the direct Mesh Particle fallback in
		   the WModel carrier's geometry unit even though portable SourceRecipe
		   playback consumes StartSize as a dimensionless instance scale.  Once
		   the recipe is removed, preserving that 0.01-scaled fallback would apply
		   modelPreScale a second time and make the copied mesh about 100x too
		   small.  Compare against the immutable source StartSize distribution so
		   already-normalized documents (for example Artist F) remain unchanged. */
		const float4_t SourceSize = CEffectDistribution::Evaluate(
			SourceStartSize, 0.f, 0.5f);
		const f32_t fModelPreScale = Lowered.Detail.Mesh.fModelPreScale;
		const auto NearlyEqualRelative = [](const f32_t A, const f32_t B)
		{
			const f32_t fTolerance = (std::max)(
				1.0e-5f, (std::max)(std::abs(A), std::abs(B)) * 1.0e-4f);
			return std::abs(A - B) <= fTolerance;
		};
		const float2_t SourceDimensionless = {
			std::abs(SourceSize.x), std::abs(SourceSize.y) };
		const float2_t DirectStart = Lowered.Detail.Particle.vStartSize;
		const bool_t bLegacyGeometryScaledSize =
			std::isfinite(fModelPreScale) && fModelPreScale > 0.f &&
			fModelPreScale < 1.f &&
			SourceDimensionless.x > 0.f && SourceDimensionless.y > 0.f &&
			NearlyEqualRelative(
				DirectStart.x, SourceDimensionless.x * fModelPreScale) &&
			NearlyEqualRelative(
				DirectStart.y, SourceDimensionless.y * fModelPreScale) &&
			(!NearlyEqualRelative(DirectStart.x, SourceDimensionless.x) ||
			 !NearlyEqualRelative(DirectStart.y, SourceDimensionless.y));
		if (bLegacyGeometryScaledSize)
		{
			const f32_t fDimensionlessSizeScale = 1.f / fModelPreScale;
			Lowered.Detail.Particle.vStartSize.x *= fDimensionlessSizeScale;
			Lowered.Detail.Particle.vStartSize.y *= fDimensionlessSizeScale;
			Lowered.Detail.Particle.vEndSize.x *= fDimensionlessSizeScale;
			Lowered.Detail.Particle.vEndSize.y *= fDimensionlessSizeScale;
		}
	}
	if (bSourceMeshParticle)
	{
		/* SourceScale is consumed only by SourceRecipe playback.  Bake its size
		   authority into the direct fallback even when no unambiguous StartSize
		   distribution was available, then neutralize it before the recipe is
		   discarded. */
		const f32_t fSourceSizeScale =
			Lowered.Detail.Particle.SourceScale.fSize;
		if (std::isfinite(fSourceSizeScale) && fSourceSizeScale > 0.f &&
			fSourceSizeScale != 1.f)
		{
			Lowered.Detail.Particle.vStartSize.x *= fSourceSizeScale;
			Lowered.Detail.Particle.vStartSize.y *= fSourceSizeScale;
			Lowered.Detail.Particle.vEndSize.x *= fSourceSizeScale;
			Lowered.Detail.Particle.vEndSize.y *= fSourceSizeScale;
			Lowered.Detail.Particle.SourceScale.fSize = 1.f;
		}
	}
	if (Lowered.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
		Lowered.Detail.Particle.fSpawnRatePerSecond <= 0.f &&
		0u == Lowered.Detail.Particle.iBurstCount &&
		Lowered.Detail.Particle.fFixedCenterSpacingWorldUnits <= 0.f)
	{
		/* Source-authored Particle carriers may receive every occurrence from
		   SourceRecipe while their editable direct emission stays at 0/0.  The
		   generic copy intentionally drops that compiler/source ownership, so
		   give the lowered authoring carrier the same bounded one-shot default
		   used by Create Element instead of admitting a drawable that can never
		   instantiate a Sprite/Mesh Particle. */
		Lowered.Detail.Particle.iMaxParticles = (std::max)(
			1u, Lowered.Detail.Particle.iMaxParticles);
		Lowered.Detail.Particle.iBurstCount = 1u;
	}
	const auto IsZeroFloat3 = [](const float3_t& Value)
	{
		return Value.x == 0.f && Value.y == 0.f && Value.z == 0.f;
	};
	const EFFECT_LINEAR_LERP_DESC& LinearLerp = Lowered.Detail.LinearLerp;
	if (Lowered.eKind == EFFECT_ELEMENT_KIND::TRAIL &&
		IsZeroFloat3(Lowered.Detail.Transform.vVelocityPerSecond) &&
		IsZeroFloat3(Lowered.Detail.Transform.vRevolutionDegreesPerSecond) &&
		!LinearLerp.bPosition && !LinearLerp.bRotation &&
		!LinearLerp.bRevolution && !LinearLerp.bScale &&
		!LinearLerp.bVelocity)
	{
		/* A Trail needs at least two distinct carrier samples to become
		   visible.  Source attachment/occurrence motion was intentionally
		   removed above, so seed a bounded authoring-only carrier velocity
		   only when the selected recipe has no surviving motion of its own. */
		Lowered.Detail.Transform.vVelocityPerSecond = { 0.f, 0.f, 1.f };
	}

	/* The ordinary serializer deliberately omits native Renderer, compiler,
	   geometry, admission, and distribution-evidence lanes.  The explicit
	   clearing above also removes source occurrence/attachment ownership while
	   preserving WModel/DDS bindings, an already compiled authored Material
	   execution snapshot, and editable Detail values.  No source receipt or
	   adapter lookup remains necessary after that snapshot exists. */
	const std::string Canonical = Serialize(Candidate);
	EFFECT_DOCUMENT_DESC Staged;
	if (!Parse(Canonical, Staged, strOutError) ||
		!Validate_Drawable(Staged, strOutError) ||
		Staged.Elements.size() != 1u ||
		Staged.Elements.front().strElementId != strElementId ||
		Staged.Elements.front().eKind != First->eKind ||
		Staged.Elements.front().Renderer.eType != EFFECT_RENDERER_TYPE::END ||
		Staged.Elements.front().Renderer.eSourceSpace !=
			EFFECT_SOURCE_SPACE::END ||
		!Staged.Elements.front().strSourceNode.empty() ||
		Staged.Elements.front().ActionCueAttachment.bEnabled ||
		Staged.Elements.front().TransformInheritance.bEnabled ||
		Staged.Elements.front().SourceRecipe.bEnabled ||
		Staged.Elements.front().SourcePresentation.bEnabled ||
		Serialize(Staged) != Canonical)
	{
		if (strOutError.empty())
		{
			strOutError =
				"Generic authored starting copy did not survive ordinary codec validation exactly.";
		}
		return false;
	}
	InOutDocument = std::move(Staged);
	strOutError.clear();
	return true;
}


bool_t Client::CEffectDocumentCodec::Build_PortableAuthoredElementStartingCopy(
	const EFFECT_DOCUMENT_DESC& SourceDocument,
	const std::string_view strElementId,
	const std::string_view strTargetEffectAssetId,
	EFFECT_DOCUMENT_DESC& InOutDocument,
	std::string& strOutError)
{
	if (SourceDocument.bSourceContract)
	{
		strOutError =
			"Portable authored Saved Element copy accepts only an admitted ordinary v13 authored document, not native source-contract evidence.";
		return false;
	}
	const auto Source = std::find_if(SourceDocument.Elements.begin(),
		SourceDocument.Elements.end(),
		[strElementId](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == strElementId;
		});
	if (Source == SourceDocument.Elements.end() ||
		std::find_if(std::next(Source), SourceDocument.Elements.end(),
			[strElementId](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == strElementId;
			}) != SourceDocument.Elements.end())
	{
		strOutError =
			"Portable authored Saved Element copy requires exactly one source Element.";
		return false;
	}
	if ((Source->eKind != EFFECT_ELEMENT_KIND::MESH &&
		 Source->eKind != EFFECT_ELEMENT_KIND::SPRITE &&
		 Source->eKind != EFFECT_ELEMENT_KIND::PARTICLE &&
		 Source->eKind != EFFECT_ELEMENT_KIND::DECAL &&
		 Source->eKind != EFFECT_ELEMENT_KIND::TRAIL) ||
		!Is_EffectElementAuthoringExecutionTarget(*Source))
	{
		strOutError =
			"Portable authored Saved Element copy requires a self-contained executable Mesh, Sprite, Particle, Decal, or Trail Element.";
		return false;
	}
	if (!Source->bVisible)
	{
		strOutError =
			"Portable authored Saved Element copy cannot turn an invisible source Element into a visible occurrence; load the complete Effect instead.";
		return false;
	}
	if (Source->eKind == EFFECT_ELEMENT_KIND::TRAIL)
	{
		/* A Trail is geometry made from more than one owner/animation transform
		   history sample.  Copying its editable material and width fields cannot
		   make that history portable, even when the row has no explicit source
		   recipe.  Keep this a single fail-closed boundary instead of inventing a
		   second synthetic-motion Trail Family for Saved Element reuse. */
		strOutError =
			"Portable authored Saved Element copy cannot detach Trail transform history; load the complete Effect instead.";
		return false;
	}
	if (Source->SourceRecipe.bSimulationOnly ||
		!Source->SourceRecipe.strParticleSystemOccurrenceId.empty())
	{
		strOutError = "Saved Element copy cannot detach a live particle provider; use the complete Effect or Element Timeline Solo.";
		return false;
	}
	if (Source->TransformInheritance.bEnabled)
	{
		strOutError =
			"Portable authored Saved Element copy cannot detach a transform-inheritance dependent from its master; load the complete Effect instead.";
		return false;
	}
	if (Source->SourcePresentation.bEnabled)
	{
		strOutError =
			"Portable authored Saved Element copy cannot detach SourcePresentation occurrence ownership; load the complete Effect instead.";
		return false;
	}
	if (Source->Renderer.eType != EFFECT_RENDERER_TYPE::END ||
		Source->Renderer.eSourceSpace != EFFECT_SOURCE_SPACE::END)
	{
		strOutError =
			"Portable authored Saved Element copy cannot detach a native Renderer carrier; load the complete source-program Effect instead.";
		return false;
	}
	const bool_t bDetachNativeOwnerYawSprite =
		Source->ActionCueAttachment.bEnabled &&
		Source->ActionCueAttachment.bFollow &&
		Source->ActionCueAttachment.eOrientation ==
			EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW &&
		Source->eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
		Source->Detail.Particle.bBillboard &&
		(Source->strSourceNode.empty() ||
		 Source->strSourceNode.starts_with("authored-copy:")) &&
		!Source->SourceRecipe.bEnabled && Source->RuntimeCarrier.Is_Empty() &&
		Source->Material.strTemplateId == EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
		Source->Material.strSourceMaterialPath.empty() &&
		!Source->Material.SourceMaterial.bEnabled &&
		!Source->Material.Execution.bEnabled &&
		!Source->Material.Execution.bFailClosed &&
		!Source->Material.Execution.bAuthoringApproximate &&
		std::none_of(Source->ResourceBindings.begin(), Source->ResourceBindings.end(),
			[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{ return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID; });
	/* Camera-view attachments resolve from the active view and their socket,
	   independently of the source animation or a model cue. Preserve that
	   attachment when reusing the Element instead of treating it as bone history. */
	const bool_t bPortableCameraViewFollow =
		Source->ActionCueAttachment.bEnabled &&
		Source->ActionCueAttachment.bFollow &&
		Source->ActionCueAttachment.eOrientation ==
			EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW &&
		Source->ActionCueAttachment.strRuntimeBoneName.empty() &&
		Source->ActionCueAttachment.strModelCueId.empty();
	if ((bDetachNativeOwnerYawSprite || bPortableCameraViewFollow) &&
		!Validate(SourceDocument, strOutError))
		return false;
	if (Source->ActionCueAttachment.bFollow &&
		!bDetachNativeOwnerYawSprite && !bPortableCameraViewFollow)
	{
		/* A matching textual owner prefix does not prove that the target preview
		   owns the source bone, clip, or fixed-step anchor history.  Snapshot/root
		   attachments remain portable; source-owned FOLLOW rows require the
		   complete Effect. Native owner-yaw sprite emitters copy only their
		   authored settings and must be attached again in the target Effect. */
		strOutError =
			"Portable authored Saved Element copy cannot detach a FOLLOW attachment from its owner animation history; load the complete Effect instead.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Candidate;
	Candidate.strEffectAssetId = std::string(strTargetEffectAssetId);
	Candidate.strDisplayName = Source->strDisplayName;
	Candidate.ParticleSystem = SourceDocument.ParticleSystem;
	Candidate.Elements.push_back(*Source);
	EFFECT_ELEMENT_DESC& Portable = Candidate.Elements.front();
	/* Saved Element reuse strips only native compiler/occurrence ownership.
	   Unlike the generic fallback path, it must never validate an intermediate
	   recipe-less Particle because StandardColor and other executable materials
	   require their renderer Family recipe as part of material admission. */
	Portable.strGroupId = "manual.authoring";
	Portable.Renderer = {};
	Portable.TransformInheritance = {};
	Portable.SourceRecipe = {};
	Portable.SourcePresentation = {};
	/* Reassert the complete editable payload in one assignment so size,
	   SourceScale, TypeData rotation, timing, color, motion, and every future
	   Effect Detail field cannot drift when this boundary evolves. */
	Portable.Detail = Source->Detail;
	Portable.ActionCueAttachment = Source->ActionCueAttachment;
	if (bDetachNativeOwnerYawSprite)
		Portable.ActionCueAttachment = {};
	constexpr std::string_view AuthoredCopyPrefix = "authored-copy:";
	Portable.strSourceNode =
		Source->strSourceNode.starts_with(AuthoredCopyPrefix) &&
		Source->strSourceNode.size() > AuthoredCopyPrefix.size() ?
			Source->strSourceNode :
			std::string(AuthoredCopyPrefix) + Source->strElementId;

	if (Source->SourceRecipe.bEnabled)
	{
		bool_t bCarrierApplied = false;
		if (EFFECT_ELEMENT_KIND::PARTICLE == Source->eKind)
		{
			bCarrierApplied = Apply_PortableAuthoredParticleRuntimeCarrier(
				*Source, Portable, strOutError);
		}
		else if (EFFECT_ELEMENT_KIND::DECAL == Source->eKind)
		{
			bCarrierApplied = Apply_PortableAuthoredDecalRuntimeCarrier(
				*Source, Portable, strOutError);
		}
		else
		{
			strOutError =
				"Portable authored Saved Element copy has no self-contained runtime carrier for this sourceRecipe Family.";
			return false;
		}
		if (!bCarrierApplied)
			return false;
		/* Generic import bakes emitter delay into its sampled starting state, but
		   Saved Element reuse does not sample or bake. Preserve the source delay
		   so Detail.startDelay + recipe.emitterDelay remains exactly unchanged. */
		Portable.SourceRecipe.fEmitterDelaySeconds =
			Source->SourceRecipe.fEmitterDelaySeconds;
	}

	const std::string Canonical = Serialize(Candidate);
	EFFECT_DOCUMENT_DESC Staged;
	if (!Parse(Canonical, Staged, strOutError) ||
		!Validate(Staged, strOutError) ||
		Staged.Elements.size() != 1u ||
		Staged.Elements.front().strElementId != strElementId ||
		Staged.Elements.front().eKind != Source->eKind ||
		Staged.Elements.front().Renderer.eType != EFFECT_RENDERER_TYPE::END ||
		Staged.Elements.front().Renderer.eSourceSpace !=
			EFFECT_SOURCE_SPACE::END ||
		Staged.Elements.front().TransformInheritance.bEnabled ||
		Staged.Elements.front().SourcePresentation.bEnabled ||
		Serialize(Staged) != Canonical)
	{
		if (strOutError.empty())
		{
			strOutError =
				"Portable authored Saved Element copy did not survive ordinary codec validation exactly.";
		}
		return false;
	}
	InOutDocument = std::move(Staged);
	strOutError.clear();
	return true;
}

namespace Client::EffectDocumentCodecDetail
{

	constexpr f32_t GENERIC_STARTING_BAKE_AFFINE_EPSILON = 0.00001f;

	constexpr f32_t GENERIC_STARTING_BAKE_MATRIX_EPSILON = 0.0002f;


	bool_t Is_FiniteMatrix(const float4x4_t& Value)
	{
		const f32_t* pComponent = &Value._11;
		for (size_t i = 0u; i < 16u; ++i)
		{
			if (!std::isfinite(pComponent[i]))
				return false;
		}
		return true;
	}


	bool_t Is_AffineMatrix(const float4x4_t& Value)
	{
		return std::abs(Value._14) <= GENERIC_STARTING_BAKE_AFFINE_EPSILON &&
			std::abs(Value._24) <= GENERIC_STARTING_BAKE_AFFINE_EPSILON &&
			std::abs(Value._34) <= GENERIC_STARTING_BAKE_AFFINE_EPSILON &&
			std::abs(Value._44 - 1.f) <=
				GENERIC_STARTING_BAKE_AFFINE_EPSILON;
	}


	bool_t Matrices_NearlyEqual(const float4x4_t& Left,
		const float4x4_t& Right)
	{
		const f32_t* pLeft = &Left._11;
		const f32_t* pRight = &Right._11;
		for (size_t i = 0u; i < 16u; ++i)
		{
			const f32_t fScale = (std::max)(1.f,
				(std::max)(std::abs(pLeft[i]), std::abs(pRight[i])));
			if (std::abs(pLeft[i] - pRight[i]) >
				GENERIC_STARTING_BAKE_MATRIX_EPSILON * fScale)
			{
				return false;
			}
		}
		return true;
	}


	matrix_t Build_AuthoredTransformMatrix(
		const EFFECT_TRANSFORM_DESC& Transform)
	{
		return XMMatrixScaling(
			Transform.vScale.x, Transform.vScale.y, Transform.vScale.z) *
			XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(Transform.vRotationDegrees.x),
				XMConvertToRadians(Transform.vRotationDegrees.y),
				XMConvertToRadians(Transform.vRotationDegrees.z)) *
			XMMatrixTranslation(
				Transform.vPosition.x,
				Transform.vPosition.y,
				Transform.vPosition.z);
	}


	matrix_t Build_SourceTypeDataRotation(
		const float3_t& SourceDegrees)
	{
		/* Source TypeDataMesh rotation is UE roll(X), pitch(Y), yaw(Z).
		   Preserve its source Euler composition, then conjugate it into the
		   Client X-forward/Y-up basis exactly as source playback does. */
		const f32_t Roll = XMConvertToRadians(SourceDegrees.x);
		const f32_t Pitch = XMConvertToRadians(SourceDegrees.y);
		const f32_t Yaw = XMConvertToRadians(SourceDegrees.z);
		const f32_t SP = std::sin(Pitch);
		const f32_t CP = std::cos(Pitch);
		const f32_t SY = std::sin(Yaw);
		const f32_t CY = std::cos(Yaw);
		const f32_t SR = std::sin(Roll);
		const f32_t CR = std::cos(Roll);
		const matrix_t Source = XMMatrixSet(
			CP * CY, CP * SY, SP, 0.f,
			SR * SP * CY - CR * SY,
			SR * SP * SY + CR * CY, -SR * CP, 0.f,
			-CR * SP * CY - SR * SY,
			-CR * SP * SY + SR * CY, CR * CP, 0.f,
			0.f, 0.f, 0.f, 1.f);
		const matrix_t Basis = XMMatrixSet(
			1.f, 0.f, 0.f, 0.f,
			0.f, 0.f, -1.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 1.f);
		return XMMatrixTranspose(Basis) * Source * Basis;
	}


	bool_t Decompose_AuthoredTransform(fmatrix_t Matrix,
		EFFECT_TRANSFORM_DESC& OutTransform, std::string& strOutError)
	{
		float4x4_t Stored{};
		XMStoreFloat4x4(&Stored, Matrix);
		if (!Is_FiniteMatrix(Stored) || !Is_AffineMatrix(Stored))
		{
			strOutError =
				"Generic authored starting-state bake requires a finite affine transform.";
			return false;
		}

		const f32_t fDeterminant = XMVectorGetX(XMMatrixDeterminant(Matrix));
		if (!std::isfinite(fDeterminant) || fDeterminant <= 0.f)
		{
			strOutError =
				"Generic authored starting-state bake rejects degenerate or reflected transforms.";
			return false;
		}

		vector_t Scale{};
		vector_t RotationQuaternion{};
		vector_t Translation{};
		if (!XMMatrixDecompose(
				&Scale, &RotationQuaternion, &Translation, Matrix))
		{
			strOutError =
				"Generic authored starting-state bake could not decompose the transform.";
			return false;
		}

		float3_t DecomposedScale{};
		float3_t DecomposedTranslation{};
		XMStoreFloat3(&DecomposedScale, Scale);
		XMStoreFloat3(&DecomposedTranslation, Translation);
		if (!Is_Finite(DecomposedScale) || !Is_Finite(DecomposedTranslation) ||
			DecomposedScale.x <= 0.f || DecomposedScale.y <= 0.f ||
			DecomposedScale.z <= 0.f)
		{
			strOutError =
				"Generic authored starting-state bake requires positive finite scale.";
			return false;
		}

		const matrix_t RotationMatrix =
			XMMatrixRotationQuaternion(XMQuaternionNormalize(RotationQuaternion));
		float4x4_t StoredRotation{};
		XMStoreFloat4x4(&StoredRotation, RotationMatrix);
		const f32_t fCosPitch = std::sqrt(
			StoredRotation._33 * StoredRotation._33 +
			StoredRotation._31 * StoredRotation._31);
		float3_t EulerRadians{};
		EulerRadians.x = std::atan2(-StoredRotation._32, fCosPitch);
		if (fCosPitch > 16.f * (std::numeric_limits<f32_t>::epsilon)())
		{
			EulerRadians.y =
				std::atan2(StoredRotation._31, StoredRotation._33);
			EulerRadians.z =
				std::atan2(StoredRotation._12, StoredRotation._22);
		}
		else
		{
			EulerRadians.y = 0.f;
			EulerRadians.z =
				std::atan2(-StoredRotation._21, StoredRotation._11);
		}

		EFFECT_TRANSFORM_DESC Candidate = OutTransform;
		Candidate.vPosition = DecomposedTranslation;
		Candidate.vRotationDegrees = {
			XMConvertToDegrees(EulerRadians.x),
			XMConvertToDegrees(EulerRadians.y),
			XMConvertToDegrees(EulerRadians.z)
		};
		Candidate.vScale = DecomposedScale;
		float4x4_t Recomposed{};
		XMStoreFloat4x4(&Recomposed,
			Build_AuthoredTransformMatrix(Candidate));
		if (!Is_Finite(Candidate.vRotationDegrees) ||
			!Matrices_NearlyEqual(Stored, Recomposed))
		{
			strOutError =
				"Generic authored starting-state bake rejects shear or a lossy Euler decomposition.";
			return false;
		}
		OutTransform = Candidate;
		return true;
	}

}


bool_t Client::CEffectDocumentCodec::Bake_GenericAuthoredElementStartingState(
	const EFFECT_ELEMENT_DESC& LoweredElement,
	const EFFECT_GENERIC_AUTHORED_STARTING_BAKE_REQUEST& Request,
	EFFECT_ELEMENT_DESC& OutBakedElement,
	std::string& strOutError)
{
	if (LoweredElement.Renderer.eType != EFFECT_RENDERER_TYPE::END ||
		LoweredElement.Renderer.eSourceSpace != EFFECT_SOURCE_SPACE::END ||
		!LoweredElement.strSourceNode.empty() ||
		LoweredElement.ActionCueAttachment.bEnabled ||
		LoweredElement.TransformInheritance.bEnabled ||
		LoweredElement.SourceRecipe.bEnabled ||
		LoweredElement.SourcePresentation.bEnabled ||
		LoweredElement.Detail.Mesh.vSourceTypeDataRotationDegrees.x != 0.f ||
		LoweredElement.Detail.Mesh.vSourceTypeDataRotationDegrees.y != 0.f ||
		LoweredElement.Detail.Mesh.vSourceTypeDataRotationDegrees.z != 0.f)
	{
		strOutError =
			"Generic authored starting-state bake requires an already lowered ordinary Element.";
		return false;
	}
	if (Request.bTransformInheritanceEnabled)
	{
		strOutError =
			"Generic authored starting-state bake cannot flatten transform inheritance.";
		return false;
	}
	if ((!Request.bAttachmentEnabled &&
			(Request.bFollowAttachment ||
			 Request.bHasFollowParentLocalTransform ||
			 Request.fSnapshotRootSourceBasisYawDegrees != 0.f)) ||
		(!Request.bFollowAttachment &&
			Request.bHasFollowParentLocalTransform) ||
		(Request.bFollowAttachment &&
			(!Request.bHasFollowParentLocalTransform ||
			 Request.fSnapshotRootSourceBasisYawDegrees != 0.f)))
	{
		strOutError = Request.bFollowAttachment ?
			"Generic authored starting-state bake requires the exact emit-start follow parent-local transform." :
			"Generic authored starting-state bake has an invalid attachment request.";
		return false;
	}

	const EFFECT_TRANSFORM_DESC& Cue = Request.CueLocalTransform;
	const EFFECT_TRANSFORM_DESC& EmitterLocal = Request.EmitterLocalTransform;
	if (!Is_Finite(Cue.vPosition) || !Is_Finite(Cue.vRotationDegrees) ||
		!Is_Finite(Cue.vScale) || Cue.vScale.x <= 0.f ||
		Cue.vScale.y <= 0.f || Cue.vScale.z <= 0.f ||
		!Is_Finite(EmitterLocal.vPosition) ||
		!Is_Finite(EmitterLocal.vRotationDegrees) ||
		!Is_Finite(EmitterLocal.vScale) || EmitterLocal.vScale.x <= 0.f ||
		EmitterLocal.vScale.y <= 0.f || EmitterLocal.vScale.z <= 0.f ||
		!Is_Finite(Request.vSourceTypeDataRotationDegrees) ||
		!std::isfinite(Request.fScheduleStartDelaySeconds) ||
		!std::isfinite(Request.fScheduleLifeTimeSeconds) ||
		!std::isfinite(Request.fEmitterDelaySeconds) ||
		!std::isfinite(Request.fEmitterDurationSeconds) ||
		!std::isfinite(Request.fSnapshotRootSourceBasisYawDegrees) ||
		std::abs(Request.fSnapshotRootSourceBasisYawDegrees) > 3600.f ||
		std::abs(Request.vSourceTypeDataRotationDegrees.x) > 3600.f ||
		std::abs(Request.vSourceTypeDataRotationDegrees.y) > 3600.f ||
		std::abs(Request.vSourceTypeDataRotationDegrees.z) > 3600.f ||
		Request.fScheduleStartDelaySeconds < 0.f ||
		Request.fScheduleLifeTimeSeconds <= 0.f ||
		Request.fEmitterDelaySeconds < 0.f ||
		Request.fEmitterDurationSeconds < 0.f)
	{
		strOutError =
			"Generic authored starting-state bake request contains invalid transform or timing values.";
		return false;
	}

	const f32_t fStartDelay = Request.fScheduleStartDelaySeconds +
		Request.fEmitterDelaySeconds;
	const f32_t fLifeTime =
		Request.fEmitterDurationSeconds > 0.f &&
		Request.iEmitterLoopCount != 0u ?
		Request.fEmitterDurationSeconds *
			static_cast<f32_t>(Request.iEmitterLoopCount) :
		Request.fScheduleLifeTimeSeconds;
	if (!std::isfinite(fStartDelay) || !std::isfinite(fLifeTime) ||
		fStartDelay < 0.f || fLifeTime <= 0.f)
	{
		strOutError =
			"Generic authored starting-state bake timing overflows the ordinary contract.";
		return false;
	}

	matrix_t ParentLocal = XMMatrixIdentity();
	if (Request.bAttachmentEnabled)
	{
		if (Request.bFollowAttachment)
		{
			if (!Is_FiniteMatrix(Request.FollowParentLocalTransform) ||
				!Is_AffineMatrix(Request.FollowParentLocalTransform))
			{
				strOutError =
					"Generic authored starting-state bake follow parent is not finite affine.";
				return false;
			}
			ParentLocal = XMLoadFloat4x4(
				&Request.FollowParentLocalTransform);
		}
		else
		{
			ParentLocal = XMMatrixRotationY(XMConvertToRadians(
				Request.fSnapshotRootSourceBasisYawDegrees));
		}
	}

	const matrix_t BakedMatrix =
		Build_SourceTypeDataRotation(
			Request.vSourceTypeDataRotationDegrees) *
		Build_AuthoredTransformMatrix(EmitterLocal) *
		Build_AuthoredTransformMatrix(Cue) * ParentLocal;
	EFFECT_ELEMENT_DESC Candidate = LoweredElement;
	EFFECT_TRANSFORM_DESC BakedTransform = Candidate.Detail.Transform;
	if (!Decompose_AuthoredTransform(
			BakedMatrix, BakedTransform, strOutError))
	{
		return false;
	}
	Candidate.Detail.Transform.vPosition = BakedTransform.vPosition;
	Candidate.Detail.Transform.vRotationDegrees =
		BakedTransform.vRotationDegrees;
	Candidate.Detail.Transform.vScale = BakedTransform.vScale;
	Candidate.Detail.Timing.fStartDelaySeconds = fStartDelay;
	Candidate.Detail.Timing.fLifeTimeSeconds = fLifeTime;
	Candidate.Detail.Mesh.vSourceTypeDataRotationDegrees = {};

	OutBakedElement = std::move(Candidate);
	strOutError.clear();
	return true;
}


bool_t Client::CEffectDocumentCodec::Merge_GenericAuthoredElements(
	const EFFECT_DOCUMENT_DESC& TargetDocument,
	const std::vector<EFFECT_ELEMENT_DESC>& Elements,
	EFFECT_DOCUMENT_DESC& InOutDocument,
	std::string& strOutError)
{
	if (!Validate_Drawable(TargetDocument, strOutError))
		return false;
	if (Elements.empty())
	{
		strOutError =
			"Generic authored Element merge requires at least one Element.";
		return false;
	}

	std::unordered_set<std::string> ElementIds;
	ElementIds.reserve(TargetDocument.Elements.size() + Elements.size());
	for (const EFFECT_ELEMENT_DESC& Element : TargetDocument.Elements)
		ElementIds.insert(Element.strElementId);

	EFFECT_DOCUMENT_DESC Candidate = TargetDocument;
	Candidate.Elements.reserve(TargetDocument.Elements.size() + Elements.size());
	for (const EFFECT_ELEMENT_DESC& Element : Elements)
	{
		if (!Is_StableId(Element.strElementId))
		{
			strOutError =
				"Generic authored Element merge requires an explicit stable target Element ID.";
			return false;
		}
		if (!ElementIds.insert(Element.strElementId).second)
		{
			strOutError =
				"Generic authored Element merge rejects duplicate target Element IDs.";
			return false;
		}
		if (Element.eKind != EFFECT_ELEMENT_KIND::MESH &&
			Element.eKind != EFFECT_ELEMENT_KIND::SPRITE &&
			Element.eKind != EFFECT_ELEMENT_KIND::PARTICLE &&
			Element.eKind != EFFECT_ELEMENT_KIND::DECAL &&
			Element.eKind != EFFECT_ELEMENT_KIND::TRAIL)
		{
			strOutError =
				"Generic authored Element merge supports Mesh, Sprite, Particle, Decal, and Trail Elements only.";
			return false;
		}
		if (Element.Renderer.eType != EFFECT_RENDERER_TYPE::END ||
			Element.Renderer.eSourceSpace != EFFECT_SOURCE_SPACE::END ||
			Element.Detail.Mesh.vSourceTypeDataRotationDegrees.x != 0.f ||
			Element.Detail.Mesh.vSourceTypeDataRotationDegrees.y != 0.f ||
			Element.Detail.Mesh.vSourceTypeDataRotationDegrees.z != 0.f)
		{
			strOutError =
				"Generic authored Element merge rejects native renderer state.";
			return false;
		}

		EFFECT_DOCUMENT_DESC ProvenanceProbe = TargetDocument;
		ProvenanceProbe.ModelCues.clear();
		ProvenanceProbe.Elements.assign(1u, Element);
		EFFECT_DOCUMENT_DESC ClearedProbe = ProvenanceProbe;
		EFFECT_ELEMENT_DESC& Cleared = ClearedProbe.Elements.front();
		Cleared.strSourceNode.clear();
		Cleared.Renderer = {};
		Cleared.ActionCueAttachment = {};
		Cleared.TransformInheritance = {};
		Cleared.SourceRecipe = {};
		Cleared.SourcePresentation = {};
		Cleared.Detail.Mesh.vSourceTypeDataRotationDegrees = {};
		if (Serialize(ProvenanceProbe) != Serialize(ClearedProbe))
		{
			strOutError =
				"Generic authored Element merge requires source provenance to remain in the migration binding.";
			return false;
		}

		Candidate.Elements.push_back(Element);
	}

	if (!Validate_Drawable(Candidate, strOutError))
		return false;
	const std::string Canonical = Serialize(Candidate);
	EFFECT_DOCUMENT_DESC Staged;
	if (!Parse(Canonical, Staged, strOutError) ||
		!Validate_Drawable(Staged, strOutError) ||
		Staged.strEffectAssetId != TargetDocument.strEffectAssetId ||
		Staged.Elements.size() != Candidate.Elements.size() ||
		Serialize(Staged) != Canonical)
	{
		if (strOutError.empty())
		{
			strOutError =
				"Generic authored Element merge did not survive canonical validation exactly.";
		}
		return false;
	}
	const size_t iFirstMerged = TargetDocument.Elements.size();
	for (size_t i = 0u; i < Elements.size(); ++i)
	{
		const EFFECT_ELEMENT_DESC& Expected = Elements[i];
		const EFFECT_ELEMENT_DESC& Actual = Staged.Elements[iFirstMerged + i];
		if (Actual.strElementId != Expected.strElementId ||
			Actual.strGroupId != Expected.strGroupId ||
			Actual.strDisplayName != Expected.strDisplayName)
		{
			strOutError =
				"Generic authored Element merge changed caller-owned identity or display metadata.";
			return false;
		}
	}

	InOutDocument = std::move(Staged);
	strOutError.clear();
	return true;
}


bool_t Client::CEffectDocumentCodec::Build_GenericAuthoredElementImportStage(
	const EFFECT_DOCUMENT_DESC& SourceDocument,
	const EFFECT_DOCUMENT_DESC& TargetDocument,
	const EFFECT_GENERIC_AUTHORED_ELEMENT_IMPORT_REQUEST& Request,
	EFFECT_DOCUMENT_DESC& InOutDocument,
	std::string& strOutError)
{
	if (Request.strSourceElementId.empty() ||
		Request.strTargetElementId.empty() ||
		Request.strTargetGroupId.empty() ||
		Request.strTargetDisplayName.empty() ||
		TargetDocument.strEffectAssetId.empty())
	{
		strOutError =
			"Generic authored import requires explicit source, target, group, display, and Effect IDs.";
		return false;
	}

	const auto Source = std::find_if(SourceDocument.Elements.begin(),
		SourceDocument.Elements.end(),
		[&Request](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == Request.strSourceElementId;
		});
	if (Source == SourceDocument.Elements.end() ||
		std::find_if(std::next(Source), SourceDocument.Elements.end(),
			[&Request](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == Request.strSourceElementId;
			}) != SourceDocument.Elements.end())
	{
		strOutError =
			"Generic authored import requires exactly one source Element.";
		return false;
	}

	const std::string SourceCanonicalBefore = Serialize(SourceDocument);
	const std::string TargetCanonicalBefore = Serialize(TargetDocument);
	if (Source->SourceRecipe.bEnabled)
	{
		const auto& Starting = Request.StartingState;
		if (Source->eKind != EFFECT_ELEMENT_KIND::PARTICLE ||
			Starting.fScheduleStartDelaySeconds !=
				Source->Detail.Timing.fStartDelaySeconds ||
			Starting.fScheduleLifeTimeSeconds !=
				Source->Detail.Timing.fLifeTimeSeconds ||
			Starting.fEmitterDelaySeconds !=
				Source->SourceRecipe.fEmitterDelaySeconds ||
			Starting.fEmitterDurationSeconds !=
				Source->SourceRecipe.fEmitterDurationSeconds ||
			Starting.iEmitterLoopCount !=
				Source->SourceRecipe.iEmitterLoopCount ||
			Starting.bAttachmentEnabled !=
				Source->ActionCueAttachment.bEnabled ||
			Starting.bFollowAttachment !=
				Source->ActionCueAttachment.bFollow ||
			Starting.fSnapshotRootSourceBasisYawDegrees !=
				Source->ActionCueAttachment.
					fSnapshotRootSourceBasisYawDegrees ||
			Starting.bTransformInheritanceEnabled !=
				Source->TransformInheritance.bEnabled)
		{
			strOutError =
				"Generic authored import starting state does not exactly identify its source Particle occurrence.";
			return false;
		}
	}

	EFFECT_DOCUMENT_DESC LoweredDocument;
	if (!Build_GenericAuthoredElementStartingCopy(
			SourceDocument, Request.strSourceElementId,
			TargetDocument.strEffectAssetId, LoweredDocument, strOutError) ||
		LoweredDocument.Elements.size() != 1u)
	{
		return false;
	}

	EFFECT_ELEMENT_DESC BakedElement;
	if (!Bake_GenericAuthoredElementStartingState(
			LoweredDocument.Elements.front(), Request.StartingState,
			BakedElement, strOutError))
	{
		return false;
	}
	BakedElement.strElementId = Request.strTargetElementId;
	BakedElement.strGroupId = Request.strTargetGroupId;
	BakedElement.strDisplayName = Request.strTargetDisplayName;
	if (Request.bOverrideMaterialExecution)
	{
		BakedElement.Material.SourceMaterial = {};
		BakedElement.Material.Execution = Request.MaterialExecution;
	}

	EFFECT_DOCUMENT_DESC MergedDocument;
	if (!Merge_GenericAuthoredElements(TargetDocument, { BakedElement },
			MergedDocument, strOutError))
	{
		return false;
	}
	const auto Imported = std::find_if(MergedDocument.Elements.begin(),
		MergedDocument.Elements.end(),
		[&Request](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == Request.strTargetElementId;
		});
	if (Imported == MergedDocument.Elements.end())
	{
		strOutError =
			"Generic authored import lost its stable target Element identity.";
		return false;
	}
	if (Source->SourceRecipe.bEnabled &&
		!Apply_PortableAuthoredParticleRuntimeCarrier(
			*Source, *Imported, strOutError))
	{
		return false;
	}

	const std::string Canonical = Serialize(MergedDocument);
	EFFECT_DOCUMENT_DESC Staged;
	if (!Parse(Canonical, Staged, strOutError) ||
		!Validate_Drawable(Staged, strOutError) ||
		Serialize(Staged) != Canonical ||
		Serialize(SourceDocument) != SourceCanonicalBefore ||
		Serialize(TargetDocument) != TargetCanonicalBefore)
	{
		if (strOutError.empty())
		{
			strOutError =
				"Generic authored import did not survive canonical validation without mutating its inputs.";
		}
		return false;
	}
	const auto StagedElement = std::find_if(Staged.Elements.begin(),
		Staged.Elements.end(),
		[&Request](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == Request.strTargetElementId;
		});
	if (StagedElement == Staged.Elements.end() ||
		StagedElement->strGroupId != Request.strTargetGroupId ||
		StagedElement->strDisplayName != Request.strTargetDisplayName ||
		StagedElement->SourceRecipe.bEnabled !=
			Source->SourceRecipe.bEnabled ||
		(StagedElement->SourceRecipe.bEnabled &&
		 StagedElement->SourceRecipe.fEmitterDelaySeconds != 0.f))
	{
		strOutError =
			"Generic authored import changed target identity or portable Particle timing.";
		return false;
	}

	InOutDocument = std::move(Staged);
	strOutError.clear();
	return true;
}
