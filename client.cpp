#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include<stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <fstream>
#include <string.h>
#include "structures.h"

#define SIZE 4294967295

using namespace std;

int main()
{
	MESSAGE mes_struct;
	int sock = -1;
	struct sockaddr_in addr;
	char server_ip[12] = "127.0.0.1";

	sock = socket(AF_INET, SOCK_STREAM, 0);
	//cout << "Socket ID : " << sock << endl;
	if (sock == -1)
	{
		cout << "CLIENT: There is a problem with making socket." << endl;
		exit(-1);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(1234);
	//addr.sin_addr.s_addr = inet_addr(ip.c_str());
	inet_pton(AF_INET, server_ip,
		&addr.sin_addr);


	string choice = "";
	cout << "Do you want MD5 or database(\"M\"-MD5/\"D\"-Database)?\n(\"quit!\" - quit)" << endl << ">>>";
	bool flag = 1;
	while (flag)
	{
		cin >> choice;
		flag = 0;
		if (choice == "M")
		{
			mes_struct.type = MESSAGE::MD5;
		}
		else if (choice == "D")
		{
			mes_struct.type = MESSAGE::DATABASE;
		}
		else if (choice == "quit!")
		{
			return 0;
		}
		else
		{
			cout << "Error, try again:";
			flag = 1;
		}
	}

	if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		cout << "CLIENT: There is a problem with connecting socket." << endl;
		if (close(sock) == -1)
		{
			cout << "CLIENT: There is error with closing socket." << endl;
		}
		exit(-1);
	}

	// TODO: тут надо будет начать цикл в котором, будет общение с сервером на сокетах, ведь там будет цикл для отсыла хешей или команд для бд

	cout << "Sending..." << endl;

	send(sock, (MESSAGE*)&mes_struct, sizeof(mes_struct), 0);
	cout << "Sent." << endl;
	recv(sock, (MESSAGE*)&mes_struct, sizeof(mes_struct), 0);
	cout << mes_struct.data << endl;

	// MD5 обработка
	if (choice == "M")
	{
		cout << "Do you want to cache a file or string: ";
		cin >> choice;

		if (choice == "file")
		{
			cout << "Enter the file name: ";
			cin >> choice;
			int  f_hesh = open("file.txt", ios::in);

			mes_struct.count_byte = read(f_hesh, mes_struct.data, 64);
			while (mes_struct.count_byte == 64)
			{
				send(sock, (MESSAGE*)&mes_struct, sizeof(mes_struct), 0);
				mes_struct.count_byte = read(f_hesh, mes_struct.data, 64);
			}
			send(sock, (MESSAGE*)&mes_struct, sizeof(mes_struct), 0);
			recv(sock, (MESSAGE*)&mes_struct, sizeof(mes_struct), 0); // получаем хеш
			cout << "Server_md5 returned: " << mes_struct.data << endl;
		}
		else if (choice == "string")
		{
			cout << "Enter a string to hash it\n>>>";
			getchar();
			getline(cin, choice);

			int i = 0;

			while (i < choice.length())
			{
				mes_struct.data[i] = choice[i];
				i++;
				if (i % 64 == 0)
				{
					mes_struct.count_byte = 64;
					send(sock, (MESSAGE*)&mes_struct, sizeof(mes_struct), 0);
				}
			}
			mes_struct.count_byte = i % 64;
			send(sock, (MESSAGE*)&mes_struct, sizeof(mes_struct), 0);
			recv(sock, (MESSAGE*)&mes_struct, sizeof(mes_struct), 0); // получаем хеш
			cout << "Server_md5 returned: " << mes_struct.data << endl;
		}
		else
		{
			cout << "Input error" << endl;
		}
	}
	else // База данных обработка
	{
		getchar();
		while (1)
		{
			recv(sock, (MESSAGE*)&mes_struct, sizeof(mes_struct), 0);
			cout << mes_struct.data;
			if (!strcmp(mes_struct.data, ">>>"))
			{
				string buf;

				getline(cin, buf);

				if (buf == "quit!")
				{
					break;
				}

				if (buf.size() > BUFFSIZE)
				{
					cout << "Слишком длинная строка" << endl;
					break;
				}
				strcpy(mes_struct.data, buf.c_str());

				send(sock, (MESSAGE*)&mes_struct, sizeof(mes_struct), 0);
			}
			else
			{
				cout << endl;
			}
		}
	}

	if (shutdown(sock, 2) == -1)
	{

		cout << "CLIENT: There is error with shutdowning socket." << endl;
		if (close(sock) == -1)
		{
			cout << "CLIENT: There is error with closing socket." << endl;
		}
		exit(-1);
	}

	if (close(sock) == -1)
	{
		cout << "CLIENT: There is error with closing socket." << endl;
		exit(-1);
	}
	return 0;
}
