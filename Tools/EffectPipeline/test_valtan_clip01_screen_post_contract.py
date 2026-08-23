#!/usr/bin/env python3
"""Focused contract for Valtan clip-01 typed ScreenPost occurrences.

The source cue identity, notify order, timing, material identity, and decoded
curves are source evidence.  The runtime ScreenPost equations are deliberately
classified separately: ZoomBlur is a bounded reconstruction, while the visible
FilmNoise scalar is project-tuned and must never be reported as source-exact.
"""

from __future__ import annotations

import copy
import json
import math
import struct
import unittest
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
DOCUMENT_PATH = (
    ROOT
    / "Data/Effects/Authored/"
    "effect.valtan.carrier-v1.attack.four-slash.active.clip-01.effect.json"
)
INVENTORY_PATH = (
    ROOT
    / "Data/Effects/Imported/Valtan/"
    "Valtan.source-occurrence-inventory.v1.json"
)
PRESENTATION_SOURCE_PATH = ROOT / "Client/Private/Effect_PresentationService.cpp"
PRESENTATION_HEADER_PATH = ROOT / "Client/Public/Effect_PresentationService.h"

SOURCE_TIME_SECONDS = 3.144474983215332
TRIPLE_ACTIVE_END_SECONDS = 3.5
CLIP01_ID = "valtan.attack.four-slash.active.clip.01"
CLIP02_OCCURRENCE_ID = "occurrence.0b1f192b7838db7bbb5ca80a"
CLIP02_FULL_KEY = (
    "occurrence-key.0b1f192b7838db7bbb5ca80a"
    "12913fd8a36f4475074c8bc6cf66172e1fb3333d"
)

ALPHA_CURVE = [
    0.0,
    1.0,
    0.0,
    0.25,
    0.5,
    0.75,
    1.0,
    1.0,
    1.0,
    1.0,
    1.0,
    1.0,
    1.0,
    1.0,
    1.0,
    1.0,
    1.0,
    0.8333333134651184,
    0.6666666269302368,
    0.49999988079071045,
    0.33333319425582886,
    0.1666666865348816,
    0.0,
]
ZOOM_STRENGTH_CURVE = [
    0.0,
    0.5,
    0.0,
    0.125,
    0.25,
    0.375,
    0.5,
    0.46875,
    0.4375,
    0.40625,
    0.375,
    0.34375,
    0.3125,
    0.28125,
    0.2499999701976776,
    0.2187499701976776,
    0.1875,
    0.15625,
    0.125,
    0.09374997019767761,
    0.06249997019767761,
    0.03125,
    0.0,
]

TARGETS = (
    {
        "occurrenceId": "occurrence.c627ba06ba0a8d5d48086907",
        "fullKey": (
            "occurrence-key.c627ba06ba0a8d5d48086907"
            "319ea4861dfd4521c7f532dff8eeb8befdc17957"
        ),
        "displayName": "FilmNoise [PROJECT_TUNED_APPROX]",
        "fidelity": "PROJECT_TUNED_APPROX",
        "systemId": "fx_post.fx_par.par_c_filmnoise_01",
        "sourceObjectPath": (
            "FX_POST.fx_par.par_c_filmnoise_01.particlespriteemitter_0"
        ),
        "notifyOrdinal": 15,
        "notifyId": "action-420609/stage-008/notify-025",
        "materialPath": "fx_mi.fx_c_pa_filmnoise_01_tr",
        "profileId": "screen.film-noise.reconstructed.v1",
        "moduleClasses": (
            "particlemodulerequired",
            "particlemodulelifetime",
            "particlemodulecoloroverlife",
            "particlemodulespawn",
        ),
    },
    {
        "occurrenceId": "occurrence.14794cdb89c73ee33f1dead3",
        "fullKey": (
            "occurrence-key.14794cdb89c73ee33f1dead3"
            "a920092369cde4326f3d0b29a41c484ff99e6cb6"
        ),
        "displayName": "ZoomBlur [BOUNDED_RECONSTRUCTED]",
        "fidelity": "BOUNDED_RECONSTRUCTED",
        "systemId": "fx_post.fx_par.par_c_zoomblur_03",
        "sourceObjectPath": (
            "FX_POST.fx_par.par_c_zoomblur_03.particlespriteemitter_0"
        ),
        "notifyOrdinal": 16,
        "notifyId": "action-420609/stage-008/notify-026",
        "materialPath": "fx_mi.fx_c_pa_zoomblur_01_tr",
        "profileId": "screen.zoom-blur.reconstructed.v1",
        "moduleClasses": (
            "particlemodulerequired",
            "particlemodulelifetime",
            "particlemodulecoloroverlife",
            "particlemoduleparameterdynamic",
            "particlemodulespawn",
        ),
    },
)


