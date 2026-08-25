#include "Effect_VisualProgramCorpus.h"

#include "DataJson.h"
#include "Effect_DocumentCodec.h"
#include "Effect_RuntimeAuthority.h"

#include <algorithm>
#include <cmath>
#include <set>

namespace
{
	using namespace Client;

	bool_t Is_LowerSha256(const std::string_view Value)
	{
		return 64u == Value.size() && std::all_of(
			Value.begin(), Value.end(), [](const char Character)
			{
				return (Character >= '0' && Character <= '9') ||
					(Character >= 'a' && Character <= 'f');
			});
	}

	bool_t Is_StableId(const std::string_view Value)
	{
		return !Value.empty() && Value.size() <= 256u && std::all_of(
			Value.begin(), Value.end(), [](const char Character)
			{
				return (Character >= 'a' && Character <= 'z') ||
					(Character >= 'A' && Character <= 'Z') ||
					(Character >= '0' && Character <= '9') ||
					Character == '_' || Character == '.' || Character == '-';
			});
	}

	std::string Canonical_Sha(const DATA_JSON_VALUE& Value)
	{
		return CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
			CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(Value));
	}

	DATA_JSON_VALUE Normalize_TypedSemanticZero(const DATA_JSON_VALUE& Value)
	{
		switch (Value.Get_Type())
		{
		case DATA_JSON_TYPE::NUMBER:
			return DATA_JSON_VALUE::Number(
				Value.Get_Number() == 0.0 ? 0.0 : Value.Get_Number(),
				Value.Was_FloatingPointToken());
		case DATA_JSON_TYPE::ARRAY:
		{
			DATA_JSON_VALUE::ARRAY Normalized;
			Normalized.reserve(Value.Get_Array().size());
			for (const DATA_JSON_VALUE& Item : Value.Get_Array())
				Normalized.push_back(Normalize_TypedSemanticZero(Item));
			return DATA_JSON_VALUE::Array(std::move(Normalized));
		}
		case DATA_JSON_TYPE::OBJECT:
		{
			DATA_JSON_VALUE::OBJECT Normalized;
			for (const auto& [Name, Item] : Value.Get_Object())
				Normalized.emplace(Name, Normalize_TypedSemanticZero(Item));
			return DATA_JSON_VALUE::Object(
				std::move(Normalized), Value.Get_ObjectInsertionOrder());
		}
		case DATA_JSON_TYPE::BOOLEAN:
			return DATA_JSON_VALUE::Boolean(Value.Get_Boolean());
		case DATA_JSON_TYPE::STRING:
			return DATA_JSON_VALUE::String(Value.Get_String());
		default:
			return DATA_JSON_VALUE::Null();
		}
	}

	bool_t Parse_DocumentJson(
		const EFFECT_DOCUMENT_DESC& Document,
		DATA_JSON_VALUE& Out,
		std::string& strOutError)
	{
		const std::string Json = CEffectDocumentCodec::Serialize(Document);
		DATA_JSON_PARSE_LIMITS Limits;
		Limits.iMaximumBytes = 64u * 1024u * 1024u;
		Limits.iMaximumDepth = 64u;
		Limits.iMaximumValues = 3'000'000u;
		return CDataJson::Parse(Json, Out, strOutError, Limits);
	}

	EFFECT_ELEMENT_DESC* Find_Element(
		EFFECT_DOCUMENT_DESC& Document,
		const std::string_view ElementId)
	{
		EFFECT_ELEMENT_DESC* Found = nullptr;
		for (EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (Element.strElementId == ElementId)
			{
				if (nullptr != Found) return nullptr;
				Found = &Element;
			}
		}
		return Found;
	}

	const EFFECT_ELEMENT_DESC* Find_Element(
		const EFFECT_DOCUMENT_DESC& Document,
		const std::string_view ElementId)
	{
		const EFFECT_ELEMENT_DESC* Found = nullptr;
		for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (Element.strElementId == ElementId)
			{
				if (nullptr != Found) return nullptr;
				Found = &Element;
			}
		}
		return Found;
	}

	bool_t Materialize_CascadeRibbonSupplemental(
		EFFECT_DOCUMENT_DESC& Document,
		const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental,
		std::string& strOutError)
	{
		if (Supplemental.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON ||
			!Supplemental.CascadeRibbonPacket.has_value())
			return true;
		EFFECT_ELEMENT_DESC* Target = Find_Element(
			Document, Supplemental.TargetIdentity.strTargetElementId);
		const EFFECT_VISUAL_PROGRAM_CASCADE_RIBBON_PACKET& Packet =
			*Supplemental.CascadeRibbonPacket;
		if (nullptr == Target || Target->eKind != EFFECT_ELEMENT_KIND::TRAIL ||
			!Target->SourceRecipe.bEnabled ||
			Target->SourceRecipe.strRendererShape != "ribbon" ||
			Target->SourceRecipe.Modules.size() != Packet.iModuleCount)
		{
			strOutError = "CascadeRibbon supplemental target/SourceRecipe is stale.";
			return false;
		}
		Target->Detail.Trail.iMaxPoints = Packet.iOperationalMaxPoints;
		Target->Detail.Trail.fTilingDistanceWorldUnits =
			static_cast<f32_t>(Packet.fTilingDistance);
		Target->Detail.Trail.fDistanceTessellationStepWorldUnits =
			static_cast<f32_t>(Packet.fDistanceTessellationStepSize);
		Target->Detail.Trail.fPointLifeTimeSeconds =
			static_cast<f32_t>(Packet.Trail.fPointLifeTimeSeconds);
		Target->Detail.Trail.fSampleIntervalSeconds =
			static_cast<f32_t>(Packet.Trail.fSampleIntervalSeconds);
		Target->Detail.Trail.fMinimumDistance =
			static_cast<f32_t>(Packet.Trail.fMinimumDistance);
		Target->Detail.Trail.fStartWidth =
			static_cast<f32_t>(Packet.Trail.fStartWidth);
		Target->Detail.Trail.fEndWidth =
			static_cast<f32_t>(Packet.Trail.fEndWidth);
		Target->Detail.Trail.bFaceCamera = Packet.Trail.bFaceCamera;
		return true;
	}

	bool_t Append_LocalDecalRoleTextureEvidence(
		const EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_PACKET& Packet,
		EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError)
	{
		static constexpr std::array<std::string_view, 6u> Roles = {
			"HEIGHT", "DIFFUSE", "DISSOLVE", "NORMAL", "SPECULAR",
			"EMISSIVE" };
		static constexpr std::array<std::string_view, 6u> CardNames = {
			"Track A LocalDecal | HEIGHT",
			"Track A LocalDecal | DIFFUSE",
			"Track A LocalDecal | DISSOLVE",
			"Track A LocalDecal | NORMAL",
			"Track A LocalDecal | SPECULAR",
			"Track A LocalDecal | EMISSIVE" };
		constexpr std::string_view Group = "Track A LocalDecal DDS Roles";
		constexpr std::string_view SamplingEvidence =
			"track-a-local-decal-six-role-v1";

		EFFECT_SOURCE_MATERIAL_DESC& Source = Element.Material.SourceMaterial;
		if (Element.eKind != EFFECT_ELEMENT_KIND::DECAL || !Source.bEnabled ||
			Source.Textures.size() > 26u)
		{
			strOutError =
				"LocalDecal role evidence requires one ordinary staged Decal Material.";
			return false;
		}

		for (size_t iRole = 0u; iRole < Roles.size(); ++iRole)
		{
			const EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_SRV& Srv =
				Packet.Srvs[iRole];
			EFFECT_RESOURCE_FILE_KIND FileKind =
				EFFECT_RESOURCE_FILE_KIND::END;
			if (Srv.strRole != Roles[iRole] || Srv.strAssetId.empty() ||
				!CEffectDocumentCodec::Is_SafeResourceAssetId(
					Srv.strAssetId, &FileKind) ||
				FileKind != EFFECT_RESOURCE_FILE_KIND::TEXTURE)
			{
				strOutError =
					"LocalDecal role evidence lost its exact DDS role binding: " +
					std::string(Roles[iRole]);
				return false;
			}

			EFFECT_NAMED_TEXTURE_DESC Expected;
			Expected.strName = CardNames[iRole];
			Expected.strGroup = Group;
			/* The packet has a resolved runtime DDS identity, but no source-object
			   path field.  Reuse that exact identity here instead of inventing a
			   package path; this value is shown only as read-only provenance. */
			Expected.strSourceObjectPath = Srv.strAssetId;
			Expected.strAssetId = Srv.strAssetId;
			Expected.eAddressU = EFFECT_TEXTURE_ADDRESS_MODE::CLAMP;
			Expected.eAddressV = EFFECT_TEXTURE_ADDRESS_MODE::CLAMP;
			Expected.eColorSpace = Srv.bSrgb ?
				EFFECT_TEXTURE_COLOR_SPACE::SRGB :
				EFFECT_TEXTURE_COLOR_SPACE::LINEAR;
			Expected.strSamplingEvidence = SamplingEvidence;

			const auto Existing = std::find_if(
				Source.Textures.begin(), Source.Textures.end(),
				[&Expected](const EFFECT_NAMED_TEXTURE_DESC& Texture)
				{ return Texture.strName == Expected.strName; });
			if (Existing == Source.Textures.end())
			{
				Source.Textures.push_back(std::move(Expected));
				continue;
			}
			if (Existing->strGroup != Expected.strGroup ||
				Existing->strSourceObjectPath != Expected.strSourceObjectPath ||
				Existing->strAssetId != Expected.strAssetId ||
				Existing->eAddressU != Expected.eAddressU ||
				Existing->eAddressV != Expected.eAddressV ||
				Existing->eColorSpace != Expected.eColorSpace ||
				Existing->strSamplingEvidence != Expected.strSamplingEvidence)
			{
				strOutError =
					"LocalDecal role evidence alias conflicts with the admitted packet: " +
					Expected.strName;
				return false;
			}
		}
		return true;
	}

	constexpr double SOURCE_OWNED_RUNTIME_EPSILON = 5e-5;

	std::string Compute_SourceOwnedRuntimeSha(
		const std::string_view strDocumentSha256,
		const std::string_view strDomain,
		const std::string_view strStableId,
		const std::string_view strDependency = {})
	{
		std::string Material = "effect-source-owned-runtime-v1\n";
		Material.append(strDocumentSha256);
		Material.push_back('\n');
		Material.append(strDomain);
		Material.push_back('\n');
		Material.append(strStableId);
		Material.push_back('\n');
		Material.append(strDependency);
		Material.push_back('\n');
		return CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(Material);
	}

	bool_t Is_FiniteSourceRuntimeVector(const float3_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z);
	}

	std::array<double, 3u> To_SourceRuntimeArray(const float3_t& Value)
	{
		return { static_cast<double>(Value.x), static_cast<double>(Value.y),
			static_cast<double>(Value.z) };
	}

	void Copy_SourceRuntimeTiming(
		const EFFECT_TIMING_DESC& Source,
		EFFECT_VISUAL_PROGRAM_TRAIL_TIMING& Out)
	{
		Out.fStartDelaySeconds = Source.fStartDelaySeconds;
		Out.fLifeTimeSeconds = Source.fLifeTimeSeconds;
		Out.fAfterImageSeconds = Source.fAfterImageSeconds;
		Out.fDissolveStartNormalized = Source.fDissolveStartNormalized;
	}

	void Copy_SourceRuntimeAttachment(
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Source,
		EFFECT_VISUAL_PROGRAM_TRAIL_ATTACHMENT& Out)
	{
		Out.bEnabled = Source.bEnabled;
		Out.bFollow = Source.bFollow;
		Out.strSourceAnchorSlotId = Source.strSourceAnchorSlotId;
		Out.strRuntimeAnchorSlotId = Source.strRuntimeAnchorSlotId;
		Out.strRuntimeBoneName = Source.strRuntimeBoneName;
		Out.fSnapshotRootSourceBasisYawDegrees =
			Source.fSnapshotRootSourceBasisYawDegrees;
		Out.vPosition = To_SourceRuntimeArray(
			Source.SocketLocalTransform.vPosition);
		Out.vRotationDegrees = To_SourceRuntimeArray(
			Source.SocketLocalTransform.vRotationDegrees);
		Out.vScale = To_SourceRuntimeArray(
			Source.SocketLocalTransform.vScale);
	}

	void Copy_SourceRuntimeTrail(
		const EFFECT_TRAIL_DESC& Source,
		EFFECT_VISUAL_PROGRAM_TRAIL_GEOMETRY& Out)
	{
		Out.iMaxPoints = Source.iMaxPoints;
		Out.fPointLifeTimeSeconds = Source.fPointLifeTimeSeconds;
		Out.fSampleIntervalSeconds = Source.fSampleIntervalSeconds;
		Out.fMinimumDistance = Source.fMinimumDistance;
		Out.fStartWidth = Source.fStartWidth;
		Out.fEndWidth = Source.fEndWidth;
		Out.bFaceCamera = Source.bFaceCamera;
	}

	const EFFECT_SOURCE_MODULE_DESC* Find_SourceRuntimeModule(
		const EFFECT_ELEMENT_DESC& Element,
		const std::string_view strStableId)
	{
		const EFFECT_SOURCE_MODULE_DESC* Found = nullptr;
		for (const EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (Module.strStableId != strStableId)
				continue;
			if (nullptr != Found)
				return nullptr;
			Found = &Module;
		}
		return Found;
	}

	bool_t Try_ReadSourceRuntimeLiteral(
		const EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view strPropertyPath,
		double& fOutValue,
		bool_t& bOutFound)
	{
		bOutFound = false;
		for (const EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
		{
			if (Literal.strPropertyPath != strPropertyPath)
				continue;
			if (bOutFound ||
				Literal.eKind != EFFECT_SOURCE_LITERAL_KIND::NUMBER ||
				!std::isfinite(Literal.fNumber))
			{
				return false;
			}
			bOutFound = true;
			fOutValue = Literal.fNumber;
		}
		return true;
	}

	bool_t Build_SourceOwnedRuntimeHistory(
		const EFFECT_DOCUMENT_DESC& Document,
		const EFFECT_AUTHORED_RUNTIME_EDGE_HISTORY_DESC& Source,
		const std::string_view strDocumentSha256,
		EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY& Out,
		std::string& strOutError)
	{
		if (!Is_StableId(Source.strHistoryId) ||
			Source.eCoordinateBasis !=
				EFFECT_AUTHORED_RUNTIME_COORDINATE_BASIS::
					UE3_CM_X_Z_NEG_Y_TO_RUNTIME_METERS ||
			!std::isfinite(Source.fSourceEndTimeSeconds) ||
			!std::isfinite(Source.fPlaybackClampSeconds) ||
			Source.fSourceEndTimeSeconds <= 0.f ||
			Source.fPlaybackClampSeconds <= 0.f ||
			Source.fPlaybackClampSeconds > Source.fSourceEndTimeSeconds ||
			Source.Samples.size() < 2u ||
			Source.Samples.size() > UINT32_MAX)
		{
			strOutError =
				"Source-owned baked-edge history metadata is invalid.";
			return false;
		}

		EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY Staged;
		Staged.strHistoryId = Source.strHistoryId;
		Staged.strSourceKind = "UE3_ANIMTRAIL_BAKED_EDGE_HISTORY_V1";
		Staged.strSourceArtifactPath = "Data/Effects/Authored/" +
			Document.strEffectAssetId + ".effect.json";
		Staged.strSourceArtifactRawSha256 = strDocumentSha256;
		Staged.strCoordinateBasis =
			"UE3_CM_X_Z_NEG_Y_TO_RUNTIME_METERS";
		Staged.fSourceEndTimeSeconds = Source.fSourceEndTimeSeconds;
		Staged.fPlaybackClampSeconds = Source.fPlaybackClampSeconds;
		Staged.iSampleCount = static_cast<uint32_t>(Source.Samples.size());
		Staged.Samples.reserve(Source.Samples.size());

		double fPreviousTime = -1.0;
		for (const EFFECT_AUTHORED_RUNTIME_EDGE_SAMPLE_DESC& SourceSample :
			Source.Samples)
		{
			if (!std::isfinite(SourceSample.fRelativeTimeSeconds) ||
				SourceSample.fRelativeTimeSeconds <= fPreviousTime ||
				!Is_FiniteSourceRuntimeVector(SourceSample.vFirstEdgeUE3Cm) ||
				!Is_FiniteSourceRuntimeVector(SourceSample.vControlPointUE3Cm) ||
				!Is_FiniteSourceRuntimeVector(SourceSample.vSecondEdgeUE3Cm))
			{
				strOutError =
					"Source-owned baked-edge history samples are invalid or unsorted.";
				return false;
			}
			fPreviousTime = SourceSample.fRelativeTimeSeconds;
			EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_SAMPLE Sample;
			Sample.fRelativeTimeSeconds = SourceSample.fRelativeTimeSeconds;
			Sample.vFirstEdgeUE3Cm = To_SourceRuntimeArray(
				SourceSample.vFirstEdgeUE3Cm);
			Sample.vControlPointUE3Cm = To_SourceRuntimeArray(
				SourceSample.vControlPointUE3Cm);
			Sample.vSecondEdgeUE3Cm = To_SourceRuntimeArray(
				SourceSample.vSecondEdgeUE3Cm);
			Staged.Samples.push_back(std::move(Sample));
		}
		if (std::abs(Staged.Samples.front().fRelativeTimeSeconds) > 1e-6 ||
			Staged.Samples.back().fRelativeTimeSeconds +
				SOURCE_OWNED_RUNTIME_EPSILON < Staged.fPlaybackClampSeconds ||
			Staged.Samples.back().fRelativeTimeSeconds >
				Staged.fSourceEndTimeSeconds + SOURCE_OWNED_RUNTIME_EPSILON)
		{
			strOutError =
				"Source-owned baked-edge history time closure is invalid.";
			return false;
		}
		Staged.strSamplesSha256 = Compute_SourceOwnedRuntimeSha(
			strDocumentSha256, "baked-edge-samples", Source.strHistoryId);
		Staged.strHistorySha256 = Compute_SourceOwnedRuntimeSha(
			strDocumentSha256, "baked-edge-history", Source.strHistoryId,
			Staged.strSamplesSha256);
		Out = std::move(Staged);
		return true;
	}

	bool_t Validate_SourceRuntimeTrailTarget(
		const EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError)
	{
		const EFFECT_TRAIL_DESC& Trail = Element.Detail.Trail;
		const EFFECT_TIMING_DESC& Timing = Element.Detail.Timing;
		if (!Element.bVisible || Element.eKind != EFFECT_ELEMENT_KIND::TRAIL ||
			Trail.iMaxPoints < 2u ||
			!std::isfinite(Trail.fPointLifeTimeSeconds) ||
			!std::isfinite(Trail.fSampleIntervalSeconds) ||
			!std::isfinite(Trail.fMinimumDistance) ||
			!std::isfinite(Trail.fStartWidth) ||
			!std::isfinite(Trail.fEndWidth) ||
			Trail.fPointLifeTimeSeconds <= 0.f ||
			Trail.fSampleIntervalSeconds <= 0.f ||
			Trail.fMinimumDistance < 0.f || Trail.fStartWidth < 0.f ||
			Trail.fEndWidth < 0.f ||
			!std::isfinite(Timing.fStartDelaySeconds) ||
			!std::isfinite(Timing.fLifeTimeSeconds) ||
			Timing.fStartDelaySeconds < 0.f || Timing.fLifeTimeSeconds <= 0.f)
		{
			strOutError =
				"Source-owned runtime carrier requires one visible bounded Trail target.";
			return false;
		}
		return true;
	}

	void Initialize_SourceOwnedSupplementalIdentity(
		const EFFECT_DOCUMENT_DESC& Document,
		const EFFECT_ELEMENT_DESC& Element,
		const std::string_view strDocumentSha256,
		EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Out)
	{
		Out.Selector.strEffectAssetId = Document.strEffectAssetId;
		Out.Selector.strOccurrenceId = Element.strElementId;
		Out.Selector.strSelectorSha256 = Compute_SourceOwnedRuntimeSha(
			strDocumentSha256, "selector", Element.strElementId);
		Out.eDisposition = EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED;
		Out.strFidelity = "BOUNDED_RECONSTRUCTION";
		Out.bTuningEligibleTransform = true;
		Out.strSourceRecordId = Element.strElementId;
		Out.strSourceRecordSha256 = Compute_SourceOwnedRuntimeSha(
			strDocumentSha256, "source-record", Element.strElementId);
		Out.strSourcePayloadRawSha256 = Compute_SourceOwnedRuntimeSha(
			strDocumentSha256, "source-payload", Element.strElementId);
		Out.TargetIdentity.strTargetElementId = Element.strElementId;
		Out.TargetIdentity.strTargetRecordSha256 =
			Out.strSourceRecordSha256;
		Out.TargetIdentity.strTargetPayloadRawSha256 =
			Out.strSourcePayloadRawSha256;
		Out.strStageId = Element.SourcePresentation.bEnabled &&
			!Element.SourcePresentation.strSourceActionCueId.empty() ?
			Element.SourcePresentation.strSourceActionCueId :
			Document.strEffectAssetId;
		Out.strSourceEventId = Element.SourcePresentation.bEnabled &&
			!Element.SourcePresentation.strSourceEventId.empty() ?
			Element.SourcePresentation.strSourceEventId : Element.strElementId;
		Out.fSourceTimelineSeconds = Element.SourcePresentation.bEnabled ?
			Element.SourcePresentation.fSourceTimeSeconds :
			Element.Detail.Timing.fStartDelaySeconds;
		Out.fLocalTimeSeconds = Element.Detail.Timing.fStartDelaySeconds;
		Out.fDurationSeconds = Element.Detail.Timing.fLifeTimeSeconds;
	}

	bool_t Build_SourceOwnedCascadeRibbon(
		const EFFECT_DOCUMENT_DESC& Document,
		const EFFECT_ELEMENT_DESC& Element,
		const std::string_view strDocumentSha256,
		EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Out,
		std::string& strOutError)
	{
		const EFFECT_AUTHORED_RUNTIME_CARRIER_DESC& Carrier =
			Element.RuntimeCarrier;
		if (!Validate_SourceRuntimeTrailTarget(Element, strOutError) ||
			!Carrier.strHistoryId.empty() ||
			Carrier.eEdgeLane != EFFECT_AUTHORED_RUNTIME_BAKED_EDGE_LANE::END ||
			!Element.SourceRecipe.bEnabled ||
			Element.SourceRecipe.strRendererShape != "ribbon" ||
			Carrier.strTypeDataModuleStableId.empty() ||
			Carrier.strTypeDataModuleStableId.size() > 512u)
		{
			if (strOutError.empty())
				strOutError =
					"Source-owned CascadeRibbon carrier target closure is invalid.";
			return false;
		}
		const EFFECT_SOURCE_MODULE_DESC* const pTypeData =
			Find_SourceRuntimeModule(
				Element, Carrier.strTypeDataModuleStableId);
		if (nullptr == pTypeData ||
			pTypeData->strClassName != "particlemoduletypedataribbon" ||
			pTypeData->strObjectPath.empty())
		{
			strOutError =
				"Source-owned CascadeRibbon type-data module join is stale.";
			return false;
		}

		EFFECT_VISUAL_PROGRAM_CASCADE_RIBBON_PACKET Packet;
		Packet.iPacketVersion = 1u;
		Packet.strAdapterId = "cascade-ribbon-document-v12";
		Packet.bBoundedSemanticReplay = true;
		Packet.bNativeExecution = false;
		Packet.strRuntimeCarrier = "EFFECT_TYPED_CASCADE_RIBBON_V1";
		Packet.strTypeDataStableId = pTypeData->strStableId;
		Packet.strTypeDataClassName = pTypeData->strClassName;
		Packet.strTypeDataObjectPath = pTypeData->strObjectPath;
		Packet.strTypeDataModuleSha256 = Compute_SourceOwnedRuntimeSha(
			strDocumentSha256, "cascade-type-data", Element.strElementId,
			pTypeData->strStableId);
		Packet.strResolvedRendererShape = "ribbon";
		Packet.fTilingDistance =
			Element.Detail.Trail.fTilingDistanceWorldUnits;
		Packet.fDistanceTessellationStepSize =
			Element.Detail.Trail.fDistanceTessellationStepWorldUnits;
		double fLiteral = 0.0;
		bool_t bFound = false;
		if (!Try_ReadSourceRuntimeLiteral(*pTypeData,
				"tangenttessellationscalar", fLiteral, bFound))
		{
			strOutError =
				"Source-owned CascadeRibbon tangent literal is ambiguous.";
			return false;
		}
		Packet.fTangentTessellationScalar = bFound ? fLiteral : 0.0;
		if (!Try_ReadSourceRuntimeLiteral(
				*pTypeData, "lodvalidity", fLiteral, bFound))
		{
			strOutError =
				"Source-owned CascadeRibbon LOD literal is ambiguous.";
			return false;
		}
		Packet.fLodValidity = bFound ? fLiteral : 0.0;
		Packet.iOperationalMaxPoints = Element.Detail.Trail.iMaxPoints;
		Copy_SourceRuntimeTiming(Element.Detail.Timing, Packet.Timing);
		Copy_SourceRuntimeAttachment(
			Element.ActionCueAttachment, Packet.Attachment);
		Copy_SourceRuntimeTrail(Element.Detail.Trail, Packet.Trail);
		Packet.strSourceRecipeSha256 = Compute_SourceOwnedRuntimeSha(
			strDocumentSha256, "cascade-source-recipe", Element.strElementId);
		Packet.strModuleClosureSha256 = Compute_SourceOwnedRuntimeSha(
			strDocumentSha256, "cascade-module-closure", Element.strElementId);
		if (Element.SourceRecipe.Modules.size() > UINT32_MAX)
		{
			strOutError =
				"Source-owned CascadeRibbon module closure exceeds uint32.";
			return false;
		}
		Packet.iModuleCount = static_cast<uint32_t>(
			Element.SourceRecipe.Modules.size());
		Packet.PreservedLimitations = {
			"SOURCE_OWNED_BOUNDED_RUNTIME_EXTENSION_V1" };
		Packet.strPacketSha256 = Compute_SourceOwnedRuntimeSha(
			strDocumentSha256, "cascade-packet", Element.strElementId,
			Packet.strTypeDataModuleSha256);

		Initialize_SourceOwnedSupplementalIdentity(
			Document, Element, strDocumentSha256, Out);
		Out.eFamily = EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON;
		Out.strAdapterId = Packet.strAdapterId;
		Out.strPacketLayout = "CASCADE_RIBBON_TYPED_PACKET_V1";
		Out.CascadeRibbonPacket = std::move(Packet);
		Out.strRowSha256 = Compute_SourceOwnedRuntimeSha(
			strDocumentSha256, "cascade-row", Element.strElementId,
			Out.CascadeRibbonPacket->strPacketSha256);
		return true;
	}

	bool_t Build_SourceOwnedAnimationTrail(
		const EFFECT_DOCUMENT_DESC& Document,
		const EFFECT_ELEMENT_DESC& Element,
		const EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY& History,
		const std::string_view strDocumentSha256,
		EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Out,
		std::string& strOutError)
	{
		if (!Validate_SourceRuntimeTrailTarget(Element, strOutError) ||
			Element.SourceRecipe.bEnabled ||
			!Element.SourcePresentation.bEnabled ||
			Element.SourcePresentation.strSourceObjectPath.empty() ||
			Element.SourcePresentation.strSourceActionCueId.empty() ||
			Element.SourcePresentation.strSourceEventId.empty() ||
			std::abs(static_cast<double>(Element.Detail.Timing.fLifeTimeSeconds) -
				History.fPlaybackClampSeconds) > SOURCE_OWNED_RUNTIME_EPSILON)
		{
			if (strOutError.empty())
				strOutError =
					"Source-owned AnimationTrail carrier target/history closure is invalid.";
			return false;
		}

		EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_PACKET Packet;
		Packet.iPacketVersion = 2u;
		Packet.strAdapterId = "animation-trail-document-v12";
		Packet.bBoundedSemanticReplay = true;
		Packet.bNativeExecution = false;
		Packet.strRuntimeCarrier =
			"EFFECT_TYPED_ANIMATION_TRAIL_BAKED_EDGE_V1";
		Packet.strSourceNotifyType = "Trails";
		Packet.strSourceEventId =
			Element.SourcePresentation.strSourceEventId;
		Packet.strSourceEventRecordSha256 = Compute_SourceOwnedRuntimeSha(
			strDocumentSha256, "animation-trail-source-event",
			Element.strElementId, Packet.strSourceEventId);
		Packet.strSourceAsset =
			Element.SourcePresentation.strSourceObjectPath;
		/* The source-owned schema has no separate clip string.  Keep the existing
		   packet ABI populated with the stable action-cue identity; playback uses
		   the typed baked history and never interprets this compatibility field. */
		Packet.strClip = Element.SourcePresentation.strSourceActionCueId;
		Packet.fLocalTimeSeconds = Element.Detail.Timing.fStartDelaySeconds;
		Packet.fGlobalTimeSeconds =
			Element.SourcePresentation.fSourceTimeSeconds;
		Packet.fDurationSeconds = Element.Detail.Timing.fLifeTimeSeconds;
		Packet.strTargetElementId = Element.strElementId;
		Copy_SourceRuntimeTiming(Element.Detail.Timing, Packet.TargetTiming);
		Copy_SourceRuntimeAttachment(
			Element.ActionCueAttachment, Packet.Attachment);
		Copy_SourceRuntimeTrail(Element.Detail.Trail, Packet.Trail);
		Packet.strHistoryId = History.strHistoryId;
		Packet.strHistorySha256 = History.strHistorySha256;
		Packet.fPlaybackClampSeconds = History.fPlaybackClampSeconds;
		Packet.strCoordinateBasis = History.strCoordinateBasis;
		Packet.PreservedLimitations = {
			"SOURCE_OWNED_BAKED_EDGE_GEOMETRY_V1" };
		Packet.strPacketSha256 = Compute_SourceOwnedRuntimeSha(
			strDocumentSha256, "animation-trail-packet", Element.strElementId,
			History.strHistorySha256);

		Initialize_SourceOwnedSupplementalIdentity(
			Document, Element, strDocumentSha256, Out);
		Out.eFamily = EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL;
		Out.strAdapterId = Packet.strAdapterId;
		Out.strPacketLayout =
			"ANIMATION_TRAIL_BAKED_EDGE_HISTORY_V1";
		Out.strSourceEventId = Packet.strSourceEventId;
		Out.fSourceTimelineSeconds = Packet.fGlobalTimeSeconds;
		Out.fLocalTimeSeconds = Packet.fLocalTimeSeconds;
		Out.fDurationSeconds = Packet.fDurationSeconds;
		Out.AnimationTrailPacket = std::move(Packet);
		Out.strRowSha256 = Compute_SourceOwnedRuntimeSha(
			strDocumentSha256, "animation-trail-row", Element.strElementId,
			Out.AnimationTrailPacket->strPacketSha256);
		return true;
	}

	bool_t Build_SourceOwnedBakedEdgeLight(
		const EFFECT_DOCUMENT_DESC& Document,
		const EFFECT_ELEMENT_DESC& Element,
		const EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY& History,
		const std::string_view strDocumentSha256,
		EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Out,
		std::string& strOutError)
	{
		const EFFECT_LIGHT_DETAIL_DESC& Light = Element.Detail.Light;
		const EFFECT_TIMING_DESC& Timing = Element.Detail.Timing;
		const double fActiveEnd = static_cast<double>(
			Timing.fStartDelaySeconds) + Timing.fLifeTimeSeconds;
		if (!Element.bVisible || Element.eKind != EFFECT_ELEMENT_KIND::LIGHT ||
			Element.SourceRecipe.bEnabled ||
			!Element.SourcePresentation.bEnabled ||
			Element.SourcePresentation.strSourceObjectPath.empty() ||
			Element.SourcePresentation.strSourceActionCueId.empty() ||
			Element.SourcePresentation.strSourceEventId.empty() ||
			!std::isfinite(Timing.fStartDelaySeconds) ||
			!std::isfinite(Timing.fLifeTimeSeconds) ||
			Timing.fStartDelaySeconds < 0.f || Timing.fLifeTimeSeconds <= 0.f ||
			fActiveEnd > History.fPlaybackClampSeconds +
				SOURCE_OWNED_RUNTIME_EPSILON ||
			!Light.bEnabled ||
			Light.eProfile != EFFECT_LIGHT_PROFILE::POINT_RECONSTRUCTED_V1 ||
			Light.eStatus !=
				EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE ||
			!std::isfinite(Light.fRange) ||
			!std::isfinite(Light.fIntensity) ||
			!std::isfinite(Light.fFalloffExponent) || Light.fRange <= 0.f ||
			Light.fIntensity < 0.f || Light.fFalloffExponent <= 0.f)
		{
			strOutError =
				"Source-owned baked-edge Light target/history closure is invalid.";
			return false;
		}

		EFFECT_VISUAL_PROGRAM_BAKED_EDGE_LIGHT_PACKET Packet;
		Packet.iPacketVersion = 1u;
		Packet.strAdapterId = "light-particle-document-v12";
		Packet.bBoundedSemanticReplay = true;
		Packet.bNativeExecution = false;
		Packet.strRuntimeCarrier =
			"EFFECT_TYPED_LIGHT_BAKED_EDGE_ATTACHMENT_V1";
		Packet.strSourceEventId =
			Element.SourcePresentation.strSourceEventId;
		Packet.strSourceEventRecordSha256 = Compute_SourceOwnedRuntimeSha(
			strDocumentSha256, "baked-edge-light-source-event",
			Element.strElementId, Packet.strSourceEventId);
		Packet.strTargetElementId = Element.strElementId;
		Packet.strHistoryId = History.strHistoryId;
		Packet.strHistorySha256 = History.strHistorySha256;
		Packet.eLane = EFFECT_VISUAL_PROGRAM_BAKED_EDGE_LANE::FIRST_EDGE;
		Packet.fActiveStartSeconds = Timing.fStartDelaySeconds;
		Packet.fActiveDurationSeconds = Timing.fLifeTimeSeconds;
		Packet.fActiveEndSeconds = fActiveEnd;
		Packet.fHistoryPlaybackClampSeconds =
			History.fPlaybackClampSeconds;
		Packet.strCoordinateBasis = History.strCoordinateBasis;
		Packet.strAttachmentEvidenceStatus =
			"SOURCE_AUTHORED_RUNTIME_EXTENSION_V1";
		Packet.TargetLight.bEnabled = Light.bEnabled;
		Packet.TargetLight.strProfileId =
			"light.point.reconstructed.v1";
		Packet.TargetLight.strStatus = "reconstructed_profile";
		Packet.TargetLight.fRange = Light.fRange;
		Packet.TargetLight.fIntensity = Light.fIntensity;
		Packet.TargetLight.vColor = { Light.vColor.x, Light.vColor.y,
			Light.vColor.z, Light.vColor.w };
		Packet.TargetLight.vAmbient = { Light.vAmbient.x, Light.vAmbient.y,
			Light.vAmbient.z, Light.vAmbient.w };
		Packet.TargetLight.fFalloffExponent = Light.fFalloffExponent;
		Packet.PreservedLimitations = {
			"SOURCE_OWNED_FIRST_EDGE_ATTACHMENT_V1" };
		Packet.strPacketSha256 = Compute_SourceOwnedRuntimeSha(
			strDocumentSha256, "baked-edge-light-packet", Element.strElementId,
			History.strHistorySha256);

		Initialize_SourceOwnedSupplementalIdentity(
			Document, Element, strDocumentSha256, Out);
		Out.eFamily = EFFECT_VISUAL_PROGRAM_FAMILY::LIGHT_PARTICLE;
		Out.strAdapterId = Packet.strAdapterId;
		Out.strPacketLayout = "LIGHT_BAKED_EDGE_ATTACHMENT_V1";
		Out.strSourceEventId = Packet.strSourceEventId;
		Out.fSourceTimelineSeconds =
			Element.SourcePresentation.fSourceTimeSeconds;
		Out.fLocalTimeSeconds = Packet.fActiveStartSeconds;
		Out.fDurationSeconds = Packet.fActiveDurationSeconds;
		Out.BakedEdgeLightPacket = std::move(Packet);
		Out.strRowSha256 = Compute_SourceOwnedRuntimeSha(
			strDocumentSha256, "baked-edge-light-row", Element.strElementId,
			Out.BakedEdgeLightPacket->strPacketSha256);
		return true;
	}
}

