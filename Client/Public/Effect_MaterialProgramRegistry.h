#pragma once

#include "DataJson.h"
#include "Effect_AuthoringDocument.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>

NS_BEGIN(Client)

inline constexpr std::string_view EFFECT_MATERIAL_PROGRAM_REGISTRY_SCHEMA =
	"lostark.effect-material-program-registry";
inline constexpr uint32_t EFFECT_MATERIAL_PROGRAM_REGISTRY_FORMAT_VERSION = 1u;
inline constexpr std::string_view
	EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ADAPTER_ID =
		"effect.adapter.sprite-particle.scene-color-rt0.zero-distortion-rt1."
		"alpha-two-sided.v1";
inline constexpr std::string_view
	EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ALPHA_ONE_SIDED_ADAPTER_ID =
		"effect.adapter.sprite-particle.scene-color-rt0.zero-distortion-rt1."
		"alpha-one-sided.v1";
inline constexpr std::string_view
	EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ADDITIVE_TWO_SIDED_ADAPTER_ID =
		"effect.adapter.sprite-particle.scene-color-rt0.zero-distortion-rt1."
		"additive-two-sided.v1";
inline constexpr std::string_view
	EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ADDITIVE_ONE_SIDED_ADAPTER_ID =
		"effect.adapter.sprite-particle.scene-color-rt0.zero-distortion-rt1."
		"additive-one-sided.v1";
inline constexpr std::string_view
	EFFECT_MESH_PARTICLE_SCENE_COLOR_ALPHA_TWO_SIDED_ADAPTER_ID =
		"effect.adapter.mesh-particle.cmodel.scene-color-rt0."
		"zero-distortion-rt1.alpha-two-sided.v1";
inline constexpr std::string_view
	EFFECT_MESH_PARTICLE_SCENE_COLOR_ALPHA_ONE_SIDED_ADAPTER_ID =
		"effect.adapter.mesh-particle.cmodel.scene-color-rt0."
		"zero-distortion-rt1.alpha-one-sided.v1";
inline constexpr std::string_view
	EFFECT_LOCAL_DECAL_SCENE_COLOR_ALPHA_ONE_SIDED_ADAPTER_ID =
		"effect.adapter.local-decal.projector.scene-color-rt0."
		"zero-distortion-rt1.alpha-one-sided.v1";
inline constexpr std::string_view
	EFFECT_LOCAL_DECAL_SCENE_COLOR_ALPHA_TWO_SIDED_ADAPTER_ID =
		"effect.adapter.local-decal.projector.scene-color-rt0."
		"zero-distortion-rt1.alpha-two-sided.v1";
inline constexpr std::string_view
	EFFECT_PROJECT_TUNED_SPRITE_ALPHA_TWO_SIDED_ADAPTER_ID =
		"effect.adapter.sprite-particle.scene-color-rt0.zero-distortion-rt1."
		"project-tuned-alpha-two-sided.v1";
inline constexpr std::string_view
	EFFECT_PROJECT_TUNED_SPRITE_ADDITIVE_TWO_SIDED_ADAPTER_ID =
		"effect.adapter.sprite-particle.scene-color-rt0.zero-distortion-rt1."
		"project-tuned-additive-two-sided.v1";
inline constexpr std::string_view
	EFFECT_PROJECT_TUNED_SPRITE_ALPHA_ONE_SIDED_ADAPTER_ID =
		"effect.adapter.sprite-particle.scene-color-rt0.zero-distortion-rt1."
		"project-tuned-alpha-one-sided.v1";
inline constexpr std::string_view
	EFFECT_PROJECT_TUNED_SPRITE_ADDITIVE_ONE_SIDED_ADAPTER_ID =
		"effect.adapter.sprite-particle.scene-color-rt0.zero-distortion-rt1."
		"project-tuned-additive-one-sided.v1";
inline constexpr std::string_view
	EFFECT_PROJECT_TUNED_MESH_ALPHA_TWO_SIDED_ADAPTER_ID =
		"effect.adapter.mesh-particle.cmodel.scene-color-rt0."
		"zero-distortion-rt1.project-tuned-alpha-two-sided.v1";
inline constexpr std::string_view
	EFFECT_PROJECT_TUNED_MESH_ADDITIVE_TWO_SIDED_ADAPTER_ID =
		"effect.adapter.mesh-particle.cmodel.scene-color-rt0."
		"zero-distortion-rt1.project-tuned-additive-two-sided.v1";
inline constexpr std::string_view
	EFFECT_PROJECT_TUNED_MESH_ALPHA_ONE_SIDED_ADAPTER_ID =
		"effect.adapter.mesh-particle.cmodel.scene-color-rt0."
		"zero-distortion-rt1.project-tuned-alpha-one-sided.v1";
inline constexpr std::string_view
	EFFECT_PROJECT_TUNED_MESH_ADDITIVE_ONE_SIDED_ADAPTER_ID =
		"effect.adapter.mesh-particle.cmodel.scene-color-rt0."
		"zero-distortion-rt1.project-tuned-additive-one-sided.v1";

