"""Non-visual source/data wiring regression; actual pixels are user-reviewed."""
import json
import math
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

    def test_eight_selected_slots_have_complete_hud_rows(self):
        doc = json.loads((ROOT / 'Data/UI/Colosseum/CombatHUD_Layout.json').read_text())
        ids = {slot['id'] for slot in doc['slots']}
        for arrival in range(8):
            prefix = ('Left' if arrival % 2 == 0 else 'Right') + str(arrival // 2)
            self.assertTrue({prefix + '_' + part for part in
                             ('BG', 'HP', 'Frame', 'Name', 'Health', 'Kills', 'Number')} <= ids)

    def test_four_person_lineup_and_winner_framing(self):
        # Independent frustum check of all camera samples, feet through 2.5m body height.
        # This checks data geometry only; actual models, occlusion and pixels need user review.
        for filename in ('ColosseumIntro.cutscene.json', 'ColosseumVictory.cutscene.json'):
            doc = json.loads((ROOT / 'Data/Camera' / filename).read_text())
            if 'teams' in doc:
                self.assertEqual(len(doc['rows']), 4)
                self.assertEqual([len(team) for team in doc['teams'].values()], [4, 4])
                positions = [(p['x'], doc['floorY'], p['z'])
                             for team in doc['teams'].values() for p in team]
            else:
                self.assertEqual(len(doc['actors']), 4)
                positions = [p['position'] for p in doc['actors']]
            for shot in doc['shots']:
                for key in shot['keys']:
                    forward = key.get('forward', shot['forward'])
                    length = math.sqrt(sum(v * v for v in forward))
                    forward = [v / length for v in forward]
                    right = [-forward[2], 0, forward[0]]
                    up = [-forward[0] * forward[1], forward[0] ** 2 + forward[2] ** 2,
                          -forward[2] * forward[1]]
                    length = math.sqrt(sum(v * v for v in up))
                    up = [v / length for v in up]
                    for position in positions:
                        for height in (0, 2.5):
                            relative = [v + (height if axis == 1 else 0) - key['eye'][axis]
                                        for axis, v in enumerate(position)]
                            depth = sum(a * b for a, b in zip(relative, forward))
                            self.assertGreater(depth, 0)
                            width = depth * math.tan(math.radians(doc['fovXDegrees']) / 2)
                            self.assertLess(abs(sum(a * b for a, b in zip(relative, right))), width)
                            self.assertLess(abs(sum(a * b for a, b in zip(relative, up))),
                                            width / doc['letterboxAspect'])

    def test_project_registration(self):
        for file in ('Client.vcxproj', 'Client.vcxproj.filters'):
            self.assertEqual(source('Client/Default/' + file).count('Colosseum\\CombatHUD_Layout.json'), 1)


if __name__ == '__main__':
    unittest.main()
