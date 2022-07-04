
#include <Windows.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <tchar.h>
#include <vector>
#include <string>
#include <atlstr.h>
#include "FileHandler.h"
#include <filesystem>
#include <chrono>
#include <map>
#include "DeviceIf.h"
#include "version.h"
#include "Logger.h"
#include <strsafe.h>
#include <wx/filename.h>
#include <wx/progdlg.h>
#include "CRCcalculator.h"
#include <ShlObj.h>

#include <sstream>
#include <vector>

#include "DialogStartUpDate.h"

tPort selectedPort;
std::wstring selectedFirmware;
std::wstring File2Download;
DWORD dwDataSize(0);
DCB dcb;

bool UsbEnabled;
bool Startupdate;
// Macros

#define MAX_PORTS_NUMBER (257)
#define COM_PORT_NAME_LENGTH (12) //(7)
#define MAX_DEVICE_RESPONSE_LEN (80)
#define NUM_SET_DEVICE_RETRIES (3)
#define SEND_MESSAGE_DELAY (10)
#define DELAY_READ_RESPONSE (10)

#define STATUS_RESPONSE_LEN (15)
#define STATUS_RESPONSE_IN_WORDS_LEN (256)
#define FIRST_UPGRADE_WAIT_TIME_IN_100_MILI (1400)
#define SECOND_UPGRADE_WAIT_TIME_IN_100_MILI (2000)
#define THIRD_UPGRADE_WAIT_TIME_IN_100_MILI (100)
#define BATTERY_RESPONSE_LEN (15)
#define MIN_BATTERY_LOAD_FOR_UPGRADE (10)
#define CRC_BUFFER_SIZE (4096)
#define MAX_APPLICATION_NAME_SIZE (100)
#define MAX_DATA_BLOCK_SIZE (1536)  //1536
#define MAX_PREFIX_SIZE (200)
#define MAX_MF_COMMAND_LENGTH (MAX_PREFIX_SIZE+ MAX_DATA_BLOCK_SIZE)
#define MAX_DOWNLOAD_STATUS_QUERY_ATTEMPS (20)
#define MIN_CHUNKS_BEFORE_STATUS_QUERY (1)
#define MAX_DOWNLOAD_CHUNK_ATTEMPS (5)
#define CRC_ERROR_RETRIES (3)

//==============================================================================
// Device Status Report
#define MISSING_START_CHARACTER (0x1)
#define UNKNOWN_COMMAND (0x2)
#define INVALID_ARGUMENTS (0x4)
#define INVALID_RESPONSE (0x8)
#define NUMBER_NOT_IN_SEQUENCE (0x20)
#define FLASH_WRITE_ERROR (0x40)
#define BUFFER_SIZE_ERROR (0x80)
#define INVALID_MODE (0x100)
#define FLASH_CRC_ERROR (0x800)
#define BAD_REPLY (0x1000)
#define INVALID_CRC (0x2000)
#define HARDWARE_ERROR (0x4000)
#define COMMAND_FAILED (0x10000)
#define VALIDATION_ERRORS (MISSING_START_CHARACTER | UNKNOWN_COMMAND | INVALID_ARGUMENTS |NUMBER_NOT_IN_SEQUENCE |FLASH_WRITE_ERROR | BUFFER_SIZE_ERROR | INVALID_MODE | FLASH_CRC_ERROR | COMMAND_FAILED)
#define DEVICE_APPLICATION_VERSION_LENGTH (80)
#define UPGRADE_ERRORS (INVALID_MODE | COMMAND_FAILED)



//==============================================================================
// eNums
typedef enum
{
    NO_DRIVE = 0,
    DRIVE_A = 0x1,
    DRIVE_B = 0x2,
    DRIVE_C = 0x4,
    DRIVE_D = 0x8,
    DRIVE_E = 0x10,
    DRIVE_F = 0x20,
    DRIVE_G = 0x40,
    DRIVE_H = 0x80,
    DRIVE_I = 0x100,
    DRIVE_J = 0x200,
    DRIVE_K = 0x400
}eDrive;

typedef enum
{
    INSUFFICIENT_BATTERY_CHARGE,
    FLASH_WRITE_ERR,
    DEVICE_HARDWARE_ERROR,
    DEVICE_BUSY,
    CAUSES_NUM
} eCause;

typedef enum
{
    E_UPGRADE_IDLE,
    E_UPGRADE_VALIDATING,
    E_UPGRADE_VALIDATED,
    E_UPGRADE_PREPARING,
    E_UPGRADE_UPGRADING,
    E_UPGRADE_ERROR_DEVICE_BUSY,
    E_UPGRADE_ERROR_BATTERY_TOO_LOW,
    E_UPGRADE_ERROR_INVALID_BOOTLOADER,
    E_UPGRADE_ERROR_INVALID_OPTION_BYTES,
    E_UPGRADE_ERROR_FILE_NOT_FOUND,
    E_UPGRADE_ERROR_FILE_HEADER_INVALID,
    E_UPGRADE_ERROR_FILE_SIZE_INVALID,
    E_UPGRADE_ERROR_API_FAIL,
    E_UPGRADE_ERROR_ERASE_FAIL,
    E_UPGRADE_ERROR_DATA_READ_FAIL,
    E_UPGRADE_ERROR_DATA_WRITE_FAIL,
    E_UPGRADE_STATUS_RESPONSES_NUM
} eUpgradeStatus_t;


//=========================================================
// Device Commands
static constexpr char sCreateSerialFile[] = "#mp9=SerialFile,1,";
static constexpr char sFlipToMassStorage[] = "#TU0=";
static constexpr char sDeviceSetRemote[] = "#KM=R\r\n";
static constexpr char sDeviceSetLocal[] = "#KM=L\r\n";
static constexpr char sDeviceProbe[] = "#RI?\r\n";
static constexpr char sDeviceSNprobe[] = "#SN?\r\n";
static constexpr char sDeviceAppVersionProbe[] = "#RV0?\r\n";
static constexpr char sDeviceBLVersionProbe[] = "#RV1?\r\n";
static constexpr char sDeviceStatusProbe[] = "#RE?\r\n";
//static constexpr char sDeviceSetMassStorage[] = "#SC0=1\r\n";
static constexpr char sDeviceSetMassStorage[] = "#SX\r\n";
static constexpr char sDeviceUpgradeFirmware[] = "#UF\r\n";
static constexpr char sDevicePinProtection[] = "#PP=";
static constexpr char sPPforMassStorage[] = "000\r\n";
static constexpr char sPPforUpgrade[] = "5487\r\n";
static constexpr char sBatteryPercentageStateProbe[] = "#RB3?\r\n";
static constexpr char sDeviceUpgradeStatus[] = "#UF?\r\n";

//======================================================

// Device Responses
static constexpr char sDeviceIdResponse[] = "!RI=";
static constexpr char sDeviceSNResponse[] = "!SN=";
static constexpr char sDeviceAppVerResponse[] = "!RV0=";
static constexpr char sDeviceBLVerResponse[] = "!RV1=";
static constexpr char sDeviceStatusResponse[] = "!RE=";
static constexpr char sBatteryPercentageStateResponse[] = "!RB3=";
static constexpr char sDeviceUpgradeStatusResponse[] = "!UF=";

//======================================================
// Globals
//======================================================
static const std::map< eDrive, char> mDrives{
    { DRIVE_A, 'A'},
    { DRIVE_B, 'B'},
    { DRIVE_C, 'C'},
    { DRIVE_D, 'D'},
    { DRIVE_E, 'E'},
    { DRIVE_F, 'F'},
    { DRIVE_G, 'G'},
    { DRIVE_H, 'H'},
    { DRIVE_I, 'I'},
    { DRIVE_J, 'J'},
    { DRIVE_K, 'K'}
};

static const char* cCauseReason[CAUSES_NUM]
{
    "Insufficient battery",
    "Flush write error",
    "Hardware error",
    "Device busy"
};

static const char* cUpgradeStatus[E_UPGRADE_STATUS_RESPONSES_NUM] =
{
    "Idle",
    "Validating",
    "Validated",
    "Preparing",
    "Upgrading",
    "Device busy",
    "Battery too low",
    "Invalid bootloader",
    "Invalid option bytes",
    "File not found",
    "File header invalid",
    "File size invalid",
    "API fail",
    "Erase fail",
    "Data read fail",
    "Data write fail"
};
///////////////////////////////////////////////////////////////
// 
// Module local variables
//
///////////////////////////////////////////////////////////////
static char sDeviceResponse[MAX_DEVICE_RESPONSE_LEN];
static char cApplicationName[80] = { 0 };
static BYTE DataBuffer[MAX_DATA_BLOCK_SIZE] = { 0 };
static char cFile2Download[MAX_PATH_LENGTH] = { 0 };
static char cDownloadDir[MAX_PATH_LENGTH] = { 0 };
static char cDownloadFileName[MAX_PATH_LENGTH] = { 0 };
static char cMFCommand[MAX_MF_COMMAND_LENGTH] = { 0 };

