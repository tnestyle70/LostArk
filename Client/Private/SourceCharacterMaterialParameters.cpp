#include "SourceCharacterMaterialParameters.h"
#include "SourceCharacterMaterialParameters_Generated.inl"

namespace Client::SourceCharacterMaterial
{
bool Read(const DATA_JSON_VALUE& parameters, PARAMETER_VALUES& result)
{
    return Detail::Read(parameters, result);
}

bool Configure(const std::string& family, const PARAMETER_VALUES& parameters,
    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& result)
{
    return Detail::Configure(family, parameters, result);
}

bool Patch_NamedVector(Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& material,
    std::string_view parameter, const float4_t& value)
{
    return Detail::Patch_NamedVector(material, parameter, value);
}

bool Configure(const std::string& family, const DATA_JSON_VALUE& parameters,
    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& result)
{
    return Detail::Configure(family, parameters, result);
}
}
