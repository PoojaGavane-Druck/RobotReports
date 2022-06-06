import dpi620gLib as dpi
import pv624Lib as pvComms
import dpiAttributes as dpiAttr
from datetime import datetime
import time

dpi620gSn = ['FTBTBC9KA']

def terpsFwUpgrade():

    DPI620G = dpi.DPI620G(dpi620gSn)
    # First switch genii to remote mode
    DPI620G.setKM('R')

    DPI620G.getSZ()

terpsFwUpgrade()
