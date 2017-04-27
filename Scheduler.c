// Brendan Thompson
// 04/13/17
// Scheduler.c

// Linked List Abstract Data Type of a Queue made up of pointers to Processes

#include "Kernel.h"

// Member Variables (Static = local to this file)
static PROCESS headProcess;
static PROCESS *lastProcess;
static int numProcesses;
static BOOL repeatProcess;

// Constructor
	void init_Scheduler(){
		numProcesses = 0;
		repeatProcess = FALSE;
		strcpy(headProcess.name, "Header");
		lastProcess = &headProcess;
		headProcess.nextProcess = 0;
		return;
	}

// Destructor
	void shutdown_Scheduler(){
		if (numProcesses == 0){
			return;
		}
		int i;
		PROCESS *cursor, *nextCursor;

		*cursor = headProcess;
		for (i = 0; i <= numProcesses; i++)
		{
			nextCursor = (*cursor).nextProcess;
			Sched_removeProcess((*cursor).name);
			cursor = nextCursor;
		}
		free(cursor);
		return;
	}

// Mutators
	// Sched_addProcess = pushes a process to the back of the Scheduling Queue
	void Sched_addProcess(STRING processName){
		// create new process
		PROCESS *newProcess = malloc(sizeof(PROCESS));
		strcpy((*newProcess).name, processName);
		(*newProcess).blocked = FALSE;

		// set nextPointers to and from newProcess
		(*lastProcess).nextProcess = newProcess;
		(*newProcess).nextProcess = 0;

		// maintain the queue
		numProcesses++;
		lastProcess = newProcess;
	}

	// Sched_removeProcess = removes a process from the Scheduling Queue
	void Sched_removeProcess(STRING processName){
		PROCESS *cursor, *nextCursor;

		// if no processes
		if(numProcesses == 0){
			printf("\tError During Removal: No Processes\n");
			return;
		}
		
		// if first process
		cursor = headProcess.nextProcess;
		if(!(strcmp((*cursor).name, processName))){
			// if only process
			if (numProcesses == 1){
				headProcess.nextProcess = 0;
			}
			else {
				headProcess.nextProcess = (*cursor).nextProcess;
			}
			printf("\t[Sched]: Removed %s\n", processName);
			free(cursor);
			numProcesses--;
			return;
		}

		// locate process
		int i = 2;
		nextCursor = (*cursor).nextProcess;
		while (i <= numProcesses){
			if (!(strcmp((*nextCursor).name, processName))){
				(*cursor).nextProcess = (*nextCursor).nextProcess;
				free(nextCursor);
				numProcesses--;
				printf("\t[Sched]: Removed %s\n", processName);
				return;
			}
			else {
				cursor = nextCursor;
				nextCursor = (*cursor).nextProcess;
				i++;
			}
		}
		printf("\tError During Removal: Unable to Locate Process %s\n", processName);
		return;
	}
	
	// Sched_nextProcess = sets *nameProcess to the next unblocked process in the Queue
		// pops old process off queue and pushes it to the back
	void Sched_nextProcess(STRING *nameProcess){
		PROCESS *tempProcess;
		int i = 1;

		if (repeatProcess){
			strcpy(*nameProcess, (*headProcess.nextProcess).name);
			repeatProcess = FALSE;
			return;
		}

		if (numProcesses == 0){
			strcpy(*nameProcess, "N/A");
			// printf("\t[SCHED]: No Processes\n");
			return;
		}

		if (numProcesses == 1){
			strcpy(*nameProcess, (*headProcess.nextProcess).name);
			// printf("\t[SCHED]: Only One Process\n");
			return;
		}

		while (i <= numProcesses){
			// Pop first and Push to back
			tempProcess = headProcess.nextProcess;
			headProcess.nextProcess = (*tempProcess).nextProcess;
			(*lastProcess).nextProcess = tempProcess;
			lastProcess = tempProcess;

			//Check if blocked
			if (!(*headProcess.nextProcess).blocked){
				strcpy(*nameProcess, (*headProcess.nextProcess).name);
				return;
			}
			i++;
		}

		printf("\t[SCHED]: ERROR! All Processes Blocked\n");
		return;
	}

	// Sched_blockProcess = blocks process, type 0 = OB, type 1 = Mem
	void Sched_blockProcess(STRING processName, int type){
		PROCESS *cursor;
		cursor = headProcess.nextProcess;
		int i = 1;
		BOOL found = FALSE;

		// locate process
		while (found == FALSE){
			if (i > numProcesses){
				printf("\tError Blocking Process: Unable to Locate Process %s\n", processName);
				return;
			}
			// printf("\t\t[Sched]: Checking Process %s...\n", (*cursor).name);
			if (!(strcmp((*cursor).name, processName))){
				if (type == 0){
					(*cursor).OBblock = TRUE;
				}
				if (type == 1){
					(*cursor).memBlock = TRUE;
				}
				(*cursor).blocked = TRUE;
				// printf("\t\t[Sched]: Blocked Process %s\n", (*cursor).name);
				return;
			}
			else {
				cursor = (*cursor).nextProcess;
				i++;
			}
		}
	}

	// Sched_unBlockProcess = unblocks process, type 0 = OB, type 1 = Mem
	void Sched_unBlockProcess(STRING processName, int type){
		PROCESS *cursor;
		cursor = headProcess.nextProcess;
		int i = 1;
		BOOL found = FALSE;

		// locate process
		while (found == FALSE){
			if (i > numProcesses){
				printf("\tError unBlocking Process: Unable to Locate Process %s\n", processName);
				return;
			}
			if (!(strcmp((*cursor).name, processName))){
				if (type == 0){
					(*cursor).OBblock = FALSE;
				}
				if (type == 1){
					(*cursor).memBlock = FALSE;
				}
				if ((!(*cursor).OBblock) && (!(*cursor).memBlock)){
					(*cursor).blocked = FALSE;
				}
				return;
			}
			else {
				cursor = (*cursor).nextProcess;
				i++;
			}
		}
	}

	// Sched_rollbackProcess = sets the scheduling queue back one so that the current process gets called again
	void Sched_returnControl(){
		repeatProcess = TRUE;
		return;
	}

