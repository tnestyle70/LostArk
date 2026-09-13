#include "Effect_DimensionMasterVMaterial.h"

NS_BEGIN(Client)
#include "Effect_DimensionMasterVMaterial_Tables.inl"

// The backing arrays have static storage; no runtime initialization is needed.
constinit const std::span<const DIMENSIONMASTER_V_PROGRAM_DESC> DIMENSIONMASTER_V_PROGRAMS{DIMENSIONMASTER_V_PROGRAM_STORAGE};
NS_END
