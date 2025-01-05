//
// Created by yuval on 4/6/2023.
//
#include <iostream>
#include <cassert>
#include "uthreads.h"
#include <vector>
#include "algorithm"

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>

#define SECOND 1000000

typedef unsigned long address_t;

void func1(){

}

void thread0(void)
{
  std::cout << uthread_get_tid() << std::endl;
  int i = 0;
  std::cout << "Thread 0 moving to sleep" << std::endl;
  uthread_sleep (10);
  std::cout << "Thread 0 finished sleeping" << std::endl;
  while (1)
  {
    ++i;
    std::cout << "in thread 0" << std::endl;
    for (int j=0; j<SECOND*10; j++){

    }
  }
}


void thread1(void)
{
  int i = 0;
  while (1)
  {
    ++i;
    std::cout << "in thread 1" << std::endl;
    for (int j=0; j<SECOND*10; j++){

    }
  }
}

void thread2(void)
{
  int i = 0;
  std::cout << "Blocking thread " << uthread_get_tid() << std::endl;
  uthread_block(uthread_get_tid());
  while (1)
  {
    ++i;
    std::cout << "in thread 2" << std::endl;
    for (int j=0; j<SECOND*10; j++){

    }
  }
}

void thread3(void)
{
  int i = 0;
  while (i < 100)
  {
    ++i;
    std::cout << "in thread 3" << std::endl;
    for (int j=0; j<SECOND*10; j++){

    }
  }
  std::cout << "Terminating thread 3" << std::endl;
  uthread_terminate(uthread_get_tid());
}

void test1(){
  int res = uthread_init(100000);
  assert(res == 0);
  int res2 = uthread_spawn (thread0);
  assert(res2 == 1);
  int res3 = uthread_spawn (thread0);
  assert(res3 == 2);
  int res4 = uthread_terminate(1);
  assert(res4 == 0);
  int res5 = uthread_spawn (thread1);
  assert(res5 == 1);
  for (int i=0; i<100000; i++){
    for (int j=0; j<1000; j++){

    }
  }
  std::cout << "Blocking tid 1" << std::endl;
  uthread_block(1);
  for (int i=0; i<100000; i++){
    for (int j=0; j<1000; j++){

    }
  }
  std::cout << "Resuming tid 1" << std::endl;
  uthread_resume(1);
  for (int i=0; i<100000; i++){
    for (int j=0; j<1000; j++){

    }
  }
  std::cout << "total: " << uthread_get_total_quantums() << std::endl;
  std::cout << "main: "<< uthread_get_quantums(0) << std::endl;
  uthread_terminate (0);
  assert(false);
}

void test2(){
  uthread_init(100000);
  uthread_spawn (thread1);
  uthread_spawn (thread2);
  for (int i=0; i<100000; i++){
    for (int j=0; j<5000; j++){

    }
  }
  std::cout << "Resuming tid 2" << std::endl;
  uthread_resume (2);
  for (int i=0; i<100000; i++){
    for (int j=0; j<1000; j++){

    }
  }
  uthread_terminate(0);
}

void test3(){
  uthread_init(100000);
  uthread_spawn (thread1);
  uthread_spawn (thread3);
  for (int i=0; i<100000; i++){
    for (int j=0; j<5000; j++){

    }
  }
  uthread_spawn (thread0);
  for (int i=0; i<100000; i++){
    for (int j=0; j<5000; j++){

    }
  }
  uthread_terminate (0);
}

void test4(){
  uthread_init(100000);
  uthread_spawn (thread0);
  for (int i=0; i<100000; i++){
    for (int j=0; j<1000; j++){

    }
  }
  std::cout << "Blocking thread 0" << std::endl;
  assert(uthread_block (1) == 0);
  uthread_spawn (thread1);
  for (int i=0; i<100000; i++){
    for (int j=0; j<5000; j++){

    }
  }
  std::cout << "Resuming thread 0" << std::endl;
  assert(uthread_block (1) == 0);
  uthread_resume (1);
  assert(uthread_resume (2) == 0);
  for (int i=0; i<100000; i++){
    for (int j=0; j<5000; j++){

    }
  }
  uthread_terminate (0);
}

void test5(){
  uthread_init(100000);
  uthread_spawn (thread0);
  for (int i=0; i<100000; i++){
    for (int j=0; j<1000; j++){

    }
  }
  std::cout << "Blocking thread 0" << std::endl;
  assert(uthread_block (1) == 0);
  uthread_spawn (thread1);
  std::cout << "Resuming thread 0" << std::endl;
  assert(uthread_block (1) == 0);
  uthread_resume (1);
  for (int i=0; i<100000; i++){
    for (int j=0; j<5000; j++){

    }
  }
  assert(uthread_resume (2) == 0);
  for (int i=0; i<100000; i++){
    for (int j=0; j<5000; j++){

    }
  }
  uthread_terminate (0);
}

void test6(){
  uthread_init (10);
  for (int i=0; i<10; i++){
    std::cout << "main thread - running for " << uthread_get_quantums
    (uthread_get_tid()) << " quantums" << std::endl;
    for (int j=0; j<SECOND*3; j++){

    }
  }
  uthread_terminate (0);
}

int main(){
  test5();
  return EXIT_SUCCESS;
}