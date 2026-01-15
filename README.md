# NASA APOD Desktop Background

This program automatically downloads the Astronomy Picture of the Day (APOD) from NASA and sets it as your desktop background.

## Requirements for building

### macOS
```bash
brew install curl jsoncpp
```

### Linux (Ubuntu/Debian)
```bash
sudo apt-get install libcurl4-openssl-dev libjsoncpp-dev
```

### Linux (Fedora)
```bash
sudo dnf install libcurl-devel jsoncpp-devel
```

### Windows
- Install MSYS2 or MinGW
- Install curl and jsoncpp libraries

## Build

```bash
make
```

## Configuration

The program reads settings from the `apod-settings.conf` file:

```properties
# Save downloaded images to an archive folder (0 = disabled, 1 = enabled)
save=1

# Auto-run daily (0 = disabled, 1 = enabled, 2 = already configured - ignore)
autorun=1
```

### Available Options:

- **save=0**: Don't save downloaded images
- **save=1**: Automatically save images to `~/apod_archive/` (no duplicates)

- **autorun=0**: Disable automatic configuration
- **autorun=1**: Configure daily execution (at first run)
- **autorun=2**: Already configured (won't reconfigure)

## Usage

### Basic execution (downloads today's APOD)
```bash
./apod-bg
```

### Available Commands

#### Show help
```bash
./apod-bg --help
./apod-bg -h
```

#### Configure image saving
```bash
# Enable image archiving
./apod-bg set save 1

# Disable image archiving
./apod-bg set save 0
```

#### Configure daily auto-run
```bash
# Enable auto-run (configured at first run)
./apod-bg set autorun 1

# Disable auto-run
./apod-bg set autorun 0
```

## First Run

On first run with `autorun=1`:
- **macOS/Linux**: Creates a cron job to run the program daily at 9:00 AM
- **Windows**: Creates a shortcut in the Startup folder

The `apod-settings.conf` file will be automatically updated to `autorun=2` after configuration.

## API Key

The program uses `DEMO_KEY` which has request limits. For frequent use:
1. Visit https://api.nasa.gov/
2. Register for free to get your own API key
3. Replace `DEMO_KEY` in main.cpp with your key

## Features

- ✅ Automatically detects operating system (Windows, macOS, Linux)
- ✅ Downloads HD image when available
- ✅ Archives downloaded images without duplicates
- ✅ Automatically configures daily execution
- ✅ Supports GNOME and KDE on Linux
- ✅ Handles cases where today's APOD is a video (doesn't set it)

## Image Archive

Images are saved to:
- **macOS/Linux**: `~/apod_archive/`
- **Windows**: `%USERPROFILE%\apod_archive\`

The filename is derived from the APOD image title. The program automatically prevents duplicate saves by checking if the file already exists.

## Automation

### macOS/Linux - Crontab
Configured automatically on first run with `autorun=1`. To modify the time:
```bash
crontab -e
# Edit the line:
# 0 9 * * * /full/path/apod-bg
```

### Windows - Task Scheduler
A shortcut is created in the Startup folder.

## Uninstall

### macOS/Linux
Remove the cron job:
```bash
crontab -e
# Delete the line containing apod-bg
```

### Windows
Delete the shortcut from the Startup folder


