# G01. BossToolTests 이름·Visual Studio 분류 정리

## 현재 상태와 목표

기존 ValtanPatternAuditionServiceHarness는 발탄 audition뿐 아니라 쿠크와 공용 편집·문서·연출도 검사한다.
filters 파일은 있지만 항목별 Filter가 없어 역할을 파악하기 어렵다.
폴더·프로젝트·실행 이름을 BossToolTests로 통일하고 시작점/검증 사례/Client 검사 대상을 구분한다.
구현 상태와 실제 검증은 같은 이름의 RESULT에 기록한다.

## 파일과 순서

1. 기존 Tools/ValtanPatternAuditionServiceHarness의 추적 소스를 Tools/BossToolTests로 이동한다. 기존 ignored Bin/Intermediate/user 파일은 이동·삭제하지 않는다.
2. Default 프로젝트와 filters의 파일명·프로젝트 이름을 BossToolTests로 변경한다. 프로젝트 GUID와 기본 솔루션 빌드 제외를 보존한다.
3. 기존 진입 CPP를 Private/Main.cpp로 바꾸고 Usage/결과 출력 이름만 바꾼다. 테스트 함수·호출 흐름·15개 옵션은 유지한다.
4. 아래 README와 필터를 추가하고 Framework.sln, build runner, 경로를 읽는 기존 Python 검사, CLAUDE와 현재 팀 설명 경로를 갱신한다. 과거 PLAN/RESULT는 당시 기록을 유지한다.
5. XML과 모든 경로/필터 대응, 관련 기존 Python 검사, x64 Debug 빌드와 개별 콘솔 검사를 확인한다.

Client/Engine/Shared/Server의 C++ 구현, Data, 기존 테스트용 매크로는 변경하지 않는다.
다른 작업의 Engine/Public/GameInstance.h 수정은 보존한다.

## Main.cpp 책임과 흐름

main은 개별 옵션을 검사해 해당 검사 함수에 분기하고, 기본 실행은 30개 audition 사례와 14개 그룹을 호출한다.
Fixture가 입력과 예상 상태를 구성하고 기존 Client 서비스 함수를 호출하며 실패 예외를 FAIL 로그와 종료 코드로 바꾼다.
새 상태·공개 API·알고리즘은 추가하지 않는다. 다른 Private CPP는 바이트 변경 없이 이동한다.

## C:/Users/tnest/Desktop/LostArk/Tools/BossToolTests/README.md

이름 변경 후 파일 전체:

````markdown
# BossToolTests — 보스 도구 자동 검사

발탄·쿠크 보스 도구를 수정했을 때 **기존 기능이 깨졌는지 검사하는 콘솔 프로그램**이다.
이전 프로젝트 이름은 `ValtanPatternAuditionServiceHarness`였다. 검사 범위가 쿠크와 공용 편집 코드까지 넓어져 `BossToolTests`로 바꿨다.

## 무엇을 검사하나

- **패턴 재생 요청:** Play, Next 예약, Restart, Clear 요청에 서버 응답을 흉내 낸 입력을 주어 상태가 올바르게 바뀌는지 검사한다. 늦은 응답, 거절, 타임아웃, 재시도 때 기존 예약과 요청 ID를 보존하는지도 확인한다.
- **편집 문서:** 패턴·애니메이션·이펙트·사운드 데이터의 읽기, 검증, 저장, 재로드와 실패 시 기존 상태 보존을 검사한다.
- **그래프·연출 계산:** 패턴 연결, 선택한 분기, 타임라인, 카메라·본 부착 계산 등을 검사한다.

실제 Client 서비스·문서 코드를 이 테스트 EXE에 함께 컴파일한다. 테스트 입력을 넣고 결과를 기대값과 비교한다. 게임 화면을 띄우거나 실제 서버의 패킷 처리량을 측정하지 않는다. IOCP·Job System 성능 비교와는 별도 프로젝트다.

## Visual Studio에서 읽는 순서

`Framework.sln`의 **BossToolTests** 프로젝트를 펼치고 `00.Start/README.md`부터 읽는다.
기본 솔루션 빌드에서는 제외되어 있으므로 필요한 때 이 프로젝트를 우클릭해 **빌드**한다.

| 필터 | 내용 |
|---|---|
| `00.Start` | 이 설명서와 `Main.cpp`. `main()`에서 실행 옵션과 테스트 호출 순서를 확인한다. |
| `01.Tests.Valtan` | 발탄의 흐름·튜닝·문서·연출 검증 사례. |
| `02.Tests.SharedAndKouku` | 공용 그래프·편집기·디버그·본 부착 검사. 쿠크 편집 검사는 `BossCompositionDocumentContractTests.cpp`에 있다. |
| `03.Client.Services` | 검사하는 실제 Client의 재생 요청·흐름·튜닝 서비스. |
| `04.Client.Documents` | 검사하는 실제 Client의 편집 문서·그래프·워크벤치 코드. |
| `05.Client.Presentation` | 검사하는 실제 Client의 연출·카메라·변환 코드. |
| `06.Client.Support` | 문서 파싱·경로·리소스 조회 등 보조 코드. |

`03`~`06`의 파일은 `Client/Private`, `Client/Public`에 있는 원본을 참조한다. 여기서 편집하면 Client 원본이 바뀐다. 필터는 VS 표시용 분류이며 실제 복사 폴더가 아니다.

## 실행과 결과

빌드 결과는 `Tools/BossToolTests/Bin/<Debug 또는 Release>/BossToolTests.exe`이다.
VS에서 이 프로젝트를 시작 프로젝트로 선택하면 콘솔 검사 프로그램을 실행할 수 있다.
디버거 작업 디렉터리는 저장소 루트로 설정되어 있다. 명령행에서도 저장소 루트를 작업 디렉터리로 사용한다.

```powershell
& '.\Tools\BossToolTests\Bin\Debug\BossToolTests.exe'
& '.\Tools\BossToolTests\Bin\Debug\BossToolTests.exe' --action-composition-graph-contract
```

인자가 없으면 audition 서비스 30개 사례와 추가 14개 그룹의 **기본 회귀 검사**를 실행한다. 처음 출력되는 `30/30 passed`는 audition 서비스만의 결과다. 기본 실행이 아래 모든 개별 옵션을 포함하지는 않는다.

| 개별 옵션 | 검사 범위 |
|---|---|
| `--action-composition-graph-contract` | 패턴 그래프와 분기·시간 계산 |
| `--cinematic-view-rebase-contract` | 시네마틱 카메라 view 재계산 |
| `--valtan-presentation-contract` | 발탄 연출 계약 |
| `--presentation-generation-admission-contract` | 연출 세대·리소스 승인 |
| `--kouku-pattern-delete-contract` | 쿠크 패턴 삭제 |
| `--kouku-collider-duplicate-contract` | 쿠크 collider 복제 |
| `--kouku-fixed-damage-contract` | 쿠크 고정 피해 편집 |
| `--kouku-collider-group-contract` | 쿠크 collider 그룹 |
| `--kouku-sound-timeline-contract` | 쿠크 사운드 타임라인 |
| `--kouku-preview-transport-contract` | 쿠크 preview/publisher 호출 경계 |
| `--kouku-sequence-document-contract` | 쿠크 sequence 문서 |
| `--kouku-parent-timing-contract` | 쿠크 순차 부모 시간 |
| `--kouku-independent-row-clock-contract` | 쿠크 독립 row 시간 |
| `--kouku-world-effect-frame-contract` | 쿠크 world 이펙트 좌표 |
| `--kouku-composition-editor-contract` | 쿠크 composition 편집 |

옵션은 한 번에 하나를 지정한다. 종료 코드는 성공 `0`, 검사 실패 `1`, 잘못된 인자 `2`이다. 실패 시 `FAIL` 행과 마지막 종료 코드를 확인한다.

## 실제 데이터와 검사 한계

일부 검사는 현재 `Data` 문서와 `Client/Bin/Resources` 참조를 읽는다. 저장·충돌 검사는 주로 임시 복사본에 수행한다. 생성 문서 저장 거부 검사는 실제 원본 경로에 저장을 시도하되 거부와 원본 바이트 보존을 확인한다. 쿠크 preview transport 검사는 임시 PowerShell publisher stub을 자식 프로세스로 실행한다.

검사 통과는 게임의 최종 화면, GPU 표시, 실제 서버 연결, 패킷 처리 속도까지 확인했다는 뜻이 아니다.
내부 매크로 `LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS`는 기존 Client 테스트 연결과 호환되도록 유지한다.

````

## C:/Users/tnest/Desktop/LostArk/Tools/BossToolTests/Private/Main.cpp

이름 변경 후 파일 전체:

