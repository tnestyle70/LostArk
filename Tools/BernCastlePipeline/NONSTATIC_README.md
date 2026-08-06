# Bern Castle non-static source manifest

`build_bern_castle_nonstatic_manifest.py` freezes the original Bern Castle
non-static evidence before Decal, foliage, particle, light, fog, wind, and water
runtime paths are implemented. It is deliberately not a renderer or an asset
converter.

## Input contract

- Only levels whose logical package starts with `LV_BER_BERNCASTLE_T_` are read.
- Level package paths come from the verified `*.placements.json` files. The
  package export count must still match the placement extraction receipt.
- The exact 950-asset static manifest supplies stable water/foliage availability
  joins. Water candidates are the 15 exact asset paths containing one of
  `water`, `fountain`, `river`, `pond`, `pool`, `canal`, or `ocean`.
- `--source-root` is optional, but should point at the 950 exact source packs so
  water material and texture role receipts can be embedded. Missing receipts are
  written as missing-reference records; they are never guessed by filename.

Every component carries its source level, export index/path, owner actor,
transform evidence, typed object references, serial offset/size/hash, tagged
properties, and the undecoded native-prefix/native-suffix offsets and hashes.
Foliage instance transforms are explicitly marked as not decoded; the tool does
not pretend the component transform is the 1,697 components' per-instance
placement array. Dominant lights also retain their large native shadow prefix as
inventory evidence rather than treating it as ordinary tagged properties.

## Exact Bern Castle gates

The default run rejects drift from all of these measured counts:

| Evidence | Count |
|---|---:|
| Core level packages | 22 |
| DecalComponent / DecalActor | 86 / 86 |
| InstancedStaticMeshComponent / InstancedFoliageActor | 1,697 / 14 |
| ParticleSystemComponent / Emitter | 1,373 / 1,373 |
| Light components | 326 |
| ExponentialHeightFogComponent | 1 |
| WindDirectionalSourceComponent | 1 |
| Water candidate placements / unique assets | 108 / 15 |
| Unique decal materials | 11 |
| Unique foliage meshes | 15 |
| Unique particle systems | 109 |
| Total manifest items | 3,592 |

`--expected key=count` may replace the full default gate set for an isolated
fixture. Production generation should use the defaults.

## Production command

Use the bundled Python executable available on the workstation:

```powershell
& $BundledPython Tools/BernCastlePipeline/build_bern_castle_nonstatic_manifest.py `
  --placements C:\LostArkExtract\bern\placements `
  --placements C:\LostArkExtract\bern\placements_rest `
  --static-assets C:\LostArkExtract\bern_full\manifests\bern_castle_assets.json `
  --source-root C:\LostArkExtract\bern_full\source `
  --output C:\LostArkExtract\bern_full\manifests\bern_castle_nonstatic.json `
  --summary
```

Add `--dry-run` to perform parse, stage, validation, duplicate-ID checks, and all
count gates without touching the output. A normal run writes a temporary sibling,
reopens and validates it, then atomically replaces the final JSON. A failed run
leaves the previous manifest intact.

## Runtime boundary

This manifest records what exists and what is missing. It does not claim that
the current renderer supports the data. Implement and verify runtime paths in
this order: light lifecycle/proxy, water material, decal projection, foliage
native-tail decoder, ParticleSystem placement, fog/wind, then baked lightmaps.

## Exact foliage mesh supplement

The non-static manifest currently identifies 11 exact StaticMesh object paths
used by 1,407 foliage components but absent from the base 950-asset manifest.
`build_bern_castle_foliage_supplement.py` reuses the exact UModel export and
ModelAssetConverter cook functions from `build_bern_castle_assets.py`. It writes
only below `C:\LostArkExtract\bern_full\foliage`; it does not edit the base
manifests, mapset, placements, or the undecoded per-instance transforms.

```powershell
& $BundledPython Tools/BernCastlePipeline/build_bern_castle_foliage_supplement.py all `
  --umodel 'C:\Users\USER\OneDrive\바탕 화면\UModel\umodel_win32\umodel_lostark_v7.exe' `
  --package-root 'C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages' `
  --converter Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe `
  --workers 2
```

The command gates the evidence at 11 assets and 1,407 component references,
exports each exact package/object with material properties and referenced
textures, cooks with `--pretransform --no-auto-textures --scale 100`, validates
every receipt hash plus the `WINT`/`WMOD` header and converter `info` output,
then writes these separate manifests:

```text
C:\LostArkExtract\bern_full\foliage\manifests\
  bern_castle_foliage_supplement_assets.json
  bern_castle_foliage_supplement_runtime_assets.json
```

Re-run `verify --converter <path>` to re-hash every source/runtime output and
atomically refresh only the supplemental runtime manifest.

## Native foliage instance overlay

`build_bern_castle_foliage_overlay.py` reopens the original UPKs recorded by the
non-static manifest and strictly verifies the native suffix before producing a
MapTool overlay. The verified Bern layout is version, instance count, one
ShadowMap2D export reference per instance, one to five 16-byte light GUID
blocks, the fixed native cache tail, element size `80`, repeated instance count,
and exactly `N` instance records. Each 80-byte record is a 64-byte UE3 `FMatrix`
followed by lightmap and shadowmap UV biases.

The production gates are 1,697 components, 17,651 instances, 15 unique meshes,
and zero source/hash/layout errors. Four meshes reuse their existing base
950-asset IDs. Only the 11 supplemental meshes are emitted in the overlay
`assets` array; their model paths are rooted at
`Map/LV_BER_BERNCASTLE/<assetId>/<assetId>.wmodel`.

Run the complete source audit without writing output first:

```powershell
& $BundledPython Tools/BernCastlePipeline/build_bern_castle_foliage_overlay.py `
  --dry-run
```

After the dry run passes, omit `--dry-run` to atomically write:

```text
C:\LostArkExtract\bern_full\manifests\bern_castle_foliage_overlay.json
```

The overlay keeps the exact component export identity, serial and native-suffix
hashes, ShadowMap2D references, source UE3 matrix, and UV biases. Runtime
placements use deterministic IDs in the low 63-bit editor domain and the same
coordinate conversion as `build_maptool_scene.py`.
