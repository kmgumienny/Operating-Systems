This code creates threads that act as mailboxes by utilizing semaphores. 

To create executable, navigate to directory and in terminal run "make"

There must be a text file in the directory. Each line in the .txt file contains an interger followed by a space, and followed by another integer. The first integer indicates the value to be sent and the second integer indicates the destination mailbox.
When a mailbox receives a message, it sleeps for 1 second. 

The game has two modes, non-NB mode and NB mode. The non-NB mode waits for a mailbox that is sleeping whereas the NB mode adds messages that cannot be delivered to a queue and continues to send messages until it reaches EOF. Then, it runs through the queue and sends undelivered messages.
 

To run the code in non-NB mode, run "./proj3 #mailboxes values.txt" in directory

To run the code in NB mode, run "./proj3 #mailboxes values.txt NB" 

To clean the code, simply run "make clean" in directory. 

For more information on the code, read threads19.pdf