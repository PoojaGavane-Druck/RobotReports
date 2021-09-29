# -*- coding: utf-8 -*-
"""
Created on Fri Jun 18 18:51:35 2021

@author: 212596572
"""
import time

start = time.time()
counter = 0
while counter < 44:
    print("")
    counter = counter + 1
    
end = time.time()

print(start, end, end - start)