#include "MapPublishRunner.h"

//게시 스크립트가 있는 프로젝트 루트를 찾는다.
#include "ProjectDataRoot.h"

//Area ID의 문자 검사와 CreateProcessW에 넘길 수정 가능한 명령 버퍼를 준비한다.
#include <algorithm>
#include <cctype>
#include <vector>

namespace
{
	//명령행에 들어갈 Area ID를 영숫자와 _, ., -로 제한한다.
	bool_t IsPublisherAreaId(const std::string& areaId)
	{
		return !areaId.empty() && std::all_of(areaId.begin(), areaId.end(),
			[](const unsigned char character)
			{
				return 0 != std::isalnum(character) || '_' == character ||
					'.' == character || '-' == character;
			});
	}
}

NS_BEGIN(Client)

//추적 HANDLE만 해제한다. 이미 시작한 publisher는 자체 작업을 마친다.
CMapPublishRunner::~CMapPublishRunner()
{
	if (nullptr != m_hProcess)
	{
		CloseHandle(m_hProcess);
	}
}

bool_t CMapPublishRunner::Start(
	const std::string& areaId,
	const std::string& scope,
	const std::wstring& logTag,
	std::string& outStatus)
{
	if (nullptr != m_hProcess)
	{
		outStatus = "A publish is already running for " + m_AreaId + ".";
		return false;
	}
	if (!IsPublisherAreaId(areaId) ||
		("Area" != scope && "WorldSequences" != scope && "Lights" != scope &&
			"Placements" != scope))
	{
		outStatus = "Publish rejected: invalid Area id or scope.";
		return false;
	}

	//현재 Data 루트의 형제 Tools에서 정식 게시 스크립트를 찾는다.
	const std::filesystem::path root = CProjectDataRoot::Get().parent_path();
	const std::filesystem::path script =
		root / L"Tools/MapPipeline/Publish-MapAuthoring.ps1";
	std::error_code error;
	if (!std::filesystem::is_regular_file(script, error) || error)
	{
		outStatus = "Publish failed to start: publisher script is missing: " +
			script.string();
		return false;
	}

	//자식의 출력은 로그로, 입력은 NUL로 연결한다. 시작 실패 시 준비한 HANDLE을 닫는다.
	wchar_t temporary[MAX_PATH]{};
	if (!GetTempPathW(MAX_PATH, temporary))
	{
		outStatus = "Publish failed to start: log folder is unavailable.";
		return false;
	}
	const std::filesystem::path logPath = std::filesystem::path(temporary) /
		(L"LostArk-" + logTag + L"-" +
			std::to_wstring(GetCurrentProcessId()) + L".log");
	SECURITY_ATTRIBUTES security{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
	const HANDLE log = CreateFileW(logPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
		&security, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == log)
	{
		outStatus = "Publish failed to start: cannot create log " + logPath.string();
		return false;
	}
	const HANDLE input = CreateFileW(L"NUL", GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, &security, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == input)
	{
		CloseHandle(log);
		outStatus = "Publish failed to start: cannot prepare standard input.";
		return false;
	}

	//검증한 Area·scope만 인자로 넘기고 별도 콘솔 창 없이 실행한다.
	std::wstring command =
		L"powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File \"" +
		script.wstring() + L"\" -AreaId " +
		std::wstring(areaId.begin(), areaId.end()) + L" -Scope " +
		std::wstring(scope.begin(), scope.end()) + L" -Mode Publish";
	std::vector<wchar_t> arguments(command.begin(), command.end());
	arguments.push_back(0);
	STARTUPINFOW startup{};
	startup.cb = sizeof(startup);
	startup.dwFlags = STARTF_USESTDHANDLES;
	startup.hStdOutput = log;
	startup.hStdError = log;
	startup.hStdInput = input;
	PROCESS_INFORMATION process{};
	const bool_t started = !!CreateProcessW(nullptr, arguments.data(), nullptr,
		nullptr, TRUE, CREATE_NO_WINDOW, nullptr, root.c_str(), &startup, &process);
	const DWORD startError = started ? ERROR_SUCCESS : GetLastError();
	CloseHandle(log);
	CloseHandle(input);
	if (!started)
	{
		outStatus = "Publish failed to start: cannot start powershell (Windows error " +
			std::to_string(startError) + ").";
		return false;
	}

	//시작 성공 뒤에만 실행 상태를 보관한다. stdout/stderr의 자식 참조는 자식이 유지한다.
	CloseHandle(process.hThread);
	m_hProcess = process.hProcess;
	m_AreaId = areaId;
	m_Scope = scope;
	m_LogPath = logPath;
	outStatus = "Publishing " + areaId + " (" + scope + "). Log: " +
		logPath.string();
	return true;
}

//저작 도구의 매 프레임 호출에서 사용한다. UI 스레드를 기다리게 하지 않는다.
bool_t CMapPublishRunner::Poll(bool_t& outSucceeded, uint32_t& outExitCode)
{
	if (nullptr == m_hProcess ||
		WAIT_TIMEOUT == WaitForSingleObject(m_hProcess, 0))
	{
		return false;
	}
	DWORD code = 1u;
	GetExitCodeProcess(m_hProcess, &code);
	CloseHandle(m_hProcess);
	m_hProcess = nullptr;
	outExitCode = static_cast<uint32_t>(code);
	outSucceeded = 0u == code;
	return true;
}

NS_END
