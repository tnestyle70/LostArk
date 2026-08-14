uint g_ReconstructedMaterialEvaluatorEnabled = 0;
uint g_ReconstructedMaterialFeatureMask = 0;
float2 g_ReconstructedUVScale = float2(1.f, 1.f);
float4 g_ReconstructedPanRotationAux = float4(0.f, 0.f, 0.f, 0.f);
float4 g_ReconstructedColor = float4(1.f, 1.f, 1.f, 1.f);
float4 g_ReconstructedParams0 = float4(0.f, 0.f, 0.f, 0.f);
float4 g_ReconstructedParams1 = float4(0.f, 0.f, 0.f, 0.f);

/*
 * Artist visual-program v4 is an occurrence-to-capability adapter.  It does
 * not change the shared source-material profile ABI in Shader_EffectCommon.
 * A non-zero opcode owns the complete RT0/RT1 projection for the occurrence.
 */
uint g_ArtistVisualV4Opcode = 0u;
uint g_ArtistVisualV4TextureMask = 0u;
float4 g_ArtistVisualV4Params[8];
float4 g_ArtistVisualV4Colors[2];

static const uint ARTIST_VISUAL_V4_BASIC_MISSILETRAIL = 1u;
static const uint ARTIST_VISUAL_V4_MAKEFLOW = 2u;
static const uint ARTIST_VISUAL_V4_COMPLEX_MISSILETRAIL = 3u;
static const uint ARTIST_VISUAL_V4_DISTORTION_ONLY = 4u;
static const uint ARTIST_VISUAL_V4_UNSUPPORTED_SUPPRESSED = 5u;
static const uint ARTIST_VISUAL_V4_SPLA = 6u;
static const uint ARTIST_VISUAL_V4_FLOW02_RECOVERED_EQUATION = 7u;
static const uint ARTIST_VISUAL_V4_SKULL_RECOVERED_EQUATION = 8u;

/*
 * V4 lanes own explicit sampler descriptors staged with their texture rows.
 * Do not route them through EffectCommon's generic LinearSampler/clamp-mask
 * helpers: that would validate one sampler while the pixel shader consumes
 * another one.
 */
float4 ArtistVisualV4_SampleSourceTexture0(float2 uv)
{
    return g_SourceTexture0.Sample(g_RuntimeMaterialV2Sampler0, uv);
}

float4 ArtistVisualV4_SampleSourceTexture1(float2 uv)
{
    return g_SourceTexture1.Sample(g_RuntimeMaterialV2Sampler1, uv);
}

float4 ArtistVisualV4_SampleSourceTexture2(float2 uv)
{
    return g_SourceTexture2.Sample(g_RuntimeMaterialV2Sampler2, uv);
}

float4 ArtistVisualV4_SampleLevelSourceTexture2(float2 uv, float mipLevel)
{
    return g_SourceTexture2.SampleLevel(
        g_RuntimeMaterialV2Sampler2, uv, mipLevel);
}

float4 ArtistVisualV4_SampleSourceTexture3(float2 uv)
{
    return g_SourceTexture3.Sample(g_RuntimeMaterialV2Sampler3, uv);
}

float4 ArtistVisualV4_SampleSourceTexture4(float2 uv)
{
    return g_SourceTexture4.Sample(g_RuntimeMaterialV2Sampler4, uv);
}

static const uint RECONSTRUCTED_FEATURE_SECOND_TEXTURE = 1u << 0u;
static const uint RECONSTRUCTED_FEATURE_UV_TRANSFORM = 1u << 1u;
static const uint RECONSTRUCTED_FEATURE_PANNER = 1u << 2u;
static const uint RECONSTRUCTED_FEATURE_COLOR = 1u << 3u;
static const uint RECONSTRUCTED_FEATURE_DESATURATION = 1u << 4u;
static const uint RECONSTRUCTED_FEATURE_POWER = 1u << 5u;
static const uint RECONSTRUCTED_FEATURE_DISSOLVE = 1u << 6u;
static const uint RECONSTRUCTED_FEATURE_FRESNEL = 1u << 7u;
static const uint RECONSTRUCTED_FEATURE_DISTORTION = 1u << 8u;
static const uint RECONSTRUCTED_FEATURE_ALPHA = 1u << 9u;

float Reconstructed_SignedPower(float value, float exponent)
{
    return pow(abs(value), exponent) * (value >= 0.f ? 1.f : -1.f);
}

