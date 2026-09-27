# Guide AI Tool · 가이드 차원술사 구현 결과

작성일: 2026-09-27

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
