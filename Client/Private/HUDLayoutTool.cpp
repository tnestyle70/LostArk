#include "imgui.h"

#include "HUDLayoutTool.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "CombatHUDViewModel.h"
#include "MainApp.h"

#include <fstream>
#include <filesystem>
#include <cstring>
#include <cmath>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <unordered_set>

namespace
{
	struct DOCUMENT_DEF
	{
		const char*	szLabel;
		const char*	szDataPath;
		const char*	szTextureRoot;
		bool		bPerClass;
	};

	constexpr DOCUMENT_DEF g_Documents[] =
	{
		{ "Combat HUD",     "UI/HUD/HUD_Layout.json",           "UI/HUD/",      true  },
		{ "Screen UI",      "UI/ScreenUI/ScreenUI.json",        "UI/ScreenUI/", false },
		{ "Loading Screen", "UI/Loading/LoadingLayout.json", "UI/Loading/",  false },
		{ "Skill Window",   "UI/SkillWindow/SkillWindow_Layout.json", "UI/SkillWindow/", false },
		{ "Lobby",          "UI/Lobby/Lobby_Layout.json", "UI/Lobby/TitleBackground/", false },
		{ "Class Select",   "UI/ClassSelect/ClassSelect_Layout.json", "UI/ClassSelect/", true  },
		/* Top-right area minimap frame/markers (retail EFUI_MAP minimap.gfx). Map images and
		markers are placed at runtime; the palette root is deliberately the small control set. */
		{ "Minimap",        "UI/Minimap/Minimap_Layout.json", "UI/Minimap/", false },
		/* KoukuSaydon madness gauge (retail indicator.gfx madnessGauge). Authored around
		the Madness_Anchor slot; CKoukuMadnessGaugeView moves the whole group to the local
		character's head point at runtime, so only the offsets between slots matter here. */
		{ "Kouku Madness",  "UI/KoukuSaydon/MadnessGauge_Layout.json", "UI/KoukuSaydon/Madness/", false },
		/* Target/boss status display -- not the local player's own class (Combat HUD) and not the
		always-on top/bottom menu chrome (Screen UI), so it gets its own document instead of being
		squeezed into either. */
		{ "Boss UI",        "UI/BossUI/BossUI.json", "UI/BossUI/", false },
		/* Esther skill-select window (3 portrait slots + shared charge gauge) -- shared across
		classes like Boss UI, so its own document rather than squeezed into Combat HUD. */
		{ "Esther UI",      "UI/Esther/EstherUI.json", "UI/Esther/", false },
		/* Inventory panel -- background/tabs/material icons and the real 10-column slot grid
		traced from the actual inventory.gfx data (mainBag origin/slotSize/spacing), scaled from
		its native 1920x1080 authoring canvas to this project's 1280x720 HUD reference. Slot cell
		border art (ARKSlotBackgroundV2) lives in a separate shared component package that hasn't
		been located yet, so the 30 Inventory_Slot_* markers are position-only (empty layers) for
		now -- real slot border art is a follow-up, not fabricated here. */
		{ "Inventory UI",   "UI/Inventory/InventoryUI.json", "UI/Inventory/", false },
		/* Character info window (P) -- retail characterinfo.gfx equipment tab, every rect the
		1920x1080 placement trace scaled by 2/3 around a centred window origin; shared V2 window/
		tab/button skins and the equipment slot silhouettes come from shareImageV2, the item icons
		from the EFUI_ICONATLAS pages (CCharacterInfoWindowView). */
		{ "Character Info", "UI/CharacterInfo/CharacterInfo_Layout.json", "UI/CharacterInfo/", false },
		/* Avatar book (아바타 도감) window content, avatarbook.gfx + preview.gfx camera controls
		(CAvatarBookWindowView). */
		{ "Avatar Book", "UI/AvatarBook/AvatarBook_Layout.json", "UI/AvatarBook/", false },
		/* Item enhancement (장비 재련) window content. itemupgrade.gfx (the original source for this
		document) turned out to be an outdated/wrong-version window -- confirmed against a real
		current in-game screenshot and against itembuilduplevel.gfx's identical `enhance.*` loc-key
		namespace. Rebuilt from itembuilduplevel.gfx's real ItemBuildUpLevelWndContent (char 992)
		SymbolClassTag/PlaceObject placement matrices (native 1920x1080 canvas, scaled to this
		project's 1280x720 HUD reference). Panel background, the materials-row backdrop, the
		success-gauge cover/fill, post/auction quick-buy buttons, the breakthrough-recipe backdrop,
		the success/fail result modal background, two grade-tier badge crops, the combobox
		background/arrow, the shared V2btn button skin (buildUp/continuously/success-ok/fail-ok/
		levelUp), the level-up-complete glow, and several per-row/per-state "example" crops (list
		selected/locked/disabled, grade-list selected/side-strips, recipe icon frame) are real
		bitmaps -- most resolved through itembuilduplevel.xml's own DefineSubImage bitmapId->imageId
		chain, the shared V2 widgets (buttons/dropdown/lock icon) resolved one hop further through
		the external EFUI_ShareImage/shareImageV2.swf and shareImage.swf packages named in
		itembuilduplevel.xml's own ImportAssets2Tag (their native pixel size matches the traced
		shapeBounds/DefineSubImage region, and buildUp_btn/continuously_btn's on-screen size also
		applies their real root placement scale, 2.174652x/1.277771y, on top of the 103x36 native
		V2btn_normal bitmap -- but rendered at native size anyway, never stretched, per explicit
		direction; same for BuildUpIconFrame/SpecialMaterialSlot below despite their own real
		placement scale). ItemUpgrade_ResultEffect (35 frames, 483x320) and ItemUpgrade_
		CompleteEffect (80 frames, 150x150) are real flipbooks -- itembuilduplevel.gfx's own
		"result_*"/"completeEffectCore_*" character families, both real children of
		levelUpComplete_mc(569), confirmed by trace-to-root (not the external EFUI_Effect package).
		A named "ItemUpgrade_SuccessEffect" using EFUI_Effect's "energeEffect_*" briefly stood in
		this slot's place -- wrong: matched on "successEffect" appearing inside a longer wrapper
		name (smeltEffect_slotEnergyEffect_successEffect) without ever checking the actual frame
		color, which turned out to be plain cyan, not the real gold/orange burst. Real playback fps
		for these two is itembuilduplevel.gfx's own actual frameRate (40.0), not a guessed default.
		The generic item-slot frame (ItemUpgrade_BuildUpIconFrame/SpecialMaterialSlot) is arkSlot_renew_basic_V2
		-> arkSlot_renew_basic_Frame_V2 -> V2Slot_border, a real 70x70 crop from
		EFUI_ShareImage/shareImageV2.swf's own shareImageV2_I6.tga, scaled by each slot's own real
		root placement scale -- this is the same shared symbol behind the long-standing InventoryUI
		ARKSlotBackgroundV2 gap (see reference_lostark_umodel_extraction memory). noReforgingMc/
		noneCostMc are real backdrop strips resolved directly in itembuilduplevel.xml. What's left
		with a visible authoring marker/empty layer is either a genuine dynamic text field
		(className=SimpleLabel/SimpleLabel_YoonGasiIIM in the real trace -- there was never a bitmap
		to find), a vector-only fill with no texture at all (itemBuildUpGradeList's own background
		shape, bitmapId=None), or a container that genuinely has no background of its own
		(itemBuildUpList's only children are 14 row-template instances, no frame shape) -- these
		three are dead ends, not unfinished work. Anything else empty is a real remaining gap.
		PanelBg's rect was WRONG in an earlier version (borrowed detailInfoBox's anchor because its
		own placing shape was never traced) -- caught from a real in-game screenshot not lining up,
		then fixed for real: bitmapId 299 (the saved crop's exact source region) is shape char300's
		fill, char300 sits inside deco_mc(303) at char992's own root (0,0), and char300's own
		shapeBounds (381,1)-(1420,730) is the real on-screen rect -- not the crop's native 663x728
		(the fill stretches non-1:1 across a wider vector shape, same as every other CUI_Sprite
		draw). The list/grade/recipe per-row "example" pieces' positions are NOT further traceable:
		exhaustively searched itembuilduplevel.xml for every PlaceObject2Tag/PlaceObject3Tag whose
		characterId matches each row-renderer template and its internal state children (296, 1049,
		1086, 1046, 1047, 1048, 1069, 1064, 1066, 84, 81) -- all return zero placements anywhere in
		the document. Scaleform List/DataGrid item renderers are instantiated purely by AS3 at
		runtime (row Y = index * itemRenderer height, computed in ActionScript, never baked into the
		SWF timeline), so no real static position exists for any of them -- confirmed dead end, not
		an unfinished trace. GaugeCover had the same borrowed-anchor bug as PanelBg (used
		percentGauge/407's own root instead of char409's real parent, sprite 410, itself char992's
		OWN unnamed child at (594,212) -- caught because the user noticed the item-icon frame and
		the gauge ring share the same real X in the actual reference screenshot, which this fix now
		matches almost exactly). After these two hits, every remaining real-art anchor was
		re-verified by a generic trace-to-root script (walk each art shape's real containing sprite
		chain all the way up to char992, summing local offsets, instead of trusting a same-area
		named field's position) -- RecipeSlotBg/PostBtn/AuctionBtn/BreakThroughBg/SuccessModalBg/
		FailModalBg/NoReforgingMc/NoneCostMc/CompleteGlow/successOkBtn/failOkBtn all traced back
		exactly matching what was already used (no bug). Two more real bugs turned up this pass:
		GaugeFill was computed as "centered inside GaugeCover", but it isn't GaugeCover's child at
		all -- its real shape (char337) traces through char338 into percentGauge(407) instead, a
		SEPARATE real sprite tree from GaugeCover's (410) that only visually happens to overlap on
		screen, giving a real independent anchor (533,265) that matches neither the old nor the
		"centered" guess. FailDeco's real shape (char898, inside fail_mc) traces to (666,54), not
		the (613,54) used before -- a plain arithmetic slip, not a wrong-parent bug like the other
		three. Lesson for next time an anchor is picked by "borrow the nearby named field's
		position": verify the actual containing sprite chain instead of assuming proximity means
		parentage. Success/complete effect correction: ItemUpgrade_SuccessEffect (EFUI_Effect's
		"energeEffect_*", cyan) was a wrong name-match and was replaced by ItemUpgrade_ResultEffect/
		CompleteEffect -- itembuilduplevel.gfx's own real "result_*"/"completeEffectCore_*" families
		(orange/gold, confirmed by opening actual frames), both real children of levelUpComplete_mc.
		Their wrapper sprites (char446/538) also carry a real uniform placement scale the
		translate-only trace had missed (1.1428x/1.1437y and 0.9800x/0.9989y) -- applied on top of
		native size since it's a genuine SWF transform, not an artificial rect-fit stretch (unlike
        BuildUpBtn/BuildUpIconFrame's own real placement scale, which per explicit direction stays
		unapplied -- every slot renders at native pixel size unless a full transform-chain audit
		confirms the scale is real and non-degenerate). GaugeFill's own chain scale is (0,0) and
		NoReforgingMc's is a wildly non-uniform (1.16,3.04) -- both are almost certainly animation-
		tween snapshots (percentGauge/noRequiredRecipe_mc grow in from a hidden/collapsed start
		state), not real static design values, so both were left at native size on purpose.
		Full-page sweep (rounds 4-6): every DefineSubImage region on all 9 real atlas pages this
		document uses (itemBuildUpLevel_I43/I60/IAA/I102/I112/I1C1/IB7/I115/IB5.tga) was resolved
		through bitmapId->shape->trace-to-root, not just the handful first noticed -- I43 alone has
		47 regions and I60 has 33; most were never even opened before a real screenshot (the user's)
		showed a gold winged-ring piece from IB7 that had no slot at all. The result is ~60 more real
		crops (lockMotion_mc's own ~14-piece cluster, levelUpMotion_mc1/mc2's own pieces, several
		success_mc/childWindow/equipExpPage backgrounds, detailInfoBox's own real bg_mc distinct from
		PanelBg, etc.) -- multi-state button hover/pressed variants were skipped since one
		representative state per button is already wired and no hover-state swap logic exists yet.
		Lesson: "I checked the pieces I noticed" is not the same as "I checked the page" -- when a
		crop comes from an atlas page, list every other region on that same page before moving on. */
		{ "Item Upgrade",   "UI/ItemUpgrade/ItemUpgradeUI.json", "UI/ItemUpgrade/", false },
		/* Valtan death screen (CLevel_ValtanArena, real deadscene.gfx). All 4 image slots are sized
		to their real extracted PNG's native pixel dimensions on purpose -- do not stretch them to an
		arbitrary rect here; resize in this Tool if a different on-screen size is wanted, the same way
		any dropped texture auto-fits its slot. DeadScene_WingedArch and DeadScene_Effect's positions
		are real: both traced from deadscene.xml's own placement matrices relative to "bg" (the panel
		art's own placement, char109), not guessed -- the arch sits mostly above the panel with only
		its lower wingtips overlapping, and the ambient ring/smoke effect is centered further back and
		taller than either, per the real relative offsets. DeadScene_TitleTextMarker is a synthesized
		placeholder (no bitmap exists for it in the source) sized so "사망하였습니다" fills it --
		CMainApp::RenderDeadSceneText() draws the real Korean text separately via CGameInstance::
		Draw_Text (ImGui's own font lacks Korean glyphs), so this slot only marks where that text
		lands. DeadScene_ReviveButton reuses the same generic pill-button skin as ItemUpgrade's
		buildUp/continuously/success-ok/fail-ok buttons (V2btn_normal, real 103x36). */
		{ "Dead Scene",     "UI/DeadScene/DeadSceneUI.json", "UI/DeadScene/", false },
		/* Bern's Valtan-entry confirm window (CLevel_Bern::Render_ValtanEntryModal).
		Panel/Confirm/Cancel currently reuse Class Select's create-character modal
		assets (UI/ClassSelect/Common/); szTextureRoot points at UI/RaidEntry/ instead so
		this window's own art (e.g. Accept.png/Decline.png) shows in the palette. */
		{ "Valtan Entry",   "UI/RaidEntry/BernValtanEntry_Layout.json", "UI/RaidEntry/", false },
		/* Same-room party invite popup (CPartyInteractionView::Render_InvitePopup).
		Cloned from Valtan Entry at 0.7x -- see that document's own comment for
		why Panel/Confirm/Cancel reuse Class Select's create-character modal art. */
		{ "Party Invite",   "UI/Party/PartyInviteConfirm_Layout.json", "UI/Party/", false },
		/* Right-click-another-player menu (CPartyInteractionView::Render_ContextMenu).
		Placeholder art (not extracted from the original game) pending a real
		asset find -- Panel/HoverHighlight starting rects are arbitrary, real
		placement is done in this Tool. */
		{ "Party Context Menu", "UI/Party/PartyContextMenu_Layout.json", "UI/Party/", false },
		/* Valtan clear celebration overlay -- real trace of EFUI_EPICGATECOMMONCLEAR's
		epicgatecommonclear.gfx (driver class ark.ui.epicGateCommonClear.EpicGateCommonClearFrame,
		result_101/"EpicGateCommonClearSuccessSet101" variant -- the one entry in that document not
		named "epicGateCommanderClearSuccess_SetNN", matching this package's own "COMMON" clear
		naming rather than a specific commander's own art). RaidClear_TitleTextBox is a
		position-only marker (no bitmap in the source either) for CMainApp::RenderRaidClearText()'s
		real "[$]commander.dungeon_clear" ("던전 클리어") headline, same split as Dead Scene's own
		TitleTextMarker/RenderDeadSceneText. BgFlash/Emblem are real crops from
		epicgatecommonclear_i6a.dds. */
		{ "Raid Clear",     "UI/RaidClear/RaidClear_Layout.json", "UI/RaidClear/", false },
		/* Commander raid entry window. Everything the source could give is already placed from
		epicgatecommanderentrance.gfx: instances sitting directly in EpicGateCommanderEntranceContent
		carry real PlaceObject matrices, traced on that movie's own 1920x1080 canvas and then scaled
		by a uniform 2/3 (1280/1920 == 720/1080, so no aspect change) onto this project's shared
		1280x720 authoring canvas.

		What the source cannot give is anything inside a runtime-laid-out list -- raidStageTab's
		boss tabs, rewardList's tiles, bindConditionList's rows. Their authored child coordinates
		are scratch positions, not final layout: the tab frames sit at a regular 123px x spacing but
		at three different y values (116/156.1/187), because the component repositions them itself
		at runtime. Those are placed by hand here rather than guessed from the shipped art. */
		{ "Raid Entry",     "UI/RaidEntry/ValtanRaidEntry_Layout.json", "UI/RaidEntry/", false },
		/* Retail-style character-select window over the Lobby (start-sequence Phase 1) --
		traced from the real client's characterselect.gfx (CCharacterSelectWindowView). */
		{ "Character Select Window", "UI/CharacterSelect/CharacterSelectWindow_Layout.json",
			"UI/CharacterSelect/", false },
	};

