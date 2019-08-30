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

/*
 * Difference between this shell and a real shell is how processes communicate with each other.
 * Here, child processes chatter whenever they are done or want to complain. In a real shell
 * that communication would be supervised through pipes which is something I tried to implement
 * but my knowledge of C is not that great.
 */

void print_stats(double wall, struct timeval user, struct timeval sys, long inv, long vol, long maj, long min){
    cout << setw(40) << "Total wall clock time till this message: " << wall << endl;
    cout << setw(40) << "User CPU time: " << ((double)(user.tv_usec / 1000) + (double)(user.tv_sec * 1000)) << "ms" << endl;
    cout << setw(40) << "System CPU time: " << ((double)(sys.tv_usec / 1000) + (double)(sys.tv_sec * 1000)) << "ms" << endl;
    cout << setw(40) << "Times process preempted involuntarily: " << inv << endl;
    cout << setw(40) << "Times process preempted voluntarily: " << vol << endl;
    cout << setw(40) << "Number major page faults: " << maj << endl;
    cout << setw(40) << "Number minor page faults: " << min << endl;
}


void run_command(char **argv, int **list_pid, char ** list_output, long **list_times, int run_background) {
    int pid, index;
    struct rusage info;
    struct timeval start, stop;
    double wall_clock_duration;

    if ((pid = fork()) < 0) {
        cerr << "Fork error\n";
        exit(1);
    } else {
        gettimeofday(&start, NULL);
        if (pid == 0) {
            /* child process */
            if (execvp(argv[0], &argv[0]) < 0) {
                cerr << "Execvp error\n";
                exit(1);
            }
        }
        else {
            /* parent */
            if(run_background){
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
    if (!run_background)
        print_stats(wall_clock_duration, info.ru_utime, info.ru_stime, info.ru_nivcsw, info.ru_nvcsw,
                    info.ru_majflt, info.ru_minflt);
}


int check_processes(int **list_pid, char ** list_output, long **list_times){
    int status;
    struct rusage child_info;
    int curr_index = 0;

    for(curr_index; curr_index < 20; curr_index++) {
        if (*list_pid[curr_index] != 0) {
            pid_t return_pid = waitpid(*list_pid[curr_index], &status, WNOHANG);
            if (return_pid == 0)
                return 1;
            else
            if (return_pid == -1) {
                *list_pid[curr_index] = 0;
            } else if (return_pid == *list_pid[curr_index]) {
                cout << "Child process " << *list_pid[curr_index] << " running function \"" << list_output[curr_index * 2]
                     << "\" finished with output above somewhere" << endl;
                //<< list_output[(curr_index * 2) + 1] << endl;
                *list_pid[curr_index] = 0;
                *list_output[curr_index * 2] = '0';
                *list_output[(curr_index * 2) + 1] = '0';

                getrusage(RUSAGE_CHILDREN, &child_info);
                timeval stop;
                gettimeofday(&stop, NULL);

                double wall_clock_duration = ((double) (stop.tv_sec - *list_times[curr_index*2]) * 1000 + (double) (stop.tv_usec - *list_times[(curr_index*2)+1]) / 1000);
                print_stats(wall_clock_duration, child_info.ru_utime, child_info.ru_stime, child_info.ru_nivcsw, child_info.ru_nvcsw,
                            child_info.ru_majflt, child_info.ru_minflt);
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
    char default_prompt[] = "==>";
    char *prompt = (char*)malloc(4*sizeof(char));
    strncpy(prompt, default_prompt, 4);
    int index, size_string;
    char *token = (char *) malloc(MAX_CHAR * sizeof(char));
    int will_background = 0;
    char input[MAX_CHAR];
    char delimiter[] = " \n";
    char **args = (char **) calloc(50*(MAX_ARG + 1), sizeof(char));
    int **list_pid = (int **) calloc(20*2, sizeof(int));
    long **list_times = (long **) calloc(20 * 2, sizeof(long));
    char **list_output = (char **) calloc(40*300, sizeof(char));

//    for (int i = 0; i < MAX_ARG + 1; i++) {
//        args[i] = (char *) calloc(1080, sizeof(char));
//    }

    for (int i = 0; i < 20; i++) {
        list_pid[i] = (int *) calloc(2, sizeof(int));
        *list_pid[i] = 0;
    }
    for (int i = 0; i < 20; i++) {
        list_times[i] = (long *) calloc(2, sizeof(long));
    }
    for (int i = 0; i < 40; i++) {
        list_output[i] = (char *) calloc(300, sizeof(char));
    }

    if (argc > 1) {
        if (strcmp(argv[argc-1], "&") == 0) {
            will_background = 1;
            cout << will_background <<endl;
            argv[argc - 1] = NULL;
        }
        run_command(&argv[1], &list_pid[0], &list_output[0], &list_times[0], will_background);
    }
    will_background = 0;

    while (1) {
        cout << prompt;
        fgets(input, MAX_CHAR, stdin);
        int size_input = strlen(input);
        if (size_input == 1) {
            cout << "Nothing entered" << endl;
            continue;
        }
        token = strtok(input, delimiter);
        size_string = strlen(token);
        args[0] = (char*)calloc((size_string+1), sizeof(char));
        strncpy(args[0], token, size_string);
//        args[0][size_string+1] = '\0\';

        for (index = 1; index < MAX_ARG; index++) {
            token = strtok(NULL, delimiter);
            if (token == NULL) {
                if (strcmp(args[index - 1], "&") == 0) {
                    will_background = 1;
                    //This adds a null pointer to the end of the argument. For the next command ran, this has to be cleared out.
//                    free(args[index-1]);
                    args[index - 1] = NULL;
                }
                if (!will_background) {
                    args[index] = NULL;
                }
                break;
            }
            size_string = strlen(token);
            args[index] = (char*)calloc((size_string+1), sizeof(char));
            strncpy(args[index], token, size_string);
        }


        if (strcmp(args[0], "exit") == 0) {
            int are_processes = check_processes(&list_pid[0], &list_output[0], &list_times[0]);
            if(are_processes)
                cout << "There are still processes running, please wait as you cannot yet exit." << endl;
            int no_exit = are_processes;
            while (no_exit) {
                if (are_processes) {
                    are_processes = check_processes(&list_pid[0], &list_output[0], &list_times[0]);
                } else {
                    //remove null pointer so memory can be cleared
                    cout << "Goodbye" << endl;
                    args[index] = (char *) calloc(1, sizeof(char));
                    no_exit = 0;
                }
            }
            break;
        }
        else
        if (strcmp(args[0], "cd") == 0) {
            if (chdir(args[1]) != 0) {
                cout << "Could not change directory" << endl;
            }
            char s[100];
            cout << "Now in directory " << getcwd(s, 100) << endl;
        }

        else if ((strcmp(args[0], "set") == 0) && (strcmp(args[1], "prompt") == 0) && (strcmp(args[2], "=") == 0)) {
            free(prompt);
            size_string = strlen(args[3]);
            prompt = (char *) calloc(size_string + 1, sizeof(char));
            strncpy(prompt, args[3], size_string);
        }
        else if (strcmp(args[0], "jobs") == 0){
            for (int i = 0; i < 20; ++i) {
                if (*list_pid[i] != 0){
                    cout << "Process " << *list_pid[i] << " is running " << list_output[i*2] << endl;
                }
            }
        }
        else
            run_command(&args[0], &list_pid[0], &list_output[0], &list_times[0], will_background);

        // reallocate memory where a null pointer was added to be executed in execvp
//        for (int i = 0; i < index; i++) {
//            free(args[i]);
//        }
        will_background = 0;

    }
    free(token);
    free(prompt);

    for (int i = 0; i < 20; i++)
        free(list_pid[i]);

    for (int i = 0; i < 20; i++)
        free(list_times[i]);

    for (int i = 0; i < 40; i++)
        free(list_output[i]);

    for (int i = 0; i < MAX_ARG + 1; i++){
        free(args[i]);
    }
    return 0;
}