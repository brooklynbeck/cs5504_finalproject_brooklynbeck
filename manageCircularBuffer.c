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

/*  Function initializeCircularBuffer
    Initializes circular buffer data structure for resource and state variable timelines
    Returns the buffer
*/
struct circularBuffer* initializeCircularBuffer(int maxlength, double initialValue, double initialRate){
    struct circularBuffer *newBuffer;
    newBuffer = (struct circularBuffer*)malloc(sizeof(struct circularBuffer));
    newBuffer->head = 0;
    newBuffer->tail = 0;
    newBuffer->maxlen = maxlength;
    newBuffer->buffer = (struct timelineResource*)malloc(sizeof(struct timelineResource)*maxlength);
    for(int i=0;i<maxlength;i++){
        newBuffer->buffer[i].timestamp = 0.0;
        newBuffer->buffer[i].currentValue = 0.0;
        newBuffer->buffer[i].currentRate = 0.0;
    }
    newBuffer->buffer[0].currentValue = initialValue;
    newBuffer->buffer[0].currentRate = initialRate;
    return newBuffer;
}

/*  Function updateCircularBuffer
    Updates the circular buffers after any schedule changes
*/
void updateCircularBuffer(struct schedule *headS, struct domain *Domain){
    struct schedule * temp = headS;
    double simLength = 0;
    
    //dequeue the timelines and initialize with starting values
    for(int i=0;i<Domain->numResources;i++)
    {
        while(Domain->resources[i].timeline->head != Domain->resources[i].timeline->tail)
            dequeue(Domain->resources[i].timeline);
        Domain->resources[i].timeline->buffer[Domain->resources[i].timeline->head].timestamp = 0.0;
        Domain->resources[i].timeline->buffer[Domain->resources[i].timeline->head].currentValue = Domain->resources[i].initialValue;
        Domain->resources[i].timeline->buffer[Domain->resources[i].timeline->head].currentRate = Domain->resources[i].initialRate;
    }
    for(int i=0;i<Domain->numStateVariables;i++)
    {
        while(Domain->stateVariables[i].timeline->head != Domain->stateVariables[i].timeline->tail)
            dequeue(Domain->stateVariables[i].timeline);
        Domain->stateVariables[i].timeline->buffer[Domain->stateVariables[i].timeline->head].timestamp = 0.0;
        Domain->stateVariables[i].timeline->buffer[Domain->stateVariables[i].timeline->head].currentValue = Domain->stateVariables[i].initialState;
    }
    
    //requeue
    while(temp != NULL)
    {
        simLength = temp->startTime + temp->executionTime;
        for (int j=0; j<temp->T.numRR; j++)
        {
            if((temp->T.RR[j].preImpact != 0.0) || (temp->T.RR[j].maintainImpact != 0.0))
                temp->T.RR[j].timelineIndex = updateResourceTimeline(temp->T.RR[j].R, temp->startTime, temp->T.RR[j].preImpact, temp->T.RR[j].maintainImpact);
            if((temp->T.RR[j].postImpact != 0.0)|| (temp->T.RR[j].maintainImpact != 0.0))
                temp->T.RR[j].timelineIndex = updateResourceTimeline(temp->T.RR[j].R, simLength, temp->T.RR[j].postImpact, -1* temp->T.RR[j].maintainImpact);
        }
        for(int j=0; j<temp->T.numSR; j++)
        {
            if(temp->T.SR[j].preImpact != -1)
            {
                temp->T.SR[j].timelineIndex = updateStateTimeline(temp->T.SR[j].SV, temp->startTime, temp->T.SR[j].preImpact);
            }
            if(temp->T.SR[j].maintainImpact != -1)
                temp->T.SR[j].timelineIndex = updateStateTimeline(temp->T.SR[j].SV, temp->startTime, temp->T.SR[j].maintainImpact);
            if(temp->T.SR[j].postImpact != -1)
                temp->T.SR[j].timelineIndex = updateStateTimeline(temp->T.SR[j].SV, simLength, temp->T.SR[j].postImpact);
        }
        temp = temp->next;
    }
    
}

/*  Function freeCircularBuffer
    Frees the memory of a resource or state variable timeline
*/
void freeCircularBuffer(struct circularBuffer *circularBuffer){
    free(circularBuffer->buffer);
    circularBuffer->buffer = NULL;
    free(circularBuffer);
    circularBuffer = NULL;
}

/*  Function isFull
    Checks if a circular buffer is full (head+1=tail)
*/
int isFull(struct circularBuffer * c){
    if (c->head +1 >= c->maxlen)
        return (0==c->tail); //returns 1 if full, 0 if not full
    else
        return (c->head + 1 == c->tail); //returns 1 if full, 0 if not full
}

/*  Function isEmpty
    Checks if a circular buffer is empty (head = tail)
*/
int isEmpty(struct circularBuffer * c){
    return (c->head == c->tail); //returns 1 if empty, 0 if not empty
}

