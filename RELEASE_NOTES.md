# NASA APOD Desktop Background - Release v1.0.0

## Overview

Complete release package for the NASA APOD Desktop Background application with support for macOS, Linux, and Windows.

## What's Included

### Directory Structure
```
apod-bg-1.0.0/
├── bin/
│   ├── macos/           # Pre-built macOS binary (ARM64)
│   ├── windows/         # Windows build instructions
│   └── linux/          # Linux build instructions
├── source/             # Source code
│   ├── main.cpp
│   └── Makefile
├── README.md          # Main documentation
├── BUILD_INSTRUCTIONS.md   # Platform-specific build guides
├── apod-settings.conf     # Configuration template
├── Dockerfile.linux   # Linux container build file
└── Dockerfile.windows # Windows container build file
```

## Quick Start

### macOS
```bash
# Extract the release
tar -xzf apod-bg-1.0.0.tar.gz
cd apod-bg-1.0.0

# Copy configuration
cp apod-settings.conf ~/.apod-settings.conf

# Run the binary
./bin/macos/apod-bg-macos-arm64

# Or view help
./bin/macos/apod-bg-macos-arm64 --help
```

### Linux
```bash
# Extract and build from source
tar -xzf apod-bg-1.0.0.tar.gz
cd apod-bg-1.0.0/source

# Install dependencies (Ubuntu/Debian)
sudo apt-get install libcurl4-openssl-dev libjsoncpp-dev

# Compile
g++ -std=c++17 -O3 main.cpp -o apod-bg-linux-x64 -lcurl -ljsoncpp

# Run
./apod-bg-linux-x64
```

### Windows
```bash
# Extract the release
tar -xzf apod-bg-1.0.0.tar.gz
cd apod-bg-1.0.0\source

# Build with MinGW or MSVC
g++ -std=c++17 -O3 main.cpp -o apod-bg.exe -lcurl -ljsoncpp

# Run
apod-bg.exe
```

## Features

✅ **Cross-Platform Support**
- macOS (Apple Silicon & Intel)
- Linux (Ubuntu, Fedora, Debian, etc.)
- Windows (x86_64)

✅ **Automatic Wallpaper Setting**
- Detects OS automatically
- Uses native methods for each platform
- Downloads HD images when available

✅ **Image Archiving**
- Optional automatic archiving
- Prevents duplicate downloads
- Organized by APOD title

✅ **Scheduled Execution**
- Cron-based scheduling on macOS/Linux
- Startup shortcut on Windows
- Configurable settings

✅ **Command-Line Interface**
- Easy configuration
- Help system
- Flexible commands

## Configuration

Edit `apod-settings.conf`:
```properties
# Enable/disable image archiving (0 or 1)
save=1

# Enable/disable automatic daily execution (0 or 1)
autorun=1
```

## Commands

```bash
# Download and set today's APOD
./apod-bg

# Configure image saving
./apod-bg set save 1    # Enable
./apod-bg set save 0    # Disable

# Configure daily auto-run
./apod-bg set autorun 1 # Enable
./apod-bg set autorun 0 # Disable

# Show help
./apod-bg --help
```

## Building from Source

### Requirements
- C++17 capable compiler
- libcurl development library
- jsoncpp library

### macOS
```bash
brew install curl jsoncpp
cd source
make
```

### Linux (Ubuntu/Debian)
```bash
sudo apt-get install libcurl4-openssl-dev libjsoncpp-dev
cd source
g++ -std=c++17 -O3 main.cpp -o apod-bg -lcurl -ljsoncpp
```

### Windows
```bash
# With MinGW
x86_64-w64-mingw32-g++ -std=c++17 -O3 main.cpp -o apod-bg.exe \
    -lcurl -ljsoncpp -static-libgcc -static-libstdc++
```

## Docker Builds

For isolated and reproducible builds:

### Linux
```bash
docker build -f Dockerfile.linux -t apod-bg-linux .
docker run --rm -v $(pwd):/output apod-bg-linux
```

### Windows
```bash
docker build -f Dockerfile.windows -t apod-bg-windows .
docker run --rm -v %cd%:/output apod-bg-windows
```

## Archive Information

Two archive formats provided:
- **apod-bg-1.0.0.tar.gz** (26 KB) - For macOS/Linux
- **apod-bg-1.0.0.zip** (28 KB) - For Windows/cross-platform

Both contain identical content, choose based on your preference.

## Customization

### API Key
Get your own NASA API key from https://api.nasa.gov/

Edit `source/main.cpp` line with `DEMO_KEY` and replace:
```cpp
const std::string API_KEY = "YOUR_API_KEY_HERE";
```

## Troubleshooting

### macOS
- Ensure curl and jsoncpp are installed: `brew install curl jsoncpp`
- Check Finder wallpaper settings for proper integration

### Linux
- GNOME and KDE are supported
- Install required development libraries
- Run with appropriate privileges for system commands

### Windows
- Use MinGW for GCC compilation
- Or install Visual Studio Build Tools for MSVC
- Run with appropriate user permissions

## System Requirements

- **macOS**: 10.15+ (Catalina and later)
- **Linux**: Any modern distribution with glibc 2.29+
- **Windows**: Windows 10 or later

## License and Attribution

This program uses:
- NASA APOD API (public domain)
- libcurl (MIT license)
- jsoncpp (MIT license)

## Support

For issues or questions:
1. Check BUILD_INSTRUCTIONS.md
2. Verify all dependencies are installed
3. Try building from source with `-Wall` flag to see warnings

## Version History

### v1.0.0 (2026-01-15)
- Initial release
- Cross-platform support
- Configuration system
- Image archiving
- Auto-run scheduling
- Command-line interface
