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

namespace Client::EffectDocumentCodecDetail
{


	bool_t Validate_ExecutableSourceMaterialCarrier(
		const Client::EFFECT_ELEMENT_DESC& Element,
		bool_t& bOutOwnsDrawableContract,
		std::string& strOutError)
	{
		using namespace Client;
		bOutOwnsDrawableContract = false;
		const EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial =
			Element.Material.SourceMaterial;
		const bool_t bSourceMaterialCarrier = SourceMaterial.bEnabled ||
			Element.Material.strTemplateId == EFFECT_SOURCE_MATERIAL_TEMPLATE_ID;
		if (!bSourceMaterialCarrier)
		{
			if (Element.Material.Execution.bAuthoringApproximate)
			{
				strOutError =
					"Authoring-approximate carrier requires an enabled source Material profile.";
				return false;
			}
			return true;
		}

		if (!SourceMaterial.bEnabled ||
			SourceMaterial.eStatus >= EFFECT_SOURCE_MATERIAL_STATUS::UNSUPPORTED ||
			!Is_SupportedEffectSourceRuntimeShaderProfile(
				SourceMaterial.strRuntimeShaderProfileId))
		{
			strOutError =
				"Active source Material carrier has no ready source profile.";
			return false;
		}
		if (SourceMaterial.strRuntimeShaderProfileId ==
			"effect.ue3.fallback-blocked.v1")
		{
			strOutError =
				"Fallback-blocked source Material profile cannot be activated.";
			return false;
		}
		if (Element.Material.Execution.bAuthoringApproximate)
		{
			const bool_t bHasTextureCarrier = std::any_of(
				Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId != EFFECT_MESH_SHAPE_SLOT_ID;
				});
			const bool_t bRequiresMesh =
				Element.eKind == EFFECT_ELEMENT_KIND::MESH ||
				Element.SourceRecipe.strRendererShape == "mesh";
			const bool_t bHasMeshCarrier = std::any_of(
				Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
				});
			if (!bHasTextureCarrier || (bRequiresMesh && !bHasMeshCarrier))
			{
				strOutError =
					"Authoring-approximate carrier lacks its exact texture or mesh resource.";
				return false;
			}
		}

		const EFFECT_MATERIAL_TEMPLATE_DESC* pTemplate =
			Find_EffectMaterialTemplate(Element.Material.strTemplateId);
		if (nullptr == pTemplate)
		{
			strOutError = "Active source Material Template is not registered.";
			return false;
		}
		const auto FindBinding = [&](const EFFECT_MATERIAL_INPUT_SEMANTIC eSemantic)
			-> const EFFECT_RESOURCE_BINDING_DESC*
		{
			const EFFECT_MATERIAL_INPUT_SLOT_DESC* pInput =
				Find_EffectMaterialInput(*pTemplate, eSemantic);
			if (nullptr == pInput)
				return nullptr;
			const auto Iterator = std::find_if(
				Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
				[pInput](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == pInput->strSlotId;
				});
			return Iterator == Element.ResourceBindings.end() ? nullptr : &*Iterator;
		};
		const EFFECT_RESOURCE_BINDING_DESC* pBaseBinding = FindBinding(
			EFFECT_MATERIAL_INPUT_SEMANTIC::BASE);
		const bool_t bSafeBase = nullptr != pBaseBinding &&
			!Is_UnsafeEffectBaseTextureAssetId(pBaseBinding->strAssetId);
		const bool_t bHasNoise = nullptr != FindBinding(
			EFFECT_MATERIAL_INPUT_SEMANTIC::NOISE);
		const bool_t bHasMask = nullptr != FindBinding(
			EFFECT_MATERIAL_INPUT_SEMANTIC::MASK);
		const bool_t bHasEmissive = nullptr != FindBinding(
			EFFECT_MATERIAL_INPUT_SEMANTIC::EMISSIVE);
		const bool_t bHasDissolve = nullptr != FindBinding(
			EFFECT_MATERIAL_INPUT_SEMANTIC::DISSOLVE);
		const bool_t bParticleMesh =
			EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind &&
			std::any_of(Element.ResourceBindings.begin(),
				Element.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
				});
		const std::string_view strProfile =
			SourceMaterial.strRuntimeShaderProfileId;
		if (nullptr != Find_LanceMasterVAProgram(strProfile) && !Has_LanceMasterVAMaterialContract(Element))
		{
			strOutError = "Native Lance Master requires its recovered material, carrier and named inputs.";
			return false;
		}
		if (nullptr != Find_ArtistProgram(strProfile) && !Has_ArtistMaterialContract(Element))
		{
			strOutError = "Native Artist requires its recovered material variant, carrier and named inputs.";
			return false;
		}
		if (nullptr != Find_WarlordNativeProgram(strProfile) && !Has_WarlordNativeMaterialContract(Element))
		{
			strOutError = "Native Warlord requires its recovered material variant, carrier and named inputs.";
			return false;
		}
		if (nullptr != Find_DimensionMasterSDProgram(strProfile) &&
			!Has_DimensionMasterSDMaterialContract(Element))
		{
			strOutError = "Native S material requires its recovered source carrier and named inputs.";
			return false;
		}
		if (nullptr != Find_DimensionMasterWRProgram(strProfile) &&
			!Has_DimensionMasterWRMaterialContract(Element))
		{
			strOutError = "Native W/R material requires its recovered source variant, carrier and named inputs.";
			return false;
		}
		if (nullptr != Find_DimensionMasterALTVProgram(strProfile) &&
			!Has_DimensionMasterALTVMaterialContract(Element))
		{
			strOutError = "Native ALT_V material requires its recovered source variant, carrier and named inputs.";
			return false;
		}
		if (nullptr != Find_DimensionMasterVProgram(strProfile) &&
			!Has_DimensionMasterVMaterialContract(Element))
		{
			strOutError = "Native V material requires its recovered source variant, carrier and named inputs.";
			return false;
		}
		if (nullptr != Find_DimensionMasterQProgram(strProfile) &&
			!Has_DimensionMasterQMaterialContract(Element))
		{
			strOutError = "Native Q material requires its recovered source variant, carrier and named inputs.";
			return false;
		}
		if (strProfile == EFFECT_SLICE_SCENE_DEPTH_RUNTIME_PROFILE_ID &&
			!Has_EffectSliceSceneDepthContract(Element))
		{
			strOutError = "Slice scene-depth material requires its sprite, flow texture and exact four dynamic lanes.";
			return false;
		}
		if (strProfile == EFFECT_CUBESAMPLE_SCENE_RUNTIME_PROFILE_ID &&
			!Has_EffectCubeSampleSceneContract(Element))
		{
			strOutError = "CubeSample scene material requires its mesh, named spec texture and unclamped source variant.";
			return false;
		}
		if (strProfile == "effect.ue3.grouped-translucent.v1" &&
			!Is_EffectGroupedTranslucentResourceContractSatisfied(
				SourceMaterial, bSafeBase, bHasMask, bHasEmissive, bHasDissolve))
		{
			strOutError =
				"Grouped-translucent source Material resource contract is not satisfied.";
			return false;
		}
		if (strProfile == "effect.ue3.linearflow-02.v1" &&
			!Has_EffectLinearFlowNamedTextureContract(SourceMaterial))
		{
			strOutError =
				"Linear-flow source Material named texture contract is not satisfied.";
			return false;
		}
		if (strProfile == "effect.ue3.blackline-aura.v1" &&
			!Has_EffectBlacklineNamedTextureContract(SourceMaterial))
		{
			strOutError =
				"Blackline source Material named texture contract is not satisfied.";
			return false;
		}
		if (strProfile == EFFECT_WATERTRAIL_RUNTIME_PROFILE_ID &&
			!Has_EffectWaterTrailNamedTextureContract(SourceMaterial))
		{
			strOutError =
				"Water-trail source Material named texture contract is not satisfied.";
			return false;
		}
		if (strProfile ==
				EFFECT_MISSILETRAIL_TWO_EMISSIVE_RUNTIME_PROFILE_ID &&
			!Has_EffectMissileTrailNamedTextureContract(SourceMaterial))
		{
			strOutError =
				"Missile-trail source Material named texture contract is not satisfied.";
			return false;
		}
		if (strProfile == "effect.ue3.local-crack.v1")
		{
			const bool_t bLegacyContract =
				Is_EffectLegacyLocalCrackResourceContractSatisfied(
					SourceMaterial, bHasDissolve, bParticleMesh);
			const bool_t bNamedContract = !SourceMaterial.Textures.empty() &&
				Is_EffectLocalCrackResourceContractSatisfied(
					Has_EffectLocalCrackNamedTextureContract(SourceMaterial),
					true, true, bParticleMesh);
			if (!bLegacyContract && !bNamedContract)
			{
				strOutError =
					"Local-crack source Material resource contract is not satisfied.";
				return false;
			}
		}
		else if ((strProfile == "effect.ue3.shine.v1" ||
				strProfile == "effect.ue3.slice.v1" ||
				strProfile == EFFECT_MISSILETRAIL_RUNTIME_PROFILE_ID ||
				strProfile ==
					EFFECT_MISSILETRAIL_TWO_EMISSIVE_RUNTIME_PROFILE_ID ||
				strProfile == EFFECT_WATERTRAIL_RUNTIME_PROFILE_ID ||
				strProfile == "effect.ue3.procedural-center-glow.v1") &&
			!Is_EffectFiniteProfileResourceContractSatisfied(
				strProfile, bSafeBase, bHasNoise, bHasMask, bHasEmissive,
				bHasDissolve,
				bParticleMesh))
		{
			strOutError =
				"Finite source Material profile resource contract is not satisfied.";
			return false;
		}
		if (strProfile == "effect.ue3.reconstructed-standard.v1" && !bSafeBase)
		{
			strOutError =
				"Reconstructed-standard source Material requires a safe Base texture.";
			return false;
		}

		bOutOwnsDrawableContract = true;
		return true;
	}


	bool_t Read_Warlord17090TypeDataMeshRotation(
		const Client::EFFECT_ELEMENT_DESC& Element,
		float3_t& vOutRotationDegrees,
		std::string& strOutError)
	{
		using namespace Client;
		const EFFECT_SOURCE_MODULE_DESC* pTypeDataMesh = nullptr;
		for (const EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (Normalize_SourceModuleClass(Module.strClassName) !=
				"particlemoduletypedatamesh")
			{
				continue;
			}
			if (nullptr != pTypeDataMesh)
			{
				strOutError =
					"Warlord 17090 Mesh source has duplicate TypeDataMesh modules.";
				return false;
			}
			pTypeDataMesh = &Module;
		}
		if (nullptr == pTypeDataMesh)
		{
			strOutError =
				"Warlord 17090 Mesh source is missing its TypeDataMesh module.";
			return false;
		}

		float3_t Projected{};
		std::array<bool_t, 3u> Seen{};
		for (const EFFECT_SOURCE_LITERAL_DESC& Literal :
			pTypeDataMesh->Literals)
		{
			std::string Property = Literal.strPropertyPath;
			std::transform(Property.begin(), Property.end(), Property.begin(),
				[](const unsigned char Character)
				{
					return static_cast<char_t>(std::tolower(Character));
				});
			size_t iComponent = 3u;
			if (Property == "roll")
				iComponent = 0u;
			else if (Property == "pitch")
				iComponent = 1u;
			else if (Property == "yaw")
				iComponent = 2u;
			if (iComponent == 3u)
				continue;
			if (Seen[iComponent])
			{
				strOutError =
					"Warlord 17090 TypeDataMesh rotation literal is duplicated.";
				return false;
			}
			if (Literal.eKind != EFFECT_SOURCE_LITERAL_KIND::NUMBER ||
				!std::isfinite(Literal.fNumber) ||
				std::abs(Literal.fNumber) > 3600.0)
			{
				strOutError =
					"Warlord 17090 TypeDataMesh rotation literal is not a finite number.";
				return false;
			}
			Seen[iComponent] = true;
			const f32_t fValue = static_cast<f32_t>(Literal.fNumber);
			if (iComponent == 0u)
				Projected.x = fValue;
			else if (iComponent == 1u)
				Projected.y = fValue;
			else
				Projected.z = fValue;
		}
		vOutRotationDegrees = Projected;
		return true;
	}


	const Client::EFFECT_SOURCE_MODULE_DESC*
		Find_UniqueWarlordChainModule(
			const Client::EFFECT_ELEMENT_DESC& Element,
			const std::string_view strClassName,
			std::string& strOutError)
	{
		const Client::EFFECT_SOURCE_MODULE_DESC* pResult = nullptr;
		for (const Client::EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (Normalize_SourceModuleClass(Module.strClassName) != strClassName)
				continue;
			if (nullptr != pResult)
			{
				strOutError = "Warlord 17090 chain source has a duplicate " +
					std::string(strClassName) + " module.";
				return nullptr;
			}
			pResult = &Module;
		}
		if (nullptr == pResult)
		{
			strOutError = "Warlord 17090 chain source is missing its " +
				std::string(strClassName) + " module.";
		}
		return pResult;
	}


	const Client::EFFECT_DISTRIBUTION_DESC* Find_WarlordChainDistribution(
		const Client::EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view strPropertyPath)
	{
		const auto Iterator = std::find_if(
			Module.Distributions.begin(), Module.Distributions.end(),
			[strPropertyPath](
				const Client::EFFECT_DISTRIBUTION_DESC& Distribution)
			{
				return Distribution.strPropertyPath == strPropertyPath;
			});
		return Iterator == Module.Distributions.end() ? nullptr : &*Iterator;
	}


	bool_t Validate_Warlord17090ChainSourceRecipe(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strMeshAssetId,
		std::string& strOutError)
	{
		using namespace Client;
		constexpr f32_t EPSILON = 1.e-5f;
		const auto NearlyEqual = [](const f32_t Left, const f32_t Right)
		{
			return std::abs(Left - Right) <= EPSILON;
		};
		constexpr std::array<std::string_view, 8u> CHAIN_06_IDS =
		{
			"authored.source-particle.fe0d3291da32d9c797705c72",
			"authored.source-particle.1bb7a50a0c3ff729ca586851",
			"authored.source-particle.eedca4cf57247af0f82fba11",
			"authored.source-particle.cd8d5f0dbedb3e620880378d",
			"authored.source-particle.a3cbc6a5ccb3876669945ebd",
			"authored.source-particle.6ad86a3bed5164050280ee47",
			"authored.source-particle.3903aa4912d4eeeba9d7d951",
			"authored.source-particle.71f0b215e9cff2426ac0ce47"
		};
		constexpr std::array<std::string_view, 4u> CHAIN_07_IDS =
		{
			"authored.source-particle.a7fae01987d89775468563e4",
			"authored.source-particle.a2d9eabb94a5702c933cd727",
			"authored.source-particle.72835f216dd26bb28166301f",
			"authored.source-particle.78efab80dc7c76fc2b416cba"
		};
		const std::string_view OriginElementId =
			Resolve_EffectPortableOriginElementId(Element);
		const auto ContainsId = [OriginElementId](const auto& Ids)
		{
			return std::find(Ids.begin(), Ids.end(), OriginElementId) !=
				Ids.end();
		};
		if ((strMeshAssetId == WARLORD_CHAIN_06_MODEL_ASSET_ID &&
				!ContainsId(CHAIN_06_IDS)) ||
			(strMeshAssetId == WARLORD_CHAIN_07_MODEL_ASSET_ID &&
				!ContainsId(CHAIN_07_IDS)))
		{
			strOutError =
				"Warlord 17090 chain stable Element/Mesh identity is invalid.";
			return false;
		}
		if (Element.SourceRecipe.iEmitterLoopCount != 1u ||
			Element.SourceRecipe.Bursts.size() != 1u ||
			!NearlyEqual(Element.SourceRecipe.Bursts.front().fTimeSeconds, 0.f) ||
			Element.SourceRecipe.Bursts.front().iCountMinimum != 1u ||
			Element.SourceRecipe.Bursts.front().iCountMaximum != 1u ||
			!NearlyEqual(Element.Detail.Timing.fStartDelaySeconds, 0.3668f))
		{
			strOutError =
				"Warlord 17090 chain burst/timing contract is invalid.";
			return false;
		}

		const EFFECT_SOURCE_MODULE_DESC* pLocationDirect =
			Find_UniqueWarlordChainModule(Element,
				"particlemodulelocationdirect", strOutError);
		const EFFECT_SOURCE_MODULE_DESC* pLifetime =
			Find_UniqueWarlordChainModule(Element,
				"particlemodulelifetime", strOutError);
		const EFFECT_SOURCE_MODULE_DESC* pDynamic =
			Find_UniqueWarlordChainModule(Element,
				"particlemoduleparameterdynamic", strOutError);
		const EFFECT_SOURCE_MODULE_DESC* pMeshRotation =
			Find_UniqueWarlordChainModule(Element,
				"particlemodulemeshrotation", strOutError);
		if (nullptr == pLocationDirect || nullptr == pLifetime ||
			nullptr == pDynamic || nullptr == pMeshRotation)
		{
			return false;
		}

		const EFFECT_DISTRIBUTION_DESC* pLocation =
			Find_WarlordChainDistribution(*pLocationDirect, "location");
		const EFFECT_DISTRIBUTION_DESC* pDirection =
			Find_WarlordChainDistribution(*pLocationDirect, "direction");
		const EFFECT_DISTRIBUTION_DESC* pLocationOffset =
			Find_WarlordChainDistribution(*pLocationDirect, "locationoffset");
		const EFFECT_DISTRIBUTION_DESC* pScaleFactor =
			Find_WarlordChainDistribution(*pLocationDirect, "scalefactor");
		const auto IsExactZeroVectorDefault = [](const auto* pDistribution)
		{
			return nullptr != pDistribution &&
				pDistribution->strSourceClass.empty() &&
				pDistribution->strSourceObjectPath.empty() &&
				pDistribution->iComponentCount == 3u &&
				pDistribution->iOperation == 1u &&
				pDistribution->iRandomLockAxes == 0u &&
				pDistribution->iLookupTableChunkSize == 0u &&
				pDistribution->iLookupTableNumElements == 0u &&
				pDistribution->fLookupTableTimeScale == 0.f &&
				pDistribution->fLookupTableStartTime == 0.f &&
				pDistribution->vDefaultMinimum.x == 0.f &&
				pDistribution->vDefaultMinimum.y == 0.f &&
				pDistribution->vDefaultMinimum.z == 0.f &&
				pDistribution->vDefaultMinimum.w == 0.f &&
				pDistribution->vDefaultMaximum.x == 0.f &&
				pDistribution->vDefaultMaximum.y == 0.f &&
				pDistribution->vDefaultMaximum.z == 0.f &&
				pDistribution->vDefaultMaximum.w == 0.f &&
				pDistribution->LookupTable.empty() &&
				pDistribution->Keys.empty();
		};
		constexpr std::array<f32_t, 8u> LOCATION_TABLE =
			{ -30.f, 35.f, 0.f, 35.f, 0.f, -20.f, -30.f, 0.f };
		if (!IsExactZeroVectorDefault(pDirection) ||
			!IsExactZeroVectorDefault(pLocationOffset) ||
			!IsExactZeroVectorDefault(pScaleFactor) ||
			nullptr == pLocation || pLocation->iComponentCount != 3u ||
			pLocation->iOperation != 1u ||
			pLocation->iLookupTableChunkSize != 3u ||
			pLocation->iLookupTableNumElements != 1u ||
			!NearlyEqual(pLocation->fLookupTableStartTime, 0.9f) ||
			!NearlyEqual(pLocation->fLookupTableTimeScale, 10.f) ||
			pLocation->LookupTable.size() != LOCATION_TABLE.size() ||
			!std::equal(pLocation->LookupTable.begin(),
				pLocation->LookupTable.end(), LOCATION_TABLE.begin(),
				[&NearlyEqual](const f32_t Left, const f32_t Right)
				{
					return NearlyEqual(Left, Right);
				}))
		{
			strOutError =
				"Warlord 17090 LocationDirect launch/return/default identity is invalid.";
			return false;
		}

		const EFFECT_DISTRIBUTION_DESC* pLife =
			Find_WarlordChainDistribution(*pLifetime, "lifetime");
		if (nullptr == pLife || pLife->LookupTable.size() != 4u ||
			!NearlyEqual(pLife->LookupTable[2], 0.6f) ||
			!NearlyEqual(pLife->LookupTable[3], 0.6f))
		{
			strOutError = "Warlord 17090 chain lifetime is not exact 0.6s.";
			return false;
		}

		constexpr std::array<std::string_view, 4u> DYNAMIC_PARAMETER_NAMES =
		{
			"worldpositionoffset_str", "worldposition_uvscale",
			"x.pan", "worldposition_zoffset"
		};
		for (size_t i = 0u; i < DYNAMIC_PARAMETER_NAMES.size(); ++i)
		{
			const std::string Property = "dynamicparams[" +
				std::to_string(i) + "].paramname";
			const auto Literal = std::find_if(
				pDynamic->Literals.begin(), pDynamic->Literals.end(),
				[&Property](const EFFECT_SOURCE_LITERAL_DESC& Candidate)
				{
					return Candidate.strPropertyPath == Property;
				});
			if (Literal == pDynamic->Literals.end() ||
				Literal->eKind != EFFECT_SOURCE_LITERAL_KIND::STRING ||
				Literal->strString != DYNAMIC_PARAMETER_NAMES[i])
			{
				strOutError =
					"Warlord 17090 chain WPO DynamicParameter identity is invalid.";
				return false;
			}
		}
		return true;
	}


	bool_t Apply_Warlord17090RetainedSourceProjection(
		Client::EFFECT_DOCUMENT_DESC& InOutDocument,
		std::string& strOutError)
	{
		using namespace Client;
		if (InOutDocument.strEffectAssetId != WARLORD_17090_EFFECT_ASSET_ID)
			return true;

		size_t iChainCount = 0u;
		size_t iChain06Count = 0u;
		size_t iChain07Count = 0u;
		const auto ResolveCompilerAssetId = [](
			const EFFECT_ELEMENT_DESC& Element,
			const EFFECT_RESOURCE_BINDING_DESC& Binding,
			std::string_view& strOutCompilerAssetId) -> bool_t
		{
			const auto First = std::find_if(
				Element.AuthoringOverrides.ResourceBindings.begin(),
				Element.AuthoringOverrides.ResourceBindings.end(),
				[&Binding](const EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Override)
				{
					return Override.strSlotId == Binding.strSlotId;
				});
			if (First == Element.AuthoringOverrides.ResourceBindings.end())
			{
				strOutCompilerAssetId = Binding.strAssetId;
				return true;
			}
			if (std::find_if(std::next(First),
					Element.AuthoringOverrides.ResourceBindings.end(),
					[&Binding](
						const EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Override)
					{
						return Override.strSlotId == Binding.strSlotId;
					}) != Element.AuthoringOverrides.ResourceBindings.end() ||
				First->strAssetId != Binding.strAssetId ||
				First->strCompilerAssetId.empty())
			{
				return false;
			}
			strOutCompilerAssetId = First->strCompilerAssetId;
			return true;
		};
		for (EFFECT_ELEMENT_DESC& Element : InOutDocument.Elements)
		{
			if (!Element.SourceRecipe.bEnabled ||
				Element.SourceRecipe.strRendererShape != "mesh")
			{
				continue;
			}
			float3_t SourceTypeDataRotation{};
			if (!Read_Warlord17090TypeDataMeshRotation(
				Element, SourceTypeDataRotation, strOutError))
			{
				return false;
			}
			Element.Detail.Mesh.vSourceTypeDataRotationDegrees =
				SourceTypeDataRotation;

			if (Element.Material.strSourceMaterialPath !=
				WARLORD_CHAIN_SOURCE_MATERIAL_PATH)
			{
				continue;
			}
			++iChainCount;
			if (Element.eKind != EFFECT_ELEMENT_KIND::PARTICLE)
			{
				strOutError =
					"Warlord 17090 chain source is not a Particle Mesh.";
				return false;
			}

			size_t iMeshBindingCount = 0u;
			size_t iPreviewBaseCount = 0u;
			std::string_view strMeshAssetId;
			for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
				Element.ResourceBindings)
			{
				if (Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID)
				{
					++iMeshBindingCount;
					if (!ResolveCompilerAssetId(
							Element, Binding, strMeshAssetId))
					{
						strOutError =
							"Warlord 17090 chain Mesh override baseline/effective identity is invalid.";
						return false;
					}
					if (strMeshAssetId == WARLORD_CHAIN_06_MODEL_ASSET_ID)
						++iChain06Count;
					else if (strMeshAssetId ==
						WARLORD_CHAIN_07_MODEL_ASSET_ID)
					{
						++iChain07Count;
					}
					else
					{
						strOutError =
							"Warlord 17090 chain Mesh identity is invalid.";
						return false;
					}
				}
				else if (Binding.strSlotId == "base")
				{
					std::string_view strCompilerBaseAssetId;
					if (!ResolveCompilerAssetId(Element, Binding,
							strCompilerBaseAssetId) ||
						strCompilerBaseAssetId !=
							WARLORD_CHAIN_BASE_ALIAS_ASSET_ID)
					{
						strOutError =
							"Warlord 17090 non-exact preview Base override baseline/effective identity is invalid.";
						return false;
					}
					++iPreviewBaseCount;
				}
				else
				{
					strOutError =
						"Warlord 17090 chain carries an unproven Material resource.";
					return false;
				}
			}
			if (iMeshBindingCount != 1u || iPreviewBaseCount != 1u)
			{
				strOutError =
					"Warlord 17090 chain Mesh/non-exact preview Base cardinality is invalid.";
				return false;
			}
			const EFFECT_SOURCE_MATERIAL_DESC& SourceProfile =
				Element.Material.SourceMaterial;
			if (Element.Material.strTemplateId !=
					EFFECT_SOURCE_MATERIAL_TEMPLATE_ID ||
				!SourceProfile.bEnabled ||
				SourceProfile.strProfileId != WARLORD_CHAIN_SOURCE_PROFILE_ID ||
				SourceProfile.strParentMaterialPath !=
					WARLORD_CHAIN_PARENT_MATERIAL_PATH ||
				SourceProfile.strRuntimeShaderProfileId !=
					WARLORD_CHAIN_RUNTIME_PROFILE_ID)
			{
				strOutError =
					"Warlord 17090 non-exact preview source profile is invalid.";
				return false;
			}
			if (!Validate_Warlord17090ChainSourceRecipe(
				Element, strMeshAssetId, strOutError))
			{
				return false;
			}
			Element.Material.Execution = {};
			Element.Material.Execution.bFailClosed = true;
			Element.Material.Execution.bAuthoringApproximate = true;
			/* atypical_028 is a same-group authoring baseline, not an exact parent
			   Base.  Keep the enabled grouped profile and any valid G3 override so
			   the Tool can tune/reset a physical DDS.  The disabled approximate
			   execution records SOURCE_MASKED_WPO_ARITHMETIC_UNAVAILABLE and must
			   never promote this carrier to Full merely because it draws. */
			Element.Detail.Mesh.bUseModelMaterial = false;
		}
		/* Mutable authored documents are legal source-backed subsets.  Exact
		   source cardinality belongs to the immutable Track-A evidence gate,
		   never to ordinary Tool Load/Save.  Every retained chain row has already
		   passed the stable-ID/model/material/recipe checks above; these upper
		   bounds only prevent a future codec caller from exceeding that evidence
		   denominator. */
		if (iChainCount != iChain06Count + iChain07Count ||
			iChain06Count > 8u || iChain07Count > 4u)
		{
			strOutError =
				"Warlord 17090 retained Chain subset exceeds the source evidence denominator.";
			return false;
		}
		return true;
	}

}


