#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4416) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4416u: nativeColor=ArtistNative4416(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4416) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4417u: nativeColor=ArtistNative4417(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4416) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4418u: nativeColor=ArtistNative4418(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4416) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4419u: nativeColor=ArtistNative4419(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4416) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4420u:
    {
        nativeColor=ArtistNative4420(input);
        const float4 accumulated=ArtistNative4420Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4416) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4421u:
    {
        nativeColor=ArtistNative4421(input);
        const float4 accumulated=ArtistNative4421Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4416) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4422u: nativeColor=ArtistNative4422(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4416) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4423u: nativeColor=ArtistNative4423(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4416) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4424u: nativeColor=ArtistNative4424(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4416) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4425u:
    {
        nativeColor=ArtistNative4425(input);
        const float4 accumulated=ArtistNative4425Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4416) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4426u: nativeColor=ArtistNative4426(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4416) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4427u: nativeColor=ArtistNative4427(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4416) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4428u: nativeColor=ArtistNative4428(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4416) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4429u:
    {
        nativeColor=ArtistNative4429(input);
        const float4 accumulated=ArtistNative4429Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
