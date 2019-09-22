//
// Created by Kamil Gumienny on 9/21/19.
//

#ifndef PROJ3_MSG_H
#define PROJ3_MSG_H
struct msg {
    int iFrom; /* who sent the message (0 .. number-of-threads) */
    int value; /* its value */
    int cnt; /* count of operations (not needed by all msgs) */
    int tot; /* total time (not needed by all msgs) */
};
#endif //PROJ3_MSG_H
