#include "Effect_DimensionMasterQMaterial.h"

NS_BEGIN(Client)
#include "Effect_DimensionMasterQMaterial_Tables.inl"

// The backing arrays have static storage; no runtime initialization is needed.
constinit const std::span<const DIMENSIONMASTER_Q_PROGRAM_DESC> DIMENSIONMASTER_Q_PROGRAMS{DIMENSIONMASTER_Q_PROGRAM_STORAGE};
NS_END
