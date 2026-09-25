// Generated from original local-factory VS output signatures.
#ifndef SOURCE_MOVIE_STATIC_INPUTS_INCLUDED
#define SOURCE_MOVIE_STATIC_INPUTS_INCLUDED
bool IsSourceMovieStatic(uint program)
{
    return (program >= 1100u && program <= 1166u) ||
        (program >= 1400u && program <= 1413u) || (program >= 1500u && program <= 1525u);
}
void PackSourceMovieBaseInput(inout SOURCE_CHARACTER_NATIVE_INPUT input,
    float4 tangentX, float4 tangentZ, float4 color, float2 uv,
    float4 view, float4 light, float4 up, float4 clip, float4 fog)
{
    if (!IsSourceMovieStatic(g_SourceCharacterProgram)) return;
    [unroll] for (uint lane=0u;lane<10u;++lane) input.values[lane]=0.f;
    if (g_SourceCharacterProgram==1100u || g_SourceCharacterProgram==1103u || g_SourceCharacterProgram==1104u || g_SourceCharacterProgram==1105u || g_SourceCharacterProgram==1108u || g_SourceCharacterProgram==1110u || g_SourceCharacterProgram==1111u || g_SourceCharacterProgram==1112u || g_SourceCharacterProgram==1115u || g_SourceCharacterProgram==1116u || g_SourceCharacterProgram==1118u || g_SourceCharacterProgram==1119u || g_SourceCharacterProgram==1120u || g_SourceCharacterProgram==1121u || g_SourceCharacterProgram==1122u || g_SourceCharacterProgram==1123u || g_SourceCharacterProgram==1124u || g_SourceCharacterProgram==1125u || g_SourceCharacterProgram==1126u || g_SourceCharacterProgram==1127u || g_SourceCharacterProgram==1128u || g_SourceCharacterProgram==1129u || g_SourceCharacterProgram==1130u || g_SourceCharacterProgram==1131u || g_SourceCharacterProgram==1132u || g_SourceCharacterProgram==1133u || g_SourceCharacterProgram==1134u || g_SourceCharacterProgram==1135u || g_SourceCharacterProgram==1136u || g_SourceCharacterProgram==1137u || g_SourceCharacterProgram==1138u || g_SourceCharacterProgram==1139u || g_SourceCharacterProgram==1144u || g_SourceCharacterProgram==1145u || g_SourceCharacterProgram==1146u || g_SourceCharacterProgram==1147u || g_SourceCharacterProgram==1148u || g_SourceCharacterProgram==1149u || g_SourceCharacterProgram==1151u || g_SourceCharacterProgram==1152u || g_SourceCharacterProgram==1154u || g_SourceCharacterProgram==1155u || g_SourceCharacterProgram==1157u || g_SourceCharacterProgram==1158u || g_SourceCharacterProgram==1160u || g_SourceCharacterProgram==1161u || g_SourceCharacterProgram==1162u || g_SourceCharacterProgram==1163u || g_SourceCharacterProgram==1164u || g_SourceCharacterProgram==1165u || g_SourceCharacterProgram==1402u || g_SourceCharacterProgram==1404u || g_SourceCharacterProgram==1408u || g_SourceCharacterProgram==1409u || g_SourceCharacterProgram==1410u || g_SourceCharacterProgram==1504u || g_SourceCharacterProgram==1514u || g_SourceCharacterProgram==1518u || g_SourceCharacterProgram==1521u || g_SourceCharacterProgram==1408u || g_SourceCharacterProgram==1166u || g_SourceCharacterProgram==1103u)
    {
        input.values[0].xyzw=(tangentX).xyzw;
        input.values[1].xyzw=(tangentZ).xyzw;
        input.values[2].xyzw=(color).xyzw;
        input.values[3].xy=(float4(0.f,0.f,0.f,0.f)).xy;
        input.values[4].xyzw=(float4(uv,0.f,0.f)).xyzw;
        input.values[5].xyzw=(view).xyzw;
        input.values[6].xyz=(up).xyz;
        input.values[7].xyzw=(clip).xyzw;
        return;
    }
    if (g_SourceCharacterProgram==1101u || g_SourceCharacterProgram==1102u || g_SourceCharacterProgram==1106u || g_SourceCharacterProgram==1107u || g_SourceCharacterProgram==1109u || g_SourceCharacterProgram==1113u || g_SourceCharacterProgram==1114u || g_SourceCharacterProgram==1117u || g_SourceCharacterProgram==1140u || g_SourceCharacterProgram==1141u || g_SourceCharacterProgram==1142u || g_SourceCharacterProgram==1143u || g_SourceCharacterProgram==1150u || g_SourceCharacterProgram==1153u || g_SourceCharacterProgram==1159u || g_SourceCharacterProgram==1400u || g_SourceCharacterProgram==1401u || g_SourceCharacterProgram==1403u || g_SourceCharacterProgram==1405u || g_SourceCharacterProgram==1406u || g_SourceCharacterProgram==1407u || g_SourceCharacterProgram==1411u || g_SourceCharacterProgram==1412u || g_SourceCharacterProgram==1413u || g_SourceCharacterProgram==1509u || g_SourceCharacterProgram==1513u || g_SourceCharacterProgram==1517u || g_SourceCharacterProgram==1522u || g_SourceCharacterProgram==1523u || g_SourceCharacterProgram==1525u)
    {
        input.values[0].xyzw=(tangentX).xyzw;
        input.values[1].xyzw=(tangentZ).xyzw;
        input.values[2].xyzw=(color).xyzw;
        input.values[3].xy=(float4(0.f,0.f,0.f,0.f)).xy;
        input.values[4].xyzw=(float4(uv,0.f,0.f)).xyzw;
        input.values[5].xyzw=(fog).xyzw;
        input.values[6].xyzw=(view).xyzw;
        input.values[7].xyz=(up).xyz;
        input.values[8].xyzw=(clip).xyzw;
        return;
    }
    if (g_SourceCharacterProgram==1156u || g_SourceCharacterProgram==1500u || g_SourceCharacterProgram==1501u || g_SourceCharacterProgram==1502u || g_SourceCharacterProgram==1503u || g_SourceCharacterProgram==1505u || g_SourceCharacterProgram==1506u || g_SourceCharacterProgram==1507u || g_SourceCharacterProgram==1508u || g_SourceCharacterProgram==1510u || g_SourceCharacterProgram==1511u || g_SourceCharacterProgram==1516u || g_SourceCharacterProgram==1519u || g_SourceCharacterProgram==1520u || g_SourceCharacterProgram==1524u)
    {
        input.values[0].xyzw=(tangentX).xyzw;
        input.values[1].xyzw=(tangentZ).xyzw;
        input.values[2].xyzw=(color).xyzw;
        input.values[3].xy=(float4(0.f,0.f,0.f,0.f)).xy;
        input.values[4].xyzw=(float4(uv,0.f,0.f)).xyzw;
        input.values[5].xyzw=(fog).xyzw;
        input.values[6].xyzw=(view).xyzw;
        input.values[7].xyzw=(clip).xyzw;
        return;
    }
    if (g_SourceCharacterProgram==1512u)
    {
        input.values[0].xyzw=(tangentX).xyzw;
        input.values[1].xyzw=(tangentZ).xyzw;
        input.values[2].xyzw=(color).xyzw;
        input.values[3].xy=(float4(0.f,0.f,0.f,0.f)).xy;
        input.values[4].xyzw=(view).xyzw;
        input.values[5].xyz=(up).xyz;
        input.values[6].xyzw=(clip).xyzw;
        return;
    }
    if (g_SourceCharacterProgram==1515u)
    {
        input.values[0].xyzw=(tangentX).xyzw;
        input.values[1].xyzw=(tangentZ).xyzw;
        input.values[2].xyzw=(color).xyzw;
        input.values[3].xy=(float4(0.f,0.f,0.f,0.f)).xy;
        input.values[4].xyzw=(float4(uv,0.f,0.f)).xyzw;
        input.values[5].xyzw=(view).xyzw;
        input.values[6].xyzw=(clip).xyzw;
        return;
    }
}
void PackSourceMovieLightInput(inout SOURCE_CHARACTER_NATIVE_INPUT input,
    float4 tangentX, float4 tangentZ, float4 color, float2 uv,
    float4 view, float4 light, float4 up, float4 clip, float4 fog)
{
    if (!IsSourceMovieStatic(g_SourceCharacterProgram)) return;
    [unroll] for (uint lane=0u;lane<10u;++lane) input.values[lane]=0.f;
    if (g_SourceCharacterProgram==1100u || g_SourceCharacterProgram==1101u || g_SourceCharacterProgram==1102u || g_SourceCharacterProgram==1103u || g_SourceCharacterProgram==1104u || g_SourceCharacterProgram==1105u || g_SourceCharacterProgram==1106u || g_SourceCharacterProgram==1107u || g_SourceCharacterProgram==1108u || g_SourceCharacterProgram==1109u || g_SourceCharacterProgram==1110u || g_SourceCharacterProgram==1111u || g_SourceCharacterProgram==1117u || g_SourceCharacterProgram==1118u || g_SourceCharacterProgram==1119u || g_SourceCharacterProgram==1120u || g_SourceCharacterProgram==1121u || g_SourceCharacterProgram==1122u || g_SourceCharacterProgram==1123u || g_SourceCharacterProgram==1124u || g_SourceCharacterProgram==1125u || g_SourceCharacterProgram==1126u || g_SourceCharacterProgram==1127u || g_SourceCharacterProgram==1128u || g_SourceCharacterProgram==1129u || g_SourceCharacterProgram==1130u || g_SourceCharacterProgram==1131u || g_SourceCharacterProgram==1132u || g_SourceCharacterProgram==1133u || g_SourceCharacterProgram==1134u || g_SourceCharacterProgram==1135u || g_SourceCharacterProgram==1136u || g_SourceCharacterProgram==1137u || g_SourceCharacterProgram==1138u || g_SourceCharacterProgram==1139u || g_SourceCharacterProgram==1140u || g_SourceCharacterProgram==1141u || g_SourceCharacterProgram==1142u || g_SourceCharacterProgram==1143u || g_SourceCharacterProgram==1144u || g_SourceCharacterProgram==1145u || g_SourceCharacterProgram==1146u || g_SourceCharacterProgram==1147u || g_SourceCharacterProgram==1148u || g_SourceCharacterProgram==1149u || g_SourceCharacterProgram==1152u || g_SourceCharacterProgram==1154u || g_SourceCharacterProgram==1155u || g_SourceCharacterProgram==1158u || g_SourceCharacterProgram==1159u || g_SourceCharacterProgram==1160u || g_SourceCharacterProgram==1161u || g_SourceCharacterProgram==1162u || g_SourceCharacterProgram==1163u || g_SourceCharacterProgram==1164u || g_SourceCharacterProgram==1402u || g_SourceCharacterProgram==1404u || g_SourceCharacterProgram==1408u || g_SourceCharacterProgram==1409u || g_SourceCharacterProgram==1411u || g_SourceCharacterProgram==1412u || g_SourceCharacterProgram==1413u || g_SourceCharacterProgram==1518u || g_SourceCharacterProgram==1521u || g_SourceCharacterProgram==1408u || g_SourceCharacterProgram==1166u || g_SourceCharacterProgram==1103u)
    {
        input.values[0].xyzw=(tangentX).xyzw;
        input.values[1].xyzw=(tangentZ).xyzw;
        input.values[2].xyzw=(color).xyzw;
        input.values[3].xy=(float4(0.f,0.f,0.f,0.f)).xy;
        input.values[4].xyzw=(float4(uv,0.f,0.f)).xyzw;
        input.values[5].xyz=(light).xyz;
        input.values[6].xyzw=(float4(0.f,0.f,0.f,0.f)).xyzw;
        input.values[7].xyzw=(view).xyzw;
        input.values[8].xyzw=(clip).xyzw;
        return;
    }
    if (g_SourceCharacterProgram==1112u || g_SourceCharacterProgram==1113u || g_SourceCharacterProgram==1114u || g_SourceCharacterProgram==1115u || g_SourceCharacterProgram==1116u || g_SourceCharacterProgram==1150u || g_SourceCharacterProgram==1151u || g_SourceCharacterProgram==1153u || g_SourceCharacterProgram==1157u || g_SourceCharacterProgram==1165u || g_SourceCharacterProgram==1400u || g_SourceCharacterProgram==1401u || g_SourceCharacterProgram==1403u || g_SourceCharacterProgram==1405u || g_SourceCharacterProgram==1406u || g_SourceCharacterProgram==1407u || g_SourceCharacterProgram==1410u || g_SourceCharacterProgram==1504u || g_SourceCharacterProgram==1509u || g_SourceCharacterProgram==1513u || g_SourceCharacterProgram==1514u || g_SourceCharacterProgram==1517u || g_SourceCharacterProgram==1522u || g_SourceCharacterProgram==1523u || g_SourceCharacterProgram==1525u)
    {
        input.values[0].xyzw=(color).xyzw;
        input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
        input.values[2].xyzw=(float4(uv,0.f,0.f)).xyzw;
        input.values[3].xyz=(light).xyz;
        input.values[4].xyzw=(float4(0.f,0.f,0.f,0.f)).xyzw;
        input.values[5].xyzw=(view).xyzw;
        input.values[6].xyzw=(clip).xyzw;
        return;
    }
    if (g_SourceCharacterProgram==1512u)
    {
        input.values[0].xyzw=(color).xyzw;
        input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
        input.values[2].xyz=(light).xyz;
        input.values[3].xyzw=(float4(0.f,0.f,0.f,0.f)).xyzw;
        input.values[4].xyzw=(view).xyzw;
        input.values[5].xyzw=(clip).xyzw;
        return;
    }
}
#endif
