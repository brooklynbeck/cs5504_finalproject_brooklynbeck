#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#ifndef GERRY_H
#define GERRY_H

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

//data structures and functions used in the GERRY algorithm

/*  structs: domain, task, WCconstraint, TCconstraint, RRconstraint, StateConstraint, 
        resource, stateVariable, timelineResource, circularBuffer, conflict, schedule, simData, */

//domain: a set of state variables, resources, and tasks
struct domain {
    int numStateVariables;
    struct stateVariable *stateVariables;
    int numResources;
    struct resource *resources;
    int numTasks;
    struct task *T;
    struct task *templateTasks;
    double stateAnnotations[9]; //{greenwich time, true anomaly, 3 angular positions, 3 angular velocities, temperature}
};
//task: task information including name, runtime statistical data, and constraints
struct task {
    char name[80];
    double avExecutionTime; // minutes, metadata for Monte Carlo simulation
    double sdExecutionTime; // minutes, metadata for Monte Carlo simulation
    int movableCheck; //check if a task can be moved to another time at the same orbital position

    //constraints
    double WD; // Every task should have exactly one WD constraint (wcet)
    struct WCconstraint *WC; // Tasks may have one or no WC constraint (releasetime and deadline)
    int numTC;
    struct TCconstraint *TC; // pointer to an array of TC constraints (relativeTask and time before/after)
    int numRR;
    struct RRconstraint *RR; // pointer to an array of RR constraints (resource and requirement)
    int numSR;
    struct StateConstraint *SR; // pointer to an array of SR constraints (state variable and integer state)
    
    struct schedule * S; //pointer to the scheduled task
};
//WCconstraint: releasetime and deadline for a task
struct WCconstraint {
	double releasetime;
	double deadline;
};
//TCconstraint: prior task and temporal relation to that task
struct TCconstraint {
    double minTimeAfter; //minimum time constrainted task can be after the relative task
    int relativePrev; //relative task must be immediately before the given task
    int relativeNext; //relative task must be immediately after the given task
    struct task *relativeTask; // pointer to a specific prior task
};
//RRconstraint: resource and type of constraint (before/during/after, impact/constraint)
struct RRconstraint {
    struct resource *R; // timeline of how much resource there is / expected to be
    double preConstraint;
    double maintainConstraint;
    double preImpact;
    double maintainImpact;
    double postImpact;
    int timelineIndex;
};
//StateConstraint: state variable and type of constraint (before/during/after, impact/constraint)
struct StateConstraint {
    struct stateVariable *SV; // timeline of what state the variable is in / is expected to be in
    int preConstraint;
    int maintainConstraint;
    int preImpact;
    int maintainImpact;
    int postImpact;
    int timelineIndex;
};
//resource: domain resource information including name, measurement unit, constraint info, and resource timeline
struct resource {
    char name[80];
    char unit[80];
    double initialValue;
    double initialRate;
    double globalMin;
    double globalMax;
    double rateofchangeMin;
    double rateofchangeMax;
    struct circularBuffer *timeline; //circular buffer
};
//stateVariable: domain state variable information including name, potential states, and state variable timeline
struct stateVariable {
    char name[80];
    int numStates;
    char states[5][80]; //list of potential states
    int initialState;
    struct circularBuffer *timeline;
};
//timelineResource: information stored in a resource or state timeline including, timestamp, value, and rate
struct timelineResource {
    double timestamp;
    double currentValue;
    double currentRate;
};
//circularBuffer: list of resource or state information overtime, tracked in a ciruclar buffer with a maximum length to limit memory
struct circularBuffer {
    int head;
    int tail;
    int maxlen;
    struct timelineResource* buffer;
};
//conflict: information about a constraint violation, including the conflict type, state, resource, priority, and related task
struct conflict {
    int conflictType;
    int stateVariableAffected;
    int resourceAffected;
    int priority;
    struct schedule *T;
};
// schedule: a linked list of tasks in order with their execution time, as determined in a monte carlo simulation
struct schedule {
    struct task T;
    double startTime; //acutal start time, based on previous tasks
    double executionTime; //actual time to execute, determined in a monte carlo simulaiton
    struct schedule *next;
    struct schedule *prev; //use to add tasks earlier for constraint violations
};
// simData: tracks metadata across the entire simulation, including the number of failures, early tasks, and late tasks
struct simData {
    int numFail;
    int numEarly;
    int numLate;
    int numResourceViolation;
    int numStateViolation;
    int errorCode; // 0 = successful simulation, 1 = failed simulation, 2 = code failure
};

