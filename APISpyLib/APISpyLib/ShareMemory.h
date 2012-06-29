#pragma once

#ifndef _SHAREMEMORY_H_
#define _SHAREMEMORY_H_

class ShareMemory
{
public:
	ShareMemory(void);
	const LPVOID GetBuffer();
	ShareMemory(const char* pszMapName, int nFileSize = 0, BOOL bServer = FALSE);
	~ShareMemory(void);
protected:
	HANDLE m_hFileMap;
	LPVOID m_pBuffer;
};

#endif //_SHAREMEMORY_H_