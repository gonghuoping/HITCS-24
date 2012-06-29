#include "StdAfx.h"
#include "Memory.h"


Memory::Memory(void)
{
}


Memory::~Memory(void)
{
	delete m_pMem;
}

APISpyData * Memory::GetData(){
	 return m_pData;
}

Memory ::Memory(BOOL bServer ){
   m_pMem= new ShareMemory( "APISpyLib",sizeof(APISpyData),bServer);
   m_pData = (APISpyData*)(m_pMem->GetBuffer());
   if(m_pData ==NULL)
   ExitProcess(-1);
}
