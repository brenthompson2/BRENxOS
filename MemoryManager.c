// Brendan Thompson
// 04/12/17
// memoryManager.c

// Header file for interacting with the memory manager (Paged Segmentation)

// A Segment Table stores the info for the logical segments for the process's memory
// A Page Table stores the info for the pages of the processes's memory
// The Frame Table stores the info for the Frames of physical memory

#include "Kernel.h"

// Type Definitions
	typedef struct {
		STRING name;
		int modified, referenced, locked, segmentNumber, pageNumber;
		// REGTYPE presAbsBit;
		FNPTR diskAddress;
	} frameInfo;

	typedef struct {
		FNPTR diskAddress;
		int numProcesses, semaLockNum, numInQueue;
		STRING nameProcesses[MAXPROCESSES];
		STRING semaQueue[MAXPROCESSES];
	} sharedSegSema;

// Member Variables (Static = local to this file)
	static frameInfo *frameTable[MAXFRAMES];
	static sharedSegSema *sharedSegments[MAXPROCESSES];
	static int numProcesses, numSharedSegs;
	static FILE *pagingOutfile;

// Constructor
	void init_MemMan (){
		int i;
		numProcesses = 0;
		numSharedSegs = 0;
		for (i = 0; i < MAXPROCESSES; i++){
			frameTable[i] = 0;
			sharedSegments[i] = 0;
		}
		pagingOutfile = fopen("paging.txt", "w");
		fprintf(pagingOutfile, "-----paging.txt-----\n");
	}

// Destructor
	void shutdown_MemMan(){
		if (numProcesses == 0){
			return;
		}
		int i;
		for (i = 0; i < numProcesses; i++){
			free(frameTable[i]);
			free(sharedSegments[i]);
			numProcesses--;
		}
		fclose(pagingOutfile);
		return;
	}

