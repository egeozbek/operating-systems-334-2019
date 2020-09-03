#include "Foundry.hpp"

Foundry::Foundry(FoundryInfo * foundryInfo,unsigned int sleepTime):foundryInfo(foundryInfo),foundrySleepTime(sleepTime)
{
    isAlive = true;
    foundryMutexUniqueName = "FoundryMutex"+to_string(foundryInfo->ID);
    foundryIronArrivedUniqueName = "FoundryIronArrived"+to_string(foundryInfo->ID);
    foundryCoalArrivedUniqueName = "FoundryIronArrived"+to_string(foundryInfo->ID);
    
    
    sem_unlink(foundryMutexUniqueName.c_str());
    sem_unlink(foundryIronArrivedUniqueName.c_str());
    sem_unlink(foundryCoalArrivedUniqueName.c_str());
    
    mutex = sem_open(foundryMutexUniqueName.c_str(),O_CREAT,0600,1);
    foundryIronArrivedSemaphore = sem_open(foundryIronArrivedUniqueName.c_str(),O_CREAT,0600,0);
    foundryCoalArrivedSemaphore = sem_open(foundryCoalArrivedUniqueName.c_str(),O_CREAT,0600,0);
}
Foundry::~Foundry()
{
    sem_close(mutex);
    sem_close(foundryIronArrivedSemaphore);
    sem_close(foundryCoalArrivedSemaphore);
}
