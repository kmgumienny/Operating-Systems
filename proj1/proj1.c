#include <iostream>
using namespace std;
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX_CHAR 128
#define MAX_ARG 32

void run_command(char **argv){
    int pid;
    if ((pid = fork()) < 0) {
        cerr << "Fork error\n";
        exit(1);
    }
    else if (pid == 0) {
		/* child process */
		if (execvp(argv[0], &argv[0]) < 0) {
			cerr << "Execve error\n";
			exit(1);
		}
	}else {
		/* parent */
		wait(0); /* wait for the child to finish */
	}
}

/*
 * Main will want to do is check if the user wants to run a command
 * in shell mode or simply run a command that would run as
 * a background command.
 */
main(int argc, char **argv){
/* argc -- number of arguments */
/* argv -- an array of strings */

    char prompt[] = "==>";

    /*
     * The user started the program with a command so the program
     * will simply run that command
    */
    if(argc > 1){
    	run_command(&argv[1]);
    }

    cout << prompt;
//    while(0){
//    	cout << prompt
//		c
//    }

}