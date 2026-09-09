from pathlib import Path
import argparse,json,copy,collections,sys
from materialize_artist_31470_portable_particle_carriers import (
 SOURCE_ONLY_RECIPE_FIELDS, SOURCE_ONLY_DISTRIBUTION_FIELDS)
R=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser(description='Assemble Artist full.restore documents from recovered source candidates and admitted native material contracts.')
parser.add_argument('--source-dir',type=Path,default=R/'out/ArtistCoreRestore20260909')
O=parser.parse_args().source_dir.resolve()
read=lambda p:json.loads(p.read_text(encoding='utf8'));native=read(O/'native_material_patch.json')['programs'];programs=read(O/'native_runtime_contract.json')['programs'];lookup={eid:m['material']for m in native for eid in m['occurrences']};prog={eid:p for p in programs for eid in p['occurrences']};geometry={r['source']:r for r in read(O/'mesh_cook_receipt.json')if r['status']=='COOKED_NATIVE_GEOMETRY'};sourceinfo={i['target']:i for i in read(O/'source_projection.json')['projected']};selections={r['elementId']:r for r in read(O/'selected_runtime_material_programs.json')['occurrences']};rows=[];excluded=[];vectorfields={r['sourceObjectPath']:r['assetId']for r in read(O/'vector_field_receipt.json')}
# These source IDs are joined by the existing F compiler's stable renderer /
# emitter digest. Their ordinary Ribbon, LocalDecal and weapon carriers already
# consume RuntimeMaterialV2; preserve that path instead of losing them when a
# particle-only VF or new native prefix is unavailable.
f_portable_ids={
 'fx_pc_sdm_07.par_v_sdm_ink_spw_01.particlespriteemitter_0':'ribbon.a6fe27caa16b2630',
 'fx_pc_sdm_07.par_v_smd_onestroke_weapon_01.particlespriteemitter_6':'mesh.cc04feee8a36940b',
 'fx_pc_sdm_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_43':'decal.f3b5c3b63b4a7e34',
 'fx_pc_sdm_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_6':'decal.6f78bff02c657a14',
}
f_portable={e['id']:e for e in read(R/'Data/Effects/Authored/effect.artist.skill.31470.unified.effect.json')['elements']}
reused=[]
def project_authored_document(document):
 # Native-v14 is read-only source evidence. Full restore uses the existing
 # ordinary-v13 carriers, including RuntimeMaterialV2, so keep the evidence in
 # the input receipts and retain only fields consumed by those carriers.
 document['version']=13
 document.pop('purpose',None)
 for element in document['elements']:
  element.pop('renderer',None)
  recipe=element.get('sourceRecipe',{})
  for field in SOURCE_ONLY_RECIPE_FIELDS:recipe.pop(field,None)
  for module in recipe.get('modules',[]):
   for distribution in module.get('distributions',[]):
    if distribution.get('parameterBinding','none')!='none' or distribution.get('parameterName',''):
     raise ValueError('Unresolved source distribution binding: '+element['id'])
    for field in SOURCE_ONLY_DISTRIBUTION_FIELDS:distribution.pop(field,None)
 return document

