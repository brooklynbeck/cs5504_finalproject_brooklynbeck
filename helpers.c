#include "gerry.h"
#include "helpers.h"

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

/*  Function slew
    Helper function that takes in 3D vectors and returns execution time to slew and energy use
    Note: If a slew task is failed, assume that it did not execute at all
*/
void slew(double* A, double* B, double * executionTime, double * energyUse){
    //assumes smooth rotation between two points along shortest path
    //assumes starting and ending speed of 0
    double maxSlewRate = 3 * M_PI/180; //rad/sec
    double power = -0.15; //150 mW
    //angle between points
    double theta0 = 0;
    double theta1 = acos((A[0]*B[0] + A[1]*B[1] + A[2]*B[2])/(sqrt(A[0]*A[0] + A[1]*A[1] + A[2]*A[2])*sqrt(B[0]*B[0] + B[1]*B[1] + B[2]*B[2])));
    double thetadot0 = 0;
    double thetadot1 = 0;
    //using a cubic hermite spline
    double t_maxVelocity = (-6*theta0 - 4*thetadot0 - 6*theta1 - 2*thetadot1)/(12*theta0 + 6*thetadot0 - 12*theta1 + 6*thetadot1);
    double v_max = (6*t_maxVelocity*t_maxVelocity - 6*t_maxVelocity)*theta0 + (3*t_maxVelocity*t_maxVelocity - 4*t_maxVelocity + 1)*thetadot0
                    + (-6*t_maxVelocity*t_maxVelocity + 6*t_maxVelocity)*theta1 + (3*t_maxVelocity*t_maxVelocity - 2*t_maxVelocity)*thetadot1;
    double t_scaled = v_max / maxSlewRate; //seconds
    executionTime[0] = t_scaled / 60; //minutes
    energyUse[0] = t_scaled*power; //Watt-seconds
}

/*  Function solarRechargeEnergyFromTime
    Helper function that takes in a start and end time and returns the energy generated in that time
    Note: Start and End time in minutes
*/
double solarRechargeEnergyFromTime(double startTime, double endTime){
    double efficency = 0.42; //standard
    double area = 1.5 * .1*.1; //1u cubesat, m^2, average projection of a cube
    //assume that the three sides are pointed to the sun, ignoring angle of sunlight
    double solarIrradiance = 1361; //watts/m^2
    double rechargeRate = efficency*area*solarIrradiance; //Watts
    //printf("%lf %lf\n", startTime, endTime);
    double timeInSun = sunlightStatus(startTime*60, endTime*60); //send time in seconds
    //printf("time in sun = %lf s\n", timeInSun);
    return rechargeRate * timeInSun * 1/60/60 ; //Watt-seconds * hours/seconds = Watt-hours
}

/*  Function solarRechargeTimeFromEnergy
    Helper function that takes in a start time and the energy needed and returns the execution time of a recharge task
*/
double solarRechargeTimeFromEnergy(double startTime, double energyNeeded){
    double efficency = 0.42;
    double area = 1.5 * .1*.1;
    double solarIrradiance = 1361; //watts/m^2
    double rechargeRate = efficency*area*solarIrradiance; //watts
    double executionTime = 0;
    
    double sunlightTime = energyNeeded * 60*60 / rechargeRate;  //watt-hours * seconds/hours / watts = seconds
    
    executionTime = timeTillRequiredSunlight(startTime * 60, sunlightTime); //send startTime in seconds, returns executionTime in seconds
    
    return executionTime / 60; //minutes
}

