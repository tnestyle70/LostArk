#pragma once

#include "Network/PacketMessages.h"

#include <string>
#include <unordered_set>

namespace LostArk::Server
{
	/* The honor titles a player may wear, straight from the published
	Server/Bin/DataFiles/HonorTitles/HonorTitles.bootstrap (Publish-HonorTitles.ps1 from
	Data/Titles/HonorTitles.json). Titles are cosmetic and there is no achievement system, so
	admission is membership only: an id the bootstrap does not list is refused. */
	class CHonorTitleCatalog final
	{
	public:
		bool Load();
		bool Has_Title(LostArk::Shared::HONOR_TITLE_ID titleId) const;
		const std::string& Get_Status() const { return m_strStatus; }

	private:
		std::unordered_set<LostArk::Shared::HONOR_TITLE_ID> m_Titles;
		std::string m_strStatus;
	};
}
