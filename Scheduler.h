// Brendan Thompson
// 03/06/2017
// Scheduler.c

// Header File for interacting with the Scheduling Queue implemented in Scheduler.c

#ifndef SCHED_H
#define SCHED_H

// Type Definitions
	typedef struct PROCESS {
		STRING name;
		BOOL blocked, OBblock, memBlock;
		struct PROCESS *nextProcess;
	} PROCESS;

// Function Declarations

// Constructor
	void init_Scheduler();

// Destructor
	void shutdown_Scheduler();

// Manipulators
	// Sched_addProcess = adds a process to the Scheduling Queue
		// Pre-Condition: the process must already have a corresponding PCB
	void Sched_addProcess(STRING processName);

	// Sched_removeProcess = removes a process from the scheduling Queue
	void Sched_removeProcess(STRING processName);
	
	// Sched_nextProcess = sets *nameProcess to the next unblocked process in the Queue
		// and manipulates the Queue
	void Sched_nextProcess(STRING *nameProcess);

	void Sched_blockProcess(STRING processName, int type);

	void Sched_unBlockProcess(STRING processName, int type);

	// Sched_returnControl = makes it so that the current process gets repeated
	void Sched_returnControl();

// Accessors
	void Sched_getCurrentProcess(STRING *processName);
	
	int getNumProcesses();

	void printSchedQueue();

	BOOL Sched_checkBlock(STRING processName);

#endif