enum class EFFECT_COMPILED_MATERIAL_ADAPTER_ID : uint8_t
{
	SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1,
	MESH_PARTICLE_CMODEL_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1,
	LOCAL_DECAL_PROJECTOR_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_ONE_SIDED_V1,
	SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_ONE_SIDED_V1,
	SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ADDITIVE_TWO_SIDED_V1,
	SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ADDITIVE_ONE_SIDED_V1,
	MESH_PARTICLE_CMODEL_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_ONE_SIDED_V1,
	LOCAL_DECAL_PROJECTOR_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1,
	PROJECT_TUNED_SPRITE_PARTICLE_ALPHA_TWO_SIDED_V1,
	PROJECT_TUNED_SPRITE_PARTICLE_ADDITIVE_TWO_SIDED_V1,
	PROJECT_TUNED_SPRITE_PARTICLE_ALPHA_ONE_SIDED_V1,
	PROJECT_TUNED_SPRITE_PARTICLE_ADDITIVE_ONE_SIDED_V1,
	PROJECT_TUNED_MESH_PARTICLE_ALPHA_TWO_SIDED_V1,
	PROJECT_TUNED_MESH_PARTICLE_ADDITIVE_TWO_SIDED_V1,
	PROJECT_TUNED_MESH_PARTICLE_ALPHA_ONE_SIDED_V1,
	PROJECT_TUNED_MESH_PARTICLE_ADDITIVE_ONE_SIDED_V1,
	END,
};

enum class EFFECT_COMPILED_MATERIAL_CARRIER : uint8_t
{
	SPRITE_PARTICLE,
	MESH_PARTICLE_CMODEL,
	LOCAL_DECAL_PROJECTOR,
	END,
};

enum class EFFECT_MATERIAL_INLINE_MIRROR_POLICY : uint8_t
{
	INLINE_MIRROR_REQUIRED,
	END,
};

/* This descriptor is compiled code's allowlist receipt.  Registry JSON can
   select eAdapterId, but cannot author or replace any field below. */
struct EFFECT_COMPILED_MATERIAL_ADAPTER_DESC final
{
	EFFECT_COMPILED_MATERIAL_ADAPTER_ID eAdapterId =
		EFFECT_COMPILED_MATERIAL_ADAPTER_ID::END;
	EFFECT_COMPILED_MATERIAL_CARRIER eCarrier =
		EFFECT_COMPILED_MATERIAL_CARRIER::END;
	std::string_view strAdapterId;
	std::string_view strShaderId;
	std::string_view strVertexLayoutId;
	EFFECT_RENDER_PROFILE eRenderProfile = EFFECT_RENDER_PROFILE::END;
	uint32_t iPassIndex = UINT32_MAX;
	std::string_view strMrtId;
	uint32_t iSceneColorRenderTargetIndex = UINT32_MAX;
	std::string_view strSceneColorSemantic;
	uint32_t iDistortionRenderTargetIndex = UINT32_MAX;
	std::string_view strDistortionSemantic;
	bool_t bDistortionDeterministicZero = false;
	std::string_view strRasterizerState;
	std::string_view strDepthStencilState;
	std::string_view strBlendState;
	uint32_t iStencilReference = 0u;
};

struct EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING final
{
	uint64_t iCatalogRevision = 0u;
	uint64_t iRegistryGenerationId = 0u;
	std::string strEffectAssetId;
	std::string strElementId;
	std::string strProgramId;
	std::string strLayoutId;
	std::string strDescriptorId;
	std::string strAdapterId;
	EFFECT_MATERIAL_INLINE_MIRROR_POLICY eInlineMirrorPolicy =
		EFFECT_MATERIAL_INLINE_MIRROR_POLICY::END;
	EFFECT_MATERIAL_EXECUTION_DESC Execution;
	EFFECT_COMPILED_MATERIAL_ADAPTER_DESC Adapter;
};

struct EFFECT_MATERIAL_PROGRAM_BINDING_TARGET final
{
	std::string strEffectAssetId;
	std::string strElementId;
};

/* Immutable generation owned by CEffectCatalog.  Create/Create_Empty perform
   all validation before publishing an instance.  Resolve never consults the
   current catalog and returns only pre-materialized immutable bindings. */
class CEffectMaterialProgramRegistry final
{
public:
	static std::shared_ptr<const CEffectMaterialProgramRegistry> Create_Empty(
		uint64_t iCatalogRevision,
		uint64_t iGenerationId,
		std::string& strOutError);
	static std::shared_ptr<const CEffectMaterialProgramRegistry> Create(
		const DATA_JSON_VALUE& Root,
		uint64_t iCatalogRevision,
		uint64_t iGenerationId,
		std::string& strOutError);

	std::shared_ptr<const EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING> Resolve(
		std::string_view strEffectAssetId,
		std::string_view strElementId) const noexcept;

	uint64_t Get_CatalogRevision() const noexcept
	{
		return m_iCatalogRevision;
	}
	uint64_t Get_GenerationId() const noexcept
	{
		return m_iGenerationId;
	}
	size_t Get_BindingCount() const noexcept;
	std::span<const EFFECT_MATERIAL_PROGRAM_BINDING_TARGET>
		Get_BindingTargets() const noexcept;

	static bool_t Is_ExecutionBitExact(
		const EFFECT_MATERIAL_EXECUTION_DESC& Left,
		const EFFECT_MATERIAL_EXECUTION_DESC& Right) noexcept;

private:
	struct IMPLEMENTATION;

	CEffectMaterialProgramRegistry(
		uint64_t iCatalogRevision,
		uint64_t iGenerationId,
		std::shared_ptr<const IMPLEMENTATION> pImplementation) noexcept;

private:
	uint64_t m_iCatalogRevision = 0u;
	uint64_t m_iGenerationId = 0u;
	std::shared_ptr<const IMPLEMENTATION> m_pImplementation;
};

NS_END
