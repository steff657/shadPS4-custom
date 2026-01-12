// SPDX-FileCopyrightText: Copyright 2025 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Common {

/**
 * @brief Argument parser for command-line interface
 * 
 * Provides a clean, maintainable way to handle command-line arguments
 * using callback functions organized in a map-based structure.
 */
class ArgParser {
public:
    using ArgCallback = std::function<void(int&)>;
    
    /**
     * @brief Structure to hold parsed game arguments
     */
    struct ParsedArgs {
        bool has_game_argument = false;
        std::string game_path;
        std::vector<std::string> game_args;
        std::optional<std::filesystem::path> game_folder;
        bool wait_for_debugger = false;
        std::optional<int> wait_pid;
    };

    ArgParser();

    /**
     * @brief Parse command-line arguments
     * 
     * @param argc Argument count
     * @param argv Argument vector
     * @return ParsedArgs structure with parsed values
     */
    ParsedArgs Parse(int argc, char* argv[]);

private:
    std::unordered_map<std::string, ArgCallback> arg_map;
    ParsedArgs result;

    void RegisterArguments();
    void PrintHelp() const;
    std::optional<std::string> ValidatePath(const std::string& path, bool is_file = false) const;
};

} // namespace Common
