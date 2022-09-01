@echo off
@set "exe=%~dp0\Astyle.exe"
@set "nonrecursiveargs=--indent=spaces=4 --style=allman --break-blocks=all --pad-comma --pad-oper --unpad-paren --attach-return-type --align-pointer=name --align-reference=name --add-braces --convert-tabs --suffix=none"
@set "recursiveargs=--recursive %nonrecursiveargs%"

rem Only touch manually created files for project
%exe% %recursiveargs% "%~dp0\..\Application\*.cpp"
%exe% %recursiveargs% "%~dp0\..\Application\*.c"
%exe% %recursiveargs% "%~dp0\..\Application\*.h"

%exe% %recursiveargs% "%~dp0\..\Drivers\Devices\*.cpp"
%exe% %recursiveargs% "%~dp0\..\Drivers\Devices\*.h"
rem %exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\*.c"
rem %exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\*.h"
%exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\BAROMETER\*.c"
%exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\BAROMETER\*.h"

%exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\BATTERY\*.c"
%exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\BATTERY\*.h"

%exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\BATTERY_CHARGER\*.c"
%exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\BATTERY_CHARGER\*.h"

%exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\BLUETOOTH\*.c"
%exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\BLUETOOTH\*.h"

%exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\EEPROM\*.c"
%exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\EEPROM\*.h"

%exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\LEDS\*.c"
%exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\LEDS\*.h"

%exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\RTC\*.c"
%exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\RTC\*.h"

%exe% %nonrecursiveargs% "%~dp0\..\src\cVersion.c"
%exe% %nonrecursiveargs% "%~dp0\..\src\os_app_hooks.c"
%exe% %nonrecursiveargs% "%~dp0\..\src\MainApp.c"
%exe% %nonrecursiveargs% "%~dp0\..\inc\MainApp.h"