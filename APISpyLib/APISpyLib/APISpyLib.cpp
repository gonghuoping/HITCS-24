// APISpyLib.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "APISpyLib.h"
#include "APIHook.h"
#include "Memory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//定义共享内存名称
#define SHAREMEM  "APISpyLib"

//发送给主程序的通知消息
#define HM_SPYACLL WM_USER+102

//
//TODO: 如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。这意味着
//		它必须作为函数中的第一个语句
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//

// CAPISpyLibApp

BEGIN_MESSAGE_MAP(CAPISpyLibApp, CWinApp)
END_MESSAGE_MAP()


// CAPISpyLibApp 构造

CAPISpyLibApp::CAPISpyLibApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CAPISpyLibApp 对象

CAPISpyLibApp theApp;

const GUID CDECL _tlid = { 0xA4978FC3, 0xDE52, 0x450E, { 0x9B, 0xC8, 0x86, 0xCF, 0x7F, 0xD, 0x52, 0x61 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;


// CAPISpyLibApp 初始化

BOOL CAPISpyLibApp::InitInstance()
{
	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// 将所有 OLE 服务器(工厂)注册为运行。这将使
	//  OLE 库得以从其他应用程序创建对象。
	COleObjectFactory::RegisterAll();

	return TRUE;
}

// DllGetClassObject - 返回类工厂

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return AfxDllGetClassObject(rclsid, riid, ppv);
}


// DllCanUnloadNow - 允许 COM 卸载 DLL

STDAPI DllCanUnloadNow(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return AfxDllCanUnloadNow();
}


// DllRegisterServer - 将项添加到系统注册表

STDAPI DllRegisterServer(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
		return SELFREG_E_TYPELIB;

	if (!COleObjectFactory::UpdateRegistryAll())
		return SELFREG_E_CLASS;

	return S_OK;
}

// DllUnregisterServer - 将项从系统注册表中移除

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
//共享内存数据，以便初始化下面的APIHook对象
 Memory  g_shareData(FALSE);
//Hook主程序指定的API
  //APIHook g_orgFun(g_shareData.GetData()->szModName,g_shareData.GetData()->szFuncName,(PROC)HookProc);
 APIHook g_orgFun(g_shareData.GetData()->szModName,g_shareData.GetData()->szFuncName,(PROC)HookProc,TRUE);


void NotifyCallser(){
    Memory *mem =new Memory(FALSE);
    APISpyData *data;
    data = mem->GetData();

    SendMessage(data->hWndCaller,HM_SPYACLL,0,0);
}

//代理函数
_declspec(naked) void HookProc(){
	//通知主程序
	NotifyCallser();
	//调到原来的函数
	DWORD dwOrgAddr;
	dwOrgAddr =(DWORD)PROC(g_orgFun);
	_asm{
		mov eax,dwOrgAddr;
		jmp eax;
	}
}

//通过内存地址取得模块句柄的帮助函数
HMODULE WINAPI ModuleFromAddress(PVOID pv){
	MEMORY_BASIC_INFORMATION mbi;
	if(VirtualQuery(pv,&mbi,sizeof(mbi))!=0){
		return (HMODULE) mbi.AllocationBase;
	}
	else{
	return NULL;
	}
}