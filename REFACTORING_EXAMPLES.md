# shadPS4 Refactoring Examples and Migration Guide

## Quick Reference: Old vs New Code Patterns

### 1. Argument Parsing

**BEFORE** (276 lines, complex lambdas):
```cpp
std::unordered_map<std::string, std::function<void(int&)>> arg_map = {
    {"-g", [&](int& i) {
        if (i + 1 < argc) {
            game_path = argv[++i];
            has_game_argument = true;
        } else {
            std::cerr << "Error: Missing argument for -g/--game\n";
            exit(1);
        }
    }},
    // ... 50 more lambdas ...
};

for (int i = 1; i < argc; ++i) {
    std::string cur_arg = argv[i];
    auto it = arg_map.find(cur_arg);
    if (it != arg_map.end()) {
        it->second(i);
    }
    // ... complex branching ...
}
```

**AFTER** (clean, type-safe):
```cpp
Common::ArgParser parser;
auto args = parser.Parse(argc, argv);

if (!args.has_game_argument) {
    std::cerr << "Error: Please provide a game path or ID.\n";
    return 1;
}

auto eboot_path = ResolveGamePath(args.game_path);
auto* emulator = Common::Singleton<Core::Emulator>::Instance();
emulator->Run(eboot_path, args.game_args, args.game_folder);
```

**Migration Path**:
1. Keep old code as-is initially
2. Import new `ArgParser` in main.cpp
3. Replace argument parsing section
4. Test thoroughly
5. Remove old unordered_map code

---

### 2. Game Metadata Loading

**BEFORE** (mixed in emulator.cpp):
```cpp
std::string id;
std::string title;
std::string app_version;
u32 sdk_version;
u32 fw_version;
Common::PSFAttributes psf_attributes{};

if (param_sfo_exists) {
    auto* param_sfo = Common::Singleton<PSF>::Instance();
    ASSERT_MSG(param_sfo->Open(param_sfo_path), "Failed to open param.sfo");

    const auto content_id = param_sfo->GetString("CONTENT_ID");
    const auto title_id = param_sfo->GetString("TITLE_ID");
    if (content_id.has_value() && !content_id->empty()) {
        id = std::string(*content_id, 7, 9);
    } else if (title_id.has_value()) {
        id = *title_id;
    }
    
    title = param_sfo->GetString("TITLE").value_or("Unknown title");
    fw_version = param_sfo->GetInteger("SYSTEM_VER").value_or(0x4700000);
    app_version = param_sfo->GetString("APP_VER").value_or("Unknown version");
    
    // ... 20 more lines of parsing ...
}
```

**AFTER** (extracted, typed, reusable):
```cpp
auto psf_data = Core::LoadPSFData(param_sfo_path);
if (psf_data) {
    const auto& data = *psf_data;
    LOG_INFO(Loader, "Game: {} ({})", data.title, data.id);
    LOG_INFO(Loader, "Versions: FW={:#x}, App={}", data.fw_version, data.app_version);
}
```

**Benefits of New Approach**:
- Single `PSFData` struct replaces 5 variables
- Extraction logic is testable
- Reusable across project
- Clear ownership and lifetime

---

### 3. Configuration Logging

**BEFORE** (30+ LOG_INFO calls in Run() function):
```cpp
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
// ... 18 more similar lines ...
```

**AFTER** (single function call):
```cpp
Core::LogConfiguration();
```

**The Implementation** (`game_metadata.cpp`):
```cpp
void LogConfiguration() {
    LOG_INFO(Config, "General LogType: {}", Config::getLogType());
    LOG_INFO(Config, "General isNeo: {}", Config::isNeoModeConsole());
    // ... same logging, but organized and reusable
}
```

---

### 4. Directory Mounting and Creation

**BEFORE** (repeated pattern):
```cpp
const auto& mount_data_dir = Common::FS::GetUserPath(Common::FS::PathType::GameDataDir) / id;
if (!std::filesystem::exists(mount_data_dir)) {
    std::filesystem::create_directory(mount_data_dir);
}
mnt->Mount(mount_data_dir, "/data");

const auto& mount_temp_dir = Common::FS::GetUserPath(Common::FS::PathType::TempDataDir) / id;
if (std::filesystem::exists(mount_temp_dir)) {
    std::filesystem::remove_all(mount_temp_dir);
}
std::filesystem::create_directory(mount_temp_dir);
mnt->Mount(mount_temp_dir, "/temp0");

// ... similar pattern repeats ...
```

**AFTER** (with utility function):
```cpp
const auto mount_data_dir = Common::FS::GetUserPath(Common::FS::PathType::GameDataDir) / id;
if (Core::GameUtil::EnsureDirectory(mount_data_dir, "game data")) {
    mnt->Mount(mount_data_dir, "/data");
}

const auto mount_temp_dir = Common::FS::GetUserPath(Common::FS::PathType::TempDataDir) / id;
if (Core::GameUtil::EnsureDirectory(mount_temp_dir, "game temp")) {
    mnt->Mount(mount_temp_dir, "/temp0");
}
```

---

### 5. Game Folder Resolution

**BEFORE** (logic duplicated, hard to understand):
```cpp
std::filesystem::path game_folder;
if (p_game_folder.has_value()) {
    game_folder = p_game_folder.value();
} else {
    game_folder = file.parent_path();
    if (const auto game_folder_name = game_folder.filename().string();
        game_folder_name.ends_with("-UPDATE") || game_folder_name.ends_with("-patch")) {
        const std::string base_name = game_folder_name.substr(0, game_folder_name.rfind('-'));
        const auto base_path = game_folder.parent_path() / base_name;
        if (std::filesystem::is_directory(base_path)) {
            game_folder = base_path;
        }
    }
}
```

