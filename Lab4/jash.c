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
#include <fcntl.h>

#define MAXLINE 1000
#define DEBUG 0

// Prototype to run file.
void runFile(char ** tokens);

// The struct for use with cron command.
typedef struct _cron_entry
{
	char command[100];
	int minute;
	int hour;
	int dayOfMonth;
	int month;
	int dayOfWeek;
}cron;

char cronFileName[100]; // For maintaining a cron file.
char ** tokenize(char*);

// Prototype to check whether a function is a background or not.
int isValidBackground(char* input);

int pid;
struct itimerval it;

// The interrupt handler for background processes.

void  INThandler(int sig)
{
  	pid_t pid;
  	pid = wait(NULL);
	printf("PID %d killed\n",pid);
}

// The interrupt handler for parent processes.

void  InterruptHandler(int sig)
{
  	//pid_t pid;
  	//pid = wait(NULL);
	//printf("PID %d killed\n",pid);
	//kill(pid,SIGTERM);
	//raise();
	//return ;

}

// The interrupt handler for foreground child processes.

void  NormalHandler(int sig)
{

}

// To execute processes in the background.

int executeInBackGround(char * input) {
	signal(SIGTERM,InterruptHandler);	
	char** tokens;
	tokens = tokenize(input);
	pid = fork();
	int waitid;
	signal(SIGCHLD, INThandler);
	if(pid == 0) {
		setpgid(0,0);
		if(execvp(tokens[0], tokens) == -1) {
			perror("Error in executeCommand"); return 0;
		}
	}
	//signal(SIGINT,InterruptHandler);
	pid = 0;
	return 1;
}

// For cron command.

int returnValue(char* v)
{
	int val = atoi(v);
	if(val == 0 && v[0] == '*')
		return -1;
	return val;
}

// For cron - checks whether the current time matches the time specified for cron.
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

// For cron command - Signal alarm handler.
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

// Executes the cron command given an input (schedules).

