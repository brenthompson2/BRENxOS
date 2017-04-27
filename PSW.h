// Brendan Thompson
// 04/09/17
// PSW.h
//
// Header File for interacting with the Program Status Word Fields in PSW.c

#ifndef PSW_H
#define PSW_H

// Function Declarations

//enableInterrupts (enabled if value = 1)
void enableInterrupts(unsigned value);

//checkEnabledInterrupts (returns 1 if enabled)
unsigned checkEnabledInterrupts();

//userMode (0 = supervisor, 3 = userMode)
void userMode(unsigned value);

//getUserMode (0 = supervisor, 3 = userMode)
unsigned getUserMode();

//getFlag returns the lowest index that contains a 1, else returns 0
unsigned getInterruptFlag();

//getSysCallValue returns value of 4-bit value represented by bits 10-13 (value 0-15)
unsigned getSysCallValue();

//getVirtualMemoryValue returns value of 2-bit value represented by bits 14 & 15 (value 0-3)
unsigned getVirtualMemoryValue();

// setTimer = sets the timer to value
void setTimer(int value);

// getTime = returns the time left in Timer
int getTime();

// setPC
void setPC(FNPTR newPC);

// getPC
FNPTR getPC();

//callPC
void callPC();

// setBit takes in <bArray> and sets the bit at <index> to <bitValue>
	// modeled after Dr. England's
void setBit(unsigned char bArray[], int index, int bitValue);

#endif
