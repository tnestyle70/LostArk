// 저장본의 정식 Publish와 클릭 시점 draft의 임시 실행 준비를 서로 다른 process로 관리한다.
// 실제 build-domain transaction·Server admission은 기존 script와 BossTool이 소유한다.
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonPatternAuditionService.h"
#include "Level_KoukuSaydonArena.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"
#include "NetworkManager.h"

#include <Windows.h>
#include <array>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace
{
using namespace Client;

// 완료 후 UI 진단에 사용할 마지막 4096 byte만 읽는다. 로그가 없으면 빈 진단이며 process 결과와 구분한다.
std::string Read_PublishDiagnosticTail(const std::filesystem::path &path)
{
	constexpr std::streamoff MAX_TAIL_BYTES = 4096;
	std::ifstream input(path, std::ios::binary);
	if (!input)
		return {};
	input.seekg(0, std::ios::end);
	const std::streamoff size = input.tellg();
	if (size > MAX_TAIL_BYTES)
		input.seekg(size - MAX_TAIL_BYTES, std::ios::beg);
	else
		input.seekg(0, std::ios::beg);
	return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}
} // 이 파일의 보조 구현 범위 끝

// Pattern pane의 명시 Publish가 호출한다. 저장 완료·freshness·중복 실행을 확인한 뒤 정본 build-domain 소유자를 숨김 실행한다.
// input/output handle은 CreateProcess 뒤 부모에서 닫고 process 관찰 handle만 세션이 계속 소유한다.
bool_t Client::CKoukuSaydonActionWorkbench::Publish_AllPatterns(std::string &outStatus)
{
	if (m_bSequenceWorkspace)
	{
		outStatus = m_strStatus = "The Sequencer workspace supports local preview and Save only.";
		return false;
	}
	if (!m_bHasDraft || Is_Dirty() || !m_Document.Is_Fresh())
	{
		outStatus = "Save all Composition changes before Publish All Patterns; reload if the saved source changed.";
		m_strStatus = outStatus;
		return false;
	}
	if (nullptr != m_hPublishProcess)
	{
		outStatus = "A KoukuSaydon Product publisher is already running or still observed.";
		m_strStatus = outStatus;
		return false;
	}

	std::error_code error;
	const std::filesystem::path projectRoot =
		std::filesystem::weakly_canonical(CProjectDataRoot::Get().parent_path(), error);
	const std::filesystem::path script = projectRoot / L"Tools" / L"Build" / L"Invoke-BuildDomainOwner.ps1";
	const std::filesystem::path diagnosticDirectory = projectRoot / L"out" / L"KoukuSaydon";
	if (error || projectRoot.empty() || !std::filesystem::is_regular_file(script, error) || error)
	{
		outStatus = "The canonical KoukuSaydon Product publisher could not be resolved.";
		m_strStatus = outStatus;
		return false;
	}
	std::array<wchar_t, MAX_PATH> systemDirectory{};
	const UINT systemLength = GetSystemDirectoryW(systemDirectory.data(), static_cast<UINT>(systemDirectory.size()));
	const std::filesystem::path powershell = systemLength && systemLength < systemDirectory.size()
		? std::filesystem::path(systemDirectory.data()) / L"WindowsPowerShell" / L"v1.0" / L"powershell.exe"
		: std::filesystem::path{};
	if (powershell.empty() || !std::filesystem::is_regular_file(powershell, error) || error)
	{
		outStatus = m_strStatus = "The Windows PowerShell executable for Product publish could not be resolved.";
		return false;
	}
	std::filesystem::create_directories(diagnosticDirectory, error);
	if (error)
	{
		outStatus = "The KoukuSaydon publish diagnostic directory could not be created.";
		m_strStatus = outStatus;
		return false;
	}
	m_PublishDiagnosticPath = diagnosticDirectory /
		(L"KoukuSaydonComposition." + std::to_wstring(GetCurrentProcessId()) + L"." +
			std::to_wstring(GetTickCount64()) + L".publish.log");

	SECURITY_ATTRIBUTES security{};
	security.nLength = sizeof(security);
	security.bInheritHandle = TRUE;
	const HANDLE output = CreateFileW(m_PublishDiagnosticPath.c_str(), GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_DELETE, &security, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == output)
	{
		outStatus = "Could not create the preserved KoukuSaydon publish log.";
		m_strStatus = outStatus;
		return false;
	}
	const HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &security, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == input)
	{
		CloseHandle(output);
		outStatus = "Could not create the KoukuSaydon publisher input handle.";
		m_strStatus = outStatus;
		return false;
	}

	const std::wstring command = L"\"" + powershell.wstring() +
		L"\" -NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass -File \"" + script.wstring() +
		L"\" -Owner KoukuSaydon " + L"-ExpectedKoukuSaydonSourceRevision " + std::to_wstring(m_Draft.iRevision);
	// CreateProcessW는 명령 버퍼를 수정할 수 있어 불변 wstring의 c_str 대신 쓰기 가능한 vector를 넘긴다. 마지막 NUL까지 이 호출 동안 버퍼가 살아 있다.
	std::vector<wchar_t> mutableCommand(command.begin(), command.end());
	mutableCommand.push_back(L'\0');
	STARTUPINFOW startup{};
	startup.cb = sizeof(startup);
	startup.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	startup.wShowWindow = SW_HIDE;
	startup.hStdInput = input;
	startup.hStdOutput = output;
	startup.hStdError = output;
	PROCESS_INFORMATION process{};
	const BOOL created = CreateProcessW(powershell.c_str(), mutableCommand.data(), nullptr, nullptr, TRUE,
		CREATE_NO_WINDOW, nullptr, projectRoot.c_str(), &startup, &process);
	const DWORD createError = created ? ERROR_SUCCESS : GetLastError();
	CloseHandle(input);
	CloseHandle(output);
	if (!created)
	{
		outStatus = "Could not start the KoukuSaydon Product publisher (Win32 " + std::to_string(createError) +
			"): " + powershell.string() + ". Preserved log: " + m_PublishDiagnosticPath.string();
		std::ofstream diagnostic(m_PublishDiagnosticPath, std::ios::binary | std::ios::app);
		if (diagnostic)
			diagnostic << outStatus << '\n';
		m_strStatus = outStatus;
		return false;
	}
	CloseHandle(process.hThread);
	m_hPublishProcess = process.hProcess;
	m_iPublishStartedAtMilliseconds = GetTickCount64();
	outStatus =
		"Publishing saved Patterns and Kouku world placements. Timeline editing remains available; Save and another Publish must wait for completion.";
	m_strStatus = outStatus;
	return true;
}

