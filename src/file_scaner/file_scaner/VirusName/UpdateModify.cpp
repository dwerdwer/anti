#include "VirusFileAPI.h"
#include "ImportData.h"

using namespace std;

void Create_HeaderInfo(HeaderInfo* &headerInfo, SectionInfo* &secInfos, int_t eleSum);

SectionElementLen* Create_SecEleLens(map<long, const char*> &VirusMap,
	int_t eleSum, SectionElementName* &secEleNames, int_t baseNo);

void Create_SecEleNames(map<long, const char*> &VirusMap,
	int_t eleSum, SectionElementLen* secEleLens, SectionElementName* &secEleNames);


/* 添加内容 */
struct UpdatePar
{
	int secIndexStart; // 添加节起始
	int secIndexEnd;   // 添加节末尾
	int newEleSum;     // 添加元素总数

	int_t eleSum;     // 生成文件元素总数
	int_t secSum;     // 生成文件的节总数

	SectionElementLen* secEleLens;   // 长度信息数组
	SectionElementName* secEleNames; // 元素名称数组

	char* newEleBuf;  // 添加源数据
};


/* 获取添加内容 */
int Get_NewElements(IN map<long, const char*> &VirusMap, IN UpdatePar &UP, IN const char* readPath)
{
	UP.newEleBuf = Get_FileBuf(readPath, NULL);

	if (!UP.newEleBuf)
		return -1;

	char* strBuf = UP.newEleBuf;
	char* nameStr = NULL;

	int keyNo = 0; // 编号
	int keyLen = 0; // 编号长度

	int i = 0;

	while ((nameStr = win_strsep(&strBuf, "\n")) != NULL)
	{
		keyNo = atoi(nameStr);
		keyLen = 0;

		for (int key = keyNo; key; keyLen++)
			key = key / 10;

		// 跳过空格
		i = 1;
		while (*(nameStr + keyLen + i) == ' ')
			i++;

		int nameLen = (int)strlen(nameStr + keyLen + 1);

		if (keyNo != 0 && nameLen != 0)
		{
			// 判断 Name 是否包含空格
			for (int j = 0; j < nameLen; ++j)
			{
				if ((nameStr + keyLen + 1)[j] == ' ')
					return -1;
			}

			if (strlen(nameStr + keyLen + 1) < MAX_VIR_NAME)
				VirusMap.insert(make_pair(keyNo, nameStr + keyLen + 1));
			else
				VirusMap.insert(make_pair(keyNo, "Exceed MAX_VIR_NAME"));
		}
	}
	return 0;
}


