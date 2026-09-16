"""Map the already supported PBR, foliage and ice source expressions.

This maps surface inputs only. Wind/deformation remains an explicit separate
vertex-program dependency and is recorded by the caller.
"""

def build_surface(source, terminal, values, switches, texture, *, asset_id, material_name):
    v,s=values,switches
    enabled=lambda name:bool(s.get('1.'+name,s.get(name,False)))
    row=dict(assetId=asset_id,materialName=material_name,sourceMaterial=source,
             castsShadow=True,renderMode='deferred',cullMode='back')
    spaces=row['textureColorSpace']={}
    def tex(field,key,slot):
        path,space=texture(key);row[field]=path;spaces[slot]=space
    def copy(fields):
        for field,key in fields.items():row[field]=v[key]
    def emission(flicker, *, pbr=False):
        path,space=texture('texture_emissive')
        row['emissive']=dict(texture=path,color=v['emissive_color'],intensity=v['emissive_intensity'],
            uvTiling=(v['emissive_uv_tiling'] if pbr else v.get('emissive_uv_tiling',[1,1]))[:2],colorSpace=space,
            flicker=dict(minimum=v['emissive_intensitymin'] if flicker else (0 if pbr else 1),
                         speed=v['emissive_flicker_speed'] if flicker else 0,phaseOffset=0))
        if pbr and not flicker: row['emissive']['flicker']['mode']='none'
    if terminal.endswith(('.bg_base_pbr_opa','.bg_base_pbr_seamless_opa','.bg_base_pbr_msk')):
        allowed={'1.'+k for k in ['use_reflection','use_normalmap','use_diffuse_saturation','use_diffuse_to_albedo','use_uv_tiling','use_detail_normal','use_worldreflection','uv_fixed_normal','use_emissive','use_flicker','use_diffuse_metallicmask']}
        if terminal.endswith('.bg_base_pbr_msk'):
            allowed.add('is_blend_masked')

            if enabled('use_uv_tiling'):
                raise ValueError('masked source UV branch requires its own native evidence')
        unsupported={k for k,x in s.items() if x}-allowed
        if unsupported:raise ValueError(f'{source}: unsupported PBR switches {unsupported}')

        if not enabled('use_diffuse_to_albedo'):
            raise ValueError(f'{source}: PBR branch without diffuse-to-albedo is unsupported')
        if not enabled('use_normalmap'):
            raise ValueError(f'{source}: PBR branch without normalmap requires separate source shader evidence')
        row['family']=terminal.rsplit('.',1)[-1]
        copy({'diffuseBrightness':'diffuse_brightness','normalIntensity':'normal_intensity',
            'reflectionContrast':'reflection_contrast','diffuseColor':'diffuse_color','reflectionColor':'reflection_color',
            'metallicIntensity':'metallic_intensity','metallicPower':'metallic_power',
            'roughnessIntensity':'roughness_intensity','roughnessPower':'roughness_power',
            'aoIntensity':'ao_intensity','aoPower':'ao_power','specularPBRIntensity':'specular_pbr_intensity',
            'nonmetallicBrightness':'nonmetallic_brightness','metallicBrightness':'metallic_brightness'})
        row.update(reflectionIntensity=v['reflection_intensity'] if enabled('use_reflection') else 0,
            reflectionTiling=v['reflection_tiling'] if enabled('use_worldreflection') else 1,
            diffuseSaturation=v['diffuse_saturation'] if enabled('use_diffuse_saturation') else 1,
            detailNormalIntensity=v['detail_normal_intensity'] if enabled('use_detail_normal') else 0,
            detailNormalTiling=v['detail_normal_tiling'] if enabled('use_detail_normal') else 1,
            uvFixedNormal=enabled('uv_fixed_normal'),useWorldReflection=enabled('use_worldreflection'),
            minimumRoughness=.04,vertexAlpha=1,reflectionOriginOffset=[0,0])
        tile=v['uv_tiling'] if enabled('use_uv_tiling') else 1
        row['uvTiling']=[tile,tile*v.get('uv_tiling_y_scale',1) if enabled('use_uv_tiling') else 1]
        for field,key,slot in [('diffuseTexture','texture_diffuse','diffuse'),('normalTexture','texture_normal','normal'),('detailNormalTexture','texture_detail_normal','detailNormal'),('ormTexture','texture_orm','orm'),('reflectionTexture','texture_reflection','reflection')]:tex(field,key,slot)
        if enabled('use_diffuse_metallicmask'):
            # Native PBR PS lerps the two diffuse branches by metallic before the
            # common nonmetallic/metallic multiplier. Admit only the exact fold:
            # matching tint, no saturation/reflection, and equal outer multipliers.
            # Other combinations need extra carrier inputs and remain unsupported.
            mask_color=v['metallicmask_diffuse_color']
            mask_brightness=v['metallicmask_diffuse_brightness']
            if (row['diffuseSaturation'] != 1 or enabled('use_reflection') or
                enabled('use_diffuse_saturation_metallicmask') or
                mask_color != row['diffuseColor'] or
                row['nonmetallicBrightness'] != row['metallicBrightness']):
                raise ValueError(f'{source}: metallic-mask diffuse cannot be folded exactly')
            common=row['nonmetallicBrightness']
            row['nonmetallicBrightness']=common*row['diffuseBrightness']
            row['metallicBrightness']=common*mask_brightness
            row['diffuseBrightness']=1
        if enabled('use_emissive'):emission(enabled('use_flicker'),pbr=True)
        return row
    if terminal.endswith(('.bg_foliage_msk','.bg_grass_msk')):
        grass=terminal.endswith('.bg_grass_msk')
        allowed={'1.'+k for k in ['use_dynamicfoliage(wind)','use_specular','use_transmission','use_normalmap','use_specular_texture','use_diffuse_saturation','use_emissive','use_emissive_texture','use_flicker']}|{'is_blend_masked'}
        unsupported={k for k,x in s.items() if x}-allowed
        if unsupported:raise ValueError(f'{source}: unsupported foliage switches {unsupported}')
        flags=sum(bit for key,bit in [('use_normalmap',1),('use_diffuse_saturation',2),('use_specular',4),('use_specular_texture',8),('use_transmission',16),('use_emissive',32),('use_flicker',64)] if enabled(key))
        row.update(family='bg-source-grass-masked' if grass else 'bg-source-foliage-masked',sourceFlags=flags,
            transmissionColor=[x*v.get('transmission_intensity',1) for x in v['transmission_color'][:3]]+[1],
            normalIntensity=v.get('normal_intensity',1),diffuseSaturation=v['diffuse_saturation'] if flags&2 else 1)
        copy({'diffuseBrightness':'diffuse_brightness','specularIntensity':'specular_intensity',
            'specularPower':'specular_power','diffuseColor':'diffuse_color','specularColor':'specular_color'})
        spaces.update(diffuse='linear',normal='linear',specular='linear',mask='linear')
        tex('diffuseTexture','texture_diffuse','diffuse')
        if flags&1:tex('normalTexture','texture_normal','normal')
        if flags&8:tex('specularTexture','texture_specular','specular')
        if not grass:tex('maskTexture','texture_foliagemask','mask')
        if flags&32:emission(bool(flags&64))
        return row
    if terminal.endswith('.preset_overlay_snowice_opa'):
        allowed={'1.use_detail_normal','1.use_specular_texture'}
        unsupported={k for k,x in s.items() if x}-allowed
        if unsupported:raise ValueError(f'{source}: unsupported ice switches {unsupported}')
        flags=(1 if enabled('use_detail_normal') else 0)|(2 if enabled('use_specular_texture') else 0)
        row.update(family='bg-source-snowice-opaque',sourceSpecialFlags=flags,
            diffuseBrightness=1,specularColor=[1,1,1,1],uvTiling=[v['diffuse_uv_tiling']]*2,
            detailNormalIntensity=v['detail_normal_intensity'] if flags&1 else 0,
            detailNormalTiling=v['detail_normal_tiling'] if flags&1 else 1,
            sourceIceBlend=[v['icemask_tiling'],v['ice_blending'],v['overlay_sharpeness'],v['ice_emissiveboost']])
        copy({'diffuseColor':'diffuse_color','normalIntensity':'normal_intensity','specularIntensity':'specular_intensity',
            'specularPower':'specular_power','reflectionColor':'iceenviroment_color','sourceNormalTiling':'normal_tiling',
            'sourceIceCoreColor':'ice_corecolor','sourceIceOuterColor':'ice_outercolor','sourceIceBumpOffset':'diffuse_bumpoffset'})
        for field,key,slot in [('diffuseTexture','texture_diffuse','diffuse'),('normalTexture','texture_normal','normal'),('specialMaskTexture','texture_icemask','specialMask'),('reflectionTexture','texture_iceenviroment','reflection')]:tex(field,key,slot)
        if flags&1:tex('detailNormalTexture','texture_detail_normal','detailNormal')
        if flags&2:tex('specularTexture','texture_specular','specular')
        return row
    raise ValueError(f'{source}: unsupported terminal {terminal}')
