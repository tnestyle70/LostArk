"""Restore the Esther Wei cameo from its summons sequence 532100."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/EffectPipeline'))
import build_esther_bahuntur_source_effects as sequence
import build_esther_inanna_source_effects as drv

sequence.EVIDENCE = ROOT / 'out/WeiFX20260924'
sequence.ARCHETYPE = 'NPC_58700'
sequence.ASSET_PREFIX = 'effect.esther.wei.cameo.'
sequence.CLIP = 'npc_sk_dochul'
sequence.CLIP_SECONDS = 7.1
sequence.CAMEO_MESH = 'SK_Dochul'
sequence.SOURCE_MESH = ('np_dpwi_00', 'mesh.np_dpwi_00_dead_sk')
sequence.TEXTURE_ROOT = 'Effect/Esther/Wei/FullRestore/Textures'
sequence.PROJECTILE = sequence.PROJECTILE.with_name('532100.loa')
sequence.NATIVE_FIRST = 4684
sequence.SKIPPED_SYSTEMS = ('fx_cm_02.light.par_mp_light_01',)

configure = sequence.configure


def configure_wei():
    configure()
    drv.MESH_ROOTS = ('Effect/Esther/Wei/Meshes',) + tuple(r for r in drv.MESH_ROOTS if r != 'Effect/Esther/Wei/Meshes')
    drv.SILIAN_REVIEWED = ROOT / 'out/BahunturFX20260924/material/reviewed'


sequence.configure = configure_wei

if __name__ == '__main__':
    sequence.main()
