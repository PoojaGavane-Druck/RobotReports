import dpi620gLibDuci as dpi
import time

def calBaro():
    DPI620G = dpi.DPI620G()
    # Set KM = R
    msg = DPI620G.setKM('R')
    print(msg)
    msg = DPI620G.getKM()
    print(msg)
    # Set PT = 2
    msg = DPI620G.setPT("2")
    print(msg)
    msg = DPI620G.getPT()
    print(msg)
    # Set PP = 123
    msg = DPI620G.setPP("123")
    print(msg)
    # Set CT = 0,0
    msg = DPI620G.setCT("0,0")
    print(msg)
    # Get CN, cn should be 2,2
    minPoints, maxPoints = DPI620G.getCN()
    # Set CS
    calPoints = 0
    while calPoints < maxPoints:
        print("Calibration point no. " + str(calPoints + 1))
        msg = DPI620G.setCS()
        # Get CS, read until CS = 0
        csVal = 100
        while csVal != 0:
            csVal = DPI620G.getCS()
        # Set CP1 = Applied value 
        DPI620G.setCP()
        time.sleep(0.3)
        calPoints = calPoints + 1
        a = input("Change set point")
    # Set CA - accept calibration
    DPI620G.setCA()

calBaro()