```cpp
#include "ValtanPatternAuditionService.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

int Run_ValtanPresentationContractTests();
int Run_ValtanEncounterReferenceContractTests();
int Run_ValtanCanonicalGraphContractTests();
int Run_BossCompositionDocumentContractTests();
int Run_KoukuIndependentRowClockContractTests();
int Run_KoukuSequentialParentTimingContractTests();
int Run_KoukuCompositionEditorContractTests();
int Run_KoukuWorldEffectFrameContractTests();
int Run_KoukuPatternDeleteContractTests();
int Run_KoukuSequenceDocumentContractTests();
int Run_KoukuPreviewTransportContractTests();
int Run_KoukuSoundTimelineContractTests();
int Run_KoukuColliderGroupContractTests();
int Run_KoukuFixedDamageContractTests();
int Run_KoukuColliderDuplicateContractTests();
int Run_ActionCompositionGraphModelContractTests();
int Run_BossLogicFlowViewModelContractTests();
int Run_ValtanPatternSoundCueDocumentContractTests();
int Run_ValtanPatternAnimationBindingDocumentContractTests();
int Run_ValtanPatternEffectCueAuthoringContractTests();
int Run_ValtanPresentationGenerationAdmissionContractTests();
int Run_CombatDebugVisibilityContractTests();
int Run_PlayerHandGripTransformContractTests();

using namespace Client;
using namespace LostArk::Shared;

int Run_ValtanPatternFlowServiceTests();
int Run_ValtanTuningCommandServiceTests();

namespace
{
	constexpr const char* BOSS = "boss.valtan.center";
	constexpr const char* A = "VALTAN_FOUR_SLASH";
	constexpr const char* B = "VALTAN_FIST_IN_OUT";
	constexpr const char* C = "VALTAN_TRASH";
	constexpr uint32_t EPOCH = 17u;
	constexpr uint32_t SEQUENCE = 31u;

	GameplayDataRevision ActiveRevision()
	{
		GameplayDataRevision Revision{};
		Revision.Bytes.fill(0x42u);
		return Revision;
	}

	GameplayDataRevision ReplacementRevision()
	{
		GameplayDataRevision Revision{};
		Revision.Bytes.fill(0x24u);
		return Revision;
	}

	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT SoundReceipt(
		const char Value = 'a')
	{
		return { std::string(64u, Value), 1u };
	}

	void Require(bool Condition, const char* Message)
	{
		if (!Condition)
			throw std::runtime_error(Message);
	}

	S2C_VALTAN_AUDITION_RESULT Verdict(
		const C2S_VALTAN_AUDITION_REQUEST& Request,
		VALTAN_AUDITION_RESULT Result = VALTAN_AUDITION_RESULT::QUEUED)
	{
		S2C_VALTAN_AUDITION_RESULT Out{};
		Out.iRequestSequence = Request.iRequestSequence;
		Out.eOperation = Request.eOperation;
		Out.iTargetHealthBar = Request.iTargetHealthBar;
		Out.strBossPlacementId = Request.strBossPlacementId;
		Out.strPatternId = Request.strPatternId;
		Out.iPredecessorRoomAuditionEpoch = Request.iPredecessorRoomAuditionEpoch;
		Out.iPredecessorPatternSequence = Request.iPredecessorPatternSequence;
		Out.iExpectedNextRequestSequence = Request.iExpectedNextRequestSequence;
		Out.ExpectedDefinitionRevision = Request.ExpectedDefinitionRevision;
		Out.ReplacementDefinitionRevision =
			Request.ReplacementDefinitionRevision;
		Out.eResult = Result;
		return Out;
	}

	S2C_VALTAN_AUDITION_LIFECYCLE Event(
		const C2S_VALTAN_AUDITION_REQUEST& Request,
		VALTAN_AUDITION_LIFECYCLE_STATE State)
	{
		S2C_VALTAN_AUDITION_LIFECYCLE Out{};
		Out.iRequestSequence = Request.iRequestSequence;
		Out.iRoomAuditionEpoch =
			(VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID == Request.eOperation ||
			 VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID == Request.eOperation) ?
				EPOCH : Request.iPredecessorRoomAuditionEpoch;
		Out.iPatternSequence = VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID == Request.eOperation ?
			SEQUENCE : Request.iPredecessorPatternSequence + 1u;
		Out.strPatternId = Request.strPatternId;
		Out.eState = State;
		Out.PinnedDefinitionRevision =
			VALTAN_AUDITION_OPERATION::RESTART_PATTERN_ID == Request.eOperation ?
				Request.ReplacementDefinitionRevision : ActiveRevision();
		if (VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED == State)
			Out.strReason = "harness terminal abort";
		return Out;
	}

	std::vector<uint8_t> Wire(const C2S_VALTAN_AUDITION_REQUEST& Request)
	{
		CPacketWriter Writer;
		Require(Write_Message(Writer, Request), "production request codec rejected a service command");
		return Writer.Get_Buffer();
	}

	struct Fixture
	{
		CValtanPatternAuditionService& Service = CValtanPatternAuditionService::Get();
		std::string Status;
		C2S_VALTAN_AUDITION_REQUEST Current;

		Fixture() { Service.Harness_Reset(); }
		auto& Input() { return Service.Harness_Input(); }

		void StartAt(
			const char* Pattern,
			const uint32_t PatternSequence,
			const char* Consumer = "Valtan Boss Tool")
		{
			Require(Service.Submit(
				Consumer, BOSS, Pattern, ActiveRevision(), Status),
				"initial Play failed");
			Require(Status.find("only Valtan") != std::string::npos &&
				Status.find("current arena") != std::string::npos &&
				Status.find("first Stage") != std::string::npos,
				"single Pattern Play status did not distinguish boss-only replay from Flow reset");
			Current = Input().SentRequests.back();
			Input().Results.push_back(Verdict(Current));
			auto Active = Event(Current, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE);
			Active.iPatternSequence = PatternSequence;
			Input().Lifecycles.push_back(Active);
			Service.Update();
			Require(Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE,
				"initial Play did not activate");
		}

		void Start(const char* Pattern = A, const char* Consumer = "Valtan Boss Tool")
		{
			StartAt(Pattern, SEQUENCE, Consumer);
		}

		C2S_VALTAN_AUDITION_REQUEST Queue(const char* Pattern = B)
		{
			Require(Service.Queue_NextPattern(
				"Valtan Boss Tool", BOSS, Pattern, ActiveRevision(), Status),
				"Next command failed");
			return Input().SentRequests.back();
		}

		C2S_VALTAN_AUDITION_REQUEST Reserve(const char* Pattern = B)
		{
			auto Request = Queue(Pattern);
			Input().Results.push_back(Verdict(Request));
			Service.Update();
			Require(Service.Get_NextSnapshot().Is_Live(), "approved reservation was not live");
			return Request;
		}

		void Complete()
		{
			Input().Lifecycles.push_back(Event(Current, VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED));
			Service.Update();
		}

		void Advance(uint64_t Milliseconds)
		{
			Input().iNowMilliseconds += Milliseconds;
			Service.Update();
		}
	};

	void VerifyAuthoritativePredecessorAndNoReset()
	{
		Fixture F;
		Require(!F.Service.Queue_NextPattern(
			"Valtan Boss Tool", BOSS, B, ActiveRevision(), F.Status),
			"Next accepted without a live boss or audition");
		Require(F.Service.Submit(
			"Effect Tool", BOSS, A, ActiveRevision(), F.Status),
			"Play failed");
		F.Current = F.Input().SentRequests.back();
		Require(!F.Service.Queue_NextPattern(
			"Valtan Boss Tool", BOSS, B, ActiveRevision(), F.Status),
			"Next accepted without an authoritative predecessor epoch");
		F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_AUDITION_LIFECYCLE_STATE::PENDING));
		F.Service.Update();
		auto StaleRevision = ActiveRevision();
		StaleRevision.Bytes.front() ^= 0xffu;
		Require(!F.Service.Queue_NextPattern(
			"Valtan Boss Tool", BOSS, B, StaleRevision, F.Status),
			"Next accepted a revision other than the isolated predecessor pin");
		const auto Next = F.Queue();
		Require(Next.eOperation == VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID &&
			Next.iPredecessorRoomAuditionEpoch == EPOCH &&
			Next.iPredecessorPatternSequence == SEQUENCE &&
			Next.iExpectedNextRequestSequence == 0u &&
			Next.ExpectedDefinitionRevision == ActiveRevision(),
			"Next did not carry the authoritative predecessor");
		Require(F.Input().SentRequests.size() == 2u &&
			F.Service.Get_Snapshot().iRequestSequence == F.Current.iRequestSequence &&
			F.Service.Get_Snapshot().strConsumerId == "Effect Tool" &&
			!F.Service.Get_NextSnapshot().Is_Live(), "Queue reset or overwrote the current owner");
		(void)Wire(Next);
	}

	void VerifyCompletionBeforeVerdictAndPending()
	{
		Fixture F;
		F.Start();
		auto Next = F.Queue();
		F.Complete();
		Require(F.Service.Has_PlaybackOwnership(), "completion dropped pending Next ownership");
		F.Input().Results.push_back(Verdict(Next));
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::PENDING));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::COMPLETED &&
			F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::START_PENDING &&
			F.Service.Get_NextSnapshot().bReservationConsumed,
			"Next PENDING replaced the completed current snapshot");
		const auto SentBeforeConsumedControls = F.Input().SentRequests.size();
		Require(!F.Service.Clear_NextPattern(F.Status) &&
			!F.Service.Queue_NextPattern(
				"Valtan Boss Tool", BOSS, C, ActiveRevision(), F.Status) &&
			SentBeforeConsumedControls == F.Input().SentRequests.size(),
			"promoted Next sent a clear or replacement against the consumed token");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::WAITING_FOR_PLAYER &&
			F.Service.Get_NextSnapshot().bReservationConsumed &&
			!F.Service.Clear_NextPattern(F.Status) &&
			!F.Service.Queue_NextPattern(
				"Effect Tool", BOSS, C, ActiveRevision(), F.Status) &&
			SentBeforeConsumedControls == F.Input().SentRequests.size(),
			"target loss hid WAITING or reopened a consumed reservation");
		F.Advance(60000u);
		Require(F.Service.Get_NextSnapshot().Is_Live(), "Next inherited Play's 15s timeout");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().strPatternId == B &&
			F.Service.Get_Snapshot().iRequestSequence == Next.iRequestSequence &&
			F.Service.Get_Snapshot().iObservedPatternSequence == SEQUENCE + 1u &&
			!F.Service.Get_NextSnapshot().Is_Live(), "Next ACTIVE did not transfer its exact identity");
		const auto Following = F.Queue(C);
		Require(Following.iPredecessorPatternSequence == SEQUENCE + 1u &&
			Following.iExpectedNextRequestSequence == 0u,
			"ACTIVE did not reopen Next with the promoted predecessor identity");
	}

	void VerifyVerdictBeforeCompletionAndDeadPlayerWait()
	{
		Fixture F;
		F.Start();
		auto Next = F.Reserve();
		F.Advance(60000u);
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
			F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::RESERVED,
			"long current pattern expired Next");
		F.Complete();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER));
		F.Service.Update();
		F.Advance(120000u);
		Require(F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::WAITING_FOR_PLAYER &&
			!F.Service.Get_NextSnapshot().bReservationConsumed &&
			F.Service.Has_PlaybackOwnership(), "dead-player wait lost reservation ownership");
		Require(!F.Service.Submit(
			"Effect Tool", BOSS, C, ActiveRevision(), F.Status),
			"another tool reset a waiting Next");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().strPatternId == B, "revive-triggered ACTIVE was not consumed");
	}

	void VerifyLifecycleBeforeVerdict()
	{
		Fixture F;
		F.Start();
		auto Next = F.Queue();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().iRequestSequence == Next.iRequestSequence &&
			!F.Service.Has_PendingNextCommand(), "matching lifecycle did not confirm the candidate");
		F.Complete();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		F.Input().Results.push_back(Verdict(Next));
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
			!F.Service.Get_NextSnapshot().Is_Live(), "late verdict recreated an activated reservation");
	}

	void VerifyOutcomeFollowupRebasesNextOccurrence()
	{
		Fixture F;
		F.Start();
		const auto Next = F.Queue();
		F.Input().Lifecycles.push_back(
			Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED));
		F.Service.Update();
		F.Complete();

		/* Two outcome-owned children used root+1 and root+2. The queued user
		   pattern is therefore born at root+3 even though its immutable command
		   CAS still names the root occurrence. */
		auto Pending = Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::PENDING);
		Pending.iPatternSequence = SEQUENCE + 3u;
		F.Input().Lifecycles.push_back(Pending);
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().eState ==
				VALTAN_NEXT_PATTERN_STATE::START_PENDING &&
			F.Service.Get_NextSnapshot().iPredecessorPatternSequence == SEQUENCE &&
			F.Service.Get_NextSnapshot().iExpectedPatternSequence == SEQUENCE + 3u &&
			F.Service.Get_NextSnapshot().bReservationConsumed,
			"outcome chain did not rebase the consumed Next occurrence");

		F.Input().Lifecycles.push_back(
			Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().iExpectedPatternSequence == SEQUENCE + 3u,
			"late root+1 reservation lifecycle regressed the rebased occurrence");

		auto Active = Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE);
		Active.iPatternSequence = SEQUENCE + 3u;
		F.Input().Lifecycles.push_back(Active);
		F.Service.Update();
		Require(F.Service.Get_Snapshot().strPatternId == B &&
			F.Service.Get_Snapshot().iObservedPatternSequence == SEQUENCE + 3u,
			"rebased Next ACTIVE lifecycle was discarded by the Client service");
		const auto Following = F.Queue(C);
		Require(Following.iPredecessorPatternSequence == SEQUENCE + 3u,
			"rebased Next did not become the next command's predecessor");
	}

	void VerifyRejectedReplacementPreservesReservation()
	{
		Fixture F;
		F.Start();
		auto Next = F.Reserve();
		auto Replacement = F.Queue(C);
		Require(Replacement.iExpectedNextRequestSequence == Next.iRequestSequence &&
			F.Service.Get_NextSnapshot().strPatternId == B, "replacement modified B before approval");
		F.Input().Results.push_back(Verdict(Replacement, VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().iRequestSequence == Next.iRequestSequence &&
			F.Service.Get_NextSnapshot().strPatternId == B && !F.Service.Has_PendingNextCommand(),
			"rejected replacement erased B");
		F.Complete();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().strPatternId == B, "B could not activate after C rejection");
	}

	void VerifyAcceptedReplacementIgnoresOldAbort()
	{
		Fixture F;
		F.Start();
		auto Next = F.Reserve();
		F.Complete();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER));
		F.Service.Update();
		auto Replacement = F.Queue(C);
		F.Input().Results.push_back(Verdict(Replacement));
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().strPatternId == C &&
			F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::RESERVED,
			"old B abort erased accepted C");
		F.Complete();
		F.Input().Lifecycles.push_back(Event(Replacement, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().iRequestSequence == Replacement.iRequestSequence,
			"old B ACTIVE replaced C");
	}

	void VerifyClearTimeoutRetryAndReplayedVerdict()
	{
		Fixture F;
		F.Start();
		auto Next = F.Reserve();
		F.Complete();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER));
		F.Service.Update();
		Require(F.Service.Clear_NextPattern(F.Status), "clear failed");
		const auto Clear = F.Input().SentRequests.back();
		Require(Clear.eOperation == VALTAN_AUDITION_OPERATION::CLEAR_NEXT_PATTERN_ID &&
			Clear.iExpectedNextRequestSequence == Next.iRequestSequence &&
			Clear.strPatternId == B, "clear did not name the exact B reservation");
		F.Advance(5001u);
		Require(F.Service.Get_NextCommand().eState == VALTAN_NEXT_COMMAND_STATE::UNCONFIRMED &&
			F.Service.Get_NextSnapshot().Is_Live() && F.Service.Has_PlaybackOwnership(),
			"clear timeout dropped ownership");
		Require(!F.Service.Queue_NextPattern(
			"Valtan Boss Tool", BOSS, C, ActiveRevision(), F.Status) &&
			!F.Service.Submit(
				"Effect Tool", BOSS, C, ActiveRevision(), F.Status),
			"unconfirmed clear allowed a new command");
		Require(F.Service.Retry_NextPatternCommand(F.Status), "same clear retry failed");
		Require(Wire(Clear) == Wire(F.Input().SentRequests.back()), "retry changed the original payload");
		F.Input().Results.push_back(Verdict(Clear, VALTAN_AUDITION_RESULT::CLEARED));
		F.Service.Update();
		F.Input().Results.push_back(Verdict(Clear, VALTAN_AUDITION_RESULT::CLEARED));
		F.Service.Update();
		Require(!F.Service.Get_NextSnapshot().Is_Live() && !F.Service.Has_PlaybackOwnership(),
			"replayed CLEARED left ownership behind");
		const auto NewNext = F.Queue(C);
		Require(NewNext.iRequestSequence == Clear.iRequestSequence + 1u,
			"retry allocated a new command sequence");
	}

	void VerifyRejectedClearPreservesReservation()
	{
		Fixture F;
		F.Start();
		auto Next = F.Reserve();
		Require(F.Service.Clear_NextPattern(F.Status), "clear failed");
		const auto Clear = F.Input().SentRequests.back();
		F.Input().Results.push_back(Verdict(Clear, VALTAN_AUDITION_RESULT::REJECTED_NEXT_CHANGED));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().iRequestSequence == Next.iRequestSequence &&
			F.Service.Get_NextSnapshot().Is_Live(), "clear rejection erased approved B");
	}

	void VerifyFullEchoAndUnexpectedDuplicate()
	{
		Fixture F;
		F.Start();
		auto Next = F.Queue();
		for (int Field = 0; Field < 8; ++Field)
		{
			auto Wrong = Verdict(Next);
			switch (Field)
			{
			case 0: ++Wrong.iRequestSequence; break;
			case 1: Wrong.eOperation = VALTAN_AUDITION_OPERATION::CLEAR_NEXT_PATTERN_ID; break;
			case 2: Wrong.strBossPlacementId = "boss.valtan.other"; break;
			case 3: Wrong.strPatternId = C; break;
			case 4: ++Wrong.iPredecessorRoomAuditionEpoch; break;
			case 5: ++Wrong.iPredecessorPatternSequence; break;
			case 6: ++Wrong.iExpectedNextRequestSequence; break;
			case 7: ++Wrong.iTargetHealthBar; break;
			}
			F.Input().Results.push_back(Wrong);
			F.Service.Update();
			Require(F.Service.Has_PendingNextCommand() && !F.Service.Get_NextSnapshot().Is_Live(),
				"partial echoed identity was accepted");
		}
		F.Input().Results.push_back(Verdict(Next, VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED));
		F.Service.Update();
		Require(F.Service.Get_NextCommand().eState == VALTAN_NEXT_COMMAND_STATE::UNCONFIRMED &&
			!F.Service.Get_NextSnapshot().Is_Live(), "legacy duplicate verdict was treated as Next approval");
		Require(F.Service.Retry_NextPatternCommand(F.Status), "unconfirmed duplicate could not retry");
		F.Input().Results.push_back(Verdict(Next));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().Is_Live(), "original QUEUED verdict did not resolve retry");
	}

	void VerifyGenerationAndDisconnectBeforeDrain()
	{
		for (bool Disconnect : { false, true })
		{
			Fixture F;
			F.Start();
			auto Next = F.Reserve();
			F.Complete();
			Require(F.Service.Clear_NextPattern(F.Status), "clear failed");
			const auto Clear = F.Input().SentRequests.back();
			F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
			F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED));
			F.Input().Results.push_back(Verdict(Clear, VALTAN_AUDITION_RESULT::CLEARED));
			if (Disconnect)
				F.Input().bConnected = false;
			else
				++F.Input().iWorldInboundGeneration;
			F.Service.Update();
			Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ABORTED &&
				F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::ABORTED &&
				!F.Service.Has_PlaybackOwnership(), "session change adopted late packets");
			F.Input().bConnected = true;
			F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
			F.Service.Update();
			Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ABORTED,
				"late packet resurrected the previous connection");
			Require(F.Service.Submit(
				"Effect Tool", BOSS, C, ActiveRevision(), F.Status),
				"new world could not start a new audition");
			Require(F.Service.Get_Snapshot().iRequestSequence > Clear.iRequestSequence,
				"new world reused an old request sequence");
		}
	}

	void VerifySamePatternOccurrencesRemainDistinct()
	{
		Fixture F;
		F.Start(A);
		auto Next = F.Reserve(A);
		F.Complete();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
			F.Service.Get_Snapshot().iRequestSequence == Next.iRequestSequence &&
			F.Service.Get_Snapshot().iObservedPatternSequence == SEQUENCE + 1u,
			"A completion terminated the second A");
		F.Service.Harness_ObserveBoss(true, C, SEQUENCE + 3u);
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE,
			"HUD inference completed an authoritative Next");
		auto Wrong = Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED);
		++Wrong.iRoomAuditionEpoch;
		F.Input().Lifecycles.push_back(Wrong);
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE,
			"mismatched epoch completed the second A");
	}

	void VerifyFlowConflictSendRollbackAndExhaustion()
	{
		Fixture F;
		F.Input().bFlowInFlight = true;
		Require(!F.Service.Submit(
			"Valtan Boss Tool", BOSS, A, ActiveRevision(), F.Status) &&
			F.Input().SentRequests.empty(),
			"Play bypassed ordered Flow ownership");
		F.Input().bFlowInFlight = false;
		F.Start();
		auto Next = F.Reserve();
		F.Input().bFlowInFlight = true;
		F.Input().bFlowStartPending = true;
		Require(!F.Service.Queue_NextPattern(
			"Valtan Boss Tool", BOSS, C, ActiveRevision(), F.Status),
			"Next bypassed pending Flow restart");
		F.Input().bFlowStartPending = false;
		F.Input().bFlowInFlight = false;
		F.Input().bSendSucceeds = false;
		Require(!F.Service.Queue_NextPattern(
			"Valtan Boss Tool", BOSS, C, ActiveRevision(), F.Status) &&
			F.Service.Get_NextSnapshot().iRequestSequence == Next.iRequestSequence &&
			!F.Service.Has_PendingNextCommand(), "send failure mutated an approved reservation");
		F.Input().bSendSucceeds = true;
		F.Service.Harness_SetNextRequestSequence((std::numeric_limits<uint32_t>::max)());
		auto Last = F.Queue(C);
		F.Input().Results.push_back(Verdict(Last, VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE));
		F.Service.Update();
		const auto Sent = F.Input().SentRequests.size();
		Require(!F.Service.Clear_NextPattern(F.Status) && F.Input().SentRequests.size() == Sent,
			"request counter wrapped into a reused identity");
	}

	void VerifyNextAbortAndPresentationIsolation()
	{
		Fixture F;
		F.Start();
		auto Next = F.Reserve();
		F.Complete();
		F.Input().bPresentationAvailable = false;
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
			!F.Service.Get_Snapshot().isPresentationRevisionAvailable,
			"missing presentation changed authoritative playback");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED));
		F.Service.Update();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ABORTED &&
			!F.Service.Has_PlaybackOwnership(), "aborted Next resurrected");
	}

	void VerifyLiveProductWithoutIsolatedPlay()
	{
		Fixture F;
		F.Input().bLiveBossValid = true;
		F.Input().iLivePatternSequence = SEQUENCE;
		Require(F.Service.Can_QueueNextPattern(
			BOSS, ActiveRevision(), F.Status),
			"live Product Next picker was disabled");
		const auto Next = F.Queue();
		Require(Next.eOperation == VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID &&
			Next.iPredecessorRoomAuditionEpoch == 0u && Next.iExpectedNextRequestSequence == 0u &&
			Next.iPredecessorPatternSequence == SEQUENCE, "live Next invented an audition predecessor");
		Require(F.Input().SentRequests.size() == 1u &&
			F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::IDLE &&
			F.Service.Has_PlaybackOwnership(), "live Next reset or fabricated the current audition");
		(void)Wire(Next);
		F.Input().Results.push_back(Verdict(Next));
		F.Service.Update();
		Require(F.Service.Has_PendingNextCommand() && !F.Service.Get_NextSnapshot().Is_Live(),
			"epoch-zero echo was treated as an authoritative reservation");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().iRoomAuditionEpoch == EPOCH &&
			F.Service.Get_NextSnapshot().iPredecessorPatternSequence == SEQUENCE &&
			F.Service.Get_NextSnapshot().iExpectedPatternSequence == SEQUENCE + 1u &&
			!F.Service.Has_PendingNextCommand(), "live reservation did not adopt its Server-issued epoch");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().strPatternId == B &&
			F.Service.Get_Snapshot().iObservedPatternSequence == SEQUENCE + 1u,
			"live Next did not promote its authoritative occurrence");
		const auto Following = F.Queue(C);
		Require(Following.eOperation == VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID &&
			Following.iPredecessorRoomAuditionEpoch == EPOCH &&
			Following.iPredecessorPatternSequence == SEQUENCE + 1u,
			"promoted live Next did not continue the same isolated chain");
	}

	void VerifyLiveEpochIdentityAndExactRetry()
	{
		Fixture F;
		F.Input().bLiveBossValid = true;
		F.Input().iLivePatternSequence = SEQUENCE;
		const auto Next = F.Queue();
		for (int Field = 0; Field < 4; ++Field)
		{
			auto Wrong = Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED);
			switch (Field)
			{
			case 0: ++Wrong.iRequestSequence; break;
			case 1: Wrong.iRoomAuditionEpoch = 0u; break;
			case 2: ++Wrong.iPatternSequence; break;
			case 3: Wrong.strPatternId = C; break;
			}
			F.Input().Lifecycles.push_back(Wrong);
			F.Service.Update();
			Require(F.Service.Has_PendingNextCommand() && !F.Service.Get_NextSnapshot().Is_Live(),
				"mismatched live lifecycle claimed the reservation");
		}
		auto WrongEcho = Verdict(Next);
		WrongEcho.iPredecessorRoomAuditionEpoch = EPOCH;
		F.Input().Results.push_back(WrongEcho);
		F.Service.Update();
		Require(F.Service.Has_PendingNextCommand(), "live result accepted a non-echoed epoch");
		F.Input().Results.push_back(Verdict(Next));
		F.Service.Update();
		F.Advance(5001u);
		Require(F.Service.Get_NextCommand().eState == VALTAN_NEXT_COMMAND_STATE::UNCONFIRMED &&
			F.Service.Has_PlaybackOwnership(), "missing live lifecycle silently dropped ownership");
		Require(F.Service.Retry_NextPatternCommand(F.Status) &&
			Wire(Next) == Wire(F.Input().SentRequests.back()), "live retry changed its original request");
		F.Input().bPresentationAvailable = false;
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().Is_Live() &&
			!F.Service.Get_NextSnapshot().isPresentationRevisionAvailable &&
			!F.Service.Has_PendingNextCommand(), "matching live lifecycle could not resolve the retry");
	}

	void VerifyLiveLifecycleBeforeVerdict()
	{
		for (const auto State : { VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED,
			VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER,
			VALTAN_AUDITION_LIFECYCLE_STATE::PENDING,
			VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE,
			VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED,
			VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED })
		{
			Fixture F;
			F.Input().bLiveBossValid = true;
			F.Input().iLivePatternSequence = SEQUENCE;
			const auto Next = F.Queue();
			F.Input().Lifecycles.push_back(Event(Next, State));
			F.Service.Update();
			Require(!F.Service.Has_PendingNextCommand(), "live lifecycle could not overtake its verdict");
			const bool bPromoted = VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE == State ||
				VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED == State;
			if (bPromoted)
				Require(F.Service.Get_Snapshot().iRequestSequence == Next.iRequestSequence &&
					F.Service.Get_Snapshot().iRoomAuditionEpoch == EPOCH,
					"live occurrence lost its identity without RESERVED or PENDING");
			else
				Require(F.Service.Get_NextSnapshot().iRequestSequence == Next.iRequestSequence &&
					F.Service.Get_NextSnapshot().iRoomAuditionEpoch == EPOCH,
					"live reservation lost its identity without a verdict");
			F.Input().Results.push_back(Verdict(Next));
			F.Service.Update();
			Require(!F.Service.Has_PendingNextCommand() &&
				(bPromoted || VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED == State ?
					!F.Service.Get_NextSnapshot().Is_Live() : F.Service.Get_NextSnapshot().Is_Live()),
				"late live verdict resurrected or regressed the reservation");
		}
	}

	void VerifyLiveReservationReplacementAndClear()
	{
		Fixture F;
		F.Input().bLiveBossValid = true;
		F.Input().iLivePatternSequence = SEQUENCE;
		const auto Next = F.Queue();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED));
		F.Service.Update();
		const auto Pin = F.Service.Get_NextSnapshot().PinnedDefinitionRevision;
		const auto Replacement = F.Queue(C);
		Require(Replacement.eOperation == VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID &&
			Replacement.iPredecessorRoomAuditionEpoch == EPOCH &&
			Replacement.iPredecessorPatternSequence == SEQUENCE &&
			Replacement.iExpectedNextRequestSequence == Next.iRequestSequence &&
			F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::IDLE,
			"replacement needed a fabricated current audition instead of the reservation tuple");
		F.Input().Results.push_back(Verdict(Replacement, VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().iRequestSequence == Next.iRequestSequence,
			"rejected live replacement erased the approved reservation");
		const auto Accepted = F.Queue(C);
		F.Input().Results.push_back(Verdict(Accepted));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().PinnedDefinitionRevision == Pin,
			"live replacement lost its pin because there was no current audition snapshot");
		Require(F.Service.Clear_NextPattern(F.Status), "live reservation could not be cancelled");
		const auto Clear = F.Input().SentRequests.back();
		Require(Clear.iPredecessorRoomAuditionEpoch == EPOCH &&
			Clear.iPredecessorPatternSequence == SEQUENCE &&
			Clear.iExpectedNextRequestSequence == Accepted.iRequestSequence &&
			Clear.ExpectedDefinitionRevision == Pin && Clear.strPatternId == C,
			"live clear did not use the adopted epoch and exact reservation identity");
		(void)Wire(Replacement);
		(void)Wire(Clear);
		F.Input().Results.push_back(Verdict(Clear, VALTAN_AUDITION_RESULT::CLEARED));
		F.Service.Update();
		Require(!F.Service.Has_PlaybackOwnership(), "live cancellation kept stale playback ownership");
	}

	void VerifyLiveIdleConsumedPlayerWait()
	{
		Fixture F;
		F.Input().bLiveBossValid = true;
		const auto Next = F.Queue();
		Require(Next.iPredecessorPatternSequence == 0u, "initial idle invented a completed predecessor");
		(void)Wire(Next);
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED));
		F.Service.Update();
		Require(!F.Service.Can_QueueNextPattern(
			BOSS, ActiveRevision(), F.Status) &&
			!F.Service.Clear_NextPattern(F.Status), "initial idle issued a forbidden zero-sequence CAS control");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::PENDING));
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER));
		F.Service.Update();
		F.Advance(60000u);
		Require(F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::WAITING_FOR_PLAYER &&
			F.Service.Get_NextSnapshot().bReservationConsumed &&
			!F.Service.Clear_NextPattern(F.Status) &&
			!F.Service.Queue_NextPattern(
				"Valtan Boss Tool", BOSS, C, ActiveRevision(), F.Status),
			"idle player wait timed out or reopened the consumed reservation");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().iObservedPatternSequence == 1u &&
			F.Service.Get_Snapshot().iRoomAuditionEpoch == EPOCH && F.Input().SentRequests.size() == 1u,
			"idle Next did not start sequence one without a reset command");
	}

	void VerifyLiveFlowAndReadinessRevalidation()
	{
		Fixture F;
		F.Input().bLiveBossValid = true;
		F.Input().iLivePatternSequence = SEQUENCE;
		F.Input().bFlowInFlight = true;
		Require(F.Service.Can_QueueNextPattern(
			BOSS, ActiveRevision(), F.Status),
			"active Flow disabled live Next");
		F.Input().bFlowStartPending = true;
		Require(!F.Service.Can_QueueNextPattern(
			BOSS, ActiveRevision(), F.Status) &&
			!F.Service.Queue_NextPattern(
				"Valtan Boss Tool", BOSS, B, ActiveRevision(), F.Status),
			"Next submission did not recheck a newly pending Flow restart");
		F.Input().bFlowStartPending = false;
		const auto Rejected = F.Queue();
		Require(Rejected.eOperation == VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID,
			"active Flow used isolated Next without an authoritative epoch");
		F.Input().Results.push_back(Verdict(Rejected, VALTAN_AUDITION_RESULT::REJECTED_NOT_OWNER));
		F.Service.Update();
		Require(!F.Service.Has_PlaybackOwnership() &&
			F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::REJECTED &&
			F.Input().bFlowInFlight, "rejected Next cancelled the original Flow locally");
		F.Input().bLiveBossAlive = false;
		Require(!F.Service.Can_QueueNextPattern(
			BOSS, ActiveRevision(), F.Status), "dead live boss enabled Next");
		F.Input().bLiveBossAlive = true;
		F.Input().iLivePatternSequence = (std::numeric_limits<uint32_t>::max)();
		Require(!F.Service.Can_QueueNextPattern(
			BOSS, ActiveRevision(), F.Status), "live predecessor sequence wrapped");
		F.Input().iLivePatternSequence = SEQUENCE;
		F.Input().bSendSucceeds = false;
		const auto Sent = F.Input().SentRequests.size();
		Require(!F.Service.Queue_NextPattern(
			"Valtan Boss Tool", BOSS, B, ActiveRevision(), F.Status) &&
			!F.Service.Has_PendingNextCommand() && F.Input().SentRequests.size() == Sent,
			"failed live send created ownership or consumed a request identity");
		F.Input().bSendSucceeds = true;
		const auto Next = F.Queue();
		Require(Next.iRequestSequence == Rejected.iRequestSequence + 1u,
			"failed live send consumed a request sequence");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED));
		F.Service.Update();
		F.Input().bFlowInFlight = false;
		Require(F.Service.Get_NextSnapshot().Is_Live(), "live Flow Next lost ownership on Flow termination");
	}

	void VerifyNewWorldLiveNextAfterOldAudition()
	{
		for (const bool bPreviousLive : { false, true })
		{
			Fixture F;
			F.Input().bLiveBossValid = true;
			F.Input().iLivePatternSequence = SEQUENCE;
			C2S_VALTAN_AUDITION_REQUEST OldRequest{};
			if (bPreviousLive)
				OldRequest = F.Queue();
			else
			{
				F.Start();
				OldRequest = F.Current;
				F.Complete();
			}
			++F.Input().iWorldInboundGeneration;
			F.Input().Lifecycles.push_back(Event(OldRequest, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
			F.Service.Update();
			Require(!F.Service.Has_PlaybackOwnership(), "world change retained old live command ownership");
			const auto Next = F.Queue(C);
			F.Input().Results.push_back(Verdict(OldRequest));
			F.Input().Lifecycles.push_back(Event(OldRequest, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
			F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
			F.Service.Update();
			Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
				F.Service.Get_Snapshot().iRequestSequence == Next.iRequestSequence &&
				F.Service.Get_Snapshot().iWorldInboundGeneration == F.Input().iWorldInboundGeneration,
				"old terminal generation kept discarding the new live Next lifecycle");
		}
	}

	void VerifyFlowResetInvalidatesCompletedAudition()
	{
		Fixture F;
		F.Start();
		F.Complete();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::COMPLETED,
			"fixture did not retain the isolated completed hold");
		F.Input().bFlowInFlight = true;
		F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ABORTED,
			"Flow reset did not retire the exact completed isolated epoch");
		F.Input().bFlowInFlight = false;
		F.Input().bLiveBossValid = true;
		F.Input().iLivePatternSequence = SEQUENCE + 8u;
		const auto Next = F.Queue();
		Require(Next.eOperation == VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID &&
			Next.iPredecessorRoomAuditionEpoch == 0u &&
			Next.iPredecessorPatternSequence == F.Input().iLivePatternSequence &&
			Next.iExpectedNextRequestSequence == 0u,
			"Next after completed Flow reused a retired isolated predecessor tuple");
		(void)Wire(Next);
		auto LiveReservation = Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED);
		F.Input().Lifecycles.push_back(LiveReservation);
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().Is_Live() &&
			F.Service.Get_NextSnapshot().PinnedDefinitionRevision ==
				Next.ExpectedDefinitionRevision,
			"live Next did not retain the exact definition revision sent in its CAS request");
		auto WrongRevision = Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER);
		WrongRevision.PinnedDefinitionRevision.Bytes.fill(0x43u);
		F.Input().Lifecycles.push_back(WrongRevision);
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::RESERVED &&
			F.Service.Get_NextSnapshot().PinnedDefinitionRevision ==
				LiveReservation.PinnedDefinitionRevision,
			"a later live Next lifecycle changed its pinned definition revision");
	}

	void VerifyDataDrivenOccurrencesRetainRequestAndAdvanceNextPredecessor()
	{
		for (const char* PatternId : { A, "VALTAN_GHOST_FINALE" })
		{
			for (const uint32_t Initial : { SEQUENCE, (std::numeric_limits<uint32_t>::max)() })
			{
				for (const auto TerminalState : {
					VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED,
					VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED })
				{
					Fixture F;
					F.StartAt(PatternId, Initial);
					auto First = Event(F.Current, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE);
					First.iPatternSequence = Initial;
					auto ActiveOccurrence = First;
					ActiveOccurrence.iPatternSequence =
						Initial == (std::numeric_limits<uint32_t>::max)() ? 1u : Initial + 1u;
					F.Input().Lifecycles.push_back(ActiveOccurrence);
					F.Service.Update();
					Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
						F.Service.Get_Snapshot().iObservedPatternSequence == ActiveOccurrence.iPatternSequence &&
						F.Service.Get_Snapshot().iRequestSequence == F.Current.iRequestSequence &&
						F.Service.Get_Snapshot().iRoomAuditionEpoch == EPOCH,
						"data-driven occurrence lost its request/epoch or failed wrap-safe advance");

					auto Stale = First;
					auto Ambiguous = ActiveOccurrence;
					Ambiguous.iPatternSequence += 0x80000000u;
					auto WrongEpoch = ActiveOccurrence;
					++WrongEpoch.iPatternSequence;
					++WrongEpoch.iRoomAuditionEpoch;
					auto WrongRequest = ActiveOccurrence;
					++WrongRequest.iPatternSequence;
					++WrongRequest.iRequestSequence;
					auto WrongPattern = ActiveOccurrence;
					++WrongPattern.iPatternSequence;
					WrongPattern.strPatternId = B;
					auto WrongRevision = ActiveOccurrence;
					++WrongRevision.iPatternSequence;
					WrongRevision.PinnedDefinitionRevision.Bytes.fill(0x43u);
					auto ForwardTerminal = ActiveOccurrence;
					++ForwardTerminal.iPatternSequence;
					ForwardTerminal.eState = VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED;
					for (const auto& Invalid : { Stale, Ambiguous, WrongEpoch, WrongRequest,
						WrongPattern, WrongRevision, ForwardTerminal })
					{
						F.Input().Lifecycles.push_back(Invalid);
					}
					F.Service.Update();
					Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
						F.Service.Get_Snapshot().iObservedPatternSequence == ActiveOccurrence.iPatternSequence,
						"stale, ambiguous, foreign, or forward-terminal lifecycle changed the occurrence");

					auto PendingOccurrence = ActiveOccurrence;
					++PendingOccurrence.iPatternSequence;
					PendingOccurrence.eState = VALTAN_AUDITION_LIFECYCLE_STATE::PENDING;
					F.Input().Lifecycles.push_back(PendingOccurrence);
					F.Service.Update();
					Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::QUEUED &&
						F.Service.Get_Snapshot().iObservedPatternSequence == PendingOccurrence.iPatternSequence,
						"new PENDING occurrence did not become the authoritative cursor");
					const auto Next = F.Queue();
					Require(Next.iPredecessorPatternSequence == PendingOccurrence.iPatternSequence &&
						Next.iPredecessorRoomAuditionEpoch == EPOCH,
						"Next used an occurrence before the last emitted PENDING cursor");
					(void)Wire(Next);

					auto Terminal = PendingOccurrence;
					Terminal.eState = TerminalState;
					if (VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED == TerminalState)
						Terminal.strReason = "harness abort after repeated occurrence";
					F.Input().Lifecycles.push_back(Terminal);
					F.Service.Update();
					const VALTAN_PATTERN_AUDITION_STATE ExpectedState =
						VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED == TerminalState ?
						VALTAN_PATTERN_AUDITION_STATE::ABORTED : VALTAN_PATTERN_AUDITION_STATE::COMPLETED;
					Require(F.Service.Get_Snapshot().eState == ExpectedState &&
						F.Service.Get_Snapshot().iObservedPatternSequence == PendingOccurrence.iPatternSequence,
						"terminal lifecycle did not reuse the last emitted occurrence cursor");
				}
			}
		}
	}

	void VerifyInitialPlayTimeoutsStayBounded()
	{
		Fixture F;
		Require(F.Service.Submit(
			"Valtan Boss Tool", BOSS, A, ActiveRevision(), F.Status),
			"Play failed");
		F.Advance(5001u);
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ABORTED,
			"initial verdict no longer times out");
		Require(F.Service.Submit(
			"Valtan Boss Tool", BOSS, A, ActiveRevision(), F.Status),
			"second Play failed");
		F.Input().Results.push_back(Verdict(F.Input().SentRequests.back()));
		F.Service.Update();
		F.Advance(15001u);
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ABORTED,
			"initial queued start no longer times out");
	}

	void VerifyInitialPlayCarriesExactActiveRevision()
	{
		Fixture F;
		const GameplayDataRevision Revision = ActiveRevision();
		const GameplayDataRevision Missing{};
		Require(!F.Service.Submit(
				"Valtan Boss Tool", BOSS, A, Missing, F.Status) &&
			F.Input().SentRequests.empty(),
			"Play accepted a missing expected active definition revision");
		Require(F.Service.Submit(
				"Valtan Boss Tool", BOSS, A, Revision, F.Status),
			"Play rejected a valid expected active definition revision");
		const C2S_VALTAN_AUDITION_REQUEST Request =
			F.Input().SentRequests.back();
		Require(Request.ExpectedDefinitionRevision == Revision &&
			F.Service.Get_Snapshot().PinnedDefinitionRevision == Revision,
			"Play did not retain the exact expected active revision");

		auto WrongVerdict = Verdict(Request);
		WrongVerdict.ExpectedDefinitionRevision.Bytes[0] ^= 0xffu;
		F.Input().Results.push_back(WrongVerdict);
		auto WrongLifecycle = Event(
			Request, VALTAN_AUDITION_LIFECYCLE_STATE::PENDING);
		WrongLifecycle.PinnedDefinitionRevision.Bytes[0] ^= 0xffu;
		F.Input().Lifecycles.push_back(WrongLifecycle);
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::REQUEST_PENDING,
			"Play accepted a verdict or lifecycle for another definition revision");

		F.Input().Results.push_back(Verdict(Request));
		F.Input().Lifecycles.push_back(Event(
			Request, VALTAN_AUDITION_LIFECYCLE_STATE::PENDING));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::QUEUED &&
			F.Service.Get_Snapshot().PinnedDefinitionRevision == Revision,
			"Play did not admit the matching exact revision lifecycle");
	}

	void VerifyActiveRestartReplacesExactOccurrence()
	{
		Fixture F;
		F.Start();
		const VALTAN_PATTERN_AUDITION_SNAPSHOT Before =
			F.Service.Get_Snapshot();
		Require(F.Service.Restart_ActivePattern(
			"Valtan Boss Tool", BOSS, A, ReplacementRevision(), F.Status),
			"exact active restart was rejected");
		Require(F.Status.find("one-Pattern occurrence") != std::string::npos &&
			F.Status.find("first Stage") != std::string::npos &&
			F.Status.find("current arena") != std::string::npos,
			"Pattern Restart status did not distinguish exact boss-only replay from saved Flow restart");
		const auto Restart = F.Input().SentRequests.back();
		Require(Restart.eOperation ==
				VALTAN_AUDITION_OPERATION::RESTART_PATTERN_ID &&
			Restart.iRequestSequence == Before.iRequestSequence + 1u &&
			Restart.strBossPlacementId == BOSS && Restart.strPatternId == A &&
			Restart.iPredecessorRoomAuditionEpoch == Before.iRoomAuditionEpoch &&
			Restart.iPredecessorPatternSequence == Before.iObservedPatternSequence &&
			Restart.ExpectedDefinitionRevision == Before.PinnedDefinitionRevision &&
			Restart.ReplacementDefinitionRevision == ReplacementRevision() &&
			F.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::REQUEST_PENDING &&
			F.Service.Get_Snapshot().PinnedDefinitionRevision ==
				ReplacementRevision(),
			"restart did not allocate a fresh stable-ID request identity");
		const std::vector<uint8_t> RestartWire = Wire(Restart);
		CPacketReader RestartReader{ RestartWire };
		C2S_VALTAN_AUDITION_REQUEST DecodedRestart{};
		Require(Read_Message(RestartReader, DecodedRestart) &&
			0u == RestartReader.Get_RemainingSize() &&
			DecodedRestart.iRequestSequence == Restart.iRequestSequence &&
			DecodedRestart.eOperation ==
				VALTAN_AUDITION_OPERATION::RESTART_PATTERN_ID &&
			DecodedRestart.iPredecessorRoomAuditionEpoch ==
				Before.iRoomAuditionEpoch &&
			DecodedRestart.iPredecessorPatternSequence ==
				Before.iObservedPatternSequence &&
			DecodedRestart.ExpectedDefinitionRevision ==
				Before.PinnedDefinitionRevision &&
			DecodedRestart.ReplacementDefinitionRevision ==
				ReplacementRevision(),
			"restart wire dropped a predecessor or replacement CAS field");
		auto InvalidRestart = Restart;
		InvalidRestart.iPredecessorRoomAuditionEpoch = 0u;
		CPacketWriter InvalidEpochWriter;
		Require(!Write_Message(InvalidEpochWriter, InvalidRestart),
			"restart wire accepted a zero predecessor epoch");
		InvalidRestart = Restart;
		InvalidRestart.iPredecessorPatternSequence = 0u;
		CPacketWriter InvalidSequenceWriter;
		Require(!Write_Message(InvalidSequenceWriter, InvalidRestart),
			"restart wire accepted a zero predecessor sequence");
		InvalidRestart = Restart;
		InvalidRestart.ExpectedDefinitionRevision = {};
		CPacketWriter InvalidRevisionWriter;
		Require(!Write_Message(InvalidRevisionWriter, InvalidRestart),
			"restart wire accepted a missing predecessor revision");
		InvalidRestart = Restart;
		InvalidRestart.ReplacementDefinitionRevision = {};
		CPacketWriter InvalidReplacementWriter;
		Require(!Write_Message(InvalidReplacementWriter, InvalidRestart),
			"restart wire accepted a missing replacement revision");
		InvalidRestart = Restart;
		InvalidRestart.iExpectedNextRequestSequence = 7u;
		CPacketWriter HiddenNextWriter;
		Require(!Write_Message(HiddenNextWriter, InvalidRestart),
			"restart wire accepted a hidden Next reservation token");

		/* Server boss-only replacement aborts the old request before emitting the
		   new occurrence. That old edge must not terminate the replacement. */
		F.Input().Lifecycles.push_back(Event(
			F.Current, VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED));
		auto WrongReplacementVerdict = Verdict(Restart);
		WrongReplacementVerdict.ReplacementDefinitionRevision = ActiveRevision();
		F.Input().Results.push_back(WrongReplacementVerdict);
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::REQUEST_PENDING,
			"restart admitted a verdict for another replacement revision");
		F.Input().Results.push_back(Verdict(Restart));
		auto Pending = Event(
			Restart, VALTAN_AUDITION_LIFECYCLE_STATE::PENDING);
		Pending.iRoomAuditionEpoch = EPOCH + 1u;
		Pending.iPatternSequence = SEQUENCE + 1u;
		F.Input().Lifecycles.push_back(Pending);
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::QUEUED &&
			F.Service.Get_Snapshot().iRequestSequence == Restart.iRequestSequence &&
			F.Service.Get_Snapshot().iRoomAuditionEpoch == EPOCH + 1u &&
			F.Service.Get_Snapshot().iObservedPatternSequence == SEQUENCE + 1u &&
			F.Service.Get_Snapshot().PinnedDefinitionRevision ==
				ReplacementRevision(),
			"restart did not ignore the old abort and adopt the new pending occurrence");

		auto Active = Pending;
		Active.eState = VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE;
		F.Input().Lifecycles.push_back(Active);
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
			F.Service.Get_Snapshot().iRequestSequence == Restart.iRequestSequence &&
			F.Service.Get_Snapshot().iObservedPatternSequence == SEQUENCE + 1u,
			"replacement occurrence did not become authoritative ACTIVE");
	}

	void VerifyActiveRestartRejectsConflictsAndRollsBack()
	{
		Fixture F;
		F.Start();
		const size_t SentBefore = F.Input().SentRequests.size();
		Require(!F.Service.Restart_ActivePattern(
			"Valtan Boss Tool", BOSS, A, GameplayDataRevision{}, F.Status) &&
			!F.Service.Restart_ActivePattern(
			"Effect Tool", BOSS, A, F.Status) &&
			!F.Service.Restart_ActivePattern(
				"Valtan Boss Tool", "boss.valtan.other", A, F.Status) &&
			!F.Service.Restart_ActivePattern(
				"Valtan Boss Tool", BOSS, B, F.Status) &&
			SentBefore == F.Input().SentRequests.size(),
			"restart accepted another consumer or a non-exact active identity");

		F.Input().bFlowInFlight = true;
		Require(!F.Service.Restart_ActivePattern(
			"Valtan Boss Tool", BOSS, A, F.Status),
			"restart replaced an ordered Flow");
		F.Input().bFlowInFlight = false;
		F.Input().bFlowStartPending = true;
		Require(!F.Service.Restart_ActivePattern(
			"Valtan Boss Tool", BOSS, A, F.Status),
			"restart replaced a pending Flow start");
		F.Input().bFlowStartPending = false;
		(void)F.Reserve();
		Require(!F.Service.Restart_ActivePattern(
			"Valtan Boss Tool", BOSS, A, F.Status),
			"restart discarded an approved Next reservation");

		Fixture SendFailure;
		SendFailure.Start();
		const VALTAN_PATTERN_AUDITION_SNAPSHOT BeforeSendFailure =
			SendFailure.Service.Get_Snapshot();
		SendFailure.Input().bSendSucceeds = false;
		Require(!SendFailure.Service.Restart_ActivePattern(
				"Valtan Boss Tool", BOSS, A, SendFailure.Status) &&
			SendFailure.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
			SendFailure.Service.Get_Snapshot().iRequestSequence ==
				BeforeSendFailure.iRequestSequence,
			"transport failure replaced the active occurrence locally");

		Fixture Rejected;
		Rejected.Start();
		const VALTAN_PATTERN_AUDITION_SNAPSHOT BeforeRejection =
			Rejected.Service.Get_Snapshot();
		Require(Rejected.Service.Restart_ActivePattern(
			"Valtan Boss Tool", BOSS, A, Rejected.Status),
			"restart request setup failed");
		const auto Restart = Rejected.Input().SentRequests.back();
		Require(!Rejected.Service.Restart_ActivePattern(
			"Valtan Boss Tool", BOSS, A, Rejected.Status),
			"a second restart replaced the pending verdict");
		Rejected.Input().Results.push_back(Verdict(
			Restart, VALTAN_AUDITION_RESULT::REJECTED_OCCURRENCE_PRESERVED));
		Rejected.Service.Update();
		const auto& Restored = Rejected.Service.Get_Snapshot();
		Require(Restored.eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
			Restored.iRequestSequence == BeforeRejection.iRequestSequence &&
			Restored.iRoomAuditionEpoch == BeforeRejection.iRoomAuditionEpoch &&
			Restored.iObservedPatternSequence ==
				BeforeRejection.iObservedPatternSequence &&
			Restored.strStatus.find("previous Server occurrence remains authoritative") !=
				std::string::npos,
			"typed preserved Restart rejection did not restore the prior occurrence");

		Fixture Stale;
		Stale.Start();
		Require(Stale.Service.Restart_ActivePattern(
			"Valtan Boss Tool", BOSS, A, Stale.Status),
			"stale restart request setup failed");
		const auto StaleRequest = Stale.Input().SentRequests.back();
		Stale.Input().Results.push_back(Verdict(
			StaleRequest, VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION));
		Stale.Service.Update();
		Require(Stale.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::REJECTED &&
			!Stale.Service.Has_PlaybackOwnership() &&
			!Stale.Service.Restart_ActivePattern(
				"Valtan Boss Tool", BOSS, A, Stale.Status),
			"stale Restart verdict falsely restored a retired predecessor");
	}

	void VerifyCompletedRestartReplacesExactHold()
	{
		Fixture F;
		F.Start();
		F.Complete();
		const VALTAN_PATTERN_AUDITION_SNAPSHOT Before =
			F.Service.Get_Snapshot();
		Require(Before.eState == VALTAN_PATTERN_AUDITION_STATE::COMPLETED &&
			F.Service.Restart_ActivePattern(
				"Valtan Boss Tool", BOSS, A, F.Status),
			"authoritative completed hold could not restart");
		const auto Restart = F.Input().SentRequests.back();
		Require(Restart.eOperation ==
				VALTAN_AUDITION_OPERATION::RESTART_PATTERN_ID &&
			Restart.iPredecessorRoomAuditionEpoch == Before.iRoomAuditionEpoch &&
			Restart.iPredecessorPatternSequence == Before.iObservedPatternSequence &&
			Restart.ExpectedDefinitionRevision == Before.PinnedDefinitionRevision,
			"completed Restart lost its exact predecessor tuple");
		F.Input().Results.push_back(Verdict(Restart));
		auto Pending = Event(Restart, VALTAN_AUDITION_LIFECYCLE_STATE::PENDING);
		Pending.iRoomAuditionEpoch = Before.iRoomAuditionEpoch + 1u;
		Pending.iPatternSequence = Before.iObservedPatternSequence + 1u;
		F.Input().Lifecycles.push_back(Pending);
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::QUEUED &&
			F.Service.Get_Snapshot().iRequestSequence == Restart.iRequestSequence &&
			F.Service.Get_Snapshot().iObservedPatternSequence ==
				Before.iObservedPatternSequence + 1u,
			"completed Restart did not adopt the replacement occurrence");
	}

	void VerifyRestartTimeoutKeepsExactRetryIdentity()
	{
		Fixture F;
		F.Start();
		Require(F.Service.Restart_ActivePattern(
			"Valtan Boss Tool", BOSS, A, F.Status),
			"restart timeout fixture could not send its first request");
		const C2S_VALTAN_AUDITION_REQUEST Restart =
			F.Input().SentRequests.back();
		/* The predecessor may complete while the Restart verdict is lost. Its
		   old request identity must update the fallback instead of being dropped. */
		F.Input().Lifecycles.push_back(Event(
			F.Current, VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED));
		F.Service.Update();
		F.Advance(5100u);
		Require(F.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::RESTART_UNCONFIRMED &&
			F.Service.Has_PlaybackOwnership(),
			"ambiguous restart timeout discarded its in-flight ownership");
		Require(F.Service.Retry_UnconfirmedRestart(F.Status),
			"unconfirmed restart did not permit an exact retry");
		const C2S_VALTAN_AUDITION_REQUEST Retry =
			F.Input().SentRequests.back();
		Require(Retry.iRequestSequence == Restart.iRequestSequence &&
			Retry.eOperation == Restart.eOperation &&
			Retry.strBossPlacementId == Restart.strBossPlacementId &&
			Retry.strPatternId == Restart.strPatternId &&
			Retry.iPredecessorRoomAuditionEpoch ==
				Restart.iPredecessorRoomAuditionEpoch &&
			Retry.iPredecessorPatternSequence ==
				Restart.iPredecessorPatternSequence &&
			Retry.ExpectedDefinitionRevision ==
				Restart.ExpectedDefinitionRevision &&
			Retry.ReplacementDefinitionRevision ==
				Restart.ReplacementDefinitionRevision,
			"restart retry allocated or altered a wire identity");
		F.Input().Results.push_back(Verdict(
			Retry, VALTAN_AUDITION_RESULT::REJECTED_OCCURRENCE_PRESERVED));
		F.Service.Update();
		const auto& Reconciled = F.Service.Get_Snapshot();
		Require(Reconciled.eState == VALTAN_PATTERN_AUDITION_STATE::COMPLETED &&
			!F.Service.Has_PlaybackOwnership() &&
			Reconciled.iRequestSequence == F.Current.iRequestSequence &&
			Reconciled.iObservedPatternSequence == SEQUENCE,
			"preserved exact-retry verdict revived ACTIVE after predecessor completion");

		Fixture Queued;
		Queued.Start();
		Require(Queued.Service.Restart_ActivePattern(
			"Valtan Boss Tool", BOSS, A, Queued.Status),
			"queued timeout fixture could not send restart");
		const auto QueuedRequest = Queued.Input().SentRequests.back();
		Queued.Input().Results.push_back(Verdict(QueuedRequest));
		Queued.Service.Update();
		Queued.Advance(15100u);
		Require(Queued.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::RESTART_UNCONFIRMED &&
			Queued.Service.Has_PlaybackOwnership(),
			"queued restart lifecycle timeout became a false terminal abort");
	}

	void VerifyPatternSoundReceiptPinsOccurrenceRestartAndAutoNext()
	{
		Fixture Invalid;
		Require(!Invalid.Service.Submit(
				"Valtan Boss Tool", BOSS, A, ActiveRevision(),
				VALTAN_PATTERN_SOUND_SOURCE_RECEIPT{}, Invalid.Status) &&
			Invalid.Input().SentRequests.empty(),
			"Play accepted a missing S receipt or sent before fail-closed validation");
		Fixture F;
		const auto SoundA = SoundReceipt('a');
		const auto SoundB = SoundReceipt('b');
		Require(F.Service.Submit(
			"Valtan Boss Tool", BOSS, A, ActiveRevision(), SoundA, F.Status),
			"Play rejected a valid exact Pattern Sound receipt");
		F.Current = F.Input().SentRequests.back();
		Require(F.Service.Get_Snapshot().PinnedPatternSoundSourceReceipt ==
				SoundA && F.Service.Has_PatternSoundMutationBarrier(),
			"pending Play did not pin S or expose its mutation barrier");
		F.Input().Results.push_back(Verdict(F.Current));
		F.Input().Lifecycles.push_back(Event(
			F.Current, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().PinnedPatternSoundSourceReceipt ==
				SoundA &&
			F.Service.Verify_PatternSoundSourceReceipt(SoundA, F.Status) &&
			!F.Service.Verify_PatternSoundSourceReceipt(SoundB, F.Status),
			"ACTIVE occurrence did not retain or verify its exact S receipt");
		Require(!F.Service.Restart_ActivePattern(
				"Valtan Boss Tool", BOSS, A, ActiveRevision(), SoundB, F.Status) &&
			F.Service.Restart_ActivePattern(
				"Valtan Boss Tool", BOSS, A, ActiveRevision(), SoundA, F.Status),
			"Restart did not reject a foreign S or retain the predecessor S");
		F.Advance(5100u);
		Require(!F.Service.Retry_UnconfirmedRestart(SoundB, F.Status) &&
			F.Service.Retry_UnconfirmedRestart(SoundA, F.Status),
			"unconfirmed Restart did not bind exact retry to S");

		Fixture Next;
		Next.Start();
		Require(!Next.Service.Queue_NextPattern(
				"Valtan Boss Tool", BOSS, B, ActiveRevision(), SoundB, Next.Status) &&
			Next.Service.Queue_NextPattern(
				"Valtan Boss Tool", BOSS, B, ActiveRevision(), SoundA, Next.Status),
			"Next did not enforce its predecessor S receipt");
		const auto NextRequest = Next.Input().SentRequests.back();
		Require(Next.Service.Get_NextCommand().PinnedPatternSoundSourceReceipt ==
			SoundA, "pending Next command did not pin S");
		Next.Input().Results.push_back(Verdict(NextRequest));
		Next.Service.Update();
		Require(Next.Service.Get_NextSnapshot().
				PinnedPatternSoundSourceReceipt == SoundA,
			"approved Next reservation lost S");
		Next.Input().Lifecycles.push_back(Event(
			NextRequest, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		Next.Service.Update();
		Require(Next.Service.Get_Snapshot().strPatternId == B &&
			Next.Service.Get_Snapshot().PinnedPatternSoundSourceReceipt == SoundA &&
			Next.Service.Verify_PatternSoundSourceReceipt(SoundA, Next.Status),
			"auto-promoted Next occurrence lost its pinned S receipt");
	}
}

int Run_CinematicViewRebaseContractTests();

int main(const int argc, const char* const argv[])
{
	if (argc == 2 && std::string(argv[1]) == "--action-composition-graph-contract")
		return Run_ActionCompositionGraphModelContractTests();
	if (argc == 2 && std::string(argv[1]) == "--cinematic-view-rebase-contract")
		return Run_CinematicViewRebaseContractTests();
	if (argc == 2 && std::string(argv[1]) == "--valtan-presentation-contract")
		return Run_ValtanPresentationContractTests();
	if (argc == 2 && std::string(argv[1]) == "--kouku-pattern-delete-contract")
		return Run_KoukuPatternDeleteContractTests();
	if (argc == 2 && std::string(argv[1]) == "--kouku-collider-duplicate-contract")
		return Run_KoukuColliderDuplicateContractTests();
	if (argc == 2 && std::string(argv[1]) == "--kouku-fixed-damage-contract")
        return Run_KoukuFixedDamageContractTests();
	if (argc == 2 && std::string(argv[1]) == "--kouku-collider-group-contract")
		return Run_KoukuColliderGroupContractTests();
	if (argc == 2 && std::string(argv[1]) == "--presentation-generation-admission-contract")
		return Run_ValtanPresentationGenerationAdmissionContractTests();
	if (argc == 2 && std::string(argv[1]) == "--kouku-sound-timeline-contract")
		return Run_KoukuSoundTimelineContractTests();
	if (argc == 2 && std::string(argv[1]) == "--kouku-preview-transport-contract")
		return Run_KoukuPreviewTransportContractTests();
	if (argc == 2 && std::string(argv[1]) == "--kouku-sequence-document-contract")
		return Run_KoukuSequenceDocumentContractTests();
	if (argc == 2 && std::string(argv[1]) == "--kouku-parent-timing-contract")
		return Run_KoukuSequentialParentTimingContractTests();
	if (argc == 2 && std::string(argv[1]) == "--kouku-independent-row-clock-contract")
		return Run_KoukuIndependentRowClockContractTests();
	if (argc == 2 && std::string(argv[1]) == "--kouku-world-effect-frame-contract")
		return Run_KoukuWorldEffectFrameContractTests();
	if (argc == 2 && std::string(argv[1]) == "--kouku-composition-editor-contract")
		return Run_KoukuCompositionEditorContractTests();
	if (argc != 1)
	{
		std::cerr << "Usage: BossToolTests "
			"[--action-composition-graph-contract | --cinematic-view-rebase-contract | --valtan-presentation-contract | --kouku-pattern-delete-contract | --presentation-generation-admission-contract | --kouku-composition-editor-contract | --kouku-world-effect-frame-contract | --kouku-preview-transport-contract | --kouku-sequence-document-contract | --kouku-collider-group-contract | --kouku-collider-duplicate-contract | --kouku-fixed-damage-contract | --kouku-parent-timing-contract | --kouku-independent-row-clock-contract | --kouku-sound-timeline-contract]\n";
		return 2;
	}
	const std::vector<std::pair<const char*, std::function<void()>>> Tests{
		{ "authoritative predecessor and no reset", VerifyAuthoritativePredecessorAndNoReset },
		{ "A completed before B verdict and PENDING", VerifyCompletionBeforeVerdictAndPending },
		{ "B verdict before A completed and player wait", VerifyVerdictBeforeCompletionAndDeadPlayerWait },
		{ "B lifecycle before verdict", VerifyLifecycleBeforeVerdict },
		{ "outcome followup rebases Next occurrence", VerifyOutcomeFollowupRebasesNextOccurrence },
		{ "rejected C preserves B", VerifyRejectedReplacementPreservesReservation },
		{ "accepted C ignores late B abort", VerifyAcceptedReplacementIgnoresOldAbort },
		{ "clear timeout same-payload retry and replay", VerifyClearTimeoutRetryAndReplayedVerdict },
		{ "rejected clear preserves B", VerifyRejectedClearPreservesReservation },
		{ "full echoed identity and legacy duplicate", VerifyFullEchoAndUnexpectedDuplicate },
		{ "generation/disconnect invalidates before drain", VerifyGenerationAndDisconnectBeforeDrain },
		{ "A to A occurrence identity and HUD isolation", VerifySamePatternOccurrencesRemainDistinct },
		{ "Flow conflict send rollback request exhaustion", VerifyFlowConflictSendRollbackAndExhaustion },
		{ "Next abort and presentation isolation", VerifyNextAbortAndPresentationIsolation },
		{ "initial Play 5s/15s timeouts", VerifyInitialPlayTimeoutsStayBounded },
		{ "initial Play exact active definition revision", VerifyInitialPlayCarriesExactActiveRevision },
		{ "exact active restart allocates a new occurrence", VerifyActiveRestartReplacesExactOccurrence },
		{ "active restart rejects conflicts and rolls back", VerifyActiveRestartRejectsConflictsAndRollsBack },
		{ "completed hold restart allocates a new occurrence", VerifyCompletedRestartReplacesExactHold },
		{ "restart timeout preserves exact retry identity", VerifyRestartTimeoutKeepsExactRetryIdentity },
		{ "Pattern Sound S pins Play Restart and auto-Next", VerifyPatternSoundReceiptPinsOccurrenceRestartAndAutoNext },
		{ "all data-driven occurrences keep request identity and refresh Next predecessor", VerifyDataDrivenOccurrencesRetainRequestAndAdvanceNextPredecessor },
		{ "live Product Next without isolated Play", VerifyLiveProductWithoutIsolatedPlay },
		{ "live epoch identity and exact retry", VerifyLiveEpochIdentityAndExactRetry },
		{ "live lifecycle before verdict", VerifyLiveLifecycleBeforeVerdict },
		{ "live reservation replacement and clear", VerifyLiveReservationReplacementAndClear },
		{ "live idle consumed player wait", VerifyLiveIdleConsumedPlayerWait },
		{ "live Flow and readiness revalidation", VerifyLiveFlowAndReadinessRevalidation },
		{ "new world live Next after old audition", VerifyNewWorldLiveNextAfterOldAudition },
		{ "Flow reset retires completed isolated predecessor", VerifyFlowResetInvalidatesCompletedAudition },
	};
	size_t Failed = 0u;
	for (const auto& [Name, Test] : Tests)
	{
		try { Test(); }
		catch (const std::exception& Error)
		{
			++Failed;
			std::cerr << "FAIL " << Name << ": " << Error.what() << '\n';
		}
	}
	std::cout << "BossToolTests: " << Tests.size() - Failed
		<< "/" << Tests.size() << " passed\n";
	const int FlowFailures = Run_ValtanPatternFlowServiceTests();
	const int TuningFailures = Run_ValtanTuningCommandServiceTests();
	const int PresentationFailures = Run_ValtanPresentationContractTests();
	const int EncounterReferenceFailures =
		Run_ValtanEncounterReferenceContractTests();
	const int CanonicalGraphFailures =
		Run_ValtanCanonicalGraphContractTests();
	const int BossCompositionDocumentFailures =
		Run_BossCompositionDocumentContractTests();
	const int CompositionGraphFailures =
		Run_ActionCompositionGraphModelContractTests();
	const int BossLogicFlowFailures =
		Run_BossLogicFlowViewModelContractTests();
	const int PatternSoundCueFailures =
		Run_ValtanPatternSoundCueDocumentContractTests();
	const int AnimationBindingDocumentFailures =
		Run_ValtanPatternAnimationBindingDocumentContractTests();
	const int EffectCueAuthoringFailures =
		Run_ValtanPatternEffectCueAuthoringContractTests();
	const int PresentationGenerationAdmissionFailures =
		Run_ValtanPresentationGenerationAdmissionContractTests();
	const int CombatDebugVisibilityFailures =
		Run_CombatDebugVisibilityContractTests();
	const int PlayerHandGripTransformFailures =
		Run_PlayerHandGripTransformContractTests();
	return 0u == Failed && 0 == FlowFailures && 0 == TuningFailures &&
		0 == PresentationFailures && 0 == EncounterReferenceFailures &&
		0 == CanonicalGraphFailures &&
		0 == BossCompositionDocumentFailures &&
		0 == CompositionGraphFailures &&
		0 == BossLogicFlowFailures &&
		0 == PatternSoundCueFailures &&
		0 == AnimationBindingDocumentFailures &&
		0 == EffectCueAuthoringFailures &&
		0 == PresentationGenerationAdmissionFailures &&
		0 == CombatDebugVisibilityFailures &&
		0 == PlayerHandGripTransformFailures ? 0 : 1;
}

```

