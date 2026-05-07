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

/*  Function initalizeDomain
    
    Builds the resources, state variables, and tasks in the case domain
    Returns the domain struct
    
    Parameters:
        casenum - specified task case number, described in initializeTasks
        timelineLength - memory allocated for timeline circular buffers
*/
struct domain initializeDomain(int casenum, int timelineLength) 
{
	struct domain Domain = allocateDomain();
	initializeResources(&Domain, timelineLength);
	initializeStateVariables(&Domain, timelineLength);
	initializeTasks(casenum, &Domain);
	initializeTemplateTasks(&Domain);
	return Domain;
}

/*  Function initializeResources
    
    Builds and returns the resources for the domain
    Currently supports three resources:
        Energy
        CPU
        Memory
*/
void initializeResources(struct domain *Domain, int timelineLength) 
{
	Domain->numResources = 3;
	allocateResources(Domain);
	
	int count = 0;
	
	strcpy(Domain->resources[count].name, "Energy");
	strcpy(Domain->resources[count].unit, "Watt-hours");
	Domain->resources[count].globalMax = 40.0;
	Domain->resources[count].rateofchangeMin = -10.0;//energy, Watts, specific rate arbitrary
	Domain->resources[count].rateofchangeMax = 10.0;//energy, Watts, specific rate arbitrary
	Domain->resources[count].initialValue = 10;
	Domain->resources[count].timeline = initializeCircularBuffer(timelineLength, Domain->resources[count].initialValue, Domain->resources[count].initialRate);
	count +=1;
	
	strcpy(Domain->resources[count].name, "CPU");
	strcpy(Domain->resources[count].unit, "CPU Units");
	Domain->resources[count].globalMax =  100.0; //EO-1 limit (includes image processing and scl)
	Domain->resources[count].initialValue = 100.0;
	Domain->resources[count].timeline = initializeCircularBuffer(timelineLength, Domain->resources[count].initialValue, Domain->resources[count].initialRate);
	count +=1;
	
	strcpy(Domain->resources[count].name, "Memory");
	strcpy(Domain->resources[count].unit, "MB");
	Domain->resources[count].globalMax = 40; //EO-1 limit
	Domain->resources[count].initialValue = 40.0;
	Domain->resources[count].timeline = initializeCircularBuffer(timelineLength, Domain->resources[count].initialValue, Domain->resources[count].initialRate);
}

/*  Function initializeStateVariables
    
    Builds and returns the list of state variables for the domain
    Currently supports five state variables:
        Heater Status = {On, Heated, Off, Cool}
        Slew Status = {Pointed, Tracking, Broken, Saturated}
        Battery Status = {Charging, Discharging, Near Upper Limit, Near lower Limit}
        Data Quality = {Poor, Good, Anomalous, New}
        Solar Condition = {Nominal, Solar Flare}
*/
void initializeStateVariables(struct domain *Domain, int timelineLength) 
{
    Domain->numStateVariables = 5;
    allocateStateVariables(Domain);
    
    int count = 0;
    
    strcpy(Domain->stateVariables[count].name, "Heater Status");
	Domain->stateVariables[count].numStates = 4;
	strcpy(Domain->stateVariables[count].states[0], "On");
	strcpy(Domain->stateVariables[count].states[1], "Heated");
	strcpy(Domain->stateVariables[count].states[2], "Off");
	strcpy(Domain->stateVariables[count].states[3], "Cool");
	Domain->stateVariables[count].initialState = 2;
	Domain->stateVariables[count].timeline = initializeCircularBuffer(timelineLength, Domain->stateVariables[count].initialState, 0);
    count += 1;
    
    strcpy(Domain->stateVariables[count].name, "Slew Status");
	Domain->stateVariables[count].numStates = 4;
	strcpy(Domain->stateVariables[count].states[0], "Pointed");
	strcpy(Domain->stateVariables[count].states[1], "Tracking");
	strcpy(Domain->stateVariables[count].states[2], "Broken");
	strcpy(Domain->stateVariables[count].states[3], "Saturated");
	Domain->stateVariables[count].initialState = 0;
	Domain->stateVariables[count].timeline = initializeCircularBuffer(timelineLength, Domain->stateVariables[count].initialState, 0);
    count += 1;
    
    strcpy(Domain->stateVariables[count].name, "Battery Status");
	Domain->stateVariables[count].numStates = 4;
	strcpy(Domain->stateVariables[count].states[0], "Charging");
	strcpy(Domain->stateVariables[count].states[1], "Discharging");
	strcpy(Domain->stateVariables[count].states[2], "Near Upper Limit");
	strcpy(Domain->stateVariables[count].states[3], "Near Lower Limit");
	Domain->stateVariables[count].initialState = 1;
	Domain->stateVariables[count].timeline = initializeCircularBuffer(timelineLength, Domain->stateVariables[count].initialState, 0);
    count += 1;
    
    strcpy(Domain->stateVariables[count].name, "Solar Condition");
	Domain->stateVariables[count].numStates = 2;
	strcpy(Domain->stateVariables[count].states[0], "Nominal");
	strcpy(Domain->stateVariables[count].states[1], "Solar Flare");
	Domain->stateVariables[count].initialState = 0;
	Domain->stateVariables[count].timeline = initializeCircularBuffer(timelineLength, Domain->stateVariables[count].initialState, 0);
	count += 1;
	
	strcpy(Domain->stateVariables[count].name, "Data Quality");
	Domain->stateVariables[count].numStates = 4;
	strcpy(Domain->stateVariables[count].states[0], "Poor");
	strcpy(Domain->stateVariables[count].states[1], "Good");
	strcpy(Domain->stateVariables[count].states[2], "Anomalous");
	strcpy(Domain->stateVariables[count].states[3], "New");
	Domain->stateVariables[count].initialState = 1;
	Domain->stateVariables[count].timeline = initializeCircularBuffer(timelineLength, Domain->stateVariables[count].initialState, 0);

}

