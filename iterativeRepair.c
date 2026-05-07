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

/*  Function iterativeRepair
    https://www.researchgate.net/publication/3114264_Scheduling_and_Rescheduling_with_Iterative_Repair
    
    Algorithm: 
        1. Update the current task using a Monte Carlo simulation
        2. Search the schedule for conflicts
        3. While there are still conflicts,
            i. Select a conflict to repair, based on priority heuristic
            ii. Select a method of repairing the conflict
            iii. Repair the conflict
                a. move a task
                b. add a task
                c. delete a task
            iv. Search for new conflicts
        4. Repeat until all tasks have been simulated and repaired
    
    Parameters
        currentS - schedule struct at the current time
        headS - schedule struct at the start of the schedule
        currentDomain - domain and task information, as updated by the Monte Carlo simulation
        taskList - struct containing all possible tasks
        simData - struct holding metadata about the simulation, including number of task failures
        failureChance - chance of task failure
    
    Returns the current schedule struct, repaired according to the needs
*/
struct schedule * iterativeRepair(struct schedule * currentS, struct schedule *headS, struct domain *currentDomain, struct simData * data, double failureChance) //struct domain currentDomain)
{
    int iterations = 0;
    int maxIterations = 100;
    struct conflict* conflicts;
    struct conflict currentConflict;
    double *method = (double *) malloc(sizeof(double)*3);
    
    //update the domain using a Monte Carlo Simulation, and update the circular buffer accordingly
    updateDomain(failureChance, currentS);
    updateCircularBuffer(headS, currentDomain);
    //log the updated schedule
    logSchedule(headS);
    //formal GERRY Algorithm steps
    logUpdates(0);
    
    //search for all conflicts in schedule
    conflicts = getConflicts(currentS);
    logConflictList(conflicts);
    //calculate number of conflicts
    int lengthConflicts = countConflicts(conflicts);
    while (lengthConflicts>0 && iterations<maxIterations)
    {
        //choose a conflict based on priority info
        currentConflict = chooseConflict(conflicts);
        //printf("TEST repairing %s\n", currentConflict.T->T.name);
        logConflict(currentConflict);
        
        //choose a method based on the type of conflict
        chooseMethod(method, currentConflict, data, currentDomain);
        if(currentConflict.T->T.name == NULL)
            return currentS;
        //repair the conflict based on the method identified and log the repair
        
        switch((int)method[0])
        {
            case 0:
                printf("Error, no method chosen (Current Conflict %d)\n", currentConflict.conflictType);
                break;
            case 1:
                logRepair(currentConflict.T, (int)method[0]);
                move(currentConflict.T, method[1], currentDomain);
                break;
            case 2:
                add(currentConflict.T, (int)method[1], currentDomain, method[2]);
                logRepair(currentConflict.T, (int)method[0]);
                break;
            case 3:
                logRepair(currentConflict.T, (int)method[0]);
                delete(currentConflict.T, currentConflict.T->next);
                break;
            case 4:
                logRepair(currentConflict.T, (int)method[0]);
                aide();
                break;
        }
        
        logSchedule(headS);
        updateCircularBuffer(headS, currentDomain);
        free(conflicts);
        conflicts = NULL;
        
        //check for new conflicts, since more could have been generated in repair
        conflicts = getConflicts(currentS);
        lengthConflicts = countConflicts(conflicts);
        iterations +=1;
    }
    
    free(conflicts);
    conflicts = NULL;
    free(method);
    method = NULL;
    logUpdates(1);
    return currentS;
}

/*  Function updateDomain

    Parameters:
        failureChance - chance of task failure, user determined in main.c
        currentS - scheduled task to be updated
    
    Calls a gaussian Monte Carlo Simulation to generate simulated runtimes from provided distribution task information
    Logs the MCS results
*/
void updateDomain(double failureChance, struct schedule * currentS)
{
    currentS = mcs(currentS, failureChance);
    logRepair(currentS, 0);
}

