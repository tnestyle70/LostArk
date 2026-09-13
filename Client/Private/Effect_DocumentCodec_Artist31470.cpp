#include "Effect_DocumentCodec_Internal.h"
#include "Effect_Artist31470ShaderRegistry.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_RuntimeAuthority.h"

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


	enum class ARTIST31470_UNIFIED_FAMILY : uint8_t
	{
		MESH,
		SPRITE,
		DECAL,
		RIBBON,
		END
	};


	bool_t Try_ResolveArtist31470UnifiedFamily(
		const EFFECT_RUNTIME_RENDERER_KIND eRenderer,
		ARTIST31470_UNIFIED_FAMILY& eOutFamily)
	{
		switch (eRenderer)
		{
		case EFFECT_RUNTIME_RENDERER_KIND::MESH_PARTICLE:
			eOutFamily = ARTIST31470_UNIFIED_FAMILY::MESH;
			return true;
		case EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE:
			eOutFamily = ARTIST31470_UNIFIED_FAMILY::SPRITE;
			return true;
		case EFFECT_RUNTIME_RENDERER_KIND::DECAL_PARTICLE:
			eOutFamily = ARTIST31470_UNIFIED_FAMILY::DECAL;
			return true;
		case EFFECT_RUNTIME_RENDERER_KIND::CASCADE_RIBBON:
			eOutFamily = ARTIST31470_UNIFIED_FAMILY::RIBBON;
			return true;
		default:
			eOutFamily = ARTIST31470_UNIFIED_FAMILY::END;
			return false;
		}
	}


	const char_t* Artist31470UnifiedFamilyLabel(
		const ARTIST31470_UNIFIED_FAMILY eFamily)
	{
		switch (eFamily)
		{
		case ARTIST31470_UNIFIED_FAMILY::MESH: return "MeshParticle";
		case ARTIST31470_UNIFIED_FAMILY::SPRITE: return "SpriteParticle";
		case ARTIST31470_UNIFIED_FAMILY::DECAL: return "LocalDecal";
		case ARTIST31470_UNIFIED_FAMILY::RIBBON: return "CascadeRibbon";
		default: return "Invalid";
		}
	}


	std::string Artist31470UnifiedStableElementId(
		const ARTIST31470_UNIFIED_FAMILY eFamily,
		const std::string_view strSourceIdentity)
	{
		const std::string Digest =
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				std::string(Artist31470UnifiedFamilyLabel(eFamily)) + "\n" +
				std::string(strSourceIdentity));
		const char_t* pPrefix = nullptr;
		switch (eFamily)
		{
		case ARTIST31470_UNIFIED_FAMILY::MESH: pPrefix = "mesh"; break;
		case ARTIST31470_UNIFIED_FAMILY::SPRITE: pPrefix = "sprite"; break;
		case ARTIST31470_UNIFIED_FAMILY::DECAL: pPrefix = "decal"; break;
		case ARTIST31470_UNIFIED_FAMILY::RIBBON: pPrefix = "ribbon"; break;
		default: return {};
		}
		return std::string(pPrefix) + "." + Digest.substr(0u, 16u);
	}


	bool_t Artist31470UnifiedElementMatchesFamily(
		const EFFECT_ELEMENT_DESC& Element,
		const ARTIST31470_UNIFIED_FAMILY eFamily)
	{
		const bool_t bHasMeshShape = std::any_of(
			Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
			[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
			});
		switch (eFamily)
		{
		case ARTIST31470_UNIFIED_FAMILY::MESH:
			return Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE && bHasMeshShape;
		case ARTIST31470_UNIFIED_FAMILY::SPRITE:
			return Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE && !bHasMeshShape;
		case ARTIST31470_UNIFIED_FAMILY::DECAL:
			return Element.eKind == EFFECT_ELEMENT_KIND::DECAL;
		case ARTIST31470_UNIFIED_FAMILY::RIBBON:
			return Element.eKind == EFFECT_ELEMENT_KIND::TRAIL;
		default:
			return false;
		}
	}


	bool_t Try_ResolveArtist31470FixedBurstCount(
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
		uint32_t& iOutCount,
		std::string& strOutError)
	{
		uint64_t iCount = 0u;
		for (const EFFECT_RUNTIME_PROGRAM_BURST& Burst : Emitter.Timing.Bursts)
		{
			if (!std::isfinite(Burst.fTimeSeconds) ||
				std::abs(Burst.fTimeSeconds) > 1.0e-9 ||
				Burst.iCountMinimum != Burst.iCountMaximum)
			{
				strOutError =
					"Artist F Track A burst is not a fixed t=0 authored burst.";
				return false;
			}
			iCount += Burst.iCountMaximum;
		}
		if (iCount > (std::numeric_limits<uint32_t>::max)())
		{
			strOutError = "Artist F Track A fixed burst count overflowed uint32.";
			return false;
		}
		iOutCount = static_cast<uint32_t>(iCount);
		return true;
	}


	std::string NormalizeArtist31470MaterialRole(const std::string_view Value)
	{
		std::string Result;
		Result.reserve(Value.size());
		bool_t bLastSeparator = false;
		for (const unsigned char Character : Value)
		{
			if (0 != std::isalnum(Character))
			{
				Result.push_back(static_cast<char_t>(std::tolower(Character)));
				bLastSeparator = false;
			}
			else if (!Result.empty() && !bLastSeparator)
			{
				Result.push_back('_');
				bLastSeparator = true;
			}
		}
		while (!Result.empty() && Result.back() == '_')
			Result.pop_back();
		return Result;
	}


	int32_t ScoreArtist31470GenericMaterialLane(
		const std::string_view strSlotId,
		const ARTIST31470_UNIFIED_FAMILY eFamily,
		const std::string_view strRole)
	{
		const std::string Role = NormalizeArtist31470MaterialRole(strRole);
		const auto Has = [&Role](const std::string_view Token)
		{
			return std::string::npos != Role.find(Token);
		};
		if (strSlotId == "base")
		{
			if (eFamily == ARTIST31470_UNIFIED_FAMILY::DECAL &&
				Role == "diffuse")
			{
				return 120;
			}
			if (Role == "base") return 110;
			if (Has("diffuse") || Has("albedo")) return 100;
			if (Has("alpha_tex_01") || Has("main_tex")) return 60;
		}
		else if (strSlotId == "noise")
		{
			if (Role == "noise") return 110;
			if (Has("noise")) return 100;
		}
		else if (strSlotId == "mask")
		{
			if (Role == "mask") return 110;
			if (Has("mask")) return 100;
		}
		else if (strSlotId == "emissive")
		{
			if (Role == "emissive") return 110;
			if (Has("emissive") || Has("emap")) return 100;
		}
		else if (strSlotId == "dissolve")
		{
			if (Role == "dissolve") return 110;
			if (Has("dissolve")) return 100;
		}
		return 0;
	}


	std::optional<size_t> ResolveArtist31470GenericMaterialLane(
		const EFFECT_RESOURCE_BINDING_DESC* pSourceBinding,
		const std::string_view strSlotId,
		const ARTIST31470_UNIFIED_FAMILY eFamily,
		const EFFECT_MATERIAL_EXECUTION_DESC& Execution)
	{
		/* The generic Decal editor exposes its color texture as Base, while the
		   Track A six-SRV packet names that same authoring intent DIFFUSE.  This
		   semantic bridge is stronger than a source-asset match because Base may
		   already contain an artist-selected DDS. */
		if (eFamily == ARTIST31470_UNIFIED_FAMILY::DECAL &&
			strSlotId == "base")
		{
			std::optional<size_t> DiffuseLane;
			for (size_t iLane = 0u; iLane < Execution.TextureLanes.size(); ++iLane)
			{
				if (NormalizeArtist31470MaterialRole(
						Execution.TextureLanes[iLane].strRole) != "diffuse")
				{
					continue;
				}
				if (DiffuseLane.has_value())
					return std::nullopt;
				DiffuseLane = iLane;
			}
			if (DiffuseLane.has_value())
				return DiffuseLane;
		}

		std::vector<size_t> SourceAssetMatches;
		if (nullptr != pSourceBinding && !pSourceBinding->strAssetId.empty())
		{
			for (size_t iLane = 0u; iLane < Execution.TextureLanes.size(); ++iLane)
			{
				if (Execution.TextureLanes[iLane].strAssetId ==
					pSourceBinding->strAssetId)
				{
					SourceAssetMatches.push_back(iLane);
				}
			}
			if (SourceAssetMatches.size() == 1u)
				return SourceAssetMatches.front();
		}

		const bool_t bRestrictToSourceAsset = SourceAssetMatches.size() > 1u;
		std::optional<size_t> BestLane;
		int32_t iBestScore = 0;
		bool_t bTied = false;
		for (size_t iLane = 0u; iLane < Execution.TextureLanes.size(); ++iLane)
		{
			if (bRestrictToSourceAsset &&
				std::find(SourceAssetMatches.begin(), SourceAssetMatches.end(),
					iLane) == SourceAssetMatches.end())
			{
				continue;
			}
			const int32_t iScore = ScoreArtist31470GenericMaterialLane(
				strSlotId, eFamily, Execution.TextureLanes[iLane].strRole);
			if (iScore > iBestScore)
			{
				iBestScore = iScore;
				BestLane = iLane;
				bTied = false;
			}
			else if (iScore > 0 && iScore == iBestScore)
			{
				bTied = true;
			}
		}
		return iBestScore > 0 && !bTied ? BestLane : std::nullopt;
	}


	bool_t PromoteArtist31470GenericMaterialOverrides(
		const EFFECT_ELEMENT_DESC& SourceElement,
		const ARTIST31470_UNIFIED_FAMILY eFamily,
		const std::set<std::string, std::less<>>& ExistingTypedLaneIds,
		const EFFECT_ELEMENT_DESC& ExistingElement,
		EFFECT_MATERIAL_EXECUTION_DESC& InOutExecution,
		std::string& strOutError)
	{
		for (const EFFECT_MATERIAL_INPUT_SLOT_DESC& Input :
			EFFECT_STANDARD_MATERIAL_INPUTS)
		{
			const auto ExistingBinding = std::find_if(
				ExistingElement.ResourceBindings.begin(),
				ExistingElement.ResourceBindings.end(),
				[&Input](const EFFECT_RESOURCE_BINDING_DESC& Candidate)
				{
					return Candidate.strSlotId == Input.strSlotId;
				});
			if (ExistingBinding == ExistingElement.ResourceBindings.end() ||
				ExistingBinding->strAssetId.empty())
			{
				continue;
			}
			EFFECT_RESOURCE_FILE_KIND FileKind = EFFECT_RESOURCE_FILE_KIND::END;
			if (!CEffectDocumentCodec::Is_SafeResourceAssetId(
					ExistingBinding->strAssetId, &FileKind) ||
				FileKind != EFFECT_RESOURCE_FILE_KIND::TEXTURE)
			{
				strOutError = "Artist F generic material override is not a safe DDS: " +
					ExistingBinding->strSlotId + ".";
				return false;
			}

			const auto SourceBinding = std::find_if(
				SourceElement.ResourceBindings.begin(),
				SourceElement.ResourceBindings.end(),
				[&Input](const EFFECT_RESOURCE_BINDING_DESC& Candidate)
				{
					return Candidate.strSlotId == Input.strSlotId;
				});
			const EFFECT_RESOURCE_BINDING_DESC* pSourceBinding =
				SourceBinding == SourceElement.ResourceBindings.end() ?
				nullptr : &*SourceBinding;
			const std::optional<size_t> LaneIndex =
				ResolveArtist31470GenericMaterialLane(
					pSourceBinding, Input.strSlotId, eFamily, InOutExecution);
			if (!LaneIndex.has_value())
			{
				if (nullptr != pSourceBinding &&
					pSourceBinding->strAssetId != ExistingBinding->strAssetId)
				{
					strOutError =
						"Artist F generic DDS override has no unambiguous typed lane: " +
						ExistingElement.strElementId + "/" +
						ExistingBinding->strSlotId + ".";
					return false;
				}
				continue;
			}
			EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane =
				InOutExecution.TextureLanes[*LaneIndex];
			if (ExistingTypedLaneIds.contains(Lane.strLaneId))
				continue;
			Lane.strAssetId = ExistingBinding->strAssetId;
		}
		return true;
	}


	bool_t Artist31470CarrierNearlyEqual(
		const f32_t Left, const f32_t Right)
	{
		return std::abs(Left - Right) <= 1.0e-5f *
			(std::max)({ 1.f, std::abs(Left), std::abs(Right) });
	}


	void NormalizeArtist31470LegacyGeneratedParticleCarrier(
		const uint32_t iOrder,
		EFFECT_ELEMENT_DESC& InOutElement)
	{
		if (EFFECT_ELEMENT_KIND::PARTICLE != InOutElement.eKind)
			return;
		EFFECT_DETAIL_DESC& Detail = InOutElement.Detail;
		float4_t& Multiply = Detail.Color.vColorMultiply;
		if ((iOrder == 2u || iOrder == 19u || iOrder == 31u) &&
			Artist31470CarrierNearlyEqual(Multiply.x, 1.f) &&
			Artist31470CarrierNearlyEqual(Multiply.y, 1.f) &&
			Artist31470CarrierNearlyEqual(Multiply.z, 1.f) &&
			Artist31470CarrierNearlyEqual(Multiply.w, 50.f) &&
			!Detail.LinearLerp.bColorMultiply)
		{
			/* This alpha-50 value was a bounded generic fallback for op6.
			   SourceRecipe now supplies the exact HDR color/alpha curve. */
			Multiply.w = 1.f;
		}
		if (iOrder == 23u &&
			Artist31470CarrierNearlyEqual(Multiply.x, 0.5f) &&
			Artist31470CarrierNearlyEqual(Multiply.y, 0.7f) &&
			Artist31470CarrierNearlyEqual(Multiply.z, 0.5f) &&
			Artist31470CarrierNearlyEqual(Multiply.w, 0.3f) &&
			Detail.LinearLerp.bColorMultiply &&
			Artist31470CarrierNearlyEqual(
				Detail.LinearLerp.vEndColorMultiply.x, 0.5f) &&
			Artist31470CarrierNearlyEqual(
				Detail.LinearLerp.vEndColorMultiply.y, 0.7f) &&
			Artist31470CarrierNearlyEqual(
				Detail.LinearLerp.vEndColorMultiply.z, 0.5f) &&
			Artist31470CarrierNearlyEqual(
				Detail.LinearLerp.vEndColorMultiply.w, 0.f))
		{
			/* Same migration-only fallback: preserve a non-matching value as a
			   user-authored tint, but remove the exact generated green envelope. */
			Multiply = { 1.f, 1.f, 1.f, 1.f };
			Detail.LinearLerp.bColorMultiply = false;
			Detail.LinearLerp.vEndColorMultiply = { 1.f, 1.f, 1.f, 1.f };
		}
	}


	bool_t Try_ResolveArtistVisualV4ParticleColorAbi(
		const uint32_t iOpcode,
		uint32_t& iOutPolicy,
		uint32_t& iOutConsumedMask)
	{
		switch (iOpcode)
		{
		case 1u: /* BasicMissileTrail */
		case 2u: /* MakeFlow */
		case 3u: /* ComplexMissileTrail */
		case 6u: /* SPLA */
		case 7u: /* Flow02 recovered equation */
		case 8u: /* Skull recovered equation */
			iOutPolicy = 2u;
			iOutConsumedMask = 0x0fu;
			return true;
		case 4u: /* DistortionOnly consumes alpha coverage only. */
			iOutPolicy = 1u;
			iOutConsumedMask = 0x08u;
			return true;
		case 5u: /* Explicit zero-draw suppression. */
			iOutPolicy = 0u;
			iOutConsumedMask = 0u;
			return true;
		default:
			iOutPolicy = 0u;
			iOutConsumedMask = 0u;
			return false;
		}
	}


	bool_t ApplyArtist31470TrackAElementData(
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
		const EFFECT_ELEMENT_DESC& SourceElement,
		const std::unordered_map<std::string,
			EFFECT_MATERIAL_EXECUTION_DESC>& MaterialSnapshots,
		EFFECT_ELEMENT_DESC& InOutElement,
		std::string& strOutError)
	{
		ARTIST31470_UNIFIED_FAMILY eFamily =
			ARTIST31470_UNIFIED_FAMILY::END;
		if (!Emitter.bVisible ||
			!Try_ResolveArtist31470UnifiedFamily(Emitter.eRenderer, eFamily) ||
			!Artist31470UnifiedElementMatchesFamily(InOutElement, eFamily) ||
			SourceElement.strElementId != Emitter.strSourceElementId)
		{
			strOutError =
				"Artist F Track A seed no longer matches its authored Family/Element identity.";
			return false;
		}
		if (!Emitter.strMaterialOccurrenceId.has_value())
		{
			strOutError = "Artist F Track A seed has no material occurrence ID.";
			return false;
		}
		const auto Registry = Find_Artist31470ShaderRegistry(
			Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId);
		if (!Registry.has_value() ||
			!Validate_Artist31470ShaderRegistryEmitterIdentity(
				Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId,
				Emitter.strSourceElementId, Emitter.strSourceEmitterPath))
		{
			strOutError =
				"Artist F Track A seed no longer matches the shader registry.";
			return false;
		}

		if (EFFECT_ELEMENT_KIND::PARTICLE == InOutElement.eKind)
		{
			uint32_t iFixedBurstCount = 0u;
			if (!Try_ResolveArtist31470FixedBurstCount(
					Emitter, iFixedBurstCount, strOutError))
			{
				return false;
			}
			InOutElement.Detail.Particle.iBurstCount = iFixedBurstCount;
			InOutElement.Detail.Particle.bLocalSpace = Emitter.bLocalSpace;
			InOutElement.Detail.Particle.iRandomSeed =
				Emitter.Random.iEmitterRandomSeed;
			InOutElement.Detail.Particle.iMaxParticles = (std::max)(
				InOutElement.Detail.Particle.iMaxParticles,
				Emitter.iOperationalMaxParticles);
		}
		if (ARTIST31470_UNIFIED_FAMILY::MESH == eFamily)
		{
			const EFFECT_SOURCE_GEOMETRY_BINDING_DESC& Geometry =
				SourceElement.SourceRecipe.GeometryBinding;
			const auto ModelBinding = std::find_if(
				InOutElement.ResourceBindings.begin(),
				InOutElement.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
				});
			constexpr f32_t MODEL_PRE_SCALE = 0.01f;
			EFFECT_RESOURCE_FILE_KIND FileKind = EFFECT_RESOURCE_FILE_KIND::END;
			if (Emitter.strSizeUnitPolicy != "DIMENSIONLESS_AXIS_REORDER_ONLY" ||
				ModelBinding == InOutElement.ResourceBindings.end() ||
				!CEffectDocumentCodec::Is_SafeElementResourceAssetId(
					InOutElement.eKind, EFFECT_MESH_SHAPE_SLOT_ID,
					ModelBinding->strAssetId, &FileKind) ||
				FileKind != EFFECT_RESOURCE_FILE_KIND::MODEL ||
				(Geometry.bEnabled &&
					(Geometry.strParticleScaleSemantics !=
						Emitter.strSizeUnitPolicy ||
					 !std::isfinite(Geometry.fCarrierGeometryPreScale) ||
					 std::abs(Geometry.fCarrierGeometryPreScale - MODEL_PRE_SCALE) >
						 1.0e-7f)))
			{
				strOutError =
					"Artist F MeshParticle lost its WModel geometry pre-scale contract: " +
					Emitter.strSourceElementId + ", source=" +
					Geometry.strAssetId + ", authored=" +
					(ModelBinding == InOutElement.ResourceBindings.end() ?
						std::string("<missing>") : ModelBinding->strAssetId) + ".";
				return false;
			}
			/* The native-v14 generic projection stored Mesh particle sizes in the
			   carrier's 0.01 geometry unit.  CModel already applies that pre-scale,
			   while ordinary particle playback consumes StartSize/EndSize as a
			   dimensionless instance scale.  Restore the source dimensionless value
			   from the immutable source document exactly once.  Assigning from the
			   source keeps repeated Upgrade operations idempotent and leaves authored
			   Transform and resource overrides untouched. */
			/* Some admitted source projections omit their geometry receipt while
			   the authored Element retains the verified WModel binding.  Artist F's
			   carrier contract is still the pinned 0.01 conversion in that case. */
			const f32_t fDimensionlessScale = 1.f / MODEL_PRE_SCALE;
			const float2_t vDimensionlessStart = {
				SourceElement.Detail.Particle.vStartSize.x * fDimensionlessScale,
				SourceElement.Detail.Particle.vStartSize.y * fDimensionlessScale };
			const float2_t vDimensionlessEnd = {
				SourceElement.Detail.Particle.vEndSize.x * fDimensionlessScale,
				SourceElement.Detail.Particle.vEndSize.y * fDimensionlessScale };
			if (!Is_Finite(vDimensionlessStart) ||
				vDimensionlessStart.x <= 0.f || vDimensionlessStart.y <= 0.f ||
				!Is_Finite(vDimensionlessEnd) ||
				vDimensionlessEnd.x < 0.f || vDimensionlessEnd.y < 0.f)
			{
				strOutError =
					"Artist F MeshParticle source dimensionless size is invalid: " +
					Emitter.strSourceElementId + ".";
				return false;
			}
			InOutElement.Detail.Particle.vStartSize = vDimensionlessStart;
			InOutElement.Detail.Particle.vEndSize = vDimensionlessEnd;
			InOutElement.Detail.Mesh.fModelPreScale = MODEL_PRE_SCALE;
		}

		/* The existing unified document already owns the root/follow basis in its
		   authored Transform.  Re-enabling attachment would apply that basis twice. */
		InOutElement.ActionCueAttachment = {};
		InOutElement.TransformInheritance = {};
		const auto MaterialSnapshot = MaterialSnapshots.find(
			SourceElement.strElementId);
		if (MaterialSnapshot == MaterialSnapshots.end())
		{
			strOutError =
				"Artist F material snapshot no longer matches its source Element.";
			return false;
		}
		if (MaterialSnapshot->second.bEnabled)
		{
			if ((Registry->eBackend !=
					EFFECT_ARTIST31470_SHADER_BACKEND::RUNTIME_V2 &&
				 Registry->eBackend !=
					EFFECT_ARTIST31470_SHADER_BACKEND::ARTIST_V4) ||
				!Registry->bDrawAdmitted)
			{
				strOutError =
					"Artist F typed material snapshot disagrees with the shader registry.";
				return false;
			}
			EFFECT_MATERIAL_EXECUTION_DESC StagedExecution =
				MaterialSnapshot->second;
			/* Every ArtistVisualV4 particle opcode resolves and multiplies the
			   evaluated particle RGBA carrier in HLSL. Older snapshot metadata
			   described several opcodes as policy NONE even though the shader ABI
			   consumed all four channels; that mismatch flattened Track A's dark
			   ink/color-over-life carrier to identity white after authoring. */
			if (EFFECT_ELEMENT_KIND::PARTICLE == InOutElement.eKind &&
				StagedExecution.eBackend ==
					EFFECT_MATERIAL_EXECUTION_BACKEND::ARTIST_VISUAL_V4)
			{
				if (!Try_ResolveArtistVisualV4ParticleColorAbi(
						StagedExecution.iOpcode,
						StagedExecution.iParticleColorPolicy,
						StagedExecution.iParticleColorConsumedMask))
				{
					strOutError =
						"ArtistVisualV4 particle opcode has no declared color ABI.";
					return false;
				}
				StagedExecution.iParticleColorSuppressedMask = 0u;
			}
			std::set<std::string, std::less<>> ExistingTypedLaneIds;
			if (InOutElement.Material.Execution.bEnabled)
			{
				for (EFFECT_MATERIAL_TEXTURE_LANE_DESC& StagedLane :
					StagedExecution.TextureLanes)
				{
					const auto ExistingLane = std::find_if(
						InOutElement.Material.Execution.TextureLanes.begin(),
						InOutElement.Material.Execution.TextureLanes.end(),
						[&StagedLane](
							const EFFECT_MATERIAL_TEXTURE_LANE_DESC& Candidate)
						{
							return Candidate.strLaneId == StagedLane.strLaneId;
						});
					if (ExistingLane !=
							InOutElement.Material.Execution.TextureLanes.end() &&
						!ExistingLane->strAssetId.empty())
					{
						StagedLane.strAssetId = ExistingLane->strAssetId;
						ExistingTypedLaneIds.insert(StagedLane.strLaneId);
					}
				}
			}
			if (!PromoteArtist31470GenericMaterialOverrides(
					SourceElement, eFamily, ExistingTypedLaneIds, InOutElement,
					StagedExecution, strOutError))
			{
				return false;
			}
			InOutElement.Material.Execution = std::move(StagedExecution);
			InOutElement.Material.SourceMaterial = {};
			InOutElement.Material.strTemplateId =
				std::string(EFFECT_STANDARD_MATERIAL_TEMPLATE_ID);
			if (EFFECT_ELEMENT_KIND::PARTICLE == InOutElement.eKind)
			{
				const uint32_t iConsumedMask =
					InOutElement.Material.Execution.iDynamicConsumedMask & 0x0fu;
				f32_t* pStart =
					&InOutElement.Detail.Particle.vDynamicParameterStart.x;
				f32_t* pEnd =
					&InOutElement.Detail.Particle.vDynamicParameterEnd.x;
				for (uint32_t iComponent = 0u; iComponent < 4u; ++iComponent)
				{
					const uint32_t iBit = 1u << iComponent;
					if (0u == (iConsumedMask & iBit) ||
						0u != (InOutElement.Detail.Particle.
							iDynamicParameterComponentMask & iBit))
					{
						continue;
					}
					pStart[iComponent] = 1.f;
					pEnd[iComponent] = 1.f;
				}
				InOutElement.Detail.Particle.iDynamicParameterComponentMask |=
					iConsumedMask;
			}
		}
		else if (Registry->eBackend ==
				EFFECT_ARTIST31470_SHADER_BACKEND::FINITE_COMMON &&
			Registry->eFidelity ==
				EFFECT_ARTIST31470_SHADER_FIDELITY::BOUNDED_EXPLICIT &&
			Registry->bDrawAdmitted && Emitter.Row.iOrder == 17u)
		{
			const EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial =
				SourceElement.Material.SourceMaterial;
			if (!SourceMaterial.bEnabled ||
				SourceMaterial.strRuntimeShaderProfileId !=
					"effect.ue3.missiletrail-01.v1")
			{
				strOutError =
					"Artist F #17 lost its bounded FiniteCommon material profile.";
				return false;
			}
			EFFECT_MATERIAL_DESC StagedMaterial = SourceElement.Material;
			StagedMaterial.Execution = {};
			for (EFFECT_NAMED_TEXTURE_DESC& StagedTexture :
				StagedMaterial.SourceMaterial.Textures)
			{
				const auto ExistingTexture = std::find_if(
					InOutElement.Material.SourceMaterial.Textures.begin(),
					InOutElement.Material.SourceMaterial.Textures.end(),
					[&StagedTexture](const EFFECT_NAMED_TEXTURE_DESC& Candidate)
					{
						return Candidate.strName == StagedTexture.strName;
					});
				if (ExistingTexture !=
						InOutElement.Material.SourceMaterial.Textures.end() &&
					!ExistingTexture->strAssetId.empty())
				{
					StagedTexture.strAssetId = ExistingTexture->strAssetId;
				}
			}
			InOutElement.Material = std::move(StagedMaterial);
		}
		else if (Registry->eBackend ==
				EFFECT_ARTIST31470_SHADER_BACKEND::NONE &&
			Registry->eFidelity ==
				EFFECT_ARTIST31470_SHADER_FIDELITY::UNRESOLVED_FAIL_CLOSED &&
			!Registry->bDrawAdmitted &&
			(Emitter.Row.iOrder == 1u || Emitter.Row.iOrder == 16u ||
			 Emitter.Row.iOrder == 26u || Emitter.Row.iOrder == 33u))
		{
			InOutElement.Material.Execution = {};
			InOutElement.Material.Execution.bFailClosed = true;
			InOutElement.Material.SourceMaterial = {};
			InOutElement.Material.strTemplateId =
				std::string(EFFECT_STANDARD_MATERIAL_TEMPLATE_ID);
			InOutElement.bVisible = false;
		}
		else
		{
			strOutError =
				"Artist F disabled material row has no admitted authored policy.";
			return false;
		}
		if (EFFECT_ELEMENT_KIND::PARTICLE == InOutElement.eKind &&
			!CEffectDocumentCodec::Apply_PortableAuthoredParticleRuntimeCarrier(
				SourceElement, InOutElement, strOutError))
		{
			return false;
		}
		NormalizeArtist31470LegacyGeneratedParticleCarrier(
			Emitter.Row.iOrder, InOutElement);
		return true;
	}


	bool_t InspectArtist31470TrackAUpgrade(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_DOCUMENT_DESC* pSourceDocument,
		const EFFECT_DOCUMENT_DESC& Document,
		EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS& OutStats,
		std::string& strOutError)
	{
		constexpr std::string_view SOURCE_RUNTIME_ID =
			"effect.artist.skill.31470";
		constexpr std::string_view SOURCE_CANDIDATE_ID =
			"effect.artist.skill.31470.native-v14.source-contract-candidate";
		constexpr std::string_view TARGET_ID =
			"effect.artist.skill.31470.unified";
		if (Program.strRuntimeCatalogAssetId != SOURCE_RUNTIME_ID ||
			(nullptr != pSourceDocument &&
			 pSourceDocument->strEffectAssetId != SOURCE_CANDIDATE_ID) ||
			Document.strEffectAssetId != TARGET_ID ||
			!CEffectDocumentCodec::Validate_Drawable(Document, strOutError))
		{
			if (strOutError.empty())
				strOutError = "Artist F authored migration identity is invalid.";
			return false;
		}

		EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS Stats;
		std::array<size_t, 4u> FamilyCounts{};
		std::set<std::string, std::less<>> JoinedTargetIds;
		for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : Program.Emitters)
		{
			ARTIST31470_UNIFIED_FAMILY eFamily =
				ARTIST31470_UNIFIED_FAMILY::END;
			if (!Emitter.bVisible ||
				!Try_ResolveArtist31470UnifiedFamily(Emitter.eRenderer, eFamily))
			{
				continue;
			}
			const std::string strStableId =
				Artist31470UnifiedStableElementId(eFamily, Emitter.Row.strId);
			const EFFECT_ELEMENT_DESC* pSourceElement = nullptr;
			if (nullptr != pSourceDocument)
			{
				const auto SourceElement = std::find_if(
					pSourceDocument->Elements.begin(),
					pSourceDocument->Elements.end(),
					[&Emitter](const EFFECT_ELEMENT_DESC& Candidate)
					{
						return Candidate.strElementId == Emitter.strSourceElementId;
					});
				if (SourceElement != pSourceDocument->Elements.end())
					pSourceElement = &*SourceElement;
			}
			const auto TargetElement = std::find_if(
				Document.Elements.begin(), Document.Elements.end(),
				[&strStableId](const EFFECT_ELEMENT_DESC& Candidate)
				{
					return Candidate.strElementId == strStableId;
				});
			if (strStableId.empty() ||
				(nullptr != pSourceDocument && nullptr == pSourceElement) ||
				TargetElement == Document.Elements.end() ||
				!JoinedTargetIds.insert(strStableId).second ||
				!Artist31470UnifiedElementMatchesFamily(*TargetElement, eFamily) ||
				!Emitter.strMaterialOccurrenceId.has_value())
			{
				strOutError =
					"Artist F authored migration lost a stable source/target join.";
				return false;
			}
			const auto Registry = Find_Artist31470ShaderRegistry(
				Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId);
			if (!Registry.has_value() ||
				!Validate_Artist31470ShaderRegistryEmitterIdentity(
					Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId,
					Emitter.strSourceElementId, Emitter.strSourceEmitterPath))
			{
				strOutError = "Artist F authored migration lost its registry row.";
				return false;
			}

			++Stats.iCoreElementCount;
			++FamilyCounts[static_cast<size_t>(eFamily)];
			if (EFFECT_ELEMENT_KIND::PARTICLE == TargetElement->eKind)
			{
				uint32_t iFixedBurstCount = 0u;
				if (!Try_ResolveArtist31470FixedBurstCount(
						Emitter, iFixedBurstCount, strOutError) ||
					TargetElement->Detail.Particle.iBurstCount !=
						iFixedBurstCount ||
					TargetElement->Detail.Particle.bLocalSpace !=
						Emitter.bLocalSpace ||
					TargetElement->Detail.Particle.iRandomSeed !=
						Emitter.Random.iEmitterRandomSeed ||
					TargetElement->Detail.Particle.iMaxParticles <
						Emitter.iOperationalMaxParticles)
				{
					if (strOutError.empty())
						strOutError =
							"Artist F authored particle carrier differs from Track A.";
					return false;
				}
				++Stats.iParticleElementCount;
				if (!ValidatePortableAuthoredParticleRuntimeCarrier(
						*TargetElement, strOutError))
				{
					if (strOutError.empty())
						strOutError =
							"Artist F authored particle lost its portable Track A runtime carrier.";
					return false;
				}
				if (nullptr != pSourceElement)
				{
					EFFECT_ELEMENT_DESC ExpectedCarrier = *TargetElement;
					if (!CEffectDocumentCodec::
							Apply_PortableAuthoredParticleRuntimeCarrier(
								*pSourceElement, ExpectedCarrier, strOutError))
					{
						return false;
					}
					std::ostringstream ActualRecipe;
					std::ostringstream ExpectedRecipe;
					Write_SourceRecipe(
						ActualRecipe, TargetElement->SourceRecipe, false);
					Write_SourceRecipe(
						ExpectedRecipe, ExpectedCarrier.SourceRecipe, false);
					if (ActualRecipe.str() != ExpectedRecipe.str())
					{
						strOutError =
							"Artist F authored particle runtime carrier differs from Track A.";
						return false;
					}
				}
				++Stats.iPortableParticleRecipeCount;
				Stats.iPortableParticleModuleCount +=
					TargetElement->SourceRecipe.Modules.size();
				for (const EFFECT_SOURCE_MODULE_DESC& Module :
					TargetElement->SourceRecipe.Modules)
				{
					Stats.iPortableParticleDistributionCount +=
						Module.Distributions.size();
				}
				Stats.iFixedBurstTotal += iFixedBurstCount;
				if (iFixedBurstCount > 0u)
					++Stats.iFixedBurstEmitterCount;
			}
			const bool_t bSourceAttachmentEnabled = nullptr != pSourceElement ?
				pSourceElement->ActionCueAttachment.bEnabled :
				Emitter.ActionCueAttachment.bEnabled;
			const bool_t bSourceAttachmentFollow = nullptr != pSourceElement ?
				pSourceElement->ActionCueAttachment.bFollow :
				Emitter.ActionCueAttachment.bFollow;
			if (bSourceAttachmentEnabled && bSourceAttachmentFollow)
			{
				++Stats.iFollowBasisBakedCount;
			}
			else if (bSourceAttachmentEnabled)
			{
				++Stats.iRootBasisBakedCount;
			}
			if (TargetElement->ActionCueAttachment.bEnabled ||
				TargetElement->TransformInheritance.bEnabled)
			{
				strOutError =
					"Artist F authored migration would apply an already-baked basis twice.";
				return false;
			}
			if (ARTIST31470_UNIFIED_FAMILY::MESH == eFamily)
			{
				if (std::abs(TargetElement->Detail.Mesh.fModelPreScale - 0.01f) >
					1.0e-7f)
				{
					strOutError = "Artist F authored MeshParticle lost pre-scale 0.01.";
					return false;
				}
				if (nullptr != pSourceElement)
				{
					const EFFECT_SOURCE_GEOMETRY_BINDING_DESC& Geometry =
						pSourceElement->SourceRecipe.GeometryBinding;
					if (Emitter.strSizeUnitPolicy !=
							"DIMENSIONLESS_AXIS_REORDER_ONLY" ||
						(Geometry.bEnabled &&
							(Geometry.strParticleScaleSemantics !=
								Emitter.strSizeUnitPolicy ||
							 !std::isfinite(Geometry.fCarrierGeometryPreScale) ||
							 std::abs(Geometry.fCarrierGeometryPreScale - 0.01f) >
								 1.0e-7f)))
					{
						strOutError =
							"Artist F source MeshParticle lost its dimensionless size contract.";
						return false;
					}
					const f32_t fDimensionlessScale = 100.f;
					const float2_t vExpectedStart = {
						pSourceElement->Detail.Particle.vStartSize.x *
							fDimensionlessScale,
						pSourceElement->Detail.Particle.vStartSize.y *
							fDimensionlessScale };
					const float2_t vExpectedEnd = {
						pSourceElement->Detail.Particle.vEndSize.x *
							fDimensionlessScale,
						pSourceElement->Detail.Particle.vEndSize.y *
							fDimensionlessScale };
					const auto NearlyEqual = [](const f32_t Left, const f32_t Right)
					{
						return std::abs(Left - Right) <=
							1.0e-5f * (std::max)({ 1.f, std::abs(Left),
								std::abs(Right) });
					};
					if (!NearlyEqual(TargetElement->Detail.Particle.vStartSize.x,
							vExpectedStart.x) ||
						!NearlyEqual(TargetElement->Detail.Particle.vStartSize.y,
							vExpectedStart.y) ||
						!NearlyEqual(TargetElement->Detail.Particle.vEndSize.x,
							vExpectedEnd.x) ||
						!NearlyEqual(TargetElement->Detail.Particle.vEndSize.y,
							vExpectedEnd.y))
					{
						strOutError =
							"Artist F authored MeshParticle lost its dimensionless size contract.";
						return false;
					}
				}
				++Stats.iMeshPreScaleCount;
			}

			if (TargetElement->Material.Execution.bEnabled)
			{
				const bool_t bBackendMatches =
					(Registry->eBackend ==
						EFFECT_ARTIST31470_SHADER_BACKEND::RUNTIME_V2 &&
					 (TargetElement->Material.Execution.eBackend ==
						EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 ||
					  TargetElement->Material.Execution.eBackend ==
						EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL)) ||
					(Registry->eBackend ==
						EFFECT_ARTIST31470_SHADER_BACKEND::ARTIST_V4 &&
					 TargetElement->Material.Execution.eBackend ==
						EFFECT_MATERIAL_EXECUTION_BACKEND::ARTIST_VISUAL_V4);
				if (!bBackendMatches || !Registry->bDrawAdmitted ||
					TargetElement->Material.SourceMaterial.bEnabled)
				{
					strOutError =
						"Artist F authored typed material has an invalid execution boundary.";
					return false;
				}
				if (TargetElement->eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
					TargetElement->Material.Execution.eBackend ==
						EFFECT_MATERIAL_EXECUTION_BACKEND::ARTIST_VISUAL_V4)
				{
					uint32_t iExpectedPolicy = 0u;
					uint32_t iExpectedMask = 0u;
					if (!Try_ResolveArtistVisualV4ParticleColorAbi(
							TargetElement->Material.Execution.iOpcode,
							iExpectedPolicy, iExpectedMask) ||
						TargetElement->Material.Execution.iParticleColorPolicy !=
							iExpectedPolicy ||
						TargetElement->Material.Execution.iParticleColorConsumedMask !=
							iExpectedMask ||
						TargetElement->Material.Execution.
							iParticleColorSuppressedMask != 0u)
					{
						strOutError =
							"Artist F ArtistVisualV4 particle color ABI differs from its shader opcode.";
						return false;
					}
				}
				++Stats.iTypedMaterialCount;
			}
			else if (Registry->eBackend ==
					EFFECT_ARTIST31470_SHADER_BACKEND::FINITE_COMMON &&
				TargetElement->Material.SourceMaterial.bEnabled &&
				TargetElement->Material.SourceMaterial.
					strRuntimeShaderProfileId ==
						"effect.ue3.missiletrail-01.v1")
			{
				++Stats.iFiniteCommonCount;
			}
			else if (Registry->eFidelity ==
					EFFECT_ARTIST31470_SHADER_FIDELITY::UNRESOLVED_FAIL_CLOSED &&
				!Registry->bDrawAdmitted && !TargetElement->bVisible &&
				TargetElement->Material.Execution.bFailClosed &&
				!TargetElement->Material.SourceMaterial.bEnabled)
			{
				++Stats.iFailClosedCount;
			}
			else
			{
				strOutError =
					"Artist F authored material is neither typed, FiniteCommon, nor fail-closed.";
				return false;
			}
		}

		if (FamilyCounts != std::array<size_t, 4u>{ 13u, 16u, 3u, 1u } ||
			Stats.iCoreElementCount != 33u ||
			Stats.iParticleElementCount != 29u ||
			Stats.iFixedBurstEmitterCount != 26u ||
			Stats.iFixedBurstTotal != 167u ||
			Stats.iRootBasisBakedCount != 28u ||
			Stats.iFollowBasisBakedCount != 5u ||
			Stats.iTypedMaterialCount != 28u ||
			Stats.iFiniteCommonCount != 1u ||
			Stats.iFailClosedCount != 4u ||
			Stats.iMeshPreScaleCount != 13u ||
			Stats.iPortableParticleRecipeCount != 29u ||
			Stats.iPortableParticleModuleCount != 350u ||
			Stats.iPortableParticleDistributionCount != 564u ||
			JoinedTargetIds.size() != 33u)
		{
			strOutError =
				"Artist F authored migration denominator changed; expected Core33, Particle29 with 29 portable recipes (350 modules/564 distributions), burst 26/167, basis 28/5, pre-scale 13, material 28/1/4.";
			return false;
		}
		OutStats = Stats;
		strOutError.clear();
		return true;
	}

}


