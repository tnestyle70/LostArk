"""Source-wiring regressions for the F1 audition (not GPU/visual validation).

Run together with the product build, --colosseum-contract-test and the existing
build_colosseum_match_result_ui.py --validate asset validator. No Client is run.
"""
from pathlib import Path
import json
import re
import unittest

ROOT = Path(__file__).resolve().parents[2]


def source(path):
    return (ROOT / path).read_text(encoding="utf-8-sig")


def function(text, signature):
    start = text.index(signature)
    return text[start:text.index("\n}\n", start) + 3]


class PreviewContract(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.view = source("Client/Private/ColosseumMatchView.cpp")
        cls.level = source("Client/Private/Level_Development.cpp")
        cls.play = function(cls.view, "bool_t CColosseumMatchView::Play_DebugPreview(")
        cls.update = function(cls.view, "void CColosseumMatchView::Update_DebugPreview(")

    def test_debug_entry_is_typed_and_release_f1_keeps_direct_preview(self):
        lobby = source("Client/Private/Level_Lobby.cpp")
        self.assertRegex(lobby, r'#ifdef _DEBUG\s+if \(ImGui::Button\("Colosseum Preview"\)\)\s+CLobbyCommandService::Request\(LOBBY_STAGE::COLOSSEUM\);')
        resolve = function(lobby, 'bool_t CLevel_Lobby::Resolve_Stage(')
        self.assertIn('case LOBBY_STAGE::COLOSSEUM:', resolve)
        self.assertIn('outWorldId = WORLD_ID::COLOSSEUM;', resolve)
        self.assertNotIn('#ifdef _DEBUG', resolve)
        # The Release F1 destination is unmatched. It must never wait forever for a READY roster.
        def release_source(text):
            return re.sub(r'#ifdef _DEBUG\n[\s\S]*?#endif', '', text)
        release = release_source(function(self.level, 'void CLevel_Development::Update_ColosseumMatch('))
        preview = release[release.index('if (m_bColosseumPreview)'):release.index('const bool waiting')]
        self.assertIn('m_ColosseumLoading->Hide();', preview)
        self.assertIn('return;', preview)
        self.assertNotIn('Request_ColosseumLoadReady', preview)
        self.assertNotIn('Update_DebugPreview', preview)
        header = release_source(source('Client/Public/Level_Development.h'))
        self.assertIn('bool_t m_bColosseumPreview = false;', header)

    def test_entry_uses_queue_roster_not_late_snapshot_to_skip_loading(self):
        self.assertIn('m_bColosseumPreview = !CLevelTransitionService::Try_Get_ColosseumMatch(roster);', self.level)
        update = function(self.level, 'void CLevel_Development::Update_ColosseumMatch(')
        preview = update[update.index('if (m_bColosseumPreview)'):update.index('const bool waiting')]
        self.assertIn('m_ColosseumLoading->Hide();', preview)
        self.assertIn('Update_DebugPreview(deltaSeconds, m_Replication);', preview)
        self.assertIn('return;', preview)
        self.assertNotIn('Request_ColosseumLoadReady', preview)

    def test_unique_buttons_reuse_product_sampler(self):
        for label, kind in [('Play Victory Cutscene', 'VICTORY_CUTSCENE'),
                            ('Show Score HUD', 'SCORE_HUD'), ('Play Victory UI', 'VICTORY_UI'),
                            ('Play Defeat UI', 'DEFEAT_UI')]:
            self.assertEqual(self.level.count(f'ImGui::Button("{label}")'), 1)
            self.assertRegex(self.level, re.escape(f'ImGui::Button("{label}")') + r'[\s\S]*?DEBUG_PREVIEW::' + kind)
        self.assertIn('Sample_Presentation(replication, true);', self.view)
        self.assertIn('Sample_Presentation(replication, false);', self.update)
        sampler = function(self.view, 'void CColosseumMatchView::Sample_Presentation(')
        self.assertLess(sampler.index('if (!allowReturn) return;'), sampler.index('CUIPointerScope pointer(this);'))

    def test_invalid_input_preserves_previous_preview(self):
        commit = self.play.index('Stop_DebugPreview();')
        for condition in ['replication.Get_ColosseumMatchState().iMatchId',
                          'preview != DEBUG_PREVIEW::VICTORY_CUTSCENE', '!p.documentReady',
                          '!p.uiReady', 'local == players.end()', 'p.camera.expired()']:
            self.assertLess(self.play.index(condition), commit)
        self.assertGreaterEqual(self.play[:commit].count('return false;'), 4)
        self.assertIn('!std::isfinite(delta) || delta < 0.f', self.update)

    def test_actual_player_only_and_no_server_commands(self):
        self.assertIn('player.isLocal && !player.pCharacter.expired()', self.play)
        self.assertIn('participant.iPlayerId = local->iPlayerId;', self.play)
        self.assertIn('participant.iNetEntityId = local->iNetEntityId;', self.play)
        self.assertNotIn('staged.iMatchId =', self.play)
        for forbidden in ['Request_ColosseumReturn(', 'Request_ColosseumLoadReady(', 'Send_Message(', 'Change_Level(', 'Clone_GameObject(']:
            self.assertNotIn(forbidden, self.view)

    def test_repeat_stop_restores_actors_camera_and_hides_all_ui(self):
        stop = function(self.view, 'void CColosseumMatchView::Stop_DebugPreview(')
        for statement in ['p.End_Camera();', 'view->Set_AllSlotsVisible(false);',
                          'p.debugPreview = DEBUG_PREVIEW::NONE;', 'p.debugPaused = false;',
                          'p.showHud = p.showReturn = p.returnIntent = false;', 'p.state = {};']:
            self.assertIn(statement, stop)
        for statement in ['character->Clear_CutscenePoseOverride();', 'character->Clear_CutsceneAnimation();',
                          'character->Set_CinematicPresentationSuppressed(actor.previouslySuppressed);',
                          'End_PresentationOverride(VICTORY_CAMERA_OWNER)']:
            self.assertIn(statement, self.view)

    def test_live_match_supersedes_debug_preview(self):
        self.assertIn('replayAllowed = replayAllowed && active && active->m_bColosseumPreview;', self.level)
        self.assertIn('if (m_bColosseumPreview && state.iMatchId)', self.level)
        product = function(self.view, 'void CColosseumMatchView::Update(')
        self.assertIn('if (Is_DebugPreviewActive()) Stop_DebugPreview();', product)
        self.assertIn('if (replication.Get_ColosseumMatchState().iMatchId)', self.update)

    def test_pause_bounded_clock_and_automatic_cleanup(self):
        self.assertIn('if (!p.debugPaused)', self.update)
        self.assertIn('(std::min)(delta, .2f)', self.update)
        self.assertIn('p.debugClockMs >= duration', self.update)
        self.assertIn('p.debugClockMs = (std::min)(p.debugClockMs, duration);', self.update)
        self.assertIn('Stop_DebugPreview();', self.update)
        self.assertIn('ImGui::Button("Stop Preview")', self.level)

    def test_defeat_preview_uses_opposing_winner_without_network_mutation(self):
        self.assertIn('const bool defeat = preview == DEBUG_PREVIEW::DEFEAT_UI;', self.play)
        self.assertIn('staged.iWinningTeam = defeat ? 1u : 0u;', self.play)
        self.assertIn('staged.iLeftScore = defeat ? 1u : 3u;', self.play)
        self.assertIn('staged.iRightScore = defeat ? 3u : 1u;', self.play)
        self.assertIn('DEBUG_PREVIEW::DEFEAT_UI ? p.document.bannerDurations[1]', self.update)

    def test_unknown_local_team_is_not_reported_as_defeat(self):
        sampler = function(self.view, 'void CColosseumMatchView::Sample_Presentation(')
        self.assertIn('player.iPlayerId == participant.iPlayerId && player.iNetEntityId == participant.iNetEntityId', sampler)
        guard = sampler.index('state.iWinningTeam > 1u || p.localTeam > 1u')
        selection = sampler.index('p.bannerId = state.iWinningTeam')
        self.assertLess(guard, selection)
        self.assertIn('return;', sampler[guard:selection])
        self.assertIn('(p.localTeam == state.iWinningTeam ? "Result_Victory" : "Result_Defeat")', sampler)

    def test_native_defeat_last_frame_and_shared_ceremony_clock(self):
        camera = json.loads(source('Data/Camera/ColosseumVictory.cutscene.json'))
        lengths = []
        for name, count in [('Victory', 139), ('Defeat', 140), ('Draw', 140)]:
            animation = json.loads(source(f'Data/UI/Colosseum/Result_{name}.keyframes.json'))
            track = camera['bannerTitles'][name]
            self.assertEqual(animation['frameCount'], count)
            self.assertEqual(animation['frameRate'], track['frameRate'])
            self.assertEqual(len(track['keys']), count)
            duration = track['keys'][-1]['timeMs'] + 1000 / track['frameRate']
            self.assertEqual(duration, count * 1000 / animation['frameRate'])
            lengths.append(duration)
        self.assertEqual(lengths, [3475, 3500, 3500])
        self.assertIn('staged.bannerDuration = (std::max)(staged.bannerDuration, staged.bannerDurations[index]);', self.view)
        self.assertIn('p.sceneMs = p.resultMs - p.document.bannerDuration;', self.view)


if __name__ == '__main__':
    unittest.main(verbosity=2)
