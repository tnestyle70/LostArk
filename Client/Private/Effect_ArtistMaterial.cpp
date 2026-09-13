#include "Effect_ArtistMaterial.h"

NS_BEGIN(Client)
#include "Effect_ArtistMaterial_Tables.inl"

// The backing arrays have static storage; no runtime initialization is needed.
constinit const std::span<const ARTIST_PROGRAM_DESC> ARTIST_PROGRAMS{ARTIST_PROGRAM_STORAGE};
NS_END
