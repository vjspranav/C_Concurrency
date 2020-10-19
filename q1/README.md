# Concurrent MergeSort

This ia a merge sort implementation in 3 different ways  
* Normal
* Using Processes
* Using Threads  

And comparing there performances.

## Compile and Run
__To compile__
```
gcc q1.c -o q1 -lpthread
```
__To run__
```
./q1
```

__Expected Inputs__
> Length of array to be sorted, n and n space separated integers

## Implementation
__Normal__  
The normal merge sort implementaion recursively dividing the array and merging from single sized array to complete. The usual.  

__Processes__  
Here instead of recursively dividing, we create a child process and let it do the division and mergin and let the parent wait untill both childs complete execution.  

__Threads__  
Almost like processes but we get multiple threads which each run independent of the other and sort among themselve =s till they finally come together and Join.  

## Expected Outcome
The performance of multithreaded would be faster then multi process which would be faster then normal merge sort as instead of dividing one after other, it takes place simultaneously.

## Actual Output
__After sorting an array of 7 elements__
```
Please enter 7 elements with a space: 78 76 54 102 87 65 34

Running concurrent_mergesort for n = 7
34 54 65 76 78 87 102 

time = 0.001961
Running threaded_concurrent_mergesort for n = 7
34 54 65 76 78 87 102 

time = 0.001601

Running normal_merge for n = 7
34 54 65 76 78 87 102 

time = 0.000003
normal_merge ran:
        [ 621.313772 ] times faster than concurrent_mergesort
        [ 507.263434 ] times faster than threaded_concurrent_mergesort
```
Even on running this code on 500+ numbers of an array normal sort is still faster than the other two.

## Reasoning 
In theory concurrent merging should be faster then straight forward iterative/recursive model.  
As much as the above holds true, the overhead time taken to creat n processes/threads for n different sorts/divisions is way more then the benefit gained by concurrent processing. i.e. The time taken to create so many number of threads\processess is much more than the time we gain by doing concurrent merge sort.    
### So what can be done?
One way would be to not create n number of threads for n merges and let just a two threads handle two sides of a merge sort.  
For smaller sized arrays this doesn't make sene, but as the sie increases the time szved by two threads sorting simultaneously would be a lot.
Also to make it more profitable inside thread before sorting we could place a condition to creattwo more threads if tha array that is being sorted in thread is relatively large.
