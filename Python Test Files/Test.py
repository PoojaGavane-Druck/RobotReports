# -*- coding: utf-8 -*-
"""
Created on Tue Oct 12 13:33:09 2021

@author: 212596572
"""



import dpi620gLib as dpi
from datetime import datetime
import time
import csv


def dpiTest():
    DPI620G = dpi.DPI620G()
    samples = 3600
        
    km = DPI620G.getKM()
    print(km)
    DPI620G.setKM('R')
    km = DPI620G.getKM()
    
    DPI620G.setSC("1")
    
dpiTest()
