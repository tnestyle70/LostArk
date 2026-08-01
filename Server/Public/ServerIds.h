#pragma once

#include <cstdint>

namespace LostArk::Server
{
	//왜 이렇게 했냐라는 질문이 나온다면?
	using SESSION_ID = std::uint64_t;
	//inline constexpr 부분도 설명해줘
	inline constexpr SESSION_ID INVALID_SESSION_ID = 0;
}