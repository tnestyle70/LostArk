#pragma once

#include <array>
#include <cstdint>

namespace Engine
{
    // Generated registration describes available programs, never renderer semantics.
    inline constexpr std::array<std::uint32_t, 119> SourceCharacterAddedPrograms = {
        // BEGIN REGISTERED SOURCE CHARACTER PROGRAMS
        600u,
        601u,
        700u,
        701u,
        702u,
        703u,
        800u,
        801u,
        901u,
        902u,
        903u,
        904u,
        1100u,
        1101u,
        1102u,
        1103u,
        1104u,
        1105u,
        1106u,
        1107u,
        1108u,
        1109u,
        1110u,
        1111u,
        1112u,
        1113u,
        1114u,
        1115u,
        1116u,
        1117u,
        1118u,
        1119u,
        1120u,
        1121u,
        1122u,
        1123u,
        1124u,
        1125u,
        1126u,
        1127u,
        1128u,
        1129u,
        1130u,
        1131u,
        1132u,
        1133u,
        1134u,
        1135u,
        1136u,
        1137u,
        1138u,
        1139u,
        1140u,
        1141u,
        1142u,
        1143u,
        1144u,
        1145u,
        1146u,
        1147u,
        1148u,
        1149u,
        1150u,
        1151u,
        1152u,
        1153u,
        1154u,
        1155u,
        1156u,
        1157u,
        1158u,
        1159u,
        1160u,
        1161u,
        1162u,
        1163u,
        1164u,
        1165u,
        1166u,
        1400u,
        1401u,
        1402u,
        1403u,
        1404u,
        1405u,
        1406u,
        1407u,
        1408u,
        1409u,
        1410u,
        1411u,
        1412u,
        1413u,
        1500u,
        1501u,
        1502u,
        1503u,
        1504u,
        1505u,
        1506u,
        1507u,
        1508u,
        1509u,
        1510u,
        1511u,
        1512u,
        1513u,
        1514u,
        1515u,
        1516u,
        1517u,
        1518u,
        1519u,
        1520u,
        1521u,
        1522u,
        1523u,
        1524u,
        1525u,
        // END REGISTERED SOURCE CHARACTER PROGRAMS
    };

    struct SOURCE_CHARACTER_PROGRAM_GROUP
    {
        std::uint32_t first;
        std::uint32_t last;
    };

    inline constexpr SOURCE_CHARACTER_PROGRAM_GROUP SourceCharacterProgramGroups[] = {
        {1u, 8u}, {9u, 16u}, {17u, 24u}, {25u, 32u},
        {80u, 83u}, {84u, 112u}, {160u, 175u}, {176u, 191u},
        {192u, 200u}, {208u, 213u}, {214u, 223u}, {224u, 234u},
        {235u, 236u}, {237u, 238u},
        // BEGIN REGISTERED SOURCE CHARACTER GROUPS
        {576u, 639u},
        {640u, 703u},
        {768u, 831u},
        {896u, 959u},
        {1088u, 1151u},
        {1152u, 1215u},
        {1344u, 1407u},
        {1408u, 1471u},
        {1472u, 1535u},
        // END REGISTERED SOURCE CHARACTER GROUPS
    };

    constexpr bool Is_SourceCharacterProgramSupported(std::uint32_t program)
    {
        // Keep the established forward-only programs and historical gaps unchanged.
        if ((program >= 1u && program <= 65u) || (program >= 80u && program <= 112u) ||
            (program >= 160u && program <= 200u) || (program >= 208u && program <= 238u))
            return true;
        for (const auto registered : SourceCharacterAddedPrograms)
            if (registered == program) return true;
        return false;
    }
}
