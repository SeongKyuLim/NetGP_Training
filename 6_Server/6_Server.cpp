#define _CRT_SECURE_NO_WARNINGS // ���� C �Լ� ��� �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // ���� ���� API ��� �� ��� ����

#include <WS2tcpip.h>
#include <iostream>
#include <fstream>
#include <vector>

#pragma comment(lib, "ws2_32") // ws2_32.lib ��ũ

// ���� �Լ� ���� ��� �� ����
void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// ���� �Լ� ���� ���
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// ���� �Լ� ���� ���
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[����] %s\n", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}


using namespace std;

#define SERVERPORT 9000
#define BUFSIZE    1024


class printProcess {
public:
	static int count;

	SOCKET client_sock;
	int addrlen;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];

	int myIndex;
	int prograss;
	bool isEnd;

	printProcess() = default;
	printProcess(SOCKET client_sock) : client_sock(client_sock) {
		addrlen = sizeof(clientaddr);
		getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		myIndex = count++;
		isEnd = false;
		prograss = 0;
	}
};
int printProcess::count = 0;

vector<printProcess> processList;

DWORD WINAPI outCommand(LPVOID arg)
{
	while (true) {
		system("cls");
		for (int i = 0; i < printProcess::count; ++i) {
			printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
				processList[i].addr, ntohs(processList[i].clientaddr.sin_port));
		}

		cout << endl;

		for (int i = 0; i < printProcess::count; ++i) {
			printf("IP �ּ�=%s, ��Ʈ ��ȣ=%d - ",
				processList[i].addr, ntohs(processList[i].clientaddr.sin_port));
			cout << "����� : " << processList[i].prograss << " %" << endl;
		}

		cout << endl;

		for (int i = 0; i < printProcess::count; ++i) {
			if(processList[i].isEnd)
				printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
					processList[i].addr, ntohs(processList[i].clientaddr.sin_port));
		}
		Sleep(50);
	}
}

// Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI ProcessClient(LPVOID arg)
{
	int myIndex = printProcess::count - 1;
	
	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char buf[BUFSIZE];

	char name[50];
	int len; // ���� ���� ������(���� ����)�ִ� int������
	ofstream fout;

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	while (1) {
		// ������ �ޱ�(���� �̸�)
		retval = recv(client_sock, name, sizeof(name), MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// ������ �ޱ�(���� ����)
		retval = recv(client_sock, (char*)&len, sizeof(int), MSG_WAITALL);
		if (len == -1) {
			cout << "�������� ������ �ƴմϴ�." << endl;
			exit(1);
		}
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// ���Ͽ���
		fout.open(name, ios::binary);

		// ������ �ޱ�(���� ����)
		int total_size = 0;
		for (int i = 0; i < len / BUFSIZE + 1; ++i) {
			int size;
			if (i == len / BUFSIZE)
				size = len - len / BUFSIZE * BUFSIZE;
			else
				size = BUFSIZE;

			retval = recv(client_sock, buf, size, MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			fout.write(buf, size);

			total_size += size;

			if (i % 10000 == 0 || i == len / BUFSIZE) {
				//cout << "\r����� : " << (int)((float)total_size / len * 100) << " %";
				processList[myIndex].prograss = (int)((float)total_size / len * 100);
			}
		}
		cout << endl;

		fout.close();
	}

	// ���� �ݱ�
	closesocket(client_sock);
	/*printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		addr, ntohs(clientaddr.sin_port));*/
	processList[myIndex].isEnd = true;
	return 0;
}

int main(int argc, char* argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	HANDLE hThread;

	hThread = CreateThread(NULL, 0, outCommand,
		(LPVOID)0, 0, NULL);
	if (hThread == NULL) { ; }
	else { CloseHandle(hThread); }

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		//// ������ Ŭ���̾�Ʈ ���� ���
		//char addr[INET_ADDRSTRLEN];
		//inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		//printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		//	addr, ntohs(clientaddr.sin_port));

		processList.push_back(printProcess(client_sock));

		// ������ ����
		hThread = CreateThread(NULL, 0, ProcessClient,
			(LPVOID)client_sock, 0, NULL);
		if (hThread == NULL) { closesocket(client_sock); }
		else { CloseHandle(hThread); }
	}

	// ���� �ݱ�
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}