	constexpr int32_t g_iDocumentCount = static_cast<int32_t>(sizeof(g_Documents) / sizeof(g_Documents[0]));

	struct PALETTE_ENTRY
	{
		Client::CHUDLayoutTool::SLOT_TYPE	eType;
		const char*							szLabel;
		float								fSizeX;
		float								fSizeY;
		bool								bPerClass;
		bool								bRepeatable;
	};

	constexpr PALETTE_ENTRY g_PaletteEntries[] =
	{
		{ Client::CHUDLayoutTool::SLOT_TYPE::EVADE,          "Evade",         48.f,  48.f,  false, false },
		{ Client::CHUDLayoutTool::SLOT_TYPE::POTION,         "Potion",        48.f,  24.f,  false, false },
		{ Client::CHUDLayoutTool::SLOT_TYPE::HEALTHBAR,      "HealthBar",     260.f, 20.f,  false, false },
		{ Client::CHUDLayoutTool::SLOT_TYPE::MANABAR,        "ManaBar",       260.f, 20.f,  false, false },
		{ Client::CHUDLayoutTool::SLOT_TYPE::CLASS_EMBLEM,   "ClassEmblem",   140.f, 140.f, true,  false },
		{ Client::CHUDLayoutTool::SLOT_TYPE::IDENTITY_GAUGE, "IdentityGauge", 200.f, 24.f,  true,  false },
		{ Client::CHUDLayoutTool::SLOT_TYPE::SKILL,          "Skill",         44.f,  44.f,  false, true  },
		{ Client::CHUDLayoutTool::SLOT_TYPE::ITEM,           "Item",          46.f,  46.f,  false, true  },
	};

	const PALETTE_ENTRY* Find_Palette_Entry(Client::CHUDLayoutTool::SLOT_TYPE eType)
	{
		for (const PALETTE_ENTRY& Entry : g_PaletteEntries)
			if (Entry.eType == eType)
				return &Entry;

		return nullptr;
	}

	const char* SlotType_Label(Client::CHUDLayoutTool::SLOT_TYPE eType)
	{
		const PALETTE_ENTRY* pEntry = Find_Palette_Entry(eType);
		return pEntry ? pEntry->szLabel : "Custom";
	}

	wstring Utf8_To_Wide(const string& strUtf8)
	{
		if (strUtf8.empty())
			return wstring();

		const int32_t iLen = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
		if (iLen <= 0)
			return wstring();

		wstring strWide(iLen - 1, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, strWide.data(), iLen);

		return strWide;
	}

	string Path_To_Utf8(const filesystem::path& Path)
	{
		const u8string strU8 = Path.u8string();
		return string(strU8.begin(), strU8.end());
	}

	bool_t Is_ValidIdentifier(const string& value)
	{
		if (value.empty() || value.size() > 128u)
			return false;
		return none_of(value.begin(), value.end(), [](const char character)
		{
			return 0 != iscntrl(static_cast<unsigned char>(character));
		});
	}

	bool_t Is_SafeUIResourceId(const string& value)
	{
		if (value.empty() || value.size() > 512u ||
			!value.starts_with("UI/") ||
			string::npos != value.find('\\') ||
			string::npos != value.find(':'))
		{
			return false;
		}

		size_t segmentStart = {};
		while (segmentStart < value.size())
		{
			const size_t separator = value.find('/', segmentStart);
			const size_t segmentEnd = string::npos == separator
				? value.size() : separator;
			const string_view segment(
				value.data() + segmentStart,
				segmentEnd - segmentStart);
			if (segment.empty() || "." == segment || ".." == segment)
				return false;
			segmentStart = segmentEnd + 1u;
		}
		return true;
	}

	/* Best-effort local safety net against the exact failure that lost days of authored HUD work
	once already: this document is edited live in a running debug tool and can go a long time
	between commits, so an unrelated git operation elsewhere in the working tree (checkout, reset,
	a bad merge) can silently wipe it with no way back if nothing local remembers the previous
	state. This keeps the last MAX_BACKUPS on-disk copies of whatever Save() is about to overwrite,
	entirely outside git (each document's .backups folder is gitignored) -- it does not replace committing, but
	it means a lost commit is a recoverable mistake instead of gone work. A backup failure must
	never block the actual save it is protecting. */
	void Backup_PreviousVersion(const filesystem::path& path)
	{
		error_code existsError;
		if (!filesystem::exists(path, existsError) || existsError)
			return;

		const filesystem::path backupDirectory = path.parent_path() / L".backups";
		error_code directoryError;
		filesystem::create_directories(backupDirectory, directoryError);
		if (directoryError)
			return;

		const auto now = chrono::system_clock::now();
		const time_t nowTime = chrono::system_clock::to_time_t(now);
		tm localTime{};
		if (0 != localtime_s(&localTime, &nowTime))
			return;

		wostringstream stamp;
		stamp << put_time(&localTime, L"%Y%m%d_%H%M%S");
		const filesystem::path backupPath = backupDirectory /
			(path.stem().wstring() + L"_" + stamp.str() + path.extension().wstring());

		error_code copyError;
		filesystem::copy_file(path, backupPath,
			filesystem::copy_options::overwrite_existing, copyError);
		if (copyError)
			return;

		/* Prune to the most recent MAX_BACKUPS for this document stem only -- a second document
		(ScreenUI.json, ...) in the same .backups folder keeps its own independent history. */
		constexpr size_t MAX_BACKUPS = 20u;
		vector<filesystem::path> existing;
		error_code iterateError;
		/* A range-based for here would call directory_iterator's throwing operator++ on every
		step after the first (the error_code overload only protects construction), so this steps
		manually through the non-throwing increment(error_code&) instead. */
		filesystem::directory_iterator directoryIt(backupDirectory, iterateError);
		const filesystem::directory_iterator directoryEnd;
		while (!iterateError && directoryEnd != directoryIt)
		{
			const filesystem::path& entryPath = directoryIt->path();
			if (entryPath.extension() == path.extension() &&
				entryPath.stem().wstring().starts_with(path.stem().wstring() + L"_"))
			{
				existing.push_back(entryPath);
			}
			directoryIt.increment(iterateError);
		}
		if (existing.size() <= MAX_BACKUPS)
			return;
		sort(existing.begin(), existing.end());
		for (size_t index = 0; index + MAX_BACKUPS < existing.size(); ++index)
		{
			error_code removeError;
			filesystem::remove(existing[index], removeError);
		}
	}

	const DATA_JSON_VALUE* Find_Member(
		const DATA_JSON_VALUE& object,
		const string_view key,
		const DATA_JSON_TYPE type)
	{
		const DATA_JSON_VALUE* pValue = object.Find(key);
		return nullptr != pValue && pValue->Get_Type() == type
			? pValue : nullptr;
	}

	bool_t Read_Number(
		const DATA_JSON_VALUE& object,
		const string_view key,
		f32_t& outValue,
		const f32_t minimum,
		const f32_t maximum)
	{
		const DATA_JSON_VALUE* pValue = Find_Member(
			object, key, DATA_JSON_TYPE::NUMBER);
		if (nullptr == pValue || !isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() < minimum ||
			pValue->Get_Number() > maximum)
		{
			return false;
		}
		outValue = static_cast<f32_t>(pValue->Get_Number());
		return true;
	}

	bool_t Read_Integer(
		const DATA_JSON_VALUE& object,
		const string_view key,
		int32_t& outValue,
		const int32_t minimum,
		const int32_t maximum)
	{
		const DATA_JSON_VALUE* pValue = Find_Member(
			object, key, DATA_JSON_TYPE::NUMBER);
		if (nullptr == pValue || !isfinite(pValue->Get_Number()))
			return false;
		const double number = pValue->Get_Number();
		if (number != floor(number) || number < minimum || number > maximum)
			return false;
		outValue = static_cast<int32_t>(number);
		return true;
	}

	bool_t Read_Boolean(
		const DATA_JSON_VALUE& object,
		const string_view key,
		bool_t& outValue)
	{
		const DATA_JSON_VALUE* pValue = Find_Member(
			object, key, DATA_JSON_TYPE::BOOLEAN);
		if (nullptr == pValue)
			return false;
		outValue = pValue->Get_Boolean();
		return true;
	}

	void Write_String(ostream& output, const string_view value)
	{
		output << '"' << CDataJson::Escape(value) << '"';
	}

	void Get_Rotated_Rect_Corners(const ImVec2& vTopLeft, const ImVec2& vBotRight, float fDegrees, ImVec2 outCorners[4])
	{
		outCorners[0] = vTopLeft;
		outCorners[1] = ImVec2(vBotRight.x, vTopLeft.y);
		outCorners[2] = vBotRight;
		outCorners[3] = ImVec2(vTopLeft.x, vBotRight.y);

		if (0.f == fDegrees)
			return;

		const ImVec2 vCenter((vTopLeft.x + vBotRight.x) * 0.5f, (vTopLeft.y + vBotRight.y) * 0.5f);
		const float fRadians = fDegrees * (3.14159265f / 180.f);
		const float fCos = cosf(fRadians);
		const float fSin = sinf(fRadians);

		for (int32_t i = 0; i < 4; ++i)
		{
			const float fDX = outCorners[i].x - vCenter.x;
			const float fDY = outCorners[i].y - vCenter.y;
			outCorners[i].x = vCenter.x + fDX * fCos - fDY * fSin;
			outCorners[i].y = vCenter.y + fDX * fSin + fDY * fCos;
		}
	}
}