**AFTER** (single utility call):
```cpp
auto game_folder = Core::GameUtil::ResolveGameFolder(file, p_game_folder);
```

**Why This Matters**:
- Updates/patches handled consistently
- Logic reusable in Restart()
- Testable function
- Clear responsibility

---

## Integration Checklist

### Step 1: Add New Files to CMakeLists.txt
```cmake
# In src/common/CMakeLists.txt
target_sources(common PRIVATE
    arg_parser.cpp
    # ... existing files ...
)

# In src/core/CMakeLists.txt
target_sources(core PRIVATE
    game_util.cpp
    game_metadata.cpp
    # ... existing files ...
)
```

### Step 2: Update Includes in main.cpp
```cpp
#include "common/arg_parser.h"
#include "core/game_util.h"
```

### Step 3: Update Includes in emulator.cpp
```cpp
#include "core/game_metadata.h"
#include "core/game_util.h"
```

### Step 4: Verify Compilation
```bash
cmake --build build --config Release
```

### Step 5: Run Tests
```bash
ctest --output-on-failure
```

---

## Common Mistakes to Avoid

### ❌ DON'T: Keep Both Old and New Code

```cpp
// Wrong - causes confusion
auto args = parser.Parse(argc, argv);  // New way
std::string game_path = argv[1];       // Old way mixed in
```

### ✓ DO: Complete Migration

```cpp
// Right - use new utilities consistently
auto args = parser.Parse(argc, argv);
auto eboot_path = ResolveGamePath(args.game_path);
// Use args.* for all command-line values
```

### ❌ DON'T: Ignore Error Cases

```cpp
// Wrong - silently fails
auto psf = LoadPSFData(path);
// Assumed to be valid, may crash
auto id = psf->id;
```

### ✓ DO: Handle Optional Results

```cpp
// Right - check for validity
if (auto psf = LoadPSFData(path)) {
    auto id = psf->id;
} else {
    LOG_WARN(Loader, "No PSF found, using defaults");
}
```

### ❌ DON'T: Pass Too Many Parameters

```cpp
// Wrong - hard to remember order
void Init(string a, string b, int c, int d, bool e, bool f);
```

### ✓ DO: Use Structured Parameters

```cpp
// Right - clear what each value is
struct InitParams {
    std::string id;
    std::string title;
    uint32_t fw_version;
};
void Init(const InitParams& params);
```

---

## Performance Impact

### Memory
- **Before**: Multiple variables scattered = potential alignment issues
- **After**: Structured types = better cache locality

### Speed
- **Argument parsing**: $O(n)$ (unchanged)
- **PSF loading**: $O(1)$ (same complexity, now cached in struct)
- **Overall startup**: Negligible impact (parsing is tiny part)

### Compilation
- New files add minimal compile time
- Can be compiled separately in modular builds

---

## Testing the Refactored Code

### Unit Test Example: ArgParser

```cpp
#include <gtest/gtest.h>
#include "common/arg_parser.h"

TEST(ArgParser, ParseGamePath) {
    const char* argv[] = {"shadps4", "-g", "/path/to/game.elf"};
    Common::ArgParser parser;
    auto args = parser.Parse(3, (char**)argv);
    
    EXPECT_TRUE(args.has_game_argument);
    EXPECT_EQ(args.game_path, "/path/to/game.elf");
}

TEST(ArgParser, MissingGamePath) {
    const char* argv[] = {"shadps4"};
    Common::ArgParser parser;
    
    // Should exit or return error state
    EXPECT_THROW(parser.Parse(1, (char**)argv), std::exception);
}
```

### Unit Test Example: GameUtil

```cpp
TEST(GameUtil, ResolvesUpdateFolder) {
    auto base_path = std::filesystem::path("C:/Games/MyGame");
    auto update_path = base_path / "MyGame-UPDATE";
    
    // Simulate UPDATE folder existing
    auto resolved = Core::GameUtil::ResolveGameFolder(
        update_path / "eboot.bin", 
        std::nullopt
    );
    
    EXPECT_EQ(resolved, base_path);
}
```

---

## Future Enhancements

### Phase 2: Configuration Refactoring
Once this refactoring is stable, consider:
```cpp
struct EmulatorConfiguration {
    struct GPUSettings {
        bool null_gpu;
        int gpu_id;
        bool dump_shaders;
    };
    
    struct VulkanSettings {
        bool validation;
        bool crash_diagnostics;
    };
    
    GPUSettings gpu;
    VulkanSettings vulkan;
};

auto config = EmulatorConfiguration::Load(game_id);
```

### Phase 3: Dependency Injection
Replace Singleton usage with DI container:
```cpp
class EmulatorFactory {
    std::unique_ptr<Emulator> CreateEmulator(
        std::unique_ptr<IMemoryManager> memory,
        std::unique_ptr<ILinker> linker
    );
};
```

---

## Questions & Answers

**Q: Do I have to use the new utilities?**
A: No, they're optional. But recommended for new code.

**Q: Can I mix old and new patterns?**
A: Gradual migration is fine, but avoid confusion in same function.

**Q: What if a utility doesn't fit my use case?**
A: Create a new utility or extend existing ones.

**Q: Are these changes breaking?**
A: No, all changes are additive. Existing code works unchanged.

---

## Support & Questions

For questions about the refactoring:
1. Check REFACTORING_GUIDE.md
2. Review CODE_QUALITY_STANDARDS.md
3. See ARCHITECTURE_IMPROVEMENTS.md for patterns
4. File an issue on GitHub

---

**Last Updated**: January 12, 2026  
**Version**: 1.0