for p in (O/'candidate').glob('*.json'):
 d=read(p);sid=int(d['effectAssetId'].split('.')[3]);elements=[]
 for e in d['elements']:
  key=f'skill.{sid}:'+e['id'];mat=lookup.get(key);s=sourceinfo[e['id']]
  portable_id=f_portable_ids.get(s['sourceEmitter']) if sid==31470 else None
  if portable_id:
   carrier=copy.deepcopy(f_portable[portable_id])
   assert carrier['visible'] and carrier['material']['execution']['enabled']
   assert carrier['material']['sourceMaterialPath'].lower()==e['material']['sourceMaterialPath'].lower()
   carrier['sourceNode']=e['sourceNode'];elements.append(carrier)
   reused.append(dict(document=d['effectAssetId'],sourceElementId=s['sourceElementId'],id=portable_id,contract='EXISTING_RUNTIME_MATERIAL_V2_CARRIER'));continue
  blocked=[m for m in e['sourceRecipe'].get('modules',[]) if m['className'] in ['particlemoduletypedataribbon','particlemodulelocationemitter','efparticlemodulelocationemitter']]
  if blocked:
   excluded.append(dict(document=d['effectAssetId'],sourceElementId=s['sourceElementId'],sourceEmitter=s['sourceEmitter'],sourceMaterial=e['material']['sourceMaterialPath'],id=e['id'],reason='SOLO_REQUIRES_LIVE_SIBLING_PARTICLE_PROVIDER' if any(m['className'] in ['particlemodulelocationemitter','efparticlemodulelocationemitter']for m in blocked)else'SOURCE_RIBBON_HISTORY_GEOMETRY_NOT_SUPPORTED',modules=blocked));continue
  orbits=[m for m in e['sourceRecipe'].get('modules',[]) if m['className']=='particlemoduleorbit']
  if len(orbits)>1 and any(l['propertyPath']=='chainmode' and l['value']=='eochainmode_link' for m in orbits for l in m['literals']):
   excluded.append(dict(document=d['effectAssetId'],sourceElementId=s['sourceElementId'],sourceEmitter=s['sourceEmitter'],sourceMaterial=e['material']['sourceMaterialPath'],id=e['id'],reason='SOURCE_MULTI_MODULE_ORBIT_LINK_NOT_SUPPORTED',modules=orbits));continue
  for module in e['sourceRecipe'].get('modules',[]):
   if module['className']=='particlemodulelocalvectorfield':
    source=next(l['value']for l in module['literals']if l['propertyPath']=='vectorfield.objectpath');module['literals'].append(dict(propertyPath='vectorfield.assetid',kind='string',value=vectorfields[source]))
  if e['kind']=='light' and e['detail']['light']['enabled'] and e['sourcePresentation'].get('status')=='source_exact':
   e['material']={'templateId':'effect.standard','sourceMaterialPath':'','renderProfile':'alpha_two_sided_depth_read','sourceProfile':{'enabled':False}};e['resources']=[];elements.append(e);continue
  if mat is None:
   status=selections.get(key,{}).get('status','NATIVE_INPUT_UNCLOSED');status=('SOURCE_ENGINE_PREFIX_AND_VERTEX_INPUTS_UNCLOSED' if status=='EXACT_MAP_AND_VF_SELECTED' else status);excluded.append(dict(document=d['effectAssetId'],sourceElementId=s['sourceElementId'],sourceEmitter=s['sourceEmitter'],sourceMaterial=e['material']['sourceMaterialPath'],id=e['id'],reason=status));continue
  e['material']=copy.deepcopy(mat);e['resources']=[];pr=prog[key]
  if pr['rendererShape']=='staticMesh':
   e['resources']=[dict(slotId='meshModel',assetId='Effect/Artist/Meshes/Native/LV_MATTE/sky_mirror_sm.wmodel')]
  if pr['rendererShape']=='mesh':
   td=next(m for m in e['sourceRecipe'].get('modules',[])if m['className']=='particlemoduletypedatamesh');source=next(l['value']for l in td['literals']if l['propertyPath']=='mesh.objectpath');g=geometry[source];e['resources']=[dict(slotId='meshModel',assetId=g['assetId'])];e['detail']['mesh'].update(useModelMaterial=False,modelPreScale=g['modelPreScale'])
  if pr['rendererShape']=='screenPost':
   source_material=e['material']['sourceMaterialPath'].lower()
   profile='screen.film-noise.reconstructed.v1' if 'filmnoise' in source_material else ('screen.motion-blur.reconstructed.v1' if 'motionblur' in source_material else 'screen.zoom-blur.reconstructed.v1')
   e['kind']='screenPost';e['detail']['screenPost'].update(enabled=True,profileId=profile,status='reconstructed_profile',intensity=1,secondaryIntensity=0,frequency=1,tint=[1,1,1,1],randomSeed=1)
  e['sourcePresentation']={'enabled':False};elements.append(e)
 d['elements']=elements
 # Prepared native model lifecycle/material contract is kept while particle candidates iterate.
 output=R/'Data/Effects/Authored'/p.name
 if sid in [31490,31950] and output.exists():d['modelCues']=read(output)['modelCues']
 if sid==31480:d['modelCues']=read(R/'Data/Effects/Authored/effect.artist.skill.31480.unified.effect.json')['modelCues']
 project_authored_document(d)
 output.write_text(json.dumps(d,ensure_ascii=False,indent=2),encoding='utf8');rows.append(dict(path=str(output),elements=len(elements),models=len(d['modelCues']),shapes=dict(collections.Counter(e['sourceRecipe'].get('rendererShape',e['kind'])for e in elements))))
(O/'full_restore_admission.json').write_text(json.dumps(dict(documents=rows,excluded=excluded,reusedPortableCarriers=reused),ensure_ascii=False,indent=2),encoding='utf8');print(rows);print('excluded',collections.Counter(x['reason']for x in excluded))

# glTF exports carry millisecond ticks. CAnimation's cooked contract is 30 ticks
# per second, so preserve wall-clock timing by retiming duration and every key.
sys.path.insert(0,str(R/'Tools/ActorXAssetCooker'))
import retime_wmodel_ticks as tick_retime
for row in rows:
 for cue in read(Path(row['path']))['modelCues']:
  if cue['modelAssetId'] not in ('Effect/Artist/Models/SK_SDM_TIG_00/sk_sdm_tig_00_sk.wmodel','Effect/Artist/Models/SK_SDM_DRA_00/sk_sdm_dra_00_sk.wmodel'):continue
  model=R/'Client/Bin/Resources'/cue['modelAssetId'];data=tick_retime.read_bounded(model)
  changes=tick_retime.retime(data,30.0,None);tick_retime.verify(data,changes,30.0)
  if model.read_bytes()!=bytes(data):tick_retime.write_atomically(model,bytes(data))
  (O/(cue['cueId']+'.tick-retime.json')).write_text(json.dumps(dict(assetId=cue['modelAssetId'],clips=changes),indent=2),encoding='utf8')
