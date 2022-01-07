#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dirent.h>

#define MAXNITEMS 1000000
#define MAXNTHREADS 100
int nitems; /* read-only by producer and consumer */
struct {
pthread_mutex_t mutex;
int buff[MAXNITEMS];
int nput;
int nval;
} shared = { PTHREAD_MUTEX_INITIALIZER };
int buff[MAXNITEMS];
struct {
pthread_mutex_t mutex;
int nput; /* next index to store */
int nval; /* next value to store */
} put = { PTHREAD_MUTEX_INITIALIZER };
struct {
pthread_mutex_t mutex;
pthread_cond_t cond;
int nready; /* number ready for consumer */
} nready = { PTHREAD_MUTEX_INITIALIZER,
PTHREAD_COND_INITIALIZER };
void *produce(void *), *consume(void *);

int main(int argc, char **argv) {
int i, nthreads, count[MAXNTHREADS];
pthread_t tid_produce[MAXNTHREADS], tid_consume;

nitems = min(atoi(argv[1]), MAXNITEMS);
nthreads = min(atoi(argv[2]), MAXNTHREADS);
/* create all producers and one consumer */
pthread_setconcurrency(nthreads + 1);
for (i = 0; i < nthreads; i++) {
count[i] = 0;
pthread_create(&tid_produce[i],NULL,produce,&count[i]);
}
pthread_create(&tid_consume, NULL, consume, NULL);
/* wait for all producers and the consumer */
for (i = 0; i < nthreads; i++) {
pthread_join(tid_produce[i], NULL);
printf("count[%d] = %d\n", i, count[i]);
}
pthread_join(tid_consume, NULL);
exit(0);
}

void * produce(void *arg) {
for ( ; ; ) {
pthread_mutex_lock(&put.mutex);
if (put.nput >= nitems) {
pthread_mutex_unlock(&put.mutex);
return(NULL); /* array is full, we’re done */
}
buff[put.nput] = put.nval;
put.nput++;
put.nval++;
pthread_mutex_unlock(&put.mutex);
pthread_mutex_lock(&nready.mutex);
if (nready.nready == 0)
pthread_cond_signal(&nready.cond);
nready.nready++;
pthread_mutex_unlock(&nready.mutex);
*((int *) arg) += 1;
}
}
void * consume(void *arg) {
int i;
for (i = 0; i < nitems; i++) {
pthread_mutex_lock(&nready.mutex);
while (nready.nready == 0)
pthread_cond_wait(&nready.cond, &nready.mutex);
nready.nready--;
pthread_mutex_unlock(&nready.mutex);
if (buff[i] != i)
printf("buff[%d] = %d\n", i, buff[i]);
}
return(NULL);
}