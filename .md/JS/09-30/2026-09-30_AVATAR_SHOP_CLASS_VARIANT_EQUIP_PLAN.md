# 2026-09-30 아바타 상점 구매 → 클래스 치환 지급 → 장착·복제 PLAN

작성자 JS · 브랜치 `feature/source-hair-masked-pass` 위 미커밋 상태에서 시작한다(모코코 150 visualSet·재질 행·program 905~915 포함).
규칙 파일 `.md/GB/local.md`를 따른다. 코드 사실의 정본은 현재 저장소이며 아래 앵커는 2026-09-30 실측이다.

## 0. 목표와 종료 증거

사용자 설계: 베른 상점 NPC가 아바타를 판다. 상점에는 "기분 좋은 모코코 머리" 아이콘 하나만 있고,
구매하면 인벤토리에 **구매자 클래스에 맞는** 아바타 아이템이 들어온다. 인벤토리에서 우클릭하면 착용되고,
착용한 모습은 **다른 플레이어에게도** 보인다. 상점은 **페이지 넘김**으로 10줄 초과를 처리한다.
아이콘은 클래스 중립 `UI/Items/Avatar/mokoko_036<var>_<head|outfit>.png`(UI 담당 제작).

| 계층 | 이번에 추가되는 계약 | 종료 증거 |
|---|---|---|
| Data | 클래스 중립 template 아이템 30개(`classVariants`), 클래스별 아이템 150개(`visualSetId`), 상점 `shop.bern.avatar.mokoko` 30줄 | `Publish-ItemCatalog.ps1 -Mode Validate/Publish`, bootstrap v7 |
| Shared | `EQUIPMENT_SLOT::AVATAR_HEAD/AVATAR_OUTFIT`, `PLAYER_SNAPSHOT.strAvatarHeadItemId/strAvatarOutfitItemId`, protocol 125 | NetworkProtocolHarness failures 0 |
| Server | bootstrap v7 `ITEMVARIANT` 행, 구매 시 template → 클래스 아이템 치환, 스냅샷에 착용 아바타 ID | Server contract test·실행 |
| Client | 우클릭 아바타 착용/해제, 상점 페이지, 스냅샷 착용 ID → visualSet 적용(본인·타인) | Debug 빌드, 로컬 Server+Client 착용 확인(사용자) |

G 순서: G01 Data/publisher → G02 Shared → G03 Server → G04 Client 데이터·UI → G05 Client 복제 표현 → G06 검증.
G02~G05는 protocol 125라 Server/Client를 함께 빌드·재시작한다.

## 1. 실측 요약(변경 근거)

- `Apply_BuyItems`(`Server/Private/GameRoom_Inventory.cpp:300-363`)는 `entry.strItemId`를 그대로 가방에 넣는다. 치환 지점은 `Find_ShopItem/Find_Item` 뒤, `bagEntry(...)` 앞.
- `Apply_SetEquipment`(204-260)는 `definition->strEquipSlot == Equipment_SlotKind(slot)`와 `Is_UsableByClass`를 검사한다. `EQUIPMENT_SLOT`(`Shared/Public/Network/PacketMessages.h:2825-2842`)에 아바타 슬롯이 없어 아바타는 장착이 거부된다. 슬롯과 kind 문자열만 추가하면 기존 검증이 그대로 동작한다.
- `PLAYER_SNAPSHOT`(1594-1743)에는 장비 정보가 없다. 타인에게 보이려면 스냅샷 필드가 필요하다. writer 꼬리는 `PacketMessages.cpp:3324`(`iHonorTitleId`), reader 꼬리는 3704, validator `Is_Valid_PlayerSnapshot`은 157행(`Is_Valid_ItemId`는 457행이라 전방 선언 필요).
- `Broadcast_WorldSnapshot`(`Server/Private/GameRoom_Replication.cpp:491`)의 `snapshot.iHonorTitleId = player.iHonorTitleId;`(634) 뒤가 착용 아바타 채우기 앵커.
- Client `CItemCatalog`는 `Data/Items/ItemCatalog.json`을 직접 읽는다(`Client/Private/ItemCatalog.cpp:52`). Server는 `Items.bootstrap` v5만 읽는다(`Server/Private/ItemCatalog.cpp:148`).
- 우클릭 착용은 `InventoryView.cpp:574-586`이 아바타를 제외하고, `MainApp.cpp:5014-5048`이 `Equipment_SlotKind`로 슬롯을 고른다 → kind 추가만으로 아바타 슬롯이 선택된다.
- 상점 창 `ShopWindowView.cpp`는 `CELL_COUNT=10`으로 `m_pShop->Items[iCell]`을 직접 인덱스한다(323,338,378,381,398,483,489). `ShopUI.json`에 `Shop_PageBg_0/1`(y 456, 228x52) 두 plate가 코드 참조 없이 있다 → 이전/다음 버튼으로 쓴다.
- 인게임에서 `CEquipmentPresentationService`를 소유한 곳이 없다(캐릭터 선택·F1 도구만). `CClientReplication`이 소유하고 `Apply_PlayerSnapshot`(`ClientReplication.cpp:4644-4647`)에서 적용한다. 클래스 교체(`Replace_CharacterClass` 4367-4372)는 새 `CCharacter`라 적용 기록을 지워 재적용한다.
- `EQUIPMENT_SLOT::END`로 배열 크기를 잡는 코드는 없다(`MainApp.cpp:5026`의 반복만) → enum 끝에 추가해도 안전.

## 2. G01 Data · publisher

### 2.1 `Data/Items/ItemCatalog.json`

**작업: 추가** — `items` 배열의 마지막 항목(`AVATAR_LANCEMASTER_MOKOKO_OUTFIT`, 59행) 바로 아래에 다음 180줄을 넣는다(앞 항목 끝의 `,`를 유지). 각 변형마다 template 1 + 클래스 5 순서이며, template은 `equipSlot`/`characterClass`가 없고 `classVariants`만 갖는다. 클래스 아이템은 `visualSetId`로 `EquipmentPresentationCatalog.json`의 visualSet을 가리킨다(150개 모두 09-29에 등록됨).