static const wchar_t wsReceivedFileExtension[] = L".dpi";
static const wchar_t wsOutFileExtension[] = L".raw";
static const char sStartOfData[] = "[DATA]";

static  COMMTIMEOUTS NormalCommsTimeOut{ MAXDWORD, 0, 0, 100, 0 };
static  COMMTIMEOUTS DownloadCommsTimeOut{ MAXDWORD, 0, 0, 10, 0 };

typedef struct
{
    std::string sVersion;
    DWORD crc;

}tDPI610eHeader;

//////////////////////////////////////////////////////////////////////////
// 
//  Internal Function Declarations 
//
////////////////////////////////////////////////////////////////////////

std::string GetLastErrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return {}; //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}

void CloseComPort(HANDLE& hCom)
{
    //SendCommand(hCom, sDeviceSetLocal, &NormalCommsTimeOut);
    CloseHandle(hCom);
}
////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 
/// </summary>
/// <param name="uStatusResponse"></param>
/// <param name="sOutErrorMessage"></param>
/// <returns> None
/// ////////////////////////////////////////////////////////////////////////

static void StatusResponse2String(size_t uStatusResponse, std::string& sOutErrorMessage)
{
    std::string sErrorMessage;

    if (uStatusResponse & MISSING_START_CHARACTER)
    {
        sErrorMessage += std::string("Missing start character,");
    }
    if (uStatusResponse & UNKNOWN_COMMAND)
    {
        sErrorMessage += std::string("Invalid command received,");
    }
    if (uStatusResponse & INVALID_ARGUMENTS)
    {
        sErrorMessage += std::string("Invalid arguments received,");
    }
    if (uStatusResponse & NUMBER_NOT_IN_SEQUENCE)
    {
        sErrorMessage += std::string("Last block's index is out of sequence,");
    }
    if (uStatusResponse & FLASH_WRITE_ERROR)
    {
        sErrorMessage += std::string("Error writing to flash,");
    }
    if (uStatusResponse & BUFFER_SIZE_ERROR)
    {
        sErrorMessage += std::string("Invalid buffer size,");
    }
    if (uStatusResponse & INVALID_MODE)
    {
        sErrorMessage += std::string("File already exists on the device's file system,");
    }
    if (uStatusResponse & FLASH_CRC_ERROR)
    {
        sErrorMessage += std::string("Block's CRC error,");
    }
    if (uStatusResponse & COMMAND_FAILED)
    {
        sErrorMessage += std::string("First block's index >1,");
    }
    sOutErrorMessage = sErrorMessage;
}

void FindActiveComPorts(std::vector<tPort>& PortsList)
{
    HANDLE hCom;
    TCHAR tComPortName[COM_PORT_NAME_LENGTH * 2];

    memset(tComPortName, 0, sizeof(tComPortName));

    for (int i = 1; i < MAX_PORTS_NUMBER; ++i)
    {
       swprintf_s((wchar_t*)tComPortName, COM_PORT_NAME_LENGTH, L"\\\\.\\COM%d", i);

        //  Open a handle to the specified com port.
        hCom = CreateFile(tComPortName,
            GENERIC_READ | GENERIC_WRITE,
            0,      //  must be opened with exclusive-access
            NULL,   //  default security attributes
            OPEN_EXISTING, //  must use OPEN_EXISTING
            0,      //  not overlapped I/O
            NULL); //  hTemplate must be NULL for comm devices

        if (hCom != INVALID_HANDLE_VALUE)
        {
            // Convert the port name to string and add it to PortsList
#ifdef UNICODE

            char cComPortName[COM_PORT_NAME_LENGTH];

            WideCharToMultiByte(CP_UTF8, 0, tComPortName, -1, &cComPortName[0], COM_PORT_NAME_LENGTH, NULL, NULL);

            std::string sComPortName(&cComPortName[0]);
#else
            std::string sComPortName(tComPortName);
#endif
            auto pPort = new tPort();

            pPort->sPortNumber = sComPortName;

            PortsList.push_back(*pPort);

            CloseComPort(hCom);

            delete pPort;
        }        
    }
}

static void PrintCommState(DCB dcb)
{
    //  Print some of the DCB structure values
    _tprintf(TEXT("\r\n BaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\n"),
        dcb.BaudRate,
        dcb.ByteSize,
        dcb.Parity,
        dcb.StopBits);
}

static bool SendCommand(HANDLE& hCom, const char* sMessage, COMMTIMEOUTS* pCommsTimeOut = nullptr)
{
    DWORD dNoOfBytesWritten(0);
    BOOL bSuccess(FALSE);

    if (nullptr != pCommsTimeOut)
    {
        bSuccess = SetCommTimeouts(hCom, pCommsTimeOut);

        if (!bSuccess)
        {
            _tprintf(TEXT("\r\n Failed in setting the device's communication timeouts \r\n"));
            return FALSE;
        }
    }

    int retries = NUM_SET_DEVICE_RETRIES;
    do
    {
        // Supress warning C4267  
#pragma warning(push)
#pragma warning(disable:4267)
        bSuccess = WriteFile(hCom,        // Handle to the Serial port
            sMessage,     // Data to be written to the port
            strlen(sMessage),
            &dNoOfBytesWritten, //Bytes written
            NULL);
#pragma warning (pop)
        Sleep(SEND_MESSAGE_DELAY);

    } while (!bSuccess && --retries);

    thislogger << LoggerPrefix(Info) << "SendCommand: command (" << sMessage << ") was sent " << ((bSuccess) ? "successfully" : "unsuccessfully") << std::endl;

    if (!bSuccess)
    {
        thislogger << LoggerPrefix(Error) << "SendCommand: reported send error is " << GetLastErrorAsString() << std::endl;
    }
    return bSuccess;
}

static BOOL SendData(HANDLE& hCom, const char* sMessage, DWORD dwDataLength)
{
    DWORD dNoOfBytesWritten(0);
    BOOL bSuccess(FALSE);

    int retries = NUM_SET_DEVICE_RETRIES;
    do
    {
        bSuccess = WriteFile(hCom,        // Handle to the Serial port
            sMessage,     // Data to be written to the port
            dwDataLength,
            &dNoOfBytesWritten, //Bytes written
            NULL);
        Sleep(SEND_MESSAGE_DELAY);
        --retries;
    } while (!bSuccess && retries);

    if (dNoOfBytesWritten == strlen(sMessage))
    {
        thislogger << LoggerPrefix(Info) << "SendData: command (" << sMessage << ") of " << dNoOfBytesWritten << " bytes was sent " << ((bSuccess) ? "successfully" : "unsuccessfully") << std::endl;
    }
    else
    {
        thislogger << LoggerPrefix(Info) << "SendData: command (";

        for (size_t i = 0; i < dNoOfBytesWritten; ++i)
        {
            thislogger << sMessage[i];
        }
        thislogger << ") of " << dNoOfBytesWritten << " bytes was sent " << ((bSuccess) ? "successfully" : "unsuccessfully") << std::endl;
    }

    if (!bSuccess)
    {
        thislogger << LoggerPrefix(Error) << "SendData: reported send error is " << GetLastErrorAsString() << std::endl;
    }

    return bSuccess;
}

static size_t ReceiveDeviceResponse(HANDLE& hCom, char* inBuffer, COMMTIMEOUTS* pCommsTimeOut = nullptr)
{
    size_t index = 0;
    char ReadData;
    BOOL bSuccess(FALSE);

    if (nullptr != pCommsTimeOut)
    {
        bSuccess = SetCommTimeouts(hCom, pCommsTimeOut);

        if (!bSuccess)
        {
            _tprintf(TEXT("\r\n Failed in setting the device's communication timeouts \r\n"));
            return 0;
        }
    }

    DWORD dNoBytesRead(0);
    do
    {
        Sleep(DELAY_READ_RESPONSE);
        bSuccess = ReadFile(hCom, &ReadData, sizeof(ReadData), &dNoBytesRead, NULL);
        if (dNoBytesRead > 0)
        {
            inBuffer[index] = ReadData;
            ++index;
        }
    } while ((dNoBytesRead > 0) && ('\n' != ReadData));

    return index;
}