float2 ArtistVisualV4_RotateUV(float2 uv, float turns)
{
    const float angle = turns * 6.28318530717958647692f;
    const float sineValue = sin(angle);
    const float cosineValue = cos(angle);
    const float2 centered = uv - float2(0.5f, 0.5f);
    return float2(
        centered.x * cosineValue - centered.y * sineValue,
        centered.x * sineValue + centered.y * cosineValue) +
        float2(0.5f, 0.5f);
}

float ArtistVisualV4_EdgeFeather(float2 uv, float feather)
{
    if (feather <= 0.f)
        return 1.f;
    const float distanceToEdge = min(
        min(uv.x, 1.f - uv.x), min(uv.y, 1.f - uv.y));
    return saturate(distanceToEdge * feather);
}

float4 ArtistVisualV4_ResolveParticleColor(float4 vertexColor)
{
    /*
     * SpriteParticle carries ColorOverLife in the instance vertex.  A
     * MeshParticle carries the same value through g_ColorMultiply and passes
     * an identity vertex color.  Particle batching binds an identity global,
     * so this product is the common renderer-family carrier.
     */
    return max((g_ColorMultiply + g_ColorOffset) * vertexColor, 0.f);
}

EFFECT_PS_OUT ArtistVisualV4_Reject()
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    clip(-1.f);
    return output;
}

EFFECT_PS_OUT Shade_ArtistVisualV4BasicMissileTrail(
    float2 sourceUV,
    float3 lighting,
    float4 vertexColor)
{
    if (g_ArtistVisualV4TextureMask != 0x03u)
        return ArtistVisualV4_Reject();

    const float4 alphaParameters = g_ArtistVisualV4Params[0];
    const float4 dissolveParameters = g_ArtistVisualV4Params[1];
    const float4 materialParameters = g_ArtistVisualV4Params[2];
    const float4 tint = g_ArtistVisualV4Colors[0];
    const float4 particleColor = ArtistVisualV4_ResolveParticleColor(
        vertexColor);

    const float2 alphaRUV = sourceUV *
        float2(max(abs(alphaParameters.x), 0.0001f), 1.f);
    const float2 alphaGUV = sourceUV *
        float2(max(abs(alphaParameters.y), 0.0001f), 1.f);
    const float alphaR = ArtistVisualV4_SampleSourceTexture0(alphaRUV).r;
    const float alphaG = ArtistVisualV4_SampleSourceTexture0(alphaGUV).g;
    const float alphaCarrier = pow(saturate(
        max(alphaR, alphaG) * max(alphaParameters.z, 0.f)),
        max(abs(alphaParameters.w), 0.001f));

    const float2 dissolveRUV = sourceUV *
        float2(max(abs(dissolveParameters.x), 0.0001f), 1.f);
    const float2 dissolveGUV = sourceUV *
        float2(max(abs(dissolveParameters.y), 0.0001f), 1.f);
    const float dissolveValue = max(
        ArtistVisualV4_SampleSourceTexture1(dissolveRUV).r,
        ArtistVisualV4_SampleSourceTexture1(dissolveGUV).g);
    const float dissolveSoftness = rcp(
        max(abs(dissolveParameters.w), 1.f));
    const float dissolveGate = smoothstep(
        saturate(dissolveParameters.z) - dissolveSoftness,
        saturate(dissolveParameters.z) + dissolveSoftness,
        dissolveValue);
    const float coverage = saturate(
        alphaCarrier * dissolveGate * particleColor.a);

    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    const float emissivePower = max(abs(materialParameters.w), 0.001f);
    const float emissiveCarrier = pow(
        saturate(max(alphaR, alphaG)), emissivePower);
    const float3 sourceColor = emissiveCarrier.xxx;
    output.SceneColor.rgb = clamp(sourceColor * max(tint.rgb, 0.f) *
        max(tint.a, 0.f) * lighting * particleColor.rgb,
        float3(0.f, 0.f, 0.f), float3(16.f, 16.f, 16.f));
    output.SceneColor.a = coverage;
    clip(coverage - max(g_ColorClip, 1.f / 255.f));
    return output;
}

