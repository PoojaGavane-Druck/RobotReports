/**
* Baker Hughes Confidential
* Copyright 2020.  Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file		Controller.cpp
* @version	1.00.00
* @author	Makarand Deshmukh
* @date		2021-07-21
*
* @brief	Control algorithm coarse control source file
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ControlAlgorithm.h"
#include "Controller.h"
#include "math.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

#define CPP_ON_PC 

extern float controllerData[NO_OF_HEADINGS];
extern uint32_t opticalSensorAdcCounts;
extern uint32_t currentPistonPosition;

extern char coarseControlFileName[100];
extern char fineControlFileName[100];

#define max(a,b) (a>b?a:b)
#define min(a,b) (a<b?a:b)

static const float EPSILON = (float)1E-10;  //arbitrary 'epsilon' value
static const float gaugeSensorUncertainty = (float)20.0; //uncertainty gage sensor pressure
static const float piValue = (float)3.14159;

#ifdef CPP_ON_PC
string resultFilePath = "138_1_2021-06-28T16-03-21_RES.csv";
#endif
/*
 * @brief Compare two floating point numbers for equality
 * @param a - first floating point value
 * @param b - second floating point value
 * @return true if the difference between the two values is less than EPSILON, else false
 */
bool floatEqual(float a, float b)
{
    return (fabsf(a - b) < EPSILON) ? (bool)true : (bool)false;
}

controller::controller()
{
    ventValve = new DValve();

    valve2 = new DValve();

    valve3 = new DValve();

    motor = new DStepperMotor();

    pressureSetPoint = 0.0f;
    setPointG = 0.0f;
    absolutePressure = 0.0f;
    gaugePressure = 0.0f;
    atmosphericPressure = 0.0f;
    controllerStatus.bytes = (uint32_t)0;
    sensorFsValue = (float)20000;
    fsValue = (float)7000;
    ventReadingNum = (eControlVentReading_t)eControlVentGetFirstReading;

    initialize();
}
void controller::initialize(void)
{
#ifdef CPP_ON_PC
    strcpy_s(coarseControlLogParams.fileName, coarseControlFileName);
    strcpy_s(fineControlLogParams.fileName, fineControlFileName);
#endif
    initPidParams();
    initMotorParams();
    initScrewParams();
    initBayesParams();
    initTestParams();

 }
void controller::setMeasure(void)
{
    //# isolate pump for measure
    ventValve->triggerValve(VALVE_STATE_OFF);   //# isolate vent port
    valve2->triggerValve(VALVE_STATE_OFF);    //# isolate pump inlet, should isolate external source too if vacuum
    valve3->triggerValve(VALVE_STATE_OFF);     //# isolate pump outlet, should isolate external source too if pressure
   
    controllerStatus.bit.control = 0;
    controllerStatus.bit.measure = 1;
    controllerStatus.bit.venting = 0;
    controllerStatus.bit.vented = 0;
    controllerStatus.bit.pumpUp = 0;
    controllerStatus.bit.pumpDown = 0;
    controllerStatus.bit.centering = 0;
    controllerStatus.bit.controlledVent = 0;
    controllerStatus.bit.centeringVent = 0;
    controllerStatus.bit.ventDir = 0;
    
}
void controller::setControlUp(void)
{
    // set for pump up
    ventValve->triggerValve(VALVE_STATE_OFF);   //# isolate vent port
    valve2->triggerValve(VALVE_STATE_OFF);  //# isolate pump inlet
    valve3->triggerValve(VALVE_STATE_ON);//# connect pump outlet
   
    controllerStatus.bit.control = 1;
    controllerStatus.bit.measure = 0;
    controllerStatus.bit.venting = 0;
    controllerStatus.bit.vented = 0;
    controllerStatus.bit.pumpUp = 1;
    controllerStatus.bit.pumpDown = 0;
    controllerStatus.bit.centering = 0;
    controllerStatus.bit.controlledVent = 0;
    controllerStatus.bit.centeringVent = 0;
    controllerStatus.bit.ventDir = 0;
   
}
void controller::setControlDown(void)
{
    // set for pump down, TBD safety check initial pressure vs atmosphere
    ventValve->triggerValve(VALVE_STATE_OFF);     //# isolate vent port
    valve2->triggerValve(VALVE_STATE_OFF);   //# connect pump inlet
    valve3->triggerValve(VALVE_STATE_ON);    //# isolate pump outlet
        
    controllerStatus.bit.control = 1;
    controllerStatus.bit.measure = 0;
    controllerStatus.bit.venting = 0;
    controllerStatus.bit.vented = 0;
    controllerStatus.bit.pumpUp = 0;
    controllerStatus.bit.pumpDown = 1;
    controllerStatus.bit.centering = 0;
    controllerStatus.bit.controlledVent = 0;
    controllerStatus.bit.centeringVent = 0;
    controllerStatus.bit.ventDir = 0;
        
}
void controller::setControlIsolate(void)
{
    //# isolate pump for screw control
    ventValve->triggerValve(VALVE_STATE_OFF);    //# isolate vent port
    valve2->triggerValve(VALVE_STATE_OFF);   //# isolate pump inlet
    valve3->triggerValve(VALVE_STATE_OFF);   //# isolate pump outlet
    
    controllerStatus.bit.control = 1;
    controllerStatus.bit.measure = 0;
    controllerStatus.bit.venting = 0;
    controllerStatus.bit.vented = 0;
    controllerStatus.bit.pumpUp = 0;
    controllerStatus.bit.pumpDown = 0;
    controllerStatus.bit.centering = 0;
    controllerStatus.bit.controlledVent = 0;
    controllerStatus.bit.centeringVent = 0;
    controllerStatus.bit.ventDir = 0;
}
void controller::setControlCentering(void)
{
    //# isolate pump for centering piston
    ventValve->triggerValve(VALVE_STATE_OFF);   //# isolate vent port
    valve2->triggerValve(VALVE_STATE_OFF);    //# isolate pump inlet
    valve3->triggerValve(VALVE_STATE_OFF);   //# isolate pump outlet

    controllerStatus.bit.control = 1;
    controllerStatus.bit.measure = 0;
    controllerStatus.bit.venting = 0;
    controllerStatus.bit.vented = 0;
    controllerStatus.bit.pumpUp = 0;
    controllerStatus.bit.pumpDown = 0;
    controllerStatus.bit.centering = 1u;
    controllerStatus.bit.controlledVent = 0;
    controllerStatus.bit.centeringVent = 0;
    controllerStatus.bit.ventDir = 0;
}
void controller::setVent(void)
{
    //# isolate pumpand vent
    valve2->triggerValve(VALVE_STATE_OFF);  //# isolate pump inlet
    valve3->triggerValve(VALVE_STATE_OFF);   //# isolate pump outlet
    ventValve->triggerValve(VALVE_STATE_ON);  //# connect vent port
    
    controllerStatus.bit.control = 0;
    controllerStatus.bit.measure = 0;
    controllerStatus.bit.venting = 1;
    controllerStatus.bit.vented = 0;
    controllerStatus.bit.pumpUp = 0;
    controllerStatus.bit.pumpDown = 0;
    controllerStatus.bit.centering = 0;
    controllerStatus.bit.controlledVent = 0;
    controllerStatus.bit.centeringVent = 0;
    controllerStatus.bit.ventDir = 0;
}
void controller::setControlVent(void)
{
    //# isolate pump for controlled vent to setpoint
    valve2->triggerValve(VALVE_STATE_OFF);   //# isolate pump inlet
    valve3->triggerValve(VALVE_STATE_OFF);   //# isolate pump outlet
    ventValve->triggerValve(VALVE_STATE_ON);  //# connect vent port

    controllerStatus.bit.control = 1;
    controllerStatus.bit.measure = 0;
    controllerStatus.bit.venting = 0;  //# set to 0 because vent was not requested by GENII
    controllerStatus.bit.pumpUp = 0;
    controllerStatus.bit.pumpDown = 0;
    controllerStatus.bit.centering = 0;
    controllerStatus.bit.controlledVent = 1;
    controllerStatus.bit.centeringVent = 0;
    controllerStatus.bit.ventDir = 0;
}

float getSign(float value)
{
	float sign = 0.0f;

	if (value == 0.0f)
	{
		sign = 0;
	}
	else if (value > 0.0f)
	{
		sign = 1.0;
	}
	else
	{
		sign = -1.0;
	}

	return sign;
}

