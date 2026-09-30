# 콜로세움 입장창 3D 초상 + 베른 성 내부 NPC 진입 RESULT

작성: 2026-09-30. 상태: **소스·데이터 반영 완료, 빌드와 화면 확인은 사용자 몫**(이 작업에서 빌드·게임 실행·git 쓰기 없음).

## 1. 원인 확인

사용자의 가설은 절반만 맞았다. 로비 직접 입장에서는 커스터마이징 데이터(`CCharacterSelectionState::Get_ActiveAppearanceJson`)가 비어 있는 것이 맞지만, 3D를 못 띄운 직접 이유는 `CCharacterPortraitRenderer`가 **살아 있는 `CCharacter`** 를 그려야 하는데 콜로세움 로딩 단계에는 그 캐릭터가 없다는 점이다. 커마 적용 자체는 `ClientReplication.cpp`(~2366)에서 캐릭터 스폰 때 모든 레벨(캐릭터 선택 제외)에 이미 이뤄진다.

## 2. 3D 초상 스냅샷(1번 방식)

베른에서 콜로세움 전환이 확정된 프레임에, 아직 살아 있는 베른의 로컬 캐릭터(커마·장비 포함)를 한 번 오프스크린 타깃에 그려 두고, 그 SRV를 로딩 화면 좌측 주목 캐릭터 슬롯에 붙인다.

프레임 순서(전환 호출 순서 추적):

| 순서 | 시점 | 하는 일 |
|---|---|---|
| 1 | 프레임 N 시작 `Apply_LevelRequest` | 대기 요청 없음 |
| 2 | 프레임 N 베른 Update | `Pump_ServerApprovedWorldTransfer(BERN)`이 `S2C_ENTER_ACCEPTED(COLOSSEUM)`를 받아 `Request_Load(COLOSSEUM)`를 대기열에 넣음 |
| 3 | 프레임 N `CMainApp::Render` | `Render_ColosseumTransferPortrait()`가 `Peek_Pending`으로 "다음 목적지 = COLOSSEUM"을 보고, 살아 있는 로컬 캐릭터를 `CCharacterPortraitRenderer`로 그려 SRV를 `CLevelTransitionService`에 보관 |
| 4 | 프레임 N+1 시작 `Apply_LevelRequest` | 요청을 소비해 LOADING 진입. 베른 캐릭터는 정리되지만 타깃은 MainApp이 소유해 살아 있음 |
| 5 | LOADING `Ready_ColosseumMatchView` | `Get_TransferPortraitSRV()`가 있으면 `MatchLoading_TeamA_Portrait`에 `Set_SlotTextureSRV`, 없으면 기존 2D 직업 일러스트 |
| 6 | COLOSSEUM 진입 | 다음 Render에서 레벨이 BERN/LOADING이 아니므로 보관 SRV 해제 |

- 카메라: 거리 3.0 m, 눈 높이 1.05 m, 주시 높이 0.95 m, FOV 35°(캐릭터 정보창 기본값 3.2와 같은 계열에서 전신이 조금 크게 나오도록 3.0). 타깃 크기는 슬롯 `373.333×324`(1280×720 기준)를 현재 뷰포트로 환산. 값은 `MainApp.cpp`의 `Render_ColosseumTransferPortrait` 안 상수이며 화면을 보고 조정할 수 있다.
- 배경: 초상 타깃은 캐릭터 커버리지 알파를 가진 투명 배경이라 로딩 화면 패널 위에 그대로 얹힌다(별도 배경 처리 없음).
- 오른쪽 팀의 주목 캐릭터는 지금처럼 2D 샘플 그대로.
- 로딩 스레드에서 D3D/GameObject를 건드리지 않는다(캡처는 메인 스레드 Render 안).

### 2D로 대체되는 조건 (로그: `[Colosseum.MatchLoading] portrait=3d|2d ...`, OutputDebugString)
- 로비에서 콜로세움 직접 진입(Debug) — 베른을 거치지 않으므로 스냅샷 없음
- 베른에 로컬 캐릭터가 없음 / 뷰포트 크기 0
- 초상 렌더가 S_OK가 아님
- 전환 요청이 콜로세움이 아닌 경우(다른 레벨 로딩 화면은 변경 없음)

## 3. 베른 성 내부 NPC로 콜로세움 진입

