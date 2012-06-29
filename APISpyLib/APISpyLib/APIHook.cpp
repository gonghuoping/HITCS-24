#include "StdAfx.h"
#include "APIHook.h"
#include<TlHelp32.h>
#include<Windows.h>
#include<ImageHlp.h>

#pragma comment(lib , "ImageHlp")


APIHook::APIHook(void)
{
}



//APIHOOk对象链表的头指针
APIHook * APIHook :: sm_pHeader = NULL;

APIHook ::APIHook (LPSTR pszModName,LPSTR pszFuncName,PROC pfnHook,BOOL bExcludeAPIHookMod ){
	
	//保存这个HOOk函数的信息
	m_bExcludeAPIHookMod = bExcludeAPIHookMod;
	m_pszModName = pszModName;
	m_pszFuncName = pszFuncName;
	m_pfnHook = pfnHook;
	m_pfnOrig= GetProcAddress(GetModuleHandle((LPCWSTR)pszModName),pszFuncName);
	
	//将此对象添加到链表中
	m_pNext = sm_pHeader;
	sm_pHeader= this;
	
	//在所有当前已加载的模块中HOOK这个函数
    ReplaceIATEntryInAllMods( m_pszModName ,m_pfnOrig,m_pfnHook,bExcludeAPIHookMod);
}

APIHook ::~APIHook(){
	
	 //取消所有模块的中函数的HOOK
	 ReplaceIATEntryInAllMods( m_pszModName ,m_pfnOrig,m_pfnHook,m_bExcludeAPIHookMod);
	 APIHook * p = sm_pHeader;
	
	 //从链表中移除此对象
	 if(p== this){
		 sm_pHeader=p->m_pNext;
	 }
	 else{
	 while(p!= NULL){
		 if(p->m_pNext == this){
			 p->m_pNext = this->m_pNext;
			 break;
		 }
		 p=p->m_pNext;
	  }
	}
}

void APIHook ::ReplaceIATEntryInOneMod(LPSTR pszExportMod, PROC pfnCurrent, PROC pfnNew , HMODULE hModCaller){
	
	//取得模块的导入表（import descriptor）首地址。ImageDirectoryEntryToData函数可以返回导入表地址
	ULONG ulSize;
	
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR) ImageDirectoryEntryToData(hModCaller, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT,&ulSize);
	
	//如果这个模块没有导入表则返回
	if(pImportDesc == NULL)
		return;
	
	//查找包含pszExprotMod 模块中导入信息的导入表项
	while(pImportDesc->Name != 0){
		LPSTR pszMod = (LPSTR)(DWORD)hModCaller+pImportDesc->Name;
		if(lstrcmpiA(pszMod,pszExportMod)==0) //找到
			break;
		pImportDesc++;
	}
	if(pImportDesc->Name==0)//hModeCaller模块没有从pszExportMod模块导入任何函数
		return;
	//取得调用者的导入地址表（import address table， IAT）
	PIMAGE_THUNK_DATA pThunk =(PIMAGE_THUNK_DATA)(pImportDesc->FirstThunk+(DWORD)hModCaller);

	//查找我们要的HOOK的函数，将他的地址用新的函数的地址替换掉
	while(pThunk->u1.Function){

		//lpAddr指向的内存保存了函数的地址
		PDWORD lpAddr = (PDWORD)&(pThunk->u1.Function);

		if(* lpAddr==(DWORD)pfnCurrent){
			//修改页的保护属性
			DWORD dwOldProtect;
			MEMORY_BASIC_INFORMATION mbi;
			VirtualQuery(lpAddr,&mbi,sizeof(mbi));
			VirtualProtect(lpAddr,sizeof(DWORD),PAGE_READWRITE,&dwOldProtect);
			//修改内存地址相当于“*lpAddr=(DWORD)pfnNew；”
			WriteProcessMemory(GetCurrentProcess(),lpAddr, &pfnNew ,sizeof(DWORD),NULL);
			VirtualProtect(lpAddr,sizeof(DWORD),dwOldProtect,0);
			break;
		}
		pThunk++;
	}
}

void APIHook :: ReplaceIATEntryInAllMods(LPSTR pszExPortMod , PROC pfnCurrent, PROC pfnNew, BOOL bExcludeAPIHookMod){
	
	//取的当前模块句柄
	HMODULE	hModThis = NULL;

	if(bExcludeAPIHookMod){

	MEMORY_BASIC_INFORMATION mbi;
	if(VirtualQuery(ReplaceIATEntryInAllMods,&mbi,sizeof(mbi))!= 0 )

	hModThis= (HMODULE)mbi.AllocationBase;
	}

	//取得本进程的模块列表
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,GetCurrentProcessId());

	//遍历所有模块，分别对他们调用ReplaceIATEntryInOneMod（）函数，修改导入表地址表
	MODULEENTRY32 me = {sizeof(MODULEENTRY32)};

	BOOL bOK = Module32First(hSnap,&me);

	while(bOK){
	//不HOOK当前的模块的函数
		if(me.hModule!= hModThis)
			ReplaceIATEntryInOneMod(pszExPortMod,pfnCurrent,pfnNew,me.hModule);
		bOK = Module32Next(hSnap,&me);
	}
	CloseHandle(hSnap);
}

