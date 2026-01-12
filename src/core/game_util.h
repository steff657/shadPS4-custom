// SPDX-FileCopyrightText: Copyright 2025 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <cstdint>

namespace Core {

/**
 * @brief Game initialization utilities
 * 
 * Helper functions for initializing game-related data structures
 * and validating game parameters.
 */
namespace GameUtil {

/**
 * @brief Information about a mounted game
 */
struct GameInfo {
    std::string id;
    std::string title;
    std::string app_version;
    uint32_t sdk_version;
    uint32_t fw_version;
    std::filesystem::path game_folder;
};

/**
 * @brief Resolve the game folder from multiple sources
 * 
 * @param file Path to the eboot executable
 * @param provided_folder Optional folder provided by user
 * @return Resolved game folder path
 */
std::filesystem::path ResolveGameFolder(const std::filesystem::path& file,
                                       std::optional<std::filesystem::path> provided_folder);

/**
 * @brief Ensure directory exists, creating it if necessary
 * 
 * @param path Directory path
 * @param context Context string for logging
 * @return true if directory exists or was created successfully
 */
bool EnsureDirectory(const std::filesystem::path& path, const std::string& context = "");

/**
 * @brief Safely load a file, returning empty optional if not found
 * 
 * @param path File path to load
 * @return File path if exists, empty optional otherwise
 */
std::optional<std::filesystem::path> FindFileIfExists(const std::filesystem::path& path);

/**
 * @brief Validate and convert fullscreen parameter
 * 
 * @param param String parameter ("true" or "false")
 * @return Converted boolean, or nullopt if invalid
 */
std::optional<bool> ParseFullscreenParam(const std::string& param);

} // namespace GameUtil

} // namespace Core
