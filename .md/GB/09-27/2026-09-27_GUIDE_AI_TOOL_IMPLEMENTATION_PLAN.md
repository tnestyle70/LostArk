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


## G08. 서버 단일 안내와 귀환 이벤트로 전환 (2026-10-01 요청)

이 항목은 G03의 파티 companion·레이드 동행 기획을 대체한다. 가이드 시작은 기존 typed
명시적인 안내 시작/종료 typed command를 사용하며 인간 파티를 생성하거나 roster slot을 차지하지 않는다.
서버의 shared Bern에 처음 배치한 단일 actor를 유지한다. 안내 시작은 clone을 생성하지 않고
그 actor를 최초 요청 owner session에 귀속한다. 다른 사용자는 안내 중으로 표시하며 제어권을
빼앗지 않는다. 첫 인사와 공간 대사는 안내 사용자에게만 보낸다.
보스 레이드 동행은 폐기하며 보스 HP·damage·기믹·보상은 수정하지 않는다.

Guide는 배 승선 또는 레이드·콜로세움·마하라카 이동 성공 때 마지막 Bern 위치에 대기한다.
실패한 transfer는 기존 안내 상태도 보존한다. 실제 Bern 입장 commit 후 같은 owner session을
새 player identity와 연결하고 기존 navigation 이동으로 접근한다. 이 접근 중 원거리 복구
teleport를 사용하지 않는다. 안내 종료와 owner disconnect는 대기·예약 대사·추종 상태를
해제하며 actor는 유지한다. 종료 후 늦게 도착한 world 귀환으로 다시 활성화하지 않는다.
다른 사용자가 같은 actor에서 안내를 시작할 수 있다. 용의 탑승·비행 추종은 유지한다.

GUIDE_STARTED는 안내 시작, RAID_RETURNED는 raidWorldId의 발탄/쿠크 귀환,
WORLD_RETURNED는 sourceWorldId의 MAHARAKA/COLOSSEUM 귀환을 뜻한다. 모두 Bern category만
허용한다. 과거 PARTY_JOINED 입력은 reader 호환 범위로 남기되 새 저작은 GUIDE_STARTED다.
SPACE_ENTER는 Bern의 owner player와 기존 Guide OBB의 outside→inside 전이로 판정한다.
guideId → triggerId / boxId → promptId → text segments의 기존 JSON 저장 구조를 유지한다.

수리 NPC npc.bern.src.31/48, 항구 shipwright.1/2와 harbormaster.1에 실제 Gameplay 배치
좌표를 사용한다. 물 안내는 현재 mapwater가 참조하는 WATER01 mesh의 실제 authoring 배치
[107.434014,35.9760303,-28.5882275]를 중심으로 준비한다. 해당 문구는 각각 수리·승선·
생명의 나무 안내이며 발탄/쿠크 귀환 문구는 사용자 지정 원문을 저장한다.

F1 Debug의 DimensionMaster Guide는 저장 대사·trigger를 선택하고 바닥을 한 번 picking해
box 중심을 지정한다. half extents/yaw 조절과 화면 preview, stable ID 참조, Save/Publish의
기존 CAS·원자 교체·실패 rollback을 재사용한다. Tool draft·디스크·게시본·Server 적용은 구분한다.

Server guide 전용 계약에서 단일 actor/시작·종료/다른 사용자 선점 차단/인사/공간재진입/승선대기/용추종/실패보존/실제4인·solo
raid귀환/PvP·섬귀환/disconnect를 검사한다. publisher는 신규 event·잘못된 world/category와
동시 저장 보존을 검사한다. 최종 Debug/Release Product와 제품 UI 사용자 확인을 분리한다.

## G09. 콜라이더 편집 진입과 Show Debug (2026-10-01)

현재 GUIDE_STARTED 선택에는 공간 정보가 없고, SPACE_ENTER의 위치·half extents·회전과
preview는 긴 트리거 표 아래에서만 접근할 수 있다. CGuideAITool의 기존 draft와 world
picking을 유지하며 콜라이더 전용 탭에서 공간 트리거만 고르고 같은 편집기를 사용한다.
목록 높이를 제한하고 행을 선택하면 별도 `Collider Detail` ImGui 창을 연다.
상세 창에서 위치·전체 크기·Y 회전과 대사 연결을 조절하고 Save/Publish를 실행한다.
전체 크기는 미터 단위로 표시하고 기존 halfExtents 저장값으로 절반 변환한다.
NPC anchor는 위치를 복사하는 기존 선택 기능이며 직접 위치·회전을 바꾸면 참조를 해제한다.

GuideAITool.h의 Render_Triggers에 공간 목록 필터와 상세 창 열기를 추가하고,
Render_ColliderDetail은 선택 stable trigger ID의 같은 draft를 편집한다. 별도 debug 표시 함수가
현재 Area의 모든 SPACE_ENTER 또는 선택한 항목을 그린다. Show Debug는 탭 바깥에 두고,
선택 항목·다른 활성 항목·비활성 항목의 색과 box ID를 구분한다. 다른 탭이나 접힌 창에서도
미리보기를 유지하고 도구를 닫으면 끈다. 미리보기는 현재 draft이며 Server 적용 증거가 아니다.
카메라 near plane을 가로지르는 박스 선분은 clip한 뒤 투영한다.

수정 소스는 기존 Client/Public/GuideAITool.h와 Client/Private/GuideAITool.cpp다.
새 C++ 파일·project/filter 등록·JSON schema·Server 판정·publisher 변경은 없다.
기존 Save의 stable ID 병합과 원자 교체, Publish의 저장본 소비와 실패 보존을 유지한다.
Debug Product 증분 빌드, Guide의 기존 저장/게시 회귀, scoped diff-check를 확인한다.
Client 실행·화면 확인은 사용자가 F1 → DimensionMaster Guide → 콜라이더에서 수행한다.

## G10. 기능 NPC 안내 위치와 가이드 상태 회귀 수정 (2026-10-01)

사용자의 수정 승인에 따라 안내 시작은 `guide.bern.party.first_invite`로 복원한다.
사용자가 만든 아바타 대사와 빈 trigger의 stable ID는 유지하고, 빈 trigger를
`npc.bern.plaza.17` 중심의 SPACE_ENTER로 연결한다. 기존 베다 레이드 안내를 보존하며
아일라라에도 레이드 안내를 추가한다. 물약 상점과 PvP 입장 안내도 실제 기능 NPC의
Gameplay position/yaw를 복사한 OBB로 추가한다. 제련·수리·항구·생명의 물 박스와
발탄·쿠크·워터팡·콜로세움 귀환 대사는 유지한다.

`Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json`에서는 수리 NPC 옆의
`npc.bern.plaza.05`를 유지하고 나머지 물약 상인 배치 9개는 비활성화한다.
`Data/Items/ItemCatalog.json`의 `shop.bern.potion`도 같은 1개 placement만 참조한다.
기존 배치 ID·외형·좌표는 삭제하거나 재배치하지 않는다. Guide·World(BERN)·Item의
기존 publisher로 각각 검증·게시하고 관련 Client/Server 실행 데이터만 함께 전달한다.

Server의 공간 안내 큐는 현재 promptId와 segment index만 저장하여 NPC 구역을 떠난 뒤에도
대사가 재생된다. `GameRoom.h`와 `GameRoom_Guide.cpp`의 기존 큐에 공간 trigger 출처를
보관하고, 아직 시작하지 않은 공간 대사는 전송 직전 실제 owner가 해당 박스 안인지 확인한다.
시작 인사·실제 월드 귀환·이미 시작한 여러 segment는 이 공간 취소 조건과 구분한다.
추가로 시작/종료·중복 요청·승선 대기·귀환·연결 종료와 Client 대사 소비 경로를 검토하여
재현되는 결함만 같은 기존 계약 안에서 수정한다. 새 runtime이나 protocol을 만들지 않는다.

검증은 실제 저장본과 게시본의 NPC/상점/공간 연결 대조, 기존 publisher 회귀,
`ServerGameplayContractTests_Guide.cpp`의 공간 이탈·정상 체류 회귀 및 기존 시작/종료·
귀환 계약을 사용한다. C++ 변경은 해당 제품 Debug/Release 빌드로 확인한다.
실행 중 Client/Server를 자동 종료하지 않으며 EXE 잠금, 디스크 게시와 실행 중 메모리,
사용자 화면 확인은 RESULT에서 구분한다. 새 C++ 파일이 없으면 project/filter 추가도 없다.

독립 검토에서 확인된 미니맵의 비활성 NPC 표시도 `Client/Private/MinimapView.cpp`의
`Load_AreaNpcSymbols`에서 enabled=false를 제외하여 바로잡는다. 비활성 물약 NPC를 바라보던
ambient lookTarget 참조는 null로 정리하되 나머지 행동과 배치를 보존한다.
Guide publisher는 일회 복사 anchor의 enabled NPC/좌표·회전 일치를 저장·게시 전에 검사한다.
편집용 Validate는 잘못된 anchor와 빈 활성 trigger를 경고하여 도구에서 고칠 수 있게 하고,
Save/Publish/CheckPublished는 거부한다. 자동 좌표 덮어쓰기는 추가하지 않는다.
관련 임시 fixture에서 실패 시 저작·게시본 보존과 복구 후 정상 게시를 검증한다.

`PartyInteractionView.h/.cpp`는 서버 시작 조건과 같은 XZ 거리10m를 시작 메뉴에 표시한다.
멀리서는 가까이 가서 시작하도록 비활성화하며 owner의 종료는 원거리에서도 유지한다.
접촉 초기화는 안내 시작·월드 귀환에 한정한다. `Guide_AnchorArrived`와 가이드 부활은
owner의 기존 접촉 기록을 보존하여 Guide만 이동한 경우의 가짜 진입을 막되 실제 스퀘어홀
박스 진입은 허용한다. 같은 대사를 공유하는 중첩 박스는 실제 발생한 출처 후보를 유지하여
한 박스만 벗어나도 다른 박스에 계속 있으면 안내가 한 번 전달되게 한다.

## G11. 베른 건물 동행과 가이드 착지 연결성 (2026-10-02)

사용자가 요청한 도서관·성 출입을 기존 스퀘어홀 도착 계약에 연결한다. Server가 실제
authored move의 hold와 이동을 끝낸 뒤 같은 Guide actor를 옮긴다. 배 승선 대기와
다른 world 귀환의 도보 접근은 유지한다. 지형 nav 자체의 누락·고립은 별도 복구 대상이다.

Find_GuideLanding의 optional anchor는 local travel에서만 연결성 기준을 제공한다.
모든 후보는 exact walkability·높이·충돌·다른 player 겹침을 확인하며, local 후보는
owner와 직접 걸을 수 있는 같은 nav 경로를 요구한다. 뒤쪽 offset 후보가 없으면 owner
주변을 검사한다. 후보가 없으면 guide 위치·action·goal·combo를 교체하지 않는다.

GameRoom_PlayerSimulation은 Bern의 실제 authored move가 완료됐을 때만 localMapTravel을
전달한다. Guide 계약은 실제 네 building trigger의 진입·hold·완료와 항구24방향·착지 실패
보존, 기존 스퀘어홀·승선·world transfer 회귀를 검증한다. 신규 파일·protocol·project/filter
등록·저작 JSON 변경은 없다. Debug와 Release에 공통으로 컴파일되는 Server 코드다.

빌드는 기존 Product Debug/Release를 사용한다. 실행 중 게임은 종료하지 않고 링크 잠금이
풀린 뒤 반영한다. 새 Server의 --guide-ai-contract-test 및 사용자 건물 왕복·출항 준비를
구분하여 RESULT에 기록한다. 팀 사용서의 local travel 계약을 같은 변경에서 갱신한다.

### G11. Server/Public/GameRoom.h 전체 코드

