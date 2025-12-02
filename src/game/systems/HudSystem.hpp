#pragma once

namespace engine {
    struct EngineContext;
}

namespace game {

    class HudSystem {
    public:
        explicit HudSystem(engine::EngineContext& context);

        void render();

    private:
        engine::EngineContext& m_context;
    };

} // namespace game