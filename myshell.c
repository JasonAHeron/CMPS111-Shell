#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

extern char **getline(void);
int match(char **args, int i);

int main(void) {
	int i;
	char **args;
	while(1) {
		printf("waiting to get a line\n");
		args = getline();
		for(i = 0; args[i] != NULL; i++) {
			printf("Argument %d: %s\n", i, args[i]);
			i = match(args, i);
		}
	}
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