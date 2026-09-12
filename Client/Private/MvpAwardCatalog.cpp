#include "MvpAwardCatalog.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <fstream>

namespace
{
	constexpr const char* MVP_AWARDS_DOCUMENT = "UI/MVP/MvpAwards.json";
	constexpr const char* MVP_CONTENT_NAMES_DOCUMENT =
		"UI/MVP/MvpContentNames.json";

	bool_t Convert_Utf8ToWide(const string& strUtf8, wstring_t& outWide)
	{
		outWide.clear();
		if (strUtf8.empty())
			return true;

		const int32_t iLength = ::MultiByteToWideChar(
			CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
		if (iLength <= 0)
			return false;

		outWide.resize(static_cast<size_t>(iLength) - 1u);
		::MultiByteToWideChar(
			CP_UTF8, 0, strUtf8.c_str(), -1, outWide.data(), iLength);
		return true;
	}

	/* Reads and parses one document; a missing or malformed file is not fatal,
	   the caller just gets nothing for that lookup. */
	bool_t Read_Document(const char* const szRelativePath, DATA_JSON_VALUE& outRoot)
	{
		const filesystem::path DataPath = CProjectDataRoot::Resolve(szRelativePath);
		ifstream Stream(DataPath, ios::binary);
		if (!Stream.is_open())
			return false;

		const string Text(
			(istreambuf_iterator<char>(Stream)),
			istreambuf_iterator<char>());

		string Error;
		return CDataJson::Parse(Text, outRoot, Error) && outRoot.Is_Object();
	}

	/* "#rrggbb" as GameMsg writes it. Anything else leaves the piece uncoloured,
	   which draws it in the field's own white. */
	bool_t Parse_HexColor(const string& strValue, float4_t& outColor)
	{
		if (7u != strValue.size() || '#' != strValue[0])
			return false;

		int32_t iChannels[3] = {};
		for (size_t i = 0; i < 3u; ++i)
		{
			int32_t iValue = 0;
			for (size_t iDigit = 0; iDigit < 2u; ++iDigit)
			{
				const char_t cDigit = strValue[1u + i * 2u + iDigit];
				int32_t iNibble = 0;
				if (cDigit >= '0' && cDigit <= '9')		iNibble = cDigit - '0';
				else if (cDigit >= 'a' && cDigit <= 'f')	iNibble = cDigit - 'a' + 10;
				else if (cDigit >= 'A' && cDigit <= 'F')	iNibble = cDigit - 'A' + 10;
				else return false;
				iValue = iValue * 16 + iNibble;
			}
			iChannels[i] = iValue;
		}
		outColor = float4_t(
			iChannels[0] / 255.f, iChannels[1] / 255.f, iChannels[2] / 255.f, 1.f);
		return true;
	}

	const DATA_JSON_VALUE* Find_Member(
		const DATA_JSON_VALUE& Value, const char* const szKey)
	{
		return Value.Is_Object() ? Value.Find(szKey) : nullptr;
	}

	bool_t Read_Number(
		const DATA_JSON_VALUE& Value, const char* const szKey, f32_t& outValue)
	{
		const DATA_JSON_VALUE* pMember = Find_Member(Value, szKey);
		if (nullptr == pMember || !pMember->Is_Number())
			return false;
		outValue = static_cast<f32_t>(pMember->Get_Number());
		return true;
	}

	bool_t Read_Int(
		const DATA_JSON_VALUE& Value, const char* const szKey, int32_t& outValue)
	{
		f32_t fValue = 0.f;
		if (!Read_Number(Value, szKey, fValue))
			return false;
		outValue = static_cast<int32_t>(fValue);
		return true;
	}

	bool_t Read_Wide(
		const DATA_JSON_VALUE& Value, const char* const szKey, wstring_t& outValue)
	{
		const DATA_JSON_VALUE* pMember = Find_Member(Value, szKey);
		if (nullptr == pMember || !pMember->Is_String())
			return false;
		return Convert_Utf8ToWide(pMember->Get_String(), outValue);
	}

	/* Three ascending lower bounds. A partial or unsorted list is dropped whole,
	   so a bad row cannot silently award the wrong tier. */
	bool_t Read_Thresholds(
		const DATA_JSON_VALUE& Value, const char* const szKey, f32_t outValues[3])
	{
		const DATA_JSON_VALUE* pMember = Find_Member(Value, szKey);
		if (nullptr == pMember || !pMember->Is_Array())
			return false;

		const DATA_JSON_VALUE::ARRAY& Entries = pMember->Get_Array();
		if (3u != Entries.size())
			return false;

		for (size_t i = 0; i < 3u; ++i)
		{
			if (!Entries[i].Is_Number())
				return false;
			outValues[i] = static_cast<f32_t>(Entries[i].Get_Number());
		}
		return outValues[0] <= outValues[1] && outValues[1] <= outValues[2];
	}
}

Client::CMvpAwardCatalog& Client::CMvpAwardCatalog::Get()
{
	static CMvpAwardCatalog instance;
	return instance;
}

Client::CMvpAwardCatalog::CMvpAwardCatalog()
{
	Load_ContentNames();

	DATA_JSON_VALUE Root;
	if (!Read_Document(MVP_AWARDS_DOCUMENT, Root))
		return;

	int32_t iFormatVersion = 0;
	if (!Read_Int(Root, "formatVersion", iFormatVersion) || 1 != iFormatVersion)
		return;
	(void)Read_Int(Root, "mvpGroupId", m_iMvpGroupId);

	/* Staged locally and swapped in at the end, so a document that runs out
	   halfway leaves the catalog empty instead of partly populated. */
	vector<MVP_AWARD_STAT> Stats;
	if (const DATA_JSON_VALUE* pStats = Root.Find("stats"))
	{
		if (!pStats->Is_Array())
			return;

		for (const DATA_JSON_VALUE& Value : pStats->Get_Array())
		{
			MVP_AWARD_STAT Stat;
			if (!Read_Int(Value, "primaryKey", Stat.iPrimaryKey) ||
				!Read_Int(Value, "statType", Stat.iStatType) ||
				!Read_Wide(Value, "name", Stat.strName))
				return;
			(void)Read_Int(Value, "calcType", Stat.iCalcType);

			const DATA_JSON_VALUE* pTitles = Value.Find("titles");
			if (nullptr == pTitles || !pTitles->Is_Array() ||
				3u != pTitles->Get_Array().size())
				return;
			for (size_t i = 0; i < 3u; ++i)
			{
				const DATA_JSON_VALUE& Title = pTitles->Get_Array()[i];
				if (!Title.Is_String() ||
					!Convert_Utf8ToWide(Title.Get_String(), Stat.strTitles[i]))
					return;
			}

			const DATA_JSON_VALUE* pRule = Value.Find("titleRule");
			if (nullptr != pRule && pRule->Is_String() &&
				"survive" == pRule->Get_String())
				Stat.eTitleRule = MVP_TITLE_RULE::SURVIVE;

			(void)Read_Int(Value, "weightTier", Stat.iWeightTier);
			if (const DATA_JSON_VALUE* pRole = Value.Find("scoreRole"))
			{
				if (pRole->Is_String())
					Stat.strScoreRole = pRole->Get_String();
			}

			if (const DATA_JSON_VALUE* pThresholds =
				Value.Find("sharePercentThresholds"))
			{
				Stat.bHasThresholds4 =
					Read_Thresholds(*pThresholds, "party4", Stat.fThresholds4);
				Stat.bHasThresholds8 =
					Read_Thresholds(*pThresholds, "party8", Stat.fThresholds8);
			}

			Stats.push_back(std::move(Stat));
		}
	}

	vector<MVP_AWARD_MEDAL> Medals;
	if (const DATA_JSON_VALUE* pMedals = Root.Find("medals"))
	{
		if (!pMedals->Is_Array())
			return;

		for (const DATA_JSON_VALUE& Value : pMedals->Get_Array())
		{
			MVP_AWARD_MEDAL Medal;
			if (!Read_Int(Value, "medalIndex", Medal.iMedalIndex) ||
				!Read_Int(Value, "iconIndex", Medal.iIconIndex) ||
				!Read_Wide(Value, "name", Medal.strName))
				return;
			(void)Read_Int(Value, "param1", Medal.iParam1);
			(void)Read_Int(Value, "param2", Medal.iParam2);
			(void)Read_Int(Value, "param3", Medal.iParam3);
			Medals.push_back(std::move(Medal));
		}
	}

	vector<int32_t> Exclusive;
	if (const DATA_JSON_VALUE* pExclusive = Root.Find("columnExclusiveStatTypes"))
	{
		if (!pExclusive->Is_Array())
			return;

		for (const DATA_JSON_VALUE& Value : pExclusive->Get_Array())
		{
			if (!Value.Is_Number())
				return;
			Exclusive.push_back(static_cast<int32_t>(Value.Get_Number()));
		}
	}

	if (Stats.empty() || Medals.empty())
		return;

	m_Stats = std::move(Stats);
	m_Medals = std::move(Medals);
	m_ColumnExclusiveStatTypes = std::move(Exclusive);
	m_bLoaded = true;
}

const Client::MVP_AWARD_STAT* Client::CMvpAwardCatalog::Find_Stat(
	const int32_t iStatType) const
{
	const auto it = std::find_if(m_Stats.begin(), m_Stats.end(),
		[iStatType](const MVP_AWARD_STAT& Stat)
		{ return Stat.iStatType == iStatType; });
	return m_Stats.end() == it ? nullptr : &(*it);
}

const wchar_t* Client::CMvpAwardCatalog::Resolve_ShareTitle(
	const int32_t iStatType,
	const int32_t iPartySize,
	const f32_t fSharePercent) const
{
	const MVP_AWARD_STAT* pStat = Find_Stat(iStatType);
	if (nullptr == pStat || MVP_TITLE_RULE::SHARE != pStat->eTitleRule)
		return nullptr;

	const bool_t bEightPlayer = (8 == iPartySize);
	if (bEightPlayer ? !pStat->bHasThresholds8 : !pStat->bHasThresholds4)
		return nullptr;
	const f32_t* pThresholds =
		bEightPlayer ? pStat->fThresholds8 : pStat->fThresholds4;

	/* Highest tier whose lower bound the share reaches. */
	for (int32_t iTier = 2; iTier >= 0; --iTier)
	{
		if (fSharePercent >= pThresholds[iTier])
			return pStat->strTitles[iTier].c_str();
	}
	return nullptr;
}

const wchar_t* Client::CMvpAwardCatalog::Resolve_SurvivalTitle(
	const int32_t iStatType) const
{
	const MVP_AWARD_STAT* pStat = Find_Stat(iStatType);
	if (nullptr == pStat || MVP_TITLE_RULE::SURVIVE != pStat->eTitleRule)
		return nullptr;
	return pStat->strTitles[0].c_str();
}

const Client::MVP_AWARD_MEDAL* Client::CMvpAwardCatalog::Find_Medal(
	const int32_t iMedalIndex) const
{
	const auto it = std::find_if(m_Medals.begin(), m_Medals.end(),
		[iMedalIndex](const MVP_AWARD_MEDAL& Medal)
		{ return Medal.iMedalIndex == iMedalIndex; });
	return m_Medals.end() == it ? nullptr : &(*it);
}

bool_t Client::CMvpAwardCatalog::Is_ColumnExclusive(const int32_t iStatType) const
{
	return m_ColumnExclusiveStatTypes.end() != std::find(
		m_ColumnExclusiveStatTypes.begin(),
		m_ColumnExclusiveStatTypes.end(), iStatType);
}

vector<Client::MVP_RESULT_STAT> Client::CMvpAwardCatalog::Select_Rows(
	const MVP_AWARD_PARTICIPANT& Participant,
	const int32_t iPartySize,
	const bool_t bIsMvpCard,
	vector<int32_t>& ExclusiveStatTypesTaken) const
{
	/* Highest score first. Two equal scores fall back on the reference's own
	   ordering of the contributions, then on the stat id so the page is stable
	   from one replay to the next. */
	vector<const MVP_AWARD_CONTRIBUTION*> Ordered;
	Ordered.reserve(Participant.Contributions.size());
	for (const MVP_AWARD_CONTRIBUTION& Contribution : Participant.Contributions)
		Ordered.push_back(&Contribution);

	std::stable_sort(Ordered.begin(), Ordered.end(),
		[this](const MVP_AWARD_CONTRIBUTION* pLeft,
			const MVP_AWARD_CONTRIBUTION* pRight)
		{
			if (pLeft->fScore != pRight->fScore)
				return pLeft->fScore > pRight->fScore;

			const MVP_AWARD_STAT* pLeftStat = Find_Stat(pLeft->iStatType);
			const MVP_AWARD_STAT* pRightStat = Find_Stat(pRight->iStatType);
			const int32_t iLeftTier = (nullptr != pLeftStat) ? pLeftStat->iWeightTier : 0;
			const int32_t iRightTier = (nullptr != pRightStat) ? pRightStat->iWeightTier : 0;
			/* Tier 0 is outside the total, so it sorts last rather than first. */
			const int32_t iLeftRank = (0 == iLeftTier) ? INT32_MAX : iLeftTier;
			const int32_t iRightRank = (0 == iRightTier) ? INT32_MAX : iRightTier;
			if (iLeftRank != iRightRank)
				return iLeftRank < iRightRank;
			return pLeft->iStatType < pRight->iStatType;
		});

	vector<MVP_RESULT_STAT> Rows;
	for (const MVP_AWARD_CONTRIBUTION* pContribution : Ordered)
	{
		if (Rows.size() >= MVP_RESULT_MAX_STATS)
			break;

		const MVP_AWARD_STAT* pStat = Find_Stat(pContribution->iStatType);
		if (nullptr == pStat)
			continue;

		/* Already spoken for by an earlier column. */
		const bool_t bExclusive =
			!bIsMvpCard && Is_ColumnExclusive(pContribution->iStatType);
		if (bExclusive && ExclusiveStatTypesTaken.end() != std::find(
			ExclusiveStatTypesTaken.begin(), ExclusiveStatTypesTaken.end(),
			pContribution->iStatType))
			continue;

		const wchar_t* pTitle = (MVP_TITLE_RULE::SURVIVE == pStat->eTitleRule)
			? Resolve_SurvivalTitle(pContribution->iStatType)
			: Resolve_ShareTitle(pContribution->iStatType, iPartySize,
				pContribution->fSharePercent);
		if (nullptr == pTitle)
			continue;

		if (bExclusive)
			ExclusiveStatTypesTaken.push_back(pContribution->iStatType);

		/* The page highlights the contribution the card leads with. */
		Rows.push_back({ pTitle, pStat->strName, pContribution->strValue,
			Rows.empty() });
	}
	return Rows;
}

Client::MVP_RESULT_DATA Client::CMvpAwardCatalog::Compose_Page(
	const vector<MVP_TEXT_RUN>& ContentName,
	const vector<MVP_AWARD_PARTICIPANT>& Participants,
	const int32_t iPartySize) const
{
	MVP_RESULT_DATA Data;
	Data.ContentName = ContentName;
	if (!m_bLoaded || Participants.empty())
		return Data;

	vector<const MVP_AWARD_PARTICIPANT*> Ranked;
	Ranked.reserve(Participants.size());
	for (const MVP_AWARD_PARTICIPANT& Participant : Participants)
		Ranked.push_back(&Participant);

	std::stable_sort(Ranked.begin(), Ranked.end(),
		[](const MVP_AWARD_PARTICIPANT* pLeft, const MVP_AWARD_PARTICIPANT* pRight)
		{ return pLeft->fTotalScore > pRight->fTotalScore; });

	auto Fill = [&](const MVP_AWARD_PARTICIPANT& Participant,
		const bool_t bIsMvpCard, vector<int32_t>& Taken)
	{
		MVP_RESULT_ENTRY Entry;
		Entry.strCharacterName = Participant.strCharacterName;
		Entry.strGuildName = Participant.strGuildName;
		Entry.Stats = Select_Rows(Participant, iPartySize, bIsMvpCard, Taken);
		for (const int32_t iMedal : Participant.Medals)
		{
			if (nullptr != Find_Medal(iMedal))
				Entry.Medals.push_back(iMedal);
		}
		return Entry;
	};

	/* The MVP card is scored on its own; the columns share one exclusivity
	   ledger between them. */
	vector<int32_t> MvpTaken;
	Data.Mvp = Fill(*Ranked.front(), true, MvpTaken);

	vector<int32_t> ColumnTaken;
	const size_t iColumns = (std::min)(
		Ranked.size() - 1u, MVP_RESULT_MAX_PARTY_COLUMNS);
	for (size_t i = 0; i < iColumns; ++i)
		Data.Party.push_back(Fill(*Ranked[i + 1u], false, ColumnTaken));

	return Data;
}

void Client::CMvpAwardCatalog::Load_ContentNames()
{
	DATA_JSON_VALUE Root;
	if (!Read_Document(MVP_CONTENT_NAMES_DOCUMENT, Root))
		return;

	int32_t iFormatVersion = 0;
	if (!Read_Int(Root, "formatVersion", iFormatVersion) || 1 != iFormatVersion)
		return;

	/* difficulties are keyed by a string id, gates and raids by a number. */
	const auto Fn_ReadList = [&](const char* const szKey, const char* const szIdKey,
		const char* const szTextKey, vector<CONTENT_NAME_PIECE>& outPieces)
	{
		const DATA_JSON_VALUE* pList = Root.Find(szKey);
		if (nullptr == pList || !pList->Is_Array())
			return;

		for (const DATA_JSON_VALUE& Value : pList->Get_Array())
		{
			CONTENT_NAME_PIECE Piece;
			if (!Read_Wide(Value, szTextKey, Piece.strText) || Piece.strText.empty())
				continue;

			const DATA_JSON_VALUE* pId = Find_Member(Value, szIdKey);
			if (nullptr == pId)
				continue;
			if (pId->Is_String())
				Piece.strId = pId->Get_String();
			else if (pId->Is_Number())
				Piece.iKey = static_cast<int32_t>(pId->Get_Number());
			else
				continue;

			const DATA_JSON_VALUE* pColor = Find_Member(Value, "color");
			if (nullptr != pColor && pColor->Is_String())
				Piece.bHasColor = Parse_HexColor(pColor->Get_String(), Piece.vColor);

			outPieces.push_back(std::move(Piece));
		}
	};

	Fn_ReadList("difficulties", "id", "text", m_Difficulties);
	Fn_ReadList("gates", "gate", "text", m_Gates);
	Fn_ReadList("raids", "groupId", "name", m_Raids);
}

vector<Client::MVP_TEXT_RUN> Client::CMvpAwardCatalog::Build_ContentName(
	const int32_t iRaidGroupId,
	const int32_t iGate,
	const char* const szDifficultyId) const
{
	vector<MVP_TEXT_RUN> Runs;

	const auto Fn_Append = [&Runs](const CONTENT_NAME_PIECE& Piece)
	{
		MVP_TEXT_RUN Run;
		Run.strText = Piece.strText;
		if (Piece.bHasColor)
			Run.vColor = Piece.vColor;
		Runs.push_back(std::move(Run));
	};

	if (nullptr != szDifficultyId)
	{
		const auto it = std::find_if(m_Difficulties.begin(), m_Difficulties.end(),
			[szDifficultyId](const CONTENT_NAME_PIECE& Piece)
			{ return Piece.strId == szDifficultyId; });
		if (m_Difficulties.end() != it)
			Fn_Append(*it);
	}

	{
		const auto it = std::find_if(m_Raids.begin(), m_Raids.end(),
			[iRaidGroupId](const CONTENT_NAME_PIECE& Piece)
			{ return Piece.iKey == iRaidGroupId; });
		if (m_Raids.end() != it)
			Fn_Append(*it);
	}

	{
		const auto it = std::find_if(m_Gates.begin(), m_Gates.end(),
			[iGate](const CONTENT_NAME_PIECE& Piece)
			{ return Piece.iKey == iGate; });
		if (m_Gates.end() != it)
			Fn_Append(*it);
	}

	return Runs;
}