static BOOL ReadDeviceResponse(HANDLE& hCom, char* inBuffer, const char* sExpectedResponse, std::string& sOutParam)
{
    BOOL bSuccess = FALSE;

    memset(sDeviceResponse, 0, sizeof(sDeviceResponse));

    size_t NumOfBytesReceived = ReceiveDeviceResponse(hCom, sDeviceResponse);

    if (NumOfBytesReceived > 0)
    {
        if (0 == (strncmp(sDeviceResponse, sExpectedResponse, strlen(sExpectedResponse))))
        {
            std::string sTemp(&sDeviceResponse[strlen(sExpectedResponse)], strlen(sDeviceResponse) + 1);
            sOutParam = sTemp.substr(0, sTemp.find_first_of('\r'));
            bSuccess = TRUE;
        }
    }

    thislogger << LoggerPrefix(Info) << "ReadDeviceResponse: received " << sOutParam << " from the device" << std::endl;
    return bSuccess;
}

static BOOL ConfigureComPort(tPort& port, DCB& dcb)
{
    HANDLE hCom;
    TCHAR tComPortName[COM_PORT_NAME_LENGTH];
    int retries = NUM_SET_DEVICE_RETRIES;
    DWORD dNoOfBytesWritten(0);
    BOOL bSuccess(FALSE);
    _tcscpy_s(tComPortName, CA2T(port.sPortNumber.c_str()));

    //  Open a handle to the specified com port.
    hCom = CreateFile(tComPortName,
        GENERIC_READ | GENERIC_WRITE,
        0,      //  must be opened with exclusive-access
        NULL,   //  default security attributes
        OPEN_EXISTING, //  must use OPEN_EXISTING
        0,      //  not overlapped I/O
        NULL); //  hTemplate must be NULL for comm devices

    if (hCom != INVALID_HANDLE_VALUE)
    {
        if (!SetCommState(hCom, &dcb))
        {
            //_tprintf(TEXT("\r\n Failed in setting the comms state \r\n"));
            thislogger << LoggerPrefix(Error) << "Failed in setting the comms state" << std::endl;
            CloseHandle(hCom);
            return FALSE;
        }

       // if (!SendCommand(hCom, sDeviceSetLocal, &NormalCommsTimeOut))
       // {
       //     _tprintf(TEXT("\r\n Failed in transferring the device to remote mode \r\n"));
     
       //thislogger << LoggerPrefix(Error) << "Failed in transferring the device to remote mode" << std::endl;
       //     CloseHandle(hCom);
       //     return FALSE;
       // }
        
        if (!SendCommand(hCom, sDeviceSetRemote, &NormalCommsTimeOut))
        {
            //_tprintf(TEXT("\r\n Failed in transferring the device to remote mode \r\n"));
            thislogger << LoggerPrefix(Error) << "Failed in transferring the device to remote mode" << std::endl;
            CloseHandle(hCom);
            return FALSE;
        }

        if (!SendCommand(hCom, sDeviceProbe))
        {
            //_tprintf(TEXT("\r\n Failed in sending a device id query \r\n"));
            thislogger << LoggerPrefix(Error) << "Failed in sending a device id query" << std::endl;
            CloseComPort(hCom);
            return FALSE;
        }

        if (ReadDeviceResponse(hCom, sDeviceResponse, sDeviceIdResponse, port.sConnectedDevice))
        {

            if (!SendCommand(hCom, sDeviceSNprobe))
            {
                //_tprintf(TEXT("\r\n Failed in sending a device serial number query \r\n"));
                thislogger << LoggerPrefix(Error) << "Failed in sending a device serial number query" << std::endl;
                port.sSerialNumber = std::string("UNKNOWN");
                CloseComPort(hCom);
                return FALSE;
            }

            if (!ReadDeviceResponse(hCom, sDeviceResponse, sDeviceSNResponse, port.sSerialNumber))
            {
                //_tprintf(TEXT("\r\n Failed in reading device's application version \r\n"));
                thislogger << LoggerPrefix(Error) << "Failed in reading device's application version" << std::endl;
                CloseComPort(hCom);
                return FALSE;
            }
            // Query device for its application version

            if (!SendCommand(hCom, sDeviceAppVersionProbe))
            {
                //_tprintf(TEXT("\r\n Failed in sending a device's application version query \r\n"));
                thislogger << LoggerPrefix(Error) << "Failed in sending a device's application version query" << std::endl;
                CloseComPort(hCom);
                return FALSE;
            }

            if (!ReadDeviceResponse(hCom, sDeviceResponse, sDeviceAppVerResponse, port.sApplicationVersion))
            {
                //_tprintf(TEXT("\r\n Failed in reading device's application version \r\n"));
                thislogger << LoggerPrefix(Error) << "Failed in reading device's application version" << std::endl;
                CloseComPort(hCom);
                return FALSE;
            }

            // Query device for its Bootloader version
            if (!SendCommand(hCom, sDeviceBLVersionProbe))
            {
                //_tprintf(TEXT("\r\n Failed in sending a device's bootloader version query \r\n"));
                thislogger << LoggerPrefix(Error) << "Failed in sending a device's bootloader version query" << std::endl;
                CloseComPort(hCom);
                return FALSE;
            }

            if (!ReadDeviceResponse(hCom, sDeviceResponse, sDeviceBLVerResponse, port.sBootLoaderVersion))
            {
                //_tprintf(TEXT("\r\n Failed in reading device's bootloader version \r\n"));
                thislogger << LoggerPrefix(Error) << "Failed in reading device's bootloader version" << std::endl;
                CloseComPort(hCom);
                return FALSE;
            }
            
            if (UsbEnabled)
            {
                if (!SendCommand(hCom, (LPCUWSTR)sCreateSerialFile + port.sSerialNumber + "\r\n", &NormalCommsTimeOut))
                {
                    _tprintf(TEXT("\r\n Failed Creating Serial File \r\n"));
                    thislogger << LoggerPrefix(Error) << "Failed Creating Serial File" << std::endl;
                    CloseHandle(hCom);
                    return FALSE;
                }
            }

            bSuccess = TRUE;
        }
        CloseComPort(hCom);
    }
    return bSuccess;
}

static BOOL SendNextChunk(HANDLE& hCom, const char* cDownloadFileName, const size_t index, DWORD crc, BYTE* DataBuffer, DWORD dwNumberOfBytesRead, size_t ChunksBeforeStatus, unsigned short& uStatusResponse)
{
    BOOL bSuccess(FALSE);

    memset(cMFCommand, 0, sizeof(cMFCommand));

    sprintf_s(cMFCommand, sizeof(cMFCommand), "#MF9=%s,%u,%lu,%u,", cDownloadFileName, (unsigned int)index, crc, dwNumberOfBytesRead);

    char* endOfCommand = strchr(cMFCommand, '\0');

    memcpy(endOfCommand, DataBuffer, dwNumberOfBytesRead);

    strcpy_s(endOfCommand + dwNumberOfBytesRead, sizeof(cMFCommand) - dwNumberOfBytesRead, "\r\n");

    size_t retries = MAX_DOWNLOAD_CHUNK_ATTEMPS;

    do
    {
        bSuccess = SendData(hCom, cMFCommand, static_cast<DWORD>((endOfCommand - cMFCommand) + dwNumberOfBytesRead + strlen("\r\n")));
        if (!bSuccess)
        {
            thislogger << LoggerPrefix(Error) << "Sendata Error" << std::endl;
            Sleep(100);
        }

    } while ((!bSuccess) && (--retries));

    if (!bSuccess)
    {
        thislogger << LoggerPrefix(Error) << "Failed in sending data chunck" << std::endl;
        // Query the device for last block's reception status
//#define HEATHER_DEBUGGING
#ifdef HEATHER_DEBUGGING
        thislogger << LoggerPrefix(Error) << "should not run" << std::endl;
        Sleep(1000);
#endif
        if (!SendCommand(hCom, sDeviceStatusProbe))
        {
            thislogger << LoggerPrefix(Error) << "Failed in sending device status probe" << std::endl;
            return FALSE;
        }

        std::string sStatusResponse(STATUS_RESPONSE_LEN, 0);

        if (!ReadDeviceResponse(hCom, sDeviceResponse, sDeviceStatusResponse, sStatusResponse))
        {
            thislogger << LoggerPrefix(Error) << "Failed in reading device's status" << std::endl;
            return(FALSE);
        }
        thislogger << LoggerPrefix(Info) << " device's status is " << sStatusResponse << std::endl;
        return(FALSE);
    }
#define STATUS_QUERY
#ifdef STATUS_QUERY

    //    if (0 == (index % ChunksBeforeStatus))
    {
        thislogger << LoggerPrefix(Info) << "Sent " << index << " blocks of " << dwNumberOfBytesRead << " bytes" << std::endl;

        retries = MAX_DOWNLOAD_STATUS_QUERY_ATTEMPS;
        std::string sStatusResponse(STATUS_RESPONSE_LEN, 0);
        Sleep(15);
        do
        {
            // Query the device for last block's reception status
            if (!SendCommand(hCom, sDeviceStatusProbe))
            {
                thislogger << LoggerPrefix(Error) << "Failed in sending device status probe" << std::endl;
                return FALSE;
            }

            bSuccess = ReadDeviceResponse(hCom, sDeviceResponse, sDeviceStatusResponse, sStatusResponse);
            //if (!bSuccess)
            //{
            //    Sleep(10);
            //}

        } while ((!bSuccess) && (--retries));

        if (bSuccess)
        {
            std::string sHexStatusResponse = std::string("0x") + sStatusResponse;
            uStatusResponse = std::stoi(sHexStatusResponse, 0, 16);

            if (0 != (uStatusResponse & VALIDATION_ERRORS))
            {
                std::string sStatusResponseInWords(STATUS_RESPONSE_IN_WORDS_LEN, 0);
                StatusResponse2String(uStatusResponse, sStatusResponseInWords);
                thislogger << LoggerPrefix(Error) << "Block transfer failed, reported error: " << sStatusResponseInWords << std::endl;
#define HEATHER_DEBUGGING
#ifdef HEATHER_DEBUGGING
                HANDLE hOutFile = CreateFile(TEXT("failed_buffer.hex"),                // name of the write
                    GENERIC_WRITE,          // open for writing
                    0,                      // do not share
                    NULL,                   // default security
                    CREATE_NEW,             // create new file only
                    FILE_ATTRIBUTE_NORMAL,  // normal file
                    NULL);

                bSuccess = WriteFile(
                    hOutFile,
                    DataBuffer,
                    dwNumberOfBytesRead,
                    NULL,
                    NULL
                );

                CloseHandle(hOutFile);
#endif
                return(FALSE);
            }
        }
        else
        {
            thislogger << LoggerPrefix(Error) << "Failed in reading device's status" << std::endl;
        }
    }
#endif

    return bSuccess;
}

