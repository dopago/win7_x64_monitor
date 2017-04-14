#include "HookApi.h"

typedef long (__fastcall *pfnRtlAdjustPrivilege64)(ULONG,ULONG,ULONG,PVOID);
pfnRtlAdjustPrivilege64 RtlAdjustPrivilege;

char * GetProcessPath(){
	DWORD pid=(DWORD)_getpid();
	HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,pid);
	GetModuleFileNameEx(hProcess,NULL,pathname,MAX_PATH);
	int iLength ;
	iLength = WideCharToMultiByte(CP_ACP, 0, pathname, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, pathname, -1, path, iLength, NULL, NULL);
	//OpenProcess֮��һ��Ҫ��סclose
	CloseHandle(hProcess);
	return path;
}

char * LogTime(){
	time_t t=time(NULL);
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	sprintf_s(tim,"%4d-%02d-%02d-%02d-%02d-%02d-%03d:\0",sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
	return tim;
}

//���ַ�ת��Ϊ���ֽ�
string WideToMutilByte(const wstring& _src)
{
	if (&_src==NULL)
	{
		return "NULL";
	}
	int nBufSize = WideCharToMultiByte(GetACP(), 0, _src.c_str(),-1, NULL, 0, NULL, NULL);
	char *szBuf = new char[nBufSize];
	WideCharToMultiByte(GetACP(), 0, _src.c_str(),-1, szBuf, nBufSize, NULL, NULL);
	string strRet(szBuf);
	delete []szBuf;
	szBuf = NULL;
	return strRet;
}

char * GetDate(){
	time_t t=time(NULL);
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	sprintf_s(dat,"%4d%02d%02d",sys.wYear,sys.wMonth,sys.wDay);
	return dat;
}

void GetProcessName(char* szProcessName,int* nLen){
	DWORD dwProcessID = GetCurrentProcessId();  
	HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,dwProcessID);   
	if(hProcess)  
	{  
		HMODULE hMod;  
		DWORD   dwNeeded;   
		if(EnumProcessModules(hProcess,&hMod,sizeof(hMod),&dwNeeded))  
		{  
			GetModuleBaseNameA(hProcess,hMod,szProcessName,*nLen);  
		}  
	}
	CloseHandle(hProcess);
}

std::string GetKeyPathFromKKEY(HKEY key)
{
	std::wstring keyPath;
	if (key==NULL)
	{
		return "NULL";
	}
	if (key != NULL)
	{
		HMODULE dll = LoadLibrary(L"ntdll.dll");
		if (dll != NULL) {
			typedef DWORD (__stdcall *ZwQueryKeyType)(
				HANDLE  KeyHandle,
				int KeyInformationClass,
				PVOID  KeyInformation,
				ULONG  Length,
				PULONG  ResultLength);

			ZwQueryKeyType func = reinterpret_cast<ZwQueryKeyType>(::GetProcAddress(dll, "ZwQueryKey"));

			if (func != NULL) {
				DWORD size = 0;
				DWORD result = 0;
				result = func(key, 3, 0, 0, &size);
				if (result == STATUS_BUFFER_TOO_SMALL)
				{
					size = size + 2;
					wchar_t* buffer = new (std::nothrow) wchar_t[size];
					if (buffer != NULL)
					{
						result = func(key, 3, buffer, size, &size);
						if (result == STATUS_SUCCESS)
						{
							buffer[size / sizeof(wchar_t)] = L'\0';
							keyPath = std::wstring(buffer + 2);
						}

						delete[] buffer;
					}
				}
			}

			FreeLibrary(dll);
		}
	}
	return WideToMutilByte(keyPath);
}

char * GetIPbySocket(SOCKET s){
	char *sock_ip;
	sockaddr_in sock;
	int socklen=sizeof(sock);
	//char sock_ip[]="NULL";
	//char sock_ip[1000]="NULL";
	getsockname(s,(struct sockaddr*)&sock,&socklen);
	sock_ip=inet_ntoa(sock.sin_addr);
	return sock_ip;
}

