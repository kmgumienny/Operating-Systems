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

sem_t* sems;
int bad_files;
int directories;
int regular_files;
int special_files;
int regular_file_size;
int text_files;
int text_file_size;


int check_is_textfile(char* file_name){
    // Go byte by byte and check to see if byte is printable
    int is_text = 1;
    char* char_buf = (char *)malloc(sizeof(char));
    ifstream is(file_name, ifstream::binary);
    if (is){
        while(!is.eof()){
            is.read(char_buf, sizeof(char));
            if(isprint(*char_buf) == 0 && isspace(*char_buf) == 0){
                continue;
            } else{
                is_text = 0;
                break;
            }
        }
    }
        return is_text;
}

int main(int argc, char *argv[]) {
    struct rusage process_info;
    struct timeval start, end;
    struct stat statbuf;
    char* file_buffer = (char*)malloc(sizeof(char) * MAX_BUFFER);
    size_t buffer_size = MAX_BUFFER;

    bad_files = 0;
    directories = 0;
    regular_files = 0;
    special_files = 0;
    regular_file_size = 0;
    text_files = 0;
    text_file_size = 0;

    // Run the serial version of the program
    if (argc == 1){

        while(getline(&file_buffer, &buffer_size, stdin) != -1){
            // remove newline character and replace with null terminator for stat
            file_buffer[strlen(file_buffer) - 1] = '\0';

            // Check for bad file
            if(stat(file_buffer, &statbuf) == -1){
                bad_files++;


            } else
                // Check for directory
                if (S_ISDIR(statbuf.st_mode)){
                    directories++;
            } else
                // Check for regular file
                if(S_ISREG(statbuf.st_mode)){

                    regular_files++;
                    regular_file_size += statbuf.st_size;

                    //Check if printable chars
                    if(check_is_textfile(file_buffer)){
                        text_files++;
                        text_file_size += statbuf.st_size;

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
        return 0;
    }

}