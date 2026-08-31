#include "EffectV2_Catalog.h"

#include <algorithm>
#include <exception>
#include <filesystem>
#include <limits>
#include <map>
#include <utility>

namespace
{
	constexpr const char* BOSS_VALTAN_ARCHETYPE_ID = "BOSS_VALTAN";
	constexpr std::string_view DOCUMENT_SUFFIX = ".effectv2.json";
	constexpr std::string_view GROUP_SUFFIX = ".effectv2group.json";

	bool_t Fail(std::string& strOutError, std::string strMessage)
	{
		strOutError = std::move(strMessage);
		return false;
	}

	bool_t Enumerate_StableIds(
		const std::filesystem::path& Directory,
		const std::string_view Suffix,
		std::vector<std::string>& OutIds,
		std::string& strOutError)
	{
		OutIds.clear();
		std::error_code Error;
		if (Directory.empty() ||
			!std::filesystem::is_directory(Directory, Error) || Error)
		{
			return Fail(strOutError,
				"Effect V2 catalog directory is unavailable: " +
				Directory.string());
		}

		std::filesystem::directory_iterator Iterator(
			Directory, std::filesystem::directory_options::none, Error);
		if (Error)
		{
			return Fail(strOutError,
				"Effect V2 catalog directory could not be opened: " +
				Directory.string());
		}
		const std::filesystem::directory_iterator End;
		for (; Iterator != End; Iterator.increment(Error))
		{
			if (Error)
			{
				return Fail(strOutError,
					"Effect V2 catalog directory enumeration failed: " +
					Directory.string());
			}

			const std::filesystem::directory_entry& Entry = *Iterator;
			const std::string strFileName = Entry.path().filename().string();
			if (!strFileName.ends_with(Suffix))
				continue;

			const std::filesystem::file_status Status =
				Entry.symlink_status(Error);
			if (Error || !std::filesystem::is_regular_file(Status))
			{
				return Fail(strOutError,
					"Effect V2 catalog source must be a regular file: " +
					Entry.path().string());
			}

			const std::string strId = strFileName.substr(
				0u, strFileName.size() - Suffix.size());
			if (!Client::CEffectV2Document::Is_ValidEffectId(strId))
			{
				return Fail(strOutError,
					"Effect V2 catalog source has an invalid stable ID: " +
					strFileName);
			}
			OutIds.push_back(strId);
		}
		if (Error)
		{
			return Fail(strOutError,
				"Effect V2 catalog directory enumeration failed: " +
				Directory.string());
		}

		std::sort(OutIds.begin(), OutIds.end());
		if (std::adjacent_find(OutIds.begin(), OutIds.end()) != OutIds.end())
		{
			return Fail(strOutError,
				"Effect V2 catalog contains a duplicate stable ID in: " +
				Directory.string());
		}
		return true;
	}

	bool_t Stage_Documents(
		std::vector<Client::EFFECT_V2_DOCUMENT>& OutDocuments,
		std::string& strOutError)
	{
		std::vector<std::string> Ids;
		if (!Enumerate_StableIds(
			Client::CEffectV2Document::Document_Directory(),
			DOCUMENT_SUFFIX, Ids, strOutError))
		{
			return false;
		}
		if (Ids.empty())
			return Fail(strOutError, "Effect V2 authored catalog is empty.");

		std::vector<Client::EFFECT_V2_DOCUMENT> Staged;
		Staged.reserve(Ids.size());
		for (const std::string& strEffectId : Ids)
		{
			Client::EFFECT_V2_DOCUMENT Document;
			if (!Client::CEffectV2Document::Load_DocumentFile(
				strEffectId, Document, strOutError))
			{
				strOutError = "Effect V2 document '" + strEffectId +
					"' was rejected: " + strOutError;
				return false;
			}
			Staged.push_back(std::move(Document));
		}
		OutDocuments = std::move(Staged);
		return true;
	}

