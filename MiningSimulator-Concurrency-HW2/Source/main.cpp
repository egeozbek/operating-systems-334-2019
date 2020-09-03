#include <iostream>
#include "writeOutput.h"
#include "Miner.hpp"
#include "Transporter.hpp"
#include "Smelter.hpp"
#include "Foundry.hpp"
#include <pthread.h>
#include <vector>
#include "writeOutput.h"

//TODO clean up comments

using namespace std;

unsigned int i;
int lastCheckedMiner = -1;
int semaphoreValue;

vector<Miner *> minerVector;
vector<Transporter *> transporterVector;
vector<Smelter *> smelterVector;
vector<Foundry *> foundryVector;

vector<unsigned int> copperSmelterIds;
vector<unsigned int> ironSmelterIds;


sem_t * availableOreSemaphore;
sem_t * availableCopperInSmelterSemaphore;
sem_t * availableIronInSmelterOrFoundrySemaphore;
sem_t * availableCoalInFoundrySemaphore;

int numberOfNotFinishedMiners;
int numberOfMiners;
int numberOfTransporters;
int numberOfSmelters;
int numberOfFoundries;


sem_t * globalMinerMutex;
sem_t * globalCopperMutex;
sem_t * globalIronMutex;
sem_t * globalCoalMutex;
sem_t * transporterBarrierSemaphore;

void closeSemaphores()
{
    sem_close(transporterBarrierSemaphore);
    sem_close(availableOreSemaphore);
    sem_close(globalMinerMutex);
    sem_close(globalCopperMutex);
    sem_close(globalIronMutex);
    sem_close(globalCoalMutex);
    sem_close(availableCopperInSmelterSemaphore);
    sem_close(availableIronInSmelterOrFoundrySemaphore);
    sem_close(availableCoalInFoundrySemaphore);
}

void initializeSemaphores()
{
    sem_unlink("globalMinerMutex");
    globalMinerMutex = sem_open("globalMinerMutex",O_CREAT,0600,1);
    sem_unlink("availableOreSemaphore");
    availableOreSemaphore = sem_open("availableOreSemaphore",O_CREAT,0600,0);
    sem_unlink("transporterBarrierSemaphore");
    transporterBarrierSemaphore = sem_open("transporterBarrierSemaphore",O_CREAT,0600,0);
    sem_unlink("globalCopperMutex");
    globalCopperMutex = sem_open("globalCopperMutex",O_CREAT,0600,1);
    sem_unlink("globalIronMutex");
    globalIronMutex = sem_open("globalIronMutex",O_CREAT,0600,1);
    sem_unlink("globalCoalMutex");
    globalCoalMutex = sem_open("globalCoalMutex",O_CREAT,0600,1);
    sem_unlink("availableCopperInSmelterSemaphore");
    availableCopperInSmelterSemaphore = sem_open("availableCopperInSmelterSemaphore",O_CREAT,0600,(unsigned int )copperSmelterIds.size());
    sem_unlink("availableIronInSmelterOrFoundrySemaphore");
    unsigned int availableIronTargets = (unsigned int )ironSmelterIds.size()+(unsigned int)foundryVector.size();
    availableIronInSmelterOrFoundrySemaphore = sem_open("availableIronInSmelterOrFoundrySemaphore",O_CREAT,0600,availableIronTargets);
    sem_unlink("availableIronInSmelterOrFoundrySemaphore");
    cout<<"AvailableCoal in semaphore value is : "<<foundryVector.size()<<endl;
    cout<<"AvailableIron in semaphore value is : "<<availableIronTargets<<endl;
    availableCoalInFoundrySemaphore = sem_open("availableCoalInFoundrySemaphore",O_CREAT,0600,(unsigned int)foundryVector.size());
    
}

