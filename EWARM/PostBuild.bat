:: Based on https://www.iar.com/support/tech-notes/general/ielftool-checksum---when-using-lpc-device/
:: Batch parameter %~1 expands %1 and removes surrounding quotation marks, to allow concatenation of directory with filename, resulting in full path containing spaces in quotation marks.
:: jar file built from repo https://github.build.ge.com/Druck-Pressure/TestTools_EncryptBin branch DPI610Mods

set OUT="%~1\%~2.out"
set HEX="%~1\%~2.hex"
set BIN="%~1\%~2.bin"
set EXE="%~3\arm\bin\ielftool.exe"
set LOG="%~1\..\..\PostBuild.log"
set JAR=java -jar "%~1\..\..\SignDPI610V13.jar"

:: calculate application checksum
%EXE% --fill 0xFF;crc_start-crc_end+3 --checksum __checksum:4,crc32:,0xffffffff;crc_start-crc_end+3 --verbose %OUT% %OUT% > %LOG%

:: generate additional output: hex
%EXE% --ihex --verbose %OUT% %HEX% >> %LOG%

:: generate additional output: binary
%EXE% --bin --verbose %OUT% %BIN% >> %LOG%

:: generate raw file for firmware upgrade via USB MSC
%JAR% %HEX% .raw >> %LOG%

:: generate dpi file for firmware upgrade via PC App
%JAR% %HEX% .dpi >> %LOG%