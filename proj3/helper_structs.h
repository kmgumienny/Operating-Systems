//
// Created by Kamil Gumienny on 9/21/19.
//

#ifndef PROJ3_HELPER_STRUCTS_H
#define PROJ3_HELPER_STRUCTS_H
struct msg {
    int iFrom; /* who sent the message (0 .. number-of-threads) */
    int value; /* its value */
    int cnt; /* count of operations (not needed by all msgs) */
    int tot; /* total time (not needed by all msgs) */
};

struct msg_node {
    int iTo, value;
    msg_node(int iTo = -1, int value = -1){
        this->iTo = iTo;
        this->value = value;
    }
};


#endif //PROJ3_HELPER_STRUCTS_H
