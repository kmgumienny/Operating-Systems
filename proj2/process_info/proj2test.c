#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include "process_struct.h"

// These values MUST match the syscall_32.tbl modifications:
#define __NR_cs3013_syscall1 377
#define __NR_cs3013_syscall2 378
#define __NR_cs3013_syscall3 379





long testCall1 (void) {
        return (long) syscall(__NR_cs3013_syscall1);
}
long testCall2 (struct processinfo *info) {
	// this struct will hold information on system usage	
	struct processinfo info;
	
	// if intercept and get -1, there was an error
	if ((int) syscall(__NR_cs3013_syscall2, &info) == -1)
		return -1;
	
	

	return 0;
}
long testCall3 ( void) {
        return (long) syscall(__NR_cs3013_syscall3);
}
int main () {
        printf("The return values of the system calls are:\n");
        printf("\tcs3013_syscall1: %ld\n", testCall1());
        printf("\tcs3013_syscall2: %ld\n", testCall2());
        printf("\tcs3013_syscall3: %ld\n", testCall3());
        return 0;
}
