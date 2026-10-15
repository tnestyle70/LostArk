#pragma once

// Client/Engine 공통 타입과 선언 매크로를 사용한다.
#include "Client_Defines.h"
#include "Engine_Defines.h"

// 불변 descriptor 목록·ID 인덱스와 snapshot 공유/교체 동기화에 필요한 표준 타입이다.
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

// 문서 소유자별 명령 분기용 종류다. 브라우저는 저장 버전 대신 descriptor의 stable ID와 분류를 표시한다.
enum class EFFECT_RESOURCE_OWNER_KIND : uint8_t
{
	V1_DOCUMENT,
	V2_LEAF,
	V2_GROUP,
	END
};

// 같은 stable ID를 어떤 문서 소유자에게 전달할지 묶는다. UI 표시 이름은 식별자로 사용하지 않는다.
struct EFFECT_RESOURCE_KEY final
{
	EFFECT_RESOURCE_OWNER_KIND eOwnerKind =
		EFFECT_RESOURCE_OWNER_KIND::END;
	std::string strStableId;

	// 빈 ID와 END를 거르는 최소 검사다. 등록 여부와 소유자 일치는 snapshot Find가 확인한다.
	[[nodiscard]] bool_t Is_Valid() const noexcept;
	[[nodiscard]] bool_t operator==(const EFFECT_RESOURCE_KEY& Other) const
		noexcept;
};

// 현재 소유자가 지원하는 저작 명령을 UI에 전달한다. 자원 로드 성공이나 GPU 준비 여부는 아니다.
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

// 브라우저가 표시할 ID·분류와 명령 가능 여부만 소유한다. 원본 Effect 문서나 렌더 자원은 포함하지 않는다.
struct EFFECT_RESOURCE_DESCRIPTOR final
{
	EFFECT_RESOURCE_KEY Key;
	std::string strDisplayLabel;
	std::string strCategoryLabel;
	EFFECT_RESOURCE_CAPABILITIES Capabilities;
};

// Effect Tool과 조합 화면이 한 작업 동안 유지하는 불변 조회 view다.
// Find는 이 목록만 검색하며 파일을 읽거나 다른 revision으로 갈아타지 않는다.
class EFFECT_RESOURCE_CATALOG_SNAPSHOT final
{
public:
	[[nodiscard]] uint64_t Get_Revision() const noexcept;
	[[nodiscard]] uint64_t Get_DirectAuthoredSourceRevision() const noexcept;
	[[nodiscard]] uint64_t Get_TypedSourceRevision() const noexcept;
	[[nodiscard]] bool_t Is_Ready() const noexcept;
	// 반환 참조와 Find 포인터는 이 snapshot의 수명 동안 유효하다. 호출자는 shared_ptr를 함께 보유한다.
	[[nodiscard]] const std::vector<EFFECT_RESOURCE_DESCRIPTOR>&
		Get_Resources() const noexcept;
	[[nodiscard]] const EFFECT_RESOURCE_DESCRIPTOR* Find(
		const EFFECT_RESOURCE_KEY& Key) const noexcept;
	[[nodiscard]] const EFFECT_RESOURCE_DESCRIPTOR* Find(
		std::string_view strStableId) const noexcept;

private:
	friend class CEffectResourceCatalog;

	EFFECT_RESOURCE_CATALOG_SNAPSHOT() = default;

	// 이 facade의 교체 revision과 두 원본 catalog revision을 별도로 보관한다. 0은 미준비다.
	uint64_t m_iRevision = 0u;
	uint64_t m_iDirectAuthoredSourceRevision = 0u;
	uint64_t m_iTypedSourceRevision = 0u;
	std::vector<EFFECT_RESOURCE_DESCRIPTOR> m_Resources;
	// 검증을 통과한 stable ID를 같은 snapshot의 Resources 위치에 연결한다.
	std::map<std::string, std::size_t, std::less<>> m_StableIdIndex;
};

// 메인 스레드 저작용 통합 목록이다. 이미 준비된 V1/V2 catalog를 읽고 ID 충돌을 검사한다.
// 검증된 snapshot 포인터만 교체하므로 거부된 후보는 기존 화면의 목록을 바꾸지 않는다.
class CEffectResourceCatalog final
{
public:
	static CEffectResourceCatalog& Get();

	// Effect Tool이 준비한 두 catalog에서 Valtan descriptor만 모아 검증한 뒤 교체한다. 실패하면 이전 view를 유지한다.
	bool_t Reload_Valtan(std::string& strOutError);
	// 현재 view의 공유 소유권을 복사한다. 이후 Reload가 성공해도 이미 얻은 snapshot은 바뀌지 않는다.
	[[nodiscard]] std::shared_ptr<const EFFECT_RESOURCE_CATALOG_SNAPSHOT>
		Get_Snapshot() const;
	[[nodiscard]] uint64_t Get_Revision() const;

private:
	CEffectResourceCatalog();

	CEffectResourceCatalog(const CEffectResourceCatalog&) = delete;
	CEffectResourceCatalog& operator=(const CEffectResourceCatalog&) = delete;

private:
	// 공유 포인터의 읽기/교체만 보호한다. 원본 catalog 갱신과 Reload 호출 자체는 메인 스레드 저작 흐름이 직렬화한다.
	mutable std::mutex m_SnapshotMutex;
	std::shared_ptr<const EFFECT_RESOURCE_CATALOG_SNAPSHOT> m_pSnapshot;
};

NS_END