```cpp
#pragma once

#include "RoomCommand.h"
#include <atomic>
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"
#include "WorldBootstrap.h"
#include "GameplayCatalog.h"
#include "ItemCatalog.h"
#include "HonorTitleCatalog.h"
#include "VehicleCatalog.h"
#include "GuideCatalog.h"
#include "ValtanClearRewards.h"
#include "PlayerSkillSystem.h"
#include "CombatObjectRuntime.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ServerTriggerSystem.h"
#include "SpawnGroupBootstrap.h"
#include "SpawnGroupRuntime.h"
#include "MonsterBrain.h"
#include "NpcBehaviorRuntime.h"
#include "ValtanBrain.h"
#include "KoukuSaydonBrain.h"
#include "KoukuSaydonLogicRuntime.h"
#include "EncounterPropRuntime.h"
#include "EstherSkillSystem.h"
#include "Gameplay/EstherStrikeContract.h"
#include "Gameplay/MaharakaWaterpangContract.h"
#include "WorldDestructionBootstrap.h"
#include "WorldDestructionRuntime.h"
#include "Network/PacketFrame.h"
#include "Network/SessionDiagnostic.h"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace LostArk::Server
{
	class CServerGameplayContractRunner;

	class CClientSession;

	/* A room never mutates an admitted gameplay catalog. The facade preserves
	   the established lookup surface while retaining immutable old generations
	   until every replicated occurrence releases its revision pin. */
	class CGameplayCatalogGenerations final
	{
	public:
		static constexpr std::size_t MAX_GENERATION_COUNT = 16u;

		CGameplayCatalogGenerations();
		bool Load();
		bool Initialize(
			const std::shared_ptr<const CGameplayCatalog>& initialGeneration);
		bool Stage(
			std::uint32_t transactionSequence,
			const LostArk::Shared::GameplayDataRevision& baseRevision,
			const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
			std::string& status);
		bool Commit(std::uint32_t transactionSequence) noexcept;
		bool Stage_NumericBalance(std::uint32_t transactionSequence,
			const std::shared_ptr<const CGameplayCatalog>& candidate, std::string& status);
		bool Commit_NumericBalance(std::uint32_t transactionSequence) noexcept;
		void Abort_NumericBalance(std::uint32_t transactionSequence) noexcept;

		void Abort(std::uint32_t transactionSequence) noexcept;
		void Collect_Garbage(
			const std::vector<LostArk::Shared::GameplayDataRevision>& livePins);

		[[nodiscard]] const CGameplayCatalog* Resolve(
			const LostArk::Shared::GameplayDataRevision& revision) const noexcept;
		[[nodiscard]] const CGameplayCatalog& Active() const noexcept;
		[[nodiscard]] std::shared_ptr<const CGameplayCatalog>
			Get_ActiveGeneration() const noexcept { return m_pActiveGeneration; }
		[[nodiscard]] std::size_t Get_GenerationCount() const noexcept
		{
			return m_Generations.size();
		}
		[[nodiscard]] std::uint16_t Get_ActiveGenerationEpoch() const noexcept
		{
			return m_iActiveGenerationEpoch;
		}

		const PLAYER_SKILL_DEFINITION* Find_Skill(
			LostArk::Shared::SKILL_ID skillId) const;
		const BOSS_RUNTIME_PROFILE* Find_Boss(
			const std::string& archetypeId) const;
		const std::vector<BOSS_PART_DEFINITION>* Find_BossParts(
			const std::string& archetypeId) const;
		const std::vector<BOSS_PATTERN_DEFINITION>* Find_BossPatterns(
			const std::string& encounterId) const;
		const BOSS_COMBAT_OBJECT_DEFINITION* Find_BossCombatObject(
			const std::string& archetypeId) const;
		const VALTAN_TIMELINE_DEFINITION* Find_ValtanTimeline(
			const std::string& encounterId) const;
		const VALTAN_TIMELINE_ROW* Find_ValtanTimelineRow(
			const std::string& encounterId, std::uint32_t commandId) const;
		const BOSS_PATTERN_ROTATION_DEFINITION* Find_BossPatternRotation(
			const std::string& encounterId, std::uint32_t gameplayPhase,
			std::uint32_t healthBar) const;
		const std::string& Find_IntroPatternId(
			const std::string& encounterId) const;
		const PLAYER_RUNTIME_PROFILE* Find_Player(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass) const;
		std::uint32_t Find_DamageRatePercent(
			const std::string& damageProfileId) const;
		[[nodiscard]] const LostArk::Shared::GameplayDataRevision&
			Get_ActiveRevision() const noexcept;
		[[nodiscard]] const std::string& Get_Status() const noexcept;
		operator const CGameplayCatalog&() const noexcept { return Active(); }

	private:
		std::shared_ptr<const CGameplayCatalog> m_pActiveGeneration;
		std::shared_ptr<const CGameplayCatalog> m_pStagedGeneration;
		std::uint32_t m_iStagedTransactionSequence = 0u;
		std::uint16_t m_iActiveGenerationEpoch = 0u;
		std::vector<std::shared_ptr<const CGameplayCatalog>> m_Generations;
		std::vector<std::shared_ptr<const CGameplayCatalog>> m_NumericStagedGenerations;
		std::shared_ptr<const CGameplayCatalog> m_pNumericStagedActive;
		std::uint32_t m_iNumericTransactionSequence = 0u;

		std::string m_strStatus;
	};

	// The last completed outer room-loop iteration, shared by all rooms on the next tick.
	struct SERVER_ROOM_SCHEDULER_METRICS final
	{
		std::uint64_t iSampleUnixMilliseconds = 0u;
		std::uint64_t iPreviousLoopLatenessMicroseconds = 0u;
		std::uint64_t iMaximumLoopLatenessMicroseconds = 0u;
		std::uint64_t iScheduleResetCount = 0u;
	};

	struct SERVER_ROOM_PERFORMANCE_METRICS final
	{
		SERVER_NAVIGATION_PERFORMANCE_METRICS Navigation;
		SERVER_ROOM_SCHEDULER_METRICS Scheduler;
		std::uint64_t iTickCount = 0;
		std::uint64_t iLastTickMicroseconds = 0;
		std::uint64_t iMaximumTickMicroseconds = 0;
		std::size_t iLastIngressDepth = 0;
		std::size_t iIngressHighWatermark = 0;
		std::size_t iLastDrainedCommandCount = 0;
		std::size_t iLastRemainingCommandCount = 0;
		std::size_t iLastCleanupIngressDepth = 0;
		std::size_t iCleanupIngressHighWatermark = 0;
		std::size_t iLastDrainedCleanupCommandCount = 0;
		std::size_t iLastRemainingCleanupCommandCount = 0;
		std::uint64_t iDrainLimitedTickCount = 0;
		std::uint64_t iCoalescedMoveCommandCount = 0;
		std::uint64_t iCoalescedAimCommandCount = 0;
		std::uint64_t iDroppedBestEffortCommandCount = 0;
		std::uint64_t iRejectedReliableCommandCount = 0;
		std::uint64_t iRejectedCleanupCommandCount = 0;
		std::uint64_t iDeduplicatedCleanupCommandCount = 0;
		std::uint64_t iCancelledCommandCountByCleanup = 0;
		std::uint64_t iSnapshotEncodeCount = 0;
		std::uint64_t iSnapshotEncodeFailureCount = 0;
		std::uint64_t iLastSnapshotEncodeMicroseconds = 0;
		std::uint64_t iMaximumSnapshotEncodeMicroseconds = 0;
		std::uint64_t iSnapshotEnqueueBatchCount = 0;
		std::uint64_t iSnapshotRecipientCount = 0;
		std::uint64_t iSnapshotEnqueueFailureCount = 0;
		std::uint64_t iLastSnapshotEnqueueMicroseconds = 0;
		std::uint64_t iMaximumSnapshotEnqueueMicroseconds = 0;
		std::uint64_t iLastMaximumSessionEnqueueMicroseconds = 0;
		std::uint64_t iMaximumSessionEnqueueMicroseconds = 0;
	};

	/* Receive-thread admission is not a bool: a full reliable queue, a room
	   runtime failure, a sealed private arena, and cleanup already in flight
	   require different session policy and diagnostics.  Keep success values
	   explicit too so best-effort shedding remains non-terminal. */
	enum class ROOM_COMMAND_ENQUEUE_RESULT : std::uint8_t
	{
		ACCEPTED,
		DROPPED_BEST_EFFORT,
		DEDUPLICATED_CLEANUP,
		REJECTED_INVALID_COMMAND,
		REJECTED_ROOM_NOT_READY,
		REJECTED_ROOM_SEALED,
		REJECTED_PENDING_CLEANUP,
		REJECTED_RELIABLE_CAPACITY,
		REJECTED_BINDING_MISSING
	};

	[[nodiscard]] constexpr bool Is_AcceptedRoomCommandEnqueueResult(
		const ROOM_COMMAND_ENQUEUE_RESULT result) noexcept
	{
		return ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED == result ||
			ROOM_COMMAND_ENQUEUE_RESULT::DROPPED_BEST_EFFORT == result ||
			ROOM_COMMAND_ENQUEUE_RESULT::DEDUPLICATED_CLEANUP == result;
	}

	struct SERVER_ROOM_RUNTIME_FAILURE final
	{
		std::uint32_t iServerTick = 0u;
		std::string strSource;
		std::string strDetail;
	};

	class CGameRoom final
	{
		friend class CServerGameplayContractRunner;
		friend int Run_ServerKoukuSupportSurfaceContractTests();
        friend int Run_ServerBingoContractTests();
		friend int Run_ServerCardMazeContractTests();
        friend int Run_ServerKoukuObjectOverlapContractTests();
		friend int Run_ServerVehicleRidingContractTests();
	public:
		explicit CGameRoom(
			LostArk::Shared::WORLD_ID worldId,
			std::shared_ptr<const CGameplayCatalog> initialGameplayGeneration = {},
			const std::atomic_bool* pPreparationCancelled = nullptr);

		bool Enqueue(ROOM_COMMAND command);
		[[nodiscard]] ROOM_COMMAND_ENQUEUE_RESULT Enqueue_Detailed(
			ROOM_COMMAND command);
		[[nodiscard]] std::string Describe_EnqueueResult(
			ROOM_COMMAND_ENQUEUE_RESULT result) const;
		[[nodiscard]] bool Try_GetRuntimeFailure(
			SERVER_ROOM_RUNTIME_FAILURE& outFailure) const;
		void Tick(float fixedDeltaSeconds,
			const SERVER_ROOM_SCHEDULER_METRICS& schedulerMetrics = {});
		// Room-thread only. At most one sampled line is retained until ServerApp writes it.
		[[nodiscard]] std::string Take_PerformanceDiagnostic();
		bool Try_DequeueWorldTransfer(
			SERVER_WORLD_TRANSFER_REQUEST& outTransfer);
		bool Configure_ColosseumMatch(std::uint64_t matchId, const std::vector<SESSION_ID>& sessions);
		void Remove_ColosseumExpectedSession(SESSION_ID sessionId);

		[[nodiscard]] LostArk::Shared::WORLD_ID Get_WorldId() const
		{
			return m_eWorldId;
		}

		[[nodiscard]] bool Is_Ready() const { return m_isReady; }
		[[nodiscard]] const std::string& Get_Status() const
		{
			return m_strStatus;
		}
		/* Room-thread only. Stage is allowed to fail before publication; Commit
		   is a bounded pointer swap after every process room has staged. */
		bool Stage_NumericBalance(std::uint32_t transactionSequence,
			const std::shared_ptr<const CGameplayCatalog>& candidate, std::string& status);
		bool Commit_NumericBalance(std::uint32_t transactionSequence) noexcept;
		void Abort_NumericBalance(std::uint32_t transactionSequence) noexcept;
		bool Stage_GameplayGeneration(
			std::uint32_t transactionSequence,
			const LostArk::Shared::GameplayDataRevision& baseRevision,
			const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
			std::string& status);
		bool Commit_GameplayGeneration(
			std::uint32_t transactionSequence) noexcept;
		void Abort_GameplayGeneration(
			std::uint32_t transactionSequence) noexcept;
		[[nodiscard]] std::shared_ptr<const CGameplayCatalog>
			Get_ActiveGameplayGeneration() const noexcept
		{
			return m_GameplayCatalog.Get_ActiveGeneration();
		}
		[[nodiscard]] const CGameplayCatalog* Resolve_GameplayGeneration(
			const LostArk::Shared::GameplayDataRevision& revision) const noexcept
		{
			return m_GameplayCatalog.Resolve(revision);
		}
		/* Room-thread only. Decision observability reads the same authoritative
		   brain and immutable selector generation as the Valtan simulation. */
		bool Build_ValtanDecisionTraceResponse(
			const LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY& request,
			LostArk::Shared::S2C_VALTAN_DECISION_TRACE_RESPONSE& outResponse,
			std::string& status) const;
		[[nodiscard]] SERVER_ROOM_PERFORMANCE_METRICS
			Get_PerformanceMetrics() const;

		// Room thread only. A sealed private arena rejects every later command.
		[[nodiscard]] bool Try_SealPrivateArenaForRetirement();
		// Room thread only. Removes the source player before the target room can
		// process its queued ENTER_WORLD and bind the same session again.
		[[nodiscard]] bool Commit_WorldTransferDeparture(SESSION_ID sessionId);
		// Room-thread only, while ServerApp holds its session-binding mutex.
		[[nodiscard]] bool Has_PersonalGuideOwner(const std::shared_ptr<CClientSession>& ownerSession) const;
		bool Transfer_PartyTo(CGameRoom& target,
			const std::vector<SESSION_ID>& leaderFirstSessionIds,
			LostArk::Shared::PARTY_TRANSFER_RESULT& outResult, std::string& status,
			const std::string& raidReturnNpcPlacementId = {},
            const std::string& spawnPlacementOverrideId = {});
		bool Transfer_ColosseumMatchTo(CGameRoom& target,
			const std::vector<SESSION_ID>& orderedSeats, std::uint64_t matchId, std::string& status);
		void Notify_ColosseumTransferResult(bool committed);
		[[nodiscard]] bool Try_SealColosseumForRetirement();
		void Notify_PartyTransferFailure(SESSION_ID sessionId,
			std::uint32_t requestSequence, LostArk::Shared::WORLD_ID targetWorldId,
			LostArk::Shared::PARTY_TRANSFER_RESULT result);

	private:
		void Mark_RuntimeFailure(std::string_view source);
		std::size_t Count_HumanPlayers() const;
		void Initialize_Guide();
		bool Build_GuidePlayer(LostArk::Shared::PLAYER_ID playerId, LostArk::Shared::NET_ENTITY_ID entityId,
			float x, float y, float z, SERVER_PLAYER& outPlayer) const;
		bool Find_GuideLanding(const SERVER_PLAYER& guide, float x, float y, float z, SERVER_NAV_POINT& point,
			const SERVER_PLAYER* anchor = nullptr) const;
		bool Start_Guide(const SERVER_PLAYER& inviter, LostArk::Shared::NET_ENTITY_ID target);
		void Handle_GuideControl(SESSION_ID sessionId, const LostArk::Shared::C2S_GUIDE_CONTROL& request);
		void Broadcast_GuideOwnership();
		void Broadcast_GuideState(const LostArk::Shared::S2C_GUIDE_STATE& message);
		void Update_Guides(float seconds);
		void Update_Colosseum(float seconds);
		void Handle_ColosseumRecruit(SESSION_ID sessionId, const LostArk::Shared::C2S_COLOSSEUM_RECRUIT& request);
		LostArk::Shared::S2C_COLOSSEUM_MATCH_STATE Build_ColosseumState() const;
		void Broadcast_ColosseumState();
        float Predict_GuideContactRisk(const SERVER_PLAYER& guide, float x, float z);
		void Guide_AnchorArrived(const SERVER_PLAYER& anchor, bool localMapTravel = false);
		void Guide_ChatCommand(const SERVER_PLAYER& sender, const std::string& text);
		void Remove_Guide(SESSION_ID ownerSessionId, bool publish = true);
		void Suspend_PersonalGuide(SESSION_ID ownerSessionId);
		void Resume_PersonalGuide(SESSION_ID ownerSessionId, LostArk::Shared::WORLD_ID sourceWorld);
		void Seed_GuideSpaceEntries(SESSION_ID ownerSessionId, const SERVER_PLAYER& anchor);
		void Prune_GuideSpacePrompts(SESSION_ID ownerSessionId);
		void Queue_GuidePrompt(SESSION_ID ownerSessionId, const GUIDE_TRIGGER& trigger);
		void Execute_PlayerMove(SERVER_PLAYER& player, const LostArk::Shared::C2S_MOVE& move);
		bool Execute_PlayerSkill(SERVER_PLAYER& player, const LostArk::Shared::C2S_USE_SKILL& skill);
		struct STAGED_PLAYER_ENTRY final
		{
			std::shared_ptr<CClientSession> pSession;
			SERVER_PLAYER Player;
			std::vector<LostArk::Shared::PACKET_FRAME> Frames;
		};
		bool Stage_PlayerEntry(const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
			std::span<const STAGED_PLAYER_ENTRY> precedingEntries,
			STAGED_PLAYER_ENTRY& staged,
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON& outReason, std::string& status,
			const std::string& spawnPlacementOverrideId = {},
			const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>&
				carriedInventory = {},
			LostArk::Shared::HONOR_TITLE_ID carriedHonorTitleId =
				LostArk::Shared::INVALID_HONOR_TITLE_ID,
			const std::string& raidReturnNpcPlacementId = {},
			const SERVER_PURSE& carriedPurse = {});
		bool Build_PlayerEntryFrames(STAGED_PLAYER_ENTRY& entry,
			std::span<const STAGED_PLAYER_ENTRY> batch, std::string& status);
		void Commit_PlayerEntry(const STAGED_PLAYER_ENTRY& entry);
		void Flush_PartyTransferResults();
		void Handle_Register(const std::shared_ptr<CClientSession>& session);
		bool Join(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
			const std::string& spawnPlacementOverrideId = {},
			const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>&
				carriedInventory = {},
			LostArk::Shared::HONOR_TITLE_ID carriedHonorTitleId =
				LostArk::Shared::INVALID_HONOR_TITLE_ID,
			const std::string& raidReturnNpcPlacementId = {},
			const SERVER_PURSE& carriedPurse = {},
			LostArk::Shared::WORLD_ID sourceWorld = LostArk::Shared::WORLD_ID::BERN);
		void Leave(
			SESSION_ID sessionId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason, bool publishDeparture = true);
		void Close_SessionForBindingFailure(
			SESSION_ID sessionId,
			std::string_view packetName,
			std::string_view validation);
		void Handle_Move(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_MOVE& move);
		[[nodiscard]] bool Is_BufferableComboAction(
			const SERVER_PLAYER& player) const;
		/* A move goal inside the running skill's authored move-cancel window
		ends the action now instead of waiting out the recovery pose. */
		[[nodiscard]] bool Is_MoveCancellableAction(
			const SERVER_PLAYER& player) const;
		[[nodiscard]] bool Commit_MoveGoal(
			SERVER_PLAYER& player, float goalX, float goalZ);
		void Commit_PendingPlayerCommand(
			SERVER_PLAYER& player, std::uint32_t actionStartTick);
		void Handle_UseSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_SKILL& useSkill);
		void Handle_ReleaseSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RELEASE_SKILL& releaseSkill);
		void Handle_UpdateSkillAim(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_UPDATE_SKILL_AIM& updateSkillAim);
		void Handle_UseEstherSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_ESTHER_SKILL& useEstherSkill);
		/* Debug F1 Esther summon by name: same caster lock and summon timeline as
		the slot path, without the gauge or the world roster. Release ignores it. */
		void Handle_DebugUseEsther(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_USE_ESTHER& request);
		/* The caster the session owns, if it may start an Esther call right now:
		bound, idle, on its feet and not riding. */
		SERVER_PLAYER* Find_EstherCaster(SESSION_ID sessionId, const char* pCommandName);
		/* Queues the summon forward along the aim and locks the caster into
		ESTHER_CAST. The gauge decision is the caller's. */
		void Begin_EstherCall(
			SERVER_PLAYER& caster,
			const ESTHER_ROSTER_ENTRY& rosterEntry,
			float aimX,
			float aimZ);
		void Handle_UseSquareHole(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_SQUAREHOLE& useSquareHole);
		/* The landing of a square hole: the world's disabled "squarehole.<id>" movePlayer
		   row, admitted with the debug-teleport ground/height/collision rules. False when
		   the world has no such row or the landing is not standable for this player. */
		bool Resolve_SquareHoleDestination(
			const SERVER_PLAYER& player,
			std::uint16_t squareHoleId,
			SERVER_NAV_POINT& ground);
		/* The song lock ended: land the player at the destination, or leave them in place. */
		void Finish_SquareHoleSong(SERVER_PLAYER& player);
		bool Spawn_EstherSummon(
			const ESTHER_ROSTER_ENTRY& rosterEntry,
			LostArk::Shared::PLAYER_ID casterPlayerId,
			float positionX,
			float positionY,
			float positionZ,
			float yawDegrees);
		void Update_PendingEstherSummons(float fixedDeltaSeconds);
		void Apply_EstherStrikeHits(SERVER_WORLD_ENTITY& summon, std::uint32_t serverTick);
		void Open_EstherZone(const LostArk::Shared::EstherStrike::ZONE& zone, float positionX, float positionZ, std::uint32_t serverTick);
		void Update_EstherZones(std::uint32_t serverTick);
		void Handle_RevivePlayer(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_REVIVE_PLAYER& revivePlayer);
		/* Debug/Development-build test aid only -- zeroes the caster's own HP and
		sets PLAYER_ACTION_STATE::DEAD so a death-screen tester does not have to
		survive a real hit. Real body is compiled out in Release, matching
		Evaluate_ValtanAudition's convention. */
		void Handle_DebugKillSelf(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_KILL_SELF& debugKillSelf);
		/* Debug-only Character Select audition entry. This stages the ordinary
		Server world-transfer transaction; it never changes a Client level directly. */
		void Handle_DebugEnterKakulSaydonArena(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_ENTER_KAKULSAYDON_ARENA& request);
		/* Debug-only authored waypoint audition. The placement must be a Kakul
		playerSpawn waypoint and Server navigation remains the position authority. */
		void Handle_DebugTeleportToPlacement(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_PLACEMENT& request);
		void Handle_DebugTeleportToPosition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request);
		LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT Apply_DebugTeleportToPosition(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request);
		LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT Apply_DebugReturnToKoukuStart(
			SERVER_PLAYER& player, std::uint32_t requestSequence);
		void Reset_PlayerForDebugTeleport(SERVER_PLAYER& player);
		void Handle_DebugMarioJump(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_MARIO_JUMP& request);
		LostArk::Shared::S2C_DEBUG_MARIO_JUMP_RESULT Apply_DebugMarioJump(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_MARIO_JUMP& request);
		void Handle_MarioMove(SESSION_ID sessionId, const LostArk::Shared::C2S_MARIO_MOVE& request);
		void Handle_MarioReturn(SESSION_ID sessionId, const LostArk::Shared::C2S_MARIO_RETURN& request);
		LostArk::Shared::S2C_MARIO_RETURN_RESULT Apply_MarioReturn(
			SERVER_PLAYER& player, const LostArk::Shared::C2S_MARIO_RETURN& request);
		bool Resolve_MarioReturnDestination(const SERVER_PLAYER& player, SERVER_NAV_POINT& destination) const;
		static void Reset_MarioContactAction(SERVER_PLAYER& player);
		SERVER_TRIGGER_MOVE_ENTRY_RESULT Begin_MarioTriggerMove(
			const WORLD_BOOTSTRAP_PLACEMENT& trigger, SERVER_PLAYER& player,
			std::uint32_t actionStartTick);
		void Update_MarioControlState(SERVER_PLAYER& player);
		std::uint8_t Begin_MarioStageObjects(std::uint8_t stage);
		void Begin_MarioBallChallenge(SERVER_PLAYER& player);
		std::uint8_t Mario_MatchingBallCount(const SERVER_PLAYER& player) const;
		std::uint8_t Mario_MarkerColor(LostArk::Shared::NET_ENTITY_ID targetId) const;
		void Reset_MarioStageObjects(std::uint8_t stage);
		void Cleanup_EmptyMarioStages();
		void Update_MarioMoveGoal(SERVER_PLAYER& player, std::uint32_t updateTick);
		bool Configure_MarioRail(SERVER_PLAYER& player, const std::string& arrivalPlacementId);
		/* Debug F1 clown/player avatar toggle: swaps only the replicated
		madness form of this session's player; Release answers REJECTED_DISABLED. */
		/* Debug F1 bingo check: paints cells and promotes completed lines. The
		board replicates on the world snapshot, so there is no result message. */
		void Handle_DebugBingoFill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_FILL& request);
		/* Debug F1 bingo bomb: marks this session's own player. The bomb
		rides the world snapshot, so there is no result message. */
		void Handle_DebugBingoBomb(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_BOMB& request);
		/* Debug F1 bingo hammer: rolls one of the twenty row/column anchors
		and starts the sweep. The hammer rides the world snapshot. */
		void Handle_DebugBingoHammer(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_HAMMER& request);
		/* Debug F1 "Normal Monster 1/2" (Kouku Book1/Book2, Valtan Stage_1/Stage_2):
		removes the mapped wave group's live monsters, resets the group and starts it
		over at its authored anchors. Release ignores it; the monsters ride the world
		snapshot, so there is no result message. */
		void Handle_DebugResummonWaveMonsters(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_RESUMMON_WAVE_MONSTERS& request);
		void Handle_DebugSetMadnessForm(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM& request);
		LostArk::Shared::S2C_DEBUG_SET_MADNESS_FORM_RESULT Apply_DebugMadnessForm(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM& request);
		/* H key riding toggle for this session's player. The verdict is sent
		back; the ridden vehicle itself rides the world snapshot. */
		void Handle_SetVehicleRiding(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request);
		LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT Apply_SetVehicleRiding(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request);
		/* True while nothing the player is doing forbids a vehicle underneath. */
		bool Can_RideVehicle(const SERVER_PLAYER& player) const;
		/* Title window change for this session's player. The verdict is sent back; the
		worn title itself rides the world snapshot. */
		void Handle_SetHonorTitle(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_HONOR_TITLE& request);
		LostArk::Shared::S2C_SET_HONOR_TITLE_RESULT Apply_SetHonorTitle(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_HONOR_TITLE& request);
		/* Metres per second the player walks at: the ridden vehicle's speed, or
		the class speed scaled by its held stance. */
		float Resolve_PlayerMoveSpeed(const SERVER_PLAYER& player) const;
		/* Dismounts every player the world, catalog or current state no longer
		lets ride. Runs once per tick before the snapshot is committed. */
		void Enforce_VehicleRidingState();
		/* Bern voyage ships (EFTable_VoyageShip 8200..8208) sail on the BernSea navigation region. Boarding
		   moves the player to the nearest open sea cell and keeps the pier position; leaving the ship, or any
		   forced dismount, brings the player back to that pier position. Begin returns false when no sea cell
		   lies within reach (the player is not at a harbour). */
		bool Begin_ShipVoyage(SERVER_PLAYER& player, LostArk::Shared::VEHICLE_ID vehicleId);
		void End_ShipVoyage(SERVER_PLAYER& player, const char* reason);
		/* A skill press while mounted. Only a skill of the ridden vehicle starts,
		from an idle mount, off cooldown and with a newer sequence; it faces the
		player's current yaw. */
		bool Try_StartVehicleSkill(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_USE_SKILL& command);
		/* Advances a running vehicle skill one fixed tick: authored root motion is
		clamped to walkable ground and collision, and the action ends at its length. */
		void Update_VehicleSkill(SERVER_PLAYER& player, float fixedDeltaSeconds);
		void End_VehicleSkill(SERVER_PLAYER& player);
		/* One quick-slot press while this session's player shows an interaction
		HUD. DANCE answers the open pose window; the other modes only record the
		press until their skills own a Server judgement. */
		void Handle_InteractionSlot(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_INTERACTION_SLOT& request);
		/* Debug F1 "HUD Mode: MARIO / MAZE / Clear": forces one of the two
		modes whose gimmick has no Server trigger yet. Release answers
		REJECTED_DISABLED. */
		void Handle_DebugSetKoukuHudMode(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_SET_KOUKU_HUD_MODE& request);
		LostArk::Shared::S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT Apply_DebugKoukuHudMode(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_SET_KOUKU_HUD_MODE& request);
		/* Every tick: madness maximum from the encounter policy, clown hold
		expiry, and the interaction HUD mode plus slot layout per player. */
		void Update_KoukuPlayerModes(std::uint32_t serverTick);
		void Apply_KoukuGateEntryCard(SERVER_PLAYER& player, const SERVER_WORLD_ENTITY& boss);
		void Handle_ChangeCharacterClass(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request);
		LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT Apply_CharacterClassChange(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request);
		void Handle_SpawnWorldEntity(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SPAWN_WORLD_ENTITY& request);
		/* Debug-only. Moves a live Valtan onto an authored health-bar threshold
		so CValtanBrain judges the crossing itself on a later fixed tick. The
		room never starts a pattern, breaks a wall or plays a cue directly.
		Evaluate owns the decision and the boss mutation and is what the contract
		tests drive; Handle only resolves the session and answers it. */
		LostArk::Shared::VALTAN_AUDITION_RESULT Evaluate_ValtanAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			std::uint32_t& outCurrentHealthBar);
		void Handle_ValtanAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request);
		LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT
			Evaluate_ValtanPatternFlowStart(
				SESSION_ID sessionId,
				const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request,
				std::uint32_t& outRoomFlowEpoch,
				LostArk::Shared::GameplayDataRevision& outPinnedRevision,
				std::string& outReason);
		LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT
			Evaluate_ValtanPatternFlowStopAfterCurrent(
				SESSION_ID sessionId,
				const LostArk::Shared::
					C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& request,
				std::uint32_t& outRoomFlowEpoch,
				LostArk::Shared::GameplayDataRevision& outPinnedRevision,
				std::string& outReason);
		void Handle_ValtanPatternFlowStart(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request);
		void Handle_ValtanPatternFlowStopAfterCurrent(
			SESSION_ID sessionId,
			const LostArk::Shared::
				C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& request);
		void Handle_KoukuRaidRequest(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request);
		bool Is_KoukuRaidInputBlocked() const;
		struct KOUKU_RAID_RUN final
		{
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST Request;
			LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE State;
			std::shared_ptr<const CGameplayCatalog> pCatalog;
			std::optional<LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST> PriorAuditionRequest;
			std::optional<LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT> PriorAuditionResult;
			std::optional<LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE> PriorAuditionLifecycle;
			std::vector<LostArk::Shared::PLAYER_ID> PlayerIds;
			std::string strEntryTriggerSequenceId;
			std::set<std::string> CompletedArrivals;
			LostArk::Shared::NET_ENTITY_ID iPrimaryBossId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iAuditionRequestSequence = 0u, iAuditionEpoch = 0u, iNextEntryTick = 0u;
			bool bClearCinematic = false, bEntryRunning = false, bGate3CombatEntered = false;
            bool bClearedBossPreparation = false;
            bool bGateVoteEntry = false;
            bool bBingoSpecialRunning = false;
            std::uint32_t iGate3ClearTick = 0u;
		};
		KOUKU_RAID_RUN m_KoukuRaid;
		std::uint32_t m_iNextKoukuRaidEpoch = 1u;
		std::map<SESSION_ID, std::pair<LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST, LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE>> m_KoukuRaidReceipts;
		bool Is_KoukuRaidRunning() const;
		bool Start_KoukuBingoSpecialPattern(std::uint32_t tick);
		bool Build_KoukuRaidState(LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE& state) const;
		void Broadcast_KoukuRaidState();
		void Update_KoukuRaid(std::uint32_t tick);
		void Notify_KoukuRaidBossDeath(const SERVER_WORLD_ENTITY& boss, std::uint32_t tick);
		void Stop_KoukuRaid(std::string reason, bool completed = false);
		bool Begin_KoukuRaidPreparation(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request, std::string& reason, bool clearedGateBoss = false, bool gateVoteEntry = false);
		bool Apply_KoukuRaidReadiness(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request, std::string& reason);
		bool Begin_KoukuRaidCinematic(const std::string& gateId, bool clear, std::uint32_t tick);
		bool Advance_KoukuRaidGate(std::uint8_t nextGate, bool restart);
		bool Start_KoukuRaidCombat(std::uint32_t tick, const std::string& preflightGateId = {});
        bool Build_KoukuRaidEntryRequest(const KOUKU_RAID_GATE_DEFINITION& gate, std::uint32_t entryIndex,
            LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request);
		bool Enter_KoukuRaidCombat(std::uint8_t gate, std::uint32_t tick);
		bool Start_KoukuRaidEntry(std::uint32_t tick);
		void Handle_KoukuSaydonDraftChunk(SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK& chunk);
		void Handle_KoukuSaydonPatternAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request);
		LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_RESULT
			Evaluate_KoukuSaydonPatternAudition(
				SESSION_ID sessionId,
				const LostArk::Shared::
					C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request,
				LostArk::Shared::
					S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& outResult, bool continueRaid = false,
                const std::vector<SERVER_WORLD_ENTITY>* stagedBosses = nullptr);
		SERVER_WORLD_ENTITY* Find_KoukuSaydonAuditionBoss();
		/* The live arena boss a Debug audition scope names: the Gate 1 Kouku or
		a gate boss raised from a disabled placement. Null when that placement
		is not currently spawned. */
		SERVER_WORLD_ENTITY* Find_KoukuSaydonArenaBoss(
			const std::string& placementId,
			const std::string& archetypeId);
		bool Update_KoukuSaydonBoss(
			SERVER_WORLD_ENTITY& boss, std::uint32_t serverTick);
		/* Broadcasts the cues a Logic tick produced, inserts follow-up patterns
		after the running audition slot, and returns true when a window asked
		the running pattern to end now (the brain commits it as COMPLETED). */
		struct KOUKU_PENDING_MECHANIC_TRIGGER final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iPatternSequence = 0u;
			BOSS_PATTERN_MECHANIC_TRIGGER Trigger;
		};
		std::vector<KOUKU_PENDING_MECHANIC_TRIGGER> m_PendingKoukuMechanicTriggers;
		[[nodiscard]] bool Commit_KoukuAlbionAirborne(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t serverTick);
		void Commit_KoukuMechanicTriggers(std::uint32_t serverTick);
		void Update_KoukuActorContacts(SERVER_WORLD_ENTITY& actor, const BOSS_PATTERN_DEFINITION& pattern,
			const CGameplayCatalog& product, std::uint32_t serverTick);
		void Update_KoukuPursuitProjectiles(SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
			KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE& window, const CGameplayCatalog& catalog, std::uint32_t serverTick);
		void Update_KoukuPlayerTargets(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern, KOUKUSAYDON_LOGIC_LEDGER& ledger,
			const CGameplayCatalog& catalog, std::uint32_t serverTick);
		void Clear_KoukuPlayerTargets(SERVER_WORLD_ENTITY& boss, KOUKUSAYDON_LOGIC_LEDGER& ledger);
		void Update_KoukuRandomVolley(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE& window,
			const CGameplayCatalog& catalog, std::uint32_t serverTick, bool hasAlivePlayers);
		void Update_KoukuGazeClones(std::uint32_t serverTick);
		[[nodiscard]] bool Update_KoukuSummonTriggers(SERVER_WORLD_ENTITY& clone,
			const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		bool Apply_KoukuLogicOutput(
			const KOUKUSAYDON_LOGIC_OUTPUT& output,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		enum class KOUKUSAYDON_PATTERN_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE
		};

		struct KOUKUSAYDON_PATTERN_AUDITION_MEMBER final
		{
			std::string strMemberId;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			KOUKUSAYDON_PATTERN_AUDITION_PHASE ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING;
			std::vector<std::string> PatternIds;
			std::vector<std::uint32_t> TransitionTicks;
			std::size_t iPatternIndex = 0u;
			std::uint32_t iNextStartTick = 0u;
			std::uint32_t iScheduledStartTick = 0u;
			std::uint32_t iPatternSequence = 0u;
			bool bCompleted = false;
			bool bOwnsPlayerMode = false;
			KOUKUSAYDON_LOGIC_LEDGER LogicLedger;
			// The entry root owns its portal across the completion-driven children.
			std::optional<SERVER_WORLD_ENTITY> MarioEntryAnchor;
			std::string strMarioEntryPatternId;
			std::uint32_t iMarioEntryStartTick = 0u;
			std::uint8_t iMarioEntryStage = 0u;
			bool bMarioEntryConsumed = false;
			// Pin the successful entrant, not whichever player is present later.
			bool bMarioSoloReturnRequired = false;
			LostArk::Shared::PLAYER_ID iMarioEntrantPlayerId = 0u;
			SESSION_ID iMarioEntrantSessionId = INVALID_SESSION_ID;
			LostArk::Shared::NET_ENTITY_ID iMarioEntrantNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			bool bMarioReturnCompleted = false;
			std::size_t iCompletionChainFirstIndex = 0u;
			std::uint32_t iCompletionChainCount = 0u;
			std::uint32_t iCompletionChainCompleted = 0u;
			std::string strCompletionChainSuccessPatternId;
			bool bCompletionChainStarted = false;
            bool bParentSequenceStarted = false;
            bool bParentSequenceLoops = false;
            std::size_t iParentLoopIndex = 0u;
            std::size_t iParentLastIndex = 0u;
			bool bCompletionChainAwaitingReturn = false;
			bool bCompletionChainSuccessQueued = false;
			std::uint32_t iNextWorldCue = 1u;
			std::unordered_map<std::string, std::string> WorldCueByInstance;
			std::unordered_map<std::string, std::string> WorldCueByOccurrence;
		};
		// Stage completion releases the actor clock, while these occurrence rows retain theirs.
		struct KOUKU_PATTERN_TAIL final
		{
			std::shared_ptr<SERVER_WORLD_ENTITY> pOwner;
			KOUKUSAYDON_PATTERN_AUDITION_MEMBER Member;
			std::uint32_t iEndTick = 0u;
			std::uint32_t iLastUpdateTick = 0u;
		};
		struct KOUKU_SCHEDULED_SUPPORT_SURFACE final
		{
			std::string strMemberId;
			std::uint32_t iStartTick = 0u;
			std::uint32_t iEndTick = 0u;
			SERVER_NAVIGATION_SUPPORT_SURFACE Surface;
		};
		struct KOUKUSAYDON_PATTERN_AUDITION_STATE final
		{
			KOUKUSAYDON_PATTERN_AUDITION_PHASE ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST Request;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::uint32_t iCommonStartTick = 0u;
			LostArk::Shared::GameplayDataRevision PinnedGameplayRevision{};
			std::uint32_t iPinnedSourceRevision = 0u;
			/* Global gameplay authority remains PinnedGameplayRevision. The
			   separate Kouku Product source owns pattern/logic rows for this run. */
			std::shared_ptr<const CGameplayCatalog> pProductGeneration;
			std::vector<KOUKUSAYDON_PATTERN_AUDITION_MEMBER> Members;
			std::vector<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> WorldPlays;
			std::vector<KOUKU_SCHEDULED_SUPPORT_SURFACE> SupportSchedule;
			std::vector<KOUKU_PATTERN_TAIL> Tails;
		};
		KOUKUSAYDON_PATTERN_AUDITION_MEMBER* Find_KoukuAuditionMember(LostArk::Shared::NET_ENTITY_ID bossId, std::uint32_t patternSequence = 0u);
		KOUKUSAYDON_LOGIC_LEDGER* Active_KoukuPlayerLedger();
		[[nodiscard]] const CGameplayCatalog* Resolve_KoukuProductCatalog() const noexcept;
		void Prepare_KoukuAuditionTick(std::uint32_t serverTick);
		void Update_KoukuPatternTails(std::uint32_t serverTick);
		SERVER_WORLD_ENTITY* Find_KoukuOccurrenceOwner(LostArk::Shared::NET_ENTITY_ID bossId, std::uint32_t patternSequence);
		bool Retain_KoukuPatternTail(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			const SERVER_WORLD_ENTITY& sourceOwner, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
        bool Start_KoukuParentSequence(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
            SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		bool Start_KoukuCompletionChain(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		void Update_KoukuMarioEntry(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member, std::uint32_t serverTick);
		void Commit_KoukuMarioEntries();
		bool Commit_KoukuMarioPhasePlayers(SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t serverTick);
		void Complete_KoukuMarioReturn(SERVER_PLAYER& player,
			const std::string& sourcePlacementId, std::uint32_t updateTick);
		void Queue_KoukuCompletionChainSuccess(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			std::uint32_t serverTick);
		struct KOUKU_PENDING_MARIO_ENTRY final { std::string strMemberId; LostArk::Shared::PLAYER_ID iPlayerId; std::uint32_t iRootStartTick; std::uint8_t iStage = 0u; };
		std::vector<KOUKU_PENDING_MARIO_ENTRY> m_PendingKoukuMarioEntries;
		bool Enter_MarioFromPattern(SERVER_PLAYER& player, std::uint8_t stage);
		bool Refresh_KoukuSupportSurfaces(std::uint32_t serverTick);
		bool Build_KoukuBundleState(LostArk::Shared::S2C_KOUKUSAYDON_BUNDLE_STATE& message) const;
		void Broadcast_KoukuBundleState(LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE state);
		void Broadcast_OwnedWorldSequence(const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& message);
		void Stop_KoukuWorldOwner(const std::string& memberId = {}, bool finished = false);
		// Survives natural Pattern completion; reset/cancel and HP zero own removal.
		struct KOUKU_DAMAGEABLE_WORLD_CUE final
		{
			LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY Play;
			LostArk::Shared::NET_ENTITY_ID iBodyId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			bool bCancelled = false;
			std::uint32_t iNextMadnessTick = 0u;
			BOSS_ENCOUNTER_MADNESS_POLICY MadnessPolicy;
			std::uint8_t iMadnessSource = 0u; // 1 circus ball, 2 odd doll
		};
		std::vector<KOUKU_DAMAGEABLE_WORLD_CUE> m_KoukuDamageableWorldCues;
		std::vector<SERVER_WORLD_ENTITY> m_PendingKoukuWorldBodies;
		bool Stage_KoukuWorldBody(const BOSS_PATTERN_WORLD_COMBAT_BODY& body,
			LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play, bool authoredMadness = false);
		void Cancel_KoukuWorldBodies(const std::string& memberId = {}, SESSION_ID ownerSession = INVALID_SESSION_ID);
		void Update_KoukuWorldBodies(std::uint32_t serverTick);

		struct KOUKUSAYDON_PATTERN_AUDITION_RECEIPT final
		{
			LostArk::Shared::
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST Request;
			LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT Result;
			std::optional<LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE>
				LastLifecycle;
		};

		void Queue_KoukuSaydonPatternAuditionLifecycle(
			const std::string& patternId,
			std::uint32_t patternSequence,
			std::uint32_t stageIndex,
			LostArk::Shared::
				KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {}, LostArk::Shared::NET_ENTITY_ID bossId = LostArk::Shared::INVALID_NET_ENTITY_ID);
		bool Flush_KoukuSaydonPatternAuditionLifecycle();
		void Clear_KoukuSaydonPatternAudition(bool completed = false, std::string reason = {});
		SERVER_WORLD_ENTITY* Find_AuditionBoss();
		SERVER_WORLD_ENTITY* Find_AuditionBoss(
			const std::string& placementId);
		bool Has_EngagedAuditionPlayer(const SERVER_WORLD_ENTITY& boss) const;
		bool Build_ValtanBossOnlyAuditionReset(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			SERVER_WORLD_ENTITY& outBoss,
			std::string& status);
		bool Reset_ValtanBossOnlyAuditionState(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		bool Reset_ValtanAuditionState(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		enum class VALTAN_PATTERN_ID_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE,
			COMPLETED_HOLD,
			IDLE_HOLD
		};

		struct VALTAN_PATTERN_ID_AUDITION_STATE final
		{
			VALTAN_PATTERN_ID_AUDITION_PHASE ePhase =
				VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = 0u;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::string strBossPlacementId;
			std::string strPatternId;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			bool bResetlessContinuation = false;
			bool bReportedWaitingForPlayer = false;
			// A live predecessor has no Client Play request to report a lifecycle for.
			bool bAdoptedLivePredecessor = false;
			// Keep only the current Flow occurrence on its existing ordered Brain path.
			std::optional<BOSS_PATTERN_SEQUENCE_DEFINITION> AdoptedFlowSequence;
		};

		struct VALTAN_NEXT_PATTERN_RESERVATION final
		{
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::uint32_t iPredecessorPatternSequence = 0u;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::string strBossPlacementId;
			std::string strPatternId;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			bool bReportedWaitingForPlayer = false;
		};

		struct VALTAN_NEXT_PATTERN_COMMAND_RECEIPT final
		{
			LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request;
			LostArk::Shared::VALTAN_AUDITION_RESULT Result =
				LostArk::Shared::VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
			std::uint32_t iCurrentHealthBar = 0u;
		};

		[[nodiscard]] bool Is_ValtanPatternIdAuditionRunning() const noexcept;
		LostArk::Shared::VALTAN_AUDITION_RESULT Evaluate_ValtanNextPatternControl(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			std::uint32_t& outCurrentHealthBar);
		LostArk::Shared::VALTAN_AUDITION_RESULT Adopt_ValtanLiveNextPattern(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			SERVER_WORLD_ENTITY& boss);
		void Cancel_ValtanNextPatternReservation(std::string reason);
		void Cancel_ValtanPatternIdAudition(std::string reason);
		void Try_PromoteValtanNextPattern(SERVER_WORLD_ENTITY& boss);
		bool Prepare_ValtanPatternIdAuditionBeforeBrain(SERVER_WORLD_ENTITY& boss);
		bool Refresh_ValtanPatternIdAuditionState();
		void Queue_ValtanAuditionLifecycle(
			SESSION_ID ownerSessionId,
			std::uint32_t requestSequence,
			std::uint32_t roomEpoch,
			std::uint32_t patternSequence,
			const std::string& patternId,
			const LostArk::Shared::GameplayDataRevision& pinnedRevision,
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		void Queue_ValtanNextPatternLifecycle(
			const VALTAN_NEXT_PATTERN_RESERVATION& reservation,
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		void Queue_ValtanPatternIdAuditionLifecycle(
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		bool Flush_ValtanPatternIdAuditionLifecycle();

		enum class VALTAN_PATTERN_FLOW_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE
		};

		struct VALTAN_PATTERN_FLOW_AUDITION_STATE final
		{
			VALTAN_PATTERN_FLOW_AUDITION_PHASE ePhase =
				VALTAN_PATTERN_FLOW_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::PLAYER_ID iOwnerPlayerId =
				LostArk::Shared::INVALID_PLAYER_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomFlowEpoch = 0u;
			std::uint32_t iFirstPatternSequence = 0u;
			std::size_t iStartSlotIndex = 0u;
			std::size_t iReportedSequenceIndex =
				(static_cast<std::size_t>(-1));
			std::uint32_t iReportedPatternSequence = 0u;
			bool bReportedPausedForRevive = false;
			bool bStopAfterCurrent = false;
			std::string strBossPlacementId;
			std::string strFlowId;
			std::string strFlowRevision;
			std::string strStartSlotId;
			std::vector<LostArk::Shared::VALTAN_PATTERN_FLOW_SLOT_WIRE> Slots;
			BOSS_PATTERN_SEQUENCE_DEFINITION Sequence;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
		};

		[[nodiscard]] bool Is_ValtanPatternFlowRunning() const noexcept;
		[[nodiscard]] const BOSS_PATTERN_SEQUENCE_DEFINITION*
			Resolve_ValtanPatternFlowSequence(
				const SERVER_WORLD_ENTITY& boss) const noexcept;
		void Refresh_ValtanPatternFlowState(SERVER_WORLD_ENTITY& boss);
		void Finish_ValtanPatternFlow(
			SERVER_WORLD_ENTITY& boss,
			LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE terminalState,
			std::string reason = {});
		void Abort_ValtanPatternFlowForOwner(
			SESSION_ID sessionId,
			std::string reason);
		void Queue_ValtanPatternFlowLifecycle(
			LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE state,
			const SERVER_WORLD_ENTITY* boss,
			std::string reason = {});
		bool Flush_ValtanPatternFlowLifecycle();

		enum class VALTAN_TIMELINE_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			WAITING_ENVIRONMENT,
			READY,
			WAITING_PATTERN_START,
			WAITING_PATTERN_FINISH,
			COMPLETED_HOLD,
			FAILED_HOLD
		};

		struct VALTAN_TIMELINE_AUDITION_STATE final
		{
			VALTAN_TIMELINE_AUDITION_PHASE ePhase =
				VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = 0;
			LostArk::Shared::PLAYER_ID iOwnerPlayerId =
				LostArk::Shared::INVALID_PLAYER_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::size_t iRowIndex = 0u;
			std::size_t iActionIndex = 0u;
			std::uint32_t iRepeatIndex = 0u;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::uint32_t iEnvironmentDeadlineTick = 0u;
			std::uint32_t iHeldBossHp = 0u;
			std::uint32_t iHeldBossHealthBar = 0u;
			bool bAllowProductPropBreak = false;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			std::string strExpectedPatternId;
			std::vector<std::string> ExpectedGoneGroupIds;
		};

		/* A page start differs from a one-row timeline audition: it stages the
		already-destroyed arena, releases the real Brain at that page boundary,
		and then leaves the encounter running normally. */
		struct VALTAN_FIGHT_PAGE_START_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iCommandId = 0u;
			std::uint32_t iEnvironmentDeadlineTick = 0u;
			std::vector<std::string> ExpectedGoneGroupIds;

			bool Is_Active() const noexcept
			{
				return LostArk::Shared::INVALID_NET_ENTITY_ID != iBossEntityId;
			}
		};

		bool Prepare_ValtanTimelineArenaState(
			const CWorldDestructionRuntime& runtime,
			const SERVER_WORLD_ENTITY& boss,
			VALTAN_TIMELINE_ARENA_STATE arenaState,
			std::uint32_t requestTick,
			WORLD_DESTRUCTION_TRANSACTION& outTransaction,
			std::vector<std::string>& outExpectedGoneGroupIds,
			std::string& status) const;
		bool Stage_ValtanTimelineRowStart(
			SESSION_ID sessionId,
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			SERVER_PLAYER& outOwner,
			std::string& status) const;
		bool Start_ValtanTimelineRow(
			SESSION_ID sessionId,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			std::string& status);
		bool Stop_ValtanTimelineRow(bool resetEncounter = false);
		bool Prepare_ValtanTimelineRowBeforeBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		void Restore_ValtanTimelineRowAfterBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		bool Start_ValtanFightPage(
			SESSION_ID sessionId,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			std::string& status);
		bool Prepare_ValtanFightPageBeforeBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		struct VALTAN_DECISION_TRACE_REVISION_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::string strBossPlacementId;
			std::uint64_t iTraceSequence = 0u;
			LostArk::Shared::GameplayDataRevision DefinitionRevision{};
		};
		// Validates itemId against the loaded catalog and stacks quantity into
		// player.Inventory, capped at maxStack and MAX_INVENTORY_ITEMS distinct
		// stacks. Returns false (no-op) for an unknown item or a full inventory
		// that would need a new stack. Shared by Handle_DebugGiveItem and the
		// Valtan clear-reward grant in the world entity tick loop.
		bool Grant_Item(
			SERVER_PLAYER& player,
			const std::string& itemId,
			std::uint32_t quantity);
		// Debug-only. Validates the item against the loaded catalog and
		// stacks it into the player's inventory, capped at maxStack.
		void Handle_DebugGiveItem(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_GIVE_ITEM& request);
		// Validates the item is owned, is a consumable (iHealPercent > 0), and
		// the player is alive; heals iMaximumHp * iHealPercent / 100, then
		// decrements/removes the stack. HP reaches the Client through the next
		// S2C_WORLD_SNAPSHOT tick like any other HP change; no separate result.
		void Handle_UseItem(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_ITEM& request);
		/* Right-click equip / unequip. Checks the slot kind, the class and bag room,
		   then answers with the whole inventory whether or not anything moved. */
		void Handle_SetEquipment(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_EQUIPMENT& request);
		bool Apply_SetEquipment(SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_EQUIPMENT& request) const;
		/* Repair NPC window: every worn part goes back to 100 percent (free for now); the
		   inventory answer carries the repaired percents. */
		void Handle_RepairEquipment(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_REPAIR_EQUIPMENT& request);
		/* After a class change: items bound to another class go back to the bag. */
		bool Unequip_OtherClassItems(SERVER_PLAYER& player) const;
		/* NPC shop basket. The player must stand by that shop NPC; every line must be in
		   its stock, the total price must be covered and the bag must take every line, or
		   nothing changes. Answers with the whole inventory either way. */
		void Handle_BuyItems(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_BUY_ITEMS& request);
		bool Apply_BuyItems(SERVER_PLAYER& player,
			const LostArk::Shared::C2S_BUY_ITEMS& request) const;
		/* One-shot restore of a Client-saved character (inventory, purse, honor title).
		   Bern only, once per fresh entry, and only before any inventory change; the
		   whole request is validated first and a rejected one changes nothing. */
		void Handle_RestoreCharacter(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RESTORE_CHARACTER& request);
		bool Validate_RestoreCharacter(const SERVER_PLAYER& player,
			const LostArk::Shared::C2S_RESTORE_CHARACTER& request) const;
		// Debug Character Select Arena "되돌리기" -- despawns every world entity the
		// debug spawn buttons created in this room (Broadcast_WorldEntityDespawned per
		// entity) and resets the spawn group runtime so the same groups can be
		// re-activated. CHARACTER_SELECT_ARENA only; no-op reply for anything else.
		void Handle_DespawnAllWorldEntities(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DESPAWN_ALL_WORLD_ENTITIES& request);
		/* KoukuSaydon arena form of the Debug revert: removes only the entities
		raised from disabled bootstrap placements (the F1 gate buttons) and
		their dependents, keeping the statically enabled Gate 1 Kouku. */
		bool Despawn_KoukuSaydonArenaDebugEntities(bool allArenaBosses = false);
		bool Despawn_KoukuSaydonArenaDebugEntities(bool allArenaBosses, bool preflightOnly);
		// Bern's Valtan-entry confirm window (right-click a guide NPC). Replaces the
		// old automatic changeLevel triggerBox OBB fire: validates the requesting
		// player is still near the named guide NPC world entity, alive, and idle,
		// then stages the same SERVER_WORLD_TRANSFER_REQUEST the trigger used to
		// build. BERN only; no-op for anything else.
		void Handle_ConfirmNpcEntry(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CONFIRM_NPC_ENTRY& request);
		// Colosseum match queue (BERN only). JOIN is the answer to the Colosseum NPC's offer: the
		// Server re-tests distance/state, collects up to four humans for ten seconds,
		// and reserves acceptance-order parity teams for one atomic
		// admission into a private Colosseum match. Only a committed transfer consumes the queue.
		// LEAVE removes only the requesting session.
		void Handle_ColosseumQueueJoin(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_COLOSSEUM_QUEUE_JOIN& request);
		void Handle_ColosseumQueueLeave(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_COLOSSEUM_QUEUE_LEAVE& request);
		void Send_ColosseumQueueState(
			SESSION_ID sessionId, LostArk::Shared::COLOSSEUM_QUEUE_STATE state);
		void Try_FormColosseumMatch();
		void Broadcast_ColosseumQueueState();
		void Try_StartColosseumEntry();
		void Handle_ColosseumLoadReady(SESSION_ID, const LostArk::Shared::C2S_COLOSSEUM_LOAD_READY&);
		void Handle_ColosseumReturn(SESSION_ID, const LostArk::Shared::C2S_COLOSSEUM_RETURN&);
		void Update_ColosseumMatch(std::uint32_t tick);
		void Broadcast_ColosseumMatchState();
		void Score_ColosseumKills(std::uint32_t tick);
		// The player pressed the key an interact-gated trigger box offered.
		// Names only the box; the trigger system re-tests that this player is
		// still standing in it before anything runs, so a stale or forged
		// request changes nothing.
		void Handle_InteractTrigger(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_INTERACT_TRIGGER& request);
		// Raid Clear screen's "돌아가기" button -- the reverse trip. No proximity
		// or party-leader gating (unlike Handle_ConfirmNpcEntry): any player in
		// a cleared Valtan/Kouku raid can return independently to its recorded entry guide.
		// Direct Lobby entries retain the default guide; NPC entries keep their source ID.
		void Handle_ReturnToBern(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RETURN_TO_BERN& request);
		// Stages one player's world transfer back to Bern. Shared by the cleared-raid exit
		// button and the gate progress EXIT vote; returns false when nothing was staged.
		bool Stage_ReturnToBern(
			LostArk::Shared::PLAYER_ID playerId,
			std::uint32_t requestSequence);
		/* Same-room-only: request.iTargetNetEntityId must resolve to a real
		   player currently in this room's m_PlayerIdByEntityId. There is no
		   cross-room player identity yet (nickname is display text only, see
		   CLAUDE.md), so an invite naming a player in a different room or a
		   stale/unknown NetEntityId is rejected, not queued. */
		void Handle_PartyInvite(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_PARTY_INVITE& request);
		void Handle_PartyInviteRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_PARTY_INVITE_RESPOND& request);
		void Broadcast_PartyRoster(std::uint32_t partyId);

		/* 파티 레이드 입장 전원 수락 투표. 한 플레이어는 동시에 하나의 열린 proposal에만
		   속한다(propose가 그 불변식을 검사). iProposalId는 이 방에서 발급하는 단조 증가
		   식별자로 pointer/index가 아니다. Voters는 발의 시점 멤버 스냅샷(솔로는 1명),
		   Accepted는 그 부분집합. m_iServerTick이 iDeadlineTick을 넘으면 TIMEOUT으로 닫는다. */
		/* Commander raid gate progress (KoukuSaydon gates). The room marks a gate cleared
		   when its last primary boss dies (Notify_GateBossDeath from the world update),
		   the leader / solo player proposes to move on, members answer, and on
		   ALL_ACCEPTED Advance_Gate despawns the arena, raises the next gate's disabled
		   placements and moves every player to the gate position -- the product path of
		   what the Debug gate buttons do by hand. Implemented in GameRoom_GateProgress.cpp. */
		struct GATE_PROGRESS_STATE
		{
			std::uint8_t iCurrentGate = 0u;     // 1-based, 0 = no gate raised yet
			std::uint8_t iClearedMask = 0u;
			std::uint32_t iProposalId = 0u;     // 0 = no vote open
			std::uint32_t iRaidEpoch = 0u;      // Nonzero pins a vote to its immutable raid run
			LostArk::Shared::GATE_PROGRESS_KIND eKind = LostArk::Shared::GATE_PROGRESS_KIND::ADVANCE;
			std::uint32_t iRequestSequence = 0u;
			LostArk::Shared::PLAYER_ID iProposerId = LostArk::Shared::INVALID_PLAYER_ID;
			std::vector<LostArk::Shared::PLAYER_ID> Voters;
			std::vector<LostArk::Shared::PLAYER_ID> Accepted;
			std::uint32_t iDeadlineTick = 0u;
		};
		void Handle_GateProgressPropose(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_GATE_PROGRESS_PROPOSE& request);
		void Handle_GateProgressRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_GATE_PROGRESS_RESPOND& request);
		void Close_GateProgressVote(LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result);
		void Expire_GateProgressVote();
		bool Complete_GateProgressTransition(const GATE_PROGRESS_STATE& transition);
		void Update_ArenaAssembly();
		bool Complete_ArenaAssembly();
		bool Collect_KoukuEntryParticipants(std::vector<LostArk::Shared::PLAYER_ID>& participants, std::uint32_t& raidEpoch) const;
		bool Collect_ValtanEntryParticipants(std::vector<LostArk::Shared::PLAYER_ID>& participants) const;
		bool Start_ValtanEntry(const std::vector<LostArk::Shared::PLAYER_ID>& expectedParticipants);
		// A primary boss is about to be removed DEAD: clears its gate when it was the last one.
		void Notify_GateBossDeath(const SERVER_WORLD_ENTITY& deadBoss);
		// A gate placement came up (Debug button or Advance_Gate): that gate is now current.
		void Note_GatePlacementRaised(const std::string& placementId);
		bool Advance_Gate(std::uint8_t nextGate);
		bool Advance_Gate(std::uint8_t nextGate, const std::vector<LostArk::Shared::PLAYER_ID>* participants);
		bool Spawn_GatePlacement(const std::string& placementId);
		bool Spawn_GatePlacement(const std::string& placementId, SERVER_WORLD_ENTITY* prepared);
		bool Build_GateProgressState(LostArk::Shared::S2C_GATE_PROGRESS_STATE& message,
			bool bClosed, LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result) const;
		void Broadcast_GateProgressState(
			bool bClosed, LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result,
			LostArk::Shared::GATE_PROGRESS_KIND completedKind = LostArk::Shared::GATE_PROGRESS_KIND::END);
		std::uint8_t Gate_Count() const;
		std::uint8_t Resolve_CurrentKoukuGate() const;
		bool Resolve_KoukuRevivePosition(const SERVER_PLAYER& player, SERVER_NAV_POINT& position, float& yaw) const;
		int Gate_IndexOfPlacement(const std::string& placementId) const;
		/* Raid-clear award input. Every fought primary boss advances its players' fight
		   clock each tick; a dying gate boss hands its ledger to the room, and the clear
		   sends the room ledger to every player and empties it. */
		void Tick_MvpLedgers();
		void Merge_MvpLedger(SERVER_WORLD_ENTITY& boss);
		void Broadcast_RaidMvpResult(std::uint8_t iGate);

		struct RAID_ENTRY_PROPOSAL
		{
			std::uint32_t iProposalId = 0u;
			std::uint32_t iPartyId = 0u;
			std::uint32_t iRequestSequence = 0u;
			LostArk::Shared::RAID_ENTRY_TARGET eTarget =
				LostArk::Shared::RAID_ENTRY_TARGET::VALTAN;
			std::string strNpcPlacementId;
			std::vector<LostArk::Shared::PLAYER_ID> Voters;
			std::vector<LostArk::Shared::PLAYER_ID> Accepted;
			std::uint32_t iDeadlineTick = 0u;
		};

		// 리더/솔로가 입장하기로 발의 -> 대상 전원(솔로는 본인)에게 프롬프트, 투표 개시.
		void Handle_RaidEntryPropose(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RAID_ENTRY_PROPOSE& request);
		// 개별 수락/거절 반영. 거절이면 즉시 DECLINED 종료, 전원 수락이면 ALL_ACCEPTED 종료.
		void Handle_RaidEntryRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RAID_ENTRY_RESPOND& request);
		// 투표를 result로 종료해 전원에 통지하고 proposal을 제거한다. ALL_ACCEPTED면
		// Stage_PartyWorldTransfer로 batch 전송을 stage하고, stage 실패면 CANCELLED로 낮춘다.
		void Close_RaidEntryVote(
			RAID_ENTRY_PROPOSAL& proposal,
			LostArk::Shared::RAID_ENTRY_VOTE_RESULT result);
		// 진행/종료 S2C_RAID_ENTRY_VOTE를 proposal의 present voters에게 보낸다.
		void Broadcast_RaidEntryVote(
			const RAID_ENTRY_PROPOSAL& proposal, bool bClosed,
			LostArk::Shared::RAID_ENTRY_VOTE_RESULT result);
		// m_iServerTick 기준 만료 proposal을 TIMEOUT으로 닫는다(tick 루프에서 호출).
		void Expire_RaidEntryProposals();
		// playerId가 voter인 열린 proposal을 CANCELLED로 닫는다(이탈/파티 해산 시).
		void Cancel_RaidEntryProposalsInvolving(LostArk::Shared::PLAYER_ID playerId);
		// 검증된 batch 멤버(front=리더)를 기존 SERVER_WORLD_TRANSFER_REQUEST 경로로 stage.
		// 멤버 unavailable/이미 staged면 false(호출자가 투표를 CANCELLED로 닫는다).
		bool Stage_PartyWorldTransfer(
			const std::vector<LostArk::Shared::PLAYER_ID>& batchMemberIds,
			LostArk::Shared::WORLD_ID targetWorldId,
			std::uint32_t requestSequence, const std::string& raidReturnNpcPlacementId,
			const std::string& spawnPlacementOverrideId = {});
		// player가 알려진 Valtan 입장 guide NPC 근처(proximity)인지 검증한다.
		bool Is_PlayerNearValtanEntryNpc(
			const SERVER_PLAYER& player, const std::string& npcPlacementId) const;
		/* Tells every session in this room that an authored world sequence
		   instance started. Presentation only: the Server keeps no sequence
		   state, so a session that joins later simply misses a played edge.
		   False rejects the action without consuming its trigger or moving players. */
		bool Broadcast_WorldSequencePlay(
			const std::string& instanceId, float playbackSpeed = 1.f,
			float positionOffsetX = 0.f, float positionOffsetY = 0.f, float positionOffsetZ = 0.f,
			std::uint32_t durationMs = 0u, const std::string& targetSequenceInstanceId = {},
			LostArk::Shared::WORLD_SEQUENCE_OPERATION operation = LostArk::Shared::WORLD_SEQUENCE_OPERATION::PLAY);
		void Handle_DebugKillGateBosses(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KILL_GATE_BOSSES& request);
		LostArk::Shared::DEBUG_KILL_GATE_BOSSES_RESULT Apply_DebugKillGateBosses(
			SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KILL_GATE_BOSSES& request, std::uint8_t& killedCount);
		std::unordered_map<SESSION_ID, std::uint32_t> m_KillGateBossesRequestSequences;
		void Handle_SetCooldownMode(SESSION_ID sessionId, const LostArk::Shared::C2S_SET_COOLDOWN_MODE& request);
		LostArk::Shared::SET_COOLDOWN_MODE_RESULT Apply_SetCooldownMode(
			SESSION_ID sessionId, const LostArk::Shared::C2S_SET_COOLDOWN_MODE& request);
		LostArk::Shared::COOLDOWN_MODE m_eCooldownMode = LostArk::Shared::COOLDOWN_MODE::DEBUG_THREE_SECONDS;
		std::unordered_map<SESSION_ID, std::uint32_t> m_CooldownModeRequestSequences;
		void Handle_DebugWorldPlayback(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK& request);
		std::unordered_map<SESSION_ID, std::uint32_t> m_WorldPlaybackRequestSequences;
		LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT Apply_DebugRoomPlayerArrival(
			SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK& request);
		LostArk::Shared::DEBUG_TELEPORT_RESULT Validate_DebugTeleportDestination(
			const SERVER_PLAYER& player, const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request,
			SERVER_NAV_POINT& ground, LostArk::Shared::NET_ENTITY_ID ignoredBodyId = LostArk::Shared::INVALID_NET_ENTITY_ID);
		struct ROOM_PLAYER_ARRIVAL_RUN final
		{
			std::uint32_t iEpoch = 0u;
			std::string strRootPatternId;
			std::vector<std::pair<LostArk::Shared::PLAYER_ID, SESSION_ID>> Players;
			std::map<std::string, LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT> Occurrences;
		};
		std::unordered_map<SESSION_ID, ROOM_PLAYER_ARRIVAL_RUN> m_RoomPlayerArrivalRuns;
		/* Offers or withdraws one interact-gated box for the one player it
		   concerns. Unlike the sequence broadcast this is never room-wide. */
		void Send_InteractPrompt(const SERVER_INTERACT_PROMPT_EDGE& edge);
		/* The spawn-group activation every trigger path shares: starts a dormant
		   group, restarts a finished one once its monsters are gone, and never
		   stacks a wave on a group that is still running. */
		bool Activate_SpawnGroupFromTrigger(const std::string& spawnGroupId);
		/* What one trigger action does in this room, whether the box fired because
		   the player stepped in (a scripted flow) or pressed G inside it. */
		bool Activate_TriggerTarget(
			WORLD_TRIGGER_ACTION_KIND kind, const std::string& targetId);
		/* Leave() calls this so a disconnecting player does not linger as a
		   ghost roster entry for whoever they partied with. */
		void Remove_FromParty(LostArk::Shared::PLAYER_ID playerId);
		/* Same room-scoped broadcast Broadcast_PartyRoster already uses --
		   every current session in this room receives the relayed line,
		   including the sender (its own head bubble is driven off the same
		   S2C_CHAT the rest of the room gets, not a second local-only path). */
		void Handle_RoomPing(SESSION_ID sessionId, const LostArk::Shared::C2S_ROOM_PING& request);
		void Handle_Chat(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CHAT& request);

		bool Send_Accepted(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		bool Send_EnterRejected(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::ENTER_WORLD_REJECTION_REASON reason);
		bool Send_Spawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		static bool Build_WorldEntitySpawnedPayload(
			const SERVER_WORLD_ENTITY& entity,
			std::vector<std::uint8_t>& outPayload);
		bool Send_WorldEntitySpawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_WORLD_ENTITY& entity);
		bool Send_WorldEntityDespawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason =
				LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::REMOVED);
		bool Send_CombatObjectSpawned(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned);
		bool Send_WorldEntitySpawnResult(
			const std::shared_ptr<CClientSession>& session,
			const std::string& placementId,
			LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT result,
			LostArk::Shared::NET_ENTITY_ID netEntityId);
		bool Send_ValtanAuditionResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			LostArk::Shared::VALTAN_AUDITION_RESULT result,
			std::uint32_t currentHealthBar);
		bool Send_KoukuSaydonPatternAuditionResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& message);
		bool Send_ValtanPatternFlowResult(
			const std::shared_ptr<CClientSession>& session,
			std::uint32_t commandSequence,
			LostArk::Shared::VALTAN_PATTERN_FLOW_COMMAND command,
			LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT result,
			const std::string& flowId,
			const std::string& flowRevision,
			std::uint32_t roomFlowEpoch,
			const LostArk::Shared::GameplayDataRevision& pinnedRevision,
			const std::string& reason);
		bool Build_RequiredPinnedGameplayRevisions(
			std::vector<LostArk::Shared::GameplayDataRevision>&
				outRevisions) const;
		[[nodiscard]] const CGameplayCatalog* Resolve_ValtanGameplayCatalog(
			const SERVER_WORLD_ENTITY& boss) const noexcept;
		bool Send_CharacterClassChangeResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request,
			LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT result,
			LostArk::Shared::CHARACTER_CLASS_ID activeClass);
		// Single-session send: inventory is per-player state, not room-shared
		// like S2C_WORLD_SNAPSHOT, so it never broadcasts.
		bool Send_InventorySnapshot(
			const std::shared_ptr<CClientSession>& session,
			std::uint32_t requestSequence,
			const SERVER_PLAYER& player);
		bool Send_Despawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		bool Send_WorldDestructionFullSync(
			const std::shared_ptr<CClientSession>& session);
		// Server-owned collision/navigation counters carried by every
		// destruction message so the Debug audition panel never has to infer
		// passage from the replicated wall states.
		LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS
			Build_WorldDestructionDiagnostics() const;
		void Broadcast_Spawned(
			const SERVER_PLAYER& player,
			SESSION_ID exceptSessionId);
		void Broadcast_Despawned(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		void Broadcast_WorldEntitySpawned(
			const SERVER_WORLD_ENTITY& entity);
		void Broadcast_WorldEntityDespawned(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason =
				LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::REMOVED);
		bool Broadcast_WorldDestructionDelta(
			const std::vector<WORLD_DESTRUCTION_STATE_TRANSITION>& transitions,
			const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::uint32_t serverTick);
		void Broadcast_WorldSnapshot();

		std::shared_ptr<CClientSession> Find_Session(
			SESSION_ID sessionId) const;
		void Rollback_Join(SESSION_ID sessionId);
		[[nodiscard]] bool Is_PlayerAdmissionFull() const;
		const WORLD_BOOTSTRAP_PLACEMENT* Find_AvailablePlayerSpawn() const;
		const WORLD_BOOTSTRAP_PLACEMENT* Find_Placement(
			const std::string& placementId) const;
		bool Build_WorldEntity(
			const WORLD_BOOTSTRAP_PLACEMENT& placement,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			SERVER_WORLD_ENTITY& outEntity,
			const CGameplayCatalog* definitionCatalog = nullptr,
			LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID, std::uint32_t ownerPatternSequence = 0u);
		bool Initialize_WorldEntities();
		bool Reset_ReplayableArenaWhenEmpty();
		bool Reset_ValtanArenaWhenEmpty();
		bool Apply_BossPatternStageActions(
			SERVER_WORLD_ENTITY& boss,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			std::uint32_t spawnWaveOrdinal = 0u,
			bool scheduledSpawnWave = false);
		bool Apply_BossPatternScheduledSpawnWave(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Apply_BossPatternStageTransition(
			SERVER_WORLD_ENTITY& boss,
			const std::string& previousPatternId,
			const std::string& previousActionId,
			const std::string& nextPatternId,
			const std::string& nextActionId,
			const LostArk::Shared::GameplayDataRevision&
				previousDefinitionRevision,
			const LostArk::Shared::GameplayDataRevision& nextDefinitionRevision,
			std::uint32_t serverTick);
		bool Stage_BossPatternStageActions(
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			SERVER_BOSS_COMBAT_STATE& stagedCombat,
			std::uint8_t& stagedGameplayPhase,
			SERVER_COMBAT_OBJECT_TRANSACTION& combatObjectTransaction,
			std::uint32_t spawnWaveOrdinal = 0u,
			bool scheduledSpawnWave = false);
		/* Runs only after every stage-action preflight transaction commits. These
		actions own player/target state and therefore cannot be staged inside the
		boss-combat or combat-object value transactions above. */
		bool Prepare_GrabbedPlayerImpact(
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const BOSS_PATTERN_STAGE_ACTION& action,
			std::uint32_t serverTick,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& stagedPlayers,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& stagedDamageEvents);
		SERVER_PLAYER* Select_BossRandomAliveTarget(const SERVER_WORLD_ENTITY& boss,
			const std::string& actionId, const std::string& targetId, std::uint32_t serverTick);
		bool Commit_BossPatternPlayerStageActions(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			std::uint32_t spawnWaveOrdinal = 0u);
		bool Resolve_ArenaRandomVolleyOrigins(
			const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_ACTION& action,
			const BOSS_COMBAT_OBJECT_DEFINITION& definition,
			std::uint32_t spawnWaveOrdinal,
			std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET>& outOrigins,
			float explicitMinimumSpacingM = 0.f, const SERVER_NAV_POINT* anchorOverride = nullptr);
		bool Broadcast_CombatObjectLifecycle();
		void Drain_BossCombatEvents();
		bool Apply_WorldDestructionStageEntry(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		/* Commit the 69 ordinary contact walls and the 30 outer ring walls in one
		transaction, leaving every floor sector INTACT. A floor-collapse bar only
		arrives after the fight has already taken those walls down, so the
		audition for such a bar has to clear them inside the same atomic request
		instead of a second one the boss could start a pattern between. */
		bool Break_EveryWallForAudition(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		/* The navigation grid is the ground a boss pattern stride may cross.
		The collision sweep owns wall contact, while the furthest sample the grid
		still owns is what any stride is allowed to reach, so a charge cannot
		leave the floor before its wall contact is evaluated. A start the grid
		already refuses passes through
		unchanged, because refusing it there would strand the boss for good. */
		static void Resolve_NavigableStep(
			const CServerNavigation& navigation,
			float fromX,
			float fromZ,
			float targetX,
			float targetZ,
			float& outX,
			float& outZ);
		/* Raise the pillar slots on the authored stage edge of the pattern that
		owns them. The shatter has no identified product owner yet. */
		bool Apply_EncounterPropStageEntry(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Commit_DueEncounterProps(std::uint32_t serverTick);
		bool Initialize_WorldPickups();
		void Reset_WorldPickups(std::uint32_t serverTick);
		void Update_WorldPickups(std::uint32_t serverTick, bool allowCollection = true);
		void Apply_WorldPickupDestruction(const WORLD_DESTRUCTION_TRANSACTION& transaction,
			std::uint32_t serverTick);
		void Remove_RemainingWorldPickups(std::uint32_t serverTick);
		bool Send_EncounterPropSync(
			const std::shared_ptr<CClientSession>& session);
		void Broadcast_EncounterPropSync();
		/* Break whatever a non-impact boss body physically reached between its
		previous and current position. A charge-impact stage bypasses this generic
		pass and owns one exact swept wall transaction: impact receiver first,
		then the co-located ordinary contact binding. */
		bool Apply_WorldDestructionBodyContact(
			SERVER_WORLD_ENTITY& boss,
			float previousX,
			float previousY,
			float previousZ,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionPatternHitContact(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionContacts(
			SERVER_WORLD_ENTITY& boss,
			const std::vector<std::string>& contactPlacementIds,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionImpact(
			SERVER_WORLD_ENTITY& boss,
			const std::string& receiverPlacementId,
			std::uint32_t serverTick,
			bool& outTriggered);
		bool Commit_WorldDestructionTransaction(
			const WORLD_DESTRUCTION_TRANSACTION& transaction,
			const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::uint32_t serverTick,
			std::string& status);
		void Invalidate_DynamicNavigationPaths();
		bool Build_WorldDestructionLiveEvents(
			const WORLD_DESTRUCTION_TRANSACTION& transaction,
			const SERVER_WORLD_ENTITY& boss,
			std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::string& status) const;
		bool Commit_DueWorldDestruction(std::uint32_t serverTick);
		bool Activate_Encounter(const std::string& placementId);
		bool Spawn_Monster(
			const std::string& spawnGroupId,
			const SPAWN_GROUP_ENTRY& entry,
			const SPAWN_GROUP_ANCHOR& anchor,
			const MONSTER_RUNTIME_PROFILE& profile,
			std::uint32_t ordinal);
		/* Card maze. The telescope claim deals the suits and raises the
		targets; the MAZE hammer press judges its swing once, at the runtime's
		hit tick, against those targets. */
		bool Spawn_KoukuCardRainSoldiers(LostArk::Shared::NET_ENTITY_ID ownerId, std::uint32_t tick,
			const BOSS_PATTERN_MECHANIC_TRIGGER* tuning = nullptr);
		void Update_KoukuCardRainSoldiers(std::uint32_t tick);
		bool Begin_CardMaze(LostArk::Shared::PLAYER_ID claimantId);
		void Reset_CardMaze();
		void Despawn_CardMazeTargets();
		void Resolve_CardMazeHammerHit(SERVER_PLAYER& player, std::uint32_t updateTick);
		void Resolve_MarioHammerHit(SERVER_PLAYER& player, std::uint32_t updateTick);
		void Update_MarioBombContacts(SERVER_PLAYER& player, std::uint32_t updateTick);
		void Update_MarioBouncingBallContacts(SERVER_PLAYER& player, std::uint32_t updateTick);
		// Rotating cannon jets and the big mokoko waterfall of a live Waterpang match.
		void Update_MaharakaWaterpangHazards(SERVER_PLAYER& player, std::uint32_t updateTick);
        void Handle_MaharakaAITuning(SESSION_ID sessionId, const LostArk::Shared::C2S_MAHARAKA_AI_TUNING& request);
        bool Spawn_MaharakaWaterpangAI();
        void Update_MaharakaWaterpangMatch(std::uint32_t updateTick);
        void Clear_MaharakaWaterpangAI(std::uint32_t keepCount = 0u);
        bool Finish_MaharakaWaterpangMatch();
		// The running Debug forced event first, else the match schedule; false when neither runs.
		bool Sample_MaharakaWaterpangNow(std::uint32_t tick, LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& out) const;
		// The Server's water gun arming rule: replicated as PLAYER_SNAPSHOT.isWaterpangArmed.
		bool Is_MaharakaWaterpangArmed(const SERVER_PLAYER& player) const;
		// Water gun Q/W/E/R cast of an armed body: cooldown, no movement lock, shot scheduled.
		bool Try_StartMaharakaWaterGunSkill(SERVER_PLAYER& player, const LostArk::Shared::C2S_USE_SKILL& command);
		// Flies the scheduled shots and pushes the bodies they strike.
		void Update_MaharakaWaterGunShots(std::uint32_t updateTick);
		// The forced event plus a short tail, so its last push can still leave the deck.
		bool Is_MaharakaWaterpangDebugEventLive(std::uint32_t tick) const;
		// Debug F1 Waterpang pattern button: starts a forced event for the whole room.
		LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT Start_MaharakaWaterpangDebugEvent(const std::string& instanceId);
		std::uint8_t Mario_CurseReleasedMask(std::uint8_t stage, std::uint8_t layout) const;
		bool Spawn_CardMazeTarget(const CKoukuCardMazeRuntime::SPAWN_REQUEST& request);
		void Remove_CardMazeTarget(LostArk::Shared::NET_ENTITY_ID id);
		void Update_CardMaze(std::uint32_t tick);
		/* Before the run: raises the clown box for players inside the maze and
		latches its destruction. Clear forgets it and removes a living box. */
		void Update_CardMazeClownBox(std::uint32_t tick);
		void Clear_CardMazeClownBox();
		/* Advances the bingo bomb clock: a mark whose deadline passed is
		planted where its carrier stands, and a carrier that left the room
		takes its mark with it. */
		void Update_KoukuBingo(std::uint32_t tick);
		bool Begin_CardMazeTransfer(SERVER_PLAYER& player, float x, float y, float z,
			std::uint32_t tick, bool leaving);
		std::uint32_t Count_SpawnGroupEntities(
			const std::string& spawnGroupId) const;
		/* 1 unless the player is standing in the stance its identity gauge pays
		for, which is the only thing that changes how fast anyone walks. */
		float Resolve_StanceMoveSpeedScale(const SERVER_PLAYER& player) const;
		/* Hands the living monster and boss bodies to the collision system so this
		tick's player walks and root motion stop at them. */
		void Refresh_PlayerBlockingBodies();
		bool Try_KoukuWalkOffFloor(SERVER_PLAYER& player, float x, float z,
			float fixedDeltaSeconds, std::uint32_t updateTick);
		const WORLD_BOOTSTRAP_PLACEMENT* Resolve_KoukuFallCenter(const SERVER_PLAYER& player) const;
		bool Update_PlayerFall(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds,
			std::uint32_t updateTick);
		void Begin_PlayerFall(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds,
			std::uint32_t updateTick);
		/* Product boss-pattern adapters call these with replicated identities. The
		room owns interruption, fixed-tick fallback motion and release reaction so
		no pattern can leave half of a grabbed player state behind. */
		bool Capture_PlayerAttachment(
			LostArk::Shared::NET_ENTITY_ID playerEntityId,
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
			std::uint32_t serverTick, std::uint32_t holdEndTick = 0u, std::uint32_t sourcePatternSequence = 0u);
		bool Update_PlayerAttachment(
			SERVER_PLAYER& player,
			std::uint32_t serverTick);
		bool Release_PlayerAttachment(
			SERVER_PLAYER& player,
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			float pushRangeM,
			std::uint32_t pushMs,
			bool knockdown,
			std::uint32_t downMs,
			std::uint32_t serverTick);
		std::size_t Release_PlayerAttachments(
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			float pushRangeM,
			std::uint32_t pushMs,
			bool knockdown,
			std::uint32_t downMs,
			std::uint32_t serverTick);
		[[nodiscard]] bool Restore_PatternBoundPlayer(SERVER_PLAYER& player);
		void Update_Players(float fixedDeltaSeconds);
		bool Prepare_ArenaEjection(
			SERVER_PLAYER& staged,
			const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_ACTION& action,
			std::uint32_t serverTick);
		bool Resolve_ArenaCenter(
			const SERVER_WORLD_ENTITY& boss,
			SERVER_NAV_POINT& point);
		bool Activate_ValtanGhostPhaseLoop(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog);
		bool Begin_ValtanGhostRelocation(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick);
		bool Update_ValtanGhostPortalScheduler(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick);
		bool Update_DependentBosses(std::uint32_t serverTick);
		/* Slides a hit player along the armed knockback window, clamped to
		walkable floor and blocking bodies; a wall ends the window early. */
		void Advance_PlayerKnockback(
			SERVER_PLAYER& player, float fixedDeltaSeconds);
		void Update_WorldEntities(float fixedDeltaSeconds);

	private:
		// Best-effort traffic leaves room for gameplay/control commands. LEAVE
		// never shares this bounded queue: disconnect cleanup has its own
		// session-deduplicated priority queue below.
		static constexpr std::size_t MAX_BEST_EFFORT_COMMAND_COUNT = 768u;
		static constexpr std::size_t MAX_RELIABLE_COMMAND_COUNT = 960u;
		static constexpr std::size_t MAX_COMMANDS_DRAINED_PER_TICK = 256u;

		mutable std::mutex m_CommandMutex;
		std::deque<ROOM_COMMAND> m_InboundCommands;
		std::deque<ROOM_COMMAND> m_CleanupCommands;
		std::unordered_set<SESSION_ID> m_QueuedCleanupSessionIds;
		SERVER_ROOM_PERFORMANCE_METRICS m_PerformanceMetrics;
		SERVER_ROOM_PERFORMANCE_METRICS m_LastRoomPerfLogSample;
		std::string m_strPendingPerformanceDiagnostic;
		std::uint64_t m_iLastRoomPerfSnapshotDroppedCount = 0;
		std::uint64_t m_iLastRoomPerfReliableRejectedCount = 0;
		std::uint64_t m_iLastRoomPerfWireSendFailureCount = 0;
		std::size_t m_iLastRoomPerfOutboundHighWatermark = 0u;
		bool m_acceptsCommands = true;
		std::deque<SERVER_WORLD_TRANSFER_REQUEST> m_PendingWorldTransfers;
		/* Bern remembers the ship a session sailed to Maharaka on, so the return trip can put that
		   session back on the same ship at the pier it left from. Bern room only; keyed by session. */
		struct SHIP_RETURN_STATE final
		{
			LostArk::Shared::VEHICLE_ID iVehicleId = LostArk::Shared::INVALID_VEHICLE_ID;
			float fDockX = 0.f;
			float fDockY = 0.f;
			float fDockZ = 0.f;
			float fDockYawDegrees = 0.f;
		};
		std::unordered_map<SESSION_ID, SHIP_RETURN_STATE> m_MaharakaShipReturnBySession;
		void Remember_ShipForWorldTransfer(const SERVER_WORLD_TRANSFER_REQUEST& transfer);
		struct PENDING_ESTHER_SUMMON final
		{
			const ESTHER_ROSTER_ENTRY* pRosterEntry = nullptr;
			LostArk::Shared::PLAYER_ID iCasterPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
			float fPositionX = 0.f;
			float fPositionY = 0.f;
			float fPositionZ = 0.f;
			float fYawDegrees = 0.f;
			float fRemainingSeconds = 0.f;
		};
		std::vector<PENDING_ESTHER_SUMMON> m_PendingEstherSummons;
		struct ESTHER_ZONE_RUNTIME final
		{
			const LostArk::Shared::EstherStrike::ZONE* pZone = nullptr;
			float fPositionX = 0.f;
			float fPositionZ = 0.f;
			std::uint32_t iEndTick = 0u;
			std::uint32_t iNextPulseTick = 0u;
		};
		std::vector<ESTHER_ZONE_RUNTIME> m_EstherZones;

		std::unordered_map<SESSION_ID, std::weak_ptr<CClientSession>> m_Sessions;
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER> m_Players;
		struct GUIDE_PROMPT_OCCURRENCE
		{
			std::string PromptId, TriggerId;
			std::size_t NextSegment = 0;
			int Priority = 0;
			bool IsSpaceEnter = false;
			std::vector<std::string> SpaceTriggerIds;
			bool operator==(const GUIDE_PROMPT_OCCURRENCE&) const = default;
		};
		struct GUIDE_RUNTIME
		{
			LostArk::Shared::PLAYER_ID PlayerId = 0, AnchorId = 0;
			std::weak_ptr<CClientSession> OwnerSession;
			LostArk::Shared::NET_ENTITY_ID OwnerNetEntityId = 0;
			bool WaitingForOwner = false, WaitingForShip = false, ReturningOnFoot = false;
			std::string ComboId, PendingComboId, Reason;
			std::size_t ComboStep = 0;
			float ThinkElapsed = 0.f, ComboElapsed = 0.f, StepElapsed = 0.f, FarElapsed = 0.f, HoldElapsed = 0.f, PromptRemaining = 0.f;
			std::uint32_t Sequence = 0;
            std::map<std::string, std::uint32_t> CommandTicks;
			std::uint8_t Action = 0;
            float FollowScore = 0.f, EvadeScore = 0.f, CombatScore = 0.f;
			std::deque<GUIDE_PROMPT_OCCURRENCE> PromptQueue;
			std::map<std::string, std::uint32_t> TriggerTicks;
			std::unordered_set<std::string> InsideBoxes;
			std::map<LostArk::Shared::NET_ENTITY_ID, std::uint32_t> PatternSequences;
		};
		CGuideCatalog m_GuideCatalog;
		// At most one owner binds the one placed Bern guide; no companion clone or party slot.
		std::map<SESSION_ID, GUIDE_RUNTIME> m_PersonalGuides;
		std::uint32_t m_iGuideEventSequence = 0;
		float m_fGuideOwnershipElapsed = 0.f;
		std::unordered_map<SESSION_ID, std::uint32_t> m_GuideControlSequences;
		LostArk::Shared::PLAYER_ID m_iGuideReceptionId = 0;
        LostArk::Shared::PLAYER_ID m_iNextGuidePlayerId = 0x80000000u;
		/* Grants what a started skill buffs, to the caster, the party in this room
		or the entities it targets. */
		void Apply_SkillBuffs(SERVER_PLAYER& caster, std::uint32_t skillId,
			std::uint32_t serverTick);
		std::unordered_map<SESSION_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdBySessionId;
		std::unordered_map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdByEntityId;

		/* Same-room party state -- PLAYER_ID is room-local (freshly allocated
		   per room on Join), so this map does not by itself survive a member
		   moving to a different room. 0 means "no party" -- never a real party
		   ID. Invite/accept/join only; leave/kick/leader promotion is a
		   separate follow-up.
		   A party-leader-triggered group Valtan entry (Handle_ConfirmNpcEntry
		   -> Transfer_PartyTo) is the one case
		   that does survive a room change: every member transfers together in
		   one batch and gets re-grouped into a fresh room-local party in the
		   target room, so the party itself is never actually split across two
		   rooms at once. There is still no general cross-room party identity
		   (e.g. inviting or chatting with someone in a different room). */
		std::uint32_t m_iNextPartyId = 1u;
		std::unordered_map<LostArk::Shared::PLAYER_ID, std::uint32_t>
			m_PartyIdByPlayerId;
		std::unordered_map<std::uint32_t, std::vector<LostArk::Shared::PLAYER_ID>>
			m_PartyMembersByPartyId;
		// One pending invite per target at a time; a new invite silently
		// replaces whatever that target's last unanswered invite was.
		std::unordered_map<LostArk::Shared::PLAYER_ID, LostArk::Shared::PLAYER_ID>
			m_PendingPartyInviteByTargetPlayerId;
		// At most one latest failure per present player. A full reliable queue
		// delays the notice instead of disconnecting a rejected source party.
		std::unordered_map<SESSION_ID, LostArk::Shared::S2C_PARTY_TRANSFER_RESULT>
			m_PendingPartyTransferResults;

		// 파티 레이드 입장 투표 상태. struct RAID_ENTRY_PROPOSAL은 위 메서드 선언부에 정의한다.
		std::vector<RAID_ENTRY_PROPOSAL> m_RaidEntryProposals;
		std::uint32_t m_iNextRaidEntryProposalId = 1u;
		// Colosseum match queue: accepted sessions in join order (BERN room only).
		struct COLOSSEUM_QUEUE_ENTRY
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			std::uint32_t iRequestSequence = 0u;
		};
		std::vector<COLOSSEUM_QUEUE_ENTRY> m_ColosseumQueue;
		bool m_bColosseumTransferPending = false;
		std::uint32_t m_iColosseumRetryTick = 0u;
		std::uint64_t m_iColosseumMatchId = 0u;
		LostArk::Shared::COLOSSEUM_MATCH_PHASE m_eColosseumPhase = LostArk::Shared::COLOSSEUM_MATCH_PHASE::RECRUITING;
		std::uint8_t m_iColosseumWinnerTeam = 255u;
		std::uint32_t m_iColosseumRevision = 1u;
		std::array<std::uint32_t, 2> m_ColosseumTeamPartyIds{};
		struct COLOSSEUM_MERCENARY_RUNTIME final
		{
			float fThinkElapsed = 0.f;
			std::uint32_t iSequence = 0u;
			// Match-owned admission clock survives player respawn and rotation resets.
			std::optional<std::uint32_t> iLastAltVAdmissionTick;
			std::size_t iSkillCursor = 0u;
			std::vector<LostArk::Shared::SKILL_ID> ComboSkills;
			float fComboElapsed = 0.f, fStepWaitElapsed = 0.f;
			float fComboTimeout = 45.f, fStepWaitTimeout = 5.f;
			const char* pReason = "Waiting for recruitment";
		};
		std::map<LostArk::Shared::PLAYER_ID, COLOSSEUM_MERCENARY_RUNTIME> m_ColosseumMercenaries;
		std::unordered_map<SESSION_ID, std::uint32_t> m_ColosseumRecruitSequences;
		std::vector<SESSION_ID> m_ColosseumSessions;
		std::uint32_t m_iColosseumPhaseStart = 0u, m_iColosseumPhaseEnd = 0u;
		std::uint32_t m_iColosseumScores[2]{};
		std::uint32_t m_iColosseumKillSequence = 0u;
		std::vector<LostArk::Shared::COLOSSEUM_KILL_EVENT> m_ColosseumRecentKills;
		std::uint32_t m_iColosseumQueueDeadline = 0u, m_iColosseumQueueBroadcastTick = 0u;
		std::array<std::uint8_t, 2> m_ColosseumInitialHumans{};
		GATE_PROGRESS_STATE m_GateProgress;
		std::uint32_t m_iArenaAssemblyStartTick = 0u;
		std::uint32_t m_iArenaAssemblyRaidEpoch = 0u;
		bool m_bArenaAssemblyAttempted = false;
		std::vector<LostArk::Shared::PLAYER_ID> m_ArenaAssemblyParticipants;
		std::vector<SERVER_MVP_LEDGER_ROW> m_GateMvpLedger;
		std::uint32_t m_iNextGateProposalId = 1u;

		LostArk::Shared::WORLD_ID m_eWorldId = LostArk::Shared::WORLD_ID::END;
		CWorldBootstrap m_WorldBootstrap;
		CGameplayCatalogGenerations m_GameplayCatalog;
		std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY> m_StagedNumericEntries;
		std::vector<std::pair<std::shared_ptr<const CGameplayCatalog>, std::shared_ptr<const CGameplayCatalog>>>
			m_StagedNumericCatalogRemaps;
		CItemCatalog m_ItemCatalog;
		CVehicleCatalog m_VehicleCatalog;
		CHonorTitleCatalog m_HonorTitleCatalog;
		CValtanClearRewards m_ValtanClearRewards;
		CServerNavigation m_ServerNavigation;
		CServerCollisionSystem m_ServerCollisionSystem;
		CServerTriggerSystem m_ServerTriggerSystem;
        struct MAHARAKA_WATERPANG_AI final
        {
            std::uint32_t iSlot = 0u, iSequence = 0u, iNextThinkTick = 0u, iNextMoveTick = 0u, iNextShotTick = 0u, iSkillSlot = 0u;
        };
        std::map<LostArk::Shared::PLAYER_ID, MAHARAKA_WATERPANG_AI> m_MaharakaWaterpangAI;
        LostArk::Shared::PLAYER_ID m_iNextWaterpangAIPlayerId = 0x90000000u;
        std::uint32_t m_iWaterpangAIRetryTick = 0u;
        LostArk::Shared::MAHARAKA_AI_TUNING m_MaharakaAITuning;
        bool m_bMaharakaAITuningLoaded = false;
        std::string m_strMaharakaAISourceBytes;
		// One room-wide scheduled intro, retained for late join until the room empties.
		std::optional<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> m_MaharakaWaterpangIntro;
		// A Waterpang water gun shot from cast to burst; bodies already struck are remembered.
		struct MAHARAKA_WATERGUN_SHOT final
		{
			LostArk::Shared::PLAYER_ID iOwnerId = 0;
			std::uint32_t iSkillId = 0u;
			std::uint32_t iSpawnTick = 0u;
            std::uint32_t iProjectileIndex = 0u;
            LostArk::Shared::COMBAT_OBJECT_ID iVisualObjectId = LostArk::Shared::INVALID_COMBAT_OBJECT_ID;
            LostArk::Shared::NET_ENTITY_ID iSourceNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			float fAimDistanceM = 0.f;
			bool bLaunched = false;
			bool bSpent = false;
			float fX = 0.f, fY = 0.f, fZ = 0.f;
			float fDirX = 0.f, fDirZ = 1.f;
			float fTravelM = 0.f;
			float fReachM = 0.f;
			std::vector<LostArk::Shared::PLAYER_ID> Struck;
		};
		std::vector<MAHARAKA_WATERGUN_SHOT> m_MaharakaWaterGunShots;
		// Debug forced waterfall/cannon broadcast; replaced by the next press, kept for late join.
		std::optional<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> m_MaharakaWaterpangDebugEvent;
		CSpawnGroupBootstrap m_SpawnGroupBootstrap;
		CSpawnGroupRuntime m_SpawnGroupRuntime;
		std::mt19937 m_MarioLayoutRandom{std::random_device{}()};
		// Popped source-ball slots per Mario stage (index 1..4), bit = bootstrap slot.
		std::uint16_t m_MarioPoppedBalls[5] = {};
		std::uint8_t m_iNextMarioEntryStage = 1u;
		struct KOUKU_CARD_RAIN_SOLDIER_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID ownerId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t patternSequence = 0u, expiresAt = 0u;
		};
		std::map<LostArk::Shared::NET_ENTITY_ID, KOUKU_CARD_RAIN_SOLDIER_STATE> m_KoukuCardRainSoldiers;
		CKoukuCardMazeRuntime m_KoukuCardMaze;
		CKoukuBingoRuntime m_KoukuBingo;
        struct KOUKU_BINGO_DURATION final
        {
            LostArk::Shared::NET_ENTITY_ID iOwnerId = LostArk::Shared::INVALID_NET_ENTITY_ID;
            std::uint32_t iPatternSequence = 0u, iEndTick = 0u;
            std::uint32_t iNextBombTick = 0u, iNextHammerTick = 0u, iNextMadnessTick = 0u;
            std::uint32_t iMarkedBombCount = 0u;
            float fHammerHalfForwardM = 0.f, fHammerHalfWidthM = 0.f;
            bool bEncounterOwned = false, bSpecialPatternPending = false;
            bool bLastLineCompletionSucceeded = false;
            bool bLineRewardSinceLastJudgement = false;
            std::uint32_t iLastLineJudgementTick = 0u;
            struct HAMMER { std::int32_t anchor = -1; std::uint32_t startTick = 0u; };
            std::array<HAMMER, 2u> Hammers{};
        } m_KoukuBingoDuration;
        std::uint32_t m_iKoukuBingoBoardEpoch = 0u;
        void Begin_KoukuBingoDuration(const SERVER_WORLD_ENTITY& owner,
            const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t tick);
        void Stop_KoukuBingoDuration(bool clearBoard);

		std::uint32_t m_iCardMazeMarchStartTick = 0u;
		std::uint32_t m_iCardMazeCycleMs = 0u;
		std::map<LostArk::Shared::PLAYER_ID, std::pair<float, float>> m_CardMazePreviousPositions;
		std::map<LostArk::Shared::PLAYER_ID, std::uint32_t> m_CardMazeContactTicks;
		LostArk::Shared::NET_ENTITY_ID m_iCardMazeClownBoxId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t m_iCardMazeClownBoxDueTick = 0u;
		bool m_bCardMazeClownBoxDestroyed = false;
		CPlayerSkillSystem m_PlayerSkillSystem;
		CCombatObjectRuntime m_CombatObjectRuntime;
		CMonsterBrain m_MonsterBrain;
		CNpcBehaviorRuntime m_NpcBehaviorRuntime;
		CValtanBrain m_ValtanBrain;
		CKoukuSaydonBrain m_KoukuSaydonBrain;
		std::unique_ptr<CValtanBrain> m_DependentValtanBrain =
			std::make_unique<CValtanBrain>();
		VALTAN_DECISION_TRACE_REVISION_STATE m_ValtanDecisionTraceRevision;
		CEstherSkillSystem m_EstherSkillSystem;
		CWorldDestructionBootstrap m_WorldDestructionBootstrap;
		CWorldDestructionRuntime m_WorldDestructionRuntime;
		struct WORLD_PICKUP_RUNTIME final
		{
			WORLD_PICKUP_DESCRIPTOR Descriptor;
			LostArk::Shared::WORLD_PICKUP_SNAPSHOT Snapshot;
		};
		std::vector<WORLD_PICKUP_RUNTIME> m_WorldPickups;
		std::uint32_t m_iWorldPickupEncounterEpoch = 0u;
		/* The four pillars come back four times in one fight, so they live in a
		reversible prop runtime instead of a one-way destruction group. */
		CEncounterPropRuntime m_EncounterPropRuntime;
		/* Room-authoritative completion latch. The primary Product Valtan death
		   raises it before that entity is reliably despawned; the last-player reset
		   clears it for the next party. */
		bool m_bValtanRaidCleared = false;
		/* Debug audition only: the tick a whole pillar cycle shatters on, and
		the flag the next raise turns into that tick. No product trigger for the
		shatter is identified yet, so nothing else writes these. */
		std::uint32_t m_iPillarAuditionBreakTick = 0u;
		bool m_bPillarAuditionCycleArmed = false;
		std::vector<SERVER_WORLD_ENTITY> m_WorldEntities;
		/* One tick's resolved hits. Cleared at the top of every simulation phase
		and consumed by Broadcast_WorldSnapshot, so an event can only ever ride
		the snapshot of the tick that produced it. */
		std::vector<LostArk::Shared::DAMAGE_EVENT> m_TickDamageEvents;
		/* Damage-text events raised while draining room commands, which happens before
		m_TickDamageEvents is cleared for the tick. Moved in right after that clear so a
		potion heal reaches the same broadcast as a combat hit. */
		std::vector<LostArk::Shared::DAMAGE_EVENT> m_PendingCommandDamageEvents;
		std::vector<LostArk::Shared::BOSS_COMBAT_EVENT>
			m_TickBossCombatEvents;
		std::string m_strStatus;
		SERVER_ROOM_RUNTIME_FAILURE m_RuntimeFailure;
		bool m_isReady = false;

		LostArk::Shared::PLAYER_ID m_iNextPlayerId = 1;
		LostArk::Shared::NET_ENTITY_ID m_iNextNetEntityId = 100;
		std::uint32_t m_iServerTick = 0;
		std::uint64_t m_iNextWorldDestructionEventSequence = 1u;
		std::uint64_t m_iNextBossCombatEventSequence = 1u;
		/* Debug Valtan audition. The armed bar is the one an ARM parked the boss
		above; a CROSS is only honoured for that same bar, so a crossing can
		never span an unknown number of authored thresholds. Both reset with the
		encounter, and the handled sequences reject a resent request instead of
		replaying it. Stable-ID pattern requests have an independent ledger because
		the Effect Tool and the Valtan level own independent sequence counters. */
		std::uint32_t m_iValtanAuditionArmedHealthBar = 0;
		std::unordered_map<SESSION_ID, std::uint32_t>
			m_ValtanAuditionSequenceBySessionId;
		struct VALTAN_PATTERN_ID_COMMAND_RECEIPT final
		{
			LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request;
			LostArk::Shared::VALTAN_AUDITION_RESULT Result =
				LostArk::Shared::VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
			std::uint32_t iCurrentHealthBar = 0u;
			/* A QUEUED receipt must remain reconcilable after its occurrence is no
			   longer the room's current audition. Keep the last authoritative edge
			   so an exact retry cannot loop on a verdict without lifecycle. */
			std::optional<LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE>
				LastLifecycle;
		};
		/* Stable-ID Play/Restart keeps the exact payload and verdict. A retry of
		   one identity replays that verdict; an altered tuple never inherits it. */
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_ID_COMMAND_RECEIPT>
			m_ValtanPatternIdAuditionSequenceBySessionId;
		struct VALTAN_PATTERN_FLOW_COMMAND_RECEIPT final
		{
			std::uint32_t iSequence = 0u;
			std::uint32_t iRoomFlowEpoch = 0u;
			std::string strFlowId;
			std::string strFlowRevision;
			std::string strRequestIdentity;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT eResult =
				LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW;
			std::string strReason;
			/* Exact Start retries replay the latest authoritative edge for that
			   admitted program. This settles an unconfirmed Client even when the
			   Flow already reached COMPLETED_HOLD; it never starts a second run. */
			std::optional<LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE>
				LastLifecycle;
		};
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT>
			m_ValtanPatternFlowStartSequenceBySessionId;
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT>
			m_ValtanPatternFlowControlSequenceBySessionId;
		struct TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE Message;
		};
		std::uint32_t m_iNextKoukuSaydonPatternAuditionEpoch = 1u;
		struct KOUKU_DRAFT_UPLOAD final
		{
			std::uint32_t iRequestSequence = 0u, iTotalBytes = 0u;
			std::uint64_t iStartedAtMs = 0u;
			LostArk::Shared::GameplayDataRevision RowsRevision{};
			std::string Rows;
		};
		std::unordered_map<SESSION_ID, KOUKU_DRAFT_UPLOAD> m_KoukuDraftUploads;
		KOUKUSAYDON_PATTERN_AUDITION_STATE m_KoukuSaydonPatternAudition;
		std::shared_ptr<const CGameplayCatalog> m_pKoukuPublishedProductGeneration;
		std::unordered_map<SESSION_ID, KOUKUSAYDON_PATTERN_AUDITION_RECEIPT>
			m_KoukuSaydonPatternAuditionReceiptBySessionId;
		std::vector<TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE>
			m_PendingKoukuSaydonPatternAuditionLifecycle;
		struct TARGETED_VALTAN_AUDITION_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE Message;
		};
		std::uint32_t m_iNextValtanAuditionEpoch = 1u;
		std::vector<TARGETED_VALTAN_AUDITION_LIFECYCLE>
			m_PendingValtanAuditionLifecycle;
		struct TARGETED_VALTAN_PATTERN_FLOW_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE Message;
		};
		std::uint32_t m_iNextValtanPatternFlowEpoch = 1u;
		std::vector<TARGETED_VALTAN_PATTERN_FLOW_LIFECYCLE>
			m_PendingValtanPatternFlowLifecycle;
		VALTAN_PATTERN_ID_AUDITION_STATE m_ValtanPatternIdAudition;
		std::optional<VALTAN_NEXT_PATTERN_RESERVATION> m_ValtanNextPattern;
		std::unordered_map<SESSION_ID, VALTAN_NEXT_PATTERN_COMMAND_RECEIPT>
			m_ValtanNextPatternReceiptBySessionId;
		VALTAN_PATTERN_FLOW_AUDITION_STATE m_ValtanPatternFlowAudition;
		VALTAN_TIMELINE_AUDITION_STATE m_ValtanTimelineAudition;
		VALTAN_FIGHT_PAGE_START_STATE m_ValtanFightPageStart;
	};
}
```

