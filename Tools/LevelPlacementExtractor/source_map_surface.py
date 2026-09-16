"""Compile resolved source material values into the existing map surface schema.

The caller owns exact MIC/static-set resolution, texture export and installation.
No source object is inferred from its leaf name.
"""
import math


def build_surface(source, terminal, values, switches, texture, *, asset_id, material_name):
    v, s = values.copy(), switches
    simple = terminal.endswith(('.bg_simple_opa', '.bg_simple_msk'))
    if simple:
        # The source simple graph has no tint, overlay, bump, UV or reflection
        # expressions. These schema values are inactive carrier defaults.
        v.update(specular_color=[1, 1, 1, 1], uv_tiling=1,
                 bump_offset=0, bump_intensity=0, bump_brightness=1,
                 reflection_contrast=0.5, reflection_tiling=1,
                 reflection_color=[1, 1, 1, 1], reflection_intensity=0)
    enabled = lambda name: bool(s.get('1.' + name, s.get(name, False)))
    row = dict(assetId=asset_id, materialName=material_name, sourceMaterial=source,
               castsShadow=True, renderMode='deferred', cullMode='back')
    spaces = row['textureColorSpace'] = {}

    def tex(field, parameter, slot=None):
        path, color_space = texture(parameter)
        row[field] = path
        if slot:
            spaces[slot] = color_space
        return path

    def scalar(field, parameter, feature=None, inactive=None):
        row[field] = v[parameter] if feature is None or enabled(feature) else inactive

    def copy(fields):
        for field, parameter in fields.items():
            scalar(field, parameter)

    def emission():
        path, space = texture('texture_emissive')
        flicker = enabled('use_flicker') or enabled('use_flicker_linear')
        row['emissive'] = dict(texture=path, color=v['emissive_color'], intensity=v['emissive_intensity'],
            uvTiling=v.get('emissive_uv_tiling', [1, 1])[:2], colorSpace=space,
            flicker=dict(minimum=v['emissive_intensitymin'] if flicker else 0,
                         speed=v['emissive_flicker_speed'] if flicker else 0, phaseOffset=0))

    opaque_bg = simple or terminal.endswith(('.bg_base_opa', '.bg_base_msk', '.bg_seamless-specular_opa'))
    if opaque_bg:
        active = {k for k, value in s.items() if value}
        supported = {'1.' + name for name in (
            'use_normalmap', 'use_specular', 'use_specular_texture', 'use_reflection',
            'use_worldreflection', 'use_detail_normal', 'use_diffuse_saturation',
            'use_uv_tiling', 'uv_fixed_normal', 'use_emissive', 'use_flicker',
            'use_subspecular', 'use_rimlight', 'use_overlay', 'use_vertexcolor_paint',
            'use_overlay_normal', 'use_overlay_direction', 'invert_overlay',
            'use_bump', 'use_uv_panning', 'use_uv_rotate', 'use_uv_moving',
            'use_flicker_linear', 'use_specular_saturation')}
        supported |= {'is_blend_masked', 'is_blend_opaque'}
        unsupported = active - supported
        if unsupported:
            raise ValueError(f'{source}: unhandled static switches {sorted(unsupported)}')
        copy({'diffuseBrightness': 'diffuse_brightness', 'normalIntensity': 'normal_intensity',
              'diffuseColor': 'diffuse_color', 'specularIntensity': 'specular_intensity',
              'specularPower': 'specular_power', 'specularColor': 'specular_color'})
        scalar('diffuseSaturation', 'diffuse_saturation', None if simple else 'use_diffuse_saturation', 1)
        tile = v['uv_tiling'] if enabled('use_uv_tiling') else 1
        row['uvTiling'] = [tile, tile * v.get('uv_tiling_y_scale', 1) if enabled('use_uv_tiling') else 1]
        angle = v.get('uv_rotate', 0) * 2 * math.pi if enabled('use_uv_rotate') else 0
        row['sourceUV'] = [math.sin(angle), math.cos(angle),
            v.get('uv_move_x', 0) if enabled('use_uv_moving') else 0,
            v.get('uv_move_y', 0) if enabled('use_uv_moving') else 0]
        tex('diffuseTexture', 'texture_diffuse', 'diffuse')
        if enabled('use_overlay'):
            row['family'] = 'bg_base_opa_overlay'
            flags = 0
            unsupported_overlay = active & {'1.' + name for name in (
                'use_reflection', 'use_worldreflection', 'use_rimlight',
                'use_uv_panning', 'use_flicker', 'use_flicker_linear')}
            if unsupported_overlay:
                raise ValueError(f'{source}: unhandled overlay switches {sorted(unsupported_overlay)}')
            for feature, bit in [('use_normalmap', 1), ('use_overlay_normal', 2),
                ('use_vertexcolor_paint', 4), ('use_overlay_direction', 8), ('invert_overlay', 16),
                ('use_detail_normal', 32), ('uv_fixed_normal', 128), ('use_specular', 256), ('use_bump', 512)]:
                if enabled(feature): flags |= bit
            if enabled('is_blend_masked') or terminal.endswith('.bg_base_msk'): flags |= 64
            if not enabled('use_vertexcolor_paint'): flags |= 1024
            row['sourceOverlayFlags'] = flags
            direction = v['overlay_direction']
            row['sourceDirection'] = [direction[0], direction[2], -direction[1], v['overlay_amount']]
            copy({'overlayColor':'overlay_color', 'overlayTiling':'overlay_tiling',
                  'overlayNormalIntensity':'overlay_normal_intensity', 'overlaySharpness':'overlay_sharpeness',
                  'overlayBrightness':'overlay_brightness', 'overlaySaturation':'overlay_saturation',
                  'overlaySpecularIntensity':'overlay_specular_intensity'})
            spaces.update(normal='linear', overlayNormal='linear')
            tex('overlayDiffuseTexture', 'texture_overlay_diffuse', 'overlayDiffuse')
            if flags & 1: tex('normalTexture', 'texture_normal', 'normal')
            if flags & 2: tex('overlayNormalTexture', 'texture_overlay_normal', 'overlayNormal')
            if enabled('use_specular_texture'): tex('specularTexture', 'texture_specular', 'specular')
            if enabled('use_subspecular'):
                row['sourceSubspecular'] = [v['subspecular_intensity'], v['subspecular_power']]
            if enabled('use_specular_saturation'):
                row['sourceSpecularSaturation'] = v['specular_saturation']
            if enabled('use_bump'):
                row['sourceBump'] = [v['bump_offset'], v['bump_intensity'], v['bump_brightness'], 0]
            if enabled('use_emissive'): emission()

        else:
            row['family'] = 'bg-source-opaque-masked'
            flags = 260 if simple else 128
            for feature, bit in [('use_normalmap',1), ('use_bump',2), ('use_specular',4),
                ('use_specular_texture',8), ('use_reflection',16), ('use_worldreflection',32),
                ('is_blend_masked',64), ('uv_fixed_normal',16384), ('use_detail_normal',32768)]:
                if enabled(feature): flags |= bit
            if terminal.endswith('bg_seamless-specular_opa'): flags |= 512
            if terminal.endswith(('.bg_base_msk', '.bg_simple_msk')): flags |= 64
            row.update(sourceFlags=flags, sourceBump=[v['bump_offset'],v['bump_intensity'],v['bump_brightness'],0],
                       flickerMode=2 if enabled('use_flicker_linear') else (1 if enabled('use_flicker') else 0), addressU='WRAP')
            spaces.update(normal='linear', specular='linear', reflection='linear')
            for field, param, slot, bit in [('normalTexture','texture_normal','normal',1),
                ('specularTexture','texture_specular','specular',8), ('reflectionTexture','texture_reflection','reflection',16)]:
                if flags & bit: tex(field,param,slot)
            copy({'reflectionContrast':'reflection_contrast','reflectionTiling':'reflection_tiling','reflectionColor':'reflection_color'})
            scalar('reflectionIntensity','reflection_intensity','use_reflection',0)
            row['reflectionOriginOffset'] = [0,0]
            if enabled('use_subspecular'):
                row['sourceSubspecular'] = [v['subspecular_intensity'],v['subspecular_power']]
            if enabled('use_rimlight'):
                row['sourceRimlight'] = [x*v['rimlight_intensity'] for x in v['rimlight_color'][:3]]+[v['rimlight_power']]
            row['sourceSpecularSaturation'] = v.get('specular_saturation',1) if enabled('use_specular_saturation') else 1
            if enabled('use_uv_panning'): row['sourcePanning'] = v['uv_panning_speed'][:2]
            if enabled('use_emissive'): emission()
        if enabled('use_detail_normal'):
            tex('detailNormalTexture','texture_detail_normal')
            copy({'detailNormalIntensity':'detail_normal_intensity','detailNormalTiling':'detail_normal_tiling'})
        return row
    raise ValueError(f'{source}: unsupported terminal {terminal}')
