#pragma once
#pragma once

#include<Windows.h>
class APIHook
{
public:
	 APIHook(void);
     APIHook(LPSTR pszModName,LPSTR pszFuncName,PROC pfnHook,BOOL bExcludeAPIHookMod );
	 ~APIHook(void);
	operator PROC (){ return m_pfnOrig;}

//ʵ��
private:
	LPSTR   m_pszFuncName;   //����Ҫ�ȣϣϣ˺�����ģ������
	LPSTR   m_pszModName;    //Ҫ�ȣϣϣ˵ĺ���������
	PROC    m_pfnOrig;       //ԭ���Уɺ�����ַ
	PROC    m_pfnHook;       //�ȣϣϣ˺����ĵ�ַ
	BOOL    m_bExcludeAPIHookMod;  //�Ƿ񽫣ȣϣϣˡ�API��ģ���ų�����

private:
	static void  ReplaceIATEntryInAllMods(LPSTR pszExPortMod , PROC pfnCurrent, PROC pfnNew, BOOL bExcludeAPIHookMod);
	static void  ReplaceIATEntryInOneMod(LPSTR pszExportMod, PROC pfnCurrent, PROC pfnNew , HMODULE hModCaller);

//�������ģ�鶯̬���أģ̣�����
private:
	//������ָ�뽫���������еģ��Уɣȣ��롡��������һ��
	static APIHook * sm_pHeader;
	APIHook * m_pNext;

private:
	//��һ���µ�DLL������ʱ�����ô˺���
	static void WINAPI HookNewlyLoadedModule(HMODULE hModule, DWORD dwFlags);
	//�������ٵ�ǰ���̼����µ�DLL
	static HMODULE WINAPI LoadLibraryA(PCSTR  pszModulePath);
	static HMODULE WINAPI LoadLibraryW(PCWSTR pszModulePath);
	static HMODULE WINAPI LoadLibraryExA(PCSTR pszModulePath, HANDLE hFile, DWORD dwFlags);
	static HMODULE WINAPI LoadLibraryExW(PCWSTR pszModulePath, HANDLE hFile, DWORD dwFlags);
	
	//���������HOOK�ģ��Уɺ������򷵻��û��Զ��庯���ĵ�ַ
	static FARPROC WINAPI GetProcAddress(HMODULE hModule, PCSTR pszProcName);

private:
	//�Զ�����Щ�������йҹ�
	static APIHook sm_LoadLibraryA;
	static APIHook sm_LoadLibraryW;
	static APIHook sm_LoadLibfaryExA;
	static APIHook sm_LoadLibraryExW;
	static APIHook sm_GetProcAddress;

public:
	bool setHook(HMODULE hMod);
};


