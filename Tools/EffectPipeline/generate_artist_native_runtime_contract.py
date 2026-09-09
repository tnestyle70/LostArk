from pathlib import Path
import argparse,json,hashlib,re,shutil
R=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser(description='Generate Artist material contracts and descriptors from the native runtime source receipt.')
parser.add_argument('--source-dir',type=Path,default=R/'out/ArtistCoreRestore20260909')
parser.add_argument('--extend-existing',action='store_true',
    help='Update only the supplied program IDs and preserve the existing Artist shader groups and material contracts.')
arguments=parser.parse_args()
O=arguments.source_dir.resolve()
rows=json.loads((O/'native_runtime_contract.json').read_text())['programs'];q=lambda x:json.dumps(x,ensure_ascii=False)
source=(R/'Client/Public/Effect_DimensionMasterWRMaterial.h').read_text();head=source[:source.index('inline constexpr std::array<std::string_view,2>')];head=head.replace('DIMENSIONMASTER_WR','ARTIST').replace('DimensionMasterWR','Artist').replace('W/R unlit','Artist source')
head=head.replace('    bool bMesh;','    bool bMesh;\n    bool bModelCue;')
materials=[]
for p in rows:
 i=p['program'];parent=p['parentMaterial'];slug=re.sub('[^a-z0-9]+','.',parent.lower()).strip('.');pid='ue3.material.'+slug[:72]+'.'+hashlib.sha256(parent.encode()).hexdigest()[:12];p['profileId']=pid
 blend='ADDITIVE' if p['nativeBlend']=='blend_additive'else 'ALPHA';render=blend+('_TWO_SIDED_DEPTH_READ'if p['nativeTwoSided']else '_ONE_SIDED_DEPTH_READ');render='OPAQUE_BACK_DEPTH_WRITE' if p['nativeBlend']=='blend_opaque'and not p['nativeTwoSided']else render;p['renderProfile']=render
 head+=f'inline constexpr std::array<std::string_view,{len(p["textures"])}> ARTIST_TEXTURES_{i} = {{{{'+','.join(q(t['name'])for t in p['textures'])+'}};\n'
 head+=f'inline constexpr std::array<ARTIST_PARAMETER_DESC,{len(p["parameters"])}> ARTIST_PARAMETERS_{i} = {{{{\n'+''.join('    {'+q(v['name'])+f', {v["row"]}u, {v["lane"] or 0}u, '+str(v['kind']=='vector').lower()+'},\n'for v in p['parameters'])+'}};\n'
 sw=p['staticSwitches'];head+=f'inline constexpr std::array<ARTIST_SWITCH_DESC,{len(sw)}> ARTIST_SWITCHES_{i} = {{{{\n'+''.join('    {'+q(t['parameterName'])+', '+str(t['value']).lower()+'},\n'for t in sw)+'}};\n'
 sp=dict(enabled=True,profileId=pid,runtimeShaderProfileId=p['runtimeShaderProfileId'],parentMaterialPath=parent,semanticStatus='reconstructed_profile',textures=[{k:t[k]for k in ['name','sourceObjectPath','assetId','addressU','addressV','colorSpace','samplingEvidence']}for t in p['textures']],scalars=[dict(name=v['name'],group='None',value=v['effective'])for v in p['parameters']if v['kind']=='scalar'],vectors=[dict(name=v['name'],group='None',value=v['effective'])for v in p['parameters']if v['kind']=='vector'],staticSwitches=[dict(name=t['parameterName'],group='None',value=t['value'])for t in sw],dynamicParameterSemantics=['unbound']*4,subUVMode='none')
 materials.append(dict(program=i,occurrences=p['occurrences'],material=dict(templateId='effect.source_material',sourceMaterialPath=p['sourceMaterial'],renderProfile=render.lower(),sourceProfile=sp)))
head+=f'inline constexpr std::array<ARTIST_PROGRAM_DESC,{len(rows)}> ARTIST_PROGRAMS = {{{{\n'
for p in rows:
 i=p['program'];head+='    {'+f'{i}u,'+','.join(q(p[k])for k in ['runtimeShaderProfileId','sourceMaterial','parentMaterial','profileId'])+','+str(p['rendererShape'] in ['mesh','staticMesh']).lower()+','+str(p['modelCue']).lower()+','+q(p['rendererShape'])+','+','.join(str(x).lower()for x in [p['requiresSceneColor'],p['requiresDepthSample'],p['requiresTangentView'],'dynamicparameter'in p['sourceVF']])+',EFFECT_RENDER_PROFILE::'+p['renderProfile']+f',ARTIST_TEXTURES_{i},ARTIST_PARAMETERS_{i},ARTIST_SWITCHES_{i}'+'},\n'
head+='}};\n\n';tail=source[source.index('inline const DIMENSIONMASTER_WR_PROGRAM_DESC*'):];tail=tail.replace('DIMENSIONMASTER_WR','ARTIST').replace('DimensionMasterWR','Artist');tail=tail.replace('if (!Program || Element.Material.Execution.bEnabled','if (!Program || Program->bModelCue || Element.Material.Execution.bEnabled')

