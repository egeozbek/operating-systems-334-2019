# Mining Simulation 

Mining Simulation in which several agents of different types such as _Miners_ produce _Ores_ in accordance with their production times and capacities and agents of type _Transporter_ carry these _Ores_ to agents of type _Smelter_ or _Foundry_ according to _Ore_ type of its current _Ore_.
_Smelter_ agents produce _Iron_ or _Copper_ ingots and _Foundries_ produce _Steel_ ingots using different combinations of these _Ores_ with their production times and capacities respectively.

## Finishing Conditions
_Miners_ quit if they produce their maximum number of _Ores_,
_Transporters_ quit if no _Miner_ left and no _Ores_ left in any of the _Miner's_ storage,
_Smelters_ and _Foundries_ quit if they can't produce _Ingots_ (due to lack of _Ores_) for 5 seconds.

#### Keywords
```
Concurrency, Synchronization, Thread, Semaphore, Mutex, Condition Variable
```

## Usage
```
git clone https://github.com/egeozbek/operating-systems-334-2019
cd MiningSimulator-Concurrency-HW2/Source
make all
./simulator < inp.txt > out.txt
```
##Tests
Testing is taken from ```https://github.com/ysyesilyurt/OperatingSystems-2019```.

Tests can be found ```Sample-IO``` folder.

It will run all the inputs in the form ```inp#.txt``` under ```tests/inputs``` folder and output to ```tests/outputs``` folder with its correponding number.