static BOOL DownLoadFileVCP(HANDLE& hCom, std::wstring tFileName, const DWORD dwDataSize , dialog_start_update* sud)
{
    BOOL bSuccess(FALSE);
    // Verify arguments integrity
    if (tFileName.empty())
    {
        thislogger << LoggerPrefix(Error) << "Empty download file path" << std::endl;
        CloseComPort(hCom);
        return FALSE;
    }
    char cDrive[4] = { 0 };
    char cExt[6] = { 0 };
    memset(cDownloadFileName, 0, sizeof(cDownloadFileName));




#ifdef UNICODE

    std::wstring fileName1 = std::wstring(tFileName.begin(), tFileName.end());
    LPCWSTR fn1 = fileName1.c_str();

 

    int nConvertedChars = WideCharToMultiByte(CP_ACP, 0, fn1, -1, cFile2Download, sizeof(cFile2Download), NULL, NULL);
    errno_t  Result = _splitpath_s(cFile2Download, cDrive, sizeof(cDrive), cDownloadDir, sizeof(cDownloadDir), cDownloadFileName, sizeof(cDownloadFileName), cExt, sizeof(cExt));
#else
    errno_t  Result = _splitpath_s(tFileName, cDrive, sizeof(cDrive), cDownloadDir, sizeof(cDownloadDir), cDownloadFileName, sizeof(cDownloadFileName), cExt, sizeof(cExt));
#endif

    std::wstring fileName = std::wstring(tFileName.begin(), tFileName.end());
    LPCWSTR fn = fileName.c_str();


    // Open file to download
    HANDLE hFile = CreateFile(fileName.c_str(),               // file to open
        GENERIC_READ,          // open for reading
        FILE_SHARE_READ,       // share for reading
        NULL,                  // default security
        OPEN_EXISTING,         // existing file only
        FILE_ATTRIBUTE_NORMAL, // normal file
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        thislogger << LoggerPrefix(Error) << "Couldn't open download file(" << cFile2Download << "." << cExt << ")" << std::endl;
        CloseComPort(hCom);
        return FALSE;
    }

    strcpy_s(&cDownloadFileName[strlen(cDownloadFileName)], sizeof(cDownloadFileName) - strlen(cDownloadFileName), cExt);

    DWORD dwNumberOfBytesRead(0);

    CRCcalculator crc;

    size_t index = 1;

    DWORD dwRemainingDataBytes = dwDataSize;

    DWORD dwBlockSize = 1536; //1536
    BOOL bWrongInput(FALSE);

    DWORD dwInterBlocksDelay(10); //50

    bSuccess = SetCommTimeouts(hCom, &DownloadCommsTimeOut);

    if (!bSuccess)
    {
        thislogger << LoggerPrefix(Error) << "Couldn't set Comms timeout" << std::endl;
        CloseHandle(hFile);
        CloseComPort(hCom);
        return FALSE;
    }

    // Verify device's ready status before the download
    if (!SendCommand(hCom, sDeviceStatusProbe))
    {
        thislogger << LoggerPrefix(Error) << "Failed in sending device status probe" << std::endl;
        CloseHandle(hFile);
        CloseComPort(hCom);
        return FALSE;
    }

    std::string sStatusResponse(STATUS_RESPONSE_LEN, 0);

    if (!ReadDeviceResponse(hCom, sDeviceResponse, sDeviceStatusResponse, sStatusResponse))
    {
        thislogger << LoggerPrefix(Error) << "Failed in reading device's status" << std::endl;
        CloseHandle(hFile);
        CloseComPort(hCom);
        return(FALSE);
    }

    std::string sHexStatusResponse = std::string("0x") + sStatusResponse;
    unsigned short uStatusResponse = std::stoi(sHexStatusResponse, 0, 16);

    if (0 != (uStatusResponse & HARDWARE_ERROR))
    {
        std::string sStatusResponseInWords(STATUS_RESPONSE_IN_WORDS_LEN, 0);
        thislogger << LoggerPrefix(Error) << "Selected device has a hardware error " << std::endl;
        CloseHandle(hFile);
        CloseComPort(hCom);
        return(FALSE);
    }

    // Delete copies of the downloaded file from the device's file system in case they exist
    std::string sEraseCommand("#ME9=");

    sEraseCommand += std::string(cDownloadFileName) + std::string("\r\n");

    if (!SendCommand(hCom, sEraseCommand.c_str()))
    {
        thislogger << LoggerPrefix(Error) << "Failed in sending erase file command " << std::endl;
        CloseHandle(hFile);
        CloseComPort(hCom);
        return(FALSE);
    }

    Sleep(1000);

    if (!SendCommand(hCom, sDeviceStatusProbe))
    {
        thislogger << LoggerPrefix(Error) << "Failed in sending device status query command " << std::endl;
        CloseHandle(hFile);
        CloseComPort(hCom);
        return(FALSE);
    }

    if (!ReadDeviceResponse(hCom, sDeviceResponse, sDeviceStatusResponse, sStatusResponse))
    {
        thislogger << LoggerPrefix(Error) << "Failed in reading device's status" << std::endl;
        CloseHandle(hFile);
        CloseComPort(hCom);
        return(FALSE);
    }

    sHexStatusResponse = std::string("0x") + sStatusResponse;
    uStatusResponse = std::stoi(sHexStatusResponse, 0, 16);

    if (uStatusResponse & INVALID_RESPONSE)
    {
        thislogger << LoggerPrefix(Error) << "Failed in erasing existing copy from device's file system" << std::endl;
        CloseHandle(hFile);
        CloseComPort(hCom);
        return(FALSE);
    }

    DWORD ChunksBeforeStatus = (MAX_DATA_BLOCK_SIZE / dwBlockSize) * MIN_CHUNKS_BEFORE_STATUS_QUERY;
    thislogger << LoggerPrefix(Info) << "Downloading file started" << std::endl;
    thislogger << LoggerPrefix(Info) << "Block size is:" << dwBlockSize << " bytes, inter-block delay is:" << dwInterBlocksDelay << " mili" << std::endl;

    _tprintf(TEXT("\r\n Downloading file"));
#define MEASURE_DOWNLOAD_TIME
#ifdef MEASURE_DOWNLOAD_TIME
    // Start measuring time
    auto begin = std::chrono::high_resolution_clock::now();
#endif  

   // wxProgressDialog dialog("DPI FIRMWARE UPDATE TOOL", "Uploading Firmware", dwBlockSize + 5, NULL, wxPD_APP_MODAL);

    sud->m_gauge1->SetRange(dwBlockSize + 5);

    do
    {
        memset(DataBuffer, 0, sizeof(DataBuffer));
        bSuccess = ReadFile(hFile, DataBuffer, dwBlockSize, &dwNumberOfBytesRead, nullptr);
        if (dwNumberOfBytesRead > 0)
        {

            DWORD dwChunkCRC = crc.crc32(DataBuffer, dwNumberOfBytesRead, 0);

            unsigned short uStatusReport;

            size_t retries = CRC_ERROR_RETRIES;

            do
            {
                Sleep(10);
                bSuccess = SendNextChunk(hCom, cDownloadFileName, index, dwChunkCRC, DataBuffer, dwNumberOfBytesRead, ChunksBeforeStatus, uStatusReport);

            } while ((0 != (uStatusReport & VALIDATION_ERRORS)) && (--retries));

            Sleep(dwInterBlocksDelay);

            if (!bSuccess)
            {
                thislogger << LoggerPrefix(Error) << "Sending chunk #" << index << " with size " << dwBlockSize << " bytes failed" << std::endl;
            }
            ++index;
            dwRemainingDataBytes -= dwNumberOfBytesRead;


            sud->m_gauge1->SetValue(index);

        }

        wxYield();

    } while ((bSuccess) && (dwNumberOfBytesRead > 0) && (dwRemainingDataBytes > 0));


    _tprintf(TEXT("\r\n"));
#ifdef MEASURE_DOWNLOAD_TIME
    // Stop measuring time and calculate the elapsed time
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);

    //printf("Result: %.20f\n", sum);

    _tprintf(TEXT("Time measured for download: %.3f seconds.\n"), elapsed.count() * 1e-9);

