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

        auto view = m_context.m_registry->view<AnimationComponent, RenderableComponent>();

        view.each([&](AnimationComponent& anim, RenderableComponent& renderable) {
            if (!anim.isPlaying || anim.isFinished) return;

            const auto* clip = m_context.m_animationLibrary->getClip(anim.currentClipId);
            if (!clip) {
                static std::unordered_set<std::string> missingClips;
                if (missingClips.find(anim.currentClipId) == missingClips.end()) {
                    if (m_logger) {
                        m_logger->warn("Missing animation clip: {}", anim.currentClipId);
                    }
                    missingClips.insert(anim.currentClipId);
                }
                return;
            }

            // Update timer
            anim.timer += deltaTime;

            // Check for frame change
            while (anim.timer >= clip->frameDuration) {
                anim.timer -= clip->frameDuration;
                anim.currentFrameIndex++;

                // Handle looping or finishing
                if (anim.currentFrameIndex >= clip->spriteIds.size()) {
                    if (clip->loop) {
                        anim.currentFrameIndex = 0;
                    } else {
                        anim.currentFrameIndex = static_cast<int>(clip->spriteIds.size()) - 1;
                        anim.isFinished = true;
                        break; // Stop processing if finished
                    }
                }
            }

            // Sync sprite ID to RenderableComponent
            if (anim.currentFrameIndex < clip->spriteIds.size()) {
                renderable.spriteId = clip->spriteIds[anim.currentFrameIndex];
            }
        });
    }

} // namespace engine
