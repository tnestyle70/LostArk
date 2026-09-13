#include "Effect_DocumentCodec_Internal.h"
#include "Effect_MaterialTemplate.h"
#include "Generated/Effect_SourceContractRegistry.generated.h"

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


bool_t Client::CEffectDocumentCodec::Validate_SourceContract(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	if (Document.iFormatVersion != EFFECT_AUTHORING_FORMAT_VERSION ||
		Document.iLoadedFormatVersion != EFFECT_SOURCE_CONTRACT_FORMAT_VERSION ||
		!Document.bSourceContract)
	{
		strOutError = "Effect document is not a native-v14 source contract.";
		return false;
	}
	if (!Is_ValidEffectBloomIntensity(Document.fBloomIntensity))
	{
		strOutError = "Effect bloomIntensity must be finite and between 0 and 16.";
		return false;
	}
	if (!Validate_AuthoredRuntimeExtensions(Document, strOutError))
		return false;
	if (!Is_StableId(Document.strEffectAssetId) ||
		Document.strDisplayName.empty() || Document.strDisplayName.size() > 128u ||
		Document.Elements.empty() || Document.Elements.size() > MAX_ELEMENTS)
	{
		strOutError = "Source-contract document identity or size is invalid.";
		return false;
	}

	const auto SafeAssetId = [](const std::string& Value)
	{
		return !Value.empty() && Value.size() <= MAX_RESOURCE_ID_BYTES &&
			'/' != Value.front() && std::string::npos == Value.find('\\') &&
			std::string::npos == Value.find(':') &&
			std::string::npos == Value.find("../") &&
			std::string::npos == Value.find("/..");
	};
	const auto ShapeForRenderer = [](const EFFECT_RENDERER_TYPE eType)
		-> std::string_view
	{
		switch (eType)
		{
		case EFFECT_RENDERER_TYPE::MESH_PARTICLE: return "mesh";
		case EFFECT_RENDERER_TYPE::SPRITE_PARTICLE: return "sprite";
		case EFFECT_RENDERER_TYPE::DECAL_PARTICLE: return "decal";
		case EFFECT_RENDERER_TYPE::CASCADE_RIBBON: return "ribbon";
		case EFFECT_RENDERER_TYPE::LIGHT_PARTICLE: return "light";
		case EFFECT_RENDERER_TYPE::SCREEN_POST: return "screenPost";
		default: return {};
		}
	};

	const auto IsFiniteFloat3 = [](const float3_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z);
	};
	const auto IsFiniteFloat4 = [](const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	};
	const auto IsZeroFloat4 = [](const float4_t& Value)
	{
		return Value.x == 0.f && Value.y == 0.f && Value.z == 0.f &&
			Value.w == 0.f;
	};
	const auto IsBlockedAdmission = [](const EFFECT_SOURCE_ADMISSION_DESC& Admission)
	{
		std::unordered_set<std::string> UniqueBlockers;
		return !Admission.bAllowed && !Admission.Blockers.empty() &&
			std::all_of(Admission.Blockers.begin(), Admission.Blockers.end(),
				[&UniqueBlockers](const std::string& Blocker)
				{
					return !Blocker.empty() && Blocker.size() <= 256u &&
						Has_VisibleCharacter(Blocker) &&
						UniqueBlockers.insert(Blocker).second;
				});
	};
	const auto IncludesBlockers = [](const std::vector<std::string>& Superset,
		const std::vector<std::string>& Subset)
	{
		return std::all_of(Subset.begin(), Subset.end(),
			[&Superset](const std::string& Blocker)
			{
				return std::find(Superset.begin(), Superset.end(), Blocker) !=
					Superset.end();
			});
	};
	const auto ValidateTypedFields = [&](const auto& Fields)
	{
		if (Fields.size() > MAX_SOURCE_TYPED_FIELDS_PER_REFERENCE)
			return false;
		std::unordered_set<std::string> PropertyPaths;
		for (const EFFECT_SOURCE_TYPED_FIELD_DESC& Field : Fields)
		{
			if (Field.strPropertyPath.empty() ||
				Field.strPropertyPath.size() > 512u ||
				!Has_VisibleCharacter(Field.strPropertyPath) ||
				Field.eKind >= EFFECT_SOURCE_TYPED_FIELD_KIND::END ||
				!PropertyPaths.insert(Field.strPropertyPath).second ||
				(EFFECT_SOURCE_TYPED_FIELD_KIND::NUMBER == Field.eKind &&
					!std::isfinite(Field.fNumber)) ||
				(EFFECT_SOURCE_TYPED_FIELD_KIND::STRING == Field.eKind &&
					Field.strString.size() > 2048u) ||
				(EFFECT_SOURCE_TYPED_FIELD_KIND::VECTOR == Field.eKind &&
					!IsFiniteFloat4(Field.vVector)))
			{
				return false;
			}
		}
		return true;
	};
	constexpr std::array<std::string_view, 6u> CompositionOrder = {
		"carrierGeometryPreScale", "signedParticleScaleRotationLocation",
		"emitterElementTransform", "cueLocalTransform",
		"attachmentSocketOrRoot", "actorWorld"
	};
	std::unordered_set<std::string> ElementIds;
	std::unordered_set<std::string> EvidenceIds;
	std::string strEvidenceArtifactFileSha256;
	std::string strEvidenceArtifactSelfSha256;
	std::string strLocalReferenceFileSha256;
	std::string strLocalReferenceSelfSha256;
	std::string strGeometryFileSha256;
	std::string strGeometrySelfSha256;
	std::unordered_set<std::string> LocalReferenceOccurrenceIds;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (!Is_StableId(Element.strElementId) ||
			!ElementIds.insert(Element.strElementId).second ||
			Element.eKind >= EFFECT_ELEMENT_KIND::END ||
			Element.eCompositionLayer != EFFECT_COMPOSITION_LAYER::NORMAL ||
			!Element.Detail.Decal.Is_ReceiverDefault() ||
			Element.Renderer.eType >= EFFECT_RENDERER_TYPE::END ||
			Element.Renderer.eSourceSpace >= EFFECT_SOURCE_SPACE::END ||
			Kind_ForRenderer(Element.Renderer.eType) != Element.eKind)
		{
			strOutError = "Source-contract Element identity or renderer is invalid.";
			return false;
		}
		const EFFECT_SOURCE_SPACE eExpectedSpace =
			EFFECT_RENDERER_TYPE::SCREEN_POST == Element.Renderer.eType ?
				EFFECT_SOURCE_SPACE::SCREEN_SPACE_V1 :
				EFFECT_SOURCE_SPACE::UE3_CASCADE_V1;
		if (Element.Renderer.eSourceSpace != eExpectedSpace)
		{
			strOutError = "Source-contract renderer source space is inconsistent.";
			return false;
		}
		const f32_t fTransformMotionDuration = Element.Detail.Timing.
			fTransformMotionDurationSeconds;
		if (!std::isfinite(fTransformMotionDuration) ||
			fTransformMotionDuration < 0.f ||
			(fTransformMotionDuration > 0.f &&
			 (!std::isfinite(Element.Detail.Timing.fLifeTimeSeconds) ||
			  fTransformMotionDuration >
				Element.Detail.Timing.fLifeTimeSeconds ||
			  (Element.Renderer.eType !=
				EFFECT_RENDERER_TYPE::STANDALONE_MESH &&
			   Element.Renderer.eType !=
				EFFECT_RENDERER_TYPE::MESH_PARTICLE))))
		{
			strOutError =
				"Source-contract transform motion timing is invalid.";
			return false;
		}
		for (const EFFECT_RESOURCE_BINDING_DESC& Resource :
			Element.ResourceBindings)
		{
			if (!Is_StableId(Resource.strSlotId) ||
				!SafeAssetId(Resource.strAssetId))
			{
				strOutError = "Source-contract resource identity is unsafe.";
				return false;
			}
		}
		if (!Validate_MaterialExecution(Element.Material.Execution,
			strOutError))
		{
			return false;
		}
		if (!Element.Detail.Mesh.SourceMaterialSlots.empty() ||
			Element.Material.SourceMaterial.bEnabled ||
			Element.Material.Execution.bEnabled ||
			Element.Material.Execution.bFailClosed)
		{
			strOutError =
				"Source-contract candidates cannot enable runtime SourceMaterial or authored Material execution.";
			return false;
		}

		const EFFECT_CASCADE_RECIPE_DESC& Recipe = Element.SourceRecipe;
		const EFFECT_SOURCE_COMPILER_EVIDENCE_DESC& Evidence =
			Recipe.CompilerEvidence;
		if (!Recipe.bEnabled ||
			Recipe.strRendererShape != ShapeForRenderer(Element.Renderer.eType) ||
			Recipe.strSourceContractProfileId != EFFECT_SOURCE_CONTRACT_PROFILE_ID ||
			Recipe.strSourceContractSha256 != EFFECT_SOURCE_CONTRACT_SHA256 ||
			!Is_LowerHexSha256(Recipe.strSourceGraphSha256) ||
			!Is_LowerHexSha256(Recipe.strSourceClosureSha256) ||
			!Is_LowerHexSha256(Recipe.strSourceMaterialClosureSha256) ||
			0u == Recipe.iSourcePeakActiveParticles ||
			Recipe.Modules.empty() ||
			Recipe.Modules.size() > MAX_SOURCE_MODULES_PER_ELEMENT ||
			Recipe.LocalReferenceBindings.size() >
				MAX_SOURCE_LOCAL_REFERENCE_BINDINGS_PER_ELEMENT ||
			Recipe.ModuleCoverage.size() != Recipe.Modules.size())
		{
			strOutError = "Source-contract recipe identity or coverage is invalid.";
			return false;
		}

		const auto SharedSha = [&](const std::string& Value,
			std::string& Shared) -> bool_t
		{
			if (!Is_LowerHexSha256(Value))
				return false;
			if (Shared.empty())
				Shared = Value;
			return Shared == Value;
		};
		if (!SharedSha(Evidence.strArtifactFileSha256,
				strEvidenceArtifactFileSha256) ||
			!SharedSha(Evidence.strArtifactSelfSha256,
				strEvidenceArtifactSelfSha256) ||
			!SharedSha(Evidence.strLocalReferenceClosureFileSha256,
				strLocalReferenceFileSha256) ||
			!SharedSha(Evidence.strLocalReferenceClosureSelfSha256,
				strLocalReferenceSelfSha256) ||
			!SharedSha(Evidence.strGeometryParityFileSha256,
				strGeometryFileSha256) ||
			!SharedSha(Evidence.strGeometryParitySelfSha256,
				strGeometrySelfSha256) ||
			Evidence.strSourceEvidenceStatus.empty() ||
			Evidence.strEvidenceId.empty() ||
			!EvidenceIds.insert(Evidence.strEvidenceId).second ||
			Evidence.strSourceCueId.empty() ||
			Evidence.strSourceOccurrenceId.empty() ||
			Evidence.strSourceSystemId.empty() ||
			Evidence.strSourceEmitterPath.empty() ||
			Evidence.strSourceEmitterNodeId.empty() ||
			Evidence.strLodSelectionPolicy != "FIRST_LOD_ONLY" ||
			Evidence.strSelectedLodPath.empty() ||
			Evidence.strSelectedLodNodeId.empty() ||
			0u != Evidence.iSelectedLodArrayIndex ||
			Evidence.strSelectedLodLevelProvenance.empty() ||
			Evidence.strSelectedLodEnabledProvenance.empty() ||
			Evidence.ModuleReferenceOrder.size() != Recipe.Modules.size() ||
			Evidence.CompositionOrder.size() != CompositionOrder.size() ||
			!std::equal(Evidence.CompositionOrder.begin(),
				Evidence.CompositionOrder.end(), CompositionOrder.begin()) ||
			!IsFiniteFloat3(Evidence.vCueSourcePositionUeUnits) ||
			!IsFiniteFloat3(Evidence.CueLocalTransform.vPosition) ||
			!IsFiniteFloat3(Evidence.CueLocalTransform.vRotationDegrees) ||
			!IsFiniteFloat3(Evidence.CueLocalTransform.vScale))
		{
			strOutError = "Source-contract compiler evidence is invalid.";
			return false;
		}
		for (size_t iReference = 0u;
			iReference < Evidence.ModuleReferenceOrder.size(); ++iReference)
		{
			const EFFECT_SOURCE_MODULE_REFERENCE_DESC& Reference =
				Evidence.ModuleReferenceOrder[iReference];
			const std::string strExpectedStableId = Reference.strSourceObjectId +
				"@ref:" + std::to_string(Reference.iSourceReferenceIndex);
			if (Reference.iOrder != iReference ||
				Reference.iSourceReferenceIndex != iReference ||
				(Reference.strRole != "REQUIRED" &&
					Reference.strRole != "MODULE" &&
					Reference.strRole != "SPAWN" &&
					Reference.strRole != "TYPE_DATA") ||
				Reference.strSourceObjectId.empty() ||
				!Is_LowerHexSha256(Reference.strSourceRecordSha256) ||
				Recipe.Modules[iReference].strStableId != strExpectedStableId)
			{
				strOutError = "Source-contract module reference order is invalid.";
				return false;
			}
		}
		std::unordered_set<uint32_t> ParameterIndices;
		for (const EFFECT_SOURCE_PARAMETER_OVERRIDE_DESC& Parameter :
			Evidence.ParameterOverrides)
		{
			if (!ParameterIndices.insert(Parameter.iSourceIndex).second ||
				Parameter.strName.empty() ||
				(Parameter.strType != "scalar" && Parameter.strType != "vector") ||
				(Parameter.strType == "scalar" &&
					!std::isfinite(Parameter.fScalarValue)) ||
				(Parameter.strType == "vector" &&
					!IsFiniteFloat3(Parameter.vVectorValue)))
			{
				strOutError = "Source-contract cue parameter evidence is invalid.";
				return false;
			}
		}

		if (!IsBlockedAdmission(Recipe.CompiledExecutionAdmission))
		{
			strOutError = "Source-contract compiled execution is not fail-closed.";
			return false;
		}

		std::unordered_map<std::string,
			const EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC*>
			LocalReferenceBindingsByOccurrence;
		std::unordered_set<std::string> DistributionBindingOccurrenceIds;
		for (const EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC& Binding :
			Recipe.LocalReferenceBindings)
		{
			const bool_t bDistributionReference =
				Binding.strReferenceKind == "DISTRIBUTION_TARGET";
			const bool_t bTypedDataReference =
				Binding.strReferenceKind == "TYPEDATA_COMPONENT";
			if ((!bDistributionReference && !bTypedDataReference) ||
				Binding.strReferenceId.empty() ||
				Binding.strReferenceId.size() > 512u ||
				!Has_VisibleCharacter(Binding.strReferenceId) ||
				Binding.strDefinitionId.empty() ||
				Binding.strDefinitionId.size() > 512u ||
				!Has_VisibleCharacter(Binding.strDefinitionId) ||
				Binding.strOccurrenceId.empty() ||
				Binding.strOccurrenceId.size() > 512u ||
				!Has_VisibleCharacter(Binding.strOccurrenceId) ||
				Binding.strModuleStableId.empty() ||
				Binding.strModuleStableId.size() > 512u ||
				!Has_VisibleCharacter(Binding.strModuleStableId) ||
				Binding.strPropertyPath.empty() ||
				Binding.strPropertyPath.size() > 512u ||
				!Has_VisibleCharacter(Binding.strPropertyPath) ||
				Binding.strProvenance.empty() ||
				Binding.strProvenance.size() > 2048u ||
				!Has_VisibleCharacter(Binding.strProvenance) ||
				std::none_of(Recipe.Modules.begin(), Recipe.Modules.end(),
					[&Binding](const EFFECT_SOURCE_MODULE_DESC& Module)
					{
						return Module.strStableId == Binding.strModuleStableId;
					}) ||
				!ValidateTypedFields(Binding.ExactPayload) ||
				!ValidateTypedFields(Binding.CurrentDefaultEvidence) ||
				!IsBlockedAdmission(Binding.ExecutionAdmission) ||
				!IncludesBlockers(Recipe.CompiledExecutionAdmission.Blockers,
					Binding.ExecutionAdmission.Blockers) ||
				!LocalReferenceOccurrenceIds.insert(
					Binding.strOccurrenceId).second ||
				!LocalReferenceBindingsByOccurrence.emplace(
					Binding.strOccurrenceId, &Binding).second)
			{
				strOutError =
					"Source-contract local-reference binding is invalid.";
				return false;
			}
			if (bDistributionReference)
				DistributionBindingOccurrenceIds.insert(Binding.strOccurrenceId);
		}
		const bool_t bNonRenderLight =
			Recipe.MaterialAdmission.strStatus ==
				"NON_RENDER_BUILTIN_NOT_APPLICABLE";
		const bool_t bBlockedMaterial = Recipe.MaterialAdmission.strStatus.starts_with(
			"BLOCKED_");
		if (Recipe.MaterialAdmission.strStatus.empty() ||
			(bBlockedMaterial &&
				(Recipe.MaterialAdmission.Blockers.empty() ||
				 !Recipe.MaterialAdmission.strMaterialRecipeId.empty() ||
				 !Recipe.MaterialAdmission.strRenderStateRecipeId.empty())) ||
			(!bBlockedMaterial && !bNonRenderLight &&
				(Recipe.MaterialAdmission.strMaterialRecipeId.empty() ||
				 Recipe.MaterialAdmission.strRenderStateRecipeId.empty() ||
				 !Recipe.MaterialAdmission.Blockers.empty())) ||
			(bNonRenderLight &&
				(Element.Renderer.eType != EFFECT_RENDERER_TYPE::LIGHT_PARTICLE ||
				 !Recipe.MaterialAdmission.Blockers.empty())))
		{
			strOutError = "Source-contract material admission is invalid.";
			return false;
		}

		const EFFECT_SOURCE_GEOMETRY_BINDING_DESC& Geometry =
			Recipe.GeometryBinding;
		if (Geometry.strReceiptFileSha256 != strGeometryFileSha256 ||
			Geometry.strReceiptSelfSha256 != strGeometrySelfSha256 ||
			!std::isfinite(Geometry.fCarrierGeometryPreScale) ||
			Geometry.fCarrierGeometryPreScale <= 0.f)
		{
			strOutError = "Source-contract geometry receipt binding is invalid.";
			return false;
		}
		const auto MeshBinding = std::find_if(
			Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
			[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
			});
		if (Element.Renderer.eType == EFFECT_RENDERER_TYPE::MESH_PARTICLE)
		{
			if (!Geometry.bEnabled || Geometry.strAssetId.empty() ||
				MeshBinding == Element.ResourceBindings.end() ||
				Geometry.strAssetId != MeshBinding->strAssetId ||
				Geometry.strParticleScaleSemantics.empty() ||
				Geometry.strStatus.empty())
			{
				strOutError = "Source-contract Mesh carrier scale contract is invalid.";
				return false;
			}
		}
		else if (Geometry.bEnabled || !Geometry.strAssetId.empty() ||
			std::abs(Geometry.fCarrierGeometryPreScale - 1.f) > 1e-7f ||
			Geometry.strParticleScaleSemantics != "NOT_APPLICABLE" ||
			Geometry.strStatus != "NOT_APPLICABLE" || !Geometry.Blockers.empty())
		{
			strOutError = "Source-contract non-Mesh geometry binding is invalid.";
			return false;
		}
		std::unordered_map<std::string,
			const EFFECT_SOURCE_MODULE_COVERAGE_DESC*> CoverageById;
		for (const EFFECT_SOURCE_MODULE_COVERAGE_DESC& Coverage :
			Recipe.ModuleCoverage)
		{
			std::unordered_set<std::string> CoverageBlockers;
			const bool_t bBlockersValid = std::all_of(
				Coverage.Blockers.begin(), Coverage.Blockers.end(),
				[&CoverageBlockers](const std::string& Blocker)
				{
					return !Blocker.empty() &&
						CoverageBlockers.insert(Blocker).second;
				});
			if (Coverage.strModuleStableId.empty() ||
				Coverage.strModuleStableId.size() > 512u ||
				!Has_VisibleCharacter(Coverage.strModuleStableId) ||
				(!Coverage.strExactSourceClass.empty() &&
					(Coverage.strExactSourceClass.size() > 256u ||
					 !Has_VisibleCharacter(Coverage.strExactSourceClass))) ||
				(Coverage.strExactSourceClass.empty() &&
					!Coverage.strAliasId.empty()) ||
				Coverage.strAliasId.size() > 256u ||
				(!Coverage.strAliasId.empty() &&
					!Has_VisibleCharacter(Coverage.strAliasId)) ||
				Coverage.strNormalizedClass.empty() ||
				Coverage.eStatus >= EFFECT_SOURCE_COVERAGE_STATUS::END ||
				!bBlockersValid ||
				(Coverage.eStatus == EFFECT_SOURCE_COVERAGE_STATUS::UNRESOLVED &&
					Coverage.Blockers.empty()) ||
				!IncludesBlockers(
					Recipe.CompiledExecutionAdmission.Blockers, Coverage.Blockers) ||
				Coverage.Properties.size() >
					MAX_SOURCE_COVERAGE_PROPERTIES_PER_MODULE ||
				!CoverageById.emplace(
					Coverage.strModuleStableId, &Coverage).second)
			{
				strOutError = "Source-contract module coverage is invalid.";
				return false;
			}
		}

		std::unordered_set<std::string>
			ConsumedDistributionBindingOccurrenceIds;
		for (const EFFECT_SOURCE_MODULE_DESC& Module : Recipe.Modules)
		{
			const auto CoverageIterator = CoverageById.find(Module.strStableId);
			if (Module.strStableId.empty() || Module.strStableId.size() > 512u ||
				!Has_VisibleCharacter(Module.strStableId) || Module.strClassName.empty() ||
				Module.strObjectPath.empty() ||
				CoverageIterator == CoverageById.end() ||
				(!CoverageIterator->second->strExactSourceClass.empty() &&
					CoverageIterator->second->strExactSourceClass !=
						Module.strClassName) ||
				CoverageIterator->second->strNormalizedClass !=
					(CoverageIterator->second->strExactSourceClass.empty() ?
						Normalize_SourceModuleClass(Module.strClassName) :
						Canonicalize_ExactSourceModuleClass(
							Module.strClassName)))
			{
				strOutError = "Source-contract module identity is invalid.";
				return false;
			}

			std::unordered_set<std::string> CoveredProperties;
			std::unordered_map<std::string,
				const EFFECT_SOURCE_PROPERTY_COVERAGE_DESC*>
				CoveragePropertiesByKey;
			for (const EFFECT_SOURCE_PROPERTY_COVERAGE_DESC& Property :
				CoverageIterator->second->Properties)
			{
				std::unordered_set<std::string> PropertyBlockers;
				const bool_t bPropertyBlockersValid = std::all_of(
					Property.Blockers.begin(), Property.Blockers.end(),
					[&PropertyBlockers](const std::string& Blocker)
					{
						return !Blocker.empty() && Blocker.size() <= 256u &&
							Has_VisibleCharacter(Blocker) &&
							PropertyBlockers.insert(Blocker).second;
					});
				const std::string strCoverageKey = Property.strStorage + "\n" +
					Property.strPropertyPath;
				if ((Property.strStorage != "literal" &&
						Property.strStorage != "distribution") ||
					Property.strPropertyPath.empty() ||
					Property.strPropertyPath.size() > 512u ||
					Property.strProvenance.empty() ||
					Property.strProvenance.size() > 2048u ||
					Property.eStatus >= EFFECT_SOURCE_COVERAGE_STATUS::END ||
					!bPropertyBlockersValid ||
					(Property.eStatus == EFFECT_SOURCE_COVERAGE_STATUS::UNRESOLVED &&
						Property.Blockers.empty()) ||
					!IncludesBlockers(
						CoverageIterator->second->Blockers, Property.Blockers) ||
					!CoveredProperties.insert(strCoverageKey).second ||
					!CoveragePropertiesByKey.emplace(
						strCoverageKey, &Property).second)
				{
					strOutError =
						"Source-contract property coverage is invalid.";
					return false;
				}
			}
			for (const EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC& Binding :
				Recipe.LocalReferenceBindings)
			{
				if (Binding.strModuleStableId != Module.strStableId)
					continue;
				const auto BindingCoverage = std::find_if(
					CoverageIterator->second->Properties.begin(),
					CoverageIterator->second->Properties.end(),
					[&Binding](
						const EFFECT_SOURCE_PROPERTY_COVERAGE_DESC& Property)
					{
						const std::string_view ExpectedStorage =
							Binding.strReferenceKind == "DISTRIBUTION_TARGET" ?
								"distribution" : "literal";
						return Property.strStorage == ExpectedStorage &&
							Property.strPropertyPath == Binding.strPropertyPath;
					});
				if (BindingCoverage ==
						CoverageIterator->second->Properties.end() ||
					!IncludesBlockers(BindingCoverage->Blockers,
						Binding.ExecutionAdmission.Blockers))
				{
					strOutError =
						"Source local-reference blockers are not propagated.";
					return false;
				}
			}

			std::unordered_set<std::string> SourceProperties;
			for (const EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
			{
				if (Literal.strPropertyPath.empty() ||
					Literal.eKind >= EFFECT_SOURCE_LITERAL_KIND::END ||
					(EFFECT_SOURCE_LITERAL_KIND::NUMBER == Literal.eKind &&
						!std::isfinite(Literal.fNumber)) ||
					!SourceProperties.insert(
						"literal\n" + Literal.strPropertyPath).second)
				{
					strOutError = "Source-contract literal is invalid or duplicated.";
					return false;
				}
			}
			for (const EFFECT_DISTRIBUTION_DESC& Distribution :
				Module.Distributions)
			{
				const std::string strCoverageKey = "distribution\n" +
					Distribution.strPropertyPath;
				const auto PropertyCoverageIterator =
					CoveragePropertiesByKey.find(strCoverageKey);
				const bool_t bHasReference =
					!Distribution.strReferenceId.empty();
				const bool_t bHasOccurrence =
					!Distribution.strOccurrenceId.empty();
				if (Distribution.strPropertyPath.empty() ||
					Distribution.strPropertyPath.size() > 256u ||
					Distribution.strSourceClass.size() > 128u ||
					Distribution.strSourceObjectPath.size() > 512u ||
					Distribution.iComponentCount < 1u ||
					Distribution.iComponentCount > 4u ||
					Distribution.strPayloadStatus.empty() ||
					Distribution.strPayloadStatus.size() > 256u ||
					!Has_VisibleCharacter(Distribution.strPayloadStatus) ||
					Distribution.strFidelity.empty() ||
					Distribution.strFidelity.size() > 256u ||
					!Has_VisibleCharacter(Distribution.strFidelity) ||
					bHasReference != bHasOccurrence ||
					!IsBlockedAdmission(Distribution.ExecutionAdmission) ||
					PropertyCoverageIterator == CoveragePropertiesByKey.end() ||
					!IncludesBlockers(
						PropertyCoverageIterator->second->Blockers,
						Distribution.ExecutionAdmission.Blockers) ||
					!SourceProperties.insert(strCoverageKey).second)
				{
					strOutError =
						"Source-contract distribution admission is invalid.";
					return false;
				}

				const bool_t bUnresolved =
					Is_UnresolvedSourceToken(Distribution.strPayloadStatus) ||
					Is_UnresolvedSourceToken(Distribution.strFidelity);
				if (bUnresolved &&
					PropertyCoverageIterator->second->eStatus !=
						EFFECT_SOURCE_COVERAGE_STATUS::UNRESOLVED)
				{
					strOutError =
						"Unresolved source distribution coverage is not unresolved.";
					return false;
				}
				if (bUnresolved &&
					(Distribution.eParameterBinding !=
						EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE ||
					 !Distribution.strParameterName.empty() ||
					 0u != Distribution.iOperation ||
					 0u != Distribution.iRandomLockAxes ||
					 0u != Distribution.iLookupTableChunkSize ||
					 0u != Distribution.iLookupTableNumElements ||
					 0.f != Distribution.fLookupTableTimeScale ||
					 0.f != Distribution.fLookupTableStartTime ||
					 !IsZeroFloat4(Distribution.vDefaultMinimum) ||
					 !IsZeroFloat4(Distribution.vDefaultMaximum) ||
					 !Distribution.LookupTable.empty() ||
					 !Distribution.Keys.empty()))
				{
					strOutError =
						"Unresolved source distribution carries executable payload.";
					return false;
				}

				if (bHasOccurrence)
				{
					const auto BindingIterator =
						LocalReferenceBindingsByOccurrence.find(
							Distribution.strOccurrenceId);
					if (BindingIterator ==
							LocalReferenceBindingsByOccurrence.end() ||
						BindingIterator->second->strReferenceKind !=
							"DISTRIBUTION_TARGET" ||
						BindingIterator->second->strReferenceId !=
							Distribution.strReferenceId ||
						BindingIterator->second->strOccurrenceId !=
							Distribution.strOccurrenceId ||
						BindingIterator->second->strModuleStableId !=
							Module.strStableId ||
						BindingIterator->second->strPropertyPath !=
							Distribution.strPropertyPath ||
						!IncludesBlockers(
							BindingIterator->second->ExecutionAdmission.Blockers,
							Distribution.ExecutionAdmission.Blockers) ||
						!IncludesBlockers(
							Distribution.ExecutionAdmission.Blockers,
							BindingIterator->second->ExecutionAdmission.Blockers) ||
						!IncludesBlockers(
							PropertyCoverageIterator->second->Blockers,
							BindingIterator->second->ExecutionAdmission.Blockers) ||
						!ConsumedDistributionBindingOccurrenceIds.insert(
							Distribution.strOccurrenceId).second)
					{
						strOutError =
							"Source distribution local-reference link is invalid.";
						return false;
					}
				}

				if (!bUnresolved &&
					!CEffectDistribution::Validate(Distribution, strOutError))
				{
					return false;
				}
				const bool_t bParticleParameter =
					Is_ParticleParameterDistribution(Distribution.strSourceClass);
				if ((bParticleParameter &&
						Distribution.eParameterBinding >=
							EFFECT_DISTRIBUTION_PARAMETER_BINDING::END) ||
					(!bParticleParameter &&
						(Distribution.eParameterBinding !=
							EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE ||
						 !Distribution.strParameterName.empty())))
				{
					strOutError =
						"Source-contract ParticleParameter binding is invalid.";
					return false;
				}
			}

			if (SourceProperties != CoveredProperties)
			{
				strOutError =
					"Source-contract property coverage does not match source payload.";
				return false;
			}
		}
		if (ConsumedDistributionBindingOccurrenceIds !=
			DistributionBindingOccurrenceIds)
		{
			strOutError =
				"Source-contract distribution bindings are orphaned or missing.";
			return false;
		}
	}
	strOutError.clear();
	return true;
}
