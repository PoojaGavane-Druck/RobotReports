#-------------------------------------------------------------------------------
# Name:        bayes
# Purpose:  adaptive parameter estimation and noise filtering using Bayes law

#Dec 22
#added prediction error correction of leak rate and varLeak_
#changed definition of targetdP in controller2.py
#Feb 01 2021
#removed vestigial code
#March 2021
#added single-point gas law volume estimation during initial approach to setpoint

#March 25 2021
#removed leak estimation with single-point gas law (use prev value)
#increased varMeasLeak estimate with gas law regression,
#using varMeasV instead of varV to prevent regression leak estimate
#from affecting stable PE leak estimate, TBD revisit this
#added dynamic IIR filter of pressure error for leak rate correction in PE mode
#added fast leak correction when measured leak is opposite sign to estimated value
#added additional restrictions for low-power mode PE adjust of volume
#TBD, add volume estimation during controlled vent and piston centering ops

# Aug 13 2021
# added additional state variables for c-code debug in estimate()

# Sept 6 2021
# updated residualL calculation to remove dependence on kP without affecting leak performance
# residualL is meant to have units of mbar/iteration, not mbar^2/step

# Sept 9 2021
# removed range estimate calcs (not used)
# removed measkP and varMeaskP (not used)
# removed redundant local variables n, residualL, measL, dV2, dP2 (replaced by bayes dataStructure)
# added volume measurement during controlled vent

# above code and changes released as V2 in-loop control algo Nov 2021 (MD)

# Nov 18 2021
# corrected error in estimate of bayes['uncerInSmoothedMeasPresErr']
# that produced negative values for varE when when E >> 0
# tightened-up stability flag criteria to 3-sigma from 5-sigma on varE and smoothE
# removed smoothE2 state variable update, not used anymore
# lookfor: # correction to varE estimate Nov 18 2021

# Apr 5 2022
# removed volume estimate during vent

# May 27 2002 added condition reduce estimatedVolume if oscillation is detected in Algo 3


# -------------------------------------------------------------------------------

import numpy as np


