#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "CombatHUDViewModel.h"

#include <memory>

NS_BEGIN(Client)

class CUILayoutRuntime;

/* KoukuSaydon minigame time limit -- the emblem and countdown that sit in the
screen's top-left corner during the card maze and the Mario stage.

Traced from the shipped client: package EFUI_DUNGEONTIMER, movie dungeontimer.gfx,
class ark.ui.dungeonTimer.DungeonTimerFrame. That class carries a per-raid
titleImageType whose values name the raids outright -- NONE 0, KOUKUSATON 1,
COMMON 2, ILLIAKAN 3, MOCOCONG 4, ECHIDNA 5, KAZEROTH 6 -- and shows the matching
frame with raidImageMc.gotoAndStop(type + 1), so KoukuSaydon is frame 2. That
frame's art is the 241x218 crop this view draws.

Authored and reproduced here:
  * emblem   raidImageMc at the widget origin, 241x218.
  * number   timeTF, font $YoonGasiIIM (= Font_YoonGasiIIM) at 34 pt, field
             206x44.8 placed at (21, 133), so it centres under the emblem.
  * under    below warningTime the movie hides timeTF and splits the value across
             underBigTF (34 pt, 100x44.8 at (19, 133)) and underSmallTF (24 pt,
             100x32.8 at (116, 140)) -- a seconds-and-hundredths readout.
  * colour   read at runtime from a hidden "setting" clip whose text fields carry
             them literally: normal 0xFFD200, warning 0xCC3300. The class's own
             fallbacks (0xFFD700 / 0xCC3300) are overridden by that clip.
  * format   TranslateTime.getTime with timeType "min": minutes and seconds both
             zero padded to two digits, joined by ":" with no spaces.

Not reproduced: warningEffect and checkBG. Both are nested animated sprites, not
single crops, so the warning flourish is left out until they are extracted.

Placement is authored after all, on the movie's own main timeline:
  widgetX 298, widgetY 9, pivotType "topLeft", sortingLayer "1_defaultHUD",
  orderInLayer 199, and layoutContents naming both cases --
  "\uB2E8\uB3C5\uC73C\uB85C\uD50C\uB808\uC774\uB418\uC5C8\uC744\uB54C\uC704\uCE58,298,9" (normal play) and
  "\uC5F0\uC2B5\uBAA8\uB4DC\uC5D0\uC11C\uD50C\uB808\uC774\uB418\uC5C8\uC744\uB54C,27,9" (practice mode).
The movie's stage is 1920x1080, so every number above scales by 1280/1920 into
this project's layout reference. Data/UI/KoukuSaydon/DungeonTimer_Layout.json
carries the result and everything else derives from that slot's width -- move or
resize the slot and the number follows.

Screen-anchored, unlike the world-anchored madness gauge, so CMainApp owns this
the way it owns the boss bar: it draws in every Level, including the one the HUD
Layout Tool is used from.

This view owns no gameplay state. It reads the seconds it is handed. */
class CDungeonTimerView final
{
public:
	CDungeonTimerView(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iGameObjectLevelIndex);
	~CDungeonTimerView();

public:
	void Update(f32_t fTimeDelta, const HUD_DUNGEON_TIMER_STATE& State);
	/* Called from CMainApp's HUD text pass: the emblem is a layout slot the engine
	draws, the number is text drawn here. */
	void Render() const;
	void Hide();

private:
	unique_ptr<CUILayoutRuntime>	m_pView;
	/* Authored emblem rect in reference units; every other offset scales off it. */
	f32_t							m_fEmblemX = 0.f;
	f32_t							m_fEmblemY = 0.f;
	f32_t							m_fEmblemWidth = 0.f;
	f32_t							m_fEmblemHeight = 0.f;
	bool_t							m_bSlotFound = false;
	bool_t							m_bVisible = false;
	f32_t							m_fSeconds = 0.f;
	f32_t							m_fWarningSeconds = 10.f;
};

NS_END
