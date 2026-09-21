#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4288u:
    {
        nativeColor=ArtistNative4288(input);
        const float4 accumulated=ArtistNative4288Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4289u:
    {
        nativeColor=ArtistNative4289(input);
        const float4 accumulated=ArtistNative4289Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4290u: nativeColor=ArtistNative4290(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4291u: nativeColor=ArtistNative4291(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4292u:
    {
        nativeColor=ArtistNative4292(input);
        const float4 accumulated=ArtistNative4292Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288 || defined(EFFECT_NATIVE_DECAL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4293u: nativeColor=ArtistNative4293(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288 || defined(EFFECT_NATIVE_DECAL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4294u: nativeColor=ArtistNative4294(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4295u: nativeColor=ArtistNative4295(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4296u: nativeColor=ArtistNative4296(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4297u: nativeColor=ArtistNative4297(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4298u:
    {
        nativeColor=ArtistNative4298(input);
        const float4 accumulated=ArtistNative4298Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4299u: nativeColor=ArtistNative4299(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4300u: nativeColor=ArtistNative4300(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4301u:
    {
        nativeColor=ArtistNative4301(input);
        const float4 accumulated=ArtistNative4301Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4302u:
    {
        nativeColor=ArtistNative4302(input);
        const float4 accumulated=ArtistNative4302Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4303u:
    {
        nativeColor=ArtistNative4303(input);
        const float4 accumulated=ArtistNative4303Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4336u:
    {
        nativeColor=ArtistNative4336(input);
        const float4 accumulated=ArtistNative4336Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4337u:
    {
        nativeColor=ArtistNative4337(input);
        const float4 accumulated=ArtistNative4337Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4338u:
    {
        nativeColor=ArtistNative4338(input);
        const float4 accumulated=ArtistNative4338Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4339u:
    {
        nativeColor=ArtistNative4339(input);
        const float4 accumulated=ArtistNative4339Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4340u:
    {
        nativeColor=ArtistNative4340(input);
        const float4 accumulated=ArtistNative4340Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4341u:
    {
        nativeColor=ArtistNative4341(input);
        const float4 accumulated=ArtistNative4341Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4342u: nativeColor=ArtistNative4342(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4343u: nativeColor=ArtistNative4343(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4344u: nativeColor=ArtistNative4344(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4345u: nativeColor=ArtistNative4345(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4346u: nativeColor=ArtistNative4346(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4347u: nativeColor=ArtistNative4347(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4348u: nativeColor=ArtistNative4348(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4349u:
    {
        nativeColor=ArtistNative4349(input);
        const float4 accumulated=ArtistNative4349Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4350u: nativeColor=ArtistNative4350(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4351u:
    {
        nativeColor=ArtistNative4351(input);
        const float4 accumulated=ArtistNative4351Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER)
    case 4304u: nativeColor=ArtistNative4304(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER)
    case 4305u: nativeColor=ArtistNative4305(input); opaqueCoverage=false; break;
#endif
