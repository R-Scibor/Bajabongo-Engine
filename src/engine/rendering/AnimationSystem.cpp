#include "AnimationSystem.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/rendering/AnimationClip.hpp"
#include "engine/components/AnimationComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include <entt/entt.hpp>
#include <unordered_set>

namespace engine {

    AnimationSystem::AnimationSystem(EngineContext& context)
        : m_context(context) {
        if (context.m_logManager) {
            m_logger = context.m_logManager->GetLogger("AnimationSystem");
        }
    }

    void AnimationSystem::update(float deltaTime) {
        if (!m_context.m_animationLibrary) return;

        // We iterate only over entities that have BOTH an animation state and a renderable sprite.
        auto view = m_context.m_registry->view<AnimationComponent, RenderableComponent>();

        view.each([&](AnimationComponent& anim, RenderableComponent& renderable) {
            if (!anim.isPlaying || anim.isFinished) return;

            // Retrieve the shared clip data
            const auto* clip = m_context.m_animationLibrary->getClip(anim.currentClipId);
            if (!clip) {
                // Log missing clip only once per ID to avoid flooding the console
                static std::unordered_set<std::string> missingClips;
                if (missingClips.find(anim.currentClipId) == missingClips.end()) {
                    if (m_logger) {
                        m_logger->warn("Missing animation clip: {}", anim.currentClipId);
                    }
                    missingClips.insert(anim.currentClipId);
                }
                return;
            }

            // 1. Advance the timer
            anim.timer += deltaTime;

            // 2. Advance frames based on duration
            // Using a while loop handles cases where deltaTime > frameDuration (lag spikes)
            while (anim.timer >= clip->frameDuration) {
                anim.timer -= clip->frameDuration;
                anim.currentFrameIndex++;

                // 3. Handle End of Clip
                if (anim.currentFrameIndex >= clip->spriteIds.size()) {
                    if (clip->loop) {
                        anim.currentFrameIndex = 0; // Loop back to start
                    } else {
                        // Clamp to last frame and mark as finished
                        anim.currentFrameIndex = static_cast<int>(clip->spriteIds.size()) - 1;
                        anim.isFinished = true;
                        break; 
                    }
                }
            }

            // 4. Update the RenderableComponent (Visual representation)
            if (anim.currentFrameIndex < clip->spriteIds.size()) {
                renderable.spriteId = clip->spriteIds[anim.currentFrameIndex];
            }
        });
    }

} // namespace engine