bool_t Client::CEffectDocumentCodec::Build_Artist31470UnifiedTrackAUpgrade(
	const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
	const EFFECT_DOCUMENT_DESC& SourceDocument,
	const std::unordered_map<std::string, EFFECT_MATERIAL_EXECUTION_DESC>&
		MaterialSnapshots,
	const EFFECT_DOCUMENT_DESC& ExistingDocument,
	EFFECT_DOCUMENT_DESC& OutDocument,
	EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS& OutStats,
	std::string& strOutError)
{
	EFFECT_DOCUMENT_DESC Staged = ExistingDocument;
	std::set<std::string, std::less<>> JoinedTargetIds;
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : Program.Emitters)
	{
		ARTIST31470_UNIFIED_FAMILY eFamily =
			ARTIST31470_UNIFIED_FAMILY::END;
		if (!Emitter.bVisible ||
			!Try_ResolveArtist31470UnifiedFamily(Emitter.eRenderer, eFamily))
		{
			continue;
		}
		const std::string strStableId =
			Artist31470UnifiedStableElementId(eFamily, Emitter.Row.strId);
		const auto SourceElement = std::find_if(
			SourceDocument.Elements.begin(), SourceDocument.Elements.end(),
			[&Emitter](const EFFECT_ELEMENT_DESC& Candidate)
			{
				return Candidate.strElementId == Emitter.strSourceElementId;
			});
		auto TargetElement = std::find_if(
			Staged.Elements.begin(), Staged.Elements.end(),
			[&strStableId](const EFFECT_ELEMENT_DESC& Candidate)
			{
				return Candidate.strElementId == strStableId;
			});
		if (strStableId.empty() ||
			SourceElement == SourceDocument.Elements.end() ||
			TargetElement == Staged.Elements.end() ||
			!JoinedTargetIds.insert(strStableId).second)
		{
			strOutError =
				"Artist F authored migration rejected a missing or duplicate stable Element join.";
			return false;
		}
		if (!ApplyArtist31470TrackAElementData(
				Emitter, *SourceElement, MaterialSnapshots,
				*TargetElement, strOutError))
		{
			return false;
		}
	}
	EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS Stats;
	if (!InspectArtist31470TrackAUpgrade(
			Program, &SourceDocument, Staged, Stats, strOutError))
	{
		return false;
	}
	OutDocument = std::move(Staged);
	OutStats = Stats;
	strOutError.clear();
	return true;
}