Client::CHUDLayoutTool::CHUDLayoutTool(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
	/* CreateWICTextureFromFile needs COM on the calling thread; the main thread never initializes it (only the level-loading worker thread does). */
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	/* Glow/particle art is authored for additive blending (see TEXTURE_LAYER::bAdditive); this is the
	   blend state Draw_Image_Quad switches to for layers that opt in. */
	D3D11_BLEND_DESC BlendDesc{};
	BlendDesc.RenderTarget[0].BlendEnable = TRUE;
	BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_pDevice->CreateBlendState(&BlendDesc, &m_pAdditiveBlendState);

	Load(Current_Save_Path());

	if (m_Slots.empty())
		Reset_Default();
}

void Client::CHUDLayoutTool::Enable_Additive_Blend(const ImDrawList* pParentList, const ImDrawCmd* pCmd)
{
	auto pTool = static_cast<CHUDLayoutTool*>(pCmd->UserCallbackData);
	pTool->m_pContext->OMSetBlendState(pTool->m_pAdditiveBlendState.Get(), nullptr, 0xffffffff);
}

void Client::CHUDLayoutTool::Draw_Image_Quad(ImDrawList* pDrawList, ID3D11ShaderResourceView* pSRV,
	const ImVec2 Corners[4], uint32_t iTint, bool_t bAdditive, bool_t bFlipX)
{
	if (bAdditive)
		pDrawList->AddCallback(&CHUDLayoutTool::Enable_Additive_Blend, this);

	/* Flipping is just swapping which U coordinate each corner samples; the on-screen quad and
	   winding stay the same, only the texture reads mirrored. */
	if (bFlipX)
		pDrawList->AddImageQuad(pSRV, Corners[0], Corners[1], Corners[2], Corners[3],
			ImVec2(1, 0), ImVec2(0, 0), ImVec2(0, 1), ImVec2(1, 1), iTint);
	else
		pDrawList->AddImageQuad(pSRV, Corners[0], Corners[1], Corners[2], Corners[3],
			ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1), iTint);

	if (bAdditive)
		pDrawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
}

filesystem::path Client::CHUDLayoutTool::Current_Save_Path() const
{
	return CProjectDataRoot::Resolve(
		g_Documents[m_iActiveDocument].szDataPath);
}

void Client::CHUDLayoutTool::Switch_Document(int32_t iDocument)
{
	if (iDocument < 0 || iDocument >= g_iDocumentCount)
		return;
	if (iDocument == m_iActiveDocument)
		return;

	/* Persist the document we are leaving so unsaved placement isn't lost on switch -- but only if
	it actually loaded successfully. If it never loaded (JSON contract rejected, unknown class,
	etc.), what's on screen is a placeholder, not the real document; saving it would silently
	destroy the file this session never actually opened. */
	if (m_bActiveDocumentLoaded)
		Save(Current_Save_Path());

	m_iActiveDocument = iDocument;

	/* Start from an empty document; Load() fills it if the target file exists. */
	m_Slots.clear();
	m_ClassNames = vector<string>{ "Default" };
	m_iSelectedClass = 0;
	m_iSelectedSlot = -1;
	m_SelectedSlots.clear();
	m_iLastScannedClass = -1;

	Load(Current_Save_Path());
}

void Client::CHUDLayoutTool::Render()
{
	/* Wide enough for palette + canvas viewport + inspector side by side. */
	ImGui::SetNextWindowSize(ImVec2(1450.f, 700.f), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("LostArk HUD Layout Tool"))
	{
		ImGui::End();
		return;
	}

	ImGui::TextUnformatted("F1: open / close the editor tool workspace   |   drag box body to move, drag yellow corner to resize.");
	ImGui::TextUnformatted("Drag this window onto Map Tool to dock as a tab.");
#ifdef _DEBUG
	if (ImGui::Button("Complete Play (Server/Arena)##HUDTool"))
	{
		if (CMainApp* const app = CMainApp::Get_Active())
			(void)app->Debug_CompletePlaySelected(m_strCompletePlayStatus);
		else
			m_strCompletePlayStatus = "Complete Play workspace is unavailable.";
	}
	ImGui::SameLine();
	ImGui::TextDisabled("%s", m_strCompletePlayStatus.c_str());
#endif

	if (ImGui::BeginTabBar("HUDDocuments"))
	{
		for (int32_t i = 0; i < g_iDocumentCount; ++i)
		{
			if (ImGui::BeginTabItem(g_Documents[i].szLabel))
			{
				if (i != m_iActiveDocument)
					Switch_Document(i);
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}

	if (ImGui::Button("Save"))
		Save(Current_Save_Path());
	ImGui::SameLine();
	if (ImGui::Button("Load"))
		Load(Current_Save_Path());
	ImGui::SameLine();
	ImGui::TextDisabled("%s", m_strDataStatus.c_str());
	ImGui::SameLine();
	if (g_Documents[m_iActiveDocument].bPerClass && ImGui::Button("Reset to Default Layout"))
		Reset_Default();

	/* Debug-only iteration aid for the "Boss UI" document -- Get_Boss().isValid otherwise only
	goes true from a real Server snapshot (live Valtan encounter), so without this, placing these
	slots means walking all the way to Valtan every time just to see the result. Never touches
	Server truth; only affects the local HUD_BOSS_STATE the boss bar reads from. */
	if (0 == std::strcmp(g_Documents[m_iActiveDocument].szLabel, "Boss UI"))
	{
		if (ImGui::Checkbox("Preview Sample Boss Data (Debug)", &m_bPreviewBossData))
			CCombatHUDViewModel::Get().Debug_Set_Boss_Preview(m_bPreviewBossData);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Fakes a Valtan-shaped HUD_BOSS_STATE locally so the boss bar renders here without a live encounter.\nTurn off before testing real gameplay data.");
	}

	/* Debug-only iteration aid for the "Esther UI" document -- RenderEstherGauge() otherwise only
	draws once the maximum is nonzero and the player is valid (a real Server gauge/snapshot), so
	without this, placing the fill means walking into Valtan Arena as Sillian every time just to
	see it. Never touches Server truth; only affects the local player/gauge state the Esther bar
	reads from. */
	if (0 == std::strcmp(g_Documents[m_iActiveDocument].szLabel, "Esther UI"))
	{
		if (ImGui::Checkbox("Preview Sample Gauge Data (Debug)", &m_bPreviewEstherData))
			CCombatHUDViewModel::Get().Debug_Set_Esther_Preview(m_bPreviewEstherData);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Fakes a full Esther gauge locally so the fill/label/ready-glow render here without a live Server gauge.\nTurn off before testing real gameplay data.");
	}

	ImGui::SameLine();
	ImGui::Checkbox("Preview Hover (all)", &m_bPreviewHover);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Forces every slot to its hover art.\nWithout this, mousing over a slot on the canvas previews just that one.");
	ImGui::SameLine();
	ImGui::Checkbox("Boost Dark Art", &m_bBoostDarkArt);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Editor-only: layers under a dark background lose their edges when aligning by eye.\nThis redraws them with extra additive passes just for visibility here; it changes nothing saved to the cfg or seen in-game.");
	ImGui::SameLine();
	ImGui::Checkbox("Show Empty Slots", &m_bShowEmptySlots);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Editor-only: outlines and names the slots that draw nothing -- text boxes, whose string the runtime draws at the slot rect, and any slot whose layers are fully transparent.\nWithout this they are invisible on the canvas and cannot be picked up.");

	/* One control drives every slot, so a charge state can be judged as a whole. */
	if (ImGui::Button("Animation Stage"))
		m_iPreviewStage = (m_iPreviewStage + 1) % (ms_iMaxStage + 1);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Click to cycle 0 -> 1 -> 2 -> 0.\nEach slot sets which stage its base and lit art appear at.");
	ImGui::SameLine();
	ImGui::Text("%d / %d   (0 = empty, higher = more charged)", m_iPreviewStage, ms_iMaxStage);

	ImGui::SetNextItemWidth(160.f);
	ImGui::SliderFloat("Zoom", &m_fCanvasScale, ms_fMinZoom, ms_fMaxZoom, "%.2fx");
	ImGui::SameLine();
	if (ImGui::Button("100%"))
		m_fCanvasScale = 1.f;
	ImGui::SameLine();
	if (ImGui::Button("Fit"))
		m_fCanvasScale = ms_fViewportWidth / ms_fRefWidth;
	ImGui::SameLine();
	ImGui::TextDisabled("(ctrl+wheel to zoom at cursor, middle-drag to pan)");

	if (g_Documents[m_iActiveDocument].bPerClass)
		Render_ClassBar();
	ImGui::Separator();

	Render_Palette();
	ImGui::SameLine();
	Render_Canvas();
	ImGui::SameLine();

	Render_Inspector();

	ImGui::End();
}

bool_t Client::CHUDLayoutTool::Select_Class(const string& classId)
{
	const auto found = find(m_ClassNames.begin(), m_ClassNames.end(), classId);
	if (m_ClassNames.end() != found)
	{
		const int32_t selected = static_cast<int32_t>(
			distance(m_ClassNames.begin(), found));
		if (m_iSelectedClass != selected)
		{
			m_iSelectedClass = selected;
			m_iLastScannedClass = -1;
		}
		return true;
	}

	const auto defaultClass = find(
		m_ClassNames.begin(), m_ClassNames.end(), "Default");
	if (m_ClassNames.end() != defaultClass)
	{
		m_iSelectedClass = static_cast<int32_t>(
			distance(m_ClassNames.begin(), defaultClass));
		m_iLastScannedClass = -1;
	}
	return false;
}

void Client::CHUDLayoutTool::Render_RuntimePreview(const string& classId)
{
	Select_Class(classId);
	if (m_Slots.empty() || m_ClassNames.empty())
		return;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	if (nullptr == viewport || viewport->WorkSize.x <= 0.f ||
		viewport->WorkSize.y <= 0.f)
	{
		return;
	}

	/* Foreground, not background: the combat HUD ImGui stat panel (semi-transparent black) sits at
	SetNextWindowBgAlpha(0.78f) over roughly the same screen region, and a background draw list
	renders underneath every window -- that panel's backdrop was painting over this preview and
	reading as "everything is too dark" even though the source art itself is at full brightness. */
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(viewport);
	const float scaleX = viewport->WorkSize.x / ms_fRefWidth;
	const float scaleY = viewport->WorkSize.y / ms_fRefHeight;
	/* Was hardcoded to 1, forcing lit/shine art on for every preview regardless of the tool's own
	Animation Stage toggle -- share that toggle instead so this defaults to 0 (off) like the real
	runtime view, and only shows charged art when the user actually steps the stage up. */
	const int32_t previewStage = m_iPreviewStage;

	for (HUD_SLOT& slot : m_Slots)
	{
		if (!Is_Slot_Visible(slot))
			continue;

		const ImVec2 topLeft(
			viewport->WorkPos.x + slot.fX * scaleX,
			viewport->WorkPos.y + slot.fY * scaleY);
		const ImVec2 bottomRight(
			topLeft.x + slot.fSizeX * scaleX,
			topLeft.y + slot.fSizeY * scaleY);
		ImVec2 corners[4];
		Get_Rotated_Rect_Corners(
			topLeft, bottomRight, slot.fRotation, corners);

		if (previewStage >= slot.iShineFromStage &&
			!slot.AnimationFrames.empty())
		{
			const int32_t frameCount = static_cast<int32_t>(
				slot.AnimationFrames.size());
			const int32_t frameIndex = frameCount <= 1 ? 0 :
				static_cast<int32_t>(
					ImGui::GetTime() * slot.fAnimationFPS) % frameCount;
			ID3D11ShaderResourceView* pFrame = Get_Or_Load_Texture(
				slot.AnimationFrames[frameIndex]);
			if (nullptr != pFrame)
			{
				const ImVec2 center(

					(topLeft.x + bottomRight.x) * 0.5f +
						slot.fAnimationOffsetX * scaleX,
					(topLeft.y + bottomRight.y) * 0.5f +
						slot.fAnimationOffsetY * scaleY);
				const float halfWidth =
					(bottomRight.x - topLeft.x) * 0.5f *
					slot.fAnimationScale;
				const float halfHeight =
					(bottomRight.y - topLeft.y) * 0.5f *
					slot.fAnimationScale;
				ImVec2 animationCorners[4];
				Get_Rotated_Rect_Corners(
					ImVec2(center.x - halfWidth, center.y - halfHeight),
					ImVec2(center.x + halfWidth, center.y + halfHeight),
					slot.fRotation,
					animationCorners);
				pDrawList->AddImageQuad(
					pFrame,
					animationCorners[0], animationCorners[1],
					animationCorners[2], animationCorners[3]);
			}
		}

		if (previewStage >= slot.iBaseFromStage)
		{
			for (const TEXTURE_LAYER& layer : slot.TextureLayers)
			{
				ID3D11ShaderResourceView* pTexture =
					Get_Or_Load_Texture(layer.strPath);
				if (nullptr == pTexture)
					continue;
				const ImU32 tint = ImGui::ColorConvertFloat4ToU32(
					ImVec4(
						layer.vTint[0], layer.vTint[1],
						layer.vTint[2], layer.vTint[3]));
				Draw_Image_Quad(
					pDrawList, pTexture, corners, tint,
					layer.bAdditive, layer.bFlipX);
			}
		}

		if (previewStage >= slot.iShineFromStage &&
			!slot.strShineTexture.empty())
		{
			ID3D11ShaderResourceView* pShine =
				Get_Or_Load_Texture(slot.strShineTexture);
			if (nullptr != pShine)
			{
				Draw_Image_Quad(
					pDrawList, pShine, corners,
					IM_COL32(255, 255, 255, 255),
					slot.bShineAdditive);
			}
		}
	}
}

void Client::CHUDLayoutTool::Render_ClassBar()
{
	ImGui::SeparatorText("Character Class");

	if (!m_ClassNames.empty())
	{
		if (m_iSelectedClass >= static_cast<int32_t>(m_ClassNames.size()))
			m_iSelectedClass = 0;

		if (ImGui::BeginCombo("Current Class", m_ClassNames[m_iSelectedClass].c_str()))
		{
			for (int32_t i = 0; i < static_cast<int32_t>(m_ClassNames.size()); ++i)
			{
				const bool_t bSelected = (i == m_iSelectedClass);
				if (ImGui::Selectable(m_ClassNames[i].c_str(), bSelected))
					m_iSelectedClass = i;
			}
			ImGui::EndCombo();
		}

		ImGui::SameLine();
		if (ImGui::Button("Delete Class") && m_ClassNames.size() > 1)
			Delete_Class(m_iSelectedClass);
	}

	ImGui::SetNextItemWidth(160.f);
	ImGui::InputText("##NewClassName", m_szNewClassName, sizeof(m_szNewClassName));
	ImGui::SameLine();
	if (ImGui::Button("Add Class"))
		Add_Class(m_szNewClassName);
}

void Client::CHUDLayoutTool::Render_Palette()
{
	ImGui::BeginChild("Palette", ImVec2(190.f, ms_fViewportHeight), true);

	ImGui::SeparatorText("Slot Palette");
	ImGui::TextWrapped("Drag a slot onto the canvas to place it.");
	ImGui::Spacing();

	for (const PALETTE_ENTRY& Entry : g_PaletteEntries)
	{
		ImGui::PushID(static_cast<int32_t>(Entry.eType));

		const ImVec2 vBoxSize(122.f, 32.f);
		ImGui::InvisibleButton(Entry.szLabel, vBoxSize);

		const ImVec2 vMin = ImGui::GetItemRectMin();
		const ImVec2 vMax = ImGui::GetItemRectMax();
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();

		const ImU32 iFillColor = Entry.bPerClass
			? IM_COL32(120, 70, 170, 160)
			: IM_COL32(60, 130, 190, 150);

		pDrawList->AddRectFilled(vMin, vMax, iFillColor);
		pDrawList->AddRect(vMin, vMax, IM_COL32(200, 200, 210, 200));
		pDrawList->AddText(ImVec2(vMin.x + 4.f, vMin.y + 8.f), IM_COL32(255, 255, 255, 255), Entry.szLabel);

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			const int32_t iType = static_cast<int32_t>(Entry.eType);
			ImGui::SetDragDropPayload("HUD_PALETTE_SLOT", &iType, sizeof(iType));
			ImGui::Text("%s", Entry.szLabel);
			ImGui::EndDragDropSource();
		}

		ImGui::PopID();
		ImGui::Spacing();
	}

	Render_ClassTexturePalette();

	ImGui::EndChild();
}

void Client::CHUDLayoutTool::Render_ClassTexturePalette()
{
	if (m_iSelectedClass != m_iLastScannedClass)
	{
		Refresh_ClassTexturePalette();
		m_iLastScannedClass = m_iSelectedClass;
	}

	const bool_t bPerClass = g_Documents[m_iActiveDocument].bPerClass;

	ImGui::SeparatorText(bPerClass ? ("Textures: " + Current_Class()).c_str() : "Textures");
	ImGui::TextWrapped("Drag a thumbnail onto a placed slot to add it as a layer.");

	if (m_TextureAssetPaths.empty())
	{
		ImGui::TextWrapped("(no images found for this document)");
		return;
	}

	for (int32_t i = 0; i < static_cast<int32_t>(m_TextureAssetPaths.size()); ++i)
	{
		const string& strPath = m_TextureAssetPaths[i];
		const string strFileName = Path_To_Utf8(filesystem::path(Utf8_To_Wide(strPath)).filename());

		ImGui::PushID(i);

		ID3D11ShaderResourceView* pSRV = Get_Or_Load_Texture(strPath);

		const ImVec2 vThumbSize(40.f, 40.f);
		if (nullptr != pSRV)
			ImGui::Image(pSRV, vThumbSize);
		else
			ImGui::Dummy(vThumbSize);

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			ImGui::SetDragDropPayload("HUD_TEXTURE_ASSET", &i, sizeof(i));
			if (nullptr != pSRV)
				ImGui::Image(pSRV, vThumbSize);
			ImGui::TextUnformatted(strFileName.c_str());
			ImGui::EndDragDropSource();
		}

		ImGui::SameLine();
		ImGui::TextWrapped("%s", strFileName.c_str());

		ImGui::PopID();
	}
}

