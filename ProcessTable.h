// Brendan Thompson
// 04/10/17
// ProcessTable.h

// Header file for interacting with the Process Table made of PCBs implemented in ProcessTable.c

#ifndef PTABLE_H
#define PTABLE_H

// Type Definitions
	// Process Table
		typedef struct {
			int limit, protectionKey;
			FNPTR diskAddress;
			// FNPTR pageTableAddress;
		} segInfo;

		typedef struct {
			STRING name;
			FNPTR address;
			FNPTR nextInstruction, lastPC;
			REGTYPE storedRx, storedSx, storedTx, storedUx, storedVx;
			int timeSlice, lastTimeSlice;
			int timeRemaining, lastTimeRemaining;
			segInfo segTable[MAXSEGMENTS];
		} PCB;

// Constructor
	void init_ProcessTable();

// Destructor
	void shutdown_ProcessTable();

// Mutators
	// addProcess = Creates a new Process Control Block for the process table
		// using the current values stored in the virtual shared hardware
	void addPCB();

	// removeProcess = Clears the data from <processName> out of the Table
	void removePCB(STRING processName);

	// loadProcess = Populate the virtual shared hardware with the
		// data for the corresponding <processName>
	void loadPCB(STRING processName);

	// saveProcess = Saves the current values stored in the virtual shared hardware 
		// into the PCB for the corresponding <processName>
	void savePCB(STRING processName);

	// PT_rollbackProcess = resets the PC, TimeSlice, and TimeRemaining to the last saved copy
	void PT_rollbackProcess(STRING processName);

// Accessors
	// getProcessNextPC = returns address of next instruction
	FNPTR PT_getProcessNextPC(STRING processName);

	// getProcessAddress = returns address of first instruction
	FNPTR PT_getProcessAddress(STRING processName);

	int PT_getTimeSlice(STRING processName);

	int PT_getTimeRemaining(STRING processName);

	// getPCB = returns a pointer to the PCB corresponding to <processName> 
	PCB *getPCB(STRING processName);

	BOOL PCBExists(STRING processName);

	void printProcessTable();
#endif