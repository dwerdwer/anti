#include "Include/LinuxUtils.h"
#include "Include/LinuxScan.h"

#include <Module.h>
// #include <time.h>

#include "../AntiVirEngine/Include/VL_Type.h"
#include "../Interface/CEngData.h"
// #include "../AntiVirEngine/Include/pub.h"

// #include "../Util/std_base.h"
#include "../VirusName/VirusFileAPI.h"


/*
#include "../../KVPAV2017/AVE/!KVE/AntiVirEngine/Include/Pub.h"
#include "../../KVPAV2017/AVE/!KVE/AntiVirEngine/kv0048/OS/OS.h"
#include "../../KVPAV2017/AVE/!KVE/AntiVirEngine/kv0051/Base/FI.h"
#include "../../KVPAV2017/AVE/!KVE/AntiVirEngine/kv0050/VM/VM.h"
*/


// #pragma comment (lib,"AVOSEnv.lib")
#pragma comment (lib,"AVUtil.lib")
#pragma comment (lib,"AVFRec.lib")
#pragma comment (lib,"AVPkMgr.lib")
#pragma comment (lib,"AVScanEP.lib")

int  AV_CALLTYPE SCANFT_File(CEngData* pAVEData, int blCure=0);
int  AV_CALLTYPE SCANFT_Clear();
int  AV_CALLTYPE SCANFT_Init(const char* rootPath);

extern "C" void Check_Error(CEngData* pAVE);

int g_fileCount;
int g_virusCount;
int g_unpackCount;
int g_unpackFailCount;
extern BOOL g_blShowDebug;

char	g_szCurrentFilePath[MAX_PATH];
char 	szLog[MAX_PATH];
FILE* 	g_fLog;
extern bool g_blLog;

NameDatReader reader;

int VNAME_Init(const char* pRoot)
{
	char  vNameFile[MAX_PATH];
	sprintf(vNameFile, "%sFTName.dat", pRoot);

	if (reader.Init(vNameFile) == 0)
	{
		return 0;
	}
	return -1;
}


