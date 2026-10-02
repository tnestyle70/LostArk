# Guide AI Tool · 가이드 차원술사 구현 결과

작성일: 2026-09-27
갱신일: 2026-10-01

> 현재 동작은 아래 G06이 정본이다. G00~G05는 2026-09-27 당시의 구현·검증 기록이다.
> 그중 파티 생성/별도 companion 행/레이드 동행/가이드 동반 전송/마지막 인간 퇴장 시
> 가이드 삭제 기획은 사용자 정정에 따라 폐기했다. 당시 protocol·게시 revision·화면 절차도
> 현재 버전의 완료 증거로 사용하지 않는다.

## G00. 완료 범위

사용자가 확정한 **사람 4명 + 가이드 별도 1명** 계약으로 편집 도구와 서버 실행을 연결했다.
기존 미커밋 변경과 같은 작업 사본을 사용했으며 다른 기능의 변경은 정리하거나 되돌리지 않았다.

- Debug F1 → Guide AI, World Level Tool → Guide AI 진입.
- 베른 기본 카테고리, 시작 Position/Rotation Y, 현재 Bern spawn 가져오기.
- 대사 선택·Rewrite·Apply/Cancel, segment 추가, 저장·로드와 참조 중 삭제 차단.
- 별도 Guide OBB, NPC stable anchor, 게시 boss pattern, 도움 command 트리거 행.
- 순서형 슬롯 콤보와 정확한 도움 별칭, 명령별 독립 cooldown, 실행 중 rescue 콤보 교체 대기.
- Combat Detail 별도 창, 상황별 가중치와 거리/위협/HP/생존 우선/판단 이유/적용 revision 관측.
- Guide 전용 Save/Publish와 Client/Server 동일 revision runtime.
- 서버 초대 자동 수락, 별도 companion roster, 추종·스퀘어홀 도착·용 탑승/비행·부활.
- 4인/솔로 NPC 레이드 입장·귀환에 companion을 같은 전송 transaction으로 포함.
- boss target 및 쿠크 기믹 배정 제외, 공간 콜라이더 damage/CC/갈고리/즉사 접촉 유지.

## G01. 실제 데이터와 소비자

저작 정본은 `Data/Guide/GuideCatalog.json`,
`Data/Guide/DimensionMaster/{Placement,Prompts,Triggers,Combat}.json`이다.
`Tools/GuidePipeline/Publish-Guide.ps1`가 두 `Bin/DataFiles/Guide/Guide.runtime.json`을 생성한다.
최종 게시 revision은 **128350564805445**, 대사 4개/트리거 4개/콤보 2개다.

초기 제련 Box는 `npc.bern.schmidt`, 레이드 Box는 `npc.bern.beda.guide`에 연결했다.
진짜 세이튼 설명은 `KAKULSAYDON_G1_PATTERN_2`의 실제 시작을 소비한다.

`CGuideAIDocument`가 baseline/draft를 보존하고 비동기 publisher 결과를 Poll한다.
Load는 parse 후 strict Validate와 source freshness가 모두 성공한 후보만 교체한다.
Save는 stable ID/필드 단위로 최신 디스크 저장본에 병합한다. Publish는 저장본만 게시한다.

`CGuideCatalog`는 게시본을 서버 시작 시 읽고 schema, ID, UTF-8, 시간/가중치/참조와
실제 skill binding을 검증한다. `GameRoom_Guide`는 실제 player executor를 통해 이동·스킬을
요청한다. `GameRoom_GuideThreat`는 기존 contact geometry와 실제 발생 clock을 읽는다.
Client는 기존 CCharacter/party/chat/bubble 표현을 사용한다. Guide 말풍선은 asset 로딩 중
대사를 보존하고 actor 준비 후 표시 수명을 시작한다.

Shared protocol은 **116**이다. actor control kind, optional companion roster,
Guide prompt event, 실제 decision state를 모두 codec으로 전달한다.
Server와 Client는 같은 protocol 빌드를 사용해야 한다.

## G02. 의사결정과 기믹 정책

기본 도움 명령은 `도움!`, `도와줘`, `도와줘!`이고 기본 콤보는
`W → A → S → D → F → V → T → ALT_V`다.
`살려줘`, `살려줘!`는 `ALT_V → V → W → A`를 요청한다.
`그만`, `그만!`, `멈춰`는 보조를 중단한다. ALT_V에 임의의 회복 효과는 추가하지 않았다.

실행기는 실제 스킬 승인과 종료를 기다린다. cooldown/resource/status/range 실패는
대기 또는 접근으로 처리하고 단계 대기/전체 deadline이 지나면 멈춘다.
도움 명령별 cooldown을 분리하여 일반 도움 직후 긴급 요청을 받을 수 있게 했다.

FOLLOW/ASSIST의 세 행동 점수를 비교하고 hold/margin을 적용한다.
즉사 또는 위험 상태에서 HP 임계치 미만이면 회피를 우선한다.
회피 후보는 navigation/collision과 실제 경로 중간 위험을 확인한다.
발사체, boss stage shape/schedule, 쿠크 Logic의 이동 collider/잔존 tail, 빙고 망치를 조회한다.

위험도는 알려진 공간 접촉의 계획 점수다. 미래 stage pose는 현재 확정 pose를 바탕으로
추정하므로 완벽한 회피나 실제 피해량 예측을 보장하지 않는다. 스킬 행동 잠금과 피격은
기존 실행기가 결정한다. 도구에 표시한 draft는 현재 Server 적용값과 분리한다.

Guide는 인간 입장 인원/공대장/투표/MVP, 카드·광기·춤·마리오·미로·빙고 머리 표식과
boss targeting에서 제외했다. 도움 피해의 counter/stagger/part/MVP 기여도 제외했다.
실제 공간 접촉의 damage/CC/즉사 결과는 유지한다. 인간이 미니게임 내부에 있으면
가이드는 다른 인간을 임시 추종하거나 바깥에서 기다린다. 마리오 timeout 벌칙,
솔로 귀환 인원, phase 2 철창 대상/배치 인원도 인간만 계산한다. phase 2의 인간 이동이
commit된 뒤 가이드는 추종 anchor 도착으로 이동한다.

