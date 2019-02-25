#include "VirusFileAPI.h"

using namespace std;

/* 计算一个字符串与前一个字符串的相同和不同字符长度 */
char g_LastName[MAX_VIR_NAME] = "";

static int_t Get_EleRepeatInfo(const char* virname, int_t* repeat)
{
    char* p = g_LastName;
    int_t current_len = 0;

    *repeat = 0;
    while (*p == *virname && *virname != 0)
    {
        (*repeat)++;
        p++;
        virname++;

        if ((*repeat) >= MAX_REPEAT_COUNT)
            break;
    }
    current_len = (int_t)strlen(virname);

    strncpy(p, virname, MAX_VIR_NAME); // 设置当前字符串为最后字符串

    return current_len;
}


/* 赋值各元素长度信息			   SectionElementName 数组赋值长度字节  */
SectionElementLen* Create_SecEleLens(map<long, const char*> &VirusMap,
                                     int_t eleSum, SectionElementName* &secEleNames, int_t baseNo)
{
    // 节区元素长度信息
    SectionElementLen* secEleLens = (SectionElementLen*)
        malloc(eleSum * sizeof(SectionElementLen));

    if (!secEleLens)
    {
        perror("malloc error");
        return NULL;
    }
    memset(secEleLens, 0, eleSum * sizeof(SectionElementLen));

    // 节区元素名
    secEleNames = (SectionElementName*)
        malloc(eleSum * sizeof(SectionElementName));

    if (!secEleNames)
    {
        perror("malloc error");
        return NULL;
    }
    memset(secEleNames, 0, eleSum * sizeof(SectionElementName));

    // 重复与当前长度
    int_t secSum = eleSum / SECTION_HEAD_SIZE;

    int_t secIndex = 0; // 节索引

    int_t eleIndex = 0; // 元素索引 

    bool headFlag = true; // 有无节首元素

    for (auto vmit = VirusMap.begin(); secIndex < secSum && vmit != VirusMap.end(); ++secIndex)
    {
        if (eleIndex >= eleSum)
            break;

        // 每个节首元素作为基准，没有重复
        secEleLens[eleIndex].repeatLen = 0;

        if ((eleIndex + baseNo) == vmit->first)
        {
            //cout << "eleIndex = " << eleIndex << " " << vmit->first << " " << vmit->second << endl;
            if (strlen(vmit->second) >= OUTLINE_MAX_LEN)
            {
                secEleLens[eleIndex].currentLen = OUTLINE_MAX_LEN;

                secEleNames[eleIndex].lenByte = (BYTE)strlen(vmit->second);
            }
            else
                secEleLens[eleIndex].currentLen = (BYTE)strlen(vmit->second);

            headFlag = true;
            ++vmit;
        }
        else
            headFlag = false; // 节首为空，需要向后累加

        // 余下元素
        ++eleIndex;

        int_t repeat = 0;  // 重复长度
        int_t current = 0; // 当前剩余长度

        for (int_t i = 0; i < SECTION_HEAD_SIZE - 1 && vmit != VirusMap.end(); ++i, ++eleIndex)
        {
            if (eleIndex >= eleSum)
                break;

            if (false == headFlag)
            {
                // 找基准元素
                while ((eleIndex + baseNo) < (int_t)vmit->first)
                    eleIndex++;

                secEleLens[eleIndex].repeatLen = 0;

                if (strlen(vmit->second) >= OUTLINE_MAX_LEN)
                {
                    secEleLens[eleIndex].currentLen = OUTLINE_MAX_LEN;

                    secEleNames[eleIndex].lenByte = (BYTE)strlen(vmit->second);
                }
                else
                    secEleLens[eleIndex].currentLen = (BYTE)strlen(vmit->second);

                headFlag = true;
                ++vmit;
            }
            else if ((eleIndex + baseNo) == vmit->first)
            {
                repeat = 0;
                current = Get_EleRepeatInfo(vmit->second, &repeat);

                secEleLens[eleIndex].repeatLen = (BYTE)repeat;

                // 当剩余长度大于等于 OUTLINE_MAX_LEN 赋值长度字节
                if (current >= OUTLINE_MAX_LEN)
                {
                    secEleLens[eleIndex].currentLen = OUTLINE_MAX_LEN;

                    // 若没有重复数据 lenByte = 原长
                    secEleNames[eleIndex].lenByte = (BYTE)(current);
                }
                else
                    secEleLens[eleIndex].currentLen = (BYTE)(current);

                ++vmit;
            }
        }
    }
    return secEleLens;
}