// MainApp background tick이 0 ms wait로 종료만 확인한다. UI를 막지 않고, 관찰 실패 시 handle을 유지해 중복 게시를 막는다.
void Client::CKoukuSaydonActionWorkbench::Poll_PublishProcess()
{
	if (nullptr == m_hPublishProcess)
		return;
	const HANDLE process = static_cast<HANDLE>(m_hPublishProcess);
	const DWORD wait = WaitForSingleObject(process, 0u);
	// 60초는 강제 종료 시간이 아니다. 진행 중 진단만 바꾸고 transaction의 완료/rollback을 계속 기다린다.
	if (WAIT_TIMEOUT == wait)
	{
		if (GetTickCount64() - m_iPublishStartedAtMilliseconds >= 60000u &&
			m_strStatus.find("still running") == std::string::npos)
		{
			m_strStatus =
				"KoukuSaydon Product publish is still running. Timeline editing remains available; Save waits for the atomic domain transaction to finish.";
		}
		return;
	}
	if (WAIT_OBJECT_0 != wait)
	{
		m_strStatus =
			"KoukuSaydon publisher observation failed; no second publisher will start while this handle remains live.";
		return;
	}

	DWORD exitCode = 1u;
	const bool_t exitKnown = FALSE != GetExitCodeProcess(process, &exitCode);
	CloseHandle(process);
	m_hPublishProcess = nullptr;
	const std::string diagnostic = Read_PublishDiagnosticTail(m_PublishDiagnosticPath);
	// 게시 파일 성공은 실행 중 Server 메모리 교체가 아니다. 제품 목록 refresh edge만 MainApp으로 보낸다.
	if (exitKnown && 0u == exitCode)
	{
		m_bProductInventoryRefreshRequested = true;
		m_strStatus =
			"Published saved Patterns. The next Complete Play admits this revision on the Server; a running replay keeps its original revision. F1 shows unavailable items with their reasons. World placement edits still require a Server restart.";
		if (Is_Dirty())
			m_strStatus += " Edits made during publish are still unsaved. Save and Publish again to include them.";
		return;
	}
	m_strStatus =
		"Publish All Patterns failed. Previous runtime data is retained unless the log reports a rollback failure. Use Publish All Patterns to retry";
	if (exitKnown)
		m_strStatus += " with exit code " + std::to_string(exitCode);
	m_strStatus += ". Preserved log: " + m_PublishDiagnosticPath.string();
	if (!diagnostic.empty())
		m_strStatus += "\n" + diagnostic;
}

