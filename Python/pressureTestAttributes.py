statusBits = {}
statusBits['pumpUp'] = 1 << 0
statusBits['pumpDown'] = 1 << 1
statusBits['control'] = 1 << 2
statusBits['venting'] = 1 << 3
statusBits['stable'] = 1 << 4
statusBits['vented'] = 1 << 5
statusBits['excessLeak'] = 1 << 6
statusBits['excessVolume'] = 1 << 7
statusBits['overPressure'] = 1 << 8
statusBits['excessOffset'] = 1 << 9
statusBits['measure'] = 1 << 10
statusBits['fineControl'] = 1 << 11
statusBits['pistonCentered'] = 1 << 12
statusBits['centering'] = 1 << 13
statusBits['controlledVent'] = 1 << 14
statusBits['centeringVent'] = 1 << 15
statusBits['rangeExceeded'] = 1 << 16
statusBits['coarseControlError'] = 1 << 17
statusBits['ventDirUp'] = 1 << 18
statusBits['ventDirDown'] = 1 << 19
statusBits['controlRate'] = 1 << 20
statusBits['maxLim'] = 1 << 21
statusBits['minLim'] = 1 << 22
statusBits['fastVent'] = 1 << 23
statusBits['reserved8'] = 1 << 24
statusBits['reserved7'] = 1 << 25
statusBits['reserved6'] = 1 << 26
statusBits['reserved5'] = 1 << 27
statusBits['reserved4'] = 1 << 28
statusBits['reserved3'] = 1 << 29
statusBits['reserved2'] = 1 << 30
statusBits['reserved1'] = 1 << 31


stepCheck = {}
stepCheck['one'] = 1 << 0
stepCheck['two'] = 1 << 1
stepCheck['three'] = 1 << 2
stepCheck['four'] = 1 << 3
stepCheck['five'] = 1 << 4
stepCheck['six'] = 1 << 5
stepCheck['seven'] = 1 << 6
stepCheck['eight'] = 1 << 7
stepCheck['nine'] = 1 << 8
stepCheck['ten'] = 1 << 9
stepCheck['eleven'] = 1 << 10
stepCheck['twelve'] = 1 << 11
stepCheck['thirteen'] = 1 << 12
stepCheck['fourteen'] = 1 << 13
stepCheck['fifteen'] = 1 << 14
stepCheck['sixteen'] = 1 << 15

testFailedTimeout = 100

testSteps = {}
testSteps[0] = "Key mode Setting"
testSteps[1] = "Set point Setting"
testSteps[2] = "Control mode Setting"
testSteps[3] = "Step 2 check"
testSteps[4] = "Step 4 check"
testSteps[5] = "Step 9 check"
testSteps[6] = "Step 10 check"
testSteps[7] = "Step 11 check"
testSteps[8] = "Measure mode setting"
testSteps[9] = "Step 12 check"
testSteps[10] = "Vent mode setting"
testSteps[11] = "Step 15 check"
testSteps[12] = "Step 16 check"