EFFECT_PS_OUT Shade_ArtistVisualV4MakeFlow(
    float2 sourceUV,
    float3 lighting,
    float4 vertexColor)
{
    if (g_ArtistVisualV4TextureMask != 0x0fu)
        return ArtistVisualV4_Reject();

    const float4 diffuseTiles = g_ArtistVisualV4Params[0];
    const float4 flowTransform = g_ArtistVisualV4Params[1];
    const float4 opacityTransform = g_ArtistVisualV4Params[2];
    const float4 panWarp = g_ArtistVisualV4Params[3];
    const float4 materialParameters = g_ArtistVisualV4Params[4];
    const float4 tint = g_ArtistVisualV4Colors[0];
    const float4 particleColor = ArtistVisualV4_ResolveParticleColor(
        vertexColor);

    const float2 flowUV = sourceUV * max(abs(flowTransform.xy),
        float2(0.0001f, 0.0001f)) +
        flowTransform.zw * g_EffectLocalTime;
    const float2 signedFlow =
        ArtistVisualV4_SampleSourceTexture2(flowUV).rg * 2.f - 1.f;
    const float2 internalWarp = signedFlow * panWarp.z * 0.01f;
    const float2 diffuse1UV = sourceUV * max(abs(diffuseTiles.xy),
        float2(0.0001f, 0.0001f)) + internalWarp;
    const float2 diffuse2UV = sourceUV * max(abs(diffuseTiles.zw),
        float2(0.0001f, 0.0001f)) +
        panWarp.xy * g_EffectLocalTime + internalWarp;
    const float4 diffuse1 = ArtistVisualV4_SampleSourceTexture0(diffuse1UV);
    const float4 diffuse2 = ArtistVisualV4_SampleSourceTexture1(diffuse2UV);

    float2 opacityUV = sourceUV * max(abs(opacityTransform.xy),
        float2(0.0001f, 0.0001f));
    opacityUV = ArtistVisualV4_RotateUV(
        opacityUV, opacityTransform.z);
    opacityUV += signedFlow * panWarp.w * 0.01f;
    const float opacityMask = ArtistVisualV4_SampleSourceTexture3(opacityUV).r;
    const float coverage = saturate(
        opacityMask * max(opacityTransform.w, 0.f) *
        particleColor.a);

    const float diffusePower = max(abs(materialParameters.x), 0.001f);
    const float diffuseStrength = max(materialParameters.y, 0.f);
    const float3 diffuse = pow(saturate(diffuse1.rgb * diffuse2.rgb),
        diffusePower) * diffuseStrength;

    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    output.SceneColor.rgb = clamp(diffuse * max(tint.rgb, 0.f) *
        lighting * particleColor.rgb,
        float3(0.f, 0.f, 0.f), float3(16.f, 16.f, 16.f));
    output.SceneColor.a = coverage;

    /* The recovered make-flow contract uses this flow only as an internal UV
       warp.  The selected stable programs for #12/#15 are RT0-only; do not
       let a dormant scalar silently turn the same data into screen MRT output. */
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    clip(coverage - max(g_ColorClip, 1.f / 255.f));
    return output;
}

