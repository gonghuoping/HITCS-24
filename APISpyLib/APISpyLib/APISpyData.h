#pragma once
class APISpyData
{
public:
	APISpyData(void);
	~APISpyData(void);

public:
	char szModName[256];
	char szFuncName[256];
    HWND hWndCaller;
};

