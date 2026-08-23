#include "Effect_Playback.h"

#include "Effect_DocumentCodec.h"
#include "Effect_Distribution.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_VisualProgramCorpus.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace
{
	constexpr f32_t FIXED_STEP_SECONDS = 1.f / 60.f;
	constexpr f64_t FIXED_STEP_SECONDS_EXACT = 1.0 / 60.0;
	constexpr f64_t FIXED_STEP_EPSILON = 1.0e-9;
	// Keep a one-second bounded budget so a 3 FPS frame (20 fixed steps)
	// advances at the same rate as the authoritative action/animation clock.
	constexpr uint32_t MAX_CATCH_UP_STEPS = 60u;
	constexpr uint32_t MAX_SOURCE_EVENTS_PER_STEP = 4096u;
	struct DYNAMIC_PARAMETER_PROPERTY_PATHS final
	{
		std::string_view strSpawnTimeOnly;
		std::string_view strUseEmitterTime;
		std::string_view strValue;
		std::string_view strScaleVelocityByValue;
	};
	constexpr std::array<DYNAMIC_PARAMETER_PROPERTY_PATHS, 4u>
		DYNAMIC_PARAMETER_PATHS = {{
			{ "dynamicparams[0].bspawntimeonly",
			  "dynamicparams[0].buseemittertime",
			  "dynamicparams[0].paramvalue",
			  "dynamicparams[0].bscalevelocitybyparamvalue" },
			{ "dynamicparams[1].bspawntimeonly",
			  "dynamicparams[1].buseemittertime",
			  "dynamicparams[1].paramvalue",
			  "dynamicparams[1].bscalevelocitybyparamvalue" },
			{ "dynamicparams[2].bspawntimeonly",
			  "dynamicparams[2].buseemittertime",
			  "dynamicparams[2].paramvalue",
			  "dynamicparams[2].bscalevelocitybyparamvalue" },
			{ "dynamicparams[3].bspawntimeonly",
			  "dynamicparams[3].buseemittertime",
			  "dynamicparams[3].paramvalue",
			  "dynamicparams[3].bscalevelocitybyparamvalue" }
		}};
	constexpr std::string_view WARLORD_CHAIN_SOURCE_MATERIAL_PATH =
		"fx_m_mi_d_00.fx_mi.fx_d_me_chain_01_101_ma";
	constexpr std::string_view WARLORD_CHAIN_BASE_ALIAS_ASSET_ID =
		"Effect/Warlord/Textures/FX_TEX_02/fx_d_atypical_028.dds";
	constexpr std::string_view WARLORD_CHAIN_SOURCE_PROFILE_ID =
		"ue3.material.fx.m.mi.00.fx.m.fx.d.me.chain.01.ma.a8a92d2a6abc";
	constexpr std::string_view WARLORD_CHAIN_PARENT_MATERIAL_PATH =
		"fx_m_mi_00.fx_m.fx_d_me_chain_01_ma";
	constexpr std::string_view WARLORD_CHAIN_RUNTIME_PROFILE_ID =
		"effect.ue3.grouped-translucent.v1";
	constexpr std::string_view WARLORD_CHAIN_06_MODEL_ASSET_ID =
		"Effect/Warlord/Meshes/FX_SM_01/fm_d_berchain_06.wmodel";
	constexpr std::string_view WARLORD_CHAIN_07_MODEL_ASSET_ID =
		"Effect/Warlord/Meshes/FX_SM_01/fm_d_berchain_07.wmodel";

	bool_t Is_FiniteAffineMatrix(const float4x4_t& Matrix)
	{
		const f32_t* const Values = &Matrix._11;
		for (size_t i = 0u; i < 16u; ++i)
		{
			if (!std::isfinite(Values[i]))
				return false;
		}
		const f32_t Determinant =
			Matrix._11 * (Matrix._22 * Matrix._33 - Matrix._23 * Matrix._32) -
			Matrix._12 * (Matrix._21 * Matrix._33 - Matrix._23 * Matrix._31) +
			Matrix._13 * (Matrix._21 * Matrix._32 - Matrix._22 * Matrix._31);
		return std::isfinite(Determinant) && std::abs(Determinant) > 1.0e-8f &&
			std::abs(Matrix._14) <= 1.0e-5f &&
			std::abs(Matrix._24) <= 1.0e-5f &&
			std::abs(Matrix._34) <= 1.0e-5f &&
			std::abs(Matrix._44 - 1.f) <= 1.0e-5f;
	}

	struct SOURCE_VECTOR_FIELD final
	{
		uint32_t iSizeX = 0u;
		uint32_t iSizeY = 0u;
		uint32_t iSizeZ = 0u;
		std::vector<float3_t> Samples;
	};

	struct VECTOR_FIELD_HEADER final
	{
		char Magic[4]{};
		uint32_t iVersion = 0u;
		uint32_t iSizeX = 0u;
		uint32_t iSizeY = 0u;
		uint32_t iSizeZ = 0u;
		uint32_t iSampleCount = 0u;
	};

	std::atomic_uint64_t g_iVectorFieldDiskLoadCount{ 0u };

	uint32_t Hash_StableId(const std::string& Value)
	{
		uint32_t Hash = 2166136261u;
		for (const unsigned char Character : Value)
		{
			Hash ^= Character;
			Hash *= 16777619u;
		}
		return 0u == Hash ? 1u : Hash;
	}

	f32_t Clamp01(const f32_t Value)
	{
		return std::clamp(Value, 0.f, 1.f);
	}

	float3_t Lerp3(const float3_t& A, const float3_t& B, const f32_t T)
	{
		return float3_t(
			A.x + (B.x - A.x) * T,
			A.y + (B.y - A.y) * T,
			A.z + (B.z - A.z) * T);
	}

	float4_t Lerp4(const float4_t& A, const float4_t& B, const f32_t T)
	{
		return float4_t(
			A.x + (B.x - A.x) * T,
			A.y + (B.y - A.y) * T,
			A.z + (B.z - A.z) * T,
			A.w + (B.w - A.w) * T);
	}

	float3_t Add3(const float3_t& A, const float3_t& B)
	{
		return float3_t(A.x + B.x, A.y + B.y, A.z + B.z);
	}

	float3_t Scale3(const float3_t& Value, const f32_t Scale)
	{
		return float3_t(Value.x * Scale, Value.y * Scale, Value.z * Scale);
	}

	float3_t UE3_VectorToClient(const float3_t& Value)
	{
		return float3_t(Value.x, Value.z, -Value.y);
	}

	float3_t UE3_CentimetersToClient(const float3_t& Value)
	{
		return Scale3(UE3_VectorToClient(Value), 0.01f);
	}

	float3_t UE3_AxisScaleToClient(const float3_t& Value)
	{
		// A component-wise scale follows the remapped axes, but does not inherit
		// the handedness sign used by position/direction vectors.
		return float3_t(Value.x, Value.z, Value.y);
	}

	matrix_t UE3_EulerDegreesToClientRotation(const float3_t& SourceDegrees)
	{
		// Source MeshRotation vectors are UE roll(X), pitch(Y), yaw(Z).  Rotation
		// is not a vector: preserve the source Euler composition and conjugate the
		// resulting row matrix into the Client X-forward/Y-up basis.
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

	bool_t Is_MeshParticle(const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return std::any_of(
			Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
			[](const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == "meshModel";
			});
	}

	float3_t UE3_SizeToClient(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const float3_t& Value)
	{
		// Billboard StartSize X/Y are renderer-plane axes, not UE world axes.
		// Mesh particles use a converted WModel and therefore need the same
		// component-axis reorder as the mesh transform, without a handedness sign.
		return Is_MeshParticle(Element) ? UE3_AxisScaleToClient(Value) : Value;
	}

	float3_t UE3_StartSizeToClient(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const float3_t& Value)
	{
		// Mesh StartSize is a dimensionless instance scale.  Its WModel carrier
		// already applies geometryPreScale exactly once during model creation.
		// Sprite StartSize remains a centimetre-sized billboard extent.
		return Is_MeshParticle(Element) ?
			UE3_AxisScaleToClient(Value) : Scale3(Value, 0.01f);
	}

	f32_t DistanceSquared(const float3_t& A, const float3_t& B)
	{
		const f32_t X = A.x - B.x;
		const f32_t Y = A.y - B.y;
		const f32_t Z = A.z - B.z;
		return X * X + Y * Y + Z * Z;
	}

	bool_t Interpolate_BakedEdgeHistorySample(
		const Client::EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY& History,
		const double fTimeSeconds,
		Client::EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_SAMPLE& Out)
	{
		if (History.Samples.empty() || !std::isfinite(fTimeSeconds) ||
			fTimeSeconds < History.Samples.front().fRelativeTimeSeconds - 1e-9 ||
			fTimeSeconds > History.Samples.back().fRelativeTimeSeconds + 1e-9)
			return false;
		const auto Upper = std::lower_bound(
			History.Samples.begin(), History.Samples.end(), fTimeSeconds,
			[](const auto& Sample, const double Time)
			{
				return Sample.fRelativeTimeSeconds < Time;
			});
		if (Upper == History.Samples.begin())
		{
			Out = *Upper;
			return true;
		}
		if (Upper == History.Samples.end())
		{
			Out = History.Samples.back();
			return true;
		}
		if (std::abs(Upper->fRelativeTimeSeconds - fTimeSeconds) <= 1e-9)
		{
			Out = *Upper;
			return true;
		}
		const auto& Lower = *(Upper - 1);
		const double fSpan = Upper->fRelativeTimeSeconds -
			Lower.fRelativeTimeSeconds;
		if (!(fSpan > 0.0))
			return false;
		const double fRatio = std::clamp(
			(fTimeSeconds - Lower.fRelativeTimeSeconds) / fSpan, 0.0, 1.0);
		Client::EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_SAMPLE Staged;
		Staged.fRelativeTimeSeconds = fTimeSeconds;
		for (size_t iAxis = 0u; iAxis < 3u; ++iAxis)
		{
			Staged.vFirstEdgeUE3Cm[iAxis] = Lower.vFirstEdgeUE3Cm[iAxis] +
				(Upper->vFirstEdgeUE3Cm[iAxis] -
				 Lower.vFirstEdgeUE3Cm[iAxis]) * fRatio;
			Staged.vControlPointUE3Cm[iAxis] = Lower.vControlPointUE3Cm[iAxis] +
				(Upper->vControlPointUE3Cm[iAxis] -
				 Lower.vControlPointUE3Cm[iAxis]) * fRatio;
			Staged.vSecondEdgeUE3Cm[iAxis] = Lower.vSecondEdgeUE3Cm[iAxis] +
				(Upper->vSecondEdgeUE3Cm[iAxis] -
				 Lower.vSecondEdgeUE3Cm[iAxis]) * fRatio;
		}
		Out = std::move(Staged);
		return true;
	}

	float3_t Transform_BakedEdgeUE3Cm(
		const std::array<double, 3u>& Source,
		const matrix_t& WorldMatrix)
	{
		const float3_t SourceUE3Cm(
			static_cast<f32_t>(Source[0]),
			static_cast<f32_t>(Source[1]),
			static_cast<f32_t>(Source[2]));
		const float3_t Local = UE3_CentimetersToClient(SourceUE3Cm);
		float3_t World{};
		XMStoreFloat3(&World, XMVector3TransformCoord(
			XMLoadFloat3(&Local), WorldMatrix));
		return World;
	}

	float3_t Get_Translation(const float4x4_t& Matrix)
	{
		return float3_t(Matrix._41, Matrix._42, Matrix._43);
	}

	float3_t Multiply3(const float3_t& A, const float3_t& B)
	{
		return float3_t(A.x * B.x, A.y * B.y, A.z * B.z);
	}

	float3_t Subtract3(const float3_t& A, const float3_t& B)
	{
		return float3_t(A.x - B.x, A.y - B.y, A.z - B.z);
	}

	float3_t Lerp3Clamped(
		const float3_t& A,
		const float3_t& B,
		const f32_t fRatio)
	{
		return Lerp3(A, B, std::clamp(fRatio, 0.f, 1.f));
	}

	f32_t Length3(const float3_t& Value)
	{
		return std::sqrt(
			Value.x * Value.x + Value.y * Value.y + Value.z * Value.z);
	}

	float3_t Normalize3(const float3_t& Value)
	{
		const f32_t fLength = Length3(Value);
		return fLength > 1.e-6f ? Scale3(Value, 1.f / fLength) : float3_t{};
	}

	const Client::EFFECT_SOURCE_MODULE_DESC* Find_SourceModule(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const char_t* pClassName)
	{
		const auto Iterator = std::find_if(
			Element.SourceRecipe.Modules.begin(),
			Element.SourceRecipe.Modules.end(),
			[pClassName](const Client::EFFECT_SOURCE_MODULE_DESC& Module)
			{
				return Module.strClassName == pClassName;
			});
		return Iterator == Element.SourceRecipe.Modules.end() ? nullptr :
			&*Iterator;
	}

	const Client::EFFECT_DISTRIBUTION_DESC* Find_SourceDistribution(
		const Client::EFFECT_SOURCE_MODULE_DESC* pModule,
		const std::string_view PropertyPath)
	{
		if (nullptr == pModule)
			return nullptr;
		const auto Iterator = std::find_if(
			pModule->Distributions.begin(), pModule->Distributions.end(),
			[PropertyPath](const Client::EFFECT_DISTRIBUTION_DESC& Distribution)
			{
				return Distribution.strPropertyPath == PropertyPath;
			});
		return Iterator == pModule->Distributions.end() ? nullptr : &*Iterator;
	}

	bool_t Uses_ImplicitAlphaIdentity(
		const Client::EFFECT_DISTRIBUTION_DESC& Distribution,
		const std::string_view PropertyPath)
	{
		if (!Distribution.LookupTable.empty() || !Distribution.Keys.empty())
			return false;
		if (Distribution.vDefaultMinimum.x != 0.f ||
			Distribution.vDefaultMaximum.x != 0.f)
		{
			return false;
		}
		return PropertyPath == "startalpha" ||
			PropertyPath == "alphaoverlife" ||
			PropertyPath == "alphascaleoverlife";
	}

	bool_t Is_SourceNullCdoDistribution(
		const Client::EFFECT_DISTRIBUTION_DESC& Distribution,
		const uint32_t iExpectedComponentCount)
	{
		const auto IsZero4 = [](const float4_t& Value)
		{
			return Value.x == 0.f && Value.y == 0.f &&
				Value.z == 0.f && Value.w == 0.f;
		};
		return Distribution.strSourceClass.empty() &&
			Distribution.strSourceObjectPath.empty() &&
			Distribution.iComponentCount == iExpectedComponentCount &&
			Distribution.iOperation == 1u &&
			Distribution.iRandomLockAxes == 0u &&
			Distribution.iLookupTableChunkSize == 0u &&
			Distribution.iLookupTableNumElements == 0u &&
			Distribution.fLookupTableTimeScale == 0.f &&
			Distribution.fLookupTableStartTime == 0.f &&
			IsZero4(Distribution.vDefaultMinimum) &&
			IsZero4(Distribution.vDefaultMaximum) &&
			Distribution.LookupTable.empty() && Distribution.Keys.empty();
	}

	bool_t Is_Warlord17090ChainTypedScaleIdentity(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const Client::EFFECT_SOURCE_MODULE_DESC& Module)
	{
		using namespace Client;
		const EFFECT_SOURCE_MATERIAL_DESC& SourceProfile =
			Element.Material.SourceMaterial;
		if (Module.strClassName != "particlemodulelocationdirect" ||
			Element.Material.strSourceMaterialPath !=
				WARLORD_CHAIN_SOURCE_MATERIAL_PATH ||
			Element.Material.strTemplateId !=
				EFFECT_SOURCE_MATERIAL_TEMPLATE_ID ||
			!SourceProfile.bEnabled ||
			SourceProfile.strProfileId != WARLORD_CHAIN_SOURCE_PROFILE_ID ||
			SourceProfile.strParentMaterialPath !=
				WARLORD_CHAIN_PARENT_MATERIAL_PATH ||
			SourceProfile.strRuntimeShaderProfileId !=
				WARLORD_CHAIN_RUNTIME_PROFILE_ID ||
			Element.Material.Execution.bEnabled ||
			!Element.Material.Execution.bFailClosed ||
			!Element.Material.Execution.bAuthoringApproximate ||
			Element.Detail.Mesh.bUseModelMaterial ||
			!Element.strSourceNode.starts_with(
				"authored-source-particle:effect.warlord.skill.17090.unified|"))
		{
			return false;
		}
		const auto ResolveCompilerAssetId = [&Element](
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
		bool_t bExactMesh = false;
		bool_t bPreviewBase = false;
		for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
			Element.ResourceBindings)
		{
			std::string_view strCompilerAssetId;
			if (!ResolveCompilerAssetId(Binding, strCompilerAssetId))
				return false;
			if (Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID &&
				(strCompilerAssetId == WARLORD_CHAIN_06_MODEL_ASSET_ID ||
				 strCompilerAssetId == WARLORD_CHAIN_07_MODEL_ASSET_ID))
			{
				if (bExactMesh)
					return false;
				bExactMesh = true;
			}
			else if (Binding.strSlotId == "base" &&
				strCompilerAssetId == WARLORD_CHAIN_BASE_ALIAS_ASSET_ID)
			{
				if (bPreviewBase)
					return false;
				bPreviewBase = true;
			}
			else
			{
				return false;
			}
		}
		const EFFECT_DISTRIBUTION_DESC* pScaleFactor =
			Find_SourceDistribution(&Module, "scalefactor");
		const auto IsZero4 = [](const float4_t& Value)
		{
			return Value.x == 0.f && Value.y == 0.f &&
				Value.z == 0.f && Value.w == 0.f;
		};
		return bExactMesh && bPreviewBase && nullptr != pScaleFactor &&
			pScaleFactor->strSourceClass.empty() &&
			pScaleFactor->strSourceObjectPath.empty() &&
			pScaleFactor->iComponentCount == 3u &&
			pScaleFactor->iOperation == 1u &&
			pScaleFactor->iRandomLockAxes == 0u &&
			pScaleFactor->iLookupTableChunkSize == 0u &&
			pScaleFactor->iLookupTableNumElements == 0u &&
			pScaleFactor->fLookupTableTimeScale == 0.f &&
			pScaleFactor->fLookupTableStartTime == 0.f &&
			IsZero4(pScaleFactor->vDefaultMinimum) &&
			IsZero4(pScaleFactor->vDefaultMaximum) &&
			pScaleFactor->LookupTable.empty() && pScaleFactor->Keys.empty();
	}

	bool_t SourceClass_Matches(
		const Client::EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view BaseClass)
	{
		std::string_view Class = Module.strClassName;
		if (Class.starts_with("efparticlemodule"))
			Class.remove_prefix(2u);
		if (Class.ends_with("_seeded"))
			Class.remove_suffix(7u);
		return Class == BaseClass;
	}

	const Client::EFFECT_ELEMENT_DESC* Find_ElementById(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		const std::string_view strElementId)
	{
		const auto Iterator = std::find_if(
			Document.Elements.begin(), Document.Elements.end(),
			[strElementId](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == strElementId;
			});
		return Iterator == Document.Elements.end() ? nullptr : &*Iterator;
	}

	bool_t Nearly_EqualTrailValue(const double fLeft, const double fRight)
	{
		return std::isfinite(fLeft) && std::isfinite(fRight) &&
			std::abs(fLeft - fRight) <= 1.0e-5;
	}

	bool_t Matches_TrailTiming(
		const Client::EFFECT_VISUAL_PROGRAM_TRAIL_TIMING& Packet,
		const Client::EFFECT_TIMING_DESC& Target)
	{
		return Nearly_EqualTrailValue(
			Packet.fStartDelaySeconds, Target.fStartDelaySeconds) &&
			Nearly_EqualTrailValue(
				Packet.fLifeTimeSeconds, Target.fLifeTimeSeconds) &&
			Nearly_EqualTrailValue(
				Packet.fAfterImageSeconds, Target.fAfterImageSeconds) &&
			Nearly_EqualTrailValue(Packet.fDissolveStartNormalized,
				Target.fDissolveStartNormalized);
	}

	bool_t Matches_TrailAttachment(
		const Client::EFFECT_VISUAL_PROGRAM_TRAIL_ATTACHMENT& Packet,
		const Client::EFFECT_ACTION_CUE_ATTACHMENT_DESC& Target)
	{
		const auto Matches3 = [](const std::array<double, 3u>& Left,
			const float3_t& Right)
		{
			return Nearly_EqualTrailValue(Left[0u], Right.x) &&
				Nearly_EqualTrailValue(Left[1u], Right.y) &&
				Nearly_EqualTrailValue(Left[2u], Right.z);
		};
		return Packet.bEnabled == Target.bEnabled &&
			Packet.bFollow == Target.bFollow &&
			Packet.strSourceAnchorSlotId == Target.strSourceAnchorSlotId &&
			Packet.strRuntimeAnchorSlotId == Target.strRuntimeAnchorSlotId &&
			Packet.strRuntimeBoneName == Target.strRuntimeBoneName &&
			Nearly_EqualTrailValue(
				Packet.fSnapshotRootSourceBasisYawDegrees,
				Target.fSnapshotRootSourceBasisYawDegrees) &&
			Matches3(Packet.vPosition,
				Target.SocketLocalTransform.vPosition) &&
			Matches3(Packet.vRotationDegrees,
				Target.SocketLocalTransform.vRotationDegrees) &&
			Matches3(Packet.vScale, Target.SocketLocalTransform.vScale);
	}

	bool_t Matches_TrailGeometry(
		const Client::EFFECT_VISUAL_PROGRAM_TRAIL_GEOMETRY& Packet,
		const Client::EFFECT_TRAIL_DESC& Target)
	{
		return Packet.iMaxPoints == Target.iMaxPoints &&
			Nearly_EqualTrailValue(
				Packet.fPointLifeTimeSeconds, Target.fPointLifeTimeSeconds) &&
			Nearly_EqualTrailValue(
				Packet.fSampleIntervalSeconds, Target.fSampleIntervalSeconds) &&
			Nearly_EqualTrailValue(
				Packet.fMinimumDistance, Target.fMinimumDistance) &&
			Nearly_EqualTrailValue(Packet.fStartWidth, Target.fStartWidth) &&
			Nearly_EqualTrailValue(Packet.fEndWidth, Target.fEndWidth) &&
			Packet.bFaceCamera == Target.bFaceCamera;
	}

	bool_t Is_CascadeRibbonTarget(
		const Client::EFFECT_ELEMENT_DESC& Target)
	{
		return Target.eKind == Client::EFFECT_ELEMENT_KIND::TRAIL &&
			Target.SourceRecipe.bEnabled &&
			Target.SourceRecipe.strRendererShape == "ribbon" &&
			std::ranges::any_of(Target.SourceRecipe.Modules,
				[](const Client::EFFECT_SOURCE_MODULE_DESC& Module)
				{
					return SourceClass_Matches(
						Module, "particlemoduletypedataribbon");
				});
	}

	bool_t Validate_CascadeRibbonPacket(
		const Client::EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental,
		const Client::EFFECT_ELEMENT_DESC& Target,
		std::string& strOutError)
	{
		if (!Supplemental.CascadeRibbonPacket.has_value() ||
			Supplemental.AnimationTrailPacket.has_value() ||
			!Is_CascadeRibbonTarget(Target))
		{
			strOutError =
				"CascadeRibbon supplemental target or immutable packet is invalid.";
			return false;
		}
		const Client::EFFECT_VISUAL_PROGRAM_CASCADE_RIBBON_PACKET& Packet =
			*Supplemental.CascadeRibbonPacket;
		const bool_t bTypeDataExact = std::ranges::any_of(
			Target.SourceRecipe.Modules,
			[&Packet](const Client::EFFECT_SOURCE_MODULE_DESC& Module)
			{
				return SourceClass_Matches(
						Module, "particlemoduletypedataribbon") &&
					Module.strStableId == Packet.strTypeDataStableId &&
					Module.strClassName == Packet.strTypeDataClassName &&
					Module.strObjectPath == Packet.strTypeDataObjectPath;
			});
		if (0u == Packet.iPacketVersion ||
			Packet.strAdapterId != Supplemental.strAdapterId ||
			!Packet.bBoundedSemanticReplay || Packet.bNativeExecution ||
			Packet.strRuntimeCarrier.empty() ||
			Packet.strTypeDataModuleSha256.empty() ||
			Packet.strResolvedRendererShape != "ribbon" ||
			Packet.strSourceRecipeSha256.empty() ||
			Packet.strModuleClosureSha256.empty() ||
			Packet.strPacketSha256.empty() || !bTypeDataExact ||
			Packet.iModuleCount != Target.SourceRecipe.Modules.size() ||
			Packet.iOperationalMaxPoints != Target.Detail.Trail.iMaxPoints ||
			!Nearly_EqualTrailValue(Packet.fTilingDistance,
				Target.Detail.Trail.fTilingDistanceWorldUnits) ||
			!Nearly_EqualTrailValue(Packet.fDistanceTessellationStepSize,
				Target.Detail.Trail.fDistanceTessellationStepWorldUnits) ||
			!Matches_TrailTiming(Packet.Timing, Target.Detail.Timing) ||
			!Matches_TrailAttachment(Packet.Attachment,
				Target.ActionCueAttachment) ||
			!Matches_TrailGeometry(Packet.Trail, Target.Detail.Trail))
		{
			strOutError =
				"CascadeRibbon immutable packet does not match its projected Trail target.";
			return false;
		}
		return true;
	}

	bool_t Validate_AnimationTrailPacket(
		const Client::EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental,
		const Client::EFFECT_ELEMENT_DESC& Target,
		std::string& strOutError)
	{
		if (!Supplemental.AnimationTrailPacket.has_value() ||
			Supplemental.CascadeRibbonPacket.has_value() ||
			Target.eKind != Client::EFFECT_ELEMENT_KIND::TRAIL ||
			Target.SourceRecipe.bEnabled || !Target.SourceRecipe.Modules.empty())
		{
			strOutError =
				"AnimationTrail supplemental target or immutable packet is invalid.";
			return false;
		}
		const Client::EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_PACKET& Packet =
			*Supplemental.AnimationTrailPacket;
		const bool_t bLegacyPacket = 1u == Packet.iPacketVersion &&
			Packet.strRuntimeCarrier == "EFFECT_TYPED_ANIMATION_TRAIL_V1";
		const bool_t bBakedEdgePacket = 2u == Packet.iPacketVersion &&
			Packet.strRuntimeCarrier ==
				"EFFECT_TYPED_ANIMATION_TRAIL_BAKED_EDGE_V1" &&
			!Packet.strHistoryId.empty() && !Packet.strHistorySha256.empty() &&
			Packet.fPlaybackClampSeconds > 0.0 &&
			Packet.strCoordinateBasis ==
				"UE3_CM_X_Z_NEG_Y_TO_RUNTIME_METERS";
		if ((!bLegacyPacket && !bBakedEdgePacket) ||
			Packet.strAdapterId != Supplemental.strAdapterId ||
			!Packet.bBoundedSemanticReplay || Packet.bNativeExecution ||
			Packet.strRuntimeCarrier.empty() ||
			Packet.strSourceNotifyType.empty() ||
			Packet.strSourceEventId != Supplemental.strSourceEventId ||
			Packet.strSourceEventRecordSha256.empty() ||
			Packet.strSourceAsset.empty() || Packet.strClip.empty() ||
			Packet.strTargetElementId != Target.strElementId ||
			Packet.strTargetElementId !=
				Supplemental.TargetIdentity.strTargetElementId ||
			Packet.strPacketSha256.empty() ||
			!Nearly_EqualTrailValue(Packet.fLocalTimeSeconds,
				Supplemental.fLocalTimeSeconds) ||
			!Nearly_EqualTrailValue(Packet.fGlobalTimeSeconds,
				Supplemental.fSourceTimelineSeconds) ||
			!Nearly_EqualTrailValue(Packet.fDurationSeconds,
				Supplemental.fDurationSeconds) ||
			(bBakedEdgePacket && !Nearly_EqualTrailValue(
				Packet.fPlaybackClampSeconds, Packet.fDurationSeconds)) ||
			!Matches_TrailTiming(Packet.TargetTiming, Target.Detail.Timing) ||
			!Matches_TrailAttachment(Packet.Attachment,
				Target.ActionCueAttachment) ||
			!Matches_TrailGeometry(Packet.Trail, Target.Detail.Trail))
		{
			strOutError =
				"AnimationTrail immutable notify packet does not match its projected Trail target.";
			return false;
		}
		return true;
	}

	bool_t Validate_BakedEdgeLightPacket(
		const Client::EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental,
		const Client::EFFECT_ELEMENT_DESC& Target,
		std::string& strOutError)
	{
		if (!Supplemental.BakedEdgeLightPacket.has_value() ||
			Supplemental.CascadeRibbonPacket.has_value() ||
			Supplemental.AnimationTrailPacket.has_value() ||
			Target.eKind != Client::EFFECT_ELEMENT_KIND::LIGHT ||
			!Target.bVisible || Target.SourceRecipe.bEnabled ||
			!Target.SourceRecipe.Modules.empty())
		{
			strOutError =
				"Baked-edge Light target or immutable packet is invalid.";
			return false;
		}
		const Client::EFFECT_VISUAL_PROGRAM_BAKED_EDGE_LIGHT_PACKET& Packet =
			*Supplemental.BakedEdgeLightPacket;
		const auto& Profile = Packet.TargetLight;
		const auto Matches4 = [](const std::array<double, 4u>& Left,
			const float4_t& Right)
		{
			return Nearly_EqualTrailValue(Left[0u], Right.x) &&
				Nearly_EqualTrailValue(Left[1u], Right.y) &&
				Nearly_EqualTrailValue(Left[2u], Right.z) &&
				Nearly_EqualTrailValue(Left[3u], Right.w);
		};
		if (1u != Packet.iPacketVersion ||
			Packet.strAdapterId != Supplemental.strAdapterId ||
			!Packet.bBoundedSemanticReplay || Packet.bNativeExecution ||
			Packet.strRuntimeCarrier !=
				"EFFECT_TYPED_LIGHT_BAKED_EDGE_ATTACHMENT_V1" ||
			Packet.strSourceEventId != Supplemental.strSourceEventId ||
			Packet.strSourceEventRecordSha256.empty() ||
			Packet.strTargetElementId != Target.strElementId ||
			Packet.strTargetElementId !=
				Supplemental.TargetIdentity.strTargetElementId ||
			Packet.strPacketSha256.empty() ||
			Packet.eLane !=
				Client::EFFECT_VISUAL_PROGRAM_BAKED_EDGE_LANE::FIRST_EDGE ||
			!Nearly_EqualTrailValue(Packet.fActiveStartSeconds,
				Supplemental.fLocalTimeSeconds) ||
			!Nearly_EqualTrailValue(Packet.fActiveDurationSeconds,
				Supplemental.fDurationSeconds) ||
			!Nearly_EqualTrailValue(Packet.fActiveEndSeconds,
				Packet.fActiveStartSeconds + Packet.fActiveDurationSeconds) ||
			!Nearly_EqualTrailValue(Packet.fActiveStartSeconds,
				Target.Detail.Timing.fStartDelaySeconds) ||
			!Nearly_EqualTrailValue(Packet.fActiveDurationSeconds,
				Target.Detail.Timing.fLifeTimeSeconds) ||
			!Profile.bEnabled ||
			Profile.strProfileId != "light.point.reconstructed.v1" ||
			Profile.strStatus != "reconstructed_profile" ||
			!Target.Detail.Light.bEnabled ||
			Target.Detail.Light.eProfile !=
				Client::EFFECT_LIGHT_PROFILE::POINT_RECONSTRUCTED_V1 ||
			Target.Detail.Light.eStatus !=
				Client::EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE ||
			!Nearly_EqualTrailValue(Profile.fRange,
				Target.Detail.Light.fRange) ||
			!Nearly_EqualTrailValue(Profile.fIntensity,
				Target.Detail.Light.fIntensity) ||
			!Matches4(Profile.vColor, Target.Detail.Light.vColor) ||
			!Matches4(Profile.vAmbient, Target.Detail.Light.vAmbient) ||
			!Nearly_EqualTrailValue(Profile.fFalloffExponent,
				Target.Detail.Light.fFalloffExponent))
		{
			strOutError =
				"Baked-edge Light packet does not match its projected target.";
			return false;
		}
		return true;
	}

	bool_t Is_SourceVisualDecalParticle(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const bool_t bSourceVisualProgramActive)
	{
		return bSourceVisualProgramActive &&
			Client::EFFECT_ELEMENT_KIND::DECAL == Element.eKind &&
			Element.SourceRecipe.bEnabled &&
			Element.SourceRecipe.strRendererShape == "decal" &&
			std::ranges::any_of(Element.SourceRecipe.Modules,
				[](const Client::EFFECT_SOURCE_MODULE_DESC& Module)
				{
					return SourceClass_Matches(
						Module, "particlemoduletypedatadecal");
				});
	}

	bool_t Is_SourceVisualMeshOrSpriteParticle(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const bool_t bSourceVisualProgramActive)
	{
		if (!bSourceVisualProgramActive || !Element.SourceRecipe.bEnabled)
			return false;
		return (Client::EFFECT_ELEMENT_KIND::MESH == Element.eKind &&
				Element.SourceRecipe.strRendererShape == "mesh") ||
			(Client::EFFECT_ELEMENT_KIND::SPRITE == Element.eKind &&
				Element.SourceRecipe.strRendererShape == "sprite");
	}

	bool_t Is_ParticleSimulationElement(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const bool_t bSourceVisualProgramActive)
	{
		return Client::EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind ||
			Is_SourceVisualMeshOrSpriteParticle(
				Element, bSourceVisualProgramActive) ||
			Is_SourceVisualDecalParticle(
				Element, bSourceVisualProgramActive);
	}

	/* Detail.Particle.SourceScale.fCount applied to a particle count the source
	   produced.  Only source-owned Elements consult it: an authored Element
	   already owns iMaxParticles and iBurstCount outright, so scaling those
	   again would apply the trim twice.  Rounds to nearest so a trim that lands
	   above half still emits, and saturates rather than wrapping. */
	uint32_t Scale_SourceCount(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const uint32_t iCount)
	{
		const f32_t fFactor = Element.Detail.Particle.SourceScale.fCount;
		if (!Element.SourceRecipe.bEnabled || 1.f == fFactor || 0u == iCount)
			return iCount;
		const f64_t fScaled = std::round(
			static_cast<f64_t>(iCount) * static_cast<f64_t>(fFactor));
		return static_cast<uint32_t>((std::min)(
			(std::max)(fScaled, 0.0), static_cast<f64_t>(UINT32_MAX)));
	}

	bool_t Is_GpuVisualOccurrence(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		switch (Element.Renderer.eType)
		{
		case Client::EFFECT_RENDERER_TYPE::STANDALONE_MESH:
		case Client::EFFECT_RENDERER_TYPE::MESH_PARTICLE:
		case Client::EFFECT_RENDERER_TYPE::LEGACY_STANDALONE_SPRITE:
		case Client::EFFECT_RENDERER_TYPE::SPRITE_PARTICLE:
		case Client::EFFECT_RENDERER_TYPE::DECAL_PARTICLE:
		case Client::EFFECT_RENDERER_TYPE::ANIM_TRAIL:
		case Client::EFFECT_RENDERER_TYPE::CASCADE_RIBBON:
			return true;
		case Client::EFFECT_RENDERER_TYPE::LIGHT_PARTICLE:
		case Client::EFFECT_RENDERER_TYPE::SCREEN_POST:
			return false;
		case Client::EFFECT_RENDERER_TYPE::END:
			/* Legacy authored/source-recipe documents up through format v13 do
			   not carry the native-v14 renderer descriptor.  Their drawable
			   Element kind remains the occurrence authority.  Native-v14 and
			   reconstructed Artist documents are validated separately and carry
			   a non-END typed renderer before reaching this compatibility path. */
			switch (Element.eKind)
			{
			case Client::EFFECT_ELEMENT_KIND::MESH:
			case Client::EFFECT_ELEMENT_KIND::SPRITE:
			case Client::EFFECT_ELEMENT_KIND::PARTICLE:
			case Client::EFFECT_ELEMENT_KIND::DECAL:
			case Client::EFFECT_ELEMENT_KIND::TRAIL:
				return true;
			case Client::EFFECT_ELEMENT_KIND::LIGHT:
			case Client::EFFECT_ELEMENT_KIND::SCREEN_POST:
			case Client::EFFECT_ELEMENT_KIND::END:
			default:
				return false;
			}
		default:
			return false;
		}
	}

	const Client::EFFECT_SOURCE_LITERAL_DESC* Find_SourceLiteral(
		const Client::EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view PropertyPath)
	{
		const auto Iterator = std::find_if(
			Module.Literals.begin(), Module.Literals.end(),
			[PropertyPath](const Client::EFFECT_SOURCE_LITERAL_DESC& Literal)
			{
				return Literal.strPropertyPath == PropertyPath;
			});
		return Iterator == Module.Literals.end() ? nullptr : &*Iterator;
	}

	bool_t SourceBool(
		const Client::EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view PropertyPath,
		const bool_t bFallback)
	{
		const Client::EFFECT_SOURCE_LITERAL_DESC* pLiteral =
			Find_SourceLiteral(Module, PropertyPath);
		return nullptr != pLiteral &&
			Client::EFFECT_SOURCE_LITERAL_KIND::BOOLEAN == pLiteral->eKind ?
			pLiteral->bBoolean : bFallback;
	}

	f32_t SourceNumber(
		const Client::EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view PropertyPath,
		const f32_t fFallback)
	{
		const Client::EFFECT_SOURCE_LITERAL_DESC* pLiteral =
			Find_SourceLiteral(Module, PropertyPath);
		return nullptr != pLiteral &&
			Client::EFFECT_SOURCE_LITERAL_KIND::NUMBER == pLiteral->eKind ?
			static_cast<f32_t>(pLiteral->fNumber) : fFallback;
	}

	std::string_view SourceString(
		const Client::EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view PropertyPath)
	{
		const Client::EFFECT_SOURCE_LITERAL_DESC* pLiteral =
			Find_SourceLiteral(Module, PropertyPath);
		return nullptr != pLiteral &&
			Client::EFFECT_SOURCE_LITERAL_KIND::STRING == pLiteral->eKind ?
			std::string_view(pLiteral->strString) : std::string_view{};
	}

	bool_t SourceModule_Enabled(
		const Client::EFFECT_SOURCE_MODULE_DESC& Module)
	{
		return SourceBool(Module, "benabled", true);
	}

	bool_t Is_PortableAuthoredParticleCarrier(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Element.eKind == Client::EFFECT_ELEMENT_KIND::PARTICLE &&
			Element.Renderer.eType == Client::EFFECT_RENDERER_TYPE::END &&
			Element.Renderer.eSourceSpace == Client::EFFECT_SOURCE_SPACE::END &&
			Element.SourceRecipe.bEnabled &&
			(Element.SourceRecipe.strRendererShape == "mesh" ||
			 Element.SourceRecipe.strRendererShape == "sprite");
	}

	bool_t Validate_PortableSourceEventRoutes(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		bool_t& bOutHasEvents,
		std::string& strOutError)
	{
		bOutHasEvents = false;
		std::unordered_map<std::string, std::vector<std::string>> Generators;
		std::unordered_map<std::string, std::vector<std::string>> Receivers;
		uint64_t iMaximumQueuedEvents = 0u;
		const auto RouteKey = [](const std::string_view Type,
			const std::string_view Name)
		{
			std::string Result(Type);
			Result.push_back('\0');
			Result.append(Name);
			return Result;
		};
		for (const Client::EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (!Element.bVisible ||
				!Client::Is_EffectAuthoringExecutionTarget(
					Element.Material.Execution) ||
				!Is_PortableAuthoredParticleCarrier(Element))
			{
				continue;
			}
			uint32_t iGeneratorCount = 0u;
			for (const Client::EFFECT_SOURCE_MODULE_DESC& Module :
				Element.SourceRecipe.Modules)
			{
				if (!SourceModule_Enabled(Module))
					continue;
				if (SourceClass_Matches(Module,
					"particlemoduleeventgenerator"))
				{
					const std::string_view Type =
						SourceString(Module, "events[0].type");
					const std::string_view Name =
						SourceString(Module, "events[0].customname");
					if (Type != "epet_spawn" || Name.empty())
					{
						strOutError =
							"Portable source event generator route is invalid: " +
							Element.strElementId + ".";
						return false;
					}
					Generators[RouteKey(Type, Name)].push_back(
						Element.strElementId);
					++iGeneratorCount;
					bOutHasEvents = true;
				}
				else if (SourceClass_Matches(Module,
					"particlemoduleeventreceiverspawn"))
				{
					const std::string_view Type =
						SourceString(Module, "eventgeneratortype");
					const std::string_view Name =
						SourceString(Module, "eventname");
					if (Type != "epet_spawn" || Name.empty())
					{
						strOutError =
							"Portable source event receiver route is invalid: " +
							Element.strElementId + ".";
						return false;
					}
					Receivers[RouteKey(Type, Name)].push_back(
						Element.strElementId);
					bOutHasEvents = true;
				}
			}
			iMaximumQueuedEvents +=
				static_cast<uint64_t>(Element.Detail.Particle.iMaxParticles) *
				static_cast<uint64_t>(iGeneratorCount);
		}
		if (!bOutHasEvents)
		{
			strOutError.clear();
			return true;
		}
		if (iMaximumQueuedEvents > MAX_SOURCE_EVENTS_PER_STEP)
		{
			strOutError =
				"Portable source event queue has an unbounded per-step upper limit.";
			return false;
		}
		for (const auto& [Route, Elements] : Generators)
		{
			UNREFERENCED_PARAMETER(Elements);
			if (!Receivers.contains(Route))
			{
				strOutError =
					"Portable source event generator has no same-document receiver.";
				return false;
			}
		}
		for (const auto& [Route, Elements] : Receivers)
		{
			UNREFERENCED_PARAMETER(Elements);
			if (!Generators.contains(Route))
			{
				strOutError =
					"Portable source event receiver has no same-document generator.";
				return false;
			}
		}

		std::unordered_map<std::string, std::vector<std::string>> Edges;
		for (const auto& [Route, SourceElements] : Generators)
		{
			const std::vector<std::string>& TargetElements = Receivers.at(Route);
			for (const std::string& SourceElement : SourceElements)
			{
				auto& Targets = Edges[SourceElement];
				Targets.insert(Targets.end(), TargetElements.begin(),
					TargetElements.end());
			}
		}
		std::unordered_map<std::string, uint8_t> Visit;
		const auto VisitNode = [&Edges, &Visit](
			const auto& Self, const std::string& Node) -> bool_t
		{
			uint8_t& State = Visit[Node];
			if (State == 1u)
				return false;
			if (State == 2u)
				return true;
			State = 1u;
			const auto Iterator = Edges.find(Node);
			if (Iterator != Edges.end())
			{
				for (const std::string& Target : Iterator->second)
				{
					if (!Self(Self, Target))
						return false;
				}
			}
			State = 2u;
			return true;
		};
		for (const auto& [Node, Targets] : Edges)
		{
			UNREFERENCED_PARAMETER(Targets);
			if (!VisitNode(VisitNode, Node))
			{
				strOutError =
					"Portable source event route graph contains a cycle.";
				return false;
			}
		}
		strOutError.clear();
		return true;
	}

	void Resolve_SourceSpritePresentation(
		const Client::EFFECT_ELEMENT_DESC& Element,
		Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT& eOutAlignment,
		float2_t& vOutPivot,
		bool_t& bOutAllowImageFlipping)
	{
		eOutAlignment = Element.SourceRecipe.bEnabled ?
			Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::CAMERA_SQUARE :
			Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::CAMERA_RECTANGLE;
		vOutPivot = { 0.5f, 0.5f };
		bOutAllowImageFlipping = false;
		for (const Client::EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (!SourceModule_Enabled(Module))
				continue;
			if (SourceClass_Matches(Module, "particlemodulerequired"))
			{
				const std::string_view Alignment =
					SourceString(Module, "screenalignment");
				if (Alignment.ends_with("psa_rectangle"))
				{
					eOutAlignment = Client::
						EFFECT_PARTICLE_SPRITE_ALIGNMENT::CAMERA_RECTANGLE;
				}
				else if (Alignment.ends_with("psa_velocity"))
				{
					eOutAlignment = Client::
						EFFECT_PARTICLE_SPRITE_ALIGNMENT::CAMERA_VELOCITY;
				}
				else if (Alignment.ends_with("psa_square"))
				{
					eOutAlignment = Client::
						EFFECT_PARTICLE_SPRITE_ALIGNMENT::CAMERA_SQUARE;
				}
				if (SourceBool(Module, "boffsetcenter", false))
				{
					vOutPivot.x = SourceNumber(
						Module, "offsetcenterx", 0.5f);
					vOutPivot.y = SourceNumber(
						Module, "offsetcentery", 0.5f);
				}
				bOutAllowImageFlipping = SourceBool(
					Module, "ballowimageflipping", false);
			}
			else if (SourceClass_Matches(
				Module, "particlemoduleorientationaxislock"))
			{
				const std::string_view Axis =
					SourceString(Module, "lockaxisflags");
				if (Axis.ends_with("epal_negative_x"))
					eOutAlignment = Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_NEGATIVE_X;
				else if (Axis.ends_with("epal_negative_y"))
					eOutAlignment = Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_POSITIVE_Z;
				else if (Axis.ends_with("epal_negative_z"))
					eOutAlignment = Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_NEGATIVE_Y;
				else if (Axis.ends_with("epal_rotate_x"))
					eOutAlignment = Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::ROTATE_X;
				else if (Axis.ends_with("epal_rotate_y"))
					eOutAlignment = Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::ROTATE_Z;
				else if (Axis.ends_with("epal_rotate_z"))
					eOutAlignment = Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::ROTATE_Y;
				else if (Axis.ends_with("epal_x"))
					eOutAlignment = Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_POSITIVE_X;
				else if (Axis.ends_with("epal_y"))
					eOutAlignment = Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_NEGATIVE_Z;
				else if (Axis.ends_with("epal_z"))
					eOutAlignment = Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_POSITIVE_Y;
			}
		}
	}

	f32_t UE_RandomFraction(uint32_t& iSeed)
	{
		iSeed = iSeed * 196314165u + 907633515u;
		const uint32_t iBits = 0x3f800000u | (iSeed >> 9u);
		return std::bit_cast<f32_t>(iBits) - 1.f;
	}

	float3_t Transform_Coord(
		const float3_t& Value,
		const matrix_t& Matrix)
	{
		float3_t Result{};
		XMStoreFloat3(&Result,
			XMVector3TransformCoord(XMLoadFloat3(&Value), Matrix));
		return Result;
	}

	float3_t Transform_Normal(
		const float3_t& Value,
		const matrix_t& Matrix)
	{
		float3_t Result{};
		XMStoreFloat3(&Result,
			XMVector3TransformNormal(XMLoadFloat3(&Value), Matrix));
		return Result;
	}

	std::shared_ptr<const SOURCE_VECTOR_FIELD> Load_VectorFieldFromDisk(
		const std::string_view AssetId)
	{
		if (AssetId.empty() || !AssetId.starts_with("Effect/") ||
			AssetId.find("..") != std::string_view::npos ||
			!AssetId.ends_with(".wvectorfield"))
		{
			return nullptr;
		}
		const std::string Key(AssetId);
		const std::filesystem::path Path =
			Client::CRuntimeAssetRoot::Resolve(std::filesystem::path(Key));
		g_iVectorFieldDiskLoadCount.fetch_add(1u, std::memory_order_relaxed);
		std::ifstream Input(Path, std::ios::binary | std::ios::ate);
		if (!Input)
			return nullptr;
		const std::streamsize iFileSize = Input.tellg();
		Input.seekg(0, std::ios::beg);
		VECTOR_FIELD_HEADER Header{};
		if (iFileSize < static_cast<std::streamsize>(sizeof(Header)) ||
			!Input.read(reinterpret_cast<char*>(&Header), sizeof(Header)) ||
			0 != std::memcmp(Header.Magic, "WVF1", 4u) ||
			1u != Header.iVersion || 0u == Header.iSizeX ||
			0u == Header.iSizeY || 0u == Header.iSizeZ ||
			Header.iSampleCount != Header.iSizeX * Header.iSizeY * Header.iSizeZ)
		{
			return nullptr;
		}
		const uint64_t iExpected = sizeof(Header) +
			static_cast<uint64_t>(Header.iSampleCount) * sizeof(float3_t);
		if (static_cast<uint64_t>(iFileSize) != iExpected)
			return nullptr;
		auto Field = std::make_shared<SOURCE_VECTOR_FIELD>();
		Field->iSizeX = Header.iSizeX;
		Field->iSizeY = Header.iSizeY;
		Field->iSizeZ = Header.iSizeZ;
		Field->Samples.resize(Header.iSampleCount);
		if (!Input.read(reinterpret_cast<char*>(Field->Samples.data()),
			static_cast<std::streamsize>(Header.iSampleCount * sizeof(float3_t))))
		{
			return nullptr;
		}
		for (const float3_t& Value : Field->Samples)
		{
			if (!std::isfinite(Value.x) || !std::isfinite(Value.y) ||
				!std::isfinite(Value.z))
			{
				return nullptr;
			}
		}
		return Field;
	}

	float3_t Sample_VectorField(
		const SOURCE_VECTOR_FIELD& Field,
		const float3_t& UV,
		const float3_t& Tiling)
	{
		const float3_t Size(
			static_cast<f32_t>(Field.iSizeX),
			static_cast<f32_t>(Field.iSizeY),
			static_cast<f32_t>(Field.iSizeZ));
		float3_t SamplePosition(
			UV.x * Size.x - 0.5f,
			UV.y * Size.y - 0.5f,
			UV.z * Size.z - 0.5f);
		float3_t Index0(
			std::floor(SamplePosition.x),
			std::floor(SamplePosition.y),
			std::floor(SamplePosition.z));
		float3_t Index1 = Add3(Index0, float3_t(1.f, 1.f, 1.f));
		const float3_t Fraction = Subtract3(SamplePosition, Index0);
		auto ResolveIndex = [](f32_t Value, const uint32_t iSize,
			const bool_t bTile) -> uint32_t
		{
			if (bTile)
			{
				Value -= std::floor(Value / static_cast<f32_t>(iSize)) *
					static_cast<f32_t>(iSize);
			}
			return static_cast<uint32_t>(std::clamp(
				Value, 0.f, static_cast<f32_t>(iSize - 1u)));
		};
		const uint32_t X0 = ResolveIndex(Index0.x, Field.iSizeX, Tiling.x > 0.f);
		const uint32_t X1 = ResolveIndex(Index1.x, Field.iSizeX, Tiling.x > 0.f);
		const uint32_t Y0 = ResolveIndex(Index0.y, Field.iSizeY, Tiling.y > 0.f);
		const uint32_t Y1 = ResolveIndex(Index1.y, Field.iSizeY, Tiling.y > 0.f);
		const uint32_t Z0 = ResolveIndex(Index0.z, Field.iSizeZ, Tiling.z > 0.f);
		const uint32_t Z1 = ResolveIndex(Index1.z, Field.iSizeZ, Tiling.z > 0.f);
		auto At = [&Field](const uint32_t X, const uint32_t Y,
			const uint32_t Z) -> const float3_t&
		{
			return Field.Samples[X + Field.iSizeX * (Y + Field.iSizeY * Z)];
		};
		const float3_t V00 = Lerp3(At(X0, Y0, Z0), At(X1, Y0, Z0), Fraction.x);
		const float3_t V10 = Lerp3(At(X0, Y1, Z0), At(X1, Y1, Z0), Fraction.x);
		const float3_t V01 = Lerp3(At(X0, Y0, Z1), At(X1, Y0, Z1), Fraction.x);
		const float3_t V11 = Lerp3(At(X0, Y1, Z1), At(X1, Y1, Z1), Fraction.x);
		return Lerp3(
			Lerp3(V00, V10, Fraction.y),
			Lerp3(V01, V11, Fraction.y), Fraction.z);
	}

	bool_t Reject_ReconstructedSourceRuntime(
		std::string& strOutError,
		const std::string& strReason)
	{
		strOutError = "Reconstructed source runtime rejected: " + strReason;
		return false;
	}

	bool_t Equals_NoCaseAscii(
		const std::string_view Left,
		const std::string_view Right)
	{
		if (Left.size() != Right.size())
			return false;
		for (size_t Index = 0u; Index < Left.size(); ++Index)
		{
			const auto Lower = [](const char_t Value)
			{
				return Value >= 'A' && Value <= 'Z' ?
					static_cast<char_t>(Value - 'A' + 'a') : Value;
			};
			if (Lower(Left[Index]) != Lower(Right[Index]))
				return false;
		}
		return true;
	}

	bool_t Validate_ReconstructedSourceLiteral(
		const Client::EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view PropertyPath,
		const Client::EFFECT_SOURCE_LITERAL_KIND ExpectedKind,
		std::string& strOutError)
	{
		const size_t Count = std::ranges::count_if(Module.Literals,
			[PropertyPath](const Client::EFFECT_SOURCE_LITERAL_DESC& Literal)
			{
				return Literal.strPropertyPath == PropertyPath;
			});
		if (Count > 1u)
		{
			strOutError = "duplicate source literal: " +
				std::string(PropertyPath);
			return false;
		}
		const Client::EFFECT_SOURCE_LITERAL_DESC* pLiteral =
			Find_SourceLiteral(Module, PropertyPath);
		if (nullptr == pLiteral)
			return true;
		if (pLiteral->eKind != ExpectedKind)
		{
			strOutError = "source literal kind mismatches: " +
				std::string(PropertyPath);
			return false;
		}
		if (ExpectedKind == Client::EFFECT_SOURCE_LITERAL_KIND::NUMBER &&
			!std::isfinite(pLiteral->fNumber))
		{
			strOutError = "source numeric literal is non-finite: " +
				std::string(PropertyPath);
			return false;
		}
		return true;
	}

	bool_t Validate_ReconstructedSourceDistribution(
		const Client::EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view PropertyPath,
		const uint32_t iExpectedComponentCount,
		std::string& strOutError)
	{
		if (1u != std::ranges::count_if(Module.Distributions,
			[PropertyPath](const Client::EFFECT_DISTRIBUTION_DESC& Distribution)
			{
				return Distribution.strPropertyPath == PropertyPath;
			}))
		{
			strOutError = "source distribution is missing or duplicate: " +
				std::string(PropertyPath);
			return false;
		}
		const Client::EFFECT_DISTRIBUTION_DESC* pDistribution =
			Find_SourceDistribution(&Module, PropertyPath);
		if (nullptr == pDistribution ||
			pDistribution->iComponentCount != iExpectedComponentCount)
		{
			strOutError = "source distribution component count mismatches: " +
				std::string(PropertyPath);
			return false;
		}
		std::string DistributionError;
		if (!Client::CEffectDistribution::Validate(
				*pDistribution, DistributionError))
		{
			strOutError = "source distribution is malformed: " +
				std::string(PropertyPath) + ": " + DistributionError;
			return false;
		}
		return true;
	}

	bool_t Validate_ReconstructedCircleSurface(
		const Client::EFFECT_SOURCE_MODULE_DESC& Module,
		std::string& strOutError)
	{
		using namespace Client;
		if (Module.Distributions.size() != 4u)
		{
			strOutError =
				"CircleSurface has an unconsumed or missing distribution.";
			return false;
		}
		for (const std::string_view BooleanPath : {
				std::string_view("benabled"), std::string_view("bhalfmode"),
				std::string_view("bnegativeaxis"), std::string_view("velocity") })
		{
			if (!Validate_ReconstructedSourceLiteral(Module, BooleanPath,
					EFFECT_SOURCE_LITERAL_KIND::BOOLEAN, strOutError))
				return false;
		}
		if (!Validate_ReconstructedSourceLiteral(Module, "splitcirclecount",
				EFFECT_SOURCE_LITERAL_KIND::NUMBER, strOutError) ||
			!Validate_ReconstructedSourceLiteral(Module, "surfaceaxis",
				EFFECT_SOURCE_LITERAL_KIND::STRING, strOutError))
		{
			return false;
		}
		const EFFECT_SOURCE_LITERAL_DESC* pSplit =
			Find_SourceLiteral(Module, "splitcirclecount");
		if (nullptr != pSplit && (pSplit->fNumber < 0.0 ||
			pSplit->fNumber > 4096.0 ||
			std::floor(pSplit->fNumber) != pSplit->fNumber))
		{
			strOutError =
				"CircleSurface splitCircleCount must be an integer in [0,4096].";
			return false;
		}
		const std::string_view SurfaceAxis =
			SourceString(Module, "surfaceaxis");
		if (!SurfaceAxis.empty() &&
			SurfaceAxis != "pmlcs_circle_axis_xy" &&
			SurfaceAxis != "pmlcs_circle_axis_yz" &&
			SurfaceAxis != "pmlcs_circle_axis_zx")
		{
			strOutError = "CircleSurface surfaceAxis is unsupported.";
			return false;
		}
		return Validate_ReconstructedSourceDistribution(
				Module, "startlocation", 3u, strOutError) &&
			Validate_ReconstructedSourceDistribution(
				Module, "startradius", 1u, strOutError) &&
			Validate_ReconstructedSourceDistribution(
				Module, "startrot", 1u, strOutError) &&
			Validate_ReconstructedSourceDistribution(
				Module, "velocityscale", 1u, strOutError);
	}

	bool_t Validate_ReconstructedVortex(
		const Client::EFFECT_SOURCE_MODULE_DESC& Module,
		std::string& strOutError)
	{
		using namespace Client;
		if (Module.Distributions.size() != 1u)
		{
			strOutError = "Vortex has an unconsumed or missing distribution.";
			return false;
		}
		return Validate_ReconstructedSourceLiteral(Module, "benabled",
				EFFECT_SOURCE_LITERAL_KIND::BOOLEAN, strOutError) &&
			Validate_ReconstructedSourceLiteral(Module, "power",
				EFFECT_SOURCE_LITERAL_KIND::NUMBER, strOutError) &&
			Validate_ReconstructedSourceDistribution(
				Module, "poweracceleration", 1u, strOutError);
	}

	bool_t Validate_ReconstructedSourceModuleExecutionContract(
		const Client::EFFECT_SOURCE_MODULE_DESC& Module,
		std::string& strOutError)
	{
		strOutError.clear();
		static constexpr std::array<std::string_view, 34u> Supported = {
			"particlemoduleacceleration",
			"particlemodulecameraoffset",
			"particlemodulecolor",
			"particlemodulecoloroverlife",
			"particlemodulecolorscaleoverlife",
			"particlemodulelifetime",
			"particlemodulelocation",
			"particlemodulelocationcirclesurface",
			"particlemodulelocationdirect",
			"particlemodulelocationonground",
			"particlemodulelocationprimitivecylinder",
			"particlemodulelocationprimitivecylinderspin",
			"particlemodulelocationprimitivesphere",
			"particlemodulemeshrotation",
			"particlemodulemeshrotationrate",
			"particlemodulemeshrotationratemultiplylife",
			"particlemodulemeshrotationrateoverlife",
			"particlemoduleorientationaxislock",
			"particlemoduleparameterdynamic",
			"particlemodulerequired",
			"particlemodulerotation",
			"particlemodulerotationrate",
			"particlemodulesize",
			"particlemodulesizemultiplylife",
			"particlemodulespawn",
			"particlemodulespawnperunit",
			"particlemodulesubuv",
			"particlemoduletypedatadecal",
			"particlemoduletypedatalight",
			"particlemoduletypedatamesh",
			"particlemoduletypedataribbon",
			"particlemodulevelocity",
			"particlemodulevelocityoverlifetime",
			"particlemodulevortex"
		};
		const bool_t bSupported = std::ranges::any_of(Supported,
			[&Module](const std::string_view ClassName)
			{
				return SourceClass_Matches(Module, ClassName);
			});
		if (!bSupported)
		{
			strOutError = "source module class is not in the strict executor allowlist";
			return false;
		}
		if (SourceClass_Matches(Module,
				"particlemodulelocationcirclesurface"))
		{
			return Validate_ReconstructedCircleSurface(Module, strOutError);
		}
		if (SourceClass_Matches(Module, "particlemodulevortex"))
			return Validate_ReconstructedVortex(Module, strOutError);
		strOutError.clear();
		return true;
	}

	bool_t Try_ResolveReconstructedParameter(
		const Client::EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION& Source,
		std::array<double, 4u>& OutValue,
		std::string& strOutError)
	{
		const uint32_t ComponentCount = Source.iComponentCount;
		if (0u == ComponentCount || ComponentCount > OutValue.size() ||
			Source.ParamModes.size() != ComponentCount ||
			Source.MinimumInput.size() != ComponentCount ||
			Source.MaximumInput.size() != ComponentCount ||
			Source.MinimumOutput.size() != ComponentCount ||
			Source.MaximumOutput.size() != ComponentCount ||
			Source.ConstantValues.size() != ComponentCount)
		{
			return Reject_ReconstructedSourceRuntime(strOutError,
				"parameter distribution shape is not exhaustive");
		}

		std::optional<std::vector<double>> BoundInput;
		for (const Client::EFFECT_RUNTIME_PROGRAM_ACTION_CUE_VALUE& Binding :
			Source.ActionCueBindings)
		{
			if (!Equals_NoCaseAscii(Binding.strName, Source.strParameterName))
				continue;
			if (1u == ComponentCount &&
				Client::EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::SCALAR == Binding.eKind &&
				Binding.fScalarValue.has_value())
			{
				BoundInput = std::vector<double>{ *Binding.fScalarValue };
				break;
			}
			if (ComponentCount > 1u &&
				Client::EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::VECTOR == Binding.eKind &&
				Binding.VectorValue.size() >= ComponentCount)
			{
				BoundInput = std::vector<double>(Binding.VectorValue.begin(),
					Binding.VectorValue.begin() + ComponentCount);
				break;
			}
			/* A typed cue with the same name but the wrong scalar/vector kind is
			   not an executable binding.  The approved Artist F source contract
			   classifies this branch as CONSTANT_FALLBACK. */
			continue;
		}

		OutValue.fill(0.0);
		for (uint32_t Index = 0u; Index < ComponentCount; ++Index)
		{
			if (!BoundInput.has_value())
			{
				OutValue[Index] = Source.ConstantValues[Index];
			}
			else if ("dpm_direct" == Source.ParamModes[Index])
			{
				OutValue[Index] = (*BoundInput)[Index];
			}
			else if ("dpm_normal" == Source.ParamModes[Index])
			{
				const double InputRange = Source.MaximumInput[Index] -
					Source.MinimumInput[Index];
				const double Ratio = 0.0 == InputRange ? 0.0 : std::clamp(
					((*BoundInput)[Index] - Source.MinimumInput[Index]) / InputRange,
					0.0, 1.0);
				OutValue[Index] = std::lerp(Source.MinimumOutput[Index],
					Source.MaximumOutput[Index], Ratio);
			}
			else
			{
				return Reject_ReconstructedSourceRuntime(strOutError,
					"parameter mode is unsupported");
			}
			if (!std::isfinite(OutValue[Index]) ||
				std::abs(OutValue[Index]) > FLT_MAX)
			{
				return Reject_ReconstructedSourceRuntime(strOutError,
					"parameter result is not a finite f32 value");
			}
		}
		return true;
	}

	bool_t Overlay_ReconstructedParameterDistribution(
		const Client::EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION& Source,
		Client::EFFECT_DISTRIBUTION_DESC& Destination,
		std::string& strOutError)
	{
		std::array<double, 4u> Resolved{};
		if (!Try_ResolveReconstructedParameter(Source, Resolved, strOutError))
			return false;
		float4_t Value{};
		for (uint32_t Index = 0u; Index < Source.iComponentCount; ++Index)
			(&Value.x)[Index] = static_cast<f32_t>(Resolved[Index]);

		Destination.iOperation = 1u;
		Destination.iRandomLockAxes = 0u;
		Destination.iLookupTableChunkSize = 0u;
		Destination.iLookupTableNumElements = 0u;
		Destination.fLookupTableTimeScale = 0.f;
		Destination.fLookupTableStartTime = 0.f;
		Destination.vDefaultMinimum = Value;
		Destination.vDefaultMaximum = Value;
		Destination.LookupTable.clear();
		Destination.Keys.clear();
		return true;
	}

	bool_t Build_ReconstructedSourceRuntimeDocument(
		const Client::EFFECT_DOCUMENT_DESC& SourceDocument,
		const Client::EFFECT_RECONSTRUCTED_EXECUTION_PLAN& Plan,
		Client::EFFECT_DOCUMENT_DESC& OutDocument,
		std::string& strOutError)
	{
		constexpr uint32_t EXPECTED_SCHEDULES = 7u;
		constexpr uint32_t EXPECTED_EMITTERS = 35u;
		constexpr uint32_t EXPECTED_MODULES = 399u;
		constexpr uint32_t EXPECTED_DISTRIBUTIONS = 629u;
		const auto Program = Plan.Get_Program();
		const auto& Summary = Plan.Get_Summary();
		if (nullptr == Program || Program->strCharacterClass != "ARTIST" ||
			Program->iSkillId != 31470u || Program->strInputSlot != "F" ||
			Program->Admission.bRuntimeExecution || Program->Admission.bProduct ||
			Summary.iScheduleCount != EXPECTED_SCHEDULES ||
			Summary.iEmitterCount != EXPECTED_EMITTERS ||
			Summary.iModuleCount != EXPECTED_MODULES ||
			Summary.iDistributionCount != EXPECTED_DISTRIBUTIONS ||
			SourceDocument.Elements.size() != EXPECTED_EMITTERS)
		{
			return Reject_ReconstructedSourceRuntime(strOutError,
				"only the frozen non-Product Artist 31470 F program is supported");
		}

		std::unordered_map<std::string,
			const Client::EFFECT_RUNTIME_PROGRAM_EMITTER*> ProgramEmitters;
		for (const Client::EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter :
			Program->Emitters)
		{
			if (Emitter.strSourceElementId.empty() ||
				!ProgramEmitters.emplace(Emitter.strSourceElementId, &Emitter).second)
			{
				return Reject_ReconstructedSourceRuntime(strOutError,
					"program source element identity is duplicate or empty");
			}
		}
		std::unordered_map<std::string,
			const Client::EFFECT_RUNTIME_PROGRAM_DISTRIBUTION*>
			ProgramDistributions;
		ProgramDistributions.reserve(Program->Distributions.size());
		for (const Client::EFFECT_RUNTIME_PROGRAM_DISTRIBUTION& Distribution :
			Program->Distributions)
		{
			if (Distribution.Row.strId.empty() ||
				!ProgramDistributions.emplace(
					Distribution.Row.strId, &Distribution).second)
			{
				return Reject_ReconstructedSourceRuntime(strOutError,
					"program distribution identity is duplicate or empty");
			}
		}

		OutDocument = SourceDocument;
		uint32_t ModuleCount = 0u;
		uint32_t DistributionCount = 0u;
		uint32_t GroundModuleCount = 0u;
		uint32_t SpawnPerUnitModuleCount = 0u;
		uint32_t ParameterDistributionCount = 0u;
		uint32_t FloatCurveCount = 0u;
		std::unordered_set<std::string> ConsumedEmitterIds;
		for (Client::EFFECT_ELEMENT_DESC& Element : OutDocument.Elements)
		{
			const auto ProgramEmitterIt = ProgramEmitters.find(Element.strElementId);
			if (ProgramEmitterIt == ProgramEmitters.end() ||
				!Element.SourceRecipe.bEnabled)
			{
				return Reject_ReconstructedSourceRuntime(strOutError,
					"document element is not an aligned source-recipe emitter");
			}
			const Client::EFFECT_RUNTIME_PROGRAM_EMITTER& ProgramEmitter =
				*ProgramEmitterIt->second;
			const auto PlanEmitterIt = Plan.Get_Emitters().find(
				ProgramEmitter.Row.strId);
			if (PlanEmitterIt == Plan.Get_Emitters().end() ||
				!ConsumedEmitterIds.emplace(ProgramEmitter.Row.strId).second ||
				PlanEmitterIt->second.ModuleIds.size() !=
					Element.SourceRecipe.Modules.size())
			{
				return Reject_ReconstructedSourceRuntime(strOutError,
					"document/program emitter module order is not closed");
			}

			for (size_t ModuleIndex = 0u;
				ModuleIndex < Element.SourceRecipe.Modules.size(); ++ModuleIndex)
			{
				Client::EFFECT_SOURCE_MODULE_DESC& SourceModule =
					Element.SourceRecipe.Modules[ModuleIndex];
				const std::string& ModuleId =
					PlanEmitterIt->second.ModuleIds[ModuleIndex];
				const auto PlanModuleIt = Plan.Get_Modules().find(ModuleId);
				if (PlanModuleIt == Plan.Get_Modules().end() ||
					PlanModuleIt->second.strExactSourceClass !=
						SourceModule.strClassName ||
					PlanModuleIt->second.DistributionIds.size() !=
						SourceModule.Distributions.size())
				{
					return Reject_ReconstructedSourceRuntime(strOutError,
						"source module class or distribution order is unsupported");
				}
				std::string ModuleExecutionError;
				if (!Validate_ReconstructedSourceModuleExecutionContract(
						SourceModule, ModuleExecutionError))
				{
					return Reject_ReconstructedSourceRuntime(strOutError,
						"source module execution contract is unsupported: " +
						ModuleExecutionError);
				}
				++ModuleCount;
				if (SourceClass_Matches(
					SourceModule, "particlemodulelocationonground"))
				{
					++GroundModuleCount;
				}
				if (SourceClass_Matches(
					SourceModule, "particlemodulespawnperunit"))
				{
					const f32_t fUnitScalar = SourceNumber(
						SourceModule, "unitscalar", 0.f);
					if (!std::isfinite(fUnitScalar) || fUnitScalar <= 0.f)
					{
						return Reject_ReconstructedSourceRuntime(strOutError,
							"SpawnPerUnit unitScalar must be finite and positive");
					}
					++SpawnPerUnitModuleCount;
				}

				std::unordered_map<std::string,
					const Client::EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION*>
					PlanDistributionsByPropertyPath;
				PlanDistributionsByPropertyPath.reserve(
					PlanModuleIt->second.DistributionIds.size());
				for (const std::string& DistributionId :
					PlanModuleIt->second.DistributionIds)
				{
					const auto PlanDistributionIt =
						Plan.Get_Distributions().find(DistributionId);
					const auto ProgramDistributionIt =
						ProgramDistributions.find(DistributionId);
					if (PlanDistributionIt == Plan.Get_Distributions().end() ||
						ProgramDistributionIt == ProgramDistributions.end() ||
						ProgramDistributionIt->second->strModuleId != ModuleId ||
						ProgramDistributionIt->second->strPropertyPath.empty() ||
						PlanDistributionIt->second.strPropertyId.empty() ||
						PlanDistributionIt->second.strPropertyId !=
							ProgramDistributionIt->second->strPropertyId ||
						!PlanDistributionsByPropertyPath.emplace(
							ProgramDistributionIt->second->strPropertyPath,
							&PlanDistributionIt->second).second)
					{
						return Reject_ReconstructedSourceRuntime(strOutError,
							"compiled module distribution property path is missing or duplicate");
					}
				}
				std::unordered_set<std::string> CoveredDistributionPropertyPaths;
				for (Client::EFFECT_DISTRIBUTION_DESC& SourceDistribution :
					SourceModule.Distributions)
				{
					const auto PlanDistributionIt =
						PlanDistributionsByPropertyPath.find(
							SourceDistribution.strPropertyPath);
					if (PlanDistributionIt ==
							PlanDistributionsByPropertyPath.end() ||
						!CoveredDistributionPropertyPaths.emplace(
							SourceDistribution.strPropertyPath).second)
					{
						return Reject_ReconstructedSourceRuntime(strOutError,
							"source distribution property path is missing or duplicate");
					}
					const auto& PlanDistribution = *PlanDistributionIt->second;
					const auto Variant = PlanDistribution.eVariant;
					if (Client::EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_PARAMETER ==
							Variant ||
						Client::EFFECT_RUNTIME_DISTRIBUTION_VARIANT::VECTOR_PARAMETER ==
							Variant ||
						Client::EFFECT_RUNTIME_DISTRIBUTION_VARIANT::EF_MULTIPLY == Variant)
					{
						if (!Overlay_ReconstructedParameterDistribution(
							PlanDistribution, SourceDistribution,
							strOutError))
						{
							return false;
						}
						++ParameterDistributionCount;
					}
					else if (Client::EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_CURVE ==
						Variant)
					{
						++FloatCurveCount;
					}
					++DistributionCount;
				}
				if (CoveredDistributionPropertyPaths.size() !=
					PlanDistributionsByPropertyPath.size())
				{
					return Reject_ReconstructedSourceRuntime(strOutError,
						"source distribution property membership is not exhaustive");
				}
			}
		}

		if (ConsumedEmitterIds.size() != EXPECTED_EMITTERS ||
			ModuleCount != EXPECTED_MODULES ||
			DistributionCount != EXPECTED_DISTRIBUTIONS ||
			GroundModuleCount != 2u || SpawnPerUnitModuleCount != 2u ||
			ParameterDistributionCount != 16u || FloatCurveCount != 1u)
		{
			return Reject_ReconstructedSourceRuntime(strOutError,
				"35/399/629 source coverage or special handler counts changed");
		}
		strOutError.clear();
		return true;
	}

}

bool_t Client::CEffectPlayback::Validate_ReconstructedSourceModuleExecution(
	const EFFECT_SOURCE_MODULE_DESC& Module,
	std::string& strOutError)
{
	return Validate_ReconstructedSourceModuleExecutionContract(
		Module, strOutError);
}

struct Client::CEffectPlayback::PREPARED_RESOURCES final
{
	std::string strEffectAssetId;
	std::string strDocumentCanonicalSha256;
	std::shared_ptr<const EFFECT_DOCUMENT_DESC> pImmutableDocument;
	std::unordered_map<std::string, std::shared_ptr<const SOURCE_VECTOR_FIELD>>
		VectorFields;
};

const Client::EFFECT_DOCUMENT_DESC&
Client::CEffectPlayback::Get_StagedDocument() const
{
	return nullptr != m_pPreparedResources &&
		nullptr != m_pPreparedResources->pImmutableDocument ?
		*m_pPreparedResources->pImmutableDocument : m_Document;
}

uint64_t Client::CEffectPlayback::Get_VectorFieldDiskLoadCount()
{
	return g_iVectorFieldDiskLoadCount.load(std::memory_order_relaxed);
}

bool_t Client::CEffectPlayback::Is_SourceVisualProgramElementAdmitted(
	const EFFECT_ELEMENT_DESC& Element) const
{
	return m_bSourceVisualProgramActive &&
		(!m_bRequireSourceVisualTargetClosure ||
			m_SourceVisualTargetElementIds.contains(Element.strElementId));
}

bool_t Client::CEffectPlayback::Is_PlaybackElementAdmitted(
	const EFFECT_ELEMENT_DESC& Element) const
{
	if (!Element.bVisible)
		return false;
	if (Is_EffectAuthoringExecutionTarget(Element.Material.Execution))
		return true;
	return Is_EffectFailClosedSourceGeometryCarrier(Element) &&
		Is_SourceVisualProgramElementAdmitted(Element);
}

bool_t Client::CEffectPlayback::Stage_Document(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	if (!CEffectDocumentCodec::Validate(Document, strOutError))
		return false;
	std::shared_ptr<const PREPARED_RESOURCES> Prepared;
	if (!Prepare_DocumentResources(Document, Prepared, strOutError))
		return false;
	return Stage_PrevalidatedDocument(
		Document, std::move(Prepared), strOutError);
}

bool_t Client::CEffectPlayback::Prepare_DocumentResources(
	const EFFECT_DOCUMENT_DESC& Document,
	std::shared_ptr<const PREPARED_RESOURCES>& OutPrepared,
	std::string& strOutError,
	std::shared_ptr<const EFFECT_DOCUMENT_DESC> pImmutableDocument)
{
	if (nullptr != pImmutableDocument &&
		pImmutableDocument.get() != &Document)
	{
		strOutError =
			"Prepared Effect immutable document identity does not match.";
		return false;
	}
	auto Staged = std::make_shared<PREPARED_RESOURCES>();
	Staged->strEffectAssetId = Document.strEffectAssetId;
	Staged->strDocumentCanonicalSha256 =
		CEffectVisualProgramCorpusCodec::Compute_DocumentCanonicalSha256(
			Document, strOutError);
	if (Staged->strDocumentCanonicalSha256.empty())
	{
		if (strOutError.empty())
			strOutError =
				"Prepared Effect document canonical identity is unavailable.";
		return false;
	}
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (Element.Material.Execution.bFailClosed &&
			!Element.Material.Execution.bAuthoringApproximate)
		{
			continue;
		}
		for (const EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (!SourceClass_Matches(Module, "particlemodulelocalvectorfield"))
				continue;
			const std::string_view AssetId =
				SourceString(Module, "vectorfield.assetid");
			if (AssetId.empty() ||
				Staged->VectorFields.contains(std::string(AssetId)))
			{
				continue;
			}
			const std::shared_ptr<const SOURCE_VECTOR_FIELD> Field =
				Load_VectorFieldFromDisk(AssetId);
			if (nullptr == Field)
			{
				strOutError = "Source vector field asset is missing or invalid: " +
					std::string(AssetId);
				return false;
			}
			Staged->VectorFields.emplace(std::string(AssetId), Field);
		}
	}
	Staged->pImmutableDocument = std::move(pImmutableDocument);
	OutPrepared = std::move(Staged);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectPlayback::Stage_PrevalidatedDocument(
	const EFFECT_DOCUMENT_DESC& Document,
	std::shared_ptr<const PREPARED_RESOURCES> pPreparedResources,
	std::string& strOutError)
{
	/* Existing authored documents may stage value-only edits against a prepared
	   resource bundle.  Preserve that contract here; the immutable visual-program
	   entry point below is the only seam that requires exact projected-document
	   canonical identity. */
	return Stage_PrevalidatedDocumentInternal(
		Document, std::move(pPreparedResources), false, false, {}, strOutError);
}

bool_t Client::CEffectPlayback::Stage_PrevalidatedVisualProgramDocument(
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> pProjection,
	std::shared_ptr<const PREPARED_RESOURCES> pPreparedResources,
	std::string& strOutError)
{
	if (nullptr == pProjection || !pProjection->Is_Valid() ||
		nullptr == pPreparedResources)
	{
		strOutError =
			"Admitted source visual-program projection or prepared resources are missing.";
		return false;
	}

	const EFFECT_DOCUMENT_DESC& Document = pProjection->Get_Document();
	if (pProjection->Get_EffectAssetId().empty() ||
		pProjection->Get_EffectAssetId() != Document.strEffectAssetId ||
		(pProjection->Get_AdmittedRows().empty() &&
		 pProjection->Get_AdmittedSupplementalElements().empty()))
	{
		strOutError =
			"Admitted source visual-program projection identity is invalid.";
		return false;
	}
	for (const EFFECT_VISUAL_PROGRAM_SELECTOR& Selector :
		pProjection->Get_AdmittedSelectors())
	{
		if (Selector.strEffectAssetId != pProjection->Get_EffectAssetId() ||
			Selector.strOccurrenceId.empty())
		{
			strOutError =
				"Admitted source visual-program selector closure is invalid.";
			return false;
		}
	}

	const std::shared_ptr<const EFFECT_DOCUMENT_DESC>& pImmutableDocument =
		pPreparedResources->pImmutableDocument;
	if (nullptr != pImmutableDocument &&
		pImmutableDocument.get() != &Document)
	{
		strOutError =
			"Admitted source visual-program immutable document identity mismatches.";
		return false;
	}
	std::string IdentityError;
	const std::string DocumentCanonicalSha256 =
		nullptr != pImmutableDocument ?
			pPreparedResources->strDocumentCanonicalSha256 :
			CEffectVisualProgramCorpusCodec::Compute_DocumentCanonicalSha256(
				Document, IdentityError);
	if (DocumentCanonicalSha256.empty() ||
		pProjection->Get_ProjectedDocumentSha256().empty() ||
		pPreparedResources->strEffectAssetId != Document.strEffectAssetId ||
		pPreparedResources->strDocumentCanonicalSha256 !=
			DocumentCanonicalSha256)
	{
		strOutError = IdentityError.empty() ?
			"Admitted source visual-program document/prepared identity mismatch." :
			"Admitted source visual-program canonical identity failed: " +
				IdentityError;
		return false;
	}

	std::unordered_set<std::string> SourceVisualTargetElementIds;
	if (pProjection->Get_ProjectionKind() !=
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1 &&
		pProjection->Get_ProjectionKind() !=
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1)
	{
		strOutError =
			"Admitted source visual-program projection kind is unsupported.";
		return false;
	}
	for (const EFFECT_VISUAL_PROGRAM_ROW& Row : pProjection->Get_AdmittedRows())
	{
		if (pProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1)
		{
			if (!Row.LocalDecalPacket.has_value())
			{
				strOutError =
					"Admitted adapter visual-program row lacks its immutable packet.";
				return false;
			}
			continue;
		}
		if (Row.LocalDecalPacket.has_value())
		{
			strOutError =
				"Source-recipe overlay visual-program row contains an adapter packet.";
			return false;
		}
		if (!Row.TargetIdentity.has_value() ||
			Row.TargetIdentity->strTargetElementId.empty())
		{
			strOutError =
				"Admitted source visual-program target closure is invalid.";
			return false;
		}
		const EFFECT_ELEMENT_DESC* const pTarget = Find_ElementById(
			Document, Row.TargetIdentity->strTargetElementId);
		const bool_t bFamilyTargetMatches = nullptr != pTarget && [&]()
		{
			switch (Row.eFamily)
			{
			case EFFECT_VISUAL_PROGRAM_FAMILY::MESH_PARTICLE:
				return pTarget->eKind == EFFECT_ELEMENT_KIND::MESH &&
					pTarget->SourceRecipe.bEnabled &&
					pTarget->SourceRecipe.strRendererShape == "mesh";
			case EFFECT_VISUAL_PROGRAM_FAMILY::SPRITE_PARTICLE:
				return pTarget->eKind == EFFECT_ELEMENT_KIND::SPRITE &&
					pTarget->SourceRecipe.bEnabled &&
					pTarget->SourceRecipe.strRendererShape == "sprite";
			case EFFECT_VISUAL_PROGRAM_FAMILY::DECAL_PARTICLE:
				return pTarget->eKind == EFFECT_ELEMENT_KIND::DECAL &&
					pTarget->SourceRecipe.bEnabled &&
					pTarget->SourceRecipe.strRendererShape == "decal";
			case EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON:
				return Is_CascadeRibbonTarget(*pTarget);
			case EFFECT_VISUAL_PROGRAM_FAMILY::LIGHT_PARTICLE:
				return pTarget->eKind == EFFECT_ELEMENT_KIND::LIGHT;
			case EFFECT_VISUAL_PROGRAM_FAMILY::SCREEN_POST:
				return pTarget->eKind == EFFECT_ELEMENT_KIND::SCREEN_POST;
			case EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL:
			case EFFECT_VISUAL_PROGRAM_FAMILY::END:
			default:
				return false;
			}
		}();
		if (!bFamilyTargetMatches ||
			!SourceVisualTargetElementIds.emplace(
				Row.TargetIdentity->strTargetElementId).second)
		{
			strOutError =
				"Admitted source visual-program family does not match its unique projected target.";
			return false;
		}
	}
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental :
		pProjection->Get_AdmittedSupplementalElements())
	{
		if (Supplemental.Selector.strEffectAssetId !=
				pProjection->Get_EffectAssetId() ||
			Supplemental.Selector.strOccurrenceId.empty() ||
			Supplemental.TargetIdentity.strTargetElementId.empty())
		{
			strOutError =
				"Admitted supplemental visual-program identity is invalid.";
			return false;
		}
		const EFFECT_ELEMENT_DESC* const pTarget = Find_ElementById(
			Document, Supplemental.TargetIdentity.strTargetElementId);
		if (nullptr == pTarget)
		{
			strOutError =
				"Admitted supplemental visual-program target is missing.";
			return false;
		}

		if (pProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1)
		{
			bool_t bSupplementalValid = false;
			if (Supplemental.eFamily ==
				EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON &&
				Supplemental.CascadeRibbonPacket.has_value())
			{
				bSupplementalValid = Validate_CascadeRibbonPacket(
					Supplemental, *pTarget, strOutError);
			}
			else if (Supplemental.eFamily ==
				EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL &&
				Supplemental.AnimationTrailPacket.has_value())
			{
				const auto& Packet = *Supplemental.AnimationTrailPacket;
				const auto* const pHistory = 2u == Packet.iPacketVersion ?
					pProjection->Find_BakedEdgeHistory(Packet.strHistoryId) : nullptr;
				const bool_t bHistoryValid = 1u == Packet.iPacketVersion ||
					(nullptr != pHistory &&
					 pHistory->strHistorySha256 == Packet.strHistorySha256 &&
					 pHistory->strCoordinateBasis == Packet.strCoordinateBasis &&
					 Nearly_EqualTrailValue(pHistory->fPlaybackClampSeconds,
						 Packet.fPlaybackClampSeconds));
				bSupplementalValid = bHistoryValid &&
					Validate_AnimationTrailPacket(
						Supplemental, *pTarget, strOutError);
			}
			else if (Supplemental.eFamily ==
				EFFECT_VISUAL_PROGRAM_FAMILY::LIGHT_PARTICLE &&
				Supplemental.BakedEdgeLightPacket.has_value())
			{
				const auto& Packet = *Supplemental.BakedEdgeLightPacket;
				const auto* const pHistory =
					pProjection->Find_BakedEdgeHistory(Packet.strHistoryId);
				const bool_t bHistoryValid = nullptr != pHistory &&
					pHistory->strHistorySha256 == Packet.strHistorySha256 &&
					pHistory->strCoordinateBasis == Packet.strCoordinateBasis &&
					Nearly_EqualTrailValue(pHistory->fPlaybackClampSeconds,
						Packet.fHistoryPlaybackClampSeconds);
				bSupplementalValid = bHistoryValid &&
					Validate_BakedEdgeLightPacket(
						Supplemental, *pTarget, strOutError);
			}
			if (!bSupplementalValid ||
				!SourceVisualTargetElementIds.emplace(
					Supplemental.TargetIdentity.strTargetElementId).second)
			{
				if (strOutError.empty())
				{
					strOutError =
						"Baked-edge supplemental target closure is invalid.";
				}
				return false;
			}
		}
		else if (Supplemental.eFamily !=
				EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON ||
			!Validate_CascadeRibbonPacket(
				Supplemental, *pTarget, strOutError))
		{
			if (strOutError.empty())
			{
				strOutError =
					"CascadeRibbon adapter supplemental identity is invalid.";
			}
			return false;
		}
	}
	const bool_t bSourceVisualProgramActive =
		!SourceVisualTargetElementIds.empty();
	if (!Stage_PrevalidatedDocumentInternal(Document,
			std::move(pPreparedResources),
			bSourceVisualProgramActive, true,
			std::move(SourceVisualTargetElementIds), strOutError))
	{
		return false;
	}
	m_strSourceVisualProgramStatus =
		"Admitted source visual program staged: " +
		pProjection->Get_AdmissionTokenSha256();
	m_pSourceVisualProgramProjection = std::move(pProjection);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectPlayback::Stage_PrevalidatedDocumentInternal(
	const EFFECT_DOCUMENT_DESC& Document,
	std::shared_ptr<const PREPARED_RESOURCES> pPreparedResources,
	const bool_t bSourceVisualProgramActive,
	const bool_t bRequireSourceVisualTargetClosure,
	std::unordered_set<std::string> SourceVisualTargetElementIds,
	std::string& strOutError)
{
	if (nullptr == pPreparedResources)
	{
		strOutError = "Prepared Effect playback resources are missing.";
		return false;
	}
	const std::shared_ptr<const EFFECT_DOCUMENT_DESC>& pImmutableDocument =
		pPreparedResources->pImmutableDocument;
	if (nullptr != pImmutableDocument &&
		pImmutableDocument.get() != &Document)
	{
		strOutError =
			"Prepared Effect immutable playback document identity mismatches.";
		return false;
	}
	bool_t bStagedHasPortableSourceEvents = false;
	if (!Document.bSourceContract && !Validate_PortableSourceEventRoutes(
			Document, bStagedHasPortableSourceEvents, strOutError))
	{
		return false;
	}
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (Element.Material.Execution.bFailClosed &&
			!Element.Material.Execution.bAuthoringApproximate)
		{
			continue;
		}
		for (const EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (!SourceClass_Matches(Module, "particlemodulelocalvectorfield"))
				continue;
			const std::string_view AssetId =
				SourceString(Module, "vectorfield.assetid");
			if (!AssetId.empty() &&
				!pPreparedResources->VectorFields.contains(std::string(AssetId)))
			{
				strOutError = "Prepared source vector field is missing: " +
					std::string(AssetId);
				return false;
			}
		}
	}
	EFFECT_DOCUMENT_DESC StagedOwnedDocument;
	const EFFECT_DOCUMENT_DESC* pStagedDocument = &Document;
	if (nullptr == pImmutableDocument)
	{
		StagedOwnedDocument = Document;
		pStagedDocument = &StagedOwnedDocument;
	}
	std::unordered_map<std::string, ELEMENT_STATE> StagedStates;
	std::unordered_map<std::string, size_t> StagedTransformMasterIndices;
	std::unordered_map<std::string, size_t> ElementIndices;
	for (size_t iElement = 0u; iElement < pStagedDocument->Elements.size();
		++iElement)
	{
		ElementIndices.emplace(
			pStagedDocument->Elements[iElement].strElementId, iElement);
	}
	f32_t fStagedDuration = 0.f;
	for (const EFFECT_ELEMENT_DESC& Element : pStagedDocument->Elements)
	{
		const bool_t bElementSourceVisualActive =
			bSourceVisualProgramActive &&
			(!bRequireSourceVisualTargetClosure ||
				SourceVisualTargetElementIds.contains(Element.strElementId));
		if (Element.TransformInheritance.bEnabled)
		{
			const auto MasterIterator = ElementIndices.find(
				Element.TransformInheritance.strMasterElementId);
			if (MasterIterator == ElementIndices.end())
			{
				strOutError =
					"Prepared Effect transform inheritance master is missing.";
				return false;
			}
			if (pStagedDocument->Elements[MasterIterator->second]
				.TransformInheritance.bEnabled)
			{
				strOutError =
					"Prepared Effect companions must reference one terminal transform master.";
				return false;
			}
			StagedTransformMasterIndices.emplace(
				Element.strElementId, MasterIterator->second);
		}
		ELEMENT_STATE State;
		State.iRandomState = Hash_StableId(Element.strElementId) ^
			Element.Detail.Particle.iRandomSeed;
		if (0u == State.iRandomState)
			State.iRandomState = 1u;
		Initialize_ModuleRandomStates(Element, State);
		StagedStates.emplace(Element.strElementId, std::move(State));
		const bool_t bElementPlaybackAdmitted =
			Element.bVisible &&
			(Is_EffectAuthoringExecutionTarget(
				Element.Material.Execution) ||
			 (Is_EffectFailClosedSourceGeometryCarrier(Element) &&
				bElementSourceVisualActive));
		if (!bElementPlaybackAdmitted)
			continue;
		f32_t fElementTail = 0.f;
		if (Is_ParticleSimulationElement(
			Element, bElementSourceVisualActive))
		{
			fElementTail = Element.Detail.Particle.vLifeTimeSeconds.y *
				(Element.SourceRecipe.bEnabled ?
					Element.Detail.Particle.SourceScale.fLifeTime : 1.f);
		}
		else if (EFFECT_ELEMENT_KIND::TRAIL == Element.eKind)
			fElementTail = Element.Detail.Trail.fPointLifeTimeSeconds;
		f32_t fElementDuration = Element.Detail.Timing.fLifeTimeSeconds;
		if (Element.SourceRecipe.bEnabled &&
			Element.SourceRecipe.fEmitterDurationSeconds > 0.f)
		{
			if (0u != Element.SourceRecipe.iEmitterLoopCount)
			{
				fElementDuration = Element.SourceRecipe.fEmitterDurationSeconds *
					static_cast<f32_t>(
						Element.SourceRecipe.iEmitterLoopCount);
			}
		}
		f32_t fElementEnd =
			Element.Detail.Timing.fStartDelaySeconds +
			(Element.SourceRecipe.bEnabled ?
				Element.SourceRecipe.fEmitterDelaySeconds : 0.f) +
			fElementDuration +
			Element.Detail.Timing.fAfterImageSeconds +
			fElementTail;
		/* CascadeRibbon owns a closed point-lifetime tail. Preserve the global
		   fixed-step/Seek ABI and expose only this admitted family's next
		   representable end time so the final eviction step is evaluated after
		   emission stops instead of clamping one step early on a float boundary. */
		if (bElementSourceVisualActive && Is_CascadeRibbonTarget(Element))
		{
			fElementEnd = std::nextafter(fElementEnd,
				std::numeric_limits<f32_t>::infinity());
		}
		fStagedDuration = (std::max)(fStagedDuration, fElementEnd);
	}
	for (const EFFECT_MODEL_CUE_DESC& Cue : pStagedDocument->ModelCues)
	{
		if (Cue.bVisible)
		{
			fStagedDuration = (std::max)(fStagedDuration,
				Cue.fStartDelaySeconds + Cue.fDurationSeconds);
		}
	}

	m_Document = nullptr == pImmutableDocument ?
		std::move(StagedOwnedDocument) : EFFECT_DOCUMENT_DESC{};
	m_pPreparedResources = std::move(pPreparedResources);
	m_ReconstructedRuntimeBoundary.Clear();
	m_pReconstructedExecutionPlan.reset();
	m_pSourceVisualProgramProjection.reset();
	m_bSourceVisualProgramActive = bSourceVisualProgramActive;
	m_bRequireSourceVisualTargetClosure =
		bRequireSourceVisualTargetClosure;
	m_SourceVisualTargetElementIds =
		std::move(SourceVisualTargetElementIds);
	m_bReconstructedSourceRuntimeActive = false;
	m_bHasPortableSourceEvents = bStagedHasPortableSourceEvents;
	m_strSourceVisualProgramStatus.clear();
	m_States = std::move(StagedStates);
	m_TransformMasterIndices = std::move(StagedTransformMasterIndices);
	m_fDurationSeconds = fStagedDuration;
	Reset();
	strOutError.clear();
	return true;
}

