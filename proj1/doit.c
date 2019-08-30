using namespace std;
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <iomanip>
#include <cstring>

#define MAX_CHAR 128
#define MAX_ARG 32

void run_command(char **argv, int **list_pid, char ** list_output, long **list_times, int run_background) {
    int pid, index;
    struct rusage info;
    struct timeval start, stop;
    double wall_clock_duration;
    char inbuf[MAX_CHAR];
    int fds[2];
    const char* output;

//    if(run_background)
//        pipe(fds);


    if ((pid = fork()) < 0) {
        cerr << "Fork error\n";
        exit(1);
    } else {
        gettimeofday(&start, NULL);
        if (pid == 0) {
            /* child process */
//            if(run_background){
//                dup2(fds[1], STDOUT_FILENO);
//                close(fds[0]);
//                close(fds[1]);
//            }
            if (execvp(argv[0], &argv[0]) < 0) {
                cerr << "Execvp error\n";
                exit(1);
            }
        }
        else {
            /* parent */
            if(run_background){
//                close(fds[1]);
//                int nbytes = read(fds[0], inbuf, sizeof(inbuf));
//                output = inbuf;
                cout << "Process " << pid << " started running " << argv[0] << endl;
                for(index = 0; index < 20; index++){
                    if(*list_pid[index] == 0) {
                        *list_pid[index] = pid;
                        *list_times[index*2] = start.tv_sec;
                        *list_times[(index*2)+1] = start.tv_usec;
                        strcpy(list_output[index*2], argv[0]);
                        break;
                    }
                }
            }
            else
                wait(NULL);
            gettimeofday(&stop, NULL);
            wall_clock_duration = ((double) (stop.tv_sec - start.tv_sec) * 1000 + (double) (stop.tv_usec - start.tv_usec) / 1000);
        }
    }
    getrusage(RUSAGE_SELF, &info);
//    string wall_time = "Wall clock time: " + to_string(wall_clock_duration);
//    string cpu_time = "User CPU time: " + to_string(info.ru_utime.tv_usec);
//    string sys_cpu_time = "System CPU time: " +  to_string(info.ru_stime.tv_usec);
//    string preempted_inv = "Times process preempted involuntarily: " + to_string(info.ru_nivcsw);
//    string preempted_vol = "Times process preempted voluntarily: " + to_string( info.ru_nvcsw);
//    string maj_fault = "Number major page faults: " + to_string(info.ru_majflt);
//    string min_fault = "Number minor page faults: " + to_string(info.ru_minflt);
    if (!run_background) {
        cout << setw(40) << list_output[41] << wall_clock_duration << "ms" << endl;
        cout << setw(40) << list_output[42] << info.ru_utime.tv_usec / 1000 << "ms" << endl;
        cout << setw(40) << list_output[43] << info.ru_stime.tv_usec / 1000 << "ms" << endl;
        cout << setw(40) << list_output[44] << info.ru_nivcsw << endl;
        cout << setw(40) << list_output[45] << info.ru_nvcsw << endl;
        cout << setw(40) << list_output[46] << info.ru_majflt << endl;
        cout << setw(40) << list_output[47] << info.ru_minflt << endl;
    }

}

void configureStatistics(char** list_output){
    strcpy(list_output[40], "Command Output: ");
    strcpy(list_output[41], "Total wall clock time till this message: ");
    strcpy(list_output[42], "User CPU time: ");
    strcpy(list_output[43], "System CPU time: ");
    strcpy(list_output[44], "Times process preempted involuntarily: ");
    strcpy(list_output[45], "Times process preempted voluntarily: ");
    strcpy(list_output[46], "Number major page faults: ");
    strcpy(list_output[47], "Number minor page faults: ");
}

int check_processes(int **list_pid, char ** list_output, long **list_times){
    int status;
    struct rusage child_info;
    int curr_index = 0;
    for(curr_index; curr_index < 20; curr_index++) {
        if (*list_pid[curr_index] != '\0') {
            pid_t return_pid = waitpid(*list_pid[curr_index], &status, WNOHANG);
            if (return_pid == 0)
                return 1;
            else if (return_pid == -1) {
                list_pid[curr_index] = 0;
            } else if (return_pid == *list_pid[curr_index]) {
                cout << "Child process " << *list_pid[curr_index] << " running function \"" << list_output[curr_index * 2]
                     << "\" finished with output above somewhere" << endl;
                //<< list_output[(curr_index * 2) + 1] << endl;
                *list_pid[curr_index] = 0;
                *list_output[curr_index * 2] = '\0';
                *list_output[(curr_index * 2) + 1] = '\0';

                getrusage(RUSAGE_CHILDREN, &child_info);
                timeval stop;
                gettimeofday(&stop, NULL);

                double wall_clock_duration = ((double) (stop.tv_sec - *list_times[curr_index*2]) * 1000 + (double) (stop.tv_usec - *list_times[(curr_index*2)+1]) / 1000);

                cout << setw(40) << list_output[41] << wall_clock_duration << "ms" << endl;
                cout << setw(40) << list_output[42] << child_info.ru_utime.tv_usec /1000 << "ms" << endl;
                cout << setw(40) << list_output[43] << child_info.ru_stime.tv_usec /1000 << "ms" << endl;
                cout << setw(40) << list_output[44] << child_info.ru_nivcsw << endl;
                cout << setw(40) << list_output[45] << child_info.ru_nvcsw  << endl;
                cout << setw(40) << list_output[46] << child_info.ru_majflt  << endl;
                cout << setw(40) << list_output[47] << child_info.ru_minflt  << endl;


            }
        }
    }
    cout << endl << endl << endl;
    return 0;
}

