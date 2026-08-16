"""Move the readable Cascade module values into Detail, then hand playback over.

An Element whose sourceRecipe.enabled is true has its particle axes driven by
the original module stack; every authored number under detail.particle is read
and then ignored (Effect_Playback.cpp gates each module read on bEnabled). So
tuning does nothing on those Elements, which is the whole reason this exists.

Flipping enabled to false alone would drop the Element to codec defaults and the
picture would disappear. This projects what the modules actually say onto the
authored fields first, and only then flips ownership.

Two rules keep that projection honest:

  compare first   an axis is written only where Detail still holds the codec
                  default. The Track A import already extracted lifetime,
                  startSize, spawnRate, maxParticles and the timing pair, and
                  re-deriving them here would overwrite good values with worse
                  ones.

  no guessing     a module class that is not in the table below, or a curve
                  with more than two control points, is left entirely alone.
                  sourceRecipe is never deleted, so it stays as the evidence a
                  later pass can read again.

Distribution evaluation mirrors CEffectDistribution::Evaluate exactly, including
the two leading range values in a cooked lookup table and the chunk layout, so
the numbers written here are the numbers playback was already computing.

Read Data/Effects/Authored, rewrite in place. Dry run unless --write.
"""

import argparse
import collections
import glob
import json
import math
import os
import sys
import time

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
AUTHORED = os.path.join(REPO, "Data", "Effects", "Authored")

# CEffectDistribution.cpp: a cooked table is prefixed by its two range values.
COOKED_LOOKUP_RANGE_VALUE_COUNT = 2
# Effect_Playback.cpp UE3_CentimetersToClient.
UE3_TO_CLIENT_METRES = 0.01

# Codec defaults (Effect_AuthoringDocument.h). An axis equal to these is empty.
DEFAULT_POSITION_BOX = ([0.0, 0.0, 0.0], [0.0, 0.0, 0.0])
DEFAULT_VELOCITY_BOXES = (
    # The header default, and the explicit all-zero box the Track A import wrote
    # for every Element that had no velocity to record.
    ([-0.5, 1.0, -0.5], [0.5, 2.0, 0.5]),
    ([0.0, 0.0, 0.0], [0.0, 0.0, 0.0]),
)
DEFAULT_COLOR_MULTIPLY = [1.0, 1.0, 1.0, 1.0]

LOCATION_PRIMITIVES = (
    "particlemodulelocationprimitivesphere",
    "particlemodulelocationprimitivecylinder",
    "particlemodulelocationprimitivecylinderspin",
    "particlemodulelocationcirclesurface",
)


def base_class(name):
    """SourceClass_Matches: drop the ef prefix and the _seeded suffix."""
    name = name or ""
    if name.startswith("efparticlemodule"):
        name = name[2:]
    if name.endswith("_seeded"):
        name = name[: -len("_seeded")]
    return name


def module_enabled(module):
    return literal(module, "benabled", True)


def literal(module, path, fallback=None):
    for entry in module.get("literals") or []:
        if entry.get("propertyPath") == path:
            return entry.get("value")
    return fallback


def find_distribution(module, path):
    for entry in module.get("distributions") or []:
        if entry.get("propertyPath") == path:
            return entry
    return None


def chunk_size(distribution):
    declared = distribution.get("lookupTableChunkSize") or 0
    if declared:
        return declared
    components = distribution.get("componentCount") or 1
    return components * (2 if (distribution.get("operation") or 1) >= 2 else 1)


def table_entry_count(distribution):
    """How many control points the distribution actually carries."""
    table = distribution.get("lookupTable") or []
    if table:
        size = chunk_size(distribution)
        if not size:
            return 0
        payload = max(0, len(table) - COOKED_LOOKUP_RANGE_VALUE_COUNT)
        return payload // size
    return len(distribution.get("keys") or [])


def read_table_value(distribution, entry, maximum):
    components = distribution.get("componentCount") or 1
    size = chunk_size(distribution)
    table = distribution.get("lookupTable") or []
    offset = COOKED_LOOKUP_RANGE_VALUE_COUNT + entry * size
    if maximum and size >= components * 2:
        offset += components
    result = [0.0, 0.0, 0.0, 0.0]
    for component in range(components):
        index = offset + component
        if index < len(table):
            result[component] = float(table[index])
    return result


