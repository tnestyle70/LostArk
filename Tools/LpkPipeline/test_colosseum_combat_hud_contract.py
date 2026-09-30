"""Non-visual source/data wiring regression; actual pixels are user-reviewed."""
import json
import unittest
from pathlib import Path
from build_colosseum_combat_ui import validate
from test_colosseum_debug_preview_contract import source, function

ROOT = Path(__file__).resolve().parents[2]


class CombatHUDContract(unittest.TestCase):
    def test_all_four_player_hud_gates_admit_colosseum(self):
        app = source('Client/Private/MainApp.cpp')
        for name in ('Update_CombatHUD', 'RenderCombatHUDText', 'RenderQuickSlotKeyLabels', 'RenderSkillCooldownText'):
            self.assertIn('LEVEL::COLOSSEUM', function(app, f'void CMainApp::{name}('))

    def test_source_images_and_layout(self):
        validate(ROOT)
        doc = json.loads((ROOT / 'Data/UI/Colosseum/CombatHUD_Layout.json').read_text())
        self.assertEqual(doc['resolution'], {'width': 1280, 'height': 720})
        for slot in doc['slots']:
            r = slot['rect']
            self.assertLessEqual(r['x'] + r['width'], 1280)
            self.assertLessEqual(r['y'] + r['height'], 720)

    def test_hp_uses_exact_replicated_identity_not_estimated_full_health(self):
        view = source('Client/Private/ColosseumMatchView.cpp')
        self.assertIn('item.iPlayerId == row.iPlayerId && item.iNetEntityId == row.iNetEntityId', view)
        self.assertIn('replication.Get_PlayerHealth().Find(row.iNetEntityId)', view)
        self.assertIn('std::clamp(health.Get_Ratio(), 0.f, 1.f)', view)
        self.assertIn('!health.hasSnapshot ? L"..."', view)

    def test_kill_feed_bounded_age_and_no_local_scoring(self):
        view = source('Client/Private/ColosseumMatchView.cpp')
        self.assertIn('state.RecentKills.rbegin()', view)
        self.assertIn('slot < 3u', view)
        self.assertIn('age >= 6.0 * TICKS_PER_SECOND', view)
        self.assertNotIn('RecentKills.push_back', view)
        self.assertNotIn('iKills++', view)

    def test_no_feed_outside_play_and_sprite_lifetime(self):
        view = source('Client/Private/ColosseumMatchView.cpp')
        self.assertIn('combatText.clear();', view)
        self.assertIn('combat->Set_AllSlotsVisible(false);', view)
        self.assertIn('if (!showHud || !combatReady) return;', view)
        stop = function(view, 'void CColosseumMatchView::Stop_DebugPreview()')
        self.assertIn('p.combat.get()', stop)
        self.assertIn('m_Impl->combat.get()', view[view.index('CColosseumMatchView::~'):])

    def test_edge_anchor_geometry_for_wide_and_standard_screens(self):
        # Independent numeric expectation for the expression used by the consumer.
        for w, h in ((1280, 720), (1920, 1080), (3440, 1440), (1280, 1024)):
            scale = h / 720 * 1280 / w
            left = 8 * scale * w / 1280
            right = (1280 - (1280 - 1088) * scale) * w / 1280
            self.assertAlmostEqual(left, 8 * h / 720)
            self.assertAlmostEqual(w - right, 192 * h / 720)
        view = source('Client/Private/ColosseumMatchView.cpp')
        self.assertIn('left ? rect.x * xScale : 1280.f - (1280.f - rect.x) * xScale', view)

    def test_project_registration(self):
        for file in ('Client.vcxproj', 'Client.vcxproj.filters'):
            self.assertEqual(source('Client/Default/' + file).count('Colosseum\\CombatHUD_Layout.json'), 1)


if __name__ == '__main__':
    unittest.main()
