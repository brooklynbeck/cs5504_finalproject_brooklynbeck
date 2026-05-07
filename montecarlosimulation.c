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

/*  Function mcs
    
    Determines if a scheduled task fails and changes the execution time of a scheduled task in a monte carlo simulation
    
    Parameters 
        S - specific task to be simulated
        failureChance - chance of task failure
    
    Returns the schedule struct with updated execution time
*/
struct schedule * mcs(struct schedule * S, double failureChance)
{
    //ignore any tasks with a standard deviation of 100, flag for unalterable task (ex "wait" or "solar recharge")
    if(S->T.sdExecutionTime ==100) //strcmp(S->T.name, "Wait") == 0 || strcmp(S->T.name, "Solar Recharge") == 0)
        return S;
    // failure check uses flat distribution
    double checkFailure = (double)rand() / (double)RAND_MAX;
    if(checkFailure > 1-failureChance)
        S->executionTime = -1.00;
    // execution time uses a gaussian distribution
    else
        S->executionTime = gaussianDistribution(S->T.avExecutionTime, S->T.sdExecutionTime);
    
    //adjust actual resource use by +/- 10%
    double resourceRange = 0.1;
    double randomResourceAdjustment[3] = {0,0,0};
    for (int i = 0; i < S->T.numRR; i++)
    {
        randomResourceAdjustment[0] = (rand() / (double)RAND_MAX) * (2*resourceRange) - resourceRange;
        //randomResourceAdjustment[1] = (rand() / (double)RAND_MAX) * (2*resourceRange) - resourceRange;
        //randomResourceAdjustment[2] = (rand() / (double)RAND_MAX) * (2*resourceRange) - resourceRange;
        S->T.RR[i].preConstraint += S->T.RR[i].preConstraint * randomResourceAdjustment[0]; //remains zero if preConstraint = 0
        //S->T.RR[i].maintainConstraint += S->T.RR[i].maintainConstraint * randomResourceAdjustment[1];
        S->T.RR[i].preImpact += S->T.RR[i].preImpact * randomResourceAdjustment[0];
        //S->T.RR[i].maintainImpact += S->T.RR[i].maintainImpact * randomResourceAdjustment[1];
    }
    return S;
}

/*  Function gaussianDistribution
    
    Generates a random number on a gaussian distribution using the Box–Muller transform
    Parameters: mean and standard deviation of desired distribution
    Returns a random number
*/
double gaussianDistribution(double mean, double stddev)
{
    if (stddev == 100) //standard value for unalterable value
        return mean;
    double u1 = (double)rand() / RAND_MAX;
    double u2 = (double)rand() / RAND_MAX;
    double z = sqrt(-2 * log(u1)) * cos(2 * M_PI * u2);
    double gaussianNum = mean + stddev * z;
    return gaussianNum;
}

/* Function targetVector
    
    Generates a random 3D vector using preallocated memory (vector)
*/
void targetVector(double * vector)
{
    vector[0] = ((double)rand() *2 / RAND_MAX) - 1.0;
    vector[1] = ((double)rand() *2 / RAND_MAX) - 1.0;
    vector[2] = ((double)rand() *2 / RAND_MAX) - 1.0; //assume can be in different hemispheres
    //vector[2] = ((double)rand()/ RAND_MAX); //assume all in same hemisphere
    //double magnitude = sqrt(vector[0]*vector[0] + vector[1]*vector[1] +vector[2]*vector[2]);
}



