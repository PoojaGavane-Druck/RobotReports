# 18 May 2021
# parse PV624 debug data acquired by parseStream.py
# generate summary plot from final performance metrics of all data files in a directory
# generate file dialog to select outlier data files to plot separately as time series
# save all plots as html with csv base filename for later viewing
# May 24
# identify failed setpoints and extract data for plots and closer analysis

import tkinter as tk
from tkinter import filedialog as fd
import plotly.express as px
import pandas as pd
import glob
import numpy as np


def getDirectory():
    gui = tk.Tk()  # initiate tkinter gui, will create a blank gui window
    directory = fd.askdirectory()  # select data directory
    gui.destroy()  # close the blank gui window
    return directory


def getFilename():
    gui = tk.Tk()  # initiate tkinter gui, will create a blank gui window
    filename = fd.askopenfilename()  # open file select dialog and return selected value into filename
    gui.destroy()  # close the blank gui window
    return filename


# choose a working directory containing csv debug data file(s)
directory = getDirectory()

csvFiles = glob.glob(directory + '/*.csv')  # all csv files in working directory
refFiles = []  # may be more ref files than fine or coarse if coarse control does not terminate before hold time (60s)
controlFiles = []
for filename in csvFiles:
    f = filename.split('\\')[-1]  # discard the paths
    if '_Ref' in f:
        refFiles.append(f)  # only reference files have a '_R_'
        # TBD need a way to match up ref files with control files
        # date stamps will be close but not identical because created in different subprocesses
        # setpoint values should match but may not be unique
    elif len(f.split('-')) == 5:
        controlFiles.append(f)  # remaining files are control files if they have four '-'
        # e.g. 2022-05-12T11-57-30_25mL25C20bar.csv

# get statistics for summary plots vs setpoint pressure and time
computeStats = True

