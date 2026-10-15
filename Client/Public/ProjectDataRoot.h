#pragma once

// Client 이름 공간과 프로젝트 공통 선언을 사용한다.
#include "Client_Defines.h"
// Engine의 공통 타입과 filesystem 이름을 사용하는 기존 헤더 의존성이다.
#include "Engine_Defines.h"

// 경로의 구성 요소를 보존하여 절대 경로와 루트 밖 이동을 검사한다.
#include <filesystem>

NS_BEGIN(Client)

// 저작 문서와 도구가 프로젝트 Data 정본에 접근할 때 사용하는 경로 경계다.
// Resources나 게시된 DataFiles와 구분하며, 파일의 읽기·저장·게시 자체는 호출자가 담당한다.
class CProjectDataRoot final
{
public:
	// 명시 환경 변수, 실행 파일의 상위 Data 탐색, 기존 상대 배치 규칙 순서로 루트를 고른다.
	static filesystem::path Get();
	// 정규화한 Data 루트 내부의 자손 경로만 반환한다. 거부하면 빈 경로이며 파일을 변경하지 않는다.
	static filesystem::path Resolve(const filesystem::path& relativePath);
};

NS_END
