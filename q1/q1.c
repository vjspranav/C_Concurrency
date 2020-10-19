#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>

static int cnt;
//Selection sort
void swap(int *xp, int *yp) 
{ 
    int temp = *xp; 
    *xp = *yp; 
    *yp = temp; 
} 
  
void selectionSort(int arr[], int f, int l) 
{ 
    int i, j, min_idx; 
    for (i = f; i < l; i++) { 
        min_idx = i; 
        for (j = i+1; j <= l; j++) 
          if (arr[j] < arr[min_idx]) 
            min_idx = j; 
        swap(&arr[min_idx], &arr[i]); 
    } 
}

// For a shared memory across processes
int * shareMem(size_t size){
     key_t mem_key = IPC_PRIVATE;
     int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
     return (int*)shmat(shm_id, NULL, 0);
}

void merge(int a[], int f, int m, int l){
    int i, j, k;
    int n1 = m-f+1;
    int n2 = l-m;
    int L[n1], R[n2];
    for(i=0;i<n1;i++){
        L[i] = a[f+i];
    }
   for(j=0;j<n2;j++){
        R[j] = a[m+1+j];
    }
    L[i] = INT_MAX;
    R[j] = INT_MAX;
    i=0, j=0;
    for(k=f;k<=l;k++){
        if(L[i]<R[j]){
            a[k] = L[i];
           i++;
        }
        else{
            a[k] = R[j];
            j++;
        }
    }
}

int normal_mergesort(int a[], int f, int l){
    if(f<l){
        int m = (l+f)/2;
        normal_mergesort(a, f, m);
        normal_mergesort(a, m+1, l); 
        merge(a, f, m, l);
    }
}

int process_mergesort(int a[], int f, int l){
    if(f<l){
        int m = (l+f)/2;
        int pid1=fork();
        int pid2;
        if(pid1==0){
            process_mergesort(a, f, m);
            if(m-f<5)
                selectionSort(a, f, m);
            _exit(1);
        }else{
            pid2=fork();
            if(pid2==0){
                process_mergesort(a, m+1, l); 
                // Wait before printing so they don't collide.
                // waitpid(pid2, &status, 0);
                if(l-m+1<5)
                selectionSort(a, f, m);
                _exit(1);
            }else{
                int status;
                waitpid(pid1, &status, 0);
                waitpid(pid2, &status, 0);
                merge(a, f, m, l);
            }
        }
    }
}

struct arg{
     int f;
     int l;
     int* arr;
};

void *threaded_mergesort(void *a){
    struct arg *args = (struct arg*) a;
    int f = args->f;
    int l = args->l;
    int *arr = args->arr;
    if(f>=l) 
        return NULL;

    int m = (l+f)/2;
    //Sending Left Half to merge
    struct arg a1;
        a1.f = f;
        a1.l = m;
        a1.arr = arr;
    pthread_t tid1;
    pthread_create(&tid1, NULL, threaded_mergesort, &a1);
    //Sending Right Half to merge
    struct arg a2;
        a2.f = m+1;
        a2.l = l;
        a2.arr = arr;
    pthread_t tid2;
    pthread_create(&tid2, NULL, threaded_mergesort, &a2);

    // Waiting for threads to complete
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    merge(arr, f, m, l);
}

void runSorts(long long int n){
    struct timespec ts;
    
    //getting shared memory
    int *arr = shareMem(sizeof(int)*(n+1));
    int brr[n+1], crr[n+1];

    /** Taking input and copying into three seperate arrays
     * arr for multiprocess with shared memory
     * brr for multithreaded
     * crr for normal
     */
    printf("Please enter %d elements with a space: ", n);
    for(int i=0;i<n;i++){
        scanf("%d", arr+i);
    }
    for(int i=0;i<n;i++){
        brr[i] = arr[i];
        crr[i] = arr[i];
    }   

    //multiprocess mergesort with timer start
    printf("\nRunning concurrent_mergesort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double st = ts.tv_nsec/(1e9)+ts.tv_sec;

    process_mergesort(arr, 0, n-1);
    for(int i=0; i<n; i++){
        printf("%d ",arr[i]);
    }
    printf("\n\n");
    // multiprocess mergesort with timer stop

    // multithreaded mergesort with timer start
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t1 = en-st;

    pthread_t tid;
    struct arg a;
    a.f = 0;
    a.l = n-1;
    a.arr = brr;
    printf("Running threaded_concurrent_mergesort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;
    // Input 7 87 6 5 4 90  8 6 
    pthread_create(&tid, NULL, threaded_mergesort, &a);
    pthread_join(tid, NULL);
    for(int i=0; i<n; i++){
        printf("%d ",a.arr[i]);
    }
    printf("\n\n");
    // multithreaded mergesort with timer stop

    // normal mergesort with timer start
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t2 = en-st;

    printf("\nRunning normal_merge for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;

    normal_mergesort(crr, 0, n-1);
    for(int i=0; i<n; i++){
        printf("%d ",crr[i]);
    }
    printf("\n\n");
    // normal mergesort with timer stop

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t3 = en - st;

    printf("normal_merge ran:\n\t[ %Lf ] times faster than concurrent_mergesort\n\t[ %Lf ] times faster than threaded_concurrent_mergesort\n\n\n", t1/t3, t2/t3);
    shmdt(arr);
    return;
}

int main(){
    long long int n;
    printf("Please enter length of array: ");
    scanf("%lld", &n);
    runSorts(n);
    return 0;
}