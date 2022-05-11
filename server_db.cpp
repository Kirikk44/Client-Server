#include <iostream>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include "structures.h"
#include "DBLibrary.h"

int main()
{
	int msgqid, rc;
	key_t msgkey;
	msgkey = ftok("string_for_identical_key_queue", 'm');
    char line[BUFFSIZE];

	msgqid = msgget(msgkey, IPC_CREAT | 0660);

    if (msgqid < 0)
    {
        perror(strerror(errno));
        printf("failed to create message queue with msgqid = %d\n", msgqid);
        return 1;
    }
    else
        cout << "The server is ready" << endl;

	DBLibrary db;

    while (1)
    {
        rc = msgrcv(msgqid, &msg, sizeof(msg) - sizeof(long), 0, 0);
        //printf("rc: %d\n", rc);
        if (rc < 0)
        {
            perror(strerror(errno));
            printf("msgrcv failed, rc=%d\n", rc);
            break;
        }
        printf("received msg: %s it's PID = %ld\n", msg.mtext, msg.mtype);

        strcpy(line, msg.mtext); // TODO: получили строчку, тут, наверное, стоит вызвать функцию обработку
        cout << "line: " << line << endl;
        strcpy(msg.mtext, db.new_command(string(line)).c_str());

        rc = msgsnd(msgqid, &msg, sizeof(msg) - sizeof(long), msg.mtype); // отправили обратно
        if (rc < 0)
        {
            printf("msgsnd failed, rc = %d\n", rc);
            return 1;
        }
        else
            printf("Message send done!\n");
    }
}

