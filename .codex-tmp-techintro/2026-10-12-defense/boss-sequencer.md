## B01. 보스 패턴: 저작한 규칙이 실제 전투가 되는 과정 — PPT 6~7

이 카드는 발탄의 반복 판정으로 시작한다. 아래 후속 질문까지 범위를 넓혀야 쿠크세이튼과 빙고를 설명할 수 있다. 한 번의 90초 답변을 전체 보스 시스템 이해의 증거로 삼지 않는다.

### ⓪ 코드를 열고 먼저 확인할 것

현재 파일의 2026-10-12 확인 위치다. 편집으로 행이 이동하면 아래 심볼을 찾는다. 답변 연습 전에 다음 표를 채워라. `값의 의미 / 소유자 / 초기화 지점 / 변경 지점 / 다음 소비자` 다섯 칸이다.

| 순서 | 열 파일과 범위 | 직접 찾아 적을 내용 |
|---|---|---|
| 1 | [Valtan.gameplay.json](C:/Users/tnest/Desktop/LostArk/Data/Valtan/Valtan.gameplay.json:681), `VALTAN_WHIRLWIND`의 `SPIN` 729~744행 | `durationMs`, `hit.schedule`을 읽고 판정 예약 시각을 계산해 봐. 저작 값에 이번 보스가 처리한 이력이 있는지도 확인해. |
| 2 | [ServerWorldEntity.h](C:/Users/tnest/Desktop/LostArk/Server/Public/ServerWorldEntity.h:405), 405~408행 | `iPatternHitCount`, `iPatternHitIntervalMs`, `iPatternHitDelayMs`, `iAppliedPatternHitCount` 중 무엇을 실행 중에 바꾸는지 구분해. 이름만 보고 의미를 정하지 마. |
| 3 | [ValtanBrain.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:1791), `EnterPatternStage` 1791~1844행 | stage 진입에서 복사·초기화하는 상태와 이전 action의 정리 호출을 찾아. 동일 stage 재진입 때 무엇이 달라져야 하지? |
| 4 | [ValtanBrain.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:3473), `Update` 안의 3473~3537행 | 반복문 조건, 다음 시각 계산, `ApplyPatternHit` 호출, 소비 횟수 증가, stage 종료의 순서를 그려. 처음에는 피해 계산 내부까지 들어가지 않아도 된다. |
| 5 | [ValtanBrain.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:100), `StageElapsedTicks`·`HasElapsedMilliseconds` | 시작 tick, 현재 tick, ms의 관계를 확인해. 설명용 ms 시계와 제품의 정수 tick 계산에서 생략한 조건을 적어. |
| 6 | [GameplayCatalog.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:5039), `PATTERNSTAGE` 분기 | 게시 행의 어떤 필드가 stage 정의로 들어가는지 연결해. 저작 JSON을 이 함수가 직접 읽는다고 말해도 되는지 실제 입력 경로로 확인해. |

현재 저작 값은 count 4, firstOffset 0ms, interval 350ms다. 다음은 **가상 관측 입력**이다. 첫 기회를 이미 소비한 뒤 800ms에 갱신하고, 같은 800ms로 다시 호출한다. 각 호출의 `처리 전 상태 / 반환한 판정 기회 / 처리 후 상태`를 먼저 적어라. 제품에서 800ms 그대로의 지역 변수를 찾으라는 뜻은 아니다. 실제 실행은 사용자가 breakpoint로 확인하고, 예측과 관측을 별도로 남긴다.

### ① 문제 — 15초

“이 패턴 시스템이 책임져야 할 일을 입력과 결과로 말해 봐. 저작한 횟수대로 판정하는 것 외에, 같은 시점으로 두 번 갱신되거나 다음 갱신이 늦었을 때 반드시 지켜야 할 것은 뭐지? 판정 기회 소비와 실제 피해 발생을 같은 결과로 취급해도 되나?”

### ② 직접적인 해법과 한계 — 20초

“매 갱신마다 `시간이 됐으면 한 번 공격한다`로 시작해 보자. 방금 적은 800ms 입력에서 이 구현이 무엇을 하는지 보여 줘. 요구사항을 만족한다면 왜 충분한지, 만족하지 못한다면 처음 어긋나는 호출과 상태를 지정해. ‘프레임 드롭에 약하다’라는 말만으로 끝내지 마.”

### ③ 현재 메커니즘 — 25초