def estimate(bayes, screw, PID):
    # Estimate volume and leak rate using Bayesian conditional probability to merge new measurement
    # data with previous parameter estimates (aka extended Kalman filter).
    # Added empirical predictionError adjustment to volume and leak estimates for smaller/slower corrections where gas
    # law estimation methods are noisy.
    # Jun 10 2021
    # updated to use with fineControl.py and coarseControl.py modules
    # improved comment clarity and code flow

    # defaults when measV cannot be calculated
    bayes['measuredVolume'] = 0
    bayes['uncertaintyMeasuredVolume'] = bayes['maxSysVolumeEstimate']
    bayes['algorithmType'] = 0

    if PID['fineControl'] == 1:
        # calculate prediction error and error type from last control iteration
        # do this in fine control mode only

        # prediction error (mbar)
        bayes['predictionError'] = bayes['changeInPressure'] - bayes['targetdP']

        # predictionErrorType > 0 --> excessive correction, reduce system gain to fix (decrease volume estimate)
        # predictionErrorType < 0 --> insufficient correction, increase system gain to fix (increase volume estimate)
        bayes['predictionErrType'] = np.sign(bayes['targetdP']) * np.sign(bayes['predictionError'])

        # low pass filtered pressure error (mbar)
        bayes['smoothedPressureErr'] = PID['pressureError'] * bayes['lambda'] + bayes['smoothedPressureErr'] * (1 - bayes['lambda'])
        # correction to varE estimate Nov 18 2021
        # low pass filtered squared error (mbar**2)
        # (not used)
        # bayes['smoothE2'] = (PID['pressureError']**2) * bayes['lambda'] + bayes['smoothE2'] * (1-bayes['lambda'])
        # dynamic estimate of pressure error variance
        # see https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
        # http://cpsc.yale.edu/sites/default/files/files/tr222.pdf
        # Chan, Tony F.; Golub, Gene H.; LeVeque, Randall J. (1983).
        # "Algorithms for computing the sample variance: Analysis and recommendations" (PDF).
        # The American Statistician. 37 (3): 242–247. doi:10.1080/00031305.1983.10483115. JSTOR 2683386.
        # Note: varE estimate not valid when smoothE2 >> varE
        #bayes['uncerInSmoothedMeasPresErr'] = bayes['smoothE2'] - bayes['smoothedPressureErr']**2
        # Use Expontially Weighted Moving variance estimate
        # https://en.wikipedia.org/wiki/Moving_average#cite_note-13
        # Finch, Tony. "Incremental calculation of weighted mean and variance" (PDF).
        # University of Cambridge. Retrieved 19 December 2019.
        bayes['uncerInSmoothedMeasPresErr'] = (PID['pressureError'] - bayes['smoothedPressureErr'])**2 * bayes['lambda'] + bayes['uncerInSmoothedMeasPresErr'] * (1 - bayes['lambda'])

        # decide if pressure is stable, based on smoothed variance of pressure readings
        if bayes['uncerInSmoothedMeasPresErr']**0.5 < 3 * bayes['sensorUncertainty']**0.5 and bayes['smoothedPressureErr'] < 3 * bayes['sensorUncertainty']**0.5:
            # pressure is stable, within 5 sigma of sensor measurement uncertainty
            PID['stable'] = 1
        else:
            PID['stable'] = 0

    # Step (1)
    # Estimate volume and leak rate from isothermal gas law
    # Vmeas = -P * dV / (dP - Lmeas)
    # Three methods: regression, single point, or prediction error nudge

    # Regression calc:
    # Observe that Vmeas is the equation of a line: y = m*x + b
    # with y == dP, x == dV, slope m == -P / Vmeas and intercept b == Lmeas
    # A unique solution for both (Vmeas, Lmeas) from gas law regression
    # requires a minimum of two unique measurements of (dP,dV) at constant (P, Lmeas, Vmeas)

    # Single Point calc:
    # For single-point method assume Lmeas = 0 and calculate Vmeas from a single control iteration.

    # Prediction Error calc:
    # Use prediction error, smoothed prediction error, and prediction error type
    # to nudge volume and leak estimates instead of gas law calculation.
    # This is a recursive correction method than can take many control iterations to converge to correct
    # parameter estimates if the initial estimation error is large,
    # hence use of the other two methods for initial estimates
    # when relatively far from setpoint.

    # difference in pressure change from previous pressure change (mbar)
    bayes['dP2'] = bayes['changeInPressure'] - bayes['prevChangeInPressure']

    # difference in volume change from previous volume change (mL)
    bayes['dV2'] = bayes['changeInVolume'] - bayes['prevChangeInVolume']

    # adjust previous volume estimate by most recent dV
    bayes['estimatedVolume'] = bayes['estimatedVolume'] + bayes['changeInVolume']

    # uncertainty of V before combining with new measured value
    bayes['uncertaintyVolumeEstimate'] = bayes['uncertaintyVolumeEstimate'] + bayes['uncertaintyVolumeChange']

    if abs(bayes['dV2']) > 10 * bayes['uncertaintyVolumeChange']**0.5 and abs(bayes['dP2']) > bayes['uncertaintyPressureDiff']**0.5 and \
            PID['fineControl'] == 1:
        bayes['algorithmType'] = 1
        # print('*')
        # Estimate volume with linear fit to gas law equation.
        # Done only during fine control where
        # the two most recent measurements (dP, dV) and (dP_, dV_)
        # are significantly different from each other so that
        # slope and intercept of a regression line are uniquely determined.
        # This method tends to underestimate volume if dV2 is small
        # or at low pressures because of non-linear noise-shaping near the gas-law equation singularity.
        # Unclear how to improve it, hence restricting method to "large" dV2 only, where the
        # method gives more accurate estimates.
        # Empirical results indicate regression method leak estimate is
        # very noisy compared to the volume estimate.
        # Unclear why, possibly because leak parameter effect is less observable in short-term data,
        # and so cannot be accurately estimated from only two consecutive measurements.  Adiabatic
        # decay also looks like a large, time-varying leak,
        # which would lead to an inaccurate estimate of the steady-state leak rate.
        # The long-term effect of an inaccurate leak estimate can be substantial and take a long time to decay to zero
        # via the prediction error adjustment method.  It is therefore safer to assume the leak rate is zero and adjust
        # it up rather than adjust a large inaccurate value down to zero.  If the leak rate is truly a large value
        # then the zero-assumption will cause longer settling time, but for a good reason (there is large leak).
        # Hence, ignore the regression method leak estimate and just use the volume estimate.  This minimizes the
        # effect of transient adiabatic effects or large leaks on volume estimation error.
        # Steady-state leak estimation done by another method (filtered prediction error) in Step 2.

        # measured volume estimate with linear fit (slope), (mL)
        bayes['measuredVolume'] = -bayes['measuredPressure'] * (bayes['dV2'] / bayes['dP2'])

        # uncertainty in measured volume estimate with Bayes regression (mL)
        bayes['uncertaintyMeasuredVolume'] = bayes['sensorUncertainty'] * (bayes['dV2']/bayes['dP2'])**2 + \
                            2 * bayes['uncertaintyPressureDiff'] * (bayes['measuredPressure'] * bayes['dV2'] / bayes['dP2']**2)**2 + \
                            2 * bayes['uncertaintyVolumeChange'] * (bayes['measuredPressure'] / bayes['dP2'])**2

    elif abs(bayes['changeInVolume']) > 10 * bayes['uncertaintyVolumeChange']**0.5 and abs(bayes['changeInPressure']) > bayes['uncertaintyPressureDiff']**0.5:
        bayes['algorithmType'] = 2
        # print('**')
        # Estimate volume with gas law equation, assuming leak/adiabatic effect is zero.
        # Done when far from setpoint and during coarse control piston centering.
        # Valid when volume adjustment is large compared to leak rate effect or adiabatic,
        # e.g. when moving at max speed towards setpoint.
        # This estimate is has better SNR than other methods because dP and dV are large,
        # but dP will be a time-averaged value over the
        # duration of dV adjustment, rather than the final value (integration error).
        # If the applied dV is repeated (i.e. motor rotating at max speed) then
        # the error from pressure sensor integration time is greatly reduced
        # because lag-error in previous measured dP cancels in the following one when it is the same value
        # as before.
        # The range estimate is most accurate in this case, which allows for
        # sensible decisions on when a setpoint is achievable long before reaching
        # the maximum extent of piston travel.  The method is also useful when a linear fit is not possible
        # because of repeated dP and dV.
        # Since the leak / adiabatic is assumed to be zero, this method
        # can yield inaccurate volume estimates when the transient adiabatic pressure change is large compared
        # to the isothermal pressure change.  However, on final approach to setpoint adiabatic effect
        # should be addressed by the regression calculation method that adjusts measured volume for leak and
        # adiabatic effects.

        # measured volume (mL), ignore negative volume estimates
        # negative values can result from a large adiabatic or
        # leak rate effects when (dV,dP) are relatively small
        bayes['measuredVolume'] = max(-bayes['measuredPressure'] * (bayes['changeInVolume'] / bayes['changeInPressure']), 0)

        # uncertainty in measured volume (mL)
        bayes['uncertaintyMeasuredVolume'] = bayes['sensorUncertainty'] * (bayes['changeInVolume'] / bayes['changeInPressure']) ** 2 + \
                            bayes['uncertaintyPressureDiff'] * (bayes['measuredPressure'] * bayes['changeInVolume'] / bayes['changeInPressure'] ** 2) ** 2 + \
                            bayes['uncertaintyVolumeChange'] * (bayes['measuredPressure'] / bayes['changeInPressure']) ** 2

    elif PID['fineControl'] == 1:
        # use Prediction Error method to slowly adjust estimates of volume and leak rate
        # during fine control loop when close to setpoint
        bayes['algorithmType'] = 3
        # print('***')

        # measured residual leak rate from steady-state pressure error (mbar / iteration)
        # residualL = -bayes['smoothedPressureErrForPECorrection'] / bayes['estimatedKp']
        bayes['residualLeakRate'] = -bayes['smoothedPressureErrForPECorrection'] / bayes['numberOfControlIterations']  # fixed error in calc

        # estimate of true leak rate magnitude (mbar/iteration)
        bayes['measuredLeakRate1'] = abs(bayes['residualLeakRate'] + bayes['estimatedLeakRate'])

        if bayes['predictionErrType'] == 1:
            # Excessive gain, decrease V and increase leak estimates to keep same net leak rate control effect.
            # Decrease volume estimate only when steady state pressure error and leak have the same sign
            # and the measured error in leak rate estimate is less than +/- 50%
            # Otherwise keep volume estimate as is until leak estimate is closer to steady state value
            # to prevent underestimating volume when leak is inaccurate (can cause sluggish control).
            # The effect of a leak on PE will grow if a small dV correction has been skipped, e.g. in
            # low-power mode, so do not make volume estimate corrections unless the
            # the control has applied a correction at least twice consecutively (dV > 0 and dV_ > 0)
            if bayes['residualLeakRate'] * bayes['estimatedLeakRate'] >= 0 and bayes['measuredLeakRate1'] < 2 * abs(bayes['estimatedLeakRate']) and bayes['changeInVolume'] != 0 and bayes[
                    'prevChangeInVolume'] != 0:
                bayes['measuredVolume'] = bayes['estimatedVolume'] * bayes['gamma']
                bayes['uncertaintyMeasuredVolume'] = bayes['uncertaintyVolumeEstimate']
                bayes['estimatedLeakRate'] = bayes['estimatedLeakRate'] / bayes['gamma']
                # print('***-')

        if bayes['predictionErrType'] == -1:
            # Insufficient gain, increase V and decrease leak estimates to keep same net leak rate control effect.
            # Increase volume estimate only when steady state pressure error and leak have the same sign
            # and the measured error in leak rate estimate is less than +/- 50%
            # Otherwise keep volume estimate as is until L estimate is closer to steady state value
            # to prevent overestimating volume when leak is inaccurate (can cause control oscillation).
            # The effect of a leak on PE will be amplified if no correction has been made (leak is not being fed,
            # low-power mode), so do not make volume estimate corrections unless the
            # the control has applied a correction at least twice in a row (dV >0 and dV_ > 0)
            # May 27 2002 added condition reduce estimatedVolume if oscillation is detected
            if np.sign(bayes['changeInVolume']) != np.sign(bayes['prevChangeInVolume']) and \
                    abs(PID['pressureError']) > 2 * bayes['sensorUncertainty']**0.5:
                # Decrease volume estimate aggressively if significant oscillation detected to help quench it quickly.
                # Required because the open loop system appears to have a loop delay in pressure response to
                # volume correction under certain conditions, which makes volume nudge by prediction error type
                # adjust the volume in the wrong direction.
                bayes['measuredVolume'] = bayes['estimatedVolume'] * bayes['gamma']**2
                bayes['uncertaintyMeasuredVolume'] = bayes['uncertaintyVolumeEstimate']
                bayes['estimatedLeakRate'] = bayes['estimatedLeakRate'] * bayes['gamma']**2
                print('***osc***')

            elif bayes['residualLeakRate'] * bayes['estimatedLeakRate'] >= 0 and \
                    bayes['measuredLeakRate1'] < 2 * abs(bayes['estimatedLeakRate']) and \
                    bayes['changeInVolume'] != 0 and \
                    bayes['prevChangeInVolume'] != 0:
                bayes['measuredVolume'] = bayes['estimatedVolume'] / bayes['gamma']
                bayes['uncertaintyMeasuredVolume'] = bayes['uncertaintyVolumeEstimate']
                bayes['estimatedLeakRate'] = bayes['estimatedLeakRate'] * bayes['gamma']
                # print('***+')

    # combine prior volume estimate with latest measured volume, if available
    if bayes['measuredVolume'] != 0:
        # probability weighted average of prior and measured values (mL)
        bayes['estimatedVolume'] = (bayes['measuredVolume'] * bayes['uncertaintyVolumeEstimate'] + bayes['estimatedVolume'] * bayes['uncertaintyMeasuredVolume']) / (bayes['uncertaintyMeasuredVolume'] + bayes['uncertaintyVolumeEstimate'])

        # new uncertainty in estimated volume (mL)
        bayes['uncertaintyVolumeEstimate'] = bayes['uncertaintyMeasuredVolume'] * bayes['uncertaintyVolumeEstimate'] / (bayes['uncertaintyMeasuredVolume'] + bayes['uncertaintyVolumeEstimate'])

        # increase varV if measV is not within maxZScore standard deviations of V
        # increases convergence rate to true value in the event of a step change in system volume
        du = abs(bayes['estimatedVolume'] - bayes['measuredVolume'])
        bayes['uncertaintyVolumeEstimate'] = max(bayes['uncertaintyVolumeEstimate'], du / bayes['maxZScore'] - bayes['uncertaintyMeasuredVolume'])

    # bound updated volume estimate
    bayes['estimatedVolume'] = max(min(bayes['estimatedVolume'], bayes['maxSysVolumeEstimate']), bayes['minSysVolumeEstimate'])

    # Step 2
    # Adjust leak rate estimate

    if bayes['algorithmType'] == 3:
        # Use dynamically averaged pressure error when near setpoint to adjust leak rate.
        # Leak rate averaging window length is inversely proportional to the
        # larger of the estimated leak rate or pressure error.
        # This makes the filter react faster when either the pressure error or
        # leak rate estimate is "large" to improve convergence rate to true values,
        # while providing accurate leak rate correction for small leaks.

        # Use larger of pressure error or leak rate estimate to define IIR filter length
        # to use on residual pressure error smoothE_PE
        # larger values means shorter observation time / faster response
        # limit filter length to maxN control iterations
        # Average smoothE_PE over more iterations when leak rate estimate is small and averaged pressure error is small
        # so that the expected value of the pressure error from a leak
        # is at least as big as pressure measurement noise.

        minError = bayes['uncertaintyPressureDiff'] ** 0.5 / bayes['maxIterationsForIIRfilter']
        maxError = max(abs(bayes['smoothedPressureErrForPECorrection']), abs(bayes['estimatedLeakRate']), minError)

        # number of control iterations to average over (minN,maxN)
        bayes['numberOfControlIterations'] = max(int(bayes['uncertaintyPressureDiff']**0.5 / maxError), bayes['minIterationsForIIRfilter'])

        # IIR filter memory factor to achieve epsilon * initial condition residual after n iterations
        bayes['alpha'] = 10**(bayes['log10epsilon']/bayes['numberOfControlIterations'])

        # smoothed pressure error with dynamic IIR filter (mbar)
        bayes['smoothedPressureErrForPECorrection'] = (1-bayes['alpha']) * PID['pressureError'] + bayes['alpha'] * bayes['smoothedPressureErrForPECorrection']

        # measured residual leak rate from steady-state pressure error (mbar/iteration)
        # residualL = -bayes['smoothedPressureErrForPECorrection'] / bayes['estimatedKp']
        bayes['residualLeakRate'] = -bayes['smoothedPressureErrForPECorrection'] / bayes['numberOfControlIterations']  # fixed error

        # Apply 1/n of estimated leak correction to reduce remaining leak estimate error gradually over n iterations.
        # Correction to leak estimate must be gradual because leak effect takes time to accurately measure.
        # Applying entire dL correction to leak rate after
        # each control iteration could cause sustained oscillation in pressure error,
        # similar to "integrator windup" in a PI controller when the "kI" term is too large.
        # This controller's correction for leak rate is a feed-forward parameter
        # with leak rate estimation error inferred from smoothed pressure error.
        # At steady-state, feed-forward control allows the controller to anticipate the effect
        # of the leak before it accumulates as a significant pressure error, using estimated leak rate
        # to compensate for future effects.  This is particularly helpful for minimizing
        # steady-state pressure error for leak rates greater than the measurement
        # noise when the iteration rate of the control loop is slow and leak effect is significant on each iteration.
        bayes['changeToEstimatedLeakRate'] = bayes['residualLeakRate'] / bayes['numberOfControlIterations']

        # Special cases:

        if bayes['residualLeakRate'] * bayes['estimatedLeakRate'] < 0:
            # Residual leak estimate error is opposite sign to estimated leak rate.
            # It could take a long time to correct this estimation error while the
            # pressure error continues to accumulate.
            # Speed-up convergence to the correct leak rate value by removing all dL in one iteration
            # instead of n iterations.
            # This may temporarily cause the pressure error to
            # oscillate/overshoot, but reduces the maximum error
            # that would otherwise accumulate by use of significantly incorrect leak rate estimate.
            # This situation could happen if the true leak rate is suddenly changed
            # while hovering around setpoint, e.g. by user stopping the leak, or by a decaying adiabatic condition.
            bayes['estimatedLeakRate'] = bayes['estimatedLeakRate'] + bayes['changeToEstimatedLeakRate'] * bayes['numberOfControlIterations']
            # print('***L++')

        else:
            # Residual pressure error is the same sign as estimated leak.
            # Adjust leak rate estimate a small amount to minimize its effect
            bayes['estimatedLeakRate'] = bayes['estimatedLeakRate'] + bayes['changeToEstimatedLeakRate']
            # Note: If dV == 0 because of low-power mode control operation then the leak rate correction will grow
            # larger than the "correct" value because the leak effect is not being offset with feed-forward control.
            # Prolonged operation with non-zero leak rate and dV == 0 may cause leak estimate to increase
            # above true value.  This is ok because effectively the control loop iteration time is increased as well,
            # and so the leak rate per control iteration is truly increasing.  However, if there is a disturbance
            # the controller will revert back to a faster iteration rate, which may cause temporary oscillation while
            # the leak rate estimate is nudged back down.

        # varLeak the variance of low-pass filtered pressure sensor noise,
        # which decreases as 1/n for large n (Poisson statistics)
        # n is the low-pass filter observation time (iterations)
        bayes['uncertaintyEstimatedLeakRate'] = (bayes['uncertaintyPressureDiff'] / bayes['numberOfControlIterations'])

        # increase leak uncertainty if measLeak is not within maxZScore standard deviations of leak
        # increases convergence to true value in the event of a step change in leak rate
        du = abs(bayes['estimatedLeakRate'] - bayes['measuredLeakRate'])
        bayes['uncertaintyEstimatedLeakRate'] = max(bayes['uncertaintyEstimatedLeakRate'], du / bayes['maxZScore'] - bayes['uncertaintyMeasuredLeakRate'])

    else:
        # volume estimated with gas law
        # keep previous leak estimate
        bayes['estimatedLeakRate'] = bayes['estimatedLeakRate']
        bayes['uncertaintyEstimatedLeakRate'] = bayes['uncertaintyEstimatedLeakRate']

    # bound leak correction to reasonable values, scaled by volume estimate and pressure
    # higher pressure and lower volume can have larger leak rate estimate
    bayes['maxEstimatedLeakRate'] = screw['maxLeak'] / bayes['estimatedVolume'] * bayes['measuredPressure'] / screw['maxP'] * screw['nominalV']
    bayes['estimatedLeakRate'] = min(max(bayes['estimatedLeakRate'], -bayes['maxEstimatedLeakRate']), bayes['maxEstimatedLeakRate'])

    # Calculate optimal controller gain for next control iteration
    # optimal kP to achieve correction of pressure error in one step (steps / mbar)
    bayes['estimatedKp'] = bayes['estimatedVolume'] / (bayes['measuredPressure'] * screw['dV'])


if __name__ == '__main__':
    print('no tests defined')