void controller::estimate(void)
{
    /*
    # Estimate volumeand leak rate using Bayesian conditional probability to merge new measurement
    # data with previous parameter estimates(aka extended Kalman filter).
    # Added empirical predictionError adjustment to volumeand leak estimates for smaller / slower corrections where gas
    # law estimation methods are noisy.
    # Jun 10 2021
    # updated to use with fineControl.py and coarseControl.py modules
    # improved comment clarity and code flow

    # defaults when measV cannot be calculated
    */
    float residualL = 0.0f;

    bayesParams.measuredVolume = 0u;
    bayesParams.uncertaintyMeasuredVolume = bayesParams.maxSysVolumeEstimateValue;
    bayesParams.algorithmType = eRegressionMethod;

    if (1u == controllerStatus.bit.fineControl)
    {
        //# do this in fine control mode only
        //# calculate prediction errorand error type from last control iteration
        //# prediction error(mbar)
        bayesParams.predictionError = bayesParams.changeInPressure - bayesParams.targetdP;

        //# predictionErrorType > 0 -- > excessive correction, reduce system gain to fix(decrease volume estimate)
        //# predictionErrorType < 0 --> insufficient correction, increase system gain to fix(increase volume estimate)
        //bayesParams.predictionErrType = np.sign(bayes['targetdP']) * np.sign(bayes['predictionError'])
		float signTargetDp = 0.0f;
		float signPredictionError = 0.0f;

		signTargetDp = getSign(bayesParams.targetdP);
		signPredictionError = getSign(bayesParams.predictionError);

		bayesParams.predictionErrType = signTargetDp * signPredictionError;

        //# low pass filtered pressure error(mbar)
        bayesParams.smoothedPressureErr = pidParams.pressureError * bayesParams.lambda + bayesParams.smoothedPressureErr * (1 - bayesParams.lambda);

        //# low pass filtered squared error(mbar * *2)
        bayesParams.smoothedSqaredPressureErr = (pidParams.pressureError * pidParams.pressureError) * bayesParams.lambda + bayesParams.smoothedSqaredPressureErr * (1 - bayesParams.lambda);

        //# dynamic estimate of pressure error variance
        //# see https ://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
        //# Note : varE estimate not valid when smoothE2 >> varE
        //bayes['varE'] = bayes['smoothE2'] - bayes['smoothE'] * *2
        bayesParams.uncerInSmoothedMeasPresErr = bayesParams.smoothedSqaredPressureErr - (bayesParams.smoothedPressureErr * bayesParams.smoothedPressureErr);

        //# decide if pressure is stable, based on smoothed variance of pressure readings
        //if bayes['varE'] * *0.5 < 5 * bayes['varP'] * *0.5 and bayes['smoothE'] < 5 * bayes['varP'] * *0.5:
		if (((sqrt(bayesParams.uncerInSmoothedMeasPresErr)) < (5.0f * sqrt(bayesParams.sensorUncertainity))) &&
			(bayesParams.smoothedPressureErr < (5.0f * sqrt(bayesParams.sensorUncertainity))))
		{
			//# pressure is stable, within 5 sigma of sensor measurement uncertainty
			controllerStatus.bit.stable = 1u;
		}
		else
		{
			controllerStatus.bit.stable = 0u;
		}

		/*
        # Step(1)
        # Estimate volume and leak rate from isothermal gas law
        # Vmeas = -P * dV / (dP - Lmeas)
        # Three methods : regression, single point, or prediction error nudge
		*/

		/*
		# Regression calc :
		# Observe that Vmeas is the equation of a line : y = m * x + b
		# with y == dP, x == dV, slope m == -P / Vmeas and intercept b == Lmeas
		# A unique solution for both(Vmeas, Lmeas) from gas law regression
		# requires a minimum of two measurements(dP, dV) at constant(P, Lmeas, Vmeas)

		# Single Point calc :
		# For single - point method assume Lmeas = 0 and calculate Vmeas from a single control iteration.

		# Prediction Error calc :
		# Use prediction error, smoothed prediction error, and prediction error type
		# to nudge volumeand leak estimates instead of gas law calculation.
		# This is a recursive correction method than can take many control iterations to converge to correct
		# parameter estimates if the initial estimation error is large,
		# hence use of the other two methods for initial estimates
		# when relatively far from setpoint.

		# difference in pressure change from previous pressure change(mbar)
		*/
        float dP2, dV2;
        dP2 = bayesParams.changeInPressure - bayesParams.prevChangeInPressure;
		bayesParams.dP2 = dP2;

        //# difference in volume change from previous volume change(mL)
        dV2 = bayesParams.changeInVolume - bayesParams.prevChangeInVolume; // bayes['dV'] - bayes['dV_']
		bayesParams.dV2 = dV2;

        //# adjust previous volume estimate by most recent dV
        bayesParams.estimatedVolume = bayesParams.estimatedVolume + bayesParams.changeInVolume;  //bayes['V'] = bayes['V'] + bayes['dV']

        //# uncertainty of V before combining with new measured value
        bayesParams.uncertaintyVolumeEstimate = bayesParams.uncertaintyVolumeEstimate + bayesParams.uncertaintyVolumeChange;  //bayes['varV'] = bayes['varV'] + bayes['vardV']

        //if abs(dV2) > 10 * bayes['vardV'] * *0.5 and abs(dP2) > bayes['vardP'] * *0.5 and PID['fineControl'] == 1:
        if ((fabs(dV2) > (10.0f * sqrt(bayesParams.uncertaintyVolumeChange))) &&
            (fabs(dP2) > sqrt(bayesParams.uncertaintyPressureDiff)) &&
            ((1u == controllerStatus.bit.fineControl)))
        {
            bayesParams.algorithmType = eRegressionMethod; //bayes['algoV'] = 1

            /*
            # print('*')
            # Estimate volume with linear fit to gas law equation.
            # Done only during fine control where
            # the two most recent measurements(dP, dV) and (dP_, dV_)
            # are significantly different from each other so that
            # slope and intercept of a regression line are uniquely determined.
            # This method tends to underestimate volume if dV2 is small
            # or at low pressures because of non - linear noise - shaping near the gas - law equation singularity.
            # Unclear how to improve it, hence restricting method to "large" dV2 only, where the
            # method gives more accurate estimates.
            # Empirical results indicate regression method leak estimate is
            # very noisy compared to the volume estimate.
            # Unclear why, possibly because leak parameter effect is less observable in short - term data,
            #and so cannot be accurately estimated from only two consecutive measurements.Adiabatic
            # decay also looks like a large, time - varying leak,
            # which would lead to an inaccurate estimate of the steady - state leak rate.
            # The long - term effect of an inaccurate leak estimate can be substantial and take a long time to decay to zero
            # via the prediction error adjustment method.It is therefore safer to assume the leak rate is zero and adjust
            # it up rather than adjust a large inaccurate value down to zero.If the leak rate is truly a large value
            # then the zero - assumption will cause longer settling time, but for a good reason(there is large leak).
            # Hence, ignore the regression method leak estimateand just use the volume estimate.This minimizes the
            # effect of transient adiabatic effects or large leaks on volume estimation error.
            # Steady - state leak estimation done by another method(filtered prediction error) in Step 2.

            # measured volume estimate with linear fit(slope), (mL)
            */
            bayesParams.measuredVolume = -bayesParams.measuredPressure * (dV2 / dP2); //bayes['measV'] = -bayes['P'] * (dV2 / dP2)

                //# uncertainty in measured volume estimate with Bayes regression(mL)
                //bayes['varMeasV'] = bayes['varP'] * (dV2 / dP2) * *2 + \
                //2 * bayes['vardP'] * (bayes['P'] * dV2 / dP2 * *2) * *2 + \
                //2 * bayes['vardV'] * (bayes['P'] / dP2) * *2
            float dp2Sqare = 0.0f;
            float temporaryVariable1 = 0.0f;
            float temporaryVariable2 = 0.0f;
			float temporaryVariable3 = 0.0f;

			/* Updated by mak on 13/08/2021 */
			temporaryVariable1 = dV2 / dP2;
			temporaryVariable1 = temporaryVariable1 * temporaryVariable1;
			temporaryVariable1 = bayesParams.sensorUncertainity * temporaryVariable1;

			temporaryVariable2 = dP2 * dP2;
			temporaryVariable2 = dV2 / temporaryVariable2;
			temporaryVariable2 = bayesParams.measuredPressure * temporaryVariable2;
			temporaryVariable2 = temporaryVariable2 * temporaryVariable2;
			temporaryVariable2 = bayesParams.uncertaintyPressureDiff * temporaryVariable2;
			temporaryVariable2 = 2.0f * temporaryVariable2;

			temporaryVariable3 = bayesParams.measuredPressure / dP2;
			temporaryVariable3 = temporaryVariable3 * temporaryVariable3;
			temporaryVariable3 = bayesParams.uncertaintyVolumeChange * temporaryVariable3;
			temporaryVariable3 = 2.0f * temporaryVariable3;

			bayesParams.uncertaintyMeasuredVolume = temporaryVariable1 + temporaryVariable2 + temporaryVariable3;

			/*
            dp2Sqare = dP2 * dP2;
            temporaryVariable1 = bayesParams.sensorUncertainity * (dV2 / dP2) * (dV2 / dP2);
            temporaryVariable2 = 2.0f * bayesParams.uncertaintyPressureDiff * (bayesParams.measuredPressure * dV2 / (dp2Sqare)) * (bayesParams.measuredPressure * dV2 / (dp2Sqare));
            temporaryVariable3 = 2 * bayesParams.uncertaintyVolumeChange * (bayesParams.measuredPressure / dP2) * (bayesParams.measuredPressure / dP2);
            bayesParams.uncertaintyMeasuredVolume = temporaryVariable1 + temporaryVariable2 + temporaryVariable3;
			*/
        }

        //elif abs(bayes['dV']) > 10 * bayes['vardV'] * *0.5 and abs(bayes['dP']) > bayes['vardP'] * *0.5:
        else if ((fabs(bayesParams.changeInVolume) > (10.0f * sqrt((double)bayesParams.uncertaintyVolumeChange))) 
						&& (fabs((double)bayesParams.changeInPressure) > sqrt(bayesParams.uncertaintyPressureDiff)))
        {
            bayesParams.algorithmType = eGasLawMethod; //bayes['algoV'] = 2
            /*
            # print('**')
            # Estimate volume with gas law equation, assuming leak / adiabatic effect is zero.
            # Done when far from setpoint and during coarse control piston centering.
            # Valid when volume adjustment is large compared to leak rate effect or adiabatic,
            # e.g.when moving at max speed towards setpoint.
            # This estimate is has better SNR than other methods because dPand dV are large,
            # but dP will be a time - averaged value over the
            # duration of dV adjustment, rather than the final value(integration error).
            # If the applied dV is repeated(i.e.motor rotating at max speed) then
            # the error from pressure sensor integration time is greatly reduced
            # because lag - error in previous measured dP cancels in the following one when it is the same value
            # as before.
            # The range estimate is most accurate in this case, which allows for
            # sensible decisions on when a setpoint is achievable long before reaching
            # the maximum extent of piston travel.The method is also useful when a linear fit is not possible
            # because of repeated dPand dV.
            # Since the leak / adiabatic is assumed to be zero, this method
            # can yield inaccurate volume estimates when the transient adiabatic pressure change is large compared
            # to the isothermal pressure change.However, on final approach to setpoint adiabatic effect
            # should be addressed by the regression calculation method that adjusts measured volume for leakand
            # adiabatic effects.

            # measured volume(mL), ignore negative volume estimates
            # negative values can result from a large adiabatic or
            # leak rate effects when(dV, dP) are relatively small
            */
            //bayes['measV'] = max(-bayes['P'] * (bayes['dV'] / bayes['dP']), 0)
            bayesParams.measuredVolume = fmax((-1.0f) * bayesParams.measuredPressure * (bayesParams.changeInVolume / bayesParams.changeInPressure), 0.0);
            //# uncertainty in measured volume(mL)
            //bayes['varMeasV'] = bayes['varP'] * (bayes['dV'] / bayes['dP']) * *2 + \
            //bayes['vardP'] * (bayes['P'] * bayes['dV'] / bayes['dP'] * *2) * *2 + \
            //bayes['vardV'] * (bayes['P'] / bayes['dP']) * *2
            float dp2Sqare = 0.0f;
            float temporaryVariable1 = 0.0f;
            float temporaryVariable2 = 0.0f;
            float temporaryVariable3 = 0.0f;
            dp2Sqare = dP2 * dP2;

			/* Modified by mak on 12/08/2021*/
			temporaryVariable1 = bayesParams.changeInVolume / bayesParams.changeInPressure;
			temporaryVariable1 = temporaryVariable1 * temporaryVariable1;
			temporaryVariable1 = temporaryVariable1 * bayesParams.sensorUncertainity;
			temporaryVariable2 = bayesParams.changeInPressure * bayesParams.changeInPressure;
			temporaryVariable2 = bayesParams.measuredPressure * bayesParams.changeInVolume / temporaryVariable2;
			temporaryVariable2 = temporaryVariable2 * temporaryVariable2;
			temporaryVariable2 = bayesParams.uncertaintyPressureDiff * temporaryVariable2;
			temporaryVariable3 = bayesParams.measuredPressure / bayesParams.changeInPressure;
			temporaryVariable3 = temporaryVariable3 * temporaryVariable3;
			temporaryVariable3 = bayesParams.uncertaintyVolumeChange * temporaryVariable3;

			bayesParams.uncertaintyMeasuredVolume = temporaryVariable1 + temporaryVariable2 + temporaryVariable3;
        }
        else if (1u == controllerStatus.bit.fineControl)
        {
            //# use Prediction Error method to slowly adjust estimates of volume and leak rate
            //# during fine control loop when close to setpoint
            bayesParams.algorithmType = ePredictionErrorMethod; //bayes['algoV'] = 3

            //# measured residual leak rate from steady - state pressure error(mbar / iteration)
            float measL = 0.0f;
            residualL = ((-1.0f) * bayesParams.smoothedPressureErrForPECorrection) / bayesParams.estimatedKp;
			bayesParams.residualLeakRate = residualL;

            //# estimate of true leak rate magnitude(mbar / iteration)
            measL = fabs(residualL + bayesParams.estimatedLeakRate);
			bayesParams.measuredLeakRate1 = measL;
            if ((int32_t)1 == bayesParams.predictionErrType)
            {
                /*
                # Excessive gain, decrease Vand increase leak estimates to keep same net leak rate control effect.
                # Decrease volume estimate only when steady state pressure errorand leak have the same sign
                #and the measured error in leak rate estimate is less than + / -50 %
                # Otherwise keep volume estimate as is until leak estimate is closer to steady state value
                # to prevent underestimating volume when leak is inaccurate(can cause sluggish control).
                # The effect of a leak on PE will grow if a small dV correction has been skipped, e.g.in
                # low - power mode, so do not make volume estimate corrections unless the
                # the control has applied a correction at least twice consecutively(dV > 0 and dV_ > 0)
                */
                if ((residualL * bayesParams.estimatedLeakRate >= 0.0f) &&
                    (measL < 2 * fabs(bayesParams.estimatedLeakRate)) &&
                    (bayesParams.changeInVolume != 0.0f) &&
                    (bayesParams.prevChangeInVolume != 0.0f))
                {
                    bayesParams.measuredVolume = bayesParams.estimatedVolume * bayesParams.gamma;//bayes['measV'] = bayes['V'] * bayes['gamma']
                    bayesParams.uncertaintyMeasuredVolume = bayesParams.uncertaintyVolumeEstimate;//bayes['varMeasV'] = bayes['varV']
                    bayesParams.estimatedLeakRate = bayesParams.estimatedLeakRate / bayesParams.gamma; //bayes['leak'] = bayes['leak'] / bayes['gamma']
                }
                //# print('***-')
            }
            if ((int32_t)-1 == bayesParams.predictionErrType)
            {
                /*
                # Insufficient gain, increase Vand decrease leak estimates to keep same net leak rate control effect.
                    # Increase volume estimate only when steady state pressure errorand leak have the same sign
                    #and the measured error in leak rate estimate is less than + / -50 %
                    # Otherwise keep volume estimate as is until L estimate is closer to steady state value
                    # to prevent overestimating volume when leak is inaccurate(can cause control oscillation).
                    # The effect of a leak on PE will be amplified if no correction has been made(leak is not being fed,
                        # low - power mode), so do not make volume estimate corrections unless the
                    # the control has applied a correction at least twice in a row(dV > 0 and dV_ > 0)
                 */
                if ((residualL * bayesParams.estimatedLeakRate >= 0.0f) &&
                    (measL < 2.0f * fabs(bayesParams.estimatedLeakRate)) &&
                    (bayesParams.changeInVolume != 0.0f) &&
                    (bayesParams.prevChangeInVolume != 0.0f)
                    )
                {
                    bayesParams.measuredVolume = bayesParams.estimatedVolume / bayesParams.gamma; //bayes['measV'] = bayes['V'] / bayes['gamma']
                    bayesParams.uncertaintyMeasuredVolume = bayesParams.uncertaintyVolumeEstimate; //bayes['varMeasV'] = bayes['varV']
                    bayesParams.estimatedLeakRate = bayesParams.estimatedLeakRate * bayesParams.gamma; //bayes['leak'] = bayes['leak'] * bayes['gamma']
                }
                //# print('***+')
            }
        }
        //# combine prior volume estimate with latest measured volume, if available
        if (bayesParams.measuredVolume != 0.0f)
        {
            //# probability weighted average of prior and measured values(mL)
            //bayes['V'] = (bayes['measV'] * bayes['varV'] + bayes['V'] * bayes['varMeasV']) / (bayes['varMeasV'] + bayes['varV'])
            bayesParams.estimatedVolume = ((bayesParams.measuredVolume * bayesParams.uncertaintyVolumeEstimate) + (bayesParams.estimatedVolume * bayesParams.uncertaintyMeasuredVolume)) / (bayesParams.uncertaintyMeasuredVolume + bayesParams.uncertaintyVolumeEstimate);

            //# new uncertainty in estimated volume(mL)
            //bayes['varV'] = bayes['varMeasV'] * bayes['varV'] / (bayes['varMeasV'] + bayes['varV'])
            bayesParams.uncertaintyVolumeEstimate = (bayesParams.uncertaintyMeasuredVolume * bayesParams.uncertaintyVolumeEstimate) / (bayesParams.uncertaintyMeasuredVolume + bayesParams.uncertaintyVolumeEstimate);

            //# increase varV if measV is not within maxZScore standard deviations of V
            //# increases convergence rate to true value in the event of a step change in system volume
            float du = 0.0f;
            //du = abs(bayes['V'] - bayes['measV'])
            du = fabs(bayesParams.estimatedVolume - bayesParams.measuredVolume);

            //bayes['varV'] = max(bayes['varV'], du / bayes['maxZScore'] - bayes['varMeasV'])
            bayesParams.uncertaintyVolumeEstimate = fmax(bayesParams.uncertaintyVolumeEstimate, du / (bayesParams.maxZScore - bayesParams.uncertaintyMeasuredVolume));
            //# bound updated volume estimate
            //bayes['V'] = max(min(bayes['V'], bayes['maxV']), bayes['minV'])
            bayesParams.estimatedVolume = fmax(fmin(bayesParams.estimatedVolume, bayesParams.maxSysVolumeEstimateValue), bayesParams.minSysVolumeEstimateValue);
        }
        //# Step 2
        //# Adjust leak rate estimate

        //if bayes['algoV'] == 3:
        if (ePredictionErrorMethod == bayesParams.algorithmType)
        {
            /*
                # Use dynamically averaged pressure error when near setpoint to adjust leak rate.
                # Leak rate averaging window length is inversely proportional to the
                # larger of the estimated leak rate or pressure error.
                # This makes the filter react faster when either the pressure error or
                # leak rate estimate is "large" to improve convergence rate to true values,
                # while providing accurate leak rate correction for small leaks.
            */
            //# limit filter length to maxN control iterations
            float minError = 0.0f;
            float maxError = 0.0f;
            uint32_t numOfIterations = 0u;
            minError = sqrt(bayesParams.uncertaintyPressureDiff) / ((float)bayesParams.maxIterationsForIIRfilter);//minError = bayes['vardP'] * *0.5 / bayes['maxN']

			//# use largest of pressure error or leak rate estimate to define IIR filter length
			//# larger values means shorter observation time / faster response
            maxError = fmax(fmax(fabs(bayesParams.smoothedPressureErrForPECorrection), fabs(bayesParams.estimatedLeakRate)), minError); //maxError = max(abs(bayes['smoothE_PE']), abs(bayes['leak']), minError)

			//# number of control iterations to average over(minN, maxN)
            numOfIterations = max(int(sqrt(bayesParams.uncertaintyPressureDiff) / maxError), bayesParams.minIterationsForIIRfilter); //n = max(int(bayes['vardP'] * *0.5 / maxError), bayes['minN'])
			bayesParams.numberOfControlIterations = numOfIterations;
			/*
			# Average E over more iterations when leak rate estimate is small and averaged pressure error is small
			# so that the expected value of the pressure error from a leak
			# is at least as big as pressure measurement noise.
			*/

	        //# IIR filter memory factor to achieve epsilon * initial condition residual after n iterations
            bayesParams.alpha = pow(10, (bayesParams.log10epsilon / numOfIterations));//bayes['alpha'] = 10 * *(bayes['log10epsilon'] / n)

		    //# smoothed pressure error with dynamic IIR filter
			//bayes['smoothE_PE'] = (1 - bayes['alpha']) * PID['E'] + bayes['alpha'] * bayes['smoothE_PE']
            bayesParams.smoothedPressureErrForPECorrection = (1 - bayesParams.alpha) * pidParams.pressureError + (bayesParams.alpha * bayesParams.smoothedPressureErrForPECorrection);

            //# measured residual leak rate from steady - state pressure error(mbar / iteration)
            residualL = (-1.0f) * bayesParams.smoothedPressureErrForPECorrection / bayesParams.estimatedKp;//residualL = -bayes['smoothE_PE'] / bayes['kP']
			bayesParams.residualLeakRate = residualL;
            /*
            # Apply 1 / n of total correction to reduce remaining leak error gradually(mbar / iteration)
            # Correction to leak estimate must be gradual because leak effect takes time to accurately measure.
            # Applying entire dL correction to leak rate after
            # each control iteration could cause sustained oscillation in pressure error,
            # similar to "integrator windup" in a PI controller when the "kI" term is too large.
            # This controller's correction for leak rate is a feed-forward compensation with gain kP and
            # leak rate estimation error inferred from smoothed pressure error.
            # At steady - state, feed - forward control allows the controller to anticipate the effect
            # of the leak before it accumulates as a significant pressure error, using historical leak disturbance
            # to compensate for future effects.This is particularly helpful for minimizing
            # steady - state pressure error for leak rates greater than the measurement
            # noise when the iteration rate of the control loop is slow.
            */
            bayesParams.changeToEstimatedLeakRate = (residualL / (float)numOfIterations);//bayes['dL'] = residualL / n

            //# Special cases :

            if (residualL * bayesParams.estimatedLeakRate < 0.0f)
            {
                /*
                 # Residual pressure error is opposite sign to estimated leak rate
                 # It could take a long time to correct this estimation error while the
                 # pressure error continues to accumulate.
                 # Speed - up convergence to the correct leak rate value by removing all dL in one iteration
                 # This may temporarily cause the pressure error to
                 # oscillate, but reduces the maximum error
                 # that would otherwise accumulate.
                 # This situation could happen if the true leak rate is suddenly changed
                 # while hovering around setpoint, e.g.by user stopping the leak.
                 */
                bayesParams.estimatedLeakRate = bayesParams.estimatedLeakRate + (bayesParams.changeToEstimatedLeakRate * numOfIterations);
                //# print('***L++')
            }
            else
            {
                //# Residual pressure error is the same sign as estimated leak.
                //# Adjust leak rate estimate a small amount to minimize its effect
                bayesParams.estimatedLeakRate = bayesParams.estimatedLeakRate + bayesParams.changeToEstimatedLeakRate;//bayes['leak'] = bayes['leak'] + bayes['dL']
                /*
                    # Note: If dV == 0 because of low - power mode control operation then the leak rate correction will grow
                # larger than the "correct" value because the leak effect is not being offset with feed - forward control.
                # Prolonged operation with non - zero leak rate and dV == 0 may cause leak estimate to increase
                # above true value.This is ok because effectively the control loop iteration time is increased as well,
                #and so the leak rate per control iteration is truly increasing.However, if there is a disturbance
                # the controller will revert back to a faster iteration rate, which may cause temporary oscillation while
                # the leak rate estimate is nudged back down.

                # varLeak the variance of low - pass filtered pressure sensor noise,
                # which decreases as 1 / n for large n(Poisson statistics)
                # n is the low - pass filter observation time(iterations)
                */
                bayesParams.uncertaintyEstimatedLeakRate = bayesParams.uncertaintyPressureDiff / numOfIterations; //bayes['varLeak'] = (bayes['vardP'] / n)

                //# increase leak uncertainty if measLeak is not within maxZScore standard deviations of leak
                //# increases convergence to true value in the event of a step change in leak rate
                float du = fabs(bayesParams.estimatedLeakRate - bayesParams.measuredLeakRate);//du = fabs(bayes['leak'] - bayes['measLeak'])
                bayesParams.uncertaintyEstimatedLeakRate = max(bayesParams.uncertaintyEstimatedLeakRate, du / (bayesParams.maxZScore - bayesParams.uncertaintyMeasuredLeakRate));//bayes['varLeak'] = max(bayes['varLeak'], du / bayes['maxZScore'] - bayes['varMeasLeak'])
            }
        }
        else
        {
            
        }
		//# volume estimated with gas law
		//# keep previous leak estimate
		//estimatedLeakRate = //bayes['leak'] = bayes['leak']
		//bayes['varLeak'] = bayes['varLeak']

		//# bound leak correction to reasonable values, scaled by volume estimate and pressure
		//# higher pressure and lower volume can have larger leak rate estimate
		//bayes['maxLeak'] = screw['maxLeak'] / bayes['V'] * bayes['P'] / screw['maxP'] * screw['nominalV']
		//(screw['maxLeak'] / bayes['V'])* (bayes['P'] / screw['maxP'])* screw['nominalV']
		bayesParams.maxEstimatedLeakRate = (screwParams.maxLeakRate / bayesParams.estimatedVolume) * (bayesParams.measuredPressure / screwParams.maxAllowedPressure) * screwParams.nominalTotalVolume;
		//bayes['leak'] = min(max(bayes['leak'], -bayes['maxLeak']), bayes['maxLeak'])
		bayesParams.estimatedLeakRate = min(max(bayesParams.estimatedLeakRate, (-1.0f * bayesParams.maxEstimatedLeakRate)), bayesParams.maxEstimatedLeakRate);

		//# Calculate optimal controller gain for next control iteration
		//# optimal kP to achieve correction of pressure error in one step(steps / mbar)
		bayesParams.estimatedKp = bayesParams.estimatedVolume / (bayesParams.measuredPressure * screwParams.changeInVolumePerPulse); //bayes['kP'] = bayes['V'] / (bayes['P'] * screw['dV'])

        if (1u == controllerStatus.bit.fineControl)
        {
            /*
            # in fine control loop
            # calculate pressure range estimate using optical sensor
            #and estimated volume to judge remaining travel range in pressure units
            # Not currently used to make control decisions but could be useful for future work
            # on rampsand to detect imminent excessiveRange fails before they occur.
            */
            //# maximum volume achievable(mL)
            float maxV = 0.0f;
            float minV = 0.0f;
            float minV2 = 0.0f;
            float maxV2 = 0.0f;
            float maxV2steps = 0.0f;
            float minV2steps = 0.0f;

            //maxV = max(min(bayes['V'] + (PID['position'] - screw['minPosition']) * screw['dV'], bayes['maxV']), bayes['V'])
            maxV = max(min(bayesParams.estimatedVolume + (pidParams.pistonPosition - screwParams.minPosition) * screwParams.changeInVolumePerPulse, bayesParams.maxSysVolumeEstimateValue), bayesParams.estimatedVolume);
            //# minimum volume achievable(mL)
            //minV = min(max(bayes['V'] - (screw['maxPosition'] - PID['position']) * screw['dV'], bayes['minV']), bayes['V'])
            minV = min(max(bayesParams.estimatedVolume - (screwParams.maxPosition - pidParams.pistonPosition) * screwParams.changeInVolumePerPulse, bayesParams.minSysVolumeEstimateValue), bayesParams.estimatedVolume);

            //# maximum achievable pressure(mbar)
            //bayes['maxP'] = bayes['P'] * bayes['V'] / minV
            bayesParams.maxAchievablePressure = bayesParams.measuredPressure * bayesParams.estimatedVolume / minV;
            //# minimum achievable pressure(mbar)
            //bayes['minP'] = bayes['P'] * bayes['V'] / maxV
            bayesParams.minAchievablePressure = bayesParams.measuredPressure * bayesParams.estimatedVolume / maxV;

            //# largest positive pressure change(mbar)
            bayesParams.maxPositivePressureChangeAchievable = bayesParams.maxAchievablePressure - bayesParams.measuredPressure;//bayes['maxdP'] = bayes['maxP'] - bayes['P']

            //# largest negative pressure change(mbar)
            bayesParams.maxNegativePressureChangeAchievable = bayesParams.minAchievablePressure - bayesParams.measuredPressure;

            //# volume required to achieve nominalRange % increase in pressure from current pressure
            minV2 = bayesParams.estimatedVolume / (1 + bayesParams.minPressureAdjustmentRangeFactor);//minV2 = bayes['V'] / (1 + bayes['nominalRange'])

            //# volume required to achieve nominalRange % decrease in pressure from current pressure
            maxV2 = bayesParams.estimatedVolume / (1 - bayesParams.minPressureAdjustmentRangeFactor);//maxV2 = bayes['V'] / (1 - bayes['nominalRange'])

            //# number of steps required to achieve maxV2 target(negative definite)
            maxV2steps = round(max(0.0f, ((maxV2 - maxV) / screwParams.changeInVolumePerPulse)));

            //# number of steps required to achieve minV2 target(positive definite)
            minV2steps = round(min(0, (minV2 - minV) / screwParams.changeInVolumePerPulse));

            if (false == (floatEqual(minV2steps * maxV2steps, 0.0f)))
            {
                //printf(error: nominalTarget position is not achievable', minV2steps, maxV2steps)

                //# nominal home position to achieve nominalRange target(steps)
                
            }
			//bayes['nominalHome'] = min(max(PID['position'] + maxV2steps + minV2steps,screw['minPosition']), screw['maxPosition'])
			bayesParams.nominalHomePosition = min(max(pidParams.pistonPosition + maxV2steps + minV2steps, screwParams.minPosition), screwParams.maxPosition);
			//# limit to allowed range of position
			//# expected pressure at center piston position
		   // bayes['centerP'] = bayes['P'] * bayes['V'] / (bayes['V'] + (PID['position'] - screw['centerPosition']) * screw['dV']) //#expected pressure at center piston position
			bayesParams.expectedPressureAtCenterPosition = bayesParams.measuredPressure * bayesParams.estimatedVolume / (bayesParams.estimatedVolume + (pidParams.pistonPosition - screwParams.centerPositionCount) * screwParams.changeInVolumePerPulse);
        }
    }
}

