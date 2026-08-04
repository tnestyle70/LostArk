#include "Logic_Dimensionist.h"

#include "Character.h"

namespace
{
	/* TownDefaultWeapon 106600901 resolves to WP_WSWP_M_06. Its LookInfo splits
	the weapon into four meshes and gives each one a distinct battle socket. The
	E piece carries the original RelativeRotationYaw=32768 (180 degrees). */
	constexpr WEAPON_PART_SPEC Weapons[] =
	{
		{ TEXT("Part_90_Weapon_L"),
		  TEXT("Prototype_Component_Model_Dimensionist_Weapon_L"),
		  "b_wp_swm_m_1" },
		{ TEXT("Part_91_Weapon_S"),
		  TEXT("Prototype_Component_Model_Dimensionist_Weapon_S"),
		  "b_wp_swm_m_2" },
		{ TEXT("Part_92_Weapon_P"),
		  TEXT("Prototype_Component_Model_Dimensionist_Weapon_P"),
		  "b_wp_swm_m_3" },
		{ TEXT("Part_93_Weapon_E"),
		  TEXT("Prototype_Component_Model_Dimensionist_Weapon_E"),
		  "b_wp_swm_m_4_02", 180.f },
	};

	unique_ptr<ICharacterLogic> Create_Logic()
	{
		return make_unique<Client::CLogic_Dimensionist>();
	}
}

NS_BEGIN(Client)

void CLogic_Dimensionist::Update_Presentation(
	CCharacter& Character,
	f32_t fTimeDelta)
{
	(void)Character;
	(void)fTimeDelta;
}

const CHARACTER_SPEC Spec_Dimensionist =
{
	"Dimensionist",
	LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONIST,

	TEXT("Prototype_Component_Model_Dimensionist"),
	TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
	0u,

	TEXT("Prototype_Component_Shader_VtxMeshBinary"),
	Weapons,
	static_cast<uint32_t>(size(Weapons)),

	nullptr,
	0u,

	{
		"pc_sp_m_00_sk_idle_battle_1",
		"pc_sp_m_00_sk_run_battle_1",
		"pc_sp_m_00_sk_dmg_idle_1",
		"pc_sp_m_00_sk_dead_1",
	},

	&Create_Logic,
};

NS_END