EFFECT_PS_OUT Shade_ArtistVisualV4ComplexMissileTrail(
    float2 sourceUV,
    float3 lighting,
    float4 vertexColor)
{
    if (g_ArtistVisualV4TextureMask != 0x1fu)
        return ArtistVisualV4_Reject();

    const float4 alphaTransform = g_ArtistVisualV4Params[0];
    const float4 dissolveTransform = g_ArtistVisualV4Params[1];
    const float4 emissivePans = g_ArtistVisualV4Params[2];
    const float4 noiseTransform = g_ArtistVisualV4Params[3];
    const float4 coverageParameters = g_ArtistVisualV4Params[4];
    const float4 materialParameters = g_ArtistVisualV4Params[5];
    const float4 tint = g_ArtistVisualV4Colors[0];
    const float4 particleColor = ArtistVisualV4_ResolveParticleColor(
        vertexColor);

    const float2 noiseUV = sourceUV * max(abs(noiseTransform.xy),
        float2(0.0001f, 0.0001f)) +
        noiseTransform.zw * g_EffectLocalTime;
    const float2 signedNoise =
        ArtistVisualV4_SampleSourceTexture4(noiseUV).rg * 2.f - 1.f;
    const float2 internalWarp = signedNoise * materialParameters.x * 0.01f;
    const float2 alphaUV = sourceUV * max(abs(alphaTransform.xy),
        float2(0.0001f, 0.0001f)) +
        alphaTransform.zw * g_EffectLocalTime + internalWarp;
    const float alphaMask = ArtistVisualV4_SampleSourceTexture0(alphaUV).r;
    const float alphaCarrier = pow(saturate(alphaMask),
        max(abs(coverageParameters.x), 0.001f)) *
        max(coverageParameters.y, 0.f);

    const float2 dissolveUV = sourceUV * max(abs(dissolveTransform.xy),
        float2(0.0001f, 0.0001f)) +
        dissolveTransform.zw * g_EffectLocalTime + internalWarp;
    const float dissolveValue = ArtistVisualV4_SampleSourceTexture1(dissolveUV).r;
    const float dissolveSoftness = rcp(
        max(abs(coverageParameters.w), 1.f));
    const float dissolveGate = smoothstep(
        saturate(coverageParameters.z) - dissolveSoftness,
        saturate(coverageParameters.z) + dissolveSoftness,
        dissolveValue);
    const float coverage = saturate(
        alphaCarrier * dissolveGate * particleColor.a);

    const float2 emissive1UV = sourceUV +
        emissivePans.xy * g_EffectLocalTime + internalWarp;
    const float2 emissive2UV = sourceUV +
        emissivePans.zw * g_EffectLocalTime + internalWarp;
    const float3 emissive =
        ArtistVisualV4_SampleSourceTexture2(emissive1UV).rgb +
        ArtistVisualV4_SampleSourceTexture3(emissive2UV).rgb;
    const float3 rgb = pow(saturate(emissive),
        max(abs(materialParameters.y), 0.001f)) *
        max(materialParameters.z, 0.f);

    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    output.SceneColor.rgb = clamp(rgb * max(tint.rgb, 0.f) * lighting *
        particleColor.rgb, float3(0.f, 0.f, 0.f),
        float3(16.f, 16.f, 16.f));
    output.SceneColor.a = coverage;
    clip(coverage - max(g_ColorClip, 1.f / 255.f));
    return output;
}

EFFECT_PS_OUT Shade_ArtistVisualV4DistortionOnly(
    float2 sourceUV,
    float4 vertexColor)
{
    if (g_ArtistVisualV4TextureMask != 0x03u)
        return ArtistVisualV4_Reject();

    const float4 tile = g_ArtistVisualV4Params[0];
    const float4 pan = g_ArtistVisualV4Params[1];
    const float4 materialParameters = g_ArtistVisualV4Params[2];
    const float4 particleColor = ArtistVisualV4_ResolveParticleColor(
        vertexColor);
    const float2 carrierUV = sourceUV * max(abs(tile.xy),
        float2(0.0001f, 0.0001f)) + pan.xy * g_EffectLocalTime;
    const float2 noiseUV = sourceUV * max(abs(tile.zw),
        float2(0.0001f, 0.0001f)) + pan.zw * g_EffectLocalTime;
    const float carrier = ArtistVisualV4_SampleSourceTexture0(carrierUV).r;
    const float carrierPower = max(abs(materialParameters.y), 0.001f);
    const float carrierGain = max(materialParameters.z, 0.f);
    const float coverage = saturate(pow(
        saturate(carrier * carrierGain), carrierPower) *
        ArtistVisualV4_EdgeFeather(sourceUV, materialParameters.w) *
        particleColor.a);
    const float2 signedNoise =
        ArtistVisualV4_SampleSourceTexture1(noiseUV).rg * 2.f - 1.f;
    const float signedDistortion = clamp(
        materialParameters.x * 0.01f, -1.f, 1.f);

    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    output.SceneColor = float4(0.f, 0.f, 0.f, 0.f);
    output.Distortion = float4(
        signedNoise * signedDistortion * coverage, 0.f, 0.f);
    clip(coverage - max(g_ColorClip, 1.f / 255.f));
    return output;
}