const EFFECT_VISUAL_PROGRAM_ROW*
Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION::Find_RowByOccurrenceId(
	const std::string_view strOccurrenceId) const
{
	const auto Found = std::find_if(m_AdmittedRows.begin(), m_AdmittedRows.end(),
		[strOccurrenceId](const EFFECT_VISUAL_PROGRAM_ROW& Row)
		{
			return Row.Selector.strOccurrenceId == strOccurrenceId;
		});
	return Found == m_AdmittedRows.end() ? nullptr : &*Found;
}

const EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY*
Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION::Find_BakedEdgeHistory(
	const std::string_view strHistoryId) const
{
	const auto Found = std::find_if(
		m_BakedEdgeHistories.begin(), m_BakedEdgeHistories.end(),
		[strHistoryId](const auto& History)
		{
			return History.strHistoryId == strHistoryId;
		});
	return Found == m_BakedEdgeHistories.end() ? nullptr : &*Found;
}

const EFFECT_VISUAL_PROGRAM_ROW*
Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION::Find_RowByTargetElementId(
	const std::string_view strTargetElementId) const
{
	const EFFECT_VISUAL_PROGRAM_ROW* Found = nullptr;
	for (const EFFECT_VISUAL_PROGRAM_ROW& Row : m_AdmittedRows)
	{
		if (Row.TargetIdentity.has_value() &&
			Row.TargetIdentity->strTargetElementId == strTargetElementId)
		{
			if (nullptr != Found) return nullptr;
			Found = &Row;
		}
	}
	return Found;
}

