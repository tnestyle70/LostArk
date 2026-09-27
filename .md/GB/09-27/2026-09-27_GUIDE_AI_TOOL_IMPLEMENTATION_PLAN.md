# Guide AI Tool · 가이드 차원술사 구현 계획서

작성일: 2026-09-27

## G00. 요청과 구현 경계

사용자가 작성한 기능 명세를 구현한다. 파티는 **사람 최대 4명 + 별도 가이드 1명**이다.
가이드는 차원술사 캐릭터와 기존 Server player 실행기를 사용한다. 미리 작성한 대사를
재생하며 `Rewrite`는 편집 모드다. 외부 생성형 AI 서비스는 호출하지 않는다.

작업 시작 기준은 `codex/bern-dragon-camera-performance`,
HEAD `8543589d4c15e8a9af06813daaeb8692b3215e4e`다. 기존 미커밋 변경을 보존하고,
Client/UI는 에이전트가 실행하지 않는다. 실제 구현·검증 결과는 같은 이름의 RESULT에서 구분한다.

## G01. 도구와 저장 구조

F1 Developer Tools와 World Level Tool에서 독립 `Guide AI Tool`을 연다.
상단에는 Load / Save / Publish, 시작 Position, Rotation Y와 Bern spawn 가져오기를 둔다.
Server player는 yaw만 지원하므로 X/Z 기울기를 저장하지 않는다.
기본 위치는 `LV_BER_BERNCASTLE/Gameplay.world.json`의 `player_1` 좌표
`[137.586334, 42.2498169, -22.4640217]`, yaw 0이다.

최상위 카테고리는 베른·발탄·쿠크세이튼 순서이며 베른을 기본으로 선택한다.
하위 편집은 대사, 트리거, 콤보/도움 명령이고 Combat Detail은 별도 창이다.

| 저작 정본 | 역할 |
|---|---|
| `Data/Guide/GuideCatalog.json` | 카테고리 순서, 가이드 정의와 문서 참조 |
| `Data/Guide/DimensionMaster/Placement.json` | 위치/yaw, 초대한 사람 또는 공대장 추종 정책 |
| `Data/Guide/DimensionMaster/Prompts.json` | stable prompt ID, category/topic/title, text segment와 표시 수명 |
| `Data/Guide/DimensionMaster/Triggers.json` | event 종류, Box/패턴/명령 ID, prompt와 우선순위·쿨다운 |
| `Data/Guide/DimensionMaster/Combat.json` | 추종 거리, 판단 변수, 상황별 가중치, 스킬 콤보, 별칭 명령 |
| `Tools/GuidePipeline/Publish-Guide.ps1` | Validate / Save / Publish / CheckPublished |
| `Client/Bin/DataFiles/Guide/Guide.runtime.json` | 검증된 Guide 게시본 |
| `Server/Bin/DataFiles/Guide/Guide.runtime.json` | 같은 revision의 서버 실행 정의 |

각 원본의 `schema`, `formatVersion=1`, `revision`, stable ID를 검증한다.
Runtime revision은 resolve된 문서의 SHA256에서 얻는 48-bit 정수다.
스킬은 `(DIMENSIONMASTER,inputSlot)`로 `PlayerSkills.json`에서 찾고 실제 게시 bootstrap의
같은 binding도 검증한다. 저장에는 skill ID를 직접 입력하지 않는다.

Save는 편집 시작 baseline, draft, 최신 disk를 stable ID/필드 단위로 병합한다.
무관한 외부 변경은 보존하며 같은 필드 충돌은 저장 실패와 기존 draft 보존으로 처리한다.
검증·source freshness·destination mutex·교체 직전 hash 확인·원자 교체·실패 rollback을
공통 경계로 사용한다. Publish는 저장본만 읽어 Guide 두 runtime 파일을 교체한다.
다른 Gameplay/Map/Vehicle 데이터 publisher를 호출하지 않는다.
Server는 시작 시 게시본을 소비하므로 파일 Publish와 실행 중 Server 적용을 구분한다.

새 C++ 파일은 UTF-8 BOM 없이 저장하고 프로젝트와 filters에 필요한 항목만 등록한다.
원본 Data는 Client 프로젝트 `96.DataFiles`의 None 항목에 둔다.

## G02. 대사·공간·패턴·도움 이벤트

대사는 `promptId/categoryId/topic/title/segments[]`로 저장한다. segment는
UTF-8 512bytes, 1~20초이며 한 prompt는 최대 16개 segment, 4096bytes, 총 60초다.
기존 일반 채팅 256byte 제한을 늘리지 않고 `S2C_GUIDE_PROMPT`를 사용한다.