/*  Function initalizeTasks
    Generates an array of tasks from a set of coded cases, returns the array of tasks
    Parameters: 
        Case number - determines which task set is used
            1. Four simple tasks, duration and calandar constraints only
            2. Eight tasks with state variables, resources, duration, temporal constraints, and calandar constraints
            3. Randomized gaussian tasks with state, resource, duration, and calandar constraints
            4. Twelve templated tasks with energy resource and spread out release/deadlines
            5. Twelve templated tasks with energy resources but no release times or deadlines
        Domain - struct to build task list into, includes resource and state information
*/
void initializeTasks(int casenum, struct domain *Domain) 
{

	//int numTasks;
	//struct task *Domain->T;
	int countT=0;
    
	if(casenum == 1) //four tasks, duration and calandar constraints only
	{
		countT = 0;
		Domain->numTasks = 4;
		allocateTasks(Domain);
		
		strcpy(Domain->T[countT].name, "Point Satellite");
		Domain->T[countT].avExecutionTime = 0.8;
		Domain->T[countT].sdExecutionTime = 0.1;
		Domain->T[countT].WD = 0.8;
		allocateWC(&Domain->T[countT]);
		countT+=1;
		
		strcpy(Domain->T[countT].name, "Run Heater"); //helper function or look up table
		Domain->T[countT].avExecutionTime = 1.4;
		Domain->T[countT].sdExecutionTime = .2;
		Domain->T[countT].WD = 1.4;
		allocateWC(&Domain->T[countT]);
        countT+=1;
		
		strcpy(Domain->T[countT].name, "Take Images");
		Domain->T[countT].avExecutionTime = 3.5;
		Domain->T[countT].sdExecutionTime = 0.25;
		Domain->T[countT].WD = 3.5;
		allocateWC(&Domain->T[countT]);
		Domain->T[countT].WC->releasetime = 2.5;
        countT+=1;
        
		strcpy(Domain->T[countT].name, "Process Images");
		Domain->T[countT].avExecutionTime = 4.5;
		Domain->T[countT].sdExecutionTime = 0.25;
		Domain->T[countT].WD = 4.5;
	    allocateWC(&Domain->T[countT]);
	}

	else if(casenum == 2) //six tasks, all constraint types
	{
		countT=0;
		Domain->numTasks = 6;
		allocateTasks(Domain);
        
		strcpy(Domain->T[countT].name, "Point Satellite, for Solar Recharge");
		Domain->T[countT].avExecutionTime = 30.0/60.0;
		Domain->T[countT].sdExecutionTime = 0.2;
		Domain->T[countT].WD = Domain->T[countT].avExecutionTime;
		allocateWC(&Domain->T[countT]);
		Domain->T[countT].WC->deadline = 3.0;
		
		//rr: power and energy
		Domain->T[countT].numRR = 1;
		allocateRR(&Domain->T[countT]);
		Domain->T[countT].RR[0].R = &Domain->resources[0];
		Domain->T[countT].RR[0].maintainImpact = -1.2;
		//sr: slew status to pointed
		Domain->T[countT].numSR = 1;
		allocateSR(&Domain->T[countT]);
		Domain->T[countT].SR[0].SV = &Domain->stateVariables[1];
		Domain->T[countT].SR[0].postImpact = 0;
        
		countT+=1;
		strcpy(Domain->T[countT].name, "Solar Recharge");
		Domain->T[countT].avExecutionTime = 1.5;
		Domain->T[countT].sdExecutionTime = 0.1;
		Domain->T[countT].WD = 1.5+.1;
		allocateWC(&Domain->T[countT]);
		Domain->T[countT].WC->deadline = 6.0;
		//rr: power increase
		//use solar recharge helper function to determine energy generated
		Domain->T[countT].numRR = 1;
		allocateRR(&Domain->T[countT]);
		Domain->T[countT].RR[0].R = &Domain->resources[0];
		Domain->T[countT].RR[0].maintainImpact = 5.0;
		//sr: impact battery status to Charging then discharging
		Domain->T[countT].numSR = 1;
		allocateSR(&Domain->T[countT]);
		Domain->T[countT].SR[0].SV = &Domain->stateVariables[2];
		Domain->T[countT].SR[0].preImpact = 0;
		Domain->T[countT].SR[0].maintainImpact = 0;
        
		countT+=1;
		strcpy(Domain->T[countT].name, "Point Satellite, Target A");
		Domain->T[countT].avExecutionTime = 50.0/60.0;
		Domain->T[countT].sdExecutionTime = 0.5;
		Domain->T[countT].WD = 50.0/60.0+.5;
		allocateWC(&Domain->T[countT]);
		Domain->T[countT].WC->deadline = 8.0;
		//rr: power and energy
		//use mechanical process helper function to determine power and energy use
		Domain->T[countT].numRR = 1;
		allocateRR(&Domain->T[countT]);
		Domain->T[countT].RR[0].R = &Domain->resources[0];
		Domain->T[countT].RR[0].preConstraint = 0.6; // takes watt-hours energy, modeled as a single instant
		Domain->T[countT].RR[0].maintainImpact = -0.5; // takes watts from power
		//sr: impact slew status to tracking then pointed
		Domain->T[countT].numSR = 1;
		allocateSR(&Domain->T[countT]);
		Domain->T[countT].SR[0].SV = &Domain->stateVariables[1];
		Domain->T[countT].SR[0].preImpact = 1;
		Domain->T[countT].SR[0].postImpact = 0;
		//tc: require a minute between charging and pointing for the next Target
		Domain->T[countT].numTC = 1;
		allocateTC(&Domain->T[countT]);
		Domain->T[countT].TC[0].minTimeAfter = 1.0; 
		Domain->T[countT].TC[0].relativePrev = 0;
		Domain->T[countT].TC[0].relativeTask = &Domain->T[countT-1];
        
		countT+=1;
		strcpy(Domain->T[countT].name, "Turn on heater");
		Domain->T[countT].avExecutionTime = .6;
		Domain->T[countT].sdExecutionTime = 0.1;
		Domain->T[countT].WD = .6+.1; //could use a heater helper function to determine how long it will take
		allocateWC(&Domain->T[countT]);
		Domain->T[countT].WC->releasetime = 5.0;
		Domain->T[countT].WC->deadline = 8.0;
		//rr: power and energy
		//use heater helper function to determine how much power/energy is used
		Domain->T[countT].numRR = 1;
		allocateRR(&Domain->T[countT]);
		Domain->T[countT].RR[0].R = &Domain->resources[0];
		Domain->T[countT].RR[0].preConstraint = 3.1; // takes watt-hours energy, modeled as a single instant
		Domain->T[countT].RR[0].maintainImpact = -2.3; // takes watts from power
		//sr: heater status to on then warm enough
		Domain->T[countT].numSR = 1;
		allocateSR(&Domain->T[countT]);
		Domain->T[countT].SR[0].SV = &Domain->stateVariables[0];
		Domain->T[countT].SR[0].maintainImpact = 1;
		Domain->T[countT].SR[0].postImpact = 0;

		countT+=1;
		strcpy(Domain->T[countT].name, "Take Images");
		Domain->T[countT].avExecutionTime = 2.0;
		Domain->T[countT].sdExecutionTime = 1.0;
		Domain->T[countT].WD = 3.0;
		allocateWC(&Domain->T[countT]);
		Domain->T[countT].WC->releasetime = 8.0;
		Domain->T[countT].WC->deadline = 14.0;
		//rr: memory
		//use imaging helper function to determine memory needed to store the images
		Domain->T[countT].numRR = 2;
		allocateRR(&Domain->T[countT]);
		Domain->T[countT].RR[0].R = &Domain->resources[2];
		Domain->T[countT].RR[0].postImpact = -10; // takes MB of memory, modeled as a step function
		//rr: power and energy (small)
		Domain->T[countT].RR[1].R = &Domain->resources[0];
		Domain->T[countT].RR[1].maintainImpact = -0.5; // takes watts from power
		//sr: slew status must be pointed
		Domain->T[countT].numSR = 2;
		allocateSR(&Domain->T[countT]);
		Domain->T[countT].SR[0].SV = &Domain->stateVariables[1];
		Domain->T[countT].SR[0].preConstraint = 0;
		//sr: impact data quality to unprocessed
		Domain->T[countT].SR[1].SV = &Domain->stateVariables[3];
		Domain->T[countT].SR[1].preImpact = 3;
		//tc: wait a half a minute between heating and taking images
		Domain->T[countT].numTC = 1;
		allocateTC(&Domain->T[countT]);
		Domain->T[countT].TC[0].minTimeAfter = 0.5;
		Domain->T[countT].TC[0].relativePrev = 0;
		Domain->T[countT].TC[0].relativeTask = &Domain->T[countT-1];
		
		countT+=1;
		strcpy(Domain->T[countT].name, "Process Images");
		Domain->T[countT].avExecutionTime = 1.0;
		Domain->T[countT].sdExecutionTime = 0.7;
		Domain->T[countT].WD = 1.7;
		allocateWC(&Domain->T[countT]);
		Domain->T[countT].WC->releasetime = 8.0;
		Domain->T[countT].WC->deadline = 14.0;
		//rr: cpu
		//use image processing helper function to estimate cpu resources
		Domain->T[countT].numRR = 1;
		allocateRR(&Domain->T[countT]);
		Domain->T[countT].RR[0].R = &Domain->resources[1];
		Domain->T[countT].RR[0].preImpact = -40; // takes CPU Units, returned at the end of the task
		Domain->T[countT].RR[0].postImpact = 30;
		//sr: (expected) data quality to acceptable
		Domain->T[countT].numSR = 1;
		allocateSR(&Domain->T[countT]);
		Domain->T[countT].SR[0].SV = &Domain->stateVariables[3];
		Domain->T[countT].SR[0].postImpact = 1;
		countT+=1;
	}
    
	else if(casenum == 3) //randomize tasks with state variables and resources
	{   
		Domain->numTasks = 30; //number of tasks to randomize
		
		int rechargeFrequency = 15; //add a long recharge task every x tasks
		double energyReq;
		double timeReq;
		
		countT = Domain->numTasks;
		double releasetimeChance = 0.2; //20% chance of including a releasetime constraint
		double deadlineChance = 0.2; //20% chance of including a deadline constraint
		double resourceChance = 0.1; //10% chance of requiring a resource
		double stateChance = 0.2; //10% chance of causing a state effect, which the following task will require
		double TCChance = 0.1; //10% chance of causing a temporal constraint
        
        allocateTasks(Domain);
        
        //preallocate all RR and SR, all possible
        for(int i=0; i<Domain->numTasks; i++)
        {
            Domain->T[i].numRR = Domain->numResources;
            Domain->T[i].numSR = Domain->numStateVariables;
            allocateRR(&Domain->T[i]);
            allocateSR(&Domain->T[i]);
            Domain->T[i].numRR = 0;
            Domain->T[i].numSR = 0;
        }
            
		double maxAv = 2.0; //high end of average runtime
		double minAv = 0.1; //low end of average runtime
		double maxSDratio = 1.0; //high end of standard deviation ratio to average runtime
		double minSDratio = 0.1; //low end of standard deviation ratio to average runtime

		double avRunningTime = 0.0; //typical overall running execution time
		double wcRunningTime = 0.0; //worst case acceptable overall running exectution time
		int impactedState;
		int numRR;
		int numSR;
		int numTC;
		double max;
		double min;
		double val;

		for(int i=0; i<Domain->numTasks; i++) //randomize tasks
		{
		    if(i!= 0 && i%rechargeFrequency == 0)
		    {
		        //energy recharge Task
		        energyReq = 5;
		        timeReq = solarRechargeTimeFromEnergy(avRunningTime, energyReq);
		        
		        strcpy(Domain->T[i].name, "Solar Recharge");
		        
		        Domain->T[i].avExecutionTime = timeReq;
		        Domain->T[i].sdExecutionTime = 100.0;
		        Domain->T[i].WD = timeReq;
		        allocateWC(&Domain->T[i]);

    			Domain->T[i].numRR =1;
            	Domain->T[i].RR[0].R = &Domain->resources[0];
            	Domain->T[i].RR[0].preImpact = energyReq;
            	
    			avRunningTime += Domain->T[i].avExecutionTime;
    			wcRunningTime += (Domain->T[i].avExecutionTime);
		    }
		    else
		    {
    			sprintf(Domain->T[i].name, "%d", i+1);
    			Domain->T[i].avExecutionTime = ((double)rand()*(maxAv-minAv))/(double)RAND_MAX+minAv;
    			Domain->T[i].sdExecutionTime =(((double)rand()*(maxSDratio-minSDratio))/(double)RAND_MAX+minSDratio)*Domain->T[i].avExecutionTime;
    			Domain->T[i].WD = Domain->T[i].avExecutionTime + Domain->T[i].sdExecutionTime;
    			allocateWC(&Domain->T[i]);

    			//chance of releasetime
    			if((double)rand()/(double)RAND_MAX < releasetimeChance)
    				Domain->T[i].WC->releasetime = avRunningTime;
    			
    			avRunningTime += Domain->T[i].avExecutionTime;
    			wcRunningTime += 1.5*(Domain->T[i].avExecutionTime + Domain->T[i].sdExecutionTime);
    			
    			//chance of deadline
    			if((double)rand()/(double)RAND_MAX < deadlineChance)
    			{
    				Domain->T[i].WC->deadline = wcRunningTime;
    				Domain->T[i].movableCheck = 1;
    			}
    			
    			//chance of resource requirement
    			numRR = 0;
    			for(int j=0; j<Domain->numResources; j++)
    			{
    				if((double)rand()/(double)RAND_MAX < resourceChance)
    				{
    					numRR +=1;
    					Domain->T[i].numRR = numRR;
    					Domain->T[i].RR[numRR-1].R = &Domain->resources[j];
    					max = Domain->resources[j].globalMax/10;
    					min = Domain->resources[j].globalMin/10;
    					val = -1 * ((double)rand() / (double)RAND_MAX * (max - min) + min);
    					
    					Domain->T[i].RR[numRR-1].preConstraint = val;
    					Domain->T[i].RR[numRR-1].preImpact = val;
    				}
    			}
    			
    			//chance of state effect
    			//numSR = 0;
    			for(int j=0; j<Domain->numStateVariables; j++)
    			{
    			    impactedState = j;
    			    //check only if previous task already has currently searched for stateChance
    			    for (int k=0; k<Domain->T[i-1].numSR; k++){
    			        if (strcmp(Domain->T[i-1].SR[k].SV->name, Domain->stateVariables[j].name) == 0){
    			            impactedState = -1;
    			            break;
    			        }
    			    }
    				if((i>2) && impactedState != -1 /*(Domain->T[i-1].numSR == 0)*/ && (double)rand()/(double)RAND_MAX < stateChance)
    				{
    					Domain->T[i].numSR +=1; //TBD set fields beforehand, so testing happens after
    					numSR = Domain->T[i].numSR;
    					Domain->T[i].SR[numSR-1].SV = &Domain->stateVariables[j];
    					impactedState = (rand() % (Domain->stateVariables[j].numStates));
    					Domain->T[i].SR[numSR-1].preConstraint = impactedState;
    					if(Domain->T[i].SR[numSR-1].maintainImpact == 0)
    					{
    					    Domain->T[i].SR[numSR-1].preConstraint = -1;
    					    Domain->T[i].SR[numSR-1].maintainConstraint = -1;
    					    Domain->T[i].SR[numSR-1].maintainImpact = -1;
    					    Domain->T[i].SR[numSR-1].postImpact = -1;
    					}
    					//update previous post impact to match new preconstraint
    					Domain->T[i-1].numSR +=1;
    					numSR = Domain->T[i-1].numSR;
    					Domain->T[i-1].SR[Domain->T[i-1].numSR-1].SV = &Domain->stateVariables[j];
    					Domain->T[i-1].SR[Domain->T[i-1].numSR-1].preImpact = impactedState;
    					if(Domain->T[i-1].SR[Domain->T[i-1].numSR-1].maintainImpact == 0)
    					{
    					    Domain->T[i-1].SR[Domain->T[i-1].numSR-1].preConstraint = -1;
    					    Domain->T[i-1].SR[Domain->T[i-1].numSR-1].maintainConstraint = -1;
    					    Domain->T[i-1].SR[Domain->T[i-1].numSR-1].maintainImpact = -1;
    					    Domain->T[i-1].SR[Domain->T[i-1].numSR-1].postImpact = -1;
    					}
    				}
    			}
    			//chance of temporal constraint with previous tasks
    			numTC = 0;
    			if((i>2) && (Domain->T[i-1].numTC == 0) && (double)rand()/(double)RAND_MAX < TCChance)
				{
				    Domain->T[i].numTC += 1;
				    allocateTC(&Domain->T[i]);
		            Domain->T[i].TC[0].minTimeAfter = (rand() / (double)RAND_MAX) * (2.0 - 0.5) + 0.5; //random delay between 30 seconds to 2 minutes
		            Domain->T[i].TC[0].relativeTask = &Domain->T[i-1];
		            
				}
    			
		    }
        }
	}

	else if(casenum == 4) //templated tasks with energy resource and spread out release times
	{
		//initialize targets
		int numTargets = 3; //number of targets (not including starting position)
		int targetCount = 1;
		double targets[numTargets+1][3];

		for(int i=0; i<numTargets+1; i++)
			targetVector(targets[i]);
        
        //allocate space for tasks
        countT=0;
		Domain->numTasks = 5*numTargets; //update as add tasks
		allocateTasks(Domain);
        
        rechargingTask(Domain, &Domain->T[countT], 0, 10);
        countT+=1;
        
		pointingTask(Domain, &Domain->T[countT], "A", targets[targetCount-1], targets[targetCount], 100.0, 0.0);
		countT +=1;
		targetCount +=1;

		heatingTask(Domain, &Domain->T[countT], 100.0, 0.0);
		countT +=1;

		imagingTask(Domain, &Domain->T[countT], "A", 100.0, 130.0, 1); //long imaging task
		countT +=1;

		processingTask(Domain, &Domain->T[countT], "A", 0.0, 130.0);
		countT +=1;
		
		rechargingTask(Domain, &Domain->T[countT], 116, 10);
        countT+=1;

		pointingTask(Domain, &Domain->T[countT], "B", targets[targetCount-1], targets[targetCount], 200.0, 0.0);
		countT+=1;
		targetCount +=1;

		heatingTask(Domain, &Domain->T[countT], 200.0, 0.0);
		countT +=1;

		imagingTask(Domain, &Domain->T[countT], "B", 200.0, 240.0, 0); //short imaging task
		countT +=1;

		processingTask(Domain, &Domain->T[countT], "B", 0.0, 240.0);
		countT +=1;
		
		rechargingTask(Domain, &Domain->T[countT], 207, 10);
        countT+=1;

		pointingTask(Domain, &Domain->T[countT], "C", targets[targetCount-1], targets[targetCount], 300.0, 0.0);
		countT +=1;
		targetCount +=1;

		heatingTask(Domain, &Domain->T[countT], 300.0, 0.0);
		countT +=1;

		imagingTask(Domain, &Domain->T[countT], "C", 300.0, 360.0, 1); //long imaging task
		countT +=1;

		processingTask(Domain, &Domain->T[countT], "C", 0.0, 360.0);
		countT +=1;
	}

	else if(casenum == 5) //same as 4, but no deadlines or release times
	{
		//initialize targets
		int numTargets = 3; //number of targets (not including starting position)
		int targetCount = 1;
		double targets[numTargets+1][3];

		for(int i=0; i<numTargets+1; i++)
			targetVector(targets[i]);
			
		countT=0;
		Domain->numTasks = 5*numTargets; //updates as add tasks
		allocateTasks(Domain);
		//Domain->T = (struct task *) malloc(sizeof(struct task) * (numTasks));

		//initial solar recharge task
		rechargingTask(Domain, &Domain->T[countT], 0, 10);
		countT+=1;

		pointingTask(Domain, &Domain->T[countT], "A", targets[targetCount-1], targets[targetCount], 0.0, 0.0);
		countT +=1;
		targetCount +=1;

		heatingTask(Domain, &Domain->T[countT], 0.0, 0.0);
		countT +=1;

		imagingTask(Domain, &Domain->T[countT], "A", 0, 0, 1); //long imaging task
		countT +=1;

		processingTask(Domain, &Domain->T[countT], "A", 0.0, 0);
		countT +=1;
		
		rechargingTask(Domain, &Domain->T[countT], 87, 10);
		countT+=1;

		pointingTask(Domain, &Domain->T[countT], "B", targets[targetCount-1], targets[targetCount], 0, 0);
		countT+=1;
		targetCount +=1;

		heatingTask(Domain, &Domain->T[countT], 0, 0);
		countT +=1;

		imagingTask(Domain, &Domain->T[countT], "B", 0, 0, 0); //short imaging task
		countT +=1;

		processingTask(Domain, &Domain->T[countT], "B", 0, 0);
		countT +=1;
		
		rechargingTask(Domain, &Domain->T[countT], 130, 10);
		countT+=1;

		pointingTask(Domain, &Domain->T[countT], "C", targets[targetCount-1], targets[targetCount], 0, 0);
		countT +=1;
		targetCount +=1;

		heatingTask(Domain, &Domain->T[countT], 0, 0);
		countT +=1;

		imagingTask(Domain, &Domain->T[countT], "C", 0, 0, 1); //long imaging task
		countT +=1;

		processingTask(Domain, &Domain->T[countT], "C", 0, 0);
		countT +=1;
	}

    else
    {
        printf("Select an acceptable case number\n");
        exit(2);
    }
    
}

