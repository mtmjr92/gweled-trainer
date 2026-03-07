# gweled-hack

External memory hack for [Gweled](https://en.wikipedia.org/wiki/Gweled) (Bejeweled clone) on Linux. Modifies the game's score by reading and writing process memory directly. For educational purposes only.

## Requirements

- Linux (uses `process_vm_readv` / `process_vm_writev`)
- g++ with C++11 or later
- Root privileges or `CAP_SYS_PTRACE` capability
- Gweled installed and running

## Build

```bash
cd Debug
make clean
make
```

The binary is output to `Debug/gweled-hack`.

## Usage

```bash
# Set score to 450 (default)
sudo ./Debug/gweled-hack

# Set score to a custom value
sudo ./Debug/gweled-hack 9999
```

## Project Structure

```
src/
├── main.cpp              # Entry point, CLI argument parsing
├── process_memory.h/cpp  # PID lookup, /proc/maps parsing, memory read/write
├── gweled_hack.h/cpp     # Game-specific logic (score offsets, read/write score)
```

### ProcessMemory

Encapsulates all interaction with the target process:

- Finds the PID by scanning `/proc/*/comm` (no shell commands)
- Parses `/proc/<pid>/maps` to resolve module base addresses
- Provides type-safe `readValue<T>()` / `writeValue<T>()` for memory access via `process_vm_readv` / `process_vm_writev`

### GweledHack

Game-specific layer built on top of `ProcessMemory`:

- Resolves the `gweled` module base address on construction
- Reads and writes the score at known offsets (`0x187E8`, `0x18800`)

## How It Works

1. Scans `/proc` to find the running `gweled` process PID
2. Parses `/proc/<pid>/maps` to get the base address of the `gweled` binary in memory
3. Adds known offsets to the base address to locate the score variables
4. Uses `process_vm_writev` to write the desired score into the game's memory

## References

- [BananaBot](https://github.com/ps1337/bananabot)