bool_t Client::CEffectDocumentCodec::Validate_Drawable(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	if (!Validate(Document, strOutError))
		return false;
	const bool_t bHasVisibleElement = std::any_of(
		Document.Elements.begin(), Document.Elements.end(),
		[](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.bVisible && !Is_EffectSimulationOnlyParticle(Element) &&
				(Is_EffectElementAuthoringExecutionTarget(Element) ||
				 Is_EffectPresentationExecutionTarget(Element));
		});
	const bool_t bHasVisibleModelCue = std::any_of(
		Document.ModelCues.begin(), Document.ModelCues.end(),
		[](const EFFECT_MODEL_CUE_DESC& Cue)
		{
			return Cue.bVisible;
		});
	if (!bHasVisibleElement && !bHasVisibleModelCue)
	{
		strOutError =
			"Effect has no visible Element or Model / Summon to preview.";
		return false;
	}
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (!Element.bVisible || Is_EffectSimulationOnlyParticle(Element) ||
			(!Is_EffectElementAuthoringExecutionTarget(Element) &&
			 !Is_EffectPresentationExecutionTarget(Element)))
			continue;
		if (EFFECT_ELEMENT_KIND::LIGHT == Element.eKind ||
			EFFECT_ELEMENT_KIND::SCREEN_POST == Element.eKind)
		{
			continue;
		}
		if (!Element.Detail.Mesh.SourceMaterialSlots.empty())
			continue;
		const EFFECT_MATERIAL_TEMPLATE_DESC* pTemplate =
			Find_EffectMaterialTemplate(Element.Material.strTemplateId);
		const EFFECT_MATERIAL_INPUT_SLOT_DESC* pBaseInput =
			nullptr == pTemplate ? nullptr : Find_EffectMaterialInput(
				*pTemplate, EFFECT_MATERIAL_INPUT_SEMANTIC::BASE);
		const auto FindBinding = [&](const EFFECT_MATERIAL_INPUT_SEMANTIC eSemantic)
			-> const EFFECT_RESOURCE_BINDING_DESC*
		{
			if (nullptr == pTemplate)
				return nullptr;
			const EFFECT_MATERIAL_INPUT_SLOT_DESC* pInput =
				Find_EffectMaterialInput(*pTemplate, eSemantic);
			if (nullptr == pInput)
				return nullptr;
			const auto Iterator = std::find_if(
				Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
				[pInput](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == pInput->strSlotId;
				});
			return Iterator == Element.ResourceBindings.end() ? nullptr : &*Iterator;
		};
		const EFFECT_RESOURCE_BINDING_DESC* pBaseBinding = FindBinding(
			EFFECT_MATERIAL_INPUT_SEMANTIC::BASE);
		bool_t bSourceMaterialOwnsDrawableContract = false;
		if (!Validate_ExecutableSourceMaterialCarrier(
				Element, bSourceMaterialOwnsDrawableContract, strOutError))
		{
			return false;
		}
		const bool_t bMaterialOwnsDrawableContract =
			Element.Material.Execution.bEnabled ||
			bSourceMaterialOwnsDrawableContract;
		const bool_t bParticleMesh =
			EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind &&
			std::any_of(Element.ResourceBindings.begin(),
				Element.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
				});
		const std::string_view strRequiredSlotId =
			EFFECT_ELEMENT_KIND::MESH == Element.eKind || bParticleMesh ?
			EFFECT_MESH_SHAPE_SLOT_ID :
			(nullptr == pBaseInput ? std::string_view{} :
				pBaseInput->strSlotId);
		const bool_t bBound = std::any_of(
			Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
			[strRequiredSlotId](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == strRequiredSlotId;
			});
		const bool_t bMeshElement =
			EFFECT_ELEMENT_KIND::MESH == Element.eKind || bParticleMesh;
		if ((strRequiredSlotId.empty() || !bBound) &&
			!(!bMeshElement && bMaterialOwnsDrawableContract))
		{
			strOutError = bMeshElement ?
				"Mesh or mesh-backed Particle requires a Mesh Model binding." :
				"Sprite/Particle/Decal/Trail Element requires a Base texture binding.";
			return false;
		}
		if (bMeshElement &&
			!Element.Detail.Mesh.bUseModelMaterial)
		{
			if (nullptr == pBaseBinding && !bMaterialOwnsDrawableContract)
			{
				strOutError = "Mesh Element with useModelMaterial=false requires a Base texture binding.";
				return false;
			}
		}
	}
	strOutError.clear();
	return true;
}
