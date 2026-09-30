#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include <memory>
#include <string>

namespace LostArk::Shared { struct S2C_COLOSSEUM_MATCH_STATE; }

NS_BEGIN(Client)
class CCamera_Free;
class CClientReplication;

// Read-only match presentation. The Level owns the server state/clock and consumes
// the Bern-return intent through IPlayerCommandSink; this view never sends packets.
class CColosseumMatchView final
{
public:
	CColosseumMatchView();
	~CColosseumMatchView();
	CColosseumMatchView(const CColosseumMatchView&) = delete;
	CColosseumMatchView& operator=(const CColosseumMatchView&) = delete;
	bool_t Initialize(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context,
		uint32_t level, const std::shared_ptr<CCamera_Free>& camera);
	void Update(f32_t delta, const CClientReplication& replication,
		const LostArk::Shared::S2C_COLOSSEUM_MATCH_STATE& state, double serverTick);
	void Render();
	bool_t Is_CinematicActive() const;
	bool_t Consume_ReturnIntent();
	const std::string& Get_Status() const;

#ifdef _DEBUG
	// Presentation only: never changes replication or submits a match/return command.
	enum class DEBUG_PREVIEW { NONE, VICTORY_CUTSCENE, SCORE_HUD, VICTORY_UI, DEFEAT_UI };
	bool_t Play_DebugPreview(DEBUG_PREVIEW preview, const CClientReplication& replication);
	void Update_DebugPreview(f32_t delta, const CClientReplication& replication);
	void Stop_DebugPreview();
	bool_t Is_DebugPreviewActive() const;
	bool_t Is_DebugPreviewPaused() const;
	void Set_DebugPreviewPaused(bool_t paused);
	f32_t Get_DebugPreviewClockMs() const;
#endif

private:
	void Sample_Presentation(const CClientReplication& replication, bool allowReturn);
	struct IMPLEMENTATION;
	std::unique_ptr<IMPLEMENTATION> m_Impl;
};
NS_END
