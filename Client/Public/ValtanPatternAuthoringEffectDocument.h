#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

// DRAFT_ATTACHED Effect의 패턴 소유 ID와 Data 상대 저작 경로다. 각 패턴은 이 목록에서 한 행만 가질 수 있다.
struct VALTAN_PATTERN_AUTHORING_EFFECT_BINDING final
{
	std::string strPatternId;
	std::string strEffectAssetId;
	std::string strAuthoringPath;
	std::string strState = "DRAFT_ATTACHED";

	bool operator==(
		const VALTAN_PATTERN_AUTHORING_EFFECT_BINDING&) const = default;
};

// 발탄의 편집용 소유 관계 목록을 값으로 보관한다. Effect 파일의 payload나 실행 상태는 다른 계층이 소유한다.
struct VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT final
{
	std::string strBossArchetypeId = "BOSS_VALTAN";
	std::vector<VALTAN_PATTERN_AUTHORING_EFFECT_BINDING> Bindings;

	bool operator==(
		const VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT&) const = default;
};

// Effect Tool의 생성/삭제가 두 문서를 처리하는 동안 같은 이름의 mutex 참여자를 직렬화한다.
// 두 파일을 한 번에 원자적으로 교체하는 장치는 아니며 실패 보상은 호출자가 담당한다.
// Win32 HANDLE은 구현 파일에 두고 공개 헤더에는 opaque 포인터만 보관한다.
class CValtanPatternAuthoringEffectTransaction final
{
public:
	CValtanPatternAuthoringEffectTransaction() = default;
	~CValtanPatternAuthoringEffectTransaction();

	CValtanPatternAuthoringEffectTransaction(
		const CValtanPatternAuthoringEffectTransaction&) = delete;
	CValtanPatternAuthoringEffectTransaction& operator=(
		const CValtanPatternAuthoringEffectTransaction&) = delete;

	// 기다리지 않고 같은 Windows 세션의 이름 있는 mutex를 획득한다. 실패하면 상태를 반환한다.
	// 같은 객체가 이미 소유하면 즉시 성공하며 소유 스레드에서 객체를 파괴해 잠금을 해제해야 한다.
	bool_t Try_Acquire(std::string& strOutStatus);

private:
	// 이 객체가 닫을 HANDLE이다. m_bOwned는 ReleaseMutex가 필요한 소유 상태를 나타낸다.
	void* m_pMutexHandle = nullptr;
	bool_t m_bOwned = false;
};

// 패턴 하나와 편집용 aggregate Effect 하나의 소유 관계만 저장한다.
// stage, clip, 시각, anchor와 Server trigger는 포함하지 않으며 실제 재생 cue의 저장소가 아니다.
class CValtanPatternAuthoringEffectDocument final
{
public:
	// Data/Effects/ValtanPatternAuthoringEffects.json의 경로만 구한다. 파일을 생성하거나 읽지 않는다.
	static std::filesystem::path Resolve_Path();
	// stable asset ID에서 Effects/Authored/<ID>.effect.json 상대 경로를 만든다. 잘못된 ID면 빈 문자열이다.
	static std::string Build_AuthoringPath(
		std::string_view strEffectAssetId);
	// binding 경로가 ID로 만든 경로와 같은지 대조한 뒤 Data 루트에서 해석한다. 사용 전에 binding 전체를 Validate해야 한다.
	static std::filesystem::path Resolve_AuthoringPath(
		const VALTAN_PATTERN_AUTHORING_EFFECT_BINDING& Binding);

	// 정확한 JSON 필드 집합과 소유 관계를 임시 문서에 검증한다. false 경로에서는 기존 OutDocument를 유지한다.
	static bool_t Parse_Text(
		std::string_view strText,
		VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT& OutDocument,
		std::string& strOutStatus);
	// 보스 ID, 최대 행 수, DRAFT 상태와 ID/경로 중복을 검사한다. 대상 패턴과 Effect 파일의 존재를 확인하는 함수는 아니다.
	static bool_t Validate(
		const VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT& Document,
		std::string& strOutStatus);
	// 입력 순서를 유지하는 정규 문자열을 만든다. 저장된 공백과 객체 키 순서는 재현하지 않으며 별도 검증을 수행하지 않는다.
	static std::string Serialize(
		const VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT& Document);
	// 문서를 읽고 정규 직렬화 문자열을 baseline으로 함께 반환한다. 파일 열기·구문·검증 실패로 false를 반환하면 이전 출력 문서와 baseline을 유지한다.
	static bool_t Load(
		VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT& OutDocument,
		std::string& strOutCanonicalBaseline,
		std::string& strOutStatus);
	// mutex 아래 기존 파일의 정규 baseline을 앞뒤로 확인하고 검증한 임시 파일로 교체한다.
	// 기존 소유 관계 파일이 있어야 하며 Effect payload 생성/삭제나 Server 게시까지 수행하지 않는다.
	static bool_t Save_AtomicIfUnchanged(
		const VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT& Document,
		std::string_view strExpectedCanonicalBaseline,
		std::string& strOutStatus);
};

NS_END
