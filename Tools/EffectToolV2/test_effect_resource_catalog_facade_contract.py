from __future__ import annotations

import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "Client/Public/EffectResourceCatalog.h"
SOURCE = ROOT / "Client/Private/EffectResourceCatalog.cpp"
PROJECT = ROOT / "Client/Default/Client.vcxproj"
FILTERS = ROOT / "Client/Default/Client.vcxproj.filters"
EFFECT_TOOL_HEADER = ROOT / "Client/Public/Effect_Tool.h"
EFFECT_TOOL_SOURCE = ROOT / "Client/Private/Effect_Tool.cpp"
EFFECT_V2_HEADER = ROOT / "Client/Public/Effect_Tool_V2.h"
EFFECT_V2_SOURCE = ROOT / "Client/Private/Effect_Tool_V2.cpp"
MAIN_APP_SOURCE = ROOT / "Client/Private/MainApp.cpp"


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8-sig")


def function_tail(text: str, signature: str, next_signature: str) -> str:
    begin = text.index(signature)
    end = text.index(next_signature, begin + len(signature))
    return text[begin:end]


class EffectResourceCatalogFacadeContractTests(unittest.TestCase):
    def test_facade_is_registered_once_in_the_common_effect_filter(self) -> None:
        project = read(PROJECT)
        filters = read(FILTERS)
        for text in (project, filters):
            self.assertEqual(text.count(r"..\Public\EffectResourceCatalog.h"), 1)
            self.assertEqual(text.count(r"..\Private\EffectResourceCatalog.cpp"), 1)
        for filename in ("EffectResourceCatalog.h", "EffectResourceCatalog.cpp"):
            entry = re.search(
                rf'<Cl(?:Include|Compile) Include="[^\"]*{filename}">(.*?)</Cl(?:Include|Compile)>',
                filters,
                re.DOTALL,
            )
            self.assertIsNotNone(entry)
            self.assertIn(r"03. Tools\02. Effect", entry.group(1))

    def test_public_contract_is_one_version_neutral_resource_view(self) -> None:
        header = read(HEADER)
        for token in (
            "EFFECT_RESOURCE_OWNER_KIND",
            "EFFECT_RESOURCE_KEY",
            "EFFECT_RESOURCE_CAPABILITIES",
            "EFFECT_RESOURCE_DESCRIPTOR",
            "EFFECT_RESOURCE_CATALOG_SNAPSHOT",
            "Reload_Valtan(",
            "Get_Snapshot() const",
            "Get_Revision() const",
            "Find(\n\t\tconst EFFECT_RESOURCE_KEY& Key)",
            "Find(\n\t\tstd::string_view strStableId)",
        ):
            self.assertIn(token, header)
        for capability in (
            "bCanLoad",
            "bCanAppendToStage",
            "bCanSave",
            "bCanReload",
            "bCanPreview",
        ):
            self.assertIn(capability, header)
        for owner_kind in ("V1_DOCUMENT", "V2_LEAF", "V2_GROUP"):
            self.assertIn(owner_kind, header)

    def test_parse_consumes_only_ready_owner_catalog_snapshots(self) -> None:
        source = read(SOURCE)
        parse = function_tail(
            source,
            "\tbool_t Parse_ValtanOwnerSnapshots(",
            "\tbool_t Validate_ValtanResources(",
        )
        ordered = (
            "CEffectCatalog::Get_RuntimeRevision()",
            "CEffectCatalog::Get_EffectAssetIds()",
            "CEffectCatalog::Is_DirectAuthoredDocument(strEffectId)",
            "CEffectV2Catalog::Get().Get_Snapshot()",
            "pTypedSnapshot->Is_Ready()",
            "pTypedSnapshot->Get_Documents()",
            "pTypedSnapshot->Get_Groups()",
        )
        positions = [parse.index(token) for token in ordered]
        self.assertEqual(positions, sorted(positions))
        self.assertIn('"effect.valtan."', source)
        self.assertIn('"boss.valtan."', source)

    def test_reload_is_parse_validate_stage_then_one_atomic_commit(self) -> None:
        source = read(SOURCE)
        reload_body = function_tail(
            source,
            "bool_t Client::CEffectResourceCatalog::Reload_Valtan(",
            "std::shared_ptr<const Client::EFFECT_RESOURCE_CATALOG_SNAPSHOT>",
        )
        ordered = (
            "Parse_ValtanOwnerSnapshots(",
            "Validate_ValtanResources(",
            "Stage_ValtanSnapshot(",
            "CEffectCatalog::Get_RuntimeRevision()",
            "CEffectV2Catalog::Get().Get_Revision()",
            "const std::lock_guard Lock(m_SnapshotMutex);",
            "pStaged->m_iRevision = iPreviousRevision + 1u;",
            "m_pSnapshot = std::move(pStaged);",
        )
        positions = [reload_body.index(token) for token in ordered]
        self.assertEqual(positions, sorted(positions))
        self.assertEqual(reload_body.count("m_pSnapshot ="), 1)
        self.assertIn("failed before commit", reload_body)
        self.assertGreater(reload_body.index("m_pSnapshot ="), reload_body.rindex("return false;"))

    def test_stable_id_alone_is_the_unique_namespace(self) -> None:
        source = read(SOURCE)
        validate = function_tail(
            source,
            "\tbool_t Validate_ValtanResources(",
            "bool_t Client::EFFECT_RESOURCE_KEY::Is_Valid()",
        )
        self.assertIn("Validated.StableIdIndex.emplace(", validate)
        self.assertIn("Descriptor.Key.strStableId, iResource", validate)
        self.assertIn("if (!bInserted)", validate)
        self.assertIn("stable ID collision", validate)
        self.assertLess(validate.index("if (!bInserted)"), validate.index("OutValidated ="))
        self.assertNotIn("eOwnerKind, iResource", validate)

    def test_dual_owner_stable_id_is_rejected_before_snapshot_commit(self) -> None:
        source = read(SOURCE)
        validate = function_tail(
            source,
            "\tbool_t Validate_ValtanResources(",
            "bool_t Client::EFFECT_RESOURCE_KEY::Is_Valid()",
        )
        self.assertIn("Validated.StableIdIndex.emplace(", validate)
        self.assertIn("if (!bInserted)", validate)
        self.assertIn("Unified Effect Resource stable ID collision", validate)
        self.assertNotIn("insert_or_assign", validate)

    def test_labels_are_version_neutral_and_capabilities_are_validated(self) -> None:
        source = read(SOURCE)
        self.assertIn("Descriptor.strDisplayLabel = Descriptor.Key.strStableId;", source)
        self.assertIn("Matches_Capabilities(", source)
        for category in (
            '"Authored"',
            '"Mesh"',
            '"Texture"',
            '"Particle"',
            '"Decal"',
            '"Trail"',
            '"Screen Post"',
            '"Composite"',
        ):
            self.assertIn(category, source)
        for forbidden_label in ('"V1 ', '"V2 ', '"Effect V1', '"Effect V2'):
            self.assertNotIn(forbidden_label, source)

    def test_v1_document_is_an_atomic_composite_and_remains_clip_appendable(self) -> None:
        source = read(SOURCE)
        capabilities = function_tail(
            source,
            "\tEFFECT_RESOURCE_CAPABILITIES Capabilities_For(",
            "\tbool_t Matches_Capabilities(",
        )
        v1_case = capabilities[
            capabilities.index("case EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT:") :
            capabilities.index("case EFFECT_RESOURCE_OWNER_KIND::V2_LEAF:")
        ]
        self.assertIn("Capabilities.bCanAppendToClip = true;", v1_case)
        self.assertIn("Capabilities.bComposite = true;", v1_case)

    def test_all_effects_consumes_last_good_facade_and_groups_v1_documents(self) -> None:
        header = read(EFFECT_TOOL_HEADER)
        source = read(EFFECT_TOOL_SOURCE)
        for token in (
            "m_pValtanEffectResourceSnapshot",
            "m_eValtanEffectResourceAdmission",
            "Refresh_ValtanEffectResourceSnapshot",
            "Render_ValtanEffectResourceSection(strSearch)",
            'RenderRows("GROUPS / COMPOSITIONS", Groups)',
            'RenderRows("LEAVES", Leaves)',
            "Resource.Capabilities.bComposite ? Groups : Leaves",
            "VALTAN_VIEW_ADMISSION::STALE_PRESERVED",
            "STALE PRESERVED / READ ONLY",
            "Can_MutateValtanView(m_eValtanEffectResourceAdmission)",
        ):
            self.assertIn(token, header + source)
        self.assertLess(
            source.index("Render_ValtanEffectResourceSection(strSearch)"),
            source.index("Render_ValtanExactAuthoredSourceSection(strSearch)"),
        )

    def test_all_effects_dispatches_open_to_the_exact_owner_backend(self) -> None:
        effect_header = read(EFFECT_TOOL_HEADER)
        effect_source = read(EFFECT_TOOL_SOURCE)
        typed_header = read(EFFECT_V2_HEADER)
        typed_source = read(EFFECT_V2_SOURCE)
        main_app = read(MAIN_APP_SOURCE)
        for token in (
            "Consume_TypedEffectResourceOpenRequest",
            "m_PendingTypedEffectResourceOpen = Resource.Key",
            "EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT",
            "Try_LoadDocumentPath(",
        ):
            self.assertIn(token, effect_header + effect_source)
        for token in (
            "Open_Resource(const EFFECT_RESOURCE_KEY& Key)",
            "EFFECT_RESOURCE_OWNER_KIND::V2_LEAF",
            "EFFECT_RESOURCE_OWNER_KIND::V2_GROUP",
            "Load_Document(Key.strStableId)",
            "Load_Group(Key.strStableId)",
        ):
            self.assertIn(token, typed_header + typed_source)
        self.assertIn(
            "m_pEffectToolV2->Open_Resource(ResourceKey)", main_app)

    def test_snapshot_queries_are_immutable_and_io_free(self) -> None:
        header = read(HEADER)
        source = read(SOURCE)
        query_region = function_tail(
            source,
            "uint64_t Client::EFFECT_RESOURCE_CATALOG_SNAPSHOT::Get_Revision()",
            "Client::CEffectResourceCatalog& Client::CEffectResourceCatalog::Get()",
        )
        self.assertIn("std::shared_ptr<const EFFECT_RESOURCE_CATALOG_SNAPSHOT>", header)
        self.assertIn("m_StableIdIndex.find(strStableId)", query_region)
        for forbidden in (
            "filesystem",
            "directory_iterator",
            "Load_DocumentFile",
            "Load_GroupFile",
            "Effect_DocumentCodec",
            "Render",
            "Tick",
        ):
            self.assertNotIn(forbidden, query_region)
        self.assertNotIn("filesystem", source)
        self.assertNotIn("directory_iterator", source)


if __name__ == "__main__":
    unittest.main()
