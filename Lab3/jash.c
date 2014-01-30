#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <conio.h>
#include <malloc.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#define MAXLINE 1000
#define DEBUG 0
void runFile(char ** tokens);
typedef struct _cron_entry
{
	char command[100];
	int minute;
	int hour;
	int dayOfMonth;
	int month;
	int dayOfWeek;
}cron;

char cronFileName[100];
//declarations
char ** tokenize(char*);

/*  This is used to abort child processes
 *  if they are running, or else abort
 *  the parent process.
 */
int pid;
struct itimerval it;

	void abortHandler(int signal) {
	if(pid == 0) exit(0);
	else kill(pid, SIGKILL);
}

int returnValue(char* v)
{
	int val = atoi(v);
	if(val == 0 && v[0] == '*')
		return -1;
	return val;
}
int now(char** listOfTokens,time_t* timer)
{
	int minute = returnValue(listOfTokens[0]);
	int hour = returnValue(listOfTokens[1]);
	int dayOfMonth = returnValue(listOfTokens[2]);
	int month = returnValue(listOfTokens[3]);
	int dayOfWeek = returnValue(listOfTokens[4]);
	struct tm *tm_struct = 	localtime(timer);
	if((minute == -1 || minute == tm_struct->tm_min) && 
		(hour == -1 || hour == tm_struct->tm_hour ) &&
		(dayOfMonth == -1 || dayOfMonth == tm_struct->tm_mday)&&
		(month == -1 || month == tm_struct->tm_mon)&&
		(dayOfWeek == -1 || dayOfWeek == tm_struct->tm_wday))
	{
		return 0;
	}
	else
		return -1;
}
void sigalrm_handler(int sig)
{
    static unsigned count = 0;
    time_t timer;
    time(&timer);
	if(access(cronFileName, F_OK) == -1) {perror("Error"); return;}
	char buf[1000];
	FILE * fp;
	fp = fopen(cronFileName,"rw");
	while(fgets(buf, 1000, fp) != NULL) 
	{

		char **listOfTokens;
		listOfTokens = tokenize(buf);
		int rightNow = now(listOfTokens,&timer);
		if(rightNow == 0) //true
		{	
			executeCommand(listOfTokens+5);
		}
		else
		{

		}
	}

}

void executeCron(char* input)
{
	pid = fork();
	int waitid;
	if(pid == 0) 
	{

		/**
			itimerval
		**/
	    it.it_value.tv_sec     = 1;       /* starts in 1 second      */
	    it.it_value.tv_usec    = 0;
	    it.it_interval.tv_sec  = 30;      
	    it.it_interval.tv_usec = 0;
    	signal(SIGALRM, sigalrm_handler); /* Install the handler    */
    	setitimer(ITIMER_REAL, &it, NULL);/* turn on interval timer */

		char** tokens;
		tokens = tokenize(input);
		if(access(tokens[1], F_OK) == -1) {perror("Error"); return;}
		strcpy(cronFileName,tokens[1]);
	}
	pid = 0;


}

/*  This function is for running multiple
 *  commands in parallel.
 */
void executeInParallel(char * input) {
	char * inputCommands;
	inputCommands = (char *)malloc(1000*sizeof(char));
	strncpy(inputCommands, input+9, strlen(input)-9);
	// Now, the word "parallel" is removed from the string.

	char * commands;
	commands = strtok(inputCommands,":");
	// The line is split into tokens delimited by colons.

	char ** lolTokens;
	int n = 0;

	// Running the commands one by one:
	while(commands != NULL) {
		n++;
		//printf("%s\n", commands);
		lolTokens = tokenize(commands);
		//printf("%s\n", lolTokens[0]);

		if( (strcmp(lolTokens[0], "cd") == 0) || (strcmp(lolTokens[0], " cd") == 0) ) {
			if(lolTokens[1] == NULL) chdir(getenv("HOME"));
			else if (chdir(lolTokens[1]) == -1)
   			perror("Error in parallel chdir");
		}

		else if(strcmp(lolTokens[0], "run") == 0) {
			runFile(lolTokens);
		}
		else executeCommandWithoutWait(lolTokens);
		commands = strtok(NULL,":");
		
	}
	//printf("%d\n", n);
	int count = 0;
	while(count != n){
		waitpid(-1, NULL, 0);
		count++;
	}
	free(inputCommands);
}

// This function is used to execute standard commands.
int executeCommand(char ** tokens) {
	pid = fork();
	int waitid;
	if(pid == 0) {
		if(execvp(tokens[0], tokens) == -1) {
			perror("Something"); return 0;
		}
	}
	wait(&waitid);
	pid = 0;
	return 1;
}


