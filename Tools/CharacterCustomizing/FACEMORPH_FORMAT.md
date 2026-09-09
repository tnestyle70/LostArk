# `.facemorphs` container format

`Client/Bin/Resources/Character/<Class>/FaceMorphs/<Class>.facemorphs` holds every face
MorphTarget of one playable class' face mesh. It is produced by
`Tools/CharacterCustomizing/build_face_morphs.py` and consumed at runtime as a Resources-relative
asset id (`Character/<Class>/FaceMorphs/<Class>.facemorphs`).

Everything is **little-endian**. Every field is 4-byte aligned, so a loader may map the file and
read the vertex blocks in place.

## Header (28 bytes)

| offset | type       | field              | meaning |
|--------|------------|--------------------|---------|
| 0      | `char[8]`  | `magic`            | `LAFMORPH`, no NUL terminator |
| 8      | `uint32`   | `version`          | `1` |
| 12     | `uint32`   | `baseVertexCount`  | the source `MorphTargetLODModel.NumBaseMeshVerts`, verbatim |
| 16     | `uint32`   | `vertexIndexBound` | derived: `1 + max(vertexIndex)` over every morph in this file |
| 20     | `uint32`   | `morphCount`       | number of morph records that follow |
| 24     | `uint32`   | `totalVertexCount` | sum of every record's `vertexCount`; lets a loader size one arena |

`baseVertexCount` is source data and is **not** guaranteed to be `>= vertexIndexBound` — see
*Known source anomalies*. Allocate and range-check against `vertexIndexBound`.

## Morph records

`morphCount` records follow the header back to back, in the order the source `MorphTargetSet`
lists them.

| type              | field         | meaning |
|-------------------|---------------|---------|
| `uint32`          | `nameLength`  | length of `name` in bytes, without a NUL terminator |
| `char[nameLength]`| `name`        | ASCII morph name |
| `uint8[]`         | padding       | zero bytes, `(-nameLength) % 4` of them, so the next field is 4-byte aligned |
| `uint32`          | `vertexCount` | number of moved vertices; may be `0` |
| `vertex[vertexCount]` | `vertices` | 28 bytes each, see below |

### Vertex (28 bytes)

| offset | type        | field            | meaning |
|--------|-------------|------------------|---------|
| 0      | `uint32`    | `vertexIndex`    | index into the base face mesh's LOD0 vertex buffer |
| 4      | `float32[3]`| `positionDelta`  | `x, y, z` offset added at weight `1.0`, in source units (cm) |
| 16     | `float32[3]`| `normalDelta`    | `x, y, z` tangent-Z (normal) offset at weight `1.0` |

`vertexIndex` is strictly ascending inside one record, so a record is a sorted sparse list and a
loader may binary-search or merge-walk it. The cooker asserts this.

A morph is applied as `position += weight * positionDelta` (and the same for the normal), with
`weight` taken from `Data/UI/Customizing/CustomizingFacePresets.json`.

## Names

The sidecar carries the morph name exactly as the retail package's name table spells it, which is
all lowercase (`mm_headbase_meshtype_07_ui`). The preset document spells the same morph in its
authoring case (`MM_HeadBase_MeshType_07_UI`), and that is what
`CustomizingFacePresets.json` carries. **Match the two case-insensitively (ASCII).** Neither
spelling was invented; they are the two source spellings.

## Provenance

Each class' face mesh, its single `MorphTargetSet` and every `MorphTarget` live in one retail
package:

| class asset id    | race tag | retail package                   | face mesh            | morphs |
|-------------------|----------|----------------------------------|----------------------|--------|
| `LanceMaster`     | `FT`     | `VC2NXN2N002NXYCQF8HEH00E.upk`   | `pc_ft_00_face_sk`   | 45 |
| `Warlord`         | `WR`     | `VC2N892N002NXYCQF8REH00E.upk`   | `pc_wr_00_face_sk`   | 52 |
| `Artist`          | `SP`     | `VC2NGV2N002NXYCQF81EH00E.upk`   | `pc_sp_00_face_sk`   | 40 |
| `DimensionMaster` | `SP_M`   | `WD3OHW3OB3I113IYZDRG1MP6M.upk`  | `pc_sp_m_00_face_sk` | 36 |

