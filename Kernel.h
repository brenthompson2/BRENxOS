// Brendan Thompson
// 04/09/17
// Kernel.h

#ifndef KERN_H
#define KERN_H

// Libraries
	#include <stdio.h> //for printf()
	#include <stdlib.h> //for free()
	#include <string.h> //for strcpy() and strcmp()

// Constants
	#define BYTE_BITS 8
	#define DEFAULTTIMESLICE 128
	#define TOTALTIME 4096
	#define MAXPROCESSES 25
	#define MAXSEGMENTS 8
	#define MAXFRAMES 8
	// #define MAXPAGES 16
	#define FALSE 0
	#define TRUE 1

// Type Definitions
	typedef char 	STRING[20];
	typedef int 	BOOL;

// Structs
	#include "moses_m.h"
	#include "ProcessTable.h"
	#include "PSW.h"
	#include "MemoryManager.h"
	#include "Scheduler.h"
	#include "Semaphore.h"

#endif