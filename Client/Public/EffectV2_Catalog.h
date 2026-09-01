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
	std::vector<std::string> m_Diagnostics;
	/* False when one or more rows from the single bindings source were
	   isolated. Typed mutation must preserve that source instead of silently
	   serializing only the valid subset. */
	bool_t m_bBossValtanBindingsComplete = true;
};

/* Stable authoring identity for one Server-stage Effect V2 binding.  The
   binding document has no ordinal identity, so mutation carries the complete
   persisted row baseline and rejects a stale or ambiguous match. */
struct EFFECT_V2_STAGE_BINDING_KEY final
{
	[[nodiscard]] static EFFECT_V2_STAGE_BINDING_KEY From_Binding(
		const EFFECT_V2_BINDING& Binding);
	[[nodiscard]] static EFFECT_V2_STAGE_BINDING_KEY From_StageBinding(
		const EFFECT_V2_BINDING& Binding);

	std::string strResourceId;
	bool_t bGroup = false;
	std::string strStageActionId;
	std::string strClipName;
	uint32_t iStartMs = 0u;
	std::string strBone;
	bool_t bFollowBone = false;
	CEffectV2Object::PIVOT_ROTATION eRotation =
		CEffectV2Object::PIVOT_ROTATION::TARGET_YAW;
	bool_t bStopWithClip = false;
	float3_t vOffset = { 0.f, 0.f, 0.f };
	f32_t fYawDegrees = 0.f;
};

/* Main-thread authoring catalog. Reload_BossValtan is the explicit full-read
   boundary; typed binding mutations are the only write boundary. Each source
   item remains strict, while a malformed document/group/binding row is
   isolated so the valid subset can atomically replace the immutable snapshot.
   Fatal directory/read-set failure preserves the previous snapshot revision. */
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
	/* Writes one exact Server-stage binding into the BOSS_VALTAN authoring
	   owner.  The candidate is cross-validated and serialized/parsed before
	   the atomic file replacement; success also commits the matching immutable
	   in-memory snapshot so the Workbench can render it without another scan. */
	bool_t Append_BossValtanStageBinding(
		const std::string& strResourceId,
		bool_t bGroup,
		const std::string& strStageActionId,
		uint32_t iStartMs,
		std::string& strOutError);
	bool_t Remove_BossValtanStageBinding(
		const EFFECT_V2_STAGE_BINDING_KEY& Key,
		std::string& strOutError);
	bool_t Duplicate_BossValtanStageBinding(
		const EFFECT_V2_STAGE_BINDING_KEY& SourceKey,
		uint32_t iDuplicateStartMs,
		std::string& strOutError);
	bool_t Update_BossValtanStageBindingStart(
		const EFFECT_V2_STAGE_BINDING_KEY& SourceKey,
		uint32_t iNewStartMs,
		std::string& strOutError);
	/* Action Composition stages V2 bindings in the immutable catalog snapshot
	   so the Sequencer and local preview see the draft immediately, but the
	   typed owner is not written until the Workbench's single Save transaction
	   commits Pattern, Sound and V2 together. */
	bool_t Stage_AppendBossValtanStageBinding(
		const std::string& strResourceId,
		bool_t bGroup,
		const std::string& strStageActionId,
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
		bool_t bWriteOwner,
		std::string& strOutError);
	bool_t Mutate_BossValtanStageBinding(
		const EFFECT_V2_STAGE_BINDING_KEY& SourceKey,
		uint32_t iTargetStartMs,
		BOSS_VALTAN_BINDING_MUTATION eMutation,
		bool_t bWriteOwner,
		std::string& strOutError);

private:
	mutable std::mutex m_SnapshotMutex;
	std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> m_pSnapshot;
	std::string m_strBossValtanBindingDraftBaselineBytes;
	bool_t m_bBossValtanBindingDraftDirty = false;
};

NS_END
