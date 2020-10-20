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
#define ANSI_ORANGE "\033[38:2:255:165:0m"
#define ANSI_DEFAULT "\033[0m"

void setDefaultColor(){
    printf(ANSI_DEFAULT"\n");
}

int randomNum(int min, int max){
    if(min==max)
        return min;
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

typedef struct stagePerformer {
    int mus_id;
    int stage_id;
    int singer_id;
    int has_music;
    int has_singer;
    pthread_mutex_t slock;
} stagePerformer;

stagePerformer asP[200];
stagePerformer esP[200];
musician *musicians[200];
pthread_t musician_thread[200];
pthread_cond_t pCond = PTHREAD_COND_INITIALIZER; 
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

void collectShirt(int num){
    sem_wait(&sem_co);
    printf(ANSI_YELLOW"\n%s collecting t-shirt", musicians[num]->name);
    setDefaultColor();
    sleep(2);
    printf(ANSI_GREEN"\n%s collected t-shirt and leaving", musicians[num]->name);
    setDefaultColor();
    sem_post(&sem_co);
}

void *stageAllocation(void *args){
    int stageID=-1, musicID, singerID, isSinger=0, i;
    int num=*(int *) args;
    int a=0, e=0, pTime=0;
    sleep(musicians[num]->arr_time);
    printf(ANSI_MAGENTA"\n%s arrived at Srujana for %s", musicians[num]->name, getPerformance(musicians[num]->instrument));
    setDefaultColor();

    if(musicians[num]->is_singer==1){
        isSinger=1;

        int as=0;
        int es=0;
        check1:
        es=sem_trywait(&sem_estage);
        if(es==-1){
            as=sem_trywait(&sem_astage);
            if(as!=-1)
                a=1;
        }else{
            e=1;
        }

        for(i=0;i<num_estages;i++){
            if(esP[i].has_singer==0 && esP[i].has_music==1){
                pthread_mutex_lock(&esP[i].slock);
                stageID=i;
                musicID=esP[i].mus_id;
                singerID=num;
                esP[i].has_singer=1;
                esP[i].singer_id=num;
                pthread_mutex_unlock(&esP[i].slock);
                break;
            }
        }
        if(singerID==0){
            for(i=0;i<num_astages;i++){
                if(asP[i].has_singer==0 && asP[i].has_music==1){
                    pthread_mutex_lock(&asP[i].slock);
                    stageID=i;
                    musicID=asP[i].mus_id;
                    singerID=num;
                    asP[i].has_singer=1;
                    asP[i].singer_id=num;
                    pthread_mutex_unlock(&asP[i].slock);
                    break;
                }
            }
        }
        if(stageID!=-1){
            musicians[musicID]->performance_time+=2;
            printf(ANSI_CYAN"\n%s %s with %s on %s (id=%d) for 2 more seconds", musicians[num]->name, getPerformance(musicians[num]->instrument), musicians[musicID]->name, stageName(a, e), stageID);
            setDefaultColor();
            return NULL;
        }
        if(es==-1 && as==-1){
            pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
            pthread_mutex_lock(&mutex);
            if(pthread_cond_timedwait(&pCond, &mutex, &ts)==ETIMEDOUT){
                printf(ANSI_ORANGE"\nPerformer %s %s Left without performing due to impatience", musicians[num]->name, getPerformance(musicians[num]->instrument));
                setDefaultColor();
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
            pthread_mutex_unlock(&mutex);
            goto check1;
        }

    }
    
    if(isSinger!=1){
        if((musicians[num]->is_accoustic==1 && num_astages>0) && (musicians[num]->is_electrical==1 && num_estages>0)){
            int as=0;
            int es=0;
            check:
            es=sem_trywait(&sem_estage);
            if(es==-1){
                as=sem_trywait(&sem_astage);
                if(as!=-1)
                    a=1;
            }else{
                e=1;
            }
            if(es==-1 && as==-1){
                pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
                pthread_mutex_lock(&mutex);
                if(pthread_cond_timedwait(&pCond, &mutex, &ts)==ETIMEDOUT){
                    printf(ANSI_ORANGE"\nPerformer %s %s Left without performing due to impatience", musicians[num]->name, getPerformance(musicians[num]->instrument));
                    setDefaultColor();
                    pthread_mutex_unlock(&mutex);
                    return NULL;
                }
                pthread_mutex_unlock(&mutex);
                goto check;
            }
        }else{
            //If only accoustic
            if(musicians[num]->is_accoustic==1 && num_astages>0){
                if(sem_timedwait(&sem_astage, &ts)!=0){
                    if(errno==ETIMEDOUT){
                        printf(ANSI_ORANGE"\nPerformer %s %s Left without performing due to impatience", musicians[num]->name, getPerformance(musicians[num]->instrument));
                        setDefaultColor();
                        return NULL;
                    }
                }
                a=1;
            }

            // If only electric
            if(musicians[num]->is_electrical==1 && num_estages>0){
                if(sem_timedwait(&sem_estage, &ts)!=0){
                    if(errno==ETIMEDOUT){
                        printf(ANSI_ORANGE"\nPerformer %s %s Left without performing due to impatience", musicians[num]->name, getPerformance(musicians[num]->instrument));
                        setDefaultColor();
                        return NULL;
                    }
                }
                e=1;
            }
        }
    }
    // Keep track of acoustic/electric stages
    if(isSinger==0){
        if(e==1){
            for(i=0;i<num_estages;i++){
                if(esP[i].has_music==0){
                    pthread_mutex_lock(&esP[i].slock);
                    stageID=esP[i].stage_id;
                    esP[i].has_music=1;
                    esP[i].mus_id=num;
                    pthread_mutex_unlock(&esP[i].slock);
                    break;
                }
            }
        }
        if(a==1){
            for(i=0;i<num_astages;i++){
                if(asP[i].has_music==0){
                    pthread_mutex_lock(&asP[i].slock);
                    stageID=asP[i].stage_id;
                    asP[i].has_music=1;
                    asP[i].mus_id=num;
                    pthread_mutex_unlock(&asP[i].slock);
                    break;
                }
            }
        }
    }else{
        if(e==1){
            for(i=0;i<num_estages;i++){
                if(esP[i].has_music==0 && esP[i].has_singer==0){
                    pthread_mutex_lock(&esP[i].slock);
                    stageID=esP[i].stage_id;
                    esP[i].has_singer=1;
                    esP[i].singer_id=num;
                    pthread_mutex_unlock(&esP[i].slock);
                    break;
                }
            }
        }
        if(a==1){
            for(i=0;i<num_astages;i++){
                if(asP[i].has_music==0 && asP[i].has_music==0){
                    pthread_mutex_lock(&asP[i].slock);
                    stageID=asP[i].stage_id;
                    asP[i].has_singer=1;
                    asP[i].singer_id=num;
                    pthread_mutex_unlock(&asP[i].slock);
                    break;
                }
            }
        }
    }

    // If only electric/acoustic and respective stage is not available
    if(a==0 && e==0){
        printf(ANSI_RED"\nPerformer %s left as he doesn't have %s", musicians[num]->name, stageName(musicians[num]->is_accoustic, musicians[num]->is_electrical));
        setDefaultColor();
        return NULL;
    }

    // Performing
    printf(ANSI_CYAN"\n%s %s on %s (id=%d) for %d seconds", musicians[num]->name, getPerformance(musicians[num]->instrument), stageName(a, e), stageID, musicians[num]->performance_time);
    setDefaultColor();
    while(pTime<musicians[num]->performance_time){
        sleep(1);
        pTime+=1;
    }

    // Performed
    if(a==1?asP[stageID].has_singer==0:esP[stageID].has_singer==0){
        printf(ANSI_CYAN"\n%s performance on %s Finished", musicians[num]->name, stageName(a, e));
        setDefaultColor();
    }else if(isSinger==1){
        printf(ANSI_CYAN"\n%s performance on %s Finished", musicians[num]->name, stageName(a, e));
        setDefaultColor();
    }else{
        printf(ANSI_CYAN"\n%s performance with %s on %s Finished", musicians[num]->name, a==1?musicians[asP[stageID].singer_id]->name:musicians[esP[stageID].singer_id]->name, stageName(a, e));
        setDefaultColor();
        if(num_coordinators>0)
            collectShirt(a==1?asP[stageID].singer_id:esP[stageID].singer_id);
    }
    sem_post(a==1?&sem_astage:&sem_estage);
    // Freeing acoustic/electric stages
    if(e==1){
        pthread_mutex_lock(&esP[i].slock);
        esP[i].has_music=0;
        esP[i].mus_id=-1;
        if(esP[i].has_singer==1 || isSinger==1){
            esP[i].has_singer=0;
            esP[i].singer_id=-1;
        }
        pthread_mutex_unlock(&esP[i].slock);
    }
    if(a==1){
        pthread_mutex_lock(&asP[i].slock);
        asP[i].has_music=0;
        asP[i].mus_id=-1;
        if(asP[i].has_singer==1 || isSinger==1){
            asP[i].has_singer=0;
            asP[i].singer_id=-1;
        }
        pthread_mutex_unlock(&asP[i].slock);
    }

    // Signal any conditionally waiting musician
    pthread_cond_signal(&pCond);
    if(num_coordinators>0)
        collectShirt(num);
    return NULL;
}

int main(){
    srand(time(0));
    int i;
    // Input Time
    printf("Please enter number of musicians: ");
    scanf("%d", &num_musicians);
    if(num_musicians<=0){
        printf(ANSI_RED"No Musician There to perform, exiting");
        setDefaultColor();
        _exit(0);
    }
    printf("Please enter number of accoustic and electirc stages with a space: ");
    scanf("%d %d", &num_astages, &num_estages);
    sem_init(&sem_estage, 0, num_estages);
    sem_init(&sem_astage, 0, num_astages);
    if(num_estages<=0 && num_astages<=0){
        printf(ANSI_RED"Acoustic and Elictric stage missing, performances Cancelled");
        setDefaultColor();
        _exit(0);
    }
    printf("Please enter number of coordinators: ");
    scanf("%d", &num_coordinators);
    sem_init(&sem_co, 0, num_coordinators);
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
        musicians[i]->performance_time=randomNum(t1, t2);
    }
    ts.tv_sec=t;

    // Show all input taken
    printf("Sujana Details\n");
    printf("Number of a/e stages=%d/%d\nt=%d t1=%d t2=%d\nNumber of musicians: %d (The details Follow)\n", num_astages, num_estages, t, t1, t2, num_musicians);
    for(i=0;i<num_musicians;i++)
        printf("%s\t%c\t%d\n", musicians[i]->name, musicians[i]->instrument, musicians[i]->arr_time);
    if(num_coordinators<=0){
        printf(ANSI_ORANGE"\n****No Co-ordinators found tshirt Distribution will not be done****\n");
        setDefaultColor();
    }

    for(i=0;i<num_astages;i++){
        asP[i].stage_id=i;
        asP[i].has_music=0;
        asP[i].has_singer=0;
        asP[i].mus_id=-1;
        asP[i].singer_id=-1;
        pthread_mutex_init(&asP[i].slock, NULL);
    }
    for(i=0;i<num_estages;i++){
        esP[i].stage_id=i;
        esP[i].has_music=0;
        esP[i].has_singer=0;
        esP[i].mus_id=-1;
        esP[i].singer_id=-1;
        pthread_mutex_init(&esP[i].slock, NULL);
    }

    printf(ANSI_YELLOW"All Inputs successfully taken. Starting perfomance..");
    setDefaultColor();
    for(i=0;i<num_musicians;i++){
        int *arg = malloc(sizeof(*arg));
        *arg = i;        
        if((pthread_create(&musician_thread[i], NULL, stageAllocation, arg)!=0)){
            perror("Musician Could Not perform: ");
            printf(ANSI_RED"Musician %s could not perform due to internal errors", musicians[i]->name);
            setDefaultColor();
        }
    }
    for(i=0;i<num_musicians;i++)
        pthread_join(musician_thread[i], NULL);
    printf(ANSI_GREEN"\nFinished\n");
    setDefaultColor();
}