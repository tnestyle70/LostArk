# 알비온 랜덤 장판·Logic 연결 검토 결과

## G00. 실제 저장본 감사

기준 저장본은 Composition revision 550이다. `logic53` 랜덤 플레이어 탐색, `logic54` 알비온
하늘 순간이동, `logic58` 마리오 괴기스러운 인형 랜덤 스폰은 `triggerKind`가 없어 Server
mechanic trigger를 만들지 않는다. `summon4`도 이름 전용 정의다. 기존 `logic44`의
ALBION_BLUE_CIRCLE 구현은 있었지만 어떤 Pattern에도 배치되지 않았다.

P39의 `presentation.2`는 합본 Effect를 4535ms부터 1053ms만 재생했다. 폭발은 Effect의
2초부터 시작하므로 해당 occurrence가 먼저 끝나 정리된다. P34의 큰 인형은 World 트랙에서
3138ms부터 고정 위치 (4.555, 1.3176, 942.9967)에 생성되며 화염 V1은 실제 World motion에
연결돼 있다. 인형·화염의 표시 연결과 랜덤 스폰 미구현을 구분한다.

## G01. 소스에 구현한 알비온 계약

기존 typed Logic에 nav 추가 개수·반경·지면 높이 허용차·최소 간격·한 명 무작위 선택을
optional 필드로 연결했다. Client parse/validate/serialize와 Apply Values, Python projector,
GameplayBalance publisher, Server bootstrap reader와 실제 GameRoom 생성이 같은 계약을
사용한다. 기존 필드 생략 시 플레이어별·원주 생성이 유지된다. 새 bootstrap 행은 25필드이며
이전 16/17/20필드 reader 호환을 보존한다.

무작위 한 명은 Server가 준비된 생존 플레이어 중 선택하며 Mario·사망·낙하는 제외한다.
기존 발탄 `Resolve_ArenaRandomVolleyOrigins`에 무피해 효과의 명시 간격 입력을 추가했다.
가짜 damage hit이나 새 Shared/Client runtime을 만들지 않았다. 대상과 nav 보충 지점이 모두
성공한 뒤 기존 CombatObject transaction으로 commit하고, 실패 시 기존 객체·ID를 보존한다.
모든 위치는 생성 순간 고정되며 지속 추적하지 않는다. 기존 합본 Effect의 예고 2초·폭발·tail과
7초 수명 및 무피해 계약을 사용한다.

## G02. 검증된 저장 변경안과 실제 적용 경계

`out/AlbionRandom20260914/KoukuSaydonComposition.candidate.json`에 최신 저장552 기반 revision553 변경안을
준비했다. logic44를 nav 랜덤5·반경14m·지면 허용차1m·간격3.2m·선택 플레이어1명으로 설정하고,
P39.logic.1의 4534ms 시점에 연결한다. 직접 재생하던 P39.presentation.2 한 행만 제거한다.
다른 animation·Logic·부채꼴·World·사용자 TRS는 보존한다. 사용자가 직접 고친 MAP Fixed position도
그대로 보존한다. 근거는 `candidate_changes.json`이며 초기550/551은 snapshots 아래 보관했다.

사용자는 실행 중 편집을 유지하고 소스 검증까지만 진행하도록 지시했다. 따라서 이 변경안을
Composition 정본에 설치하지 않았고 실제 runtime 게시·제품 빌드·프로세스 재시작도 하지 않았다.
나중에 반영할 때는 이 파일로 전체 덮어쓰지 않고 최신 저장본을 다시 읽어 해당 항목만 적용한다.

## G03. 실행한 검증

- 실제 Gate3 nav 반경14m 안 9845개 표본 중8026개가 walkable·높이차1m를 만족했다. 별도
  uniform-disk 시드10000회에서5점/간격3.2m 확보 실패0회다. 이 수치는 제품 PRNG 실행 증거와 별개다.
