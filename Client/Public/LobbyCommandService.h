#pragma once

// Client 이름 공간 매크로와 bool_t/char_t 공통 타입을 사용한다.
#include "Engine_Defines.h"
// 요청과 취소를 연결하는 64-bit token을 정의한다.
#include <cstdint>
// 마지막 접수·취소·소비 상태를 문자열 값으로 반환한다.
#include <string>

NS_BEGIN(Client)

// Lobby가 처리할 진입 목적지다. END는 요청하지 않는 초기/끝 표식이다.
enum class LOBBY_STAGE
{
	TEST,
	CHARACTER_SELECT,
	VALTAN,
	BERN,
	KOUKU_SAYDON,
	MAHARAKA,
	COLOSSEUM,
	END
};

// 같은 TEST 진입에서도 gameplay와 Map Editor workspace 준비를 구분한다.
enum class LOBBY_COMMAND_PURPOSE
{
	GAMEPLAY,
	MAP_EDITOR_WORKSPACE,
	END
};

// 소비 전까지 보관하는 값이다. token은 이 대기 요청을 취소할 때 대조하며 객체를 소유하지 않는다.
struct LOBBY_COMMAND final
{
	LOBBY_STAGE eStage = LOBBY_STAGE::END;
	LOBBY_COMMAND_PURPOSE ePurpose = LOBBY_COMMAND_PURPOSE::GAMEPLAY;
	std::uint64_t iToken = 0;
};

using LOBBY_COMMAND_TOKEN = std::uint64_t;

inline constexpr LOBBY_COMMAND_TOKEN INVALID_LOBBY_COMMAND_TOKEN = 0;

// 제품 UI/MainApp의 진입 요청 한 개를 보관하고 Level_Lobby가 가져가게 한다.
// Level이나 socket을 소유하지 않으며 접수 성공은 Server 승인/실제 전환 성공과 별개다.
class CLobbyCommandService final
{
public:
	// 목적 생략은 GAMEPLAY다. token 출력이 있는 형태는 실패 시 0, 접수 시 새 token을 돌려준다.
	static bool_t Request(LOBBY_STAGE eStage);

	static bool_t Request(
		LOBBY_STAGE eStage,
		LOBBY_COMMAND_PURPOSE purpose);

	static bool_t Request(
		LOBBY_STAGE eStage,
		LOBBY_COMMAND_TOKEN& outToken);

	static bool_t Request(
		LOBBY_STAGE eStage,
		LOBBY_COMMAND_PURPOSE purpose,
		LOBBY_COMMAND_TOKEN& outToken);

	// 대기 token이 같고 이유가 비어 있지 않을 때만 제거한다. 실패하면 기존 대기 명령은 유지한다.
	static bool_t Cancel(
		LOBBY_COMMAND_TOKEN token,
		const char_t* pReason);

	// Level_Lobby가 대기 명령을 값으로 가져간 뒤 한 칸을 비운다. 대기가 없으면 출력값은 그대로다.
	static bool_t Try_Consume(LOBBY_COMMAND& outCommand);

	// mutex 안에서 상태 문자열을 복사하므로 반환값은 이후 상태 변경과 독립이다.
	static std::string Get_Status();
};

NS_END