class ContractError(RuntimeError):
    """Raised when the focused admission predicate is not exact."""


def _object_no_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ContractError(f"duplicate JSON property: {key}")
        result[key] = value
    return result


def _read_json(path: Path) -> dict[str, Any]:
    value = json.loads(
        path.read_text(encoding="utf-8"),
        object_pairs_hook=_object_no_duplicates,
    )
    if not isinstance(value, dict):
        raise ContractError(f"JSON root is not an object: {path}")
    return value


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise ContractError(message)


def _f32_bits(value: Any) -> bytes | None:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        return None
    try:
        runtime = struct.unpack("<f", struct.pack("<f", float(value)))[0]
    except (OverflowError, struct.error, ValueError):
        return None
    if not math.isfinite(runtime):
        return None
    return struct.pack("<f", runtime)


def _same_f32(actual: Any, expected: Any) -> bool:
    expected_bits = _f32_bits(expected)
    return expected_bits is not None and _f32_bits(actual) == expected_bits


def _same_f32_sequence(actual: Any, expected: list[float]) -> bool:
    return (
        isinstance(actual, list)
        and len(actual) == len(expected)
        and all(
            _same_f32(actual_value, expected_value)
            for actual_value, expected_value in zip(
                actual, expected, strict=True
            )
        )
    )


def _codec_float(value: float) -> float:
    runtime = struct.unpack("<f", struct.pack("<f", value))[0]
    return float(format(runtime, ".9g"))


def _simulate_cpp_codec_numbers(value: Any) -> Any:
    if isinstance(value, float):
        return _codec_float(value)
    if isinstance(value, list):
        return [_simulate_cpp_codec_numbers(row) for row in value]
    if isinstance(value, dict):
        return {
            key: _simulate_cpp_codec_numbers(row)
            for key, row in value.items()
        }
    return value


def _next_float32(value: float) -> float:
    bits = struct.unpack("<I", struct.pack("<f", value))[0]
    return struct.unpack("<f", struct.pack("<I", bits + 1))[0]


def _distribution(
    element: dict[str, Any], module_class: str, property_path: str
) -> dict[str, Any]:
    matches = [
        distribution
        for module in element["sourceRecipe"]["modules"]
        if module.get("className") == module_class
        for distribution in module.get("distributions", [])
        if distribution.get("propertyPath") == property_path
    ]
    _require(
        len(matches) == 1,
        f"{element.get('id')} distribution is missing or duplicated: "
        f"{module_class}/{property_path}",
    )
    return matches[0]


def _source_parameter(element: dict[str, Any], name: str) -> dict[str, Any]:
    matches = [
        row
        for row in element["sourcePresentation"]["parameters"]
        if row.get("name") == name
    ]
    _require(
        len(matches) == 1,
        f"{element.get('id')} source parameter is missing or duplicated: {name}",
    )
    return matches[0]