const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT*
Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION::
Find_SupplementalElementByOccurrenceId(
	const std::string_view strOccurrenceId) const
{
	const auto Found = std::find_if(
		m_AdmittedSupplementalElements.begin(),
		m_AdmittedSupplementalElements.end(),
		[strOccurrenceId](const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Element)
		{
			return Element.Selector.strOccurrenceId == strOccurrenceId;
		});
	return Found == m_AdmittedSupplementalElements.end() ? nullptr : &*Found;
}

const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT*
Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION::
Find_SupplementalElementByTargetElementId(
	const std::string_view strTargetElementId) const
{
	const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* Found = nullptr;
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Element :
		m_AdmittedSupplementalElements)
	{
		if (Element.TargetIdentity.strTargetElementId == strTargetElementId)
		{
			if (nullptr != Found) return nullptr;
			Found = &Element;
		}
	}
	return Found;
}

bool_t Client::CEffectVisualProgramCorpusCodec::Validate(
	const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
	std::string& strOutError)
{
	strOutError.clear();
	if (1u != Corpus.iFormatVersion || Corpus.strRuntimeId.empty() ||
		!Is_LowerSha256(Corpus.strSourceCorpusArtifactSha256) ||
		!Is_LowerSha256(Corpus.strArtifactSha256) || Corpus.Programs.empty())
	{
		strOutError = "Visual-program corpus identity is invalid.";
		return false;
	}
	uint32_t iOverlayPrograms = 0u;
	uint32_t iAdapterPrograms = 0u;
	uint32_t iRows = 0u;
	uint32_t iOverlayRows = 0u;
	uint32_t iLocalRows = 0u;
	uint32_t iCascadeRibbonRows = 0u;
	uint32_t iSupplementalElements = 0u;
	uint32_t iArtistFCascadeRibbonElements = 0u;
	uint32_t iArtistTCascadeRibbonElements = 0u;
	uint32_t iAnimationTrailElements = 0u;
	uint32_t iBakedEdgeLightElements = 0u;
	uint32_t iFailClosedRows = 0u;
	std::string PreviousEffect;
	std::set<std::pair<std::string, std::string>> SeenSelectors;
	for (const EFFECT_VISUAL_PROGRAM& Program : Corpus.Programs)
	{
		if (!Is_StableId(Program.strEffectAssetId) ||
			!Is_LowerSha256(Program.strProgramSha256) ||
			(Program.VisualRows.empty() && Program.SupplementalElements.empty()) ||
			(!PreviousEffect.empty() && Program.strEffectAssetId <= PreviousEffect))
		{
			strOutError = "Visual-program identity/order is invalid.";
			return false;
		}
		PreviousEffect = Program.strEffectAssetId;
		if (Program.eProjectionKind == EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1)
		{
			++iOverlayPrograms;
			if (nullptr == Program.pProjectedDocument ||
				Program.pProjectedDocument->strEffectAssetId != Program.strEffectAssetId ||
				!Is_LowerSha256(Program.strBaseDocumentRawSha256) ||
				!Is_LowerSha256(Program.strBaseDocumentCanonicalSha256) ||
				!Is_LowerSha256(Program.strBaseDocumentTypedCodecSha256) ||
				!Is_LowerSha256(Program.strProjectedDocumentSha256) ||
				!Is_LowerSha256(Program.strProjectedDocumentTypedCodecSha256) ||
				0u == Program.iProjectedDocumentCanonicalByteCount)
			{
				strOutError = "SourceRecipe overlay program document identity is invalid.";
				return false;
			}
		}
		else if (Program.eProjectionKind == EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1)
		{
			++iAdapterPrograms;
			if (nullptr != Program.pProjectedDocument ||
				!Program.strBaseDocumentRawSha256.empty() ||
				!Program.strBaseDocumentCanonicalSha256.empty() ||
				!Program.strBaseDocumentTypedCodecSha256.empty() ||
				!Program.strProjectedDocumentSha256.empty() ||
				!Program.strProjectedDocumentTypedCodecSha256.empty() ||
				0u != Program.iProjectedDocumentCanonicalByteCount)
			{
				strOutError = "Adapter-packet program contains document projection state.";
				return false;
			}
		}
		else
		{
			strOutError = "Visual-program projection kind is invalid.";
			return false;
		}
		uint32_t iProgramAdmitted = 0u;
		std::set<std::string, std::less<>> SeenTargets;
		std::string PreviousOccurrence;
		for (const EFFECT_VISUAL_PROGRAM_ROW& Row : Program.VisualRows)
		{
			++iRows;
			if (Row.Selector.strEffectAssetId != Program.strEffectAssetId ||
				!Is_StableId(Row.Selector.strOccurrenceId) ||
				!Is_LowerSha256(Row.Selector.strSelectorSha256) ||
				!Is_LowerSha256(Row.strRowSha256) ||
				(!PreviousOccurrence.empty() &&
				 Row.Selector.strOccurrenceId <= PreviousOccurrence) ||
				!SeenSelectors.emplace(Row.Selector.strEffectAssetId,
					Row.Selector.strOccurrenceId).second)
			{
				strOutError = "Visual-program selector is duplicate, stale, or unsorted.";
				return false;
			}
			PreviousOccurrence = Row.Selector.strOccurrenceId;
			if (Row.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON)
				++iCascadeRibbonRows;
			if (Row.eDisposition == EFFECT_VISUAL_PROGRAM_DISPOSITION::FAIL_CLOSED)
			{
				++iFailClosedRows;
				if (Row.TargetIdentity.has_value() || Row.LocalDecalPacket.has_value() ||
					Row.strPacketLayout != "NONE")
				{
					strOutError = "Fail-closed row contains executable payload.";
					return false;
				}
				continue;
			}
			if (Row.eDisposition != EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
				!Row.TargetIdentity.has_value())
			{
				strOutError = "Visual-program row disposition/target is invalid.";
				return false;
			}
			++iProgramAdmitted;
			if (!SeenTargets.emplace(Row.TargetIdentity->strTargetElementId).second)
			{
				strOutError = "Visual-program admitted target is duplicate.";
				return false;
			}
			if (Program.eProjectionKind == EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1)
			{
				++iOverlayRows;
				if (Row.LocalDecalPacket.has_value() ||
					Row.strPacketLayout != "EFFECT_DOCUMENT_ELEMENT_V12")
				{
					strOutError = "SourceRecipe overlay row packet boundary is invalid.";
					return false;
				}
			}
			else
			{
				if (!Row.LocalDecalPacket.has_value())
				{
					strOutError = "Adapter row lacks an immutable typed packet.";
					return false;
				}
				++iLocalRows;
			}
		}
		std::set<std::string, std::less<>> SeenSupplementalOccurrences;
		std::set<std::string, std::less<>> ReferencedHistoryIds;
		for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental :
			Program.SupplementalElements)
		{
			++iSupplementalElements;
			if (Supplemental.Selector.strEffectAssetId != Program.strEffectAssetId ||
				Supplemental.eDisposition !=
					EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
				!Supplemental.AdmissionBlockers.empty() ||
				!SeenSupplementalOccurrences.emplace(
					Supplemental.Selector.strOccurrenceId).second)
			{
				strOutError = "Visual-program supplemental element is invalid.";
				return false;
			}
			if (Supplemental.eFamily ==
				EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON)
			{
				if (Program.strEffectAssetId == "effect.artist.skill.31470")
					++iArtistFCascadeRibbonElements;
				else if (Program.strEffectAssetId ==
					"effect.artist.skill.31950.unified")
				{
					++iArtistTCascadeRibbonElements;
				}
				else
				{
					strOutError =
						"Visual-program CascadeRibbon supplemental owner is invalid.";
					return false;
				}
			}
			else if (Supplemental.eFamily ==
				EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL)
				++iAnimationTrailElements;
			else if (Supplemental.eFamily ==
				EFFECT_VISUAL_PROGRAM_FAMILY::LIGHT_PARTICLE)
				++iBakedEdgeLightElements;
			else
			{
				strOutError = "Visual-program supplemental family is invalid.";
				return false;
			}
			++iProgramAdmitted;
			if (Supplemental.AnimationTrailPacket.has_value() &&
				2u == Supplemental.AnimationTrailPacket->iPacketVersion)
			{
				const auto& Packet = *Supplemental.AnimationTrailPacket;
				const auto History = std::find_if(
					Program.BakedEdgeHistories.begin(),
					Program.BakedEdgeHistories.end(),
					[&Packet](const auto& Candidate)
					{
						return Candidate.strHistoryId == Packet.strHistoryId;
					});
				if (History == Program.BakedEdgeHistories.end() ||
					History->strHistorySha256 != Packet.strHistorySha256)
				{
					strOutError =
						"Visual-program baked-edge history reference is invalid.";
					return false;
				}
				ReferencedHistoryIds.emplace(Packet.strHistoryId);
			}
			if (Supplemental.BakedEdgeLightPacket.has_value())
			{
				const auto& Packet = *Supplemental.BakedEdgeLightPacket;
				const auto History = std::find_if(
					Program.BakedEdgeHistories.begin(),
					Program.BakedEdgeHistories.end(),
					[&Packet](const auto& Candidate)
					{
						return Candidate.strHistoryId == Packet.strHistoryId;
					});
				if (History == Program.BakedEdgeHistories.end() ||
					History->strHistorySha256 != Packet.strHistorySha256 ||
					History->strCoordinateBasis != Packet.strCoordinateBasis ||
					std::abs(History->fPlaybackClampSeconds -
						Packet.fHistoryPlaybackClampSeconds) > 5e-5)
				{
					strOutError =
						"Visual-program baked-edge Light history reference is invalid.";
					return false;
				}
				ReferencedHistoryIds.emplace(Packet.strHistoryId);
			}
		}
		if (ReferencedHistoryIds.size() != Program.BakedEdgeHistories.size())
		{
			strOutError =
				"Visual-program baked-edge history closure is incomplete.";
			return false;
		}
		if (0u == iProgramAdmitted)
		{
			strOutError = "Visual program has no admitted executable row.";
			return false;
		}
	}
	if (Corpus.Programs.size() != Corpus.iDeclaredProgramCount ||
		iOverlayPrograms != Corpus.iDeclaredSourceRecipeOverlayProgramCount ||
		iAdapterPrograms != Corpus.iDeclaredAdapterPacketProgramCount ||
		iRows != Corpus.iDeclaredVisualRowCount ||
		iOverlayRows != Corpus.iDeclaredSourceRecipeOverlayCount ||
		iLocalRows != Corpus.iDeclaredLocalDecalAdapterPacketCount ||
		iCascadeRibbonRows != Corpus.iDeclaredCascadeRibbonVisualRowCount ||
		iSupplementalElements != Corpus.iDeclaredSupplementalElementCount ||
		iArtistFCascadeRibbonElements !=
			Corpus.iDeclaredArtistFCascadeRibbonElementCount ||
		iArtistTCascadeRibbonElements !=
			Corpus.iDeclaredArtistTCascadeRibbonElementCount ||
		iAnimationTrailElements != Corpus.iDeclaredAnimationTrailElementCount ||
		iBakedEdgeLightElements != Corpus.iDeclaredBakedEdgeLightElementCount ||
		iFailClosedRows != Corpus.iDeclaredFailClosedCount)
	{
		strOutError = "Visual-program declared denominators do not match internal sums.";
		return false;
	}
	return true;
}

