using namespace std;

#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <string.h>
#include <bits/ios_base.h>
#include <fstream>

#define MAX_BUFFER 256
#define MAX_THREADS 15
#define FILE_NAME_SEMAPHORE 0
#define BAD_FILE_SEMAPHORE 1
#define DIRECTORY_SEMAPHORE 2
#define REG_FILE_SEMAPHORE 3
#define SPEC_FILE_SEMAPHORE 4
#define TEXT_FILE_SEMAPHORE 5


sem_t *semaphores;
int num_threads;
char **list_files;

int bad_files;
int directories;
int regular_files;
int special_files;
long int regular_file_size;
int text_files;
long int text_file_size;


int check_is_textfile(char *file_name) {
    // Go byte by byte and check to see if byte is printable
    char *char_buf = (char *) malloc(sizeof(char));
    ifstream is(file_name, ifstream::binary);
    if (is) {
        while (!is.eof()) {
            is.read(char_buf, sizeof(char));
            if (isprint(*char_buf) == 0 && isspace(*char_buf) == 0)
                return 0;
        }
    }
    free(char_buf);
    return 1;
}


void *thread_process(void *thread_num) {
    int index = (long) thread_num;
    char *file_name = (char *) malloc(sizeof(char) * MAX_BUFFER);
    struct stat statbuf;

    sem_wait(&semaphores[FILE_NAME_SEMAPHORE]);
    strcpy(file_name, list_files[index]);
    sem_post(&semaphores[FILE_NAME_SEMAPHORE]);

    if (stat(file_name, &statbuf) == -1) {
        sem_wait(&semaphores[BAD_FILE_SEMAPHORE]);
        bad_files++;
        sem_post(&semaphores[BAD_FILE_SEMAPHORE]);
    } else
        // Check for directory
    if (S_ISDIR(statbuf.st_mode)) {
        sem_wait(&semaphores[DIRECTORY_SEMAPHORE]);
        directories++;
        sem_post(&semaphores[DIRECTORY_SEMAPHORE]);
    } else
        // Check for regular file
    if (S_ISREG(statbuf.st_mode)) {
        sem_wait(&semaphores[REG_FILE_SEMAPHORE]);
        regular_files++;
        regular_file_size += statbuf.st_size;
        sem_post(&semaphores[REG_FILE_SEMAPHORE]);

        //Check if printable chars
        if (check_is_textfile(file_name)) {
            sem_wait(&semaphores[TEXT_FILE_SEMAPHORE]);
            text_files++;
            text_file_size += statbuf.st_size;
            sem_post(&semaphores[TEXT_FILE_SEMAPHORE]);
        }
    } else {
        // Special file
        sem_wait(&semaphores[SPEC_FILE_SEMAPHORE]);
        special_files++;
        sem_post(&semaphores[SPEC_FILE_SEMAPHORE]);
    }

    free(file_name);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int thread_status, total_threads_ran = 0, this_thread_iteration = 0;
    pthread_t *threads;
    struct rusage process_info;
    struct timeval start, end;
    struct stat *statbuf;
    statbuf = (struct stat *) malloc(sizeof(struct stat));
    FILE *fp;
    char *file_buffer = (char *) malloc(sizeof(char) * MAX_BUFFER);
    size_t buffer_size = MAX_BUFFER;

    fp = fopen("devs.txt", "r");
    if (fp == NULL) {
        cout << "Error opening the file" << endl;
        exit(1);
    }

    bad_files = 0;
    directories = 0;
    regular_files = 0;
    special_files = 0;
    regular_file_size = 0;
    text_files = 0;
    text_file_size = 0;

    // Run the serial version of the program
    if (argc == 1) {
        while (getline(&file_buffer, &buffer_size, fp) != -1) {
            // remove newline character and replace with null terminator for stat
            file_buffer[strlen(file_buffer) - 1] = '\0';

            // Check for bad file
            if (stat(file_buffer, statbuf) == -1) {
                bad_files++;
            } else
                // Check for directory
            if (S_ISDIR(statbuf->st_mode)) {
                directories++;
            } else
                // Check for regular file
            if (S_ISREG(statbuf->st_mode)) {
                regular_files++;
                regular_file_size += statbuf->st_size;

                //Check if printable chars
                if (check_is_textfile(file_buffer)) {
                    text_files++;
                    text_file_size += statbuf->st_size;
                }
            } else
                // Special file
                special_files++;
        }
    } else if (argc == 3 && !strcmp(argv[1], "thread")) {
        void *status;

        num_threads = atoi(argv[2]);
        if (num_threads > MAX_THREADS) {
            cout << "Number of threads must be less than 15. Setting to 15" << endl;
            num_threads = 15;
        }
        if (num_threads < 1) {
            cout << "Number of threads must be 1 or greater. Setting to 1" << endl;
            num_threads = 1;
        }
        cout << "Running in thread mode with " << num_threads << " threads." << endl;

        list_files = (char **) (malloc(num_threads * sizeof(char *)));
        for (int i = 0; i < num_threads; i++) {
            list_files[i] = (char *) malloc(MAX_BUFFER * sizeof(char));
        }
        threads = (pthread_t *) malloc(num_threads * sizeof(pthread_t));
//            pthread_t threads[num_threads];
        semaphores = (sem_t *) malloc(6 * sizeof(sem_t));
        for (int i = 0; i < 6; i++) {
            sem_init(&semaphores[i], 0, 1);
        }

        while (getline(&file_buffer, &buffer_size, fp) != -1) {
            // remove newline character and replace with null terminator for stat
            file_buffer[strlen(file_buffer) - 1] = '\0';
            this_thread_iteration = total_threads_ran % num_threads;

            if (total_threads_ran > num_threads) {
                if (pthread_join(threads[this_thread_iteration], NULL)) {
                    cout << "Error joining threads. Exiting." << endl;
                    exit(1);
                }
            }
            strcpy(list_files[this_thread_iteration], file_buffer);
            if (pthread_create(&threads[this_thread_iteration], NULL, thread_process, (void *) this_thread_iteration)) {
                cout << "Error creating threads. Exiting." << endl;
                exit(1);
            }
            total_threads_ran++;
        }
    } else {
        cout << "Please refer to the readme on commandline arguments" << endl;
        exit(1);
    }

    if (min(total_threads_ran, num_threads) == total_threads_ran){
        for (int i = 0; i < total_threads_ran; i++) {
            if (pthread_join(threads[i], NULL)) {
                cout << "Error joining threads. Exiting." << endl;
                exit(1);
            }
        }
    } else
        for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[(total_threads_ran + i) % num_threads], NULL)) {
            cout << "Error joining threads. Exiting." << endl;
            exit(1);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        free(list_files[i]);
    }

    cout << "Bad Files: " << bad_files << endl;
    cout << "Directories: " << directories << endl;
    cout << "Regular Files: " << regular_files << endl;
    cout << "Special Files: " << special_files << endl;
    cout << "Regular File Bytes: " << regular_file_size << endl;
    cout << "Text Files: " << text_files << endl;
    cout << "Text File Bytes: " << text_file_size << endl;
    fclose(fp);

    free(list_files);
    free(semaphores);


    return 0;
}