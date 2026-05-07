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

/*  Function main
    Initializes, runs, and logs scheduling simulation using the GERRY algorithm and iterative repair
    GERRY algorithm: https://www.researchgate.net/publication/3114264_Scheduling_and_Rescheduling_with_Iterative_Repair
    Assumptions: 
        Tasks are not preemptive and cannot be parallelized
        Task failures are not caused by permanent hardware or software failure.
        A valid initial schedule is provided
        Safety-critical functions are independent of GERRY, so science task failure will not result in mission failure
        The domain S represents a closed world
*/

int main()
{
    //tracks cpu time over one test case
    clock_t start, end;
    double cpu_time_used;
    start = clock();

    // user defined data
    int casenum = 1; //see initializeDomain.c for case descriptions
    double failureChance = 0.01; // 1% chance of failure for each task
    int timelineLength = 400;
    
    // initialize rng, log, tasks, schedule, and simulation data
    //srand(time(NULL));
    srand(1); //set rng for consistant testing
    initializeLog();
    struct domain Domain = initializeDomain(casenum, timelineLength);
    struct schedule * headS = initializeSchedule(Domain);
    struct schedule * currentS = headS;
    struct simData data = {0};
    
    printf("Initial ");
    printSchedule(headS);
    double simTime = 0.0;
    
    logTaskConstraints(headS);
    
    //optional timeline printing, before repair
    printResourceTimelines(Domain);
    //printStateTimelines(Domain);
    
    int iter=0;
    int maxIter = 1000; //need to increase once working fully
    // run monte carlo simulation and iterative repair until all tasks are complete
    while(currentS != NULL && iter < maxIter)
    {
        currentS = iterativeRepair(currentS, headS, &Domain, &data, failureChance);
        simTime += currentS->executionTime;
        currentS = currentS->next;
        
        iter +=1;
    }
    // log and print final results
    logSimData(data);
    printf("Final ");
    printSchedule(headS);
    
    //optional timeline printing, after repair
    //printResourceTimelines(Domain);
    //printStateTimelines(Domain);
    
    logTaskConstraints(headS);
    
    freeSchedule(headS);
    freeDomain(&Domain);
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("CPU time used: %f seconds\n", cpu_time_used);
    
    return data.errorCode;
    return 0;
}
