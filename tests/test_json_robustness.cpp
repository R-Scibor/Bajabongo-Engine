#include "HeadlessScene.hpp"
#include "engine/core/IResourceManager.hpp"
#include "engine/assets/AssetManifestLoader.hpp"
#include "engine/assets/MapLoader.hpp"
#include "engine/ecs/ArchetypeManager.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "engine/rendering/AnimationClip.hpp"
#include "engine/rendering/Sprite.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace {

    class FakeResources final : public engine::IResourceManager {
    public:
        std::shared_ptr<engine::TextureHandle> loadTexture(const std::string&, const std::string&) override {
            return nullptr;
        }
        std::shared_ptr<engine::TextureHandle> getTexture(const std::string&) const override { return nullptr; }
        std::shared_ptr<engine::SoundBufferHandle> loadSoundBuffer(const std::string&, const std::string&) override {
            return nullptr;
        }
        std::shared_ptr<engine::SoundBufferHandle> getSoundBuffer(const std::string&) const override { return nullptr; }
        void registerMusicPath(const std::string&, const std::string&) override {}
        std::string getMusicPath(const std::string&) const override { return {}; }
    };

    std::filesystem::path writeTemp(const std::string& body) {
        static std::atomic<int> counter{0};
        auto path = std::filesystem::temp_directory_path() / ("baja_fuzz_" + std::to_string(++counter) + ".json");
        std::ofstream out(path, std::ios::binary);
        out << body;
        out.close();
        return path;
    }

    const std::vector<std::pair<const char*, std::string>> kBrokenJson = {
        {"empty file", ""},
        {"truncated object", "{\"layers\":"},
        {"truncated string", "{\"name\": \"no"},
        {"bare identifier", "not-json"},
        {"null root", "null"},
        {"array root", "[1, 2, 3]"},
        {"number root", "42"},
        {"string root", "\"map\""},
        {"true root", "true"},
        {"layers is a number", "{\"layers\": 1}"},
        {"layers is a string", "{\"layers\": \"Collision\"}"},
        {"layers is an object", "{\"layers\": {\"type\": \"objectgroup\"}}"},
        {"layers is null", "{\"layers\": null}"},
        {"mapScale is a string", "{\"mapScale\": \"2.0\", \"layers\": []}"},
        {"mapScale is an array", "{\"mapScale\": [2], \"layers\": []}"},
        {"layer entry is a string", "{\"layers\": [\"imagelayer\"]}"},
        {"layer entry is null", "{\"layers\": [null]}"},
        {"layer entry is a number", "{\"layers\": [1]}"},
        {"objects is a string",
         "{\"layers\": [{\"type\": \"objectgroup\", \"name\": \"Collision\", \"objects\": \"x\"}]}"},
        {"objects is null", "{\"layers\": [{\"type\": \"objectgroup\", \"name\": \"Collision\", \"objects\": null}]}"},
        {"object entry is a number",
         "{\"layers\": [{\"type\": \"objectgroup\", \"name\": \"Collision\", \"objects\": [1]}]}"},
        {"object entry is a string",
         "{\"layers\": [{\"type\": \"objectgroup\", \"name\": \"Entities\", \"objects\": [\"player\"]}]}"},
        {"image field is a number", "{\"layers\": [{\"type\": \"imagelayer\", \"visible\": true, \"image\": 3, "
                                    "\"imagewidth\": 10, \"imageheight\": 10}]}"},
        {"width is a string", "{\"layers\": [{\"type\": \"objectgroup\", \"name\": \"Collision\", \"objects\": "
                              "[{\"x\": 0, \"y\": 0, \"width\": \"wide\", \"height\": 4}]}]}"},
        {"UTF-8 garbage", "\x80\x81\x82{\"layers\":[]}"},
        {"only whitespace", "   \n\t  "},
        {"trailing comma", "{\"layers\": [],}"},
    };

} // namespace

TEST_CASE("MapLoader does not throw on corrupted map JSON") {
    baja::test::HeadlessScene scene;
    auto archetypes = std::make_shared<engine::ArchetypeManager>(scene.logManager);
    auto factory = std::make_shared<engine::EntityFactory>(scene.context, archetypes);
    scene.context.m_entityFactory = factory;
    engine::MapLoader loader(scene.context);

    const std::vector<std::string> skippedButValid = {
        "layer entry is a string", "layer entry is null",      "layer entry is a number",  "objects is a string",
        "objects is null",         "object entry is a number", "object entry is a string",
    };

    for (const auto& [label, body] : kBrokenJson) {
        INFO(label);
        auto path = writeTemp(body);
        bool loaded = true;
        REQUIRE_NOTHROW(loaded = loader.load(path.string()));
        const bool expectSkip =
            std::find(skippedButValid.begin(), skippedButValid.end(), label) != skippedButValid.end();
        if (!expectSkip) {
            CHECK_FALSE(loaded);
        }
    }
}