```json
    {"itemId": "AVATAR_MOKOKO_036_HEAD", "displayName": "강인한 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036_head.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036_HEAD", "Warlord": "AVATAR_WARLORD_MOKOKO_036_HEAD", "Artist": "AVATAR_ARTIST_MOKOKO_036_HEAD", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036_HEAD", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036_HEAD"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036_HEAD", "displayName": "강인한 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036.head"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036_HEAD", "displayName": "강인한 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036.head"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036_HEAD", "displayName": "강인한 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036.head"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036_HEAD", "displayName": "강인한 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036.head"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036_HEAD", "displayName": "강인한 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036.head"},
    {"itemId": "AVATAR_MOKOKO_036_OUTFIT", "displayName": "강인한 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036_outfit.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036_OUTFIT", "Warlord": "AVATAR_WARLORD_MOKOKO_036_OUTFIT", "Artist": "AVATAR_ARTIST_MOKOKO_036_OUTFIT", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036_OUTFIT", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036_OUTFIT"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036_OUTFIT", "displayName": "강인한 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036.outfit"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036_OUTFIT", "displayName": "강인한 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036.outfit"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036_OUTFIT", "displayName": "강인한 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036.outfit"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036_OUTFIT", "displayName": "강인한 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036.outfit"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036_OUTFIT", "displayName": "강인한 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036.outfit"},
    {"itemId": "AVATAR_MOKOKO_036-1_HEAD", "displayName": "기분 좋은 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-1_head.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036-1_HEAD", "Warlord": "AVATAR_WARLORD_MOKOKO_036-1_HEAD", "Artist": "AVATAR_ARTIST_MOKOKO_036-1_HEAD", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036-1_HEAD", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-1_HEAD"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036-1_HEAD", "displayName": "기분 좋은 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036-1.head"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036-1_HEAD", "displayName": "기분 좋은 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036-1.head"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036-1_HEAD", "displayName": "기분 좋은 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036-1.head"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036-1_HEAD", "displayName": "기분 좋은 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036-1.head"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-1_HEAD", "displayName": "기분 좋은 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036-1.head"},
    {"itemId": "AVATAR_MOKOKO_036-1_OUTFIT", "displayName": "기분 좋은 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-1_outfit.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036-1_OUTFIT", "Warlord": "AVATAR_WARLORD_MOKOKO_036-1_OUTFIT", "Artist": "AVATAR_ARTIST_MOKOKO_036-1_OUTFIT", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036-1_OUTFIT", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-1_OUTFIT"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036-1_OUTFIT", "displayName": "기분 좋은 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036-1.outfit"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036-1_OUTFIT", "displayName": "기분 좋은 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036-1.outfit"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036-1_OUTFIT", "displayName": "기분 좋은 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036-1.outfit"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036-1_OUTFIT", "displayName": "기분 좋은 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036-1.outfit"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-1_OUTFIT", "displayName": "기분 좋은 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036-1.outfit"},
    {"itemId": "AVATAR_MOKOKO_036-2_HEAD", "displayName": "늘푸른 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-2_head.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036-2_HEAD", "Warlord": "AVATAR_WARLORD_MOKOKO_036-2_HEAD", "Artist": "AVATAR_ARTIST_MOKOKO_036-2_HEAD", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036-2_HEAD", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-2_HEAD"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036-2_HEAD", "displayName": "늘푸른 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036-2.head"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036-2_HEAD", "displayName": "늘푸른 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036-2.head"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036-2_HEAD", "displayName": "늘푸른 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036-2.head"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036-2_HEAD", "displayName": "늘푸른 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036-2.head"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-2_HEAD", "displayName": "늘푸른 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036-2.head"},
    {"itemId": "AVATAR_MOKOKO_036-2_OUTFIT", "displayName": "늘푸른 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-2_outfit.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036-2_OUTFIT", "Warlord": "AVATAR_WARLORD_MOKOKO_036-2_OUTFIT", "Artist": "AVATAR_ARTIST_MOKOKO_036-2_OUTFIT", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036-2_OUTFIT", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-2_OUTFIT"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036-2_OUTFIT", "displayName": "늘푸른 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036-2.outfit"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036-2_OUTFIT", "displayName": "늘푸른 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036-2.outfit"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036-2_OUTFIT", "displayName": "늘푸른 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036-2.outfit"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036-2_OUTFIT", "displayName": "늘푸른 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036-2.outfit"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-2_OUTFIT", "displayName": "늘푸른 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036-2.outfit"},
    {"itemId": "AVATAR_MOKOKO_036-3_HEAD", "displayName": "벚꽃 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-3_head.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036-3_HEAD", "Warlord": "AVATAR_WARLORD_MOKOKO_036-3_HEAD", "Artist": "AVATAR_ARTIST_MOKOKO_036-3_HEAD", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036-3_HEAD", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-3_HEAD"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036-3_HEAD", "displayName": "벚꽃 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-3_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036-3.head"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036-3_HEAD", "displayName": "벚꽃 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-3_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036-3.head"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036-3_HEAD", "displayName": "벚꽃 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-3_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036-3.head"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036-3_HEAD", "displayName": "벚꽃 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-3_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036-3.head"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-3_HEAD", "displayName": "벚꽃 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-3_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036-3.head"},
    {"itemId": "AVATAR_MOKOKO_036-3_OUTFIT", "displayName": "벚꽃 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-3_outfit.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036-3_OUTFIT", "Warlord": "AVATAR_WARLORD_MOKOKO_036-3_OUTFIT", "Artist": "AVATAR_ARTIST_MOKOKO_036-3_OUTFIT", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036-3_OUTFIT", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-3_OUTFIT"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036-3_OUTFIT", "displayName": "벚꽃 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-3_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036-3.outfit"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036-3_OUTFIT", "displayName": "벚꽃 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-3_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036-3.outfit"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036-3_OUTFIT", "displayName": "벚꽃 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-3_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036-3.outfit"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036-3_OUTFIT", "displayName": "벚꽃 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-3_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036-3.outfit"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-3_OUTFIT", "displayName": "벚꽃 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-3_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036-3.outfit"},
    {"itemId": "AVATAR_MOKOKO_036-4_HEAD", "displayName": "오로라 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-4_head.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036-4_HEAD", "Warlord": "AVATAR_WARLORD_MOKOKO_036-4_HEAD", "Artist": "AVATAR_ARTIST_MOKOKO_036-4_HEAD", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036-4_HEAD", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-4_HEAD"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036-4_HEAD", "displayName": "오로라 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-4_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036-4.head"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036-4_HEAD", "displayName": "오로라 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-4_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036-4.head"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036-4_HEAD", "displayName": "오로라 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-4_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036-4.head"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036-4_HEAD", "displayName": "오로라 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-4_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036-4.head"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-4_HEAD", "displayName": "오로라 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-4_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036-4.head"},
    {"itemId": "AVATAR_MOKOKO_036-4_OUTFIT", "displayName": "오로라 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-4_outfit.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036-4_OUTFIT", "Warlord": "AVATAR_WARLORD_MOKOKO_036-4_OUTFIT", "Artist": "AVATAR_ARTIST_MOKOKO_036-4_OUTFIT", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036-4_OUTFIT", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-4_OUTFIT"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036-4_OUTFIT", "displayName": "오로라 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-4_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036-4.outfit"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036-4_OUTFIT", "displayName": "오로라 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-4_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036-4.outfit"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036-4_OUTFIT", "displayName": "오로라 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-4_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036-4.outfit"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036-4_OUTFIT", "displayName": "오로라 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-4_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036-4.outfit"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-4_OUTFIT", "displayName": "오로라 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-4_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036-4.outfit"},
    {"itemId": "AVATAR_MOKOKO_036-5_HEAD", "displayName": "레인보우 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-5_head.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036-5_HEAD", "Warlord": "AVATAR_WARLORD_MOKOKO_036-5_HEAD", "Artist": "AVATAR_ARTIST_MOKOKO_036-5_HEAD", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036-5_HEAD", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-5_HEAD"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036-5_HEAD", "displayName": "레인보우 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-5_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036-5.head"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036-5_HEAD", "displayName": "레인보우 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-5_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036-5.head"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036-5_HEAD", "displayName": "레인보우 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-5_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036-5.head"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036-5_HEAD", "displayName": "레인보우 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-5_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036-5.head"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-5_HEAD", "displayName": "레인보우 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-5_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036-5.head"},
    {"itemId": "AVATAR_MOKOKO_036-5_OUTFIT", "displayName": "레인보우 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-5_outfit.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036-5_OUTFIT", "Warlord": "AVATAR_WARLORD_MOKOKO_036-5_OUTFIT", "Artist": "AVATAR_ARTIST_MOKOKO_036-5_OUTFIT", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036-5_OUTFIT", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-5_OUTFIT"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036-5_OUTFIT", "displayName": "레인보우 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-5_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036-5.outfit"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036-5_OUTFIT", "displayName": "레인보우 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-5_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036-5.outfit"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036-5_OUTFIT", "displayName": "레인보우 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-5_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036-5.outfit"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036-5_OUTFIT", "displayName": "레인보우 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-5_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036-5.outfit"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036-5_OUTFIT", "displayName": "레인보우 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036-5_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036-5.outfit"},
    {"itemId": "AVATAR_MOKOKO_036A_HEAD", "displayName": "여름밤 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a_head.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036A_HEAD", "Warlord": "AVATAR_WARLORD_MOKOKO_036A_HEAD", "Artist": "AVATAR_ARTIST_MOKOKO_036A_HEAD", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036A_HEAD", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036A_HEAD"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036A_HEAD", "displayName": "여름밤 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036a.head"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036A_HEAD", "displayName": "여름밤 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036a.head"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036A_HEAD", "displayName": "여름밤 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036a.head"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036A_HEAD", "displayName": "여름밤 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036a.head"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036A_HEAD", "displayName": "여름밤 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036a.head"},
    {"itemId": "AVATAR_MOKOKO_036A_OUTFIT", "displayName": "여름밤 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a_outfit.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036A_OUTFIT", "Warlord": "AVATAR_WARLORD_MOKOKO_036A_OUTFIT", "Artist": "AVATAR_ARTIST_MOKOKO_036A_OUTFIT", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036A_OUTFIT", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036A_OUTFIT"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036A_OUTFIT", "displayName": "여름밤 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036a.outfit"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036A_OUTFIT", "displayName": "여름밤 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036a.outfit"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036A_OUTFIT", "displayName": "여름밤 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036a.outfit"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036A_OUTFIT", "displayName": "여름밤 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036a.outfit"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036A_OUTFIT", "displayName": "여름밤 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036a.outfit"},
    {"itemId": "AVATAR_MOKOKO_036A-1_HEAD", "displayName": "꿈꾸는 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-1_head.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036A-1_HEAD", "Warlord": "AVATAR_WARLORD_MOKOKO_036A-1_HEAD", "Artist": "AVATAR_ARTIST_MOKOKO_036A-1_HEAD", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036A-1_HEAD", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036A-1_HEAD"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036A-1_HEAD", "displayName": "꿈꾸는 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036a-1.head"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036A-1_HEAD", "displayName": "꿈꾸는 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036a-1.head"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036A-1_HEAD", "displayName": "꿈꾸는 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036a-1.head"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036A-1_HEAD", "displayName": "꿈꾸는 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036a-1.head"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036A-1_HEAD", "displayName": "꿈꾸는 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036a-1.head"},
    {"itemId": "AVATAR_MOKOKO_036A-1_OUTFIT", "displayName": "꿈꾸는 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-1_outfit.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036A-1_OUTFIT", "Warlord": "AVATAR_WARLORD_MOKOKO_036A-1_OUTFIT", "Artist": "AVATAR_ARTIST_MOKOKO_036A-1_OUTFIT", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036A-1_OUTFIT", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036A-1_OUTFIT"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036A-1_OUTFIT", "displayName": "꿈꾸는 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036a-1.outfit"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036A-1_OUTFIT", "displayName": "꿈꾸는 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036a-1.outfit"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036A-1_OUTFIT", "displayName": "꿈꾸는 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036a-1.outfit"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036A-1_OUTFIT", "displayName": "꿈꾸는 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036a-1.outfit"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036A-1_OUTFIT", "displayName": "꿈꾸는 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036a-1.outfit"},
    {"itemId": "AVATAR_MOKOKO_036A-2_HEAD", "displayName": "해질녘 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-2_head.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036A-2_HEAD", "Warlord": "AVATAR_WARLORD_MOKOKO_036A-2_HEAD", "Artist": "AVATAR_ARTIST_MOKOKO_036A-2_HEAD", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036A-2_HEAD", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036A-2_HEAD"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036A-2_HEAD", "displayName": "해질녘 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036a-2.head"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036A-2_HEAD", "displayName": "해질녘 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036a-2.head"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036A-2_HEAD", "displayName": "해질녘 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036a-2.head"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036A-2_HEAD", "displayName": "해질녘 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036a-2.head"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036A-2_HEAD", "displayName": "해질녘 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036a-2.head"},
    {"itemId": "AVATAR_MOKOKO_036A-2_OUTFIT", "displayName": "해질녘 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-2_outfit.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036A-2_OUTFIT", "Warlord": "AVATAR_WARLORD_MOKOKO_036A-2_OUTFIT", "Artist": "AVATAR_ARTIST_MOKOKO_036A-2_OUTFIT", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036A-2_OUTFIT", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036A-2_OUTFIT"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036A-2_OUTFIT", "displayName": "해질녘 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036a-2.outfit"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036A-2_OUTFIT", "displayName": "해질녘 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036a-2.outfit"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036A-2_OUTFIT", "displayName": "해질녘 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036a-2.outfit"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036A-2_OUTFIT", "displayName": "해질녘 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036a-2.outfit"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036A-2_OUTFIT", "displayName": "해질녘 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036a-2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036a-2.outfit"},
    {"itemId": "AVATAR_MOKOKO_036B_HEAD", "displayName": "눈꽃 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b_head.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036B_HEAD", "Warlord": "AVATAR_WARLORD_MOKOKO_036B_HEAD", "Artist": "AVATAR_ARTIST_MOKOKO_036B_HEAD", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036B_HEAD", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036B_HEAD"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036B_HEAD", "displayName": "눈꽃 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036b.head"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036B_HEAD", "displayName": "눈꽃 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036b.head"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036B_HEAD", "displayName": "눈꽃 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036b.head"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036B_HEAD", "displayName": "눈꽃 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036b.head"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036B_HEAD", "displayName": "눈꽃 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036b.head"},
    {"itemId": "AVATAR_MOKOKO_036B_OUTFIT", "displayName": "눈꽃 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b_outfit.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036B_OUTFIT", "Warlord": "AVATAR_WARLORD_MOKOKO_036B_OUTFIT", "Artist": "AVATAR_ARTIST_MOKOKO_036B_OUTFIT", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036B_OUTFIT", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036B_OUTFIT"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036B_OUTFIT", "displayName": "눈꽃 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036b.outfit"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036B_OUTFIT", "displayName": "눈꽃 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036b.outfit"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036B_OUTFIT", "displayName": "눈꽃 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036b.outfit"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036B_OUTFIT", "displayName": "눈꽃 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036b.outfit"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036B_OUTFIT", "displayName": "눈꽃 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036b.outfit"},
    {"itemId": "AVATAR_MOKOKO_036B-1_HEAD", "displayName": "하늘 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-1_head.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036B-1_HEAD", "Warlord": "AVATAR_WARLORD_MOKOKO_036B-1_HEAD", "Artist": "AVATAR_ARTIST_MOKOKO_036B-1_HEAD", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036B-1_HEAD", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036B-1_HEAD"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036B-1_HEAD", "displayName": "하늘 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036b-1.head"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036B-1_HEAD", "displayName": "하늘 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036b-1.head"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036B-1_HEAD", "displayName": "하늘 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036b-1.head"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036B-1_HEAD", "displayName": "하늘 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036b-1.head"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036B-1_HEAD", "displayName": "하늘 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036b-1.head"},
    {"itemId": "AVATAR_MOKOKO_036B-1_OUTFIT", "displayName": "하늘 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-1_outfit.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036B-1_OUTFIT", "Warlord": "AVATAR_WARLORD_MOKOKO_036B-1_OUTFIT", "Artist": "AVATAR_ARTIST_MOKOKO_036B-1_OUTFIT", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036B-1_OUTFIT", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036B-1_OUTFIT"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036B-1_OUTFIT", "displayName": "하늘 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036b-1.outfit"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036B-1_OUTFIT", "displayName": "하늘 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036b-1.outfit"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036B-1_OUTFIT", "displayName": "하늘 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036b-1.outfit"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036B-1_OUTFIT", "displayName": "하늘 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036b-1.outfit"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036B-1_OUTFIT", "displayName": "하늘 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036b-1.outfit"},
    {"itemId": "AVATAR_MOKOKO_036B-2_HEAD", "displayName": "별빛 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-2_head.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036B-2_HEAD", "Warlord": "AVATAR_WARLORD_MOKOKO_036B-2_HEAD", "Artist": "AVATAR_ARTIST_MOKOKO_036B-2_HEAD", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036B-2_HEAD", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036B-2_HEAD"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036B-2_HEAD", "displayName": "별빛 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036b-2.head"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036B-2_HEAD", "displayName": "별빛 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036b-2.head"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036B-2_HEAD", "displayName": "별빛 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036b-2.head"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036B-2_HEAD", "displayName": "별빛 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036b-2.head"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036B-2_HEAD", "displayName": "별빛 산타 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036b-2.head"},
    {"itemId": "AVATAR_MOKOKO_036B-2_OUTFIT", "displayName": "별빛 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-2_outfit.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036B-2_OUTFIT", "Warlord": "AVATAR_WARLORD_MOKOKO_036B-2_OUTFIT", "Artist": "AVATAR_ARTIST_MOKOKO_036B-2_OUTFIT", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036B-2_OUTFIT", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036B-2_OUTFIT"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036B-2_OUTFIT", "displayName": "별빛 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036b-2.outfit"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036B-2_OUTFIT", "displayName": "별빛 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036b-2.outfit"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036B-2_OUTFIT", "displayName": "별빛 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036b-2.outfit"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036B-2_OUTFIT", "displayName": "별빛 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036b-2.outfit"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036B-2_OUTFIT", "displayName": "별빛 산타 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036b-2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036b-2.outfit"},
    {"itemId": "AVATAR_MOKOKO_036C1_HEAD", "displayName": "아프로 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c1_head.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036C1_HEAD", "Warlord": "AVATAR_WARLORD_MOKOKO_036C1_HEAD", "Artist": "AVATAR_ARTIST_MOKOKO_036C1_HEAD", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036C1_HEAD", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036C1_HEAD"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036C1_HEAD", "displayName": "아프로 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036c1.head"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036C1_HEAD", "displayName": "아프로 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036c1.head"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036C1_HEAD", "displayName": "아프로 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036c1.head"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036C1_HEAD", "displayName": "아프로 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036c1.head"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036C1_HEAD", "displayName": "아프로 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c1_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036c1.head"},
    {"itemId": "AVATAR_MOKOKO_036C1_OUTFIT", "displayName": "아프로 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c1_outfit.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036C1_OUTFIT", "Warlord": "AVATAR_WARLORD_MOKOKO_036C1_OUTFIT", "Artist": "AVATAR_ARTIST_MOKOKO_036C1_OUTFIT", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036C1_OUTFIT", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036C1_OUTFIT"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036C1_OUTFIT", "displayName": "아프로 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036c1.outfit"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036C1_OUTFIT", "displayName": "아프로 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036c1.outfit"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036C1_OUTFIT", "displayName": "아프로 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036c1.outfit"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036C1_OUTFIT", "displayName": "아프로 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036c1.outfit"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036C1_OUTFIT", "displayName": "아프로 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c1_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036c1.outfit"},
    {"itemId": "AVATAR_MOKOKO_036C2_HEAD", "displayName": "썬그리 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c2_head.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036C2_HEAD", "Warlord": "AVATAR_WARLORD_MOKOKO_036C2_HEAD", "Artist": "AVATAR_ARTIST_MOKOKO_036C2_HEAD", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036C2_HEAD", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036C2_HEAD"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036C2_HEAD", "displayName": "썬그리 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036c2.head"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036C2_HEAD", "displayName": "썬그리 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036c2.head"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036C2_HEAD", "displayName": "썬그리 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036c2.head"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036C2_HEAD", "displayName": "썬그리 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036c2.head"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036C2_HEAD", "displayName": "썬그리 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c2_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036c2.head"},
    {"itemId": "AVATAR_MOKOKO_036C2_OUTFIT", "displayName": "썬그리 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c2_outfit.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036C2_OUTFIT", "Warlord": "AVATAR_WARLORD_MOKOKO_036C2_OUTFIT", "Artist": "AVATAR_ARTIST_MOKOKO_036C2_OUTFIT", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036C2_OUTFIT", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036C2_OUTFIT"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036C2_OUTFIT", "displayName": "썬그리 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036c2.outfit"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036C2_OUTFIT", "displayName": "썬그리 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036c2.outfit"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036C2_OUTFIT", "displayName": "썬그리 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036c2.outfit"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036C2_OUTFIT", "displayName": "썬그리 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036c2.outfit"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036C2_OUTFIT", "displayName": "썬그리 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c2_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036c2.outfit"},
    {"itemId": "AVATAR_MOKOKO_036C3_HEAD", "displayName": "스노쿨 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c3_head.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036C3_HEAD", "Warlord": "AVATAR_WARLORD_MOKOKO_036C3_HEAD", "Artist": "AVATAR_ARTIST_MOKOKO_036C3_HEAD", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036C3_HEAD", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036C3_HEAD"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036C3_HEAD", "displayName": "스노쿨 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c3_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036c3.head"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036C3_HEAD", "displayName": "스노쿨 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c3_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036c3.head"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036C3_HEAD", "displayName": "스노쿨 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c3_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036c3.head"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036C3_HEAD", "displayName": "스노쿨 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c3_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036c3.head"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036C3_HEAD", "displayName": "스노쿨 모코코 머리 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c3_head.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarHead", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036c3.head"},
    {"itemId": "AVATAR_MOKOKO_036C3_OUTFIT", "displayName": "스노쿨 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c3_outfit.png", "healPercent": 0, "category": "combat", "grade": "avatar", "classVariants": {"LanceMaster": "AVATAR_LANCEMASTER_MOKOKO_036C3_OUTFIT", "Warlord": "AVATAR_WARLORD_MOKOKO_036C3_OUTFIT", "Artist": "AVATAR_ARTIST_MOKOKO_036C3_OUTFIT", "DimensionMaster": "AVATAR_DIMENSIONMASTER_MOKOKO_036C3_OUTFIT", "GuardianKnight": "AVATAR_GUARDIANKNIGHT_MOKOKO_036C3_OUTFIT"}},
    {"itemId": "AVATAR_LANCEMASTER_MOKOKO_036C3_OUTFIT", "displayName": "스노쿨 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c3_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "LanceMaster", "grade": "avatar", "visualSetId": "character.lance_master.mokoko_av036c3.outfit"},
    {"itemId": "AVATAR_WARLORD_MOKOKO_036C3_OUTFIT", "displayName": "스노쿨 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c3_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Warlord", "grade": "avatar", "visualSetId": "character.warlord.mokoko_av036c3.outfit"},
    {"itemId": "AVATAR_ARTIST_MOKOKO_036C3_OUTFIT", "displayName": "스노쿨 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c3_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "Artist", "grade": "avatar", "visualSetId": "character.artist.mokoko_av036c3.outfit"},
    {"itemId": "AVATAR_DIMENSIONMASTER_MOKOKO_036C3_OUTFIT", "displayName": "스노쿨 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c3_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "DimensionMaster", "grade": "avatar", "visualSetId": "character.dimensionmaster.mokoko_av036c3.outfit"},
    {"itemId": "AVATAR_GUARDIANKNIGHT_MOKOKO_036C3_OUTFIT", "displayName": "스노쿨 모코코 의상 아바타", "maxStack": 1, "iconPath": "UI/Items/Avatar/mokoko_036c3_outfit.png", "healPercent": 0, "category": "combat", "equipSlot": "avatarOutfit", "characterClass": "GuardianKnight", "grade": "avatar", "visualSetId": "character.guardianknight.mokoko_av036c3.outfit"},
```

