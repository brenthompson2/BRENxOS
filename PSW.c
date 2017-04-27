// Brendan Thompson
// 04/09/17
// PSW.c
//
// Functions for interacting with the Program Status Word Fields

// PSW Bit-Map:

	// 0-1 = Supervisor Mode (00 = Supervisor, 11 = User)

	// 2 = 	Interrupt Mask 

	// 5-9 = Interrupt Flags
		// 5 = 	Termination
		// 6 =	Exception
		// 8 =	Timer
		// 9 =	System Call

	// 10-13 = System Call Types
		// 3 = Virtual Memory Access
		// 5 = New Process
		// 8 = Rquest Access to Output Buffer
		// 9 = Write to Output Buffer
		// 10 = Completed Accessing Output Buffer

	// 14-15 = Virtual Memory Suites
		// 0 = Request Vmem fetch/write (Shared Segments)
		// 1 = Vmem fetch
		// 2 = Vmem Write
		// 3 = End Vmem fetch/write (Shared Segments)

	// 16-31 = Time Slice

	// x32->64 or x64->127 = PC (Second Fnptr)

// Header Files
	#include "Kernel.h"

// Function Definitions

//enableInterrupts (enabled if value = 1) (bit 2)
void enableInterrupts(unsigned value){
	if (value)
		setBit((unsigned char *) PSW, 2, 1);
	else
		setBit((unsigned char *) PSW, 2, 0);
	return;
}

//checkEnabledInterrupts (returns 1 if enabled) (bit 2)
unsigned checkEnabledInterrupts(){
	unsigned char value, mask;
	value = ((unsigned char *) PSW)[0];
	mask = (unsigned char) (1 << (BYTE_BITS - 3));
	value &= mask;
	if (value)
		return 1;
	else
		return 0;
}

//userMode (0 = supervisor, 3 = userMode) (bits 0-1)
void userMode(unsigned value){
	if (value == 0){
		setBit((unsigned char *) PSW, 0, 0);
		setBit((unsigned char *) PSW, 1, 0);
	}
	if (value == 1){
		printf("\t[PSW]: Error: Invalid UserMode\n");
	}
	if (value == 2){
		printf("\t[PSW]: Error: Invalid UserMode\n");
	}
	if (value == 3){
		setBit((unsigned char *) PSW, 0, 1);
		setBit((unsigned char *) PSW, 1, 1);
	}
	return;
}

//getUserMode (0 = supervisor, 3 = userMode) (bits 0-1)
unsigned getUserMode(){
	unsigned char value, mask;
	value = ((unsigned char *) PSW)[0];
	mask = (unsigned char) (3 << (BYTE_BITS - 2));
	value &= mask;
	value >>= (BYTE_BITS - 2);
	if ((value == 1)||(value == 2)){
		printf("\tError: Invalid Value for UserMode\n");
	}
	return mask;
}

//getInterruptFlag returns the lowest index that contains a 1, else returns 0 (bits 5-9)
unsigned getInterruptFlag(){
	unsigned char value, mask;

	//check bits 5,6,7
	value = ((unsigned char *) PSW)[0];
	mask = (unsigned char) 7;
	value &= mask;
	if (value > 3)
		return 5;
	if(value > 1)
		return 6;
	if (value == 1)
		return 7;
	
	//check bits 8 and 9
	value = ((unsigned char *) PSW)[1];
	mask = (unsigned char) (3 << (BYTE_BITS - 2));
	value &= mask;
	if (value > 127)
		return 8;
	if (value > 63)
		return 9;
	if (!value)
		return 0;
}

//getSysCallValue returns value of 4-bit value represented by bits 10-13 (value 0-15)
unsigned getSysCallValue(){
	unsigned char value, mask;
	value = ((unsigned char *) PSW)[1];

	//check bits 5,6,7
	mask = (unsigned char) (15 << (BYTE_BITS - 6));
	value &= mask;
	value >>= (BYTE_BITS - 6);
	return value;
}

//getVirtualMemoryValue returns value of 2-bit value represented by bits 14 & 15 (value 0-3)
unsigned getVirtualMemoryValue(){
	unsigned char value, mask;
	value = ((unsigned char *) PSW)[1];

	//check bits 5,6,7
	mask = (unsigned char) 0x03;
	value &= mask;
	return value;
}

// setTimer = sets the timer to value (Bits 16-31)
void setTimer(int value){
	((short *) PSW)[1] = value;
	return;
}

// getTime = returns the time left in Timer (Bits 16-31)
int getTime (){
	short *time;
	time = (((short *) PSW)+1);
	return *time;
}

// setPC (second half of PSW)
void setPC(FNPTR newPC){
	*(((FNPTR *) PSW)+1) = newPC;
	return;
}

// getPC (second half of PSW)
FNPTR getPC(){
	FNPTR *fnAtPC;
	fnAtPC = (((FNPTR *) PSW)+1);
	return *fnAtPC;
}

// callPC (second half of PSW)
void callPC(){
	FNPTR fnToCall;
	fnToCall = ((FNPTR *) PSW)[1];
	//printf("Before Function Call:\n");
	///	printf("\tSCHED... Yay we've got an address to call: %p\n", fnToCall);
	fnToCall();
	//printf("After Function Call:\n");
	return;
}

// setBit takes in <bArray> and sets the bit at <index> to <bitValue>
	// modeled after Dr. England's
void setBit(unsigned char bArray[], int index, int bitValue){
	
	int byteIndex, bitIndex;
	unsigned char mask;
	//printf("\tbArray before setBit: %s\n", bArray);

	byteIndex = index / BYTE_BITS;
	bitIndex = index % BYTE_BITS;

	mask = (unsigned char) (1 << (BYTE_BITS -1)); //create an on bit and shift it to the front
	mask >>= bitIndex; //shift the bit right by bitIndex
	if (bitValue)
		bArray[byteIndex] |= mask;
	else
		bArray[byteIndex] &= ~mask;

	// #ifdef DEBUG
	// 	printf("\tmask: %d\n", mask);
	// 	printf("\tbyte index to change: %d\n", byteIndex);
	// 	printf("\tbit index to change: %d\n", bitIndex);
	// 	printf("\tbArray after setBit: %s\n", bArray);
	// #endif
}