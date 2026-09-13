#include "Effect_LanceMasterVAMaterial.h"

NS_BEGIN(Client)
#include "Effect_LanceMasterVAMaterial_Tables.inl"

// The backing arrays have static storage; no runtime initialization is needed.
constinit const std::span<const LANCEMASTER_VA_PROGRAM_DESC> LANCEMASTER_VA_PROGRAMS{LANCEMASTER_VA_PROGRAM_STORAGE};
NS_END