## G03. 검증 증거

| 검증 | 결과 | 증거 |
|---|---|---|
| 전체 Debug product compile/deploy | PASS | `out/guide-ai-debug-complete.log`, `out/BuildPipeline/runs/20260927T035726673Z-debug-product.json` |
| NetworkProtocolHarness Debug | 1314 PASS, failures 0 | `out/guide-protocol-build.log`, `out/guide-protocol-contract.log` |
| Server `--guide-ai-contract-test` | 36 PASS, failures 0 | `out/guide-ai-contract-complete.log` |
| Server `--kouku-object-overlap-contract-test` | 822 PASS, failures 0 | `out/guide-overlap-contract.log` |
| Server `--vehicle-riding-contract-test` | 56 PASS, failures 0 | `out/guide-vehicle-regression.log` |
| Server `--character-admission-contract-test` | 71 PASS, failures 0 | `out/guide-admission-regression.log` |
| Guide persistence regression | 21 tests OK | `python Tools/GuidePipeline/test_guide_pipeline.py` |
| Guide Publish / CheckPublished | PASS, revision 128350564805445 | 두 runtime 파일 byte 일치 |
| JSON/XML | JSON 9개, project/filter XML 4개 parse PASS | 원본5 + runtime2 + UI2, Client/Server project/filter |
| `git diff --check` | exit 0 | `out/guide-diff-check.log` |

Guide 계약은 실제 자동 판단 루프의 skill 실행, 다른 rescue 명령 즉시 접수,
미니게임 진입 차단, 같은 용 탑승/3축 비행 명령, 초대4+1, 부활, revision 불일치 rollback,
4인/솔로 전송과 마지막 인간 이탈 시 제거를 확인했다. 실제 쿠크 게시 phase 2에서
2인+Guide의 포로 미배정, 4인+Guide의 인간 배치 슬롯 유지와 별도 추종,
1인+Guide의 실제 Mario 입장·솔로 귀환 필수 조건도 확인했다.

접촉 계약에는 같은 collider에서 인간/Guide 동일 피해, Guide 재진입 latch,
즉사 collider 사망, 미응답 기믹 verdict 제외, 카드/광기/춤 제외가 포함된다.
저장 회귀는 임시 sparse repository에서만 수행했으며 invalid schema/키/ID/스킬/UTF-8,
동시 무관 필드 병합, 같은 필드 충돌, source freshness, Save-only, canonical revision,
실제 두 번째 파일 잠금 실패와 Replace 이후 예외의 rollback을 확인했다.

빌드 중 확인한 신규 C++ 이름 충돌과 UTF-8 TU 옵션을 수정했다. 새 Mario 회귀 fixture의
bundle broadcast PatternIds 초기화 누락을 보완한 뒤 36개 전부를 재실행하여 통과했다.
기존 혼합 인코딩 헤더 및 DirectXTK PDB 경고는 남아 있으며 빌드 오류는 없다.
Release 전체 빌드, 실제 LAN 다인 접속, Client/아레나 화면 검증은 실행하지 않았다.

## G04. 적용과 사용자 화면 확인

1. 같은 빌드의 Server를 재시작하여 게시된 Guide revision을 읽는다.
2. Debug Client의 F1 → Guide AI를 열거나 World Level Tool의 Guide AI를 누른다.
3. 베른 시작 위치에서 가이드 차원술사를 우클릭 초대한다. 파티의 별도 행과 첫 인사 채팅/말풍선을 확인한다.
4. 제련/레이드 NPC 접근, 도움·긴급 명령, 스퀘어홀·용 비행·파티 레이드 이동을 실제 화면에서 확인한다.
5. 대사/Box/콤보/가중치를 편집해 Save한 후 Publish한다. 파일 게시와 실행 중 Server 적용은 별개다.

에이전트는 Client를 실행하거나 도구를 자동 Reload하지 않았다. 화면 배치·한글 표시·모델
표현·실제 체감 거리와 회피 동작은 사용자의 화면 판정이 남아 있다.

## G05. 전달 경계

AGENTS, CLAUDE, 팀 인터페이스 사용서에는 Guide public 계약만 반영했다.
중간 빌드/테스트 산출물, EXE/DLL, EngineSDK, 개인 규칙, 임시 Python cache는 소스 전달 대상이 아니다.
작업 시작부터 존재한 광범위한 미커밋 변경과 다른 세션 작업을 함께 stage/commit/push하지 않았다.


## G06. 서버 한 명의 안내 시작·종료와 베른 대기·복귀 (2026-10-01)

### G06-01. 실제 반영 범위와 이전 기획의 교체

서버의 shared Bern room에 처음 배치된 **가이드 차원술사 한 명**을 그대로 사용한다.
안내 시작은 사람마다 가이드를 만들지 않고, 기존 actor의 `PlayerId/NetEntityId`에
안내 사용자 한 명을 연결한다. 사람 파티를 내부적으로도 만들지 않고 roster에 가이드를
넣지 않는다. 이미 안내 중이면 다른 사용자는 제어권을 가져가거나 안내를 종료할 수 없다.

베른에서 가이드 우클릭 메뉴는 상태에 따라 `안내 시작`, 자기 안내의 `안내 종료`,
다른 사용자의 `안내 중`을 표시한다. 아직 서버 상태를 받지 못했으면 상태 확인 중으로
입력을 막는다. 시작·종료는 파티 초대와 분리된 `C2S_GUIDE_CONTROL`을 사용하며,
예전 Guide 대상 `C2S_PARTY_INVITE`는 안내나 숨은 파티를 만들지 않는다.

G00~G05의 4인+companion, 레이드에 가이드를 함께 입장시키는 전송, 파티 해산에 따른
가이드 제거는 현재 경로에서 삭제했다. 사람 4인의 기존 파티와 레이드 입장은 유지한다.
발탄·쿠크·콜로세움·마하라카 목적지에는 안내 가이드를 생성하지 않는다.

### G06-02. 서버의 소유자와 수명

