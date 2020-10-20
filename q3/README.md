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

### Sample Outputs
* Case 1 : Normal
<img src="https://imgur.com/HTqHg6W.png"/>

* Case 2 : only 1 of the stage is available
<img src="https://imgur.com/sJNfBPF.png"/>

## Structure  
Here we have two structures
```
typedef struct musician{
    char name[20];
    char instrument;
    int arr_time;
    int performance_time;
    int performed;
    int is_accoustic;
    int is_electrical;
    int is_singer;
} musician;
```
* One denoting the the structure of musician
```
typedef struct stagePerformer {
    int mus_id;
    int stage_id;
    int singer_id;
    int has_music;
    int has_singer;
    pthread_mutex_t slock;
} stagePerformer;
```
* One for stage details
> an array of 200 for musicians, electric stage structure an dacoustic stage structure is created.  
other than them we have 3 semaphores each for electric, acoustic stage and co ordinator.
and a conditional thread variable.

```
sem_t sem_astage, sem_estage, sem_co;
pthread_t musician_thread[200];
pthread_cond_t pCond = PTHREAD_COND_INITIALIZER; 
```
* Variables for synchronisation
    * sem_astage and sem_astage semaphores for locking a stage.
    * sem_co semaphore for ocing co ordinators who distribute tshirts.
    * musician_thread at max creating 200 threads, i.e. at the maximum can handle 200 musicians.
    * pCond a conditional thread thread used for conditionally waiting for either acoustic or electric stage to free up.

## Cases Handled
* If either of acoustic or electric stage is not available (i.e. input given for one of them is 0), any musician only performing on the missing stage is sent back as the stage is unavailable.
* If co-ordinators are 0, tshirt distribution is not done.
* If musician is both acoustic and electric and neither of stage is available he/she conditionally waits for one of the performance to complete and then then checks which stage got freed.
* If singer, the above check follows but before conditionally waiting singer also checks if any musician is performing solo and joins in the musician and increases his peromace time by 2 seconds and exits alongside, if neither stage is free and no musician is solo, singer goes to conditionally wait.
* If there are no musicians or none of the stage is available, performance is cancelled.  

__Bonuses__
* The stages id has been kept in track and printed
* Even singer is eligible to recieve a tshirt.

## Functions to prettify
```
// Color Codes
#define ANSI_CYAN "\033[1;36m"
#define ANSI_RED "\033[1;31m"
#define ANSI_GREEN "\033[1;32m"
#define ANSI_YELLOW "\033[1;33m"
#define ANSI_MAGENTA "\033[1;35m"
#define ANSI_ORANGE "\033[38:2:255:165:0m"
#define ANSI_DEFAULT "\033[0m"

void setDefaultColor(){
    printf(ANSI_DEFAULT"\n");
}

// To prettify output
char *getPerformance(char inst){
    char *Performance;
    if(inst=='p')
        Performance="performing Piano";
    if(inst=='g')
        Performance="performing Guitar";
    if(inst=='v')
        Performance="performing Violin";
    if(inst=='b')
        Performance="performing Bass";
    if(inst=='s')
        Performance="singing";
    return Performance;
}

char *stageName(int a, int e){
    return a==1?"Acoustic Stage":"Electric Stage";
}
```
This part is used for printing the type of stage based on acoustic and electric.  
The kind of instrument performer is performing on based on instrument charecter (Simplifies and prettifies printing)  
ANSII Color codes used for prettifying and distinguishing steps from one another.  

## Driver Functions
```
void collectShirt(int num){
    sem_wait(&sem_co);
    printf(ANSI_YELLOW"\n%s collecting t-shirt", musicians[num]->name);
    setDefaultColor();
    sleep(2);
    printf(ANSI_GREEN"\n%s collected t-shirt and leaving", musicians[num]->name);
    setDefaultColor();
    sem_post(&sem_co);
}

void *stageAllocation(void *args);
```
* Collect shirt function is called at the end of each performance (if the musicien is still patiently waiting) to recieve a tshirt, a sempahore is initialised with number of co-ordinators and a wait and post is used accordingly to not over burden a single co ordinator with more than one performer at a time.
* stageAllocation is the main driver code of the function where every single condition is checked and satisfied (As discussed in implementation). Each musician has his own thread which simultaneously run for all functions using semaphores based on stages for maintaining sync.

## Implementation
* Initially after taking all the relevant input, we create a thread for each musician.
* Inside of it through select stage function we follow through a lot of conditions and a couple of conditioned waits the respective stages are allocated.

## Working
After taking the initial input a thread for each musician is created.  
Once the thread is created based on whether he is acoustic or electric or both an available stage is searched for  
if found the musician is alloted the stage and his records and stage id stored in respective stage array and semaphore is put on wait.  
After performance semaphore is put on post and performer is sent for collecting tshirts.
If a singer comes, and sees no stage free but a musician performing that singer id is added to struct of that stage 
this working has been done with a mutex lock on conditional wait.  
  
Complete Report Updated at: https://github.com/vjspranav/C_Concurrency/blob/master/q3/README.md
