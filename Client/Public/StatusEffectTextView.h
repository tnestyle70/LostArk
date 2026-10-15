#pragma once

// Client 이름 공간과 공통 기본 타입을 사용한다.
#include "Client_Defines.h"
// 표시 시간·위치에 쓸 f32_t와 float2_t를 사용한다.
#include "Engine_Defines.h"

// entity·문구별 마지막 발생 key를 조회한다.
#include <map>
// 캐릭터 anchor를 비소유 weak_ptr로 전달·보관한다.
#include <memory>
// 각 문구의 화면 오프셋을 난수로 한 번 정한다.
#include <random>
// 등록한 문구를 값으로 보관한다.
#include <string>
// entity ID와 문구를 묶은 key를 만든다.
#include <utility>
// 현재 재생 중인 문구 발생 목록을 순서대로 보관한다.
#include <vector>

NS_BEGIN(Client)

class CCharacter;

// Level이 전달한 상태 발생마다 캐릭터 위에 문구·색을 잠시 표시한다.
// 상태 적용을 판정하지 않으며 표시 위치의 캐릭터를 weak_ptr로 참조해 수명을 늘리지 않는다.
class CStatusEffectTextView final
{
public:
	struct REQUEST
	{
		// 소유 entity ID·문구별로 마지막 발생 key를 기억해 반복 snapshot의 재표시를 막는다.
		std::uint32_t				iOwnerEntityId = 0u;
		std::uint32_t				iOccurrenceKey = 0u;
		std::wstring				strWord;
		// 문구의 RGB 값이다. 입력 형식은 0xRRGGBB다.
		std::uint32_t				iColorRgb = 0xFFFFFFu;
		std::weak_ptr<CCharacter>	pAnchor;
	};

public:
	// 같은 entity·문구의 마지막 key와 같으면 생략한다. 새 발생은 독립 오프셋을 갖는 표시 항목으로 보관한다.
	void Submit(const REQUEST& Request);
	// 유한한 0 이상의 delta초만 누적하고 수명이 끝났거나 anchor가 사라진 항목을 지운다.
	void Update(f32_t fTimeDelta);
	// Level의 Render가 호출한다. owner ID가 0이면 전체, 아니면 해당 owner만 그리며 투영 불가·소멸 anchor는 생략한다.
	void Render(std::uint32_t onlyOwnerEntityId = 0u) const;

private:
	struct OCCURRENCE
	{
		std::uint32_t				iOwnerEntityId = 0u;
		std::wstring				strWord;
		std::uint32_t				iColorRgb = 0xFFFFFFu;
		std::weak_ptr<CCharacter>	pAnchor;
		f32_t						fAgeSeconds = 0.f;
		// 등록 시 한 번 뽑은 1080p 기준 픽셀 오프셋이다. 동시에 생긴 글자의 겹침을 줄인다.
		float2_t					vScatterPx = {};
	};

private:
	// Level 소유 view의 값 멤버다. 소멸 시 목록·중복 기록이 정리되며 캐릭터 수명에는 영향이 없다.
	vector<OCCURRENCE>						m_Occurrences;
	// entity·문구별 마지막 key다. 발생 항목이 만료되어도 이 기록은 view 수명 동안 남는다.
	std::map<std::pair<std::uint32_t, std::wstring>, std::uint32_t> m_SpawnedKeyByOwner;
	std::mt19937							m_Scatter{ std::random_device{}() };
};

NS_END