Rewrite는 선택한 대사를 별도 draft에서 수정하고 Apply/Cancel로 확정한다.
트리거의 prompt 참조와 콤보 참조는 ID로 선택한다. 참조 중인 행은 삭제를 막는다.

| event.type | 입력과 판단 |
|---|---|
| PARTY_JOINED | 서버가 가이드 동행을 commit한 직후 첫 인사 |
| SPACE_ENTER | 가이드 player collider와 독립 Guide OBB의 진입 |
| BOSS_PATTERN_STARTED | 서버의 실제 pattern ID와 occurrence sequence 변화 |
| HELP_COMMAND | 같은 파티 인간이 입력한 등록된 정확한 별칭 |

SPACE_ENTER는 boxId, position, yawDegrees, halfExtents, anchorPlacementId를 갖는다.
NPC anchor 선택은 현재 Gameplay placement의 transform을 draft로 복사한다.
초기 레이드 NPC는 `npc.bern.beda.guide`, 제련 NPC는 `npc.bern.schmidt`다.
Guide Box는 기존 Map trigger action에 섞지 않고 별도 Guide 문서로 소유한다.

초기 대사는 첫 인사, 레이드 NPC, 제련 NPC, 진짜 세이튼 설명이다.
진짜 세이튼은 실제 게시 패턴 `KAKULSAYDON_G1_PATTERN_2`에 연결한다.
보스/공간 이벤트는 설명을 내보낸다. 전투는 도움 명령이 있어야 시작한다.
우선순위와 쿨다운은 서버 큐에서 적용하며 이미 대기 중인 같은 대사는 중복 적재하지 않는다.

## G03. 서버 actor·파티·추종

`SERVER_PLAYER::eControlKind`와 Shared의 `PLAYER_CONTROL_KIND::GUIDE_AI`로 제어 주체를
구분한다. 가짜 session을 만들지 않는다. 기존 player 복제와 CCharacter 표현을 재사용한다.
Bern 초대용 actor는 공용으로 남고 초대한 파티에 별도 companion actor를 만든다.
초대용 actor는 전투 비활성이고 실제 companion은 일반 피격을 받을 수 있다.

인간 roster 상한은 4로 유지하고 `S2C_PARTY_ROSTER`의 optional GuideCompanion에
다섯 번째 행을 보낸다. 가이드는 공대장, 인간 입장 인원, 준비 투표, MVP에 포함하지 않는다.
인간과 companion을 함께 이동시키는 파티 transfer는 target admission과 초기 reliable frame
준비를 먼저 마친 후 commit한다. 실패하면 source의 파티와 actor를 보존한다.

기본 anchor는 초대한 사람이며 이탈하면 현재 인간 leader를 따른다.
거리 band 밖에서 기존 이동 실행기에 명령을 보내고 navigation/collision을 서버가 승인한다.
스퀘어홀 도착은 실제 anchor 도착 후 안전한 근처 위치를 찾는다. 원거리 복구도
전투 중 무조건 순간이동하는 방식으로 사용하지 않는다.
용 탑승·이륙·비행·착륙은 기존 vehicle 명령과 서버 상태를 소비한다.

## G04. Combat Detail과 판단

체력과 도움 요청을 함께 고려한다. FOLLOW는 attack weight가 0이고,
ASSIST는 등록된 콤보가 활성화된 상황이다. 현재 구현은 세 행동 점수를 비교한다.

- `Follow = wFollow * clamp((anchorDistance - maximumDistance) / max(1,resumeDistance),0,1)`
- `Avoid = wAvoid * clamp(contactRisk,0,1)`
- `Attack = wAttack` (도움 콤보와 유효 적이 있을 때), 그 외 0
- 가장 큰 점수의 행동을 선택하고 hold time/switch margin으로 잦은 전환을 억제한다.
- 즉사 접촉 또는 위험 상태에서 HP가 설정 임계치 아래면 회피를 우선한다.

초기 FOLLOW 가중치는 attack 0 / avoid .4 / follow .6,
ASSIST는 attack .5 / avoid .35 / follow .15다. 각 행 합은 1이다.
변수는 `follow`의 desired/minimum/maximum/resume/recover 거리와 recoverDelayMs,
`decision`의 thinkIntervalMs/horizonMs/switchMargin/minimumHoldMs/lethalHpFraction이다.

