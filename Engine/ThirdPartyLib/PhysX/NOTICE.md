# NVIDIA PhysX redistribution notice

This directory contains NVIDIA PhysX SDK headers and Windows x64 binaries built
from the official `NVIDIA-Omniverse/PhysX` source tag
`107.3-physx-5.6.1` (commit
`5ca9f472105a90d70d957c243cb0ef36fe251a9f`).

Copyright (c) NVIDIA Corporation. The bundled headers and binaries are
redistributed under the BSD 3-Clause terms reproduced in [LICENSE.md](LICENSE.md).
The license notice must remain with source redistribution and must be reproduced
in documentation or other materials distributed with the binaries.

The checked-in binaries were generated with the official
`vc17win64-cpu-only` preset, Visual Studio 2022 v143, Windows SDK 10.0.26100,
and the dynamic MSVC runtime (`/MDd` for Debug, `/MD` for Release). Only the
preset's `NV_USE_STATIC_WINCRT` setting was changed from `True` to `False` so
the SDK CRT matches this Engine.
