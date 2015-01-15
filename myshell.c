#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

extern char **getline(void);
int match(char **args, int i);
static char* path = "/bin/";
char *concat(char* a, char* b);
char *which(char* cmd);

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
		printf("###GOT: %s###\n", which(args[0])); /*just for testing */
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

char *which(char* cmd){
	/* http://stackoverflow.com/questions/19288859/how-to-redirect-stdout-to-a-string-in-ansi-c */
	/* http://www.tldp.org/LDP/lpg/node11.html */
	int fd[2];
	int nbytes;
	pid_t childpid;
	char readbuffer[80];

	pipe(fd);
	if((childpid = fork()) == -1){
		perror("fork");
        exit(1);
    }
	if(childpid == 0){
		close(fd[0]); /* close pipe read */
		close(1); /* close std_out */
		dup2(fd[1], 1); /*std_out -> pipe write */
        execl("/usr/bin/which", "/usr/bin/which", cmd ,NULL);
	}else{
		close(fd[1]); /* close pipe write */
		nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
        printf("\nReceived string: %s\n", readbuffer);
	}
	return readbuffer;
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


