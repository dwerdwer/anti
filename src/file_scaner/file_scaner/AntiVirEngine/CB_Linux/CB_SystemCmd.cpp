/*******************************************************************************
*						       �ļ�������Χ����                                *
********************************************************************************
*	Name : CB_File.cpp          Origian Place : BJ of China                    *
*	Create Data : 1998-2002     Now Version :   1.0                            *
*	Modify Time :               Translater : HeGong                            *
*==============================================================================*
*                        Modification History                                  *
*==============================================================================*
*         V1.0  1. Create this program.                                        *
*               2. 08/11/2005 fixed by Wangwei.�����»��ƣ��Ļ���Χ����        *
*******************************************************************************/

#include "stdafx.h"
// #include <windows.h>

#ifdef WINCE
#include "..\\..\\KVRT_CE\\KV_Scan.h"
#else
#include "../../AVEngP/CScan.h"
#endif

#include "../Include/pub.h"

// API�ṹ
//--------------------------------------------------------------------------------------
class SystemFun;
typedef DWORD (SystemFun::*ApiExecutePtr)();

#pragma pack(1)
// api����
typedef struct _SYSTEM_API
{
	SysFunType		eSysFunType;	// ��������
	char*			pAPIName;		// ����
	ApiExecutePtr	pAPIFun;		// ��������

}SYSTEM_API, *PSYSTEM_API;
#pragma pack()

// ������
//--------------------------------------------------------------------------------------
class SystemFun
{
public:
	CAntiVirEngine	*AVE;
	BYTE			*pParamBuf;			// ��������
	DWORD			dwParamNum;			// ��������
	DWORD			dwParamCount;		// ��������

	SystemFun(CAntiVirEngine *pAVE);

private:
	static SYSTEM_API	ApiSystem[];	// api����

public:
	ApiExecutePtr	GetApiPtr(SysFunType eSysFunType);
	DWORD			SetParamNum(WORD wValue);
	DWORD			Push(BYTE *pValue, DWORD dwLen);
	DWORD			Pop(BYTE *pValue, DWORD dwLen);

	// API����
	DWORD			CopyFile();	
	DWORD			CoverFile();	
	DWORD			GetSystemDirectory();	
	DWORD           MoveFileEx();
};

// ��������
SYSTEM_API SystemFun::ApiSystem[] = 
{
	{SYSCMD_COPYFILE, "CopyFile", &SystemFun::CopyFile},
	{SYSCMD_COVERFILE, "CoverFile", &SystemFun::CoverFile},
	{SYSCMD_SYSTEMDIR, "GetSystemDirectory", &SystemFun::GetSystemDirectory},
	{SYSCMD_MOVEFILE, "MoveFileEx", &SystemFun::MoveFileEx},
	{SYSCMD_UNKNOWN, NULL, NULL},
};

// �������ܣ����API������ַ
ApiExecutePtr SystemFun::GetApiPtr(SysFunType eSysFunType)
{
	SYSTEM_API  *pApiSystem = ApiSystem;

	while (pApiSystem->eSysFunType != SYSCMD_UNKNOWN)
	{
		if (pApiSystem->eSysFunType == eSysFunType)
			return pApiSystem->pAPIFun;
		pApiSystem++;
	}
	return NULL;
}

// Ĭ�Ϻ���
SystemFun::SystemFun(CAntiVirEngine *pAVE) 
{
	AVE = pAVE;
	pParamBuf = NULL;
	dwParamCount = 0;
	dwParamNum = 0;
}

// �������ܣ����ò�������
DWORD SystemFun::SetParamNum(WORD wValue)
{
	dwParamCount = 0;
	dwParamNum = wValue;
	*(WORD*)pParamBuf = wValue;
	return 1;
}

