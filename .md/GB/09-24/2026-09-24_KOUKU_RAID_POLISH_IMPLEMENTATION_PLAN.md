# 쿠크 전투 진입·피격·패턴 정리 구현 계획

## G00. 기준과 요청 범위

기준은 `codex/kouku-timeline-local-preview`의 현재 저장본 revision2253과 앞선
KOUKU_DAMAGE_EFFECT_SOUND_EDITING 결과다. 기존 사용자 저작·게시 변경, 35줄 ALT_V,
source-in, 공통 피해 Trigger, Play 분리를 보존한다. 최초 Composition 백업은
`out/KoukuRaidPolish20260924/Composition.before.json`이다.

사용자 요청은 연출→전투 전환, G3 색 보정, 피격 넘어짐, Collider 피킹과 표시,
돌진/파1빨2/카운터 투명도, 노란 십자 피해, 바람방구 방향, 빙고 재생, 카드비 병정 AI와
1~4배 크기 공유, 회전카드 보라 잔상, 공분열5회와 착탄 피해, 비둘기 폭발 피해,
G3/빙고 레이저, 망치/미로 이동2초 전 바닥 문양, 무력화 시 칼날 종료와 Mario 즉사칼날1개다.
아래 구현 항목은 현재 코드·데이터로 원인과 경계를 확인한 내용이며 추가 조사로 확정한
항목을 같은 계획서에 이어 기록한다. Client/UI 실행과 최종 화면 판정은 사용자가 한다.

## G01. 전투 진입의 준비 상태 인계

gate light/profile은 PREPARING에서 준비되지만 cinematic 시작에 활성화하지 않아 종료 시
이전 profile 복원과 전투 profile 적용이 연속 발생한다. 준비한 light/profile을
`Begin_ServerRaidCinematicPresentation`에서 활성화하고 Sequence preview 시작 이전에
같은 profile을 baseline으로 잡는다. 명시적인 cinematic profile cue는 그대로 재생한다.
Commit은 준비한 light cache를 인계하며 abort만 이전 조명/profile을 복원한다.
HUD와 입력의 pending 해제는 gate commit 성공 후로 맞춘다.

소유 파일은 `Level_KakulSaydonArena.h/.cpp/_WorldObjects.cpp`와 `MainApp.cpp`의 레이드
준비/시작/종료 소비자다. Gate Object Play/Seek 비용은 실제 측정과 구분하며 조명만으로
모든 hitch가 해결됐다고 단정하지 않는다. 실패·취소·반복 입장 시 이전 상태 보존을 검사한다.

## G02. 피격 상태와 카드비 병정

`Character::Commit_Locomotion`의 지연 idle 전환이 KNOCKDOWN을 덮지 않게 한다.
Server의 실제 공중 강제이동 여부를 PLAYER_SNAPSHOT으로 전달하여 공중→착지→넘어짐
전환이 clip 길이 추측 대신 Server 상태를 따른다. protocol110의 optional이 아닌 typed
bool은 양방향 codec·두 snapshot 생산 경로·Client replication·Character까지 함께 연결한다.
일반 이동·teleport는 피격으로 바꾸지 않는다. 실제 피해 push에 착지 후 최소1초 회복을
적용하며 더 긴 authored down 시간을 보존하고 공중 기상기는 거절한다.

카드비 병정은 공유된 미로 profile의 이동0.01/추적0.01/공격거리0.01 때문에 사실상
정지한다. 해당 카드병정 profile에 기존 일반병 전투 수치를 적용하고 미로 target으로
등록된 entity는 생성 직후부터 generic MonsterBrain을 건너뛰어 미로 authority를 보존한다.
추적·공격·사망·미로 target 소유를 실제 Server fixture에서 확인한다.

## G03. 잔상과 투명 표현

회전카드가 실제 소비하는 heart/clover/diamond/spade4개 asset에 원본 보라 alpha 잔상
emitter_0을 복구한다. 참조되지 않는 과거 common throw만 수정하는 경로를 반복하지 않는다.
파1빨2 safezone.blue와 counter.ring의 실제 native RGB/alpha 소비를 확인하여 해당 asset만
조절한다. Saydon dash의 MODEL 잔상은 reflection이 없는 별도 `CSkeletalAfterimage` 경로이므로
해당 NPC 설정의 RGB/alpha를 조절한다. 전체 Effect 재질/조명 배율은 변경하지 않는다.

## G04. Mario 무력화 성공과 칼날

현재 `Apply_KoukuLogicOutput`은 Counter 성공만 World owner를 중단한다. STAGGER_WINDOW의
성공에 별도 내부 완료 사유를 전달해 같은 interruption 소비자로 연결한다. 자연 종료의
Effect/World tail 보존은 유지하며 무력화 성공 시 해당 member의 일반·즉사 칼날, pending
World/support/접촉 판정이 즉시 종료되는지 확인한다. 다른 member와 다음 groggy는 보존한다.

Mario2 전용 즉사칼날 template은 count8과8개 emission이다. 기존 boss→아이언메이든
모션·회전·수명은 유지하고 발사점을 보스 기준 한 중앙 emission으로 바꾼다. XZ 이동의
끝점이 해당 두 authored 위치와 일치하는지 실제 transform 계산으로 확인한다.

## G05. 저장·게시·검증

