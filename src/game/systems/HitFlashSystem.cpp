#include "engine/pch.h"
#include "HitFlashSystem.hpp"
#include "game/components/HitFlashComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include <cstdint>

namespace game {

    HitFlashSystem::HitFlashSystem(engine::EngineContext& context)
        : m_context(context)
    {
    }

    void HitFlashSystem::update(float dt) {
        if (!m_context.m_registry) return;

        auto view = m_context.m_registry->view<HitFlashComponent, engine::RenderableComponent>();

        view.each([dt](HitFlashComponent& flash, engine::RenderableComponent& renderable) {
            if (!flash.isFlashing) return;

            flash.currentTimer += dt;

            // Phase 1: Hold
            if (flash.currentTimer < flash.flashHoldDuration) {
                renderable.color = flash.flashColor;
            } 
            // Phase 2: Lerp
            else {
                float lerpDuration = flash.flashDuration - flash.flashHoldDuration;
                if (lerpDuration <= 0.0f) lerpDuration = 0.001f;

                float t = (flash.currentTimer - flash.flashHoldDuration) / lerpDuration;

                if (t >= 1.0f) {
                    flash.isFlashing = false;
                    renderable.color = flash.originalColor;
                } else {
                    // Linear Interpolation
                    auto lerpChannel = [](std::uint8_t start, std::uint8_t end, float t) -> std::uint8_t {
                        float val = static_cast<float>(start) + (static_cast<float>(end) - static_cast<float>(start)) * t;
                        if (val < 0.0f) val = 0.0f;
                        if (val > 255.0f) val = 255.0f;
                        return static_cast<std::uint8_t>(val);
                    };

                    renderable.color.r = lerpChannel(flash.flashColor.r, flash.originalColor.r, t);
                    renderable.color.g = lerpChannel(flash.flashColor.g, flash.originalColor.g, t);
                    renderable.color.b = lerpChannel(flash.flashColor.b, flash.originalColor.b, t);
                    renderable.color.a = lerpChannel(flash.flashColor.a, flash.originalColor.a, t);
                }
            }
        });
    }

}
