#include "Effect_WarlordNativeMaterial.h"

NS_BEGIN(Client)
#include "Effect_WarlordNativeMaterial_Tables.inl"

// The backing arrays have static storage; no runtime initialization is needed.
constinit const std::span<const WARLORD_NATIVE_PROGRAM_DESC> WARLORD_NATIVE_PROGRAMS{WARLORD_NATIVE_PROGRAM_STORAGE};
NS_END
