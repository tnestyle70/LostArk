from __future__ import annotations

import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
V2_DOCUMENT = ROOT / "Client/Private/EffectV2_Document.cpp"
V1_CATALOG = ROOT / "Client/Private/Effect_Catalog.cpp"
RUNTIME_ASSET_ROOT = ROOT / "Client/Private/RuntimeAssetRoot.cpp"


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8-sig")


def function_slice(text: str, signature: str, next_signature: str) -> str:
    begin = text.index(signature)
    end = text.index(next_signature, begin + len(signature))
    return text[begin:end]


class EffectV2ProductContractTests(unittest.TestCase):
    def test_v2_file_loads_stage_before_output_commit(self) -> None:
        source = read(V2_DOCUMENT)
        document_load = function_slice(
            source,
            "bool_t Client::CEffectV2Document::Load_DocumentFile(",
            "bool_t Client::CEffectV2Document::Load_BindingsFile(",
        )
        self.assertIn("EFFECT_V2_DOCUMENT Staged;", document_load)
        self.assertIn("Parse_Document(Text, Staged, strOutError)", document_load)
        self.assertIn("Staged.strEffectId != strEffectId", document_load)
        self.assertIn("OutDocument = std::move(Staged);", document_load)
        self.assertNotIn("Parse_Document(Text, OutDocument", document_load)
        self.assertLess(
            document_load.index("Staged.strEffectId != strEffectId"),
            document_load.index("OutDocument = std::move(Staged);"),
        )

        bindings_load = function_slice(
            source,
            "bool_t Client::CEffectV2Document::Load_BindingsFile(",
            "bool_t Client::CEffectV2Document::Write_AtomicFile(",
        )
        self.assertIn("std::vector<EFFECT_V2_BINDING> Staged;", bindings_load)
        self.assertIn(
            "Parse_Bindings(Text, strArchetypeId, Staged, strOutError)",
            bindings_load,
        )
        self.assertIn("OutBindings = std::move(Staged);", bindings_load)
        self.assertNotIn("Parse_Bindings(Text, strArchetypeId, OutBindings", bindings_load)

    def test_v2_atomic_write_cleans_uncommitted_stage(self) -> None:
        source = read(V2_DOCUMENT)
        atomic_write = source[source.index(
            "bool_t Client::CEffectV2Document::Write_AtomicFile("
        ) :]
        for token in (
            'const std::filesystem::path Temporary = Target.string() + ".tmp";',
            "std::filesystem::remove(Temporary, Error);",
            "std::filesystem::rename(Temporary, Target, Error);",
            'strOutError = "Write failed: "',
            'strOutError = "Rename failed: "',
        ):
            self.assertIn(token, atomic_write)
        self.assertLess(
            atomic_write.index("std::ofstream Stream(Temporary"),
            atomic_write.index("std::filesystem::rename(Temporary, Target, Error)"),
        )

    def test_v1_product_load_stage_has_no_publication_before_exact_commit(self) -> None:
        source = read(V1_CATALOG)
        stage = function_slice(
            source,
            "bool_t Client::CEffectCatalog::Stage_ProductLoadTarget(",
            "Client::EFFECT_PRODUCT_LOAD_COMMIT_RECEIPT::\nEFFECT_PRODUCT_LOAD_COMMIT_RECEIPT(",
        )
        for token in (
            "OutResult.reset();",
            "auto Staged = std::make_shared<EFFECT_PRODUCT_LOAD_STAGE_RESULT>();",
            "Parse_DirectAuthoredRuntimeDocument(",
            "OutResult = std::move(Staged);",
        ):
            self.assertIn(token, stage)
        self.assertNotIn("g_Effects.emplace", stage)
        self.assertNotIn("g_VisualProjections.emplace", stage)

        commit = function_slice(
            source,
            "bool_t Client::CEffectCatalog::Commit_ProductLoadStage(\n\tconst std::shared_ptr<const EFFECT_PRODUCT_LOAD_STAGE_RESULT>& pResult,\n\tEFFECT_PRODUCT_LOAD_COMMIT_RECEIPT& OutCommitReceipt,",
            "bool_t Client::CEffectCatalog::Commit_ProductLoadStage(\n\tconst std::shared_ptr<const EFFECT_PRODUCT_LOAD_STAGE_RESULT>& pResult,\n\tstd::shared_ptr<const EFFECT_DOCUMENT_DESC>& OutDocument,",
        )
        for token in (
            "g_iCatalogOwnerThreadId != static_cast<uint32_t>(GetCurrentThreadId())",
            "g_iRuntimeRevision != pResult->Request.iCatalogRevision",
            "pResult->Request.pSourceRegistrationIdentity.get()",
            "Product Effect source changed before staged commit.",
            "g_Effects.emplace(EffectAssetId, pResult->pDocument)",
            "g_Effects.erase(InsertedDocument);",
            "OutCommitReceipt = std::move(CommitReceipt);",
        ):
            self.assertIn(token, commit)
        self.assertLess(
            commit.index("g_Effects.emplace(EffectAssetId, pResult->pDocument)"),
            commit.index("OutCommitReceipt = std::move(CommitReceipt);"),
        )

        rollback = function_slice(
            source,
            "bool_t Client::CEffectCatalog::Rollback_ProductLoadStage(",
            "bool_t Client::CEffectCatalog::Stage_DebugDirectAuthoredReplacement(",
        )
        for token in (
            "Product Effect load-stage rollback exact identity is stale.",
            "Document->second.get() != Result->pDocument.get()",
            "g_VisualProjections.erase(Projection);",
            "g_Effects.erase(Document);",
            "CommitReceipt.Invalidate();",
        ):
            self.assertIn(token, rollback)

    def test_product_resource_root_precedence_and_containment_remain_typed(self) -> None:
        source = read(RUNTIME_ASSET_ROOT)
        self.assertLess(
            source.index('ReadEnvironmentPath(L"LOSTARK_RESOURCE_ROOT")'),
            source.index('ReadEnvironmentPath(L"LOSTARK_SHARED_ASSET_ROOT")'),
        )
        resolve = function_slice(
            source,
            "filesystem::path CRuntimeAssetRoot::Resolve(",
            "filesystem::path CRuntimeAssetRoot::Resolve_Font(",
        )
        self.assertIn("ResolveInsideRoot(Get(), relativePath)", resolve)
        containment = function_slice(
            source,
            "filesystem::path ResolveInsideRoot(",
            "}\n\nfilesystem::path CRuntimeAssetRoot::Get_ResourceRoot()",
        )
        for token in (
            "relativePath.is_absolute()",
            "relativePath.has_root_path()",
            "Canonicalize(root / relativePath)",
            "!IsInsideRoot(canonicalRoot, canonicalCandidate)",
        ):
            self.assertIn(token, containment)


if __name__ == "__main__":
    unittest.main()
