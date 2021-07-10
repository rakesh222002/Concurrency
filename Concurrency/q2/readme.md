Logic:

Every student is a thread and the function associated with students thread is student_fun.
Every Pharmaceutical Company is a thread and the function associated with Companies is pharmacomp_fun.
Every Vaccination zone is a thread and the function associated with Vaccination zones is vaccinationzones.
Students arrive at random times, vaccination zones selects the students and slots at a time depending on
the number of students available.
Similary pharmaceutical companies select the vaccination zones by iterating through the vaccination
zones and deliver a batch if there are no vaccines left in vaccination zones.

Explaination:

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
		printf("Student %d has arrived for %dst round of vaccination\n",stdid,students[stdid].testno);
		while(students[stdid].testno != (check+1) && students[stdid].testflag != 1);
		pthread_mutex_lock(&students[stdid].mutex);
		if(students[stdid].testflag == 1){
			printf("Student %d Passed the test\n",stdid);
			counterstd--;
		}
		else if(students[stdid].testno==4){
			printf("Student %d got negative in all three rounds,He is going back to home\n",stdid);
			students[stdid].testflag = 1;
			counterstd--;
		}
		printf("Counter : %d\n",counterstd);
		pthread_mutex_unlock(&students[stdid].mutex);
		students[stdid].vacflag = 0;
	}
	return NULL;	
}

every student thread has the access to this function.
As every student should come at random times the function has sleep(randomgen(1,7))
which generates random numbers from 1 to 7
then the ready flag is 1 implies that he arrived at gate.
then the student goes to the while loop returns only when testno > 3 or testflag is 1(which is positive antibody test).
Locks are used whenever a shared data is present.

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
		for(int i=0;i < stdcnt;i++)
		{
			pthread_mutex_lock(&students[i].mutex);
			if(students[i].testflag==0 && students[i].vacflag==0 && students[i].readyflag == 1)
			{
				students[i].vacflag = 1;
				vaczone[vzid].arr[slots] = i;
				slots++;
			}
			pthread_mutex_unlock(&students[i].mutex);
			if(slots>8 || vaczone[vzid].slotsleft==slots)break;
		}
		}
		if(counterstd==0)break;
		vaczone[vzid].slotsleft = vaczone[vzid].slotsleft - slots;
		printf("Vaccination Zone %d entering Vaccination Phase with %d slots\n",vzid,slots);
		for(int i=0;i < slots;i++)
		{
			pthread_mutex_lock(&students[vaczone[vzid].arr[i]].mutex);
			printf("Student %d on Vaccination zone %d has been vaccinated which has success probablity %f\n",vaczone[vzid].arr[i],vzid,pc[vaczone[vzid].compid].prob);
			if(testvalue(pc[vaczone[vzid].compid].prob)){
				printf("Student %d has tested positive for antibodies\n",vaczone[vzid].arr[i]);
				students[vaczone[vzid].arr[i]].testflag = 1;
			}
			else{
				printf("Student %d has tested negative for antibodies\n",vaczone[vzid].arr[i]);
				students[vaczone[vzid].arr[i]].testno++;
			}
			pthread_mutex_unlock(&students[vaczone[vzid].arr[i]].mutex);
		}
		//printf("%d slotsleft : %d\n",vzid,vaczone[vzid].slotsleft);
		pthread_mutex_lock(&vaczone[vzid].mutex);
		if(vaczone[vzid].slotsleft==0){
			vaczone[vzid].recflag = 0;
			pc[vaczone[vzid].compid].endcount++;
		}
		pthread_mutex_unlock(&vaczone[vzid].mutex);
	}
	return NULL;
}

vaccination zones only exit when the student counter is 0,(student counter is reduced when student got positive test or completed 3 tests which is shown in student_fun)
while(vaczone[vzid].recflag==0 && counterstd!=0); this while loop stops the vaccination zone until it gets a batch of vaccines.
and the next while loop allots students and slots to vaccination zones and depending on slots no of slots of the zone decrease,when no of slots become 0 it makes the
vacflag = 0 again and wait for a batch,Students test results are also included in this function.
Locks are used whenever a shared data is present.

void *pharmacomp_fun(void* input)
{
	phcomp *extraphc = (phcomp*) input;
	int phcid = extraphc->id;
	while(counterstd!=0)
	{
		pc[phcid].endcount = 0;
		printf("Pharmaceutiacal Company %d is preparing %d batches of vaccines which have Success Probablity %f\n",pc[phcid].id,pc[phcid].r,pc[phcid].prob);
		sleep(pc[phcid].w);
		printf("Pharmaceutiacal Company %d has prepared %d batches of vaccines which have Success Probablity %f\n",pc[phcid].id,pc[phcid].r,pc[phcid].prob);
		int batchcount = pc[phcid].r;
		while(batchcount !=0 && counterstd != 0)
		{
			for(int j=0;j < vzcnt;j++)
			{
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
				if(batchcount==0 || counterstd==0)break;
			}
		}
		while(pc[phcid].endcount < pc[phcid].r && counterstd!=0);
		if(counterstd!=0)printf("All the vaccines prepared by Pharmaceutical Company %d are emptied.Resuming production now\n",phcid);
	}
	return NULL;
}

pharmaceutical companies deliver batches unti student counter(ccounterstd) becomes 0.
The sleep in the function implies the production time of the company.
After preparing the batches it will check for the zones with no vaccines(i.e vacflag=0) then it delivers vaccines until it is out of batches.
while(pc[phcid].endcount < pc[phcid].r && counterstd!=0); this line indicates that until it recieves a signal of completion of all batches it 
wont resume production.

int testvalue(float x)
{
	float ans = (float)randomgen(0,1000)/1000;
	return ans < x;
}

This function is used in vaccinezones function for getting the antibody test results in which it select a random float value from 0 to 1 and 
compares with original probablity,if if (random < probablity) it will return positive else negative.

An array of locks is used for every thread because of not disturbing shared data and mainly for concurrent execution of locks.
Assumptions:
I have made vaccination zones select the students so the waiting part of students is not present instead of that no of students left is printed
when each student exits;

Test Cases checked by me are:
1)3 2 20 
0.1 0.6 0.7

2)1 1 100
0.5

3)8 2 50
0.2 0.3 0.4 0.2 0.9 0.8 0.7 0.67

4)2 3 50
0.2 0.3

5)3 2 20
0.2 0.5 0.6

It worked for all these test cases.