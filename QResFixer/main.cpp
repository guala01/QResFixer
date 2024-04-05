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

int main() {
    std::ifstream configFile("config.txt");
    if (!configFile.is_open()) {
        LogMessage("Error: Unable to open config file.");
        return 1;
    }

    int monitorIndex, width, height, refreshRate;
    if (configFile >> monitorIndex >> width >> height >> refreshRate) {
        int monitorCount = GetSystemMetrics(SM_CMONITORS);
        if (monitorIndex >= 1 && monitorIndex <= monitorCount) {
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
