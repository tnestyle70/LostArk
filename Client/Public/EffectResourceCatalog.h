#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

/* The owner is a dispatch detail.  Resource browsers render the stable ID and
   category labels from the descriptor rather than exposing storage versions. */
enum class EFFECT_RESOURCE_OWNER_KIND : uint8_t
{
	DIRECT_AUTHORED_DOCUMENT,
	TYPED_DOCUMENT,
	TYPED_GROUP,
	END
};

struct EFFECT_RESOURCE_KEY final
{
	EFFECT_RESOURCE_OWNER_KIND eOwnerKind =
		EFFECT_RESOURCE_OWNER_KIND::END;
	std::string strStableId;

	[[nodiscard]] bool_t Is_Valid() const noexcept;
	[[nodiscard]] bool_t operator==(const EFFECT_RESOURCE_KEY& Other) const
		noexcept;
};

struct EFFECT_RESOURCE_CAPABILITIES final
{
	bool_t bCanLoad = false;
	bool_t bCanAppendToStage = false;
	bool_t bCanAppendToClip = false;
	bool_t bCanSave = false;
	bool_t bCanReload = false;
	bool_t bCanPreview = false;
	bool_t bComposite = false;
};

struct EFFECT_RESOURCE_DESCRIPTOR final
{
	EFFECT_RESOURCE_KEY Key;
	std::string strDisplayLabel;
	std::string strCategoryLabel;
	EFFECT_RESOURCE_CAPABILITIES Capabilities;
};

/* Immutable, query-only view shared by Effect Tool and composition surfaces.
   A consumer keeps this handle for one UI operation, so Find never observes a
   different revision and never scans resource files. */
class EFFECT_RESOURCE_CATALOG_SNAPSHOT final
{
public:
	[[nodiscard]] uint64_t Get_Revision() const noexcept;
	[[nodiscard]] uint64_t Get_DirectAuthoredSourceRevision() const noexcept;
	[[nodiscard]] uint64_t Get_TypedSourceRevision() const noexcept;
	[[nodiscard]] bool_t Is_Ready() const noexcept;
	[[nodiscard]] const std::vector<EFFECT_RESOURCE_DESCRIPTOR>&
		Get_Resources() const noexcept;
	[[nodiscard]] const EFFECT_RESOURCE_DESCRIPTOR* Find(
		const EFFECT_RESOURCE_KEY& Key) const noexcept;
	[[nodiscard]] const EFFECT_RESOURCE_DESCRIPTOR* Find(
		std::string_view strStableId) const noexcept;

private:
	friend class CEffectResourceCatalog;

	EFFECT_RESOURCE_CATALOG_SNAPSHOT() = default;

	uint64_t m_iRevision = 0u;
	uint64_t m_iDirectAuthoredSourceRevision = 0u;
	uint64_t m_iTypedSourceRevision = 0u;
	std::vector<EFFECT_RESOURCE_DESCRIPTOR> m_Resources;
	std::map<std::string, std::size_t, std::less<>> m_StableIdIndex;
};

/* Main-thread authoring facade.  Reload_Valtan consumes already-admitted owner
   catalogs, validates one collision-free resource namespace, and atomically
   replaces the immutable snapshot.  Any failure preserves the prior view. */
class CEffectResourceCatalog final
{
public:
	static CEffectResourceCatalog& Get();

	bool_t Reload_Valtan(std::string& strOutError);
	[[nodiscard]] std::shared_ptr<const EFFECT_RESOURCE_CATALOG_SNAPSHOT>
		Get_Snapshot() const;
	[[nodiscard]] uint64_t Get_Revision() const;

private:
	CEffectResourceCatalog();

	CEffectResourceCatalog(const CEffectResourceCatalog&) = delete;
	CEffectResourceCatalog& operator=(const CEffectResourceCatalog&) = delete;

private:
	mutable std::mutex m_SnapshotMutex;
	std::shared_ptr<const EFFECT_RESOURCE_CATALOG_SNAPSHOT> m_pSnapshot;
};

NS_END
