#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

extern char **getline(void);
char *concat(char* a, char* b);
char *which(char* cmd);
char* f_pipe(char* result, char** RHS);
char** concat_array(char** a, char* b);
int array_length(char** array);
void free_chararray(char** array);
char* exec_to_string(char** LHS);
void print_array(char ** array);
void working_pipe(char** LHS, char** RHS);

int main(void) {
	char** args;
	while(1) {
		printf("SEXY_SHELL#");
		args = getline();
		if (args[0]=='\0') continue; 
		parseargs(args);
	}
}

void parseargs(char** args){
	char** LHS;
	char** RHS;
	char* cmd;
	pid_t pid;
	char* result;
	int special_index, i;
	int first;
	special_index = -1;
	i = 0;
    first = 1; /*true*/

	printf("PARSING:\n");
	print_array(args);
	while(args[i] != NULL){
		switch(*args[i]){
			case '|':
			if(first) {
				args[i] = '\0';
				LHS = args;
				RHS = args+(i+1);
				working_pipe(LHS,RHS);
			/* I want to free from the malloc but how? free(cmd);*/
			}
			
			
			printf("FOUND PIPE!!! \n");
			break;
			case '>':
			printf("FOUND redirect out!!! \n");
			break;
			case '<':
			printf("FOUND redirect in !! \n");
			break;
			default: break;

		}
		++i;
	}
	/*pid = fork();*/
    /*special case for 1st LHS argument, from then on always only exec RHS*/
    /*itor through the array of arguments until we find a special character
      check to see if we have found a special character already
      we want to save the left half of the thing into Left. 
      also save the index of the special character */


      /*collapse here. we have LHS, we have old special character, we have RHS up to new special character
        call a function to execute this simple redirect or pipe. store the result*/
      /*swtich (special):
      pipe: result = f_pipe(LHS,RHS)
      red_in: result = f_red_in(LHS,RHS)
      red_out: result = f_red_out(LHS,RHS)*/
/*
	if(pid==0){
			printf("------CHILD------\n");
			printf("ARG 0 is: %s\n",args[0]);
			cmd = which(args[0]);
			printf("command is: %s\n",cmd);
			execv(cmd, args);
			printf("------BROKEN------\n");
	}else{
			wait(&pid);
			printf("------PARENT------\n");
			printf("child died :)\n");
			/* I want to free from the malloc but how? free(cmd);
	}*/
}

/*
Given a result and a command, where the result is additional arguments for the command,
it will execute the command with the additional args (result) and it will return
the output of that call.
*/
void working_pipe(char** LHS, char** RHS){
	char* cmd;
	char* cmd2;
	pid_t pid;
	int fd[2];
	printf("--LHS--\n");
	print_array(LHS);
	printf("--RHS--\n");
	print_array(RHS);
	cmd = which(LHS[0]);
	cmd2 = which(RHS[0]);
	pipe(fd);
	pid = fork();
	if(pid == 0){
        /*write from stdout into pipe*/
        close(fd[0]); /* close pipe read, we are not using it */
		close(1); /* close std_out so we can dup it */
		dup2(fd[1], 1); /*std_out (the output of which) -> pipe write */
		execv(cmd, LHS);
	}else{
		close(fd[1]); /* close pipe write, we are not using it */
		dup2(fd[0], 0);
		wait(&pid);
		pid = fork();
		if(pid == 0){
   		/*READ FROM PIPE!!*/
			execv(cmd2, stdin);
		}else{
			wait(&pid);
		}
	}
}



char* f_pipe(char* result, char** RHS){
	int i;
	char* cmd;
	char** full_args;
	pid_t pid;
	int fd[2];
	char readbuffer[1024];
	i=0;
	print_array(RHS);
	printf("%s", RHS[1]);
	cmd = which(RHS[1]);
	++RHS;
	full_args = concat_array(RHS, result);
	print_array(full_args);
	pipe(fd);
	pid = fork();
	if(pid == 0){
        /*write from stdout into pipe*/
        close(fd[0]); /* close pipe read, we are not using it */
		close(1); /* close std_out so we can dup it */
		dup2(fd[1], 1); /*std_out (the output of which) -> pipe write */
		execv(cmd, full_args);
	}else{
		close(fd[1]); /* close pipe write, we are not using it */
		wait(&pid);
   		/*READ FROM PIPE!!*/
        /* put the contents of fd[0] into readbuffer*/
		read(fd[0], readbuffer, sizeof(readbuffer)); 
	}
	return readbuffer;
}


/*this needs to be exec to file!*/
char* exec_to_string(char** LHS){
	char* cmd;
	pid_t pid;
	int fd[2];
	char readbuffer[1024];
	cmd = which(LHS[0]);
	pipe(fd);
	pid = fork();
	if(pid == 0){
        /*write from stdout into pipe*/
        close(fd[0]); /* close pipe read, we are not using it */
		close(1); /* close std_out so we can dup it */
		dup2(fd[1], 1); /*std_out (the output of which) -> pipe write */
		execv(cmd, LHS);
	}else{
		close(fd[1]); /* close pipe write, we are not using it */
		wait(&pid);
   		/*READ FROM PIPE!!*/
        /* put the contents of fd[0] into readbuffer*/
		read(fd[0], readbuffer, sizeof(readbuffer)); 
	}
	return readbuffer;
}

/*concatinates a string*/
char* concat(char* a, char* b){
	char *c = (char *) malloc(1 + strlen(a)+ strlen(b));
	strcpy(c, a);
	strcat(c, b);
	return c;
}

char** concat_array(char** a, char* b){
	int len, i;
	char** newargs;
	len = array_length(a);
	newargs = (char **) malloc(1+1+len);
	for(i = 0; i < len; i++){
		newargs[i] = (char*) malloc((int)strlen(a[i])+1);
		newargs[i] = a[i];
	}
	newargs[len] = (char*) malloc((int)strlen(b)+1);
	newargs[len] = b;
	newargs[len+1] = '\0';
	return newargs;
}

void free_chararray(char** array){

}

void print_array(char ** array){
	int i;
	i=-1;
	printf("------ARRAY_PRINTER------\n");
	while(array[++i] != NULL)printf("ARRAY[%d]: %s\n", i, array[i]);
	printf("------ARRAY_PRINTER------\n");
}

/*is this assuming that the first index is not null?*/
int array_length(char** array){
	int count = 0; 
	while(array[++count] != NULL);
	return count;
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
   		/* printf("Size of buffer is %d\n",sizeof(readbuffer)); */
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