/* Function initializeTemplateTasks
    Creates a list of tasks that can be used by GERRY to fix constraint violations
    Three template tasks currently supported:
        Wait
        Request Scheduling Aide
        Solar Recharge

    Note: more template tasks could be added, to support more resources and state variables
*/
void initializeTemplateTasks(struct domain *Domain) 
{
	int numTemplates = 3; // full list of 12, only using energy for now
	allocateTemplateTasks(Domain, numTemplates);
	//struct task *TemplateList = (struct task *) malloc(sizeof(struct task) * (numTemplates+1));
	int countT=0;

	//wait task, used by iterative repair
	strcpy(Domain->templateTasks[countT].name, "Wait");
	Domain->templateTasks[countT].avExecutionTime = 0.0;
	Domain->templateTasks[countT].sdExecutionTime = 100;
	Domain->templateTasks[countT].WD = 1000;
	allocateWC(&Domain->templateTasks[countT]);

	//request aide task, used by iterative repair
	countT +=1;
	strcpy(Domain->templateTasks[countT].name, "Request Scheduling Aide");
	Domain->templateTasks[countT].avExecutionTime = 0.0;
	Domain->templateTasks[countT].sdExecutionTime = 100;
	Domain->templateTasks[countT].WD = 1000;
	allocateWC(&Domain->templateTasks[countT]);

	//resource tasks, used to increase or decrease resources
	countT +=1;
	strcpy(Domain->templateTasks[countT].name, "Solar Recharge");
	Domain->templateTasks[countT].avExecutionTime = 0.0;
	Domain->templateTasks[countT].sdExecutionTime = 100.0;
	Domain->templateTasks[countT].WD = 1000;
	allocateWC(&Domain->templateTasks[countT]);
	//rr: increases avaliable energy
	Domain->templateTasks[countT].numRR = 1;
	allocateRR(&Domain->templateTasks[countT]);
	Domain->templateTasks[countT].RR[0].R = &Domain->resources[0];

}

