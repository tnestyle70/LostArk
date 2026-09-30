# 발탄 무력화·부위 파괴 위치와 에스더 초상화

## G00. 현재 실측과 범위

`MainApp::Update_EstherGauge`는 발탄의 Ctrl+Z/Ctrl+C 초상화를 각각 바훈투르/실리안으로
매 프레임 덮어쓴다. `Data/UI/Esther/EstherUI.json`과 Server Esther roster는 이미
실리안/웨이/바훈투르 순서다. 런타임 override만 올바른 순서로 교정한다.

무력화의 `mechanicHeadOffsetX/Y`, `mechanicWidthScale/HeightScale`은 쿠크와 발탄이
공유한다. `WorldHealthBarView`의 발탄 부위 파괴 PNG는 머리 기준 -24/-40과 원본 72px를
고정 사용한다. Server 판정은 변경하지 않고 기존 HUD 소비자에서 표시·저장을 연결한다.

## G01. MainApp의 발탄 전용 HUD 값

기존 `ReadHudBarPositions`, Save/Reload와 `healthBarPositions` 객체를 확장한다.
`valtanStaggerHeadOffsetX/Y`, `valtanStaggerWidthScale/HeightScale`과
`valtanArmorBreakOffsetX/Y`, `valtanArmorBreakWidthScale/HeightScale`을 optional로 읽는다.
무력화 누락값은 로드 시 쿠크의 현재 값을 상속하며 이후 별도 배열에 저장한다. 파괴 PNG
누락값은 기존 위치 -24/-40, 배율 1/1을 보존한다. offset은 finite ±1280, 배율은 0.1..3이다.
현재 Data JSON에 새 8개 필드만 추가해 최초 무력화 값을 쿠크와 같게 고정한다.

`Apply_MechanicBarRect`는 실제 boss archetype이 발탄/망령 발탄일 때 전용 값을 소비한다.
F1 `Health bar positions`의 `Valtan Stagger`와 `Valtan Armor Break PNG`에서 위치·크기와
각 `Show debug`를 조절한다. debug 표시만 기믹 진행 여부를 생략하며 실제 살아 있는
발탄의 머리 투영, 화면/UI 숨김 경계는 유지한다. debug 상태는 저장하지 않는다.

Save는 기존 필드별 CAS, 최신 디스크 병합, 공유 writer lock, 임시 파일 검증, 백업·원자
교체와 충돌 복구를 그대로 확장한다. 잘못된 새 값 하나라도 있으면 전체 읽기를 거절한다.

## G02. WorldHealthBarView의 PNG 소비

기존 `WorldHealthBarView.h/.cpp`에 PNG tuning setter를 추가한다. 각 bar는 생성 시
layout의 원본 rect를 보관하며 매 프레임 원본 크기에 배율을 곱해 누적 확대를 막는다.
MainApp가 저장/preview 값을 전달하고 기존 Server armor availability 또는 명시 debug일
때만 표시한다. 실제 파괴 성공 텍스트·Server 상태와 HP/shield offset은 변경하지 않는다.

## G03. 검증과 전달

새 C++ 파일과 project/filter 항목은 없다. 기존 UTF-8 BOM 없음·CRLF를 보존한다.
JSON parse, 새 필드와 기존 값 보존, 기존 Server roster·키와 초상화 대응, 실제 소비 함수의
읽기/저장·독립 값·잘못된 값·CAS·비누적 크기를 집중 검증한다. root가 통합 Debug/Release
Client Build를 조율한다. 제품 Client/UI는 실행하지 않으며 최종 화면과 튜닝은 사용자 확인이다.
public 계약 변경은 root가 TEAM_GAMEPLAY_INTERFACE_HANDBOOK의 HUD 절에 통합한다.

## G04. World Object 폭발 Sound 편집

추가 담당 범위는 `ValtanCombatObjectSoundCueDocument`와 Workbench의 Sound owner다.
기존 cue의 stable bindingId를 유지하고 soundEvent와 playbackOffsetMs만 메모리 draft로
편집한다. offset은 서버 발생 시각을 미루는 값이 아니라 WAV 시작 부분을 생략하는 ms다.
실제 hit 시각은 World Object collider의 TIMED/owner-hit-chain 값이 소유한다.

