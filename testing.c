#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

char** concat(char** a, char* b);
int array_length(char** array);


int main(int argc, char** argv){
   char* text = "hello";
   int count = 0;
   //print args first
   while(argv[count]!=NULL){
   	  printf("argv[%d] = %s\n", count, argv[count]);
      ++count;
   }
   count = 0;
   printf("NEW ARRAY:\n");
   char** newarr = concat(argv, text);
   while(newarr[count]!=NULL){
   	  printf("argv[%d] = %s\n", count, newarr[count]);
      ++count;
   }
   return 0;
}

char** concat(char** a, char* b){
	int len;
	len = array_length(a);
	char** newargs = (char **) malloc(1+1+len);
	for(int i = 0; i < len; i++){
		newargs[i] = (char*) malloc((int)strlen(a[i])+1);
		newargs[i] = a[i];
	}
	newargs[len] = (char*) malloc((int)strlen(b)+1);
	newargs[len] = b;
	newargs[len+1] = '\0';
	return newargs;
}

int array_length(char** array){
	int count = 0; 
    while(array[++count] != NULL);
    return count;
}