const EFFECT_VISUAL_PROGRAM* Client::CEffectVisualProgramCorpusCodec::Find_Program(
	const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
	const std::string_view strEffectAssetId)
{
	const auto Found = std::lower_bound(Corpus.Programs.begin(), Corpus.Programs.end(),
		strEffectAssetId, [](const EFFECT_VISUAL_PROGRAM& Program, const std::string_view Id)
		{
			return Program.strEffectAssetId < Id;
		});
	return Found != Corpus.Programs.end() && Found->strEffectAssetId == strEffectAssetId ?
		&*Found : nullptr;
}

const EFFECT_VISUAL_PROGRAM_ROW* Client::CEffectVisualProgramCorpusCodec::Find_Row(
	const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
	const EFFECT_VISUAL_PROGRAM_SELECTOR& Selector)
{
	const EFFECT_VISUAL_PROGRAM* Program = Find_Program(Corpus, Selector.strEffectAssetId);
	if (nullptr == Program) return nullptr;
	const auto Found = std::lower_bound(Program->VisualRows.begin(), Program->VisualRows.end(),
		Selector.strOccurrenceId,
		[](const EFFECT_VISUAL_PROGRAM_ROW& Row, const std::string_view Id)
		{
			return Row.Selector.strOccurrenceId < Id;
		});
	return Found != Program->VisualRows.end() &&
		Found->Selector.strOccurrenceId == Selector.strOccurrenceId ? &*Found : nullptr;
}

