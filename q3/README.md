# Music Maheym
Managing the music event at IIIT-H.  
The following table has been followed  

|Instrument| Players Instrument  | Character Stages eligible to perform|  
|----------|---------------------|-------------------------------------|  
|Piano     | p                   |Acoustic, Electric                   |  
|Guitar    | g                   |Acoustic, Electric                   |  
|Violin    | v                   |Acoustic                             |  
|Bass      | b                   |Electric                             |  
|Singer*   |(special case)s      |Acoustic, Electric                   |  

## Compile and Run
__To Compile__  
```
gcc q3.c -o q3 -lpthread
```
__To Run__  
```
./q3
```
### Expected Inputs
First line give 
```
k, a, e, c, t1, t2, t
```
(note: 0 <= k, a, e, c, t1, t2, t   and   t1 <= t2)  
The following k lines will have info of each instrumentalist. Each line is of the form  
```
<name> <instrument character> <time in secs of arrival>
```

## Structure  
Here we have two structures
* One denoting the the structure of musician
* One for stage details
an array of 200 for musicians, electric stage structure an dacoustic stage structure is created.  
other than them we have 3 semaphores each for electric, acoustic stage and co ordinator.
and a conditional thread variable.

## Working
After taking the initial input a thread for each musician is created.  
Once the thread is created based on whether he is acoustic or electric or both an available stage is searched for  
if found the musician is alloted the stage and his records and stage id stored in respective stage array and semaphore is put on wait.  
After performance semaphore is put on post and performer is sent for collecting tshirts.
If a singer comes, and sees no stage free but a musician performing that singer id is added to struct of that stage 
this working has been done with a mutex lock on conditional wait.  
  
Complete Report Updated at: https://github.com/vjspranav/C_Concurrency/blob/master/q3/README.md
