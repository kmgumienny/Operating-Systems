using namespace std;
#include <iostream>
#include <iomanip>
#include <sys/syscall.h>
#include <sys/resource.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "process_struct.h"

// These values MUST match the syscall_32.tbl modifications:
#define __NR_cs3013_syscall1 377
#define __NR_cs3013_syscall2 378
#define __NR_cs3013_syscall3 379

int main () {
	// this struct will hold information on system usage	
	struct processinfo info;
	
	int pid1 = 0, pid2 = 0;

	if ((pid1 = fork()) == 0){
		if(pid1 == 0){
			sleep(10);
			return 0;	
		}
			
		cout << "Fork 1 Error" << endl;
		exit(1);
	}else{
		if (pid1 != 0 && (pid2 = fork()) == 0){
			if(pid2 == 0){
				sleep(12);
				return 0;
			}
			cout << "Fork 2 Error" << endl;
			exit(1);
		}else{

			if ((int) syscall(__NR_cs3013_syscall2, &info) == -1)
				return -1;
	
			cout << "It worked" << endl;

    			cout << setw(40) << "Current state of process: " << info.state << endl;
    			cout << setw(40) << "Process ID of this process: " << info.pid << endl;
    			cout << setw(40) << "Process ID of parent process: " << info.parent_pid << endl;
    			cout << setw(40) << "Process ID of youngest child process: " << info.youngest_child << endl;
    			cout << setw(40) << "Process ID of a child process: " << info.younger_sibling << endl;
    			cout << setw(40) << "Process ID of next oldest process: " << info.older_sibling << endl;
    			cout << setw(40) << "User ID of process owner: " << info.uid << endl;
    			cout << setw(40) << "Time since boot: " << info.start_time << endl;
    			cout << setw(40) << "CPU time in user mode: " << info.user_time << endl;
    			cout << setw(40) << "CPU time in system mode: " << info.sys_time << endl;
    			cout << setw(40) << "CPU user time of children: " << info.cutime << endl;
    			cout << setw(40) << "CPU system time of children: " << info.cstime << endl;

	return 0;
		}
	}


        return 0;
}
