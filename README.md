# cs5504_finalproject_brooklynbeck

All files and code related to the final class project

"Analyzing Surrogate Satellite Workload Performance on Simulated Computer Architectures"

To reproduce results, compile the workloads and simulation code as written below

  gcc -O0 -ggdb3 -std=c99 -static main.c helpers.c initializeDomain.c initializeSchedule.c iterativeRepair.c manageCircularBuffer.c manageLog.c manageSchedule.c manageTasks.c montecarlosimulation.c -o gerry -lm

  gcc -O0 -fopenmp -static -march=x86-64 -mtune=generic independent_writes.c -o independent_writes

  gcc -O0 -ggdb3 -std=c99 -static densemv.c -o dense
 
  chmod +x run_sat_tests.sh

Note that two of the workloads, independent_writes and dense, are reused from supplied workloads from class homework assignments

Then, run the simulation code as written below

  ./run_sat_tests.sh

This project requires the updates to gem5 installed in Homework 9, such that gem5 can simulate multicore architectures.

The files included in this github repository are:

  The files for the workload 'gerry' that simulates satellite onboard schedule repair, including the following: 

    main.c 

    helpers.c 

    initializeDomain.c 

    initializeSchedule.c 

    iterativeRepair.c 
 
    manageCircularBuffer.c 
 
    manageLog.c 
 
    manageSchedule.c 

    manageTasks.c 
 
    montecarlosimulation.c

  The simulation script to run all three workloads through two architectures and store the stats:

    run_sat_tests.sh

  Stat files from each simulation:
  
    stats_rpi5_dense
 
    stats_rpi5_independent_writes
  
    stats_rpi5_gerry
 
    stats_jestson_dense
  
    stats_jetson_independent_writes
 
    stats_jetson_gerry

  And the file for the final paper:

    finalProject-brooklynbeck.pdf
  