def control_point_count(distribution):
    """How many control points playback can actually reach.

    Evaluate only advances through the table when lookupTableTimeScale is
    positive; with a zero scale it clamps to entry 0 forever, so a two entry
    table with no time scale is a constant, not a ramp. Reading it as a ramp
    would invent motion the source never had.
    """
    if distribution is None:
        return 0
    if distribution.get("lookupTable"):
        if float(distribution.get("lookupTableTimeScale") or 0.0) <= 0.0:
            return 1 if table_entry_count(distribution) > 0 else 0
        return table_entry_count(distribution)
    return len(distribution.get("keys") or []) or 1


def evaluate(distribution, entry_index, maximum):
    """One end of the distribution range at a given control point.

    maximum=False returns the Minimum branch and maximum=True the Maximum
    branch, which is what SelectRange picks for random unit 0 and 1 under
    operation 2. Operation 1 has no Maximum, so both ends are the Minimum.
    """
    if distribution is None:
        return None
    operation = distribution.get("operation") or 1
    if operation < 2:
        maximum = False
    table = distribution.get("lookupTable") or []
    if table:
        count = table_entry_count(distribution)
        if count <= 0:
            return None
        if float(distribution.get("lookupTableTimeScale") or 0.0) <= 0.0:
            entry_index = 0
        entry = min(max(entry_index, 0), count - 1)
        return read_table_value(distribution, entry, maximum)
    keys = distribution.get("keys") or []
    if keys:
        key = keys[min(max(entry_index, 0), len(keys) - 1)]
        source = key.get("maximum" if maximum else "minimum") or []
        return [float(v) for v in (list(source) + [0.0, 0.0, 0.0, 0.0])[:4]]
    default = distribution.get(
        "defaultMaximum" if maximum else "defaultMinimum") or []
    if not default:
        return None
    return [float(v) for v in (list(default) + [0.0, 0.0, 0.0, 0.0])[:4]]


def expected(distribution, entry_index):
    """The midpoint of the range at one control point.

    Detail keeps a single colour rather than a range, so a distribution that
    randomizes between a minimum and a maximum has to collapse to one number.
    The midpoint is the mean of the uniform draw playback performs.
    """
    minimum = evaluate(distribution, entry_index, False)
    maximum = evaluate(distribution, entry_index, True)
    if minimum is None:
        return None
    if maximum is None:
        return minimum
    return [0.5 * (a + b) for a, b in zip(minimum, maximum)]


def is_meaningful(distribution):
    """A null CDO distribution carries no payload at all; skip it silently."""
    if distribution is None:
        return False
    if distribution.get("lookupTable") or distribution.get("keys"):
        return True
    for name in ("defaultMinimum", "defaultMaximum"):
        if any(abs(float(v)) > 1e-9 for v in distribution.get(name) or []):
            return True
    return False


def ue3_vector_to_client(value):
    return [
        value[0] * UE3_TO_CLIENT_METRES,
        value[2] * UE3_TO_CLIENT_METRES,
        -value[1] * UE3_TO_CLIENT_METRES,
    ]


def component_box(low, high):
    """Order a pair of client-space vectors into a min/max box."""
    return ([min(a, b) for a, b in zip(low, high)],
            [max(a, b) for a, b in zip(low, high)])


def vectors_equal(left, right):
    if left is None or right is None or len(left) != len(right):
        return False
    return all(abs(float(a) - float(b)) < 1e-6 for a, b in zip(left, right))


def box_is_default(minimum, maximum, defaults):
    for low, high in defaults:
        if vectors_equal(minimum, low) and vectors_equal(maximum, high):
            return True
    return False


def modules_of(recipe, *names):
    wanted = set(names)
    return [m for m in recipe.get("modules") or []
            if module_enabled(m) and base_class(m.get("className")) in wanted]


