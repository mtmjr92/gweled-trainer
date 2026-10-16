# gweled-trainer

External memory trainer for [Gweled](https://en.wikipedia.org/wiki/Gweled) (Bejeweled clone) on Linux. An interactive terminal menu, built with [FTXUI](https://github.com/ArthurSonzogni/FTXUI), that reads and writes the game's process memory directly. For educational purposes only.

> **Supported game version: Gweled 1.0-beta1** (Debian/Ubuntu package `1.0~beta1-1`). The memory offsets only fit this build, so the trainer refuses to attach to any other version.

```
╭────────────────── Gweled Trainer v1.0 ───────────────────╮
│ Process: gweled  PID 3385774                  ● Attached │
│ Base: 0x58c4ed4a4000                  Gweled 1.0-beta1 ✓ │
├──────────────────────────────────────────────────────────┤
│>  [ON ] Freeze Score                                   F1│
│         Set Score  (uses value)                        F2│
│         Add +1000                                      F3│
│         Reattach                                       F5│
│         Quit                                          Esc│
├──────────────────────────────────────────────────────────┤
│ Value: 9999                                              │
├──────────────────────────────────────────────────────────┤
│ Score: 9999   Score frozen at 9999                       │
╰──────────────────────────────────────────────────────────╯
 ↑↓ move · Enter select · Tab value · F1-F5 keys · Esc quit
```

## Features

- **Freeze Score**: locks the score at its current value. The game can no longer change it.
- **Set Score**: writes any value typed in the value field.
- **Add +1000**: adds 1000 to the current score.
- **Auto-attach**: start the trainer before or after the game. It waits for the process, attaches, and detaches cleanly when the game closes.
- **Version check**: confirms that the running game is Gweled 1.0-beta1 before touching its memory.
- **Live status**: PID, module base address, current score and a log of the last action.
- **CLI mode**: `gweled-trainer 9999` sets the score once and exits, for scripts.

## Requirements

- Linux (uses `process_vm_readv` / `process_vm_writev`)
- CMake 3.14+ and a C++17 compiler (e.g. g++)
- Internet access on first configure: FTXUI v5.0.0 is fetched with `FetchContent`
- Root privileges or the `CAP_SYS_PTRACE` capability (see [Permissions](#permissions))
- Gweled **1.0-beta1** (`sudo apt install gweled` on Debian/Ubuntu; check with `dpkg -s gweled | grep Version`)

## Build

```bash
cmake -B build && cmake --build build
```

The binary is `build/gweled-trainer`. `config.json` is copied next to it on every build.

## Usage

```bash
cd build
sudo ./gweled-trainer                # interactive menu
sudo ./gweled-trainer 9999           # set the score once and exit
sudo ./gweled-trainer -c other.json  # use another config file
./gweled-trainer -h                  # help
```

`config.json` is looked up in the current directory unless `-c` is given.

### Controls

| Key | Action |
|---|---|
| `↑` `↓` | Move through the menu |
| `Enter` | Run the selected option. On **Set Score**, it jumps to the value field |
| `Tab` | Switch between the menu and the value field |
| `Enter` (in the value field) | Apply **Set Score** with the typed value |
| `F1` / `F2` / `F3` | Freeze Score / Set Score / Add +1000 |
| `F5` | Reattach to the game |
| `Esc` / `q` | Quit (`q` only outside the value field) |

### Editing the score

1. Select **Set Score** and press `Enter`. The cursor moves to the value field.
2. Delete the old value with `Backspace` and type the new one. Only digits and `-` are accepted.
3. Press `Enter` to apply it, or `Tab` to go back to the menu without applying.

If **Freeze Score** is on, the frozen value follows the new score.

### Permissions

Reading another process's memory requires privileges. Ubuntu and most distros also restrict it via Yama (`/proc/sys/kernel/yama/ptrace_scope` = 1). Without them, the trainer finds the game but cannot access it, and the menu shows:

```
│ Process: gweled  PID -                   ✕ Run with sudo │
│ Score: -   Restart with: sudo ./gweled-trainer           │
```

Two options:

```bash
# Run as root
sudo ./gweled-trainer

# Or grant the capability once per build and run as a normal user
sudo setcap cap_sys_ptrace=eip build/gweled-trainer
./build/gweled-trainer
```

## Supported version

Only **Gweled 1.0-beta1** is supported. The score lives at fixed offsets inside the game binary, and any other build has a different layout. Writing to those addresses there could corrupt or crash the game.

On every attach, the trainer reads the running executable (`/proc/<pid>/exe`) and checks that it contains the `game_version` string from `config.json`. If it does not, the trainer stays detached:

```
│ Process: gweled  PID -              ✕ Wrong game version │
│ Base: -                        Requires Gweled 1.0-beta1 │
```

In CLI mode, it prints `Error: Unsupported game version (requires Gweled 1.0-beta1)`.

To support another version, find the new offsets (e.g. with a memory scanner) and put them in a config file, with `game_version` set to the version string embedded in that binary.

## Configuration

All game-specific settings are in `config.json`, so offsets can be changed without recompiling:

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
| `game_version` | Game version these offsets were found for. It must appear in the game binary, or the trainer refuses to attach |
| `process_name` | Process name matched against `/proc/*/comm` |
| `module_name` | Module whose base address is read from `/proc/<pid>/maps` |
| `offsets.score` | Hex offset from the module base to the score value |
| `offsets.current_score` | Hex offset from the module base to the displayed score value |

## Troubleshooting

| Symptom | Cause / fix |
|---|---|
| `✕ Run with sudo` | No permission to access the game's memory. See [Permissions](#permissions) |
| `○ Waiting for game` | Gweled is not running, or `process_name` does not match. The trainer keeps retrying every second |
| `✕ Wrong game version` | The running Gweled is not 1.0-beta1. See [Supported version](#supported-version) |
| `Module not found in maps` | `module_name` does not match the game binary's name |
| An `F` key does nothing | The terminal uses the key itself (e.g. GNOME Terminal's `F1` = Help). Use the arrows and `Enter`, or disable the shortcut |
| `Cannot open config file` | Run from `build/`, or pass `-c path/to/config.json` |

## Project Structure

```
gweled-trainer/
├── config.json               # Game offsets and process configuration
├── CMakeLists.txt            # CMake build (fetches FTXUI)
└── src/
    ├── main.cpp              # Entry point: CLI parsing, menu or one-shot mode
    ├── config.h/cpp          # Minimal JSON config loader
    ├── process_memory.h/cpp  # PID lookup, module base, memory read/write
    ├── trainer.h/cpp         # Game layer: attach/detach, score, score lock
    ├── feature.h/cpp         # Menu options (Freeze, Set, Add)
    └── tui.h/cpp             # FTXUI menu and background worker
```

All source files are under the `gwl` namespace.

| Component | Role |
|---|---|
| `Config` | Loads `config.json` with a small parser and no external dependencies |
| `ProcessMemory` | Reusable for any Linux process: finds the PID in `/proc/*/comm`, resolves module bases from `/proc/<pid>/maps`, and offers type-safe `readValue<T>()` / `writeValue<T>()`. It throws `PermissionError` when the kernel denies access |
| `Trainer` | Attaches to the game, checks that memory is accessible and that the binary is the configured version. Reads and writes the score at configured offsets and holds the score lock used by Freeze |
| `Feature` | One menu option: a name, toggle or action, and whether it uses the value field. Features are stateless; their state lives in `Trainer` |
| TUI | FTXUI menu plus a worker thread that, every 100 ms, applies the score lock, refreshes the score and retries attaching. All `Trainer` access is guarded by a mutex |

### Adding a feature

1. Subclass `Feature` in `src/feature.cpp` and implement `name()` and `activate()`.
2. For a toggle, also override `isToggle()` and `isEnabled()`. If the feature reads the value field, override `usesInput()`.
3. Register it in `makeFeatures()`.

The menu entry and the next free hotkey (`F1`–`F4`) are created automatically.

## How It Works

1. Loads offsets and the process name from `config.json`
2. Scans `/proc` to find the game's PID
3. Parses `/proc/<pid>/maps` to get the module base address
4. Probes a read at the score offset to confirm memory access
5. Checks that `/proc/<pid>/exe` contains the configured `game_version` (1.0-beta1)
6. Reads and writes the score at `base + offset` with `process_vm_readv` / `process_vm_writev`. While frozen, the score is rewritten whenever it changes

## References

- [BananaBot](https://github.com/ps1337/bananabot)
- [FTXUI](https://github.com/ArthurSonzogni/FTXUI)
