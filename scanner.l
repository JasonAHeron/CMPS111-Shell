%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int _numargs = 10;
char *_args[10];
int _argcount = 0;

/*
char *strdup (const char *s) {
    char *d = malloc (strlen (s) + 1);   
    if (d != NULL) strcpy (d,s);         
    return d;                            
}
*/

%}

WORD [a-zA-Z0-9\/\.-]+
SPECIAL [()><|&;*]

_argcount = 0;
_args[0] = NULL;

%%
{WORD}|{SPECIAL} {
	if(_argcount < _numargs-1) {
		_args[_argcount++] = (char *)strdup(yytext);
		_args[_argcount] = NULL;
	}
}

\n {return (int)_args;}
[ \t]+
.
%%
char **getline(void) { return (char **)yylex(); }