`Server/Public/GameRoom.h`의 `m_PersonalGuides`는 이름과 달리 최대 한 항목만 가지며,
key는 안내 사용자의 `SESSION_ID`다. `GUIDE_RUNTIME`은 기존 가이드 actor ID,
현재 인간 anchor ID, Bern owner net ID, owner의 `weak_ptr<CClientSession>`, 대기·도보 접근
상태와 대사/콤보 예약을 가진다. actor 자체는 `m_Players`에 한 번 배치된 항목이다.
프로세스 안의 안내 상태이며 재실행 후 영구 복구하는 저장 기능은 아니다.

`GameRoom_Guide.cpp`의 실제 책임은 다음과 같다.

| 함수 | 현재 책임 |
|---|---|
| `Initialize_Guide` | 게시 정의와 실제 슬롯 binding을 검증하고 Bern에만 한 명 배치한다. Colosseum은 용병 콤보 조회를 위한 catalog 로드·검증만 하고 actor는 만들지 않는다. |
| `Handle_GuideControl` | 요청 순서, 현재 인간 session, 정확한 guide ID를 확인해 시작·종료를 적용한다. STOP은 현재 owner만 허용한다. |
| `Start_Guide` | 10m 안에서 현재 actor를 owner에게 연결하고 상태를 먼저 방송한 뒤 첫 인사를 예약한다. 다른 owner가 있으면 기존 소유자를 보존한다. |
| `Suspend_PersonalGuide` | 성공한 세계 이탈 때 동작·이동·대사 예약을 정리하고 같은 Bern 위치에서 대기시킨다. 전송 commit 안에서 packet을 보내지 않는다. |
| `Resume_PersonalGuide` | 성공한 Bern 입장 뒤 같은 session의 새 player ID로 연결하고 도보 접근과 해당 귀환 대사를 예약한다. |
| `Remove_Guide` | 함수 이름과 달리 actor를 삭제하지 않는다. 안내 상태를 비우고 같은 actor를 idle로 남긴다. 종료 뒤 늦은 귀환 신호는 재개하지 못한다. |
| `Update_Guides` | owner의 살아 있는 연결을 확인하고 기존 이동·스킬 실행기를 사용한다. owner가 다른 방에 있어도 weak session으로 연결 종료를 감지해 안내만 해제한다. |

마지막 인간이 Bern에서 나가도 대기 actor는 남는다. Bern은 기존 레이드 빈 방 초기화의
대상이 아니며, 파티 정리에서도 안내를 삭제하지 않는다. `Update_Players`는 Bern의
비활성 `GUIDE_AI`만 정지시키므로 대기 중의 잔여 이동이나 비행 속도로 위치가 흐르지 않는다.

Client Level이 바뀌면 안내 요청 순번이 다시 1에서 시작하므로 `Leave`에서 해당 session의
control 순번만 지운다. 안내 owner 예약은 유지한다. 따라서 복귀한 새 Bern Level의
`STOP(seq=1)`도 받아들인다. 같은 Level 안의 중복·과거 요청은 계속 거절한다.
actor의 내부 이동·스킬 명령 순번은 재시작 때 마지막 승인값부터 이어받는다.
대사 event sequence는 room 멤버가 소유하여 종료·새 owner 시작에도 증가를 이어간다.

### G06-03. 항구와 세계 이동, 정상 복귀의 도보 접근

배 승선이 서버에서 승인되어 `bShipDockValid`가 설정되면 가이드는 당시 Bern 위치에서
기다리고 배를 복제해 타지 않는다. 같은 Bern에서 하선하거나 다른 세계에서 Bern으로
귀환하면 기존 navigation과 `Execute_PlayerMove`로 owner에게 접근한다.
용 `9523`의 탑승·이륙·3축 비행 추종은 기존 vehicle 승인·이동 경로를 유지한다.

사람이 레이드·콜로세움·마하라카로 이동할 때 안내 actor는 Bern에 남는다.
파티 전송은 outbound 예약과 사람 입장이 성공한 뒤에만 source 안내를 대기시킨다.
실제 outbound capacity 거절에서는 사람 파티, guide identity·위치·예약 대사가 보존된다.
복귀는 `Join` 또는 `Transfer_PartyTo`의 입장 commit 뒤에만 재개한다.
일반 세계 전송의 출발 world는 서버 내부 `ROOM_COMMAND::eEntrySourceWorldId`로 전달하며,
Client 입장 packet이 임의로 귀환 이벤트를 만드는 경로는 없다.

정상 복귀 접근에서는 원거리 복구 teleport를 사용하지 않는다. 가이드 자신의 이동 목표만
설정하고 인간의 위치·이동 목표는 바꾸지 않는다. 복귀 거리가 일반 추종의 정지/재개 거리
사이에 있어도 `ReturningOnFoot` 동안은 `MaximumDistance`까지 접근하도록 소스에 반영했다.
귀환 대사는 그 거리 안에 들어오기 전까지 queue에서 기다려, 멀리서 말풍선 수명을 소진하지
않는다. 기존 용의 검증된 안전 착륙은 별도 vehicle 계약이며 이 도보 접근과 구분한다.

### G06-04. 대사 ID, 공간 배치와 Client 소비자

현재 저작·게시 결과는 **대사 11개, 트리거 14개, 콤보 2개**, revision
**19755261657453**이다. `Data/Guide` 정본과 독립 Guide publisher를 유지한다.

| event | 저장 필드와 실제 실행 조건 |
|---|---|
| `GUIDE_STARTED` | Bern의 안내 시작 후 첫 인사. 과거 `PARTY_JOINED`는 reader 호환으로만 수용한다. |
| `SPACE_ENTER` | `boxId`, 위치·회전·half extents, optional NPC anchor. Bern에서는 안내 인간의 기존 몸체가 box 밖→안으로 들어온 전이를 판정한다. 가이드의 접촉으로 대체하지 않는다. |
| `RAID_RETURNED` | `raidWorldId=VALTAN_ARENA/KAKULSAYDON_ARENA`. 성공한 Bern 복귀 후 그 world의 대사를 예약한다. |
| `WORLD_RETURNED` | `sourceWorldId=MAHARAKA/COLOSSEUM`. 섬·콜로세움에서 성공한 Bern 복귀를 구분한다. |

