#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_THREADS 8
#define TOT_COUNT 39999

long double myPI[NUM_THREADS];

void *doCals(void *arg){
  
  int idOfThread = (int) arg;
  int pointsInACircle = 0;
  int pointsInASquare = 0;

  long double temporaryPiToAdd =0;

  
  for(int i=0; i<= TOT_COUNT; i++){
    double x = drand48();
    double y = drand48();

    float distanceFromZero = sqrt((x*x)+(y*y));
    
    if(distanceFromZero < 1)
      pointsInACircle++;

    pointsInASquare++;

  }

  long double proportionBetween = (long double) pointsInACircle/pointsInASquare;
  temporaryPiToAdd = 4*proportionBetween;
  myPI[idOfThread] = temporaryPiToAdd;

  pthread_exit(NULL);
}


int main(){

  pthread_t threads[NUM_THREADS];
  int rc;


  for(int t=0; t<NUM_THREADS; t++){
    rc = pthread_create(&threads[t], NULL, doCals, (void *)t);

    if(rc){
      fprintf(stderr,"ERROR; return code from pthread_create() is %d \n", rc);
      exit(-1);
    }
  }
  
  long double averageValue = 0;
  
  for(int t=0; t<NUM_THREADS; t++){
    pthread_join(threads[t], NULL);
    printf("Thread number %d, value of Pi: %Lf \n", t ,myPI[t]);
    averageValue = averageValue + myPI[t];
  }
  printf("Value for myPI is %Lf \n", averageValue/NUM_THREADS);
  
  pthread_exit(NULL);
  
}