def transplant_position(element, recipe, report):
    """particlemodulelocation.startlocation -> the initialPosition box."""
    particle = element["detail"]["particle"]
    if not box_is_default(particle.get("initialPositionMin"),
                          particle.get("initialPositionMax"),
                          (DEFAULT_POSITION_BOX,)):
        report["position.alreadyAuthored"] += 1
        return False
    modules = modules_of(recipe, "particlemodulelocation")
    low = [0.0, 0.0, 0.0]
    high = [0.0, 0.0, 0.0]
    found = False
    for module in modules:
        distribution = find_distribution(module, "startlocation")
        if not is_meaningful(distribution):
            continue
        minimum = evaluate(distribution, 0, False)
        maximum = evaluate(distribution, 0, True)
        if minimum is None or maximum is None:
            continue
        # Playback sums every location module, so the box sums too.
        client_min = ue3_vector_to_client(minimum)
        client_max = ue3_vector_to_client(maximum)
        low = [a + b for a, b in zip(low, client_min)]
        high = [a + b for a, b in zip(high, client_max)]
        found = True
    if not found:
        report["position.noModule"] += 1
        return False
    minimum, maximum = component_box(low, high)
    if vectors_equal(minimum, [0.0, 0.0, 0.0]) and \
            vectors_equal(maximum, [0.0, 0.0, 0.0]):
        report["position.zero"] += 1
        return False
    particle["initialPositionMin"] = minimum
    particle["initialPositionMax"] = maximum
    report["position.transplanted"] += 1
    return True


def transplant_fixed_velocity(element, recipe, report):
    """particlemodulevelocity.startvelocity -> the initialVelocity box."""
    particle = element["detail"]["particle"]
    if not box_is_default(particle.get("initialVelocityMin"),
                          particle.get("initialVelocityMax"),
                          DEFAULT_VELOCITY_BOXES):
        report["velocity.alreadyAuthored"] += 1
        return False
    low = [0.0, 0.0, 0.0]
    high = [0.0, 0.0, 0.0]
    found = False
    for module in modules_of(recipe, "particlemodulevelocity"):
        distribution = find_distribution(module, "startvelocity")
        if not is_meaningful(distribution):
            continue
        minimum = evaluate(distribution, 0, False)
        maximum = evaluate(distribution, 0, True)
        if minimum is None or maximum is None:
            continue
        client_min = ue3_vector_to_client(minimum)
        client_max = ue3_vector_to_client(maximum)
        low = [a + b for a, b in zip(low, client_min)]
        high = [a + b for a, b in zip(high, client_max)]
        found = True
    if not found:
        report["velocity.noModule"] += 1
        return False
    minimum, maximum = component_box(low, high)
    if vectors_equal(minimum, [0.0, 0.0, 0.0]) and \
            vectors_equal(maximum, [0.0, 0.0, 0.0]):
        report["velocity.zero"] += 1
        return False
    particle["initialVelocityMin"] = minimum
    particle["initialVelocityMax"] = maximum
    report["velocity.transplanted"] += 1
    return True


def primitive_shape(module, report):
    """One location primitive as (spawnShape, outward speed range) or None.

    Only the primitives the four spawnShape kinds can actually hold are
    converted. A cylinder with real height has no kind to land in, so it is
    reported and left to its source modules.
    """
    name = base_class(module.get("className"))
    radius_distribution = find_distribution(module, "startradius")
    radius_min = evaluate(radius_distribution, 0, False)
    radius_max = evaluate(radius_distribution, 0, True)
    if radius_min is None or radius_max is None:
        report["shape.noRadius"] += 1
        return None
    radius = max(abs(radius_min[0]), abs(radius_max[0])) * UE3_TO_CLIENT_METRES
    if radius <= 0.0:
        report["shape.zeroRadius"] += 1
        return None

    if name == "particlemodulelocationprimitivesphere":
        shape = {
            "kind": "sphere",
            "radius": radius,
            "innerRadius": radius if literal(module, "surfaceonly", False)
                           else 0.0,
            "extents": [0.0, 0.0, 0.0],
            "arcDegrees": 360.0,
        }
    elif name == "particlemodulelocationcirclesurface":
        # A circle surface is a ring with no thickness.
        shape = {
            "kind": "ring",
            "radius": radius,
            "innerRadius": radius,
            "extents": [0.0, 0.0, 0.0],
            "arcDegrees": 180.0 if literal(module, "bhalfmode", False)
                          else 360.0,
        }
    elif name in ("particlemodulelocationprimitivecylinder",
                  "particlemodulelocationprimitivecylinderspin"):
        height = evaluate(find_distribution(module, "startheight"), 0, True)
        if height is not None and abs(height[0]) > 1e-6:
            report["shape.cylinderHasHeight"] += 1
            return None
        shape = {
            "kind": "ring",
            "radius": radius,
            "innerRadius": radius if literal(module, "surfaceonly", False)
                           else 0.0,
            "extents": [0.0, 0.0, 0.0],
            "arcDegrees": 360.0,
        }
    else:
        report["shape.unsupportedPrimitive"] += 1
        return None

    speed = None
    if literal(module, "velocity", False) is True:
        scale_distribution = find_distribution(module, "velocityscale")
        scale_min = evaluate(scale_distribution, 0, False)
        scale_max = evaluate(scale_distribution, 0, True)
        if scale_min is not None and scale_max is not None:
            # Playback pushes the spawn offset outward scaled by velocityscale,
            # so the outward speed is the radius times that scale.
            low = radius * scale_min[0]
            high = radius * scale_max[0]
            if abs(low) > 1e-9 or abs(high) > 1e-9:
                speed = [min(low, high), max(low, high)]
    return shape, speed


