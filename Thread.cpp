//
// Created by yuval on 4/6/2023.
//
#include "Thread.h"


Thread::Thread (int id): tid(id), state("READY"), sp(nullptr),
quantum_sleep(0), quantum_count(0)
{
}
