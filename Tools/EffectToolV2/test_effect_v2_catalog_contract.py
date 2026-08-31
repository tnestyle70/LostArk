from __future__ import annotations

import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "Client/Public/EffectV2_Catalog.h"
SOURCE = ROOT / "Client/Private/EffectV2_Catalog.cpp"
PROJECT = ROOT / "Client/Default/Client.vcxproj"
FILTERS = ROOT / "Client/Default/Client.vcxproj.filters"
AUTHORED = ROOT / "Data/Effects/V2/Authored"
GROUPS = ROOT / "Data/Effects/V2/Groups"
BOSS_BINDINGS = ROOT / "Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json"


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8-sig")


def function_tail(text: str, signature: str, next_signature: str) -> str:
    begin = text.index(signature)
    end = text.index(next_signature, begin + len(signature))
    return text[begin:end]


class EffectV2CatalogContractTests(unittest.TestCase):
    def test_catalog_is_registered_as_one_effect_v2_source_pair(self) -> None:
        project = read(PROJECT)
        filters = read(FILTERS)
        for text in (project, filters):
            self.assertEqual(text.count(r"..\Public\EffectV2_Catalog.h"), 1)
            self.assertEqual(text.count(r"..\Private\EffectV2_Catalog.cpp"), 1)
        self.assertIn("03. Tools\\06. Effect V2", filters)

    def test_snapshot_is_read_only_and_queries_do_no_io(self) -> None:
        header = read(HEADER)
        source = read(SOURCE)
        for token in (
            "std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>",
            "const std::vector<EFFECT_V2_DOCUMENT>&",
            "const std::vector<EFFECT_V2_GROUP>&",
            "const std::vector<EFFECT_V2_BINDING>&",
            "Find_Document(",
            "Find_Group(",
        ):
            self.assertIn(token, header)

        query_region = source[source.index(
            "uint64_t Client::EFFECT_V2_CATALOG_SNAPSHOT::Get_Revision()"
        ) : source.index("Client::CEffectV2Catalog& Client::CEffectV2Catalog::Get()")]
        for forbidden in (
            "filesystem",
            "Load_DocumentFile",
            "Load_GroupFile",
            "Load_BindingsFile",
            "directory_iterator",
        ):
            self.assertNotIn(forbidden, query_region)
        self.assertNotIn("Render", header)
        self.assertNotIn("Tick", header)

    def test_explicit_reload_stages_cross_validates_then_commits_once(self) -> None:
        source = read(SOURCE)
        reload_body = function_tail(
            source,
            "bool_t Client::CEffectV2Catalog::Reload_BossValtan(",
            "bool_t Client::CEffectV2Catalog::Commit_BossValtanBindingsLocked(",
        )
        ordered = (
            "Stage_Documents(StagedDocuments",
            "Stage_Groups(StagedGroups",
            "CEffectV2Document::Load_BindingsFile(",
            "Cross_Validate(",
            "Validate_NoLeafGroupClockOverlap(",
            "std::make_shared<EFFECT_V2_CATALOG_SNAPSHOT>()",
            "pStaged->m_iRevision = iPreviousRevision + 1u;",
            "m_pSnapshot = std::move(pStaged);",
        )
        positions = [reload_body.index(token) for token in ordered]
        self.assertEqual(positions, sorted(positions))
        self.assertEqual(reload_body.count("m_pSnapshot ="), 1)
        self.assertIn("return false;", reload_body[: positions[5]])
        self.assertIn("const std::lock_guard Lock(m_SnapshotMutex);", reload_body)
        self.assertIn("failed before commit", reload_body)

    def test_binding_commit_self_validates_then_atomically_commits(self) -> None:
        source = read(SOURCE)
        commit_body = function_tail(
            source,
            "bool_t Client::CEffectV2Catalog::Commit_BossValtanBindingsLocked(",
            "bool_t Client::CEffectV2Catalog::Mutate_BossValtanStageBinding(",
        )
        ordered = (
            "Cross_Validate(",
            "CEffectV2Document::Serialize_Bindings(",
            "CEffectV2Document::Parse_Bindings(",
            "Stage_Documents(DiskDocuments",
            "Stage_Groups(DiskGroups",
            "CEffectV2Document::Load_BindingsFile(",
            "strDiskBaseline != strSnapshotBaseline",
            "CEffectV2Document::Write_AtomicFile(",
            "m_pSnapshot = std::move(pCandidate);",
        )
        positions = [commit_body.index(token) for token in ordered]
        self.assertEqual(positions, sorted(positions))
        self.assertIn("BOSS_VALTAN.effectv2bindings.json", commit_body)
        self.assertIn("Product publish + Server restart", commit_body)
        self.assertIn("source changed; reload", commit_body)
        self.assertIn("Matches_DocumentBaseline(", commit_body)
        self.assertIn("Matches_GroupBaseline(", commit_body)
        self.assertIn("DiskDocuments, DiskGroups,", commit_body)
        self.assertIn("Validate_NoLeafGroupClockOverlap(", commit_body)
        self.assertEqual(commit_body.count("m_pSnapshot ="), 1)

    def test_stage_binding_mutations_use_one_typed_exact_key(self) -> None:
        header = read(HEADER)
        source = read(SOURCE)
        for token in (
            "struct EFFECT_V2_STAGE_BINDING_KEY final",
            "From_StageBinding(",
            "std::string strResourceId;",
            "bool_t bGroup = false;",
            "std::string strStageActionId;",
            "uint32_t iStartMs = 0u;",
            "std::string strBone;",
            "bool_t bFollowBone = false;",
            "CEffectV2Object::PIVOT_ROTATION eRotation =",
            "bool_t bStopWithClip = false;",
            "float3_t vOffset = { 0.f, 0.f, 0.f };",
            "f32_t fYawDegrees = 0.f;",
            "Remove_BossValtanStageBinding(",
            "Duplicate_BossValtanStageBinding(",
            "Update_BossValtanStageBindingStart(",
        ):
            self.assertIn(token, header)

        mutation_body = function_tail(
            source,
            "bool_t Client::CEffectV2Catalog::Mutate_BossValtanStageBinding(",
            "bool_t Client::CEffectV2Catalog::Append_BossValtanStageBinding(",
        )
        for token in (
            "Resolve_UniqueStageBindingIndex(",
            "CandidateBindings.erase(",
            "EFFECT_V2_BINDING Duplicate =",
            "Duplicate.iStartMs = iTargetStartMs;",
            "CandidateBindings[iSourceIndex].iStartMs = iTargetStartMs;",
            "Commit_BossValtanBindingsLocked(",
        ):
            self.assertIn(token, mutation_body)

        match_body = function_tail(
            source,
            "\tbool_t Matches_StageBindingKey(",
            "\tbool_t Resolve_UniqueStageBindingIndex(",
        )
        for field in (
            "Binding.strBone == Key.strBone",
            "Binding.bFollowBone == Key.bFollowBone",
            "Binding.eRotation == Key.eRotation",
            "Binding.bStopWithClip == Key.bStopWithClip",
            "Binding.vOffset.x == Key.vOffset.x",
            "Binding.vOffset.y == Key.vOffset.y",
            "Binding.vOffset.z == Key.vOffset.z",
            "Binding.fYawDegrees == Key.fYawDegrees",
        ):
            self.assertIn(field, match_body)

        key_factory = function_tail(
            source,
            "Client::EFFECT_V2_STAGE_BINDING_KEY::From_StageBinding(",
            "Client::CEffectV2Catalog& Client::CEffectV2Catalog::Get()",
        )
        for field in (
            "Binding.strGroupId",
            "Binding.strEffectId",
            "Binding.strStage",
            "Binding.iStartMs",
            "Binding.strBone",
            "Binding.bFollowBone",
            "Binding.eRotation",
            "Binding.bStopWithClip",
            "Binding.vOffset",
            "Binding.fYawDegrees",
        ):
            self.assertIn(field, key_factory)

        wrapper_region = source[source.index(
            "bool_t Client::CEffectV2Catalog::Append_BossValtanStageBinding("
        ) : source.index(
            "std::shared_ptr<const Client::EFFECT_V2_CATALOG_SNAPSHOT>"
        )]
        self.assertEqual(wrapper_region.count("Mutate_BossValtanStageBinding("), 4)

    def test_new_boss_mutations_reject_foreign_subjects_and_overlaps(self) -> None:
        source = read(SOURCE)
        validation = function_tail(
            source,
            "\tbool_t Validate_BossValtanMutationSubject(",
            "\tbool_t Matches_StageBindingKey(",
        )
        self.assertIn('BOSS_VALTAN_RESOURCE_PREFIX = "boss.valtan."', source)
        self.assertIn("starts_with(BOSS_VALTAN_RESOURCE_PREFIX)", validation)

        overlap = function_tail(
            source,
            "\tbool_t Validate_NoLeafGroupClockOverlap(",
            "}\n\nuint64_t Client::EFFECT_V2_CATALOG_SNAPSHOT::Get_Revision()",
        )
        for token in (
            "GroupBinding.iStartMs",
            "Child.iStartMs",
            "Child.strEffectId == LeafBinding.strEffectId",
            "iEffectiveStartMs == LeafBinding.iStartMs",
            "leaf overlaps the same leaf inside group",
        ):
            self.assertIn(token, overlap)

        mutation_body = function_tail(
            source,
            "bool_t Client::CEffectV2Catalog::Mutate_BossValtanStageBinding(",
            "bool_t Client::CEffectV2Catalog::Append_BossValtanStageBinding(",
        )
        self.assertIn(
            "BOSS_VALTAN_BINDING_MUTATION::REMOVE_BINDING != eMutation &&",
            mutation_body,
        )
        self.assertIn("Validate_BossValtanMutationSubject(", mutation_body)
        self.assertIn("Validate_NoLeafGroupClockOverlap(", mutation_body)

    def test_cross_validation_joins_documents_groups_and_boss_bindings(self) -> None:
        source = read(SOURCE)
        validation = function_tail(
            source,
            "\tbool_t Cross_Validate(",
            "}\n\nuint64_t Client::EFFECT_V2_CATALOG_SNAPSHOT::Get_Revision()",
        )
        for token in (
            "DocumentsById.emplace",
            "groupId collides with an authored effect",
            "Group.Children",
            "group child has no authored leaf document",
            "Binding.strEffectId",
            "Binding.strEffectId.empty() == Binding.strGroupId.empty()",
            "Binding.strClip.empty() == Binding.strStage.empty()",
            "GroupsById.contains(Binding.strGroupId)",
        ):
            self.assertIn(token, validation)

        documents = {
            path.name.removesuffix(".effectv2.json")
            for path in AUTHORED.glob("*.effectv2.json")
        }
        groups = {}
        for path in GROUPS.glob("*.effectv2group.json"):
            value = json.loads(path.read_text(encoding="utf-8"))
            groups[value["groupId"]] = value
            self.assertNotIn(value["groupId"], documents)
            for child in value["children"]:
                self.assertIn(child["effectId"], documents)

        bindings = json.loads(BOSS_BINDINGS.read_text(encoding="utf-8"))
        self.assertEqual(bindings["archetypeId"], "BOSS_VALTAN")
        self.assertGreater(len(bindings["bindings"]), 0)
        for binding in bindings["bindings"]:
            if "effectId" in binding:
                self.assertIn(binding["effectId"], documents)
            else:
                self.assertIn(binding["group"], groups)


if __name__ == "__main__":
    unittest.main()
