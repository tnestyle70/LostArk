#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
#include "Shader_SourceCharacterLightGroup001.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
#include "Shader_SourceCharacterLightGroup009.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
#include "Shader_SourceCharacterLightGroup017.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
#include "Shader_SourceCharacterLightGroup025.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 80
#include "Shader_SourceCharacterLightGroup080.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
#include "Shader_SourceCharacterLightGroup084.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
#include "Shader_SourceCharacterLightGroup160.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
#include "Shader_SourceCharacterLightGroup176.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
#include "Shader_SourceCharacterLightGroup192.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 208
#include "Shader_SourceCharacterLightGroup208.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 235
#include "Shader_SourceCharacterLightGroup235.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 214
#include "Shader_SourceCharacterLightGroup214.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP

#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 224
#include "Shader_SourceCharacterLightGroup224.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP

#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 237
#include "Shader_SourceCharacterLightGroup237.hlsli"
#endif // SOURCE_CHARACTER_PROGRAM_GROUP

SOURCE_CHARACTER_NATIVE_OUTPUT EvaluateSourceCharacterLight(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    switch (g_SourceCharacterProgram)
    {
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
    case 1u: return SourceCharacterLight1(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
    case 2u: return SourceCharacterLight2(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
    case 3u: return SourceCharacterLight3(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
    case 4u: return SourceCharacterLight4(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
    case 5u: return SourceCharacterLight5(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
    case 6u: return SourceCharacterLight6(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
    case 7u: return SourceCharacterLight7(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 1
    case 8u: return SourceCharacterLight8(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
    case 9u: return SourceCharacterLight9(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
    case 10u: return SourceCharacterLight10(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
    case 11u: return SourceCharacterLight11(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
    case 12u: return SourceCharacterLight12(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
    case 13u: return SourceCharacterLight13(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
    case 14u: return SourceCharacterLight14(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
    case 15u: return SourceCharacterLight15(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 9
    case 16u: return SourceCharacterLight16(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
    case 17u: return SourceCharacterLight17(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
    case 18u: return SourceCharacterLight18(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
    case 19u: return SourceCharacterLight19(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
    case 20u: return SourceCharacterLight20(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
    case 21u: return SourceCharacterLight21(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
    case 22u: return SourceCharacterLight22(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
    case 23u: return SourceCharacterLight23(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 17
    case 24u: return SourceCharacterLight24(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
    case 25u: return SourceCharacterLight25(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
    case 26u: return SourceCharacterLight26(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
    case 27u: return SourceCharacterLight27(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
    case 28u: return SourceCharacterLight28(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
    case 29u: return SourceCharacterLight29(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
    case 30u: return SourceCharacterLight30(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
    case 31u: return SourceCharacterLight31(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 25
    case 32u: return SourceCharacterLight32(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 84u: return SourceCharacterLight84(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 85u: return SourceCharacterLight85(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 86u: return SourceCharacterLight86(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 87u: return SourceCharacterLight87(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 88u: return SourceCharacterLight88(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 89u: return SourceCharacterLight89(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 90u: return SourceCharacterLight90(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 91u: return SourceCharacterLight91(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 92u: return SourceCharacterLight92(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 93u: return SourceCharacterLight93(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 94u: return SourceCharacterLight94(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 95u: return SourceCharacterLight95(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 96u: return SourceCharacterLight96(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 97u: return SourceCharacterLight97(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 98u: return SourceCharacterLight98(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 99u: return SourceCharacterLight99(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 100u: return SourceCharacterLight100(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 101u: return SourceCharacterLight101(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 102u: return SourceCharacterLight102(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 103u: return SourceCharacterLight103(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 104u: return SourceCharacterLight104(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 105u: return SourceCharacterLight105(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 106u: return SourceCharacterLight106(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 107u: return SourceCharacterLight107(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 108u: return SourceCharacterLight108(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 109u: return SourceCharacterLight109(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 110u: return SourceCharacterLight110(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 111u: return SourceCharacterLight111(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 84
    case 112u: return SourceCharacterLight112(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 80
    case 80u: return SourceCharacterLight80(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 80
    case 81u: return SourceCharacterLight81(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 80
    case 82u: return SourceCharacterLight82(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 80
    case 83u: return SourceCharacterLight83(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 160u: return SourceCharacterLight160(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 161u: return SourceCharacterLight161(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 162u: return SourceCharacterLight162(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 163u: return SourceCharacterLight163(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 164u: return SourceCharacterLight164(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 165u: return SourceCharacterLight165(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 166u: return SourceCharacterLight166(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 167u: return SourceCharacterLight167(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 168u: return SourceCharacterLight168(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 169u: return SourceCharacterLight169(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 170u: return SourceCharacterLight170(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 171u: return SourceCharacterLight171(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 172u: return SourceCharacterLight172(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 173u: return SourceCharacterLight173(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 174u: return SourceCharacterLight174(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 160
    case 175u: return SourceCharacterLight175(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 176u: return SourceCharacterLight176(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 177u: return SourceCharacterLight177(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 178u: return SourceCharacterLight178(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 179u: return SourceCharacterLight179(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 180u: return SourceCharacterLight180(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 181u: return SourceCharacterLight181(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 182u: return SourceCharacterLight182(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 183u: return SourceCharacterLight183(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 184u: return SourceCharacterLight184(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 185u: return SourceCharacterLight185(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 186u: return SourceCharacterLight186(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 187u: return SourceCharacterLight187(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 188u: return SourceCharacterLight188(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 189u: return SourceCharacterLight189(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 190u: return SourceCharacterLight190(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 176
    case 191u: return SourceCharacterLight191(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 192u: return SourceCharacterLight192(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 193u: return SourceCharacterLight193(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 194u: return SourceCharacterLight194(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 195u: return SourceCharacterLight195(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 196u: return SourceCharacterLight196(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 197u: return SourceCharacterLight197(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 198u: return SourceCharacterLight198(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 199u: return SourceCharacterLight199(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 192
    case 200u: return SourceCharacterLight200(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 208
    case 208u: return SourceCharacterLight208(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 208
    case 210u: return SourceCharacterLight210(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 208
    case 211u: return SourceCharacterLight211(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 208
    case 212u: return SourceCharacterLight212(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 208
    case 213u: return SourceCharacterLight213(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 235
    case 235u: return SourceCharacterLight235(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 235
    case 236u: return SourceCharacterLight236(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 214
    case 214u: return SourceCharacterLight214(input);
    case 215u: return SourceCharacterLight215(input);
    case 216u: return SourceCharacterLight216(input);
    case 217u: return SourceCharacterLight217(input);
    case 218u: return SourceCharacterLight218(input);
    case 219u: return SourceCharacterLight219(input);
    case 220u: return SourceCharacterLight220(input);
    case 221u: return SourceCharacterLight221(input);
    case 222u: return SourceCharacterLight222(input);
    case 223u: return SourceCharacterLight223(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 224
    case 224u: return SourceCharacterLight224(input);
    case 225u: return SourceCharacterLight225(input);
    case 226u: return SourceCharacterLight226(input);
    case 227u: return SourceCharacterLight227(input);
    case 228u: return SourceCharacterLight228(input);
    case 229u: return SourceCharacterLight229(input);
    case 230u: return SourceCharacterLight230(input);
    case 231u: return SourceCharacterLight231(input);
    case 232u: return SourceCharacterLight232(input);
    case 233u: return SourceCharacterLight233(input);
    case 234u: return SourceCharacterLight234(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == 237
    case 237u: return SourceCharacterLight237(input);
#endif // SOURCE_CHARACTER_PROGRAM_GROUP
    default: { SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;output.discarded=true;return output; }
    }
}