void Client::CHUDLayoutTool::Refresh_ClassTexturePalette()
{
	m_TextureAssetPaths.clear();

	const DOCUMENT_DEF& Document = g_Documents[m_iActiveDocument];

	/* Per-class documents browse just the current class's folder; global ones browse the whole
	   document tree, since their art is grouped by purpose (QuickMenu, Top Menu, Currency) not by class. */
	string strFolder = Document.szTextureRoot;
	if (Document.bPerClass)
	{
		const string& strClass = Current_Class();
		if (strClass.empty())
			return;

		strFolder += strClass;
	}

	error_code ec;
	const u8string folderUtf8(strFolder.begin(), strFolder.end());
	const filesystem::path Folder = CRuntimeAssetRoot::Resolve(
		filesystem::path(folderUtf8));
	if (Folder.empty())
		return;
	const filesystem::path ResourceRoot = CRuntimeAssetRoot::Get();

	auto Fn_Collect = [this, &ResourceRoot](
		const filesystem::directory_entry& Entry)
	{
		if (!Entry.is_regular_file())
			return;

		wstring strExt = Entry.path().extension().wstring();
		for (wchar_t& ch : strExt)
			ch = static_cast<wchar_t>(towlower(ch));

		if (L".png" != strExt && L".dds" != strExt && L".jpg" != strExt)
			return;

		error_code relativeError;
		const filesystem::path relativePath = filesystem::relative(
			Entry.path(), ResourceRoot, relativeError);
		if (!relativeError && !relativePath.empty() &&
			relativePath.native().find(L"..") != 0)
		{
			m_TextureAssetPaths.push_back(
				Path_To_Utf8(relativePath.generic_wstring()));
		}
	};

	if (Document.bPerClass)
	{
		for (const filesystem::directory_entry& Entry : filesystem::directory_iterator(Folder, ec))
			Fn_Collect(Entry);
	}
	else
	{
		for (const filesystem::directory_entry& Entry : filesystem::recursive_directory_iterator(Folder, ec))
			Fn_Collect(Entry);
	}
}

