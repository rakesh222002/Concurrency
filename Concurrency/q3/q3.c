#include <stdio.h> 
#include <string.h>
#include <pthread.h> 
#include <semaphore.h> 
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#define NOTARRIVED 0
#define WAITINGTOPERFORM 1
#define PERFORMINGSOLO 2
#define PERFORMINGDOUBLE 3
#define WAITINGFORTSHIRT 4
#define TSHIRTBEINGCOLLECTED 5
#define EXITED 6
int randomgen(int x,int y)
{
	int ans = rand() % (y - x + 1) + x;
	return ans;
}
int k,acc,elec,clubco,time1,time2,threshtime;
typedef struct Person{
	char name[30],instrument;
	int type[4],status,id,arrtime,perfflag,singflag,singid;
	pthread_mutex_t mutex;
} Person;
Person performer[100000];
sem_t sem_acc,sem_elec,sem_club;
void *stages_Accoustic(void* input)
{
	Person *inpper = (Person*)input;
	int pmrid = inpper->id;
	pthread_mutex_lock(&performer[pmrid].mutex);
	performer[pmrid].status = WAITINGTOPERFORM;
	if(performer[pmrid].type[3]==1)
	{
		for(int i=0;i<k;i++)
		{
			if(performer[i].perfflag==1 && performer[i].singflag==0 && performer[i].type[3]==0)
			{
				performer[i].singflag = 1;
				performer[i].singid = pmrid;
				return NULL;
				break;
			}
		}
	}	
	pthread_mutex_unlock(&performer[pmrid].mutex);
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += threshtime; 
	int s = sem_timedwait(&sem_acc, &ts);
	if(s == -1){
		printf("%s %c left because of impatience from a accoustic stage\n",performer[pmrid].name,performer[pmrid].instrument);
		return NULL;
	}
	performer[pmrid].perfflag = 1;
	printf("%s performing %c at accoustic stage\n",performer[pmrid].name,performer[pmrid].instrument);
	sleep(randomgen(time1,time2));
	if(performer[pmrid].singflag == 1){
		printf("%s joined %s's performance,performance extended by 2sec\n",performer[performer[pmrid].singid].name,performer[pmrid].name);
		sleep(2);
	}
	performer[pmrid].perfflag=0;
	sem_post(&sem_acc);
	sem_wait(&sem_club);
	printf("%s Collecting Tshirt\n",performer[pmrid].name);
	sleep(2);
	sem_post(&sem_club);
	printf("%s is exitting\n",performer[pmrid].name);
	performer[pmrid].status = EXITED;
	return NULL;
}
void *stages_Electric(void* input)
{
	Person *inpper = (Person*)input;
	int pmrid = inpper->id;
	pthread_mutex_lock(&performer[pmrid].mutex);
	performer[pmrid].status = WAITINGTOPERFORM;
	if(performer[pmrid].type[3]==1)
	{
		for(int i=0;i<k;i++)
		{
			if(performer[i].perfflag==1 && performer[i].singflag==0 && performer[i].type[3]==0)
			{
				performer[i].singflag = 1;
				performer[i].singid = pmrid;
				return NULL;
				break;
			}
		}
	}	
	pthread_mutex_unlock(&performer[pmrid].mutex);
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += threshtime; 
	int s = sem_timedwait(&sem_elec, &ts);
	if(s == -1){
		printf("%s %c left because of impatience from a electric stage\n",performer[pmrid].name,performer[pmrid].instrument);
		return NULL;
	}
	performer[pmrid].perfflag = 1;
	printf("%s performing %c at electric stage\n",performer[pmrid].name,performer[pmrid].instrument);
	sleep(randomgen(time1,time2));
	if(performer[pmrid].singflag == 1){
		printf("%s joined %s's performance,performance extended by 2sec\n",performer[performer[pmrid].singid].name,performer[pmrid].name);
		sleep(2);
	}
	performer[pmrid].perfflag=0;
	sem_post(&sem_elec);
	sem_wait(&sem_club);
	printf("%s Collecting Tshirt\n",performer[pmrid].name);
	sleep(2);
	sem_post(&sem_club);
	printf("%s is exitting\n",performer[pmrid].name);
	performer[pmrid].status = EXITED;
	return NULL;
}
void *srujana(void* input)
{
	Person *inpper = (Person*) input;
	int pmrid = inpper->id;
	sleep(performer[pmrid].arrtime);
	performer[pmrid].status = WAITINGTOPERFORM;
	printf("%s %c arrived\n",performer[pmrid].name,performer[pmrid].instrument);
	if(performer[pmrid].type[0])stages_Accoustic(&performer[pmrid]);
	else if(performer[pmrid].type[1])stages_Electric(&performer[pmrid]);
	else if(performer[pmrid].type[2])
	{
		int rnum = randomgen(1,2);
		if(rnum==1)stages_Accoustic(&performer[pmrid]);
		else stages_Electric(&performer[pmrid]);
	}
	else if(performer[pmrid].type[3])
	{
		int rnum = randomgen(1,2);
		if(rnum==1)stages_Accoustic(&performer[pmrid]);
		else stages_Electric(&performer[pmrid]);
	}
	return NULL;
} 
int main()
{
	srand(0);
	int ind=0;
	scanf("%d %d %d %d %d %d %d",&k,&acc,&elec,&clubco,&time1,&time2,&threshtime);
	pthread_t ptid[k];
	sem_init(&sem_acc,0,acc);
	sem_init(&sem_elec,0,elec);
	sem_init(&sem_club,0,clubco);
	for(int i=0;i<k;i++)
	{
		scanf("%s %c %d",performer[i].name,&performer[i].instrument,&performer[i].arrtime);//input for musicians and singers
		performer[i].status = NOTARRIVED;
		performer[i].id = i;
		performer[i].perfflag = 0;
		performer[i].singflag = 0;
		for(int j=0;j<4;j++)performer[i].type[j]=0;
		if(performer[i].instrument=='p')performer[i].type[2] = 1;
		else if(performer[i].instrument=='g')performer[i].type[2] = 1;
		else if(performer[i].instrument=='v')performer[i].type[0] = 1;
		else if(performer[i].instrument=='b')performer[i].type[1] = 1;
		else if(performer[i].instrument=='s')performer[i].type[3] = 1;
	}
	for(int i=0;i<k;i++)
	{
		pthread_mutex_init(&performer[i].mutex,NULL);
		pthread_create(&ptid[i], NULL, srujana, &performer[i]);
	}
	for(int i=0;i<k;i++)pthread_join(ptid[i],NULL);
	printf("Finished\n");
}