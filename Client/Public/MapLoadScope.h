#pragma once

#include "Client_Defines.h"

#include <string>

NS_BEGIN(Client)

// 베른의 모든 맵 표시 경로가 공유하는 파생 분류다. 저작 데이터나 게임 판정을 변경하지 않는다.
// 지면·하늘·변형/반투명 표면은 임의 거리 삭제 대신 기존 표시를 유지하는 정책에 등록한다.
enum class MAP_SCENE_VISIBILITY_KIND : uint8_t
{
    STATIC_SURFACE, VEGETATION, LANDSCAPE, DEFORMING, ORDERED_SURFACE
};
//frustum, distance, occlusion, lod, hlod 최적화
struct MAP_FRUSTUM_CULLING_POLICY final
{
	bool_t bypass = false;
	bool_t diagnostics = false;
	f32_t baseMargin = 0.f;
	f32_t largeObjectRadiusThreshold = 0.f;
	f32_t largeObjectAbsoluteMargin = 0.f;
	f32_t largeObjectRelativeMargin = 0.f;
	uint32_t rejectHysteresisFrames = 0u;
};

struct MAP_FRUSTUM_RUNTIME_STATE final
{
	bool_t initialized = false;
	bool_t lastFrustumVisible = true;
	uint32_t rejectGraceFrames = 0u;
};
//culling을 하고 난 이후에 Load되는 Map에 대한 Scope
struct MAP_LOAD_SCOPE final
{
	bool_t isEnabled = false;
	bool_t includeBackground = false;
	f32_t minimumX = {};
	f32_t minimumZ = {};
	f32_t maximumX = {};
	f32_t maximumZ = {};
	std::string excludedAssetGroupId;
	MAP_FRUSTUM_CULLING_POLICY frustumCulling{};

	bool_t Contains(const float3_t& position) const
	{
		return !isEnabled ||
			(position.x >= minimumX && position.x <= maximumX &&
			 position.z >= minimumZ && position.z <= maximumZ);
	}
};

NS_END
