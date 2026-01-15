#include <iostream>
#include <string>
#include <curl/curl.h>
#include <fstream>
#include <cstdlib>
#include <json/json.h>
#include <sstream>
#include <map>
#include <algorithm>
#include <filesystem>
#include <unistd.h>

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
#elif __APPLE__
    #include <TargetConditionals.h>
    #include <mach-o/dyld.h>
#endif

namespace fs = std::filesystem;

// Callback function for CURL to write data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Callback function for CURL to write file
size_t WriteFileCallback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

// Download content to string
std::string downloadToString(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if(res != CURLE_OK) {
            std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
            return "";
        }
    }
    return readBuffer;
}

// Download file
bool downloadFile(const std::string& url, const std::string& outputPath) {
    CURL* curl;
    FILE* fp;
    CURLcode res;
    
    curl = curl_easy_init();
    if(curl) {
        fp = fopen(outputPath.c_str(), "wb");
        if(!fp) {
            std::cerr << "Cannot open file: " << outputPath << std::endl;
            return false;
        }
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
        
        if(res != CURLE_OK) {
            std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
            return false;
        }
        return true;
    }
    return false;
}

bool setWallpaperMacOS(const std::string& imagePath) {
    std::string command = "osascript -e 'tell application \"Finder\" to set desktop picture to POSIX file \"" + imagePath + "\"'";
    int result = system(command.c_str());
    return result == 0;
}

bool setWallpaperLinux(const std::string& imagePath) {
    // Try GNOME first
    std::string command = "gsettings set org.gnome.desktop.background picture-uri 'file://" + imagePath + "'";
    int result = system(command.c_str());
    
    if(result != 0) {
        // Try KDE Plasma
        command = "qdbus org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.evaluateScript 'var allDesktops = desktops();for (i=0;i<allDesktops.length;i++) {d = allDesktops[i];d.wallpaperPlugin = \"org.kde.image\";d.currentConfigGroup = Array(\"Wallpaper\", \"org.kde.image\", \"General\");d.writeConfig(\"Image\", \"file://" + imagePath + "\")}'";
        result = system(command.c_str());
    }
    
    return result == 0;
}

bool setWallpaperWindows(const std::string& imagePath) {
#ifdef _WIN32
    std::wstring wImagePath(imagePath.begin(), imagePath.end());
    return SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (void*)wImagePath.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
#else
    return false;
#endif
}

bool setWallpaper(const std::string& imagePath) {
#ifdef _WIN32
    std::cout << "Detected Windows OS" << std::endl;
    return setWallpaperWindows(imagePath);
#elif __APPLE__
    std::cout << "Detected macOS" << std::endl;
    return setWallpaperMacOS(imagePath);
#elif __linux__
    std::cout << "Detected Linux OS" << std::endl;
    return setWallpaperLinux(imagePath);
#else
    std::cerr << "Unsupported OS" << std::endl;
    return false;
#endif
}

