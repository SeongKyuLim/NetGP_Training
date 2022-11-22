#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기

#include <WS2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32") // ws2_32.lib 링크

using namespace std;

class portList {
private:
	int port;
	
	static int count;
public:
	portList() {
		port = 0;
		++count;
	}
	portList(int num) {
		port = num;
		++count;
	}
	~portList() {
		--count;
	}

	static int isCount() {
		return count;
	}
	int isPort() {
		return port;
	}
};
int portList::count = 0;

int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	portList* list[10]{};

	cout << "호스트 IP : " << argv[1] << "\n\n";

	for (int i = stoi(argv[2]); i <= stoi(argv[3]); ++i) {		// 명령행에서 입력받은 포트 범위
		// 소켓 생성
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

		// connect()
		struct sockaddr_in serveraddr;
		memset(&serveraddr, 0, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		inet_pton(AF_INET, argv[1], &serveraddr.sin_addr);		// 입력한 서버
		serveraddr.sin_port = htons(i);							//i번째 포트 호출

		retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

		if (retval == SOCKET_ERROR) {
			cout << i << " 포트 호출 실패" << endl;
		}
		else {
			cout << i << " 포트 호출 성공" << endl;
			list[portList::isCount() - 1] = new portList(i);
		}

		// 소켓 닫기
		closesocket(sock);
	}

	// 성공한 포트 개수 출력
	cout << "\nLISTENING 상태인 포트번호 : ";
	for (int i = 0; i < portList::isCount(); ++i) {
		cout << list[i]->isPort() << " ";
	}
	cout << endl;
	cout << portList::isCount() << " 개 성공" << endl;

	// 윈속 종료
	WSACleanup();

	cout << "\n검증----------------------------\n" << endl;
	system("netstat -anp tcp | findstr LISTENING");


	return 0;
}
