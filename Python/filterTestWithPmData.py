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
    pointer = (window + 1) / 2 
    median = arr[(int)(pointer)]
    
    return median

def main():
    filterWindow = 15
    readingIndex = 0
    initValue = 0.0
    countData = [None] * 200000
    timeData = [None] * 200000
    pressureDataAct = [None] * 200000
    pressureData = [None] * 200000
    dataForPressureUse = [None] * 199999
    filteredPressure = [None] * 200000
    filterData = [initValue] * filterWindow
    with open("PA_TEST_2022-07-07-10-51-28.csv", newline='') as f:
        reader = csv.reader(f)
        data = list(reader)
        print(data[0][0])

        for count in range(0, 200000):
            countData[count] = data[count][0]
            timeData[count] = data[count][1]
            pressureDataAct[count] = data[count][2]
            pressureData[count] = data[count][3]
            if count < 199999:
                dataForPressureUse[count] = data[count + 1][3]

        for count in range(0, filterWindow):
            filterData[count] = 0.0

        for count in range(0, 199998):
            filterData[readingIndex] = float(dataForPressureUse[count])
            readingIndex = readingIndex + 1
            if readingIndex >= (filterWindow - 1):
                readingIndex = 0
            filteredPressure[count] = medianFilter(filterData, filterWindow)

            if filteredPressure[count] == 0:
                filteredPressure[count] = dataForPressureUse[count]
        # create first line
    
    result = [countData[0], timeData[0], pressureDataAct[0], pressureData[0], "Filtered pressure"]
    fileName = 'PM_DATA_MSWN_FILTER_TEST_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
    with open(fileName,'w',newline='') as f:
        csvFile = csv.writer(f,delimiter = ',')
        csvFile.writerow(result) 
        for count in range(1, 199999, 1):
            result = [countData[count], timeData[count], pressureDataAct[count], pressureData[count], filteredPressure[count - 1]]
            csvFile.writerow(result) 

        
            
    print("Done")



main()