// Accessors
	void Sched_getCurrentProcess(STRING *processName){
		strcpy(*processName, (*headProcess.nextProcess).name);
		return;
	}

	int getNumProcesses(){
		return numProcesses;
	}

	void printSchedQueue(){
		int i;
		PROCESS *cursor;
		STRING processName;
		FNPTR processAddress, processNextPC;
		cursor = headProcess.nextProcess;

		//Print Queue Members
		if (numProcesses == 0){
			printf("\n[Sched]: Queue Empty\n");
			return;

		}
		printf("\nScheduling Queue: (%d Processes)\n", numProcesses);
		strcpy(processName, (*lastProcess).name);
		printf("\tLast Process = %s\n", processName);
		//printf("\tAddress of Head = %p\n", &headProcess);

		// Print Processes
		for (i = 1; i <= numProcesses; i++){
			strcpy(processName, (*cursor).name);
			//processAddress = PT_getProcessAddress(processName);
			//processNextPC = PT_getProcessNextPC(processName);
			if ((*cursor).blocked){
				printf("\t%d: %s \t(%p) (BLOCKED)\n", i, processName, processAddress);
			}
			else {
				printf("\t%d: %s \t(%p)\n", i, processName, processAddress);
			}
			
			cursor = (*cursor).nextProcess;
		}
		printf("\n");
		return;
	}

	// Sched_checkBlock = returns TRUE if blocked
	BOOL Sched_checkBlock(STRING processName){
		PROCESS *cursor;
		cursor = headProcess.nextProcess;
		int i = 1;
		BOOL found = FALSE;

		// locate process
		while (found == FALSE){
			if (i > numProcesses){
				printf("\tError Checking Block: Unable to Locate Process %s\n", processName);
				exit(1);
			}
			if (!(strcmp((*cursor).name, processName))){
				return (*cursor).blocked;
			}
			else {
				cursor = (*cursor).nextProcess;
				i++;
			}
		}
	}