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

/*  Function initializeSchedule

    Generates a linked list of scheduled tasks from a preordered array of tasks. Updates the resource and state variable timelines
    
    Parameters:
        T - domain, including tasks, resources, and state variables
    
    Returns the head of a linked list of schedule structs
*/
struct schedule * initializeSchedule(struct domain T)
{
    double simLength = 0.0;
    struct schedule * headS = createNode();
    
    int start = 0;
    headS->T = T.T[start];
    
    headS->executionTime = T.T[start].avExecutionTime;
    headS->startTime = simLength;
    //link task back to schedule
    T.T[start].S = headS;
    simLength += headS->executionTime;
    struct schedule * prevS = headS;
    struct schedule * temp;
    //int tempStateValue;
    
    for (int j=0; j<T.T[start].numRR; j++)
    {
        if((T.T[start].RR[j].preImpact != 0.0) ||(T.T[start].RR[j].maintainImpact != 0.0))
            updateResourceTimeline(T.T[start].RR[j].R, headS->startTime, T.T[start].RR[j].preImpact, T.T[start].RR[j].maintainImpact);
        if((T.T[start].RR[j].postImpact != 0.0)||(T.T[start].RR[j].maintainImpact != 0.0))
            updateResourceTimeline(T.T[start].RR[j].R, simLength, T.T[start].RR[j].postImpact, -1* T.T[start].RR[j].maintainImpact);
    }
    
    for(int j=0; j<T.T[start].numSR; j++)
    {
        if(T.T[start].SR[j].preImpact !=-1)
            updateStateTimeline(T.T[start].SR[j].SV, headS->startTime, T.T[start].SR[j].preImpact);
        if(T.T[start].SR[j].maintainImpact != -1)
            updateStateTimeline(T.T[start].SR[j].SV, headS->startTime, T.T[start].SR[j].maintainImpact);
        if(T.T[start].SR[j].postImpact != -1)
            updateStateTimeline(T.T[start].SR[j].SV, simLength, T.T[start].SR[j].postImpact);
    }
    
    // adds tasks in the order of the task array
    // updates resource and state timelines
    for(int i=start+1;i<T.numTasks;i++)
    {
        temp = createNode();
        temp->T = T.T[i];
        // initalize with work duration
        temp->executionTime = T.T[i].WD;
        if (simLength < T.T[i].WC->releasetime)
            simLength = T.T[i].WC->releasetime;
        //link task back to schedule
        T.T[i].S = temp;
        temp->startTime = simLength;
        simLength += temp->executionTime;
        temp->prev = prevS;
        prevS->next = temp;
        prevS = temp;
        
        for (int j=0; j<T.T[i].numRR; j++)
        {
            if((T.T[i].RR[j].preImpact != 0.0) ||(T.T[i].RR[j].maintainImpact != 0.0))
                updateResourceTimeline(T.T[i].RR[j].R, temp->startTime, T.T[i].RR[j].preImpact, T.T[i].RR[j].maintainImpact);
            if((T.T[i].RR[j].postImpact != 0.0)||(T.T[i].RR[j].maintainImpact != 0.0))
                updateResourceTimeline(T.T[i].RR[j].R, simLength, T.T[i].RR[j].postImpact, -1* T.T[i].RR[j].maintainImpact);
        }
        for(int j=0; j<T.T[i].numSR; j++)
        {
            if(T.T[i].SR[j].preImpact !=-1)
                updateStateTimeline(T.T[i].SR[j].SV, temp->startTime, T.T[i].SR[j].preImpact);
            if(T.T[i].SR[j].maintainImpact != -1)
                updateStateTimeline(T.T[i].SR[j].SV, temp->startTime, T.T[i].SR[j].maintainImpact);
            if(T.T[i].SR[j].postImpact != -1)
                updateStateTimeline(T.T[i].SR[j].SV, simLength, T.T[i].SR[j].postImpact);
        }
    }
    
    // record inital schedule in the log
    logSchedule(headS);
    return headS;
}