document에 원문 baseline·draft generation·dirty를 보관하고 Update_CueDraft,
Prepare_Save, Accept_Save를 추가한다. Prepare는 검증된 candidate만 만들고 직접 파일을
쓰지 않는다. BalanceTool의 단일 canonical owner transaction에 CombatObjectSound
baseline/candidate pair를 포함하며 pipeline 인자는 root가 연결한다. 이전 직접
Begin_SourceReplacement는 retired 상태를 유지한다. Workbench는 변경 중 reload를 막고
명시 discard·save receipt accept를 연결한다. UI는 기존 Valtan sound catalog dropdown과
source offset을 Object hit에 붙인다. 새 schema·C++ 파일·project/filter는 없다.

## G05. 바위 원본 준비 연출과 실제 폭발 시각

원본 off.full Effect의 처음 1820ms 준비 구간을 생략하지 않는다. 세 owner-hit-chain
바위는 기존 cone 분류의 0/1500ms 시작 순서를 유지하고 각 시작 때 armed marker를
발행한다. 기존 hit.trigger.atMs=1820를 준비 길이로 소비하여 해당 시작+1820ms에
collider와 Sound를 발행한다. Shared 분류 함수와 별도 runtime 경로는 추가하지 않는다.
발악 바위는 기존 hit 시각4133ms에 명시 preparation presentation event를 추가하고
실제 hit5953ms, 수명7153ms로 옮겨 원래 terminal tail1200ms도 보존한다.

BossCatalog의 기존 armed visual·stopActiveOnArmed·armedEffectOwnsTerminal 계약으로
원본 전체 Effect를 한 번만 재생한다. local preview는 Server와 같은 preparation/hit
시각을 수집하고 fixed preparation도 같은 armed owner로 처리해 hit 때 이중 재생하지
않는다. Source 데이터는 최신 bytes 재확인·backup·원자 교체 후 canonical publisher를
통해 product로 만든다. Server native direct/delayed hit 경계 검증은 gameplay agent가
담당하고 root가 빌드·publisher·typed editor 저장을 통합한다.

## G06. 실제 마력구 무력화 snapshot의 HUD 소비 교정

사용자는 Show debug에서는 저장 위치가 보이지만 실제 마력구에서 무력화바가 나오지
않는다고 확인했다. G13 이후 authored VALTAN_STAGGER_SLOT의 CHANNEL은 Server의
SET_STAGGER_GAUGE를 사용하고 snapshot은 iCurrentStagger/iMaximumStagger를 보낸다.
그러나 CombatHUDViewModel::Apply_Boss는 이 CHANNEL에서 옛 HP 누적 response threshold가
0이면 최대치를0으로 덮어써 gauge를 숨긴다. debug만 보이는 원인이다.

Client/Private/CombatHUDViewModel.cpp의 기존 마력구 projection 분기에서 authored CHANNEL은
Server stagger current/max를 소비하도록 바꾼다. 이전 VALTAN_MAGIC_ORB_STAGGER_76/window만
response threshold가 있을 때 기존 response progress를 사용하고, 없으면 stagger로 돌아간다.
정확한 pattern/action/archetype 조건, current 상한 clamp,0최대치 숨김과 다른 기믹의
직접 mechanic snapshot 소비는 유지한다. Server·Shared·MainApp·저장 HUD JSON은 변경하지 않는다.

같은 실제 Apply_Boss 함수와 WORLD_ENTITY_SNAPSHOT 타입을 작은 native 검사에 연결하여
기존 함수에서 authored response0/current0/max50000이 숨겨지는 실패를 먼저 기록하고
수정 후 현재량·다른 최대치·상한·0최대치·다른 action/pattern·legacy response/fallback 및
다른 보스의 직접 gauge를 검증한다. Product 빌드는 root가 수행하며 GPU 화면은 사용자가 판정한다.
새 제품 파일/project/filter 등록은 없다.