if computeStats:
    for filename in controlFiles:
        print(filename)
        # parse csv for final state values to generate summary plots and csv
        data = pd.read_csv(directory + '/' + filename)

        # extract set-up meta data from filename
        # e.g. 2022-05-12T11-57-30_25mL25C20bar
        # trueVolume = float(filename.split('_')[1].split('mL')[0])  # attached volume in mL
        # ambientTemperature = filename.split('mL')[1].split('C')[0]  # ambient temperature
        sensorFS = float(filename.split('C')[1].split('bar')[0]) * 1000  # sensor FS in mbar, used for scaling below
        # timestamp = filename.split('_')[0]  # timestamp as a string
        metaData = (filename.split('_')[1]).split('.')[0]  # all meta data for naming output files

        # find changes in control flag indicating start and end series indices of a setpoint attempt
        transitions = data['control'].diff()  # changes in control flag status (+/- 1)
        startIndex = list(np.where(transitions == 1)[0])  # index of changes from 0 to 1
        endIndex = list(np.where(transitions == -1)[0])  # index changes from 1 to 0

        # filter rangeExceeded flag to give one and only entry per rangeExceeded event
        transitions = data['rangeExceeded'].diff()  # changes in rangeExceeded flag status (+/- 1)
        data['rangeExceeded'] = transitions.mask(transitions != 1, other=0)

        #  remove invalid index pairs
        if startIndex[0] > endIndex[0]:
            del endIndex[0]  # remove first endIndex value as it has no matching startIndex
        if startIndex[-1] > endIndex[-1]:
            del startIndex[-1]  # remove last startIndex value as it has no matching endIndex

        # extract final values of data fields for each setpoint
        finalValues = {}
        for variable in data.keys():
            finalValues[variable] = data[variable][endIndex]

        # re-select final values of stable flag to just before endIndex
        # as this flag is reset by PV624 at endIndex
        finalValues['stable'] = list(data['stable'][list(np.array(endIndex)-2)])

        # convert pressure error variance to standard deviation for plotting of standard error bars
        # and scale pressure values to PPM FS
        finalValues['stabilityPPM'] = ((finalValues['uncerInSmoothedMeasPresErr'].apply(np.sqrt)).
                                       multiply(1e6/sensorFS)).apply(np.round)
        finalValues['errorPPM'] = (finalValues['pressureError'].multiply(1e6/sensorFS)).apply(np.round)

        # calculate changes in final pressures between consecutive setPoints
        finalValues['pressureChange'] = round(finalValues['pressureGauge'].diff())
        # assume start of test from vented state
        finalValues['pressureChange'].iloc[0] = round(finalValues['pressureGauge'].iloc[0])

        # calculate the number of coarse control attempts required for each setpoint.
        # Nominally only 1 attempt is required, but for larger volumes or large pressure changes
        # the adiabatic at end of coarse control may exceed piston adjustment range during fine control,
        # or if there is a leak the piston may bottom out before the end of the setpoint time.
        finalValues['attempts'] = []
        for start, end in zip(startIndex, endIndex):
            finalValues['attempts'].append((data['rangeExceeded'][start:end]).sum() + 1)

        # store start and end row numbers in original data file for each setpoint data set
        finalValues['startIndex'] = startIndex
        finalValues['endIndex'] = endIndex

        # calculate pass rate based on value of stable flag at end of each setpoint
        finalValues['passRate'] = round(np.sum(finalValues['stable']) / len(finalValues['stable']) * 100)
        finalValues['attemptRate'] = round(np.average(finalValues['attempts']), 1)
        finalValues['averageError'] = round(np.average(finalValues['errorPPM']))
        finalValues['averageStability'] = round(np.average(finalValues['stabilityPPM']))

        # provide summary stats on screen
        print('Pass rate (%) = ', finalValues['passRate'], 'in ', len(finalValues['stable']), 'setpoints')
        print('Average number of setpoint attempts = ', finalValues['attemptRate'])
        print('Average error and stability (PPM FS) = ', finalValues['averageError'],
              '+/-', finalValues['averageStability'], '\n')

        # isolate start and end indices of setpoints that did not achieve stable status
        # for closer inspection
        failedSetpointIndex = list(np.where(np.array(finalValues['stable']) == 0)[0])
        failedStartIndex = list(np.array(finalValues['startIndex'])[failedSetpointIndex])
        failedEndIndex = list(np.array(finalValues['endIndex'])[failedSetpointIndex])

        # save final summary values to csv
        pd.DataFrame.from_dict(finalValues).to_csv(directory + '/' + 'finalValues' +
                                                   '_' + metaData + '.csv', index=False)

        #  make plot of final value error with standard error bars and specification limits
        tolerance = 50  # allowed control tolerance in PPM FS (USL and LSL)
        fig0 = px.scatter(finalValues, x='pressureGauge', y='errorPPM',
                          hover_data=['startIndex', 'pressureChange', 'attempts',
                                      'estimatedVolume', 'estimatedLeakRate', 'stable'],
                          error_y='stabilityPPM', title=metaData)
        fig0.add_hline(y=tolerance, line_width=3, line_dash="dash", line_color="red")
        fig0.add_hline(y=-tolerance, line_width=3, line_dash="dash", line_color="red")
        fig0.write_html(directory + '/' + 'errorPPM' + metaData + '.html', auto_open=False)

        #  make plots and save extracted data to excel file
        #  for failed setpoints where stable == 0 at end of setpoint time
        for start, end in zip(failedStartIndex, failedEndIndex):
            df = data.iloc[start:end]
            fig = px.scatter(df, x=df['elapsedTime'], y='pressureGauge',
                             hover_data=['rangeExceeded', 'estimatedVolume',
                                         'estimatedLeakRate', 'pressureError', 'stable'])
            exportFile = 'Fail' + metaData + str(start)
            fig.write_html(directory + '/' + exportFile + '.html', auto_open=False)
            df.to_excel(directory + '/' + exportFile + '.xlsx', index=False)