// Mutators
	// mem_addProcess = add a process to the Segment table using the data in the array pointed at by regiser Sx
	void mem_addProcess(STRING processName, segInfo *segmentTable){

		unsigned long secondElement;
		int arrayCounter, numSegments, nextLimit, tempProtectionKey;
		FNPTR tempDiskAddress;
		arrayCounter = 0;
		numSegments = 0;

		nextLimit = ((unsigned long *) Sx)[arrayCounter];
		while (nextLimit != 0){
			// printf("\t[MemMan]: Populating segment %d...\n", numSegments);

			if (numSegments == 8){
				printf("\t\t[MemMan]: Error! Too many segments\n");
				exit(1);
			}

			// Check Limit
			if (nextLimit >= 1024){
				printf("\t\t[MemMan]: Error! Limit too large\n");
				exit(1);
			}
			(segmentTable[numSegments]).limit = nextLimit;

			// Access the Second Element of the segInfo (w/ help from Dr. England's p4example)
        	secondElement = (unsigned long)(((REGTYPE *) Sx)[(numSegments*2 + 1)]);

			// Get Disk Address (Right Half)
        	tempDiskAddress = (REGTYPE) (secondElement & DISK_ADDR_MASK);
			(segmentTable[numSegments]).diskAddress = tempDiskAddress;

			// Get Protection Key (Left Half)
			tempProtectionKey = (unsigned int) (secondElement >> (((sizeof(void *)) * 8) / 2));
			tempProtectionKey = (tempProtectionKey & 0x03);
			(segmentTable[numSegments]).protectionKey = tempProtectionKey;

			// Create Sema for sharedSeg
			if (tempProtectionKey == 2){
				mem_addSharedSeg(processName, tempDiskAddress);
			}

			// Prepare for next element of Sx
			numSegments++;
			nextLimit = ((unsigned long *) Sx)[arrayCounter];
			arrayCounter = arrayCounter + 2;
		}
		return;
	}

	// mem_addSharedSeg = creates a sharedSegment or adds a process to an existing one
	void mem_addSharedSeg(STRING processName, FNPTR processDiskAddress){
		int i, cursorNP;
		FNPTR cursorDA;
		sharedSegSema *tempSharedSeg;
		i = 0;
		printf("\t[MemMan]: Adding %s to shared seg with disk address %p\n", processName, processDiskAddress);

		// If No Shared Segs
		if (numSharedSegs == 0){
			tempSharedSeg = malloc(sizeof(sharedSegSema));
			(*tempSharedSeg).diskAddress = processDiskAddress;
			strcpy((*tempSharedSeg).nameProcesses[0], processName);
			(*tempSharedSeg).numProcesses = 1;
			(*tempSharedSeg).numInQueue = 0;
			(*tempSharedSeg).semaLockNum = 1;
			sharedSegments[0] = tempSharedSeg;
			numSharedSegs++;
			return;
		}

		// Check Existing Segs
		while (i < numSharedSegs){

			tempSharedSeg = sharedSegments[i];
			cursorDA = (*tempSharedSeg).diskAddress;

			// if Seg Already Created
			if (cursorDA == processDiskAddress){
				// printf("Adding to Shared Seg -- CDA: %p PDA: %p\n", cursorDA, processDiskAddress);
				cursorNP = (*tempSharedSeg).numProcesses;
				strcpy((*tempSharedSeg).nameProcesses[cursorNP], processName);
				// printf("Adding %s as process num %d\n",processName, (*tempSharedSeg).numProcesses);
				(*tempSharedSeg).numProcesses++;
				return;
			}
			i++;
		}
 		
		// Create New Shared Seg
		// printf("Creating New Shared Seg %d\n", i);
		tempSharedSeg = malloc(sizeof(sharedSegSema));
		(*tempSharedSeg).diskAddress = processDiskAddress;
		strcpy((*tempSharedSeg).nameProcesses[0], processName);
		(*tempSharedSeg).numProcesses = 1;
		(*tempSharedSeg).numInQueue = 0;
		(*tempSharedSeg).semaLockNum = 1;
		sharedSegments[i] = tempSharedSeg;
		numSharedSegs++;
		return;
	}

	// mem_removeProcess = removes any frames associated with the process
	void mem_removeProcess(STRING processName){
		int frameNumber;
		frameInfo *currentFrame;
		FNPTR locationAddress;

		if (numProcesses == 0){
			return;
		}

		mem_removeSharedSeg(processName);

		for (frameNumber = 0; frameNumber < MAXFRAMES; frameNumber++){
			currentFrame = frameTable[frameNumber];

			//Check if frame holds a Process
			if (currentFrame != 0){
				// Check if Correct Process
				if(!(strcmp((*currentFrame).name, processName))){
					if ((*currentFrame).modified){
						locationAddress = (FNPTR) (((unsigned long) frameNumber) << DISP_BITS);
						pageW(locationAddress, (*currentFrame).diskAddress, (*currentFrame).pageNumber);
					}
					free(currentFrame);
					frameTable[frameNumber] = 0;
					numProcesses--;
				}
			}
		}
		printf("\t[MemMan]: Removed %s\n", processName);

		return;

	}

	// mem_removeSharedSeg = removes all instances of processName from shared segments
	void mem_removeSharedSeg(STRING processName){
		int segCounter, processCounter, i;
		sharedSegSema *tempSharedSeg;

		// Check Segments
		for (segCounter = 0; segCounter < numSharedSegs; segCounter++){
			tempSharedSeg = sharedSegments[segCounter];

			// Check Processes
			for (processCounter = 0; processCounter < (*tempSharedSeg).numProcesses; processCounter++){
				// if one of the processes
				if (!(strcmp(processName, (*tempSharedSeg).nameProcesses[processCounter]))){
					// Remove process and shift rest of processes
					(*tempSharedSeg).numProcesses--;
					for (i = processCounter; i < (*tempSharedSeg).numProcesses; i++){
						strcpy(((*tempSharedSeg).nameProcesses[i]), ((*tempSharedSeg).nameProcesses[i + 1]));
					}
					processCounter--;
				}
			}
			// if no more processes for Seg
			if ((*tempSharedSeg).numProcesses == 0){
				// Remove seg and shift rest of segs
				numSharedSegs--;
				free(tempSharedSeg);
				for (i = segCounter; i < numSharedSegs; i++){
					sharedSegments[i] = sharedSegments[i + 1];
				}
				segCounter--;
			}
		}
		return;
	}

	void handleVirtualMemoryAccess(STRING processName){
		int vMemValue;

		// Select Virtual Memory Suite
		vMemValue = getVirtualMemoryValue();
		switch (vMemValue){
			case 0:{
				FNPTR diskAddress = Rx;
				printf("\t[MemMan]: %s is requesting to Vmem fetch/write at %p...\n",processName, diskAddress);
				mem_addToQueue(processName, diskAddress);
				return;
			}
			case 1: {
				printf("\t[MemMan]: Vmem fetch...\n");
				REGTYPE logicalAddress, physicalAddress, destination;
				int rLength, segNumber, pageNumber, frameNumber, displacement;
				BOOL isOnePage;

				// Load Information From Registers
				logicalAddress = Rx;
				destination = Tx;
				rLength = (unsigned long) Sx;
				printf("\t\t[MemMan]: logicalAddress: %p\tdestination: %p\trLength: %d\n",
					logicalAddress, destination, rLength);
				
				// Get Logical Values
				displacement = (int) (((unsigned long) logicalAddress) & 0x3F); // & for last 6 bits
				pageNumber = (int) ((((unsigned long) logicalAddress) >> DISP_BITS) & 0x0F); // shift right then & for last 4 bits
				segNumber = (int) ((((unsigned long) logicalAddress) >> 10) & 0x03); // shift right then & for last 3 bits
				printf("\t\t[MemMan]: displacement: %d\tpageNumber: %d\tsegNumber: %d\n",
					displacement, pageNumber, segNumber);

				// Get Frame Number of page
				frameNumber = mem_getFrame(processName, segNumber, pageNumber);
				printf("\t\t[MemMan]: frameNumber: %d\n", frameNumber);
				
				// if not loaded, load the page, set the saved PC back to the old PC, and return
				if (frameNumber == -1){
					mem_loadPage(processName, segNumber, pageNumber);
					printf("\t\t[MemMan]: Process %s's Seg %d Page %d was loaded into memory (PageR)\n", 
						processName, segNumber, pageNumber);
					PT_rollbackProcess(processName);
					// vmemDump();
					return;
				}

				// Convert Logical Values to physicalAddress
        		physicalAddress = (FNPTR) ((unsigned long) displacement);
        		physicalAddress = physicalAddress + (((unsigned long) frameNumber) << DISP_BITS);

				// Check rLength < limit
        		isOnePage = mem_checkLength(processName, segNumber, pageNumber, displacement, rLength);

				vmemR(destination, physicalAddress, rLength);
				mem_referencedFrame(frameNumber, FALSE);

				printf("\t[MemMan]: %s conducted a vmemR of %d bits at %p\n", processName, rLength, physicalAddress);
				printf("\t[MemMan]: %s read:\t-->%s<--\n", processName, (char *) destination);
				Sched_returnControl();
				return;
			}
			case 2: {
				printf("\t[MemMan]: Vmem Write...\n");
				REGTYPE logicalAddress, physicalAddress, source;
				int wLength, segNumber, pageNumber, frameNumber, displacement, protectionKey;
				BOOL isOnePage;

				// Load Information From Registers
				logicalAddress = Rx;
				source = Tx;
				wLength = (unsigned long) Sx;

        		printf("\t[MemMan]: %s is gonna write:\t-->%s<--\n", processName, (char *) Tx);
				printf("\t\t[MemMan]: logicalAddress: %p\tsource: %p\trLength: %d\n",
					logicalAddress, source, wLength);
				
				// Get Logical Values
				displacement = (int) (((unsigned long) logicalAddress) & 0x3F); // & for last 6 bits
				pageNumber = (int) ((((unsigned long) logicalAddress) >> DISP_BITS) & 0x0F); // shift right then & for last 4 bits
				segNumber = (int) ((((unsigned long) logicalAddress) >> 10) & 0x03); // shift right then & for last 3 bits
				printf("\t\t[MemMan]: displacement: %d\tpageNumber: %d\tsegNumber: %d\n",
					displacement, pageNumber, segNumber);

				// Check Protections
				protectionKey = mem_getProtections(processName, segNumber);
				if (protectionKey == 0){
					printf("\t\t[MemMan]: Insufficient Permisions\n");
					return;
				}

				// Get Frame Number of page
				frameNumber = mem_getFrame(processName, segNumber, pageNumber);
				printf("\t\t[MemMan]: frameNumber: %d\n", frameNumber);
				
				// if frame not loaded -> load the page; rollback PC, timeSlice, and timeRemaining; and return
				if (frameNumber == -1){
					mem_loadPage(processName, segNumber, pageNumber);
					printf("\t\t[MemMan]: Process %s's Seg %d Page %d was loaded into memory (PageR)\n", 
						processName, segNumber, pageNumber);
					PT_rollbackProcess(processName);
					// vmemDump();
					return;
				}

				// Convert Logical Values to physicalAddress
        		physicalAddress = (FNPTR) ((unsigned long) displacement);
        		physicalAddress = physicalAddress + (((unsigned long) frameNumber) << DISP_BITS);
        		printf("\t[MemMan]: Frame Number = %d, displacement = %d, physicalAddress = %p\n", 
        			frameNumber, displacement, physicalAddress);
				
				// Check wLength < limit
        		isOnePage = mem_checkLength(processName, segNumber, pageNumber, displacement, wLength);

        		// Conduct vmemW
        		printf("\t[MemMan]: %s is gonna write:\t-->%s<--\n", processName, (char *) Tx);
				// vmemDump();
				vmemW(physicalAddress, (char *) Tx, wLength);
				// vmemDump();
				mem_referencedFrame(frameNumber, TRUE);

				// Write frame out to memory (NUCLEAR OPTION)
				FNPTR locationAddress, diskAddress;
				locationAddress = (FNPTR) (((unsigned long) frameNumber) << DISP_BITS);
				diskAddress = (*frameTable[frameNumber]).diskAddress;
				// segDump(diskAddress);
				pageW(locationAddress, diskAddress, pageNumber);
				// segDump(diskAddress);
				mem_outputPageSwap(processName, frameNumber, 1);

				printf("\t[MemMan]: %s conducted a vmemW of %d bits at %p\n", processName, wLength, physicalAddress);
				Sched_returnControl();
				return;
			}
			case 3:{
				printf("\t[MemMan]: End Vmem fetch/write...\n");
				FNPTR diskAddress = Rx;
				mem_removeFromQueue(processName, diskAddress);
				return;
			}
		}
	}

	// mem_loadPage = loads a page into a frame
	void mem_loadPage(STRING processName, int segNumber, int pageNumber){
		PCB *currentPCB;
		segInfo *currentSeg;
		int frameNumber;
		FNPTR diskAddress, locationAddress;

		// Get the Frame Number
		frameNumber = mem_findNextFrame();
		if (frameTable[frameNumber] != 0){
			numProcesses--;
		}

		locationAddress = (FNPTR) (((unsigned long) frameNumber) << DISP_BITS);

		// Get the diskAddress
		currentPCB = getPCB(processName);
		diskAddress = (((*currentPCB).segTable)[segNumber]).diskAddress;
		printf("\t\t\t[MemMan]: %s's Seg %d has diskAddress %p\n", 
			processName, segNumber, diskAddress);

		printf("\t\t\t[MemMan]: FrameNum = %d, DiskAddress = %p, PageNum = %d\n", frameNumber, diskAddress, pageNumber);

		// Load the page into memory
		pageR(locationAddress, diskAddress, pageNumber);

		// Update the Frame Table
		frameInfo *currentFrame = malloc(sizeof(frameInfo));
		strcpy((*currentFrame).name, processName);
		(*currentFrame).modified = FALSE;
		(*currentFrame).referenced = FALSE;
		(*currentFrame).locked = TRUE;
		(*currentFrame).segmentNumber = segNumber;
		(*currentFrame).pageNumber = pageNumber;
		//(*currentFrame).presAbsBit = ?; //Address of present-absent int
		(*currentFrame).diskAddress = diskAddress;
		frameTable[frameNumber] = currentFrame;

		numProcesses++;
		mem_outputPageSwap(processName, frameNumber, 0);
		printf("\t\t\t[MemMan]: Loaded page into vmem\n");

		return;
	}

	// mem_referencedFrame = sets <referenced> of the correct frame table element to 0
	void mem_referencedFrame(int frameNumber, BOOL modified){
		frameInfo *currentFrame;
		currentFrame = frameTable[frameNumber];
		
		(*currentFrame).referenced = TRUE;
		(*currentFrame).locked = FALSE;
		if (modified){
			(*currentFrame).modified = TRUE;
		}
		return;
	}

	// Adds the process to the wait list associated with the disk address and conditionally blocks the proces
	void mem_addToQueue(STRING processName, FNPTR diskAddress){
		sharedSegSema *tempSharedSeg;
		int segCounter, currentSemaCount;
		diskAddress = Rx;

		// Look for the Shared Seg
		for (segCounter = 0; segCounter < numSharedSegs; segCounter++){
			tempSharedSeg = sharedSegments[segCounter];

			// If disk addresses match
			if (diskAddress == (*tempSharedSeg).diskAddress){
				currentSemaCount = (*tempSharedSeg).numInQueue;
				strcpy((*tempSharedSeg).semaQueue[currentSemaCount], processName);
				(*tempSharedSeg).semaLockNum--;
				(*tempSharedSeg).numInQueue++;

				// If not first process, block
				if ((*tempSharedSeg).numInQueue > 1){
					Sched_blockProcess((*tempSharedSeg).semaQueue[currentSemaCount], 1);
				}
				else {
					Sched_returnControl();
				}
				return;
			}
		}		
		printf("\t[MemMan]: Error! Unable to locate %s's Shared Seg with Disk Address %p\n",processName, diskAddress);
		exit(1);
	}

	// Removes the process from the wait list associated with the disk address and conditionally unblocks the next process
	void mem_removeFromQueue(STRING processName, FNPTR diskAddress){
		sharedSegSema *tempSharedSeg;
		int segCounter, semaCounter, i;
		diskAddress = Rx;

		printf("Removing process %s from Sema Queue %p\n", processName, diskAddress);
		printSharedSegs();

		// Look Through the Shared Segs
		for (segCounter = 0; segCounter < numSharedSegs; segCounter++){
			tempSharedSeg = sharedSegments[segCounter];

			// If  Disk Addresses match
			if (diskAddress == (*tempSharedSeg).diskAddress){

				// Look for Process Name In Sema Queue
				for (semaCounter = 0; semaCounter < (*tempSharedSeg).numInQueue; semaCounter++){
					
					// If names match
					if (!(strcmp((*tempSharedSeg).semaQueue[semaCounter], processName))){

						// If only process
						if ((*tempSharedSeg).numInQueue == 1){
							strcpy((*tempSharedSeg).semaQueue[semaCounter], "");
							(*tempSharedSeg).numInQueue = 0;
							(*tempSharedSeg).semaLockNum++;
							return;
						}

						// If First Process, unBlock Next
						if (semaCounter == 0){
							printf("\t[MemMan]: %s is the first process, so were unblocking %s\n", processName, (*tempSharedSeg).semaQueue[1]);
							Sched_unBlockProcess((*tempSharedSeg).semaQueue[1], 1);
						}

						(*tempSharedSeg).numInQueue--;
						(*tempSharedSeg).semaLockNum++;

						// Shift Rest of Processes Forward in Queue
						for (i = semaCounter; i < ((*tempSharedSeg).numInQueue); i++){
							printf("%d We're shifting\n", i);
							strcpy((*tempSharedSeg).semaQueue[i], (*tempSharedSeg).semaQueue[i + 1]);
						}
						strcpy((*tempSharedSeg).semaQueue[i], "");
						return;
					}
				}
			}
		}
		
		printf("\t[MemMan]: Error! Unable to locate %s's Shared Seg with Disk Address %p\n",processName, diskAddress);
		exit(1);
	}