void Client::CHUDLayoutTool::Render_Canvas()
{
	/* Fixed viewport with scrollbars: the zoomed canvas can exceed it and is panned instead of
	   forcing the whole tool window to grow. */
	ImGui::BeginChild("HUDCanvas", ImVec2(ms_fViewportWidth, ms_fViewportHeight),
		true, ImGuiWindowFlags_HorizontalScrollbar);

	const ImVec2 vOrigin = ImGui::GetCursorScreenPos();
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();

	/* Hit-testing happens after drawing, so this frame's art uses last frame's hover result. */
	const int32_t iHoveredLastFrame = m_iHoveredSlot;
	m_iHoveredSlot = -1;

	/* Ctrl + wheel zooms about the cursor: the content point under the mouse stays put. */
	if (ImGui::IsWindowHovered() && ImGui::GetIO().KeyCtrl && 0.f != ImGui::GetIO().MouseWheel)
	{
		const ImVec2 vMouse = ImGui::GetMousePos();
		const float fContentX = (vMouse.x - vOrigin.x) / m_fCanvasScale;
		const float fContentY = (vMouse.y - vOrigin.y) / m_fCanvasScale;

		const float fOldScale = m_fCanvasScale;
		m_fCanvasScale *= (1.f + ImGui::GetIO().MouseWheel * 0.12f);
		if (m_fCanvasScale < ms_fMinZoom)
			m_fCanvasScale = ms_fMinZoom;
		if (m_fCanvasScale > ms_fMaxZoom)
			m_fCanvasScale = ms_fMaxZoom;

		const float fScaleDelta = m_fCanvasScale - fOldScale;
		ImGui::SetScrollX(ImGui::GetScrollX() + fContentX * fScaleDelta);
		ImGui::SetScrollY(ImGui::GetScrollY() + fContentY * fScaleDelta);
	}

	/* Middle-drag pans, which beats chasing scrollbars while zoomed in. */
	if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
	{
		const ImVec2 vDelta = ImGui::GetIO().MouseDelta;
		ImGui::SetScrollX(ImGui::GetScrollX() - vDelta.x);
		ImGui::SetScrollY(ImGui::GetScrollY() - vDelta.y);
	}

	const ImVec2 vCanvasEnd(vOrigin.x + ms_fRefWidth * m_fCanvasScale, vOrigin.y + ms_fRefHeight * m_fCanvasScale);

	/* A flat mid-gray fill let dark class art (near-black emblems, orbs) blend into the
	   background and lose contrast. A checkerboard reads as "transparent" the same way
	   external atlas viewers do, so dark art keeps its edges visible against it. */
	const float fCheckerSize = 32.f * m_fCanvasScale;
	const ImU32 iCheckerLight = IM_COL32(150, 150, 156, 255);
	const ImU32 iCheckerDark = IM_COL32(120, 120, 126, 255);
	int32_t iRow = 0;
	for (float fY = vOrigin.y; fY < vCanvasEnd.y; fY += fCheckerSize, ++iRow)
	{
		int32_t iCol = 0;
		for (float fX = vOrigin.x; fX < vCanvasEnd.x; fX += fCheckerSize, ++iCol)
		{
			const bool_t bLight = (0 == ((iRow + iCol) % 2));
			const ImVec2 vCellMax((min)(fX + fCheckerSize, vCanvasEnd.x), (min)(fY + fCheckerSize, vCanvasEnd.y));
			pDrawList->AddRectFilled(ImVec2(fX, fY), vCellMax, bLight ? iCheckerLight : iCheckerDark);
		}
	}
	pDrawList->AddRect(vOrigin, vCanvasEnd, IM_COL32(200, 200, 205, 255));

	ImGui::SetCursorScreenPos(vOrigin);
	ImGui::SetNextItemAllowOverlap();
	ImGui::InvisibleButton("CanvasBackground", ImVec2(vCanvasEnd.x - vOrigin.x, vCanvasEnd.y - vOrigin.y));

	if (ImGui::IsItemActivated())
	{
		m_SelectedSlots.clear();
		m_iSelectedSlot = -1;
		m_bMarqueeActive = true;
		const ImVec2 vMouse = ImGui::GetMousePos();
		m_fMarqueeStartX = vMouse.x;
		m_fMarqueeStartY = vMouse.y;
	}

	if (m_bMarqueeActive && ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		const ImVec2 vMouse = ImGui::GetMousePos();
		const ImVec2 vMin((min)(m_fMarqueeStartX, vMouse.x), (min)(m_fMarqueeStartY, vMouse.y));
		const ImVec2 vMax((max)(m_fMarqueeStartX, vMouse.x), (max)(m_fMarqueeStartY, vMouse.y));
		pDrawList->AddRectFilled(vMin, vMax, IM_COL32(90, 160, 255, 60));
		pDrawList->AddRect(vMin, vMax, IM_COL32(90, 160, 255, 200));
	}

	if (m_bMarqueeActive && ImGui::IsItemDeactivated())
	{
		const ImVec2 vMouse = ImGui::GetMousePos();
		const ImVec2 vMin((min)(m_fMarqueeStartX, vMouse.x), (min)(m_fMarqueeStartY, vMouse.y));
		const ImVec2 vMax((max)(m_fMarqueeStartX, vMouse.x), (max)(m_fMarqueeStartY, vMouse.y));

		if (fabsf(vMouse.x - m_fMarqueeStartX) > 3.f || fabsf(vMouse.y - m_fMarqueeStartY) > 3.f)
		{
			for (int32_t i = 0; i < static_cast<int32_t>(m_Slots.size()); ++i)
			{
				const HUD_SLOT& Slot = m_Slots[i];
				if (!Is_Slot_Visible(Slot))
					continue;

				const ImVec2 vSlotMin(vOrigin.x + Slot.fX * m_fCanvasScale, vOrigin.y + Slot.fY * m_fCanvasScale);
				const ImVec2 vSlotMax(vSlotMin.x + Slot.fSizeX * m_fCanvasScale, vSlotMin.y + Slot.fSizeY * m_fCanvasScale);

				const bool_t bOverlaps = (vSlotMin.x <= vMax.x && vSlotMax.x >= vMin.x &&
					vSlotMin.y <= vMax.y && vSlotMax.y >= vMin.y);

				if (bOverlaps)
					m_SelectedSlots.push_back(i);
			}

			m_iSelectedSlot = m_SelectedSlots.empty() ? -1 : m_SelectedSlots.back();
		}

		m_bMarqueeActive = false;
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("HUD_PALETTE_SLOT"))
		{
			const int32_t iType = *static_cast<const int32_t*>(pPayload->Data);
			const ImVec2 vMouse = ImGui::GetMousePos();

			const float fDropX = (vMouse.x - vOrigin.x) / m_fCanvasScale;
			const float fDropY = (vMouse.y - vOrigin.y) / m_fCanvasScale;

			Add_Slot_From_Palette(static_cast<SLOT_TYPE>(iType), fDropX, fDropY);
		}

		ImGui::EndDragDropTarget();
	}

	/* Visual pass: back-to-front (array order), draws images/labels only. */
	for (int32_t i = 0; i < static_cast<int32_t>(m_Slots.size()); ++i)
	{
		HUD_SLOT& Slot = m_Slots[i];
		if (!Is_Slot_Visible(Slot))
			continue;

		const ImVec2 vTopLeft(vOrigin.x + Slot.fX * m_fCanvasScale, vOrigin.y + Slot.fY * m_fCanvasScale);
		const ImVec2 vBotRight(vTopLeft.x + Slot.fSizeX * m_fCanvasScale, vTopLeft.y + Slot.fSizeY * m_fCanvasScale);

		ImVec2 Corners[4];
		Get_Rotated_Rect_Corners(vTopLeft, vBotRight, Slot.fRotation, Corners);

		const bool_t bSelected = Is_Slot_Selected(i);
		const ImU32 iBorderColor = IM_COL32(255, 220, 90, 255);

		bool_t bAnyLayerDrawn = false;

		const bool_t bBaseVisible = (m_iPreviewStage >= Slot.iBaseFromStage);
		const bool_t bLitVisible = (m_iPreviewStage >= Slot.iShineFromStage);

		/* Fire animation renders first (furthest back), enlarged, only once the slot is lit. */
		if (bLitVisible)
		{
			if (!Slot.AnimationFrames.empty())
			{
				const vector<string>& Frames = Slot.AnimationFrames;
				const int32_t iFrameCount = static_cast<int32_t>(Frames.size());

				/* Continuous ping-pong position in [0, N-1], then cross-fade the two bracketing frames.
				   With only a few source frames a hard cut reads as a strobe/flicker; the dissolve makes it flow.
				   Only for looping slots (torch/fire-style flicker effects this preview was built for) --
				   a one-shot burst (loop=false, e.g. a success-flash flipbook never meant to actually loop)
				   freezes on a single middle frame instead, since animating it while positioning things
				   is misleading (CHUDRuntimeView plays it once and stops; this canvas isn't "playing" at all). */
				int32_t iFrameA = iFrameCount / 2;
				int32_t iFrameB = iFrameA;
				float fBlend = 0.f;
				if (Slot.bAnimationLoop && iFrameCount > 1)
				{
					const float fPeriod = 2.f * (iFrameCount - 1);
					float fCycle = fmodf(static_cast<float>(ImGui::GetTime()) * Slot.fAnimationFPS, fPeriod);
					if (fCycle < 0.f)
						fCycle += fPeriod;
					const float fPos = (fCycle <= iFrameCount - 1) ? fCycle : (fPeriod - fCycle);

					iFrameA = static_cast<int32_t>(fPos);
					iFrameB = (iFrameA + 1 < iFrameCount) ? iFrameA + 1 : iFrameA;
					fBlend = fPos - iFrameA;
				}

				const ImVec2 vSlotCenter((vTopLeft.x + vBotRight.x) * 0.5f, (vTopLeft.y + vBotRight.y) * 0.5f);
				const ImVec2 vFireCenter(
					vSlotCenter.x + Slot.fAnimationOffsetX * m_fCanvasScale,
					vSlotCenter.y + Slot.fAnimationOffsetY * m_fCanvasScale);

				const float fHalfWidth = (vBotRight.x - vTopLeft.x) * 0.5f * Slot.fAnimationScale;
				const float fHalfHeight = (vBotRight.y - vTopLeft.y) * 0.5f * Slot.fAnimationScale;

				const ImVec2 vFireTopLeft(vFireCenter.x - fHalfWidth, vFireCenter.y - fHalfHeight);
				const ImVec2 vFireBotRight(vFireCenter.x + fHalfWidth, vFireCenter.y + fHalfHeight);

				ImVec2 FireCorners[4];
				Get_Rotated_Rect_Corners(vFireTopLeft, vFireBotRight, Slot.fRotation, FireCorners);

				/* Base frame at full opacity, next frame dissolved on top by the blend factor. Routed
				through Draw_Image_Quad (not a raw AddImageQuad) so Slot.bAnimationAdditive actually
				applies here -- without it, an additive flipbook (SmeltGlow/CoreFlash/ShockwaveRing/
				CompleteEffect, all baked on a near-black background meant to vanish under additive
				blending) previews as an opaque black square in the Tool even though the same slot
				renders correctly in-game via CHUDRuntimeView, which already respects this flag. */
				ID3D11ShaderResourceView* pFrameA = Get_Or_Load_Texture(Frames[iFrameA]);
				if (nullptr != pFrameA)
					Draw_Image_Quad(pDrawList, pFrameA, FireCorners, IM_COL32(255, 255, 255, 255),
						Slot.bAnimationAdditive, false);

				if (iFrameB != iFrameA && fBlend > 0.f)
				{
					ID3D11ShaderResourceView* pFrameB = Get_Or_Load_Texture(Frames[iFrameB]);
					if (nullptr != pFrameB)
					{
						const int32_t iAlpha = static_cast<int32_t>(fBlend * 255.f);
						Draw_Image_Quad(pDrawList, pFrameB, FireCorners, IM_COL32(255, 255, 255, iAlpha),
							Slot.bAnimationAdditive, false);
					}
				}
			}
		}

		const bool_t bShowHover = m_bPreviewHover || (i == iHoveredLastFrame);

		if (bBaseVisible)
		{
			for (const TEXTURE_LAYER& Layer : Slot.TextureLayers)
			{
				/* Layers without hover art (backgrounds, frames) keep their base image. */
				const string& strDrawPath = (bShowHover && !Layer.strHoverPath.empty())
					? Layer.strHoverPath : Layer.strPath;

				if (strDrawPath.empty())
					continue;

				ID3D11ShaderResourceView* pLayerSRV = Get_Or_Load_Texture(strDrawPath);
				if (nullptr == pLayerSRV)
					continue;

				const ImU32 iTint = ImGui::ColorConvertFloat4ToU32(
					ImVec4(Layer.vTint[0], Layer.vTint[1], Layer.vTint[2], Layer.vTint[3]));

				Draw_Image_Quad(pDrawList, pLayerSRV, Corners, iTint, Layer.bAdditive, Layer.bFlipX);
				/* A layer at zero alpha put nothing on screen, so it must not count as drawn -- that
				is exactly what a text-only slot's placeholder layer is, and counting it suppressed
				the empty-slot outline that makes such a slot selectable. */
				if (Layer.vTint[3] > (1.f / 255.f))
					bAnyLayerDrawn = true;

				/* Extra additive passes of the same art lift dark pixels toward visible without
				   touching the layer's own (correct) alpha-blended look; already-additive layers
				   are already at full visibility and skip this. */
				if (m_bBoostDarkArt && !Layer.bAdditive)
				{
					Draw_Image_Quad(pDrawList, pLayerSRV, Corners, iTint, true, Layer.bFlipX);
					Draw_Image_Quad(pDrawList, pLayerSRV, Corners, iTint, true, Layer.bFlipX);
				}
			}
		}

		/* Lit state layers its variant over the base (white orb -> blue orb, gauge -> shine). */
		if (bLitVisible && !Slot.strShineTexture.empty())
		{
			ID3D11ShaderResourceView* pShineSRV = Get_Or_Load_Texture(Slot.strShineTexture);
			if (nullptr != pShineSRV)
			{
				Draw_Image_Quad(pDrawList, pShineSRV, Corners, IM_COL32(255, 255, 255, 255), Slot.bShineAdditive);
				bAnyLayerDrawn = true;
			}
		}

		/* A slot with nothing to show at this stage gets a faint outline so it stays findable while
		   editing, but that marker is editor chrome: clicking the background clears the selection and
		   with it every outline, leaving a clean preview of the layout itself. Show Empty Slots keeps
		   them outlined and named with nothing selected, which is the only way to find and drag a
		   text-only slot -- turn it off for that clean preview. */
		if (!bAnyLayerDrawn && m_bShowEmptySlots)
		{
			pDrawList->AddQuad(Corners[0], Corners[1], Corners[2], Corners[3], IM_COL32(120, 220, 150, 190));
			if (!bSelected)
				pDrawList->AddText(ImVec2(Corners[0].x + 3.f, Corners[0].y + 2.f),
					IM_COL32(120, 220, 150, 220), Slot.strName.c_str());
		}
		else if (!bAnyLayerDrawn && !m_SelectedSlots.empty())
			pDrawList->AddQuad(Corners[0], Corners[1], Corners[2], Corners[3], IM_COL32(150, 150, 160, 80));

		if (bSelected)
		{
			pDrawList->AddQuad(Corners[0], Corners[1], Corners[2], Corners[3], iBorderColor);
			pDrawList->AddText(ImVec2(Corners[0].x + 3.f, Corners[0].y + 2.f), IM_COL32(255, 255, 255, 255), Slot.strName.c_str());
		}
	}

	/* Interaction pass: front-to-back (reverse array order) so the topmost slot claims the click first. */
	for (int32_t i = static_cast<int32_t>(m_Slots.size()) - 1; i >= 0; --i)
	{
		HUD_SLOT& Slot = m_Slots[i];
		if (!Is_Slot_Visible(Slot))
			continue;

		const ImVec2 vTopLeft(vOrigin.x + Slot.fX * m_fCanvasScale, vOrigin.y + Slot.fY * m_fCanvasScale);
		const ImVec2 vBotRight(vTopLeft.x + Slot.fSizeX * m_fCanvasScale, vTopLeft.y + Slot.fSizeY * m_fCanvasScale);

		const bool_t bSelected = Is_Slot_Selected(i);

		ImGui::PushID(i);

		ImGui::SetCursorScreenPos(vTopLeft);
		ImGui::InvisibleButton("body", ImVec2(vBotRight.x - vTopLeft.x, vBotRight.y - vTopLeft.y));
		/* Front-to-back pass, so the first hit is the topmost slot. */
		if (-1 == m_iHoveredSlot && ImGui::IsItemHovered())
			m_iHoveredSlot = i;
		if (ImGui::IsItemActivated())
		{
			if (ImGui::GetIO().KeyCtrl)
				Toggle_Slot_Selection(i);
			else if (!bSelected || m_SelectedSlots.size() <= 1)
				Select_Slot_Exclusive(i);
		}
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			ImVec2 vDelta = ImGui::GetIO().MouseDelta;

			if (ImGui::GetIO().KeyShift)
			{
				const ImVec2 vTotalDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
				if (fabsf(vTotalDelta.x) >= fabsf(vTotalDelta.y))
					vDelta.y = 0.f;
				else
					vDelta.x = 0.f;
			}

			if (Is_Slot_Selected(i) && m_SelectedSlots.size() > 1)
			{
				for (int32_t iSelected : m_SelectedSlots)
				{
					m_Slots[iSelected].fX += vDelta.x / m_fCanvasScale;
					m_Slots[iSelected].fY += vDelta.y / m_fCanvasScale;
				}
			}
			else
			{
				Slot.fX += vDelta.x / m_fCanvasScale;
				Slot.fY += vDelta.y / m_fCanvasScale;
			}
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("HUD_TEXTURE_ASSET"))
			{
				const int32_t iAssetIndex = *static_cast<const int32_t*>(pPayload->Data);
				if (iAssetIndex >= 0 && iAssetIndex < static_cast<int32_t>(m_TextureAssetPaths.size()))
				{
					TEXTURE_LAYER Layer{};
					Layer.strPath = m_TextureAssetPaths[iAssetIndex];

					/* A slot's first layer defines its footprint: size to the texture's actual
					pixel dimensions instead of leaving whatever generic palette-type default the
					slot was created with, so a dropped background/frame image is not silently
					stretched off its own aspect ratio. Later layers on the same slot (hover art,
					shine, ...) do not resize it again. */
					if (Slot.TextureLayers.empty())
					{
						if (ID3D11ShaderResourceView* pSRV = Get_Or_Load_Texture(Layer.strPath))
						{
							ComPtr<ID3D11Resource> pResource;
							pSRV->GetResource(&pResource);
							ComPtr<ID3D11Texture2D> pTexture2D;
							if (SUCCEEDED(pResource.As(&pTexture2D)))
							{
								D3D11_TEXTURE2D_DESC Desc{};
								pTexture2D->GetDesc(&Desc);
								Slot.fSizeX = static_cast<float>(Desc.Width);
								Slot.fSizeY = static_cast<float>(Desc.Height);
							}
						}
					}

					Slot.TextureLayers.push_back(move(Layer));
				}
			}

			ImGui::EndDragDropTarget();
		}

		if (bSelected && 1 == m_SelectedSlots.size())
		{
			/* Fixed screen-space size: at the tool's default 0.7x canvas zoom an 8-canvas-unit
			handle was only ~5-6 screen pixels, effectively unclickable. A hit target that stays
			constant on screen regardless of zoom is grabbable at any zoom level. */
			const float fHandle = 12.f;
			const ImVec2 vHandleTopLeft(vBotRight.x - fHandle, vBotRight.y - fHandle);
			ImGui::SetCursorScreenPos(vHandleTopLeft);
			ImGui::InvisibleButton("resize", ImVec2(fHandle, fHandle));
			const bool_t bHandleHot = ImGui::IsItemHovered() || ImGui::IsItemActive();
			pDrawList->AddRectFilled(vHandleTopLeft, vBotRight,
				bHandleHot ? IM_COL32(255, 240, 150, 255) : IM_COL32(255, 220, 90, 220));
			pDrawList->AddRect(vHandleTopLeft, vBotRight, IM_COL32(40, 30, 0, 255));
			if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				const ImVec2 vDelta = ImGui::GetIO().MouseDelta;
				Slot.fSizeX += vDelta.x / m_fCanvasScale;
				Slot.fSizeY += vDelta.y / m_fCanvasScale;
				if (Slot.fSizeX < 8.f)
					Slot.fSizeX = 8.f;
				if (Slot.fSizeY < 8.f)
					Slot.fSizeY = 8.f;
			}
		}

		ImGui::PopID();
	}

	ImGui::EndChild();
}

