#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <ctype.h>
#include "openssl/md5.h"

# define NUMBEROFPRODUCER 4
# define NUMBER 20 

/*TESTOWE DANE*/
char dataFromFIle[1000][33];
char** tab;
int numberOfLine=0;
int numberLoop = 0;

char* slownik[10] = {"adam", "marek", "jan", "kuba", "michal", "adrian", "ola", "jacek", "mariola", "weronika"};

/*KONIEC TESTOWYCH DANYCH */



void handler (int sig);
void producer(void *);
void consumer(void *);
char* Generate(char* password, int number78, int option, int letter);
char* appendCharToCharArray(char* array, char a, int option);
char* MakePassword(char* password, int numner, int letter,  int taskid);
char* Dictionary(char* password, int letter);
void getDataFromFile(char* dictionary, char* FileWithData);
struct Data
{
    int numberOfBreakPass;
    char* BreakPass;
};

struct Data numberofBreakPass[1000];
struct{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int nready; /*number of ready consumer*/ 
} nready = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER,0}; 

struct {
    pthread_mutex_t mutex;
    int number; /* next index to store */
    int toShow;
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
    getDataFromFile("words_alpha.txt", "dataMD5.txt");

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
    long taskid;
    taskid = (long)threadid;
    for(int j=0;j<8;j++){
        for(int h=0; h<numberLoop; h++){
            for(int k=0; k<3; k++){
                char* passToMD5 = MakePassword(tab[h], j, k, taskid);
                printf("%s\n", passToMD5);
        
                unsigned char digest[32];
                MD5(passToMD5, strlen(passToMD5), (unsigned char*)&digest); 
                                
                char convertFromHex[32];
                for(int i = 0; i < 16; i++)
                    sprintf(&convertFromHex[i*2], "%02x", (unsigned int)digest[i]);

                for(int i=0; i<numberOfLine; i++){
                    if(dataFromFIle[i][32] == '0'){
                        if(strncmp(dataFromFIle[i], convertFromHex, 32)==0){
                            
                            printf("------------------SEND ---------------------------\n");

                            pthread_mutex_lock(&put.mutex);
                            numberofBreakPass[put.number].numberOfBreakPass=h;
                            numberofBreakPass[put.number].BreakPass=passToMD5;
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
        

    }
    pthread_exit (NULL);
}


void consumer(void *threadid){
    for(int i=0; i<4; i++){
        pthread_mutex_lock(&nready.mutex);
        while(nready.nready == 0)
            pthread_cond_wait(&nready.cond, &nready.mutex);
        nready.nready--;
        dataFromFIle[i][33] == '1';
        printf("Cracked password is:");
        printf("%s \n", numberofBreakPass[put.number-1].BreakPass);
        pthread_mutex_unlock(&nready.mutex);
    }
    
    pthread_exit (NULL);
}

void handler (int signal_number){
    printf ("\n The number of broken passwords is %d\n \n", put.number);
    for(int i=0; i<put.number-1; i++){
        printf("%s \n", numberofBreakPass[put.number-1].BreakPass);
    }
}


char* MakePassword(char* password, int numner, int letter, int taskid){
    char* toReturn;
    switch (taskid)
    {
    case 0:
        if(numner<1){
            if(letter == 0)
                toReturn = Dictionary(password, 1);
            else if(letter == 1)
                toReturn = Dictionary(password,2);
            else if(letter == 2)
                toReturn = Dictionary(password,3);
        }else{
            if(letter == 0)
                toReturn = Generate(password, numner-1, 1,1);
            else if(letter == 1)
                toReturn = Generate(password, numner-1, 2,1);
            else if(letter == 2)
                toReturn = Generate(password, numner-1, 3,1);
        }
        break;
    case 1:
        if(letter == 0)
            toReturn = Generate(password, numner, 2,1);
        else if(letter == 1)
            toReturn = Generate(password, numner, 2,2);
        else if(letter == 2)
            toReturn = Generate(password, numner, 2,3);
        break;
    case 2:
        if(letter == 0)
            toReturn = Generate(password, numner, 3,1);
        else if(letter == 1)
            toReturn = Generate(password, numner, 3,2);
        else if(letter == 2)
            toReturn = Generate(password, numner, 3,3);
        break;
    case 3:
        toReturn = Generate(password, numner, 4,1);
        break;
    
    default:
        printf("ERROR: Not enough number of thread");
        break;
    }

    return toReturn;

}


char *addToTab = "0123456789!@#$%^&*()";
char* appendCharToCharArray(char* array, char a, int option)
{
    size_t len = strlen(array);
    char* ret= (char *) calloc(len+2, sizeof(char));
    if (option ==1) {
        strcpy(ret, array);
        ret[len] = a;
        ret[len + 1] = '\0';
    }
    if (option ==2) {
        for(int i=0; i<len+1; i++){
            ret[i+1]=array[i];
        }
        ret[0] = a;
        ret[len + 1] = '\0';
    }

    return ret;
}


char* Generate(char* password, int number78, int option, int letter) {
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
        if(letter == 1 )
            toReturn[i] = password[i];

        if(letter == 2){
            toReturn[i] = password[i];
            toReturn[0] = toupper(password[0]); 
        }

        if(letter == 3)
            toReturn[i] = toupper(password[i]);
    }

    for (int i = 0; i < NUMBER; i++) {
        int number = tab[i];
        if (number != -1) {
            toShow[i] = addToTab[number];
        }
    }
    if(option == 1 || option == 2) {
        for (int i = 0; i < number123456; i++) {
            toReturn = appendCharToCharArray(toReturn, toShow[i], option);
        }
    }
    if(option ==3 ){
        for (int i = 0; i < number123456; i++) {
            toReturn = appendCharToCharArray(toReturn, toShow[i], 1);
        }
        for (int i = 0; i < number123456; i++) {
            toReturn = appendCharToCharArray(toReturn, toShow[i], 2);
        }
    }

    if(option ==4 ) {
        for (int i = 0; i < number123456; i++) {
            toReturn = appendCharToCharArray(toReturn, toShow[i], 1);
        }
        for (int i = 0; i < length; i++) {
            toReturn = appendCharToCharArray(toReturn, password[i], 1);
        }


    }


        return toReturn;

}

char* Dictionary(char* password, int letter) {
    size_t length = strlen(password);
    char *toReturn;
    char *tmp;
    toReturn = (char *) calloc( length+1, sizeof(char));
    for (int i = 0; i < length; i++) {
        if(letter == 1 )
            toReturn[i] = password[i];

        if(letter == 2){
            toReturn[i] = password[i];
            toReturn[0] = toupper(password[0]); 
        }

        if(letter == 3)
            toReturn[i] = toupper(password[i]);
    }
    toReturn[length+1] = '\0';
    tmp =toReturn;
    free(toReturn);
    return tmp;

}

void getDataFromFile(char* dictionary, char* FileWithData){
  FILE *handler;
  char *buffer = NULL;
  ssize_t nread;
  size_t len = 0;
  
  if((handler = fopen(FileWithData, "r")) == NULL){
    printf("Error! opening file\n");
    exit(-1);
  }
  
  numberOfLine=0;
  while ((nread = getline(&buffer, &len, handler)) != -1) {
    for(int i=0; i<32; i++){
      dataFromFIle[numberOfLine][i]=buffer[i];
    }
    dataFromFIle[numberOfLine][32]='0';
    numberOfLine++;
  }

  fclose(handler);
/*------------------------------------------------------------*/
  FILE *handlerDictionary;
  char *bufferDictionary = NULL;
  ssize_t nreadDictionary;
  size_t lenDictionary = 0;
  numberLoop = 0;
    if((handlerDictionary = fopen(dictionary, "r")) == NULL){
    printf("Error! opening file\n");
    exit(-1);
  }
  char buff[50];
  fgets( buff, 50, handlerDictionary);
  numberLoop = atoi(buff);

  tab = (char**)malloc(sizeof(char *) * numberLoop);
  int number =0;
  while(nreadDictionary = getline(&bufferDictionary, &lenDictionary, handlerDictionary)!= -1) {
    tab[number]=malloc ( strlen(bufferDictionary));
    strtok(bufferDictionary, "\n");
    strcpy( tab[number], bufferDictionary);
    (tab[number])[strlen(tab[number])-1]='\0';
    number++;
  }
  fclose(handlerDictionary);

}