/*  Function countConflicts
    
    Counts and returns the number of conflicts in a list of conflicts
*/
int countConflicts(struct conflict* conflictList)
{
    int i = 0;
    while (conflictList[i].conflictType != 0 && abs(conflictList[i].conflictType) <= 15)
    {
        i+=1;
    }
    return i;
}

//TBD TC constraints and state constraints
/*  Function getConflicts
    
    Searches a schedule from a given starting point (currentS) and creates a list of conflicts found, noting the type of conflict and the priority
    
    Types of conflicts:
        Task failure
        Task runs too long (halting problem, cannot predict when or if it will end)
        Task runs too long (ends after the next task's deadline to start)
        Task runs too long (ends after the next task is currently scheduled to start)
        Task completes early (ends before next task is released to run)
        Task completes early (ends before next task is currently scheduled to start)
        Temporal constraint violation: violates min time between tasks
        Violates the minimum value of a resource
        Violates the maximum value of a resource
        Violates a task's preConstraint of a resource
        Violates a task's maintainConstraint of a resource
        Violates a task's preConstraint state variable requirement
        Violates a task's maintainConstraint state variable requirement
*/
struct conflict* getConflicts(struct schedule * currentS)
{
    
    struct schedule * temp;
    struct schedule * search;
    int tempInt = 0;
    temp = currentS;
    int conflictCount = 0;
    struct conflict* conflictList;
    conflictList = (struct conflict *) malloc(sizeof(struct conflict) * (conflictCount));
    int searchIteration = 0;
    
