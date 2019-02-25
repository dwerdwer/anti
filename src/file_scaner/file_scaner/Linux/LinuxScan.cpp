#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "Include/LinuxUtils.h"
#include "Include/LinuxScan.h"

#include <Module.h>
#include "../Interface/CEngData.h"
#include "../Include/VL_Const.h"
#include "../Include/FileType.h"
// #include "../AntiVirEngine/Include/pub.h"
#include "../VirusName/VirusFileAPI.h"

extern NameDatReader reader;
extern bool		g_blShowDebug;
extern bool		g_blLog;
extern FILE* 	g_fLog;

int  AV_CALLTYPE SCAN_File(CEngData* pAVEData, int blCure);
extern "C" void Check_Error(CEngData* pAVE);
const char* GetCPUName(WORD wCPUType);
const char* GetImageName(WORD wImageType);

LinuxScan::LinuxScan(IScanNotify* _notifyCallback)
{
	notifyCallback = _notifyCallback;

	g_fileCount = 0;
	g_virusCount = 0;
	g_unpackCount = 0;
	g_unpackFailCount = 0;
	g_szCurrentFilePath[0] = 0;
}

LinuxScan::~LinuxScan()
{

}

void Debug_Show_VirInfo(CEngData* pAVE, int deepth, int recId,char vName[])
{
	//		printf("\t===>Virus: ID(%d, %s), %s\n", recId, vName,(LPCSTR) CStringA(pAVE->Full_Name));
	if (!g_blShowDebug || !deepth)
		printf("%s\n", (LPCSTR)pAVE->hScanFile->GetPath());
	printf("\t===>Virus: VID(%8d, 0x%08X) %s\n", recId, recId, vName);
	printf("\t    FileType: %02X(%3d), wFileType: %02X(%3d),SubType:%02X(%3d), InfoFlag = %08X\n",
		pAVE->File_Type, pAVE->File_Type, pAVE->wFileType, pAVE->wFileType, pAVE->wSubType, pAVE->wSubType,
		pAVE->dwInfoFlags);
}

void Debug_Show_FileType(CEngData* pAVE, const char*g_szCurrentFilePath)
{
	char buf[4096];
	int offs = 0;

	// PE / PE64
	if (pAVE->wFileType >= ST2_PE && pAVE->wFileType < ST2_ELF)
	{
		offs = sprintf(buf, "FileType: %02X(%3d), wFileType: %02X(%3d),SubType:%02X(%3d), %s, %s, InfoFlag = %08X",
			pAVE->File_Type, pAVE->File_Type, pAVE->wFileType, pAVE->wFileType, pAVE->wSubType, pAVE->wSubType,
			GetCPUName(pAVE->wCPUType),
			GetImageName(pAVE->wImageType),
			pAVE->dwInfoFlags);
		if (pAVE->dwInfoFlags&INFO_HAS_LESSDATA)
			offs += sprintf(buf + offs, " LessData");

		if (pAVE->dwInfoFlags&INFO_HAS_MOREDATA)
			offs += sprintf(buf + offs, " MoreData");
		if (pAVE->dwInfoFlags&INFO_HAS_SIGN)
			offs += sprintf(buf + offs, " Sign");
		if (pAVE->dwInfoFlags&INFO_HAS_SPECIAL_SEC)
			offs += sprintf(buf + offs, " Special");
		if (pAVE->dwInfoFlags&INFO_IS_TEXT)
			offs += sprintf(buf + offs, " Text");
		if (pAVE->dwInfoFlags&INFO_HAS_EXECDATA)
			offs += sprintf(buf + offs, " *****HasExecData");
	}
	else
	{
		offs = sprintf(buf, "FileType: %02X(%3d), wFileType: %02X(%3d),SubType:%02X(%3d)",
			pAVE->File_Type, pAVE->File_Type, pAVE->wFileType, pAVE->wFileType, pAVE->wSubType, pAVE->wSubType);
	}

	printf("    %s\n", buf);

	if (g_fLog != NULL && (pAVE->dwInfoFlags&(INFO_HAS_SPECIAL_SEC | INFO_HAS_EXECDATA)))
	{
		char buf2[4096];

		offs = sprintf(buf2, "%s\n    %s\n", g_szCurrentFilePath, buf);
		fwrite(buf2, 1, offs, g_fLog);
	}
}

void Debug_Show_ExepInfo(CEngData* pAVE, int decodeId,KVFILE pTempFile)
{
	if (decodeId > 0) {
		printf("%s\n\tUnpack success: UpID(%4d (0x%04X), %-12s), %s\n",
			(LPCSTR)pAVE->hScanFile->GetPath(), decodeId, decodeId, PKEXE_GetName(decodeId), (LPCSTR)pTempFile->GetPath());
	}
	else
	{
		printf("%s\n\tUnpack fail   : UpID(%4d (0x%04X), %-12s) *********************\n",
			(LPCSTR)pAVE->hScanFile->GetPath(), decodeId, 0 - decodeId, PKEXE_GetName(0 - decodeId));
	}
}

