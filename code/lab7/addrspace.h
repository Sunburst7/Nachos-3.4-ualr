// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "bitmap.h"
#include "list.h"

#define UserStackSize		1024 	// increase this as necessary!
#define PROCESS_NUM 256 //自定义最大用户进程数

class AddrSpace {
  public:

    AddrSpace(char* FileName);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch
    // 获取虚拟页表
    TranslationEntry *getPageTable();
    // 打印
    void Print();
    // 返回进程id
    int getSpaceId();
    // 被修改的页写回磁盘
    void writeBack(int oldPage);
    // 读取需要置换的页到内存中
    void readIn(int newPage);
    // 先进先出的置换
    int FIFO(int newPage);

  private:
    TranslationEntry *pageTable;	// Assume linear page table translation
					                // for now!
    char* fileName;
    unsigned int numPages;		// Number of pages in the virtual address space，整个进程的最大的虚拟页数
    int spaceId; // 地址空间标识符
    List* pageList;// 当前内存页号队列
    // 全局静态变量
    static BitMap *freeMap;// 内存可用帧
    static BitMap *spaceIdMap;// 进程可用ID


};

#endif // ADDRSPACE_H
