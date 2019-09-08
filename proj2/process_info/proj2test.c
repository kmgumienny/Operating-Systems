using namespace std;
#include <iostream>
#include <iomanip>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "process_struct.h"

// These values MUST match the syscall_32.tbl modifications:
#define __NR_cs3013_syscall1 377
#define __NR_cs3013_syscall2 378
#define __NR_cs3013_syscall3 379


long testCall1 (void) {
        return (long) syscall(__NR_cs3013_syscall1);
}
long testCall2 (struct processinfo *info) {
	
	// if intercept and get -1, there was an error
	if ((int) syscall(__NR_cs3013_syscall2, &info) == -1)
		return -1;

    cout << setw(40) << "Current state of process: " << info->state << endl;
    cout << setw(40) << "Process ID of this process: " << info->pid << endl;
    cout << setw(40) << "Process ID of parent process: " << info->parent_pid << endl;
    cout << setw(40) << "Process ID of youngest child process: " << info->youngest_child << endl;
    cout << setw(40) << "Process ID of a child process: " << info->younger_sibling << endl;
    cout << setw(40) << "Process ID of next oldest process: " << info->older_sibling << endl;
    cout << setw(40) << "User ID of process owner: " << info->uid << endl;
    cout << setw(40) << "Time since boot: " << info->start_time << endl;
    cout << setw(40) << "CPU time in user mode: " << info->user_time << endl;
    cout << setw(40) << "CPU time in system mode: " << info->sys_time << endl;
    cout << setw(40) << "CPU user time of children: " << info->cutime << endl;
    cout << setw(40) << "CPU system time of children: " << info->cstime << endl;

	return 0;
}
long testCall3 ( void) {
        return (long) syscall(__NR_cs3013_syscall3);
}
int main () {
	// this struct will hold information on system usage	
	struct processinfo *info;
	
	int pid1 = 0, pid2 = 0;

	pid1 = fork();
	pid2 = fork();
	if (pid1 == 0)
		sleep(5);
	else if (pid2 == 0)
		sleep(5);
	else{
        printf("The return values of the system calls are:\n");
        printf("\tcs3013_syscall1: %ld\n", testCall1());
        printf("\tcs3013_syscall2: %ld\n", testCall2(info));
        printf("\tcs3013_syscall3: %ld\n", testCall3());
	}
        return 0;
}
