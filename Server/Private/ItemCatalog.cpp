// 아이템/상점 bootstrap 로드와 읽기 조회의 구현에 공개 형식과 저장 멤버를 사용한다.
#include "ItemCatalog.h"

// 데이터 루트 환경 변수와 현재 실행 파일 경로를 UTF-16 Win32 API로 읽는다.
#include <Windows.h>

// ID 문자 전체 검사와 시작 장비의 슬롯 순서 정렬을 사용한다.
#include <algorithm>
// 아직 연결하지 않은 ITEMVARIANT 행을 template/class/variant 세 문자열로 보관한다.
#include <array>
// 정수 필드를 locale 없이 읽고 끝까지 소비했는지 검사한다.
#include <charconv>
// stable ID 문자를 현재 C locale의 영숫자 또는 허용 구두점으로 검사한다.
#include <cctype>
// 실행 파일/환경 변수 기반 데이터 루트와 Items.bootstrap 경로를 조합한다.
#include <filesystem>
// 게시 파일을 binary 입력 stream으로 열어 줄 단위로 검증한다.
#include <fstream>
// 현재 한 줄의 tab 필드를 복사 없이 참조한다.
#include <string_view>
// 행별 field view와 나중에 연결할 ITEMVARIANT 후보를 보관한다.
#include <vector>

namespace
{
	// 길이가 버퍼 안에 들어오는 LOSTARK_SERVER_DATA_ROOT를 우선하며 경로의 존재 여부는 여기서 검사하지 않는다.
	// 그 외에는 실행 파일 디렉터리의 부모 아래 DataFiles를 사용한다. 실행 파일 경로 조회 실패는 빈 경로다.
	std::filesystem::path Resolve_DataRoot()
	{
		wchar_t configured[32768]{};
		const DWORD configuredLength =
		    GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", configured, static_cast<DWORD>(std::size(configured)));
		if (0u != configuredLength && configuredLength < std::size(configured))
			return std::filesystem::path(configured).lexically_normal();

		wchar_t modulePath[32768]{};
		const DWORD moduleLength = GetModuleFileNameW(nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0u == moduleLength || moduleLength >= std::size(modulePath))
			return {};
		return std::filesystem::path(modulePath).parent_path().parent_path() / L"DataFiles";
	}

	// line의 tab 구간을 차용한다. 연속/마지막 tab의 빈 필드도 남기며 line을 다시 읽기 전에만 소비한다.
	std::vector<std::string_view> SplitTabs(const std::string& line)
	{
		std::vector<std::string_view> fields;
		const std::string_view view(line);
		std::size_t start = 0;
		while (true)
		{
			const std::size_t tab = view.find('\t', start);
			fields.push_back(view.substr(start, std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab)
				break;
			start = tab + 1;
		}
		return fields;
	}

	// binary getline 뒤 끝 CR 하나만 제거한다. 필드 공백과 tab을 trim하지 않는다.
	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
	}

	// from_chars가 숫자를 출력 인자에 직접 쓰고 전체 필드를 소비하면 true다.
	// 숫자 뒤 문자가 남는 false에서도 출력이 바뀔 수 있으므로 Load의 임시 값으로만 호출한다.
	template <typename T> bool ParseNumber(const std::string_view value, T& output)
	{
		const auto result = std::from_chars(value.data(), value.data() + value.size(), output);
		return std::errc{} == result.ec && result.ptr == value.data() + value.size();
	}

	// 게시 시작 장착 슬롯 이름을 공유 enum으로 바꾼다. "-"는 NONE이며 모르는 이름은 output을 건드리지 않고 false다.
	bool ParseStartingSlot(const std::string_view value, LostArk::Shared::EQUIPMENT_SLOT& output)
	{
		using LostArk::Shared::EQUIPMENT_SLOT;
		static constexpr std::pair<std::string_view, EQUIPMENT_SLOT> SLOTS[] = {
		    {"-", EQUIPMENT_SLOT::NONE},
		    {"helmet", EQUIPMENT_SLOT::HELMET},
		    {"shoulder", EQUIPMENT_SLOT::SHOULDER},
		    {"top", EQUIPMENT_SLOT::TOP},
		    {"pants", EQUIPMENT_SLOT::PANTS},
		    {"gloves", EQUIPMENT_SLOT::GLOVES},
		    {"weapon", EQUIPMENT_SLOT::WEAPON},
		    {"necklace", EQUIPMENT_SLOT::NECKLACE},
		    {"earring1", EQUIPMENT_SLOT::EARRING1},
		    {"earring2", EQUIPMENT_SLOT::EARRING2},
		    {"ring1", EQUIPMENT_SLOT::RING1},
		    {"ring2", EQUIPMENT_SLOT::RING2},
		    {"stone", EQUIPMENT_SLOT::STONE},
		    {"bracelet", EQUIPMENT_SLOT::BRACELET},
		};
		for (const auto& [name, slot] : SLOTS)
		{
			if (name == value)
			{
				output = slot;
				return true;
			}
		}
		return false;
	}