/* 合并新旧数据 */
static int Merge_Section(IN map<long, const char*> &VirusMap,
	IN UpdatePar &UP, NameDatReader* oldDatR, int newBase, OUT char** &nameBuf)
{
	int baseNo = (int)oldDatR->headerInfo->baseNo;

	map<long, const char*>::iterator endIt = VirusMap.end();
	endIt--;
	// newDate 最大元素
	int_t newMaxEle = endIt->first;

	// 确定修改范围
	UP.secIndexStart = newBase / SECTION_HEAD_SIZE;

	UP.secIndexEnd = (int)newMaxEle / SECTION_HEAD_SIZE;

	int eleStart = newBase - newBase % SECTION_HEAD_SIZE + baseNo; // 起始索引
	int eleEnd = 0; // 最后索引

	// 总数增多
	if (newMaxEle > oldDatR->headerInfo->maxEle)
	{
		// cout << " Add virus name" << endl;

		UP.secSum = newMaxEle / SECTION_HEAD_SIZE + 1;

		if (newMaxEle % SECTION_HEAD_SIZE == 0)
			UP.secSum = newMaxEle / SECTION_HEAD_SIZE;

		/* 补全尾部 */
		UP.eleSum = UP.secSum * SECTION_HEAD_SIZE;
		/* Optional */

		if (newBase >= (int)oldDatR->headerInfo->maxSec * SECTION_HEAD_SIZE)
		{
			if (oldDatR->headerInfo->maxSec == 0)
			{
				UP.secIndexStart = 0;
				eleStart = baseNo;
			}
			else 
			{
				eleStart = (int)(oldDatR->headerInfo->maxSec) * SECTION_HEAD_SIZE + baseNo;
				UP.secIndexStart = eleStart / SECTION_HEAD_SIZE;
			}
		}
		eleEnd = (int)UP.eleSum;

		UP.newEleSum = eleEnd - eleStart + 1;
	}
	// 总数不变
	else if (newMaxEle <= oldDatR->headerInfo->maxEle)
	{
		// 若整除，从前一节开始
		if (newBase % SECTION_HEAD_SIZE == 0)
		{
			eleStart = newBase - SECTION_HEAD_SIZE + baseNo;
			UP.secIndexStart -= 1;
		}

		if (eleStart < baseNo + SECTION_HEAD_SIZE)
		{
			cout << " This newBase is not supported" << endl;
			return -1;
		}

		UP.eleSum = oldDatR->headerInfo->eleSum;

		eleEnd = (int)newMaxEle - newMaxEle % SECTION_HEAD_SIZE + SECTION_HEAD_SIZE + baseNo - 1;

		// 尾节可能不足 SECTION_HEAD_SIZE
		if (UP.secIndexEnd == (int)oldDatR->headerInfo->maxSec)
			eleEnd = (int)oldDatR->headerInfo->eleSum + baseNo - 1;
		/* Optional */

		UP.newEleSum = eleEnd - eleStart + 1;
	}
	// 原数据并入集合
	nameBuf = (char**)malloc(UP.newEleSum * sizeof(char*));

	for (int i = 0; i < UP.newEleSum; ++i)
	{
		nameBuf[i] = (char*)malloc(MAX_VIR_NAME);
		memset(nameBuf[i], 0, MAX_VIR_NAME);
	}

	int start = eleStart; // 起始索引
	int index = 0;

	while (eleStart <= eleEnd)
	{
		oldDatR->GetName(eleStart, nameBuf[index]);

		if (strlen(nameBuf[index]) != 0)
		{
			VirusMap.insert(make_pair(eleStart, nameBuf[index]));
		}
		index++;
		eleStart++;
	}

	// 重构节数据
	UP.secEleLens = Create_SecEleLens(VirusMap, UP.newEleSum, UP.secEleNames, start);

	Create_SecEleNames(VirusMap, UP.newEleSum, UP.secEleLens, UP.secEleNames);

	return 0;
}

