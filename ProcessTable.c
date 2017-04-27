// Brendan Thompson
// 04/10/17
// ProcessTable.c

// Abstract Data Type that stores the data from the shared hardware of each process 
	// into a corresponding Process Control Block (PCB)

// Header Files
	#include "Kernel.h"

// Member Variables (Static = local to this file)
	static PCB* ProcessTable[MAXPROCESSES];
	int numPCBs;

// Constructor
	void init_ProcessTable (){
		int i;
		numPCBs = 0;
		for (i = 0; i < MAXPROCESSES; i++){
			ProcessTable[i] = 0;
		}
	}

// Destructor
	void shutdown_ProcessTable(){
		if (numPCBs == 0){
			return;
		}
		int i;
		for (i = 0; i < numPCBs; i++){
			free(ProcessTable[i]);
			numPCBs--;
		}
		return;
	}


// Mutators
	// addPCB = Creates a new Process Control Block for the process table
		// using the current values stored in the virtual shared hardware
	void addPCB(){
		PCB *newPCB = malloc(sizeof(PCB));
		STRING processName;
		int i;
		strcpy(processName, Ux);
		printf("\t[PT]: Creating Process %s...\n", processName);

		// Populate PCB Registers
		strcpy((*newPCB).name, processName);
		(*newPCB).address = Vx;
		(*newPCB).nextInstruction = Vx;
		(*newPCB).storedRx = Rx;
		(*newPCB).storedSx = Sx;
		(*newPCB).storedTx = Tx;
		(*newPCB).storedUx = Ux;
		(*newPCB).storedVx = Vx;
		(*newPCB).timeRemaining = TOTALTIME;

		// Populate Segment Table
		for (i = 0; i < MAXSEGMENTS; i++){
			((*newPCB).segTable[i]).limit = 0;
		}
		mem_addProcess(processName, (*newPCB).segTable);

		// Store PCB
		numPCBs++;
		ProcessTable[numPCBs] = newPCB;
		strcpy(processName, ((*ProcessTable[numPCBs]).name));
		printf("\t[PT]: Created Process %s\n", processName);
		return;
	}

	// removePCB = Clears the data from <processName> out of the Table
	void removePCB(STRING processName){
		int index = 1;
		BOOL match = FALSE;
		PCB *tempPCB;
		STRING tempName;

		// Locate the PCB
		while(index <= numPCBs){
			tempPCB = ProcessTable[index];
			strcpy(tempName, (*tempPCB).name);
			if (!(strcmp(tempName, processName))){
				free(ProcessTable[index]);
				ProcessTable[index] = 0;

				// Condense Table
				for (index; index <= numPCBs; index++){
					ProcessTable[index] = ProcessTable[index + 1];
				}
				numPCBs--;
				printf("\t[PT]: Removed %s\n", processName);
				return;
			}
			index++;
		}
	}

	// loadPCB = Populate the virtual shared hardware with the
		// PCB data for the corresponding <processName>
	void loadPCB(STRING processName){
		PCB* currentPCB;
		FNPTR fnToCall;
		int timeSlice;
		currentPCB = getPCB(processName);

		// Populate the Registers
		Rx = (*currentPCB).storedRx;
		Sx = (*currentPCB).storedSx;
		Tx = (*currentPCB).storedTx;
		Ux = (*currentPCB).storedUx;
		Vx = (*currentPCB).storedVx;
		// printf("\t[PT]: Loaded Registers\n");

		// Set the Timer
		timeSlice = PT_getTimeSlice(processName);
		// printf("\t[PT]: %s's timeSlice was: %d\n", processName, timeSlice);
		if (timeSlice == 0){
			timeSlice = PT_getTimeRemaining(processName);
			if (timeSlice > DEFAULTTIMESLICE){
				timeSlice = DEFAULTTIMESLICE;
			}
		}
		setTimer(timeSlice);
		(*currentPCB).timeSlice = timeSlice;
		// printf("\t[PT]: Set %s's timeSlice to %d\n", processName, timeSlice);
		
		// Set the PC
		fnToCall = PT_getProcessNextPC(processName);
		// printf("\t[PT]: %s's PC will be %p\n", processName, fnToCall);
		setPC(fnToCall);
		// printf("\t[PT]: Set %s's PC\n", processName);

		// Save Last Values
		(*currentPCB).lastTimeSlice = timeSlice;
		(*currentPCB).lastTimeRemaining = PT_getTimeRemaining(processName);
		(*currentPCB).lastPC = fnToCall;

		return;
	}

	// savePCB = Saves the current values stored in the virtual shared hardware 
			// into the PCB for the corresponding <processName>
	void savePCB(STRING processName){
		PCB *currentPCB;
		currentPCB = getPCB(processName);
		int oldTime, newTime, changeInTime;

		// Update PCB
		(*currentPCB).nextInstruction = getPC();
		(*currentPCB).storedRx = Rx;
		(*currentPCB).storedSx = Sx;
		(*currentPCB).storedTx = Tx;
		(*currentPCB).storedUx = Ux;
		(*currentPCB).storedVx = Vx;

		// Update Timers
		oldTime = PT_getTimeSlice(processName);
		// printf("\t[PT]: %s's timeSlice was %d\n", processName, oldTime);
		// printf("\t[PT]: %s's timeRemaining was %d\n", processName, (*currentPCB).timeRemaining);
		newTime = getTime();
		(*currentPCB).timeSlice = newTime;
		changeInTime =  oldTime - newTime;
		// printf("\t[PT]: %s's changeInTime was %d\n", processName, changeInTime);
		(*currentPCB).timeRemaining = (*currentPCB).timeRemaining - changeInTime;
		// printf("\t[PT]: %s's timeSlice is now %d\n", processName, oldTime);
		// printf("\t[PT]: %s's timeRemaining is now %d\n", processName, (*currentPCB).timeRemaining);
	}

	// PT_rollbackProcess = resets the PC, TimeSlice, and TimeRemaining to the last saved copy
	void PT_rollbackProcess(STRING processName){
		PCB *tempPCB;
		tempPCB = getPCB(processName);

		(*tempPCB).nextInstruction = (*tempPCB).lastPC;
		(*tempPCB).timeSlice = (*tempPCB).lastTimeSlice;
		(*tempPCB).timeRemaining = (*tempPCB).lastTimeRemaining;
	}
	
