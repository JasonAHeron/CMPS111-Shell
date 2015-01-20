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
	char** args;
	pid_t pid;

	while(1) {
		printf("waiting to get a line\n");
		args = getline();
		pid = fork();
		if(pid==0){
			if (args[0]=='\0') continue;
			printf("------CHILD------\n");
			printf("ARG 0 is: %s\n",args[0]);
			cmd = which(args[0]);
			printf("command is: %s\n",cmd);
			++args;
			execv(cmd, args);
			printf("------BROKEN------\n");
		}else{
			wait(&pid);
			printf("------PARENT------\n");
			printf("child died :)\n");
			/* I want to free from the malloc but how? free(cmd);*/
		}
	}
}

/*concatinates a string*/
char* concat(char* a, char* b){
	char *c = (char *) malloc(1 + strlen(a)+ strlen(b));
    strcpy(c, a);
    strcat(c, b);
    return c;
}


/* http://stackoverflow.com/questions/19288859/how-to-redirect-stdout-to-a-string-in-ansi-c 
   http://www.tldp.org/LDP/lpg/node11.html 
   http://stackoverflow.com/questions/4812891/fork-and-pipes-in-c */
char* which(char* cmd){
	int fd[2];
	int nbytes, i, ln;
	pid_t childpid;
	char readbuffer[80]; /*why do we have a limit of 80? can we redesign to prevent it from overflowing?*/
	char *c;
	/*http://stackoverflow.com/questions/22852514/c-string-null-terminator-in-oversized-buffer*/
	for(i = 0; i<80; i++){
		readbuffer[i] = '\0'; 
	}
	pipe(fd);
	childpid = fork();
	if(childpid == -1){
		perror("fork");
        exit(1);
    }
    /*Whatever is written to fd[1] will be read from fd[0].*/
	if(childpid == 0){
		close(fd[0]); /* close pipe read, we are not using it */
		close(1); /* close std_out so we can dup it */
		dup2(fd[1], 1); /*std_out (the output of which) -> pipe write */
        execl("/usr/bin/which", "/usr/bin/which", cmd ,NULL); /* now the output is in our pipe*/
	}else{
		close(fd[1]); /* close pipe write, we are not using it */
        wait(&childpid);
        /* put the contents of fd[0] into readbuffer*/
        printf("Size of buffer is %d\n",sizeof(readbuffer));
		nbytes = read(fd[0], readbuffer, sizeof(readbuffer)); 
	}
    c = strrchr(readbuffer, '\n'); /* strip \n which is added*/
	if (c != NULL){
       *(c) = '\0'; 
	} 
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

/*

ls *.c | cat > recordings
<command> <args>
<ls> <-a -l *.c | cat > recordings>

old = ls -a -l *.c
gather all of this together 

bool pipe
bool redirect

pipe = true

new = cat

pipe == true
:: pipe old into new
to get a new old
set pipe = false
redirect = true







*/









