#define _POSIX_C_SOURCE 199309L //required for clock
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <wait.h>
#include <sys/shm.h>

// Color Codes
#define ANSI_CYAN "\033[1;36m"
#define ANSI_RED "\033[1;31m"
#define ANSI_GREEN "\033[1;32m"
#define ANSI_YELLOW "\033[1;33m"
#define ANSI_MAGENTA "\033[1;35m"
#define ANSI_DEFAULT "\033[0m"

void setDefaultColor(){
    printf(ANSI_DEFAULT"\n");
}

int random(int min, int max){
    return (rand()%( max -min))+min;
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

typedef struct musician{
    char name[20];
    char instrument;
    int arr_time;
    int performance_time;
    int performed;
    int is_accoustic;
    int is_electrical;
    int is_singer;
    //pthread_mutex_t performing; Fall back
} musician;

musician *musicians[200];
pthread_t musician_thread[200];
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER; 
int num_musicians, num_astages, num_estages, num_coordinators, t1, t2, t;
sem_t sem_astage, sem_estage, sem_co;
struct timespec ts;

/**
 * pthread_t a_thread[200];
 * pthread_t e_thread[200];
 * void *accoustic_stage(){}
 * void *electrical_stage(){}
 * Fall back
 */

void *stageAllocation(void *args){
    int num=*(int *) args;
    int a=0, e=0, pTime=0;
    sleep(musicians[num]->arr_time);
    printf(ANSI_MAGENTA"%s arrived at Srujana for %s", musicians[num]->name, getPerformance(musicians[num]->instrument));
    setDefaultColor();
    if(musicians[num]->is_accoustic){
        if(sem_timedwait(&sem_astage, &ts)!=0){
            if(errno==ETIMEDOUT){
                printf(ANSI_YELLOW"Performer %s %s Left without performing due to impatience", musicians[num]->name, getPerformance(musicians[num]->instrument));
                setDefaultColor();
                return NULL;
            }
        }
        a=1;
    }
    if(musicians[num]->is_electrical){
        if(sem_timedwait(&sem_estage, &ts)!=0){
            if(errno==ETIMEDOUT){
                printf(ANSI_YELLOW"Performer %s %s Left without performing due to impatience", musicians[num]->name, getPerformance(musicians[num]->instrument));
                setDefaultColor();
                return NULL;
            }
        }
        e=1;
    }
    printf(ANSI_CYAN"%s %s on %s for %d seconds", musicians[num]->name, getPerformance(musicians[num]->instrument), stageName(a, e), musicians[num]->performance_time);
    setDefaultColor();
    while(pTime<musicians[num]->performance_time){
        sleep(1);
        pTime+=1;
    }
    sem_post(a==1?&sem_astage:&sem_estage);
    return NULL;
}

int main(){
    srand(time(0));
    int i;
    // Input Time
    printf("Please enter number of musicians: ");
    scanf("%d", &num_musicians);
    if(num_musicians<=0){
        printf(ANSI_RED"No Musician There to perform, exiting\n");
        setDefaultColor();
        _exit(0);
    }
    printf("Please enter number of accoustic and electirc stages with a space: ");
    scanf("%d %d", &num_astages, &num_estages);
    if(num_estages<=0 || num_astages<=0){
        printf(ANSI_RED"Acoustic or Elictric stage missing, performances Cancelled\n");
        setDefaultColor();
        _exit(0);
    }
    printf("Please enter number of coordinators: ");
    scanf("%d", &num_coordinators);
    printf("Please enter t1, t2 and t with a space: ");
    scanf("%d %d %d", &t1, &t2, &t);
    
    if(t2<t1){
        printf(ANSI_RED"Invalid time range\n");
        setDefaultColor();
        _exit(0);
    }

    printf("Please enter name, instrument character and time of arrival with a space for\n");
    for(i=0;i<num_musicians;i++){
        printf("musician %d: ", i+1);
        musicians[i]=(musician *)malloc(sizeof(musician));
        musicians[i]->is_accoustic=musicians[i]->is_electrical=musicians[i]->is_singer=0;
        scanf("%s", musicians[i]->name);
        scanf(" %c", &musicians[i]->instrument);
        if(musicians[i]->instrument!='p' && musicians[i]->instrument!='g' && musicians[i]->instrument!='v' && musicians[i]->instrument!='b' && musicians[i]->instrument!='s'){
            printf(ANSI_RED"Not a valid instrument. aborting...");
            setDefaultColor();
            _exit(0);
        }
        scanf("%d", &musicians[i]->arr_time);
        char temp_type=musicians[i]->instrument;
        if(temp_type=='p'||temp_type=='g'||temp_type=='v'||temp_type=='s')
            musicians[i]->is_accoustic=1;
        if(temp_type=='p'||temp_type=='g'||temp_type=='b'||temp_type=='s')
            musicians[i]->is_electrical=1;
        if(temp_type=='s')
            musicians[i]->is_singer=1; 
        musicians[i]->performance_time=random(t1, t2);
    }
    ts.tv_sec=t;
    // Show all input taken
    printf("Sujana Details\n");
    printf("Number of a/e stages=%d/%d\nt=%d t1=%d t2=%d\nNumber of musicians: %d (The details Follow)\n", num_astages, num_estages, t, t1, t2, num_musicians);
    for(i=0;i<num_musicians;i++)
        printf("%s\t%c\t%d\n", musicians[i]->name, musicians[i]->instrument, musicians[i]->arr_time);
    printf(ANSI_YELLOW"All Inputs successfully taken. Starting perfomance..");
    setDefaultColor();
    for(i=0;i<num_musicians;i++){
        int *arg = malloc(sizeof(*arg));
        *arg = i;        
        if((pthread_create(&musician_thread[i], NULL, stageAllocation, arg)!=0)){
            perror("Musician Could Not perform: ");
            printf(ANSI_RED"Musician %s could not perform due to internal errors\n", musicians[i]->name);
            setDefaultColor();
        }
    }
    for(i=0;i<num_musicians;i++)
        pthread_join(musician_thread[i], NULL);
}