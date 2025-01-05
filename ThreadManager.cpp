//
// Created by yuval on 4/6/2023.
//
#include "ThreadManager.h"

ThreadManager::ThreadManager (): quantum(0), running_tid(0),
                                 ready_queue(), threads(), total_quantums(1)
                                 , thread_to_delete(nullptr)
{
  ready_queue = new std::vector<int>;
  if (ready_queue == nullptr){
    std::cerr << SYS_ERR << ALLOC_ERR << std::endl;
    exit(EXIT_FAILURE);
  }
  threads = new std::map<int, Thread*>;
  if (threads == nullptr){
    delete ready_queue;
    std::cerr << SYS_ERR << ALLOC_ERR << std::endl;
    exit(EXIT_FAILURE);
  }
  Thread *main_thread = new Thread(0);
  if (main_thread == nullptr){
    delete ready_queue;
    delete threads;
    std::cerr << SYS_ERR << ALLOC_ERR << std::endl;
    exit(EXIT_FAILURE);
  }
  main_thread->state = "RUNNING";
  main_thread->quantum_count = 1;
  (*threads)[0] = main_thread;
}

void ThreadManager::free_resources(){
  for (const auto item : *threads){
    if (item.second->tid != 0){
      delete [] item.second->sp;
    }
    delete item.second;
  }
  delete ready_queue;
  delete threads;
}

void ThreadManager::set_timer(){
  timer.it_value.tv_sec = floor(quantum / pow(10, 6));
  timer.it_value.tv_usec = quantum % (int) pow(10, 6);
  timer.it_interval.tv_sec = floor(quantum / pow(10, 6));
  timer.it_interval.tv_usec = quantum % (int) pow(10, 6);
  if (setitimer(ITIMER_VIRTUAL, &timer, nullptr))
  {
    std::cerr << SYS_ERR << TIMER_ERR << std::endl;
    free_resources();
    exit(EXIT_FAILURE);
  }
}

void ThreadManager::set_quantum(int q){
  quantum = q;
}
