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
)

bajabongo_apply_compiler_options(baja_tests)

target_link_libraries(baja_tests
    PRIVATE
        bajabongo::engine
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(baja_tests)