//挂钩LoadLibrary和GetProcAddress函数，以便在这些函数被调用以后，挂钩的函数也能被正确的处理
APIHook APIHook::sm_LoadLibraryA("Kernel32.dall","LoadLibraryA",(PROC)APIHook::LoadLibraryA,TRUE);
APIHook APIHook::sm_LoadLibraryW("Kernel32.dall","LoadLibraryW",(PROC)APIHook::LoadLibraryW,TRUE);
APIHook APIHook::sm_LoadLibfaryExA("Kernel32.dall","LoadLibraryExA",(PROC)APIHook::LoadLibraryExA,TRUE);
APIHook APIHook::sm_LoadLibraryExW("Kernel32.dall","LoadLibraryExW",(PROC)APIHook::LoadLibraryExW,TRUE);
APIHook APIHook::sm_GetProcAddress("Kernel32.dall","GetAddress",(PROC)APIHook::GetProcAddress,TRUE);

void WINAPI APIHook::HookNewlyLoadedModule(HMODULE hModule ,DWORD dwFlags){
	//如果一个新的模块被加载，挂钩个APIHook对象要求的API函数
	if((hModule != NULL)&&((dwFlags&LOAD_LIBRARY_AS_DATAFILE)==0)){
		APIHook * p = sm_pHeader;
		while(p!= NULL){
			ReplaceIATEntryInOneMod(p->m_pszFuncName,p->m_pfnOrig,p->m_pfnHook,hModule);
			p=p->m_pNext;
		}
	}
}

HMODULE WINAPI APIHook::LoadLibraryA(PCSTR pszModulePath){
	HMODULE hModule=LoadLibraryA(pszModulePath);
	HookNewlyLoadedModule(hModule,0);
	return (hModule);
}

HMODULE WINAPI APIHook::LoadLibraryW(PCWSTR pszModulePath){
	HMODULE hModule = LoadLibraryW(pszModulePath);
	HookNewlyLoadedModule(hModule,0);
	return (hModule);
}

HMODULE WINAPI APIHook::LoadLibraryExA(PCSTR pszModulePath , HANDLE hFile, DWORD dwFlags){
  HMODULE hModule=LoadLibraryExA(pszModulePath,hFile,dwFlags);
  HookNewlyLoadedModule(hModule,dwFlags);
  return (hModule);
}

HMODULE WINAPI APIHook::LoadLibraryExW(PCWSTR pszModulePath, HANDLE hFile , DWORD dwFlags){
	HMODULE hModule=LoadLibraryExW(pszModulePath,hFile,dwFlags);
	HookNewlyLoadedModule(hModule , dwFlags);
	return (hModule);
}

FARPROC WINAPI APIHook::GetProcAddress(HMODULE hModule,PCSTR pszProcName){
	//得到这个函数的真实地址
	FARPROC pfn = GetProcAddress(hModule,pszProcName);
	//看他是不是我们要Hook的函数
	APIHook *p = sm_pHeader;
	while(p!= NULL){
		if(p->m_pfnOrig == pfn){
			pfn = p->m_pfnHook;
			break;
		}
		p = p->m_pNext;
	}
	return pfn;
}

bool APIHook::setHook(HMODULE hMod){
	
	IMAGE_DOS_HEADER *pDosHeader =(IMAGE_DOS_HEADER*)hMod;
	IMAGE_OPTIONAL_HEADER *pOptHeader = (IMAGE_OPTIONAL_HEADER *)((BYTE*) hMod+pDosHeader->e_lfanew+24);
	IMAGE_IMPORT_DESCRIPTOR * pImportDesc = (IMAGE_IMPORT_DESCRIPTOR *)((BYTE*)hMod+pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	while(pImportDesc->Characteristics){

		char* pszDllName = (char*)((BYTE*)hMod+pImportDesc->Name);
		printf("\n 模块名称 ： %s \n ",pszDllName);
		//一个IMAGE_THUNK_DATA 就是一个双字，它指定了一个导入函数
		IMAGE_THUNK_DATA *pThunk=(IMAGE_THUNK_DATA*)((BYTE*)hMod+pImportDesc->OriginalFirstThunk);
		int n = 0;
		while(pThunk->u1.Function){
			//取得函数名称。hint/name 表向前2个字节是函数序号，后面才是函数名字字符串
			char* pszFunName =(char*)((BYTE*)hMod+(DWORD)pThunk->u1.AddressOfData+2);
			//获取函数地址。IAT表就是一个DWORD类型的数组，每个成员记录一个函数的地址
			PDWORD lpAddr =(DWORD*)((BYTE*)hMod+pImportDesc->FirstThunk)+n;
			//打印函数名字和地址
			printf(" 导入的函数： %-25s,",pszFunName);
			printf("函数地址： %X\n",lpAddr);
			n++;
			pThunk++;
		}
		pImportDesc++;
	}
	return true;
}