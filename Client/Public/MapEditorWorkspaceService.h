#pragma once

// Client 이름 공간을 사용한다.
#include "Client_Defines.h"
// 외부 조회 API의 bool_t 타입을 사용한다.
#include "Engine_Defines.h"

NS_BEGIN(Client)

// Lobby·Loader·Development·도구가 공유하는 프로세스 단위 맵 편집 요청/활성 플래그다. 맵 객체는 소유하지 않는다.
class CMapEditorWorkspaceService final
{
public:
	// 편집 진입 요청만 켠다. 실제 로드·활성화는 Loader와 Development가 처리한다.
	static void Request();
	// 활성·요청 플래그를 각각 false로 저장한다. 두 플래그의 일괄 transaction은 아니다.
	static void Cancel();
	// Loader/Development가 편집 진입 요청을 조회한다.
	static bool_t Is_Requested();
	// 활성화 시 요청도 켠다. false를 넣어도 요청은 그대로 남는다.
	static void Set_Active(bool_t isActive);
	// 현재 도구가 편집 workspace 분기를 사용할지 조회한다.
	static bool_t Is_Active();
};

NS_END
