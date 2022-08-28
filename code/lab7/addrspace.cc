// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------
BitMap *AddrSpace:: freeMap = new BitMap(NumPhysPages);// 初始化bitMap
BitMap *AddrSpace:: spaceIdMap = new BitMap(PROCESS_NUM);// 定义spaceID池，一个用户进程对应一个唯一的地址空间id，大小为256个

AddrSpace::AddrSpace(char* FileName)
{
    fileName = FileName;
    OpenFile *executable = fileSystem->Open(FileName);
    NoffHeader noffH;
    unsigned int i, size;
    pageList = new List;// 初始化一个FIFO队列

    // 分配进程空间标识符
    ASSERT(spaceIdMap->NumClear() > 0);// 确认页面足够分配
    spaceId = spaceIdMap->Find();

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    // 整个用户进程的虚拟页长度
    numPages = divRoundUp(size, PageSize);

    // 用户进程的初始化部分（代码段+初始化数据段的页长度）,永远不能被换出
    int initPages = divRoundUp(noffH.code.size + noffH.initData.size,PageSize);

    // 用户进程无法初始化（代码段+数据段大于目前可用页长度）
    ASSERT(initPages <= NumPhysPages && initPages <= freeMap->NumClear());

    DEBUG('a', "Initializing address space, num pages %d, size %d\n",
          numPages, size);

    // 每个用户进程必须的页数（包含了之后可自由置换的页：3个）
    int necessaryPages = max(4,initPages+3);

    size = numPages * PageSize;

//  声明虚拟页表
    pageTable = new TranslationEntry[numPages];
//    ASSERT(freeMap->NumClear() >= numPages);// 确认页面足够分配
    for (i = 0; i < necessaryPages; i++) {
        if(i >= initPages)// 可置换的页放入优先队列
            pageList->Append((void*)i);

        // 用户进程的初始化部分的虚拟页表初始化
        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = freeMap->Find();   // 初始不分配
        pageTable[i].valid = TRUE;
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;  // if the code segment was entirely on
                                        // a separate page, we could set its
                                        // pages to be read-only
    }
    for(i;i<numPages;i++){
        // 用户进程的其他部分的虚拟页表的初始化
        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = -1;
        pageTable[i].valid = false;
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;
    }
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment

    bzero(machine->mainMemory, size);//初始化物理内存

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n",
              noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
                           noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n",
              noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
                           noffH.initData.size, noffH.initData.inFileAddr);
    }

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    for(int i = 0;i < numPages;i++){
        freeMap->Clear(pageTable[i].physicalPage);// 释放位图中的内存页标识
    }
    spaceIdMap->Clear(spaceId);// 释放位图中的进程地址空间标识符
    delete [] pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
        machine->WriteRegister(i, 0);

        // Initial program counter -- must be location of "Start"
        machine->WriteRegister(PCReg, 0);

        // Need to also tell MIPS where next instruction is, because
        // of branch delay possibility
        machine->WriteRegister(NextPCReg, 4);

       // Set the stack register to the end of the address space, where we
       // allocated the stack; but subtract off a bit, to make sure we don't
       // accidentally reference off the end!
        machine->WriteRegister(StackReg, numPages * PageSize - 16);
        DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

//自定义程序：
void AddrSpace::Print()
{
    printf("spaceID: %d\n",spaceId);
    printf("page table dump: %d pages in total\n", numPages);
    printf("============================================\n");
    printf("  VirtPage, PhysPage, valid\n");
    for (int i=0; i < numPages; i++) {
        printf("\t%d,\t%d,\t%d\n", pageTable[i].virtualPage,pageTable[i].physicalPage,pageTable[i].valid);
    }
    printf("============================================\n\n");
}

int AddrSpace::getSpaceId() {
    return spaceId;
}

TranslationEntry * AddrSpace::getPageTable(){
    return pageTable;
}

void AddrSpace::readIn(int newPage) {
    OpenFile *executable = fileSystem->Open(fileName);
    if (executable == NULL) {
        printf("Unable to open file %s\n", fileName);
        return;
    }
    executable->ReadAt(&(machine->mainMemory[pageTable[newPage].physicalPage]),PageSize,newPage*PageSize);
    delete executable;
    printf("置换页已写入内存!");
}

void AddrSpace::writeBack(int oldPage)
{
    if(pageTable[oldPage].dirty){
        printf("被修改 dirty!写回磁盘，spaceId:%d,oldPage:%d\n",spaceId,oldPage);
        OpenFile *executable = fileSystem->Open(fileName);

        if (executable == NULL) {
            printf("Unable to open file %s\n", fileName);
            return;
        }
        executable->WriteAt(&(machine->mainMemory[pageTable[oldPage].physicalPage]),PageSize,oldPage*PageSize);
        delete executable;
    }else{
        printf("无需修改\n");
    }
}

int AddrSpace::FIFO(int newPage) {
    pageList->Append((void*)newPage);
    return (int)pageList->Remove();
}

