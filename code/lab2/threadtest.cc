// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(_int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", (int) which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    currentThread->SetPriority(1);
    printf("name=%s,threadId=%d,priority=%d\n",currentThread->getName(),0,currentThread->getPriority());

    Thread* t1 = new Thread("Thread1", 2);
    printf("name=%s,priority=%d\n",t1->getName(),t1->getPriority());
    t1->Fork(SimpleThread, 1);

    Thread* t2 = new Thread("Thread2", 3);
    printf("name=%s,priority=%d\n",t2->getName(),t2->getPriority());
    t2->Fork(SimpleThread, 2);

    Thread* t3 = new Thread("Thread3", 4);
    printf("name=%s,priority=%d\n",t3->getName(),t3->getPriority());
    t3->Fork(SimpleThread, 3);

    SimpleThread(0);
}
// 创建四个线程，加上主线程共五个，优先值越大优先级越高