int EnableDebugPriv(const char* name)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	LUID luid;
	//�򿪽������ƻ�
	OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken);
	//��ý��̱���ΨһID
	if(!LookupPrivilegeValueA(NULL,name,&luid))
	{
		//printf("LookupPrivilegeValueAʧ��");
	}
	tp.PrivilegeCount=1;
	tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
	tp.Privileges[0].Luid=luid;
	//����Ȩ��
	if(!AdjustTokenPrivileges(hToken,0,&tp,sizeof(TOKEN_PRIVILEGES),NULL,NULL))
	{
		//printf("AdjustTokenPrivilegesʧ��");
	}
	CloseHandle(hToken);
	return 0;
}

BOOL InjectModuleToProcessById (const char *DllFullPath, DWORD dwProcessId )
{
	ofstream f("C:/test.txt",ios::app);
	f<<dwProcessId<<"\n";

    DWORD    dwRet = 0 ;
    BOOL    bStatus = FALSE ;
    LPVOID    lpData = NULL ;
    UINT    uLen = strlen(DllFullPath) + 1;
	DWORD                  dwRetVal    = 0;
#ifdef _WIN64   // x64 OpenProcess��Ȩ����
 // x64 OpenProcess��Ȩ����
	LPTHREAD_START_ROUTINE FuncAddress = NULL;
    DWORD  dwSize = 0;
    TCHAR* VirtualAddress = NULL;
    //Ԥ���룬֧��Unicode
#ifdef _UNICODE
    FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(L"Kernel32"), "LoadLibraryW");
#else
    FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle("Kernel32"), "LoadLibraryA");
#endif

    if (FuncAddress==NULL)
    {

        return FALSE;
    }
     RtlAdjustPrivilege=(pfnRtlAdjustPrivilege64)GetProcAddress((HMODULE)(FuncAddress("ntdll.dll")),"RtlAdjustPrivilege");

    if (RtlAdjustPrivilege==NULL)
    {
		f<<"flag 4 failed";
        return FALSE;
    }
	f<<"flag 4 success\n";
        
    RtlAdjustPrivilege(20,1,0,&dwRetVal);  //19
	f<<"flag 5 success\n";
#endif
    // ��Ŀ�����
	//EnableDebugPriv("SeDebugPrivilege");
    HANDLE hProcess = OpenProcess ( PROCESS_ALL_ACCESS, FALSE, dwProcessId ) ;
	//LPCWCH path1;
	//char buf[MAX_PATH];
	//GetModuleFileNameEx(hProcess, NULL, (LPSTR)path1, MAX_PATH);
	//int iLength ;
	//iLength = WideCharToMultiByte(CP_ACP, 0, path1, -1, NULL, 0, NULL, NULL);
	//WideCharToMultiByte(CP_ACP, 0, path1, -1, buf, iLength, NULL, NULL);
	//f<<buf<<endl;
    if ( hProcess )
    {
        // ����ռ�
		f<<"flag 6 success\n";
        lpData = VirtualAllocEx ( hProcess, NULL, uLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE ) ;
        if ( lpData )
        {
			f<<"flag 7 success\n";
            // д����Ҫע���ģ��·��ȫ��
            bStatus = WriteProcessMemory ( hProcess, lpData, DllFullPath, uLen, NULL) ;
        }
        CloseHandle ( hProcess ) ;
    }

    if ( bStatus == FALSE )
	{
		f<<"flag 8 failed";
        return FALSE ;
	}
	f<<"flag 8 success\n";
    // �����߳̿���
    THREADENTRY32 te32 = { sizeof(THREADENTRY32) } ;
    HANDLE hThreadSnap = CreateToolhelp32Snapshot ( TH32CS_SNAPTHREAD, 0 ) ;
    if ( hThreadSnap == INVALID_HANDLE_VALUE )
	{
		f<<"flag 9 failed";
        return FALSE ; 
	}
	f<<"flag 9 success\n";
    bStatus = FALSE ;
    // ö�������߳�
    if ( Thread32First ( hThreadSnap, &te32 ) )
    {
		f<<"flag 10 success\n";
        do{
            // �ж��Ƿ�Ŀ������е��߳�
            if ( te32.th32OwnerProcessID == dwProcessId )
            {
                // ���߳�
                HANDLE hThread = OpenThread ( THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID ) ;
                if ( hThread )
                {
					f<<"flag 11 success\n";
                    // ��ָ���߳�����APC
                    DWORD dwRet = QueueUserAPC ( (PAPCFUNC)LoadLibraryA, hThread, (ULONG_PTR)lpData ) ;
                    if ( dwRet > 0 )
					{
						f<<"flag 12 success\n";
                        bStatus = TRUE ;
					}
                    CloseHandle ( hThread ) ;
                }
            } 

        }while ( Thread32Next ( hThreadSnap, &te32 ) ) ;
    }
	f.close();
    CloseHandle ( hThreadSnap ) ;
    return bStatus;
}

