/*
 * Github: https://github.com/JakubBacik/OperatingSystem/tree/main/BreakPasswordMD5
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <ctype.h>
#include "openssl/md5.h"

#define NUMBEROFPRODUCER 4
#define NUMBER 20

char *addToTab = "0123456789!@#$%^&*()";
char password[1000][33];
char **dictionary;
int numberOfPasswordInFile = 0;
int numberOfDictionaryFile = 0;
char newFileFromPassword[100] = "dataMD5.txt";

void *producer(void *threadid);
void *consumer(void *threadid);
void handler(int signal_number);
void *mainThread(void *threadid);
char *MakePassword(char *password, int iteration, int letterSize, int taskid);
char *AddCharToCharArray(char *charArray, char a, int optionDirection);
char *GeneratePassword(char *password, int numberOfIteration, int optionAddChar, int letterSize);
char *GeneratePasswordFromDictionay(char *password, int letterSize);
void GetDataFromFile(char *dictionaryFile, char *passwordFile);


char* numberofBreakPass[1000];
int numberOfMD5[1000];
int number=0;
struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int nready; /*number of ready consumer*/
} nready = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0};

struct
{
    pthread_mutex_t mutex;
    int toShow;
} put = {PTHREAD_MUTEX_INITIALIZER};

int main(int argc, char **argv)
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;

    if (sigaction(SIGHUP, &act, 0) < 0)
    {
        perror("sigaction");
        exit(-1);
    }

    pthread_t tid_mainThread;
    pthread_create(&tid_mainThread, NULL, mainThread, NULL);

    pthread_join(tid_mainThread, NULL);
}
/*
 * Watek glowny
 */