/*
 * Main will want to do is check if the user wants to run a command
 * in shell mode or simply run a command that would run as
 * a background command.
 */
int main(int argc, char **argv) {

    char prompt[] = "==>";
    char *token = (char *) malloc(MAX_CHAR * sizeof(char));
    int will_background = 0;
    char input[MAX_CHAR];
    char delimiter[] = " \n";
    char **args = (char **) calloc(MAX_ARG + 1, sizeof(char));
    int **list_pid = (int **) calloc(20, sizeof(int));
    long **list_times = (long **) calloc(20 * 2, sizeof(long));
    char **list_output = (char **) calloc(48, sizeof(char));


    for (int i = 0; i < MAX_ARG + 1; i++) {
        args[i] = (char *) calloc(1080, sizeof(char));
    }
    for (int i = 0; i < 20; i++) {
        list_pid[i] = (int *) calloc(10, sizeof(int));
    }
    for (int i = 0; i < 20; i++) {
        list_times[i] = (long *) calloc(2, sizeof(long));
    }
    for (int i = 0; i < 48; i++) {
        list_output[i] = (char *) calloc(300, sizeof(char));
    }

    configureStatistics(&list_output[0]);

//    This command will overwrite list_output[8] with garbage.
    strncpy(args[0], "./doit", sizeof("./doit"));
    list_output[8] = (char *) calloc(300, sizeof(char));

    if (argc > 1) {
        if (strcmp(argv[argc - 1], "&") == 0)
            will_background = 1;
        run_command(&argv[1], &list_pid[0], &list_output[0], &list_times[0], will_background);
    }

    while (1) {
        cout << prompt;
        fgets(input, MAX_CHAR, stdin);
        int size_input = strlen(input);
        if (size_input == 1) {
            cout << "Nothing entered" << endl;
            continue;
        }
        token = strtok(input, delimiter);
        int index;
        for (index = 1; index < MAX_ARG; index++) {
            if (token == NULL) {
                if (strcmp(args[index - 1], "&") == 0) {
                    will_background = 1;
                    //This adds a null pointer to the end of the argument. For the next command ran, this has to be cleared out.
                    args[index - 1] = NULL;
                }
                args[index] = NULL;
                break;
            }

            strcpy(args[index], token);
            token = strtok(NULL, delimiter);
        }

        int are_processes = check_processes(&list_pid[0], &list_output[0], &list_times[0]);

        if (strcmp(args[1], "exit") == 0) {
            int no_exit = 1;
            while (no_exit) {
                if (are_processes) {
                    cout << "There are still processes running. Cannot exit." << endl;
                    wait(0);
                } else {
                    //remove null pointer so memory can be cleared
                    args[index] = (char *) calloc(50, sizeof(char));
                    no_exit = 0;
                }
            }
            break;
        } else if (strcmp(args[1], "cd") == 0) {
            if (chdir(args[2]) != 0) {
                cout << "Could not change directory" << endl;
            }
            char s[100];
            cout << "Now in directory " << getcwd(s, 100) << endl;
        } else if ((strcmp(args[1], "set") == 0) && (strcmp(args[2], "prompt") == 0) && (strcmp(args[3], "=") == 0))
            strcpy(prompt, args[4]);
        else if (strcmp(args[1], "jobs") == 0){
            for (int i = 0; i < 20; ++i) {
                if (list_pid[i] != 0){
                    cout << "Process " << list_pid[i] << " is running " << list_output[i*2] << endl;
                }
            }
        }

        else
            run_command(&args[1], &list_pid[0], &list_output[0], &list_times[0], will_background);

        // reallocate memory where a null pointer was added to be executed in execvp
        if (will_background)
            args[index - 1] = (char *) calloc(50, sizeof(char));
        args[index] = (char *) calloc(50, sizeof(char));
        will_background = 0;

    }
    free(token);
    /*
     * After a ton of testing, I could not get the program to free the allocated memory.
     * After debugging I found that the strcpy command has been overwriting calloc'd and malloc'd
     * memory with garbage. I'm not sure how to fix it so I decided to write a bad program instead.
     */
//    cout << "free token" << endl;
//
//    for (int i = 0; i < 20; i++)
//        free(list_pid[i]);
//
////    free(list_pid);
//    cout << "free list_pid" << endl;
//
//    for (int i = 0; i < 20; i++)
//        free(list_times[i]);
//
////    free(list_times);
//    cout << "free list_times" << endl;
//
//    for (int i = 0; i < 48; i++)
//        free(list_output[i]);
//
////    free(list_output);
//    cout << "free list_output" << endl;
//
//    args[0] = (char*)calloc(1080, sizeof(char));
//    for (int i = 0; i < MAX_ARG + 1; i++){
//        free(args[i]);
//        cout << i << endl;
//    }
////    free(args);
//    cout << "free args" << endl;
    return 0;
}