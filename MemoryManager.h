// Brendan Thompson
// 04/10/17
// memoryManager.h

// Header file for interacting with the memory manager

#ifndef MEMMAN_H
#define MEMMAN_H

// Constants
	// Mask for masking disk address when loading a segTable
	#define	DISK_ADDR_MASK ((unsigned long)(-1)>>(sizeof(REGTYPE)*BYTE_BITS/2)) //Dr.England's

	/* bits for seg num in a logical address */
	#define	SEG_MASK	(0x1C00)
	#define	SEG_BITS	3

	/* bits for log pg num in a logical address */
	#define	PAGE_MASK	(0x03C0)
	#define	PAGE_BITS	4

	/* bits for displacement in a logical address */
	#define	DISP_MASK	(0x003F)
	#define	DISP_BITS	6

	/* bits for frame num in a physical address */
	#define	FRAME_MASK	(0x01C0)
	#define	FRAME_BITS	3

// Constructor
	void init_MemMan();

// Destructor
	void shutdown_MemMan();

// Mutators
	// mem_addProcess = add a process to the tables using the data pointed at by regiser Sx
	void mem_addProcess(STRING processName, segInfo *segmentTable);

	// mem_addSharedSeg = creates a sharedSegment or adds a process to an existing one
	void mem_addSharedSeg(STRING processName, FNPTR processDiskAddress);

	// mem_removeProcess = removes any frames associated with the process
	void mem_removeProcess(STRING processName);

	// mem_removeSharedSeg = removes all instances of processName from shared segments
	void mem_removeSharedSeg(STRING processName);

	void handleVirtualMemoryAccess(STRING processName);

	// mem_loadPage = loads a page into a frame
	void mem_loadPage(STRING processName, int segNumber, int pageNumber);

	// mem_referencedFrame = sets <referenced> and conditionaly sets <modified> of corresponding frame
	void mem_referencedFrame(int frameNumber, BOOL modified);

	// Adds the process to the wait list associated with the disk address and conditionally blocks the proces
	void mem_addToQueue(STRING processName, FNPTR diskAddress);

	// Removes the process from the wait list associated with the disk address and conditionally unblocks the next process
	void mem_removeFromQueue(STRING processName, FNPTR diskAddress);
	
// Accessors
	void printFrameTable();

	void printSharedSegs();

	// mem_getFrame = returns the index of the process into the frame table, or -1 if it does not exist
	int mem_getFrame(STRING processName, int segNumber, int pageNumber);

	// mem_findNextFrame = Returns the index of the next available frame
	int mem_findNextFrame();

	// mem_checkLength = returns true if the read/write operation will complete within the limit
	BOOL mem_checkLength(STRING processName, int segNumber, int pageNumber, int displacement, int length);

	// mem_getProtections = returns the protecion key for the segment
	int mem_getProtections(STRING processName, int segNumber);

	// mem_outputPageSwap = outputs info about a page swap (type 0 = R, 1 = W)
	void mem_outputPageSwap(STRING processName, int frameNumber, int type);

#endif