int32_t readingToPosition(uint32_t opticalSensorReading)
{
    return (int32_t)100;
}

void controller::pressureControlLoop(pressureInfo_t* ptrPressureInfo)
{
    if (NULL != ptrPressureInfo)
    {
        absolutePressure = ptrPressureInfo->absolutePressure;
        gaugePressure = ptrPressureInfo->gaugePressure;
        atmosphericPressure = ptrPressureInfo->atmosphericPressure;;
        pressureSetPoint = ptrPressureInfo->pressureSetPoint;
        myMode = ptrPressureInfo->mode;
        setPointType = ptrPressureInfo->setPointType;
		pidParams.controlledPressure = ptrPressureInfo->pressure;
		pidParams.pressureAbs = absolutePressure;
		pidParams.pressureGauge = gaugePressure;
		pidParams.pressureBaro = atmosphericPressure;
		pidParams.pressureOld = ptrPressureInfo->oldPressure;

        pidParams.elapsedTime = ptrPressureInfo->elapsedTime;
        switch (controllerState)
        {
            case eCoarseControlLoopEntry:
                coarseControlSmEntry();
                resetBayesParameters();
                break;

            case eCoarseControlLoop:
                //# run coarse control loop until close to setpoint
                coarseControlLoop();
                resetBayesParameters();
                break;

            case eCoarseControlExit:
                coarseControlSmExit();
                resetBayesParameters();
                fineControlSmEntry();
                break;

            case eFineControlLoopEntry:
                fineControlSmEntry();
                break;

            case eFineControlLoop:
                fineControlLoop(); //# run fine control loop until Genii mode changes from control, or control error occurs
                break;

            default:
                break;
        }

    }
}
void controller::resetBayesParameters(void)
{
    bayesParams.changeInPressure = 0.0f;
    bayesParams.prevChangeInPressure = 0.0f;
    bayesParams.changeInVolume = 0.0f;
    bayesParams.prevChangeInVolume = 0.0f;
    bayesParams.targetdP = 0.0f;
    bayesParams.estimatedLeakRate = 0.0f;
    bayesParams.uncertaintyEstimatedLeakRate = bayesParams.uncertaintyPressureDiff;
    testParams.fakeLeak = 0.0f;
    bayesParams.measuredPressure = absolutePressure;
    bayesParams.smoothedPresure = absolutePressure;
    bayesParams.smoothedPressureErr = 0.0f;
    bayesParams.smoothedSqaredPressureErr = 0.0f;
    bayesParams.uncerInSmoothedMeasPresErr = 0.0f;
    bayesParams.smoothedPressureErrForPECorrection = 0.0f;
}
void controller::fineControlLoop()
{
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    int32_t completedCnt = (int32_t)0;
    if (1u == controllerStatus.bit.fineControl)
    {

        //# read pressure with highest precision
        //pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllSlow()
        //    PID['mode'] = mode
        
        //if PID['mode'] != 1 or PID['excessLeak'] == 1 or PID['excessVolume'] == 1 or \
        //    PID['overPressure'] == 1 or PID['rangeExceeded'] == 1:
        //if ((controllerStatus.bytes && 0X00FF) == 0u)
        if((myMode == eModeControl) &&
                (controllerStatus.bit.excessLeak != (uint32_t)1) &&
                (controllerStatus.bit.excessVolume != (uint32_t)1)&&
                (controllerStatus.bit.overPressure != (uint32_t)1)&&
                (controllerStatus.bit.rangeExceeded != (uint32_t)1))
                
        {
            //# adjust measured pressure by simulated leak rate effect
            //# control to pressure in the same units as setpoint
            pidParams.setPointType = setPointType;
            pidParams.controlledPressure = pressureAsPerSetPointType(); //[pressureG, pressure, atmPressure][spType] + testParams.fakeLeak;
            pidParams.pressureSetPoint = pressureSetPoint;

            //# store previous dP and dV values
            bayesParams.prevChangeInPressure = bayesParams.changeInPressure; //bayes['dP_'] = bayes['dP']  //# previous measured dP(mbar)
            bayesParams.prevChangeInVolume = bayesParams.changeInVolume; //bayes['dV_'] = bayes['dV']  //# previous applied dV(mL)

            //# change in pressure from last measurement(mbar), +ve == increasing pressure
            bayesParams.changeInPressure = absolutePressure + testParams.fakeLeak - bayesParams.measuredPressure;//bayes['dP'] = pressure + testing['fakeLeak'] - bayes['P']

            //# use absolute pressure for volume estimates using gas law
            bayesParams.measuredPressure = absolutePressure + testParams.fakeLeak;// bayes['P'] = pressure + testing['fakeLeak']

            //# store previous pressure error for prediction error calculation
            bayesParams.targetdP = pidParams.pressureError;    //bayes['targetdP'] = PID['E']
            /*
            # Note: Unlike PID['targetdP'], bayes['targetdP']
            # is not adjusted for leak estimate.
            # The leak estimate may be incorrectand the bayes prediction error
            # should therefore be non - zero even if the PID['targetdP'] was achieved.
            */

            //# pressure error error(mbar), -ve if pressure above setpoint
            pidParams.pressureError = pidParams.pressureSetPoint - pidParams.controlledPressure;//PID['E'] = PID['setpoint'] - PID['pressure']

            //# target pressure correction after anticipating leak effect(mbar)
            pidParams.pressureCorrectionTarget = pidParams.pressureError - bayesParams.estimatedLeakRate;//PID['targetdP'] = PID['E'] - bayes['leak']

            //# steps to take to achieve zero pressure error in one control iteration
            pidParams.stepSize = int(round(bayesParams.estimatedKp * pidParams.pressureCorrectionTarget)); //PID['stepSize'] = int(round(bayes['kP'] * PID['targetdP']))

            //# setpoint achievable status(True / False)
            //# TBD use this to abort setpoint if not achievable before reaching max / min piston positions
            //PID['inRange'] = (PID['E'] > bayes['mindP']) and (PID['E'] < bayes['maxdP'])
            pidParams.isSetpointInControllerRange = (pidParams.pressureError > bayesParams.maxNegativePressureChangeAchievable) && (pidParams.pressureError < bayesParams.maxPositivePressureChangeAchievable);

            //# abort correction if pressure error is within the measurement noise floor to save power,
            //# also reduces control noise when at setpoint without a leak
            if ((fabs(pidParams.pressureCorrectionTarget) < sqrt(bayesParams.uncertaintyPressureDiff)) &&
                (fabs(pidParams.pressureError) < sqrt(bayesParams.uncertaintyPressureDiff))
                )
            {
                pidParams.pressureCorrectionTarget = 0.0f;// PID['targetdP'] = 0.0f;
                pidParams.stepSize = 0.0f; //PID['stepSize'] = 0.0f;
                //print('*LP')
            }
            //# increase motor current with gauge pressure
            //# saves power for lower pressure setPoints
            //PID['current'] = screw['minCurrent'] + (screw['maxCurrent'] - screw['minCurrent']) * \
            //abs(pressureG) / screw['maxPressure']
            pidParams.requestedMeasuredMotorCurrent = motorParams.minMotorCurrrent + (motorParams.maxMotorCurrent - motorParams.minMotorCurrrent) * \
                fabs(gaugePressure) / screwParams.maxPressure;
            motor->writeAcclCurrent(pidParams.requestedMeasuredMotorCurrent);// SetAcclCurrent(PID['current'])
            motor->writeDecelCurrent(pidParams.requestedMeasuredMotorCurrent);//(PID['current'])
            /*
            # request new stepSize and read back previous stepCount
            # motor acceleration limits number of steps that can be taken per iteration
            # so count != stepSize except when controlling around setpoint with small stepSize
            */
            //pidParams.stepCount = motor->writeMoveContinuous(pidParams.stepSize); //.MOTOR_MoveContinuous(pidParams.stepSize);
            errorStatus = motor->writeMoveContinuous(pidParams.stepSize, &completedCnt);      //# stop the motor
            if ((eIbcError_t)E_IBC_ERROR_NONE == errorStatus)
            {
                pidParams.stepCount = completedCnt;
            }
            else
            {
                pidParams.stepCount = 0;
            }
            //# total number of steps taken
            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;

            //# change in volume(mL)
            bayesParams.changeInVolume = -screwParams.changeInVolumePerPulse * pidParams.stepCount;

            //# read the optical sensor piston position
            pidParams.opticalSensorAdcReading = readOpticalSensorCounts();//pv624.readOpticalSensor();
            pidParams.pistonPosition = int(getPistonPosition(pidParams.opticalSensorAdcReading));

            //# decide if in allowed range of piston position
            if (screwParams.minPosition < pidParams.pistonPosition < screwParams.maxPosition)
            {
                controllerStatus.bit.rangeExceeded = 0.0f; // # in allowed range, continue
            }
            else
            {
                controllerStatus.bit.rangeExceeded = 1.0f;  //# outside of allowed range, abort setpoint
            }
            //print('Control Error: rangeExceeded')
            float currentADC = 0.0f;
            //need to uncomment PID['speed'], currentADC = motor.GetSpeedAndCurrent();
            pidParams.measuredMotorCurrent = int(currentADC * screwParams.readingToCurrent);

            //# update system parameter estimates from latest measurement data
            estimate();

            //# report back status to GENII
            //# calcStatus() imported from dataStructures4.py
            //calcStatus(PID = PID)
            //pv624.setControllerStatus(PID['status'])
            //# and abort on error or mode change
            /*
            # Code below used for testing controller performance, not for product use :
            #------------------------------------------------------------------------
            # scale maxFakeLeakRate by measured gage pressureand test volume
            # Don't use volume estimate to scale leak rate
            # so that errors in the estimate do not affect applied leak rate
            #and the true leak rate can be back - calculated from pressure and nominal test volume.
            */
            testParams.fakeLeakRate = -fabs(testParams.maxFakeLeakRate) * \
                gaugePressure / screwParams.maxAllowedPressure * screwParams.nominalTotalVolume / testParams.volumeForLeakRateAdjustment;

            //# new cumulative pressure error from the simulated leak(mbar)
            //testing['fakeLeak'] = testing['fakeLeak'] + testing['fakeLeakRate']
            testParams.fakeLeak = testParams.fakeLeak + testParams.fakeLeakRate;
            //# Code below to be replaced with logging of data to PV624 non - volatile memory
            //# for diagnostic purposes.# -------------------------------------------------
            /*
            if logging['logData']:
            //# get timestamp
            PID['elapsedTime'] = round((datetime.now() - logging['startTime']).total_seconds(), 3)
            //# append dynamic data to dataFile
            csvFile = csv.writer(f, delimiter = ',')
            csvFile.writerow(list(PID.values()) + list(bayes.values()) + list(testing.values()))

            # write a small subset of global data structure content to screen
            printValues = [round(PID['pressure'], 2),
            round(PID['E'], 2),
            round(bayes['V'], 1),
            round(bayes['leak'], 4),
            round(testing['fakeLeakRate'], 4),
            round(PID['position']),
            round(PID['measCurrent'], 2)
            ]

            formatString = len(printValues) * '{: >8} '
            //print(formatString.format(*printValues))
                */
#ifdef CPP_ON_PC
            logPidBayesAndTestParamDataValues(eFineControl);
            printf_s("\n\r%f\t%f\t%f\t%f\t%d\t%f", 
                        pidParams.controlledPressure,
                        pidParams.pressureError,
                        bayesParams.estimatedVolume,
                        bayesParams.estimatedLeakRate,
                        testParams.fakeLeakRate,
                        pidParams.pistonPosition,
                        pidParams.measuredMotorCurrent);
#endif
        }
        else
        {
            //# Genii has changed operating mode to vent or measure
            //# or a control error has occurred
            //# abort fine control and return to coarse control loop
            controllerStatus.bit.fineControl = 0u;
        }
    }
}
float controller:: pressureAsPerSetPointType(void)
{
    float pressureValue = 0.0f;
    if ((setPointType_t)eGauge == setPointType)
    {
        pressureValue = gaugePressure;
    }
    else if ((setPointType_t)eAbsolute == setPointType)
    {
        pressureValue = absolutePressure;
    }
    else
    {
        pressureValue = atmosphericPressure;
    }
    return pressureValue;
}