### G11. Server/Private/GameRoom_Guide.cpp 전체 코드

```cpp
#include "GameRoom.h"
#include "ClientSession.h"
#include "ServerCombatGeometry.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

using namespace LostArk::Server;
using namespace LostArk::Shared;
namespace
{
 constexpr float PI = 3.14159265359f;
 float distance(const SERVER_PLAYER& a,const SERVER_PLAYER& b){return std::hypot(a.fPositionX-b.fPositionX,a.fPositionZ-b.fPositionZ);}
 std::uint32_t ticks(std::uint32_t ms){return (ms*30u+999u)/1000u;}
 bool isMinigameAnchor(const SERVER_PLAYER& player){return player.iMarioStage!=0||player.eCardMazeRole!=CARD_MAZE_ROLE::NONE||player.eKoukuHudMode==KOUKU_HUD_MODE::MARIO||player.eKoukuHudMode==KOUKU_HUD_MODE::DANCE||player.eKoukuHudMode==KOUKU_HUD_MODE::MAZE;}
 std::string trim(const std::string& value){auto a=value.find_first_not_of(" \t\r\n");auto b=value.find_last_not_of(" \t\r\n");return a==std::string::npos?std::string{}:value.substr(a,b-a+1);}
}

std::size_t CGameRoom::Count_HumanPlayers() const
{
 return std::count_if(m_Players.begin(),m_Players.end(),[](const auto& p){return p.second.Is_Human();});
}

bool CGameRoom::Find_GuideLanding(const SERVER_PLAYER& guide,float x,float y,float z,SERVER_NAV_POINT& point,const SERVER_PLAYER* anchor) const
{
 if(anchor&&!m_ServerNavigation.Is_Loaded())return false;
 // Deterministic rings share the normal navigation and actual collision admission.
 for(unsigned sample=0;sample<49;++sample)
 {
  const float radius=sample?(.75f+static_cast<float>((sample-1)/12)*.75f):0.f;
  const float angle=static_cast<float>(sample%12)*PI/6.f;
  SERVER_NAV_POINT candidate{x+std::sin(angle)*radius,y,z+std::cos(angle)*radius};
  if(m_ServerNavigation.Is_Loaded())
  {
   // Sampling height alone does not include live navigation blockers.
   if(!m_ServerNavigation.Is_PointWalkableExact(candidate.x,candidate.z,y)||
      !m_ServerNavigation.Sample_Position(candidate.x,candidate.z,candidate,y)||std::abs(candidate.y-y)>2.f)continue;
   // A nearby island or another overlapping deck is not a usable follow landing.
   if(anchor&&!m_ServerNavigation.Has_LineOfSight(anchor->fPositionX,anchor->fPositionZ,candidate.x,candidate.z,anchor->fPositionY))continue;
  }
  if(!m_ServerCollisionSystem.Is_PlayerPositionClear(candidate.x,candidate.y,candidate.z,guide.iNetEntityId))continue;
  bool overlap=false;
  for(const auto& [id,other]:m_Players)if(id!=guide.iPlayerId&&id!=m_iGuideReceptionId&&other.iCurrentHp&&std::abs(other.fPositionY-candidate.y)<1.5f&&std::hypot(other.fPositionX-candidate.x,other.fPositionZ-candidate.z)<.75f){overlap=true;break;}
  if(!overlap){point=candidate;return true;}
 }
 return false;
}

bool CGameRoom::Build_GuidePlayer(PLAYER_ID playerId,NET_ENTITY_ID entityId,float x,float y,float z,SERVER_PLAYER& out) const
{
 const auto* profile=m_GameplayCatalog.Find_Player(CHARACTER_CLASS_ID::DIMENSIONMASTER);
 if(!m_GuideCatalog.Loaded||!profile||!playerId||!entityId||(!m_Players.contains(playerId)&&m_Players.size()>=MAX_WORLD_SNAPSHOT_PLAYERS))return false;
 SERVER_PLAYER guide;
 guide.eControlKind=PLAYER_CONTROL_KIND::GUIDE_AI;guide.iPlayerId=playerId;guide.iNetEntityId=entityId;
 guide.eCharacterClass=CHARACTER_CLASS_ID::DIMENSIONMASTER;guide.strNickName=m_GuideCatalog.Name;
 guide.strSpawnPlacementId=m_GuideCatalog.PlacementId;guide.eStance=profile->eDefaultStance;
 guide.iCurrentHp=guide.iMaximumHp=profile->iMaximumHp;guide.iCurrentResource=guide.iMaximumResource=profile->iMaximumResource;
 guide.iMaximumIdentity=profile->iMaximumIdentity;guide.fMoveSpeed=profile->fMoveSpeed;guide.fYawDegrees=m_GuideCatalog.Yaw;
 CPlayerSkillSystem::Reset_Gauges(guide,m_GameplayCatalog);guide.isCombatReady=true;
 SERVER_NAV_POINT position;
 if(!Find_GuideLanding(guide,x,y,z,position))return false;
 guide.fPositionX=position.x;guide.fPositionY=position.y;guide.fPositionZ=position.z;
 out=std::move(guide);return true;
}

void CGameRoom::Initialize_Guide()
{
 if(m_eWorldId!=WORLD_ID::BERN&&m_eWorldId!=WORLD_ID::VALTAN_ARENA&&m_eWorldId!=WORLD_ID::KAKULSAYDON_ARENA&&m_eWorldId!=WORLD_ID::COLOSSEUM)return;
 std::string status;
 if(!m_GuideCatalog.Load(status)){std::cout<<"[Guide] "<<status<<'\n';return;}
 // Class/slot references are checked again against the actual installed Server generation.
 for(const auto& combo:m_GuideCatalog.Combos)
 {
  if(combo.Skills.size()!=combo.Slots.size()){m_GuideCatalog.Loaded=false;std::cout<<"[Guide] unresolved combo "<<combo.Id<<'\n';return;}
  for(std::size_t i=0;i<combo.Skills.size();++i){const auto* skill=m_GameplayCatalog.Find_Skill(combo.Skills[i]);if(!skill||skill->eCharacterClass!=CHARACTER_CLASS_ID::DIMENSIONMASTER||skill->strInputSlot!=combo.Slots[i]){m_GuideCatalog.Loaded=false;std::cout<<"[Guide] published skill binding mismatch "<<combo.Id<<'\n';return;}}
 }
 if(m_eWorldId!=WORLD_ID::BERN)return;
 SERVER_PLAYER reception;
 if(!Build_GuidePlayer(m_iNextGuidePlayerId,m_iNextNetEntityId,m_GuideCatalog.Position[0],m_GuideCatalog.Position[1],m_GuideCatalog.Position[2],reception)){std::cout<<"[Guide] reception position is unavailable\n";return;}
 reception.isCombatReady=false;m_iGuideReceptionId=reception.iPlayerId;
 m_PlayerIdByEntityId.emplace(reception.iNetEntityId,reception.iPlayerId);m_Players.emplace(reception.iPlayerId,std::move(reception));++m_iNextGuidePlayerId;++m_iNextNetEntityId;
}

bool CGameRoom::Start_Guide(const SERVER_PLAYER& inviter,const NET_ENTITY_ID target)
{
 const auto targetId=m_PlayerIdByEntityId.find(target);
 if(targetId==m_PlayerIdByEntityId.end())return false;
 const auto actor=m_Players.find(targetId->second);
 if(actor==m_Players.end()||!actor->second.Is_Guide())return false;
 if(!inviter.Is_Human()||!inviter.iCurrentHp||m_eWorldId!=WORLD_ID::BERN||!m_GuideCatalog.Loaded)return true;
 const auto ownerSessionId=inviter.iSessionId;
 auto ownerSession=Find_Session(ownerSessionId);
 if(!ownerSession||ownerSession->Is_Closing()||!ownerSessionId)return true;
 if(!m_PersonalGuides.empty()){Broadcast_GuideOwnership();return true;}
 if(targetId->second!=m_iGuideReceptionId||distance(inviter,actor->second)>10.f)return true;
 auto& guide=actor->second;
 // Restarting with a grounded owner uses the existing validated flight landing.
 if(guide.eVehicleFlightPhase!=VEHICLE_FLIGHT_PHASE::GROUNDED&&inviter.iVehicleId!=ANCIENT_SEA_VEHICLE_ID){End_VehicleSkill(guide);if(guide.eAction==PLAYER_ACTION_STATE::FALLING)return true;}
 Reset_PlayerForDebugTeleport(guide);guide.isCombatReady=true;
 GUIDE_RUNTIME runtime;runtime.PlayerId=guide.iPlayerId;runtime.AnchorId=inviter.iPlayerId;runtime.OwnerSession=ownerSession;runtime.OwnerNetEntityId=inviter.iNetEntityId;
 runtime.Sequence=(std::max)({guide.iLastMoveSequence,guide.iLastSkillSequence,guide.LastVehicleRidingResult.iRequestSequence});
 // Bind the already placed actor. Starting guidance never creates a player or a party.
 m_PersonalGuides.emplace(ownerSessionId,std::move(runtime));
 Seed_GuideSpaceEntries(ownerSessionId,inviter);
 Broadcast_GuideOwnership();
 const auto category=m_GuideCatalog.Categories.find(WORLD_ID::BERN);
 for(const auto& trigger:m_GuideCatalog.Triggers)if(trigger.Enabled&&category!=m_GuideCatalog.Categories.end()&&trigger.Category==category->second&&(trigger.Type=="GUIDE_STARTED"||trigger.Type=="PARTY_JOINED"))Queue_GuidePrompt(ownerSessionId,trigger);
 return true;
}

void CGameRoom::Handle_GuideControl(SESSION_ID sessionId,const C2S_GUIDE_CONTROL& request)
{
 if(m_eWorldId!=WORLD_ID::BERN||request.iRequestSequence==0)return;
 auto& last=m_GuideControlSequences[sessionId];
 if(last&&static_cast<std::int32_t>(request.iRequestSequence-last)<=0)return;
 last=request.iRequestSequence;
 const auto binding=m_PlayerIdBySessionId.find(sessionId);
 if(binding==m_PlayerIdBySessionId.end())return;
 const auto owner=m_Players.find(binding->second),actor=m_Players.find(m_iGuideReceptionId);
 if(owner==m_Players.end()||actor==m_Players.end()||!owner->second.Is_Human()||request.iGuideNetEntityId!=actor->second.iNetEntityId)return;
 if(request.eAction==GUIDE_CONTROL_ACTION::START)(void)Start_Guide(owner->second,request.iGuideNetEntityId);
 else if(request.eAction==GUIDE_CONTROL_ACTION::STOP&&m_PersonalGuides.contains(sessionId))Remove_Guide(sessionId);
}

void CGameRoom::Broadcast_GuideState(const S2C_GUIDE_STATE& message)
{
 CPacketWriter writer;if(!Write_Message(writer,message))return;
 for(const auto& [id,human]:m_Players)if(human.Is_Human())if(auto session=Find_Session(human.iSessionId);session&&!session->Is_Closing()&&!session->Send_Frame(PACKET_TYPE::S2C_GUIDE_STATE,writer.Get_Buffer()))session->Request_Close();
}

void CGameRoom::Broadcast_GuideOwnership()
{
 const auto actor=m_Players.find(m_iGuideReceptionId);if(actor==m_Players.end())return;
 S2C_GUIDE_STATE state;state.iGuideNetEntityId=actor->second.iNetEntityId;state.iRevision=m_GuideCatalog.Revision;state.iServerTick=m_iServerTick?m_iServerTick:1;
 if(!m_PersonalGuides.empty()){const auto& runtime=m_PersonalGuides.begin()->second;state.iOwnerNetEntityId=runtime.OwnerNetEntityId;state.iAction=runtime.WaitingForOwner||runtime.WaitingForShip?8:1;state.strReason="Guidance is reserved for its current owner";}
 else state.strReason="Ready to start guidance";
 Broadcast_GuideState(state);
}

void CGameRoom::Remove_Guide(SESSION_ID ownerSessionId,bool publish)
{
 const auto found=m_PersonalGuides.find(ownerSessionId);if(found==m_PersonalGuides.end())return;
 auto actor=m_Players.find(found->second.PlayerId);
 if(actor!=m_Players.end()){
  Reset_PlayerForDebugTeleport(actor->second);actor->second.isCombatReady=false;
  actor->second.fVehicleFlightInputX=actor->second.fVehicleFlightInputY=actor->second.fVehicleFlightInputZ=0.f;
  actor->second.fVehicleFlightVelocityX=actor->second.fVehicleFlightVelocityY=actor->second.fVehicleFlightVelocityZ=0.f;
 }
 // Releasing guidance discards every pending occurrence, while the same actor remains placed.
 m_PersonalGuides.erase(found);
 if(publish)Broadcast_GuideOwnership();
}

void CGameRoom::Suspend_PersonalGuide(SESSION_ID ownerSessionId)
{
 if(m_eWorldId!=WORLD_ID::BERN)return;
 const auto found=m_PersonalGuides.find(ownerSessionId);if(found==m_PersonalGuides.end())return;
 auto& state=found->second;
 auto actor=m_Players.find(state.PlayerId);
 if(actor!=m_Players.end()){
  Reset_PlayerForDebugTeleport(actor->second);
  actor->second.isCombatReady=false;
  actor->second.fVehicleFlightInputX=actor->second.fVehicleFlightInputY=actor->second.fVehicleFlightInputZ=0.f;
  actor->second.fVehicleFlightVelocityX=actor->second.fVehicleFlightVelocityY=actor->second.fVehicleFlightVelocityZ=0.f;
 }
 // Called inside a transfer commit: no packets or callbacks to outbound queues.
 state.WaitingForOwner=true;state.ReturningOnFoot=false;state.ComboId.clear();state.PendingComboId.clear();state.Reason.clear();
 state.PromptQueue.clear();state.PromptRemaining=0.f;state.InsideBoxes.clear();state.FarElapsed=0.f;
}

void CGameRoom::Resume_PersonalGuide(SESSION_ID ownerSessionId,WORLD_ID sourceWorld)
{
 if(m_eWorldId!=WORLD_ID::BERN)return;
 const auto found=m_PersonalGuides.find(ownerSessionId);if(found==m_PersonalGuides.end()||!found->second.WaitingForOwner)return;
 auto& state=found->second;const auto owner=state.OwnerSession.lock();
 const auto binding=m_PlayerIdBySessionId.find(ownerSessionId);
 if(!owner||owner->Is_Closing()||owner!=Find_Session(ownerSessionId)||binding==m_PlayerIdBySessionId.end())return;
 const auto actor=m_Players.find(state.PlayerId);if(actor==m_Players.end())return;
 state.AnchorId=binding->second;state.OwnerNetEntityId=m_Players.at(binding->second).iNetEntityId;state.WaitingForOwner=false;state.ReturningOnFoot=true;state.FarElapsed=0.f;
 actor->second.isCombatReady=actor->second.iCurrentHp!=0;
 state.Reason="Owner returned; approaching on the existing navigation path";
 Seed_GuideSpaceEntries(ownerSessionId,m_Players.at(binding->second));
 const char* type=nullptr;const char* world=nullptr;
 switch(sourceWorld){
 case WORLD_ID::VALTAN_ARENA:type="RAID_RETURNED";world="VALTAN_ARENA";break;
 case WORLD_ID::KAKULSAYDON_ARENA:type="RAID_RETURNED";world="KAKULSAYDON_ARENA";break;
 case WORLD_ID::MAHARAKA:type="WORLD_RETURNED";world="MAHARAKA";break;
 case WORLD_ID::COLOSSEUM:type="WORLD_RETURNED";world="COLOSSEUM";break;
 default:break;
 }
 // This runs only after successful entry commit. Actual sending stays in Update_Guides.
 const auto category=m_GuideCatalog.Categories.find(WORLD_ID::BERN);
 if(type)for(const auto& trigger:m_GuideCatalog.Triggers)if(trigger.Enabled&&category!=m_GuideCatalog.Categories.end()&&trigger.Category==category->second&&trigger.Type==type&&trigger.PatternId==world)Queue_GuidePrompt(ownerSessionId,trigger);
}

void CGameRoom::Seed_GuideSpaceEntries(SESSION_ID ownerSessionId,const SERVER_PLAYER& anchor)
{
 const auto guide=m_PersonalGuides.find(ownerSessionId);if(guide==m_PersonalGuides.end())return;
 auto& inside=guide->second.InsideBoxes;inside.clear();
 const auto category=m_GuideCatalog.Categories.find(m_eWorldId);if(category==m_GuideCatalog.Categories.end())return;
 // Starting or arriving while already inside a box is not an outside-to-inside edge.
 for(const auto& trigger:m_GuideCatalog.Triggers)
  if(trigger.Enabled&&trigger.Category==category->second&&trigger.Type=="SPACE_ENTER"&&CServerTriggerSystem::Contains_Placement(trigger.Box,anchor))inside.insert(trigger.Id);
}

void CGameRoom::Prune_GuideSpacePrompts(SESSION_ID ownerSessionId)
{
 const auto guide=m_PersonalGuides.find(ownerSessionId);if(guide==m_PersonalGuides.end())return;
 const auto anchor=m_Players.find(guide->second.AnchorId);const auto category=m_GuideCatalog.Categories.find(m_eWorldId);
 // Retain only sources whose entry really fired; never adopt an unfired nearby box.
 auto& queue=guide->second.PromptQueue;
 for(auto occurrence=queue.begin();occurrence!=queue.end();){
  auto& queued=*occurrence;
  if(!queued.IsSpaceEnter||queued.NextSegment>0){++occurrence;continue;}
  const GUIDE_TRIGGER* best=nullptr;
  std::erase_if(queued.SpaceTriggerIds,[&](const auto& id){
   const auto source=std::find_if(m_GuideCatalog.Triggers.begin(),m_GuideCatalog.Triggers.end(),[&](const auto& row){return row.Id==id;});
   if(anchor==m_Players.end()||source==m_GuideCatalog.Triggers.end()||!source->Enabled||source->Type!="SPACE_ENTER"||source->PromptId!=queued.PromptId||category==m_GuideCatalog.Categories.end()||source->Category!=category->second||!CServerTriggerSystem::Contains_Placement(source->Box,anchor->second))return true;
   if(!best||source->Priority>best->Priority)best=&*source;
   return false;
  });
  if(!best){occurrence=queue.erase(occurrence);continue;}
  queued.TriggerId=best->Id;queued.Priority=best->Priority;++occurrence;
 }
 auto first=queue.begin();if(first!=queue.end()&&first->NextSegment>0)++first;
 std::stable_sort(first,queue.end(),[](const auto& a,const auto& b){return a.Priority>b.Priority;});
}

void CGameRoom::Queue_GuidePrompt(SESSION_ID ownerSessionId,const GUIDE_TRIGGER& trigger)
{
 auto guide=m_PersonalGuides.find(ownerSessionId);const auto* prompt=m_GuideCatalog.Find_Prompt(trigger.PromptId);
 if(guide==m_PersonalGuides.end()||!prompt)return;
 // A same-text occurrence at a new location must not inherit a departed source.
 Prune_GuideSpacePrompts(ownerSessionId);
 auto& queue=guide->second.PromptQueue;
 const auto duplicate=std::find_if(queue.begin(),queue.end(),[&](const auto& queued){return queued.PromptId==trigger.PromptId;});
 if(duplicate!=queue.end()){
  if(duplicate->NextSegment>0)return;
  if(duplicate->IsSpaceEnter&&trigger.Type=="SPACE_ENTER"){
   if(std::find(duplicate->SpaceTriggerIds.begin(),duplicate->SpaceTriggerIds.end(),trigger.Id)==duplicate->SpaceTriggerIds.end())duplicate->SpaceTriggerIds.push_back(trigger.Id);
   Prune_GuideSpacePrompts(ownerSessionId);return;
  }
  if(duplicate->Priority>=trigger.Priority)return;
  queue.erase(duplicate);
 }
 if(queue.size()>=16)return;
 auto place=queue.begin();if(place!=queue.end()&&place->NextSegment>0)++place;
 // Rank the occurrence that actually fired, not an unrelated trigger sharing its text.
 while(place!=queue.end()&&place->Priority>=trigger.Priority)++place;
 queue.insert(place,{trigger.PromptId,trigger.Id,0,trigger.Priority,trigger.Type=="SPACE_ENTER",trigger.Type=="SPACE_ENTER"?std::vector<std::string>{trigger.Id}:std::vector<std::string>{}});
}

void CGameRoom::Guide_ChatCommand(const SERVER_PLAYER& sender,const std::string& line)
{
 if(!sender.Is_Human())return;
 auto guide=m_PersonalGuides.find(sender.iSessionId);if(guide==m_PersonalGuides.end()||guide->second.WaitingForOwner||guide->second.WaitingForShip)return;
 const auto input=trim(line);auto& state=guide->second;
 for(const auto& command:m_GuideCatalog.Commands)
 {
  if(!command.Enabled||std::find(command.Aliases.begin(),command.Aliases.end(),input)==command.Aliases.end())continue;
  const auto commandTick=m_iServerTick?m_iServerTick:1;
  const auto lastCommand=state.CommandTicks.find(command.Id);
  if(!command.Stop&&lastCommand!=state.CommandTicks.end()&&commandTick-lastCommand->second<ticks(command.CooldownMs))return;
  state.CommandTicks[command.Id]=commandTick;
  std::string comboId=command.ComboId;int comboPriority=(std::numeric_limits<int>::min)();
  const auto category=m_GuideCatalog.Categories.find(m_eWorldId);
  for(const auto& trigger:m_GuideCatalog.Triggers)if(trigger.Enabled&&trigger.Type=="HELP_COMMAND"&&trigger.PatternId==command.Id&&category!=m_GuideCatalog.Categories.end()&&trigger.Category==category->second&&!trigger.ComboId.empty()&&trigger.Priority>comboPriority){comboId=trigger.ComboId;comboPriority=trigger.Priority;}
  if(command.Stop){state.ComboId.clear();state.PendingComboId.clear();state.ComboStep=0;state.Reason="Assistance stopped by owner command";}
  else if(state.ComboId!=comboId)
  {
   auto actor=m_Players.find(state.PlayerId);
   if(actor!=m_Players.end()&&actor->second.eAction==PLAYER_ACTION_STATE::SKILL)state.PendingComboId=comboId;
   else {state.ComboId=comboId;state.ComboStep=0;state.ComboElapsed=state.StepElapsed=0;}
  }
  for(const auto& trigger:m_GuideCatalog.Triggers)if(trigger.Enabled&&trigger.Type=="HELP_COMMAND"&&trigger.PatternId==command.Id&&category!=m_GuideCatalog.Categories.end()&&trigger.Category==category->second)Queue_GuidePrompt(sender.iSessionId,trigger);
  return;
 }
}

void CGameRoom::Guide_AnchorArrived(const SERVER_PLAYER& anchor, const bool localMapTravel)
{
 if(!anchor.Is_Human()||isMinigameAnchor(anchor))return;
 const auto found=m_PersonalGuides.find(anchor.iSessionId);
 if(found==m_PersonalGuides.end()||found->second.WaitingForOwner)return;
 auto& state=found->second;
 if(state.AnchorId!=anchor.iPlayerId)return;
 auto actor=m_Players.find(state.PlayerId);if(actor==m_Players.end())return;
 const bool relocate=localMapTravel&&m_eWorldId==WORLD_ID::BERN&&!anchor.bShipDockValid;
 SERVER_NAV_POINT landing;
 if(relocate)
 {
  const float yaw=anchor.fYawDegrees*PI/180.f;
  // Prefer the following offset, then search beside the committed owner. Neither
  // search may cross a navigation seam or admit a blocked destination.
  if(!Find_GuideLanding(actor->second,
      anchor.fPositionX-std::sin(yaw)*m_GuideCatalog.DesiredDistance,anchor.fPositionY,
      anchor.fPositionZ-std::cos(yaw)*m_GuideCatalog.DesiredDistance,landing,&anchor)&&
     !Find_GuideLanding(actor->second,anchor.fPositionX,anchor.fPositionY,anchor.fPositionZ,landing,&anchor))
  {
   state.Reason="Owner arrived; no connected guide landing is available";
   return;
  }
 }
 // Admission precedes mutation so a refused landing preserves the existing actor.
 Reset_PlayerForDebugTeleport(actor->second);
 state.ComboId.clear();state.PendingComboId.clear();state.FarElapsed=0.f;
 // Preserve owner contact: an actual relocation into a new space still has an entry edge.
 state.ReturningOnFoot=true;state.Reason="Following committed owner arrival on foot";
 // Bern squareholes, Set Sail preparation and authored building travel are local
 // arrivals. Actual ship boarding and cross-world returns keep their existing policy.
 if(relocate)
 {
  actor->second.fPositionX=landing.x;actor->second.fPositionY=landing.y;actor->second.fPositionZ=landing.z;
  actor->second.fYawDegrees=anchor.fYawDegrees;actor->second.isCombatReady=actor->second.iCurrentHp!=0u;
  state.ReturningOnFoot=false;state.WaitingForShip=false;
  state.Reason="Following committed owner arrival on connected ground";
 }
}

void CGameRoom::Update_Guides(float seconds)
{
 if(!m_GuideCatalog.Loaded||m_eWorldId!=WORLD_ID::BERN)return;
 m_fGuideOwnershipElapsed+=seconds;
 if(m_fGuideOwnershipElapsed>=m_GuideCatalog.ThinkSeconds&&(m_PersonalGuides.empty()||m_PersonalGuides.begin()->second.WaitingForOwner)){m_fGuideOwnershipElapsed=0.f;Broadcast_GuideOwnership();}
 for(auto it=m_PersonalGuides.begin();it!=m_PersonalGuides.end();)
 {
  const auto ownerSessionId=it->first;auto& state=it->second;
  const auto ownerSession=state.OwnerSession.lock();
  if(!ownerSession||ownerSession->Is_Closing()){++it;Remove_Guide(ownerSessionId);continue;}
  auto guideIt=m_Players.find(state.PlayerId);
  if(guideIt==m_Players.end()){++it;Remove_Guide(ownerSessionId);continue;}
  if(state.WaitingForOwner){++it;continue;}
  const auto binding=m_PlayerIdBySessionId.find(ownerSessionId);
  if(binding==m_PlayerIdBySessionId.end()){++it;continue;}
  state.AnchorId=binding->second;
  auto anchorIt=m_Players.find(state.AnchorId);if(anchorIt==m_Players.end()){++it;continue;}
  const bool anchorSuppressed=isMinigameAnchor(anchorIt->second);
  auto& guide=guideIt->second;const auto& anchor=anchorIt->second;
  auto send=[&](PACKET_TYPE kind,const auto& message){CPacketWriter writer;if(!Write_Message(writer,message))return false;if(!ownerSession->Send_Frame(kind,writer.Get_Buffer())){ownerSession->Request_Close();return false;}return true;};
  float evaluatedThreat=0.f;bool survivalOverride=false;
  auto publishTrace=[&](){S2C_GUIDE_STATE trace;trace.iGuideNetEntityId=guide.iNetEntityId;trace.iOwnerNetEntityId=anchor.iNetEntityId;trace.iRevision=m_GuideCatalog.Revision;trace.iServerTick=m_iServerTick?m_iServerTick:1;trace.iContext=state.ComboId.empty()?0:1;trace.iAction=state.Action;trace.fFollowScore=state.FollowScore;trace.fEvadeScore=state.EvadeScore;trace.fCombatScore=state.CombatScore;trace.fThreat=evaluatedThreat;trace.fAnchorDistance=distance(guide,anchor);trace.fHpRatio=guide.iMaximumHp?std::clamp(static_cast<float>(guide.iCurrentHp)/guide.iMaximumHp,0.f,1.f):0.f;trace.bSurvivalOverride=survivalOverride;trace.strReason=state.Reason;trace.strComboId=state.ComboId;trace.iComboStep=static_cast<std::uint32_t>(state.ComboStep);Broadcast_GuideState(trace);};
  // Ships leave the guide at the pier; the dragon still uses the shared flight path below.
  if(anchor.bShipDockValid){
   if(!state.WaitingForShip){Reset_PlayerForDebugTeleport(guide);guide.isCombatReady=false;state.WaitingForShip=true;state.ComboId.clear();state.PendingComboId.clear();}
   state.Action=8;state.FarElapsed=0.f;state.Reason="Waiting at the pier while the owner sails";publishTrace();++it;continue;
  }
  if(state.WaitingForShip){state.WaitingForShip=false;state.ReturningOnFoot=true;guide.isCombatReady=true;}
  // Prune even while another prompt is playing so a fresh re-entry can queue it again.
  Prune_GuideSpacePrompts(ownerSessionId);
  const bool waitingToSpeak=state.ReturningOnFoot&&distance(guide,anchor)>m_GuideCatalog.MaximumDistance;
  if(!waitingToSpeak)state.PromptRemaining-=seconds;
  // Ownership precedes every new prompt, including a world-return first frame.
  if(!waitingToSpeak&&state.PromptRemaining<=0&&!state.PromptQueue.empty())publishTrace();
  if(!waitingToSpeak&&state.PromptRemaining<=0&&!state.PromptQueue.empty())
  {
   auto& queued=state.PromptQueue.front();const auto* prompt=m_GuideCatalog.Find_Prompt(queued.PromptId);
   if(prompt&&queued.NextSegment<prompt->Segments.size()){
    const auto& segment=prompt->Segments[queued.NextSegment];S2C_GUIDE_PROMPT message;message.iGuideNetEntityId=guide.iNetEntityId;message.iEventSequence=++m_iGuideEventSequence;if(!message.iEventSequence)message.iEventSequence=++m_iGuideEventSequence;message.iRevision=m_GuideCatalog.Revision;message.strPromptId=prompt->Id;message.strText=segment.Text;message.iDurationMs=segment.DurationMs;
    if(send(PACKET_TYPE::S2C_GUIDE_PROMPT,message)){
     // Cancelled/overflowed reservations never spend the source trigger's cooldown.
     if(queued.NextSegment==0){
      if(queued.IsSpaceEnter)for(const auto& id:queued.SpaceTriggerIds)state.TriggerTicks[id]=m_iServerTick;
      else if(!queued.TriggerId.empty())state.TriggerTicks[queued.TriggerId]=m_iServerTick;
     }
     ++queued.NextSegment;state.PromptRemaining=segment.DurationMs/1000.f;
    }
   }
   if(!prompt||queued.NextSegment>=prompt->Segments.size())state.PromptQueue.pop_front();
  }
  state.ThinkElapsed+=seconds;state.HoldElapsed+=seconds;if(!state.ComboId.empty())state.ComboElapsed+=seconds;
  if(state.ThinkElapsed<m_GuideCatalog.ThinkSeconds){++it;continue;}
  const float elapsed=state.ThinkElapsed;state.ThinkElapsed=0;state.FollowScore=state.EvadeScore=state.CombatScore=0;
  if(anchorSuppressed){guide.hasMoveGoal=false;guide.MovePath.clear();state.Action=8;state.ComboId.clear();state.PendingComboId.clear();state.Reason="Waiting outside the human-only minigame";publishTrace();++it;continue;}
  if(!anchor.iCurrentHp||anchor.eAction==PLAYER_ACTION_STATE::DEAD){state.Action=7;guide.hasMoveGoal=false;guide.MovePath.clear();state.ComboId.clear();state.PendingComboId.clear();state.Reason="Waiting for the owner to revive";publishTrace();++it;continue;}
  if(!guide.iCurrentHp)
  {
   state.Action=4;
   SERVER_NAV_POINT center{anchor.fPositionX,anchor.fPositionY,anchor.fPositionZ};float yaw=0;
   if(m_eWorldId==WORLD_ID::VALTAN_ARENA){if(const auto* p=Find_Placement("boss.valtan.center"))center={p->fPositionX,p->fPositionY,p->fPositionZ};}
   else if(m_eWorldId==WORLD_ID::KAKULSAYDON_ARENA)(void)Resolve_KoukuRevivePosition(guide,center,yaw);
   SERVER_PLAYER revived;if(Build_GuidePlayer(guide.iPlayerId,guide.iNetEntityId,center.x,center.y,center.z,revived)){m_CombatObjectRuntime.Cancel_Source(guide.iNetEntityId);guide=std::move(revived);state.ComboId.clear();state.PendingComboId.clear();state.Reason="Guide revived at an admitted arena position";}
   publishTrace();++it;continue;
  }
  const auto category=m_GuideCatalog.Categories.find(m_eWorldId);
  for(const auto& trigger:m_GuideCatalog.Triggers)
  {
   if(!trigger.Enabled||category==m_GuideCatalog.Categories.end()||trigger.Category!=category->second)continue;
   bool fire=false;
   if(trigger.Type=="SPACE_ENTER"){const bool inside=CServerTriggerSystem::Contains_Placement(trigger.Box,anchor);const bool was=state.InsideBoxes.contains(trigger.Id);if(inside)state.InsideBoxes.insert(trigger.Id);else state.InsideBoxes.erase(trigger.Id);fire=inside&&!was;}
   else if(trigger.Type=="BOSS_PATTERN_STARTED")for(const auto& boss:m_WorldEntities)if(boss.iCurrentHp&&boss.strPatternId==trigger.PatternId&&boss.iPatternSequence&&state.PatternSequences[boss.iNetEntityId]!=boss.iPatternSequence){fire=true;break;}
   auto last=state.TriggerTicks.find(trigger.Id);if(fire&&(last==state.TriggerTicks.end()||m_iServerTick-last->second>=ticks(trigger.CooldownMs))){Queue_GuidePrompt(ownerSessionId,trigger);}
  }
  for(const auto& boss:m_WorldEntities)if(!boss.strPatternId.empty())state.PatternSequences[boss.iNetEntityId]=boss.iPatternSequence;
  SERVER_WORLD_ENTITY* enemy=nullptr;float enemyDistance=100000;
  for(auto& candidate:m_WorldEntities)if((candidate.eKind==WORLD_BOOTSTRAP_KIND::BOSS||candidate.eKind==WORLD_BOOTSTRAP_KIND::MONSTER)&&candidate.iCurrentHp&&!candidate.isEstherSummon){const float d=std::hypot(candidate.fPositionX-guide.fPositionX,candidate.fPositionZ-guide.fPositionZ);if(d<enemyDistance){enemy=&candidate;enemyDistance=d;}}
  const bool battle=enemy&&enemyDistance<35.f&&(enemy->eAction!=SERVER_ENTITY_ACTION::IDLE||!state.ComboId.empty());
  const float anchorDistance=distance(guide,anchor);
  if(state.ReturningOnFoot&&anchorDistance<=m_GuideCatalog.MaximumDistance)state.ReturningOnFoot=false;
  state.FarElapsed=!state.ReturningOnFoot&&anchorDistance>m_GuideCatalog.RecoverDistance&&!battle?state.FarElapsed+elapsed:0.f;
  if(state.FarElapsed>=m_GuideCatalog.RecoverDelay){state.Action=4;Guide_AnchorArrived(anchor);publishTrace();++it;continue;}
  if(guide.bPatternBound||guide.TriggerMove.isActive||guide.eAction==PLAYER_ACTION_STATE::GRABBED||guide.eAction==PLAYER_ACTION_STATE::FALLING||guide.fKnockbackRemainingSeconds>0){state.Action=5;state.Reason="Contact reaction owns the guide movement";publishTrace();++it;continue;}
  if(anchor.iVehicleId!=guide.iVehicleId&&guide.eAction==PLAYER_ACTION_STATE::NONE){C2S_SET_VEHICLE_RIDING command;command.eWorldId=m_eWorldId;command.iRequestSequence=++state.Sequence;command.iVehicleId=anchor.iVehicleId;(void)Apply_SetVehicleRiding(guide,command);}
  if(guide.iVehicleId==ANCIENT_SEA_VEHICLE_ID)
  {
   if(((anchor.eVehicleFlightPhase==VEHICLE_FLIGHT_PHASE::FLYING||anchor.eVehicleFlightPhase==VEHICLE_FLIGHT_PHASE::TAKEOFF)&&guide.eVehicleFlightPhase==VEHICLE_FLIGHT_PHASE::GROUNDED)||((anchor.eVehicleFlightPhase==VEHICLE_FLIGHT_PHASE::GROUNDED||anchor.eVehicleFlightPhase==VEHICLE_FLIGHT_PHASE::LANDING)&&guide.eVehicleFlightPhase==VEHICLE_FLIGHT_PHASE::FLYING)){const auto* vehicle=m_VehicleCatalog.Find_Vehicle(guide.iVehicleId);if(vehicle){for(const auto& skill:vehicle->Skills)if(skill.eSlot==VEHICLE_SKILL_SLOT::E){C2S_USE_SKILL command;command.iClientSequence=++state.Sequence;command.iSkillId=skill.iSkillId;command.fAimX=anchor.fPositionX;command.fAimZ=anchor.fPositionZ;(void)Try_StartVehicleSkill(guide,command);break;}}}
   if(guide.eVehicleFlightPhase==VEHICLE_FLIGHT_PHASE::FLYING){const float yaw=anchor.fYawDegrees*PI/180.f;float dx=anchor.fPositionX-std::sin(yaw)*m_GuideCatalog.DesiredDistance-guide.fPositionX,dz=anchor.fPositionZ-std::cos(yaw)*m_GuideCatalog.DesiredDistance-guide.fPositionZ;float length=std::hypot(dx,dz);C2S_MOVE move;move.iClientSequence=++state.Sequence;move.eIntent=PLAYER_MOVE_INTENT::VEHICLE_FLIGHT;move.fGoalX=length>.5f?dx/(std::max)(1.f,length):0;move.fGoalZ=length>.5f?dz/(std::max)(1.f,length):0;move.fVerticalInput=std::clamp(anchor.fPositionY-guide.fPositionY,-1.f,1.f);Execute_PlayerMove(guide,move);state.Action=6;state.Reason="Following replicated dragon flight";publishTrace();++it;continue;}
  }
  if(guide.eAction==PLAYER_ACTION_STATE::SKILL)if(const auto* running=m_GameplayCatalog.Find_Skill(guide.iCurrentSkillId);running&&running->eSkillKind==PLAYER_SKILL_KIND::HOLD&&guide.fActionElapsedSeconds>=1.f){C2S_RELEASE_SKILL release;release.iClientSequence=++state.Sequence;release.iSkillId=guide.iCurrentSkillId;m_PlayerSkillSystem.Release(guide,release,m_GameplayCatalog);}
  if(!state.PendingComboId.empty()&&guide.eAction==PLAYER_ACTION_STATE::NONE){state.ComboId=std::move(state.PendingComboId);state.PendingComboId.clear();state.ComboStep=0;state.ComboElapsed=state.StepElapsed=0;}
  auto combo=m_GuideCatalog.Find_Combo(state.ComboId);if(combo&&state.ComboElapsed*1000>=combo->TimeoutMs){state.ComboId.clear();combo=nullptr;state.Reason="Combo total deadline reached";}
  // Predict actual published combat-object shapes at bounded future samples.
  auto danger=[&](float x,float z){float risk=Predict_GuideContactRisk(guide,x,z);for(const auto& object:m_CombatObjectRuntime.Get_LiveObjects()){if(object.eSourceKind==SERVER_COMBAT_OBJECT_SOURCE_KIND::PLAYER)continue;const auto& pose=object.LiveState.CurrentPose;if(std::abs(pose.fPositionY-guide.fPositionY)>3.f)continue;for(const auto& hit:object.Hits){if(hit.iEndMs&&object.fElapsedMilliseconds>hit.iEndMs)continue;if(hit.iAtMs>object.fElapsedMilliseconds+m_GuideCatalog.HorizonSeconds*1000)continue;for(int sample=0;sample<3;++sample){const float t=m_GuideCatalog.HorizonSeconds*sample*.5f;float ox=pose.fPositionX+pose.fDirectionX*object.fSpeedMps*t,oz=pose.fPositionZ+pose.fDirectionZ*object.fSpeedMps*t;if(CServerCombatGeometry::Overlaps_Pose(hit.Shape,ox,oz,pose.fDirectionX,pose.fDirectionZ,{x,z,.5f})){risk+=(hit.bInstantDeath?5.f:1.f);break;}}}}return risk;};
  auto pathSafe=[&](const SERVER_NAV_POINT& goal,float currentRisk){
   std::vector<SERVER_NAV_POINT> path;
   if(m_ServerNavigation.Is_Loaded()){
    if(!m_ServerNavigation.Find_Path(guide.fPositionX,guide.fPositionZ,goal.x,goal.z,path,guide.fPositionY))return false;
    m_ServerNavigation.Smooth_Path(guide.fPositionX,guide.fPositionZ,goal.x,goal.z,path,guide.fPositionY);
   }else path.push_back(goal);
   SERVER_NAV_POINT from{guide.fPositionX,guide.fPositionY,guide.fPositionZ};unsigned samples=0;
   for(const auto& to:path){const float length=std::hypot(to.x-from.x,to.z-from.z);const unsigned count=(std::max)(1u,static_cast<unsigned>(std::ceil(length/.5f)));if(samples+count>128)return false;for(unsigned i=1;i<=count;++i){float t=float(i)/count;if(danger(from.x+(to.x-from.x)*t,from.z+(to.z-from.z)*t)>currentRisk+.001f)return false;}samples+=count;from=to;}return true;
  };
  const float risk=danger(guide.fPositionX,guide.fPositionZ);const auto& weight=combo?m_GuideCatalog.AssistWeights:m_GuideCatalog.FollowWeights;
  const bool needsFollow=(state.ReturningOnFoot&&anchorDistance>m_GuideCatalog.MaximumDistance)||anchorDistance>m_GuideCatalog.ResumeDistance||(guide.hasMoveGoal&&anchorDistance>m_GuideCatalog.MaximumDistance)||anchorDistance<m_GuideCatalog.MinimumDistance;
  const float distanceError=anchorDistance<m_GuideCatalog.MinimumDistance?m_GuideCatalog.MinimumDistance-anchorDistance:anchorDistance-m_GuideCatalog.MaximumDistance;
  const float followScore=(needsFollow?weight.Follow:0.f)*std::clamp(distanceError/(std::max)(1.f,m_GuideCatalog.ResumeDistance),0.f,1.f);
  const float evadeScore=weight.Avoid*std::clamp(risk,0.f,1.f);const float combatScore=combo&&enemy?weight.Attack:0;
  const bool lethal=risk>=5||(risk>0&&guide.iCurrentHp<guide.iMaximumHp*m_GuideCatalog.LethalHpFraction);evaluatedThreat=risk;survivalOverride=lethal;
  std::uint8_t action=lethal||evadeScore>(std::max)(followScore,combatScore)?2:combatScore>followScore?3:1;
  const float scores[]={0,followScore,evadeScore,combatScore};if(!lethal&&action!=state.Action&&state.Action>=1&&state.Action<=3&&state.HoldElapsed<m_GuideCatalog.MinimumHoldSeconds&&scores[action]<scores[state.Action]+m_GuideCatalog.SwitchMargin)action=state.Action;
  if(action!=state.Action){state.Action=action;state.HoldElapsed=0;}
  if(action==2){float bestRisk=risk;SERVER_NAV_POINT best{};bool found=false;for(unsigned i=0;i<16;++i){const float angle=i*PI/8.f;SERVER_NAV_POINT p;if(!Find_GuideLanding(guide,guide.fPositionX+std::sin(angle)*3.f,guide.fPositionY,guide.fPositionZ+std::cos(angle)*3.f,p))continue;const float candidateRisk=danger(p.x,p.z);if(candidateRisk<bestRisk&&pathSafe(p,risk)){bestRisk=candidateRisk;best=p;found=true;}}if(found){C2S_MOVE move;move.iClientSequence=++state.Sequence;move.fGoalX=best.x;move.fGoalZ=best.z;Execute_PlayerMove(guide,move);state.Reason=lethal?"Lethal contact predicted; survival override":"Evade has the highest weighted score";}else state.Reason="Danger detected; no lower-risk navigation candidate";}
  else if(action==3&&combo&&enemy)
  {
   if(state.ComboStep>=combo->Skills.size()){if(combo->Repeat){state.ComboStep=0;state.StepElapsed=0;}else{state.ComboId.clear();state.Reason="Combo completed";}}
   else if(guide.eAction==PLAYER_ACTION_STATE::NONE){const auto* skill=m_GameplayCatalog.Find_Skill(combo->Skills[state.ComboStep]);const float range=skill?(std::max)(2.f,skill->fMaximumRange):2.f;if(enemyDistance>range){C2S_MOVE move;move.iClientSequence=++state.Sequence;const float stop=(std::max)(1.f,range*.75f);move.fGoalX=enemy->fPositionX+(guide.fPositionX-enemy->fPositionX)*stop/enemyDistance;move.fGoalZ=enemy->fPositionZ+(guide.fPositionZ-enemy->fPositionZ)*stop/enemyDistance;Execute_PlayerMove(guide,move);state.Reason="Approaching the next combo skill range";}
    else {C2S_USE_SKILL command;command.iClientSequence=++state.Sequence;command.iSkillId=combo->Skills[state.ComboStep];command.eTargetIntent=skill?skill->eTargetIntent:SKILL_TARGET_INTENT_KIND::AIM_POINT;command.fAimX=enemy->fPositionX;command.fAimZ=enemy->fPositionZ;if(Execute_PlayerSkill(guide,command)){++state.ComboStep;state.StepElapsed=0;state.Reason="Skill admitted by the shared player executor";}else {state.StepElapsed+=elapsed;state.Reason="Skill waiting: cooldown, resource or current status";if(state.StepElapsed*1000>=combo->StepWaitMs){state.ComboId.clear();state.Reason="Unavailable skill exceeded its wait deadline";}}}}
   else state.Reason="Current skill is still executing";
  }
  else if(needsFollow){const float yaw=anchor.fYawDegrees*PI/180.f;C2S_MOVE move;move.iClientSequence=++state.Sequence;move.fGoalX=anchor.fPositionX-std::sin(yaw)*m_GuideCatalog.DesiredDistance;move.fGoalZ=anchor.fPositionZ-std::cos(yaw)*m_GuideCatalog.DesiredDistance;Execute_PlayerMove(guide,move);state.Reason="Maintaining the configured anchor distance";}
  else {if(guide.eAction==PLAYER_ACTION_STATE::NONE){guide.hasMoveGoal=false;guide.MovePath.clear();}state.Reason="Within the configured following band";}
  state.FollowScore=followScore;state.EvadeScore=evadeScore;state.CombatScore=combatScore;publishTrace();
  ++it;
 }
}
```

