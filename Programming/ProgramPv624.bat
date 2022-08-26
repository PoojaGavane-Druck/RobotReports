@echo off
setlocal EnableDelayedExpansion

REM Cater for ST-Link installation on 64 and 32 bit machines
set EXE="%ProgramFiles(x86)%\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe"
if not defined ProgramFiles(x86) set EXE="%PROGRAMFILES%\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe"

set PRIMARY_MICRO=Device family: STM32L4Rx/L4Sx
set SECONDARY_MICRO=Device family: STM32L4x3
echo %PRIMARY_MICRO%
echo %SECONDARY_MICRO%

set /a output_cnt=0
for /F "delims=" %%f in ('%EXE% -List') do (
    set /a output_cnt+=1
    set "output[!output_cnt!]=%%f"
)
echo !output_cnt!
::iterate thru output array to find SN and parse it to get the serial number
set /A stLinkOneFound=0
set "serialOne=NONE"
set "serialTwo=NONE"
echo %serialOne%
echo %serialTwo%
for /L %%n in (1 1 %output_cnt%) do (
    echo !output[%%n]!
    FOR /F "tokens=1,* delims=: " %%1 in ("!output[%%n]!") do (
        if "%%1" == "SN" (
			if !stLinkOneFound!==0 (
				set /A stLinkOneFound=1
				echo STLINK 1 Found
				set "serialOne=%%2"
			) else (
				echo STLINK 2 Found
				set "serialTwo=%%2"
			)
			
        )
    )
)
echo %serialOne%
echo %serialTwo%

SET lf=-.
FOR /F "delims=" %%i IN ('%EXE% -c') DO if ("!out!"=="") (set out=%%i) else (set out=!out!!lf!%%i)
ECHO %out%

set /A this=out
echo %this%

SET count=1
FOR /F "delims=" %%F IN ('%EXE% -c') DO (
  SET var!count!=%%F
  SET /a count=!count!+1
)
ECHO "Items available %count%"

:: Run loop to display data as long as the previous count
FOR /L %%X IN (1,1,%count%) DO (
call set newvar=%%var%%X%%
:: Display the concatenated variable - delayed
echo !newvar!
)

set "primSerial=NONE"
set "secSerial=NONE"

if !var12!==!SECONDARY_MICRO! (
	set secSerial=!var3!
	for /F "tokens=1,* delims=: " %%1 in ("!secSerial!") do (
		set "snStrSec=%%2"
	   
		for /F "tokens=1,* delims=: " %%1 in ("!snStrSec!") do (
			set "secSerial=%%2"
			
		)
	   
	)
) else (
	if !var12!==!PRIMARY_MICRO! (
		set primSerial=!var3!
		for /F "tokens=1,* delims=: " %%1 in ("!primSerial!") do (
			set "snStrPrim=%%2"		   
			for /F "tokens=1,* delims=: " %%1 in ("!snStrPrim!") do (
				set "primSerial=%%2"
			)
		)
	)
)

if !serialTwo!==!primSerial! (
	set "secSerial=!SerialOne!"
) else (
	if !serialOne!==!primSerial! (
		set "secSerial=!SerialTwo!"
		) else (
		if !secSerial!==!serialOne! (
			set "primSerial=!SerialTwo!"
			) else (
			if !secSerial!==!serialTwo! (
				set "primSerial=!SerialOne!"
			)
		)
	)
)

echo Primary ST LINK serial - !primSerial!
echo Secondary ST LINK serial - !secSerial!

echo Programming secondary micro controller...
echo Erasing flash
%EXE% -c SN=!secSerial! -ME

echo Programming secondary bootloader...
%EXE% -c SN=!secSerial! -P "DK0510.hex" -V

echo Programming secondary application...
%EXE% -c SN=!secSerial! -P "DK0509.hex" -V

echo Programming primary micro controller...
echo Erasing flash
%EXE% -c SN=!primSerial! -ME

echo Programming primary bootloader...
%EXE% -c SN=!primSerial! -P "DK0498.hex" -V

echo Programming primary application...
%EXE% -c SN=!primSerial! -P "DK0499.hex" -V

echo Please power cycle the PV624 

endlocal
pause