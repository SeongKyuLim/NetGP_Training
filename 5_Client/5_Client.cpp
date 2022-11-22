#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기

#include <WS2tcpip.h>
#include <iostream>
#include <fstream>

#pragma comment(lib, "ws2_32") // ws2_32.lib 링크

// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
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

// 소켓 함수 오류 출력
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[오류] %s\n", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}


using namespace std;

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    1024

int main(int argc, char* argv[])
{
	int retval;

	// 명령행 인수가 있으면 IP 주소로 사용
	if (argc > 1) SERVERIP = argv[1];

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
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

	// 데이터 통신에 사용할 변수
	char name[50];
	int len;
	char buf[BUFSIZE];
	ifstream fin;

	if (argc > 1) {													//인수로 들어오면 파일이름 추출
		cout << argv[2] << endl;
		string pullPath = argv[2];
		string a = "\\";
		int find = pullPath.rfind(a) + 1;
		strcpy(name, pullPath.substr(find, pullPath.length() - find).c_str());
	}
	else
		strcpy(name, "data.mp4");

	if (argc > 1)
		fin.open(argv[2], ios::binary);		// 바이너리 타입으로 보낼 파일 열기
	else
		fin.open(name, ios::binary);		// 바이너리 타입으로 보낼 파일 열기

	fin.seekg(0, ios::end);					// 파일의 끝으로 가서
	len = fin.tellg();						// 파일 크기 읽어오고
	fin.seekg(0, ios::beg);					// 다시 처음으로


	// 데이터 보내기(파일 이름)
	retval = send(sock, name, sizeof(name), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		//break;
	}

	// 데이터 보내기(고정 길이)				// 파일 크기 보내줌
	retval = send(sock, (char*)&len, sizeof(int), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		//break;
	}

	// 데이터 보내기(가변 길이)
	int total_size = 0;
	for (int i = 0; i < len / BUFSIZE + 1; ++i) {
		//fin.seekg(i * BUFSIZE, ios::beg);				// 파일 포인터 움직이면서
		
		int size;
		if (i == len / BUFSIZE)
			size = len - len / BUFSIZE * BUFSIZE;
		else
			size = BUFSIZE;

		fin.read(buf, size);							// BUFSIZE == 1KB씩 읽어서

		retval = send(sock, buf, size, 0);				// 보낸다
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			cout << "error" << endl;
			system("pause");
			exit(1);
			break;
		}

		total_size += size;

		if (i % 10000 == 0 || i == len / BUFSIZE) {
			cout << "\r진행률 : " << (int)((float)total_size / len * 100) << " %";
		}

		//cout << len << "중 " << total_size << " 보냄" << endl;
	}
	cout << endl;
	fin.close();

	//printf("[TCP 클라이언트] %d+%d바이트를 "
	//	"보냈습니다.\n", (int)sizeof(int), retval);


	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