void Client::CHUDLayoutTool::Render_Inspector()
{
	ImGui::BeginChild("Inspector", ImVec2(300.f, ms_fViewportHeight), true);

	ImGui::SeparatorText("Slot List");
	{
		ImGui::BeginChild("SlotList", ImVec2(0.f, 180.f), true);
		for (int32_t i = 0; i < static_cast<int32_t>(m_Slots.size()); ++i)
		{
			if (!Is_Slot_Visible(m_Slots[i]))
				continue;

			ImGui::PushID(i);
			const bool_t bSelected = Is_Slot_Selected(i);
			if (ImGui::Selectable(m_Slots[i].strName.c_str(), bSelected))
				Select_Slot_Exclusive(i);
			ImGui::PopID();
		}
		ImGui::EndChild();
	}

	if (m_SelectedSlots.size() > 1)
		ImGui::TextWrapped("%d slots selected (drag any of them together; edit fields below apply to the last-clicked one).", static_cast<int32_t>(m_SelectedSlots.size()));

	ImGui::SetNextItemWidth(160.f);
	ImGui::InputText("##NewSlotName", m_szNewSlotName, sizeof(m_szNewSlotName));
	ImGui::SameLine();
	if (ImGui::Button("Add Slot"))
		Add_Slot(m_szNewSlotName);

	if (m_iSelectedSlot >= 0 && m_iSelectedSlot < static_cast<int32_t>(m_Slots.size()))
	{
		HUD_SLOT& Slot = m_Slots[m_iSelectedSlot];

		ImGui::SeparatorText("Selected Slot");

		ImGui::Text("Type: %s", SlotType_Label(Slot.eType));

		char szNameBuf[64] = {};
		strncpy_s(szNameBuf, Slot.strName.c_str(), sizeof(szNameBuf) - 1);
		if (ImGui::InputText("Name", szNameBuf, sizeof(szNameBuf)))
			Slot.strName = szNameBuf;

		ImGui::DragFloat("X", &Slot.fX, 1.f, 0.f, ms_fRefWidth);
		ImGui::DragFloat("Y", &Slot.fY, 1.f, 0.f, ms_fRefHeight);
		ImGui::DragFloat("Width", &Slot.fSizeX, 1.f, 4.f, ms_fRefWidth);
		ImGui::DragFloat("Height", &Slot.fSizeY, 1.f, 4.f, ms_fRefHeight);
		ImGui::DragFloat("Rotation", &Slot.fRotation, 1.f, -180.f, 180.f, "%.1f deg");

		ImGui::SeparatorText("Texture Layers (bottom to top)");
		for (int32_t iLayer = 0; iLayer < static_cast<int32_t>(Slot.TextureLayers.size()); ++iLayer)
		{
			TEXTURE_LAYER& Layer = Slot.TextureLayers[iLayer];

			ImGui::PushID(iLayer);

			/* Tint swatch first: monochrome mask icons are colored here rather than by swapping images. */
			ImGui::ColorEdit4("##LayerTint", Layer.vTint,
				ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreviewHalf);

			ImGui::SameLine();
			char szLayerBuf[260] = {};
			strncpy_s(szLayerBuf, Layer.strPath.c_str(), sizeof(szLayerBuf) - 1);
			ImGui::SetNextItemWidth(150.f);
			if (ImGui::InputText("##LayerPath", szLayerBuf, sizeof(szLayerBuf)))
				Layer.strPath = szLayerBuf;

			ImGui::SameLine();
			if (ImGui::SmallButton("Remove"))
			{
				Slot.TextureLayers.erase(Slot.TextureLayers.begin() + iLayer);
				ImGui::PopID();
				break;
			}

			/* Glow art (orbs, gauges, emblems) is authored for additive blending; alpha-blended it
			   just looks dim, since most of the sprite is low-alpha halo rather than solid color. */
			ImGui::Checkbox("Additive", &Layer.bAdditive);
			ImGui::SameLine();
			/* For a mirrored copy of directional art (an L-bracket, a wing) instead of a second file. */
			ImGui::Checkbox("Flip X", &Layer.bFlipX);

			/* Optional hover swap for this layer; left blank the layer stays put on hover. */
			char szHoverBuf[260] = {};
			strncpy_s(szHoverBuf, Layer.strHoverPath.c_str(), sizeof(szHoverBuf) - 1);
			ImGui::SetNextItemWidth(150.f);
			if (ImGui::InputText("hover", szHoverBuf, sizeof(szHoverBuf)))
				Layer.strHoverPath = szHoverBuf;

			ImGui::PopID();
		}

		ImGui::SetNextItemWidth(160.f);
		ImGui::InputText("##NewLayerPath", m_szNewLayerPathBuffer, sizeof(m_szNewLayerPathBuffer));
		ImGui::SameLine();
		if (ImGui::Button("Add Layer") && '\0' != m_szNewLayerPathBuffer[0])
		{
			TEXTURE_LAYER Layer{};
			Layer.strPath = m_szNewLayerPathBuffer;
			Slot.TextureLayers.push_back(move(Layer));
			m_szNewLayerPathBuffer[0] = '\0';
		}

		ImGui::SeparatorText("Charge Stages");
		ImGui::SliderInt("Base from stage", &Slot.iBaseFromStage, 0, ms_iMaxStage);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Stage at which this slot's base texture starts showing.\n0 = always visible.");
		ImGui::SliderInt("Lit from stage", &Slot.iShineFromStage, 0, ms_iMaxStage);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Stage at which the shine texture and animation frames kick in.");

		if (g_Documents[m_iActiveDocument].bPerClass)
		{
			/* Owner decides which class this slot belongs to; shared slots show for every class. */
			const char* szOwnerLabel = Slot.strOwnerClass.empty() ? "(shared by all classes)" : Slot.strOwnerClass.c_str();
			if (ImGui::BeginCombo("Owner", szOwnerLabel))
			{
				if (ImGui::Selectable("(shared by all classes)", Slot.strOwnerClass.empty()))
					Slot.strOwnerClass.clear();

				for (const string& strClass : m_ClassNames)
				{
					const bool_t bSelectedOwner = (Slot.strOwnerClass == strClass);
					if (ImGui::Selectable(strClass.c_str(), bSelectedOwner))
						Slot.strOwnerClass = strClass;
				}
				ImGui::EndCombo();
			}
		}

		char szShineBuf[260] = {};
		strncpy_s(szShineBuf, Slot.strShineTexture.c_str(), sizeof(szShineBuf) - 1);
		if (ImGui::InputText("Shine Texture (on-state)", szShineBuf, sizeof(szShineBuf)))
			Slot.strShineTexture = szShineBuf;
		ImGui::Checkbox("Shine Additive", &Slot.bShineAdditive);

		ImGui::DragFloat("Fire Animation Scale", &Slot.fAnimationScale, 0.01f, 0.5f, 3.f);
		ImGui::DragFloat("Fire Offset X", &Slot.fAnimationOffsetX, 0.5f, -100.f, 100.f);
		ImGui::DragFloat("Fire Offset Y", &Slot.fAnimationOffsetY, 0.5f, -100.f, 100.f);

		ImGui::SeparatorText("Animation Frames (plays behind, e.g. flickering flame)");

		vector<string>& Frames = Slot.AnimationFrames;
		for (int32_t iFrame = 0; iFrame < static_cast<int32_t>(Frames.size()); ++iFrame)
		{
			ImGui::PushID(iFrame);

			char szFrameBuf[260] = {};
			strncpy_s(szFrameBuf, Frames[iFrame].c_str(), sizeof(szFrameBuf) - 1);
			ImGui::SetNextItemWidth(200.f);
			if (ImGui::InputText("##FramePath", szFrameBuf, sizeof(szFrameBuf)))
				Frames[iFrame] = szFrameBuf;

			ImGui::SameLine();
			if (ImGui::SmallButton("Remove"))
			{
				Frames.erase(Frames.begin() + iFrame);
				ImGui::PopID();
				break;
			}
			ImGui::PopID();
		}

		ImGui::SetNextItemWidth(-1.f);
		ImGui::InputText("##NewAnimFramePath", m_szNewAnimFramePathBuffer, sizeof(m_szNewAnimFramePathBuffer));
		if (ImGui::Button("Add Frame") && '\0' != m_szNewAnimFramePathBuffer[0])
		{
			Frames.push_back(m_szNewAnimFramePathBuffer);
			m_szNewAnimFramePathBuffer[0] = '\0';
		}
		if (!Frames.empty())
		{
			ImGui::SameLine();
			ImGui::SetNextItemWidth(80.f);
			ImGui::DragFloat("FPS", &Slot.fAnimationFPS, 0.5f, 1.f, 60.f);
		}

		if (ImGui::Button("Move Backward"))
			Move_Slot_Backward(m_iSelectedSlot);
		ImGui::SameLine();
		if (ImGui::Button("Move Forward"))
			Move_Slot_Forward(m_iSelectedSlot);

		if (ImGui::Button("Duplicate Slot (same rect)"))
			Duplicate_Slot(m_iSelectedSlot);
		ImGui::SameLine();
		if (ImGui::Button("Delete Slot"))
		{
			Delete_Slot(m_iSelectedSlot);
			m_iSelectedSlot = -1;
			m_SelectedSlots.clear();
		}
	}

	ImGui::EndChild();
}

void Client::CHUDLayoutTool::Add_Slot(const string& strName)
{
	HUD_SLOT Slot{};
	Slot.strName = strName.empty() ? "New_Slot" : strName;
	Slot.fX = ms_fRefWidth * 0.5f - 20.f;
	Slot.fY = ms_fRefHeight * 0.5f - 20.f;
	Slot.fSizeX = 40.f;
	Slot.fSizeY = 40.f;

	m_Slots.push_back(move(Slot));
	m_iSelectedSlot = static_cast<int32_t>(m_Slots.size()) - 1;
}

void Client::CHUDLayoutTool::Add_Slot_From_Palette(SLOT_TYPE eType, float fDropCenterX, float fDropCenterY)
{
	const PALETTE_ENTRY* pEntry = Find_Palette_Entry(eType);
	if (nullptr == pEntry)
		return;

	HUD_SLOT Slot{};
	Slot.strName = Generate_Unique_Name(pEntry->szLabel, pEntry->bRepeatable);
	Slot.eType = eType;
	Slot.fSizeX = pEntry->fSizeX;
	Slot.fSizeY = pEntry->fSizeY;
	/* Class-specific palette entries (emblem, identity gauge) are born owned by the class being edited. */
	if (pEntry->bPerClass && g_Documents[m_iActiveDocument].bPerClass)
		Slot.strOwnerClass = Current_Class();
	Slot.fX = fDropCenterX - Slot.fSizeX * 0.5f;
	Slot.fY = fDropCenterY - Slot.fSizeY * 0.5f;

	m_Slots.push_back(move(Slot));
	m_iSelectedSlot = static_cast<int32_t>(m_Slots.size()) - 1;
}

string Client::CHUDLayoutTool::Generate_Unique_Name(const string& strBase, bool_t bRepeatable) const
{
	auto Fn_Exists = [this](const string& strName)
	{
		for (const HUD_SLOT& Slot : m_Slots)
			if (Slot.strName == strName)
				return true;
		return false;
	};

	if (!bRepeatable && !Fn_Exists(strBase))
		return strBase;

	int32_t iSuffix = 1;
	string strCandidate = strBase + "_" + to_string(iSuffix);
	while (Fn_Exists(strCandidate))
	{
		++iSuffix;
		strCandidate = strBase + "_" + to_string(iSuffix);
	}

	return strCandidate;
}

void Client::CHUDLayoutTool::Delete_Slot(int32_t iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<int32_t>(m_Slots.size()))
		return;

	m_Slots.erase(m_Slots.begin() + iIndex);
}

void Client::CHUDLayoutTool::Duplicate_Slot(int32_t iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<int32_t>(m_Slots.size()))
		return;

	HUD_SLOT Slot = m_Slots[iIndex];
	Slot.strName = Generate_Unique_Name(m_Slots[iIndex].strName, true);
	Slot.TextureLayers.clear();
	Slot.strShineTexture.clear();
	Slot.AnimationFrames.clear();

	m_Slots.push_back(move(Slot));
	m_iSelectedSlot = static_cast<int32_t>(m_Slots.size()) - 1;
}