The retail `MorphTarget` export is serialised as
`[4B export prefix][8B empty property list ("None")][uint32 LOD model count = 1][uint32 vertex
count][20B per vertex][uint32 NumBaseMeshVerts]`. The 20-byte source vertex is
`FVector PositionDelta` (12B) + `FPackedNormal TangentZDelta` (4B) + `SourceIdx` (4B, a WORD with
two zero pad bytes). The cooker decodes the packed normal with the UE3 convention
`component = byte / 127.5 - 1.0` and stores it as three floats; the original bytes are recoverable
as `round((component + 1.0) * 127.5)`.

## Scale

Positions are in the source mesh's units. The face `SkeletalMesh` bounds read from the same
packages give a head box extent of roughly `(12.3, 10.6, 11.1)` for every class, i.e. a ~22 cm
head, so the units are centimetres and the deltas below are centimetres.

| class             | `|delta|` min | max     | mean    |
|-------------------|---------------|---------|---------|
| `LanceMaster`     | 0.008009      | 3.053639| 0.149295|
| `Warlord`         | 0.008002      | 0.920167| 0.133847|
| `Artist`          | 0.008004      | 1.789853| 0.163419|
| `DimensionMaster` | 0.008011      | 0.706553| 0.089015|

## Known source anomalies

* `Warlord` declares `NumBaseMeshVerts = 1317` in every one of its 52 morphs but its morphs
  reference vertex indices up to `1361`. The declared value is stale retail data; the cooker keeps
  it verbatim in `baseVertexCount` and publishes the observed bound in `vertexIndexBound` (`1362`).
  The other three classes agree exactly (`bound == baseVertexCount`).
* `vertexIndex` addresses the retail LOD0 vertex buffer, i.e. the source PSK's *wedge* index. It is
  **not** an index into the converted `.wmodel` vertex buffer — `LanceMaster_Face.wmodel` for
  instance has 2089 render vertices against the retail mesh's 1884, because the two conversions
  split seam vertices differently. See `.facemorphmap` below for the correspondence.

## `.facemorphmap` — source wedge to runtime vertex

`Client/Bin/Resources/Character/<Class>/FaceMorphs/<Class>.facemorphmap` maps every `vertexIndex`
above (a retail PSK wedge index) to the one or more runtime `.wmodel` vertex indices it corresponds
to. It is produced by `Tools/CharacterCustomizing/build_face_morph_vertex_map.py` from the retail
PSK (**not** a UModel `.gltf` export — see *Why not the .gltf export* below) and the cooked
`.wmodel`.

Matching works because the retail PSK and the cooked `.wmodel` share the same units and the same
axes up to one sign (`x, y, -z`, scale 1:1, confirmed per class by brute-forcing every axis
permutation/sign/scale against a position sample: only that one combination has zero residual).
Every source wedge's transformed position is looked up in the runtime mesh by an *exact* match (a
hash-grid neighbour search, not nearest-point), and ties — several runtime vertices sitting at the
same position, which happens at multi-material submesh seams — are broken by UV (trying both `v`
and `1-v`, since the two conversions do not agree on which is "up"). A wedge can legitimately map to
more than one runtime vertex; apply the same delta to all of them.

### Header (20 + `meshCount` * 4 bytes)

| offset | type      | field                | meaning |
|--------|-----------|----------------------|---------|
| 0      | `char[8]` | `magic`              | `LAFMVMAP`, no NUL terminator |
| 8      | `uint32`  | `version`             | `2` |
| 12     | `uint32`  | `sourceWedgeCount`    | number of records that follow, one per PSK wedge |
| 16     | `uint32`  | `meshCount`           | number of submeshes in the target `.wmodel`'s WMSH |
| 20     | `uint32[meshCount]` | `meshVertexCount` | each submesh's own vertex count, for bounds validation |

