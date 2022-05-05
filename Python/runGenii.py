# -*- coding: utf-8 -*-
"""
Created on Fri Jun 18 11:09:59 2021

@author: 212596572
"""


import dpi620gLib as dpi
import time

refCom = 2
error = 4294967295

print(refCom & error)

if refCom == (refCom & error):
    print('Same')
else:
    print('Different')

while refCom == (refCom and error):
    print('Hello')

print('Ended')