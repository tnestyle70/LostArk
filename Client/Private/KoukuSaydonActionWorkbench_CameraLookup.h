#pragma once

// Area 소유 shot을 조회하는 구현만 Level의 중첩 shot 타입에 의존한다.
// 일반 문서/Logic 편집에는 Level과 그 렌더링 의존성을 전파하지 않는다.
#include "Level_KoukuSaydonArena.h"
// 반환형이 CLevel_KoukuSaydonArena::KOUKU_CAMERA_SHOT이라는 중첩 타입이라 바깥 클래스 전방 선언만으로는 선언할 수 없다.
#include <string_view>
// shot ID는 조회 호출 동안만 빌려 읽고 이 경계에서 문자열이나 Level의 소유권을 옮기지 않는다.

// Camera/Timeline/Patterns와 Helpers.cpp만 공유하는 조회 연결이다. 실제 정의는 Helpers.cpp 한 곳에 있다.
// 현재 조회 소비 범위에는 Presentation.cpp도 포함된다. 위 파일 목록은 전체 소비자를 한정하지 않는다.
namespace Client::KoukuWorkbenchDetail
{
// 반환값은 Level 소유 목록을 차용한다. 카메라 목록 수정/재로드 뒤에는 다시 조회한다.
// 활성 Level의 Ensure_CameraShotAuthoring을 먼저 호출한다. Level 부재·준비 실패·ID 부재는 모두 nullptr이며 호출자는 기본 길이나 읽기 제한으로 처리한다.
const CLevel_KoukuSaydonArena::KOUKU_CAMERA_SHOT *Find_AuthoringCamera(std::string_view shotId);
} // Client::KoukuWorkbenchDetail 이름공간 끝
