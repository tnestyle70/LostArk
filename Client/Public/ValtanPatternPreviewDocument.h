#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

// 원본 동작과 패턴 번호의 대응 근거를 표시한다. 확정/후보 표시는 Server 동작의 검증 결과가 아니다.
// UNRESOLVED와 NO_ANIMATION은 sequence 없이 marker로만 재생하며 END는 잘못된 값을 나타낸다.
enum class VALTAN_PATTERN_PREVIEW_EVIDENCE
{
	SOURCE_FAMILY_DIRECT,
	USER_CONFIRMED_FAMILY,
	CANDIDATE,
	COMPOSITE,
	UNRESOLVED,
	NO_ANIMATION,
	END,
};

// JSON에는 개별 clip 대신 (sourceActionId, sequenceIndex)와 반복 횟수를 저장한다.
// 표시 이름, mode와 clip 순서는 Load가 Valtan.clipseq의 같은 키를 찾아 값으로 복사한다.
struct VALTAN_PATTERN_PREVIEW_SOURCE_SEQUENCE_REF final
{
	uint32_t iSourceActionId = 0u;
	int32_t iSequenceIndex = -1;
	uint32_t iRepeat = 1u;

	// patternpreview JSON의 저작 필드가 아닌 연결 결과다. Parse_Text만 호출하면 비어 있다.
	// Resolve_SourceSequences가 정확한 키와 모델 clip 존재를 확인한 뒤 채운다.
	std::string strSequenceName;
	std::string strSequenceMode;
	std::vector<std::string> ResolvedClips;

	// 재생 목록 작성 전 연결 결과 세 필드가 비어 있지 않은지만 확인한다. 파일이나 모델을 다시 검증하지 않는다.
	bool_t Is_Resolved() const
	{
		return !strSequenceName.empty() && !strSequenceMode.empty() &&
			!ResolvedClips.empty();
	}

	bool operator==(
		const VALTAN_PATTERN_PREVIEW_SOURCE_SEQUENCE_REF&) const = default;
};

// 1~67번의 읽기 전용 참조 행이다. label/note와 sequence 목록을 값으로 소유하며 배열 순서가 번호와 같아야 한다.
struct VALTAN_PATTERN_PREVIEW_ENTRY final
{
	uint32_t iNumber = 0u;
	std::string strLabel;
	VALTAN_PATTERN_PREVIEW_EVIDENCE eEvidence =
		VALTAN_PATTERN_PREVIEW_EVIDENCE::END;
	std::string strNote;
	std::vector<VALTAN_PATTERN_PREVIEW_SOURCE_SEQUENCE_REF> Sequences;

	bool operator==(const VALTAN_PATTERN_PREVIEW_ENTRY&) const = default;
};

// Animation Tool이 보관하는 참조 문서다. 모델 포인터와 Server 패턴 실행 상태는 저장하지 않는다.
struct VALTAN_PATTERN_PREVIEW_DOCUMENT final
{
	std::string strAnimationAssetId;
	std::vector<VALTAN_PATTERN_PREVIEW_ENTRY> Patterns;

	bool operator==(const VALTAN_PATTERN_PREVIEW_DOCUMENT&) const = default;
};

// sequence와 반복을 clip 단위로 펼친 미리보기 항목이다. 문자열을 복사하므로 원본 문서를 교체해도 차용 포인터가 남지 않는다.
// 실제 모델의 weak_ptr, 재생 시간과 대상 generation은 Animation Tool이 별도로 관리한다.
struct VALTAN_PATTERN_PREVIEW_PLAY_ITEM final
{
	uint32_t iPatternNumber = 0u;
	std::string strPatternLabel;
	VALTAN_PATTERN_PREVIEW_EVIDENCE eEvidence =
		VALTAN_PATTERN_PREVIEW_EVIDENCE::END;
	std::string strNote;

	uint32_t iSourceActionId = 0u;
	int32_t iSequenceIndex = -1;
	// 반복 번호와 원본 내부 step 번호는 재생 항목에서 1부터 센다. marker와 미설정 항목은 기본값 0을 사용한다.
	uint32_t iSequenceRepeatNumber = 0u;
	uint32_t iSequenceRepeatCount = 0u;
	uint32_t iSourceStepNumber = 0u;
	uint32_t iSourceStepCount = 0u;
	std::string strSequenceName;
	std::string strSequenceMode;
	std::string strClipName;