bool_t Client::CEffectDocumentCodec::Validate_Artist31470UnifiedTrackAUpgrade(
	const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
	const EFFECT_DOCUMENT_DESC& SourceDocument,
	const EFFECT_DOCUMENT_DESC& Document,
	EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS& OutStats,
	std::string& strOutError)
{
	return InspectArtist31470TrackAUpgrade(
		Program, &SourceDocument, Document, OutStats, strOutError);
}


bool_t Client::CEffectDocumentCodec::
	Validate_Artist31470UnifiedAuthoredReadiness(
	const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
	const EFFECT_DOCUMENT_DESC& Document,
	EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS& OutStats,
	std::string& strOutError)
{
	return InspectArtist31470TrackAUpgrade(
		Program, nullptr, Document, OutStats, strOutError);
}


bool_t Client::CEffectDocumentCodec::
	Validate_ReconstructedRuntimeDrawable(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	if (Document.iFormatVersion != EFFECT_AUTHORING_FORMAT_VERSION ||
		Document.iLoadedFormatVersion != EFFECT_AUTHORING_FORMAT_VERSION ||
		Document.bSourceContract ||
		Document.Elements.empty())
	{
		strOutError =
			"Reconstructed runtime drawable identity is invalid.";
		return false;
	}
	EFFECT_DOCUMENT_DESC LegacyValidationProjection = Document;
	for (size_t iElement = 0u; iElement < Document.Elements.size(); ++iElement)
	{
		const EFFECT_ELEMENT_DESC& Element = Document.Elements[iElement];
		const EFFECT_SOURCE_SPACE eExpectedSourceSpace =
			Element.Renderer.eType == EFFECT_RENDERER_TYPE::SCREEN_POST ?
				EFFECT_SOURCE_SPACE::SCREEN_SPACE_V1 :
				EFFECT_SOURCE_SPACE::UE3_CASCADE_V1;
		if (Element.Renderer.eType >= EFFECT_RENDERER_TYPE::END ||
			Kind_ForRenderer(Element.Renderer.eType) != Element.eKind ||
			Element.Renderer.eSourceSpace != eExpectedSourceSpace)
		{
			strOutError =
				"Reconstructed runtime renderer type/source-space does not match "
				"its Element kind.";
			return false;
		}
		LegacyValidationProjection.Elements[iElement].Renderer = {};
		/*
		 * Validate_Drawable below deliberately exercises the legacy carrier
		 * projection after stripping the typed Renderer.  Source TypeDataMesh
		 * rotation is renderer-owned, so retaining it in that legacy-only copy
		 * would create an impossible "no mesh renderer + mesh carrier rotation"
		 * document.  The original typed Document remains unchanged and is
		 * validated by the renderer contract above.
		 */
		LegacyValidationProjection.Elements[iElement].Detail.Mesh.
			vSourceTypeDataRotationDegrees = {};
	}
	if (!Validate_Drawable(LegacyValidationProjection, strOutError))
		return false;
	strOutError.clear();
	return true;
}


