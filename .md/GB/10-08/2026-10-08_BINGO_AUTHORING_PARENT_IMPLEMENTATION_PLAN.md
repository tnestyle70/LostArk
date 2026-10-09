# 빙고 전체 Parent와 공통 Logic 편집

## 목표와 현재 기준

빙고를 Composition과 F1에서 전체 Parent, 공통 보드 Logic, 반복 패턴, 세 번째 폭탄 특수 Parent로 구분하여 선택한다. Box Detail에서 수정한 폭탄 시간과 Logic 구간은 기존 저장·게시 경로를 통해 Server가 소비한다.

현재 정본은 `kakulsaydon.flow.bingo`의 35개 entry와 반복 시작 entry, `bingoSpecialPatternId`다. `KAKULSAYDON_G1_PATTERN_129`의 BINGO_BOARD가 공통 보드를 시작하고, 특수 Parent 107이 이동·메두사·블랙홀을 묶는다. 공통 보드는 일반 공격 및 특수 전환과 독립적으로 살아 있으므로, 전체 Parent는 이 저장된 Flow의 편집 뷰로 제공한다. 일반 Pattern으로 다시 감싸서 두 번째 반복 시계를 만들지 않는다.

## G01. Logic 저장과 게시 계약

`KoukuSaydonCompositionDocument.h/.cpp`의 BINGO_BOARD 정의에 `bingoActiveMode`, `bingoFirstBombDelayMs`, `bingoBombIntervalMs`, `bingoBombMarkMs`, `bingoBombDropDelayMs`, `bingoBombFuseMs`, `bingoInitialMarkedCells`를 추가한다. 기존 문서의 생략값은 ENCOUNTER, 30000, 20000, 6000, 2000, 4000, 2다. 첫 폭탄 지연 30000ms는 현재 상수 10000+20000ms와 같다.

ENCOUNTER는 전투가 끝날 때까지 새 폭탄을 예약한다. WINDOW는 기존 Logic box의 startMs/durationMs 동안 예약한다. 종료 시 이미 생성된 폭탄은 마저 진행하며 보드 소유권을 지우지 않아 자동 재시작하지 않는다.

시간 범위는 first 0..600000, interval/mark/drop 1..600000, fuse 250..80000ms, 초기 칸 0..25다. mark+drop+fuse는 4*interval 이하로 검증하고 각 phase를30Hz tick으로 올림한 합도 네 interval tick 이내인지 확인한다. Client와 projector와 Server에서 같은 범위를 확인한다. `PATTERNBINGOBOARD` 행은 기본 mechanic trigger를 참조하여 위 값을 운반한다. 기존 정의/게시 데이터의 생략 계약을 보존한다.

## G02. Server 실행

`GameRoom_KoukuPlayerCommands`의 보드 상태가 설정값과 활성 구간을 소유한다. Raid Flow가 공통 로직을 시작할 때 전투 시작 기준의 구간을 사용한다. 일반/특수 패턴 전환에서 보드를 재생성하지 않는다. publisher가 PATTERNBINGOCONTROL marker로 확인한 보드 전용 controller Pattern은 기존 짧은 시작 동작으로 소비하여 그 Logic lifetime만큼 일반 공격을 멈추지 않는다. 보드 전용 Logic을 비활성화한 경우에는 원본 구간을 보존하면서 두 실행 투영에34ms 빈 carrier를 사용한다.

폭탄마다 mark/drop/fuse 시간을 고정한다. planted WORLD 연출은 기존 playback speed로 4000/fuse를 전달하여 심지와 실제 폭발을 맞춘다. 새 protocol이나 별도 Client 판정은 만들지 않는다.

## G03. Composition과 F1

`KoukuSaydonActionWorkbench`는 Bingo 전체 Parent를 저장 Flow에 연결한다. 공통 보드 box와 특수 Parent, 진입 순서와 반복 구간을 시각화한다. Flow entry의 순서·대기와 반복 시작 및 특수 Parent 연결을 기존 candidate validation/undo/save로 편집한다. 자식 Pattern을 열면 기존 모든 lane과 Box Detail을 사용한다.

BINGO_BOARD Box Detail에 위 설정을 표시하고 window를 확장하면 보드 전용 Pattern의 전체 길이도 기존 lane 확장 함수로 맞춘다. 전체 Parent의 공통 row 선택은 실제 Logic occurrence를 가리킨다.

F1의 게시 패턴 목록에 Bingo 전체 Parent, 공통 보드, 반복 패턴, 특수 패턴을 표시한다. 전체 Parent 재생은 기존 Complete Play/Server raid admission을 사용하고 개별 Pattern 재생과 구분한다. 현재 기본 데이터의 숫자와 렌더링 설정은 바꾸지 않는다.

## G04. 검증과 전달

기존 파일만 확장하므로 vcxproj/filters 신규 등록은 없다. native codec 왕복·거절 검사, pipeline projection/bootstrap 검사, Server 빙고 및 raid 계약을 실행한다. Debug Product build와 JSON parse, git diff --check를 확인한다. Client 실행·화면 조작은 하지 않으며 사용자 촬영 화면 검증은 RESULT에서 별도로 남긴다.
