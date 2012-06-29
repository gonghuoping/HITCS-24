#include "StdAfx.h"
#include "APIHook.h"
#include<TlHelp32.h>
#include<Windows.h>
#include<ImageHlp.h>

#pragma comment(lib , "ImageHlp")


APIHook::APIHook(void)
{
}



//APIHOOk���������ͷָ��
APIHook * APIHook :: sm_pHeader = NULL;

APIHook ::APIHook (LPSTR pszModName,LPSTR pszFuncName,PROC pfnHook,BOOL bExcludeAPIHookMod ){
	
	//�������HOOk��������Ϣ
	m_bExcludeAPIHookMod = bExcludeAPIHookMod;
	m_pszModName = pszModName;
	m_pszFuncName = pszFuncName;
	m_pfnHook = pfnHook;
	m_pfnOrig= GetProcAddress(GetModuleHandle((LPCWSTR)pszModName),pszFuncName);
	
	//���˶�����ӵ�������
	m_pNext = sm_pHeader;
	sm_pHeader= this;
	
	//�����е�ǰ�Ѽ��ص�ģ����HOOK�������
    ReplaceIATEntryInAllMods( m_pszModName ,m_pfnOrig,m_pfnHook,bExcludeAPIHookMod);
}

APIHook ::~APIHook(){
	
	 //ȡ������ģ����к�����HOOK
	 ReplaceIATEntryInAllMods( m_pszModName ,m_pfnOrig,m_pfnHook,m_bExcludeAPIHookMod);
	 APIHook * p = sm_pHeader;
	
	 //���������Ƴ��˶���
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
	
	//ȡ��ģ��ĵ����import descriptor���׵�ַ��ImageDirectoryEntryToData�������Է��ص�����ַ
	ULONG ulSize;
	
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR) ImageDirectoryEntryToData(hModCaller, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT,&ulSize);
	
	//������ģ��û�е�����򷵻�
	if(pImportDesc == NULL)
		return;
	
	//���Ұ���pszExprotMod ģ���е�����Ϣ�ĵ������
	while(pImportDesc->Name != 0){
		LPSTR pszMod = (LPSTR)(DWORD)hModCaller+pImportDesc->Name;
		if(lstrcmpiA(pszMod,pszExportMod)==0) //�ҵ�
			break;
		pImportDesc++;
	}
	if(pImportDesc->Name==0)//hModeCallerģ��û�д�pszExportModģ�鵼���κκ���
		return;
	//ȡ�õ����ߵĵ����ַ��import address table�� IAT��
	PIMAGE_THUNK_DATA pThunk =(PIMAGE_THUNK_DATA)(pImportDesc->FirstThunk+(DWORD)hModCaller);

	//��������Ҫ��HOOK�ĺ����������ĵ�ַ���µĺ����ĵ�ַ�滻��
	while(pThunk->u1.Function){

		//lpAddrָ����ڴ汣���˺����ĵ�ַ
		PDWORD lpAddr = (PDWORD)&(pThunk->u1.Function);

		if(* lpAddr==(DWORD)pfnCurrent){
			//�޸�ҳ�ı�������
			DWORD dwOldProtect;
			MEMORY_BASIC_INFORMATION mbi;
			VirtualQuery(lpAddr,&mbi,sizeof(mbi));
			VirtualProtect(lpAddr,sizeof(DWORD),PAGE_READWRITE,&dwOldProtect);
			//�޸��ڴ��ַ�൱�ڡ�*lpAddr=(DWORD)pfnNew����
			WriteProcessMemory(GetCurrentProcess(),lpAddr, &pfnNew ,sizeof(DWORD),NULL);
			VirtualProtect(lpAddr,sizeof(DWORD),dwOldProtect,0);
			break;
		}
		pThunk++;
	}
}

void APIHook :: ReplaceIATEntryInAllMods(LPSTR pszExPortMod , PROC pfnCurrent, PROC pfnNew, BOOL bExcludeAPIHookMod){
	
	//ȡ�ĵ�ǰģ����
	HMODULE	hModThis = NULL;

	if(bExcludeAPIHookMod){

	MEMORY_BASIC_INFORMATION mbi;
	if(VirtualQuery(ReplaceIATEntryInAllMods,&mbi,sizeof(mbi))!= 0 )

	hModThis= (HMODULE)mbi.AllocationBase;
	}

	//ȡ�ñ����̵�ģ���б�
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,GetCurrentProcessId());

	//��������ģ�飬�ֱ�����ǵ���ReplaceIATEntryInOneMod�����������޸ĵ�����ַ��
	MODULEENTRY32 me = {sizeof(MODULEENTRY32)};

	BOOL bOK = Module32First(hSnap,&me);

	while(bOK){
	//��HOOK��ǰ��ģ��ĺ���
		if(me.hModule!= hModThis)
			ReplaceIATEntryInOneMod(pszExPortMod,pfnCurrent,pfnNew,me.hModule);
		bOK = Module32Next(hSnap,&me);
	}
	CloseHandle(hSnap);
}

//�ҹ�LoadLibrary��GetProcAddress�������Ա�����Щ�����������Ժ󣬹ҹ��ĺ���Ҳ�ܱ���ȷ�Ĵ���
APIHook APIHook::sm_LoadLibraryA("Kernel32.dall","LoadLibraryA",(PROC)APIHook::LoadLibraryA,TRUE);
APIHook APIHook::sm_LoadLibraryW("Kernel32.dall","LoadLibraryW",(PROC)APIHook::LoadLibraryW,TRUE);
APIHook APIHook::sm_LoadLibfaryExA("Kernel32.dall","LoadLibraryExA",(PROC)APIHook::LoadLibraryExA,TRUE);
APIHook APIHook::sm_LoadLibraryExW("Kernel32.dall","LoadLibraryExW",(PROC)APIHook::LoadLibraryExW,TRUE);
APIHook APIHook::sm_GetProcAddress("Kernel32.dall","GetAddress",(PROC)APIHook::GetProcAddress,TRUE);

void WINAPI APIHook::HookNewlyLoadedModule(HMODULE hModule ,DWORD dwFlags){
	//���һ���µ�ģ�鱻���أ��ҹ���APIHook����Ҫ���API����
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
	//�õ������������ʵ��ַ
	FARPROC pfn = GetProcAddress(hModule,pszProcName);
	//�����ǲ�������ҪHook�ĺ���
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
		printf("\n ģ������ �� %s \n ",pszDllName);
		//һ��IMAGE_THUNK_DATA ����һ��˫�֣���ָ����һ�����뺯��
		IMAGE_THUNK_DATA *pThunk=(IMAGE_THUNK_DATA*)((BYTE*)hMod+pImportDesc->OriginalFirstThunk);
		int n = 0;
		while(pThunk->u1.Function){
			//ȡ�ú������ơ�hint/name ����ǰ2���ֽ��Ǻ�����ţ�������Ǻ��������ַ���
			char* pszFunName =(char*)((BYTE*)hMod+(DWORD)pThunk->u1.AddressOfData+2);
			//��ȡ������ַ��IAT�����һ��DWORD���͵����飬ÿ����Ա��¼һ�������ĵ�ַ
			PDWORD lpAddr =(DWORD*)((BYTE*)hMod+pImportDesc->FirstThunk)+n;
			//��ӡ�������ֺ͵�ַ
			printf(" ����ĺ����� %-25s,",pszFunName);
			printf("������ַ�� %X\n",lpAddr);
			n++;
			pThunk++;
		}
		pImportDesc++;
	}
	return true;
}