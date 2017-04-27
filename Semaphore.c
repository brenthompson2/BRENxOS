// Brendan Thompson
// 04/09/17
// Semaphore.c

// Linked List Abstract Data Type of a Queue made up of pointers to SemaProcesses

#include "Kernel.h"

// Variables
	static int lockNum;
	static SemaPROCESS headProcess;
	static SemaPROCESS *lastProcess;

// Constructor
	void init_Semaphore(){
		lockNum = 1;
		strcpy(headProcess.name, "Header");
		lastProcess = &headProcess;
		headProcess.nextProcess = 0;
	}

// Destructor
	void shutdown_Semaphore(){
		if (lockNum == 1){
			return;
		}
		int i;
		SemaPROCESS *cursor, *nextCursor;;
		*cursor = headProcess;
		for (i = 0; i > lockNum; i--)
		{
			nextCursor = (*cursor).nextProcess;
			sema_removeProcess((*cursor).name);
			cursor = nextCursor;
		}
		free(cursor);
		return;
	}

// Mutators
	// sema_Wait = Down/P - request access to buffer
	void sema_Wait(STRING processName){
		strcpy(headProcess.name, "Header");
		if (lockNum <= 0){
			// printf("\t[Sema]: Blocking %s...\n", processName);
			Sched_blockProcess(processName, 0);
		}
		// printf("\t[Sema]: Adding %s to Queue...\n", processName);
		sema_addProcess(processName);

		// if this is the first process in the queue then control should be returned
		if (lockNum == 0){
			Sched_returnControl();
		}

	}

	// sema_Signal = Up/V - leave buffer queue
	void sema_Signal(STRING processName){
		// printf("\t[Sema]: Attempting to remove %s...\n", processName);
		sema_removeProcess(processName);
		return;
	}

	void sema_addProcess(STRING processName){
		// create new process
		SemaPROCESS *newProcess = malloc(sizeof(SemaPROCESS));
		strcpy((*newProcess).name, processName);
		// printf("\t[Sema]: Created new SemaProcess %s\n", (*newProcess).name);

		// set nextPointers to and from newProcess
		(*lastProcess).nextProcess = newProcess;
		(*newProcess).nextProcess = 0;
		// printf("\t[Sema]: Added new SemaProcess\n");

		// maintain the queue
		lastProcess = newProcess;
		lockNum--;
		return;
	}

	void sema_removeProcess(STRING processName){
		SemaPROCESS *cursor, *nextCursor;
		STRING tempName;

		// printf("\t[SEMA]: Attempting to remove %s...\n", processName);
		// printSemaQueue();

		// if no processes
		if(lockNum == 1){
			printf("\t[SEMA]: Error During Removal. No Processes\n");
			return;
		}

		// if head process
		cursor = headProcess.nextProcess;
		if(!(strcmp((*cursor).name, processName))){
			//if only process
			if (lockNum == 0) {
				headProcess.nextProcess = 0;
				lastProcess = &headProcess;
			}
			else {
				nextCursor = (*cursor).nextProcess;
				headProcess.nextProcess = nextCursor;
				// printf("\t[Sema]: unBlocking %s...\n", (*nextCursor).name);
				Sched_unBlockProcess((*nextCursor).name, 0);
			}
			free(cursor);
			lockNum++;
			// printf("\t[Sema]: Removed %s\n", processName);
			return;
		}

		// if head was only process
		if (lockNum == 0){
			return;
		}

		// Otherwise locate process ... remains blocked for safety
		int i = -1;
		BOOL found = FALSE;
		nextCursor = (*cursor).nextProcess;
		while (found == FALSE){
			if (i >= lockNum){
				printf("\t[SEMA]: Error During Removal. Unable to Locate Process %s\n", processName);
				return;
			}
			// If found
			// printf("\t[SEMA]: Looking at %d...\n", i);
			strcpy((*nextCursor).name, tempName);
			// printf("\t[SEMA]: Comparing to %s...\n", tempName);
			if (!(strcmp(tempName, processName))){
				// printf("\t[SEMA]: Found It!\n");
				// if last process
				if (!(strcmp((*nextCursor).name, (*lastProcess).name))){
					lastProcess = cursor;
					(*cursor).nextProcess = 0;
				}
				else {
					(*cursor).nextProcess = (*nextCursor).nextProcess;
				}
				free(nextCursor);
				lockNum++;
				// printf("Finished\n");
				return;
			}
			else {
				// printf("\t[SEMA]: Incrementing...\n");
				cursor = nextCursor;
				i--;
			}
			// printf("\t[SEMA]: Trying again...\n");			
		}
		return;
	}

// Accessors
	void printSemaQueue(){
		int i;
		SemaPROCESS *cursor;
		STRING processName;

		if (lockNum == 1){
			printf("\n[Sema]: Queue Empty\n\n");
			return;
		}

		printf("Here 1\n");
		//Print Queue Member Variables
		printf("\nSemaphore Queue:\n");
		printf("\tLock Number = %d\n", lockNum);
		strcpy(processName, (*lastProcess).name);
		printf("\tLast Process = %s\n", processName);

		printf("Here 1.2\n");
		// Print Processes
		cursor = headProcess.nextProcess;
		for (i = 0; i >= lockNum; i--){
			strcpy(processName, (*cursor).name);
			printf("\t%d: %s\n", i, processName);
			cursor = (*cursor).nextProcess;
		}
		printf("\n");

		printf("Here 2\n");
		return;
	}

	BOOL sema_compareNext(STRING processName){
		STRING nextProcess;
		strcpy(nextProcess, (*headProcess.nextProcess).name);
		if (strcmp(nextProcess, processName)){
			return FALSE;
		}
		else {
			return TRUE;
		}
	}