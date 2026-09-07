#pragma once

#include "Client_Defines.h"
#include "MapLightDocument.h"
#include "PresentationProvider.h"

#include <filesystem>
#include <memory>
#include <string>

NS_BEGIN(Client)

class CMapLightPresentationRuntime final :
	public IPresentationProvider,
	public std::enable_shared_from_this<CMapLightPresentationRuntime>
{
public:
	bool_t Load(
		const std::filesystem::path& path,
		const std::string& expectedAreaId);
	bool_t Load_Runtime(const std::string& areaId);
	bool_t Replace_Document(const CMapLightDocument& document);
	bool_t Submit_Frame();
	virtual HRESULT Submit_Presentation() override;
	void Clear();

	bool_t Is_Ready() const { return m_Document.Is_Ready(); }
	const std::string& Get_Status() const { return m_Status; }
	const CMapLightDocument& Get_Document() const { return m_Document; }

private:
	friend class CRenderingProfileService;
	/* The one active scene commits its validated scalar after all fallible render changes.
	   Providers read it at submission, after the current frame has selected its scene. */
	static void Commit_SceneIntensityMultiplier(const f32_t multiplier)
	{
		s_fSceneIntensityMultiplier = multiplier;
	}
	inline static f32_t s_fSceneIntensityMultiplier = 1.f;
	CMapLightDocument m_Document;
	std::string m_Status = "Map light presentation is not loaded";
};

NS_END
