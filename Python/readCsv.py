import dpi620gLib as dpi

dpi620gSn = ['FTBTA7ISA']

def readCsv():
    DPI620G = dpi.DPI620G(dpi620gSn)

    DPI620G.setSC('1')

    
