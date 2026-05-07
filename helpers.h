#include "gerry.h"

#ifndef HELPERS_H
#define HELPERS_H

/*  GERRY-Iterative-Repair
    Copyright (C) 2026  Brooklyn Beck

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see https://www.gnu.org/licenses/.
*/

//takes in randomized points in space
//returns execution time to slew and energy use
void slew(double* A, double* B, double * executionTime, double * energyUse);

//takes in start and end time of charging
//returns amount of energy charged
double solarRechargeEnergyFromTime(double startTime, double endTime);

//takes in start time and required energy
//returns execution time
double solarRechargeTimeFromEnergy(double startTime, double energyNeeded);
double timeTillRequiredSunlight(double t0, double sunlightTime);

//takes in a start and end time
//returns the amount of time in the sun during that period
double sunlightStatus(double startTime, double endTime);
void findEclipse(double * R_Sun_eci, double RAAN, double omega, double i, double e, double r_p);
double trueAnomaly(double e, double a, double t);
double timeSincePeriapse(double e, double a, double nu);
void nextTrueAnomaly(double * releaseTime, double * deadline);
void peri2Equatorial(double * r_equatorial, double * r_perifocal, double RAAN, double omega, double i);
void equatorial2Peri(double * r_perifocal, double * r_equatorial, double RAAN, double omega, double i);

//not currently applied
/*
//calculates the next communication window with the ground station
struct WCConstraint groundStationWindow(double stateAnnotations[9], double taskAnnotations[9]);

//table look up of time, power, and energy to accomplish a specific mechanical task
struct resource * mechanicalProcess(double stateAnnotations[9], double taskAnnotations[9]);

//returns memory freed, cpu taken up, and power/energy used, given a downlink window
//calls the groundStationWindow helper function
struct resource * downlink(double stateAnnotations[9], double taskAnnotations[9]);

//returns memory used and cpu taken up by uplinking data given a ground station
//calls the groundStationWindow helper function
struct resource * uplink(double stateAnnotations[9], double taskAnnotations[9]);

//calculates the average power and energy required to run the heater to get to desired temperature
//contains a look up table of required temperatures for operations
struct resource heating(double stateAnnotations[9], double taskAnnotations[9]);

//calculates the average power, energy, and memory required for an imaging task given the window
//calls the sunlightStatus helper function
struct resource * imaging(double stateAnnotations[9], double taskAnnotations[9]);

//calculates the expected CPU and power to process a known amount of image data
struct resource * imageProcessing(double stateAnnotations[9], double taskAnnotations[9]);
*/
#endif


