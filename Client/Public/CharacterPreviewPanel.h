#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <array>

NS_BEGIN(Client)

class CPart_Body;
struct ANIMATION_PREVIEW_ASSET;

enum class CHARACTER_PREVIEW_LOCK_OWNER : uint8_t
{
	ANIMATION_TOOL,
	EFFECT_TOOL,
	END
};

// Owns the one debug preview character every authoring tool looks at.
//
// This lifecycle used to live inside CAnimation_Tool, which made the preview a
// private detail of one tool even though Effect authoring needs the same target.
// MainApp shares one panel between tools. The panel owns creation, teardown and
// level scoping; it publishes the result
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
	void Render_Selector(
		bool_t isLocked,
		const string& strLockReason,
		bool_t includePreviewOnlyTargets = true);

	// Drops the preview when the level it was staged into is gone. Callers run
	// this before reading the target each frame.
	void Refresh_Level();
	void Set_SessionLock(
		CHARACTER_PREVIEW_LOCK_OWNER eOwner,
		bool_t isLocked,
		string strReason);

	// Selects one admitted authoring model by its stable animation asset name.
	// Effect Tool uses this to replace a retained preview-only prop (for example
	// Dimension Core) with the playable class body that owns the loaded skill.
	bool_t Select_TargetAsset(const string& strAnimationAssetName);

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
	array<bool_t, ETOI(CHARACTER_PREVIEW_LOCK_OWNER::END)> m_SessionLocks{};
	array<string, ETOI(CHARACTER_PREVIEW_LOCK_OWNER::END)> m_SessionLockReasons{};
};

NS_END