### 3.1 NPC 특정 근거
- 스크린샷 `스크린샷 2026-09-30 173206.png`: 미니맵 라벨 `베른 성 (성 내부)`, 금장식 소파 위 흰 옷 NPC 한 명, 플레이어 `Test-38292`가 계단 아래.
- `Data/UI/Minimap/MinimapAreas.json`의 `베른 성 (성 내부)`: worldMinCm `[1760,-37644]`, worldMaxCm `[8928,-26380]` → 클라이언트 X 17.6~89.28, Z 263.8~376.44(`client Z = -y/100`).
- `Gameplay.world.json`의 NPC 53개 중 이 범위 안은 **정확히 1명**: `npc.bern.25184_1.2`, archetype `NPC_25001`, 위치 (53.08, 41.53, 283.85), 모델 `Character/NPC/Npc_25001/Npc_25001.wmodel`, idle `npc_idle_normal_1`. (`npc.bern.src.42` NPC_SRC_25016은 X 239.7로 다른 내부 미니맵 영역이라 제외.)
- 이 NPC의 기존 대사·역할·상점·behavior는 없음(`behavior: null`). `NPC_25001`은 Bern의 이 placement 하나만 사용하므로 catalog/presentation은 수정하지 않았다(다른 월드 영향 없음).

### 3.2 기존 경로 조사 결론
- Valtan/Kouku 입장은 `CRaidEntryPreviewView`(Bern이 소유) → 투표(`Request_RaidEntryPropose`)이고, 이전 단일 NPC 방식은 `C2S_CONFIRM_NPC_ENTRY`(NPC placement id) → `CGameRoom::Handle_ConfirmNpcEntry`가 근접 3 m·생존·행동 상태를 검증한 뒤 `SERVER_WORLD_TRANSFER_REQUEST`를 만든다. **이 기존 wire를 재사용**하므로 protocol은 126 그대로다.
- Bern의 우클릭 NPC 피킹(레이-구 1.5 m)→접근 걷기→반경 3 m 도달 시 창을 여는 기존 흐름(`Update_ValtanEntryInteraction`/`Advance_ValtanEntryWalk`)에 이 NPC를 추가했다.

### 3.3 동작
1. 성 내부에서 그 NPC를 우클릭 → 캐릭터가 NPC 쪽으로 걷는다.
2. NPC 3 m 안에 도달하면 작은 수락/거절 창(`BernValtanEntry_Layout.json` 재사용)이 뜬다. 제목 **"증명의 전장"**, 본문 **"콜로세움에 입장하시겠습니까?"**(원본 문자열 키를 못 찾아 한글 문구를 코드 상수로 둠. 데이터화는 하지 않음).
3. 수락 → `Request_ConfirmNpcEntry(seq, "npc.bern.25184_1.2")` 하나만 전송. 거절/Esc는 그냥 닫힘.
4. Server가 NPC 근접 3 m·생존·행동 없음을 재검증하고 BERN→COLOSSEUM 전환을 확정, Client는 `S2C_ENTER_ACCEPTED` 뒤 기존 pump가 `LEVEL::COLOSSEUM`으로 전환(3D 초상은 이 시점에 캡처).
5. 입장 조건(레벨/티켓 등)은 만들지 않았다.

### 3.4 파티 동작 (이번 범위 아님)
- 솔로가 기본. 파티(가이드 동행 포함, 멤버 2명 이상)로 확인하면 Server가 `PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED`를 회신하고 전원이 베른에 남는다(분리·동반 이동 없음). 파티 비리더는 기존대로 `REJECTED_NOT_LEADER`.
- 가이드를 초대한 상태로는 콜로세움에 못 들어간다. 필요하면 후속으로 가이드 제외 정책을 정해야 한다.

### 3.5 임시 트리거 `colosseum.entry`
NPC 경로로 대체되어 **제거**했다(revision 928, `Publish-WorldGameplay.ps1 -Mode Publish -WorldId BERN` 재게시, viewer/bootstrap에서 0건 확인). 서버 `Build_WorldTransfer`의 COLOSSEUM 허용·publisher/Client 문서 검증 허용은 나중에 트리거로 진입하는 경우를 위해 남겼다(동작에 영향 없음).

## 4. 변경 파일