“정의, 실행용 복사본, 소비 이력을 실제 변수로 구분해 봐. stage 진입부터 반복문이 멈출 때까지 호출 순서로 설명해. 이력을 하나 지운다면 과거가 다른 두 실행을 여전히 구별할 수 있나? 명중 대상이 없었던 호출에서는 무엇이 남는지 코드로 확인했나?”

### ④ 비교 — 15초

“소비 개수 하나, 기회마다 처리 여부를 저장하는 배열, 시각순 큐 가운데 현재 요구에 맞는 표현을 골라 봐. 임의의 한 공격 취소나 순서가 다른 실행을 새로 요구한다면 어떤 선택을 다시 검토하겠어? 자료구조 이름 대신 필요한 연산과 보존할 이력을 말해.”

### ⑤ 얻은 것과 비용 — 15초

“현재 표현으로 보장하는 것과 보장하지 못하는 것을 하나씩 말해 봐. 저작 필드를 하나 추가하면 정의 파싱, 실행 상태, 판정, Client 표현 중 어디까지 함께 확인해야 하지? 직접 측정하지 않은 속도 개선 수치는 붙이지 마.”

### 독립 구현과 검증

원본을 닫고 **예약된 판정 기회를 소비하는 작은 함수**를 작성해라. 입력은 예약 정의·현재 경과 시간·이번 실행의 상태, 출력은 이번에 소비한 기회 목록이다. 실제 충돌·HP·네트워크는 넣지 않는다. 같은 시간의 중복 호출, 여러 예약을 건너뛴 호출, 마지막 예약 뒤의 호출, 새 실행 시작을 직접 검증한다. 기대값을 실행 전에 쓴다.

기본형을 확인한 뒤 **가상 요구 변경**으로 불규칙 예약 시각을 지원해 봐. 수정한 상태와 수정하지 않은 상태를 설명하고, 제품의 `HitOffsetsMs` 경로와 차이를 대조해라. 정답 코드의 문장 일치는 평가하지 않는다. 직접 작성한 범위, 받은 힌트, 첫 실패와 수정 근거, 아직 생략한 제품 제약을 남긴다.

증거는 소스 위치를 연결한 상태표, 예측과 실제 출력, 작성한 함수, 경계 입력, 조건 변경의 수정 근거다. 이 증거가 있으면 해당 소비 계약을 설명·구현했다고 말할 수 있다. 보스 전체를 설계했다거나 과거에 같은 장애를 해결했다는 증거가 되지는 않는다.

### 전체 이해를 위해 이어갈 질문