old='        Element.eKind!=(Program->strRendererShape=="screenPost" ? EFFECT_ELEMENT_KIND::SCREEN_POST : EFFECT_ELEMENT_KIND::PARTICLE) || !Element.SourceRecipe.bEnabled ||\n        Element.Material.strSourceMaterialPath!=Program->strSourceMaterialPath ||\n        Element.SourceRecipe.strRendererShape!=Program->strRendererShape) return false;'
new='        Element.Material.strSourceMaterialPath!=Program->strSourceMaterialPath) return false;\n    const bool bStaticAction = Program->strRendererShape == "staticMesh";\n    if (bStaticAction)\n    {\n        const auto& Attachment = Element.ActionCueAttachment;\n        if (Element.eKind != EFFECT_ELEMENT_KIND::MESH || Element.SourceRecipe.bEnabled ||\n            !Attachment.bEnabled || Attachment.bFollow ||\n            Attachment.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::BONE ||\n            Attachment.strSourceAnchorSlotId != "EffectRoot" || Attachment.strRuntimeAnchorSlotId != "root" ||\n            !Attachment.strRuntimeBoneName.empty() || Attachment.fSnapshotRootSourceBasisYawDegrees != -90.f ||\n            Element.Detail.Mesh.bUseModelMaterial || Element.Detail.Mesh.fModelPreScale != .01f) return false;\n        if (std::count_if(Element.ResourceBindings.begin(),Element.ResourceBindings.end(),[](const auto& B)\n            { return B.strSlotId=="meshModel" && B.strAssetId=="Effect/Artist/Meshes/Native/LV_MATTE/sky_mirror_sm.wmodel"; }) != 1) return false;\n    }\n    else if (Element.eKind!=(Program->strRendererShape=="screenPost" ? EFFECT_ELEMENT_KIND::SCREEN_POST : EFFECT_ELEMENT_KIND::PARTICLE) ||\n        !Element.SourceRecipe.bEnabled || Element.SourceRecipe.strRendererShape!=Program->strRendererShape) return false;'
assert old in tail;tail=tail.replace(old,new)

a=tail.index('NS_END');tail=tail[:a]+'''inline bool Has_ArtistModelCueMaterialContract(const EFFECT_MODEL_CUE_DESC& Cue)
{
    if (!Cue.Material) return false;
    const auto& Material=*Cue.Material;
    const auto& Source=Material.SourceMaterial;
    const auto* Program=Find_ArtistProgram(Source.strRuntimeShaderProfileId);
    if (!Program || !Program->bModelCue || Material.Execution.bEnabled || Material.Execution.bFailClosed ||
        Material.eRenderProfile!=Program->eRenderProfile || Material.strSourceMaterialPath!=Program->strSourceMaterialPath ||
        Source.Textures.size()!=Program->TextureNames.size() || Source.StaticSwitches.size()!=Program->StaticSwitches.size()) return false;
    const std::string_view expected=Program->iProfileIndex==460u ?
        "Effect/Artist/Models/SK_SDM_TIG_00/sk_sdm_tig_00_sk.wmodel" :
        "Effect/Artist/Models/SK_SDM_DRA_00/sk_sdm_dra_00_sk.wmodel";
    if (Cue.strModelAssetId!=expected) return false;
    for (const auto name: Program->TextureNames)
        if(std::count_if(Source.Textures.begin(),Source.Textures.end(),[&](const auto& T){return T.strName==name&&!T.strAssetId.empty()&&!T.strSourceObjectPath.empty();})!=1) return false;
    for (const auto& S:Program->StaticSwitches)
        if(std::count_if(Source.StaticSwitches.begin(),Source.StaticSwitches.end(),[&](const auto& V){return V.strName==S.strName&&V.bValue==S.bValue;})!=1) return false;
    std::array<float4_t,32> Parameters{};
    return Build_ArtistParameters(Source,Parameters);
}
NS_END
'''
header_path=R/'Client/Public/Effect_ArtistMaterial.h'
if arguments.extend_existing:
    existing=header_path.read_text(encoding='utf8')
    supplied='(?:'+'|'.join(str(p['program']) for p in rows)+')'
    existing=re.sub(r'inline constexpr std::array<(?:std::string_view|ARTIST_PARAMETER_DESC|ARTIST_SWITCH_DESC),\d+> ARTIST_(?:TEXTURES|PARAMETERS|SWITCHES)_'+supplied+r' = \{\{.*?\}\};\n','',existing,flags=re.S)
    existing=re.sub(r'^    \{'+supplied+r'u,.*\n','',existing,flags=re.M)
    marker='inline constexpr std::array<ARTIST_PROGRAM_DESC,'
    start=existing.index(marker); end=existing.index('}};',start)+3
    old_entries=existing[existing.index('\n',start)+1:end-3]
    generated_start=head.index(marker)
    new_arrays=head[head.index('inline constexpr std::array<std::string_view,'):generated_start]
    new_entries=head[head.index('\n',generated_start)+1:head.index('}};',generated_start)]
    entries=old_entries+new_entries
    count=sum(line.startswith('    {') for line in entries.splitlines())
    final=existing[:start]+new_arrays+f'inline constexpr std::array<ARTIST_PROGRAM_DESC,{count}> ARTIST_PROGRAMS = {{{{\n'+entries+'}};'+existing[end:]
else:
    final=head+tail
    shutil.copy2(O/'Shader_EffectArtistNative.hlsli',R/'Client/Bin/ShaderFiles/Shader_EffectArtistNative.hlsli')
header_path.write_text(final,encoding='utf8')
(O/'native_material_patch.json').write_text(json.dumps(dict(programs=materials),indent=2),encoding='utf8')
(O/'native_header_contract.json').write_text(json.dumps(rows,indent=2))
print('header/models',len(rows),'extendExisting',arguments.extend_existing)