| 파일 | 변경 |
|---|---|
| `Server/Private/GameRoom_Inventory.cpp` | 진입 NPC 표에 `npc.bern.25184_1.2 → COLOSSEUM`, 파티 거부 |
| `Server/Private/ServerTriggerSystem.cpp` | `Build_WorldTransfer`에 COLOSSEUM 허용(트리거 경로용) |
| `Client/Private/WorldGameplayDocument.cpp` | changeLevel 대상에 COLOSSEUM 허용 |
| `Tools/WorldPipeline/Publish-WorldGameplay.ps1` | changeLevel 대상 COLOSSEUM, Bern에서만 허용 |
| `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json` | revision 928(트리거는 추가 후 제거, 순증 변화 없음) |
| `Server/Bin/DataFiles/World/BERN.worldbootstrap`, `Client/Bin/DataFiles/World/LV_BER_BERNCASTLE.viewer.world.json`, `BERN.npcpresentation.json` | publisher 재게시 산출물 |
| `Client/Public/RaidEntryPreviewView.h`, `Client/Private/RaidEntryPreviewView.cpp` | `Open_SimpleConfirm`, `SIMPLE_ACCEPT` intent(투표 없는 수락/거절) |
| `Client/Public/Level_Bern.h`, `Client/Private/Level_Bern.cpp` | 성 내부 NPC를 우클릭 대상에 추가(가이드 뒤에 배치해 Debug O 키 미리보기 불변), 수락 시 `Request_ConfirmNpcEntry` |
| `Client/Public/LevelTransitionService.h`, `Client/Private/LevelTransitionService.cpp` | `Peek_Pending`, 전환 초상 SRV 보관 |
| `Client/Public/MainApp.h`, `Client/Private/MainApp.cpp` | `Render_ColosseumTransferPortrait`, `m_pTransferPortrait` |
| `Client/Private/Level_Loading.cpp` | 좌측 슬롯에 3D 초상 연결, 2D 대체 |

새 파일 없음 → `.vcxproj`/`.filters` 변경 없음. 인코딩·EOL은 파일별 원본 유지(백업 대비 CRLF/LF 수 증감이 패치 줄 수와 일치, BOM·U+FFFD 없음). `Data/Rendering/**` 변경 없음.

## 5. 검증

실행한 것:
- `cl /Zs`(C++20, `_DEBUG`) 8개 파일 rc=0: LevelTransitionService, WorldGameplayDocument, RaidEntryPreviewView, Level_Loading, Level_Bern, MainApp, ServerTriggerSystem, GameRoom_Inventory
- `Publish-WorldGameplay.ps1 -Mode Validate/Publish -WorldId BERN` 성공(69 placements), bootstrap에 `npc.bern.25184_1.2` 행 확인
- `Gameplay.world.json` JSON parse, `git diff --check` 문제 없음(ServerTriggerSystem.cpp의 기존 혼합 EOL 경고만)
- protocol 126 유지

실행하지 않은 것(미검증):
- 빌드, Server/Client 실행, 화면 확인, NetworkProtocolHarness(wire 변경 없음)

## 6. 미검증 위험
- 우클릭 접근 목표점(NPC에서 약 2.1 m, 캐릭터 쪽)이 금장식 옥좌/단상 위 이동 불가 셀이면 반경 3 m에 못 닿을 수 있음. 그러면 창이 안 열리므로 계단 위쪽에서 다시 우클릭해야 한다.
- 초상 카메라 값(3.0 m 등)이 실제 슬롯 비율에서 발·머리를 자를 수 있음. 캐릭터 체형에 따른 크기 차이도 있음.
- `Get_LocalCharacter`의 캐릭터가 전환 프레임에 숨김/사망 상태이면 그려지지 않을 수 있음(그 경우 2D로 대체).
- 초상 타깃 알파 합성 결과가 로딩 패널 위에서 어색할 수 있음(정보창과 같은 합성 방식 사용).
- 가이드 동행 중 입장 불가(3.4).

## 7. 확인 순서
1. VS를 닫은 상태에서 Product Build(Engine → Shared → Server → Client)
2. **Server 재시작**(worldbootstrap과 서버 코드가 바뀜) 후 Client 실행 → Bern 입장
3. 베른 성 내부(미니맵 `성 내부`)로 이동해 흰 옷 NPC를 우클릭 → 가까이 가면 "증명의 전장" 확인창
4. 수락 → 콜로세움 입장창(로딩)에 커마한 내 캐릭터가 3D로 표시되는지 확인. 로비에서 직접 입장(Debug)하면 기존 2D 일러스트인지도 확인
5. 파티 상태에서 수락하면 베른에 남는지, 다른 레벨(Valtan/Kouku/Maharaka) 진입 화면이 그대로인지 확인
