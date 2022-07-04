#if !defined DEVICEIF_H__
#define DEVICEIF_H__

#pragma once

//#include "DialogStartUpDate.h"
#include "Logger.h"
#include "dialogNoDevice.h"


class dialog_start_update;
class ApplicationFrame;

//
class ProgressData
{
public:
    ApplicationFrame* frame;
};

static constexpr wchar_t wsValidDownLoadFileName[] = L"DK0492.dpi";

struct tPort
{
public:
    tPort(std::string& PortNumber, std::string ConnectedDevice, std::string SerialNumber, std::string BootLoaderVersion, std::string ApplicationVersion) :
        sPortNumber(PortNumber),
        sConnectedDevice(ConnectedDevice),
        sSerialNumber(SerialNumber),
        sBootLoaderVersion(BootLoaderVersion),
        sApplicationVersion(ApplicationVersion)
    {};
    tPort()
    {
        sPortNumber = "";
        sConnectedDevice = "";
        sSerialNumber = "";
        sBootLoaderVersion = "";
        sApplicationVersion = "";
    }
    std::string sPortNumber;
    std::string sConnectedDevice;
    std::string sSerialNumber;
    std::string sBootLoaderVersion;
    std::string sApplicationVersion;
};

void StartLogger(LoggerSettings& settings, std::wstring& wSelectedFile, tPort& selectedPort);

bool FindDevices(std::vector<tPort>& available_devices);
bool StartDownload(dialog_start_update* sud);
std::string GetLoggingPath();
void SetSelectedPort(tPort SelectedPort);
bool ValidateFirmwareFile(std::wstring firmwareFile , std::wstring FullName , std::wstring Path );
std::vector<std::string> split(const std::string& s, char delim);
void SetUSBMode(bool bstatus);
bool GetUSBMode();
void SetStartUpdate(bool bstatus);
bool GetStartUpdate();
void DownloadUSB();
void DownloadSerial();
std::wstring GetTempFolder();
bool DoFirmwareUpdate(ProgressData* pd, bool (*callbackfn)(ProgressData* pdata, int n, std::string message));
DWORD CALLBACK CopyProgressRoutine(LARGE_INTEGER TotalFileSize,
    LARGE_INTEGER TotalBytesTransferred,
    LARGE_INTEGER StreamSize,
    LARGE_INTEGER StreamBytesTransferred,
    DWORD dwStreamNumber,
    DWORD dwCallbackReason,
    HANDLE hSourceFile,
    HANDLE hDestinationFile,
    LPVOID lpData);

#endif // DEVICEIF_H__