위 새 시작·귀환 event는 Bern category만 허용한다. runtime은 고정 대문자 문자열이 아니라
catalog의 실제 category ID인 `bern`을 찾아 비교한다. 발탄/쿠크 귀환, 수리 NPC, 항구의
승선 안내, 생명의 나무와 물 안내는 저장된 prompt ID와 text segment를 통해 출력한다.

`CGuideAITool`은 `guideId → triggerId/boxId → promptId → text segments`를 보여 주며,
선택한 대사는 Rewrite/Apply/Cancel로 편집한다. Text는 segment별 UTF-8와 표시 시간을
검증한다. Debug F1의 `DimensionMaster Guide`에서 현재 Area의 `SPACE_ENTER` 행을
선택하고 한 번의 world picking으로 box 중심을 지정할 수 있다. 위치를 고른 뒤에는
NPC anchor를 해제하고 직접 정한 위치를 draft에 보관한다. 대기 중 다른 행·category·event로
바뀌면 피킹 결과를 적용하지 않으며 기존 위치를 보존한다. half extents/Y 회전 조절과
현재 Area box preview는 기존 편집 경로를 사용한다.

Save는 디스크 정본, Publish는 양쪽 runtime 파일을 교체한다. stable ID/필드 병합,
freshness 재확인과 원자 교체·실패 rollback을 유지한다. 편집 draft, 디스크 저장본,
게시 파일, 실행 중 Server가 읽은 revision은 서로 다른 상태다.

Shared protocol은 현재 통합 **130**, 새 `C2S_GUIDE_CONTROL` packet ID는 **122**다.
필드는 request sequence, guide net ID, `START/STOP`이다. `S2C_GUIDE_STATE`는 Bern 사람
전체에 idle owner 0 또는 안내 owner ID를 알리고, 실제 대사 packet은 owner에게만 보낸다.
시작·복귀 첫 대사보다 ownership state가 먼저 queue에 들어간다.
Client는 상태의 owner 변경/idle 때 이전 대기·표시 중 말풍선을 비우지만 채팅 이력과
event 중복 방지 순번은 유지한다. 같은 guide ID이고 자기 안내인 prompt만 수용한다.
Guide 상태를 더 이상 `GuideCompanion` 파티 행의 유무로 지우지 않는다.

### G06-05. 레이드와 공유 전투의 보존 경계

이번 Guide 전환에서 보스 HP·피해 수치·공용 스킬 정의·패턴·기믹·보상은 변경하지 않았다.
실제 레이드 room에 안내 actor가 들어가지 않으므로 가이드가 보스 타겟이나 인간 참가 인원을
추가하지 않는다. 기존 `GUIDE_AI`의 인간 전용 기믹 제외 코드는 유지했고, 자동 회귀의
synthetic nonhuman fixture는 그 제외 계약을 확인하는 용도다. fixture의 존재를 실제 레이드
동행 기능으로 해석하지 않는다. Colosseum의 별도 PvP 정책은 해당 작업 RESULT를 따른다.

### G06-06. 확인한 증거와 아직 확인하지 않은 범위

| 검증 | 확인된 결과 | 증거 |
|---|---|---|
| Guide Publish / CheckPublished | PASS, revision 19755261657453, 11 prompts / 14 triggers / 2 combos | `out/GuidePersonal20261001/publish.log`, `check-published.log` |
| Guide publisher 회귀 | 25 tests, OK | `out/GuidePersonal20261001/pipeline-tests.log` |
| Guide + 통합 protocol 130 Debug | build / guide / integrated exit 0 | `out/GuidePersonal20261001/protocol-debug-exits.json`, `protocol-guide-debug.log`, `protocol-integrated-debug.log` |
| 1차 Debug Product | PASS | `out/GuidePersonal20261001/debug-product-console.log`, 로그가 출력한 receipt `20260930T191451340Z-debug-product.json` |
| 1차 Server `--guide-ai-contract-test` | 54 PASS, failures 0, exit 0 | `out/GuidePersonal20261001/guide-ai-debug-first.log`, `contracts-debug-first.json` |
| 변경 Server source 인코딩·공백 | UTF-8/기존 CRLF 보존, scoped `git diff --check` PASS | `out/GuidePersonal20261001/freeze_checks.py` 및 실제 diff 검사 |

1차 Guide 실행은 singleton/no party, 다른 사용자 선점·종료 거절, 소문자 category 첫 인사,
상태가 인사보다 앞서는 순서, owner만 받는 대사와 도움 명령, 인간 공간 접촉, 용 비행,
ship 대기, 실제 outbound 거절 보존, 4인 입장 뒤 Bern identity·위치 유지, 가이드 없는 raid,
귀환 event, 새 Level의 STOP 1, 중복 START, 재시작·owner 연결 종료와 기존 Mario 인간 판정을
확인했다. ship 분기는 승인 상태인 `bShipDockValid`를 주입해 검사했으며 실제 승선 UI를
조작한 증거는 아니다. Kouku/섬/Colosseum 귀환은 해당 source world를 넣은 실제 `Join`
commit에서 event 선택을 검사했고, 각 지역의 모든 여행 UI를 실행한 것은 아니다.

위 1차 Debug 실행 파일은 마지막 `needsFollow`의 복귀 거리 구간 보완과 그 회귀 변경을
포함하지 않는다. 두 CPP의 후속 Debug 재빌드·최종 회귀, Release Product, 실제 LAN과
Client 화면 판정은 이 G06에서 완료로 기록하지 않는다. 최종 증거는 G08에서 별도로
기록한다. 가이드 메뉴·한글 말풍선·월드 picking·항구/귀환 도보 동작의 화면 확인은 사용자
실행이 남아 있으며, 에이전트는 Client 실행이나 자동 Reload를 하지 않았다.

## G07. 솔로 안내 사용자의 입장·귀환 트랜잭션 소스 보완

개인 안내로 바꾸며 내부 파티를 제거한 뒤, 솔로 레이드 이동이 목적지의 실제 입장 준비보다
먼저 출발 방을 떠나던 경계를 확인했다. 안내 사용자 한 명의 Bern↔발탄/쿠크 이동도
`ServerApp::Transfer_SessionWorld → Transfer_PartyTo`의 기존 admission·reliable FIFO 준비를
거치도록 보완했다. 준비가 모두 성공하기 전에는 출발 player·session binding과 가이드
owner·위치·대사 예약을 바꾸지 않으며, 성공 뒤에만 대기 또는 복귀 접근을 시작한다.

