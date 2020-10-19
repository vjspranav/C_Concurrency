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

typedef struct musician{
    char name[20];
    char instrument;
    int arr_time;
    int performed;
} musician;

musician *musicians[200];
pthread_t m_thread[200];
int num_musicians, num_astages, num_estages, t1, t2, t;
sem_t sem_astage, sem_estage;

void *stage(){}

int main(){
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
        scanf("%s", musicians[i]->name);
        scanf(" %c", &musicians[i]->instrument);
        if(musicians[i]->instrument!='p' || musicians[i]->instrument!='g' || musicians[i]->instrument!='v' || musicians[i]->instrument!='b' || musicians[i]->instrument!='s'){
            printf(ANSI_RED"Not a valid instrument. aborting...");
            setDefaultColor();
            _exit(0);
        }
        scanf("%d", &musicians[i]->arr_time);
    }
    // Show all input taken
    printf("Sujana Details\n");
    printf("Number of a/e stages=%d/%d\nt=%d t1=%d t2=%d\nNumber of musicians: %d (The details Follow)\n", num_astages, num_estages, t, t1, t2, num_musicians);
    for(i=0;i<num_musicians;i++)
        printf("%s\t%c\t%d\n", musicians[i]->name, musicians[i]->instrument, musicians[i]->arr_time);
}