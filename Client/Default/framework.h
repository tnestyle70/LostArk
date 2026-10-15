// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.

// Win32 응용 프로그램에서 사용하는 기본 헤더 묶음이다. Client.cpp가 직접 include한다.
// 이 파일 이름의 framework는 Update/Render를 실행하는 객체를 뜻하지 않는다.
// include는 선언을 컴파일러에게 보이며, 창·장치·MainApp의 생성은 CPP의 함수 호출이 담당한다.
#pragma once

// Windows SDK의 버전 매크로를 준비한 뒤 windows.h가 해당 정책에 맞는 선언을 읽도록 순서를 둔다.
// 실제 지원 OS를 바꾸려면 빌드 옵션과 /FI를 포함한 최초 Windows 헤더 진입 순서도 함께 확인한다.
#include "targetver.h"
// windows.h가 덜 쓰이는 API 묶음을 생략하게 하는 전처리 선택이다. 실행 중 옵션이 아니다.
// 필요한 개별 Windows 헤더까지 없애는 의미는 아니며, 이 프로젝트의 WinSock2 포함 순서와도 관계가 있다.
//Windows.h가 기본으로 함께 포함하는 일부 부가 API 헤더를 생략한다. 필요한 부가 API는 해당를 직접 포함한다.
#define WIN32_LEAN_AND_MEAN 
// Windows 헤더 파일
// Client.cpp의 HINSTANCE/HWND/MSG/WNDCLASSEXW와 CreateWindowW, PeekMessage, DispatchMessage 등을 선언한다.
// HWND는 OS가 관리하는 창을 가리키는 handle이며 new로 만든 C++ 객체의 소유 포인터와 다르다.
#include <windows.h>
// C 런타임 헤더 파일입니다.
// 아래 네 헤더는 기존 Windows 프로젝트 기본 묶음이다. stdlib.h는 C 런타임의 변환·종료 등,
// malloc.h는 메모리 할당 확장, memory.h는 바이트 메모리 처리, tchar.h는 문자 집합 선택용 선언을 제공한다.
// 현재 Client.cpp는 std::strtol/std::abort를 위해 cstdlib도 직접 포함한다.
// malloc.h와 memory.h의 함수를 이 진입점에서 직접 호출하는 것은 아니다. 포함돼 있다는 사실과 실제 사용을 구분한다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
