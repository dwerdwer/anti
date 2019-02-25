#include "ImportData.h"


/* 成功返回文件缓冲区		传出文件大小，不需要可传 NULL */
char* Get_FileBuf(const char* openPath, OUT int_t* fileSize_out)
{
	if (openPath == NULL)
		return NULL;

	FILE* rfp = fopen(openPath, "rb");

	if (!rfp)
	{
		perror("Fopen error");
		return NULL;
	}
	fseek(rfp, 0, SEEK_END);

	int_t fileSize = ftell(rfp);

	fseek(rfp, 0, SEEK_SET);

	char* fileBuf = (char*)malloc(fileSize);
	if (!fileBuf)
	{
		perror("malloc error");
		fclose(rfp);
		return NULL;
	}
	memset(fileBuf, 0, fileSize);

	if (fread(fileBuf, 1, fileSize, rfp) == 0)
	{
		perror("Fread error");
		return NULL;
	}

	fclose(rfp);
	rfp = NULL;

	if (fileSize_out)
		*fileSize_out = fileSize;

	return fileBuf;
}

/* 按 delim 分割 stringp */
char* win_strsep(char **stringp, const char *delim)
{
	char *s;
	const char *spanp;
	int c, sc;
	char *tok;
	if ((s = *stringp) == NULL)
		return (NULL);
	for (tok = s;;)
	{
		c = *s++;
		spanp = delim;
		do
		{
			if ((sc = *spanp++) == c)
			{
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
}

/* 获取txt文件起始索引 */
int GetBaseNo(const char* readPath)
{
	if (readPath == NULL)
		return -1;

	FILE* rfp = fopen(readPath, "rb");

	if (!rfp)
	{
		perror("Fopen error");
		return -1;
	}

	char strBuf[32] = { 0 };

	if (!fgets(strBuf, 32, rfp))
		return -1;

	char* baseStr = strtok(strBuf, "=");

	if (strcmp(baseStr, "baseno"))
	{
		printf("Invalid txt File\n");
		return -1;
	}
	fclose(rfp);

	return atoi(strBuf + strlen(baseStr) + 1);
}


/* 导入内容 */
int ImportData::Import(NameDatBuilder* nameDatBuilder, const char* readPath)
{
	if (readPath == NULL)
		return -1;

	readBuf = Get_FileBuf(readPath, NULL);

	if (!readBuf)
		return -1;

	char* strBuf = readBuf;
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
			
		if (keyNo != 0 && strlen(nameStr + keyLen + i) != 0)
		{
			if (nameDatBuilder->PutName(keyNo, nameStr + keyLen + i) == -1)
				return -1;
		}		
	}
	return 0;
}
