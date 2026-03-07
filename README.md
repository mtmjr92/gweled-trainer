# gweled-hack

External memory hack for [Gweled](https://en.wikipedia.org/wiki/Gweled) (Bejeweled clone) on Linux. Modifies the game's score by reading and writing process memory directly. For educational purposes only.

## Requirements

- Linux (uses `process_vm_readv` / `process_vm_writev`)
- g++ with C++11 or later
- Root privileges or `CAP_SYS_PTRACE` capability
- Gweled installed and running

## Build

### Make (Eclipse CDT)

```bash
cd Debug
make clean
make
```

### CMake

```bash
mkdir build && cd build
cmake ..
make
```

## Configuration

All game-specific settings are defined in `config.json`:

```json
{
    "game_version": "1.0-beta1",
    "process_name": "gweled",
    "module_name": "gweled",
    "offsets": {
        "score": "0xF480",
        "current_score": "0xF4A0"
    }
}
```

| Field | Description |
|---|---|
| `game_version` | Target game version these offsets were found for |
| `process_name` | Process name to search in `/proc/*/comm` |
| `module_name` | Module name to resolve base address from `/proc/<pid>/maps` |
| `offsets.score` | Hex offset from module base to the score value |
| `offsets.current_score` | Hex offset from module base to the displayed score value |

Offsets can be changed without recompiling.

## Usage

Run from the **project root** (where `config.json` is located):

```bash
# Set score to 450 (default)
sudo ./Debug/gweled-hack

# Set score to a custom value
sudo ./Debug/gweled-hack 9999

# Use a different config file
sudo ./Debug/gweled-hack -c myconfig.json 9999

# Show help
./Debug/gweled-hack -h
```

If running from the `Debug/` directory, specify the config path:

```bash
cd Debug
sudo ./gweled-hack -c ../config.json
```

## Project Structure

```
gweled-hack/
├── config.json               # Game offsets and process configuration
├── CMakeLists.txt            # CMake build
├── Debug/                    # Eclipse CDT build
│   ├── makefile
│   └── src/subdir.mk
└── src/
    ├── main.cpp              # Entry point, CLI parsing
    ├── config.h/cpp          # JSON config loader
    ├── process_memory.h/cpp  # PID lookup, memory read/write
    └── gweled_hack.h/cpp     # Game-specific score logic
```

All source files are under the `gwl` namespace.

### Config

Loads and parses `config.json` at startup. Lightweight JSON parser with no external dependencies.

### ProcessMemory

Reusable class for interacting with any Linux process:

- Finds PID by scanning `/proc/*/comm`
- Parses `/proc/<pid>/maps` to resolve module base addresses
- Type-safe `readValue<T>()` / `writeValue<T>()` via `process_vm_readv` / `process_vm_writev`

### GweledHack

Game-specific layer built on `ProcessMemory` and `Config`:

- Resolves module base address on construction
- Reads and writes score at config-defined offsets

## How It Works

1. Loads offsets and process name from `config.json`
2. Scans `/proc` to find the running process PID
3. Parses `/proc/<pid>/maps` to get the module base address
4. Adds configured offsets to the base address to locate score variables
5. Uses `process_vm_writev` to write the desired score into memory

## References

- [BananaBot](https://github.com/ps1337/bananabot)
