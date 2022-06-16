# 28 Jun 2021
# parse life data acquired by finControl.py with GENIIsim2.py
# generate summary plot from final performance metrics of all data files in a directory
# generate file dialog to select outlier data files to plot separately as time series
# save all plots as html with csv base filename for later viewing


import tkinter as tk
from tkinter import filedialog as fd
import plotly.express as px
import pandas as pd
import glob
from datetime import datetime
import webbrowser


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


directory = getDirectory()
# directory = r'C:/Users/200008458/OneDrive - Baker Hughes/BHIF7DPDB3/APB/controller/Test35 - Copy'
summaryFile_P = 'summary_pressure.html'  # look for this file before computing summary stats in directory
summaryFile_t = 'summary_time.html'
summaryFile_Range = 'summary_range.html'
summaryFile_Ref = 'summary_ref.html'

htmlFiles = glob.glob(directory + '/*.html')  # all html files in directory
csvFiles = glob.glob(directory + '/*.csv')  # all csv files in directory
refFiles = []  # may be more ref files than fine or coarse if coarse control does not terminate before hold time (60s)
fineControlFiles = []
coarseControlFiles = []
for filename in csvFiles:
    f = filename.split('\\')[-1]  # discard the paths
    if '_R_' in f:
        refFiles.append(f)  # only reference files have a '_R_'
        # TBD need a way to match up ref files with fine control files
        # date stamps will be close but not identical because created in different subprocesses
        # setpoint values should match but may not be unique
    elif len(f.split('_')) > 1:
        fineControlFiles.append(f)  # fine control files have no '_R_' but have at least one _
    elif len(f.split('-')) == 5:
        coarseControlFiles.append(f)  # remaining files are control files if they have four '-'

# get statistics for summary plots vs setpoint pressure and time
computeStats = False
if not any(summaryFile_P in filename for filename in htmlFiles):
    print('generating summary plot')
    computeStats = True
else:
    response = (input('summary plot found in directory, recompute results (y)?') or 'y')
    if 'y' in response:
        computeStats = True

if computeStats:
    varE = []
    smoothE = []
    setpoint = []
    leak = []
    volume = []
    csv = []
    timestamp = []
    position = []
    rangeExceeded = []
    stable = []
    elapsedTime = []
    spType = []

    for filename in fineControlFiles:
        print(filename)
        # parse csv for final state values to generate summary plots and csv
        data = pd.read_csv(directory + '/' + filename)
        varE.append(data['varE'].iloc[-1] / data['FS'].iloc[0] * 1e6)
        smoothE.append(data['smoothE'].iloc[-1] / data['FS'].iloc[0] * 1e6)
        setpoint.append(data['setpoint'].iloc[-1])
        spType.append(data['spType'].iloc[-1])
        leak.append(data['leak'].iloc[-1])
        volume.append(data['V'].iloc[-1])
        csv.append(filename.split('\\')[-1])
        position.append(data['position'].iloc[-1])
        # extract date from filename,e.g.
        # C:/Users/200008458/Box Sync/APB/controller/M3x0.5/10000-21000_4mL\19124_2021-01-15T20-13-52.csv
        timestamp.append(datetime.strptime(filename.split('_')[-1].split('.')[0], '%Y-%m-%dT%H-%M-%S'))
        rangeExceeded.append(data['rangeExceeded'].iloc[-1])
        stable.append(data['stable'].iloc[-1])
        elapsedTime.append(data['elapsedTime'].iloc[-1])


    # convert final data values to pandas dataframe for easier plotting
    finalValues = {}
    finalValues['varE'] = varE
    finalValues['smoothE'] = smoothE
    finalValues['setpoint'] = setpoint
    finalValues['spType'] = spType
    finalValues['leak'] = leak
    finalValues['V'] = volume
    finalValues['csv'] = csv
    finalValues['timestamp'] = timestamp
    finalValues['position'] = position
    finalValues['rangeExceeded'] = rangeExceeded
    finalValues['stable'] = stable
    finalValues['elapsedTime'] = elapsedTime

    # do the same for the reference sensor data
    refVar = []
    refSetpoint = []
    ref_csv = []
    for filename in refFiles:
        print(filename)
        refData = pd.read_csv(directory + '/' + filename, header=None)
        refSetpoint.append(float(filename.split('_')[0]))
        refVar.append(refData[1].std())
        ref_csv.append(filename.split('\\')[-1])

    refFinalValues = {}
    refFinalValues['setpoint'] = refSetpoint
    refFinalValues['var'] = refVar / data['FS'].iloc[0] * 1e6
    refFinalValues['csv'] = ref_csv

    # save final summary values to csv
    pd.DataFrame.from_dict(finalValues).to_csv(directory + '/' + 'finalValues.csv', index=False)
    pd.DataFrame.from_dict(refFinalValues).to_csv(directory + '/' + 'refFinalValues.csv', index=False)

    fig0 = px.scatter(finalValues, x='setpoint',
                      y=['spType', 'varE', 'smoothE', 'leak', 'V', 'position', 'rangeExceeded',
                         'stable', 'elapsedTime'], hover_data=['csv'])
    fig1 = px.scatter(finalValues, x='timestamp', y=['setpoint', 'spType', 'varE', 'smoothE', 'leak', 'V', 'position',
                                                     'rangeExceeded', 'stable', 'elapsedTime'], hover_data=['csv'])
    fig2 = px.scatter(finalValues, x='rangeExceeded',
                      y=['setpoint', 'spType', 'varE', 'smoothE', 'leak', 'V', 'position',
                         'stable', 'elapsedTime'], hover_data=['csv'])
    fig3 = px.scatter(refFinalValues, x='setpoint',
                      y='var', hover_data=['csv'])
    fig0.write_html(directory + '/' + summaryFile_P, auto_open=True)
    fig1.write_html(directory + '/' + summaryFile_t, auto_open=True)
    fig2.write_html(directory + '/' + summaryFile_Range, auto_open=True)
    fig3.write_html(directory + '/' + summaryFile_Ref, auto_open=True)


else:
    print('Summary already in directory, not computing final stats, opening html instead')
    webbrowser.open(directory + '/' + summaryFile_P, new=0)
    webbrowser.open(directory + '/' + summaryFile_t, new=0)
    webbrowser.open(directory + '/' + summaryFile_Range, new=0)
    webbrowser.open(directory + '/' + summaryFile_Ref, new=0)

# create a file select dialog
# to select a data file for closer inspection
# and repeat until dialog is canceled by user
# save the plot as html for later review


filename = getFilename()
print(filename)
while filename:
    if filename.endswith('.html'):
        webbrowser.open(filename, new=0)  # previously plotted and saved, just open the html to save time
    else:
        data = pd.read_csv(filename)
        data['stabilityPPM'] = data['varE'] ** 0.5 / data['FS'][0] * 1e6
        data['errorPPM'] = data['E'] / data['FS'][0] * 1e6
        data['smoothErrorPPM'] = data['smoothE'] / data['FS'][0] * 1e6

        fig2 = px.scatter(data, y=['pressure', 'errorPPM', 'smoothErrorPPM', 'stabilityPPM',
                                   'V', 'leak', 'maxFakeLeakRate',
                                   'total', 'position',
                                   'measCurrent', 'rangeExceeded', 'stable', 'controlledVent', 'pumpUp', 'pumpDown',
                                   'centering'
                                   ],
                          x='elapsedTime',
                          title=filename.split('/')[-1])

        fig2.update_yaxes(title_text='various units')
        fig2.write_html(filename + '.html', auto_open=True)
    filename = getFilename()