int LinuxScan::AV_ScanFile_Internal(CEngData* pAVE, int deepth)
{
	int recId = 0;

	recId = SCAN_File(pAVE, 0);
	if (recId)
	{
		char vName[0x100] = "";
		reader.GetName(recId, vName);

		g_virusCount++;
		if (g_blShowDebug)
			Debug_Show_VirInfo(pAVE, deepth, recId, vName);
		if (notifyCallback != NULL)
		{
			notifyCallback->OnFindVirus((LPCSTR)pAVE->hScanFile->GetPath(), vName, recId, ERESULT_FIND);
		}
	}

	if (g_blShowDebug)
		Debug_Show_FileType(pAVE, g_szCurrentFilePath);

	return recId;
}

int LinuxScan::ScanFile_Loop(CEngData* pAVE, int deepth)
{
	FTRESULT result;
	int		 virId = 0;

	// 首次识别，获取Entry
	int ret = FT_RecongizeFirst(pAVE, &result);
	if (ret < 0) {
		//if (g_blShowDebug)
		//	printf("%s\n\t---------- Recongize Fail %s\n", (LPCSTR)pAVE->hScanFile->GetPath(), pAVE->Full_Name);
		return 0;	// 无需查毒
	}

	// 识别成功
	Check_Error(pAVE);

	do {
		// 需要先查文件，再检查Exep 脱壳
		// 因为在查文件时初步经过 VM16/32, 填充缓冲区

		// 检查是否文件病毒
		virId = AV_ScanFile_Internal(pAVE, deepth);
		if (virId > 0)
			break;

		// 解压
		// if (result.wResult == EFT_NEED_CHECK_UNPACK) 
		{
			// 准备 临时文件
			IFileEx* pTempFile = FileCreateTemp();
			if (pTempFile == NULL)
				break;

			Check_Error(pAVE);

			// 脱壳
			int decodeId = PKEXE_Unpack(pAVE, pTempFile, &result);
			if (g_blShowDebug)
				Debug_Show_ExepInfo(pAVE, decodeId, pTempFile);
			if (decodeId>0) {
				if (notifyCallback != NULL)
				{
					notifyCallback->OnFindExep((LPCSTR)pAVE->hScanFile->GetPath(), PKEXE_GetName(decodeId), decodeId, ERESULT_EXEP_OK);
				}

				// 递归扫描解压出的文件
				CEngData* pAVE2 = FT_GetScanPool(pAVE->hTempP);
				if (pAVE != NULL) {
					g_unpackCount++;
					ScanFile_Loop(pAVE2, deepth + 1);

					// 扫描结束
					FT_ReleaseScanPool(pAVE2);
				}
			}
			else {
				if (decodeId < 0) {
					g_unpackFailCount++;
					if (notifyCallback != NULL)
					{
						notifyCallback->OnFindExep((LPCSTR)pAVE->hScanFile->GetPath(), PKEXE_GetName(0 - decodeId), 0 - decodeId, ERESULT_EXEP_FAIL);
					}
				}

				// Not pack_exe
			}
			pTempFile->Dispose();
		}
	} while ((ret = FT_RecongizeNext(pAVE, &result)) == 0);

	return virId;
}

int LinuxScan::AV_ScanFile(LPCSTR path, SCANMODE mode, char* virName, unsigned int* pdwVirRecNo)
{
	if (notifyCallback != NULL)
		notifyCallback->OnScanStart(path);
	IFileEx* pFile = FileOpen(path);
	if (pFile == NULL)
	{
		if (notifyCallback != NULL)
			notifyCallback->OnScanEnd(path);
		return -1;
	}

	// Todo: 检查文件指纹
	//

	// 获得扫描区
	CEngData* pAVE = FT_GetScanPool(pFile);
	if (pAVE == NULL)
	{
		pFile->Dispose();
		if (notifyCallback != NULL)
			notifyCallback->OnScanEnd(path);
		return -2;
	}
	Check_Error(pAVE);

	// 扫描处理
	int virId = ScanFile_Loop(pAVE, 0);
	if (virId >= 0)
	{
		if (pdwVirRecNo != NULL)
			*pdwVirRecNo = virId;
		if (virName != NULL)
			reader.GetName(virId, virName);
	}

	// 扫描结束
	FT_ReleaseScanPool(pAVE);
	pFile->Dispose();
	if (notifyCallback != NULL)
		notifyCallback->OnScanEnd(path);

	return virId;
}

ILinuxScan* NewInstance_ILinuxScan(IScanNotify* notifyCallback)
{
	return new LinuxScan(notifyCallback);
}