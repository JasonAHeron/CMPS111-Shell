#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
extern char **getline(void);

int main(void) {
	int i;
	char **args;
	while(1) {
		args = getline();
		for(i = 0; args[i] != NULL; i++) {
			printf("Argument %d: %s\n", i, args[i]);
			/*
			yyin = args[i]
			while(1) {
				x = yylex();
				if(x == YYEOF){
					break;
				}
				printf("%s", x)
			}
			*/
		}
	}
}