- 기존 projector의 알비온 집중 회귀3건 통과. candidate가 Product P39의 유일한
  ALBION_BLUE_CIRCLE trigger로 투영되고 count5/선택1/4534ms/7000ms를 보존했다.
  GameplayBalance의 실제 Validate에 out overlay를 전달한 검증도 성공했다. runtime 게시는 아니다.
- Client 변경2TU와 실제 codec console 링크가 MSVC14.44.35207/SDK10.0.26100.0에서 성공했다.
  정본550·candidate551 각각47 checks 통과: legacy/5+1/선택 모드 roundtrip, 범위·타입·소유권 거절,
  실패 시 last-good 보존. 입력 파일 hash 전후 동일. `client/receipt.json`을 따른다.
- Server 변경4TU 컴파일 후 일관된 private snapshot의 Server85TU+Shared8TU를 out에 격리
  컴파일·링크했다. 실제 `--kouku-support-surface-contract-test`는33 PASS, failures0이다.
  5+1·nav 높이/간격·대상 제외·생성 후 고정·결정적 재현·snapshot 시계·rollback·만료/취소를 확인했다.
  `server/validation.json`, `server/support-surface-test.log`를 따른다.
- 초기 광범위 `-k albion`에는 기존 rootmotion 기대값 실패1건이 포함됐다. HEAD29ad2df5의
  변경 전 projector에서도 기대6.556622863264095/실제13.678813305200356로 동일 재현했다.
  이번 Logic·nav 변경으로 이 관계없는 animation 검사를 수정하지 않았다.

Client/UI 실행·조작·캡처·시각 PASS는 수행하지 않았다. 기존 컴파일 인코딩 경고는 보존했다.

## G04. Reset 오른쪽 Play Pattern

후속 요청으로 Workbench Pattern/Parent/Bundle transport의 Reset 오른쪽에 서버용
`Play Pattern`을 추가했다. 기존 Complete Play(Server)도 공통 `Request_SelectedServerPlay`를
사용한다. 현재 타임라인 owner의 stable ID를 기존 MainApp → BossTool → audition service로
전달하며 Collider/Logic은 Server에서 실행한다. 별도 Sequence workspace는 제외한다.

클릭 시 dirty·진행 중 게시·stale source·중복 요청·선택을 확인하고 실제 저장 문서와 draft의
동일성, 기존 Product reader의 게시 revision과 실행 가능한 Pattern/Bundle을 대조한다.
실패는 상태에 이유를 남기며 자동 Save/Publish·local collider simulation을 만들지 않는다.
버튼 tooltip은 실제 대상 이름과 stable ID를 보여준다. Resources의 Pattern 선택은 Append
원본이며 타임라인 owner를 바꾸지 않는다. 일반 local 버튼은 `Play Preview`로 표시한다.

최종 Workbench TU는 같은14.44 toolset/SDK26100 격리 컴파일 exit0이다.
`client/workbench-serverplay-compile.log`, `client/serverplay-source-checks.json`의13개
소스 연결 확인을 남겼다. 실제 UI 클릭·네트워크 요청 실행 검증은 하지 않았으며 제품 EXE도
갱신하지 않았다. 따라서 실행 중인 Client에는 아직 새 버튼이 없다.

## G05. 1페이즈 재생과 네 후보 요청의 현재 경계

사용자가 Complete Play(Server)/F1에서 재생했다고 확인했다. 저장/게시550의 P34
`세이튼_마리오_1페이즈`는4650ms에서 P38 무지개댄스·P39 알비온·P43 백스텝 불뿜기를
무작위 순서로 모두 재생하고, 실제3개 완료 후 기존 Success의 P33으로 이어진다.
현재 Server의 선택은 후보를 shuffle 후 completionCount만큼 선택하므로4후보중3개도
같은 경로로 가능하다. 솔로 Mario 진입 시 필요한 복귀 완료 대기는 기존 계약을 유지한다.

