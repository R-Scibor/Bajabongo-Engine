#include "engine/assets/MapLoader.hpp"
#include "engine/core/EngineContext.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace {

    std::filesystem::path writeTemp(const std::string& name, const std::string& body) {
        auto path = std::filesystem::temp_directory_path() / name;
        std::ofstream out(path);
        out << body;
        out.close();
        return path;
    }

} // namespace

TEST_CASE("readMapJsonFile rejects a missing file and invalid JSON") {
    nlohmann::json root;
    std::string error;
    CHECK_FALSE(engine::readMapJsonFile("/tmp/does-not-exist-bajabongo-map.json", root, &error));
    CHECK(error.find("Failed to open map file") != std::string::npos);

    auto bad = writeTemp("baja_map_bad.json", "{ not json");
    CHECK_FALSE(engine::readMapJsonFile(bad.string(), root, &error));
    CHECK_FALSE(error.empty());
}

TEST_CASE("readMapJsonFile and isTiledMapJson accept a Tiled-style map object") {
    auto path = writeTemp("baja_map_ok.json", R"({
        "type": "map",
        "mapScale": 1.8,
        "width": 32,
        "height": 32,
        "layers": [
            { "type": "imagelayer", "name": "Image Layer 1", "image": "../textures/map_hideout.png" },
            { "type": "objectgroup", "name": "Entities", "objects": [] }
        ]
    })");

    nlohmann::json root;
    std::string error;
    REQUIRE(engine::readMapJsonFile(path.string(), root, &error));
    CHECK(error.empty());
    CHECK(engine::isTiledMapJson(root));
    CHECK(root["mapScale"] == 1.8);
    CHECK(root["layers"].size() == 2);
    CHECK(root["layers"][0]["type"] == "imagelayer");
}

TEST_CASE("isTiledMapJson rejects a non-object root and a non-array layers field") {
    CHECK_FALSE(engine::isTiledMapJson(nlohmann::json::array({1, 2, 3})));
    CHECK_FALSE(engine::isTiledMapJson(nlohmann::json{{"layers", 5}}));
    CHECK(engine::isTiledMapJson(nlohmann::json::object()));
    CHECK(engine::isTiledMapJson(nlohmann::json{{"layers", nlohmann::json::array()}}));
}