/*  Function timeTillRequiredSunlight
    Helper function that takes in a start time and the amount of sunlight needed, and returns the total time (including eclipse) to get that much sun
*/
double timeTillRequiredSunlight(double t0, double sunlightTime){ //t0 and sunlightTime in seconds
    double completeTime = 0;
    double mu = 3.98601 * pow(10,5); //km^3/s^2
    
    //assume starts at periapse at time 0.00
    //assume periapse facing the sun
    //other assumptions
    double R_E = 6378; //km
    double r_p = R_E + 488;  //km
    double r_a = R_E + 509;  //km
    
    //double i = M_PI/180 * 97.4; //radians
    //double RAAN = 0; //needs update
    //double omega = 0; //needs update
    
    //orbit calculations
    double a = (r_a + r_p) / 2.; //km
    //printf("test a: %lf\n", a);
    double e = (r_a - r_p) / (r_p + r_a); //km
    double T = 2 * M_PI * sqrt(pow(a,3)/mu); //orbit period, seconds
    //printf("Test %lf\n", T);
    t0 = fmod(t0,T);
    
    //use function below to determine angles of eclipse, need to update with satelite orbit
    /*//R_Sun: location of the sun from earth
    double R_Sun_mag = 1.496*pow(10,8);
    double RA_Sun = (23+10/60+1/60/60)*15 *M_PI/180; //midnight on march 7, 2026, radians, values from theskylive.com
    double declination_Sun = -(5 + 21/60 +33/60/60) *M_PI/180; //midnight on march 7, 2026, radians, values from theskylive.com
    double R_Sun_eci[3] = {R_Sun_mag * cos(declination_Sun)*cos(RA_Sun), R_Sun_mag * cos(declination_Sun)*sin(RA_Sun), R_Sun_mag * sin(declination_Sun)};
    findEclipse(R_Sun_eci, RAAN, omega, i, e, r_p);//*/
    
    double nu_entersEclipse = 1.911224; //radians, determined by funtion above
    double nu_leavesEclipse = 4.261047; //radians, determined by function above
    //printf("enters at %lf, leaves at %lf, period %lf\n", timeSincePeriapse(e,a,nu_entersEclipse), timeSincePeriapse(e,a,nu_leavesEclipse), T);
    double t_entersEclipse = timeSincePeriapse(e, a, nu_entersEclipse); //seconds since periapse
    double t_leavesEclipse = timeSincePeriapse(e, a, nu_leavesEclipse); //seconds since periapse
    
    double sunlightTimePerOrbit = T - (t_leavesEclipse - t_entersEclipse);
    
    if (sunlightTime > sunlightTimePerOrbit){
        printf("NOTE multiple orbits! (T=%lf)\n",T);
        completeTime += floor(sunlightTime / sunlightTimePerOrbit) * T;
        sunlightTime = fmod(sunlightTime, sunlightTimePerOrbit);
    }
    
    double theta0 = trueAnomaly(e, a, t0);
    
    if(theta0 < nu_entersEclipse) //starts before eclipse
    { 
        if (trueAnomaly(e, a, t0 + sunlightTime) < nu_entersEclipse) //does not enter eclipse
        {
            completeTime += sunlightTime;
        }
        else //passes through eclipse
        {
            completeTime += sunlightTime + (t_leavesEclipse - t_entersEclipse);
        }
    }
    else if (theta0 < nu_leavesEclipse)//starts in eclipse
    { 
        completeTime += (t_leavesEclipse - t0); //wait to leave the eclipse
        
        if (trueAnomaly(e, a, t_leavesEclipse + sunlightTime) < t_entersEclipse) //doesn't enter back into the eclipse
        {
            completeTime += sunlightTime;
        }
        else //enters back into the eclipse
        {
            completeTime += sunlightTime + (t_leavesEclipse - t_entersEclipse);
        }
    }
    else //starts after eclipse
    {
        if (trueAnomaly(e, a, t_leavesEclipse + sunlightTime) < t_entersEclipse) //doesn't enter back into the eclipse
        {
            completeTime += sunlightTime;
        }
        else //passes through the eclipse
        {
            completeTime += sunlightTime + (t_leavesEclipse - t_entersEclipse);
        }
    }
    return completeTime;
}