### Records

`sourceWedgeCount` records follow, in PSK wedge order (the same order and count as the `vertexIndex`
space `.facemorphs` uses):

| type              | field           | meaning |
|-------------------|-----------------|---------|
| `uint32`          | `targetCount`   | number of runtime vertices this wedge maps to (`>= 1`) |
| `(uint32, uint32)[targetCount]` | `(meshIndex, vertexIndex)` | which submesh, and the index inside *that submesh's own* vertex buffer |

**A target is (mesh index, local vertex index), not one flat wmodel-wide index.** The `.wmodel`'s
WMSH holds one flat vertex blob, but `Engine/Private/BinaryAsset/Winters/WMeshReader.cpp` slices it
by `SUBMESH_DESC` into one `MODEL_MESH_DATA` per submesh, and `CModel::Ready_Meshes` builds one
`CMesh` per those, each with its own vertex buffer indexed from 0. `meshIndex` indexes
`CModel::m_Meshes` in that same order. (`SUBMESH_DESC.vertexOffset` is a *byte* offset in the file --
`WMeshReader.cpp` adds it to a `uint8_t*` -- so the cooker divides it by the vertex stride before
using it as an index; getting that wrong is what version 1 got wrong.)

The cooker asserts every wedge resolves to at least one runtime vertex before writing the file, and
that every runtime vertex is claimed when the target is a dedicated face mesh (for a combined body
`.wmodel` the body/clothing vertices are correctly left unclaimed).

### Why not the `.gltf` export

UModel's `.gltf` export re-orders/re-welds vertices, so its vertex `i` is **not** the retail wedge
`i` the morph target's `SourceIdx` addresses. Grouping `.gltf` vertices by exact position and
checking whether every group's members agreed on a morph's delta (they should — true seam
duplicates move together) found the opposite: 2376-3421 of ~2400-3400 touched groups disagreed per
class, by up to 0.84 (source units). Doing the identical check against the source **PSK**'s wedge
order instead — grouping wedges by their shared `PNTS0000` point index — gave zero disagreement,
0 of 2046 touched groups, confirming the PSK's wedge order is the correct `vertexIndex` space and
the `.gltf` export order is not.

### Coverage

Every map targets the `.wmodel` the character actually loads, which for all four classes is the
combined full-body model: the face is a material range inside it, so only the vertices that really
are the face get claimed and the rest are body/clothing, correctly untouched.

`LanceMaster` also ships a dedicated `LanceMaster_Face.wmodel`, and the first version of its map
targeted that. Nothing loads it — `CharacterCatalog.json` names `LanceMaster.wmodel` as the body
and never lists the face model — and, worse, every (meshIndex, localIndex) that file produced
happened to be in range on the body too, so `CFaceMorphApplier`'s bounds check passed and the
morphs silently deformed the upper body, lower body and arm. Rebuilt against the body it claims
the same 2089 vertices, all inside the body's face/eye/eyelash submeshes. **Build a map against
the model the runtime loads, not against whichever file has the matching name.**

| class             | source wedges | runtime verts | target mesh | claimed | 1:1 | maps to >1 |
|-------------------|---------------|----------------|-------------|---------|-----|------------|
| `LanceMaster`     | 1884          | 9120           | combined `LanceMaster.wmodel` | 2089 | 1612 | 272 |
| `Warlord`         | 1362          | 9842           | combined `Warlord.wmodel` | 1508 | 1151 | 211 |
| `Artist`          | 2899          | 14212          | combined `Artist.wmodel` | 2628 | 2804 | 95 |
| `DimensionMaster` | 4132          | 34612          | combined `DimensionMaster_Character.wmodel` | 4265 | 3955 | 177 |

Which submeshes the targets land in is itself a check that the match found the face and not
something near it -- they concentrate in a handful of submeshes per class, never spread across all
of them:

