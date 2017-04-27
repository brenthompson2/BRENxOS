// Brendan Thompson
// 03/06/16
// Semaphore.h

// Header File for interacting with the Semaphore Queue implemented in Semaphore.c

#ifndef SEMA_H
#define SEMA_H

// Type Definitions
	typedef struct SemaPROCESS{
		STRING name;
		struct SemaPROCESS *nextProcess;
	} SemaPROCESS;

// Constructor
	void init_Semaphore();

// Destructor
	void shutdown_Semaphore();

// Mutators
	// sema_Wait = Down/P - request access to buffer
	void sema_Wait(STRING processName);

	// sema_Signal = Up/V - leave buffer
	void sema_Signal(STRING processName);

	void sema_addProcess(STRING processName);

	void sema_removeProcess(STRING processName);

// Accessors
	void printSemaQueue();

	BOOL sema_compareNext(STRING processName);

#endif