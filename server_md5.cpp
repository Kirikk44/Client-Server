#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h> 
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "MD5.h"
#include "structures.h"

using namespace std;

int main()
{
	int semkey = ftok(".", 'm');
	MD5 md5;
	if (semkey < 0)
	{
		printf("Wrong key\n");
		return 1;
	}

	int shmkey = ftok(".", 'm');
	if (shmkey < 0)
	{
		cout << "Wrong key" << endl;
		return 1;
	}

	int semid = semget(semkey, 2, 0666 | IPC_CREAT /*| IPC_EXCL*/);
	if (semid == -1)
	{
		cout << "Can't create sem" << endl;
		return 1;
	}

	short sarray[2];
	sarray[0] = 0;
	sarray[1] = 1;
	if (semctl(semid, 1, SETALL, sarray) < 0)
	{
		cout << "Can't do init" << endl;
		return 1;
	}

	int shmid = shmget(shmkey, sizeof(SEM_STRUCT), 0666 | IPC_CREAT/* | IPC_EXCL*/);
	if (shmid < 0)
	{
		cout << "Can't get shared memory" << endl;
		return 1;
	}

	SEM_STRUCT* shm_address = (SEM_STRUCT*)shmat(shmid, NULL, 0);
	if (shm_address == NULL)
	{
		cout << "Can't get acess to shared memory" << endl;
		return 1;
	}
	cout << "The server is ready" << endl;

	struct sembuf operations[2];
	struct shmid_ds shmid_struct;
	while (1)
	{
		operations[0].sem_num = 0;
		operations[0].sem_op = -1;
		operations[0].sem_flg = 0;

		operations[1].sem_num = 1;
		operations[1].sem_op = 0;
		operations[1].sem_flg = 0;

		if (semop(semid, operations, 2) == -1)
		{
			cout << "Can't do oper for sem" << endl;
			break;
		}
		cout << "Client reseved : " << shm_address->data << endl;

		cout << "data: " << shm_address->data << "count" << shm_address->count_bytes << endl;
		
		md5.new_block(shm_address->data, shm_address->count_bytes);

		operations[0].sem_num = 0;
		operations[0].sem_op = 1;
		operations[0].sem_flg = SEM_UNDO;

		operations[1].sem_num = 1;
		operations[1].sem_op = 1;
		operations[1].sem_flg = SEM_UNDO;

		if (semop(semid, operations, 2) == -1)
		{
			cout << "Can't do oper for sem" << endl;;
			return 1;
		}
	}

	if (semctl(semid, 1, IPC_RMID) == -1)
	{
		cout << "Remove id failed" << endl;
		return 1;
	}


	if (shmdt(shm_address) == -1)
	{
		cout << "Shmdt failed" << endl;
		return 1;
	}


	if (shmctl(shmid, IPC_RMID, &shmid_struct) == -1)
	{
		cout << "Shmctl failed" << endl;
		return 1;
	}

	return 0;
}