    // loop until the final schedule struct, or for the scheduling horizon
    int schedulingHorizon = 5;
    int tasksSearched = 0;
    while(temp != NULL && tasksSearched < schedulingHorizon)
    {
        //Failure
        if(temp->executionTime < 0.0)
        {
            conflictCount+=1;
            conflictList = (struct conflict*)realloc(conflictList, sizeof(struct conflict)*conflictCount);
            conflictList[conflictCount-1].conflictType = 1;
            conflictList[conflictCount-1].priority = 4; //how should priority be handled?
            conflictList[conflictCount-1].T = temp;
        }
        //WD constraint: if a task has not stopped (need to truncate, halting problem)
        if(temp->executionTime > 1.5*temp->T.WD) //1.5 arbitrary, how much can I use standard deviation? simulation only??
        {
            conflictCount +=1;
            conflictList = (struct conflict*)realloc(conflictList, sizeof(struct conflict)*conflictCount);
            conflictList[conflictCount-1].conflictType = 2;
            conflictList[conflictCount-1].priority = 2;
            conflictList[conflictCount-1].T = temp;
        }
        //WD constraint: task ended after next task's deadline
        if((temp->next!=NULL)&&(temp->next->T.WC->deadline > 0)&&(temp->startTime+temp->executionTime > temp->next->T.WC->deadline))
        {
            conflictCount +=1;
            conflictList = (struct conflict*)realloc(conflictList, sizeof(struct conflict)*conflictCount);
            conflictList[conflictCount-1].conflictType = 3;
            conflictList[conflictCount-1].priority = 3;
            conflictList[conflictCount-1].T = temp->next;
        }
        //WC constraint: if a task completes before the next task is released
        if((temp->next!=NULL)&&(temp->executionTime + temp->startTime < temp->next->T.WC->releasetime))
        {
            conflictCount +=1;
            conflictList = (struct conflict*)realloc(conflictList, sizeof(struct conflict)*conflictCount);
            conflictList[conflictCount-1].conflictType = 4;
            conflictList[conflictCount-1].priority = 1;
            conflictList[conflictCount-1].T = temp;
        }
        
        //WC constraint: if a task completes before the next task starts
        if((temp->next!=NULL)&&(temp->executionTime + temp->startTime < temp->next->startTime))
        {
            conflictCount+=1;
            conflictList = (struct conflict*)realloc(conflictList, sizeof(struct conflict)*conflictCount);
            conflictList[conflictCount-1].conflictType = 5;
            conflictList[conflictCount-1].priority = 0;
            conflictList[conflictCount-1].T = temp->next;
        }
        //TC: assume non-preemptive, check if next task starts before current start completes
        if((temp->next!=NULL)&&(temp->next->startTime < temp->startTime + temp->executionTime))
        {
            conflictCount +=1;
            conflictList = (struct conflict*)realloc(conflictList, sizeof(struct conflict)*conflictCount);
            //if the next task is a wait task, delete instead of moving
            if(strcmp(temp->next->T.name, "Wait")==0)
            {
                conflictList[conflictCount-1].conflictType = 3;
                conflictList[conflictCount-1].priority = 1;
                conflictList[conflictCount-1].T = temp->next;
            }
            else
            {
                conflictList[conflictCount-1].conflictType = 6;
                conflictList[conflictCount-1].priority = 1;
                conflictList[conflictCount-1].T = temp->next;
            }
        }
        for(int i=0; i<temp->T.numTC; i++)
        {
            //manditory minimum wait between tasks
            if((temp->T.TC[i].minTimeAfter>0.0) && (temp->startTime < temp->T.TC[i].relativeTask->S->startTime + temp->T.TC[i].relativeTask->S->executionTime + temp->T.TC[i].minTimeAfter))
            {
                conflictCount +=1;
                conflictList = (struct conflict*)realloc(conflictList, sizeof(struct conflict)*conflictCount);
                conflictList[conflictCount-1].conflictType = 7;
                conflictList[conflictCount-1].priority = 1;
                conflictList[conflictCount-1].T = temp->prev;
                //printf("Test %s\n", conflictList[conflictCount-1].T->T.name);
            }
            
            //manditory order between tasks (relative task immediately before or after current task)
            if(temp->T.TC[i].relativePrev==1 && strcmp(temp->prev->T.name, temp->T.TC[i].relativeTask->name)!=0)
            {
                conflictCount +=1;
                conflictList = (struct conflict*)realloc(conflictList, sizeof(struct conflict)*conflictCount);
                conflictList[conflictCount-1].conflictType = 14;
                conflictList[conflictCount-1].priority = 3;
                conflictList[conflictCount-1].T = temp;
            }
            if(temp->T.TC[i].relativeNext==1 && strcmp(temp->next->T.name, temp->T.TC[i].relativeTask->name)!=0)
            {
                conflictCount +=1;
                conflictList = (struct conflict*)realloc(conflictList, sizeof(struct conflict)*conflictCount);
                conflictList[conflictCount-1].conflictType = 15;
                conflictList[conflictCount-1].priority = 3;
                conflictList[conflictCount-1].T = temp;
            }
        }
        
        for(int i=0; i<temp->T.numRR; i++)
        {
            //find timeline resource closest to current task timeline
            tempInt = temp->T.RR[i].R->timeline->tail;
            searchIteration = 0;
            while(temp->startTime + temp->executionTime > temp->T.RR[i].R->timeline->buffer[tempInt].timestamp &&(searchIteration <100))
            {
                searchIteration +=1;
                if(tempInt >= temp->T.RR[i].R->timeline->maxlen)
                    tempInt=0;
                else
                    tempInt+=1;
            }
            //RC: Check if violates global min/max value
            if(temp->T.RR[i].R->globalMin > temp->T.RR[i].R->timeline->buffer[tempInt].currentValue)
            {
                conflictCount +=1;
                conflictList = (struct conflict*)realloc(conflictList, sizeof(struct conflict)*conflictCount);
                conflictList[conflictCount-1].conflictType = 8;
                conflictList[conflictCount-1].resourceAffected = i;
                conflictList[conflictCount-1].priority = 3;
                conflictList[conflictCount-1].T = temp->prev;
            }
            if(temp->T.RR[i].R->globalMax < temp->T.RR[i].R->timeline->buffer[tempInt].currentValue)
            {
                conflictCount +=1;
                conflictList = (struct conflict*)realloc(conflictList, sizeof(struct conflict)*conflictCount);
                conflictList[conflictCount-1].conflictType = 9;
                conflictList[conflictCount-1].resourceAffected = i;
                conflictList[conflictCount-1].priority = 3;
                conflictList[conflictCount-1].T = temp->prev;
            }
            
            //RC: Check if preConstraints are met
            if((temp->T.RR[i].preConstraint!=0) && (temp->T.RR[i].preConstraint > temp->T.RR[i].R->timeline->buffer[tempInt].currentValue))
            {
                conflictCount +=1;
                conflictList = (struct conflict*)realloc(conflictList, sizeof(struct conflict)*conflictCount);
                conflictList[conflictCount-1].conflictType = 10;
                conflictList[conflictCount-1].resourceAffected = i;
                conflictList[conflictCount-1].priority = 2;
                conflictList[conflictCount-1].T = temp;
            }
            
            //RC: Check if maintainConstraints are met
            //update to search over whole interval
            if((temp->T.RR[i].maintainConstraint!=0)&&(temp->T.RR[i].maintainConstraint > temp->T.RR[i].R->timeline->buffer[tempInt].currentRate))
            {
                conflictCount +=1;
                conflictList = (struct conflict*)realloc(conflictList, sizeof(struct conflict)*conflictCount);
                conflictList[conflictCount-1].conflictType = 11;
                conflictList[conflictCount-1].resourceAffected = i;
                conflictList[conflictCount-1].priority = 2;
                conflictList[conflictCount-1].T = temp;
            }
        }
        for(int i=0; i<temp->T.numSR; i++)
        {
            //find previous state in the same state variable
            tempInt = -1;
            search = temp->prev;
            while(search != NULL){
                for (int j=0; j<search->T.numSR; j++)
                {
                    if(search->T.SR[j].SV == temp->T.SR[i].SV) //state variable found
                    {
                        tempInt = search->T.SR[j].postImpact; 
                        if (tempInt == -1)
                            tempInt = search->T.SR[j].preImpact;
                        break;
                    }
                }
                if(tempInt != -1) //stop searching if a state has been found
                    break;
                search = search->prev;
            }
            if(search = NULL) //if no state matches were found, use the initial state
                tempInt = temp->T.SR[i].SV->initialState;
            
            if((abs(temp->T.SR[i].preConstraint) < 100) && (temp->T.SR[i].preConstraint != -1) && temp->T.SR[i].preConstraint != tempInt)
            {
                //printf("want state %d, currently in state %d at time %lf\n", temp->T.SR[i].preConstraint, tempInt, temp->startTime);
                conflictCount +=1;
                conflictList = (struct conflict*)realloc(conflictList, sizeof(struct conflict)*conflictCount);
                conflictList[conflictCount-1].conflictType = 12;
                conflictList[conflictCount-1].stateVariableAffected = i;
                conflictList[conflictCount-1].priority = 2;
                conflictList[conflictCount-1].T = temp;
            }
            //SR: Check if maintainConstraints maintained
            //Note: not currently checking for maintain constraints
            if((abs(temp->T.SR[i].maintainConstraint) < 100) && (temp->T.SR[i].maintainConstraint != -1) && temp->T.SR[i].maintainConstraint != temp->T.SR[i].SV->timeline->buffer[tempInt].currentValue)
            {
                conflictCount +=1;
                conflictList = (struct conflict*)realloc(conflictList, sizeof(struct conflict)*conflictCount);
                conflictList[conflictCount-1].conflictType = 13;
                conflictList[conflictCount-1].stateVariableAffected = i;
                conflictList[conflictCount-1].priority = 2;
                conflictList[conflictCount-1].T = temp;
            }
        }
        temp = temp->next;
        tasksSearched += 1;
    }
    return conflictList;
}

