// ServicesDll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

DWORD g_dwServiceType;
SERVICE_STATUS_HANDLE g_hServiceStatus;
DWORD g_dwCurrState;
wchar_t buffer[1024] = {};

void debugOutput(const wchar_t * format,...)
{
	va_list argList;
	va_start(argList, format);
	vswprintf_s(buffer, format, argList);
	//swprintf_s(buffer, L"ServicesDll %s", buffer);
	OutputDebugString(buffer);
	va_end(argList);
}

int TellSCM(DWORD dwState, DWORD dwExitCode, DWORD dwProgress)
{
	SERVICE_STATUS srvStatus;
	srvStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
	srvStatus.dwCurrentState = g_dwCurrState = dwState;
	srvStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	srvStatus.dwWin32ExitCode = dwExitCode;
	srvStatus.dwServiceSpecificExitCode = 0;
	srvStatus.dwCheckPoint = dwProgress;
	srvStatus.dwWaitHint = 1000;
	return SetServiceStatus(g_hServiceStatus, &srvStatus);
}

void __stdcall ServiceHandler(DWORD dwControl)
{
	switch (dwControl)
	{
	case SERVICE_CONTROL_STOP:
		TellSCM(SERVICE_STOP_PENDING, 0, 1);
		Sleep(10);
		TellSCM(SERVICE_STOPPED, 0, 0);
		break;
	case SERVICE_CONTROL_PAUSE:
		TellSCM(SERVICE_PAUSE_PENDING, 0, 1);
		TellSCM(SERVICE_PAUSED, 0, 0);
		break;
	case SERVICE_CONTROL_CONTINUE:
		TellSCM(SERVICE_CONTINUE_PENDING, 0, 1);
		TellSCM(SERVICE_RUNNING, 0, 0);
		break;
	case SERVICE_CONTROL_INTERROGATE:
		TellSCM(g_dwCurrState, 0, 0);
		break;
	}
}

extern "C" __declspec(dllexport) void ServiceMain(int argc, wchar_t* argv[]) 
{
	//char szSvcName[MAX_PATH];
    //wcstombs(szSvcName, argv[0], sizeof szSvcName);
	swprintf_s(buffer, L"Service %s", argv[0]);
	//debugOutput(L"ServiceName->:%s", argv[0]);
	OutputDebugString(buffer);
	OutputDebugString(L"Service Start");
	g_hServiceStatus = RegisterServiceCtrlHandler(argv[0], (LPHANDLER_FUNCTION)ServiceHandler);
	if (g_hServiceStatus == NULL) {
		//swprintf_s(buffer,L"","");
		OutputDebugStringW(L"ServicesDll RegisterFailurl!");
	}
	else FreeConsole();
	TellSCM(SERVICE_START_PENDING, 0, 1); // 通知服务控制管理器该服务开始
	TellSCM(SERVICE_RUNNING, 0, 0);  // 通知服务控制管理器该服务运行
	debugOutput(L"Service OJBK");
	// 在D盘创建一个txt创建并写入服务名作为测试
	HANDLE hFile = CreateFile(L"D:\\TestSvchost.txt", GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD nTemp = 0;
		WriteFile(hFile, argv[0], lstrlenW(argv[0]), &nTemp, NULL);
		CloseHandle(hFile);
	}

	do {
		Sleep(100);
	} while (g_dwCurrState != SERVICE_STOP_PENDING && g_dwCurrState != SERVICE_STOPPED);
	return;
}