void Client::CHUDLayoutTool::Move_Slot_Forward(int32_t iIndex)
{
	if (iIndex < 0 || iIndex + 1 >= static_cast<int32_t>(m_Slots.size()))
		return;

	swap(m_Slots[iIndex], m_Slots[iIndex + 1]);

	for (int32_t& iSelected : m_SelectedSlots)
	{
		if (iSelected == iIndex)
			iSelected = iIndex + 1;
		else if (iSelected == iIndex + 1)
			iSelected = iIndex;
	}

	if (m_iSelectedSlot == iIndex)
		m_iSelectedSlot = iIndex + 1;
	else if (m_iSelectedSlot == iIndex + 1)
		m_iSelectedSlot = iIndex;
}

void Client::CHUDLayoutTool::Move_Slot_Backward(int32_t iIndex)
{
	if (iIndex <= 0 || iIndex >= static_cast<int32_t>(m_Slots.size()))
		return;

	swap(m_Slots[iIndex], m_Slots[iIndex - 1]);

	for (int32_t& iSelected : m_SelectedSlots)
	{
		if (iSelected == iIndex)
			iSelected = iIndex - 1;
		else if (iSelected == iIndex - 1)
			iSelected = iIndex;
	}

	if (m_iSelectedSlot == iIndex)
		m_iSelectedSlot = iIndex - 1;
	else if (m_iSelectedSlot == iIndex - 1)
		m_iSelectedSlot = iIndex;
}

bool_t Client::CHUDLayoutTool::Is_Slot_Visible(const HUD_SLOT& Slot) const
{
	/* Documents without classes (Screen UI) show everything; otherwise shared + current class only. */
	if (!g_Documents[m_iActiveDocument].bPerClass)
		return true;

	return Slot.strOwnerClass.empty() || Slot.strOwnerClass == Current_Class();
}

bool_t Client::CHUDLayoutTool::Is_Slot_Selected(int32_t iIndex) const
{
	for (int32_t iSelected : m_SelectedSlots)
		if (iSelected == iIndex)
			return true;

	return false;
}

void Client::CHUDLayoutTool::Toggle_Slot_Selection(int32_t iIndex)
{
	for (auto Iter = m_SelectedSlots.begin(); Iter != m_SelectedSlots.end(); ++Iter)
	{
		if (*Iter == iIndex)
		{
			m_SelectedSlots.erase(Iter);
			m_iSelectedSlot = m_SelectedSlots.empty() ? -1 : m_SelectedSlots.back();
			return;
		}
	}

	m_SelectedSlots.push_back(iIndex);
	m_iSelectedSlot = iIndex;
}

void Client::CHUDLayoutTool::Select_Slot_Exclusive(int32_t iIndex)
{
	m_SelectedSlots.clear();
	m_SelectedSlots.push_back(iIndex);
	m_iSelectedSlot = iIndex;
}

void Client::CHUDLayoutTool::Add_Class(const string& strName)
{
	if (strName.empty())
		return;

	for (const string& strExisting : m_ClassNames)
		if (strExisting == strName)
			return;

	m_ClassNames.push_back(strName);
}

void Client::CHUDLayoutTool::Delete_Class(int32_t iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<int32_t>(m_ClassNames.size()))
		return;
	if (m_ClassNames.size() <= 1)
		return;

	const string strRemoved = m_ClassNames[iIndex];
	m_ClassNames.erase(m_ClassNames.begin() + iIndex);

	/* Slots owned by the removed class would become unreachable, so drop them with it. */
	for (auto Iter = m_Slots.begin(); Iter != m_Slots.end(); )
		Iter = (Iter->strOwnerClass == strRemoved) ? m_Slots.erase(Iter) : Iter + 1;

	m_iSelectedSlot = -1;
	m_SelectedSlots.clear();

	if (m_iSelectedClass >= static_cast<int32_t>(m_ClassNames.size()))
		m_iSelectedClass = static_cast<int32_t>(m_ClassNames.size()) - 1;
}

const string& Client::CHUDLayoutTool::Current_Class() const
{
	static const string strEmpty;

	if (m_ClassNames.empty())
		return strEmpty;

	return m_ClassNames[m_iSelectedClass];
}

ID3D11ShaderResourceView* Client::CHUDLayoutTool::Get_Or_Load_Texture(const string& strPath)
{
	if (strPath.empty())
		return nullptr;

	auto Iter = m_TextureCache.find(strPath);
	if (m_TextureCache.end() != Iter)
		return Iter->second.Get();

	const u8string utf8Path(strPath.begin(), strPath.end());
	const filesystem::path resolvedPath =
		CRuntimeAssetRoot::Resolve(filesystem::path(utf8Path));
	if (resolvedPath.empty())
		return nullptr;
	const filesystem::path Ext = resolvedPath.extension();

	ComPtr<ID3D11ShaderResourceView> pSRV = { nullptr };

	HRESULT hr = {};
	/* The authoring canvas and Debug runtime preview share the product HUD's
	   display-space contract, so their cached SRVs must preserve authored bytes. */
	if (0 == _wcsicmp(Ext.c_str(), L".dds"))
	{
		hr = CreateDDSTextureFromFileEx(
			m_pDevice.Get(), resolvedPath.c_str(), 0,
			D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
			0, 0, DDS_LOADER_IGNORE_SRGB, nullptr, &pSRV);
	}
	else
	{
		hr = CreateWICTextureFromFileEx(
			m_pDevice.Get(), resolvedPath.c_str(), 0,
			D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
			0, 0, WIC_LOADER_IGNORE_SRGB, nullptr, &pSRV);
	}

	m_TextureCache[strPath] = pSRV;

	return FAILED(hr) ? nullptr : pSRV.Get();
}

