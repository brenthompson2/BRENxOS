/************************************************************************** moses_m.h *
 *									
 *	FILE:	header for MOSES II virtual O/S environment: 
 *				P2 virtual memory management
 *									
 *	Contains constants and declarations used in file	
 *	moses_m.c, a set of simulated user processes and simulated hardware features.		
 *    	(See CS 3074 Proj2 assignment description.)
 *									
 *	PROGRAMMED:	R England
 *				Transy U
 *	COURSE:		CS 3074: Netcentric Computing 
 *				Winter 2017
 *
 *	VERSION:	1.0: 30 March 2017
 *                 no split reads / writes
 *                 no I/O interrupts
 *                 no segment access errors
 *           
 **************************************************************************************/

#ifndef	MOSES_M_H
#define	MOSES_M_H

/* Variable/Function access types:
 */
#define PUBLIC
#define PRIVATE static
#define BELOW   extern

/* Types for Program Status Word (PSW):
 */
typedef void	(*FNPTR)();
typedef	short	tm_t;
typedef void*	psw_t[2];
BELOW	psw_t	PSW;


/* Registers
 */
typedef	void *REGTYPE;
BELOW   REGTYPE	Rx, Sx, Tx, Ux, Vx;


/* System initialization function
 */
BELOW   void            init_moses (void);

/* I/O functions
 */
BELOW   void            io (char *s);
BELOW   void            iofl (void);
BELOW   void            iolog (void);


/* Simulated address types: Vmem / Vdisk / logical				
 */
typedef	REGTYPE	VMEMADDR;
typedef REGTYPE	VDISKADDR;
typedef	REGTYPE	LOGICADDR;


/* Read / Write an entire page: 
 *		disk <-- --> frame
 */
BELOW	void	pageW (VMEMADDR	location,
			VDISKADDR seg_addr,
			unsigned long page_no);
BELOW	void	pageR (VMEMADDR	location,
			VDISKADDR seg_addr,
			unsigned long page_no);


/* Virtual memory read / write: 
 *		char array <-- --> memory
 */
BELOW	void	vmemW (VMEMADDR	destination,
			char source[],
			unsigned long wlength);
BELOW	void	vmemR (char destination[],
			VMEMADDR source,
			unsigned long rlength);


/* segPrint:	show current contents of one data segment [to SCREEN]
 * vmemPrint:	show current contents of all frames [to SCREEN]
 */
BELOW	void	segPrint (VDISKADDR seg_addr);
BELOW	void	vmemPrint (void);

/* segDump:	show current contents of one or all data segments (0 for all) [to file dump.txt]
 * vmemDump:	show current contents of all frames [to file dump.txt]
 */
BELOW	void	segDump (VDISKADDR seg_addr);
BELOW	void	vmemDump (void);

/* printff: macro for marking required Prog 2 screen output
 */
#define	printff	printf("\nKERNEL::");printf

#endif

/* (End of moses_m.h)
 ************************************************************************/