| 후속 범위 | 실제 시작점 | 질문과 확인할 증거 |
|---|---|---|
| B01-A. 발탄 선택·분기·종료 | [ValtanBrain.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:1247) `SelectPattern`; 3522행 `ApplyTimeoutBranch` 호출, 3533행 `FinishPattern` 호출 | “다음 stage와 다음 pattern은 누가 결정하지? 현재 저작 선택 정책, HP 경계, branch 조건을 실제 입력 하나로 따라가 봐. 취소·재진입에서 이전 action 결과와 판정 이력이 섞이지 않는 근거는?” 선택 입력→결정→초기화의 상태표를 남긴다. 가중 선택 코드의 존재만으로 현행 정책을 단정하지 않는다. |
| B01-B. 저장·게시·실행 정의 | [KoukuSaydonActionWorkbench.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:1926) 저장 호출; [KoukuSaydonCompositionDocument.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonCompositionDocument.cpp:5928) `Save_Atomic`; [GameplayCatalog.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:1646) `Load_BootstrapBytes` | “필드 하나를 고르고 저작본→projector→게시 row→Server 정의를 연결해 봐. Save, Publish, 실행 중 활성화가 각각 실패하면 무엇이 유지되나?” 실제 필드와 파일의 대응표, 실패 검사 위치를 남긴다. 이 카드 자체는 Save·Publish를 수행하라는 명령이 아니다. |
| B01-C. 쿠크 Logic·occurrence·플레이어별 상태 | [KoukuSaydonComposition.json](C:/Users/tnest/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:26728) `KAKULSAYDON_G1_PATTERN_109`; [KoukuSaydonLogicRuntime.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:128) `Build`, 1749행 `Update`, 2316~2382행 접촉 처리; [KoukuSaydonLogicRuntime.h](C:/Users/tnest/Desktop/LostArk/Server/Public/KoukuSaydonLogicRuntime.h:49) | “같은 Logic을 두 시점에 배치하면 무엇을 공유하고 무엇을 따로 기억하지? 두 플레이어의 진입·이탈·재진입을 섞지 않는 key는? 시작과 끝을 tick으로 바꿀 때 각각 어느 절대 시각을 변환하나?” 가상 두 플레이어 입력으로 `Answers`, `InsidePlayers`, `NextContactHitTicks` 중 실제 분기가 사용하는 상태를 추적한다. 파일의 Gate1 이름으로 실제 관문을 추정하지 않는다. |
| B01-D. 빙고 보드와 Flow의 수명 | [GameRoom_KoukuPlayerCommands.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuPlayerCommands.cpp:202) `Stop_KoukuBingoDuration`, 261행 `Begin_KoukuBingoDuration`, 300행 `Update_KoukuBingo`; [GameRoom_KoukuRaidFlow.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuRaidFlow.cpp:446) `Start_KoukuBingoSpecialPattern`; [KoukuSaydonLogicRuntime.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:2656) `CKoukuBingoRuntime::Detonate` | “일반 패턴을 끝내는 것과 빙고 보드를 끝내는 것이 같은 사건인가? WINDOW가 닫힐 때 이미 표시·투하·설치한 폭탄은 누가 끝내지? 전체 Parent UI가 새 반복 실행기를 만드는지 실제 호출로 보여 줘.” 가상 입력으로 일반→특수→일반 전환과 종료 중 폭탄의 상태표를 작성한다. 별도 연습에서는 작은 보드의 칸 반전·줄 판정만 구현하고 실제 폭탄·레이드까지 검증했다고 확대하지 않는다. |
| B01-E. 판정과 Client 표현 | [GameRoom_KoukuAudition.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuAudition.cpp:1588) Logic `Update` 소비; [KoukuSaydonPresentationPlayer.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:3716) `Prepare_AdmittedRun`·3761행 `Update` | “판정 결과와 화면 재생이 연결되는 ID·실행 identity·시계는 무엇이지? 늦은 상태가 왔을 때 어떤 실행의 표현인지 어떻게 확인하나? Client collider 표시를 피해 성공의 증거로 삼을 수 있나?” Server 출력→Shared 전달→Client 소비의 실제 필드 하나를 끝까지 추적하고 중단·재시작을 별도로 확인한다. |

근거 기록: [보스 학습 RESULT](C:/Users/tnest/Desktop/LostArk/.md/GB/10-11/2026-10-11_BOSS_PATTERN_LEARNING_RESULT.md), [빙고 Parent PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-08/2026-10-08_BINGO_AUTHORING_PARENT_IMPLEMENTATION_PLAN.md), [빙고 Parent RESULT](C:/Users/tnest/Desktop/LostArk/.md/GB/10-08/2026-10-08_BINGO_AUTHORING_PARENT_RESULT.md). 과거 PASS는 이번 독립 수행 결과로 옮기지 않는다.

## S01. 시퀀서: 편집 화면, 시간 평가, 실행 소유권 — PPT 8

Boss·Sequence·World/Movie를 같은 창에서 다룬다는 것과 문서·플레이어·게임 판정의 소유자가 같다는 것은 따로 확인한다. 첫 카드는 공용 UI에서 실제 재생 owner까지 연결하고, 시간 평가의 작은 계약을 구현하는 범위다.

### ⓪ 코드를 열고 먼저 확인할 것

