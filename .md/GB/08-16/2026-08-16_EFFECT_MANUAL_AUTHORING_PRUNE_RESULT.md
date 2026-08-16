# Effect manual-authoring cutover and Character Select cold-entry result

## Outcome

- Track A bulk materialization is retired as an active restoration workflow. Existing Imported/source evidence and runtime resources remain available for manual Effect Tool reconstruction.
- Removed exactly 2,004 authored Elements whose explicit JSON `visible` value was `false` from 109 documents. No retained root field or Element value changed.
- Current `Data/Effects/Authored` state is 258 documents, 8,047 Elements, zero `visible:false` Elements, and one intentionally empty editable document (`effect.artist.skill.31210.ba4.unified`).
- Republished the Product Effect catalog. The runtime set is 99 Effects: 98 direct authored documents plus one active Legacy assembly. The direct authored payload is 2,995 Elements with zero invisible Elements.
- Removed 95 unreferenced content-addressed sealed documents (195,693,608 bytes). The sealed directory now contains exactly the 98 files referenced by the runtime catalog.

## Character Select cold-entry fix

- Removed synchronous reconstructed Artist F preparation from `CLevel_CharacterSelect::Initialize`. Artist F remains lazily prepared by its existing acquisition path when the Effect Tool or its owning route actually requests it.
- Coalesced only adjacent raw `S2C_WORLD_SNAPSHOT` frames in the receive worker while preserving lifecycle and destruction frames as ordering barriers. This prevents a long cold activation from exhausting the bounded raw inbound queue before the main thread resumes network updates.
- The catalog still prewarms all active cues for the selected class. Invisible pruning reduces that work, but manual timing verification remains user-owned.

## Verification

- `prune_invisible_authored_elements.py --require-clean`: PASS, zero invisible rows.
- Retained-field comparison against `HEAD`: PASS for all 109 changed authored documents; exactly 2,004 rows removed.
- `Publish-Effects.ps1 -Mode Validate`: PASS, 99 runtime Effects.
- `Sync-EffectDataProject.ps1 -Check`: PASS, 1,596 files and 188 filters.
- Direct-authored runtime validator: 4/4 PASS.
- Client x64 Debug build: PASS, 0 errors.
- Server x64 Debug build: PASS, 0 errors.
- `git diff --check`: PASS (line-ending notices only).

## Deliberate boundaries

- Imported graphs, source receipts, DDS, WModel resources, skillbindings, animevents, Valtan pattern bindings, and AuthoredCorrections evidence were not deleted.
- Obsolete Track A materializer CLI modes and exact hidden-partition role tests must not be used to regenerate the removed rows. They remain historical tools/evidence and are not part of the default publisher path.
- No Client/UI or visual fidelity assertion was performed by the agent. The user owns the Character Select entry timing and manual Effect Tool visual review.