/* 修正偏移 */
static int Fix_SecOffset(NameDatReader* oldDatR, OUT SectionInfo* &newSecInfos, IN UpdatePar &UP)
{
	if (!UP.secEleLens || !UP.secEleNames)
		return -1;

	newSecInfos = (SectionInfo*)malloc(UP.secSum * sizeof(SectionInfo));
	if (!newSecInfos)
	{
		perror("malloc error");
		return -1;
	}
	memset(newSecInfos, 0, UP.secSum * sizeof(SectionInfo));

	int nameIndex = 0;	// name len 索引
	int secIndex = 0;	// section 索引

	int_t secOffset = 0;
	// 如果总数不变
	if (UP.eleSum == oldDatR->headerInfo->eleSum)
	{
		// 添加节之前
		if (UP.secIndexStart == 0)
			secOffset = oldDatR->headerInfo->firstSecOffset;
		else
			secOffset = oldDatR->secInfos[UP.secIndexStart - 1].secEndOffset;

		while (secIndex < UP.secIndexStart)
		{
			newSecInfos[secIndex].secEndOffset = oldDatR->secInfos[secIndex].secEndOffset;

			secIndex++;
		}
	}
	// 如果总数增多
	else if (UP.eleSum > oldDatR->headerInfo->eleSum)
	{
		// 添加节之前
		int_t addHead = (UP.secSum - (oldDatR->headerInfo->maxSec + 1)) * sizeof(SectionInfo);

		while (secIndex < UP.secIndexStart)
		{
			newSecInfos[secIndex].secEndOffset = oldDatR->secInfos[secIndex].secEndOffset + addHead;
			secIndex++;
		}

		if (UP.secIndexStart == 0)
			secOffset = oldDatR->headerInfo->firstSecOffset + addHead;
		else
			secOffset = newSecInfos[secIndex - 1].secEndOffset;
	}
	// 添加节及之后新偏移
	for (; secIndex <= UP.secIndexEnd; ++secIndex)
	{
		secOffset += SECTION_HEAD_SIZE;// 偏移值在节区最后

		for (int_t i = 0; i < SECTION_HEAD_SIZE; ++i, ++nameIndex)
		{
			if (nameIndex >= UP.newEleSum)
				break;

			// 如果剩余长度 ！= OUTLINE_MAX_LEN
			if (UP.secEleLens[nameIndex].currentLen != OUTLINE_MAX_LEN)
			{
				secOffset += UP.secEleLens[nameIndex].currentLen;
			}
			else // 剩余实际长度在首字节
			{
				secOffset += 1;
				secOffset += UP.secEleNames[nameIndex].lenByte;
			}
		}
		newSecInfos[secIndex].secEndOffset = secOffset;
	}

	// 尾节元素数
	int_t lastEleSum = UP.newEleSum % SECTION_HEAD_SIZE;

	// 可能不足 SECTION_HEAD_SIZE
	if (lastEleSum != 0)
		newSecInfos[secIndex - 1].secEndOffset = secOffset - (SECTION_HEAD_SIZE - lastEleSum);

	// 如果总数不变
	if (UP.eleSum == oldDatR->headerInfo->eleSum)
	{
		int_t addOffset = newSecInfos[secIndex - 1].secEndOffset - oldDatR->secInfos[secIndex - 1].secEndOffset;

		while (secIndex < (int)UP.secSum)
		{
			newSecInfos[secIndex].secEndOffset = oldDatR->secInfos[secIndex].secEndOffset + addOffset;

			secIndex++;
		}
	}
	return 0;
}


/* 写入新索引文件 */
static int Write_NewFile(NameDatReader* oldDatR, OUT HeaderInfo* &newHeaderInfo,
	IN UpdatePar &UP, SectionInfo* newSecInfos, const char* savePath, char* readBuf)
{
	FILE* wfp = NULL;
	wfp = fopen(savePath, "wb+");

	if (NULL == wfp)
	{
		perror("fopen error");
		return -1;
	}
	// 文件头与节表
	int_t secSum = UP.eleSum / SECTION_HEAD_SIZE;

	fwrite(newHeaderInfo, sizeof(HeaderInfo), 1, wfp);

	for (int_t i = 0; i < secSum; ++i)
	{
		fwrite(&newSecInfos[i], sizeof(SectionInfo), 1, wfp);
	}
	// 改动之前的节
	if (UP.secIndexStart != 0)
	{
		int_t beforeSize = oldDatR->secInfos[UP.secIndexStart - 1].secEndOffset
			- oldDatR->headerInfo->firstSecOffset;

		fwrite(readBuf + oldDatR->headerInfo->firstSecOffset, beforeSize, 1, wfp);
	}

	// 写入修改的节
	int upSecSum = UP.newEleSum / SECTION_HEAD_SIZE;

	int offsetIndex = 0;
	int nameIndex = 0;

	register int j = 0;

	for (int i = 0; i <= upSecSum; ++i)
	{
		// 一个 section 的前半段
		for (j = 0; j < SECTION_HEAD_SIZE; ++j, ++offsetIndex)
		{
			if (offsetIndex >= UP.newEleSum)
				break;

			fwrite(&UP.secEleLens[offsetIndex], sizeof(SectionElementLen), 1, wfp);
		}
		// Names data
		for (j = 0; j < SECTION_HEAD_SIZE; ++j, ++nameIndex)
		{
			if (nameIndex >= UP.newEleSum)
				break;

			// 如果剩余长度 ！= OUTLINE_MAX_LEN
			if (UP.secEleLens[nameIndex].currentLen != OUTLINE_MAX_LEN)
			{
				fwrite(UP.secEleNames[nameIndex].virusName,	
					UP.secEleLens[nameIndex].currentLen, 1, wfp);
			}
			else // 将长度写入首字节
			{
				fwrite(&UP.secEleNames[nameIndex].lenByte, sizeof(BYTE), 1, wfp);

				fwrite(UP.secEleNames[nameIndex].virusName,
					UP.secEleNames[nameIndex].lenByte, 1, wfp);
			}
		}
	}
	// 如果总数未增加，写入之后的节
	if (UP.eleSum == oldDatR->headerInfo->eleSum)
	{
		int_t behindSize = oldDatR->headerInfo->totalSize - oldDatR->secInfos[UP.secIndexEnd].secEndOffset;

		if (behindSize != 0)
			fwrite(readBuf + oldDatR->secInfos[UP.secIndexEnd].secEndOffset, behindSize, 1, wfp);
	}

	free(readBuf);
	fclose(wfp);
	wfp = NULL;

	return 0;
}

