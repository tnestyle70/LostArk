#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <variant>
#include <vector>

NS_BEGIN(Client)

// descriptor의 선언 상태다. enum에 값이 있어도 각 loader가 그 상태를 실제로 허용하는지는 별도 검사한다.
enum class COMPOSITION_SOURCE_STATUS : uint8_t
{
	SHADOW,
	REFERENCE_ONLY,
	AUTHORITATIVE,
};

// 원본 파일의 역할과 저장소 Data 상대 경로다. 실제 내용 전체를 복제하지 않는다.
struct BOSS_COMPOSITION_SOURCE_DOCUMENT final
{
	std::string role;
	std::string path;
};

// 쿠크 프로필별 Action 참조, 슬롯 재정의와 로컬 패턴의 경로 및 기대값을 묶는다.
struct BOSS_COMPOSITION_REFERENCE_PROFILE final
{
	std::string profileId;
	std::string actionReferencePath;
	std::string actionBindingPath;
	std::string patternBindingPath;
	std::string expectedReferenceRevision;
	uint32_t expectedActionCount = 0u;
};

// descriptor에 선언한 기대 개수와 identity 해시다. loader는 형식과 내부 합계를 검사한다.
// 원본 파일의 전체 내용을 재집계하거나 이 해시를 새로 계산한 결과는 아니다.
struct BOSS_COMPOSITION_COVERAGE final
{
	std::string kind;
	std::string expectedIdentitySha256;
	uint32_t expectedPatternCount = 0u;
	uint32_t expectedStageCount = 0u;
	uint32_t expectedProfileCount = 0u;
	uint32_t expectedActionCount = 0u;
	std::vector<BOSS_COMPOSITION_REFERENCE_PROFILE> profiles;
};

// 다른 descriptor가 연결할 패턴 ID 색인이다. 공격 단계나 판정 payload를 담는 정의가 아니다.
struct BOSS_COMPOSITION_PATTERN final
{
	std::string patternId;
};

// 보스 저작 파일들의 관계를 설명하는 v1 descriptor를 읽는다. 이 클래스는 전투나 연출을 실행하지 않는다.
// 현재 loader는 발탄의 SHADOW와 쿠크의 REFERENCE_ONLY만 허용하며 AUTHORITATIVE 문서는 받지 않는다.
// 실제 보스 편집 문서와 Server 게시 데이터의 권위를 이 descriptor로 옮기는 기능은 구현되어 있지 않다.
class CBossCompositionDocument final
{
public:
	// 파일을 임시 descriptor로 읽고 검증한다. false를 반환하는 경로에서는 기존 문서 상태를 바꾸지 않는다.
	bool_t Load(
		const std::filesystem::path& path,
		std::string& outStatus);
	// 보유한 descriptor 값을 초기 상태로 되돌린다. 디스크 파일은 삭제하거나 변경하지 않는다.
	void Clear();

	// 아래 const 참조 조회는 이 문서 객체의 멤버를 차용한다. Load나 Clear 뒤에는 내용이 바뀔 수 있다.
	[[nodiscard]] const std::string& Get_CompositionId() const noexcept
	{
		return m_CompositionId;
	}
	[[nodiscard]] const std::string& Get_BossArchetypeId() const noexcept
	{
		return m_BossArchetypeId;
	}
	[[nodiscard]] const std::string& Get_EncounterId() const noexcept
	{
		return m_EncounterId;
	}
	[[nodiscard]] const std::string& Get_AreaId() const noexcept
	{
		return m_AreaId;
	}
	[[nodiscard]] uint32_t Get_Revision() const noexcept
	{
		return m_Revision;
	}
	[[nodiscard]] COMPOSITION_SOURCE_STATUS Get_Status() const noexcept
	{
		return m_Status;
	}
	[[nodiscard]] const std::vector<BOSS_COMPOSITION_SOURCE_DOCUMENT>&
	Get_SourceDocuments() const noexcept
	{
		return m_SourceDocuments;
	}
	[[nodiscard]] const std::string& Get_DisplayName() const noexcept
	{
		return m_DisplayName;
	}
	[[nodiscard]] const BOSS_COMPOSITION_COVERAGE& Get_Coverage() const noexcept
	{
		return m_Coverage;
	}
	[[nodiscard]] const std::vector<BOSS_COMPOSITION_PATTERN>&
	Get_Patterns() const noexcept
	{
		return m_Patterns;
	}
	// 패턴 색인에서 stable ID를 선형 조회한다. 반환 포인터는 차용이며 Load나 Clear로 무효화될 수 있다.
	[[nodiscard]] const BOSS_COMPOSITION_PATTERN* Find_Pattern(
		const std::string& patternId) const;