## C:/Users/tnest/Desktop/LostArk/Tools/BossToolTests/Default/BossToolTests.vcxproj

이름 변경 후 파일 전체:

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|x64"><Configuration>Debug</Configuration><Platform>x64</Platform></ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64"><Configuration>Release</Configuration><Platform>x64</Platform></ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <VCProjectVersion>17.0</VCProjectVersion>
    <Keyword>Win32Proj</Keyword>
    <ProjectGuid>{4970BA90-455F-4ADB-B855-93D1CC2C1CA2}</ProjectGuid>
    <RootNamespace>BossToolTests</RootNamespace>
    <ProjectName>BossToolTests</ProjectName>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
    <UseDebugLibraries Condition="'$(Configuration)'=='Debug'">true</UseDebugLibraries>
    <UseDebugLibraries Condition="'$(Configuration)'=='Release'">false</UseDebugLibraries>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <PropertyGroup>
    <OutDir>$(ProjectDir)..\Bin\$(Configuration)\</OutDir>
    <IntDir>$(ProjectDir)..\Intermediate\$(Platform)\$(Configuration)\</IntDir>
    <TargetName>BossToolTests</TargetName>
    <LocalDebuggerWorkingDirectory>$(ProjectDir)..\..\..\</LocalDebuggerWorkingDirectory>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
  </PropertyGroup>
  <ItemDefinitionGroup>
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <ConformanceMode>true</ConformanceMode>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <MultiProcessorCompilation>true</MultiProcessorCompilation>
      <AdditionalOptions>/MP /bigobj /utf-8 %(AdditionalOptions)</AdditionalOptions>
      <PreprocessorDefinitions>LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS;_CONSOLE;NOMINMAX;WIN32_LEAN_AND_MEAN;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <AdditionalIncludeDirectories>$(ProjectDir)..\..\..\Client\Public;$(ProjectDir)..\..\..\Shared\Public;$(ProjectDir)..\..\..\Engine\Public;$(ProjectDir)..\..\..\Engine\External\imgui;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
	  <!-- No-PCH Client sources expose legacy global `using namespace std`.
	       Complete WinSDK COM/RPC declarations first so its `byte` typedef can
	       never become ambiguous with C++17 std::byte in a later include. -->
	  <ForcedIncludeFiles>Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <!-- Editor contracts call draft methods only. Delay loading keeps the
           unused ImGui virtual methods from loading Engine or opening a UI. -->
      <AdditionalLibraryDirectories>$(ProjectDir)..\..\..\EngineSDK\lib\$(Configuration);%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
      <AdditionalDependencies>Engine.lib;delayimp.lib;%(AdditionalDependencies)</AdditionalDependencies>
      <DelayLoadDLLs>Engine.dll;%(DelayLoadDLLs)</DelayLoadDLLs>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)'=='Debug'">
    <ClCompile><PreprocessorDefinitions>_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions></ClCompile>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)'=='Release'">
    <ClCompile>
      <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <Optimization>MaxSpeed</Optimization>
    </ClCompile>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\Main.cpp" />
	<ClCompile Include="..\Private\ValtanCanonicalGraphContractTests.cpp" />
	<ClCompile Include="..\Private\BossCompositionDocumentContractTests.cpp" />
	<ClCompile Include="..\Private\ActionCompositionGraphModelContractTests.cpp" />
	<ClCompile Include="..\Private\BossLogicFlowViewModelContractTests.cpp" />
	<ClCompile Include="..\Private\ValtanPresentationGenerationAdmissionContractTests.cpp" />
	<ClCompile Include="..\Private\CombatDebugVisibilityContractTests.cpp" />
	<ClCompile Include="..\Private\PlayerHandGripTransformContractTests.cpp" />
	<ClCompile Include="..\Private\ValtanEncounterReferenceContractTests.cpp" />
    <ClCompile Include="..\Private\ValtanPatternFlowServiceTests.cpp" />
	<ClCompile Include="..\Private\ValtanPatternAnimationBindingDocumentContractTests.cpp" />
	<ClCompile Include="..\Private\ValtanPatternEffectCueAuthoringContractTests.cpp">
	  <ForcedIncludeFiles>Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
	</ClCompile>
	<ClCompile Include="..\Private\ValtanPatternSoundCueDocumentContractTests.cpp" />
    <ClCompile Include="..\Private\ValtanPresentationContractTests.cpp" />
    <ClCompile Include="..\Private\ValtanTuningCommandServiceTests.cpp" />
    <ClCompile Include="..\..\..\Client\Private\ValtanPatternAuditionService.cpp" />
	<ClCompile Include="..\..\..\Client\Private\ValtanPatternTree.cpp" />
	<ClCompile Include="..\..\..\Client\Private\ActionCompositionGraphModel.cpp" />
	<ClCompile Include="..\..\..\Client\Private\BossLogicFlowViewModel.cpp" />
	<ClCompile Include="..\..\..\Client\Private\BossCompositionDocument.cpp" />
	<ClCompile Include="..\..\..\Client\Private\KoukuSaydonActionWorkbench.cpp" />
	<ClCompile Include="..\..\..\Client\Private\KoukuSaydonCompositionDocument.cpp" />
	<ClCompile Include="..\..\..\Client\Private\KoukuSaydonAnimationActionDocument.cpp" />
	<ClCompile Include="..\..\..\Client\Private\CompositionResourceTree.cpp" />
	<ClCompile Include="..\..\..\Client\Private\ValtanPresentationGenerationAdmission.cpp" />
	<ClCompile Include="..\..\..\Client\Private\EffectV2_Document.cpp" />
	<ClCompile Include="..\..\..\Client\Private\EffectV2_Catalog.cpp" />
	<ClCompile Include="..\..\..\Client\Private\ValtanPatternFlowDocument.cpp">
	  <ForcedIncludeFiles>Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
	</ClCompile>
	<ClCompile Include="..\..\..\Client\Private\ValtanPatternEffectCueDocument.cpp" />
	<ClCompile Include="..\..\..\Client\Private\ValtanPatternEffectCueAuthoring.cpp" />
	<ClCompile Include="..\..\..\Client\Private\Effect_DirectAuthoredSourceIndex.cpp">
	  <ForcedIncludeFiles>Windows.h;objbase.h;bcrypt.h;%(ForcedIncludeFiles)</ForcedIncludeFiles>
	</ClCompile>
    <ClCompile Include="..\..\..\Client\Private\ValtanPatternFlowService.cpp" />
    <ClCompile Include="..\..\..\Client\Private\ValtanTuningCommandService.cpp" />
	<ClCompile Include="..\..\..\Client\Private\AnimationSkillBindingDocument.cpp" />
    <ClCompile Include="..\..\..\Client\Private\DataJson.cpp" />
	<ClCompile Include="..\..\..\Client\Private\ProjectDataRoot.cpp" />
	<ClCompile Include="..\..\..\Client\Private\RuntimeAssetRoot.cpp" />
	<ClCompile Include="..\..\..\Client\Private\SoundCueCatalog.cpp" />
	<ClCompile Include="..\..\..\Client\Private\ValtanCombatObjectSoundCueDocument.cpp" />
	<ClCompile Include="..\..\..\Client\Private\ValtanPatternSoundCueDocument.cpp" />
    <ClCompile Include="..\..\..\Client\Private\ActionPresentationTimeline.cpp" />
    <ClCompile Include="..\..\..\Client\Private\CameraShakeService.cpp" />
    <ClCompile Include="..\..\..\Client\Private\EncounterPatternReference.cpp" />
    <ClCompile Include="..\..\..\Client\Private\ValtanCinematicCameraController.cpp" />
    <ClCompile Include="..\..\..\Client\Private\ValtanCinematicCameraDocument.cpp" />
    <ClInclude Include="..\..\..\Client\Public\ValtanPatternAuditionService.h" />
	<ClInclude Include="..\..\..\Client\Public\ValtanPatternTree.h" />
	<ClInclude Include="..\..\..\Client\Public\PlayerHandGripTransform.h" />
	<ClInclude Include="..\..\..\Client\Public\CombatDebugVisibility.h" />
	<ClInclude Include="..\..\..\Client\Public\ActionCompositionGraphModel.h" />
	<ClInclude Include="..\..\..\Client\Public\BossLogicFlowView.h" />
	<ClInclude Include="..\..\..\Client\Public\BossCompositionDocument.h" />
	<ClInclude Include="..\..\..\Client\Public\ValtanPresentationGenerationAdmission.h" />
	<ClInclude Include="..\..\..\Client\Public\EffectV2_Document.h" />
	<ClInclude Include="..\..\..\Client\Public\EffectV2_Catalog.h" />
	<ClInclude Include="..\..\..\Client\Public\ValtanPatternEffectCueDocument.h" />
	<ClInclude Include="..\..\..\Client\Public\ValtanPatternEffectCueAuthoring.h" />
	<ClInclude Include="..\..\..\Client\Public\Effect_DirectAuthoredSourceIndex.h" />
    <ClInclude Include="..\..\..\Client\Public\ValtanPatternFlowService.h" />
    <ClInclude Include="..\..\..\Client\Public\ValtanTuningCommandService.h" />
    <ClInclude Include="..\..\..\Client\Public\DataJson.h" />
	<ClInclude Include="..\..\..\Client\Public\RuntimeAssetRoot.h" />
	<ClInclude Include="..\..\..\Client\Public\SoundCueCatalog.h" />
	<ClInclude Include="..\..\..\Client\Public\ValtanCombatObjectSoundCueDocument.h" />
    <ClInclude Include="..\..\..\Client\Public\ActionPresentationTimeline.h" />
    <ClInclude Include="..\..\..\Client\Public\CameraShakeService.h" />
    <ClInclude Include="..\..\..\Client\Public\EncounterPatternReference.h" />
    <ClInclude Include="..\..\..\Client\Public\MonsterPresentationContract.h" />
    <ClInclude Include="..\..\..\Client\Public\ValtanCinematicCameraController.h" />
    <ClInclude Include="..\..\..\Client\Public\ValtanCinematicCameraDocument.h" />
    <ClInclude Include="..\..\..\Client\Public\ValtanPatternFlowDocument.h" />
  </ItemGroup>
  <ItemGroup>
    <ProjectReference Include="..\..\..\Shared\Default\Shared.vcxproj">
      <Project>{F4CCF815-6D51-412F-A76E-84D2F1D05571}</Project>
    </ProjectReference>
  </ItemGroup>
  <ItemGroup>
    <None Include="..\README.md" />
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>

