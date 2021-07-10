#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
int compcnt,vzcnt,stdcnt,counterstd; //3 2 20 0.1 0.6 0.7
typedef struct phcomp{
	int id,w,r,p,endcount;
	float prob;
	pthread_mutex_t mutex;
} phcomp;
typedef struct student{
	int id,testno,testflag,vacflag,readyflag;
	pthread_mutex_t mutex;
} student;
typedef struct vzone{
	int id,startflag,compid,recflag,arr[8],slotsleft;
	pthread_mutex_t mutex;	
} vzone;
pthread_mutex_t compmutex,stdmutex,vzmutex;
phcomp pc[100000];
student students[100000];
vzone vaczone[100000];
pthread_mutex_t new_lock;
int randomgen(int x,int y)
{
	int ans = rand() % (y - x + 1) + x;
	return ans;
}
int testvalue(float x)
{
	float ans = (float)randomgen(0,1000)/1000;
	return ans < x;
}
void *pharmacomp_fun(void* input)
{
	phcomp *extraphc = (phcomp*) input;
	int phcid = extraphc->id;
	while(counterstd!=0)
	{
		pthread_mutex_lock(&pc[phcid].mutex);		
		pc[phcid].endcount = 0;
		pthread_mutex_unlock(&pc[phcid].mutex);
		printf("Pharmaceutiacal Company %d is preparing %d batches of vaccines which have Success Probablity %f\n",pc[phcid].id,pc[phcid].r,pc[phcid].prob);
		sleep(pc[phcid].w);
		printf("Pharmaceutiacal Company %d has prepared %d batches of vaccines which have Success Probablity %f\n",pc[phcid].id,pc[phcid].r,pc[phcid].prob);
		int batchcount = pc[phcid].r;
		while(batchcount !=0 && counterstd != 0)
		{
			for(int j=0;j<vzcnt;j++)
			{
				if(batchcount==0 || counterstd==0)break;
				pthread_mutex_lock(&vaczone[j].mutex);
				if(vaczone[j].recflag == 0)
				{
					vaczone[j].compid = phcid;
					vaczone[j].recflag = 1;
					vaczone[j].slotsleft = pc[phcid].p;
					batchcount--;
					printf("Pharmaceutical Company %d is delivering a vaccine batch to Vaccination Zone %d which has success probablity %f\n",phcid,j,pc[phcid].prob);
				}
				pthread_mutex_unlock(&vaczone[j].mutex);
			}
		}
		while(pc[phcid].endcount<pc[phcid].r && counterstd!=0);
		if(counterstd!=0)printf("All the vaccines prepared by Pharmaceutical Company %d are emptied.Resuming production now\n",phcid);
	}
	return NULL;
}
void *vaccinezones(void* input)
{
	vzone *vzinfun = (vzone*) input;
	int vzid = vzinfun->id;
	int slots;
	while(counterstd!=0)
	{	
		while(vaczone[vzid].recflag==0 && counterstd!=0);
		slots=0;
		while(slots<1 && counterstd!=0){
		for(int i=0;i<stdcnt;i++)
		{
			//printf("rakesh \n");
			pthread_mutex_lock(&students[i].mutex);
			if(students[i].testflag==0 && students[i].vacflag==0 && students[i].readyflag == 1)
			{
				printf("%d is selected by vaccinezone %d\n",students[i].id,vaczone[vzid].id);
				students[i].vacflag = 1;
				pthread_mutex_lock(&vaczone[vzid].mutex);
				vaczone[vzid].arr[slots] = i;
				pthread_mutex_unlock(&vaczone[vzid].mutex);
				slots++;
			}
			pthread_mutex_unlock(&students[i].mutex);
			if(slots>=8 || vaczone[vzid].slotsleft==slots)break;
		}
			if(slots>=8 || vaczone[vzid].slotsleft==slots)break;
		}
		if(counterstd==0)break;
		pthread_mutex_lock(&vaczone[vzid].mutex);
		vaczone[vzid].slotsleft = vaczone[vzid].slotsleft - slots;
		pthread_mutex_unlock(&vaczone[vzid].mutex);
		printf("Vaccination Zone %d entering Vaccination Phase with %d slots\n",vzid,slots);
		for(int i=0;i<slots;i++)
		{
			pthread_mutex_lock(&students[vaczone[vzid].arr[i]].mutex);
			pthread_mutex_lock(&vaczone[vzid].mutex);
			printf("Student %d on Vaccination zone %d has been vaccinated which has success probablity %f\n",vaczone[vzid].arr[i],vzid,pc[vaczone[vzid].compid].prob);
			if(testvalue(pc[vaczone[vzid].compid].prob)){
				printf("Student %d has tested positive for antibodies\n",vaczone[vzid].arr[i]);
				students[vaczone[vzid].arr[i]].testflag = 1;
				students[vaczone[vzid].arr[i]].vacflag = 0;
			}
			else{
				printf("Student %d has tested negative for antibodies\n",vaczone[vzid].arr[i]);
				students[vaczone[vzid].arr[i]].testno++;
				students[vaczone[vzid].arr[i]].vacflag = 0;
			}
			pthread_mutex_unlock(&vaczone[vzid].mutex);
			pthread_mutex_unlock(&students[vaczone[vzid].arr[i]].mutex);
		}
		//printf("%d slotsleft : %d\n",vzid,vaczone[vzid].slotsleft);
		pthread_mutex_lock(&vaczone[vzid].mutex);
		if(vaczone[vzid].slotsleft==0){
			vaczone[vzid].recflag = 0;
			pc[vaczone[vzid].compid].endcount++;
			printf("Vaccination zone %d is out of vaccines\n",vzid);
		}
		pthread_mutex_unlock(&vaczone[vzid].mutex);
	}
	return NULL;
}
void *student_fun(void* input)
{
	student *extrastd = (student*) input;
	int stdid = extrastd->id;
	int check;
	sleep(randomgen(1,7));
	students[stdid].readyflag = 1;
	while(students[stdid].testno<=3 && students[stdid].testflag==0)
	{
		check = students[stdid].testno;
		printf("Student %d has arrived for round %d of vaccination\n",stdid,students[stdid].testno);
		while(students[stdid].testno != (check+1) && students[stdid].testflag != 1);
		pthread_mutex_lock(&students[stdid].mutex);
		if(students[stdid].testflag == 1){
			printf("Student %d Passed the test\n",stdid);
			pthread_mutex_lock(&new_lock);
			counterstd--;
			pthread_mutex_unlock(&new_lock);
		}
		else if(students[stdid].testno==4){
			printf("Student %d got negative in all three rounds,He is going back to home\n",stdid);
			students[stdid].testflag = 1;
			pthread_mutex_lock(&new_lock);
			counterstd--;
			pthread_mutex_unlock(&new_lock);
		}
		printf("Students left : %d\n",counterstd);
		pthread_mutex_unlock(&students[stdid].mutex);
	}
	return NULL;	
}
int main(){
	srand(time(0));
	scanf("%d %d %d",&compcnt,&vzcnt,&stdcnt);
	counterstd = stdcnt;
	pthread_t sttid[stdcnt+1],vztid[vzcnt+1],pctid[compcnt+1];
	float args[compcnt+1];
	for(int i=0;i<compcnt;i++)scanf("%f",args+i);
	pthread_mutex_init(&new_lock,NULL);
	for(int i=0;i<stdcnt;i++){
		students[i].id = i;
		students[i].testno = 1;
		students[i].testflag = 0;
		students[i].readyflag = 0;
		students[i].vacflag = 0;
		pthread_mutex_init(&students[i].mutex, NULL);
		pthread_create(&sttid[i],NULL, student_fun,&students[i]);
	}
	for(int i=0;i<compcnt;i++){
		pc[i].id = i;	
		pc[i].prob = args[i];
		pc[i].w = randomgen(2,5);
		pc[i].r = randomgen(1,5);
		pc[i].p = randomgen(1,10);
		pthread_mutex_init(&pc[i].mutex, NULL);
		pthread_create(&pctid[i], NULL, pharmacomp_fun, &pc[i]);
	}
	for(int i=0;i<vzcnt;i++)
	{
		vaczone[i].id = i;
		vaczone[i].startflag = 0;
		vaczone[i].recflag = 0;
		vaczone[i].slotsleft = 0;
		pthread_mutex_init(&vaczone[i].mutex, NULL);
		pthread_create(&vztid[i], NULL, vaccinezones, &vaczone[i]); 
	}
	for(int i=0;i<stdcnt;i++)pthread_join(sttid[i],NULL);
	for(int i=0;i<compcnt;i++)pthread_join(pctid[i],NULL);
	for(int i=0;i<vzcnt;i++)pthread_join(vztid[i],NULL);
	printf("Simulation is Over\n");
	return 0;
}