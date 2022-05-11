#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include<stdlib.h>
#include <cstring>
# include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "structures.h"

using namespace std;

int db_msg_queue(int, MESSAGE&);
int md5_sem(int, MESSAGE&);

int main()
{
	int accepted_sock;
	int sock = -1;
	struct sockaddr_in addr;
	MESSAGE mes_struct;

	unsigned int bytes_read = 0;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		cout << "SERVER: There is an error with openning socket." << endl;
		exit(-1);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(1234);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		cout << "SERVER: There is an error with binding sockets." << endl;
		close(sock);
		exit(-1);
	}

	bool stop_flag = 0;

	while (1)
	{
		cout << "Listening..." << endl;
		int status = listen(sock, 5);
		if (status != 0)
		{
			cout << "SERVER: There is an error with listenning sockets." << endl;
			close(sock);
			exit(-1);
		}

		accepted_sock = accept(sock, NULL, NULL);
		if (accepted_sock == -1)
		{
			cout << "SERVER: There is an error with accepting sockets." << endl;
			close(sock);
			exit(-1);
		}
		//cout << "Access Socket ID: " << accepted_sock << endl;

		if (fork() == 0)
		{
			cout << "Recieving..." << endl;
			close(sock);
			bytes_read = recv(accepted_sock, (MESSAGE*)&mes_struct, sizeof(MESSAGE), 0);
			cout << "Recieved..." << endl;

			if (bytes_read <= 0)
			{
				cout << "SERVER: There is an error with receiving." << endl;
				close(accepted_sock);
				exit(-1);
			}

			if (mes_struct.type == MESSAGE::MD5) // общение с сервером вычисл€ющим хеш на семафорах --------------------------------------------------------------------------
			{
				md5_sem(accepted_sock, mes_struct);
			}
			else if(mes_struct.type == MESSAGE::DATABASE) // база данных на очереди сообщений  -----------------------------------------------------------------------------------------------------------------------
			{
				db_msg_queue(accepted_sock, mes_struct);
			}

			//cout << "data: " << mes_struct.data << endl;
			//
			//mes_struct.data[0] = '0';

			//cout << "Sending..." << endl;


			//if (send(accepted_sock, (MESSAGE*)&mes_struct, sizeof(mes_struct), 0) == -1)
			//{
			//	cout << "SERVER: There is an error with sending message." << endl;
			//}
			//cout << "Sent." << endl;
			return 0;
		}
		else
		{
			close(accepted_sock);
		}
	}
}

int db_msg_queue(int accepted_sock, MESSAGE& mes_struct)
{
	int msgqid, rc;
	key_t msgkey;
	msgkey = ftok("string_for_identical_keys_queue", 'm');
	char text[128];
	int bytes_read;

	msgqid = msgget(msgkey, IPC_CREAT | 0660);
	if (msgqid < 0)
	{
		perror(strerror(errno));
		cout << "Failed to create message queue with msgqid:" << msgqid << endl;
		strcpy(mes_struct.data, "Failed to create message queue with msgqid:\n");
		if (send(accepted_sock, (MESSAGE*)&mes_struct, sizeof(mes_struct), 0) == -1)
		{
			cout << "SERVER: There is an error with sending message." << endl;
		}
		return 1;
	}
	else
		printf("Message queue %d created\n", msgqid); // TODO: возможно стоит убрать это

	cout << "SERVER: The connection to the database is established" << endl;
	strcpy(mes_struct.data, "The connection to the database is established.\n To exit, enter: quit!\n");

	if (send(accepted_sock, (MESSAGE*)&mes_struct, sizeof(mes_struct), 0) == -1)
	{
		cout << "SERVER: There is an error with sending message." << endl;
	}
	cout << "Sent." << endl;

	while (1) // цикл общени€ с бд
	{
		strcpy(mes_struct.data, ">>>"); // сделаем строку, котора€ предлагает ввести команду

		if (send(accepted_sock, (MESSAGE*)&mes_struct, sizeof(mes_struct), 0) == -1) // отправим клиенту
		{
			cout << "SERVER: There is an error with sending message." << endl;
		}
		cout << "Sent." << endl;

		cout << "Sending..." << endl;

		cout << "Recieving..." << endl;
		bytes_read = recv(accepted_sock, (MESSAGE*)&mes_struct, sizeof(MESSAGE), 0); // получим от клиента строку 
		cout << "Recieved..." << endl;

		if (bytes_read <= 0)
		{
			cout << "SERVER: There is an error with receiving." << endl;
			close(accepted_sock);
			exit(-1);
		}
		msg.mtype = getpid();
		strcpy(msg.mtext, mes_struct.data);

		if (strcmp(text, "stop\n") == 0)
		{
			break;
		}

		rc = msgsnd(msgqid, &msg, sizeof(msg) - sizeof(long), 0);

		if (rc < 0)
		{
			printf("msgsnd failed, rc = %d\n", rc);
			return 1;
		}
		else
			printf("Message send done!\n");

		if (strcmp(text, "stop the server\n") == 0)
		{
			break;
		}

		// получим 
		rc = msgrcv(msgqid, &msg, sizeof(msg) - sizeof(long), getpid(), 0); // посмотреть как узнать измениласи или нет строка.
		if (rc < 0)
		{
			perror(strerror(errno));
			printf("msgrcv failed, rc=%d\n", rc);
			break;
		}
		printf("received from server msg: %s\n", msg.mtext);

		strcpy(mes_struct.data, msg.mtext);

		if (mes_struct.data[0] == '\0')
			continue;

		if (send(accepted_sock, (MESSAGE*)&mes_struct, sizeof(mes_struct), 0) == -1) // отправим клиенту
		{
			cout << "SERVER: There is an error with sending message." << endl;
		}
	}
	return 0;
}

