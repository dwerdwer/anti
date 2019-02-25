#include "VirusFileAPI.h"

class ImportData
{
public:
	ImportData()
	{
		this->readBuf = NULL;
	}

	/* 导入内容 */
	int Import(NameDatBuilder* nameDatBuilder, const char* readPath);

	~ImportData()
	{
		if (this->readBuf)
		{
			free(this->readBuf);
			this->readBuf = NULL;
		}
	}
private:
	char* readBuf;
};


/* 获取txt文件起始索引 */
int GetBaseNo(const char* readPath);

/* 按 delim 分割 stringp */
char* win_strsep(char **stringp, const char *delim);

/* 成功返回文件缓冲区		传出文件大小，不需要可传 NULL */
char* Get_FileBuf(const char* openPath, OUT int_t* fileSize_out);