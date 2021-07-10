#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>
struct threadarr{
	int* arr;
	int st,en;
};
int * sharedmemory(size_t sz){
     key_t mem_key = IPC_PRIVATE;
     int shm_id = shmget(mem_key, sz, IPC_CREAT | 0666);
     return (int*)shmat(shm_id, NULL, 0);
}
void merge(int *arr,int st,int mid,int en)
{
	int x,y;
	x = (mid+1)-st;
	y = en - mid;
	int larr[x],rarr[y];
	for(int i=0;i<y;i++)rarr[i] = arr[mid+1+i];
	for(int i=0;i<x;i++)larr[i] = arr[st+i];
	int a=0,b=0,c = st;
	while(a < x && b < y)
	{
		if(larr[a] > rarr[b])
		{
			arr[c] = rarr[b];
			b++;
		}
		else
		{
			arr[c] = larr[a];
			a++;
		}
		c++;
	}
	while(a < x)
	{
		arr[c] = larr[a];
		a++;
		c++;
	}
	while(b < y)
	{
		arr[c] = rarr[b];
		b++;
		c++;
	}	
}
void selectSort(int *arr,int len)
{
	int minind,temp;
	for(int i=0;i<len;i++)
	{
		minind = i;
		for(int j=i+1;j<len;j++)if(arr[j]<arr[minind])minind = j;
		temp = arr[minind];
		arr[minind] = arr[i];
		arr[i] = temp;
	}	
}
void procmergesort(int *arr,int st,int en)
{
	int arrsize = en-st+1;
	if(arrsize < 5){
		selectSort(arr+st,arrsize);
		return;
	}
	int mid = st + (en-st)/2;
	pid_t pid1,pid2;
	pid1 = fork();
	if(pid1<0){
		printf("unable to create fork\n");
		exit(-1);
	}
	else if(pid1==0)
	{
		procmergesort(arr, st, mid);
		exit(1);
	}
	pid2 = fork();
	if(pid2<0){
		printf("unable to create fork\n");
		exit(-1);
	}
	else if(pid2==0)
	{
		procmergesort(arr, mid+1, en);
		exit(1);
	}
	int status;
	waitpid(pid1,&status,0);
	waitpid(pid2,&status,0);
	merge(arr,st,mid,en);
	return;
}
void normalmergesort(int *arr,int st,int en)
{
	int arrsize = en-st+1;
	if(arrsize < 5){
		selectSort(arr+st,arrsize);
		return;
	}
	int mid = st + (en-st)/2;
	normalmergesort(arr, st, mid);
	normalmergesort(arr, mid+1, en);
	merge(arr,st,mid,en);
}
void *threadmergesort(void *x)
{
	struct threadarr *args = (struct threadarr*) x;
	int st = args->st,en = args->en;
	int *exarr = args->arr;
	int arrsize = en-st+1;
	if(arrsize < 5){
		selectSort(exarr+st,arrsize);
		return NULL;
	}
	int mid = st + (en-st)/2;
	struct threadarr args1;
	args1.st = st;
	args1.en = mid;
	args1.arr = exarr;
    pthread_t tid1;
    pthread_create(&tid1, NULL, threadmergesort, &args1);
    struct threadarr args2;
	args2.st = st;
	args2.en = mid;
	args2.arr = exarr;
    pthread_t tid2;
    pthread_create(&tid2, NULL, threadmergesort, &args2);
    merge(exarr,st,mid,en);
}
void sorttypes(int n)
{
	struct timespec tim; 
	int *arr = sharedmemory(sizeof(int)*(n+1));
	int exarr[n+1];
	for(int i=0;i<n;i++)scanf("%d",arr+i);
	for(int i=0;i<n;i++)exarr[i] = arr[i];
	struct threadarr tharr;
	tharr.st = 0;
	tharr.en = n-1;
	tharr.arr = exarr;
	//process merge sort
	printf("Process merge sort\n");
	clock_gettime(CLOCK_MONOTONIC_RAW, &tim);
	long double st = tim.tv_nsec/(1e9)+tim.tv_sec;
	procmergesort(arr,0,n-1);
	for(int i=0;i<n;i++)printf("%d ",arr[i]);
	clock_gettime(CLOCK_MONOTONIC_RAW, &tim);
	long double en = tim.tv_nsec/(1e9)+tim.tv_sec;
	printf("\n");
	printf("Time taken by process merge sort is %Lf\n",en-st);
	long double proctime = en-st;
	//Normal mergesort;
	printf("Normal merge sort\n");
	clock_gettime(CLOCK_MONOTONIC_RAW, &tim);
	st = tim.tv_nsec/(1e9)+tim.tv_sec;
	normalmergesort(exarr,0,n-1);
	for(int i=0;i<n;i++)printf("%d ",exarr[i]);
	clock_gettime(CLOCK_MONOTONIC_RAW, &tim);
	en = tim.tv_nsec/(1e9)+tim.tv_sec;
	printf("\n");
	printf("Time taken by normal merge sort is %Lf\n",en-st);
	long double normtime = en - st;
	//threaded merge sort
	printf("Threaded merge sort\n");
	pthread_t tid;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tim);
	st = tim.tv_nsec/(1e9)+tim.tv_sec;
    pthread_create(&tid, NULL, threadmergesort, &tharr);
    pthread_join(tid, NULL);
    for(int i=0;i<n;i++)printf("%d ",tharr.arr[i]);
    printf("\n");
	clock_gettime(CLOCK_MONOTONIC_RAW, &tim);
    en = tim.tv_nsec/(1e9)+tim.tv_sec;
    printf("Time taken by Threaded merge sort is %Lf\n",en-st);
    long double threadtime = en-st;
    printf("Normal merge sort is %Lf times faster than process merge sort\n",proctime/normtime);
    printf("Normal merge sort is %Lf times faster than thread merge sort\n",threadtime/normtime);
}
int main()
{
	int n;
	scanf("%d",&n);
	sorttypes(n);
	return 0;
}