/*  Function printConflictList
    
    Prints a list of conflicts, including the name of the task experiencing a constraint violation and the type of conflict
*/
void printConflictList(struct conflict* conflicts)
{
    int i = 0;
    int maxConflictList = 10;
    printf("Conflict List:\n");
    while (conflicts[i].conflictType != 0 && abs(conflicts[i].conflictType) <= 15 && i<maxConflictList)
    {
        printf("%d. Conflict for %s: type %d\n", i+1, conflicts[i].T->T.name, conflicts[i].conflictType);
        i+=1;
    }
}

/*  Function freeConflicts
    
    Frees a list of conflicts from the memory
*/
void freeConflicts(struct conflict* conflicts)
{
    free(conflicts);
    conflicts = NULL;
}

/*  Function chooseConflict
    
    Selects the conflict from a list of conflicts according to the following heuristics:
        1. Highest priority
        2. Nearest startTime
*/
struct conflict chooseConflict(struct conflict* conflicts)
{
    int maxPriorityConflict = 0;
    int numConflicts = countConflicts(conflicts);
    for(int i=1;i<numConflicts;i++)
    {
        if((abs(conflicts[i].conflictType) <= 30)&&(conflicts[i].priority>conflicts[maxPriorityConflict].priority))
            maxPriorityConflict = i;
    }
    return conflicts[maxPriorityConflict];
}