	// 아이템/상점/NPC 식별자를 1~64byte와 영숫자·밑줄·하이픈·마침표로 제한한다.
	// unsigned char로 isalnum에 전달하며 UTF-8 유효성이나 경로 안전성을 보증하는 함수는 아니다.
	bool IsStableId(const std::string_view value)
	{
		return !value.empty() && value.size() <= 64u &&
		       std::all_of(value.begin(), value.end(),
		                   [](const unsigned char character)
		                   {
			                   return 0 != std::isalnum(character) || character == '_' || character == '-' ||
			                          character == '.';
		                   });
	}
}

// GameRoom 초기화가 Items.bootstrap schema 7을 읽는다. 예전 아이템 map을 보관하고 새 아이템은 멤버 map에 구성한다.
// 명시적 검증 실패는 이전 아이템 map과 나머지 기존 자료를 보존하고 진단만 바꾼다. 할당/파일시스템 예외는 여기서 catch하지 않는다.
bool LostArk::Server::CItemCatalog::Load()
{
	// 현재 map을 지역으로 옮겨 모든 false 경로가 복구할 수 있게 한다. 이 구간에는 m_Items가 비어 있거나 부분 후보이므로 동시 조회하면 안 된다.
	using ITEM_MAP = decltype(m_Items);
	ITEM_MAP previousItems = std::move(m_Items);
	m_Items.clear();
	std::unordered_map<std::string, std::string> shopIdByNpc;
	std::unordered_map<std::string, std::vector<SERVER_SHOP_ITEM>> shopItems;
	// ITEM 행의 위치와 무관하게 연결할 (template, class, variant) 문자열을 복사한다. 모든 ITEM을 읽은 뒤 ID 간 관계를 검사한다.
	std::vector<std::array<std::string, 3>> variantRows;
	SERVER_PURSE startingPurse{};
	// CURRENCY/SHOPITEM의 대문자 화폐 이름만 받는다. 알 수 없는 값은 output을 유지한 채 false다.
	const auto parseCurrency = [](const std::string_view value, SERVER_CURRENCY& output)
	{
		if ("SILVER" == value)
		{
			output = SERVER_CURRENCY::SILVER;
			return true;
		}
		if ("GOLD" == value)
		{
			output = SERVER_CURRENCY::GOLD;
			return true;
		}
		return false;
	};

	// 게시 파일을 열고 실패하면 구성 중인 map을 이전 map으로 되돌린다. 다른 멤버 자료는 아직 수정하지 않았다.
	const std::filesystem::path dataRoot = Resolve_DataRoot();
	const std::filesystem::path path = dataRoot / L"Items" / L"Items.bootstrap";
	std::ifstream input(path, std::ios::binary);
	if (dataRoot.empty() || !input)
	{
		m_strStatus = "Missing item bootstrap: " + path.string();
		m_Items = std::move(previousItems);
		return false;
	}

	// 헤더의 magic·3필드·행 수 1~4096을 확인한 다음 version 7을 별도로 검사해 재게시 안내를 남긴다.
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
	if (3u != header.size() || "LOSTARK_ITEM_BOOTSTRAP" != header[0] || !ParseNumber(header[1], version) ||
	    !ParseNumber(header[2], rowCount) || 0u == rowCount || rowCount > 4096u)
	{
		m_strStatus = "Item bootstrap header is invalid: " + path.string();
		m_Items = std::move(previousItems);
		return false;
	}

	if (7u != version)
	{
		m_strStatus = "Item bootstrap version mismatch: expected 7, got " + std::to_string(version) +
		              "; path=" + path.string() +
		              "; run powershell -ExecutionPolicy Bypass -File "
		              "Tools/GameplayPipeline/Publish-ItemCatalog.ps1 -Mode Publish";
		m_Items = std::move(previousItems);
		return false;
	}

	// 선언된 행을 순서대로 읽는다. fields는 현재 line을 참조하므로 멤버/후속 검사에는 문자열을 복사한다.
	// 각 종류의 필드 수가 맞지 않으면 마지막 ITEM 검증 경로에서 형식 오류가 된다.
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
		// CURRENCY <SILVER|GOLD> <startingAmount>는 시작 소지금이다. 같은 화폐가 다시 나오면 마지막 행 값이 앞선 값을 덮는다.
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
		// SHOPNPC <shopId> <npcPlacementId>는 NPC→상점 연결이다. 같은 NPC placement ID를 두 번 선언하면 거부한다.
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
		// SHOPITEM <shopId> <itemId> <SILVER|GOLD> <price>는 양수 가격의 재고 행이다.
		// 같은 상품의 중복 행은 여기서 거부하지 않으며 조회는 저장된 순서에서 첫 행을 돌려준다.
		if (5u == fields.size() && "SHOPITEM" == fields[0])
		{
			SERVER_SHOP_ITEM stock{};
			if (!IsStableId(fields[1]) || !IsStableId(fields[2]) || !parseCurrency(fields[3], stock.eCurrency) ||
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
		// BATTLEITEM은 앞서 읽은 ITEM만 확장한다. 같은 아이템의 두 효과·회복 겸용·장비 겸용을 거부한다.
		// 고정 네 ID와 효과 종류, 수치 상한·필수 거리/지속시간, 회오리 최대 무력화 분모 3 계약을 함께 검사한다.
		if (15u == fields.size() && "BATTLEITEM" == fields[0])
		{
			auto item = m_Items.find(std::string(fields[1]));
			SERVER_BATTLE_ITEM_USE use;
			if (fields[2] == "DESTRUCTION" && fields[1] == "BATTLE_DESTRUCTION_BOMB")
				use.eKind = BATTLE_ITEM_KIND::DESTRUCTION;
			if (fields[2] == "WHIRLWIND" && fields[1] == "BATTLE_WHIRLWIND_GRENADE")
				use.eKind = BATTLE_ITEM_KIND::WHIRLWIND;
			if (fields[2] == "CLEANSE" && fields[1] == "BATTLE_HOLY_CHARM")
				use.eKind = BATTLE_ITEM_KIND::CLEANSE;
			if (fields[2] == "TIME_STOP" && fields[1] == "BATTLE_TIME_STOP_POTION")
				use.eKind = BATTLE_ITEM_KIND::TIME_STOP;
			if (item == m_Items.end() || use.eKind == BATTLE_ITEM_KIND::NONE ||
			    item->second.BattleUse.eKind != BATTLE_ITEM_KIND::NONE || item->second.iHealPercent ||
			    !item->second.strEquipSlot.empty() || !ParseNumber(fields[3], use.iSkillId) || !use.iSkillId ||
			    !ParseNumber(fields[4], use.iDamageRatePercent) || use.iDamageRatePercent > 1000000u ||
			    !ParseNumber(fields[5], use.iPartDamage) || use.iPartDamage > 1000000u ||
			    !ParseNumber(fields[6], use.iStaggerDamage) || use.iStaggerDamage > 1000000u ||
			    !ParseNumber(fields[7], use.iRangeCm) || use.iRangeCm > 5000u ||
			    !ParseNumber(fields[8], use.iRadiusCm) || use.iRadiusCm > 5000u ||
			    !ParseNumber(fields[9], use.iDurationMs) || use.iDurationMs > 600000u ||
			    !ParseNumber(fields[10], use.iCooldownMs) || use.iCooldownMs < 1000u || use.iCooldownMs > 600000u ||
			    ((use.eKind == BATTLE_ITEM_KIND::DESTRUCTION || use.eKind == BATTLE_ITEM_KIND::WHIRLWIND) &&
			     (!use.iRangeCm || !use.iRadiusCm)) ||
			    (use.eKind == BATTLE_ITEM_KIND::TIME_STOP && use.iRangeCm) ||
			    ((use.eKind == BATTLE_ITEM_KIND::CLEANSE || use.eKind == BATTLE_ITEM_KIND::TIME_STOP) &&
			     !use.iDurationMs) ||
			    !ParseNumber(fields[11], use.iProjectileSpeedCmPerSecond) || use.iProjectileSpeedCmPerSecond > 10000u ||
			    !ParseNumber(fields[12], use.iProjectileArcHeightCm) || use.iProjectileArcHeightCm > 1000u ||
			    !ParseNumber(fields[13], use.iProjectileLaunchHeightCm) || use.iProjectileLaunchHeightCm > 1000u ||
			    !ParseNumber(fields[14], use.iStaggerMaximumDivisor) ||
			    (use.eKind == BATTLE_ITEM_KIND::WHIRLWIND
			         ? (use.iStaggerMaximumDivisor != 3u || use.iDamageRatePercent || use.iStaggerDamage)
			         : use.iStaggerMaximumDivisor != 0u) ||
			    ((use.eKind == BATTLE_ITEM_KIND::DESTRUCTION || use.eKind == BATTLE_ITEM_KIND::WHIRLWIND) &&
			     !use.iProjectileSpeedCmPerSecond))
			{
				m_strStatus = "Item bootstrap BATTLEITEM row is invalid";
				m_Items = std::move(previousItems);
				return false;
			}
			item->second.BattleUse = use;
			continue;
		}
		// ITEMVARIANT <templateId> <className> <itemId>는 class별 실제 지급 ID다.
		// 지금은 문자열 형식만 검사하고 아직 읽지 않은 ITEM도 참조할 수 있도록 연결 검사를 뒤로 미룬다.
		if (4u == fields.size() && "ITEMVARIANT" == fields[0])
		{
			if (!IsStableId(fields[1]) || fields[2].empty() || !IsStableId(fields[3]))
			{
				m_strStatus = "Item bootstrap ITEMVARIANT row is invalid";
				m_Items = std::move(previousItems);
				return false;
			}
			variantRows.push_back({std::string(fields[1]), std::string(fields[2]), std::string(fields[3])});
			continue;
		}
		// 기본 ITEM은 ID·양수 stack·HP 회복율·선택 필드·시작 슬롯을 검사한다.
		// 장착 종류/직업 문자열의 의미 전체를 여기서 검증하지 않으며 게시 도구와 실제 장착 경로의 검증도 필요하다.
		SERVER_ITEM_DEFINITION item{};
		if (7u != fields.size() || "ITEM" != fields[0] || !IsStableId(fields[1]) ||
		    !ParseNumber(fields[2], item.iMaxStack) || 0u == item.iMaxStack ||
		    !ParseNumber(fields[3], item.iHealPercent) || item.iHealPercent > 100u || fields[4].empty() ||
		    fields[5].empty() || !ParseStartingSlot(fields[6], item.eStartingEquippedSlot))
		{
			m_strStatus = "Item bootstrap row is invalid";
			m_Items = std::move(previousItems);
			return false;
		}
		item.strItemId = fields[1];
		// 선택 장착 종류/직업 필드의 "-"는 값 없음이다. 값이 있으면 게시 문자열을 그대로 소유한다.
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

	// 선언한 수 뒤의 추가 행은 빈 줄도 거부한다. 이 검사는 stream의 EOF와 I/O 실패를 별도로 구분하지 않는다.
	if (std::getline(input, line))
	{
		m_strStatus = "Item bootstrap has trailing rows";
		m_Items = std::move(previousItems);
		return false;
	}

	// 모든 ITEM이 모인 뒤 template/variant가 존재하는지, template이 비장비·비직업 바인딩인지,
	// variant가 해당 직업 장비인지와 template 안의 직업 중복을 검사한다. 게시 도구의 모든 검사를 대신하지는 않는다.
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

	// 상점 재고의 아이템 참조가 실제 catalog에 있고 NPC에 연결된 상점에 재고 행이 있는지 검사한다.
	// 실제 월드 NPC placement의 존재와 구매 거리/화폐 잔액은 룸의 구매 명령 경로가 확인한다.
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

	// 명시적인 false 검증은 모두 끝났다. 시작 장비를 새 map에서 모아 슬롯 순서로 정렬하고 소지금·상점을 교체한다.
	// 이후 할당 중 예외에는 기존 자료를 돌려놓는 guard가 없어 새 아이템 map과 일부 갱신된 시작 장비가 남을 수 있다.
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

// 지급·사용·장착·복원/상점 검증의 읽기 조회다. 반환 포인터는 catalog의 멤버이며 호출자가 해제하지 않는다.
const LostArk::Server::SERVER_ITEM_DEFINITION* LostArk::Server::CItemCatalog::Find_Item(const std::string& itemId) const
{
	const auto iter = m_Items.find(itemId);
	return m_Items.end() == iter ? nullptr : &iter->second;
}

// 구매 검증이 NPC placement→상점→아이템 순서로 조회한다. 누락은 nullptr, 중복 재고는 첫 행이며 상태를 변경하지 않는다.
const LostArk::Server::SERVER_SHOP_ITEM* LostArk::Server::CItemCatalog::Find_ShopItem(const std::string& npcPlacementId,
                                                                                      const std::string& itemId) const
{
	const auto shop = m_ShopIdByNpcPlacementId.find(npcPlacementId);
	if (m_ShopIdByNpcPlacementId.end() == shop)
		return nullptr;
	const auto stock = m_ShopItemsByShopId.find(shop->second);
	if (m_ShopItemsByShopId.end() == stock)
		return nullptr;
	for (const SERVER_SHOP_ITEM& line : stock->second)
		if (line.strItemId == itemId)
			return &line;
	return nullptr;
}
