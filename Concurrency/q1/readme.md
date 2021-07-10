Normal Merge Sort runs faster than both Concurrent Processes and Concurrent Threads merge sort
 for most test cases. That is test for small test cases. Threaded may be  faster for the 
 big test cases.

Conclusions:
1)There is some lag in creating processes and threads, which is not there in normal merge sort.
2)Context switches happen in process and threaded merge sort which consumes CPU time which is 
not present in normal merge sort.

example test cases:
1) 20
-2 4 100 3 6 9999 -330 28 31 -39 10293 65 -128 918237 -129 0 12 -2 4 5
Process merge sort
-330 -129 -128 -39 -2 -2 0 3 4 4 5 6 12 28 31 65 100 9999 10293 918237 
Time taken by process merge sort is 0.001550
Normal merge sort
-330 -129 -128 -39 -2 -2 0 3 4 4 5 6 12 28 31 65 100 9999 10293 918237 
Time taken by normal merge sort is 0.000026
Threaded merge sort
-330 -129 -128 -39 -2 -2 0 3 4 4 5 6 12 28 31 65 100 9999 10293 918237 
Time taken by Threaded merge sort is 0.000308
Normal merge sort is 58.721257 times faster than process merge sort
Normal merge sort is 11.666186 times faster than thread merge sort

2) 40
-2 -2 -100 3 4 10 1 22 44 -11 384 -12398 138 1834 -123 -138746 -2 34 -23 1247 1247 1238 -457 678 384 -123 -456 19 45 -23 11 15 09 1 45 16 -12398 67 24 40
Process merge sort
-138746 -12398 -12398 -457 -456 -123 -123 -100 -23 -23 -11 -2 -2 -2 1 1 3 4 9 10 11 15 16 19 22 24 34 40 44 45 45 67 138 384 384 678 1238 1247 1247 1834 
Time taken by process merge sort is 0.001279
Normal merge sort
-138746 -12398 -12398 -457 -456 -123 -123 -100 -23 -23 -11 -2 -2 -2 1 1 3 4 9 10 11 15 16 19 22 24 34 40 44 45 45 67 138 384 384 678 1238 1247 1247 1834 
Time taken by normal merge sort is 0.000007
Threaded merge sort
-138746 -12398 -12398 -457 -456 -123 -123 -100 -23 -23 -11 -2 -2 -2 1 1 3 4 9 10 11 15 16 19 22 24 34 40 44 45 45 67 138 384 384 678 1238 1247 1247 1834 
Time taken by Threaded merge sort is 0.000132
Normal merge sort is 188.368334 times faster than process merge sort
Normal merge sort is 19.388954 times faster than thread merge sort

From the above results we can see that normal merge sort is faster than other two and threaded merge sort is faster than process merge sort
because threads are faster to start than processes because very little memory copying is required (just a thread stack).

Code Explaination:

Normal merge sort:
void selectSort(int *arr,int len)
{
	int minind,temp;
	for(int i=0;i< len;i++)
	{
		minind = i;
		for(int j=i+1;j< len;j++)if(arr[j]< arr[minind])minind = j;
		temp = arr[minind];
		arr[minind] = arr[i];
		arr[i] = temp;
	}	
}
this is selection sort which finds the minimum in every iteration and swaps it according to first loop.
merge is done as usually, no need to explain.
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
this is the traditional merge sort with a modification of selection sort
which divides the array into left and right halfs and when any of the array
size is < 5 selection sort takes place. 
.
.
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
this is merge sort in which left and right half divides into processes(using forks).
this is like a normal merge sort in which arrays are divided and allocated to different
processes.
.
.
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
this is same as process merge sort in which processes are replaced by threads. 