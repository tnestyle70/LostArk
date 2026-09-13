#include "Effect_DimensionMasterWRMaterial.h"

NS_BEGIN(Client)
#include "Effect_DimensionMasterWRMaterial_Tables.inl"

// The backing arrays have static storage; no runtime initialization is needed.
constinit const std::span<const DIMENSIONMASTER_WR_PROGRAM_DESC> DIMENSIONMASTER_WR_PROGRAMS{DIMENSIONMASTER_WR_PROGRAM_STORAGE};
NS_END