`Has_PersonalGuideOwner`는 Bern의 singleton 예약과 실제 `weak_ptr<CClientSession>`의
동일성, 현재 인간 binding 또는 다른 세계에서 돌아오기를 기다리는 상태를 읽어서 확인한다.
이 조건에 맞는 솔로 요청만 트랜잭션에 넣으며 숨은 인간 파티나 목적지 가이드는 만들지 않는다.
정상 연결에서 준비가 거절되면 기존 typed transfer failure를 보내고 연결을 유지한다.
FIFO가 가득 차면 그 응답도 보류했다가 송신 여유가 생기면 전달한다. 이미 종료된 session은
기존 종료 판정을 유지한다. 보스 HP·피해·스킬·패턴·기믹 코드는 이 보완에서 바꾸지 않았다.

반영 파일은 `Server/Public/GameRoom.h`, `Server/Public/ServerApp.h`,
`Server/Private/GameRoom_PartyWorld.cpp`, `Server/Private/ServerApp.cpp`,
`Server/Private/ServerGameplayContractTests_Guide.cpp` 다섯 개다. 기존 변경을 보존하고
반영 직전 SHA-256을 재확인했으며 UTF-8·기존 CRLF를 유지했다.

실제 NPC 솔로 입장과 클리어 후 귀환 요청을 `CServerApp`까지 연결하는 회귀 assertion 13개를 추가했다.
양방향의 목적지 identity/admission 거절, 실제 outbound FIFO 포화, 오류 응답 보류·재송신,
실패 시 캐릭터·가이드 ID/위치·대사·binding·연결 보존, 성공 시 파티 없는 이동과 raid Guide 0을
검사한다. G07 작성 시점에는 추가 회귀 실행 전이었으며, 실제 후속 실행 결과는 G08에 기록한다.
소스의 scoped `git diff --check`는 PASS이며 정확한 증분 diff와 적용 hash는
`out/GuideSoloTransaction20261001/candidate.patch`, `manifest.json`, `applied.json`에 있다.
이 소스 보완을 포함하는 최종 Debug/Release 빌드·회귀 결과와 남은 화면 확인은 G08에 기록한다.

## G08. 최종 Guide 계약·실패 응답 검사와 제품 빌드 대기

2026-10-01 04:53 KST의 최신 Debug `--guide-ai-contract-test`는 **67 PASS, 0 FAIL,
exit 0**이다. G07의 실제 솔로 입장·귀환 transaction, 양방향 admission 거절, outbound FIFO
포화와 실패 응답 보류·재송신, 기존 인간·guide identity 및 연결 보존을 포함한다.
증거는 `out/GuidePersonal20261001/guide-ai-debug-confirmed.log`와 같은 이름의 JSON이다.
05:05 KST의 최신 Release도 **67 PASS, 0 FAIL, exit 0**이다.
[Release 계약 로그](../../../out/GuidePersonal20261001/guide-ai-release-complete.log)에서
솔로 귀환의 실제 `S2C_PARTY_TRANSFER_RESULT`를 reader로 decode하여 Bern 목적지와
귀환 request sequence, admission 실패 및 FIFO 해소 뒤 outbound-busy 응답을 확인했다.
오류 알림만 생성했다는 검사로 대체하지 않았으며 출발 raid binding과 대기 guide가 보존된다.
실행 종료값은 [Release 계약 JSON](../../../out/GuidePersonal20261001/contracts-release-complete.json)에 있다.

이 검사에서 확인한 Bern 귀환 실패 응답의 codec 제한도 수정했다.
`S2C_PARTY_TRANSFER_RESULT` writer/reader는 실제 대상인 Bern·발탄·쿠크·Maharaka·Colosseum을
허용하고 그 외 world는 거절한다. protocol 130, packet ID와 기존 7-byte payload는 유지한다.
Client는 기존 실패 알림을 소비해 현재 지역을 유지하며, 쿠크와 발탄 클리어 후에도 동일한
8초 알림을 표시하도록 연결했다. 파티 메뉴·입력 소유·보스 전투 수치는 추가하거나 바꾸지 않았다.
이 Client 알림 연결의 화면 확인은 수행하지 않았다.

| 후속 검사 | 확인 결과 | 증거 (`out/GuidePersonal20261001/`) |
|---|---|---|
| Debug `--party-transfer-only` | 216 PASS, 0 FAIL, exit 0 | `protocol-party-debug.log`, `protocol-party-exits.json` |
| Debug `--integrated-130-only` | 268 PASS, 0 FAIL, exit 0 | `protocol-integrated-final-debug.log`, `protocol-party-exits.json` |
| 실패 응답 입력 | 5 world × 5 reason 왕복, 모든 truncation과 잘못된 world/reason 거절 | `party-transfer-codec-client-audit.json` |

04:43 KST의 이전 전체 Product 성공은 `product-delivery-exits.json`에 남아 있다.
후속 정상 Server [Debug 빌드](../../../out/GuidePersonal20261001/server-guide-final-debug-build.log)와
[Release 빌드](../../../out/GuidePersonal20261001/server-guide-final-release-build.log)는 실제 제품
Server.exe 링크까지 성공했다(각 경고 0/오류 0, 4.85초/14.91초). Server의 이전 링크 보류는 해소됐다.
이후 최종 이동 수정과 실패 알림 연결을 포함하는 Client 전체 Product 빌드는 아직 대기 중이며,
Server·Shared 성공을 최신 전체 제품 완료로 대신 기록하지 않는다.
실제 LAN/Client 화면·월드 picking·안내 시작/종료·귀환 도보
확인은 사용자가 수행하며 에이전트는 Client/UI를 실행하거나 자동 Reload하지 않았다.

