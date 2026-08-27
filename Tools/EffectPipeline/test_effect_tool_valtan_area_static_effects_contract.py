from __future__ import annotations

import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


tool = read("Client/Private/Effect_Tool.cpp")
tool_header = read("Client/Public/Effect_Tool.h")
catalog_header = read("Client/Public/Effect_Catalog.h")
catalog_runtime = read("Client/Private/Effect_Catalog.cpp")
document_header = read("Client/Public/MapEffectDocument.h")
document_runtime = read("Client/Private/MapEffectDocument.cpp")
presentation_header = read("Client/Public/MapEffectPresentationRuntime.h")
presentation_runtime = read("Client/Private/MapEffectPresentationRuntime.cpp")
source = json.loads(read(
    "Data/Maps/Authoring/LV_LUT_HEARTRB_ED/"
    "LV_LUT_HEARTRB_ED.mapeffects.json"
))

for required in (
    "Render_ValtanAreaStaticEffectSection",
    "AREA-STATIC",
    "PATTERN-OWNED INDEPENDENT EFFECT (2)",
    "INDEPENDENT EFFECT (PATTERN 2 + AREA ",
    "Apply Draft to Live Valtan",
    "Save + Publish",
    "Reload Source",
    "Reload Published",
    "Emissive Intensity",
    "Emissive Color",
    "Mask Power",
    "Position",
    "Scale",
    "Rotation Quaternion",
    "Orientation Policy",
    "Open Editor",
    "Preview at Placement",
    "Play Server Activation Owner",
    "New Static World Effect",
    "UNPUBLISHED DRAFT",
    "Register in Area",
):
    require(required in tool, f"Area/static Effect Tool surface missing: {required}")

render_all = tool[
    tool.index("void Client::CEffect_Tool::Render_AllEffectsWindow"):
    tool.index("bool_t Client::CEffect_Tool::Try_CreateDocument")
]
render_valtan = tool[
    tool.index("void Client::CEffect_Tool::Render_ValtanPatternTreeSection"):
    tool.index("void Client::CEffect_Tool::Render_AllEffectsWindow")
]
require(
    "Render_ValtanPatternTreeSection(Search)" in render_all
    and "Render_ValtanAreaStaticEffectSection(Search)" not in render_all
    and "Render_ValtanAreaStaticEffectSection(strSearch)" in render_valtan,
    "Area/static rows must be nested under Valtan Independent Effect inventory",
)
require(
    '"valtan.independent-effect.donut-in-out"' in tool
    and '"valtan.independent-effect.target-axe"' in tool
    and "VALTAN_ALL_EFFECTS_INDEPENDENT_EFFECT_IDS" in tool,
    "the exact two pattern-owned Independent Effect rows were not preserved",
)
require("boss.valtan.center" not in tool[
    tool.index("bool_t Client::CEffect_Tool::Try_ApplyValtanAreaStaticEffectDraft"):
    tool.index("void Client::CEffect_Tool::Render_ValtanPatternTreeSection")
], "Area/static Apply must not manufacture a boss owner")

for required in (
    "Save_AtomicIfUnchanged",
    "expectedRawBytes",
    "Load_WithRawBaseline",
    "Restore_RawBytesAtomicIfUnchanged",
    "FlushFileBuffers",
    "MoveFileExW",
    "MOVEFILE_WRITE_THROUGH",
    "Map Effect source changed on disk after it was loaded",
):
    require(required in document_header + document_runtime,
            f"Map Effect source CAS contract missing: {required}")

for required in (
    "Request_DebugApply",
    "Request_PublishedReload",
    "Commit_StagedDocument",
    "Set_SurfacePresentations",
    "Probe_WorldEffectAdmissions",
    "m_Document = std::move(stagedDocument)",
):
    require(required in presentation_header + presentation_runtime,
            f"Map Effect typed live transaction missing: {required}")

require(
    "CMapAssetCatalog::Get_MapDataRoot()" in presentation_runtime,
    "Product Area load must consume the published Map data root",
)
require(
    "VALTAN_AREA_MAP_EFFECT_SOURCE" in tool
    and "Maps/Authoring/LV_LUT_HEARTRB_ED" in tool,
    "Effect Tool must consume the source authoring document, not runtime JSON",
)
require(
    "Publish-MapAuthoring.ps1" in tool
    and "Source CAS rollback succeeded" in tool
    and "Write_TransactionRawBytesAtomicIfUnchanged" in tool
    and "Create_TransactionRawBytesDurableIfAbsent" in tool
    and "Remove_TransactionFileIfExactRaw" in tool,
    "Save + Publish does not have publisher failure rollback",
)
for required in (
    "STATIC_AREA_PLACEMENT",
    "Update_StaticAreaPreviewRoot",
    "Try_CreateStaticAreaWorldEffectDraft",
    "Try_RegisterStaticAreaWorldEffectDraft",
    "Replace_ProductPreparedTarget",
    "Product consumer(s)",
):
    require(required in tool + tool_header,
            f"static placement/register contract missing: {required}")

registration = tool[
    tool.index("bool_t Client::CEffect_Tool::Try_RegisterStaticAreaWorldEffectDraft()"):
    tool.index(
        "void Client::CEffect_Tool::Render_ValtanAreaStaticEffectSection(",
        tool.index(
            "bool_t Client::CEffect_Tool::Try_RegisterStaticAreaWorldEffectDraft()"
        ),
    )
]
require(
    "Create_TransactionRawBytesDurableIfAbsent(" in registration,
    "new authored Effect source is not created through the durable "
    "collision-safe transaction seam",
)
require(
    "CEffectDocumentCodec::Save_AtomicIfUnchanged(" not in registration,
    "static world registration still bypasses durable raw source creation",
)

for required in (
    "EFFECT_DEBUG_DIRECT_AUTHORED_REGISTRATION",
    "Stage_DebugDirectAuthoredRegistration",
    "Commit_DebugDirectAuthoredRegistration",
    "Restore_DebugDirectAuthoredRegistration",
    "Stage_SourceDirectAuthoredCatalog",
    "unchanged catalog revision",
):
    require(required in catalog_header + catalog_runtime,
            f"typed direct-authored registration contract missing: {required}")

static_update = tool[
    tool.index("void Client::CEffect_Tool::Update(const f32_t fTimeDelta)"):
    tool.index("void Client::CEffect_Tool::Render()")
]
require(
    "bStaticAreaEffectActive" in static_update
    and "Update_StaticAreaPreviewRoot" in static_update,
    "static placement transform must be refreshed every frame",
)

rows = source["presentations"]
surface_rows = [row for row in rows
                if row["presentationKind"] == "DEPLOY_SURFACE_OVERLAY"]
world_rows = [row for row in rows
              if row["presentationKind"] == "EFFECT_DOCUMENT"]
require(len(surface_rows) == 1 and len(world_rows) == 1,
        "focused Valtan authoring inventory must contain one surface and one world row")
require(surface_rows[0]["visibleStates"] == ["INTACT"],
        "surface authoring must use the exact destruction visibility gate")
require(world_rows[0]["effectAssetId"] ==
        "effect.valtan.environment.red-vortex-sky",
        "world row lost the exact direct-authored Effect target")
require(world_rows[0]["activationPolicy"] == "SERVER_PATTERN_WINDOW"
        and world_rows[0]["playbackPolicy"] == "SERVER_CLOCK_SAMPLE",
        "world row must stay sampled from the Server pattern clock")

print("Effect Tool Valtan Area/static Effect contract tests passed")