| class | targets per submesh |
|-------|---------------------|
| `LanceMaster` | 3: 1809, 4: 361, 5: 157 (of 7 -- face, eyelashes, eye) |
| `Warlord` | 4: 1443, 5: 185 (of 6) |
| `Artist` | 0: 2420, 1: 322, 2: 226, **5: 26** (of 8) |
| `DimensionMaster` | 6: 3584, 7: 242, 8: 512 (of 10) |

**`Artist`'s 26 targets in submesh 5 are a known imprecision**, not a separate face part: Artist is
the class where 928 of 2899 wedges missed a bit-exact position match and fell back to nearest-vertex
(see above), and a handful of those, at the boundary of the face region, resolved to a neighbouring
body vertex in another submesh instead. It is 26 of 2994 targets (0.9%), all at the face's edge where
the deltas are smallest, so the visible effect should be negligible -- but it is a real wrong-vertex
risk that a tighter match (or a per-submesh restriction on the search) would remove.

### `Warlord`, `Artist`, `DimensionMaster`: UModel could not export a PSK, so this reads the `.upk` directly

Neither `umodel_lostark_v7.exe -game=lostark -kr` nor `umodel_lostark_custom.exe -game=loakr` (nor
a third build, `umodel_lostark_rawdump.exe`) can export a PSK for these three classes' face
`skeletalmesh` objects — confirmed not to be a which-patch issue (it fails identically on
`LanceMaster`'s own package when tried fresh, the same package a working PSK had once come from by
some no-longer-reproducible method). All three throw inside `FStaticLODModel3`'s native array
serialization:

```
ArrayProperty: unknown USkeletalMesh3 clothingassets
BoolProperty: unknown FSkeletalMeshLODInfo bdisablecompressions
BoolProperty: unknown USkeletalMesh3 bhasbeensimplified
ByteProperty: unknown USkeletalMesh3 floorconformtype
WARNING: FMultisizeIndexContainer data size 0, assuming int32
*** ERROR: Serializing behind stopper (18C227+8080000 > 1B8958)
FUE3ArchiveReader::Serialize <- ... <- FSkelIndexBuffer3<< <- FStaticLODModel3<< <- ...
<- USkeletalMesh3::Serialize <- LoadObject: SkeletalMesh3'...'
```

The new tagged `UProperty` fields (`clothingassets`, `bdisablecompressions`, `bhasbeensimplified`,
`floorconformtype`) are each self-describing and skip cleanly; the archive position is already off
by the time it reaches `FSkelIndexBuffer3`.

