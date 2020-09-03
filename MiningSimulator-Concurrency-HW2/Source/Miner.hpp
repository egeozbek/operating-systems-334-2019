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

typedef enum MinerStatus {
    RUNNING,
    STOPPED,
    FINISHED
} MinerStatus;

class Miner
{
public:
    MinerInfo * minerInfo;
    unsigned int maxProduce;
    unsigned int minerSleepTime;
    MinerStatus minerStatus;
    string minerCapacityUniqueName;
    string minerMutexUniqueName;
    bool firstOre;
    
    
    unsigned int numberOfExtractedOres;
    
    sem_t * mutex;
    sem_t * minerNotFullSemaphore;
    
    Miner(MinerInfo * minerInfo,unsigned int maxProduce,unsigned int minerSleepTime);
    ~Miner();
    
};