/*  Function: pointingTask
    
    Used to build a schedule with predefined pointing constraints
    
    Parameters:
        Domain - Information about the domain
        Task - Allocated task struct to become a pointing task
        ID - Indication of the target pointed to
        VectorA, VectorB - 3D vectors of the start/end orientation of the telescope
        releasetime - time that the task may begin (default to 0)
        deadline - time that the task must begin by or be failed (default to 0)
*/
void pointingTask(struct domain * Domain, struct task * Task, char* ID, double * vectorA, double * vectorB, double releasetime, double deadline) 
{
	strcpy(Task->name, "Point Satellite, Target ");
	strcat(Task->name, ID);
    
    allocateWC(Task);
	Task->WC->releasetime = releasetime;
	Task->WC->deadline = deadline;

	Task->numRR = 1;
	allocateRR(Task);
	Task->RR[0].R = &Domain->resources[0];

	slew(vectorA, vectorB, &Task->avExecutionTime, &Task->RR[0].preImpact);
	Task->sdExecutionTime = Task->avExecutionTime * 0.1; //10% of execution time standard deviation
	Task->WD = Task->avExecutionTime;
}

/*  Function heatingTask
    
    Used to build a schedule with predefined heating constraints
    
    Parameters:
        Domain - Information about the domain
        Task - Allocated task struct to become a pointing task
        releasetime - time that the task may begin (default to 0)
        deadline - time that the task must begin by or be failed (default to 0)

*/
void heatingTask(struct domain * Domain, struct task * Task, double releasetime, double deadline) 
{
	strcpy(Task->name, "Run Heater");
	Task->avExecutionTime = 1.4;
	Task->sdExecutionTime = 0.1;
	Task->WD = Task->avExecutionTime;
	allocateWC(Task);
	Task->WC->releasetime = releasetime;
	Task->WC->deadline = deadline;
	//rr: use heater helper function
	Task->numRR = 1;
	allocateRR(Task);
	Task->RR[0].R = &Domain->resources[0];
	Task->RR[0].preImpact = -0.05;
	Task->RR[0].maintainImpact = -3.0;
}

