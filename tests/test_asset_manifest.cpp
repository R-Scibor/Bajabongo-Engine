#include "engine/assets/AssetManifestLoader.hpp"
#include "engine/rendering/AnimationClip.hpp"
#include "engine/rendering/Sprite.hpp"
#include "TestHelpers.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <memory>

namespace {

    std::filesystem::path writeTemp(const std::string& name, const std::string& body) {
        auto path = std::filesystem::temp_directory_path() / name;
        std::ofstream out(path);
        out << body;
        out.close();
        return path;
    }

} // namespace

TEST_CASE("AssetManifestLoader rejects a missing file and invalid JSON") {
    auto resources = std::make_shared<baja::test::FakeResourceManager>();
    auto sprites = std::make_shared<engine::SpriteManager>();
    auto clips = std::make_shared<engine::AnimationLibrary>();
    auto logger = std::make_shared<baja::test::NullLogger>();
    engine::AssetManifestLoader loader(resources, sprites, clips, logger);

    CHECK_FALSE(loader.load("/tmp/does-not-exist-bajabongo-manifest.json"));
    auto bad = writeTemp("baja_manifest_bad.json", "{ not json");
    CHECK_FALSE(loader.load(bad.string()));
}

TEST_CASE("AssetManifestLoader parses textures, sprites, animations, and audio paths") {
    auto path = writeTemp("baja_manifest_ok.json", R"({
        "textures": [
            {
                "id": "bullet_texture",
                "path": "../../assets/textures/bullet.png",
                "sprites": [
                    { "id": "bullet_sprite", "region": [0, 0, 32, 32], "origin": [16.0, 16.0] }
                ],
                "animations": [
                    {
                        "id": "bullet",
                        "frameStart": [0, 0],
                        "frameSize": [32, 32],
                        "origin": [16.0, 16.0],
                        "frameCount": 1,
                        "columns": 1,
                        "duration": 0.1,
                        "loop": true
                    }
                ]
            },
            { "path": "missing-id.png" }
        ],
        "sounds": [
            { "id": "ak_fullauto", "path": "../../assets/sounds/ak_fullauto.mp3" },
            { "id": "no-path" }
        ],
        "music": [
            { "id": "game_music_1", "path": "../../assets/music/game_music_1.mp3" }
        ]
    })");

    auto resources = std::make_shared<baja::test::FakeResourceManager>();
    auto sprites = std::make_shared<engine::SpriteManager>();
    auto clips = std::make_shared<engine::AnimationLibrary>();
    auto logger = std::make_shared<baja::test::NullLogger>();
    engine::AssetManifestLoader loader(resources, sprites, clips, logger);

    REQUIRE(loader.load(path.string()));

    REQUIRE(resources->textures.size() == 1);
    CHECK(resources->textures["bullet_texture"] == "../../assets/textures/bullet.png");
    CHECK(resources->sounds["ak_fullauto"] == "../../assets/sounds/ak_fullauto.mp3");
    CHECK(resources->sounds.find("no-path") == resources->sounds.end());
    CHECK(resources->music["game_music_1"] == "../../assets/music/game_music_1.mp3");

    const auto* sprite = sprites->getSprite("bullet_sprite");
    REQUIRE(sprite != nullptr);
    CHECK(sprite->textureId == "bullet_texture");
    CHECK(sprite->uvRect.position.x == 0);
    CHECK(sprite->uvRect.size.x == 32);
    CHECK(sprite->origin.x == 16.0f);

    const auto* frame = sprites->getSprite("bullet_0");
    REQUIRE(frame != nullptr);
    CHECK(frame->origin.x == 16.0f);

    const auto* clip = clips->getClip("bullet");
    REQUIRE(clip != nullptr);
    CHECK(clip->loop);
    CHECK(clip->frameDuration == 0.1f);
    REQUIRE(clip->spriteIds.size() == 1);
    CHECK(clip->spriteIds[0] == "bullet_0");
}