/*  Function sunlightTime
    Helper function that takes in a start and end time and returns the amount of time in the sun during that period
*/
double sunlightStatus(double t0, double t1){
    double timeInSun = 0;
    double mu = 3.98601 * pow(10,5); //km^3/s^2
    
    //assume starts at periapse at time 0.00
    //assume periapse facing the sun
    //other assumptions
    double R_E = 6378; //km
    double r_p = R_E + 488;  //km
    double r_a = R_E + 509;  //km
    
    //double i = M_PI/180 * 97.4; //radians
    //double RAAN = 0; //needs update
    //double omega = 0; //needs update
    
    //orbit calculations
    double a = (r_a + r_p) / 2.; //km
    double e = (r_a - r_p) / (r_p + r_a); //km
    double T = 2 * M_PI * sqrt(pow(a,3)/mu); //orbit period, seconds
    
    //R_Sun: location of the sun from earth
    //double R_Sun_mag = 1.496*pow(10,8);
    //double RA_Sun = (23+10/60+1/60/60)*15 *M_PI/180; //midnight on march 7, 2026, radians, values from theskylive.com
    //double declination_Sun = -(5 + 21/60 +33/60/60) *M_PI/180; //midnight on march 7, 2026, radians, values from theskylive.com
    //double R_Sun_eci[3] = {R_Sun_mag * cos(declination_Sun)*cos(RA_Sun), R_Sun_mag * cos(declination_Sun)*sin(RA_Sun), R_Sun_mag * sin(declination_Sun)};
    
    //use function below to determine angles of eclipse, need to update with satelite orbit
    //findEclipse(R_Sun_eci, RAAN, omega, i, e, r_p);
    double nu_entersEclipse = 1.911224; //radians, determined by funtion above
    double nu_leavesEclipse = 4.261047; //radians, determined by function above
    
    //full orbits
    int fullOrbits = floor((t1 - t0)/T); //number of complete orbits
    timeInSun += (T - (timeSincePeriapse(e, a, nu_entersEclipse) - timeSincePeriapse(e, a, nu_leavesEclipse)))*fullOrbits;
    
    //partial orbit
    double nu0 = trueAnomaly(e, a, t0);
    double nu1 = trueAnomaly(e, a, t1);
    
    if(nu0<=nu_entersEclipse)
    {
        if(nu1<=nu_entersEclipse && nu0<nu1)
        {
            timeInSun += timeSincePeriapse(e,a,nu1) - timeSincePeriapse(e,a,nu0);
            //printf("time in sun (blah) %lf\n", timeInSun);
        }
        else if(nu1<=nu_entersEclipse)
        {
            timeInSun += T - (timeSincePeriapse(e, a, nu_entersEclipse) - timeSincePeriapse(e, a, nu_leavesEclipse)) - (timeSincePeriapse(e,a,nu0) - timeSincePeriapse(e,a,nu1));
        }
        else if(nu_entersEclipse<nu1 && nu1<=nu_leavesEclipse)
        {
            timeInSun += timeSincePeriapse(e, a, nu_entersEclipse) - timeSincePeriapse(e,a,nu0);
        }
        else if(nu_leavesEclipse<nu1)
        {
            timeInSun += timeSincePeriapse(e, a, nu_entersEclipse) - timeSincePeriapse(e,a,nu0) + timeSincePeriapse(e,a,nu1) - timeSincePeriapse(e, a, nu_leavesEclipse);
        }
    }
    else if(nu_entersEclipse<nu0 && nu0<=nu_leavesEclipse)
    {
        if(nu_entersEclipse<=nu1 && nu1<=nu_leavesEclipse)
        {
            //doesn't add anything, starts and ends in eclipse
        }
        else if(nu1<nu_entersEclipse)
        {
            timeInSun += T-timeSincePeriapse(e, a, nu_leavesEclipse) + timeSincePeriapse(e,a,nu1);
        }
        else if(nu_leavesEclipse<nu1)
        {
            timeInSun += timeSincePeriapse(e,a,nu1) - timeSincePeriapse(e, a, nu_leavesEclipse);
        }
    }
    else if(nu_leavesEclipse<nu0)
    {
        if(nu1<=nu_entersEclipse)
        {
            timeInSun += T-timeSincePeriapse(e, a, nu0) + timeSincePeriapse(e,a,nu1);
        }
        else if(nu_entersEclipse<nu1 && nu1<=nu_leavesEclipse)
        {
            timeInSun += T-timeSincePeriapse(e, a, nu0) + timeSincePeriapse(e, a, nu_entersEclipse);
        }
        else if(nu_leavesEclipse<nu1 && nu0<=nu1)
        {
            timeInSun += timeSincePeriapse(e,a,nu1) - timeSincePeriapse(e,a,nu0);
        }
        else if(nu_leavesEclipse<nu1)
        {
            timeInSun += T - (timeSincePeriapse(e, a, nu_entersEclipse) - timeSincePeriapse(e, a, nu_leavesEclipse)) - (timeSincePeriapse(e,a,nu0) - timeSincePeriapse(e,a,nu1));
        }
    }
    
    return timeInSun;
}

