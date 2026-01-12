// SPDX-FileCopyrightText: Copyright 2025-2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <SDL3/SDL_messagebox.h>
#include <iostream>
#include <string>

#include <core/emulator_state.h>
#include <fmt/core.h>

#include "common/arg_parser.h"
#include "common/config.h"
#include "common/logging/backend.h"
#include "common/memory_patcher.h"
#include "common/path_util.h"
#include "core/debugger.h"
#include "core/file_sys/fs.h"
#include "core/game_util.h"
#include "core/ipc/ipc.h"
#include "emulator.h"

#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @brief Display no-argument info dialog
 */
void ShowNoArgumentDialog() {
    if (!SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_INFORMATION, "shadPS4",
            "This is a CLI application. Please use the QTLauncher for a GUI: "
            "https://github.com/shadps4-emu/shadps4-qtlauncher/releases",
            nullptr)) {
        std::cerr << "Could not display SDL message box! Error: " << SDL_GetError() << "\n";
    }
}

/**
 * @brief Resolve game path from ID or file path
 * 
 * @param game_path Input game path or ID
 * @return Resolved path to eboot.bin
 */
std::filesystem::path ResolveGamePath(const std::string& game_path) {
    std::filesystem::path eboot_path(game_path);

    // Check if the provided path is a valid file
    if (std::filesystem::exists(eboot_path)) {
        return eboot_path;
    }

    // If not a file, treat it as a game ID and search in install directories
    constexpr int max_depth = 5;
    for (const auto& install_dir : Config::getGameInstallDirs()) {
        if (auto found_path = Common::FS::FindGameByID(install_dir, game_path, max_depth)) {
            return *found_path;
        }
    }

    // Game not found - error will be handled by caller
    return eboot_path;
}

/**
 * @brief Initialize system state and configuration
 */
void InitializeSystem() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    // Initialize IPC
    IPC::Instance().Init();

    // Initialize emulator state
    auto emulator_state = std::make_shared<EmulatorState>();
    EmulatorState::SetInstance(emulator_state);

    // Load global configuration
    const auto user_dir = Common::FS::GetUserPath(Common::FS::PathType::UserDir);
    Config::load(user_dir / "config.toml");
}

int main(int argc, char* argv[]) {
    InitializeSystem();

    // Display help dialog if no arguments provided
    if (argc == 1) {
        ShowNoArgumentDialog();
        Common::ArgParser parser;
        parser.Parse(argc, argv);  // This will exit with help
        return -1;
    }

    // Parse command-line arguments
    Common::ArgParser parser;
    auto args = parser.Parse(argc, argv);

    // Validate game argument
    if (!args.has_game_argument) {
        std::cerr << "Error: Please provide a game path or ID.\n";
        return 1;
    }

    // Check if game directories are configured
    if (Config::getGameInstallDirs().empty()) {
        std::cerr << "Warning: No game folder set. Please set it using:\n"
                  << "  shadps4 --add-game-folder <folder_name>\n";
    }

    // Resolve game path
    auto eboot_path = ResolveGamePath(args.game_path);
    if (!std::filesystem::exists(eboot_path)) {
        std::cerr << "Error: Game ID or file path not found: " << args.game_path << "\n";
        return 1;
    }

    // Wait for debugger if specified
    if (args.wait_pid.has_value()) {
        Core::Debugger::WaitForPid(args.wait_pid.value());
    }

    // Launch emulator
    auto* emulator = Common::Singleton<Core::Emulator>::Instance();
    emulator->executableName = argv[0];
    emulator->waitForDebuggerBeforeRun = args.wait_for_debugger;
    emulator->Run(eboot_path, args.game_args, args.game_folder);

    return 0;
}
