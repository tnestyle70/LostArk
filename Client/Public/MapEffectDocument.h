#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

enum class MAP_EFFECT_PRESENTATION_KIND
{
	DEPLOY_SURFACE_OVERLAY,
	EFFECT_DOCUMENT,
	END
};

enum class MAP_EFFECT_ORIENTATION_POLICY
{
	WORLD,
	CAMERA_FACING_WORLD,
	END
};

enum class MAP_EFFECT_ACTIVATION_POLICY
{
	LEVEL_ACTIVE,
	SERVER_PATTERN_WINDOW,
	END
};

enum class MAP_EFFECT_PLAYBACK_POLICY
{
	LOCAL_LOOP,
	SERVER_CLOCK_SAMPLE,
	END
};

struct MAP_EFFECT_ACTIVATION_WINDOW final
{
	std::string patternId;
	std::string stageId;
	uint32_t effectTimelineOffsetMs = 0u;

	bool operator==(const MAP_EFFECT_ACTIVATION_WINDOW&) const = default;
};

struct MAP_EFFECT_SURFACE_OWNER final
{
	std::string groupId;
	uint64_t placementId = 0u;

	bool operator==(const MAP_EFFECT_SURFACE_OWNER&) const = default;
};

struct MAP_EFFECT_SURFACE_PRESENTATION final
{
	std::string independentEffectId;
	std::string displayName;
	std::vector<MAP_EFFECT_SURFACE_OWNER> owners;
	std::vector<std::string> visibleStates;
	uint32_t materialIndex = 0u;
	f32_t emissiveIntensity = 1.f;
	float4_t emissiveColor = { 1.f, 1.f, 1.f, 1.f };
	f32_t maskPower = 1.f;

	bool operator==(const MAP_EFFECT_SURFACE_PRESENTATION& other) const
	{
		return independentEffectId == other.independentEffectId &&
			displayName == other.displayName && owners == other.owners &&
			visibleStates == other.visibleStates &&
			materialIndex == other.materialIndex &&
			emissiveIntensity == other.emissiveIntensity &&
			emissiveColor.x == other.emissiveColor.x &&
			emissiveColor.y == other.emissiveColor.y &&
			emissiveColor.z == other.emissiveColor.z &&
			emissiveColor.w == other.emissiveColor.w &&
			maskPower == other.maskPower;
	}
};

struct MAP_EFFECT_WORLD_PRESENTATION final
{
	std::string independentEffectId;
	std::string displayName;
	std::string placementId;
	std::string effectAssetId;
	float3_t position = {};
	float4_t rotationQuaternion = { 0.f, 0.f, 0.f, 1.f };
	float3_t scale = { 1.f, 1.f, 1.f };
	MAP_EFFECT_ORIENTATION_POLICY orientationPolicy =
		MAP_EFFECT_ORIENTATION_POLICY::WORLD;
	MAP_EFFECT_ACTIVATION_POLICY activationPolicy =
		MAP_EFFECT_ACTIVATION_POLICY::LEVEL_ACTIVE;
	std::string activationSetId;
	std::vector<MAP_EFFECT_ACTIVATION_WINDOW> activationWindows;
	MAP_EFFECT_PLAYBACK_POLICY playbackPolicy =
		MAP_EFFECT_PLAYBACK_POLICY::LOCAL_LOOP;

	bool operator==(const MAP_EFFECT_WORLD_PRESENTATION& other) const
	{
		return independentEffectId == other.independentEffectId &&
			displayName == other.displayName &&
			placementId == other.placementId && effectAssetId == other.effectAssetId &&
			position.x == other.position.x && position.y == other.position.y &&
			position.z == other.position.z &&
			rotationQuaternion.x == other.rotationQuaternion.x &&
			rotationQuaternion.y == other.rotationQuaternion.y &&
			rotationQuaternion.z == other.rotationQuaternion.z &&
			rotationQuaternion.w == other.rotationQuaternion.w &&
			scale.x == other.scale.x && scale.y == other.scale.y &&
			scale.z == other.scale.z &&
			orientationPolicy == other.orientationPolicy &&
			activationPolicy == other.activationPolicy &&
			activationSetId == other.activationSetId &&
			activationWindows == other.activationWindows &&
			playbackPolicy == other.playbackPolicy;
	}
};

/* Strict Area-owned projection for static surface presentation and world-root
   Effect instances.  A row is exactly one tagged kind; runtime pointers,
   prototype tags and vector indices are deliberately absent from the save
   contract. */
class CMapEffectDocument final
{
public:
	static constexpr size_t MAX_PRESENTATION_COUNT = 64u;
	static constexpr size_t MAX_SURFACE_OWNER_COUNT = 256u;

public:
	bool_t Load(
		const std::filesystem::path& path,
		const std::string& expectedAreaId,
		std::string& outStatus);
	bool_t Load_WithRawBaseline(
		const std::filesystem::path& path,
		const std::string& expectedAreaId,
		std::string& outRawBaseline,
		std::string& outStatus);
	bool_t Parse(
		const std::string& text,
		const std::string& expectedAreaId,
		std::string& outStatus);
	std::string Serialize() const;
	/* Authoring CAS save. expectedRawBytes is the exact file byte sequence
	   captured by Load_WithRawBaseline, not a reserialized approximation. */
	bool_t Save_AtomicIfUnchanged(
		const std::filesystem::path& path,
		std::string_view expectedRawBytes,
		std::string& outStatus) const;
	static bool_t Restore_RawBytesAtomicIfUnchanged(
		const std::filesystem::path& path,
		const std::string& expectedAreaId,
		std::string_view replacementRawBytes,
		std::string_view expectedCurrentRawBytes,
		std::string& outStatus);
	void Clear();

	bool_t Is_Ready() const { return m_isReady; }
	const std::string& Get_AreaId() const { return m_AreaId; }
	const std::vector<MAP_EFFECT_SURFACE_PRESENTATION>& Get_Surfaces() const
	{
		return m_Surfaces;
	}
	const std::vector<MAP_EFFECT_WORLD_PRESENTATION>& Get_WorldEffects() const
	{
		return m_WorldEffects;
	}
	const MAP_EFFECT_SURFACE_PRESENTATION* Find_Surface(
		const std::string& independentEffectId) const;
	MAP_EFFECT_SURFACE_PRESENTATION* Edit_Surface(
		const std::string& independentEffectId);
	const MAP_EFFECT_WORLD_PRESENTATION* Find_WorldEffect(
		const std::string& independentEffectId) const;
	MAP_EFFECT_WORLD_PRESENTATION* Edit_WorldEffect(
		const std::string& independentEffectId);
	bool_t Add_WorldEffectForAuthoring(
		const MAP_EFFECT_WORLD_PRESENTATION& presentation,
		std::string& outStatus);
	bool_t Semantically_Equals(const CMapEffectDocument& other) const;

private:
	std::string m_AreaId;
	std::vector<MAP_EFFECT_SURFACE_PRESENTATION> m_Surfaces;
	std::vector<MAP_EFFECT_WORLD_PRESENTATION> m_WorldEffects;
	bool_t m_isReady = false;
};

NS_END