void sleepWithVariation(unsigned int sleepLength)
{
    usleep(sleepLength -(sleepLength*0.01) +(rand()%(int)(sleepLength*0.02)));
}
void getInput()
{
    cin >> numberOfMiners;
    numberOfNotFinishedMiners = numberOfMiners;
    
    for(i=0;i<numberOfMiners;i++)
    {
        //id no is i+1
        unsigned int tempSleepTime;
        unsigned int tempStorageCapacity;
        unsigned int tempOreType;
        unsigned int tempMaxProduce;
        cin >> tempSleepTime;
        cin >> tempStorageCapacity;
        cin >> tempOreType;
        cin >> tempMaxProduce;
        
        MinerInfo * tempInfo = new MinerInfo;
        FillMinerInfo(tempInfo,i+1,(OreType)tempOreType,tempStorageCapacity,0);
        Miner * tempMiner = new Miner(tempInfo,tempMaxProduce,tempSleepTime);
        minerVector.push_back(tempMiner);
    }
    cin >> numberOfTransporters;
    for(i=0;i<numberOfTransporters;i++)
    {
        unsigned int tempSleepTime;
        cin >>tempSleepTime;
        TransporterInfo * tempInfo = new TransporterInfo;
        FillTransporterInfo(tempInfo,i+1,NULL);
        Transporter * tempTransporter = new Transporter(tempInfo,tempSleepTime);
        transporterVector.push_back(tempTransporter);
    }
    cin >> numberOfSmelters;
    for(i = 0 ;i< numberOfSmelters ; i++)
    {
        unsigned int tempSleepTime;
        unsigned int tempStorageCapacity;
        unsigned int tempOreType;
        cin >> tempSleepTime;
        cin >> tempStorageCapacity;
        cin >> tempOreType;
        cout<<"Smelter ore type is : "<<tempOreType<<endl;
        if(tempOreType == IRON) //IRON = 0
        {
            ironSmelterIds.push_back(i); //just their original ids in smelterVector;
        }
        else if(tempOreType == COPPER) //COPPER is
        {
            copperSmelterIds.push_back(i);
        }
        
        SmelterInfo * tempInfo = new SmelterInfo;
        FillSmelterInfo(tempInfo,i+1,(OreType)tempOreType,tempStorageCapacity,0,0);
        Smelter * tempSmelter = new Smelter(tempInfo,tempSleepTime);
        smelterVector.push_back(tempSmelter);
    }
    cin >>numberOfFoundries;
    for(i = 0 ; i<numberOfFoundries ; i++)
    {
        unsigned int tempSleepTime;
        unsigned int tempStorageCapacity;
        cin >> tempSleepTime;
        cin >> tempStorageCapacity;
        FoundryInfo * tempInfo = new FoundryInfo; //TODO add delete FoundryInfo to destructors
        FillFoundryInfo(tempInfo,i+1,tempStorageCapacity,0,0,0);
        Foundry * tempFoundry = new Foundry(tempInfo,tempSleepTime);
        foundryVector.push_back(tempFoundry);
    }
}

void * minerThreadFunction(void * minerptr) //DONE
{
    Miner * miner;
    miner = (Miner *) minerptr;
    
    FillMinerInfo(miner->minerInfo,miner->minerInfo->ID,miner->minerInfo->oreType,miner->minerInfo->capacity,miner->minerInfo->current_count);
    WriteOutput(miner->minerInfo,NULL,NULL,NULL,MINER_CREATED);
    while(miner->numberOfExtractedOres < miner->maxProduce)
    {
        // cout<<"Just entered while in miner thread"<<endl;
        sem_wait(miner->minerNotFullSemaphore);
        // cout<<"Passed sem wait"<<endl;
        FillMinerInfo(miner->minerInfo,miner->minerInfo->ID,miner->minerInfo->oreType,miner->minerInfo->capacity,miner->minerInfo->current_count);
        // cout<<"Filled miner info"<<endl;
        WriteOutput(miner->minerInfo,NULL,NULL,NULL,MINER_STARTED);
        // cout<<"write output"<<endl;
        sleepWithVariation(miner->minerSleepTime);
        
        sem_wait(miner->mutex);
        miner->minerInfo->current_count++;
        miner->numberOfExtractedOres++;
        sem_post(availableOreSemaphore);
        if(miner->firstOre)
        {
            // sem_getvalue(transporterBarrierSemaphore,&semaphoreValue);
            // cout<<"Miner Before Barrier"<<"barrier semaphore value is "<<semaphoreValue<<endl;
            sem_post(transporterBarrierSemaphore);
            // cout<<"Miner Posted Barrier"<<endl;
            miner->firstOre = false;
        }
        
        // cout<<"Miner current count is "<<miner->minerInfo->current_count<<endl;
        FillMinerInfo(miner->minerInfo,miner->minerInfo->ID,miner->minerInfo->oreType,miner->minerInfo->capacity,miner->minerInfo->current_count);
        WriteOutput(miner->minerInfo,NULL,NULL,NULL,MINER_FINISHED);
        sem_post(miner->mutex);
        sleepWithVariation(miner->minerSleepTime);
    }
    sem_wait(miner->mutex);
    cout<<"*******STOPPED***********"<<endl;
    miner->minerStatus=STOPPED; // not necessary since decrement is only called by manager
    FillMinerInfo(miner->minerInfo,miner->minerInfo->ID,miner->minerInfo->oreType,miner->minerInfo->capacity,miner->minerInfo->current_count);
    // cout<<"Miner Before Stopped"<<endl;
    WriteOutput(miner->minerInfo,NULL,NULL,NULL,MINER_STOPPED);
    sem_post(miner->mutex);
    // cout<<"DEBUG MinerID is: "<<miner->ID<<endl;
    return 0;
}


