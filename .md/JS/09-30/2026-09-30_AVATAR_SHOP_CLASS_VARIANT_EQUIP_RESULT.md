# 2026-09-30 아바타 상점 구매 → 클래스 치환 지급 → 장착·복제 RESULT

PLAN: 같은 폴더 `2026-09-30_AVATAR_SHOP_CLASS_VARIANT_EQUIP_PLAN.md`. 브랜치 `feature/source-hair-masked-pass` 위 미커밋(09-29 모코코 자산·재질 행·program 905~915 포함).

## 구현 완료

| G | 상태 | 내용 |
|---|---|---|
| G01 | 완료 | `ItemCatalog.json` +180 아이템(template 30 `classVariants`, 클래스 아이템 150 `visualSetId`), 상점 `shop.bern.avatar.mokoko`(NPC `npc.bern.plaza.17`, 30줄 GOLD 10). publisher: optional `visualSetId`/`classVariants` 검증, `ITEMVARIANT` 행, 상점 줄 상한 40, bootstrap **v6**(432행) 게시 |
| G02 | 완료 | `EQUIPMENT_SLOT::AVATAR_HEAD/AVATAR_OUTFIT` + `Equipment_SlotKind`, `PLAYER_SNAPSHOT.strAvatarHeadItemId/strAvatarOutfitItemId`(writer/reader/validator), **protocol 122** |
| G03 | 완료 | Server `ItemCatalog` v6 파서(`ITEMVARIANT` → `ClassVariants`, template/variant 정합 검사), `Apply_BuyItems` 구매자 클래스 치환(변형 없는 클래스는 basket 거부), `Broadcast_WorldSnapshot`이 착용 아바타 ID 채움 |
| G04 | 완료 | Client `ITEM_DEFINITION.strVisualSetId/ClassVariants` 파싱, 인벤 우클릭 아바타 착용·착용 중 아바타는 가방에 남고 우클릭 해제(`Try_Consume_UnequipRequest`), `MainApp` 해제 송신, 상점 페이지(`Shop_PageBg_0/1` = 이전/다음, 제목줄 `n / m`) |
| G05 | 완료 | `CClientReplication`이 `CEquipmentPresentationCatalog/Service`를 소유하고 `Apply_PlayerSnapshot` 뒤 `Apply_AvatarPresentation`으로 본인·타인 캐릭터에 visualSet 적용. despawn·Reset_World·클래스 교체 시 적용 기록 초기화 |

## 실행한 검증

- `Publish-ItemCatalog.ps1 -Mode Validate`: 236 items, 150 class variants, 2 currencies, 2 shops. `-Mode Publish` → `Items.bootstrap` v6 432행.
- Product Debug 빌드 PASS: Shared OBJ 3 / Server OBJ 87 / Client OBJ 205, 오류 0.
- NetworkProtocolHarness: **1374 PASS, 실패 0**. 추가: World Snapshot 왕복에 아바타 ID 2개, `Protocol 122 carries the avatar equipment slots`, payload 크기 테스트에 `playerAvatarBytes` 반영. (121→122 일괄 치환 중 무관한 `iServerTick = 121u`, `Make_GameplayDataRevision(121u)` 두 리터럴은 원복.)
- Server `--contract-test`: 625 PASS 시점까지 인벤토리·상점 관련 실패 없음. `[FAILURE]` 3건은 Valtan Bind 슬롯·threshold 테스트(발탄 패턴 소유자 영역)로 이번 변경 파일과 무관하며 별도 확인 대상.
- 로컬 Server 시작: bootstrap v6 로드 후 `Listening on 127.0.0.1:7777` 확인. 주의: stdout 리다이렉트로 띄우면 `Press Enter to stop`이 EOF를 받아 즉시 종료된다 — 콘솔로 띄워야 한다.
- `git diff --check` clean.

## 미확인·남은 것

- **화면 확인 없음.** 사용자 판단: Debug 프레임이 낮아 Release 빌드로 확인 예정. Release는 `-Configuration Release` Product 빌드 후 같은 로컬 Server/Client 절차.
- 이 PC에는 상점창 프레임 이미지 `UI/Repair/{repair_window_bg,repair_top_deco,repair_cost_bar}.png`가 없다(TJ 09-27 수리창·09-28 상점창 작업분, Git 밖). 없으면 셀·글자만 보인다.
- 아이콘 30장 `UI/Items/Avatar/mokoko_036<var>_<head|outfit>.png` 미제작(UI 담당).
- 아바타 책 창은 옛 고정 파츠 방식 그대로(착용 경로는 인벤 우클릭).
- 아바타 상인 위치: 베른 광장 NPC 묶음 남쪽, X 131.7 / Z −75.2(`NPC_13203`). 물약 상인(plaza.19/.20)과 붙어 있어 헷갈리면 상점 NPC 배정을 바꿀 수 있다.
- 팀 문서(`TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` 아이템·상점 절) 갱신은 화면 확인 뒤.

## 변경 파일

Shared `PacketMessages.h/.cpp`, `PacketType.h` · Server `ItemCatalog.h/.cpp`, `GameRoom_Inventory.cpp`, `GameRoom_Replication.cpp` · Client `ItemCatalog.h/.cpp`, `InventoryView.h/.cpp`, `MainApp.cpp`, `ShopWindowView.h/.cpp`, `ClientReplication.h/.cpp` · `Tools/GameplayPipeline/Publish-ItemCatalog.ps1`, `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp` · `Data/Items/ItemCatalog.json`, `Server/Bin/DataFiles/Items/Items.bootstrap`.
