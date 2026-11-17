#include "engine/ecs/ArchetypeManager.hpp"
#include "TestHelpers.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

namespace {

    std::filesystem::path writeTemp(const std::string& name, const std::string& body) {
        auto path = std::filesystem::temp_directory_path() / name;
        std::ofstream out(path);
        out << body;
        out.close();
        return path;
    }

} // namespace

TEST_CASE("ArchetypeManager rejects a missing file") {
    engine::ArchetypeManager manager;
    CHECK_FALSE(manager.loadArchetypes("/tmp/does-not-exist-bajabongo-archetypes.json"));
    CHECK(manager.getAllArchetypes().empty());
}

TEST_CASE("ArchetypeManager rejects invalid JSON and a non-object root") {
    engine::ArchetypeManager manager;
    auto bad = writeTemp("baja_archetypes_bad.json", "{ not json");
    CHECK_FALSE(manager.loadArchetypes(bad.string()));

    auto arrayRoot = writeTemp("baja_archetypes_array.json", "[1, 2, 3]");
    CHECK_FALSE(manager.loadArchetypes(arrayRoot.string()));
}

TEST_CASE("ArchetypeManager loads named archetypes from a JSON object") {
    auto path = writeTemp("baja_archetypes_ok.json", R"({
        "player": { "Player": { "moveSpeed": 150.0 } },
        "gopnik_rusher": { "Enemy": { "type": "rusher" } }
    })");

    auto logs = std::make_shared<baja::test::NullLoggerManager>();
    engine::ArchetypeManager manager(logs);
    REQUIRE(manager.loadArchetypes(path.string()));
    REQUIRE(manager.getAllArchetypes().size() == 2);

    const auto* player = manager.getArchetype("player");
    REQUIRE(player != nullptr);
    CHECK((*player)["Player"]["moveSpeed"] == 150.0);

    const auto* rusher = manager.getArchetype("gopnik_rusher");
    REQUIRE(rusher != nullptr);
    CHECK((*rusher)["Enemy"]["type"] == "rusher");

    CHECK(manager.getArchetype("missing") == nullptr);
}
