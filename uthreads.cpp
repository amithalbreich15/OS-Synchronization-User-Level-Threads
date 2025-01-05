//
// Created by yuval on 4/3/2023.
//
#include "uthreads.h"
#include "Thread.h"
#include "ThreadManager.h"
#include "algorithm"
#include <cassert>


#define TO_BLOCK 1
#define TO_SLEEP 2
#define TO_TERMINATE 3
#define LIBRARY_SUCCESS 0
#define LIBRARY_FAILURE -1

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
  address_t ret;
  asm volatile("xor    %%fs:0x30,%0\n"
               "rol    $0x11,%0\n"
      : "=g" (ret)
      : "0" (addr));
  return ret;
}

#else

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5


address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}


#endif

ThreadManager manager;
sigset_t block_set;

void switch_threads (int sig)
{
  /**
   * switches context from the running thread to the top thread in the ready
   * queue. if sig==TO_BLOCK, running thread is moved to "BLOCKED"
   */
  int next_tid = manager.ready_queue->front();
  manager.ready_queue->erase(manager.ready_queue->begin());
  if (sig == TO_SLEEP)
  {
    (*manager.threads)[manager.running_tid]->state = "READY";
  }
  if (sig == TO_BLOCK){
    (*manager.threads)[manager.running_tid]->state = "BLOCKED";
  }
  if (sig != TO_BLOCK && sig != TO_SLEEP && sig != TO_TERMINATE){
    manager.ready_queue->push_back (manager.running_tid);
    (*manager.threads)[manager.running_tid]->state = "READY";
  }
  manager.running_tid = next_tid;
  (*manager.threads)[next_tid]->state = "RUNNING";
  (*manager.threads)[manager.running_tid]->quantum_count++;
  manager.set_timer();
  sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
  siglongjmp((*manager.threads)[next_tid]->buf, 1);
}

void sleep_handler ()
{
  // go over all sleeping threads, decrease sleep time by 1 and revert to READY
  // if sleep time is over
  for (const auto item : *manager.threads){
    if (item.second->quantum_sleep > 0){
      item.second->quantum_sleep--;
      if (item.second->quantum_sleep == 0 && item.second->state == "READY"){
        manager.ready_queue->push_back (item.first);
      }
    }
  }
}

void schedule(int sig){
  /**
   * This function fetches the next ready thread, switches context between
   * the current thread and the new thread and starts a new quantum. It is
   * called whenever a timer signal is triggered or when a thread is
   * blocking/terminating itself.
   */
  sigprocmask(SIG_BLOCK, &block_set, nullptr);
  manager.total_quantums++;
  bool outgoing_thread = true;
  if (sig != TO_TERMINATE){
    int ret_val = sigsetjmp((*manager.threads)[manager.running_tid]->buf, 1);
    if (ret_val != 0){
      outgoing_thread = false;
    }
  }

  // if the current thread is the first thread (before context switch), handle
  // sleep time values and switch context if necessary
  if (outgoing_thread)
  {
    sleep_handler ();
    if (manager.ready_queue->empty()){
      (*manager.threads)[manager.running_tid]->quantum_count++;
    }
    else{
      switch_threads (sig);
    }
  }

  // if the current thread is the last thread (after context switch) and the
  // previous thread terminated itself, free its resources
  else{
    if (manager.thread_to_delete != nullptr){
      delete [] manager.thread_to_delete->sp;
      delete manager.thread_to_delete;
      manager.thread_to_delete = nullptr;
    }
  }
  sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
}

void config_thread (char *stack, address_t sp, address_t pc, int next_tid,
                    Thread *thread)
{
  /**
   * Receives a newly allocated Thread pointer and parameters, configures the
   * thread to hold the parameters and pushes it to the ready queue
   */
  thread->sp = stack;
  (*manager.threads)[next_tid] = thread;

  sigsetjmp((*manager.threads)[thread->tid]->buf, 1);
  ((*manager.threads)[thread->tid]->buf->__jmpbuf)[JB_SP] =
      translate_address(sp);
  ((*manager.threads)[thread->tid]->buf->__jmpbuf)[JB_PC] =
      translate_address(pc);
  sigemptyset(&(*manager.threads)[thread->tid]->buf->__saved_mask);
  manager.ready_queue->push_back(thread->tid);
}

void remove_thread (int tid)
{
  /**
   * Receives a tid, frees all resources allocated for the thread with given
   * tid and removes it from the thread pool and the ready queue. If the
   * thread is terminating itself, then resources will be deleted only in the
   * next quantum.
   */
  if (manager.running_tid == tid){
    manager.thread_to_delete = (*manager.threads)[tid];
  }
  else{
    delete [] (*manager.threads)[tid]->sp;
    delete (*manager.threads)[tid];
  }
  manager.threads->erase (tid);
  manager.ready_queue->erase(std::remove(manager.ready_queue->begin(),
                                         manager.ready_queue->end(), tid),
                             manager.ready_queue->end());
}


