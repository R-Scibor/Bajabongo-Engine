#pragma once

#include "engine/core/math/Rect.hpp"
#include "engine/core/math/Vector2.hpp"
#include <nlohmann/json.hpp>

namespace engine {
    template<typename T>
    void to_json(nlohmann::json& j, const Vector2<T>& v) {
        j = nlohmann::json{{"x", v.x}, {"y", v.y}};
    }

    template<typename T>
    void from_json(const nlohmann::json& j, Vector2<T>& v) {
        j.at("x").get_to(v.x);
        j.at("y").get_to(v.y);
    }

    template<typename T>
    void to_json(nlohmann::json& j, const Rect<T>& r) {
        j = nlohmann::json{{"left", r.left}, {"top", r.top}, {"width", r.width}, {"height", r.height}};
    }

    template<typename T>
    void from_json(const nlohmann::json& j, Rect<T>& r) {
        j.at("left").get_to(r.left);
        j.at("top").get_to(r.top);
        j.at("width").get_to(r.width);
        j.at("height").get_to(r.height);
    }
} // namespace engine
