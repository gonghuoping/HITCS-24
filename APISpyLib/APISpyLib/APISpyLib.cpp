// APISpyLib.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"
#include "APISpyLib.h"
#include "APIHook.h"
#include "Memory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//���干���ڴ�����
#define SHAREMEM  "APISpyLib"

//���͸��������֪ͨ��Ϣ
#define HM_SPYACLL WM_USER+102

//
//TODO: ����� DLL ����� MFC DLL �Ƕ�̬���ӵģ�
//		��Ӵ� DLL �������κε���
//		MFC �ĺ������뽫 AFX_MANAGE_STATE ����ӵ�
//		�ú�������ǰ�档
//
//		����:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// �˴�Ϊ��ͨ������
//		}
//
//		�˺������κ� MFC ����
//		������ÿ��������ʮ����Ҫ������ζ��
//		��������Ϊ�����еĵ�һ�����
//		���֣������������ж������������
//		������Ϊ���ǵĹ��캯���������� MFC
//		DLL ���á�
//
//		�й�������ϸ��Ϣ��
//		����� MFC ����˵�� 33 �� 58��
//

// CAPISpyLibApp

BEGIN_MESSAGE_MAP(CAPISpyLibApp, CWinApp)
END_MESSAGE_MAP()


// CAPISpyLibApp ����

CAPISpyLibApp::CAPISpyLibApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CAPISpyLibApp ����

CAPISpyLibApp theApp;

const GUID CDECL _tlid = { 0xA4978FC3, 0xDE52, 0x450E, { 0x9B, 0xC8, 0x86, 0xCF, 0x7F, 0xD, 0x52, 0x61 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;


// CAPISpyLibApp ��ʼ��

BOOL CAPISpyLibApp::InitInstance()
{
	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// ������ OLE ������(����)ע��Ϊ���С��⽫ʹ
	//  OLE ����Դ�����Ӧ�ó��򴴽�����
	COleObjectFactory::RegisterAll();

	return TRUE;
}

// DllGetClassObject - �����๤��

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return AfxDllGetClassObject(rclsid, riid, ppv);
}


// DllCanUnloadNow - ���� COM ж�� DLL

STDAPI DllCanUnloadNow(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return AfxDllCanUnloadNow();
}


// DllRegisterServer - ������ӵ�ϵͳע���

STDAPI DllRegisterServer(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
		return SELFREG_E_TYPELIB;

	if (!COleObjectFactory::UpdateRegistryAll())
		return SELFREG_E_CLASS;

	return S_OK;
}

// DllUnregisterServer - �����ϵͳע������Ƴ�

STDAPI DllUnregisterServer(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
		return SELFREG_E_TYPELIB;

	if (!COleObjectFactory::UpdateRegistryAll(FALSE))
		return SELFREG_E_CLASS;

	return S_OK;
}

void HookProc();
//�����ڴ����ݣ��Ա��ʼ�������APIHook����
 Memory  g_shareData(FALSE);
//Hook������ָ����API
  //APIHook g_orgFun(g_shareData.GetData()->szModName,g_shareData.GetData()->szFuncName,(PROC)HookProc);
 APIHook g_orgFun(g_shareData.GetData()->szModName,g_shareData.GetData()->szFuncName,(PROC)HookProc,TRUE);


void NotifyCallser(){
    Memory *mem =new Memory(FALSE);
    APISpyData *data;
    data = mem->GetData();

    SendMessage(data->hWndCaller,HM_SPYACLL,0,0);
}

//������
_declspec(naked) void HookProc(){
	//֪ͨ������
	NotifyCallser();
	//����ԭ���ĺ���
	DWORD dwOrgAddr;
	dwOrgAddr =(DWORD)PROC(g_orgFun);
	_asm{
		mov eax,dwOrgAddr;
		jmp eax;
	}
}

//ͨ���ڴ��ַȡ��ģ�����İ�������
HMODULE WINAPI ModuleFromAddress(PVOID pv){
	MEMORY_BASIC_INFORMATION mbi;
	if(VirtualQuery(pv,&mbi,sizeof(mbi))!=0){
		return (HMODULE) mbi.AllocationBase;
	}
	else{
	return NULL;
	}
}