// Parse configuration file
std::map<std::string, std::string> loadConfig(const std::string& configPath) {
    std::map<std::string, std::string> config;
    std::ifstream file(configPath);
    std::string line;
    
    while(std::getline(file, line)) {
        // Skip comments and empty lines
        if(line.empty() || line[0] == '#') continue;
        
        size_t pos = line.find('=');
        if(pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // Remove whitespace
            key.erase(key.find_last_not_of(" \t") + 1);
            key.erase(0, key.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            
            config[key] = value;
        }
    }
    
    return config;
}

// Setup cron job for macOS/Linux
bool setupCronJob(const std::string& programPath) {
    // Get current crontab
    std::string getcronCmd = "crontab -l > /tmp/crontab_backup.txt 2>/dev/null || true";
    system(getcronCmd.c_str());
    
    // Check if job already exists
    std::ifstream crontabFile("/tmp/crontab_backup.txt");
    std::string line;
    while(std::getline(crontabFile, line)) {
        if(line.find(programPath) != std::string::npos) {
            std::cout << "Cron job already exists" << std::endl;
            return true;
        }
    }
    crontabFile.close();
    
    // Add new cron job (daily at 9:00 AM)
    std::string addCronCmd = "echo '0 9 * * * " + programPath + "' >> /tmp/crontab_backup.txt && crontab /tmp/crontab_backup.txt";
    int result = system(addCronCmd.c_str());
    
    return result == 0;
}

// Setup Windows Task Scheduler
bool setupWindowsAutorun(const std::string& programPath) {
#ifdef _WIN32
    // Create a shortcut in startup folder
    char* appdata = getenv("APPDATA");
    std::string startupPath = std::string(appdata) + "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\apod-bg.lnk";
    
    // Use PowerShell to create shortcut
    std::string psCommand = "powershell -Command \"$WshShell = New-Object -ComObject WScript.Shell; $Shortcut = $WshShell.CreateShortcut('" + startupPath + "'); $Shortcut.TargetPath = '" + programPath + "'; $Shortcut.Save()\"";
    
    int result = system(psCommand.c_str());
    return result == 0;
#else
    return false;
#endif
}

// Archive downloaded images
bool archiveImage(const std::string& sourcePath, const std::string& title) {
    char* home = getenv("HOME");
    std::string archivePath = std::string(home) + "/apod_archive";
    
    // Create archive directory if it doesn't exist
    if(!fs::exists(archivePath)) {
        fs::create_directories(archivePath);
    }
    
    // Generate filename from title
    std::string filename = title;
    // Replace spaces with underscores and remove special characters
    for(auto& c : filename) {
        if(!isalnum(c)) c = '_';
    }
    
    // Extract image format from URL
    std::string extension = ".jpg";
    size_t dotPos = sourcePath.rfind('.');
    if(dotPos != std::string::npos) {
        extension = sourcePath.substr(dotPos);
    }
    
    std::string destPath = archivePath + "/" + filename + extension;
    
    // Check for duplicates
    if(fs::exists(destPath)) {
        std::cout << "Image already in archive: " << destPath << std::endl;
        return true;
    }
    
    try {
        fs::copy_file(sourcePath, destPath);
        std::cout << "Image archived to: " << destPath << std::endl;
        return true;
    } catch(const fs::filesystem_error& e) {
        std::cerr << "Failed to archive image: " << e.what() << std::endl;
        return false;
    }
}

// Update configuration file
bool updateConfig(const std::string& configPath, const std::string& key, const std::string& value) {
    std::map<std::string, std::string> config = loadConfig(configPath);
    config[key] = value;
    
    std::ofstream file(configPath);
    file << "# NASA APOD Settings Configuration\n\n";
    file << "# Save downloaded images to an archive folder (0 = disabled, 1 = enabled)\n";
    file << "save=" << config["save"] << "\n\n";
    file << "# Auto-run daily (0 = disabled, 1 = enabled, 2 = already configured - ignore)\n";
    file << "autorun=" << config["autorun"] << "\n";
    
    file.close();
    std::cout << "Configuration updated: " << key << "=" << value << std::endl;
    return true;
}

// Print usage information
void printUsage(const std::string& programName) {
    std::cout << "Usage: " << programName << " [command] [arguments]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  (no arguments)      Download today's APOD and set as wallpaper\n";
    std::cout << "  set save 0/1        Enable (1) or disable (0) image archiving\n";
    std::cout << "  set autorun 0/1     Disable (0) or enable (1) daily auto-run\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << programName << "\n";
    std::cout << "  " << programName << " set save 1\n";
    std::cout << "  " << programName << " set autorun 0\n";
}

int main(int argc, char* argv[]) {
    // Find the config file in the same directory as the executable
    char exePath[PATH_MAX];
    #ifdef _WIN32
        GetModuleFileNameA(NULL, exePath, sizeof(exePath));
    #else
        // Get path of current executable
        uint32_t bufsize = sizeof(exePath);
        if(_NSGetExecutablePath(exePath, &bufsize) != 0) {
            strcpy(exePath, "./apod-bg");
        }
    #endif
    
    std::string exePathStr(exePath);
    size_t lastSlash = exePathStr.find_last_of("/\\");
    std::string exeDir = (lastSlash != std::string::npos) ? exePathStr.substr(0, lastSlash) : ".";
    std::string configPath = exeDir + "/apod-settings.conf";
    std::string programName = exePathStr.substr(lastSlash + 1);
    
    // Handle command-line arguments
    if(argc > 1) {
        std::string command = argv[1];
        
        if(command == "set" && argc == 4) {
            std::string key = argv[2];
            std::string value = argv[3];
            
            if((key == "save" || key == "autorun") && (value == "0" || value == "1")) {
                updateConfig(configPath, key, value);
                return 0;
            } else {
                std::cerr << "Invalid arguments for 'set' command" << std::endl;
                printUsage(programName);
                return 1;
            }
        } else if(command == "--help" || command == "-h") {
            printUsage(programName);
            return 0;
        } else {
            std::cerr << "Unknown command: " << command << std::endl;
            printUsage(programName);
            return 1;
        }
    }
    
    // Load configuration for default behavior (no arguments)
    std::map<std::string, std::string> config = loadConfig(configPath);
    
    int saveSetting = std::stoi(config.count("save") ? config["save"] : "1");
    int autorunSetting = std::stoi(config.count("autorun") ? config["autorun"] : "0");
    
    // NASA API key - get your own from https://api.nasa.gov/
    const std::string API_KEY = "DEMO_KEY"; // Replace with your own API key
    const std::string API_URL = "https://api.nasa.gov/planetary/apod?api_key=" + API_KEY;
    
    std::cout << "Fetching NASA APOD..." << std::endl;
    
    // Get APOD data
    std::string jsonData = downloadToString(API_URL);
    if(jsonData.empty()) {
        std::cerr << "Failed to fetch APOD data" << std::endl;
        return 1;
    }
    
    // Parse JSON
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    std::istringstream jsonStream(jsonData);
    
    if(!Json::parseFromStream(builder, jsonStream, &root, &errors)) {
        std::cerr << "Failed to parse JSON: " << errors << std::endl;
        return 1;
    }
    
    // Check if it's an image (not a video)
    std::string mediaType = root.get("media_type", "").asString();
    if(mediaType != "image") {
        std::cerr << "Today's APOD is not an image (media type: " << mediaType << ")" << std::endl;
        return 1;
    }
    
    std::string imageUrl = root.get("hdurl", root.get("url", "").asString()).asString();
    std::string title = root.get("title", "NASA APOD").asString();
    
    std::cout << "Title: " << title << std::endl;
    std::cout << "Image URL: " << imageUrl << std::endl;
    
    // Determine output path based on OS
    std::string outputPath;
#ifdef _WIN32
    char* userProfile = getenv("USERPROFILE");
    outputPath = std::string(userProfile) + "\\apod_wallpaper.jpg";
#else
    char* home = getenv("HOME");
    outputPath = std::string(home) + "/apod_wallpaper.jpg";
#endif
    
    std::cout << "Downloading image to: " << outputPath << std::endl;
    
    // Download the image
    if(!downloadFile(imageUrl, outputPath)) {
        std::cerr << "Failed to download image" << std::endl;
        return 1;
    }
    
    std::cout << "Image downloaded successfully!" << std::endl;
    std::cout << "Setting as wallpaper..." << std::endl;
    
    // Set as wallpaper
    if(setWallpaper(outputPath)) {
        std::cout << "Wallpaper set successfully!" << std::endl;
    } else {
        std::cerr << "Failed to set wallpaper" << std::endl;
        return 1;
    }
    
    // Archive image if enabled
    if(saveSetting == 1) {
        std::cout << "Archiving image..." << std::endl;
        archiveImage(outputPath, title);
    }
    
    // Setup autorun if enabled and not already configured
    if(autorunSetting == 1) {
        std::cout << "Setting up daily auto-run..." << std::endl;
        
        #ifdef _WIN32
            std::cout << "Setting up Windows autorun..." << std::endl;
            if(setupWindowsAutorun(exePathStr)) {
                std::cout << "Windows autorun configured successfully!" << std::endl;
                // Update config to mark as already configured
                system(("echo # NASA APOD Settings Configuration > " + configPath).c_str());
                system(("echo. >> " + configPath).c_str());
                system(("echo # Save downloaded images to an archive folder (0 = disabled, 1 = enabled) >> " + configPath).c_str());
                system(("echo save=" + std::to_string(saveSetting) + " >> " + configPath).c_str());
                system(("echo. >> " + configPath).c_str());
                system(("echo # Auto-run daily (0 = disabled, 1 = enabled, 2 = already configured - ignore) >> " + configPath).c_str());
                system(("echo autorun=2 >> " + configPath).c_str());
            }
        #else
            std::cout << "Setting up cron job..." << std::endl;
            if(setupCronJob(exePathStr)) {
                std::cout << "Cron job configured successfully!" << std::endl;
                // Update config to mark as already configured
                system(("sed -i.bak 's/autorun=.*/autorun=2/' " + configPath).c_str());
            }
        #endif
    } else if(autorunSetting == 2) {
        std::cout << "Auto-run already configured (autorun=2)" << std::endl;
    }
    
    return 0;
}