	// 상태 표시용 정적 문자열을 반환한다. 알 수 없는 값은 물음표로 표시한다.
	static const char_t* Status_ToString(COMPOSITION_SOURCE_STATUS status);

private:
	// 로드가 성공한 descriptor의 값 복사본을 이 객체가 소유한다. 실행 객체나 Server 상태는 소유하지 않는다.
	std::string m_CompositionId;
	std::string m_DisplayName;
	std::string m_BossArchetypeId;
	std::string m_EncounterId;
	std::string m_AreaId;
	uint32_t m_Revision = 0u;
	COMPOSITION_SOURCE_STATUS m_Status = COMPOSITION_SOURCE_STATUS::SHADOW;
	std::vector<BOSS_COMPOSITION_SOURCE_DOCUMENT> m_SourceDocuments;
	BOSS_COMPOSITION_COVERAGE m_Coverage;
	std::vector<BOSS_COMPOSITION_PATTERN> m_Patterns;
};

// 현재 reader가 지원하는 세 가지 typed 참조 종류다. 지원하지 않는 payload는 로드에서 거절한다.
enum class ARENA_SEQUENCER_TRACK_KIND : uint8_t
{
	WORLD_SEQUENCE,
	CAMERA_SHOT,
	ACTOR_PATTERN,
};

struct ARENA_WORLD_SEQUENCE_REFERENCE final
{
	std::string instanceId;
};

struct ARENA_CAMERA_SHOT_REFERENCE final
{
	std::string shotId;
};

struct ARENA_ACTOR_PATTERN_REFERENCE final
{
	std::string bossCompositionId;
	std::string patternId;
};

// kind에 대응하는 한 종류의 참조만 저장한다. 로더가 kind와 variant의 대응을 함께 구성한다.
using ARENA_SEQUENCER_TRACK_REFERENCE = std::variant<
	ARENA_WORLD_SEQUENCE_REFERENCE,
	ARENA_CAMERA_SHOT_REFERENCE,
	ARENA_ACTOR_PATTERN_REFERENCE>;

// 트랙 identity와 시작 및 선택적 종료 시각을 ms로 저장한다. endMs 생략과 명시 값을 hasEndMs로 구분한다.
struct ARENA_SEQUENCER_TRACK final
{
	std::string trackId;
	ARENA_SEQUENCER_TRACK_KIND kind =
		ARENA_SEQUENCER_TRACK_KIND::WORLD_SEQUENCE;
	ARENA_SEQUENCER_TRACK_REFERENCE reference;
	uint32_t startMs = 0u;
	uint32_t endMs = 0u;
	bool_t hasEndMs = false;
};

// 아레나 descriptor에서 공통 identity와 ms 시계 및 typed 참조를 읽는다.
// 월드 시퀀스, 카메라와 actor 패턴의 실제 payload는 각 소유 문서에 남는다.
// 현재는 알려진 두 SHADOW descriptor만 허용하며 이 객체가 트랙을 재생하지는 않는다.
class CArenaSequencerDocument final
{
public:
	// 파일을 임시 descriptor로 읽고 검증한다. false를 반환하는 경로에서는 기존 문서 상태를 바꾸지 않는다.
	bool_t Load(
		const std::filesystem::path& path,
		std::string& outStatus);
	// 보유한 descriptor 값을 초기 상태로 되돌린다. 디스크 파일은 삭제하거나 변경하지 않는다.
	void Clear();