| 순서 | 열 파일과 범위 | 직접 찾아 적을 내용 |
|---|---|---|
| 1 | [CompositionWorkbenchSession.h](C:/Users/tnest/Desktop/LostArk/Client/Public/CompositionWorkbenchSession.h:65), `ICompositionWorkbenchSession`; [SequencerTool.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/SequencerTool.cpp:1562), `Selected_Session` | Boss/Sequence/World 선택이 어느 session으로 연결되는지 표로 적어. UI가 소유하는 것과 session이 소유하는 것을 실제 멤버·호출로 구분해. |
| 2 | [SequencerTool.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/SequencerTool.cpp:1967), `Render`의 1967~2022행 | `Begin_WorkbenchFrame`과 `End_WorkbenchFrame` 사이에 무엇을 읽고 언제 편집 명령을 적용하는지 찾아. 같은 session을 다른 도구가 그 프레임에 이미 열었을 때의 분기도 확인해. |
| 3 | [WorldSequencePlayer.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequencePlayer.cpp:535), `Sample_Track`·`Compose_SampledRecord`; 707행 `Play` | key 시간, 위치·회전·크기·표시, 배치 baseline의 의미를 적어. 같은 시각을 반복 평가할 때 값이 누적되는지 대조해. |
| 4 | [WorldSequencePlayer.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequencePlayer.cpp:952), `Stop_Instance`·`Seek_InstanceToMs`; 1120행 `Update` | 정상 완료, 실패, 명시 Stop, 시간 이동이 각각 어디로 가는지 그려. 모든 완료가 원상복구라고 결론내리지 말고 `m_Active`·`m_Held`·duration 분기를 확인해. |
| 5 | [MainApp.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:11895), Movie seek callback; [ClassSelectionPresentation.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/ClassSelectionPresentation.cpp:427), `PHASE::SourceTimeMs`·`MovieTimeMs`, 935행 `Seek`, 1064행 `Sample_Frame`, 1427행 `Update` | UI cursor, Movie 시간, source 시간, playback rate가 소비되는 자리를 구분해. 카메라·world·Effect가 어느 시각을 받는지 하나씩 연결해. |

**가상 관측 입력**으로 위치 key 두 개와 표시 key를 정해라. 중간 시각, key 경계, 이전 시각으로 이동, 같은 시각 반복 평가의 기대값을 먼저 적는다. 화면이 비슷해 보인다는 인상 대신 현재 cursor·sample 시간·선택된 두 key·출력 transform을 비교한다. 실행 화면과 breakpoint 조작은 사용자가 수행한다.

### ① 문제 — 15초

“시퀀서를 ‘시간에 맞춰 재생하는 도구’보다 구체적으로 설명해 봐. 누구의 어떤 데이터를 편집하고, 어떤 대상으로 무엇을 평가하며, 전투 판정까지 바꿀 권한이 있는지는 어디에서 정해지지?”

### ② 직접적인 해법과 한계 — 20초

“매 프레임 현재 위치에 이동량을 더하는 방법과, 지정 시각의 key를 평가하는 방법을 비교해 봐. 일시정지, 뒤로 이동, 같은 시각 반복 평가 중 어느 입력에서 차이가 드러나지? 특정 시각의 상태를 구하는 일과 그 구간을 지나는 동안 사건을 발생시키는 일도 같은 함수로 충분한가?”

### ③ 현재 메커니즘 — 25초

“선택한 box 하나를 잡고 공용 UI→session→문서 또는 재생 owner→대상 반영을 실제 함수로 연결해 봐. 시간 단위와 기준을 말하고, key 보간값이 어느 baseline에 합성되는지 설명해. Movie 시간과 source 시간이 다를 때도 cursor 숫자를 그대로 모든 소비자에 넣을 수 있나?”

### ④ 비교 — 15초

“구간 안의 key를 매번 처음부터 찾는 방법과 이전 구간을 기억하는 방법을 비교해 봐. 앞으로만 재생할 때와 임의 seek할 때 각각 어떤 조건이 필요하지? 공용 UI를 유지하면서 각 owner에 위임하는 방식과 한 실행기에 전부 넣는 방식은 어떤 변경 비용이 다를까?”

### ⑤ 얻은 것과 비용 — 15초

“편집·재생을 나눠 얻는 이점 하나와 추가로 관리해야 하는 상태 하나를 말해 봐. Stop, 자연 종료, 실패, 새 문서 적용에서 무엇을 복원하거나 남길지는 어디서 결정하지? 실제 측정 없이 전체 시퀀서가 빠르다고 주장하지 마.”

### 독립 구현과 검증

원본을 닫고 **시각을 받아 작은 key track의 값을 반환하는 함수**를 작성해라. 처음에는 scalar 값 하나와 표시 bool이면 된다. key 앞·정확한 key·중간·마지막 이후·같은 시각 반복·뒤로 이동을 검증한다. 잘못 정렬된 key와 중복 시각을 받는 정책을 먼저 정하고, 실패 시 이전 정상 track을 보존하는 작은 로드 경계를 붙여라.

