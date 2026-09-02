from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8-sig")


class DynamicGhostConsumerContractTests(unittest.TestCase):
    def test_client_consumers_use_structural_finale_inventory(self) -> None:
        header = read("Client/Public/ValtanPatternTree.h")
        tree = read("Client/Private/ValtanPatternTree.cpp")
        reference = read("Client/Private/EncounterPatternReference.cpp")

        self.assertIn("std::vector<std::string> GhostPatternIds;", header)
        self.assertNotIn("std::array<std::string, 3u> GhostPatternIds", header)
        self.assertIn("Finale.iMaximumActiveGhosts > 64u", tree)
        self.assertIn("pPatterns->Get_Array().size() > 64u", tree)
        self.assertIn("Finale.GhostPatternIds.push_back", tree)
        self.assertIn("BossArchetypeIds.contains(Pattern.Finale->strGhostArchetypeId)", tree)
        self.assertNotIn("1u != Finale.iMaximumActiveGhosts", tree)
        self.assertNotIn("3u != pPatterns->Get_Array().size()", tree)

        self.assertIn('Read_Unsigned(*finale, "maximumActiveGhosts", 64u', reference)
        self.assertIn("children->Get_Array().size() > 64u", reference)
        self.assertIn("std::unordered_set<std::string> childIds", reference)
        self.assertIn("archetype == bossArchetypeId", reference)
        self.assertNotIn("children->Get_Array().size() != 3u", reference)
        self.assertNotIn("expectedChildren", reference)

    def test_server_and_publisher_accept_variable_children_and_capacity(self) -> None:
        server = read("Server/Private/GameplayCatalog.cpp")
        publisher = read("Tools/GameplayPipeline/Publish-GameplayBalance.ps1")
        server_start = server.index('else if (!fields.empty() && "PATTERNFINALE" == fields[0])')
        server_end = server.index('else if (!fields.empty()', server_start + 1)
        server_finale = server[server_start:server_end]
        publisher_start = publisher.index("\tif ($hasFinale) {")
        publisher_end = publisher.index("\tif ($hasServerMotion) {", publisher_start)
        publisher_finale = publisher[publisher_start:publisher_end]

        self.assertIn("fields.size() < 9u || fields.size() > 72u", server_finale)
        self.assertIn("finale.iMaximumActiveGhosts > 64u", server_finale)
        self.assertIn("index = 8u; index < fields.size(); ++index", server_finale)
        self.assertIn("childPatternIds.emplace(fields[index])", server_finale)
        self.assertIn("m_BossParts.end() != ghostParts && !ghostParts->second.empty()", server)
        self.assertNotIn("11u != fields.size()", server_finale)
        self.assertNotIn("3u != pattern.Finale.GhostPatternIds.size()", server)

        self.assertIn(
            "Assert-JsonInteger $finale.maximumActiveGhosts 'pattern finale maximumActiveGhosts' 1 64",
            publisher_finale,
        )
        self.assertIn("@($finale.ghostPatternIds).Count -gt 64", publisher_finale)
        self.assertIn("$ghostBoss.Count -ne 1", publisher_finale)
        self.assertIn("@($ghostBoss[0].armorPlates).Count -ne 0", publisher_finale)
        self.assertNotIn("$requiredGhostPatterns", publisher_finale)
        self.assertNotIn("@($finale.ghostPatternIds).Count -ne 3", publisher_finale)

    def test_executable_contract_fixtures_cover_dynamic_and_rollback_paths(self) -> None:
        client_test = read(
            "Tools/ValtanPatternAuditionServiceHarness/Private/"
            "ValtanEncounterReferenceContractTests.cpp"
        )
        server_test = read("Server/Private/ServerGameplayContractTests.cpp")

        for source in (client_test, server_test):
            self.assertIn("VALTAN_FOUR_SLASH", source)
            self.assertIn("VALTAN_WHIRLWIND", source)
        self.assertIn("data-driven two-child reordered finale was rejected", client_test)
        self.assertIn('"maximumActiveGhosts", "2"', client_test)
        self.assertIn("finale-data-driven", server_test)
        self.assertIn("const std::string dynamicFinaleRow =", server_test)
        self.assertIn(
            'finalePrefix + "10\\t10\\t2" + finaleChildren;', server_test
        )
        for child_pattern_id in (
            "VALTAN_SIX_PIZZA_106",
            "VALTAN_GROUND_ROAR",
            "VALTAN_STAGGER_SLOT",
            "VALTAN_BIND_SLOT",
            "VALTAN_SILENCE_SLOT",
            "VALTAN_TRIPLE_COUNTER",
        ):
            self.assertIn(child_pattern_id, server_test)
        self.assertIn("finale-duplicate-child", server_test)
        self.assertIn("finale-missing-child", server_test)


if __name__ == "__main__":
    unittest.main()
