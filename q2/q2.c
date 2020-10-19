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
// Some static values
#define NOT_VACCINATED 0
#define VACCINATING 1
#define VACCINATED 2

/**
 * Company Struct
 * probability - probability of success of set of batches
 * num_batches - number of batches (1, 5)
 * time_taken - time taken to prepare r batches (2 sec, 5 sec)
 * num_vaccines - Number of vaccines per batch (10, 20)
 * company_num - Company number
 */
typedef struct company {
    float probability; 
    int time_taken; 
    int num_batches; 
    int num_vaccines; 
    int company_num;
} company;

/**
 * Vaccine Zones in IIIT H Struct
 * probability - probability of success of current vaccine
 * slots - number of available slots, maximum 8
 * is_vaccinating - boolean 0/1
 * zone_num - zone number of current zone
 * has_vaccine - 0 if no vaccines/1 if vaccines are there 
 * num_slots - Current number of free slots
 * num_slots_filled - number of slots filled
 * company_num - the number of company from which it recieved vaccine
 */
typedef struct vaccine_zones {
    float probability;
    int slots[8];
    int is_vaccinating;
    int zone_num;
    int numVaccines;
    int num_slots;
    int num_slots_filled;
    int company_num;
    pthread_mutex_t mutex;    
} vaccine_zones;

/**
 * Student Struct
 * cur_status - 0 if not vaccinated/1 if vaccinating/2 if vaccinated
 * num_try - how many times he completed so far
 * zone - -1 if none else zone number
 * antibody - 0 if negative/1 if possitive
 * time - arrival time
 * num - id
 */ 
typedef struct student{
    int cur_status;
    int num_try;
    int zone_num;
    int antibody;
    int time;
    int num;
}student;

int numVaccines, numCompanies, numZones, numStudents, numStudentsleft, numStudentsWaiting;
pthread_t company_thread[200], zone_thread[200], student_thread[200];
company *companies[200];
vaccine_zones *zones[200];
student *students[200];
pthread_mutex_t mutex, studentMutex, zoneMutex, zone1Mutex;

/* Main Functions */

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

void *production(void *arg){
    int i, numBatchesDistributed;
    while(numStudentsleft>0){
        int num=*(int *)arg;
        companies[num]->time_taken=random(2, 5);
        companies[num]->num_batches=random(1, 5);
        companies[num]->num_vaccines=random(10, 20);
        numBatchesDistributed=companies[num]->num_batches;
        printf(ANSI_YELLOW"Pharmaceutical Company %d is preparing %d batches of vaccines which have success probability %f\n", companies[num]->company_num, companies[num]->num_batches, companies[num]->probability);
        setDefaultColor();
        sleep(companies[num]->time_taken);
        printf(ANSI_CYAN"Pharmaceutical Company %d has prepared %d batches of vaccines which have success probability %f.\nWaiting for all the vaccines to be used to resume production\n", companies[num]->company_num, companies[num]->num_batches, companies[num]->probability);
        setDefaultColor();

        // Distributing available vaccines
        while(numBatchesDistributed>0){
            for(i=0;i<numZones;i++){
                if(zones[i]->numVaccines==0){
                    pthread_mutex_lock(&zones[i]->mutex);
                    zones[i]->numVaccines=companies[num]->num_vaccines;
                    printf(ANSI_CYAN"Pharmaceutical Company %d is delivering a vaccine batch to Vaccination Zone %d which has successprobability %f\n", companies[num]->company_num, zones[i]->zone_num, companies[num]->probability);
                    setDefaultColor();
                    numBatchesDistributed-=1;
                    zones[i]->probability=companies[num]->probability;
                    zones[i]->company_num=companies[num]->company_num;
                    pthread_mutex_unlock(&zones[i]->mutex);
                }
            }
        }

        //Waiting for all batches to be completely used
        while(companies[num]->num_batches!=0)
            ;
        printf("All the vaccines prepared by Pharmaceutical Company %d are emptied. Resuming production now.\n", companies[num]->company_num);
    }
}

