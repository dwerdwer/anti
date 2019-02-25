#include "../Include/LinuxUtils.h"
#include <Module.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_PATH	1024

class LinuxIFile:public IFileEx {
public:
	bool  blWrite;
	bool  blDelete;
	char  path[MAX_PATH];
	FILE* f;

public:
	LinuxIFile()
	{
		f = NULL;
		path[0] = 0;
		blWrite = false;
		blDelete = false;
	}

protected:
	virtual ~LinuxIFile() 
	{
	}
public:

	virtual void AV_CALLTYPE Dispose()
	{
		if (f!=NULL)
			fclose(f);
		f = NULL;
		if ( blDelete)
		{
			unlink( path );
		}
		blWrite = false;
		blDelete = false;

		delete this;
	}

	/**
	@brief 读取文件

	@param buffer 读取到的内存缓冲区
	@param ulByteCount 缓冲区的大小

	@retval 返回 读取的字节数，如果失败返回 0
	*/
	virtual uint32_t AV_CALLTYPE Read(OUT void* buffer, IN uint32_t ulByteCount)
	{
		if (ulByteCount > 0x10000000)
			ulByteCount = 0x10000000;
		uint32_t nb = fread( buffer,1,ulByteCount,f);
		return nb;
	}


	/**
	@brief 写文件

	@param buffer 要写的内存缓冲区
	@param ulByteCount 缓冲区大小

	@retval 返回写入的字节数
	*/
	virtual uint32_t AV_CALLTYPE Write(IN void const* buffer, IN uint32_t ulByteCount)
	{
		if (ulByteCount > 0x10000000)
			ulByteCount = 0x10000000;
		uint32_t nb = fwrite( buffer,1,ulByteCount,f);
		return nb;

	}


	/**
	@brief 调整文件访访问偏移

	如果文件已经删除或者改名，调用该方法时不需要做任何处理，直接返回　INVALID_SIZE 即可

	@param qwOffset 调整的文件偏移量
	@param nFrom 调整的起始位置

	@retval 返回 当前的文件偏移，如果失败返回 INVALID_SIZE

	@see ESeekFrom
	*/
	virtual uint64_t AV_CALLTYPE Seek(IN int64_t qwOffset, IN ESeekFrom nFrom)
	{
		fseek( f, (uint32_t)qwOffset, nFrom);

		return ftell(f);
	}


	/**
	@brief 取当前文件偏移位置

	如果文件已经删除或者改名，调用该方法是直接返回 INVALID_SIZE

	@retval 返回 当前文件偏移，如果失败返回 INVALID_SIZE
	*/
	virtual uint64_t AV_CALLTYPE GetPosition()
	{
		return ftell(f);
	}


	/**
	@brief 设置文件的大小

	@param qwNewSize 将该文件大小调整为的大小

	@retval 如果成功返回 TRUE，否则返回 FALSE
	*/
	virtual bool AV_CALLTYPE SetSize(IN uint64_t qwNewSize)
	{
		if ( ftruncate(fileno(f), qwNewSize) )
			return true;
		return false;
	}


	/**
	@brief 读取文件的大小

	@retval 返回文件大小，如果失败返回 UINT64_MAX
	*/
	virtual uint64_t AV_CALLTYPE GetSize()
	{
		long lOffs = ftell(f);
		fseek(f,0,2);
		long lSize = ftell(f);
		fseek(f,lOffs,0);
		return lSize;
	}

	/**
	* @brief 读取文件的完整路径
	*
	*/
	virtual const wchar_t* AV_CALLTYPE GetPath()
	{
		return (wchar_t*)path;
	}

	/**
	@brief 读取文件对象名称

	该方法返回的是名称，不包含其他路径，由于返回的是名称的指针，文件对象负责
	保证在文件对象有效性，该指针指向的内容有效

	@retval 返回具体名称
	*/
	virtual const wchar_t* AV_CALLTYPE GetName()
	{
		char* p = strrchr(path,'/');
		if (p==NULL)
			return (wchar_t*)"";
		else
			return (wchar_t*)(++p);
	}


	/**
	@brief 读取文件的扩展包，包含开头的 .

	*/
	virtual const char* AV_CALLTYPE GetExtA()
	{
		char* p1 = strrchr(path,'/');
		char* p2 = strrchr(path,'.');
		if (p2==NULL)
			return "";
		else if (p1==NULL)
			return ++p2;
		else if (p2>p1)
			return ++p2;
		return NULL;
	}

	/**
	@brief 保证该文件对象是可写的，以便于清除处理

	在扫描过程中，PrepareForCure 会调用多次，因此实现时应该进行相应的处理，
	支持对该函数的重复调用
	该函数返回时，应该保证文件指针与调用函数时的值一致。

	@retval 返回具体结果，可能值见 EPrepareForCureResult
	*/
	virtual EPrepareForCureResult AV_CALLTYPE PrepareForCure()
	{
		long lOffs = ftell(f);
		fclose(f);
		f = fopen(path,"r+b");
		if (f==NULL)
		{
			f = fopen(path,"rb");
			fseek(f,lOffs,0);
			return forCureFailed;
		}
		fseek(f,lOffs,0);
		blWrite = true;
		return forCureSuccess;
	}

	/**
	@brief 本方法通知对应的文件对象需要删除

	具体实现时可以在文件关闭时再作删除，同时应该注意调用该函数后不应该再调用其他相关的文件
	操作，否则可能导致未知结果。
	对于删除后的文件，需要支持　Seek 和 GetPosition 函数，这时执行空操作即可，并返回
	INVALID_SIZE 。

	@retval 返回枚举 EDeleteResult 中的值
	*/
	virtual EDeleteResult AV_CALLTYPE Delete()
	{
		blDelete = true;
		return deleteSuccess;
	}

	/**
	@breif 取修改时间

	主要用于指纹中的相关计算
	*/
	virtual bool AV_CALLTYPE GetModifyTime(FILETIME* tm)
	{
		return false;
	}

public:
	bool OpenFile(const char* path, EOPMODE opMode)
	{
		if ( opMode==OP_READONLY )
			f = fopen( path,"rb");
		else 
		{
			if (opMode==OP_READWRITE)
				f = fopen( path, "r+b");
			if ( f==NULL )
				f = fopen(path,"w+b");
		}

		if ( f==NULL)
			return false;
		if (opMode!=OP_READONLY )
			blWrite = true;
		strcpy( this->path, path);
		return true;
	}
};


IFileEx* FileOpen(const char* path, EOPMODE opMode)
{
	LinuxIFile* pFile = new LinuxIFile();
	if ( pFile == NULL)
		return NULL;

	if ( pFile->OpenFile(path,opMode))
	{
		return pFile;
	}
	pFile->Dispose();
	return NULL;
}

unsigned int  g_tmpFileNo = 0;

IFileEx* FileCreateTemp()
{
	char name[MAX_PATH];

	++g_tmpFileNo;
	sprintf(name,"/tmp/~tmp%08X.tmp", g_tmpFileNo);

	LinuxIFile* pFile = (LinuxIFile*)FileOpen(name, OP_CREATE);
	if (pFile!=NULL)
	{
		pFile->blDelete = true;
	}

	return pFile;
}
