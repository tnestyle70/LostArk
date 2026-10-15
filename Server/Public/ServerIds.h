#pragma once

// 플랫폼에 관계없이 연결 ID를 64비트 정수로 표현한다.
#include <cstdint>

namespace LostArk::Server
{
	// 왜 이렇게 했냐라는 질문이 나온다면?
	// ServerApp이 accept 성공마다 발급하는 프로세스 내 연결 식별자다.
	// player ID나 socket handle과 분리해 재접속·월드 이동에서도 연결과 캐릭터를 혼동하지 않는다.
	using SESSION_ID = std::uint64_t;
	// inline constexpr 부분도 설명해줘
	// constexpr은 0을 컴파일 시점 상수로 쓰게 하고 inline은 여러 CPP가 이 헤더를 포함해도 같은 정의를 허용한다.
	// 정상 session 발급은 1부터 시작하므로 0은 미연결·미지정 상태를 뜻한다.
	inline constexpr SESSION_ID INVALID_SESSION_ID = 0;
}