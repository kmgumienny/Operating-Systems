//
// Created by Kamil Gumienny on 9/21/19.
//

using namespace std;
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "msg.h"

#define REQUEST 1
#define REPLY 2
#define MAX_THREADS 10
#define MAX_CHAR 128


sem_t* psem; /* pointers to producers (sender) */
sem_t* csem; /* pointers to consumers (receivers) */
struct msg* mailboxes;


void SendMsg(int iTo, struct msg *pMsg) {
    sem_wait(&psem[iTo]);
    mailboxes[iTo].iFrom = pMsg->iFrom;
    mailboxes[iTo].value = pMsg->value;
    mailboxes[iTo].cnt = pMsg->cnt;
    mailboxes[iTo].tot = pMsg->tot;
    sem_post(&csem[iTo]);
}


void RecvMsg(int iRecv, struct msg *pMsg) { // msg as ptr, C/C++
    sem_wait(&csem[iRecv]);
    pMsg->value = mailboxes[iRecv].value;
    sem_post(&psem[iRecv]);
}


void* adder(void* argv){
    // cast to long here to avoid losing information (can't cast int).
    time_t start, end;
    start = time(NULL);
    int mailbox_index = (long) argv;

//    cout << "Started Thread " << mailbox_index << endl;
    // Malloc and then enter 0s to clear garbage data malloc designated
    msg* my_message = (msg*)(malloc(sizeof(struct msg)));
    my_message->iFrom = 0;
    my_message->value = 0;
    my_message->cnt = 0;
    my_message->tot = 0;

    while(1) {
        RecvMsg(mailbox_index, my_message);

        if (my_message->value > 0) {
            my_message->cnt += 1;
            my_message->value += my_message->value;
            sleep(1);
        }
        // Terminate Thread because value less than -1 received.
        else{
            my_message->iFrom = mailbox_index;
            my_message->tot = time(NULL)-start;
            SendMsg(0, my_message);
            return 0;
        }
    }
}


void InitMailBox(int num_mailboxes, pthread_t* threads){
    // add 1 to the total number of mailboxes because main thread
    mailboxes = (msg*)(malloc(num_mailboxes+1 * sizeof(struct msg)));
    threads = (pthread_t*)malloc((sizeof(pthread_t) * num_mailboxes));
    psem = (sem_t *)(malloc(num_mailboxes+1 * sizeof(sem_t)));
    csem = (sem_t *)(malloc(num_mailboxes+1 * sizeof(sem_t)));

    //Threads are initialized and psem + csem for all threads but the last.
    for(int i = 1; i <= num_mailboxes; i++){
        sem_init(&psem[i-1], 0, 1);
        sem_init(&csem[i-1], 0, 0);
        // https://stackoverflow.com/questions/6990888/c-how-to-create-thread-using-pthread-create-function
        if ((pthread_create(&threads[i-1], NULL, adder, (void *) i)) != 0){
            cout << "Issue creating thread " << i << ". Exiting.";
            exit(1);
        }
    }
    //Initialize the last psem and csem.
    sem_init(&psem[num_mailboxes], 0, 1);
    sem_init(&csem[num_mailboxes], 0, 0);
    return;
}


int main(int argc, char *argv[]) {
    int num_mailboxes, value, destination, scan_return;
    pthread_t* threads;
    char input[MAX_CHAR];
    struct msg* a_msg = (msg*)malloc(sizeof(struct msg));

    if (!(argc == 2)){
        cout << "You entered an invalid number of arguments. Refer to the readme." << endl;
        return -1;
    }

    try {
        num_mailboxes = atoi(argv[1]);
    } catch (const std::exception& e) { // caught by reference to base
        std::cout << " Error regarding # mailboxes. Error reads: '"
                  << e.what() << "'\n";
    }

    if (num_mailboxes > MAX_THREADS){
        cout << "Too many threads specified. Using the maximum of 10" << endl;
        num_mailboxes = 10;
    }

    InitMailBox(num_mailboxes, threads);

    cout << "Program initialized with " << num_mailboxes << " mailboxes. Enter two integers between 1 and "
         << num_mailboxes << " separated by a space (value destination_thread)."
                             " Enter nonsense to end program and print statistics." << endl;

    while(1){
        fgets(input, MAX_CHAR, stdin);
        scan_return = sscanf(input, "%d %d", &value, &destination);

        // check if we scanned in 2 elements or if time to exit
        if (scan_return != 2){
            cout << "Beginning termination" << endl;
            a_msg -> iFrom = 0;
            a_msg -> value = -1;
            a_msg -> cnt = 0;
            a_msg -> tot = 0;

            for (int i = 1; i <= num_mailboxes; i++){
                SendMsg(i, a_msg);
            }

            for (int i = 1; i <= num_mailboxes; i++){
                RecvMsg(i, a_msg);
                cout << "The result from thread "
                     << a_msg -> iFrom << " is "
                     << a_msg -> value << " from "
                     << a_msg -> cnt << " operations during "
                     << a_msg -> tot << " secs." << endl;
            }

            for (int i = 0; i <= num_mailboxes; i++){
                pthread_join(threads[i], NULL);
                sem_destroy(&csem[i]);
                sem_destroy(&psem[i]);
            }
            return 0;
        }

        if (!((value >= 1 && value <= num_mailboxes) && (destination >= 1 && destination <= num_mailboxes))) {
            cout << "Please enter a number between 1 and " << num_mailboxes << endl;
            continue;
        }

        a_msg -> iFrom = 0;
        a_msg -> value = value;
        a_msg -> cnt = 0;
        a_msg -> tot = 0;

        SendMsg(destination, a_msg);
    }

}