	// 아래 조회는 아레나 descriptor의 값을 읽는다. const 참조의 내용은 이후 Load나 Clear에서 바뀔 수 있다.
	[[nodiscard]] const std::string& Get_SequencerId() const noexcept
	{
		return m_SequencerId;
	}
	[[nodiscard]] const std::string& Get_AreaId() const noexcept
	{
		return m_AreaId;
	}
	[[nodiscard]] uint32_t Get_Revision() const noexcept
	{
		return m_Revision;
	}
	[[nodiscard]] uint32_t Get_DurationMs() const noexcept
	{
		return m_DurationMs;
	}
	[[nodiscard]] COMPOSITION_SOURCE_STATUS Get_Status() const noexcept
	{
		return m_Status;
	}
	[[nodiscard]] const std::string& Get_DisplayName() const noexcept
	{
		return m_DisplayName;
	}
	[[nodiscard]] const std::string& Get_BossCompositionId() const noexcept
	{
		return m_BossCompositionId;
	}
	[[nodiscard]] const std::vector<BOSS_COMPOSITION_SOURCE_DOCUMENT>&
	Get_SourceDocuments() const noexcept
	{
		return m_SourceDocuments;
	}
	[[nodiscard]] const std::vector<ARENA_SEQUENCER_TRACK>&
	Get_Tracks() const noexcept
	{
		return m_Tracks;
	}
	// 지원 트랙 종류의 표시 이름을 정적 문자열로 반환한다. 알 수 없는 값은 물음표다.
	static const char_t* TrackKind_ToString(ARENA_SEQUENCER_TRACK_KIND kind);

private:
	// 로드한 트랙 참조와 시계를 값으로 소유한다. 참조 대상의 리소스나 재생 상태는 여기서 생성하지 않는다.
	std::string m_SequencerId;
	std::string m_DisplayName;
	std::string m_AreaId;
	std::string m_BossCompositionId;
	uint32_t m_Revision = 0u;
	uint32_t m_DurationMs = 0u;
	COMPOSITION_SOURCE_STATUS m_Status = COMPOSITION_SOURCE_STATUS::SHADOW;
	std::vector<BOSS_COMPOSITION_SOURCE_DOCUMENT> m_SourceDocuments;
	std::vector<ARENA_SEQUENCER_TRACK> m_Tracks;
};

// 선택한 보스와 아레나 descriptor 한 쌍을 임시 객체로 읽고 identity, Area 및 actor 패턴 참조를 맞춘다.
// 다른 보스 파일은 읽지 않는다. 성공한 한 쌍으로 기존 목록을 교체하며 실패하면 기존 목록을 유지한다.
// 현재 직접 호출자는 BossToolTests이며 제품 Workbench의 runtime scheduler로 연결된 클래스는 아니다.
class CCompositionDocumentCatalog final
{
public:
	// 알려진 발탄 또는 쿠크 쌍만 Data 루트에서 읽는다. 두 문서와 교차 참조가 모두 맞아야 목록을 교체한다.
	bool_t Load_Pair(
		const std::string& compositionId,
		const std::string& sequencerId,
		std::string& outStatus);
	// 보유한 descriptor 값을 초기 상태로 되돌린다. 디스크 파일은 삭제하거나 변경하지 않는다.
	void Clear();
	// 현재 보유한 보스 descriptor를 차용한다. 없으면 nullptr이며 성공한 Load_Pair나 Clear 뒤에는 포인터가 무효화된다.
	[[nodiscard]] const CBossCompositionDocument* Find_Boss(
		const std::string& compositionId) const;
	// 현재 보유한 아레나 descriptor를 차용한다. 포인터의 소유권은 카탈로그에 남는다.
	[[nodiscard]] const CArenaSequencerDocument* Find_Arena(
		const std::string& sequencerId) const;

private:
	// vector 형식이지만 현재 Load_Pair는 선택한 보스 하나와 아레나 하나로 기존 목록을 교체한다.
	std::vector<CBossCompositionDocument> m_BossDocuments;
	std::vector<CArenaSequencerDocument> m_ArenaDocuments;
};

NS_END
