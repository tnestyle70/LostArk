#ifndef Engine_Define_h__
#define Engine_Define_h__

// 장치/context/자원과 Windows 기반 공통 타입을 선언한다. 실제 자원 소유와 생성은 각 엔진 컴포넌트가 맡는다.
#include <d3d11_1.h>

// Windows의 min/max 함수 매크로는 외부 헤더의 부수 효과에 기대지 않고 여기서 해제한다.
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
// 벡터·행렬 계산과 교차 검사에서 공유하는 DirectX 수학 타입을 가져온다.
#include <DirectXMath.h>
#include <DirectXCollision.h>



using namespace DirectX;

// 기존 엔진 헤더가 공유하는 컨테이너·정렬·함수 객체·문자열·소유 포인터의 표준 선언이다.
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <functional>
#include <string>
#include <unordered_map>
#include <ctime>
#include <memory>
using namespace std;

// D3D COM 객체의 참조 수명을 관리하는 ComPtr를 사용한다.
#include <wrl/client.h>
using namespace Microsoft::WRL;

// 공통 enum, DLL/namespace 매크로, 수학 타입 별칭과 기존 소유권 보조 함수를 연결한다.
#include "Engine_Enum.h"
#include "Engine_Macro.h"
#include "Engine_Typedef.h"
#include "Engine_Function.h"

namespace Engine
{
	//const unsigned int g_iMaxWidth = 2560;
	//const unsigned int g_iMaxHeight = 1440;
	// const unsigned int g_iMaxWidth = 1280;
	// const unsigned int g_iMaxHeight = 720;

	/*
	const unsigned int g_iMaxWidth = 16384;
	const unsigned int g_iMaxHeight = 9216;
	*/
	// 엔진의 공통 최대 화면 크기 상수다. 실제 창 크기나 사용자가 저장한 렌더링 옵션을 여기서 변경하지 않는다.
	const unsigned int g_iMaxWidth = 8192;
	const unsigned int g_iMaxHeight = 4608;

	// 기존 컴포넌트 조회가 공유하는 Transform 태그다. 저장 문서의 stable asset/placement ID와는 별개다.
	static const wstring_t g_strTransformComTag = TEXT("Com_Transform");
}




// 기존 DLL 공개 클래스의 STL 멤버 관련 경고 정책을 유지한다. ABI 호환성 자체를 보장하는 설정은 아니다.
#pragma warning(disable : 4251)

#ifdef _DEBUG

//Debug CRT의 누수 보고와 할당 번호 추적을 사용한다. C++ new 키워드는 재정의하지 않는다.
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#endif

using namespace std;
using namespace Engine;

#endif // Engine_Define_h__
