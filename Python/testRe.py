import dpi620gLib as dpi
import dpiAttributes as dpiAttr
import time

dpi620gSn = ['FTBTBC9KA']
connectionType = dpiAttr.connectionOwi

def testRe(sleepTime, iterations):
    DPI620G = dpi.DPI620G(dpi620gSn, connectionType)
    count = 0
    while count < iterations:
        error = DPI620G.getRE()
        print(error)
        time.sleep(sleepTime)

testRe(0.1, 100)
