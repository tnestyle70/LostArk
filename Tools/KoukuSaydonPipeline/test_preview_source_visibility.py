"""Compile the production NPC visibility methods; check preview lifecycle wiring separately."""
import os
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[2]


class PreviewSourceVisibility(unittest.TestCase):
    def test_native_visibility_counter(self):
        text=(ROOT/'Client/Public/Npc.h').read_text(encoding='utf8')
        names=('Set_PresentationVisible','Acquire_CompositionPreviewSuppression',
               'Release_CompositionPreviewSuppression','Is_PresentationVisible')
        methods=[]
        for name in names:
            matches=re.findall(r'^\s*(?:void|bool) '+name+r'\([^\n]*$',text,re.M)
            self.assertEqual(len(matches),1,name)
            methods.append(matches[0].strip())
        fixture='struct Visibility { bool m_bPresentationVisible=true; unsigned m_iCompositionPreviewSuppressions=0;\n'+'\n'.join(methods)+'\n};\n'
        fixture+='''int main() {
          Visibility v;
          if (!v.Is_PresentationVisible()) return 1;
          v.Acquire_CompositionPreviewSuppression();
          if (v.Is_PresentationVisible()) return 2;
          v.Acquire_CompositionPreviewSuppression();
          v.Release_CompositionPreviewSuppression();
          if (v.Is_PresentationVisible()) return 3;
          v.Set_PresentationVisible(false);
          v.Release_CompositionPreviewSuppression();
          if (v.Is_PresentationVisible()) return 4;
          v.Set_PresentationVisible(true);
          if (!v.Is_PresentationVisible()) return 5;
          v.Release_CompositionPreviewSuppression();
          if (!v.Is_PresentationVisible()) return 6;
          for (unsigned i=0;i<10000;++i) {
            v.Acquire_CompositionPreviewSuppression();
            v.Release_CompositionPreviewSuppression();
          }
          return v.Is_PresentationVisible() ? 0 : 7;
        }'''
        vs=Path(os.environ.get('VSINSTALLDIR','C:/Program Files/Microsoft Visual Studio/2022/Community'))
        with tempfile.TemporaryDirectory(prefix='showtime-visibility-') as temp:
            source=Path(temp)/'visibility.cpp'
            source.write_text(fixture,encoding='ascii')
            setup=vs/'Common7/Tools/VsDevCmd.bat'
            command=f'call "{setup}" -arch=x64 -host_arch=x64 >nul && cl /nologo /EHsc visibility.cpp /Fe:visibility.exe && visibility.exe'
            result=subprocess.run(command,shell=True,cwd=temp,capture_output=True)
            self.assertEqual(result.returncode,0,result.stdout.decode(errors='replace')+result.stderr.decode(errors='replace'))

    def test_production_lifecycle_wiring(self):
        text=(ROOT/'Client/Private/KoukuSaydonPresentationPlayer.cpp').read_text(encoding='utf8')
        release=text[text.index('void Client::CKoukuSaydonPresentationPlayer::Release_BundlePreviewMembers('):text.index('bool Client::CKoukuSaydonPresentationPlayer::Prepare_CloneSplitPreview(')]
        self.assertIn('source->Release_CompositionPreviewSuppression();',release)
        begin=text[text.index('bool Client::CKoukuSaydonPresentationPlayer::Begin_BundlePreview('):text.index('bool Client::CKoukuSaydonPresentationPlayer::Begin_ModelReferencePreview(')]
        self.assertLess(begin.index('Stop_Preview();'),begin.index('Sync_PreviewSourceVisibility();'))
        self.assertLess(begin.index('m_bPreviewPlaying ='),begin.index('Sync_PreviewSourceVisibility();'))
        self.assertLess(begin.index('Sync_PreviewSourceVisibility();'),begin.index('if (!externalWorldPreview) Sample_BundlePreview();'))
        sync=text[text.index('void Client::CKoukuSaydonPresentationPlayer::Sync_PreviewSourceVisibility()'):text.index('void Client::CKoukuSaydonPresentationPlayer::Sample_BundlePreview()')]
        for guard in ('!member.finiteActorLifetime','!boss.iOwnerBossNetEntityId','boss.strArchetypeId == member.sourceArchetypeId','previous == replacement'):
            self.assertIn(guard,sync)
        npc=(ROOT/'Client/Private/Npc.cpp').read_text(encoding='utf8')
        self.assertIn('if (!Is_PresentationVisible()) return;',npc)
        self.assertIn('if (!Is_PresentationVisible()) return S_OK;',npc)


if __name__=='__main__': unittest.main()
