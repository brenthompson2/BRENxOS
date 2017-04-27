# Brendan Thompson
# 04/09/17
# Makefile for Netcentric Project 2 Program 5

# Compile and Link With "make main"
# clear object, txt, and exe files with "make clean"
# run with ./BRENxOS

main: moses_m.o PSW.o ProcessTable.o Scheduler.o Semaphore.o Kernel.o MemoryManager.o moses_m.h PSW.h ProcessTable.h Scheduler.h Semaphore.h MemoryManager.h
	gcc -o BRENxOS moses_m.o PSW.o ProcessTable.o Scheduler.o Semaphore.o Kernel.o MemoryManager.o

Kernel.o: Kernel.c PSW.h ProcessTable.h Scheduler.h moses_m.h
	gcc -c Kernel.c

MemoryManager.o: MemoryManager.c MemoryManager.h ProcessTable.h PSW.h moses_m.h
	gcc -c MemoryManager.c

Semaphore.o: Semaphore.c Semaphore.h ProcessTable.h
	gcc -c Semaphore.c

Scheduler.o: Scheduler.c Scheduler.h ProcessTable.h PSW.h moses_m.h
	gcc -c Scheduler.c

ProcessTable.o: ProcessTable.c ProcessTable.h moses_m.h
	gcc -c ProcessTable.c

PSW.o: PSW.c PSW.h moses_m.h
	gcc -c PSW.c

clean:
	rm BRENxOS Kernel.o MemoryManager.o Semaphore.o Scheduler.o ProcessTable.o PSW.o moses_m.txt dump.txt paging.txt