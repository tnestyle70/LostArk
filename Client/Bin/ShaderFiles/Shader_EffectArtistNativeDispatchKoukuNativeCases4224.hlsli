#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4224u: nativeColor=ArtistNative4224(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4225u:
    {
        nativeColor=ArtistNative4225(input);
        const float4 accumulated=ArtistNative4225Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4226u:
    {
        nativeColor=ArtistNative4226(input);
        const float4 accumulated=ArtistNative4226Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4227u:
    {
        nativeColor=ArtistNative4227(input);
        const float4 accumulated=ArtistNative4227Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4228u:
    {
        nativeColor=ArtistNative4228(input);
        const float4 accumulated=ArtistNative4228Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4229u: nativeColor=ArtistNative4229(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224 || defined(EFFECT_NATIVE_TRAIL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4230u:
    {
        nativeColor=ArtistNative4230(input);
        const float4 accumulated=ArtistNative4230Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4231u: nativeColor=ArtistNative4231(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4232u: nativeColor=ArtistNative4232(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4233u:
    {
        nativeColor=ArtistNative4233(input);
        const float4 accumulated=ArtistNative4233Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224 || defined(EFFECT_NATIVE_DECAL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4234u: nativeColor=ArtistNative4234(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224 || defined(EFFECT_NATIVE_DECAL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4235u: nativeColor=ArtistNative4235(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224 || defined(EFFECT_NATIVE_DECAL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4236u: nativeColor=ArtistNative4236(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224 || defined(EFFECT_NATIVE_DECAL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4237u: nativeColor=ArtistNative4237(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4238u:
    {
        nativeColor=ArtistNative4238(input);
        const float4 accumulated=ArtistNative4238Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4239u: nativeColor=ArtistNative4239(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4240u:
    {
        nativeColor=ArtistNative4240(input);
        const float4 accumulated=ArtistNative4240Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4241u:
    {
        nativeColor=ArtistNative4241(input);
        const float4 accumulated=ArtistNative4241Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4242u:
    {
        nativeColor=ArtistNative4242(input);
        const float4 accumulated=ArtistNative4242Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4243u:
    {
        nativeColor=ArtistNative4243(input);
        const float4 accumulated=ArtistNative4243Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4244u: nativeColor=ArtistNative4244(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4245u: nativeColor=ArtistNative4245(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4246u:
    {
        nativeColor=ArtistNative4246(input);
        const float4 accumulated=ArtistNative4246Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4247u: nativeColor=ArtistNative4247(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4248u:
    {
        nativeColor=ArtistNative4248(input);
        const float4 accumulated=ArtistNative4248Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4249u: nativeColor=ArtistNative4249(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4250u:
    {
        nativeColor=ArtistNative4250(input);
        const float4 accumulated=ArtistNative4250Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4251u:
    {
        nativeColor=ArtistNative4251(input);
        const float4 accumulated=ArtistNative4251Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4252u: nativeColor=ArtistNative4252(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4253u: nativeColor=ArtistNative4253(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4254u: nativeColor=ArtistNative4254(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224 || defined(EFFECT_NATIVE_TRAIL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4255u:
    {
        nativeColor=ArtistNative4255(input);
        const float4 accumulated=ArtistNative4255Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224 || defined(EFFECT_NATIVE_TRAIL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4256u:
    {
        nativeColor=ArtistNative4256(input);
        const float4 accumulated=ArtistNative4256Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4257u:
    {
        nativeColor=ArtistNative4257(input);
        const float4 accumulated=ArtistNative4257Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4258u:
    {
        nativeColor=ArtistNative4258(input);
        const float4 accumulated=ArtistNative4258Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4259u: nativeColor=ArtistNative4259(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4260u: nativeColor=ArtistNative4260(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4261u: nativeColor=ArtistNative4261(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4262u: nativeColor=ArtistNative4262(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224 || defined(EFFECT_NATIVE_DECAL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4263u: nativeColor=ArtistNative4263(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4264u: nativeColor=ArtistNative4264(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4265u: nativeColor=ArtistNative4265(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4266u:
    {
        nativeColor=ArtistNative4266(input);
        const float4 accumulated=ArtistNative4266Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224 || defined(EFFECT_NATIVE_DECAL_CARRIER)) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4267u: nativeColor=ArtistNative4267(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4268u:
    {
        nativeColor=ArtistNative4268(input);
        const float4 accumulated=ArtistNative4268Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4269u: nativeColor=ArtistNative4269(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4270u: nativeColor=ArtistNative4270(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4271u: nativeColor=ArtistNative4271(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4272u: nativeColor=ArtistNative4272(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4273u: nativeColor=ArtistNative4273(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4274u: nativeColor=ArtistNative4274(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4275u: nativeColor=ArtistNative4275(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4276u:
    {
        nativeColor=ArtistNative4276(input);
        const float4 accumulated=ArtistNative4276Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4277u:
    {
        nativeColor=ArtistNative4277(input);
        const float4 accumulated=ArtistNative4277Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4278u:
    {
        nativeColor=ArtistNative4278(input);
        const float4 accumulated=ArtistNative4278Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4279u: nativeColor=ArtistNative4279(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4280u: nativeColor=ArtistNative4280(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4281u:
    {
        nativeColor=ArtistNative4281(input);
        const float4 accumulated=ArtistNative4281Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4282u: nativeColor=ArtistNative4282(input); opaqueCoverage=true; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4283u: nativeColor=ArtistNative4283(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4284u: nativeColor=ArtistNative4284(input); opaqueCoverage=false; break;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4285u:
    {
        nativeColor=ArtistNative4285(input);
        const float4 accumulated=ArtistNative4285Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4286u:
    {
        nativeColor=ArtistNative4286(input);
        const float4 accumulated=ArtistNative4286Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 4287u: nativeColor=ArtistNative4287(input); opaqueCoverage=false; break;
#endif