/* 读取新txt文件，合成新索引文件 */
int UpdateNameData(OUT const char* newdatPath, const char* datPath, const char* addNameTxtPath)
{
	if (newdatPath == NULL || datPath == NULL || addNameTxtPath == NULL)
		return -1;

    int result = -1;

	map<long, const char*>VirusMap;

	UpdatePar UP = { 0 }; // 添加内容

	if (Get_NewElements(VirusMap, UP, addNameTxtPath) == -1)
		return result;

	if (VirusMap.empty())
		return result;

	NameDatReader* oldDatR = new NameDatReader;

	HeaderInfo* newHeaderInfo = new HeaderInfo; // 新的文件头

	SectionInfo* newSecInfos = NULL; // 新节头偏移

	char** nameBuf = NULL; // 病毒名缓冲区
    char* readBuf = NULL;

	if (oldDatR->Init(datPath) == -1)
	{
		delete oldDatR;
		delete newHeaderInfo;
		
		return result;
	}
	// 升级文件 baseNo
	int newBase = GetBaseNo(addNameTxtPath);

	auto minEle = VirusMap.begin();

	if (newBase > minEle->first || newBase == -1)
	    goto ErrEnd;
	
    UP.secSum = oldDatR->headerInfo->maxSec + 1;

	// 合并
	if (Merge_Section(VirusMap, UP, oldDatR, newBase, nameBuf) == -1)
	    goto ErrEnd;
	
    // 修正偏移
	if (Fix_SecOffset(oldDatR, newSecInfos, UP) == -1)
	    goto ErrEnd;
	
    // 赋值文件头
	newHeaderInfo->magic = VNAME_FILE_MAGIC;
	newHeaderInfo->baseNo = oldDatR->headerInfo->baseNo;

	Create_HeaderInfo(newHeaderInfo, newSecInfos, UP.eleSum);

	readBuf = Get_FileBuf(datPath, NULL);

	// 写入新文件
	if (Write_NewFile(oldDatR, newHeaderInfo, UP, newSecInfos, newdatPath, readBuf) == -1)
	    goto ErrEnd;

    result = 0;

ErrEnd:
    if(oldDatR) {
        delete oldDatR; oldDatR = NULL;
    }
    if (newHeaderInfo) {
        delete newHeaderInfo; newHeaderInfo = NULL;
    }
    if (UP.newEleBuf){
		free(UP.newEleBuf); UP.newEleBuf = NULL;
    }
	if (nameBuf) {
	    for (int i = 0; i < UP.newEleSum; ++i)
		    free(nameBuf[i]);
        free(nameBuf); nameBuf = NULL;
    }
	if (UP.secEleLens) {
		free(UP.secEleLens); UP.secEleLens = NULL;
    }
    if (UP.secEleNames) {
		free(UP.secEleNames); UP.secEleNames = NULL;
    }
	if (newSecInfos) {
		free(newSecInfos); newSecInfos = NULL;
    }

	return result;
}
