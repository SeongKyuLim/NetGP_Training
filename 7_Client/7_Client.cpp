#define _CRT_SECURE_NO_WARNINGS // ���� C �Լ� ��� �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // ���� ���� API ��� �� ��� ����

//������ 6�������� �����ϴ�

#include <WS2tcpip.h>
#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include <fstream>

#include "resource.h"

#pragma comment(lib, "ws2_32") // ws2_32.lib ��ũ

using namespace std;

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    1024

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

// ��ȭ���� ���ν���
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// ����Ʈ ��Ʈ�� ��� �Լ�
void DisplayText(const char *fmt, ...);
// ���� �Լ� ���� ���
void DisplayError(const char *msg);
// ���� ��� ������ �Լ�
DWORD WINAPI ClientMain(LPVOID hDlg);

SOCKET sock; // ����
char buf[BUFSIZE]; // ������ �ۼ��� ����
HWND hSelectButton, hSendButton; // ������ ��ư
HWND hEdit; // ����Ʈ ��Ʈ��
HWND hProgress;	//���α׷�����

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ��ȭ���� ����
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	// ���� ����
	WSACleanup();
	return 0;
}

// ��ȭ���� ���ν���
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:			//������
		hEdit = GetDlgItem(hDlg, IDC_EDIT1);
		hSelectButton = GetDlgItem(hDlg, IDC_BUTTON1);
		hSendButton = GetDlgItem(hDlg, IDOK);
		hProgress = GetDlgItem(hDlg, IDC_PROGRESS1);
		SendMessage(hProgress, PBM_SETBARCOLOR, 0, RGB(0, 255, 0));
		SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 10000));//���α׷��� �ʱ�ȭ
		SendMessage(hProgress, PBM_SETPOS, 0, 0);//���α׷��� �ʱⰪ
		SendMessage(hEdit, EM_SETLIMITTEXT, BUFSIZE, 0);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			CreateThread(NULL, 0, ClientMain, hDlg, 0, NULL);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL); // ��ȭ���� �ݱ�
			closesocket(sock); // ���� �ݱ�
			return TRUE;
		case IDC_BUTTON1:		//�̰Ŵ����� ���ϼ��ñ� ������ ��
								//�׸��� ���� ���õǸ� hEdit�� �Է�
			TCHAR szFilePath[MAX_PATH] = { 0, };
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = NULL;
			ofn.lpstrFile = szFilePath;
			ofn.nMaxFile = sizeof(szFilePath);
			ofn.lpstrFilter = NULL;
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			if (::GetOpenFileName(&ofn) == false) return FALSE;

			char return_path[MAX_PATH];
			WideCharToMultiByte(CP_ACP, 0, ofn.lpstrFile, MAX_PATH, return_path, MAX_PATH, NULL, NULL);

			DisplayText(return_path);

			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

// ����Ʈ ��Ʈ�� ��� �Լ�
void DisplayText(const char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	char cbuf[BUFSIZE * 2];
	vsprintf(cbuf, fmt, arg);
	va_end(arg);

	int nLength = GetWindowTextLength(hEdit);
	SendMessage(hEdit, EM_SETSEL, 0, -1);
	SendMessageA(hEdit, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
}

// ���� �Լ� ���� ���
void DisplayError(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char *)&lpMsgBuf, 0, NULL);
	DisplayText("[%s] %s\r\n", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// TCP Ŭ���̾�Ʈ ���� �κ�
DWORD WINAPI ClientMain(LPVOID hDlg)
{
	EnableWindow(hSendButton, FALSE); // ������ ��ư ��Ȱ��ȭ

	int retval;

	// ���� ����
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// ������ ��ſ� ����� ����
	char path[MAX_PATH];
	char name[50];
	int len;
	char buf[BUFSIZE];
	ifstream fin;

	GetDlgItemTextA((HWND)hDlg, IDC_EDIT1, path, sizeof(path));			// �������� ���ϰ�� �����´�.

	string pullPath = path;														//���Ͽ��� �̸� ����
	string a = "\\";
	int find = pullPath.rfind(a) + 1;
	strcpy(name, pullPath.substr(find, pullPath.length() - find).c_str());


	fin.open(path, ios::binary);		// ���̳ʸ� Ÿ������ ���� ���� ����

	fin.seekg(0, ios::end);					// ������ ������ ����
	len = fin.tellg();						// ���� ũ�� �о����
	fin.seekg(0, ios::beg);					// �ٽ� ó������


	// ������ ������(���� �̸�)
	retval = send(sock, name, sizeof(name), 0);
	if (retval == SOCKET_ERROR) {
		DisplayError("send()");
		return 1;
	}

	// ������ ������(���� ����)				// ���� ũ�� ������
	retval = send(sock, (char*)&len, sizeof(int), 0);
	if (retval == SOCKET_ERROR) {
		DisplayError("send()");
		return 1;
	}

	// ������ ������(���� ����)
	int total_size = 0;
	for (int i = 0; i < len / BUFSIZE + 1; ++i) {
		int size;
		if (i == len / BUFSIZE)
			size = len - len / BUFSIZE * BUFSIZE;
		else
			size = BUFSIZE;

		fin.read(buf, size);							// BUFSIZE == 1KB�� �о

		retval = send(sock, buf, size, 0);				// ������
		if (retval == SOCKET_ERROR) {
			DisplayError("send()");
			return 1;
		}

		total_size += size;

		if (i % 1000 == 0 || i == len / BUFSIZE) {
			SendMessage(hProgress, PBM_SETPOS, (int)((float)total_size / len * 10000), 0);
			//cout << "\r����� : " << (int)((float)total_size / len * 100) << " %";
		}
	}
	fin.close();

	// ���� �ݱ�
	closesocket(sock);

	EnableWindow(hSendButton, TRUE); // ������ ��ư Ȱ��ȭ

	// ���� ����
	return 0;
}