const EFFECT_VISUAL_PROGRAM_ROW*
Client::CEffectVisualProgramCorpusCodec::Find_RowByTargetElementId(
	const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
	const std::string_view strEffectAssetId,
	const std::string_view strTargetElementId)
{
	const EFFECT_VISUAL_PROGRAM* Program = Find_Program(Corpus, strEffectAssetId);
	if (nullptr == Program) return nullptr;
	const EFFECT_VISUAL_PROGRAM_ROW* Found = nullptr;
	for (const EFFECT_VISUAL_PROGRAM_ROW& Row : Program->VisualRows)
	{
		if (Row.TargetIdentity.has_value() &&
			Row.TargetIdentity->strTargetElementId == strTargetElementId)
		{
			if (nullptr != Found) return nullptr;
			Found = &Row;
		}
	}
	return Found;
}

const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT*
Client::CEffectVisualProgramCorpusCodec::Find_SupplementalElement(
	const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
	const EFFECT_VISUAL_PROGRAM_SELECTOR& Selector)
{
	const EFFECT_VISUAL_PROGRAM* Program = Find_Program(
		Corpus, Selector.strEffectAssetId);
	if (nullptr == Program) return nullptr;
	const auto Found = std::lower_bound(
		Program->SupplementalElements.begin(), Program->SupplementalElements.end(),
		Selector.strOccurrenceId,
		[](const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Element,
			const std::string_view Id)
		{
			return Element.Selector.strOccurrenceId < Id;
		});
	return Found != Program->SupplementalElements.end() &&
		Found->Selector.strOccurrenceId == Selector.strOccurrenceId ?
		&*Found : nullptr;
}

const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT*
Client::CEffectVisualProgramCorpusCodec::
Find_SupplementalElementByTargetElementId(
	const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
	const std::string_view strEffectAssetId,
	const std::string_view strTargetElementId)
{
	const EFFECT_VISUAL_PROGRAM* Program = Find_Program(Corpus, strEffectAssetId);
	if (nullptr == Program) return nullptr;
	const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* Found = nullptr;
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Element :
		Program->SupplementalElements)
	{
		if (Element.TargetIdentity.strTargetElementId == strTargetElementId)
		{
			if (nullptr != Found) return nullptr;
			Found = &Element;
		}
	}
	return Found;
}

std::string Client::CEffectVisualProgramCorpusCodec::Compute_DocumentCanonicalSha256(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	strOutError.clear();
	DATA_JSON_VALUE Value;
	if (!Parse_DocumentJson(Document, Value, strOutError)) return {};
	return Canonical_Sha(Normalize_TypedSemanticZero(Value));
}