/* 获取元素名称 */
void Create_SecEleNames(map<long, const char*> &VirusMap,
                        int_t eleSum, SectionElementLen* secEleLens, SectionElementName* &secEleNames)
{
    if (!secEleLens || !secEleNames)
        return;

    int_t eleIndex = 0;

    for (auto vmit = VirusMap.begin(); eleIndex < eleSum; ++eleIndex)
    {
        if (secEleLens[eleIndex].currentLen != 0 || secEleLens[eleIndex].repeatLen != 0)
        {
            // 剩余长度 ！= OUTLINE_MAX_LEN
            if (secEleLens[eleIndex].currentLen != OUTLINE_MAX_LEN)
            {
                // 跳过 repeatLen
                secEleNames[eleIndex].virusName = vmit->second + secEleLens[eleIndex].repeatLen;
            }
            else// 剩余长度 == OUTLINE_MAX_LEN，表明实际长度在首字节
                secEleNames[eleIndex].virusName = vmit->second + secEleLens[eleIndex].repeatLen;

            ++vmit;
        }
    }
}


/* 构造 SectionInfo 数组 */
static SectionInfo* Create_SectionInfos(SectionElementName* secEleNames,
                                        SectionElementLen* secEleLens, int_t eleSum)
{
    if (!secEleLens || !secEleNames)
        return NULL;

    // section 数组
    int_t secSum = eleSum / SECTION_HEAD_SIZE;

    SectionInfo* secInfos = (SectionInfo*)malloc(secSum * sizeof(SectionInfo));

    if (!secInfos)
    {
        perror("malloc error");
        return NULL;
    }
    memset(secInfos, 0, secSum * sizeof(SectionInfo));

    // 计算每一节尾部的绝对偏移

    int_t nameIndex = 0;	// secEleLens 的索引
    int_t i = 0;			// section 的索引

    int_t secOffset = sizeof(HeaderInfo) + secSum * sizeof(SectionInfo);

    for (; i < secSum; ++i)
    {
        secOffset += SECTION_HEAD_SIZE;// 偏移值在节区最后

        for (int_t j = 0; j < SECTION_HEAD_SIZE; ++j, ++nameIndex)
        {
            if (nameIndex >= eleSum)// 最后一段可能不完整
                break;

            // 如果剩余长度 ！= OUTLINE_MAX_LEN
            if (secEleLens[nameIndex].currentLen != OUTLINE_MAX_LEN)
            {
                secOffset += secEleLens[nameIndex].currentLen;
            }
            else // 剩余实际长度在首字节
            {
                secOffset += 1;// + 长度字节
                secOffset += secEleNames[nameIndex].lenByte;
            }
        }
        secInfos[i].secEndOffset = secOffset;
    }

    // 最后节元素总数
    int_t lastEleSum = eleSum % SECTION_HEAD_SIZE;

    // 实际节头可能小于 SECTION_HEAD_SIZE
    if (lastEleSum != 0)
        secInfos[i - 1].secEndOffset = secOffset - (SECTION_HEAD_SIZE - lastEleSum);
    /* Optional */

    return secInfos;
}


