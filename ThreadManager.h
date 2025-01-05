//
// Created by yuval on 4/6/2023.
//

#ifndef _THREADMANAGER_H_
#define _THREADMANAGER_H_

#include "Thread.h"

#include <queue>
#include <string>
#include <map>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <sys/time.h>
#include <cmath>
#include <setjmp.h>

#define SYS_ERR "system error: "
#define LIB_ERR "thread library error: "

#define TIMER_ERR "setitimer error"
#define ALLOC_ERR "allocation error"
#define SIGACTION_ERR "sigaction error"
#define QUANTUM_ERR "invalid quantum"
#define MAX_THREADS_ERR "maximum number of threads reached"
#define ENTRY_POINT_ERR "invalid entry point"
#define TID_ERR "invalid tid"
#define MAIN_THREAD_ERR "attempt to move the main thread to sleep"

class ThreadManager{

 public:
  int quantum;
  int running_tid;
  std::vector<int> *ready_queue;
  std::map<int, Thread*> *threads;
  struct sigaction sa;
  struct itimerval timer;
  int total_quantums;
  Thread *thread_to_delete;

  explicit ThreadManager();
  void set_quantum(int q);
  void set_timer();
  void free_resources();
};

#endif //_THREADMANAGER_H_