/*  Function imagingTask
    
    Used to build a schedule with predefined imaging constraints
    
    Parameters:
        Domain - Information about the domain
        Task - Allocated task struct to become a pointing task
        ID - Indicates the specific target to be imaged
        releasetime - time that the task may begin (default to 0)
        deadline - time that the task must begin by or be failed (default to 0)
        shortOrLong - Indicates whether to assume a short observation (0) or a long observation (1)
*/
void imagingTask(struct domain * Domain, struct task * Task, char* ID, double releasetime, double deadline, int shortOrLong) 
{
	//0 = short observation, 1 = long observation
	double shortObservation = 2.0; //minutes
	double longObservation = 10.0; //minutes

	strcpy(Task->name, "Take Images, Target ");
	strcat(Task->name, ID);
	if (shortOrLong == 0)
		Task->avExecutionTime = shortObservation;
	else
		Task->avExecutionTime = longObservation;
	Task->sdExecutionTime = 0.1*Task->avExecutionTime;
    
    Task->movableCheck = 1; //can be moved to another time at the same orbit position
	
	Task->WD = Task->avExecutionTime;
	//wc
	allocateWC(Task);
	Task->WC->releasetime = releasetime;
	Task->WC->deadline = deadline;
	//rr: costs energy to turn on and off, along with maintained energy use
	Task->numRR = 1;
	allocateRR(Task);
	Task->RR[0].R = &Domain->resources[0];
	Task->RR[0].preImpact = -0.5;
	Task->RR[0].maintainImpact = -4.0;
}

