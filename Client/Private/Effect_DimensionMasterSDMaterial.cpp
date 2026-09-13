#include "Effect_DimensionMasterSDMaterial.h"

NS_BEGIN(Client)
#include "Effect_DimensionMasterSDMaterial_Tables.inl"

// The backing arrays have static storage; no runtime initialization is needed.
constinit const std::span<const DIMENSIONMASTER_SD_PROGRAM_DESC> DIMENSIONMASTER_SD_PROGRAMS{DIMENSIONMASTER_SD_PROGRAM_STORAGE};
NS_END
