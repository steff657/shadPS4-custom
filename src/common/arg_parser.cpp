// SPDX-FileCopyrightText: Copyright 2025 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "arg_parser.h"
#include "common/config.h"
#include "common/logging/backend.h"
#include "common/memory_patcher.h"
#include "common/path_util.h"
#include "core/debugger.h"
#include "core/file_sys/fs.h"
#include <iostream>

namespace Common {

ArgParser::ArgParser() {
    RegisterArguments();
}

void ArgParser::PrintHelp() const {
    std::cout << "Usage: shadps4 [options] <elf or eboot.bin path>\n"
              << "Options:\n"
              << "  -g, --game <path|ID>          Specify game path to launch\n"
              << "  -- ...                         Parameters passed to the game ELF. "
              << "Needs to be at the end of the line, and everything after \"--\" is a "
              << "game argument.\n"
              << "  -p, --patch <patch_file>      Apply specified patch file\n"
              << "  -i, --ignore-game-patch       Disable automatic loading of game patch\n"
              << "  -f, --fullscreen <true|false> Specify window initial fullscreen "
              << "state. Does not overwrite the config file.\n"
              << "  --add-game-folder <folder>    Adds a new game folder to the config.\n"
              << "  --set-addon-folder <folder>   Sets the addon folder to the config.\n"
              << "  --log-append                  Append log output to file instead of "
              << "overwriting it.\n"
              << "  --override-root <folder>      Override the game root folder. Default is the "
              << "parent of game path\n"
              << "  --wait-for-debugger           Wait for debugger to attach\n"
              << "  --wait-for-pid <pid>          Wait for process with specified PID to stop\n"
              << "  --config-clean                Run the emulator with the default config "
              << "values, ignores the config file(s) entirely.\n"
              << "  --config-global               Run the emulator with the base config file "
              << "only, ignores game specific configs.\n"
              << "  --show-fps                    Enable FPS counter display at startup\n"
              << "  -h, --help                    Display this help message\n";
}

std::optional<std::string> ArgParser::ValidatePath(const std::string& path, bool is_file) const {
    std::filesystem::path fs_path(path);
    std::error_code ec;
    
    if (is_file) {
        if (std::filesystem::is_regular_file(fs_path, ec)) {
            return path;
        }
    } else {
        if (std::filesystem::is_directory(fs_path, ec)) {
            return path;
        }
    }
    return std::nullopt;
}

void ArgParser::RegisterArguments() {
    // Help argument
    auto help_callback = [this](int&) {
        PrintHelp();
        exit(0);
    };

    arg_map["-h"] = help_callback;
    arg_map["--help"] = help_callback;

    // Game path argument
    auto game_callback = [this](int& i) {
        // Will be handled in Parse()
    };
    arg_map["-g"] = game_callback;
    arg_map["--game"] = game_callback;

    // Patch file argument
    auto patch_callback = [this](int& i) {
        // Will be handled in Parse()
    };
    arg_map["-p"] = patch_callback;
    arg_map["--patch"] = patch_callback;

    // Ignore game patch
    arg_map["-i"] = [](int&) {
        Core::FileSys::MntPoints::ignore_game_patches = true;
    };
    arg_map["--ignore-game-patch"] = arg_map["-i"];

    // Fullscreen argument
    arg_map["-f"] = [this](int& i) {
        // Will be handled in Parse()
    };
    arg_map["--fullscreen"] = arg_map["-f"];

    // Add game folder
    arg_map["--add-game-folder"] = [this](int& i) {
        // Will be handled in Parse()
    };

    // Set addon folder
    arg_map["--set-addon-folder"] = [this](int& i) {
        // Will be handled in Parse()
    };

    // Log append
    arg_map["--log-append"] = [](int&) {
        Common::Log::SetAppend();
    };

    // Config modes
    arg_map["--config-clean"] = [](int&) {
        Config::setConfigMode(Config::ConfigMode::Clean);
    };
    arg_map["--config-global"] = [](int&) {
        Config::setConfigMode(Config::ConfigMode::Global);
    };

    // Override root
    arg_map["--override-root"] = [this](int& i) {
        // Will be handled in Parse()
    };

    // Wait for debugger
    arg_map["--wait-for-debugger"] = [this](int&) {
        result.wait_for_debugger = true;
    };

    // Wait for PID
    arg_map["--wait-for-pid"] = [this](int& i) {
        // Will be handled in Parse()
    };

    // Show FPS
    arg_map["--show-fps"] = [](int&) {
        Config::setShowFpsCounter(true);
    };
}

ArgParser::ParsedArgs ArgParser::Parse(int argc, char* argv[]) {
    result = ParsedArgs{};

    if (argc == 1) {
        PrintHelp();
        exit(1);
    }

    for (int i = 1; i < argc; ++i) {
        std::string cur_arg = argv[i];

        // Handle special arguments that require parameters
        if (cur_arg == "-g" || cur_arg == "--game") {
            if (i + 1 < argc) {
                result.game_path = argv[++i];
                result.has_game_argument = true;
            } else {
                std::cerr << "Error: Missing argument for " << cur_arg << "\n";
                exit(1);
            }
            continue;
        }

        if (cur_arg == "-p" || cur_arg == "--patch") {
            if (i + 1 < argc) {
                MemoryPatcher::patch_file = argv[++i];
            } else {
                std::cerr << "Error: Missing argument for " << cur_arg << "\n";
                exit(1);
            }
            continue;
        }

        if (cur_arg == "-f" || cur_arg == "--fullscreen") {
            if (++i >= argc) {
                std::cerr << "Error: Missing argument for " << cur_arg << "\n";
                exit(1);
            }
            std::string f_param(argv[i]);
            bool is_fullscreen;
            if (f_param == "true") {
                is_fullscreen = true;
            } else if (f_param == "false") {
                is_fullscreen = false;
            } else {
                std::cerr << "Error: Invalid argument for " << cur_arg 
                          << ". Use 'true' or 'false'.\n";
                exit(1);
            }
            Config::setIsFullscreen(is_fullscreen);
            continue;
        }

        if (cur_arg == "--add-game-folder") {
            if (++i >= argc) {
                std::cerr << "Error: Missing argument for --add-game-folder\n";
                exit(1);
            }
            std::string config_dir(argv[i]);
            if (auto validated = ValidatePath(config_dir, false)) {
                std::filesystem::path config_path = std::filesystem::path(*validated);
                Config::addGameInstallDir(config_path);
                Config::save(Common::FS::GetUserPath(Common::FS::PathType::UserDir) / "config.toml");
                std::cout << "Game folder successfully saved.\n";
                exit(0);
            } else {
                std::cerr << "Error: File does not exist: " << config_dir << "\n";
                exit(1);
            }
            continue;
        }

        if (cur_arg == "--set-addon-folder") {
            if (++i >= argc) {
                std::cerr << "Error: Missing argument for --set-addon-folder\n";
                exit(1);
            }
            std::string config_dir(argv[i]);
            if (auto validated = ValidatePath(config_dir, false)) {
                std::filesystem::path config_path = std::filesystem::path(*validated);
                Config::setAddonInstallDir(config_path);
                Config::save(Common::FS::GetUserPath(Common::FS::PathType::UserDir) / "config.toml");
                std::cout << "Addon folder successfully saved.\n";
                exit(0);
            } else {
                std::cerr << "Error: File does not exist: " << config_dir << "\n";
                exit(1);
            }
            continue;
        }

        if (cur_arg == "--override-root") {
            if (++i >= argc) {
                std::cerr << "Error: Missing argument for --override-root\n";
                exit(1);
            }
            std::string folder_str{argv[i]};
            std::filesystem::path folder{folder_str};
            if (std::filesystem::exists(folder) && std::filesystem::is_directory(folder)) {
                result.game_folder = folder;
            } else {
                std::cerr << "Error: Folder does not exist: " << folder_str << "\n";
                exit(1);
            }
            continue;
        }

        if (cur_arg == "--wait-for-pid") {
            if (++i >= argc) {
                std::cerr << "Error: Missing argument for --wait-for-pid\n";
                exit(1);
            }
            try {
                result.wait_pid = std::stoi(argv[i]);
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid PID argument: " << e.what() << "\n";
                exit(1);
            }
            continue;
        }

        // Handle arguments registered in the map
        auto it = arg_map.find(cur_arg);
        if (it != arg_map.end() && cur_arg != "-h" && cur_arg != "--help") {
            it->second(i);
            continue;
        }

        // Handle game arguments after "--"
        if (cur_arg == "--") {
            if (i + 1 == argc) {
                std::cerr << "Warning: -- is set, but no game arguments are added!\n";
                break;
            }
            for (int j = i + 1; j < argc; ++j) {
                result.game_args.push_back(argv[j]);
            }
            break;
        }

        // Assume last argument is game file if not specified
        if (i == argc - 1 && !result.has_game_argument) {
            // Check if next argument after current arg is "--"
            if (i + 1 < argc && std::string(argv[i + 1]) == "--") {
                result.game_path = argv[i];
                result.has_game_argument = true;
                continue;
            }
            if (cur_arg[0] != '-') {
                result.game_path = argv[i];
                result.has_game_argument = true;
                continue;
            }
        }

        std::cerr << "Unknown argument: " << cur_arg << ", see --help for info.\n";
    }

    return result;
}

} // namespace Common