사용자의 최종 지정에 따라 이번 통합 전달 리소스의 목적지는
`C:/Users/user/Desktop/GBResources2`다. 기존 콜로세움 1,109개는 동일하여 건너뛰고
워터팡 추가분 7개를 합쳤으며, 총 1,116개 / 412,862,726 bytes의 원본 SHA-256 일치를 확인했다.
기존 `GBResources`는 변경하거나 삭제하지 않았다. 범위별 개수·hash는
`out/GuidePersonal20261001/resource-delivery/delivery-manifest-gbresources2.json`과
[콜로세움 리소스 전달 기록](../10-01/2026-10-01_COLOSSEUM_MATERIAL_RESTORE_RESULT.md)을 따른다. Guide 자체에 새 asset을 생성한 작업은 아니다.

## G09. 이동 보완을 포함한 최종 제품 빌드와 전달

마지막 LocalMovePrediction/Character 수정과 Shared의 다섯 목적지 party-transfer 실패 codec,
Valtan/Kouku 실패 notice 소비자를 포함하여 정상 Product Debug/Release를 순서대로 실행했고
Engine, Shared, Server, Client 컴파일·링크·배포가 모두 성공했다. Clean/Rebuild·FXC 생략·
출력 시각 조작·사용자 프로세스 종료는 사용하지 않았다.

증거는 `out/GuidePersonal20261001/product-complete-exits.json`,
`debug-product-complete-console.log`, `release-product-complete-console.log`와
각 `product-complete-debug/release` 로그 디렉터리다. 실제 출력은
`Client/Bin/Debug/Client.exe`, `Client/Bin/Release/Client.exe`,
`Server/Bin/Debug/Server.exe`, `Server/Bin/Release/Server.exe`다.
앞선 RESULT의 EXE 잠금 및 최종 Client 빌드 대기는 이 성공 결과로 해소한다.

이동 primitive의 최신 Debug/Release 결과와 연타·지연·방향·코너 검증 범위는
`../09-22/2026-09-22_BERN_RELEASE_PROFILER_OPTIMIZATION_RESULT.md`의 최신 항목을 따른다.
Guide/Colosseum 계약은 Debug/Release 각각67/134 PASS이며, 나머지 서버 검증과
GBResources2의 리소스7개 신규 전달·기존1109개 동일 확인·총1116개 해시 일치 증거는
G08 및 기능별 RESULT를 따른다. 기존 GBResources는 보존한다.

제품 실행·UI·4인 LAN·GPU 성능을 자동 확인한 결과는 아니다. 새 Server와 새 Client를
protocol130으로 함께 실행해야 하며 이전 zip을 새 서버와 혼용하지 않는다. Guide 게시 파일은
현재 실행 중인 Server의 메모리나 열린 저작 draft를 자동 갱신하지 않는다.

## G10. 콜라이더 전체 표시와 별도 상세 창 (2026-10-01)

### G10-01. 실제 소스 반영

`Client/Public/GuideAITool.h`, `Client/Private/GuideAITool.cpp`의 기존 도구를 확장했다.
`콜라이더` 탭은 현재 category의 SPACE_ENTER만 표시하며 행을 선택하면 별도 ImGui
`Collider Detail` 창을 연다. 일반 트리거 목록에서도 공간 행 선택으로 같은 창을 연다.
목록은 높이를 제한하고 스크롤하여 편집 진입을 목록 아래에 묻지 않는다.

상세 창은 월드 중심 Position, 전체 X/Y/Z Size(m), Y축 Rotation(degree), world picking,
NPC 위치·회전 복사, Enabled, 대사, cooldown, priority와 Save/Publish를 제공한다.
전체 Size는 저장 시 기존 halfExtents로 나누며 최소값은 JSON double 기준0.01로 보정한다.
직접 위치·회전을 바꾸면 NPC 복사 참조를 해제한다. 피킹 중 Detail을 닫으면 취소하며
메인 창만 닫고 Detail을 남기면 상세 편집을 계속할 수 있다.

상단과 상세 창의 `Show Debug`는 공통 상태다. 기본 범위인 `All colliders in active Area`는
현재 지역의 활성·비활성 Guide 박스를 모두 표시하고 끄면 선택한 박스만 표시한다.
선택은 노랑, 다른 활성은 청록, 비활성은 회색이며 box ID와 disabled 표시를 붙인다.
선분을 clip-space에서 자른 뒤 투영하여 near plane을 가로지르는 박스도 표시한다.
탭 전환과 창 접힘에도 draft 표시를 유지한다. 모든 Guide 편집 창을 닫으면 표시하지 않는다.
서버 판정·JSON schema·publisher·protocol·Resources 변경이나 신규 C++ 파일은 없다.

### G10-02. 자동 검증과 실행 파일 경계

- 최종 Debug `Client.vcxproj /t:ClCompile` exit0. `out/GuideCollider20261001/client-compile-final.log`.
  GuideAITool와 헤더 소비자의 컴파일을 확인했고 기존 SDK 헤더의 C4828 경고는 남아 있다.
- Client 정상 증분 `Build`에서 컴파일 후 `Client/Bin/Debug/Client.exe` 링크가 LNK1168로
  실패했다. 실행 중인 기존 Debug Client가 출력 파일을 점유한다.
  `out/GuideCollider20261001/client-build.log`. 마지막 피킹 취소 보완도 별도 최종 컴파일은 성공했다.
- 기존 Guide 저장 회귀4개 성공: 독립 필드 병합·명시 게시, 동일 필드 충돌 보존,
  잘못된 입력의 원본 보존, 동시 추가 행 보존. `out/GuideCollider20261001/save-tests.log`.
  fixture는 임시 복사본이며 사용자 Data/Guide를 덮어쓰거나 게시하지 않았다.
- 변경 소스의 UTF-8 BOM 없음·LF를 유지했고 scoped `git diff --check`를 확인했다.

최종 EXE 링크는 사용자 Client 종료 후 이어서 수행한다. Client/UI를 실행하거나 종료하지
않았고 실제 화면·입력·재로드 확인은 사용자에게 남아 있다. 확인 경로는 Debug F1 →
DimensionMaster Guide → 콜라이더 → 행 선택 → Collider Detail → Show Debug →
Position/Size/Rotation 변경 → Save → 완료 상태 확인 → Publish다.

### G10-03. 사용자가 추가한 아바타 구매 행의 읽기 전용 검토

