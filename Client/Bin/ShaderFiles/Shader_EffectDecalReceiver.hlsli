#ifndef LOSTARK_EFFECT_DECAL_RECEIVER_HLSLI
#define LOSTARK_EFFECT_DECAL_RECEIVER_HLSLI

// Projected effects belong to environment surfaces. Material profile identity
// must not opt a newly restored floor decal into painting actors.
// Target_PickPos is RGBA32_FLOAT: load W without texture filtering.
bool Reject_EffectDecalActorReceiver(float depthMarker, float receiverPayload)
{
    const uint receiverBits = asuint(receiverPayload);
    // Only default/source geometry owns bit8. Animated map backgrounds return
    // before writing it; other map markers use these bits for unrelated data.
    const bool skinnedActor = (depthMarker == 0.f || depthMarker == 5.f) &&
        (receiverBits & 256u) != 0u;
    const uint sourceProgram = receiverBits & 255u;
    // Native skin/equipment also covers rigid actor attachments. Static map
    // families 25/30/80..83 remain receivers; animated program30 uses bit8.
    const bool nativeActor = (sourceProgram >= 1u && sourceProgram <= 24u) ||
        (sourceProgram >= 26u && sourceProgram <= 29u) || sourceProgram == 84u;
    return skinnedActor || (depthMarker == 5.f && nativeActor);
}

#endif
