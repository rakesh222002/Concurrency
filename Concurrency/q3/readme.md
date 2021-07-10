Logic :

we have three functions srujana, stages_accoustic and stages_electric. Every musician arrives at srujana and notified
if they arrive according to their arrival times.
Then srujana function decides which musician should go to which stage according to instrument they play.
Then both stage functions uses semaphores to allow musicains where semaphore value is number of stages.

Code Explaination:

void *srujana(void* input)
{
	Person *inpper = (Person*) input;
	int pmrid = inpper->id;
	sleep(performer[pmrid].arrtime);
	performer[pmrid].status = WAITINGTOPERFORM;
	printf("%s %c arrived\n",performer[pmrid].name,performer[pmrid].instrument);
	//pthread_mutex_lock(&performer[pmrid].mutex);
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
	//pthread_mutex_unlock(&performer[pmrid].mutex);
	return NULL;
}
Every musician and singer arrives at srujana immediately after the input but waits for completion os their arrival time.
Then they are alloted stages according to their instrument characters,if they are eligible for both a random allocation is 
done.

void *stages_Accoustic(void* input)
{
	Person *inpper = (Person*)input;
	int pmrid = inpper->id;
	pthread_mutex_lock(&performer[pmrid].mutex);
	performer[pmrid].status = WAITINGTOPERFORM;
	if(performer[pmrid].type[3]==1)
	{
		for(int i=0;i < k;i++)
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
		printf("%s %c left because of impatience\n",performer[pmrid].name,performer[pmrid].instrument);
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
	return NULL;
}

This part of the code takes care of the performers ariived at accoustic stage,at first it will check that the arrived performer is
singer or not,if he is a singer it will check for a stage with musician without singer if there is any he will join them.
this is the snippet for that.
	if(performer[pmrid].type[3]==1)
	{
		for(int i=0;i < k;i++)
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
Next there is a time checking part which checks for the time for every performer,if its more than threshold time he will exit.
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += threshtime; 
	int s = sem_timedwait(&sem_acc, &ts);
	if(s == -1){
		printf("%s %c left because of impatience\n",performer[pmrid].name,performer[pmrid].instrument);
		}
There is a semaphore used for no of stages sem_acc which will only allow musicains according to no of stages. 
This is similar for electric stages.
Assumption : Singer joins in the middle but notified 2 sec before end of performance 
Statuses of musicians are used for debugging. 

The test case checked is 

1)4 1 1 2 5 10 5
Rakesh p 0
siva b 1
manasa g 0
sriya s 1