int Init_AVScan(const char* pRoot)
{
	if (pRoot == NULL || *pRoot == 0)
		return -1;

	FT_Init(pRoot);
	PKEXE_Init(pRoot);

	if (SCAN_Init(pRoot) != 0)
		return -1;
	if ( VNAME_Init(pRoot) !=0 )
		return -2;

	if (g_blLog)
	{
		char szLog[MAX_PATH];
	    time_t now;
	    struct tm *tm_now;
	 
	    time(&now);
	    tm_now = localtime(&now);
	 	sprintf(szLog, "%04d%02d%02d_%02d%02d_%02d.txt", tm_now->tm_year, tm_now->tm_mon, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
		g_fLog = fopen(szLog, "w+");
	}

	return 0;
}

void Finish_AVScan()
{
	if (g_fLog != NULL)
		fclose(g_fLog);
	g_fLog = NULL;

	FT_Clear();
	PKEXE_Clear();
	SCAN_Clear();

}

const char* GetCPUName(WORD wCPUType)
{
	switch (wCPUType)
	{
	case CPU_X86:	return "x86";
	case CPU_X64:	return "x64";
	case CPU_ARM32:	return "arm";
	case CPU_ARM64:	return "a64";
	}

	return "Unk";
}

const char* GetImageName(WORD wImageType)
{
		switch (wImageType)
		{
		case IMAGE_EXE:	return "EXE";
		case IMAGE_DLL:	return "DLL";
		case IMAGE_SYS:	return "SYS";
		}

	return "Unk";
}



char buf[4096];

int AV_ScanFile_Internal(CEngData* pAVE, int deepth, IScanNotify* notifyCallback)
{
	int recId = 0;
	int offs;

	recId = SCAN_File(pAVE, 0);
	if (recId)
	{
		g_virusCount++;
		char vName[0x100] = "";
		reader.GetName(recId, vName);
		//		printf("\t===>Virus: ID(%d, %s), %s\n", recId, vName,(LPCSTR) CStringA(pAVE->Full_Name));
		if (!g_blShowDebug || !deepth)
			printf("%s\n", (LPCSTR)pAVE->hScanFile->GetPath());
		printf("\t===>Virus: VID(%8d, 0x%08X) %s\n", recId, recId, vName);
		printf("\t    FileType: %02X(%3d), wFileType: %02X(%3d),SubType:%02X(%3d), InfoFlag = %08X\n",
			pAVE->File_Type, pAVE->File_Type, pAVE->wFileType, pAVE->wFileType, pAVE->wSubType, pAVE->wSubType,
			pAVE->dwInfoFlags);

		if (notifyCallback != NULL)
		{
			notifyCallback->OnFindVirus((LPCSTR)pAVE->hScanFile->GetPath(), vName, recId, ERESULT_FIND);
		}
	}

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

	printf("    %s\n",buf);

	if (g_fLog!=NULL && (pAVE->dwInfoFlags&(INFO_HAS_SPECIAL_SEC| INFO_HAS_EXECDATA)))
	{
		char buf2[4096];

		offs = sprintf(buf2, "%s\n    %s\n", g_szCurrentFilePath, buf);
		fwrite( buf2, 1, offs, g_fLog);
	}

	return recId;
}

int ScanFile_Loop(CEngData* pAVE, int deepth, IScanNotify* notifyCallback)
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
		virId = AV_ScanFile_Internal(pAVE, deepth, notifyCallback);
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
			if (decodeId>0) {
				if (g_blShowDebug)
					printf("%s\n\tUnpack success: UpID(%4d (0x%04X), %-12s), %s\n",
					(LPCSTR)pAVE->hScanFile->GetPath(), decodeId, decodeId, PKEXE_GetName(decodeId), (LPCSTR)pTempFile->GetPath());
				if (notifyCallback != NULL)
				{
					notifyCallback->OnFindExep((LPCSTR)pAVE->hScanFile->GetPath(), PKEXE_GetName(decodeId), decodeId, ERESULT_EXEP_OK);
				}

				// 扫描解压出的文件
				CEngData* pAVE2 = FT_GetScanPool(pAVE->hTempP);
				if (pAVE != NULL) {
					g_unpackCount++;
					ScanFile_Loop(pAVE2, deepth+1, notifyCallback);

					// 扫描结束
					FT_ReleaseScanPool(pAVE2);
				}
			}
			else {
				if (decodeId < 0) {
					g_unpackFailCount++;
					if (g_blShowDebug)
						printf("%s\n\tUnpack fail   : UpID(%4d (0x%04X), %-12s) *********************\n",
						(LPCSTR)pAVE->hScanFile->GetPath(), decodeId, 0-decodeId, PKEXE_GetName(0-decodeId));
					if (notifyCallback != NULL)
					{
						notifyCallback->OnFindExep((LPCSTR)pAVE->hScanFile->GetPath(), PKEXE_GetName(0-decodeId), 0-decodeId, ERESULT_EXEP_FAIL);
					}
				}

				// Not pack_exe
			}
			pTempFile->Dispose();
		}
	} while ((ret = FT_RecongizeNext(pAVE, &result)) == 0);

	return virId;
}

int ScanFile_IFile(LPCSTR path, SCANMODE mode, char* virName, unsigned int* pdwVirRecNo, IScanNotify* notifyCallback)
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
	int virId = ScanFile_Loop(pAVE,0, notifyCallback);
	if (virId >= 0)
	{
		if (pdwVirRecNo != NULL)
			*pdwVirRecNo = virId;
		if (virName!=NULL)
			reader.GetName(virId, virName);
	}

	// 扫描结束
	FT_ReleaseScanPool(pAVE);
	pFile->Dispose();
	if (notifyCallback != NULL)
		notifyCallback->OnScanEnd(path);

	return virId;
}