	bool_t Stage_Groups(
		std::vector<Client::EFFECT_V2_GROUP>& OutGroups,
		std::string& strOutError)
	{
		std::vector<std::string> Ids;
		if (!Enumerate_StableIds(
			Client::CEffectV2Document::Group_Directory(),
			GROUP_SUFFIX, Ids, strOutError))
		{
			return false;
		}

		std::vector<Client::EFFECT_V2_GROUP> Staged;
		Staged.reserve(Ids.size());
		for (const std::string& strGroupId : Ids)
		{
			Client::EFFECT_V2_GROUP Group;
			if (!Client::CEffectV2Document::Load_GroupFile(
				strGroupId, Group, strOutError))
			{
				strOutError = "Effect V2 group '" + strGroupId +
					"' was rejected: " + strOutError;
				return false;
			}
			Staged.push_back(std::move(Group));
		}
		OutGroups = std::move(Staged);
		return true;
	}

	bool_t Cross_Validate(
		const std::vector<Client::EFFECT_V2_DOCUMENT>& Documents,
		const std::vector<Client::EFFECT_V2_GROUP>& Groups,
		const std::vector<Client::EFFECT_V2_BINDING>& Bindings,
		std::string& strOutError)
	{
		std::map<std::string, const Client::EFFECT_V2_DOCUMENT*, std::less<>>
			DocumentsById;
		for (const Client::EFFECT_V2_DOCUMENT& Document : Documents)
		{
			if (!DocumentsById.emplace(Document.strEffectId, &Document).second)
			{
				return Fail(strOutError,
					"Effect V2 catalog has duplicate effectId: " +
					Document.strEffectId);
			}
		}

		std::map<std::string, const Client::EFFECT_V2_GROUP*, std::less<>> GroupsById;
		for (const Client::EFFECT_V2_GROUP& Group : Groups)
		{
			if (DocumentsById.contains(Group.strGroupId))
			{
				return Fail(strOutError,
					"Effect V2 groupId collides with an authored effect: " +
					Group.strGroupId);
			}
			if (!GroupsById.emplace(Group.strGroupId, &Group).second)
			{
				return Fail(strOutError,
					"Effect V2 catalog has duplicate groupId: " +
					Group.strGroupId);
			}
			for (const Client::EFFECT_V2_GROUP_CHILD& Child : Group.Children)
			{
				if (!DocumentsById.contains(Child.strEffectId))
				{
					return Fail(strOutError,
						"Effect V2 group child has no authored leaf document: " +
						Group.strGroupId + " -> " + Child.strEffectId);
				}
			}
		}

		for (const Client::EFFECT_V2_BINDING& Binding : Bindings)
		{
			if (Binding.strEffectId.empty() == Binding.strGroupId.empty() ||
				Binding.strClip.empty() == Binding.strStage.empty())
			{
				return Fail(strOutError,
					"BOSS_VALTAN Effect V2 binding lost its exact subject or clock owner.");
			}
			if (!Binding.strEffectId.empty())
			{
				if (!DocumentsById.contains(Binding.strEffectId))
				{
					return Fail(strOutError,
						"BOSS_VALTAN Effect V2 binding has no authored document: " +
						Binding.strEffectId);
				}
			}
			else if (!GroupsById.contains(Binding.strGroupId))
			{
				return Fail(strOutError,
					"BOSS_VALTAN Effect V2 binding has no group document: " +
					Binding.strGroupId);
			}
		}
		return true;
	}
}

uint64_t Client::EFFECT_V2_CATALOG_SNAPSHOT::Get_Revision() const noexcept
{
	return m_iRevision;
}

bool_t Client::EFFECT_V2_CATALOG_SNAPSHOT::Is_Ready() const noexcept
{
	return 0u != m_iRevision;
}

const std::vector<Client::EFFECT_V2_DOCUMENT>&
Client::EFFECT_V2_CATALOG_SNAPSHOT::Get_Documents() const noexcept
{
	return m_Documents;
}

const std::vector<Client::EFFECT_V2_GROUP>&
Client::EFFECT_V2_CATALOG_SNAPSHOT::Get_Groups() const noexcept
{
	return m_Groups;
}

const std::vector<Client::EFFECT_V2_BINDING>&
Client::EFFECT_V2_CATALOG_SNAPSHOT::Get_BossValtanBindings() const noexcept
{
	return m_BossValtanBindings;
}

