// SPDX-FileCopyrightText: Copyright 2025 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/**
 * @file CODE_QUALITY_STANDARDS.md
 * @brief Code quality standards and best practices for the shadPS4 project
 */

/*
 * # shadPS4 Code Quality Standards
 * 
 * ## 1. Naming Conventions
 * 
 * ### Files
 * - Use snake_case for files: `game_util.h`, `arg_parser.cpp`
 * - Header guards: `FILENAME_H` or use `#pragma once` (preferred)
 * - Group related files in directories: `core/`, `common/`, `video_core/`
 * 
 * ### Classes and Types
 * - Use PascalCase: `ArgParser`, `GameMetadata`, `EmulatorState`
 * - Abbreviations stay together: `PSFData` (not `PsfData`)
 * - Template parameters: `T` for single type, `Elem` for containers
 * 
 * ### Functions and Methods
 * - Use camelCase: `ParseArguments()`, `LoadGameMetadata()`
 * - Getters: `GetWindowWidth()` or `windowWidth()` (both acceptable)
 * - Predicates: `IsValid()`, `HasConfiguration()`
 * 
 * ### Variables
 * - Local variables: `snake_case`: `game_path`, `wait_pid`
 * - Member variables: `snake_case`: `m_instance`, `m_game_folder`
 * - Constants: `SCREAMING_SNAKE_CASE`: `MAX_ARGS`, `DEFAULT_TIMEOUT`
 * 
 * ## 2. Code Organization
 * 
 * ### Header Organization
 * ```cpp
 * #pragma once
 * 
 * #include <standard_headers>
 * #include <third_party>
 * #include "project_headers"
 * 
 * namespace ProjectNamespace {
 * 
 * /// Documentation comment
 * class MyClass { };
 * 
 * } // namespace ProjectNamespace
 * ```
 * 
 * ### Implementation Organization
 * ```cpp
 * #include "myclass.h"
 * #include "dependencies"
 * 
 * namespace ProjectNamespace {
 * 
 * // Static/private functions first
 * namespace {
 * 
 * void HelperFunction() { }
 * 
 * } // anonymous namespace
 * 
 * // Class implementation
 * MyClass::MyClass() { }
 * 
 * } // namespace ProjectNamespace
 * ```
 * 
 * ## 3. Function Design
 * 
 * ### Single Responsibility Principle
 * Each function should have one clear purpose:
 * 
 * ✓ Good:
 * ```cpp
 * std::optional<PSFData> LoadPSFData(const std::filesystem::path& path);
 * void LogConfiguration();
 * ```
 * 
 * ✗ Bad:
 * ```cpp
 * void LoadAndLogEverything(); // Too many concerns
 * ```
 * 
 * ### Function Signatures
 * - Keep parameter count ≤ 4. Use struct for more parameters:
 * 
 * ✓ Good:
 * ```cpp
 * struct GameConfig {
 *     std::string id;
 *     std::string title;
 *     uint32_t version;
 * };
 * void InitializeGame(const GameConfig& config);
 * ```
 * 
 * ✗ Bad:
 * ```cpp
 * void InitializeGame(const std::string& id, const std::string& title,
 *                     uint32_t version, /* ... 10 more params */);
 * ```
 * 
 * ### Return Values
 * - Use `std::optional<T>` for operations that might fail
 * - Use `Result<T>` or `Expected<T, Error>` for detailed error info
 * - Avoid out-parameters; use return values instead
 * 
 * ✓ Good:
 * ```cpp
 * std::optional<GameInfo> LoadGameInfo(const std::filesystem::path& path);
 * if (auto info = LoadGameInfo(path)) {
 *     // use *info
 * }
 * ```
 * 
 * ✗ Bad:
 * ```cpp
 * bool LoadGameInfo(const std::filesystem::path& path, GameInfo& out_info);
 * GameInfo info;
 * if (!LoadGameInfo(path, info)) { /* ... */ }
 * ```
 * 
 * ## 4. Memory Management
 * 
 * ### Smart Pointers
 * - Prefer `std::unique_ptr` for exclusive ownership
 * - Use `std::shared_ptr` only when necessary
 * - Avoid raw `new`/`delete`
 * 
 * ✓ Good:
 * ```cpp
 * auto window = std::make_unique<Frontend::WindowSDL>(...);
 * auto shared = std::make_shared<Resource>();
 * ```
 * 
 * ✗ Bad:
 * ```cpp
 * Frontend::WindowSDL* window = new Frontend::WindowSDL(...);
 * delete window; // Manual cleanup prone to errors
 * ```
 * 
 * ### RAII Principle
 * Resources should be acquired in constructor and released in destructor:
 * 
 * ```cpp
 * class FileHandle {
 * public:
 *     FileHandle(const std::filesystem::path& path) {
 *         file = std::fopen(path.c_str(), "rb");
 *     }
 *     ~FileHandle() {
 *         if (file) std::fclose(file);
 *     }
 * private:
 *     FILE* file = nullptr;
 * };
 * ```
 * 
 * ## 5. Error Handling
 * 
 * ### Exception Safety
 * Provide strong exception guarantee when possible:
 * ```cpp
 * void ModifyData(const std::vector<int>& new_values) {
 *     auto backup = data;  // Backup
 *     try {
 *         data = ProcessData(new_values);
 *     } catch (...) {
 *         data = backup;  // Restore on error
 *         throw;
 *     }
 * }
 * ```
 * 
 * ### Error Codes vs Exceptions
 * - Use exceptions for exceptional conditions (rare, non-recoverable)
 * - Use return codes/optional for normal failures (missing config, invalid input)
 * 
 * ✓ Good:
 * ```cpp
 * std::optional<Configuration> LoadConfig(const std::filesystem::path& path);
 * // If file doesn't exist, return empty optional
 * ```
 * 
 * ### Logging Errors
 * ```cpp
 * if (!std::filesystem::exists(path)) {
 *     LOG_ERROR(Loader, "File not found: {}", path.string());
 *     return std::nullopt;
 * }
 * ```
 * 
 * ## 6. Const Correctness
 * 
 * ### Basic Rules
 * - Mark functions const when they don't modify state:
 * 
 * ✓ Good:
 * ```cpp
 * class Configuration {
 * public:
 *     bool IsValid() const { return !config.empty(); }
 *     void SetValue(const std::string& key, const std::string& value) {
 *         config[key] = value;
 *     }
 * };
 * ```
 * 
 * ### Reference Parameters
 * - Use const references for input: `const std::string& name`
 * - Non-const reference for output: `std::vector<Result>& results`
 * - Never use mutable to bypass const (usually a smell)
 * 
 * ## 7. Template Guidelines
 * 
 * ### Keep Templates Simple
 * Complex logic should be in non-template code:
 * 
 * ```cpp
 * // Template handles type safety
 * template<typename T>
 * class ObjectPool {
 *     std::vector<T> objects;
 * public:
 *     T* Allocate() { /* ... */ }
 * };
 * 
 * // Implementation details in non-template
 * void ConfigurePool(ObjectPool<GameInstance>& pool) { }
 * ```
 * 
 * ## 8. Documentation
 * 
 * ### Comments Should Explain WHY, Not WHAT
 * 
 * ✗ Bad (restates code):
 * ```cpp
 * // Increment counter
 * counter++;
 * ```
 * 
 * ✓ Good (explains intent):
 * ```cpp
 * // Track retries to abort after too many failures
 * counter++;
 * if (counter > MAX_RETRIES) {
 *     AbortOperation();
 * }
 * ```
 * 
 * ### Doxygen Comments for Public APIs
 * ```cpp
 * /**
 *  * @brief Load game metadata from PSF file
 *  * 
 *  * @param path Path to param.sfo file
 *  * @return PSFData structure, or empty optional if load fails
 *  * 
 *  * @note Logs errors via LOG_ERROR if file cannot be opened
 *  */
 * std::optional<PSFData> LoadPSFData(const std::filesystem::path& path);
 * ```
 * 
 * ## 9. Testing Considerations
 * 
 * ### Testable Code Characteristics
 * - Functions have clear inputs and outputs
 * - No global state dependency (or explicit)
 * - Dependencies are passed in (not hardcoded)
 * - Deterministic behavior
 * 
 * ✓ Testable:
 * ```cpp
 * std::string FormatGameTitle(const PSFData& psf) {
 *     return fmt::format("{} - {}", psf.id, psf.title);
 * }
 * // Test: FormatGameTitle(test_data) == expected_output
 * ```
 * 
 * ✗ Hard to test:
 * ```cpp
 * std::string GetGameTitle() {
 *     auto psf = LoadPSFData(Config::GetGamePath());  // Hard dependencies
 *     return fmt::format("{} - {}", psf.id, psf.title);
 * }
 * ```
 * 
 * ## 10. Performance Considerations
 * 
 * ### Avoid Unnecessary Copies
 * ```cpp
 * // ✗ Bad: Copies vector
 * void ProcessGames(std::vector<GameInfo> games);
 * 
 * // ✓ Good: Uses reference
 * void ProcessGames(const std::vector<GameInfo>& games);
 * ```
 * 
 * ### Use Move Semantics
 * ```cpp
 * // ✓ Good: Transfers ownership
 * void SetGames(std::vector<GameInfo>&& games) {
 *     m_games = std::move(games);
 * }
 * ```
 * 
 * ### Reserve Capacity
 * ```cpp
 * // ✓ Good: Avoids reallocations
 * std::vector<Result> results;
 * results.reserve(games.size());
 * for (const auto& game : games) {
 *     results.push_back(ProcessGame(game));
 * }
 * ```
 * 
 * ## 11. Common Patterns in shadPS4
 * 
 * ### Singleton Pattern (When Used)
 * ```cpp
 * auto* instance = Common::Singleton<MyComponent>::Instance();
 * // Prefer dependency injection where possible
 * ```
 * 
 * ### Filesystem Operations
 * Always use `std::filesystem` and check `std::error_code`:
 * ```cpp
 * std::error_code ec;
 * if (!std::filesystem::exists(path, ec)) {
 *     if (ec) {
 *         LOG_ERROR(Common, "Error checking path: {}", ec.message());
 *     }
 *     return false;
 * }
 * ```
 * 
 * ### Logging Pattern
 * ```cpp
 * LOG_INFO(Category, "Message with formatting: {}", value);
 * LOG_ERROR(Category, "Error condition: {}", error_details);
 * LOG_DEBUG(Category, "Debug info: {}", debug_value);
 * ```
 */

// This file serves as documentation only - see CODE_QUALITY_STANDARDS.md for full details
