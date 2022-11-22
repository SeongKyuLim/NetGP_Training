#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기

//서버는 6번과제와 동일하다

#include <WS2tcpip.h>
#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include <fstream>

#include "resource.h"

#pragma comment(lib, "ws2_32") // ws2_32.lib 링크

using namespace std;

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    1024

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

// 대화상자 프로시저
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// 에디트 컨트롤 출력 함수
void DisplayText(const char *fmt, ...);
// 소켓 함수 오류 출력
void DisplayError(const char *msg);
// 소켓 통신 스레드 함수
DWORD WINAPI ClientMain(LPVOID hDlg);

SOCKET sock; // 소켓
char buf[BUFSIZE]; // 데이터 송수신 버퍼
HWND hSelectButton, hSendButton; // 보내기 버튼
HWND hEdit; // 에디트 컨트롤
HWND hProgress;	//프로그레스바

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 대화상자 생성
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	// 윈속 종료
	WSACleanup();
	return 0;
}

// 대화상자 프로시저
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:			//생성시
		hEdit = GetDlgItem(hDlg, IDC_EDIT1);
		hSelectButton = GetDlgItem(hDlg, IDC_BUTTON1);
		hSendButton = GetDlgItem(hDlg, IDOK);
		hProgress = GetDlgItem(hDlg, IDC_PROGRESS1);
		SendMessage(hProgress, PBM_SETBARCOLOR, 0, RGB(0, 255, 0));
		SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 10000));//프로그레스 초기화
		SendMessage(hProgress, PBM_SETPOS, 0, 0);//프로그레스 초기값
		SendMessage(hEdit, EM_SETLIMITTEXT, BUFSIZE, 0);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			CreateThread(NULL, 0, ClientMain, hDlg, 0, NULL);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL); // 대화상자 닫기
			closesocket(sock); // 소켓 닫기
			return TRUE;
		case IDC_BUTTON1:		//이거누르면 파일선택기 열려야 함
								//그리고 파일 선택되면 hEdit에 입력
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

// 에디트 컨트롤 출력 함수
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

// 소켓 함수 오류 출력
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

// TCP 클라이언트 시작 부분
DWORD WINAPI ClientMain(LPVOID hDlg)
{
	EnableWindow(hSendButton, FALSE); // 보내기 버튼 비활성화

	int retval;

	// 소켓 생성
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

	// 데이터 통신에 사용할 변수
	char path[MAX_PATH];
	char name[50];
	int len;
	char buf[BUFSIZE];
	ifstream fin;

	GetDlgItemTextA((HWND)hDlg, IDC_EDIT1, path, sizeof(path));			// 에딧에서 파일경로 가져온다.

	string pullPath = path;														//파일에서 이름 추출
	string a = "\\";
	int find = pullPath.rfind(a) + 1;
	strcpy(name, pullPath.substr(find, pullPath.length() - find).c_str());


	fin.open(path, ios::binary);		// 바이너리 타입으로 보낼 파일 열기

	fin.seekg(0, ios::end);					// 파일의 끝으로 가서
	len = fin.tellg();						// 파일 크기 읽어오고
	fin.seekg(0, ios::beg);					// 다시 처음으로


	// 데이터 보내기(파일 이름)
	retval = send(sock, name, sizeof(name), 0);
	if (retval == SOCKET_ERROR) {
		DisplayError("send()");
		return 1;
	}

	// 데이터 보내기(고정 길이)				// 파일 크기 보내줌
	retval = send(sock, (char*)&len, sizeof(int), 0);
	if (retval == SOCKET_ERROR) {
		DisplayError("send()");
		return 1;
	}

	// 데이터 보내기(가변 길이)
	int total_size = 0;
	for (int i = 0; i < len / BUFSIZE + 1; ++i) {
		int size;
		if (i == len / BUFSIZE)
			size = len - len / BUFSIZE * BUFSIZE;
		else
			size = BUFSIZE;

		fin.read(buf, size);							// BUFSIZE == 1KB씩 읽어서

		retval = send(sock, buf, size, 0);				// 보낸다
		if (retval == SOCKET_ERROR) {
			DisplayError("send()");
			return 1;
		}

		total_size += size;

		if (i % 1000 == 0 || i == len / BUFSIZE) {
			SendMessage(hProgress, PBM_SETPOS, (int)((float)total_size / len * 10000), 0);
			//cout << "\r진행률 : " << (int)((float)total_size / len * 100) << " %";
		}
	}
	fin.close();

	// 소켓 닫기
	closesocket(sock);

	EnableWindow(hSendButton, TRUE); // 보내기 버튼 활성화

	// 윈속 종료
	return 0;
}
