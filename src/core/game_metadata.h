// SPDX-FileCopyrightText: Copyright 2025 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <cstdint>
#include "common/types.h"

namespace Core {

/**
 * @brief PSF (Parameter SFO) data and metadata
 */
struct PSFData {
    std::string id;
    std::string title;
    std::string app_version;
    uint32_t fw_version = 0;
    uint32_t sdk_version = 0;
    std::filesystem::path splash_path;

    bool is_psvr_supported = false;
    bool is_psvr_required = false;
};

/**
 * @brief Loads PSF metadata from param.sfo file
 * 
 * @param param_sfo_path Path to param.sfo file
 * @return PSFData structure, or empty optional if load fails
 */
std::optional<PSFData> LoadPSFData(const std::filesystem::path& param_sfo_path);

/**
 * @brief Logs all loaded configuration options
 * 
 * Helper to consolidate configuration logging in one place
 */
void LogConfiguration();

/**
 * @brief Logs game metadata information
 * 
 * @param id Game ID
 * @param title Game title
 * @param fw_version Firmware version
 * @param app_version Application version
 * @param sdk_version SDK version
 * @param psvr_supported PSVR support flag
 * @param psvr_required PSVR requirement flag
 */
void LogGameMetadata(const std::string& id, const std::string& title, uint32_t fw_version,
                     const std::string& app_version, uint32_t sdk_version, bool psvr_supported,
                     bool psvr_required);

/**
 * @brief Logs game arguments
 * 
 * @param args Vector of arguments
 */
void LogGameArguments(const std::vector<std::string>& args);

} // namespace Core
