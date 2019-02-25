#include <stdio.h>
#include <stdlib.h>

#include "../Include/pub.h"

#define INSULATION_SIZE	0x0	// 原定义为0x20，保留一定的空间，现由于增加Realloc函数
// 地址得对齐，否则无法定位而出错。

#if 0

// 函数功能：存缓冲
DWORD __thiscall CAntiVirEngine::MemBlock_Save(void* buffer, DWORD size)
{
	LPBYTE	lp;

    if (((CScan*)pScan)->ri >= SAVE_MAXCOUNT) 
		return 0;

    if ((lp = (BYTE*)malloc(size))==NULL) 
		return 0;
    _mem_cpy(lp, (BYTE *)buffer, size);

    ((CScan*)pScan)->SavePtr[((CScan*)pScan)->ri] = lp;
    ((CScan*)pScan)->rsize  [((CScan*)pScan)->ri] = size;
    ((CScan*)pScan)->ri++;
    return size;
}

// 函数功能：加载缓冲
DWORD __thiscall CAntiVirEngine::MemBlock_Load(void* buffer)
{
    if (((CScan*)pScan)->ri == 0) 
		return 0;

    ((CScan*)pScan)->ri--;
    if (((CScan*)pScan)->SavePtr[((CScan*)pScan)->ri]==NULL)
		return 0;
    _mem_cpy((BYTE *)buffer, ((CScan*)pScan)->SavePtr[((CScan*)pScan)->ri], ((CScan*)pScan)->rsize[((CScan*)pScan)->ri]);
    free(((CScan*)pScan)->SavePtr[((CScan*)pScan)->ri]);
    ((CScan*)pScan)->SavePtr[((CScan*)pScan)->ri] = NULL;

    return ((CScan*)pScan)->rsize[((CScan*)pScan)->ri];
}

/*// 函数功能：追加内存
DWORD __thiscall CAntiVirEngine::Realloc(DWORD ptr, DWORD length)
{
	DWORD Oldptr, size;
	
	// 开辟内存
	Oldptr = ptr;
	size = _msize((void*)ptr);
	ptr = (DWORD)realloc((void*)ptr, size + length);
	if (ptr == NULL)
		return NULL;

	// 删除老指针并添加新指针
	((CScan*)pScan)->MemPtr_Delete(this, Oldptr);	 
	((CScan*)pScan)->MemPtr_Save(this, ptr);	
	return ptr;
}*/

#endif // 0

// 函数功能：开辟内存
void *__thiscall CAntiVirEngine::New(int length)
{
	void *addr;

	// 禁止申请80M以上的内存
	if (length > 0x2000000 || length < 0)
	{
		//printf("Large memory: %d\n",length);
		return NULL;
	}

	addr = malloc(length);
	if (addr == NULL)
		return NULL;

	//if (!((CScan*)pScan)->MemPtr_Save(this, addr))
	//	return NULL;

	return addr;
}

// 函数功能：删除内存
void __thiscall CAntiVirEngine::Delete(void *addr)
{
	if (addr <= INSULATION_SIZE)
		return;

	addr = (void*)((BYTE*)addr - INSULATION_SIZE);
//	if (((CScan*)pScan)->MemPtr_Delete(this, addr))
		free((void *)addr);
}


void* __thiscall CAntiVirEngine::_malloc(DWORD size)
{
	void* p = malloc(size);

	// printf(" malloc:%p, %08X\n", p, size);
	return p;
}

void __thiscall CAntiVirEngine::_free(void* memblock)
{
	free(memblock);
}




 