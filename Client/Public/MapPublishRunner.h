#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>

NS_BEGIN(Client)

/* Runs Tools/MapPipeline/Publish-MapAuthoring.ps1 for one Area and scope as a
   detached process, so an authoring tool keeps drawing while the runtime files
   are rebuilt. The publisher owns the runtime file transaction and its own
   rollback; this only starts it, logs it under %TEMP% and reports the exit
   code once. */
class CMapPublishRunner final
{
public:
	CMapPublishRunner() = default;
	~CMapPublishRunner();
	CMapPublishRunner(const CMapPublishRunner&) = delete;
	CMapPublishRunner& operator=(const CMapPublishRunner&) = delete;

	bool_t Is_Running() const { return nullptr != m_hProcess; }
	const std::string& Get_AreaId() const { return m_AreaId; }
	const std::string& Get_Scope() const { return m_Scope; }
	const std::filesystem::path& Get_LogPath() const { return m_LogPath; }

	/* scope is one of the publisher's Area / WorldSequences / Lights. logTag
	   names the log file so two tools never write the same one. */
	bool_t Start(
		const std::string& areaId,
		const std::string& scope,
		const std::wstring& logTag,
		std::string& outStatus);
	/* True exactly once after the started process has exited. */
	bool_t Poll(bool_t& outSucceeded, uint32_t& outExitCode);

private:
	HANDLE m_hProcess = nullptr;
	std::string m_AreaId;
	std::string m_Scope;
	std::filesystem::path m_LogPath;
};

NS_END
