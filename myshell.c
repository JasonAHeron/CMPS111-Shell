#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

extern char **getline(void);
char *concat(char* a, char* b);
char *which(char* cmd);

int main(void) {
	int i;
	char *cmd;
	char** args, argptr;
	pid_t pid;

	while(1) {
		printf("waiting to get a line!!!!\n");
		args = getline();
		pid = fork();
		if(pid==0){
			/*if (args[0]=='\0') continue;*/
			printf("------CHILD------\n");
			printf("ARG 0 is: %s\n",args[0]);
			cmd = which(args[0]);
			printf("command is: %s\n",cmd);
			/*argptr = args + 1;*/
			printf("argptr is: %s\n", args[1]);
			execl(cmd, cmd, args[1], NULL);
			printf("------BROKEN------\n");
		}else{
			wait(&pid);
			printf("------PARENT------\n");
			printf("child died :)\n");
		}
	}
}

/*concatinates a string, because C is that dumb*/
char* concat(char* a, char* b){
	char *c = (char *) malloc(1 + strlen(a)+ strlen(b));
    strcpy(c, a);
    strcat(c, b);
    return c;
}


/* http://stackoverflow.com/questions/19288859/how-to-redirect-stdout-to-a-string-in-ansi-c */
/* http://www.tldp.org/LDP/lpg/node11.html */
char* which(char* cmd){
	int fd[2];
	int nbytes, i, ln;
	pid_t childpid;
	char readbuffer[80];
	char *c;
	for(i = 0; i<80; i++){
		readbuffer[i] = '\0';  /*because C is stupid*/
	}
	pipe(fd);
	if((childpid = fork()) == -1){
		perror("fork");
        exit(1);
    }
	if(childpid == 0){
		printf("Brit Debug. 1 \n");
		close(fd[0]); /* close pipe read */
		close(1); /* close std_out */
		dup2(fd[1], 1); /*std_out -> pipe write */
        execl("/usr/bin/which", "/usr/bin/which", cmd ,NULL);
	}else{
		printf("Brit Debug. 2 \n");
		close(fd[1]); /* close pipe write */
		nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
        /*printf("\nReceived string: %s\n", readbuffer);*/
        wait(&childpid);
	}
	/* c = strrchr(readbuffer, '\n'); strip \n which is added*/
	ln = strlen(readbuffer) - 1; /*strip \n which is added*/
    if (readbuffer[ln] == '\n') {
       readbuffer[ln] = '\0';
    }

	if (c != NULL) *(c) = '\0';
	return concat("",readbuffer);
}

/*
void redirect_output(char* location){
	 https://www.cs.rutgers.edu/~pxk/416/notes/c-tutorials/dup2.html 
	int newfd;
	if ((newfd = open(location, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
		perror(location);	
		exit(1);
	}

} */