bool_t Client::CHUDLayoutTool::Save(const filesystem::path& path)
{
	if (path.empty() || L".json" != path.extension())
	{
		m_strDataStatus = "Save rejected: JSON path required";
		return false;
	}

	unordered_set<string> classSet;
	for (const string& className : m_ClassNames)
	{
		if (!Is_ValidIdentifier(className) ||
			!classSet.insert(className).second)
		{
			m_strDataStatus = "Save rejected: invalid class ID";
			return false;
		}
	}

	unordered_set<string> slotSet;
	for (const HUD_SLOT& slot : m_Slots)
	{
		if (!Is_ValidIdentifier(slot.strName) ||
			!slotSet.insert(slot.strName).second ||
			(!slot.strOwnerClass.empty() &&
				!classSet.contains(slot.strOwnerClass)) ||
			!isfinite(slot.fX) || !isfinite(slot.fY) ||
			!isfinite(slot.fSizeX) || !isfinite(slot.fSizeY) ||
			slot.fSizeX <= 0.f || slot.fSizeY <= 0.f)
		{
			m_strDataStatus = "Save rejected: invalid slot contract";
			return false;
		}
		for (const TEXTURE_LAYER& layer : slot.TextureLayers)
		{
			if (!Is_SafeUIResourceId(layer.strPath) ||
				(!layer.strHoverPath.empty() &&
					!Is_SafeUIResourceId(layer.strHoverPath)))
			{
				m_strDataStatus = "Save rejected: invalid UI resource ID";
				return false;
			}
		}
		if ((!slot.strShineTexture.empty() &&
				!Is_SafeUIResourceId(slot.strShineTexture)) ||
			any_of(slot.AnimationFrames.begin(),
				slot.AnimationFrames.end(), [](const string& frame)
				{
					return !Is_SafeUIResourceId(frame);
				}))
		{
			m_strDataStatus = "Save rejected: invalid animation resource ID";
			return false;
		}
	}

	error_code directoryError;
	filesystem::create_directories(path.parent_path(), directoryError);
	if (directoryError)
	{
		m_strDataStatus = "Save failed: could not create data directory";
		return false;
	}

	filesystem::path temporaryPath = path;
	temporaryPath += L".tmp";
	ofstream file(temporaryPath, ios::binary | ios::trunc);
	if (!file)
	{
		m_strDataStatus = "Save failed: could not open temporary file";
		return false;
	}
	file.imbue(locale::classic());
	file << setprecision(9);
	file << "{\n  \"schema\": \"lostark.ui-layout\",\n"
		<< "  \"formatVersion\": 1,\n"
		<< "  \"resolution\": { \"width\": " << ms_fRefWidth
		<< ", \"height\": " << ms_fRefHeight << " },\n"
		<< "  \"classes\": [";
	for (size_t index = 0; index < m_ClassNames.size(); ++index)
	{
		if (0u != index)
			file << ", ";
		Write_String(file, m_ClassNames[index]);
	}
	file << "],\n  \"slots\": [\n";

	for (size_t slotIndex = 0; slotIndex < m_Slots.size(); ++slotIndex)
	{
		const HUD_SLOT& slot = m_Slots[slotIndex];
		file << "    {\n      \"id\": ";
		Write_String(file, slot.strName);
		file << ",\n      \"ownerClass\": ";
		if (slot.strOwnerClass.empty())
			file << "null";
		else
			Write_String(file, slot.strOwnerClass);
		file << ",\n      \"type\": "
			<< static_cast<int32_t>(slot.eType)
			<< ",\n      \"rect\": { \"x\": " << slot.fX
			<< ", \"y\": " << slot.fY
			<< ", \"width\": " << slot.fSizeX
			<< ", \"height\": " << slot.fSizeY
			<< " },\n      \"rotation\": " << slot.fRotation
			<< ",\n      \"stages\": { \"baseFrom\": "
			<< slot.iBaseFromStage << ", \"shineFrom\": "
			<< slot.iShineFromStage << " },\n"
			<< "      \"layers\": [";

		for (size_t layerIndex = 0;
			layerIndex < slot.TextureLayers.size(); ++layerIndex)
		{
			const TEXTURE_LAYER& layer = slot.TextureLayers[layerIndex];
			if (0u != layerIndex)
				file << ',';
			file << "\n        { \"path\": ";
			Write_String(file, layer.strPath);
			file << ", \"hoverPath\": ";
			if (layer.strHoverPath.empty())
				file << "null";
			else
				Write_String(file, layer.strHoverPath);
			file << ", \"tint\": [" << layer.vTint[0] << ", "
				<< layer.vTint[1] << ", " << layer.vTint[2] << ", "
				<< layer.vTint[3] << "], \"additive\": "
				<< (layer.bAdditive ? "true" : "false")
				<< ", \"flipX\": "
				<< (layer.bFlipX ? "true" : "false") << " }";
		}
		if (!slot.TextureLayers.empty())
			file << '\n' << "      ";
		file << "],\n      \"shine\": { \"texture\": ";
		if (slot.strShineTexture.empty())
			file << "null";
		else
			Write_String(file, slot.strShineTexture);
		file << ", \"additive\": "
			<< (slot.bShineAdditive ? "true" : "false")
			<< " },\n      \"animation\": { \"fps\": "
			<< slot.fAnimationFPS << ", \"scale\": "
			<< slot.fAnimationScale << ", \"offset\": { \"x\": "
			<< slot.fAnimationOffsetX << ", \"y\": "
			<< slot.fAnimationOffsetY << " }, \"frames\": [";
		for (size_t frameIndex = 0;
			frameIndex < slot.AnimationFrames.size(); ++frameIndex)
		{
			if (0u != frameIndex)
				file << ", ";
			Write_String(file, slot.AnimationFrames[frameIndex]);
		}
		file << "], \"loop\": " << (slot.bAnimationLoop ? "true" : "false")
			<< ", \"additive\": " << (slot.bAnimationAdditive ? "true" : "false") << " }";
		if (!slot.strKeyframeAnimationPath.empty())
		{
			file << ",\n      \"keyframeAnimationPath\": ";
			Write_String(file, slot.strKeyframeAnimationPath);
			file << ",\n      \"keyframeAnimationScale\": " << slot.fKeyframeAnimationScale;
		}
		file << "\n    }";
		if (slotIndex + 1u != m_Slots.size())
			file << ',';
		file << '\n';
	}
	file << "  ]\n}\n";
	file.flush();
	const bool_t writeSucceeded = file.good();
	file.close();
	if (!writeSucceeded)
	{
		m_strDataStatus = "Save failed: atomic promote failed";
		return false;
	}

	Backup_PreviousVersion(path);

	if (!MoveFileExW(temporaryPath.c_str(), path.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		m_strDataStatus = "Save failed: atomic promote failed";
		return false;
	}

	m_strDataStatus = "JSON saved";
	return true;
}

bool_t Client::CHUDLayoutTool::Load(const filesystem::path& path)
{
	/* Every early-out below leaves this false, so a failed load on an existing (but invalid) file
	is distinguishable from a freshly-created empty document -- Switch_Document uses this to avoid
	auto-saving placeholder state over the real file. */
	m_bActiveDocumentLoaded = false;

	ifstream file(path, ios::binary);
	if (!file)
	{
		m_strDataStatus = "JSON not found";
		return false;
	}
	const string text{
		istreambuf_iterator<char>(file),
		istreambuf_iterator<char>()};
	DATA_JSON_VALUE root;
	string parseError;
	if (!CDataJson::Parse(text, root, parseError) || !root.Is_Object())
	{
		m_strDataStatus = "JSON parse failed: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* pSchema = Find_Member(
		root, "schema", DATA_JSON_TYPE::STRING);
	int32_t formatVersion = {};
	const DATA_JSON_VALUE* pResolution = Find_Member(
		root, "resolution", DATA_JSON_TYPE::OBJECT);
	f32_t width = {};
	f32_t height = {};
	if (nullptr == pSchema || "lostark.ui-layout" != pSchema->Get_String() ||
		!Read_Integer(root, "formatVersion", formatVersion, 1, 1) ||
		nullptr == pResolution ||
		!Read_Number(*pResolution, "width", width, 1.f, 16384.f) ||
		!Read_Number(*pResolution, "height", height, 1.f, 16384.f) ||
		fabsf(width - ms_fRefWidth) > 0.01f ||
		fabsf(height - ms_fRefHeight) > 0.01f)
	{
		m_strDataStatus = "JSON contract or resolution mismatch";
		return false;
	}

	const DATA_JSON_VALUE* pClasses = Find_Member(
		root, "classes", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* pSlots = Find_Member(
		root, "slots", DATA_JSON_TYPE::ARRAY);
	if (nullptr == pClasses || pClasses->Get_Array().empty() ||
		pClasses->Get_Array().size() > 64u || nullptr == pSlots ||
		pSlots->Get_Array().empty() || pSlots->Get_Array().size() > 4096u)
	{
		m_strDataStatus = "JSON collection limits rejected";
		return false;
	}

	vector<string> stagedClasses;
	unordered_set<string> classSet;
	for (const DATA_JSON_VALUE& classValue : pClasses->Get_Array())
	{
		if (!classValue.Is_String() ||
			!Is_ValidIdentifier(classValue.Get_String()) ||
			!classSet.insert(classValue.Get_String()).second)
		{
			m_strDataStatus = "JSON contains invalid class IDs";
			return false;
		}
		stagedClasses.push_back(classValue.Get_String());
	}

	vector<HUD_SLOT> stagedSlots;
	unordered_set<string> slotSet;
	for (const DATA_JSON_VALUE& slotValue : pSlots->Get_Array())
	{
		if (!slotValue.Is_Object())
		{
			m_strDataStatus = "JSON slot is not an object";
			return false;
		}

		HUD_SLOT slot;
		const DATA_JSON_VALUE* pId = Find_Member(
			slotValue, "id", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pOwner = slotValue.Find("ownerClass");
		int32_t type = {};
		const DATA_JSON_VALUE* pRect = Find_Member(
			slotValue, "rect", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pStages = Find_Member(
			slotValue, "stages", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pLayers = Find_Member(
			slotValue, "layers", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pShine = Find_Member(
			slotValue, "shine", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pAnimation = Find_Member(
			slotValue, "animation", DATA_JSON_TYPE::OBJECT);

		if (nullptr == pId || !Is_ValidIdentifier(pId->Get_String()) ||
			!slotSet.insert(pId->Get_String()).second || nullptr == pOwner ||
			!Read_Integer(slotValue, "type", type, 0,
				static_cast<int32_t>(SLOT_TYPE::KEYFRAME_ANIMATION)) ||
			nullptr == pRect || nullptr == pStages || nullptr == pLayers ||
			pLayers->Get_Array().size() > 32u || nullptr == pShine ||
			nullptr == pAnimation)
		{
			m_strDataStatus = "JSON slot contract rejected";
			return false;
		}

		slot.strName = pId->Get_String();
		if (pOwner->Is_String())
		{
			if (!classSet.contains(pOwner->Get_String()))
			{
				m_strDataStatus = "JSON slot owner is unknown";
				return false;
			}
			slot.strOwnerClass = pOwner->Get_String();
		}
		else if (!pOwner->Is_Null())
		{
			m_strDataStatus = "JSON ownerClass must be string or null";
			return false;
		}
		slot.eType = static_cast<SLOT_TYPE>(type);
		if (!Read_Number(*pRect, "x", slot.fX, -10000.f, 10000.f) ||
			!Read_Number(*pRect, "y", slot.fY, -10000.f, 10000.f) ||
			!Read_Number(*pRect, "width", slot.fSizeX, 0.01f, 10000.f) ||
			!Read_Number(*pRect, "height", slot.fSizeY, 0.01f, 10000.f) ||
			!Read_Number(slotValue, "rotation", slot.fRotation,
				-36000.f, 36000.f) ||
			!Read_Integer(*pStages, "baseFrom", slot.iBaseFromStage,
				0, ms_iMaxStage) ||
			!Read_Integer(*pStages, "shineFrom", slot.iShineFromStage,
				0, ms_iMaxStage))
		{
			m_strDataStatus = "JSON slot numeric bounds rejected";
			return false;
		}

		for (const DATA_JSON_VALUE& layerValue : pLayers->Get_Array())
		{
			if (!layerValue.Is_Object())
			{
				m_strDataStatus = "JSON layer is not an object";
				return false;
			}
			const DATA_JSON_VALUE* pPath = Find_Member(
				layerValue, "path", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pHover = layerValue.Find("hoverPath");
			const DATA_JSON_VALUE* pTint = Find_Member(
				layerValue, "tint", DATA_JSON_TYPE::ARRAY);
			TEXTURE_LAYER layer;
			if (nullptr == pPath || !Is_SafeUIResourceId(pPath->Get_String()) ||
				nullptr == pHover || nullptr == pTint ||
				4u != pTint->Get_Array().size() ||
				!Read_Boolean(layerValue, "additive", layer.bAdditive) ||
				!Read_Boolean(layerValue, "flipX", layer.bFlipX))
			{
				m_strDataStatus = "JSON layer contract rejected";
				return false;
			}
			layer.strPath = pPath->Get_String();
			if (pHover->Is_String())
			{
				if (!Is_SafeUIResourceId(pHover->Get_String()))
				{
					m_strDataStatus = "JSON hover resource ID rejected";
					return false;
				}
				layer.strHoverPath = pHover->Get_String();
			}
			else if (!pHover->Is_Null())
			{
				m_strDataStatus = "JSON hoverPath must be string or null";
				return false;
			}
			for (size_t tintIndex = 0; tintIndex < 4u; ++tintIndex)
			{
				const DATA_JSON_VALUE& tint = pTint->Get_Array()[tintIndex];
				if (!tint.Is_Number() || !isfinite(tint.Get_Number()) ||
					tint.Get_Number() < 0.0 || tint.Get_Number() > 64.0)
				{
					m_strDataStatus = "JSON tint rejected";
					return false;
				}
				layer.vTint[tintIndex] = static_cast<f32_t>(tint.Get_Number());
			}
			slot.TextureLayers.push_back(move(layer));
		}

		const DATA_JSON_VALUE* pShineTexture = pShine->Find("texture");
		if (nullptr == pShineTexture ||
			!Read_Boolean(*pShine, "additive", slot.bShineAdditive))
		{
			m_strDataStatus = "JSON shine contract rejected";
			return false;
		}
		if (pShineTexture->Is_String())
		{
			if (!Is_SafeUIResourceId(pShineTexture->Get_String()))
			{
				m_strDataStatus = "JSON shine resource ID rejected";
				return false;
			}
			slot.strShineTexture = pShineTexture->Get_String();
		}
		else if (!pShineTexture->Is_Null())
		{
			m_strDataStatus = "JSON shine texture must be string or null";
			return false;
		}

		const DATA_JSON_VALUE* pOffset = Find_Member(
			*pAnimation, "offset", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pFrames = Find_Member(
			*pAnimation, "frames", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pOffset || nullptr == pFrames ||
			pFrames->Get_Array().size() > 128u ||
			!Read_Number(*pAnimation, "fps", slot.fAnimationFPS,
				0.1f, 240.f) ||
			!Read_Number(*pAnimation, "scale", slot.fAnimationScale,
				0.01f, 100.f) ||
			!Read_Number(*pOffset, "x", slot.fAnimationOffsetX,
				-10000.f, 10000.f) ||
			!Read_Number(*pOffset, "y", slot.fAnimationOffsetY,
				-10000.f, 10000.f))
		{
			m_strDataStatus = "JSON animation contract rejected";
			return false;
		}
		for (const DATA_JSON_VALUE& frame : pFrames->Get_Array())
		{
			if (!frame.Is_String() ||
				!Is_SafeUIResourceId(frame.Get_String()))
			{
				m_strDataStatus = "JSON animation frame rejected";
				return false;
			}
			slot.AnimationFrames.push_back(frame.Get_String());
		}

		/* Optional, defaults to true (matches CHUDRuntimeView's own default) if the JSON omits
		it -- round-tripped so Save() below does not drop a one-shot slot's loop=false back to
		looping. */
		if (const DATA_JSON_VALUE* pLoop = pAnimation->Find("loop"))
		{
			if (!pLoop->Is_Boolean())
			{
				m_strDataStatus = "JSON animation loop must be boolean";
				return false;
			}
			slot.bAnimationLoop = pLoop->Get_Boolean();
		}

		/* Optional, defaults to false if the JSON omits it -- round-tripped so Save() below does
		not drop an additive flipbook's flag back to non-additive. */
		if (const DATA_JSON_VALUE* pAdditive = pAnimation->Find("additive"))
		{
			if (!pAdditive->Is_Boolean())
			{
				m_strDataStatus = "JSON animation additive must be boolean";
				return false;
			}
			slot.bAnimationAdditive = pAdditive->Get_Boolean();
		}

		/* Optional: only KEYFRAME_ANIMATION slots carry this. Round-tripped as-is (not required,
		not defaulted away) so Save() below does not drop it out from under CHUDRuntimeView. */
		if (const DATA_JSON_VALUE* pKeyframePath = slotValue.Find("keyframeAnimationPath"))
		{
			if (pKeyframePath->Is_String())
			{
				if (!Is_SafeUIResourceId(pKeyframePath->Get_String()))
				{
					m_strDataStatus = "JSON keyframeAnimationPath rejected";
					return false;
				}
				slot.strKeyframeAnimationPath = pKeyframePath->Get_String();
			}
			else if (!pKeyframePath->Is_Null())
			{
				m_strDataStatus = "JSON keyframeAnimationPath must be string or null";
				return false;
			}
		}
		if (!slot.strKeyframeAnimationPath.empty() &&
			!Read_Number(slotValue, "keyframeAnimationScale",
				slot.fKeyframeAnimationScale, 0.01f, 100.f))
		{
			m_strDataStatus = "JSON keyframeAnimationScale rejected";
			return false;
		}

		stagedSlots.push_back(move(slot));
	}

	m_Slots = move(stagedSlots);
	m_ClassNames = move(stagedClasses);
	m_iSelectedClass = 0;
	m_iSelectedSlot = -1;
	m_SelectedSlots.clear();
	m_iLastScannedClass = -1;
	m_strDataStatus = "JSON loaded";
	m_bActiveDocumentLoaded = true;
	return true;
}

void Client::CHUDLayoutTool::Reset_Default()
{
	m_Slots.clear();
	m_ClassNames.clear();
	m_ClassNames.push_back("Default");
	m_iSelectedClass = 0;
	m_iSelectedSlot = -1;

	auto Fn_AddSlot = [this](const string& strName, float fX, float fY, float fW, float fH, const string& strOwnerClass = string())
	{
		HUD_SLOT Slot{};
		Slot.strName = strName;
		Slot.fX = fX;
		Slot.fY = fY;
		Slot.fSizeX = fW;
		Slot.fSizeY = fH;
		Slot.strOwnerClass = strOwnerClass;
		m_Slots.push_back(move(Slot));
	};

	Fn_AddSlot("Evade", 328.f, 600.f, 50.f, 50.f);
	Fn_AddSlot("Potion", 332.f, 656.f, 42.f, 42.f);

	Fn_AddSlot("HealthBar", 390.f, 600.f, 270.f, 20.f);
	Fn_AddSlot("Skill_Q", 392.f, 628.f, 50.f, 50.f);
	Fn_AddSlot("Skill_W", 446.f, 628.f, 50.f, 50.f);
	Fn_AddSlot("Skill_E", 500.f, 628.f, 50.f, 50.f);
	Fn_AddSlot("Skill_R", 554.f, 628.f, 50.f, 50.f);
	Fn_AddSlot("Skill_A", 420.f, 682.f, 46.f, 46.f);
	Fn_AddSlot("Skill_S", 470.f, 682.f, 46.f, 46.f);
	Fn_AddSlot("Skill_D", 520.f, 682.f, 46.f, 46.f);
	Fn_AddSlot("Skill_F", 570.f, 682.f, 46.f, 46.f);

	Fn_AddSlot("Identity", 592.f, 590.f, 96.f, 96.f, "Default");

	Fn_AddSlot("ManaBar", 712.f, 600.f, 258.f, 20.f);
	Fn_AddSlot("Skill_F1", 968.f, 628.f, 46.f, 46.f);

	for (int32_t i = 0; i < 5; ++i)
		Fn_AddSlot("Item_" + to_string(i + 1), 1018.f + i * 50.f, 628.f, 46.f, 46.f);

	for (int32_t i = 0; i < 4; ++i)
		Fn_AddSlot("Item_" + to_string(i + 6), 1018.f + i * 50.f, 682.f, 46.f, 46.f);
}
