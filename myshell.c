#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

extern char **getline(void);
char *concat(char* a, char* b);
char *which(char* cmd);
int array_length(char** array);
void shell_pipe(char** LHS, char** RHS);
void shell_pipe2(char** command, int save[]);
void redirect_output(char** LHS, char* filename);
void print_array(char ** array, char* caller);
void standard_exec(char** command, int save[], int original[]);
int get_cmd_end(char** RHS_start);
void parseargs(char** args);

int main(void) {
	char** args;
	while(1) {
		printf("SEXY_SHELL# ");
		args = getline();
		/*printf("args is: %s\n\n", *args);*/
		if (args[0]=='\0'){
			continue;
		} 
		parseargs(args);
	}
}

void parseargs(char** args){
	char** LHS; 
	char** RHS;
	char* filename;
	char* char_save;
	int i;
	int start;
	int end;
	int save[2];
	int original[2];
	FILE* stream;
	int c;
	start = 0;
	i = 0;
	/*execute the first argument*/
    end = get_cmd_end(args);
    char_save = args[end];
    printf("end  here is: %d\n",end);
    args[end] = '\0';
    standard_exec(args, save, original);
    args[end] = char_save;
    i = end;
    if(char_save == '\0'){

       stream = fdopen (save[0], "r");
       while ((c = fgetc (stream)) != EOF)
          putchar (c);
       dup2(original[0], 0);
	   dup2(original[1], 1);
	   close(stream);
    }
    /*now stdout of this arg is saved into fd[1]*/
/*
bugs:
ls | cat | ls
*/
    while(args[i] != NULL){
		switch(*args[i]){
			case '|': 
               /*execute start to the pipe. save it into stdout of w/e.*/
               end = get_cmd_end(args+i+1);
               printf("end is: %d\n",end);
               char_save = args[end];
               args[end] = '\0';
               printf("args == %s\n",*(args+i+1));
               /*shell_pipe2(args+i+1, save);*/
               args[end] = char_save;
               i = end;
			break;
			case '>':
			   /*args[i] = '\0';
			   LHS = args;
			   filename = args[i+1];
			   redirect_output(LHS,filename);*/
			break;
			case '<':
			break;
			default: ++i;
			   /*standard exec*/
			break;
		}
		++i;
	}
	stream = fdopen (save[0], "r");
    while ((c = fgetc (stream)) != EOF) {
    	printf("___ val is %d of ",c);
    	if(c == '\0'){
    		printf("NULL FOUND\n");
    	}
        putchar (c);
    }
    printf("The end\n");

	dup2(original[0], 0);
	dup2(original[1], 1);

    
    /*
    	while(args[i] != NULL){
		switch(*args[i]){
			case '|':
               /*execute start to the pipe. save it into stdout of w/e.
			   args[i] = '\0';
               
			   LHS = args;
			   get_cmd_end(args+(i+1));
			   shell_pipe(LHS,RHS);
			break;
			case '>':
			   args[i] = '\0';
			   LHS = args;
			   filename = args[i+1];
			   redirect_output(LHS,filename);
			break;
			case '<':
			break;
			default:
			   /*standard exec
			break;

		}
		++i;
	}

    */


	/*LHS = args+start;
	standard_exec(LHS);*/
}

int get_cmd_end(char** cmd_start){
   int index;
   char* word;
   index = 0;
   while(cmd_start[index]!=NULL && *cmd_start[index]!='|' && *cmd_start[index]!='<' && *cmd_start[index]!='>' ){
   	/*printf("examining: %s",cmd_start[index]);*/
      ++index;
   }
   return index;
}

void standard_exec(char** command, int save[], int original[]){
	if(command[0] != '\0'){ /* I can't return if null for some reason*/
		pid_t pid;
		char* cmd;
		FILE* stream;
		int c;
		cmd = which(command[0]);
		pipe(save);
		pid = fork();
		if(pid == 0){
			close(save[0]); /* close pipe read, we are not using it */
			original[1] = dup(1);
			close(1); /* close std_out so we can dup it */
	        dup2(save[1], 1); /*std_out (the output of which) -> pipe write */
			execv(cmd, command); /*This is the difference*/
		}else{
			close(save[1]); /* close pipe write, we are not using it */
			original[0] = dup(0);
		    dup2(save[0], 0);
			wait(&pid);
		}
    }
}


/*printf("cmd is: %s\n",cmd);
printf("stdin is: %s\n",stdin);*/
void shell_pipe2(char** command, int save[]){
	char* cmd;
	pid_t pid;
	FILE* stream;
	int c;
	cmd = which(command[0]);
	pid = fork();
	if(pid == 0){
   	/*READ FROM PIPE!!*/
        stream = fdopen (save[0], "r");
        close(save[0]);
        close(1);
        dup2(save[1],1);
        /*I want to put the new output into stdout*/
        /*    while ((c = fgetc (stream)) != EOF)
               putchar (c);*/
        execv(cmd, stream);
	}else{
		wait(&pid);
		close(stream);
	}
}

/*
Given a result and a command, where the result is additional arguments for the command,
it will execute the command with the additional args (result) and it will return
the output of that call.
*/
void redirect_output(char** LHS, char* filename){
	char* cmd;
	pid_t pid;
	FILE* fp;
	fp = fopen(filename, "w+");
	print_array(LHS, "rd_out_LHS");
	cmd = which(LHS[0]);
	pid = fork();
	if (pid == 0){
		fflush(stdout);
		dup2(fileno(fp), 1);
		execv(cmd, LHS);
	}else{
		wait(&pid);
		fclose(fp);
	}
}

void shell_pipe(char** LHS, char** RHS){
	char* cmd;
	char* cmd2;
	pid_t pid;
	int fd[2];
	int stdin_save;
	int stdout_save;
	/*print_array(LHS, "pipe_LHS");
	print_array(RHS, "pipe_RHS");*/
	cmd = which(LHS[0]);
	cmd2 = which(RHS[0]);
	pipe(fd);
	pid = fork();
	if(pid == 0){
        /*write from stdout into pipe*/
        close(fd[0]); /* close pipe read, we are not using it */
		stdout_save = dup(1);
		close(1); /* close std_out so we can dup it */
		dup2(fd[1], 1); /*std_out (the output of which) -> pipe write */
		execv(cmd, LHS);
	}else{
		close(fd[1]); /* close pipe write, we are not using it */
		stdin_save = dup(0);
		dup2(fd[0], 0);
		wait(&pid);
		pid = fork();
		if(pid == 0){
   		    /*READ FROM PIPE!!*/
			execv(cmd2, stdin);
		}else{
			wait(&pid);
			dup2(stdin_save, 0);
			dup2(stdout_save, 1);
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

void print_array(char ** array, char* caller){
	int i;
	i=-1;
	printf("------ARRAY_PRINTER<%s>\n", caller);
	while(array[++i] != NULL)printf("ARRAY[%d]: %s\n", i, array[i]);
	printf("------END_ARRAY_PRINTER<%s>\n", caller);

}
/*is this assuming that the first index is not null?*/
int array_length(char** array){
	int count = 0; 
	while(array[++count] != NULL);
	return count;
}

/*http://stackoverflow.com/questions/19288859/how-to-redirect-stdout-to-a-string-in-ansi-c 
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
