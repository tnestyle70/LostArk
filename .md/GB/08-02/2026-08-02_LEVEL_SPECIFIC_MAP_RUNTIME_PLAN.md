# Level-specific map runtime plan

## Status

- Completed on 2026-08-02.
- Debug/Release builds, map generator tests, and direct Bern/Valtan runtime smoke tests passed.

## Goal

- `LEVEL::BAREN` loads the fixed `LV_BER_BERNCASTLE` map.
- `LEVEL::VALTAN_ARENA` loads the fixed `LV_LUT_HEARTRB_ED` map.
- `LEVEL::ASSET_TEST` continues to use `ACTIVE.maparea` for MapTool editing.
- Only map runtime and area navigation prototype registration are changed. Character, boss, combat, effect, and deploy-prop behavior stay out of scope.

## Implementation

1. Add `CMapAssetCatalog::Load_Area()` and make `Load_Default()` delegate to it after reading `ACTIVE.maparea`.
2. Extract placement reading, batching, fallback creation, and rollback into a Release-safe common map runtime shared by MapTool and actual levels.
3. Pass the target prototype level explicitly through `CMapAssetObject` and `CMapStaticBatchObject` descriptors.
4. Add a map-only Loader helper for shaders, map models, map objects, free camera, and the area navigation prototype when a runtime grid exists.
5. Connect `BAREN` and `VALTAN_ARENA` through Loading and create only their map, light, and inspection camera.
6. Preserve all existing uncommitted AssetTest and Valtan combat changes.

## Validation

- Confirm `BAREN`, `VALTAN_ARENA`, and `ASSET_TEST` use separate catalog selection paths.
- Confirm failed placement staging rolls back staged objects and does not replace an existing runtime.
- Build Engine, run UpdateLib, then build Client in x64 Debug and Release.
- Report runtime data limitations separately: Bern has no finished navgrid. Valtan Landscape 6 assets / 6 placements are now part of the single `LV_LUT_HEARTRB_ED` catalog, while their WModel/textures remain in the shared runtime resource pack.
