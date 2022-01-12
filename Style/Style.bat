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
%exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\*.c"
%exe% %recursiveargs% "%~dp0\..\Drivers\Peripherals\*.h"

%exe% %nonrecursiveargs% "%~dp0\..\src\cVersion.c"
%exe% %nonrecursiveargs% "%~dp0\..\src\os_app_hooks.c"
%exe% %nonrecursiveargs% "%~dp0\..\src\MainApp.c"
%exe% %nonrecursiveargs% "%~dp0\..\inc\MainApp.h"