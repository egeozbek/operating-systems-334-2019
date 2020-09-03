//id is passed as i, but screen shown id is i+1
#include "Miner.hpp"
#include <iostream>
using namespace std;

Miner::Miner(MinerInfo * minerInfo,unsigned int maxProduce,unsigned int minerSleepTime):minerInfo(minerInfo),maxProduce(maxProduce),minerSleepTime(minerSleepTime)
{
    minerStatus = RUNNING;
    minerCapacityUniqueName = "MinerCapacity"+to_string(minerInfo->ID);
    minerMutexUniqueName = "MinerMutex"+to_string(minerInfo->ID);
    
    sem_unlink(minerCapacityUniqueName.c_str());
    sem_unlink(minerMutexUniqueName.c_str());
    
    minerNotFullSemaphore = sem_open(minerCapacityUniqueName.c_str(),O_CREAT,0600,minerInfo->capacity);
    mutex = sem_open(minerMutexUniqueName.c_str(),O_CREAT,0600,1);
    
    numberOfExtractedOres=0;
    firstOre = true;
}
Miner::~Miner()
{
    sem_close(minerNotFullSemaphore);
    sem_close(mutex);
}