EFFECT_PS_OUT Shade_ArtistVisualV4Spla(
    float2 sourceUV,
    float3 lighting,
    float4 vertexColor,
    float4 dynamicParameter)
{
    /*
     * Exact #25/#29 base-pass semantic lanes, remapped from the native slots:
     *   t0 06.map.r         : scalar internal UV warp
     *   t1 01.specmap_a.rg  : two-phase specular carrier
     *   t2 00.map_alpha.r   : coverage carrier (DDS alpha is never sampled)
     *   t3 06.map_a.g       : coverage modulation
     * Params[0] = map-d tile.xy, pan.zw
     * Params[1] = map-a tile.xy, pan.zw
     * Params[2] = distort strength, shape strength, shape power, min alpha
     * Params[3] = spec strength, spec power, spec UV scale, spec time scale
     * Params[4].x = pass-local external opacity; yzw are reserved zero
     * Colors[0] = source color, Colors[1] = source specular color.
     *
     * dynamic.y is dissolve/life cut and dynamic.z enables internal UV warp.
     * dynamic.w belongs to a separate native distortion-accumulation pass and
     * is deliberately unread here.  This base program produces RT0 only.
     */
    if (g_ArtistVisualV4TextureMask != 0x0fu)
        return ArtistVisualV4_Reject();

    const float4 mapDTransform = g_ArtistVisualV4Params[0];
    const float4 mapATransform = g_ArtistVisualV4Params[1];
    const float4 shapeParameters = g_ArtistVisualV4Params[2];
    const float4 specularParameters = g_ArtistVisualV4Params[3];
    const float externalOpacity = g_ArtistVisualV4Params[4].x;
    const float4 sourceColor = g_ArtistVisualV4Colors[0];
    const float4 sourceSpecularColor = g_ArtistVisualV4Colors[1];
    const float4 particleColor = ArtistVisualV4_ResolveParticleColor(
        vertexColor);

    const float2 mapDUV = sourceUV * mapDTransform.xy +
        mapDTransform.zw * g_EffectLocalTime;
    const float mapD = ArtistVisualV4_SampleSourceTexture0(mapDUV).r;
    const float warpAmount = mapD * dynamicParameter.z * shapeParameters.x;
    const float2 warpedUV = sourceUV + warpAmount.xx;

    const float2 mapAUV = warpedUV * mapATransform.xy +
        mapATransform.zw * g_EffectLocalTime;
    const float mapA = ArtistVisualV4_SampleSourceTexture3(mapAUV).g;
    /* #25/#29 have exact identity alpha rotation and (1,1) alpha UV scale. */
    const float alphaTexture = ArtistVisualV4_SampleSourceTexture2(warpedUV).r;
    const float lifeCut = 1.f - saturate(dynamicParameter.y);
    const float shape = saturate(
        (alphaTexture * mapA - lifeCut) * shapeParameters.y);
    const float poweredShape = shape < 0.000001f ? 0.f :
        pow(shape, shapeParameters.z);
    const float alphaGate = saturate(
        shapeParameters.w - alphaTexture + 1.f);
    const float coverage =
        externalOpacity * particleColor.a * alphaGate * poweredShape;

    const float specularTime = g_EffectLocalTime * specularParameters.w;
    const float2 specularUV1 = warpedUV * specularParameters.z +
        float2(0.f, frac(specularTime * 0.1f));
    const float2 specularUV2 = warpedUV +
        float2(0.f, frac(specularTime * 0.05f));
    const float specularR =
        ArtistVisualV4_SampleSourceTexture1(specularUV2).r;
    const float specularG =
        ArtistVisualV4_SampleSourceTexture1(specularUV1).g;
    const float specularBase = specularR * specularG *
        specularParameters.x;
    const float specularPower = specularParameters.y;
    const float specularCarrier = abs(specularBase) < 0.000001f ? 0.f :
        pow(abs(specularBase), specularPower);
    const float3 materialColor = specularCarrier *
        sourceSpecularColor.rgb + sourceColor.rgb;

    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    /*
     * Native selection-color and fog MAD carriers are not present in this
     * reconstructed caller ABI.  Preserve the exact recovered material and
     * particle-color term; do not substitute the generic lighting carrier.
     */
    output.SceneColor.rgb = materialColor * particleColor.rgb;
    output.SceneColor.a = coverage;
    clip(coverage - max(g_ColorClip, 1.f / 255.f));
    return output;
}

