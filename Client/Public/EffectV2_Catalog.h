#pragma once

#include "Client_Defines.h"
#include "EffectV2_Document.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

/* Raw resource identity captured by the explicit catalog Reload boundary.
   A Composition draft may later reference any admitted leaf/group, so the
   snapshot retains the whole valid catalog and filters the exact candidate
   closure only when Save is prepared. */
struct EFFECT_V2_RESOURCE_READ_ROW final
{
	EFFECT_V2_RESOURCE_KIND eKind = EFFECT_V2_RESOURCE_KIND::LEAF;
	std::string strResourceId;
	std::string strRepositoryPath;
	std::string strSha256;
};

/* Immutable authoring view used by tools after an explicit catalog reload.
   Consumers retain the shared snapshot for the whole UI operation; no query
   below touches the filesystem or the runtime's lazy playback caches. */
class EFFECT_V2_CATALOG_SNAPSHOT final
{
public:
	[[nodiscard]] uint64_t Get_Revision() const noexcept;
	[[nodiscard]] bool_t Is_Ready() const noexcept;
	[[nodiscard]] const std::vector<EFFECT_V2_DOCUMENT>&
		Get_Documents() const noexcept;
	[[nodiscard]] const std::vector<EFFECT_V2_GROUP>&
		Get_Groups() const noexcept;
	[[nodiscard]] const std::vector<EFFECT_V2_BINDING>&
		Get_BossValtanBindings() const noexcept;
	[[nodiscard]] const std::vector<std::string>&
		Get_Diagnostics() const noexcept;
	[[nodiscard]] bool_t Has_IsolatedItems() const noexcept;
	[[nodiscard]] bool_t Can_MutateBossValtanBindings() const noexcept;
	[[nodiscard]] const EFFECT_V2_DOCUMENT* Find_Document(
		std::string_view strEffectId) const noexcept;
	[[nodiscard]] const EFFECT_V2_GROUP* Find_Group(
		std::string_view strGroupId) const noexcept;

private:
	friend class CEffectV2Catalog;

	uint64_t m_iRevision = 0u;
	std::vector<EFFECT_V2_DOCUMENT> m_Documents;
	std::vector<EFFECT_V2_GROUP> m_Groups;
	std::vector<EFFECT_V2_BINDING> m_BossValtanBindings;
	std::vector<EFFECT_V2_RESOURCE_READ_ROW> m_ResourceReadRows;
	std::vector<std::string> m_Diagnostics;
	/* The canonical v2 bindings owner is admitted as one strict document.
	   False is retained as a compatibility guard for snapshots created before
	   that admission completes; a partially parsed bindings source is never
	   committed. */
	bool_t m_bBossValtanBindingsComplete = true;
};

/* Stable authoring identity for one Server-stage Effect V2 binding. Existing
   rows are addressed only by formatVersion 2 bindingId. The remaining fields
   carry the typed scope/resource request used when appending a new row. */
struct EFFECT_V2_STAGE_BINDING_KEY final
{
	[[nodiscard]] static EFFECT_V2_STAGE_BINDING_KEY From_Binding(
		const EFFECT_V2_BINDING& Binding);
	[[nodiscard]] static EFFECT_V2_STAGE_BINDING_KEY From_StageBinding(
		const EFFECT_V2_BINDING& Binding);

	std::string strBindingId;
	std::string strResourceId;
	bool_t bGroup = false;
	std::string strPatternId;
	std::string strStageId;
	std::string strActionId;
	uint32_t iStartMs = 0u;
};

/* Main-thread authoring catalog. Reload_BossValtan is the explicit full-read
   boundary; typed binding mutations are the only write boundary. Each source
   item remains strict. Malformed leaf/group files may be isolated, but the
   canonical BOSS_VALTAN binding document is admitted only as one complete
   formatVersion 2 owner. Any binding read/parse/cross-reference failure
   preserves the previous snapshot revision. */
class CEffectV2Catalog final
{
public:
	static CEffectV2Catalog& Get();

	bool_t Reload_BossValtan(std::string& strOutError);
	/* Explicit Workbench navigation boundary.  Unlike ordinary Reload, this
	   typed command may replace an unsaved BOSS_VALTAN binding draft after the
	   user chooses Discard; staging failure preserves the previous snapshot. */
	bool_t Discard_BossValtanBindingDraftAndReload(
		std::string& strOutError);
	/* Action Composition stages V2 bindings in the immutable catalog snapshot
	   so the Sequencer and local preview see the draft immediately, but the
	   typed owner is not written until the Workbench's single Save transaction
	   commits Pattern, Sound and V2 together. There is deliberately no public
	   immediate-write API for the BOSS_VALTAN owner. */
	bool_t Stage_AppendBossValtanStageBinding(
		const std::string& strResourceId,
		bool_t bGroup,
		const std::string& strPatternId,
		const std::string& strStageId,
		const std::string& strActionId,
		uint32_t iStartMs,
		std::string& strOutError);
	bool_t Stage_RemoveBossValtanStageBinding(
		const EFFECT_V2_STAGE_BINDING_KEY& Key,
		std::string& strOutError);
	bool_t Stage_DuplicateBossValtanStageBinding(
		const EFFECT_V2_STAGE_BINDING_KEY& SourceKey,
		uint32_t iDuplicateStartMs,
		std::string& strOutError);
	bool_t Stage_UpdateBossValtanStageBindingStart(
		const EFFECT_V2_STAGE_BINDING_KEY& SourceKey,
		uint32_t iNewStartMs,
		std::string& strOutError);
	bool_t Prepare_BossValtanBindingDraftSave(
		std::string& strOutBaselineBytes,
		std::string& strOutCandidateBytes,
		std::string& strOutResourceReadSetBytes,
		uint64_t& iOutDraftRevision,
		bool_t& bOutDirty,
		std::string& strOutError) const;
	bool_t Accept_BossValtanBindingDraftSave(
		uint64_t iExpectedDraftRevision,
		const std::string& strExpectedCandidateBytes,
		std::string& strOutError);
	[[nodiscard]] bool_t Has_BossValtanBindingDraft() const;
	[[nodiscard]] std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>
		Get_Snapshot() const;
	[[nodiscard]] uint64_t Get_Revision() const;

private:
	CEffectV2Catalog();

	CEffectV2Catalog(const CEffectV2Catalog&) = delete;
	CEffectV2Catalog& operator=(const CEffectV2Catalog&) = delete;

	enum class BOSS_VALTAN_BINDING_MUTATION : uint8_t
	{
		APPEND_BINDING,
		REMOVE_BINDING,
		DUPLICATE_BINDING,
		UPDATE_BINDING_START
	};

	bool_t Commit_BossValtanBindingsLocked(
		std::vector<EFFECT_V2_BINDING> CandidateBindings,
		const char* pOperation,
		std::string& strOutError);
	bool_t Mutate_BossValtanStageBinding(
		const EFFECT_V2_STAGE_BINDING_KEY& SourceKey,
		uint32_t iTargetStartMs,
		BOSS_VALTAN_BINDING_MUTATION eMutation,
		std::string& strOutError);

private:
	mutable std::mutex m_SnapshotMutex;
	std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> m_pSnapshot;
	std::string m_strBossValtanBindingDraftBaselineBytes;
	bool_t m_bBossValtanBindingDraftDirty = false;
};

NS_END
