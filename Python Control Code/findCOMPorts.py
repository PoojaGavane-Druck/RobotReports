import serial as ser
import serial.tools.list_ports as prtlst

pts = prtlst.comports()
for pt in pts:
    print(pt.device)
    print(pt.description)
    print(pt.serial_number)
    print(pt.hwid)
    print('\n')

## unused STM32 board
##    COM12
##STMicroelectronics STLink Virtual COM Port (COM12)
##8
##USB VID:PID=0483:374B SER=8 LOCATION=1-5.7.1:x.2

## used STD32 board
##COM13
##STMicroelectronics STLink Virtual COM Port (COM13)
##8
##USB VID:PID=0483:374B SER=8 LOCATION=1-5.7.1:x.2
    