// Accessors
	void printFrameTable(){
		int frameNumber;
		frameInfo *currentFrame;

		if (numProcesses == 0){
			printf("[MemMan]: Frame Table Empty\n\n");
			return;
		}

		printf("Frame Table:\n");
		for (frameNumber = 0; frameNumber < MAXFRAMES; frameNumber++){
			currentFrame = frameTable[frameNumber];

			//Check if frame holds a Process
			if (currentFrame != 0){
				printf("\t%d: %s:\tM:%d  R:%d  L:%d", 
					frameNumber, (*currentFrame).name, (*currentFrame).modified, (*currentFrame).referenced, (*currentFrame).locked);
				printf("\tSegNum: %d\tPageNum: %d\tDiskAddr: %p\n", 
					(*currentFrame).segmentNumber, (*currentFrame).pageNumber, (*currentFrame).diskAddress);
				//(*currentFrame).presAbsBit = ?;
			}
		}
		printf("\n");
		return;
	}

	void printSharedSegs(){
		int segCount, processCount;
		sharedSegSema *tempSharedSeg;

		if (numSharedSegs == 0){
			printf("[MemMan]: No Shared Segs\n\n");
			return;
		}

		printf("\nShared Segments: (%d Segs)\n", numSharedSegs);
		for (segCount = 0; segCount < numSharedSegs; segCount++){
			tempSharedSeg = sharedSegments[segCount];
			printf("\t%d: %p  NumProcesses: %d  LockNum: %d\n",	
				segCount, (*tempSharedSeg).diskAddress, (*tempSharedSeg).numProcesses, (*tempSharedSeg).semaLockNum);
			printf("\t\tAll Processes: ");
			for (processCount = 0; processCount < (*tempSharedSeg).numProcesses; processCount++){
				printf("%s, ",(*tempSharedSeg).nameProcesses[processCount]);
			}
			printf("\n");
			if ((*tempSharedSeg).semaLockNum < 1){
				printf("\t\tSemaphore Queue (%d): ", (*tempSharedSeg).numInQueue);
				for (processCount = 0; processCount < (*tempSharedSeg).numInQueue; processCount++){
					printf("%d) %s, ",processCount, (*tempSharedSeg).semaQueue[processCount]);
				}
			}
			printf("\n");
		}
		printf("\n");
		return;
	}

	// mem_getFrame = returns the index of the process into the frame table, or -1 if it does not exist
	int mem_getFrame(STRING processName, int segNumber, int pageNumber){
		int frameNumber;
		frameInfo *currentFrame;

		if (numProcesses == 0){
			return -1;
		}

		for (frameNumber = 0; frameNumber < MAXFRAMES; frameNumber++){
			currentFrame = frameTable[frameNumber];

			//Check if frame holds a Process
			if (currentFrame != 0){
				// Check if Correct Process
				if(!(strcmp((*currentFrame).name, processName))){
					// Check if Correct Segment
					if ((*currentFrame).segmentNumber == segNumber){
						// Check if correct Page
						if ((*currentFrame).pageNumber == pageNumber){
							return frameNumber;
						}
					}
				}
			}
		}
		return -1;
	}

	// mem_findNextFrame = Returns the index of the next available frame
	int mem_findNextFrame(){
		int frameNumber, attemptNumber;
		frameInfo *currentFrame;
		FNPTR locationAddress;
		STRING processName;
		attemptNumber = 0;

		if (numProcesses == 0){
			return 0;
		}

		for (frameNumber = 0; frameNumber < MAXFRAMES; frameNumber++){
			currentFrame = frameTable[frameNumber];

			// If frame doesn't hold a process
			if (currentFrame == 0){
				return frameNumber;
			}

			// Check if Not Locked
			if((*currentFrame).locked == FALSE){

				// Check if Referenced
				if ((*currentFrame).referenced == 0){

					// Check if Not Modified
					if ((*currentFrame).modified == FALSE){
						return frameNumber;
					}
					else {
						locationAddress = (FNPTR) (((unsigned long) frameNumber) << DISP_BITS);
						strcpy(processName, (*currentFrame).name);
						segDump((*currentFrame).diskAddress);
						pageW(locationAddress, (*currentFrame).diskAddress, (*currentFrame).pageNumber);
						segDump((*currentFrame).diskAddress);
						mem_outputPageSwap(processName, frameNumber, 1);
						return frameNumber;
					}
				}
				else {
					(*currentFrame).referenced = 0;
				}
			}

			// Second Chance
			if ((frameNumber + 1) == MAXFRAMES){
				if (attemptNumber == 0){
					frameNumber = -1;
					attemptNumber++;
				}
			}
		}
	}

	BOOL mem_checkLength(STRING processName, int segNumber, int pageNumber, int displacement, int length){
		PCB *currentPCB;
		int segLimit, lastCharacter;

		currentPCB = getPCB(processName);
		segLimit = (((*currentPCB).segTable)[segNumber]).limit;

		lastCharacter = (pageNumber << DISP_BITS);
		lastCharacter = lastCharacter | displacement;
		lastCharacter = lastCharacter + length;

		if (lastCharacter < segLimit){
			return TRUE;
		}
		else {
			printf("\t\t[MemMan]: Amount to be read/wrote exceeds segment limit\n");
			return FALSE;
		}
	}

	// mem_getProtections = returns the protecion key for the segment
	int mem_getProtections(STRING processName, int segNumber){
		PCB *currentPCB;
		int protectionKey;

		currentPCB = getPCB(processName);
		protectionKey = (((*currentPCB).segTable)[segNumber]).protectionKey;
		return protectionKey;
	}

	// mem_outputPageSwap = outputs info about a page swap (type 0 = R, 1 = W)
	void mem_outputPageSwap(STRING processName, int frameNumber, int type){
		FNPTR diskAddress = (*frameTable[frameNumber]).diskAddress;
		int segNumber = (*frameTable[frameNumber]).segmentNumber;
		int pageNumber = (*frameTable[frameNumber]).pageNumber;
		if (type == 0){
			fprintf(pagingOutfile, "\n\n[READ]: %s read in from DiskAddress %p, which is seg %d page %d\n",
				processName, diskAddress, segNumber, pageNumber);
			fprintf(pagingOutfile, "\tThe opperation read to frame %d\n",
				frameNumber);
			return;
		}
		else {
			fprintf(pagingOutfile, "\n\n[WRITE]: %s wrote to DiskAddress %p, which is seg %d page %d\n",
				processName, diskAddress, segNumber, pageNumber);
			fprintf(pagingOutfile, "\tThe opperation wrote from frame %d\n",
				frameNumber);
			return;
		}
	}