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

/*  Function initializeLog
    Clears previous log and records time of simulation
*/
void initializeLog()
{
    FILE *Log;
    Log = fopen("log.txt", "w");
    fprintf(Log, "GERRY Iterative Repair Log\n");
    time_t now = time(NULL);
    fprintf(Log, "%s", asctime(localtime(&now)));
    fprintf(Log, "Initial ");
    fclose(Log);
    
    FILE *Constraints;
    Constraints = fopen("constraints.txt", "w");
    fprintf(Constraints, "Initial Constraints\n");
    fclose(Constraints);
}

/*  Function logSchedule
    Prints a full schedule to log.txt
    Parameter headS: pointer to the head of a schedule linked list
*/
void logSchedule(struct schedule * headS)
{
    //print to a log
    FILE *Log;
    Log = fopen("log.txt", "a");
    fprintf(Log, "Schedule:\n");
    struct schedule * current;
    current = headS;
    //printf("logging check\n");
    while(current->T.name != NULL)
    {
        //printf("works %lf-%lf %s\n", current->startTime, current->startTime + current->executionTime, current->T.name);
        fprintf(Log, "   %lf-%lf %s\n", current->startTime, current->startTime + current->executionTime, current->T.name);
        current = current->next;
    }
    fclose(Log);
}

/*  Function logRepair
    Prints details of operations done to the schedule in log.txt
*/
void logRepair(struct schedule *currentS, int repairType){
    //print to a log
    FILE *Log;
    Log = fopen("log.txt", "a");
    switch(repairType){
        case 0:
            fprintf(Log, "Updated %s with Monte Carlo Simulation\n", currentS->T.name);
            break;
        case 1:
            fprintf(Log, "Moving %s\n", currentS->T.name);
            break;
        case 2:
            fprintf(Log, "Adding %s\n", currentS->next->T.name);
            break;
        case 3:
            fprintf(Log, "Removing %s\n", currentS->T.name);
            break;
        case 4:
            fprintf(Log, "Requesting Aide\n");
            break;
    }
    fclose(Log);
}

/*  Function describeConflictType
    Returns a description of a conflict based on the input type integer
*/
const char* describeConflictType(int conflictType){
    switch(conflictType){
        case 0:
            return "Error, not a conflict type";
            break;
        case 1:
            return "Task failure";
            break;
        case 2:
            return "Halting problem";
            break;
        case 3:
            return "Previous task completed after the task's deadline (task failure)";
            break;
        case 4:
            return "Task completed before the next task's release time";
            break;
        case 5:
            return "Task completed before the next task's start time";
            break;
        case 6:
            return "Task completed after the next task's start time";
            break;
        case 7:
            return "TC violation: Min distance between tasks";
            break;
        case 14:
            return "TC violation: Relative Task (previous)";
            break;
        case 15:
            return "TC violation: Relative Task (next)";
            break;
        case 8:
            return "Task violates a resource minimum";
            break;
        case 9:
            return "Task violates a resource maximum";
            break;
        case 10:
            return "Task violates a resource preConstraint";
            break;
        case 11:
            return "Task violates a resource maintainConstraint";
            break;
        case 12:
            return "Task violates a state preConstraint";
            break;
        case 13:
            return "Task violates a state maintainConstraint";
            break;
        default:
            printf("Invalid case\n");
            abort();
    }
}

/*  Function logConflictList
    Prints a list of conflicts to log.txt
*/
void logConflictList(struct conflict* conflicts){
    FILE *Log;
    Log = fopen("log.txt", "a");
    int i = 0;
    int maxConflictList = 10;
    const char* conflictTypeDescription;
    fprintf(Log, "Identified Conflict List:\n");
    while (conflicts[i].conflictType != 0 && abs(conflicts[i].conflictType) <= 30 && i<maxConflictList)
    {
        conflictTypeDescription = describeConflictType(conflicts[i].conflictType);
        //printf("%d\n", i+1);
        //printf("%s\n", conflictTypeDescription);
        //printf("%s\n", conflicts[i].T->T.name);
        fprintf(Log, "%d. Conflict for %s: %s\n", i+1, conflicts[i].T->T.name, conflictTypeDescription);
        i+=1;
    }
    fclose(Log);
}

