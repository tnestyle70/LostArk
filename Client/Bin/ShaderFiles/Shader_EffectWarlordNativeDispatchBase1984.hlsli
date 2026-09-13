#if !defined(EFFECT_WARLORD_GROUP) || EFFECT_WARLORD_GROUP == 1984
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 2000u: nativeColor=WarlordNative2000(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 2001u: nativeColor=WarlordNative2001(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 2002u: nativeColor=WarlordNative2002(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 2003u: nativeColor=WarlordNative2003(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 2004u: nativeColor=WarlordNative2004(input); additive=false; break;
    case 2005u: nativeColor=WarlordNative2005(input); additive=true; break;
    case 2006u: nativeColor=WarlordNative2006(input); additive=true; break;
    case 2007u: nativeColor=WarlordNative2007(input); additive=false; break;
    case 2008u: nativeColor=WarlordNative2008(input); additive=true; break;
#endif
#endif
