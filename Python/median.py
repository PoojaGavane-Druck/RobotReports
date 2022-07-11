import random
import matplotlib.pyplot as plt
import csv
from datetime import datetime

def arrangeAscending(arr, window):
    indexOne = 0
    indexTwo = 0

    for indexOne in range(0, window, 1):
        for indexTwo in range(indexOne + 1, window, 1):
            if arr[indexOne] > arr[indexTwo]:
                temp = arr[indexOne]
                arr[indexOne] = arr[indexTwo]
                arr[indexTwo] = temp

    return arr

def medianFilter(arr, window):
    arr = arrangeAscending(arr, window)
    median = arr[3]
    
    return median


def addDataToArray(data, arr, window, index):
    # Update the oldest reading, given by readingIndex
    if index >= (window - 1):
        index = 0
    else:
        index = index + 1

    arr[index] = data
    # update readingIndex


    return arr, index

def createDataArray():
    dataArr = [None] * 1000
    for i in range(0, 1000, 1):
        dataArr[i] = random.uniform(942.3, 942.6)

    return dataArr

def addNoiseInDataArray(dataArr):
    previous = 0
    # Adds noise in data array at different places
    for i in range(0, 30, 1):
        modder = random.randint(16, 45)
        previous = previous + modder
        dataArr[previous] = dataArr[previous] + random.uniform(2.6, 4.6)

    return dataArr

filterArray = [None] * 1000
filterWindow = 5
readingIndex = 0
valueArr = [942.0, 942.1, 942.3, 942.2, 942.0]
median = medianFilter(valueArr, filterWindow)

dataArr = createDataArray()

dataArr = addNoiseInDataArray(dataArr)

dataIndexer = 0
readingIndexer = 0
filterOutput = 0

valueArr[0] = dataArr[0]
valueArr[1] = dataArr[1]
valueArr[2] = dataArr[2]
valueArr[3] = dataArr[3]
valueArr[4] = dataArr[4]

filterVal = 0

fileName = 'MEDIAN_FILTER_TEST_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
with open(fileName,'w',newline='') as f:
    csvFile = csv.writer(f,delimiter = ',')
    result = ['Count', 'Value', 'Filtered value']
    csvFile.writerow(result) 
    for count in range(6, 1000, 1):
        filterVal = medianFilter(valueArr, filterWindow)
        dataIndexer = count
        valueArr, readingIndexer = addDataToArray(dataArr[dataIndexer], valueArr, filterWindow, readingIndexer)
        filterArray[count] = filterVal
        result = [count, dataArr[dataIndexer], filterArray[count]]
        csvFile.writerow(result) 
        print(result)