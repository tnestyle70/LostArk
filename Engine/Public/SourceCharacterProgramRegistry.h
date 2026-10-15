#pragma once

// 추가 등록된 정확한 program ID들을 고정 크기 배열로 제공한다.
#include <array>
// program ID와 그룹 경계를 고정 폭 32bit 값으로 표현한다.
#include <cstdint>

namespace Engine
{
    // 생성된 등록 목록은 사용 가능한 program만 선언한다. 각 program의 렌더 의미는 실제 shader가 정의한다.
    // Tools/EffectPipeline/source_character_registration.py가 BEGIN/END 표식 사이 ID와 배열 크기를 갱신한다.
    // 표식은 도구가 검색하는 계약이므로 영어 원문을 유지한다.
    inline constexpr std::array<std::uint32_t, 140> SourceCharacterAddedPrograms = {
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
        905u,
        906u,
        907u,
        908u,
        909u,
        910u,
        911u,
        912u,
        913u,
        914u,
        915u,
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
        1474u,
        1475u,
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
        1526u,
        1527u,
        1528u,
        1529u,
        1530u,
        1531u,
        1532u,
        1533u,
        // END REGISTERED SOURCE CHARACTER PROGRAMS
    };

    // CShader가 _SourceGroupXXX shader 변형을 찾는 양끝 포함 ID 범위다. 범위에 속한다고 모든 ID가 지원되는 것은 아니다.
    struct SOURCE_CHARACTER_PROGRAM_GROUP
    {
        std::uint32_t first;
        std::uint32_t last;
    };

    // shader 묶음 선택용 범위다. 기존 그룹과 등록 도구가 계산한 추가 그룹을 함께 사용한다.
    inline constexpr SOURCE_CHARACTER_PROGRAM_GROUP SourceCharacterProgramGroups[] = {
        {1u, 8u}, {9u, 16u}, {17u, 24u}, {25u, 32u},
        {80u, 83u}, {84u, 95u}, {96u, 107u}, {108u, 112u}, {160u, 175u}, {176u, 191u},
        {192u, 200u}, {208u, 213u}, {214u, 223u}, {224u, 234u},
        {235u, 236u}, {237u, 238u},
        // BEGIN REGISTERED SOURCE CHARACTER GROUPS
        {576u, 639u},
        {640u, 703u},
        {768u, 831u},
        {896u, 959u},
        {1088u, 1103u},
        {1104u, 1119u},
        {1120u, 1135u},
        {1136u, 1151u},
        {1152u, 1215u},
        {1344u, 1407u},
        {1408u, 1471u},
        {1472u, 1535u},
        // END REGISTERED SOURCE CHARACTER GROUPS
    };

    // Model/Shader가 정확한 program 지원 여부를 검사한다. 기존 허용 구간 또는 개별 추가 등록 ID만 true다.
    // 그룹 범위 안의 빈 번호를 임의의 shader 동작으로 해석하지 않는다.
    constexpr bool Is_SourceCharacterProgramSupported(std::uint32_t program)
    {
        // 기존 forward 전용 program 허용 구간과 미할당 번호 간격을 유지한다.
        if ((program >= 1u && program <= 65u) || (program >= 80u && program <= 112u) ||
            (program >= 160u && program <= 200u) || (program >= 208u && program <= 238u))
            return true;
        for (const auto registered : SourceCharacterAddedPrograms)
            if (registered == program)
                return true;
        return false;
    }
}