마지막 항목의 끝 `,`는 제거한다(배열 마지막). 기존 `AVATAR_LANCEMASTER_MOKOKO_HEAD/OUTFIT`(036-1 `mokoko_pleasant`)는 그대로 둔다.

**작업: 추가** — `shops` 배열의 `shop.bern.potion` 객체 닫는 `}` 바로 아래(`,` 추가 후):

```json
    {
      "shopId": "shop.bern.avatar.mokoko",
      "npcPlacementIds": [ "npc.bern.plaza.17" ],
      "items": [
          {"itemId": "AVATAR_MOKOKO_036_HEAD", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036_OUTFIT", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036-1_HEAD", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036-1_OUTFIT", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036-2_HEAD", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036-2_OUTFIT", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036-3_HEAD", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036-3_OUTFIT", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036-4_HEAD", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036-4_OUTFIT", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036-5_HEAD", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036-5_OUTFIT", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036A_HEAD", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036A_OUTFIT", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036A-1_HEAD", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036A-1_OUTFIT", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036A-2_HEAD", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036A-2_OUTFIT", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036B_HEAD", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036B_OUTFIT", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036B-1_HEAD", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036B-1_OUTFIT", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036B-2_HEAD", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036B-2_OUTFIT", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036C1_HEAD", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036C1_OUTFIT", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036C2_HEAD", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036C2_OUTFIT", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036C3_HEAD", "currencyId": "GOLD", "price": 10},
          {"itemId": "AVATAR_MOKOKO_036C3_OUTFIT", "currencyId": "GOLD", "price": 10}
      ]
    }
```

`npc.bern.plaza.17`은 `Data/Worlds/LV_BERN/Gameplay.world.json`(베른 Area)의 실제 NPC placement ID로 바꾼다. 상점을 여는 NPC는 `CShopWindowView::Open`이 `Find_ShopByNpc`로 찾으므로 placement ID가 정확해야 한다. 가격 10 GOLD는 시험값이다.

### 2.2 `Tools/GameplayPipeline/Publish-ItemCatalog.ps1` (전체 교체)

변경: (a) item optional에 `visualSetId`(Client 전용 문자열)·`classVariants`(template 전용) 추가, (b) template 검증 — `equipSlot`/`characterClass` 없음, 값은 같은 `characterClass`·비어 있지 않은 `equipSlot`을 가진 실재 아이템, 키는 7 클래스 이름 중 하나, (c) `ITEMVARIANT <template> <class> <item>` 행, (d) 상점 줄 상한 10 → 40, (e) header `7`.

```powershell
[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish', 'CheckPublished')]
    [string]$Mode = 'Validate',
    [string]$OutputRoot = 'Server/Bin/DataFiles/Items'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$stableIdPattern = '^[A-Za-z0-9_.-]{1,64}$'
. (Join-Path $PSScriptRoot 'Publish-FileTransaction.ps1')
$publishSources = @{}

function Read-JsonDocument([string]$RelativePath) {
    $path = [IO.Path]::GetFullPath((Join-Path $repoRoot $RelativePath))
    if (-not [IO.File]::Exists($path)) { throw "Missing item document: $RelativePath" }
    return Read-PublishJsonSnapshot $path $publishSources
}

function Assert-ExactProperties([object]$Value, [string[]]$Expected, [string]$Context) {
    $actual = @($Value.PSObject.Properties.Name | Sort-Object)
    $expectedSorted = @($Expected | Sort-Object)
    if (($actual -join "`n") -ne ($expectedSorted -join "`n")) {
        throw "$Context fields are invalid. expected=[$($expectedSorted -join ',')] actual=[$($actual -join ',')]"
    }
}

function Assert-JsonInteger([object]$Value, [string]$Context, [long]$Minimum, [long]$Maximum) {
    if (($Value -isnot [int]) -and ($Value -isnot [long]) -and
        ($Value -isnot [uint32]) -and ($Value -isnot [uint64])) {
        throw "$Context must be a JSON integer."
    }
    $number = [long]$Value
    if ($number -lt $Minimum -or $number -gt $Maximum) {
        throw "$Context integer is out of range: $number"
    }
}

function Assert-JsonString([object]$Value, [string]$Context) {
    if ($Value -isnot [string]) { throw "$Context must be a JSON string." }
}

# Required fields must all be present; optional ones may be absent. Nothing else is allowed.
function Assert-Properties([object]$Value, [string[]]$Required, [string[]]$Optional, [string]$Context) {
    $actual = @($Value.PSObject.Properties.Name)
    foreach ($name in $Required) {
        if ($actual -cnotcontains $name) { throw "$Context is missing field '$name'." }
    }
    foreach ($name in $actual) {
        if (($Required -cnotcontains $name) -and ($Optional -cnotcontains $name)) {
            throw "$Context has an unknown field '$name'."
        }
    }
}

$itemDocument = Read-JsonDocument 'Data/Items/ItemCatalog.json'
Assert-Properties $itemDocument @('schema', 'formatVersion', 'items') @('currencies', 'shops') 'item catalog document'
Assert-JsonString $itemDocument.schema 'item catalog schema'
Assert-JsonInteger $itemDocument.formatVersion 'item catalog formatVersion' 2 2
if ($itemDocument.schema -ne 'lostark.item-catalog' -or $itemDocument.formatVersion -ne 2) {
    throw 'Item catalog header is invalid.'
}

$items = @($itemDocument.items)
if ($items.Count -eq 0 -or $items.Count -gt 4096) {
    throw "Item catalog item count is out of range: $($items.Count)"
}

# The Server's CHARACTER_CLASS_ID names as ItemCatalog.json spells characterClass.
$classNames = @('LanceMaster', 'Gunslinger', 'Slayer', 'Artist', 'DimensionMaster', 'Warlord', 'GuardianKnight')

$itemIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$itemById = @{}
$itemRows = [Collections.Generic.List[string]]::new()
$startingSlots = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($item in $items) {
    # grade is a Client presentation field (inventory grade art). equipSlot and characterClass
    # also travel to the Server, which checks them on equip; startingEquippedSlot names the
    # equipment slot a fresh character already wears the item in. "-" marks an absent value.
    # visualSetId is Client-only (EquipmentPresentationCatalog visual set an equipped avatar
    # shows). classVariants marks a shop template: the Server hands the buyer the variant of
    # the buyer's class instead of the template itself.
    Assert-Properties $item @('itemId', 'displayName', 'maxStack', 'iconPath', 'healPercent', 'category') `
        @('equipSlot', 'characterClass', 'grade', 'startingEquippedSlot', 'visualSetId', 'classVariants') 'item'
    Assert-JsonString $item.itemId 'item itemId'
    Assert-JsonString $item.displayName 'item displayName'
    Assert-JsonInteger $item.maxStack 'item maxStack' 1 ([uint32]::MaxValue)
    Assert-JsonString $item.iconPath 'item iconPath'
    Assert-JsonInteger $item.healPercent 'item healPercent' 0 100
    Assert-JsonString $item.category 'item category'
    if ($item.category -ne 'combat' -and $item.category -ne 'use') {
        throw "item category must be 'combat' or 'use': $($item.itemId)"
    }
    foreach ($optional in @('equipSlot', 'characterClass', 'grade', 'startingEquippedSlot', 'visualSetId')) {
        if ($null -ne $item.PSObject.Properties[$optional]) {
            Assert-JsonString $item.$optional "item $optional"
        }
    }
    if ($null -ne $item.PSObject.Properties['equipSlot']) {
        $slots = @('weapon', 'helmet', 'shoulder', 'top', 'pants', 'gloves', 'necklace', 'earring', 'ring', 'stone', 'bracelet', 'avatarHead', 'avatarOutfit')
        if ($slots -cnotcontains $item.equipSlot) { throw "item equipSlot is unknown: $($item.itemId)" }
    }
    if ($null -ne $item.PSObject.Properties['characterClass']) {
        if ($classNames -cnotcontains $item.characterClass) { throw "item characterClass is unknown: $($item.itemId)" }
    }
    if ($null -ne $item.PSObject.Properties['grade']) {
        if (@('normal', 'rare', 'epic', 'legend', 'relic', 'ancient', 'avatar') -cnotcontains $item.grade) {
            throw "item grade is unknown: $($item.itemId)"
        }
    }
    if ($null -ne $item.PSObject.Properties['visualSetId']) {
        if ($null -eq $item.PSObject.Properties['equipSlot'] -or
            @('avatarHead', 'avatarOutfit') -cnotcontains $item.equipSlot -or
            [string]::IsNullOrWhiteSpace([string]$item.visualSetId)) {
            throw "item visualSetId belongs only to an avatarHead/avatarOutfit item: $($item.itemId)"
        }
    }
    if ($item.itemId -notmatch $stableIdPattern) {
        throw "item itemId is not a stable ID: '$($item.itemId)'"
    }
    if ([string]::IsNullOrWhiteSpace([string]$item.displayName) -or
        ([string]$item.displayName).Length -gt 64) {
        throw "item displayName is invalid: $($item.itemId)"
    }
    if (-not $itemIds.Add([string]$item.itemId)) {
        throw "Duplicate item ID: $($item.itemId)"
    }
    $itemById[[string]$item.itemId] = $item
    $equipSlotField = if ($null -ne $item.PSObject.Properties['equipSlot']) { [string]$item.equipSlot } else { '-' }
    $classField = if ($null -ne $item.PSObject.Properties['characterClass']) { [string]$item.characterClass } else { '-' }
    $startingField = '-'
    if ($null -ne $item.PSObject.Properties['startingEquippedSlot']) {
        # The slot must take the item's kind (earring1/earring2 take "earring", ring1/ring2 "ring"),
        # the item must fit every class, and no two items may start in one slot.
        $startingField = [string]$item.startingEquippedSlot
        $startingKind = $startingField -replace '[12]$', ''
        if ($equipSlotField -ceq '-' -or $startingKind -cne $equipSlotField -or
            @('earring', 'ring') -ccontains $startingField -or
            @('helmet', 'shoulder', 'top', 'pants', 'gloves', 'weapon', 'necklace', 'earring', 'ring', 'stone', 'bracelet') -cnotcontains $startingKind) {
            throw "item startingEquippedSlot does not fit its equipSlot: $($item.itemId)"
        }
        if ($classField -cne '-') { throw "item startingEquippedSlot must be class-free: $($item.itemId)" }
        if (-not $startingSlots.Add($startingField)) { throw "Two items start in slot $startingField" }
    }
    $itemRows.Add((@('ITEM', $item.itemId, [uint32]$item.maxStack, [uint32]$item.healPercent, $equipSlotField, $classField, $startingField) -join "`t"))
}