대형 JSON은 stable ID와 변경 필드 기준으로 최신 저장본에 병합한다. writer lock, hash/revision
재확인, 백업, 원자 교체와 실패 시 자기 변경 rollback을 유지한다. 변경된 원본·publisher·
공식 생성 runtime을 함께 검증한다. 실행 중 미저장 draft와 게시 파일을 혼동하지 않는다.

새 C++ 파일을 만드는 대신 기존 typed 경로와 fixture를 확장한다. 필요 시 project/filter
등록을 같은 단위로 처리한다. Shared snapshot 변경 후 Shared→Server→Client 정상 증분
Debug/Release Build와 관련 codec/Server/authoring 집중 검증을 수행한다. HLSL을 바꾸면
정상 CSO와 Engine SDK 배포까지 확인한다. 변경하지 않은 광역 하네스는 완료 조건이 아니다.

## G06. 저작 판정과 재생 순서

카드비는 기존 Server randomVolley의 uniformScale을 장판/카드/피해에 함께 적용하는 경로를
유지하고 범위만1~4로 조정한다. 개수는 기존500ms간격을 보존한다. 바람방구는 BOX의
회전만180도 돌리면 같은 도형이 되므로 BOSS local XZ offset도 함께 반전한다.

P79/119/122 노란 십자는 native floor mesh 실측 중심 localZ=-1, 장축반길이5.27533m,
짧은축1.27536m를 사용하는 두 BOX를 같은 공통 Trigger창에 연결한다. 실제 FX 시작+34ms에
100ms 접촉창을 열며 두 BOX가 겹쳐도 같은 창의 player 피해는 한 번이다. 후속 원/안쪽링/
바깥링은 현재 저작 MAP 위치와 impact시점에 기존 판정을 맞추며 원본 hollow ring을 보존한다.

P83 공 발사6개 중 가장 늦은6번째만 제거한다. 낙하/경고에는 피해를 달지 않고3개 실제
착탄의 첫 visible tick에 피해창을 둔다. 원본 warning의 반경1m,2.5~4m,5.5~7m를 보존해
안전한 두 간격을 넓은 폭발/debris AABB로 덮지 않는다.

P82 비둘기는 native Sk02_3 ring이 비행3.04167초 뒤 생성되는 것을 확인했다. 각 FX의
local 폭발중심 X=-2.4/-1.6/-.8/0, Z=-10.1859를 occurrence yaw/scale로 변환하고 원형
ring 최대직경4.67778m에서 반올림한 radius2.35m를 쓴다.4개 원형은 같은 Trigger창으로
피해를 중복하지 않는다. 이는 명시적인 gameplay 크기이며 GPU texture coverage나 원본
SkillEffect 피해치수라고 주장하지 않는다. 각 비둘기의 비행 중에는 피해가 없다.

G3 P62와 Bingo P123 레이저는 기존 animation/effect 정의를 연결하고 실제 beam발사창의
판정을 추가한다. G3 flow 반복구간과 Bingo P96의 초기/반복 child sequence에 넣고 기존
stable loop 시작ID를 보존한다. Parent duration과 지속 logic을 함께 늘린다.

## G07. 바닥 예고와 G3 색 보정

G3 최종화면 색을 바꾸는 LUT01을 compare profile 및 g1.base/g3.dark/source-rendering의
G3 region48에서 해제한다. 조명·노출·안개·tone 입력은 유지한다. 망치 warning은 object가
3000ms에 나타날 때 이미 종료되던 범위를 실제 이동4400ms 시작까지 연장한다. 미로는
기존 snapshot march clock과 World sequence를 이용해 이동2초 전 warning을 보여 주고,
이동/contact 시작은 같은 publisher 투영 결과를 사용한다. 별도 Client 판정은 만들지 않는다.

## G08. Load 이후 몸체 표시와 막힌 목적지

기존 CCollider/Bounding/Renderer Debug draw를 Debug/Release 공통 컴파일해 F1 Load
KoukuSaydon Inventory에서 플레이어·세이튼 몸체 표시를 켠다. 전역 visibility snapshot과
기존 Character/CNpc setter를 사용하며 새 renderer/Client gameplay collider 경로는 없다.
출력 플래그는 기존 spawn과 이후 spawn이 모두 소비하고 F1 checkbox로 해제할 수 있다.
Engine Component virtual까지 공통 ABI로 맞추므로 Engine/SDK→Client 정상 Build를 수행한다.

우클릭 목적지가 dynamic body 안인 경우 nav path는 계속 성공하지만 body sweep은 도달을
영원히 막는 원인을 확인했다. 이번 이동에서 실제로 접촉한 body가 최종 목적지를 가릴 때만
Server에서 도착 처리한다. 미접촉/다른 층/보스 너머 이동과 새 우클릭은 유지한다.

## G09. 삽입된 Parent child의 게시 순서

게시 후 실제 Server admission에서 P96 신규 child44/45가 시간보다 stable ID 순서로
정렬되어 전체 catalog가 거부되는 결함을 확인했다. Product parent entries는 시간순이나
공통 bootstrap row sorter가 이를 다시 ID순으로 바꿨다. PATTERNPARENTCHILD만
encounter/parent/startMs/occurrenceId 순으로 정렬하고 다른 indexed row의 순서를 보존한다.
기존 ID와 loop 참조를 다시 번호 매기지 않는다. 실제 ID순서와 시작 시간이 어긋나는 fixture를
공통 row-order 검사에 넣고 공식 재게시 후 설치 Server의 catalog/빙고/레이드로 확인한다.