void executeCron(char* input)
{
	pid = fork();
	int waitid;
	signal(SIGCHLD, NormalHandler);
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

// Runs the piped processes. Uses child and grandchild process.

void executePiped(char ** tokens) {
	int i, pid1, pid2, status1, status2;
	int fd[2];
	
	signal(SIGCHLD, NormalHandler);
	for(i = 0; tokens[i] != NULL; i++) {
		if(strcmp(tokens[i], "|") == 0) break;
	}

	//printf("%d\n", i);

	tokens[i] = NULL;

	// Now tokens[i] is '\0', so it can be passed to execvp as follows.

	if(pid1 = fork() == 0) {
		pipe(fd);
		
		if(pid2 = fork() == 0) {
			dup2(fd[1], STDOUT_FILENO);
			close(fd[0]);
			

			if(execvp(tokens[0], tokens) == -1) {
				perror("Error in 1st command");
			}
		}
		wait(&status1);
		dup2(fd[0], STDIN_FILENO);
		close(fd[1]);
		if(execvp(tokens[i+1], &tokens[i+1])== -1) {
			perror("Error in 2nd command");
		}

		// close(fd[0]);
		// close(fd[1]);
		
	}
	wait(&status2);

	pid = 0;
}

// To find the input and output files from input tokens. Two versions - for overwrite and append.

void findInOutFiles(char ** tokens, char * inputFile, char * outputFile) {
	int i;
	for(i = 0; tokens[i] != NULL; i++) {
		if(strcmp(tokens[i], ">") == 0) strcpy(outputFile, tokens[i+1]);
		if(strcmp(tokens[i], "<") == 0) strcpy(inputFile, tokens[i+1]);
	}
	// printf("%s %s\n", inputFile, outputFile);
}

void findInOutFilesAppend(char ** tokens, char * inputFile, char * outputFile) {
	int i;
	for(i = 0; tokens[i] != NULL; i++) {
		if(strcmp(tokens[i], ">>") == 0) strcpy(outputFile, tokens[i+1]);
		if(strcmp(tokens[i], "<") == 0) strcpy(inputFile, tokens[i+1]);
	}
	// printf("%s %s\n", inputFile, outputFile);
}

// The execution of the I/O redirection. Two versions - for overwrite and append.

void executeRedirected(char * input, char * inputFile, char * outputFile) {
	char * command = (char *) malloc(1000*(sizeof(char)));
	int waitid;
	// This is the command whose I/O will be redirected.
	command = strtok(input,"<>");
	char ** listOfTokens;
	listOfTokens = tokenize(command);
	// printf("%s\n", command);
	pid = fork();
	signal(SIGCHLD, NormalHandler);
	int out, in;

	in = open(inputFile,O_RDONLY);

	if(strcmp(outputFile,"") != 0) {
		out = open(outputFile,O_CREAT|O_TRUNC|O_WRONLY,0644);
		if(out<0)
		{
			perror("Error: rfesdv");
			exit(0);
		}
	}
	if(pid == 0)
	{
		dup2(in,0);
		if(strcmp(outputFile,"") != 0) {
			dup2(out,1);
		}
		if(execvp(listOfTokens[0], listOfTokens) == -1) {
			perror("Error in execvp");
		}
		if(strcmp(outputFile,"") != 0) {
			close(out);
		}
		close(in);
	}
	wait(&waitid);
	pid = 0;

}

void executeRedirectedAppend(char * input, char * inputFile, char * outputFile) {
	char * command = (char *) malloc(1000*(sizeof(char)));
	int waitid;
	// This is the command whose I/O will be redirected.
	command = strtok(input,"<>");
	char ** listOfTokens;
	listOfTokens = tokenize(command);
	// printf("%s\n", command);
	pid = fork();
	signal(SIGCHLD, NormalHandler);
	int out, in;

	in = open(inputFile,O_RDONLY);

	if(strcmp(outputFile,"") != 0) {
		out = open(outputFile,O_CREAT|O_APPEND|O_WRONLY,0644);
		if(out<0)
		{
			perror("Error: could not open output file.");
			exit(0);
		}
	}
	if(pid == 0)
	{
		dup2(in,0);
		if(strcmp(outputFile,"") != 0) {
			dup2(out,1);
		}
		if(execvp(listOfTokens[0], listOfTokens) == -1) {
			perror("Error in execvp");
		}
		if(strcmp(outputFile,"") != 0) {
			close(out);
		}
		close(in);
	}
	wait(&waitid);
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
	signal(SIGCHLD, NormalHandler);
	if(pid == 0) {
		if(execvp(tokens[0], tokens) == -1) {
			perror("Error in executeCommand"); return 0;
		}
	}
	wait(&waitid);
	pid = 0;
	return 1;
}


// This function is for the parallel processes.

int executeCommandWithoutWait(char ** tokens) {
	pid = fork();
	signal(SIGCHLD, NormalHandler);
	if(pid == 0) {
		if(execvp(tokens[0], tokens) == -1) {
			perror("Error in executeCommandWithoutWait"); return 0;
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

/*  The main function.
 */

int main(int argc, char** argv){

	//Setting the signal interrupt to its default function. 
	signal(SIGINT, InterruptHandler);

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
	 
		// for(i=0;tokens[i]!=NULL;i++){
		// 	printf("%s\n", tokens[i]);
		// }
		//printf("%d\n", strcmp(tokens[0], "cd\n"));
		
		
		// Various cases - cd, run, parallel, and other commands.
		// Piping, redirection and background processes added.

		// It falls back on executeCommand() if no other condition is satisfied.
		
		if(isPipedCommand(input)) {
			//printf("%s\n", "It is piped\n");
			executePiped(tokens);
		}

		else if(isValidRedirectionAppend(tokens)) {
			char * inputFile = (char *) malloc(1000*sizeof(char)), * outputFile = (char *) malloc(1000*(sizeof(char)));
			findInOutFilesAppend(tokens, inputFile, outputFile);
			// printf("%s\n", "It is a redirection\n");
			// printf("%s %s\n", inputFile, outputFile);
			executeRedirectedAppend(input, inputFile, outputFile);
		}

		else if(isValidRedirection(input)) {
			char * inputFile = (char *) malloc(1000*sizeof(char)), * outputFile = (char *) malloc(1000*(sizeof(char)));
			findInOutFiles(tokens, inputFile, outputFile);
			// printf("%s\n", "It is a redirection\n");
			// printf("%s %s\n", inputFile, outputFile);
			executeRedirected(input, inputFile, outputFile);
		}

		else if(isValidBackground(input))
		{
			char* tokenInput = strtok(input,"&");
			executeInBackGround(tokenInput);
		}
		
		else if(strcmp(tokens[0], "cd") == 0) {
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

int isPipedCommand(char * input) {
	int i = 0;
	for(i = 0; input[i] != '\0'; i++) {
		if(input[i] == '|') return 1;
	}
	return 0;
}

int isValidRedirection(char * input) {
	int i = 0, outcount = 0, incount = 0;
	for(i = 0; input[i] != '\0'; i++) {
		if(input[i] == '>') {
			if(outcount == 0) outcount++;
			else perror("More than one out redirection.\n");
		}

		if(input[i] == '<') {
			if(incount == 0) incount++;
			else perror("More than one in redirection.\n");
		}
	}
	if(incount == 1 || outcount == 1) return 1;
	else return 0;
}

int isValidRedirectionAppend(char ** tokens) {
	int i = 0, outcount = 0, incount = 0;
	for(i = 0; tokens[i] != NULL; i++) {
		if(strcmp(tokens[i],"<") == 0) {
			if(incount == 0) incount++;
			else perror("More than one in redirection.\n");
		}

		if(strcmp(tokens[i],">>") == 0) {
			if(outcount == 0) outcount++;
			else perror("More than one append redirection.\n");
		}
	}
	if(outcount == 1) return 1;
	else return 0;
}

int isValidBackground(char* input)
{
	int i = 0;
	for(i = 0; input[i] != '\0';i++)
	{
		if(input[i] == '&') return 1;
	}
	return 0;
}