별도 P37 `3관문_쿠크세이튼_마리오`는 고정 Parent다. 현재 게시 inventory는 동적 follow-up을
가진 P34를 자식으로 포함한 이유로 P37을 unavailable 처리한다. P34를 P37로 바꾸는 ID
dispatch는 없다. 같은 이름의 Parent timeline P56은 비어 있는15초 타임라인이며 P34와 다른
stable ID다. 실제 실행 로그의 선택 ID는 확보하지 않았으므로 사용자가 어떤 ID를 눌렀는지
확정하지 않았다. 현재 P34 연결은 사용자가 관찰한 전체 진행과 일치한다.

요청한 네 번째 후보인 P40 `쿠크세이튼_십자화염폭발`과 P45 분신 버전은 저장본에서stage0이라
제품 재생 불가다. 미저장 편집 여부/원본 기반 실행안 준비에 대한 질문은 미응답이며,
사용자가 편집 중인 정본을 보존한다. 따라서4후보 연결은 아직 반영하지 않았다. 빈 패턴을
참조해 정상 P34까지 unavailable로 만들거나 임의 animation을 넣지 않는다. 최신 저장본의
실행 가능한 P40을 확인한 뒤 logic55.patternIds를 P38/P39/P40/P43, completionCount3으로
바꾸면 기존 2페이즈 Success와 Server 알고리즘을 그대로 사용한다.

## G06. 전체 이펙트 미표시와 World Fixed position

게시550의 P39.presentation.2/.3/.4는 WORLD·빈 worldId였다. 실제 Product Read_Occurrence가
거절하고 Reload_Product가 staged 전체 교체 전에 실패한다. 첫 로드라면 presentation이 없고,
이전 로드가 있더라도 새 Server run의 source revision과 다르면 샘플링하지 않는다. 서버 animation과
Logic은 별개이므로 패턴 동작만 보일 수 있다. Product 오류는 OutputDebugString 경로이며 현재
JSONL에는 실제 예외가 기록되지 않아 사용자가 누른 프레임의 exception 발생 시점은 확정하지 않았다.

P34 작은 오망성 boss.pentagram.full.restore, 큰 오망성 center.full.restore, P38
rainbow.grid.full.restore는 catalog와 저장 occurrence가 존재한다. P43 백스텝 불뿜기는
저장본·Product의 Effect occurrence가0개다. 후자는 전체 로드가 고쳐져도 별도 Effect 연결이 필요하다.

사용자가 의도한 World는 고정 맵 좌표다. UI의 World → Fixed position은 MAP·followBoss=false로
저장하며 특정 오브젝트 부착인 WORLD와 다르다. 사용자가 직접 저장한552에서 P39 세 행은
MAP·고정위치(-2.157323122,1.317625642,942.926086426)로 교정됐다. 에이전트가 정본을 바꾸지
않았다. 확인 당시 게시본은 여전히550이므로 Complete Play/F1은 최신 Publish가 필요하다.
새553 후보는 이 사용자 위치를 보존하며, 두 부채꼴의 이펙트 내부는 로컬 원점임을 별도 확인했다.

Client codec·Python publisher에 WORLD의 유효 worldId 검증을 추가하고 Product 오류에
정확한 occurrence ID를 남겼다. 잘못된 자식 패턴을 완료 후보로 참조할 때도 원래 로드 사유를
보존하므로 generic Completion 오류가 P39의 원인을 가리지 않는다. 자동 MAP 치환이나 검증 완화는 없다. projector의18개 누락/빈값/
없는ID 조건은 변경 전 실패 재현 후 관련4개 테스트가 통과했다. 최신553 candidate의 실제 투영은
P39 typed5+1/4534ms/7000ms와43개 제품 패턴을 유지했다.
553 overlay로 다시 실행한 GameplayBalance Validate는 정본552와 게시550의 freshness 불일치를
먼저 검출해 종료했다(`publisher-candidate-553.log`). 게시하거나 freshness를 우회하지 않았다.
따라서 G03의 이전551 전체 Validate 통과와 최신553 projector 성공을 구분한다.

