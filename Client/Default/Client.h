// 이 헤더는 Client.cpp가 Win32 리소스의 번호를 사용할 수 있게 연결한다.
// CClient 같은 객체를 선언하거나 EXE를 시작하는 파일은 아니다. 실제 진입점은 Client.cpp의 wWinMain이다.
// #pragma once는 한 번역 단위에서 이 헤더가 중복 포함돼도 본문을 한 번만 읽게 하는 컴파일 지시다.
#pragma once

// resource.h의 IDS_APP_TITLE, IDC_CLIENT, IDI_CLIENT, IDM_EXIT 등은 정수 식별자다.
// Client.rc에 등록한 문자열·아이콘·메뉴·대화 상자를 LoadStringW/LoadIcon/DialogBox 등이 그 번호로 찾는다.
// 번호를 include하는 시점에는 리소스나 게임 객체가 생성되지 않는다. 실제 로드는 Win32 API 호출 때 일어난다.
// resource.h의 NO_DEPENDENCIES와 _APS_NEXT_* 주석·매크로는 리소스 편집기의 관리 영역이다.
// Engine의 Component/Prototype 등록 ID와 Win32 리소스 번호는 서로 다른 계약이다.
#include "resource.h"
