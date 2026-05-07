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

/*  Function printTasks
    Prints all tasks
    Parameter: 
        myDomain - domain, including tasks, state variables, and resources
*/
void printTasks(struct domain myDomain)
{
    //printf("test %d\n", myDomain.numTasks);
    
    int numTasks = myDomain.numTasks;
    printf("Tasks:\n");
    for(int i=0; i<numTasks; i++)
    {
        printf("Task %d: %s\n", i+1, myDomain.T[i].name);
        printf("   WD: %lf\n", myDomain.T[i].WD);
        
        if(myDomain.T[i].WC->releasetime!=0.0)
        {
            printf("   WC Release Time: %lf\n", myDomain.T[i].WC->releasetime);
        }
        if(myDomain.T[i].WC->deadline!=0.0)
        {
            printf("   WC Deadline: %lf\n", myDomain.T[i].WC->deadline);
        }
        //still need tc constraint printed
        for(int j=0;j<myDomain.T[i].numRR;j++)
        {
            if(myDomain.T[i].RR[j].preConstraint != 0)
                printf("   RR %s preConstraint: %lf %s\n", myDomain.T[i].RR[j].R->name, myDomain.T[i].RR[j].preConstraint, myDomain.T[i].RR[j].R->unit);
            if(myDomain.T[i].RR[j].maintainConstraint != 0)
                printf("   RR %s maintainConstraint: %lf %s/s\n", myDomain.T[i].RR[j].R->name, myDomain.T[i].RR[j].maintainConstraint, myDomain.T[i].RR[j].R->unit);
            if(myDomain.T[i].RR[j].preImpact != 0)
                printf("   RR %s preImpact: %lf %s\n", myDomain.T[i].RR[j].R->name, myDomain.T[i].RR[j].preImpact, myDomain.T[i].RR[j].R->unit);
            if(myDomain.T[i].RR[j].maintainImpact != 0)
                printf("   RR %s maintainImpact: %lf %s/s\n", myDomain.T[i].RR[j].R->name, myDomain.T[i].RR[j].maintainImpact, myDomain.T[i].RR[j].R->unit);
            if(myDomain.T[i].RR[j].postImpact != 0)
                printf("   RR %s postImpact: %lf %s\n", myDomain.T[i].RR[j].R->name, myDomain.T[i].RR[j].postImpact, myDomain.T[i].RR[j].R->unit);   
        }
        for(int j=0;j<myDomain.T[i].numSR;j++)
        {
            if(myDomain.T[i].SR[j].preConstraint !=-1)
                printf("   SR %s preConstraint: %s\n", myDomain.T[i].SR[j].SV->name, myDomain.T[i].SR[j].SV->states[myDomain.T[i].SR[j].preConstraint]);
            if(myDomain.T[i].SR[j].maintainConstraint !=-1)
                printf("   SR %s maintainConstraint: %s\n", myDomain.T[i].SR[j].SV->name, myDomain.T[i].SR[j].SV->states[myDomain.T[i].SR[j].maintainConstraint]);
            if(myDomain.T[i].SR[j].preImpact !=-1)
                printf("   SR %s preImpact: %s\n", myDomain.T[i].SR[j].SV->name, myDomain.T[i].SR[j].SV->states[myDomain.T[i].SR[j].preImpact]);
            if(myDomain.T[i].SR[j].maintainImpact!=-1)
                printf("   SR %s maintainImpact: %s\n", myDomain.T[i].SR[j].SV->name, myDomain.T[i].SR[j].SV->states[myDomain.T[i].SR[j].maintainImpact]);
            if(myDomain.T[i].SR[j].postImpact !=-1)
                printf("   SR %s postImpact: %s\n", myDomain.T[i].SR[j].SV->name, myDomain.T[i].SR[j].SV->states[myDomain.T[i].SR[j].postImpact]); 
        }
    }
}

/*  Function freeDomain
    Frees the memory of the domain
*/
void freeDomain(struct domain *myDomain)
{
    freeTasks(myDomain);
    freeStates(myDomain);
    freeResources(myDomain);
}

/*  Function freeTasks
    Frees the memory of all tasks
*/
void freeTasks(struct domain *myDomain)
{
    for(int i=0; i< myDomain->numTasks; i++)
    {
        //printf("%s: %d SR constraints\n", myDomain->T[i].name, myDomain->T[i].numSR);
        free(myDomain->T[i].WC);
        if(myDomain->T[i].numRR>0)
            free(myDomain->T[i].RR);
        if(myDomain->T[i].numSR>0)
            free(myDomain->T[i].SR);
        if(myDomain->T[i].numTC>0)
            free(myDomain->T[i].TC);
    }
    free(myDomain->T);
    myDomain->T = NULL;
    myDomain->numTasks = 0;
}

/*  Function freeStates
    Frees the memory of all state variables, including the timelines
*/
void freeStates(struct domain *myDomain)
{
    
    for(int i=0;i<myDomain->numStateVariables;i++){
        freeCircularBuffer(myDomain->stateVariables[i].timeline);
    }
    
    free(myDomain->stateVariables);
}
    
/*  Function freeResources
    Frees the memory of the resources, including resource timelines
*/
void freeResources(struct domain *myDomain)
{
    for(int i=0;i<myDomain->numResources;i++){
        freeCircularBuffer(myDomain->resources[i].timeline);
    }
    
    free(myDomain->resources);
}