bool_t Client::CEffectPlayback::Stage_ReconstructedSourceRuntime(
	const EFFECT_DOCUMENT_DESC& Document,
	std::shared_ptr<const PREPARED_RESOURCES> pPreparedResources,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION> pPreparation,
	std::string& strOutError)
{
	CEffectReconstructedRuntimeBoundary StagedBoundary;
	if (nullptr == pPreparedResources ||
		!StagedBoundary.Stage(std::move(pPreparation),
			EFFECT_RECONSTRUCTED_RUNTIME_SEAM::PLAYBACK, strOutError))
	{
		if (strOutError.empty())
			strOutError =
				"Reconstructed source runtime prepared resources are missing.";
		return false;
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN> StagedPlan;
	if (!CEffectReconstructedExecutionPlanCompiler::Compile_Preparation(
		StagedBoundary.Get_Preparation(), StagedPlan, strOutError))
	{
		return false;
	}
	EFFECT_DOCUMENT_DESC StagedDocument;
	if (nullptr == StagedPlan ||
		!Build_ReconstructedSourceRuntimeDocument(
			Document, *StagedPlan, StagedDocument, strOutError))
	{
		if (strOutError.empty())
			strOutError = "Reconstructed source runtime plan is missing.";
		return false;
	}
	if (!Stage_PrevalidatedDocumentInternal(
		StagedDocument, std::move(pPreparedResources), true, false, {},
		strOutError))
	{
		return false;
	}

	m_ReconstructedRuntimeBoundary = std::move(StagedBoundary);
	m_pReconstructedExecutionPlan = std::move(StagedPlan);
	m_bSourceVisualProgramActive = true;
	m_bRequireSourceVisualTargetClosure = false;
	m_SourceVisualTargetElementIds.clear();
	m_bReconstructedSourceRuntimeActive = true;
	m_strSourceVisualProgramStatus =
		"Artist 31470 reconstructed source runtime staged; Product remains false.";
	Reset();
	strOutError.clear();
	return true;
}

bool_t Client::CEffectPlayback::
	Stage_ReconstructedSourceRuntimeWithVisualProgramAdapter(
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection,
		std::shared_ptr<const PREPARED_RESOURCES> pPreparedResources,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::string& strOutError)
{
	if (nullptr == pProjection || !pProjection->Is_Valid() ||
		pProjection->Get_ProjectionKind() !=
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 ||
		nullptr == pPreparedResources || nullptr == pPreparation ||
		nullptr == pPreparation->Get_Program() ||
		pProjection->Get_EffectAssetId() !=
			pPreparation->Get_Program()->strRuntimeCatalogAssetId ||
		pProjection->Get_EffectAssetId() !=
			pProjection->Get_Document().strEffectAssetId ||
		pProjection->Get_AdmittedRows().empty() ||
		!std::all_of(pProjection->Get_AdmittedRows().begin(),
			pProjection->Get_AdmittedRows().end(),
			[](const EFFECT_VISUAL_PROGRAM_ROW& Row)
			{
				return Row.LocalDecalPacket.has_value() &&
					Row.TargetIdentity.has_value() &&
					!Row.TargetIdentity->strTargetElementId.empty();
			}))
	{
		strOutError =
			"Reconstructed source runtime visual adapter identity is invalid.";
		return false;
	}
	std::string IdentityError;
	const std::string DocumentCanonicalSha256 =
		CEffectVisualProgramCorpusCodec::Compute_DocumentCanonicalSha256(
			pProjection->Get_Document(), IdentityError);
	if (DocumentCanonicalSha256.empty() ||
		pPreparedResources->strEffectAssetId !=
			pProjection->Get_EffectAssetId() ||
		pPreparedResources->strDocumentCanonicalSha256 !=
			DocumentCanonicalSha256)
	{
		strOutError = IdentityError.empty() ?
			"Reconstructed source runtime visual adapter prepared identity mismatches." :
			"Reconstructed source runtime visual adapter identity failed: " +
				IdentityError;
		return false;
	}
	CEffectPlayback AdapterValidator;
	if (!AdapterValidator.Stage_PrevalidatedVisualProgramDocument(
			pProjection, pPreparedResources, strOutError) ||
		AdapterValidator.Is_SourceVisualProgramActive())
	{
		if (strOutError.empty())
		{
			strOutError =
				"Reconstructed source runtime visual adapter validation diverged.";
		}
		return false;
	}
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pCommittedProjection = pProjection;
	if (!Stage_ReconstructedSourceRuntime(
			pProjection->Get_Document(), std::move(pPreparedResources),
			std::move(pPreparation), strOutError))
	{
		return false;
	}
	m_pSourceVisualProgramProjection = pCommittedProjection;
	m_strSourceVisualProgramStatus =
		"Reconstructed source runtime staged with immutable visual adapter: " +
		pCommittedProjection->Get_AdmissionTokenSha256();
	strOutError.clear();
	return true;
}

bool_t Client::CEffectPlayback::Stage_ReconstructedRuntimeProgram(
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION> pPreparation,
	std::string& strOutError)
{
	CEffectReconstructedRuntimeBoundary StagedBoundary;
	if (!StagedBoundary.Stage(std::move(pPreparation),
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM::PLAYBACK, strOutError))
		return false;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN> StagedPlan;
	if (!CEffectReconstructedExecutionPlanCompiler::Compile_Preparation(
		StagedBoundary.Get_Preparation(), StagedPlan, strOutError))
	{
		return false;
	}
	m_Document = {};
	m_pPreparedResources.reset();
	m_States.clear();
	m_TransformMasterIndices.clear();
	m_SourceAnchorWorlds.clear();
	m_PendingSourceEvents.clear();
	m_Frame = {};
	m_fSampleTimeSeconds = 0.f;
	m_fAccumulatorSeconds = 0.0;
	m_fDurationSeconds = 0.f;
	m_iSimulationStep = 0u;
	m_ReconstructedRuntimeBoundary = std::move(StagedBoundary);
	m_pReconstructedExecutionPlan = std::move(StagedPlan);
	m_pSourceVisualProgramProjection.reset();
	m_bSourceVisualProgramActive = false;
	m_bRequireSourceVisualTargetClosure = false;
	m_SourceVisualTargetElementIds.clear();
	m_bReconstructedSourceRuntimeActive = false;
	m_strSourceVisualProgramStatus.clear();
	strOutError.clear();
	return true;
}

Client::EFFECT_SUBUV_FRAME_DESC
Client::CEffectPlayback::Resolve_SourceSubUVFrame(
	uint32_t iColumns,
	uint32_t iRows,
	f32_t fFrameValue,
	bool_t bAllowFlip,
	bool_t bSquareFlip,
	f32_t fDistributionRandom,
	bool_t bLinearBlend)
{
	iColumns = (std::max)(1u, iColumns);
	iRows = (std::max)(1u, iRows);
	const uint32_t iFrameCount = iColumns * iRows;
	fFrameValue = (std::max)(0.f, fFrameValue);
	const uint32_t iFrame = static_cast<uint32_t>(
		std::floor(fFrameValue)) % iFrameCount;
	const uint32_t iNextFrame = (iFrame + 1u) % iFrameCount;
	const f32_t fScaleX = 1.f / static_cast<f32_t>(iColumns);
	const f32_t fScaleY = 1.f / static_cast<f32_t>(iRows);
	const bool_t bFlipX = bAllowFlip && fDistributionRandom >= 0.5f;
	const bool_t bFlipY = bAllowFlip && bSquareFlip &&
		fDistributionRandom >= 0.75f;
	const auto MakeTransform = [=](const uint32_t iSourceFrame)
	{
		const uint32_t iColumn = iSourceFrame % iColumns;
		const uint32_t iRow = iSourceFrame / iColumns;
		return float4_t{
			bFlipX ? -fScaleX : fScaleX,
			bFlipY ? -fScaleY : fScaleY,
			static_cast<f32_t>(iColumn + (bFlipX ? 1u : 0u)) * fScaleX,
			static_cast<f32_t>(iRow + (bFlipY ? 1u : 0u)) * fScaleY };
	};
	EFFECT_SUBUV_FRAME_DESC Result;
	Result.Current = MakeTransform(iFrame);
	Result.Next = MakeTransform(iNextFrame);
	Result.fBlend = bLinearBlend ?
		fFrameValue - std::floor(fFrameValue) : 0.f;
	return Result;
}

bool_t Client::CEffectPlayback::Resolve_ParticleSpriteSubUV(
	const EFFECT_EVALUATED_PARTICLE& Particle,
	const EFFECT_SUBUV_FRAME_DESC& SourceSubUV,
	EFFECT_SUBUV_FRAME_DESC& OutSubUV)
{
	EFFECT_SUBUV_FRAME_DESC Staged = SourceSubUV;
	/* Mesh particles retain their signed model transform.  Cascade image
	   flipping is a Sprite renderer contract and must not mirror mesh UVs. */
	if (!Particle.bSourceImageFlipping ||
		(nullptr != Particle.pElement && Is_MeshParticle(*Particle.pElement)))
	{
		OutSubUV = Staged;
		return true;
	}
	const auto IsUnitSign = [](const f32_t fValue)
	{
		return fValue == -1.f || fValue == 1.f;
	};
	if (!IsUnitSign(Particle.vSourceImageFlipSign.x) ||
		!IsUnitSign(Particle.vSourceImageFlipSign.y))
	{
		return false;
	}
	const auto ApplyImageFlip = [&Particle](float4_t& Transform)
	{
		if (Particle.vSourceImageFlipSign.x < 0.f)
		{
			Transform.z += Transform.x;
			Transform.x = -Transform.x;
		}
		if (Particle.vSourceImageFlipSign.y < 0.f)
		{
			Transform.w += Transform.y;
			Transform.y = -Transform.y;
		}
	};
	/* Apply the evaluated StartSize signs exactly once, within each atlas cell.
	   Positive billboard geometry keeps front-face winding stable. */
	ApplyImageFlip(Staged.Current);
	ApplyImageFlip(Staged.Next);
	OutSubUV = Staged;
	return true;
}

void Client::CEffectPlayback::Reset()
{
	std::string GateStatus;
	if (!m_bReconstructedSourceRuntimeActive &&
		!m_bSourceVisualProgramActive &&
		!m_ReconstructedRuntimeBoundary.Admit_Execution(GateStatus))
	{
		m_Frame = {};
		return;
	}
	m_fSampleTimeSeconds = 0.f;
	m_fAccumulatorSeconds = 0.0;
	m_iSimulationStep = 0u;
	m_vPreviousRootPosition = {};
	m_vParentVelocity = {};
	m_bPreviousRootPositionInitialized = false;
	m_bSourceEventQueueOverflow = false;
	m_PendingSourceEvents.clear();
	m_Frame = {};
	for (const EFFECT_ELEMENT_DESC& Element : Get_StagedDocument().Elements)
	{
		if (!Is_PlaybackElementAdmitted(Element))
			continue;
		ELEMENT_STATE& State = m_States[Element.strElementId];
		State = {};
		State.iRandomState = Hash_StableId(Element.strElementId) ^
			Element.Detail.Particle.iRandomSeed;
		if (0u == State.iRandomState)
			State.iRandomState = 1u;
		Initialize_ModuleRandomStates(Element, State);
	}
	float4x4_t Identity{};
	XMStoreFloat4x4(&Identity, XMMatrixIdentity());
	Rebuild_Frame(Identity);
}

void Client::CEffectPlayback::Update(
	const f32_t fTimeDelta,
	const float4x4_t& RootWorld)
{
	std::string GateStatus;
	if (!m_bReconstructedSourceRuntimeActive &&
		!m_bSourceVisualProgramActive &&
		!m_ReconstructedRuntimeBoundary.Admit_Execution(GateStatus))
	{
		m_Frame = {};
		return;
	}
	if (!std::isfinite(fTimeDelta) || fTimeDelta <= 0.f)
	{
		Rebuild_Frame(RootWorld);
		return;
	}

	m_fAccumulatorSeconds += static_cast<f64_t>(fTimeDelta);
	uint32_t iSteps = 0u;
	while (m_fAccumulatorSeconds + FIXED_STEP_EPSILON >=
		FIXED_STEP_SECONDS_EXACT &&
		iSteps < MAX_CATCH_UP_STEPS)
	{
		if (!Step(FIXED_STEP_SECONDS, RootWorld))
			return;
		m_fAccumulatorSeconds = (std::max)(0.0,
			m_fAccumulatorSeconds - FIXED_STEP_SECONDS_EXACT);
		++iSteps;
	}
	Rebuild_Frame(RootWorld);
}

void Client::CEffectPlayback::Seek(
	const f32_t fSampleTimeSeconds,
	const float4x4_t& RootWorld)
{
	std::string GateStatus;
	if (!m_bReconstructedSourceRuntimeActive &&
		!m_bSourceVisualProgramActive &&
		!m_ReconstructedRuntimeBoundary.Admit_Execution(GateStatus))
	{
		m_Frame = {};
		return;
	}
	const f32_t fTarget = std::clamp(
		std::isfinite(fSampleTimeSeconds) ? fSampleTimeSeconds : 0.f,
		0.f,
		m_fDurationSeconds);
	Reset();
	/* A zero-time product cue still needs to freeze snapshot attachments at the
	post-Character root supplied by the presentation service. Product staging
	does not call Seek, so this is the first snapshot capture for that instance. */
	for (const EFFECT_ELEMENT_DESC& Element : Get_StagedDocument().Elements)
	{
		if (!Element.bVisible || !Element.ActionCueAttachment.bEnabled ||
			Element.ActionCueAttachment.bFollow ||
			Element.Detail.Timing.fStartDelaySeconds > fTarget)
		{
			continue;
		}
		ELEMENT_STATE& State = m_States[Element.strElementId];
		State.ActionRootWorld = RootWorld;
		State.bActionRootCaptured = true;
	}
	const uint64_t iTargetSteps = static_cast<uint64_t>(std::floor(
		static_cast<f64_t>(fTarget) / FIXED_STEP_SECONDS_EXACT +
		FIXED_STEP_EPSILON));
	for (uint64_t iStep = 0u; iStep < iTargetSteps; ++iStep)
	{
		if (!Step(FIXED_STEP_SECONDS, RootWorld))
			return;
	}
	m_fAccumulatorSeconds = (std::max)(0.0,
		static_cast<f64_t>(fTarget) -
		static_cast<f64_t>(iTargetSteps) * FIXED_STEP_SECONDS_EXACT);
	m_fSampleTimeSeconds = fTarget;
	Rebuild_Frame(RootWorld);
}

bool_t Client::CEffectPlayback::Collect_TransformHistorySample(
	const f32_t fSampleTimeSeconds,
	const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER& TransformProvider,
	EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
	std::string& strOutError) const
{
	if (!std::isfinite(fSampleTimeSeconds) || fSampleTimeSeconds < 0.f ||
		!TransformProvider)
	{
		strOutError = "Effect transform history request is invalid.";
		return false;
	}

	EFFECT_FIXED_STEP_TRANSFORM_SAMPLE Staged;
	std::string ProviderError;
	if (!TransformProvider(fSampleTimeSeconds, Staged, ProviderError))
	{
		strOutError = ProviderError.empty() ?
			"Effect transform history provider rejected the sample." :
			std::move(ProviderError);
		return false;
	}
	if (!Is_FiniteAffineMatrix(Staged.RootWorld))
	{
		strOutError = "Effect transform history root is not a finite affine matrix.";
		return false;
	}
	for (const auto& [AnchorId, AnchorWorld] : Staged.SourceAnchorWorlds)
	{
		if (AnchorId.empty() || !Is_FiniteAffineMatrix(AnchorWorld))
		{
			strOutError =
				"Effect transform history contains an invalid source anchor.";
			return false;
		}
	}

	for (const EFFECT_ELEMENT_DESC& Element : Get_StagedDocument().Elements)
	{
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Attachment =
			Element.ActionCueAttachment;
		if (Element.bVisible && Attachment.bEnabled && Attachment.bFollow &&
			(Attachment.strRuntimeAnchorSlotId.empty() ||
			 !Staged.SourceAnchorWorlds.contains(
				 Attachment.strRuntimeAnchorSlotId)))
		{
			strOutError = "Effect transform history is missing required anchor: " +
				Attachment.strRuntimeAnchorSlotId;
			return false;
		}
	}
	const auto Preparation =
		m_ReconstructedRuntimeBoundary.Get_Preparation();
	if (m_bReconstructedSourceRuntimeActive && nullptr != Preparation)
	{
		for (const EFFECT_RECONSTRUCTED_ANCHOR_BINDING& Binding :
			Preparation->Get_AnchorRequests())
		{
			const EFFECT_RUNTIME_PROGRAM_ANCHOR_REQUEST& Request = Binding.Request;
			if (Request.bFollow &&
				(Request.strRuntimeAnchorSlotId.empty() ||
				 !Staged.SourceAnchorWorlds.contains(
					 Request.strRuntimeAnchorSlotId)))
			{
				strOutError =
					"Effect transform history is missing prepared anchor: " +
					Request.strRuntimeAnchorSlotId;
				return false;
			}
		}
	}

	OutSample = std::move(Staged);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectPlayback::Update_WithTransformHistory(
	const f32_t fTimeDelta,
	const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER& TransformProvider,
	std::string& strOutError)
{
	std::string GateStatus;
	if ((!m_bReconstructedSourceRuntimeActive &&
		 !m_bSourceVisualProgramActive &&
		 !m_ReconstructedRuntimeBoundary.Admit_Execution(GateStatus)) ||
		!std::isfinite(fTimeDelta) || fTimeDelta < 0.f)
	{
		strOutError = GateStatus.empty() ?
			"Effect transform-history update request is invalid." : GateStatus;
		return false;
	}

	const f64_t fPendingAccumulator =
		m_fAccumulatorSeconds + static_cast<f64_t>(fTimeDelta);
	if (!std::isfinite(fPendingAccumulator) || fPendingAccumulator < 0.0)
	{
		strOutError = "Effect transform-history accumulator is invalid.";
		return false;
	}
	/* This typed path must never claim animation-clock equality after applying
	   only part of a backlog.  The deferred Tool start absorbs cache-build wall
	   time; a later hitch larger than the bounded 60-step transaction is rejected
	   before any provider call or playback mutation. */
	if (fPendingAccumulator >
		(static_cast<f64_t>(MAX_CATCH_UP_STEPS) + 1.0) *
			FIXED_STEP_SECONDS_EXACT)
	{
		strOutError =
			"Effect transform-history update exceeds the 60-step catch-up transaction.";
		return false;
	}
	const uint64_t iAvailableSteps = static_cast<uint64_t>(std::floor(
		(fPendingAccumulator + FIXED_STEP_EPSILON) /
		FIXED_STEP_SECONDS_EXACT));
	if (iAvailableSteps > static_cast<uint64_t>(MAX_CATCH_UP_STEPS))
	{
		strOutError =
			"Effect transform-history update exceeds the 60-step catch-up transaction.";
		return false;
	}
	const uint32_t iStepCount = static_cast<uint32_t>(iAvailableSteps);
	std::vector<EFFECT_FIXED_STEP_TRANSFORM_SAMPLE> Samples;
	Samples.reserve(iStepCount);
	for (uint32_t iStep = 0u; iStep < iStepCount; ++iStep)
	{
		const f32_t fStepTime = static_cast<f32_t>(
			static_cast<f64_t>(m_iSimulationStep + iStep + 1u) *
			FIXED_STEP_SECONDS_EXACT);
		EFFECT_FIXED_STEP_TRANSFORM_SAMPLE Sample;
		if (!Collect_TransformHistorySample(
				fStepTime, TransformProvider, Sample, strOutError))
		{
			return false;
		}
		Samples.push_back(std::move(Sample));
	}

	f64_t fCommittedAccumulator = fPendingAccumulator;
	float4x4_t FinalRoot = m_Frame.RootWorld;
	for (EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& Sample : Samples)
	{
		m_SourceAnchorWorlds = std::move(Sample.SourceAnchorWorlds);
		FinalRoot = Sample.RootWorld;
		if (!Step(FIXED_STEP_SECONDS, FinalRoot))
		{
			strOutError = m_strSourceVisualProgramStatus;
			return false;
		}
		fCommittedAccumulator = (std::max)(
			0.0, fCommittedAccumulator - FIXED_STEP_SECONDS_EXACT);
	}
	m_fAccumulatorSeconds = fCommittedAccumulator;
	Rebuild_Frame(FinalRoot);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectPlayback::Seek_WithTransformHistory(
	const f32_t fSampleTimeSeconds,
	const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER& TransformProvider,
	std::string& strOutError)
{
	std::string GateStatus;
	if ((!m_bReconstructedSourceRuntimeActive &&
		 !m_bSourceVisualProgramActive &&
		 !m_ReconstructedRuntimeBoundary.Admit_Execution(GateStatus)) ||
		!std::isfinite(fSampleTimeSeconds))
	{
		strOutError = GateStatus.empty() ?
			"Effect transform-history seek request is invalid." : GateStatus;
		return false;
	}
	const f32_t fTarget = std::clamp(
		fSampleTimeSeconds, 0.f, m_fDurationSeconds);
	const uint64_t iTargetSteps = static_cast<uint64_t>(std::floor(
		static_cast<f64_t>(fTarget) / FIXED_STEP_SECONDS_EXACT +
		FIXED_STEP_EPSILON));
	std::vector<EFFECT_FIXED_STEP_TRANSFORM_SAMPLE> Samples;
	Samples.reserve(static_cast<size_t>(iTargetSteps));
	for (uint64_t iStep = 0u; iStep < iTargetSteps; ++iStep)
	{
		const f32_t fStepTime = static_cast<f32_t>(
			static_cast<f64_t>(iStep + 1u) * FIXED_STEP_SECONDS_EXACT);
		EFFECT_FIXED_STEP_TRANSFORM_SAMPLE Sample;
		if (!Collect_TransformHistorySample(
				fStepTime, TransformProvider, Sample, strOutError))
		{
			return false;
		}
		Samples.push_back(std::move(Sample));
	}
	const f64_t fSteppedTime =
		static_cast<f64_t>(iTargetSteps) * FIXED_STEP_SECONDS_EXACT;
	EFFECT_FIXED_STEP_TRANSFORM_SAMPLE FinalSample;
	if (!Samples.empty() &&
		std::abs(static_cast<f64_t>(fTarget) - fSteppedTime) <=
			FIXED_STEP_EPSILON)
	{
		FinalSample = Samples.back();
	}
	else if (!Collect_TransformHistorySample(
			fTarget, TransformProvider, FinalSample, strOutError))
	{
		return false;
	}

	Reset();
	for (EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& Sample : Samples)
	{
		m_SourceAnchorWorlds = std::move(Sample.SourceAnchorWorlds);
		if (!Step(FIXED_STEP_SECONDS, Sample.RootWorld))
		{
			strOutError = m_strSourceVisualProgramStatus;
			return false;
		}
	}
	m_fAccumulatorSeconds = (std::max)(
		0.0, static_cast<f64_t>(fTarget) - fSteppedTime);
	m_fSampleTimeSeconds = fTarget;
	m_SourceAnchorWorlds = std::move(FinalSample.SourceAnchorWorlds);
	Rebuild_Frame(FinalSample.RootWorld);
	strOutError.clear();
	return true;
}

f64_t Client::CEffectPlayback::Get_FixedStepClockSeconds() const
{
	return static_cast<f64_t>(m_iSimulationStep) *
		FIXED_STEP_SECONDS_EXACT + m_fAccumulatorSeconds;
}

bool_t Client::CEffectPlayback::Step(
	const f32_t fFixedDelta,
	const float4x4_t& RootWorld)
{
	const auto PreviousStates = m_bHasPortableSourceEvents ?
		m_States : decltype(m_States){};
	const uint64_t iPreviousSimulationStep = m_iSimulationStep;
	const f32_t fPreviousSampleTimeSeconds = m_fSampleTimeSeconds;
	const float3_t vPreviousRootPosition = m_vPreviousRootPosition;
	const float3_t vPreviousParentVelocity = m_vParentVelocity;
	const bool_t bPreviousRootPositionInitialized =
		m_bPreviousRootPositionInitialized;
	const float3_t CurrentRootPosition = Get_Translation(RootWorld);
	if (m_bPreviousRootPositionInitialized)
	{
		m_vParentVelocity = Scale3(Subtract3(
			CurrentRootPosition, m_vPreviousRootPosition), 1.f / fFixedDelta);
	}
	else
	{
		m_vParentVelocity = {};
	}
	m_vPreviousRootPosition = CurrentRootPosition;
	m_bPreviousRootPositionInitialized = true;
	++m_iSimulationStep;
	m_fSampleTimeSeconds = static_cast<f32_t>(
		static_cast<f64_t>(m_iSimulationStep) * FIXED_STEP_SECONDS_EXACT);
	m_PendingSourceEvents.clear();
	m_bSourceEventQueueOverflow = false;

	for (const EFFECT_ELEMENT_DESC& Element : Get_StagedDocument().Elements)
	{
		if (!Is_PlaybackElementAdmitted(Element))
			continue;
		ELEMENT_STATE& State = m_States[Element.strElementId];
		const f32_t fLocalTime = m_fSampleTimeSeconds -
			Element.Detail.Timing.fStartDelaySeconds;
		if (fLocalTime < 0.f)
			continue;
		if (Element.ActionCueAttachment.bEnabled &&
			!Element.ActionCueAttachment.bFollow &&
			!State.bActionRootCaptured)
		{
			State.ActionRootWorld = RootWorld;
			State.bActionRootCaptured = true;
		}
		if (!Can_EvaluateElementWorld(Element))
			continue;

		const f32_t fEmitterElapsed = Element.SourceRecipe.bEnabled ?
			fLocalTime - Element.SourceRecipe.fEmitterDelaySeconds :
			fLocalTime;
		const f32_t fSourceEmissionDuration =
			Element.SourceRecipe.bEnabled &&
			Element.SourceRecipe.fEmitterDurationSeconds > 0.f ?
			(0u == Element.SourceRecipe.iEmitterLoopCount ?
				Element.Detail.Timing.fLifeTimeSeconds :
				Element.SourceRecipe.fEmitterDurationSeconds *
					static_cast<f32_t>(
						Element.SourceRecipe.iEmitterLoopCount)) :
			Element.Detail.Timing.fLifeTimeSeconds;
		if (Is_ParticleSimulationElement(
				Element, Is_SourceVisualProgramElementAdmitted(Element)) &&
			fEmitterElapsed >= 0.f &&
			fEmitterElapsed <= fSourceEmissionDuration)
		{
			if (Element.SourceRecipe.bEnabled)
			{
				const EFFECT_CASCADE_RECIPE_DESC& Recipe = Element.SourceRecipe;
				const f32_t fEmitterDuration =
					Recipe.fEmitterDurationSeconds > 0.f ?
					Recipe.fEmitterDurationSeconds :
					Element.Detail.Timing.fLifeTimeSeconds;
				const uint32_t iLoopIndex = fEmitterDuration > 0.f ?
					static_cast<uint32_t>(std::floor(fEmitterElapsed /
						fEmitterDuration)) : 0u;
				const bool_t bLoopAllowed = 0u == Recipe.iEmitterLoopCount ||
					iLoopIndex < Recipe.iEmitterLoopCount;
				if (bLoopAllowed)
				{
					const f32_t fEmitterTime = fEmitterDuration > 0.f ?
						fEmitterElapsed - static_cast<f32_t>(iLoopIndex) *
						fEmitterDuration : fLocalTime;
					if (iLoopIndex != State.iSourceLoopIndex)
					{
						State.iSourceLoopIndex = iLoopIndex;
						State.iNextSourceBurst = 0u;
						State.fSpawnAccumulator = 0.f;
						State.fSourceSpawnPerUnitAccumulator = 0.f;
						State.bSourceSpawnPerUnitOriginInitialized = false;
						Reset_LoopingModuleRandomStates(State);
					}
					while (State.iNextSourceBurst < Recipe.Bursts.size() &&
						Recipe.Bursts[State.iNextSourceBurst].fTimeSeconds <=
							fEmitterTime + 0.5f * fFixedDelta)
					{
						const EFFECT_PARTICLE_BURST_DESC& Burst =
							Recipe.Bursts[State.iNextSourceBurst++];
						const uint32_t iCount = Burst.iCountMaximum <=
							Burst.iCountMinimum ? Burst.iCountMaximum :
							Burst.iCountMinimum + Next_Random(State) %
							(Burst.iCountMaximum - Burst.iCountMinimum + 1u);
						/* The random draw happens first either way, so trimming
						   the burst does not shift the emitter random stream. */
						Spawn_Particles(Element, State, Scale_SourceCount(
							Element, iCount), RootWorld);
					}
					const EFFECT_SOURCE_MODULE_DESC* pSpawnModule =
						Find_SourceModule(Element, "particlemodulespawn");
					const f32_t fRate = nullptr == pSpawnModule ?
						Element.Detail.Particle.fSpawnRatePerSecond :
						Evaluate_ModuleFloat(State, *pSpawnModule, "rate",
							fEmitterTime,
							Element.Detail.Particle.fSpawnRatePerSecond);
					const EFFECT_DISTRIBUTION_DESC* pRateScaleDistribution =
						Find_SourceDistribution(pSpawnModule, "ratescale");
					/* Cascade's null CDO RateScale is multiplicative identity.  It must
					   neither collapse Rate to zero nor advance the emitter random stream. */
					const f32_t fRateScale =
						nullptr == pRateScaleDistribution ||
						Is_SourceNullCdoDistribution(
							*pRateScaleDistribution, 1u) ? 1.f :
						Evaluate_ModuleFloat(State, *pSpawnModule, "ratescale",
							fEmitterTime, 1.f);
					State.fSpawnAccumulator +=
						(std::max)(0.f, fRate * fRateScale) * fFixedDelta *
						Element.Detail.Particle.SourceScale.fCount;
					const uint32_t iSpawnCount = static_cast<uint32_t>(
						std::floor(State.fSpawnAccumulator));
					State.fSpawnAccumulator -=
						static_cast<f32_t>(iSpawnCount);
					Spawn_Particles(Element, State, iSpawnCount, RootWorld);
					const uint32_t iSpawnPerUnitCount = Consume_SourceSpawnPerUnit(
						Element, State, fEmitterTime, RootWorld);
					Spawn_Particles(
						Element, State, iSpawnPerUnitCount, RootWorld);
				}
			}
			else
			{
				if (!State.bBurstSpawned)
				{
					Spawn_Particles(Element, State,
						Element.Detail.Particle.iBurstCount, RootWorld);
					State.bBurstSpawned = true;
				}
				State.fSpawnAccumulator +=
					Element.Detail.Particle.fSpawnRatePerSecond * fFixedDelta;
				const uint32_t iSpawnCount = static_cast<uint32_t>(
					std::floor(State.fSpawnAccumulator));
				State.fSpawnAccumulator -= static_cast<f32_t>(iSpawnCount);
				Spawn_Particles(Element, State, iSpawnCount, RootWorld);
			}
		}
	}

	if (!Dispatch_SourceEvents(fFixedDelta, RootWorld))
	{
		if (m_bHasPortableSourceEvents)
			m_States = PreviousStates;
		m_iSimulationStep = iPreviousSimulationStep;
		m_fSampleTimeSeconds = fPreviousSampleTimeSeconds;
		m_vPreviousRootPosition = vPreviousRootPosition;
		m_vParentVelocity = vPreviousParentVelocity;
		m_bPreviousRootPositionInitialized =
			bPreviousRootPositionInitialized;
		m_PendingSourceEvents.clear();
		m_bSourceEventQueueOverflow = false;
		m_strSourceVisualProgramStatus =
			"Portable source event queue overflow rolled back the fixed step.";
		return false;
	}

	for (const EFFECT_ELEMENT_DESC& Element : Get_StagedDocument().Elements)
	{
		if (!Is_PlaybackElementAdmitted(Element))
			continue;
		ELEMENT_STATE& State = m_States[Element.strElementId];
		Update_Particles(Element, State, fFixedDelta, RootWorld);
		if (EFFECT_ELEMENT_KIND::TRAIL == Element.eKind)
			Sample_Trail(Element, State, fFixedDelta, RootWorld);
		if ((EFFECT_ELEMENT_KIND::MESH == Element.eKind ||
			EFFECT_ELEMENT_KIND::SPRITE == Element.eKind) &&
			Element.Detail.Timing.fAfterImageSeconds > 0.f &&
			Element.Detail.AfterImage.iMaxCopies > 0u)
		{
			Sample_AfterImages(Element, State, fFixedDelta, RootWorld);
		}
	}
	return true;
}

uint32_t Client::CEffectPlayback::Consume_SourceSpawnPerUnit(
	const EFFECT_ELEMENT_DESC& Element,
	ELEMENT_STATE& State,
	const f32_t fEmitterTimeSeconds,
	const float4x4_t& RootWorld)
{
	if (!Is_SourceVisualProgramElementAdmitted(Element) &&
		!Is_PortableAuthoredParticleCarrier(Element))
		return 0u;

	std::vector<const EFFECT_SOURCE_MODULE_DESC*> SpawnPerUnitModules;
	for (const EFFECT_SOURCE_MODULE_DESC& Module : Element.SourceRecipe.Modules)
	{
		if (SourceModule_Enabled(Module) &&
			SourceClass_Matches(Module, "particlemodulespawnperunit"))
		{
			SpawnPerUnitModules.push_back(&Module);
		}
	}
	if (SpawnPerUnitModules.empty() || !Can_EvaluateElementWorld(Element))
		return 0u;

	const float3_t CurrentOrigin = Get_Translation(Evaluate_ElementWorld(
		Element, m_fSampleTimeSeconds, RootWorld));
	if (!std::isfinite(CurrentOrigin.x) || !std::isfinite(CurrentOrigin.y) ||
		!std::isfinite(CurrentOrigin.z))
	{
		m_strSourceVisualProgramStatus =
			"SpawnPerUnit rejected a nonfinite emitter origin: " +
			Element.strElementId;
		State.bSourceSpawnPerUnitOriginInitialized = false;
		State.fSourceSpawnPerUnitAccumulator = 0.f;
		return 0u;
	}
	if (!State.bSourceSpawnPerUnitOriginInitialized)
	{
		State.vSourceSpawnPerUnitPreviousOrigin = CurrentOrigin;
		State.bSourceSpawnPerUnitOriginInitialized = true;
		return 0u;
	}

	const f32_t fDistance = Length3(Subtract3(
		CurrentOrigin, State.vSourceSpawnPerUnitPreviousOrigin));
	State.vSourceSpawnPerUnitPreviousOrigin = CurrentOrigin;
	if (!std::isfinite(fDistance) || fDistance < 0.f)
	{
		m_strSourceVisualProgramStatus =
			"SpawnPerUnit rejected a nonfinite movement distance: " +
			Element.strElementId;
		State.fSourceSpawnPerUnitAccumulator = 0.f;
		return 0u;
	}

	double SpawnContribution = 0.0;
	for (const EFFECT_SOURCE_MODULE_DESC* pModule : SpawnPerUnitModules)
	{
		const f32_t fUnitScalarUeCentimeters = SourceNumber(
			*pModule, "unitscalar", 0.f);
		const f32_t fSpawnPerUnit = Evaluate_ModuleFloat(
			State, *pModule, "spawnperunit", fEmitterTimeSeconds, 0.f);
		const f32_t fUnitScalarClient = 0.01f * fUnitScalarUeCentimeters;
		if (!std::isfinite(fUnitScalarClient) || fUnitScalarClient <= 0.f ||
			!std::isfinite(fSpawnPerUnit) || fSpawnPerUnit < 0.f)
		{
			m_strSourceVisualProgramStatus =
				"SpawnPerUnit rejected invalid rate/unitScalar: " +
				Element.strElementId;
			State.fSourceSpawnPerUnitAccumulator = 0.f;
			return 0u;
		}
		SpawnContribution += static_cast<double>(fDistance) *
			static_cast<double>(fSpawnPerUnit) /
			static_cast<double>(fUnitScalarClient);
	}

	const double Accumulated =
		static_cast<double>(State.fSourceSpawnPerUnitAccumulator) +
		SpawnContribution;
	if (!std::isfinite(Accumulated) || Accumulated < 0.0 ||
		Accumulated > static_cast<double>(
			(std::numeric_limits<uint32_t>::max)()))
	{
		m_strSourceVisualProgramStatus =
			"SpawnPerUnit rejected an unbounded accumulator: " +
			Element.strElementId;
		State.fSourceSpawnPerUnitAccumulator = 0.f;
		return 0u;
	}
	const uint32_t iWholeParticles = static_cast<uint32_t>(
		std::floor(Accumulated));
	State.fSourceSpawnPerUnitAccumulator = static_cast<f32_t>(
		Accumulated - static_cast<double>(iWholeParticles));
	return (std::min)(iWholeParticles, Element.Detail.Particle.iMaxParticles);
}

void Client::CEffectPlayback::Spawn_Particles(
	const EFFECT_ELEMENT_DESC& Element,
	ELEMENT_STATE& State,
	const uint32_t iCount,
	const float4x4_t& RootWorld,
	const SOURCE_PARTICLE_EVENT* pSourceEvent)
{
	const EFFECT_PARTICLE_DESC& Desc = Element.Detail.Particle;
	if (!Can_EvaluateElementWorld(Element))
		return;
	const float4x4_t ElementWorld = Evaluate_ElementWorld(
		Element, m_fSampleTimeSeconds, RootWorld);
	const matrix_t InverseElementWorld = XMMatrixInverse(
		nullptr, XMLoadFloat4x4(&ElementWorld));
	/* The ceiling rises with the trim, otherwise asking for more particles would
	   stop at the count the source was extracted with. */
	const uint32_t iMaxParticles = Scale_SourceCount(
		Element, Desc.iMaxParticles);
	const uint32_t iAvailable = iMaxParticles > State.Particles.size() ?
		iMaxParticles - static_cast<uint32_t>(State.Particles.size()) : 0u;
	const uint32_t iSpawnCount = (std::min)(iCount, iAvailable);
	for (uint32_t iParticle = 0u; iParticle < iSpawnCount; ++iParticle)
	{
		PARTICLE_STATE Particle;
		Particle.fDistributionRandom = static_cast<f32_t>(Next_Random(State)) /
			static_cast<f32_t>(UINT32_MAX);
		if (Element.SourceRecipe.bEnabled)
		{
			Particle.vPosition = nullptr == pSourceEvent ? float3_t{} :
				Transform_Coord(pSourceEvent->vPosition, InverseElementWorld);
			Particle.vVelocity = nullptr == pSourceEvent ? float3_t{} :
				Transform_Normal(pSourceEvent->vVelocity, InverseElementWorld);
			Particle.vBaseSize = {};
			Particle.vSize = {};
			Particle.vBaseColor = { 1.f, 1.f, 1.f, 1.f };
			Particle.vColor = Particle.vBaseColor;
			Particle.fLifeTimeSeconds = 0.f;
			/* The trim slides the emitter's own delay so one Element can be
			   pushed later inside the cue without editing the cue itself. */
			const f32_t fEmitterElapsed = (std::max)(0.f,
				m_fSampleTimeSeconds -
				Element.Detail.Timing.fStartDelaySeconds -
				Element.SourceRecipe.fEmitterDelaySeconds *
					Desc.SourceScale.fSpawnDelay);
			const f32_t fEmitterDuration =
				Element.SourceRecipe.fEmitterDurationSeconds > 0.f ?
				Element.SourceRecipe.fEmitterDurationSeconds :
				Element.Detail.Timing.fLifeTimeSeconds;
			const f32_t fEmitterTime = fEmitterDuration > 0.f ?
				std::fmod(fEmitterElapsed, fEmitterDuration) :
				fEmitterElapsed;
			Particle.fSpawnEmitterTimeSeconds = fEmitterTime;
			Apply_SourceSpawnModules(
				Element, State, Particle, fEmitterTime, ElementWorld);
			/* Authored trim over what the modules just produced.  Scaling the
			   base size here carries through Apply_SourceUpdateModules, which
			   re-derives vSize from vBaseSize every step, and scaling the
			   lifetime stretches the whole over-life evolution with it. */
			if (!Desc.SourceScale.Is_Default())
			{
				Particle.vBaseSize = Scale3(
					Particle.vBaseSize, Desc.SourceScale.fSize);
				Particle.vSize = Particle.vBaseSize;
				Particle.fLifeTimeSeconds *= Desc.SourceScale.fLifeTime;
				/* Velocity is scaled on both the live and the base copy so the
				   update modules that re-derive from vBaseVelocity keep the
				   trimmed speed instead of snapping back on the next step. */
				Particle.vVelocity = Scale3(
					Particle.vVelocity, Desc.SourceScale.fSpeed);
				Particle.vBaseVelocity = Scale3(
					Particle.vBaseVelocity, Desc.SourceScale.fSpeed);
				/* Source rotation modules have already converted UE3 turns to
				   degrees.  Scale their initial sprite and mesh results exactly
				   once here; update rates are scaled at their integration sites
				   after per-frame module scales have been rebuilt. */
				Particle.vRotationDegrees = Scale3(
					Particle.vRotationDegrees, Desc.SourceScale.fRotation);
				Particle.vSourceMeshRotationDegrees = Scale3(
					Particle.vSourceMeshRotationDegrees,
					Desc.SourceScale.fRotation);
				Particle.vBaseColor.w *= Desc.SourceScale.fAlpha;
				Particle.vColor.w = Particle.vBaseColor.w;
			}
		}
		else
		{
			Particle.vPosition = Sample_AuthoredSpawnPosition(Desc, State);
			Particle.vVelocity = Sample_AuthoredInitialVelocity(
				Desc, State, Particle.vPosition);
			Particle.vBaseSize = float3_t(
				Desc.vStartSize.x, Desc.vStartSize.y,
				0.5f * (Desc.vStartSize.x + Desc.vStartSize.y));
			Particle.vSize = Particle.vBaseSize;
			Particle.fLifeTimeSeconds = Random_Range(
				State, Desc.vLifeTimeSeconds.x, Desc.vLifeTimeSeconds.y);
			Particle.vDynamicParameter = {};
			const f32_t* pDynamicStart = &Desc.vDynamicParameterStart.x;
			f32_t* pParticleDynamic = &Particle.vDynamicParameter.x;
			for (uint32_t iComponent = 0u; iComponent < 4u; ++iComponent)
			{
				if (0u != (Desc.iDynamicParameterComponentMask &
					(1u << iComponent)))
				{
					pParticleDynamic[iComponent] = pDynamicStart[iComponent];
				}
			}
		}
		Particle.vVelocity = Apply_ParticleEmissionModifier(
			Particle.vVelocity);
		Particle.vVelocity = Add3(
			Particle.vVelocity, Particle.vInheritedParentVelocity);
		Particle.vBaseVelocity = Particle.vVelocity;
		if (Particle.fLifeTimeSeconds <= 0.f)
			Particle.fLifeTimeSeconds = Random_Range(
				State, Desc.vLifeTimeSeconds.x, Desc.vLifeTimeSeconds.y);
		Particle.fLifeTimeSeconds = (std::max)(
			0.001f, Particle.fLifeTimeSeconds);
		Particle.iSpawnSimulationStep = m_iSimulationStep;
		Particle.SpawnRootWorld = ElementWorld;
		State.Particles.push_back(Particle);
		Queue_SpawnEvents(Element, State, State.Particles.back(), ElementWorld);
	}
}

void Client::CEffectPlayback::Queue_SpawnEvents(
	const EFFECT_ELEMENT_DESC& Element,
	ELEMENT_STATE& State,
	const PARTICLE_STATE& Particle,
	const float4x4_t& ElementWorld)
{
	for (const EFFECT_SOURCE_MODULE_DESC& Module : Element.SourceRecipe.Modules)
	{
		if (!SourceModule_Enabled(Module) ||
			!SourceClass_Matches(Module, "particlemoduleeventgenerator"))
			continue;
		for (uint32_t iEvent = 0u; iEvent < 64u; ++iEvent)
		{
			const std::string Prefix = "events[" +
				std::to_string(iEvent) + "].";
			const std::string_view Type = SourceString(Module, Prefix + "type");
			if (Type.empty())
			{
				if (0u == iEvent)
					continue;
				break;
			}
			if (Type != "epet_spawn")
				continue;
			const std::string CounterId = Module.strStableId + ":" +
				std::to_string(iEvent) + ":spawn";
			const uint32_t iTrackingCount = ++State.EventTrackingCounts[CounterId];
			const uint32_t iFrequency = static_cast<uint32_t>((std::max)(0.f,
				SourceNumber(Module, Prefix + "frequency", 0.f)));
			if (0u != iFrequency && 0u != iTrackingCount % iFrequency)
				continue;
			const float4x4_t& ParticleRoot = Element.Detail.Particle.bLocalSpace ?
				ElementWorld : Particle.SpawnRootWorld;
			float3_t EventPosition = Particle.vPosition;
			if (SourceBool(Module, Prefix + "buseorbitoffset", false))
				EventPosition = Add3(EventPosition, Particle.vOrbitOffset);
			SOURCE_PARTICLE_EVENT Event;
			Event.strType = std::string(Type);
			Event.strName = std::string(SourceString(
				Module, Prefix + "customname"));
			Event.fEmitterTimeSeconds = Particle.fSpawnEmitterTimeSeconds;
			Event.vPosition = Transform_Coord(
				EventPosition, XMLoadFloat4x4(&ParticleRoot));
			Event.vVelocity = Transform_Normal(
				Particle.vVelocity, XMLoadFloat4x4(&ParticleRoot));
			if (m_PendingSourceEvents.size() >= MAX_SOURCE_EVENTS_PER_STEP)
			{
				m_bSourceEventQueueOverflow = true;
				return;
			}
			m_PendingSourceEvents.push_back(std::move(Event));
		}
	}
}

bool_t Client::CEffectPlayback::Dispatch_SourceEvents(
	const f32_t fFixedDelta,
	const float4x4_t& RootWorld)
{
	UNREFERENCED_PARAMETER(fFixedDelta);
	if (m_bSourceEventQueueOverflow)
		return false;
	for (size_t iEvent = 0u;
		iEvent < m_PendingSourceEvents.size() &&
		iEvent < MAX_SOURCE_EVENTS_PER_STEP;
		++iEvent)
	{
		const SOURCE_PARTICLE_EVENT Event = m_PendingSourceEvents[iEvent];
		for (const EFFECT_ELEMENT_DESC& Element : Get_StagedDocument().Elements)
		{
			if (!Element.bVisible || !Is_ParticleSimulationElement(
				Element, Is_SourceVisualProgramElementAdmitted(Element)))
				continue;
			ELEMENT_STATE& State = m_States[Element.strElementId];
			for (const EFFECT_SOURCE_MODULE_DESC& Module :
				Element.SourceRecipe.Modules)
			{
				if (!SourceModule_Enabled(Module) ||
					!SourceClass_Matches(Module,
						"particlemoduleeventreceiverspawn"))
				{
					continue;
				}
				const std::string_view EventName = SourceString(Module, "eventname");
				const std::string_view EventType = SourceString(
					Module, "eventgeneratortype");
				if (EventName != Event.strName ||
					EventType != "epet_spawn" ||
					Event.strType != "epet_spawn")
				{
					continue;
				}
				const int32_t iCount = static_cast<int32_t>(std::lround(
					Evaluate_ModuleFloat(State, Module, "spawncount",
						Event.fEmitterTimeSeconds, 0.f)));
				if (iCount <= 0)
					continue;
				SOURCE_PARTICLE_EVENT Forced = Event;
				if (SourceBool(Module, "busepsyslocation", false))
					Forced.vPosition = Get_Translation(RootWorld);
				if (!SourceBool(Module, "binheritvelocity", false))
					Forced.vVelocity = {};
				else
					Forced.vVelocity = Multiply3(Forced.vVelocity,
						UE3_AxisScaleToClient(Evaluate_ModuleVector(State, Module,
							"inheritvelocityscale", Event.fEmitterTimeSeconds,
							float3_t(1.f, 1.f, 1.f))));
				Spawn_Particles(Element, State,
					static_cast<uint32_t>(iCount), RootWorld, &Forced);
				if (m_bSourceEventQueueOverflow)
					return false;
			}
		}
	}
	return !m_bSourceEventQueueOverflow;
}

void Client::CEffectPlayback::Set_SourceAnchorWorlds(
	const std::unordered_map<std::string, float4x4_t>& SourceAnchorWorlds)
{
	m_SourceAnchorWorlds = SourceAnchorWorlds;
}

void Client::CEffectPlayback::Set_SourceAnchorWorlds(
	std::unordered_map<std::string, float4x4_t>&& SourceAnchorWorlds)
{
	/* Swap returns the previous frame's buckets to the caller scratch map, so
	   FOLLOW anchor resolution reuses both maps without copying or reallocating. */
	m_SourceAnchorWorlds.swap(SourceAnchorWorlds);
}

void Client::CEffectPlayback::Initialize_ModuleRandomStates(
	const EFFECT_ELEMENT_DESC& Element,
	ELEMENT_STATE& State)
{
	State.ModuleRandomStates.clear();
	for (const EFFECT_SOURCE_MODULE_DESC& Module :
		Element.SourceRecipe.Modules)
	{
		if (!Module.strClassName.ends_with("_seeded"))
			continue;
		std::vector<std::pair<uint32_t, int32_t>> IndexedSeeds;
		for (const EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
		{
			constexpr std::string_view CanonicalPrefix =
				"randomseedinfo.randomseeds[";
			constexpr std::string_view SourceStructPrefix =
				"randomseedinfo.properties.randomseeds[";
			const std::string_view PropertyPath = Literal.strPropertyPath;
			const std::string_view Prefix =
				PropertyPath.starts_with(CanonicalPrefix) ? CanonicalPrefix :
				PropertyPath.starts_with(SourceStructPrefix) ? SourceStructPrefix :
				std::string_view{};
			if (EFFECT_SOURCE_LITERAL_KIND::NUMBER != Literal.eKind ||
				Prefix.empty())
			{
				continue;
			}
			const size_t iClose = PropertyPath.find(']', Prefix.size());
			if (std::string::npos == iClose)
				continue;
			try
			{
				IndexedSeeds.emplace_back(
					static_cast<uint32_t>(std::stoul(
						std::string(PropertyPath.substr(Prefix.size(),
							iClose - Prefix.size())))),
					static_cast<int32_t>(Literal.fNumber));
			}
			catch (...)
			{
				continue;
			}
		}
		if (IndexedSeeds.empty())
		{
			const auto HexLiteral = std::find_if(
				Module.Literals.begin(), Module.Literals.end(),
				[](const EFFECT_SOURCE_LITERAL_DESC& Literal)
				{
					return Literal.strPropertyPath == "randomseedinfo.hex" &&
						Literal.eKind == EFFECT_SOURCE_LITERAL_KIND::STRING;
				});
			if (HexLiteral != Module.Literals.end() &&
				HexLiteral->strString.size() == 64u)
			{
				std::array<uint8_t, 32u> Bytes{};
				const auto HexNibble = [](const char Value) -> int32_t
				{
					if (Value >= '0' && Value <= '9')
						return Value - '0';
					if (Value >= 'a' && Value <= 'f')
						return 10 + Value - 'a';
					if (Value >= 'A' && Value <= 'F')
						return 10 + Value - 'A';
					return -1;
				};
				bool_t bValidHex = true;
				for (size_t i = 0u; i < Bytes.size(); ++i)
				{
					const int32_t High = HexNibble(HexLiteral->strString[i * 2u]);
					const int32_t Low = HexNibble(
						HexLiteral->strString[i * 2u + 1u]);
					if (High < 0 || Low < 0)
					{
						bValidHex = false;
						break;
					}
					Bytes[i] = static_cast<uint8_t>((High << 4) | Low);
				}
				if (bValidHex)
				{
					const uint32_t Seed =
						static_cast<uint32_t>(Bytes[28u]) |
						(static_cast<uint32_t>(Bytes[29u]) << 8u) |
						(static_cast<uint32_t>(Bytes[30u]) << 16u) |
						(static_cast<uint32_t>(Bytes[31u]) << 24u);
					IndexedSeeds.emplace_back(0u, static_cast<int32_t>(Seed));
				}
			}
		}
		if (IndexedSeeds.empty())
			continue;
		std::sort(IndexedSeeds.begin(), IndexedSeeds.end(),
			[](const auto& A, const auto& B) { return A.first < B.first; });
		size_t iSelected = 0u;
		if (SourceBool(Module,
			"randomseedinfo.brandomlyselectseedarray", false))
		{
			iSelected = static_cast<size_t>(std::floor(
				UE_RandomFraction(State.iRandomState) *
				static_cast<f32_t>(IndexedSeeds.size())));
			iSelected = (std::min)(iSelected, IndexedSeeds.size() - 1u);
		}
		MODULE_RANDOM_STATE Random;
		Random.iInitialSeed = static_cast<uint32_t>(IndexedSeeds[iSelected].second);
		Random.iCurrentSeed = Random.iInitialSeed;
		Random.bResetOnEmitterLoop = SourceBool(Module,
			"randomseedinfo.bresetseedonemitterlooping", true);
		State.ModuleRandomStates.emplace(Module.strStableId, Random);
	}
}

void Client::CEffectPlayback::Reset_LoopingModuleRandomStates(
	ELEMENT_STATE& State)
{
	for (auto& [StableId, Random] : State.ModuleRandomStates)
	{
		UNREFERENCED_PARAMETER(StableId);
		if (Random.bResetOnEmitterLoop)
			Random.iCurrentSeed = Random.iInitialSeed;
	}
}

f32_t Client::CEffectPlayback::Next_ModuleRandom(
	ELEMENT_STATE& State,
	const EFFECT_SOURCE_MODULE_DESC& Module)
{
	const auto Iterator = State.ModuleRandomStates.find(Module.strStableId);
	return Iterator == State.ModuleRandomStates.end() ?
		UE_RandomFraction(State.iRandomState) :
		UE_RandomFraction(Iterator->second.iCurrentSeed);
}

f32_t Client::CEffectPlayback::Evaluate_ModuleFloat(
	ELEMENT_STATE& State,
	const EFFECT_SOURCE_MODULE_DESC& Module,
	const std::string_view PropertyPath,
	const f32_t fTime,
	const f32_t fFallback)
{
	const EFFECT_DISTRIBUTION_DESC* pDistribution =
		Find_SourceDistribution(&Module, PropertyPath);
	if (nullptr == pDistribution)
		return fFallback;
	if (Uses_ImplicitAlphaIdentity(*pDistribution, PropertyPath))
		return fFallback;
	float4_t RandomUnits{};
	if (2u == pDistribution->iOperation || 3u == pDistribution->iOperation)
		RandomUnits.x = Next_ModuleRandom(State, Module);
	return CEffectDistribution::Evaluate(
		*pDistribution, fTime, RandomUnits).x;
}

float3_t Client::CEffectPlayback::Evaluate_ModuleVector(
	ELEMENT_STATE& State,
	const EFFECT_SOURCE_MODULE_DESC& Module,
	const std::string_view PropertyPath,
	const f32_t fTime,
	const float3_t& Fallback)
{
	const EFFECT_DISTRIBUTION_DESC* pDistribution =
		Find_SourceDistribution(&Module, PropertyPath);
	if (nullptr == pDistribution)
		return Fallback;
	float4_t RandomUnits{};
	if (2u == pDistribution->iOperation)
	{
		RandomUnits.x = Next_ModuleRandom(State, Module);
		RandomUnits.y = Next_ModuleRandom(State, Module);
		RandomUnits.z = Next_ModuleRandom(State, Module);
		if (pDistribution->iComponentCount > 3u)
			RandomUnits.w = Next_ModuleRandom(State, Module);
		switch (pDistribution->iRandomLockAxes)
		{
		case 1u: RandomUnits.y = RandomUnits.x; break;
		case 2u: RandomUnits.z = RandomUnits.x; break;
		case 3u: RandomUnits.z = RandomUnits.y; break;
		case 4u:
			RandomUnits.y = RandomUnits.x;
			RandomUnits.z = RandomUnits.x;
			break;
		default: break;
		}
	}
	else if (3u == pDistribution->iOperation)
	{
		RandomUnits.x = Next_ModuleRandom(State, Module);
	}
	const float4_t Value = CEffectDistribution::Evaluate(
		*pDistribution, fTime, RandomUnits);
	return { Value.x, Value.y, Value.z };
}

void Client::CEffectPlayback::Apply_SourceSpawnModules(
	const EFFECT_ELEMENT_DESC& Element,
	ELEMENT_STATE& State,
	PARTICLE_STATE& Particle,
	const f32_t fEmitterTimeSeconds,
	const float4x4_t& ElementWorld)
{
	bool_t bHasSize = false;
	const bool_t bSourceVisualDecalParticle =
		Is_SourceVisualDecalParticle(
			Element, Is_SourceVisualProgramElementAdmitted(Element));
	for (const EFFECT_SOURCE_MODULE_DESC& Module :
		Element.SourceRecipe.Modules)
	{
		if (!SourceModule_Enabled(Module))
			continue;

		if (SourceClass_Matches(Module, "particlemodulelifetime"))
		{
			Particle.fLifeTimeSeconds += (std::max)(0.f,
				Evaluate_ModuleFloat(State, Module, "lifetime",
					fEmitterTimeSeconds, 0.f));
		}
		else if (SourceClass_Matches(Module, "particlemodulelocation"))
		{
			Particle.vPosition = Add3(Particle.vPosition,
				UE3_CentimetersToClient(Evaluate_ModuleVector(
					State, Module, "startlocation",
					fEmitterTimeSeconds, float3_t{})));
		}
		else if (SourceClass_Matches(
			Module, "particlemodulelocationdirect"))
		{
			const float3_t Location = Evaluate_ModuleVector(
				State, Module, "location", fEmitterTimeSeconds, float3_t{});
			const float3_t Offset = Evaluate_ModuleVector(
				State, Module, "locationoffset", fEmitterTimeSeconds,
				float3_t{});
			const float3_t Scale =
				Is_Warlord17090ChainTypedScaleIdentity(Element, Module) ?
				float3_t(1.f, 1.f, 1.f) : Evaluate_ModuleVector(
					State, Module, "scalefactor", fEmitterTimeSeconds,
					float3_t(1.f, 1.f, 1.f));
			Particle.vSourceDirectLocationContribution =
				UE3_CentimetersToClient(
					Add3(Multiply3(Location, Scale), Offset));
			Particle.vSourceDirectDirectionContribution =
				UE3_CentimetersToClient(Evaluate_ModuleVector(
					State, Module, "direction",
					fEmitterTimeSeconds, float3_t{}));
			Particle.vPosition = Add3(Particle.vPosition,
				Particle.vSourceDirectLocationContribution);
			Particle.vVelocity = Add3(Particle.vVelocity,
				Particle.vSourceDirectDirectionContribution);
		}
		else if (SourceClass_Matches(
			Module, "particlemodulelocationonground"))
		{
			if (!Is_SourceVisualProgramElementAdmitted(Element) &&
				!Is_PortableAuthoredParticleCarrier(Element))
				continue;
			const f32_t fSkipLocation = Evaluate_ModuleFloat(
				State, Module, "skiplocation", fEmitterTimeSeconds,
				SourceNumber(Module, "skiplocation", 0.f));
			if (fSkipLocation >= 0.5f)
				continue;
			const float3_t Adjust = UE3_CentimetersToClient(
				Evaluate_ModuleVector(State, Module, "adjustlocation",
					fEmitterTimeSeconds, float3_t{}));
			Particle.vPosition = Add3(Particle.vPosition, Adjust);
		}
		else if (SourceClass_Matches(
			Module, "particlemodulelocationbonesocket"))
		{
			std::vector<std::pair<uint32_t, std::string>> Locations;
			for (const EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
			{
				constexpr std::string_view Prefix = "sourcelocations[";
				constexpr std::string_view Suffix = "].bonesocketname";
				if (EFFECT_SOURCE_LITERAL_KIND::STRING != Literal.eKind ||
					!Literal.strPropertyPath.starts_with(Prefix) ||
					!Literal.strPropertyPath.ends_with(Suffix))
				{
					continue;
				}
				const size_t iClose = Literal.strPropertyPath.find(']', Prefix.size());
				if (std::string::npos == iClose)
					continue;
				try
				{
					Locations.emplace_back(
						static_cast<uint32_t>(std::stoul(
							Literal.strPropertyPath.substr(Prefix.size(),
								iClose - Prefix.size()))),
						Literal.strString);
				}
				catch (...)
				{
					continue;
				}
			}
			std::sort(Locations.begin(), Locations.end(),
				[](const auto& A, const auto& B) { return A.first < B.first; });
			if (!Locations.empty())
			{
				uint32_t& iNext = State.BoneSocketNextIndices[Module.strStableId];
				uint32_t iSelected = iNext++ % static_cast<uint32_t>(Locations.size());
				if (SourceString(Module, "selectionmethod").ends_with("random"))
				{
					iSelected = static_cast<uint32_t>(std::floor(
						Next_ModuleRandom(State, Module) *
						static_cast<f32_t>(Locations.size())));
					iSelected = (std::min)(iSelected,
						static_cast<uint32_t>(Locations.size() - 1u));
				}
				const uint32_t iSourceIndex = Locations[iSelected].first;
				const std::string& AnchorName = Locations[iSelected].second;
				const auto Anchor = m_SourceAnchorWorlds.find(AnchorName);
				if (Anchor != m_SourceAnchorWorlds.end())
				{
					const std::string PrefixPath = "sourcelocations[" +
						std::to_string(iSourceIndex) + "].offset.";
					const float3_t Offset = UE3_CentimetersToClient(Add3(
						float3_t(
							SourceNumber(Module, PrefixPath + "x", 0.f),
							SourceNumber(Module, PrefixPath + "y", 0.f),
							SourceNumber(Module, PrefixPath + "z", 0.f)),
						float3_t(
							SourceNumber(Module, "universaloffset.x", 0.f),
							SourceNumber(Module, "universaloffset.y", 0.f),
							SourceNumber(Module, "universaloffset.z", 0.f))));
					const matrix_t AnchorLocal = XMMatrixTranslation(
						Offset.x, Offset.y, Offset.z) *
						XMLoadFloat4x4(&Anchor->second) *
						XMMatrixInverse(nullptr, XMLoadFloat4x4(&ElementWorld));
					float4x4_t AnchorLocalStored{};
					XMStoreFloat4x4(&AnchorLocalStored, AnchorLocal);
					Particle.vPosition = Get_Translation(AnchorLocalStored);
					Particle.strSourceAnchorName = AnchorName;
					Particle.vSourceAnchorOffset = Offset;
					Particle.bUpdateSourceAnchor = SourceBool(Module,
						"bupdatepositioneachframe", false);
				}
			}
		}
		else if (SourceClass_Matches(
			Module, "particlemodulelocationprimitivesphere"))
		{
			const bool_t bPositiveX = SourceBool(Module, "positive_x", true);
			const bool_t bNegativeX = SourceBool(Module, "negative_x", true);
			const bool_t bPositiveY = SourceBool(Module, "positive_y", true);
			const bool_t bNegativeY = SourceBool(Module, "negative_y", true);
			const bool_t bPositiveZ = SourceBool(Module, "positive_z", true);
			const bool_t bNegativeZ = SourceBool(Module, "negative_z", true);
			const auto SelectDirectionAxis = [](const f32_t fRandom,
				const bool_t bPositive, const bool_t bNegative) noexcept
				{
					if (bPositive && bNegative)
						return -1.f + 2.f * fRandom;
					if (bPositive)
						return fRandom;
					if (bNegative)
						return -fRandom;
					return 0.f;
				};
			float3_t Direction(
				SelectDirectionAxis(Next_ModuleRandom(State, Module),
					bPositiveX, bNegativeX),
				SelectDirectionAxis(Next_ModuleRandom(State, Module),
					bPositiveY, bNegativeY),
				SelectDirectionAxis(Next_ModuleRandom(State, Module),
					bPositiveZ, bNegativeZ));
			if (SourceBool(Module, "surfaceonly", false))
				Direction = Normalize3(Direction);
			const f32_t fRadius = Evaluate_ModuleFloat(
				State, Module, "startradius", fEmitterTimeSeconds, 0.f);
			const float3_t Start = Evaluate_ModuleVector(
				State, Module, "startlocation", fEmitterTimeSeconds,
				float3_t{});
			const float3_t Offset = Scale3(Direction, fRadius);
			Particle.vPosition = Add3(Particle.vPosition,
				UE3_CentimetersToClient(Add3(Start, Offset)));
			if (SourceBool(Module, "velocity", false))
			{
				const f32_t fVelocityScale = Evaluate_ModuleFloat(
					State, Module, "velocityscale", fEmitterTimeSeconds, 0.f);
				Particle.vVelocity = Add3(Particle.vVelocity,
					Scale3(UE3_CentimetersToClient(Offset), fVelocityScale));
			}
		}
		else if (SourceClass_Matches(
			Module, "particlemodulelocationprimitivecylinder"))
		{
			const f32_t fRadius = Evaluate_ModuleFloat(
				State, Module, "startradius", fEmitterTimeSeconds, 0.f);
			const f32_t fHalfHeight = 0.5f * Evaluate_ModuleFloat(
				State, Module, "startheight", fEmitterTimeSeconds, 0.f);
			const f32_t fAngle = Next_ModuleRandom(State, Module) * XM_2PI;
			const f32_t fRadial = SourceBool(Module, "surfaceonly", false) ?
				fRadius : std::sqrt(Next_ModuleRandom(State, Module)) * fRadius;
			float3_t Offset(
				std::cos(fAngle) * fRadial,
				std::sin(fAngle) * fRadial,
				-fHalfHeight + 2.f * fHalfHeight *
					Next_ModuleRandom(State, Module));
			const std::string_view Axis = SourceString(Module, "heightaxis");
			if (Axis.ends_with("_x"))
				Offset = float3_t(Offset.z, Offset.x, Offset.y);
			else if (Axis.ends_with("_y"))
				Offset = float3_t(Offset.x, Offset.z, Offset.y);
			const float3_t Start = Evaluate_ModuleVector(
				State, Module, "startlocation", fEmitterTimeSeconds,
				float3_t{});
			Particle.vPosition = Add3(Particle.vPosition,
				UE3_CentimetersToClient(Add3(Start, Offset)));
			if (SourceBool(Module, "velocity", false))
			{
				const f32_t fVelocityScale = Evaluate_ModuleFloat(
					State, Module, "velocityscale", fEmitterTimeSeconds, 0.f);
				Particle.vVelocity = Add3(Particle.vVelocity,
					Scale3(UE3_CentimetersToClient(Offset), fVelocityScale));
			}
		}
		else if (SourceClass_Matches(
			Module, "particlemodulelocationprimitivecylinderspin"))
		{
			const f32_t fRadius = Evaluate_ModuleFloat(
				State, Module, "startradius", fEmitterTimeSeconds, 0.f);
			const f32_t fHalfHeight = 0.5f * Evaluate_ModuleFloat(
				State, Module, "startheight", fEmitterTimeSeconds, 0.f);
			f32_t fAngle = Next_ModuleRandom(State, Module) * XM_2PI;
			if (!SourceBool(Module, "negative_x", true) &&
				std::cos(fAngle) < 0.f)
			{
				fAngle = XM_PI - fAngle;
			}
			if (!SourceBool(Module, "positive_x", true) &&
				std::cos(fAngle) > 0.f)
			{
				fAngle = XM_PI - fAngle;
			}
			if (!SourceBool(Module, "negative_y", true) &&
				std::sin(fAngle) < 0.f)
			{
				fAngle = -fAngle;
			}
			if (!SourceBool(Module, "positive_y", true) &&
				std::sin(fAngle) > 0.f)
			{
				fAngle = -fAngle;
			}
			fAngle += XMConvertToRadians(Evaluate_ModuleFloat(
				State, Module, "startcylinderrot", fEmitterTimeSeconds, 0.f) +
				Evaluate_ModuleFloat(State, Module, "spinangle",
					fEmitterTimeSeconds, 0.f) * fEmitterTimeSeconds);
			const f32_t fRadial = SourceBool(Module, "surfaceonly", false) ?
				fRadius : std::sqrt(Next_ModuleRandom(State, Module)) * fRadius;
			float3_t Offset(
				std::cos(fAngle) * fRadial,
				std::sin(fAngle) * fRadial,
				-fHalfHeight + 2.f * fHalfHeight *
					Next_ModuleRandom(State, Module));
			const std::string_view Axis = SourceString(Module, "heightaxis");
			if (Axis.ends_with("_x"))
				Offset = float3_t(Offset.z, Offset.x, Offset.y);
			else if (Axis.ends_with("_y"))
				Offset = float3_t(Offset.x, Offset.z, Offset.y);
			const float3_t Start = Evaluate_ModuleVector(
				State, Module, "startlocation", fEmitterTimeSeconds,
				float3_t{});
			Particle.vPosition = Add3(Particle.vPosition,
				UE3_CentimetersToClient(Add3(Start, Offset)));
			if (SourceBool(Module, "velocity", false))
			{
				const f32_t fVelocityScale = Evaluate_ModuleFloat(
					State, Module, "velocityscale", fEmitterTimeSeconds, 0.f);
				Particle.vVelocity = Add3(Particle.vVelocity,
					Scale3(UE3_CentimetersToClient(Offset), fVelocityScale));
			}
		}
		else if (SourceClass_Matches(
			Module, "particlemodulelocationcirclesurface"))
		{
			const f32_t fRadius = Evaluate_ModuleFloat(
				State, Module, "startradius", fEmitterTimeSeconds, 0.f);
			const uint32_t iSplit = static_cast<uint32_t>((std::max)(
				0.f, SourceNumber(Module, "splitcirclecount", 0.f)));
			const f32_t fStep = 0u == iSplit ?
				Next_ModuleRandom(State, Module) :
				static_cast<f32_t>(std::floor(
					Next_ModuleRandom(State, Module) * iSplit)) /
				static_cast<f32_t>(iSplit);
			const f32_t fStartRotation = Evaluate_ModuleFloat(
				State, Module, "startrot", fEmitterTimeSeconds, 0.f);
			const bool_t bHalfMode = SourceBool(Module, "bhalfmode", false);
			const f32_t fAngle = fStep * (bHalfMode ? XM_PI : XM_2PI) +
				fStartRotation * XM_2PI +
				(SourceBool(Module, "bnegativeaxis", false) ? XM_PI : 0.f);
			const f32_t fCosRadius = std::cos(fAngle) * fRadius;
			const f32_t fSinRadius = std::sin(fAngle) * fRadius;
			const std::string_view SurfaceAxis =
				SourceString(Module, "surfaceaxis");
			float3_t Offset(fCosRadius, fSinRadius, 0.f);
			if (SurfaceAxis.ends_with("_yz"))
				Offset = float3_t(0.f, fCosRadius, fSinRadius);
			else if (SurfaceAxis.ends_with("_zx"))
				Offset = float3_t(fCosRadius, 0.f, fSinRadius);
			const float3_t Start = Evaluate_ModuleVector(
				State, Module, "startlocation", fEmitterTimeSeconds,
				float3_t{});
			Particle.vPosition = Add3(Particle.vPosition,
				UE3_CentimetersToClient(Add3(Start, Offset)));
			if (SourceBool(Module, "velocity", false))
			{
				Particle.vVelocity = Add3(Particle.vVelocity,
					Scale3(UE3_CentimetersToClient(Offset),
						Evaluate_ModuleFloat(State, Module,
							"velocityscale", fEmitterTimeSeconds, 0.f)));
			}
		}
		else if (SourceClass_Matches(Module, "particlemodulevelocity"))
		{
			const float3_t Velocity = Evaluate_ModuleVector(
				State, Module, "startvelocity", fEmitterTimeSeconds,
				float3_t{});
			float3_t StartVelocity = UE3_CentimetersToClient(Velocity);
			if (Element.Detail.Particle.bLocalSpace &&
				SourceBool(Module, "binworldspace", false))
			{
				const matrix_t InverseRoot = XMMatrixInverse(
					nullptr, XMLoadFloat4x4(&ElementWorld));
				StartVelocity = Transform_Normal(StartVelocity, InverseRoot);
			}
			Particle.vVelocity = Add3(Particle.vVelocity, StartVelocity);
			const f32_t fRadial = Evaluate_ModuleFloat(
				State, Module, "startvelocityradial", fEmitterTimeSeconds, 0.f);
			Particle.vVelocity = Add3(Particle.vVelocity,
				Scale3(Normalize3(Particle.vPosition), fRadial * 0.01f));
		}
		else if (SourceClass_Matches(
			Module, "particlemodulevelocityinheritparent"))
		{
			float3_t ParentVelocity = m_vParentVelocity;
			if (Element.Detail.Particle.bLocalSpace)
			{
				ParentVelocity = Transform_Normal(ParentVelocity,
					XMMatrixInverse(nullptr, XMLoadFloat4x4(&ElementWorld)));
			}
			Particle.vInheritedParentVelocity = Add3(
				Particle.vInheritedParentVelocity,
				Multiply3(ParentVelocity, UE3_AxisScaleToClient(
					Evaluate_ModuleVector(State, Module, "scale",
						fEmitterTimeSeconds, float3_t(1.f, 1.f, 1.f)))));
		}
		else if (SourceClass_Matches(
			Module, "particlemodulevectorfieldscale"))
		{
			Particle.fVectorFieldScale *= Evaluate_ModuleFloat(
				State, Module, "scale", fEmitterTimeSeconds, 1.f);
		}
		else if (SourceClass_Matches(Module, "particlemodulesize"))
		{
			const EFFECT_DISTRIBUTION_DESC* pStartSizeDistribution =
				Find_SourceDistribution(&Module, "startsize");
			/* A null Cascade StartSize CDO carries no executable size.  Keep
			   Detail.Particle as the already-converted runtime fallback instead of
			   treating the empty descriptor as an authored zero-size particle. */
			if (nullptr == pStartSizeDistribution ||
				Is_SourceNullCdoDistribution(*pStartSizeDistribution, 3u))
			{
				continue;
			}
			Particle.vBaseSize = Add3(Particle.vBaseSize,
				UE3_StartSizeToClient(Element, Evaluate_ModuleVector(
					State, Module, "startsize", fEmitterTimeSeconds,
					float3_t{})));
			bHasSize = true;
		}
		else if (SourceClass_Matches(Module, "particlemodulecolor"))
		{
			const float3_t Color = Evaluate_ModuleVector(
				State, Module, "startcolor", fEmitterTimeSeconds,
				float3_t(1.f, 1.f, 1.f));
			const f32_t fAlpha = Evaluate_ModuleFloat(
				State, Module, "startalpha", fEmitterTimeSeconds, 1.f);
			Particle.vBaseColor = { Color.x, Color.y, Color.z, fAlpha };
			Particle.vColor = Particle.vBaseColor;
		}
		else if (SourceClass_Matches(
			Module, "particlemodulecoloroverlife"))
		{
			const float3_t Color = Evaluate_ModuleVector(
				State, Module, "coloroverlife", 0.f,
				float3_t(1.f, 1.f, 1.f));
			const f32_t fAlpha = Evaluate_ModuleFloat(
				State, Module, "alphaoverlife", 0.f, 1.f);
			Particle.vBaseColor = { Color.x, Color.y, Color.z, fAlpha };
			Particle.vColor = Particle.vBaseColor;
		}
		else if (SourceClass_Matches(
			Module, "particlemodulecolorscaleoverlife"))
		{
			const f32_t fTime = SourceBool(Module, "bemittertime", false) ?
				fEmitterTimeSeconds : 0.f;
			const float3_t Scale = Evaluate_ModuleVector(
				State, Module, "colorscaleoverlife", fTime,
				float3_t(1.f, 1.f, 1.f));
			const f32_t fAlpha = Evaluate_ModuleFloat(
				State, Module, "alphascaleoverlife", fTime, 1.f);
			Particle.vColor = {
				Particle.vColor.x * Scale.x,
				Particle.vColor.y * Scale.y,
				Particle.vColor.z * Scale.z,
				Particle.vColor.w * fAlpha
			};
		}
		else if (SourceClass_Matches(Module, "particlemodulerotation"))
		{
			f32_t& fYawOrSpriteRoll = bSourceVisualDecalParticle ?
				Particle.vRotationDegrees.y : Particle.vRotationDegrees.z;
			fYawOrSpriteRoll += Evaluate_ModuleFloat(
				State, Module, "startrotation", fEmitterTimeSeconds, 0.f) * 360.f;
		}
		else if (SourceClass_Matches(
			Module, "particlemodulemeshrotation"))
		{
			Particle.vSourceMeshRotationDegrees = Add3(
				Particle.vSourceMeshRotationDegrees,
				Scale3(Evaluate_ModuleVector(State, Module, "startrotation",
					fEmitterTimeSeconds, float3_t{}), 360.f));
		}
		else if (SourceClass_Matches(
			Module, "particlemodulerotationrate"))
		{
			f32_t& fYawOrSpriteRollRate = bSourceVisualDecalParticle ?
				Particle.vRotationRateDegreesPerSecond.y :
				Particle.vRotationRateDegreesPerSecond.z;
			fYawOrSpriteRollRate +=
				Evaluate_ModuleFloat(State, Module, "startrotationrate",
					fEmitterTimeSeconds, 0.f) * 360.f;
		}
		else if (SourceClass_Matches(
			Module, "particlemodulemeshrotationrate"))
		{
			Particle.vSourceMeshRotationRateDegreesPerSecond = Add3(
				Particle.vSourceMeshRotationRateDegreesPerSecond,
				Scale3(Evaluate_ModuleVector(State, Module, "startrotationrate",
					fEmitterTimeSeconds, float3_t{}), 360.f));
		}
		else if (SourceClass_Matches(Module, "particlemodulecameraoffset"))
		{
			const f32_t fValue = Evaluate_ModuleFloat(
				State, Module, "cameraoffset", 0.f, 0.f) * 0.01f;
			const std::string_view Method = SourceString(Module, "updatemethod");
			if (Method.ends_with("additive"))
				Particle.fCameraOffset += fValue;
			else if (Method.ends_with("scalar"))
				Particle.fCameraOffset *= fValue;
			else
				Particle.fCameraOffset = fValue;
		}
		else if (SourceClass_Matches(Module, "particlemodulesubuv"))
		{
			Particle.fSubImageIndex = Evaluate_ModuleFloat(
				State, Module, "subimageindex", 0.f, 0.f);
		}
		else if (SourceClass_Matches(
			Module, "particlemoduleparameterdynamic"))
		{
			f32_t* pValues = &Particle.vDynamicParameter.x;
			for (uint32_t iParameter = 0u; iParameter < 4u; ++iParameter)
			{
				const DYNAMIC_PARAMETER_PROPERTY_PATHS& Paths =
					DYNAMIC_PARAMETER_PATHS[iParameter];
				const f32_t fTime = SourceBool(Module,
					Paths.strUseEmitterTime, false) ?
					fEmitterTimeSeconds : 0.f;
				pValues[iParameter] = Evaluate_ModuleFloat(
					State, Module, Paths.strValue, fTime, 0.f);
				if (SourceBool(Module,
					Paths.strScaleVelocityByValue, false))
				{
					pValues[iParameter] *= Length3(Particle.vVelocity);
				}
			}
		}
		else if (SourceClass_Matches(
			Module, "particlemodulespawnperunit"))
		{
			/* Distance-based spawn is consumed once per emitter step by
			   Consume_SourceSpawnPerUnit. It has no per-particle mutation. */
			continue;
		}
		else if (SourceClass_Matches(Module, "particlemoduleorbit"))
		{
			Particle.vOrbitOffset = Add3(Particle.vOrbitOffset,
				UE3_CentimetersToClient(Evaluate_ModuleVector(
					State, Module, "offsetamount", 0.f, float3_t{})));
			Particle.vSourceOrbitRotationDegrees = Add3(
				Particle.vSourceOrbitRotationDegrees,
				Scale3(Evaluate_ModuleVector(State, Module, "rotationamount",
					0.f, float3_t{}), 360.f));
			Particle.vSourceOrbitRotationRateDegreesPerSecond = Add3(
				Particle.vSourceOrbitRotationRateDegreesPerSecond,
				Scale3(Evaluate_ModuleVector(State, Module, "rotationrateamount",
					0.f, float3_t{}), 360.f));
		}
	}
	if (!bHasSize)
	{
		const EFFECT_PARTICLE_DESC& Desc = Element.Detail.Particle;
		Particle.vBaseSize = float3_t(
			Desc.vStartSize.x, Desc.vStartSize.y,
			0.5f * (Desc.vStartSize.x + Desc.vStartSize.y));
	}
	Particle.vSize = Particle.vBaseSize;
}

void Client::CEffectPlayback::Update_Particles(
	const EFFECT_ELEMENT_DESC& Element,
	ELEMENT_STATE& State,
	const f32_t fFixedDelta,
	const float4x4_t& RootWorld)
{
	const f32_t fEmitterElapsed = (std::max)(0.f,
		m_fSampleTimeSeconds -
		Element.Detail.Timing.fStartDelaySeconds -
		(Element.SourceRecipe.bEnabled ?
			Element.SourceRecipe.fEmitterDelaySeconds : 0.f));
	const f32_t fEmitterDuration =
		Element.SourceRecipe.fEmitterDurationSeconds > 0.f ?
		Element.SourceRecipe.fEmitterDurationSeconds :
		Element.Detail.Timing.fLifeTimeSeconds;
	f32_t fEmitterTime = fEmitterElapsed;
	if (Element.SourceRecipe.bEnabled && fEmitterDuration > 0.f)
	{
		const bool_t bFiniteComplete =
			0u != Element.SourceRecipe.iEmitterLoopCount &&
			fEmitterElapsed >= fEmitterDuration * static_cast<f32_t>(
				Element.SourceRecipe.iEmitterLoopCount);
		fEmitterTime = bFiniteComplete ? fEmitterDuration :
			std::fmod(fEmitterElapsed, fEmitterDuration);
	}

	/* A particle's spawn-owning step evaluates age zero.  On later steps,
	   reject the terminal age before running another module update, then commit
	   the next age before evaluating distributions.  This keeps the normalized
	   age exported by Rebuild_Frame identical to the age used by ParameterDynamic
	   and the other over-life modules without changing the existing per-step
	   velocity/position integration count. */
	std::erase_if(State.Particles,
		[this, fFixedDelta](const PARTICLE_STATE& Particle)
		{
			return Particle.iSpawnSimulationStep != m_iSimulationStep &&
				Particle.fAgeSeconds + fFixedDelta >=
					Particle.fLifeTimeSeconds;
		});
	for (PARTICLE_STATE& Particle : State.Particles)
	{
		if (!Can_EvaluateElementWorld(Element))
			return;
		if (Particle.iSpawnSimulationStep != m_iSimulationStep)
			Particle.fAgeSeconds += fFixedDelta;
		const float4x4_t ElementWorld = Evaluate_ElementWorld(
			Element, m_fSampleTimeSeconds, RootWorld);
		const f32_t fNormalizedAge = Clamp01(
			Particle.fAgeSeconds / Particle.fLifeTimeSeconds);
		if (Element.SourceRecipe.bEnabled)
		{
			Apply_SourceUpdateModules(Element, State, Particle, fEmitterTime,
				fNormalizedAge, fFixedDelta, ElementWorld);
		}
		else
		{
			Particle.vVelocityScale = { 1.f, 1.f, 1.f };
			Particle.vVelocity = Add3(Particle.vVelocity,
				Scale3(Element.Detail.Particle.vAcceleration, fFixedDelta));
		}
		const float3_t EffectiveVelocity = Apply_TargetAttractor(
			Element, Particle, fNormalizedAge, fFixedDelta,
			ElementWorld, RootWorld);
		Particle.vPosition = Add3(Particle.vPosition,
			Scale3(EffectiveVelocity, fFixedDelta));
	}
}

float3_t Client::CEffectPlayback::Apply_TargetAttractor(
	const EFFECT_ELEMENT_DESC& Element,
	PARTICLE_STATE& Particle,
	const f32_t fNormalizedAge,
	const f32_t fFixedDelta,
	const float4x4_t& ElementWorld,
	const float4x4_t& RootWorld)
{
	const EFFECT_PARTICLE_TARGET_ATTRACTOR_DESC& Attractor =
		Element.Detail.Particle.TargetAttractor;
	const float3_t SourceLocalVelocity = Multiply3(
		Particle.vVelocity, Particle.vVelocityScale);
	if (!Attractor.bEnabled ||
		fNormalizedAge < Attractor.vActiveNormalized.x ||
		fNormalizedAge > Attractor.vActiveNormalized.y)
	{
		Particle.vTargetAttractorWorldVelocity = {};
		return SourceLocalVelocity;
	}

	const float4x4_t& ParticleRoot =
		Element.Detail.Particle.bLocalSpace ?
			ElementWorld : Particle.SpawnRootWorld;
	const matrix_t ParticleRootMatrix = XMLoadFloat4x4(&ParticleRoot);
	const matrix_t InverseParticleRoot = XMMatrixInverse(
		nullptr, ParticleRootMatrix);
	const float3_t SourceWorldVelocity = Transform_Normal(
		SourceLocalVelocity, ParticleRootMatrix);
	float3_t TargetWorld{};
	if (EFFECT_PARTICLE_ATTRACTOR_TARGET_SPACE::ROOT_LOCAL ==
		Attractor.eTargetSpace)
	{
		TargetWorld = Transform_Coord(
			Attractor.vTargetOffset, XMLoadFloat4x4(&RootWorld));
	}
	else
	{
		TargetWorld = Transform_Coord(
			Attractor.vTargetOffset, XMLoadFloat4x4(&ElementWorld));
	}

	const float3_t PositionWorld = Transform_Coord(
		Particle.vPosition, ParticleRootMatrix);
	const float3_t Delta = Subtract3(TargetWorld, PositionWorld);
	const f32_t fDistance = Length3(Delta);
	float3_t EffectiveWorldVelocity = Add3(
		SourceWorldVelocity, Particle.vTargetAttractorWorldVelocity);
	if (fDistance <= Attractor.fConvergenceRadius)
	{
		Particle.vPosition = Transform_Coord(
			TargetWorld, InverseParticleRoot);
		Particle.vTargetAttractorWorldVelocity =
			Scale3(SourceWorldVelocity, -1.f);
		return {};
	}

	const float3_t Direction = Scale3(Delta, 1.f / fDistance);
	const float3_t Tangent = Normalize3(float3_t(
		-Direction.z, 0.f, Direction.x));
	const float3_t Acceleration = Add3(
		Scale3(Direction, Attractor.fRadialAcceleration),
		Scale3(Tangent, Attractor.fTangentialAcceleration));
	EffectiveWorldVelocity = Add3(
		EffectiveWorldVelocity, Scale3(Acceleration, fFixedDelta));

	/* Use the stopping distance implied by the authored speed/acceleration as
	   the arrival zone. Damping is therefore independent of frame rate and does
	   not introduce another guessed distance field. */
	const f32_t fStoppingDistance = Attractor.fRadialAcceleration > 1.e-6f ?
		Attractor.fMaximumSpeed * Attractor.fMaximumSpeed /
			(2.f * Attractor.fRadialAcceleration) :
		Attractor.fConvergenceRadius;
	const f32_t fBrakingRadius = (std::max)(
		Attractor.fConvergenceRadius, fStoppingDistance);
	const f32_t fArrivalWeight = Clamp01(
		(fBrakingRadius -
			(fDistance - Attractor.fConvergenceRadius)) / fBrakingRadius);
	const f32_t fDamping = (std::max)(0.f,
		1.f - Attractor.fArrivalDamping * fArrivalWeight * fFixedDelta);
	EffectiveWorldVelocity = Scale3(EffectiveWorldVelocity, fDamping);

	const f32_t fSpeed = Length3(EffectiveWorldVelocity);
	if (fSpeed > Attractor.fMaximumSpeed)
	{
		EffectiveWorldVelocity = Scale3(
			EffectiveWorldVelocity, Attractor.fMaximumSpeed / fSpeed);
	}

	/* Preserve the authored tangent until the final radial step.  At that
	   boundary, remove the tangent as well so a particle cannot orbit forever
	   just outside the capture radius without ever entering it. */
	const f32_t fRadialSpeed =
		EffectiveWorldVelocity.x * Direction.x +
		EffectiveWorldVelocity.y * Direction.y +
		EffectiveWorldVelocity.z * Direction.z;
	const f32_t fMaximumRadialSpeed =
		(fDistance - Attractor.fConvergenceRadius) / fFixedDelta;
	if (fRadialSpeed > fMaximumRadialSpeed)
	{
		EffectiveWorldVelocity = Scale3(Direction, fMaximumRadialSpeed);
	}
	Particle.vTargetAttractorWorldVelocity = Subtract3(
		EffectiveWorldVelocity, SourceWorldVelocity);
	return Transform_Normal(EffectiveWorldVelocity, InverseParticleRoot);
}

void Client::CEffectPlayback::Apply_SourceUpdateModules(
	const EFFECT_ELEMENT_DESC& Element,
	ELEMENT_STATE& State,
	PARTICLE_STATE& Particle,
	const f32_t fEmitterTimeSeconds,
	const f32_t fNormalizedAge,
	const f32_t fFixedDelta,
	const float4x4_t& ElementWorld)
{
	Particle.vSize = Particle.vBaseSize;
	Particle.vColor = Particle.vBaseColor;
	Particle.vVelocityScale = { 1.f, 1.f, 1.f };
	Particle.vRotationRateScale = { 1.f, 1.f, 1.f };
	Particle.vSourceMeshRotationRateScale = { 1.f, 1.f, 1.f };
	const bool_t bSourceVisualDecalParticle =
		Is_SourceVisualDecalParticle(
			Element, Is_SourceVisualProgramElementAdmitted(Element));

	for (const EFFECT_SOURCE_MODULE_DESC& Module :
		Element.SourceRecipe.Modules)
	{
		if (!SourceModule_Enabled(Module))
			continue;

		if (SourceClass_Matches(
			Module, "particlemodulelocationdirect"))
		{
			/* Cascade LocationDirect evaluates its curves over particle relative
			   time.  Replacing only the module's previous contribution preserves
			   every other source module and makes the source 0.9 -> 1.0 return
			   segment execute instead of freezing at its spawn sample. */
			const float3_t Location = Evaluate_ModuleVector(
				State, Module, "location", fNormalizedAge, float3_t{});
			const float3_t Offset = Evaluate_ModuleVector(
				State, Module, "locationoffset", fNormalizedAge, float3_t{});
			const float3_t Scale =
				Is_Warlord17090ChainTypedScaleIdentity(Element, Module) ?
				float3_t(1.f, 1.f, 1.f) : Evaluate_ModuleVector(
					State, Module, "scalefactor", fNormalizedAge,
					float3_t(1.f, 1.f, 1.f));
			const float3_t DirectLocation = UE3_CentimetersToClient(
				Add3(Multiply3(Location, Scale), Offset));
			const float3_t DirectDirection = UE3_CentimetersToClient(
				Evaluate_ModuleVector(State, Module, "direction",
					fNormalizedAge, float3_t{}));
			Particle.vPosition = Add3(Particle.vPosition,
				Subtract3(DirectLocation,
					Particle.vSourceDirectLocationContribution));
			Particle.vVelocity = Add3(Particle.vVelocity,
				Subtract3(DirectDirection,
					Particle.vSourceDirectDirectionContribution));
			Particle.vSourceDirectLocationContribution = DirectLocation;
			Particle.vSourceDirectDirectionContribution = DirectDirection;
		}
		else if (SourceClass_Matches(Module, "particlemodulelocationbonesocket"))
		{
			if (!Particle.bUpdateSourceAnchor ||
				Particle.strSourceAnchorName.empty())
			{
				continue;
			}
			const auto Anchor = m_SourceAnchorWorlds.find(
				Particle.strSourceAnchorName);
			if (Anchor == m_SourceAnchorWorlds.end())
				continue;
			const matrix_t AnchorLocal = XMMatrixTranslation(
				Particle.vSourceAnchorOffset.x,
				Particle.vSourceAnchorOffset.y,
				Particle.vSourceAnchorOffset.z) *
				XMLoadFloat4x4(&Anchor->second) *
				XMMatrixInverse(nullptr, XMLoadFloat4x4(&ElementWorld));
			float4x4_t Stored{};
			XMStoreFloat4x4(&Stored, AnchorLocal);
			Particle.vPosition = Get_Translation(Stored);
		}
		else if (SourceClass_Matches(
			Module, "particlemodulelocalvectorfield"))
		{
			const std::string_view AssetId =
				SourceString(Module, "vectorfield.assetid");
			std::shared_ptr<const SOURCE_VECTOR_FIELD> Field;
			if (nullptr != m_pPreparedResources)
			{
				const auto PreparedField = m_pPreparedResources->VectorFields.find(
					std::string(AssetId));
				if (PreparedField != m_pPreparedResources->VectorFields.end())
					Field = PreparedField->second;
			}
			if (nullptr == Field)
				continue;
			float3_t Rotation(
				SourceNumber(Module, "relativerotation.x", 0.f),
				SourceNumber(Module, "relativerotation.y", 0.f),
				SourceNumber(Module, "relativerotation.z", 0.f));
			for (const EFFECT_SOURCE_MODULE_DESC& RotationModule :
				Element.SourceRecipe.Modules)
			{
				if (!SourceModule_Enabled(RotationModule) ||
					!SourceClass_Matches(RotationModule,
						"particlemodulevectorfieldrotationrate"))
				{
					continue;
				}
				Rotation = Add3(Rotation, Scale3(float3_t(
					SourceNumber(RotationModule, "rotationrate.x", 0.f),
					SourceNumber(RotationModule, "rotationrate.y", 0.f),
					SourceNumber(RotationModule, "rotationrate.z", 0.f)),
					fEmitterTimeSeconds));
			}
			const float3_t FieldScale = Scale3(float3_t(
				SourceNumber(Module, "relativescale3d.x", 1.f),
				SourceNumber(Module, "relativescale3d.y", 1.f),
				SourceNumber(Module, "relativescale3d.z", 1.f)), 0.01f);
			const float3_t Translation = Scale3(float3_t(
				SourceNumber(Module, "relativetranslation.x", 0.f),
				SourceNumber(Module, "relativetranslation.y", 0.f),
				SourceNumber(Module, "relativetranslation.z", 0.f)), 0.01f);
			const matrix_t FieldRotation = XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(Rotation.x),
				XMConvertToRadians(Rotation.y),
				XMConvertToRadians(Rotation.z));
			const matrix_t FieldToEmitter = XMMatrixScaling(
				FieldScale.x, FieldScale.y, FieldScale.z) *
				FieldRotation * XMMatrixTranslation(
					Translation.x, Translation.y, Translation.z);
			const float3_t FieldPosition = Transform_Coord(
				Particle.vPosition, XMMatrixInverse(nullptr, FieldToEmitter));
			float3_t UV(
				0.5f * (FieldPosition.x + 1.f),
				0.5f * (FieldPosition.y + 1.f),
				0.5f * (FieldPosition.z + 1.f));
			const float3_t Tiling(
				SourceBool(Module, "btilex", false) ? 1.f : 0.f,
				SourceBool(Module, "btiley", false) ? 1.f : 0.f,
				SourceBool(Module, "btilez", false) ? 1.f : 0.f);
			if (Tiling.x > 0.f) UV.x -= std::floor(UV.x);
			if (Tiling.y > 0.f) UV.y -= std::floor(UV.y);
			if (Tiling.z > 0.f) UV.z -= std::floor(UV.z);
			const float3_t VolumeSize(
				static_cast<f32_t>(Field->iSizeX),
				static_cast<f32_t>(Field->iSizeY),
				static_cast<f32_t>(Field->iSizeZ));
			const float3_t AxisWeight(
				Clamp01(UV.x * VolumeSize.x) *
					Clamp01((1.f - UV.x) * VolumeSize.x),
				Clamp01(UV.y * VolumeSize.y) *
					Clamp01((1.f - UV.y) * VolumeSize.y),
				Clamp01(UV.z * VolumeSize.z) *
					Clamp01((1.f - UV.z) * VolumeSize.z));
			const f32_t fDistanceWeight = (std::min)(AxisWeight.x,
				(std::min)(AxisWeight.y, AxisWeight.z));
			if (fDistanceWeight <= 0.f)
				continue;
			f32_t fPerParticleScale = 1.f;
			for (const EFFECT_SOURCE_MODULE_DESC& ScaleModule :
				Element.SourceRecipe.Modules)
			{
				if (!SourceModule_Enabled(ScaleModule) ||
					!SourceClass_Matches(ScaleModule,
						"particlemodulevectorfieldscaleoverlife"))
				{
					continue;
				}
				fPerParticleScale *= Evaluate_ModuleFloat(State, ScaleModule,
					"scaleoverlife", fNormalizedAge, 1.f);
			}
			const float3_t Sample = Transform_Normal(
				Sample_VectorField(*Field, UV, Tiling), FieldRotation);
			const f32_t fIntensity = SourceNumber(Module, "intensity", 1.f) *
				Particle.fVectorFieldScale * fPerParticleScale;
			const f32_t fTightness = Clamp01(
				SourceNumber(Module, "tightness", 0.f));
			const float3_t FieldVelocity = Scale3(
				Sample, fIntensity * 0.01f);
			Particle.vVelocity = Lerp3Clamped(Particle.vVelocity,
				FieldVelocity, fDistanceWeight * fTightness);
			Particle.vVelocity = Add3(Particle.vVelocity,
				Scale3(FieldVelocity, fDistanceWeight * fFixedDelta));
		}
		else if (SourceClass_Matches(Module, "particlemoduleacceleration"))
		{
			const bool_t bEffectAcceleration =
				Module.strClassName.starts_with("ef");
			const std::string_view PropertyPath =
				bEffectAcceleration ? "acceldata" : "acceleration";
			const f32_t fTime = bEffectAcceleration ? fNormalizedAge :
				Particle.fSpawnEmitterTimeSeconds;
			float3_t Acceleration = UE3_CentimetersToClient(
				Evaluate_ModuleVector(State, Module, PropertyPath, fTime,
					float3_t{}));
			if (SourceBool(Module, "balwaysinworldspace", false))
			{
				const float4x4_t& ParticleRoot =
					Element.Detail.Particle.bLocalSpace ?
						ElementWorld : Particle.SpawnRootWorld;
				const matrix_t InverseRoot = XMMatrixInverse(
					nullptr, XMLoadFloat4x4(&ParticleRoot));
				Acceleration = Transform_Normal(Acceleration, InverseRoot);
			}
			Particle.vVelocity = Add3(Particle.vVelocity,
				Scale3(Acceleration, fFixedDelta));
		}
		else if (SourceClass_Matches(
			Module, "particlemodulevelocityoverlifetime"))
		{
			const float3_t Velocity = Evaluate_ModuleVector(
				State, Module, "veloverlife", fNormalizedAge,
				float3_t(1.f, 1.f, 1.f));
			if (SourceBool(Module, "absolute", false) ||
				SourceBool(Module, "babsolute", false))
			{
				Particle.vVelocity = UE3_CentimetersToClient(Velocity);
				Particle.vVelocityScale = { 1.f, 1.f, 1.f };
			}
			else
			{
				Particle.vVelocityScale = Multiply3(
					Particle.vVelocityScale,
					UE3_AxisScaleToClient(Velocity));
			}
		}
		else if (SourceClass_Matches(
			Module, "particlemodulesizemultiplylife"))
		{
			Particle.vSize = Multiply3(Particle.vSize,
				UE3_SizeToClient(Element, Evaluate_ModuleVector(
					State, Module, "lifemultiplier", fNormalizedAge,
					float3_t(1.f, 1.f, 1.f))));
		}
		else if (SourceClass_Matches(
			Module, "particlemodulesizescalebytime"))
		{
			Particle.vSize = Multiply3(Particle.vSize,
				UE3_SizeToClient(Element, Evaluate_ModuleVector(
					State, Module, "sizescalebytime", Particle.fAgeSeconds,
					float3_t(1.f, 1.f, 1.f))));
		}
		else if (SourceClass_Matches(Module, "particlemodulesizescale"))
		{
			Particle.vSize = Multiply3(Particle.vSize,
				UE3_SizeToClient(Element, Evaluate_ModuleVector(
					State, Module, "sizescale", fNormalizedAge,
					float3_t(1.f, 1.f, 1.f))));
		}
		else if (SourceClass_Matches(
			Module, "particlemodulecoloroverlife"))
		{
			const float3_t Color = Evaluate_ModuleVector(
				State, Module, "coloroverlife", fNormalizedAge,
				float3_t(1.f, 1.f, 1.f));
			const f32_t fAlpha = Evaluate_ModuleFloat(
				State, Module, "alphaoverlife", fNormalizedAge, 1.f);
			Particle.vColor = { Color.x, Color.y, Color.z, fAlpha };
		}
		else if (SourceClass_Matches(
			Module, "particlemodulecolorscaleoverlife"))
		{
			const f32_t fTime = SourceBool(Module, "bemittertime", false) ?
				fEmitterTimeSeconds : fNormalizedAge;
			const float3_t Scale = Evaluate_ModuleVector(
				State, Module, "colorscaleoverlife", fTime,
				float3_t(1.f, 1.f, 1.f));
			const f32_t fAlpha = Evaluate_ModuleFloat(
				State, Module, "alphascaleoverlife", fTime, 1.f);
			Particle.vColor = {
				Particle.vColor.x * Scale.x,
				Particle.vColor.y * Scale.y,
				Particle.vColor.z * Scale.z,
				Particle.vColor.w * fAlpha
			};
		}
		else if (SourceClass_Matches(
			Module, "particlemodulemeshrotationratemultiplylife"))
		{
			Particle.vSourceMeshRotationRateScale = Multiply3(
				Particle.vSourceMeshRotationRateScale,
				Evaluate_ModuleVector(State, Module, "lifemultiplier",
					fNormalizedAge, float3_t(1.f, 1.f, 1.f)));
		}
		else if (SourceClass_Matches(
			Module, "particlemodulerotationratemultiplylife"))
		{
			f32_t& fYawOrSpriteRollRateScale = bSourceVisualDecalParticle ?
				Particle.vRotationRateScale.y : Particle.vRotationRateScale.z;
			fYawOrSpriteRollRateScale *= Evaluate_ModuleFloat(
				State, Module, "lifemultiplier", fNormalizedAge, 1.f);
		}
		else if (SourceClass_Matches(
			Module, "particlemodulemeshrotationrateoverlife"))
		{
			Particle.vSourceMeshRotationDegrees = Add3(
				Particle.vSourceMeshRotationDegrees,
				Scale3(Evaluate_ModuleVector(State, Module, "rotrate",
					fNormalizedAge, float3_t{}), 360.f * fFixedDelta *
						Element.Detail.Particle.SourceScale.fRotation));
		}
		else if (SourceClass_Matches(Module, "particlemodulecameraoffset"))
		{
			const f32_t fValue = Evaluate_ModuleFloat(State, Module,
				"cameraoffset", fNormalizedAge, 0.f) * 0.01f;
			const std::string_view Method = SourceString(Module, "updatemethod");
			if (Method.ends_with("additive"))
				Particle.fCameraOffset += fValue;
			else if (Method.ends_with("scalar"))
				Particle.fCameraOffset *= fValue;
			else
				Particle.fCameraOffset = fValue;
		}
		else if (SourceClass_Matches(Module, "particlemodulesubuv"))
		{
			Particle.fSubImageIndex = Evaluate_ModuleFloat(
				State, Module, "subimageindex", fNormalizedAge, 0.f);
		}
		else if (SourceClass_Matches(
			Module, "particlemoduleparameterdynamic"))
		{
			f32_t* pValues = &Particle.vDynamicParameter.x;
			for (uint32_t iParameter = 0u; iParameter < 4u; ++iParameter)
			{
				const DYNAMIC_PARAMETER_PROPERTY_PATHS& Paths =
					DYNAMIC_PARAMETER_PATHS[iParameter];
				if (SourceBool(Module, Paths.strSpawnTimeOnly, false))
					continue;
				const f32_t fTime = SourceBool(Module,
					Paths.strUseEmitterTime, false) ?
					fEmitterTimeSeconds : fNormalizedAge;
				pValues[iParameter] = Evaluate_ModuleFloat(State, Module,
					Paths.strValue, fTime, 0.f);
				if (SourceBool(Module,
					Paths.strScaleVelocityByValue, false))
				{
					pValues[iParameter] *= Length3(Particle.vVelocity);
				}
			}
		}
		else if (SourceClass_Matches(Module, "particlemoduleorbit"))
		{
			if (SourceBool(Module,
				"offsetoptions.bprocessduringupdate", false))
			{
				const f32_t fTime = SourceBool(Module,
					"offsetoptions.buseemittertime", false) ?
					fEmitterTimeSeconds : fNormalizedAge;
				Particle.vOrbitOffset = Add3(Particle.vOrbitOffset,
					UE3_CentimetersToClient(Evaluate_ModuleVector(
						State, Module, "offsetamount", fTime, float3_t{})));
			}
			if (SourceBool(Module,
				"rotationoptions.bprocessduringupdate", false))
			{
				const f32_t fTime = SourceBool(Module,
					"rotationoptions.buseemittertime", false) ?
					fEmitterTimeSeconds : fNormalizedAge;
				Particle.vSourceOrbitRotationDegrees = Add3(
					Particle.vSourceOrbitRotationDegrees,
					Scale3(Evaluate_ModuleVector(State, Module,
						"rotationamount", fTime, float3_t{}), 360.f));
			}
			if (SourceBool(Module,
				"rotationrateoptions.bprocessduringupdate", false))
			{
				const f32_t fTime = SourceBool(Module,
					"rotationrateoptions.buseemittertime", false) ?
					fEmitterTimeSeconds : fNormalizedAge;
				Particle.vSourceOrbitRotationRateDegreesPerSecond = Add3(
					Particle.vSourceOrbitRotationRateDegreesPerSecond,
					Scale3(Evaluate_ModuleVector(State, Module,
						"rotationrateamount", fTime, float3_t{}), 360.f));
			}
		}
		else if (SourceClass_Matches(Module, "particlemodulevortex"))
		{
			const f32_t fPower = SourceNumber(Module, "power", 1.f) *
				Evaluate_ModuleFloat(State, Module,
					"poweracceleration", fNormalizedAge, 1.f);
			const float3_t Radial = Normalize3(Particle.vPosition);
			const float3_t Tangent(-Radial.z, 0.f, Radial.x);
			Particle.vVelocity = Add3(Particle.vVelocity,
				Scale3(Tangent, fPower * 0.01f * fFixedDelta));
		}
	}

	const f32_t fSourceRotationScale =
		Element.Detail.Particle.SourceScale.fRotation;
	Particle.vRotationDegrees = Add3(Particle.vRotationDegrees,
		Scale3(Multiply3(Particle.vRotationRateDegreesPerSecond,
			Particle.vRotationRateScale), fFixedDelta * fSourceRotationScale));
	Particle.vSourceMeshRotationDegrees = Add3(
		Particle.vSourceMeshRotationDegrees,
		Scale3(Multiply3(Particle.vSourceMeshRotationRateDegreesPerSecond,
			Particle.vSourceMeshRotationRateScale),
			fFixedDelta * fSourceRotationScale));
}

void Client::CEffectPlayback::Sample_Trail(
	const EFFECT_ELEMENT_DESC& Element,
	ELEMENT_STATE& State,
	const f32_t fFixedDelta,
	const float4x4_t& RootWorld)
{
	const bool_t bDirectAuthoredFlowRibbon01 =
		!Is_SourceVisualProgramElementAdmitted(Element) &&
		Has_EffectFlowRibbon01TrailContract(Element);
	bool_t bCascadeRibbon = bDirectAuthoredFlowRibbon01;
	bool_t bAnimationTrail = false;
	bool_t bBakedEdgeAnimationTrail = false;
	if (Is_SourceVisualProgramElementAdmitted(Element))
	{
		/* Reconstructed Artist and admitted BA documents share this consumer.
		   Reconstructed execution owns its already-validated source closure;
		   generic visual programs additionally require the immutable row/target
		   family match established during token staging. */
		if (m_bReconstructedSourceRuntimeActive)
			bCascadeRibbon = Is_CascadeRibbonTarget(Element);
		if (nullptr != m_pSourceVisualProgramProjection)
		{
			const EFFECT_VISUAL_PROGRAM_ROW* const pRow =
				m_pSourceVisualProgramProjection->Find_RowByTargetElementId(
					Element.strElementId);
			const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* const pSupplemental =
				m_pSourceVisualProgramProjection->
					Find_SupplementalElementByTargetElementId(
						Element.strElementId);
			bCascadeRibbon = bCascadeRibbon ||
				(nullptr != pRow && pRow->eFamily ==
					EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON) ||
				(nullptr != pSupplemental && pSupplemental->eFamily ==
					EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON);
			bAnimationTrail = nullptr != pSupplemental &&
				pSupplemental->eFamily ==
					EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL;
			bBakedEdgeAnimationTrail = bAnimationTrail &&
				pSupplemental->AnimationTrailPacket.has_value() &&
				2u == pSupplemental->AnimationTrailPacket->iPacketVersion;
		}
	}
	const bool_t bSourceVisualTrail = bCascadeRibbon || bAnimationTrail;
	const bool_t bFlowRibbon01 = bCascadeRibbon &&
		Has_EffectFlowRibbon01TrailContract(Element);
	const bool_t bRibbonLiquid01ParentDefault =
		Element.SourceRecipe.bEnabled &&
		Element.Material.Execution.bEnabled &&
		Element.Material.Execution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
		Element.Material.Execution.iOpcode == 20u;
	if (bBakedEdgeAnimationTrail)
	{
		State.TrailPoints.clear();
		State.fTrailSampleAccumulator = 0.f;
		State.fTrailCumulativeDistance = 0.f;
		return;
	}
	constexpr f32_t SOURCE_VISUAL_RIBBON_TIME_EPSILON_SECONDS = 1e-6f;
	const auto EvaluateDeterministicFloat = [&Element](
		const char_t* pModuleClass,
		const std::string_view PropertyPath,
		const f32_t fTime,
		f32_t& fOutValue)
	{
		const EFFECT_SOURCE_MODULE_DESC* pModule =
			Find_SourceModule(Element, pModuleClass);
		const EFFECT_DISTRIBUTION_DESC* pDistribution =
			Find_SourceDistribution(pModule, PropertyPath);
		if (nullptr == pDistribution || 1u != pDistribution->iOperation ||
			Uses_ImplicitAlphaIdentity(*pDistribution, PropertyPath))
		{
			return false;
		}
		const f32_t fValue = CEffectDistribution::Evaluate(
			*pDistribution, fTime, float4_t{}).x;
		if (!std::isfinite(fValue))
			return false;
		fOutValue = fValue;
		return true;
	};
	const auto EvaluateDeterministicVector = [&Element](
		const char_t* pModuleClass,
		const std::string_view PropertyPath,
		const f32_t fTime,
		const float3_t& Fallback,
		float3_t& vOutValue)
	{
		const EFFECT_SOURCE_MODULE_DESC* pModule =
			Find_SourceModule(Element, pModuleClass);
		const EFFECT_DISTRIBUTION_DESC* pDistribution =
			Find_SourceDistribution(pModule, PropertyPath);
		if (nullptr == pDistribution || 1u != pDistribution->iOperation)
			return false;
		const bool_t bImplicitIdentity =
			pDistribution->LookupTable.empty() && pDistribution->Keys.empty() &&
			pDistribution->vDefaultMinimum.x == 0.f &&
			pDistribution->vDefaultMinimum.y == 0.f &&
			pDistribution->vDefaultMinimum.z == 0.f &&
			pDistribution->vDefaultMaximum.x == 0.f &&
			pDistribution->vDefaultMaximum.y == 0.f &&
			pDistribution->vDefaultMaximum.z == 0.f;
		if (bImplicitIdentity)
		{
			vOutValue = Fallback;
			return true;
		}
		const float4_t Value = CEffectDistribution::Evaluate(
			*pDistribution, fTime, float4_t{});
		if (!std::isfinite(Value.x) || !std::isfinite(Value.y) ||
			!std::isfinite(Value.z))
		{
			return false;
		}
		vOutValue = { Value.x, Value.y, Value.z };
		return true;
	};
	const auto EvaluateIdentityFloat = [&Element](
		const char_t* pModuleClass,
		const std::string_view PropertyPath,
		const f32_t fTime,
		const f32_t fFallback,
		f32_t& fOutValue)
	{
		const EFFECT_SOURCE_MODULE_DESC* pModule =
			Find_SourceModule(Element, pModuleClass);
		const EFFECT_DISTRIBUTION_DESC* pDistribution =
			Find_SourceDistribution(pModule, PropertyPath);
		if (nullptr == pDistribution || 1u != pDistribution->iOperation)
			return false;
		if (Uses_ImplicitAlphaIdentity(*pDistribution, PropertyPath))
		{
			fOutValue = fFallback;
			return true;
		}
		const f32_t fValue = CEffectDistribution::Evaluate(
			*pDistribution, fTime, float4_t{}).x;
		if (!std::isfinite(fValue))
			return false;
		fOutValue = fValue;
		return true;
	};
	const auto UpdatePointPayload = [this, &Element, bFlowRibbon01,
		bRibbonLiquid01ParentDefault,
		&EvaluateDeterministicFloat, &EvaluateDeterministicVector,
		&EvaluateIdentityFloat](
		EFFECT_EVALUATED_TRAIL_POINT& Point)
	{
		Point.vSourceColor = { 1.f, 1.f, 1.f, 1.f };
		Point.vDynamicParameter = {};
		Point.fSourceWidth = 0.f;
		Point.iSourceColorComponentMask = 0u;
		Point.iDynamicParameterComponentMask = 0u;

		if (bFlowRibbon01)
		{
			float3_t StartColor{};
			float3_t ColorScale{};
			f32_t fStartAlpha = 0.f;
			f32_t fAlphaScale = 0.f;
			const bool_t bColorExact = EvaluateDeterministicVector(
				"particlemodulecolor", "startcolor", 0.f,
				float3_t(1.f, 1.f, 1.f), StartColor) &&
				EvaluateDeterministicVector(
					"particlemodulecolorscaleoverlife", "colorscaleoverlife",
					Point.fNormalizedAge, float3_t(1.f, 1.f, 1.f),
					ColorScale) &&
				EvaluateIdentityFloat(
					"particlemodulecolor", "startalpha", 0.f, 1.f,
					fStartAlpha) &&
				EvaluateIdentityFloat("particlemodulecolorscaleoverlife",
					"alphascaleoverlife", Point.fNormalizedAge, 1.f,
					fAlphaScale);
			if (bColorExact)
			{
				Point.vSourceColor = {
					StartColor.x * ColorScale.x,
					StartColor.y * ColorScale.y,
					StartColor.z * ColorScale.z,
					fStartAlpha * fAlphaScale };
				if (std::isfinite(Point.vSourceColor.x) &&
					std::isfinite(Point.vSourceColor.y) &&
					std::isfinite(Point.vSourceColor.z) &&
					std::isfinite(Point.vSourceColor.w))
				{
					Point.iSourceColorComponentMask = 0x0fu;
				}
			}

			float3_t StartSize{};
			float3_t WidthScale(1.f, 1.f, 1.f);
			if (EvaluateDeterministicVector(
					"particlemodulesize", "startsize", 0.f, float3_t{},
					StartSize))
			{
				const auto ApplyWidthScale = [&EvaluateDeterministicVector,
					&WidthScale](const char_t* pClass, const char_t* pProperty,
					const f32_t fTime)
				{
					float3_t Scale{};
					if (EvaluateDeterministicVector(
							pClass, pProperty, fTime,
							float3_t(1.f, 1.f, 1.f), Scale))
					{
						WidthScale = Multiply3(WidthScale, Scale);
					}
				};
				ApplyWidthScale("particlemodulesizescalebytime",
					"sizescalebytime", Point.fNormalizedAge *
						Element.Detail.Trail.fPointLifeTimeSeconds);
				ApplyWidthScale("particlemodulesizescale", "sizescale",
					Point.fNormalizedAge);
				ApplyWidthScale("particlemodulesizemultiplylife",
					"lifemultiplier", Point.fNormalizedAge);
				Point.fSourceWidth = std::abs(StartSize.x * WidthScale.x) * 0.01f;
			}
		}
		else
		{
			f32_t fStartAlpha = 0.f;
			f32_t fAlphaScale = 0.f;
			const bool_t bStartAlphaValid = bRibbonLiquid01ParentDefault ?
				EvaluateIdentityFloat("particlemodulecolor", "startalpha",
					0.f, 1.f, fStartAlpha) :
				EvaluateDeterministicFloat("particlemodulecolor", "startalpha",
					0.f, fStartAlpha);
			const bool_t bAlphaScaleValid = bRibbonLiquid01ParentDefault ?
				EvaluateIdentityFloat("particlemodulecolorscaleoverlife",
					"alphascaleoverlife", Point.fNormalizedAge, 1.f,
					fAlphaScale) :
				EvaluateDeterministicFloat("particlemodulecolorscaleoverlife",
					"alphascaleoverlife", Point.fNormalizedAge, fAlphaScale);
			if (bStartAlphaValid && bAlphaScaleValid &&
				std::isfinite(fStartAlpha * fAlphaScale))
			{
				Point.vSourceColor.w = fStartAlpha * fAlphaScale;
				Point.iSourceColorComponentMask = 1u << 3u;
			}
		}
		if (!bFlowRibbon01 && !Element.SourceRecipe.bEnabled &&
			Element.Material.Execution.bEnabled &&
			Element.Material.Execution.eBackend ==
				EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
			Element.Material.Execution.iOpcode == 9u &&
			(Element.Material.Execution.iParticleColorConsumedMask & 0x08u) != 0u)
		{
			/* Ordinary authored Ribbon points do not own the reconstructed source
			   modules.  Supply the bounded generic alpha carrier that opcode 9
			   consumes without manufacturing any suppressed DynamicParameter lane. */
			const EFFECT_COLOR_DESC Color = Evaluate_Color(
				Element, Point.fNormalizedAge);
			Point.vSourceColor.w = (std::max)(0.f,
				Color.vColorMultiply.w) *
				(std::max)(0.f, 1.f - Point.fNormalizedAge);
			Point.iSourceColorComponentMask |= 0x08u;
		}

		const EFFECT_SOURCE_MODULE_DESC* pDynamic = Find_SourceModule(
			Element, "particlemoduleparameterdynamic");
		if (nullptr == pDynamic)
			return;
		f32_t* const pValues = &Point.vDynamicParameter.x;
		for (uint32_t iParameter = 0u; iParameter < 4u; ++iParameter)
		{
			const DYNAMIC_PARAMETER_PROPERTY_PATHS& Paths =
				DYNAMIC_PARAMETER_PATHS[iParameter];
			/* These branches require particle spawn/emitter time or velocity state;
			   a geometry-only trail point cannot manufacture that authority. */
			if (SourceBool(*pDynamic, Paths.strSpawnTimeOnly, false) ||
				SourceBool(*pDynamic, Paths.strUseEmitterTime, false) ||
				SourceBool(*pDynamic,
					Paths.strScaleVelocityByValue, false))
			{
				continue;
			}
			f32_t fValue = 0.f;
			if (EvaluateDeterministicFloat("particlemoduleparameterdynamic",
				Paths.strValue, Point.fNormalizedAge, fValue))
			{
				pValues[iParameter] = fValue;
				Point.iDynamicParameterComponentMask |= 1u << iParameter;
			}
		}
	};

	for (EFFECT_EVALUATED_TRAIL_POINT& Point : State.TrailPoints)
	{
		Point.fNormalizedAge += fFixedDelta /
			Element.Detail.Trail.fPointLifeTimeSeconds;
	}
	std::erase_if(State.TrailPoints,
		[bSourceVisualTrail](const EFFECT_EVALUATED_TRAIL_POINT& Point)
		{
			return Point.fNormalizedAge +
				(bSourceVisualTrail ?
					SOURCE_VISUAL_RIBBON_TIME_EPSILON_SECONDS : 0.f) >= 1.f;
		});
	for (EFFECT_EVALUATED_TRAIL_POINT& Point : State.TrailPoints)
		UpdatePointPayload(Point);
	if (State.TrailPoints.empty())
		State.fTrailCumulativeDistance = 0.f;

	const f32_t fLocalTime = m_fSampleTimeSeconds -
		Element.Detail.Timing.fStartDelaySeconds;
	const f32_t fEmitterElapsed = bSourceVisualTrail &&
		Element.SourceRecipe.bEnabled ?
		fLocalTime - Element.SourceRecipe.fEmitterDelaySeconds : fLocalTime;
	const f32_t fEmissionDuration = bDirectAuthoredFlowRibbon01 ?
		Element.Detail.Timing.fLifeTimeSeconds :
		(bSourceVisualTrail &&
			Element.SourceRecipe.fEmitterDurationSeconds > 0.f ?
			(0u == Element.SourceRecipe.iEmitterLoopCount ?
				Element.Detail.Timing.fLifeTimeSeconds :
				Element.SourceRecipe.fEmitterDurationSeconds *
				static_cast<f32_t>(Element.SourceRecipe.iEmitterLoopCount)) :
			Element.Detail.Timing.fLifeTimeSeconds);
	if (fEmitterElapsed < 0.f || fEmitterElapsed > fEmissionDuration)
	{
		return;
	}

	State.fTrailSampleAccumulator += fFixedDelta;
	if (State.fTrailSampleAccumulator +
		(bSourceVisualTrail ?
			SOURCE_VISUAL_RIBBON_TIME_EPSILON_SECONDS : 0.f) <
		Element.Detail.Trail.fSampleIntervalSeconds)
	{
		return;
	}
	if (bSourceVisualTrail)
	{
		/* Keep the fractional remainder.  Resetting to zero quantizes the
		   typed 40 Hz Ribbon to 30 Hz on the 60 Hz fixed simulation clock. */
		State.fTrailSampleAccumulator = (std::max)(0.f,
			State.fTrailSampleAccumulator -
			Element.Detail.Trail.fSampleIntervalSeconds);
	}
	else
	{
		/* Preserve the legacy Trail cadence outside source-visual program
		   admission until that public contract has its own regression evidence. */
		State.fTrailSampleAccumulator = 0.f;
	}
	if (!Can_EvaluateElementWorld(Element))
		return;
	const float4x4_t World = Evaluate_ElementWorld(
		Element, m_fSampleTimeSeconds, RootWorld);
	const float3_t Position = Get_Translation(World);
	if (!State.TrailPoints.empty() &&
		DistanceSquared(Position, State.TrailPoints.back().vWorldPosition) <
		Element.Detail.Trail.fMinimumDistance *
		Element.Detail.Trail.fMinimumDistance)
	{
		return;
	}
	if (!State.TrailPoints.empty())
	{
		const f32_t fSegmentDistance = std::sqrt(DistanceSquared(
			Position, State.TrailPoints.back().vWorldPosition));
		if (!std::isfinite(fSegmentDistance))
			return;
		State.fTrailCumulativeDistance += fSegmentDistance;
	}
	EFFECT_EVALUATED_TRAIL_POINT Point;
	Point.vWorldPosition = Position;
	Point.fCumulativeDistance = State.fTrailCumulativeDistance;
	UpdatePointPayload(Point);
	State.TrailPoints.push_back(std::move(Point));
	while (State.TrailPoints.size() > Element.Detail.Trail.iMaxPoints)
		State.TrailPoints.erase(State.TrailPoints.begin());
}

bool_t Client::CEffectPlayback::Build_BakedAnimationTrail(
	const EFFECT_ELEMENT_DESC& Element,
	const float4x4_t& RootWorld,
	EFFECT_EVALUATED_TRAIL& OutTrail) const
{
	if (nullptr == m_pSourceVisualProgramProjection)
		return false;
	const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* const pSupplemental =
		m_pSourceVisualProgramProjection->
			Find_SupplementalElementByTargetElementId(Element.strElementId);
	if (nullptr == pSupplemental ||
		pSupplemental->eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL ||
		!pSupplemental->AnimationTrailPacket.has_value() ||
		2u != pSupplemental->AnimationTrailPacket->iPacketVersion)
	{
		return false;
	}
	const EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_PACKET& Packet =
		*pSupplemental->AnimationTrailPacket;
	const EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY* const pHistory =
		m_pSourceVisualProgramProjection->Find_BakedEdgeHistory(
			Packet.strHistoryId);
	OutTrail = {};
	OutTrail.pElement = &Element;
	if (nullptr == pHistory || pHistory->Samples.size() < 2u ||
		pHistory->strHistorySha256 != Packet.strHistorySha256)
	{
		return true;
	}

	const double fLocalTime = static_cast<double>(m_fSampleTimeSeconds) -
		Packet.fLocalTimeSeconds;
	if (fLocalTime < 0.0 ||
		fLocalTime > Packet.fPlaybackClampSeconds + 1e-6)
		return true;
	const double fClampedTime = (std::min)(
		fLocalTime, Packet.fPlaybackClampSeconds);
	const auto Upper = std::upper_bound(
		pHistory->Samples.begin(), pHistory->Samples.end(), fClampedTime,
		[](const double Time, const auto& Sample)
		{
			return Time < Sample.fRelativeTimeSeconds;
		});
	const size_t iCompleteSampleCount = static_cast<size_t>(
		std::distance(pHistory->Samples.begin(), Upper));
	OutTrail.EdgePairs.reserve(iCompleteSampleCount + 1u);
	const float4x4_t ElementWorld = Evaluate_ElementWorld(
		Element, m_fSampleTimeSeconds, RootWorld);
	const matrix_t WorldMatrix = XMLoadFloat4x4(&ElementWorld);
	const auto AppendSample = [this, &Element, &OutTrail, &WorldMatrix,
		fClampedTime](
		const EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_SAMPLE& Sample)
	{
		EFFECT_EVALUATED_TRAIL_EDGE_PAIR Pair;
		Pair.vFirstEdgeWorld = Transform_BakedEdgeUE3Cm(
			Sample.vFirstEdgeUE3Cm, WorldMatrix);
		Pair.vControlPointWorld = Transform_BakedEdgeUE3Cm(
			Sample.vControlPointUE3Cm, WorldMatrix);
		Pair.vSecondEdgeWorld = Transform_BakedEdgeUE3Cm(
			Sample.vSecondEdgeUE3Cm, WorldMatrix);
		Pair.Payload.vWorldPosition = Pair.vControlPointWorld;
		const double fPointLifeTime = (std::max)(1e-6,
			static_cast<double>(Element.Detail.Trail.fPointLifeTimeSeconds));
		Pair.Payload.fNormalizedAge = static_cast<f32_t>((std::min)(
			0.999999, (std::max)(0.0,
				(fClampedTime - Sample.fRelativeTimeSeconds) / fPointLifeTime)));
		if (!OutTrail.EdgePairs.empty())
		{
			const float3_t& Previous =
				OutTrail.EdgePairs.back().vControlPointWorld;
			const f32_t fDistance = std::sqrt(DistanceSquared(
				Pair.vControlPointWorld, Previous));
			if (std::isfinite(fDistance))
			{
				Pair.Payload.fCumulativeDistance =
					OutTrail.EdgePairs.back().Payload.fCumulativeDistance +
					fDistance;
			}
		}
		const EFFECT_COLOR_DESC Color = Evaluate_Color(
			Element, Pair.Payload.fNormalizedAge);
		Pair.Payload.vSourceColor.w = (std::max)(0.f,
			Color.vColorMultiply.w) *
			(1.f - Pair.Payload.fNormalizedAge);
		Pair.Payload.iSourceColorComponentMask = 0x08u;
		OutTrail.EdgePairs.push_back(std::move(Pair));
	};

	for (size_t iSample = 0u; iSample < iCompleteSampleCount; ++iSample)
		AppendSample(pHistory->Samples[iSample]);
	if (Upper != pHistory->Samples.end() && iCompleteSampleCount > 0u)
	{
		const auto& Previous = pHistory->Samples[iCompleteSampleCount - 1u];
		if (fClampedTime > Previous.fRelativeTimeSeconds + 1e-9)
		{
			EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_SAMPLE Interpolated;
			if (Interpolate_BakedEdgeHistorySample(
					*pHistory, fClampedTime, Interpolated))
				AppendSample(Interpolated);
		}
	}
	return true;
}

bool_t Client::CEffectPlayback::Evaluate_BakedEdgeLightWorldPosition(
	const EFFECT_ELEMENT_DESC& Element,
	const float4x4_t& RootWorld,
	float3_t& vOutWorldPosition) const
{
	if (nullptr == m_pSourceVisualProgramProjection)
		return false;
	const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* const pSupplemental =
		m_pSourceVisualProgramProjection->
			Find_SupplementalElementByTargetElementId(Element.strElementId);
	if (nullptr == pSupplemental ||
		pSupplemental->eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::LIGHT_PARTICLE ||
		!pSupplemental->BakedEdgeLightPacket.has_value())
		return false;
	const EFFECT_VISUAL_PROGRAM_BAKED_EDGE_LIGHT_PACKET& Packet =
		*pSupplemental->BakedEdgeLightPacket;
	const double fActionTimeSeconds = static_cast<double>(m_fSampleTimeSeconds);
	if (fActionTimeSeconds < Packet.fActiveStartSeconds - 1e-6 ||
		fActionTimeSeconds > Packet.fActiveEndSeconds + 1e-6 ||
		Packet.eLane != EFFECT_VISUAL_PROGRAM_BAKED_EDGE_LANE::FIRST_EDGE)
		return false;
	const EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY* const pHistory =
		m_pSourceVisualProgramProjection->Find_BakedEdgeHistory(
			Packet.strHistoryId);
	if (nullptr == pHistory ||
		pHistory->strHistorySha256 != Packet.strHistorySha256 ||
		pHistory->strCoordinateBasis != Packet.strCoordinateBasis)
		return false;
	EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_SAMPLE Sample;
	const double fHistoryTimeSeconds = (std::min)(
		fActionTimeSeconds, Packet.fHistoryPlaybackClampSeconds);
	if (!Interpolate_BakedEdgeHistorySample(
			*pHistory, fHistoryTimeSeconds, Sample))
		return false;
	const float4x4_t ElementWorld = Evaluate_ElementWorld(
		Element, m_fSampleTimeSeconds, RootWorld);
	vOutWorldPosition = Transform_BakedEdgeUE3Cm(
		Sample.vFirstEdgeUE3Cm, XMLoadFloat4x4(&ElementWorld));
	return std::isfinite(vOutWorldPosition.x) &&
		std::isfinite(vOutWorldPosition.y) &&
		std::isfinite(vOutWorldPosition.z);
}

void Client::CEffectPlayback::Sample_AfterImages(
	const EFFECT_ELEMENT_DESC& Element,
	ELEMENT_STATE& State,
	const f32_t fFixedDelta,
	const float4x4_t& RootWorld)
{
	for (AFTERIMAGE_STATE& AfterImage : State.AfterImages)
		AfterImage.fAgeSeconds += fFixedDelta;
	std::erase_if(State.AfterImages,
		[&Element](const AFTERIMAGE_STATE& AfterImage)
		{
			return AfterImage.fAgeSeconds >=
				Element.Detail.Timing.fAfterImageSeconds;
		});

	const f32_t fLocalTime = m_fSampleTimeSeconds -
		Element.Detail.Timing.fStartDelaySeconds;
	if (fLocalTime < 0.f ||
		fLocalTime > Element.Detail.Timing.fLifeTimeSeconds)
	{
		return;
	}
	State.fAfterImageAccumulator += fFixedDelta;
	if (State.fAfterImageAccumulator <
		Element.Detail.AfterImage.fSampleIntervalSeconds)
	{
		return;
	}
	State.fAfterImageAccumulator = 0.f;
	if (!Can_EvaluateElementWorld(Element))
		return;
	State.AfterImages.push_back({
		Evaluate_ElementWorld(Element, m_fSampleTimeSeconds, RootWorld), 0.f });
	while (State.AfterImages.size() > Element.Detail.AfterImage.iMaxCopies)
		State.AfterImages.erase(State.AfterImages.begin());
}

bool_t Client::CEffectPlayback::Can_EvaluateElementWorld(
	const EFFECT_ELEMENT_DESC& Element) const
{
	const EFFECT_DOCUMENT_DESC& Document = Get_StagedDocument();
	const auto MasterIndex = m_TransformMasterIndices.find(
		Element.strElementId);
	if (MasterIndex != m_TransformMasterIndices.end())
	{
		if (MasterIndex->second >= Document.Elements.size())
			return false;
		return Can_EvaluateElementWorld(
			Document.Elements[MasterIndex->second]);
	}
	const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Attachment =
		Element.ActionCueAttachment;
	if (!Attachment.bEnabled)
		return true;
	if (Attachment.bFollow)
	{
		return m_SourceAnchorWorlds.contains(
			Attachment.strRuntimeAnchorSlotId);
	}
	const auto Iterator = m_States.find(Element.strElementId);
	return Iterator != m_States.end() &&
		Iterator->second.bActionRootCaptured;
}

float4x4_t Client::CEffectPlayback::Evaluate_ElementWorld(
	const EFFECT_ELEMENT_DESC& Element,
	const f32_t fSampleTimeSeconds,
	const float4x4_t& RootWorld) const
{
	const EFFECT_DOCUMENT_DESC& Document = Get_StagedDocument();
	const auto MasterIndex = m_TransformMasterIndices.find(
		Element.strElementId);
	if (MasterIndex != m_TransformMasterIndices.end() &&
		MasterIndex->second < Document.Elements.size())
	{
		return Evaluate_ElementWorld(
			Document.Elements[MasterIndex->second],
			fSampleTimeSeconds, RootWorld);
	}
	const EFFECT_DETAIL_DESC& Detail = Element.Detail;
	const f32_t fLocalTime = (std::max)(0.f,
		fSampleTimeSeconds - Detail.Timing.fStartDelaySeconds);
	const f32_t T = Clamp01(fLocalTime / Detail.Timing.fLifeTimeSeconds);
	const EFFECT_LINEAR_LERP_DESC& Lerp = Detail.LinearLerp;

	float3_t Position = Lerp.bPosition ?
		Lerp3(Detail.Transform.vPosition, Lerp.vEndPosition, T) :
		Detail.Transform.vPosition;
	const float3_t StartVelocity = Detail.Transform.vVelocityPerSecond;
	const float3_t EndVelocity = Lerp.bVelocity ?
		Lerp.vEndVelocityPerSecond : StartVelocity;
	Position = Add3(Position, Add3(
		Scale3(StartVelocity, fLocalTime),
		Scale3(Add3(EndVelocity, Scale3(StartVelocity, -1.f)),
			0.5f * fLocalTime * T)));

	const float3_t Rotation = Lerp.bRotation ?
		Lerp3(Detail.Transform.vRotationDegrees,
			Lerp.vEndRotationDegrees, T) :
		Detail.Transform.vRotationDegrees;
	const float3_t Revolution = Lerp.bRevolution ?
		Lerp3(Detail.Transform.vRevolutionDegreesPerSecond,
			Lerp.vEndRevolutionDegreesPerSecond, T) :
		Detail.Transform.vRevolutionDegreesPerSecond;
	const float3_t Scale = Lerp.bScale ?
		Lerp3(Detail.Transform.vScale, Lerp.vEndScale, T) :
		Detail.Transform.vScale;

	const matrix_t Local =
		XMMatrixScaling(Scale.x, Scale.y, Scale.z) *
		XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(Rotation.x + Revolution.x * fLocalTime),
			XMConvertToRadians(Rotation.y + Revolution.y * fLocalTime),
			XMConvertToRadians(Rotation.z + Revolution.z * fLocalTime)) *
		XMMatrixTranslation(Position.x, Position.y, Position.z);
	matrix_t Parent = XMLoadFloat4x4(&RootWorld);
	const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Attachment =
		Element.ActionCueAttachment;
	if (Attachment.bEnabled)
	{
		if (Attachment.bFollow)
		{
			const auto Iterator = m_SourceAnchorWorlds.find(
				Attachment.strRuntimeAnchorSlotId);
			if (Iterator != m_SourceAnchorWorlds.end())
				Parent = XMLoadFloat4x4(&Iterator->second);
		}
		else
		{
			const auto Iterator = m_States.find(Element.strElementId);
			if (Iterator != m_States.end() &&
				Iterator->second.bActionRootCaptured)
			{
				Parent = XMLoadFloat4x4(
					&Iterator->second.ActionRootWorld);
			}
		}
	}
	if (Is_ParticleSimulationElement(
		Element, Is_SourceVisualProgramElementAdmitted(Element)))
	{
		const EFFECT_PARTICLE_SYSTEM_DESC& ParticleSystem =
			Get_StagedDocument().ParticleSystem;
		Parent = XMMatrixScaling(
			ParticleSystem.fUniformScaleMultiplier,
			ParticleSystem.fUniformScaleMultiplier,
			ParticleSystem.fUniformScaleMultiplier) *
			XMMatrixRotationY(XMConvertToRadians(
				ParticleSystem.fYawOffsetDegrees)) *
			Parent;
	}
	matrix_t SnapshotRootSourceBasis = XMMatrixIdentity();
	if (Attachment.bEnabled && !Attachment.bFollow)
	{
		SnapshotRootSourceBasis = XMMatrixRotationY(XMConvertToRadians(
			Attachment.fSnapshotRootSourceBasisYawDegrees));
	}
	float4x4_t Result{};
	XMStoreFloat4x4(&Result, Local * SnapshotRootSourceBasis * Parent);
	return Result;
}

f32_t Client::CEffectPlayback::Evaluate_SourceFloat(
	const EFFECT_ELEMENT_DESC& Element,
	const char_t* pModuleClass,
	const char_t* pPropertyPath,
	const f32_t fTime,
	const f32_t fRandomUnit,
	const f32_t fFallback) const
{
	const EFFECT_SOURCE_MODULE_DESC* pModule =
		Find_SourceModule(Element, pModuleClass);
	if (nullptr == pModule)
	{
		const std::string strSeeded = std::string(pModuleClass) + "_seeded";
		pModule = Find_SourceModule(Element, strSeeded.c_str());
	}
	if (nullptr == pModule && 0u == std::string_view(pModuleClass).find(
		"particlemodule"))
	{
		const std::string strEffect = std::string("ef") + pModuleClass;
		pModule = Find_SourceModule(Element, strEffect.c_str());
		if (nullptr == pModule)
		{
			const std::string strEffectSeeded = strEffect + "_seeded";
			pModule = Find_SourceModule(Element, strEffectSeeded.c_str());
		}
	}
	const EFFECT_DISTRIBUTION_DESC* pDistribution =
		Find_SourceDistribution(pModule, pPropertyPath);
	if (nullptr == pDistribution)
		return fFallback;
	if (Uses_ImplicitAlphaIdentity(*pDistribution, pPropertyPath))
		return fFallback;
	return CEffectDistribution::Evaluate(
		*pDistribution, fTime, fRandomUnit).x;
}

float3_t Client::CEffectPlayback::Evaluate_SourceVector(
	const EFFECT_ELEMENT_DESC& Element,
	const char_t* pModuleClass,
	const char_t* pPropertyPath,
	const f32_t fTime,
	const f32_t fRandomUnit,
	const float3_t& Fallback) const
{
	const EFFECT_SOURCE_MODULE_DESC* pModule =
		Find_SourceModule(Element, pModuleClass);
	if (nullptr == pModule)
	{
		const std::string strSeeded = std::string(pModuleClass) + "_seeded";
		pModule = Find_SourceModule(Element, strSeeded.c_str());
	}
	if (nullptr == pModule && 0u == std::string_view(pModuleClass).find(
		"particlemodule"))
	{
		const std::string strEffect = std::string("ef") + pModuleClass;
		pModule = Find_SourceModule(Element, strEffect.c_str());
		if (nullptr == pModule)
		{
			const std::string strEffectSeeded = strEffect + "_seeded";
			pModule = Find_SourceModule(Element, strEffectSeeded.c_str());
		}
	}
	const EFFECT_DISTRIBUTION_DESC* pDistribution =
		Find_SourceDistribution(pModule, pPropertyPath);
	if (nullptr == pDistribution)
		return Fallback;
	const float4_t Value = CEffectDistribution::Evaluate(
		*pDistribution, fTime, fRandomUnit);
	return { Value.x, Value.y, Value.z };
}

float3_t Client::CEffectPlayback::Apply_ParticleEmissionModifier(
	const float3_t& Velocity) const
{
	const EFFECT_PARTICLE_SYSTEM_DESC& ParticleSystem =
		Get_StagedDocument().ParticleSystem;
	const vector_t Rotated = XMVector3TransformNormal(
		XMLoadFloat3(&Velocity),
		XMMatrixRotationY(XMConvertToRadians(
			ParticleSystem.fDirectionYawDegrees)));
	float3_t Result{};
	XMStoreFloat3(&Result, XMVectorScale(
		Rotated, ParticleSystem.fInitialSpeedMultiplier));
	return Result;
}

Client::EFFECT_COLOR_DESC Client::CEffectPlayback::Evaluate_Color(
	const EFFECT_ELEMENT_DESC& Element,
	const f32_t fNormalizedLife) const
{
	EFFECT_COLOR_DESC Color = Element.Detail.Color;
	const EFFECT_LINEAR_LERP_DESC& Lerp = Element.Detail.LinearLerp;
	if (Lerp.bColorOffset)
		Color.vColorOffset = Lerp4(Color.vColorOffset, Lerp.vEndColorOffset,
			fNormalizedLife);
	if (Lerp.bColorMultiply)
		Color.vColorMultiply = Lerp4(Color.vColorMultiply,
			Lerp.vEndColorMultiply, fNormalizedLife);
	if (Lerp.bEmissiveIntensity)
	{
		Color.fEmissiveIntensity +=
			(Lerp.fEndEmissiveIntensity - Color.fEmissiveIntensity) *
			fNormalizedLife;
	}
	return Color;
}

void Client::CEffectPlayback::Rebuild_Frame(const float4x4_t& RootWorld)
{
	const EFFECT_DOCUMENT_DESC& Document = Get_StagedDocument();
	/* Rebuild runs for every active Effect.  Keep the retained row storage so
	   a stable particle/trail population does not free and reallocate all
	   frame vectors every render frame.  Stage/reset paths still replace the
	   complete frame when the document lifetime changes. */
	m_Frame.GpuOccurrences.clear();
	m_Frame.Elements.clear();
	m_Frame.Particles.clear();
	m_Frame.Trails.clear();
	m_Frame.AfterImages.clear();
	m_Frame.Lights.clear();
	m_Frame.ScreenPosts.clear();
	m_Frame.fSampleTimeSeconds = m_fSampleTimeSeconds;
	m_Frame.RootWorld = RootWorld;
	for (size_t iElement = 0u; iElement < Document.Elements.size(); ++iElement)
	{
		const EFFECT_ELEMENT_DESC& Element = Document.Elements[iElement];
		if (!Element.bVisible)
			continue;
		/* GPU occurrence denominators describe every visible carrier in document
		   order, including fail-closed rows that are intentionally dormant.  Keep
		   those rows inactive while restricting simulation/evaluated draw rows to
		   the admitted target closure. */
		if (!Is_PlaybackElementAdmitted(Element))
		{
			if (Is_GpuVisualOccurrence(Element))
				m_Frame.GpuOccurrences.push_back({ &Element, false, 0u });
			continue;
		}
		const f32_t fLocalTime = m_fSampleTimeSeconds -
			Element.Detail.Timing.fStartDelaySeconds;
		const f32_t fPresentationTime = fLocalTime -
			(Element.SourceRecipe.bEnabled ?
				Element.SourceRecipe.fEmitterDelaySeconds : 0.f);
		const f32_t fStableRandom = static_cast<f32_t>(
			Hash_StableId(Element.strElementId) ^
			Element.Detail.Particle.iRandomSeed) /
			static_cast<f32_t>(UINT32_MAX);
		f32_t fPresentationLifeTimeSeconds =
			Element.Detail.Timing.fLifeTimeSeconds;
		const bool_t bSourceVisualLightOrPost =
			Is_SourceVisualProgramElementAdmitted(Element) &&
			Element.SourceRecipe.bEnabled &&
			((EFFECT_ELEMENT_KIND::LIGHT == Element.eKind &&
				Element.SourceRecipe.strRendererShape == "light") ||
			 (EFFECT_ELEMENT_KIND::SCREEN_POST == Element.eKind &&
				Element.SourceRecipe.strRendererShape == "screenPost"));
		if (bSourceVisualLightOrPost)
		{
			fPresentationLifeTimeSeconds = (std::max)(0.f,
				Evaluate_SourceFloat(Element, "particlemodulelifetime", "lifetime",
					0.f, fStableRandom, fPresentationLifeTimeSeconds));
		}
		const f32_t T = fPresentationLifeTimeSeconds > 0.f ?
			Clamp01(fPresentationTime / fPresentationLifeTimeSeconds) : 1.f;
		ELEMENT_STATE& State = m_States[Element.strElementId];
		const bool_t bPresentationActive = fPresentationTime >= 0.f &&
			fPresentationTime < fPresentationLifeTimeSeconds;
		const bool_t bGpuVisualOccurrence = Is_GpuVisualOccurrence(Element);
		const size_t iGpuOccurrence = m_Frame.GpuOccurrences.size();
		if (bGpuVisualOccurrence)
		{
			m_Frame.GpuOccurrences.push_back({
				&Element, bPresentationActive, 0u });
		}
		const size_t iElementRowBegin = m_Frame.Elements.size();
		const size_t iParticleRowBegin = m_Frame.Particles.size();
		const size_t iTrailRowBegin = m_Frame.Trails.size();
		const size_t iAfterImageRowBegin = m_Frame.AfterImages.size();
		if (!Can_EvaluateElementWorld(Element))
			continue;

		const bool_t bSourceVisualDecalParticle =
			Is_SourceVisualDecalParticle(
				Element, Is_SourceVisualProgramElementAdmitted(Element));
		const bool_t bParticleSimulationElement =
			Is_ParticleSimulationElement(
				Element, Is_SourceVisualProgramElementAdmitted(Element));
		if (bPresentationActive &&
			!bParticleSimulationElement &&
			EFFECT_ELEMENT_KIND::TRAIL != Element.eKind &&
			EFFECT_ELEMENT_KIND::LIGHT != Element.eKind &&
			EFFECT_ELEMENT_KIND::SCREEN_POST != Element.eKind)
		{
			m_Frame.Elements.push_back({
				&Element,
				Evaluate_ElementWorld(Element, m_fSampleTimeSeconds, RootWorld),
				Evaluate_Color(Element, T),
				fLocalTime,
				T });
		}

		if (bPresentationActive &&
			EFFECT_ELEMENT_KIND::LIGHT == Element.eKind &&
			Element.Detail.Light.bEnabled &&
			Element.Detail.Light.eProfile ==
				EFFECT_LIGHT_PROFILE::POINT_RECONSTRUCTED_V1 &&
			Element.Detail.Light.eStatus ==
				EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE)
		{
			const float4x4_t World = Evaluate_ElementWorld(
				Element, m_fSampleTimeSeconds, RootWorld);
			const float3_t ColorOverLife = Evaluate_SourceVector(
				Element, "particlemodulecoloroverlife", "coloroverlife",
				T, fStableRandom, float3_t(1.f, 1.f, 1.f));
			const float3_t ColorScaleOverLife = Evaluate_SourceVector(
				Element, "particlemodulecolorscaleoverlife",
				"colorscaleoverlife", T, fStableRandom,
				float3_t(1.f, 1.f, 1.f));
			const f32_t fAlpha = Evaluate_SourceFloat(
				Element, "particlemodulecoloroverlife", "alphaoverlife",
				T, fStableRandom, 1.f) * Evaluate_SourceFloat(
				Element, "particlemodulecolorscaleoverlife",
				"alphascaleoverlife", T, fStableRandom, 1.f);
			EFFECT_EVALUATED_LIGHT Light;
			Light.pElement = &Element;
			const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* const
				pLightSupplemental = nullptr != m_pSourceVisualProgramProjection ?
					m_pSourceVisualProgramProjection->
						Find_SupplementalElementByTargetElementId(
							Element.strElementId) : nullptr;
			const bool_t bBakedEdgeLight = nullptr != pLightSupplemental &&
				pLightSupplemental->BakedEdgeLightPacket.has_value();
			if (bBakedEdgeLight)
			{
				if (!Evaluate_BakedEdgeLightWorldPosition(
						Element, RootWorld, Light.vWorldPosition))
					continue;
			}
			else
			{
				XMStoreFloat3(&Light.vWorldPosition, XMVector3TransformCoord(
					XMLoadFloat3(&Element.Detail.Particle.vInitialPositionMin),
					XMLoadFloat4x4(&World)));
			}
			Light.fRange = Element.Detail.Light.fRange;
			Light.fIntensity = Element.Detail.Light.fIntensity *
				(std::max)(0.f, fAlpha) *
				(std::max)(0.f, Element.Detail.Color.vColorMultiply.w);
			Light.vColor = {
				Element.Detail.Light.vColor.x *
					Element.Detail.Color.vColorMultiply.x * ColorOverLife.x *
					ColorScaleOverLife.x,
				Element.Detail.Light.vColor.y *
					Element.Detail.Color.vColorMultiply.y * ColorOverLife.y *
					ColorScaleOverLife.y,
				Element.Detail.Light.vColor.z *
					Element.Detail.Color.vColorMultiply.z * ColorOverLife.z *
					ColorScaleOverLife.z,
				Element.Detail.Light.vColor.w *
					Element.Detail.Color.vColorMultiply.w };
			Light.vAmbient = Element.Detail.Light.vAmbient;
			Light.fFalloffExponent = Element.Detail.Light.fFalloffExponent;
			Light.fNormalizedLife = T;
			m_Frame.Lights.push_back(Light);
		}
		else if (bPresentationActive &&
			EFFECT_ELEMENT_KIND::SCREEN_POST == Element.eKind &&
			Element.Detail.ScreenPost.bEnabled &&
			Element.Detail.ScreenPost.eProfile <
				EFFECT_SCREEN_POST_PROFILE::END &&
			Element.Detail.ScreenPost.eStatus ==
				EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE)
		{
			EFFECT_EVALUATED_SCREEN_POST Post;
			Post.pElement = &Element;
			Post.eProfile = Element.Detail.ScreenPost.eProfile;
			Post.iSourceOrder = static_cast<uint32_t>(iElement);
			Post.iRandomSeed = Element.Detail.ScreenPost.iRandomSeed;
			Post.fSampleTimeSeconds = fPresentationTime;
			Post.fIntensity = Element.Detail.ScreenPost.fIntensity;
			Post.fSecondaryIntensity =
				Element.Detail.ScreenPost.fSecondaryIntensity;
			Post.fFrequency = Element.Detail.ScreenPost.fFrequency;
			Post.vTint = Element.Detail.ScreenPost.vTint;
			Post.fNormalizedLife = T;

			if (EFFECT_SCREEN_POST_PROFILE::RGB_NOISE_RECONSTRUCTED_V1 ==
				Post.eProfile)
			{
				Post.fSecondaryIntensity = Evaluate_SourceFloat(
					Element, "particlemoduleparameterdynamic",
					"dynamicparams[0].paramvalue", T, fStableRandom,
					Post.fSecondaryIntensity);
				Post.fIntensity = Evaluate_SourceFloat(
					Element, "particlemoduleparameterdynamic",
					"dynamicparams[1].paramvalue", T, fStableRandom,
					Post.fIntensity);
			}
			else if (EFFECT_SCREEN_POST_PROFILE::ZOOM_BLUR_RECONSTRUCTED_V1 ==
				Post.eProfile)
			{
				Post.fIntensity = Evaluate_SourceFloat(
					Element, "particlemoduleparameterdynamic",
					"dynamicparams[0].paramvalue", T, fStableRandom,
					Post.fIntensity);
			}

			Post.fIntensity *= (std::max)(0.f, Evaluate_SourceFloat(
				Element, "particlemodulecoloroverlife", "alphaoverlife",
				T, fStableRandom, 1.f));
			m_Frame.ScreenPosts.push_back(Post);
		}

		float4x4_t ParticleElementWorld{};
		if (!State.Particles.empty() && Element.Detail.Particle.bLocalSpace)
		{
			ParticleElementWorld = Evaluate_ElementWorld(
				Element, m_fSampleTimeSeconds, RootWorld);
		}
		for (const PARTICLE_STATE& Particle : State.Particles)
		{
			const f32_t ParticleT = Clamp01(
				Particle.fAgeSeconds / Particle.fLifeTimeSeconds);
			const float3_t Size = Element.SourceRecipe.bEnabled ?
				Particle.vSize : float3_t(
					Element.Detail.Particle.vStartSize.x +
						(Element.Detail.Particle.vEndSize.x -
							Element.Detail.Particle.vStartSize.x) * ParticleT,
					Element.Detail.Particle.vStartSize.y +
						(Element.Detail.Particle.vEndSize.y -
							Element.Detail.Particle.vStartSize.y) * ParticleT,
					0.f);
			const float4x4_t& ParticleRoot =
				Element.Detail.Particle.bLocalSpace ?
					ParticleElementWorld : Particle.SpawnRootWorld;
			const bool_t bMeshParticle = Is_MeshParticle(Element);
			const f32_t fDepthScale = bMeshParticle ?
				(Element.SourceRecipe.bEnabled ? Size.z :
					0.5f * (Size.x + Size.y)) : 1.f;
			const float3_t WorldScale = bSourceVisualDecalParticle ?
				float3_t(Size.x, Element.Detail.Decal.fDepth, Size.y) :
				float3_t(Size.x, Size.y, fDepthScale);
			const float3_t SourceOrbitRotation = Add3(
				Particle.vSourceOrbitRotationDegrees,
				Scale3(Particle.vSourceOrbitRotationRateDegreesPerSecond,
					Particle.fAgeSeconds));
			float3_t OrbitOffset{};
			XMStoreFloat3(&OrbitOffset, XMVector3TransformNormal(
				XMLoadFloat3(&Particle.vOrbitOffset),
				UE3_EulerDegreesToClientRotation(SourceOrbitRotation)));
			const float3_t Position = Add3(
				Particle.vPosition, OrbitOffset);
			const matrix_t ParticleRotation = bMeshParticle ?
				UE3_EulerDegreesToClientRotation(
					Particle.vSourceMeshRotationDegrees) *
					XMMatrixRotationRollPitchYaw(
						XMConvertToRadians(Particle.vRotationDegrees.x),
						XMConvertToRadians(Particle.vRotationDegrees.y),
						XMConvertToRadians(Particle.vRotationDegrees.z)) :
				XMMatrixRotationRollPitchYaw(
					XMConvertToRadians(Particle.vRotationDegrees.x),
					XMConvertToRadians(Particle.vRotationDegrees.y),
					XMConvertToRadians(Particle.vRotationDegrees.z));
			const matrix_t TypeDataPreRotation = bMeshParticle ?
				UE3_EulerDegreesToClientRotation(
					Element.Detail.Mesh.vSourceTypeDataRotationDegrees) :
				XMMatrixIdentity();
			const matrix_t World = TypeDataPreRotation *
				XMMatrixScaling(WorldScale.x, WorldScale.y, WorldScale.z) *
				ParticleRotation *
				XMMatrixTranslation(
					Position.x,
					Position.y,
					Position.z) *
				XMLoadFloat4x4(&ParticleRoot);
			EFFECT_EVALUATED_PARTICLE Evaluated;
			Evaluated.pElement = &Element;
			XMStoreFloat4x4(&Evaluated.World, World);
			const float4_t ElementColor =
				Evaluate_Color(Element, ParticleT).vColorMultiply;
			if (Element.SourceRecipe.bEnabled)
			{
				Evaluated.Color = {
					ElementColor.x * Particle.vColor.x,
					ElementColor.y * Particle.vColor.y,
					ElementColor.z * Particle.vColor.z,
					ElementColor.w * Particle.vColor.w
				};
			}
			else
			{
				Evaluated.Color = ElementColor;
				Evaluated.Color.w *= 1.f - ParticleT;
			}
			Evaluated.vDynamicParameter = Particle.vDynamicParameter;
			if (!Element.SourceRecipe.bEnabled)
			{
				const EFFECT_PARTICLE_DESC& ParticleDesc = Element.Detail.Particle;
				const f32_t* pStart = &ParticleDesc.vDynamicParameterStart.x;
				const f32_t* pEnd = &ParticleDesc.vDynamicParameterEnd.x;
				f32_t* pEvaluated = &Evaluated.vDynamicParameter.x;
				for (uint32_t iComponent = 0u; iComponent < 4u; ++iComponent)
				{
					if (0u != (ParticleDesc.iDynamicParameterComponentMask &
						(1u << iComponent)))
					{
						pEvaluated[iComponent] = pStart[iComponent] +
							(pEnd[iComponent] - pStart[iComponent]) * ParticleT;
					}
				}
			}
			/* A billboard is rebuilt from the camera each frame, so this roll is
			   the only rotation it has. The authored rate turns the constant
			   offset into an actual spin over the particle's own age. */
			Evaluated.fSpriteRotationDegrees =
				Particle.vRotationDegrees.z +
				Element.Detail.Sprite.fBillboardRollDegrees +
				Element.Detail.Sprite.fBillboardRollDegreesPerSecond *
					Particle.fAgeSeconds;
			Evaluated.fCameraOffset = Particle.fCameraOffset;
			Resolve_SourceSpritePresentation(Element,
				Evaluated.eSpriteAlignment, Evaluated.vSpritePivot,
				Evaluated.bSourceImageFlipping);
			if (Evaluated.bSourceImageFlipping)
			{
				Evaluated.vSourceImageFlipSign = {
					Size.x < 0.f ? -1.f : 1.f,
					Size.y < 0.f ? -1.f : 1.f
				};
			}
			const float3_t SourceVelocity = Multiply3(
				Particle.vVelocity, Particle.vVelocityScale);
			const float3_t EvaluatedVelocity = Add3(SourceVelocity,
				Transform_Normal(Particle.vTargetAttractorWorldVelocity,
					XMMatrixInverse(nullptr, XMLoadFloat4x4(&ParticleRoot))));
			XMStoreFloat3(&Evaluated.vWorldVelocity,
				XMVector3TransformNormal(
					XMLoadFloat3(&EvaluatedVelocity),
					XMLoadFloat4x4(&ParticleRoot)));
			Evaluated.fSubImageIndex = Particle.fSubImageIndex;
			Evaluated.fDistributionRandom = Particle.fDistributionRandom;
			Evaluated.fNormalizedLife = ParticleT;
			if (bSourceVisualDecalParticle)
			{
				EFFECT_EVALUATED_ELEMENT DecalParticle;
				DecalParticle.pElement = &Element;
				DecalParticle.World = Evaluated.World;
				DecalParticle.Color = Element.Detail.Color;
				DecalParticle.Color.vColorMultiply = Evaluated.Color;
				DecalParticle.fLocalTimeSeconds =
					Particle.fAgeSeconds;
				DecalParticle.fNormalizedLife = ParticleT;
				DecalParticle.bWorldOwnsDecalProjectionVolume = true;
				m_Frame.Elements.push_back(std::move(DecalParticle));
			}
			else
			{
				m_Frame.Particles.push_back(Evaluated);
			}
		}

		EFFECT_EVALUATED_TRAIL BakedTrail;
		if (Build_BakedAnimationTrail(Element, RootWorld, BakedTrail))
		{
			if (BakedTrail.EdgePairs.size() >= 2u)
				m_Frame.Trails.push_back(std::move(BakedTrail));
		}
		else if (!State.TrailPoints.empty())
		{
			EFFECT_EVALUATED_TRAIL Trail;
			Trail.pElement = &Element;
			Trail.Points = State.TrailPoints;
			m_Frame.Trails.push_back(std::move(Trail));
		}

		for (const AFTERIMAGE_STATE& AfterImage : State.AfterImages)
		{
			const f32_t Age = Clamp01(AfterImage.fAgeSeconds /
				Element.Detail.Timing.fAfterImageSeconds);
			m_Frame.AfterImages.push_back({
				&Element,
				AfterImage.World,
				std::pow(1.f - Age,
					Element.Detail.AfterImage.fAlphaExponent) });
		}
		if (bGpuVisualOccurrence)
		{
			const size_t iCandidateRowCount =
				(m_Frame.Elements.size() - iElementRowBegin) +
				(m_Frame.Particles.size() - iParticleRowBegin) +
				(m_Frame.Trails.size() - iTrailRowBegin) +
				(m_Frame.AfterImages.size() - iAfterImageRowBegin);
			EFFECT_EVALUATED_GPU_OCCURRENCE& Occurrence =
				m_Frame.GpuOccurrences[iGpuOccurrence];
			Occurrence.iCandidateRowCount =
				static_cast<uint32_t>(iCandidateRowCount);
			Occurrence.bActive = Occurrence.bActive || 0u < iCandidateRowCount;
		}
	}
}

bool_t Client::CEffectPlayback::Query_ParticleRuntimeProbe(
	const std::string_view strElementId,
	EFFECT_PARTICLE_RUNTIME_PROBE& OutProbe) const
{
	OutProbe = {};
	OutProbe.fSampleTimeSeconds = m_Frame.fSampleTimeSeconds;
	const EFFECT_ELEMENT_DESC* pElement = nullptr;
	for (const EFFECT_ELEMENT_DESC& Element : Get_StagedDocument().Elements)
	{
		if (Element.strElementId == strElementId)
		{
			pElement = &Element;
			break;
		}
	}
	if (nullptr == pElement)
		return false;

	OutProbe.bMeshRenderer = Is_MeshParticle(*pElement);
	for (const EFFECT_EVALUATED_PARTICLE& Particle : m_Frame.Particles)
	{
		if (nullptr == Particle.pElement ||
			Particle.pElement->strElementId != strElementId)
		{
			continue;
		}
		if (0u == OutProbe.iActiveParticleCount)
		{
			OutProbe.fFirstAlpha = Particle.Color.w;
			OutProbe.fMinAlpha = Particle.Color.w;
			OutProbe.fMaxAlpha = Particle.Color.w;
			OutProbe.vFirstDynamicParameter = Particle.vDynamicParameter;
			OutProbe.vMinDynamicParameter = Particle.vDynamicParameter;
			OutProbe.vMaxDynamicParameter = Particle.vDynamicParameter;
			OutProbe.fFirstNormalizedLife = Particle.fNormalizedLife;
			OutProbe.fFirstSubImageIndex = Particle.fSubImageIndex;
			OutProbe.vFirstWorldPosition = Get_Translation(Particle.World);
		}
		else
		{
			OutProbe.fMinAlpha = (std::min)(
				OutProbe.fMinAlpha, Particle.Color.w);
			OutProbe.fMaxAlpha = (std::max)(
				OutProbe.fMaxAlpha, Particle.Color.w);
			OutProbe.vMinDynamicParameter = {
				(std::min)(OutProbe.vMinDynamicParameter.x,
					Particle.vDynamicParameter.x),
				(std::min)(OutProbe.vMinDynamicParameter.y,
					Particle.vDynamicParameter.y),
				(std::min)(OutProbe.vMinDynamicParameter.z,
					Particle.vDynamicParameter.z),
				(std::min)(OutProbe.vMinDynamicParameter.w,
					Particle.vDynamicParameter.w)
			};
			OutProbe.vMaxDynamicParameter = {
				(std::max)(OutProbe.vMaxDynamicParameter.x,
					Particle.vDynamicParameter.x),
				(std::max)(OutProbe.vMaxDynamicParameter.y,
					Particle.vDynamicParameter.y),
				(std::max)(OutProbe.vMaxDynamicParameter.z,
					Particle.vDynamicParameter.z),
				(std::max)(OutProbe.vMaxDynamicParameter.w,
					Particle.vDynamicParameter.w)
			};
		}
		++OutProbe.iActiveParticleCount;
	}

	if (0u == OutProbe.iActiveParticleCount ||
		!pElement->SourceRecipe.bEnabled)
	{
		return true;
	}

	const EFFECT_SOURCE_MODULE_DESC* pRequired =
		Find_SourceModule(*pElement, "particlemodulerequired");
	const std::string_view strSubUVMode = nullptr == pRequired ?
		std::string_view{} : SourceString(*pRequired, "interpolationmethod");
	const uint32_t iColumns = static_cast<uint32_t>((std::max)(1.f,
		nullptr == pRequired ? 1.f :
			SourceNumber(*pRequired, "subimages_horizontal", 1.f)));
	const uint32_t iRows = static_cast<uint32_t>((std::max)(1.f,
		nullptr == pRequired ? 1.f :
			SourceNumber(*pRequired, "subimages_vertical", 1.f)));
	const uint64_t iFrameCount =
		static_cast<uint64_t>(iColumns) * static_cast<uint64_t>(iRows);
	if (iFrameCount > UINT32_MAX)
		return false;
	const EFFECT_EVALUATED_PARTICLE* pFirst = nullptr;
	for (const EFFECT_EVALUATED_PARTICLE& Particle : m_Frame.Particles)
	{
		if (nullptr != Particle.pElement &&
			Particle.pElement->strElementId == strElementId)
		{
			pFirst = &Particle;
			break;
		}
	}
	if (nullptr != pFirst)
	{
		const bool_t bHasSubUV = iFrameCount > 1u &&
			!strSubUVMode.empty() && strSubUVMode != "none" &&
			strSubUVMode != "psuvim_none";
		const EFFECT_SUBUV_FRAME_DESC SourceSubUV =
			Resolve_SourceSubUVFrame(
				bHasSubUV ? iColumns : 1u,
				bHasSubUV ? iRows : 1u,
				bHasSubUV ? pFirst->fSubImageIndex : 0.f,
				false, false, pFirst->fDistributionRandom,
				bHasSubUV &&
					Is_EffectSourceLinearBlendSubUVMode(strSubUVMode));
		EFFECT_SUBUV_FRAME_DESC ResolvedSubUV;
		if (!Resolve_ParticleSpriteSubUV(
			*pFirst, SourceSubUV, ResolvedSubUV))
		{
			return false;
		}
		OutProbe.FirstSubUV = ResolvedSubUV;
	}
	return true;
}

uint32_t Client::CEffectPlayback::Next_Random(ELEMENT_STATE& State) const
{
	uint32_t Value = State.iRandomState;
	Value ^= Value << 13u;
	Value ^= Value >> 17u;
	Value ^= Value << 5u;
	State.iRandomState = 0u == Value ? 1u : Value;
	return State.iRandomState;
}

f32_t Client::CEffectPlayback::Random_Range(
	ELEMENT_STATE& State,
	const f32_t fMin,
	const f32_t fMax) const
{
	const f32_t T = static_cast<f32_t>(Next_Random(State)) /
		static_cast<f32_t>(UINT32_MAX);
	return fMin + (fMax - fMin) * T;
}

float3_t Client::CEffectPlayback::Sample_AuthoredSpawnPosition(
	const EFFECT_PARTICLE_DESC& Desc,
	ELEMENT_STATE& State) const
{
	/* The min/max box is the only spawn volume the authored Detail had before
	   spawnShape existed, so POINT has to keep drawing exactly the same three
	   randoms in the same order.  The other kinds add the volume Cascade
	   expressed through particlemodulelocationprimitive*, on top of that box so
	   an authored offset still applies. */
	const float3_t vBoxOffset(
		Random_Range(State, Desc.vInitialPositionMin.x,
			Desc.vInitialPositionMax.x),
		Random_Range(State, Desc.vInitialPositionMin.y,
			Desc.vInitialPositionMax.y),
		Random_Range(State, Desc.vInitialPositionMin.z,
			Desc.vInitialPositionMax.z));
	const EFFECT_PARTICLE_SPAWN_SHAPE_DESC& Shape = Desc.SpawnShape;
	if (EFFECT_PARTICLE_SPAWN_SHAPE::POINT == Shape.eKind)
		return vBoxOffset;

	const f32_t fArc = XMConvertToRadians(
		std::clamp(Shape.fArcDegrees, 0.f, 360.f));
	const f32_t fAngle = Random_Range(State, -0.5f * fArc, 0.5f * fArc);
	switch (Shape.eKind)
	{
	case EFFECT_PARTICLE_SPAWN_SHAPE::SPHERE:
	{
		/* Uniform by volume between the inner shell and the radius: the cube root
		   keeps particles from clustering at the centre. */
		const f32_t fUnit = Random_Range(State, 0.f, 1.f);
		const f32_t fInnerCube = Shape.fInnerRadius * Shape.fInnerRadius *
			Shape.fInnerRadius;
		const f32_t fOuterCube = Shape.fRadius * Shape.fRadius * Shape.fRadius;
		const f32_t fRadius = std::cbrt(
			fInnerCube + (fOuterCube - fInnerCube) * fUnit);
		const f32_t fCosPolar = Random_Range(State, -1.f, 1.f);
		const f32_t fSinPolar = std::sqrt(
			(std::max)(0.f, 1.f - fCosPolar * fCosPolar));
		return Add3(vBoxOffset, float3_t(
			fRadius * fSinPolar * std::cos(fAngle),
			fRadius * fCosPolar,
			fRadius * fSinPolar * std::sin(fAngle)));
	}
	case EFFECT_PARTICLE_SPAWN_SHAPE::RING:
	{
		/* Uniform by area on the XZ annulus; the square root plays the same role
		   the cube root plays for the sphere. */
		const f32_t fUnit = Random_Range(State, 0.f, 1.f);
		const f32_t fInnerSquare = Shape.fInnerRadius * Shape.fInnerRadius;
		const f32_t fOuterSquare = Shape.fRadius * Shape.fRadius;
		const f32_t fRadius = std::sqrt(
			fInnerSquare + (fOuterSquare - fInnerSquare) * fUnit);
		return Add3(vBoxOffset, float3_t(
			fRadius * std::cos(fAngle), 0.f, fRadius * std::sin(fAngle)));
	}
	case EFFECT_PARTICLE_SPAWN_SHAPE::BOX:
		return Add3(vBoxOffset, float3_t(
			Random_Range(State, -Shape.vExtents.x, Shape.vExtents.x),
			Random_Range(State, -Shape.vExtents.y, Shape.vExtents.y),
			Random_Range(State, -Shape.vExtents.z, Shape.vExtents.z)));
	case EFFECT_PARTICLE_SPAWN_SHAPE::POINT:
	case EFFECT_PARTICLE_SPAWN_SHAPE::END:
	default:
		return vBoxOffset;
	}
}

float3_t Client::CEffectPlayback::Sample_AuthoredInitialVelocity(
	const EFFECT_PARTICLE_DESC& Desc,
	ELEMENT_STATE& State,
	const float3_t& vSpawnPosition) const
{
	const EFFECT_PARTICLE_INITIAL_VELOCITY_DESC& Emission = Desc.InitialVelocity;
	if (EFFECT_PARTICLE_VELOCITY_MODE::FIXED == Emission.eMode)
	{
		return float3_t(
			Random_Range(State, Desc.vInitialVelocityMin.x,
				Desc.vInitialVelocityMax.x),
			Random_Range(State, Desc.vInitialVelocityMin.y,
				Desc.vInitialVelocityMax.y),
			Random_Range(State, Desc.vInitialVelocityMin.z,
				Desc.vInitialVelocityMax.z));
	}

	const f32_t fSpeed = Random_Range(
		State, Emission.vSpeedRange.x, Emission.vSpeedRange.y);
	float3_t vDirection{};
	if (EFFECT_PARTICLE_VELOCITY_MODE::CONE == Emission.eMode)
	{
		/* The cone opens around the Element +Y axis, which is the axis the
		   authored Transform rotation already aims. */
		const f32_t fHalfAngle = XMConvertToRadians(
			std::clamp(Emission.fConeAngleDegrees, 0.f, 180.f));
		const f32_t fPolar = Random_Range(State, 0.f, fHalfAngle);
		const f32_t fAzimuth = Random_Range(State, 0.f, XM_2PI);
		const f32_t fSinPolar = std::sin(fPolar);
		vDirection = float3_t(
			fSinPolar * std::cos(fAzimuth),
			std::cos(fPolar),
			fSinPolar * std::sin(fAzimuth));
	}
	else
	{
		/* OUTWARD and INWARD are radial about the Element origin.  A particle that
		   spawned exactly on the origin has no radial direction to inherit, so it
		   takes a random one rather than silently emitting nothing. */
		vDirection = vSpawnPosition;
		if (DistanceSquared(vDirection, float3_t{}) <= 1.0e-12f)
		{
			const f32_t fCosPolar = Random_Range(State, -1.f, 1.f);
			const f32_t fAzimuth = Random_Range(State, 0.f, XM_2PI);
			const f32_t fSinPolar = std::sqrt(
				(std::max)(0.f, 1.f - fCosPolar * fCosPolar));
			vDirection = float3_t(
				fSinPolar * std::cos(fAzimuth),
				fCosPolar,
				fSinPolar * std::sin(fAzimuth));
		}
		vDirection = Normalize3(vDirection);
		if (EFFECT_PARTICLE_VELOCITY_MODE::INWARD == Emission.eMode)
			vDirection = Scale3(vDirection, -1.f);
	}
	return Scale3(vDirection, fSpeed);
}

bool_t Client::CEffectPlayback::Is_Finished() const
{
	if (m_fSampleTimeSeconds < m_fDurationSeconds)
		return false;
	for (const auto& Pair : m_States)
	{
		if (!Pair.second.Particles.empty() ||
			!Pair.second.TrailPoints.empty() ||
			!Pair.second.AfterImages.empty())
		{
			return false;
		}
	}
	return true;
}
