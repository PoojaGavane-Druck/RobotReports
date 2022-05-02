# -*- coding: utf-8 -*-
"""
Created on Tue Mar 30 15:00:50 2021

@author: 212596572
"""

sensorType = {}
sensorType['PM620'] = 0x00
sensorType['PM620TERPS'] = 0x01

sensorPressType = {}
sensorPressType['Gauge'] = 0x02
sensorPressType['Absolute'] = 0x01

deviceModes = {}
deviceModes['Measure'] = 0x00
deviceModes['Control'] = 0x01
deviceModes['Vent'] = 0x02
deviceModes['Rate'] = 0x03

spType = {}
spType['Gauge'] = 0x00
spType['Absolute'] = 0x01
spType['Barometer'] = 0x02

pmSpeed = {}
pmSpeed['E_ADC_SAMPLE_RATE_CH_OFF'] = 0x00
pmSpeed['E_ADC_SAMPLE_RATE_3520_HZ'] = 0x01
pmSpeed['E_ADC_SAMPLE_RATE_1760_HZ'] = 0x02
pmSpeed['E_ADC_SAMPLE_RATE_880_HZ'] = 0x03
pmSpeed['E_ADC_SAMPLE_RATE_440_HZ'] = 0x04
pmSpeed['E_ADC_SAMPLE_RATE_220_HZ'] = 0x05
pmSpeed['E_ADC_SAMPLE_RATE_110_HZ'] = 0x06
pmSpeed['E_ADC_SAMPLE_RATE_55_HZ'] = 0x07
pmSpeed['E_ADC_SAMPLE_RATE_27_5_HZ'] = 0x08
pmSpeed['E_ADC_SAMPLE_RATE_13_75_HZ'] = 0x09
pmSpeed['E_ADC_SAMPLE_RATE_6_875_HZ'] = 0x0F