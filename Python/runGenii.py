# -*- coding: utf-8 -*-
"""
Created on Fri Jun 18 11:09:59 2021

@author: 212596572
"""


import dpi620gLib as dpi
import time

DPI620G = dpi.DPI620G()  # connect to PV624 OWI GENII port
DPI620G.setKM('R')  # set PV624 GENII interface to remote mode
