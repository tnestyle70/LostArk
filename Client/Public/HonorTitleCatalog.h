#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/NetworkIds.h"

#include <string>
#include <vector>

NS_BEGIN(Client)

struct HONOR_TITLE_ENTRY
{
	LostArk::Shared::HONOR_TITLE_ID iTitleId = LostArk::Shared::INVALID_HONOR_TITLE_ID;
	wstring strName;
};

/* honortitle.gfx / characterinfo.gfx labels the title UI draws (EFTable_GameMsg). */
struct HONOR_TITLE_STRINGS
{
	wstring strWindowTitle;   // sys.honortitle.ui_title
	wstring strApply;         // sys.honortitle.ui_button_apply
	wstring strRemove;        // sys.honortitle.ui_button_remove
	wstring strCurrent;       // sys.honortitle.ui_rabel_using_honortitle
	wstring strUsing;         // useType_mc "using" frame text
	wstring strChange;        // sys.characterinfo.button_change_honortitle
	wstring strNone;          // sys.honortitle.ui_filter_empty_info
};

/* Data/Titles/HonorTitles.json: the titles a player may wear and their display names. The
Server admits ids from the bootstrap Publish-HonorTitles.ps1 makes of the same document; this
side only turns a replicated id into the text drawn over a head and in the title window. */
class CHonorTitleCatalog final
{
public:
	/* On failure the previously loaded set is kept and outStatus explains why, same
	contract as CItemCatalog::Load. */
	static bool_t Load(std::string& outStatus);

	/* Document order (the title window lists them as they are). */
	static const std::vector<HONOR_TITLE_ENTRY>& Get_Titles();
	static const HONOR_TITLE_ENTRY* Find(LostArk::Shared::HONOR_TITLE_ID iTitleId);
	/* nullptr for INVALID_HONOR_TITLE_ID and for an id the document does not carry. */
	static const wstring* Find_Name(LostArk::Shared::HONOR_TITLE_ID iTitleId);
	static const HONOR_TITLE_STRINGS& Get_Strings();
};

NS_END