void *vaccinating(void *arg){
    int i;
    int num=*(int *) arg;
    while(numStudentsleft>0){
        zones[num]->is_vaccinating=-1;
        // Waiting for company to provide vaccine
        while(zones[num]->numVaccines==0)
            ;
        while(zones[num]->numVaccines>0){
            zones[num]->num_slots_filled=0;
            zones[num]->is_vaccinating=0;
            zones[num]->num_slots=min(8, numStudentsWaiting, zones[num]->numVaccines);
            pthread_mutex_lock(&mutex);
            printf("Zone %d Recieved %d vaccines from Company %d\n", zones[num]->zone_num, zones[num]->numVaccines, zones[num]->company_num);

            // Waiting for slots to fill up
            printf(ANSI_CYAN"Vaccination Zone %d entering Vaccination Phase\n",zones[num]->zone_num);
            setDefaultColor();        
            zones[num]->is_vaccinating=-1;
            pthread_mutex_lock(&zoneMutex);
            zones[num]->is_vaccinating=1;
            printf("%d slots filled, total slots are %d\n", zones[num]->num_slots_filled, zones[num]->num_slots);
            for(i=0;i<zones[num]->num_slots;i++){
                printf("In zone %d, slot %d=%d\n", zones[num]->zone_num, i+1, zones[num]->slots[i]);
            }
            for(i=0;i<zones[num]->num_slots;i++){
                if(zones[num]->slots[i]>0){
                    printf("\nVaccinating %d student in zone %d\n", zones[num]->slots[i]-1, zones[num]->zone_num);
                    pthread_mutex_lock(&zone1Mutex);
                    students[zones[num]->slots[i]-1]->cur_status=VACCINATING;
                    zones[num]->numVaccines-=1;
                    students[zones[num]->slots[i]-1]->cur_status=VACCINATED;
                    zones[num]->slots[i]=0;
                    zones[num]->num_slots_filled-=1;
                    numStudentsleft-=1;
                    numStudentsWaiting-=1;
                    pthread_mutex_unlock(&zone1Mutex);
                }
            }
            for(i=0;i<8;i++){
                zones[num]->slots[i]=0;   //again block all the slots and mext iteration it releases only designated slots
            } 
            pthread_mutex_unlock(&zoneMutex);
            zones[num]->is_vaccinating=0;

            pthread_mutex_unlock(&mutex);
            sleep(2);
            if(numStudentsleft==0){
                return NULL;
            }

        }
    }
}
void *incomingStudents(void *arg){
    int i, j;
    int num=*(int *) arg;
    sleep(students[num]->time);
    numStudentsWaiting+=1;
    while(students[num]->num_try<3){
        printf(ANSI_MAGENTA"Student %d has arrived for his %d round of Vaccination\n", students[num]->num,students[num]->num_try+1);
        setDefaultColor();
        while(students[num]->cur_status!=VACCINATED && students[num]->zone_num==0){
            for(i=0;i<numZones;i++){
                pthread_mutex_lock(&studentMutex);
                for(j=0;j<zones[i]->num_slots;j++){
                    if(zones[i]->slots[j]==0 && zones[i]->is_vaccinating==0){
                        zones[i]->slots[j]=students[num]->num;
                        students[num]->zone_num=zones[i]->zone_num;
                        students[num]->cur_status=NOT_VACCINATED;
                        students[num]->num_try+=1;
                        zones[i]->num_slots_filled+=1;
                        break;
                    }
                }
                pthread_mutex_unlock(&studentMutex);
            }
        }
        // Waiting for vaccination to be done
        while(students[num]->cur_status!=VACCINATED)
            ;
        printf("Student %d vaccinated\n", num+1);
        //printf(ANSI_YELLOW"Student %d on Vaccination Zone %d has been vaccinated which has success probability %f\n", students[num]->num, students[num]->zone_num , zones[students[num]->zone_num]->probability);
        // Check Antibody  
        int a=1; //random(0, 1);  
        if(a==1){
            printf(ANSI_GREEN"Student %d tested positive for antibodies\n", students[num]->num);
            //printf("Num Student left = %d\n", numStudentsleft);
            if(numStudentsleft==0){
                //sleep(4);
                printf("\nSimulation Done\n");
                _exit(0);
            }

            break;
        }
    }
}

int main(){
    int i;
    printf("Starting Vaccination Process\n");
    srand(time(0));
    printf("Enter number of Vaccine companies: ");
    scanf("%d", &numCompanies);
    if(numCompanies<=0){
        printf(ANSI_RED "No vaccine company to provide vaccines\n");
        setDefaultColor();
        _exit(1);
    }
    printf("Enter number of Vaccine Zones in IIIT-H: ");
    scanf("%d", &numZones);
    if(numZones<=0){
        printf(ANSI_RED"No vaccine zones in IIIT-H\n");
        setDefaultColor();
        _exit(1);
    }
    printf("Enter number of Students in IIIT: ");
    scanf("%d", &numStudents);
    numStudentsleft=numStudents;
    if(numStudents<=0){
        printf(ANSI_RED"No Students to be vaccinated\n");
        setDefaultColor();
        _exit(1);
    }
    for(i=0;i<numCompanies;i++){
        companies[i] = (company *)malloc(sizeof(company));
        companies[i]->company_num=i+1;
        printf("Please enter probability of vaccine of Company %d: ", i+1);
        scanf("%f", &companies[i]->probability);
    }

    for(i=0;i<numZones;i++){
        zones[i] = (vaccine_zones *)malloc(sizeof(vaccine_zones));
        zones[i]->zone_num = i+1;
        zones[i]->numVaccines=0;
    }
    for(i=0;i<numStudents;i++){
        students[i] = (student *)malloc(sizeof(student));
        students[i]->cur_status=0;
        students[i]->antibody=0;
        students[i]->num_try=0;
        students[i]->time=0;
        students[i]->num=i+1;
    }

    // Creating threads
    for(i=0;i<numCompanies;i++){
        int *arg = malloc(sizeof(*arg));
        *arg = i;
        pthread_create(&company_thread[i], NULL, production, arg);
        sleep(1);
    }
    for(i=0;i<numZones;i++){
        int *arg = malloc(sizeof(*arg));
        *arg = i;
        pthread_create(&zone_thread[i], NULL, vaccinating, arg);
        sleep(1);
    }
    for(i=0;i<numStudents;i++){
        int *arg = malloc(sizeof(*arg));
        *arg = i;
        pthread_create(&zone_thread[i], NULL, incomingStudents, arg);
        sleep(1);
    }
//    for(i=0;i<numZones;i++){
//        pthread_join(student_thread[i], NULL);
//    }
    for(i=0;i<numZones;i++){
        pthread_join(zone_thread[i], NULL);
    }
    while(numStudentsleft>0)
        ;
}

