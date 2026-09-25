#pragma once
#include <cstdint>
namespace Client::SourceEquipmentMaterial
{
// Native BLEND_Translucent permutations restored from the installed equipment MICs.
inline bool Is_Translucent(uint32_t program)
{
    return program == 160u || program == 166u || program == 168u || program == 169u || program == 170u || program == 171u || program == 172u || program == 174u || program == 182u || program == 187u || program == 190u || program == 192u || program == 195u || program == 212u || program == 213u || program == 600u || program == 601u ||
        program == 700u || program == 701u || program == 702u || program == 801u || program == 903u;
}
inline bool Is_TwoSidedMasked(uint32_t program)
{
    return program == 161u || program == 167u || program == 179u;
}
}
