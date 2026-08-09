# Warlord Effect Import Status

## Extraction evidence

- ParticleSystem graphs cataloged: 680 / 680
- Material references: 883
- Mesh runtime assets cooked: 143 / 143
- Texture runtime assets cooked: 618 / 618
- MaterialInstance parameter sets recovered: 32
- Unsupported UE3 DecalMaterial: 2
- Missing or ambiguous runtime candidates in the legacy unbound index: 0

`Warlord.unbound-effect-draft-index.json` remains the historical resource-level reference for
all 680 source systems. It is not the product skill ownership source.

## Current skill-bound evidence

Warlord is no longer in the earlier unbound-only state. The repository now contains:

- `Data/Animation/Authored/Warlord/Warlord.skillbindings.json`
- `Data/Animation/Authored/Warlord/Warlord.animevents`
- Warlord rows in `Data/Balance/PlayerSkills.json`
- `Warlord.action-particle-resource-catalog.json`

The action-bound catalog contains 29 source packages, 398 action-bound ParticleSystems,
1,528 resolved assets, 113 Mesh assets, 518 Texture assets, and zero missing requested source
systems. Representative product rollout begins with `LMB / 17000`, whose binding has three
combo stages.

The exact `LMB / 17000` diagnostic source chain is now materialized under this folder without
creating a product Authored Effect or cue. Its source receipt resolves all six particle notify
occurrences to three source systems, selects nine action-owned material bindings from the audited
catalog (eight exact source packages plus the explicit engine default-particle fallback), and
resolves all 17 required runtime Mesh/Texture assets from `Effect/Warlord`. The external module
closure remains 157 / 157 with zero unresolved requests. The Imported conversion emits 31
diagnostic elements from 21 emitter partitions with zero missing-resource partitions; four
partitions still require an executable source-material profile, and no source-exact/reconstructed
player anchor contract is available yet.

Filename heuristics are still forbidden. A package/object is assigned to a skill only through
the current action/animation binding evidence. A source catalog or receipt is not a finished
product Effect Document; the output must be materialized into `Data/Effects/Authored`, published
through the existing Effect pipeline, and referenced by the actual animation cue.

The first product pass keeps standalone Mesh, Cascade Mesh Particle, standalone Sprite, and
Cascade Sprite Particle as distinct source types. It restores evidence-backed standalone Mesh
and Sprite carriers, excludes Mesh Particle output, and stores Sprite rotation correction per
Authored element.
