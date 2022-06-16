# -*- coding: utf-8 -*-
"""
Created on Mon May 24 10:07:42 2021

@author: 212596572
"""

pressureType = {}
pressureType['Gauge'] = 0x00
pressureType['Absolute'] = 0x01
pressureType['Barometer'] = 0x02

controlMode = {}
controlMode['Measure'] = 0x00
controlMode['Control'] = 0x01
controlMode['Vent'] = 0x02