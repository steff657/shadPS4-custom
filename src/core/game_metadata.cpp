// SPDX-FileCopyrightText: Copyright 2025 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "game_metadata.h"
#include "common/config.h"
#include "common/logging/log.h"
#include "core/file_format/psf.h"
#include <hwinfo/hwinfo.h>
#include <fmt/core.h>

namespace Core {

std::optional<PSFData> LoadPSFData(const std::filesystem::path& param_sfo_path) {
    if (!std::filesystem::exists(param_sfo_path)) {
        return std::nullopt;
    }

    PSFData data;
    auto* param_sfo = Common::Singleton<PSF>::Instance();
    
    if (!param_sfo->Open(param_sfo_path)) {
        LOG_ERROR(Loader, "Failed to open param.sfo");
        return std::nullopt;
    }

    // Extract game ID
    const auto content_id = param_sfo->GetString("CONTENT_ID");
    const auto title_id = param_sfo->GetString("TITLE_ID");
    if (content_id.has_value() && !content_id->empty()) {
        data.id = std::string(*content_id, 7, 9);
    } else if (title_id.has_value()) {
        data.id = *title_id;
    }

    // Extract other metadata
    data.title = param_sfo->GetString("TITLE").value_or("Unknown title");
    data.fw_version = param_sfo->GetInteger("SYSTEM_VER").value_or(0x4700000);
    data.app_version = param_sfo->GetString("APP_VER").value_or("Unknown version");

    // Parse SDK version from PUBTOOLINFO
    std::string_view pubtool_info =
        param_sfo->GetString("PUBTOOLINFO").value_or("Unknown value");
    u64 sdk_ver_offset = pubtool_info.find("sdk_ver");

    if (sdk_ver_offset != pubtool_info.npos) {
        sdk_ver_offset += 8;  // Skip "sdk_ver="
        u64 sdk_ver_len = pubtool_info.find(",", sdk_ver_offset);
        if (sdk_ver_len == pubtool_info.npos) {
            sdk_ver_len = pubtool_info.size();
        }
        sdk_ver_len -= sdk_ver_offset;
        std::string sdk_ver_string = pubtool_info.substr(sdk_ver_offset, sdk_ver_len).data();
        data.sdk_version = std::stoi(sdk_ver_string, nullptr, 16);
    } else {
        data.sdk_version = data.fw_version;
    }

    // Parse attributes
    if (const auto raw_attributes = param_sfo->GetInteger("ATTRIBUTE")) {
        Common::PSFAttributes attributes{};
        attributes.raw = *raw_attributes;
        data.is_psvr_supported = attributes.support_ps_vr.Value();
        data.is_psvr_required = attributes.require_ps_vr.Value();
    }

    return data;
}

void LogConfiguration() {
    LOG_INFO(Config, "General LogType: {}", Config::getLogType());
    LOG_INFO(Config, "General isNeo: {}", Config::isNeoModeConsole());
    LOG_INFO(Config, "General isDevKit: {}", Config::isDevKitConsole());
    LOG_INFO(Config, "General isConnectedToNetwork: {}", Config::getIsConnectedToNetwork());
    LOG_INFO(Config, "General isPsnSignedIn: {}", Config::getPSNSignedIn());

    LOG_INFO(Config, "GPU isNullGpu: {}", Config::nullGpu());
    LOG_INFO(Config, "GPU readbacks: {}", Config::readbacks());
    LOG_INFO(Config, "GPU readbackLinearImages: {}", Config::readbackLinearImages());
    LOG_INFO(Config, "GPU directMemoryAccess: {}", Config::directMemoryAccess());
    LOG_INFO(Config, "GPU shouldDumpShaders: {}", Config::dumpShaders());
    LOG_INFO(Config, "GPU vblankFrequency: {}", Config::vblankFreq());
    LOG_INFO(Config, "GPU shouldCopyGPUBuffers: {}", Config::copyGPUCmdBuffers());

    LOG_INFO(Config, "Vulkan gpuId: {}", Config::getGpuId());
    LOG_INFO(Config, "Vulkan vkValidation: {}", Config::vkValidationEnabled());
    LOG_INFO(Config, "Vulkan vkValidationCore: {}", Config::vkValidationCoreEnabled());
    LOG_INFO(Config, "Vulkan vkValidationSync: {}", Config::vkValidationSyncEnabled());
    LOG_INFO(Config, "Vulkan vkValidationGpu: {}", Config::vkValidationGpuEnabled());
    LOG_INFO(Config, "Vulkan crashDiagnostics: {}", Config::getVkCrashDiagnosticEnabled());
    LOG_INFO(Config, "Vulkan hostMarkers: {}", Config::getVkHostMarkersEnabled());
    LOG_INFO(Config, "Vulkan guestMarkers: {}", Config::getVkGuestMarkersEnabled());
    LOG_INFO(Config, "Vulkan rdocEnable: {}", Config::isRdocEnabled());

    // Log system information
    hwinfo::Memory ram;
    hwinfo::OS os;
    const auto cpus = hwinfo::getAllCPUs();
    for (const auto& cpu : cpus) {
        LOG_INFO(Config, "CPU Model: {}", cpu.modelName());
        LOG_INFO(Config, "CPU Physical Cores: {}, Logical Cores: {}", cpu.numPhysicalCores(),
                 cpu.numLogicalCores());
    }
    LOG_INFO(Config, "Total RAM: {} GB", std::round(ram.total_Bytes() / pow(1024, 3)));
    LOG_INFO(Config, "Operating System: {}", os.name());
}

void LogGameMetadata(const std::string& id, const std::string& title, uint32_t fw_version,
                     const std::string& app_version, uint32_t sdk_version, bool psvr_supported,
                     bool psvr_required) {
    LOG_INFO(Loader, "Game id: {} Title: {}", id, title);
    LOG_INFO(Loader, "Fw: {:#x} App Version: {}", fw_version, app_version);
    LOG_INFO(Loader, "param.sfo SDK version: {:#x}", sdk_version);
    LOG_INFO(Loader, "PSVR Supported: {}", psvr_supported);
    LOG_INFO(Loader, "PSVR Required: {}", psvr_required);
}

void LogGameArguments(const std::vector<std::string>& args) {
    if (args.empty()) {
        return;
    }

    constexpr auto max_logged_args = 32;
    const auto argc = std::min<size_t>(args.size(), max_logged_args);
    
    for (size_t i = 0; i < argc; ++i) {
        LOG_INFO(Loader, "Game argument {}: {}", i, args[i]);
    }
    
    if (args.size() > max_logged_args) {
        LOG_ERROR(Loader, "Too many game arguments, only passing the first {}", max_logged_args);
    }
}

} // namespace Core
