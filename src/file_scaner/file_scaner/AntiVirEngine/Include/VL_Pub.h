/****************************************************
 *
 *  �����������õĹ����ⲿ���ã��ڴ˵Ǽ�
 *
 *  �����ι���
 *  ���ڣ�2003.12.9
 *
 ****************************************************/

#include "VL_Type.h"

#ifndef VL_PUB_H
#define VL_PUB_H

// 1.Prop�ṹ
//--------------------------------------------------------------------------------------------------
#pragma pack(push,1)

// ϵͳ����
enum SysFunType
{
	SYSCMD_UNKNOWN = 0,
	SYSCMD_COPYFILE,
	SYSCMD_COVERFILE,
	SYSCMD_SYSTEMDIR,
	SYSCMD_MOVEFILE,
};
// �������ݶ��壬�����������⣬����������ΪWORD
// ����ͷ���������ͣ����������������������ȣ���������
typedef struct _SYSTEMCMD
{
	WORD			wSysFunType;	// ��������
	WORD			wParamNum;		// ��������
} SYSTEMCMD;

#pragma pack(pop)

#endif //!VL_PUB_H