EFFECT_PS_OUT Shade_ArtistVisualV4Flow02RecoveredEquation(
    float2 sourceUV,
    float4 vertexColor,
    float4 dynamicParameter)
{
    /*
     * #13/#14 recovered LocalVF base-PS equation
     * (shaderId 98a9cc70d9b42e45a66ea097407a0894):
     *   t0.g = internal UV noise, t1.r = coverage,
     *   t2.rgb = two SampleLevel(-1) emissive carriers,
     *   t3.g = head-emission mask.
     * Params[0] = map-E scale.xy, pan.zw
     * Params[1] = map-D scale.xy, pan.zw
     * Params[2] = UV-Y stretch, distortion strength, desaturation,
     *             emission power
     * Params[3] = head power, head strength, shape strength, shape power
     * Params[4] = external opacity, direct, reserved zero, reserved zero
     * Colors[0] = source emissive color and strength in alpha.
     *
     * The recovered PS also writes native normal/fog auxiliary targets.  This
     * bounded replay owns RT0 only: it never fabricates those targets or RT1,
     * and native VF/pass admission remains false in the stable registry.
     */
    if (g_ArtistVisualV4TextureMask != 0x0fu)
        return ArtistVisualV4_Reject();

    const float4 mapETransform = g_ArtistVisualV4Params[0];
    const float4 mapDTransform = g_ArtistVisualV4Params[1];
    const float4 materialParameters = g_ArtistVisualV4Params[2];
    const float4 shapeParameters = g_ArtistVisualV4Params[3];
    const float4 rotationParameters = g_ArtistVisualV4Params[4];
    const float4 sourceColor = g_ArtistVisualV4Colors[0];
    const float4 particleColor = ArtistVisualV4_ResolveParticleColor(
        vertexColor);

    const float theta = -0.52359877559829887308f * rotationParameters.y;
    float sineTheta = 0.f;
    float cosineTheta = 1.f;
    sincos(theta, sineTheta, cosineTheta);
    const float2 centered = sourceUV - float2(0.5f, 0.5f);
    const float2 rotated = float2(
        dot(float2(cosineTheta, sineTheta), centered),
        dot(float2(-sineTheta, cosineTheta), centered));
    const float radial = 1.f - saturate(2.f * length(rotated));
    const float2 rotatedUV = rotated + float2(0.5f, 0.5f);

    const float2 mapDUV = float2(
        rotatedUV.x * mapDTransform.x +
            g_EffectLocalTime * mapDTransform.z,
        rotatedUV.y * mapDTransform.y +
            g_EffectLocalTime * mapDTransform.w);
    const float internalNoise = ArtistVisualV4_SampleSourceTexture0(
        mapDUV).g;
    const float2 distortedUV = float2(
        rotatedUV.x,
        saturate(rotated.y * dynamicParameter.z * materialParameters.x +
            0.5f)) +
        internalNoise * (dynamicParameter.w * materialParameters.y);
    const float2 emissiveUV = distortedUV * mapETransform.xy +
        g_EffectLocalTime * mapETransform.zw;
    const float3 emissive0 = ArtistVisualV4_SampleLevelSourceTexture2(
        emissiveUV, -1.f).rgb;
    const float3 emissive1 = ArtistVisualV4_SampleLevelSourceTexture2(
        emissiveUV * 1.66f, -1.f).rgb;
    const float3 emissiveProduct = emissive0 * emissive1;
    const float luminance = dot(emissiveProduct,
        float3(0.3f, 0.59f, 0.11f));
    const float3 desaturated = lerp(
        emissiveProduct, luminance.xxx, materialParameters.z);
    const float3 emissive = pow(abs(desaturated),
        materialParameters.w) * sourceColor.rgb * sourceColor.a;

    const float2 shiftedUV = distortedUV +
        float2(dynamicParameter.y - 1.f, 0.f);
    const float headBase = abs(
        ArtistVisualV4_SampleSourceTexture3(shiftedUV).g);
    const float headMask = (headBase < 0.000001f ? 0.f :
        pow(headBase, shapeParameters.x)) * shapeParameters.y;
    const float3 rgb = ((1.f + headMask) * emissive) * particleColor.rgb;

    const float coverageTexture =
        ArtistVisualV4_SampleSourceTexture1(shiftedUV).r;
    const float shape = saturate(
        radial * coverageTexture * shapeParameters.z);
    const float poweredShape = shape < 0.000001f ? 0.f :
        pow(shape, shapeParameters.w);
    const float alpha = rotationParameters.x * particleColor.a * poweredShape;

    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    output.SceneColor = float4(rgb, alpha);
    return output;
}

