#pragma once

#include "DataJson.h"
#include "BinaryAsset/ModelAssetData.h"
#include <array>
#include <map>
#include <string>
#include <string_view>

namespace Client::SourceCharacterMaterial
{
// Parameters retain the native source names. Packing is compiled by one owner.
using PARAMETER_VALUES = std::map<std::string, std::array<float, 4>>;

bool Read(const DATA_JSON_VALUE& parameters, PARAMETER_VALUES& result);
bool Configure(const std::string& family, const PARAMETER_VALUES& parameters,
    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& result);
bool Patch_NamedVector(Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& material,
    std::string_view parameter, const float4_t& value);
bool Configure(const std::string& family, const DATA_JSON_VALUE& parameters,
    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& result);
}
