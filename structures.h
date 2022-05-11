#include <iostream>

using namespace std;
#define BUFFSIZE 128

typedef struct
{
	char data[BUFFSIZE];
	int count_byte;
	enum type_task
	{
		DATABASE,
		MD5,
	};
    type_task type;
}MESSAGE;

typedef struct
{
	char data[BUFFSIZE];
	int count_bytes;
} SEM_STRUCT;

struct msg_buf
{
	long mtype;
	char mtext[BUFFSIZE];
} msg;