```

## C:/Users/tnest/Desktop/LostArk/Tools/BossToolTests/Default/BossToolTests.vcxproj.filters

이름 변경 후 파일 전체:

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <Filter Include="00.Start">
      <UniqueIdentifier>{86467246-DC48-501B-B673-F12B0B319D1F}</UniqueIdentifier>
    </Filter>
    <Filter Include="01.Tests.Valtan">
      <UniqueIdentifier>{456AD3BB-3E23-559A-9122-8FA219A1C9B9}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.Tests.SharedAndKouku">
      <UniqueIdentifier>{81E3E450-1651-5C85-8A05-C7714919B8EF}</UniqueIdentifier>
    </Filter>
    <Filter Include="03.Client.Services">
      <UniqueIdentifier>{AEE2C837-B6E2-5CCD-B832-124702299C4B}</UniqueIdentifier>
    </Filter>
    <Filter Include="04.Client.Documents">
      <UniqueIdentifier>{BE5FE3A8-0771-5380-8E79-7D9403DB410D}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Client.Presentation">
      <UniqueIdentifier>{133704AC-0D0E-5437-952C-1FB826A40E08}</UniqueIdentifier>
    </Filter>
    <Filter Include="06.Client.Support">
      <UniqueIdentifier>{055DB138-CF65-5A9F-BAA5-E8F56C9BB014}</UniqueIdentifier>
    </Filter>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\Main.cpp">
      <Filter>00.Start</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanCanonicalGraphContractTests.cpp">
      <Filter>01.Tests.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\BossCompositionDocumentContractTests.cpp">
      <Filter>02.Tests.SharedAndKouku</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ActionCompositionGraphModelContractTests.cpp">
      <Filter>02.Tests.SharedAndKouku</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\BossLogicFlowViewModelContractTests.cpp">
      <Filter>02.Tests.SharedAndKouku</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPresentationGenerationAdmissionContractTests.cpp">
      <Filter>01.Tests.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CombatDebugVisibilityContractTests.cpp">
      <Filter>02.Tests.SharedAndKouku</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\PlayerHandGripTransformContractTests.cpp">
      <Filter>02.Tests.SharedAndKouku</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanEncounterReferenceContractTests.cpp">
      <Filter>01.Tests.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPatternFlowServiceTests.cpp">
      <Filter>01.Tests.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPatternAnimationBindingDocumentContractTests.cpp">
      <Filter>01.Tests.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPatternEffectCueAuthoringContractTests.cpp">
      <Filter>01.Tests.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPatternSoundCueDocumentContractTests.cpp">
      <Filter>01.Tests.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanPresentationContractTests.cpp">
      <Filter>01.Tests.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanTuningCommandServiceTests.cpp">
      <Filter>01.Tests.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\ValtanPatternAuditionService.cpp">
      <Filter>03.Client.Services</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\ValtanPatternTree.cpp">
      <Filter>04.Client.Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\ActionCompositionGraphModel.cpp">
      <Filter>04.Client.Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\BossLogicFlowViewModel.cpp">
      <Filter>04.Client.Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\BossCompositionDocument.cpp">
      <Filter>04.Client.Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\KoukuSaydonActionWorkbench.cpp">
      <Filter>04.Client.Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\KoukuSaydonCompositionDocument.cpp">
      <Filter>04.Client.Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\KoukuSaydonAnimationActionDocument.cpp">
      <Filter>04.Client.Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\CompositionResourceTree.cpp">
      <Filter>04.Client.Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\ValtanPresentationGenerationAdmission.cpp">
      <Filter>05.Client.Presentation</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\EffectV2_Document.cpp">
      <Filter>04.Client.Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\EffectV2_Catalog.cpp">
      <Filter>06.Client.Support</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\ValtanPatternFlowDocument.cpp">
      <Filter>04.Client.Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\ValtanPatternEffectCueDocument.cpp">
      <Filter>04.Client.Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\ValtanPatternEffectCueAuthoring.cpp">
      <Filter>04.Client.Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\Effect_DirectAuthoredSourceIndex.cpp">
      <Filter>06.Client.Support</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\ValtanPatternFlowService.cpp">
      <Filter>03.Client.Services</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\ValtanTuningCommandService.cpp">
      <Filter>03.Client.Services</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\AnimationSkillBindingDocument.cpp">
      <Filter>04.Client.Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\DataJson.cpp">
      <Filter>06.Client.Support</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\ProjectDataRoot.cpp">
      <Filter>06.Client.Support</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\RuntimeAssetRoot.cpp">
      <Filter>06.Client.Support</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\SoundCueCatalog.cpp">
      <Filter>06.Client.Support</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\ValtanCombatObjectSoundCueDocument.cpp">
      <Filter>04.Client.Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\ValtanPatternSoundCueDocument.cpp">
      <Filter>04.Client.Documents</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\ActionPresentationTimeline.cpp">
      <Filter>05.Client.Presentation</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\CameraShakeService.cpp">
      <Filter>05.Client.Presentation</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\EncounterPatternReference.cpp">
      <Filter>05.Client.Presentation</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\ValtanCinematicCameraController.cpp">
      <Filter>05.Client.Presentation</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\ValtanCinematicCameraDocument.cpp">
      <Filter>04.Client.Documents</Filter>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\..\..\Client\Public\ValtanPatternAuditionService.h">
      <Filter>03.Client.Services</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\ValtanPatternTree.h">
      <Filter>04.Client.Documents</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\PlayerHandGripTransform.h">
      <Filter>05.Client.Presentation</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\CombatDebugVisibility.h">
      <Filter>05.Client.Presentation</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\ActionCompositionGraphModel.h">
      <Filter>04.Client.Documents</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\BossLogicFlowView.h">
      <Filter>04.Client.Documents</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\BossCompositionDocument.h">
      <Filter>04.Client.Documents</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\ValtanPresentationGenerationAdmission.h">
      <Filter>05.Client.Presentation</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\EffectV2_Document.h">
      <Filter>04.Client.Documents</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\EffectV2_Catalog.h">
      <Filter>06.Client.Support</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\ValtanPatternEffectCueDocument.h">
      <Filter>04.Client.Documents</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\ValtanPatternEffectCueAuthoring.h">
      <Filter>04.Client.Documents</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\Effect_DirectAuthoredSourceIndex.h">
      <Filter>06.Client.Support</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\ValtanPatternFlowService.h">
      <Filter>03.Client.Services</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\ValtanTuningCommandService.h">
      <Filter>03.Client.Services</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\DataJson.h">
      <Filter>06.Client.Support</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\RuntimeAssetRoot.h">
      <Filter>06.Client.Support</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\SoundCueCatalog.h">
      <Filter>06.Client.Support</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\ValtanCombatObjectSoundCueDocument.h">
      <Filter>04.Client.Documents</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\ActionPresentationTimeline.h">
      <Filter>05.Client.Presentation</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\CameraShakeService.h">
      <Filter>05.Client.Presentation</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\EncounterPatternReference.h">
      <Filter>05.Client.Presentation</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\MonsterPresentationContract.h">
      <Filter>05.Client.Presentation</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\ValtanCinematicCameraController.h">
      <Filter>05.Client.Presentation</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\ValtanCinematicCameraDocument.h">
      <Filter>04.Client.Documents</Filter>
    </ClInclude>
    <ClInclude Include="..\..\..\Client\Public\ValtanPatternFlowDocument.h">
      <Filter>04.Client.Documents</Filter>
    </ClInclude>
  </ItemGroup>
  <ItemGroup>
    <None Include="..\README.md">
      <Filter>00.Start</Filter>
    </None>
  </ItemGroup>
</Project>

```

