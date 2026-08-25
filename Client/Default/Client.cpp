// Client.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "Client.h"

#include "Client_Defines.h"
#include "MainApp.h"
#include "NetworkManager.h"
#include "GameInstance.h"
#include "Profiler.h"
#include "ProfilerCaptureIO.h"

#include "ImGuiLayer.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <string_view>
#include <vector>

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE g_hInst; // 현재 인스턴스입니다.
HWND    g_hWnd;
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

namespace
{
    constexpr uint64_t MAX_AUTOMATIC_CAPTURE_FRAMES =
        Engine::CProfiler::MAX_HISTORY_FRAMES;
    constexpr array<string_view, 4> EXPECTED_PROFILER_CHARACTER_CLASSES = {
        "LanceMaster", "Artist", "DimensionMaster", "Warlord"
    };
    constexpr array<LostArk::Shared::CHARACTER_CLASS_ID, 4>
        EXPECTED_PROFILER_CHARACTER_CLASS_IDS = {
            LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,
            LostArk::Shared::CHARACTER_CLASS_ID::ARTIST,
            LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER,
            LostArk::Shared::CHARACTER_CLASS_ID::WARLORD
        };

    struct AUTOMATIC_PROFILER_CAPTURE_CONFIG final
    {
        bool_t isEnabled = false;
        uint64_t iWarmupFrames = 0u;
        uint64_t iCaptureFrames = 0u;
        uint32_t iClientSlot = 0u;
        LostArk::Shared::CHARACTER_CLASS_ID eExpectedCharacterClass =
            LostArk::Shared::CHARACTER_CLASS_ID::END;
        string strCharacterClass;
        string strPhase;
		string strRunId;
        string strBuildRevision;
        string strDataRevision;
        filesystem::path OutputPath;
    };

    void WriteProfilerCaptureDiagnostic(const string& message)
    {
        const string debuggerMessage =
            "[Client profiler capture] " + message + "\n";
        OutputDebugStringA(debuggerMessage.c_str());

        std::ofstream output(
            "ClientProfilerCapture.user.log",
            std::ios::binary | std::ios::app);
        if (!output)
            return;

        SYSTEMTIME time{};
        ::GetLocalTime(&time);
        output << std::setfill('0')
            << time.wYear << '-'
            << std::setw(2) << time.wMonth << '-'
            << std::setw(2) << time.wDay << ' '
            << std::setw(2) << time.wHour << ':'
            << std::setw(2) << time.wMinute << ':'
            << std::setw(2) << time.wSecond
            << ' ' << message << '\n';
    }

