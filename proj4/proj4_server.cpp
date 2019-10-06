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

sem_t* semaphores;
int num_threads;
char ** list_files;

int bad_files;
int directories;
int regular_files;
int special_files;
long int regular_file_size;
int text_files;
long int text_file_size;


void* thread_proccess(void* argv) {
    int index = (long) argv;
    return 0;
}


int check_is_textfile(char* file_name){
    // Go byte by byte and check to see if byte is printable
    char* char_buf = (char *)malloc(sizeof(char));
    ifstream is(file_name, ifstream::binary);
    if (is){
        while(!is.eof()){
            is.read(char_buf, sizeof(char));
            if(isprint(*char_buf) == 0 && isspace(*char_buf) == 0)
                return 0;
            }
        }
    free(char_buf);
    return 1;
}

int main(int argc, char *argv[]) {
    struct rusage process_info;
    struct timeval start, end;
    struct stat *statbuf;
    statbuf = (struct stat*)malloc(sizeof(struct stat));
    FILE *fp;
    char* file_buffer = (char*)malloc(sizeof(char) * MAX_BUFFER);
    size_t buffer_size = MAX_BUFFER;

    fp = fopen("devs.txt", "r");
    if (fp == NULL)
    {
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
    if (argc == 1){
        while(getline(&file_buffer, &buffer_size, fp) != -1){
            // remove newline character and replace with null terminator for stat
            file_buffer[strlen(file_buffer) - 1] = '\0';

            // Check for bad file
            if(stat(file_buffer, statbuf) == -1){
                bad_files++;
            } else
                // Check for directory
                if (S_ISDIR(statbuf->st_mode)){
                    directories++;
            } else
                // Check for regular file
                if(S_ISREG(statbuf->st_mode)){
                    regular_files++;
                    regular_file_size += statbuf->st_size;

                    //Check if printable chars
                    if(check_is_textfile(file_buffer)){
                        text_files++;
                        text_file_size += statbuf->st_size;
                    }

                }
                else
                // Special file
                special_files++;
        }

        cout << "Bad Files: " << bad_files << endl;
        cout << "Directories: " << directories << endl;
        cout << "Regular Files: " << regular_files << endl;
        cout << "Special Files: " << special_files << endl;
        cout << "Regular File Bytes: " << regular_file_size << endl;
        cout << "Text Files: " << text_files << endl;
        cout << "Text File Bytes: " << text_file_size << endl;
        fclose(fp);
        return 0;
    }else
        if (argc == 3 && !strcmp(argv[1], "thread")){
            int thread_status, total_threads_ran = 0, this_thread_iteration = 0;
            void* status;



            num_threads = atoi(argv[2]);
            if (num_threads > MAX_THREADS ){
                cout << "Number of threads must be less than 15. Setting to 15" << endl;
                num_threads = 15;
            }
            if (num_threads < 1){
                cout << "Number of threads must be 1 or greater. Setting to 1" << endl;
                num_threads = 1;
            }
            cout << "Running in thread mode with " << num_threads << " threads." << endl;

            list_files = (char **)(malloc(num_threads * sizeof(char *)));
            for (int i = 0; i < num_threads; i++){
                list_files[i] = (char *)malloc(MAX_BUFFER * sizeof(char));
            }

            pthread_t threads[num_threads];
            semaphores = (sem_t*)malloc(num_threads * sizeof(sem_t));
            for (int i = 0; i < num_threads; i++){
                sem_init(&semaphores[i], 1, 1);
            }

            while(getline(&file_buffer, &buffer_size, fp) != -1) {
                // remove newline character and replace with null terminator for stat
                file_buffer[strlen(file_buffer) - 1] = '\0';
                this_thread_iteration = total_threads_ran % num_threads;

                if(total_threads_ran > num_threads){
                    if(pthread_join(threads[this_thread_iteration], NULL)){
                        cout << "Error joining threads. Exiting." << endl;
                        exit(1);
                    }
                }
                strcpy(list_files[this_thread_iteration], file_buffer);
                if(pthread_create(&threads[this_thread_iteration], NULL, thread_proccess, (void*)this_thread_iteration)){
                    cout << "Error creating threads. Exiting." << endl;
                    exit(1);
                }







            }







        }else{
            cout << "Please refer to the readme on commandline arguments" << endl;
            exit(1);
        }

    for (int i = 0; i < num_threads; i++){
        free(list_files[i]);
    }
    free(list_files);
    free(semaphores);


    return 0;
}