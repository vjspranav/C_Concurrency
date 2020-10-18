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
 * num_slots - Current number of slots in use
 * company_num - the number of company from which it recieved vaccine
 */
typedef struct vaccine_zones {
    float probability;
    int slots[8];
    int is_vaccinating;
    int zone_num;
    int numVaccines;
    int num_slots;
    int company_num;
    pthread_mutex_t mutex;    
} vaccine_zones;

/**
 * Student Struct
 * cur_status - 0 if not vaccinated/1 if vaccinating/2 if vaccinated
 * num_try - how many times he completed so far
 * antibody - 0 if negative/1 if possitive
 */ 
typedef struct student{
    int cur_status;
    int num_try;
    int antibody;
}student;

int numVaccines, numCompanies, numZones, numStudents, numStudentsleft;
pthread_t company_thread[200], zone_thread[200], student_thread[200];
company *companies[200];
vaccine_zones *zones[200];
student *students[200];
pthread_mutex_t mutex;

/* Main Functions */

int random(int min, int max){
    return (rand()%( max -min))+min;
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
    int num=*(int *) arg;
    while(numStudentsleft>0){
        // Waiting for company to provide vaccine
        while(zones[num]->numVaccines==0)
            ;
        pthread_mutex_lock(&mutex);
        printf("Zone %d Recieved %d vaccines from Company %d\n", zones[num]->zone_num, zones[num]->numVaccines, zones[num]->company_num);
        printf("%d Students Vaccinated Remaining =%d\n", zones[num]->numVaccines, numStudentsleft-(zones[num]->numVaccines));
        if(numStudentsleft>zones[num]->numVaccines){
            numStudentsleft-=zones[num]->numVaccines;
            zones[num]->numVaccines=0;
            companies[zones[num]->company_num-1]->num_batches-=1;
        }else{
            printf("Simulation Done\n");
            _exit(0);
        }
        pthread_mutex_unlock(&mutex);
        sleep(2);
    }
}
void *incomingStudents(){}

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

    for(i=0;i<numZones;i++){
        pthread_join(zone_thread[i], NULL);
    }

}

