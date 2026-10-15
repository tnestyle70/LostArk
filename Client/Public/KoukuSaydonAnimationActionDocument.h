#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
	// 원본 Action 단계의 클립 슬롯이다. 추출 클립과 런타임 클립의 연결 및 재생 구간을 담는다.
	// 시작과 길이는 ms, 재생 배율은 원본 시계에 곱할 값이다. 클립 이름을 실제 모델과 맞추는 검증은 별도로 수행한다.
	struct KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE final
	{
		std::string strSlotId;
		std::string strExtractedClip;
		std::string strRuntimeClip;
		std::uint32_t iSourceStartMs = 0u;
		std::uint32_t iPlayMs = 0u;
		f32_t fPlayRate = 1.f;
		bool_t bLoop = false;
		std::string strMappingBasis;
		std::string strAuthority;

		bool operator==(
			const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE&) const = default;
	};

	// Action 내부 단계와 슬롯 목록을 소유한다. stageOrdinal은 순서 검사용이고 저장 연결에는 stageId와 slotId를 사용한다.
	// HoldoutClipNames는 검토가 보류된 이름이며 이미 확정한 재생 슬롯과 구분한다.
	struct KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE final
	{
		std::string strStageId;
		std::uint32_t iStageOrdinal = 0u;
		std::vector<std::string> HoldoutClipNames;
		std::vector<KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE> Slots;

		bool operator==(
			const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE&) const = default;
	};

	// 원본 Action ID 아래 단계들을 모은 참조 단위다. HOLDOUT 상태는 실제 보류 이름의 존재와 일치해야 한다.
	struct KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE final
	{
		std::uint32_t iSourceActionId = 0u;
		std::string strDisplayName;
		std::string strReviewStatus;
		std::string strAuthority;
		std::vector<KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE> Stages;

		bool operator==(
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE&) const = default;
	};

	// 추출 참조 문서가 프로필, 모델, 원본 근거 해시와 참조 revision 및 Action 목록을 소유한다.
	// 저작 binding은 referenceRevision으로 어느 참조를 바탕으로 편집했는지 확인한다.
	struct KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT final
	{
		std::uint32_t iFormatVersion = 1u;
		std::string strProfileId;
		std::string strModelAssetId;
		std::string strSourceEvidenceSha256;
		std::string strReferenceRevision;
		std::string strAuthority;
		std::vector<KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE> Actions;

		bool operator==(
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT&) const = default;
	};

	// 참조의 Action, Stage와 Slot을 정확히 지정한 선택적 재정의다.
	// REFERENCE_ONLY를 유지하며 이 문서만으로 Server의 PRODUCT 패턴을 만들지 않는다.
	struct KOUKU_SAYDON_ANIMATION_ACTION_BINDING final
	{
		std::uint32_t iSourceActionId = 0u;
		std::string strStageId;
		std::string strSlotId;
		std::string strRuntimeClip;
		std::uint32_t iSourceStartMs = 0u;
		std::uint32_t iPlayMs = 0u;
		f32_t fPlayRate = 1.f;
		bool_t bLoop = false;
		std::string strMappingBasis;
		std::string strAuthority;

		bool operator==(
			const KOUKU_SAYDON_ANIMATION_ACTION_BINDING&) const = default;
	};

	// 원본 참조 전체를 복제하지 않고 바꾼 슬롯만 Bindings에 저장한다.
	// 같은 Action, Stage와 Slot 조합의 중복 재정의와 다른 referenceRevision의 저장은 거절한다.
	struct KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT final
	{
		std::uint32_t iFormatVersion = 1u;
		std::string strProfileId;
		std::string strReferenceRevision;
		std::string strAuthority;
		std::vector<KOUKU_SAYDON_ANIMATION_ACTION_BINDING> Bindings;

		bool operator==(
			const KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT&) const = default;
	};

	// Animation Tool의 참조와 재정의 파일 경계다. 객체 상태를 소유하지 않고 호출자가 전달한 문서를 검사하고 교체한다.
	// CompositionDocument는 별도로 참조 파서의 부분 로드 옵션을 사용하여 편집 목록을 구성한다.
	class CKoukuSaydonAnimationActionDocument final
	{
	public:
		// 프로필 ID를 파일 이름으로 검증하고 Data/Animation/Reference/KoukuSaydon의 저작 참조 경로를 구한다.
		// 안전한 ID가 아니면 빈 경로를 반환하며 파일 존재 여부는 읽기 단계에서 확인한다.
		static std::filesystem::path Resolve_ReferencePath(
			std::string_view profileId);
		// Data/Animation/Authored/KoukuSaydon의 프로필별 재정의 저장 경로를 구한다. 참조 경로와 분리한다.
		static std::filesystem::path Resolve_AuthoredPath(
			std::string_view profileId);
		// Animation Tool과 Composition Workbench가 카탈로그를 분류할 때 사용한다.
		// RPCZ의 저작 이름 접두사로 큰 쿠크를 구분하고 RPCT_07은 쿠크, 나머지는 세이튼으로 표시한다.
		// 저작 이름만 비교하며 모델 파일을 읽거나 패턴의 Server 권한을 바꾸지 않는다.
		static const char_t* Resolve_ActionCategory(
			std::string_view profileId,
			std::string_view displayName);

		// 참조 JSON을 임시 문서로 읽고 성공할 때만 outDocument를 교체한다. 실패한 기존 출력은 보존한다.
		// allowPartial은 잘못된 하위 행을 건너뛸 수 있게 한다. true 반환만으로 모델 연결과 문서 전체 의미 검증까지 끝난 것은 아니다.
		static bool_t Parse_ReferenceText(
			std::string_view text,
			KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& outDocument,
			std::string& outStatus,
			bool_t allowPartial = false);
		// 재정의 JSON의 타입과 필수 키를 읽는다. 참조와 슬롯 연결 검증은 Validate_Authored가 맡는다.
		// 파싱 실패 시 기존 outDocument를 보존한다.
		static bool_t Parse_AuthoredText(
			std::string_view text,
			KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT& outDocument,
			std::string& outStatus);

		// 예상 프로필과 모델, ID 중복, 단계 순서와 실제 availableClips의 runtimeClip을 검사한다.
		// 해시는 형식만 확인하며 원본 파일을 다시 해시하거나 문서를 변경하지 않는다.
		static bool_t Validate_Reference(
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& document,
			std::string_view expectedProfileId,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			std::string& outStatus);
		// 참조를 먼저 검증하고 Action, Stage와 Slot 조합으로 재정의 대상을 확인한다.
		// revision 불일치, 중복 슬롯과 모델에 없는 클립을 거절하며 입력 문서를 변경하지 않는다.
		static bool_t Validate_Authored(
			const KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT& document,
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
			std::string_view expectedProfileId,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			std::string& outStatus);

		// 참조와 재정의 문서를 임시 값으로 읽고 실제 모델의 클립 및 정확한 슬롯 연결을 검증한다.
		// 둘 다 성공한 뒤에만 두 출력을 교체하므로 실패하면 기존 출력 문서 둘을 보존한다.
		static bool_t Load_FromPaths(
			const std::filesystem::path& referencePath,
			const std::filesystem::path& authoredPath,
			std::string_view expectedProfileId,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& outReference,
			KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT& outAuthored,
			std::string& outStatus);
		// Animation Tool이 프로필에서 두 경로를 구해 Load_FromPaths를 호출하는 기본 진입점이다.
		static bool_t Load(
			std::string_view profileId,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& outReference,
			KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT& outAuthored,
			std::string& outStatus);

		// Animation Tool이 재정의 문서만 저장한다. 참조 파일의 현재 내용이 입력 참조와 같은지 두 번 확인한다.
		// 검증, 임시 파일 flush와 재파싱 비교를 마친 뒤 목적 파일을 원자적으로 교체한다.
		// Composition 저장의 초안 병합이나 목적 파일 bytes CAS는 이 API의 기능이 아니다.
		static bool_t Save_Atomic(
			const KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT& document,
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
			std::string_view expectedProfileId,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			std::string& outStatus);
	};
}