// Accessors
	// getProcessNextPC = returns address of next instruction
	FNPTR PT_getProcessNextPC(STRING processName){
		PCB *tempPCB;
		FNPTR nextPC;
		// printf("\t\t[PT]: Looking for %s\n", processName);
		tempPCB = getPCB(processName);
		// printf("\t\t[PT]: Found %s\n", (*tempPCB).name);
		nextPC = (*tempPCB).nextInstruction;
		// printf("\t\t[PT]: Next PC: %p\n", nextPC);
		return nextPC;
	}

	// getProcessAddress = returns address of first instruction
	FNPTR PT_getProcessAddress(STRING processName){
		FNPTR processAddress;
		PCB *tempPCB;
		tempPCB = getPCB(processName);
		processAddress = (*tempPCB).address;
		// printf("Process Address: %p\n", processAddress);
		return processAddress;
	}

	int PT_getTimeSlice(STRING processName){
		PCB *tempPCB;
		tempPCB = getPCB(processName);
		return (*tempPCB).timeSlice;
	}

	int PT_getTimeRemaining(STRING processName){
		PCB *tempPCB;
		tempPCB = getPCB(processName);
		return (*tempPCB).timeRemaining;
	}

	// getPCB = returns a pointer to the PCB corresponding to <processName> 
	PCB *getPCB(STRING processName){
		int index = 1;
		BOOL match = FALSE;
		PCB *tempPCB;
		STRING tempName;

		// Locate the PCB
		while ((index <= numPCBs) && (match == FALSE)){
			tempPCB = ProcessTable[index];
			strcpy(tempName, (*tempPCB).name);
			// printf("tempName: ->%s<-\n", tempName);
			// printf("processName: ->%s<-\n", processName);
			if (!(strcmp(tempName, processName))){
				match = TRUE;
			}
			else{
				index++;
			}
		}

		if (match == FALSE){
			printf("\t[PT]: Error! Unable to Locate %s\n", processName);
			exit(1);
		}
		return tempPCB;
	}

	BOOL PCBExists(STRING processName){
		int index = 1;
		BOOL match = FALSE;
		PCB *tempPCB;
		STRING tempName;

		if (numPCBs == 0){
			return FALSE;
		}

		// Locate the PCB
		while (index <= numPCBs){
			tempPCB = ProcessTable[index];
			strcpy(tempName, (*tempPCB).name);
			// printf("tempName: ->%s<-\n", tempName);
			// printf("processName: ->%s<-\n", processName);
			if (!(strcmp(tempName, processName))){
				return TRUE;
			}
			else{
				index++;
			}
		}

		return FALSE;
	}


	void printProcessTable(){
		if (numPCBs == 0){
			printf("\n[PT]: Process Table Empty\n");
			return;
		}

		int i, segCounter;
		PCB *tempPCB;
		STRING processName, valueUx;

		printf("\nProcess Table: (%d PCBs)\n", numPCBs);
		for(i = 1; i <= numPCBs; i++){
			tempPCB = ProcessTable[i];
			strcpy(processName, (*tempPCB).name);
			//strcpy(valueUx, (*tempPCB).storedUx);
			printf("   %s:\n", processName);
			printf("\tAddress Process: %p\n", (*tempPCB).address);
			printf("\tAddress Next Instruction: %p\n", (*tempPCB).nextInstruction);
			printf("\tTime Slice: %d\tTime Remaining: %d\n", (*tempPCB).timeSlice, (*tempPCB).timeRemaining);
			printf("\tRegisters:\n\t\tRx: %p\tSx: %p\tTx: %p\tUx: %p\tVx: %p\n", 
				(*tempPCB).storedRx,
				(*tempPCB).storedSx,
				(*tempPCB).storedTx,
				(*tempPCB).storedUx,
				(*tempPCB).storedVx
			);
			
			// Print Seg Table
			segCounter = 0;
			if (((*tempPCB).segTable[segCounter]).limit == 0){
				printf("\tSegment Table Empty\n");
			}
			else{
				printf("\tSegment Table:\n");
				while (((*tempPCB).segTable[segCounter]).limit != 0){
					printf("\t\t%d:\tLimit: %d\tProtection Key: %d\tDisk Address: %p\n", 
						segCounter,
						((*tempPCB).segTable[segCounter]).limit,
						((*tempPCB).segTable[segCounter]).protectionKey,
						((*tempPCB).segTable[segCounter]).diskAddress
					);
					segCounter++;
				}
			}


		}
		printf("\n");
		return;
	}