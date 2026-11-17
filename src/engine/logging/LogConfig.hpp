#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <spdlog/common.h>

namespace engine {
    /**
     * @brief Removes leading and trailing spaces and tabs.
     */
    inline std::string trim(const std::string& str) {
        const std::string whitespace = " \t";
        const auto strBegin = str.find_first_not_of(whitespace);
        if (strBegin == std::string::npos) return ""; // no content

        const auto strEnd = str.find_last_not_of(whitespace);
        const auto strRange = strEnd - strBegin + 1;

        return str.substr(strBegin, strRange);
    }

    /**
     * @brief Parses one logger.ini line of the form Name=Level.
     *
     * Empty lines and lines whose first character is '#' or ';' are skipped.
     * Returns false when the line should be ignored; otherwise writes the
     * trimmed name and level and returns true.
     */
    inline bool parseLogConfigLine(const std::string& line, std::string& name, std::string& level) {
        if (line.empty() || line[0] == '#' || line[0] == ';') return false;

        const auto delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) return false;

        name = trim(line.substr(0, delimiterPos));
        level = trim(line.substr(delimiterPos + 1));
        return true;
    }

    /**
     * @brief Converts a string log level to a spdlog enum. Case-insensitive.
     * Unknown values fall back to info, matching the historic loader.
     */
    inline spdlog::level::level_enum levelFromString(const std::string& level_str) {
        std::string lower_level = level_str;
        std::transform(lower_level.begin(), lower_level.end(), lower_level.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        if (lower_level == "trace") return spdlog::level::trace;
        if (lower_level == "debug") return spdlog::level::debug;
        if (lower_level == "warn") return spdlog::level::warn;
        if (lower_level == "error") return spdlog::level::err;
        if (lower_level == "critical") return spdlog::level::critical;

        return spdlog::level::info;
    }
} // namespace engine
