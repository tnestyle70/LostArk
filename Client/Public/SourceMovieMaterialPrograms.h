#pragma once
#include <cstdint>

namespace Client::SourceMovieMaterial
{
inline bool Is_Forward(uint32_t program)
{
    return program==1101u || program==1102u || program==1106u || program==1107u || program==1109u || program==1113u || program==1114u || program==1117u || program==1140u || program==1141u || program==1142u || program==1143u || program==1150u || program==1153u || program==1156u || program==1159u || program==1400u || program==1401u || program==1403u || program==1405u || program==1406u || program==1407u || program==1411u || program==1412u || program==1413u || program==1500u || program==1501u || program==1502u || program==1503u || program==1505u || program==1506u || program==1507u || program==1508u || program==1509u || program==1510u || program==1511u || program==1513u || program==1516u || program==1517u || program==1519u || program==1520u || program==1522u || program==1523u || program==1524u || program==1525u;
}
inline bool Is_Additive(uint32_t program)
{
    return program==1500u || program==1501u || program==1502u;
}
inline bool Needs_SceneColor(uint32_t program)
{
    return program==1510u || program==1511u;
}
inline bool Is_Static(uint32_t program)
{
    return (program>=1100u && program<=1166u) || (program>=1400u && program<=1413u) || (program>=1500u && program<=1525u);
}
}
