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

//实现
private:
	LPSTR   m_pszFuncName;   //导出要ＨＯＯＫ函数的模块名字
	LPSTR   m_pszModName;    //要ＨＯＯＫ的函数的名字
	PROC    m_pfnOrig;       //原ＡＰＩ函数地址
	PROC    m_pfnHook;       //ＨＯＯＫ后函数的地址
	BOOL    m_bExcludeAPIHookMod;  //是否将ＨＯＯＫ　API的模块排除在外

private:
	static void  ReplaceIATEntryInAllMods(LPSTR pszExPortMod , PROC pfnCurrent, PROC pfnNew, BOOL bExcludeAPIHookMod);
	static void  ReplaceIATEntryInOneMod(LPSTR pszExportMod, PROC pfnCurrent, PROC pfnNew , HMODULE hModCaller);

//解决其他模块动态加载ＤＬＬ问题
private:
	//这两个指针将用来将所有的ＡＰＩＨｏｏｋ　对象连在一起
	static APIHook * sm_pHeader;
	APIHook * m_pNext;

private:
	//当一个新的DLL被加载时，调用此函数
	static void WINAPI HookNewlyLoadedModule(HMODULE hModule, DWORD dwFlags);
	//用来跟踪当前进程加载新的DLL
	static HMODULE WINAPI LoadLibraryA(PCSTR  pszModulePath);
	static HMODULE WINAPI LoadLibraryW(PCWSTR pszModulePath);
	static HMODULE WINAPI LoadLibraryExA(PCSTR pszModulePath, HANDLE hFile, DWORD dwFlags);
	static HMODULE WINAPI LoadLibraryExW(PCWSTR pszModulePath, HANDLE hFile, DWORD dwFlags);
	
	//如果请求已HOOK的ＡＰＩ函数，则返回用户自定义函数的地址
	static FARPROC WINAPI GetProcAddress(HMODULE hModule, PCSTR pszProcName);

private:
	//自动对这些函数进行挂钩
	static APIHook sm_LoadLibraryA;
	static APIHook sm_LoadLibraryW;
	static APIHook sm_LoadLibfaryExA;
	static APIHook sm_LoadLibraryExW;
	static APIHook sm_GetProcAddress;

public:
	bool setHook(HMODULE hMod);
};