#endif    
    std::string sDownLoadResult = (dwRemainingDataBytes == 0) ? "successfully" : "in failure";
    thislogger << LoggerPrefix(Info) << "Downloading file ended " << std::endl;

    CloseHandle(hFile);
    CloseComPort(hCom);

    //dialog.Close();

    return (dwRemainingDataBytes == 0);
}

void GetComPortHandle(HANDLE& hCom, TCHAR tComPortName[12])
{
	hCom = CreateFile(tComPortName,
	                  GENERIC_READ | GENERIC_WRITE,
	                  0,      //  must be opened with exclusive-access
	                  NULL,   //  default security attributes
	                  OPEN_EXISTING, //  must use OPEN_EXISTING
	                  0,      //  not overlapped I/O
	                  NULL); //  hTemplate must be NULL for comm devices
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// download : Downloads the selected file to the selected device
/// </summary>
/// <param name="port"></param>
/// <param name="dcb"></param>
/// <param name="tFileName"></param>
/// <param name="dwDataSize"></param>
/// <returns> true if download succeeded</returns>
//////////////////////////////////////////////////////////////////////////////////////////////////////
bool download(tPort& port, DCB& dcb, std::wstring tFileName, const DWORD dwDataSize , dialog_start_update* sud)
{
    HANDLE hCom;
    TCHAR tComPortName[COM_PORT_NAME_LENGTH];

    // Check that we are still connected to the right device

    thislogger << LoggerPrefix(Info) << "\r\n Verifying correct device connection: \r\n" << std::endl;
    BOOL bSuccess = ConfigureComPort(port, dcb);

    if (!bSuccess)
    {
        dcb.BaudRate = CBR_19200;
        bSuccess = ConfigureComPort(port, dcb);
    }

    if (!bSuccess)
    {
        thislogger << LoggerPrefix(Error) << " Couldn't read connected device's details " << std::endl;
        return FALSE;
    }

    _tprintf(TEXT("  %hs is connected to:%hs, Serial Number:%hs, Application Ver.:%hs, Bootloader ver.:%hs\r\n\r\n"), port.sPortNumber.c_str(), \
        port.sConnectedDevice.c_str(), port.sSerialNumber.c_str(), port.sApplicationVersion.c_str(), port.sBootLoaderVersion.c_str());

    thislogger << LoggerPrefix(Info) << port.sPortNumber.c_str() << "  is connected to:" << port.sConnectedDevice.c_str() << ",Serial Number:" << port.sSerialNumber.c_str() << ", Application Ver.:" << port.sApplicationVersion.c_str() << ", Bootloader ver.:" << port.sBootLoaderVersion.c_str() << std::endl;

    //if (0 != port.sConnectedDevice.compare(std::string("DPI610E")))
    //{
    //    _tprintf(TEXT("\r\n Conn-0-]cted device is not DPI610E !\r\n"));
    //    thislogger << LoggerPrefix(I  ##nfo) << " Selected device is not DPI610E" << std::endl;
    //    thislogger << LoggerPrefix(Info) << " Quitting download" << std::endl;
    //    return FALSE;
    //}
    
    _tcscpy_s(tComPortName, CA2T(port.sPortNumber.c_str()));

    //  Open a handle to the specified com port.
    GetComPortHandle(hCom, tComPortName);

    if (INVALID_HANDLE_VALUE == hCom)
    {
        thislogger << LoggerPrefix(Error) << "Failed in opening the device " << std::endl;
        return FALSE;
    }

    thislogger << LoggerPrefix(Info) << "\r\n Verifying Device's Communication... \r\n" << std::endl;

    // Verify connection to the device
    if (!SendCommand(hCom, sDeviceStatusProbe))
    {
        //_tprintf(TEXT("\r\n Failed in sending a device's status query \r\n"));
        thislogger << LoggerPrefix(Error) << "Failed in sending a device's status query" << std::endl;
        CloseHandle(hCom);
        return FALSE;
    }
    std::string sStatusResponse(STATUS_RESPONSE_LEN, 0);

    if (!ReadDeviceResponse(hCom, sDeviceResponse, sDeviceStatusResponse, sStatusResponse))
    {
        thislogger << LoggerPrefix(Error) << "Failed in reading device's status" << std::endl;
        CloseHandle(hCom);
        return(FALSE);
    }

    if (!SendCommand(hCom, sDeviceSetRemote))
    {
        thislogger << LoggerPrefix(Error) << "Failed in setting the device to remote mode" << std::endl;
        CloseHandle(hCom);
        return FALSE;
    }

    UsbEnabled = false;

    if (UsbEnabled)
    {

        // For now - switch the device access mode to mass storage  

        std::string sSetPP4MassStorage(sDevicePinProtection);

        sSetPP4MassStorage.append(sPPforMassStorage);

        if (!SendCommand(hCom, sSetPP4MassStorage.c_str(), &DownloadCommsTimeOut))
        {
            thislogger << LoggerPrefix(Error) << "Failed in setting the device's pin protection to enable mass storage setting  " << std::endl;
            return FALSE;
        }
        // Verify correct mode 
        if (!SendCommand(hCom, sDeviceStatusProbe))
        {
            thislogger << LoggerPrefix(Error) << "Failed in sending a device's status query " << std::endl;
            return FALSE;
        }

        std::string sHexStatusResponse = std::string("0x") + sStatusResponse;
        unsigned short uStatusResponse = std::stoi(sHexStatusResponse, 0, 16);

        if (0 != (uStatusResponse & INVALID_MODE))
        {
            thislogger << LoggerPrefix(Error) << "Setting Device's PP failed " << std::endl;
            return(FALSE);
        }

   //      if (!SendCommand(hCom, sDeviceSetLocal, &NormalCommsTimeOut))
   //      {
			//_tprintf(TEXT("\r\n Failed in transferring the device to remote mode \r\n"));

			//thislogger << LoggerPrefix(Error) << "Failed in transferring the device to remote mode" << std::endl;
			//CloseHandle(hCom);
			//return FALSE;
		 //}

        //if (!SendCommand(hCom, sDeviceSetRemote, &NormalCommsTimeOut))
        //{
        //    //_tprintf(TEXT("\r\n Failed in transferring the device to remote mode \r\n"));
        //    thislogger << LoggerPrefix(Error) << "Failed in transferring the device to remote mode" << std::endl;
        //    CloseHandle(hCom);
        //    return FALSE;
        //}

        if (!SendCommand(hCom, sDeviceSetMassStorage))
        {
            thislogger << LoggerPrefix(Error) << "Failed in sending the device the set-mass-storage command" << std::endl;
            return FALSE;
        }

        wchar_t sDevicePath[] = L"D:\\DK0492.raw";

        thislogger << LoggerPrefix(Info) << "Downloading file to device (can take up to 45 seconds)..." << std::endl;
        
        sud->m_gauge1->SetRange(100);

        sud->Text_UploadingUpdateToDevice->Show();
        
    	do
        {
            wxYield();

        } while (!Startupdate);
        
    	bSuccess = CopyFileExW(tFileName.c_str(), sDevicePath, &CopyProgressRoutine, sud, FALSE, FALSE);
        
        if (!bSuccess)
        {
            thislogger << LoggerPrefix(Error) << "Failed in device download" << std::endl;
            return FALSE;
        }

        // Verify successfull download
        if (!PathFileExistsW(sDevicePath))
        {
            thislogger << LoggerPrefix(Error) << "Failed in downloaded file's existence verification" << std::endl;
            return FALSE;
        }

        thislogger << LoggerPrefix(Info) << "File (" << tFileName << ") downloaded successfully" << std::endl;

        sud->Text_UploadingUpdateToDevice->Hide();
        sud->Text_SwitchToVCP->Show();
        sud->Refresh();

        hCom = INVALID_HANDLE_VALUE;

        do
        {

            //  Open a handle to the specified com port.
            GetComPortHandle(hCom, tComPortName);
            wxYield();

        } while (hCom == INVALID_HANDLE_VALUE);


        sud->Text_SwitchToVCP->Hide();
        sud->Refresh();

    	//////////////////////

        _tprintf(TEXT("\r\n Verifying Device's Communication... \r\n"));

        // Verify connection to the device
        if (!SendCommand(hCom, sDeviceStatusProbe, &NormalCommsTimeOut))
        {
            thislogger << LoggerPrefix(Error) << "Failed in sending a device's status query" << std::endl;
            CloseHandle(hCom);
            return FALSE;
        }

        if (!ReadDeviceResponse(hCom, sDeviceResponse, sDeviceStatusResponse, sStatusResponse))
        {
            thislogger << LoggerPrefix(Error) << "Failed in reading device's status" << std::endl;
            CloseHandle(hCom);
            return(FALSE);
        }
        // Switch back to remote mode
        if (!SendCommand(hCom, sDeviceSetRemote))
        {
            thislogger << LoggerPrefix(Error) << "Failed in setting the device to remote mode" << std::endl;
            CloseHandle(hCom);
            return FALSE;
        }

        Sleep(100);

        if (!SendCommand(hCom, sDeviceStatusProbe))
        {
            thislogger << LoggerPrefix(Error) << "Failed in sending a device's status query" << std::endl;
            CloseComPort(hCom);
            return FALSE;
        }

        if (!ReadDeviceResponse(hCom, sDeviceResponse, sDeviceStatusResponse, sStatusResponse))
        {
            thislogger << LoggerPrefix(Error) << "Failed in reading device's status" << std::endl;
            CloseComPort(hCom);
            return(FALSE);
        }
        
        //////////
        
    }
    else
    {
		//copy file to USB via VCP
        
    	bSuccess = DownLoadFileVCP(hCom, tFileName, dwDataSize, sud);

        if (!bSuccess)
        {
            thislogger << LoggerPrefix(Error) << "Download file failed" << std::endl;
            return FALSE;
        }

        //hCom = CreateFile(tComPortName,
        //    GENERIC_READ | GENERIC_WRITE,
        //    0,      //  must be opened with exclusive-access
        //    NULL,   //  default security attributes
        //    OPEN_EXISTING, //  must use OPEN_EXISTING
        //    0,      //  not overlapped I/O
        //    NULL); //  hTemplate must be NULL for comm devices

        //if (INVALID_HANDLE_VALUE == hCom)
        //{
        //    //_tprintf(TEXT("\r\n Failed in opening the device \r\n"));
        //    thislogger << LoggerPrefix(Error) << "Failed in opening the device after the download " << std::endl;
        //    return FALSE;
        //}

        if (!SetCommState(hCom, &dcb))
        {
            thislogger << LoggerPrefix(Error) << "Failed in setting the comms state" << std::endl;
            CloseComPort(hCom);
            return FALSE;
        }

        _tprintf(TEXT("\r\n Verifying Device's Communication... \r\n"));

        // Verify connection to the device
        if (!SendCommand(hCom, sDeviceStatusProbe, &NormalCommsTimeOut))
        {
            thislogger << LoggerPrefix(Error) << "Failed in sending a device's status query" << std::endl;
            CloseHandle(hCom);
            return FALSE;
        }

        if (!ReadDeviceResponse(hCom, sDeviceResponse, sDeviceStatusResponse, sStatusResponse))
        {
            thislogger << LoggerPrefix(Error) << "Failed in reading device's status" << std::endl;
            CloseHandle(hCom);
            return(FALSE);
        }
        // Switch back to remote mode
        if (!SendCommand(hCom, sDeviceSetRemote))
        {
            thislogger << LoggerPrefix(Error) << "Failed in setting the device to remote mode" << std::endl;
            CloseHandle(hCom);
            return FALSE;
        }

        Sleep(100);

        if (!SendCommand(hCom, sDeviceStatusProbe))
        {
            thislogger << LoggerPrefix(Error) << "Failed in sending a device's status query" << std::endl;
            CloseComPort(hCom);
            return FALSE;
        }

        if (!ReadDeviceResponse(hCom, sDeviceResponse, sDeviceStatusResponse, sStatusResponse))
        {
            thislogger << LoggerPrefix(Error) << "Failed in reading device's status" << std::endl;
            CloseComPort(hCom);
            return(FALSE);
        }
    }
    
    std::string sSetPP4Upgrade(sDevicePinProtection);

    sSetPP4Upgrade.append(sPPforUpgrade);

    if (!SendCommand(hCom, sSetPP4Upgrade.c_str()))
    {
        thislogger << LoggerPrefix(Error) << "Failed in setting the device's pin protection to enable f/w upgrade" << std::endl;
        CloseComPort(hCom);
        return FALSE;
    }

    // Verify correct mode 
    if (!SendCommand(hCom, sDeviceStatusProbe))
    {
        //_tprintf(TEXT("\r\n Failed in sending a device's status query \r\n"));
        thislogger << LoggerPrefix(Error) << "Failed in sending a device's status query" << std::endl;
        CloseComPort(hCom);
        return FALSE;
    }

    std::string sHexStatusResponse = std::string("0x") + sStatusResponse;
    auto uStatusResponse = std::stoi(sHexStatusResponse, 0, 16);

    if (0 != (uStatusResponse & INVALID_MODE))
    {
        thislogger << LoggerPrefix(Error) << "Setting Device's PP failed" << std::endl;
        CloseComPort(hCom);
        return(FALSE);
    }

    thislogger << LoggerPrefix(Info) << "Upgrade device started" << std::endl;

    // Initiate an upgrade (UF command)
    if (!SendCommand(hCom, sDeviceUpgradeFirmware))
    {
        //_tprintf(TEXT("\r\n Failed in sending an upgrade command \r\n"));
        thislogger << LoggerPrefix(Error) << "Failed in sending an upgrade command" << std::endl;
        CloseComPort(hCom);
        return FALSE;
    }

    unsigned short uUpgradeStatusResponse(0);

    Sleep(100);

    // Query the device for its upgrade status
    if (!SendCommand(hCom, sDeviceUpgradeStatus))
    {
        thislogger << LoggerPrefix(Error) << "Failed in sending an upgrade status query " << std::endl;
        CloseComPort(hCom);
        return FALSE;
    }

    if (!ReadDeviceResponse(hCom, sDeviceResponse, sDeviceStatusResponse, sStatusResponse))
    {
        thislogger << LoggerPrefix(Error) << "Failed in reading device's upgrade status" << std::endl;
        CloseComPort(hCom);
        return(FALSE);
    }

    uUpgradeStatusResponse = std::stoi(sStatusResponse);
    if (uStatusResponse < E_UPGRADE_ERROR_DEVICE_BUSY)
    {
        thislogger << LoggerPrefix(Info) << "Device's reported upgrade status is " << cUpgradeStatus[uUpgradeStatusResponse] << std::endl;
    }
    else
    {
        _tprintf(TEXT("\r\n Device upgrade failed due to %hs \r\n"), cUpgradeStatus[uUpgradeStatusResponse]);
        thislogger << LoggerPrefix(Error) << "Device upgrade failed due to " << cUpgradeStatus[uUpgradeStatusResponse] << std::endl;
        CloseComPort(hCom);
        return(FALSE);
    }


#if 0
    // Query the device to check validation errors
    if (!ReadDeviceResponse(hCom, sDeviceResponse, sDeviceStatusResponse, sStatusResponse))
    {
        //_tprintf(TEXT("\r\n Failed in reading device's status \r\n"));
        thislogger << LoggerPrefix(Error) << "Failed in reading device's status" << std::endl;
        CloseHandle(hCom);
        return(FALSE);
    }

    std::string sHexStatusResponse = std::string("0x") + sStatusResponse;
    auto uStatusResponse = std::stoi(sHexStatusResponse, 0, 16);

    if (0 != (uStatusResponse & VALIDATION_ERRORS))
    {
        //_tprintf(TEXT("\r\n Validation of new version failed \r\n"));
        thislogger << LoggerPrefix(Error) << "Validation of new version failed" << std::endl;
        CloseHandle(hCom);
        return(FALSE);
    }
#endif

    _tprintf(TEXT("\r\n Waiting for the device to finish the upgrade.."));

#define MEASURE_UPGRADE_TIME
#ifdef MEASURE_UPGRADE_TIME
    // Start measuring time
    auto begin = std::chrono::high_resolution_clock::now();
#endif
    DWORD dwUpgradeTimeOut = FIRST_UPGRADE_WAIT_TIME_IN_100_MILI;
    sud->m_gauge1->SetRange(FIRST_UPGRADE_WAIT_TIME_IN_100_MILI);
    
    do
    {

        // Wait up to 2 minutes for it to finish
        Sleep(100);
        if (0 == (dwUpgradeTimeOut % 10))
        {
            _tprintf(TEXT("."));
        }

        int x = dwUpgradeTimeOut % 10;
        sud->m_gauge1->SetValue(FIRST_UPGRADE_WAIT_TIME_IN_100_MILI - dwUpgradeTimeOut);
        
    } while (--dwUpgradeTimeOut);
    _tprintf(TEXT("\r\n"));

    // Close and reopen comms with the device
    CloseHandle(hCom);
    dwUpgradeTimeOut = SECOND_UPGRADE_WAIT_TIME_IN_100_MILI;
    sud->m_gauge1->SetRange(SECOND_UPGRADE_WAIT_TIME_IN_100_MILI);

    _tprintf(TEXT("\r\n Trying to reopen device's com port.."));

	do
    {

        //  Open a handle to the specified com port.
        GetComPortHandle(hCom, tComPortName);

        if (0 == (dwUpgradeTimeOut % 10))
        {
            //_tprintf(TEXT("."));
            sud->m_gauge1->SetValue(SECOND_UPGRADE_WAIT_TIME_IN_100_MILI - dwUpgradeTimeOut);
        }
        
        Sleep(100);

    } while ((INVALID_HANDLE_VALUE == hCom) && (--dwUpgradeTimeOut));

    if (INVALID_HANDLE_VALUE == hCom)
    {
        thislogger << LoggerPrefix(Error) << "Couldn't reopen comm port after upgrade." << std::endl;
        return FALSE;
    }

    if (!SetCommState(hCom, &dcb))
    {
        //_tprintf(TEXT("\r\n Failed in setting the comms state \r\n"));
        thislogger << LoggerPrefix(Error) << "Failed in setting the comms state" << std::endl;
        CloseHandle(hCom);
        return FALSE;
    }

    _tprintf(TEXT("\r\n\r\n Querying device's status.."));

    dwUpgradeTimeOut = THIRD_UPGRADE_WAIT_TIME_IN_100_MILI;
    sud->m_gauge1->SetRange(THIRD_UPGRADE_WAIT_TIME_IN_100_MILI);

    do
    {
        bSuccess = SendCommand(hCom, sDeviceStatusProbe, &NormalCommsTimeOut);

        if (bSuccess)
        {
            bSuccess = ReadDeviceResponse(hCom, sDeviceResponse, sDeviceStatusResponse, sStatusResponse);
        }
        if (0 == (dwUpgradeTimeOut % 10))
        {
            //_tprintf(TEXT("."));

            sud->m_gauge1->SetValue(THIRD_UPGRADE_WAIT_TIME_IN_100_MILI - dwUpgradeTimeOut);
        }



        Sleep(10);

    } while ((!bSuccess) && (--dwUpgradeTimeOut));

    if (!bSuccess)
    {
        thislogger << LoggerPrefix(Error) << "Couldn't query device's status after upgrade." << std::endl;
        return FALSE;
    }
    _tprintf(TEXT("\r\n"));

    _tprintf(TEXT("\r\n  Device upgrade completed \r\n"));

#ifdef MEASURE_UPGRADE_TIME
    // Stop measuring time and calculate the elapsed time
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);

    _tprintf(TEXT("\r\nTime measured for the upgrade device's comms : %.3f seconds.\n"), elapsed.count() * 1e-9);

#endif  

    if (!bSuccess)
    {
        //_tprintf(TEXT("\r\n Timed out waiting for the device's upgrade"));
        thislogger << LoggerPrefix(Error) << "Timed out waiting for the device's upgrade" << std::endl;
        CloseHandle(hCom);
        return FALSE;
    }

    thislogger << LoggerPrefix(Info) << "Upgrade completed" << std::endl;

    // Verify successfull upgrade    
    sHexStatusResponse = std::string("0x") + sStatusResponse;
    uStatusResponse = std::stoi(sHexStatusResponse, 0, 16);


    if (0 != (uStatusResponse & UPGRADE_ERRORS))
    {
        //_tprintf(TEXT("\r\n Upgrade device failed \r\n"));
        thislogger << LoggerPrefix(Error) << "Upgrade device failed" << std::endl;
        CloseHandle(hCom);
        return(FALSE);
    }

    if (!SendCommand(hCom, sDeviceSetRemote))
    {
        //_tprintf(TEXT("\r\n Failed in setting the device to remote mode \r\n"));
        thislogger << LoggerPrefix(Error) << "Failed in setting the device to remote mode" << std::endl;
        CloseHandle(hCom);
        return FALSE;
    }

    // Check that current device's application version is the downloaded one.
    if (!SendCommand(hCom, sDeviceAppVersionProbe))
    {
        //_tprintf(TEXT("\r\n Failed in sending a device's application version query \r\n"));
        thislogger << LoggerPrefix(Error) << "Failed in sending a device's application version query" << std::endl;
        CloseComPort(hCom);
        return FALSE;
    }

    std::string sAppVersion(DEVICE_APPLICATION_VERSION_LENGTH, 0);

    if (!ReadDeviceResponse(hCom, sDeviceResponse, sDeviceAppVerResponse, sAppVersion))
    {
        //_tprintf(TEXT("\r\n Failed in reading device's application version \r\n"));
        thislogger << LoggerPrefix(Error) << "Failed in reading device's application version" << std::endl;
        CloseComPort(hCom);
        return(FALSE);
    }

    thislogger << LoggerPrefix(Info) << "Devices application version after the upgrade is " << sAppVersion << std::endl;

    CloseComPort(hCom);
    return TRUE;
}