bool_t Client::CEffectDocumentCodec::
	Validate_Artist31470ReconstructedRuntimeDrawable(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	constexpr std::string_view ARTIST_31470_EFFECT_ID =
		"effect.artist.skill.31470";
	if (Document.strEffectAssetId != ARTIST_31470_EFFECT_ID ||
		Document.Elements.size() != 35u)
	{
		strOutError =
			"Artist 31470 reconstructed runtime drawable identity is invalid.";
		return false;
	}
	if (!Validate_ReconstructedRuntimeDrawable(Document, strOutError))
		return false;
	std::array<uint32_t,
		static_cast<size_t>(EFFECT_RENDERER_TYPE::END)> RendererCounts{};
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
		++RendererCounts[static_cast<size_t>(Element.Renderer.eType)];
	if (RendererCounts[static_cast<size_t>(
			EFFECT_RENDERER_TYPE::MESH_PARTICLE)] != 13u ||
		RendererCounts[static_cast<size_t>(
			EFFECT_RENDERER_TYPE::SPRITE_PARTICLE)] != 16u ||
		RendererCounts[static_cast<size_t>(
			EFFECT_RENDERER_TYPE::DECAL_PARTICLE)] != 3u ||
		RendererCounts[static_cast<size_t>(
			EFFECT_RENDERER_TYPE::CASCADE_RIBBON)] != 1u ||
		RendererCounts[static_cast<size_t>(
			EFFECT_RENDERER_TYPE::LIGHT_PARTICLE)] != 1u ||
		RendererCounts[static_cast<size_t>(
			EFFECT_RENDERER_TYPE::SCREEN_POST)] != 1u)
	{
		strOutError =
			"Artist 31470 reconstructed runtime renderer denominator changed.";
		return false;
	}
	strOutError.clear();
	return true;
}