/*  Function processingTask
    
    Used to build a schedule with predefined processing constraints
    
    Parameters:
        Domain - Information about the domain
        Task - Allocated task struct to become a pointing task
        ID - Indicates specific target which needs data processing
        releasetime - time that the task may begin (default to 0)
        deadline - time that the task must begin by or be failed (default to 0)
*/
void processingTask(struct domain * Domain, struct task * Task, char* ID, double releasetime, double deadline) 
{
	strcpy(Task->name, "Process Images, Target ");
    strcat(Task->name, ID);
	Task->avExecutionTime = 4.5;
	Task->sdExecutionTime = 0.4;
	//wd: use a lookup table
	Task->WD = Task->avExecutionTime;
	//wc: use a lookup table
	allocateWC(Task);
	Task->WC->releasetime = releasetime;
	Task->WC->deadline = deadline;
	//rr: use helper function
	Task->numRR = 1;
	allocateRR(Task);
	Task->RR[0].R = &Domain->resources[0];
	Task->RR[0].maintainImpact = -3.5;
}

void rechargingTask(struct domain * Domain, struct task * Task, double estimatedStartTime, double energyReq) 
{
    double timeReq = solarRechargeTimeFromEnergy(estimatedStartTime, energyReq);
    strcpy(Task->name, "Solar Recharge");
    Task->avExecutionTime = timeReq;
    Task->sdExecutionTime = 100.0;
    Task->WD = timeReq;
    allocateWC(Task);
	Task->numRR =1;
	allocateRR(Task);
	Task->RR[0].R = &Domain->resources[0];
	Task->RR[0].preImpact = energyReq;
}

