#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <string>
#include <vector>

NS_BEGIN(Client)

// Effect 제작 데이터의 저장 형식 버전이다.
inline constexpr uint32_t EFFECT_AUTHORING_FORMAT_VERSION = 1u;

// 하나의 Effect Document를 구성할 수 있는 시각 요소 종류다.
// END는 유효한 요소가 아니라 초기화 누락을 검출하기 위한 sentinel이다.
enum class EFFECT_ELEMENT_KIND : uint8_t
{
	MESH,
	SPRITE,
	PARTICLE,
	DECAL,
	TRAIL,
	END
};

// Effect Document 안에 들어가는 시각 요소 하나의 데이터다.
// G1에서는 정체성만 정의하고 Transform과 렌더링 값은 이후 G에서 추가한다.
struct EFFECT_ELEMENT_DESC final
{
	// Document 내부에서 Element를 구분하는 안정적인 고유 ID다.
	std::string strElementId;
	// Mesh, Sprite, Particle, Decal, Trail 중 어떤 종류인지 나타낸다.
	EFFECT_ELEMENT_KIND eKind = EFFECT_ELEMENT_KIND::END;
	// 절대 경로가 아닌 Resources 기준 상대 Asset ID다.
	std::string strResourceId;
};

// 메모리에서 제작 중인 재사용 가능한 Effect Asset 하나의 문서다.
// ImGui, GPU, Character, Animation 객체를 소유하지 않는 순수 데이터다.
struct EFFECT_DOCUMENT_DESC final
{
	// 이후 파일을 읽을 때 호환성을 검증할 schema version이다.
	uint32_t iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	// 저장과 런타임 연결에 사용하는 Effect Asset의 안정적인 ID다.
	std::string strEffectAssetId;
	// 제작자에게 표시하는 사람이 읽을 수 있는 이름이다.
	std::string strDisplayName;
	// 이 Effect Asset을 구성하는 시각 요소들을 순서대로 소유한다.
	std::vector<EFFECT_ELEMENT_DESC> Elements;
};

NS_END
