Easy Build Guide
================

Quick overview of build methods for 1776CASH.

Build Methods
-------------

| Script | Type | Best For | Output |
|--------|------|----------|--------|
| `build-depends.sh` | Static/Release | Production, distribution | `src/` |
| `build.sh` | Dynamic/Dev | Development, fast iteration | `build/` |

Use `build-depends.sh` for production wallets.
Use `build.sh` for development work.

Quick Start
-----------

Production build:
```bash
./build-depends.sh
./src/qt/1776cash-qt
```

Development build:
```bash
./build.sh
./build/1776cash-qt
```

Prerequisites
-------------

See OS-specific guides:
- `doc/build-macos.md`
- `doc/build-linux.md`
- `doc/build-windows.md`

Common requirements:
- Git, C++ compiler, CMake
- Autotools (autoconf, automake, libtool)
- Rust (cargo)

Build Options
-------------

Both scripts support:

```bash
--no-gui       # Daemon only (faster)
--debug        # Debug build
--clean        # Clean rebuild
--jobs N       # Parallel jobs (default: auto)
--help         # Show all options
```

CMake Presets
-------------

With CMake 3.14+:

```bash
cmake --preset=vcpkg              # Configure release
cmake --build --preset=vcpkg -j8  # Build

# Available presets:
# vcpkg        - Release with GUI
# vcpkg-debug  - Debug build
# vcpkg-no-gui - Daemon only
```

Build Outputs
-------------

`build-depends.sh` output in `src/`:
```
src/1776cashd        - Daemon
src/1776cash-cli     - CLI client
src/1776cash-tx      - Transaction utility
src/qt/1776cash-qt   - GUI wallet
```

`build.sh` output in `build/`:
```
build/1776cashd      - Daemon
build/1776cash-qt    - GUI wallet (convenience copy)
```

Running
-------

```bash
# GUI wallet
./src/qt/1776cash-qt

# Daemon
./src/1776cashd -daemon
./src/1776cash-cli getblockchaininfo
./src/1776cash-cli stop

# Testnet
./src/1776cashd -testnet -daemon
```

Data Directories
----------------

- macOS: `~/Library/Application Support/1776CASH/`
- Linux: `~/.1776cash/`
- Windows: `%APPDATA%\1776CASH\`

Backup `wallet.dat` - contains private keys.

Troubleshooting
---------------

**"cargo: command not found"**
```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source ~/.cargo/env
```

**Out of memory**
```bash
./build-depends.sh --jobs 2
```

**macOS: "Berkeley DB not found" (build.sh only)**
```bash
brew install berkeley-db@4
```

**Clean rebuild**
```bash
./build-depends.sh --clean
```

Build System Details
--------------------

`build-depends.sh` uses the `depends/` directory - a traditional Bitcoin/PIVX build system that compiles all dependencies from source with static linking. First build takes 30-60 minutes but creates portable, self-contained binaries.

`build.sh` uses Microsoft's vcpkg package manager for faster development builds with dynamic linking.
