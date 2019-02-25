#ifndef _CMYTIMESPAN_
#define _CMYTIMESPAN_



//#include "platform.h"  

#ifdef _WIN32
#include <Windows.h>  
#define timelong_t ULARGE_INTEGER  

#else

#include <sys/time.h>  
#include <linux/errno.h>  
#define timelong_t struct timeval  
#endif  


class CMyTimeSpan
{
public:
	CMyTimeSpan();
	void Reset();
	unsigned long long GetSpaninMicroseconds();
	unsigned int GetSpaninMilliseconds();
	unsigned int GetSpaninSeconds();

private:
	timelong_t m_start;
	void getCurrentTimeLong(timelong_t *tl);

};
//=====================================================================================  







#endif // !_CMYTIMESPAN_
