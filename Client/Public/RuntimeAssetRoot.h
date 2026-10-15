#pragma once

// Client 이름 공간과 프로젝트 공통 선언을 사용한다.
#include "Client_Defines.h"
// Engine의 공통 타입과 filesystem 이름을 사용하는 기존 헤더 의존성이다.
#include "Engine_Defines.h"

// 경로의 구성 요소를 보존하여 절대 경로와 루트 밖 이동을 검사한다.
#include <filesystem>

NS_BEGIN(Client)

// 모델·텍스처·폰트 소비자가 Resources 기준 상대 asset ID를 실제 파일 경로로 바꿀 때 사용한다.
// 파일을 열거나 소유하지 않으며, Resolve 계열은 허용 루트를 벗어나면 빈 경로를 반환한다.
class CRuntimeAssetRoot final
{
public:
	// 환경 변수와 실행 파일 위치 순서로 Resources 루트를 고른다. 반환 경로의 존재를 항상 보장하지는 않는다.
	static filesystem::path Get_ResourceRoot();
	// 기존 asset 소비자가 사용하는 Get_ResourceRoot의 별칭이다.
	static filesystem::path Get();
	// Resources의 Fonts 하위 경로를 반환한다. 디렉터리를 생성하지 않는다.
	static filesystem::path Get_FontRoot();
	// Resources 상대 경로를 정규화하고 루트 내부의 자손 경로만 허용한다.
	static filesystem::path Resolve(const filesystem::path& relativePath);
	// 폰트 상대 경로는 Fonts 내부로 한정한다. 빈 입력·절대 경로·상위 루트 이탈은 실패다.
	static filesystem::path Resolve_Font(
		const filesystem::path& relativePath);
};

NS_END
