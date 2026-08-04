#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

class CPart_Body;
struct ANIMATION_PREVIEW_ASSET;

// Owns the debug preview character every authoring tool looks at.
//
// This lifecycle used to live inside CAnimation_Tool, which made the preview a
// private detail of one tool even though Effect authoring needs the same target.
// The panel owns creation, teardown and level scoping; it publishes the result
// through CAnimationTargetService so tools read one contract instead of
// searching a level/layer/part by convention.
//
// The panel deliberately does not own clip playback, animation events or effect
// assets. Those stay with the tool that authors them.
class CCharacterPreviewPanel final
{
public:
	~CCharacterPreviewPanel();

	// Draws the target selector. `isLocked` disables switching while the caller
	// holds unsaved work, so a dirty document cannot lose its target.
	void Render_Selector(bool_t isLocked, const string& strLockReason);

	// Drops the preview when the level it was staged into is gone. Callers run
	// this before reading the target each frame.
	void Refresh_Level();

	void Release(bool_t removeFromLayer);

	bool_t Is_PreviewActive() const {
		return !m_pPreviewBody.expired();
	}

	const string& Get_Status() const {
		return m_Status;
	}

private:
	bool_t Select_Asset(const ANIMATION_PREVIEW_ASSET& asset);

private:
	weak_ptr<CPart_Body> m_pPreviewBody;
	const ANIMATION_PREVIEW_ASSET* m_pPreviewAsset = nullptr;
	uint32_t m_iPreviewLevelIndex = UINT32_MAX;
	// The staged body is parented to this matrix, so it must outlive the body
	// and never move once the asset is selected.
	float4x4_t m_PreviewParentMatrix{};
	string m_Status;
};

NS_END
