#include <stdio.h>
#include <stdlib.h>

#include "../Include/pub.h"

///////////////////////////////////////////
//	call back for debug
///////////////////////////////////////////

extern "C" int	DebugWriteFile(char* filename, BYTE* pBuffer, int Len)
{
	FILE	*fp;
	if ((fp=fopen(filename,"wb")) == NULL)
		return false;
	fwrite(pBuffer, 1, Len, fp);
	fclose(fp);
	return true;
}

extern "C" int	DebugAppendFile(char* filename, BYTE* pBuffer, int Len)
{
	FILE	*fp;
	if ((fp=fopen(filename,"a+b")) == NULL)
		return false;
	(void)fseek(fp, 0, 2);
	fwrite(pBuffer, 1, Len, fp);
	fclose(fp);
	return true;
}

/*extern "C" __declspec(naked) int DebugPrintf(const char* format,...)
{
	__asm jmp printf;
}
*/

extern "C" int DebugPrintf(const char* format,...)
{
	//char	buf[1024];   
	//va_list	va;   

	//va_start(va, format);   
	//vsprintf(buf, format, va);   
	//va_end(va);   

	//OutputDebugString(buf);
	return 0;
}

extern "C" int FilePrintf(const char* filename, const char* format,...)
{
	//FILE	*fp;
	//if ((fp=fopen(filename,"a+")) == NULL)
	//	return -1;

	//va_list	va;   

	//va_start(va, format);   
	//vfprintf(fp, format, va);   
	//va_end(va);   

	//fclose(fp);

	return 0;
}