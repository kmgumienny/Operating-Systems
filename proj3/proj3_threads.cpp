//
// Created by Kamil Gumienny on 9/21/19.
//

using namespace std;
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "helper_structs.h"
#include <queue>
#include <cstring>

#define REQUEST 1
#define REPLY 2
#define MAX_THREADS 10
#define MAX_CHAR 128


sem_t* psem; /* pointers to producers (sender) */
sem_t* csem; /* pointers to consumers (receivers) */
struct msg* mailboxes;


/*
 * TODO implement try_wait sendmsg and return
 */
int NBSendMsg(int iTo, struct msg* pMsg){
    if (sem_trywait(&psem[iTo]) == 0){
        mailboxes[iTo].iFrom = pMsg->iFrom;
        mailboxes[iTo].value = pMsg->value;
        mailboxes[iTo].cnt = pMsg->cnt;
        mailboxes[iTo].tot = pMsg->tot;
        sem_post(&csem[iTo]);
        return 0;
    }else
        return -1;
}

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
    pMsg->iFrom = mailboxes[iRecv].iFrom;
    pMsg->value = mailboxes[iRecv].value;
    pMsg->cnt = mailboxes[iRecv].cnt;
    pMsg->tot = mailboxes[iRecv].tot;
    sem_post(&psem[iRecv]);
}


void* adder(void* argv){
    int my_val, my_count;
    time_t start, end;
    start = time(NULL);
    int mailbox_index = (long) argv;
    struct msg my_message;

    my_message.iFrom = 0;
    my_message.value = 0;
    my_message.cnt = 0;
    my_message.tot = 0;
    my_val = 0;
    my_count = 0;

    while(1) {
        RecvMsg(mailbox_index, &my_message);

        if (my_message.value != -1) {
            my_count += 1;
            my_val += my_message.value;
            mailboxes[mailbox_index].iFrom = 0;
            mailboxes[mailbox_index].value = 0;
            mailboxes[mailbox_index].cnt = 0;
            mailboxes[mailbox_index].tot = 0;
            sleep(1);
        }else{
            my_message.iFrom = mailbox_index;
            my_message.value = my_val;
            my_message.cnt = my_count;
            my_message.tot = time(NULL)-start;
            SendMsg(0, &my_message);
            return 0;
        }
    }
}


void InitMailBox(int num_mailboxes, pthread_t* threads){
    mailboxes = (struct msg*)(malloc((num_mailboxes + 1) * sizeof(struct msg)));
    psem = (sem_t *)(malloc((num_mailboxes+1) * sizeof(sem_t)));
    csem = (sem_t *)(malloc((num_mailboxes+1) * sizeof(sem_t)));

    for (int i = 0; i <= num_mailboxes; i++){
        sem_init(&psem[i], 0, 1);
        sem_init(&csem[i], 0, 0);
    }

    for(int i = 0; i < num_mailboxes; i++){
        int x = i+1;
        // https://stackoverflow.com/questions/6990888/c-how-to-create-thread-using-pthread-create-function
        if ((pthread_create(&threads[i], NULL, adder, (void *) x)) != 0){
            cout << "Issue creating thread " << i << ". Exiting.";
            exit(1);
        }
    }
    return;
}


int main(int argc, char *argv[]) {
    int num_mailboxes, value, destination, scan_return, terminate, read_from_file, nb_terminate, nb, nb_return;
    char input[MAX_CHAR];
    FILE *fp;
    struct msg* a_msg = (msg*)malloc(sizeof(struct msg));
    struct msg* term_msg = (msg*)malloc(sizeof(struct msg));
    queue <struct msg_node> NBQueue;

    if ((argc < 2)){
        cout << "You entered an invalid number of arguments. Refer to the readme. \n "
                "Required arguments \"$./proj3 #threads\". or \"$./proj3 #threads filename.txt\". "
                "\"nb\" argument following either input mode is optional." <<endl;
        return -1;
    }

    try {
        num_mailboxes = atoi(argv[1]);
    } catch (const std::exception& e) { // caught by reference to base
        std::cout << " Error regarding # mailboxes. Error reads: '"
                  << e.what() << endl;
        exit(-1);
    }

    if (num_mailboxes > MAX_THREADS){
        cout << "Too many threads specified. Using the maximum of 10" << endl;
        num_mailboxes = 10;
    }

    if (strcmp(argv[argc-1], "nb") == 0){
        cout << "NB mode activated. All unsent messages will be sent upon termination";
        nb = 1;
    } else
        nb = 0;


    terminate = 0;
    nb_terminate = 0;
    pthread_t threads[num_mailboxes];
    InitMailBox(num_mailboxes, threads);
    cout << "Program initialized with " << num_mailboxes << " mailboxes. " << endl;

    // If the program cannot open a file, it will accept input from stdin
    if ((argc > 2 && !nb) || (argc > 3 && nb)) {
        if (nb)
            fp = fopen(argv[argc-2], "r");
        else
            fp = fopen(argv[argc-1], "r");
        if (fp == NULL) {
            cout << "Error opening the file" << endl;
            exit(1);
        } else
            read_from_file = 1;
    } else
        read_from_file = 0;


    while(1){
        if (read_from_file == 1) {
            if (fgets(input, MAX_CHAR, fp) == NULL)
                terminate = 1;
        }else if (!terminate)
            fgets(input, MAX_CHAR, stdin);

        scan_return = sscanf(input, "%d %d\n", &value, &destination);

        if (scan_return != 2 || scan_return == EOF)
            terminate = 1;
        if ((!(destination >= 1 && destination <= num_mailboxes)) && !terminate) {
            cout << "Skipping message. Destination value must be between 1 and " << num_mailboxes << endl;
            continue;
        }

        if (terminate && nb_terminate == 1){
            cout << "Beginning termination" << endl;
            term_msg -> iFrom = 0;
            term_msg -> value = -1;
            term_msg -> cnt = 0;
            term_msg -> tot = 0;

            for (int i = 1; i <= num_mailboxes; i++){
                SendMsg(i, term_msg);
                RecvMsg(0, a_msg);
                cout << "The result from thread "
                     << a_msg -> iFrom << " is "
                     << a_msg -> value << " from "
                     << a_msg -> cnt << " operations during "
                     << a_msg -> tot << " secs." << endl;
                (void)pthread_join(threads[i-1], NULL);
            }

            for (int i = 0; i <= num_mailboxes; i++){
                sem_destroy(&csem[i]);
                sem_destroy(&psem[i]);
            }

            return 0;

        }else if (terminate == 1 && nb_terminate == 0){
            while(!NBQueue.empty()) {
                struct msg_node aNode = NBQueue.front();
                a_msg->iFrom = 0;
                a_msg->value = aNode.value;
                a_msg->cnt = 0;
                a_msg->tot = 0;
                nb_return = NBSendMsg(aNode.iTo, a_msg);
                if (nb_return == 0){
                    cout << "Message to " << aNode.iTo << " with value " << aNode.value
                         << " delivered from queue." << endl;
                    NBQueue.pop();
                }
            }
            nb_terminate = 1;
            continue;
        }

        a_msg -> iFrom = 0;
        a_msg -> value = value;
        a_msg -> cnt = 0;
        a_msg -> tot = 0;

        if (nb)
            nb_return = NBSendMsg(destination, a_msg);
        else
            SendMsg(destination, a_msg);

        if (nb_return == -1){
            cout << "Message to " << destination << " with value " << value
            << " could not be delivered. Queued." << endl;
            struct msg_node aNode(destination, value);
            NBQueue.push(aNode);
        }
    }
}