/* 文件头 */
void Create_HeaderInfo(HeaderInfo* &headerInfo, SectionInfo* &secInfos, int_t eleSum)
{
    int_t secSum = eleSum / SECTION_HEAD_SIZE;

    int_t last = secSum - 1;

    // 文件大小 == last_section 的节尾偏移
    headerInfo->totalSize = secInfos[last].secEndOffset;

    headerInfo->maxSec = secSum - 1;

    headerInfo->eleSum = eleSum; 

    headerInfo->firstSecOffset = sizeof(HeaderInfo) + secSum * sizeof(SectionInfo);

    // 最大节占用空间
    int_t maxSecSize = secInfos[0].secEndOffset - headerInfo->firstSecOffset;

    int_t secSize = 0;

    for (int_t i = 1; i < secSum; ++i)
    {
        secSize = secInfos[i].secEndOffset - secInfos[i - 1].secEndOffset;
        if (secSize	> maxSecSize)
            maxSecSize = secSize;
    }
    // 包括节头大小
    headerInfo->maxSecSize = maxSecSize;
}


/* 写入文件 */
static void Write_CompressedFile(SectionElementLen* secEleLens, SectionElementName* secEleNames, 
                                 SectionInfo* secInfos, HeaderInfo* headerInfo, int_t eleSum, const char* savePath)
{
    if (!secEleLens || !secEleNames || !secInfos || !headerInfo)
        return;

    // 文件头与节表
    FILE* wfp = NULL;
    if( NULL == (wfp = fopen(savePath, "wb+")) )
    {
        perror("fopen error");
        return;
    }
    int_t secSum = eleSum / SECTION_HEAD_SIZE;

    fwrite(headerInfo, sizeof(HeaderInfo), 1, wfp);

    for (int_t i = 0; i < secSum; ++i)
    {
        fwrite(&secInfos[i], sizeof(SectionInfo), 1, wfp);
    }
    // 写入每个节区数据
    int_t offsetIndex = 0;
    int_t nameIndex = 0;

    register int j = 0;

    for (int_t i = 0; i < secSum; ++i)
    {
        // 一个 section 的前半段数据
        for (j = 0; j < SECTION_HEAD_SIZE; ++j, ++offsetIndex)
        {
            if (offsetIndex >= eleSum)
                break;

            fwrite(&secEleLens[offsetIndex], sizeof(SectionElementLen), 1, wfp);
        }
        // Names data
        for (j = 0; j < SECTION_HEAD_SIZE; ++j, ++nameIndex)
        {
            if (nameIndex >= eleSum)
                break;

            // 如果剩余长度 ！= OUTLINE_MAX_LEN
            if (secEleLens[nameIndex].currentLen != OUTLINE_MAX_LEN)
            {
                fwrite(secEleNames[nameIndex].virusName,
                       secEleLens[nameIndex].currentLen, 1, wfp);
            }
            else // 剩余实际长度在首字节
            {
                fwrite(&secEleNames[nameIndex].lenByte, sizeof(BYTE), 1, wfp);

                fwrite(secEleNames[nameIndex].virusName,
                       secEleNames[nameIndex].lenByte, 1, wfp);
            }
        }
    }
    fclose(wfp);
    wfp = NULL;
}


/* 传入基本索引 */
NameDatBuilder::NameDatBuilder(long baseNo)
{
    this->headerInfo = (HeaderInfo*)malloc(sizeof(HeaderInfo));

    memset(this->headerInfo, 0, sizeof(HeaderInfo));

    this->headerInfo->magic = VNAME_FILE_MAGIC;
    this->headerInfo->baseNo = baseNo;

    this->secInfos = NULL;

    this->secEleLens = NULL;
    this->secEleNames = NULL;

    // 初始化内存块
    this->blockNo = 0;

    char *initBlock = (char*)malloc(BLOCK_SIZE);

    if (initBlock == NULL)
    {
        perror("malloc error");
        return;
    }
    memset(initBlock, 0, BLOCK_SIZE);

    this->blockArray.push_back(initBlock);

    this->currentSize = 0;
}