def transplant_spawn_shape(element, recipe, report):
    """Location primitives -> the spawnShape block and its outward velocity."""
    particle = element["detail"]["particle"]
    if "spawnShape" in particle:
        report["shape.alreadyAuthored"] += 1
        return False
    modules = modules_of(recipe, *LOCATION_PRIMITIVES)
    if not modules:
        return False
    if len(modules) > 1:
        # Two stacked volumes are one composite shape the schema cannot hold.
        report["shape.multiplePrimitives"] += 1
        return False
    converted = primitive_shape(modules[0], report)
    if converted is None:
        return False
    shape, speed = converted
    particle["spawnShape"] = shape
    report["shape.transplanted"] += 1
    if speed is None:
        return True
    if not box_is_default(particle.get("initialVelocityMin"),
                          particle.get("initialVelocityMax"),
                          DEFAULT_VELOCITY_BOXES):
        report["outward.velocityBoxOccupied"] += 1
        return True
    particle["initialVelocity"] = {
        "mode": "inward" if speed[1] < 0.0 else "outward",
        "speed": [abs(speed[0]), abs(speed[1])] if speed[1] >= 0.0
                 else [abs(speed[1]), abs(speed[0])],
        "coneAngleDegrees": 0.0,
    }
    report["outward.transplanted"] += 1
    return True


COLOR_OVER_LIFE_PATHS = {
    "particlemodulecolorscaleoverlife":
        ("colorscaleoverlife", "alphascaleoverlife"),
    "particlemodulecoloroverlife": ("coloroverlife", "alphaoverlife"),
}


def color_over_life_factors(recipe, report):
    """The colour the source paints at the first and last moment of a life.

    coloroverlife assigns and colorscaleoverlife multiplies, in that order, and
    both are re-evaluated every frame at the particle's normalized age. The
    authored host is the LinearLerp colour ramp, which is start to end only, so
    a curve with more than two reachable control points is refused rather than
    flattened into a straight line it never was.

    Returns (assign_start, assign_end, scale_start, scale_end) or None.
    """
    assign = None
    scale_start = [1.0, 1.0, 1.0, 1.0]
    scale_end = [1.0, 1.0, 1.0, 1.0]
    for module in modules_of(recipe, *COLOR_OVER_LIFE_PATHS):
        name = base_class(module.get("className"))
        rgb_path, alpha_path = COLOR_OVER_LIFE_PATHS[name]
        rgb = find_distribution(module, rgb_path)
        alpha = find_distribution(module, alpha_path)
        points = 1
        for distribution in (rgb, alpha):
            if not is_meaningful(distribution):
                continue
            count = control_point_count(distribution)
            if count > 2:
                report["colorOverLife.curveTooComplex"] += 1
                return None
            points = max(points, count)
        if not is_meaningful(rgb) and not is_meaningful(alpha):
            continue
        start = [1.0, 1.0, 1.0, 1.0]
        end = [1.0, 1.0, 1.0, 1.0]
        if is_meaningful(rgb):
            first = expected(rgb, 0)
            last = expected(rgb, points - 1)
            if first is not None:
                start[0], start[1], start[2] = first[0], first[1], first[2]
            if last is not None:
                end[0], end[1], end[2] = last[0], last[1], last[2]
        if is_meaningful(alpha):
            first = expected(alpha, 0)
            last = expected(alpha, points - 1)
            if first is not None:
                start[3] = first[0]
            if last is not None:
                end[3] = last[0]
        if name == "particlemodulecoloroverlife":
            assign = (start, end)
        else:
            scale_start = [a * b for a, b in zip(scale_start, start)]
            scale_end = [a * b for a, b in zip(scale_end, end)]
    if assign is None and scale_start == [1.0, 1.0, 1.0, 1.0] and \
            scale_end == [1.0, 1.0, 1.0, 1.0]:
        return None
    return assign, scale_start, scale_end


