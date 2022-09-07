import dpi620gLib as dpi
import csv
import time
from datetime import datetime

dpi620gSn = ['FTBTBC9KA']

def run():
    DPI620G = dpi.DPI620G(dpi620gSn)
    iterationsPerFile = 20
    maxIterations = input("Enter number of hours to run test: ")
    maxIterations = float(maxIterations) * 72000
    files = int(maxIterations / iterationsPerFile)
    modulus = int(maxIterations) % int(iterationsPerFile)
    if modulus != 0:
        files = files + 1
    remainingIterations = maxIterations
    fileNo = 0
    totalIterations = 0
    DPI620G.setKM('R')
    km = DPI620G.getKM()

    if km == 1:
        DPI620G.setPT(str(1))
        pt = DPI620G.getPT()
        if pt == 1:
            DPI620G.setCM(0)
            cm = DPI620G.getCM()
            if cm == 0:
                for n in range(0, files):
                    fileNo = fileNo + 1
                    dataFile = 'TERPS_MED_FILT_' + 'FILE_NO_' + str(fileNo) + '_' + datetime.now().strftime('%Y-%m-%d-%H-%M-%S') + '.csv'
                    myFile = open(dataFile, 'a', newline='')
                    csvFile = csv.writer(myFile, delimiter=',')
                    result = ['Iterations', 'Elapsed time', 'Pressure', 'Error', 'Status', 'Baro Pressure']
                    csvFile.writerow(result)
                    myFile.close()
                    if iterationsPerFile < remainingIterations:
                        iterations = iterationsPerFile
                    else:
                        iterations = remainingIterations

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
                        result = [count, round(elapsedTime, 3) * 1000, round(pressure, 3), error, status, round(baro, 3)]
                        print(result)
                        csvFile.writerow(result)
                        myFile.close()
                    
                    remainingIterations = remainingIterations - iterations
            else:
                print("ERROR: Control mode not set to MEASURE mode")
        else:
            print("ERROR: Pressure type not set to ABSOLUTE")
    else:
        print("ERROR: Key mode not set to remote")
    
    print("Complete")

run()