# Shop templates: a class-neutral item whose purchase gives the buyer the class variant.
# The template itself is never equippable (no equipSlot, no class); every variant is a
# real equippable item of exactly that class, with the same equipSlot for every class.
$variantRows = [Collections.Generic.List[string]]::new()
foreach ($item in $items) {
    if ($null -eq $item.PSObject.Properties['classVariants']) { continue }
    if ($null -ne $item.PSObject.Properties['equipSlot'] -or $null -ne $item.PSObject.Properties['characterClass'] -or
        $null -ne $item.PSObject.Properties['startingEquippedSlot'] -or $null -ne $item.PSObject.Properties['visualSetId']) {
        throw "item classVariants template must not be equipment itself: $($item.itemId)"
    }
    $variants = $item.classVariants
    if ($variants -isnot [PSCustomObject]) { throw "item classVariants must be an object: $($item.itemId)" }
    $classKeys = @($variants.PSObject.Properties.Name)
    if ($classKeys.Count -eq 0) { throw "item classVariants is empty: $($item.itemId)" }
    $sharedSlot = $null
    foreach ($className in $classKeys) {
        if ($classNames -cnotcontains $className) { throw "item classVariants names an unknown class '$className': $($item.itemId)" }
        $variantId = $variants.$className
        Assert-JsonString $variantId "item classVariants.$className"
        if (-not $itemById.ContainsKey([string]$variantId)) { throw "item classVariants names an unknown item '$variantId': $($item.itemId)" }
        $variant = $itemById[[string]$variantId]
        if ($null -eq $variant.PSObject.Properties['equipSlot'] -or $null -eq $variant.PSObject.Properties['characterClass'] -or
            [string]$variant.characterClass -cne $className -or $null -ne $variant.PSObject.Properties['classVariants']) {
            throw "item classVariants.$className must be an equippable item of that class: $($item.itemId) -> $variantId"
        }
        if ($null -eq $sharedSlot) { $sharedSlot = [string]$variant.equipSlot }
        elseif ($sharedSlot -cne [string]$variant.equipSlot) { throw "item classVariants mix equipSlots: $($item.itemId)" }
        $variantRows.Add((@('ITEMVARIANT', $item.itemId, $className, $variantId) -join "`t"))
    }
}

# Currencies are the player's purse (실링, 골드), not bag items. The Server knows exactly these
# two; startingAmount is what a fresh character is given.
$currencyRows = [Collections.Generic.List[string]]::new()
$currencyIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($currency in @($itemDocument.currencies | Where-Object { $null -ne $_ })) {
    Assert-ExactProperties $currency @('currencyId', 'displayName', 'iconPath', 'startingAmount') 'currency'
    Assert-JsonString $currency.currencyId 'currency currencyId'
    Assert-JsonString $currency.displayName 'currency displayName'
    Assert-JsonString $currency.iconPath 'currency iconPath'
    Assert-JsonInteger $currency.startingAmount 'currency startingAmount' 0 999999999
    if (@('SILVER', 'GOLD') -cnotcontains $currency.currencyId) { throw "currency is unknown: $($currency.currencyId)" }
    if (-not $currencyIds.Add([string]$currency.currencyId)) { throw "Duplicate currency: $($currency.currencyId)" }
    $currencyRows.Add((@('CURRENCY', $currency.currencyId, [uint32]$currency.startingAmount) -join "`t"))
}

# NPC shops: which NPC placements run each shop, and what each sells for which currency item.
# The shop window pages stock ten cells at a time, so a shop sells at most forty lines.
$shopRows = [Collections.Generic.List[string]]::new()
$shopIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$shopNpcIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($shop in @($itemDocument.shops | Where-Object { $null -ne $_ })) {
    Assert-ExactProperties $shop @('shopId', 'npcPlacementIds', 'items') 'shop'
    Assert-JsonString $shop.shopId 'shop shopId'
    if ($shop.shopId -notmatch $stableIdPattern) { throw "shop shopId is not a stable ID: '$($shop.shopId)'" }
    if (-not $shopIds.Add([string]$shop.shopId)) { throw "Duplicate shop ID: $($shop.shopId)" }
    $npcIds = @($shop.npcPlacementIds)
    if ($npcIds.Count -eq 0) { throw "shop names no NPC: $($shop.shopId)" }
    foreach ($npcId in $npcIds) {
        Assert-JsonString $npcId 'shop npcPlacementId'
        if ($npcId -notmatch $stableIdPattern) { throw "shop npcPlacementId is not a stable ID: '$npcId'" }
        if (-not $shopNpcIds.Add([string]$npcId)) { throw "NPC runs two shops: $npcId" }
        $shopRows.Add((@('SHOPNPC', $shop.shopId, $npcId) -join "`t"))
    }
    $stock = @($shop.items)
    if ($stock.Count -eq 0 -or $stock.Count -gt 40) { throw "shop stock count is out of range: $($shop.shopId)" }
    $stockIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($line in $stock) {
        Assert-ExactProperties $line @('itemId', 'currencyId', 'price') 'shop item'
        Assert-JsonString $line.itemId 'shop item itemId'
        Assert-JsonString $line.currencyId 'shop item currencyId'
        Assert-JsonInteger $line.price 'shop item price' 1 999999999
        if (-not $itemIds.Contains([string]$line.itemId)) { throw "shop sells an unknown item: $($line.itemId)" }
        if (-not $currencyIds.Contains([string]$line.currencyId)) { throw "shop charges an unknown currency: $($line.currencyId)" }
        if (-not $stockIds.Add([string]$line.itemId)) { throw "shop lists an item twice: $($shop.shopId) $($line.itemId)" }
        $shopRows.Add((@('SHOPITEM', $shop.shopId, $line.itemId, $line.currencyId, [uint32]$line.price) -join "`t"))
    }
}

if ($Mode -eq 'Validate') {
    Write-Output "Item catalog Validate succeeded: $($itemRows.Count) items, $($variantRows.Count) class variants, $($currencyRows.Count) currencies, $($shopIds.Count) shops."
    return
}

