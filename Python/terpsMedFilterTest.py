import dpi620gLib as dpi
import csv
import time
from datetime import datetime

dpi620gSn = ['FTBTBC9KA']

def run():
    DPI620G = dpi.DPI620G(dpi620gSn)
    iterations = input("Enter number of hours to run test: ")
    iterations = float(iterations) * 3600 * 50

    DPI620G.setKM('R')
    km = DPI620G.getKM()

    if km == 1:
        DPI620G.setPT(str(1))
        pt = DPI620G.getPT()
        if pt == 1:
            DPI620G.setCM(0)
            cm = DPI620G.getCM()
            if cm == 0:
                dataFile = 'TERPS_MEDIAN_FILTER_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
                myFile = open(dataFile, 'a', newline='')
                csvFile = csv.writer(myFile, delimiter=',')
                result = ['Iterations', 'Elapsed time', 'Pressure', 'Raw Pressure', 'Raw Temperature', 'Filtered Temperature']
                csvFile.writerow(result)
                myFile.close()
                iterations = int(iterations)
                count = 0
                for n in range(0, iterations):
                    startTime = time.time()
                    myFile = open(dataFile, 'a', newline='')
                    csvFile = csv.writer(myFile, delimiter=',')
                    tic = datetime.now()
                    pressure, error, status, baro = DPI620G.getPV()
                    endTime = time.time()
                    elapsedTime = endTime - startTime
                    count = count + 1
                    result = [count, elapsedTime, pressure, error, status, baro]
                    print(result)
                    csvFile.writerow(result)
                    myFile.close()
            else:
                print("ERROR: Control mode not set to MEASURE mode")
        else:
            print("ERROR: Pressure type not set to ABSOLUTE")
    else:
        print("ERROR: Key mode not set to remote")
    
    print("Complete")

run()