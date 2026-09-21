#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4096u: nativeColor=ArtistNative4096(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4097u: nativeColor=ArtistNative4097(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4098u: nativeColor=ArtistNative4098(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4099u:
    {
        nativeColor=ArtistNative4099(input);
        const float4 accumulated=ArtistNative4099Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4100u: nativeColor=ArtistNative4100(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4101u:
    {
        nativeColor=ArtistNative4101(input);
        const float4 accumulated=ArtistNative4101Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4102u: nativeColor=ArtistNative4102(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4103u:
    {
        nativeColor=ArtistNative4103(input);
        const float4 accumulated=ArtistNative4103Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096 || defined(EFFECT_NATIVE_DECAL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4104u: nativeColor=ArtistNative4104(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4105u: nativeColor=ArtistNative4105(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4106u:
    {
        nativeColor=ArtistNative4106(input);
        const float4 accumulated=ArtistNative4106Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4107u: nativeColor=ArtistNative4107(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4108u: nativeColor=ArtistNative4108(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4109u: nativeColor=ArtistNative4109(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4110u:
    {
        nativeColor=ArtistNative4110(input);
        const float4 accumulated=ArtistNative4110Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4111u: nativeColor=ArtistNative4111(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4112u:
    {
        nativeColor=ArtistNative4112(input);
        const float4 accumulated=ArtistNative4112Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4113u:
    {
        nativeColor=ArtistNative4113(input);
        const float4 accumulated=ArtistNative4113Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4114u:
    {
        nativeColor=ArtistNative4114(input);
        const float4 accumulated=ArtistNative4114Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4115u:
    {
        nativeColor=ArtistNative4115(input);
        const float4 accumulated=ArtistNative4115Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4116u: nativeColor=ArtistNative4116(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096 || defined(EFFECT_NATIVE_DECAL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4117u: nativeColor=ArtistNative4117(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4118u: nativeColor=ArtistNative4118(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4119u: nativeColor=ArtistNative4119(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4120u:
    {
        nativeColor=ArtistNative4120(input);
        const float4 accumulated=ArtistNative4120Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4121u: nativeColor=ArtistNative4121(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4122u:
    {
        nativeColor=ArtistNative4122(input);
        const float4 accumulated=ArtistNative4122Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4123u: nativeColor=ArtistNative4123(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4124u: nativeColor=ArtistNative4124(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4125u:
    {
        nativeColor=ArtistNative4125(input);
        const float4 accumulated=ArtistNative4125Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096 || defined(EFFECT_NATIVE_TRAIL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4126u: nativeColor=ArtistNative4126(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4127u: nativeColor=ArtistNative4127(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4128u:
    {
        nativeColor=ArtistNative4128(input);
        const float4 accumulated=ArtistNative4128Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4129u: nativeColor=ArtistNative4129(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4130u: nativeColor=ArtistNative4130(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4131u: nativeColor=ArtistNative4131(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4132u: nativeColor=ArtistNative4132(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4133u: nativeColor=ArtistNative4133(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4134u:
    {
        nativeColor=ArtistNative4134(input);
        const float4 accumulated=ArtistNative4134Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4135u: nativeColor=ArtistNative4135(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4136u: nativeColor=ArtistNative4136(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4137u: nativeColor=ArtistNative4137(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4138u: nativeColor=ArtistNative4138(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4139u: nativeColor=ArtistNative4139(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4140u:
    {
        nativeColor=ArtistNative4140(input);
        const float4 accumulated=ArtistNative4140Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4141u:
    {
        nativeColor=ArtistNative4141(input);
        const float4 accumulated=ArtistNative4141Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4142u: nativeColor=ArtistNative4142(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4143u: nativeColor=ArtistNative4143(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4144u: nativeColor=ArtistNative4144(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096 || defined(EFFECT_NATIVE_TRAIL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4145u:
    {
        nativeColor=ArtistNative4145(input);
        const float4 accumulated=ArtistNative4145Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096 || defined(EFFECT_NATIVE_TRAIL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4146u:
    {
        nativeColor=ArtistNative4146(input);
        const float4 accumulated=ArtistNative4146Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4147u: nativeColor=ArtistNative4147(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4148u: nativeColor=ArtistNative4148(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4149u: nativeColor=ArtistNative4149(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4150u: nativeColor=ArtistNative4150(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4151u: nativeColor=ArtistNative4151(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4152u: nativeColor=ArtistNative4152(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4153u:
    {
        nativeColor=ArtistNative4153(input);
        const float4 accumulated=ArtistNative4153Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4154u: nativeColor=ArtistNative4154(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4155u:
    {
        nativeColor=ArtistNative4155(input);
        const float4 accumulated=ArtistNative4155Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096 || defined(EFFECT_NATIVE_DECAL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4156u: nativeColor=ArtistNative4156(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096 || defined(EFFECT_NATIVE_DECAL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4157u: nativeColor=ArtistNative4157(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096 || defined(EFFECT_NATIVE_DECAL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4158u: nativeColor=ArtistNative4158(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096 || defined(EFFECT_NATIVE_DECAL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4159u: nativeColor=ArtistNative4159(input); opaqueCoverage=false; break;
#endif