검사 시 대사12/트리거15, revision277706130077721의 Validate와 CheckPublished가 성공했다.
Client/Server Guide.runtime.json SHA256은
`26e0ee1a4029ffe3c27e4c858e2f052b020bfc99bfa93669055bd07a743125dc`로 동일했고,
원본5개·게시본2개·Bern Gameplay.world.json은 검사 전후 hash/수정시각이 같았다.

`guide.prompt.19889218.1`의 아바타 구매 대사·6초 표시는 정상 저장됐다. 그러나 기존
`guide.trigger.bern.first_invite`의 GUIDE_STARTED가 그 대사를 참조하고,
신규 `guide.trigger.20001546.2`는 enabled=true / GUIDE_STARTED / promptId 빈 값이다.
아바타 대사와 연결된 SPACE_ENTER가 없어 상점 접근 안내용 위치·크기·회전은 아직 없다.
Publish 성공과 의도한 trigger 연결을 구분해 사용자에게 보고했으며 데이터 수정·재게시하지 않았다.

## G11. 기능 NPC 안내와 가이드 버그 수정 (2026-10-01)

### G11-01. 승인 범위와 데이터 반영

사용자가 읽기 전용 검토 후 전체 수정을 승인했다. 안내 시작은 기존 일반 인사
`guide.bern.party.first_invite`로 복원하고, 아바타 대사와 사용자가 만든 빈 trigger의
stable ID를 보존하여 도서관 `npc.bern.plaza.17`의 SPACE_ENTER에 연결했다.
베다 레이드 박스를 유지하고 회전을 현재 NPC와 맞췄으며 아일라라 레이드·물약·PvP
박스를 추가했다. 기존 제련·수리·항구·물 박스와 귀환4개, 기존 대사12개 내용은 보존했다.

물약 상점은 수리 NPC 옆 `npc.bern.plaza.05` 한 명만 활성화했다. 다른 배치9개는
enabled=false로 보존하고 상점 binding에서 제외했다. 비활성 NPC를 바라보던 lookTarget
5개는 null로 정리했다. 배틀 아이템4종과 가격·아바타 상점 상품은 변경하지 않았다.
Guide·BERN World·Item publisher로 Client/Server 실행 데이터를 게시했다.

현재 Guide revision은 `54095755338659`, 대사14개·트리거18개·SPACE_ENTER12개이며
NPC anchor11개의 position/yaw는 활성 Gameplay NPC와 일치한다. 두 Guide 실행 파일의
SHA256은 `77db40dd33f03fdcdf2ba9eef88b013c75a18da41864db6f7f26a0cf3d8adeb0`다.
원본 백업·수정 hash는 `out/GuideNpcFix20261001/backup`, `manifest.json`,
게시 이전 파일은 같은 폴더의 `runtime-before`에 있다.

### G11-02. 코드 반영과 검증 경계

공간 대사는 발생한 trigger 출처와 priority를 유지하고, 첫 발화 전에 실제 owner의
현재 접촉을 검사한다. 떠난 공간의 미시작 대기를 버리고 실제 첫 송신에만 cooldown을
소비한다. 시작 인사·실제 귀환·이미 시작한 여러 segment는 공간 이탈로 버리지 않는다.
안내 시작과 월드 귀환 시 기존 접촉을 초기화하며, Guide만 이동할 때는 owner 접촉을
보존하여 실제 지역 이동과 구분한다. 같은 대사를 공유하는 박스의 중첩·출처 변경도
별도 회귀 범위다. Client 미니맵은 비활성 NPC를 제외하고, Guide 시작 메뉴는 서버와
같은 XZ10m 조건을 표시하며 안내 종료는 원거리에서도 유지한다.

Guide publisher는 삭제·비활성·비NPC anchor와 좌표·회전 불일치 및 대사·콤보가 없는
활성 trigger를 검출한다. 편집용 Validate는 경고로 문서를 열 수 있게 하고
Save/Publish/CheckPublished는 실패 시 원본·게시본을 보존한다. 좌표 자동 덮어쓰기는 없다.

완료한 자동 검증:

- BERN World Validate/Publish, Item Validate/Publish/CheckPublished, Guide Publish/CheckPublished 성공.
- `python Tools/GuidePipeline/test_guide_pipeline.py`:34개 PASS,111.585초. 임시 fixture에서
  저장 병합·실패 보존·anchor 오류·재복사·yaw360도·자유 배치·빈 trigger·구조 오류를 검증했다.
- `python out/GuideNpcFix20261001/verify_data.py`:PASS. 저작/게시 source 연결, 활성 물약1명,
  상품 보존, 기존 대사12개·무관 trigger12개 보존을 검사했다. `data-verification.json` 참조.

### G11-03. 최종 컴파일·계약 검증과 남은 Release 적용

Engine → Shared → Server → Client를 x64 Debug의 정상 MSBuild `/t:Build`로 실행하여
각각 exit0을 확인했다. `Server/Bin/Debug/Server.exe`, `Client/Bin/Debug/Client.exe` 링크와
기존 Debug 의존 DLL 배포도 성공했다. 로그는 `out/GuideNpcFix20261001/`의
`engine-debug-build.log`, `shared-debug-build.log`, `server-debug-build.log`,
`client-debug-build.log`다. Source/IntDir/OutDir 변경이나 Clean/Rebuild는 하지 않았다.
Client의 기존 MainApp·EngineSDK 문자 집합 경고는 남아 있으며 이번 변경으로 일괄 변환하지 않았다.

새 Debug Server의 `--guide-ai-contract-test`는 **97 PASS,0 FAIL,exit0**이다.
`guide-debug-contract.log`, `guide-debug-result.json`에서 실제 송신 패킷을 decode하여
이탈 예약 취소·즉시 재진입·중첩 출처 승계·미발생 출처 배제·우선순위·발화 cooldown·
여러 segment·Guide 재배치와 실제 지역 이동의 구분을 확인했다. 기존 시작/종료·선점·
연결 종료·승선·용 추종·원자적 입장 실패 보존·발탄/쿠크/워터팡/PvP 귀환 검사도 통과했다.
추가 diff의 독립 리뷰에서도 중첩 후보와 참조 수명·귀환 상태 보존을 확인했다.

