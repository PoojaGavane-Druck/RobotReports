@echo off
setlocal EnableDelayedExpansion

REM Cater for ST-Link installation on 64 and 32 bit machines
set EXE="%ProgramFiles(x86)%\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe"
if not defined ProgramFiles(x86) set EXE="%PROGRAMFILES%\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe"

echo Programming primary micro controller...
echo Erasing flash
%EXE% -c SN=141D17028315303030303032 -ME

echo Programming primary bootloader...
%EXE% -c SN=141D17028315303030303032 -P "DK0498.hex" -V

echo Programming primary application...
%EXE% -c SN=141D17028315303030303032 -P "DK0499.hex" -V

echo Programming secondary micro controller...
echo Erasing flash
%EXE% -c SN=1C1A17018315303030303032 -ME

echo Programming secondary bootloader...
%EXE% -c SN=1C1A17018315303030303032 -P "DK0510.hex" -V

echo Programming secondary application...
%EXE% -c SN=1C1A17018315303030303032 -P "DK0509.hex" -V


echo Please power cycle the PV624 

endlocal
pause