void *mainThread(void *threadid){
    GetDataFromFile("words_alpha.txt", newFileFromPassword);
    pthread_t tid_producer[NUMBEROFPRODUCER], tid_consumer;
    /* create all producers and one consumer */
    for (int t = 0; t < NUMBEROFPRODUCER; t++)
    {
        printf("Main: creating thread %d\n", t);
        int rc = pthread_create(&tid_producer[t], NULL, producer, (void *)t);
        if (rc)
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    pthread_create(&tid_consumer, NULL, consumer, NULL);
    while(1){
        if(scanf("%s", &newFileFromPassword)>0){
            pthread_kill(tid_consumer, SIGHUP);
            pthread_kill(tid_producer[0], SIGHUP);
            pthread_kill(tid_producer[1], SIGHUP);
            pthread_kill(tid_producer[2], SIGHUP);
            pthread_kill(tid_producer[3], SIGHUP);
            for(int i=0; i<numberOfPasswordInFile;i++){
                free(dictionary[i]);
            }
            GetDataFromFile("words_alpha.txt", newFileFromPassword);
            for (int t = 0; t < NUMBEROFPRODUCER; t++)
            {
                printf("Main: creating thread %d\n", t);
                int rc = pthread_create(&tid_producer[t], NULL, producer, (void *)t);
                if (rc){
                    printf("ERROR; return code from pthread_create() is %d\n", rc);
                    exit(-1);
                }
            }
            pthread_create(&tid_consumer, NULL, consumer, NULL);
        }
    }
    /* wait for all producers and the consumer */
    for (int i = 0; i < NUMBEROFPRODUCER; i++)
    {
        pthread_join(tid_producer[i], NULL);
    }
    pthread_join(tid_consumer, NULL);

    pthread_mutex_destroy(&nready.mutex);
    pthread_mutex_destroy(&put.mutex);
    pthread_cond_destroy(&nready.cond);
    pthread_exit(NULL);
}

/*
 * Watek producenta
 */

void *producer(void *threadid)
{
    long taskid;
    taskid = (long)threadid;
    for (int j = 0; ; j++){
        for (int h = 0; h < numberOfDictionaryFile-1; h++){
            for (int k = 0; k < 3; k++){
                char *passToMD5 = MakePassword(dictionary[h], j, k, taskid);
                unsigned char digest[32];
                MD5(passToMD5, strlen(passToMD5), (unsigned char *)&digest);
                char convertFromHex[32];
                for (int i = 0; i < 16; i++)
                    sprintf(&convertFromHex[i * 2], "%02x", (unsigned int)digest[i]);

                for (int i = 0; i < numberOfPasswordInFile; i++)
                {
                    if (password[i][32] == '0')
                    {
                        if (strncmp(password[i], convertFromHex, 32) == 0)
                        {
                            numberOfMD5[number] = i;
                            numberofBreakPass[number]  = passToMD5;
                            number++;
                            pthread_mutex_unlock(&put.mutex);

                            pthread_mutex_lock(&nready.mutex);
                            if (nready.nready == 0)
                            {
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
    pthread_exit(NULL);
}

/*
 * Watek konsumenta
 */

void *consumer(void *threadid)
{
    while (1)
    {
        pthread_mutex_lock(&nready.mutex);

        while(nready.nready == 0) 
         pthread_cond_wait(&nready.cond, &nready.mutex);

        nready.nready--;

        printf("Cracked password is:");
        printf("%s\n", numberofBreakPass[number-1]);
        password[numberOfMD5[number-1]][32] == '1';
        
        pthread_mutex_unlock(&nready.mutex);
    }

    pthread_exit(NULL);
}
/*
 * Handler dla sygnalu SIGHUP
 */

void handler(int signal_number)
{
    printf("\n The number of broken passwords is %d\n \n", number);
    for (int i = 0; i < number - 1; i++)
    {
        printf("%s \n", numberofBreakPass[i]);
    }
}

/*
 * Funkcja odpowiedzialna za odpowiednie dobranie sekwecji generacji 
 * hasla wzgledem numeru watku.
 */

char *MakePassword(char *password, int iteration, int letterSize, int taskid)
{
    char *toReturn;
    switch (taskid)
    {
    case 0:
        if (iteration < 1)
        {
            if (letterSize == 0){
                toReturn = GeneratePasswordFromDictionay(password, 1);
            }
            else if (letterSize == 1)
                toReturn = GeneratePasswordFromDictionay(password, 2);
            else if (letterSize == 2)
                toReturn = GeneratePasswordFromDictionay(password, 3);
        }
        else
        {
            if (letterSize == 0)
                toReturn = GeneratePassword(password, iteration - 1, 1, 1);
            else if (letterSize == 1)
                toReturn = GeneratePassword(password, iteration- 1, 2, 1);
            else if (letterSize == 2)
                toReturn = GeneratePassword(password, iteration - 1, 3, 1);
        }
        break;
    case 1:
        if (letterSize == 0)
            toReturn = GeneratePassword(password, iteration, 1, 2);
        else if (letterSize == 1)
            toReturn = GeneratePassword(password, iteration, 2, 2);
        else if (letterSize == 2)
            toReturn = GeneratePassword(password, iteration, 3, 2);
        break;
    case 2:
        if (letterSize == 0)
            toReturn = GeneratePassword(password, iteration, 1, 3);
        else if (letterSize == 1)
            toReturn = GeneratePassword(password, iteration, 2, 3);
        else if (letterSize == 2)
            toReturn = GeneratePassword(password, iteration, 3, 3);
        break;
    case 3:
        if (letterSize == 0)
            toReturn = GeneratePassword(password, iteration, 4, 1);
        else if (letterSize == 1)
            toReturn = GeneratePassword(password, iteration, 4, 2);
        else if (letterSize == 2)
            toReturn = GeneratePassword(password, iteration, 4, 3);
        break;

    default:
        printf("ERROR: Not enough number of thread\n");
        break;
    }

    return toReturn;
}

/*
 * Funkcja odpowiedzialna za dodanie do ciagu znakowego char*  jednego
 * char'a z przodu lub tylu zaleznie od opcji
 */

char *AddCharToCharArray(char *charArray, char a, int optionDirection)
{
    size_t lengthOfCharArray = strlen(charArray);

    char* toReturn = malloc(sizeof(char)* (lengthOfCharArray+ 2));

    if (optionDirection == 1)
    {
        strcpy(toReturn, charArray);
        toReturn[lengthOfCharArray] = a;
        toReturn[lengthOfCharArray + 1] = '\0';
    }

    if (optionDirection == 2)
    {
        for (int i = 0; i < lengthOfCharArray + 1; i++)
        {
            toReturn[i + 1] = charArray[i];
        }
        toReturn[0] = a;
        toReturn[lengthOfCharArray + 1] = '\0';
    }

    return toReturn;
}

/*
 * Funkcja ktora genreuje hasla poprzez modyfikacje wielkosci liter oraz odpowiednim 
 * dokladaniu znakow z przodu, z tylu lub tworzenie innego wozru.
 */

char *GeneratePassword(char *passwordToGenerate, int numberOfIteration, int optionAddChar, int letterSize)
{
    char charToAdd[20] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    int moduloNumber = 0;
    int lengthOfCharToAdd = 0;
    size_t lengthOfPassword = strlen(passwordToGenerate);
    int tab[NUMBER] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    for (int i = 0; i < NUMBER; i++)
    {
        moduloNumber = numberOfIteration % NUMBER;
        numberOfIteration = numberOfIteration / NUMBER;
        if (moduloNumber != 0 || i == 0)
        {
            tab[i] = moduloNumber;
            lengthOfCharToAdd++;
        }
    }

    char *toReturn;
    toReturn = malloc(sizeof(char) * (lengthOfPassword + lengthOfCharToAdd));

    for (int i = 0; i < lengthOfPassword; i++)
    {
        if (letterSize == 1)
            toReturn[i] = passwordToGenerate[i];

        if (letterSize == 2)
        {
            toReturn[i] = passwordToGenerate[i];
            toReturn[0] = toupper(passwordToGenerate[0]);
        }

        if (letterSize == 3)
            toReturn[i] = toupper(passwordToGenerate[i]);
    }


    for (int i = 0; i < NUMBER; i++)
    {
        if (tab[i] != -1)
        {
            charToAdd[i] = addToTab[tab[i]];
        }
    }

    if (optionAddChar == 1 || optionAddChar == 2)
    {
        for (int i = 0; i < lengthOfCharToAdd; i++)
        {
            toReturn = AddCharToCharArray(toReturn, charToAdd[i], optionAddChar);
        }
    }
    if (optionAddChar == 3)
    {
        for (int i = 0; i < lengthOfCharToAdd; i++)
        {
            toReturn = AddCharToCharArray(toReturn, charToAdd[i], 1);
        }
        for (int i = 0; i < lengthOfCharToAdd; i++)
        {
            toReturn = AddCharToCharArray(toReturn, charToAdd[i], 2);
        }
    }

    if (optionAddChar == 4)
    {
        for (int i = 0; i < lengthOfCharToAdd; i++)
        {
            toReturn = AddCharToCharArray(toReturn, charToAdd[i], 1);
        }
        for (int i = 0; i < lengthOfPassword; i++)
        {
            toReturn = AddCharToCharArray(toReturn, passwordToGenerate[i], 1);
        }
    }

    return toReturn;
}

/*
 * Funkcja ktora tworzy haslo wylacznie zmieniaja wielkosc litter
 * nie doklada rzadnych znakow.
 */

char *GeneratePasswordFromDictionay(char *passwordOfDictionary, int letterSize)
{

    size_t lengthOfPassword = strlen(passwordOfDictionary);
    char *toReturn;
    
    toReturn = malloc(sizeof(char) * (lengthOfPassword + 1));

    for (int i = 0; i < lengthOfPassword; i++)
    {
        if (letterSize == 1){
            toReturn[i] =passwordOfDictionary[i];
        }

        if (letterSize == 2)
        {
            toReturn[i] = passwordOfDictionary[i];
            toReturn[0] = toupper(passwordOfDictionary[0]);
        }

        if (letterSize == 3)
            toReturn[i] = toupper(passwordOfDictionary[i]);
    }
 
    toReturn[lengthOfPassword] = '\0';
    return toReturn;
}

/*
 * Funkcja ktora sluzy do pobrania danych z slownika i 
 * pliku z haslami MD5
 */

void GetDataFromFile(char *dictionaryFile, char *passwordFile)
{
    FILE *handlerPasswordFile;
    char *bufferPasswordFile = NULL;
    ssize_t nread;
    size_t len = 0;

    if ((handlerPasswordFile = fopen(passwordFile, "r")) == NULL)
    {
        printf("Error! opening file\n");
        exit(-1);
    }

    numberOfPasswordInFile = 0;
    while (getline(&bufferPasswordFile, &len, handlerPasswordFile) != -1)
    {
        for (int i = 0; i < 32; i++)
        {
            password[numberOfPasswordInFile][i] = bufferPasswordFile[i];
        }
        password[numberOfPasswordInFile][32] = '0';
        numberOfPasswordInFile++;
    }

    fclose(handlerPasswordFile);
    /*------------------------------------------------------------*/
    /*                      DICTIONARY                            */
    /*------------------------------------------------------------*/
    FILE *handlerDictionaryFile;
    char *bufferDictionaryFile = NULL;
    ssize_t nreadDictionary;
    size_t lenDictionary = 0;
    numberOfDictionaryFile = 0;
    if ((handlerDictionaryFile = fopen(dictionaryFile, "r")) == NULL)
    {
        printf("Error! opening file\n");
        exit(-1);
    }
    char firstLine[50];
    fgets(firstLine, 50, handlerDictionaryFile);
    numberOfDictionaryFile = atoi(firstLine);

    dictionary = (char **)malloc(sizeof(char *) * numberOfDictionaryFile);

    int i = 0;
    while (nreadDictionary = getline(&bufferDictionaryFile, &lenDictionary, handlerDictionaryFile) != -1)
    {
        dictionary[i] = malloc(sizeof(char) * strlen(bufferDictionaryFile));
        strtok(bufferDictionaryFile, "\n");
        strcpy(dictionary[i], bufferDictionaryFile);
        (dictionary[i])[strlen(dictionary[i]) - 1] = '\0';
        i++;
    }
    fclose(handlerDictionaryFile);
}

