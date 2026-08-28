# 아이템 획득 알림 + 실제 인벤토리/카테고리 시스템 RESULT

## 요청 요약

발탄 처치 시 6개 장비(운명의 업화 세트)를 방에 있는 모든 플레이어에게 실제로 지급하고, 지급 순간
`announce_i3e3.dds` 기반 알림창을 아이템마다 순서대로 띄운다. 지급된 아이템은 실제 인벤토리에
반영되고, 강화창 좌측 리스트와 인벤토리창의 카테고리(전체/전투/소지) 버튼이 실제 데이터로 동작한다.

## 완료된 범위

### Data
- `Data/Items/ItemCatalog.json`: 기존 7개 항목에 `category`(`use`) 추가, 신규 6개 장비 항목
  추가(`category: "combat"`, `MainApp.cpp`의 기존 `ITEM_UPGRADE_SLOTS` 이름/아이콘 그대로).
- 신규 `Data/Valtan/Valtan.clearrewards.json` — 알림 순서대로 6개 itemId.
- `Tools/GameplayPipeline/Publish-ItemCatalog.ps1`: `category`가 `combat`/`use` 중 하나인지
  검증하는 로직 추가. TSV bootstrap 포맷(Server로 가는 값)은 변경 없음 — displayName/iconPath와
  동일하게 Client 전용 필드.
- 신규 `Tools/ValtanPipeline/Publish-ValtanClearRewards.ps1` — `ItemCatalog.json`과
  cross-reference 검증 후 `Server/Bin/DataFiles/Valtan/ClearRewards.bootstrap` 생성.
  `Server.vcxproj` pre-build에 `Publish-ItemCatalog.ps1` 바로 다음 줄로 연결.

### Server
- 신규 `Server/Public/ValtanClearRewards.h` + `.cpp` (`CValtanClearRewards`) — `CItemCatalog`와
  동일한 로더 패턴으로 부트스트랩을 읽어 순서 있는 itemId 벡터 노출. `CGameRoom`에
  `m_ValtanClearRewards` 멤버로 소유, 다른 catalog들과 같은 지점에서 `Load()`.
- `GameRoom.cpp`의 `Handle_DebugGiveItem` 안 지급 로직을 `Grant_Item(SERVER_PLAYER&, itemId,
  quantity)` 헬퍼로 분리 — 기존 디버그 지급과 신규 루팅 경로가 공유.
- `Server/Public/ServerWorldEntity.h`에 `bLootGranted` 플래그 추가(BOSS는 DEAD 후에도 despawn
  안 하므로 매 틱 재지급 방지용 latch). 월드 엔티티 틱 루프(despawn 체크 루프 직전)에 분기 추가:
  `BOSS`이고 `DEAD`이고 `!bLootGranted`이면 그 순간 `m_Players`의 전원에게 6개 아이템을
  `Grant_Item` 후 각자에게 인벤토리 스냅샷 재전송. Area 재입장(`Reset_ValtanArenaWhenEmpty` →
  `Initialize_WorldEntities`)은 엔티티를 새로 만들므로 플래그가 자연히 초기화됨.

### Client
- `Client/Public/ItemCatalog.h`/`.cpp`: `ITEM_DEFINITION`에 `strCategory` 추가 + 파싱/검증.
- `Client/Public/InventoryView.h`/`.cpp`: `Render_Items()`에 실제 카테고리 필터 추가.
  All(또는 재클릭으로 해제된 상태)은 전체, Combat/Use는 `strCategory` 일치 항목만 필터링된
  index 목록(`filteredIndices`)을 통해 그림. 드래그앤드롭도 같은 간접 참조로 갱신.
- `Client/Private/MainApp.cpp`: 하드코딩 `ITEM_UPGRADE_SLOTS[6]` 배열 제거 →
  `BuildItemUpgradeSlots()`(매 호출 시 실제 인벤토리에서 `category=="combat"` 항목만 추림)로 교체.
  `m_iItemUpgradeLevels[6]` 고정 배열 → `unordered_map<string, int32_t> m_ItemUpgradeLevels` +
  `ItemUpgradeLevelRef(itemId)` 헬퍼(첫 조회 시 10으로 초기화)로 교체. 좌측 리스트/재련 단계
  사다리/중앙 현재·다음 레벨/성공·실패 상세 텍스트 전부 이 소스로 갱신, 선택 인덱스는 매번
  `upgradeSlots.size()`로 clamp.
- 신규 `Client/Bin/Resources/UI/ItemAnnounce/ItemAnnounce_Frame.png` — 실제
  `announce_i3e3.dds`(2048×256)에서 프레임+아이콘 자리+텍스트 배경 구간만 크롭(1140×177,
  x:[0,1140] y:[8,185]). 오른쪽 완전 투명 페이드 구간(x>1152)은 버림.