def admit_clip01_screen_posts(
    document: dict[str, Any], inventory: dict[str, Any]
) -> tuple[dict[str, Any], dict[str, Any]]:
    """Fail closed unless the two exact stable occurrences form the typed pair."""

    _require(
        document.get("schema") == "lostark.effect-authoring"
        and document.get("version") == 13
        and document.get("effectAssetId")
        == "effect.valtan.carrier-v1.attack.four-slash.active.clip-01",
        "authored document identity mismatch",
    )
    elements = document.get("elements")
    _require(isinstance(elements, list), "authored elements are invalid")
    target_ids = tuple(target["occurrenceId"] for target in TARGETS)
    screen_posts = [
        element
        for element in elements
        if isinstance(element, dict) and element.get("id") in target_ids
    ]
    _require(
        tuple(element.get("id") for element in screen_posts) == target_ids,
        "clip-01 ScreenPost identity or Film-to-Zoom order mismatch",
    )

    serialized = json.dumps(elements, sort_keys=True)
    _require(
        CLIP02_OCCURRENCE_ID not in serialized
        and CLIP02_FULL_KEY not in serialized,
        "clip-02 ScreenPost occurrence contaminated clip-01",
    )

    occurrences = inventory.get("occurrences")
    _require(isinstance(occurrences, list), "source occurrence inventory invalid")
    inventory_by_id = {
        row.get("occurrenceId"): row
        for row in occurrences
        if isinstance(row, dict)
    }

    for element, target in zip(screen_posts, TARGETS, strict=True):
        occurrence_id = target["occurrenceId"]
        source = inventory_by_id.get(occurrence_id)
        _require(isinstance(source, dict), f"source occurrence missing: {occurrence_id}")
        _require(source.get("fullKey") == target["fullKey"], "source fullKey mismatch")
        _require(source.get("clipOccurrenceId") == CLIP01_ID, "source clip identity mismatch")
        _require(source.get("sourceStageIndex") == 8, "source stage identity mismatch")
        _require(source.get("sourceClip") == "Att_Battle_10_01", "source clip mismatch")
        _require(source.get("notifyOrdinal") == target["notifyOrdinal"], "source notify order mismatch")
        _require(source.get("notifyId") == target["notifyId"], "source notify identity mismatch")
        _require(
            _same_f32(source.get("sourceTimeSeconds"), SOURCE_TIME_SECONDS),
            "source time mismatch",
        )
        _require(source.get("sourceSystemId") == target["systemId"], "source system mismatch")
        _require(source.get("reachabilityDisposition") == "REACHABLE_REVIEWED", "source occurrence is not reviewed")

        _require(element.get("id") == occurrence_id, "element occurrence identity mismatch")
        _require(element.get("displayName") == target["displayName"], "fidelity class is not explicit")
        _require(target["fidelity"] in element["displayName"], "fidelity class was laundered")
        _require(element.get("groupId") == target["systemId"], "element system identity mismatch")
        _require(element.get("resources") == [], "ScreenPost must not bind a DDS quad")
        _require(element.get("visible") is True, "typed ScreenPost is not visible")
        _require(element["material"].get("sourceMaterialPath") == target["materialPath"], "source material mismatch")

        timing = element["detail"]["timing"]
        _require(
            _same_f32(
                timing.get("startDelaySeconds"), SOURCE_TIME_SECONDS
            ),
            "typed start time mismatch",
        )
        _require(
            _same_f32(timing.get("lifeTimeSeconds"), 0.35),
            "stage-bounded screen-post lifetime mismatch",
        )
        _require(
            timing["startDelaySeconds"] + timing["lifeTimeSeconds"]
            <= TRIPLE_ACTIVE_END_SECONDS,
            "ScreenPost outlives the Server action edge",
        )
        post = element["detail"]["screenPost"]
        _require(post.get("enabled") is True, "typed ScreenPost is disabled")
        _require(post.get("profileId") == target["profileId"], "typed ScreenPost profile mismatch")
        _require(post.get("status") == "reconstructed_profile", "typed runtime status mismatch")
        _require(type(post.get("randomSeed")) is int and post["randomSeed"] > 0, "invalid ScreenPost seed")

        recipe = element.get("sourceRecipe")
        _require(isinstance(recipe, dict) and recipe.get("enabled") is True, "source recipe disabled")
        _require(recipe.get("rendererShape") == "screenPost", "source renderer shape mismatch")
        bursts = recipe.get("bursts")
        _require(
            isinstance(bursts, list)
            and len(bursts) == 1
            and isinstance(bursts[0], dict)
            and _same_f32(bursts[0].get("timeSeconds"), 0.0)
            and bursts[0].get("countMinimum") == 1
            and bursts[0].get("countMaximum") == 1,
            "source one-shot burst mismatch",
        )
        _require(
            tuple(module.get("className") for module in recipe.get("modules", []))
            == target["moduleClasses"],
            "source module projection mismatch",
        )
        lifetime = _distribution(element, "particlemodulelifetime", "lifetime")
        _require(
            _same_f32_sequence(
                lifetime.get("defaultMinimum"), [1.0, 0.0, 0.0, 0.0]
            ),
            "source lifetime minimum mismatch",
        )
        _require(
            _same_f32_sequence(
                lifetime.get("defaultMaximum"), [1.0, 0.0, 0.0, 0.0]
            ),
            "source lifetime maximum mismatch",
        )
        alpha = _distribution(element, "particlemodulecoloroverlife", "alphaoverlife")
        _require(
            _same_f32(alpha.get("lookupTableTimeScale"), 20.0),
            "source alpha time scale mismatch",
        )
        _require(
            _same_f32_sequence(alpha.get("lookupTable"), ALPHA_CURVE),
            "source alpha curve mismatch",
        )

        presentation = element.get("sourcePresentation")
        _require(isinstance(presentation, dict) and presentation.get("enabled") is True, "source presentation disabled")
        _require(presentation.get("profileId") == target["profileId"], "source/runtime profile join mismatch")
        _require(presentation.get("status") == "reconstructed", "source presentation fidelity mismatch")
        _require(presentation.get("sourceObjectPath") == target["sourceObjectPath"], "source emitter identity mismatch")
        _require(presentation.get("sourceActionCueId") == target["notifyId"], "source action cue mismatch")
        _require(presentation.get("sourceEventId") == occurrence_id, "sourceEventId mismatch")
        _require(
            _same_f32(
                presentation.get("sourceTimeSeconds"), SOURCE_TIME_SECONDS
            ),
            "source presentation time mismatch",
        )
        _require(_source_parameter(element, "sourceSystemId").get("stringValue") == target["systemId"], "sourceSystemId parameter mismatch")
        _require(_source_parameter(element, "sourceOccurrenceId").get("stringValue") == occurrence_id, "sourceOccurrenceId parameter mismatch")
        _require(_source_parameter(element, "sourceOccurrenceFullKey").get("stringValue") == target["fullKey"], "sourceOccurrenceFullKey parameter mismatch")
        _require(_source_parameter(element, "sourceClipOccurrenceId").get("stringValue") == CLIP01_ID, "sourceClipOccurrenceId parameter mismatch")
        _require(
            _same_f32(
                _source_parameter(element, "sourceNotifyOrdinal").get(
                    "numberValue"
                ),
                target["notifyOrdinal"],
            ),
            "sourceNotifyOrdinal parameter mismatch",
        )

    film, zoom = screen_posts
    film_post = film["detail"]["screenPost"]
    _require(
        _same_f32(film_post.get("intensity"), 0.08),
        "FilmNoise tuned intensity changed",
    )
    _require(
        _same_f32(film_post.get("secondaryIntensity"), 0.02),
        "FilmNoise tuned scanline changed",
    )
    _require(
        _same_f32(film_post.get("frequency"), 1.0),
        "FilmNoise tuned frequency changed",
    )
    _require(
        not any(
            module.get("className") == "particlemoduleparameterdynamic"
            for module in film["sourceRecipe"]["modules"]
        ),
        "FilmNoise falsely acquired a source gain parameter",
    )

    zoom_post = zoom["detail"]["screenPost"]
    _require(
        _same_f32(zoom_post.get("intensity"), 0.0),
        "ZoomBlur fallback scalar must stay zero",
    )
    zoom_strength = _distribution(
        zoom,
        "particlemoduleparameterdynamic",
        "dynamicparams[0].paramvalue",
    )
    _require(
        _same_f32(zoom_strength.get("lookupTableTimeScale"), 20.0),
        "ZoomBlur source time scale mismatch",
    )
    _require(
        _same_f32_sequence(
            zoom_strength.get("lookupTable"), ZOOM_STRENGTH_CURVE
        ),
        "ZoomBlur source strength curve mismatch",
    )
    return film, zoom


