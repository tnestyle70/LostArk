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

// Create(JSON)가 허용하는 문서 형식 식별자다. 이 선언만으로 제품이 해당 JSON 파일을 자동 로드하지는 않는다.
inline constexpr std::string_view EFFECT_MATERIAL_PROGRAM_REGISTRY_SCHEMA =
	"lostark.effect-material-program-registry";
inline constexpr uint32_t EFFECT_MATERIAL_PROGRAM_REGISTRY_FORMAT_VERSION = 1u;
// stable adapter 문자열은 C++ allowlist를 선택한다. carrier, alpha/additive, one/two-sided와 RT 출력 계약을 구분한다.
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

// 이미 컴파일된 adapter의 내부 선택값이다. 배열 위치나 이 숫자를 JSON의 영구 ID로 대신 쓰지 않는다.
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

// 이 registry가 허용하는 geometry/VF 경계다. material equation이나 스킬 종류와는 독립이다.
enum class EFFECT_COMPILED_MATERIAL_CARRIER : uint8_t
{
	SPRITE_PARTICLE,
	MESH_PARTICLE_CMODEL,
	LOCAL_DECAL_PROJECTOR,
	END,
};

// registry binding이 있는 element는 저작 inline Execution과 동일해야 한다. 서로 다른 두 packet을 조용히 선택하지 않는다.
enum class EFFECT_MATERIAL_INLINE_MIRROR_POLICY : uint8_t
{
	INLINE_MIRROR_REQUIRED,
	END,
};

// 컴파일된 코드가 허용한 adapter 계약이다. registry JSON은 등록된 adapterId만 고를 수 있다.
// shader/VF/pass/MRT/render state 필드를 JSON에서 새로 저작하거나 바꾸는 경로는 아니다.
struct EFFECT_COMPILED_MATERIAL_ADAPTER_DESC final
{
	EFFECT_COMPILED_MATERIAL_ADAPTER_ID eAdapterId =
		EFFECT_COMPILED_MATERIAL_ADAPTER_ID::END;
	EFFECT_COMPILED_MATERIAL_CARRIER eCarrier =
		EFFECT_COMPILED_MATERIAL_CARRIER::END;
	// 이 descriptor의 string_view는 Compiled_Adapters가 가진 정적 literal을 차용한다. 복사해도 별도 문자열 할당은 하지 않는다.
	std::string_view strAdapterId;
	std::string_view strShaderId;
	std::string_view strVertexLayoutId;
	EFFECT_RENDER_PROFILE eRenderProfile = EFFECT_RENDER_PROFILE::END;
	// 실제 shader technique의 pass index다. state 문자열과 함께 Renderer의 compiled adapter 검증에 사용한다.
	uint32_t iPassIndex = UINT32_MAX;
	// 현재 allowlist는 MRT_SceneHDR의 RT0 색과 RT1 왜곡을 지정한다. 이 typed adapter의 RT1은 확정적으로 0인 계약이다.
	// 이 필드를 모든 native Effect가 왜곡을 출력하지 않는다는 뜻으로 일반화하지 않는다.
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

// effect/element stable ID 하나를 program/layout/descriptor/adapter와 결합한 검증 결과다. 문자열과 Execution은 값으로 소유한다.
struct EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING final
{
	// 준비한 문서와 registry가 같은 세대인지 소비자가 확인한다. 이 숫자만 비교해 외부 파일 freshness를 증명하지는 않는다.
	uint64_t iCatalogRevision = 0u;
	uint64_t iRegistryGenerationId = 0u;
	std::string strEffectAssetId;
	std::string strElementId;
	// program은 계산식 선택, layout은 입력 배치 계약, descriptor는 그 배치에 넣을 값, adapter는 실제 draw 연결을 식별한다.
	std::string strProgramId;
	std::string strLayoutId;
	std::string strDescriptorId;
	std::string strAdapterId;
	EFFECT_MATERIAL_INLINE_MIRROR_POLICY eInlineMirrorPolicy =
		EFFECT_MATERIAL_INLINE_MIRROR_POLICY::END;
	// backend/opcode와 texture/scalar/vector/mask/state의 실행 값이다. Renderer가 GPU 자원과 packed uniform으로 준비한다.
	EFFECT_MATERIAL_EXECUTION_DESC Execution;
	EFFECT_COMPILED_MATERIAL_ADAPTER_DESC Adapter;
};

// 검증된 binding 대상의 effect+element ID 목록이다. vector index 대신 이 stable 문자열 쌍으로 Resolve한다.
struct EFFECT_MATERIAL_PROGRAM_BINDING_TARGET final
{
	std::string strEffectAssetId;
	std::string strElementId;
};

// Catalog가 shared_ptr로 보관하는 불변 generation이다. factory는 검증한 새 값을 반환하며 기존 catalog를 직접 교체하지 않는다.
// 현재 CEffectCatalog의 direct-authored 로드는 Create_Empty를 사용한다. 저장된 element의 inline Execution이 제품 입력이다.
// Create(JSON)는 검증 API로 남아 있으며, binding이 있는 경우 Renderer가 inline mirror와 일치하는지 별도로 대조한다.
class CEffectMaterialProgramRegistry final
{
public:
	// 현재 Catalog 로드가 호출하는 빈 generation 생성이다. revision/generation의 0은 nullptr와 오류로 거절한다.
	static std::shared_ptr<const CEffectMaterialProgramRegistry> Create_Empty(
		uint64_t iCatalogRevision,
		uint64_t iGenerationId,
		std::string& strOutError);
	// program/layout/descriptor/adapter/binding 참조를 검증하고 미리 실행 값으로 만든다. 명시적 실패는 nullptr다.
	// 오류 문자열은 갱신하며 allocation 예외를 전부 nullptr로 바꾸는 API는 아니다.
	static std::shared_ptr<const CEffectMaterialProgramRegistry> Create(
		const DATA_JSON_VALUE& Root,
		uint64_t iCatalogRevision,
		uint64_t iGenerationId,
		std::string& strOutError);

	// 두 stable ID를 map에서 조회하고 불변 binding의 shared_ptr를 돌려준다. 없으면 nullptr이며 현재 Catalog를 재조회하지 않는다.
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
	// 이 generation에 준비된 대상 수와 읽기 전용 목록이다. span의 수명은 registry의 implementation 수명에 묶인다.
	size_t Get_BindingCount() const noexcept;
	std::span<const EFFECT_MATERIAL_PROGRAM_BINDING_TARGET>
		Get_BindingTargets() const noexcept;

	// PreparedDocument/Catalog 경계가 inline mirror를 대조한다. 지정 필드와 float bit를 비교하며 struct padding을 memcmp하는 함수는 아니다.
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
	// ID별 map과 대상 vector를 불변으로 공유한다. 반환한 binding shared_ptr는 registry보다 오래 보관할 수 있지만 span은 owner를 유지해야 한다.
	std::shared_ptr<const IMPLEMENTATION> m_pImplementation;
};

NS_END