/*  Function chooseMethod
    
    Selects a method of repair and organizes needed task information for easy repair
    
    Parameters:
        methodInfo - memory allocated tuple, used to indicate (repair method, additional repair information)
        currentConflict - conflict selected for repair
        data - simulation information, including number of task repairs done
    
    Repair methods:
        1. Move (additional information indicates new startTime)
        2. Add (additional information indicates type of new task added)
        3. Delete (no additional information needed)
*/
void chooseMethod(double *methodInfo, struct conflict currentConflict, struct simData * data, struct domain *currentDomain)
{
    methodInfo[0] = 0;
    methodInfo[1] = 0;
    methodInfo[2] = 0;
    //printf("TESTING CASE %d\n", currentConflict.conflictType);
    switch(currentConflict.conflictType)
    {
        case 0:
            //unused, indicates code error
            aide();
            break;
        case 1:
            //case of task failure
            //(2,-1) indicates adding a repeated task
            methodInfo[0] = 2;
            methodInfo[1] = -1;
            data->numFail+=1;
            break;
        case 2:
            //case of task requiring truncation (halting problem)
            //(2,-1) indicates adding a repeated task
            methodInfo[0] = 2;
            methodInfo[1] = -1;
            data->numLate +=1;
            break;
        case 3:
            //case of task ending after next task's deadline
            //(3,0) indicates deleting the next task
            methodInfo[0] = 3;
            data->numFail =1; //task failed, need to reschedule
            break;
        case 4:
            //case of task completing before next task is released
            //(2,0) indicates adding a wait task after the currentConflict
            methodInfo[0] = 2;
            methodInfo[1] = 0;
            data->numEarly +=1;
            break;
        case 5:
            //case of task completing before next task starts (can move forward)
            //(1,newStartTime) indicates moving the next task to a new startTime
            methodInfo[0] = 1;
            methodInfo[1] = currentConflict.T->prev->startTime + currentConflict.T->prev->executionTime;
            data->numEarly +=1;
            break;
        case 6:
            //case of task completing after next task starts (can be moved back)
            //(1,newStartTime) indicates moving the next task to a new time
            methodInfo[0] = 1;
            methodInfo[1] = currentConflict.T->prev->startTime + currentConflict.T->prev->executionTime;
            data->numLate +=1;
            break;
        case 7:
            //printf("test\n");
            //case minimum distance between tasks not kept
            //(2,0) indicates adding a wait task
            int i = 0; //currently supports only one tc constraint per task, need to update TBD
            methodInfo[0] = 2;
            methodInfo[1] = 0;
            methodInfo[2] = currentConflict.T->next->T.TC[i].minTimeAfter + 0.0001;
            break;
        case 14:
            //case of relative task (prev)
            //(1,-1) indicates moving task forward relatively to temporally connected task
            methodInfo[0] = 1;
            methodInfo[1] = -1;
            break;
        case 15:
            //case of relative task (next)
            //(1,-1) indicates moving task forward relatively to temporally connected task
            methodInfo[0] = 1;
            methodInfo[1] = -1;
            break;
        case 8:
            //case of violating resource global min
            //(2,resourceID) adds a task that changes the resource appropriately
            //printf("resource %d\n", currentConflict.resourceAffected);
            methodInfo[0] = 2;
            if(currentConflict.resourceAffected == 0)
                methodInfo[1] = 2; //note: currently only energy is implemented
            else
                methodInfo[1] = -2; //will request aide, not able to handle other resources now
            data->numResourceViolation +=1;
            break;
        case 9:
            //case of violating resource global maxPriorityConflict
            //note: currently no maximums are supported
            //(2,resourceID) adds a task that changes the resource appropriately
            methodInfo[0] = 2;
            if(currentConflict.resourceAffected == -1)
                methodInfo[1] = 2; 
            else
                methodInfo[1] = -2; //will request aide
            data->numResourceViolation +=1;
            break;
        case 10:
            //case of violating resource preConstraint
            //(2,resourceID) adds a task to update the resource before the current task
            methodInfo[0] = 2;
            if(currentConflict.resourceAffected == 0)
                methodInfo[1] = 2; //note: currently only energy is implemented
            else
                methodInfo[1] = -2; //will request aide
            data->numResourceViolation +=1;
            break;
        case 11:
            //case of violating resource maintainConstraint
            //(2,resourceID) adds a task to update the resource before the current task
            methodInfo[0] = 2;
            if(currentConflict.resourceAffected == 0)
                methodInfo[1] = 2; //note: currently only energy is implemented
            else
                methodInfo[1] = -2; //will request aide
            data->numResourceViolation +=1;
            break;
        case 12:
            //case of violating state preConstraint
            //(2,templateID) adds a task to update the state variable before the current task
            methodInfo[0] = 2;
            if(currentConflict.stateVariableAffected == -1) //currently no states supported
                methodInfo[1] = 1; 
            else
                methodInfo[1] = -2; //will request aide
            data->numStateViolation +=1;
            break;
        case 13:
            //case of violating state maintainConstraint
            //(2,templateID) adds a task to update the state variable before the current task
            methodInfo[0] = 2;
            if(currentConflict.stateVariableAffected == -1) //currently no states supported
                methodInfo[1] = 1; 
            else
                methodInfo[1] = -2; //will request aide
            data->numStateViolation +=1;
            break;
    }
}