Release Server와 Client는 `/t:ClCompile`을 각각 실행하여 exit0을 확인했다.
`server-release-compile.log`, `client-release-compile.log` 참조. 실행 중인 기존 Release
Server(PID39408)/Client(PID29056)의 EXE는 교체하지 않았다. 제품 runner는 실행 중인
표준 출력을 일괄 차단하므로, 잠기지 않은 별도 Debug 출력의 정상 빌드와 Release 소스
컴파일을 분리하여 검증했다. 사용자에게 실제 Release 종료 확인을 한 번 요청했으며
아직 종료·Release 링크·새 Release 계약 실행·사용자 화면 확인은 남아 있다.

관련 코드·게시본과 무관한 변경은 보존했다. C++는 기존 UTF-8 BOM 없음/CRLF,
JSON은 기존 인코딩과 줄바꿈을 유지했고 `git diff --check`를 확인했다.
새 C++ 파일·protocol·project/filter 변경은 없다. 디스크 게시를 실행 중 Server revision
갱신이나 사용자 화면 확인으로 기록하지 않는다. 쿠크1관문 첫 팝업 음원은 이전 제거
사유가 있는 별도 항목으로 복원하지 않았다.

### G11-04. 종료 확인 후 최종 Release 배포 완료

2026-10-01 사용자가 Client·Server 종료를 알린 뒤 정식 Product Release 빌드와
새 Release Server Guide 계약97개 PASS를 확인했다. G11-03의 Release 링크 대기는
해소됐다. 레이드 캐릭터 생성 정보 인계와 강화 창 이름표 가림 수정도 함께 포함한
`C:/Users/user/Desktop/LostArk-Release-20261001-GUIDE-RAID-FIX.zip`을 생성했다.
크기169183087 bytes, SHA256
`46bc1600add091587047ccb12f59590bbd2a582f601476950183f7633db62053`이다.
빌드 receipt·ZIP CRC/manifest 검증·실행하지 않는 launcher 검사와 남은 사용자 화면
확인 경계는 `../10-01/2026-10-01_RAID_ENTRY_CHARACTER_HANDOFF_RESULT.md` G02를 따른다.


## G12. 건물 출입 동행·연결된 가이드 착지 (2026-10-02)

Bern의 castle/castle.2/library/library.2는 Server의 authored MOVE_PLAYER다.
기존 PlayerSimulation은 완료 후 Guide_AnchorArrived(player)의 기본 false를 전달해
Guide가 실외에서 실내까지 도보로 접근하도록 요청했다. 완료 시 Bern의 authored 이동에만
localMapTravel=true를 전달하여 기존 스퀘어홀 도착 경로를 재사용한다.

기존 Find_GuideLanding은 base surface·높이·충돌만 확인하고 owner와 연결된 보행 영역인지
확인하지 않았다. 실제 설치된 ship 목적지 주변에서 yaw180/195의 후보가 4칸 고립 영역에
선택되는 nav 계산을 확인했다. 현재 Bern runtime blocker는 모두0이므로 blocked mask
누락을 이 증상의 원인으로 단정하지 않는다.

모든 후보의 exact walkability를 확인하고 local 도착은 owner부터의 navigation line of sight를
추가로 요구한다. 선호하는 뒤쪽 위치가 없으면 owner 주변에서 찾는다. 후보를 먼저 검증한 뒤
actor 이동·명령 초기화를 적용하며 거부 시 이전 Guide 상태를 보존한다. 실제 승선·세계 전송
대기와 Bern 귀환 도보 접근은 유지한다. Debug/Release 공통 Server 코드다.

실제 네 건물 trigger의 hold 중 위치 보존·완료 동행, 항구24방향 착지 연결성,
nav 밖 후보의 기존 상태 보존, 기존 스퀘어홀·안내 수명·승선·세계 전송 계약을 검사했다.
소스4파일과 PLAN 전문의 byte 일치, UTF-8 BOM 없음/CRLF 및 git diff --check를 확인했다.

- 실제 v143 환경의 Server Debug ClCompile exit0, 약80초. Compile-Exit.json 참조.
- 현재 Server product object 102개와 Shared library를 사용하는 외부 진단 EXE의 링크 exit0.
  Link-Retry-Exit.json에서 표준 Server.exe/pdb/lib 및 link tracking 파일의 보존을 확인했다.
- 새 외부 EXE의 --guide-ai-contract-test: 111 PASS, 0 FAIL, exit0, 53.057초.
  Guide-Contract-Exit.json 및 Guide-Contract.stdout.log 참조. 동일 actor identity와 실제 published
  trigger 소비, 패킷·상태 결과를 검사했으며 GUI Client를 조작한 검사가 아니다.
- 같은 EXE의 --navigation-contract-test: exit0, navigation failures0, 12.460초.
  Navigation-Baseline-Exit.json 참조. 기존 nav의 기본 계약 검증이며 제작 지구나 전체 맵의
  연결성 복구 성공을 의미하지 않는다.

당시 실행 중인 Debug Client/Server와 기존 표준 제품 출력은 보존했다. 공통 Server 소스이지만
새 변경의 표준 Product Debug/Release 링크·배포 및 사용자의 실제 건물/항구 화면 확인은 남아 있다.

원본 백업은 저장소 밖 LostArkTransfer/Sync-20261002/Recovery-20261002-1315/GuideTravel-20261002다.
베른 전체 nav의 제작 구역 고립·누락 복구는 별도 조사 중이며 이 Guide 코드로 복구됐다고
기록하지 않는다. 기존 idle build queue는 새 변경 검증을 위해 중지했으며 게임은 유지했다.


### G12-01. 복구된 Bern nav와의 결합 검증

외부 후보02의 전체 Server DataFiles를 사용한 새 진단 EXE에서 Guide111개 PASS/exit0,
47.02초를 확인했다. Nav 추가 회귀도58개 PASS로 제작 지구 스퀘어홀 연결을 확인했다.
데이터8파일의 정본 반영과 미구현 실내 입구, 실제 게임 재시작 경계는
[10-02 Bern nav RESULT](../10-02/2026-10-02_BERN_NAV_GROUND_RECOVERY_RESULT.md)에 둔다.