BOOL InjectDll(const char *DllFullPath,const DWORD dwRemoteProcessId)
{
	HANDLE hRemoteProcess;
	EnableDebugPriv("SeDebugPrivilege");
	//��Զ���߳�
	hRemoteProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE,dwRemoteProcessId);
	if (hRemoteProcess==NULL)
	{
		return FALSE;
	}
	char *pszLibFileRemote;
	//ʹ��VirtualAllocEx������Զ�̽��̵��ڴ��ַ�ռ����DLL�ļ����ռ�
	pszLibFileRemote=(char *)VirtualAllocEx(hRemoteProcess,NULL,lstrlenA(DllFullPath)+1,MEM_COMMIT,PAGE_READWRITE);
	if (pszLibFileRemote==NULL)
	{
		CloseHandle(hRemoteProcess);
		return FALSE;
	}
	//ʹ��WriteProcessMemory������DLL��·����д�뵽Զ�̽��̵��ڴ�
	WriteProcessMemory(hRemoteProcess,pszLibFileRemote,(void *)DllFullPath,lstrlenA(DllFullPath)+1,NULL);
	DWORD dwID;
	LPVOID pFunc = LoadLibraryA;
	HANDLE hRemoteThread = CreateRemoteThread(hRemoteProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pFunc, pszLibFileRemote, 0, &dwID );
	if (hRemoteThread==NULL)
	{
		CloseHandle(hRemoteProcess);
		return FALSE;
	}
	//�ͷž��
	CloseHandle(hRemoteProcess);
	CloseHandle(hRemoteThread);
	return TRUE;
}

//�жϽ����Ƿ���64λ�������64λ����0�������32λ����1
int GetProcessIsWOW64(DWORD pid)
{
	int nRet=-1;
	EnableDebugPriv("SeDebugPrivilege");
	HANDLE hProcess;

	//��Զ���߳�
	hProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL); 
	LPFN_ISWOW64PROCESS fnIsWow64Process; 
	BOOL bIsWow64 = FALSE; 
	BOOL bRet;
	DWORD nError;
	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( GetModuleHandle(L"kernel32"),"IsWow64Process"); 
	if (NULL != fnIsWow64Process) 
	{ 
		bRet=fnIsWow64Process(hProcess,&bIsWow64);
		if (bRet==0)
		{
			nError=GetLastError();
			nRet=-2;
		}
		else
		{
			if (bIsWow64)
			{
				nRet=1;
			}
			else
			{
				nRet=0;
			}
		}
	} 
	CloseHandle(hProcess);
	return nRet;
}

