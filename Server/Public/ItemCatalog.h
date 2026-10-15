#pragma once

// 인벤토리 snapshot과 시작 장착 목록이 공유하는 EQUIPMENT_SLOT 형식을 사용한다.
#include "Network/PacketMessages.h"
// 시작 소지금과 상점 화폐가 사용하는 SERVER_PURSE/SERVER_CURRENCY를 가져온다.
#include "ServerPlayer.h"

// 아이템 수량·전투 수치·가격을 고정 폭 정수로 보관한다.
#include <cstdint>
// 게시된 아이템·직업·상점 식별자와 로드 진단 문자열을 소유한다.
#include <string>
// 아이템 조회, 직업별 지급 변형과 NPC→상점→재고 연결을 저장한다.
#include <unordered_map>
// 시작 장비를 아이템 ID와 장착 슬롯의 pair로 보관한다.
#include <utility>
// 시작 장비의 슬롯 순서와 상점별 재고 행의 게시 순서를 보관한다.
#include <vector>

namespace LostArk::Server
{
	// 룸의 아이템 사용 분기가 폭탄/수류탄/정화/시간 정지를 구분하는 Server 내부 분류다. NONE은 전투 아이템 효과가 없음을 뜻한다.
	enum class BATTLE_ITEM_KIND : std::uint8_t
	{
		NONE,
		DESTRUCTION,
		WHIRLWIND,
		CLEANSE,
		TIME_STOP
	};
	// BATTLEITEM 행의 값 복사본이다. GameRoom이 사용 가능 상태를 판정하고 CombatObjectRuntime이 투사체/피격 후보를 만든다.
	// 거리·높이는 cm, 속도는 cm/s, 지속·재사용 대기는 ms이며 소비자가 월드 m와 Server tick으로 변환한다.
	struct SERVER_BATTLE_ITEM_USE
	{
		BATTLE_ITEM_KIND eKind = BATTLE_ITEM_KIND::NONE;
		// skill ID는 피격/연출 식별에 쓰고 피해율은 공격 profile 공격력에 대한 백분율이다. 부위/무력화 수치는 피격 경로로 전달한다.
		std::uint32_t iSkillId = 0u, iDamageRatePercent = 0u, iPartDamage = 0u, iStaggerDamage = 0u;
		// 0이면 일반 무력화 수치를 사용한다. 회오리 수류탄은 3으로 고정되어 대상 최대 무력화의 1/3을 올림해 적용한다.
		std::uint32_t iStaggerMaximumDivisor = 0u;
		std::uint32_t iRangeCm = 0u, iRadiusCm = 0u, iDurationMs = 0u, iCooldownMs = 0u;
		std::uint32_t iProjectileSpeedCmPerSecond = 0u, iProjectileArcHeightCm = 0u, iProjectileLaunchHeightCm = 0u;
	};

	// Publish-ItemCatalog가 Data/Items/ItemCatalog.json에서 게시한 Items.bootstrap의 권위 아이템 정의다.
	// 지급뿐 아니라 소비·장착·구매·복원 검증도 사용하며 표시 이름/등급 이미지 등 Client presentation은 담지 않는다.
	struct SERVER_ITEM_DEFINITION
	{
		std::string strItemId;
		std::uint32_t iMaxStack = 0;
		// 1회 사용 시 최대 HP에서 회복할 비율(0~100)이다. 0인 전투 아이템도 있으므로 0만으로 비소비품을 판정하지 않는다.
		std::uint32_t iHealPercent = 0;
		SERVER_BATTLE_ITEM_USE BattleUse;
		// 장착 종류("helmet", "earring" 등)와 직업 이름("LanceMaster" 등)이다.
		// 게시 행의 "-"는 빈 문자열로 읽으며 장착/구매 경로가 현재 캐릭터와 대조한다.
		std::string strEquipSlot;
		std::string strCharacterClass;
		// carried 인벤토리 없는 초기 입장에서 이미 장착할 슬롯이다. NONE은 시작 장착 대상이 아님을 뜻한다.
		LostArk::Shared::EQUIPMENT_SLOT eStartingEquippedSlot = LostArk::Shared::EQUIPMENT_SLOT::NONE;
		// 상점 template의 직업 이름→실제 지급 아이템 ID다. 일반 아이템은 비어 있다.
		// 룸의 구매 판정이 구매자 직업으로 선택하며 template 자체는 장비/직업 바인딩을 갖지 않도록 Load가 검사한다.
		std::unordered_map<std::string, std::string> ClassVariants;
	};

