# Back to College Vaccination
This here is an implementation of Risubrik's idea of distributing vaccines to students attending the college, before getting into details let us look at how to compile and run.  
## Compile and Run
__To Compile__  
```
gcc q2.c -o q2 -lpthread
```
__To Run__  
```
./q2
```
### Expected Inputs
> numCompanies, numZones, numStudents //Three integer values  
numCompanies - Number of vaccine producing companies(Range 1-200)  
numZones - Number of vaccination zones(Range 1-200)  
numStudents - Number of students supposed to come(Range 1-200)  
(The above Will Error on input 0)  
Then n probabilities of success where n=numCompanies

## Structure  
There are 3 structures created each for student, zones and companies each with their respective attributes.  
An array of length 200 is created of each of the above structure. (The max lenght for each is 200)  
And a thread for each of the element above. (We are taking each zone/student/company as a thread)  
and an array studentStatus that keeps track whether a student has been done with or not.    

__Driver Functions__   
```
void setDefaultColor(){
    printf(ANSI_DEFAULT"\n");
}

int random(int min, int max){
    return (rand()%( max -min))+min;
}

int min(int a, int b, int c){
    if(a<b && a<c)
        return a;
    else if(b<c && b<a)
        return b;
    else
        return c;
}

int checkStatus(){
    int i;
    for(i=0;i<numStudents;i++)
        if(studentStatus[i]==0)
            return 1;
    return 0;
}
void *production(void *args);
void *vaccinating(void *arg);
void *incomingStudents(void *args);
```
* setDefaultColor swithces color back to default whenever we call it, used after every statement consisitng ANSI Color coding.
* random returns a random number in between min and max integeres passed to it
* min returns the minimum of three numbers (Terniary would have been a cleaner implementation)
* checkStatus loops through the studentStatus array and returns 0 if any student is remaining or 1 if all are successfully vaccinated/sent back.
* The last three are the major driver codes, described in detail below.  

### Production
This is a function which each company thread keeps running.  
It allocates randomly the time taken for each batch, number of vaccines per batch and number of batches.  
Each company independently starts production of vaccines which takes the allocated time and prepares the vaccines.  
Then it starts looking for zones with numVacccines=0, i.e. zones where vaccines are exhausted.  
Each zone holds the Company number and the probability of success of vaccine by the company.  
After distributing all batches, company thread goes to wait state and waits for all the produced batches to be consumed,  
i.e. a signal from all zones it has distributed vaccines to (Will be discussed in the respective part in vaccine zones).  
Once all the vaccines are used the company starts production again.  
While distributing vaccine a mutex lock is placed so that other company threads don't access the the same zone.   

## Vaccinating
This is a driver function for the complete happening of the code.  
Initially it waits for sum students to be in waiting state  
Then frees number of slots using the minimum function, first checks which is lower 8, numWaitingStudents or numVaccines and then frees those many slots.  
these slots are filled by students which will be dicussed in students part.  
For each student in slot vaccination is done, number of waiting student, number of vaccines are decremented by one.  
Before vaccination the boolean is_vaccinating of Zone is set so that no furthr student can join in.  
status of student is changed to VACCINATED //A static integer value defined at top of code
A mutex lock is placed while decrementing the value of the global vars so that other zones vaccinating wait before decrementing the value.  
After each vaccination we wait for a second.  
When the complete process ends the vcalue of all slots are set to 0 i.e. representing that the slots are empty.  
if the number of vaccines become 0, numVaccines is set to 0 and numBatches of company from which the vaccine batch was taken is decreased hence notifying the compny that the batch is completed.  
__Here two things happen__
* As soon as hasVaccines becomes 0 any company that has vaccine provides it to the zone  
* As soon as all the zones that took batch from a company report completed by each decrementing the value, the company starts production again.  

## IncomingStudents
The third major function of the code which each student thread runs.  
Initially as the student enters the numStudentsWaiting counter is incremented(miutex lock and unlock before and after doing it)  
and the student is alloted to a zone which has empty slot and has not yet started vaccinating, if no such zone exists the student keeps busy waiting.  
_Each Student_ Arrives at a random time.  
The assigning process is also mutex locked so that the slot and zone don't get accesed by other students till the place is secured.  
Once assinged the student probability variable will inherit from zone probabilit variable.  
After vaccinationnn the numTries is incremented by 1 (If it crosses 3 the student is sent back)  
Whether he got antibodies or not is checked by a value generated based on probability of success
```
int a= random(1, 100) < (students[num]->probability * 100); 
```
* if a is 1 antibody is generated and the student is allowed to go in  
* if a is 0 antibody is not generated and student is sent back in queue (if it is not his third try).

On possitive/failure(3 tries no antibody) the student status array stores this student num indexed value as 1 in the array.  
else while sending to wait the numStudentsLeft is incremented and numStudentsWaiting is incremented.

# Conclusion
Above each threads are interdependent on each other 
* Zone waiting on students to come for vaccine
* Student waiting for zone to get free and vaccinate them.
* Company waiting for zone to run out of vaccine to distribute.
* Company waiting for all the zones it distributed vaccines to to run out so as to start production again

All of this takes place parallelly usin threads an hence mutexes are used for the critical parts of codes where we'd want that part to be accessed by only one thread at a time, like for eg. changing the value of a global variable.

> All the code has been written adhering to the instruuctions given in the problem statement.
