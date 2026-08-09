# Artist Effect Import Status

## Current evidence

- Normalized animation reference: available
- `PlayParticleEffect` notify occurrences: 540
- Unique referenced ParticleSystem asset paths: 247
- Resource source manifest: `Artist.resource-source-manifest.json`
- Skill source inventory: `Artist.skill-effect-source-inventory.json`
- Skill source receipts: 9, including representative combat skill `31000`
- Authored product rollout: representative `LMB / 31000` is the first gate

A source receipt proves extraction provenance; it does not by itself declare a finished
product Effect Document. Product output must be materialized into `Data/Effects/Authored`,
published through the existing Effect pipeline, and referenced by the actual animation cue.

The restoration classifier keeps standalone Mesh, Cascade Mesh Particle, standalone Sprite,
and Cascade Sprite Particle distinct. The first product pass admits evidence-backed standalone
Mesh and Sprite carriers, excludes Mesh Particle output, and stores the Sprite `-90` correction
per Authored element rather than as a runtime-wide hardcode.

This folder must not receive copies of DimensionMaster resources. Shared source assets are
resolved from exact package/object references and copied only when an Artist receipt proves
the dependency.
