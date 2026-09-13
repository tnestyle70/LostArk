#include "Effect_DimensionMasterALTVMaterial.h"

NS_BEGIN(Client)
#include "Effect_DimensionMasterALTVMaterial_Tables.inl"

// The backing arrays have static storage; no runtime initialization is needed.
constinit const std::span<const DIMENSIONMASTER_ALTV_PROGRAM_DESC> DIMENSIONMASTER_ALTV_PROGRAMS{DIMENSIONMASTER_ALTV_PROGRAM_STORAGE};
NS_END