class ValtanClip01ScreenPostContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.document = _read_json(DOCUMENT_PATH)
        cls.inventory = _read_json(INVENTORY_PATH)

    def test_checked_in_pair_is_admitted(self) -> None:
        film, zoom = admit_clip01_screen_posts(self.document, self.inventory)
        self.assertEqual(film["resources"], [])
        self.assertEqual(zoom["resources"], [])

    def test_cpp_codec_normalization_preserves_runtime_float_bits(self) -> None:
        normalized = _simulate_cpp_codec_numbers(copy.deepcopy(self.document))
        posts = [
            row
            for row in normalized["elements"]
            if row.get("id") in {
                target["occurrenceId"] for target in TARGETS
            }
        ]
        for post in posts:
            # Disabled presentation defaults are intentionally omitted by the
            # C++ writer and are outside the typed ScreenPost contract.
            post["detail"]["light"] = {"enabled": False}
        self.assertEqual(
            posts[0]["detail"]["timing"]["startDelaySeconds"],
            3.14447498,
        )
        self.assertEqual(
            posts[0]["detail"]["timing"]["lifeTimeSeconds"],
            0.349999994,
        )
        self.assertEqual(
            posts[0]["detail"]["screenPost"]["intensity"],
            0.0799999982,
        )
        film, zoom = admit_clip01_screen_posts(normalized, self.inventory)
        self.assertEqual(film["id"], TARGETS[0]["occurrenceId"])
        self.assertEqual(zoom["id"], TARGETS[1]["occurrenceId"])

    def test_one_runtime_float_bit_mutation_fails_closed(self) -> None:
        mutated = _simulate_cpp_codec_numbers(copy.deepcopy(self.document))
        film = next(
            row
            for row in mutated["elements"]
            if row.get("id") == TARGETS[0]["occurrenceId"]
        )
        film["detail"]["screenPost"]["intensity"] = _next_float32(0.08)
        with self.assertRaisesRegex(
            ContractError, "FilmNoise tuned intensity changed"
        ):
            admit_clip01_screen_posts(mutated, self.inventory)

    def test_duplicate_protected_identity_fails_closed(self) -> None:
        mutated = copy.deepcopy(self.document)
        film = next(
            row
            for row in mutated["elements"]
            if row.get("id") == TARGETS[0]["occurrenceId"]
        )
        mutated["elements"].append(copy.deepcopy(film))
        with self.assertRaisesRegex(ContractError, "identity or Film-to-Zoom"):
            admit_clip01_screen_posts(mutated, self.inventory)

    def test_clip02_occurrence_is_not_materialized(self) -> None:
        serialized = json.dumps(self.document, sort_keys=True)
        self.assertNotIn(CLIP02_OCCURRENCE_ID, serialized)
        self.assertNotIn(CLIP02_FULL_KEY, serialized)

    def test_identity_mutation_fails_closed(self) -> None:
        mutated = copy.deepcopy(self.document)
        posts = [row for row in mutated["elements"] if row.get("kind") == "screenPost"]
        posts[0]["sourcePresentation"]["sourceEventId"] = "occurrence.invalid"
        with self.assertRaisesRegex(ContractError, "sourceEventId mismatch"):
            admit_clip01_screen_posts(mutated, self.inventory)

    def test_clip02_identity_injection_fails_closed(self) -> None:
        mutated = copy.deepcopy(self.document)
        posts = [row for row in mutated["elements"] if row.get("kind") == "screenPost"]
        posts[1]["sourcePresentation"]["sourceEventId"] = CLIP02_OCCURRENCE_ID
        with self.assertRaisesRegex(ContractError, "clip-02 ScreenPost occurrence"):
            admit_clip01_screen_posts(mutated, self.inventory)

    def test_profile_mutation_fails_closed_without_fallback(self) -> None:
        mutated = copy.deepcopy(self.document)
        posts = [row for row in mutated["elements"] if row.get("kind") == "screenPost"]
        posts[0]["detail"]["screenPost"]["profileId"] = (
            "screen.zoom-blur.reconstructed.v1"
        )
        with self.assertRaisesRegex(ContractError, "profile mismatch"):
            admit_clip01_screen_posts(mutated, self.inventory)

    def test_unknown_profile_fails_closed(self) -> None:
        mutated = copy.deepcopy(self.document)
        posts = [row for row in mutated["elements"] if row.get("kind") == "screenPost"]
        posts[1]["detail"]["screenPost"]["profileId"] = "screen.unknown.v1"
        with self.assertRaisesRegex(ContractError, "profile mismatch"):
            admit_clip01_screen_posts(mutated, self.inventory)

    def test_film_to_zoom_order_is_not_silently_reordered(self) -> None:
        mutated = copy.deepcopy(self.document)
        positions = [
            index
            for index, row in enumerate(mutated["elements"])
            if row.get("kind") == "screenPost"
        ]
        mutated["elements"][positions[0]], mutated["elements"][positions[1]] = (
            mutated["elements"][positions[1]],
            mutated["elements"][positions[0]],
        )
        with self.assertRaisesRegex(ContractError, "Film-to-Zoom order"):
            admit_clip01_screen_posts(mutated, self.inventory)

    def test_dds_quad_injection_fails_closed(self) -> None:
        mutated = copy.deepcopy(self.document)
        posts = [row for row in mutated["elements"] if row.get("kind") == "screenPost"]
        posts[0]["resources"] = [
            {"slotId": "base", "assetId": "Effect/Valtan/fake.dds"}
        ]
        with self.assertRaisesRegex(ContractError, "must not bind a DDS quad"):
            admit_clip01_screen_posts(mutated, self.inventory)

    def test_unrelated_manual_rows_do_not_lock_the_protected_projection(self) -> None:
        mutated = copy.deepcopy(self.document)
        mutated["elements"].insert(
            0,
            {
                "id": "valtan.clip01.weapon-slash.manual-test",
                "kind": "particle",
            },
        )
        mutated["elements"].append(
            {
                "id": "screen.manual.unrelated-test",
                "kind": "screenPost",
            }
        )
        admitted = admit_clip01_screen_posts(mutated, self.inventory)
        self.assertEqual(
            tuple(row["id"] for row in admitted),
            tuple(row["occurrenceId"] for row in TARGETS),
        )

    def test_boss_action_stop_boundaries_are_fail_closed(self) -> None:
        source = PRESENTATION_SOURCE_PATH.read_text(encoding="utf-8-sig")
        header = PRESENTATION_HEADER_PATH.read_text(encoding="utf-8-sig")

        predicate_begin = source.index(
            "constexpr bool Should_StopBossActionActiveEffect("
        )
        predicate_end = source.index(
            "struct RECONSTRUCTED_SOURCE_RUNTIME_CACHE_VIEW", predicate_begin
        )
        predicate_source = source[predicate_begin:predicate_end]

        action_begin = source.index(
            "Client::CEffectPresentationService::Stop_BossAction("
        )
        owner_begin = source.index(
            "void Client::CEffectPresentationService::Stop_BossOwner(",
            action_begin,
        )
        clear_level_begin = source.index(
            "void Client::CEffectPresentationService::Clear_Level(", owner_begin
        )
        action_source = source[action_begin:owner_begin]
        owner_source = source[owner_begin:clear_level_begin]

        pending_end = action_source.index("for (size_t iEffect")
        pending_source = action_source[:pending_end]
        active_source = action_source[pending_end:]

        # Every queued spawn owned by this non-world action is cancelled at the
        # edge; pending cancellation is deliberately independent of stopPolicy.
        self.assertIn(
            "0u == Pending.Desc.iWorldRootHandle &&\n"
            "\t\t\t\tPending.Desc.pBossOwner.lock() == pOwner &&\n"
            "\t\t\t\tPending.Desc.iActionStartTick == iActionStartTick;",
            pending_source,
        )
        self.assertNotIn("eStopPolicy", pending_source)
        self.assertIn(
            "iPendingBefore - g_PendingEffectSpawns.size()", pending_source
        )

        # World-root work and work owned by another boss/action tick must not be
        # consumed by this action edge.
        ownership_guard = (
            "if (0u != Effect.iWorldRootHandle ||\n"
            "\t\t\tEffect.pBossOwner.lock() != pOwner ||\n"
            "\t\t\tEffect.iActionStartTick != iActionStartTick)\n"
            "\t\t{\n"
            "\t\t\tcontinue;\n"
            "\t\t}"
        )
        self.assertIn(ownership_guard, active_source)
        self.assertLess(
            active_source.index(ownership_guard),
            active_source.index("Should_StopBossActionActiveEffect"),
        )

        # The exact inequality is intentional: NATURAL is the sole retained
        # value, so CUE_END and END/any invalid enum value fail closed to stop.
        self.assertIn(
            "return Client::EFFECT_STOP_POLICY::NATURAL != eStopPolicy;",
            predicate_source,
        )
        self.assertIn(
            "static_assert(!Should_StopBossActionActiveEffect(\n"
            "\t\tClient::EFFECT_STOP_POLICY::NATURAL));",
            predicate_source,
        )
        self.assertIn(
            "static_assert(Should_StopBossActionActiveEffect(\n"
            "\t\tClient::EFFECT_STOP_POLICY::CUE_END));",
            predicate_source,
        )
        self.assertIn(
            "if (!Should_StopBossActionActiveEffect(Effect.eStopPolicy))\n"
            "\t\t{\n"
            "\t\t\t++Result.iActiveRetainedNatural;\n"
            "\t\t\tcontinue;\n"
            "\t\t}\n"
            "\t\tRemove_At(iEffect);\n"
            "\t\t++Result.iActiveStopped;",
            active_source,
        )

        # Owner teardown is unconditional across action tick, world-root handle,
        # and stop policy for both pending and active boss-owned effects.
        self.assertIn(
            "return Pending.Desc.pBossOwner.lock() == pOwner;", owner_source
        )
        self.assertIn(
            "if (g_ActiveEffects[iEffect].pBossOwner.lock() == pOwner)\n"
            "\t\t\tRemove_At(iEffect);",
            owner_source,
        )
        self.assertNotIn("iActionStartTick", owner_source)
        self.assertNotIn("iWorldRootHandle", owner_source)
        self.assertNotIn("eStopPolicy", owner_source)

        self.assertIn("uint64_t iPendingStopped = 0u;", header)
        self.assertIn("uint64_t iActiveStopped = 0u;", header)
        self.assertIn("uint64_t iActiveRetainedNatural = 0u;", header)


if __name__ == "__main__":
    unittest.main()
