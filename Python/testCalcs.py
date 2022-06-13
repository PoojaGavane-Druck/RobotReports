gaugePressure = -18.485
gaugeUncertainty = 53.711
setPoint = 1.0
passed = 0

if (((-1 * gaugeUncertainty) <= gaugePressure) and (gaugePressure <= gaugeUncertainty) and \
    ((-1 * gaugeUncertainty) < setPoint) and (setPoint <= gaugeUncertainty)):
    passed = 1

print(passed)