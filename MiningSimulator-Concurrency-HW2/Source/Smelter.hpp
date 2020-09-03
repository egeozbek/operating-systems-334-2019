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

class Smelter
{
public:
    bool isAlive;
    Smelter(SmelterInfo * smelterInfo , unsigned int sleepTime);
    ~Smelter();
    SmelterInfo * smelterInfo;
    unsigned int smelterSleepTime;
    
    string smelterMutexUniqueName;
    string smelterArrivedUniqueName;
    
    sem_t * mutex;
    sem_t * smelterArrivedSemaphore;
    
};