/*  Function findEclipse
    Prints orbit values near the conditions S>0 and phi>90. Both must be true to be in eclipse
    Use these values to update other helper functions for a specific orbit
*/
void findEclipse(double * R_Sun_eci, double RAAN, double omega, double i, double e, double r_p){
    //two conditions for eclipse for a 3d orbit: S>0 and phi>90 degrees (both must be true)
    //this function prints all orbit values near these conditions
    //double pi = 2 * acos(0.0);
    double R_Sun_mag = 1.496*pow(10,8);
    double R_E = 6378; //km
    //calculate time in eclipse
    double l1 = cos(RAAN)*cos(omega) - sin(RAAN)*sin(omega)*cos(i);
    double l2 = -cos(RAAN)*sin(omega) - sin(RAAN)*cos(omega)*cos(i);
    double m1 = sin(RAAN)*cos(omega) + cos(RAAN)*sin(omega)*cos(i);
    double m2 = -sin(RAAN)*sin(omega) + cos(RAAN)*cos(omega)*cos(i);
    double n1 = sin(omega)*sin(i);
    double n2 = cos(omega)*sin(i);
    double alpha = (l1*R_Sun_eci[0]+m1*R_Sun_eci[1]+n1*R_Sun_eci[2])/R_Sun_mag;
    double beta = (l2*R_Sun_eci[0]+m2*R_Sun_eci[1]+n2*R_Sun_eci[2])/R_Sun_mag;
    double S;
    double phi;
    
    for(double nu=0; nu<=2*M_PI; nu += M_PI/180/100000)
    {
        phi = acos(alpha*cos(nu)+beta*sin(nu)); //radians
        S = pow(R_E,2) * pow((1+e*cos(nu)),2) + pow(r_p,2) * pow(alpha*cos(nu)+beta*sin(nu),2) - pow(r_p,2);
        if(abs(S)<10 || (M_PI/2 - 0.000001<phi && phi< M_PI/2 + 0.000001))
        {
            printf("nu=%lf, S=%lf, phi=%lf\n", nu, S, phi *180/M_PI);
        }
    }
}

/*  Function trueAnomaly
    Helper function that takes in orbital elements and time and returns the angle the satellite is from perigee
*/
double trueAnomaly(double e, double a, double t){
    double mu = 3.98601 * pow(10,5); //km^3/s^2
    double T = 2 * M_PI * sqrt(pow(a,3)/mu);
    double M = sqrt(mu/pow(a,3)) * (t - floor(t/T)*T);
    
    double oldE;
    double E = M;
    do 
    {
        oldE = E;
        E = oldE - (oldE-e*sin(oldE)-M)/(1-e*cos(oldE));
    }
    while (abs(E - oldE)>pow(10,-4));
    
    double nu = 2 * atan2(sqrt((1 + e) / (1 - e)) * tan(E / 2),1);
    if(nu<0)
        nu+=2*M_PI;
    if(nu>T)
        nu = fmod(nu, T); //remainder
    return nu;
}

