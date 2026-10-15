#pragma once

#include "KoukuSaydonAnimationActionDocument.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
	// 패턴 안에 배치한 클립 하나다. occurrenceId는 배치의 identity이고 stageId와 slotId는 원본 Action 슬롯의 identity다.
	// 시작과 길이는 ms, 배율은 원본 재생 시계에 곱한다. 종료 정책은 EXACT, HOLD_LAST_POSE 또는 LOOP_TO_WINDOW다.
	struct KOUKU_SAYDON_ANIMATION_PATTERN_CLIP final
	{
		std::string strOccurrenceId;
		std::string strStageId;
		std::string strSlotId;
		std::string strRuntimeClip;
		std::uint32_t iSourceStartMs = 0u;
		std::uint32_t iPlayMs = 0u;
		f32_t fPlayRate = 1.f;
		std::string strEndPolicy;

		bool operator==(const KOUKU_SAYDON_ANIMATION_PATTERN_CLIP&) const = default;
	};

	// 참조 Action 하나에서 고른 클립 배치 목록을 소유한다. 같은 원본 슬롯을 여러 배치에서 사용할 수 있다.
	// 배치마다 고유 occurrenceId를 사용하며 다음 번호는 삭제 뒤에도 기존 ID를 다시 할당하지 않도록 저장한다.
	struct KOUKU_SAYDON_ANIMATION_PATTERN final
	{
		std::string strPatternId;
		std::string strDisplayName;
		std::uint32_t iSourceActionId = 0u;
		std::uint32_t iNextOccurrenceOrdinal = 1u;
		std::vector<KOUKU_SAYDON_ANIMATION_PATTERN_CLIP> Clips;

		bool operator==(const KOUKU_SAYDON_ANIMATION_PATTERN&) const = default;
	};

	// 하나의 Action 참조 revision을 바탕으로 만든 로컬 애니메이션 조합이다.
	// REFERENCE_ONLY만 허용하며 Server의 판정과 보스 PRODUCT 문서를 대신하지 않는다.
	struct KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT final
	{
		std::uint32_t iFormatVersion = 1u;
		std::string strProfileId;
		std::string strReferenceRevision;
		std::string strAuthority;
		std::uint32_t iNextPatternOrdinal = 1u;
		std::vector<KOUKU_SAYDON_ANIMATION_PATTERN> Patterns;

		bool operator==(const KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT&) const = default;
	};

	// Animation Tool의 로컬 애니메이션 패턴 파일 경계다. 상태를 가진 서비스가 아니라 입력 문서를 검증하고 읽고 쓰는 정적 API다.
	class CKoukuSaydonAnimationPatternDocument final
	{
	public:
		// 지원하는 네 프로필의 Data/Animation/Authored/KoukuSaydon patternbindings 경로를 만든다.
		// 지원하지 않는 프로필이면 빈 경로를 반환한다.
		static std::filesystem::path Resolve_Path(
			std::string_view profileId);

		// JSON을 임시 문서로 읽고 ID, 카운터, 개수와 시간 범위까지 구조 검증한다.
		// 성공할 때만 출력을 교체한다. Action 참조와 실제 모델의 연결은 별도 Validate가 맡는다.
		static bool_t Parse_Text(
			std::string_view text,
			KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT& outDocument,
			std::string& outStatus);

		// Action 참조를 먼저 검증하고 같은 profile과 referenceRevision인지 확인한다.
		// REVIEW_CANDIDATE Action의 슬롯과 실제 availableClips에 연결되어야 하며 HOLDOUT Action은 거절한다.
		static bool_t Validate(
			const KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT& document,
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
			std::string_view expectedProfileId,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			std::string& outStatus);

		// 경로, 파싱, 구조와 원본 슬롯 identity 및 실제 모델의 클립 연결을 모두 검사한다.
		// 임시 문서의 검증이 성공한 뒤에만 outDocument를 교체하므로 실패하면 이전 출력은 보존한다.
		static bool_t Load_FromPath(
			const std::filesystem::path& path,
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
			std::string_view expectedProfileId,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT& outDocument,
			std::string& outStatus);
		// Animation Tool이 프로필의 기본 경로를 구해 Load_FromPath를 호출하는 진입점이다.
		static bool_t Load(
			std::string_view profileId,
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT& outDocument,
			std::string& outStatus);

		// Animation Tool이 검증한 패턴을 같은 디렉터리의 임시 파일로 쓰고 재파싱하여 원문서와 비교한다.
		// Action 참조의 현재 내용도 다시 확인한 뒤 목적 파일을 원자적으로 교체한다.
		// 참조 비교는 목적 pattern 파일의 변경 감시나 초안 병합을 뜻하지 않는다.
		static bool_t Save_Atomic(
			const KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT& document,
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
			std::string_view expectedProfileId,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			std::string& outStatus);
	};
}