bool_t Client::CEffectVisualProgramCorpusCodec::
Create_DocumentOwnedRuntimeProjection(
	const std::shared_ptr<const EFFECT_DOCUMENT_DESC>& pDocument,
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>&
		InOutProjection,
	std::string& strOutError)
{
	strOutError.clear();
	if (nullptr == pDocument ||
		pDocument->iLoadedFormatVersion !=
			EFFECT_AUTHORED_RUNTIME_EXTENSION_FORMAT_VERSION ||
		pDocument->bSourceContract ||
		pDocument->RuntimeExtensions.iFormatVersion !=
			EFFECT_AUTHORED_RUNTIME_EXTENSION_PAYLOAD_VERSION ||
		!Is_StableId(pDocument->strEffectAssetId) ||
		!CEffectDocumentCodec::Validate_Drawable(*pDocument, strOutError))
	{
		if (strOutError.empty())
		{
			strOutError =
				"Document-owned runtime projection requires one valid authored v15 document.";
		}
		return false;
	}

	const std::string DocumentSha =
		Compute_DocumentCanonicalSha256(*pDocument, strOutError);
	if (DocumentSha.empty())
		return false;

	std::vector<EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY>
		StagedHistories;
	StagedHistories.reserve(
		pDocument->RuntimeExtensions.BakedEdgeHistories.size());
	std::set<std::string, std::less<>> SeenHistoryIds;
	for (const EFFECT_AUTHORED_RUNTIME_EDGE_HISTORY_DESC& SourceHistory :
		pDocument->RuntimeExtensions.BakedEdgeHistories)
	{
		EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY History;
		if (!SeenHistoryIds.emplace(SourceHistory.strHistoryId).second ||
			!Build_SourceOwnedRuntimeHistory(*pDocument, SourceHistory,
				DocumentSha, History, strOutError))
		{
			if (strOutError.empty())
			{
				strOutError =
					"Document-owned runtime projection contains a duplicate baked-edge history.";
			}
			return false;
		}
		StagedHistories.push_back(std::move(History));
	}
	std::sort(StagedHistories.begin(), StagedHistories.end(),
		[](const auto& Left, const auto& Right)
		{
			return Left.strHistoryId < Right.strHistoryId;
		});

	const auto FindHistory = [&StagedHistories](
		const std::string_view strHistoryId)
		-> const EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY*
	{
		const auto Found = std::lower_bound(
			StagedHistories.begin(), StagedHistories.end(), strHistoryId,
			[](const auto& Candidate, const std::string_view Identity)
			{
				return Candidate.strHistoryId < Identity;
			});
		return Found != StagedHistories.end() &&
			Found->strHistoryId == strHistoryId ? &*Found : nullptr;
	};

	std::vector<EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT>
		StagedSupplementals;
	std::set<std::string, std::less<>> SeenTargetElementIds;
	std::set<std::string, std::less<>> ReferencedHistoryIds;
	for (const EFFECT_ELEMENT_DESC& Element : pDocument->Elements)
	{
		const EFFECT_AUTHORED_RUNTIME_CARRIER_DESC& Carrier =
			Element.RuntimeCarrier;
		if (Carrier.Is_Empty())
			continue;
		if (Carrier.iFormatVersion !=
				EFFECT_AUTHORED_RUNTIME_EXTENSION_PAYLOAD_VERSION ||
			Carrier.eAdmission !=
				EFFECT_AUTHORED_RUNTIME_CARRIER_ADMISSION::BOUNDED ||
			!Is_StableId(Element.strElementId) ||
			!SeenTargetElementIds.emplace(Element.strElementId).second)
		{
			strOutError =
				"Document-owned runtime carrier identity/admission is invalid or duplicate.";
			return false;
		}

		EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT Supplemental;
		switch (Carrier.eKind)
		{
		case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_RIBBON_V1:
			if (!Build_SourceOwnedCascadeRibbon(*pDocument, Element,
					DocumentSha, Supplemental, strOutError))
			{
				return false;
			}
			break;

		case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::
			ANIMATION_TRAIL_BAKED_EDGE_V1:
		{
			if (Carrier.strTypeDataModuleStableId.empty() == false ||
				Carrier.eEdgeLane !=
					EFFECT_AUTHORED_RUNTIME_BAKED_EDGE_LANE::END ||
				!Is_StableId(Carrier.strHistoryId))
			{
				strOutError =
					"Document-owned AnimationTrail carrier shape is invalid.";
				return false;
			}
			const auto* const pHistory = FindHistory(Carrier.strHistoryId);
			if (nullptr == pHistory ||
				!Build_SourceOwnedAnimationTrail(*pDocument, Element,
					*pHistory, DocumentSha, Supplemental, strOutError))
			{
				if (strOutError.empty())
					strOutError =
						"Document-owned AnimationTrail history join is missing.";
				return false;
			}
			ReferencedHistoryIds.emplace(Carrier.strHistoryId);
			break;
		}

		case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::
			LIGHT_BAKED_EDGE_ATTACHMENT_V1:
		{
			if (!Carrier.strTypeDataModuleStableId.empty() ||
				Carrier.eEdgeLane !=
					EFFECT_AUTHORED_RUNTIME_BAKED_EDGE_LANE::FIRST_EDGE ||
				!Is_StableId(Carrier.strHistoryId))
			{
				strOutError =
					"Document-owned baked-edge Light carrier shape is invalid.";
				return false;
			}
			const auto* const pHistory = FindHistory(Carrier.strHistoryId);
			if (nullptr == pHistory ||
				!Build_SourceOwnedBakedEdgeLight(*pDocument, Element,
					*pHistory, DocumentSha, Supplemental, strOutError))
			{
				if (strOutError.empty())
					strOutError =
						"Document-owned baked-edge Light history join is missing.";
				return false;
			}
			ReferencedHistoryIds.emplace(Carrier.strHistoryId);
			break;
		}

		case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::END:
		default:
			strOutError =
				"Document-owned runtime carrier kind is unsupported.";
			return false;
		}
		StagedSupplementals.push_back(std::move(Supplemental));
	}

	if (StagedSupplementals.empty())
	{
		strOutError =
			"Authored v15 document contains no admitted runtime carrier.";
		return false;
	}
	if (ReferencedHistoryIds.size() != StagedHistories.size())
	{
		strOutError =
			"Document-owned runtime extension contains an unreferenced baked-edge history.";
		return false;
	}
	std::sort(StagedSupplementals.begin(), StagedSupplementals.end(),
		[](const auto& Left, const auto& Right)
		{
			return Left.Selector.strOccurrenceId <
				Right.Selector.strOccurrenceId;
		});

	std::string ProgramMaterial =
		"effect-source-owned-runtime-program-v1\n" + DocumentSha + "\n";
	std::vector<EFFECT_VISUAL_PROGRAM_SELECTOR> StagedSelectors;
	StagedSelectors.reserve(StagedSupplementals.size());
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental :
		StagedSupplementals)
	{
		ProgramMaterial += Supplemental.Selector.strOccurrenceId + "\n" +
			Supplemental.strRowSha256 + "\n";
		StagedSelectors.push_back(Supplemental.Selector);
	}
	for (const EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY& History :
		StagedHistories)
	{
		ProgramMaterial += History.strHistoryId + "\n" +
			History.strHistorySha256 + "\n";
	}
	const std::string ProgramSha =
		CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(ProgramMaterial);
	const std::string AdmissionToken =
		CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
			"effect-source-owned-runtime-admission-token-v1\n" +
			pDocument->strEffectAssetId + "\n" + ProgramSha + "\n" +
			DocumentSha + "\n");

	auto Projection =
		std::make_shared<EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>();
	/* This pointer identity is the core source-owned invariant.  Packets are a
	   transient execution view; they never replace or mutate the authored
	   document staged by the catalog/tool transaction. */
	Projection->m_pDocument = pDocument;
	Projection->m_eProjectionKind =
		EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
	Projection->m_strEffectAssetId = pDocument->strEffectAssetId;
	Projection->m_strProgramSha256 = ProgramSha;
	Projection->m_strBaseDocumentCanonicalSha256 = DocumentSha;
	Projection->m_strProjectedDocumentSha256 = DocumentSha;
	Projection->m_strAdmissionTokenSha256 = AdmissionToken;
	Projection->m_AdmittedSelectors = std::move(StagedSelectors);
	Projection->m_AdmittedRows.clear();
	Projection->m_AdmittedSupplementalElements =
		std::move(StagedSupplementals);
	Projection->m_BakedEdgeHistories = std::move(StagedHistories);
	if (!Projection->Is_Valid() ||
		Projection->Get_DocumentShared().get() != pDocument.get() ||
		Projection->Get_BaseDocumentCanonicalSha256() !=
			Projection->Get_ProjectedDocumentSha256())
	{
		strOutError =
			"Document-owned runtime projection construction lost source identity.";
		return false;
	}

	InOutProjection = std::move(Projection);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectVisualProgramCorpusCodec::Create_DocumentProjection(
	const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
	const EFFECT_DOCUMENT_DESC& BaseDocument,
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>& InOutProjection,
	std::string& strOutError)
{
	strOutError.clear();
	if (!Validate(Corpus, strOutError)) return false;
	const EFFECT_VISUAL_PROGRAM* Program =
		Find_Program(Corpus, BaseDocument.strEffectAssetId);
	if (nullptr == Program)
	{
		strOutError = "No visual program exists for the supplied Effect document.";
		return false;
	}
	const bool_t bAdapterPacketProjection =
		Program->eProjectionKind ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
	if (bAdapterPacketProjection)
	{
		if (!CEffectDocumentCodec::
			Validate_ReconstructedRuntimeDrawable(
				BaseDocument, strOutError))
		{
			return false;
		}
	}
	else if (!CEffectDocumentCodec::Validate_Drawable(
		BaseDocument, strOutError))
	{
		return false;
	}
	const std::string BaseSha = Compute_DocumentCanonicalSha256(BaseDocument, strOutError);
	if (BaseSha.empty()) return false;
	if (Program->eProjectionKind ==
		EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1 &&
		BaseSha != Program->strBaseDocumentTypedCodecSha256)
	{
		strOutError = "Visual-program base document typed-codec SHA is stale: expected=" +
			Program->strBaseDocumentTypedCodecSha256 + " actual=" + BaseSha + ".";
		return false;
	}
	/* baseDocumentCanonicalSha256 is the legacy projected raw-JSON identity.
	   A typed EFFECT_DOCUMENT_DESC uses
	   f32 fields and cannot reproduce arbitrary source JSON f64 lexemes.  The
	   admission-time identity below is therefore an exact typed-codec compare,
	   not a false equality between those two domains. */
	if (Program->eProjectionKind ==
		EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1)
	{
		if (nullptr == Program->pProjectedDocument)
		{
			strOutError = "Visual-program projected document is missing.";
			return false;
		}
		EFFECT_DOCUMENT_DESC ExpectedTypedBase = *Program->pProjectedDocument;
		for (const EFFECT_VISUAL_PROGRAM_ROW& Row : Program->VisualRows)
		{
			if (Row.eDisposition != EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED)
				continue;
			if (!Row.TargetIdentity.has_value())
			{
				strOutError = "Visual-program admitted overlay target is missing.";
				return false;
			}
			if (Row.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON)
			{
				std::erase_if(ExpectedTypedBase.Elements,
					[&Row](const EFFECT_ELEMENT_DESC& Element)
					{
						return Element.strElementId ==
							Row.TargetIdentity->strTargetElementId;
					});
				continue;
			}
			EFFECT_ELEMENT_DESC* Target = Find_Element(
				ExpectedTypedBase, Row.TargetIdentity->strTargetElementId);
			if (nullptr == Target)
			{
				strOutError = "Visual-program typed base target is missing.";
				return false;
			}
			Target->SourceRecipe = {};
			/* Legacy v12/v13 carrier rows serialize an explicit loopCount=0. */
			Target->SourceRecipe.iEmitterLoopCount = 0u;
		}
		for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental :
			Program->SupplementalElements)
		{
			if (Supplemental.eFamily !=
					EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL &&
				Supplemental.eFamily !=
					EFFECT_VISUAL_PROGRAM_FAMILY::LIGHT_PARTICLE)
				continue;
			const EFFECT_ELEMENT_DESC* BaseTarget = Find_Element(
				BaseDocument, Supplemental.TargetIdentity.strTargetElementId);
			if (nullptr == BaseTarget)
			{
				std::erase_if(ExpectedTypedBase.Elements,
					[&Supplemental](const EFFECT_ELEMENT_DESC& Element)
					{
						return Element.strElementId ==
							Supplemental.TargetIdentity.strTargetElementId;
					});
			}
			else
			{
				EFFECT_ELEMENT_DESC* ProjectedTarget = Find_Element(
					ExpectedTypedBase,
					Supplemental.TargetIdentity.strTargetElementId);
				if (nullptr == ProjectedTarget)
				{
					strOutError =
						"Visual-program projected supplemental target is missing.";
					return false;
				}
				*ProjectedTarget = *BaseTarget;
			}
		}
		/* BaseSha above already seals the authored element order.  A projected
		   document intentionally owns its own deterministic element order, so
		   reverse-projection equality compares the same stable element identities
		   after ID normalization instead of rejecting an otherwise exact payload
		   solely because the projection sorted those rows. */
		EFFECT_DOCUMENT_DESC ActualTypedBase = BaseDocument;
		const auto SortElementsByStableId = [](EFFECT_DOCUMENT_DESC& Document)
		{
			std::sort(Document.Elements.begin(), Document.Elements.end(),
				[](const EFFECT_ELEMENT_DESC& Left,
					const EFFECT_ELEMENT_DESC& Right)
				{
					return Left.strElementId < Right.strElementId;
				});
		};
		SortElementsByStableId(ExpectedTypedBase);
		SortElementsByStableId(ActualTypedBase);
		std::string ExpectedTypedBaseError;
		std::string ActualTypedBaseError;
		const std::string ExpectedTypedBaseSha =
			CEffectVisualProgramCorpusCodec::Compute_DocumentCanonicalSha256(
				ExpectedTypedBase, ExpectedTypedBaseError);
		const std::string ActualTypedBaseSha =
			CEffectVisualProgramCorpusCodec::Compute_DocumentCanonicalSha256(
				ActualTypedBase, ActualTypedBaseError);
		if (ExpectedTypedBaseSha.empty() || ActualTypedBaseSha.empty() ||
			ExpectedTypedBaseSha != ActualTypedBaseSha)
		{
			strOutError =
				"Visual-program base document typed codec identity is stale.";
			return false;
		}
	}

	/* The projected document is the immutable ordering authority.  Rebuilding it
	   by appending admitted targets in row/supplemental traversal order changes
	   the typed codec bytes whenever source elements were interleaved.  The
	   reverse projection above already proved that this exact document lowers
	   back to BaseDocument, so stage its complete ordered copy transactionally. */
	EFFECT_DOCUMENT_DESC StagedDocument =
		Program->eProjectionKind ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1 ?
			*Program->pProjectedDocument : BaseDocument;
	std::vector<EFFECT_VISUAL_PROGRAM_ROW> AdmittedRows;
	std::vector<EFFECT_VISUAL_PROGRAM_SELECTOR> AdmittedSelectors;
	std::vector<EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT>
		AdmittedSupplementalElements;
	for (const EFFECT_VISUAL_PROGRAM_ROW& Row : Program->VisualRows)
	{
		if (Row.eDisposition == EFFECT_VISUAL_PROGRAM_DISPOSITION::FAIL_CLOSED)
			continue;
		if (Row.eDisposition != EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
			!Row.TargetIdentity.has_value())
		{
			strOutError = "Visual-program admitted row is malformed.";
			return false;
		}
		if (Program->eProjectionKind ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1)
		{
			if (nullptr == Program->pProjectedDocument)
			{
				strOutError = "Visual-program projected document is missing.";
				return false;
			}
			EFFECT_ELEMENT_DESC* Target = Find_Element(
				StagedDocument, Row.TargetIdentity->strTargetElementId);
			const EFFECT_ELEMENT_DESC* Source = Find_Element(
				*Program->pProjectedDocument, Row.TargetIdentity->strTargetElementId);
			const EFFECT_ELEMENT_KIND ExpectedKind =
				Row.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::MESH_PARTICLE ?
					EFFECT_ELEMENT_KIND::MESH :
				Row.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::SPRITE_PARTICLE ?
					EFFECT_ELEMENT_KIND::SPRITE : EFFECT_ELEMENT_KIND::TRAIL;
			const std::string_view ExpectedShape =
				Row.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::MESH_PARTICLE ?
					"mesh" :
				Row.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::SPRITE_PARTICLE ?
					"sprite" : "ribbon";
			if (nullptr == Target || nullptr == Source ||
				(Row.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::MESH_PARTICLE &&
				 Row.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::SPRITE_PARTICLE &&
				 Row.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON) ||
				Target->eKind != ExpectedKind || Source->eKind != ExpectedKind ||
				!Source->SourceRecipe.bEnabled ||
				Source->SourceRecipe.strRendererShape != ExpectedShape ||
				Source->SourceRecipe.Modules.size() != Row.SourceIdentity.iModuleCount)
			{
				strOutError = "Visual-program SourceRecipe target/family/module count is stale.";
				return false;
			}

			/* The sealed row pins targetRecordSha256 and targetPayloadRawSha256,
			   while the full base-document canonical SHA pins every carrier,
			   material, resource, transform, and attachment field.  Only this
			   SourceRecipe assignment is allowed to differ in the staged copy. */
			Target->SourceRecipe = Source->SourceRecipe;
		}
		else if (!Row.LocalDecalPacket.has_value())
		{
			strOutError = "Visual-program adapter row lacks its immutable packet.";
			return false;
		}
		else
		{
			const EFFECT_ELEMENT_DESC* Target = Find_Element(
				StagedDocument, Row.TargetIdentity->strTargetElementId);
			if (Row.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::DECAL_PARTICLE ||
				nullptr == Target || Target->eKind != EFFECT_ELEMENT_KIND::DECAL ||
				Target->Renderer.eType != EFFECT_RENDERER_TYPE::DECAL_PARTICLE ||
				Target->Renderer.eSourceSpace !=
					EFFECT_SOURCE_SPACE::UE3_CASCADE_V1)
			{
				strOutError =
					"Visual-program adapter packet target/family/renderer is stale.";
				return false;
			}
		}
		AdmittedSelectors.push_back(Row.Selector);
		AdmittedRows.push_back(Row);
	}
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental :
		Program->SupplementalElements)
	{
		if (Supplemental.eDisposition !=
				EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
			!Supplemental.AdmissionBlockers.empty())
		{
			strOutError = "Visual-program supplemental admission is malformed.";
			return false;
		}
		EFFECT_ELEMENT_DESC* SupplementalTarget = Find_Element(
			StagedDocument, Supplemental.TargetIdentity.strTargetElementId);
		if (nullptr == SupplementalTarget)
		{
			strOutError = "Visual-program supplemental target is missing.";
			return false;
		}
		if (!Materialize_CascadeRibbonSupplemental(
			StagedDocument, Supplemental, strOutError))
			return false;
		if (bAdapterPacketProjection)
		{
			const EFFECT_RENDERER_TYPE eExpectedRenderer =
				Supplemental.CascadeRibbonPacket.has_value() ?
					EFFECT_RENDERER_TYPE::CASCADE_RIBBON :
					EFFECT_RENDERER_TYPE::ANIM_TRAIL;
			if (SupplementalTarget->eKind != EFFECT_ELEMENT_KIND::TRAIL ||
				SupplementalTarget->Renderer.eType != eExpectedRenderer ||
				SupplementalTarget->Renderer.eSourceSpace !=
					EFFECT_SOURCE_SPACE::UE3_CASCADE_V1)
			{
				strOutError =
					"Visual-program supplemental adapter packet renderer is stale.";
				return false;
			}
		}
		AdmittedSupplementalElements.push_back(Supplemental);
	}
	if (AdmittedRows.empty() && AdmittedSupplementalElements.empty())
	{
		strOutError = "Visual program admitted no executable rows.";
		return false;
	}
	if (bAdapterPacketProjection)
	{
		if (!CEffectDocumentCodec::
			Validate_ReconstructedRuntimeDrawable(
				StagedDocument, strOutError))
		{
			return false;
		}
	}
	else if (!CEffectDocumentCodec::Validate_Drawable(
		StagedDocument, strOutError))
	{
		return false;
	}
	const std::string ProjectedSha =
		Compute_DocumentCanonicalSha256(StagedDocument, strOutError);
	if (ProjectedSha.empty()) return false;
	if (Program->eProjectionKind ==
		EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1)
	{
		if (ProjectedSha != Program->strProjectedDocumentTypedCodecSha256 ||
			nullptr == Program->pProjectedDocument ||
			CEffectDocumentCodec::Serialize(StagedDocument) !=
			CEffectDocumentCodec::Serialize(*Program->pProjectedDocument))
		{
			strOutError = "Visual-program projected document codec validation failed.";
			return false;
		}
	}

	std::string TokenMaterial =
		"effect-visual-program-admission-token-v1\n" +
		Program->strEffectAssetId + "\n" + Program->strProgramSha256 + "\n" +
		BaseSha + "\n" + ProjectedSha + "\n";
	for (const EFFECT_VISUAL_PROGRAM_ROW& Row : AdmittedRows)
	{
		TokenMaterial += Row.Selector.strEffectAssetId + "\n" +
			Row.Selector.strOccurrenceId + "\n" +
			Row.Selector.strSelectorSha256 + "\n" + Row.strRowSha256 + "\n" +
			Row.TargetIdentity->strTargetElementId + "\n" +
			Row.TargetIdentity->strTargetRecordSha256 + "\n" +
			Row.SourceIdentity.strSourceRecipeSha256 + "\n" +
			Row.SourceIdentity.strModuleClosureSha256 + "\n";
		if (Row.LocalDecalPacket.has_value())
			TokenMaterial += Row.LocalDecalPacket->strPacketSha256 + "\n";
	}
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental :
		AdmittedSupplementalElements)
	{
		TokenMaterial += Supplemental.Selector.strEffectAssetId + "\n" +
			Supplemental.Selector.strOccurrenceId + "\n" +
			Supplemental.Selector.strSelectorSha256 + "\n" +
			Supplemental.strRowSha256 + "\n" +
			Supplemental.TargetIdentity.strTargetElementId + "\n" +
			Supplemental.TargetIdentity.strTargetRecordSha256 + "\n";
		if (Supplemental.CascadeRibbonPacket.has_value())
			TokenMaterial += Supplemental.CascadeRibbonPacket->strPacketSha256 + "\n";
		if (Supplemental.AnimationTrailPacket.has_value())
			TokenMaterial += Supplemental.AnimationTrailPacket->strPacketSha256 + "\n";
		if (Supplemental.BakedEdgeLightPacket.has_value())
			TokenMaterial += Supplemental.BakedEdgeLightPacket->strPacketSha256 + "\n";
	}
	for (const EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY& History :
		Program->BakedEdgeHistories)
	{
		TokenMaterial += History.strHistoryId + "\n" +
			History.strHistorySha256 + "\n";
	}
	auto Projection = std::make_shared<EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>();
	Projection->m_pDocument =
		std::make_shared<const EFFECT_DOCUMENT_DESC>(std::move(StagedDocument));
	Projection->m_eProjectionKind = Program->eProjectionKind;
	Projection->m_strEffectAssetId = Program->strEffectAssetId;
	Projection->m_strProgramSha256 = Program->strProgramSha256;
	Projection->m_strBaseDocumentCanonicalSha256 = BaseSha;
	Projection->m_strProjectedDocumentSha256 = ProjectedSha;
	Projection->m_strAdmissionTokenSha256 =
		CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(TokenMaterial);
	Projection->m_AdmittedSelectors = std::move(AdmittedSelectors);
	Projection->m_AdmittedRows = std::move(AdmittedRows);
	Projection->m_AdmittedSupplementalElements =
		std::move(AdmittedSupplementalElements);
	Projection->m_BakedEdgeHistories = Program->BakedEdgeHistories;
	if (!Projection->Is_Valid())
	{
		strOutError = "Visual-program admission token construction failed.";
		return false;
	}
	InOutProjection = std::move(Projection);
	return true;
}