	// 패턴 안에서 반복까지 펼친 전체 step 번호/개수다. source sequence 안의 step 번호와 구분한다.
	uint32_t iStepNumber = 0u;
	uint32_t iStepCount = 0u;
	// 동작이 없는 참조 행도 목록에 남기는 표시 항목이다. Tool 재생기는 0.45초 idle 표시로 처리한다.
	bool_t bPatternMarker = false;
	// 재생 길이(초)다. 이 문서의 Build_Playlist는 기본값 0을 유지해 모델의 원래 길이를 사용한다.
	// 같은 타입을 쓰는 원본 sequence/custom chain 경로는 cut, notify 길이나 사용자 길이를 채운다.
	// 재생기는 양수면 그 길이를 사용하고 모델 길이보다 0.001초 넘게 길면 clip을 반복한다.
	f32_t fAuthoredDurationSeconds = 0.f;

	bool operator==(const VALTAN_PATTERN_PREVIEW_PLAY_ITEM&) const = default;
};

// 원본 참조 JSON/.clipseq를 읽어 로컬 재생 목록으로 변환한다. Save/Publish와 Server 패턴 변경 API는 없다.
// 정적 함수는 호출 중 임시 값만 소유하며 호출자가 출력 문서/목록과 상태 문자열의 수명을 관리한다.
class CValtanPatternPreviewDocument final
{
public:
	// Data/Animation/Authored/Valtan/Valtan.patternpreview.json의 경로를 구한다. 파일 존재 여부는 Load가 확인한다.
	static std::filesystem::path Resolve_Path();
	// 허용된 asset token으로 Data/Animation/Reference/<asset>/<asset>.clipseq 경로를 만든다.
	static std::filesystem::path Resolve_ClipSequencePath(
		std::string_view animationAssetId);

	// JSON v2의 저작 참조만 임시 문서에 읽는다. false이면 기존 outDocument를 보존한다.
	// 실제 .clipseq 연결과 대상 모델의 clip 대조는 Resolve_SourceSequences가 맡는다.
	static bool_t Parse_Text(
		std::string_view text,
		VALTAN_PATTERN_PREVIEW_DOCUMENT& outDocument,
		std::string& outStatus);
	// 저작 참조를 디스크 .clipseq 및 호출자가 준 모델 clip 이름 목록과 대조한다. false이면 이전 출력 문서는 유지한다.
	// 입력 document와 출력이 같아도 임시 복사본을 먼저 완성한 뒤 교체한다.
	static bool_t Resolve_SourceSequences(
		const VALTAN_PATTERN_PREVIEW_DOCUMENT& document,
		std::string_view expectedAnimationAssetId,
		const std::vector<std::string>& availableClips,
		VALTAN_PATTERN_PREVIEW_DOCUMENT& outResolvedDocument,
		std::string& outStatus);
	// Resolve_SourceSequences를 수행한 결과 문서만 버린다. 메모리 필드 검사뿐 아니라 디스크 .clipseq 읽기도 수행한다.
	static bool_t Validate(
		const VALTAN_PATTERN_PREVIEW_DOCUMENT& document,
		std::string_view expectedAnimationAssetId,
		const std::vector<std::string>& availableClips,
		std::string& outStatus);
	// Render_ValtanPatternPreview가 최초 참조 탭 접근에서 호출한다. JSON 읽기/parse/연결 성공 뒤에만 출력 문서를 교체한다.
	// 파일 열기·구문·검증 실패는 false와 상태 메시지로 전달한다. 읽은 뒤 stream 오류 상태를 별도로 확인하지는 않는다.
	static bool_t Load(
		std::string_view expectedAnimationAssetId,
		const std::vector<std::string>& availableClips,
		VALTAN_PATTERN_PREVIEW_DOCUMENT& outDocument,
		std::string& outStatus);
	// Start_ValtanPatternPreview가 1부터 세는 양끝 포함 번호 범위를 요청한다. 입력/연결 상태를 확인해 임시 목록을 만든다.
	// false이면 이전 outPlaylist는 보존하며 실제 모델의 재생 시작은 호출자가 수행한다.
	static bool_t Build_Playlist(
		const VALTAN_PATTERN_PREVIEW_DOCUMENT& resolvedDocument,
		uint32_t iFirstPattern,
		uint32_t iLastPattern,
		std::vector<VALTAN_PATTERN_PREVIEW_PLAY_ITEM>& outPlaylist,
		std::string& outStatus);
	// Tool 표시용 문자열 literal을 반환한다. 반환 포인터는 정적 수명이며 호출자가 해제하지 않는다.
	static const char_t* Evidence_Name(
		VALTAN_PATTERN_PREVIEW_EVIDENCE eEvidence);
};

NS_END
