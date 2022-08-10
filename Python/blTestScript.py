import bleLib as dpi
import dpiAttributes as dpiAttr
from datetime import datetime
import time

printIt = 1
writeLog = 1

dpi620gSn = ['FTBTBC9KA']
pv624MacAddr = "01D9EA93F98832"

def main():
    okReceived = 0
    DPI620G = dpi.BleDPI(dpi620gSn)

    while okReceived == 0:
        # try for a 100 times
        okReceived = DPI620G.receiveOk("00000000")

    okReceived = 0
    print("Connected to DPI620G BLE")

    # set filter to 0
    DPI620G.setFilter('0')
    while okReceived == 0:
        # try for a 100 times
        okReceived = DPI620G.receiveOk("00000000")

    print("Filter set to 0")
    print("Ready for scanning")
    totalTime = 1
    print(totalTime)
    time.sleep(totalTime)
    print(totalTime)
    time.sleep(totalTime)
    print(totalTime)
    time.sleep(totalTime)
    print(totalTime)
    time.sleep(totalTime)
    print(totalTime)
    time.sleep(totalTime)
    DPI620G.scan()

    status = DPI620G.connect(pv624MacAddr)
    status = status.decode('utf-8')
    connected = 0
    print(status)
    if status == "CN=00000000,00000005,00000001\r\n":
        print("PV624 connected")
        connected = 1

    if connected == 1:
        while True:
            startTime = time.time()
            riData = DPI620G.getRI()
            riMessage = riData.decode('utf-8')
            endTime = time.time()
            time.sleep(0.1)
            endTime = time.time()
            elapsedTime = endTime - startTime
            print(riMessage + "\r\n")





main()