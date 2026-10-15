#pragma once

// 공간 진입 trigger의 위치·반범위·yaw를 기존 world placement 형식으로 보관하고 Shared world/skill ID를 사용한다.
#include "WorldBootstrap.h"
// 가이드 시작 위치의 X/Y/Z 세 성분을 소유한다.
#include <array>
// world ID별 category ID를 하나씩 연결한다.
#include <map>
// 게시된 ID·별칭·대사·진단 문자열을 값으로 소유한다.
#include <string>
// 대사 segment·trigger·combo 단계·명령 목록의 게시 순서를 유지한다.
#include <vector>

namespace LostArk::Server
{
	// 클라이언트에 한 번 전송할 UTF-8 대사와 표시 ms다. 룸은 송신 성공 뒤 DurationMs/1000초 동안 다음 segment를 기다린다.
	struct GUIDE_PROMPT_SEGMENT
	{
		std::string Text;
		std::uint32_t DurationMs = 5000;
	};

	// stable prompt ID의 순서 있는 대사 묶음이다. trigger는 ID만 저장하고 재생할 때 catalog에서 다시 찾는다.
	struct GUIDE_PROMPT
	{
		std::string Id;
		std::vector<GUIDE_PROMPT_SEGMENT> Segments;
	};

	// 공간 진입·도움 명령·시작/복귀 등의 사건을 category와 대사/콤보에 연결한다. 런타임 예약 큐 자체는 룸이 소유한다.
	struct GUIDE_TRIGGER
	{
		// PatternId는 Type에 따라 보스 pattern ID·command ID·복귀 source world 이름이다. HELP_COMMAND만 ComboId를 지정할 수 있다.
		std::string Id, Type, Category, PatternId, PromptId, ComboId;
		// SPACE_ENTER에서만 읽는 world 공간 상자다. 위치와 반범위는 m, yaw는 degree이며 기존 Contains_Placement가 소비한다.
		WORLD_BOOTSTRAP_PLACEMENT Box;
		bool Enabled = true;
		// 룸은 최초 segment 송신 시 trigger cooldown을 소비하고, 시작 전 대사는 큰 Priority부터 안정 정렬한다.
		std::uint32_t CooldownMs = 10000;
		int Priority = 0;
	};

	// 저작 input slot과 publisher가 해석한 skill ID를 같은 index로 보관한다. 실제 서버 세대의 class/slot 일치는 Initialize_Guide가 재검사한다.
	struct GUIDE_COMBO
	{
		std::string Id;
		std::vector<std::string> Slots;
		std::vector<LostArk::Shared::SKILL_ID> Skills;
		// 전체 combo 제한과 현재 skill 실행 거절에 대한 대기 한도(ms)다. Repeat도 전체 TimeoutMs를 초기화하지 않는다.
		std::uint32_t TimeoutMs = 45000, StepWaitMs = 5000;
		bool Repeat = false;
	};

	// 소유자의 채팅 입력을 trim한 뒤 별칭과 정확히 비교한다. Stop은 콤보 대기를 취소하며 일반 명령 cooldown 검사를 우회한다.
	struct GUIDE_COMMAND
	{
		std::string Id, ComboId;
		std::vector<std::string> Aliases;
		bool Stop = false, Enabled = true;
		std::uint32_t CooldownMs = 3000;
	};

	// 룸의 공격·회피·추적 점수 가중치다. Load는 각 값 0~1 및 합 1±0.001을 검사하고 FOLLOW의 Attack=0을 요구한다.
	// 아래 멤버 기본값은 구성 중 초기값이며 그 자체로 게시 검증을 통과한 정규화 값은 아니다.
	struct GUIDE_WEIGHTS
	{
		float Attack = 1.f, Avoid = 3.f, Follow = 5.f;
	};

	// GameRoom이 소유하는 독립 Guide 게시 데이터다. parser 임시 문서에서 문자열·배열을 복사하므로 로드 뒤 원문 버퍼에 의존하지 않는다.
	// 룸 초기화에서 쓰고 worker가 읽는다. 공개 멤버 수정과 Load/조회 사이에는 자체 동기화가 없으므로 소유자가 직렬 접근을 보장한다.
	class CGuideCatalog final
	{
	  public:
		// Initialize_Guide가 Guide.runtime.json을 읽어 지역 후보를 검사한다. commit 전 파싱/검증 예외는 기존 catalog를 유지하고 false를 반환한다.
		// 완료 status 문자열은 commit 뒤에 만든다. 그 할당 예외까지 기존 상태로 되돌리는 강한 보장은 없으며 룸의 skill 세대 검사는 별도다.
		bool Load(std::string& status);
		// Load 성공 뒤 true가 되지만 Initialize_Guide의 설치된 skill 검증이 실패하면 룸이 다시 false로 내린다.
		bool Loaded = false;
		// publisher가 정규화한 게시 내용에서 만든 revision이다. 룸의 state/prompt 메시지에 포함되며 Load는 이전 revision과 신구를 비교하지 않는다.
		std::uint64_t Revision = 0;
		// 가이드/배치 identity와 표시 이름이다. AnchorPolicy는 허용 문자열을 검사해 보관하지만 현재 룸은 owner session의 player를 추적한다.
		std::string Id, Name, PlacementId, AnchorPolicy;
		// Bern의 안내 actor를 만들 때 navigation에 투영할 시작 world 위치(m)와 yaw(degree)다.
		std::array<float, 3> Position{};

		float Yaw = 0.f;
		// 추적 거리는 m다. Desired는 owner 뒤의 목표점, Minimum/Maximum은 유지 구간, Resume는 멀어진 뒤 재접근 기준이다.
		float DesiredDistance = 3.f, MinimumDistance = 2.f, MaximumDistance = 4.f, ResumeDistance = 6.f;
		// 전투·도보 복귀 상황을 제외하고 지나치게 멀어진 시간이 RecoverDelay초 쌓이면 룸이 재접근/착지 처리를 요청한다.
		float RecoverDistance = 30.f, RecoverDelay = 3.f;
		// 판단 간격·위험 예측 구간·최소 상태 유지 시간은 초다. SwitchMargin은 점수 차, LethalHpFraction은 생존 회피 우선의 HP 비율 기준이다.
		float ThinkSeconds = .1f, HorizonSeconds = 1.f, SwitchMargin = .15f, MinimumHoldSeconds = .25f,
		      LethalHpFraction = .25f;
		GUIDE_WEIGHTS FollowWeights, AssistWeights;
		std::map<LostArk::Shared::WORLD_ID, std::string> Categories;
		std::vector<GUIDE_PROMPT> Prompts;
		std::vector<GUIDE_TRIGGER> Triggers;
		std::vector<GUIDE_COMBO> Combos;
		std::vector<GUIDE_COMMAND> Commands;
		// 룸의 대사 예약/송신과 Load 내부 참조 검사에 쓰는 비소유 포인터다. 없으면 nullptr이며 목록 교체·수정 뒤 기존 포인터를 재사용하지 않는다.
		const GUIDE_PROMPT* Find_Prompt(const std::string& id) const;
		// 룸의 도움 명령/실행과 Load 참조 검사가 같은 ID로 조회한다. 반환 주소의 수명은 현재 Combos 저장소에 한정된다.
		const GUIDE_COMBO* Find_Combo(const std::string& id) const;
	};

}
