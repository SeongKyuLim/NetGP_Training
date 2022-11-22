#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기

#include <WS2tcpip.h>
#include <iostream>
#include <string.h>

#pragma comment(lib, "ws2_32") // ws2_32.lib 링크

using namespace std;

char** GetIPbyName(const char* name)
{
	struct hostent* ptr = gethostbyname(name);

	if (ptr == NULL) {
		cout << "데이터를 받아오는데 실패하였습니다." << endl;
		return NULL;
	}

	cout << "Official Domain : " << ptr->h_name << endl;

	cout << "\nIP Addr List" << endl;

	for (int i = 0; ptr->h_addr_list[i] != NULL; ++i) {
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, ptr->h_addr_list[i], str, sizeof(str));
		cout << "IP주소 " << i + 1 << " : " << str << endl;
	}

	cout << "\nHost Aliases List" << endl;

	for (int i = 0; ptr->h_aliases[i] != NULL; ++i) {
		cout << "별칭 " << i + 1 << " : " << ptr->h_aliases[i] << endl;
	}

	return ptr->h_addr_list;
}

void GetIPbyAddr(char** list)
{
	if (list == NULL)
		return;

	for (int i = 0; list[i] != NULL; ++i) {		
		struct hostent* ptr = gethostbyaddr((const char*)list[i], 4, AF_INET);

		char temp[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, list[i], temp, sizeof(temp));

		cout << i + 1 << "번 : " << temp << " IP에 대하여 검증 결과\n" << endl;
		
		if (ptr == NULL) {
			cout << "데이터를 받아오는데 실패하였습니다.\n" << endl;
			continue;
		}

		cout << "Official Domain : " << ptr->h_name << endl;

		cout << "\nIP Addr List" << endl;

		for (int i = 0; ptr->h_addr_list[i] != NULL; ++i) {
			char str[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, ptr->h_addr_list[i], str, sizeof(str));
			cout << "IP주소 " << i + 1 << " : " << str << endl;
		}

		cout << "\nHost Aliases List" << endl;

		for (int i = 0; ptr->h_aliases[i] != NULL; ++i) {
			cout << "별칭 " << i + 1 << " : " << ptr->h_aliases[i] << endl;
		}

	}
}

int main(int argc, char* argv[])
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	char** addrlist = GetIPbyName(argv[1]);

	cout << "\n\n\n";

	cout << "-------------------검증 1번-------------------" << endl;
	GetIPbyAddr(addrlist);
	cout << "-------------------검증 2번-------------------" << endl;

	char str[100] = "nslookup ";
	strcat(str, argv[1]);

	system(str);

	// 윈속 종료
	WSACleanup();
	return 0;
}
