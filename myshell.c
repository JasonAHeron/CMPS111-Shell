#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

extern char **getline(void);
int match(char **args, int i);
static char* path = "/bin/";
char *concat(char* a, char* b);

int main(void) {
	int i;
	char *cmd;
	char **args;
	char **argptr;
	pid_t pid;
	while(1) {
		printf("waiting to get a line\n");
		args = getline();
		printf("Argument %d: %s\n", i, args[i]);
		pid = fork();
		if(pid==0){
			printf("child born. ARG 0 IS: %s\n",args[0]);
			cmd = concat(path, args[0]);
			argptr = args + 1;
			printf("argptr is %s\n", argptr[0]);
			execl(cmd, cmd, argptr[0], NULL);
		}
		else{
			wait(&pid);
			printf("child died\n");
		}
	}
}

char *concat(char* a, char* b){
	char *c = (char *) malloc(1 + strlen(a)+ strlen(b));
    strcpy(c, a);
    strcat(c, b);
    return c;
}

int match(char **args, int i) {
	char *current = args[i];
	printf("in match\n");
	if(!strcmp(current,"exit")){
		printf("found match against exit\n");
		exit(0);
	}
	else {
	pid_t pid = fork();
	if(pid==0){
		printf("running: %s\n",current);
		/*
		char str[100];
		str = strcat(str, "/bin/");
		str = strcat(str, current);
		*/
		execlp(current, current, args);
	}
	else{
		waitpid(pid,0,0);
		printf("DONE WAITING, READY TO GO!\n");
	}
	return i;
	}
}