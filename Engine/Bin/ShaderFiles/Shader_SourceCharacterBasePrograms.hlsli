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
    default: { SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;output.discarded=true;return output; }
    }
}
