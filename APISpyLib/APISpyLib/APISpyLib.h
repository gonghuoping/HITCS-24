// APISpyLib.h : APISpyLib DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������
#include "APISpyData.h"
#include "Memory.h"
#include <string>

// CAPISpyLibApp
// �йش���ʵ�ֵ���Ϣ������� APISpyLib.cpp

class CAPISpyLibApp : public CWinApp
{
public:
	CAPISpyLibApp();

// ��д
public:
	virtual BOOL InitInstance();
	DECLARE_MESSAGE_MAP()

	APISpyData memData;
	Memory sharememory;
};



 