/*  Function move
    
    Repairs by moving a task on the timeline
    Special rules are provided for wait and solar recharge tasks
*/
void move(struct schedule * toMove, double newTime, struct domain *currentDomain){
        if (newTime < 0) //change relative place in the schedule, currently just using for temporal constraints
        {
            struct schedule * relTask = toMove->T.TC->relativeTask->S;
            
            if(toMove->T.TC->relativeNext == 1) //move "toMove" forward to be just before "relTask"
            {
                toMove->startTime = relTask->startTime - toMove->executionTime;
                
                toMove->prev->next = toMove->next;
                toMove->next->prev = toMove->prev;
                
                toMove->prev = relTask->prev;
                relTask->prev->next = toMove;
                toMove->next = relTask;
                relTask->prev = toMove;
            }
            else //move "toMove" forward to be just after "relTask"
            {
                toMove->startTime = relTask->startTime + relTask->executionTime;
            
                toMove->prev->next = toMove->next;
                toMove->next->prev = toMove->prev;
                
                toMove->prev = relTask;
                toMove->next = relTask->next;
                relTask->next->prev = toMove;
                relTask->next = toMove;
            }
            
        }
        else
        {
            toMove->startTime = newTime;
            //check if moved task is a wait task, which would need to extend execution time
            if(strcmp(toMove->T.name,"Wait")==0)
            {
                //slightly different proceedure for solar charge
                if(strcmp(toMove->next->T.name, "Solar Recharge")==0)
                {
                    toMove->executionTime = toMove->next->next->T.WC->releasetime - toMove->startTime - toMove->next->executionTime;
                    if(toMove->executionTime <0)
                    {
                        toMove->executionTime = 0;
                    }
                }
                else
                    toMove->executionTime = toMove->next->startTime - newTime;
            }
        }
        
}