void * transporterThreadFunction(void * transporterptr)
{
    Transporter * transporter = (Transporter *) transporterptr;
    FillTransporterInfo(transporter->transporterInfo,transporter->transporterInfo->ID,NULL);
    WriteOutput(NULL,transporter->transporterInfo,NULL,NULL,TRANSPORTER_CREATED);
    
    sem_wait(transporterBarrierSemaphore); // wait for at least one miner to produce
    // cout<<"Transporter woke up from sleep: id : "<<transporter->transporterInfo->ID<<endl;
    sem_post(transporterBarrierSemaphore); //signal other transporters to wake up
    
    while(true) // TODO here we require a
    {
        //buraya girdiğinde net bulması lazım
        // cout<<"Transporter in while"<<endl;
        transporter->transporterInfo->carry=NULL;
        sem_wait(availableOreSemaphore); // waits here for available ore
        if(numberOfNotFinishedMiners==0)
        {
            //no need to post semaphore here since we already posted numberOfTransporters times in exit condition
            break;
        }
        // cout<<"There is a non-finished miner available"<<endl;
        sem_wait(globalMinerMutex);
        bool foundAvailable = false;
        cout<<"Transporter got global miner mutex"<<endl;
        if(lastCheckedMiner+1==numberOfMiners)
        {
            for(unsigned int j = 0; i<numberOfMiners;j++)
            {
                sem_wait(minerVector[j]->mutex);
                if(minerVector[j]->minerInfo->current_count>0)
                {
                    // cout<<"Transporter found its target in second for loop"<<endl;
                    transporter->lastCheckedMiner=j;
                    foundAvailable = true;
                    sem_post(minerVector[j]->mutex);
                    break;
                }
                else
                {
                    sem_post(minerVector[j]->mutex);
                }
            }
        }
        else
        {
            for(unsigned int j = (lastCheckedMiner+1)%numberOfMiners;j<numberOfMiners;j++)
            {
                sem_wait(minerVector[j]->mutex);
                if(minerVector[j]->minerInfo->current_count>0)
                {
                    // cout<<"Transporter found its target in first for loop"<<endl;
                    transporter->lastCheckedMiner=j;
                    foundAvailable = true;
                    sem_post(minerVector[j]->mutex);
                    break;
                }
                else
                {
                    sem_post(minerVector[j]->mutex);
                }
            }
            if(!foundAvailable)
            {
                for(unsigned int j = 0; j<transporter->lastCheckedMiner;j++)
                {
                    sem_wait(minerVector[j]->mutex);
                    if(minerVector[j]->minerInfo->current_count>0)
                    {
                        // cout<<"Transporter found its target in second for loop"<<endl;
                        transporter->lastCheckedMiner=j;
                        foundAvailable = true;
                        sem_post(minerVector[j]->mutex);
                        break;
                    }
                    else
                    {
                        sem_post(minerVector[j]->mutex);
                    }
                }
            }
        }
        // for(unsigned int j = 0; j<numberOfMiners;j++)
        //   {
        //     sem_wait(minerVector[j]->mutex);
        //     if(minerVector[j]->minerInfo->current_count>0)
        //     {
        //       // cout<<"Transporter found its target in second for loop"<<endl;
        //       transporter->lastCheckedMiner=j;
        //       foundAvailable = true;
        //       sem_post(minerVector[j]->mutex);
        //       break;
        //     }
        //     else
        //     {
        //       sem_post(minerVector[j]->mutex);
        //     }
        //   }
        if(!foundAvailable){
            cout<<"COULDN'T FIND A MINER"<<endl;
        }
        else{
            //I didnt want to rename everything
            cout<<"FOUND A MINER , GOING TO :"<<transporter->lastCheckedMiner<<endl;
            FillTransporterInfo(transporter->transporterInfo,transporter->transporterInfo->ID,NULL);
            cout<<"MALIM-0"<<endl;
            WriteOutput(minerVector[transporter->lastCheckedMiner]->minerInfo,transporter->transporterInfo,NULL,NULL,TRANSPORTER_TRAVEL);
            cout<<"MALIM1"<<endl;
            sem_wait(minerVector[transporter->lastCheckedMiner]->mutex);
            minerVector[transporter->lastCheckedMiner]->minerInfo->current_count--;
            OreType *newOre = new OreType();
            *newOre = minerVector[transporter->lastCheckedMiner]->minerInfo->oreType;
            transporter->transporterInfo->carry = newOre;
            cout<<"MALIM66666"<<*newOre<<endl;
            // cout<<"Transporter decremented miners ore"<<endl;
            // cout<<"Transporter decremented miner ore count is "<<minerVector[i]->minerInfo->current_count<<endl;
            sem_post(minerVector[transporter->lastCheckedMiner]->mutex);
            
            
            sleepWithVariation(transporter->transporterSleepTime);
            
            // after sleep notify miner that it has a empty space
            sem_post(minerVector[transporter->lastCheckedMiner]->minerNotFullSemaphore);
            
            FillMinerInfo(minerVector[transporter->lastCheckedMiner]->minerInfo,minerVector[transporter->lastCheckedMiner]->minerInfo->ID,minerVector[transporter->lastCheckedMiner]->minerInfo->oreType,minerVector[transporter->lastCheckedMiner]->minerInfo->capacity,minerVector[transporter->lastCheckedMiner]->minerInfo->current_count);
            // cout<<"MALIM5"<<endl;
            FillTransporterInfo(transporter->transporterInfo,transporter->transporterInfo->ID, (OreType*) (transporter->transporterInfo->carry));
            cout<<"MALIM6"<<endl;
            WriteOutput(minerVector[transporter->lastCheckedMiner]->minerInfo,transporter->transporterInfo,NULL,NULL,TRANSPORTER_TAKE_ORE);
            // cout<<"MALIM7"<<endl;
            sleepWithVariation(transporter->transporterSleepTime);
            
            // sem_post(availableOreSemaphore);
            sem_wait(minerVector[transporter->lastCheckedMiner]->mutex);
            if(minerVector[transporter->lastCheckedMiner]->minerInfo->current_count == 0 && minerVector[transporter->lastCheckedMiner]->minerStatus == STOPPED)
            {
                // sem_wait(availableOreSemaphore);
                numberOfNotFinishedMiners--;
                if(numberOfNotFinishedMiners==0) // end case wake up all waiting transporters
                {
                    for(unsigned int j=0;j<numberOfTransporters;j++)
                    {
                        sem_post(availableOreSemaphore);
                    }
                }
            }
            sem_post(minerVector[transporter->lastCheckedMiner]->mutex);
        }
        // cout<<"Found its tar get "<<i<<endl;
        sem_post(globalMinerMutex);
        
        if(*(transporter->transporterInfo->carry) == COPPER) //keep 3 semaphores, for each type of resource, if a ex. smelter is full decrement it, when its available again post it
        {
            cout<<"Before waiting for available smelter"<<endl;
            sem_wait(availableCopperInSmelterSemaphore); //to find a suitable smelter, else wait
            cout<<"After waiting for available smelter"<<endl;
            sem_wait(globalCopperMutex);
            FillTransporterInfo(transporter->transporterInfo,transporter->transporterInfo->ID,(OreType *) transporter->transporterInfo->carry);
            //TODO change this according to smelter availability and priority
            Smelter * smelter;
            bool priorityFound = false;
            //TODO I THINK THIS DOESN'T CHECK IF SMELTER DIED OR NOT!
            for(unsigned int j = 0; j<copperSmelterIds.size();j++)
            {
                sem_wait(smelterVector[copperSmelterIds[j]]->mutex);
                if(smelterVector[copperSmelterIds[j]]->isAlive && smelterVector[copperSmelterIds[j]]->smelterInfo->waiting_ore_count==1){
                    cout<<"Priority found, id is : "<<copperSmelterIds[j]<<endl;
                    smelter = smelterVector[copperSmelterIds[j]];
                    priorityFound  = true;
                    sem_post(smelterVector[copperSmelterIds[j]]->mutex);
                    break;
                }
                else
                {
                    sem_post(smelterVector[copperSmelterIds[j]]->mutex);
                }
            }
            if(!priorityFound)
            {
                cout<<"Priority smelter not found. Searching again for non-priority one."<<endl;
                for(unsigned int j = 0; j<copperSmelterIds.size();j++)
                {
                    sem_wait(smelterVector[copperSmelterIds[j]]->mutex);
                    if(smelterVector[copperSmelterIds[j]]->isAlive && smelterVector[copperSmelterIds[j]]->smelterInfo->waiting_ore_count!=smelterVector[copperSmelterIds[j]]->smelterInfo->loading_capacity){
                        smelter = smelterVector[copperSmelterIds[j]];
                        sem_post(smelterVector[copperSmelterIds[j]]->mutex);
                        break;
                    }
                    else
                    {
                        sem_post(smelterVector[copperSmelterIds[j]]->mutex);
                    }
                }
            }
            cout<<"MALIM 12"<<endl;
            WriteOutput(NULL,transporter->transporterInfo,smelter->smelterInfo,NULL,TRANSPORTER_TRAVEL);
            sleepWithVariation(transporter->transporterSleepTime);
            sem_wait(smelter->mutex);
            smelter->smelterInfo->waiting_ore_count++;
            sem_post(smelter->mutex);
            
            FillSmelterInfo(smelter->smelterInfo,smelter->smelterInfo->ID,smelter->smelterInfo->oreType,smelter->smelterInfo->loading_capacity,smelter->smelterInfo->waiting_ore_count,smelter->smelterInfo->total_produce);
            FillTransporterInfo(transporter->transporterInfo,transporter->transporterInfo->ID,(OreType *) transporter->transporterInfo->carry);
            cout<<"MALIM  10"<<endl;
            WriteOutput(NULL,transporter->transporterInfo,smelter->smelterInfo,NULL,TRANSPORTER_DROP_ORE);
            sleepWithVariation(transporter->transporterSleepTime);
            
            sem_post(smelter->smelterArrivedSemaphore); //Unloaded signal
            sem_wait(smelter->mutex);
            if(smelter->smelterInfo->loading_capacity != smelter->smelterInfo->waiting_ore_count)
            {
                sem_post(availableCopperInSmelterSemaphore);
            }
            sem_post(smelter->mutex);
            
            sem_post(globalCopperMutex);
        }
        else if(*(transporter->transporterInfo->carry) == IRON)
        {
            //TODO mutex here
            sem_wait(availableIronInSmelterOrFoundrySemaphore);
            sem_wait(globalIronMutex);
            FillTransporterInfo(transporter->transporterInfo,transporter->transporterInfo->ID,(OreType *) transporter->transporterInfo->carry);
            Smelter * smelter = NULL;
            Foundry * foundry = NULL;
            //maybe do a mod or remember which one you selected previously
            bool found = false;
            //TODO I THINK THIS DOESN'T CHECK IF SMELTER DIED OR NOT!
            for(unsigned int j = 0; j<ironSmelterIds.size();j++)
            {
                sem_wait(smelterVector[ironSmelterIds[j]]->mutex);
                if(smelterVector[ironSmelterIds[j]]->isAlive && smelterVector[ironSmelterIds[j]]->smelterInfo->waiting_ore_count==1){
                    cout<<"Priority found, id is : "<<ironSmelterIds[j]<<endl;
                    smelter = smelterVector[ironSmelterIds[j]];
                    found  = true;
                    sem_post(smelterVector[ironSmelterIds[j]]->mutex);
                    break;
                }
                else
                {
                    sem_post(smelterVector[ironSmelterIds[j]]->mutex);
                }
            }
            if(!found)
            {
                for(unsigned int j = 0; j<numberOfFoundries;j++)
                {
                    sem_wait(foundryVector[j]->mutex);
                    if(foundryVector[j]->isAlive && foundryVector[j]->foundryInfo->waiting_coal!=0 && foundryVector[j]->foundryInfo->waiting_iron==0) // if waiting iron is not 0 is it priority?
                    {
                        cout<<"Priority foundry found, id is : "<<foundryVector[j]->foundryInfo->ID<<endl;
                        foundry = foundryVector[j];
                        found  = true;
                        sem_post(foundryVector[j]->mutex);
                        break;
                    }
                    else
                    {
                        sem_post(foundryVector[j]->mutex);
                    }
                }
            }
            if(!found)
            {
                cout<<"Priority smelter not found for IRON. Searching again for non-priority one."<<endl;
                for(unsigned int j = 0; j<ironSmelterIds.size();j++)
                {
                    sem_wait(smelterVector[ironSmelterIds[j]]->mutex);
                    if(smelterVector[ironSmelterIds[j]]->isAlive && smelterVector[ironSmelterIds[j]]->smelterInfo->waiting_ore_count!=smelterVector[ironSmelterIds[j]]->smelterInfo->loading_capacity){
                        smelter = smelterVector[ironSmelterIds[j]];
                        found = true;
                        sem_post(smelterVector[ironSmelterIds[j]]->mutex);
                        break;
                    }
                    else
                    {
                        sem_post(smelterVector[ironSmelterIds[j]]->mutex);
                    }
                }
            }
            if(!found)
            {
                cout<<"Priority foundry not found for IRON. Searching again for non-priority one."<<endl;
                for(unsigned int j = 0; j<numberOfFoundries;j++)
                {
                    sem_wait(foundryVector[j]->mutex);
                    if(foundryVector[j]->isAlive && foundryVector[j]->foundryInfo->waiting_coal!=foundryVector[j]->foundryInfo->loading_capacity){
                        foundry = foundryVector[j];
                        found = true;
                        sem_post(foundryVector[j]->mutex);
                        break;
                    }
                    else
                    {
                        sem_post(foundryVector[j]->mutex);
                    }
                }
            }
            if(!found)
            {
                cout<<"Finding a good place for IRON failed"<<endl;
                break;
            }
            if(smelter!=NULL)
            {
                WriteOutput(NULL,transporter->transporterInfo,smelter->smelterInfo,NULL,TRANSPORTER_TRAVEL);
                sleepWithVariation(transporter->transporterSleepTime);
                sem_wait(smelter->mutex);
                smelter->smelterInfo->waiting_ore_count++;
                sem_post(smelter->mutex);
                FillSmelterInfo(smelter->smelterInfo,smelter->smelterInfo->ID,smelter->smelterInfo->oreType,smelter->smelterInfo->loading_capacity,smelter->smelterInfo->waiting_ore_count,smelter->smelterInfo->total_produce);
                FillTransporterInfo(transporter->transporterInfo,transporter->transporterInfo->ID,(OreType *) transporter->transporterInfo->carry);
                WriteOutput(NULL,transporter->transporterInfo,smelter->smelterInfo,NULL,TRANSPORTER_DROP_ORE);
                sleepWithVariation(transporter->transporterSleepTime);
                sem_post(smelter->smelterArrivedSemaphore);
                sem_wait(smelter->mutex);
                if(smelter->smelterInfo->loading_capacity != smelter->smelterInfo->waiting_ore_count)
                {
                    sem_post(availableIronInSmelterOrFoundrySemaphore);
                }
                sem_post(smelter->mutex);
                sem_post(globalIronMutex);
            }
            else if(foundry!=NULL)
            {
                WriteOutput(NULL,transporter->transporterInfo,NULL,foundry->foundryInfo,TRANSPORTER_TRAVEL);
                sleepWithVariation(transporter->transporterSleepTime);
                sem_wait(foundry->mutex);
                foundry->foundryInfo->waiting_iron++;
                sem_post(foundry->mutex);
                FillFoundryInfo(foundry->foundryInfo,foundry->foundryInfo->ID,foundry->foundryInfo->loading_capacity,foundry->foundryInfo->waiting_iron,foundry->foundryInfo->waiting_coal,foundry->foundryInfo->total_produce);
                FillTransporterInfo(transporter->transporterInfo,transporter->transporterInfo->ID,(OreType *) transporter->transporterInfo->carry);
                WriteOutput(NULL,transporter->transporterInfo,NULL,foundry->foundryInfo,TRANSPORTER_DROP_ORE);
                sleepWithVariation(transporter->transporterSleepTime);
                sem_post(foundry->foundryIronArrivedSemaphore);
                sem_wait(foundry->mutex);
                if(foundry->foundryInfo->loading_capacity != foundry->foundryInfo->waiting_iron)
                {
                    sem_post(availableIronInSmelterOrFoundrySemaphore);
                }
                sem_post(foundry->mutex);
                sem_post(globalIronMutex);
            }
            else
            {
                cout<<"Fuckup"<<endl;
            }
        }
        else if(*(transporter->transporterInfo->carry) == COAL)
        {
            cout<<"Before waiting for coal available foundry"<<endl;
            sem_wait(availableCoalInFoundrySemaphore); //to find a suitable smelter, else wait
            cout<<"After waiting for coal available foundry"<<endl;
            sem_wait(globalCoalMutex);
            FillTransporterInfo(transporter->transporterInfo,transporter->transporterInfo->ID,(OreType *) transporter->transporterInfo->carry);
            Foundry * foundry;
            bool priorityFound = false;
            for(unsigned int j = 0; j<numberOfFoundries;j++)
            {
                sem_wait(foundryVector[j]->mutex);
                if(foundryVector[j]->isAlive && foundryVector[j]->foundryInfo->waiting_iron!=0 && foundryVector[j]->foundryInfo->waiting_coal==0){
                    cout<<"Priority foundry found, id is : "<<foundryVector[j]->foundryInfo->ID<<endl;
                    foundry = foundryVector[j];
                    priorityFound  = true;
                    sem_post(foundryVector[j]->mutex);
                    break;
                }
                else
                {
                    sem_post(foundryVector[j]->mutex);
                }
            }
            if(!priorityFound)
            {
                cout<<"Priority foundry not found. Searching again for non-priority one."<<endl;
                for(unsigned int j = 0; j<numberOfFoundries;j++)
                {
                    sem_wait(foundryVector[j]->mutex);
                    if(foundryVector[j]->foundryInfo->waiting_coal!=foundryVector[j]->foundryInfo->loading_capacity){
                        foundry = foundryVector[j];
                        sem_post(foundryVector[j]->mutex);
                        break;
                    }
                    else
                    {
                        sem_post(foundryVector[j]->mutex);
                    }
                }
            }
            if(!priorityFound)
            {
                cout<<"NO FOUNDRY FOUND!"<<endl;
            }
            
            WriteOutput(NULL,transporter->transporterInfo,NULL,foundry->foundryInfo,TRANSPORTER_TRAVEL);
            sleepWithVariation(transporter->transporterSleepTime);
            
            sem_wait(foundry->mutex);
            foundry->foundryInfo->waiting_coal++;
            sem_post(foundry->mutex);
            
            FillFoundryInfo(foundry->foundryInfo,foundry->foundryInfo->ID,foundry->foundryInfo->loading_capacity,foundry->foundryInfo->waiting_iron,foundry->foundryInfo->waiting_coal,foundry->foundryInfo->total_produce);
            FillTransporterInfo(transporter->transporterInfo,transporter->transporterInfo->ID,(OreType *) transporter->transporterInfo->carry);
            WriteOutput(NULL,transporter->transporterInfo,NULL,foundry->foundryInfo,TRANSPORTER_DROP_ORE);
            sleepWithVariation(transporter->transporterSleepTime);
            sem_post(foundry->foundryCoalArrivedSemaphore); //Unloaded signal
            sem_wait(foundry->mutex);
            if(foundry->foundryInfo->loading_capacity != foundry->foundryInfo->waiting_coal)
            {
                sem_post(availableCoalInFoundrySemaphore);
            }
            sem_post(foundry->mutex);
            
            sem_post(globalCoalMutex);
        }
        else
        {
            cout<<"You fucked up"<<endl;
        }
    }
    FillTransporterInfo(transporter->transporterInfo,transporter->transporterInfo->ID,NULL);
    WriteOutput(NULL,transporter->transporterInfo,NULL,NULL,TRANSPORTER_STOPPED);
}

