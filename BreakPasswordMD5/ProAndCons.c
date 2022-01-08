#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "openssl/md5.h"

# define NUMBEROFPRODUCER 4

/*TESTOWE DANE*/
char password[3][33] = {"1d7c2923c1684726dc23d2901c4d81570", "2c4c8979a63e1eb71c23ae173a60dd0c0", "518ff78aea202ba72d59851aa8462fb50" };;

char* slownik[10] = {"adam", "marek", "jan", "kuba", "michal", "adrian", "ola", "jacek", "mariola", "weronika"};

/*KONIEC TESTOWYCH DANYCH */


void producer(void *);
void consumer(void *);

struct{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int nready; /*number of ready consumer*/
} nready = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER}; 

struct {
    pthread_mutex_t mutex;
    int nput; /* next index to store */
    int nval; /* next value to store */
} put = { PTHREAD_MUTEX_INITIALIZER };

int main(int argc, char **argv){
   pthread_t tid_producer[NUMBEROFPRODUCER], tid_consumer;

    /* create all producers and one consumer */
/*    for(int t=0; t<NUMBEROFPRODUCER; t++) {
        printf("Main: creating thread %d\n", t);
        int rc = pthread_create(&tid_producer[t], NULL, producer, (void *)t);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    pthread_create(&tid_consumer,NULL,consumer,NULL);
*/
    /* wait for all producers and the consumer */
 /*  for (int i = 0; i < NUMBEROFPRODUCER; i++) {
        pthread_join(tid_producer[i], NULL);
    }
    pthread_join(tid_consumer, NULL);

    pthread_mutex_destroy(&nready.mutex);
    pthread_mutex_destroy(&put.mutex);
    pthread_cond_destroy(&nready.cond);
    pthread_exit (NULL);
*/

    for(int h=0; h<10; h++){
        printf("haslo %s \n", slownik[h]);
        unsigned char digest[32];
        MD5(slownik[h], strlen(slownik[h]), (unsigned char*)&digest); 
                        
        char convertFromHex[32];
        for(int i = 0; i < 16; i++)
            sprintf(&convertFromHex[i*2], "%02x", (unsigned int)digest[i]);

        for(int i=0; i<3; i++){
            if(password[i][32] == '0'){

                for(int i = 0; i < 32; i++){
                    printf("%c", convertFromHex[i]);
                }
                printf("\n");

                for(int k = 0; k < 32; k++){
                printf("%c", password[i][k]);
                }
                printf("\n");

                if(strncmp(password[i], convertFromHex, 32)==0){
                    printf("Haslo zlamane \n");

                }
            }
        }
    }

}


void producer(void *threadid){
    pthread_mutex_lock(&put.mutex);
    pthread_mutex_unlock(&put.mutex);
    pthread_mutex_lock(&nready.mutex);
    pthread_cond_signal(&nready.cond);
    pthread_mutex_unlock(&nready.mutex);

    pthread_exit (NULL);
}


void consumer(void *threadid){

    pthread_mutex_lock(&nready.mutex);

    pthread_cond_wait(&nready.cond, &nready.mutex);

    pthread_mutex_unlock(&nready.mutex);

    pthread_exit (NULL);
}