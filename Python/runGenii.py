import dpi620gLib as dpi
import pv624Lib as pvComms
import dpiAttributes as dpiAttr
from datetime import datetime

dpi620gSn = ['FTBTBC9KA']

def test():
    DPI620G = dpi.DPI620G(dpi620gSn)

    secAppVer = DPI620G.getRV(dpiAttr.versionInfo['secondaryApplication'])
    secBootVer = DPI620G.getRV(dpiAttr.versionInfo['secondaryBootloader'])

    print(secAppVer, secBootVer)

test()