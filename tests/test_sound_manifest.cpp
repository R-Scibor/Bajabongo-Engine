#include "game/systems/DeathSounds.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <set>
#include <string>

#ifndef BAJA_ASSETS_DIR
#define BAJA_ASSETS_DIR ""
#endif

namespace {

    nlohmann::json loadJson(const std::filesystem::path& path) {
        std::ifstream in(path);
        REQUIRE(in.is_open());
        return nlohmann::json::parse(in);
    }

    std::set<std::string> manifestSoundIds() {
        const auto manifest = loadJson(std::filesystem::path{BAJA_ASSETS_DIR} / "data" / "resources.json");
        std::set<std::string> ids;
        for (const auto& sound : manifest.at("sounds")) {
            ids.insert(sound.at("id").get<std::string>());
        }
        return ids;
    }

} // namespace

TEST_CASE("Every death sound DamageSystem can play is listed in resources.json") {
    const auto ids = manifestSoundIds();

    CHECK(ids.contains(std::string{game::death_sounds::PLAYER}));
    for (const auto id : game::death_sounds::ENEMY) {
        INFO("enemy death sound: " << id);
        CHECK(ids.contains(std::string{id}));
    }
}

TEST_CASE("Every weapon shoot sound in archetypes.json is listed in resources.json") {
    const auto ids = manifestSoundIds();
    const auto archetypes = loadJson(std::filesystem::path{BAJA_ASSETS_DIR} / "data" / "archetypes.json");

    int checked = 0;
    for (const auto& [name, archetype] : archetypes.items()) {
        if (!archetype.is_object() || !archetype.contains("Weapon")) continue;
        const auto soundId = archetype.at("Weapon").value("shootSoundId", std::string{});
        if (soundId.empty()) continue;
        INFO("archetype " << name << " uses " << soundId);
        CHECK(ids.contains(soundId));
        ++checked;
    }
    CHECK(checked > 0);
}