struct domain allocateDomain()
{
    struct domain * D;
    D = (struct domain*) malloc(sizeof(struct domain));
    D->numStateVariables = 0;
    D->stateVariables = NULL;
    D->numResources = 0;
    D->resources = NULL;
    D->numTasks = 0;
    D->T = NULL;
    return D[0];
}

void allocateTasks(struct domain *D)
{
    D->T = (struct task *) malloc(sizeof(struct task) * D->numTasks);
    for (int i=0; i<D->numTasks; i++)
    {
        D->T[i].avExecutionTime = 0.0;
        D->T[i].sdExecutionTime = 0.0;
        D->T[i].movableCheck = 0;
        D->T[i].WD = 0.0;
        D->T[i].numTC = 0;
        D->T[i].TC = NULL;
        D->T[i].numRR = 0;
        D->T[i].RR = NULL;
        D->T[i].numSR = 0;
        D->T[i].SR = NULL;
        D->T[i].S = NULL;
    }
}

void allocateTemplateTasks(struct domain *D, int num)
{
    D->templateTasks = (struct task *) malloc(sizeof(struct task) * num);
    for (int i=0; i<num; i++)
    {
        D->templateTasks[i].avExecutionTime = 0.0;
        D->templateTasks[i].sdExecutionTime = 0.0;
        D->templateTasks[i].movableCheck = 0;
        D->templateTasks[i].WD = 0.0;
        D->templateTasks[i].numTC = 0;
        D->templateTasks[i].TC = NULL;
        D->templateTasks[i].numRR = 0;
        D->templateTasks[i].RR = NULL;
        D->templateTasks[i].numSR = 0;
        D->templateTasks[i].SR = NULL;
        D->templateTasks[i].S = NULL;
    }
}