const Client::EFFECT_V2_DOCUMENT*
Client::EFFECT_V2_CATALOG_SNAPSHOT::Find_Document(
	const std::string_view strEffectId) const noexcept
{
	const auto Found = std::lower_bound(
		m_Documents.begin(), m_Documents.end(), strEffectId,
		[](const EFFECT_V2_DOCUMENT& Document, const std::string_view Id)
		{
			return Document.strEffectId < Id;
		});
	return Found != m_Documents.end() && Found->strEffectId == strEffectId ?
		&*Found : nullptr;
}

const Client::EFFECT_V2_GROUP*
Client::EFFECT_V2_CATALOG_SNAPSHOT::Find_Group(
	const std::string_view strGroupId) const noexcept
{
	const auto Found = std::lower_bound(
		m_Groups.begin(), m_Groups.end(), strGroupId,
		[](const EFFECT_V2_GROUP& Group, const std::string_view Id)
		{
			return Group.strGroupId < Id;
		});
	return Found != m_Groups.end() && Found->strGroupId == strGroupId ?
		&*Found : nullptr;
}

Client::CEffectV2Catalog& Client::CEffectV2Catalog::Get()
{
	static CEffectV2Catalog Instance;
	return Instance;
}

Client::CEffectV2Catalog::CEffectV2Catalog()
	: m_pSnapshot(std::make_shared<const EFFECT_V2_CATALOG_SNAPSHOT>())
{
}

bool_t Client::CEffectV2Catalog::Reload_BossValtan(std::string& strOutError)
{
	try
	{
		std::vector<EFFECT_V2_DOCUMENT> StagedDocuments;
		std::vector<EFFECT_V2_GROUP> StagedGroups;
		std::vector<EFFECT_V2_BINDING> StagedBindings;
		if (!Stage_Documents(StagedDocuments, strOutError) ||
			!Stage_Groups(StagedGroups, strOutError) ||
			!CEffectV2Document::Load_BindingsFile(
				BOSS_VALTAN_ARCHETYPE_ID, StagedBindings, strOutError) ||
			!Cross_Validate(
				StagedDocuments, StagedGroups, StagedBindings, strOutError))
		{
			return false;
		}

		auto pStaged = std::make_shared<EFFECT_V2_CATALOG_SNAPSHOT>();
		pStaged->m_Documents = std::move(StagedDocuments);
		pStaged->m_Groups = std::move(StagedGroups);
		pStaged->m_BossValtanBindings = std::move(StagedBindings);

		{
			const std::lock_guard Lock(m_SnapshotMutex);
			const uint64_t iPreviousRevision =
				nullptr != m_pSnapshot ? m_pSnapshot->Get_Revision() : 0u;
			if ((std::numeric_limits<uint64_t>::max)() == iPreviousRevision)
			{
				return Fail(strOutError,
					"Effect V2 catalog revision is exhausted.");
			}
			pStaged->m_iRevision = iPreviousRevision + 1u;
			m_pSnapshot = std::move(pStaged);
		}
		strOutError.clear();
		return true;
	}
	catch (const std::exception& Exception)
	{
		return Fail(strOutError,
			"Effect V2 catalog reload failed before commit: " +
			std::string(Exception.what()));
	}
	catch (...)
	{
		return Fail(strOutError,
			"Effect V2 catalog reload failed before commit.");
	}
}