/*  Function Enqueue
    Add a new timestamp to the head of the circular buffer
*/
void enqueue(struct circularBuffer * c, struct timelineResource *n){
    if (isFull(c)==1)
    {
        printf("Enqueuing Error\n"); //signal that there is an error
        abort();
    }
    int next = c->head + 1;
    if (next >= c->maxlen) //check if at the end of the circular buffer
        next = 0;
    c->buffer[next].timestamp = n->timestamp;
    c->buffer[next].currentValue = n->currentValue;
    c->buffer[next].currentRate = n->currentRate;
    c->head = next;
    return;
}

/*  Function dequeue
    Remove a timestamp from the tail of the circular buffer
*/
struct timelineResource dequeue(struct circularBuffer * c){
    struct timelineResource temp;
    if (isEmpty(c)==1)
    {
        printf("Dequeuing Error");
    }
    int tail = c->tail;
    c->tail = tail+1;
    if(c->tail >= c->maxlen)
        c->tail = 0;
    temp.timestamp = c->buffer[tail].timestamp;
    c->buffer[tail].timestamp = 0.0;
    temp.currentValue = c->buffer[tail].currentValue;
    c->buffer[tail].currentValue = 0.0;
    temp.currentRate = c->buffer[tail].currentRate;
    c->buffer[tail].currentRate = 0.0;
    return temp;
}

/*  Function peek
    Returns the timestamp from the tail of the circular buffer without changing anything
    */
struct timelineResource peek(struct circularBuffer * c){
    return c->buffer[c->tail];
}

//TBD!!!! general rates
/*  Function updateResourceTimeline
    Adds a new timestamp to a resource timeline
    Timestamp in minutes, change in watt-hours, rate in watts
    currently only supports power and energy, need to update with general rates
*/
int updateResourceTimeline(struct resource * r, double timestamp, double change, double rateChange){
    struct timelineResource *next;
    next = (struct timelineResource *)malloc(sizeof(struct timelineResource));
    
    int prev = r->timeline->head;
    double prevTimestamp = r->timeline->buffer[prev].timestamp;
    double prevValue = r->timeline->buffer[prev].currentValue;
    double prevRate = r->timeline->buffer[prev].currentRate;
    
    next->timestamp = timestamp;
    next->currentValue = change + prevValue + (timestamp - prevTimestamp)*prevRate/60; //minutes * watts * hours/minutes = watt-hours
    next->currentRate = prevRate + rateChange;
    
    enqueue(r->timeline, next);
    free(next);
    return prev+1; //timelineIndex
}

/*  Function updateStateTimeline
    Adds a new timestamp to a state timeline
*/
int updateStateTimeline(struct stateVariable *s, double timestamp, int newState){
    
    struct timelineResource *next;
    next = (struct timelineResource *)malloc(sizeof(struct timelineResource));
    int prev = s->timeline->head;
    //double prevTimestamp = s->timeline->buffer[prev].timestamp;
    int prevState = s->timeline->buffer[prev].currentValue;
    
    if(newState != prevState)
    {
        //printf("test state queing\n");
        next->timestamp = timestamp;
        next->currentValue = newState;
        enqueue(s->timeline, next);
    }

    free(next);
    return prev+1; //timelineIndex
}

/*  Function printResourceTimeline
    Print a resource Timeline
*/
void printResourceTimeline(struct resource r)
{
    printf("%s Timeline\n", r.name);
    int i = r.timeline->tail;
    int head = r.timeline->head;
    //int max = r.timeline->maxlen;
    struct timelineResource temp;
    
    do
    {
        temp = r.timeline->buffer[i];
        printf("%lf: %lf %s, %lf %s/s \n", temp.timestamp, temp.currentValue, r.unit, temp.currentRate, r.unit);
        
        i+=1;
    }
    while(i-1 != head);
}

/*  Function printResourceTimelines
    Prints all resource timelines
*/
void printResourceTimelines(struct domain Domain)
{
    for (int i = 0; i < Domain.numResources; i++)
    {
        printResourceTimeline(Domain.resources[i]);
    }
}

/*  Function printStateTimeline
    Prints a state variable timeline
*/
void printStateTimeline(struct stateVariable s)
{
    printf("%s Timeline\n", s.name);
    int i = s.timeline->tail;
    int head = s.timeline->head;
    //int max = s.timeline->maxlen;
    struct timelineResource temp;
    do
    {
        temp = s.timeline->buffer[i];
        printf("%lf: %s\n", temp.timestamp, s.states[(int) temp.currentValue]);
        
        i+=1;
    }
    while(i-1 != head);
}

/*  Function printStateTimelines
    Prints all state variable timelines
*/
void printStateTimelines(struct domain Domain)
{
    for (int i = 0; i < Domain.numStateVariables; i++)
    {
        printStateTimeline(Domain.stateVariables[i]);
    }
}
