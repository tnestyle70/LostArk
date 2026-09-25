#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
#include "Shader_SourceCharacterBaseGroup001.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
#include "Shader_SourceCharacterBaseGroup009.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
#include "Shader_SourceCharacterBaseGroup017.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
#include "Shader_SourceCharacterBaseGroup025.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 80
#include "Shader_SourceCharacterBaseGroup080.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
#include "Shader_SourceCharacterBaseGroup084.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
#include "Shader_SourceCharacterBaseGroup160.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
#include "Shader_SourceCharacterBaseGroup176.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
#include "Shader_SourceCharacterBaseGroup192.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 208
#include "Shader_SourceCharacterBaseGroup208.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 235
#include "Shader_SourceCharacterBaseGroup235.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 214
#include "Shader_SourceCharacterBaseGroup214.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 224
#include "Shader_SourceCharacterBaseGroup224.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 237
#include "Shader_SourceCharacterBaseGroup237.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 576
#include "Shader_SourceCharacterBaseGroup576.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 640
#include "Shader_SourceCharacterBaseGroup640.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 768
#include "Shader_SourceCharacterBaseGroup768.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 896
#include "Shader_SourceCharacterBaseGroup896.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
#include "Shader_SourceCharacterBaseGroup1088.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1152
#include "Shader_SourceCharacterBaseGroup1152.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1344
#include "Shader_SourceCharacterBaseGroup1344.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1408
#include "Shader_SourceCharacterBaseGroup1408.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
#include "Shader_SourceCharacterBaseGroup1472.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
SOURCE_CHARACTER_NATIVE_OUTPUT EvaluateSourceCharacterBase(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    switch (g_SourceCharacterProgram)
    {
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
    case 1u: return SourceCharacterBase1(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
    case 2u: return SourceCharacterBase2(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
    case 3u: return SourceCharacterBase3(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
    case 4u: return SourceCharacterBase4(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
    case 5u: return SourceCharacterBase5(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
    case 6u: return SourceCharacterBase6(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
    case 7u: return SourceCharacterBase7(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
    case 8u: return SourceCharacterBase8(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
    case 9u: return SourceCharacterBase9(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
    case 10u: return SourceCharacterBase10(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
    case 11u: return SourceCharacterBase11(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
    case 12u: return SourceCharacterBase12(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
    case 13u: return SourceCharacterBase13(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
    case 14u: return SourceCharacterBase14(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
    case 15u: return SourceCharacterBase15(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
    case 16u: return SourceCharacterBase16(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
    case 17u: return SourceCharacterBase17(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
    case 18u: return SourceCharacterBase18(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
    case 19u: return SourceCharacterBase19(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
    case 20u: return SourceCharacterBase20(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
    case 21u: return SourceCharacterBase21(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
    case 22u: return SourceCharacterBase22(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
    case 23u: return SourceCharacterBase23(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
    case 24u: return SourceCharacterBase24(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
    case 25u: return SourceCharacterBase25(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
    case 26u: return SourceCharacterBase26(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
    case 27u: return SourceCharacterBase27(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
    case 28u: return SourceCharacterBase28(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
    case 29u: return SourceCharacterBase29(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
    case 30u: return SourceCharacterBase30(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
    case 31u: return SourceCharacterBase31(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
    case 32u: return SourceCharacterBase32(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 84u: return SourceCharacterBase84(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 85u: return SourceCharacterBase85(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 86u: return SourceCharacterBase86(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 87u: return SourceCharacterBase87(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 88u: return SourceCharacterBase88(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 89u: return SourceCharacterBase89(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 90u: return SourceCharacterBase90(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 91u: return SourceCharacterBase91(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 92u: return SourceCharacterBase92(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 93u: return SourceCharacterBase93(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 94u: return SourceCharacterBase94(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 95u: return SourceCharacterBase95(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 96u: return SourceCharacterBase96(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 97u: return SourceCharacterBase97(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 98u: return SourceCharacterBase98(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 99u: return SourceCharacterBase99(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 100u: return SourceCharacterBase100(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 101u: return SourceCharacterBase101(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 102u: return SourceCharacterBase102(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 103u: return SourceCharacterBase103(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 104u: return SourceCharacterBase104(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 105u: return SourceCharacterBase105(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 106u: return SourceCharacterBase106(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 107u: return SourceCharacterBase107(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 108u: return SourceCharacterBase108(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 109u: return SourceCharacterBase109(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 110u: return SourceCharacterBase110(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 111u: return SourceCharacterBase111(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 112u: return SourceCharacterBase112(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 80
    case 80u: { if (input.hasBakedLighting) return SourceMapMonsterBaked80(input); return SourceCharacterBase80(input); }
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 80
    case 81u: { if (input.hasBakedLighting) return SourceMapMonsterBaked81(input); return SourceCharacterBase81(input); }
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 80
    case 82u: { if (input.hasBakedLighting) return SourceMapMonsterBaked82(input); return SourceCharacterBase82(input); }
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 80
    case 83u: { if (input.hasBakedLighting) return SourceMapMonsterBaked83(input); return SourceCharacterBase83(input); }
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 160u: return SourceCharacterBase160(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 161u: return SourceCharacterBase161(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 162u: return SourceCharacterBase162(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 163u: return SourceCharacterBase163(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 164u: return SourceCharacterBase164(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 165u: return SourceCharacterBase165(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 166u: return SourceCharacterBase166(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 167u: return SourceCharacterBase167(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 168u: return SourceCharacterBase168(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 169u: return SourceCharacterBase169(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 170u: return SourceCharacterBase170(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 171u: return SourceCharacterBase171(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 172u: return SourceCharacterBase172(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 173u: return SourceCharacterBase173(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 174u: return SourceCharacterBase174(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 175u: return SourceCharacterBase175(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 176u: return SourceCharacterBase176(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 177u: return SourceCharacterBase177(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 178u: return SourceCharacterBase178(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 179u: return SourceCharacterBase179(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 180u: return SourceCharacterBase180(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 181u: return SourceCharacterBase181(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 182u: return SourceCharacterBase182(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 183u: return SourceCharacterBase183(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 184u: return SourceCharacterBase184(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 185u: return SourceCharacterBase185(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 186u: return SourceCharacterBase186(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 187u: return SourceCharacterBase187(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 188u: return SourceCharacterBase188(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 189u: return SourceCharacterBase189(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 190u: return SourceCharacterBase190(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 191u: return SourceCharacterBase191(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 192u: return SourceCharacterBase192(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 193u: return SourceCharacterBase193(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 194u: return SourceCharacterBase194(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 195u: return SourceCharacterBase195(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 196u: return SourceCharacterBase196(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 197u: return SourceCharacterBase197(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 198u: return SourceCharacterBase198(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 199u: return SourceCharacterBase199(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 200u: return SourceCharacterBase200(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 208
    case 208u: return SourceCharacterBase208(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 208
    case 210u: return SourceCharacterBase210(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 208
    case 211u: return SourceCharacterBase211(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 208
    case 212u: return SourceCharacterBase212(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 208
    case 213u: return SourceCharacterBase213(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 235
    case 235u: return SourceCharacterBase235(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 235
    case 236u: return SourceCharacterBase236(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 214
    case 214u: if(input.hasBakedLighting) return SourceMapMonsterBaked214(input); return SourceCharacterBase214(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 214
    case 215u: if(input.hasBakedLighting) return SourceMapMonsterBaked215(input); return SourceCharacterBase215(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 214
    case 216u: if(input.hasBakedLighting) return SourceMapMonsterBaked216(input); return SourceCharacterBase216(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 214
    case 217u: if(input.hasBakedLighting) return SourceMapMonsterBaked217(input); return SourceCharacterBase217(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 214
    case 218u: if(input.hasBakedLighting) return SourceMapMonsterBaked218(input); return SourceCharacterBase218(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 214
    case 219u: if(input.hasBakedLighting) return SourceMapMonsterBaked219(input); return SourceCharacterBase219(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 214
    case 220u: if(input.hasBakedLighting) return SourceMapMonsterBaked220(input); return SourceCharacterBase220(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 214
    case 221u: if(input.hasBakedLighting) return SourceMapMonsterBaked221(input); return SourceCharacterBase221(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 214
    case 222u: if(input.hasBakedLighting) return SourceMapMonsterBaked222(input); return SourceCharacterBase222(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 214
    case 223u: if(input.hasBakedLighting) return SourceMapMonsterBaked223(input); return SourceCharacterBase223(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 224
    case 224u: if(input.hasBakedLighting) return SourceMapMonsterBaked224(input); return SourceCharacterBase224(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 224
    case 225u: if(input.hasBakedLighting) return SourceMapMonsterBaked225(input); return SourceCharacterBase225(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 224
    case 226u: if(input.hasBakedLighting) return SourceMapMonsterBaked226(input); return SourceCharacterBase226(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 224
    case 227u: if(input.hasBakedLighting) return SourceMapMonsterBaked227(input); return SourceCharacterBase227(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 224
    case 228u: if(input.hasBakedLighting) return SourceMapMonsterBaked228(input); return SourceCharacterBase228(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 224
    case 229u: if(input.hasBakedLighting) return SourceMapMonsterBaked229(input); return SourceCharacterBase229(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 224
    case 230u: if(input.hasBakedLighting) return SourceMapMonsterBaked230(input); return SourceCharacterBase230(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 224
    case 231u: if(input.hasBakedLighting) return SourceMapMonsterBaked231(input); return SourceCharacterBase231(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 224
    case 232u: if(input.hasBakedLighting) return SourceMapMonsterBaked232(input); return SourceCharacterBase232(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 224
    case 233u: if(input.hasBakedLighting) return SourceMapMonsterBaked233(input); return SourceCharacterBase233(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 224
    case 234u: if(input.hasBakedLighting) return SourceMapMonsterBaked234(input); return SourceCharacterBase234(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 237
    case 237u: if(input.hasBakedLighting) return SourceMapMonsterBaked237(input); return SourceCharacterBase237(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 237
    case 238u: return SourceCharacterBase238(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 576
    case 600u: return SourceCharacterBase600(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 576
    case 601u: return SourceCharacterBase601(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 640
    case 700u: return SourceCharacterBase700(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 640
    case 701u: return SourceCharacterBase701(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 640
    case 702u: return SourceCharacterBase702(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 640
    case 703u: return SourceCharacterBase703(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 768
    case 800u: return SourceCharacterBase800(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 768
    case 801u: return SourceCharacterBase801(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 896
    case 901u: return SourceCharacterBase901(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 896
    case 902u: return SourceCharacterBase902(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 896
    case 903u: return SourceCharacterBase903(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 896
    case 904u: return SourceCharacterBase904(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1100u: return SourceCharacterBase1100(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1101u: return SourceCharacterBase1101(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1102u: return SourceCharacterBase1102(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1103u: return SourceCharacterBase1103(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1104u: return SourceCharacterBase1104(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1105u: return SourceCharacterBase1105(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1106u: return SourceCharacterBase1106(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1107u: return SourceCharacterBase1107(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1108u: return SourceCharacterBase1108(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1109u: return SourceCharacterBase1109(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1110u: return SourceCharacterBase1110(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1111u: return SourceCharacterBase1111(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1112u: return SourceCharacterBase1112(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1113u: return SourceCharacterBase1113(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1114u: return SourceCharacterBase1114(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1115u: return SourceCharacterBase1115(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1116u: return SourceCharacterBase1116(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1117u: return SourceCharacterBase1117(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1118u: return SourceCharacterBase1118(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1119u: return SourceCharacterBase1119(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1120u: return SourceCharacterBase1120(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1121u: return SourceCharacterBase1121(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1122u: return SourceCharacterBase1122(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1123u: return SourceCharacterBase1123(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1124u: return SourceCharacterBase1124(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1125u: return SourceCharacterBase1125(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1126u: return SourceCharacterBase1126(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1127u: return SourceCharacterBase1127(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1128u: return SourceCharacterBase1128(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1129u: return SourceCharacterBase1129(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1130u: return SourceCharacterBase1130(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1131u: return SourceCharacterBase1131(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1132u: return SourceCharacterBase1132(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1133u: return SourceCharacterBase1133(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1134u: return SourceCharacterBase1134(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1135u: return SourceCharacterBase1135(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1136u: return SourceCharacterBase1136(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1137u: return SourceCharacterBase1137(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1138u: return SourceCharacterBase1138(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1139u: return SourceCharacterBase1139(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1140u: return SourceCharacterBase1140(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1141u: return SourceCharacterBase1141(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1142u: return SourceCharacterBase1142(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1143u: return SourceCharacterBase1143(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1144u: return SourceCharacterBase1144(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1145u: return SourceCharacterBase1145(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1146u: return SourceCharacterBase1146(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1147u: return SourceCharacterBase1147(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1148u: return SourceCharacterBase1148(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1149u: return SourceCharacterBase1149(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1150u: return SourceCharacterBase1150(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1088
    case 1151u: return SourceCharacterBase1151(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1152
    case 1152u: return SourceCharacterBase1152(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1152
    case 1153u: return SourceCharacterBase1153(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1152
    case 1154u: return SourceCharacterBase1154(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1152
    case 1155u: return SourceCharacterBase1155(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1152
    case 1156u: return SourceCharacterBase1156(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1152
    case 1157u: return SourceCharacterBase1157(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1152
    case 1158u: return SourceCharacterBase1158(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1152
    case 1159u: return SourceCharacterBase1159(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1152
    case 1160u: return SourceCharacterBase1160(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1152
    case 1161u: return SourceCharacterBase1161(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1152
    case 1162u: return SourceCharacterBase1162(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1152
    case 1163u: return SourceCharacterBase1163(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1152
    case 1164u: return SourceCharacterBase1164(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1152
    case 1165u: return SourceCharacterBase1165(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1152
    case 1166u: return SourceCharacterBase1166(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1344
    case 1400u: return SourceCharacterBase1400(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1344
    case 1401u: return SourceCharacterBase1401(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1344
    case 1402u: return SourceCharacterBase1402(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1344
    case 1403u: return SourceCharacterBase1403(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1344
    case 1404u: return SourceCharacterBase1404(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1344
    case 1405u: return SourceCharacterBase1405(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1344
    case 1406u: return SourceCharacterBase1406(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1344
    case 1407u: return SourceCharacterBase1407(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1408
    case 1408u: return SourceCharacterBase1408(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1408
    case 1409u: return SourceCharacterBase1409(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1408
    case 1410u: return SourceCharacterBase1410(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1408
    case 1411u: return SourceCharacterBase1411(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1408
    case 1412u: return SourceCharacterBase1412(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1408
    case 1413u: return SourceCharacterBase1413(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1500u: return SourceCharacterBase1500(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1501u: return SourceCharacterBase1501(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1502u: return SourceCharacterBase1502(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1503u: return SourceCharacterBase1503(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1504u: return SourceCharacterBase1504(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1505u: return SourceCharacterBase1505(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1506u: return SourceCharacterBase1506(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1507u: return SourceCharacterBase1507(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1508u: return SourceCharacterBase1508(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1509u: return SourceCharacterBase1509(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1510u: return SourceCharacterBase1510(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1511u: return SourceCharacterBase1511(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1512u: return SourceCharacterBase1512(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1513u: return SourceCharacterBase1513(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1514u: return SourceCharacterBase1514(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1515u: return SourceCharacterBase1515(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1516u: return SourceCharacterBase1516(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1517u: return SourceCharacterBase1517(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1518u: return SourceCharacterBase1518(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1519u: return SourceCharacterBase1519(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1520u: return SourceCharacterBase1520(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1521u: return SourceCharacterBase1521(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1522u: return SourceCharacterBase1522(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1523u: return SourceCharacterBase1523(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1524u: return SourceCharacterBase1524(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1472
    case 1525u: return SourceCharacterBase1525(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
    default: { SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;output.discarded=true;return output; }
    }
}
