# MaskedOcclusionCulling

Upstream: https://github.com/GameTechDev/MaskedOcclusionCulling

Pinned commit: `1fd7974456cffa481a1a534328a1d02523d19ce8`

License: Apache-2.0 (`license.txt`). Source files are unmodified upstream files;
`UPSTREAM_README.md` preserves upstream documentation. Upstream was archived on
2026-09-07. This repository maintains its integration.

Engine compiles `USE_D3D=1`, `PRECISE_COVERAGE=1`, `USE_AVX512=0`. The baseline TU
uses the x64/SSE2 ISA; only the AVX2 TU enables AVX2. The upstream AVX512 TU builds
its disabled fallback factory. Runtime requests AVX2 and allows SSE4.1/SSE2
fallback. No external DLL, worker thread, GPU readback or installation is needed.

`COcclusionCuller` owns viewport-aligned pixel coverage, perspective
validation, near-plane fail-open, depth bias and bounded current-frame scratch.
