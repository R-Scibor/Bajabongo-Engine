# Mirrors src/game/game.vcxproj (Application, Console subsystem, 23 ClCompile
# sources, ProjectReference to engine). No post-build events exist in the
# vcxproj; asset layout is handled later by cmake/CopyRuntimeAssets.cmake.

set(GAME_SOURCES
    ai/AIUtils.cpp
    ai/EnemyCombatSystem.cpp
    ai/EnemyDetectionSystem.cpp
    ai/EnemyMovementSystem.cpp
    ai/EnemyStateTransitions.cpp
    ai/EnemyStuckRecoverySystem.cpp
    game.cpp
    setup/GameComponentRegistry.cpp
    states/GameplayState.cpp
    states/MainMenuState.cpp
    states/MapSelectionState.cpp
    states/SplashScreenState.cpp
    systems/DamageSystem.cpp
    systems/EnemyAISystem.cpp
    systems/FogRenderSystem.cpp
    systems/HitFlashSystem.cpp
    systems/HudSystem.cpp
    systems/PickupSystem.cpp
    systems/PlayerControllerSystem.cpp
    systems/VisibilitySystem.cpp
    systems/VisibilityTaggingSystem.cpp
    systems/WeaponSwitchSystem.cpp
    systems/WeaponSystem.cpp
)

add_executable(game ${GAME_SOURCES})
add_executable(bajabongo::game ALIAS game)

bajabongo_apply_compiler_options(game)

# _CONSOLE is defined in every game.vcxproj configuration.
# SubSystem is Console (not Windows).
target_compile_definitions(game PRIVATE _CONSOLE)
if(MSVC)
    target_link_options(game PRIVATE /SUBSYSTEM:CONSOLE)
endif()

target_link_libraries(game PRIVATE bajabongo::engine)