bool_t Client::CEffectVisualProgramCorpusCodec::Derive_TransformTunedProjection(
	const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION& SourceProjection,
	const EFFECT_DOCUMENT_DESC& TunedDocument,
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>& InOutProjection,
	std::string& strOutError)
{
	strOutError.clear();
	if (!SourceProjection.Is_Valid() ||
		TunedDocument.strEffectAssetId != SourceProjection.Get_EffectAssetId() ||
		(SourceProjection.Get_AdmittedRows().empty() &&
		 SourceProjection.Get_AdmittedSupplementalElements().empty()))
	{
		strOutError = "Transform-tuned visual projection source identity is invalid.";
		return false;
	}
	const bool_t bAdapterPacketProjection =
		SourceProjection.Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
	if (bAdapterPacketProjection)
	{
		if (!CEffectDocumentCodec::
			Validate_ReconstructedRuntimeDrawable(
				TunedDocument, strOutError))
		{
			return false;
		}
	}
	else if (!CEffectDocumentCodec::Validate_Drawable(
		TunedDocument, strOutError))
	{
		return false;
	}
	EFFECT_DOCUMENT_DESC Normalized = TunedDocument;
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental :
		SourceProjection.Get_AdmittedSupplementalElements())
	{
		const EFFECT_ELEMENT_DESC* SourceElement = Find_Element(
			SourceProjection.Get_Document(),
			Supplemental.TargetIdentity.strTargetElementId);
		EFFECT_ELEMENT_DESC* NormalizedElement = Find_Element(
			Normalized, Supplemental.TargetIdentity.strTargetElementId);
		if (nullptr == SourceElement || nullptr == NormalizedElement)
		{
			strOutError = "Transform-tuned supplemental target join is missing.";
			return false;
		}
		if (Supplemental.bTuningEligibleTransform)
			NormalizedElement->Detail.Transform = SourceElement->Detail.Transform;
	}
	for (const EFFECT_VISUAL_PROGRAM_ROW& Row : SourceProjection.Get_AdmittedRows())
	{
		if (!Row.TargetIdentity.has_value())
		{
			strOutError = "Transform-tuned visual row target is missing.";
			return false;
		}
		const EFFECT_ELEMENT_DESC* SourceElement = Find_Element(
			SourceProjection.Get_Document(), Row.TargetIdentity->strTargetElementId);
		EFFECT_ELEMENT_DESC* NormalizedElement = Find_Element(
			Normalized, Row.TargetIdentity->strTargetElementId);
		if (nullptr == SourceElement || nullptr == NormalizedElement)
		{
			strOutError = "Transform-tuned visual target join is missing.";
			return false;
		}
		if (Row.bTuningEligibleTransform)
			NormalizedElement->Detail.Transform = SourceElement->Detail.Transform;
	}
	/* Normalizing every eligible target transform back to the source token
	   must make the complete typed document byte-identical.  This rejects
	   material/resource/recipe/attachment edits and transforms on ineligible
	   or fail-closed rows without maintaining a second field-by-field list. */
	if (CEffectDocumentCodec::Serialize(Normalized) !=
		CEffectDocumentCodec::Serialize(SourceProjection.Get_Document()))
	{
		strOutError =
			"Transform-tuned visual projection changed a non-eligible field.";
		return false;
	}
	const std::string TunedSha =
		Compute_DocumentCanonicalSha256(TunedDocument, strOutError);
	if (TunedSha.empty()) return false;
	auto Projection = std::make_shared<EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>();
	Projection->m_pDocument =
		std::make_shared<const EFFECT_DOCUMENT_DESC>(TunedDocument);
	Projection->m_eProjectionKind = SourceProjection.Get_ProjectionKind();
	Projection->m_strEffectAssetId = SourceProjection.Get_EffectAssetId();
	Projection->m_strProgramSha256 = SourceProjection.Get_ProgramSha256();
	Projection->m_strBaseDocumentCanonicalSha256 =
		SourceProjection.Get_BaseDocumentCanonicalSha256();
	Projection->m_strProjectedDocumentSha256 = TunedSha;
	Projection->m_AdmittedSelectors = SourceProjection.Get_AdmittedSelectors();
	Projection->m_AdmittedRows = SourceProjection.Get_AdmittedRows();
	Projection->m_AdmittedSupplementalElements =
		SourceProjection.Get_AdmittedSupplementalElements();
	Projection->m_BakedEdgeHistories =
		SourceProjection.Get_BakedEdgeHistories();
	std::string TokenMaterial =
		"effect-visual-program-transform-tuned-token-v1\n" +
		SourceProjection.Get_AdmissionTokenSha256() + "\n" + TunedSha + "\n";
	for (const EFFECT_VISUAL_PROGRAM_ROW& Row : Projection->m_AdmittedRows)
	{
		TokenMaterial += Row.Selector.strEffectAssetId + "\n" +
			Row.Selector.strOccurrenceId + "\n" + Row.strRowSha256 + "\n";
	}
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental :
		Projection->m_AdmittedSupplementalElements)
	{
		TokenMaterial += Supplemental.Selector.strEffectAssetId + "\n" +
			Supplemental.Selector.strOccurrenceId + "\n" +
			Supplemental.strRowSha256 + "\n";
	}
	Projection->m_strAdmissionTokenSha256 =
		CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(TokenMaterial);
	if (!Projection->Is_Valid())
	{
		strOutError = "Transform-tuned visual admission token construction failed.";
		return false;
	}
	InOutProjection = std::move(Projection);
	return true;
}

