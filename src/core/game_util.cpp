// SPDX-FileCopyrightText: Copyright 2025 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "game_util.h"
#include "common/logging/log.h"
#include <iostream>

namespace Core::GameUtil {

std::filesystem::path ResolveGameFolder(const std::filesystem::path& file,
                                       std::optional<std::filesystem::path> provided_folder) {
    if (provided_folder.has_value()) {
        return provided_folder.value();
    }

    auto game_folder = file.parent_path();
    const auto game_folder_name = game_folder.filename().string();

    // Check for update/patch directories
    if (game_folder_name.ends_with("-UPDATE") || game_folder_name.ends_with("-patch")) {
        const std::string base_name = game_folder_name.substr(0, game_folder_name.rfind('-'));
        const auto base_path = game_folder.parent_path() / base_name;
        if (std::filesystem::is_directory(base_path)) {
            return base_path;
        }
    }

    return game_folder;
}

bool EnsureDirectory(const std::filesystem::path& path, const std::string& context) {
    std::error_code ec;
    if (std::filesystem::exists(path, ec)) {
        return true;
    }

    if (!std::filesystem::create_directories(path, ec)) {
        if (context.empty()) {
            LOG_ERROR(Common, "Failed to create directory: {}", path.string());
        } else {
            LOG_ERROR(Common, "Failed to create {} directory: {}", context, path.string());
        }
        return false;
    }

    return true;
}

std::optional<std::filesystem::path> FindFileIfExists(const std::filesystem::path& path) {
    std::error_code ec;
    if (std::filesystem::exists(path, ec)) {
        return path;
    }
    return std::nullopt;
}

std::optional<bool> ParseFullscreenParam(const std::string& param) {
    if (param == "true") {
        return true;
    } else if (param == "false") {
        return false;
    }
    return std::nullopt;
}

} // namespace Core::GameUtil
