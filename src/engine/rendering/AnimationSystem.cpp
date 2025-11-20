#include "AnimationSystem.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/rendering/AnimationClip.hpp"
#include "engine/components/AnimationComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include <entt/entt.hpp>

namespace engine {

    AnimationSystem::AnimationSystem(EngineContext& context)
        : m_context(context) {}

    void AnimationSystem::update(float deltaTime) {
        if (!m_context.m_animationLibrary) return;

        auto view = m_context.m_registry->view<AnimationComponent, RenderableComponent>();

        for (auto entity : view) {
            auto& anim = view.get<AnimationComponent>(entity);
            auto& renderable = view.get<RenderableComponent>(entity);

            if (!anim.isPlaying || anim.isFinished) continue;

            const auto* clip = m_context.m_animationLibrary->getClip(anim.currentClipId);
            if (!clip) continue;

            // Update timer
            anim.timer += deltaTime;

            // Check for frame change
            if (anim.timer >= clip->frameDuration) {
                anim.timer -= clip->frameDuration;
                anim.currentFrameIndex++;

                // Handle looping or finishing
                if (anim.currentFrameIndex >= clip->spriteIds.size()) {
                    if (clip->loop) {
                        anim.currentFrameIndex = 0;
                    } else {
                        anim.currentFrameIndex = static_cast<int>(clip->spriteIds.size()) - 1;
                        anim.isFinished = true;
                    }
                }
            }

            // Sync sprite ID to RenderableComponent
            if (anim.currentFrameIndex < clip->spriteIds.size()) {
                renderable.spriteId = clip->spriteIds[anim.currentFrameIndex];
            }
        }
    }

} // namespace engine
