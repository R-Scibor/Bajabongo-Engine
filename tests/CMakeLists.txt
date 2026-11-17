# Unit tests for engine-pure logic (no window, no game audio playback).

add_executable(baja_tests
    test_wrap_angle.cpp
    test_archetypes.cpp
    test_asset_manifest.cpp
    test_animation_state_machine.cpp
    test_weapon_ammo.cpp
    test_health.cpp
    test_state_manager.cpp
    test_map_json.cpp
    test_rect_aabb.cpp
    test_vector2.cpp
    test_geometry.cpp
    test_log_config.cpp
    test_math_json.cpp
    test_damage_collision.cpp
    test_visibility_math.cpp
    test_enemy_detection.cpp
    test_box2d_headless.cpp
    test_entity_factory.cpp
    test_map_loader.cpp
    test_map_integration.cpp
    test_json_robustness.cpp
    test_headless_simulation.cpp
    ../src/game/systems/DamageSystem.cpp
    ../src/game/systems/VisibilitySystem.cpp
    ../src/game/ai/EnemyDetectionSystem.cpp
    ../src/game/ai/AIUtils.cpp
)

bajabongo_apply_compiler_options(baja_tests)

target_link_libraries(baja_tests
    PRIVATE
        bajabongo::engine
        Catch2::Catch2WithMain
)

target_compile_definitions(baja_tests
    PRIVATE
        BAJA_ASSETS_DIR="${PROJECT_SOURCE_DIR}/assets"
)


include(Catch)
catch_discover_tests(baja_tests)

option(BAJA_BUILD_BENCH "Build the optional headless gameplay benchmark (baja_bench)" OFF)
if(BAJA_BUILD_BENCH)
    add_executable(baja_bench bench_headless.cpp
        ../src/game/systems/DamageSystem.cpp
        ../src/game/systems/VisibilitySystem.cpp
        ../src/game/ai/EnemyDetectionSystem.cpp
        ../src/game/ai/AIUtils.cpp
    )
    bajabongo_apply_compiler_options(baja_bench)
    target_link_libraries(baja_bench
        PRIVATE
            bajabongo::engine
    )
    target_compile_definitions(baja_bench
        PRIVATE
            BAJA_ASSETS_DIR="${PROJECT_SOURCE_DIR}/assets"
    )
endif()