void WriteLog(string s){
	s=s+"\n";
	extern HANDLE g_handleMailServer;
	extern LPTSTR g_strInjectMailSlot;
	//cout<<s<<endl;
	if (g_handleMailServer != INVALID_HANDLE_VALUE)
	{
		DWORD cbWritten = 0;
		BOOL result = realWriteFile?realWriteFile(g_handleMailServer,s.c_str(),s.length(),&cbWritten,NULL):WriteFile(g_handleMailServer,s.c_str(),s.length(),&cbWritten,NULL);
		//�ɹ��Ļ���ֱ���˳�
		//////////////////////////////////////////////////////////////////////////
		// to do :���ɹ��Ļ�������һ���жϣ�������������Ѿ��˳�����g_handleMailServer��Ϊ��Ч
		//////////////////////////////////////////////////////////////////////////
		if(result)
			return;
	}
	//ofstream f("C:\\test.txt",ios::app);
	ofstream f(log_path,ios::app);
	f<<s;
	f.close();
}

bool Acpro_Operation (int threadid)
{
	char Buff[9];
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);

	int processid = GetCurrentProcessId();
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, processid);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		OutputDebugString(L"CreateToolhelp32Snapshot failed");
		return false;
	}
	THREADENTRY32 te32 = {sizeof(te32)};
	if (Thread32First(hProcessSnap, &te32))
	{
		do {
			if (processid == te32.th32OwnerProcessID)
			{
				if (threadid == te32.th32ThreadID)
					return false;
			}
		} while (Thread32Next(hProcessSnap, &te32));
		return true;
	}
}
char * GetProPath(DWORD pid)
{
	EnableDebugPriv("SeDebugPrivilege");
	HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, pid);
	GetModuleFileNameEx(hProcess, NULL, path1, MAX_PATH);
	int iLength ;
	iLength = WideCharToMultiByte(CP_ACP, 0, path1, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, path1, -1, propath, iLength, NULL, NULL);

	CloseHandle(hProcess);
	return propath;
}
BOOL DosPathToNtPath(LPTSTR pszDosPath, LPTSTR pszNtPath)
{
	TCHAR			szDriveStr[500];
	TCHAR			szDrive[3];
	TCHAR			szDevName[100];
	INT				cchDevName;
	INT				i;

	//������
	if(!pszDosPath || !pszNtPath )
		return FALSE;

	//��ȡ���ش����ַ���
	if(GetLogicalDriveStrings(sizeof(szDriveStr), szDriveStr))
	{
		for(i = 0; szDriveStr[i]; i += 4)
		{
			if(!lstrcmpi(&(szDriveStr[i]), L"A:\\") || !lstrcmpi(&(szDriveStr[i]), L"B:\\"))
				continue;

			szDrive[0] = szDriveStr[i];
			szDrive[1] = szDriveStr[i + 1];
			szDrive[2] = '\0';
			if(!QueryDosDevice(szDrive, szDevName, 100))//��ѯ Dos �豸��
				return FALSE;

			cchDevName = lstrlen(szDevName);
			if(_wcsnicmp(pszDosPath, szDevName, cchDevName) == 0)//����
			{
				lstrcpy(pszNtPath, szDrive);//����������
				lstrcat(pszNtPath, pszDosPath + cchDevName);//����·��

				return TRUE;
			}			
		}
	}

	lstrcpy(pszNtPath, pszDosPath);

	return FALSE;
}
BOOL GetProcessFullPath(HANDLE hProcess, TCHAR pszFullPath[MAX_PATH])
{
	TCHAR	szImagePath[MAX_PATH];
	if(!hProcess)
		return FALSE;

	if(!GetProcessImageFileName(hProcess, szImagePath, MAX_PATH))
	{
		//CloseHandle(hProcess);
		return FALSE;
	}

	if(!DosPathToNtPath(szImagePath, pszFullPath))
	{
		//CloseHandle(hProcess);
		return FALSE;
	}

	//CloseHandle(hProcess);

	return TRUE;
}