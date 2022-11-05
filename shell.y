
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN GREAT NEWLINE SMALL AND DGREAT PIPE ANDDGREAT EXIT ANDGREAT

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command
        ;

simple_command:	
	command_and_args iomodifier_opt NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	;

command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	|
	command_and_args PIPE command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );\
	}
	;


command_word:
	EXIT {
		_Exit(1);
	}
	|
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

iomodifier_opt:
	GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	|
	SMALL WORD {
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	|
	DGREAT WORD {
		printf("   Yacc: insert output append \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._appendOut = 1;
	}
	|
	ANDDGREAT WORD {
		printf("   Yacc: insert error and output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._errFile = $2;
		Command::_currentCommand._appendOut = 1;
		Command::_currentCommand._appendErr = 1;
	}
	|
	ANDGREAT WORD {
		printf("   Yacc: insert input and error \"%s\"\n", $2);
		Command::_currentCommand._errFile = $2;
	}
	|
	AND {
		printf(" Yacc: background task");
		Command::_currentCommand._background = 1;
	}
	|
	iomodifier_opt SMALL WORD {
		printf("   Yacc: insert input \"%s\"\n", $3);
		Command::_currentCommand._inputFile = $3;
	}
	|
	iomodifier_opt GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $3);
		Command::_currentCommand._outFile = $3;
	}
	|
	iomodifier_opt DGREAT WORD {
		printf("   Yacc: insert output append \"%s\"\n", $3);
		Command::_currentCommand._outFile = $3;
		Command::_currentCommand._appendOut = 1;
	}
	|
	iomodifier_opt AND {
		printf(" Yacc: background task");
		Command::_currentCommand._background = 1;
	}
	|
	iomodifier_opt ANDDGREAT WORD {
		printf("   Yacc: insert error and output \"%s\"\n", $3);
		Command::_currentCommand._outFile = $3;
		Command::_currentCommand._errFile = $3;
		Command::_currentCommand._appendOut = 1;
		Command::_currentCommand._appendErr = 1;
	}
	|
	iomodifier_opt ANDGREAT WORD {
		printf("   Yacc: insert error \"%s\"\n", $3);
		Command::_currentCommand._errFile = $3;
	}
	| /* can be empty */ 
	;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
