
import pv624Lib as pvComms
import traceback
pv624 = []
motor = []

try:
    pv624 = pvComms.PV624()  # find STM32 main board
    # motor = mtr.Motor()  # find STM32 motor controller
except:
    print('error opening COM devices')
    traceback.print_exc()
    if pv624:
        pv624.ClosePort()
    exit()

try:
    while True:
        pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllFast()
        result = [pressure, pressureG, atmPressure, setPoint, spType, mode]
        print(result)
except:
    traceback.print_exc()
    if pv624:
        pv624.closePort()