/* 析构缓冲区 */
NameDatBuilder::~NameDatBuilder()
{
    if (this->headerInfo) {
        free(this->headerInfo); this->headerInfo = NULL;
    }
    if (this->secInfos) {
        free(this->secInfos); this->secInfos = NULL;
    }
    if (this->secEleLens) {
        free(this->secEleLens); this->secEleLens = NULL;
    }
    if (this->secEleNames) {
        free(this->secEleNames); this->secEleNames = NULL;
    }
    // 释放内存块
    for (auto bit = blockArray.begin(); bit != blockArray.end(); bit++)
    {
        if (*bit != NULL) {
            free(*bit); *bit = NULL;
        }
    }
}

/* 传入 key 值与病毒名 */
int NameDatBuilder::PutName(long no, const char* pszName)
{
    int nameLen = (int)strlen(pszName);

    // 判断 pszName 是否包含空格
    for (int i = 0; i < nameLen; ++i)
    {
        if (pszName[i] == ' ')
        {
            //fprintf(stderr, "[%s] pszName format error\n", __func__);
            return -1;
        }
    }
    // 新建内存块
    if (currentSize > BLOCK_SIZE - MAX_VIR_NAME)
    {
        this->blockNo++;

        char *tmpBlock = (char*)malloc(BLOCK_SIZE);

        if (tmpBlock == NULL)
        {
            perror("malloc error");
            return -1;
        }
        memset(tmpBlock, 0, BLOCK_SIZE);

        this->blockArray.push_back(tmpBlock);

        currentSize = 0;
    }

    // 保存病毒名字符串
    memcpy(blockArray[blockNo] + currentSize, pszName, nameLen);

    if (nameLen < MAX_VIR_NAME)
        VirusMap.insert(make_pair(no, blockArray[blockNo] + currentSize));
    else
        VirusMap.insert(make_pair(no, "Exceed MAX_VIR_NAME"));

    // 字符串以 0 相隔
    currentSize += nameLen + 1;

    return 0;
}

/* 保存索引文件   传入文件路径*/
int NameDatBuilder::SaveToFile(const char* pszDatFilePath)
{
    if (this->VirusMap.empty() || pszDatFilePath == NULL)
        return -1;

    auto endIt = this->VirusMap.end();
    endIt--;

    if (headerInfo->baseNo == 0)
        headerInfo->baseNo = this->VirusMap.begin()->first;
    else if (headerInfo->baseNo > (int_t)VirusMap.begin()->first)
        return -1;

    // 元素总数
    int_t eleSum = endIt->first + 1 - headerInfo->baseNo;

    headerInfo->maxEle = endIt->first;

    int_t secSum = eleSum / SECTION_HEAD_SIZE + 1;

    if (eleSum % SECTION_HEAD_SIZE == 0)
        secSum = eleSum / SECTION_HEAD_SIZE;

    /* 补全尾部 */
    eleSum = secSum * SECTION_HEAD_SIZE;
    /* Optional */

    // 存入长度信息
    this->secEleLens = Create_SecEleLens(this->VirusMap,
                                         eleSum, this->secEleNames, headerInfo->baseNo);

    if (this->secEleLens == NULL)
    {
        fprintf(stderr, "[BUILD VIRUS DATA] Create_SecEleLens error %s: %d\n", __FILE__, __LINE__);
        return -1;
    }

    // 获取病毒名
    Create_SecEleNames(this->VirusMap, eleSum, this->secEleLens, this->secEleNames);

    // 节尾偏移数组
    this->secInfos = Create_SectionInfos(this->secEleNames, this->secEleLens, eleSum);

    if (this->secInfos == NULL)
    {
        fprintf(stderr, "[BUILD VIRUS DATA] Create_SectionInfos error %s: %d\n", __FILE__, __LINE__);
        return -1;
    }

    // 文件头信息
    Create_HeaderInfo(this->headerInfo, this->secInfos, eleSum);

    // 写入文件
    Write_CompressedFile(this->secEleLens,
                         this->secEleNames, this->secInfos, this->headerInfo, eleSum, pszDatFilePath);

    return 0;
}