/*  Function timeSincePeriapse
    Takes in orbital elements and the angle of a satellite from perigee and returns the time since the last periapse
*/
double timeSincePeriapse(double e, double a, double nu){
    double mu = 3.98601 * pow(10,5); //km^3/s^2
    double T = 2 * M_PI * sqrt(pow(a,3)/mu);
    double E = 2*atan2(tan(nu/2),sqrt((1+e)/(1-e)));
    double M = E-e*sin(E);
    double t = M*sqrt(pow(a,3)/mu);
    if(t<0)
        t+=T;
    if(t>T)
        t = fmod(t,T);
    return t;
}

/*  Function nextTrueAnomaly
    Helper function that updates a task's releaseTime and deadline by propagating to the next matching true anomaly
*/
void nextTrueAnomaly(double * releaseTime, double * deadline){
    //printf("test prev release = %lf, prev deadline = %lf\n", releaseTime[0], deadline[0]);
    double R_E = 6378; //km
    double mu = 3.98601 * pow(10,5); //km^3/s^2
    double r_p = R_E + 488;  //km
    double r_a = R_E + 509;  //km
    //double i = M_PI/180 * 97.4; //radians
    
    //orbit calculations
    double a = (r_a + r_p) / 2.; //km
    double T = 2 * M_PI * sqrt(pow(a,3)/mu); //orbit period, seconds
    
    if(releaseTime[0] != 0.0)
        releaseTime[0] = releaseTime[0] + T/60;
    if(deadline[0] != 0.0)
        deadline[0] = deadline[0] + T/60;

    //printf("test new release = %lf, new deadline = %lf\n", releaseTime[0], deadline[0]);
}

void peri2Equatorial(double * r_equatorial, double * r_perifocal, double RAAN, double omega, double i){
    r_equatorial[0] = (cos(RAAN)*cos(omega)-sin(RAAN)*cos(i)*sin(omega))*r_perifocal[0]
                    + (-cos(RAAN)*sin(omega)-sin(RAAN)*cos(i)*cos(omega))*r_perifocal[1]
                    + (sin(RAAN)*sin(i))*r_perifocal[2];
    r_equatorial[1] = (sin(RAAN)*cos(omega)+cos(RAAN)*cos(i)*sin(omega))*r_perifocal[0]
                    + (-sin(RAAN)*sin(omega)+cos(RAAN)*cos(i)*cos(omega))*r_perifocal[1]
                    + (-cos(RAAN)*sin(i))*r_perifocal[2];
    r_equatorial[2] = (sin(i)*sin(omega))*r_perifocal[0]
                    + (sin(i)*cos(omega))*r_perifocal[1]
                    + (cos(i))*r_perifocal[2];
}

void equatorial2Peri(double * r_perifocal, double * r_equatorial, double RAAN, double omega, double i){
    r_perifocal[0] = (cos(RAAN)*cos(omega)-sin(RAAN)*cos(i)*sin(omega))*r_equatorial[0]
                    +(sin(RAAN)*cos(omega)+cos(RAAN)*cos(i)*sin(omega))*r_equatorial[1]
                    +(sin(i)*sin(omega))*r_equatorial[2];
    r_perifocal[1] = (-cos(RAAN)*sin(omega)-sin(RAAN)*cos(i)*cos(omega))*r_perifocal[0]
                    +(-sin(RAAN)*sin(omega)+cos(RAAN)*cos(i)*cos(omega))*r_perifocal[1]
                    +(sin(i)*cos(omega))*r_perifocal[2];
    r_perifocal[2] = (sin(RAAN)*sin(i))*r_perifocal[0]
                    +(-cos(RAAN)*sin(i))*r_perifocal[1]
                    +(cos(i))*r_perifocal[2];
}