// Play 클릭이 고정한 문서를 임시 디렉터리에 직렬화하고 projection script를 실행한다.
// 저작 source와 정식 runtime은 변경하지 않는다. 모든 준비가 끝나기 전까지 Server Play 요청으로 소비할 수 없다.
bool_t Client::CKoukuSaydonActionWorkbench::Start_DraftPlayPreparation(
	KOUKU_DRAFT_PLAY_REQUEST request, const KOUKU_SAYDON_COMPOSITION_DOCUMENT &snapshot, std::string &status)
{
#ifndef _DEBUG
	status = m_strStatus = "Draft Server Play is available only in Debug builds.";
	return false;
#else
	const auto fail = [&](const std::string &reason) {
		status = m_strStatus = reason;
		return false;
	};
	if (m_hDraftPlayProcess || m_bDraftPlayReady)
		return fail("A draft playback snapshot is already being prepared.");
	if (!CNetworkManager::Get().Is_Connected() || !CLevel_KoukuSaydonArena::Get_Active())
		return fail("Enter the connected KoukuSaydon arena before Server preview.");
	std::error_code error;
	const auto root = std::filesystem::weakly_canonical(CProjectDataRoot::Get().parent_path(), error);
	const auto script = root / L"Tools/KoukuSaydonPipeline/Prepare-KoukuDraftPlay.ps1";
	if (error || !std::filesystem::is_regular_file(script, error))
		return fail("Draft playback compiler is unavailable.");
	std::array<wchar_t, MAX_PATH> system{};
	const auto length = GetSystemDirectoryW(system.data(), static_cast<UINT>(system.size()));
	if (!length || length >= system.size())
		return fail("Windows PowerShell could not be resolved.");
	const auto powershell = std::filesystem::path(system.data()) / L"WindowsPowerShell/v1.0/powershell.exe";
	const auto directory = root / L"out/KoukuSaydon" /
		(L"draft-play-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(GetTickCount64()));
	std::filesystem::create_directories(directory, error);
	if (error)
		return fail("The temporary draft playback directory could not be created.");
	// 각 클릭마다 고유 디렉터리를 사용하여 진행 중 UI 편집과 다른 실행의 결과가 섞이지 않게 한다.
	const auto source = directory / L"snapshot.json";
	{
		const auto bytes = CKoukuSaydonCompositionDocument::Serialize(snapshot);
		std::ofstream output(source, std::ios::binary | std::ios::trunc);
		output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
		output.close();
		if (!output)
			return fail("Could not write the immutable draft playback snapshot.");
	}
	SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
	const auto log = directory / L"prepare.log";
	const HANDLE output = CreateFileW(log.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, &security,
		CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (output == INVALID_HANDLE_VALUE)
		return fail("Could not create the draft preparation log.");
	const HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &security, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (input == INVALID_HANDLE_VALUE)
	{
		CloseHandle(output);
		return fail("Could not create draft compiler input.");
	}
	// command 인자에 넣을 stable ID는 저작 식별자 문자만 허용한다. shell 의미가 있는 문자는 준비 전에 거부한다.
	if (request.strTargetId.empty() ||
		request.strTargetId.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.-") !=
			std::string::npos)
	{
		CloseHandle(input);
		CloseHandle(output);
		return fail("Draft playback target is not a valid stable ID.");
	}
	const std::wstring target(request.strTargetId.begin(), request.strTargetId.end());
	const auto command = L"\"" + powershell.wstring() +
		L"\" -NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass -File \"" + script.wstring() +
		L"\" -RepoRoot \"" + root.wstring() + L"\" -SourcePath \"" + source.wstring() + L"\" -OutputDirectory \"" +
		directory.wstring() + L"\" " + (request.bBundle ? L"-BundleId \"" : L"-PatternId \"") + target + L"\"";
	// CreateProcessW는 명령 버퍼를 수정할 수 있어 불변 wstring의 c_str 대신 쓰기 가능한 vector를 넘긴다. 마지막 NUL까지 이 호출 동안 버퍼가 살아 있다.
	std::vector<wchar_t> mutableCommand(command.begin(), command.end());
	mutableCommand.push_back(L'\0');
	STARTUPINFOW startup{};
	startup.cb = sizeof(startup);
	startup.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	startup.wShowWindow = SW_HIDE;
	startup.hStdInput = input;
	startup.hStdOutput = output;
	startup.hStdError = output;
	PROCESS_INFORMATION process{};
	const bool created = CreateProcessW(powershell.c_str(), mutableCommand.data(), nullptr, nullptr, TRUE,
							 CREATE_NO_WINDOW, nullptr, root.c_str(), &startup, &process) != FALSE;
	const auto failure = GetLastError();
	CloseHandle(input);
	CloseHandle(output);
	if (!created)
		return fail("Could not start draft playback preparation (Win32 " + std::to_string(failure) + ").");
	CloseHandle(process.hThread);
	m_hDraftPlayProcess = process.hProcess;
	m_DraftPlayDirectory = directory;
	m_DraftPlayRequest = std::move(request);
	m_bDraftPlayCancelled = false;
	m_bDraftPlayReady = false;
	m_bServerPlayPreparationPending = true;
	status = m_strStatus =
		"Validating the current draft for Server preview. Editing may continue; this click keeps its original values.";
	return true;
#endif
}

// 외부 준비 종료 뒤 취소/월드 세대/출력 크기/target·revision을 확인한다. 세 파일을 모두 읽은 뒤에만 ready로 공개한다.
void Client::CKoukuSaydonActionWorkbench::Poll_DraftPlayProcess()
{
	if (!m_hDraftPlayProcess)
		return;
	DWORD exitCode = STILL_ACTIVE;
	if (!GetExitCodeProcess(static_cast<HANDLE>(m_hDraftPlayProcess), &exitCode))
		exitCode = ERROR_GEN_FAILURE;
	if (exitCode == STILL_ACTIVE)
		return;
	CloseHandle(static_cast<HANDLE>(m_hDraftPlayProcess));
	m_hDraftPlayProcess = nullptr;
	m_bServerPlayPreparationPending = false;
	const auto fail = [&](const std::string &reason) {
		m_bDraftPlayReady = false;
		m_DraftPlayRequest = {};
		m_strStatus = reason + " Log: " + (m_DraftPlayDirectory / L"prepare.log").string();
	};
	if (m_bDraftPlayCancelled)
	{
		fail("Draft playback preparation cancelled.");
		return;
	}
	if (exitCode != 0u)
	{
		fail("Draft playback validation failed: " + Read_PublishDiagnosticTail(m_DraftPlayDirectory / L"prepare.log"));
		return;
	}
	// 접속이 바뀐 뒤 오래된 월드의 projection으로 실행을 요청할 수 없게 generation을 비교한다.
	if (!CNetworkManager::Get().Is_Connected() ||
		CNetworkManager::Get().Get_WorldInboundGeneration() != m_DraftPlayRequest.iWorldGeneration)
	{
		fail("The connected world changed while the draft was being prepared. Start playback again.");
		return;
	}
	try
	{
		// 크기 상한을 확인하고 정확한 길이만 읽는다. 읽는 동안 파일 길이가 달라졌어도 결과를 채택하지 않는다.
		const auto read = [&](const wchar_t *name, const std::uint64_t limit) {
			const auto path = m_DraftPlayDirectory / name;
			std::error_code error;
			const auto size = std::filesystem::file_size(path, error);
			if (error || !size || size > limit)
				throw std::runtime_error("Missing or oversized draft output: " + path.string());
			std::ifstream input(path, std::ios::binary);
			std::string bytes(static_cast<std::size_t>(size), '\0');
			input.read(bytes.data(), static_cast<std::streamsize>(size));
			if (!input || input.peek() != std::char_traits<char>::eof())
				throw std::runtime_error("Draft output changed while reading.");
			return bytes;
		};
		auto rows = read(L"gameplay.rows", 16u * 1024u * 1024u);
		auto presentation = read(L"presentation.json", 64u * 1024u * 1024u);
		auto encounter = read(L"encounter.json", 64u * 1024u * 1024u);
		DATA_JSON_VALUE admission;
		std::string reason;
		if (!CDataJson::Parse(read(L"admission.json", 1024u * 1024u), admission, reason) || !admission.Is_Object())
			throw std::runtime_error("Invalid draft admission: " + reason);
		const auto *target = admission.Find("targetId");
		const auto *revision = admission.Find("sourceRevision");
		if (!target || !target->Is_String() || target->Get_String() != m_DraftPlayRequest.strTargetId || !revision ||
			!revision->Is_Number() || revision->Get_Number() != m_DraftPlayRequest.iSourceRevision)
			throw std::runtime_error("Draft compiler returned a different target or source revision.");
		// admission까지 검증한 뒤 요청이 bytes를 소유한다. 일부 파일만 준비된 상태는 ready로 내보내지 않는다.
		m_DraftPlayRequest.GameplayRows = std::move(rows);
		m_DraftPlayRequest.PresentationJson = std::move(presentation);
		m_DraftPlayRequest.EncounterJson = std::move(encounter);
		m_bDraftPlayReady = true;
		m_bServerPlayPreparationPending = true;
		m_strStatus = "Draft validated. Preparing its Gate and resources before Server admission.";
	}
	catch (const std::exception &error)
	{
		fail(error.what());
	}
}

// MainApp이 ready 결과를 BossTool로 넘기도록 한 번 꺼낸다. 준비 중 추가 편집이 있어도 클릭 snapshot은 유지한다.
// draft generation 차이는 실행 거부가 아니라 자동 Pattern 선택 추적 보류에 사용한다.
bool_t Client::CKoukuSaydonActionWorkbench::Consume_DraftServerPlayRequest(KOUKU_DRAFT_PLAY_REQUEST &request)
{
	if (!m_bDraftPlayReady || m_bSequenceWorkspace)
		return false;
	request = std::move(m_DraftPlayRequest);
	m_DraftPlayRequest = {};
	m_bDraftPlayReady = false;
	m_iServerFollowDraftGeneration = request.iDraftGeneration;
	m_strServerFollowRootPatternId = request.bBundle ? std::string{} : request.strTargetId;
	m_strServerFollowLivePatternId.clear();
	m_iServerFollowPreviousRequest = CKoukuSaydonPatternAuditionService::Get().Get_Snapshot().iRequestSequence;
	m_bServerSelectionFollowSuspended = m_iDraftGeneration != request.iDraftGeneration;
	return true;
}

// 취소는 결과 채택을 막는다. 이미 실행 중인 compiler는 강제 종료하지 않고 poll에서 종료 결과를 버린다.
void Client::CKoukuSaydonActionWorkbench::Cancel_DraftPlayPreparation()
{
	m_bDraftPlayCancelled = true;
	m_bDraftPlayReady = false;
	m_DraftPlayRequest = {};
	m_bServerPlayPreparationPending = m_hDraftPlayProcess != nullptr;
	m_strStatus = "Draft playback cancelled; temporary compiler output will be discarded.";
}