if ([IO.Path]::IsPathRooted($OutputRoot)) {
    throw 'Item catalog OutputRoot must be repository-relative.'
}
$outputDirectory = [IO.Path]::GetFullPath((Join-Path $repoRoot $OutputRoot))
$repoPrefix = $repoRoot.TrimEnd('\') + '\'
if (-not $outputDirectory.StartsWith($repoPrefix, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Item catalog OutputRoot escaped the repository.'
}

$lines = [Collections.Generic.List[string]]::new()
$lines.Add("LOSTARK_ITEM_BOOTSTRAP`t7`t$($itemRows.Count + $variantRows.Count + $currencyRows.Count + $shopRows.Count)")
foreach ($row in $itemRows) { $lines.Add($row) }
foreach ($row in $variantRows) { $lines.Add($row) }
foreach ($row in $currencyRows) { $lines.Add($row) }
foreach ($row in $shopRows) { $lines.Add($row) }

$destination = Join-Path $outputDirectory 'Items.bootstrap'
Write-PublishTextCatalog -Mode $Mode -Destination $destination -Lines $lines `
    -Sources $publishSources -Context 'Item catalog' `
    -RepairCommand 'powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-ItemCatalog.ps1 -Mode Publish'
```

게시 결과 `Server/Bin/DataFiles/Items/Items.bootstrap`은 header `LOSTARK_ITEM_BOOTSTRAP\t6\t<n>`, ITEM 235행(55+180), ITEMVARIANT 150행, CURRENCY 2, SHOPNPC 11, SHOPITEM 33이다. 같은 PR에 포함한다.
`test_published_catalog_freshness.py`는 header 버전 숫자를 1로 바꿔 stale을 확인하므로 6으로 올려도 통과한다.

## 3. G02 Shared

### 3.1 `Shared/Public/Network/PacketMessages.h`

**작업: 교체** — 기준점 `enum class EQUIPMENT_SLOT : std::uint8_t`(2825행) 블록 전체.

```cpp
	/* The character info window's thirteen equipment slots, then the two avatar slots the
	   avatar book shows (an avatar hides the default look; it is not gear). NONE is an item
	   in the bag. */
	enum class EQUIPMENT_SLOT : std::uint8_t
	{
		NONE = 0,
		HELMET,
		SHOULDER,
		TOP,
		PANTS,
		GLOVES,
		WEAPON,
		NECKLACE,
		EARRING1,
		EARRING2,
		RING1,
		RING2,
		STONE,
		BRACELET,
		AVATAR_HEAD,
		AVATAR_OUTFIT,
		END
	};
```

**작업: 교체** — 기준점 `constexpr const char* Equipment_SlotKind(const EQUIPMENT_SLOT slot)` 함수 전체(2845-2866).

```cpp
	/* The Data/Items/ItemCatalog.json equipSlot an item needs to go into this slot;
	   nullptr for NONE/END. Both earrings take "earring", both rings "ring". */
	[[nodiscard]]
	constexpr const char* Equipment_SlotKind(const EQUIPMENT_SLOT slot)
	{
		switch (slot)
		{
		case EQUIPMENT_SLOT::HELMET: return "helmet";
		case EQUIPMENT_SLOT::SHOULDER: return "shoulder";
		case EQUIPMENT_SLOT::TOP: return "top";
		case EQUIPMENT_SLOT::PANTS: return "pants";
		case EQUIPMENT_SLOT::GLOVES: return "gloves";
		case EQUIPMENT_SLOT::WEAPON: return "weapon";
		case EQUIPMENT_SLOT::NECKLACE: return "necklace";
		case EQUIPMENT_SLOT::EARRING1:
		case EQUIPMENT_SLOT::EARRING2: return "earring";
		case EQUIPMENT_SLOT::RING1:
		case EQUIPMENT_SLOT::RING2: return "ring";
		case EQUIPMENT_SLOT::STONE: return "stone";
		case EQUIPMENT_SLOT::BRACELET: return "bracelet";
		case EQUIPMENT_SLOT::AVATAR_HEAD: return "avatarHead";
		case EQUIPMENT_SLOT::AVATAR_OUTFIT: return "avatarOutfit";
		default: return nullptr;
		}
	}
```

**작업: 추가** — `struct PLAYER_SNAPSHOT` 안, 기준점 `PLAYER_CONTROL_KIND eControlKind = PLAYER_CONTROL_KIND::HUMAN;`(1742행) 바로 아래, 닫는 `};` 바로 위.

```cpp
		/* Avatar items the Server has this player wearing (protocol 125): the itemId in the
		   AVATAR_HEAD / AVATAR_OUTFIT inventory slot, or empty. Every Client maps the id to
		   its class visual set; the Server never knows a model or visual set id. */
		std::string strAvatarHeadItemId;
		std::string strAvatarOutfitItemId;
```

### 3.2 `Shared/Private/Network/PacketMessages.cpp`

**작업: 추가** — 익명 namespace 안, 기준점 `bool Is_Valid_PlayerSnapshot(`(157행 근처, 주석 `//유효한 플레이어 스냅샷인지 검증` 바로 위)에 전방 선언을 넣는다.

```cpp
    bool Is_Valid_ItemId(const std::string& value);
```

**작업: 추가** — `Is_Valid_PlayerSnapshot`의 반환식에서 기준점 `LostArk::Shared::Is_Known_Player_Control_Kind(snapshot.eControlKind) &&` 바로 아래에 두 조건을 넣는다.

```cpp
            (snapshot.strAvatarHeadItemId.empty() || Is_Valid_ItemId(snapshot.strAvatarHeadItemId)) &&
            (snapshot.strAvatarOutfitItemId.empty() || Is_Valid_ItemId(snapshot.strAvatarOutfitItemId)) &&
```

**작업: 추가** — `Write_Message(CPacketWriter&, const S2C_WORLD_SNAPSHOT&)`의 player 루프 꼬리, 기준점 `writer.Write_U32(player.iHonorTitleId);`(3324행) 바로 아래.

```cpp
		if (!writer.Write_String(player.strAvatarHeadItemId, MAX_ITEM_ID_BYTES)) return false;
		if (!writer.Write_String(player.strAvatarOutfitItemId, MAX_ITEM_ID_BYTES)) return false;
```

**작업: 교체** — `Read_Message(CPacketReader&, S2C_WORLD_SNAPSHOT&)`의 player 루프 꼬리 조건에서 기준점 `!reader.Read_U32(player.iHonorTitleId))`(3704행)를 다음으로 바꾼다.

```cpp
			!reader.Read_U32(player.iHonorTitleId) ||
			!reader.Read_String(player.strAvatarHeadItemId, MAX_ITEM_ID_BYTES) ||
			!reader.Read_String(player.strAvatarOutfitItemId, MAX_ITEM_ID_BYTES))
```

`Read_String`은 빈 문자열을 허용한다(기존 `strFearPresentationId`와 같은 계약). 유효성은 뒤따르는 `Is_Valid_PlayerSnapshot`이 검사한다.

### 3.3 `Shared/Public/Network/PacketType.h`

**작업: 교체** — 기준점 `inline constexpr std::uint16_t NETWORK_PROTOCOL_VERSION = 121;`.

```cpp
	// 122 carries each player's worn avatar head/outfit item ids in PLAYER_SNAPSHOT and adds
	// the AVATAR_HEAD/AVATAR_OUTFIT equipment slots.
	inline constexpr std::uint16_t NETWORK_PROTOCOL_VERSION = 122;
```

### 3.4 `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`

- 121 pin 11곳(2238, 2426, 3103, 3224, 3409, 3416, 3788, 3905, 4150, 7490, 7518)의 `121u` → `122u`, 메시지 문자열 `"Protocol 121` → `"Protocol 125`(2242, 2429, 3107, 3231, 3789, 3906, 4386, 4404, 7491, 7519).
- **작업: 추가** — `Test_WorldSnapshotRoundTrip`에서 기준점 `first.Cooldowns.push_back({ 34060, 330, 720 });` 바로 아래.

```cpp
		first.strAvatarHeadItemId = "AVATAR_LANCEMASTER_MOKOKO_036-1_HEAD";
		first.strAvatarOutfitItemId = "AVATAR_LANCEMASTER_MOKOKO_036-1_OUTFIT";
```

- **작업: 교체** — 같은 테스트의 마지막 Require에서 기준점 `!decoded.Players[1].isCombatReady,` 바로 위 줄 `!decoded.Players[1].hasSkillTarget &&`를 다음으로 바꾼다.

```cpp
			!decoded.Players[1].hasSkillTarget &&
			decoded.Players[0].strAvatarHeadItemId == "AVATAR_LANCEMASTER_MOKOKO_036-1_HEAD" &&
			decoded.Players[0].strAvatarOutfitItemId == "AVATAR_LANCEMASTER_MOKOKO_036-1_OUTFIT" &&
			decoded.Players[1].strAvatarHeadItemId.empty() &&
			decoded.Players[1].strAvatarOutfitItemId.empty() &&
```

- **작업: 추가** — `Test_Integrated120ShopProtocol`의 `testRunner.Require(basketWritten && ...)` 바로 아래(함수 닫는 `}` 위)에 장착 왕복을 추가한다.

```cpp
        C2S_SET_EQUIPMENT wear{};
        wear.iRequestSequence = 79u;
        wear.eSlot = EQUIPMENT_SLOT::AVATAR_OUTFIT;
        wear.bEquip = true;
        wear.strItemId = "AVATAR_WARLORD_MOKOKO_036B_OUTFIT";
        CPacketWriter wearWriter;
        const bool wearWritten = Write_Message(wearWriter, wear);
        CPacketReader wearReader{wearWriter.Get_Buffer()}; C2S_SET_EQUIPMENT worn{};
        testRunner.Require(wearWritten && Read_Message(wearReader, worn) &&
            wearReader.Get_RemainingSize() == 0u && worn.eSlot == EQUIPMENT_SLOT::AVATAR_OUTFIT &&
            worn.bEquip && worn.strItemId == wear.strItemId &&
            std::string_view("avatarOutfit") == Equipment_SlotKind(EQUIPMENT_SLOT::AVATAR_OUTFIT) &&
            std::string_view("avatarHead") == Equipment_SlotKind(EQUIPMENT_SLOT::AVATAR_HEAD),
            "Protocol 125 carries the avatar equipment slots");
```

`Server/Private/ServerGameplayContractTests_SpawnGroups.cpp:1285`의 `NETWORK_PROTOCOL_VERSION == 101u`는 이미 stale이라 이번 변경과 무관하게 실패 중이다. 건드리지 않는다(팀장 소유).

## 4. G03 Server

### 4.1 `Server/Public/ItemCatalog.h` (전체)

```cpp
#pragma once

#include "Network/PacketMessages.h"
#include "ServerPlayer.h"

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace LostArk::Server
{
	// One row of the published Data/Items/ItemCatalog.json. Debug-only slice:
	// the Server only needs enough to validate a give-item request and cap a
	// stack, so no display name or category travels through the bootstrap.
	struct SERVER_ITEM_DEFINITION
	{
		std::string strItemId;
		std::uint32_t iMaxStack = 0;
		// 0 for a non-consumable; otherwise the percent of maximum HP a single
		// use restores (e.g. the three HP potion tiers: 15/30/45).
		std::uint32_t iHealPercent = 0;
		// ItemCatalog.json equipSlot ("helmet", "earring"...) and characterClass
		// ("LanceMaster"...); empty when the item is not equipment / not class-bound.
		std::string strEquipSlot;
		std::string strCharacterClass;
		// The slot a fresh character already wears this item in; NONE otherwise.
		LostArk::Shared::EQUIPMENT_SLOT eStartingEquippedSlot =
			LostArk::Shared::EQUIPMENT_SLOT::NONE;
		/* ITEMVARIANT rows: a shop template's class name -> the item the buyer of that class
		   receives. Empty for an ordinary item. A template is never equipment itself. */
		std::unordered_map<std::string, std::string> ClassVariants;
	};

	// One stock line of an NPC shop: the item, the currency it is paid in and the price of one.
	struct SERVER_SHOP_ITEM
	{
		std::string strItemId;
		SERVER_CURRENCY eCurrency = SERVER_CURRENCY::SILVER;
		std::uint32_t iPrice = 0;
	};

	class CItemCatalog final
	{
	public:
		bool Load();

		const SERVER_ITEM_DEFINITION* Find_Item(
			const std::string& itemId) const;

		const std::string& Get_Status() const { return m_strStatus; }

		/* Every (item, slot) a fresh character starts wearing, in slot order. */
		const std::vector<std::pair<std::string, LostArk::Shared::EQUIPMENT_SLOT>>&
			Get_StartingEquipment() const { return m_StartingEquipment; }
		/* The purse a fresh character starts with (CURRENCY rows). */
		const SERVER_PURSE& Get_StartingPurse() const { return m_StartingPurse; }

		/* The stock line for itemId in the shop npcPlacementId runs; null when that NPC
		   runs no shop or its shop does not sell the item. */
		const SERVER_SHOP_ITEM* Find_ShopItem(
			const std::string& npcPlacementId, const std::string& itemId) const;

	private:
		std::unordered_map<std::string, SERVER_ITEM_DEFINITION> m_Items;
		std::vector<std::pair<std::string, LostArk::Shared::EQUIPMENT_SLOT>> m_StartingEquipment;
		SERVER_PURSE m_StartingPurse;
		std::unordered_map<std::string, std::string> m_ShopIdByNpcPlacementId;
		std::unordered_map<std::string, std::vector<SERVER_SHOP_ITEM>> m_ShopItemsByShopId;
		std::string m_strStatus;
	};
}
```

현재 파일의 include 목록이 위와 다르면 include는 현재 파일 것을 유지하고 `ClassVariants` 멤버와 주석만 추가한다(구조체 나머지는 동일).

### 4.2 `Server/Private/ItemCatalog.cpp`

**작업: 교체** — `bool LostArk::Server::CItemCatalog::Load()` 함수 전체(103-271). 변경점: version 6, `ITEMVARIANT` 행 파싱, 로드 뒤 template/variant 정합 검사.

```cpp
bool LostArk::Server::CItemCatalog::Load()
{
	using ITEM_MAP = decltype(m_Items);
	ITEM_MAP previousItems = std::move(m_Items);
	m_Items.clear();
	std::unordered_map<std::string, std::string> shopIdByNpc;
	std::unordered_map<std::string, std::vector<SERVER_SHOP_ITEM>> shopItems;
	/* (template, class) -> variant, applied to m_Items after every ITEM row is known. */
	std::vector<std::array<std::string, 3>> variantRows;
	SERVER_PURSE startingPurse{};
	const auto parseCurrency = [](const std::string_view value, SERVER_CURRENCY& output)
	{
		if ("SILVER" == value) { output = SERVER_CURRENCY::SILVER; return true; }
		if ("GOLD" == value) { output = SERVER_CURRENCY::GOLD; return true; }
		return false;
	};

	const std::filesystem::path dataRoot = Resolve_DataRoot();
	const std::filesystem::path path = dataRoot / L"Items" / L"Items.bootstrap";
	std::ifstream input(path, std::ios::binary);
	if (dataRoot.empty() || !input)
	{
		m_strStatus = "Missing item bootstrap: " + path.string();
		m_Items = std::move(previousItems);
		return false;
	}

	std::string line;
	if (!std::getline(input, line))
	{
		m_strStatus = "Item bootstrap is empty";
		m_Items = std::move(previousItems);
		return false;
	}
	StripCarriageReturn(line);
	const std::vector<std::string_view> header = SplitTabs(line);
	std::uint32_t version = 0;
	std::uint32_t rowCount = 0;
	if (3u != header.size() || "LOSTARK_ITEM_BOOTSTRAP" != header[0] ||
		!ParseNumber(header[1], version) ||
		!ParseNumber(header[2], rowCount) || 0u == rowCount || rowCount > 4096u)
	{
		m_strStatus = "Item bootstrap header is invalid: " + path.string();
		m_Items = std::move(previousItems);
		return false;
	}

	if (7u != version)
	{
		m_strStatus = "Item bootstrap version mismatch: expected 7, got " +
			std::to_string(version) + "; path=" + path.string() +
			"; run powershell -ExecutionPolicy Bypass -File "
			"Tools/GameplayPipeline/Publish-ItemCatalog.ps1 -Mode Publish";
		m_Items = std::move(previousItems);
		return false;
	}

	for (std::uint32_t row = 0; row < rowCount; ++row)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "Item bootstrap row is truncated";
			m_Items = std::move(previousItems);
			return false;
		}
		StripCarriageReturn(line);
		const std::vector<std::string_view> fields = SplitTabs(line);
		/* CURRENCY <SILVER|GOLD> <startingAmount>: the fresh-character purse. */
		if (3u == fields.size() && "CURRENCY" == fields[0])
		{
			SERVER_CURRENCY currency{};
			std::uint32_t amount = 0;
			if (!parseCurrency(fields[1], currency) || !ParseNumber(fields[2], amount))
			{
				m_strStatus = "Item bootstrap CURRENCY row is invalid";
				m_Items = std::move(previousItems);
				return false;
			}
			startingPurse.Amount(currency) = amount;
			continue;
		}
		/* SHOPNPC <shopId> <npcPlacementId>: the NPC runs that shop. */
		if (3u == fields.size() && "SHOPNPC" == fields[0])
		{
			if (!IsStableId(fields[1]) || !IsStableId(fields[2]) ||
				!shopIdByNpc.emplace(std::string(fields[2]), std::string(fields[1])).second)
			{
				m_strStatus = "Item bootstrap SHOPNPC row is invalid";
				m_Items = std::move(previousItems);
				return false;
			}
			continue;
		}
		/* SHOPITEM <shopId> <itemId> <SILVER|GOLD> <price>: one stock line. */
		if (5u == fields.size() && "SHOPITEM" == fields[0])
		{
			SERVER_SHOP_ITEM stock{};
			if (!IsStableId(fields[1]) || !IsStableId(fields[2]) ||
				!parseCurrency(fields[3], stock.eCurrency) ||
				!ParseNumber(fields[4], stock.iPrice) || 0u == stock.iPrice)
			{
				m_strStatus = "Item bootstrap SHOPITEM row is invalid";
				m_Items = std::move(previousItems);
				return false;
			}
			stock.strItemId = fields[2];
			shopItems[std::string(fields[1])].push_back(std::move(stock));
			continue;
		}
		/* ITEMVARIANT <templateId> <className> <itemId>: the class variant a template buys. */
		if (4u == fields.size() && "ITEMVARIANT" == fields[0])
		{
			if (!IsStableId(fields[1]) || fields[2].empty() || !IsStableId(fields[3]))
			{
				m_strStatus = "Item bootstrap ITEMVARIANT row is invalid";
				m_Items = std::move(previousItems);
				return false;
			}
			variantRows.push_back({ std::string(fields[1]), std::string(fields[2]), std::string(fields[3]) });
			continue;
		}
		SERVER_ITEM_DEFINITION item{};
		if (7u != fields.size() || "ITEM" != fields[0] || !IsStableId(fields[1]) ||
			!ParseNumber(fields[2], item.iMaxStack) || 0u == item.iMaxStack ||
			!ParseNumber(fields[3], item.iHealPercent) || item.iHealPercent > 100u ||
			fields[4].empty() || fields[5].empty() ||
			!ParseStartingSlot(fields[6], item.eStartingEquippedSlot))
		{
			m_strStatus = "Item bootstrap row is invalid";
			m_Items = std::move(previousItems);
			return false;
		}
		item.strItemId = fields[1];
		/* "-" is the publisher's "no value" marker for the two optional columns. */
		if ("-" != fields[4])
			item.strEquipSlot = fields[4];
		if ("-" != fields[5])
			item.strCharacterClass = fields[5];
		if (!m_Items.emplace(item.strItemId, std::move(item)).second)
		{
			m_strStatus = "Duplicate item ID";
			m_Items = std::move(previousItems);
			return false;
		}
	}

	if (std::getline(input, line))
	{
		m_strStatus = "Item bootstrap has trailing rows";
		m_Items = std::move(previousItems);
		return false;
	}

	/* A template is not equipment and every variant is equipment of exactly that class;
	   the publisher checks the same so a hand-edited file stays honest. */
	for (const auto& [templateId, className, variantId] : variantRows)
	{
		const auto templateIter = m_Items.find(templateId);
		const auto variantIter = m_Items.find(variantId);
		if (m_Items.end() == templateIter || m_Items.end() == variantIter ||
			!templateIter->second.strEquipSlot.empty() || !templateIter->second.strCharacterClass.empty() ||
			variantIter->second.strEquipSlot.empty() || variantIter->second.strCharacterClass != className ||
			!templateIter->second.ClassVariants.emplace(className, variantId).second)
		{
			m_strStatus = "Item bootstrap ITEMVARIANT row does not fit its items: " + templateId;
			m_Items = std::move(previousItems);
			return false;
		}
	}

	/* Every stock line must name a catalog item, and every shop an NPC runs must sell
	   something. The publisher checks the same; this keeps a hand-edited file honest. */
	for (const auto& [shopId, stock] : shopItems)
		for (const SERVER_SHOP_ITEM& line : stock)
			if (!m_Items.contains(line.strItemId))
			{
				m_strStatus = "Item bootstrap shop sells an unknown item";
				m_Items = std::move(previousItems);
				return false;
			}
	for (const auto& [npcId, shopId] : shopIdByNpc)
		if (!shopItems.contains(shopId))
		{
			m_strStatus = "Item bootstrap shop has no stock: " + shopId;
			m_Items = std::move(previousItems);
			return false;
		}

	m_StartingEquipment.clear();
	for (const auto& [itemId, item] : m_Items)
		if (LostArk::Shared::EQUIPMENT_SLOT::NONE != item.eStartingEquippedSlot)
			m_StartingEquipment.emplace_back(itemId, item.eStartingEquippedSlot);
	std::sort(m_StartingEquipment.begin(), m_StartingEquipment.end(),
		[](const auto& left, const auto& right) { return left.second < right.second; });
	m_StartingPurse = startingPurse;
	m_ShopIdByNpcPlacementId = std::move(shopIdByNpc);
	m_ShopItemsByShopId = std::move(shopItems);
	m_strStatus = "Loaded item bootstrap";
	return true;
}
```

`#include <array>`가 파일 상단 include에 없으면 추가한다. `ParseStartingSlot`은 아바타 슬롯을 모른 채 둔다(시작 장비로 아바타를 주지 않는다는 publisher 규칙과 일치).

### 4.3 `Server/Private/GameRoom_Inventory.cpp`

**작업: 교체** — `bool LostArk::Server::CGameRoom::Apply_BuyItems(...)` 함수 전체(300-363). 변경점: 상점 줄이 template이면 구매자 클래스 variant를 지급하고, variant가 없는 클래스는 basket 전체를 거부한다. 가격은 template 줄의 값이다.

```cpp
bool LostArk::Server::CGameRoom::Apply_BuyItems(
	SERVER_PLAYER& player, const LostArk::Shared::C2S_BUY_ITEMS& request) const
{
	using namespace LostArk::Shared;
	/* The window stays open while the player walks a little, so the check is a few metres
	   wider than the 3 m the client stops at. */
	constexpr float SHOP_INTERACTION_RADIUS = 6.f;
	if (WORLD_ID::BERN != m_eWorldId || 0u == player.iCurrentHp)
		return false;
	const auto npc = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
		[&request](const SERVER_WORLD_ENTITY& entity)
		{
			return WORLD_BOOTSTRAP_KIND::NPC == entity.eKind &&
				entity.strPlacementId == request.strNpcPlacementId;
		});
	if (m_WorldEntities.end() == npc)
		return false;
	const float deltaX = player.fPositionX - npc->fPositionX;
	const float deltaZ = player.fPositionZ - npc->fPositionZ;
	if (deltaX * deltaX + deltaZ * deltaZ > SHOP_INTERACTION_RADIUS * SHOP_INTERACTION_RADIUS)
		return false;

	/* Price every line first; the basket is bought whole or not at all. */
	std::vector<INVENTORY_ITEM_SNAPSHOT> staged = player.Inventory;
	SERVER_PURSE stagedPurse = player.Purse;
	const auto bagEntry = [&staged](const std::string& itemId)
	{
		return std::find_if(staged.begin(), staged.end(), [&itemId](const INVENTORY_ITEM_SNAPSHOT& item)
			{ return item.strItemId == itemId && EQUIPMENT_SLOT::NONE == item.eEquippedSlot; });
	};
	for (const SHOP_BASKET_ENTRY& entry : request.Entries)
	{
		const SERVER_SHOP_ITEM* stock = m_ItemCatalog.Find_ShopItem(request.strNpcPlacementId, entry.strItemId);
		const SERVER_ITEM_DEFINITION* definition = m_ItemCatalog.Find_Item(entry.strItemId);
		if (nullptr == stock || nullptr == definition)
			return false;
		/* A shop template is one icon for every class: the buyer receives the variant of the
		   buyer's class. A class the template has no variant for cannot buy it. */
		const SERVER_ITEM_DEFINITION* granted = definition;
		if (!definition->ClassVariants.empty())
		{
			const auto variant = definition->ClassVariants.find(Item_ClassName(player.eCharacterClass));
			if (definition->ClassVariants.end() == variant)
				return false;
			granted = m_ItemCatalog.Find_Item(variant->second);
			if (nullptr == granted)
				return false;
		}
		const std::uint64_t cost = static_cast<std::uint64_t>(stock->iPrice) * entry.iQuantity;
		std::uint32_t& purse = stagedPurse.Amount(stock->eCurrency);
		if (purse < cost)
			return false;
		purse -= static_cast<std::uint32_t>(cost);

		/* No silent cap: a line that would overflow the stack refuses the basket. */
		const auto owned = bagEntry(granted->strItemId);
		if (staged.end() == owned)
		{
			if (staged.size() >= MAX_INVENTORY_ITEMS || entry.iQuantity > granted->iMaxStack)
				return false;
			INVENTORY_ITEM_SNAPSHOT item{};
			item.strItemId = granted->strItemId;
			item.iQuantity = entry.iQuantity;
			staged.push_back(std::move(item));
		}
		else
		{
			if (static_cast<std::uint64_t>(owned->iQuantity) + entry.iQuantity > granted->iMaxStack)
				return false;
			owned->iQuantity += entry.iQuantity;
		}
	}
	player.Inventory = std::move(staged);
	player.Purse = stagedPurse;
	return true;
}
```

`Item_ClassName`은 같은 파일 167-202행의 익명 namespace 함수이며 `Apply_BuyItems`보다 앞에 있어 그대로 쓴다. `Apply_SetEquipment`·`Unequip_OtherClassItems`는 수정 없이 아바타 슬롯을 처리한다(kind 문자열 일치 + `Is_UsableByClass`).

### 4.4 `Server/Private/GameRoom_Replication.cpp`

**작업: 추가** — `Broadcast_WorldSnapshot()`에서 기준점 `snapshot.iHonorTitleId = player.iHonorTitleId;`(634행) 바로 아래.

```cpp
		for (const INVENTORY_ITEM_SNAPSHOT& item : player.Inventory)
		{
			if (EQUIPMENT_SLOT::AVATAR_HEAD == item.eEquippedSlot)
				snapshot.strAvatarHeadItemId = item.strItemId;
			else if (EQUIPMENT_SLOT::AVATAR_OUTFIT == item.eEquippedSlot)
				snapshot.strAvatarOutfitItemId = item.strItemId;
		}
```

이 함수의 `using namespace LostArk::Shared;` 여부를 확인하고 없으면 `LostArk::Shared::` 접두를 붙인다.

## 5. G04 Client 데이터·UI

### 5.1 `Client/Public/ItemCatalog.h`

**작업: 추가** — `struct ITEM_DEFINITION`에서 기준점 `std::string strGrade;` 바로 아래.

```cpp
		/* Avatar only: the EquipmentPresentationCatalog visual set the character wears when
		   this item sits in its avatar slot. Empty for gear and for a shop template. */
		std::string strVisualSetId;
		/* Shop template only: characterClass -> the class item the Server hands the buyer.
		   Shown in the shop under the template's own name and icon; never equippable itself. */
		std::unordered_map<std::string, std::string> ClassVariants;
```

파일 상단 include에 `#include <unordered_map>`을 추가한다(`<string>` 아래).

### 5.2 `Client/Private/ItemCatalog.cpp`

**작업: 추가** — item 파싱 루프에서 기준점 `ReadOptionalText("grade", definition.strGrade);` 바로 아래.

```cpp
		ReadOptionalText("visualSetId", definition.strVisualSetId);
		if (const DATA_JSON_VALUE* pVariants = value.Find("classVariants");
			nullptr != pVariants && pVariants->Get_Type() == DATA_JSON_TYPE::OBJECT)
		{
			for (const auto& [strClass, variant] : pVariants->Get_Object())
			{
				if (variant.Get_Type() != DATA_JSON_TYPE::STRING || variant.Get_String().empty())
				{
					outStatus = "ItemCatalog.json has an invalid classVariants entry";
					return false;
				}
				definition.ClassVariants[strClass] = variant.Get_String();
			}
		}
```

`DATA_JSON_VALUE::Get_Object()`의 실제 컨테이너 타입(`std::map<std::string, DATA_JSON_VALUE>` 계열)은 `Engine/Public` 헤더에서 확인해 반복 형태를 맞춘다.

### 5.3 `Client/Public/InventoryView.h` / `Client/Private/InventoryView.cpp`

목표: 우클릭으로 아바타를 착용하고, 착용 중인 아바타는 가방에 계속 보이며 우클릭하면 해제한다(장비 13슬롯은 캐릭터 정보 창에서 해제하지만 아바타 슬롯은 그 창에 없다).

**InventoryView.h 작업: 추가** — 기준점 `bool_t Try_Consume_EquipRequest(string& outItemId);`(76행) 바로 아래.

```cpp
	/* Right-click on a worn avatar in the bag: the slot to take off. */
	bool_t Try_Consume_UnequipRequest(LostArk::Shared::EQUIPMENT_SLOT& outSlot);
```

**작업: 추가** — 기준점 `string m_strPendingEquipItemId;`(138행) 바로 아래.

```cpp
	LostArk::Shared::EQUIPMENT_SLOT m_ePendingUnequipSlot = LostArk::Shared::EQUIPMENT_SLOT::NONE;
```

**작업: 교체** — `Cancel_Interaction()`의 `m_strPendingEquipItemId.clear();` 줄을 다음으로.

```cpp
		m_strPendingEquipItemId.clear(); m_ePendingUnequipSlot = LostArk::Shared::EQUIPMENT_SLOT::NONE;
```

**InventoryView.cpp 작업: 추가** — 기준점 `Try_Consume_EquipRequest` 함수 정의(114-121) 바로 아래.

```cpp
bool_t Client::CInventoryView::Try_Consume_UnequipRequest(LostArk::Shared::EQUIPMENT_SLOT& outSlot)
{
	if (LostArk::Shared::EQUIPMENT_SLOT::NONE == m_ePendingUnequipSlot)
		return false;
	outSlot = m_ePendingUnequipSlot;
	m_ePendingUnequipSlot = LostArk::Shared::EQUIPMENT_SLOT::NONE;
	return true;
}
```

**작업: 교체** — `Build_FilteredIndices`의 기준점 블록(501-505):

```cpp
		/* Worn equipment lives in the character info window, not the bag. */
		if (LostArk::Shared::EQUIPMENT_SLOT::NONE != items[i].eEquippedSlot)
			continue;
```
를
```cpp
		/* Worn gear lives in the character info window, not the bag. A worn avatar has no
		   window slot, so it stays in the bag and a right-click takes it off. */
		if (LostArk::Shared::EQUIPMENT_SLOT::NONE != items[i].eEquippedSlot &&
			LostArk::Shared::EQUIPMENT_SLOT::AVATAR_HEAD != items[i].eEquippedSlot &&
			LostArk::Shared::EQUIPMENT_SLOT::AVATAR_OUTFIT != items[i].eEquippedSlot)
			continue;
```

**작업: 교체** — 우클릭 블록(574-586) 전체:

```cpp
	/* Right-click on a bag item: equip gear or an avatar; a worn avatar comes off instead. */
	if (Router.Is_RightClickEdge() && iHoveredSlot >= 0 &&
		static_cast<size_t>(iHoveredSlot) < m_DisplayOrder.size() &&
		m_DisplayOrder[iHoveredSlot] < filteredIndices.size() &&
		filteredIndices[m_DisplayOrder[iHoveredSlot]] < items.size())
	{
		const LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& Item = items[filteredIndices[m_DisplayOrder[iHoveredSlot]]];
		if (LostArk::Shared::EQUIPMENT_SLOT::NONE != Item.eEquippedSlot)
		{
			m_ePendingUnequipSlot = Item.eEquippedSlot;
		}
		else
		{
			const ITEM_DEFINITION* pEquip = CItemCatalog::Find_ById(Item.strItemId);
			if (nullptr != pEquip && !pEquip->strEquipSlot.empty())
				m_strPendingEquipItemId = Item.strItemId;
		}
	}
```

### 5.4 `Client/Private/MainApp.cpp`

**작업: 추가** — 기준점 `if (EQUIPMENT_SLOT::NONE != eTarget) (void)CNetworkManager::Get().Send_SetEquipment(m_iNextUseItemSequence++, eTarget, true, strEquipItemId);`(5045-5047) 블록의 닫는 `}`(5048, `if (m_pInventoryView->Try_Consume_EquipRequest(...))`의 닫는 중괄호) 바로 아래, 바깥 `if (nullptr != m_pInventoryView)` 블록 안.

```cpp
		LostArk::Shared::EQUIPMENT_SLOT eAvatarUnequipSlot = LostArk::Shared::EQUIPMENT_SLOT::NONE;
		if (m_pInventoryView->Try_Consume_UnequipRequest(eAvatarUnequipSlot))
			(void)CNetworkManager::Get().Send_SetEquipment(
				m_iNextUseItemSequence++, eAvatarUnequipSlot, false, {});
```

기존 5014-5048의 착용 루프는 수정하지 않는다: `Equipment_SlotKind`가 `"avatarHead"/"avatarOutfit"`를 돌려주므로 아바타 아이템이 `AVATAR_HEAD/AVATAR_OUTFIT`로 보내진다.

### 5.5 `Client/Public/ShopWindowView.h` / `Client/Private/ShopWindowView.cpp` — 페이지

**ShopWindowView.h 작업: 추가** — 기준점 `bool_t m_bIconsDirty = true;`(91행) 바로 아래.

```cpp
	/* Ten stock lines per page; the two page plates under the list are prev/next. */
	uint32_t m_iPage = 0;
```

**작업: 추가** — private 함수 목록에서 기준점 `void Update_Basket();` 바로 아래.

```cpp
	uint32_t Get_PageCount() const;
	/* Absolute stock index the cell shows on the current page, or SIZE_MAX past the end. */
	size_t Get_StockIndex(uint32_t iCell) const;
```

**ShopWindowView.cpp 작업: 추가** — 기준점 `constexpr const char* EMPTY_BUTTON_ID = "Shop_EmptyBtn";` 바로 아래.

```cpp
	constexpr const char* PAGE_PREV_ID = "Shop_PageBg_0";
	constexpr const char* PAGE_NEXT_ID = "Shop_PageBg_1";
```

**작업: 추가** — 기준점 `const wchar_t* BALANCE_TEXT = ...;` 바로 아래.

```cpp
	/* Page plates: "이전 페이지" / "다음 페이지"; the current/total page goes on the next plate's tail. */
	const wchar_t* PAGE_PREV_TEXT = L"\x25C0 \xC774\xC804 \xD398\xC774\xC9C0";
	const wchar_t* PAGE_NEXT_TEXT = L"\xB2E4\xC74C \xD398\xC774\xC9C0 \x25B6";
```

**작업: 교체** — `Open`의 `m_bIconsDirty = true;` 줄 앞에 `m_iPage = 0;`를 넣는다(`m_pShop = pShop;` 바로 아래).

**작업: 추가** — `Update_Basket` 정의 바로 아래에 두 함수 정의.

```cpp
uint32_t Client::CShopWindowView::Get_PageCount() const
{
	if (nullptr == m_pShop || m_pShop->Items.empty())
		return 1u;
	return static_cast<uint32_t>((m_pShop->Items.size() + CELL_COUNT - 1u) / CELL_COUNT);
}

size_t Client::CShopWindowView::Get_StockIndex(const uint32_t iCell) const
{
	const size_t iIndex = static_cast<size_t>(m_iPage) * CELL_COUNT + iCell;
	return (nullptr != m_pShop && iIndex < m_pShop->Items.size()) ? iIndex : SIZE_MAX;
}
```

**작업: 교체** — `Update_Stock`, `Refresh_Icons`, `Apply_Visibility`, `Render_Text`에서 `m_pShop->Items[iCell]` / `iStockCount` 계산을 페이지 기준으로 바꾼다. 각 함수의 교체 블록:

`Update_Stock`(318-353) 전체:
```cpp
void Client::CShopWindowView::Update_Stock()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pBackgroundView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pBackgroundView->Get_ResolutionHeight();

	for (uint32_t iCell = 0; iCell < CELL_COUNT; ++iCell)
	{
		const size_t iStock = Get_StockIndex(iCell);
		if (SIZE_MAX == iStock)
			break;
		const string strCellId = Make_Id("Shop_Cell_", iCell);
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect(strCellId, fX, fY, fWidth, fHeight))
			continue;
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		m_pBackgroundView->Set_SlotTintMultiplier(strCellId, bHovered ?
			float4_t(1.25f, 1.25f, 1.25f, 1.f) : float4_t(1.f, 1.f, 1.f, 1.f));
		if (!Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
			continue;

		/* One more of this item: an existing line grows, otherwise a free slot takes it. */
		const SHOP_ITEM_DEFINITION& Stock = m_pShop->Items[iStock];
		auto Line = std::find_if(m_Basket.begin(), m_Basket.end(),
			[&Stock](const BASKET_LINE& Candidate) { return Candidate.strItemId == Stock.strItemId; });
		if (m_Basket.end() != Line)
		{
			if (Line->iQuantity < LostArk::Shared::MAX_SHOP_BASKET_QUANTITY)
				++Line->iQuantity;
		}
		else if (m_Basket.size() < BASKET_COUNT)
		{
			m_Basket.push_back({ Stock.strItemId, 1u, Stock.iPrice });
			m_bIconsDirty = true;
		}
		CMainApp::Play_UIButtonClickSound();
	}

	/* Page plates: prev/next when there is more than one page. The Server sees only item ids,
	   so paging never touches the basket. */
	const uint32_t iPages = Get_PageCount();
	const float4_t vNormal(1.f, 1.f, 1.f, 1.f), vHover(1.25f, 1.25f, 1.25f, 1.f), vDimmed(0.5f, 0.5f, 0.5f, 1.f);
	const auto Page = [&](const char* pId, const bool_t bEnabled, const int32_t iDelta)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect(pId, fX, fY, fWidth, fHeight))
			return;
		const bool_t bHovered = bEnabled && Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		m_pBackgroundView->Set_SlotTintMultiplier(pId, !bEnabled ? vDimmed : (bHovered ? vHover : vNormal));
		if (bEnabled && Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			m_iPage = static_cast<uint32_t>(static_cast<int32_t>(m_iPage) + iDelta);
			m_bIconsDirty = true;
		}
	};
	Page(PAGE_PREV_ID, m_iPage > 0u, -1);
	Page(PAGE_NEXT_ID, m_iPage + 1u < iPages, 1);
}
```

`Refresh_Icons`의 재고 루프(376-382):
```cpp
	for (uint32_t iCell = 0; iCell < CELL_COUNT; ++iCell)
	{
		const size_t iStock = Get_StockIndex(iCell);
		if (SIZE_MAX == iStock)
			break;
		const ITEM_DEFINITION* pItem = CItemCatalog::Find_ById(m_pShop->Items[iStock].strItemId);
		if (nullptr != pItem && !pItem->strIconPath.empty())
			m_pBackgroundView->Set_SlotTexture(Make_Id("Shop_CellIcon_", iCell), pItem->strIconPath);
	}
```

`Apply_Visibility`의 `iShownStock` 계산(398행)을
```cpp
	const size_t iShownStock = (std::min)(m_pShop->Items.size() - (std::min)(m_pShop->Items.size(), static_cast<size_t>(m_iPage) * CELL_COUNT), static_cast<size_t>(CELL_COUNT));
```
로 바꾼다. 뒤의 `for (uint32_t iCell = static_cast<uint32_t>(iShownStock); ...)` 숨김 루프는 그대로.

`Render_Text`의 재고 루프(483-501):
```cpp
	for (uint32_t iCell = 0; iCell < CELL_COUNT; ++iCell)
	{
		const size_t iStock = Get_StockIndex(iCell);
		if (SIZE_MAX == iStock)
			break;
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect(Make_Id("Shop_Cell_", iCell), fX, fY, fWidth, fHeight))
			continue;
		const SHOP_ITEM_DEFINITION& Stock = m_pShop->Items[iStock];
		const ITEM_DEFINITION* pItem = CItemCatalog::Find_ById(Stock.strItemId);
		if (nullptr != pItem)
		{
			const std::wstring strName = Utf8_ToWide(pItem->strDisplayName);
			DrawInBox(fX + 56.f * STAGE_SCALE, fY + 5.f * STAGE_SCALE, 194.f * STAGE_SCALE,
				23.f * STAGE_SCALE, strName.c_str(), TEXT("Font_YG760"), 14.f, 0.f,
				Grade_Color(pItem->strGrade));
		}
		const std::wstring strPrice = Format_Money(Stock.iPrice);
		DrawInBox(fX + 179.f * STAGE_SCALE, fY + 54.f * STAGE_SCALE, 134.f * STAGE_SCALE,
			22.f * STAGE_SCALE, strPrice.c_str(), TEXT("Font_YG760"), 14.f, 1.f, COLOR_WHITE);
	}
```

**작업: 추가** — `Render_Text`에서 기준점 `DrawLabel(EMPTY_BUTTON_ID, EMPTY_TEXT, ...)` 바로 아래.

```cpp
	if (Get_PageCount() > 1u)
	{
		const std::wstring strPage = std::to_wstring(m_iPage + 1u) + L" / " + std::to_wstring(Get_PageCount());
		DrawLabel(PAGE_PREV_ID, PAGE_PREV_TEXT, TEXT("Font_YG760"), 14.f, 0.5f,
			m_iPage > 0u ? COLOR_WHITE : float4_t(0.6f, 0.6f, 0.6f, 1.f));
		DrawLabel(PAGE_NEXT_ID, PAGE_NEXT_TEXT, TEXT("Font_YG760"), 14.f, 0.5f,
			m_iPage + 1u < Get_PageCount() ? COLOR_WHITE : float4_t(0.6f, 0.6f, 0.6f, 1.f));
		DrawLabel(TITLE_ID, strPage.c_str(), TEXT("Font_YG760"), 14.f, 1.f, COLOR_WHITE);
	}
```

`Shop_PageBg_0/1`은 `ShopUI.json`에 이미 있는 plate(각 228x52)라 새 이미지가 필요 없다. 페이지가 1개면 plate는 그대로 보이고 글자만 없다.

## 6. G05 Client 복제 표현 — `CClientReplication`

### 6.1 `Client/Public/ClientReplication.h`

**작업: 추가** — include 목록에서 기준점 `#include "ValtanPresentationGenerationAdmission.h"` 바로 아래.

```cpp
#include "EquipmentPresentationCatalog.h"
#include "EquipmentPresentationService.h"
```
표준 include에 `#include <array>`를 `<chrono>` 위에 추가한다.

**작업: 추가** — private 함수 구역, 기준점 `bool Apply_PlayerSnapshot(` 선언 바로 아래(실제 선언 위치를 헤더에서 찾아 그 아래).

```cpp
		/* Puts the snapshot's worn avatar items on the character through the equipment
		   presentation service; idempotent per (entity, head, outfit). A failure keeps the
		   character as it is and is not retried until the worn pair changes. */
		void Apply_AvatarPresentation(
			LostArk::Shared::NET_ENTITY_ID iNetEntityId, CCharacter& character,
			const LostArk::Shared::PLAYER_SNAPSHOT& player);
```

**작업: 추가** — 멤버 구역, 기준점 `m_HonorTitleByNetEntityId;`(756행) 바로 아래.

```cpp
		/* Worn avatar item ids this replication last applied (or failed to apply) per player.
		   Erased on despawn and on body replacement so the fresh body is dressed again. */
		std::unordered_map<LostArk::Shared::NET_ENTITY_ID, std::pair<std::string, std::string>>
			m_AppliedAvatarByNetEntityId;
		/* Built on first avatar; the catalog is Data/Actors/EquipmentPresentationCatalog.json. */
		CEquipmentPresentationCatalog m_EquipmentCatalog;
		unique_ptr<CEquipmentPresentationService> m_pEquipmentPresentation;
		bool_t m_isEquipmentCatalogLoaded = false;
		bool_t m_isEquipmentCatalogLoadAttempted = false;
```

### 6.2 `Client/Private/ClientReplication.cpp`

**작업: 추가** — `Apply_PlayerSnapshot`에서 기준점 `character->Apply_NetworkPresentationHidden((player.CardMaze.flags & LostArk::Shared::CARD_MAZE_ENTRY_HIDDEN) != 0u);`(4647행) 바로 아래.

```cpp
	Apply_AvatarPresentation(player.iNetEntityId, *character, player);
```

**작업: 추가** — `Apply_Despawn`에서 기준점 `m_GuidePromptSequences.erase(despawned.iNetEntityId);` 바로 아래.

```cpp
	m_AppliedAvatarByNetEntityId.erase(despawned.iNetEntityId);
```

**작업: 추가** — `Reset_World`에서 기준점 `m_HonorTitleByNetEntityId.clear();`(4526행) 바로 아래.

```cpp
	m_AppliedAvatarByNetEntityId.clear();
	if (nullptr != m_pEquipmentPresentation)
		m_pEquipmentPresentation->On_LevelChanged();
```

**작업: 추가** — `Replace_CharacterClass`에서 기준점 `if (isLocallyControlled) { m_LocalCharacterHandle = newHandle; CAnimationTargetService::Bind(stagedCharacter); }`(4367-4371) 바로 위.

```cpp
	/* A replaced body is a fresh CCharacter: the next snapshot dresses it again. */
	m_AppliedAvatarByNetEntityId.erase(snapshot.iNetEntityId);
```

**작업: 추가** — 함수 정의: `Apply_PlayerSnapshot` 정의 바로 뒤(4719행 닫는 `}` 아래).

```cpp
void Client::CClientReplication::Apply_AvatarPresentation(
	const LostArk::Shared::NET_ENTITY_ID iNetEntityId, CCharacter& character,
	const LostArk::Shared::PLAYER_SNAPSHOT& player)
{
	const std::pair<std::string, std::string> worn{ player.strAvatarHeadItemId, player.strAvatarOutfitItemId };
	const auto applied = m_AppliedAvatarByNetEntityId.find(iNetEntityId);
	if (m_AppliedAvatarByNetEntityId.end() != applied && applied->second == worn)
		return;
	/* Nothing worn and nothing ever applied: the default look needs no service. */
	if (worn.first.empty() && worn.second.empty() &&
		m_AppliedAvatarByNetEntityId.end() == applied)
	{
		m_AppliedAvatarByNetEntityId[iNetEntityId] = worn;
		return;
	}

	if (nullptr == m_pEquipmentPresentation)
		m_pEquipmentPresentation =
			std::make_unique<CEquipmentPresentationService>(m_Desc.pDevice, m_Desc.pContext);
	if (!m_isEquipmentCatalogLoadAttempted)
	{
		m_isEquipmentCatalogLoadAttempted = true;
		std::string loadError;
		m_isEquipmentCatalogLoaded = m_EquipmentCatalog.Load(loadError);
		if (!m_isEquipmentCatalogLoaded)
			OutputDebugStringA(("[ClientReplication][Avatar] catalog: " + loadError + "\n").c_str());
	}
	/* Recorded before the attempt: a broken pair is tried once, not every snapshot. */
	m_AppliedAvatarByNetEntityId[iNetEntityId] = worn;
	if (!m_isEquipmentCatalogLoaded)
		return;

	std::array<std::string, ETOI(EQUIPMENT_SLOT_ID::END)> selected{};
	const auto Resolve = [&](const std::string& strItemId, const EQUIPMENT_SLOT_ID eSlot)
	{
		if (strItemId.empty())
			return;
		const ITEM_DEFINITION* pItem = CItemCatalog::Find_ById(strItemId);
		if (nullptr == pItem || pItem->strVisualSetId.empty())
		{
			OutputDebugStringA(("[ClientReplication][Avatar] no visual set for item " + strItemId + "\n").c_str());
			return;
		}
		selected[ETOI(eSlot)] = pItem->strVisualSetId;
	};
	Resolve(worn.first, EQUIPMENT_SLOT_ID::HEAD);
	Resolve(worn.second, EQUIPMENT_SLOT_ID::UPPER);

	std::string error;
	const bool_t bNothing = std::all_of(selected.begin(), selected.end(),
		[](const std::string& strSet) { return strSet.empty(); });
	const bool_t bApplied = bNothing ?
		m_pEquipmentPresentation->Reset_Preview(character, error) :
		m_pEquipmentPresentation->Apply_Preview(character, m_EquipmentCatalog, selected, error);
	if (!bApplied)
		OutputDebugStringA(("[ClientReplication][Avatar] " + error + "\n").c_str());
}
```

이 CPP 상단에 `#include "ItemCatalog.h"`가 없으면 추가한다(`ITEM_DEFINITION`/`CItemCatalog`). `EQUIPMENT_SLOT_ID::UPPER`는 outfit visualSet의 `primarySlot`이며 `occupiedSlots`(UPPER/LOWER/HANDS/SHOULDER)는 `Apply_Preview`가 확장한다. 캐릭터 선택의 `Wear_CustomizingSet`과 같은 트랜잭션이라 실패 시 이전 외형이 유지된다.

## 7. G06 검증

```text
G01  Publish-ItemCatalog.ps1 -Mode Validate → Publish; Items.bootstrap header 6·행 수; git diff --check
G02  NetworkProtocolHarness 빌드·실행 failures 0 (World Snapshot Players Round Trip, Protocol 125 ...)
G03  Server 빌드; Server.exe --contract-test; 로컬 Server 시작 로그에 Item bootstrap 오류 없음
G04  Client 빌드; 베른 상점 NPC에서 30줄 3페이지 표시·구매(GOLD 차감·인벤 입고 = 자기 클래스 아이템)
G05  인벤 우클릭 착용 → 본인 화면·두 번째 Client 화면에서 모코코 착용 보임; 우클릭 해제; 클래스 변경 시 Server가 벗김
```

화면 확인은 사용자가 로컬 Server + Client 2개로 수행한다. 아이콘 PNG 30장(`UI/Items/Avatar/`)이 없으면 셀 아이콘이 비고 이름·가격만 보인다(구매·착용은 아이콘과 무관).

## 8. 범위 밖

- 아바타 책(`AvatarBookWindowView`)은 옛 고정 파츠 방식 그대로 둔다(착용 경로는 인벤토리 우클릭). 책을 visualSet 기반으로 바꾸는 것은 후속.
- 상점 판매(junk)·재구매 탭, 아바타 염색, 곰탈(AV_040) 아이템은 이번 범위가 아니다.
- 팀 문서: `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`의 아이템·상점 절에 template/`classVariants`, `visualSetId`, 아바타 슬롯 2개와 protocol 125를 RESULT 뒤 갱신한다.