// This function is for the parallel processes.
// The parents waits for all the children separately (line 68), not here.
int executeCommandWithoutWait(char ** tokens) {
	pid = fork();
	if(pid == 0) {
		if(execvp(tokens[0], tokens) == -1) {
			perror("Something"); return 0;
		}
	}
	pid = 0;	
	return 1;
}

//Changing directory.
void changeDirectory(char ** tokens) {
	if(tokens[1] == NULL) chdir(getenv("HOME"));
	else if (chdir(tokens[1]) == -1)
   	perror("Error");
}

// Running a batch file.
void runFile(char ** tokens) {
	if(access(tokens[1], F_OK) == -1) {perror("Error"); return;}
	char buf[1000];
	char ** listOfTokens;
	FILE * fp;
	fp = fopen(tokens[1],"rw");
	while(fgets(buf, 1000, fp) != NULL) {
		listOfTokens = tokenize(buf);
		if(strcmp(listOfTokens[0], "cd") == 0) {
			changeDirectory(listOfTokens);
		}
		else if(strcmp(listOfTokens[0], "run") == 0) runFile(listOfTokens);
		else if(executeCommand(listOfTokens) == 0) break;
	}
}


int main(int argc, char** argv){

	//Setting the signal interrupt to its default function. 
	signal(SIGINT, abortHandler);

	//Allocating space to store the previous commands.
	int numCmds = 0;
	char **cmds = (char **)malloc(1000 * sizeof(char *));

	int printDollar = 1;

	char input[MAXLINE];
	char** tokens;

	int notEOF = 1;
	int i;

	FILE* stream = stdin;

	while(notEOF) { 
		if (printDollar == 1){ 
			printf("$ "); // the prompt
			fflush(stdin);
		}

		char *in = fgets(input, MAXLINE, stream); //taking input one line at a time

		//Checking for EOF
		if (in == NULL){
			if (DEBUG) printf("EOF found\n");
			exit(0);
		}

		//add the command to the command list.
		cmds[numCmds] = (char *)malloc(sizeof(input));
		strcpy(cmds[numCmds++], input); 

		// Calling the tokenizer function on the input line    
		tokens = tokenize(input);
		// Uncomment to print tokens
	 
		/*for(i=0;tokens[i]!=NULL;i++){
			printf("%s\n", tokens[i]);
		}   */
		//printf("%d\n", strcmp(tokens[0], "cd\n"));
		/*char key = tokens[0][0];
		if (key == 65)
		{
			printf("hello\n" );
		}*/
		
		// Various cases - cd, run, parallel, and other commands.
		if(strcmp(tokens[0], "cd") == 0) {
			changeDirectory(tokens);
		}

		else if(strcmp(tokens[0], "run") == 0) {
			runFile(tokens);
		}
		else if(strcmp(tokens[0], "parallel") == 0) {
			executeInParallel(input);
		}
		else if(strcmp(tokens[0],"cron") == 0)
		{
			executeCron(input);
		}
		else {
			executeCommand(tokens);
		}
	}
  
  
	printf("Print and deallocate %s\n", tokens[0]);
	// Freeing the allocated memory	
	for(i=0;tokens[i]!=NULL;i++){
		free(tokens[i]);
	}
	free(tokens);
	return 0;
}

/*the tokenizer function takes a string of chars and forms tokens out of it*/
char ** tokenize(char* input){
	int i;
	int doubleQuotes = 0;
	
	char *token = (char *)malloc(1000*sizeof(char));
	int tokenIndex = 0;

	char **tokens;
	tokens = (char **) malloc(MAXLINE*sizeof(char**));
 
	int tokenNo = 0;
	
	for(i =0; i < strlen(input); i++){
		char readChar = input[i];
		
		if (readChar == '"'){
			doubleQuotes = (doubleQuotes + 1) % 2;
			if (doubleQuotes == 0){
				token[tokenIndex] = '\0';
				if (tokenIndex != 0){
					tokens[tokenNo] = (char*)malloc(sizeof(token));
					strcpy(tokens[tokenNo++], token);
					tokenIndex = 0; 
				}
			}
		}
		else if (doubleQuotes == 1){
			token[tokenIndex++] = readChar;
		}
		else if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
			token[tokenIndex] = '\0';
			if (tokenIndex != 0){
				tokens[tokenNo] = (char*)malloc(sizeof(token));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0; 
			}
		}
		else{
			token[tokenIndex++] = readChar;
		}
	}
	
	if (doubleQuotes == 1){
		token[tokenIndex] = '\0';
		if (tokenIndex != 0){
			tokens[tokenNo] = (char*)malloc(sizeof(token));
			strcpy(tokens[tokenNo++], token);
		}
	}
	
	return tokens;
}