/*  Function add
    Repairs by adding a task to the schedule, indicated by the variable newTaskIndicator
    
    Potential added tasks:
        Repeat previous task
        Wait task
        Request Aide task
        Solar Recharge task - uses helper function solarRechargeTimeFromEnergy
*/
void add(struct schedule * toAddAfter, int newTaskIndicator, struct domain *currentDomain, double waitTime)
{
    //struct schedule * temp = createNode();
    //repeat task
    if(newTaskIndicator==-1) 
    {
        //reset failed task
        toAddAfter->executionTime = toAddAfter->T.WD;
        //add repeat
        toAddAfter = insertNext(toAddAfter,toAddAfter->T); 
        
        if(toAddAfter->T.numTC != 0)
        {
            for(int i=0; i<toAddAfter->T.numTC; i++)
            {
                if(toAddAfter->T.TC[i].relativeNext == 1)
                {
                    toAddAfter->next->T.TC[i].relativeNext = toAddAfter->T.TC[i].relativeNext;
                    toAddAfter->next->T.TC[i].relativeTask = toAddAfter->T.TC[i].relativeTask;
                    toAddAfter->T.TC[i].relativeNext = 0;
                    toAddAfter->T.TC[i].relativeTask = NULL;
                    //toAddAfter->T.TC[i].relativeTask->TC[0].relativeTask = toAddAfter->next->T;
                }
            }
            
        }
        /*if(toAddAfter->prev->T.numRR != 0)
        {
            free(toAddAfter->T.RR);
            toAddAfter->T.numRR = 0;
        }
        if(toAddAfter->T.numSR != 0)
        {
            free(toAddAfter->T.SR);
            toAddAfter->T.numSR = 0;
        }
        if(toAddAfter->T.numTC != 0)
        {
            free(toAddAfter->T.TC);
            toAddAfter->T.numTC = 0;
        }*/
        //printf("TEST %s should have no SR (check %d)\n",toAddAfter->T.name, toAddAfter->T.numSR);
    }
    //wait task
    else if(newTaskIndicator==0)
    {
        //extend next wait task rather than adding a new one
        if(strcmp(toAddAfter->next->T.name, currentDomain->templateTasks[0].name)==0)
        {
            toAddAfter->next->startTime = toAddAfter->startTime + toAddAfter->executionTime;
            toAddAfter->next->executionTime = toAddAfter->next->next->startTime - toAddAfter->next->startTime;
        }
        //extend current wait task rather than adding a new one
        if(strcmp(toAddAfter->T.name, currentDomain->templateTasks[0].name)==0)
            toAddAfter->executionTime = toAddAfter->next->startTime - toAddAfter->startTime;
        else
        {
            struct schedule * temp=createNode();
            temp->T=currentDomain->templateTasks[0];
            temp->startTime = toAddAfter->startTime + toAddAfter->executionTime;
            if(waitTime!= 0){
                temp->executionTime = waitTime;
                toAddAfter->next->startTime += waitTime;
            }
            else
                temp->executionTime =  toAddAfter->next->T.WC->releasetime - temp->startTime;
            temp->prev=toAddAfter;
            toAddAfter->next->prev = temp;
            temp->next = toAddAfter->next;
            toAddAfter->next=temp;
        }
    }
    //task to increase power avalible
    //broke something, keep working on it
    else if(newTaskIndicator==2)
    {
        double energyReq;
        double timeReq;
        //printf("adding solar recharge (prev) %s (next) %s\n", toAddAfter->T.name, toAddAfter->next->T.name);
        energyReq = -1* (toAddAfter->next->T.RR[0].preImpact 
                        + toAddAfter->next->T.RR[0].maintainImpact/60 
                        + toAddAfter->next->T.RR[0].postImpact);
        /*if (toAddAfter->next->next != NULL && toAddAfter->next->next->T.numRR != 0)
        {
            energyReq += -1 * (toAddAfter->next->next->T.RR[0].preImpact 
                        + toAddAfter->next->next->T.RR[0].maintainImpact/60 
                        + toAddAfter->next->next->T.RR[0].postImpact);
        }*/
        timeReq = solarRechargeTimeFromEnergy(toAddAfter->startTime + toAddAfter->executionTime, energyReq);
        //if previous task is a wait task, reduce executionTime of the wait task
        if(strcmp(toAddAfter->T.name, currentDomain->templateTasks[0].name)==0)
        {
            toAddAfter->executionTime = toAddAfter->next->T.WC->releasetime - toAddAfter->startTime - timeReq;
            if(toAddAfter->executionTime <0)
                toAddAfter->executionTime = 0;
        }
        
        struct schedule * temp=createNode();
        temp->T=currentDomain->templateTasks[2];
        temp->startTime = toAddAfter->startTime + toAddAfter->executionTime;
        temp->executionTime =  timeReq;
        temp->T.RR[0].preImpact = energyReq;
        temp->T.RR[0].maintainImpact = 0;
        temp->prev=toAddAfter;
        toAddAfter->next->prev = temp;
        temp->next = toAddAfter->next;
        toAddAfter->next=temp;
        if (temp->startTime == temp->next->startTime){
            temp->next->startTime += 0.01;
        }
        //printResourceTimeline(currentDomain->resources[0]);
        //exit(1);
    }
    //task to request scheduling aide
    else
    {
        aide();
    }
}

