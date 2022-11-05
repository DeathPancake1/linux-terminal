
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>

#include "command.h"


char homeDir[1024];
SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}
	if( _outFile == _errFile){
		free( _errFile );
	}
	else{
		if ( _errFile ) {
			free( _errFile );
		}
		if ( _outFile ) {
			free( _outFile );
		}
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_appendOut = 0;
	_appendErr = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

void
Command::openPipe(int i, int defaultin, int defaultout, int defaulterr, int fdpipe[][2], int outfd, int errfd)
{
	if(i != 0){
		dup2( fdpipe[i-1][0], 0);
	}
	else{
		if(_inputFile==0){
			dup2( defaultin, 0 );
		}
		dup2( defaulterr, 2 );
	}
	if(i != _numberOfSimpleCommands-1){	
		dup2( fdpipe[i][ 1 ], 1 );
		dup2( defaulterr, 2 );
	}
	else{
		if(_outFile != 0){
			if(_appendOut){
				outfd = open( _outFile, O_WRONLY | O_APPEND | O_CREAT, 0666);
			}
			else{
				outfd = creat( _outFile, 0666 );
			}
			if ( outfd < 0 ) {
				perror( "ls : create outfile" );
				exit( 2 );
			}
			dup2( outfd, 1 );
			close( outfd );
		}
		else{
			dup2( defaultout ,1);
		}
		if(_errFile != 0){
			if(_appendErr){
				errfd = open( _errFile, O_WRONLY | O_APPEND | O_CREAT, 0666);
			}
			else{
				errfd = open( _errFile, O_WRONLY | O_CREAT, 0666);
			}
			if ( errfd < 0 ) {
				perror( "ls : create errfile" );
				exit( 4 );
			}
			dup2( errfd, 2 );
			close( errfd );
		}
		else{
			dup2( defaulterr, 2 );
		}
	}
	
}

void
Command::execute()
{
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}

	// Print contents of Command data structure
	print();

	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec
	int defaultin = dup( 0 );
	int defaultout = dup( 1 );
	int defaulterr = dup( 2 );
	int outfd;
	int infd;
	int errfd;
	if(_inputFile != 0){
		infd = open( _inputFile, O_RDONLY | O_CREAT, 0777);
		if ( infd < 0 ) {
			perror( "ls : create infile" );
			exit( 3 );
		}
		dup2( infd, 0 );
		close( infd );
	}
	int fdpipe[_numberOfSimpleCommands-1][2];
	for(int i=0 ; i< _numberOfSimpleCommands-1; i++){
		if ( pipe(fdpipe[i]) == -1) {
			perror( "cat_grep: pipe");
			exit( 2 );
		}
	}
	if(strcmp(_simpleCommands[0]->_arguments[0],"cd")==0){
		if(_simpleCommands[0]->_arguments[1]){
			chdir(_simpleCommands[0]->_arguments[1]);
		}else{
			chdir(homeDir);
		}
		
	}
	else{
		for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
			char s[1024];
			openPipe(i,defaultin,defaultout,defaulterr,fdpipe,outfd,errfd);
			int pid = fork();
			if ( pid == -1 ) {
				perror( "new process: fork\n");
				exit( 2 );
			}
			if (pid == 0) {
				//Child
				
				// close file descriptors that are not needed
				for(int j=0 ; j< _numberOfSimpleCommands-1; j++){
					close(fdpipe[j][0]);
					close(fdpipe[j][1]);
				}
				close( defaultin );
				close( defaultout );
				close( defaulterr );
				// You can use execvp() instead if the arguments are stored in an array
				
				execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
				// exec() is not suppose to return, something went wrong
				perror( ": exec ");
				exit( 2 );
			}
			if(i == _numberOfSimpleCommands -1){
				dup2( defaultin, 0 );
				dup2( defaultout, 1 );
				dup2( defaulterr, 2 );
				for(int j=0 ; j< _numberOfSimpleCommands-1; j++){
					close(fdpipe[j][0]);
					close(fdpipe[j][1]);
				}
				close( defaultin );
				close( defaultout );
				close( defaulterr );
				if(_background==0){
					waitpid( pid, NULL, 0 );
				}
			}
		}
	}
	
	// Clear to prepare for next command
	clear();
	
	// Print new prompt
	prompt();
}

// Shell implementation
void sigintHandler(int sig_num)
{
    signal(SIGINT, sigintHandler);
    printf("\n Cannot be terminated using Ctrl+C \n");
    fflush(stdout);
	printf("myshell>");
	fflush(stdout);
}
void proc_exit(int sig_num){
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	char * new_str ;
	if((new_str =(char*) malloc(sizeof(char)*strlen(homeDir)+strlen("/proc.log")+1)) != NULL){
		new_str[0] = '\0';   // ensures the memory is an empty string
		strcat(new_str,homeDir);
		strcat(new_str,"/proc.log");
	}
	FILE* f=fopen(new_str,"a");
	fprintf(f,"Shell: Process terminated, time:%s",asctime(timeinfo));
	fclose(f);
}
void
Command::prompt()
{
	printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int 
main()
{
	getcwd(homeDir, 1024);
	signal(SIGINT, sigintHandler);
	signal (SIGCHLD, proc_exit);
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}

