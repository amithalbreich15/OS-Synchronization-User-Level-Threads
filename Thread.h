//
// Created by yuval on 4/6/2023.
//

#ifndef _THREAD_H_
#define _THREAD_H_

#include <string>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>

class Thread{

 public:
  int tid;
  std::string state;
  char *sp;
  sigjmp_buf buf{};
  int quantum_sleep;
  int quantum_count;

  explicit Thread(int tid);
};

#endif //_THREAD_H_
