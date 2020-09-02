# MapReduce

MapReduce programming model implementation in C . MapReduce is associated with processing and generating big data sets with a parallel and distributed algorithm on a platform.

#### Keywords
```
MapReduce, Inter Process Communication(IPC), fork , pipe, dup(2), exec
```

## Usage
```
git clone https://github.com/egeozbek/operating-systems-334-2019
cd MapReduce-HW1/Source
make all
./mapreduce N MapperPath ReducerPath
```
Where 
	```N``` is the number of mappers and reducers,
	```MapperPath``` is the path of the Mapper executable and 
	```ReducerPath``` is the path of the Reducer executable. 

If no ```ReducerPath``` is specified, it just performs the Map model without Reduce operation as following:

```
./mapreduce N MapperPath 
```

Note that, in both models, executing ```Mapper``` & ```Reducer``` are given their corresponding ```MapperID``` & ```ReducerID``` as parameters respectively.

There are sample MapReduce algorithms with sample i/o and Makefiles under ```samples``` folder.

#### Example usage
Assuming current directory is in Sources folder,
```
./mapreduce 5 ../Sample-IO/word_count/src/WC_Mapper ../Sample-IO/word_count/src/WC_Reducer > out.txt < ../Sample-IO/WC/input/input.txt
```