#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "Include/LinuxScan.h"

#define MAX_PATH_LEN 512

bool g_blShowDebug;
bool g_blLog;
//int  count =0;
//

int  Init_AVScan(const char* pRoot);
void Finish_AVScan();


// 初始化引擎
// 
// pszVLibPath 病毒库根目录，以'\'结尾
//
// 返回:
//		0 = 成功
//
int AV_InitEng(const char* pszVLibPath)
{
	int ret = 0;

	// 初始化病毒库
	ret = Init_AVScan(pszVLibPath);
	if (ret)
		return ret;

	// Todo: 初始化指纹
	// 

	return ret;
}

// 清理引擎
int AV_ClearEng()
{
	// Todo: 清理指纹
	// 

	Finish_AVScan();
	return 0;
}

//
// 对文件查毒
//
// filePath			查毒文件完整路径，非文件夹
// virName,			返回发现的病毒名
// pdwVirRecNo		返回病毒记录号
//
// 返回：
// 
//
//int AV_ScanFileDirect(const char* filePath, SCANMODE mode, char* virName, unsigned int* pdwVirRecNo)
//{
//	// 查毒
//	int ret = ScanFile_IFile(filePath, mode, virName, pdwVirRecNo, NULL);
//
//	return ret;
//}
//
int AV_ScanFile(const char* filePath, SCANMODE mode, char* virName, unsigned int* pdwVirRecNo)
{
	LinuxScan Scan(NULL);

	// 查毒
	int ret = Scan.AV_ScanFile(filePath, mode, virName, pdwVirRecNo);

	return ret;
}

// 
// 文件夹查毒
//
// dirname		文件夹，不以'/'结尾
//
int LinuxScan::AV_ScanDir(const char* dirname, SCANMODE mode)
{
	struct dirent *filename;//readdir 的返回类型
	DIR *dir;

	dir = opendir(dirname);
	if (dir == NULL)
		return 0;

	while ((filename = readdir(dir)) != NULL)
	{
		if (!strcmp(filename->d_name, ".") || !strcmp(filename->d_name, ".."))
			continue;

		char path[MAX_PATH];
		sprintf(path, "%s/%s", dirname, filename->d_name);
		struct stat s;
		lstat(path, &s);

		if (S_ISDIR(s.st_mode))
		{
			AV_ScanDir(path, mode);//递归调用
		}
		else
		{
			AV_ScanFile(path, mode, NULL, NULL);
		}
	}
	closedir(dir);

	return 0;
}



#define SCANOBJ_MYCOMPUTER	"\x1"		// 全盘扫描
#define SCANOBJ_PERSONAL	"\x2"
#define SCANOBJ_MYDOC		"\x3"

//
// 扫描一个或一组目标
//
int AV_ScanObject(const char** scanObj, int iObjCount, SCANMODE mode, ILinuxScan* pILinuxScan)
{
	for (int i = 0; i < iObjCount; i++)
	{
		// 判断特殊预定义扫描目标
		if (strcmp(scanObj[i], SCANOBJ_MYCOMPUTER) == 0)
		{

		}
		else if (strcmp(scanObj[i], SCANOBJ_MYCOMPUTER) == 0)
		{

		}
		else if (strcmp(scanObj[i], SCANOBJ_MYCOMPUTER) == 0)
		{
		}
		else
		{
			//判断一个路径是否是目录
			struct   stat   s;
			if (lstat(scanObj[i], &s)   <   0)
			{
				continue;
			}
			if (!S_ISDIR(s.st_mode))
			{
				pILinuxScan->AV_ScanFile(scanObj[i],mode, NULL, NULL);
			}
			else
			{
				pILinuxScan->AV_ScanDir(scanObj[i],mode);
			}
		}
	}

	return 0;
}
