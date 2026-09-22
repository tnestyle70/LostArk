"""Project a typed action post material onto the existing screenPost carrier.

The caller supplies an admitted screenPost element as a schema template and
exact native material/table evidence. No native parameter or resource fallback
is invented here. The returned receipt keeps explicit null texture overrides
and the source uniform expression that resolves each one.
"""
from pathlib import Path
import sys
import copy
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "LevelPlacementExtractor"))
from build_action_cue_recipe import decode_post_process_skill_envelope

def project(notify, material, element_id, group_id, texture_wires, material_input, template):
 d=decode_post_process_skill_envelope(notify['serializedPayload'],notify['assetReferences']);envelope=d['skillEnvelope']
 assert not envelope['onlyLocalPlayer'], 'Local-only post requires an owner gate'
 e=copy.deepcopy(template);e.update(id=element_id,displayName=notify['notifyId']+' / original post material',groupId=group_id,sourceNode=notify['notifyId'])
 e['material']=copy.deepcopy(material);profile=e['material']['sourceProfile']
 for name,field in [('scalarParameters','scalars'),('vectorParameters','vectors')]:
  index={x['name'].casefold():x for x in profile[field]}
  for x in d[name]:
   assert x['name'].casefold() in index,(element_id,name,x['name'])
   index[x['name'].casefold()]['value']=copy.deepcopy(x['value'])
 texture_receipt=[]
 for parameter in d['textureParameters']:
  name=parameter['name'].casefold();wires=[wire for wire in texture_wires if wire['name']==name];assert len(wires)==1,(element_id,name)
  wire=wires[0];source=parameter['sourceTexture'].casefold()
  if source:assert source==wire['sourceObjectPath'],(element_id,name,source,wire)
  else:
   expressions=material_input['materialMap']['uniformExpressionSet']['pixelTexture2DExpressions']
   expression=expressions[wire['index']];assert expression['typeName']=='fmaterialuniformexpressiontextureparameter' and expression['parameterName']==name
   fallback=material_input['effectiveTextures'][expression['referencedTextureIndex']]
   assert fallback['parameterName']==name and fallback['sourceObjectPath']==wire['sourceObjectPath'] and name not in material_input['textureOverrides']
  texture_receipt.append(dict(name=name,sourceOverride=source,resolvedSourceTexture=wire['sourceObjectPath'],nativeExpressionIndex=wire['index'],resolution='SOURCE_EXPLICIT_REFERENCE' if source else 'SOURCE_NATIVE_TEXTURE_PARAMETER_FALLBACK'))
 assert any(x['name']=='source_effect_opacity' for x in profile['scalars'])
 start=notify['localTimeSeconds'];fade_in=envelope['fadeInSeconds'];hold=envelope['playSeconds'];fade_out=envelope['fadeOutSeconds'];opacity=envelope['maxOpacity'];duration=fade_in+hold+fade_out
 e['detail']['timing'].update(startDelaySeconds=start,lifeTimeSeconds=duration,afterImageSeconds=0)
 e['detail']['particle']['randomSeed']=1
 e['sourceRecipe'].update(emitterDurationSeconds=duration)
 points=[(start,0.0 if fade_in else opacity)]
 if fade_in:points.append((start+fade_in,opacity))
 if hold:points.append((start+fade_in+hold,opacity))
 if fade_out:points.append((start+duration,0.0))
 keys=[dict(timeSeconds=t,value=[v],arriveTangent=[0.0],leaveTangent=[0.0],interpolation='linear') for t,v in points]
 track=e['sourceTransformTrack'];track.update(sourceOccurrenceId=notify['notifyId'],sourceTimeOriginSeconds=0)
 track['nodes'][0]['sourceObjectPath']=notify['notifyId'];track['materialParameterTracks']=[dict(name='source_effect_opacity',kind='SCALAR',keys=keys)]
 e['sourcePresentation'].update(sourceObjectPath=notify['notifyId'],sourceActionCueId=group_id,sourceEventId=notify['notifyId'],sourceOccurrenceIndex=0,sourceTimeSeconds=start)
 return e,dict(notifyId=notify['notifyId'],sourceSha256=d['sourceSha256'],sourceMaterial=d['sourceMaterial'],envelope=envelope,startSeconds=start,endSeconds=start+duration,textureResolution=texture_receipt,sourceTypedBlockEnd=d['sourceTypedBlockEnd'],remainingBytes=d['unresolvedTrailingByteCount'])