그다음 **가상 요구 변경**으로 한 번 발생하는 이벤트를 추가한다. 연속 재생, 큰 시간 건너뜀, 명시 seek, 반복 재생의 정책을 각각 먼저 써라. 샘플 함수와 사건 소비 함수의 입력·상태를 구분하고, 예상 이벤트 목록과 실제 목록을 대조한다. 이 연습의 정책을 제품 사운드·Effect 정책의 정답으로 옮기지 말고 차이를 설명해라.

증거는 각 시간 입력의 사전 예상값, 실제 출력, 독립 코드, 잘못된 key 거절 후 상태, 요구 변경의 차이다. 이후 제품의 소유권·회전·좌표계·사운드·Effect 수명을 붙여야 전체 시퀀서에 대한 설명으로 넓힐 수 있다.

### 전체 이해를 위해 이어갈 질문

| 후속 범위 | 실제 시작점 | 질문과 확인할 증거 |
|---|---|---|
| S01-A. Boss와 Sequence의 실행 권한 | [MainApp_SequenceViewer.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_SequenceViewer.cpp:246) `ExecuteSequenceViewerAction`, 299~312행 | “패턴 재생, world trigger, sequence 정지 요청이 같은 명령인가? UI에서 나간 요청이 어느 typed sink에 도달하지? 정지 요청이 이미 적용된 피해·소환까지 되돌리는지 코드로 확인해.” 요청별 필드→Server handler→결과 응답을 하나씩 추적한다. |
| S01-B. World의 baseline·대상 수명 | [WorldSequencePlayer.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequencePlayer.cpp:751) 재생 준비·baseline 수집; 952행 정지; 1120행 완료 처리 | “두 재생이 같은 placement를 건드리면 원래 상태의 소유자는 누구지? 외부에서 숨긴 대상을 Stop이 다시 보이게 할 수 있나? 정상 완료와 고장 난 instance를 같은 상태로 지워도 되는가?” 대상 ID, 원본 transform, runtime visibility, active/held 이동을 추적한다. |
| S01-C. Movie 시간과 source 시간 | [ClassSelectionPresentation.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/ClassSelectionPresentation.cpp:92) `SampleClock`, 427~445행 시각 변환, 1427~1508행 `Update` | “슬로모션 곡선의 한 구간을 골라 두 시간의 대응을 계산해 봐. 역변환은 언제 성립하지? Intro 완료·Loop 진입·마지막 화면 유지에서 시계가 어떻게 바뀌나?” 실제 clock key를 읽고 계산과 관측을 분리한다. 250ms delta 제한을 Server 판정 시계의 계약으로 확대하지 않는다. |
| S01-D. 연속 재생과 scrub의 부작용 | [WorldSequencePlayer.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequencePlayer.cpp:992) `Seek_InstanceToMs`; [ClassSelectionPresentation.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/ClassSelectionPresentation.cpp:1064) `Sample_Frame` | “큰 한 프레임과 사용자가 뒤로 이동한 사건이 같은가? `discontinuous`, `seekSounds`를 누가 설정하고 소비하지? 같은 시각 재평가가 새 사운드·Effect 생성이 되는지 실제 소비자를 찾아.” 두 호출의 인수·이전 시각·handle 수명 증거를 남긴다. |
| S01-E. Undo/Redo와 저장의 경계 | [ClassSelectionPresentation_Authoring.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/ClassSelectionPresentation_Authoring.cpp:520) `Capture_AuthoringHistory`, 558행 `Restore_AuthoringHistory`, 595행 `Undo_Authoring`, 607행 `Redo_Authoring` | “Save 뒤 Undo가 디스크도 과거로 돌리는가? 외부 편집과 같은 필드 충돌이 있으면 draft와 history stack이 각각 어떻게 되는가? Preview 실패를 끼웠을 때도 같은가?” 가상 작은 문서에서 수정→저장 baseline 갱신→외부 변경→Undo 성공/실패를 구분해 추적한다. 제품 원본을 덮어쓰며 연습하지 않는다. |

근거 기록: [World Sequence RESULT](C:/Users/tnest/Desktop/LostArk/.md/GB/08-31/2026-08-31_WORLD_SEQUENCE_MAP_TOOL_RESULT.md), [Sequencer Undo/Redo PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SEQUENCER_UNDO_REDO_IMPLEMENTATION_PLAN.md), [Sequencer Undo/Redo RESULT](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SEQUENCER_UNDO_REDO_RESULT.md). 오래된 문서의 당시 미구현 경계를 현재 상태로 복사하지 않고 위의 실제 호출 경로를 우선한다.