Rather than fully reverse the native (non-tagged) part of `FStaticLODModel3` — which turned out to
branch in ways this project's UModel build's bundled parser does not handle, and which differ
slightly asset to asset (`FMeshBone` is 52 bytes per bone here, not any stock UE3 size; a
`FSkelMeshSection3` record sometimes carries extra trailing bytes UModel's source has no case for) —
`Tools/CharacterCustomizing/read_skeletalmesh_vertices.py` sidesteps understanding that struct at
all. The one number it actually needs, `NumVertices`, is already known independently (this file's
own `vertexIndexBound`, read by `build_face_morphs.py`'s already-working `MorphTarget` decoder
earlier in the very same export). So it searches the raw export for the one place a
`[stride:int32][count:int32]` pair is immediately followed by `count` consecutive `stride`-byte
records whose position field (at a fixed +16 byte offset, confirmed byte-for-byte against
`LanceMaster`'s real PSK, worst delta `0.0` over all 1884 vertices) looks like a real head-sized 3D
point. In practice this has always produced exactly one match. See that module's docstring for the
full reasoning.

`build_face_morph_vertex_map.py --upk --object-name --expect-verts` uses it as an alternative to
`--psk`, producing the same `(points, wedges)` shape the rest of the mapping pipeline already
consumed for `LanceMaster`.

One more difference from `LanceMaster` showed up matching against a *combined* body `.wmodel`: a
small fraction of vertices (0% for `Warlord`, 32% for `Artist`, 0% for `DimensionMaster`) do not
land bit-exact — the combined cook re-bakes the face at a very slightly different bind pose than
the standalone retail face mesh, typically well under a millimetre off (median 0.045, worst observed
0.35, source-unit cm) and never close enough to plausibly be a different, wrong vertex. `find_axis_transform`
accepts by *median* per-sample distance rather than the sum for exactly this reason (a strict sum
rejected an otherwise-correct, bit-exact-for-most-vertices transform outright), and `build_mapping`
falls back to a widening nearest-vertex search only for the individual wedges that miss the tight
exact-match epsilon, instead of failing the whole class.

## Runtime application

`Client/Public/FaceMorphApplier.h` (`CFaceMorphApplier`) reads both files, joins them (each morph's
sparse wedge list resolved through the map into runtime `(meshIndex, vertexIndex)` targets) and owns
the per-morph weights. `CCharacter` holds one (`m_FaceMorph`), loads it in `Load_FaceMorphs()` from
`Character/<Class>/FaceMorphs/<Class>.facemorph{s,map}`, and calls `Apply()` in `Late_Update()` next
to the existing `CFaceCustomizeApplier` (bone sliders) call. `Apply()` is a no-op unless a weight
actually changed.

On a change it recomputes each touched vertex as `base + sum(weight * positionDelta)` (and the same
for the normal) over every currently-nonzero morph -- including vertices that *were* touched but no
longer are, so lowering a weight reverts them -- and pushes the result through
`CModel::Update_Mesh_Vertices`.

The engine side is **pure addition** -- no existing `CMesh`/`CModel` function's signature or body was
changed, and nothing is opt-in at load time, so a model that never morphs pays nothing at all:

* `CMesh::Make_VertexBuffer_Unique()` does everything, lazily, on the first actual weight change:
  it copies the vertex buffer into a `D3D11_USAGE_STAGING` buffer, `Map()`s that to read the
  unmorphed position/normal back (the vertex buffer itself is `D3D11_USAGE_DEFAULT` with
  `CPUAccessFlags` 0, so it cannot be mapped directly), then `CopyResource`s into a fresh
  per-character buffer and swaps it in. That is the one stalling call here -- once per character
  that opens the face editor. Giving that character its own buffer is what stops one character's
  face edits leaking into every other clone of that class, since `CMesh::Clone()` copy-constructs
  and otherwise shares one `ComPtr<ID3D11Buffer>`.
* `CMesh::Update_Vertices()` writes one `UpdateSubresource` per vertex, each box covering only that
  vertex's 24-byte position+normal span (`vPosition` at offset 0, `vNormal` at 12, in both `VTXMESH`
  and `VTXANIMMESH`), so UV/tangent/binormal/blend data is never rewritten.
* `CMesh::Reset_Vertices()` puts every vertex it has touched back to that base snapshot.

**LanceMaster is not wired up.** Its face is a separate `LanceMaster_Face.wmodel` that nothing loads
yet -- unlike the other three, whose face is a material range inside the combined body model that
already loads for every character of that class. Its `.facemorphs`/`.facemorphmap` are cooked and
correct; what is missing is a load path that puts that mesh on the character at all.

## What this file does *not* contain

The character-creation base tab drives the face with two mechanisms. Only one of them is
MorphTargets:

* `MM_<group>_MeshType_<NN>_UI` — real MorphTargets. This file.
* `ADD_<slider>_UI` — additive three-frame face **AnimSequences** in the same package's
  `pc_<race>_00_face_ani` AnimSet (their `SequenceName` is the slider name without the `add_`
  prefix). Those are cooked separately by `build_face_sliders.py` into
  `Data/Customizing/FaceSliders/<race>.facesliders.json`.

Neither mechanism covers the face *textures* -- the eye tab's iris, and the eyemake / lip /
cheek / decal stamps. Those live in `Data/UI/Customizing/CustomizingFaceTextures.json` and are
read by `CCustomizingFaceTextureDocument`. Only `iris` is a whole texture slot (512x512, swapped
onto the eye material by `CCharacter::Set_FaceIrisTexture`); the others are stamps the retail
engine composites into the face texture, and this renderer has no compositing path for them, so
they are listed but not applied.