최종 Client CompositionDocument·PresentationPlayer 두 TU 격리 컴파일과 두 native console 링크가
성공했다. 최신552와 후보553 각각 기존 알비온47 checks, 새 anchor46 checks가 통과했다. 원래550
오류와 그 P39 행을 복사한 fixture의 거절·정확한 원인, 정상 BOSS/MAP/WORLD, Parse/Reload/Save
실패 시 last-good·파일·generation 보존, out의 실제 atomic save/reopen을 확인했다. 최종 입력
hash가 모두 유지됐다. `client/final_validation/receipt.json`과4개 run log를 따른다. 제품 EXE를
교체하거나 Client/UI를 실행·조작하지 않았다.

## G07. 작은 오망성 폭발의 설치 상태

작은 오망성의 기존 native2811/2812 alpha1 수정은 현재 실행 Client보다 먼저 생성된 설치
Mesh/Particle CSO에도 포함돼 있다. 같은 실제 codec packet·DDS 입력으로 기존 native compare
probe를 다시 실행한38개 draw 수치가 이전 수정 후 JSON과 완전히 일치했다. 2811의1.6초 RGBsum은
3.67375944다. 이는 해당 shader와 입력의 수치 검증이며 최종 게임 화면 PASS가 아니다.

기존 explosion 자산·shader를 다시 변경하거나 대체하지 않았다. 여러 이펙트가 함께 빠지는 Product
로드 실패와 분리해 최신 게시 후 사용자 관찰을 먼저 확인한다. 그래도 해당 폭발만 남지 않으면
사용자가 허용한 같은 원형 크기의 기존 폭발 교체를 다음 판단으로 유지한다. 증거는
`pentagram_audit/receipt.json`, `installed-shader-numeric.json`, `fan-local-origin.json`이다.

## G08. 무지개댄스 중앙이동 검토

P38 logic47 중앙이동은 triggerKind 없는 TRIGGER다. 배치는1990ms·674ms지만 projector가
실행 항목을 만들지 않아 Product mechanicTriggers는 비어 있다. 현재 중앙으로 이동시키지 않는다.

발탄 RETURN_TO_ARENA_CENTER는 기존 Server nav 중앙 해석 후 즉시 위치를 옮기는 기능이며
걷는 이동이 아니다. 중앙은 boss spawn의 nav 지점이며 실패 시 같은 층1.5m·거리8m 안에서 찾는다.
현재 Kouku stage action으로 그대로 붙이면 validation과1990ms 타이밍이 달라지므로 최소 연결은
typed Logic을 추가해 같은 resolver·snapshot을 소비하는 방식이다. 순간이동과 시간에 따른 보행 중
어느 동작을 원하는지는 현재 저장674ms만으로 결정하지 않았다. root motion과 이동 기준을 함께
연결해야 하므로 이번 검토에서 이름만 보고 임의 이동을 넣지 않았다.

## G09. 쇼타임 양손 기관총 교체 검토

사용자는 왼손/오른손 무기를 서로 바꾸는 것이 아니라 기존 무기를 숨기고 양손 기관총으로
교체하는 동작이라고 확인했다. 실제 제품은 P35 세이튼_쇼타임이며 동명 P32는 stage0이라 제외된다.
P35 World16은 왼손 b_wp_2에0..50358ms, World17은 오른손 b_wp_1에0..50359ms 연결됐다.
authoring과 설치된 objectResource/template/instance6개가 같고 Encounter550에도 두 cue가 있다.

WorldSequencePlayer는 실제 BODY bone matrix와 root를 합성한다. 제품은 살아 있는 replicated
G3 보스를 사용하고 Model View/Bundle preview도 해당 preview actor BODY로 anchor를 바꾼다.
총 소켓 연결이 제품에 없는 문제는 아니다.

