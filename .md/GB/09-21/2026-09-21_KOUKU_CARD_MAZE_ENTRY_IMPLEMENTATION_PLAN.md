# 쿠크 카드미로 진입 배우 재사용과 플레이어 배치 구현 계획

## G00. 현재 코드와 저장본

2026-09-21 revision 2025의 Action P77은 WORLD26으로 별도 쿠크 모델을 생성한다. 실제 G2 쿠크는 그대로 남는다. 새 logic112는 TRIGGER 이름만 있고 triggerKind가 없다. 소멸 Effect21–24의 MAP 좌표와 시작 3245/3426/3695/3930ms가 저장돼 있으나 hide occurrence는 다른 시각이다.

## G01. 기존 모델과 Server 위치 계약

KoukuSaydonPresentationAssetService.cpp의 기존 body prototype 준비 과정에서 BossCatalog.animationSetId가 bodyModel과 다른 경우 동일 skeleton donor를 Attach_AnimationSet한다. 이미 포함된 clip을 중복 부착하지 않으며 donor admission 실패는 기존 body를 버리는 silent fallback으로 숨기지 않는다. P77은 WORLD26을 제거하고 기존 actor animation occurrence로 동일 11.95초 baked clip을 재생한다. 원본 actor TRS와 native b_root translation을 합성해 기존 Server bossMotion의 optional keys 2..512개로 반영한다. 기존 nav root 이동은 7410~7590ms 퇴장 경계를 차단하므로 쓰지 않는다. GameRoom_Helpers는 keys의 최초 지면 유효성을 유지하고 명시된 컷씬 퇴장 경로를 허용한다. 원본 donor는 유지하고 Client rootVerticalScale=0으로 XYZ 이중 이동을 제거한다. 새 객체·새 Engine 경로·새 C++ 파일은 만들지 않는다.

## G02. typed 진입 스테이징

새 CARD_MAZE_STAGE_PLAYERS는 Composition의 playerEntryEffectOccurrenceIds 1..4개를 같은 Pattern MAP EFFECT occurrence로 resolve한다. Product playerEntryPositions와 PATTERNCARDMAZESTAGING bootstrap row를 Server가 읽는다. Client codec/UI/projector/publisher는 root 담당이다. GameplayCatalog.h의 enum과 PlayerEntryPositions, GameplayCatalog.cpp parser, Brain의 runtime defense, LogicRuntime의 transactional staging은 이 작업에서 변경한다.

Server는 기존 카드미로 entry roster를 한 번 고정하고, 모든 대상의 navigation exact ground와 collision, 참여 상태, destination 수를 검증한 뒤 모든 player 복사본을 한 번에 commit한다. 실패는 위치를 보존하고 이 run의 이후 숨김과 maze transfer도 실행하지 않는다. 성공 후 기존 HIDE_NEXT와 ENTRY_HIDDEN snapshot/render 경로를 재사용한다. 실제 maze 입장, 종료, 중단의 reveal을 유지한다. 소멸 hide occurrence 시각은 참조 Effect 시작에 맞춘 후보로 준비한다.

## G03. 검증과 교체 경계

실제 donor/base WModel skeleton·clip 검사, 변경한 실제 C++ TU scratch compile, 기존 Server contract runner에 실제 staging/rollback/hide/reveal 회귀를 추가 또는 scratch native 소비자로 검증한다. Data 후보는 out에 stable ID/field patch로 준비하고 root가 최신 저장본에 병합한다. Client/UI 실행, 제품 링크, 실행 중 process 종료, live JSON 직접 교체는 하지 않는다. 새 프로젝트 등록은 필요 없다. 결과는 같은 topic RESULT에 기록한다.

## G04. Client authoring codec와 선택 UI

Client CompositionDocument의 기존 Logic 정의에 PlayerEntryEffectOccurrenceIds 순서 배열을 추가한다. JSON에는 playerEntryEffectOccurrenceIds만 저장하며 좌표는 복제하지 않는다. parser는 CARD_MAZE_STAGE_PLAYERS에만 필드를 허용하고 1..4개의 유일한 stable ID를 요구한다. 각 Logic box는 동일 Pattern의 고정 MAP EFFECT를 참조해야 한다. Bone·WORLD·followBoss anchor는 제외하며 이동 시각은 각 Effect 시작 시각 이하이어야 한다. 전체 문서 Validate와 기존 Save_Atomic 경로가 이 경계를 소비한다.

ActionWorkbench의 기존 Trigger kind 선택과 Apply Values 경로에 새 kind를 추가한다. 선택 Pattern의 적합한 Effect 중 최대 4개를 순서대로 고르고, 선택 목록에서 자리 순서를 바꾸거나 제거할 수 있다. bundle의 stateful owner와 parent expansion 제한에도 같은 trigger를 추가한다. 기존 dirty 3파일은 out/KoukuCardMazeAuthoring20260921/before에 보존하며 변경 TU scratch compile과 실제 parser/serialize/validate roundtrip 및 잘못된 참조·타이밍·형식 거부를 검증한다. Client UI 실행은 하지 않는다.

## G05. 원본 이동 곡선의 기존 bossMotion 확장

기존 bossMotion의 선택적 keys 배열은 2..512개의 {timeMs, position}로 저장한다. 시간은 엄격히 증가하고 첫·마지막 key는 기존 interval/endpoints와 일치해야 한다. keys 없는 문서는 기존 동일 base Y의 선형 이동을 보존한다. 샘플 경로는 XYZ를 보간하며 Client codec과 product parser, 기존 Sample_KoukuSaydonBossMotion 경로가 같은 곡선을 소비한다. Parent clock offset은 모든 key에 적용한다. Workbench는 샘플 경로의 원본 key를 보존하고 단순 endpoint UI로 덮어쓰지 않는다. timeline 삽입은 key 시간을 함께 이동하며 삭제로 key 시간이 겹치면 candidate를 거부한다. 현재 소스 및 실제 키 후보로 roundtrip/경계·중간 보간/linear 회귀를 확인한다.

## G06. World publisher의 같은 이동 계약

최종 domain publish의 world.gameplay가 encounter를 읽을 때에도 bossMotion의 optional keys를 같은 계약으로 검증한다. 기존 선형 Y 고정 검증은 keys가 없을 때 유지한다. keys가 있으면 2..512개, 각 timeMs/position 두 필드, 구간 안의 엄격한 시간 증가, finite XYZ와 ±100000 범위, 양 끝점 시간·좌표 일치를 확인한다. World publisher는 기존처럼 mechanicTriggers 내부 정의를 Gameplay publisher에 위임하므로 새 staging trigger의 별도 허용 목록은 만들지 않는다. 변경 전 파일을 out에 보존하고 PowerShell 구문 검사 및 실제 World publisher Validate로 확인한다.