int md5_sem(int accepted_sock, MESSAGE& mes_struct)
{
	int semkey = ftok(".", 'm');

	int bytes_read;


	if (semkey < 0)
	{
		cout << "Wrong key" << endl;
		return 1;
	}

	int shmkey = ftok(".", 'm');
	if (shmkey < 0)
	{
		cout << "Wrong key" << endl;
		return 1;
	}

	int semid = semget(semkey, 2, 0666);
	if (semid == -1)
	{
		cout << "Semaphore does not exist" << endl;
		return 1;
	}

	int shmid = shmget(shmkey, sizeof(SEM_STRUCT), 0666);
	if (shmid == -1)
	{
		cout << "Shared memory does not exist" << endl;
		return 1;
	}

	SEM_STRUCT* shm_address = (SEM_STRUCT*)shmat(shmid, NULL, 0);
	if (shm_address == NULL)
	{
		cout << "Can't get acess to shared memory" << endl;
		return 1;
	}

	strcpy(mes_struct.data, "MD5 is ready\n");

	if (send(accepted_sock, (MESSAGE*)&mes_struct, sizeof(mes_struct), 0) == -1)
	{
		cout << "SERVER: There is an error with sending message." << endl;
	}

	cout << "semid netserver: " << semid << endl;

	char line[BUFFSIZE] = "123456";
	struct sembuf operations[2];

	while (1)
	{

		bytes_read = recv(accepted_sock, (MESSAGE*)&mes_struct, sizeof(MESSAGE), 0); // получим от клиента строку 
		cout << "Recieved..." << endl;

		if (bytes_read <= 0)
		{
			cout << "SERVER: There is an error with receiving." << endl;
			close(accepted_sock);
			exit(-1);
		}

		//cout << "netserver received the following line:" << mes_struct.data << endl;

		operations[0].sem_num = 0;
		operations[0].sem_op = 0;
		operations[0].sem_flg = 0;

		operations[1].sem_num = 1;
		operations[1].sem_op = -1;
		operations[1].sem_flg = 0;


		if (semop(semid, operations, 2) == -1) // set value 
		{
			if (strcmp(line, "\n") != 0)
				cout << "Can't do oper for sem" << endl;
			break;
		}

		strcpy(shm_address->data, mes_struct.data); // copy str // строчка в server_md5.cpp
		shm_address->count_bytes = mes_struct.count_byte;

		operations[0].sem_num = 0;
		operations[0].sem_op = 1;
		operations[0].sem_flg = SEM_UNDO;

		operations[1].sem_num = 1;
		operations[1].sem_op = 0;
		operations[1].sem_flg = SEM_UNDO;

		if (semop(semid, operations, 2) == -1)
		{
			if (strcmp(line, "\n") != 0)
				cout << "Can't do oper for sem" << endl;
			return 1;
		}

		operations[0].sem_num = 0;
		operations[0].sem_op = -1;
		//operations[0].sem_flg = 0;

		operations[1].sem_num = 1;
		operations[1].sem_op = -1;
		//operations[1].sem_flg = 0;

		if (semop(semid, operations, 2) == -1) // set value 
		{
			if (strcmp(line, "\n") != 0)
				cout << "Can't do oper for sem" << endl;
			return 1;
		}

		if (mes_struct.count_byte < 64)
		{
			strcpy(mes_struct.data, shm_address->data);// отправка хеша
			if (send(accepted_sock, (MESSAGE*)&mes_struct, sizeof(mes_struct), 0) == -1) // отправим клиенту
			{
				cout << "SERVER: There is an error with sending message." << endl;
			}
			cout << "Sent." << endl;
			break;
		}

		operations[0].sem_num = 0;
		operations[0].sem_op = 0;
		//operations[0].sem_flg = IPC_NOWAIT;

		operations[1].sem_num = 1;
		operations[1].sem_op = 1;
		//operations[1].sem_flg = IPC_NOWAIT;

		if (semop(semid, operations, 2) == -1)
		{
			if (strcmp(line, "\n") != 0)
				cout << "Can't do oper for sem" << endl;
			return 1;
		}
	}
	return 0;
}