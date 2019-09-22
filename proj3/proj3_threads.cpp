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
    sem_wait(&psem[iRecv]);
    pMsg->value = mailboxes[iRecv].value;
    sem_post(&csem[iRecv]);
}


void* adder(void* argv){
    // cast to long here to avoid losing information (can't cast int).
    time_t start, end;
    start = time(NULL);
    int mailbox_index = (long) argv;

    // Malloc and then enter 0s to clear garbage data malloc designated
    msg* my_message = static_cast<msg*>(malloc(sizeof(struct msg)));
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


void InitMailBox(int num_mailboxes, pthread_t * threads){
    // add 1 to the total number of mailboxes because main thread
    mailboxes = static_cast<msg*>(malloc(num_mailboxes+1 * sizeof(struct msg)));
    threads = static_cast<pthread_t *>(malloc(num_mailboxes * sizeof(pthread_t)));
    psem = static_cast<sem_t *>(malloc(num_mailboxes+1 * sizeof(sem_t)));
    csem = static_cast<sem_t *>(malloc(num_mailboxes+1 * sizeof(sem_t)));

    for(int i = 1; i <= num_mailboxes; i++){
        sem_init(&psem[i], 0, 1);
        sem_init(&csem[i], 0, 0);
        // https://stackoverflow.com/questions/6990888/c-how-to-create-thread-using-pthread-create-function
        if (!(pthread_create(&threads[i], NULL, adder, (void *) i))){
            cout << "Issue creating thread " << i << ". Exiting.";
            exit(1);
        }
    }
}


int main(int argc, char *argv[]) {
    int num_mailboxes, value, destination, scan_return;
    pthread_t * threads;
    msg* msg;
    char input[MAX_CHAR];

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

    cout << "Program initialized with " << num_mailboxes << " mailboxes. Enter two integers between 1 and "
    << num_mailboxes << " separated by a space (value destination_thread)."
                        " Enter nonsense to end program and print statistics." << endl;

    InitMailBox(num_mailboxes, threads);
    sem_init(&psem[0], 0, 1);
    sem_init(&csem[0], 0, 0);

    while(1){
        fgets(input, MAX_CHAR, stdin);
        scan_return = sscanf(input, "%d %d", &value, &destination);
        if (!((value >= 1 && value <= num_mailboxes) && (destination >= 1 && destination <= num_mailboxes))) {
            cout << "Please enter a number between 1 and " << num_mailboxes << endl;
            continue;
        }
        // check if we scanned in 2 elements or if time to exit
        if (scan_return != 2){
            cout << "Beginning termination" << endl;
            msg -> iFrom = 0;
            msg -> value = -1;
            msg -> cnt = 0;
            msg -> tot = 0;

            for (int i = 1; i <= num_mailboxes; i++){
                SendMsg(i, msg);
            }

            for (int i = 1; i <= num_mailboxes; i++){
                RecvMsg(i, msg);
                cout << "The result from thread "
                        << msg -> iFrom << " is "
                        << msg -> value << " from "
                        << msg -> cnt << " operations during "
                        << msg -> tot <<" secs." << endl;
            }

            for (int i = 0; i <= num_mailboxes; i++){
                pthread_join(threads[i], NULL);
                sem_destroy(&csem[i]);
                sem_destroy(&psem[i]);
            }
            return 0;
        }

        msg -> iFrom = 0;
        msg -> value = value;
        msg -> cnt = 0;
        msg -> tot = 0;

        SendMsg(destination, msg);


    }

}