기본 WP_MN_RPCT_05는 CNpc의 별도 m_pWeaponModelCom이다. Npc.cpp Render는 포인터가
있으면 항상 그리며, ClientReplication의 Kouku snapshot 소비자는 body action/animation만
갱신한다. 현재 총 cue는 두 오브젝트를 추가할 뿐 기본 무기의 숨김·복구 상태가 없다. 따라서
사용자가 요청한 무기 교체는 실제로 미구현이다. 기존 양손 총 경로를 유지하고 해당 actor의
기본 장착무기를 총 사용 구간에 숨긴 뒤 종료·취소·실패에 복구하는 presentation 계약이 필요하다.
Pattern ID·표시명·socket 이름 하드코딩으로 전체 보스를 숨기는 방식은 사용하지 않는다.

쇼타임 Logic45의180도 회전과 Logic46의 플레이어 추적도 현재 triggerKind 없는 행이며
Product mechanicTriggers에 포함되지 않는다. 이번 요청은 읽기 전용 검토로 수행했으며
무기 교체 코드·데이터·제품 바이너리·실행 상태를 변경하지 않았다.

## G10. 쇼타임 무기 교체 소스 수정

사용자가 즉시 수정을 요청해 G09의 검토 뒤 실제 소스에 연결했다. 기존 양손 총의 정확한
objectId에만 적용하며 JSON·packet·Pattern 이름·clip 분기는 추가하지 않았다. World sampler가
실제로 resolve한 BODY model과 visible 총 clone을 등록하고, NPC와 Model View 무기는 렌더
시 같은 body에 표시 중인 총이 하나라도 있으면 기본 무기를 그리지 않는다. Model View는
일반·추가 pass와 shadow가 같은 조건을 사용하며, 기존 무기의 pose·socket·재질은 유지한다.

등록은 해당 World object 재생 수명이 소유한다. Stop·실패·Clear·pool 반환 전에 해제하고,
Hide·anchor 소실도 실제 총 visibility를 통해 즉시 반영한다. 한쪽 총이 먼저 종료돼도 다른
쪽이 보이면 숨김을 유지하고, 같은 pooled clone을 다른 보스에 재사용해도 이전 보스를 숨기지 않는다.

전체 Git 이력에서 기존 쇼타임 교체 코드의 삭제는 확인되지 않았다. 2e2f62be에서 Preview
기본 무기 표시가 대형 세이튼 전용에서 다른 세이튼까지 확대된 것은 확인했다. 이전에는 기본
무기가 없어 양손 총만 보였을 가능성은 있으나 이는 소스 이력에 대한 추론이며 사용자 화면을
관찰한 증거는 아니다. 현재 실행 Client11224·Server53512는 그대로이며 제품 EXE는 교체하지 않았다.

검증 완료: 변경·직접 영향9TU와 probe용 원본3TU, 총12TU가 MSVC14.44/SDK26100 격리 컴파일에
성공했다. 실제 Track/Is 함수의 원본 prefix를 byte 그대로 사용한 native probe와 원본
WorldSequenceObject·Part_Body 객체로30 checks가 통과했다. 양손 중 한쪽 종료·Hide·등록 해제·
owner 변경·weak 수명·pool 재사용·container Clear와 실제 Render_Pass/Render_Shadow의 숨김
조건을 확인했다. body는 역참조하지 않는 identity token이므로 실제 골격·모델 표시 검증이 아니다.
원본 함수 재구현이나 제품/UI/GPU 실행은 없었고 최종 입력 hash가 유지됐다. 근거는
`out/ShowtimeWeaponSwap20260914/receipt.json`, `probe.log`, `quick_compile/receipt.json`이다.
`git diff --check`도 통과했다. 현재 완료 범위는 소스·격리 컴파일·계약 검증이며 제품 재빌드와
사용자 화면 확인은 남아 있다.