def transplant_color(element, recipe, report):
    """The source colour at spawn and at death -> Detail.Color.multiply and the
    LinearLerp colour ramp playback already evaluates per particle age."""
    detail = element["detail"]
    color = detail["color"]
    lerp = detail["linearLerp"]
    if not vectors_equal(color.get("multiply"), DEFAULT_COLOR_MULTIPLY):
        report["color.alreadyAuthored"] += 1
        return False
    if lerp.get("colorMultiply"):
        report["colorOverLife.alreadyAuthored"] += 1
        return False

    base = None
    for module in modules_of(recipe, "particlemodulecolor"):
        rgb = find_distribution(module, "startcolor")
        alpha = find_distribution(module, "startalpha")
        if not is_meaningful(rgb) and not is_meaningful(alpha):
            continue
        base = base or [1.0, 1.0, 1.0, 1.0]
        if is_meaningful(rgb):
            value = expected(rgb, 0)
            if value is not None:
                base[0], base[1], base[2] = value[0], value[1], value[2]
        if is_meaningful(alpha):
            value = expected(alpha, 0)
            if value is not None:
                base[3] = value[0]

    over_life = color_over_life_factors(recipe, report)
    if base is None and over_life is None:
        report["color.noModule"] += 1
        return False

    spawn = list(base or DEFAULT_COLOR_MULTIPLY)
    death = list(spawn)
    if over_life is not None:
        assign, scale_start, scale_end = over_life
        if assign is not None:
            spawn = list(assign[0])
            death = list(assign[1])
        spawn = [a * b for a, b in zip(spawn, scale_start)]
        death = [a * b for a, b in zip(death, scale_end)]

    color["multiply"] = spawn
    report["color.transplanted"] += 1
    if vectors_equal(spawn, death):
        return True
    lerp["colorMultiply"] = True
    lerp["endColorMultiply"] = death
    report["colorOverLife.transplanted"] += 1
    return True


def transplant_size_over_life(element, recipe, report):
    """particlemodulesizemultiplylife -> endSize, only where the authored size
    ramp is provably absent because endSize still equals startSize."""
    particle = element["detail"]["particle"]
    start = particle.get("startSize") or []
    end = particle.get("endSize") or []
    if not vectors_equal(start, end):
        report["sizeOverLife.alreadyAuthored"] += 1
        return False
    ramp = None
    for module in modules_of(recipe, "particlemodulesizemultiplylife"):
        distribution = find_distribution(module, "lifemultiplier")
        if not is_meaningful(distribution):
            continue
        points = control_point_count(distribution)
        if points > 2:
            report["sizeOverLife.curveTooComplex"] += 1
            return False
        first = expected(distribution, 0)
        last = expected(distribution, points - 1)
        if first is None or last is None:
            continue
        if not vectors_equal(first[:2], [1.0, 1.0]):
            # The curve does not start at identity, so the authored startSize
            # already in the document is not the size this ramp begins from.
            # Rewriting only the end would silently rescale the whole Element.
            report["sizeOverLife.startNotIdentity"] += 1
            return False
        ramp = last[:2] if ramp is None else \
            [a * b for a, b in zip(ramp, last[:2])]
    if ramp is None:
        report["sizeOverLife.noModule"] += 1
        return False
    scaled = [start[0] * ramp[0], start[1] * ramp[1]]
    if vectors_equal(scaled, start):
        report["sizeOverLife.identity"] += 1
        return False
    particle["endSize"] = [max(0.0, scaled[0]), max(0.0, scaled[1])]
    report["sizeOverLife.transplanted"] += 1
    return True


