/*******************************************************************************
*						       文件操作外围函数                                *
********************************************************************************
*	Name : CB_File.cpp          Origian Place : BJ of China                    *
*	Create Data : 1998-2002     Now Version :   1.0                            *
*	Modify Time :               Translater : HeGong                            *
*==============================================================================*
*                        Modification History                                  *
*==============================================================================*
*         V1.0  1. Create this program.                                        *
*               2. 08/11/2005 fixed by Wangwei.运用新机制，改回外围函数        *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "../Include/pub.h"


#ifdef USE_FILE_HANDLE

#define LTD64K 1

// 函数功能：扫描文件时,调用_Seek,_Read
DWORD __thiscall CAntiVirEngine::_Seek(DWORD pos)
{
	return SetFilePointer(hScanFile, pos, 0, SEEK_SET);
}

DWORD __thiscall CAntiVirEngine::_Read(void* ptr, DWORD size)
{
	DWORD   r;

#if LTD64K
	size &= 0xFFFF;
#endif
	if (ReadFile(hScanFile, ptr, size, &r, NULL) == 0)
		return 0;

	return r;
}

DWORD __thiscall CAntiVirEngine::_Write(void* ptr, DWORD size)
{
	DWORD	r;

#if LTD64K
	//	size &= 0xFFFF;
#endif
	if (WriteFile(hScanFile, ptr, size, &r, NULL) == 0)
	{
		RetFlags |= 0x4000;	// Disk out of space
		return 0;
	}
	return r;
}

DWORD __thiscall CAntiVirEngine::_LSeek(DWORD pos, DWORD mode)
{
	return SetFilePointer(hScanFile, pos, 0, mode);
}

//	函数功能：解压文件时,调用Pack_Seek,Pack_Read 读原压缩文件,此时P_N为1
//            解压可执行文件时,调用Pack_Seek,Pack_Read 读原压缩文件,此时P_N为1
DWORD __thiscall CAntiVirEngine::Pack_Seek(DWORD pos, DWORD mode)
{
	KVFILE  fd;

	fd = hTempP;

	return SetFilePointer(fd, pos, 0, mode);
}

DWORD __thiscall CAntiVirEngine::Pack_Read(void* ptr, DWORD size)
{
	KVFILE	fd;
	DWORD	r;

	//	if (fStop)
	//		return 0;

	fd = hTempP;

	//	if (size>0xffff) {
	//    	printf("Pack_Read = %x.\n",size);
	//        return 0;
	//    }

#if LTD64K
	size &= 0xFFFF;
#endif

	if (ReadFile(fd, ptr, size, &r, NULL) == 0)
		return 0;

	return r;
}

BOOL __thiscall CAntiVirEngine::_SetEndOfFile(DWORD size)
{
	SetFilePointer(hScanFile, size, 0, SEEK_SET);
	return SetEndOfFile(hScanFile);
}


// 函数功能：拷贝文件句柄
BOOL CopyHandle(KVFILE	hSrc, KVFILE hDest)
{
	BYTE	*pbuf;
	DWORD	 size, rr, rw, hpos;
	BOOL	 ret;

	if (hSrc == hDest)
		return true;

	size = GetFileSize(hSrc, NULL);
	if (size > 0x200000L)
		size = 0x200000L;

	//size = 0x8000;
	pbuf = (BYTE*)malloc(size);
	if (pbuf == NULL)
		return false;

	hpos = SetFilePointer(hSrc, 0, 0, SEEK_SET);
	SetFilePointer(hSrc, 0, 0, SEEK_SET);
	SetFilePointer(hDest, 0, 0, SEEK_SET);
	ret = true;

	while (9)
	{
		ReadFile(hSrc, pbuf, size, &rr, NULL);
		if (rr == 0)
			break;
		WriteFile(hDest, pbuf, rr, &rw, NULL);
		if (rw != rr)
		{
			ret = false;
			break;
		}
	}

	SetEndOfFile(hDest);
	SetFilePointer(hSrc, hpos, 0, SEEK_SET);
	SetFilePointer(hDest, 0, 0, SEEK_SET);
	free(pbuf);
	return ret;
}

DWORD __thiscall CAntiVirEngine::Pack_Copy_File()
{
	return !CopyHandle(hTempP, hScanFile);
}


#else

#ifndef NO_USE_IFILEEX

//#include "../../../../../KVEngine2017/Interface/Module.h"
#include <Module.h>

DWORD __thiscall CAntiVirEngine::_Seek(DWORD pos)
{
	return (DWORD)hScanFile->Seek(pos, IFileEx::seekFromBegin);
}

DWORD __thiscall CAntiVirEngine::_LSeek(DWORD pos, DWORD mode)
{
	return (DWORD)hScanFile->Seek(pos, (IFileEx::ESeekFrom)mode);
}

DWORD __thiscall CAntiVirEngine::_Read(void* ptr, DWORD size)
{
	return (DWORD)hScanFile->Read(ptr, size);
}

DWORD	__thiscall	CAntiVirEngine::_Write(void* ptr, DWORD size)
{
	return (DWORD)hScanFile->Write(ptr, size);
}
BOOL	__thiscall	CAntiVirEngine::_SetEndOfFile(DWORD size)
{
	return hScanFile->SetSize(size);
}

DWORD	__thiscall	CAntiVirEngine::Pack_Seek(DWORD pos, DWORD mode)
{
	return (DWORD)hTempP->Seek(pos, (IFileEx::ESeekFrom)mode);
}

DWORD	__thiscall	CAntiVirEngine::Pack_Read(void* ptr, DWORD size)
{
	return(DWORD)hTempP->Read(ptr, size);
}

// 函数功能：拷贝文件句柄
BOOL CopyHandle(CAntiVirEngine* AVE, KVFILE	hSrc, KVFILE hDest)
{
	BYTE	*pbuf;
	DWORD	 size, rr, rw, hpos;
	BOOL	 ret;

	if (hSrc == hDest)
		return true;

	size = (DWORD)hSrc->GetSize();
	if (size > 0x200000L)
		size = 0x200000L;

	//size = 0x8000;
	pbuf = (BYTE*)malloc(size);
	if (pbuf == NULL)
		return false;

	hpos = (DWORD)hSrc->GetPosition();
	hSrc->Seek (0, IFileEx::seekFromBegin);
	hDest->Seek(0, IFileEx::seekFromBegin);
	ret = true;

	while (9)
	{
		rr = (DWORD)hSrc->Read(pbuf, size);
		if (rr == 0)
			break;
		rw = (DWORD)hDest->Write(pbuf, rr);
		if (rw != rr)
		{
			ret = false;
			break;
		}
	}

	hDest->SetSize(size);
	hSrc->Seek(hpos, IFileEx::seekFromBegin);
	hDest->Seek(0, IFileEx::seekFromBegin);
	free(pbuf);
	return ret;
}

DWORD __thiscall CAntiVirEngine::Pack_Copy_File()
{
	return !CopyHandle(this, hTempP, hScanFile);
}

#else

// #include <io.h>
#include <unistd.h>

#define LTD64K 1

// 函数功能：扫描文件时,调用_Seek,_Read
DWORD __thiscall CAntiVirEngine::_Seek(DWORD pos)
{
	fseek(hScanFile, pos, 0);
	return ftell(hScanFile);
}

DWORD __thiscall CAntiVirEngine::_Read(void* ptr, DWORD size)
{
	DWORD   r;

#if LTD64K
	size &= 0xFFFF;
#endif
	//if (ReadFile(hScanFile, ptr, size, &r, NULL) == 0)
	//	return 0;

	r = (DWORD)fread(ptr, 1, size, hScanFile );
	if (r == 0)
		return 0;

	return r;
}

DWORD __thiscall CAntiVirEngine::_Write(void* ptr, DWORD size)
{
	DWORD	r;

#if LTD64K
	//	size &= 0xFFFF;
#endif
	if ( (r = (DWORD)fwrite(ptr, 1, size, hScanFile)) == 0 )
//	if (WriteFile(hScanFile, ptr, size, &r, NULL) == 0)
	{
		RetFlags |= 0x4000;	// Disk out of space
		return 0;
	}
	return r;
}

DWORD __thiscall CAntiVirEngine::_LSeek(DWORD pos, DWORD mode)
{
	// return SetFilePointer(hScanFile, pos, 0, mode);
	fseek(hScanFile, pos, mode);
	return ftell(hScanFile);
}

//	函数功能：解压文件时,调用Pack_Seek,Pack_Read 读原压缩文件,此时P_N为1
//            解压可执行文件时,调用Pack_Seek,Pack_Read 读原压缩文件,此时P_N为1
DWORD __thiscall CAntiVirEngine::Pack_Seek(DWORD pos, DWORD mode)
{
    KVFILE  fd;

	fd = hTempP;

	// return SetFilePointer(fd, pos, 0, mode);
	fseek(fd, pos, mode);
	return ftell(fd);
}

DWORD __thiscall CAntiVirEngine::Pack_Read(void* ptr, DWORD size)
{
    KVFILE	fd;
	DWORD	r;

	//	if (fStop)
	//		return 0;

	fd = hTempP;

	//	if (size>0xffff) {
	//    	printf("Pack_Read = %x.\n",size);
	//        return 0;
	//    }

#if LTD64K
	size &= 0xFFFF;
#endif

	// if (ReadFile(fd, ptr, size, &r, NULL) == 0)
	if ( ( r = (DWORD)fread(ptr, 1, size, fd)) == 0)
    	return 0;
        
    return r;
}

BOOL __SetEndOfFile(KVFILE hSrc, DWORD size)
{
	//  SetFilePointer(, 0, SEEK_SET);
	//   return SetEndOfFile(hScanFile);

	fseek(hSrc, size, 0);
//	int r = chsize(fileno(hSrc), size);
	int r = ftruncate(fileno(hSrc), size);

	// int r = 0;
	if (r == 0)	// 成功
		return true;
	return false;

}


BOOL __thiscall CAntiVirEngine::_SetEndOfFile(DWORD size)
{
	return __SetEndOfFile(hScanFile, size);
}

DWORD __GetFileSize( KVFILE	hSrc )
{
	DWORD now = ftell(hSrc);
	fseek(hSrc, 0, 2);
	DWORD dwSize = ftell(hSrc);
	fseek(hSrc, now, 0);

	return dwSize;
}

// 函数功能：拷贝文件句柄
BOOL CopyHandle(CAntiVirEngine* AVE, KVFILE	hSrc, KVFILE hDest)
{
	BYTE	*pbuf;
	DWORD	 size, rr, rw, hpos;
	BOOL	 ret;

	if (hSrc == hDest)
		return true;

	size = __GetFileSize(hSrc);
	if (size > 0x200000L)
		size = 0x200000L;

	//size = 0x8000;
	pbuf = (BYTE*)malloc(size);
	if (pbuf == NULL)
		return false;

	hpos = ftell(hSrc);
	fseek(hSrc, 0, 0);
	fseek(hDest, 0, 0);
	ret = true;

	while (9)
	{
		rr = (DWORD)fread(pbuf, 1, size, hSrc );
		if (rr == 0)
			break;
		rw = (DWORD)fwrite(pbuf, 1, rr, hDest);
		if (rw != rr)
		{
			ret = false;
			break;
		}
	}

	__SetEndOfFile(hDest, size);
	fseek(hSrc, hpos, 0);
	fseek(hDest, 0, 0);
	free(pbuf);
	return ret;
}

DWORD __thiscall CAntiVirEngine::Pack_Copy_File()
{
    return !CopyHandle(this,hTempP, hScanFile);
}

#endif // USE_IFILEEX

#endif // #ifdef USE_FILE_HANDLE