// �������ܣ�ѹ�뺯��
// ����������pValue-����ָ�룬dwLen-��������
DWORD SystemFun::Push(BYTE *pValue, DWORD dwLen)
{
	BYTE	*pParamOffAddr;	// ����ƫ�Ƶ�ַ
	WORD	wDataLen = 0;	// һ���������ݳ���
	DWORD	i;

	// �����Ƿ�Ϸ�
	if (pValue == NULL || dwParamCount > dwParamNum || dwLen == 0)
		return 0;

	// ��Ų������ȵĵ�ַ
	pParamOffAddr = pParamBuf + 2;

	// �״�ѹ�����
	if (dwParamCount == 0)
	{
		*(WORD*)(pParamOffAddr) = (WORD)dwLen;						// ѹ�����ƫ��
		memcpy(pParamOffAddr+2*dwParamNum, &pValue, dwLen);	// ѹ���������
	}
	else
	{
		// ȡǰ���������ݳ���
		for (i = 0; i < dwParamCount; i ++)
			wDataLen += *(WORD*)(pParamOffAddr+i*2);
		*(WORD*)(pParamOffAddr+dwParamCount*2) = (WORD)dwLen;				// ѹ�����ƫ��
		memcpy(pParamOffAddr+2*dwParamNum+wDataLen, &pValue, dwLen);	// ѹ���������
	}

	dwParamCount++;
	return 1;
}

// �������ܣ���������
// �������أ�0��ʧ�ܣ�1���ɹ�
// ����������dwValue-������ַ
DWORD SystemFun::Pop(BYTE *pValue, DWORD dwLen)
{
	BYTE	*pParamOffAddr;	// ����ƫ�Ƶ�ַ
	WORD	wDataLen = 0;	// һ���������ݳ���
	DWORD	i;

	// �����Ƿ�Խ��
	if (dwLen == 0 || dwParamCount > dwParamNum)
		return 0;

	// ��Ų������ȵĵ�ַ
	pParamOffAddr = pParamBuf + 2;

	// �״�ѹ�����
	if (dwParamCount == 0)
	{
		dwLen = *(WORD*)(pParamOffAddr);	
		memcpy(pValue, pParamOffAddr+2*dwParamNum, dwLen);
	}
	else
	{
		// ȡǰ���������ݳ���
		for (i = 0; i < dwParamCount; i ++)
			wDataLen += *(WORD*)(pParamOffAddr+i*2);
		dwLen = *(WORD*)(pParamOffAddr+dwParamCount*2);
		memcpy(pValue, pParamOffAddr+2*dwParamNum+wDataLen, dwLen);
	}
	dwParamCount++;
	return 1;
}

// API����
//--------------------------------------------------------------------------------------

// �������ܣ�copyFile
// ���������2
//           ����1���ַ������ͣ�����Ŀ���ļ�����
//           ����2���ַ������ͣ�����Դ�ļ�����
// �����������
// �������أ�0-ʧ�ܣ�1-�ɹ�
DWORD SystemFun::CopyFile()
{
	return 0;
//	char	*pExistingFileName;
//	char	*pNewFileName;
////	DWORD LastErrorNumber;
//
//	// ��ò���
//	if (0 == Pop((BYTE*)&pNewFileName, sizeof(BYTE*)) ||
//		0 == Pop((BYTE*)&pExistingFileName, sizeof(BYTE*)))
//	{
//		return 0;
//	}
//
//	if (pNewFileName == NULL || pExistingFileName == NULL)
//		return 0;
//	// �����ļ�
//	if (!::CopyFile(pExistingFileName, pNewFileName, TRUE))
//		return 0;
//	else
//		return 1;
}
	//--------------------------------------------------------------------------------------

	// �������ܣ�moveFile
	// ���������2
	//           ����1���ַ������ͣ�����Ŀ���ļ�����
	//           ����2���ַ������ͣ�����Դ�ļ�����
	// �����������
	// �������أ�0-ʧ�ܣ�1-�ɹ�
DWORD SystemFun::MoveFileEx()
{
	return 0;
//	char	*pExistingFileName;
//	char	*pNewFileName;
////	DWORD LastErrorNumber;
//
//	// ��ò���
//	if (0 == Pop((BYTE*)&pNewFileName, sizeof(BYTE*)) ||
//		0 == Pop((BYTE*)&pExistingFileName, sizeof(BYTE*)))
//	{
//		return 0;
//	}
//
//	if (pNewFileName == NULL || pExistingFileName == NULL)
//		return 0;
//	// �����ļ�
//	if (!::MoveFileEx(pExistingFileName, pNewFileName,MOVEFILE_DELAY_UNTIL_REBOOT))
//		return 0;
//	else
//		return 1;
}

