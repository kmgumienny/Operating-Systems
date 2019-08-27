#include <iostream>
using namespace std;
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <sys/resource.h>
#include <iomanip>
#include <errno.h>
#include <cstring>

#define MAX_CHAR 128
#define MAX_ARG 32

void run_command(char **argv, int run_background) {
	int pid;
	struct rusage info;
	struct timeval start, stop;
	char inbuf[MAX_CHAR];
	int fds[2];

	if(run_background)
		pipe(fds);


	if ((pid = fork()) < 0) {
		cerr << "Fork error\n";
		exit(1);
	} else {
		gettimeofday(&start, NULL);
		if (pid == 0) {
			/* child process */
			if(run_background){
				dup2(fds[1], STDOUT_FILENO);
				close(fds[0]);
				close(fds[1]);
			}
			if (execvp(argv[0], &argv[0]) < 0) {
				cerr << "Execvp error\n";
				exit(1);
			}
		}
		else {
			/* parent */
			if(run_background){
				close(fds[1]);
				int nbytes = read(fds[0], inbuf, sizeof(inbuf));
				cout << "Output " << nbytes << inbuf;
				cout << "Just inbuf " << inbuf << endl;
				gettimeofday(&stop, NULL); //Stop the timer before clocking out of this function
			}
			else
				wait(NULL);
			gettimeofday(&stop, NULL);
			cout << endl << endl;
			cout << setw(40) << "Wall Clock Duration in Milliseconds " << ((double) (stop.tv_sec - start.tv_sec) * 1000 + (double) (stop.tv_usec - start.tv_usec) / 1000) << endl;
		}
	}
	getrusage(RUSAGE_SELF, &info);
	cout << setw(40) << "User CPU time: " << info.ru_utime.tv_usec << endl;
	cout << setw(40) << "System CPU time: " << info.ru_stime.tv_usec << endl;
	cout << setw(40) << "Times process preempted involuntarily: " << info.ru_nivcsw << endl;
	cout << setw(40) << "Times process preempted voluntarily: " << info.ru_nvcsw  << endl;
	cout << setw(40) << "Number major page faults " << info.ru_majflt  << endl;
	cout << setw(40) << "Number minor page faults " << info.ru_minflt  << endl;

}

/*
 * Main will want to do is check if the user wants to run a command
 * in shell mode or simply run a command that would run as
 * a background command.
 */
main(int argc, char **argv){
/* argc -- number of arguments */
/* argv -- an array of strings */
/* status -- int holding the status of waitpid */
/* will_background -- integer indicating whether a task will run in background */
/* input -- array to hold input string
/* args -- calloc'd array holding commandline args to be run*/
/* delimiter -- what we want to strip out of input using strtok  */



    char prompt[] = "==>";
    int status;
    int will_background = 0;
    char input[MAX_CHAR];
    char **args = (char**)calloc(MAX_ARG+1, sizeof(char));
    for(int i = 0; i < MAX_ARG+1; i++) {
		args[i] = (char *) calloc(50, sizeof(char));
	}
    char delimiter[] = " ";

	strcpy(args[0], "./doit");
    /*
     * The user started the program with a command so the program
     * will simply run that command
    */
    if(argc > 1) {
		if (strcmp(argv[argc], "&") == 0)
			will_background = 1;
		run_command(&argv[1], will_background);
	}

   	while (1){
   		cout << prompt;
   		fgets(input, MAX_CHAR, stdin);
   		int size_input = strlen(input);
   		if(size_input == 1){
   			cout << "Nothing entered" << endl;
   			continue;
   		}
   		char *strip = strtok(input, delimiter);
   		for(int i = 1; i < MAX_ARG; i++){
			if(strip == NULL){
				if(strcmp(args[i-1], "&"))
					will_background = 1;
				break;
			}
			strcpy(args[i], strip);
			strip = strtok(NULL, delimiter);
   		}

   		for(int i = 1; i < MAX_ARG; i++) {
			cout << args[i] << endl;
		}
   	}
	free(args);
	return 0;

}