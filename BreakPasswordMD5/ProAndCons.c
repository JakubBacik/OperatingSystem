#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include "openssl/md5.h"

# define NUMBEROFPRODUCER 1
# define NUMBER 20 

/*TESTOWE DANE*/
char password[3][33] = {"1d7c2923c1684726dc23d2901c4d81570", "2c4c8979a63e1eb71c23ae173a60dd0c0", "518ff78aea202ba72d59851aa8462fb50" };;

char* slownik[10] = {"adam", "marek", "jan", "kuba", "michal", "adrian", "ola", "jacek", "mariola", "weronika"};

/*KONIEC TESTOWYCH DANYCH */



void handler (int sig);
void producer(void *);
void consumer(void *);
char* Generate(char* password, int number78);
char* appendCharToCharArray(char* array, char a);
char MakePassword(char* password, int numner);
int numberofBreakPass[1000][2];
struct{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int nready; /*number of ready consumer*/ 
} nready = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER,0}; 

struct {
    pthread_mutex_t mutex;
    int number; /* next index to store */
} put = { PTHREAD_MUTEX_INITIALIZER };

int main(int argc, char **argv){
    struct sigaction act;
    memset (&act, 0, sizeof (act));
    act.sa_handler = handler;

    if (sigaction (SIGHUP, &act, 0) < 0) {
        perror ("sigaction");
        exit (-1);
    }

    pthread_t tid_producer[NUMBEROFPRODUCER], tid_consumer;

    /* create all producers and one consumer */
    for(int t=0; t<NUMBEROFPRODUCER; t++) {
        printf("Main: creating thread %d\n", t);
        int rc = pthread_create(&tid_producer[t], NULL, producer, (void *)t);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    pthread_create(&tid_consumer,NULL,consumer,NULL);

    /* wait for all producers and the consumer */
    for (int i = 0; i < NUMBEROFPRODUCER; i++) {
        pthread_join(tid_producer[i], NULL);
    }
    pthread_join(tid_consumer, NULL);

    pthread_mutex_destroy(&nready.mutex);
    pthread_mutex_destroy(&put.mutex);
    pthread_cond_destroy(&nready.cond);
    pthread_exit (NULL);


}


void producer(void *threadid){
    for(int j=0; ;j++){
        for(int h=0; h<10; h++){

            char* passToMD5 = Generate(slownik[h], j);
            printf("%s\n", passToMD5);
    
            unsigned char digest[32];
            MD5(passToMD5, strlen(passToMD5), (unsigned char*)&digest); 
                            
            char convertFromHex[32];
            for(int i = 0; i < 16; i++)
                sprintf(&convertFromHex[i*2], "%02x", (unsigned int)digest[i]);

            for(int i=0; i<3; i++){
                if(password[i][32] == '0'){
                    if(strncmp(password[i], convertFromHex, 32)==0){
                        
                        printf("Wyslanie\n");

                        pthread_mutex_lock(&put.mutex);
                        numberofBreakPass[put.number][0]=h;
                        numberofBreakPass[put.number][1]=i;
                        put.number++;
                        pthread_mutex_unlock(&put.mutex);

                        pthread_mutex_lock(&nready.mutex);
                        if (nready.nready == 0){
                            pthread_cond_signal(&nready.cond);
                        }
                        nready.nready++;    
                        pthread_mutex_unlock(&nready.mutex);
                    }
                    
                }
            }
        }
        

    }
    pthread_exit (NULL);
}


void consumer(void *threadid){
    for(int i=0; i<4; i++){
        pthread_mutex_lock(&nready.mutex);
        while(nready.nready == 0)
            pthread_cond_wait(&nready.cond, &nready.mutex);
        nready.nready--;
        password[i][33] == '0';
        printf("Cracked password is:");
        printf("%s \n", slownik[numberofBreakPass[i][0]]);
        
        pthread_mutex_unlock(&nready.mutex);
    }
    
    pthread_exit (NULL);
}

void handler (int signal_number){
    printf ("\n The number of broken passwords is %d\n \n", put.number);
}


char MakePassword(char* password, int numner){

}


char *addToTab = "0123456789!@#$%^&*()";
char* appendCharToCharArray(char* array, char a)
{
    size_t len = strlen(array);

    char* ret = (char *) calloc(len+2, sizeof(char));

    strcpy(ret, array);
    ret[len] = a;
    ret[len+1] = '\0';

    return ret;
}

char* Generate(char* password, int number78) {
    char toShow[20]={' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
    int number1=0, number2=0;
    int number123456=0;
    size_t length = strlen(password);
    int tab[NUMBER] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    number1 = number78;
    for (int i = 0; i < NUMBER; i++) {
        number2 = number1 % NUMBER;
        number1 = number1 / NUMBER;
        if (number2 != 0 || i == 0) {
            tab[i] = number2;
            number123456++;
        }
    }
    char *toReturn;
    char *tmp;
    toReturn = (char *) calloc( (length+number123456), sizeof(char));
    for (int i = 0; i < length; i++) {
        toReturn[i] = password[i];
    }

    for (int i = 0; i < NUMBER; i++) {
        int number = tab[i];
        if (number != -1) {
            toShow[i] = addToTab[number];
        }
    }

    for (int i = 0; i < number123456; i++){
        toReturn =appendCharToCharArray(toReturn, toShow[i]);
    }

    return toReturn;

}
