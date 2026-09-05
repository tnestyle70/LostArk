#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <array>
#include <unordered_set>
#include <utility>
#include <vector>

NS_BEGIN(Engine)
class CGameObject;
class CModel;
NS_END

NS_BEGIN(Client)

struct ANIMATION_PREVIEW_ASSET;

enum class CHARACTER_PREVIEW_LOCK_OWNER : uint8_t
{
	ANIMATION_TOOL,
	EFFECT_TOOL,
	EQUIPMENT_TOOL,
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
	CCharacterPreviewPanel(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext)
		: m_pDevice{ std::move(pDevice) },
		m_pContext{ std::move(pContext) }
	{
	}
	~CCharacterPreviewPanel();
	void On_LevelChanged();

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
	// Changes only this panel's selected generic preview root. The multiplier
	// always starts from the selected/recentered baseline, never the last scale.
	bool_t Set_PreviewScaleMultiplier(
		const shared_ptr<Engine::CModel>& expectedModel, f32_t multiplier);
	// Samples the selected animated hammer from the body's current source time
	// and exact right-hand socket. Called after body playback/seek has settled.
	void Synchronize_PreviewWeapon();

	void Release(bool_t removeFromLayer);

	bool_t Is_PreviewActive() const {
		return !m_pPreviewObject.expired();
	}

	const string& Get_Status() const {
		return m_Status;
	}

private:
	bool_t Select_Asset(const ANIMATION_PREVIEW_ASSET& asset);
	/* True when the asset declares a complete socketed weapon. A partially
	   declared one is a data error, so it is reported instead of previewed. */
	static bool_t Declares_Weapon(const ANIMATION_PREVIEW_ASSET& asset);

private:
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	/* Playable targets hold a complete CCharacter, Valtan holds its complete
	   product CValtan composition, and preview-only props hold CPart_Body. */
	weak_ptr<Engine::CGameObject> m_pPreviewObject;
	/* The optional socketed weapon of a CPart_Body preview. It is a sibling in
	   the same layer rather than a child, so it is released with the body it
	   rides; an empty slot simply means the target declares no weapon. */
	weak_ptr<Engine::CGameObject> m_pPreviewWeaponObject;
	const ANIMATION_PREVIEW_ASSET* m_pPreviewAsset = nullptr;
	uint32_t m_iPreviewLevelIndex = UINT32_MAX;
	/* Generic CPart_Body previews retain a raw parent pointer. The inactive slot
	   is staged first and becomes active only after the new object validates, so
	   a failed selection cannot move the previously committed body/root apart. */
	array<float4x4_t, 2u> m_PreviewParentMatrices{};
	float4x4_t m_PreviewUnscaledParentMatrix{};
	array<float4x4_t, 2u> m_PreviewWeaponParentMatrices{};
	std::vector<float4x4_t> m_PreviewWeaponRestPose;
	size_t m_iPreviewParentMatrixIndex = 0u;
	uint32_t m_iPreparedGenericPreviewLevelIndex = UINT32_MAX;
	std::unordered_set<string> m_PreparedGenericPreviewAssetIds;
	string m_Status;
	array<bool_t, ETOI(CHARACTER_PREVIEW_LOCK_OWNER::END)> m_SessionLocks{};
	array<string, ETOI(CHARACTER_PREVIEW_LOCK_OWNER::END)> m_SessionLockReasons{};
};

NS_END
