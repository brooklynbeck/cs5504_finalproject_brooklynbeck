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

/*  Function createNode
    Allocates memory and sets default values for a schedule struct
    Returns a pointer to a empty schedule struct
*/
struct schedule * createNode()
{
    struct schedule * temp;
    temp = (struct schedule*) malloc(sizeof(struct schedule));
    temp->next = NULL;
    temp->prev = NULL;
    return temp;
}

/*  Function insertNext
    Adds a task (newTask) after the given task (currentS)
*/
struct schedule* insertNext(struct schedule * currentS, struct task newTask)
{
        //Add in task after currentS
        struct schedule * temp = createNode();
        temp->T = newTask;
        temp->startTime = currentS->startTime + currentS->executionTime;
        temp->executionTime = temp->T.avExecutionTime;
        temp->prev = currentS;
        if(currentS->next != NULL)
        {
            temp->next = currentS->next;
            currentS->next->prev = temp;
        }
        //copy RR
        /*temp->T.numRR = newTask.numRR;
        temp->T.RR = realloc(temp->T.RR, sizeof(struct RRconstraint) * temp->T.numRR);
        for(int i=0; i<temp->T.numRR; i++)
        {
            temp->T.RR[i].R = newTask.RR[i].R;
            temp->T.RR[i].preConstraint = newTask.RR[i].preConstraint;
            temp->T.RR[i].maintainConstraint = newTask.RR[i].maintainConstraint;
            temp->T.RR[i].preImpact = newTask.RR[i].preImpact;
            temp->T.RR[i].maintainImpact = newTask.RR[i].maintainImpact;
            temp->T.RR[i].postImpact = newTask.RR[i].postImpact;
        }
        //copy SR
        temp->T.numSR = newTask.numSR;
        temp->T.SR = realloc(temp->T.SR, sizeof(struct StateConstraint) * temp->T.numSR);
        for(int i=0; i<temp->T.numSR; i++)
        {
            temp->T.SR[i].SV = newTask.SR[i].SV;
            temp->T.SR[i].preConstraint = newTask.SR[i].preConstraint;
            temp->T.SR[i].maintainConstraint = newTask.SR[i].maintainConstraint;
            temp->T.SR[i].preImpact = newTask.SR[i].preImpact;
            temp->T.SR[i].maintainImpact = newTask.SR[i].maintainImpact;
            temp->T.SR[i].postImpact = newTask.SR[i].postImpact;
        }
        //copy TC
        temp->T.numTC = newTask.numTC;
        temp->T.TC = realloc(temp->T.TC, sizeof(struct TCconstraint) * temp->T.numTC);
        for(int i=0; i<temp->T.numSR; i++)
        {
            temp->T.TC[i].relativeTask = newTask.TC[i].relativeTask;
            temp->T.TC[i].minTimeAfter = newTask.TC[i].minTimeAfter;
            temp->T.TC[i].relativePrev = newTask.TC[i].relativePrev;
            temp->T.TC[i].relativeNext = newTask.TC[i].relativeNext;
        }*/
        
        currentS->next = temp;
        
        return currentS;
}

/*  Function insertBefore
    Adds a task (newTask) before the given current task (currentS)
*/
struct schedule* insertBefore(struct schedule * currentS, struct task newTask)
{
        struct schedule * temp = createNode();
        temp->T = newTask;
        temp->executionTime = temp->T.avExecutionTime;
        temp->prev = currentS;
        if(currentS->prev != NULL)
        {
            temp->prev = currentS->prev;
            currentS->prev->next = temp;
        }
        currentS->prev = temp;
        return currentS;
}

/*  Function pop
    Removes a task from the schedule
*/
void pop(struct schedule * currentS)
{
    if (currentS->next != NULL)
        currentS->next->prev = currentS->prev;
    if (currentS->prev != NULL)
        currentS->prev->next = currentS->next;
    free(currentS);
}

/*  Function printSchedule
    Prints a schedule
    Parameter: pointer to the head of a schedule linked list
*/
void printSchedule(struct schedule * headS)
{
    printf("Schedule:\n");
    struct schedule * current;
    current = headS;
    // loop until the final schedule struct
    while(current != NULL)
    {
        printf("   %lf-%lf %s\n", current->startTime, current->startTime + current->executionTime, current->T.name);
        current = current->next;
    }
}

/*  Function freeSchedule
    Frees the memory of a schedule
    Parameter: pointer to the head of a schedule linked list
*/
void freeSchedule(struct schedule * headS)
{
    struct schedule * temp = headS;
    // loop until the final schedule struct
    while (temp->next != NULL)
    {
        temp = temp->next;
        free(temp->prev);
        temp->prev = NULL;
    }
    free(temp);
    temp = NULL;
}