    bool_t TryReadEnvironmentVariable(
        const wchar_t* pName,
        bool_t& outIsPresent,
        wstring& outValue,
        string& outError)
    {
        constexpr DWORD MAX_ENVIRONMENT_VALUE_LENGTH = 32768u;
        std::vector<wchar_t> buffer(MAX_ENVIRONMENT_VALUE_LENGTH);
        SetLastError(ERROR_SUCCESS);
        const DWORD length = GetEnvironmentVariableW(
            pName, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (0u == length)
        {
            const DWORD error = GetLastError();
            if (ERROR_ENVVAR_NOT_FOUND == error)
            {
                outIsPresent = false;
                outValue.clear();
                return true;
            }
            if (ERROR_SUCCESS == error)
            {
                outIsPresent = true;
                outValue.clear();
                return true;
            }

            outError = "GetEnvironmentVariableW failed with error " +
                std::to_string(error) + ".";
            return false;
        }
        if (length >= buffer.size())
        {
            outError = "Environment variable exceeds the Windows length limit.";
            return false;
        }

        outIsPresent = true;
        outValue.assign(buffer.data(), length);
        return true;
    }

    bool_t TryParseUnsigned(
        const wstring_view text,
        const uint64_t maximum,
        uint64_t& outValue)
    {
        if (text.empty())
            return false;

        uint64_t value = 0u;
        for (const wchar_t character : text)
        {
            if (character < L'0' || character > L'9')
                return false;
            const uint64_t digit =
                static_cast<uint64_t>(character - L'0');
            if (value > (maximum - digit) / 10u)
                return false;
            value = value * 10u + digit;
        }
        outValue = value;
        return true;
    }

    bool_t TryReadRequiredUnsignedEnvironmentVariable(
        const wchar_t* pName,
        const uint64_t minimum,
        const uint64_t maximum,
        uint64_t& outValue,
        string& outError)
    {
        bool_t isPresent = false;
        wstring value;
        if (!TryReadEnvironmentVariable(
            pName, isPresent, value, outError))
        {
            return false;
        }
        if (!isPresent || !TryParseUnsigned(value, maximum, outValue) ||
            outValue < minimum)
        {
            outError = "Required profiler environment variable is missing or invalid.";
            return false;
        }
        return true;
    }

    bool_t TryReadRequiredAsciiTokenEnvironmentVariable(
        const wchar_t* pName,
        const char* pDiagnosticName,
        const size_t maximumLength,
        string& outValue,
        string& outError)
    {
        bool_t isPresent = false;
        wstring value;
        if (!TryReadEnvironmentVariable(
            pName, isPresent, value, outError))
        {
            return false;
        }
        if (!isPresent || value.empty() || value.size() > maximumLength)
        {
            outError = string(pDiagnosticName) +
                " must be a non-empty ASCII token of at most " +
                std::to_string(maximumLength) + " characters.";
            return false;
        }

        string token;
        token.reserve(value.size());
        for (const wchar_t character : value)
        {
            const bool_t valid =
                (character >= L'a' && character <= L'z') ||
                (character >= L'A' && character <= L'Z') ||
                (character >= L'0' && character <= L'9') ||
                L'.' == character || L'_' == character || L'-' == character;
            if (!valid)
            {
                outError = string(pDiagnosticName) +
                    " may contain only ASCII letters, digits, '.', '_', and '-'.";
                return false;
            }
            token.push_back(static_cast<char>(character));
        }
        outValue = std::move(token);
        return true;
    }

    string PathToUtf8(const filesystem::path& path)
    {
        const wstring& nativePath = path.native();
        if (nativePath.empty())
            return {};
        if (nativePath.size() > static_cast<size_t>((std::numeric_limits<int>::max)()))
            return "<path-too-long>";

        const int sourceLength = static_cast<int>(nativePath.size());
        const int required = WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            nativePath.data(),
            sourceLength,
            nullptr,
            0,
            nullptr,
            nullptr);
        if (required <= 0)
            return "<path-encoding-failed>";

        string utf8(static_cast<size_t>(required), '\0');
        if (required != WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            nativePath.data(),
            sourceLength,
            utf8.data(),
            required,
            nullptr,
            nullptr))
        {
            return "<path-encoding-failed>";
        }
        return utf8;
    }

    bool_t TryLoadAutomaticProfilerCaptureConfig(
        AUTOMATIC_PROFILER_CAPTURE_CONFIG& outConfig,
        string& outError)
    {
        outConfig = {};

        bool_t hasCaptureFrames = false;
        wstring captureFramesText;
        if (!TryReadEnvironmentVariable(
            L"LOSTARK_PROFILER_CAPTURE_FRAMES",
            hasCaptureFrames,
            captureFramesText,
            outError))
        {
            return false;
        }
        if (!hasCaptureFrames)
            return true;

        uint64_t captureFrames = 0u;
        if (!TryParseUnsigned(
            captureFramesText,
            MAX_AUTOMATIC_CAPTURE_FRAMES,
            captureFrames))
        {
            outError =
                "LOSTARK_PROFILER_CAPTURE_FRAMES must be an unsigned decimal "
                "value no greater than the profiler history capacity.";
            return false;
        }
        if (0u == captureFrames)
            return true;

        uint64_t warmupFrames = 0u;
        if (!TryReadRequiredUnsignedEnvironmentVariable(
            L"LOSTARK_PROFILER_WARMUP_FRAMES",
            0u,
            (std::numeric_limits<uint64_t>::max)(),
            warmupFrames,
            outError))
        {
            outError =
                "LOSTARK_PROFILER_WARMUP_FRAMES must be an unsigned decimal value.";
            return false;
        }

        uint64_t clientSlot = 0u;
        if (!TryReadRequiredUnsignedEnvironmentVariable(
            L"LOSTARK_PROFILER_CLIENT_SLOT",
            1u,
            EXPECTED_PROFILER_CHARACTER_CLASSES.size(),
            clientSlot,
            outError))
        {
            outError =
                "LOSTARK_PROFILER_CLIENT_SLOT must be an integer from 1 through 4.";
            return false;
        }

        string characterClass;
        if (!TryReadRequiredAsciiTokenEnvironmentVariable(
            L"LOSTARK_PROFILER_CHARACTER_CLASS",
            "LOSTARK_PROFILER_CHARACTER_CLASS",
            64u,
            characterClass,
            outError))
        {
            return false;
        }
        const size_t rosterIndex = static_cast<size_t>(clientSlot - 1u);
        if (characterClass !=
            EXPECTED_PROFILER_CHARACTER_CLASSES[rosterIndex])
        {
            outError =
                "LOSTARK_PROFILER_CHARACTER_CLASS must match the canonical "
                "slot roster: 1=LanceMaster, 2=Artist, "
                "3=DimensionMaster, 4=Warlord.";
            return false;
        }

        string phase;
        if (!TryReadRequiredAsciiTokenEnvironmentVariable(
            L"LOSTARK_PROFILER_PHASE",
            "LOSTARK_PROFILER_PHASE",
            64u,
            phase,
            outError))
        {
            return false;
        }

		string runId;
		if (!TryReadRequiredAsciiTokenEnvironmentVariable(
			L"LOSTARK_PROFILER_RUN_ID",
			"LOSTARK_PROFILER_RUN_ID",
			128u,
			runId,
			outError))
		{
			return false;
		}

        string buildRevision;
        if (!TryReadRequiredAsciiTokenEnvironmentVariable(
            L"LOSTARK_PROFILER_BUILD_REVISION",
            "LOSTARK_PROFILER_BUILD_REVISION",
            128u,
            buildRevision,
            outError))
        {
            return false;
        }

        string dataRevision;
        if (!TryReadRequiredAsciiTokenEnvironmentVariable(
            L"LOSTARK_PROFILER_DATA_REVISION",
            "LOSTARK_PROFILER_DATA_REVISION",
            128u,
            dataRevision,
            outError))
        {
            return false;
        }

        bool_t hasOutput = false;
        wstring output;
        if (!TryReadEnvironmentVariable(
            L"LOSTARK_PROFILER_OUTPUT",
            hasOutput,
            output,
            outError))
        {
            return false;
        }
        if (hasOutput && output.empty())
        {
            outError =
                "LOSTARK_PROFILER_OUTPUT must be omitted or contain a non-empty path.";
            return false;
        }

        outConfig.isEnabled = true;
        outConfig.iWarmupFrames = warmupFrames;
        outConfig.iCaptureFrames = captureFrames;
        outConfig.iClientSlot = static_cast<uint32_t>(clientSlot);
        outConfig.eExpectedCharacterClass =
            EXPECTED_PROFILER_CHARACTER_CLASS_IDS[rosterIndex];
        outConfig.strCharacterClass = std::move(characterClass);
        outConfig.strPhase = std::move(phase);
		outConfig.strRunId = std::move(runId);
        outConfig.strBuildRevision = std::move(buildRevision);
        outConfig.strDataRevision = std::move(dataRevision);
        if (hasOutput)
            outConfig.OutputPath = filesystem::path(output);
        return true;
    }

    uint64_t DurationMicroseconds(
        const chrono::steady_clock::duration duration)
    {
        const auto microseconds = chrono::duration_cast<chrono::microseconds>(
            duration).count();
        return microseconds > 0 ? static_cast<uint64_t>(microseconds) : 0u;
    }

	uint64_t QueryUtc100ns()
	{
		FILETIME time{};
		GetSystemTimePreciseAsFileTime(&time);
		ULARGE_INTEGER value{};
		value.LowPart = time.dwLowDateTime;
		value.HighPart = time.dwHighDateTime;
		return value.QuadPart;
	}

    class CAutomaticProfilerCapture final
    {
    private:
        enum class STATE
        {
            OFF,
            ARMED,
            WARMUP,
            CAPTURE,
            GPU_DRAIN,
            COMPLETE
        };

    public:
        bool_t Initialize(
            const AUTOMATIC_PROFILER_CAPTURE_CONFIG& config,
            Engine::CProfiler* pProfiler,
            string& outError)
        {
            m_Config = config;
            if (!m_Config.isEnabled)
                return true;
            if (nullptr == pProfiler)
            {
                outError = "Profiler is unavailable.";
                return false;
            }

            pProfiler->Set_Enabled(false);
            pProfiler->Reset_History();
            m_eState = STATE::ARMED;
            WriteProfilerCaptureDiagnostic(
                "armed; waiting for Valtan Arena and the configured local class");
            return true;
        }

        void Prepare_Frame(Engine::CProfiler* pProfiler) const
        {
            if (nullptr != pProfiler && Is_Measuring() &&
                !pProfiler->Is_Enabled())
            {
                pProfiler->Set_Enabled(true);
            }
        }

        void On_Frame_Ended(Engine::CProfiler* pProfiler)
        {
            if (nullptr == pProfiler || !Is_Active())
                return;

            if (STATE::ARMED == m_eState)
            {
                if (Matches_ExpectedWorkload())
                    Begin_Workload(pProfiler);
                return;
            }

            if ((STATE::WARMUP == m_eState ||
                STATE::CAPTURE == m_eState) &&
                !Matches_ExpectedWorkload())
            {
                Rearm(pProfiler,
                    "workload identity changed before capture completed");
                return;
            }

            if (STATE::WARMUP == m_eState)
            {
                ++m_iCompletedWarmupFrames;
                if (m_iCompletedWarmupFrames >= m_Config.iWarmupFrames)
                {
                    pProfiler->Reset_History();
					m_iCaptureStartUtc100ns = QueryUtc100ns();
                    m_iCaptureLevelId = static_cast<uint32_t>(
                        CGameInstance::Get().Get_CurrentLevelID());
                    m_eState = STATE::CAPTURE;
                }
                return;
            }

            if (STATE::CAPTURE == m_eState)
            {
                ++m_iCompletedCaptureFrames;
                if (m_iCompletedCaptureFrames >= m_Config.iCaptureFrames)
                {
                    m_MeasurementSnapshot = pProfiler->Snapshot();
					m_iCaptureEndUtc100ns = QueryUtc100ns();
                    if (m_MeasurementSnapshot.Frames.size() !=
                        m_Config.iCaptureFrames)
                    {
                        Fail_And_Disable(
                            pProfiler,
                            "history did not contain the requested measurement frame count");
                        return;
                    }
                    m_iLastMeasurementFrame =
                        m_MeasurementSnapshot.Frames.back().FrameNumber;
                    m_eState = STATE::GPU_DRAIN;
                }
                return;
            }

            if (STATE::GPU_DRAIN != m_eState)
                return;

            ++m_iCompletedGpuDrainFrames;
            if (m_iCompletedGpuDrainFrames <
                Engine::CProfiler::GPU_QUERY_RING_SIZE)
            {
                return;
            }

            Merge_ResolvedGpuFrames(pProfiler->Snapshot());

            Client::PROFILER_CAPTURE_METADATA metadata{};
            metadata.iProcessId = GetCurrentProcessId();
            metadata.iClientSlot = m_Config.iClientSlot;
            metadata.iLevelId = m_iCaptureLevelId;
            metadata.iWidth = g_iWinSizeX;
            metadata.iHeight = g_iWinSizeY;
            metadata.iWarmupFrames = m_Config.iWarmupFrames;
            metadata.iRequestedFrames = m_Config.iCaptureFrames;
			metadata.iCaptureStartUtc100ns = m_iCaptureStartUtc100ns;
			metadata.iCaptureEndUtc100ns = m_iCaptureEndUtc100ns;
#ifdef _DEBUG
            metadata.strConfiguration = "Debug";
#else
            metadata.strConfiguration = "Release";
#endif
            metadata.strCharacterClass = m_Config.strCharacterClass;
            metadata.strPhase = m_Config.strPhase;
			metadata.strRunId = m_Config.strRunId;
            metadata.strBuildRevision = m_Config.strBuildRevision;
            metadata.strDataRevision = m_Config.strDataRevision;

            filesystem::path outputPath = m_Config.OutputPath;
            if (outputPath.empty())
            {
                const filesystem::path basePath =
                    Client::CProfilerCaptureIO::Make_DefaultPath(
                        m_iLastMeasurementFrame);
                outputPath = basePath.parent_path() /
                    (basePath.stem().wstring() + L"_slot" +
                        std::to_wstring(m_Config.iClientSlot) + L"_pid" +
                        std::to_wstring(metadata.iProcessId) +
                        basePath.extension().wstring());
            }
            string saveError;
            if (!Client::CProfilerCaptureIO::Save_Json(
                m_MeasurementSnapshot,
                metadata,
                outputPath,
                &saveError))
            {
                Fail_And_Disable(
                    pProfiler,
                    "save failed: " + saveError);
                return;
            }

            WriteProfilerCaptureDiagnostic(
                "saved output=\"" + PathToUtf8(outputPath) +
                "\" frames=" +
                std::to_string(m_MeasurementSnapshot.Frames.size()));
            m_MeasurementSnapshot = {};
            pProfiler->Set_Enabled(false);
            m_eState = STATE::COMPLETE;
        }

        void On_ApplicationExit(Engine::CProfiler* pProfiler)
        {
            if (nullptr == pProfiler || !Is_Active())
                return;
            WriteProfilerCaptureDiagnostic(
                "capture ended before completion; warmupFrames=" +
                std::to_string(m_iCompletedWarmupFrames) +
                " measurementFrames=" +
                std::to_string(m_iCompletedCaptureFrames));
            m_MeasurementSnapshot = {};
            pProfiler->Set_Enabled(false);
            m_eState = STATE::COMPLETE;
        }

    private:
        bool_t Is_Active() const
        {
            return STATE::ARMED == m_eState ||
                STATE::WARMUP == m_eState ||
                STATE::CAPTURE == m_eState ||
                STATE::GPU_DRAIN == m_eState;
        }

        bool_t Is_Measuring() const
        {
            return STATE::WARMUP == m_eState ||
                STATE::CAPTURE == m_eState ||
                STATE::GPU_DRAIN == m_eState;
        }

        bool_t Matches_ExpectedWorkload() const
        {
            return ETOUI(LEVEL::VALTAN_ARENA) ==
                    CGameInstance::Get().Get_CurrentLevelID() &&
                m_Config.eExpectedCharacterClass ==
                    CNetworkManager::Get().Get_LocalCharacterClass();
        }

        void Begin_Workload(Engine::CProfiler* pProfiler)
        {
            m_iCompletedWarmupFrames = 0u;
            m_iCompletedCaptureFrames = 0u;
            m_iCompletedGpuDrainFrames = 0u;
            m_MeasurementSnapshot = {};
			m_iCaptureStartUtc100ns = 0u;
			m_iCaptureEndUtc100ns = 0u;
            m_iCaptureLevelId = static_cast<uint32_t>(
                CGameInstance::Get().Get_CurrentLevelID());
            pProfiler->Reset_History();
			if (0u == m_Config.iWarmupFrames)
				m_iCaptureStartUtc100ns = QueryUtc100ns();
            pProfiler->Set_Enabled(true);
            m_eState = 0u == m_Config.iWarmupFrames ?
                STATE::CAPTURE : STATE::WARMUP;
            WriteProfilerCaptureDiagnostic(
                "workload matched; warmupFrames=" +
                std::to_string(m_Config.iWarmupFrames));
        }

        void Rearm(Engine::CProfiler* pProfiler, const string& reason)
        {
            WriteProfilerCaptureDiagnostic(reason + "; capture re-armed");
            m_iCompletedWarmupFrames = 0u;
            m_iCompletedCaptureFrames = 0u;
            m_iCompletedGpuDrainFrames = 0u;
            m_MeasurementSnapshot = {};
			m_iCaptureStartUtc100ns = 0u;
			m_iCaptureEndUtc100ns = 0u;
            pProfiler->Set_Enabled(false);
            pProfiler->Reset_History();
            m_eState = STATE::ARMED;
        }

        void Merge_ResolvedGpuFrames(
            const Engine::FProfilerCaptureSnapshot& resolved)
        {
            size_t resolvedIndex = 0u;
            for (Engine::FProfilerFrame& frame :
                m_MeasurementSnapshot.Frames)
            {
                while (resolvedIndex < resolved.Frames.size() &&
                    resolved.Frames[resolvedIndex].FrameNumber <
                        frame.FrameNumber)
                {
                    ++resolvedIndex;
                }
                if (resolvedIndex >= resolved.Frames.size() ||
                    resolved.Frames[resolvedIndex].FrameNumber !=
                        frame.FrameNumber)
                {
                    continue;
                }

                const Engine::FProfilerFrame& resolvedFrame =
                    resolved.Frames[resolvedIndex];
                frame.GpuFrameMs = resolvedFrame.GpuFrameMs;
                frame.GpuValid = resolvedFrame.GpuValid;
                frame.GpuLatencyFrames = resolvedFrame.GpuLatencyFrames;
                frame.Pipeline = resolvedFrame.Pipeline;
            }
            m_MeasurementSnapshot.DroppedGpuFrames =
                resolved.DroppedGpuFrames;
        }

        void Fail_And_Disable(
            Engine::CProfiler* pProfiler,
            const string& reason)
        {
            WriteProfilerCaptureDiagnostic(reason);
            m_MeasurementSnapshot = {};
            pProfiler->Set_Enabled(false);
            m_eState = STATE::COMPLETE;
        }

    private:
        AUTOMATIC_PROFILER_CAPTURE_CONFIG m_Config{};
        STATE m_eState = STATE::OFF;
        uint64_t m_iCompletedWarmupFrames = 0u;
        uint64_t m_iCompletedCaptureFrames = 0u;
        uint32_t m_iCompletedGpuDrainFrames = 0u;
        uint32_t m_iCaptureLevelId = 0u;
        uint64_t m_iLastMeasurementFrame = 0u;
		uint64_t m_iCaptureStartUtc100ns = 0u;
		uint64_t m_iCaptureEndUtc100ns = 0u;
        Engine::FProfilerCaptureSnapshot m_MeasurementSnapshot;
    };

    class CFrameDeadlineWaiter final
    {
    public:
        CFrameDeadlineWaiter()
        {
#ifdef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
            m_hTimer = CreateWaitableTimerExW(
                nullptr,
                nullptr,
                CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
                TIMER_ALL_ACCESS);
#endif
            if (nullptr == m_hTimer)
                m_hTimer = CreateWaitableTimerW(nullptr, FALSE, nullptr);
        }

        ~CFrameDeadlineWaiter()
        {
            if (nullptr != m_hTimer)
                CloseHandle(m_hTimer);
        }

        void Wait(const f32_t remainingSeconds) const
        {
            if (!(remainingSeconds > 0.f))
                return;

            if (nullptr != m_hTimer)
            {
                LARGE_INTEGER dueTime{};
                dueTime.QuadPart = -static_cast<LONGLONG>((std::max)(
                    1.0,
                    std::ceil(static_cast<double>(remainingSeconds) *
                        10000000.0)));
                if (SetWaitableTimer(
                    m_hTimer, &dueTime, 0, nullptr, nullptr, FALSE))
                {
                    const HANDLE handles[] = { m_hTimer };
                    MsgWaitForMultipleObjectsEx(
                        1u,
                        handles,
                        INFINITE,
                        QS_ALLINPUT,
                        MWMO_INPUTAVAILABLE);
                    return;
                }
            }

            const DWORD timeoutMilliseconds = static_cast<DWORD>((std::max)(
                1.0,
                std::ceil(static_cast<double>(remainingSeconds) * 1000.0)));
            MsgWaitForMultipleObjectsEx(
                0u,
                nullptr,
                timeoutMilliseconds,
                QS_ALLINPUT,
                MWMO_INPUTAVAILABLE);
        }

    private:
        HANDLE m_hTimer = nullptr;
    };

    void WriteExitDiagnostic(const char* reason, const HRESULT result = S_OK)
    {
#ifdef _DEBUG
        std::ofstream output(
            "ClientExit.user.log",
            std::ios::binary | std::ios::app);
        if (!output)
            return;

        SYSTEMTIME time{};
        ::GetLocalTime(&time);
        output << std::setfill('0')
            << time.wYear << '-'
            << std::setw(2) << time.wMonth << '-'
            << std::setw(2) << time.wDay << ' '
            << std::setw(2) << time.wHour << ':'
            << std::setw(2) << time.wMinute << ':'
            << std::setw(2) << time.wSecond
            << " reason=" << reason
            << " hr=0x" << std::hex << std::uppercase
            << static_cast<unsigned long>(result)
            << std::dec
            << " level=" << CGameInstance::Get().Get_CurrentLevelID()
            << '\n';
#else
        UNREFERENCED_PARAMETER(reason);
        UNREFERENCED_PARAMETER(result);
#endif
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    AUTOMATIC_PROFILER_CAPTURE_CONFIG automaticCaptureConfig{};
    string automaticCaptureConfigError;
    if (!TryLoadAutomaticProfilerCaptureConfig(
        automaticCaptureConfig,
        automaticCaptureConfigError))
    {
        WriteProfilerCaptureDiagnostic(
            "configuration rejected: " + automaticCaptureConfigError);
        return 2;
    }

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        WriteExitDiagnostic("InitInstance failed", E_FAIL);
        return FALSE;
    }

    auto    pMainApp = CMainApp::Create();
    if (nullptr == pMainApp)
    {
        WriteExitDiagnostic("MainApp creation failed", E_FAIL);
        return 1;
    }

    if (FAILED(CGameInstance::Get().Add_Timer(TEXT("Timer_Default"))))
    {
        WriteExitDiagnostic("Timer_Default creation failed", E_FAIL);
        return FALSE;
    }
    if (FAILED(CGameInstance::Get().Add_Timer(TEXT("Timer_60"))))
    {
        WriteExitDiagnostic("Timer_60 creation failed", E_FAIL);
        return FALSE;
    }

    Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler();
    CAutomaticProfilerCapture automaticCapture;
    string automaticCaptureInitializeError;
    if (!automaticCapture.Initialize(
        automaticCaptureConfig,
        pProfiler,
        automaticCaptureInitializeError))
    {
        WriteProfilerCaptureDiagnostic(
            "initialization failed: " + automaticCaptureInitializeError);
        return 2;
    }


    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT));

    MSG msg{};

    constexpr f32_t FRAME_INTERVAL_SECONDS = 1.f / 60.f;
    CFrameDeadlineWaiter frameDeadlineWaiter;
    f32_t fTimeAcc = {};
    bool_t isQuitRequested = false;
    uint64_t iPendingFrameWaitMicroseconds = 0u;
    chrono::steady_clock::time_point previousProfiledFrameBegin{};
    bool_t hasPreviousProfiledFrameBegin = false;

    // 기본 메시지 루프입니다:
    while (!isQuitRequested)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (WM_QUIT == msg.message)
            {
                WriteExitDiagnostic("WM_QUIT", S_OK);
                isQuitRequested = true;
                break;
            }
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        if (isQuitRequested)
            break;

        CGameInstance::Get().Update_TimeDelta(TEXT("Timer_Default"));

        fTimeAcc += CGameInstance::Get().Get_TimeDelta(TEXT("Timer_Default"));

        if (fTimeAcc < FRAME_INTERVAL_SECONDS)
        {
            if (nullptr != pProfiler && pProfiler->Is_Enabled())
            {
                const auto waitBegin = chrono::steady_clock::now();
                frameDeadlineWaiter.Wait(FRAME_INTERVAL_SECONDS - fTimeAcc);
                iPendingFrameWaitMicroseconds += DurationMicroseconds(
                    chrono::steady_clock::now() - waitBegin);
            }
            else
            {
                frameDeadlineWaiter.Wait(FRAME_INTERVAL_SECONDS - fTimeAcc);
                iPendingFrameWaitMicroseconds = 0u;
            }
            continue;
        }

        CGameInstance::Get().Update_TimeDelta(TEXT("Timer_60"));

        automaticCapture.Prepare_Frame(pProfiler);
        const bool_t isProfilingFrame =
            nullptr != pProfiler && pProfiler->Is_Enabled();
        const auto profiledFrameBegin = isProfilingFrame ?
            chrono::steady_clock::now() : chrono::steady_clock::time_point{};
        const uint64_t frameCadenceMicroseconds =
            isProfilingFrame && hasPreviousProfiledFrameBegin ?
            DurationMicroseconds(
                profiledFrameBegin - previousProfiledFrameBegin) : 0u;
        if (isProfilingFrame)
        {
            previousProfiledFrameBegin = profiledFrameBegin;
            hasPreviousProfiledFrameBegin = true;
        }
        else
        {
            hasPreviousProfiledFrameBegin = false;
        }
        if (nullptr != pProfiler)
        {
            pProfiler->Begin_Frame();
            pProfiler->Set_Counter(
                Engine::EProfilerCounter::FrameWaitMicroseconds,
                iPendingFrameWaitMicroseconds);
            pProfiler->Set_Counter(
                Engine::EProfilerCounter::FrameCadenceMicroseconds,
                frameCadenceMicroseconds);
        }
        iPendingFrameWaitMicroseconds = 0u;

        const auto updateBegin = isProfilingFrame ?
            chrono::steady_clock::now() : chrono::steady_clock::time_point{};
        {
            Engine::CProfilerScope scope(pProfiler, "Client.Update");
            pMainApp->Update(CGameInstance::Get().Get_TimeDelta(TEXT("Timer_60")));
        }
        if (isProfilingFrame)
        {
            pProfiler->Set_Counter(
                Engine::EProfilerCounter::FrameUpdateMicroseconds,
                DurationMicroseconds(chrono::steady_clock::now() - updateBegin));
        }

        HRESULT hRenderResult = S_OK;
        const auto renderBegin = isProfilingFrame ?
            chrono::steady_clock::now() : chrono::steady_clock::time_point{};
        {
            Engine::CProfilerScope scope(pProfiler, "Client.Render");
            hRenderResult = pMainApp->Render();
        }
        if (isProfilingFrame)
        {
            pProfiler->Set_Counter(
                Engine::EProfilerCounter::FrameRenderMicroseconds,
                DurationMicroseconds(chrono::steady_clock::now() - renderBegin));
        }

		const HRESULT hPresentResult = SUCCEEDED(hRenderResult) ?
			pMainApp->Present() : hRenderResult;

        if (nullptr != pProfiler)
        {
            pProfiler->End_Frame();
            automaticCapture.On_Frame_Ended(pProfiler);
        }

        if (FAILED(hRenderResult))
        {
            WriteExitDiagnostic("Render failed", hRenderResult);
            break;
        }
		if (FAILED(hPresentResult))
		{
			WriteExitDiagnostic("Present failed", hPresentResult);
			break;
		}

        /* Keep the sub-frame remainder, but do not render a burst of stale
           catch-up frames after a debugger pause or a long loading hitch. */
        fTimeAcc = std::fmod(fTimeAcc, FRAME_INTERVAL_SECONDS);
    }

    automaticCapture.On_ApplicationExit(pProfiler);
    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENT));
    wcex.hCursor        = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_DEFAULT));
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    g_hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   RECT     rcWindow = { 0, 0, g_iWinSizeX, g_iWinSizeY };

   AdjustWindowRect(&rcWindow, WS_OVERLAPPEDWINDOW, true);

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   g_hWnd = hWnd;

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (CImGuiLayer::HandleWindowMessage(hWnd, message, wParam, lParam))
        return 1;

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
