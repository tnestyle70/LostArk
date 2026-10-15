#pragma once

//Win32 HANDLE과 Client 공통 타입을 사용한다. Windows보다 Winsock2를 먼저 읽는다.
#include "Client_Defines.h"
#include "Engine_Defines.h"

//게시 로그 경로와 Area·scope 문자열을 보관한다.
#include <filesystem>
#include <string>

NS_BEGIN(Client)

//맵 저작 도구가 publisher를 별도 프로세스로 실행하고 종료 결과를 조회한다.
//실행 데이터 교체·rollback은 publisher가 맡는다. 소멸 시 HANDLE만 닫고 작업을 강제 종료하지 않는다.
class CMapPublishRunner final
{
public:
	CMapPublishRunner() = default;
	~CMapPublishRunner();
	CMapPublishRunner(const CMapPublishRunner&) = delete;
	CMapPublishRunner& operator=(const CMapPublishRunner&) = delete;

	bool_t Is_Running() const { return nullptr != m_hProcess; }
	//반환 참조는 이 객체가 소유한다. 다음 Start에서 값이 바뀔 수 있다.
	const std::string& Get_AreaId() const { return m_AreaId; }
	const std::string& Get_Scope() const { return m_Scope; }
	const std::filesystem::path& Get_LogPath() const { return m_LogPath; }

	//Area·scope를 검증해 한 작업을 시작한다. 호출자는 도구별 logTag로 로그 이름을 구분한다.
	bool_t Start(
		const std::string& areaId,
		const std::string& scope,
		const std::wstring& logTag,
		std::string& outStatus);
	//대기 없이 상태를 확인한다. 종료를 수거한 호출에서만 true와 exit code를 돌려준다.
	bool_t Poll(bool_t& outSucceeded, uint32_t& outExitCode);

private:
	//Start가 얻고 Poll 또는 소멸자가 닫는다. null이면 추적 중인 작업이 없다.
	HANDLE m_hProcess = nullptr;
	std::string m_AreaId;
	std::string m_Scope;
	std::filesystem::path m_LogPath;
};

NS_END