def transplant_emitter(element, recipe, report):
    """The recipe's own emitter delay and first burst, where Detail is empty."""
    detail = element["detail"]
    written = False
    delay = float(recipe.get("emitterDelaySeconds") or 0.0)
    if delay > 0.0:
        if abs(float(detail["timing"].get("startDelaySeconds") or 0.0)) < 1e-9:
            detail["timing"]["startDelaySeconds"] = delay
            report["emitterDelay.transplanted"] += 1
            written = True
        else:
            report["emitterDelay.alreadyAuthored"] += 1
    bursts = recipe.get("bursts") or []
    if bursts:
        count = int(bursts[0].get("countMaximum") or
                    bursts[0].get("countMinimum") or 0)
        if count > 0:
            if int(detail["particle"].get("burstCount") or 0) == 0:
                detail["particle"]["burstCount"] = min(
                    count, int(detail["particle"].get("maxParticles") or count))
                report["burst.transplanted"] += 1
                written = True
            else:
                report["burst.alreadyAuthored"] += 1
    if int(recipe.get("emitterLoopCount") or 1) != 1:
        # Detail has no loop count. Left to the source recipe on purpose.
        report["emitterLoops.notRepresentable"] += 1
    return written


def transplant_element(element, report):
    recipe = element.get("sourceRecipe") or {}
    if not recipe.get("enabled"):
        return False
    if element.get("kind") != "particle":
        # A source-owned decal or light is not simulated through the authored
        # particle path at all (Is_SourceVisualDecalParticle requires bEnabled),
        # so flipping it would change which code draws it.
        report["skipped.notParticleKind"] += 1
        return False
    if "detail" not in element:
        report["skipped.noDetail"] += 1
        return False

    transplant_position(element, recipe, report)
    transplant_fixed_velocity(element, recipe, report)
    transplant_spawn_shape(element, recipe, report)
    transplant_color(element, recipe, report)
    transplant_size_over_life(element, recipe, report)
    transplant_emitter(element, recipe, report)

    # sourceRecipe stays: it is the evidence every one of the numbers above came
    # from, and the modules this pass could not read are still only in there.
    recipe["enabled"] = False
    report["elements.ownershipMoved"] += 1
    return True


def commit(staged):
    """Write the staged documents, retrying the transient Windows open error.

    Opening one of these files for write occasionally fails with EINVAL on this
    tree even though the file is present and unlocked. Without a retry the run
    dies part way through and leaves half the documents converted.
    """
    written = 0
    for path, text in staged:
        for attempt in range(5):
            try:
                with open(path, "w", encoding="utf-8", newline="\n") as handle:
                    handle.write(text)
                written += 1
                break
            except OSError as error:
                if attempt == 4:
                    print("write failed: %s: %s" % (path, error))
                    return written
                time.sleep(0.2 * (attempt + 1))
    return written


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--document",
                        help="restrict to one document file name")
    args = parser.parse_args()

    report = collections.Counter()
    touched = []
    staged = []
    for path in sorted(glob.glob(os.path.join(AUTHORED, "*.effect.json"))):
        name = os.path.basename(path)
        if args.document and args.document not in name:
            continue
        with open(path, encoding="utf-8") as handle:
            document = json.load(handle)
        moved = 0
        for element in document.get("elements") or []:
            if transplant_element(element, report):
                moved += 1
        if not moved:
            continue
        report["documents"] += 1
        touched.append((name, moved))
        staged.append((path, json.dumps(
            document, ensure_ascii=False, indent=2) + "\n"))

    # Every document is transformed and serialized before anything is written,
    # so a failure while reading or projecting leaves the tree untouched rather
    # than half converted.
    if args.write:
        written = commit(staged)
        if written != len(staged):
            print("WROTE %d of %d documents; the tree is partially converted."
                  % (written, len(staged)))
            print("Run 'git checkout -- Data/Effects/Authored' and retry.")
            return 1

    print("documents %d   Elements moved to authored ownership %d" % (
        report["documents"], report["elements.ownershipMoved"]))
    print()
    print("per axis")
    for key in sorted(report):
        if key in ("documents", "elements.ownershipMoved"):
            continue
        print("   %-38s %5d" % (key, report[key]))
    print()
    for name, moved in sorted(touched, key=lambda row: -row[1])[:10]:
        print("   %-58s %4d" % (name[:58], moved))
    if not args.write:
        print("\n(dry run; pass --write to rewrite)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