/*  Function delete
    Repairs by deleting a task (or rescheduling it to the next true anomaly, if it is tagged as movable)
*/
void delete(struct schedule *toRemove, struct schedule *nextTask)
{
    if (toRemove->next->T.name == NULL)
    {
        toRemove->prev->next = NULL;
        free(toRemove);
    }
    else
    {
        //check if can instead move to next true anomaly
        if((toRemove->T.movableCheck == 1) && (toRemove->T.WC->deadline != 0.0 || toRemove->T.WC->releasetime!=0.0))
        {
            //update startTime and deadline
            nextTrueAnomaly(&toRemove->T.WC->releasetime, &toRemove->T.WC->deadline);
            toRemove->startTime = toRemove->T.WC->releasetime;
            struct schedule * temp;
            temp = toRemove->next;
            //get to next place in the schedule (or end of schedule)
            while(temp->next->T.name != NULL && (temp->startTime <= toRemove->T.WC->releasetime && temp->startTime + temp->executionTime <= toRemove->T.WC->deadline))
                temp = temp->next;
            toRemove->prev->next = nextTask;
            nextTask->prev = toRemove->prev;
            
            toRemove->next = temp->next;
            temp->next=toRemove;
            toRemove->prev=temp->prev->next;
        }
        else
        {
            toRemove->prev->next = nextTask;
            nextTask->prev = toRemove->prev;
            free(toRemove);
        }
    }
}

void aide(){
    printf("Requesting scheduling aide from ground, check log for more info, stopping rescheduling until new initial schedule recieved\n");
    exit(1);
}




