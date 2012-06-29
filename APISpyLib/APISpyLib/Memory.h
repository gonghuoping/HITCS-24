#pragma once

#include"APISpyData.h"
#include"ShareMemory.h"
class Memory
{
public:
	 Memory(void);
	~Memory(void);
	 APISpyData * GetData();
	 Memory(BOOL bServer);
private:
	 APISpyData * m_pData;
	 ShareMemory * m_pMem;
};

