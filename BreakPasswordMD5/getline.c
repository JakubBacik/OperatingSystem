#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){
  char dataFromFIle[1000][33];
  FILE *handler;
  char *buffer = NULL;
  ssize_t nread;
  size_t len = 0;
  

  if((handler = fopen(argv[1], "r")) == NULL){
    printf("Error! opening file\n");
    exit(-1);
  }

  int numberOfLine=0;
  while ((nread = getline(&buffer, &len, handler)) != -1) {
    for(int i=0; i<32; i++){
      dataFromFIle[numberOfLine][i]=buffer[i];
    }
    numberOfLine++;
  }

  fclose(handler);
/*------------------------------------------------------------*/
  FILE *handlerDictionary;
  char *bufferDictionary = NULL;
  ssize_t nreadDictionary;
  size_t lenDictionary = 0;
    if((handlerDictionary = fopen(argv[2], "r")) == NULL){
    printf("Error! opening file\n");
    exit(-1);
  }
  char buff[50];
  fgets( buff, 50, handlerDictionary);
  int numberLoop = atoi(buff);

  char** tab;
  tab = (char**)malloc(sizeof(char *) * numberLoop);
  int number =0;
  while(nreadDictionary = getline(&bufferDictionary, &lenDictionary, handlerDictionary)!= -1) {
    tab[number]=malloc ( strlen(bufferDictionary));
    strtok(bufferDictionary, "\n");
    strcpy( tab[number], bufferDictionary);
    number++;
  }
  for(int i=0; i<numberLoop; i++){
    printf("%s\n", tab[i]);
  }
  fclose(handlerDictionary);

  

  return 0;
}
