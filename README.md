# BRENxOS

Brendan Thompson
04/12/17
README.md

Welcome to BRENxOS V2.5, simulated operating system I designed for my Netcentric Computing class at Transylvania University. The operating system consists of the main Kernel which relies on a Scheduler, Process Table, and Memory Manager to handle the processes. There is also the Program Status Word (PSW) for process runtime information as well as a Semaphore to control access to a shared output buffer

Compilation:
	To Compile and Link the Operating System type "make"

	To Remove object, exe, and txt files type "make clean"

	To run the Operating System type "./BRENxOS"

Files:
	The Kernel handles running the processes

	The Scheduler manages the order that control is passed to the processes (.c and .h)

	The Process Table stores data in shared hardware for each process (PCBs) (.c and .h)

	The Memory Manager controls virtual memory access using a frame table (.c and .hs)

	The PSW (Program Status Word) handles information about the process's operation (.c and .h)

	The Semaphore handles access to the shared output buffer (.c and .h)

	Moses is where the processes and their code segments are held - Created by Dr. Endland
