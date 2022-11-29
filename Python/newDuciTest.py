import dpi620gLib as dpi
import dpiAttributes as dpiAttr
from datetime import datetime
import time
import csv

dpi620gSn = ['FTBTBC9KA']

printIt = 1
def display(toDisplay):
    if printIt == 1:
        print(toDisplay)

def main():
    DPI620G = dpi.DPI620G(dpi620gSn, dpiAttr.connectionOwi)

    display("Setting remote mode")
    DPI620G.setKM('R')
    mode = DPI620G.getKM()
    if mode == 1:
        display("Remote mode set")

main()