int uthread_init (int quantum_usecs){
  sigemptyset(&block_set);
  sigaddset(&block_set, SIGVTALRM);
  sigprocmask(SIG_BLOCK, &block_set, nullptr);
  if (quantum_usecs <= 0)
  {
    std::cerr << LIB_ERR << QUANTUM_ERR << std::endl;
    sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
    return LIBRARY_FAILURE;
  }
  manager.set_quantum (quantum_usecs);
  manager.sa = {nullptr};
  manager.sa.sa_handler = &schedule;
  if (sigaction(SIGVTALRM, &(manager.sa), nullptr) < 0)
  {
    std::cerr << SYS_ERR << SIGACTION_ERR << std::endl;
    manager.free_resources();
    exit(EXIT_FAILURE);
  }
  manager.set_timer();
  sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
  return LIBRARY_SUCCESS;
}

int uthread_spawn (thread_entry_point entry_point){
  sigprocmask(SIG_BLOCK, &block_set, nullptr);
  if (manager.threads->size() >= MAX_THREAD_NUM){
    std::cerr << LIB_ERR << MAX_THREADS_ERR << std::endl;
    sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
    return LIBRARY_FAILURE;
  }
  if (entry_point == nullptr){
    std::cerr << LIB_ERR << ENTRY_POINT_ERR << std::endl;
    sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
    return LIBRARY_FAILURE;
  }
  char *stack = new char[STACK_SIZE];
  if (stack == nullptr){
    std::cerr << SYS_ERR << ALLOC_ERR << std::endl;
    manager.free_resources();
    exit(EXIT_FAILURE);
  }
  address_t sp = (address_t) stack + STACK_SIZE - 1;
  address_t pc = (address_t) entry_point;

  //find next available tid
  int next_tid = 0;
  while (manager.threads->find (next_tid) != manager.threads->end()){
    next_tid ++;
  }

  Thread *thread = new Thread(next_tid);
  if (thread == nullptr){
    std::cerr << SYS_ERR << ALLOC_ERR << std::endl;
    delete [] stack;
    manager.free_resources();
    exit(EXIT_FAILURE);
  }
  config_thread (stack, sp, pc, next_tid, thread);

  sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
  return thread->tid;
}


int uthread_terminate(int tid){
  sigprocmask(SIG_BLOCK, &block_set, nullptr);
  if  (manager.threads->find (tid) == manager.threads->end() || tid < 0 ||
          tid >= MAX_THREAD_NUM )
  {
    std::cerr << LIB_ERR << TID_ERR << std::endl;
    sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
    return LIBRARY_FAILURE;
  }

  // if main thread is terminated, release all thread resources and exit
  if (tid == 0){
    manager.free_resources();
    exit(EXIT_SUCCESS);
  }

  remove_thread (tid);
  if (tid == manager.running_tid){
    sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
    schedule(TO_TERMINATE);
  }
  sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
  return LIBRARY_SUCCESS;
}

int uthread_block(int tid){
  sigprocmask(SIG_BLOCK, &block_set, nullptr);
  if (manager.threads->find (tid) == manager.threads->end() || tid <= 0 ||
        tid >= MAX_THREAD_NUM ){
    std::cerr << LIB_ERR << TID_ERR << std::endl;
    sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
    return -1;
  }
  if ((*manager.threads)[tid]->state == "BLOCKED"){
    sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
    return EXIT_SUCCESS;
  }
  if (tid == manager.running_tid){
    sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
    schedule (TO_BLOCK);
  }
  else{
    manager.ready_queue->erase(std::remove(manager.ready_queue->begin(),
                                          manager.ready_queue->end(), tid),
                              manager.ready_queue->end());
    (*manager.threads)[tid]->state = "BLOCKED";
  }
  sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
  return LIBRARY_SUCCESS;
}

int uthread_resume(int tid){
  sigprocmask(SIG_BLOCK, &block_set, nullptr);
  if (manager.threads->find (tid) == manager.threads->end() || tid <= 0 ||
      tid >= MAX_THREAD_NUM ){
    std::cerr << LIB_ERR << TID_ERR << std::endl;
    sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
    return LIBRARY_FAILURE;
  }
  if ((*manager.threads)[tid]->state == "BLOCKED")
  {
    (*manager.threads)[tid]->state = "READY";
    if ((*manager.threads)[tid]->quantum_sleep == 0){
      manager.ready_queue->push_back (tid);
    }
  }
  sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
  return LIBRARY_SUCCESS;
}

int uthread_sleep(int num_quantums){
  sigprocmask(SIG_BLOCK, &block_set, nullptr);
  if (manager.running_tid == 0){
    std::cerr << LIB_ERR << MAIN_THREAD_ERR << std::endl;
    sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
    return LIBRARY_FAILURE;
  }
  if (num_quantums < 0){
    std::cerr << LIB_ERR << QUANTUM_ERR << std::endl;
    sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
    return LIBRARY_FAILURE;
  }

  // current quantum is not counted so initial quantum should be num_quantums + 1
  (*manager.threads)[manager.running_tid]->quantum_sleep = num_quantums + 1;
  sigprocmask(SIG_UNBLOCK, &block_set, nullptr);
  schedule (TO_SLEEP);
  return LIBRARY_SUCCESS;
}

int uthread_get_tid(){
  return manager.running_tid;
}

int uthread_get_total_quantums(){
  return manager.total_quantums;
}

int uthread_get_quantums(int tid){
  if (manager.threads->find (tid) == manager.threads->end() || tid < 0 ||
      tid >= MAX_THREAD_NUM ){
    std::cerr << LIB_ERR << TID_ERR << std::endl;
    return LIBRARY_FAILURE;
  }
  return (*manager.threads)[tid]->quantum_count;
}