TEST_CASE("AssetManifestLoader does not throw on corrupted resources JSON") {
    auto resources = std::make_shared<FakeResources>();
    auto sprites = std::make_shared<engine::SpriteManager>();
    auto clips = std::make_shared<engine::AnimationLibrary>();
    auto logger = std::make_shared<baja::test::NullLogger>();
    engine::AssetManifestLoader loader(resources, sprites, clips, logger);

    const std::vector<std::pair<const char*, std::string>> cases = {
        {"empty file", ""},
        {"truncated", "{\"textures\":"},
        {"null root", "null"},
        {"array root", "[]"},
        {"textures is an object", "{\"textures\": {\"id\": \"x\"}}"},
        {"textures is a string", "{\"textures\": \"nope\"}"},
        {"texture entry is a string", "{\"textures\": [\"oops\"]}"},
        {"texture entry is a number", "{\"textures\": [1]}"},
        {"id is a number", "{\"textures\": [{\"id\": 1, \"path\": \"a.png\"}]}"},
        {"path is an array", "{\"textures\": [{\"id\": \"tex\", \"path\": [1, 2]}]}"},
        {"sprites is not an array",
         "{\"textures\": [{\"id\": \"tex\", \"path\": \"a.png\", \"sprites\": {\"id\": \"s\"}}]}"},
        {"sprite entry is a number", "{\"textures\": [{\"id\": \"tex\", \"path\": \"a.png\", \"sprites\": [1]}]}"},
        {"region values are strings", "{\"textures\": [{\"id\": \"tex\", \"path\": \"a.png\", \"sprites\": [{\"id\": "
                                      "\"s\", \"region\": [\"a\", \"b\", \"c\", \"d\"], \"origin\": [1, 2]}]}]}"},
        {"sounds is an object", "{\"sounds\": {\"id\": \"s\"}}"},
        {"sound entry is a string", "{\"sounds\": [\"nope\"]}"},
        {"music is null", "{\"music\": null}"},
        {"UTF-8 garbage", "\xff\xfe{"},
    };

    const std::vector<std::string> mustFail = {
        "empty file", "truncated", "null root", "array root", "UTF-8 garbage",
    };

    for (const auto& [label, body] : cases) {
        INFO(label);
        auto path = writeTemp(body);
        bool loaded = true;
        REQUIRE_NOTHROW(loaded = loader.load(path.string()));
        if (std::find(mustFail.begin(), mustFail.end(), label) != mustFail.end()) {
            CHECK_FALSE(loaded);
        }
    }
}

TEST_CASE("ArchetypeManager does not throw on corrupted archetype JSON") {
    engine::ArchetypeManager manager(std::make_shared<baja::test::NullLoggerManager>());
    for (const auto* body : {"", "{", "null", "[]", "1", "\"x\"", "{\"unit\":", "\x80\x81"}) {
        INFO(body);
        auto path = writeTemp(body);
        bool loaded = true;
        REQUIRE_NOTHROW(loaded = manager.loadArchetypes(path.string()));
        CHECK_FALSE(loaded);
    }
}

TEST_CASE("EntityFactory does not throw when a child archetype is missing") {
    auto path = writeTemp(R"({
        "parent": {
            "children": [ { "archetype": "missing_child" } ]
        }
    })");
    baja::test::HeadlessScene scene;
    auto archetypes = std::make_shared<engine::ArchetypeManager>(scene.logManager);
    REQUIRE(archetypes->loadArchetypes(path.string()));
    auto factory = std::make_shared<engine::EntityFactory>(scene.context, archetypes);
    scene.context.m_entityFactory = factory;

    entt::entity entity = entt::null;
    REQUIRE_NOTHROW(entity = factory->spawn("parent", {0.0f, 0.0f}));
    CHECK(entity != entt::null);
}

TEST_CASE("EntityFactory does not throw on mistyped component fields") {
    auto path = writeTemp(R"({
        "odd": {
            "Transform": { "rotation": "left", "scale": 2 },
            "Physics": { "type": 1, "size": "big", "fixtures": "none", "category": 4 },
            "Renderable": { "spriteId": 3, "layer": "front", "color": "red" }
        }
    })");
    baja::test::HeadlessScene scene;
    auto archetypes = std::make_shared<engine::ArchetypeManager>(scene.logManager);
    REQUIRE(archetypes->loadArchetypes(path.string()));
    auto factory = std::make_shared<engine::EntityFactory>(scene.context, archetypes);
    entt::entity entity = entt::null;
    REQUIRE_NOTHROW(entity = factory->spawn("odd", {1.0f, 1.0f}));
    CHECK(entity != entt::null);
}