- 신규 `Data/UI/ItemAnnounce/ItemAnnounce_Layout.json` — `ItemAnnounce_Frame`(배경),
  `ItemAnnounce_Icon`(런타임 `Set_SlotTexture`로 아이템별 아이콘 교체, placeholder는 lm_head),
  `ItemAnnounce_TextBox`(마커, 실제 텍스트는 `CMainApp`이 그림) 3슬롯. 화면 중앙(x:290~990,
  y:300~408.68)에 배치 — Tool에서 조정 가능한 추정 배치.
- `Client/Public/CombatHUDViewModel.h`: `HUD_ITEMANNOUNCE_TEXT_RECTS`(rect + 완성된
  "OO을(를) 획득하였습니다" 텍스트) 추가, RaidClear의 rect-only 패턴과 달리 텍스트 자체도 포함.
- `Client/Public/Level_ValtanArena.h`/`.cpp`: `m_pItemAnnounceView` +
  `Update_ItemAnnounce`/`Render_ItemAnnounce` 신규 — `RaidClear`와 같은 Level-owned
  `CHUDRuntimeView` 패턴. 매 프레임 `CCombatHUDViewModel::Get_Inventory()`를 이전에 관찰한
  itemId 집합과 대조해 새 itemId를 FIFO 큐에 적재, 한 번에 하나씩 2초 유지 후 다음으로 전환.
  진입 첫 프레임은 diff 없이 baseline만 캡처(이미 보유 중인 포션 3개가 "신규 획득"으로 오탐되지
  않도록). 을/를 조사는 마지막 음절 종성 유무(`(codepoint-0xAC00)%28`)로 실제 분기.
  사운드는 실제 확인된 `Sound/UI/System/sys_item_itemgetepic1__202768724.wav`
  (RaidClear의 `sys_raid_success1` 발견 당시와 같은 폴더에서 확인) 사용,
  `Client/Bin/Resources/Sound/UI/System/`에 복사.
- `CMainApp::RenderItemAnnounceText()` 신규 — `RenderRaidClearText()`와 같은 split
  (EndFrame 이후 텍스트 드로우), `Update` 호출부/`Render` 호출부 각각
  `Update_RaidClear`/`Render_RaidClear` 옆에 연결.
- `Client.vcxproj`/`.filters`: `ItemAnnounce_Layout.json`을 `96.DataFiles\UI` None 항목으로 등록.

## 검증

- `Tools/GameplayPipeline/Publish-ItemCatalog.ps1 -Mode Validate` → "13 items" 통과.
- `Tools/ValtanPipeline/Publish-ValtanClearRewards.ps1 -Mode Validate`/`Publish` → "6 items" 통과,
  `ClearRewards.bootstrap` 생성 확인.
- Server Debug 빌드: MSBuild 성공, pre-build의 두 publisher 모두 정상 실행(로그로 확인),
  `Server.exe` 생성 확인.
- Client Debug 빌드: MSBuild 성공(`Client.vcxproj -> Client.exe`), 컴파일 에러 없음. 기존
  LNK4099/C4819 경고는 이 변경과 무관한 기존 노이즈.

## 미검증 (사용자 전용 화면 검증 경계)

빌드 성공만 확인했고 실제 화면 동작은 사용자가 직접 확인해야 한다(`AGENTS.md`/`CLAUDE.md`의
Artist F·Effect 화면 검증 경계). 확인 경로:

1. Server + Client 실행 → Valtan 처치(또는 기존 O 키로 RaidClear만 테스트하는 것과 별개로 실제
   처치 필요 — ItemAnnounce는 `eAction==DEAD` 자체가 아니라 인벤토리 diff로 트리거됨).
2. 던전 클리어 화면과 겹치거나 이어서 아이템 알림 6개가 순서대로(2초 간격) 뜨는지, 아이콘/이름
   텍스트가 아이템마다 바뀌는지, "을/를" 조사가 맞는지.
3. `I`로 인벤토리 열어 6개 장비 + 기존 포션 3개가 실제로 들어있는지, All/전투/소지 버튼이 진짜
   필터링되는지.
4. `P`로 강화창 열어 좌측 리스트가 실제 보유 장비만(포션 아님) 표시하는지, 항목 클릭·재련
   시도까지 기존 흐름대로 동작하는지.

`ItemAnnounce_Frame`의 화면 내 크기/위치(현재 700×108.68, 화면 중앙)는 실제 배치 데이터가 없어
추정값이므로, 실제 확인 후 Tool에서 조정이 필요할 수 있다.
