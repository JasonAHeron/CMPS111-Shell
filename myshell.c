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
void redirect_input(char** LHS, char* filename);
void parseargs(char** args);
int begin_first_cmd(char** args);
int prefix_strcmp(char* arg, char* match);
void strict_exec(char** args);

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
	int execute_first_flag;
	FILE* stream;
	int c;
	char q;
	start = 0;
	i = 0;
	execute_first_flag = begin_first_cmd(args);
	/*execute the first argument*/
	/*strip \n which is added*/
	if(execute_first_flag){ 
       end = get_cmd_end(args);
       char_save = args[end];
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
    }

    /*now stdout of this arg is saved into fd[1]*/
/*

test cases:
ls | wc > wc.txt

pass:
ls | wc
ls > ls.txt
cat < ls.txt
ls 
ls | cat | wc

bugs:
ls | cat | ls

resolved bugs:
ls | cat | wc
*/ 

    while(args[i] != NULL){
		switch(*args[i]){
			case '|': 
               /*execute start to the pipe. save it into stdout of w/e.*/
               end = get_cmd_end(args+i+1) + i+1;
               char_save = args[end];
               args[end] = '\0';
               /*printf("args == %s\n",*(args+i+1));*/
               shell_pipe2(args+i+1, save);
               args[end] = char_save;
               i = end;
			break;
			case '>':
			   args[i] = '\0';
			   LHS = args;
			   filename = args[i+1];
			   redirect_output(LHS,filename);
			break;
			case '<':
			   args[i] = '\0';
			   LHS = args;
			   filename = args[i+1];
			   redirect_input(LHS,filename);
			break;
			default: 
               /*manually search for cd, exit, pwd. do an exec without a fork*/
			   if(prefix_strcmp(args[0], "cd")){
			   	  strict_exec(args);
			   }else if(prefix_strcmp(args[0], "pwd")){
			   	  strict_exec(args);
			   }else if(prefix_strcmp(args[0], "exit")){
			   	  strict_exec(args);
			   }
			   ++i;
			   break;
		}
	}

	if(execute_first_flag){
	   stream = fdopen (save[0], "r");
       while ((c = fgetc (stream)) != EOF) {
           putchar (c);
       }
    }
    dup2(original[0], 0);
	dup2(original[1], 1);
}

void strict_exec(char** args){
	char* cmd;
	cmd = which(args[0]);
    execv(cmd, args);
}

/*
Function takes in 2 character arrays called arg and match.
It checks to see if the prefix of arg is equal to match.
If there is a match, it will return 1.
*/
int prefix_strcmp(char* arg, char* match){
   int i;
   i = 0;
   while(arg[i]!='\0' && match[i]!='\0'){
      if(arg[i]!=match[i]){
      	  return 0;
      }
      ++i;
   }
   /*check to see if we have reached the end of match. If yes, then we know that
     the prefixs match*/
   if(match[i]=='\0'){
   	  return 1;
   }
}

int begin_first_cmd(char** args){
   int end;
   end = get_cmd_end(args);
   if(prefix_strcmp(args[0],"cd")||prefix_strcmp(args[0],"pwd")||prefix_strcmp(args[0],"exit")){
   	  return 0;
   }
   if(args[end] == NULL){
   	  return 1;
   }else if(strcmp(args[end],"|") == 0){
   	  return 1;
   }
   return 0;
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
			close(0);
		    dup2(save[0], 0);
			wait(&pid);
		}
    }
}


/*This function reads from the stdin to get the arguments*/
void shell_pipe2(char** command, int save[]){
	char* cmd;
	pid_t pid;
	FILE* stream;
	int save0;
	int save1;
	int fd[2];
	cmd = which(command[0]);
	pipe(fd);
	pid = fork();
	if(pid == 0){
   	/*READ FROM PIPE!!*/
		close(fd[0]);
        close(1);
        dup2(fd[1],1);
        execv(cmd, command); 
	}else{
		close(fd[1]);
		close(0);
		dup2(fd[0],0);
		wait(&pid);
		save[0] = fd[0];
		save[1] = fd[1];
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

/*
	char* cmd;
	pid_t pid;
	FILE* fp;
	printf("FILENAME IS : %s\n", filename);
	fp = fdopen (filename, "r");
	print_array(LHS, "rd_out_LHS");
	cmd = which(LHS[0]);
	pid = fork();
	if (pid == 0){
		fflush(stdin);
		dup2(fileno(fp), stdin);
		execv(cmd, stdin);
	}else{
		wait(&pid);
		fclose(fp);
	}*/

void redirect_input(char** LHS, char* filename){
	char* cmd;
	pid_t pid;
	FILE* fp;
	fp = fopen(filename, "rw+");
	cmd = which(LHS[0]);
	pid = fork();
	if (pid == 0){
		fflush(stdin);
		dup2(fileno(fp), 0);
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
