/******************************************************************************
* FILE: program.c
* DESCRIPTION:
*  Broken code using pthreads
******************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* Define and scope what needs to be seen by everyone */
#define NUM_THREADS  3
#define ITERATIONS 10
#define THRESHOLD 12
int flag = 0; //Watcher variable in order to keep track of signal for the loopin bit
int count = 0;
double finalresult=0.0;
pthread_mutex_t count_mutex;
pthread_cond_t count_condvar;


void *sub1(void *t)
{
  int i; 
  long tid = (long)t;
  double myresult=0.0;
 
  /* this seems to highlight one deadlocked interleaving.... 
     the solution is NOT simply to move or delete this sleep command!
  */
  sleep(1);
  /*
  Lock mutex and wait for signal if count has not yet reached the threshold.  
  Note that the pthread_cond_wait routine will automatically and atomically
  unlock mutex while it waits for the signal to proceed.
  */
  pthread_mutex_lock(&count_mutex);
  printf("sub1: thread=%ld going into wait. count=%d\n",tid,count);
  while (!flag) {pthread_cond_wait(&count_condvar, &count_mutex);} //We keep chekcing this all the time for updates
  flag = 0; //reset the watch variable
  printf("sub1: thread=%ld Condition variable signal received.",tid);
  printf(" count=%d\n",count);
  count++;
  finalresult += myresult;
  printf("sub1: thread=%ld count now equals=%d myresult=%e. Done.\n",
         tid,count,myresult);
  pthread_mutex_unlock(&count_mutex);
  pthread_exit(NULL);
}

void *sub2(void *t) 
{
  int j,i;
  long tid = (long)t;
  double myresult=0.0;

  for (i=0; i<ITERATIONS; i++) {
    for (j=0; j<100000; j++)
		myresult += sin(j) * tan(i);
    pthread_mutex_lock(&count_mutex);
    finalresult += myresult;
    count++;
    /* 
    Check count and signal waiting thread when the threshold is reached. 
    */
    if (count == THRESHOLD) {
      printf("sub2: thread=%ld Threshold reached. count=%d. ",tid,count);
	  flag = 1;//WE UPDATED THE THING!
      pthread_cond_signal(&count_condvar);
      printf("Just sent signal.\n");
      }
    else {
      printf("sub2: thread=%ld did work. count=%d\n",tid,count);
      }
    pthread_mutex_unlock(&count_mutex);
    }
    printf("sub2: thread=%ld  myresult=%e. Done. \n",tid,myresult);
  pthread_exit(NULL);
}



int main(int argc, char *argv[])
{
  long t1=1, t2=2, t3=3;
  int i, rc;
  pthread_t threads[3];
  pthread_attr_t attr;

  /* Initialize mutex and condition variable objects */
  pthread_mutex_init(&count_mutex, NULL);
  pthread_cond_init (&count_condvar, NULL);

  /* For portability, explicitly create threads in a joinable state */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&threads[0], &attr, sub1, (void *)t1);
  pthread_create(&threads[1], &attr, sub2, (void *)t2);
  pthread_create(&threads[2], &attr, sub2, (void *)t3);

  /* Wait for all threads to complete */
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  printf ("Main(): Waited on %d threads. Final result=%e. Done.\n",
          NUM_THREADS,finalresult);

  /* Clean up and exit */
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&count_mutex);
  pthread_cond_destroy(&count_condvar);
  pthread_exit (NULL);

}