/*  Function logConflict
    Prints a conflict to log.txt
*/
void logConflict(struct conflict conflicts){
    FILE *Log;
    Log = fopen("log.txt", "a");
    const char* conflictTypeDescription = describeConflictType(conflicts.conflictType);
    fprintf(Log, "Repairing Conflict for %s: %s\n", conflicts.T->T.name, conflictTypeDescription);
    fclose(Log);
}

/*  Function logUpdates
    Prints parsing text to log.txt
*/
void logUpdates(int updateID){
    FILE *Log;
    Log = fopen("log.txt", "a");
    switch(updateID)
    {
        case 0:
            fprintf(Log, "Begin GERRY Iteration\n");
            break;
        case 1:
            fprintf(Log, "Schedule Validated by GERRY, execute next task\n\n");
            break;
    }
    fclose(Log);
}

/*  Function logSimData
    Prints collected metadata about the simulation to log.txt
    Parameter: struct containing simulation metadata, including the number of task failures
*/
void logSimData(struct simData data){
    FILE *Log;
    Log = fopen("log.txt", "a");
    fprintf(Log, "Simulation Complete\n");
    fprintf(Log, "Total Failures: %d\n", data.numFail);
    fprintf(Log, "Total Early Tasks: %d\n", data.numEarly);
    fprintf(Log, "Total Late Tasks: %d\n", data.numLate);
    fprintf(Log, "Total Resource Violations: %d\n", data.numResourceViolation);
    fprintf(Log, "Total State Violations: %d\n", data.numStateViolation);
    fprintf(Log, "Error Code: %d\n", data.errorCode);
    fclose(Log);
}

void logTaskConstraints(struct schedule * headS){
    FILE *Log;
    Log = fopen("constraints.txt", "a");
    fprintf(Log, "Task Constraints:\n");
    struct schedule * current;
    current = headS;
    //fprintf("Task name | executionTime | release time | deadline | ")
    while(current->T.name != NULL)
    {
        //fprintf(Log, "%s %lf %lf %lf %lf\n", current->T.name, current->T.avExecutionTime, current->T.sdExecutionTime, current->T.WC->releasetime, current->executionTime);
        
        fprintf(Log, "%s | ", current->T.name);
        //wd
        fprintf(Log, "%lf, ", current->executionTime);
        //wc
        fprintf(Log, "%lf, %lf \n", current->T.WC->releasetime, current->T.WC->deadline);
        //tc
        fprintf(Log, "  %d TC constraints\n", current->T.numTC);
        for (int i=0; i<current->T.numTC; i++)
        {
            fprintf(Log, "      minTimeAfter:%lf | relativePrev:%d | relativeNext: %d | relativeTask: %s\n", 
                current->T.TC[i].minTimeAfter, current->T.TC[i].relativePrev, current->T.TC[i].relativeNext, current->T.TC[i].relativeTask->name);
        }
        //rr 
        fprintf(Log, "  %d RR constraints\n", current->T.numRR);
        for (int i=0; i<current->T.numRR; i++)
        {
            fprintf(Log, "      resource:%s | preContraint:%lf | maintainConstraint:%lf | preImpact:%lf | maintainImpact:%lf | postImpact:%lf\n",
                current->T.RR[i].R->name, current->T.RR[i].preConstraint, current->T.RR[i].maintainConstraint, current->T.RR[i].preImpact,
                current->T.RR[i].maintainImpact, current->T.RR[i].postImpact);
        }
        //sr
        fprintf(Log, "  %d SR constraints\n", current->T.numSR);
        for (int i=0; i<current->T.numSR; i++)
        {
            fprintf(Log, "      stateVariable:%s | preContraint:%d | maintainConstraint:%d | preImpact:%d | maintainImpact:%d | postImpact:%d\n",
                current->T.SR[i].SV->name, current->T.SR[i].preConstraint, current->T.SR[i].maintainConstraint, current->T.SR[i].preImpact,
                current->T.SR[i].maintainImpact, current->T.SR[i].postImpact);
        }
        
        
        current = current->next;
    }
    fclose(Log);
}