void allocateWC(struct task *T)
{
    T->WC = (struct WCconstraint *) malloc(sizeof(struct WCconstraint));
    T->WC->releasetime = 0.0;
    T->WC->deadline = 0.0;
}

void allocateTC(struct task *T)
{
    T->TC = (struct TCconstraint *) malloc(sizeof(struct TCconstraint)*(T->numTC));
    for (int i=0; i<T->numTC; i++)
    {
        T->TC[i].minTimeAfter = 0.0;
        T->TC[i].relativePrev = 0;
        T->TC[i].relativeNext = 0;
        T->TC[i].relativeTask = NULL;
    }
}

void allocateRR(struct task *T)
{
    T->RR = (struct RRconstraint *) malloc(sizeof(struct RRconstraint)*(T->numRR));
    for (int i=0; i<T->numRR; i++)
    {
        T->RR[i].R = NULL;
        T->RR[i].preConstraint = 0.0;
        T->RR[i].maintainConstraint = 0.0;
        T->RR[i].preImpact = 0.0;
        T->RR[i].maintainImpact = 0.0;
        T->RR[i].postImpact = 0.0;
        T->RR[i].timelineIndex = 0;
    }
}

void allocateSR(struct task *T)
{
    T->SR = (struct StateConstraint *) malloc(sizeof(struct StateConstraint)*(T->numSR));
    for (int i=0; i<T->numRR; i++)
    {
        T->SR[i].SV = NULL;
        T->SR[i].preConstraint = -1;
        T->SR[i].maintainConstraint = -1;
        T->SR[i].preImpact = -1;
        T->SR[i].maintainImpact = -1;
        T->SR[i].postImpact = -1;
        T->RR[i].timelineIndex = 0;
    }
}

void allocateResources(struct domain *D)
{
    D->resources = (struct resource *) malloc(sizeof(struct resource) * (D->numResources));
    for (int i=0; i<D->numResources; i++)
    {
        D->resources[i].initialValue = 0.0;
        D->resources[i].initialRate = 0.0;
        D->resources[i].globalMin = 0.0;
        D->resources[i].globalMax = 0.0;
        D->resources[i].rateofchangeMin = 0.0;
        D->resources[i].rateofchangeMax = 0.0;
        D->resources[i].timeline = NULL;
    }
}

void allocateStateVariables(struct domain *D)
{
    D->stateVariables = (struct stateVariable *) malloc(sizeof(struct stateVariable) * (D->numStateVariables));
    for (int i=0; i<D->numStateVariables; i++)
    {
        D->stateVariables[i].numStates = 0;
        D->stateVariables[i].initialState = 0;
        D->stateVariables[i].timeline = NULL;
    }
}