## 함께 갱신할 참조

- `C:/Users/tnest/Desktop/LostArk/.md/GB/gotchas.md`
- `C:/Users/tnest/Desktop/LostArk/.md/TEAM/발탄인수인계서.md`
- `C:/Users/tnest/Desktop/LostArk/CLAUDE.md`
- `C:/Users/tnest/Desktop/LostArk/Framework.sln`
- `C:/Users/tnest/Desktop/LostArk/Tools/Build/Invoke-BuildAndRegression.ps1`
- `C:/Users/tnest/Desktop/LostArk/Tools/Build/test_build_profile_contract.py`
- `C:/Users/tnest/Desktop/LostArk/Tools/CompositionPipeline/test_native_composition_facade_contract.py`
- `C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/test_action_presentation_workbench_contract.py`
- `C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/test_valtan_balance_tool_contract.py`
- `C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/test_valtan_boss_tool_contract.py`
- `C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/test_valtan_dynamic_ghost_consumer_contract.py`
- `C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/test_valtan_live_combat_debug_visibility_contract.py`
- `C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/test_valtan_pattern_sound_occurrence_pin_contract.py`
- `C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/test_valtan_requested_pattern_coverage_contract.py`

## 검증 명령

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Insiders\MSBuild\Current\Bin\amd64\MSBuild.exe' 'Tools\BossToolTests\Default\BossToolTests.vcxproj' /t:Build /m:1 /p:Configuration=Debug /p:Platform=x64 /p:BuildProjectReferences=false /p:PreferredToolArchitecture=x64 /p:WindowsSDKToolArchitecture=Native64Bit /v:minimal
& '.\Tools\BossToolTests\Bin\Debug\BossToolTests.exe' --action-composition-graph-contract
git diff --check
```

기존 Shared Debug library와 EngineSDK Debug import library를 사용한다. 실제 사용자가 보는 VS 필터는 solution 변경을 다시 로드한 후 확인한다. Client 화면은 실행하지 않는다.
