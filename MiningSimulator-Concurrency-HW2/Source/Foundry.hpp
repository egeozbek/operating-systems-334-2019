#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<fcntl.h>
#include<errno.h>
#include "writeOutput.h"
#include <set>
#include <iterator>
#include <string>

using namespace std;

class Foundry
{
public:
    bool isAlive;
    Foundry(FoundryInfo * foundryInfo , unsigned int sleepTime);
    ~Foundry();
    FoundryInfo * foundryInfo;
    unsigned int foundrySleepTime;
    
    string foundryMutexUniqueName;
    string foundryIronArrivedUniqueName;
    string foundryCoalArrivedUniqueName;
    
    sem_t * mutex;
    sem_t * foundryIronArrivedSemaphore;
    sem_t * foundryCoalArrivedSemaphore;
    
};
