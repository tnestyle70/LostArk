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
	constexpr const char* MVP_CLASS_SYMBOLS_DOCUMENT =
		"UI/MVP/MvpClassSymbols.json";
	constexpr const char* MVP_STAGE_REVEAL_DOCUMENT =
		"UI/MVP/MvpResult_StageReveal.json";
	/* The document names its panels the way mvp.gfx names the instances that own
	   them, so the order here is the page's own stage order. */
	constexpr const char* MVP_STAGE_REVEAL_KEYS[] =
		{ "mvp", "party0", "party1", "party2" };

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
	Load_ClassSymbols();
	Load_StageReveal();

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
	const int32_t iPartySize,
	vector<size_t>* const pOutRankedIndices) const
{
	MVP_RESULT_DATA Data;
	Data.ContentName = ContentName;
	if (nullptr != pOutRankedIndices)
		pOutRankedIndices->clear();
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
		Entry.Emblem = Find_ClassEmblem(Participant.strNetworkClassId);
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

	if (nullptr != pOutRankedIndices)
	{
		for (size_t i = 0; i <= iColumns; ++i)
			pOutRankedIndices->push_back(static_cast<size_t>(Ranked[i] - Participants.data()));
	}
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

void Client::CMvpAwardCatalog::Load_ClassSymbols()
{
	DATA_JSON_VALUE Root;
	if (!Read_Document(MVP_CLASS_SYMBOLS_DOCUMENT, Root))
		return;

	int32_t iFormatVersion = 0;
	if (!Read_Int(Root, "formatVersion", iFormatVersion) || 1 != iFormatVersion)
		return;

	if (const DATA_JSON_VALUE* pPlacement = Root.Find("placement"))
	{
		if (const DATA_JSON_VALUE* pBig = Find_Member(*pPlacement, "bigSymbol"))
		{
			(void)Read_Number(*pBig, "stageX", m_Placement.fBigStageX);
			(void)Read_Number(*pBig, "stageY", m_Placement.fBigStageY);
			(void)Read_Number(*pBig, "fadeInStartFrame",
				m_Placement.fBigFadeInStartFrame);
			(void)Read_Number(*pBig, "fadeInEndFrame",
				m_Placement.fBigFadeInEndFrame);
		}
		if (const DATA_JSON_VALUE* pColumn =
			Find_Member(*pPlacement, "partyColumnIcon"))
		{
			(void)Read_Number(*pColumn, "localX", m_Placement.fColumnLocalX);
			(void)Read_Number(*pColumn, "localY", m_Placement.fColumnLocalY);
			(void)Read_Number(*pColumn, "scale", m_Placement.fColumnScale);
		}
	}

	const DATA_JSON_VALUE* pClasses = Root.Find("classes");
	if (nullptr == pClasses || !pClasses->Is_Array())
		return;

	for (const DATA_JSON_VALUE& Value : pClasses->Get_Array())
	{
		const DATA_JSON_VALUE* pId = Find_Member(Value, "networkClassId");
		if (nullptr == pId || !pId->Is_String())
			continue;

		MVP_CLASS_EMBLEM Emblem;
		(void)Read_Int(Value, "classKey", Emblem.iClassKey);
		if (!Read_Wide(Value, "bigAsset", Emblem.strBigAsset) ||
			!Read_Wide(Value, "smallAsset", Emblem.strSmallAsset))
			continue;
		(void)Read_Number(Value, "offsetX", Emblem.fOffsetX);
		(void)Read_Number(Value, "offsetY", Emblem.fOffsetY);
		(void)Read_Number(Value, "width", Emblem.fWidth);
		(void)Read_Number(Value, "height", Emblem.fHeight);
		if (!Emblem.Is_Valid())
			continue;

		m_ClassEmblems.emplace_back(pId->Get_String(), std::move(Emblem));
	}
}

Client::MVP_CLASS_EMBLEM Client::CMvpAwardCatalog::Find_ClassEmblem(
	const string& strNetworkClassId) const
{
	const auto it = std::find_if(m_ClassEmblems.begin(), m_ClassEmblems.end(),
		[&strNetworkClassId](const pair<string, MVP_CLASS_EMBLEM>& Entry)
		{ return Entry.first == strNetworkClassId; });
	return m_ClassEmblems.end() == it ? MVP_CLASS_EMBLEM() : it->second;
}

void Client::CMvpAwardCatalog::Load_StageReveal()
{
	DATA_JSON_VALUE Root;
	if (!Read_Document(MVP_STAGE_REVEAL_DOCUMENT, Root))
		return;

	f32_t fFrameRate = 0.f;
	if (Read_Number(Root, "frameRate", fFrameRate) && fFrameRate > 0.f)
		m_fStageRevealFrameRate = fFrameRate;

	const DATA_JSON_VALUE* pSlots = Root.Find("slots");
	if (nullptr == pSlots)
		return;

	for (size_t iSlot = 0; iSlot < MVP_STAGE_SLOT_COUNT; ++iSlot)
	{
		const DATA_JSON_VALUE* pSlot = pSlots->Find(MVP_STAGE_REVEAL_KEYS[iSlot]);
		if (nullptr == pSlot)
			continue;
		const DATA_JSON_VALUE* pKeys = pSlot->Find("keyframes");
		if (nullptr == pKeys || !pKeys->Is_Array())
			continue;

		/* Staged locally so a document that runs out halfway leaves this panel
		   with no curve -- drawn without a reveal -- instead of a partial one. */
		vector<STAGE_REVEAL_KEY> Keys;
		bool_t bComplete = true;
		for (const DATA_JSON_VALUE& Value : pKeys->Get_Array())
		{
			STAGE_REVEAL_KEY Key;
			if (!Read_Number(Value, "frame", Key.fFrame) ||
				!Read_Number(Value, "alpha", Key.fAlpha))
			{
				bComplete = false;
				break;
			}
			(void)Read_Number(Value, "dx", Key.fOffsetX);
			(void)Read_Number(Value, "dy", Key.fOffsetY);
			Keys.push_back(Key);
		}
		if (bComplete && !Keys.empty())
			m_StageReveal[iSlot] = move(Keys);
	}
}

bool_t Client::CMvpAwardCatalog::Sample_StageReveal(const size_t iSlot,
	const f32_t fFrame, f32_t& fOutAlpha, f32_t& fOutOffsetX, f32_t& fOutOffsetY) const
{
	if (iSlot >= MVP_STAGE_SLOT_COUNT || m_StageReveal[iSlot].empty())
		return false;

	const vector<STAGE_REVEAL_KEY>& Keys = m_StageReveal[iSlot];
	/* Before the first authored key the panel is not on the page yet, and after
	   the last one it holds -- the same way the authored timeline behaves. */
	if (fFrame <= Keys.front().fFrame)
	{
		fOutAlpha = Keys.front().fAlpha;
		fOutOffsetX = Keys.front().fOffsetX;
		fOutOffsetY = Keys.front().fOffsetY;
		return true;
	}
	if (fFrame >= Keys.back().fFrame)
	{
		fOutAlpha = Keys.back().fAlpha;
		fOutOffsetX = Keys.back().fOffsetX;
		fOutOffsetY = Keys.back().fOffsetY;
		return true;
	}

	for (size_t i = 1; i < Keys.size(); ++i)
	{
		if (fFrame > Keys[i].fFrame)
			continue;
		const STAGE_REVEAL_KEY& From = Keys[i - 1];
		const STAGE_REVEAL_KEY& To = Keys[i];
		const f32_t fSpan = To.fFrame - From.fFrame;
		const f32_t fT = fSpan > 0.f ? (fFrame - From.fFrame) / fSpan : 0.f;
		fOutAlpha = From.fAlpha + (To.fAlpha - From.fAlpha) * fT;
		fOutOffsetX = From.fOffsetX + (To.fOffsetX - From.fOffsetX) * fT;
		fOutOffsetY = From.fOffsetY + (To.fOffsetY - From.fOffsetY) * fT;
		return true;
	}
	return false;
}

namespace
{
	/* EFTable_Mvp.StatType, for the contributions the sample page carries. */
	constexpr int32_t MVP_STAT_DAMAGE = 1;
	constexpr int32_t MVP_STAT_STAGGER = 3;
	constexpr int32_t MVP_STAT_HEAL = 4;
	constexpr int32_t MVP_STAT_BATTLE_ITEM = 9;
	constexpr int32_t MVP_STAT_COUNTER = 11;
	constexpr int32_t MVP_STAT_SURVIVAL = 12;
	constexpr int32_t MVP_STAT_SUPPORT_DAMAGE = 13;

	/* EFTable_MvpMedalDescription indices the Server's facts can decide. */
	constexpr int32_t MVP_MEDAL_DODGE_MASTER = 2;      // damaging hits taken <= Param1
	constexpr int32_t MVP_MEDAL_NEAR_DEATH = 3;        // health <= Param1 % and standing at the clear
	constexpr int32_t MVP_MEDAL_FINISHER = 7;          // took a main target's last health
	constexpr int32_t MVP_MEDAL_COUNTER_SPECIALIST = 11; // two counters within Param1 seconds
	constexpr int32_t MVP_MEDAL_DEEP_ROOTED = 12;      // <= Param2 knockdowns over a Param1 ms fight
	constexpr int32_t MVP_MEDAL_HEAVY_SMASHER = 15;    // most part (destruction) damage

	/* The reference capture shows no guild line under any of the four names:
	   MvpResultFrame fills guildNameTF only when the character has a guild, so
	   the sample leaves it empty instead of printing a stand-in word. */
	Client::MVP_AWARD_PARTICIPANT Make_PreviewParticipant(
		const wchar_t* const pName,
		const char* const szNetworkClassId,
		vector<Client::MVP_AWARD_CONTRIBUTION> Contributions,
		vector<int32_t> Medals)
	{
		Client::MVP_AWARD_PARTICIPANT Participant;
		Participant.strCharacterName = pName;
		Participant.strNetworkClassId = szNetworkClassId;
		Participant.Contributions = std::move(Contributions);
		Participant.Medals = std::move(Medals);
		for (const Client::MVP_AWARD_CONTRIBUTION& Contribution
			: Participant.Contributions)
			Participant.fTotalScore += Contribution.fScore;
		return Participant;
	}
}

/* The shares, scores and medal requests below are made-up sample play. The
   sample deliberately gives two of the three columns the damage stat as their
   best contribution so the one-damage-title-per-page rule is visible: Berserker
   takes it and Sorceress falls through to the battle-item title. Medal 16 is
   requested and dropped where the group cannot award it. */
Client::MVP_RESULT_DATA Client::CMvpAwardCatalog::Build_PreviewPage(
	const int32_t iRaidGroupId, const int32_t iGate, const char* const szDifficultyId,
	const int32_t iPartySize) const
{
	const vector<MVP_AWARD_PARTICIPANT> Participants = {
		Make_PreviewParticipant(L"Test", "LANCE_MASTER",
			{ { MVP_STAT_DAMAGE, 4250.f, 42.5f, L"42.5%" },
			  { MVP_STAT_STAGGER, 1655.f, 33.1f, L"33.1%" },
			  { MVP_STAT_COUNTER, 248.f, 24.8f, L"11" } },
			{ 1, 9, 13 }),
		Make_PreviewParticipant(L"Berserker", "WARLORD",
			{ { MVP_STAT_DAMAGE, 2830.f, 28.3f, {} },
			  { MVP_STAT_STAGGER, 1530.f, 30.6f, {} } },
			{ 2, 9 }),
		Make_PreviewParticipant(L"Bard", "ARTIST",
			{ { MVP_STAT_SUPPORT_DAMAGE, 2260.f, 22.6f, {} },
			  { MVP_STAT_HEAL, 1230.f, 41.0f, {} } },
			{ 14, 16, 17 }),
		Make_PreviewParticipant(L"Sorceress", "DIMENSIONMASTER",
			{ { MVP_STAT_DAMAGE, 1520.f, 15.2f, {} },
			  { MVP_STAT_BATTLE_ITEM, 210.f, 21.0f, {} } },
			{ 5 }),
	};
	return Compose_Page(
		Build_ContentName(iRaidGroupId, iGate, szDifficultyId),
		Participants, iPartySize);
}

namespace
{
	/* MvpClassSymbols.json keys its rows by CHARACTER_CLASS_ID name. */
	const char* Network_ClassId(const LostArk::Shared::CHARACTER_CLASS_ID eClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (eClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER: return "LANCE_MASTER";
		case CHARACTER_CLASS_ID::GUNSLINGER: return "GUNSLINGER";
		case CHARACTER_CLASS_ID::SLAYER: return "SLAYER";
		case CHARACTER_CLASS_ID::ARTIST: return "ARTIST";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "DIMENSIONMASTER";
		case CHARACTER_CLASS_ID::WARLORD: return "WARLORD";
		case CHARACTER_CLASS_ID::GUARDIANKNIGHT: return "GUARDIANKNIGHT";
		default: return "";
		}
	}

	/* The retail capture prints the MVP card's value as the party share, a whole
	   percent ("51%", "24%"), for damage and stagger alike; the party columns have no
	   value field at all. */
	wstring_t Format_Percent(const f32_t fPercent)
	{
		wchar_t szText[32] = {};
		swprintf_s(szText, L"%.0f%%", fPercent);
		return szText;
	}

	f32_t Share_Percent(const std::uint64_t iPart, const std::uint64_t iTotal)
	{
		return 0u == iTotal ? 0.f :
			static_cast<f32_t>(static_cast<f64_t>(iPart) * 100.0 / static_cast<f64_t>(iTotal));
	}
}

Client::MVP_RESULT_DATA Client::CMvpAwardCatalog::Build_ServerPage(
	const int32_t iRaidGroupId, const char* const szDifficultyId, const int32_t iPartySize,
	const LostArk::Shared::S2C_RAID_MVP_RESULT& Result,
	vector<LostArk::Shared::PLAYER_ID>& outStagePlayerIds) const
{
	outStagePlayerIds.clear();
	std::uint64_t iTotalDamage = 0u, iTotalStagger = 0u, iTotalCounters = 0u;
	for (const LostArk::Shared::RAID_MVP_PARTICIPANT& Row : Result.Participants)
	{
		iTotalDamage += Row.iDamage;
		iTotalStagger += Row.iStagger;
		iTotalCounters += Row.iCounterCount;
	}

	const auto Make_Contribution = [](const int32_t iStatType, const f32_t fShare)
	{
		MVP_AWARD_CONTRIBUTION Contribution;
		Contribution.iStatType = iStatType;
		Contribution.fScore = fShare;
		Contribution.fSharePercent = fShare;
		Contribution.strValue = Format_Percent(fShare);
		return Contribution;
	};

	std::uint64_t iMostPartDamage = 0u;
	for (const LostArk::Shared::RAID_MVP_PARTICIPANT& Row : Result.Participants)
		iMostPartDamage = (std::max)(iMostPartDamage, Row.iPartDamage);

	/* Medals are achievements of their own, not a reading of the titles. The group
	   (Find_Medal) supplies each one's Param1. */
	const auto Earned_Medals = [this, iMostPartDamage](
		const LostArk::Shared::RAID_MVP_PARTICIPANT& Row)
	{
		vector<int32_t> Medals;
		const MVP_AWARD_MEDAL* pDodge = Find_Medal(MVP_MEDAL_DODGE_MASTER);
		if (nullptr != pDodge && 0u != Row.iFightTicks &&
			Row.iDamagingHitsTaken <= static_cast<std::uint32_t>((std::max)(pDodge->iParam1, 0)))
			Medals.push_back(MVP_MEDAL_DODGE_MASTER);
		const MVP_AWARD_MEDAL* pNearDeath = Find_Medal(MVP_MEDAL_NEAR_DEATH);
		if (nullptr != pNearDeath && Row.bAliveAtClear &&
			Row.iLowestHpPermille <= static_cast<std::uint32_t>((std::max)(pNearDeath->iParam1, 0)) * 10u)
			Medals.push_back(MVP_MEDAL_NEAR_DEATH);
		if (nullptr != Find_Medal(MVP_MEDAL_FINISHER) && 0u != Row.iFinishingBlows)
			Medals.push_back(MVP_MEDAL_FINISHER);
		const MVP_AWARD_MEDAL* pCounter = Find_Medal(MVP_MEDAL_COUNTER_SPECIALIST);
		if (nullptr != pCounter && 0u != Row.iMinCounterGapMs &&
			Row.iMinCounterGapMs <= static_cast<std::uint32_t>((std::max)(pCounter->iParam1, 0)) * 1000u)
			Medals.push_back(MVP_MEDAL_COUNTER_SPECIALIST);
		const MVP_AWARD_MEDAL* pDeepRooted = Find_Medal(MVP_MEDAL_DEEP_ROOTED);
		if (nullptr != pDeepRooted &&
			Row.iFightMs >= static_cast<std::uint32_t>((std::max)(pDeepRooted->iParam1, 0)) &&
			Row.iKnockdowns <= static_cast<std::uint32_t>((std::max)(pDeepRooted->iParam2, 0)))
			Medals.push_back(MVP_MEDAL_DEEP_ROOTED);
		if (nullptr != Find_Medal(MVP_MEDAL_HEAVY_SMASHER) &&
			0u != Row.iPartDamage && Row.iPartDamage == iMostPartDamage)
			Medals.push_back(MVP_MEDAL_HEAVY_SMASHER);
		return Medals;
	};

	vector<MVP_AWARD_PARTICIPANT> Participants;
	Participants.reserve(Result.Participants.size());
	for (const LostArk::Shared::RAID_MVP_PARTICIPANT& Row : Result.Participants)
	{
		MVP_AWARD_PARTICIPANT Participant;
		Participant.Medals = Earned_Medals(Row);
		(void)Convert_Utf8ToWide(Row.strNickname, Participant.strCharacterName);
		Participant.strNetworkClassId = Network_ClassId(Row.eCharacterClass);
		Participant.Contributions.push_back(Make_Contribution(
			MVP_STAT_DAMAGE, Share_Percent(Row.iDamage, iTotalDamage)));
		Participant.Contributions.push_back(Make_Contribution(
			MVP_STAT_STAGGER, Share_Percent(Row.iStagger, iTotalStagger)));
		Participant.Contributions.push_back(Make_Contribution(
			MVP_STAT_COUNTER, Share_Percent(Row.iCounterCount, iTotalCounters)));
		/* Survival is a yes/no title, so it never outranks a share row on its card. */
		if (0u != Row.iFightTicks && Row.iAliveTicks == Row.iFightTicks)
		{
			MVP_AWARD_CONTRIBUTION Survival;
			Survival.iStatType = MVP_STAT_SURVIVAL;
			Survival.strValue = Format_Percent(100.f);
			Participant.Contributions.push_back(Survival);
		}
		/* The MVP is the largest share of the "core" contributions; a "support" share
		   only settles a tie, so it is scaled far below any core difference. */
		for (const MVP_AWARD_CONTRIBUTION& Contribution : Participant.Contributions)
		{
			const MVP_AWARD_STAT* pStat = Find_Stat(Contribution.iStatType);
			if (nullptr == pStat)
				continue;
			if ("core" == pStat->strScoreRole)
				Participant.fTotalScore += Contribution.fSharePercent;
			else if ("support" == pStat->strScoreRole)
				Participant.fTotalScore += Contribution.fSharePercent * 0.001f;
		}
		Participants.push_back(std::move(Participant));
	}

	vector<size_t> RankedIndices;
	MVP_RESULT_DATA Data = Compose_Page(
		Build_ContentName(iRaidGroupId, Result.iGate, szDifficultyId),
		Participants, iPartySize, &RankedIndices);
	for (const size_t iIndex : RankedIndices)
		outStagePlayerIds.push_back(Result.Participants[iIndex].iPlayerId);
	return Data;
}

wstring_t Client::CMvpAwardCatalog::Find_RaidName(const int32_t iRaidGroupId) const
{
	const auto it = std::find_if(m_Raids.begin(), m_Raids.end(),
		[iRaidGroupId](const CONTENT_NAME_PIECE& Piece) { return Piece.iKey == iRaidGroupId; });
	return m_Raids.end() != it ? it->strText : wstring_t();
}

wstring_t Client::CMvpAwardCatalog::Find_DifficultyText(const char* const szDifficultyId) const
{
	if (nullptr == szDifficultyId)
		return wstring_t();
	const auto it = std::find_if(m_Difficulties.begin(), m_Difficulties.end(),
		[szDifficultyId](const CONTENT_NAME_PIECE& Piece) { return Piece.strId == szDifficultyId; });
	return m_Difficulties.end() != it ? it->strText : wstring_t();
}