void * smelterThreadFunction(void * smelterptr)
{
    int rs;
    int hs;
    timespec ts;
    Smelter * smelter = (Smelter *) smelterptr;
    smelter->smelterInfo->total_produce = 0;
    FillSmelterInfo(smelter->smelterInfo,smelter->smelterInfo->ID,smelter->smelterInfo->oreType,smelter->smelterInfo->loading_capacity,smelter->smelterInfo->waiting_ore_count,smelter->smelterInfo->total_produce);
    WriteOutput(NULL,NULL,smelter->smelterInfo,NULL,SMELTER_CREATED);
    while(true)
    {
        //waituntiltwoore or 5 sec sol:keep a semaphore
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 5;
        hs =sem_timedwait(smelter->smelterArrivedSemaphore,&ts);
        rs =sem_timedwait(smelter->smelterArrivedSemaphore,&ts);
        if(rs != 0 || hs!=0){
            break;
        }
        //sem_wait twice.. but then 5 sec doesnt work
        sem_wait(smelter->mutex);
        smelter->smelterInfo->waiting_ore_count=smelter->smelterInfo->waiting_ore_count-2;
        sem_post(smelter->mutex);
        
        FillSmelterInfo(smelter->smelterInfo,smelter->smelterInfo->ID,smelter->smelterInfo->oreType,smelter->smelterInfo->loading_capacity,smelter->smelterInfo->waiting_ore_count,smelter->smelterInfo->total_produce);
        WriteOutput(NULL,NULL,smelter->smelterInfo,NULL,SMELTER_STARTED);
        sleepWithVariation(smelter->smelterSleepTime);
        
        sem_wait(smelter->mutex);
        smelter->smelterInfo->total_produce++;
        if(smelter->smelterInfo->waiting_ore_count != smelter->smelterInfo->loading_capacity)
        {
            cout<<"Smelter is not full, increasing ore count"<<endl;
            if(smelter->smelterInfo->oreType == COPPER)
            {
                sem_post(availableCopperInSmelterSemaphore);
                cout<<"Increasing available copper in smelter semaphore"<<endl;
            }
            else if (smelter->smelterInfo->oreType == IRON)
            {
                sem_post(availableIronInSmelterOrFoundrySemaphore);
            }
        }
        sem_post(smelter->mutex);
        //smelterProduced
        
        FillSmelterInfo(smelter->smelterInfo,smelter->smelterInfo->ID,smelter->smelterInfo->oreType,smelter->smelterInfo->loading_capacity,smelter->smelterInfo->waiting_ore_count,smelter->smelterInfo->total_produce);
        WriteOutput(NULL,NULL,smelter->smelterInfo,NULL,SMELTER_FINISHED);
    }
    //SmelterStopped
    sem_wait(smelter->mutex);
    smelter->isAlive = false;
    FillSmelterInfo(smelter->smelterInfo,smelter->smelterInfo->ID,smelter->smelterInfo->oreType,smelter->smelterInfo->loading_capacity,smelter->smelterInfo->waiting_ore_count,smelter->smelterInfo->total_produce);
    WriteOutput(NULL,NULL,smelter->smelterInfo,NULL,SMELTER_STOPPED);
    sem_post(smelter->mutex);
    return 0;
}
void * foundryThreadFunction(void * foundryptr)
{
    int rs;
    int hs;
    timespec ts;
    Foundry * foundry = (Foundry *) foundryptr;
    foundry->foundryInfo->total_produce = 0;
    FillFoundryInfo(foundry->foundryInfo,foundry->foundryInfo->ID,foundry->foundryInfo->loading_capacity,foundry->foundryInfo->waiting_iron,foundry->foundryInfo->waiting_coal,foundry->foundryInfo->total_produce);
    WriteOutput(NULL,NULL,NULL,foundry->foundryInfo,FOUNDRY_CREATED);
    while(true)
    {
        //waituntiltwoore or 5 sec sol:keep a semaphore
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 5;
        rs =sem_timedwait(foundry->foundryCoalArrivedSemaphore,&ts);
        hs =sem_timedwait(foundry->foundryIronArrivedSemaphore,&ts); //DOES THIS WORK?
        if(rs != 0 || hs!=0){
            break;
        }
        sem_wait(foundry->mutex);
        foundry->foundryInfo->waiting_coal--;
        foundry->foundryInfo->waiting_iron--;
        sem_post(foundry->mutex);
        
        FillFoundryInfo(foundry->foundryInfo,foundry->foundryInfo->ID,foundry->foundryInfo->loading_capacity,foundry->foundryInfo->waiting_iron,foundry->foundryInfo->waiting_coal,foundry->foundryInfo->total_produce);
        WriteOutput(NULL,NULL,NULL,foundry->foundryInfo,FOUNDRY_STARTED);
        sleepWithVariation(foundry->foundrySleepTime);
        
        sem_wait(foundry->mutex);
        foundry->foundryInfo->total_produce++;
        if(foundry->foundryInfo->waiting_coal!=foundry->foundryInfo->loading_capacity)
        {
            cout<<"Foundry is not full for COAL, increasing coal count"<<endl;
            sem_post(availableCoalInFoundrySemaphore);
        }
        if(foundry->foundryInfo->waiting_iron!=foundry->foundryInfo->loading_capacity)
        {
            cout<<"Foundry is not full for IRON, increasing iron count"<<endl;
            sem_post(availableIronInSmelterOrFoundrySemaphore);
        }
        sem_post(foundry->mutex);
        FillFoundryInfo(foundry->foundryInfo,foundry->foundryInfo->ID,foundry->foundryInfo->loading_capacity,foundry->foundryInfo->waiting_iron,foundry->foundryInfo->waiting_coal,foundry->foundryInfo->total_produce);
        WriteOutput(NULL,NULL,NULL,foundry->foundryInfo,FOUNDRY_FINISHED);
    }
    sem_wait(foundry->mutex);
    foundry->isAlive = false;
    FillFoundryInfo(foundry->foundryInfo,foundry->foundryInfo->ID,foundry->foundryInfo->loading_capacity,foundry->foundryInfo->waiting_iron,foundry->foundryInfo->waiting_coal,foundry->foundryInfo->total_produce);
    WriteOutput(NULL,NULL,NULL,foundry->foundryInfo,FOUNDRY_STOPPED);
    sem_post(foundry->mutex);
    return 0;
}
int main()
{
    getInput();
    // printInput(); // comment this out later
    // for(i = 0 ; i<copperSmelterIds.size();i++)
    // {
    //   cout <<"Copper Smelter ID : "<<copperSmelterIds[i]<<endl;
    // }
    // for(i = 0 ; i<ironSmelterIds.size();i++)
    // {
    //   cout <<"Iron Smelter ID : "<<ironSmelterIds[i]<<endl;
    // }
    // cout<<"Ops Starts"<<endl;
    
    initializeSemaphores();
    
    InitWriteOutput();
    
    pthread_t minerThreads[numberOfMiners];
    pthread_t transporterThreads[numberOfTransporters];
    pthread_t smelterThreads[numberOfSmelters];
    pthread_t foundryThreads[numberOfFoundries];
    
    
    for(i=0;i<numberOfMiners;i++)
    {
        // cout<<"MinerVector-"<<minerVector[i]->minerInfo->ID<<endl;
        pthread_create(&(minerThreads[i]),NULL,minerThreadFunction, (void *) minerVector[i]); //we pass i , but id is i+1,minerArray without & ?
    }
    for(i=0;i<numberOfTransporters;i++)
    {
        // cout<<"TransporterVector-"<<transporterVector[i]->transporterInfo->ID<<endl;
        pthread_create(&(transporterThreads[i]),NULL,transporterThreadFunction, (void *) transporterVector[i]); //we pass i , but id is i+1,minerArray without & ?
    }
    // cout<<"Number of Smelter is : "<<numberOfSmelters<<endl;
    for(i=0;i<numberOfSmelters;i++)
    {
        // cout<<"SmelterrVector-"<<smelterVector[i]->smelterInfo->ID<<endl;
        pthread_create(&(smelterThreads[i]),NULL,smelterThreadFunction, (void *) smelterVector[i]); //we pass i , but id is i+1,minerArray without & ?
    }
    cout<<"Number of Foundries is : "<<numberOfFoundries<<endl;
    for(i=0;i<numberOfFoundries;i++)
    {
        cout<<"FoundryVector-"<<foundryVector[i]->foundryInfo->ID<<endl;
        pthread_create(&(foundryThreads[i]),NULL,foundryThreadFunction, (void *) foundryVector[i]); //we pass i , but id is i+1,minerArray without & ?
    }
    
    
    for(i=0;i<numberOfMiners;i++)
    {
        // cout<<"MinerVector-ENDS"<<minerVector[i]->minerInfo->ID<<endl;
        pthread_join(minerThreads[i],NULL);
    }
    for(i=0;i<numberOfTransporters;i++)
    {
        // cout<<"TransporterVector-ENDS"<<transporterVector[i]->transporterInfo->ID<<endl;
        pthread_join(transporterThreads[i],NULL);
    }
    for(i=0;i<numberOfSmelters;i++)
    {
        pthread_join(smelterThreads[i],NULL);
    }
    for(i=0;i<numberOfFoundries;i++)
    {
        pthread_join(foundryThreads[i],NULL);
    }
    
    closeSemaphores();
    return 0;
}