static BOOL ValidateFile(std::wstring& wSelectedFile, const wchar_t* wsValidFileName, DWORD& dwFileDataSize, std::wstring sFileLocation)
{
    BOOL bSuccess(FALSE);
    DWORD dNoBytesRead;

    if (wSelectedFile.empty())
    {
        thislogger << "Selected file name is empty" << std::endl;
        return FALSE;
    }

    bSuccess = (0 == wcscmp(wSelectedFile.c_str(), wsValidFileName));

    if (!bSuccess)
    {
        USES_CONVERSION;
        //_tprintf(TEXT("\r\n Selected file name (%hs) is not a valid download file name (%hs) \r\n"), W2A(wSelectedFile.c_str()), W2A(wsValidFileName));
        thislogger << LoggerPrefix(Error) << "Selected file name (" << W2A(wSelectedFile.c_str()) << ") is not a valid download file name (" << W2A(wsValidFileName) << ")" << std::endl;
        return FALSE;
    }

    std::wstring temp = sFileLocation + "\\" + wSelectedFile;
    LPCWSTR FileReceived = temp.c_str();


    // Open the file
    HANDLE hFile = CreateFile(FileReceived,               // file to open
        GENERIC_READ,          // open for reading
        FILE_SHARE_READ,       // share for reading
        NULL,                  // default security
        OPEN_EXISTING,         // existing file only
        FILE_ATTRIBUTE_NORMAL, // normal file
        NULL);                 // no attr. template

    if (hFile == INVALID_HANDLE_VALUE)
    {
        USES_CONVERSION;

        //_tprintf(TEXT("  \r\n Unable to open file %hs for read. \r\n"), W2A(wSelectedFile.c_str()));
        thislogger << LoggerPrefix(Error) << "Unable to open file " << W2A(wSelectedFile.c_str()) << "  for read, with reported error:" << GetLastErrorAsString() << std::endl;
        return FALSE;
    }

    // Build the output file namet
    auto pos = wSelectedFile.find(wsReceivedFileExtension);
    std::wstring wStrippedFile = wSelectedFile.substr(0, pos);
    USES_CONVERSION;

    //_tcscpy_s(tOutputDir, sizeof(tOutputDir) / sizeof(TCHAR), tFileLocation);

    //StringCchCat(tOutputDir, MAX_PATH, TEXT("\\out"));

    //wsprintf(tFile2Download, L"%s\\%s%s", FileLocation.c_str(), wStrippedFile.c_str(), wsOutFileExtension);


    File2Download = GetTempFolder() + '\\' + wStrippedFile + wsOutFileExtension;

    // Extract the CRC
    char CRCbytes[9];

    memset(CRCbytes, 0, sizeof(CRCbytes));

    bSuccess = ReadFile(hFile, &CRCbytes, sizeof(CRCbytes) - 1, &dNoBytesRead, NULL);

    if (!bSuccess || ((sizeof(CRCbytes) - 1) != dNoBytesRead))
    {
        //_tprintf(TEXT("\r\n  Couldn't read CRC from candidate file's header \r\n"));
        thislogger << LoggerPrefix(Error) << "Couldn't read CRC from candidate file's header " << std::endl;
        CloseHandle(hFile);
        return FALSE;
    }

    CRCbytes[8] = '\0';

    std::string sReceivedCRC(CRCbytes);

    DWORD dReceivedCRC(0);

    try
    {
        // Disable C4244 warning (conversion from __int64 to DWORD)
#pragma warning(push)
#pragma warning (disable:4244)
        dReceivedCRC = std::stoll(sReceivedCRC, nullptr, 16);
#pragma warning(pop)
    }
    catch (...)
    {
        thislogger << LoggerPrefix(Error) << "CRC read is not valid  " << sReceivedCRC << std::endl;
        CloseHandle(hFile);
        return FALSE;
    }

    // Validate the CRC 
    CRCcalculator crc;

    DWORD dStartPosition = 2;      // Skip the CR+LF that follow the CRC

    std::string outPutFolder = GetTempFolder() + "\\Druck";

    bSuccess = crc.crc32AndRawFileCreation(hFile, dReceivedCRC, dStartPosition, TRUE, sStartOfData, (wchar_t*)File2Download.c_str(), (wchar_t*)outPutFolder.c_str()  , dwFileDataSize);

    CloseHandle(hFile);

    return bSuccess;
}