bool_t Client::CEffectVisualProgramCorpusCodec::Build_ElementAuthoringPresetStage(
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>&
		pProjection,
	const EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_REQUEST& Request,
	EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_STAGE& InOutStage,
	std::string& strOutError)
{
	strOutError.clear();
	if (nullptr == pProjection || !pProjection->Is_Valid())
	{
		strOutError =
			"Element authoring preset requires an admitted visual projection.";
		return false;
	}
	if (Request.strEffectAssetId.empty() || Request.strOccurrenceId.empty() ||
		Request.strRowSha256.empty() || Request.strTargetElementId.empty() ||
		Request.strSourceRecordId.empty())
	{
		strOutError =
			"Element authoring preset requires complete stable row identity.";
		return false;
	}
	if (Request.strEffectAssetId != pProjection->Get_EffectAssetId())
	{
		strOutError =
			"Element authoring preset effect identity does not match its projection.";
		return false;
	}

	const EFFECT_VISUAL_PROGRAM_ROW* pMatchedVisualRow = nullptr;
	const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* pMatchedSupplemental =
		nullptr;
	size_t iExactMatchCount = 0u;
	for (const EFFECT_VISUAL_PROGRAM_ROW& Row :
		pProjection->Get_AdmittedRows())
	{
		if (Row.Selector.strEffectAssetId != Request.strEffectAssetId ||
			Row.Selector.strOccurrenceId != Request.strOccurrenceId ||
			Row.strRowSha256 != Request.strRowSha256 ||
			!Row.TargetIdentity.has_value() ||
			Row.TargetIdentity->strTargetElementId !=
				Request.strTargetElementId ||
			Row.SourceIdentity.strSourceRecordId != Request.strSourceRecordId)
		{
			continue;
		}
		if (Row.eDisposition !=
				EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
			!Row.AdmissionBlockers.empty())
		{
			strOutError =
				"Element authoring preset visual row is not admitted.";
			return false;
		}
		pMatchedVisualRow = &Row;
		++iExactMatchCount;
	}
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental :
		pProjection->Get_AdmittedSupplementalElements())
	{
		if (Supplemental.Selector.strEffectAssetId !=
				Request.strEffectAssetId ||
			Supplemental.Selector.strOccurrenceId != Request.strOccurrenceId ||
			Supplemental.strRowSha256 != Request.strRowSha256 ||
			Supplemental.TargetIdentity.strTargetElementId !=
				Request.strTargetElementId ||
			Supplemental.strSourceRecordId != Request.strSourceRecordId)
		{
			continue;
		}
		if (Supplemental.eDisposition !=
				EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
			!Supplemental.AdmissionBlockers.empty())
		{
			strOutError =
				"Element authoring preset supplemental row is not admitted.";
			return false;
		}
		pMatchedSupplemental = &Supplemental;
		++iExactMatchCount;
	}
	if (1u != iExactMatchCount)
	{
		strOutError =
			"Element authoring preset requires exactly one admitted row identity match.";
		return false;
	}
	const EFFECT_VISUAL_PROGRAM_FAMILY eMatchedFamily =
		nullptr != pMatchedVisualRow ? pMatchedVisualRow->eFamily :
			pMatchedSupplemental->eFamily;
	if (eMatchedFamily != EFFECT_VISUAL_PROGRAM_FAMILY::MESH_PARTICLE &&
		eMatchedFamily != EFFECT_VISUAL_PROGRAM_FAMILY::SPRITE_PARTICLE &&
		eMatchedFamily != EFFECT_VISUAL_PROGRAM_FAMILY::DECAL_PARTICLE &&
		eMatchedFamily != EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON &&
		eMatchedFamily != EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL)
	{
		strOutError =
			"Element authoring preset row family is not authoring-supported.";
		return false;
	}

	const EFFECT_DOCUMENT_DESC& SourceDocument = pProjection->Get_Document();
	const size_t iTargetCount = static_cast<size_t>(std::count_if(
		SourceDocument.Elements.begin(), SourceDocument.Elements.end(),
		[&Request](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == Request.strTargetElementId;
		}));
	if (1u != iTargetCount)
	{
		strOutError =
			"Element authoring preset requires exactly one projected target Element.";
		return false;
	}

	EFFECT_DOCUMENT_DESC GenericStartingCopy;
	if (!CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
		SourceDocument, Request.strTargetElementId, Request.strEffectAssetId,
		GenericStartingCopy, strOutError))
	{
		return false;
	}
	if (GenericStartingCopy.Elements.size() != 1u ||
		GenericStartingCopy.Elements.front().strElementId !=
			Request.strTargetElementId)
	{
		strOutError =
			"Element authoring preset generic copy returned an invalid target.";
		return false;
	}
	if (nullptr != pMatchedVisualRow &&
		pMatchedVisualRow->LocalDecalPacket.has_value())
	{
		if (!Append_LocalDecalRoleTextureEvidence(
				*pMatchedVisualRow->LocalDecalPacket,
				GenericStartingCopy.Elements.front(), strOutError) ||
			!CEffectDocumentCodec::Validate_Drawable(
				GenericStartingCopy, strOutError))
		{
			return false;
		}
	}

	EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_STAGE Candidate;
	Candidate.Element = std::move(GenericStartingCopy.Elements.front());
	Candidate.eSourceFamily = eMatchedFamily;
	Candidate.bSupplemental = nullptr != pMatchedSupplemental;
	Candidate.pProjection = pProjection;
	Candidate.Identity = Request;
	if (!Candidate.Is_Valid())
	{
		strOutError = "Element authoring preset stage validation failed.";
		return false;
	}

	InOutStage = std::move(Candidate);
	strOutError.clear();
	return true;
}
