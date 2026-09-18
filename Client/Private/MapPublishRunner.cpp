#include "MapPublishRunner.h"

#include "ProjectDataRoot.h"

#include <algorithm>
#include <cctype>
#include <vector>

namespace
{
	/* Same character set the script's ValidatePattern on -AreaId accepts, so
	   the command line can never carry anything but an Area identifier. */
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

CMapPublishRunner::~CMapPublishRunner()
{
	if (nullptr != m_hProcess)
		CloseHandle(m_hProcess);
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
		("Area" != scope && "WorldSequences" != scope && "Lights" != scope))
	{
		outStatus = "Publish rejected: invalid Area id or scope.";
		return false;
	}
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
	CloseHandle(process.hThread);
	m_hProcess = process.hProcess;
	m_AreaId = areaId;
	m_Scope = scope;
	m_LogPath = logPath;
	outStatus = "Publishing " + areaId + " (" + scope + "). Log: " +
		logPath.string();
	return true;
}

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
