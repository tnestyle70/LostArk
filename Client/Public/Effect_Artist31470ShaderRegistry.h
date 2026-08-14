#pragma once

#include "Client_Defines.h"
#include "Effect_RuntimeAuthority.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

NS_BEGIN(Client)

inline constexpr size_t EFFECT_ARTIST31470_SHADER_REGISTRY_ROW_COUNT = 35u;
inline constexpr uint32_t EFFECT_ARTIST31470_NO_PASS = ~uint32_t{ 0u };

enum class EFFECT_ARTIST31470_SHADER_FIDELITY : uint8_t
{
	EXACT_CACHE_DXBC_SEMANTIC_REPLAY,
	BOUNDED_EXPLICIT,
	UNRESOLVED_FAIL_CLOSED,
	NON_CORE_FORBIDDEN,
};

enum class EFFECT_ARTIST31470_SHADER_BACKEND : uint8_t
{
	NONE,
	RUNTIME_V2,
	ARTIST_V4,
	FINITE_COMMON,
};

enum class EFFECT_ARTIST31470_SHADER_OUTPUT_ROLE : uint8_t
{
	NONE,
	SCENE_COLOR_RT0,
	DISTORTION_RT1,
};

struct EFFECT_ARTIST31470_SHADER_REGISTRY_ROW final
{
	uint32_t iOrder = 0u;
	std::string_view strOccurrenceId;
	std::string_view strRuntimeElementId;
	EFFECT_RUNTIME_RENDERER_KIND eRenderer =
		EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE;
	std::string_view strRecipeId;
	std::string_view strFamilyId;
	EFFECT_ARTIST31470_SHADER_FIDELITY eFidelity =
		EFFECT_ARTIST31470_SHADER_FIDELITY::UNRESOLVED_FAIL_CLOSED;
	EFFECT_ARTIST31470_SHADER_BACKEND eBackend =
		EFFECT_ARTIST31470_SHADER_BACKEND::NONE;
	uint32_t iOpcode = 0u;
	uint32_t iCurrentPassIndex = EFFECT_ARTIST31470_NO_PASS;
	std::string_view strSourceVertexFactoryCandidate;
	bool_t bNativeSelectionAdmitted = false;
	bool_t bDrawAdmitted = false;
	EFFECT_ARTIST31470_SHADER_OUTPUT_ROLE eSceneColorOutput =
		EFFECT_ARTIST31470_SHADER_OUTPUT_ROLE::NONE;
	EFFECT_ARTIST31470_SHADER_OUTPUT_ROLE eDistortionOutput =
		EFFECT_ARTIST31470_SHADER_OUTPUT_ROLE::NONE;
	std::string_view strEngineEqualityStaticSetSha256;
	std::string_view strRecoveredPixelShaderId;
	std::string_view strRecoveredPixelDxbcSha256;
	uint32_t iExpectedArtistVisualV4TextureMask = 0u;
	std::string_view strRecoveredVertexShaderId;
	std::string_view strRecoveredVertexDxbcSha256;
};

std::span<const EFFECT_ARTIST31470_SHADER_REGISTRY_ROW>
	Get_Artist31470ShaderRegistry() noexcept;

std::optional<EFFECT_ARTIST31470_SHADER_REGISTRY_ROW>
	Find_Artist31470ShaderRegistry(
		uint32_t iOrder,
		std::string_view strOccurrenceId) noexcept;

bool_t Validate_Artist31470ShaderRegistryEmitterIdentity(
	uint32_t iOrder,
	std::string_view strOccurrenceId,
	std::string_view strRuntimeElementId,
	std::string_view strSourceEmitterPath) noexcept;

bool_t Validate_Artist31470ShaderRegistryOccurrence(
	uint32_t iOrder,
	std::string_view strOccurrenceId,
	std::string_view strRuntimeElementId,
	std::string_view strSourceEmitterPath,
	const EFFECT_RUNTIME_PROGRAM_MATERIAL_OCCURRENCE& Occurrence) noexcept;

bool_t Validate_Artist31470ShaderRegistry() noexcept;

NS_END
