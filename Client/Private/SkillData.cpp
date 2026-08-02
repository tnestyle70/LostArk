#include "SkillData.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace
{
	using namespace Client;

	/* win= tokens, in SKILL_WINDOW order. extract_action_loa.py writes these. */
	const char_t* WindowTokens[ETOUI(SKILL_WINDOW::END)] =
	{
		"MOVE_PRE",
		"SKILL_PRE",
		"COMBO_PRE",
		"MOVE_CANCEL",
		"SKILL_CANCEL",
		"COMBO_CANCEL",
		"DODGE_CANCEL",
		"ANY_CANCEL",
		"COMBO_INPUT",
	};

	/* END for NONE, OTHER and anything a newer extractor adds. An unknown token is
	not an error: the row simply carries no window this build understands. */
	SKILL_WINDOW To_Window(const char_t* pToken)
	{
		for (uint32_t i = 0; i < ETOUI(SKILL_WINDOW::END); ++i)
		{
			const size_t iLength = strlen(WindowTokens[i]);
			if (0 == strncmp(pToken, WindowTokens[i], iLength) &&
				(' ' == pToken[iLength] || '\r' == pToken[iLength] ||
					'\n' == pToken[iLength] || '\0' == pToken[iLength]))
			{
				return static_cast<SKILL_WINDOW>(i);
			}
		}
		return SKILL_WINDOW::END;
	}

	/* Value of "<key>=" as a float, or fDefault when the key is absent. */
	f32_t Read_Float(const char_t* pLine, const char_t* pKey, f32_t fDefault)
	{
		const char_t* p = strstr(pLine, pKey);
		return nullptr != p ?
			static_cast<f32_t>(atof(p + strlen(pKey))) : fDefault;
	}

	int32_t Read_Int(const char_t* pLine, const char_t* pKey, int32_t iDefault)
	{
		const char_t* p = strstr(pLine, pKey);
		return nullptr != p ? atoi(p + strlen(pKey)) : iDefault;
	}

	/* Contents of the first quoted run, which every clip header opens with. */
	bool_t Read_Quoted(const char_t* pLine, std::string& strOut)
	{
		const char_t* pOpen = strchr(pLine, '\"');
		if (nullptr == pOpen)
			return false;
		const char_t* pClose = strchr(pOpen + 1, '\"');
		if (nullptr == pClose)
			return false;
		strOut.assign(pOpen + 1, pClose);
		return true;
	}
}

NS_BEGIN(Client)

/* Reads <asset>.animnotify: the per-clip timeline the game itself authored.
Only the input windows are kept -- effects, sound and shake belong to the effect
pass, not to whether a chain may be cut short. */
bool_t CSkillData::Load_Notify(const std::string& strPath)
{
	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, strPath.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[4096]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile) ||
		nullptr == strstr(szLine, "LOSTARK_ANIM_NOTIFY"))
	{
		fclose(pFile);
		return false;
	}

	CLIP* pCurrent = nullptr;
	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		/* A clip header starts at column 0 with a quoted name; its rows are
		indented and start with "n". */
		if ('\"' == szLine[0])
		{
			std::string strClip;
			if (!Read_Quoted(szLine, strClip))
			{
				pCurrent = nullptr;
				continue;
			}
			pCurrent = &m_Clips[strClip];
			pCurrent->fLength = Read_Float(szLine, "len=", 0.f);
			continue;
		}

		if (nullptr == pCurrent)
			continue;

		const char_t* pWin = strstr(szLine, "win=");
		if (nullptr == pWin)
			continue;

		const SKILL_WINDOW eKind = To_Window(pWin + 4);
		if (SKILL_WINDOW::END == eKind)
			continue;

		WINDOW window{};
		window.eKind = eKind;
		window.fStart = Read_Float(szLine, "t=", 0.f);
		window.fEnd = window.fStart + Read_Float(szLine, "d=", 0.f);
		pCurrent->Windows.push_back(window);
	}

	fclose(pFile);
	return true;
}

/* Reads <asset>.skilltiming: cooldown and the moments a skill lands.

Rows are keyed by base=, the skill the tripod variants belong to, because that is
the id a clip chain carries. Where several variants share a base the one whose
own id equals the base wins; it is the untripoded skill. */
bool_t CSkillData::Load_Timing(const std::string& strPath)
{
	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, strPath.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[4096]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile) ||
		nullptr == strstr(szLine, "LOSTARK_SKILL_TIMING"))
	{
		fclose(pFile);
		return false;
	}

	SKILL* pCurrent = nullptr;
	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		if (' ' != szLine[0] && '\t' != szLine[0])
		{
			const int32_t iSkillId = atoi(szLine);
			if (0 == iSkillId)
			{
				pCurrent = nullptr;
				continue;
			}

			const int32_t iBaseId = Read_Int(szLine, "base=", iSkillId);
			const bool_t isPrimary = iSkillId == iBaseId;
			const auto it = m_Skills.find(iBaseId);
			if (m_Skills.end() != it && !isPrimary)
			{
				/* A variant of a base already stored; keep the one we have. */
				pCurrent = nullptr;
				continue;
			}

			pCurrent = &m_Skills[iBaseId];
			pCurrent->iCooldownMs = Read_Int(szLine, "cd=", 0);
			pCurrent->Hits.clear();
			continue;
		}

		/* v1 files carry no hit rows at all, which is not an error. */
		if (nullptr == pCurrent || nullptr == strstr(szLine, "hit "))
			continue;

		HIT hit{};
		hit.iTimeMs = Read_Int(szLine, " t=", 0);
		hit.iWidthMs = Read_Int(szLine, " w=", 0);
		pCurrent->Hits.push_back(hit);
	}

	fclose(pFile);
	return true;
}

const CSkillData::CLIP* CSkillData::Find_Clip(const char_t* pClipName) const
{
	if (nullptr == pClipName)
		return nullptr;
	const auto it = m_Clips.find(pClipName);
	return m_Clips.end() != it ? &it->second : nullptr;
}

const CSkillData::SKILL* CSkillData::Find_Skill(int32_t iSkillId) const
{
	const auto it = m_Skills.find(iSkillId);
	return m_Skills.end() != it ? &it->second : nullptr;
}

bool_t CSkillData::Is_InWindow(
	const CLIP* pClip, SKILL_WINDOW eKind, f32_t fClipTime) const
{
	if (nullptr == pClip)
		return false;

	for (const WINDOW& window : pClip->Windows)
	{
		if (window.eKind == eKind &&
			fClipTime >= window.fStart && fClipTime <= window.fEnd)
		{
			return true;
		}
	}
	return false;
}

shared_ptr<const CSkillData> CSkillData::Load(const char_t* pAssetName)
{
	static std::unordered_map<std::string, shared_ptr<const CSkillData>> Cache;

	const std::string strAsset = nullptr != pAssetName ? pAssetName : "";
	const auto it = Cache.find(strAsset);
	if (Cache.end() != it)
		return it->second;

	auto pData = make_shared<CSkillData>();
	if (!strAsset.empty())
	{
		const std::string strBase = "../Bin/DataFiles/Anim/" + strAsset;
		pData->Load_Notify(strBase + ".animnotify");
		pData->Load_Timing(strBase + ".skilltiming");
	}

	Cache.emplace(strAsset, pData);
	return pData;
}

NS_END