/*  functions: organized by file, see individual files for function descriptions*/

//iterativeRepair.c
struct schedule * iterativeRepair(struct schedule * currentS, struct schedule *headS, struct domain *currentDomain, struct simData * data, double failureChance);
void updateDomain(double failureChance, struct schedule * currentS);
int countConflicts(struct conflict* conflictList);
struct conflict* getConflicts(struct schedule * currentS);
void printConflictList(struct conflict* conflicts);
void freeConflicts(struct conflict* conflicts);
struct conflict chooseConflict(struct conflict* conflicts);
void chooseMethod(double *allocatedArray, struct conflict currentConflict, struct simData * data, struct domain *currentDomain);
void move(struct schedule * toMove, double newTime, struct domain *currentDomain);
void add(struct schedule * toAddAfter, int newTaskIndicator, struct domain *currentDomain, double waitTime);
void delete(struct schedule *toRemove, struct schedule *nextTask);
void aide();

//initializeDomain.c
struct domain initializeDomain(int casenum, int timelineLength);
void initializeResources(struct domain *Domain, int timelineLength);
void initializeStateVariables(struct domain *Domain, int timelineLength);
void initializeTasks(int casenum, struct domain *Domain);
void initializeTemplateTasks(struct domain *Domain);
void pointingTask(struct domain * Domain, struct task * Task, char * ID, double * vectorA, double * vectorB, double releasetime, double deadline);
void heatingTask(struct domain * Domain, struct task * Task, double releasetime, double deadline);
void imagingTask(struct domain * Domain, struct task * Task, char* ID, double releasetime, double deadline, int shortOrLong);
void processingTask(struct domain * Domain, struct task * Task, char* ID, double releasetime, double deadline);
void rechargingTask(struct domain * Domain, struct task * Task, double estimatedStartTime, double energyReq);

struct domain allocateDomain();
void allocateTasks(struct domain *D);
void allocateTemplateTasks(struct domain *D, int num);
void allocateWC(struct task *T);
void allocateTC(struct task *T);
void allocateRR(struct task *T);
void allocateSR(struct task *T);
void allocateResources(struct domain *D);
void allocateStateVariables(struct domain *D);


//manageTasks.c
void Tasks(struct domain myDomain);
void freeDomain(struct domain *myDomain);
void freeTasks(struct domain *myDomain);
void freeStates(struct domain *myDomain);
void freeResources(struct domain *myDomain);

//initializeSchedule.c
struct schedule * initializeSchedule(struct domain T);

//manageSchedule.c
struct schedule * createNode();
struct schedule* insertNext(struct schedule * currentS, struct task newTask);
struct schedule* insertBefore(struct schedule * currentS, struct task newTask);
void pop(struct schedule * currentS);
void printSchedule(struct schedule * headS);
void freeSchedule(struct schedule * headS);

//montecarlosimulation.c
struct schedule * mcs(struct schedule * S, double failureChance);
double gaussianDistribution(double mean, double stddev);
void targetVector(double * vector);

//manageLog.c
void initializeLog();
void logSchedule(struct schedule * headS);
void logRepair(struct schedule *currentS, int repairType);
void logConflictList(struct conflict* conflicts);
void logConflict(struct conflict conflicts);
void logUpdates(int updateID);
void logSimData(struct simData data);
void logTaskConstraints(struct schedule * headS);

//helpers.h

//manageCircularBuffer.c
struct circularBuffer* initializeCircularBuffer(int maxlength, double initialValue, double initialRate);
void updateCircularBuffer(struct schedule *headS, struct domain *Domain);
void freeCircularBuffer(struct circularBuffer *circularBuffer);
int isFull(struct circularBuffer * c);
int isEmpty(struct circularBuffer * c);
void enqueue(struct circularBuffer * c, struct timelineResource *n);
struct timelineResource dequeue(struct circularBuffer * c);
struct timelineResource peek(struct circularBuffer * c);
int updateResourceTimeline(struct resource * r, double timestamp, double change, double rateChange);
int updateStateTimeline(struct stateVariable *s, double timestamp, int newState);
void printResourceTimeline(struct resource r);
void printResourceTimelines(struct domain Domain);
void printStateTimeline(struct stateVariable s);
void printStateTimelines(struct domain Domain);


#endif