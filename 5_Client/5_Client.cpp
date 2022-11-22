#define _CRT_SECURE_NO_WARNINGS // ���� C �Լ� ��� �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // ���� ���� API ��� �� ��� ����

#include <WS2tcpip.h>
#include <iostream>
#include <fstream>

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

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    1024

int main(int argc, char* argv[])
{
	int retval;

	// ����� �μ��� ������ IP �ּҷ� ���
	if (argc > 1) SERVERIP = argv[1];

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// ������ ��ſ� ����� ����
	char name[50];
	int len;
	char buf[BUFSIZE];
	ifstream fin;

	if (argc > 1) {													//�μ��� ������ �����̸� ����
		cout << argv[2] << endl;
		string pullPath = argv[2];
		string a = "\\";
		int find = pullPath.rfind(a) + 1;
		strcpy(name, pullPath.substr(find, pullPath.length() - find).c_str());
	}
	else
		strcpy(name, "data.mp4");

	if (argc > 1)
		fin.open(argv[2], ios::binary);		// ���̳ʸ� Ÿ������ ���� ���� ����
	else
		fin.open(name, ios::binary);		// ���̳ʸ� Ÿ������ ���� ���� ����

	fin.seekg(0, ios::end);					// ������ ������ ����
	len = fin.tellg();						// ���� ũ�� �о����
	fin.seekg(0, ios::beg);					// �ٽ� ó������


	// ������ ������(���� �̸�)
	retval = send(sock, name, sizeof(name), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		//break;
	}

	// ������ ������(���� ����)				// ���� ũ�� ������
	retval = send(sock, (char*)&len, sizeof(int), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		//break;
	}

	// ������ ������(���� ����)
	int total_size = 0;
	for (int i = 0; i < len / BUFSIZE + 1; ++i) {
		//fin.seekg(i * BUFSIZE, ios::beg);				// ���� ������ �����̸鼭
		
		int size;
		if (i == len / BUFSIZE)
			size = len - len / BUFSIZE * BUFSIZE;
		else
			size = BUFSIZE;

		fin.read(buf, size);							// BUFSIZE == 1KB�� �о

		retval = send(sock, buf, size, 0);				// ������
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			cout << "error" << endl;
			system("pause");
			exit(1);
			break;
		}

		total_size += size;

		if (i % 10000 == 0 || i == len / BUFSIZE) {
			cout << "\r����� : " << (int)((float)total_size / len * 100) << " %";
		}

		//cout << len << "�� " << total_size << " ����" << endl;
	}
	cout << endl;
	fin.close();

	//printf("[TCP Ŭ���̾�Ʈ] %d+%d����Ʈ�� "
	//	"���½��ϴ�.\n", (int)sizeof(int), retval);


	// ���� �ݱ�
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}