### G11. Server/Private/GameRoom_PlayerSimulation.cpp 전체 코드

```cpp
#include "GameRoom.h"

#include "ClientSession.h"
#include "ColosseumCombatPolicy.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"
#include "Gameplay/MaharakaWaterpangContract.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <set>
#include <string_view>
#include <utility>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

namespace
{
	constexpr float KOUKU_FALL_DEPTH_M = 5.f;
	constexpr float WATERPANG_FALL_DEPTH_M = 2.f;
	// The casino chairs sit above the generic five-metre fall plane. A body
	// leaving its support by more than the forced-move step limit is out.
	constexpr float KOUKU_CASINO_FALL_DEPTH_M = 1.f;

	enum class FORCED_SURFACE_RESULT
	{
		SUPPORTED,
		BLOCKED,
		FALL
	};

	/* Forced motion follows physical support, not the walking graph. A
	   non-walkable surface can support a pushed body, while a lower deck must
	   never become an instantaneous landing. Collision has already resolved
	   this straight segment; no nearest-cell projection is permitted here. */
	FORCED_SURFACE_RESULT Trace_ForcedSurface(
		const LostArk::Server::CServerNavigation& navigation,
		const LostArk::Server::SERVER_NAV_POINT& from,
		const float toX, const float toZ,
		LostArk::Server::SERVER_NAV_POINT& outPoint)
	{
		outPoint = from;
		const float distance = std::hypot(toX - from.x, toZ - from.z);
		const float sampleStep = std::clamp(navigation.Get_CellSize() * 0.5f, 0.01f, 0.25f);
		if (!std::isfinite(distance) || !std::isfinite(from.y) ||
			!std::isfinite(sampleStep) || distance / sampleStep > 4096.f)
		{
			return FORCED_SURFACE_RESULT::BLOCKED;
		}
		/* Keep forced support bounded even if an older navigation policy uses
		   zero to permit arbitrary walking height changes. */
		const float authoredStep = navigation.Get_MaximumTraversalStepHeight();
		const float maximumStep = authoredStep > 0.f ? (std::min)(authoredStep, 1.f) : 1.f;
		const auto count = static_cast<std::uint32_t>((std::max)(1.f, std::ceil(distance / sampleStep)));
		for (std::uint32_t sample = 0u; sample <= count; ++sample)
		{
			const float ratio = static_cast<float>(sample) / static_cast<float>(count);
			const float x = from.x + (toX - from.x) * ratio;
			const float z = from.z + (toZ - from.z) * ratio;
			LostArk::Server::SERVER_NAV_POINT ground{};
			if (!navigation.Sample_SurfacePosition(x, z, ground) ||
				!std::isfinite(ground.y) || ground.y < outPoint.y - maximumStep)
			{
				outPoint.x = x;
				outPoint.z = z;
				return FORCED_SURFACE_RESULT::FALL;
			}
			if (ground.y > outPoint.y + maximumStep)
				return FORCED_SURFACE_RESULT::BLOCKED;
			outPoint = ground;
		}
		return FORCED_SURFACE_RESULT::SUPPORTED;
	}
}

float LostArk::Server::CGameRoom::Resolve_StanceMoveSpeedScale(
	const SERVER_PLAYER& player) const
{
	const PLAYER_RUNTIME_PROFILE* profile =
		m_GameplayCatalog.Find_Player(player.eCharacterClass);
	if (nullptr == profile ||
		!CPlayerSkillSystem::Is_HoldingGaugedStance(player, *profile))
	{
		return 1.f;
	}
	return profile->fDefenseStanceMoveSpeedScale;
}

void LostArk::Server::CGameRoom::Refresh_PlayerBlockingBodies()
{
	std::vector<SERVER_BLOCKING_BODY> bodies;
	bodies.reserve(m_WorldEntities.size());
	for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		if (entity.isEstherSummon ||
			LostArk::Shared::INVALID_NET_ENTITY_ID != entity.iOwnerBossNetEntityId ||
			SERVER_ENTITY_ACTION::DEAD == entity.eAction ||
			(WORLD_BOOTSTRAP_KIND::NPC != entity.eKind &&
			 0u == entity.iCurrentHp))
		{
			continue;
		}
		/* Same body the skill hit test uses: monsters carry their profile
		radius, the boss reads its profile, and town NPCs use the shared upright
		player-sized body until the catalog owns a dedicated gameplay radius. */
		float radius = entity.fCollisionRadius;
		float centerY = entity.fPositionY + radius;
		float halfHeight = radius;
		if (WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind)
		{
			if (const BOSS_RUNTIME_PROFILE* bossProfile =
				m_GameplayCatalog.Find_Boss(entity.strArchetypeId))
			{
				radius = bossProfile->fCollisionRadius;
			}
		}
		else if (WORLD_BOOTSTRAP_KIND::NPC == entity.eKind)
		{
			using namespace LostArk::Shared::WorldCollision;
			radius = PLAYER_HALF_EXTENT_X;
			centerY = entity.fPositionY + PLAYER_CENTER_OFFSET_Y;
			halfHeight = PLAYER_HALF_EXTENT_Y;
		}
		else if (WORLD_BOOTSTRAP_KIND::MONSTER != entity.eKind)
		{
			continue;
		}
		if (radius <= 0.f)
			continue;
		if (WORLD_BOOTSTRAP_KIND::NPC != entity.eKind)
		{
			centerY = entity.fPositionY + radius;
			halfHeight = radius;
		}
		bodies.push_back(SERVER_BLOCKING_BODY{
			entity.fPositionX, entity.fPositionZ, radius,
			centerY, halfHeight, entity.iNetEntityId });
	}
	m_ServerCollisionSystem.Set_BlockingBodies(std::move(bodies));
}

void LostArk::Server::CGameRoom::Begin_PlayerFall(
	SERVER_PLAYER& player,
	const float fixedDeltaSeconds,
	const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	const float fallDepth = m_eWorldId == WORLD_ID::MAHARAKA ? WATERPANG_FALL_DEPTH_M :
		(Resolve_KoukuFallCenter(player) ? KOUKU_CASINO_FALL_DEPTH_M : KOUKU_FALL_DEPTH_M);
	player.fFallDeathPlaneY = (player.bKnockbackBallistic ? player.fKnockbackSupportY : player.fPositionY) - fallDepth;
	player.eAction = PLAYER_ACTION_STATE::FALLING;
	player.bKoukuFallDeath = m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA && !player.iMarioStage;
	player.bWaterpangFall = false;
	player.bWaterpangLaunch = false;
	player.iActionStartTick = 0u == updateTick ? 1u : updateTick;
	player.iFallDeathTick = Add_ServerTicksSkippingReservedZero(
		player.iActionStartTick, FALL_DEATH_TICKS);
	player.fFallVelocityY = 0.f;
	/* Everything the fall interrupts is cleared here instead of inside each
	system, so no half-finished action can resume when the body lands dead. */
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.fActionElapsedSeconds = 0.f;
	player.hasAppliedSkillDamage = false;
	player.iAppliedHitMask = 0;
	player.iSpawnedProjectileMask = 0;
	player.Projectiles.clear();
	m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.PendingCommand.Clear();
	player.hasReleasedHold = false;
	player.TriggerMove = {};
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.fKnockbackDirectionX = 0.f;
	player.fKnockbackDirectionZ = 0.f;
	player.fKnockbackSpeed = 0.f;
	player.fKnockbackRemainingSeconds = 0.f;
	/* The ejection/ballistic phase ends at this boundary.  Keep the ordinary
	   FALLING integrator authoritative after the edge crossing; leaving either
	   typed flight flag set would make Update_PlayerFall return early forever
	   and the player could never reach the dead-zone deadline. */
	player.bArenaEjectionActive = false;
	player.iEjectionOwnerNetEntityId = INVALID_NET_ENTITY_ID;
	player.bKnockbackCanLeaveArena = false;
	player.bKnockbackBallistic = false;
	player.fKnockbackVelocityY = 0.f;
	player.fKnockbackLaunchY = 0.f;
	player.iKnockdownEndTick = 0u;
	player.Clear_Attachment();
	/* Every boss and monster gate already refuses a player that is not combat
	ready, so this one flag removes the falling body from acquisition and from
	area damage without editing four separate target filters. */
	player.isCombatReady = false;
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
	/* The edge that opens the hole is also the first tick of the descent, so
	the body integrates here instead of hanging one tick at the old height and
	broadcasting a FALLING snapshot that has not moved. The deadline was just
	set a full FALL_DEATH_TICKS away, so it cannot be due on this tick. */
	player.fFallVelocityY -=
		FALL_GRAVITY_METERS_PER_SECOND_SQUARED * fixedDeltaSeconds;
	player.fPositionY += player.fFallVelocityY * fixedDeltaSeconds;
}

bool LostArk::Server::CGameRoom::Capture_PlayerAttachment(
	const LostArk::Shared::NET_ENTITY_ID playerEntityId,
	const LostArk::Shared::NET_ENTITY_ID ownerEntityId,
	const LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
	const std::uint32_t serverTick, const std::uint32_t holdEndTick, const std::uint32_t sourcePatternSequence)
{
	using namespace LostArk::Shared;
	if (INVALID_NET_ENTITY_ID == playerEntityId ||
		INVALID_NET_ENTITY_ID == ownerEntityId ||
		playerEntityId == ownerEntityId || 0u == serverTick ||
        (holdEndTick && (Has_ReachedServerTick(serverTick, holdEndTick) || holdEndTick - serverTick > 18001u)) ||
		PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND != slot)
	{
		return false;
	}

	const auto playerId = m_PlayerIdByEntityId.find(playerEntityId);
	if (m_PlayerIdByEntityId.end() == playerId)
		return false;
	const auto playerIter = m_Players.find(playerId->second);
	if (m_Players.end() == playerIter)
		return false;
	const auto liveOwner = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[ownerEntityId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.iNetEntityId == ownerEntityId;
		});
	auto* owner = sourcePatternSequence && m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA ?
		Find_KoukuOccurrenceOwner(ownerEntityId, sourcePatternSequence) :
		(liveOwner == m_WorldEntities.end() ? nullptr : &*liveOwner);
	if (nullptr == owner ||
		WORLD_BOOTSTRAP_KIND::BOSS != owner->eKind ||
		SERVER_ENTITY_ACTION::DEAD == owner->eAction ||
		0u == owner->iCurrentHp || 0u == owner->iPatternSequence ||
		!std::isfinite(owner->fPositionX) ||
		!std::isfinite(owner->fPositionY) ||
		!std::isfinite(owner->fPositionZ) ||
		!std::isfinite(owner->fYawDegrees))
	{
		return false;
	}

	SERVER_PLAYER& player = playerIter->second;
	if (PLAYER_ACTION_STATE::GRABBED == player.eAction)
	{
		return player.iAttachmentOwnerNetEntityId == ownerEntityId &&
			player.eAttachmentSlot == slot &&
			player.iAttachmentPatternSequence == owner->iPatternSequence &&
            player.iAttachmentEndTick == holdEndTick;
	}
	if (0u == player.iCurrentHp || !player.isCombatReady || player.Has_TimeStop(serverTick) ||
        PLAYER_ACTION_STATE::FEAR == player.eAction ||
		PLAYER_ACTION_STATE::DEAD == player.eAction ||
		PLAYER_ACTION_STATE::FALLING == player.eAction ||
		!std::isfinite(player.fPositionX) ||
		!std::isfinite(player.fPositionY) ||
		!std::isfinite(player.fPositionZ) ||
		!std::isfinite(player.fYawDegrees))
	{
		return false;
	}

	const float yawRadians = owner->fYawDegrees * DEGREES_TO_RADIANS;
	const float sine = std::sin(yawRadians);
	const float cosine = std::cos(yawRadians);
	const float deltaX = player.fPositionX - owner->fPositionX;
	const float deltaZ = player.fPositionZ - owner->fPositionZ;
	const float localX = deltaX * cosine - deltaZ * sine;
	const float localY = player.fPositionY - owner->fPositionY;
	const float localZ = deltaX * sine + deltaZ * cosine;
	const float localYaw = Wrap_Degrees(
		player.fYawDegrees - owner->fYawDegrees);
	if (!std::isfinite(localX) || !std::isfinite(localY) ||
		!std::isfinite(localZ) || !std::isfinite(localYaw))
	{
		return false;
	}

	/* Capture interrupts one complete action transaction. Projectiles and
	combat objects cannot remain owned by a body whose input is now frozen. */
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.fActionElapsedSeconds = 0.f;
	player.hasAppliedSkillDamage = false;
	player.iAppliedHitMask = 0u;
	player.iSpawnedProjectileMask = 0u;
	player.Projectiles.clear();
	m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.PendingCommand.Clear();
	player.hasReleasedHold = false;
	player.TriggerMove = {};
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.fKnockbackDirectionX = 0.f;
	player.fKnockbackDirectionZ = 0.f;
	player.fKnockbackSpeed = 0.f;
	player.fKnockbackRemainingSeconds = 0.f;
	player.iKnockdownEndTick = 0u;
	player.iHitReactionGraceEndTick = 0u;
	player.fFallVelocityY = 0.f;
	player.iFallDeathTick = 0u;
	player.Clear_Attachment();
	player.iAttachmentOwnerNetEntityId = ownerEntityId;
	player.eAttachmentSlot = slot;
	player.iAttachmentPatternSequence = owner->iPatternSequence;
    player.iAttachmentEndTick = holdEndTick;
	player.fAttachmentLocalOffsetX = localX;
	player.fAttachmentLocalOffsetY = localY;
	player.fAttachmentLocalOffsetZ = localZ;
	player.fAttachmentYawOffsetDegrees = localYaw;
	player.eAction = PLAYER_ACTION_STATE::GRABBED;
	player.iActionStartTick = serverTick;
	player.isCombatReady = false;
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
	return true;
}

bool LostArk::Server::CGameRoom::Release_PlayerAttachment(
	SERVER_PLAYER& player,
	const LostArk::Shared::NET_ENTITY_ID ownerEntityId,
	const float pushRangeM,
	const std::uint32_t pushMs,
	const bool knockdown,
	const std::uint32_t downMs,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::GRABBED != player.eAction ||
		player.iAttachmentOwnerNetEntityId != ownerEntityId ||
		0u == serverTick || !std::isfinite(pushRangeM))
	{
		return false;
	}

	// Every hook exit, including cancellation and an owner disappearing, must
	// land before input is unlocked. A dodge must never start from an air pose.
	if (player.iCurrentHp && !player.iMarioStage && Resolve_CurrentKoukuGate() == 3u &&
		player.eAttachmentSlot == PLAYER_ATTACHMENT_SLOT::WORLD_HOOK_TIP)
	{
		SERVER_NAV_POINT landing{};
		constexpr float maximumDistance = 2.f * WorldCollision::PLAYER_HALF_EXTENT_Y;
		const auto clearFloor = [&] {
			return std::isfinite(landing.x) && std::isfinite(landing.y) && std::isfinite(landing.z) &&
				m_ServerNavigation.Is_PointWalkableExact(landing.x, landing.z, landing.y) &&
				m_ServerCollisionSystem.Is_PlayerPositionClear(landing.x, landing.y, landing.z, player.iNetEntityId);
		};
		const auto nearbyFloor = [&] {
			return clearFloor() && std::abs(landing.y - player.fPositionY) <= maximumDistance &&
				std::hypot(landing.x - player.fPositionX, landing.z - player.fPositionZ) <= maximumDistance &&
				m_ServerNavigation.Is_InSameNavigationGrid(player.fPositionX, player.fPositionZ, landing.x, landing.z);
		};
		bool grounded = m_ServerNavigation.Is_Loaded() &&
			std::isfinite(player.fPositionX) && std::isfinite(player.fPositionY) && std::isfinite(player.fPositionZ) &&
			((m_ServerNavigation.Project_PointOnSameLevel(player.fPositionX, player.fPositionZ, landing, player.fPositionY) && nearbyFloor()) ||
			 (m_ServerNavigation.Project_Point(player.fPositionX, player.fPositionZ, landing, player.fPositionY) && nearbyFloor()));
		if (!grounded)
		{
			SERVER_NAV_POINT start{}; float yaw = 0.f;
			grounded = m_ServerNavigation.Is_Loaded() && Resolve_KoukuRevivePosition(player, start, yaw) &&
				m_ServerNavigation.Project_Point(start.x, start.z, landing, start.y) && clearFloor() &&
				std::abs(landing.y - start.y) <= maximumDistance &&
				std::hypot(landing.x - start.x, landing.z - start.z) <= maximumDistance &&
				m_ServerNavigation.Is_InSameNavigationGrid(start.x, start.z, landing.x, landing.z);
		}
		if (!grounded)
		{
			player.isCombatReady = false;
			m_strStatus = "Kouku hook release is waiting for a safe arena floor";
			return false;
		}
		player.fPositionX = landing.x;
		player.fPositionY = landing.y;
		player.fPositionZ = landing.z;
		player.fFallVelocityY = 0.f;
		player.iFallDeathTick = 0u;
	}

	float sourceX = player.fPositionX;
	float sourceZ = player.fPositionZ;
	const auto owner = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[ownerEntityId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.iNetEntityId == ownerEntityId;
		});
	if (m_WorldEntities.end() != owner &&
		std::isfinite(owner->fPositionX) &&
		std::isfinite(owner->fPositionZ))
	{
		sourceX = owner->fPositionX;
		sourceZ = owner->fPositionZ;
	}

	player.Clear_Attachment();
	player.eAction = 0u == player.iCurrentHp ?
		PLAYER_ACTION_STATE::DEAD : PLAYER_ACTION_STATE::NONE;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.iActionStartTick = 0u;
	player.fActionElapsedSeconds = 0.f;
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.PendingCommand.Clear();
	player.hasReleasedHold = false;
	player.TriggerMove = {};
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.fKnockbackRemainingSeconds = 0.f;
	player.fKnockbackSpeed = 0.f;
	player.iKnockdownEndTick = 0u;
	player.iHitReactionGraceEndTick = 0u;
	player.isCombatReady = 0u != player.iCurrentHp;
	if (0u != player.iCurrentHp)
	{
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			player, sourceX, sourceZ, pushRangeM, pushMs,
			knockdown, downMs, serverTick);
	}
	return true;
}

std::size_t LostArk::Server::CGameRoom::Release_PlayerAttachments(
	const LostArk::Shared::NET_ENTITY_ID ownerEntityId,
	const float pushRangeM,
	const std::uint32_t pushMs,
	const bool knockdown,
	const std::uint32_t downMs,
	const std::uint32_t serverTick)
{
	std::size_t released = 0u;
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (Release_PlayerAttachment(
			player, ownerEntityId, pushRangeM, pushMs,
			knockdown, downMs, serverTick))
		{
			++released;
		}
	}
	return released;
}

bool LostArk::Server::CGameRoom::Update_PlayerAttachment(
	SERVER_PLAYER& player,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::GRABBED != player.eAction)
	{
		if (INVALID_NET_ENTITY_ID != player.iAttachmentOwnerNetEntityId ||
			PLAYER_ATTACHMENT_SLOT::NONE != player.eAttachmentSlot ||
			(0u != player.iAttachmentPatternSequence || 0u != player.iAttachmentEndTick))
		{
			player.Clear_Attachment();
		}
		return false;
	}

	const NET_ENTITY_ID ownerEntityId =
		player.iAttachmentOwnerNetEntityId;
    if (player.iCurrentHp == 0u || (player.iAttachmentEndTick && Has_ReachedServerTick(serverTick, player.iAttachmentEndTick)))
    {
        (void)Release_PlayerAttachment(player, ownerEntityId, 0.f, 0u, false, 0u, serverTick ? serverTick : 1u);
        return player.eAction == PLAYER_ACTION_STATE::GRABBED;
    }
	const auto body = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[ownerEntityId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.iNetEntityId == ownerEntityId;
		});
	const auto* owner = m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA ?
		Find_KoukuOccurrenceOwner(ownerEntityId, player.iAttachmentPatternSequence) :
		(body == m_WorldEntities.end() ? nullptr : &*body);
	const bool liveOwner = nullptr != owner &&
		WORLD_BOOTSTRAP_KIND::BOSS == owner->eKind &&
		SERVER_ENTITY_ACTION::DEAD != owner->eAction &&
		0u != owner->iCurrentHp && 0u != owner->iPatternSequence &&
		player.iAttachmentPatternSequence == owner->iPatternSequence;
	/* A World Object carries this player, not a boss bone. The KoukuSaydon logic
	runtime writes the transform from the authored region every tick it runs, so
	the pose it wrote stands; all that is owned here is the deadline that region
	gave us and the ordinary release once it passes. */
	if (PLAYER_ATTACHMENT_SLOT::WORLD_HOOK_TIP == player.eAttachmentSlot)
	{
		if (!liveOwner || 0u == player.iAttachmentReleaseTick ||
			CKoukuSaydonLogicRuntime::Has_ReachedTick(
				serverTick, player.iAttachmentReleaseTick))
		{
			(void)Release_PlayerAttachment(
				player, ownerEntityId, 0.f, 0u, false, 0u,
				0u == serverTick ? 1u : serverTick);
			return player.eAction == PLAYER_ACTION_STATE::GRABBED;
		}
		player.isCombatReady = false;
		return true;
	}
	const bool validOwner = liveOwner &&
		PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND == player.eAttachmentSlot &&
		std::isfinite(owner->fPositionX) &&
		std::isfinite(owner->fPositionY) &&
		std::isfinite(owner->fPositionZ) &&
		std::isfinite(owner->fYawDegrees) &&
		std::isfinite(player.fAttachmentLocalOffsetX) &&
		std::isfinite(player.fAttachmentLocalOffsetY) &&
		std::isfinite(player.fAttachmentLocalOffsetZ) &&
		std::isfinite(player.fAttachmentYawOffsetDegrees);
	if (!validOwner)
	{
		(void)Release_PlayerAttachment(
			player, ownerEntityId, 0.f, 0u, false, 0u,
			0u == serverTick ? 1u : serverTick);
		return false;
	}

	const float yawRadians = owner->fYawDegrees * DEGREES_TO_RADIANS;
	const float sine = std::sin(yawRadians);
	const float cosine = std::cos(yawRadians);
	const float nextX = owner->fPositionX +
		player.fAttachmentLocalOffsetX * cosine +
		player.fAttachmentLocalOffsetZ * sine;
	const float nextY = owner->fPositionY +
		player.fAttachmentLocalOffsetY;
	const float nextZ = owner->fPositionZ -
		player.fAttachmentLocalOffsetX * sine +
		player.fAttachmentLocalOffsetZ * cosine;
	const float nextYaw = Wrap_Degrees(
		owner->fYawDegrees + player.fAttachmentYawOffsetDegrees);
	if (!std::isfinite(nextX) || !std::isfinite(nextY) ||
		!std::isfinite(nextZ) || !std::isfinite(nextYaw))
	{
		(void)Release_PlayerAttachment(
			player, ownerEntityId, 0.f, 0u, false, 0u,
			0u == serverTick ? 1u : serverTick);
		return false;
	}

	player.fPositionX = nextX;
	player.fPositionY = nextY;
	player.fPositionZ = nextZ;
	player.fYawDegrees = nextYaw;
	player.isCombatReady = false;
	return true;
}

/* Owns the whole falling life cycle of one player inside one tick: it starts
a fall when the authored ground under the player is gone, advances a running
fall, and turns it into the ordinary death the revive path already
understands. Returning true is what keeps trigger motion, skills and movement
from running at all this tick. */
bool LostArk::Server::CGameRoom::Restore_PatternBoundPlayer(
	SERVER_PLAYER& player)
{
	float restoreX = player.fPatternBindRestoreX;
	float restoreY = player.fPatternBindRestoreY;
	float restoreZ = player.fPatternBindRestoreZ;
	bool resolved = std::isfinite(restoreX) && std::isfinite(restoreY) &&
		std::isfinite(restoreZ);
	if (m_ServerNavigation.Is_Loaded())
	{
		resolved = false;
		SERVER_NAV_POINT ground{};
		if (std::isfinite(player.fPatternBindRestoreX) &&
			std::isfinite(player.fPatternBindRestoreZ) &&
			m_ServerNavigation.Is_PointWalkableExact(
				player.fPatternBindRestoreX, player.fPatternBindRestoreZ) &&
			m_ServerNavigation.Sample_Position(
				player.fPatternBindRestoreX,
				player.fPatternBindRestoreZ, ground) &&
			std::isfinite(ground.x) && std::isfinite(ground.y) &&
			std::isfinite(ground.z))
		{
			restoreX = player.fPatternBindRestoreX;
			restoreY = ground.y;
			restoreZ = player.fPatternBindRestoreZ;
			resolved = true;
		}
		else if (std::isfinite(player.fPatternBindRestoreX) &&
			std::isfinite(player.fPatternBindRestoreZ) &&
			(m_ServerNavigation.Project_PointOnSameLevel(
			player.fPatternBindRestoreX, player.fPatternBindRestoreZ, ground) ||
			m_ServerNavigation.Project_Point(
				player.fPatternBindRestoreX, player.fPatternBindRestoreZ, ground)) &&
			std::isfinite(ground.x) && std::isfinite(ground.y) &&
			std::isfinite(ground.z))
		{
			restoreX = ground.x;
			restoreY = ground.y;
			restoreZ = ground.z;
			resolved = true;
		}
		if (!resolved && std::isfinite(player.fPositionX) &&
			std::isfinite(player.fPositionZ) &&
			m_ServerNavigation.Is_PointWalkableExact(
				player.fPositionX, player.fPositionZ) &&
			m_ServerNavigation.Sample_Position(
				player.fPositionX, player.fPositionZ, ground) &&
			std::isfinite(ground.x) && std::isfinite(ground.y) &&
			std::isfinite(ground.z))
		{
			restoreX = player.fPositionX;
			restoreY = ground.y;
			restoreZ = player.fPositionZ;
			resolved = true;
		}
		if (!resolved && !player.strSpawnPlacementId.empty())
		{
			const WORLD_BOOTSTRAP_PLACEMENT* spawn =
				Find_Placement(player.strSpawnPlacementId);
			if (nullptr != spawn &&
				m_ServerNavigation.Is_PointWalkableExact(
					spawn->fPositionX, spawn->fPositionZ) &&
				m_ServerNavigation.Sample_Position(
					spawn->fPositionX, spawn->fPositionZ, ground) &&
				std::isfinite(ground.x) && std::isfinite(ground.y) &&
				std::isfinite(ground.z))
			{
				restoreX = spawn->fPositionX;
				restoreY = ground.y;
				restoreZ = spawn->fPositionZ;
				resolved = true;
			}
		}
	}
	if (!resolved)
		return false;
	const float restoreYaw = std::isfinite(player.fPatternBindRestoreYawDegrees) ?
		player.fPatternBindRestoreYawDegrees :
		(std::isfinite(player.fYawDegrees) ? player.fYawDegrees : 0.f);
	player.fPositionX = restoreX;
	player.fPositionY = restoreY;
	player.fPositionZ = restoreZ;
	player.fYawDegrees = restoreYaw;
	player.eAction = 0u == player.iCurrentHp ?
		LostArk::Shared::PLAYER_ACTION_STATE::DEAD :
		LostArk::Shared::PLAYER_ACTION_STATE::NONE;
	player.isCombatReady = 0u != player.iCurrentHp &&
		player.bPatternBindRestoreCombatReady;
	player.Clear_PatternBindStatus();
	return true;
}

const LostArk::Server::WORLD_BOOTSTRAP_PLACEMENT*
LostArk::Server::CGameRoom::Resolve_KoukuFallCenter(const SERVER_PLAYER& player) const
{
	if (m_eWorldId != LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA || player.iMarioStage ||
		player.eKoukuAreaHudMode == LostArk::Shared::KOUKU_HUD_MODE::MAZE) return nullptr;
	// Refinement grid rectangles are only bake coverage, not arena bounds.
	// The casino continues onto the base grid. Classify the separated authored
	// stages by their spawn anchors, not by Gate2Fine's small rectangle.
	const WORLD_BOOTSTRAP_PLACEMENT* nearest = nullptr;
	float distance = (std::numeric_limits<float>::max)();
	for (const char* id : { "stage.kakul.sl01", "stage.kakul.sl02", "stage.kakul.sl03",
		"stage.kakul.sl04", "stage.kakul.sl05" })
	{
		const auto* marker = Find_Placement(id);
		if (!marker || marker->eKind != WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN) continue;
		const float dx = player.fPositionX - marker->fPositionX;
		const float dz = player.fPositionZ - marker->fPositionZ;
		const float candidate = dx * dx + dz * dz;
		if (candidate < distance) { nearest = marker; distance = candidate; }
	}
	return nearest && nearest->strPlacementId == "stage.kakul.sl03" ? nearest : nullptr;
}

bool LostArk::Server::CGameRoom::Try_KoukuWalkOffFloor(
	SERVER_PLAYER& player, const float x, const float z,
	const float fixedDeltaSeconds, const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	if (m_eWorldId != WORLD_ID::KAKULSAYDON_ARENA || !m_ServerNavigation.Is_Loaded() ||
		!player.iCurrentHp || player.TriggerMove.isActive || player.bArenaEjectionActive ||
		(player.iVehicleId == ANCIENT_SEA_VEHICLE_ID && player.eAction == PLAYER_ACTION_STATE::VEHICLE_SKILL &&
		 player.eVehicleFlightPhase != VEHICLE_FLIGHT_PHASE::GROUNDED)) return false;
	const auto* gate2 = Resolve_KoukuFallCenter(player);
	const bool casino = nullptr != gate2;
	if (!player.iMarioStage && !casino) return false;
	SERVER_NAV_POINT surface{};
	// Obstacles keep their physical support. A blocked walking cell alone
	// must never be interpreted as a hole.
	const auto supported = [&](const float px, const float pz) {
		return m_ServerNavigation.Sample_SurfacePosition(px, pz, surface) &&
			surface.y >= player.fPositionY - m_ServerNavigation.Get_MaximumTraversalStepHeight();
	};
	if (supported(x, z)) return false;
	float resolvedX{}, resolvedY{}, resolvedZ{};
	bool blocked = false;
	if (!m_ServerCollisionSystem.Resolve_PlayerMove(player, x, player.fPositionY, z,
		resolvedX, resolvedY, resolvedZ, blocked) || blocked ||
		supported(resolvedX, resolvedZ)) return false;
	if (casino && !player.iMarioStage)
	{
		SERVER_NAV_POINT center{};
		if (!m_ServerNavigation.Project_Point(gate2->fPositionX, gate2->fPositionZ, center, gate2->fPositionY)) return false;
		player.KoukuFallRevivePosition = std::array<float, 3u>{ center.x, center.y, center.z };
	}
	player.fPositionX = resolvedX;
	player.fPositionZ = resolvedZ;
	Begin_PlayerFall(player, fixedDeltaSeconds, updateTick);
	return true;
}

bool LostArk::Server::CGameRoom::Update_PlayerFall(
	SERVER_PLAYER& player,
	const float fixedDeltaSeconds,
	const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	if ((player.bArenaEjectionActive ||
		(player.bKnockbackBallistic && player.fKnockbackRemainingSeconds > 0.f)) && 0u != player.iCurrentHp)
		return false;
	const auto gate = Resolve_CurrentKoukuGate();
	const bool gateFence = !player.iMarioStage && (gate == 1u || gate == 3u);
	if (gateFence && player.iCurrentHp && player.eAction == PLAYER_ACTION_STATE::FALLING)
	{
		SERVER_NAV_POINT start{}, ground{}; float yaw = 0.f;
		if (Resolve_KoukuRevivePosition(player, start, yaw) &&
			(!m_ServerNavigation.Is_Loaded() || m_ServerNavigation.Project_Point(start.x, start.z, ground, start.y)))
		{
			if (!m_ServerNavigation.Is_Loaded()) ground = start;
			player.fPositionX = ground.x; player.fPositionY = ground.y; player.fPositionZ = ground.z;
			player.fFallVelocityY = 0.f; player.iFallDeathTick = 0u;
			player.eAction = PLAYER_ACTION_STATE::NONE; player.isCombatReady = true;
		}
		return true;
	}
	if (PLAYER_ACTION_STATE::FALLING == player.eAction)
	{
		player.fFallVelocityY -=
			FALL_GRAVITY_METERS_PER_SECOND_SQUARED * fixedDeltaSeconds;
		player.fPositionY += player.fFallVelocityY * fixedDeltaSeconds;
		/* Signed difference so a wrapped tick counter keeps ordering, the same
		rule the cooldown deadlines use. */
		const std::int32_t sinceDeadline = static_cast<std::int32_t>(
			updateTick - player.iFallDeathTick);
		const bool reachedDeath = (m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA ||
			(m_eWorldId == WORLD_ID::MAHARAKA && player.bWaterpangFall)) ?
			(!std::isfinite(player.fFallDeathPlaneY) || player.fPositionY <= player.fFallDeathPlaneY) : sinceDeadline >= 0;
		if (!std::isfinite(player.fPositionY) || reachedDeath)
		{
			/* A Waterpang fall shows its descent, then returns the player alive on
			   the jump box nearest the fall so they can jump back onto the arena. */
			if (player.bWaterpangFall && player.iCurrentHp)
			{
				const WORLD_BOOTSTRAP_PLACEMENT* nearest = nullptr;
				float nearestDistance = (std::numeric_limits<float>::max)();
				for (const char* id : { "jump1", "jump2" })
				{
					const auto* box = Find_Placement(id);
					if (!box) continue;
					const float dx = player.fPositionX - box->fPositionX;
					const float dz = player.fPositionZ - box->fPositionZ;
					if (dx * dx + dz * dz < nearestDistance) { nearest = box; nearestDistance = dx * dx + dz * dz; }
				}
				SERVER_NAV_POINT ground{};
				if (nearest && m_ServerNavigation.Project_Point(
					nearest->fPositionX, nearest->fPositionZ, ground, nearest->fPositionY))
				{
					player.fPositionX = ground.x; player.fPositionY = ground.y; player.fPositionZ = ground.z;
					player.fFallVelocityY = 0.f; player.iFallDeathTick = 0u;
					player.eAction = PLAYER_ACTION_STATE::NONE; player.iActionStartTick = 0u;
					player.isCombatReady = true; player.bWaterpangFall = false;
					return true;
				}
			}
			player.iCurrentHp = 0u;
			player.eAction = PLAYER_ACTION_STATE::DEAD;
			player.iCurrentSkillId = INVALID_SKILL_ID;
			player.Clear_SkillTarget();
			player.iActionStartTick = 0u == updateTick ? 1u : updateTick;
			player.fFallVelocityY = 0.f;
			player.iFallDeathTick = 0u;
			Update_MarioControlState(player);
		}
		return true;
	}
	// Authored jumps and entry/exit transfers own their airborne trajectory.
	if (player.TriggerMove.isActive ||
		(player.iVehicleId == ANCIENT_SEA_VEHICLE_ID && player.eAction == PLAYER_ACTION_STATE::VEHICLE_SKILL &&
		 player.eVehicleFlightPhase != VEHICLE_FLIGHT_PHASE::GROUNDED)) return false;
	// The static island nav still contains the pre-match ring. During collapse it is
	// no longer physical support; use the same fall/revive path as Waterpang knockback.
	if (m_eWorldId == WORLD_ID::MAHARAKA && m_MaharakaWaterpangIntro && player.iCurrentHp &&
		player.fPositionY >= MAHARAKA_WATERPANG_DECK_MIN_Y_M &&
		Is_MaharakaWaterpangMissingRing(static_cast<std::int32_t>(updateTick - m_MaharakaWaterpangIntro->iStartTick),
			player.fPositionX, player.fPositionZ))
	{
		Begin_PlayerFall(player, fixedDeltaSeconds, updateTick);
		player.bWaterpangFall = true;
		return true;
	}
	if (gateFence || !m_ServerNavigation.Is_Loaded() ||
		0u == player.iCurrentHp ||
		PLAYER_ACTION_STATE::DEAD == player.eAction ||
		!m_ServerNavigation.Is_PointInVoidRegion(
			player.fPositionX, player.fPositionZ))
	{
		return false;
	}

	Begin_PlayerFall(player, fixedDeltaSeconds, updateTick);
	return true;
}

void LostArk::Server::CGameRoom::Update_Players(const float fixedDeltaSeconds)
{
	std::vector<SERVER_PLAYER*> colosseumPlayers;
	SERVER_COLOSSEUM_COMBAT_CONTEXT colosseumContext{ m_eWorldId, m_iColosseumMatchId,
		m_eColosseumPhase == LostArk::Shared::COLOSSEUM_MATCH_PHASE::ACTIVE, {} };
	if (m_iColosseumMatchId != 0u)
	{
		colosseumPlayers.reserve(m_Players.size());
		for (auto& [id, player] : m_Players) colosseumPlayers.push_back(&player);
		colosseumContext.Players = colosseumPlayers;
	}
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
        if (player.iColosseumMatchId && !player.bColosseumCombatActive &&
            !(m_eColosseumPhase == LostArk::Shared::COLOSSEUM_MATCH_PHASE::RECRUITING && player.Is_Human())) continue;
		// Bern personal guides hold their exact pose while the owner is away or sailing.
		if (m_eWorldId == LostArk::Shared::WORLD_ID::BERN && player.Is_Guide() && !player.isCombatReady) continue;
		if (player.CardMaze.transferStartTick && player.iCurrentHp) continue;
		const auto ownsLivePatternOccurrence =
			[this](const LostArk::Shared::NET_ENTITY_ID ownerEntityId,
				const std::uint32_t patternSequence)
			{
				return std::any_of(
					m_WorldEntities.begin(), m_WorldEntities.end(),
					[ownerEntityId, patternSequence](
						const SERVER_WORLD_ENTITY& entity)
					{
						return entity.iNetEntityId == ownerEntityId &&
							entity.iPatternSequence == patternSequence &&
							0u != entity.iCurrentHp &&
							SERVER_ENTITY_ACTION::DEAD != entity.eAction;
					});
			};
		if (!player.iCurrentHp || !player.Has_TimeStop(updateTick)) player.iTimeStopEndTick = 0u;
		if (!player.iCurrentHp || !player.Has_HolyCharmProtection(updateTick)) player.iHolyCharmProtectionEndTick = 0u;
        (void)CKoukuSaydonLogicRuntime::Update_PlayerFear(player, updateTick);
		Update_MarioControlState(player);
		Update_MarioBombContacts(player, updateTick);
		Update_MarioBouncingBallContacts(player, updateTick);
		Update_MaharakaWaterpangHazards(player, updateTick);
		if (m_eWorldId == LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA && player.iCurrentHp &&
			!player.iMarioStage && !player.TriggerMove.isActive &&
			player.eAction != LostArk::Shared::PLAYER_ACTION_STATE::FALLING)
		{
			const auto* center = Resolve_KoukuFallCenter(player);
			if (center)
			{
				SERVER_NAV_POINT ground{};
				if (m_ServerNavigation.Project_Point(center->fPositionX, center->fPositionZ, ground, center->fPositionY))
					player.KoukuFallRevivePosition = std::array<float, 3u>{ground.x, ground.y, ground.z};
			}
			else if (player.eAction == LostArk::Shared::PLAYER_ACTION_STATE::NONE)
				player.KoukuFallRevivePosition.reset();
		}
		/* A song that ended any way but its own timeout (a hit, a bind, death) never
		lands the player later. */
		if (0u != player.iSquareHoleId &&
			LostArk::Shared::PLAYER_ACTION_STATE::SQUAREHOLE_SONG != player.eAction)
		{
			player.iSquareHoleId = 0u;
		}
		if (0u == player.iCurrentHp ||
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD == player.eAction)
		{
			/* A lethal hit does not strand the replicated body five metres above
			the arena. Restore the admitted pose first, while preserving DEAD and
			combat-disabled state, then release both occurrence owners. */
			if (player.bPatternBound)
				(void)Restore_PatternBoundPlayer(player);
			player.Clear_SilenceStatus();
		}
		else
		{
			if (player.bPatternBound &&
				(Has_ReachedServerTick(updateTick, player.iPatternBindEndTick) ||
				 !ownsLivePatternOccurrence(
					player.iPatternBindOwnerNetEntityId,
					player.iPatternBindSequence)))
			{
				(void)Restore_PatternBoundPlayer(player);
			}
			if (0u != player.iSilenceEndTick &&
				Has_ReachedServerTick(updateTick, player.iSilenceEndTick))
			{
				player.Clear_SilenceStatus();
			}
		}
		if (LostArk::Shared::WORLD_ID::VALTAN_ARENA == m_eWorldId &&
			VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
				m_ValtanTimelineAudition.ePhase)
		{
			const bool driverMayPlay =
				player.iPlayerId == m_ValtanTimelineAudition.iOwnerPlayerId &&
				VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_FINISH ==
					m_ValtanTimelineAudition.ePhase;
			if (!driverMayPlay)
			{
				m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
				Freeze_TimelineAuditionPlayer(player);
				continue;
			}
		}
		if (player.bPatternBound)
		{
			player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
			player.iActionStartTick = 0u;
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0u;
			player.PendingCommand.Clear();
			// Binding owns the input lock; it must not make a living captive immune
			// to the same authoritative hit tests used for other participants.
			player.isCombatReady = player.bPatternBindRestoreCombatReady;
			continue;
		}
		if (LostArk::Shared::PLAYER_ACTION_STATE::FEAR == player.eAction) continue;
		if (Update_PlayerAttachment(player, updateTick))
			continue;
		if (Update_PlayerFall(player, fixedDeltaSeconds, updateTick))
			continue;
		if (player.Has_TimeStop(updateTick))
		{
			CServerBuffRuntime::Expire(player.ActiveBuffs, updateTick);
			continue;
		}
		const std::string authoredMoveSource = player.TriggerMove.strSourcePlacementId;
		// Preserve the saved jump boxes. Only a live match with a missing destination
		// extends its airborne crossing inward, so returning players do not fall forever.
		if (m_eWorldId == LostArk::Shared::WORLD_ID::MAHARAKA && m_MaharakaWaterpangIntro &&
			player.TriggerMove.isActive && LostArk::Shared::Is_MaharakaWaterpangJumpTrigger(authoredMoveSource) &&
			LostArk::Shared::Is_MaharakaWaterpangMissingRing(
				static_cast<std::int32_t>(updateTick - m_MaharakaWaterpangIntro->iStartTick),
				player.TriggerMove.fTargetX, player.TriggerMove.fTargetZ))
		{
			using namespace LostArk::Shared;
			const float dx = player.TriggerMove.fTargetX - MAHARAKA_WATERPANG_CANNON_X;
			const float dz = player.TriggerMove.fTargetZ - MAHARAKA_WATERPANG_CANNON_Z;
			const float scale = MAHARAKA_WATERPANG_COLLAPSED_LANDING_RADIUS_M / std::hypot(dx, dz);
			SERVER_NAV_POINT landing{};
			if (m_ServerNavigation.Sample_Position(MAHARAKA_WATERPANG_CANNON_X + dx * scale,
				MAHARAKA_WATERPANG_CANNON_Z + dz * scale, landing, player.TriggerMove.fTargetY) &&
				landing.y >= MAHARAKA_WATERPANG_DECK_MIN_Y_M)
			{
				player.TriggerMove.fTargetX = landing.x;
				player.TriggerMove.fTargetY = landing.y;
				player.TriggerMove.fTargetZ = landing.z;
			}
		}
		if (m_ServerTriggerSystem.Update_PlayerMotion(
			player, fixedDeltaSeconds))
		{
			if (authoredMoveSource.empty())
				Project_MarioRailPoint(player, player.fPositionX, player.fPositionZ);
			if (0u != player.iMarioStage && !player.TriggerMove.isActive && !authoredMoveSource.empty())
				(void)Configure_MarioRail(player, authoredMoveSource);
			Update_MarioControlState(player);
			if (!player.TriggerMove.isActive && !authoredMoveSource.empty())
				Complete_KoukuMarioReturn(player, authoredMoveSource, updateTick);
			/* Landing from a Waterpang crossing (jump1/jump2/jump3) protects the player for a
			   moment; the existing invulnerable tick already absorbs cannon, waterfall and
			   water gun hits, so none of them push or drop the player while it runs. */
			if (LostArk::Shared::WORLD_ID::MAHARAKA == m_eWorldId && !player.TriggerMove.isActive &&
				player.iCurrentHp &&
				LostArk::Shared::Is_MaharakaWaterpangJumpTrigger(authoredMoveSource))
			{
				const std::uint32_t protectUntil = Add_ServerTicksSkippingReservedZero(
					updateTick, LostArk::Shared::MAHARAKA_WATERPANG_JUMP_INVULNERABLE_TICKS);
				if (!player.iInvulnerableEndTick || Has_ReachedServerTick(protectUntil, player.iInvulnerableEndTick))
					player.iInvulnerableEndTick = protectUntil;
			}
            if (!player.TriggerMove.isActive && player.Is_Human())
            {
                // Bern's authored castle/library travel shares the squarehole arrival
                // contract, only after its blackout hold and displacement have completed.
                Guide_AnchorArrived(player,
                    m_eWorldId == LostArk::Shared::WORLD_ID::BERN && !authoredMoveSource.empty());
            }
			continue;
		}
		const bool wasKnockbackActive =
			player.fKnockbackRemainingSeconds > 0.f;
		Advance_PlayerKnockback(player, fixedDeltaSeconds);
		if (wasKnockbackActive)
			continue;
		if (LostArk::Shared::PLAYER_ACTION_STATE::KNOCKDOWN == player.eAction &&
			static_cast<std::int32_t>(
				updateTick - player.iKnockdownEndTick) >= 0)
		{
			player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
			player.Clear_SkillTarget();
			player.iActionStartTick = 0u;
			player.fActionElapsedSeconds = 0.f;
			player.iKnockdownEndTick = 0u;
			player.iHitReactionGraceEndTick = player.bPushOnlyHitReaction ? 0u :
				updateTick + PLAYER_HIT_REACTION_GRACE_TICKS;
			player.bPushOnlyHitReaction = false;
			player.PendingCommand.Clear();
		}
		/* The Esther call is a fixed-length lock, not a balance skill: the
		roster owns the summon, this block only releases the caster once the
		call clip has run out. Signed difference keeps ordering across a
		wrapped tick counter. */
		const bool estherCastElapsed =
			LostArk::Shared::PLAYER_ACTION_STATE::ESTHER_CAST == player.eAction &&
			static_cast<std::int32_t>(updateTick -
				(player.iActionStartTick + ESTHER_CAST_TICKS)) >= 0;
		/* The square-hole lock is the song plus a black hold: the Client screen is fully
		black when the song ticks run out, and the player lands inside the hold. */
		const bool squareHoleSongElapsed =
			LostArk::Shared::PLAYER_ACTION_STATE::SQUAREHOLE_SONG == player.eAction &&
			static_cast<std::int32_t>(updateTick -
				(player.iActionStartTick + SQUAREHOLE_LOCK_TICKS)) >= 0;
		if (squareHoleSongElapsed)
			Finish_SquareHoleSong(player);
		/* An escape teleport borrows the same INTERACTION lock and start tick,
		so a swing is judged only for a real hammer press. */
		const bool mazeHammerPress =
			LostArk::Shared::PLAYER_ACTION_STATE::INTERACTION == player.eAction &&
			LostArk::Shared::KOUKU_HUD_MODE::MAZE == player.eKoukuHudMode &&
			0u == player.CardMaze.transferStartTick;
		/* The maze hammer lands part-way through its press: judge the swing
		once, on that tick, against the run's targets in front of the player. */
		if (mazeHammerPress &&
			updateTick == player.iActionStartTick + CKoukuCardMazeRuntime::Hammer_HitTickOffset(player.iCurrentSkillId))
		{
			Resolve_CardMazeHammerHit(player, updateTick);
		}
		if (LostArk::Shared::PLAYER_ACTION_STATE::INTERACTION == player.eAction &&
			LostArk::Shared::KOUKU_HUD_MODE::MARIO == player.eKoukuHudMode &&
			0u == player.iCurrentSkillId &&
			updateTick == player.iActionStartTick + CKoukuCardMazeRuntime::HAMMER_HIT_TICK_OFFSET)
		{
			Resolve_MarioHammerHit(player, updateTick);
		}
		/* A KoukuSaydon interaction press is the same kind of lock, but it ends
		with the clip the pressed slot was authored on so the Client is never
		left holding a frozen last frame. An escape teleport borrows this action
		with no slot of its own, and INVALID_SKILL_ID is 0 -- the same value as
		slot 0 -- so it is separated by its transfer tick, not by the skill id. */
		const std::uint32_t interactionTicks =
			0u == player.CardMaze.transferStartTick ?
			CKoukuSaydonLogicRuntime::Ticks_FromMs(
				LostArk::Shared::Kouku_InteractionActionMs(
					player.eKoukuHudMode, player.iCurrentSkillId)) :
			KOUKU_INTERACTION_TICKS;
		const bool interactionElapsed =
			LostArk::Shared::PLAYER_ACTION_STATE::INTERACTION == player.eAction &&
			static_cast<std::int32_t>(updateTick -
				(player.iActionStartTick + interactionTicks)) >= 0;
		if (estherCastElapsed || squareHoleSongElapsed || interactionElapsed)
		{
			player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
			player.iActionStartTick = 0u;
			player.fActionElapsedSeconds = 0.f;
			player.PendingCommand.Clear();
		}
		CServerBuffRuntime::Expire(player.ActiveBuffs, updateTick);
		CServerBuffRuntime::Settle_Shield(m_GameplayCatalog.Active(), player);
		Update_VehicleSkill(player, fixedDeltaSeconds);
		m_PlayerSkillSystem.Update(
			player,
			m_WorldEntities,
			m_GameplayCatalog,
			m_ServerNavigation.Is_Loaded() ? &m_ServerNavigation : nullptr,
			&m_ServerCollisionSystem,
			fixedDeltaSeconds,
			updateTick,
			m_TickDamageEvents,
			m_iColosseumMatchId != 0u ? &colosseumContext : nullptr);
		if (LostArk::Shared::PLAYER_ACTION_STATE::NONE == player.eAction &&
			PLAYER_PENDING_COMMAND_KIND::NONE != player.PendingCommand.eKind)
		{
			Commit_PendingPlayerCommand(player, updateTick);
		}
		if (LostArk::Shared::PLAYER_ACTION_STATE::NONE != player.eAction)
			continue;
		Update_MarioMoveGoal(player, updateTick);
		if (!player.hasMoveGoal)
			continue;
		/* The route was pulled taut from where the player stood when it was
		built, and the player has moved since. Pull it again from here and aim
		at the farthest waypoint still in sight. Without this the player keeps
		facing the first waypoint - a neighbouring cell centre whose bearing has
		little to do with the goal's - and every rebuild puts that cell on the
		other side, which is what made a held right mouse shake. */
		if (!player.MovePath.empty() && m_ServerNavigation.Is_Loaded())
		{
			for (std::size_t candidate = player.MovePath.size();
				candidate > player.iMovePathIndex + 1u; --candidate)
			{
				const SERVER_NAV_POINT& ahead = player.MovePath[candidate - 1u];
				if (m_ServerNavigation.Has_LineOfSight(
					player.fPositionX, player.fPositionZ, ahead.x, ahead.z,
					player.fPositionY))
				{
					player.iMovePathIndex = candidate - 1u;
					break;
				}
			}
		}
		float targetX = player.fMoveGoalX;
		float targetY = player.fPositionY;
		float targetZ = player.fMoveGoalZ;
		if (player.iMovePathIndex < player.MovePath.size())
		{
			const SERVER_NAV_POINT& pathPoint =
				player.MovePath[player.iMovePathIndex];
			targetX = pathPoint.x;
			targetY = pathPoint.y;
			targetZ = pathPoint.z;
		}
		/* A smoothed path's intermediate points are corners, not arrivals. Only
		the last one is where the player stops and has to end up facing. */
		const bool targetIsDestination =
			player.iMovePathIndex + 1u >= player.MovePath.size();
		const float deltaX = targetX - player.fPositionX;
		const float deltaZ = targetZ - player.fPositionZ;
		const float distance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
		const bool reachedPathPoint = distance <= MOVE_STOP_DISTANCE;
		float proposedX = targetX;
		float proposedY = targetY;
		float proposedZ = targetZ;
		if (!reachedPathPoint)
		{
			const float desiredYaw =
				std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
			const float yawDifference =
				Wrap_Degrees(desiredYaw - player.fYawDegrees);
			const float maxYawStep =
				(LostArk::Shared::INVALID_VEHICLE_ID != player.iVehicleId ?
					VEHICLE_TURN_DEGREES_PER_SECOND :
					PLAYER_TURN_DEGREES_PER_SECOND) * fixedDeltaSeconds;
			/* Close to the destination the turn radius no longer fits, so facing
			snaps rather than orbiting the point. A corner is not a destination:
			snapping there made every path bend read as an instant pivot. */
			if (0u != player.iMarioStage ||
				(targetIsDestination && distance <= DIRECT_BEARING_DISTANCE) ||
				std::abs(yawDifference) <= maxYawStep)
			{
				player.fYawDegrees = desiredYaw;
			}
			else
			{
				player.fYawDegrees = Wrap_Degrees(player.fYawDegrees +
					(yawDifference > 0.f ? maxYawStep : -maxYawStep));
			}
			const float moveDistance = (std::min)(
				Resolve_PlayerMoveSpeed(player) *
					fixedDeltaSeconds,
				distance);
			const float moveRatio = moveDistance / distance;
			// Movement follows the requested path immediately; facing catches up
			// independently so an opposite click does not first walk sideways.
			const float stepX = deltaX * moveRatio;
			const float stepZ = deltaZ * moveRatio;
			proposedX = player.fPositionX + stepX;
			proposedY = player.fPositionY +
				(targetY - player.fPositionY) * moveRatio;
			proposedZ = player.fPositionZ + stepZ;
		}
		Project_MarioRailPoint(player, proposedX, proposedZ);
		if (Try_KoukuWalkOffFloor(player, proposedX, proposedZ, fixedDeltaSeconds, updateTick))
			continue;
		/* A smoothed path can skip many authored cells. Never interpolate Y toward
		the distant waypoint: doing so raises the player while XZ is still on the
		lower deck and lets a later height check see an already-raised player.
		Resolve both XZ positions against navigation and take only its ground Y. */
		if (m_ServerNavigation.Is_Loaded())
		{
			SERVER_NAV_POINT proposedGround{};
			if (!m_ServerNavigation.Resolve_TraversalStep(
				player.fPositionX,
				player.fPositionZ,
				proposedX,
				proposedZ,
				proposedGround,
				player.fPositionY))
			{
				player.hasMoveGoal = false;
				player.MovePath.clear();
				player.iMovePathIndex = 0u;
				continue;
			}
			proposedY = proposedGround.y;
		}

		float resolvedX = player.fPositionX;
		float resolvedY = player.fPositionY;
		float resolvedZ = player.fPositionZ;
		bool wasBlocked = false;
		if (!m_ServerCollisionSystem.Resolve_PlayerMove(
			player,
			proposedX,
			proposedY,
			proposedZ,
			resolvedX,
			resolvedY,
			resolvedZ,
			wasBlocked))
		{
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0;
			continue;
		}
		Project_MarioRailPoint(player, resolvedX, resolvedZ);
		/* Body collision may slide XZ away from the point checked above. Validate
		the final slide destination too and ground it before committing any
		authoritative coordinate. */
		if (m_ServerNavigation.Is_Loaded())
		{
			SERVER_NAV_POINT resolvedGround{};
			if (!m_ServerNavigation.Resolve_TraversalStep(
				player.fPositionX,
				player.fPositionZ,
				resolvedX,
				resolvedZ,
				resolvedGround,
				player.fPositionY))
			{
				player.hasMoveGoal = false;
				player.MovePath.clear();
				player.iMovePathIndex = 0u;
				continue;
			}
			resolvedY = resolvedGround.y;
		}
		// Successful tangent slides clear wasBlocked; a deflected step still
		// reached the body and must finish a goal inside that same body.
		const bool reachedGoalBody =
			(wasBlocked || resolvedX != proposedX || resolvedZ != proposedZ) &&
			m_ServerCollisionSystem.Is_PlayerMoveBlockedAtGoalBody(player,
				proposedX, proposedY, proposedZ,
				player.MovePath.empty() ? player.fPositionY : player.MovePath.back().y);
		player.fPositionX = resolvedX;
		player.fPositionY = resolvedY;
		player.fPositionZ = resolvedZ;
		if (reachedGoalBody)
		{
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0u;
			continue;
		}
		if (wasBlocked)
		{
			/* The body sweep met something the navigation grid does not carry:
			a collisionBox, or another body. Route around it and keep the goal.
			Dropping the goal here is what made a held right mouse shake next
			to an obstacle: every brush cancelled the move, and the re-send
			50 ms later started a fresh search from a start cell that had
			moved, so the first waypoint jumped from side to side.

			The rebuild is rate limited because brushing reports blocked on
			many ticks in a row. Between rebuilds the move is left alone and
			the existing slide carries it along the obstacle. */
			const bool mayReroute = m_ServerNavigation.Is_Loaded() &&
				updateTick - player.iMoveRerouteTick >= MOVE_REROUTE_MIN_TICKS;
			if (!mayReroute)
				continue;
			player.iMoveRerouteTick = updateTick;
			std::vector<SERVER_NAV_POINT> reroute;
			if (m_ServerNavigation.Find_Path(
					player.fPositionX,
					player.fPositionZ,
					player.fMoveGoalX,
					player.fMoveGoalZ,
					reroute,
					player.fPositionY))
			{
				m_ServerNavigation.Smooth_Path(
					player.fPositionX,
					player.fPositionZ,
					player.fMoveGoalX,
					player.fMoveGoalZ,
					reroute,
					player.fPositionY);
				player.MovePath = std::move(reroute);
				player.iMovePathIndex = 0;
				continue;
			}
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0;
			continue;
		}
		if (reachedPathPoint)
		{
			if (player.iMovePathIndex < player.MovePath.size())
				++player.iMovePathIndex;
			if (player.iMovePathIndex >= player.MovePath.size())
			{
				player.hasMoveGoal = false;
				player.MovePath.clear();
				player.iMovePathIndex = 0;
			}
			continue;
		}
	}
}

bool LostArk::Server::CGameRoom::Resolve_ArenaCenter(
	const SERVER_WORLD_ENTITY& boss, SERVER_NAV_POINT& point)
{
	const bool exact = m_ServerNavigation.Is_PointWalkableExact(
		boss.fSpawnPositionX, boss.fSpawnPositionZ) &&
		m_ServerNavigation.Sample_Position(
			boss.fSpawnPositionX, boss.fSpawnPositionZ, point);
	if (!m_ServerNavigation.Is_Loaded() ||
		(!exact && !m_ServerNavigation.Project_PointOnSameLevel(
			boss.fSpawnPositionX, boss.fSpawnPositionZ, point)) ||
		!m_ServerNavigation.Is_PointWalkableExact(point.x, point.z) ||
		std::fabs(point.y - boss.fSpawnPositionY) > 1.5f ||
		std::hypot(point.x - boss.fSpawnPositionX,
			point.z - boss.fSpawnPositionZ) > 8.f)
	{
		m_strStatus = "Arena center has no nearby walkable same-level recovery point";
		return false;
	}
	return true;
}

bool LostArk::Server::CGameRoom::Prepare_ArenaEjection(
	SERVER_PLAYER& staged,
	const SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_STAGE_ACTION& action,
	const std::uint32_t serverTick)
{
	if (BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION != action.eReleaseMode ||
		!m_ServerNavigation.Is_Loaded() ||
		!std::isfinite(action.fReleaseSpeedMps) || action.fReleaseSpeedMps <= 0.f ||
		action.fReleaseSpeedMps > 50.f || 0u == action.iDurationMs ||
		action.iDurationMs > 5000u ||
		!std::isfinite(action.fReleaseYawOffsetDegrees) ||
		std::abs(action.fReleaseYawOffsetDegrees) > 180.f ||
		!std::isfinite(boss.fYawDegrees))
	{
		m_strStatus = "Arena ejection policy or navigation is invalid";
		return false;
	}
	const float yaw = (boss.fYawDegrees + action.fReleaseYawOffsetDegrees) *
		DEGREES_TO_RADIANS;
	const float directionX = -std::sin(yaw);
	const float directionZ = -std::cos(yaw);
	constexpr float maximumDistance = 128.f;
	constexpr float outsideMargin = 2.f;
	const float sampleStep = std::clamp(m_ServerNavigation.Get_CellSize(), 0.1f, 0.5f);
	float lastArenaGround = 0.f;
	/* Scan past small holes and seams. An interior missing cell is not the arena
	   exterior; the endpoint lies beyond the last same-deck ground on this ray. */
	for (float distance = 0.f; distance <= maximumDistance; distance += sampleStep)
	{
		const float x = staged.fPositionX + directionX * distance;
		const float z = staged.fPositionZ + directionZ * distance;
		SERVER_NAV_POINT ground{};
		if (m_ServerNavigation.Is_PointWalkableExact(x, z) &&
			m_ServerNavigation.Sample_Position(x, z, ground) &&
			std::fabs(ground.y - boss.fSpawnPositionY) <= 1.5f)
			lastArenaGround = distance;
	}
	const float minimumDistance = action.fReleaseSpeedMps *
		(static_cast<float>(action.iDurationMs) / 1000.f);
	const float distance = (std::max)(minimumDistance, lastArenaGround + outsideMargin);
	if (!std::isfinite(distance) || distance > maximumDistance ||
		!Release_PlayerAttachment(staged, boss.iNetEntityId,
			0.f, 0u, false, 0u, serverTick))
	{
		m_strStatus = "Arena ejection has no bounded exterior destination";
		return false;
	}
	if (0u == staged.iCurrentHp)
		return true;
	staged.fKnockbackDirectionX = directionX;
	staged.fKnockbackDirectionZ = directionZ;
	staged.fKnockbackSpeed = action.fReleaseSpeedMps;
	staged.fKnockbackRemainingSeconds = distance / action.fReleaseSpeedMps;
	staged.bArenaEjectionActive = true;
	staged.iEjectionOwnerNetEntityId = boss.iNetEntityId;
	staged.isCombatReady = false;
	return true;
}

void LostArk::Server::CGameRoom::Advance_PlayerKnockback(
	SERVER_PLAYER& player, const float fixedDeltaSeconds)
{
	if (!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f ||
		player.fKnockbackRemainingSeconds <= 0.f)
	{
		return;
	}
	if (0u == player.iCurrentHp ||
		LostArk::Shared::PLAYER_ACTION_STATE::DEAD == player.eAction)
	{
		player.Clear_Attachment();
		player.fKnockbackRemainingSeconds = 0.f;
		player.fKnockbackSpeed = 0.f;
		player.bWaterpangLaunch = false;
		return;
	}
	const float step = (std::min)(
		fixedDeltaSeconds, player.fKnockbackRemainingSeconds);
	float desiredX = player.fPositionX +
		player.fKnockbackDirectionX * player.fKnockbackSpeed * step;
	float desiredZ = player.fPositionZ +
		player.fKnockbackDirectionZ * player.fKnockbackSpeed * step;
	const auto gate = Resolve_CurrentKoukuGate();
	const bool gateFence = !player.iMarioStage && (gate == 1u || gate == 3u);
	if (gateFence) player.bKnockbackCanLeaveArena = false;
	// Once the match has started, a push may carry a player off the Waterpang arena.
	const std::uint32_t knockbackTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;
	const bool waterpangMatch = m_eWorldId == LostArk::Shared::WORLD_ID::MAHARAKA &&
		((m_MaharakaWaterpangIntro && Has_ReachedServerTick(knockbackTick, m_MaharakaWaterpangIntro->iStartTick)) ||
		 Is_MaharakaWaterpangDebugEventLive(knockbackTick));
	const bool waterpangPush = waterpangMatch &&
		LostArk::Shared::Is_MaharakaWaterpangArenaFootprint(player.fPositionX, player.fPositionZ) &&
		m_ServerNavigation.Is_PointWalkableInRegion(LostArk::Shared::MAHARAKA_WATERPANG_REGION_ID,
			player.fPositionX, player.fPositionZ, player.fPositionY);
	if (waterpangPush) player.bKnockbackCanLeaveArena = true;
	if (player.bKnockbackBallistic)
	{
		// A bounded launch keeps the authored Y arc while its ground footprint
		// obeys the same navigation and fence collision as ordinary movement.
		const bool bounded = !player.bKnockbackCanLeaveArena && !player.bWaterpangLaunch;
		if (bounded)
		{
			SERVER_NAV_POINT reachable{desiredX, player.fKnockbackSupportY, desiredZ};
			bool clamped = false;
			if (m_ServerNavigation.Is_Loaded())
				CPlayerSkillSystem::Clamp_StepToWalkable(m_ServerNavigation, player.fPositionX, player.fPositionZ,
					desiredX, desiredZ, reachable, clamped, player.fKnockbackSupportY);
			SERVER_PLAYER groundBody = player;
			groundBody.fPositionY = player.fKnockbackSupportY;
			float x = player.fPositionX, y = player.fKnockbackSupportY, z = player.fPositionZ;
			bool blocked = false;
			if (!m_ServerCollisionSystem.Resolve_PlayerMove(groundBody, reachable.x, reachable.y, reachable.z, x, y, z, blocked))
			{ x = player.fPositionX; z = player.fPositionZ; blocked = true; }
			SERVER_NAV_POINT valid{};
			if (m_ServerNavigation.Is_Loaded() &&
				(!m_ServerNavigation.Has_LineOfSight(player.fPositionX, player.fPositionZ, x, z) ||
				 !m_ServerNavigation.Resolve_TraversalStep(player.fPositionX, player.fPositionZ, x, z, valid)))
			{ x = player.fPositionX; z = player.fPositionZ; blocked = true; }
			SERVER_NAV_POINT support{};
			if (m_ServerNavigation.Is_Loaded() &&
				(!m_ServerNavigation.Sample_SurfacePosition(x, z, support) ||
				 std::abs(support.y - player.fKnockbackSupportY) > 1.f))
			{ x = player.fPositionX; z = player.fPositionZ; blocked = true; }
			desiredX = x; desiredZ = z;
			if (clamped || blocked) player.fKnockbackSpeed = 0.f;
		}
		player.fPositionX = desiredX;
		player.fPositionZ = desiredZ;
		player.fPositionY += player.fKnockbackVelocityY * step -
			0.5f * player.fKnockbackGravityMps2 * step * step;
		player.fKnockbackVelocityY -= player.fKnockbackGravityMps2 * step;
		player.fKnockbackRemainingSeconds = (std::max)(0.f, player.fKnockbackRemainingSeconds - step);
		/* A Waterpang waterfall launch never lands on the deck, a pier or the pool
		   floor it crosses: its flight always ends in the Waterpang fall. */
		if (player.bWaterpangLaunch)
		{
			if (player.fKnockbackRemainingSeconds > 0.00001f)
				return;
			const float velocityY = player.fKnockbackVelocityY;
			const std::uint32_t tick = (std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;
			Begin_PlayerFall(player, 0.f, tick);
			player.fFallVelocityY = velocityY;
			player.bWaterpangFall = true;
			return;
		}
		const float fallDepth = Resolve_KoukuFallCenter(player) ? KOUKU_CASINO_FALL_DEPTH_M : KOUKU_FALL_DEPTH_M;
		if (!bounded && m_eWorldId == LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA &&
			player.fPositionY <= player.fKnockbackSupportY - fallDepth)
		{
			const std::uint32_t tick = (std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;
			Begin_PlayerFall(player, 0.f, tick);
			(void)Update_PlayerFall(player, 0.f, tick);
			return;
		}
		SERVER_NAV_POINT floor{ desiredX, player.fKnockbackLaunchY, desiredZ };
		bool hasFloor = !m_ServerNavigation.Is_Loaded() ||
			m_ServerNavigation.Sample_SurfacePosition(desiredX, desiredZ, floor);
		/* A ballistic player may cross an overlapping upper deck while leaving an
		   arena.  That deck is not a landing surface for a flight launched from
		   below it: accepting it would snap Y upward and turn the dead-zone fall
		   into a nav teleport.  Lower floors remain valid and are handled by the
		   normal gravity/dead-zone path. */
		constexpr float maximumLandingRiseM = 0.01f;
		if (hasFloor && std::isfinite(floor.y) &&
			floor.y > player.fKnockbackLaunchY + maximumLandingRiseM)
			hasFloor = false;
		if (bounded && (!hasFloor || std::abs(floor.y - player.fKnockbackSupportY) > 1.f))
		{
			floor = {desiredX, player.fKnockbackSupportY, desiredZ};
			hasFloor = true;
		}
		if (hasFloor && m_eWorldId == LostArk::Shared::WORLD_ID::MAHARAKA && m_MaharakaWaterpangIntro &&
			floor.y >= LostArk::Shared::MAHARAKA_WATERPANG_DECK_MIN_Y_M &&
			LostArk::Shared::Is_MaharakaWaterpangMissingRing(
				static_cast<std::int32_t>(knockbackTick - m_MaharakaWaterpangIntro->iStartTick), desiredX, desiredZ))
			hasFloor = false;
		if (player.fKnockbackVelocityY <= 0.f && hasFloor && player.fPositionY <= floor.y + 0.0001f)
		{
			player.fPositionY = floor.y;
			player.fKnockbackRemainingSeconds = player.fKnockbackSpeed = player.fKnockbackVelocityY = 0.f;
			player.bKnockbackBallistic = player.bKnockbackCanLeaveArena = false;
			const auto landingTick = Add_ServerTicksSkippingReservedZero(m_iServerTick, 1u);
			const auto recoveryEnd = Add_ServerTicksSkippingReservedZero(landingTick,
				CKoukuSaydonLogicRuntime::Ticks_FromMs(PLAYER_HIT_LANDING_RECOVERY_MS));
			if (player.eAction == LostArk::Shared::PLAYER_ACTION_STATE::KNOCKDOWN &&
				static_cast<std::int32_t>(recoveryEnd - player.iKnockdownEndTick) > 0)
				player.iKnockdownEndTick = recoveryEnd;
		}
		else if (player.fKnockbackRemainingSeconds <= 0.00001f)
		{
			if (hasFloor)
			{
				// A lower deck takes longer than the nominal same-height flight.
				// Keep descending at the endpoint without snapping down to that deck.
				player.fKnockbackSpeed = 0.f;
				player.fKnockbackRemainingSeconds = fixedDeltaSeconds;
			}
			else
			{
				const float velocityY = player.fKnockbackVelocityY;
				// A launch leaves the deck before it ends, so the match alone decides.
				const bool waterpangFall = waterpangMatch && player.bKnockbackCanLeaveArena;
				const std::uint32_t tick = (std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;
				Begin_PlayerFall(player, 0.f, tick);
				player.fFallVelocityY = velocityY;
				player.bWaterpangFall = waterpangFall;
			}
		}
		return;
	}
	Project_MarioRailPoint(player, desiredX, desiredZ);
	// Ordinary/arena pushes must reach their existing bounded or swept-surface
	// mover. Only an authored Mario exit uses the rail's walking-floor check.
	if (player.iMarioStage && player.bKnockbackCanLeaveArena &&
		Try_KoukuWalkOffFloor(player, desiredX, desiredZ, fixedDeltaSeconds,
			Add_ServerTicksSkippingReservedZero(m_iServerTick, 1u))) return;
	if (player.bArenaEjectionActive)
	{
		player.fPositionX = desiredX;
		player.fPositionZ = desiredZ;
		player.fKnockbackRemainingSeconds =
			(std::max)(0.f, player.fKnockbackRemainingSeconds - step);
		const auto owner = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
			[&player](const SERVER_WORLD_ENTITY& boss)
			{ return boss.iNetEntityId == player.iEjectionOwnerNetEntityId; });
		if (player.fKnockbackRemainingSeconds <= 0.00001f ||
			owner == m_WorldEntities.end() || 0u == owner->iCurrentHp ||
			SERVER_ENTITY_ACTION::DEAD == owner->eAction)
		{
			const std::uint32_t updateTick =
				(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
				1u : m_iServerTick + 1u;
			Begin_PlayerFall(player, fixedDeltaSeconds, updateTick);
		}
		return;
	}
	if (m_ServerNavigation.Is_Loaded() &&
		(m_eWorldId == LostArk::Shared::WORLD_ID::VALTAN_ARENA || waterpangPush ||
		 (m_eWorldId == LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA &&
		  player.bKnockbackCanLeaveArena && !player.iMarioStage)))
	{
		/* Valtan hits and authored Kouku arena-exit hits cross walking boundaries. Keep actual walls
		   and bodies authoritative, but do not route or clamp the displacement
		   to walkable cells. A push is a straight sweep, not walking avoidance
		   around another body; this also gives support one exact segment. */
		using namespace LostArk::Shared::WorldCollision;
		float resolvedX = player.fPositionX;
		float resolvedY = player.fPositionY;
		float resolvedZ = player.fPositionZ;
		bool wasBlocked = false;
		if (!m_ServerCollisionSystem.Resolve_CircleMove(
			player.fPositionX, player.fPositionY, player.fPositionZ,
			desiredX, player.fPositionY, desiredZ,
			PLAYER_HALF_EXTENT_X, PLAYER_HALF_EXTENT_Y, PLAYER_CENTER_OFFSET_Y,
			resolvedX, resolvedY, resolvedZ, wasBlocked,
			player.iNetEntityId, false))
		{
			player.fKnockbackRemainingSeconds = player.fKnockbackSpeed = 0.f;
			return;
		}
		SERVER_NAV_POINT supported{};
		const FORCED_SURFACE_RESULT support = Trace_ForcedSurface(
			m_ServerNavigation,
			{player.fPositionX, player.fPositionY, player.fPositionZ},
			resolvedX, resolvedZ, supported);
		player.fPositionX = supported.x;
		player.fPositionY = supported.y;
		player.fPositionZ = supported.z;
		if (FORCED_SURFACE_RESULT::FALL == support)
		{
			const std::uint32_t updateTick =
				(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;
			Begin_PlayerFall(player, fixedDeltaSeconds, updateTick);
			player.bWaterpangFall = waterpangPush;
			return;
		}
		player.fKnockbackRemainingSeconds = wasBlocked || FORCED_SURFACE_RESULT::BLOCKED == support ?
			0.f : (std::max)(0.f, player.fKnockbackRemainingSeconds - step);
		if (player.fKnockbackRemainingSeconds <= 0.f)
		{
			player.fKnockbackSpeed = 0.f;
			player.bKnockbackCanLeaveArena = false;
		}
		return;
	}
	SERVER_NAV_POINT reachable{ desiredX, player.fPositionY, desiredZ };
	bool wasClamped = false;
	if (m_ServerNavigation.Is_Loaded())
	{
		CPlayerSkillSystem::Clamp_StepToWalkable(
			m_ServerNavigation,
			player.fPositionX,
			player.fPositionZ,
			desiredX,
			desiredZ,
			reachable,
			wasClamped,
			player.fPositionY);
	}
	Project_MarioRailPoint(player, reachable.x, reachable.z);
	if (0u != player.iMarioStage)
	{
		SERVER_NAV_POINT railGround{};
		if (!player.bMarioRailReady || !m_ServerNavigation.Resolve_TraversalStep(
			player.fPositionX, player.fPositionZ, reachable.x, reachable.z, railGround))
		{
			player.fKnockbackRemainingSeconds = player.fKnockbackSpeed = 0.f;
			return;
		}
		reachable.y = railGround.y;
	}
	float resolvedX = player.fPositionX;
	float resolvedY = player.fPositionY;
	float resolvedZ = player.fPositionZ;
	bool wasBlocked = false;
	if (!m_ServerCollisionSystem.Resolve_PlayerMove(
		player,
		reachable.x,
		reachable.y,
		reachable.z,
		resolvedX,
		resolvedY,
		resolvedZ,
		wasBlocked))
	{
		player.fKnockbackRemainingSeconds = 0.f;
		player.fKnockbackSpeed = 0.f;
		return;
	}
	Project_MarioRailPoint(player, resolvedX, resolvedZ);
	if (0u != player.iMarioStage)
	{
		SERVER_NAV_POINT railGround{};
		if (!m_ServerNavigation.Resolve_TraversalStep(
			player.fPositionX, player.fPositionZ, resolvedX, resolvedZ, railGround))
		{
			player.fKnockbackRemainingSeconds = player.fKnockbackSpeed = 0.f;
			return;
		}
		resolvedY = railGround.y;
	}
	if (gateFence && m_ServerNavigation.Is_Loaded())
	{
		SERVER_NAV_POINT supported{};
		const auto support = Trace_ForcedSurface(m_ServerNavigation,
			{player.fPositionX, player.fPositionY, player.fPositionZ}, resolvedX, resolvedZ, supported);
		if (support != FORCED_SURFACE_RESULT::SUPPORTED)
		{
			player.fKnockbackRemainingSeconds = player.fKnockbackSpeed = 0.f;
			return;
		}
		resolvedY = supported.y;
	}
	player.fPositionX = resolvedX;
	player.fPositionY = resolvedY;
	player.fPositionZ = resolvedZ;
	player.fKnockbackRemainingSeconds = (wasClamped || wasBlocked) ?
		0.f : player.fKnockbackRemainingSeconds - step;
	if (player.fKnockbackRemainingSeconds <= 0.f)
	{
		player.fKnockbackRemainingSeconds = 0.f;
		player.fKnockbackSpeed = 0.f;
		player.bKnockbackCanLeaveArena = false;
	}
}
```