void StartLogger(LoggerSettings& settings, std::wstring& wSelectedFile, tPort& selectedPort)
{
    USES_CONVERSION;

    auto pos = wSelectedFile.find(L".");
    std::wstring  wsDKnumber = wSelectedFile.substr(0, pos);

    std::string logPath = GetLoggingPath().append("\\");

    settings.path = logPath;
    settings.filename = std::string(W2A(wsDKnumber.c_str())) + "_" + "SN" + "_" + selectedPort.sSerialNumber;

    LoggerInit(settings);

    LoggerConsoleOutput(FALSE);

    thislogger << "   Application Name:" << cApplicationName << std::endl;

    thislogger << "   Version:" << cVersion << std::endl;

    thislogger << "=======================================================" << std::endl;
    
}

bool FindDevices(std::vector<tPort>& available_devices)
{

    FindActiveComPorts(available_devices);

    LoggerSettings settings;

    if (available_devices.empty())
    {

        //throw error to giu

        /*wxMessageBox(wxString::Format
        (
            "\r\n  No Virtual COM Ports are currently connected to this machine (have you got another device/s communication window open by any chance ?) \r\n"
        ),
            "DPI UPDATE PROGRAMMING TOOL",
            wxOK | wxICON_INFORMATION,
            this);*/

        available_devices.clear();

        return false;

    }
    else
    {

        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);

        CRCcalculator crc;
        //  Initialize the DCB structure.
        SecureZeroMemory(&dcb, sizeof(DCB));
        dcb.DCBlength = sizeof(DCB);
        //  Fill in some DCB values and set the com state: 
        //  38,400 bps, 8 data bits, no parity, and 1 stop bit.
        dcb.BaudRate = CBR_256000;     //  baud rate
        //dcb.BaudRate = CBR_38400;     //  baud rate
        dcb.fBinary = TRUE;
        dcb.ByteSize = 8;             //  data size, xmit and rcv
        dcb.Parity = NOPARITY;      //  parity bit
        dcb.StopBits = ONESTOPBIT;    //  stop bit

        /*_tprintf(TEXT("\r\n  List Of Currently Active COM Ports:\r\n"));
        _tprintf(TEXT("  ===================================\r\n\r\n"));*/

        for (auto& port : available_devices)
        {
            BOOL bSuccess = ConfigureComPort(port, dcb);

            if (!bSuccess)
            {
                dcb.BaudRate = CBR_19200;
                bSuccess = ConfigureComPort(port, dcb);
                if (!bSuccess)
                {
                    thislogger << LoggerPrefix(Warning) << "Couldn't read device's data from virtual port " << port.sPortNumber.c_str() << std::endl;
                }
            }

            _tprintf(TEXT("  %hs is connected to:%hs, Serial Number:%hs, Application Ver.:%hs, Bootloader ver.:%hs\r\n\r\n"), port.sPortNumber.c_str(), \
                port.sConnectedDevice.c_str(), port.sSerialNumber.c_str(), port.sApplicationVersion.c_str(), port.sBootLoaderVersion.c_str());

            selectedPort = port;

        }
        fflush(stdin);

    }

    return true;

}