EFFECT_PS_OUT Shade_ArtistVisualV4SkullRecoveredEquation(
    float2 sourceUV,
    float4 vertexColor,
    float4 dynamicParameter)
{
    /*
     * #0 recovered SpriteParticle RT0 equation
     * (shaderId 456fc57bd455014b93b97e18c9390a4f).
     * t0/t1/t2 are respectively UV noise, main alpha and dissolve; every
     * recovered sample consumes .r. Params[0..6] preserve the 27 scalar
     * inputs in candidate order, with Params[6].w the bounded opacity 1.
     * Colors[0] is the recovered default SelectionColor (black, alpha 1),
     * and Colors[1] packs the two noise-pan direction vectors.
     *
     * Native selection/fog/normal/specular auxiliary targets are outside the
     * reconstructed caller ABI. This program owns RT0 only; zero-initialized
     * EFFECT_PS_OUT keeps every auxiliary MRT, including RT1, unwritten.
     */
    if (g_ArtistVisualV4TextureMask != 0x07u)
        return ArtistVisualV4_Reject();

    const float4 p0 = g_ArtistVisualV4Params[0];
    const float4 p1 = g_ArtistVisualV4Params[1];
    const float4 p2 = g_ArtistVisualV4Params[2];
    const float4 p3 = g_ArtistVisualV4Params[3];
    const float4 p4 = g_ArtistVisualV4Params[4];
    const float4 p5 = g_ArtistVisualV4Params[5];
    const float4 p6 = g_ArtistVisualV4Params[6];
    const float4 panDirections = g_ArtistVisualV4Colors[1];
    const float4 particleColor = ArtistVisualV4_ResolveParticleColor(
        vertexColor);
    const float T = g_EffectLocalTime;

    const float2 noiseUV0 = sourceUV * p6.xy + float2(p6.z, 0.f) +
        frac(T * p4.z * panDirections.xy);
    const float2 noiseUV1 = sourceUV * p5.zw +
        frac(T * p4.z * panDirections.zw);
    const float n0 = ArtistVisualV4_SampleSourceTexture0(noiseUV0).r;
    const float n1 = ArtistVisualV4_SampleSourceTexture0(noiseUV1).r;
    const float n = n0 * n1 * p5.y;

    const float2 baseUV = sourceUV * p4.xy + p3.zw;
    const float A = ArtistVisualV4_SampleSourceTexture1(
        baseUV + n * p4.w).r;
    const float B = ArtistVisualV4_SampleSourceTexture1(
        baseUV + n * p4.w * p5.x).r;
    const float emissiveBase = abs(A * B);
    const float p = emissiveBase < 0.000001f ? 0.f :
        pow(emissiveBase, 0.2f);
    const float emissiveNoise = p * n;

    const float2 center = sourceUV * float2(1.f, 0.82f) +
        n * 0.3f + p0.zw - float2(0.5f, 0.5f);
    float radial = saturate(
        (1.f - length(center) * 100000.f) * 100000.f);
    radial = saturate(radial * radial);
    const float alphaBase = (radial + B * p0.x) * particleColor.a;

    const float poweredBase = abs(emissiveNoise);
    const float powered = poweredBase < 0.000001f ? 0.f :
        pow(poweredBase, p3.x);
    const float intensity = A * (B + 1.f) * p2.w + powered;
    const float3 rgb = intensity * particleColor.rgb;

    const float2 dissolveUV = sourceUV * p2.xy +
        T * float2(0.f, p1.w);
    const float dissolve =
        ArtistVisualV4_SampleSourceTexture2(dissolveUV).r;
    const float gate = saturate(
        (dissolve - (dynamicParameter.x - 1.f)) * p1.z);

    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    output.SceneColor.rgb = rgb;
    output.SceneColor.a = saturate(gate * alphaBase) * p6.w;
    return output;
}

