# Warlord Effect Import Status

## Extraction status

- ParticleSystem graphs: 680 / 680 cataloged
- Material references: 883
- Mesh runtime assets: 143 / 143 cooked
- Texture runtime assets: 618 / 618 cooked
- Missing global Material map entries: 34 reviewed
  - MaterialInstance parameter sets recovered: 32
  - Unsupported UE3 DecalMaterial: 2
- `EngineMaterials.DefaultParticle`: UE3 engine fallback, not a missing game asset
- Runtime resource candidates referenced by the unbound draft index: 0 missing or ambiguous

`Warlord.unbound-effect-draft-index.json` is the Effect Tool extraction reference for all
680 source ParticleSystems. It preserves original source asset names and resolved runtime
Mesh/Texture candidates so resource-level review can start without re-running extraction.

## Skill binding status

- Warlord animation source found: 371 AnimSequences
- AnimSequence `Notifies` properties found: 0
- Normalized `Warlord.animnotify`: not available
- Authored `Warlord.skillbindings.json`: not available
- Skill-bound source systems: 0

`FX_PC_WGL_*` package names and ParticleSystem object names are search labels only. They
are not accepted as evidence that a graph belongs to a particular skill or Q/W/E slot.
This directory is an authoring staging root and does not declare Warlord as a playable
runtime class.

The final `Q | skill name` Effect Tool connection must use the same evidence chain as the
existing animation-owned classes:

1. approved Warlord skill rows with stable skill ID, input slot, and display name;
2. `Warlord.skillbindings.json` mapping each skill ID to real model clips;
3. `Warlord.animnotify` or approved `.animevents` mapping clip-local time to source effect;
4. normalized source receipts and authored Effect Documents generated from those inputs.

Until those inputs exist, the 680 graphs remain explicitly unbound and must not be assigned
to skills by filename heuristics.