위험도는 확정 피해량이 아니라 알려진 서버 콜라이더를 이용한 계획 점수다.
일반 접촉·CC를 1, 즉사 접촉을 5로 평가한다. 발사체의 게시 shape와 현재 이동,
보스 stage의 hit schedule/shape, 쿠크의 활성·잔존 Logic collider와 빙고 망치 이동을 조회한다.
미래 보스 stage의 위치는 현재 확정 pose를 기준으로 예측하므로 완벽한 회피를 보장하지 않는다.
실제 피해 판정은 기존 collision/damage 경로에만 있다.

Combat Detail은 편집 변수와 서버가 보낸 실제 context/action, 점수, 위협도,
anchor 거리, HP 비율, 생존 우선 여부, 선택 이유, 콤보 단계, 적용 revision을 함께 보여준다.
편집 draft를 실제 실행값으로 표시하지 않는다.

## G05. 콤보 실행

`comboId/displayName/inputSlots/timeoutMs/stepWaitMs/repeat`를 저장한다.
슬롯 순서를 수정하고 콤보 행을 추가한다. commands는 commandId/displayName/aliases와
comboId 또는 stop, enabled, cooldownMs를 갖는다.

초기 콤보 1: `W → A → S → D → F → V → T → ALT_V`.
초기 콤보 2: `ALT_V → V → W → A`.
`도움!`, `도와줘`, `도와줘!`는 1번, `살려줘`, `살려줘!`는 2번,
`그만`, `그만!`, `멈춰`는 보조 중단이다. ALT_V는 실제 차원술사 공격 스킬이며
회복 효과를 임의로 부여하지 않는다.

Skill executor가 range/resource/cooldown/state를 확인하고 승인한 단계만 진행한다.
현재 스킬이 끝날 때까지 기다리고 접근이 필요하면 이동한다. 미사용 스킬 대기 시간과
전체 timeout이 끝나면 정지한다. 콤보 교체는 실행 중인 스킬이 끝난 뒤 적용한다.

## G06. 기믹 제외와 물리 피해

가이드는 boss/random/volley/tracking 타겟 후보와 인간 전용 판정에서 제외한다.
카드·광기·춤·카드 미로·마리오 배정·빙고 폭탄 머리 표식은 인간에게만 준다.
가이드의 공격은 HP 보조 피해만 주고 무력화·부위파괴·카운터·MVP 기여에 포함하지 않는다.

같은 공간 콜라이더에 닿으면 일반 damage/CC/갈고리/칼날 결과를 받는다.
기믹에 참여하지 않아서 생기는 미응답 판정과 전멸 결과는 받지 않는다.
물리 접촉 latch는 밖으로 나갔다가 재진입할 때 정상적으로 다시 활성화한다.
사망한 가이드는 인간이 살아 있을 때 서버가 검증한 레이드 복귀 지점에서 부활하고 다시 추종한다.
인간 전멸 판정에는 가이드의 생존이 영향을 주지 않는다.

## G07. 파일 책임과 검증

| 파일 묶음 | 책임 |
|---|---|
| GuideAIDocument.h/.cpp | draft/baseline, 비동기 검증·Save·Publish 상태와 실패 보존 |
| GuideAITool.h/.cpp/_Combat.cpp | 편집 창과 읽기 전용 서버 관측 |
| MainApp, WorldLevelTool | 도구 열기·수명·입력 소유 연결 |
| Shared PacketType/PacketMessages | protocol 116 actor/roster/prompt/state 계약 |
| NetworkManager, ClientReplication | ordered receive, dedup, guide 생성/삭제와 대사 queue |
| PartyWindow/PartyInteraction/ChatWindow/WorldPlayerChatBubble | 5번째 행과 기존 제품 UI 표현 |
| GuideCatalog.h/.cpp | 게시 정의 parse/validate/stage/commit |
| GameRoom_Guide.cpp | 서버 companion 수명·추종·콤보·대사·진단 |
| GameRoom_GuideThreat.cpp | 기존 서버 contact geometry를 이용한 위험 조회 |
| GameRoom party/admission/player/vehicle/replication | 실제 실행·전송·입력 소비자 연결 |
| ValtanBrain/KoukuLogic/CombatHit/CombatObject/MonsterBrain | human-only 기믹과 물리 피격 경계 |

필요 검증은 Guide publisher의 저장/충돌/게시 rollback, protocol codec,
서버 실제 초대·4+1·입장 transfer·도움 실행·피격/기믹 분리,
변경 C++ 컴파일, JSON/XML parse와 diff-check다.
Client와 아레나 화면은 사용자가 직접 확인한다. 실행하지 않은 화면·라이브 LAN 테스트를
완료로 기록하지 않는다. Protocol이 바뀌므로 Client와 Server는 같은 빌드를 사용한다.