// �������ܣ����ǵ�ǰ�ļ�
// ����������1
//           ����1��Դ���ļ�����
// �����������
// �������أ�0-ʧ�ܣ�1-�ɹ�
DWORD SystemFun::CoverFile()
{
	return 0;
	//char		*pExistingFileName;
	////BYTE		Buf[0x10000];
	//DWORD		nr, r;
	//int			ret;
	//KVFILE		hSaveHandle;
	//DWORD		dwSaveAttr; 
	//FILETIME	aTime,cTime,mTime;
	//char		name[0x200];

	//// ��ò���
	//if (0 == Pop((BYTE*)&pExistingFileName, sizeof(BYTE*)))
	//	return 0;

	//// ��Դ�ļ�
	//hSaveHandle = fopen(pExistingFileName, "rb");
	//if (hSaveHandle == NULL) 
	//	return 0;

	//// �ı䵱ǰ���ļ�״̬
	//strcpy(name,AVE->Full_Name);
	//GetFileTime(AVE->hScanFile, &cTime, &aTime, &mTime);
	//CloseHandle(AVE->hScanFile);
	//dwSaveAttr = GetFileAttributes(name);
	//SetFileAttributes(name, FILE_ATTRIBUTE_NORMAL);
	//AVE->hScanFile = fopen(name, "r+b");
	//if (AVE->hScanFile == NULL) 
	//	return 0;

	//// д�ļ�
	//SetFilePointer(AVE->hScanFile, 0, 0, SEEK_SET);
	//ret = 1;
	//do 
	//{
	//	// ��Դ�ļ�����
	//	ReadFile(hSaveHandle, AVE->B_Temp, sizeof(AVE->B_Temp), &nr, NULL);
	//	if (nr == 0)
	//		break;
	//	// д�����ļ�����
	//	WriteFile(AVE->hScanFile, AVE->B_Temp, nr, &r, NULL);
	//	if (nr!=r) 
	//	{
	//		ret = 0;
	//		break;
	//	}
	//} while (nr!=0);

	//SetFileTime(AVE->hScanFile, &cTime, &aTime, &mTime);
	//SetFileAttributes(name, dwSaveAttr);
	//return ret;
}

// �������ܣ����ϵͳ·��
// �����������
// ���������2
//           ����1���ַ���ָ��
//           ����2��DWORD
// �������أ�0-ʧ�ܣ�1-�ɹ�
DWORD SystemFun::GetSystemDirectory()
{
	return 0;
	//char	*pBuffer = NULL;
	//char	*pSize = 0;

	//if (dwParamNum != 2)
	//	return 0;

	//// ��ò���
	//if (0 == Pop((BYTE*)&pBuffer, sizeof(BYTE*)) || 
	//	0 == Pop((BYTE*)&pSize, sizeof(BYTE*)))
	//{
	//	return 0;
	//}

	//// ��ȡϵͳ·��
	//if (!::GetWindowsDirectory(pBuffer, *(DWORD*)pSize))
	//	return 0;
	//else
	//	return 1;
}

//
//--------------------------------------------------------------------------------------
// �������ܣ�ϵͳ�������
// �������أ�0��ʧ�ܣ�1���ɹ�
DWORD __thiscall CAntiVirEngine::SystemCmd(BYTE *pSystemCmdBuf)
{
	//WORD			wCmd = 0;				// ��������
	//BYTE			*p = NULL;				// ����ָ��
	//SYSTEMCMD		*pSystemCmd;
	//ApiExecutePtr	Execute;
	//SystemFun		aSystemFun(this);

	//// �����Ƿ�Ϸ�
	//if (pSystemCmdBuf == NULL)
	//	return 0;

	//// �����ʽ
	//pSystemCmd = (SYSTEMCMD*)pSystemCmdBuf;

	//// ��ȡAPI����
	//Execute = aSystemFun.GetApiPtr((SysFunType)pSystemCmd->wSysFunType);
	//if (Execute != NULL)
	//{
	//	// ��ֵ����
	//	aSystemFun.pParamBuf = (BYTE*)&pSystemCmd->wParamNum;
	//	aSystemFun.SetParamNum(pSystemCmd->wParamNum);

	//	// ִ��API
	//	return (aSystemFun.*Execute)();
	//}

	return 0;
}


