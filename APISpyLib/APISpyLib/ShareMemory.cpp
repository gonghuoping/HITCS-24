#include "StdAfx.h"
#include "ShareMemory.h"

ShareMemory::ShareMemory(void)
{
}

//删掉了inline关键字
ShareMemory::ShareMemory(const char* pszMapName,int nFileSize,BOOL bServer):m_hFileMap(NULL),m_pBuffer(NULL)
{
	LPCWSTR mypszMapName=(LPCWSTR)pszMapName;
	if(bServer){
	    //创建一个内存映射文件对象
		m_hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,nFileSize,mypszMapName);
	}
	else{
	//打开一个内存映射文件对象
		m_hFileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,mypszMapName);
	}
	//映射它到内存，去的共享内存的首地址
	m_pBuffer=(LPBYTE)MapViewOfFile(m_hFileMap,FILE_MAP_ALL_ACCESS,0,0,0);
 }

const LPVOID ShareMemory::GetBuffer() 
{
	return m_pBuffer;
}

//去掉了inline关键字
ShareMemory::~ShareMemory(void)
{
	//取消文件的映射，关闭文件映射对象句柄
	UnmapViewOfFile(m_pBuffer);
	CloseHandle(m_hFileMap);
}
