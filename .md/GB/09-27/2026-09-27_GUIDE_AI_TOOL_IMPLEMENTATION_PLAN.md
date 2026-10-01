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