EFFECT_PS_OUT Shade_ArtistVisualV4(
    float2 sourceUV,
    float3 lighting,
    float4 vertexColor,
    float4 dynamicParameter)
{
    if (g_ArtistVisualV4Opcode == ARTIST_VISUAL_V4_BASIC_MISSILETRAIL)
    {
        return Shade_ArtistVisualV4BasicMissileTrail(
            sourceUV, lighting, vertexColor);
    }
    if (g_ArtistVisualV4Opcode == ARTIST_VISUAL_V4_MAKEFLOW)
        return Shade_ArtistVisualV4MakeFlow(sourceUV, lighting, vertexColor);
    if (g_ArtistVisualV4Opcode == ARTIST_VISUAL_V4_COMPLEX_MISSILETRAIL)
    {
        return Shade_ArtistVisualV4ComplexMissileTrail(
            sourceUV, lighting, vertexColor);
    }
    if (g_ArtistVisualV4Opcode == ARTIST_VISUAL_V4_DISTORTION_ONLY)
        return Shade_ArtistVisualV4DistortionOnly(sourceUV, vertexColor);
    if (g_ArtistVisualV4Opcode == ARTIST_VISUAL_V4_UNSUPPORTED_SUPPRESSED)
    {
        /* Keep the document alive while this occurrence remains zero-draw. */
        if (g_ArtistVisualV4TextureMask != 0u)
            return ArtistVisualV4_Reject();
        return ArtistVisualV4_Reject();
    }
    if (g_ArtistVisualV4Opcode == ARTIST_VISUAL_V4_SPLA)
    {
        return Shade_ArtistVisualV4Spla(
            sourceUV, lighting, vertexColor, dynamicParameter);
    }
    if (g_ArtistVisualV4Opcode ==
        ARTIST_VISUAL_V4_FLOW02_RECOVERED_EQUATION)
    {
        return Shade_ArtistVisualV4Flow02RecoveredEquation(
            sourceUV, vertexColor, dynamicParameter);
    }
    if (g_ArtistVisualV4Opcode ==
        ARTIST_VISUAL_V4_SKULL_RECOVERED_EQUATION)
    {
        return Shade_ArtistVisualV4SkullRecoveredEquation(
            sourceUV, vertexColor, dynamicParameter);
    }
    return ArtistVisualV4_Reject();
}

float4 Evaluate_ReconstructedMaterial(
    float4 texture0,
    float4 texture1)
{
    float4 result = texture0;
    float2 uv = float2(0.375f, 0.625f) * g_ReconstructedUVScale.xy
        + g_ReconstructedPanRotationAux.xy * g_EffectLocalTime;

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_SECOND_TEXTURE) != 0u)
    {
        result *= 0.5f + 0.5f * texture1;
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_UV_TRANSFORM) != 0u)
    {
        result.rgb += (frac(uv.x) + frac(uv.y)) * 0.03125f;
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_PANNER) != 0u)
    {
        result.rgb += frac(g_EffectLocalTime * 0.125f +
            g_ReconstructedPanRotationAux.z) * 0.015625f;
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_COLOR) != 0u)
    {
        result.rgb *= g_ReconstructedColor.rgb *
            max(g_ReconstructedColor.a, 0.f);
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_DESATURATION) != 0u)
    {
        float luminance = dot(result.rgb, float3(0.299f, 0.587f, 0.114f));
        result.rgb = lerp(result.rgb, luminance.xxx,
            saturate(g_ReconstructedParams0.z));
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_POWER) != 0u)
    {
        float exponent = max(abs(g_ReconstructedParams0.y), 0.001f);
        result.rgb = float3(
            Reconstructed_SignedPower(result.r, exponent),
            Reconstructed_SignedPower(result.g, exponent),
            Reconstructed_SignedPower(result.b, exponent));
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_FRESNEL) != 0u)
    {
        float gain = 1.f + pow(
            saturate(g_ReconstructedParams1.y),
            max(abs(g_ReconstructedParams0.y), 0.001f));
        result.rgb *= gain;
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_DISTORTION) != 0u)
    {
        result.xy += (texture1.xy * 2.f - 1.f) *
            g_ReconstructedParams1.z;
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_DISSOLVE) != 0u)
    {
        result.a = saturate((result.a - g_ReconstructedParams1.x) *
            max(abs(g_ReconstructedParams0.x), 0.001f));
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_ALPHA) != 0u)
    {
        result.a = saturate(result.a * g_ReconstructedParams0.w);
    }

    return result;
}

EFFECT_PS_OUT Shade_ReconstructedMaterial(
    float2 sourceUV,
    float3 lighting,
    float4 vertexColor,
    float4 dynamicParameter)
{
    if (g_ArtistVisualV4Opcode != 0u)
    {
        return Shade_ArtistVisualV4(
            sourceUV, lighting, vertexColor, dynamicParameter);
    }

    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    output.SceneColor = Evaluate_ReconstructedMaterial(
        Sample_SourceTexture0(sourceUV),
        Sample_SourceTexture1(sourceUV));
    output.SceneColor.rgb *= lighting * vertexColor.rgb;
    output.SceneColor.a = saturate(output.SceneColor.a * vertexColor.a);
    return output;
}
