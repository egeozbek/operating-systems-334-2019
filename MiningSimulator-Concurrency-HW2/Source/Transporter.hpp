#include "writeOutput.h"
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

class Transporter
{
public:
    Transporter(TransporterInfo * transporterInfo,unsigned int sleepTime);
    unsigned int transporterSleepTime;
    TransporterInfo * transporterInfo;
};
