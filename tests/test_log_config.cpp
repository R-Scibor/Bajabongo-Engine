#include "engine/logging/LogConfig.hpp"

#include <catch2/catch_test_macros.hpp>
#include <spdlog/common.h>

TEST_CASE("trim removes leading and trailing spaces and tabs") {
    CHECK(engine::trim("  hello  ") == "hello");
    CHECK(engine::trim("\tfoo\t") == "foo");
    CHECK(engine::trim("already") == "already");
    CHECK(engine::trim("   ") == "");
    CHECK(engine::trim("") == "");
}

TEST_CASE("parseLogConfigLine skips comments and empty lines") {
    std::string name;
    std::string level;
    CHECK_FALSE(engine::parseLogConfigLine("", name, level));
    CHECK_FALSE(engine::parseLogConfigLine("# Physics=debug", name, level));
    CHECK_FALSE(engine::parseLogConfigLine("; ignored", name, level));
    CHECK_FALSE(engine::parseLogConfigLine("no delimiter here", name, level));
}

TEST_CASE("parseLogConfigLine reads a trimmed name and level") {
    std::string name;
    std::string level;
    REQUIRE(engine::parseLogConfigLine("  Physics = Debug  ", name, level));
    CHECK(name == "Physics");
    CHECK(level == "Debug");
}

TEST_CASE("levelFromString is case-insensitive and falls back to info") {
    CHECK(engine::levelFromString("trace") == spdlog::level::trace);
    CHECK(engine::levelFromString("DEBUG") == spdlog::level::debug);
    CHECK(engine::levelFromString("Warn") == spdlog::level::warn);
    CHECK(engine::levelFromString("error") == spdlog::level::err);
    CHECK(engine::levelFromString("CRITICAL") == spdlog::level::critical);
    CHECK(engine::levelFromString("info") == spdlog::level::info);
    CHECK(engine::levelFromString("nope") == spdlog::level::info);
    CHECK(engine::levelFromString("") == spdlog::level::info);
}