### G11. Server/Private/ServerGameplayContractTests_Guide.cpp 전체 코드

```cpp
#include "ServerGameplayContractTests_Runner.h"
#include "GameRoom.h"
#include "ServerApp.h"
#include "ClientSession.h"
#include "Network/PacketReader.h"
#include <algorithm>
#include <memory>
#include <cmath>
#include <array>

using namespace LostArk::Server;
using namespace LostArk::Shared;

int CServerGameplayContractRunner::Run_GuideAI()
{
 TESTS tests;
 auto source=std::make_shared<CGameRoom>(WORLD_ID::BERN);
 auto target=std::make_shared<CGameRoom>(WORLD_ID::VALTAN_ARENA);
 tests.Require(source->Is_Ready()&&target->Is_Ready()&&source->m_GuideCatalog.Loaded&&target->m_GuideCatalog.Loaded,"Guide published catalogs load in Bern and Valtan");
 if(!source->Is_Ready()||!target->Is_Ready()||!source->m_GuideCatalog.Loaded||!source->m_iGuideReceptionId)return 1;
 const auto reception=source->m_Players.at(source->m_iGuideReceptionId);
 const auto guideId=reception.iPlayerId;
 tests.Require(reception.Is_Guide()&&!reception.isCombatReady&&reception.iSessionId==INVALID_SESSION_ID&&source->Count_HumanPlayers()==0,"One placed guide has no fake session or human slot");
 tests.Require(std::none_of(target->m_Players.begin(),target->m_Players.end(),[](const auto& p){return p.second.Is_Guide();}),"A raid room contains no guide actor");
 std::vector<std::shared_ptr<CClientSession>> sessions;
 auto drain=[&](){for(auto& session:sessions){session->m_OutboundFrames.clear();session->m_iQueuedOutboundBytes=0;session->m_OutboundMetrics.iCurrentQueuedByteCount=0;session->m_OutboundMetrics.iCurrentQueuedFrameCount=0;}};
 auto hasFrame=[](const auto& session,PACKET_TYPE type){return std::any_of(session->m_OutboundFrames.begin(),session->m_OutboundFrames.end(),[&](const auto& f){return f.ePacketType==type;});};
 auto lastState=[](const auto& session){S2C_GUIDE_STATE result;for(const auto& frame:session->m_OutboundFrames)if(frame.ePacketType==PACKET_TYPE::S2C_GUIDE_STATE){CPacketReader reader{std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES)};(void)Read_Message(reader,result);}return result;};
 bool joined=true;
 for(unsigned i=0;i<4;++i){auto session=std::make_shared<CClientSession>(99101u+i,INVALID_SOCKET,CClientSession::FRAME_HANDLER{},CClientSession::CLOSED_HANDLER{});session->m_isSendRunning.store(true);source->Handle_Register(session);C2S_ENTER_WORLD enter;enter.eWorldId=WORLD_ID::BERN;enter.eCharacterClass=CHARACTER_CLASS_ID::DIMENSIONMASTER;enter.strNickName="GuideContract"+std::to_string(i);joined=source->Join(session->Get_SessionId(),enter)&&joined;sessions.push_back(session);drain();}
 tests.Require(joined&&source->Count_HumanPlayers()==4,"Four human admissions remain available beside the single guide");
 if(!joined)return 1;
 auto& leader=source->m_Players.at(sessions[0]->Get_PlayerId());
 leader.fPositionX=reception.fPositionX+2.f;leader.fPositionY=reception.fPositionY;leader.fPositionZ=reception.fPositionZ;
 const auto ownerId=leader.iSessionId;
 auto control=[&](SESSION_ID owner,std::uint32_t sequence,GUIDE_CONTROL_ACTION action){C2S_GUIDE_CONTROL request;request.iRequestSequence=sequence;request.iGuideNetEntityId=reception.iNetEntityId;request.eAction=action;source->Handle_GuideControl(owner,request);};
 const auto actorCount=source->m_Players.size();
 C2S_PARTY_INVITE legacy;legacy.iTargetNetEntityId=reception.iNetEntityId;source->Handle_PartyInvite(ownerId,legacy);
 tests.Require(source->m_PersonalGuides.empty()&&source->m_PartyMembersByPartyId.empty(),"A legacy party invitation cannot create guidance or a hidden party");
 GUIDE_TRIGGER startBox;startBox.Id="guide.contract.start-inside";startBox.Type="SPACE_ENTER";startBox.Category=source->m_GuideCatalog.Categories.at(WORLD_ID::BERN);startBox.PromptId="guide.contract.start-space";
 startBox.Box.fPositionX=leader.fPositionX;startBox.Box.fPositionY=leader.fPositionY+1.f;startBox.Box.fPositionZ=leader.fPositionZ;startBox.Box.fHalfExtentX=startBox.Box.fHalfExtentY=startBox.Box.fHalfExtentZ=.5f;
 source->m_GuideCatalog.Prompts.push_back({startBox.PromptId,{{"Existing contact is not entry",1000u}}});source->m_GuideCatalog.Triggers.push_back(startBox);
 control(ownerId,1,GUIDE_CONTROL_ACTION::START);
 tests.Require(source->m_PersonalGuides.size()==1&&source->m_PersonalGuides.at(ownerId).PlayerId==guideId&&source->m_Players.size()==actorCount&&source->m_PartyMembersByPartyId.empty(),"Typed start binds the placed actor without a clone or internal party");
 if(source->m_PersonalGuides.empty())return 1;
 control(ownerId,2,GUIDE_CONTROL_ACTION::START);
 tests.Require(source->m_Players.size()==actorCount&&source->m_PersonalGuides.at(ownerId).PromptQueue.size()==1,"Duplicate start preserves the actor and one lowercase-category greeting");
 control(sessions[1]->Get_SessionId(),1,GUIDE_CONTROL_ACTION::START);
 control(sessions[1]->Get_SessionId(),2,GUIDE_CONTROL_ACTION::STOP);
 tests.Require(source->m_PersonalGuides.size()==1&&source->m_PersonalGuides.contains(ownerId),"Another human cannot take over or stop an occupied guide");
 tests.Require(lastState(sessions[1]).iOwnerNetEntityId==leader.iNetEntityId,"All Bern observers receive the occupied owner identity");
 source->Update_Guides(.2f);
 tests.Require(hasFrame(sessions[0],PACKET_TYPE::S2C_GUIDE_PROMPT)&&!hasFrame(sessions[1],PACKET_TYPE::S2C_GUIDE_PROMPT),"Prepared greeting is delivered only to the owner");
 const auto firstPrompt=std::find_if(sessions[0]->m_OutboundFrames.begin(),sessions[0]->m_OutboundFrames.end(),[](const auto& f){return f.ePacketType==PACKET_TYPE::S2C_GUIDE_PROMPT;});
 tests.Require(firstPrompt!=sessions[0]->m_OutboundFrames.end()&&std::any_of(sessions[0]->m_OutboundFrames.begin(),firstPrompt,[](const auto& f){return f.ePacketType==PACKET_TYPE::S2C_GUIDE_STATE;}),"Ownership state is queued before the first greeting");
 tests.Require(source->m_PersonalGuides.at(ownerId).InsideBoxes.contains(startBox.Id)&&
  std::none_of(source->m_PersonalGuides.at(ownerId).PromptQueue.begin(),source->m_PersonalGuides.at(ownerId).PromptQueue.end(),[&](const auto& queued){return queued.TriggerId==startBox.Id;}),
  "Starting guidance inside a space seeds contact and emits no invented entry prompt");
 source->m_GuideCatalog.Triggers.pop_back();source->m_GuideCatalog.Prompts.pop_back();source->m_PersonalGuides.at(ownerId).InsideBoxes.erase(startBox.Id);
 const auto firstEventSequence=source->m_iGuideEventSequence;
 drain();
 auto& state=source->m_PersonalGuides.at(ownerId);
 auto& guide=source->m_Players.at(guideId);
 const auto command=std::find_if(source->m_GuideCatalog.Commands.begin(),source->m_GuideCatalog.Commands.end(),[](const auto& c){return c.Enabled&&!c.Stop;});
 tests.Require(command!=source->m_GuideCatalog.Commands.end(),"Published help command is present");
 if(command!=source->m_GuideCatalog.Commands.end())
 {
  source->Guide_ChatCommand(source->m_Players.at(sessions[1]->Get_PlayerId()),command->Aliases.front());tests.Require(state.ComboId.empty(),"A non-owner cannot command the singleton guide");
  source->Guide_ChatCommand(leader,"prefix "+command->Aliases.front());tests.Require(state.ComboId.empty(),"A substring does not activate a help command");
  source->Guide_ChatCommand(leader,command->Aliases.front());tests.Require(state.ComboId==command->ComboId&&state.ComboStep==0,"Exact owner help command stages its published combo");
  source->Guide_ChatCommand(leader,command->Aliases.front());tests.Require(state.ComboStep==0&&state.PendingComboId.empty(),"Duplicate active combo does not restart or queue");
 }
 const auto* combo=source->m_GuideCatalog.Find_Combo(state.ComboId);
 if(combo&&!combo->Skills.empty())
 {
  SERVER_WORLD_ENTITY enemy;enemy.iNetEntityId=900001;enemy.eKind=WORLD_BOOTSTRAP_KIND::MONSTER;enemy.iCurrentHp=enemy.iMaximumHp=1000;enemy.fPositionX=guide.fPositionX;enemy.fPositionY=guide.fPositionY;enemy.fPositionZ=guide.fPositionZ+1;enemy.eAction=SERVER_ENTITY_ACTION::PATTERN_WINDUP;
  source->m_WorldEntities.push_back(enemy);source->Update_Guides(.2f);
  tests.Require(state.Action==3&&state.ComboStep==1&&guide.iCurrentSkillId==combo->Skills.front()&&guide.eAction==PLAYER_ACTION_STATE::SKILL,"Owner help executes a real skill through the common decision loop");
  source->m_WorldEntities.pop_back();
 }
 source->Reset_PlayerForDebugTeleport(guide);state.ComboId.clear();state.PendingComboId.clear();state.PromptQueue.clear();state.PromptRemaining=100.f;state.HoldElapsed=10.f;
 // Put only the owner collider in a tiny authored-style box, away from the guide.
 const auto ownerPose=std::array{leader.fPositionX,leader.fPositionY,leader.fPositionZ};
 GUIDE_TRIGGER box;box.Id="guide.contract.owner-box";box.Type="SPACE_ENTER";box.Category=source->m_GuideCatalog.Categories.at(WORLD_ID::BERN);box.PromptId=source->m_GuideCatalog.Prompts.front().Id;box.Box.fPositionX=guide.fPositionX+12.f;box.Box.fPositionY=guide.fPositionY+1.f;box.Box.fPositionZ=guide.fPositionZ;box.Box.fHalfExtentX=box.Box.fHalfExtentY=box.Box.fHalfExtentZ=.5f;
 source->m_GuideCatalog.Triggers.push_back(box);leader.fPositionX=box.Box.fPositionX;leader.fPositionZ=box.Box.fPositionZ;source->Update_Guides(.2f);
 tests.Require(state.InsideBoxes.contains(box.Id),"Bern SPACE_ENTER uses the human collider while the guide is outside");
 state.InsideBoxes.erase(box.Id);leader.fPositionX=guide.fPositionX;leader.fPositionZ=guide.fPositionZ;guide.fPositionX=box.Box.fPositionX;source->Update_Guides(.2f);
 tests.Require(!state.InsideBoxes.contains(box.Id),"Guide-only contact cannot trigger the owner's Bern space event");
 source->m_GuideCatalog.Triggers.pop_back();guide.fPositionX=reception.fPositionX;guide.fPositionY=reception.fPositionY;guide.fPositionZ=reception.fPositionZ;leader.fPositionX=ownerPose[0];leader.fPositionY=ownerPose[1];leader.fPositionZ=ownerPose[2];source->Reset_PlayerForDebugTeleport(guide);
 // Decode actual outgoing dialogue for location cancellation, ordering and arrival boundaries.
 {
  const auto savedCatalog=source->m_GuideCatalog;const auto savedState=state;const auto savedGuide=guide;const auto savedOwner=leader;const auto savedTick=source->m_iServerTick;
  source->m_GuideCatalog.Triggers.clear();state.PromptQueue.clear();state.TriggerTicks.clear();state.InsideBoxes.clear();state.ThinkElapsed=0.f;state.PromptRemaining=6.f;state.ReturningOnFoot=false;source->m_iServerTick=100u;
  GUIDE_TRIGGER local=box;local.Id="guide.contract.queued-space";local.PromptId="guide.contract.location";local.CooldownMs=30000;local.Priority=10;
  local.Box.fPositionX=leader.fPositionX+8.f;local.Box.fPositionY=leader.fPositionY+1.f;local.Box.fPositionZ=leader.fPositionZ;
  source->m_GuideCatalog.Prompts.push_back({local.PromptId,{{"Location",1000u}}});source->m_GuideCatalog.Triggers.push_back(local);
  auto step=[&](float seconds){++source->m_iServerTick;source->Update_Guides(seconds);};
  auto enter=[&](){leader.fPositionX=local.Box.fPositionX;leader.fPositionZ=local.Box.fPositionZ;};
  auto leave=[&](){leader.fPositionX=local.Box.fPositionX+3.f;leader.fPositionZ=local.Box.fPositionZ;};
  auto received=[&](){std::vector<S2C_GUIDE_PROMPT> messages;for(const auto& frame:sessions[0]->m_OutboundFrames)if(frame.ePacketType==PACKET_TYPE::S2C_GUIDE_PROMPT){S2C_GUIDE_PROMPT message;CPacketReader reader{std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES)};if(Read_Message(reader,message))messages.push_back(std::move(message));}return messages;};
  drain();leave();step(.2f);enter();step(.2f);
  tests.Require(state.PromptQueue.size()==1&&state.PromptQueue.front().TriggerId==local.Id&&!state.TriggerTicks.contains(local.Id),"A waiting location prompt retains its source without spending cooldown");
  leave();step(.2f);
  tests.Require(state.PromptQueue.empty()&&!state.TriggerTicks.contains(local.Id)&&received().empty(),"Leaving a location cancels only its unsaid prompt without sending stale dialogue");
  enter();step(.2f);
  tests.Require(state.PromptQueue.size()==1&&state.PromptQueue.front().PromptId==local.PromptId,"Re-entering a cancelled location can immediately reserve its dialogue again");
  state.PromptRemaining=0.f;step(.01f);auto messages=received();
  tests.Require(messages.size()==1&&messages.front().strPromptId==local.PromptId&&state.TriggerTicks.contains(local.Id)&&state.TriggerTicks.at(local.Id)==source->m_iServerTick,"The first actual location segment is delivered and starts cooldown");
  drain();leave();step(.2f);enter();step(.2f);
  tests.Require(state.PromptQueue.empty(),"An already spoken location still respects its re-entry cooldown");

  // Moving directly between two shops with shared text replaces the old provenance.
  state.PromptQueue.clear();state.PromptRemaining=6.f;state.TriggerTicks.clear();
  GUIDE_TRIGGER nextSpace=local;nextSpace.Id="guide.contract.shared-next";nextSpace.Box.fPositionX+=8.f;
  GUIDE_TRIGGER overlap=nextSpace;overlap.Id="guide.contract.shared-overlap";
  source->m_GuideCatalog.Triggers.push_back(nextSpace);source->m_GuideCatalog.Triggers.push_back(overlap);
  enter();source->Queue_GuidePrompt(ownerId,local);leader.fPositionX=nextSpace.Box.fPositionX;source->Queue_GuidePrompt(ownerId,nextSpace);
  tests.Require(state.PromptQueue.size()==1&&state.PromptQueue.front().TriggerId==nextSpace.Id&&!state.TriggerTicks.contains(local.Id),"Same-text entry at a new shop replaces an unsaid departed source before deduplication");
  source->Queue_GuidePrompt(ownerId,overlap);
  tests.Require(state.PromptQueue.size()==1&&state.PromptQueue.front().TriggerId==nextSpace.Id&&state.PromptQueue.front().SpaceTriggerIds.size()==2,"Overlapping current spaces retain both fired sources in only one dialogue");
  source->Seed_GuideSpaceEntries(ownerId,leader);state.PromptRemaining=0.f;step(.01f);messages=received();
  tests.Require(messages.size()==1&&messages.front().strPromptId==local.PromptId&&state.TriggerTicks.contains(nextSpace.Id)&&state.TriggerTicks.contains(overlap.Id)&&!state.TriggerTicks.contains(local.Id),"Shared text speaks once and starts cooldown for every valid source that actually fired");
  // Both spaces really fire, then only the chosen source is left before speaking.
  drain();state.PromptQueue.clear();state.PromptRemaining=6.f;state.TriggerTicks.clear();state.InsideBoxes.clear();
  nextSpace.Box.fHalfExtentX=2.f;nextSpace.Priority=40;overlap.Box.fHalfExtentX=2.f;overlap.Box.fPositionX=nextSpace.Box.fPositionX+3.f;
  source->m_GuideCatalog.Triggers[source->m_GuideCatalog.Triggers.size()-2]=nextSpace;source->m_GuideCatalog.Triggers.back()=overlap;
  GUIDE_TRIGGER unentered=overlap;unentered.Id="guide.contract.already-inside-shared";unentered.Priority=250;unentered.Box.fHalfExtentX=10.f;source->m_GuideCatalog.Triggers.push_back(unentered);
  leader.fPositionX=nextSpace.Box.fPositionX-4.f;source->Seed_GuideSpaceEntries(ownerId,leader);step(.2f);leader.fPositionX=nextSpace.Box.fPositionX+1.5f;step(.2f);
  tests.Require(state.PromptQueue.size()==1&&state.PromptQueue.front().TriggerId==nextSpace.Id&&state.PromptQueue.front().SpaceTriggerIds.size()==2,"Both actual overlap entry sources are retained without adopting a higher-priority unfired contact");
  leader.fPositionX=overlap.Box.fPositionX+1.5f;step(.2f);
  tests.Require(state.PromptQueue.size()==1&&state.PromptQueue.front().TriggerId==overlap.Id&&state.PromptQueue.front().Priority==overlap.Priority&&state.InsideBoxes.contains(overlap.Id),"Leaving the preferred source preserves the already-fired overlapping source without requiring re-entry");
  state.PromptRemaining=0.f;step(.01f);messages=received();
  tests.Require(messages.size()==1&&messages.front().strPromptId==local.PromptId&&state.TriggerTicks.contains(overlap.Id)&&!state.TriggerTicks.contains(nextSpace.Id),"The surviving shared source speaks once and the departed unsaid source spends no cooldown");
  source->m_GuideCatalog.Triggers.pop_back();source->m_GuideCatalog.Triggers.pop_back();source->m_GuideCatalog.Triggers.pop_back();drain();enter();source->Seed_GuideSpaceEntries(ownerId,leader);

  // An unrelated high-priority trigger sharing a prompt must not promote this occurrence.
  state.PromptQueue.clear();state.PromptRemaining=6.f;state.TriggerTicks.clear();
  GUIDE_TRIGGER dormant=local;dormant.Id="guide.contract.unfired-priority";dormant.Priority=250;dormant.Box.fPositionX+=50.f;
  GUIDE_TRIGGER greeting=local;greeting.Id="guide.contract.queue-greeting";greeting.Type="GUIDE_STARTED";greeting.PromptId="guide.contract.greeting";greeting.Priority=100;
  source->m_GuideCatalog.Prompts.push_back({greeting.PromptId,{{"Greeting",1000u}}});source->m_GuideCatalog.Triggers.push_back(dormant);source->m_GuideCatalog.Triggers.push_back(greeting);
  source->Queue_GuidePrompt(ownerId,local);source->Queue_GuidePrompt(ownerId,greeting);
  tests.Require(state.PromptQueue.size()==2&&state.PromptQueue.front().PromptId==greeting.PromptId&&state.PromptQueue.back().Priority==10,"Queue priority belongs to the fired trigger rather than every trigger sharing its prompt");
  state.PromptRemaining=0.f;leave();step(.01f);messages=received();
  tests.Require(messages.size()==1&&messages.front().strPromptId==greeting.PromptId&&state.PromptQueue.empty(),"Cancelling a stale space leaves its greeting deliverable in the same update");

  // Guide-only arrival preserves contact; real owner relocation into a new space still fires.
  drain();state.PromptQueue.clear();state.PromptRemaining=0.f;state.TriggerTicks.clear();enter();source->Seed_GuideSpaceEntries(ownerId,leader);source->Guide_AnchorArrived(leader);
  step(.2f);
  tests.Require(state.InsideBoxes.contains(local.Id)&&state.PromptQueue.empty()&&received().empty(),"Repeated guide arrival preserves owner contact without inventing another entry");
  leave();step(.2f);enter();source->Guide_AnchorArrived(leader,true);step(.2f);
  tests.Require(state.PromptQueue.size()==1&&state.PromptQueue.front().TriggerId==local.Id,"Committed local map travel from outside into a new space preserves its real entry event");
  guide.fPositionX=leader.fPositionX+2.f;guide.fPositionY=leader.fPositionY;guide.fPositionZ=leader.fPositionZ;step(.01f);messages=received();
  tests.Require(messages.size()==1&&messages.front().strPromptId==local.PromptId,"Actual local map travel into a shop delivers its location dialogue once");
  drain();state.PromptRemaining=0.f;state.TriggerTicks.clear();
  GUIDE_TRIGGER returned=greeting;returned.Id="guide.contract.return";returned.Type="RAID_RETURNED";returned.PatternId="VALTAN_ARENA";returned.PromptId="guide.contract.return-text";
  source->m_GuideCatalog.Prompts.push_back({returned.PromptId,{{"Returned",1000u}}});source->m_GuideCatalog.Triggers.push_back(returned);
  state.WaitingForOwner=true;state.InsideBoxes.clear();source->Resume_PersonalGuide(ownerId,WORLD_ID::VALTAN_ARENA);
  tests.Require(state.InsideBoxes.contains(local.Id)&&state.PromptQueue.size()==1&&state.PromptQueue.front().TriggerId==returned.Id,"World return seeds nearby spaces while reserving its genuine return dialogue");
  leave();guide.fPositionX=leader.fPositionX+2.f;guide.fPositionY=leader.fPositionY;guide.fPositionZ=leader.fPositionZ;step(.2f);messages=received();
  tests.Require(messages.size()==1&&messages.front().strPromptId==returned.PromptId,"Return dialogue is preserved after leaving the arrival space");

  // Once the first segment starts, finishing the authored sentence remains deterministic.
  drain();state.PromptQueue.clear();state.PromptRemaining=0.f;state.TriggerTicks.clear();state.ReturningOnFoot=false;
  GUIDE_TRIGGER multi=local;multi.Id="guide.contract.multisegment";multi.PromptId="guide.contract.multisegment-text";
  source->m_GuideCatalog.Prompts.push_back({multi.PromptId,{{"First segment",1000u},{"Second segment",1000u}}});source->m_GuideCatalog.Triggers.push_back(multi);
  enter();source->Seed_GuideSpaceEntries(ownerId,leader);source->Queue_GuidePrompt(ownerId,multi);step(.01f);const auto speechTick=source->m_iServerTick;
  leave();step(.2f);
  tests.Require(state.PromptQueue.size()==1&&state.PromptQueue.front().NextSegment==1,"Leaving a space does not discard a dialogue whose first segment already started");
  step(1.f);messages=received();
  tests.Require(messages.size()==2&&messages[0].strText=="First segment"&&messages[1].strText=="Second segment"&&state.TriggerTicks.contains(multi.Id)&&state.TriggerTicks.at(multi.Id)==speechTick,"Started multi-segment dialogue finishes in order without restarting its cooldown");
  source->m_GuideCatalog=savedCatalog;state=savedState;guide=savedGuide;leader=savedOwner;source->m_iServerTick=savedTick;drain();
 }
 // The admitted dragon and three-axis common executor remain unchanged.
 C2S_SET_VEHICLE_RIDING riding;riding.eWorldId=WORLD_ID::BERN;riding.iRequestSequence=100;riding.iVehicleId=ANCIENT_SEA_VEHICLE_ID;
 (void)source->Apply_SetVehicleRiding(leader,riding);source->Update_Guides(.2f);
 tests.Require(leader.iVehicleId==ANCIENT_SEA_VEHICLE_ID&&guide.iVehicleId==leader.iVehicleId,"Guide mounts the same admitted dragon as the owner");
 leader.eVehicleFlightPhase=guide.eVehicleFlightPhase=VEHICLE_FLIGHT_PHASE::FLYING;const float anchorY=leader.fPositionY;leader.fPositionY=guide.fPositionY+2;
 source->Update_Guides(.2f);tests.Require(state.Action==6&&guide.fVehicleFlightInputY>0&&guide.iLastMoveSequence>0,"Airborne guide emits admitted three-axis flight follow input");
 leader.fPositionY=anchorY;leader.eVehicleFlightPhase=guide.eVehicleFlightPhase=VEHICLE_FLIGHT_PHASE::GROUNDED;riding.iRequestSequence=200;riding.iVehicleId=INVALID_VEHICLE_ID;(void)source->Apply_SetVehicleRiding(leader,riding);source->Update_Guides(.2f);drain();
 // bShipDockValid is set only by the successful server boarding contract.
 const auto pierPose=std::array{guide.fPositionX,guide.fPositionY,guide.fPositionZ};
 leader.bShipDockValid=true;leader.iVehicleId=8200;leader.fPositionX+=100.f;guide.hasMoveGoal=true;guide.fMoveGoalX=guide.fPositionX+10.f;
 source->Update_Guides(.2f);source->Update_Players(.5f);
 tests.Require(state.WaitingForShip&&!guide.hasMoveGoal&&guide.iVehicleId!=8200&&guide.fPositionX==pierPose[0]&&guide.fPositionY==pierPose[1]&&guide.fPositionZ==pierPose[2],"Successful ship boarding holds the same guide at the pier without copying the ship");
 leader.bShipDockValid=false;leader.iVehicleId=INVALID_VEHICLE_ID;source->Update_Guides(.2f);
 tests.Require(!state.WaitingForShip&&state.ReturningOnFoot&&guide.fPositionX==pierPose[0],"Disembarking resumes an on-foot approach without relocating the guide");
 leader.fPositionX=ownerPose[0];leader.fPositionZ=ownerPose[2];source->Reset_PlayerForDebugTeleport(guide);state.ReturningOnFoot=false;
 // Local map travel follows the committed owner once; it is not a ship/world departure.
 {
  const auto savedOwner=leader,savedGuide=guide;const auto savedState=state;const auto savedTick=source->m_iServerTick;
  for(const std::uint16_t destination : {std::uint16_t{1u},std::uint16_t{2u},std::uint16_t{3u},WORLD_MAP_SHIP_TRAVEL_DESTINATION_ID})
  {
   source->Reset_PlayerForDebugTeleport(leader);source->Reset_PlayerForDebugTeleport(guide);
   const auto before=std::array{guide.fPositionX,guide.fPositionY,guide.fPositionZ};
   C2S_USE_SQUAREHOLE travel;travel.iClientSequence=1000u+destination;travel.iSquareHoleId=destination;
   source->Handle_UseSquareHole(ownerId,travel);
   const bool admitted=leader.eAction==PLAYER_ACTION_STATE::SQUAREHOLE_SONG;
   tests.Require(admitted&&guide.fPositionX==before[0]&&guide.fPositionY==before[1]&&guide.fPositionZ==before[2],
    "SquareHole and Set Sail preparation preserve the guide until the real song commits");
   if(!admitted)continue;
   const auto duration=(SQUAREHOLE_SONG_DURATION_MS*30u+999u)/1000u+(SQUAREHOLE_BLACKOUT_HOLD_MS*30u+999u)/1000u;
   source->m_iServerTick=leader.iActionStartTick+duration-1u;source->Update_Players(1.f/30.f);
   SERVER_NAV_POINT expected;const bool landed=source->Resolve_SquareHoleDestination(leader,destination,expected)&&
    std::hypot(leader.fPositionX-expected.x,leader.fPositionZ-expected.z)<.01f;
   tests.Require(landed&&guide.iNetEntityId==reception.iNetEntityId&&state.PlayerId==guideId&&state.AnchorId==leader.iPlayerId&&
    !state.WaitingForOwner&&!state.WaitingForShip&&!leader.bShipDockValid&&source->m_PendingWorldTransfers.empty()&&
    std::abs(guide.fPositionY-leader.fPositionY)<=2.f&&std::hypot(guide.fPositionX-leader.fPositionX,guide.fPositionZ-leader.fPositionZ)<=6.01f&&
    source->m_ServerCollisionSystem.Is_PlayerPositionClear(guide.fPositionX,guide.fPositionY,guide.fPositionZ,guide.iNetEntityId)&&
    source->m_ServerNavigation.Is_PointWalkableExact(guide.fPositionX,guide.fPositionZ,guide.fPositionY)&&
    source->m_ServerNavigation.Has_LineOfSight(leader.fPositionX,leader.fPositionZ,guide.fPositionX,guide.fPositionZ,leader.fPositionY),
    "Committed Bern map travel brings the same owned guide to a validated nearby landing before any ship boarding");
   drain();
  }
  const auto refusedOwner=std::array{leader.fPositionX,leader.fPositionZ},refusedGuide=std::array{guide.fPositionX,guide.fPositionZ};
  C2S_USE_SQUAREHOLE invalid;invalid.iClientSequence=70000u;invalid.iSquareHoleId=65534u;source->Handle_UseSquareHole(ownerId,invalid);
  tests.Require(leader.eAction==PLAYER_ACTION_STATE::NONE&&leader.fPositionX==refusedOwner[0]&&leader.fPositionZ==refusedOwner[1]&&
   guide.fPositionX==refusedGuide[0]&&guide.fPositionZ==refusedGuide[1],"Rejected map travel never relocates its owner or guide");
  leader=savedOwner;guide=savedGuide;state=savedState;source->m_iServerTick=savedTick;drain();
 }
 // Check every heading at the actual harbour destination, including offsets across nav seams.
 {
  const auto savedOwner=leader,savedGuide=guide;const auto savedState=state;
  SERVER_NAV_POINT harbour;const bool resolved=source->Resolve_SquareHoleDestination(leader,WORLD_MAP_SHIP_TRAVEL_DESTINATION_ID,harbour);
  bool connected=resolved;
  if(resolved)for(unsigned heading=0;heading<24;++heading)
  {
   leader.fPositionX=harbour.x;leader.fPositionY=harbour.y;leader.fPositionZ=harbour.z;leader.fYawDegrees=heading*15.f;
   source->Guide_AnchorArrived(leader,true);
   connected=connected&&!state.ReturningOnFoot&&guide.iNetEntityId==reception.iNetEntityId&&
    source->m_ServerNavigation.Is_PointWalkableExact(guide.fPositionX,guide.fPositionZ,guide.fPositionY)&&
    source->m_ServerNavigation.Has_LineOfSight(leader.fPositionX,leader.fPositionZ,guide.fPositionX,guide.fPositionZ,leader.fPositionY)&&
    source->m_ServerCollisionSystem.Is_PlayerPositionClear(guide.fPositionX,guide.fPositionY,guide.fPositionZ,guide.iNetEntityId);
  }
  tests.Require(connected,"All twenty-four harbour arrival headings keep the guide on collision-clear connected ground");
  leader=savedOwner;guide=savedGuide;state=savedState;
  const auto before=std::array{guide.fPositionX,guide.fPositionY,guide.fPositionZ};
  guide.hasMoveGoal=true;state.ComboId="guide.contract.preserve";
  leader.fPositionX=100000.f;leader.fPositionZ=100000.f;
  source->Guide_AnchorArrived(leader,true);
  tests.Require(guide.fPositionX==before[0]&&guide.fPositionY==before[1]&&guide.fPositionZ==before[2]&&
   guide.hasMoveGoal&&state.ComboId=="guide.contract.preserve",
   "A local arrival without navigation preserves the guide pose, active goal and pending state");
  leader=savedOwner;guide=savedGuide;state=savedState;
 }
 // Real published building triggers must relocate the singleton only on completed motion.
 {
  const auto savedOwner=leader,savedGuide=guide;const auto savedState=state;const auto savedTick=source->m_iServerTick;
  const auto savedTriggers=source->m_ServerTriggerSystem;
  for(const char* id:{"castle","castle.2","library","library.2"})
  {
   leader=savedOwner;guide=savedGuide;state=savedState;source->m_ServerTriggerSystem=savedTriggers;
   source->Reset_PlayerForDebugTeleport(leader);source->Reset_PlayerForDebugTeleport(guide);
   const auto* box=source->Find_Placement(id);
   tests.Require(box&&box->isEnabled&&box->TriggerActions.size()==1u&&
    box->TriggerActions.front().eKind==WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER,
    "Published castle/library entry and exit have their actual authored local travel");
   if(!box||box->TriggerActions.size()!=1u)continue;
   leader.fPositionX=box->fPositionX;leader.fPositionY=box->fPositionY;leader.fPositionZ=box->fPositionZ;
   std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;std::vector<SERVER_INTERACT_PROMPT_EDGE> prompts;
   source->m_ServerTriggerSystem.Evaluate_Entries(source->m_Players,++source->m_iServerTick,transfers,{},prompts);
   const auto before=std::array{guide.fPositionX,guide.fPositionY,guide.fPositionZ};
   const bool started=leader.TriggerMove.isActive&&leader.TriggerMove.strSourcePlacementId==id;
   source->Update_Players(.1f);
   tests.Require(started&&leader.TriggerMove.isActive&&guide.fPositionX==before[0]&&guide.fPositionY==before[1]&&guide.fPositionZ==before[2],
    "Building blackout hold never relocates the guide early");
   for(unsigned step=0;step<120&&leader.TriggerMove.isActive;++step){++source->m_iServerTick;source->Update_Players(1.f/30.f);}
   const auto& move=box->TriggerActions.front();
   tests.Require(started&&!leader.TriggerMove.isActive&&std::hypot(leader.fPositionX-move.fTargetX,leader.fPositionZ-move.fTargetZ)<.01f&&
    !state.ReturningOnFoot&&guide.iNetEntityId==reception.iNetEntityId&&
    source->m_ServerNavigation.Is_PointWalkableExact(guide.fPositionX,guide.fPositionZ,guide.fPositionY)&&
    source->m_ServerNavigation.Has_LineOfSight(leader.fPositionX,leader.fPositionZ,guide.fPositionX,guide.fPositionZ,leader.fPositionY)&&
    std::hypot(guide.fPositionX-leader.fPositionX,guide.fPositionZ-leader.fPositionZ)<=6.01f&&
    !state.WaitingForShip&&!state.WaitingForOwner&&transfers.empty(),
    "Completed castle/library entry and exit bring the existing guide onto connected local ground");
   drain();
  }
  leader=savedOwner;guide=savedGuide;state=savedState;source->m_iServerTick=savedTick;source->m_ServerTriggerSystem=savedTriggers;drain();
 }
 // Existing four-human parties still enter a raid; the guide is outside that transaction.
 for(unsigned i=1;i<4;++i){C2S_PARTY_INVITE invite;invite.iTargetNetEntityId=source->m_Players.at(sessions[i]->Get_PlayerId()).iNetEntityId;source->Handle_PartyInvite(ownerId,invite);C2S_PARTY_INVITE_RESPOND answer;answer.iFromNetEntityId=leader.iNetEntityId;answer.bAccepted=true;source->Handle_PartyInviteRespond(sessions[i]->Get_SessionId(),answer);drain();}
 tests.Require(source->m_PartyMembersByPartyId.size()==1&&source->m_PartyMembersByPartyId.begin()->second.size()==4,"The guide does not occupy or alter a four-human party");
 std::vector<SESSION_ID> batch;for(const auto& session:sessions)batch.push_back(session->Get_SessionId());
 PARTY_TRANSFER_RESULT result;std::string status;
 const auto waitingPose=std::array{guide.fPositionX,guide.fPositionY,guide.fPositionZ};
 const auto queued=state.PromptQueue;const auto nextEntity=source->m_iNextNetEntityId;
 sessions[1]->m_OutboundFrames.resize(CClientSession::MAX_OUTBOUND_FRAME_COUNT);
 const bool failed=!source->Transfer_PartyTo(*target,batch,result,status);
 tests.Require(failed&&result==PARTY_TRANSFER_RESULT::REJECTED_OUTBOUND_BUSY&&source->Count_HumanPlayers()==4&&target->Count_HumanPlayers()==0,"Actual outbound capacity rejection preserves the whole source party");
 tests.Require(!state.WaitingForOwner&&state.PromptQueue==queued&&guide.fPositionX==waitingPose[0]&&guide.iNetEntityId==reception.iNetEntityId&&source->m_iNextNetEntityId==nextEntity,"Failed transfer preserves guide owner, identity, pose and pending prompts");drain();
 const bool moved=source->Transfer_PartyTo(*target,batch,result,status);
 tests.Require(moved&&source->Count_HumanPlayers()==0&&source->m_Players.size()==1&&source->m_PersonalGuides.size()==1&&state.WaitingForOwner&&target->Count_HumanPlayers()==4&&target->m_PersonalGuides.empty(),"Four humans enter while the same single guide waits in Bern");
 if(!moved){std::cout<<"Guide transfer detail: "<<status<<'\n';return 1;}
 tests.Require(std::none_of(target->m_Players.begin(),target->m_Players.end(),[](const auto& p){return p.second.Is_Guide();}),"Successful raid entry creates no raid guide actor or fake participant");
 source->Update_Guides(10.f);source->Update_Players(1.f);
 tests.Require(guide.fPositionX==waitingPose[0]&&guide.fPositionY==waitingPose[1]&&guide.fPositionZ==waitingPose[2]&&!guide.isCombatReady&&state.ComboId.empty()&&state.PromptQueue.empty(),"Last human departure keeps an inactive guide at exactly its committed Bern pose");drain();
 const bool returned=target->Transfer_PartyTo(*source,{ownerId},result,status,"","npc.bern.beda.guide");
 tests.Require(returned,"A committed human raid return re-enters Bern");
 if(!returned){std::cout<<"Guide return detail: "<<status<<'\n';return 1;}
 auto& returnedGuide=source->m_Players.at(guideId);
 tests.Require(!state.WaitingForOwner&&state.ReturningOnFoot&&state.AnchorId==sessions[0]->Get_PlayerId()&&returnedGuide.iNetEntityId==reception.iNetEntityId&&returnedGuide.fPositionX==waitingPose[0]&&returnedGuide.fPositionZ==waitingPose[2],"Raid return rebinds the new human identity while preserving the guide identity and pose");
 const auto returnTrigger=std::find_if(source->m_GuideCatalog.Triggers.begin(),source->m_GuideCatalog.Triggers.end(),[](const auto& t){return t.Enabled&&t.Type=="RAID_RETURNED"&&t.PatternId=="VALTAN_ARENA";});
 tests.Require(returnTrigger!=source->m_GuideCatalog.Triggers.end()&&std::any_of(state.PromptQueue.begin(),state.PromptQueue.end(),[&](const auto& p){return p.PromptId==returnTrigger->PromptId;}),"Actual raid return queues the matching published lowercase-category prompt");
 const auto count=state.PromptQueue.size();source->Resume_PersonalGuide(ownerId,WORLD_ID::VALTAN_ARENA);tests.Require(state.PromptQueue.size()==count,"A repeated resume notification cannot duplicate the return prompt");
 auto& returnedOwner=source->m_Players.at(sessions[0]->Get_PlayerId());
 const auto returnOwnerPose=std::array{returnedOwner.fPositionX,returnedOwner.fPositionY,returnedOwner.fPositionZ};returnedOwner.hasMoveGoal=false;
 // Make the actual return path fall inside the ordinary follow hysteresis band.
 const float originalResumeDistance=source->m_GuideCatalog.ResumeDistance;
 source->m_GuideCatalog.ResumeDistance=std::hypot(returnedOwner.fPositionX-returnedGuide.fPositionX,returnedOwner.fPositionZ-returnedGuide.fPositionZ)+1.f;
 source->Update_Guides(source->m_GuideCatalog.RecoverDelay+1.f);
 source->m_GuideCatalog.ResumeDistance=originalResumeDistance;
 tests.Require(returnedGuide.fPositionX==waitingPose[0]&&returnedGuide.fPositionZ==waitingPose[2]&&state.ReturningOnFoot&&returnedGuide.hasMoveGoal,"Far return uses the existing nav move goal and never the recovery teleport");
 tests.Require(returnedOwner.fPositionX==returnOwnerPose[0]&&returnedOwner.fPositionY==returnOwnerPose[1]&&returnedOwner.fPositionZ==returnOwnerPose[2]&&!returnedOwner.hasMoveGoal,"Guide approach leaves the returning human's pose and move intent untouched");
 // Stopping discards all future occurrences and does not respawn the singleton.
 tests.Require(!hasFrame(sessions[0],PACKET_TYPE::S2C_GUIDE_PROMPT),"Return speech stays queued until the guide approaches its owner");
 const auto lastSequence=returnedGuide.iLastMoveSequence;control(ownerId,1,GUIDE_CONTROL_ACTION::STOP);
 tests.Require(source->m_PersonalGuides.empty()&&source->m_Players.contains(guideId)&&!returnedGuide.hasMoveGoal&&!returnedGuide.isCombatReady&&lastState(sessions[0]).iOwnerNetEntityId==0,"New-level STOP sequence 1 clears guidance and broadcasts idle while retaining the placed actor");
 source->Resume_PersonalGuide(ownerId,WORLD_ID::VALTAN_ARENA);tests.Require(source->m_PersonalGuides.empty(),"A stopped guide cannot resume from a stale world-return notification");
 control(ownerId,1,GUIDE_CONTROL_ACTION::START);tests.Require(source->m_PersonalGuides.empty(),"A stale START cannot undo a newer STOP");
 returnedOwner.fPositionX=returnedGuide.fPositionX+2.f;returnedOwner.fPositionY=returnedGuide.fPositionY;returnedOwner.fPositionZ=returnedGuide.fPositionZ;
 control(ownerId,2,GUIDE_CONTROL_ACTION::START);source->Update_Guides(.2f);
 tests.Require(source->m_PersonalGuides.size()==1&&returnedGuide.iNetEntityId==reception.iNetEntityId&&source->m_iGuideEventSequence>firstEventSequence&&source->m_PersonalGuides.at(ownerId).Sequence>=lastSequence,"Restart preserves actor identity, monotonic prompts and admitted internal command sequences");
 // Each authoritative return type selects only its own configured event, using the real Join commit.
 for(const auto world:{WORLD_ID::KAKULSAYDON_ARENA,WORLD_ID::MAHARAKA,WORLD_ID::COLOSSEUM}){
  source->Leave(ownerId,PLAYER_DESPAWN_REASON::LEVEL_CHANGED);drain();source->Handle_Register(sessions[0]);C2S_ENTER_WORLD enter;enter.eWorldId=WORLD_ID::BERN;enter.eCharacterClass=CHARACTER_CLASS_ID::DIMENSIONMASTER;enter.strNickName="GuideReturn";
  const bool admitted=source->Join(ownerId,enter,{}, {},INVALID_HONOR_TITLE_ID,{}, {},world);
  const char* name=world==WORLD_ID::KAKULSAYDON_ARENA?"KAKULSAYDON_ARENA":world==WORLD_ID::MAHARAKA?"MAHARAKA":"COLOSSEUM";
  const auto event=std::find_if(source->m_GuideCatalog.Triggers.begin(),source->m_GuideCatalog.Triggers.end(),[&](const auto& t){return t.Enabled&&(t.Type=="RAID_RETURNED"||t.Type=="WORLD_RETURNED")&&t.PatternId==name;});
  const auto& current=source->m_PersonalGuides.at(ownerId);
  tests.Require(admitted&&!current.WaitingForOwner&&event!=source->m_GuideCatalog.Triggers.end()&&std::any_of(current.PromptQueue.begin(),current.PromptQueue.end(),[&](const auto& p){return p.PromptId==event->PromptId;}),"Committed return resolves the matching Kouku, island or Colosseum prompt");
 }
 source->Leave(ownerId,PLAYER_DESPAWN_REASON::LEVEL_CHANGED);sessions[0]->Request_Close();source->Update_Guides(.2f);
 tests.Require(source->m_PersonalGuides.empty()&&source->m_Players.size()==1&&source->m_Players.at(guideId).iNetEntityId==reception.iNetEntityId,"Disconnect while away releases the owner but keeps exactly the same singleton guide");
 for(unsigned i=1;i<4;++i)target->Leave(sessions[i]->Get_SessionId(),PLAYER_DESPAWN_REASON::LEVEL_CHANGED);
 // Exercise the actual ServerApp dispatcher: guidance cannot rely on a hidden party
 // to obtain atomic destination admission and reliable initial-frame preparation.
 {
  auto app = std::make_unique<CServerApp>();
  app->m_SharedGameRooms.emplace(WORLD_ID::BERN, source);
  app->m_SharedGameRooms.emplace(WORLD_ID::VALTAN_ARENA, target);
  const SESSION_ID soloId = 99109u;
  auto session = std::make_shared<CClientSession>(soloId, INVALID_SOCKET,
   CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
  session->m_isSendRunning.store(true);
  sessions.push_back(session);
  source->Handle_Register(session);
  C2S_ENTER_WORLD enter{};
  enter.eWorldId = WORLD_ID::BERN;
  enter.eCharacterClass = CHARACTER_CLASS_ID::DIMENSIONMASTER;
  enter.strNickName = "GuideSoloTransaction";
  const bool admitted = source->Join(soloId, enter);
  tests.Require(admitted, "Solo guide transaction starts with a real Bern admission");
  if (!admitted) return 1;
  app->m_Sessions.emplace(soloId, session);
  CServerApp::SESSION_GAMEPLAY_BINDING binding{};
  binding.eWorldId = WORLD_ID::BERN;
  binding.pSimulation = source;
  app->m_GameplayBindingBySessionId.emplace(soloId, binding);
  auto& solo = source->m_Players.at(session->Get_PlayerId());
  const auto& placed = source->m_Players.at(guideId);
  const auto heldPose = std::array{placed.fPositionX, placed.fPositionY, placed.fPositionZ};
  solo.fPositionX = placed.fPositionX + 2.f;
  solo.fPositionY = placed.fPositionY;
  solo.fPositionZ = placed.fPositionZ;
  control(soloId, 1u, GUIDE_CONTROL_ACTION::START);
  auto unrelated = std::make_shared<CClientSession>(soloId, INVALID_SOCKET,
   CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
  tests.Require(source->Has_PersonalGuideOwner(session) && !source->Has_PersonalGuideOwner(unrelated) &&
   source->m_PartyMembersByPartyId.empty(), "Guide ownership matches the live session object without creating a solo party");
  if (!source->Has_PersonalGuideOwner(session)) return 1;
  const auto npc = std::find_if(source->m_WorldEntities.begin(), source->m_WorldEntities.end(),
   [](const auto& entity) { return entity.eKind == WORLD_BOOTSTRAP_KIND::NPC && entity.strPlacementId == "npc.bern.beda.guide"; });
  tests.Require(npc != source->m_WorldEntities.end(), "Solo fixture resolves the real raid entrance NPC");
  if (npc == source->m_WorldEntities.end()) return 1;
  solo.fPositionX = npc->fPositionX;
  solo.fPositionY = npc->fPositionY;
  solo.fPositionZ = npc->fPositionZ;
  C2S_CONFIRM_NPC_ENTRY confirm{};
  confirm.iRequestSequence = 81u;
  confirm.strNpcPlacementId = "npc.bern.beda.guide";
  source->Handle_ConfirmNpcEntry(soloId, confirm);
  SERVER_WORLD_TRANSFER_REQUEST entry{};
  const bool staged = source->Try_DequeueWorldTransfer(entry) && entry.iSessionId == soloId &&
   entry.eTargetWorldId == WORLD_ID::VALTAN_ARENA && entry.PartyBatchSessionIds.empty();
  tests.Require(staged, "Typed solo NPC entry remains one human and carries no fabricated party");
  if (!staged) return 1;
  drain();
  const auto entryPlayer = std::make_unique<SERVER_PLAYER>(solo);
  const auto entryPrompts = source->m_PersonalGuides.at(soloId).PromptQueue;
  const auto hasTransferFailure = [&](WORLD_ID destination, std::uint32_t sequence, PARTY_TRANSFER_RESULT reason)
  {
   for (const auto& frame : session->m_OutboundFrames)
   {
    if (frame.ePacketType != PACKET_TYPE::S2C_PARTY_TRANSFER_RESULT || frame.Bytes.size() < PACKET_HEADER_BYTES) continue;
    CPacketReader reader{std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES)};
    S2C_PARTY_TRANSFER_RESULT decoded{};
    if (Read_Message(reader, decoded) && reader.Get_RemainingSize() == 0u &&
     decoded.eTargetWorldId == destination && decoded.iRequestSequence == sequence && decoded.eResult == reason) return true;
   }
   return false;
  };
  const auto guidePreserved = [&](bool waiting)
  {
   const auto& actor = source->m_Players.at(guideId);
   const auto owner = source->m_PersonalGuides.find(soloId);
   return owner != source->m_PersonalGuides.end() && owner->second.WaitingForOwner == waiting &&
    actor.iNetEntityId == reception.iNetEntityId && actor.fPositionX == heldPose[0] &&
    actor.fPositionY == heldPose[1] && actor.fPositionZ == heldPose[2];
  };
  const auto playerPreserved = [&](const auto& room, const auto& destination, const SERVER_PLAYER& before)
  {
   const auto player = room->m_Players.find(before.iPlayerId);
   const auto& actualBinding = app->m_GameplayBindingBySessionId.at(soloId);
   return player != room->m_Players.end() && session->Get_PlayerId() == before.iPlayerId &&
    player->second.fPositionX == before.fPositionX && player->second.fPositionY == before.fPositionY &&
    player->second.fPositionZ == before.fPositionZ && player->second.iCurrentHp == before.iCurrentHp &&
    !session->Is_Closing() && actualBinding.eWorldId == room->Get_WorldId() && actualBinding.pSimulation == room &&
    !destination->m_PlayerIdBySessionId.contains(soloId) && !hasFrame(session, PACKET_TYPE::S2C_ENTER_ACCEPTED) &&
    room->m_PartyMembersByPartyId.empty() && destination->m_PartyMembersByPartyId.empty();
  };
  // Both failures pass through Handle_WorldTransfers, including its rejection policy.
  const auto targetNextEntity = target->m_iNextNetEntityId;
  target->m_iNextNetEntityId = INVALID_NET_ENTITY_ID;
  source->m_PendingWorldTransfers.push_back(entry);
  app->Handle_WorldTransfers(source);
  target->m_iNextNetEntityId = targetNextEntity;
  tests.Require(playerPreserved(source, target, *entryPlayer) && guidePreserved(false) &&
   source->m_PersonalGuides.at(soloId).PromptQueue == entryPrompts &&
   hasTransferFailure(WORLD_ID::VALTAN_ARENA, 81u, PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED),
   "Solo destination admission failure keeps Bern player, guide and connection and reports typed rejection");
  drain();
  session->m_OutboundFrames.resize(CClientSession::MAX_OUTBOUND_FRAME_COUNT);
  source->m_PendingWorldTransfers.push_back(entry);
  app->Handle_WorldTransfers(source);
  tests.Require(playerPreserved(source, target, *entryPlayer) && guidePreserved(false) &&
   source->m_PersonalGuides.at(soloId).PromptQueue == entryPrompts && source->m_PendingPartyTransferResults.contains(soloId),
   "Solo entry FIFO failure preserves source and guide and defers its rejection without disconnecting");
  drain();
  source->Flush_PartyTransferResults();
  tests.Require(hasTransferFailure(WORLD_ID::VALTAN_ARENA, 81u, PARTY_TRANSFER_RESULT::REJECTED_OUTBOUND_BUSY), "Deferred solo entry rejection decodes with the actual destination and sequence after FIFO drains");
  drain();
  CServerApp::SESSION_WORLD_TRANSFER_FAILURE transferFailure{};
  const bool entered = app->Transfer_SessionWorld(source, entry, transferFailure);
  tests.Require(entered && source->Count_HumanPlayers() == 0 && target->Count_HumanPlayers() == 1 &&
   guidePreserved(true) && source->Has_PersonalGuideOwner(session) &&
   source->m_PartyMembersByPartyId.empty() && target->m_PartyMembersByPartyId.empty() &&
   std::none_of(target->m_Players.begin(), target->m_Players.end(), [](const auto& value) { return value.second.Is_Guide(); }) &&
   app->m_GameplayBindingBySessionId.at(soloId).pSimulation == target,
   "Successful atomic solo entry leaves the same guide in Bern and creates no raid guide or hidden party");
  if (!entered) { std::cout << "Solo guide entry detail: " << transferFailure.strContext << '\n'; return 1; }
  drain();
  target->m_bValtanRaidCleared = true;
  C2S_RETURN_TO_BERN returnRequest{};
  returnRequest.iRequestSequence = 82u;
  target->Handle_ReturnToBern(soloId, returnRequest);
  SERVER_WORLD_TRANSFER_REQUEST returning{};
  const bool returnStaged = target->Try_DequeueWorldTransfer(returning) && returning.iSessionId == soloId &&
   returning.eTargetWorldId == WORLD_ID::BERN && returning.PartyBatchSessionIds.empty();
  tests.Require(returnStaged, "The actual solo raid-clear return retains its ordinary typed one-human request");
  if (!returnStaged) return 1;
  const auto raidPlayer = std::make_unique<SERVER_PLAYER>(target->m_Players.at(session->Get_PlayerId()));
  const auto bernNextEntity = source->m_iNextNetEntityId;
  source->m_iNextNetEntityId = INVALID_NET_ENTITY_ID;
  target->m_PendingWorldTransfers.push_back(returning);
  app->Handle_WorldTransfers(target);
  source->m_iNextNetEntityId = bernNextEntity;
  const bool returnAdmissionPlayer = playerPreserved(target, source, *raidPlayer);
  const bool returnAdmissionGuide = guidePreserved(true) && source->m_PersonalGuides.at(soloId).PromptQueue.empty();
  const bool returnAdmissionNotice = hasTransferFailure(WORLD_ID::BERN, 82u, PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED);
  if (!returnAdmissionPlayer || !returnAdmissionGuide || !returnAdmissionNotice)
   std::cout << "Solo return admission detail: player=" << returnAdmissionPlayer << " guide=" << returnAdmissionGuide
    << " notice=" << returnAdmissionNotice << " status=" << target->m_strStatus << '\n';
  tests.Require(returnAdmissionPlayer && returnAdmissionGuide && returnAdmissionNotice,
   "Solo Bern admission failure retains the raid player and waiting guide and decodes the actual Bern rejection");
  drain();
  session->m_OutboundFrames.resize(CClientSession::MAX_OUTBOUND_FRAME_COUNT);
  target->m_PendingWorldTransfers.push_back(returning);
  app->Handle_WorldTransfers(target);
  const bool returnFifoPlayer = playerPreserved(target, source, *raidPlayer);
  const bool returnFifoGuide = guidePreserved(true) && source->m_PersonalGuides.at(soloId).PromptQueue.empty();
  const bool returnFifoNotice = target->m_PendingPartyTransferResults.contains(soloId);
  if (!returnFifoPlayer || !returnFifoGuide || !returnFifoNotice)
   std::cout << "Solo return FIFO detail: player=" << returnFifoPlayer << " guide=" << returnFifoGuide
    << " pendingNotice=" << returnFifoNotice << " status=" << target->m_strStatus << '\n';
  tests.Require(returnFifoPlayer && returnFifoGuide && returnFifoNotice,
   "Solo return FIFO failure preserves the raid binding and waiting guide without closing the owner");
  drain();
  target->Flush_PartyTransferResults();
  tests.Require(hasTransferFailure(WORLD_ID::BERN, 82u, PARTY_TRANSFER_RESULT::REJECTED_OUTBOUND_BUSY), "Deferred solo return rejection decodes Bern and the return sequence after FIFO drains");
  drain();
  const bool returnedSolo = app->Transfer_SessionWorld(target, returning, transferFailure);
  tests.Require(returnedSolo && source->Count_HumanPlayers() == 1 && target->Count_HumanPlayers() == 0 &&
   guidePreserved(false) && source->m_PersonalGuides.at(soloId).AnchorId == session->Get_PlayerId() &&
   !source->m_PersonalGuides.at(soloId).PromptQueue.empty() &&
   source->m_PartyMembersByPartyId.empty() && target->m_PartyMembersByPartyId.empty() &&
   app->m_GameplayBindingBySessionId.at(soloId).pSimulation == source,
   "Committed solo return rebinds the original guide and queues its return speech without making a party");
  if (!returnedSolo) { std::cout << "Solo guide return detail: " << transferFailure.strContext << '\n'; return 1; }
  source->Leave(soloId, PLAYER_DESPAWN_REASON::DISCONNECTED);
 }

 // Use one real Kouku room for the human-only Mario admission boundaries.
 {
  auto mario=std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
  tests.Require(mario->Is_Ready(),"Guide Mario policy fixture loads the real product room");
  if(mario->Is_Ready())
  {
   auto& audition=mario->m_KoukuSaydonPatternAudition;
   audition.ePhase=CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
   audition.pProductGeneration=mario->m_GameplayCatalog.Get_ActiveGeneration();
   audition.PinnedGameplayRevision=mario->m_GameplayCatalog.Get_ActiveRevision();
   audition.iPinnedSourceRevision=CKoukuSaydonBrain::Resolve_ProductSourceRevision(*audition.pProductGeneration);
   std::string detail;
   const auto* phase=CKoukuSaydonBrain::Find_AnimationOnlyPattern(*audition.pProductGeneration,"KAKULSAYDON_G1_PATTERN_33",detail);
   const BOSS_PATTERN_MECHANIC_TRIGGER* formation=nullptr;
   if(phase)for(const auto& trigger:phase->MechanicTriggers)if(trigger.eKind==BOSS_PATTERN_MECHANIC_TRIGGER_KIND::MARIO_PHASE2_PLAYERS)formation=&trigger;
   tests.Require(formation!=nullptr,"Guide Mario fixture resolves the published phase-2 formation trigger");
   if(formation)
   {
    SERVER_WORLD_ENTITY boss;boss.iNetEntityId=900001;boss.iPatternSequence=7;boss.iPatternStartTick=100;boss.strPatternId=phase->strPatternId;boss.iCurrentHp=10000;
    const auto makePlayer=[](PLAYER_ID id){SERVER_PLAYER p;p.iPlayerId=id;p.iNetEntityId=100+id;p.iSessionId=99000+id;p.iCurrentHp=p.iMaximumHp=100;p.isCombatReady=true;p.eCharacterClass=CHARACTER_CLASS_ID::DIMENSIONMASTER;p.fPositionX=20.f+id;p.fPositionY=1.32f;p.fPositionZ=950.f;return p;};
    for(unsigned humans:{2u,4u})
    {
     mario->m_Players.clear();mario->m_PersonalGuides.clear();mario->m_MarioLayoutRandom.seed(3);
     auto companion=makePlayer(1);companion.eControlKind=PLAYER_CONTROL_KIND::GUIDE_AI;companion.iSessionId=INVALID_SESSION_ID;mario->m_Players.emplace(1,companion);
     for(unsigned id=2;id<humans+2;++id){auto human=makePlayer(id);if(id==humans+1)human.iMarioStage=1;mario->m_Players.emplace(id,human);}
     const bool committed=mario->Commit_KoukuMarioPhasePlayers(boss,*formation,130);
     tests.Require(committed,"Mario phase-2 formation admits human participants beside a separate guide");
     const auto& after=mario->m_Players.at(1);unsigned humanBound=0;std::vector<float> humanSlots;
     for(const auto& [id,player]:mario->m_Players)if(player.Is_Human()){humanBound+=player.bPatternBound?1u:0u;if(!player.iMarioStage)humanSlots.push_back(player.fPositionX);}
     std::sort(humanSlots.begin(),humanSlots.end());bool exactSlots=humanSlots.size()==humans-1;
     for(std::size_t slot=0;slot<humanSlots.size();++slot)exactSlots=exactSlots&&std::abs(humanSlots[slot]-(formation->fTeleportX+float(slot)*1.25f))<.01f;
     tests.Require(exactSlots&&!after.bPatternBound&&!after.MarioReturnPosition&&humanBound==(humans>=3?1u:0u),"Guide consumes no Mario formation slot, captive target or living-participant threshold");
     if(humans==2)tests.Require(after.fPositionX==companion.fPositionX&&after.fPositionZ==companion.fPositionZ,"Two humans plus guide remain a duo without a prisoner or guide formation teleport");
     else tests.Require(after.fPositionX==companion.fPositionX&&after.fPositionZ==companion.fPositionZ,"Synthetic nonhuman control kind remains outside raid formation without guide runtime");
    }
    mario->m_Players.clear();mario->m_PersonalGuides.clear();
    auto companion=makePlayer(1);companion.eControlKind=PLAYER_CONTROL_KIND::GUIDE_AI;companion.iSessionId=INVALID_SESSION_ID;mario->m_Players.emplace(1,companion);
    auto entrant=makePlayer(2);entrant.eMadnessForm=PLAYER_MADNESS_FORM::CLOWN;mario->m_Players.emplace(2,entrant);
    CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_MEMBER member;member.strMemberId="guide.solo-mario";member.iBossEntityId=boss.iNetEntityId;member.iPatternSequence=boss.iPatternSequence;member.PatternIds.push_back(phase->strPatternId);member.MarioEntryAnchor=boss;member.iMarioEntryStartTick=100;member.iMarioEntryStage=1;member.strCompletionChainSuccessPatternId=phase->strPatternId;member.ePhase=CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
    audition.Members.clear();audition.Members.push_back(std::move(member));mario->m_PendingKoukuMarioEntries.push_back({"guide.solo-mario",2,100,1});
    mario->Commit_KoukuMarioEntries();
    const bool entered=!audition.Members.empty()&&audition.Members.front().bMarioEntryConsumed&&mario->m_Players.at(2).iMarioStage==1;
    tests.Require(entered&&audition.Members.front().bMarioSoloReturnRequired,"One human plus guide still requires the real Mario entrant to complete the solo return");
    tests.Require(mario->m_Players.at(1).iMarioStage==0&&!mario->m_Players.at(1).MarioReturnPosition,"Committing the human Mario entry leaves the guide outside the minigame");
    if(!entered)std::cout<<"Guide Mario admission detail: "<<mario->m_strStatus<<'\n';
   }
  }
 }
 std::cout<<"guide AI failures: "<<tests.failures<<'\n';return tests.failures?1:0;
}
```