	// 상점 재고 한 행의 아이템 ID·결제 화폐·1개 가격이다. 구매 요청량과 곱한 비용 검사는 GameRoom이 담당한다.
	struct SERVER_SHOP_ITEM
	{
		std::string strItemId;
		SERVER_CURRENCY eCurrency = SERVER_CURRENCY::SILVER;
		std::uint32_t iPrice = 0;
	};

	// GameRoom이 값으로 소유한다. 초기화 때 게시 파일을 읽고 룸 worker의 지급·사용·장착·구매·복원 경로가 조회한다.
	// 자체 잠금은 없으며 Load 중에는 다른 스레드의 조회가 없어야 한다. 플레이어별 인벤토리/소지금은 별도 SERVER_PLAYER 소유다.
	class CItemCatalog final
	{
	  public:
		// 기존 m_Items를 지역으로 옮긴 뒤 새 행을 m_Items에 읽으며 명시적 false 반환마다 이전 map을 복구한다.
		// 시작 장비·소지금·상점은 최종 검증 뒤 교체한다. 예외를 catch하는 전체 트랜잭션은 아니다.
		bool Load();

		// 아이템 ID의 비소유 읽기 포인터 또는 nullptr다. catalog 수명과 로드 완료 상태 안에서만 사용하며 재로드를 가로질러 보관하지 않는다.
		const SERVER_ITEM_DEFINITION* Find_Item(const std::string& itemId) const;

		// 최근 Load가 남긴 진단을 차용한다. 다음 Load에서 문구가 바뀔 수 있다.
		const std::string& Get_Status() const
		{
			return m_strStatus;
		}

		// 새 입장에 지급할 (아이템, 슬롯) 목록을 슬롯 enum 순서로 차용한다.
		// 이 단계는 같은 슬롯을 여러 행이 사용하는지 추가 검사하지 않는다.
		const std::vector<std::pair<std::string, LostArk::Shared::EQUIPMENT_SLOT>>& Get_StartingEquipment() const
		{
			return m_StartingEquipment;
		}
		// carried 인벤토리 없는 입장이 복사할 기본 소지금이다. CURRENCY 행이 없는 화폐는 기본값 0이다.
		const SERVER_PURSE& Get_StartingPurse() const
		{
			return m_StartingPurse;
		}

		// NPC placement ID로 상점을 찾고 그 안에서 아이템의 첫 재고 행을 차용한다.
		// NPC/상점/아이템 연결을 찾지 못하면 nullptr이며 실제 NPC 존재·거리·구매 가능 상태는 GameRoom이 검사한다.
		const SERVER_SHOP_ITEM* Find_ShopItem(const std::string& npcPlacementId, const std::string& itemId) const;

	  private:
		// 아이템 정의와 입장/상점 lookup 자료를 소유한다. 시작 장비·소지금·상점 자료는 행/참조 검증이 끝난 뒤 교체한다.
		std::unordered_map<std::string, SERVER_ITEM_DEFINITION> m_Items;
		std::vector<std::pair<std::string, LostArk::Shared::EQUIPMENT_SLOT>> m_StartingEquipment;
		SERVER_PURSE m_StartingPurse;
		std::unordered_map<std::string, std::string> m_ShopIdByNpcPlacementId;
		std::unordered_map<std::string, std::vector<SERVER_SHOP_ITEM>> m_ShopItemsByShopId;
		std::string m_strStatus;
	};
}