bool_t Client::CEffectV2Catalog::Append_BossValtanStageBinding(
	const std::string& strResourceId,
	const bool_t bGroup,
	const std::string& strStageActionId,
	const uint32_t iStartMs,
	std::string& strOutError)
{
	if (!CEffectV2Document::Is_ValidEffectId(strResourceId))
		return Fail(strOutError,
			"Effect V2 append rejected an invalid stable resource ID.");
	if (strStageActionId.empty() || strStageActionId.size() > 160u)
		return Fail(strOutError,
			"Effect V2 append requires one stable Server Stage actionId.");
	if (iStartMs > 600000u)
		return Fail(strOutError,
			"Effect V2 append startMs exceeds the ten-minute authoring limit.");

	try
	{
		const std::lock_guard Lock(m_SnapshotMutex);
		if (nullptr == m_pSnapshot || !m_pSnapshot->Is_Ready())
		{
			return Fail(strOutError,
				"Reload the Effect V2 catalog before appending a binding.");
		}
		if ((std::numeric_limits<uint64_t>::max)() ==
			m_pSnapshot->Get_Revision())
		{
			return Fail(strOutError,
				"Effect V2 catalog revision is exhausted.");
		}
		if (bGroup)
		{
			if (nullptr == m_pSnapshot->Find_Group(strResourceId))
			{
				return Fail(strOutError,
					"Effect V2 append group is not in the admitted catalog: " +
					strResourceId);
			}
		}
		else if (nullptr == m_pSnapshot->Find_Document(strResourceId))
		{
			return Fail(strOutError,
				"Effect V2 append leaf is not in the admitted catalog: " +
				strResourceId);
		}

		auto pCandidate = std::make_shared<EFFECT_V2_CATALOG_SNAPSHOT>();
		pCandidate->m_Documents = m_pSnapshot->m_Documents;
		pCandidate->m_Groups = m_pSnapshot->m_Groups;
		pCandidate->m_BossValtanBindings =
			m_pSnapshot->m_BossValtanBindings;

		const auto Duplicate = std::find_if(
			pCandidate->m_BossValtanBindings.begin(),
			pCandidate->m_BossValtanBindings.end(),
			[&strResourceId, bGroup, &strStageActionId, iStartMs](
				const EFFECT_V2_BINDING& Binding)
			{
				return Binding.strStage == strStageActionId &&
					Binding.iStartMs == iStartMs &&
					(bGroup ? Binding.strGroupId : Binding.strEffectId) ==
						strResourceId;
			});
		if (Duplicate != pCandidate->m_BossValtanBindings.end())
		{
			return Fail(strOutError,
				"Effect V2 binding already exists at this Stage clock.");
		}

		EFFECT_V2_BINDING Binding;
		if (bGroup)
			Binding.strGroupId = strResourceId;
		else
			Binding.strEffectId = strResourceId;
		Binding.strStage = strStageActionId;
		Binding.iStartMs = iStartMs;
		Binding.bFollowBone = false;
		Binding.eRotation = CEffectV2Object::PIVOT_ROTATION::TARGET_YAW;
		pCandidate->m_BossValtanBindings.push_back(std::move(Binding));
		if (!Cross_Validate(
				pCandidate->m_Documents, pCandidate->m_Groups,
				pCandidate->m_BossValtanBindings, strOutError))
		{
			return false;
		}

		const std::string strSerialized = CEffectV2Document::Serialize_Bindings(
			BOSS_VALTAN_ARCHETYPE_ID, pCandidate->m_BossValtanBindings);
		std::vector<EFFECT_V2_BINDING> ParsedBindings;
		if (!CEffectV2Document::Parse_Bindings(
				strSerialized, BOSS_VALTAN_ARCHETYPE_ID,
				ParsedBindings, strOutError) ||
			!Cross_Validate(
				pCandidate->m_Documents, pCandidate->m_Groups,
				ParsedBindings, strOutError))
		{
			strOutError = "Effect V2 binding self-validation failed: " +
				strOutError;
			return false;
		}
		pCandidate->m_BossValtanBindings = std::move(ParsedBindings);
		if (!CEffectV2Document::Write_AtomicFile(
				CEffectV2Document::Binding_Path(BOSS_VALTAN_ARCHETYPE_ID),
				strSerialized, strOutError))
		{
			strOutError = "Effect V2 binding save failed before commit: " +
				strOutError;
			return false;
		}

		pCandidate->m_iRevision = m_pSnapshot->Get_Revision() + 1u;
		m_pSnapshot = std::move(pCandidate);
		strOutError =
			"Saved Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json. "
			"Product publish + Server restart is required before product playback.";
		return true;
	}
	catch (const std::exception& Exception)
	{
		return Fail(strOutError,
			"Effect V2 binding append failed before commit: " +
			std::string(Exception.what()));
	}
	catch (...)
	{
		return Fail(strOutError,
			"Effect V2 binding append failed before commit.");
	}
}

std::shared_ptr<const Client::EFFECT_V2_CATALOG_SNAPSHOT>
Client::CEffectV2Catalog::Get_Snapshot() const
{
	const std::lock_guard Lock(m_SnapshotMutex);
	return m_pSnapshot;
}

uint64_t Client::CEffectV2Catalog::Get_Revision() const
{
	const std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot =
		Get_Snapshot();
	return nullptr != pSnapshot ? pSnapshot->Get_Revision() : 0u;
}
