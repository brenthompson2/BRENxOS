// Brendan Thompson
// 04/09/17
// Kernel.c
//
// Kernel that mimcs an OS handling Process Scheduling, Interupts, and System Calls

// Header Files
	#include "Kernel.h"

#define DEBUG

// *** Function Declarations *** 

	// handleInterrupt = Checks the flags and handles the interrupt appropriately
	void handleInterrupt(STRING processName);

	// handleSystemCall = Checks the Sys Call Values and handles the Sys Call appropriately
	void handleSystemCall(STRING processName);

	// terminateProcess = Calls the functions necessary to properly terminate a process
	void terminateProcess(STRING processName);

	void printRegisters();


int main (){
	printf("\n\nWelcome to BRENxOS V2.5!\n");

	// Initialize System
	init_moses();
	init_ProcessTable();
	init_Scheduler();
	init_Semaphore();
	init_MemMan();
	enableInterrupts(TRUE);
	userMode(0); // (0 = Supervisor | 3 = User)

	segDump(0);
	vmemDump();

	// Prepare for Processes
	STRING theName;
	STRING *processName;
	processName = &theName;
	FNPTR fnToCall;
	int numProcesses, timeRemaining;
	int flagNumber;

	// Get First Process
	addPCB();
	printProcessTable();
	Sched_addProcess(Ux);
	printSchedQueue();
	printSharedSegs();
	Sched_getCurrentProcess(processName);
	printf("[BRENxOS]: The First Process is: %s\n", *processName); 
	numProcesses = getNumProcesses();
	fnToCall = Vx;

	// Handle Processes
	while (numProcesses > 0){
		// Load Process
		// printf("[BRENxOS]: Loading Process %s...\n", *processName); 
		loadPCB(*processName);

		// Call Process
		printf("[BRENxOS]: Calling %s...\n", *processName);
		callPC();
		printf("[BRENxOS]: Returned From %s\n", *processName);

		// Save Process
		savePCB(*processName);
		printProcessTable();
		// printf("[BRENxOS]: Saved PCB for %s\n", *processName);

		// Handle Interrupts
		if (checkEnabledInterrupts()){
			handleInterrupt(*processName);
		}
		// printf("[BRENxOS]: Handled the interrupt\n");

		// Print Current Status
		printSharedSegs();
		printSchedQueue();
		printSemaQueue();
		printFrameTable();

		// get Next Process
		Sched_nextProcess(processName);

		// update numProcesses
		numProcesses = getNumProcesses();
		if (numProcesses > 0){
			printf("[BRENxOS]: The Next Process Will Be: %s\n", *processName);
			// printf("[BRENxOS]: There are %d Process\n", numProcesses); 
		}
	}
	printf("[BRENxOS]: Done Processing! Shutting Down...\n"); 

	userMode(0);
	iolog();
	segDump(0);
	vmemDump();
	
	// Shut Down System
	shutdown_Scheduler();
	shutdown_ProcessTable();
	shutdown_Semaphore();
	printf("\nThank you for testing BRENxOS V2.5!\n\nShutting Down...\n");
}

// *** Function Definitions ***

// handleInterrupt = Checks the flags and handles the interrupt appropriately
void handleInterrupt(STRING processName){
	int flagNumber;
	flagNumber = getInterruptFlag();
	switch (flagNumber){
		case 5:
			printf("[BRENxOS]: Interrupt 5: Process %s complete. Removing %s...\n", processName, processName);
			terminateProcess(processName);
			return;
		case 6:
			printf("[BRENxOS]: Interrupt 6: Process %s Threw an Exception. Removing %s...\n", processName, processName);
			terminateProcess(processName);
			return;
		case 8:
			printf("[BRENxOS]: Interrupt 8: Timer Finished\n");
			int timeRemaining = PT_getTimeRemaining(processName);
			//printf("[BRENxOS]: Got Time Remaining\n");
			if (timeRemaining <= 0){
				printf("[BRENxOS]: Process %s is out of time. Removing %s...\n", processName, processName);
				terminateProcess(processName);
			}
			return;
		case 9:
			printf("[BRENxOS]: Interrupt 9: System Call\n");
			handleSystemCall(processName);
			return;
	}
}

// handleSystemCall = Checks the Sys Call Values and handles the Sys Call appropriately
void handleSystemCall(STRING processName){
	int sysCallValue;
	sysCallValue = getSysCallValue();
	switch (sysCallValue){
		case 3:
			printf("[BRENxOS]: SysCall 3: Virtual Memory Access...\n");

        	//printf("[BRENxOS]:  Tx Holds-->%s<--\n", (char *) Tx);
			handleVirtualMemoryAccess(processName);
			return;
		case 5:
			printf("[BRENxOS]: SysCall 5: Initiating New Process...\n");
			if (PCBExists(Ux)){
				printf("\t[BRENxOS]: Error: %p already exists\n", Ux);
				Sched_returnControl();
				return;
			}
			addPCB();
			Sched_addProcess(Ux);
			return;
		case 8:
			printf("[BRENxOS]: SysCall 8: Process %s Requests Access to Output Buffer...\n", processName);
			enableInterrupts(FALSE);
			sema_Wait(processName);
			enableInterrupts(TRUE);
			// printf("[BRENxOS]: Handled Sys 8!\n");
			return;
		case 9:
			printf("[BRENxOS]: SysCall 9: Process %s Writing to Output Buffer...\n", processName);
			userMode(0);
			io((char *) Ux);
			Sched_returnControl();
			// userMode(3);
			// printf("[BRENxOS]: Handled Sys 9!\n");
			return;
		case 10:
			printf("[BRENxOS]: SysCall 10: Process %s Completed Access to Output Buffer\n", processName);
			enableInterrupts(FALSE);
			sema_Signal(processName);
			userMode(0);
			iofl();
			iolog();
			Sched_returnControl();
			// userMode(3);
			enableInterrupts(TRUE);
			// printf("[BRENxOS]: Handled Sys 10!\n");
			return;
	}
}

// terminateProcess = Calls the functions necessary to properly terminate a process
void terminateProcess(STRING processName){
	printf("[BRENxOS]: Terminating %s..\n", processName);
	sema_removeProcess(processName);
	// printf("[BRENxOS]: Removed from Sema\n");
	removePCB(processName); // removes from Process Table
	// printf("[BRENxOS]: Removed from PT\n");
	Sched_removeProcess(processName);
	// printf("[BRENxOS]: Removed from Sched\n");
	mem_removeProcess(processName);
	printf("[BRENxOS]: Terminated Process %s\n", processName);
}

void printRegisters(){
	printf("Rx: %p\nSx: %p\nTx: %p\nUx: %p\nVx: %p\n", Rx, Sx, Tx, Ux, Vx);
}