bool StartDownload(dialog_start_update* sud)
{
    bool bSuccess = false;

    bSuccess = download(selectedPort, dcb, File2Download, dwDataSize ,sud);
    if (!bSuccess)
    {
        _tprintf(TEXT("\r\n Device upgrade failed \r\n"));
        thislogger << LoggerPrefix(Error) << "Device upgrade failed" << std::endl;
    
        return false;
    }

    return true;
}

std::string GetLoggingPath()
{
    TCHAR   logDevice[MAX_PATH];
    HRESULT  hr;

    if (SUCCEEDED(hr = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, logDevice))) {
        PathAppend(logDevice, L"Druck\\ProgrammingTool");
    }

    if ((GetFileAttributes((LPCWSTR)logDevice)) == INVALID_FILE_ATTRIBUTES)
    {
        std::filesystem::create_directories(logDevice);
    }

#ifdef UNICODE

    char clogDevice[MAX_PATH];

    WideCharToMultiByte(CP_UTF8, 0, logDevice, -1, &clogDevice[0], MAX_PATH, NULL, NULL);

    std::string slogDevice(&clogDevice[0]);
#else
    std::string slogDevice(tComPortName);
#endif

    return slogDevice;

}

void SetSelectedPort(tPort SelectedPort)
{
    selectedPort = SelectedPort;
}

void SetSelectedFile(std::wstring firmwareFile)
{
    File2Download = firmwareFile;
}

bool ValidateFirmwareFile(std::wstring sfirmwareFile , std::wstring sFullName , std::wstring sFileLocation  )
{
   //selectedFirmware = firmwareFile;

    std::wstring wSelectedFile;

    wSelectedFile = sFullName;

    bool bValidFileSelected = ValidateFile(wSelectedFile, wsValidDownLoadFileName, dwDataSize, sFileLocation);
    if (!bValidFileSelected)
    {
        //_tprintf(TEXT("\r\n  The file selected (%ls) is not valid \r\n"), wSelectedFile);

        thislogger << LoggerPrefix(Error) << _tprintf(TEXT("The file selected (%ls) is not valid"), wSelectedFile) << std::endl;

        //bWrongInput = TRUE;

        //std::string error = wxString::Format("\r\n  The file selected (%ls) is not valid \r\n"), wSelectedFile;

        return false;
    }
    
    return true;
}

void DownloadVCP()
{

}

void DownloadUSB()
{

}

void StartLogging()
{

   // LoggerSettings& settings, std::wstring& wSelectedFile, tPort& selectedPort

}

std::vector<std::string> split(const std::string& s, char delim)
{
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}

void SetUSBMode(bool bstatus)
{
    UsbEnabled = bstatus;
}

bool GetUSBMode()
{
    return UsbEnabled;
}

void SetStartUpdate(bool bStatus)
{
    Startupdate = bStatus;
}

bool GetStartUpdate()
{
    return Startupdate;
}



bool RegisKeyExists()
{
    HKEY hKey;
    LONG nResult;
    BOOL bExist = FALSE;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\abc", 0, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwType;
        nResult = RegQueryValueEx(hKey, L"xyz", NULL, &dwType, NULL, NULL);
        if (nResult == ERROR_SUCCESS)
            bExist = TRUE;
        RegCloseKey(hKey);
    }

    return true;

}

std::wstring GetTempFolder()
{
    auto temp_path = std::filesystem::temp_directory_path().wstring();

    return temp_path;
}

bool DoFirmwareUpdate(ProgressData* pdata, bool (*UpdateCallback)(ProgressData* pdata, int n, std::string message))
{
    for (int n{ 0 }; n < 100; ++n)
    {
        std::string message = "";

        if (n == 0)
        {
            message = "uploading firmware";
        }
        else if (n == 30)
        {
            message = "writing firmware";
        }
        else if (n == 70)
        {
            message = "verifying firmware";
        }

        wxMilliSleep(30);
        UpdateCallback(pdata, n, message);
    }

    return true;
}

DWORD CALLBACK CopyProgressRoutine(LARGE_INTEGER TotalFileSize,
    LARGE_INTEGER TotalBytesTransferred,
    LARGE_INTEGER StreamSize,
    LARGE_INTEGER StreamBytesTransferred,
    DWORD dwStreamNumber,
    DWORD dwCallbackReason,
    HANDLE hSourceFile,
    HANDLE hDestinationFile,
    LPVOID lpData)

{

    

    dialog_start_update* pcid = static_cast<dialog_start_update*>(lpData);
    
    long lTotalSize = (TotalBytesTransferred.QuadPart / TotalFileSize.QuadPart * 100);

    pcid->m_gauge1->SetValue(lTotalSize);

    return PROGRESS_CONTINUE;

}