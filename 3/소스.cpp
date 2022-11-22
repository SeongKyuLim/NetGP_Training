#define _CRT_SECURE_NO_WARNINGS // ���� C �Լ� ��� �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // ���� ���� API ��� �� ��� ����

#include <WS2tcpip.h>
#include <iostream>
#include <string.h>

#pragma comment(lib, "ws2_32") // ws2_32.lib ��ũ

using namespace std;

char** GetIPbyName(const char* name)
{
	struct hostent* ptr = gethostbyname(name);

	if (ptr == NULL) {
		cout << "�����͸� �޾ƿ��µ� �����Ͽ����ϴ�." << endl;
		return NULL;
	}

	cout << "Official Domain : " << ptr->h_name << endl;

	cout << "\nIP Addr List" << endl;

	for (int i = 0; ptr->h_addr_list[i] != NULL; ++i) {
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, ptr->h_addr_list[i], str, sizeof(str));
		cout << "IP�ּ� " << i + 1 << " : " << str << endl;
	}

	cout << "\nHost Aliases List" << endl;

	for (int i = 0; ptr->h_aliases[i] != NULL; ++i) {
		cout << "��Ī " << i + 1 << " : " << ptr->h_aliases[i] << endl;
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

		cout << i + 1 << "�� : " << temp << " IP�� ���Ͽ� ���� ���\n" << endl;
		
		if (ptr == NULL) {
			cout << "�����͸� �޾ƿ��µ� �����Ͽ����ϴ�.\n" << endl;
			continue;
		}

		cout << "Official Domain : " << ptr->h_name << endl;

		cout << "\nIP Addr List" << endl;

		for (int i = 0; ptr->h_addr_list[i] != NULL; ++i) {
			char str[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, ptr->h_addr_list[i], str, sizeof(str));
			cout << "IP�ּ� " << i + 1 << " : " << str << endl;
		}

		cout << "\nHost Aliases List" << endl;

		for (int i = 0; ptr->h_aliases[i] != NULL; ++i) {
			cout << "��Ī " << i + 1 << " : " << ptr->h_aliases[i] << endl;
		}

	}
}

int main(int argc, char* argv[])
{
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	char** addrlist = GetIPbyName(argv[1]);

	cout << "\n\n\n";

	cout << "-------------------���� 1��-------------------" << endl;
	GetIPbyAddr(addrlist);
	cout << "-------------------���� 2��-------------------" << endl;

	char str[100] = "nslookup ";
	strcat(str, argv[1]);

	system(str);

	// ���� ����
	WSACleanup();
	return 0;
}
