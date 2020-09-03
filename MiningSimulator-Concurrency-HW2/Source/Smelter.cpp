#include "Smelter.hpp"

Smelter::Smelter(SmelterInfo * smelterInfo,unsigned int sleepTime):smelterInfo(smelterInfo),smelterSleepTime(sleepTime)
{
    isAlive = true;
    smelterMutexUniqueName = "SmelterMutex"+to_string(smelterInfo->ID);
    smelterArrivedUniqueName = "SmelterArrived"+to_string(smelterInfo->ID);
    
    sem_unlink(smelterMutexUniqueName.c_str());
    sem_unlink(smelterArrivedUniqueName.c_str());
    
    mutex = sem_open(smelterMutexUniqueName.c_str(),O_CREAT,0600,1);
    smelterArrivedSemaphore = sem_open(smelterArrivedUniqueName.c_str(),O_CREAT,0600,0);
}
Smelter::~Smelter()
{
    sem_close(mutex);
    sem_close(smelterArrivedSemaphore);
}