uint32_t controller::readOpticalSensorCounts(void)
{
    return (uint32_t)controllerData[eOpticalADC];
    //return ::opticalSensorAdcCounts;
}

void controller::logPidBayesAndTestParamDataValues(eControllerType_t controlType)
{
#ifdef CPP_ON_PC	
    if ((eControllerType_t)eCoarseControl == controlType)
    {
        resultFile.open(coarseControlLogParams.fileName, std::ofstream::app);
    }
    else if ((eControllerType_t)eFineControl == controlType)
    {
        resultFile.open(fineControlLogParams.fileName, std::ofstream::app);
    }
    else
    {
        resultFile.open(resultFilePath, std::ofstream::app);
    }
	if (resultFile.is_open())
	{
		/* PID */
		resultFile << std::endl << pidParams.elapsedTime << std::setprecision(10) <<  ",";
		resultFile << pidParams.pressureSetPoint << std::setprecision(10) << ",";
        resultFile << pidParams.setPointType << std::setprecision(10) << ",";
        resultFile << pidParams.stepCount << std::setprecision(10) << ",";
		resultFile << pidParams.pressureError << std::setprecision(10) << ",";
		resultFile << pidParams.totalStepCount << std::setprecision(10) << ",";
		resultFile << pidParams.controlledPressure << std::setprecision(10) << ",";
		resultFile << pidParams.pressureAbs << std::setprecision(10) << ",";
		resultFile << pidParams.pressureGauge << std::setprecision(10) << ",";
		resultFile << pidParams.pressureBaro << std::setprecision(10) << ",";
		resultFile << pidParams.pressureOld << std::setprecision(10) << ",";
		resultFile << pidParams.stepSize << std::setprecision(10) << ",";
		resultFile << pidParams.pressureCorrectionTarget << std::setprecision(10) << ",";
		resultFile << pidParams.requestedMeasuredMotorCurrent << std::setprecision(10) << ",";
		resultFile << pidParams.measuredMotorCurrent << std::setprecision(10) << ",";
		resultFile << pidParams.opticalSensorAdcReading << std::setprecision(10) << ",";
		resultFile << pidParams.pistonPosition << std::setprecision(10) << ",";
		resultFile << pidParams.motorSpeed << std::setprecision(10) << ",";
		resultFile << pidParams.isSetpointInControllerRange << std::setprecision(10) << ",";
		resultFile << pidParams.pumpTolerance << std::setprecision(10) << ",";

		/* Controller Status */
		resultFile << myMode << ",";
		resultFile << controllerStatus.bytes << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.pumpUp << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.pumpDown << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.control << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.venting << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.stable << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.vented << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.excessLeak << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.excessVolume << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.overPressure << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.excessOffset << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.measure << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.fineControl << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.pistonCentered << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.centering << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.controlledVent << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.centeringVent << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.rangeExceeded << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.coarseControlError << std::setprecision(10) <<  ",";
		resultFile << controllerStatus.bit.ventDir << std::setprecision(10) <<  ",";

		/* Bayes */
		resultFile << bayesParams.minSysVolumeEstimateValue << std::setprecision(10) <<  ",";
		resultFile << bayesParams.maxSysVolumeEstimateValue << std::setprecision(10) <<  ",";
		resultFile << bayesParams.minEstimatedLeakRate << std::setprecision(10) <<  ",";
		resultFile << bayesParams.maxEstimatedLeakRate << std::setprecision(10) <<  ",";
		resultFile << bayesParams.measuredPressure << std::setprecision(10) <<  ",";
		resultFile << bayesParams.smoothedPresure << std::setprecision(10) <<  ",";
		resultFile << bayesParams.changeInPressure << std::setprecision(10) <<  ",";
		resultFile << bayesParams.prevChangeInPressure << std::setprecision(10) <<  ",";
		resultFile << bayesParams.dP2 << std::setprecision(10) << ",";
		resultFile << bayesParams.estimatedVolume << std::setprecision(10) <<  ",";
		resultFile << bayesParams.algorithmType << std::setprecision(10) <<  ",";
		resultFile << bayesParams.changeInVolume << std::setprecision(10) <<  ",";
		resultFile << bayesParams.prevChangeInVolume << std::setprecision(10) <<  ",";
		resultFile << bayesParams.dV2 << std::setprecision(10) << ",";
		resultFile << bayesParams.measuredVolume << std::setprecision(10) <<  ",";
		resultFile << bayesParams.estimatedLeakRate << std::setprecision(10) <<  ",";
		resultFile << bayesParams.measuredLeakRate << std::setprecision(10) <<  ",";
		resultFile << bayesParams.estimatedKp << std::setprecision(10) <<  ",";
		//resultFile << bayesParams.measuredKp << std::setprecision(10) <<  ",";
		resultFile << bayesParams.sensorUncertainity << std::setprecision(10) <<  ",";
		resultFile << bayesParams.uncertaintyPressureDiff << std::setprecision(10) <<  ",";
		resultFile << bayesParams.uncertaintyVolumeEstimate << std::setprecision(10) <<  ",";
		resultFile << bayesParams.uncertaintyMeasuredVolume << std::setprecision(10) <<  ",";
		resultFile << bayesParams.uncertaintyVolumeChange << std::setprecision(10) <<  ",";
		resultFile << bayesParams.uncertaintyEstimatedLeakRate << std::setprecision(10) <<  ",";
		resultFile << bayesParams.uncertaintyMeasuredLeakRate << std::setprecision(10) <<  ",";
		resultFile << bayesParams.maxZScore << std::setprecision(10) <<  ",";
		resultFile << bayesParams.lambda << std::setprecision(10) <<  ",";
		resultFile << bayesParams.uncerInSmoothedMeasPresErr << std::setprecision(10) <<  ",";
		resultFile << bayesParams.targetdP << std::setprecision(10) <<  ",";
		resultFile << bayesParams.smoothedPressureErr << std::setprecision(10) <<  ",";
		resultFile << bayesParams.smoothedSqaredPressureErr << std::setprecision(10) <<  ",";
		resultFile << bayesParams.gamma << std::setprecision(10) <<  ",";
		resultFile << bayesParams.predictionError << std::setprecision(10) <<  ",";
		resultFile << bayesParams.predictionErrType << std::setprecision(10) <<  ",";
		resultFile << bayesParams.maxAchievablePressure << std::setprecision(10) <<  ",";
		resultFile << bayesParams.minAchievablePressure << std::setprecision(10) <<  ",";
		resultFile << bayesParams.maxPositivePressureChangeAchievable << std::setprecision(10) <<  ",";
		resultFile << bayesParams.maxNegativePressureChangeAchievable << std::setprecision(10) <<  ",";
		resultFile << bayesParams.minPressureAdjustmentRangeFactor << std::setprecision(10) <<  ",";
		resultFile << bayesParams.nominalHomePosition << std::setprecision(10) <<  ",";
		resultFile << bayesParams.expectedPressureAtCenterPosition << std::setprecision(10) <<  ",";
		resultFile << bayesParams.maxIterationsForIIRfilter << std::setprecision(10) <<  ",";
		resultFile << bayesParams.minIterationsForIIRfilter << std::setprecision(10) <<  ",";
		resultFile << bayesParams.changeToEstimatedLeakRate << std::setprecision(10) <<  ",";
		resultFile << bayesParams.alpha << std::setprecision(10) <<  ",";
		resultFile << bayesParams.smoothedPressureErrForPECorrection << std::setprecision(10) <<  ",";
		resultFile << bayesParams.log10epsilon << std::setprecision(10) <<  ",";
		resultFile << bayesParams.residualLeakRate << std::setprecision(10) <<  ",";
		resultFile << bayesParams.measuredLeakRate1 << std::setprecision(10) <<  ",";
		resultFile << bayesParams.numberOfControlIterations << std::setprecision(10) <<  ",";

		/* Test Params */
		resultFile << testParams.maxFakeLeakRate << std::setprecision(10) <<  ",";
		resultFile << testParams.fakeLeakRate << std::setprecision(10) <<  ",";
		resultFile << testParams.isOpticalSensorCalibrationRequired << std::setprecision(10) <<  ",";
		resultFile << testParams.fakeLeak << std::setprecision(10) <<  ",";
		resultFile << testParams.volumeForLeakRateAdjustment ;
		resultFile.close();
	}
#endif
#ifdef CPP_FILE
    FILE* fptr = NULL;
    if ((eControllerType_t)eCoarseControl == controlType)
    {
         fopen_s (&fptr, coarseControlLogParams.fileName, "a");
    }
    else if ((eControllerType_t)eFineControl == controlType)
    {
         fopen_s (&fptr, fineControlLogParams.fileName, "a");
    }
    else
    {
        fptr = NULL;
    }
    float fValue = pidParams.pressureSetPoint;
    if (fptr != NULL)
    {
        printf_s("\n\r");
        printf_s("%d,%d,", pidParams.setPointType, pidParams.stepCount);
        printf_s("%f,%d", fValue, pidParams.elapsedTime);
        printf_s("%f,%d,", pidParams.pressureSetPoint , pidParams.elapsedTime );
        printf_s("%f,%d,", pidParams.pressureError,pidParams.totalStepCount );
        printf_s("%f,%d,", pidParams.controlledPressure,pidParams.stepSize);
        printf_s("%f,%f,", pidParams.pressureCorrectionTarget, pidParams.requestedMeasuredMotorCurrent);
        printf_s("%f,%d,", pidParams.measuredMotorCurrent, pidParams.opticalSensorAdcReading);
        printf_s("%d,%f,", pidParams.pistonPosition, pidParams.motorSpeed);
        printf_s("%d,%f,", pidParams.isSetpointInControllerRange, pidParams.pumpTolerance);

        fprintf_s(fptr,"\n\r%d,%f,", pidParams.elapsedTime, pidParams.pressureSetPoint);
        fprintf_s(fptr, "%d,%d,", pidParams.setPointType, pidParams.stepCount);
        fprintf_s(fptr, "%f,%d,", pidParams.pressureError, pidParams.totalStepCount);
        fprintf_s(fptr, "%f,%d,", pidParams.controlledPressure, pidParams.stepSize);
        fprintf_s(fptr, "%f,%f,", pidParams.pressureCorrectionTarget, pidParams.requestedMeasuredMotorCurrent);
        fprintf_s(fptr, "%f,%d,", pidParams.measuredMotorCurrent, pidParams.opticalSensorAdcReading);
        fprintf_s(fptr, "%d,%f,", pidParams.pistonPosition, pidParams.motorSpeed);
        fprintf_s(fptr, "%d,%f,", pidParams.isSetpointInControllerRange, pidParams.pumpTolerance);

#endif
        /*
        printf_s("\n\r%d,%f,%d,%d,%f,%d,%f,%d,%f,%f,%f,%d,%d,%f,%d,%f",
            pidParams.elapsedTime,
            pidParams.pressureSetPoint,
            pidParams.setPointType,
            pidParams.stepCount,
            pidParams.pressureError,
            pidParams.totalStepCount,
            pidParams.controlledPressure,
            pidParams.stepSize,
            pidParams.pressureCorrectionTarget,
            pidParams.requestedMeasuredMotorCurrent,
            pidParams.measuredMotorCurrent,
            pidParams.opticalSensorAdcReading,
            pidParams.pistonPosition,
            pidParams.motorSpeed,
            pidParams.isSetpointInControllerRange,
            pidParams.pumpTolerance);
           */
           /*
        fprintf_s(fptr, "\n%d,%f,%d,%d,%f,%d,%f,%d,%f,%f,%f,%d,%d,%f,%d,%f",
            pidParams.elapsedTime,
            pidParams.pressureSetPoint,
            pidParams.setPointType,
            pidParams.stepCount,
            pidParams.pressureError,
            pidParams.totalStepCount,
            pidParams.controlledPressure,
            pidParams.stepSize,
            pidParams.pressureCorrectionTarget,
            pidParams.requestedMeasuredMotorCurrent,
            pidParams.measuredMotorCurrent,
            pidParams.opticalSensorAdcReading,
            pidParams.pistonPosition,
            pidParams.motorSpeed,
            pidParams.isSetpointInControllerRange,
            pidParams.pumpTolerance
        );
            */
        
#if 0
        fprintf(fptr, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            myMode,
            controllerStatus.bytes,

            controllerStatus.bit.pumpUp,
            controllerStatus.bit.pumpDown,
            controllerStatus.bit.control,
            controllerStatus.bit.venting,

            controllerStatus.bit.stable,
            controllerStatus.bit.vented,
            controllerStatus.bit.excessLeak,
            controllerStatus.bit.excessVolume,

            controllerStatus.bit.overPressure,
            controllerStatus.bit.excessOffset,
            controllerStatus.bit.measure,
            controllerStatus.bit.fineControl,

            controllerStatus.bit.pistonCentered,
            controllerStatus.bit.centering,
            controllerStatus.bit.controlledVent,
            controllerStatus.bit.centeringVent,

            controllerStatus.bit.rangeExceeded,
            controllerStatus.bit.coarseControlError,
            controllerStatus.bit.ventDir

        );
        fprintf(fptr, "%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%d,	%d,	%f,	%f,	%f,	%f",
            bayesParams.minSysVolumeEstimateValue,
            bayesParams.maxSysVolumeEstimateValue,
            bayesParams.minEstimatedLeakRate,
            bayesParams.maxEstimatedLeakRate,
            bayesParams.measuredPressure,
            bayesParams.smoothedPresure,
            bayesParams.changeInPressure,
            bayesParams.prevChangeInPressure,
            bayesParams.estimatedVolume,
            bayesParams.algorithmType,
            bayesParams.changeInVolume,
            bayesParams.prevChangeInVolume,
            bayesParams.measuredVolume,
            bayesParams.estimatedLeakRate,
            bayesParams.measuredLeakRate,
            bayesParams.estimatedKp,
            bayesParams.measuredKp,
            bayesParams.sensorUncertainity,
            bayesParams.uncertaintyPressureDiff,
            bayesParams.uncertaintyVolumeEstimate,
            bayesParams.uncertaintyMeasuredVolume,
            bayesParams.uncertaintyVolumeChange,
            bayesParams.uncertaintyEstimatedLeakRate,
            bayesParams.uncertaintyMeasuredLeakRate,
            bayesParams.maxZScore,
            bayesParams.lambda,
            bayesParams.uncerInSmoothedMeasPresErr,
            bayesParams.targetdP,
            bayesParams.smoothedPressureErr,
            bayesParams.smoothedSqaredPressureErr,
            bayesParams.gamma,
            bayesParams.predictionError,
            bayesParams.predictionErrType,
            bayesParams.maxAchievablePressure,
            bayesParams.minAchievablePressure,
            bayesParams.maxPositivePressureChangeAchievable,
            bayesParams.maxNegativePressureChangeAchievable,
            bayesParams.minPressureAdjustmentRangeFactor,
            bayesParams.nominalHomePosition,
            bayesParams.expectedPressureAtCenterPosition,
            bayesParams.maxIterationsForIIRfilter,
            bayesParams.minIterationsForIIRfilter,
            bayesParams.changeToEstimatedLeakRate,
            bayesParams.alpha,
            bayesParams.smoothedPressureErrForPECorrection,
            bayesParams.log10epsilon
        );
        fprintf(fptr, "%f,%f,%d,%f,%f",
            testParams.maxFakeLeakRate,
            testParams.fakeLeakRate,
            testParams.isOpticalSensorCalibrationRequired,
            testParams.fakeLeak,
            testParams.volumeForLeakRateAdjustment);

        fclose(fptr);
#endif
}
void controller::logDataKeys(eControllerType_t controlType)
{
#ifdef CPP_ON_PC
    if ((eControllerType_t)eCoarseControl == controlType)
    {
        resultFile.open(coarseControlLogParams.fileName);
    }
    else if ((eControllerType_t)eFineControl == controlType)
    {
        resultFile.open(fineControlLogParams.fileName);
    }
    else
    {
        resultFile.open(resultFilePath);
    }
	if (resultFile.is_open())
	{
		resultFile << "elapsedTime" << ",";
		resultFile << "setPoint" << ",";
		resultFile << "spType" << ",";
		resultFile << "count" << ",";
		resultFile << "E" << ",";
		resultFile << "total" << ",";
		resultFile << "pressure" << ",";
		resultFile << "absPressure" << ",";
		resultFile << "gaugePressure" << ",";
		resultFile << "baroPressure" << ",";
		resultFile << "oldPressure" << ",";
		resultFile << "stepSize" << ",";
		resultFile << "targetdP" << ",";
		resultFile << "current" << ",";
		resultFile << "measCurrent" << ",";
		resultFile << "opticalADC" << ",";
		resultFile << "position" << ",";
		resultFile << "speed" << ",";
		resultFile << "inRange" << ",";
		resultFile << "pumpTolerance" << ",";
		resultFile << "mode" << ",";
		resultFile << "status" << ",";
		resultFile << "pumpUp" << ",";
		resultFile << "pumpDown" << ",";
		resultFile << "control" << ",";
		resultFile << "venting" << ",";
		resultFile << "stable" << ",";
		resultFile << "vented" << ",";
		resultFile << "excessLeak" << ",";
		resultFile << "excessVolume" << ",";
		resultFile << "overPressure" << ",";
		resultFile << "excessOffset" << ",";
		resultFile << "measure" << ",";
		resultFile << "fineControl" << ",";
		resultFile << "pistonCentered" << ",";
		resultFile << "centering" << ",";
		resultFile << "controlledVent" << ",";
		resultFile << "centeringVent" << ",";
		resultFile << "rangeExceeded" << ",";
		resultFile << "coarseControlError" << ",";
		resultFile << "ventDir" << ",";
		resultFile << "minV" << ",";
		resultFile << "maxV" << ",";
		resultFile << "minLeak" << ",";
		resultFile << "maxLeak" << ",";
		resultFile << "P" << ",";
		resultFile << "smoothP" << ",";
		resultFile << "dP" << ",";
		resultFile << "dP_" << ",";
		resultFile << "dP2" << ",";
		resultFile << "V" << ",";
		resultFile << "algoV" << ",";
		resultFile << "dV" << ",";
		resultFile << "dV_" << ",";
		resultFile << "dV2" << ",";
		resultFile << "measV" << ",";
		resultFile << "leak" << ",";
		resultFile << "measLeak" << ",";
		resultFile << "kP" << ",";
        //resultFile << "measkP" << ",";
		resultFile << "varP" << ",";
		resultFile << "vardP" << ",";
		resultFile << "varV" << ",";
		resultFile << "varMeasV" << ",";
		resultFile << "vardV" << ",";
		resultFile << "varLeak" << ",";
		resultFile << "varMeasLeak" << ",";
		resultFile << "maxZScore" << ",";
		resultFile << "lambda" << ",";
		resultFile << "VarE" << ",";
		resultFile << "targetdP" << ",";
		resultFile << "smoothE" << ",";
		resultFile << "smoothE2" << ",";
        resultFile << "gamma" << ",";
		resultFile << "predictionError" << ",";
		resultFile << "predictionErrorType" << ",";
		resultFile << "maxP" << ",";
		resultFile << "minP" << ",";
		resultFile << "maxdP" << ",";
		resultFile << "mindP" << ",";
		resultFile << "nominalRange" << ",";
		resultFile << "nominalHome" << ",";
		resultFile << "centerP" << ",";
		resultFile << "maxN" << ",";
		resultFile << "minN" << ",";
		resultFile << "dL" << ",";
		resultFile << "alpha" << ",";
		resultFile << "smoothE_PE" << ",";
		resultFile << "log10epsilon" << ",";
		resultFile << "residualL" << ",";
		resultFile << "measL" << ",";
		resultFile << "n" << ",";
		resultFile << "maxFakeLeakRate" << ",";
		resultFile << "fakeLeakRate" << ",";
		resultFile << "calibratePosition" << ",";
		resultFile << "fakeLeak" << ",";
		resultFile << "V" << ",";
		resultFile << "motorStepSize" << ",";
		resultFile << "microStep" << ",";
		resultFile << "gearRatio" << ",";
		resultFile << "pistonDiameter" << ",";
		resultFile << "lead" << ",";
		resultFile << "length" << ",";
		resultFile << "pistonArea" << ",";
		resultFile << "dV" << ",";
		resultFile << "alphaA" << ",";
		resultFile << "betaA" << ",";
		resultFile << "alphaD" << ",";
		resultFile << "betaD" << ",";
		resultFile << "maxCurrent" << ",";
		resultFile << "minCurrent" << ",";
		resultFile << "maxPressure" << ",";
		resultFile << "holdCurrent" << ",";
		resultFile << "maxPosition" << ",";
		resultFile << "minPosition" << ",";
		resultFile << "maxPosition" << ",";
		resultFile << "centerPosition" << ",";
		resultFile << "centerTolerance" << ",";
		resultFile << "maxStepSize" << ",";
		resultFile << "rShunt" << ",";
		resultFile << "shuntGain" << ",";
		resultFile << "readingToCurrent" << ",";
		resultFile << "maxLeak" << ",";
		resultFile << "maxP" << ",";
		resultFile << "nominalV" ;

		resultFile.close();
	}
#endif
#ifdef CPP_FILE
    FILE* fptr = NULL;
    if ((eControllerType_t)eCoarseControl == controlType)
    {
        fopen_s(&fptr, coarseControlLogParams.fileName, "w");
    }
    else if ((eControllerType_t)eFineControl == controlType)
    {
        fopen_s(&fptr, fineControlLogParams.fileName, "w");
    }
    else
    {
        fptr = NULL;
    }
    if (fptr != NULL)
    {
        fprintf(fptr,"  %s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s,	%s", 
                        "elapsedTime",
                        "setpoint",
                        "spType",
                        "count",
                        "E",
                        "total",
                        "pressure",
                        "stepSize",
                        "targetdP",
                        "current",
                        "measCurrent",
                        "opticalADC",
                        "position",
                        "speed",
                        "inRange",
                        "pumpTolerance",
                        "mode",
                        "status",
                        "pumpUp",
                        "pumpDown",
                        "control",
                        "venting",
                        "stable",
                        "vented",
                        "excessLeak",
                        "excessVolume",
                        "overPressure",
                        "excessOffset",
                        "measure",
                        "fineControl",
                        "pistonCentered",
                        "centering",
                        "controlledVent",
                        "centeringVent",
                        "rangeExceeded",
                        "coarseControlError",
                        "ventDir",
                        "minV",
                        "maxV",
                        "minLeak",
                        "maxLeak",
                        "P",
                        "smoothP",
                        "dP",
                        "dP_",
                        "V",
                        "algoV",
                        "dV",
                        "dV_",
                        "measV",
                        "leak",
                        "measLeak",
                        "kP",
                        "varP",
                        "vardP",
                        "varV",
                        "varMeasV",
                        "vardV",
                        "varLeak",
                        "varMeasLeak",
                        "maxZScore",
                        "lambda",
                        "varE",
                        "targetdP",
                        "smoothE",
                        "smoothE2",
                        "gamma",
                        "predictionError",
                        "predictionErrorType",
                        "maxP",
                        "minP",
                        "maxdP",
                        "mindP",
                        "nominalRange",
                        "nominalHome",
                        "centerP",
                        "maxN",
                        "minN",
                        "dL",
                        "alpha",
                        "smoothE_PE",
                        "log10epsilon",
                        "maxFakeLeakRate",
                        "fakeLeakRate",
                        "calibratePosition",
                        "fakeLeak",
                        "V",
                        "motorStepSize",
                        "microStep",
                        "gearRatio",
                        "pistonDiameter",
                        "lead",
                        "length",
                        "pistonArea",
                        "dV",
                        "alphaA",
                        "betaA",
                        "alphaD",
                        "betaD",
                        "maxCurrent",
                        "minCurrent",
                        "maxPressure",
                        "holdCurrent",
                        "maxPosition",
                        "minPosition",
                        "maxPostion",
                        "centerPosition",
                        "centerTolerance",
                        "maxStepSize",
                        "rShunt",
                        "shuntGain",
                        "readingToCurrent",
                        "maxLeak",
                        "maxP",
                        "nominalV"
            );
            fclose(fptr);
    }
#endif
}

void controller::logScrewAndMotorControlValues(eControllerType_t controlType)
{
#ifdef CPP_ON_PC
    if ((eControllerType_t)eCoarseControl == controlType)
    {
        resultFile.open(coarseControlLogParams.fileName, std::ofstream::app);
    }
    else if ((eControllerType_t)eFineControl == controlType)
    {
        resultFile.open(fineControlLogParams.fileName, std::ofstream::app);
    }
    else
    {
        resultFile.open(resultFilePath, std::ofstream::app);
    }
	if (resultFile.is_open())
	{
		resultFile << "," << motorParams.motorStepSize << ",";
		resultFile << motorParams.microStepSize << ",";
		resultFile << screwParams.gearRatio << ",";
		resultFile << screwParams.pistonDiameter << ",";
		resultFile << screwParams.leadScrewPitch << ",";
		resultFile << screwParams.leadScrewLength << ",";
		resultFile << screwParams.pistonArea << ",";
		resultFile << screwParams.changeInVolumePerPulse << ",";
		resultFile << motorParams.accelerationAlpha << ",";
		resultFile << motorParams.accelerationBeta << ",";
		resultFile << motorParams.decellerationAlpha << ",";
		resultFile << motorParams.decellerationBeta << ",";
		resultFile << motorParams.maxMotorCurrent << ",";
		resultFile << motorParams.minMotorCurrrent << ",";
		resultFile << screwParams.maxPressure << ",";
		resultFile << motorParams.holdCurrent << ",";
		resultFile << screwParams.maxPosition << ",";
		resultFile << screwParams.minPosition << ",";
		resultFile << screwParams.maxPosition << ",";
		resultFile << screwParams.centerPositionCount << ",";
		resultFile << screwParams.centerTolerance << ",";
		resultFile << motorParams.maxStepSize << ",";
		resultFile << screwParams.shuntResistance << ",";
		resultFile << screwParams.shuntGain << ",";
		resultFile << screwParams.readingToCurrent << ",";
		resultFile << screwParams.maxLeakRate << ",";
		resultFile << screwParams.maxAllowedPressure << ",";
		resultFile << screwParams.nominalTotalVolume ;
        resultFile.close();
	}
#endif
#ifdef CPP_FILE
    FILE* fptr = NULL;
    if ((eControllerType_t)eCoarseControl == controlType)
    {
        fopen_s(&fptr, coarseControlLogParams.fileName, "a");
    }
    else if ((eControllerType_t)eFineControl == controlType)
    {
        fopen_s(&fptr, fineControlLogParams.fileName, "a");
    }
    else
    {
        fptr = NULL;
    }
    
    if (fptr != NULL)
    {
        fprintf(fptr, "%d,	%d,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%d,	%d,	%d,	%d,	%d,	%d,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f",
            motorParams.motorStepSize,
            motorParams.microStepSize,
            screwParams.gearRatio,
            screwParams.pistonDiameter,
            screwParams.leadScrewPitch,
            screwParams.leadScrewLength,
            screwParams.pistonArea,
            screwParams.changeInVolumePerPulse,
            motorParams.accelerationAlpha,
            motorParams.accelerationBeta,
            motorParams.decellerationAlpha,
            motorParams.decellerationBeta,
            motorParams.maxMotorCurrent,
            motorParams.minMotorCurrrent,
            screwParams.maxPressure,
            motorParams.holdCurrent,
            screwParams.maxPosition,
            screwParams.minPosition,
            screwParams.maxPosition,
            screwParams.centerPositionCount,
            screwParams.centerTolerance,
            motorParams.maxStepSize,
            0.0f,
            0.0f,
            0.0f,
            screwParams.shuntResistance,
            screwParams.shuntGain,
            screwParams.readingToCurrent,
            screwParams.maxLeakRate,
            screwParams.maxAllowedPressure,
            screwParams.nominalTotalVolume);
    }
    fclose(fptr);
#endif
}


/*
 * @brief   Coarse control class destructor
 * @param   None
 * @retval  None
 */
controller::~controller()
{

}



/*
 * @brief   Measure mode in coarase control
 * @param   None
 * @retval  None
 */
uint32_t controller::coarseControlMeasure()
{
    uint32_t status = (uint32_t)(0);

    return status;
}

/*
 * @brief   Returns the absolute pressure value from 2 values
 * @param   None
 * @retval  None
 */
uint32_t controller::getAbsPressure(float p1, float p2, float *abs)
{
    uint32_t status = (uint32_t)(1);

    if (p1 > p2)
    {
        *abs = p1 - p2;
    }
    else
    {
        *abs = p2 - p1;
    }

    return status;
}
void controller::setMotorCurrent(void)
{
    float motorCurrRange = 0.0f;
    //# increase motor current with gauge pressure
    //# saves power for lower pressure setPoints
    motorCurrRange = motorParams.maxMotorCurrent - motorParams.minMotorCurrrent;
    pidParams.requestedMeasuredMotorCurrent = motorParams.minMotorCurrrent +
        ((motorCurrRange * absolutePressure) / screwParams.maxPressure);
    motor->writeAcclCurrent(pidParams.requestedMeasuredMotorCurrent);
    motor->writeDecelCurrent(pidParams.requestedMeasuredMotorCurrent);
}

/*
 * @brief   Get piston position
 * @param   None
 * @retval  None
 */
int32_t controller::getPistonPosition(uint32_t adcReading)
{
    int32_t position = (int32_t)(0);
    return ::controllerData[ePosition];
}

/*
 * @brief   Get piston in range
 * @param   None
 * @retval  None
 */
ePistonRange_t controller::validatePistonPosition(int32_t position)
{
    ePistonRange_t isInRange = ePistonOutOfRange;

    if (screwParams.maxPosition > position)
    {
        if (screwParams.minPosition < position)
        {
            isInRange = ePistonInRange;
        }
    }

    return isInRange;
}

/*
 * @brief   Get piston in centre
 * @param   None
 * @retval  None
 */
ePistonCentreStatus_t controller::isPistonCentered(int32_t position)
{
    int32_t positionLeft = (int32_t)(0);
    int32_t positionRight = (int32_t)(0);
    ePistonCentreStatus_t isInCentre = ePistonOutOfCentre;

    positionLeft = screwParams.centerPositionCount - screwParams.centerTolerance;
    positionRight = screwParams.centerPositionCount + screwParams.centerTolerance;

    if (positionLeft < position)
    {
        if (positionRight > position)
        {
            isInCentre = ePistonCentered;
        }
    }

    return isInCentre;
}



/*
 * @brief   Control mode CC CASE 1
 * @param   None
 * @retval  None
 */
uint32_t controller::coarseControlCase1()
{
    uint32_t status = (uint32_t)(0);
    float pumpTolerance = 0.0f;
    int32_t completedCnt = 0;
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
	float abs = 0.0f;
	getAbsPressure(setPointG, gaugePressure, &abs);
    pumpTolerance =  (abs/ absolutePressure);

    if ((pumpTolerance < pidParams.pumpTolerance) && (ePistonCentered == controllerStatus.bit.pistonCentered))
    {
        /* If this case is executed, make the caseStatus 1 */
        status = (uint32_t)(1);
        setControlIsolate();
        pidParams.stepSize = (int32_t)(0);
        
        errorStatus = motor->writeMoveContinuous((int32_t)0, &completedCnt);      //# stop the motor
        if ((eIbcError_t)E_IBC_ERROR_NONE == errorStatus)
        {
            pidParams.stepCount = completedCnt;
        }
        else
        {
            pidParams.stepCount = 0;
        }
        pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;

        controllerState = eCoarseControlExit;
    }

    return status;
}

/*
 * @brief   Control mode CC CASE 2
 * @param   None
 * @retval  None
 */
uint32_t controller::coarseControlCase2()
{
    uint32_t status = (uint32_t)(0);
    int32_t pistonCentreLeft = (int32_t)(0);

    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    int32_t completedCnt = (int32_t)0;
    pistonCentreLeft = screwParams.centerPositionCount - (screwParams.centerTolerance >> 1);

    if ((setPointG > gaugePressure) && (pidParams.pistonPosition < pistonCentreLeft))
    {
        /* Make the status 1 if this case is executed */
        status = (uint32_t)(1);
        pidParams.stepSize = motorParams.maxStepSize;

        if (ePistonNotCentering == controllerStatus.bit.centering)
        {
            setControlIsolate();
            controllerStatus.bit.centering = ePistonCentering;
            bayesParams.changeInVolume = (float)(0);
            setMotorCurrent();

            /* Write motor current to secondary uC */
            /* Write accl and decel currents TODO */
            //pidParams.stepCount = motorWrite(eMove, pidParams.stepSize);
            //pidParams.stepCount = motor->writeMoveContinuous(pidParams.stepSize);
            errorStatus = motor->writeMoveContinuous(pidParams.stepSize, &completedCnt);      //# stop the motor
            if ((eIbcError_t)E_IBC_ERROR_NONE == errorStatus)
            {
                pidParams.stepCount = completedCnt;
            }
            else
            {
                pidParams.stepCount = 0;
            }
            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
            pidParams.stepCount = 0;
            bayesParams.measuredPressure = absolutePressure;
        }
        bayesParams.changeInPressure = absolutePressure - bayesParams.measuredPressure;
        bayesParams.changeInVolume = bayesParams.changeInVolume -
            (pidParams.stepCount * screwParams.changeInVolumePerPulse);

        estimate();
    }

    return status;
}

/*
 * @brief   Control mode CC CASE 3
 * @param   None
 * @retval  None
 */
uint32_t controller::coarseControlCase3()
{
    uint32_t status = (uint32_t)(0);
    int32_t pistonCentreRight = (int32_t)(0);

    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    int32_t completedCnt = (int32_t)0;

    pistonCentreRight = screwParams.centerPositionCount + (screwParams.centerTolerance >> 1);

    if ((setPointG < gaugePressure) && (pidParams.pistonPosition > pistonCentreRight))
    {
		status = (uint32_t)(1);

        pidParams.stepSize = -1 * (motorParams.maxStepSize);
        if (ePistonNotCentering == controllerStatus.bit.centering)
        {
            setControlIsolate();
            controllerStatus.bit.centering = ePistonCentering;
            bayesParams.changeInVolume = (float)(0);
            setMotorCurrent();

            /* Write motor current to secondary uC */
            /* Write accl and decel currents TODO */
            //pidParams.stepCount = motor->writeMoveContinuous(pidParams.stepSize);// Write(eMove, pidParams.stepSize);
            errorStatus = motor->writeMoveContinuous(pidParams.stepSize, &completedCnt);      //# stop the motor
            if ((eIbcError_t)E_IBC_ERROR_NONE == errorStatus)
            {
                pidParams.stepCount = completedCnt;
            }
            else
            {
                pidParams.stepCount = 0;
            }
            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
            pidParams.stepCount = (int32_t)0;
            bayesParams.measuredPressure = absolutePressure;
        }
        bayesParams.changeInPressure = absolutePressure - bayesParams.measuredPressure;
        bayesParams.changeInVolume = bayesParams.changeInVolume -
            (pidParams.stepCount * screwParams.changeInVolumePerPulse);
        estimate();
    }
    return status;
}

/*
 * @brief   Control mode CC CASE 4
 * @param   None
 * @retval  None
 */
uint32_t controller::coarseControlCase4()
{
    uint32_t status = (uint32_t)(0);
    float effPressureNeg = 0.0f;
    float effPressurePos = 0.0f;
    float effPressure = 0.0f;

    uint32_t pistonCentreLeft = 0u;
    uint32_t psitonCentreRight = 0u;

    effPressureNeg = gaugePressure - gaugeSensorUncertainty;
    effPressurePos = gaugePressure + gaugeSensorUncertainty;


	if (((-gaugeSensorUncertainty > setPointG) && (setPointG > gaugePressure)) ||
		((gaugeSensorUncertainty < setPointG) && (setPointG < gaugePressure) )||
		((effPressureNeg > 0.0f) && (0.0f > setPointG)) ||
		((effPressurePos < 0.0f) && (0.0f < setPointG)))
    {
        status = (uint32_t)(1);
        pidParams.stepSize = (int32_t)(0);

        if (eControlledVentStopped == controllerStatus.bit.controlledVent)
        {
            setControlVent();

            effPressure = gaugePressure - gaugeSensorUncertainty;

            if (effPressure > atmosphericPressure)
            {
                controllerStatus.bit.ventDir = eVentDirDown;
            }
            else
            {
                controllerStatus.bit.ventDir = eVentDirUp;
            }

            bayesParams.measuredPressure = absolutePressure;
        }

        bayesParams.changeInPressure = bayesParams.measuredPressure - absolutePressure;

        pistonCentreLeft = screwParams.centerPositionCount - (screwParams.centerTolerance >> 1);
        psitonCentreRight = screwParams.centerPositionCount + (screwParams.centerTolerance >> 1);

        if (pidParams.pistonPosition > psitonCentreRight)
        {
            pidParams.stepSize = -1 * (motorParams.maxStepSize);

            if (eCenteringVentStopped == controllerStatus.bit.centeringVent)
            {
                controllerStatus.bit.centeringVent = eCenteringVentRunning;
            }
        }
        else if (pidParams.pistonPosition < pistonCentreLeft)
        {
            pidParams.stepSize = motorParams.maxStepSize;

            if (eCenteringVentStopped == controllerStatus.bit.centeringVent)
            {
                controllerStatus.bit.centeringVent = eCenteringVentRunning;
            }
        }
        else
        {
            pidParams.stepSize = (int32_t)(0);
            controllerStatus.bit.centeringVent = eCenteringVentStopped;

        }
    }
    return status;
}

/*
 * @brief   Control mode CC CASE 5
 * @param   None
 * @retval  None
 */
uint32_t controller::coarseControlCase5()
{
    uint32_t status = (uint32_t)(0);

    int32_t pistonCentreRight = (int32_t)(0);

    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    int32_t completedCnt = (int32_t)0;

    pistonCentreRight = screwParams.centerPositionCount - (screwParams.centerTolerance >> 1);

    if (pidParams.pistonPosition < pistonCentreRight)
    {
        status = (uint32_t)(1);
        pidParams.stepSize = motorParams.maxStepSize;
        if (ePistonNotCentering == controllerStatus.bit.centering)
        {
            setControlIsolate();
            controllerStatus.bit.centering = ePistonCentering;
            bayesParams.changeInVolume = 0.0f;
            setMotorCurrent();

            /* Write motor current to secondary uC */
            /* Write accl and decel currents TODO */
            //pidParams.stepCount = motor->writeMoveContinuous(pidParams.stepSize); // Write(eMove, pidParams.stepSize);
            errorStatus = motor->writeMoveContinuous((int32_t)pidParams.stepSize, &completedCnt);      //# stop the motor
            if ((eIbcError_t)E_IBC_ERROR_NONE == errorStatus)
            {
                pidParams.stepCount = completedCnt;
            }
            else
            {
                pidParams.stepCount = 0;
            }

            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
            pidParams.stepCount = (int32_t)0;
            bayesParams.measuredPressure = absolutePressure;
        }
        bayesParams.changeInPressure = absolutePressure - bayesParams.measuredPressure;
        bayesParams.changeInVolume = bayesParams.changeInVolume -
            (pidParams.stepCount * screwParams.changeInVolumePerPulse);
        estimate();
    }

    return status;
}

/*
 * @brief   Control mode CC CASE 6
 * @param   None
 * @retval  None
 */
uint32_t controller::coarseControlCase6()
{
    uint32_t status = (uint32_t)(0);
    int32_t pistonCentreLeft = (int32_t)(0);
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    int32_t completedCnt = (int32_t)0;
    pistonCentreLeft = screwParams.centerPositionCount + (screwParams.centerTolerance >> 1);

    if (pidParams.pistonPosition > pistonCentreLeft)
    {
        status = (uint32_t)(1);
        pidParams.stepSize = -1 * (motorParams.maxStepSize);
        if (ePistonNotCentering == controllerStatus.bit.centering)
        {
            setControlIsolate();
            controllerStatus.bit.centering = ePistonCentering;
            bayesParams.changeInVolume = 0.0f;
            setMotorCurrent();

            /* Write motor current to secondary uC */
            /* Write accl and decel currents TODO */
            //pidParams.stepCount = motor->writeMoveContinuous(pidParams.stepSize);// Write(eMove, pidParams.stepSize);
            errorStatus = motor->writeMoveContinuous((int32_t)pidParams.stepSize, &completedCnt);      //# stop the motor
            if ((eIbcError_t)E_IBC_ERROR_NONE == errorStatus)
            {
                pidParams.stepCount = completedCnt;
            }
            else
            {
                pidParams.stepCount = 0;
            }
            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
            pidParams.stepCount = (int32_t)0;
            bayesParams.measuredPressure = absolutePressure;
        }
        bayesParams.changeInPressure = absolutePressure - bayesParams.measuredPressure;
        bayesParams.changeInVolume = bayesParams.changeInVolume -
            (pidParams.stepCount * screwParams.changeInVolumePerPulse);
        estimate();
    }

    return status;
}

/*
 * @brief   Control mode CC CASE 7
 * @param   None
 * @retval  None
 */
uint32_t controller::coarseControlCase7()
{
    uint32_t status = (uint32_t)(0);

    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    int32_t completedCnt = (int32_t)0;
    /*
    pump up
    In small volumes a previous controlled vent may have overshot setpoint
    which could trigger an endless loop
    of vent and pump.  Break this loop by detecting if the the pump condition
    was preceded by a vent in the opposite direction.
    */
    if (setPointG > gaugePressure)
    {
        status = 1u;
        pidParams.stepSize = (int32_t)(0);

        if (ePumpUpNotRequired == controllerStatus.bit.pumpUp)
        {

            if ((eVentDirDown != controllerStatus.bit.ventDir) || (eVentDirNone == controllerStatus.bit.ventDir))
            {
                /* previous controlled vent was not in the opposite direction to pump action */
                setControlUp();
            }
            else
            {
                /*
                previous control vent overshot setpoint by a more than pumpTolerance
                This can happen if the volume is too small to catch the setpoint during one
                fast control iteration, in which case the control range is considerably larger than
                pumpTolerance anyway.  Assume that is the case and attempt to move to fine control even
                though pressure is not within pumpTolerance range.
                coarse adjustment complete, last iteration of coarse control loop
                */
                setControlIsolate();
                pidParams.stepSize = (int32_t)(0);
                //pidParams.stepCount = motor->writeMoveContinuous((int32_t)0);      //# stop the motor
                errorStatus = motor->writeMoveContinuous((int32_t)0, &completedCnt);      //# stop the motor
                if ((eIbcError_t)E_IBC_ERROR_NONE == errorStatus)
                {
                    pidParams.stepCount = completedCnt;
                }
                else
                {
                    pidParams.stepCount = 0;
                }
                pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
                                
                controllerState = eCoarseControlExit;
            }
        }
    }
    return status;
}

/*
 * @brief   Control mode CC CASE 8
 * @param   None
 * @retval  None
 */
uint32_t controller::coarseControlCase8()
{
    uint32_t status = (uint32_t)(0);

    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    int32_t completedCnt = (int32_t)0;

    if (setPointG < gaugePressure)
    {
        status = 1u;
        pidParams.stepSize = (int32_t)(0);

        if (ePumpDownNotRequired == controllerStatus.bit.pumpDown)
        {
            if ((eVentDirDown != controllerStatus.bit.ventDir) || (eVentDirNone == controllerStatus.bit.ventDir))
            {
                /* previous controlled vent was not in the opposite direction to pump action */
                setControlDown();
            }
            else
            {
                /*
                previous control vent overshot setpoint by a more than pumpTolerance
                This can happen if the volume is too small to catch the setpoint during one
                fast control iteration, in which case the control range is considerably larger than
                pumpTolerance anyway.  Assume that is the case and attempt to move to fine control even
                though pressure is not within pumpTolerance range.
                coarse adjustment complete, last iteration of coarse control loop
                */
                setControlIsolate();
                pidParams.stepSize = (int32_t)(0);
                //pidParams.stepCount = motor->writeMoveContinuous((int32_t)0);      //# stop the motor
                errorStatus = motor->writeMoveContinuous((int32_t)0, & completedCnt);      //# stop the motor
                if ((eIbcError_t)E_IBC_ERROR_NONE == errorStatus)
                {
                    pidParams.stepCount = completedCnt;
                }
                else
                {
                    pidParams.stepCount = 0;
                }
                pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;

                controllerState = eCoarseControlExit;
            }
        }
    }
    return status;
}

uint32_t controller::coarseControlVent(void)
{

    ePistonRange_t pistonRange = ePistonOutOfRange;
    uint32_t status = (uint32_t)(0);

    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    int32_t completedCnt = (int32_t)0;

    //# in vent mode
    //# vent to atmosphere while reading pressure slowly to decide if vent complete

    //pressure, pressureG, atmPressure, setPoint, spType, mode = pv624.readAllSlow()

    //# detect position of the piston
    if (eControlVentGetFirstReading == ventReadingNum)
    {
        status = (uint32_t)1;
        pidParams.opticalSensorAdcReading = readOpticalSensorCounts(); // PID['opticalADC'] = pv624.readOpticalSensor()
        pidParams.pistonPosition = getPistonPosition(pidParams.opticalSensorAdcReading); // PID['position'] = int(screw['readingToPosition'](PID['opticalADC']))

        controllerStatus.bit.rangeExceeded = validatePistonPosition(pidParams.pistonPosition);//# decide if in allowed range of piston position
        controllerStatus.bit.pistonCentered = isPistonCentered(pidParams.pistonPosition);
        //# assume optical position reading is accurate
        pidParams.totalStepCount = pidParams.pistonPosition; //PID['total'] = PID['position']
        previousGaugePressure = gaugePressure;
    }
    else if (eControlVentGetSecondReading == ventReadingNum)
    {
        status = (uint32_t)1;
        //if abs(oldPressureG - pressureG) < bayes['vardP'] * *0.5 and PID['centeringVent'] == 0:
        //# pressure is stable and screw is done centering
        //PID['vented'] = 1
        if ((fabs(previousGaugePressure - gaugePressure) < sqrt(bayesParams.uncertaintyPressureDiff)) &&
            (controllerStatus.bit.centeringVent == (uint32_t)0))
        {
            controllerStatus.bit.vented = (uint32_t)1;
            if (fabs(gaugePressure) > gaugeSensorUncertainty)
            {
                /*
                # TBD pressure offset is too large when vented
                # will reduce calibration accuracy
                #and may adversely affect controlled venting
                #and pumpUp / pumpDown decisions
                print('Warning: PM offset at 0 bar G:', pressureG, sensor['gaugeUncertainty'])
                */
            }
        }

        //# center piston while venting
        //# do not calculate volume as not possible while ventingand centering
        if (pidParams.pistonPosition > (screwParams.centerPositionCount + (screwParams.centerTolerance >> 2)))
        {
            //# move towards center position
            pidParams.stepSize = -1 * (motorParams.maxStepSize);
            if (controllerStatus.bit.centeringVent == (uint32_t)0)
            {
                controllerStatus.bit.centeringVent = (uint32_t)1;
            }
        }
        else if (pidParams.pistonPosition < (screwParams.centerPositionCount - (screwParams.centerTolerance >> 2)))
        {
            pidParams.stepSize = motorParams.maxStepSize;
            if (controllerStatus.bit.centeringVent == (uint32_t)0)
            {
                pidParams.stepSize = (uint32_t)0;
                controllerStatus.bit.centeringVent = (uint32_t)0;
            }
        }
        if (pidParams.stepSize != (int32_t)0)
        {
            setMotorCurrent();
            motor->writeAcclCurrent(pidParams.measuredMotorCurrent);
            motor->writeDecelCurrent(pidParams.measuredMotorCurrent);
        }
        //pidParams.stepCount = motor->writeMoveContinuous(pidParams.stepSize);
        errorStatus = motor->writeMoveContinuous((int32_t)pidParams.stepSize, &completedCnt);      //# stop the motor
        if ((eIbcError_t)E_IBC_ERROR_NONE == errorStatus)
        {
            pidParams.stepCount = completedCnt;
        }
        else
        {
            pidParams.stepCount = 0;
        }
        pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;
        ventReadingNum = eControlVentGetFirstReading;
    }
    return status;
}



/*
 * @brief   Coarse control main control loop
 * @param   None
 * @retval  None
 */
void controller::coarseControlLoop(void)
{
    uint32_t caseStatus = (uint32_t)(0);
    //eControllerMode_t mode = PV624->getMode(); /* read mode set by Genii */
    eIbcError_t errorStatus = E_IBC_ERROR_NONE;
    int32_t completedCnt = (int32_t)0;

    if (eFineControlDisabled == controllerStatus.bit.fineControl) /* Run coarse control only if fine control is not enabled */
    {
        if ((eModeMeasure == myMode) && ((uint32_t)(1) == controllerStatus.bit.control))
        {
            /* Mode set by genii is measure but PID is in control mode */
            /* Change mode to measure */
            setMeasure();
        }
        else if ((eModeMeasure == myMode) && ((uint32_t)(1) == controllerStatus.bit.venting))
        {
            /* Mode set by genii is measure but PID is venting */
            /* Change mode to measure */
            setMeasure();
        }
        else if ((eModeControl == myMode) && ((uint32_t)(1) == controllerStatus.bit.measure))
        {
            /* Mode set by genii is control but PID is in measure */
            /* Change mode to measure */
            setControlIsolate();
        }
        else if ((eModeControl == myMode) && ((uint32_t)(1) == controllerStatus.bit.venting))
        {
            /* Mode set by genii is control but PID is in measure */
            /* Change mode to measure */
            setControlIsolate();
        }
        else if ((eModeVent == myMode) && ((uint32_t)(1) == controllerStatus.bit.measure))
        {
            /* Mode set by genii is control but PID is in measure */
            /* Change mode to measure */
            ventReadingNum = eControlVentGetFirstReading;
            setControlVent();
        }
        else if ((eModeVent == myMode) && ((uint32_t)(1) == controllerStatus.bit.control))
        {
            /* Mode set by genii is control but PID is in measure */
            /* Change mode to measure */
            ventReadingNum = eControlVentGetFirstReading;
            setControlVent();
        }
        else if (eModeMeasure == myMode)
        {
            /* Mode is correct, so reset coarse control error */
            controllerStatus.bit.coarseControlError = eCoarseControlErrorReset;
            pidParams.controlledPressure = pressureAsPerSetPointType(); //[pressureG, pressure, atmPressure][spType] + testParams.fakeLeak;
            pidParams.pressureSetPoint = pressureSetPoint;
            pidParams.setPointType = setPointType;
            coarseControlMeasure();
        }
        else if (eModeControl == myMode)
        {
            /* Mode is correct, so reset coarse control error */
            controllerStatus.bit.coarseControlError = eCoarseControlErrorReset;

            /* Read gauge pressure also */
            pidParams.setPointType = setPointType;
            pidParams.controlledPressure = pressureAsPerSetPointType();;
            
            pidParams.pressureSetPoint = pressureSetPoint;
            pidParams.opticalSensorAdcReading = readOpticalSensorCounts();
			setPointG = pidParams.pressureSetPoint - (atmosphericPressure * setPointType);

            /* Check screw position */
            pidParams.pistonPosition = getPistonPosition(pidParams.opticalSensorAdcReading);
            /* Check if piston is within range */
            controllerStatus.bit.rangeExceeded = validatePistonPosition(pidParams.pistonPosition);
            /* Check if piston is centered */
            controllerStatus.bit.pistonCentered = isPistonCentered(pidParams.pistonPosition);

            /* The coarse contol algorithm runs in one of 8 cases as below

            Case 1, check if current pressure is within pump tolerance
            and piston is also centered
            Case 2, set point is on the way to center while increasing pressure
            Case 3, set point is on the way to center while decreasing pressure
            Case 4, set point is crossing from pressure to vacuum or from vacuum
            to pressure
            Case 5, piston is not in center and moving it towards center
            shall increase pressure error and require manual pump down action
            Case 6, piston is not in center and moving it towards center will
            increase pressure error and require manual pump up action
            Case 7, Set point is greater than pressure
            Case 8, Set point is less than pressure
            */
            caseStatus = coarseControlCase1();
            if ((uint32_t)(0) == caseStatus)
            {
                caseStatus = coarseControlCase2();
                if ((uint32_t)(0) == caseStatus)
                {
                    caseStatus = coarseControlCase3();
                    if ((uint32_t)(0) == caseStatus)
                    {
                        caseStatus = coarseControlCase4();
                        if ((uint32_t)(0) == caseStatus)
                        {
                            caseStatus = coarseControlCase5();
                            if ((uint32_t)(0) == caseStatus)
                            {
                                caseStatus = coarseControlCase6();
                                if ((uint32_t)(0) == caseStatus)
                                {
                                    caseStatus = coarseControlCase7();
                                    if ((uint32_t)(0) == caseStatus)
                                    {
                                        caseStatus = coarseControlCase8();
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if ((int32_t)(0) != pidParams.stepSize)
            {
                setMotorCurrent();                                               
            }
            //pidParams.stepCount = motor->writeMoveContinuous(pidParams.stepSize);
            errorStatus = motor->writeMoveContinuous((int32_t)pidParams.stepSize, &completedCnt);      //# stop the motor
            if ((eIbcError_t)E_IBC_ERROR_NONE == errorStatus)
            {
                pidParams.stepCount = completedCnt;
            }
            else
            {
                pidParams.stepCount = 0;
            }

            pidParams.totalStepCount = pidParams.totalStepCount + pidParams.stepCount;          

        }
        else if (eModeVent == myMode)
        {
            /* Mode is correct, so reset coarse control error */
            controllerStatus.bit.coarseControlError = eCoarseControlErrorReset;
            pidParams.setPointType = setPointType;
            pidParams.controlledPressure = pressureAsPerSetPointType();
            
            pidParams.pressureSetPoint = pressureSetPoint;
            pidParams.opticalSensorAdcReading = readOpticalSensorCounts();
            setPointG = pidParams.pressureSetPoint - (atmosphericPressure * setPointType);
                        
            coarseControlVent();
        }
        else
        {
            /* Mode is neither of measure, control or vent */
            /* This is an invalid case, hence signal error */
            controllerStatus.bit.coarseControlError = eCoarseControlErrorSet;
        }
#ifdef CPP_ON_PC
        logPidBayesAndTestParamDataValues(eCoarseControl);
#endif
    }
}

#define TERPS 1

void controller::coarseControlSmEntry(void)
{
    time_t now = time(0);
    tm* ltm = new tm;
    localtime_s(ltm ,&now);
    coarseControlLogParams.startTime = now;
    float uncertaintyScaling = (float)0.0;

#ifdef RUN_ON_MICRO
    sprintf_s(coarseControlLogParams.fileName,80u, "% d-%02d-%02d%02d:%02d:%02d", ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
#else
    //strcpy_s(coarseControlLogParams.fileName, coarseControlFileName);
#endif
    //# reset control status flags
    //    # TBD action when control error flags != 0 ?
    controllerStatus.bit.stable = 0;
    controllerStatus.bit.excessLeak = 0;
    controllerStatus.bit.excessVolume = 0;
    controllerStatus.bit.overPressure = 0;
    controllerStatus.bit.rangeExceeded = 0;

    //# scale default measurement uncertainties to PM FS value
#ifdef TERPS
    uncertaintyScaling = 16 * (sensorFsValue / fsValue) * (sensorFsValue / fsValue);
#else
	uncertaintyScaling = 1 * (sensorFsValue / fsValue) * (sensorFsValue / fsValue);
	uncertaintyScaling = 1 * (sensorFsValue / fsValue) * (sensorFsValue / fsValue);
#endif
    fsValue = sensorFsValue;  //# PM full scale pressure(mbar)
    //    sensor['Ptype'] = pv624.readSensorType()  # PM sensor type
    bayesParams.sensorUncertainity = uncertaintyScaling * bayesParams.sensorUncertainity; //bayes['varP']  //# uncertainty in pressure measurement(mbar)
    bayesParams.uncertaintyPressureDiff = uncertaintyScaling * bayesParams.uncertaintyPressureDiff;  //# uncertainty in measured pressure changes(mbar)

   controllerStatus.bit.fineControl = 0;  //# disable fine pressure control
    

    //# detect position of the piston
    pidParams.opticalSensorAdcReading = readOpticalSensorCounts();
    pidParams.pistonPosition = getPistonPosition(pidParams.opticalSensorAdcReading);
    
    controllerStatus.bit.pistonCentered = isPistonCentered(pidParams.pistonPosition);    //PID['pistonCentered'] = (

        //# assume optical position reading is accurate
    pidParams.totalStepCount = pidParams.pistonPosition;

    logDataKeys((eControllerType_t)eCoarseControl);
    logPidBayesAndTestParamDataValues((eControllerType_t)eCoarseControl);
    logScrewAndMotorControlValues((eControllerType_t)eCoarseControl);
    controllerState = eCoarseControlLoop;
}

void controller::coarseControlSmExit(void)
{
    //# coarse adjustment complete, last iteration of coarse control loop
    
    controllerStatus.bit.fineControl = (uint32_t)1;  //# exiting coarse control after this iteration
    pidParams.stepSize = (uint32_t)0;
    
    //# read pressure and position once more before moving to fine control
    pidParams.opticalSensorAdcReading = readOpticalSensorCounts();
    pidParams.pistonPosition = getPistonPosition(pidParams.opticalSensorAdcReading);
    
    pidParams.controlledPressure = pressureAsPerSetPointType();//# pressure in setpoint units
    controllerStatus.bit.rangeExceeded = ePistonInRange; //# reset rangeExceeded flag set by fineControl()
    controllerState = eFineControlLoopEntry;
}
void controller::fineControlSmEntry(void)
{
    time_t now = time(0);
    tm* ltm = new tm;
    localtime_s(ltm, &now);
    coarseControlLogParams.startTime = now;

    resetBayesParameters();
#ifdef RUN_ON_MICRO
    sprintf_s(fineControlLogParams.fileName, 80u, "%d-%02d-%02d%02d:%02d:%02d_%3.7f_%d",
                                                    ltm->tm_year + 1900,
                                                    ltm->tm_mon + 1,
                                                    ltm->tm_mday,
                                                    ltm->tm_hour,
                                                    ltm->tm_min,
                                                    ltm->tm_sec,
                                                    pidParams.pressureSetPoint,
                                                    pidParams.setPointType);
#else
   // strcpy_s(fineControlLogParams.fileName, fineControlFileName);
#endif

#ifdef CPP_ON_PC
    logDataKeys((eControllerType_t)eFineControl);
    logPidBayesAndTestParamDataValues((eControllerType_t)eFineControl);
#endif
    controllerState = eFineControlLoop;    
}
void controller::initPidParams(void)
{
    pidParams.elapsedTime = (uint32_t)0;                     //# elapsed time for log file(s)
    pidParams.pressureSetPoint = (float)0.0;                 //# pressure setpoint(mbar)
    pidParams.setPointType = (setPointType_t)0;              //# setpoint type from GENII(0 = gauge, 1 = abs, 2 = baro)
    pidParams.stepCount = (int32_t)0;                        //# number of motor pulses delivered since last stepSize request
    pidParams.pressureError = (float)0.0;                    //# pressure error for PID(mbar), +ve == below pressure setpoint
    pidParams.totalStepCount = (int32_t)0;                   //# total step count since start
    pidParams.controlledPressure = (float)0.0;               //# controlled pressure(mbar gage or mbar abs)
    pidParams.stepSize = (int32_t)0;                         //# requested number of steps to turn motor
    pidParams.pressureCorrectionTarget = (float)0.0;         //# leak - adjusted pressure correction target(mbar)
    pidParams.requestedMeasuredMotorCurrent = (float)0.0;    //# requested measured motor current(mA)
    pidParams.measuredMotorCurrent = (float)0.0;             //# measured motor current(mA)
    pidParams.opticalSensorAdcReading = (uint32_t)0;         //# optical sensor ADC reading(0 to 4096)
    pidParams.pistonPosition = (int32_t)0;                   //# optical piston position(steps), 0 == fully retracted / max volume
    pidParams.motorSpeed = (float)0;                         //# motor speed from motor controller(pps)
    pidParams.isSetpointInControllerRange = true;            //# setpoint target in controller range, based on bayes range estimate
    pidParams.pumpTolerance = (float)0.005;                  //# max relative distance from setpoint before pumping is required, e.g. 0.1 == 10 % of setpoint
}
void controller::initMotorParams(void)
{
    motorParams.motorStepSize = (float)1.8;         //#full step angle of motor(deg)
    motorParams.microStepSize = (float)4.0;        //#number of microsteps(2 == halfstep)
    motorParams.accelerationAlpha = (float)0.98;    //#motor controller s - curve alpha acceleration value, from excel calculator, fast to full speed
    motorParams.accelerationBeta = (float)2.86;     //#motor controller s - curve beta acceleration value, from excel calculator, fast to full speed
    motorParams.decellerationAlpha = (float)1.02;   //#motor controller s - curve alpha decelleration value, from excel calculator
    motorParams.decellerationBeta = (float)0.0;     //#motor controller s - curve beta decelleration value, from excel calculator
    motorParams.maxMotorCurrent = (float)2.0;       //#maximum motor current(A)
    motorParams.minMotorCurrrent= (float)0.7;       //#minimum motor current(A), amount required to overcome friction around 0 bar g
    motorParams.holdCurrent = (float)0.2;           //#hold current(A), used when when not moving, minimum = 0.2 A to avoid motor oscillations when controlling
    motorParams.maxStepSize = (int32_t)3000; //# maximum number of steps that can be taken in one control iteration, used in coarse control loop

}
void controller::initScrewParams(void)
{
    screwParams.gearRatio = (float)1.0; //#gear ratio of motor
    screwParams.pistonDiameter = (float)12.2; //#piston diameter(mm), Helix0.6 press
    screwParams.leadScrewPitch = (float)0.6; //#lead screw pitch(mm / rotation)
    screwParams.leadScrewLength = (float)38.0; //# travel piston(mm)
    screwParams.pistonArea = piValue * (screwParams.pistonDiameter / 2.0) * (screwParams.pistonDiameter / 2.0);  //#piston area(mm ^ 2)
    screwParams.changeInVolumePerPulse = (motorParams.motorStepSize * screwParams.leadScrewPitch * screwParams.pistonArea * (float)1e-3) / (motorParams.microStepSize * screwParams.gearRatio * (float)360.0); //#volume change per control pulse(mL / pulse)
    screwParams.maxPressure = (float)21000.0; //#maximum system pressure(mbar) for accel / decel current scaling
    
    //# pulse count for full compression of piston, with 2 % safety factor to avoid collisions
    screwParams.maxPosition = int(0.98 * screwParams.leadScrewLength / screwParams.leadScrewPitch * (float)360.0 / motorParams.motorStepSize * motorParams.microStepSize);

    //# pulse count for full retraction of piston, with 2 % safety factor to avoid collisions
    screwParams.minPosition = int(screwParams.maxPosition / (float)0.98 * (float)0.02);

        //# use measured values instead of calculated
    screwParams.maxPosition = (int32_t)48800;
    screwParams.minPosition = (int32_t)1600;

    screwParams.centerPositionCount = (int32_t)25000; //# step count at nominal center position(home)
    screwParams.centerTolerance = (int32_t)2000; //# tolerance for finding center position(counts)
    screwParams.shuntResistance = (float)0.05;  //# shunt resistance for current sensor, motor testing(ohm)
    screwParams.shuntGain = 0.2f * (20.0f * 50.0f / (20.0f + 50.0f));  //# V / V gain of current sensor, with 20k Rload sensor and 50k ADC input impedance
    screwParams.readingToCurrent = 3.3f / (4096.0f * screwParams.shuntGain * screwParams.shuntResistance) * 1000.0f;  //# conversion factor from shunt ADC counts to current(mA)
    screwParams.maxLeakRate = (float)0.3f;  //# maximum leak rate bound(mbar / interation) at 20 bar and 10 mA, PRD spec is < 0.2
    screwParams.maxAllowedPressure = (float)21000.0;  //# maximum allowed pressure in screw press(PM independent) (mbar), for max leak rate adjustments
    screwParams.nominalTotalVolume = (float)10.0;  //# nominal total volume(mL), for max leak rate adjustments

}
void controller::initBayesParams(void)
{
    bayesParams.minSysVolumeEstimateValue = (float)5.0; //#minimum system volume estimate value(mL)
    bayesParams.maxSysVolumeEstimateValue = (float)100.0; //#maximum system volume estimate value(mL)
    bayesParams.minEstimatedLeakRate = (float)0.0; //#minimum estimated leak rate(mbar)
    bayesParams.maxEstimatedLeakRate  = (float)0.2; //#maximum absolute value of estimated leak rate(+/ -mbar / iteration)
    bayesParams.measuredPressure = (float)1000.0; //#measured pressure(mbar)
    bayesParams.smoothedPresure = (float)1000.0; //#smoothed pressure, depends on controlled pressure stability not sensor uncertainty, (mbar)
    bayesParams.changeInPressure = (float)0.0; //#measured change in pressure from previous iteration(mbar)
    bayesParams.prevChangeInPressure = (float)0.0; //#previous dP value(mbar)
    bayesParams.estimatedVolume = bayesParams.maxSysVolumeEstimateValue; //#estimate of volume(mL), set to minV to give largest range estimate on startup
    bayesParams.algorithmType = eMethodNone; //#algorithm used to calculate V
    bayesParams.changeInVolume = (float)0.0; //#volume change from previous stepSize command(mL)
    bayesParams.prevChangeInVolume = (float)0.0; //#previous dV value(mL), used in regression method
    bayesParams.measuredVolume = bayesParams.maxSysVolumeEstimateValue; //#volume estimate using Bayes regression(mL)
    bayesParams.estimatedLeakRate = bayesParams.minEstimatedLeakRate; //#estimate in leak rate(mbar / iteration), from regression method
    bayesParams.measuredLeakRate = bayesParams.minEstimatedLeakRate; //#measured leak rate using Bayes regression(mbar / iteration)
    bayesParams.estimatedKp = (float)500.0; //#estimated kP(steps / mbar) that will reduce pressure error to zero in one iteration, large for fast initial response
    //#bayes['measkP'] = bayes['kP'] #measured optimal kP(steps / mbar) that will reduce pressure error to zero in one iteration
    //#state value variances
    //bayesParams.sensorUncertainity = (10e-6 * sensorFsValue) * (10e-6 * sensorFsValue); //#uncertainty in pressure measurement(mbar), sigma ~= 10 PPM of FS pressure @ 13 Hz read rate
    bayesParams.sensorUncertainity = (10e-6 * fsValue) * (10e-6 * fsValue);
    bayesParams.uncertaintyPressureDiff = 2 * bayesParams.sensorUncertainity; //#uncertainty in measured pressure differences(mbar)
    bayesParams.uncertaintyVolumeEstimate = bayesParams.maxSysVolumeEstimateValue * 1e6; //#uncertainty in volume estimate(mL), large because initial volume is unknown, from regression method
    bayesParams.uncertaintyMeasuredVolume = (screwParams.changeInVolumePerPulse * 10) * (screwParams.changeInVolumePerPulse * 10); //#uncertainty in volume estimate from latest measurement using bayes regression(mL)
    bayesParams.uncertaintyVolumeChange = (screwParams.changeInVolumePerPulse * 10) * (screwParams.changeInVolumePerPulse * 10); //# uncertainty in volume change, depends mostly on backlash ~= +/ -10 half - steps, constant, mL

    bayesParams.uncertaintyEstimatedLeakRate = bayesParams.uncertaintyPressureDiff; //#uncertainty in leak rate from bayes estimateand gas law(mbar / iteration), from regression method
    bayesParams.uncertaintyMeasuredLeakRate  = bayesParams.uncertaintyPressureDiff; //#measured leak rate uncertainty from bayes regresssion estimate(mbar / iteration)
    bayesParams.maxZScore = (float)2.0; // #maximum variance spread between measuredand estimated values before estimated variance is increased
    bayesParams.lambda = (float)0.1; //#forgetting factor for smoothE
    bayesParams.uncerInSmoothedMeasPresErr  = (float)0.0; //#smoothed measured pressure error variance(mbar * *2)
    bayesParams.targetdP = (float)0.0; //#target correction from previous control iteration(mbar)
    bayesParams.smoothedPressureErr  = (float)0.0; //#smoothed pressure error(mbar)
    bayesParams.smoothedSqaredPressureErr = (float)0.0; //#smoothed squared pressure error(mbar * *2)
    bayesParams.uncerInSmoothedMeasPresErr  = (float)0.0; //#smoothed measured pressure error variance(mbar * *2)
    bayesParams.gamma = (float)0.98; //#volume scaling factor for nudging estimated volume with predictionError, (0.90, 0.98), larger = faster response but noisier estimate
    bayesParams.predictionError  = (float)0.0; //#prediction error from previous control iteration(mbar)
    bayesParams.predictionErrType = (int32_t)0; //#prediction error type(+/ -1), for volume estimate adjustment near setpoint
    bayesParams.maxAchievablePressure  = (float)0.0; //#maximum achievable pressure, from bayes estimates(mbar)
    bayesParams.minAchievablePressure  = (float)0.0; //#minimum achievable pressure, from bayes estimates(mbar)
    bayesParams.maxPositivePressureChangeAchievable  = (float)1e6; //#maximum positive pressure change achievable, from bayes estimates(mbar)
    bayesParams.maxNegativePressureChangeAchievable = (float)-1e6; //#maximum negative pressure change achievable, from bayes estimates(mbar)
    bayesParams.minPressureAdjustmentRangeFactor = pidParams.pumpTolerance; //#minimum pressure adjustment range factor when at nominalHome, e.g. 0.1 = minimum + / -10 % adjustment range of P at nominalHome piston location
    bayesParams.nominalHomePosition = screwParams.centerPositionCount; //#nomimal "home" position to achieve + / -10 % adjustability
    bayesParams.expectedPressureAtCenterPosition = (float)0.0; //#expected pressure at center piston position(mbar)
    bayesParams.maxIterationsForIIRfilter  = (uint32_t)100; //#maximum iterations for leak rate integration filter in PE correction method
    bayesParams.minIterationsForIIRfilter  = (uint32_t)10; //#minimum iterations for leak rate integration filter in PE correction method
    bayesParams.changeToEstimatedLeakRate = (float)0.0; //#change to estimated leak rate for PE correction method(mbar / iteration)
    bayesParams.alpha = (float)0.1; //#low - pass IIR filter memory factor for PE correction method(0.1 to 0.98)
    bayesParams.smoothedPressureErrForPECorrection = (float)0; //#smoothed pressure error for PE correction method(mbar)
    bayesParams.log10epsilon = (float)-0.7; //#acceptable residual fractional error in PE method leak rate estimate(-2 = +/ -1 %, -1 = 10 %, -0.7 = 20 %)

}
void controller::initTestParams(void)
{
    //# simulated leak rate(mbar / iteration) at 10mL and 20 bar
    testParams.maxFakeLeakRate = screwParams.maxLeakRate * 0.5;
    testParams.maxFakeLeakRate = (float)0.0;

    //# simulated leak rate, adjusted by current pressure and volume(mbar / iteration)
    testParams.fakeLeakRate = testParams.maxFakeLeakRate;

    //# calibrate optical sensor if True
    testParams.isOpticalSensorCalibrationRequired = true;

    testParams.fakeLeak = (float)0.0;  //# simulated cumulative leak effect(mbar)
    testParams.volumeForLeakRateAdjustment = (float)10.0;  //# fixed volume value used for leak rate adjustment(mL)

}