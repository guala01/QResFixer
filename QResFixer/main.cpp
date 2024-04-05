#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

void LogMessage(const std::string& message) {
    std::ofstream logFile("log.txt", std::ios::app);
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.close();
    }
}

bool ApplyDisplaySettings(int monitorIndex, int width, int height, int refreshRate) {
    DISPLAY_DEVICE displayDevice = { 0 };
    displayDevice.cb = sizeof(displayDevice);

    if (EnumDisplayDevices(nullptr, monitorIndex - 1, &displayDevice, 0)) {
        std::wstring adapterName = L"\\\\.\\DISPLAY" + std::to_wstring(monitorIndex);
        std::string monitorName = "Monitor " + std::to_string(monitorIndex);

        DEVMODE devMode = { 0 };
        devMode.dmSize = sizeof(devMode);
        devMode.dmPelsWidth = width;
        devMode.dmPelsHeight = height;
        devMode.dmDisplayFrequency = refreshRate;
        devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

        LONG result = ChangeDisplaySettingsEx(adapterName.c_str(), &devMode, nullptr, CDS_UPDATEREGISTRY, nullptr);
        if (result == DISP_CHANGE_SUCCESSFUL) {
            std::string logMessage = "Successfully changed " + monitorName + " resolution to " +
                std::to_string(width) + "x" + std::to_string(height) + " @ " +
                std::to_string(refreshRate) + " Hz.";
            LogMessage(logMessage);
            return true;
        }
        else {
            DWORD errorCode = GetLastError();
            std::vector<wchar_t> errorMessage(MAX_PATH);
            DWORD formatResult = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                errorMessage.data(), errorMessage.size(), nullptr);
            if (formatResult == 0) {
                errorMessage[0] = L'\0';
            }

            std::wstring wideErrorMessage(errorMessage.data());
            std::string logMessage = "Error: Failed to change " + monitorName + " resolution. " +
                std::string(wideErrorMessage.begin(), wideErrorMessage.end());
            LogMessage(logMessage);
        }
    }
    else {
        LogMessage("Error: Invalid monitor index specified in the config file.");
    }

    return false;
}

int main() {
    std::ifstream configFile("config.txt");
    if (!configFile.is_open()) {
        LogMessage("Error: Unable to open config file.");
        return 1;
    }

    int monitorIndex1, width1, height1, refreshRate1;
    int monitorIndex2, width2, height2, refreshRate2;

    if (configFile >> monitorIndex1 >> width1 >> height1 >> refreshRate1 &&
        configFile >> monitorIndex2 >> width2 >> height2 >> refreshRate2) {
        int monitorCount = GetSystemMetrics(SM_CMONITORS);
        if (monitorIndex1 >= 1 && monitorIndex1 <= monitorCount &&
            monitorIndex2 >= 1 && monitorIndex2 <= monitorCount) {
            DEVMODE currentSettings;
            EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &currentSettings);

            if (monitorIndex1 == monitorIndex2) {
                if (currentSettings.dmPelsWidth == width1 && currentSettings.dmPelsHeight == height1 &&
                    currentSettings.dmDisplayFrequency == refreshRate1) {
                    if (ApplyDisplaySettings(monitorIndex2, width2, height2, refreshRate2)) {
                        return 0;
                    }
                }
                else if (currentSettings.dmPelsWidth == width2 && currentSettings.dmPelsHeight == height2 &&
                    currentSettings.dmDisplayFrequency == refreshRate2) {
                    if (ApplyDisplaySettings(monitorIndex1, width1, height1, refreshRate1)) {
                        return 0;
                    }
                }
                else {
                    if (ApplyDisplaySettings(monitorIndex1, width1, height1, refreshRate1)) {
                        return 0;
                    }
                }
            }
            else {
                if (ApplyDisplaySettings(monitorIndex1, width1, height1, refreshRate1) &&
                    ApplyDisplaySettings(monitorIndex2, width2, height2, refreshRate2)) {
                    return 0;
                }
            }
        }
        else {
            LogMessage("Error: Monitor index out of range in the config file.");
        }
    }
    else {
        LogMessage("Error: Invalid configuration format in the config file.");
    }

    configFile.close();
    return 0;
}

